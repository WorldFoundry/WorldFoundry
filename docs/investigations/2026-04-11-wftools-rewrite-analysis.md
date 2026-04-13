# Investigation: rewriting (or retiring) `wftools/`

**Date:** 2026-04-11
**Context:** Code review of `wftools/` — the C++ build pipeline that ships with the [World Foundry](https://github.com/wbniv/WorldFoundry) 3D engine. The directory holds ~23 tool subdirectories totalling ~40 kLOC of mostly mid-1990s C++. The `CHANGELOG` was last touched in March 1999 and the directory has been in maintenance-only mode since. This document answers three questions:

1. What language(s) would I rewrite these tools in today?
2. Which tools should be **dropped** in favour of an off-the-shelf replacement?
3. Which should be **replaced by modifying or adopting an existing tool** rather than maintained in-tree?

The companion investigation `WorldFoundry/2026-04-10-worldfoundry-iffcomp-format.md` documents the binary IFF dialect that most of these tools read or write — I won't re-derive that here.

## 1. What's actually built today

The top-level `wftools/GNUmakefile:36-44` only descends into 8 of the ~23 tool directories:

```
DIR_LIST = eval iffdump oaddump prep lvldump iffcomp iff2lvl textile
```

Everything else has been commented out, sometimes with annotations:

- `wftools/GNUmakefile:33` — `xcd \    // ncd clone, since OpenCD and wcd exist, abandoned`
- `wftools/GNUmakefile:90-91` — `## Not built anymore?` then `stripper`
- `wftools/CHANGELOG:16-21` (1999-03-20) — `disr3000`, `vrml2iff`, `link`, and `stripper` are explicitly flagged as *"not abandoned, but currently not upgraded due to missing files or not used"*; `chargrab`, `imagedir`, `wfbind`, `aicomp`, `attribedit` were Linux-incompatible at the time and were never resurrected.

A parallel NMAKE-style `wftools/makefile` (lower-case, BSD-make `%foreach` syntax) lists a wider DIR_LIST including `chargrab`, `imagedir`, `psxprof`, `reg`, `wfbind`, `xcd`. Comments in `wftools/makefile:27-46` explicitly file away the dead wood:

- *"Old DOS/Windows tools that we don't need anymore"* — `sendmail`, `delay`, `touch`, `lc`, `reg`, `tail`, `mktree`
- *"Probably obselete, but need to investigate further"* — `makedep`
- *"Not used anymore, but not abandoned"* — `vabdump`
- *"Needs major work before it's useful"* — `vrml2iff`, `link`
- `#disr3000  [ need to find dumpobj.cc ]`

The honest snapshot: 8 tools build on Linux, ~6 are *"keep around but don't compile"*, and another half-dozen have been dead-letter for at least 25 years.

## 2. Tool inventory

| Tool | LoC | Lang | Built (GNU)? | What it does |
|---|---:|---|:---:|---|
| **iff2lvl** | ~8.7k | C++ | yes | 3DSMax `.lev` (IFF) → runtime level format. README: *"the code base is a mess. A lot of cruft has accumulated, and it really needs a re-write."* |
| **chargrab** | ~5.0k | C++ | no | Sprite extractor (TGA/BMP/SGI → IFF/RMUV) |
| **attribedit** | ~4.7k | C++/gtkmm | no | Schema-driven OAD attribute editor. The standalone GUI is one host of OAD-editing logic that is also (re)implemented in the 3DSMax plugin `wfmaxplugins/attrib/` and elsewhere — see §3.1. |
| **aicomp** | ~3.5k | C++ + flex/bison | no | AI-script compiler — supports *both* a C-like and a Scheme-like input grammar |
| **textile** | ~4.3k | C++ | yes | Texture atlas packer (TGA/BMP/SGI → palette + UV maps) for PSX/Win/Linux |
| **link** | ~2.1k | C++ | no Makefile | R3000 (PSX) MIPS object linker |
| **lvldump** | ~2.0k | C++ | yes | `.lvl` file dumper |
| **prep** | ~1.9k | C++ | yes | Custom macro/template processor (despite the name, *not* a C preprocessor) — drives the `.oas` schema-codegen pipeline. See §4.3. |
| **disr3000** | ~1.7k | C++ | no | R3000 disassembler — needs `dumpobj.cc` which is missing |
| **iffcomp** | ~1.4k | C++ + flex/bison | yes | IFF text DSL → binary IFF (see iffcomp investigation) |
| **iffdb** | ~1.1k | C++ | no | In-memory IFF chunk-tree library |
| **stripper** | ~910 | C++ | no | Triangle-strip generator for IFF render objects |
| **psxprof** | ~674 | C++ | no | Sample-based PSX profiler → HTML report |
| **iffdump** | ~646 | C++ | yes | IFF binary → annotated hexdump |
| **oaddump** | ~616 | C++ | yes | OAD binary → text dump |
| **imagedir** | ~351 | C++ | no | HTML gallery generator for image folders |
| **xcd** | ~189 | C++ | abandoned | Directory bookmark CLI (`ncd`/`wcd` clone) |
| **reg** | ~105 | C++ | Win-only | HKLM registry read/write |
| **wfbind** | ~64 | C++ | no | Append data + size tail to a `.exe` |
| **eval** | 53 | C++ | yes | CLI test harness for the `pigs/libeval` runtime expression evaluator (`wfsource/source/eval/`) — the library that the OAD attribute editor invokes at runtime to evaluate `szEnableExpression` predicates from `.oas` schemas (see §3.1, §8.6). The wftools binary is byte-for-byte the same code as `wfsource/source/eval/evaltest.cc` (yet another duplication-problem instance). Also has `e = 2.17` hardcoded as a "constant" in its symbol-lookup callback — see `wftools/eval/eval.cc:31`. |
| **psxdisc** | — | Perl | n/a | `raw2iso.pl` strips 2352-byte raw sectors → 2048-byte ISO |
| **perl.r** | 51 | Perl | n/a | Recursive case-fixing renamer |
| **bin/** | — | data | n/a | Holds `bison.simple` / `bison.hairy` parser templates |

LoC counts are from `wc -l` over each tool's `.cc` / `.cpp` / `.l` / `.y` files. Spot-checks: `iff2lvl` 8731, `textile` 4263, `aicomp` 3508.

## 3. The shared substrate

Almost every C++ tool links a stack of WorldFoundry libraries from `wfsource/source/`, collectively known as PIGS: `pigsys`, `streams`, `memory`, `loadfile`, `iffwrite`, `iff`, `cpplib`, `math`, `recolib`, `eval`, `regexp`, `toolstub`. See `wftools/GNUMakefile.tool:40-50` for the exact link line. Two libraries do most of the work:

- **`wfsource/source/iff/`** — `iffread` for reading the WF IFF dialect (little-endian payload, FOURCC IDs, 4-byte alignment between siblings).
- **`wfsource/source/iffwrite/`** — `IffWriterBinary` for writing it.

This matters for the rewrite question: extracting *one* tool from the PIGS dependency nest is non-trivial. A clean rewrite needs to bring the IFF reader and writer along as a first-class library before touching individual tools.

### Build-system tax

- Two parallel makefile worlds: GNU Make (`GNUmakefile` + `GNUMakefile.tool`, the live one) and a stale `%foreach`-style `makefile` (the legacy Windows one). Most tools carry both `GNUmakefile` and `makefile` per-tool too.
- `wftools/fixlex.pl` post-processes flex output to swap C `malloc`/`free` for C++ `new`/`delete` and patch the `<FlexLexer.h>` include — a classic flex-vs-modern-C++ workaround that survives only because flex never grew a clean C++ output mode:

  ```perl
  s/malloc\( size \);/new char[size];/;
  s/free\( ptr \);/assert\(ptr\); delete [] ptr;/;
  ```

- There are **two** flex/bison combos in the live build path: the IFF text DSL grammar in `wftools/iffcomp/lang.y`, and the `szEnableExpression` arithmetic-predicate grammar in `wfsource/source/eval/expr.y` (used by the runtime expression evaluator that the OAD editor consumes — see §4.3 eval row and §8.6). `aicomp` would re-introduce *two* more grammars if it ever came back.

### 3.1 The duplication problem

The build-system tax is the symptom; the deeper problem is that the **core asset formats have no shared library**. OAD-handling code in particular lives in at least six different places:

| Location | What it implements |
|---|---|
| `wftools/attribedit/oad.{cc,hp}` + `oadesciffbinparser.{cc,hp,hpi}` + `parsetreenode.{hp,hpi}` + `parser.hp` | OAD schema parser + in-memory model + parse-tree infrastructure (the richest implementation; backs the GTK editor) |
| `wftools/iff2lvl/oad.{cc,hp}` | OAD reader for the level importer — the file with the `#pragma message ("KTS … this fails sometimes, figure out why")` from §6 |
| `wftools/lvldump/oad.{cc,hp}` | OAD reader for the level dumper |
| `wftools/oaddump/` | OAD reader for the standalone dumper |
| `wfmaxplugins/attrib/oad.{cc,hpp}` + `oaddlg.{cc,h}` | OAD model + dialog for the 3DSMax plugin |
| `wfmaxplugins/oad2txt/oad.h` + `scene.{cc,hpp}` | OAD model for the Max txt-export plugin |
| `wftools/eval/eval.cc` ≡ `wfsource/source/eval/evaltest.cc` | **Byte-for-byte identical** test harness for the runtime expression evaluator (modulo the GPL header on the wftools copy). Same `SymbolLookup`, same `main`, same `e = 2.17` bug. The wftools copy was vendored from the library tree at some point and the two never re-converged. |

`wftools/attribedit/attribedit.hp:75` even contains the comment `// kts this is a kludge, design a better way`.

Worse, `wfmaxplugins/lib/` is a *copy-paste* of half of `pigs2`: `_iffwr.cc`, `binary.cc`, `fixed.cc`, `loadfile.cc`, `dbstrm.hpp`, `stdstrm.cc`, `hdump.cc`, `assert.cc`, `pigtool.h`, etc. — vendored because the 3DSMax plugin SDK couldn't link directly against the engine's library tree. Every host that has ever needed to read or write OAD has reimplemented or vendored the format.

The structural lesson: the *core asset formats* (IFF, OAD, the level format) belong in standalone, embeddable libraries that any host — a CLI dumper, a standalone GUI app, a 3D-editor plugin, the engine runtime itself — links against. The "keep + rewrite" list in §4.3 is really a *library extraction* exercise as much as a tool rewrite.

## 4. Three buckets

### 4.1 Drop entirely (off-the-shelf replacement exists)

| Tool | Replacement | Notes |
|---|---|---|
| **xcd** | `zoxide`, `fasd`, `autojump`, `wcd` | Already noted as abandoned in `wftools/GNUmakefile:33` because `wcd` and `OpenCD` exist. `zoxide` is the modern winner. |
| ~~eval~~ | — | **Removed from this list.** `eval` turned out to be the CLI test harness for the `wfsource/source/eval/` runtime expression evaluator that the OAD attribute editor uses to evaluate `szEnableExpression` predicates. Moved to §4.3. See §8.6 for how option (a)/(b) of the prep decision affects it. |
| **imagedir** | `tree`, ImageMagick `montage`, any HTML gallery generator (`sigal`, `thumbsup`) | Generating an HTML gallery from a folder of TGAs is solved territory. |
| **perl.r** | `find -exec rename`, `mmv`, `prename`, `rnr`, or 30 lines of Python | The whole script is recursive lower-casing with a special case for `Makefile`/`CHANGELOG`/`TODO`. |
| **wfbind** | `objcopy --add-section`, or just ship the data file alongside the executable | Appending `<size, data, tag>` to a Windows `.exe` was a 1990s "data fork" hack. On a modern OS the data sits next to the binary; if you really want it inside, `objcopy --add-section .gamedata=foo.bin` does it without 64 lines of `windows.h`. |
| **psxdisc** | dropped (PSX no longer targeted) | `raw2iso.pl` strips raw PSX sectors to ISO. Part of the PSX toolchain that's being dropped wholesale — see the note below. |
| **disr3000** | dropped (PSX no longer targeted) | R3000 (PSX MIPS) disassembler. Already broken (`dumpobj.cc` missing per `wftools/CHANGELOG:18`); now also obsolete by virtue of the platform going away. |
| **link** | dropped (PSX no longer targeted) | R3000 (PSX MIPS) object linker. Has never had a working Makefile per `wftools/CHANGELOG:20`; the platform it links for is gone. |
| **psxprof** | dropped (PSX no longer targeted) | Sample-based PSX hardware profiler. Same reason. |
| **reg** | PowerShell, `reg.exe`, `regedit /s` | Windows-only HKLM tool — not on the Linux build path at all. Windows already ships three different registry CLIs. |

**On PSX support:** four of the rows above (`psxdisc`, `disr3000`, `link`, `psxprof`) exist solely to support PlayStation 1 as a target platform. PSX is no longer a target — the whole PSX toolchain is being dropped, not replaced. If PSX support is ever revived (paid work, like the Max/Maya story in §4.3), the modern hobbyist ecosystem covers everything these in-tree tools used to do, and is far better maintained: [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) for the SDK and toolchain, [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) for disc images, `mips-linux-gnu-objdump`/[radare2](https://rada.re/n/)/Ghidra for disassembly, `mips-elf-ld` (binutils) or LLVM `lld` for linking, and PCSX-Redux's built-in profiler for sample profiling.

(`prep` was originally on this list as "replace with `cpp` / `m4`" — turns out it isn't a C preprocessor at all; it's a custom macro/template processor that drives the `.oas` schema-codegen pipeline. Moved to §4.3. `eval` was also originally on this list as "replace with `python -c` / `bc`" — turns out it's the CLI test harness for the runtime expression evaluator that the OAD attribute editor invokes at runtime; also moved to §4.3, see §8.5 for the cross-cutting impact.)

**Net delete:** ~5 kLoC of in-tree C++ (the four PSX tools plus the unrelated drops above) + two Perl scripts + the entire DOS-era catalog (`sendmail`, `delay`, `touch`, `lc`, `tail`, `mktree`, `vabdump`, `makedep`) that's already commented out of `wftools/makefile`. (`prep` and `eval` were originally counted here, but moved to §4.3 — net delete shrinks accordingly.)

### 4.2 Replace by modifying / adopting an existing tool

These are engine-flavoured wheel-reinventions where a mature OSS project does the same job better. The "modification" is usually a config file, a linker script, or a small wrapper.

| Tool | Adopt | Modification needed |
|---|---|---|
| **stripper** | [meshoptimizer](https://github.com/zeux/meshoptimizer) | Single-header C++, MIT-licensed. `meshopt_stripify()` replaces `wftools/stripper/stripper.cc` and ships a better algorithm. The right place for it is *inside* `iff2lvl` — strip on import, not as a post-process. |
| **aicomp** | Embed Lua (or Wren, AngelScript, MicroPython) | The reason `aicomp` carries *two* grammars (`c.l`/`c.y` and `scheme.l`/`scheme.y`) is that nobody in 1996 could agree on a syntax for the AI scripting layer. In 2026 you don't write a script compiler — you embed an interpreter. Lua is small, MIT, and has been the videogame scripting answer for 20 years. The bytecode-emit half goes away; the interpreter *is* the script. |

(`attribedit` was originally on this list — the right framing turned out to be library-extraction, not GUI-replacement; see §4.3. The PSX-specific tools `disr3000`, `link`, `psxprof`, `psxdisc` were also originally on this list — they're now in §4.1 because PSX is no longer a target platform at all.)

**Net delete:** ~4.4 kLoC of in-tree C++ replaced by two small adoptions of third-party tools.

### 4.3 Keep — but rewrite

These are genuinely engine-specific. There is no off-the-shelf "WorldFoundry IFF level packer" because the format is unique to this engine. They have to live in the project. The question is *what language*.

| Tool | LoC | Why keep |
|---|---:|---|
| **iffcomp** | ~1.4k | Authoring DSL → binary IFF tree. Engine-specific format. |
| **iffdump** | ~646 | Diagnostic dumper. Engine-specific format. |
| **iffdb** | ~1.1k | In-memory chunk tree. Library, not a tool — should become the *foundation* of the rewrite. |
| **iff2lvl** | ~8.7k | Whole asset pipeline pivot. Has to be rewritten *eventually*; the README already says so. |
| **lvldump** | ~2.0k | Diagnostic dumper for the runtime level format. |
| **oaddump** | ~616 | Diagnostic dumper for OAD. |
| **textile** | ~4.3k | Texture atlas packer (PSX/Win/Linux output). |
| **chargrab** | ~5.0k | Sprite/RMUV asset pipeline. Currently unbuilt; should be folded into `textile` or the asset pipeline. |
| **eval** | ~365 LoC handwritten<br>(+ ~5k generated) | **Correction of the original §4.1 listing.** `eval` is not a standalone calculator — it's the CLI test harness for `wfsource/source/eval/`, the runtime expression evaluator that the OAD attribute editor (and any other host of `worldfoundry-oad` that supports `szEnableExpression` predicates) calls at runtime. The library is a small flex+bison combo: `expr.l` (174 LoC) + `expr.y` (191 LoC), with a tiny public surface (`double eval(const char* expr, double (*lookup)(const char* sym))`). The `wftools/eval/eval.cc` binary is *byte-for-byte identical* to `wfsource/source/eval/evaltest.cc` (see §3.1 duplication table). The Rust shape: replace the flex/bison combo with a `winnow`-based parser, ~150 LoC of Rust, hosted as a `worldfoundry-eval` crate (or folded into `worldfoundry-oad` since they're always used together). The fate of this rewrite is *coupled* to the prep / `.oas` decision in §8 — see §8.6 specifically. |
| **prep** | ~1.9k | **Correction of the original §4.1 listing.** `prep` is *not* a C preprocessor — it's a custom macro/template processor with `@`-prefixed directives (`@define`, `@include`, `@*comment`, `@\` continuation, `@+` argument concatenation, `@t` tab, `@n` newline, etc.). It drives the `.oas` → multi-target schema-codegen pipeline in `wfsource/source/oas/`: each `.oas` source file is fed through several `.s` template files (`iff.s`, `oadtypes.s`, `oaddef.s`, `xml.s`, …) by `prep`, generating the IFF descriptor source for `iffcomp` (`*.iff.txt` → `*.iff`, the on-disk OAD format), the C runtime header (`*.ht`), the scripting-language header (`*.def`), and the XML version — all from one source of truth. `cpp -E` and `m4` cannot replace it because the input syntax isn't C-preprocessor syntax. **Two viable rewrites**, pick based on appetite: **(a) keep `prep` as-is** while rewriting only its consumers — it works, it's only ~1.9 kLoC, and there's no urgent reason to touch it; or **(b) replace the whole `.oas` pipeline** by defining schemas as Rust types in `worldfoundry-oad` with derive macros that emit each downstream artifact (IFF descriptor, C runtime header, Blender bindings, form widgets) from one source of truth — both `.oas` and the `.s` templates collapse into the same crate everything else uses. Option (b) is the larger payoff and the bigger lift; option (a) is the path of least resistance for Phases 1–5 and lets you defer the schema-codegen rewrite indefinitely. **See §8 for the deep-dive with full pros/cons and a recommendation.** |
| **attribedit** | ~4.7k | OAD attribute editor. **Hugely important concept** — the schema-driven editor for the format that ties game objects to their authored data. The standalone GTK app is a justified entry point but **the value is the embeddable library**, not the GUI. The schema parser and data model (`oadesciffbinparser`, `parsetreenode`, `oad`) are already library-shaped — they just got coupled to gtkmm in the same source tree, then re-implemented from scratch in `wfmaxplugins/attrib/` for the 3DSMax plugin host. The historical Max plugins are not being ported forward; they're the cautionary tale of what *parallel* implementations look like. The rewrite is: extract `worldfoundry-oad` as a Rust library and build the funded hosts on top — a small standalone editor (`attribedit-egui` / `eframe`) and a Blender addon. **Blender is the funded 3D-editor target** going forward, so it's the only embedding host that has to ship by default; additional hosts (3DSMax, Alias/Maya, Houdini, web) are explicitly *supported by* the library's host-agnostic design but built only if someone funds the work — and crucially they consume the *same* library, not a vendored copy. The asset-pipeline tools (`iff2lvl`, `lvldump`, `oaddump`) become CLI consumers of the same library instead of each carrying their own `oad.cc`. |

That's ~30 kLoC of "must rewrite (or at least keep alive) if you want to keep building this engine," but the entire OAD column above (~7+ kLoC counting the duplicates in §3.1) collapses into one library shared by all hosts. (Of the ~30 kLoC, `prep` accounts for ~1.9 kLoC and may be deferred or kept indefinitely; `eval` is small (~365 LoC handwritten) but its rewrite is required by Phase 3 unless option (b) of §8 is taken later, in which case it disappears entirely — see §8.5.)

## 5. Language recommendation

For the **keep + rewrite** set:

### Default: Rust

Wins, in rough order of importance for *this* codebase:

1. **Memory-safe binary I/O.** Every tool in the keep-list reads or writes packed binary structures. `nom` / `winnow` / `binrw` / `zerocopy` make that ergonomic *and* safe — no `_out->write((char*)&size, 4)` casts, no off-by-one in chunk header padding. The IFF chunk reader becomes ~150 lines of `winnow` instead of `iffread.cc` + the corresponding writer.
2. **Parser combinators replace flex+bison.** `wftools/iffcomp/lang.l` + `lang.y` is a small, self-contained DSL. A `winnow` parser is roughly the same line count, builds without `fixlex.pl`, runs in the same process as the rest of the tool, and produces actual error messages with span information instead of `syntax error at line N`.
3. **Single-binary distribution.** The whole point of these tools is that artists run them as part of the asset build. `cargo build --release` produces a static binary; `cross` cross-compiles to Windows from Linux in one command. The PIGS makefile spider (`GNUMakefile.env`, `GNUMakefile.rul`, `GNUpigs.dep`, `fixlex.pl`) goes away entirely.
4. **The `image` crate.** Replaces `tga.cc`, `bmp.cc`, `sgi.cc` (collectively ~1 kLoC) with two `image::open()` calls. SGI RGB isn't in the default codec set but `image` is extensible — and you probably don't have any SGI textures left anyway.
5. **Atlas packing.** `etagere`, `guillotiere`, or `crunch_packer` replace the bespoke `wftools/textile/allocmap.cc` allocator.
6. **Workspace structure.** `cargo workspace` with member crates: `worldfoundry-iff` (reader + writer), `worldfoundry-oad` (schema + parser + data model — the "extracted library" from §4.3 attribedit row), `iffcomp`, `iffdump`, `lvldump`, `oaddump`, `iff2lvl`, `textile`, `attribedit-egui` (the standalone OAD editor as an `egui`/`eframe` app). Internal crates compile in parallel and share types — exactly the relationship the PIGS makefile is *trying* to express.
7. **Blender-first packaging.** `worldfoundry-oad` (and as much of `worldfoundry-iff` as the addon needs) gets a `pyo3` binding crate built into a Python wheel via `maturin`. The Blender addon ships that wheel and exposes the OAD schema as `bpy.types.Panel` widgets with `bpy.props` bound to the library's data model — the same library the standalone `attribedit-egui` host links directly. Blender is the *funded* host on the roadmap, so it's the only embedding target that has to work by default; the architecture explicitly does not preclude additional hosts (3DSMax, Maya, Houdini, web) — they just don't ship until someone pays for them. When they do, they link the same `worldfoundry-oad` (via `cbindgen`-generated C headers, a JNI shim, a Python wheel, whatever the host's plugin SDK needs), not a vendored copy. The historical `wfmaxplugins/lib/` copy-paste is the cautionary example of what to *avoid*.

### Reasonable second choice: Go

Works fine if Rust feels too heavy. `encoding/binary` is more verbose than `byteorder` but it's there. Cross-compiles trivially. The losses: no parser-combinator ecosystem on par with `nom` (so the iffcomp DSL stays handwritten or uses `participle`), and binary serialization is more reflection-y than Rust's `zerocopy`/`bytemuck`.

### Glue and scripts: Python

For everything *outside* the keep+rewrite set — wrapper scripts around `mkpsxiso`, build orchestration around `meshoptimizer`, anything that replaces `perl.r` — Python is the obvious answer. `Lark` for any DSL parsing if not Rust.

### What I would *not* do

**Rewrite in modern C++.** The engine itself can stay C++ — there is too much sunk cost in the runtime to justify a port — but the *tools* don't share a runtime with the engine. They read files and write files. Rewriting tools in C++ means inheriting all the PIGS-style header soup *or* recreating it from scratch, neither of which buys you anything you couldn't get from `cargo new`. The whole motivation for decoupling tools from the engine is that they get to use a more ergonomic stack.

**Go all-in on Rust without porting `iff` first.** The right first move is the foundation, not a tool. See §6.

## 6. Suggested staging — five phases

The order matters because every tool depends on `iff` reading or writing — and on the OAD descriptor format on top of that. The five phases below correspond to natural cut points: each phase produces running, testable Rust binaries that exercise the previous phase's foundations against the existing C++ binaries.

### Phase 1 — IFF foundation, dumpers, and the IFF DSL

**Goal:** establish the Rust foundation crate for IFF, port the smallest tools as a differential test harness against the existing C++ binaries, and extract enough of the OAD descriptor schema to make `oaddump` work.

1. **`worldfoundry-iff` crate.** Reader + writer for the WF IFF dialect (little-endian payload, FOURCC IDs, 4-byte sibling alignment, back-patching for `.offsetof` / `.sizeof` directives). Replaces `wfsource/source/iff/iffread.cc`, `wfsource/source/iffwrite/{binary,_iffwr,fixed}.cc`. ~1 kLoC of Rust.

2. **`worldfoundry-oad` (descriptor-reader subset).** Just enough of the OAD descriptor schema to round-trip the on-disk format — the chunks `wftools/attribedit/oadesciffbinparser.cc` actually parses (`'I32'`, `'F32'`, `'STR'`, `'STRU'`, with the `'NAME'` / `'DSNM'` / `'RANG'` / `'DATA'` / `'DISP'` / `'HELP'` sub-chunks). The full editor data model (mutation, validation, override files) comes in Phase 3; Phase 1 only needs the read-only slice that `oaddump` requires.

3. **`iffdump`.** Tiny — walk the chunk tree from `worldfoundry-iff` and pretty-print with indentation + hexdump. Useful immediately as a debugging aid for the rest of the phase.

4. **`iffcomp`.** Replace `wftools/iffcomp/lang.l` + `lang.y` with a `winnow`-based parser, feeding `worldfoundry-iff`'s writer. Same DSL, no `fixlex.pl`, real error spans.

5. **`oaddump`.** Read an OAD descriptor file via `worldfoundry-iff` + `worldfoundry-oad` (subset) and pretty-print. Tiny, but the *first* tool that exercises both foundation crates on real production data emitted by the `.oas` pipeline (see §8 for what that pipeline looks like).

6. **`lvldump`.** Initially a thin IFF chunk dumper for level files — walks the chunk tree via `worldfoundry-iff` and pretty-prints, with the OAD descriptor chunks recognised and formatted via `worldfoundry-oad` (subset). Does *not* yet need `worldfoundry-level` (that lands in Phase 5); a Phase 1 `lvldump` is structurally just `iffdump` with OAD-aware chunk formatting. Gains semantic awareness of room/path/object/collision-box structures in Phase 5 when `worldfoundry-level` lands.

7. **Differential test harness.** Every test fixture committed to the C++ tree (`wftools/iffcomp/test.iff.txt`, `wfsource/source/iffwrite/test.iff.txt`, plus real OAD descriptors and level files emitted from `wfsource/source/oas/`) gets compiled / dumped by both the old C++ binaries and the new Rust ones; the byte streams (and dumper output) must match exactly. Catches every endianness/padding regression and OAD-schema misread before it bites a downstream tool.

**Phase 1 success:** `iffcomp test.iff.txt` produces a binary that the new `iffdump` and the old C++ `iffdump` print identically; `oaddump enemy.iff` and `lvldump some.lvl` match their old C++ counterparts on real engine data emitted from `wfsource/source/oas/` and the level build path.

### Phase 2 — Texture pipeline (`textile`, `chargrab`)

**Goal:** port the texture/sprite asset path. Independent of the OAD work, so Phase 2 can run in parallel with Phase 1 once the IFF crate is stable.

1. **`textile`.** Adopt `image` (TGA/BMP), `etagere` / `crunch_packer` (atlas packing), `clap` (CLI). Replaces ~4 kLoC of C++ with ~1 kLoC of Rust. SGI RGB support in `image` is opt-in; check whether any in-tree TGAs actually rely on SGI as a source format.

2. **`chargrab` folded in.** The sprite/RMUV pipeline becomes part of `textile` rather than a separate tool — the same source of truth for "where do textures come from."

### Phase 3 — `attribedit`: OAD library and standalone editor

**Goal:** complete `worldfoundry-oad` (full editor data model, not just descriptor reading) and build the standalone `egui` host. The Blender addon — the second GUI host — gets its own Phase 4 so the standalone editor can validate the API surface before a totally different runtime is consuming it.

1. **`worldfoundry-oad` (full).** Add the editor data model — mutation, validation, defaults, override files, the `AttributeEditor` / `OADescIffBinParser` equivalent. Once this lands, the duplicated OAD readers in `iff2lvl/oad.cc`, `lvldump/oad.cc`, `wfmaxplugins/attrib/oad.cc`, and `wfmaxplugins/oad2txt/oad.h` (catalogued in §3.1) all collapse into one consumer.

2. **`worldfoundry-eval` crate** (or fold into `worldfoundry-oad`). Rust port of the `wfsource/source/eval/` runtime expression evaluator (`expr.l` + `expr.y`, ~365 LoC of flex/bison). Used by `worldfoundry-oad` (and therefore by `attribedit-egui` and the Phase 4 Blender addon) to evaluate `szEnableExpression` predicates pulled out of OAD descriptor chunks at runtime. Replace with a `winnow` parser, ~150 LoC of Rust. **Note:** if option (b) of §8 is taken later, this crate goes away entirely — `enable_when` predicates become compile-time Rust expressions instead of runtime string-eval. See §8.6.

3. **`attribedit-egui`.** The standalone OAD editor as an `egui` / `eframe` app on top of `worldfoundry-oad` + `worldfoundry-eval`. First GUI host of the library; validates the API surface before the Blender addon needs to consume it. Cross-platform single binary (Linux / macOS / Windows) via `cargo build --release`.

### Phase 4 — Blender addon

**Goal:** prove the OAD library is genuinely host-agnostic by building a second GUI host on a totally different runtime — Python inside Blender, instead of native egui. Phase 4 is also where the *funded* 3D-editor authoring path actually starts working end-to-end.

1. **`worldfoundry-py`.** A `pyo3` binding crate that exposes `worldfoundry-oad` (and as much of `worldfoundry-iff` as the addon needs) to Python, built into a wheel via `maturin`. The crate contains zero schema duplication — it's a thin Python facade over the same Rust types `attribedit-egui` uses.

2. **Blender 4.x addon.** A Python addon that imports `worldfoundry_py`'s wheel and exposes the OAD schema as `bpy.types.Panel` widgets with `bpy.props` bound to the library's data model. Same library `attribedit-egui` uses, no parallel implementation, no vendored copy.

3. **glTF export hook.** A separate hook in the addon (or a subsequent addon) that drives Blender's built-in glTF exporter to produce the wire format Phase 5's `iff2lvl` will eventually consume. This is what makes Blender authoring end-to-end useful: artists can both edit OAD attributes *and* export geometry from a single host.

(Blender is the *funded* 3D-editor host; additional hosts — Max, Maya, Houdini, web — slot in here as additional thin wrappers around the same `worldfoundry-py` / `worldfoundry-oad` if someone funds the work.)

### Phase 5 — `iff2lvl`: level converter pipeline

**Goal:** rewrite the level format importer. Largest and hardest phase by a wide margin. Two coupled rewrites: the importer implementation *and* its input contract.

The author already says `iff2lvl` needs a rewrite (`wftools/iff2lvl/README:8-9`). The numbered-split files `level.cc` / `level2.cc` / `level3.cc` / `level4.cc` / `level5.cc` are diagnostic of code that grew faster than its module boundaries. Two primary-source examples of unaddressed cruft: `wftools/iff2lvl/oad.cc:81` literally has

```cpp
#pragma message ("KTS " __FILE__ ": this fails sometimes, figure out why")
```

and `wftools/iff2lvl/channel.cc:93` has

```cpp
AssertMessageBox(0, "Channel::Read() is unimplimented");
```

in a function that's still called from the read path.

**And the input contract is also legacy.** `iff2lvl` currently eats `.lev` files exported by the 3DSMax plugin pipeline (`wfmaxplugins/max2iff/`, `wfmaxplugins/max2lev/`) — that pipeline is no longer the funded authoring environment. The Phase 5 rewrite needs a Blender input path (whatever the Phase 4 Blender addon emits via its glTF export hook), and an explicit decision about whether to keep reading Max `.lev` for compatibility with existing in-flight content.

1. **`worldfoundry-level` crate.** The level data model.

2. **`lvldump` semantic upgrade.** The Phase 1 `lvldump` was a thin IFF chunk dumper. Once `worldfoundry-level` lands, `lvldump` gains semantic awareness — printing rooms, paths, objects, collision boxes, and channels by name and type instead of as opaque chunks. Same binary, richer output.

3. **Pluggable input front-ends.** A `gltf` reader for Blender-exported assets (the funded forward path) and optionally a `max-lev` reader for existing `.lev` files (only if in-flight content needs to keep building). Treating the importer as `worldfoundry-level` + pluggable readers is the structural shape that makes "support this other authoring host if someone pays for it" cheap, the same way `worldfoundry-oad` does for the attribute editor.

4. **`iff2lvl` tool.** Glue: parse input via the chosen front-end, convert via `worldfoundry-level`, write the runtime level format. Strip generation via `meshoptimizer` happens here, not as a separate tool — replaces the unbuilt `wftools/stripper/`.

## 7. Disposition summary

| Tool | Disposition | Replacement / target language |
|---|---|---|
| xcd | **drop** | `zoxide` |
| imagedir | **drop** | `tree`, `sigal`, `thumbsup` |
| perl.r | **drop** | `find -exec rename` / Python |
| wfbind | **drop** | `objcopy --add-section` or sidecar files |
| psxdisc | **drop** (PSX no longer targeted) | — |
| disr3000 | **drop** (PSX no longer targeted) | — |
| link | **drop** (PSX no longer targeted) | — |
| psxprof | **drop** (PSX no longer targeted) | — |
| reg | **drop** | `reg.exe` / PowerShell |
| stripper | **replace** | `meshoptimizer`, ideally inside `iff2lvl` |
| aicomp | **replace** | embed Lua (or Wren / AngelScript) |
| eval | **rewrite (in Phase 3)** | **Rust** — small flex/bison expression-language library; rewrite as a `winnow` parser (~150 LoC) in a `worldfoundry-eval` crate. Required by `worldfoundry-oad` (full) for `szEnableExpression` evaluation. See §8.6 for how option (a)/(b) of the prep decision can change this — option (b) collapses `worldfoundry-eval` into compile-time Rust expressions and the runtime evaluator goes away entirely. |
| prep | **keep (or eventually rewrite)** | Custom macro/template processor — *not* a C preprocessor — that drives the `.oas` schema-codegen pipeline. Either keep alive while rewriting consumers, or replace the whole `.oas` pipeline with Rust derive macros (option (b) in §4.3 prep row). See §8 deep-dive. |
| attribedit | **rewrite as library + thin GUI host** | **Rust** — extract `worldfoundry-oad` crate; funded hosts: standalone egui editor + Blender addon (PyO3/maturin wheel). Additional 3D-editor hosts (Max, Maya, Houdini, web) supported by the architecture, not on the funded roadmap. |
| iffcomp | **rewrite** | **Rust** (`winnow` parser → `worldfoundry-iff` writer) |
| iffdump | **rewrite** | **Rust** |
| iffdb | **rewrite** | **Rust** (becomes `worldfoundry-iff` library) |
| iff2lvl | **rewrite** | **Rust** |
| lvldump | **rewrite** | **Rust** |
| oaddump | **rewrite** | **Rust** |
| textile | **rewrite** | **Rust** + `image` + `etagere` |
| chargrab | **rewrite** | **Rust** — fold into `textile` / asset pipeline |

**Net effect (in-tree LoC accounting):**

| Bucket | LoC | Notes |
|---|---:|---|
| Drop | ~5 kLoC | PSX toolchain (~4.5 kLoC) + the small everyday-tool drops (~0.7 kLoC) |
| Replace | ~4 kLoC | `stripper` (→ meshoptimizer) + `aicomp` (→ embed Lua) |
| Rewrite | ~30 kLoC of C++ → ~9–11 kLoC of Rust | The IFF/OAD/level/textile core (~28 kLoC), plus `prep` if/when option (b) of the `.oas` rewrite is taken (~1.9 kLoC). Phases 1–5 do not require the `prep` rewrite — see §8 for the option (a)/(b) deep-dive. |

…and the OAD-handling duplication catalogued in §3.1 (~6–8 kLoC of parallel implementations and vendored copies across `wfmaxplugins/` and the asset-pipeline tools) collapses into a single `worldfoundry-oad` library shared by every host. Build system collapses from `GNUmakefile` + `GNUMakefile.tool` + `fixlex.pl` + the PIGS edifice down to a `Cargo.toml` workspace and maybe a top-level `justfile`.

## 8. Deep-dive: `prep` and the `.oas` pipeline — option (a) vs option (b)

§4.3 (prep row) and §7 (disposition table) both flag two ways to handle the `.oas` schema-codegen pipeline. This section investigates both in more detail and recommends one for the staging plan in §6.

### 8.1 What `prep` actually is

`prep` is a custom macro/template processor — *not* a C preprocessor, despite the directory name. The active source is ~1.9 kLoC of C++ split across four files:

| File | LoC | Role |
|---|---:|---|
| `wftools/prep/source.cc` + `source.hp` | ~1200 | Tokenizer, line splitter, macro expansion engine, `@`-directive handler. The meat of the implementation. |
| `wftools/prep/macro.cc` + `macro.hp` | ~360 | `macro` class — stores the macro body, parameter list with default values, expansion logic. |
| `wftools/prep/prep.cc` + `prep.hp` | ~250 | `main`, command-line parsing (`-D<name>=<value>`, `-p<stream>`), output stream management. |
| `wftools/prep/global.hp` | ~40 | Global declarations. |

It depends on the PIGS library tree (`pigsys`, `cpplib/stdstrm`, and two specific `recolib` headers for tokenization and CLI parsing):

- **`recolib/ktstoken.{cc,hp}`** — `ktsRWCTokenizer`, a small rewindable string tokenizer (276 LoC). Used throughout `prep`'s source/macro/main files for splitting macro bodies, parameter lists, and `-D<name>=<value>` switches.
- **`recolib/command.{cc,hp,hpi}`** — `CommandLine`, a thin C++ wrapper around `argc`/`argv` (116 LoC).

That's ~390 LoC of straightforward C++ utility code with no flex/bison and no PIGS coupling beyond standard I/O. Six of `prep`'s seven source files include one or both. `recolib` is otherwise unused by anything else in the live `wftools/` build target — keeping `prep` alive means keeping (at least these two pieces of) `recolib` alive, which is the cleanup motivating §8.5's "inline into prep" note.

### 8.2 The `.oas` language — what option (b) would have to replace

The `.oas` source files are a domain-specific language for declaring object-attribute schemas. Real evidence:

- **42 `.oas` files** in `wfsource/source/oas/` (one per game-object type — `actor.oas`, `enemy.oas`, `camera.oas`, `room.oas`, `missile.oas`, …).
- **7 template `.s` files** that `prep` compiles `.oas` against, each producing a different downstream artifact: `iff.s` (IFF descriptor for `iffcomp`), `oadtypes.s` (C runtime header `*.ht`), `oaddef.s` (scripting-language header `*.def`), `types3ds.s` (legacy 3DSR4 dialog descriptors), `xml.s` (XML version), `assets.s` (asset list), `objects.s` (the C switch-statement constructor over object types).
- **`objects.mac`** — a single master file listing every object type via macros (`OBJECTENTRY(Enemy,1)`, `OBJECTNOACTORENTRY(Room,0)`, `OBJECTONLYTEMPLATEENTRY(Missile,1)`) and a separate collision-table block (`COLTABLEENTRY(Enemy,Player,CI_SPECIAL,CI_PHYSICS)`).
- **`.inc` shared blocks** — `actor.inc`, `mesh.inc`, `movebloc.inc`, `toolset.inc`, etc. — that get pulled into multiple `.oas` files via `@include`. This is the *composition* mechanism: an `actor.oas` is just `TYPEHEADER(Actor) @include actor.inc TYPEFOOTER`.

The macro language has more capability than `cpp`:

- Named parameters with default values: `TYPEENTRYFILENAME(name, displayName=name, filespec="*.*", help="", szEnableExpression="1", y=-1, x=-1)`.
- `@include`, `@define`, `@ifndef`, `@undef` (control flow over macro state).
- `@\` line continuation, `@+ name @+` argument concatenation (analog of `cpp`'s `##`), `@t` tab, `@n` newline, `@-` indent control, `@*` to-end-of-line comments.
- A tiny *embedded* expression language inside `szEnableExpression` strings: `"ModelType==1 || ModelType==2"`. The 3D editor evaluates these at runtime to enable / disable widgets based on other field values. This is a runtime DSL on top of the codegen DSL, and any rewrite has to preserve its semantics or replace it.

So option (b) isn't just "translate 42 .oas files into Rust structs." The full migration target is:

1. 42 schema definitions (the .oas files)
2. ~7 shared blocks (the .inc files) and their composition story (mixin/trait equivalents)
3. ~7 codegen backends (one per .s template), three of which are still load-bearing: `iff.s` (the on-disk OAD format), `oadtypes.s` (the C runtime header), `oaddef.s` (the scripting header)
4. The `objects.mac` master list (object enumeration + collision table)
5. The `szEnableExpression` mini-language

### 8.3 Option (a) — keep `prep` and the `.oas` pipeline as-is

**Pros:**

- **It works.** The `.oas` pipeline is the most production-tested part of the entire wftools tree and produces all the artifacts the engine actually needs. Authors know how to write `.oas` files; the convention is established.
- **Small surface to maintain.** ~1.9 kLoC of C++. Once detangled from the PIGS tree (or after `recolib`'s two used utilities are inlined into `prep` itself), it becomes a self-contained binary the team rarely has to touch.
- **Zero blocking dependency on other phases.** Phases 1–5 don't depend on the `.oas` pipeline at all. The on-disk OAD descriptor data they consume is *already produced* by `prep` + `iffcomp`. Whether that data was generated by a 1990s preprocessor or by Rust derive macros doesn't matter to downstream tools.
- **Risk isolation.** `.oas` is the part of the system most directly authored by content creators. Touching it has higher blast radius than touching the runtime tools — a regression breaks asset workflows, not just engineering tooling.
- **Decision is reversible.** "Keep prep" doesn't preclude "rewrite prep later." Option (b) is still on the table at any future point.

**Cons:**

- **Locks PIGS in.** `prep` is the *only* live consumer of `recolib` in the rewritten tree. The PIGS edifice can't be fully retired as long as `prep` links it. Mitigation: inline the two `recolib` utilities into `prep` itself (a few hundred LoC of mechanical work).
- **Custom DSL with no IDE support.** No syntax highlighting, no jump-to-definition, no error spans, no LSP. Authors learn the language by reading other `.oas` files and the template `.s` sources.
- **Schema is not visible to Rust at compile time.** The Rust runtime tools (`worldfoundry-oad`, `worldfoundry-level`, the Blender addon) see schemas only as *parsed* data at runtime. This means the schemas can drift between `.oas` and any Rust-side mirror, and there's no compile-time check that they agree.
- **Two languages forever.** Authors of game objects write `.oas`; everyone else writes Rust. The mental-model gap doesn't shrink over time.
- **The `szEnableExpression` mini-language stays as a runtime string-eval inside the editor.** Editor errors in this mini-language only surface when an artist tries to use the field — not at schema-author time.
- **The `worldfoundry-eval` crate has to exist.** Option (a) requires Rust ports of `wfsource/source/eval/`'s expression evaluator (the `expr.l` + `expr.y` flex/bison combo) so the new `attribedit-egui` and Blender hosts can evaluate predicates pulled out of OAD descriptors at runtime. Small (~150 LoC of `winnow` parser) but it's a permanent foundation crate that exists *only* to evaluate strings the `.oas` author wrote. See §8.6.

### 8.4 Option (b) — replace `.oas` with Rust types + derive macros

The shape: schemas live as Rust types in `worldfoundry-oad`'s `schema/` module. Derive macros (or build.rs codegen passes) emit the IFF descriptor, the C runtime header (via `cbindgen`), the Blender Python bindings (via `pyo3`), and the egui form widgets — all from one source of truth.

```rust
#[derive(OadSchema)]
#[oad(display = "Enemy")]
struct Enemy {
    #[oad(min = 0, max = 100, default = 10, help = "Initial HP")]
    hit_points: i32,

    #[oad(default = false, enable_when = "ModelType == 1 || ModelType == 2")]
    has_mesh: bool,

    #[oad(filespec = "*.iff", help = ".iff mesh")]
    mesh_name: String,

    // Composition equivalent of @include actor.inc:
    #[oad(flatten)]
    actor: ActorBlock,
}
```

**Pros:**

- **One source of truth.** Schemas live in Rust. Each downstream artifact comes from a derive macro or a build.rs codegen pass; there's no possibility of drift.
- **Compile-time validation.** Misspelled field names, wrong types, missing required attributes, malformed `enable_when` predicates — all surface as Rust compile errors instead of failing at editor runtime.
- **Full IDE story.** rust-analyzer, cargo check, jump-to-definition, refactoring tools, all the standard Rust tooling — for free, immediately.
- **Cleaner test story.** Round-trip tests on the Rust types + per-target snapshot tests on the generated artifacts. Property-based testing (`proptest`) becomes natural.
- **Multiple codegen targets share the Rust type system.** Adding a new backend (a TypeScript schema for a future web editor; a JSON Schema for tooling integration) is one new derive macro pass, not a new `.s` template.
- **Retires PIGS completely.** No `recolib`, no `cpplib`, no `pigsys`. The asset-pipeline crates compile from `cargo build` with no PIGS dependency anywhere.
- **The `enable_when` mini-language can move from runtime string-eval to compile-time validated expressions** — either by parsing the predicate string at codegen time and checking field references against the struct, or by replacing the strings with Rust closure / token-stream attributes.
- **The `LEVELCONFLAGCOMMONBLOCK` composition story** maps cleanly onto Rust traits or `#[oad(flatten)]` field attributes — better than `@include` because the composition is type-checked.
- **The `worldfoundry-eval` crate (and its flex/bison ancestor) goes away entirely.** With option (b), `enable_when` predicates are Rust expressions that exist at compile time — no runtime evaluator needed, no lexer, no parser, no symbol-lookup callback. The whole `wfsource/source/eval/` library + the `wftools/eval/` test harness + the byte-for-byte duplication catalogued in §3.1 collapse to nothing. This is one of option (b)'s biggest concrete simplifications: it removes a whole foundation crate from Phase 3 and one of the two remaining flex/bison consumers from the codebase. See §8.6.

**Cons:**

- **Migration cost.** 42 `.oas` files + their `.inc` shared blocks + `objects.mac` all need to be translated. Not pure mechanical translation: the macro semantics need careful handling, especially the `szEnableExpression` mini-language and the composition pattern.
- **Multiple derive macros to write.** Each downstream backend (IFF descriptor, C runtime header, scripting header, Blender bindings, form widgets) is a separate codegen pass. Each is small but they accumulate. A first cut needs at least IFF descriptor + C runtime header to keep the engine building.
- **The C++ engine still consumes the C runtime header.** Either keep emitting C headers (via `cbindgen` or a hand-written codegen pass) or rewrite the runtime side too. Rewriting the runtime is *vastly* out of scope; emitting compatible C headers is the realistic move.
- **Risk of breaking content workflows.** Any divergence between the new derive-emitted IFF descriptor and the old `prep`-emitted one will manifest as silent data corruption, not loud compile errors. A differential test harness against the old `prep` output is essential.
- **Bigger up-front investment** with no payoff until everything else is on Rust *and* the C++ engine still consumes the new artifacts identically.

### 8.5 Cross-cutting impact: what happens to `eval`

The `wfsource/source/eval/` library (and its CLI test harness, which lives in two byte-identical copies at `wftools/eval/eval.cc` and `wfsource/source/eval/evaltest.cc` — see §3.1) is the runtime expression evaluator that the OAD attribute editor calls to evaluate `szEnableExpression` predicates at runtime. The library is small: `expr.l` (174 LoC) + `expr.y` (191 LoC), one public function `double eval(const char* expr, double (*lookup)(const char* sym))`. It's the *third* flex/bison combo in the live build path (alongside `iffcomp/lang.y` and the unbuilt `aicomp` grammars).

The `eval` library's fate is **directly coupled to the prep / `.oas` decision**:

- **Under option (a)** — keep `prep` and the `.oas` pipeline as-is — the `enable_when` predicates remain stringly-typed in `.oas` source files. The runtime editor (`attribedit-egui`, the Blender addon, any other OAD host) has to evaluate those strings at runtime, so it has to bring along a Rust port of the `eval` library. Phase 3 substep 2 (`worldfoundry-eval`) is required. ~150 LoC of `winnow` parser; small but a permanent foundation crate, and a permanent dependency for every OAD host.

- **Under option (b)** — replace `.oas` with Rust types + derive macros — the `enable_when` predicate becomes a *compile-time* Rust expression (or a Rust closure attribute). The runtime evaluator stops existing because there are no runtime strings left to evaluate. `worldfoundry-eval` doesn't need to be written. `wfsource/source/eval/` + `wftools/eval/` + `wfsource/source/eval/evaltest.cc` + `expr.l` + `expr.y` all disappear. One whole flex/bison combo and one whole foundation crate vanish.

So option (b) has a second concrete payoff that didn't show up in the §8.4 pros list above: it eliminates the entire `eval` codebase from the new tree. That's ~365 LoC of flex/bison + the `worldfoundry-eval` crate that would otherwise be in Phase 3, *plus* the duplication catalogued in §3.1, *plus* the second flex/bison consumer in §3 (which leaves `iffcomp` as the only one).

This doesn't change the recommendation in §8.6 — the timing argument still favors option (a) for Phases 1–5 — but it sharpens the case for revisiting option (b) after Phase 5 lands. The eval-going-away win is concrete and measurable, and it accumulates with the rest of option (b)'s payoffs.

### 8.6 Recommendation

**Take option (a) for Phases 1–5. Reconsider option (b) as a possible Phase 6, with explicit gates.**

Reasoning:

1. **Decoupling.** The `.oas` rewrite blocks nothing in Phases 1–5. Phases 1–5 read OAD descriptor data that's already on disk; they don't care how it got there. Coupling the schema-codegen rewrite to the rest of the work introduces risk and timeline drag for no payoff.

2. **Risk isolation.** Phases 1–5 are runtime-tool rewrites that affect engineering workflow. Option (b) would be a content-pipeline rewrite that affects artists. Mixing those into one effort multiplies the risk surface; doing them separately keeps each one's blast radius bounded.

3. **Better timing later.** After Phase 5 lands, the team has working Rust analogs of every consumer of OAD data (`worldfoundry-iff`, `worldfoundry-oad`, `worldfoundry-level`, the Blender addon, the runtime tools). At *that* point, option (b) is much smaller in relative cost: the team has Rust expertise, has working derive-macro patterns from the rest of the tree, and can validate the new derive output against five different consumers all at once. The same option (b) attempted *before* Phase 1 has none of that scaffolding.

4. **Cost of (a) is bounded.** ~1.9 kLoC of C++ + a few hundred LoC of inlined `recolib` utilities. Once detangled from PIGS, `prep` is a self-contained binary nobody touches.

5. **Option (b) gates can be made concrete.** Don't take option (b) until: (i) `worldfoundry-oad` is the in-memory representation everywhere; (ii) there's a working differential test harness comparing `prep`-emitted descriptors against any candidate replacement; (iii) the C++ engine's expectations of the C runtime header are documented (so the new codegen path can match them); (iv) at least one non-IFF backend (Blender bindings or egui form widgets) has a working hand-written prototype, to validate that the schema translates cleanly.

**Practical cleanup as part of option (a):** while keeping `prep` alive, do two small pieces of work that make it self-contained:

- **Detangle from `recolib`.** Inline `recolib/ktstoken.hp` and `recolib/command.hp` (the two headers `prep.cc` actually uses) into `prep` itself. This is mechanical and removes the only blocker to deleting `recolib` from the rest of the live build.
- **Document the `.oas` macro language.** A short reference page (the existing `wftools/prep/prep.doc` is a binary `.doc` file unreadable on Linux — recover it, or write a fresh markdown reference). This is the highest-leverage thing you can do to lower the option (a) cost long-term.

## References

### In-tree

- `wftools/GNUmakefile` — live build list
- `wftools/makefile` — legacy NMAKE build list with abandoned-tool annotations
- `wftools/CHANGELOG` — 1999-03-20 abandonment notes
- `wftools/GNUMakefile.tool` — per-tool include with `PIGS_LIBS`
- `wftools/fixlex.pl`, `wftools/GNUfixlex.pl` — flex+C++ workaround
- `wftools/iff2lvl/README` — author's "code base is a mess" admission (lines 8–9)
- `wftools/iff2lvl/oad.cc:81`, `wftools/iff2lvl/channel.cc:93` — primary-source examples of accumulated cruft
- `wftools/eval/eval.cc:31` — `e = 2.17` (in the symbol-lookup callback of the test harness, not the library itself)
- `wfsource/source/eval/` — runtime expression evaluator library: `eval.h` (public function), `expr.l` (174 LoC flex lexer), `expr.y` (191 LoC bison grammar), `evaltest.cc` (35 LoC test harness — *byte-for-byte identical* to `wftools/eval/eval.cc`)
- `wfsource/source/recolib/{ktstoken,command}.{cc,hp,hpi}` — the two `recolib` utilities (`ktsRWCTokenizer` and `CommandLine`, ~390 LoC total) that `prep` is the only live consumer of in `wftools/`
- `wftools/attribedit/attribedit.hp:75` — `// kts this is a kludge, design a better way`
- `wftools/attribedit/{oad,oadesciffbinparser,parsetreenode,parser}.{cc,hp,hpi}` — the OAD schema/parser/data-model code that should become `worldfoundry-oad`
- `wfmaxplugins/attrib/{attrib,oad,oaddlg}.{cc,h,hpp}` — the duplicated OAD editor for the 3DSMax plugin host
- `wfmaxplugins/oad2txt/{oad2txt,scene}.{cc,hpp}` + `wfmaxplugins/oad2txt/oad.h` — yet another OAD model for the Max txt-export plugin
- `wfmaxplugins/lib/{_iffwr,binary,fixed,loadfile,stdstrm,hdump,assert}.{cc,hpp}` — vendored copy of half of pigs2, because the Max plugin SDK couldn't link the engine library tree directly
- `wftools/{iff2lvl,lvldump}/oad.{cc,hp}`, `wftools/oaddump/` — the asset-pipeline tools that each carry their own OAD reader
- `wfsource/source/iff/`, `wfsource/source/iffwrite/` — load-bearing IFF reader/writer libraries
- `WorldFoundry/2026-04-10-worldfoundry-iffcomp-format.md` — companion investigation: WF IFF binary format

### External tools referenced

- [meshoptimizer](https://github.com/zeux/meshoptimizer) — strip generator + mesh utilities
- [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) — modern PSX disc image builder
- [PCSX-Redux](https://github.com/grumpycoders/pcsx-redux) — PSX emulator with built-in profiler
- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) — modern open-source PSX SDK (uses binutils-mips)
- [zoxide](https://github.com/ajeetdsouza/zoxide) — modern `cd` replacement
- [Dear ImGui](https://github.com/ocornut/imgui) — single-header C++ GUI
- [Lua](https://www.lua.org/) — embeddable scripting language
- [winnow](https://github.com/winnow-rs/winnow) / [nom](https://github.com/rust-bakery/nom) — Rust parser combinators
- [image](https://github.com/image-rs/image) — Rust image-format crate
- [etagere](https://github.com/nical/etagere) / [crunch_packer](https://github.com/ChevyRay/crunch-rs) — Rust atlas packers
