// byte_exact.test.js — end-to-end byte-exact comparison against the C++ oracle.
//
// All reference binaries live in testdata/ (symlinked to iffcomp-go/testdata/).
// compile() receives cwd=TESTDATA so that `[ "TODO" ]` file-inclusion
// resolves relative to the fixtures without needing process.chdir().

'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs   = require('node:fs');
const os   = require('node:os');
const path = require('node:path');

const { compile, Mode } = require('../src/iffcomp.js');

const TESTDATA = path.resolve(__dirname, '..', 'testdata');

function tempFile(suffix) {
  return path.join(os.tmpdir(), `iffcomp-node-test-${process.pid}-${Date.now()}${suffix}`);
}

function hexDiffContext(got, want, center, radius) {
  const lo = Math.max(0, center - radius);
  const hi = center + radius;
  let out = '';
  for (const [label, buf] of [['got ', got], ['want', want]]) {
    let line = `  ${label} @ 0x${lo.toString(16)}:`;
    for (let i = lo; i < hi && i < buf.length; i++) {
      line += (i === center ? '>' : ' ') + buf[i].toString(16).padStart(2, '0');
    }
    out += line + '\n';
  }
  return out;
}

function byteCompare(got, want, label) {
  const n = Math.min(got.length, want.length);
  for (let i = 0; i < n; i++) {
    if (got[i] !== want[i]) {
      assert.fail(
        `${label}: byte mismatch at 0x${i.toString(16)} (${i})\n` +
        hexDiffContext(got, want, i, 16)
      );
    }
  }
  assert.equal(got.length, want.length,
    `${label}: length mismatch: got ${got.length}, want ${want.length}`);
}

// maskTimestamp zeros the 4-byte payload of the first TIME chunk found at a
// 4-byte boundary in data.
function maskTimestamp(data) {
  const pattern = Buffer.from([0x54, 0x49, 0x4d, 0x45, 0x04, 0x00, 0x00, 0x00]); // TIME + size=4
  for (let i = 0; i + pattern.length <= data.length; i += 4) {
    let match = true;
    for (let j = 0; j < pattern.length; j++) {
      if (data[i + j] !== pattern[j]) { match = false; break; }
    }
    if (match) {
      const ts = i + pattern.length;
      for (let j = ts; j < ts + 4 && j < data.length; j++) data[j] = 0;
      return;
    }
  }
}

test('binary output matches C++ oracle (test.iff.txt)', () => {
  const outFile = tempFile('.iff');
  try {
    compile(path.join(TESTDATA, 'test.iff.txt'), outFile, { mode: Mode.BINARY, cwd: TESTDATA });
    const got  = fs.readFileSync(outFile);
    const want = fs.readFileSync(path.join(TESTDATA, 'expected.iff'));
    byteCompare(got, want, 'binary');
  } finally {
    try { fs.unlinkSync(outFile); } catch (_) {}
  }
});

test('text output matches C++ oracle (test.iff.txt)', () => {
  const outFile = tempFile('.iff.txt');
  try {
    compile(path.join(TESTDATA, 'test.iff.txt'), outFile, { mode: Mode.TEXT, cwd: TESTDATA });
    const got  = fs.readFileSync(outFile);
    const want = fs.readFileSync(path.join(TESTDATA, 'expected.iff.txt'));
    byteCompare(got, want, 'text');
  } finally {
    try { fs.unlinkSync(outFile); } catch (_) {}
  }
});

test('binary output matches C++ oracle (all_features.iff.txt)', () => {
  const outFile = tempFile('-all.iff');
  try {
    compile(path.join(TESTDATA, 'all_features.iff.txt'), outFile, { mode: Mode.BINARY, cwd: TESTDATA });
    const got  = fs.readFileSync(outFile);
    const want = fs.readFileSync(path.join(TESTDATA, 'all_features.iff'));
    maskTimestamp(got);
    maskTimestamp(want);
    byteCompare(got, want, 'all_features binary');
  } finally {
    try { fs.unlinkSync(outFile); } catch (_) {}
  }
});
