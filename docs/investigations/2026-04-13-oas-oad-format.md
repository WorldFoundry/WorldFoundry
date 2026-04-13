# OAS and OAD file formats

**Date:** 2026-04-13  
**Sources:** `wfsource/source/oas/oad.h`, `wfsource/source/oas/types3ds.s`,
`wfsource/source/oas/*.oas`, `wftools/oas2oad-rs/src/main.rs`

---

## OAS files

OAS (Object Attribute Source) files are the source format for World Foundry object
schemas.  Each `.oas` file defines one object type — its editable properties, their
types, defaults, ranges, UI hints, and level-converter flags.

OAS files are preprocessed by `prep` (a custom macro expander), which expands macros
and resolves `@include` directives.  The expanded output is C source code that
initialises a `typeDescriptor[]` array.  That C source is compiled with `g++` and the
`.data` section is extracted as the binary `.oad` file.

### File structure

```
@* this is a comment

@define DEFAULT_MASS 50.0          ← preprocessor constant

@include xdata.inc                 ← pull in shared definitions

TYPEHEADER(Player)                 ← open the type; writes _oadHeader
    @include actor.inc             ← pull in common blocks (movement, mesh, etc.)

    PROPERTY_SHEET_HEADER(Player, 1)   ← begin a collapsible section
    TYPEENTRYINT32(hp, Hit Points, 0, 100, 1)
    TYPEENTRYFIXED32(Mass, , FIXED32(0), FIXED32(100), FIXED32(50))
    PROPERTY_SHEET_FOOTER          ← end section (no-op macro)

TYPEFOOTER                         ← close the type
```

### Macro system (`types3ds.s`)

All macros expand to a `typeDescriptor` struct literal (with a trailing comma).
`types3ds.s` is included by `prep` before the `.oas` file.  The key macros are:

| Macro | ButtonType used | Notes |
|-------|----------------|-------|
| `TYPEHEADER(displayName)` | — | Writes `_oadHeader` + opens `typeDescriptor[]` |
| `TYPEFOOTER` | — | Closes the array |
| `PROPERTY_SHEET_HEADER(name, active=0)` | `BUTTON_PROPERTY_SHEET` | `active`: 1=expanded by default |
| `PROPERTY_SHEET_FOOTER` | — | No-op (section end is implicit) |
| `GROUP_START(name)` | `BUTTON_GROUP_START` | Non-collapsible sub-box |
| `GROUP_STOP()` | `BUTTON_GROUP_STOP` | Close sub-box |
| `TYPEENTRYINT32(name, displayName, min, max, def, buttons, showas, help, szEnableExpr)` | `BUTTON_INT32` | `buttons`: pipe-separated enum items |
| `TYPEENTRYFIXED32(name, displayName, min, max, def, showas, help, szEnableExpr)` | `BUTTON_FIXED32` | Values use `FIXED32(n)` = `(long)(n * 65536)` |
| `TYPEENTRYBOOLEAN(name, displayName, def)` | `BUTTON_INT32` | `string = "False\|True"`, `showas = SHOW_AS_CHECKBOX` |
| `TYPEENTRYBOOLEANTOGGLE(name, displayName, def, buttons)` | `BUTTON_INT32` | `showas = SHOW_AS_RADIOBUTTONS` |
| `TYPEENTRYCOLOR(name, displayName, def)` | `BUTTON_INT32` | `min=0`, `max=0xFFFFFF`, `showas = SHOW_AS_COLOR` |
| `TYPEENTRYFILENAME(name, displayName, filespec, help, szEnableExpr)` | `BUTTON_FILENAME` | `filespec` → `lpstrFilter`; null-terminated pairs |
| `TYPEENTRYOBJREFERENCE(name, displayName, help, szEnableExpr)` | `BUTTON_OBJECT_REFERENCE` | `len=12` |
| `TYPEENTRYCAMERAREFERENCE(name, displayName)` | `BUTTON_CAMERA_REFERENCE` | `len=12` |
| `TYPEENTRYLIGHTREFERENCE(name, displayName)` | `BUTTON_LIGHT_REFERENCE` | `len=12` |
| `TYPEENTRYCLASSREFERENCE(name, displayName, def)` | `BUTTON_CLASS_REFERENCE` | `def` = class name in `string` |
| `TYPEENTRYVECTOR(name, displayName, min, max, def)` | `BUTTON_FIXED32` | Appends `X` to name; caller repeats for Y/Z |
| `TYPEENTRYCAMERA(name, displayName, followObj)` | `BUTTON_EXTRACT_CAMERA` | Camera extraction record |
| `TYPEENTRYWAVEFORM(name, displayName)` | `BUTTON_WAVEFORM` | |
| `TYPEENTRYSTRING(name, displayName, count, labels, showas)` | `BUTTON_XDATA` | In-memory string, not serialised |
| `TYPEENTRYSTRING_IGNORE(name, displayName)` | `BUTTON_XDATA` | `XDATA_IGNORE` — notes/comments field |
| `TYPEENTRYXDATA(name, displayName, chunkName)` | `BUTTON_XDATA` | XData chunk; `chunkName` stored in `string` field; `conversionAction=XDATA_IGNORE` |
| `TYPEENTRYXDATA_CONVERT(name, displayName, chunkName, required, conversion)` | `BUTTON_XDATA` | Level-converter data conversion |
| `LEVELCONFLAGCOMMONBLOCK(name)` | `LEVELCONFLAG_COMMONBLOCK` | Inserts flag + `@include name.inc` |
| `LEVELCONFLAGENDCOMMON` | `LEVELCONFLAG_ENDCOMMON` | Closes common block |
| `LEVELCONFLAGROOM` | `LEVELCONFLAG_ROOM` | Marks object as a room |
| `LEVELCONFLAGNOINSTANCES` | `LEVELCONFLAG_NOINSTANCES` | Prevents instancing |
| `LEVELCONFLAGNOMESH` | `LEVELCONFLAG_NOMESH` | Object has no mesh |
| `LEVELCONFLAGSHORTCUT` | `LEVELCONFLAG_SHORTCUT` | Shortcut/alias object |
| `LEVELCONFLAGEXTRACTLIGHT` | `LEVELCONFLAG_EXTRACTLIGHT` | Extract 3DS light data |
| `LEVELCONFLAGEXTRACTCAMERANEW` | `LEVELCONFLAG_EXTRACTCAMERANEW` | New-style camera extraction |

