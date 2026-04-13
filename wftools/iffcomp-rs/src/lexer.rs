//! Hand-rolled tokenizer for the iffcomp DSL.
//!
//! Mirrors the rules in `wftools/iffcomp/lang.l` one-for-one, and the token
//! layout in `wftools/iffcomp-go/lexer.go`. The Rust version is smaller
//! mostly because tokens are an enum with per-variant data rather than a
//! struct with always-present fields.

use crate::error::{IffError, Pos, Result};
use std::collections::VecDeque;
use std::fs;
use std::path::{Path, PathBuf};

/// Source-position span, used for speculative rollback inside
/// [`Lexer::scan_number`].
#[derive(Clone, Copy)]
struct LexPos {
    offset: usize,
    line: u32,
    col: u32,
}

/// Sign / whole / fraction bit counts. Matches `struct size_specifier` in
/// `wfsource/source/iffwrite/fixed.hp`. Total bit count determines the
/// output width for a fixed-point real.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct SizeSpec {
    pub sign: i32,
    pub whole: i32,
    pub fraction: i32,
}

/// A single DSL token. Each variant carries whatever payload the parser needs
/// to consume that token kind.
#[derive(Debug, Clone)]
pub enum Token {
    Eof,
    LBrace,
    RBrace,
    LParen,
    RParen,
    LBrack,
    RBrack,
    Comma,
    DoubleColon,
    Plus,
    Minus,
    SizeY,
    SizeW,
    SizeL,
    Timestamp,
    Align,
    Offsetof,
    Sizeof,
    FillChar,
    Start,
    Length,
    Precision,
    /// Integer literal. `width` is the explicit `[ywl]` size suffix (1/2/4)
    /// or 0 for "use current state default".
    Integer { val: u64, width: u8 },
    /// Real literal with an optional explicit `(S.W.F)` precision override.
    /// A `None` precision means "use current state default".
    Real { val: f64, precision: Option<SizeSpec> },
    /// String literal. `body` is the raw characters between quotes with
    /// escapes *unresolved* — the writer's `out_string` translates them at
    /// write time, matching `iffwrite/binary.cc::translate_escape_codes`.
    /// `size_override` is the optional trailing `(N)` byte-count pad.
    String { body: String, size_override: usize },
    /// FOURCC packed MSB-first into a u32, right-padded with NUL for 1–3
    /// character IDs.
    CharLit(u32),
    /// A bare `N.N.N` triple used as the argument of `.precision(...)`.
    PrecSpec(SizeSpec),
}

impl Token {
    pub fn name(&self) -> &'static str {
        match self {
            Token::Eof => "EOF",
            Token::LBrace => "'{'",
            Token::RBrace => "'}'",
            Token::LParen => "'('",
            Token::RParen => "')'",
            Token::LBrack => "'['",
            Token::RBrack => "']'",
            Token::Comma => "','",
            Token::DoubleColon => "'::'",
            Token::Plus => "'+'",
            Token::Minus => "'-'",
            Token::SizeY => "'Y'",
            Token::SizeW => "'W'",
            Token::SizeL => "'L'",
            Token::Timestamp => ".timestamp",
            Token::Align => ".align",
            Token::Offsetof => ".offsetof",
            Token::Sizeof => ".sizeof",
            Token::FillChar => ".fillchar",
            Token::Start => ".start",
            Token::Length => ".length",
            Token::Precision => ".precision",
            Token::Integer { .. } => "INTEGER",
            Token::Real { .. } => "REAL",
            Token::String { .. } => "STRING",
            Token::CharLit(_) => "CHAR_LITERAL",
            Token::PrecSpec(_) => "PRECISION_SPECIFIER",
        }
    }
}

#[derive(Debug, Clone)]
pub struct SpannedToken {
    pub tok: Token,
    pub pos: Pos,
}

/// One frame of the include-file stack.
struct Frame {
    src: Vec<u8>,
    offset: usize,
    line: u32,
    col: u32,
    filename: String,
}

pub struct Lexer {
    stack: Vec<Frame>,
    lookahead: VecDeque<SpannedToken>,
}

impl Lexer {
    /// Open the root input file and prime the scanner.
    pub fn open(path: impl AsRef<Path>) -> Result<Self> {
        let mut l = Lexer {
            stack: Vec::new(),
            lookahead: VecDeque::new(),
        };
        l.push_file(path.as_ref())?;
        Ok(l)
    }

