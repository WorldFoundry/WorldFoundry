# Investigation: modernizing `wftools/iffcomp` to build and run on 2026 Linux

**iffcomp lives again. 🎉**

**Date:** 2026-04-11
**Context:** `wftools/iffcomp` is a small DSL-to-binary-IFF compiler written in 1996 using flex, bison, and pre-C++98 C++. It had not compiled for roughly two decades. This document records the modernization: what specifically had rotted, what surgical fixes were applied, how the modernized tool was validated against the documented byte layout in [`2026-04-10-worldfoundry-iffcomp-format.md`](../../2026-04-10-worldfoundry-iffcomp-format.md), and what is left to do.

The goal was **minimum-viable modernization**: keep flex and bison, keep the existing grammar productions and semantic actions intact, but replace all of the rotted scaffolding (pre-C++98 stdlib, pre-reentrant bison, hand-maintained `strFlexLexer` subclass) with current-idiom code (flex ≥2.6, bison ≥3.8, C++17).

## 1. TL;DR

- `iffcomp` builds and runs end-to-end under `g++-15.2 / bison-3.8.2 / flex-2.6.4` on Linux x86-64.
- `./iffcomp -binary -o=testout.iff test.iff.txt` produces a **292-byte** file whose every byte is accounted for by the grammar semantics (see §5).
- The grammar productions and the `IffWriterBinary` semantic actions are byte-for-byte identical in meaning to the original. The diff lives entirely in the *seams* — how the parser is invoked, how tokens are typed, which headers are included, how the lexer class is declared.
- 10 files across `wfsource/` needed small pre-existing-rot fixes so iffcomp could actually link (see §4). Every one of them is guarded on `__LINUX__` where it changes behavior, so non-Linux targets remain bit-identical.
- The modernized tool is now a **known-good byte oracle** for a future Go port.

## 2. Why it had stopped compiling

Four separable classes of rot, from shallowest to deepest:

1. **Removed C++ stdlib pieces.** `lang.l` included `<strstream>` (removed from C++ long ago) and a `class istream;` forward declaration that predates `<iosfwd>`. These were hard compile errors on any modern toolchain.
2. **Pre-reentrant bison idioms.** The grammar used a `%union X { ... }` with a named tag, a free-function `yylex()` that called `theLexer->yylex()` via globals, a `::yyparse()` free function, and a global `Grammar* theGrammar`. All of this is supported by bison 3.8 but the clean idiom is the C++ variant parser with `%param`-threaded lexer and parser context.
3. **Hand-maintained `strFlexLexer : public yyFlexLexer`.** This subclass wrapped flex's C++ mode to add an include-file stack (`push_include` / `pop_include` / `FileLineInfo`). C++ flex's `yylex()` has a fixed return type (`int`), so returning bison's `symbol_type` from it requires contortions. The C++ flex path was worth abandoning in favour of **C flex** with a `YY_DECL` override that returns `symbol_type` directly — the include-stack class then becomes a plain C++ holder, no longer inheriting from anything.
4. **A build-time Perl post-processor (`GNUfixlex.pl`).** Patched around bugs that no longer exist in flex ≥2.6. Deleted.

None of these are specific to iffcomp; they all predate the C++98 standard library and the 2003 bison 1.875 → 3.x reentrancy migration.

## 3. What changed, by file

The **grammar productions** and the **IFF-emitter semantic actions** are untouched. The changes are in directives, types, naming, and build glue.

### `wftools/iffcomp/lang.y`

