// writer.go — IFF binary writer.
//
// Mirrors the behavior of wfsource/source/iffwrite/{binary,_iffwr,fixed}.cc
// closely enough to produce byte-identical output for the same grammar input.
// Key invariants:
//
//   - Chunk header: 4-byte FOURCC (source order) + 4-byte little-endian size,
//     payload, then align(4) of padding for siblings.
//   - The size field is written as 0xFFFFFFFF at enterChunk time and patched
//     to the real payload size at exitChunk time. Symbol table entries record
//     (payload-start, payload-size) under a `::'A'::'B'` path key that matches
//     the parser's chunkSpecifier format.
//   - .sizeof / .offsetof are resolved at use-site if the target chunk is
//     already closed; otherwise a back-patch record is queued and the slot is
//     written as 0 (sizeof) or ~0 (offsetof), matching the C++ placeholder.
//     The queue is drained by ResolveBackpatches() at end of parse.
//   - Fixed-point encoding: `val × 2^fraction` truncated to unsigned width
//     chosen by total bit count (≤8 → 1 byte, ≤16 → 2, else → 4).
//   - Strings have C-style `\n \t \\ \" \DDD` escapes translated at out_string
//     time, NOT at lex time — matching iffwrite's layering.

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type chunkFrame struct {
	id           uint32 // FOURCC, MSB-first packed
	startPos     int    // position where the 4-byte ID was written
	sizeFieldPos int    // position of the 4-byte size placeholder
	pathKey      string // full path for symbol-table registration
}

type chunkSym struct {
	pos  int // absolute position of the start of the payload
	size int // payload size (excluding 8-byte header)
}

type backpatchKind int

const (
	bpSizeof   backpatchKind = iota // write cs.size
	bpOffsetof                      // write cs.pos - 4 (+ offset)
)

type backpatchRec struct {
	kind     backpatchKind
	path     string // `::'A'::'B'`
	addend   int    // extra offset for .offsetof(ptr, N)
	writePos int    // where in the output buffer to patch
}

type writer struct {
	// Shared
	stack    []chunkFrame
	pathIDs  []uint32 // running list of open chunk IDs, used to build path keys
	symbols  map[string]chunkSym
	pending  []backpatchRec
	fillChar byte

	// Binary mode (text == false)
	buf []byte
	pos int

	// Text mode (text == true). Mirrors IffWriterText in
	// wfsource/source/iffwrite/text.cc. Note the matching C++ class pushes a
	// sentinel ChunkSizeBackpatch at ctor time, so C++ `chunkSize.size()` is
	// `1 + actual depth`; the indent formulas in that file all assume it.
	// Here we track actual depth and bake the off-by-one into the indent
	// helpers directly.
	text       bool
	textBuf    bytes.Buffer
	textOnLine int // bytes emitted by outFile on the current line, for the 100-wrap
}

func newBinaryWriter() *writer {
	return &writer{
		buf:     make([]byte, 0, 4096),
		symbols: map[string]chunkSym{},
	}
}

func newTextWriter() *writer {
	return &writer{
		text:    true,
		symbols: map[string]chunkSym{},
	}
}

// Bytes returns the current output buffer (binary mode) or the text buffer
// (text mode). Either way, it's what the caller should write to the output
// file.
func (w *writer) Bytes() []byte {
	if w.text {
		return w.textBuf.Bytes()
	}
	return w.buf
}

// grow ensures the buffer can hold at least n bytes total.
func (w *writer) grow(n int) {
	if n <= len(w.buf) {
		return
	}
	if cap(w.buf) >= n {
		w.buf = w.buf[:n]
		return
	}
	newCap := 2 * cap(w.buf)
	if newCap < n {
		newCap = n
	}
	next := make([]byte, n, newCap)
	copy(next, w.buf)
	w.buf = next
}

