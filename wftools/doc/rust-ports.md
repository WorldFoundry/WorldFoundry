# Plan: Rust ports of wftools command-line tools

## Which tools, and why

| Tool | C++ LOC | Verdict | Rationale |
|------|---------|---------|-----------|
| **oas2oad** | ~400 (wpp+wlink chain) | **Done** | `prep` → `g++` → `objcopy .data`; same guarantee as original toolchain — 41/41 .oas files pass |
| **iffdump** | ~372 | **Done** | Natural companion to iffcomp-rs; IFF binary format already understood; all deps are trivial stubs |
| **oaddump** | ~546 | **Done** | Self-contained OAD parser; small; no external deps |
| eval | ~53 (CLI) + ~400 (grammar) | Blender work | Expression evaluator grammar needed by `wf_attr_validate`; belongs with Blender integration, not here |
| prep | ~1 615 | Maybe later | Custom tokenizer + macro expander; interesting but larger scope |
| chargrab / textile | ~4 000+ | Maybe later | Image packing tools; Windows I/O mixed in; bigger effort |
| iff2lvl | ~8 731 | No | Massive 3D level converter; hardcoded paths; PSX/Saturn targets |
| attribedit | ~3 517 | Blender plugin | GTK+ 2.x standalone OAD property editor — the reference implementation for what the Blender plugin needs to do |
| iffdb | ~536 | Superseded | Alternative iffdump using in-memory IFF tree; `iffdump-rs` renders it redundant |
| lvldump | ~1 600 | Defer | Reads compiled `.lvl` game-level binaries; formats in `.oas` files, renderable by existing `prep`; port after oaddump-rs |

---

## Completed: `oas2oad-rs`

Linux replacement for the Windows-only `wpp + wlink + exe2bin` chain.
Uses `g++` + `objcopy` — the compiler handles struct layout, preserving
the same guarantee as the original pipeline.

### Prerequisites delivered

- **`wftools/prep/macro.cc`** — one-line 64-bit portability fix: `unsigned delimiterIndex` → `std::string::size_type delimiterIndex` (truncation of `string::npos` caused named-parameter branch to fire for all tokens)
- **`wftools/prep/build.sh`** — captures working `g++ -std=c++14` command including `regexp/` sources

### Fixups applied to prep output before g++ compilation

- Strip Watcom `huge` keyword
- Replace `pigtool.h`/`oad.h` with a portable header using `int32_t` (not `long`)
- `__attribute__((aligned(1)))` on `tempstruct` to suppress inter-variable padding
- `-Dname_KIND=0`, `-DDEFAULT_VISIBILITY=1`, `BITMAP_FILESPEC`/`MAP_FILESPEC` for missing context-dependent prep macros

### Sample invocation and output

```
$ oas2oad --prep=prep --types=wfsource/source/oas/types3ds.s \
    -o /tmp/missile.oad wfsource/source/oas/missile.oas
$ ls -l /tmp/missile.oad
-rw-rw-r-- 1 will will 196892 Apr 13 13:26 /tmp/missile.oad
```

(No output on success; exit 1 with message on error.)

### Smoke test results — 41/41 pass

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

Entry count = `(bytes − 80) ÷ 1491` (80-byte `_oadHeader` + 1491 bytes per `_typeDescriptor`).
All 41 `.oas` files are standalone types. Shared property blocks live in `.inc` files:

`activate.inc` `actor.inc` `common.inc` `mesh.inc` `meter.inc` `movebloc.inc` `shadow.inc` `toolset.inc` `xdata.inc`

---

## Completed: `iffdump-rs`

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
iffdump [-c<chunks_file>] [-d+|-d-] [-f+|-f-] [-w=<width>] <infile> [outfile]
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

### Sample output — pretty mode (`-f+`, default)

```
$ iffdump testdata/expected.iff
//=============================================================================
// <stdout> Created by iffdump v0.1.0
//=============================================================================
{ 'TEST'  	// Size = 283
	0000: 00000300 CC4C0000 66660000 00800000    .....L..ff......
	0010: 48656C6C 6F207374 72696E67 48656C6C    Hello stringHell
	0020: 6F207374 72696E67 00000102 03040005    o string........
	0030: 00AA40F9 03333020 73657020 32303032    ..@..30 sep 2002
	0040: 0D0A2D2D 2D2D2D2D 2D2D2D2D 2D2D2D2D    ..--------------
	...
	0110: 206E756D 62657273 290D0A                numbers)..
}
```

### Sample output — IFFComp source mode (`-f-`)

