// lexer.js — hand-rolled tokenizer for the iffcomp DSL.
//
// Mirrors iffcomp-go/lexer.go one-for-one. The DSL is small (about a dozen
// keyword tokens, char / integer / real / string literals, a handful of
// punctuation), so a hand-rolled scanner is faster to write and easier to
// debug than a generator.

'use strict';

const fs   = require('node:fs');
const path = require('node:path');
const { IffError } = require('./errors.js');

// Token kinds (string constants — readable in errors, no enum needed).
const T = {
  EOF:          'EOF',
  LBRACE:       '{',
  RBRACE:       '}',
  LPAREN:       '(',
  RPAREN:       ')',
  LBRACK:       '[',
  RBRACK:       ']',
  COMMA:        ',',
  DOUBLE_COLON: '::',
  PLUS:         '+',
  MINUS:        '-',
  SIZE_Y:       'Y',          // uppercase Y keyword (state-push width)
  SIZE_W:       'W',
  SIZE_L:       'L',
  INTEGER:      'INTEGER',
  REAL:         'REAL',
  STRING:       'STRING',
  CHAR_LIT:     'CHAR_LIT',   // 1-4 char FOURCC, packed MSB-first
  PREC_SPEC:    'PREC_SPEC',  // bare N.N.N triple
  TIMESTAMP:    '.timestamp',
  ALIGN:        '.align',
  OFFSETOF:     '.offsetof',
  SIZEOF:       '.sizeof',
  FILLCHAR:     '.fillchar',
  PRECISION:    '.precision',
  START:        '.start',
  LENGTH:       '.length',
};

// --- Frame (one file on the include stack) -----------------------------------

class Frame {
  constructor(src, filename) {
    this.src      = src;       // Buffer
    this.offset   = 0;
    this.line     = 1;
    this.col      = 1;
    this.filename = filename;
  }
}

// --- Lexer -------------------------------------------------------------------

class Lexer {
  // cwd: base directory for resolving relative includes. Defaults to the
  // directory of the initial file (matching how the Go port uses os.Chdir).
  constructor(filename, cwd) {
    this.stack     = [];       // Frame[]
    this.lookahead = [];       // token[]
    this.cwd       = cwd || path.dirname(path.resolve(filename));
    this._pushFile(path.resolve(this.cwd, filename));
  }

  _pushFile(filename) {
    let src;
    try {
      src = fs.readFileSync(filename);
    } catch (e) {
      throw new IffError('io', `open ${filename}: ${e.message}`);
    }
    this.stack.push(new Frame(src, filename));
  }

  _pushSystemFile(name) {
    const dir = process.env.WF_DIR;
    if (!dir) throw new IffError('io', `include <${name}>: WF_DIR not set`);
    this._pushFile(path.join(dir, name));
  }

  _cur() {
    return this.stack.length ? this.stack[this.stack.length - 1] : null;
  }

  // peek(k): return k'th lookahead token without consuming it.
  peek(k) {
    while (this.lookahead.length <= k) {
      this.lookahead.push(this._scan());
    }
    return this.lookahead[k];
  }

  next() {
    if (this.lookahead.length) return this.lookahead.shift();
    return this._scan();
  }

  // --- raw byte ops ----------------------------------------------------------

  _peekByte() {
    while (true) {
      const f = this._cur();
      if (!f) return -1;
      if (f.offset < f.src.length) return f.src[f.offset];
      // End of this file — pop include stack.
      if (this.stack.length <= 1) return -1;
      this.stack.pop();
    }
  }

  _readByte() {
    const b = this._peekByte();
    if (b === -1) return -1;
    const f = this._cur();
    f.offset++;
    if (b === 0x0a /* \n */) { f.line++; f.col = 1; }
    else { f.col++; }
    return b;
  }

  _currentPos() {
    const f = this._cur();
    if (!f) return { filename: '', line: 0, col: 0 };
    return { filename: f.filename, line: f.line, col: f.col };
  }