#### Macro parameter conventions

- `name` — internal C struct member name (no spaces)
- `displayName=name` — user-visible label (can have spaces; defaults to `name`)
- `min`, `max` — inclusive range; for FIXED32 use `FIXED32(n)` = `(long)(n * 65536)`
- `def=min` — default value; defaults to `min` when omitted
- `buttons=""` — pipe-separated enum item labels, e.g. `"Box|Mesh|Scarecrow|None"`
- `showas=SHOW_AS_NUMBER` — UI widget hint (see showAs table below)
- `help=""` — tooltip shown in attribedit
- `szEnableExpression="1"` — expression controlling when the field is editable,
  e.g. `"ModelType==1 || ModelType==2"`; `"1"` = always enabled
- `x=-1, y=-1` — screen coordinates; `-1` = auto-place

#### File specs (`lpstrFilter`)

`TYPEENTRYFILENAME` passes a Windows file-dialog filter as the last field of the struct.
Format: null-separated pairs of `Description\0Pattern\0`, terminated by `\0\0`.

```c
"World Foundry IFF (*.iff)\0*.iff\0"
"Windows Bitmap Files (*.bmp)\0*.bmp\0"
"Targa Bitmap Files (*.tga)\0*.tga\0"
"SGI Bitmap Files (*.rgb,*.rgba,*.bw)\0*.rgb;*.rgba;*.bw\0"
"\0\0"
```

Common predefined specs (from `xdata.inc`):
- `BITMAP_FILESPEC` — BMP + TGA + SGI (used alongside IFF for mesh/sprite fields)
- `MAP_FILESPEC` — scrolling map files (`*.map`)
- `ALLFILES_FILESPEC` — `*.*`

### Common include files

