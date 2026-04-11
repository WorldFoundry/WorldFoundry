// lexer.go — hand-rolled tokenizer for the iffcomp DSL.
//
// Mirrors the rules in wftools/iffcomp/lang.l. The DSL is small (about a dozen
// keyword tokens, char / integer / real / string literals, a handful of
// punctuation), so a hand-rolled scanner is faster to write and easier to
// debug than dragging in a lex/yacc-style generator.

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strconv"
)

type tokKind int

const (
	tokEOF tokKind = iota
	tokLBrace
	tokRBrace
	tokLParen
	tokRParen
	tokLBrack
	tokRBrack
	tokComma
	tokDoubleColon
	tokPlus
	tokMinus
	tokSizeY // literal 'Y'
	tokSizeW // literal 'W'
	tokSizeL // literal 'L'
	tokInteger
	tokReal
	tokString
	tokCharLit    // 1–4 char FOURCC, packed MSB-first into uint32
	tokPrecision  // .precision
	tokPrecSpec   // bare `N.N.N` sign.whole.fraction triple
	tokTimestamp  // .timestamp
	tokAlign      // .align
	tokOffsetof   // .offsetof
	tokSizeof     // .sizeof
	tokFillChar   // .fillchar
	tokStart      // .start
	tokLength     // .length
)

type token struct {
	kind     tokKind
	pos      pos // start of token in the *outer*-most source file (for errors)
	intVal   uint64
	intWidth int // 0 = default, 1/2/4 for y/w/l suffix
	realVal  float64
	realPrec sizeSpec // carries precision for a REAL token when overridden
	str      string
	strSize  int    // for STRING: (N) size override, 0 = none
	charLit  uint32 // for CHAR_LITERAL: packed FOURCC, MSB-first
	precSpec sizeSpec
}

type pos struct {
	filename string
	line     int
	col      int
}

func (p pos) String() string { return fmt.Sprintf("%s:%d:%d", p.filename, p.line, p.col) }

type sizeSpec struct {
	sign, whole, fraction int
}

// --- lexer -------------------------------------------------------------------

type frame struct {
	src      []byte
	offset   int
	line     int
	col      int
	filename string
}

type lexer struct {
	stack   []*frame
	lookahead []token // pushed-back tokens (for peek/peekN)
}

func newLexer(filename string) (*lexer, error) {
	l := &lexer{}
	if err := l.pushFile(filename); err != nil {
		return nil, err
	}
	return l, nil
}

func (l *lexer) pushFile(filename string) error {
	src, err := os.ReadFile(filename)
	if err != nil {
		return fmt.Errorf("open %s: %w", filename, err)
	}
	l.stack = append(l.stack, &frame{src: src, filename: filename, line: 1, col: 1})
	return nil
}

// pushSystemFile resolves against $WF_DIR for `include <foo>` directives.
func (l *lexer) pushSystemFile(name string) error {
	dir := os.Getenv("WF_DIR")
	if dir == "" {
		return fmt.Errorf("include <%s>: WF_DIR not set", name)
	}
	return l.pushFile(filepath.Join(dir, name))
}

func (l *lexer) cur() *frame {
	if len(l.stack) == 0 {
		return nil
	}
	return l.stack[len(l.stack)-1]
}

// peek returns the k'th token ahead without consuming it. k=0 is the next
// token. Used by the parser to disambiguate chunk vs. state-push on LBRACE.
func (l *lexer) peek(k int) token {
	for len(l.lookahead) <= k {
		tok, err := l.scan()
		if err != nil {
			// Surface the error as a synthetic EOF — the parser will report the real error.
			fmt.Fprintln(os.Stderr, "lex error:", err)
			l.lookahead = append(l.lookahead, token{kind: tokEOF})
			return l.lookahead[len(l.lookahead)-1]
		}
		l.lookahead = append(l.lookahead, tok)
	}
	return l.lookahead[k]
}

func (l *lexer) next() (token, error) {
	if len(l.lookahead) > 0 {
		tok := l.lookahead[0]
		l.lookahead = l.lookahead[1:]
		return tok, nil
	}
	return l.scan()
}

// --- raw byte ops -------------------------------------------------------------

func (l *lexer) peekByte() (byte, bool) {
	for {
		f := l.cur()
		if f == nil {
			return 0, false
		}
		if f.offset < len(f.src) {
			return f.src[f.offset], true
		}
		// End of current file — pop include stack.
		if len(l.stack) <= 1 {
			return 0, false
		}
		l.stack = l.stack[:len(l.stack)-1]
	}
}

