# Plan: Rust ports of wftools command-line tools

## Which tools, and why

| Tool | C++ LOC | Verdict | Rationale |
|------|---------|---------|-----------|
| **oas2oad** | ~400 (wpp+wlink chain) | **Done** | `prep` → `g++` → `objcopy .data`; same guarantee as original toolchain — 41/41 .oas files pass |
| **iffdump** | ~372 | **Port next** | Natural companion to iffcomp-rs; IFF binary format already understood; all deps are trivial stubs |
| **oaddump** | ~546 | Port after iffdump | Self-contained OAD parser; small; no external deps |
| eval | ~53 (CLI) + ~400 (grammar) | Blender work | Expression evaluator grammar (`wfsource/source/eval/`) needed by `wf_attr_validate`; belongs with Blender integration, not here |
| prep | ~1 615 | Maybe later | Custom tokenizer + macro expander; interesting but larger scope |
| chargrab / textile | ~4 000+ | Maybe later | Image packing tools; Windows I/O mixed in; bigger effort |
| iff2lvl | ~8 731 | No | Massive 3D level converter; hardcoded paths; PSX/Saturn targets |
| attribedit | ~3 517 | Blender plugin | GTK+ 2.x GUI that edits OAD properties on level objects — the equivalent functionality is what the Blender plugin needs to implement |
| iffdb | ~536 | Superseded | Alternative iffdump implementation (loads IFF into in-memory tree first); `iffdump-rs` renders it redundant |
| lvldump | ~1 600 | Defer | Reads compiled `.lvl` game-level binaries; formats in `.oas` files, renderable by existing `prep`; port after oaddump-rs |

---

## Port 1: `iffdump-rs`

### What it does

Reads a binary IFF file and prints its chunk structure in human-readable text,
either as a pretty hex+ASCII dump (`-f+`, default) or as iffcomp-compatible
source (`-f-`). The output of `iffdump -f-` is valid input to iffcomp — the
two tools are inverses.

### IFF reading format

Same 8-byte header as iffcomp writes, just read instead of written:

```
[4 bytes] Chunk ID  — big-endian FOURCC (e.g. 'TEST')
[4 bytes] Payload size — little-endian u32 (bytes after this field)
[N bytes] Payload
[0–3 bytes] Padding to 4-byte alignment (zeros, not part of payload size)
```

Chunks are nested: a "wrapper" chunk's payload is itself a sequence of chunks.
A whitelist (`chunks.txt`, default list hard-coded) says which FOURCCs are wrappers.

### CLI interface

```
iffdump-rs [-c<chunks_file>] [-d+|-d-] [-f+|-f-] [-w=<width>] <infile> [outfile]
```

| Flag | Default | Meaning |
|------|---------|---------|
| `-c<file>` | built-in list | Wrapper chunk whitelist file |
| `-d+` / `-d-` | `+` | Enable/disable hex dump of leaf chunks |
| `-f+` / `-f-` | `+` | Pretty HDump vs. iffcomp `$XX` format |
| `-w=<N>` | 16 | Chars per line in hex dump |

Exit codes: `0` success, `1` usage/I/O error.

### Default wrapper chunk list

`OADL TYPE LVAS L0 L1 L2 L3 PERM RM0 RM1 RM2 RM3 RM4 UDM IFFC`

Any FOURCC not in this list is treated as a leaf and its bytes are hex-dumped.

### Output format

```
//=============================================================================
// <outfile> Created by iffdump-rs v0.1
//=============================================================================

{ 'TEST'		// Size = 42
  { 'NEST'		// Size = 8
    0000: 01020304 ...    Hello...
  }
}
```

Pretty mode (`-f+`) — each line: `OOOO: HHHHHHHH HHHHHHHH HHHHHHHH HHHHHHHH    AAAA...`
IFFComp mode (`-f-`) — raw hex: `$01 $02 $03...` wrapped at `-w` chars per line.

### Shared crate: `wf_hdump`

`wfsource/source/recolib/hdump.cc` (~60 LOC) is used by `iffdb`, `lvldump`, and
`iffdump`. Port it as a standalone library crate so all three tools share one
implementation.

```
wf_hdump/
  Cargo.toml    — lib crate
  src/
    lib.rs      — pub fn hdump(data: &[u8], indent: usize, chars_per_line: usize, out: &mut impl Write)
```

Public API mirrors `HDump()`: offset-prefixed hex+ASCII lines, 4-byte groups, non-printable → `.`.

### Directory layout

```
iffdump-rs/
  Cargo.toml
  src/
    main.rs      — CLI arg parsing, open files, call lib
    lib.rs       — dump() public entry; re-exports
    reader.rs    — IffReader: open file, iterate chunks recursively
    dump.rs      — format_iffcomp_hex(); hdump delegated to wf_hdump crate
    error.rs     — IffDumpError enum (Io, Parse)
  testdata/      — symlink → ../iffcomp-go/testdata/
  tests/
    round_trip.rs  — iffdump -f- | iffcomp -binary should reproduce the original
```

### Critical reference files