// writeAt writes p starting at absolute position off without changing w.pos.
func (w *writer) writeAt(p []byte, off int) {
	w.grow(off + len(p))
	copy(w.buf[off:], p)
}

// write writes p at w.pos, advancing pos and extending the buffer as needed.
func (w *writer) write(p []byte) {
	w.grow(w.pos + len(p))
	copy(w.buf[w.pos:], p)
	w.pos += len(p)
}

// --- primitive emitters ------------------------------------------------------

func (w *writer) outInt8(v uint8) {
	if w.text {
		w.textEmit(strconv.Itoa(int(int8(v))) + "y ")
		return
	}
	w.write([]byte{v})
}

func (w *writer) outInt16(v uint16) {
	if w.text {
		w.textEmit(strconv.Itoa(int(int16(v))) + "w ")
		return
	}
	var b [2]byte
	binary.LittleEndian.PutUint16(b[:], v)
	w.write(b[:])
}

func (w *writer) outInt32(v uint32) {
	if w.text {
		w.textEmit(strconv.FormatInt(int64(int32(v)), 10) + "l ")
		return
	}
	var b [4]byte
	binary.LittleEndian.PutUint32(b[:], v)
	w.write(b[:])
}

// align pads with zero bytes until pos % n == 0 (used between sibling chunks
// and for out_id alignment — this one writes literal zeros, never fillChar).
func (w *writer) align(n int) {
	if w.text {
		return // IffWriterText::align is a no-op
	}
	if n <= 1 {
		return
	}
	rem := w.pos % n
	if rem == 0 {
		return
	}
	pad := n - rem
	var zero [1]byte
	for i := 0; i < pad; i++ {
		w.write(zero[:])
	}
}

// alignFunction is the user-callable .align(N). Unlike align(), it pads with
// fillChar, matching iffwrite's IffWriterBinary::alignFunction.
func (w *writer) alignFunction(n int) {
	if w.text {
		return
	}
	if n <= 1 {
		return
	}
	rem := w.pos % n
	if rem == 0 {
		return
	}
	pad := n - rem
	fc := [1]byte{w.fillChar}
	for i := 0; i < pad; i++ {
		w.write(fc[:])
	}
}

func (w *writer) setFillChar(b byte) { w.fillChar = b }

// outID writes a FOURCC with 4-byte alignment in front. The value is packed
// MSB-first in the uint32, so writing it big-endian matches source-order bytes.
func (w *writer) outID(id uint32) {
	if w.text {
		w.textEmit("'" + idName(id) + "' ")
		return
	}
	w.align(4)
	var b [4]byte
	binary.BigEndian.PutUint32(b[:], id)
	w.write(b[:])
}

// --- chunks -----------------------------------------------------------------

func (w *writer) enterChunk(id uint32) {
	depth := len(w.stack)
	w.pathIDs = append(w.pathIDs, id)
	pathKey := buildPathKey(w.pathIDs)

	if w.text {
		// `\n<tabs>{ 'ID' ` — no leading tabs at the outermost level.
		w.textBuf.WriteByte('\n')
		for i := 0; i < depth; i++ {
			w.textBuf.WriteByte('\t')
		}
		w.textBuf.WriteString("{ '")
		w.textBuf.WriteString(idName(id))
		w.textBuf.WriteString("' ")
		w.stack = append(w.stack, chunkFrame{id: id, pathKey: pathKey})
		return
	}

	w.align(4)
	startPos := w.pos
	// ID (4 bytes, source order)
	var idBytes [4]byte
	binary.BigEndian.PutUint32(idBytes[:], id)
	w.write(idBytes[:])
	// Size placeholder (4 bytes, LE, -1 == 0xFFFFFFFF)
	sizeFieldPos := w.pos
	w.outInt32(0xFFFFFFFF)

	w.stack = append(w.stack, chunkFrame{
		id:           id,
		startPos:     startPos,
		sizeFieldPos: sizeFieldPos,
		pathKey:      pathKey,
	})
}