func (l *lexer) readByte() (byte, bool) {
	b, ok := l.peekByte()
	if !ok {
		return 0, false
	}
	f := l.cur()
	f.offset++
	if b == '\n' {
		f.line++
		f.col = 1
	} else {
		f.col++
	}
	return b, true
}

func (l *lexer) currentPos() pos {
	f := l.cur()
	if f == nil {
		return pos{line: 0, col: 0}
	}
	return pos{filename: f.filename, line: f.line, col: f.col}
}

// --- scan --------------------------------------------------------------------

func (l *lexer) scan() (token, error) {
	for {
		// Skip whitespace and comments.
		b, ok := l.peekByte()
		if !ok {
			return token{kind: tokEOF}, nil
		}
		if b == ' ' || b == '\t' || b == '\r' || b == '\n' {
			l.readByte()
			continue
		}
		if b == '/' {
			// "//" line comment
			f := l.cur()
			if f.offset+1 < len(f.src) && f.src[f.offset+1] == '/' {
				for {
					c, ok := l.readByte()
					if !ok || c == '\n' {
						break
					}
				}
				continue
			}
		}
		break
	}

	startPos := l.currentPos()
	f := l.cur()
	b, _ := l.peekByte()

	// `.5` is a real number with leading fractional part; `.timestamp` is a
	// directive. Disambiguate on the next byte.
	if b == '.' {
		if isDigit(peekAtOffset(f, 1)) {
			return l.scanNumber(startPos)
		}
		return l.scanDotKeyword(startPos)
	}

	// `include "f"` and `include <f>` — consume them inside the lexer, not as tokens.
	if b == 'i' && hasPrefix(f.src[f.offset:], "include") && !isIdentChar(peekAtOffset(f, 7)) {
		if err := l.handleInclude(); err != nil {
			return token{}, err
		}
		return l.scan()
	}

	// Single-character literal tokens that do not require lookahead to recognise.
	switch b {
	case '{':
		l.readByte()
		return token{kind: tokLBrace, pos: startPos}, nil
	case '}':
		l.readByte()
		return token{kind: tokRBrace, pos: startPos}, nil
	case '(':
		l.readByte()
		return token{kind: tokLParen, pos: startPos}, nil
	case ')':
		l.readByte()
		return token{kind: tokRParen, pos: startPos}, nil
	case '[':
		l.readByte()
		return token{kind: tokLBrack, pos: startPos}, nil
	case ']':
		l.readByte()
		return token{kind: tokRBrack, pos: startPos}, nil
	case ',':
		l.readByte()
		return token{kind: tokComma, pos: startPos}, nil
	case '+':
		l.readByte()
		return token{kind: tokPlus, pos: startPos}, nil
	case 'Y':
		l.readByte()
		return token{kind: tokSizeY, pos: startPos}, nil
	case 'W':
		l.readByte()
		return token{kind: tokSizeW, pos: startPos}, nil
	case 'L':
		l.readByte()
		return token{kind: tokSizeL, pos: startPos}, nil
	case ':':
		l.readByte()
		b2, ok := l.peekByte()
		if ok && b2 == ':' {
			l.readByte()
			return token{kind: tokDoubleColon, pos: startPos}, nil
		}
		return token{}, fmt.Errorf("%s: expected '::' after ':'", startPos)
	case '\'':
		return l.scanCharLiteral(startPos)
	case '"':
		return l.scanString(startPos)
	}

	// '-' is tricky: could be the binary MINUS operator *or* the leading sign
	// of a negative literal. We peek one byte past the '-'; if it's a digit or
	// a '.', treat as a number; else emit tokMinus.
	if b == '-' {
		next := peekAtOffset(f, 1)
		if isDigit(next) || next == '.' {
			return l.scanNumber(startPos)
		}
		l.readByte()
		return token{kind: tokMinus, pos: startPos}, nil
	}

	// '$' prefix → hex integer.
	if b == '$' {
		return l.scanHexInteger(startPos)
	}

	// Digit or leading dot (real/integer/precSpec).
	if isDigit(b) {
		return l.scanNumber(startPos)
	}

	l.readByte()
	return token{}, fmt.Errorf("%s: unexpected character %q", startPos, b)
}

// hasPrefix / peekAtOffset / isDigit / isIdentChar: tiny helpers used by scan.
func hasPrefix(s []byte, p string) bool {
	if len(s) < len(p) {
		return false
	}
	for i := 0; i < len(p); i++ {
		if s[i] != p[i] {
			return false
		}
	}
	return true
}

func peekAtOffset(f *frame, k int) byte {
	if f.offset+k >= len(f.src) {
		return 0
	}
	return f.src[f.offset+k]
}