| File | Content |
|------|---------|
| `actor.inc` | Pulls in `movebloc`, `toolset`, `common` blocks; adds `Mesh Name` filename field |
| `mesh.inc` | Model Type enum, Animation/Visibility mailboxes, Matte/Emitter groups |
| `common.inc` | Health, AI script, slope data, contextual animations |
| `movebloc.inc` | Movement class, mass, acceleration, velocity limits |
| `toolset.inc` | Six tool-slot object references (Tool A–F) |
| `shadow.inc` | Shadow rendering settings |
| `flagbloc.inc` | Template object flag |
| `activate.inc` | Activation trigger fields |
| `meter.inc` | HUD meter display fields |
| `xdata.inc` | XDATA helpers; defines `BITMAP_FILESPEC`, `MAP_FILESPEC`, `ALLFILES_FILESPEC` |

### Example — `room.oas`

```
TYPEHEADER(Room, room)
    LEVELCONFLAGCOMMONBLOCK(common)     ← inserts LEVELCONFLAG_COMMONBLOCK + @include common.inc
    LEVELCONFLAGROOM                    ← inserts LEVELCONFLAG_ROOM flag entry

    PROPERTY_SHEET_HEADER(Room, 1)
    GROUP_START(Adjacent Rooms)
        TYPEENTRYOBJREFERENCE(Adjacent Room 1, Room)
        TYPEENTRYOBJREFERENCE(Adjacent Room 2, Room)
    GROUP_STOP()
    TYPEENTRYINT32(Room Loaded Mailbox,, 0, 3999, 0, "", SHOW_AS_NUMBER)
    PROPERTY_SHEET_FOOTER
TYPEFOOTER
```

### Example — `alias.oas`

```
TYPEHEADER(Alias)
    PROPERTY_SHEET_HEADER(Alias, 1)
    TYPEENTRYOBJREFERENCE(Base Object)
    LEVELCONFLAGSHORTCUT
    PROPERTY_SHEET_FOOTER
TYPEFOOTER
```

---

## OAD files

An OAD (Object Attribute Descriptor) file is the compiled binary form of an OAS file.
It is a flat binary with a fixed-size header followed by a packed array of
`typeDescriptor` structs, one per field entry (including section headers and flags).

All integers are little-endian.  The entire struct array uses `#pragma pack(1)` —
no padding between fields.

### File layout

```
Offset  Size   Field
──────  ─────  ──────────────────────────────────────────────────────
0       4      chunkId: 'OAD ' (0x4F 0x41 0x44 0x20, big-endian chars)
4       4      chunkSize: total byte size of the file
8       68     name: display name of the object type, null-terminated
76      4      version: 0x00010202
80      …      typeDescriptor[0], typeDescriptor[1], …
```

Header total: **80 bytes.**  
Each `typeDescriptor`: **1491 bytes** (packed).  
Entry count: `(fileSize − 80) / 1491`.

### `typeDescriptor` struct (1491 bytes, packed)

From `wfsource/source/oas/oad.h`:

```c
typedef struct _typeDescriptor            // 1491 bytes total, #pragma pack(1)
{
    buttonType      type;                 //   1  field type (see button type table)
    char            name[64];            //  64  internal key / C struct member name
    int32           min;                 //   4  minimum raw value
    int32           max;                 //   4  maximum raw value
    int32           def;                 //   4  default raw value
    int16           len;                 //   2  string length hint (reference fields: 12)
    char            string[512];         // 512  pipe-separated enum items; class tag for ObjRef
    visualRepresentation showAs;         //   1  UI widget hint (see showAs table)
    int16           x, y;               //   4  screen position (-1 = auto)
    char            helpMessage[128];    // 128  tooltip text
    union {
        struct {
            EConversionAction conversionAction;  //   1  XDATA handling
            int32   bRequired;                   //   4  1 = required
            char    displayName[64];             //  64  user-visible label
            char    szEnableExpression[128];     // 128  enable condition ("1" = always)
            int32   rollUpLength;                //   4  property sheet height hint
        } xdata;                                 // 201 bytes
        struct { char pad[255]; } pad;           // 255 bytes
    };                                           // 255 bytes (union is max of members)
    char            lpstrFilter[512];    // 512  Windows file-dialog filter (BUTTON_FILENAME)
} typeDescriptor;
// 1 + 64 + 4 + 4 + 4 + 2 + 512 + 1 + 4 + 128 + 255 + 512 = 1491
```

#### Field notes

