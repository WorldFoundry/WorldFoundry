//! IFF binary + text writer.
//!
//! Mirrors `wfsource/source/iffwrite/{binary,_iffwr,fixed,text}.cc` closely
//! enough to produce byte-identical output for both `IffWriterBinary` and
//! `IffWriterText` modes. A single `Writer` struct with a mode enum keeps
//! the binary and text implementations of each primitive next to each other
//! — same approach as `wftools/iffcomp-go/writer.go`.
//!
//! Key invariants learned the hard way in the C++ → Go port:
//!
//! 1. **LP64 int32 slot is 4 bytes**, not `sizeof(long)` (which is 8 on LP64
//!    Linux). Hard-code `4` everywhere in the binary writer.
//! 2. **Fixed-point negative-value cast** goes through `i64` first to get
//!    modular wrap, matching the gcc x86-64 output the oracle was produced
//!    with. `(val * scale) as i64 as u32`.
//! 3. **`IffWriterText` off-by-one indent**: the C++ `_IffWriter` ctor pushes
//!    a sentinel onto `chunkSize`, so every indent formula in `text.cc` uses
//!    `chunkSize.size()` which is `1 + depth`. We bake that off-by-one into
//!    `text_emit_indent`. Chunk `{` uses depth-before-push, `}` uses
//!    depth-after-pop, `out_file` wrap uses depth+1.
//! 4. **`%#.16g` float format**: C++ `setprecision(16) + showpoint` output
//!    needs a custom formatter because Rust's `format!` has no `alt .Ng`
//!    equivalent. See [`format_g_alt`] below.

use crate::error::{IffError, Result};
use crate::lexer::SizeSpec;
use std::collections::HashMap;
use std::fmt::Write as _;
use std::fs;
use std::path::Path;

/// Output format. Matches the C++ `-binary`/`-ascii` CLI switch.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum Mode {
    #[default]
    Binary,
    Text,
}

/// One open chunk on the nesting stack. Binary mode also tracks the
/// offset of the size field so `exit_chunk` can patch it.
struct ChunkFrame {
    #[allow(dead_code)] // not needed in text mode; kept for symmetry
    id: u32,
    size_field_pos: usize,
    path_key: String,
}

/// Symbol-table entry for a *closed* chunk. `pos` is the absolute offset of
/// the payload start (i.e., the byte after the size field); `size` is the
/// payload byte count (excluding the 8-byte header).
#[derive(Debug, Clone, Copy)]
struct ChunkSym {
    pos: usize,
    size: usize,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum BackpatchKind {
    Sizeof,
    Offsetof,
}

struct Backpatch {
    kind: BackpatchKind,
    path: String,
    addend: i32,
    write_pos: usize,
}

pub struct Writer {
    mode: Mode,

    // shared state
    stack: Vec<ChunkFrame>,
    path_ids: Vec<u32>,
    symbols: HashMap<String, ChunkSym>,
    pending: Vec<Backpatch>,
    fill_char: u8,

    // binary state
    buf: Vec<u8>,
    pos: usize,

    // text state
    text_buf: String,
    text_on_line: usize,
}

impl Writer {
    pub fn new(mode: Mode) -> Self {
        Writer {
            mode,
            stack: Vec::new(),
            path_ids: Vec::new(),
            symbols: HashMap::new(),
            pending: Vec::new(),
            fill_char: 0,
            buf: Vec::with_capacity(4096),
            pos: 0,
            text_buf: String::new(),
            text_on_line: 0,
        }
    }

    pub fn mode(&self) -> Mode {
        self.mode
    }

    pub fn bytes(&self) -> &[u8] {
        match self.mode {
            Mode::Binary => &self.buf,
            Mode::Text => self.text_buf.as_bytes(),
        }
    }

    // --- low-level buffer ops (binary only) ---------------------------------

    fn grow(&mut self, n: usize) {
        if n > self.buf.len() {
            self.buf.resize(n, 0);
        }
    }

    fn write_at(&mut self, p: &[u8], off: usize) {
        self.grow(off + p.len());
        self.buf[off..off + p.len()].copy_from_slice(p);
    }

    fn write_raw(&mut self, p: &[u8]) {
        self.grow(self.pos + p.len());
        self.buf[self.pos..self.pos + p.len()].copy_from_slice(p);
        self.pos += p.len();
    }

