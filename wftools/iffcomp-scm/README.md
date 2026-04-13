# iffcomp-scm

Racket port of [iffcomp](../iffcomp/), the WorldFoundry IFF compiler.
Reads a human-readable `.iff.txt` DSL and writes a binary or text IFF file.
Drop-in CLI replacement for the C++, [Go](../iffcomp-go/),
[Rust](../iffcomp-rs/), and [Node.js](../iffcomp-node/) versions.

## Build

```
racket main.rkt            # run directly
raco exe main.rkt          # compile to standalone binary
```

## Usage

```
iffcomp-scm [-o <file>] [-binary] [-ascii] [-v] [-q] <input.iff.txt>
```

| Flag | Default | Meaning |
|------|---------|---------|
| `-o <file>` | `test.wf` | Output path |
| `-binary` | ✓ | Binary IFF output |
| `-ascii` | | Human-readable text output |
| `-v` | | Verbose: trace parse productions to stderr |
| `-q` | | Quiet: suppress informational output |

Exit codes: `0` success, `10` compile error, `1` usage error.

## DSL quick reference

```
{ 'FOUR'                      // chunk — 4-char FOURCC (right-padded with NUL)
  42l                         // 4-byte LE integer  (y=1 byte, w=2, l=4)
  3.14(1.15.16)               // fixed-point real with S.W.F precision spec
  "hello"                     // C string (NUL-terminated)
  "padded"(256)               // string padded to exactly 256 bytes
  $DEADBEEF l                 // hex literal
  'ID  '                      // raw FOURCC as 4-byte value
  .timestamp                  // 4-byte POSIX time_t
  .align(4)                   // pad to alignment boundary
  .fillchar(0)                // byte used for .align padding (default 0)
  [ "file.bin" ]              // include raw file bytes
  [ "file.bin" .start(10) .length(20) ]   // sliced range
  .sizeof(::'FOUR')           // 4-byte payload size of a named chunk
  .offsetof(::'FOUR')         // 4-byte file offset of a named chunk
  { W 1 2 3 }                 // state-push: default width = 2 bytes
  { .precision(1.15.16) 3.14 }// state-push: default fixed-point precision
  { 'NEST'                    // chunks nest arbitrarily
    { 'DEEP' 99l }
  }
}
```

Forward references in `.sizeof` / `.offsetof` are resolved after the full
parse; backward references are resolved immediately. `//` line comments are
supported everywhere.

## Library API

```racket
(require "iffcomp.rkt")

(compile "input.iff.txt"     ; input path
         "output.iff"        ; output path
         'binary             ; 'binary or 'text
         #f)                 ; verbose?
; → exact-integer: bytes written
```

## Tests

```
raco test test/byte-exact.rkt
```

Byte-exact comparisons against the C++ oracle binary stored in `testdata/`
(shared with the Go port via symlink). The `.timestamp` chunk payload is
masked before comparison because it is time-varying.

## Implementation

| File | Purpose |
|------|---------|
| `lexer.rkt` | Hand-rolled tokenizer with include stack and escape continuation for early return |
| `parser.rkt` | Recursive-descent parser, state stack, 2-token lookahead |
| `writer.rkt` | Binary + text output, fixed-point encoding, back-patch queue |
| `iffcomp.rkt` | `compile` library entry point |
| `main.rkt` | CLI wrapper using `racket/cmdline` |

No external package dependencies — pure `#lang racket/base` with stdlib modules.

## Racket vs Rust

Racket is ~30% shorter code for the same behaviour and handles
arbitrary-precision integers more cleanly (no explicit cast dance for fixed-point
wrapping). Rust wins on binary size (~500 KB vs ~40 MB bundled runtime) and
startup time (<5 ms vs ~150 ms), which matters when iffcomp is called
repeatedly from a build system. For an interactive tool or a library consumed
from other Racket programs, the tradeoffs reverse.

Key porting notes from Go/Rust → Racket:

- No early `return`: replaced with `call-with-current-continuation` in `scan-number!`
- `define` not allowed in expression-position `begin`: use `let ()` instead
- `#lang racket/base` omits `file->bytes`, `path-only`, etc. — import explicitly
- File includes resolve relative to the input file's directory via `parameterize current-directory`