  _posStr(p) { return `${p.filename}:${p.line}:${p.col}`; }

  _peekAt(k) {
    const f = this._cur();
    if (!f || f.offset + k >= f.src.length) return 0;
    return f.src[f.offset + k];
  }

  // --- scan ------------------------------------------------------------------

  _scan() {
    // Skip whitespace and // comments.
    while (true) {
      const b = this._peekByte();
      if (b === -1) return { kind: T.EOF };
      if (b === 0x20 || b === 0x09 || b === 0x0d || b === 0x0a) { this._readByte(); continue; }
      if (b === 0x2f /* / */) {
        const f = this._cur();
        if (f && f.offset + 1 < f.src.length && f.src[f.offset + 1] === 0x2f) {
          // Line comment.
          while (true) { const c = this._readByte(); if (c === -1 || c === 0x0a) break; }
          continue;
        }
      }
      break;
    }

    const startPos = this._currentPos();
    const b = this._peekByte();
    if (b === -1) return { kind: T.EOF, pos: startPos };

    // `.5` → real; `.timestamp` etc → keyword. Disambiguate on next byte.
    if (b === 0x2e /* . */) {
      if (isDigit(this._peekAt(1))) return this._scanNumber(startPos);
      return this._scanDotKeyword(startPos);
    }

    // `include "f"` or `include <f>` — consume inline, not as tokens.
    if (b === 0x69 /* i */ && this._hasPrefix('include') && !isIdentChar(this._peekAt(7))) {
      this._handleInclude();
      return this._scan();
    }

    // Single-char tokens.
    switch (b) {
      case 0x7b: this._readByte(); return { kind: T.LBRACE, pos: startPos };
      case 0x7d: this._readByte(); return { kind: T.RBRACE, pos: startPos };
      case 0x28: this._readByte(); return { kind: T.LPAREN, pos: startPos };
      case 0x29: this._readByte(); return { kind: T.RPAREN, pos: startPos };
      case 0x5b: this._readByte(); return { kind: T.LBRACK, pos: startPos };
      case 0x5d: this._readByte(); return { kind: T.RBRACK, pos: startPos };
      case 0x2c: this._readByte(); return { kind: T.COMMA, pos: startPos };
      case 0x2b: this._readByte(); return { kind: T.PLUS, pos: startPos };
      case 0x59: this._readByte(); return { kind: T.SIZE_Y, pos: startPos };
      case 0x57: this._readByte(); return { kind: T.SIZE_W, pos: startPos };
      case 0x4c: this._readByte(); return { kind: T.SIZE_L, pos: startPos };
      case 0x27: return this._scanCharLiteral(startPos); // '
      case 0x22: return this._scanString(startPos);       // "
      case 0x3a: {                                         // :
        this._readByte();
        const b2 = this._peekByte();
        if (b2 === 0x3a) { this._readByte(); return { kind: T.DOUBLE_COLON, pos: startPos }; }
        throw new IffError('lex', `${this._posStr(startPos)}: expected '::' after ':'`);
      }
    }

    // '-' → negative literal or binary minus.
    if (b === 0x2d) {
      const next = this._peekAt(1);
      if (isDigit(next) || next === 0x2e) return this._scanNumber(startPos);
      this._readByte();
      return { kind: T.MINUS, pos: startPos };
    }

    // '$' → hex integer.
    if (b === 0x24) return this._scanHexInteger(startPos);

    if (isDigit(b)) return this._scanNumber(startPos);

    this._readByte();
    throw new IffError('lex', `${this._posStr(startPos)}: unexpected character ${JSON.stringify(String.fromCharCode(b))}`);
  }

  _hasPrefix(p) {
    const f = this._cur();
    if (!f || f.offset + p.length > f.src.length) return false;
    for (let i = 0; i < p.length; i++) {
      if (f.src[f.offset + i] !== p.charCodeAt(i)) return false;
    }
    return true;
  }