    /// Push an `include "f"` file onto the frame stack. Absolute or
    /// working-directory-relative.
    pub fn push_include(&mut self, name: &str) -> Result<()> {
        self.push_file(Path::new(name))
    }

    /// Push an `include <f>` file onto the stack, resolving `f` relative to
    /// the `WF_DIR` environment variable. Matches the C++ tool's
    /// `strFlexLexer::push_system_include`.
    pub fn push_system_include(&mut self, name: &str) -> Result<()> {
        let dir = std::env::var("WF_DIR").map_err(|_| IffError::Lex {
            at: self.current_pos(),
            msg: format!("include <{}>: WF_DIR not set", name),
        })?;
        let full = PathBuf::from(dir).join(name);
        self.push_file(&full)
    }

    fn push_file(&mut self, path: &Path) -> Result<()> {
        let src = fs::read(path).map_err(|e| IffError::Io {
            path: path.to_path_buf(),
            source: e,
        })?;
        self.stack.push(Frame {
            src,
            offset: 0,
            line: 1,
            col: 1,
            filename: path.display().to_string(),
        });
        Ok(())
    }

    /// Pop a frame off the include stack. Returns `true` if another frame
    /// is available underneath, `false` if we've exhausted the root file.
    fn pop_include(&mut self) -> bool {
        if self.stack.len() <= 1 {
            return false;
        }
        self.stack.pop();
        true
    }

    /// Peek the k'th token ahead (0 = next). Used by the parser to
    /// disambiguate chunk-vs-state-push on `{`.
    pub fn peek(&mut self, k: usize) -> Result<Token> {
        while self.lookahead.len() <= k {
            let tok = self.scan()?;
            self.lookahead.push_back(tok);
        }
        Ok(self.lookahead[k].tok.clone())
    }

    /// Consume and return the next token.
    pub fn next(&mut self) -> Result<SpannedToken> {
        if let Some(tok) = self.lookahead.pop_front() {
            return Ok(tok);
        }
        self.scan()
    }

    // --- raw byte access ----------------------------------------------------

    fn cur(&self) -> Option<&Frame> {
        self.stack.last()
    }

    fn cur_mut(&mut self) -> Option<&mut Frame> {
        self.stack.last_mut()
    }

    fn current_pos(&self) -> Pos {
        match self.cur() {
            Some(f) => Pos {
                filename: f.filename.clone(),
                line: f.line,
                col: f.col,
            },
            None => Pos::default(),
        }
    }

    /// Return the next byte without consuming it, advancing through exhausted
    /// include frames as needed.
    fn peek_byte(&mut self) -> Option<u8> {
        loop {
            let f = self.cur()?;
            if f.offset < f.src.len() {
                return Some(f.src[f.offset]);
            }
            // Frame exhausted — try to pop to the parent.
            if !self.pop_include() {
                return None;
            }
        }
    }

    fn read_byte(&mut self) -> Option<u8> {
        let b = self.peek_byte()?;
        let f = self.cur_mut().unwrap();
        f.offset += 1;
        if b == b'\n' {
            f.line += 1;
            f.col = 1;
        } else {
            f.col += 1;
        }
        Some(b)
    }

    /// Peek `k` bytes ahead in the *current* frame (no cross-frame peeking).
    /// Returns 0 for out-of-range, which the caller treats as "no match".
    fn peek_at(&self, k: usize) -> u8 {
        match self.cur() {
            Some(f) if f.offset + k < f.src.len() => f.src[f.offset + k],
            _ => 0,
        }
    }

    fn save_pos(&self) -> LexPos {
        let f = self.cur().unwrap();
        LexPos {
            offset: f.offset,
            line: f.line,
            col: f.col,
        }
    }

    fn restore_pos(&mut self, p: LexPos) {
        let f = self.cur_mut().unwrap();
        f.offset = p.offset;
        f.line = p.line;
        f.col = p.col;
    }

    // --- main scanner -------------------------------------------------------

