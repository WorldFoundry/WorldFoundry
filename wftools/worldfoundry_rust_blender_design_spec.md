# World Foundry Modernization Design Note
## Rust Core + Blender Plugin Bridge

### Status
Proposed implementation direction.

### Decision
Use:

- **Rust** for shared core libraries and tool ports
- **Python** only for Blender integration

Do not use Racket, Go, Common Lisp, Scheme, or Node.js in the implementation architecture.

---

## 1. Goals

The modernization effort should produce:

1. A **shared Rust core** for World Foundry data formats and attribute semantics
2. A **Blender integration layer** that hosts the old `wfmaxplugins/attrib` concepts inside Blender
3. A path to port `wftools` incrementally without duplicating binary-format logic
4. A system that preserves the important behavior of the old tools/plugins:
   - schema-driven editing
   - typed fields
   - validation
   - serialization
   - object-attached game data

---

## 2. Non-goals

This phase does **not** aim to:

- transliterate the old C++ or Max SDK code line by line
- preserve old Win32/Max UI implementation details
- build a general-purpose DCC abstraction layer
- support multiple implementation languages in the core
- redesign the World Foundry data model unless required by Blender integration

---

## 3. Architectural summary

### Core principle

**Rust owns data meaning. Blender owns presentation.**

That means:

- Rust defines what attributes are, how they are validated, and how they are serialized
- Blender/Python renders and edits those attributes in the host UI
- Python does not become a second implementation of OAD/IFF logic

### High-level shape

```text
Legacy sources
  ├── wftools/*
  └── wfmaxplugins/attrib/*
          ↓
Shared Rust libraries
  ├── wf_iff
  ├── wf_oad
  ├── wf_attr_schema
  ├── wf_attr_validate
  └── wf_attr_serialize
          ↓
Bindings / bridge
  └── Python extension module for Blender
          ↓
Blender add-on
  ├── panels
  ├── operators
  ├── object integration
  └── dynamic UI from schema
```

---

## 4. Source-of-truth model

There are three distinct kinds of truth in the system.

### 4.1 Schema truth
**OAD** is the authoritative schema source.

It defines things like:

- field names
- types
- defaults
- ranges
- enum choices
- grouping
- editor hints
- serialization rules

### 4.2 Asset/export truth
The authoritative exported asset format may be:

- `.iff`
- `.iff.txt`
- or both, depending on workflow stage

Recommended approach:

- use **`.iff.txt`** as the first debug-friendly export target
- add binary **`.iff`** once the model is stable

### 4.3 Live editing truth inside Blender
Blender should store the current editable values in a practical object-attached representation.

That representation should be:

- structured
- field-addressable
- easy for panels/widgets to manipulate
- validated by Rust

It should **not** be raw `.iff.txt` text and should **not** be a reimplementation of OAD in Python.

---

## 5. Rust crate layout

Start with these crates.

### `wf_iff`
Responsibilities:

- IFF parsing
- IFF writing
- chunk walking
- endian-aware integer IO
- binary helpers
- backpatch utilities where needed

Used by:

- tool ports
- serializers
- exporters

### `wf_oad`
Responsibilities:

- OAD parsing/loading
- schema object model
- field metadata
- grouping metadata
- default values
- editor hints

Used by:

- Blender integration
- validators
- serializers
- future tool ports

### `wf_attr_schema`
Responsibilities:

- normalized attribute schema model independent of old UI framework details
- field taxonomy
- storage kinds
- editor kinds
- object/reference semantics

This is the bridge between OAD semantics and host UI rendering.

### `wf_attr_validate`
Responsibilities:

- type checks
- range checks
- enum membership checks
- required/reference checks
- cross-field validation if needed

### `wf_attr_serialize`
Responsibilities:

- convert instance values into exportable forms
- emit `.iff`
- emit `.iff.txt`
- import existing data into normalized instance form

---

## 6. Blender integration boundary

### 6.1 Language boundary
Use **Python** in Blender, with Rust exposed through Python bindings.

Preferred approach:

- Rust compiled as a Python extension module
- Python calls Rust directly for:
  - schema loading
  - validation
  - serialization
  - import/export

Do not begin with a CLI subprocess bridge unless forced to.

### 6.2 Ownership boundary
Rust owns:

- schemas
- validation logic
- serialization logic
- binary format handling

Python owns:

- Blender registration
- UI drawing
- operator wiring
- scene/object integration
- user interaction

### 6.3 Anti-goal
Do not mirror the Rust logic in Python.

---

## 7. Blender data model