- **`type`** — one byte; values from the button type table below.
- **`name`** — null-terminated; the internal key used in code and in `.iff.txt` comments.
- **`min` / `max`** — raw `int32`.  For `BUTTON_FIXED32`, divide by 65536 to get the
  display float.  For `BUTTON_FIXED16`, divide by 256.  For levelcon flags: both are 0.
- **`def`** — raw `int32` default.  For `BUTTON_FIXED32`: `FIXED32(n)` = `(long)(n * 65536)`.
  For `BUTTON_INT32` enum fields: index into the `string` pipe-list.
- **`len`** — used as a length hint for reference types; set to 12 for
  `BUTTON_OBJECT_REFERENCE`, `BUTTON_CAMERA_REFERENCE`, `BUTTON_LIGHT_REFERENCE`,
  `BUTTON_CLASS_REFERENCE`.  0 otherwise.
- **`string`** — purpose varies by type:
  - `BUTTON_INT32` with enum items: pipe-separated labels, e.g. `"Box|Mesh|Scarecrow|None"`
  - `BUTTON_CLASS_REFERENCE`: class name filter, e.g. `"Room"`
  - `BUTTON_BOOLEAN` helpers: `"False|True"`
  - All others: empty.
- **`showAs`** — one byte; widget hint (see showAs table below).
- **`x`, `y`** — screen coordinates for attribedit layout; `-1` = auto-place.
- **`helpMessage`** — null-terminated tooltip string.
- **`xdata.displayName`** — the user-visible field label shown in attribedit.
  Distinct from `name` (the internal C identifier).
- **`xdata.szEnableExpression`** — conditional enable expression evaluated by attribedit,
  e.g. `"ModelType==1 || ModelType==2"`.  `"1"` = always enabled.
- **`xdata.conversionAction`** — `XDATA_IGNORE`(0) for most fields; non-zero for
  `BUTTON_XDATA` fields, directing levelcon how to emit the XData chunk
  (copy raw bytes, parse as object list, compile as AI script, etc.).
- **`lpstrFilter`** — for `BUTTON_FILENAME`: Windows file-dialog filter string
  (null-separated `Desc\0Pattern\0` pairs, double-null terminated).  Empty otherwise.

#### Fixed-point macros

```c
#define FIXED16(n)  ((short)(n * 256))     // 8.8  fixed-point; scale = 256
#define FIXED32(n)  ((long)(n * 65536))    // 16.16 fixed-point; scale = 65536
```

To convert a raw stored value to a display float: `display = raw / scale`.

---

## OAS → OAD conversion

The conversion is performed by `wftools/oas2oad-rs/src/main.rs`.  It replaces the
original Windows-only `wpp + wlink + exe2bin` chain.

### Pipeline

```
player.oas
    │
    ▼  prep -DTYPEFILE_OAS=player types3ds.s
    │  (macro expander — expands @define / @include / @ifdef)
    │
player.pp   ← C source with struct initialisers
    │
    ▼  fixup pass (oas2oad-rs)
    │  • strip Watcom `huge` keyword
    │  • replace #include "pigtool.h" / "oad.h" with portable oas_compat.h
    │  • add __attribute__((aligned(1))) to tempstruct
    │  • add -Dname_KIND=0 -DDEFAULT_VISIBILITY=1 and other context macros
    │
player.fixed.cpp
    │
    ▼  g++ -c -x c++ -w -Ioas_compat/ player.fixed.cpp
    │
player.o
    │
    ▼  objcopy --only-section=.data -O binary player.o player.oad
    │
player.oad   ← binary OAD, ready for use
```

The compiler is responsible for struct layout.  Using `g++` with `#pragma pack(1)`
ensures the output is byte-identical to what the original Watcom compiler produced.

### Why not parse OAS directly?

The OAS macro system is essentially a C preprocessor dialect; `prep` has enough
quirks (named parameters with defaults, `@ifdef` guards, `@+concat@+` syntax) that
reimplementing it cleanly is more work than piggy-backing on a real C compiler for
struct layout.  The `g++` approach also guarantees the field offsets stay correct if
the struct ever gains new fields.

---

## Button type table

`buttonType` is stored as a single byte (`typedef char buttonType`).  The enum is
defined in `oad.h`; the constant names are `#define`s, not a real C enum.