func (w *writer) exitChunk() {
	if len(w.stack) == 0 {
		panic("exitChunk with empty stack")
	}
	top := w.stack[len(w.stack)-1]
	w.stack = w.stack[:len(w.stack)-1]
	w.pathIDs = w.pathIDs[:len(w.pathIDs)-1]
	depth := len(w.stack) // depth *after* the pop

	if w.text {
		w.textBuf.WriteByte('\n')
		for i := 0; i < depth; i++ {
			w.textBuf.WriteByte('\t')
		}
		w.textBuf.WriteByte('}')
		return
	}

	payloadStart := top.sizeFieldPos + 4
	size := w.pos - payloadStart

	// Patch the size field in place (LE).
	var szBytes [4]byte
	binary.LittleEndian.PutUint32(szBytes[:], uint32(size))
	w.writeAt(szBytes[:], top.sizeFieldPos)

	// Record in symbol table.
	w.symbols[top.pathKey] = chunkSym{pos: payloadStart, size: size}

	// Pad to the sibling boundary. exitChunk's align is zeros, not fillChar
	// (matches IffWriterBinary::exitChunk → align(4)).
	w.align(4)
}

// --- sizeof / offsetof ------------------------------------------------------

// findSymbol returns the payload (pos, size) for a chunk path already closed.
func (w *writer) findSymbol(path string) (chunkSym, bool) {
	sym, ok := w.symbols[path]
	return sym, ok
}

// emitSizeof either resolves immediately or queues a backpatch.
// In text mode, unresolved references are emitted as 0 (text output has no
// random-access backpatching).
func (w *writer) emitSizeof(path string) {
	if sym, ok := w.findSymbol(path); ok {
		w.outInt32(uint32(sym.size))
		return
	}
	if w.text {
		w.outInt32(0)
		return
	}
	w.pending = append(w.pending, backpatchRec{
		kind:     bpSizeof,
		path:     path,
		writePos: w.pos,
	})
	w.outInt32(0)
}

// emitOffsetof either resolves immediately or queues a backpatch. `addend` is
// the constant second argument of `.offsetof(path, N)`.
func (w *writer) emitOffsetof(path string, addend int) {
	if sym, ok := w.findSymbol(path); ok {
		w.outInt32(uint32(sym.pos - 4 + addend))
		return
	}
	if w.text {
		w.outInt32(0)
		return
	}
	w.pending = append(w.pending, backpatchRec{
		kind:     bpOffsetof,
		path:     path,
		addend:   addend,
		writePos: w.pos,
	})
	w.outInt32(0xFFFFFFFF)
}

// ResolveBackpatches walks the pending queue after the parse completes. Any
// unresolved symbol is a hard error — the user referenced a chunk that never
// appeared in the input. Text mode skips this entirely (no back-patches are
// queued in the first place).
func (w *writer) ResolveBackpatches() error {
	if w.text {
		return nil
	}
	var missing []string
	for _, bp := range w.pending {
		sym, ok := w.findSymbol(bp.path)
		if !ok {
			missing = append(missing, bp.path)
			continue
		}
		var val int32
		switch bp.kind {
		case bpSizeof:
			val = int32(sym.size)
		case bpOffsetof:
			val = int32(sym.pos - 4 + bp.addend)
		}
		var b [4]byte
		binary.LittleEndian.PutUint32(b[:], uint32(val))
		w.writeAt(b[:], bp.writePos)
	}
	if len(missing) > 0 {
		return fmt.Errorf("unresolved chunk references: %s", strings.Join(missing, ", "))
	}
	return nil
}

// --- strings ----------------------------------------------------------------

// outString writes a C string (translated escapes + terminating NUL).
func (w *writer) outString(s string) {
	if w.text {
		w.textEmitString(s)
		return
	}
	translated := translateEscapeCodes(s)
	w.write([]byte(translated))
	var zero [1]byte
	w.write(zero[:])
}

