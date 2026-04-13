// parser.js — recursive-descent parser for the iffcomp DSL.
//
// Mirrors iffcomp-go/parser.go one-for-one. Productions:
//
//   statement_list -> chunk+
//   chunk          -> '{' CHAR_LIT chunk_statement* '}'
//   chunk_statement-> chunk | .align | .fillchar | expr
//   expr           -> item ( ('+' | '-') item )*
//   item           -> state_push | REAL | INTEGER | string_list
//                  |  '[' STRING extractSpec* ']'
//                  |  CHAR_LIT | .timestamp
//                  |  .offsetof '(' chunkSpec (',' INTEGER)? ')'
//                  |  .sizeof   '(' chunkSpec ')'
//   state_push     -> '{' (Y|W|L | .precision '(' PREC_SPEC ')') expr_list '}'
//   string_list    -> STRING+
//   chunkSpec      -> ('::' CHAR_LIT)+
//   extractSpec    -> .start '(' INTEGER ')' | .length '(' INTEGER ')'

'use strict';

const { T } = require('./lexer.js');
const { IffError } = require('./errors.js');

const DEFAULT_PRECISION = { sign: 1, whole: 15, fraction: 16 };
const DEFAULT_SIZE      = 1;

function tokName(k) {
  return k; // token kinds are already readable strings
}

class Parser {
  constructor(lexer, writer, verbose) {
    this.lex     = lexer;
    this.w       = writer;
    this.verbose = verbose;
    this.states  = [{ sizeOverride: DEFAULT_SIZE, precision: DEFAULT_PRECISION }];

    // Scratch for file-include extract specs.
    this.startPos = 0;
    this.length   = Infinity;
  }

  _topState() { return this.states[this.states.length - 1]; }

  _pushState(s) { this.states.push(s); }

  _popState() {
    if (this.states.length <= 1) throw new Error('popState on default stack');
    this.states.pop();
  }

  _peek()  { return this.lex.peek(0); }
  _peek1() { return this.lex.peek(1); }

  _consume() { return this.lex.next(); }

  _expect(kind, what) {
    const tok = this._peek();
    if (tok.kind !== kind) {
      const p = tok.pos ? `${tok.pos.filename}:${tok.pos.line}:${tok.pos.col}` : '?';
      throw new IffError('parse', `${p}: expected ${what}, got ${tokName(tok.kind)}`);
    }
    return this._consume();
  }

  _posStr(tok) {
    if (!tok.pos) return '?';
    return `${tok.pos.filename}:${tok.pos.line}:${tok.pos.col}`;
  }

  _trace(rule) {
    if (!this.verbose) return;
    const tok = this._peek();
    process.stderr.write(`parse: ${rule} @ ${this._posStr(tok)} (lookahead=${tokName(tok.kind)})\n`);
  }

  // --- top level -------------------------------------------------------------

  parse() {
    while (this._peek().kind !== T.EOF) {
      this._parseChunk();
    }
  }

  // --- chunk -----------------------------------------------------------------

  _parseChunk() {
    this._trace('chunk');
    this._expect(T.LBRACE, "'{' starting chunk");
    const idTok = this._expect(T.CHAR_LIT, 'chunk ID');
    this.w.enterChunk(idTok.charLit);
    while (this._peek().kind !== T.RBRACE && this._peek().kind !== T.EOF) {
      this._parseChunkStatement();
    }
    this._expect(T.RBRACE, "'}' closing chunk");
    this.w.exitChunk();
  }

  // --- chunk_statement -------------------------------------------------------

  _parseChunkStatement() {
    const tok = this._peek();
    if (tok.kind === T.LBRACE) {
      if (this._peek1().kind === T.CHAR_LIT) return this._parseChunk();
      // Fall through to expr (handles state_push).
    } else if (tok.kind === T.ALIGN) {
      return this._parseAlignment();
    } else if (tok.kind === T.FILLCHAR) {
      return this._parseFillChar();
    }
    this._parseExpr();
  }

  _parseAlignment() {
    this._consume(); // .align
    this._expect(T.LPAREN, "'(' after .align");
    const n = this._expect(T.INTEGER, 'integer in .align');
    this._expect(T.RPAREN, "')' closing .align");
    if (n.intVal === 0) {
      throw new IffError('parse', `${this._posStr(n)}: .align(0) doesn't make sense`);
    }
    this.w.alignFunction(n.intVal);
  }

  _parseFillChar() {
    this._consume(); // .fillchar
    this._expect(T.LPAREN, "'(' after .fillchar");
    const n = this._expect(T.INTEGER, 'integer in .fillchar');
    this._expect(T.RPAREN, "')' closing .fillchar");
    this.w.setFillChar(n.intVal);
  }

  // --- expr ------------------------------------------------------------------

  _parseExpr() {
    let lhs = this._parseItem();
    while (true) {
      const op = this._peek().kind;
      if (op !== T.PLUS && op !== T.MINUS) break;
      this._consume();
      const rhs = this._parseItem();
      lhs = op === T.PLUS ? (lhs + rhs) : (lhs - rhs);
    }
    return lhs;
  }

  _parseExprList() {
    while (this._peek().kind !== T.RBRACE && this._peek().kind !== T.EOF) {
      this._parseItem();
    }
  }

  // --- item ------------------------------------------------------------------

