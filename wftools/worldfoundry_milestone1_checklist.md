# World Foundry Modernization — Milestone 1 Checklist

## Objective
Inside Blender:

- attach one schema to one object
- render 5 fields
- edit them
- validate them
- export `.iff.txt`

If this works, the architecture is viable.

---

## 1. Choose one real schema

Pick one actual OAD/schema from existing project data.

Requirements:

- small
- representative
- includes at least:
  - one numeric field
  - one enum
  - one string or boolean

Avoid invented toy schemas.

---

## 2. Implement minimal Rust core API

Target API surface:

```rust
load_schema(path) -> Schema
schema.fields() -> Vec<FieldDescriptor>
validate(schema, values) -> ValidationResult
export_iff_txt(schema, values) -> String
```

### Minimal field coverage
Support only:

- int
- fixed/float
- checkbox/bool
- enum/dropmenu
- string

Optional sixth case:

- one object reference field

---

## 3. Normalize field descriptors

Minimum field descriptor shape:

```text
FieldDescriptor
  key
  label
  kind
  default
  min
  max
  enum_items
  group
```

Do not try to model every historical field variation yet.

---

## 4. Prove Rust ↔ Python ↔ Blender bridge

Build the smallest possible binding that lets Blender do:

```python
schema = wf.load_schema(path)
fields = schema.fields()
```

Goal:
- confirm Rust types can be exposed cleanly
- confirm Blender can consume field descriptors

No extra architecture yet.

---

## 5. Build one Blender panel

Render fields dynamically by kind.

Expected first-pass behavior:

- int → numeric input
- float/fixed → numeric input
- bool → checkbox
- enum → dropdown
- string → text input

Ugly is acceptable.
Hardcoded layout polish is not required.

---

## 6. Store live values on object

Use Blender object custom properties / ID properties first.

Do not begin with:

- raw `.iff.txt` storage as the live editing model
- giant static generated `PropertyGroup` trees
- Python-side schema duplication

---

## 7. Add validation

Add a validation pass from Blender to Rust.

At minimum:

- button-triggered validation is acceptable
- print errors to console if needed
- better: show basic error text in panel

Validation must remain in Rust.

---

## 8. Add `.iff.txt` export

Add one export operator that:

- gathers object values
- calls Rust serialization
- writes `.iff.txt` to disk

Use this as the first end-to-end proof.

Do not start with binary `.iff`.

---

## 9. Evaluate after slice completes

After the first slice works, answer these:

1. Does schema → UI generation feel clean?
2. Is Rust ↔ Python binding tolerable?
3. Does the field model feel right?
4. Are Blender custom properties sufficient for live state?
5. Which old button types are next-most-important?

---

## 10. Explicit “not yet” list

Do not do these in milestone 1:

- all button/control types
- full property-sheet/tab recreation
- polished UX
- all schemas
- full import compatibility
- advanced object-reference widgets
- binary `.iff` export
- packaging/distribution work

---

## Deliverable Definition

Milestone 1 is done when a user can:

1. choose one schema
2. attach it to one Blender object
3. edit five field types
4. validate values
5. export `.iff.txt`

That is the checkpoint.