| File | Purpose |
|------|---------|
| `wftools/iffdump/iffdump.cc` | Original C++ source; CLI flags, output format, wrapper whitelist |
| `wfsource/source/iff/iffread.cc` | IFF chunk iterator — shows exact byte layout and alignment padding |
| `wfsource/source/recolib/hdump.cc` | Hex+ASCII formatter — port as `wf_hdump` crate |
| `wftools/iffcomp-rs/src/writer.rs` | Already has `id_name()` and IFF constants — reuse directly |

### Implementation plan

**`error.rs`** — two variants: `Io { path, source }` and `Parse { msg }`.

**`reader.rs`** — reads and walks the chunk tree:
```rust
pub struct Chunk { pub id: u32, pub payload: Vec<u8> }

fn read_chunk(buf: &[u8], pos: &mut usize) -> Result<Chunk>
fn is_wrapper(id: u32, wrappers: &HashSet<u32>) -> bool
fn dump_chunk(chunk: &Chunk, depth: usize, wrappers: &HashSet<u32>, opts: &Opts, out: &mut impl Write)
```
- Read 4-byte ID (big-endian u32)
- Read 4-byte size (little-endian u32)
- Advance by `(size + 3) & !3` bytes (aligned)
- If wrapper: recurse into payload
- If leaf: call dump formatter

**`dump.rs`** — two formatters:
```rust
fn format_hdump(data: &[u8], width: usize, out: &mut impl Write)
fn format_iffcomp(data: &[u8], width: usize, out: &mut impl Write)
```
HDump: `OOOO: HHHHHHHH HHHHHHHH HHHHHHHH HHHHHHHH    AAAA....` where non-printable → `.`
IFFComp: `$HH $HH ...` wrapped at `width` chars, indented to current depth.

**`main.rs`** — manual flag parsing (same pattern as iffcomp-rs, no external crates).

**`lib.rs`** — `pub fn dump(in_path, out_path, opts) -> Result<()>`

### Reuse from iffcomp-rs

- Copy `id_name(u32) -> String` from `iffcomp-rs/src/writer.rs`
- FOURCC packing/unpacking logic is identical

### Testing

```bash
# Round-trip: iffdump -f- output should recompile to identical bytes
./iffdump-rs -f- testdata/expected.iff > /tmp/round.iff.txt
./iffcomp-rs -binary -o /tmp/round.iff /tmp/round.iff.txt
diff /tmp/round.iff testdata/expected.iff   # zero diff (modulo timestamp mask)

# Pretty dump smoke test
./iffdump-rs -f+ testdata/expected.iff

# all_features fixture
./iffdump-rs -f- testdata/all_features.iff > /tmp/af_rt.iff.txt
./iffcomp-rs -binary -o /tmp/af_rt.iff /tmp/af_rt.iff.txt
diff /tmp/af_rt.iff testdata/all_features.iff
```

Automated: `cargo test` runs the round-trip in `tests/round_trip.rs`.

---

## Port 2: `oaddump-rs` ✓ shipped

OAD (Object Attribute Data) files describe game object properties.
Ported as two crates: `wf_oad` (library) + `oaddump-rs` (CLI).

### `.oad` file generation pathway

`.oad` files are **not** stored in the repo — they are build artifacts.
The original Windows pipeline (`wfsource/source/oas/objects.mak`):

```
prep -DTYPEFILE_OAS=<name> types3ds.s <name>.pp   # generates C source
wpp  <name>.pp                                      # Watcom C++ → <name>.obj
wlink system dos file <name>.obj                    # Watcom linker → DOS .exe
exe2bin <name>.exe <name>.tmp                       # strip MZ header → raw binary
copy <name>.tmp $(OAD_DIR)/<name>.oad
```

The `.oad` binary is literally the initialized-data section of a tiny Watcom
DOS program, stripped of its MZ executable header. The layout is exactly the
packed C structs from `wfsource/source/oas/oad.h` with `#pragma pack(1)`:

| Section | Size | Description |
|---------|------|-------------|
| `_oadHeader` | 80 bytes | magic `OAD ` (LE u32), chunkSize, name[68], version |
| `_typeDescriptor` × N | 1491 bytes each | one entry per OAD field |

`oas2oad-rs` (see below) provides the Linux replacement using `prep` → `g++` → `objcopy`.

### Reference files

| File | Purpose |
|------|---------|
| `wfsource/source/oas/oad.h` | Binary format: `_oadHeader`, `_typeDescriptor`, button type constants |
| `wfsource/source/oas/types3ds.s` | `prep` template that generates the C source compiled into `.oad` |
| `wfsource/source/oas/objects.mas` | Master Makefile template showing the full `prep`→`wpp`→`exe2bin` pipeline |
| `wftools/oaddump/oad.{cc,hp}` | Original C++ parser and display logic |

---

## Tool: `oas2oad-rs` — Linux OAS → OAD compiler

Replaces the Windows-only `wpp + wlink + exe2bin` chain with a Rust binary
that runs on Linux.

### Design note

The key property of the `.oas` → `prep` pipeline is that a single source
file produces two things in lockstep: the `.oad` binary and the C struct
declarations (`.ht` files) used by the game engine to interpret that binary.
They are guaranteed to agree because they come from the same `prep` run.

