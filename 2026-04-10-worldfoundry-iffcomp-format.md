# Investigation: World Foundry `iffcomp` File Format

**Date:** 2026-04-10
**Context:** Reverse-engineering the input and output formats of `iffcomp`, a flex/bison-based source-to-binary compiler from the [World Foundry](https://github.com/wbniv/WorldFoundry/tree/master/wftools/iffcomp) 3D engine project (1996–2003). It transforms a platform-independent textual description into a native-endian binary blob ready for direct load on the target hardware. Source: `wftools/iffcomp/` (lexer, grammar, sample) and `wfsource/source/iffwrite/` (the binary writer library).

There are really **two** formats here:

1. The textual *source* language consumed by `iffcomp` (`.iff.txt`) — platform-independent, human-readable.
2. The *binary* output — native-endian, ready for direct load on the target platform (x86 or MIPS R3000).

Both are documented below.

## 1. The textual source language

A small flex+bison DSL whose only output is a binary IFF tree.

### Top-level structure

```
file        := chunk+
chunk       := '{' ID chunk-body '}'
chunk-body  := ( chunk | item | directive )*
```

A file is a sequence of `{ … }` chunks; chunks may nest arbitrarily. Comments are `// to end of line`. `include "f"` and `include <f>` pull in nested source files.

### Chunk IDs (`'TEST'`, `'FILE'`, …)

- 1–4 ASCII characters in **single** quotes (lexer rule `'.{1,4}'`).
- The lexer packs them MSB-first into a 32-bit value (`shift = 24; …`), then `ID(unsigned long)` byte-swaps it back so the on-disk bytes are literally `T E S T` in source order. Shorter IDs are right-padded with NULs (`'AB'` → `41 42 00 00`).

### Items (the things that produce bytes)

| Syntax | Emits |
|---|---|
| `123`, `-456`, `$DEADBEEF` | An integer. Width = current `sizeOverride` (default 1 byte). |
| `123y`, `123w`, `123l` | Same, but force width to 1 / 2 / 4 bytes respectively. Hex form `$1234w` works too. |
| `'AB'`, `'WAVE'` | A 4-byte FOURCC inline (uses `out_id`, which 4-aligns first). |
| `3.14`, `.5`, `3.0` | A fixed-point real using the current default precision. |
| `3.14(1.15.16)` | A fixed-point real with explicit `(sign.whole.fraction)` precision. Total-bit count picks int8 / int16 / int32. |
| `"hello"` | NUL-terminated C string. |
| `"hello"(256)` | NUL-terminated C string padded with zeros to *exactly* N bytes. Warns if too long. |
| `"abc" "def"` | Concatenation — `out_string_continue` seeks back over the previous NUL and appends. Result is one C string `"abcdef\0"`. |
| `[ "path/to/file.bin" ]` | Raw bytes of an external file, copied verbatim. |
| `[ "f.bin" .start(N) .length(M) ]` | Same but only the slice `[N, N+M)`. |
| `.timestamp` | Unix `time_t` as a 32-bit int. |
| `.offsetof('A'::'B')` | Back-patched 32-bit absolute file offset of chunk `A`→`B`'s payload. |
| `.offsetof('A'::'B', N)` | Same plus a constant byte offset. |
| `.sizeof('A'::'B')` | Back-patched 32-bit payload size of chunk `A`→`B`. |

`.offsetof` / `.sizeof` use a `Backpatch` queue: if the named chunk is already known when the directive is parsed, the value is written immediately; otherwise the current file position is recorded and the gap is filled in `Grammar::~Grammar()` after parsing finishes.

### Directives

| Syntax | Effect |
|---|---|
| `.align(N)` | Pad with fill bytes until current file position ≡ 0 (mod N). |
| `.fillchar(N)` | Set the byte value used for alignment padding (default 0). |

### State-push blocks (the other meaning of `{ … }`)

A `{` followed by a *non*-CHAR_LITERAL token is a temporary state push, popped at the matching `}`. Two flavours:

- `{ Y … }`, `{ W … }`, `{ L … }` — push a default integer width of 1 / 2 / 4 bytes for unsuffixed integers inside the block. Note this is **uppercase** Y/W/L (single chars), distinct from the lowercase `y/w/l` that suffix individual literals.
- `{ .precision(1.15.16) … }` — push a default fixed-point precision for unparenthesised reals inside the block.

The parser disambiguates "chunk vs. state-push" by lookahead: if the next token is a CHAR_LITERAL it's a chunk, otherwise a state push.

### Numeric expressions

`expr` allows `item + item` and `item - item`, both fully implemented (`$$ = $1 + $3` and `$$ = $1 - $3`).

### Defaults

From `Grammar::construct`:

- Default size override: **1 byte** (so bare `123` writes one byte).
- Default precision: `1.15.16` (sign 1 bit, whole 15 bits, fraction 16 bits → 32-bit total).

## 2. The binary output format

Written by `IffWriterBinary` (`wfsource/source/iffwrite/binary.cc`). Standard EA-IFF in shape, with a couple of World Foundry quirks.

The chunk payloads are typically one of two things: the in-memory layout of C/C++ structs that define object attributes (loaded directly into engine data structures at runtime), or verbatim binary data such as TGA, BMP, and WAV files embedded whole. Native byte order follows naturally from the first case — the structs are read straight off disk into memory with no conversion.

### Chunk layout

```
+--------+--------+--------- … ---------+--- pad ---+
| ID (4) | size(4)|       payload       |  to mod 4 |
+--------+--------+--------- … ---------+-----------+
```

- `ID`: 4 raw ASCII bytes, source order (already byte-swapped in `ID::ID(unsigned long)`).
- `size`: **uint32** — payload size **excluding** the 8-byte header. Written via raw `_out->write` → **native byte order** (little-endian on x86 — *not* the big-endian of standard IFF).
- payload: nested chunks and/or raw items.
- After the payload, `align(4)` zero-pads to a 4-byte boundary.

`enterChunk` writes ID + a `0xFFFFFFFF` size placeholder; `exitChunk` seeks back and patches the real size, then aligns and bumps the parent chunk's accumulated size by `child.size + pad + 8`.

### Other primitives

| What | Bytes |
|---|---|
| `out_int8` | 1 byte |
| `out_int16` | 2 bytes, native (LE) |
| `out_int32` | 4 bytes, native (LE) |
| `out_id` | `align(4)` then 4 bytes |
| `out_string` | `strlen+1` bytes, NUL-terminated. Escape codes `\n \t \\ \" \NNN` are translated by `translate_escape_codes` first. |
| `out_string_continue` | seeks back -1 (over previous NUL), then `out_string` |
| `out_timestamp` | 4 bytes — `time_t` (Unix epoch seconds) — note: Y2038-broken |
| `out_file` | raw file bytes (`LoadBinaryFile` + `out_mem`) |
| `out_fixed` | `val × 2^fraction`, truncated to unsigned, width = 1 / 2 / 4 bytes by total bit count |

Alignment between siblings is forced to 4 (`align(4)`); the in-chunk `align()` writes zeros, while `alignFunction()` (the user-callable `.align(N)`) writes `fillChar()` instead.

### Endianness — native by design

The textual `.iff.txt` source *is* an interchange format in the meaningful sense: it is platform-independent and can be compiled for any target. The binary output, however, is not — it is a native blob consumed directly on the target platform (x86 or MIPS R3000). Numeric values — integers, sizes, fixed-point reals — are therefore written in the platform's **native byte order**, not the big-endian byte order of the original EA-IFF spec.

The one exception is the chunk `ID` field: `ID::ID(unsigned long)` byte-swaps so the on-disk bytes are literally the source-order ASCII characters (`'TEST'` → `54 45 53 54`), making ID matching byte-order-agnostic. All other multi-byte fields are raw native writes.

A standard IFF reader expecting big-endian data will parse IDs correctly but mis-decode every size and numeric field — expected, since that reader is not the intended consumer.

## 3. Walk-through of `test.iff.txt`

Source (from `wftools/iffcomp/test.iff.txt`):

```
{ 'TEST'
  3(1.15.16)
  0.3(1.15.16)
  0.4
  .5(1.15.16)
  "Hello string"
  "Hello string"(256)
  0y 1y 2y 3y
  4w 5w
  66666666l
  [ "TODO" ]
}
```

| Source | On-disk bytes (little-endian) | Notes |
|---|---|---|
| `{ 'TEST'` | `54 45 53 54  FF FF FF FF` | Chunk header `'TEST'` + size placeholder |
| `3(1.15.16)` | `00 00 03 00` | `(uint32)(3.0 × 2¹⁶)` = `0x00030000` |
| `0.3(1.15.16)` | `cc 4c 00 00` | `(uint32)(0.3 × 65536)` = 19660 = `0x00004CCC` |
| `0.4` | `66 66 00 00` | uses default precision 1.15.16 → `0x00006666` |
| `.5(1.15.16)` | `00 80 00 00` | `0.5 × 65536` = `0x00008000` |
| `"Hello string"` | `48 65 6c 6c 6f 20 73 74 72 69 6e 67 00` | 13 bytes, NUL-terminated |
| `"Hello string"(256)` | the same 13 bytes followed by 243 NULs | exactly 256 bytes total |
| `0y 1y 2y 3y` | `00 01 02 03` | four int8s |
| `4w 5w` | `04 00 05 00` | two int16s |
| `66666666l` | `2a fb f8 03` | int32 (`66666666 = 0x03F8FB2A`) |
| `[ "TODO" ]` | raw bytes of the local file `TODO` | |
| `}` | back-patches the chunk's size field | size = sum of the above |

Total payload size (header excluded): `4 + 4 + 4 + 4 + 13 + 256 + 4 + 4 + 4 + sizeof(TODO)` = `297 + sizeof(TODO)` bytes, written little-endian into the placeholder slot.

## 4. Walk-through of `iffwrite/test.iff.txt`

Source (from `wfsource/source/iffwrite/test.iff.txt`):

```
{ 'FILE'
{ 'TEST' "Some text..." }
}
```

Produces a nested chunk:

```
54 45 53 54  18 00 00 00     ; parent 'FILE', size = 24 (inner header + payload + pad)
'F''I''L''E'                 ;
                             ;
  54 45 53 54  0d 00 00 00   ; inner 'TEST', size = 13 (the C string with NUL)
  53 6f 6d 65 20 74 65 78    ; "Some text..."
  74 2e 2e 2e 00             ;
  00 00 00                   ; 3 bytes pad to 4-byte boundary
```

Note the parent chunk's size in the header *includes* the inner chunk's full 8-byte header and its 4-byte trailing pad — that's exactly what `IffWriterBinary::exitChunk` rolls into the parent (`csParent->AddToSize(child.size + pad + 8)`).

## 5. WF vs EA-IFF 85 deviations

The file produced by `iffcomp` shares the EA-IFF chunk-header shape but deviates in several ways. Because the output is a platform blob (not an interchange file), the deviations are intentional.

### Structural / breaking

| # | EA-IFF 85 | WorldFoundry |
|---|---|---|
| 1 | All multi-byte values big-endian | All numeric fields in **native byte order** (little-endian on x86, native on MIPS R3000) |
| 2 | Chunks padded to **2-byte** boundaries | Everything except strings aligned to **4-byte** boundaries: chunks on exit (`exitChunk`) and inline FOURCC literals (`out_id`) both call `align(4)`. Required for direct in-place load on MIPS R3000 (unaligned 32-bit loads are a bus error) and beneficial on x86 (avoids the misaligned-read penalty). Strings are exempt — they are accessed byte-by-byte so alignment is irrelevant. |

### Extensions

| # | EA-IFF 85 | WorldFoundry |
|---|---|---|
| 5 | No typed scalars | Fixed-point reals: `val × 2^fraction`, packed into int8/int16/int32 by total bit width |
| 6 | No string convention | C-style **NUL-terminated** strings; escape sequences (`\n \t \\ \" \NNN`) translated at write time. Adjacent string literals (`"hello" "world"`) are concatenated into one C string via `out_string_continue` (seeks back over the previous NUL and appends). |
| 7 | No back-patching | `.offsetof('A'::'B')` / `.sizeof('A'::'B')` inject 32-bit absolute file offsets/sizes, resolved via `Backpatch` queue |

### Note on IDs

Chunk `ID` bytes are the one field that matches standard IFF byte order. `ID::ID(unsigned long)` byte-swaps the packed value so the on-disk bytes are the literal source-order ASCII characters — the same bytes a big-endian IFF reader would expect. This makes chunk-type matching portable even though everything else is native-endian.

## TL;DR

- **Source format**: a tiny scripting DSL whose only output is a binary IFF tree. Chunks are `{ 'ID' … }`, items are width-tagged numeric / string / file literals, with directives for alignment, timestamps, and back-patched offsets / sizes into other chunks.
- **Output format**: standard EA-IFF chunk header (`ID(4) | size(4) | payload`) with all numeric fields in **native byte order** (little-endian on x86, native on MIPS R3000) — intentional, as the output is a platform blob, not an interchange file. `align(4)` pads to 4-byte boundaries between siblings. Strings are C-style NUL-terminated. Fixed-point reals are `val << fraction_bits` truncated to int8 / int16 / int32 by their total bit width.

## References

- Tool source: [`wbniv/WorldFoundry/wftools/iffcomp/`](https://github.com/wbniv/WorldFoundry/tree/master/wftools/iffcomp)
  - `lang.l` — flex lexer
  - `lang.y` — bison grammar
  - `grammar.cc` / `grammar.hpp` — parser driver, `Backpatch`, `State`
  - `test.iff.txt` — sample 1
- Writer library: [`wbniv/WorldFoundry/wfsource/source/iffwrite/`](https://github.com/wbniv/WorldFoundry/tree/master/wfsource/source/iffwrite)
  - `iffwrite.hp` — abstract base + `IffWriterBinary` / `IffWriterText` declarations
  - `binary.cc` — binary writer (chunk header layout, alignment, back-patching)
  - `_iffwr.cc` — base writer + `enterChunk` / `exitChunk` accounting
  - `fixed.cc` / `fixed.hp` — fixed-point conversion
  - `id.hp` — FOURCC byte-swap
  - `backpatch.hp` — `ChunkSizeBackpatch` (size + position bookkeeping)
  - `test.iff.txt` — sample 2
- EA-IFF 85 spec: ["EA IFF 85" Standard for Interchange Format Files](https://wiki.amigaos.net/wiki/EA_IFF_85_Standard_for_Interchange_Format_Files) — original spec on the AmigaOS wiki