  _parseItem() {
    this._trace('item');
    const tok = this._peek();

    switch (tok.kind) {
      case T.LBRACE:
        this._parseStatePush();
        return 0;

      case T.REAL: {
        this._consume();
        const prec = tok.realPrec !== null ? tok.realPrec : this._topState().precision;
        this.w.outFixed(tok.realVal, prec);
        return 0;
      }

      case T.INTEGER: {
        this._consume();
        let width = tok.intWidth;
        if (width === 0) width = this._topState().sizeOverride;
        const v = tok.intVal;
        switch (width) {
          case 1:
            if (v > 0xFF) throw new IffError('parse', `${this._posStr(tok)}: value doesn't fit in int8`);
            this.w.outInt8(v);
            break;
          case 2:
            if (v > 0xFFFF) throw new IffError('parse', `${this._posStr(tok)}: value doesn't fit in int16`);
            this.w.outInt16(v);
            break;
          case 4:
            if (v > 0x7FFFFFFF) throw new IffError('parse', `${this._posStr(tok)}: value doesn't fit in int32`);
            this.w.outInt32(v);
            break;
          default:
            throw new IffError('parse', `${this._posStr(tok)}: bad width ${width}`);
        }
        return v;
      }

      case T.STRING:
        this._parseStringList();
        return 0;

      case T.LBRACK:
        this._parseFileInclude();
        return 0;

      case T.CHAR_LIT:
        this._consume();
        this.w.outID(tok.charLit);
        return 0;

      case T.TIMESTAMP:
        this._consume();
        this.w.outTimestamp(Math.floor(Date.now() / 1000));
        return 0;

      case T.OFFSETOF:
        this._parseOffsetof();
        return 0;

      case T.SIZEOF:
        this._parseSizeof();
        return 0;
    }

    throw new IffError('parse', `${this._posStr(tok)}: unexpected ${tokName(tok.kind)} in item`);
  }

  // --- state_push ------------------------------------------------------------

  _parseStatePush() {
    this._consume(); // '{'
    const newState = Object.assign({}, this._topState());
    const tok = this._peek();
    switch (tok.kind) {
      case T.SIZE_Y: this._consume(); newState.sizeOverride = 1; break;
      case T.SIZE_W: this._consume(); newState.sizeOverride = 2; break;
      case T.SIZE_L: this._consume(); newState.sizeOverride = 4; break;
      case T.PRECISION:
        this._consume(); // .precision
        this._expect(T.LPAREN, "'(' after .precision");
        const ps = this._expect(T.PREC_SPEC, 'precision specifier');
        this._expect(T.RPAREN, "')' closing .precision");
        newState.precision = ps.precSpec;
        break;
      default:
        throw new IffError('parse',
          `${this._posStr(tok)}: expected Y/W/L or .precision inside '{ … }' block`);
    }
    this._pushState(newState);
    this._parseExprList();
    this._expect(T.RBRACE, "'}' closing state push");
    this._popState();
  }

  // --- string_list -----------------------------------------------------------

  _parseStringList() {
    const first = this._consume(); // STRING
    if (first.strSize > 0) {
      this.w.outStringPad(first.str, first.strSize);
    } else {
      this.w.outString(first.str);
    }
    while (this._peek().kind === T.STRING) {
      const next = this._consume();
      this.w.outStringContinue(next.str);
    }
  }

  // --- file include ----------------------------------------------------------

  _parseFileInclude() {
    this._consume(); // '['
    this.startPos = 0;
    this.length   = Infinity;

    const pathTok = this._expect(T.STRING, "filename string in '[ … ]'");
    while (this._peek().kind === T.START || this._peek().kind === T.LENGTH) {
      this._parseExtractSpec();
    }
    this._expect(T.RBRACK, "']' closing file include");
    this.w.outFile(pathTok.str, this.startPos, this.length);
  }

  _parseExtractSpec() {
    const tok = this._consume(); // .start or .length
    this._expect(T.LPAREN, "'(' after extract spec");
    const n = this._expect(T.INTEGER, 'integer in extract spec');
    this._expect(T.RPAREN, "')' closing extract spec");
    if (tok.kind === T.START) this.startPos = n.intVal;
    else this.length = n.intVal;
  }

  // --- chunk specifier -------------------------------------------------------

  _parseChunkSpecifier() {
    let p = '';
    while (this._peek().kind === T.DOUBLE_COLON) {
      this._consume();
      const id = this._expect(T.CHAR_LIT, "CHAR_LIT after '::'");
      const { idName } = require('./writer.js');
      p += "::'" + idName(id.charLit) + "'";
    }
    if (!p) {
      const tok = this._peek();
      throw new IffError('parse',
        `${this._posStr(tok)}: expected chunkSpecifier starting with '::'`);
    }
    return p;
  }

  // --- .offsetof / .sizeof ---------------------------------------------------

  _parseOffsetof() {
    this._consume(); // .offsetof
    this._expect(T.LPAREN, "'(' after .offsetof");
    const path = this._parseChunkSpecifier();
    let addend = 0;
    if (this._peek().kind === T.COMMA) {
      this._consume();
      const n = this._expect(T.INTEGER, 'addend integer in .offsetof');
      addend = n.intVal;
    }
    this._expect(T.RPAREN, "')' closing .offsetof");
    this.w.emitOffsetof(path, addend);
  }

  _parseSizeof() {
    this._consume(); // .sizeof
    this._expect(T.LPAREN, "'(' after .sizeof");
    const path = this._parseChunkSpecifier();
    this._expect(T.RPAREN, "')' closing .sizeof");
    this.w.emitSizeof(path);
  }
}

module.exports = { Parser };