  // --- include ---------------------------------------------------------------

  _handleInclude() {
    // Consume 'include'.
    for (let i = 0; i < 7; i++) this._readByte();
    // Skip whitespace.
    while (true) {
      const b = this._peekByte();
      if (b !== 0x20 && b !== 0x09) break;
      this._readByte();
    }
    const b = this._peekByte();
    if (b === -1) throw new IffError('lex', 'include: unexpected EOF');
    let closer, system;
    if (b === 0x22) { closer = 0x22; system = false; }
    else if (b === 0x3c) { closer = 0x3e; system = true; }
    else throw new IffError('lex', `include: expected '"' or '<', got ${JSON.stringify(String.fromCharCode(b))}`);
    this._readByte(); // opening quote
    const name = [];
    while (true) {
      const c = this._readByte();
      if (c === -1) throw new IffError('lex', 'include: unexpected EOF inside filename');
      if (c === closer) break;
      name.push(c);
    }
    const nameStr = Buffer.from(name).toString();
    if (system) this._pushSystemFile(nameStr);
    else this._pushFile(path.resolve(this.cwd, nameStr));
  }

  // --- dot keywords ----------------------------------------------------------

  _scanDotKeyword(startPos) {
    this._readByte(); // consume '.'
    const ident = [];
    while (true) {
      const b = this._peekByte();
      if (b === -1 || !isIdentChar(b)) break;
      this._readByte();
      ident.push(b);
    }
    const s = Buffer.from(ident).toString();
    switch (s) {
      case 'timestamp': return { kind: T.TIMESTAMP, pos: startPos };
      case 'align':     return { kind: T.ALIGN,     pos: startPos };
      case 'offsetof':  return { kind: T.OFFSETOF,  pos: startPos };
      case 'sizeof':    return { kind: T.SIZEOF,    pos: startPos };
      case 'fillchar':  return { kind: T.FILLCHAR,  pos: startPos };
      case 'start':     return { kind: T.START,     pos: startPos };
      case 'length':    return { kind: T.LENGTH,    pos: startPos };
      case 'precision': return { kind: T.PRECISION, pos: startPos };
    }
    throw new IffError('lex', `${this._posStr(startPos)}: unknown directive .${s}`);
  }

  // --- char literal ----------------------------------------------------------

  _scanCharLiteral(startPos) {
    this._readByte(); // opening '
    const chars = [];
    while (true) {
      const b = this._peekByte();
      if (b === -1) throw new IffError('lex', `${this._posStr(startPos)}: unterminated char literal`);
      if (b === 0x27) { this._readByte(); break; }
      if (chars.length >= 4) throw new IffError('lex', `${this._posStr(startPos)}: char literal too long`);
      this._readByte();
      chars.push(b);
    }
    if (chars.length === 0) throw new IffError('lex', `${this._posStr(startPos)}: empty char literal`);
    // Pack MSB-first into uint32, right-pad with NUL.
    let v = 0;
    for (let i = 0; i < 4; i++) {
      const c = i < chars.length ? chars[i] : 0;
      v = (v | (c << (24 - 8 * i))) >>> 0;
    }
    return { kind: T.CHAR_LIT, pos: startPos, charLit: v };
  }

  // --- strings ---------------------------------------------------------------

