// writer.js — IFF binary/text writer.
//
// Mirrors iffcomp-go/writer.go. Key invariants:
//
//   - Chunk header: 4-byte FOURCC (big-endian / source order) + 4-byte
//     little-endian size placeholder (0xFFFFFFFF), then payload, then
//     align(4) padding with zero bytes for siblings.
//   - .sizeof / .offsetof: resolved immediately if target already closed;
//     otherwise queued as a back-patch and written as 0 (sizeof) or
//     0xFFFFFFFF (offsetof). Drained by resolveBackpatches() after parse.
//   - Fixed-point: val × 2^fraction, truncated to width from total bit count.
//   - Strings: C-style escapes resolved at write time, not lex time.

'use strict';

const fs   = require('node:fs');
const path = require('node:path');
const { IffError } = require('./errors.js');

// Back-patch kinds.
const BP_SIZEOF   = 0;
const BP_OFFSETOF = 1;

class Writer {
  constructor(textMode, cwd) {
    this.textMode  = textMode;
    this.cwd       = cwd || process.cwd();
    this.fillChar  = 0;
    this.stack     = [];       // chunkFrame[]
    this.pathIDs   = [];       // uint32[] — running open-chunk ID list
    this.symbols   = new Map();// path → {pos, size}
    this.pending   = [];       // backpatchRec[]

    // Binary mode.
    this.buf = Buffer.allocUnsafe(4096);
    this.pos = 0;

    // Text mode.
    this.textChunks = [];      // string[] — pieces joined at end
    this.textOnLine = 0;       // bytes emitted by outFile on the current line
  }

  bytes() {
    if (this.textMode) return Buffer.from(this.textChunks.join(''), 'binary');
    return this.buf.slice(0, this.pos);
  }

  // --- buffer helpers (binary) -----------------------------------------------

  _grow(needed) {
    if (needed <= this.buf.length) return;
    let cap = this.buf.length * 2;
    if (cap < needed) cap = needed;
    const next = Buffer.allocUnsafe(cap);
    this.buf.copy(next, 0, 0, this.pos);
    this.buf = next;
  }

  _writeAt(src, off) {
    this._grow(off + src.length);
    src.copy(this.buf, off);
  }

  _write(src) {
    this._grow(this.pos + src.length);
    src.copy(this.buf, this.pos);
    this.pos += src.length;
  }

  _writeByte(b) {
    this._grow(this.pos + 1);
    this.buf[this.pos++] = b;
  }

  // --- primitive emitters ----------------------------------------------------

  outInt8(v) {
    v = v & 0xFF;
    if (this.textMode) { this._textEmit(((v << 24 >> 24)).toString() + 'y '); return; }
    this._writeByte(v);
  }

  outInt16(v) {
    v = v & 0xFFFF;
    if (this.textMode) { this._textEmit(((v << 16 >> 16)).toString() + 'w '); return; }
    const b = Buffer.allocUnsafe(2);
    b.writeUInt16LE(v, 0);
    this._write(b);
  }

  outInt32(v) {
    v = v >>> 0;
    if (this.textMode) { this._textEmit(((v | 0)).toString() + 'l '); return; }
    const b = Buffer.allocUnsafe(4);
    b.writeUInt32LE(v, 0);
    this._write(b);
  }

  _align(n) {
    if (this.textMode) return; // IffWriterText::align is a no-op
    if (n <= 1) return;
    const rem = this.pos % n;
    if (rem === 0) return;
    const pad = n - rem;
    this._grow(this.pos + pad);
    this.buf.fill(0, this.pos, this.pos + pad);
    this.pos += pad;
  }

  alignFunction(n) {
    if (this.textMode) return;
    if (n <= 1) return;
    const rem = this.pos % n;
    if (rem === 0) return;
    const pad = n - rem;
    this._grow(this.pos + pad);
    this.buf.fill(this.fillChar, this.pos, this.pos + pad);
    this.pos += pad;
  }

  setFillChar(b) { this.fillChar = b & 0xFF; }

  outID(id) {
    if (this.textMode) { this._textEmit("'" + idName(id) + "' "); return; }
    this._align(4);
    const b = Buffer.allocUnsafe(4);
    b.writeUInt32BE(id >>> 0, 0);
    this._write(b);
  }

  // --- chunks ----------------------------------------------------------------

  enterChunk(id) {
    const depth = this.stack.length;
    this.pathIDs.push(id);
    const pathKey = buildPathKey(this.pathIDs);

    if (this.textMode) {
      this._textEmit('\n');
      for (let i = 0; i < depth; i++) this._textEmit('\t');
      this._textEmit("{ '" + idName(id) + "' ");
      this.stack.push({ id, pathKey });
      return;
    }

    this._align(4);
    const startPos = this.pos;
    const idBytes = Buffer.allocUnsafe(4);
    idBytes.writeUInt32BE(id >>> 0, 0);
    this._write(idBytes);
    const sizeFieldPos = this.pos;
    this.outInt32(0xFFFFFFFF); // placeholder — patched at exitChunk
    this.stack.push({ id, startPos, sizeFieldPos, pathKey });
  }