| # | Symbol | Used via macro | Serialised bytes | Typical `showAs` values |
|---|--------|---------------|-----------------|------------------------|
| 0 | `BUTTON_FIXED16` | `TYPEENTRYVECTOR`, direct | 2 (LE int16) | `SHOW_AS_N_A`, `SHOW_AS_SLIDER` |
| 1 | `BUTTON_FIXED32` | `TYPEENTRYFIXED32`, `TYPEENTRYVECTOR` | 4 (LE int32) | `SHOW_AS_N_A`, `SHOW_AS_SLIDER` |
| 2 | `BUTTON_INT8` | (rare; no dedicated macro) | 1 (LE int8) | `SHOW_AS_NUMBER` |
| 3 | `BUTTON_INT16` | (rare; no dedicated macro) | 2 (LE int16) | `SHOW_AS_NUMBER` |
| 4 | `BUTTON_INT32` | `TYPEENTRYINT32`, `TYPEENTRYBOOLEAN`, `TYPEENTRYCOLOR` | 4 (LE int32) | `SHOW_AS_NUMBER`, `SHOW_AS_DROPMENU`, `SHOW_AS_RADIOBUTTONS`, `SHOW_AS_CHECKBOX`, `SHOW_AS_COLOR`, `SHOW_AS_SLIDER` |
| 5 | `BUTTON_STRING` | `TYPEENTRYSTRING` (with `SHOW_AS_TEXTEDITOR`) | variable (null-terminated) | `SHOW_AS_TEXTEDITOR`, `SHOW_AS_N_A` |
| 6 | `BUTTON_OBJECT_REFERENCE` | `TYPEENTRYOBJREFERENCE` | 4 (LE index, 0=unresolved) | `SHOW_AS_N_A` |
| 7 | `BUTTON_FILENAME` | `TYPEENTRYFILENAME` | 0 (path string, not in binary) | `SHOW_AS_N_A`, `SHOW_AS_FILENAME` |
| 8 | `BUTTON_PROPERTY_SHEET` | `PROPERTY_SHEET_HEADER` | 0 (structural) | `SHOW_AS_N_A` |
| 9 | `LEVELCONFLAG_NOINSTANCES` | `LEVELCONFLAGNOINSTANCES` | 4 | `SHOW_AS_HIDDEN` |
| 10 | `LEVELCONFLAG_NOMESH` | `LEVELCONFLAGNOMESH` | 4 | `SHOW_AS_HIDDEN` |
| 11 | `LEVELCONFLAG_SINGLEINSTANCE` | *(commented out in types3ds.s)* | 4 | `SHOW_AS_HIDDEN` |
| 12 | `LEVELCONFLAG_TEMPLATE` | *(commented out in types3ds.s)* | 4 | `SHOW_AS_HIDDEN` |
| 13 | `LEVELCONFLAG_EXTRACTCAMERA` | *(commented out in types3ds.s)* | 4 | `SHOW_AS_HIDDEN` |
| 14 | `BUTTON_CAMERA_REFERENCE` | `TYPEENTRYCAMERAREFERENCE` | 4 (LE index) | `SHOW_AS_N_A` |
| 15 | `BUTTON_LIGHT_REFERENCE` | `TYPEENTRYLIGHTREFERENCE` | 4 (LE index) | `SHOW_AS_N_A` |
| 16 | `LEVELCONFLAG_ROOM` | `LEVELCONFLAGROOM` | 4 | `SHOW_AS_HIDDEN` |
| 17 | `LEVELCONFLAG_COMMONBLOCK` | `LEVELCONFLAGCOMMONBLOCK(name)` | 0 (structural) | `SHOW_AS_HIDDEN` |
| 18 | `LEVELCONFLAG_ENDCOMMON` | `LEVELCONFLAGENDCOMMON` | 0 (structural) | `SHOW_AS_HIDDEN` |
| 19 | `BUTTON_MESHNAME` | *(defined but unused — `BUTTON_FILENAME` used instead)* | 0 | `SHOW_AS_N_A` |
| 20 | `BUTTON_XDATA` | `TYPEENTRYXDATA`, `TYPEENTRYXDATA_CONVERT`, `TYPEENTRYSTRING_IGNORE` | variable; `chunkName` in `string` field | `SHOW_AS_N_A`, `SHOW_AS_TEXTEDITOR` |
| 21 | `BUTTON_EXTRACT_CAMERA` | `TYPEENTRYCAMERA` | 4 | `SHOW_AS_N_A` |
| 22 | `LEVELCONFLAG_EXTRACTCAMERANEW` | `LEVELCONFLAGEXTRACTCAMERANEW` | 4 | `SHOW_AS_HIDDEN` |
| 23 | `BUTTON_WAVEFORM` | `TYPEENTRYWAVEFORM` | 4 | `SHOW_AS_N_A` |
| 24 | `BUTTON_CLASS_REFERENCE` | `TYPEENTRYCLASSREFERENCE` | 4 (LE index) | `SHOW_AS_N_A` |
| 25 | `BUTTON_GROUP_START` | `GROUP_START` | 0 (structural) | `SHOW_AS_N_A` |
| 26 | `BUTTON_GROUP_STOP` | `GROUP_STOP` | 0 (structural) | `SHOW_AS_N_A` |
| 27 | `LEVELCONFLAG_EXTRACTLIGHT` | `LEVELCONFLAGEXTRACTLIGHT` | 4 | `SHOW_AS_HIDDEN` |
| 28 | `LEVELCONFLAG_SHORTCUT` | `LEVELCONFLAGSHORTCUT` | 4 | `SHOW_AS_HIDDEN` |

