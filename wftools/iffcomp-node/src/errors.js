// errors.js — IffError with source position info.

'use strict';

class IffError extends Error {
  /** @param {'lex'|'parse'|'io'|'unresolved'} kind */
  constructor(kind, message, file, line, col) {
    super(message);
    this.name = 'IffError';
    this.kind = kind;
    this.file = file;
    this.line = line;
    this.col = col;
  }
}

module.exports = { IffError };
