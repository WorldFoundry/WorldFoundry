// iffcomp.js — library entry point. Compile() is the public API.

'use strict';

const fs   = require('node:fs');
const path = require('node:path');
const { Lexer } = require('./lexer.js');
const { Writer } = require('./writer.js');
const { Parser } = require('./parser.js');

const Mode = { BINARY: 'binary', TEXT: 'text' };

/**
 * Compile an .iff.txt input file to a binary or text IFF output file.
 * Returns the number of bytes written.
 *
 * opts.cwd: working directory for relative path resolution (includes,
 * file-inclusion `[ "f" ]`). Defaults to the directory of inFile.
 */
function compile(inFile, outFile, opts = {}) {
  const mode    = opts.mode    || Mode.BINARY;
  const verbose = opts.verbose || false;
  const cwd     = opts.cwd    || path.dirname(path.resolve(inFile));

  const lex = new Lexer(inFile, cwd);
  const w   = new Writer(mode === Mode.TEXT, cwd);
  const p   = new Parser(lex, w, verbose);

  p.parse();
  w.resolveBackpatches();

  const data = w.bytes();
  fs.writeFileSync(outFile, data);
  return data.length;
}

module.exports = { compile, Mode };