  _scanString(startPos) {
    this._readByte(); // opening "
    const body = [];
    while (true) {
      const b = this._peekByte();
      if (b === -1 || b === 0x0a) break; // unterminated — permissive like original
      if (b === 0x22) { this._readByte(); break; }
      if (b === 0x5c /* \ */) {
        this._readByte();
        const c = this._readByte();
        if (c === -1) throw new IffError('lex', `${this._posStr(startPos)}: unterminated escape`);
        // Preserve escapes literally; writer resolves them.
        body.push(0x5c, c);
        continue;
      }
      this._readByte();
      body.push(b);
    }

    // Optional (N) size override immediately after closing quote.
    let sizeOverride = 0;
    if (this._peekByte() === 0x28 /* ( */) {
      const f = this._cur();
      const savedOffset = f.offset, savedLine = f.line, savedCol = f.col;
      f.offset++; f.col++; // skip '('
      const num = [];
      while (true) {
        const c = this._peekByte();
        if (c === -1 || c === 0x29 /* ) */) break;
        if (!isDigit(c)) { num.length = 0; break; }
        this._readByte();
        num.push(c);
      }
      if (num.length > 0 && this._peekByte() === 0x29) {
        this._readByte(); // ')'
        sizeOverride = parseInt(Buffer.from(num).toString(), 10);
      } else {
        // Roll back.
        f.offset = savedOffset; f.line = savedLine; f.col = savedCol;
      }
    }
    return { kind: T.STRING, pos: startPos, str: Buffer.from(body).toString('binary'), strSize: sizeOverride };
  }

  // --- numbers ---------------------------------------------------------------

  _scanWidthSuffix() {
    const b = this._peekByte();
    if (b === 0x79 /* y */) { this._readByte(); return 1; }
    if (b === 0x77 /* w */) { this._readByte(); return 2; }
    if (b === 0x6c /* l */) { this._readByte(); return 4; }
    return 0;
  }

  _scanHexInteger(startPos) {
    this._readByte(); // consume '$'
    const digits = [];
    while (true) {
      const b = this._peekByte();
      if (b === -1 || !isHexDigit(b)) break;
      this._readByte();
      digits.push(b);
    }
    if (digits.length === 0) throw new IffError('lex', `${this._posStr(startPos)}: empty hex literal`);
    const v = parseInt(Buffer.from(digits).toString(), 16);
    const width = this._scanWidthSuffix();
    return { kind: T.INTEGER, pos: startPos, intVal: v, intWidth: width };
  }

  _scanNumber(startPos) {
    const raw = [];
    let negative = false;
    if (this._peekByte() === 0x2d /* - */) {
      this._readByte();
      negative = true;
      raw.push(0x2d);
    }

    // Leading digits (may be empty for ".5").
    while (true) {
      const b = this._peekByte();
      if (b === -1 || !isDigit(b)) break;
      this._readByte();
      raw.push(b);
    }

    // Possibility: bare N.N.N precision triple (no leading sign, two dots).
    let consumedFraction = false;
    if (!negative && this._peekByte() === 0x2e /* . */) {
      const f = this._cur();
      const savedOffset = f.offset, savedLine = f.line, savedCol = f.col;
      this._readByte();
      raw.push(0x2e);
      let sawDigit = false;
      while (true) {
        const b = this._peekByte();
        if (b === -1 || !isDigit(b)) break;
        this._readByte();
        raw.push(b);
        sawDigit = true;
      }
      if (sawDigit && this._peekByte() === 0x2e) {
        // Second dot → precision triple.
        this._readByte();
        raw.push(0x2e);
        while (true) {
          const b = this._peekByte();
          if (b === -1 || !isDigit(b)) break;
          this._readByte();
          raw.push(b);
        }
        return this._buildPrecSpec(startPos, Buffer.from(raw).toString());
      }
      consumedFraction = true;
    }

    // Possibility: real starting with '.N' (e.g. `.5`).
    if (!consumedFraction && this._peekByte() === 0x2e /* . */) {
      this._readByte();
      raw.push(0x2e);
      while (true) {
        const b = this._peekByte();
        if (b === -1 || !isDigit(b)) break;
        this._readByte();
        raw.push(b);
      }
    }

    // Optional exponent.
    {
      const b = this._peekByte();
      if (b === 0x65 /* e */ || b === 0x45 /* E */) {
        this._readByte();
        raw.push(b);
        const s = this._peekByte();
        if (s === 0x2b || s === 0x2d) { this._readByte(); raw.push(s); }
        while (true) {
          const d = this._peekByte();
          if (d === -1 || !isDigit(d)) break;
          this._readByte();
          raw.push(d);
        }
      }
    }

    const rawStr = Buffer.from(raw).toString();
    const isReal = rawStr.includes('.') || rawStr.includes('e') || rawStr.includes('E');

    // Integer followed directly by `(N.N.N)` is a real with integer mantissa
    // and explicit precision override — `3(1.15.16)` tokenized as REAL.
    if (!isReal && this._peekByte() === 0x28 /* ( */) {
      const f = this._cur();
      const savedOffset = f.offset, savedLine = f.line, savedCol = f.col;
      this._readByte(); // '('
      const triple = this._tryScanPrecTriple();
      if (triple !== null && this._peekByte() === 0x29) {
        this._readByte(); // ')'
        const v = parseFloat(rawStr);
        return { kind: T.REAL, pos: startPos, realVal: v, realPrec: triple };
      }
      // Rollback.
      f.offset = savedOffset; f.line = savedLine; f.col = savedCol;
    }

    if (isReal) {
      const v = parseFloat(rawStr);
      let precOverride = null;
      if (this._peekByte() === 0x28 /* ( */) {
        const f = this._cur();
        const savedOffset = f.offset, savedLine = f.line, savedCol = f.col;
        this._readByte(); // '('
        const triple = this._tryScanPrecTriple();
        if (triple !== null && this._peekByte() === 0x29) {
          this._readByte(); // ')'
          precOverride = triple;
        } else {
          f.offset = savedOffset; f.line = savedLine; f.col = savedCol;
        }
      }
      return { kind: T.REAL, pos: startPos, realVal: v, realPrec: precOverride };
    }

    // Integer — optional width suffix.
    const width = this._scanWidthSuffix();
    const v = parseInt(rawStr, 10);
    return { kind: T.INTEGER, pos: startPos, intVal: v >>> 0, intWidth: width };
  }

