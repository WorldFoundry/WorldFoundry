//! Recursive-descent parser for the iffcomp DSL.
//!
//! Mirrors the productions in `wftools/iffcomp/lang.y` one-for-one. The two
//! major differences from the C++ bison grammar are:
//!
//! 1. `chunk` vs. `state_push` disambiguation is explicit 2-token lookahead
//!    on `{` here. Bison handled it implicitly via its shift preference
//!    (which was the source of the 30 S/R conflicts in the `.y` file).
//! 2. The `MINUS` operator in `expr` is actually wired up to subtract,
//!    where the C++ grammar left the action empty. Two-character
//!    behavior change from the unfinished original.

use crate::error::{IffError, Pos, Result};
use crate::lexer::{Lexer, SizeSpec, Token};
use crate::writer::{id_name, Writer};
use std::path::PathBuf;

/// One layer of push-down state for the nested `{ Y … }` / `{ .precision … }`
/// blocks. The parser constructor seeds one default frame; each state-push
/// pushes a derived frame; the matching `}` pops.
pub struct State {
    pub size_override: i32,
    pub precision: SizeSpec,
}

pub struct Parser<'w> {
    lex: Lexer,
    w: &'w mut Writer,

    states: Vec<State>,

    // Per-item scratch state used by the file-include path.
    start_pos_override: u64,
    length_override: u64,

    pub verbose: bool,
}

impl<'w> Parser<'w> {
    pub fn new(lex: Lexer, w: &'w mut Writer) -> Self {
        Parser {
            lex,
            w,
            states: vec![State {
                size_override: 1,
                precision: SizeSpec {
                    sign: 1,
                    whole: 15,
                    fraction: 16,
                },
            }],
            start_pos_override: 0,
            length_override: u64::MAX,
            verbose: false,
        }
    }

    fn top_state(&self) -> &State {
        self.states.last().unwrap()
    }

    fn push_state(&mut self, s: State) {
        self.states.push(s);
    }

    fn pop_state(&mut self) {
        assert!(self.states.len() > 1, "pop_state on default stack");
        self.states.pop();
    }

    // --- token helpers ------------------------------------------------------

    fn peek(&mut self) -> Result<Token> {
        self.lex.peek(0)
    }

    fn peek1(&mut self) -> Result<Token> {
        self.lex.peek(1)
    }

    fn consume(&mut self) -> Result<Token> {
        Ok(self.lex.next()?.tok)
    }

    fn expect_pos(&mut self, want: &str) -> Pos {
        // Best-effort position of the next token for error reporting.
        // Doesn't consume anything.
        match self.lex.peek(0) {
            Ok(_) => {
                // lexer already populated its lookahead; grab the span.
                // We can't actually access the span here without changing
                // the lex interface, so we synthesize from Default.
                let _ = want;
                Pos::default()
            }
            Err(_) => Pos::default(),
        }
    }

    fn expect(&mut self, want: &Token, what: &str) -> Result<Token> {
        let got = self.peek()?;
        if !discriminant_eq(&got, want) {
            return Err(IffError::Parse {
                at: self.expect_pos(what),
                msg: format!("expected {}, got {}", what, got.name()),
            });
        }
        self.consume()
    }

    fn expect_integer(&mut self, what: &str) -> Result<(u64, u8)> {
        let tok = self.peek()?;
        if let Token::Integer { val, width } = tok {
            self.consume()?;
            Ok((val, width))
        } else {
            Err(IffError::Parse {
                at: self.expect_pos(what),
                msg: format!("expected {}, got {}", what, tok.name()),
            })
        }
    }

    fn expect_char_lit(&mut self, what: &str) -> Result<u32> {
        let tok = self.peek()?;
        if let Token::CharLit(v) = tok {
            self.consume()?;
            Ok(v)
        } else {
            Err(IffError::Parse {
                at: self.expect_pos(what),
                msg: format!("expected {}, got {}", what, tok.name()),
            })
        }
    }

    fn expect_prec_spec(&mut self, what: &str) -> Result<SizeSpec> {
        let tok = self.peek()?;
        if let Token::PrecSpec(s) = tok {
            self.consume()?;
            Ok(s)
        } else {
            Err(IffError::Parse {
                at: self.expect_pos(what),
                msg: format!("expected {}, got {}", what, tok.name()),
            })
        }
    }

    fn expect_string(&mut self, what: &str) -> Result<(String, usize)> {
        let tok = self.peek()?;
        if let Token::String {
            body,
            size_override,
        } = tok
        {
            self.consume()?;
            Ok((body, size_override))
        } else {
            Err(IffError::Parse {
                at: self.expect_pos(what),
                msg: format!("expected {}, got {}", what, tok.name()),
            })
        }
    }

    fn trace(&mut self, rule: &str) {
        if !self.verbose {
            return;
        }
        match self.peek() {
            Ok(tok) => eprintln!("parse: {} (lookahead={})", rule, tok.name()),
            Err(_) => eprintln!("parse: {} (lookahead=<err>)", rule),
        }
    }

    // --- top level ----------------------------------------------------------