  exitChunk() {
    if (!this.stack.length) throw new Error('exitChunk with empty stack');
    const top = this.stack.pop();
    this.pathIDs.pop();
    const depth = this.stack.length; // depth after pop

    if (this.textMode) {
      this._textEmit('\n');
      for (let i = 0; i < depth; i++) this._textEmit('\t');
      this._textEmit('}');
      return;
    }

    const payloadStart = top.sizeFieldPos + 4;
    const size = this.pos - payloadStart;

    // Patch size field (LE).
    const szBytes = Buffer.allocUnsafe(4);
    szBytes.writeUInt32LE(size >>> 0, 0);
    this._writeAt(szBytes, top.sizeFieldPos);

    // Record in symbol table.
    this.symbols.set(top.pathKey, { pos: payloadStart, size });

    // Align to next sibling boundary (zeros, not fillChar).
    this._align(4);
  }

  // --- sizeof / offsetof -----------------------------------------------------

  emitSizeof(path) {
    const sym = this.symbols.get(path);
    if (sym) { this.outInt32(sym.size); return; }
    if (this.textMode) { this.outInt32(0); return; }
    this.pending.push({ kind: BP_SIZEOF, path, addend: 0, writePos: this.pos });
    this.outInt32(0);
  }

  emitOffsetof(path, addend) {
    const sym = this.symbols.get(path);
    if (sym) { this.outInt32(sym.pos - 4 + addend); return; }
    if (this.textMode) { this.outInt32(0); return; }
    this.pending.push({ kind: BP_OFFSETOF, path, addend, writePos: this.pos });
    this.outInt32(0xFFFFFFFF);
  }

  resolveBackpatches() {
    if (this.textMode) return;
    const missing = [];
    for (const bp of this.pending) {
      const sym = this.symbols.get(bp.path);
      if (!sym) { missing.push(bp.path); continue; }
      let val;
      if (bp.kind === BP_SIZEOF) {
        val = sym.size >>> 0;
      } else {
        // Deferred offsetof: sym.pos - 8 (= ID_pos). See Go writer for the
        // 4-byte asymmetry between immediate and deferred resolution.
        val = (sym.pos - 8 + bp.addend) >>> 0;
      }
      const b = Buffer.allocUnsafe(4);
      b.writeUInt32LE(val, 0);
      this._writeAt(b, bp.writePos);
    }
    if (missing.length) {
      throw new IffError('unresolved', `unresolved chunk references: ${missing.join(', ')}`);
    }
  }

  // --- strings ---------------------------------------------------------------

  outString(s) {
    if (this.textMode) { this._textEmitString(s); return; }
    const translated = translateEscapeCodes(s);
    this._write(Buffer.from(translated, 'binary'));
    this._writeByte(0); // NUL terminator
  }

  outStringContinue(s) {
    if (this.textMode) { this.outString(s); return; }
    if (this.pos > 0) this.pos--; // seek back over previous NUL
    this.outString(s);
  }

  outStringPad(s, totalBytes) {
    if (this.textMode) { this.outString(s); return 0; }
    this.outString(s);
    const required = translateEscapeCodes(s).length + 1; // NUL
    if (required >= totalBytes) return required - totalBytes;
    const pad = totalBytes - required;
    this._grow(this.pos + pad);
    this.buf.fill(0, this.pos, this.pos + pad);
    this.pos += pad;
    return 0;
  }

  // --- timestamp / file ------------------------------------------------------

  outTimestamp(unixSeconds) {
    this.outInt32(unixSeconds >>> 0);
  }

  outFile(filePath, start, length) {
    const resolved = path.resolve(this.cwd, filePath);
    let data;
    try {
      data = fs.readFileSync(resolved);
    } catch (e) {
      throw new IffError('io', `read ${resolved}: ${e.message}`);
    }
    if (start >= data.length) {
      throw new IffError('io', `${resolved}: start ${start} past EOF (${data.length} bytes)`);
    }
    const end = (length === Infinity || start + length >= data.length) ? data.length : start + length;
    const slice = data.slice(start, end);

    if (this.textMode) {
      for (let i = 0; i < slice.length; i++) {
        const b = slice[i];
        const signed = b > 127 ? b - 256 : b;
        this._textEmit(signed.toString() + 'y ');
        this.textOnLine++;
        if (this.textOnLine === 100) {
          this._textEmitComment();
          this.textOnLine = 0;
        }
      }
      // Trailing comment (matches C++ out_mem's out_comment("") at end).
      this._textEmitComment();
      this.textOnLine = 0;
      return;
    }

    this._write(slice);
  }

