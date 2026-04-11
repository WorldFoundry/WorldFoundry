# Investigation: Go rewrite of `wftools/iffcomp`

**Go is the primary now. C++ is the oracle.**

**Date:** 2026-04-11
**Context:** Immediately after the C++ modernization documented in [`2026-04-11-iffcomp-modernization.md`](2026-04-11-iffcomp-modernization.md), this session replaces `wftools/iffcomp`'s C++ flex/bison implementation with a hand-rolled Go port. The new version lives side-by-side with the original under `wftools/iffcomp-go/` and is validated against the modernized C++ binary as a byte-exact reference oracle. Both the binary (`IffWriterBinary`) and text (`IffWriterText`) output modes are covered, plus the `-v` verbose parse-trace option.

This document captures what was built, the two bring-up bugs, how the differential test harness is structured, and why the plan is to use Go as the primary while keeping C++ around as an oracle for the format corners the Go port hasn't yet exercised.

## 1. TL;DR

- **Binary output:** byte-exact against the C++ reference — 292 bytes for `test.iff.txt`, every byte decoded and explained in §5 of the prior investigation.
- **Text output:** byte-exact against `IffWriterText` — 1214 bytes including the ` //\n<tabs>` line-wraps every 100 bytes inside the `out_mem` file-include path.
- **Verbose mode:** rule-entry parse trace on stderr when `-v` is passed. Not identical to bison's `yydebug` shift/reduce trace, but serves the same "what did the parser decide at each step" debugging purpose.
- **~2250 lines of Go**, zero external dependencies (stdlib only), 110 ms `go build`, 2.7 MB static binary. No `libpigsys.a`, no `libmath.a`, no `libiffwrite.a`, no `libcpplib.a` — the 13-library transitive link chain the C++ version pulls in is entirely gone.
- **Two passing Go tests**, both byte-exact diffs against artifacts produced by the modernized C++ `iffcomp`: `TestByteExactAgainstCpp` (binary) and `TestTextOutputAgainstCpp` (text). Green on first post-bring-up run.
- **Side-by-side with the C++ version.** `wftools/iffcomp/` (C++) and `wftools/iffcomp-go/` (Go) coexist. The plan is to make Go the primary while keeping C++ as the reference oracle — see §6.

## 2. Structure

```
wftools/iffcomp-go/
├── go.mod              module decl, no deps
├── main.go          67 lines  CLI: -o=file, -binary|-ascii, -v, -q
├── iffcomp.go       56 lines  Compile() library entry point + Options
├── lexer.go        817 lines  hand-rolled tokenizer
├── parser.go       534 lines  recursive descent, mirrors lang.y
├── writer.go       615 lines  binary + text emitters (shared struct, mode flag)
├── iffcomp_test.go 163 lines  byte-exact diff tests vs C++ reference
└── testdata/
    ├── test.iff.txt          copied from wftools/iffcomp/
    ├── TODO                  230-byte file referenced by `[ "TODO" ]`
    ├── expected.iff          292-byte C++ IffWriterBinary reference
    └── expected.iff.txt     1214-byte C++ IffWriterText reference
```

The production ordering in `parser.go` maps one-for-one to `lang.y`:

| `lang.y` production | `parser.go` function |
|---|---|
| `statement_list` | `Parse()` |
| `chunk` | `parseChunk()` |
| `chunk_statement` | `parseChunkStatement()` |
| `alignment` | `parseAlignment()` |
| `fillchar` | `parseFillChar()` |
| `expr` | `parseExpr()` |
| `expr_list` | `parseExprList()` |
| `item` | `parseItem()` |
| `state_push` | `parseStatePush()` |
| `string_list` | `parseStringList()` |
| `'[' STRING extractSpec* ']'` | `parseFileInclude()` |
| `extractSpec` | `parseExtractSpec()` |
| `chunkSpecifier` | `parseChunkSpecifier()` |
| `.offsetof(...)` | `parseOffsetof()` |
| `.sizeof(...)` | `parseSizeof()` |

`chunk` vs. `state_push` disambiguation (both start with `{`) is handled with 2-token lookahead: if the token after `{` is `CHAR_LITERAL`, it's a chunk; otherwise it's a state push. This is the same lookahead the bison grammar implicitly relies on.

The **writer is a single struct with a `text bool` mode flag**, and every emit method branches on it internally:

- Binary mode: maintains a `[]byte` buffer, a logical position, a chunk stack with `sizeFieldPos`, and a `map[string]chunkSym` symbol table keyed by `::'A'::'B'` paths. Back-patches for `.sizeof` / `.offsetof` forward references are recorded during the parse and resolved at end-of-parse by `ResolveBackpatches()`.
- Text mode: maintains a `bytes.Buffer` and a chunk depth counter. Each emit method formats its argument the way `IffWriterText` would — ints as `<n>y` / `<n>w` / `<n>l`, reals as `<%#.16g>(<S>.<W>.<F>)`, strings as `"..."` with `\n` producing a `\n"` + newline + indent + `"` line-break, chunk boundaries as `\n<tabs>{ 'ID'` and `\n<tabs>}`. File-include bytes are unrolled as individual `<n>y` with a ` //\n<tabs>` wrap every 100 bytes, exactly matching `out_mem`.

The one-struct-with-a-flag approach is slightly dirtier than splitting into two separate types behind an interface, but it's short-term cheap and keeps the parser code polymorphism-free.

## 3. Bring-up bugs

Two lexer bugs, both disambiguation failures, both caught by `TestByteExactAgainstCpp` on the first run:

### 3.1. `3(1.15.16)` tokenizes as REAL, not INTEGER

The C++ flex regex for numbers is a single pattern that handles **both** "real with optional precision" *and* "bare integer with optional precision" under the same rule:

```
-?(([0-9]*(\.[0-9]+)([eE][+-]?[0-9]+)?)|([0-9]+)){1}(\([0-9]+\.[0-9]+\.[0-9]+\))?
```

Reading that: an optional sign, then either a dotted real (with optional exponent) *or* a bare integer, and then optionally a `(sign.whole.fraction)` precision override. Flex picks the longest match, so `3(1.15.16)` matches the full 10-character pattern as a REAL with integer mantissa `3` and explicit precision `1.15.16`, not as an INTEGER followed by a LPAREN. My initial Go lexer had an integer path and a real path, and the integer path didn't know about the trailing `(N.N.N)` form.

**Fix:** after parsing digit-only mantissa, peek for `(`. If it's followed by a well-formed triple and a `)`, reclassify the token as REAL with that precision override. Otherwise roll back and emit INTEGER. The rollback logic saves offset/line/col and restores them on mismatch. (`lexer.go` has the same rollback pattern for the speculative precision-triple form `N.N.N` used as the argument of `.precision(...)`.)

### 3.2. `.5` is a real number, not a `.keyword` directive

My top-level scan dispatcher was routing everything starting with `.` to the dot-keyword path (`.timestamp`, `.align`, `.offsetof`, `.sizeof`, `.fillchar`, `.start`, `.length`, `.precision`). `.5` — a real number with a leading fractional part — hit that path and tried to look up `5` in the keyword table, failing.

**Fix:** in the top-level `scan()` dispatcher, if the byte after `.` is a digit, route to `scanNumber` instead of `scanDotKeyword`. One-line change.

Both fixes landed on the first iteration and `TestByteExactAgainstCpp` has been green ever since.

## 4. Text mode (IffWriterText) — one additional wrinkle

The C++ `_IffWriter` base class constructor pushes a sentinel `ChunkSizeBackpatch` onto `chunkSize` at instantiation time, so `chunkSize.size()` is always `1 + actual_depth`. Every indent formula in `iffwrite/text.cc` then uses `tab(chunkSize.size())` for `{` and `}` and `tab(chunkSize.size()+1)` for the 100-byte wrap marker — all built around that off-by-one.

When I first ran `TestTextOutputAgainstCpp` it failed on tab-count mismatches in the wrap markers. The fix was to understand the off-by-one and bake it into the Go helpers directly:

- Chunk `{`: `\n<depth-tabs>{` where depth is **before** the new chunk is pushed (0 at outermost → no leading tabs).
- Chunk `}`: `\n<depth-tabs>}` where depth is **after** the chunk is popped (0 at outermost close → no leading tabs).
- `out_file` wrap marker: ` //\n<(depth+1)-tabs>` where depth is the current chunk depth (1 inside a single TEST chunk → 2 tabs).

With that right, the text output became byte-exact against the C++ reference.

Other text-mode fidelity points worth noting:

- **`out_string_continue` re-emits as a second quoted literal** in text mode, matching the C++ implementation (no seek-back-over-NUL trick like the binary version does). So `"Hello string" "Hello string"(256)` appears as two separate quoted strings in the text output, while the binary output concatenates them into one 25-byte blob. Both match the C++ implementations of each writer.
- **`%#.16g` is Go's closest match to `setprecision(16) + showpoint`** from C++ iostreams. `#` in the fmt flag keeps trailing zeros and forces the decimal point; `.16g` picks 16 significant digits. Together they produce `3.000000000000000`, `0.3000000000000000`, `0.5000000000000000` — exactly the same byte-for-byte as the C++ output for this test file.
- **`.sizeof` / `.offsetof` in text mode don't back-patch.** Text output is append-only; there's no random-access position to patch. Forward references emit zero. If the format ever grows a test fixture that uses forward sizeof/offsetof in text mode, this will need revisiting — but the current test corpus doesn't exercise that corner.