    pub fn parse(&mut self) -> Result<()> {
        loop {
            match self.peek()? {
                Token::Eof => return Ok(()),
                _ => self.parse_chunk()?,
            }
        }
    }

    // --- chunk --------------------------------------------------------------

    fn parse_chunk(&mut self) -> Result<()> {
        self.trace("chunk");
        self.expect(&Token::LBrace, "'{' starting chunk")?;
        let id = self.expect_char_lit("chunk ID")?;
        self.w.enter_chunk(id);
        loop {
            match self.peek()? {
                Token::RBrace | Token::Eof => break,
                _ => self.parse_chunk_statement()?,
            }
        }
        self.expect(&Token::RBrace, "'}' closing chunk")?;
        self.w.exit_chunk();
        Ok(())
    }

    fn parse_chunk_statement(&mut self) -> Result<()> {
        match self.peek()? {
            Token::LBrace => {
                // chunk vs. state_push: 2-token lookahead.
                if matches!(self.peek1()?, Token::CharLit(_)) {
                    return self.parse_chunk();
                }
                // Fall through to expression path (handles state_push via parse_item).
            }
            Token::Align => return self.parse_alignment(),
            Token::FillChar => return self.parse_fill_char(),
            _ => {}
        }
        self.parse_expr().map(|_| ())
    }

    fn parse_alignment(&mut self) -> Result<()> {
        self.consume()?; // .align
        self.expect(&Token::LParen, "'(' after .align")?;
        let (n, _) = self.expect_integer("integer in .align")?;
        self.expect(&Token::RParen, "')' closing .align")?;
        if n == 0 {
            return Err(IffError::Parse {
                at: Pos::default(),
                msg: "align(0) doesn't make sense".into(),
            });
        }
        self.w.align_function(n as usize);
        Ok(())
    }

    fn parse_fill_char(&mut self) -> Result<()> {
        self.consume()?; // .fillchar
        self.expect(&Token::LParen, "'(' after .fillchar")?;
        let (n, _) = self.expect_integer("integer in .fillchar")?;
        self.expect(&Token::RParen, "')' closing .fillchar")?;
        self.w.set_fill_char(n as u8);
        Ok(())
    }

    // --- expr / item --------------------------------------------------------

    fn parse_expr(&mut self) -> Result<u64> {
        let mut lhs = self.parse_item()?;
        loop {
            let op = self.peek()?;
            match op {
                Token::Plus => {
                    self.consume()?;
                    let rhs = self.parse_item()?;
                    lhs = lhs.wrapping_add(rhs);
                }
                Token::Minus => {
                    self.consume()?;
                    let rhs = self.parse_item()?;
                    lhs = lhs.wrapping_sub(rhs);
                }
                _ => break,
            }
        }
        Ok(lhs)
    }

    fn parse_expr_list(&mut self) -> Result<()> {
        loop {
            match self.peek()? {
                Token::RBrace | Token::Eof => break,
                _ => {
                    self.parse_item()?;
                }
            }
        }
        Ok(())
    }

    fn parse_item(&mut self) -> Result<u64> {
        self.trace("item");
        let tok = self.peek()?;
        match tok {
            Token::LBrace => {
                self.parse_state_push()?;
                Ok(0)
            }
            Token::Real { val, precision } => {
                self.consume()?;
                let prec = precision.unwrap_or(self.top_state().precision);
                self.w.out_fixed(val, prec);
                Ok(0)
            }
            Token::Integer { val, width } => {
                self.consume()?;
                let width = if width == 0 {
                    self.top_state().size_override as u8
                } else {
                    width
                };
                match width {
                    1 => {
                        self.w.out_int8(val as u8);
                        if val > 0xFF {
                            return Err(IffError::Parse {
                                at: Pos::default(),
                                msg: "value doesn't fit in int8".into(),
                            });
                        }
                    }
                    2 => {
                        if val > 0xFFFF {
                            return Err(IffError::Parse {
                                at: Pos::default(),
                                msg: "value doesn't fit in int16".into(),
                            });
                        }
                        self.w.out_int16(val as u16);
                    }
                    4 => {
                        if val > 0x7FFF_FFFF {
                            return Err(IffError::Parse {
                                at: Pos::default(),
                                msg: "value doesn't fit in int32".into(),
                            });
                        }
                        self.w.out_int32(val as u32);
                    }
                    _ => {
                        return Err(IffError::Parse {
                            at: Pos::default(),
                            msg: format!("bad width {}", width),
                        });
                    }
                }
                Ok(val)
            }
            Token::String { .. } => {
                self.parse_string_list()?;
                Ok(0)
            }
            Token::LBrack => {
                self.parse_file_include()?;
                Ok(0)
            }
            Token::CharLit(v) => {
                self.consume()?;
                self.w.out_id(v);
                Ok(0)
            }
            Token::Timestamp => {
                self.consume()?;
                // Same as C++ / Go: wall-clock seconds. test.iff.txt doesn't
                // exercise this, so the byte-level oracle doesn't pin it —
                // but if it did we'd need SOURCE_DATE_EPOCH support here.
                let secs = std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .map(|d| d.as_secs() as i64)
                    .unwrap_or(0);
                self.w.out_timestamp(secs);
                Ok(0)
            }
            Token::Offsetof => {
                self.parse_offsetof()?;
                Ok(0)
            }
            Token::Sizeof => {
                self.parse_sizeof()?;
                Ok(0)
            }
            _ => Err(IffError::Parse {
                at: Pos::default(),
                msg: format!("unexpected {} in item", tok.name()),
            }),
        }
    }