    // --- primitive emitters -------------------------------------------------

    pub fn out_int8(&mut self, v: u8) {
        match self.mode {
            Mode::Binary => self.write_raw(&[v]),
            Mode::Text => {
                let _ = write!(self.text_buf, "{}y ", v as i8);
            }
        }
    }

    pub fn out_int16(&mut self, v: u16) {
        match self.mode {
            Mode::Binary => self.write_raw(&v.to_le_bytes()),
            Mode::Text => {
                let _ = write!(self.text_buf, "{}w ", v as i16);
            }
        }
    }

    pub fn out_int32(&mut self, v: u32) {
        match self.mode {
            Mode::Binary => self.write_raw(&v.to_le_bytes()),
            Mode::Text => {
                let _ = write!(self.text_buf, "{}l ", v as i32);
            }
        }
    }

    /// Pad with zero bytes until `pos % n == 0`. No-op in text mode (matches
    /// `IffWriterText::align`). Used between sibling chunks and for `out_id`.
    pub fn align(&mut self, n: usize) {
        if self.mode == Mode::Text || n <= 1 {
            return;
        }
        let rem = self.pos % n;
        if rem == 0 {
            return;
        }
        let pad = n - rem;
        for _ in 0..pad {
            self.write_raw(&[0]);
        }
    }

    /// User-facing `.align(N)` — pads with `fill_char`, not zeros. Matches
    /// `IffWriterBinary::alignFunction`.
    pub fn align_function(&mut self, n: usize) {
        if self.mode == Mode::Text || n <= 1 {
            return;
        }
        let rem = self.pos % n;
        if rem == 0 {
            return;
        }
        let pad = n - rem;
        for _ in 0..pad {
            self.write_raw(&[self.fill_char]);
        }
    }

    pub fn set_fill_char(&mut self, b: u8) {
        self.fill_char = b;
    }

    /// Write a FOURCC with 4-byte alignment in front. The value is stored
    /// MSB-first in a `u32`, so writing it big-endian gives source-order
    /// bytes. Matches the C++ `IffWriterBinary::out_id` layout, including
    /// the interior `align(4)` call — which is a footgun for FOURCC-as-item
    /// usage at non-aligned positions on the C++ side (the chunkSize counter
    /// doesn't track the padding and the exit_chunk assertion fires). Go and
    /// Rust don't have that accounting bug because they compute size from
    /// `pos - payload_start` at exit time, but they preserve the byte layout
    /// for consistency with the C++ oracle.
    pub fn out_id(&mut self, id: u32) {
        if self.mode == Mode::Text {
            let _ = write!(self.text_buf, "'{}' ", id_name(id));
            return;
        }
        self.align(4);
        self.write_raw(&id.to_be_bytes());
    }

    // --- chunks -------------------------------------------------------------

    pub fn enter_chunk(&mut self, id: u32) {
        let depth = self.stack.len();
        self.path_ids.push(id);
        let path_key = build_path_key(&self.path_ids);

        if self.mode == Mode::Text {
            // `\n<tabs>{ 'ID' ` — no leading tabs at the outermost level.
            self.text_buf.push('\n');
            for _ in 0..depth {
                self.text_buf.push('\t');
            }
            let _ = write!(self.text_buf, "{{ '{}' ", id_name(id));
            self.stack.push(ChunkFrame {
                id,
                size_field_pos: 0,
                path_key,
            });
            return;
        }

        self.align(4);
        // ID (4 bytes, big-endian = source order).
        self.write_raw(&id.to_be_bytes());
        // Size placeholder (4 bytes, LE, 0xFFFFFFFF).
        let size_field_pos = self.pos;
        self.write_raw(&0xFFFF_FFFFu32.to_le_bytes());

        self.stack.push(ChunkFrame {
            id,
            size_field_pos,
            path_key,
        });
    }

    pub fn exit_chunk(&mut self) {
        let top = self.stack.pop().expect("exit_chunk with empty stack");
        self.path_ids.pop();
        let depth = self.stack.len();

        if self.mode == Mode::Text {
            self.text_buf.push('\n');
            for _ in 0..depth {
                self.text_buf.push('\t');
            }
            self.text_buf.push('}');
            return;
        }

        let payload_start = top.size_field_pos + 4;
        let size = self.pos - payload_start;

        // Patch the 4-byte LE size field.
        self.write_at(&(size as u32).to_le_bytes(), top.size_field_pos);

        // Symbol table: path_key → (payload start, payload size).
        self.symbols.insert(
            top.path_key,
            ChunkSym {
                pos: payload_start,
                size,
            },
        );

        // Pad to the sibling boundary. `align` here writes zeros, not fill_char.
        self.align(4);
    }