```
$ iffdump -f- testdata/expected.iff
//=============================================================================
// <stdout> Created by iffdump v0.1.0
//=============================================================================
{ 'TEST'  	// Size = 283
	$00 $00 $03 $00
	$CC $4C $00 $00
	$66 $66 $00 $00
	$00 $80 $00 $00
	$48 $65 $6C $6C
	...
}
```

### Directory layout

```
iffdump-rs/
  Cargo.toml
  src/
    main.rs      — CLI arg parsing, open files, call lib
    lib.rs       — dump() public entry; re-exports
    reader.rs    — dump_chunks(), read_chunk_header(), id_name(), parse_chunk_list()
    dump.rs      — format_hdump(), format_iffcomp()
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

### Testing

```bash
# Round-trip: iffdump -f- output should recompile to identical bytes
./iffdump -f- testdata/expected.iff > /tmp/round.iff.txt
./iffcomp -binary -o /tmp/round.iff /tmp/round.iff.txt
diff /tmp/round.iff testdata/expected.iff   # zero diff (modulo timestamp mask)

# all_features fixture
./iffdump -f- testdata/all_features.iff > /tmp/af_rt.iff.txt
./iffcomp -binary -o /tmp/af_rt.iff /tmp/af_rt.iff.txt
diff /tmp/af_rt.iff testdata/all_features.iff
```

`cargo test` result:

```
running 2 tests
test round_trip_expected ... ok
test round_trip_all_features ... ok

test result: ok. 2 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
```

---

## Completed: `oaddump-rs`

OAD (Object Attribute Data) files describe game object properties.
Implemented as two crates: `wf_oad` (library) + `oaddump-rs` (CLI).

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
| `_oadHeader` | 80 bytes | magic `OAD ` (LE u32 = 0x4F414420), chunkSize, name[68], version |
| `_typeDescriptor` × N | 1491 bytes each | one entry per OAD field |

`oas2oad-rs` (see above) provides the Linux replacement using `prep` → `g++` → `objcopy`.

### Sample output — `disabled.oad` (1 entry, simplest case)

```
$ oas2oad --prep=prep --types=.../types3ds.s -o /tmp/disabled.oad disabled.oas
$ oaddump /tmp/disabled.oad
Disabled

   Type: LEVELCONFLAG_NOINSTANCES <9>
   Name: LEVELCONFLAG_NOINSTANCES (length = 24)
Display: 
    Min: 0	0
    Max: 0	0
Default: 0
  String:
 ShowAs: 6
```

### Sample output — `alias.oad` (3 entries)

```
$ oaddump /tmp/alias.oad
Alias

   Type: BUTTON_PROPERTY_SHEET <8>
   Name: Alias (length = 5)
Display: Alias
    Min: 0	0
    Max: 1	0.0000152587890625
Default: 1
  String:
 ShowAs: 0

   Type: BUTTON_OBJECT_REFERENCE <6>
   Name: Base Object (length = 11)
Display: Base Object
    Min: 0	0
    Max: 0	0
Default: 0
  String:
 ShowAs: 0

   Type: LEVELCONFLAG_SHORTCUT <28>
   Name: LEVELCONFLAG_SHORTCUT (length = 21)
Display: 
    Min: 0	0
    Max: 0	0
Default: 0
  String:
 ShowAs: 6
```

### Sample output — `missile.oad` (132 entries, first few)

```
$ oaddump /tmp/missile.oad | head -30
Missile

   Type: LEVELCONFLAG_COMMONBLOCK <17>
   Name: movebloc (length = 8)
Display: 
    Min: 0	0
    Max: 0	0
Default: 0
  String:
 ShowAs: 6

   Type: BUTTON_PROPERTY_SHEET <8>
   Name: Movement (length = 8)
Display: Movement
    Min: 0	0
    Max: 1	0.0000152587890625
Default: 0
  String:
 ShowAs: 0

   Type: BUTTON_INT32 <4>
   Name: MovementClass (length = 13)
Display: Movement Class
    Min: 0	0
    Max: 100	0.00152587890625
Default: 0
  String:
 ShowAs: 1
```

### `cargo test` result

```
$ cargo test --manifest-path wf_oad/Cargo.toml
running 3 tests
test tests::round_trip_empty ... ok
test tests::bad_magic_rejected ... ok
test tests::round_trip_one_entry ... ok

test result: ok. 3 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
```

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
oas2oad <name>.oas
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
oas2oad [--prep=<path>] [--types=<path>] [--gpp=<path>] [-o <outfile>] <infile.oas>
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