## 5. Line count comparison

| Component | C++ lines | Go lines |
|---|---:|---:|
| grammar | `lang.y` 475 | `parser.go` 534 |
| lexer | `lang.l` 225 | `lexer.go` 817 |
| writer (binary + text) | `iffwrite/{_iffwr,binary,text,fixed}.cc` ~900 | `writer.go` 615 |
| driver | `grammar.cc`/`grammar.hpp`/`langlex.cc`/`langlex.hpp`/`iffcomp.cc`/`main.cc` ~720 | `main.go` 67 + `iffcomp.go` 56 = 123 |
| tests | none | `iffcomp_test.go` 163 |
| **total (excl. tests)** | **~2320** | **~2140** |

The Go version is slightly smaller than the C++ one even though the lexer grew considerably — the parser/driver/writer collapsed from ~2100 lines of C++ plus shared `iffwrite/` dependency into ~1320 lines of self-contained Go. The lexer grew because flex's regex-based state-machine compression hid a lot of complexity behind a small DSL; hand-rolling it is more explicit per-case.

And the **shared substrate** the C++ version depends on — all ~900 lines of `iffwrite/` across four files — is effectively subsumed into `writer.go`'s 615 lines. The Go lexer + parser + writer + driver + tests together compile in 110 ms with zero external libraries. The C++ version required 13 internal libraries (`libpigsys.a`, `libmath.a`, `libstreams.a`, `libmemory.a`, `libloadfile.a`, `libtoolstub.a`, `libcpplib.a`, `libiff.a`, `librecolib.a`, `libeval.a`, `libregexp.a`, `libiffwrite.a`, `libini.a`), a recursive make over `wfsource/source/`, 10 upstream patches (see the [C++ modernization investigation](2026-04-11-iffcomp-modernization.md#4-upstream-rot-fixed-along-the-way)), and about 30 seconds of cold build time.

## 6. Side-by-side differential test harness (current plan)

**Keep both implementations.** `wftools/iffcomp/` (the modernized C++ version from the previous investigation) and `wftools/iffcomp-go/` (this one) coexist in the tree.

**Go is the primary going forward.** For anything that currently shells out to `iffcomp` — build scripts, the `.oas` schema-codegen pipeline under `wfsource/source/oas/` that feeds `prep` + `iffcomp` to produce OAD descriptors, any test fixtures — the Go version should be the one invoked. Single static binary, no toolchain dependencies, fast rebuild, modern error messages.

**C++ stays as the oracle** for format corners the Go port hasn't exercised yet:

- **OAD descriptors emitted by the `.oas` pipeline.** `prep` compiles `.oas` source against `.s` templates to produce `.iff.txt` files that are then fed to `iffcomp` to produce the on-disk OAD binary. The OAD format uses multi-level nested chunks, mandatory `.sizeof` / `.offsetof` back-patches, and structured per-type sub-chunks (`NAME`, `DSNM`, `RANG`, `DATA`, `DISP`, `HELP`). `test.iff.txt` exercises none of that — the Go version has reasonable-looking code paths for all of it but nothing has actually proven they produce byte-exact output against the C++ version. Every `.oas` output file in the tree is a free regression fixture if we commit it as testdata.
- **Level files from the level build path.** Same story: big complicated IFF trees that the Go code *should* handle but hasn't been tested on.
- **`wfsource/source/iffwrite/test.scr`**, a two-chunk nested file I documented the expected byte layout for in [`2026-04-10-worldfoundry-iffcomp-format.md`](../../2026-04-10-worldfoundry-iffcomp-format.md#4-walk-through-of-iffwritetestscr) but didn't wire into the test harness. This is the simplest next fixture.

**Test harness structure.** Go tests drive it. `iffcomp_test.go` already has the shape: for each fixture, compile it with the Go implementation, read the reference output from `testdata/`, diff byte-exactly. Adding a new fixture is:

1. Check the fixture into `wftools/iffcomp-go/testdata/foo.iff.txt`.
2. Run the C++ `iffcomp` once to produce `testdata/foo.iff` (and `foo.iff.txt` if you want the text-mode comparison too).
3. Add a sub-test that compiles `foo.iff.txt` with the Go port and diffs against `testdata/foo.iff`.

If the Go port diverges, the test fails loudly with the exact offset and hex context of the first mismatching byte.

**When the C++ version can retire.** Once the Go port has been exercised against enough real production fixtures that we're confident it covers every format corner the project needs — OAD descriptors, level files, any other `.iff.txt` source in the tree — the C++ version can be deleted. Until then, it stays as the ground-truth reference. The C++ modernization from the previous session bought the right to keep running it whenever a new fixture appears.

## 7. Current test coverage

| Test | Mode | Fixture | Bytes | Status |
|---|---|---|---:|:---:|
| `TestByteExactAgainstCpp` | binary | `test.iff.txt` | 292 | ✓ |
| `TestTextOutputAgainstCpp` | text | `test.iff.txt` | 1214 | ✓ |

What `test.iff.txt` exercises:

- Chunk framing (`'TEST'` header)
- Four fixed-point reals with the default (`0.4`) and explicit (`1.15.16`) precision
- Two adjacent strings — binary mode concatenates via `out_string_continue`, text mode emits them as separate literals
- Explicit-width integers (`0y 1y 2y 3y`, `4w 5w`, `66666666l`)
- File inclusion (`[ "TODO" ]`) — 230 bytes of raw payload in binary mode, 230 unrolled `<n>y` items in text mode with the 100-byte wrap marker

What `test.iff.txt` **does not** exercise and therefore the Go port isn't validated against:

- Nested chunks at more than one level of depth
- `.sizeof` / `.offsetof` back-patches (both immediate and forward-reference)
- `.align(N)` / `.fillchar(N)` directives
- `.timestamp`
- Hex integer literals (`$DEADBEEF`)
- Negative number literals
- String escape sequences (`\n`, `\t`, `\\`, `\"`, `\DDD`)
- The `Y`/`W`/`L` state-push blocks
- `state_push` inside a concatenated string context
- `include "file"` and `include <file>` directives

Every one of these is a known-thin area. Most have plausible-looking Go code but zero test coverage. Adding any fixture that touches one of them — especially the `.sizeof`/`.offsetof` path, which is the most complex part of the writer — is highly valuable.

## 8. Next steps

1. **Wire `wfsource/source/iffwrite/test.scr` into the test harness.** Two nested chunks, documented expected bytes in the format investigation. First new fixture should take ~10 minutes.
2. **Add a fixture that exercises `.sizeof` / `.offsetof`.** Either find one in the existing `.iff.txt` files emitted by `prep`, or hand-write a small one. This is the highest-value coverage gap.
3. **Add fuzzing** (`go test -fuzz`) over the lexer. The speculative-rollback paths in `scanNumber` and the string-literal `(N)` size-override parser are the most likely places for subtle regressions to hide.
4. **Commit real `.oas`-derived `.iff.txt` files from `wfsource/source/oas/`** as testdata. Every one is a free differential regression test.
5. **Update `wftools/GNUmakefile` to drive the Go build** once the test coverage feels solid enough to cut the C++ version as the primary entry point. The C++ version stays on disk and can be rebuilt on demand via its own makefile.
6. **Figure out what to do about `iffwrite`.** The Go writer's `bytes`/`buffer` approach and its text-mode output are comfortable stand-ins for `IffWriterBinary` / `IffWriterText`, but they're tied to the iffcomp process today. If the ambition is to eventually rewrite more wftools in Go (per [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md)), `writer.go` should probably be lifted into a reusable `worldfoundry-iff` package. Not today, but worth flagging.

## 9. References

- Companion: [`2026-04-10-worldfoundry-iffcomp-format.md`](../../2026-04-10-worldfoundry-iffcomp-format.md) — DSL grammar and binary layout
- Companion: [`2026-04-11-iffcomp-modernization.md`](2026-04-11-iffcomp-modernization.md) — C++ modernization (the oracle this port was validated against)
- Companion: [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md) — broader wftools rewrite-vs-retire analysis
- Go source:
  - `wftools/iffcomp-go/lexer.go` — hand-rolled tokenizer
  - `wftools/iffcomp-go/parser.go` — recursive-descent parser
  - `wftools/iffcomp-go/writer.go` — binary + text IFF writer (mode-flagged single struct)
  - `wftools/iffcomp-go/iffcomp.go` — `Compile()` library entry point
  - `wftools/iffcomp-go/main.go` — CLI
  - `wftools/iffcomp-go/iffcomp_test.go` — byte-exact differential tests
- Test fixtures: `wftools/iffcomp-go/testdata/{test.iff.txt, TODO, expected.iff, expected.iff.txt}`