### 7.1 Object attachment model
Each relevant Blender object should have:

- a schema identity
- a set of current attribute values
- optional import/export metadata

Suggested conceptual model:

```text
Blender Object
  ├── wf_schema_id
  ├── wf_attr_values
  └── wf_attr_meta
```

Where:

- `wf_schema_id` identifies which OAD/schema applies
- `wf_attr_values` stores live current values
- `wf_attr_meta` can hold UI/cache/version data if needed

### 7.2 Storage approach
Recommended starting approach:

- store values in Blender custom properties / ID properties
- render UI dynamically from Rust-provided schema

Do **not** start with:

- raw text blobs as the main live editing state
- giant static generated `PropertyGroup` classes for every possible schema

### 7.3 Why this approach
It gives:

- dynamic behavior
- incremental UI development
- simpler schema-driven rendering
- easier debugging during early milestones

---

## 8. UI model in Blender

### 8.1 Key principle
**Do not hardcode the old UI.**  
Instead, preserve the old **field semantics** and map them into Blender’s host UI model.

### 8.2 Derived UI
Blender panels should be generated from normalized field descriptors.

A field descriptor should contain at least:

- field key
- label
- field kind
- storage type
- default
- constraints
- enum items
- grouping info
- help text
- editor hint

### 8.3 Example normalized field model

```text
FieldDescriptor
  key: "mass"
  label: "Mass"
  storage_type: fixed32
  editor_kind: numeric
  min: 0
  max: 100
  default: 1
  group: "Physics"
```

### 8.4 Blender rendering model
A panel renderer walks field descriptors and chooses appropriate UI widgets.

Examples:

- numeric → `layout.prop(...)`
- enum → dropdown
- bool → checkbox
- color → color picker
- object reference → custom object picker/search widget
- grouped fields → panel boxes/sections

---

## 9. Mapping `wfmaxplugins/attrib/buttons/*` into Blender

The old button taxonomy should be preserved conceptually, then normalized.

### First-pass mapping

| Legacy control | Normalized meaning | Blender representation |
|---|---|---|
| `int` | integer field | integer property/input |
| `fixed` | fixed-point or float-like numeric | float property/input |
| `checkbox` | boolean field | checkbox |
| `dropmenu` | enum | dropdown |
| `combobox` | enum/string hybrid | dropdown or searchable enum |
| `cycle` | cycling enum | enum/dropdown |
| `string` | string field | text input |
| `filename` | file path | file picker/text field |
| `colorpck` | color field | color picker |
| `group` | grouped fields | box/panel section |
| `propshet` | property sheet grouping | subpanel / tab-style grouping |
| `objref` | object reference | Blender object selector |
| `meshname` | mesh/data reference | filtered object/data selector |
| `classref` | typed class reference | typed selector / enum |
| `xdata` | arbitrary extra data | custom handler, likely later |
| `mailbox` | domain-specific reference/message target | custom widget later |

### Recommendation
Do not port each legacy control class literally.

Instead:

1. normalize each old control into a field/editor spec
2. implement Blender rendering for the spec
3. add custom widgets only when needed

---

## 10. Tool-port relationship

The Blender work and tool ports should share Rust foundations.

### Rust-first tools
These should rely directly on the shared crates:

- `iffdump`
- `oaddump`
- `iffdb`
- `lvldump`
- `iff2lvl`
- `chargrab`
- `textile`

### Implication
The Blender plugin should not invent its own parallel parser or serializer.

---

## 11. Import/export strategy

### Formats are first-class and interchangeable
`.iff.txt` and binary `.iff` are equivalent representations that round-trip losslessly.
`iffcomp` compiles `.iff.txt` → `.iff`; `iffdump -f-` decompiles `.iff` → `.iff.txt`.
Blender import accepts both; export targets `.iff.txt` directly and binary `.iff` via `iffcomp` or native Rust serialization.

### Phase 1
- export: `.iff.txt` (done)
- import: `.iff.txt` and `.iff` (reads values back into Blender object properties)

### Phase 2
- native binary `.iff` write path in Rust (requires `wf_iff` writer crate)
- eliminates the `iffcomp` subprocess step for binary output

### Principle
Use Rust for all serialization. Python does not implement format logic.

---

## 12. Vertical slice for milestone 1

Do not start with the full plugin.  
Build one narrow, representative end-to-end slice.

### Milestone 1 should prove:
Inside Blender, a user can:

1. attach one known schema to an object
2. see generated UI for a small field set
3. edit values
4. receive validation feedback
5. save/reload values
6. export a valid artifact