---

## showAs (`visualRepresentation`) table

`visualRepresentation` is stored as a single byte.  The constant names are `#define`s
in `oad.h`.

| # | Symbol | Applicable button types | Meaning |
|---|--------|------------------------|---------|
| 0 | `SHOW_AS_N_A` | all | No specific widget; use type default |
| 1 | `SHOW_AS_NUMBER` | `INT*`, `FIXED*` | Plain number input box |
| 2 | `SHOW_AS_SLIDER` | `INT*`, `FIXED*` | Numeric slider |
| 3 | `SHOW_AS_TOGGLE` | `INT*` | Two-state toggle |
| 4 | `SHOW_AS_DROPMENU` | `INT*` (with `string` items) | Drop-down menu |
| 5 | `SHOW_AS_RADIOBUTTONS` | `INT*` (with `string` items) | Radio button group |
| 6 | `SHOW_AS_HIDDEN` | all | Hidden; not shown in attribedit UI |
| 7 | `SHOW_AS_COLOR` | `INT32` | RGB colour picker (0–0xFFFFFF) |
| 8 | `SHOW_AS_CHECKBOX` | `INT32` | Checkbox (`False|True` in `string`) |
| 9 | `SHOW_AS_MAILBOX` | `INT32` | Integer mailbox ID reference |
| 10 | `SHOW_AS_COMBOBOX` | `INT*` | Editable drop-down |
| 11 | `SHOW_AS_TEXTEDITOR` | `BUTTON_STRING`, `BUTTON_XDATA` | Multi-line text editor |
| 12 | `SHOW_AS_FILENAME` | `BUTTON_FILENAME` | File browser / asset picker |
| 0x80 | `SHOW_AS_VECTOR` | `BUTTON_FIXED32` | Bit flag ORed with another value; marks a 3D vector component |

---

## XData conversion actions

`EConversionAction` is stored in `xdata.conversionAction` (one byte inside the union).

| # | Symbol | Meaning |
|---|--------|---------|
| 0 | `XDATA_IGNORE` | Do not process; field is metadata only |
| 1 | `XDATA_COPY` | Copy raw chunk data into level output |
| 2 | `XDATA_OBJECTLIST` | Object list chunk; levelcon parses linked-object names |
| 3 | `XDATA_CONTEXTUALANIMATIONLIST` | Contextual animation list chunk |
| 4 | `XDATA_SCRIPT` | AI script chunk |

---

## `oadFlags` enum (runtime, not stored in OAD)

Defined in `oad.h`; used by the level converter (`oad.cc`) to query type-level
properties.  Not the same as the `LEVELCONFLAG_*` button types.

| Value | Symbol | Meaning |
|-------|--------|---------|
| 0 | `OADFLAG_TEMPLATE_OBJECT` | Object is a template (not placed in level directly) |
| 1 | `OADFLAG_PERMANENT_TEXTURE` | Texture moves between rooms |
| 2 | `OADFLAG_NOMESH_OBJECT` | Object has no mesh reference |