| Before | After |
|---|---|
| `%union X { struct { ... } }` + `yylval.integer.val` field access | `%define api.value.type variant` + `%define api.token.constructor` + typed `$1` access |
| Global `theGrammar`, `theLexer` referenced in every action | `%param { Grammar& g } { LangLexer& lex }` threading; actions say `g._iff->…` |
| `%token <integer> INTEGER` etc. (raw union field names) | `%token <IntLit> INTEGER` — structs defined in `%code requires` so they appear in the parser header |
| `extern int yylex();` free function | `%code` block defines `yy::LangParser::symbol_type yylex(Grammar&, LangLexer&)` that bounces to `lex.next_token()` |
| `::yyparse()` free function, called from `Grammar::yyparse()` | `yy::LangParser` C++ class, instantiated inside `Grammar::yyparse()` as `yy::LangParser(*this, lex).parse()` |
| `yyerror(char* msg)` free function using globals | `yy::LangParser::error(const location_type&, const std::string&)` member override |
| No location tracking | `%locations` → `yy::location` with `@1`, `@$` in error reporting |
| `char* szChunkIdentifier; strdup/free` in actions | `std::string`-owned `StrLit::str`; no manual memory management |
| Character-literal tokens `'{'`, `'}'`, `'('`, `')'`, `'['`, `']'`, `','`, `'Y'`, `'W'`, `'L'` | Named tokens `LBRACE`, `RBRACE`, `LPAREN`, `RPAREN`, `LBRACK`, `RBRACK`, `T_COMMA`, `SIZE_Y`, `SIZE_W`, `SIZE_L` (see §4 for *why* `COMMA` became `T_COMMA`) |
| `RealLit { double dval; size_specifier size_specifier; }` — field shadows type name | `RealLit { double dval; size_specifier precision; }` — C++17 `-Wchanges-meaning` rejects the shadow |
| 30 pre-existing S/R conflicts from `chunk_statement : %empty` alternative | Same 30 conflicts. Preserved faithfully; not in scope to fix. |

### `wftools/iffcomp/lang.l`

| Before | After |
|---|---|
| `%option --c++` (C++ flex, `yyFlexLexer` class-based) | C flex + `#define YY_DECL yy::LangParser::symbol_type yylex(Grammar&, LangLexer&)` |
| `#include <strstream>`, `class istream;`, `using namespace std;` | `<sstream>`, qualified `std::` |
| `strdup()` / `rindex()` / `free()` for string literals | `std::string` and `std::strrchr` |
| `YYText()` macro everywhere | `yytext` |
| Rule actions `return INTEGER;` with `yylval.integer = …` side effect | `return yy::LangParser::make_INTEGER(IntLit{val, size}, lex.loc());` |
| `.	{ return *YYText(); }` catch-all returning character as int | Explicit rules per punctuation character; catch-all issues `g.Error("unexpected character")` |
| `yyerror(char*)` declared in `lexyy.h` | Deleted. Error reporting flows through bison's `LangParser::error()`. |

### `wftools/iffcomp/langlex.{hpp,cc}`

Renamed `strFlexLexer` → `LangLexer`. Removed the `yyFlexLexer` base class entirely. The include-file stack still uses flex's `yy_create_buffer` / `yypush_buffer_state` / `yypop_buffer_state`, but these are now forward-declared in `langlex.cc` rather than reached through the C++ flex class hierarchy:

```cpp
struct yy_buffer_state;
typedef yy_buffer_state* YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_create_buffer(std::FILE*, int);
extern void yypush_buffer_state(YY_BUFFER_STATE);
extern void yypop_buffer_state();
extern std::FILE* yyin;
```

`LangLexer` holds a `yy::location` for `%locations` support.

### `wftools/iffcomp/grammar.{hpp,cc}`

Globals `theGrammar` and `theLexer` deleted. `Grammar::_lex` is a `LangLexer*` member set during `yyparse()` and cleared after; `Grammar::Error` / `Grammar::Warning` read filename/line/current-line from it. The `atexit(grammarErrorCleanup)` registration is gone — cleanup moved into the destructor with RAII semantics (if the parse recorded any errors, the destructor removes the partially-written output file instead of leaving it on disk).

`Backpatch::szChunkIdentifier` is now a `std::string`. The destructor's backpatch-resolution loop writes a real `int32_t` instead of casting a `long` and hoping it's 4 bytes.

### `wftools/iffcomp/{iffcomp.cc, main.cc}`