The Linux pipeline mirrors the original: compile `prep`'s C output with
`g++` and extract the `.data` section with `objcopy`. The binary layout is
therefore determined by the same compiler and the same struct definitions —
not reimplemented independently.

The fixups needed to compile `prep`'s Watcom-targeted output with `g++`:
- Strip the `huge` keyword (Watcom memory model, meaningless on Linux)
- Replace `pigtool.h` / `oad.h` with a portable equivalent using `int32_t`
  instead of `long` (which is 8 bytes on 64-bit Linux)
- Add `__attribute__((aligned(1)))` to `tempstruct` to suppress inter-variable
  alignment padding that would corrupt the `.data` layout
- Define `name_KIND=0` (`EActorKind` enum value from ungenerated `objects.h`)
- Define `DEFAULT_VISIBILITY=1` (prep default from `movebloc.inc`, absent in
  `mesh.oas` which doesn't pull in that include chain)
- Define `BITMAP_FILESPEC` / `MAP_FILESPEC` (prep string macros from
  `xdata.inc`, absent when `xdata.inc` isn't in the include chain)

### Pipeline

```
oas2oad-rs <name>.oas
  → prep -DTYPEFILE_OAS=<name> types3ds.s   (macro expansion)
  → strip 'huge', replace headers, fix alignment attr
  → g++ -c -x c++                           (compile to object file)
  → objcopy --only-section=.data -O binary  (extract raw .data → .oad)
```

`prep` must be on `$PATH` or specified via `--prep=<path>`.
`types3ds.s` defaults to `$WF_DIR/wfsource/source/oas/types3ds.s`
or `--types=<path>`. `g++` can be overridden with `--gpp=<path>`.

### CLI

```
oas2oad-rs [--prep=<path>] [--types=<path>] [--gpp=<path>] [-o <outfile>] <infile.oas>
```

Exit codes: `0` success, `1` error.

### Directory layout

```
oas2oad-rs/
  Cargo.toml
  src/
    main.rs   — CLI, fixups, orchestrates prep → g++ → objcopy
```

### Prerequisites delivered

- **`wftools/prep/macro.cc`** — one-line 64-bit portability fix: `unsigned delimiterIndex` → `std::string::size_type delimiterIndex` (truncation of `string::npos` on 64-bit caused named-parameter branch to fire for all tokens)
- **`wftools/prep/build.sh`** — captures working `g++ -std=c++14` command including `regexp/` sources

### Smoke test results — 41/41 pass

All `.oas` files in `wfsource/source/oas/` are standalone types (none are include-only).
Shared property blocks live in `.inc` files: `activate.inc` `actor.inc` `common.inc` `mesh.inc` `meter.inc` `movebloc.inc` `shadow.inc` `toolset.inc` `xdata.inc`

Bold = not referenced by any `.inc`; plain = has a corresponding `.inc` used by other `.oas` files.

Entry count = `(bytes − 80) ÷ 1491` (80-byte `_oadHeader` + 1491 bytes per `_typeDescriptor`).

| File | Size (bytes) | Entries |
|------|-------------|---------|
| **actbox** | 222,239 | 148 |
| **actboxor** | 207,329 | 138 |
| activate | 10,517 | 7 |
| actor | 190,928 | 128 |
| **alias** | 4,553 | 3 |
| **camera** | 204,347 | 136 |
| **camshot** | 235,658 | 157 |
| common | 20,954 | 14 |
| **destroyer** | 205,838 | 137 |
| **dir** | 192,419 | 129 |
| **director** | 192,419 | 129 |
| **disabled** | 1,571 | 1 |
| **enemy** | 192,419 | 129 |
| **explode** | 195,401 | 131 |
| **file** | 192,419 | 129 |
| **font** | 193,910 | 130 |
| **generator** | 211,802 | 141 |
| **handle** | 23,936 | 16 |
| **init** | 193,910 | 130 |
| **levelobj** | 402,650 | 269 |
| **light** | 204,347 | 136 |
| **matte** | 192,419 | 129 |
| mesh | 77,612 | 52 |
| meter | 210,311 | 140 |
| **missile** | 196,892 | 132 |
| movebloc | 61,211 | 41 |
| **movie** | 220,748 | 147 |
| **platform** | 192,419 | 129 |
| **player** | 192,419 | 129 |
| **pole** | 199,874 | 134 |
| **room** | 32,882 | 22 |
| shadow | 192,419 | 129 |
| **shadowp** | 6,044 | 4 |
| **shield** | 199,874 | 134 |
| **spike** | 205,838 | 137 |
| **statplat** | 192,419 | 129 |
| **target** | 190,928 | 128 |
| **template** | 192,419 | 129 |
| **tool** | 213,293 | 142 |
| toolset | 12,008 | 8 |
| **warp** | 205,838 | 137 |

### Testing

With `oas2oad-rs` working, `wf_oad` tests can use real `.oad` fixtures
generated from the `.oas` files in `wfsource/source/oas/`. Round-trip:

```bash
oas2oad-rs missile.oas -o /tmp/missile.oad
oaddump-rs /tmp/missile.oad
```