  // Tries to scan N.N.N; returns {sign, whole, fraction} or null. Caller must
  // save/restore frame state on failure.
  _tryScanPrecTriple() {
    const readNum = () => {
      const digits = [];
      while (true) {
        const b = this._peekByte();
        if (b === -1 || !isDigit(b)) break;
        this._readByte();
        digits.push(b);
      }
      if (!digits.length) return null;
      return parseInt(Buffer.from(digits).toString(), 10);
    };
    const a = readNum();
    if (a === null) return null;
    if (this._peekByte() !== 0x2e) return null;
    this._readByte();
    const bNum = readNum();
    if (bNum === null) return null;
    if (this._peekByte() !== 0x2e) return null;
    this._readByte();
    const c = readNum();
    if (c === null) return null;
    return { sign: a < 0 || a > 1 ? 1 : a, whole: bNum, fraction: c };
  }

  _buildPrecSpec(startPos, raw) {
    const parts = raw.split('.');
    if (parts.length !== 3) throw new IffError('lex', `${this._posStr(startPos)}: bad precision spec ${JSON.stringify(raw)}`);
    let a = parseInt(parts[0], 10);
    const b = parseInt(parts[1], 10);
    const c = parseInt(parts[2], 10);
    if (a < 0 || a > 1) a = 1;
    return { kind: T.PREC_SPEC, pos: startPos, precSpec: { sign: a, whole: b, fraction: c } };
  }
}

// --- helpers -----------------------------------------------------------------

function isDigit(b)    { return b >= 0x30 && b <= 0x39; }
function isHexDigit(b) { return isDigit(b) || (b >= 0x61 && b <= 0x66) || (b >= 0x41 && b <= 0x46); }
function isIdentChar(b) {
  return (b >= 0x61 && b <= 0x7a) || (b >= 0x41 && b <= 0x5a) || isDigit(b) || b === 0x5f;
}

module.exports = { T, Lexer };
