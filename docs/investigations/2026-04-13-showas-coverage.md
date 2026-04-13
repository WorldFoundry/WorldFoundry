# Plan: ButtonType × showAs coverage audit and gap fixes

## Context

The Blender plugin renders OAD fields based on `FieldKind` (derived from `ButtonType` in
`classify()`) and `show_as` (the OAD `visualRepresentation` byte).  All OAS schema files
have been audited against the current plugin to find combinations that exist in real OAD
files but are not yet correctly handled.

---

## Coverage table

| ButtonType (value) | showAs (value) | FieldKind | Blender rendering | Status |
|---|---|---|---|---|
| Int8/16/32 — no pipe items (4) | 0 N/A | Int | `layout.prop()` | ✅ |
| Int8/16/32 — no pipe items (4) | 1 number | Int | `layout.prop()` | ✅ |
| Int8/16/32 — no pipe items (4) | 2 slider | Int | `layout.prop()` + min/max → slider | ✅ |
| Int8/16/32 — no pipe items (4) | **8 checkbox** | Int | `layout.prop()` as plain integer | ❌ `TYPEENTRYBOOLEAN` — should render as checkbox |
| Int8/16/32 — no pipe items (4) | 7 color | Int | hex swatch + `wf.pick_color` dialog | ✅ |
| Int8/16/32 + pipe items (4) | 4 dropmenu | Enum | horizontal button row | ⚠️ functional but cramped for 5+ items |
| Int8/16/32 + pipe items (4) | 5 radiobuttons | Enum | horizontal button row | ✅ correct |
| Int8/16/32 + pipe items (4) | 8 checkbox | Enum | checkbox toggle (2-item) | ✅ |
| Fixed16/Fixed32 (1/2) | 0 N/A | Float | `layout.prop()` | ✅ |
| Fixed16/Fixed32 (1/2) | 6 hidden | Float | hidden by `is_visible()` | ✅ |
| PropertySheet (8) | 0 N/A | Section | collapsible box | ✅ |
| GroupStart (25) | 0 N/A | Group | non-collapsible box | ✅ |
| GroupStop (26) | 0 N/A | GroupEnd | closes box | ✅ |
| Filename / MeshName (7/19) | 0 N/A | FileRef | text + file-browser button | ✅ |
| ObjectRef / ClassRef / CamRef / LightRef (6/24/14/15) | 0 N/A | ObjRef | `prop_search()` | ✅ |
| ObjectRef (6) | 0x80 vector | ObjRef | `prop_search()` (flag ignored) | ✅ (flag is decorative) |
| LEVELCONFLAG_* (9,10,11,16,17,22,27,28) | 6 hidden | Bool | checkbox (exempted from hidden) | ✅ |
| BUTTON_XDATA (20) | 0 N/A | Skip | hidden | ✅ (raw XData, no UI) |
| BUTTON_XDATA (20) | **11 texteditor** | Skip | hidden | ⚠️ Notes/Comments field — could surface as Str |
| BUTTON_WAVEFORM (23) | 0 N/A | Skip | hidden | ✅ (game engine only) |
| BUTTON_EXTRACT_CAMERA (21/22) | 0 N/A | Skip | hidden | ✅ (game engine only) |
| BUTTON_STRING (5) | 0 N/A | Str | `layout.prop()` | ✅ |

---

## Gap analysis

### Gap 1 — `TYPEENTRYBOOLEAN`: Int show_as=8 → should be checkbox  ❌ HIGH

`TYPEENTRYBOOLEAN` uses `BUTTON_INT32` (no pipe items) + `show_as=8`.  `classify()` maps
this to `FieldKind::Int` (correct type), but the Blender panel renders it as a plain
integer spinner.  It should render as a checkbox — the same as `FieldKind::Bool`.

Affected fields: any `TYPEENTRYBOOLEAN` field in any OAS (e.g. player.oad "No Roll").

**Fix (2 files):**

`wftools/wf_attr_schema/src/lib.rs` — pass `show_as` into `classify()`:
```rust
fn classify(bt: ButtonType, string_field: &str, lpstr_filter: &[u8], show_as: u8) -> FieldKind {
    match bt {
        ButtonType::Int8 | ButtonType::Int16 | ButtonType::Int32 => {
            let items = parse_pipe_items(string_field);
            if items.is_empty() {
                if show_as == 8 { FieldKind::Bool } else { FieldKind::Int }
            } else {
                FieldKind::Enum { items }
            }
        }
        // …rest unchanged…
    }
}
```
Update the single call site in `from_oad()`:
```rust
let kind = classify(entry.button_type, entry.string_str(), entry.lpstr_filter_bytes(), entry.show_as);
```
No Python or serialization changes needed — `Bool` already serializes as 4-byte LE int.