    fn scan(&mut self) -> Result<SpannedToken> {
        // Skip whitespace and `//` line comments.
        loop {
            let Some(b) = self.peek_byte() else {
                return Ok(SpannedToken {
                    tok: Token::Eof,
                    pos: self.current_pos(),
                });
            };
            if b == b' ' || b == b'\t' || b == b'\r' || b == b'\n' {
                self.read_byte();
                continue;
            }
            if b == b'/' && self.peek_at(1) == b'/' {
                while let Some(c) = self.read_byte() {
                    if c == b'\n' {
                        break;
                    }
                }
                continue;
            }
            break;
        }

        let start_pos = self.current_pos();
        let b = self.peek_byte().unwrap();

        // `.N` is a real, `.keyword` is a directive. Disambiguate on the next byte.
        if b == b'.' {
            if is_digit(self.peek_at(1)) {
                return self.scan_number(start_pos);
            }
            return self.scan_dot_keyword(start_pos);
        }

        // `include "..."` or `include <...>` — consumed inline, no token emitted.
        if b == b'i' && self.frame_has_prefix("include") && !is_ident_char(self.peek_at(7)) {
            self.handle_include()?;
            return self.scan();
        }

        // Single-character literal tokens.
        let simple = match b {
            b'{' => Some(Token::LBrace),
            b'}' => Some(Token::RBrace),
            b'(' => Some(Token::LParen),
            b')' => Some(Token::RParen),
            b'[' => Some(Token::LBrack),
            b']' => Some(Token::RBrack),
            b',' => Some(Token::Comma),
            b'+' => Some(Token::Plus),
            b'Y' => Some(Token::SizeY),
            b'W' => Some(Token::SizeW),
            b'L' => Some(Token::SizeL),
            _ => None,
        };
        if let Some(tok) = simple {
            self.read_byte();
            return Ok(SpannedToken { tok, pos: start_pos });
        }

        // '::'
        if b == b':' {
            self.read_byte();
            if self.peek_byte() == Some(b':') {
                self.read_byte();
                return Ok(SpannedToken {
                    tok: Token::DoubleColon,
                    pos: start_pos,
                });
            }
            return Err(IffError::Lex {
                at: start_pos,
                msg: "expected '::' after ':'".into(),
            });
        }

        // '\''
        if b == b'\'' {
            return self.scan_char_literal(start_pos);
        }

        // '"'
        if b == b'"' {
            return self.scan_string(start_pos);
        }

        // '-' — could be binary MINUS or leading sign of a negative literal.
        if b == b'-' {
            let next = self.peek_at(1);
            if is_digit(next) || next == b'.' {
                return self.scan_number(start_pos);
            }
            self.read_byte();
            return Ok(SpannedToken {
                tok: Token::Minus,
                pos: start_pos,
            });
        }

        // '$' — hex integer prefix.
        if b == b'$' {
            return self.scan_hex_integer(start_pos);
        }

        if is_digit(b) {
            return self.scan_number(start_pos);
        }

        self.read_byte();
        Err(IffError::Lex {
            at: start_pos,
            msg: format!("unexpected character {:?}", b as char),
        })
    }

    fn frame_has_prefix(&self, p: &str) -> bool {
        let Some(f) = self.cur() else { return false };
        f.src[f.offset..].starts_with(p.as_bytes())
    }

    // --- include handling ---------------------------------------------------

    fn handle_include(&mut self) -> Result<()> {
        // consume 'include'
        for _ in 0..7 {
            self.read_byte();
        }
        // whitespace
        while let Some(b) = self.peek_byte() {
            if b != b' ' && b != b'\t' {
                break;
            }
            self.read_byte();
        }
        let Some(b) = self.peek_byte() else {
            return Err(IffError::Lex {
                at: self.current_pos(),
                msg: "include: unexpected EOF".into(),
            });
        };
        let (system, closer) = match b {
            b'"' => (false, b'"'),
            b'<' => (true, b'>'),
            _ => {
                return Err(IffError::Lex {
                    at: self.current_pos(),
                    msg: format!("include: expected '\"' or '<', got {:?}", b as char),
                });
            }
        };
        self.read_byte(); // opening quote
        let mut name = Vec::new();
        loop {
            let Some(c) = self.read_byte() else {
                return Err(IffError::Lex {
                    at: self.current_pos(),
                    msg: "include: unexpected EOF inside filename".into(),
                });
            };
            if c == closer {
                break;
            }
            name.push(c);
        }
        let name = String::from_utf8_lossy(&name).into_owned();
        if system {
            self.push_system_include(&name)
        } else {
            self.push_include(&name)
        }
    }