    // --- sizeof / offsetof --------------------------------------------------

    fn find_symbol(&self, path: &str) -> Option<ChunkSym> {
        self.symbols.get(path).copied()
    }

    pub fn emit_sizeof(&mut self, path: &str) {
        if let Some(sym) = self.find_symbol(path) {
            self.out_int32(sym.size as u32);
            return;
        }
        if self.mode == Mode::Text {
            self.out_int32(0);
            return;
        }
        self.pending.push(Backpatch {
            kind: BackpatchKind::Sizeof,
            path: path.to_string(),
            addend: 0,
            write_pos: self.pos,
        });
        self.out_int32(0);
    }

    pub fn emit_offsetof(&mut self, path: &str, addend: i32) {
        // The C++ grammar uses *different* formulas for the immediate and
        // deferred resolution paths, differing by 4 bytes. Matching the
        // oracle requires reproducing the quirk:
        //
        //   immediate: cs->GetPos()     = size_field_pos = ID_pos + 4
        //   deferred : cs->GetPos() - 4 = ID_pos
        //
        // Our `sym.pos` is payload_start = ID_pos + 8, so:
        //   immediate formula: sym.pos - 4
        //   deferred formula : sym.pos - 8  (see `resolve_backpatches`)
        if let Some(sym) = self.find_symbol(path) {
            self.out_int32((sym.pos as i32 - 4 + addend) as u32);
            return;
        }
        if self.mode == Mode::Text {
            self.out_int32(0);
            return;
        }
        self.pending.push(Backpatch {
            kind: BackpatchKind::Offsetof,
            path: path.to_string(),
            addend,
            write_pos: self.pos,
        });
        self.out_int32(0xFFFF_FFFF);
    }

    pub fn resolve_backpatches(&mut self) -> Result<()> {
        if self.mode == Mode::Text {
            return Ok(());
        }
        let mut missing: Vec<String> = Vec::new();
        // Drain into a local so we're not holding a borrow of self.pending
        // while we also borrow self.buf for write_at.
        let pending = std::mem::take(&mut self.pending);
        for bp in pending {
            match self.symbols.get(&bp.path).copied() {
                Some(sym) => {
                    let val: i32 = match bp.kind {
                        BackpatchKind::Sizeof => sym.size as i32,
                        BackpatchKind::Offsetof => sym.pos as i32 - 8 + bp.addend,
                    };
                    self.write_at(&val.to_le_bytes(), bp.write_pos);
                }
                None => missing.push(bp.path),
            }
        }
        if !missing.is_empty() {
            return Err(IffError::UnresolvedSymbols(missing));
        }
        Ok(())
    }

    // --- strings ------------------------------------------------------------

    pub fn out_string(&mut self, s: &str) {
        if self.mode == Mode::Text {
            self.text_emit_string(s);
            return;
        }
        let translated = translate_escape_codes(s);
        self.write_raw(translated.as_bytes());
        self.write_raw(&[0]); // NUL
    }

    /// Adjacent-string concatenation. In binary mode, seek back 1 byte over
    /// the previous NUL and re-emit. In text mode, emit as a second quoted
    /// literal (matching `IffWriterText::out_string_continue`).
    pub fn out_string_continue(&mut self, s: &str) {
        if self.mode == Mode::Text {
            self.out_string(s);
            return;
        }
        if self.pos > 0 {
            self.pos -= 1;
        }
        self.out_string(s);
    }

    /// `"..."(N)`: write the string, then pad with zero bytes to exactly N.
    /// Returns overrun > 0 if the string plus NUL already exceeds N.
    ///
    /// In text mode, the size override is silently dropped (matches
    /// `IffWriterText`, which has no padding notion).
    pub fn out_string_pad(&mut self, s: &str, total_bytes: usize) -> usize {
        if self.mode == Mode::Text {
            self.out_string(s);
            return 0;
        }
        self.out_string(s);
        let required = translate_escape_codes(s).len() + 1; // NUL
        if required >= total_bytes {
            return required - total_bytes;
        }
        for _ in 0..(total_bytes - required) {
            self.write_raw(&[0]);
        }
        0
    }

