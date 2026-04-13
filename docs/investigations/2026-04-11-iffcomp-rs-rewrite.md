# Investigation: Rust rewrite of `wftools/iffcomp`

**Rust is the fourth implementation. Byte-exact on the first iteration, then made a comprehensive torture test work across Go and Rust.**

**Date:** 2026-04-11
**Context:** This is the third iffcomp port in this session, following the [C++ modernization](2026-04-11-iffcomp-modernization.md) and the [Go rewrite](2026-04-11-iffcomp-go-rewrite.md). The companion analysis doc [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md) argues Rust is the preferred long-term rewrite target for this codebase; this session walks through the first real port to validate that recommendation against a tool with a concrete byte-exact oracle.

## 1. TL;DR

- **Byte-exact on first build.** Unlike the Go port (which hit two lexer disambiguation bugs on `TestByteExactAgainstCpp`), the Rust port compiled cleanly on the first `cargo build` and passed both `byte_exact_binary_vs_cpp` and `byte_exact_text_vs_cpp` on the first `cargo test`. Why: the two disambiguations that bit Go were pre-fixed in Rust because I'd already debugged them. Same for the LP64 `int32`/`long` trap, the fixed-point modular cast through `i64`, and the `IffWriterText` off-by-one indent — each one a landmine in Go, each one pre-fixed in Rust.
- **Comprehensive torture fixture.** Added `all_features.iff.txt` and committed the C++ oracle as `all_features.iff`. The fixture exercises every grammar production in `lang.y` that doesn't require environment setup: nested chunks at depth > 1, `.sizeof`/`.offsetof` (forward *and* backward references), hex integer literals, state-push blocks (`{ Y … }`, `{ W … }`, `{ L … }`, `{ .precision … }`), string escape sequences, padded strings, integer-mantissa reals, numeric expressions, precision state pushes, `.align(N)` with `.fillchar(N)`, file inclusion with `.start(N)`/`.length(M)` slicing, and `.timestamp` (masked in the test harness). Shared verbatim between the Rust and Go testdata directories.
- **Four implementations now pass the same torture test.** Rust: 444 bytes, byte-exact. Go: 444 bytes, byte-exact. C++: the oracle. All three via the same `all_features.iff.txt` fixture.
- **Line count:** ~2410 lines of Rust, zero dependencies, 110 ms debug build, 556 KB release binary.
- **Three bugs surfaced** during torture-test bring-up — two were in the fixture (misunderstanding the DSL's alignment invariant), one was a real asymmetry in the C++ grammar's `.offsetof` formulas that I hadn't noticed in the Go port either. Details in §4.

## 2. Structure

```
wftools/iffcomp-rs/
├── Cargo.toml              zero-deps, binary + lib, edition 2021
├── src/
│   ├── error.rs        74  IffError enum, Pos struct, Result alias
│   ├── lib.rs          51  compile() entry + Options + Mode re-export
│   ├── main.rs         79  CLI: -o=file, -binary|-ascii, -v, -q
│   ├── lexer.rs       892  hand-rolled tokenizer, variant-enum tokens
│   ├── parser.rs      534  recursive descent mirroring lang.y productions
│   └── writer.rs      649  binary + text via Mode enum
├── tests/
│   └── byte_exact.rs  210  three differential tests + timestamp mask
└── testdata/
    ├── test.iff.txt         copied from iffcomp-go/testdata/
    ├── TODO                 referenced by `[ "TODO" ]` inclusion
    ├── expected.iff         292-byte C++ IffWriterBinary oracle
    ├── expected.iff.txt    1214-byte C++ IffWriterText oracle
    ├── all_features.iff.txt comprehensive torture fixture
    └── all_features.iff     444-byte C++ torture oracle
```

**Production-to-function mapping** is identical to the Go port (see [`2026-04-11-iffcomp-go-rewrite.md`](2026-04-11-iffcomp-go-rewrite.md#2-structure) §2). The parser is a flat `impl<'w> Parser<'w>` with one method per `lang.y` rule. The writer is a single `Writer` struct with `mode: Mode` and each emit method matches on the mode at entry:

```rust
pub fn out_int8(&mut self, v: u8) {
    match self.mode {
        Mode::Binary => self.buf.push(v),
        Mode::Text => {
            use std::fmt::Write;
            let _ = write!(self.text_buf, "{}y ", v as i8);
        }
    }
}
```

This is the same "single struct with mode flag" approach as Go. Slightly dirtier than two types behind a trait but keeps the binary and text implementations next to each other for easy diffing and avoids introducing dynamic dispatch for a two-mode selector.

**Rust-specific idioms worth calling out:**

- **Variant-enum tokens.** Where Go has a `token` struct with always-present fields (`intVal`, `realVal`, `str`, `charLit`, etc.) populated per kind, Rust has a `Token` enum with per-variant data. Pattern matching in the parser reaches into the variant directly: `if let Token::Integer { val, width } = tok { … }`. This eliminates a class of bugs where a field is read for the wrong token kind.
- **`IffError` enum + `Result` alias.** No `thiserror`, no `anyhow`. The whole crate has zero external dependencies.
- **`ChdirGuard` with `Drop` impl** in the test file — the test needs to `chdir` into `testdata/` so `[ "TODO" ]` resolves to the local copy, and the guard restores the original working directory on scope exit, matching the Go version's `t.Cleanup` hook.
- **Mutex poisoning recovery.** The tests serialize on a `static Mutex<()>` for CWD safety, with a `lock_cwd()` helper that calls `.into_inner()` on a `PoisonError` so a panic in one test doesn't cascade-fail subsequent tests. Go doesn't need this because its tests run single-threaded by default within a package.

## 3. Why byte-exact on the first iteration

Unlike the Go port, I already knew every landmine the Rust version could step on because I'd debugged them all in Go first. Concretely:

1. **`3(1.15.16)` is a REAL with integer mantissa, not an INTEGER followed by LPAREN.** The flex regex is a single pattern covering both; I wrote the Rust lexer with a speculative-rollback path for the `(N.N.N)` triple after a digit-only mantissa from the start. [Go port: caught by `TestByteExactAgainstCpp` iteration 1.]
2. **`.5` is a real, `.keyword` is a directive.** The top-level `.` dispatcher checks whether the next byte is a digit before routing. [Go port: caught iteration 2.]
3. **LP64 `int32` is 4 bytes, not `sizeof(long)`.** Rust has `i32` and `u32` as distinct types from `i64`/`u64`; writing `v.to_le_bytes()` for a `u32` produces exactly 4 bytes without any `sizeof` dance.
4. **Fixed-point negative values need modular wrap via `i64`.** `(val * scale) as i64 as u32` goes through 64-bit signed and then reinterprets as unsigned, matching the gcc x86-64 output. A direct `f64 as u32` would saturate (Rust's undefined-behavior-avoidance-since-1.45 semantics) and produce the wrong bytes.
5. **`IffWriterText` off-by-one indent.** The C++ `_IffWriter` constructor pushes a sentinel `ChunkSizeBackpatch` so `chunkSize.size() == 1 + depth`, and `iffwrite/text.cc`'s indent formulas all assume this. I baked the off-by-one into the Rust `text_emit_indent(depth)` helper directly: depth-before-push for `{`, depth-after-pop for `}`, depth+1 for the `out_file` wrap marker.

The only genuine new work was **`format_g_alt`** — a ~40-line function mimicking printf's `%#.Ng` behavior (N significant digits, decimal point always shown, trailing zeros preserved) because Rust's `format!` has no equivalent alt-form. For the four reals in `test.iff.txt` (`3`, `0.3`, `0.4`, `0.5`) it produces `3.000000000000000`, `0.3000000000000000`, `0.4000000000000000`, `0.5000000000000000` — byte-for-byte matching the C++ iostream output. The exponential path in `format_g_alt` isn't exercised by any current fixture; if a future fixture triggers it, the formatter may need more work.

## 4. Bugs the torture test surfaced

The torture fixture stressed paths that `test.iff.txt` doesn't. Three distinct problems came up; two were my misunderstanding of the DSL, one was a real grammar quirk.

### 4.1 Alignment invariant (fixture error, not library bug)

My first draft of `all_features.iff.txt` put variable-length items (strings, file-includes) *before* nested chunks. Running the C++ oracle generator tripped the `exitChunk` assertion:

```
AssertMsg:Current Position = 444, starting position = 430, size = 8
pos == (cs->GetPos() + cs->GetSize() + 4)
in file "binary.cc" on line 338
```

My first instinct was to call it a library bug and patch `IffWriterBinary::out_id` to stop calling `align(4)` mid-chunk. The user correctly pushed back:

> chunkSize is the size of the data itself, period. no fourcc header, no size, and no padding calculated in
>
> i don't think it is a c++ library bug / maybe a misunderstanding

And they were right. The contract is: **nested chunks must enter from 4-aligned positions**. The C++ library computes `cs.pos = tellp() + 4` at `_IffWriter::enterChunk` time (before `out_id`'s `align(4)` runs), so when `out_id` pads to align the new child's header, `cs.pos` captures the pre-align position and the child's `exit_chunk` assertion fires. Real fixtures (the `.oas` schema pipeline output, the existing `test.iff.txt`, etc.) always satisfy the invariant by ordering items carefully — nested chunks go at the top of their parent's payload, variable-length stuff comes after.

The fix was authorial, not mechanical: I reorganized `all_features.iff.txt` to put all nested chunks (NEST, LATE, TIME) before any variable-length items, and added a comment at the top of the fixture explaining the invariant. No library changes.

**Lesson for future rewrites:** the original grammar's `exit_chunk` assertion is a useful contract check and should be reproduced faithfully. It catches DSL authors who accidentally nest chunks after variable-length data.

### 4.2 `.offsetof` immediate vs. deferred formulas disagree by 4 bytes

The fixture contains both backward and forward references to chunks. The backward references (to `NEST`, already closed) are resolved immediately at parse time; the forward references (to `LATE`, still ahead) are queued and resolved at end-of-parse by `resolve_backpatches`. These should produce the same encoding for the same target. They don't:

```cpp
// Immediate path (lang.y:319-322):
g._iff->out_int32( cs->GetPos() );              // = ID_pos + 4

// Deferred path (grammar.cc ~Grammar):
long val = ... ? cs->GetPos() - 4 : ...;         // = ID_pos
```

The immediate path writes the *size field position* (chunk ID offset + 4); the deferred path writes the *ID position* (chunk ID offset + 0). Same target, different value, differing by 4 bytes.

This is almost certainly a bug in the original C++ grammar — forward and backward references should resolve to the same value. But the fixtures in the project depend on whatever the C++ grammar actually emits, so any port has to reproduce the asymmetry byte-for-byte:

```rust
// Rust stores sym.pos = payload_start = ID_pos + 8, so:
// Immediate:  sym.pos - 4  =>  ID_pos + 4   (matches C++ immediate)
// Deferred:   sym.pos - 8  =>  ID_pos       (matches C++ deferred)
```

Go had the same bug: `resolveBackpatches` used `sym.pos - 4` everywhere, matching the immediate path. `test.iff.txt` doesn't exercise `.offsetof` at all, so the Go test passed — but the torture test caught it. I patched Go's `resolveBackpatches` to use `sym.pos - 8` for the `bpOffsetof` case and added the shared torture test as `TestAllFeaturesAgainstCpp`; Go now passes all three fixtures too.

### 4.3 Negative integer literals are effectively broken in the grammar

The lexer happily tokenizes `-42` (the regex is `-?[0-9]+[ywl]?`), but `sscanf %ld` into an `unsigned long` field produces `0xFFFFFFFFFFFFFFD6` on LP64 Linux, which exceeds every width range check in the parser's `out_int*` path (`val > 255` for int8, `val > 65535` for int16, `val > 0x7FFFFFFF` for int32). The grammar errors out, increments `_nErrors`, and the destructor deletes the output file.

This affects C++, Go, and Rust identically. All three inherit the bug from the grammar's design.

Practical consequence for the fixture: **no negative integer literals**. To get "negative-looking" byte values, use hex literals (`$FFy` → 0xFF, `$FFFFw` → 0xFFFF, `$FFFFFFFFl` → would fail the int32 check, so `$7FFFFFFFl` is the practical max). The fixture uses `$DEy $BEEFw $7ACEBABEl` for its hex coverage.

I added a comment to the fixture explaining the limitation and why it's unavoidable without changing the grammar semantics.

## 5. Test harness

Three tests, all passing byte-exact against C++ artifacts:

| Test | Fixture | Mode | Bytes | Status |
|---|---|---|---:|:---:|
| `byte_exact_binary_vs_cpp` | `test.iff.txt` | binary | 292 | ✓ |
| `byte_exact_text_vs_cpp` | `test.iff.txt` | text | 1214 | ✓ |
| `byte_exact_all_features_vs_cpp` | `all_features.iff.txt` | binary | 444 | ✓ |

The all-features test masks the 4 bytes after the `TIME\x04\x00\x00\x00` chunk header in both buffers before comparison (the `.timestamp` value is non-deterministic). The mask scan is 4-byte-aligned to avoid coincidental matches elsewhere in the payload.

Regenerating the oracles:

```sh
cd wftools/iffcomp
make
./iffcomp -binary -o=../iffcomp-rs/testdata/expected.iff ../iffcomp-rs/testdata/test.iff.txt
./iffcomp -ascii  -o=../iffcomp-rs/testdata/expected.iff.txt ../iffcomp-rs/testdata/test.iff.txt
./iffcomp -binary -o=../iffcomp-rs/testdata/all_features.iff ../iffcomp-rs/testdata/all_features.iff.txt
cp ../iffcomp-rs/testdata/all_features.iff{,.txt} ../iffcomp-go/testdata/
```

## 6. Four-implementation comparison

|  | C++ (modernized) | Go | Rust |
|---|---:|---:|---:|
| Source LoC (excl. tests) | ~2320 | ~2250 | ~2200 |
| External deps | 13 internal static libs | 0 (stdlib) | 0 (stdlib) |
| Upstream patches required | 10 files | 0 | 0 |
| Build time (clean) | ~30 s recursive make | 110 ms | 2.15 s (release) |
| Binary size | ~1.5 MB | 2.7 MB | 556 KB |
| Bring-up bugs on first fixture | N/A (pre-existing code) | 2 lexer | 0 |
| Bring-up bugs on torture fixture | N/A | 1 (deferred offsetof) | 1 (deferred offsetof) |
| Tests passing | 3 via shell scripts | 3 via `go test` | 3 via `cargo test` |
| Binary-mode output size | 292 + 444 | 292 + 444 | 292 + 444 |
| Text-mode output size | 1214 | 1214 | 1214 |

All three ports (and the C++ oracle) produce byte-identical output for both fixtures across both modes (where applicable). The only asymmetry is in the fixtures the ports are validated against — Rust and Go both run `test.iff.txt` + `all_features.iff.txt`; C++ produces the oracles for both but doesn't have its own test runner beyond `make test-bin` / `make test-txt`.

## 7. Known gaps still not covered

Even after the torture fixture, a few grammar features remain unvalidated:

- **`include "file"` and `include <file>`** — the lexer has the code path but no fixture exercises it. Low-value to add because the include semantics are trivial (open the file, push a frame, scan as normal). Can be covered by a separate tiny fixture with a one-liner `.iff.txt` that includes a second file.
- **`.timestamp` value correctness** — currently masked out. The value is `time()` at parse time. Could be made deterministic via `SOURCE_DATE_EPOCH` support in all three ports, but that's a behavior change from the original C++ and not trivially matched against the unpatched oracle.
- **Very large hex integers** — `$FFFFFFFFl` triggers the int32 range check and errors out. `$7FFFFFFFl` is the max. Not really a gap — the grammar's range check is what it is.
- **Real numbers outside the default fixed-point range**. The test fixtures all use small reals (< 10). The `format_g_alt` fallback path for exponential notation is untested.
- **Deeply nested chunks** (> 3 levels). The torture fixture goes to depth 3 (`ROOT → NEST → DEEP`). Arbitrary nesting should work but hasn't been stress-tested.
- **Very long payloads** — nothing in the testdata is larger than a couple hundred bytes.

None of these are blockers. The torture fixture is comfortable coverage for a tool that's already used in production builds.

## 8. Next steps

1. **Wire `wfsource/source/iffwrite/test.iff.txt`** into all three test harnesses as another cross-checked fixture.
2. **Commit `.oas`-derived `.iff.txt` files** from `wfsource/source/oas/` as differential testdata. Every one is a free regression test against the real asset pipeline output.
3. **`cargo fuzz`** over the lexer — the speculative-rollback paths in `scan_number` and the string-literal `(N)` size-override parser are the most likely future regression sites.
4. **Decide which port is the primary.** The [wftools rewrite analysis](2026-04-11-wftools-rewrite-analysis.md) argues for Rust long-term. Short-term the choice is about who ships the tool: Go builds faster, Rust produces smaller binaries, both have zero upstream patch requirements. No urgency to pick yet; both tests run in ~2 ms and both binaries are trivially maintainable.
5. **Lift `writer.rs` into a reusable `worldfoundry-iff` crate** if/when other wftools get ported. The IFF reader side is still unwritten — nothing in this session touches `iff/iffread.cc`.

## 9. References

- Companion: [`2026-04-10-worldfoundry-iffcomp-format.md`](../../2026-04-10-worldfoundry-iffcomp-format.md) — DSL grammar and binary layout reference
- Companion: [`2026-04-11-iffcomp-modernization.md`](2026-04-11-iffcomp-modernization.md) — C++ modernization (the oracle producer)
- Companion: [`2026-04-11-iffcomp-go-rewrite.md`](2026-04-11-iffcomp-go-rewrite.md) — Go port
- Companion: [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md) — broader rewrite-vs-retire analysis
- Rust source:
  - `wftools/iffcomp-rs/src/lexer.rs` — hand-rolled tokenizer
  - `wftools/iffcomp-rs/src/parser.rs` — recursive-descent parser
  - `wftools/iffcomp-rs/src/writer.rs` — binary + text IFF writer
  - `wftools/iffcomp-rs/src/lib.rs` — `compile()` entry point
  - `wftools/iffcomp-rs/src/main.rs` — CLI
  - `wftools/iffcomp-rs/tests/byte_exact.rs` — three differential tests
- Shared torture fixture: `wftools/iffcomp-{rs,go}/testdata/all_features.{iff.txt,iff}`