func isDigit(b byte) bool    { return b >= '0' && b <= '9' }
func isHexDigit(b byte) bool { return isDigit(b) || (b >= 'a' && b <= 'f') || (b >= 'A' && b <= 'F') }
func isIdentChar(b byte) bool {
	return (b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') || isDigit(b) || b == '_'
}

// --- include handling --------------------------------------------------------

func (l *lexer) handleInclude() error {
	// consume "include"
	for i := 0; i < 7; i++ {
		l.readByte()
	}
	// whitespace
	for {
		b, ok := l.peekByte()
		if !ok || (b != ' ' && b != '\t') {
			break
		}
		l.readByte()
	}
	b, ok := l.peekByte()
	if !ok {
		return fmt.Errorf("include: unexpected EOF")
	}
	system := false
	var closer byte
	switch b {
	case '"':
		closer = '"'
	case '<':
		closer = '>'
		system = true
	default:
		return fmt.Errorf("include: expected '\"' or '<', got %q", b)
	}
	l.readByte() // opening quote
	var name []byte
	for {
		c, ok := l.readByte()
		if !ok {
			return fmt.Errorf("include: unexpected EOF inside filename")
		}
		if c == closer {
			break
		}
		name = append(name, c)
	}
	if system {
		return l.pushSystemFile(string(name))
	}
	return l.pushFile(string(name))
}

// --- dot keywords ------------------------------------------------------------

func (l *lexer) scanDotKeyword(startPos pos) (token, error) {
	l.readByte() // consume '.'
	var ident []byte
	for {
		b, ok := l.peekByte()
		if !ok || !isIdentChar(b) {
			break
		}
		l.readByte()
		ident = append(ident, b)
	}
	switch string(ident) {
	case "timestamp":
		return token{kind: tokTimestamp, pos: startPos}, nil
	case "align":
		return token{kind: tokAlign, pos: startPos}, nil
	case "offsetof":
		return token{kind: tokOffsetof, pos: startPos}, nil
	case "sizeof":
		return token{kind: tokSizeof, pos: startPos}, nil
	case "fillchar":
		return token{kind: tokFillChar, pos: startPos}, nil
	case "start":
		return token{kind: tokStart, pos: startPos}, nil
	case "length":
		return token{kind: tokLength, pos: startPos}, nil
	case "precision":
		return token{kind: tokPrecision, pos: startPos}, nil
	}
	return token{}, fmt.Errorf("%s: unknown directive .%s", startPos, ident)
}

// --- char literal ------------------------------------------------------------

func (l *lexer) scanCharLiteral(startPos pos) (token, error) {
	l.readByte() // opening '
	var chars []byte
	for {
		b, ok := l.peekByte()
		if !ok {
			return token{}, fmt.Errorf("%s: unterminated char literal", startPos)
		}
		if b == '\'' {
			l.readByte()
			break
		}
		if len(chars) >= 4 {
			return token{}, fmt.Errorf("%s: char literal too long", startPos)
		}
		l.readByte()
		chars = append(chars, b)
	}
	if len(chars) == 0 {
		return token{}, fmt.Errorf("%s: empty char literal", startPos)
	}
	// Pack MSB-first into uint32, right-pad with NUL.
	var v uint32
	for i := 0; i < 4; i++ {
		var c byte
		if i < len(chars) {
			c = chars[i]
		}
		v |= uint32(c) << (24 - 8*i)
	}
	return token{kind: tokCharLit, pos: startPos, charLit: v}, nil
}

// --- strings -----------------------------------------------------------------

func (l *lexer) scanString(startPos pos) (token, error) {
	l.readByte() // opening "
	var body []byte
	terminated := false
	for {
		b, ok := l.peekByte()
		if !ok {
			return token{}, fmt.Errorf("%s: unterminated string", startPos)
		}
		if b == '\n' {
			// Unterminated on this line; match original lexer's permissive behavior.
			break
		}
		if b == '"' {
			l.readByte()
			terminated = true
			break
		}
		if b == '\\' {
			l.readByte()
			c, ok := l.readByte()
			if !ok {
				return token{}, fmt.Errorf("%s: unterminated escape", startPos)
			}
			// Preserve escape sequences literally; the writer's
			// translateEscapeCodes runs at out_string time, matching
			// iffwrite's behavior.
			body = append(body, '\\', c)
			continue
		}
		l.readByte()
		body = append(body, b)
	}
	_ = terminated // (warn path lives in the writer layer)

	// Optional (N) size override immediately after the closing quote.
	sizeOverride := 0
	if b, ok := l.peekByte(); ok && b == '(' {
		// Only consume as size override if what follows is all digits up to ')'.
		f := l.cur()
		save := f.offset
		saveLine, saveCol := f.line, f.col
		f.offset++ // skip '('
		f.col++
		var num []byte
		for {
			c, ok := l.peekByte()
			if !ok || c == ')' {
				break
			}
			if !isDigit(c) {
				num = nil
				break
			}
			l.readByte()
			num = append(num, c)
		}
		if c, ok := l.peekByte(); ok && c == ')' && num != nil {
			l.readByte() // ')'
			v, _ := strconv.Atoi(string(num))
			sizeOverride = v
		} else {
			// Roll back; the '(' belongs to something else.
			f.offset = save
			f.line = saveLine
			f.col = saveCol
		}
	}
	return token{kind: tokString, pos: startPos, str: string(body), strSize: sizeOverride}, nil
}

// --- numbers -----------------------------------------------------------------

func (l *lexer) scanHexInteger(startPos pos) (token, error) {
	l.readByte() // consume '$'
	var digits []byte
	for {
		b, ok := l.peekByte()
		if !ok || !isHexDigit(b) {
			break
		}
		l.readByte()
		digits = append(digits, b)
	}
	if len(digits) == 0 {
		return token{}, fmt.Errorf("%s: empty hex literal", startPos)
	}
	v, err := strconv.ParseUint(string(digits), 16, 64)
	if err != nil {
		return token{}, fmt.Errorf("%s: bad hex literal: %w", startPos, err)
	}
	width := l.scanWidthSuffix()
	return token{kind: tokInteger, pos: startPos, intVal: v, intWidth: width}, nil
}

func (l *lexer) scanWidthSuffix() int {
	b, ok := l.peekByte()
	if !ok {
		return 0
	}
	switch b {
	case 'y':
		l.readByte()
		return 1
	case 'w':
		l.readByte()
		return 2
	case 'l':
		l.readByte()
		return 4
	}
	return 0
}

// scanNumber handles decimal integers, reals, optional size suffix on integers,
// optional (sign.whole.fraction) precision override on reals, and the bare
// `N.N.N` triple (tokPrecSpec, used as the argument of .precision(...)).
func (l *lexer) scanNumber(startPos pos) (token, error) {
	// Snapshot frame state so we can roll back if the parse turns out to be a
	// precision triple or a real instead of an integer.
	var raw []byte
	negative := false
	if b, _ := l.peekByte(); b == '-' {
		l.readByte()
		negative = true
		raw = append(raw, '-')
	}

	// Leading digits (may be empty for ".5").
	for {
		b, ok := l.peekByte()
		if !ok || !isDigit(b) {
			break
		}
		l.readByte()
		raw = append(raw, b)
	}

	// Possibility 1: bare `N.N.N` precision triple (only when there was no
	// leading sign and exactly two dots follow).
	//
	// Speculatively consume `.digits`; if a second `.` appears, continue into
	// the triple form. Otherwise keep what we've consumed and fall through to
	// the real-number path.
	consumedFraction := false
	if !negative {
		if b, _ := l.peekByte(); b == '.' {
			l.readByte()
			raw = append(raw, '.')
			sawDigit := false
			for {
				b, ok := l.peekByte()
				if !ok || !isDigit(b) {
					break
				}
				l.readByte()
				sawDigit = true
				raw = append(raw, b)
			}
			if sawDigit {
				if b, _ := l.peekByte(); b == '.' {
					// Second dot → precision triple.
					l.readByte()
					raw = append(raw, '.')
					for {
						b, ok := l.peekByte()
						if !ok || !isDigit(b) {
							break
						}
						l.readByte()
						raw = append(raw, b)
					}
					return buildPrecSpec(startPos, string(raw))
				}
			}
			consumedFraction = true
		}
	}

	// Possibility 2: integer (no `.`).
	// Possibility 3: real starting with `.N` (e.g. `.5`).
	if !consumedFraction {
		if b, _ := l.peekByte(); b == '.' {
			l.readByte()
			raw = append(raw, '.')
			for {
				b, ok := l.peekByte()
				if !ok || !isDigit(b) {
					break
				}
				l.readByte()
				raw = append(raw, b)
			}
		}
	}

	// Optional exponent (e.g. "3e6") — matches the original regex.
	if b, _ := l.peekByte(); b == 'e' || b == 'E' {
		l.readByte()
		raw = append(raw, 'e')
		if b, _ := l.peekByte(); b == '+' || b == '-' {
			l.readByte()
			raw = append(raw, b)
		}
		for {
			b, ok := l.peekByte()
			if !ok || !isDigit(b) {
				break
			}
			l.readByte()
			raw = append(raw, b)
		}
	}

	isReal := false
	for _, c := range raw {
		if c == '.' || c == 'e' || c == 'E' {
			isReal = true
			break
		}
	}

	// An integer followed directly by `(N.N.N)` is actually a real with an
	// integer mantissa and an explicit precision override — this is how
	// `3(1.15.16)` is tokenized by the C++ flex regex, which is a single
	// pattern that handles both "real with optional precision" *and* "bare
	// integer with optional precision" under the same rule.
	if !isReal {
		if b, _ := l.peekByte(); b == '(' {
			// Speculatively attempt to parse a triple; on success, reclassify
			// as a real and fall through into the real path below.
			f := l.cur()
			save := f.offset
			saveLine, saveCol := f.line, f.col
			l.readByte() // '('
			triple, ok := l.tryScanPrecTriple()
			if ok {
				if c, _ := l.peekByte(); c == ')' {
					l.readByte()
					v, err := strconv.ParseFloat(string(raw), 64)
					if err != nil {
						return token{}, fmt.Errorf("%s: bad integer-as-real %q: %w", startPos, string(raw), err)
					}
					return token{kind: tokReal, pos: startPos, realVal: v, realPrec: triple}, nil
				}
			}
			// Rollback — the '(' belongs to something else (or triple was malformed).
			f.offset = save
			f.line = saveLine
			f.col = saveCol
		}
	}

	if isReal {
		v, err := strconv.ParseFloat(string(raw), 64)
		if err != nil {
			return token{}, fmt.Errorf("%s: bad real %q: %w", startPos, string(raw), err)
		}
		// Optional explicit precision override: `(sign.whole.fraction)`.
		var precOverride *sizeSpec
		if b, _ := l.peekByte(); b == '(' {
			// Speculatively parse the triple.
			f := l.cur()
			save := f.offset
			saveLine, saveCol := f.line, f.col
			l.readByte()
			triple, ok := l.tryScanPrecTriple()
			if ok {
				if c, ok := l.peekByte(); ok && c == ')' {
					l.readByte()
					precOverride = &triple
				} else {
					f.offset = save
					f.line = saveLine
					f.col = saveCol
				}
			} else {
				f.offset = save
				f.line = saveLine
				f.col = saveCol
			}
		}
		tok := token{kind: tokReal, pos: startPos, realVal: v}
		if precOverride != nil {
			tok.realPrec = *precOverride
		}
		return tok, nil
	}

	// Integer — optional width suffix.
	width := l.scanWidthSuffix()
	v, err := strconv.ParseInt(string(raw), 10, 64)
	if err != nil {
		return token{}, fmt.Errorf("%s: bad integer %q: %w", startPos, string(raw), err)
	}
	return token{kind: tokInteger, pos: startPos, intVal: uint64(v), intWidth: width}, nil
}

// tryScanPrecTriple consumes a `N.N.N` sequence if possible. Returns
// (triple, true) on success; the caller is responsible for rolling back on
// failure.
func (l *lexer) tryScanPrecTriple() (sizeSpec, bool) {
	readNum := func() (int, bool) {
		var digits []byte
		for {
			b, ok := l.peekByte()
			if !ok || !isDigit(b) {
				break
			}
			l.readByte()
			digits = append(digits, b)
		}
		if len(digits) == 0 {
			return 0, false
		}
		n, err := strconv.Atoi(string(digits))
		if err != nil {
			return 0, false
		}
		return n, true
	}
	a, ok := readNum()
	if !ok {
		return sizeSpec{}, false
	}
	if b, _ := l.peekByte(); b != '.' {
		return sizeSpec{}, false
	}
	l.readByte()
	bNum, ok := readNum()
	if !ok {
		return sizeSpec{}, false
	}
	if b, _ := l.peekByte(); b != '.' {
		return sizeSpec{}, false
	}
	l.readByte()
	c, ok := readNum()
	if !ok {
		return sizeSpec{}, false
	}
	if a < 0 || a > 1 {
		a = 1
	}
	return sizeSpec{sign: a, whole: bNum, fraction: c}, true
}

func buildPrecSpec(startPos pos, raw string) (token, error) {
	var a, b, c int
	_, err := fmt.Sscanf(raw, "%d.%d.%d", &a, &b, &c)
	if err != nil {
		return token{}, fmt.Errorf("%s: bad precision spec %q: %w", startPos, raw, err)
	}
	if a < 0 || a > 1 {
		a = 1
	}
	return token{kind: tokPrecSpec, pos: startPos, precSpec: sizeSpec{sign: a, whole: b, fraction: c}}, nil
}
