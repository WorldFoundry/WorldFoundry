# Plan: Rust ports of wftools command-line tools

## Which tools, and why

| Tool | C++ LOC | Verdict | Rationale |
|------|---------|---------|-----------|
| **iffdump** | ~372 | **Port first** | Natural companion to iffcomp-rs; IFF binary format already understood; all deps are trivial stubs |
| **oaddump** | ~546 | Port second | Self-contained OAD parser; small; no external deps |
| eval | ~53 (CLI) + ~400 (grammar) | Blender work | Expression evaluator grammar (`wfsource/source/eval/`) needed by `wf_attr_validate`; belongs with Blender integration, not here |
| prep | ~1 615 | Maybe later | Custom tokenizer + macro expander; interesting but larger scope |
| chargrab / textile | ~4 000+ | Maybe later | Image packing tools; Windows I/O mixed in; bigger effort |
| iff2lvl | ~8 731 | No | Massive 3D level converter; hardcoded paths; PSX/Saturn targets |
| attribedit | ~3 517 | No | GTK+ 2.x GUI; not portable without full UI rewrite |
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

**No Linux pathway exists** to produce `.oad` files today — the Watcom
toolchain (`wpp`, `wlink`, `exe2bin`) is Windows-only. Test fixtures in
`wf_oad` are synthesized directly from the known binary layout.

The Linux replacement is `oas2oad-rs` (see below).

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
that runs on Linux. Uses `prep` (already working on Linux) for macro
expansion, then parses the generated C initializer syntax and writes the
binary `.oad` directly.

### Why Option B (prep + C parser) over Option A (direct OAS parser)

Reimplementing `prep`'s macro expansion for the OAD domain would require
testing correctness independently. Using the existing `prep` binary means
macro expansion is already validated — we only add a C-initializer parser
on top, which has a much smaller surface area.

### Pipeline

```
oas2oad-rs <name>.oas → (shells out) prep -DTYPEFILE_OAS=<name> types3ds.s → <name>.pp
                       → parse C initializers from <name>.pp
                       → write binary <name>.oad
```

The `prep` binary must be on `$PATH` or specified via `--prep=<path>`.
`types3ds.s` location defaults to `$WF_DIR/wfsource/source/oas/types3ds.s`
or can be overridden with `--types=<path>`.

### What the C output looks like

`prep` with `types3ds.s` produces two initializers:

```c
_oadHeader header = {'OAD ', 0, "DisplayName", 0x00010202};

typeDescriptor huge tempstruct[] = {
    { BUTTON_INT32, "speed", 0, 65536, 0, 0, "", SHOW_AS_NUMBER,
      -1, -1, "help text", {XDATA_IGNORE, 0, "Speed", "1", 0} },
    ...
};
```

The parser needs to handle:
- String literals (including escaped chars)
- Integer literals (decimal, hex `0x...`)
- Named constants (`BUTTON_INT32`, `SHOW_AS_NUMBER`, `XDATA_IGNORE`, etc.)
- Nested `{ ... }` for the union field
- Multi-char literals (`'OAD '`) — only in the header, value is known

### CLI

```
oas2oad-rs [--prep=<path>] [--types=<path>] [-o <outfile>] <infile.oas>
```

Exit codes: `0` success, `1` error.

### Directory layout

```
oas2oad-rs/
  Cargo.toml
  src/
    main.rs     — CLI, temp file management, shells out to prep
    parser.rs   — tokenizer + C initializer parser → OadHeader + Vec<OadEntry>
    error.rs    — OasError enum
```

Depends on `wf_oad` for `OadHeader`, `OadEntry`, and the binary serializer
(which needs to be added to `wf_oad` alongside the existing reader).

### Testing

With `oas2oad-rs` working, `wf_oad` tests can use real `.oad` fixtures
generated from the `.oas` files in `wfsource/source/oas/`. Round-trip:

```bash
oas2oad-rs missile.oas -o /tmp/missile.oad
oaddump-rs /tmp/missile.oad
```