    // --- dot keywords -------------------------------------------------------

    fn scan_dot_keyword(&mut self, start_pos: Pos) -> Result<SpannedToken> {
        self.read_byte(); // '.'
        let mut ident = Vec::new();
        while let Some(b) = self.peek_byte() {
            if !is_ident_char(b) {
                break;
            }
            self.read_byte();
            ident.push(b);
        }
        let name = String::from_utf8_lossy(&ident);
        let tok = match name.as_ref() {
            "timestamp" => Token::Timestamp,
            "align" => Token::Align,
            "offsetof" => Token::Offsetof,
            "sizeof" => Token::Sizeof,
            "fillchar" => Token::FillChar,
            "start" => Token::Start,
            "length" => Token::Length,
            "precision" => Token::Precision,
            _ => {
                return Err(IffError::Lex {
                    at: start_pos,
                    msg: format!("unknown directive .{}", name),
                })
            }
        };
        Ok(SpannedToken { tok, pos: start_pos })
    }

    // --- char literal -------------------------------------------------------

    fn scan_char_literal(&mut self, start_pos: Pos) -> Result<SpannedToken> {
        self.read_byte(); // opening '
        let mut chars: Vec<u8> = Vec::new();
        loop {
            let Some(b) = self.peek_byte() else {
                return Err(IffError::Lex {
                    at: start_pos,
                    msg: "unterminated char literal".into(),
                });
            };
            if b == b'\'' {
                self.read_byte();
                break;
            }
            if chars.len() >= 4 {
                return Err(IffError::Lex {
                    at: start_pos,
                    msg: "char literal too long".into(),
                });
            }
            self.read_byte();
            chars.push(b);
        }
        if chars.is_empty() {
            return Err(IffError::Lex {
                at: start_pos,
                msg: "empty char literal".into(),
            });
        }
        // Pack MSB-first, right-pad with NUL.
        let mut v: u32 = 0;
        for i in 0..4 {
            let c = if i < chars.len() { chars[i] } else { 0 };
            v |= (c as u32) << (24 - 8 * i);
        }
        Ok(SpannedToken {
            tok: Token::CharLit(v),
            pos: start_pos,
        })
    }

    // --- strings ------------------------------------------------------------

    fn scan_string(&mut self, start_pos: Pos) -> Result<SpannedToken> {
        self.read_byte(); // opening "
        let mut body: Vec<u8> = Vec::new();
        loop {
            let Some(b) = self.peek_byte() else {
                return Err(IffError::Lex {
                    at: start_pos,
                    msg: "unterminated string".into(),
                });
            };
            if b == b'\n' {
                // Permissive unterminated — matches the original lexer.
                break;
            }
            if b == b'"' {
                self.read_byte();
                break;
            }
            if b == b'\\' {
                self.read_byte();
                let Some(c) = self.read_byte() else {
                    return Err(IffError::Lex {
                        at: start_pos,
                        msg: "unterminated escape".into(),
                    });
                };
                // Preserve escape sequences literally; translation happens in
                // the writer at `out_string` time.
                body.push(b'\\');
                body.push(c);
                continue;
            }
            self.read_byte();
            body.push(b);
        }

        // Optional (N) size override immediately after the closing quote.
        let mut size_override = 0usize;
        if self.peek_byte() == Some(b'(') {
            let saved = self.save_pos();
            self.read_byte(); // '('
            let mut num = Vec::new();
            let mut ok = true;
            loop {
                let Some(c) = self.peek_byte() else {
                    ok = false;
                    break;
                };
                if c == b')' {
                    break;
                }
                if !is_digit(c) {
                    ok = false;
                    break;
                }
                self.read_byte();
                num.push(c);
            }
            if ok && self.peek_byte() == Some(b')') {
                self.read_byte();
                size_override = std::str::from_utf8(&num)
                    .unwrap_or("0")
                    .parse()
                    .unwrap_or(0);
            } else {
                self.restore_pos(saved);
            }
        }

        Ok(SpannedToken {
            tok: Token::String {
                body: String::from_utf8_lossy(&body).into_owned(),
                size_override,
            },
            pos: start_pos,
        })
    }