  // --- fixed-point -----------------------------------------------------------

  outFixed(val, prec) {
    if (this.textMode) {
      const s = formatGAlt(val, 16);
      this._textEmit(`${s}(${prec.sign}.${prec.whole}.${prec.fraction}) `);
      return;
    }
    const bits = prec.sign + prec.whole + prec.fraction;
    // Compute val × 2^fraction as int64 via BigInt to handle negatives.
    const scale = BigInt(1) << BigInt(prec.fraction);
    const scaled = BigInt(Math.trunc(val * Number(scale)));
    if (bits > 16) {
      this.outInt32(Number(BigInt.asUintN(32, scaled)));
    } else if (bits > 8) {
      this.outInt16(Number(BigInt.asUintN(16, scaled)));
    } else {
      this.outInt8(Number(BigInt.asUintN(8, scaled)));
    }
  }

  // --- text helpers ----------------------------------------------------------

  _textEmit(s) { this.textChunks.push(s); }

  _textEmitString(s) {
    this._textEmit('"');
    for (let i = 0; i < s.length; i++) {
      const c = s.charCodeAt(i);
      if (c === 0x0a) { this._textEmit('\\n"\n"'); }
      else if (c === 0x5c) { this._textEmit('\\\\'); }
      else if (c === 0x22) { this._textEmit('\\"'); }
      else { this._textEmit(s[i]); }
    }
    this._textEmit('" ');
  }

  _textEmitComment() {
    this._textEmit(' //\n');
    const tabs = this.stack.length + 1;
    for (let i = 0; i < tabs; i++) this._textEmit('\t');
  }
}

// --- helpers -----------------------------------------------------------------

function idName(id) {
  id = id >>> 0;
  const b = [id >>> 24, (id >>> 16) & 0xFF, (id >>> 8) & 0xFF, id & 0xFF];
  let n = 4;
  while (n > 0 && b[n - 1] === 0) n--;
  return Buffer.from(b.slice(0, n)).toString('binary');
}

function buildPathKey(ids) {
  let s = '';
  for (const id of ids) s += "::'" + idName(id) + "'";
  return s;
}

function translateEscapeCodes(s) {
  const out = [];
  for (let i = 0; i < s.length; i++) {
    if (s[i] !== '\\' || i + 1 >= s.length) { out.push(s.charCodeAt(i)); continue; }
    const c = s[i + 1];
    if (c === 't') { out.push(0x09); i++; }
    else if (c === 'n') { out.push(0x0a); i++; }
    else if (c === '\\') { out.push(0x5c); i++; }
    else if (c === '"') { out.push(0x22); i++; }
    else if (c >= '0' && c <= '9') {
      let j = i + 1;
      while (j < s.length && s[j] >= '0' && s[j] <= '9') j++;
      let n = 0;
      for (let k = i + 1; k < j; k++) n = n * 10 + (s.charCodeAt(k) - 0x30);
      out.push(n & 0xFF);
      i = j - 1;
    } else {
      out.push(0x5c, s.charCodeAt(i + 1));
      i++;
    }
  }
  return Buffer.from(out).toString('binary');
}

// formatGAlt implements C's `%#.16g` for the text writer.
// Rules: 16 significant digits; scientific notation when exp < -4 or exp >= 16;
// the `#` flag keeps trailing zeros and always shows the decimal point.
function formatGAlt(val, prec) {
  if (!isFinite(val)) return String(val);
  if (val === 0) {
    // `%#.16g` of 0 → "0.000000000000000" (15 zeros after decimal point).
    return '0.' + '0'.repeat(prec - 1);
  }
  const sign = val < 0 ? '-' : '';
  const abs = Math.abs(val);
  const exp = Math.floor(Math.log10(abs));

  if (exp >= prec || exp < -4) {
    // Scientific notation: e.g. 1.234567890123456e+05
    // toPrecision gives us the right number of significant digits.
    let s = abs.toPrecision(prec); // "1.23...e+5" or similar
    // Normalize exponent to C format (e+05 not e+5).
    s = s.replace(/e([+-])(\d)$/, 'e$10$2');
    return sign + s;
  } else {
    // Fixed notation with `prec` significant digits.
    const decimalPlaces = prec - 1 - exp;
    let s = abs.toFixed(Math.max(0, decimalPlaces));
    // Ensure there's a decimal point (for # flag).
    if (!s.includes('.')) s += '.';
    // Pad with trailing zeros if toFixed gave fewer digits than expected
    // (can happen when decimalPlaces is 0 but we still need the point).
    const dotIdx = s.indexOf('.');
    const afterDot = s.length - dotIdx - 1;
    if (afterDot < Math.max(0, decimalPlaces)) {
      s += '0'.repeat(Math.max(0, decimalPlaces) - afterDot);
    }
    return sign + s;
  }
}

module.exports = { Writer, idName, buildPathKey, translateEscapeCodes };