    fn parse_state_push(&mut self) -> Result<()> {
        self.consume()?; // '{'
        let top = self.top_state();
        let mut new = State {
            size_override: top.size_override,
            precision: top.precision,
        };
        match self.peek()? {
            Token::SizeY => {
                self.consume()?;
                new.size_override = 1;
            }
            Token::SizeW => {
                self.consume()?;
                new.size_override = 2;
            }
            Token::SizeL => {
                self.consume()?;
                new.size_override = 4;
            }
            Token::Precision => {
                self.consume()?;
                self.expect(&Token::LParen, "'(' after .precision")?;
                let ps = self.expect_prec_spec("precision specifier")?;
                self.expect(&Token::RParen, "')' closing .precision")?;
                new.precision = ps;
            }
            other => {
                return Err(IffError::Parse {
                    at: Pos::default(),
                    msg: format!(
                        "expected Y/W/L or .precision inside '{{ … }}' block, got {}",
                        other.name()
                    ),
                });
            }
        }
        self.push_state(new);
        self.parse_expr_list()?;
        self.expect(&Token::RBrace, "'}' closing state push")?;
        self.pop_state();
        Ok(())
    }

    fn parse_string_list(&mut self) -> Result<()> {
        let (first_body, first_size) = self.expect_string("STRING")?;
        if first_size > 0 {
            self.w.out_string_pad(&first_body, first_size);
        } else {
            self.w.out_string(&first_body);
        }
        while matches!(self.peek()?, Token::String { .. }) {
            let (body, _) = self.expect_string("STRING")?;
            self.w.out_string_continue(&body);
        }
        Ok(())
    }

    fn parse_file_include(&mut self) -> Result<()> {
        self.consume()?; // '['
        self.start_pos_override = 0;
        self.length_override = u64::MAX;

        let (path, _) = self.expect_string("filename in '[ … ]'")?;

        loop {
            match self.peek()? {
                Token::Start | Token::Length => self.parse_extract_spec()?,
                _ => break,
            }
        }
        self.expect(&Token::RBrack, "']' closing file include")?;
        self.w
            .out_file(&PathBuf::from(&path), self.start_pos_override, self.length_override)
    }

    fn parse_extract_spec(&mut self) -> Result<()> {
        let tok = self.consume()?;
        self.expect(&Token::LParen, "'(' after extract spec")?;
        let (n, _) = self.expect_integer("integer in extract spec")?;
        self.expect(&Token::RParen, "')' closing extract spec")?;
        match tok {
            Token::Start => self.start_pos_override = n,
            Token::Length => self.length_override = n,
            _ => unreachable!(),
        }
        Ok(())
    }

    fn parse_chunk_specifier(&mut self) -> Result<String> {
        let mut path = String::new();
        while matches!(self.peek()?, Token::DoubleColon) {
            self.consume()?;
            let id = self.expect_char_lit("CHAR_LITERAL after '::'")?;
            path.push_str("::'");
            path.push_str(&id_name(id));
            path.push('\'');
        }
        if path.is_empty() {
            return Err(IffError::Parse {
                at: Pos::default(),
                msg: "expected chunkSpecifier starting with '::'".into(),
            });
        }
        Ok(path)
    }

    fn parse_offsetof(&mut self) -> Result<()> {
        self.consume()?; // .offsetof
        self.expect(&Token::LParen, "'(' after .offsetof")?;
        let path = self.parse_chunk_specifier()?;
        let mut addend = 0i32;
        if matches!(self.peek()?, Token::Comma) {
            self.consume()?;
            let (n, _) = self.expect_integer("addend integer in .offsetof")?;
            addend = n as i32;
        }
        self.expect(&Token::RParen, "')' closing .offsetof")?;
        self.w.emit_offsetof(&path, addend);
        Ok(())
    }

    fn parse_sizeof(&mut self) -> Result<()> {
        self.consume()?; // .sizeof
        self.expect(&Token::LParen, "'(' after .sizeof")?;
        let path = self.parse_chunk_specifier()?;
        self.expect(&Token::RParen, "')' closing .sizeof")?;
        self.w.emit_sizeof(&path);
        Ok(())
    }
}

/// Loose discriminant comparison — the parser's `expect()` helper only cares
/// that the token is the right *kind*, not that payload fields match. Built-in
/// `std::mem::discriminant` works because the variants are well-behaved but
/// is noisier than this helper.
fn discriminant_eq(a: &Token, b: &Token) -> bool {
    std::mem::discriminant(a) == std::mem::discriminant(b)
}