// outStringContinue seeks back over the previous NUL and appends another
// NUL-terminated string. Used for adjacent-string concatenation in the
// grammar's `string_list` rule — matches IffWriterBinary::out_string_continue.
//
// In text mode, IffWriterText::out_string_continue just calls out_string again
// (emitting the second string as a separate quoted literal), matching the
// C++ implementation.
func (w *writer) outStringContinue(s string) {
	if w.text {
		w.outString(s)
		return
	}
	if w.pos > 0 {
		w.pos--
	}
	w.outString(s)
}

// translateEscapeCodes mirrors iffwrite/binary.cc's translate_escape_codes:
// `\n` → newline, `\t` → tab, `\\` → backslash, `\"` → quote, `\DDD` →
// decimal byte value (wrapping modulo 256 for values > 255, matching the C
// implementation's `atoi(...) & 0xFF` path).
func translateEscapeCodes(s string) string {
	var out []byte
	for i := 0; i < len(s); i++ {
		if s[i] != '\\' || i+1 >= len(s) {
			out = append(out, s[i])
			continue
		}
		c := s[i+1]
		switch {
		case c == 't':
			out = append(out, '\t')
			i++
		case c == 'n':
			out = append(out, '\n')
			i++
		case c == '\\':
			out = append(out, '\\')
			i++
		case c == '"':
			out = append(out, '"')
			i++
		case c >= '0' && c <= '9':
			// Decimal run (matches atoi behavior).
			j := i + 1
			for j < len(s) && s[j] >= '0' && s[j] <= '9' {
				j++
			}
			var n int
			for k := i + 1; k < j; k++ {
				n = n*10 + int(s[k]-'0')
			}
			out = append(out, byte(n&0xFF))
			i = j - 1
		default:
			// Unknown escape — emit literally.
			out = append(out, s[i], c)
			i++
		}
	}
	return string(out)
}

// outStringPad writes a string with a fixed total byte count, padding with
// zero bytes if shorter. Matches the grammar action for `"..."(N)` literals.
// Returns the byte-count overrun if the string plus NUL exceeds N (the caller
// is expected to warn in that case).
//
// In text mode, padding is not applied — IffWriterText doesn't emit padding
// bytes (they'd produce ugly `0y 0y 0y …` runs), so we just emit the string.
func (w *writer) outStringPad(s string, totalBytes int) int {
	if w.text {
		w.outString(s)
		return 0
	}
	w.outString(s)
	required := len(translateEscapeCodes(s)) + 1 // NUL
	if required >= totalBytes {
		return required - totalBytes
	}
	var zero [1]byte
	for i := 0; i < totalBytes-required; i++ {
		w.write(zero[:])
	}
	return 0
}

// --- timestamp, file ---------------------------------------------------------

func (w *writer) outTimestamp(unixSeconds int64) {
	w.outInt32(uint32(unixSeconds))
}

// outFile copies the raw bytes of an external file into the current chunk's
// payload, optionally sliced by [start, start+length). `length == ^uint64(0)`
// is the sentinel for "unlimited" (use the entire file from start onward).
// In text mode, the file is unrolled as individual `<byte>y ` items with a
// ` //\n<tabs>` wrap every 100 bytes (matching IffWriterText::out_mem).
func (w *writer) outFile(path string, start, length uint64) error {
	data, err := os.ReadFile(path)
	if err != nil {
		return fmt.Errorf("read %s: %w", path, err)
	}
	if start >= uint64(len(data)) {
		return fmt.Errorf("%s: start %d past EOF (%d bytes)", path, start, len(data))
	}
	end := uint64(len(data))
	if length != ^uint64(0) && start+length < end {
		end = start + length
	}
	slice := data[start:end]

	if w.text {
		for _, b := range slice {
			w.textEmit(strconv.Itoa(int(int8(b))) + "y ")
			w.textOnLine++
			if w.textOnLine == 100 {
				w.textEmitComment()
				w.textOnLine = 0
			}
		}
		// Emit a trailing comment marker regardless of boundary
		// (matches C++ out_mem's `out_comment(Comment(""))` at the end).
		w.textEmitComment()
		w.textOnLine = 0
		return nil
	}

	w.write(slice)
	return nil
}

