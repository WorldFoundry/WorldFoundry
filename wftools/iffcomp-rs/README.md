# iffcomp-rs

Rust port of [iffcomp](../iffcomp/), the WorldFoundry IFF compiler.
Reads a human-readable `.iff.txt` DSL and writes a binary or text IFF file.
Drop-in CLI replacement for the C++, [Go](../iffcomp-go/), and
[Node.js](../iffcomp-node/) versions.

## Build

```
cargo build --release
# binary: target/release/iffcomp-rs
```

## Usage

```
iffcomp-rs [-o=<file>] [-binary|-ascii] [-v] [-q] <input.iff.txt>
```

| Flag | Default | Meaning |
|------|---------|---------|
| `-o=<file>` | `test.wf` | Output path |
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

```rust
use iffcomp::{compile, Options, Mode};

let bytes_written = compile(
    "input.iff.txt",
    "output.iff",
    Options { mode: Mode::Binary, verbose: false },
)?;
```

## Tests

```
cargo test
```

Byte-exact comparisons against the C++ oracle binary stored in `testdata/`
(shared with the Go port). The `.timestamp` chunk payload is masked before
comparison because it is time-varying.

## Implementation

| File | Purpose |
|------|---------|
| `src/lexer.rs` | Hand-rolled tokenizer with include stack |
| `src/parser.rs` | Recursive-descent parser, state stack, back-patch queue |
| `src/writer.rs` | Binary + text output, fixed-point encoding |
| `src/error.rs` | `IffError` enum with `Io / Lex / Parse / UnresolvedSymbols` variants |
| `src/lib.rs` | `compile()` library entry point |
| `src/main.rs` | CLI wrapper |

No external crate dependencies.