### Initial field types
Start with:

- int
- fixed/float
- checkbox
- enum/dropmenu
- string

Optional harder case:

- one object reference field

### Avoid in milestone 1
Do not start with:

- all control types
- tabs/property-sheet recreation
- complicated custom reference widgets
- obscure legacy field kinds
- full import compatibility across every historical file

---

## 13. Recommended work sequence

### Step 1 — Write the Rust schema model
Define normalized schema types and field descriptors.

### Step 2 — Implement OAD loading in Rust
Load at least one real schema from existing data.

### Step 3 — Expose Rust to Python
Provide a thin Python extension API for:

- load schema
- enumerate fields
- validate values
- export values

### Step 4 — Build Blender prototype panel
Dynamic panel rendering for one object and one schema.

### Step 5 — Add persistence
Store and restore object-attached attribute values.

### Step 6 — Add `.iff.txt` export
Use Rust serialization.

### Step 7 — Expand control coverage
Add more field/editor mappings from the old plugin.

---

## 14. Proposed Python API shape

The Blender side should consume a boring, stable API.

### Example conceptual API

```python
schema = wf.load_schema("enemy.oad")
fields = schema.fields()
result = wf.validate(schema, values)
text = wf.export_iff_txt(schema, values)
binary = wf.export_iff(schema, values)
```

### Important constraint
Python should receive plain, practical data:

- field lists
- dictionaries
- validation results
- strings/bytes

Do not expose Rust-internal complexity to Blender.

---

## 15. Proposed Rust API shape

Rust should expose a clean library first, then Python bindings around it.

### Conceptual Rust library API

```rust
let schema = wf_oad::load_schema(path)?;
let fields = schema.fields();
let validation = wf_attr_validate::validate(&schema, &values);
let text = wf_attr_serialize::to_iff_txt(&schema, &values)?;
let bytes = wf_attr_serialize::to_iff(&schema, &values)?;
```

### Design principle
The Rust API should be useful even without Blender.

That keeps it reusable for tools and tests.

---

## 16. Testing strategy

### Core Rust tests
- parser tests
- schema-loading tests
- validation tests
- serialization tests
- golden-file tests for `.iff` / `.iff.txt`

### Blender integration tests
Initially lighter-weight:

- schema loads correctly
- field list appears correctly
- edited values round-trip
- export operator produces expected artifact

### Regression emphasis
Use real World Foundry schemas and samples early.  
Do not rely only on synthetic toy examples.

---

## 17. Risks

### Risk: Python reimplements logic
Mitigation:
- keep validation and serialization in Rust

### Risk: Blender property model fights schema flexibility
Mitigation:
- begin with dynamic UI over custom properties

### Risk: legacy controls map poorly to Blender
Mitigation:
- normalize first, customize later

### Risk: schema semantics are muddier than expected
Mitigation:
- start with one real schema and one narrow vertical slice

### Risk: porting becomes GUI-led instead of data-led
Mitigation:
- finish Rust schema/value/export model before broad UI expansion

---

## 18. Open questions

These should be resolved during milestone 1:

1. How should schema identity be attached to Blender objects?
2. What is the exact Blender-side storage format for live values?
3. Which real schema is best for the first vertical slice?
4. How much of the old plugin’s grouping model maps naturally to Blender panels/subpanels?
5. Which legacy field/reference controls require custom Blender widgets earliest?
6. ~~Should `.iff.txt` be considered the first-class editable export artifact long-term, or only a debug stage?~~
   **Resolved:** `.iff.txt` is a first-class storage format, fully interchangeable with binary `.iff`.
   The two formats round-trip losslessly via `iffcomp` (`.iff.txt` → `.iff`) and `iffdump -f-` (`.iff` → `.iff.txt`).
   Blender import must accept both. `wf_attr_serialize` must treat them symmetrically.

---

## 19. Final recommendation

Proceed with:

- **Rust** as the only core implementation language
- **Python** as the Blender host layer
- **OAD** as schema truth
- **dynamic Blender UI generated from normalized field descriptors**
- **shared Rust libraries reused across both tools and Blender integration**

Do not port `wfmaxplugins/attrib` literally.

Port its **conceptual architecture**:

- schema-driven attributes
- typed field controls
- validation
- object binding
- serialization

That is the durable path.

---

## 20. Immediate next action

Create a milestone-1 task for:

> Load one real OAD schema in Rust, expose it to Blender, render five field types on one object, validate edits, and export `.iff.txt`.

That is the first real proof that the plan works.