    // --- timestamp, file ----------------------------------------------------

    pub fn out_timestamp(&mut self, unix_seconds: i64) {
        self.out_int32(unix_seconds as u32);
    }

    /// Emit raw bytes of an external file, optionally sliced to `[start,
    /// start+length)`. A `length` of `u64::MAX` means "no limit".
    pub fn out_file(&mut self, path: &Path, start: u64, length: u64) -> Result<()> {
        let data = fs::read(path).map_err(|e| IffError::Io {
            path: path.to_path_buf(),
            source: e,
        })?;
        if start as usize >= data.len() {
            return Err(IffError::Io {
                path: path.to_path_buf(),
                source: std::io::Error::new(
                    std::io::ErrorKind::InvalidInput,
                    format!("start {} past EOF ({} bytes)", start, data.len()),
                ),
            });
        }
        let mut end = data.len();
        if length != u64::MAX {
            let candidate = (start + length) as usize;
            if candidate < end {
                end = candidate;
            }
        }
        let slice = &data[start as usize..end];

        if self.mode == Mode::Text {
            for &b in slice {
                let _ = write!(self.text_buf, "{}y ", b as i8);
                self.text_on_line += 1;
                if self.text_on_line == 100 {
                    self.text_emit_comment();
                    self.text_on_line = 0;
                }
            }
            // Trailing comment marker regardless of boundary (matches
            // IffWriterText::out_mem's `out_comment(Comment(""))` at the end).
            self.text_emit_comment();
            self.text_on_line = 0;
            return Ok(());
        }

        self.write_raw(slice);
        Ok(())
    }

    // --- fixed-point --------------------------------------------------------

    /// `val × 2^fraction` truncated to an unsigned integer of width
    /// determined by total bit count. Goes through `i64` so negative values
    /// wrap like the gcc x86-64 output.
    pub fn out_fixed(&mut self, val: f64, prec: SizeSpec) {
        if self.mode == Mode::Text {
            let s = format_g_alt(val, 16);
            let _ = write!(
                self.text_buf,
                "{}({}.{}.{}) ",
                s, prec.sign, prec.whole, prec.fraction
            );
            return;
        }
        let bits = prec.sign + prec.whole + prec.fraction;
        let scale = 1u64 << (prec.fraction as u32);
        let scaled = (val * scale as f64) as i64;
        if bits > 16 {
            self.out_int32(scaled as u32);
        } else if bits > 8 {
            self.out_int16(scaled as u16);
        } else {
            self.out_int8(scaled as u8);
        }
    }

    // --- text-mode helpers --------------------------------------------------

    /// Format a Go-string as a C++-iostream-style quoted literal: `"body"`
    /// with `\n` producing a `\n"` + newline + `"` break, `\\` and `"`
    /// escaped with `\`, other bytes passed through. Matches
    /// `IffWriterText::out_string`.
    fn text_emit_string(&mut self, s: &str) {
        self.text_buf.push('"');
        for b in s.bytes() {
            match b {
                b'\n' => {
                    self.text_buf.push_str("\\n\"");
                    self.text_buf.push('\n');
                    self.text_buf.push('"');
                }
                b'\\' => self.text_buf.push_str("\\\\"),
                b'"' => self.text_buf.push_str("\\\""),
                _ => self.text_buf.push(b as char),
            }
        }
        self.text_buf.push_str("\" ");
    }

