#!/usr/bin/env node
// main.js — CLI wrapper matching the C++ iffcomp switches for drop-in use.
//
// Usage: iffcomp [-o=file] [-binary|-ascii] [-v] [-q] <input.iff.txt>

'use strict';

const { compile, Mode } = require('./iffcomp.js');

function usage() {
  process.stderr.write('usage: iffcomp [-o=file] [-binary|-ascii] [-v] [-q] <input>\n');
  process.exit(1);
}

const args = process.argv.slice(2).flatMap(a => a.startsWith('--') ? [a.slice(1)] : [a]);

let outFile  = 'test.wf';
let mode     = Mode.BINARY;
let verbose  = false;
let quiet    = false;
const inputs = [];

for (let i = 0; i < args.length; i++) {
  const a = args[i];
  if (a.startsWith('-o=')) { outFile = a.slice(3); }
  else if (a === '-o') { if (i + 1 >= args.length) usage(); outFile = args[++i]; }
  else if (a === '-binary') { mode = Mode.BINARY; }
  else if (a === '-ascii')  { mode = Mode.TEXT; }
  else if (a === '-v')      { verbose = true; }
  else if (a === '-q')      { quiet = true; }
  else if (!a.startsWith('-')) { inputs.push(a); }
  else { process.stderr.write(`unknown flag: ${a}\n`); usage(); }
}

if (inputs.length !== 1) usage();

try {
  const n = compile(inputs[0], outFile, { mode, verbose });
  if (verbose) process.stderr.write(`wrote ${n} bytes to ${outFile}\n`);
} catch (e) {
  process.stderr.write('iffcomp: ' + e.message + '\n');
  process.exit(10);
}