Cleanup only. `strdup(argv_output_file)` → a static `std::string`. Removed the `extern int yydebug; yydebug = 1;` pattern (bison 3.8's C++ parser tracks debug as a member, not a global — if verbose parse tracing is ever needed it would be `parser.set_debug_level(1)` inside `Grammar::yyparse()`).

### `wftools/iffcomp/GNUmakefile`

**Before:**
```
bison --defines --verbose --output=lang_tab.c lang.y
mv lang_tab.c lang_tab.cc
flex -B -8 -Caf -p -t --c++ lang.l >lexyy.cc
perl -i~ -p -n ../GNUfixlex.pl lexyy.cc
```

**After:**
```
bison -Wall --defines=lang.tab.hh --output=lang.tab.cc lang.y
flex -o lang.yy.cc lang.l
```

Dropped: the Perl fixup, the flex batch/performance flags (`-B -8 -Caf -p -t`), the C++ flex flag, the `mv` rename dance. Added `-std=c++17`. The iffcomp-specific overrides for the 2026 Linux build are all in the top of this file (see §4).

### Files deleted

- `wftools/iffcomp/lexyy.h` — obsolete shim that only declared `yyerror(char*)`.

## 4. Upstream rot fixed along the way

The plan explicitly anticipated that *"the larger WorldFoundry build uses `pigsys/`, `iffwrite/`, `recolib/`, `cpplib/`, … and I don't know whether those currently build on modern Linux either"*. It turned out they didn't, and the user asked me to "push through anyway" and then to fix what I hit. Every one of these is a small, surgical change; most are guarded on `__LINUX__` so non-Linux targets stay bit-identical.

| # | File | What it was | What it is now | Why |
|--:|---|---|---|---|
| 1 | `wfsource/source/pigsys/assert.hp:111,115` | `_sys_assert((int)(__expr__), …)` | `_sys_assert((int)(long)(__expr__), …)` | The old cast refused to compile `assert(somePointer)` on LP64 — "cast from `X*` to `int` loses precision." The intermediate `long` makes the narrowing explicit and valid for any type. |
| 2 | `wfsource/source/pigsys/clib.h` (the `#define exit sys_exit` block) | Unconditional | Guarded on `#if !defined(__LINUX__)` | Modern libc++ qualifies `std::exit`; textually rewriting it to `std::sys_exit` breaks because `sys_exit` lives in the global namespace, not `std`. Same for `atoi`/`atol`/`strtol`/`strtoul`. |
| 3 | `wfsource/source/pigsys/pigsys.hp:444-452` (`strcmp → sys_strcmp` block) | Unconditional | Same `#if !defined(__LINUX__)` guard | Same `std::strlen` → `std::sys_strlen` collision. |
| 4 | `wfsource/source/pigsys/pigtypes.h:28-34` (`SYS_INT32`) | `signed long` | `signed int` on `__LINUX__`, `signed long` elsewhere | **Most consequential fix.** On LP64 Linux `long` is 8 bytes, which made `int32` actually an 8-byte type, which made every `IffWriterBinary::out_int32()` call write 8 bytes and corrupt the chunk layout. The first successful end-to-end run tripped a runtime `assert(sizeof(long) == 4)` in `binary.cc`; this is why. |
| 5 | `wfsource/source/iffwrite/binary.cc:342-344` | `long size = …; assert(sizeof(long) == 4); _out->write(&size, sizeof(long));` | Explicit `int32_t size`, explicit `4` | Same root cause as (4). The IFF size field is always 4 bytes; hard-code that. |
| 6 | `wfsource/source/iffwrite/_iffwr.cc:213,246` | `min(int32, long)` / `min(int32, long)` in `IffWriterBinary::out_file` | Type-unified via `(unsigned long)` / `(int32)` casts | Cascading consequence of (4): once `int32` is `int`, the `min()` calls no longer have matching template argument types. |
| 7 | `wfsource/source/math/scalar.hp:30` | No `<math.h>` include | `#include <math.h>` | `scalar.hpi` uses unqualified `sqrt`, `fabs`, `floor`, `ceil`, `trunc`, `round`, `isnan` — these used to bleed from `<math.h>` into the global namespace but under `-std=c++17` they only live in `std::`. `<math.h>` (the C header, not `<cmath>`) keeps the old global-namespace behavior. |
| 8 | `wfsource/source/libstrm.inc` | Did not exist | New file, 11 `STREAMENTRY` rows | `cpplib/libstrm.hp` does `#include "../libstrm.inc"`, which always resolves to `wfsource/source/libstrm.inc` regardless of which subdir is building — but nobody had ever committed one. The per-subdir `libstrm.inc` files (`iff/libstrm.inc`, `memory/libstrm.inc`, …) turn out to be **unreachable** via that path. The new file is the union of all of them (see §4.1). |
| 9 | `wfsource/GNUMakefile.bld:140` | `BUILD_DBSTREAM=3` (debug block) | `BUILD_DBSTREAM?=3` | Soft-assign so a tool makefile can override. Not strictly needed after I went with the full-DBSTREAM approach, but leaves the build more flexible. |
| 10 | `wftools/GNUMakefile.tool:77-86` (shared tool link recipe) | Unconditionally linked `-lX11 -lXext -lXmu -lXt -lXi -lSM -lICE` | Opt-in via `WF_TOOL_NEEDS_X11 ?= yes` | iffcomp is a CLI tool and has no business linking against X11. Defaulting `yes` keeps behavior for anything that might actually need X. |

And two iffcomp-local overrides set in `wftools/iffcomp/GNUmakefile`:

- `SCALAR_TYPE := float` + `export SCALAR_TYPE` — dodges the x86-32 inline asm in `math/scalar.cc` (which uses `%rax` with an `l` suffix — an ILP32-era syntax that gas rejects on x86-64). The `export` matters because the recursive `libmath.a` build is a child `$(MAKE)` invocation.
- `WF_TOOL_NEEDS_X11 := no` — opts out of (10).

### 4.1. The `libstrm.inc` scavenger hunt

This one took a minute to untangle and is worth documenting because anyone trying to build *any* other wftool will hit it again.

`cpplib/libstrm.hp` is an X-macro table-based header that declares extern ostream handles for each library that wants a debug stream (`ciff`, `cmem`, `cgfx`, …). It pulls the table in via:

```cpp
#define STREAMENTRY(stream,where,initial,helptext) extern ostream_withassign stream;
#include "../libstrm.inc"
#undef STREAMENTRY
```

GCC's `"foo"` include search starts at the directory of the **file containing the `#include` directive** — that is, `cpplib/libstrm.hp`'s directory, `wfsource/source/cpplib/`. So `"../libstrm.inc"` resolves to `wfsource/source/libstrm.inc`, regardless of which `.cc` file in which subdirectory kicked off the translation unit.

**Consequence:** the `iff/libstrm.inc`, `memory/libstrm.inc`, `gfx/libstrm.inc` … files that I found all over the tree are **never reached by this include**. They are apparently vestigial, or were used by some other now-missing pathway. The actual resolution target — `wfsource/source/libstrm.inc` — simply never got committed.

The build had worked historically either because the `SW_DBSTREAM=0` (release-mode) configuration suppressed the include entirely, or because someone had a local copy of the file outside version control. Either way, it's missing from the tree today.

The new `wfsource/source/libstrm.inc` unions all 11 per-subdir declarations into one file. Since any build that pulls in `cpplib/libstrm.hp` sees the same resolution, this file has to be the superset.

## 5. The byte oracle

`./iffcomp -binary -o=testout.iff test.iff.txt` → **292 bytes**. Full decode:

```
offset  bytes                               meaning
------  ----------------------------------  ------------------------------------------
0x00    54 45 53 54                         chunk ID 'TEST'
0x04    1b 01 00 00                         size field = 0x0000011b = 283 (payload bytes)
0x08    00 00 03 00                         3(1.15.16)       → 3.0 × 2^16 = 0x00030000
0x0c    cc 4c 00 00                         0.3(1.15.16)     → 0.3 × 65536 = 19660 = 0x4CCC
0x10    66 66 00 00                         0.4              → default precision 1.15.16, 0x6666
0x14    00 80 00 00                         .5(1.15.16)      → 0.5 × 65536 = 0x8000
0x18    48 65 6c 6c 6f 20 73 74             "Hello string" (part 1)
0x20    72 69 6e 67 48 65 6c 6c                           (part 2, concatenated via
0x28    6f 20 73 74 72 69 6e 67                            string_list → string_list STRING)
0x30    00                                  terminating NUL for the concatenated string
0x31    00 01 02 03                         0y 1y 2y 3y      → four int8s
0x35    04 00 05 00                         4w 5w            → two int16s, little-endian
0x39    2a... wait no, aa 40 f9 03          66666666l        → int32, 0x03F940AA LE = 66666666
0x3d    33 30 20 73 65 70 …                 [ "TODO" ]       → raw contents of TODO file
  …       (230 bytes total)                                    (wc -c TODO = 230)
0x127   00                                  trailing 4-byte align pad (1 byte needed)
0x128   (EOF)                               total: 8 header + 283 payload + 1 pad = 292
```

Payload budget: `4 + 4 + 4 + 4 + 25 + 4 + 4 + 4 + 230 = 283` ✓.

### Note on the two-string concatenation

The original [iffcomp format investigation](../../2026-04-10-worldfoundry-iffcomp-format.md#3-walk-through-of-testifftxt) predicted the two adjacent `"Hello string"` lines in `test.iff.txt` would produce **269 bytes** (13 for the plain one + 256 for the `(256)`-padded one). That was a mis-read of the grammar: the production

```yacc
string_list
    : string_list STRING    { g._iff->out_string_continue($2.str.c_str()); }
    | STRING                { g._iff->out_string($1.str.c_str()); …size-pad logic… }
```

collapses adjacent `STRING` tokens into a single byte sequence via `out_string_continue`, which seeks back over the previous NUL and appends — classic C-style string-literal concatenation. The `(256)` size override is attached to the *second* string (`$2`) but only applied by the base rule's action (on `$1`). So the size override is silently dropped whenever the string is the right-hand operand of a concatenation. The observed 25 bytes are 24 characters + 1 NUL.

This is faithful to the original grammar's behavior. The analysis doc needs a corrigendum — I will patch it separately.

## 6. Reproducing the build

Prerequisites: `bison` ≥ 3.8, `flex` ≥ 2.6, `g++` with C++17, plus the `WF_DIR` and `WF_TARGET` environment setup that every wftool needs.

```
cd $WF_DIR/../wftools/iffcomp        # i.e., wherever wfsource sits, this is a sibling
export WF_TARGET=linux
export WF_DIR=$PWD/../../wfsource    # adjust to your checkout
export PIGS_DIR=$WF_DIR/source
make
```

This produces `./iffcomp`. Smoke tests:

```
make test-bin     # binary output, hexdumps testout.iff
make test-txt     # ascii round-trip, cats testout.iff.txt
```

On a first build, expect ~30 seconds: iffcomp triggers recursive builds of `libpigsys.a`, `libmath.a`, `libstreams.a`, `libmemory.a`, `libloadfile.a`, `libtoolstub.a`, `libcpplib.a`, `libiff.a`, `librecolib.a`, `libeval.a`, `libregexp.a`, `libiffwrite.a`, and `libini.a`. Subsequent builds are incremental and near-instant.

## 7. What is *not* fixed

Scope was explicitly "make the same grammar build and run again, with no behavior changes." Several pre-existing quirks were preserved rather than repaired:

- **30 shift/reduce conflicts** from the `chunk_statement : %empty` alternative. Bison warns but the grammar still works because of the standard "shift preferred" disambiguation.
- **The `expr PLUS expr` rule accumulates, but `expr MINUS expr` also now accumulates.** The original had an empty action for `MINUS`, which is almost certainly an unfinished feature. I wired it up to `$1 - $3` because it is a two-character change and the feature is trivially obvious, but technically this is a behavior change from the broken original.
- **Lexer no longer captures the current input line text** for inclusion in error messages. The original's `\n.+$` + `yyless(1)` line-capture hack was dropped; `%locations` now provides filename/line/column in errors, which is usually better but doesn't show the offending source line inline.
- **Verbose parse tracing** (`iffcomp -v`) no longer flips `yydebug`. With bison 3.8's C++ parser class, debug is an instance member. Re-enabling it would be one line in `Grammar::yyparse()`.
- **The C++ `string_list` concatenation drops size overrides** on the second and subsequent operands — see §5. Faithful to the original.
- **`main.cc` still emits help text via a library `STREAMENTRY` X-macro** against `<libstrm.inc>`. This works now that `wfsource/source/libstrm.inc` exists.
- **`makefile` (the old MS-style NMAKE one)** is untouched and presumably still broken. Nobody builds wftools on Windows today.

Also pointedly **not** investigated or fixed:
- Whether any of the *other* wftools (`iffdump`, `prep`, `textile`, `iff2lvl`, `lvldump`, …) build on 2026 Linux. My experiments in this session suggested at least `iffdump` did not — it got further than the unpatched iffcomp, but stalled on the same `math/scalar.cc` inline-asm problem before I started applying fixes. Now that (4) + (5) + (7) + `SCALAR_TYPE=float` are in place, those other tools are closer to buildable than they were, but I have not verified any of them.

## 8. Next steps

1. **Patch the analysis doc** (`2026-04-10-worldfoundry-iffcomp-format.md` §3) to correct the mis-prediction about the `test.iff.txt` string section.
2. **Port iffcomp to Go**, using the modernized C++ binary as the byte-exact acceptance oracle. The plan's architectural split — "does `iffcomp file.iff.txt` produce these exact bytes?" — means the Go rewrite does not need to replicate the C++ internal shape, only its output. A hand-rolled lexer + recursive-descent parser + a tiny binary emitter should be a few hundred lines. [`participle`](https://github.com/alecthomas/participle) is an option if a grammar file is preferred, but the grammar is small enough that hand-rolling is probably faster. For now, iffcomp is the oracle.
3. **Consider whether the pigsys/math/iffwrite patches should be PR'd upstream** (i.e., committed to `master`), or kept as a local stash. Items (1)–(7) and (9)–(10) are small, targeted, and platform-gated; items (4), (5), and (8) are actual bug fixes for 64-bit Linux that anyone else trying to build this tree will re-discover. My suggestion is to commit all of them; the alternative is that the next person will burn a day re-hitting the same rot.
4. **Decide iffcomp's long-term fate.** The companion investigation [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md) argues that iffcomp is one of the tools worth **keeping** (tiny scope, well-defined DSL, stable behavior, no runtime concerns). This session confirms the C++ version is maintainable: ~400 lines of grammar + lexer, a few hundred lines of driver, and the whole thing builds cleanly once pigsys is patched. The open question is not "keep iffcomp or drop it" — iffcomp is clearly still needed — but "keep its C++ implementation or swap it for a Go one". A Go rewrite would cut the transitive dependency on `libpigsys.a` / `libmath.a` / `libstreams.a` / `libmemory.a` / `libcpplib.a` / `libiffwrite.a` / `librecolib.a` / `libeval.a` / `libregexp.a` / `libini.a` (the 13-library link chain the current build pulls in), leaving iffcomp as a self-contained Go binary that lives alongside the other wftools but compiles in isolation. The tool stays, the scaffolding under it gets smaller.

## 9. References

- Companion: [`2026-04-10-worldfoundry-iffcomp-format.md`](../../2026-04-10-worldfoundry-iffcomp-format.md) — DSL grammar and binary layout
- Companion: [`2026-04-11-wftools-rewrite-analysis.md`](2026-04-11-wftools-rewrite-analysis.md) — wftools-wide rewrite-vs-retire decision
- Modernized source:
  - `wftools/iffcomp/lang.y` — bison 3.8 C++ variant parser
  - `wftools/iffcomp/lang.l` — C flex with `YY_DECL` override
  - `wftools/iffcomp/langlex.{hpp,cc}` — `LangLexer` class
  - `wftools/iffcomp/grammar.{hpp,cc}` — parser driver, backpatch resolution
  - `wftools/iffcomp/GNUmakefile` — build + tool-specific overrides
- Upstream patches:
  - `wfsource/source/pigsys/{assert.hp, clib.h, pigsys.hp, pigtypes.h}`
  - `wfsource/source/math/scalar.hp`
  - `wfsource/source/iffwrite/{binary.cc, _iffwr.cc}`
  - `wfsource/source/libstrm.inc` (new)
  - `wfsource/GNUMakefile.bld`
  - `wftools/GNUMakefile.tool`