    /// Write ` //\n<tabs>` — the wrap marker used by `out_file` every
    /// 100 bytes. Uses `depth + 1` tabs to bake in the C++ off-by-one.
    fn text_emit_comment(&mut self) {
        self.text_buf.push_str(" //\n");
        let tabs = self.stack.len() + 1;
        for _ in 0..tabs {
            self.text_buf.push('\t');
        }
    }
}

// --- free functions ----------------------------------------------------------

/// Translate `\n \t \\ \" \DDD` escape sequences. Runs at write time, not at
/// lex time, matching `iffwrite/binary.cc::translate_escape_codes`.
fn translate_escape_codes(s: &str) -> String {
    let b = s.as_bytes();
    let mut out = Vec::with_capacity(b.len());
    let mut i = 0;
    while i < b.len() {
        if b[i] != b'\\' || i + 1 >= b.len() {
            out.push(b[i]);
            i += 1;
            continue;
        }
        let c = b[i + 1];
        match c {
            b't' => {
                out.push(b'\t');
                i += 2;
            }
            b'n' => {
                out.push(b'\n');
                i += 2;
            }
            b'\\' => {
                out.push(b'\\');
                i += 2;
            }
            b'"' => {
                out.push(b'"');
                i += 2;
            }
            b'0'..=b'9' => {
                // Decimal run, wraps at 256 (matches atoi() & 0xFF in C).
                let mut j = i + 1;
                while j < b.len() && b[j].is_ascii_digit() {
                    j += 1;
                }
                let mut n: i32 = 0;
                for k in (i + 1)..j {
                    n = n * 10 + (b[k] - b'0') as i32;
                }
                out.push((n & 0xFF) as u8);
                i = j;
            }
            _ => {
                // Unknown escape — emit literally.
                out.push(b[i]);
                out.push(c);
                i += 2;
            }
        }
    }
    String::from_utf8_lossy(&out).into_owned()
}

/// Decode a FOURCC packed MSB-first into a `u32`, trimming trailing NULs.
pub fn id_name(id: u32) -> String {
    let bytes = [
        (id >> 24) as u8,
        (id >> 16) as u8,
        (id >> 8) as u8,
        id as u8,
    ];
    let mut n = 4;
    while n > 0 && bytes[n - 1] == 0 {
        n -= 1;
    }
    String::from_utf8_lossy(&bytes[..n]).into_owned()
}

fn build_path_key(ids: &[u32]) -> String {
    let mut s = String::new();
    for &id in ids {
        s.push_str("::'");
        s.push_str(&id_name(id));
        s.push('\'');
    }
    s
}

/// Approximate printf's `%#.Ng`: N significant digits, decimal point always
/// shown, trailing zeros preserved. This matches C++ iostreams'
/// `setprecision(N) + showpoint` well enough for the test fixtures — verified
/// against the C++ `IffWriterText` output for `test.iff.txt`.
///
/// Algorithm:
/// 1. Choose fixed vs. exponential form the same way printf's `%g` does:
///    use exponential if the exponent is < -4 or >= N.
/// 2. For fixed form, emit `N` significant digits total: `N - (exponent+1)`
///    digits after the decimal point, clamped to 0.
/// 3. Always emit the decimal point (the `#` alt flag). Trailing zeros are
///    kept as-is (no stripping, matching `#`).
fn format_g_alt(val: f64, precision: usize) -> String {
    if val == 0.0 {
        // "0." would be weird; C++ writes "0.000000000000000" (precision-1 zeros).
        let mut s = String::from("0.");
        for _ in 0..(precision - 1) {
            s.push('0');
        }
        return s;
    }

    // Exponent of the leading digit when expressed in scientific form.
    let abs = val.abs();
    let exp10 = abs.log10().floor() as i32;

    // printf picks exponential form when exp < -4 or exp >= precision.
    let use_exp = exp10 < -4 || exp10 >= precision as i32;

    if use_exp {
        // Fall back to Rust's exponential format and post-process to match
        // printf's `<mantissa>e<±>NN` layout. `format!("{:.*e}", precision-1, val)`
        // produces `3.14e0`; we want `3.14e+00` etc. This path isn't exercised
        // by the current test fixtures, so we keep it simple.
        let mantissa_digits = precision.saturating_sub(1);
        let s = format!("{:.*e}", mantissa_digits, val);
        // Normalize the exponent suffix.
        if let Some(idx) = s.find('e') {
            let (mantissa, exp) = s.split_at(idx);
            let exp = &exp[1..];
            let (sign, digits) = if let Some(rest) = exp.strip_prefix('-') {
                ('-', rest)
            } else if let Some(rest) = exp.strip_prefix('+') {
                ('+', rest)
            } else {
                ('+', exp)
            };
            let padded = if digits.len() < 2 {
                format!("0{}", digits)
            } else {
                digits.to_string()
            };
            return format!("{}e{}{}", mantissa, sign, padded);
        }
        return s;
    }

    // Fixed form: precision - (exp+1) digits after the decimal point.
    let after = (precision as i32 - (exp10 + 1)).max(0) as usize;
    format!("{:.*}", after, val)
}