// --- fixed-point ------------------------------------------------------------

// outFixed encodes a real value as `val × 2^fraction` truncated to an unsigned
// integer of width determined by total bit count (sign+whole+fraction).
//
// For negative values C's undefined cast behavior is reproduced as modular
// wrapping via int64 intermediate, which is what gcc emits on x86-64 and what
// the reference oracle produces.
//
// In text mode, the value is emitted as `<iostream-float>(<S>.<W>.<F>) `,
// matching IffWriterText::out_scalar. The `%#.16g` format — alt form keeps
// the decimal point and trailing zeros — is Go's closest match to C++'s
// `setprecision(16) + showpoint` combination.
func (w *writer) outFixed(val float64, prec sizeSpec) {
	if w.text {
		s := fmt.Sprintf("%#.16g", val)
		w.textEmit(fmt.Sprintf("%s(%d.%d.%d) ", s, prec.sign, prec.whole, prec.fraction))
		return
	}
	bits := prec.sign + prec.whole + prec.fraction
	scaled := int64(val * float64(uint64(1)<<uint(prec.fraction)))
	switch {
	case bits > 16:
		w.outInt32(uint32(scaled))
	case bits > 8:
		w.outInt16(uint16(scaled))
	default:
		w.outInt8(uint8(scaled))
	}
}

// --- text mode helpers -------------------------------------------------------

// textEmit appends raw text to the text buffer.
func (w *writer) textEmit(s string) { w.textBuf.WriteString(s) }

// textEmitString formats a Go string as a C++-iostream-style quoted literal:
// `"body"`, with `\n` producing a `\n"` + newline + indent + `"` line-break,
// `\\` and `"` escaped with `\`, other bytes passed through. Matches
// IffWriterText::out_string.
func (w *writer) textEmitString(s string) {
	w.textBuf.WriteByte('"')
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch c {
		case '\n':
			w.textBuf.WriteString(`\n"`)
			w.textBuf.WriteByte('\n')
			w.textBuf.WriteByte('"')
		case '\\':
			w.textBuf.WriteString(`\\`)
		case '"':
			w.textBuf.WriteString(`\"`)
		default:
			w.textBuf.WriteByte(c)
		}
	}
	w.textBuf.WriteString(`" `)
}

// textEmitComment writes the ` //\n<tabs>` wrap marker used by out_file.
// IffWriterText::out_comment uses `tab(chunkSize.size()+1)` where chunkSize is
// `1 + depth` at comment time; flattening the off-by-one gives us `depth + 1`
// literal tab characters here.
func (w *writer) textEmitComment() {
	w.textBuf.WriteString(" //\n")
	tabs := len(w.stack) + 1
	for i := 0; i < tabs; i++ {
		w.textBuf.WriteByte('\t')
	}
}

// --- path key helpers -------------------------------------------------------

// idName decodes a FOURCC back to its source-order ASCII, trimming trailing NULs.
// Matches ID::name() in wfsource/source/iffwrite/id.cc for typical 1–4 char IDs.
func idName(id uint32) string {
	b := []byte{byte(id >> 24), byte(id >> 16), byte(id >> 8), byte(id)}
	n := 4
	for n > 0 && b[n-1] == 0 {
		n--
	}
	return string(b[:n])
}

// buildPathKey produces `::'A'::'B'::'C'` from the running chunk-ID stack.
func buildPathKey(ids []uint32) string {
	var sb strings.Builder
	for _, id := range ids {
		sb.WriteString("::'")
		sb.WriteString(idName(id))
		sb.WriteString("'")
	}
	return sb.String()
}