    // --- numbers ------------------------------------------------------------

    fn scan_hex_integer(&mut self, start_pos: Pos) -> Result<SpannedToken> {
        self.read_byte(); // '$'
        let mut digits = Vec::new();
        while let Some(b) = self.peek_byte() {
            if !is_hex_digit(b) {
                break;
            }
            self.read_byte();
            digits.push(b);
        }
        if digits.is_empty() {
            return Err(IffError::Lex {
                at: start_pos,
                msg: "empty hex literal".into(),
            });
        }
        let s = std::str::from_utf8(&digits).unwrap();
        let val = u64::from_str_radix(s, 16).map_err(|e| IffError::Lex {
            at: start_pos.clone(),
            msg: format!("bad hex literal: {}", e),
        })?;
        let width = self.scan_width_suffix();
        Ok(SpannedToken {
            tok: Token::Integer { val, width },
            pos: start_pos,
        })
    }

    fn scan_width_suffix(&mut self) -> u8 {
        match self.peek_byte() {
            Some(b'y') => {
                self.read_byte();
                1
            }
            Some(b'w') => {
                self.read_byte();
                2
            }
            Some(b'l') => {
                self.read_byte();
                4
            }
            _ => 0,
        }
    }

    /// `scan_number` handles:
    /// - decimal integers (`42`, `-42`, with optional `[ywl]` suffix)
    /// - reals (`3.14`, `.5`, `3e6`, `.5e-2`)
    /// - integer with precision override (`3(1.15.16)` → real with integer mantissa)
    /// - bare `N.N.N` precision triple (argument of `.precision(...)`)
    /// - real with explicit precision override (`3.14(1.15.16)`)
    fn scan_number(&mut self, start_pos: Pos) -> Result<SpannedToken> {
        let mut raw: Vec<u8> = Vec::new();

        let mut negative = false;
        if self.peek_byte() == Some(b'-') {
            self.read_byte();
            negative = true;
            raw.push(b'-');
        }

        while let Some(b) = self.peek_byte() {
            if !is_digit(b) {
                break;
            }
            self.read_byte();
            raw.push(b);
        }

        // Try for `N.N.N` precision triple (only when there's no leading sign
        // and we see a `.`).
        let mut consumed_fraction = false;
        if !negative {
            if self.peek_byte() == Some(b'.') {
                self.read_byte();
                raw.push(b'.');
                let mut saw_digit = false;
                while let Some(b) = self.peek_byte() {
                    if !is_digit(b) {
                        break;
                    }
                    self.read_byte();
                    saw_digit = true;
                    raw.push(b);
                }
                if saw_digit && self.peek_byte() == Some(b'.') {
                    // Second dot → precision triple.
                    self.read_byte();
                    raw.push(b'.');
                    while let Some(b) = self.peek_byte() {
                        if !is_digit(b) {
                            break;
                        }
                        self.read_byte();
                        raw.push(b);
                    }
                    return build_prec_spec(start_pos, &raw);
                }
                consumed_fraction = true;
            }
        }

        // Real starting with `.N` (e.g. `.5`).
        if !consumed_fraction {
            if self.peek_byte() == Some(b'.') {
                self.read_byte();
                raw.push(b'.');
                while let Some(b) = self.peek_byte() {
                    if !is_digit(b) {
                        break;
                    }
                    self.read_byte();
                    raw.push(b);
                }
            }
        }

        // Optional exponent.
        if matches!(self.peek_byte(), Some(b'e') | Some(b'E')) {
            self.read_byte();
            raw.push(b'e');
            if matches!(self.peek_byte(), Some(b'+') | Some(b'-')) {
                raw.push(self.read_byte().unwrap());
            }
            while let Some(b) = self.peek_byte() {
                if !is_digit(b) {
                    break;
                }
                self.read_byte();
                raw.push(b);
            }
        }

        let is_real = raw.iter().any(|&c| c == b'.' || c == b'e' || c == b'E');
        let raw_str = std::str::from_utf8(&raw).unwrap();

        // Integer followed by `(N.N.N)` — reclassify as real with integer
        // mantissa. This is how the C++ flex regex parses `3(1.15.16)`.
        if !is_real && self.peek_byte() == Some(b'(') {
            let saved = self.save_pos();
            self.read_byte(); // '('
            if let Some(triple) = self.try_scan_prec_triple() {
                if self.peek_byte() == Some(b')') {
                    self.read_byte();
                    let val = raw_str.parse::<f64>().map_err(|e| IffError::Lex {
                        at: start_pos.clone(),
                        msg: format!("bad integer-as-real: {}", e),
                    })?;
                    return Ok(SpannedToken {
                        tok: Token::Real {
                            val,
                            precision: Some(triple),
                        },
                        pos: start_pos,
                    });
                }
            }
            self.restore_pos(saved);
        }

        if is_real {
            let val = raw_str.parse::<f64>().map_err(|e| IffError::Lex {
                at: start_pos.clone(),
                msg: format!("bad real {:?}: {}", raw_str, e),
            })?;
            // Optional explicit precision override.
            let mut precision = None;
            if self.peek_byte() == Some(b'(') {
                let saved = self.save_pos();
                self.read_byte();
                if let Some(triple) = self.try_scan_prec_triple() {
                    if self.peek_byte() == Some(b')') {
                        self.read_byte();
                        precision = Some(triple);
                    } else {
                        self.restore_pos(saved);
                    }
                } else {
                    self.restore_pos(saved);
                }
            }
            return Ok(SpannedToken {
                tok: Token::Real { val, precision },
                pos: start_pos,
            });
        }

        // Integer — optional width suffix.
        let width = self.scan_width_suffix();
        let val = raw_str.parse::<i64>().map_err(|e| IffError::Lex {
            at: start_pos.clone(),
            msg: format!("bad integer {:?}: {}", raw_str, e),
        })?;
        Ok(SpannedToken {
            tok: Token::Integer {
                val: val as u64,
                width,
            },
            pos: start_pos,
        })
    }