### Gap 2 — Enum show_as=4 (dropmenu): cramped button row  ⚠️ MEDIUM

~185 Enum fields have `show_as=4`.  Currently all Enum fields (except the 2-item
show_as=8 checkbox) render as a horizontal button row.  For fields with 5+ items (e.g.
Mobility: 5 choices, At End Of Path: 5 choices) the row is very wide.

**Fix (`wftools/wf_blender/panels.py` only):**

In `_draw_field()`, split the else branch on `show_as`:
```python
elif kind == "Enum":
    items   = field.enum_items()
    current = obj.get(prop_key, "")

    if show_as == 8 and len(items) == 2:
        # … existing checkbox logic unchanged …

    elif show_as in (4, 5) and len(items) > 4:
        # Dropmenu / radiobuttons with many choices → stacked column
        col = layout.column(align=True)
        col.label(text=field.label + ":")
        for item in items:
            op = col.operator("wf.set_enum", text=item, depress=(item == current))
            op.field_key  = field.key
            op.item_label = item

    else:
        # ≤4 items or show_as not dropmenu: existing horizontal button row
        row = layout.row(align=True)
        row.label(text=field.label + ":")
        sub = row.row(align=True)
        for item in items:
            op = sub.operator("wf.set_enum", text=item, depress=(item == current))
            op.field_key  = field.key
            op.item_label = item
```

### Gap 3 — BUTTON_XDATA show_as=11 (Notes): currently hidden  ⚠️ LOW

`TYPEENTRYXDATA_NOTES` (defined in `xdata.inc`) is a free-text artist annotation field
present in almost every schema via `common.inc`.  Currently mapped to `Skip` and hidden.
Since `conversionAction=XDATA_IGNORE`, it carries no game data — it's pure metadata and
safe to surface as a `Str` text box.

**Fix (`wftools/wf_attr_schema/src/lib.rs`):**

Pass `show_as` into `classify()` (already done in Gap 1), then add an arm:
```rust
ButtonType::XData => {
    if show_as == 11 { FieldKind::Str } else { FieldKind::Skip }
}
```
`FieldKind::Str` already serializes as zero-width (no bytes in the IFF payload),
is seeded as `""` in `_seed_defaults()`, and renders as `layout.prop()` in panels.py.
No further changes needed.

### Gap 4 — Int show_as=7 (color): 3 fields  ✅ DONE

`FoggingColor`, `Background Color` (camera.oad / mesh.inc) are 24-bit packed RGB integers
(`0x00RRGGBB`, range 0..0xFFFFFF).

Storage stays as a plain integer ID property.  The panel renders a `#RRGGBB` hex label
and a `wf.pick_color` button.  Clicking opens an `invoke_props_dialog` with a native
Blender `FloatVectorProperty(subtype='COLOR')` picker; on confirm the RGB floats are
packed back to an int and written to the property.

---

## Critical files

| File | Change |
|------|--------|
| `wftools/wf_attr_schema/src/lib.rs` | Add `show_as: u8` param to `classify()`; map Int+show_as=8→Bool; map XData+show_as=11→Str |
| `wftools/wf_blender/panels.py` | Enum dropmenu: 2-col grid for 5+ items; Int show_as=7: hex swatch + pick_color button |
| `wftools/wf_blender/operators.py` | Add `WF_OT_pick_color`: invoke_props_dialog with FloatVectorProperty(subtype='COLOR') |

No changes needed to `wf_py`, `wf_attr_serialize`, or `wf_attr_validate`.

---

## Verification

```bash
# 1. Rust unit tests (gap 1: Bool from TYPEENTRYBOOLEAN; gap 3: Str from XData notes)
cargo test -p wf_attr_schema

# 2. Rebuild Python extension
cd wftools/wf_py && maturin develop

# 3. Python smoke test — confirm "No Roll" is Bool, "Notes" is Str
python3 -c "
import wf_core
s = wf_core.load_schema('wftools/wf_oad/tests/fixtures/player.oad')
for f in s.fields():
    if f.kind in ('Bool',):
        print(f.kind, f.show_as, f.key)
"

# 4. Blender: attach player.oad → verify 'No Roll' shows checkbox, not integer
#    attach an OAD with a dropmenu (actbox.oad) → verify 'Mobility' stacks vertically
```