    fn try_scan_prec_triple(&mut self) -> Option<SizeSpec> {
        let read_num = |lex: &mut Lexer| -> Option<i32> {
            let mut digits = Vec::new();
            while let Some(b) = lex.peek_byte() {
                if !is_digit(b) {
                    break;
                }
                lex.read_byte();
                digits.push(b);
            }
            if digits.is_empty() {
                return None;
            }
            std::str::from_utf8(&digits).ok()?.parse::<i32>().ok()
        };
        let mut a = read_num(self)?;
        if self.peek_byte() != Some(b'.') {
            return None;
        }
        self.read_byte();
        let b = read_num(self)?;
        if self.peek_byte() != Some(b'.') {
            return None;
        }
        self.read_byte();
        let c = read_num(self)?;
        if !(0..=1).contains(&a) {
            a = 1;
        }
        Some(SizeSpec {
            sign: a,
            whole: b,
            fraction: c,
        })
    }
}

fn build_prec_spec(start_pos: Pos, raw: &[u8]) -> Result<SpannedToken> {
    let s = std::str::from_utf8(raw).unwrap();
    let parts: Vec<&str> = s.split('.').collect();
    if parts.len() != 3 {
        return Err(IffError::Lex {
            at: start_pos,
            msg: format!("bad precision spec {:?}", s),
        });
    }
    let mut a: i32 = parts[0].parse().map_err(|e| IffError::Lex {
        at: start_pos.clone(),
        msg: format!("bad precision spec {:?}: {}", s, e),
    })?;
    let b: i32 = parts[1].parse().map_err(|e| IffError::Lex {
        at: start_pos.clone(),
        msg: format!("bad precision spec {:?}: {}", s, e),
    })?;
    let c: i32 = parts[2].parse().map_err(|e| IffError::Lex {
        at: start_pos.clone(),
        msg: format!("bad precision spec {:?}: {}", s, e),
    })?;
    if !(0..=1).contains(&a) {
        a = 1;
    }
    Ok(SpannedToken {
        tok: Token::PrecSpec(SizeSpec {
            sign: a,
            whole: b,
            fraction: c,
        }),
        pos: start_pos,
    })
}

fn is_digit(b: u8) -> bool {
    b.is_ascii_digit()
}

fn is_hex_digit(b: u8) -> bool {
    b.is_ascii_hexdigit()
}

fn is_ident_char(b: u8) -> bool {
    b.is_ascii_alphanumeric() || b == b'_'
}
