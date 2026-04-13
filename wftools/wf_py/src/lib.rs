//! PyO3 Python extension module exposing World Foundry OAD schema to Python.
//!
//! Compiled as `wf_core.so` (abi3, Python ≥ 3.10).
//!
//! # Python API
//!
//! ```python
//! import wf_core
//!
//! schema = wf_core.load_schema("/path/to/room.oad")
//! print(schema.name)              # "Room"
//! for field in schema.fields():
//!     print(field.key, field.kind, field.label)
//! ```

use pyo3::prelude::*;
use std::io::Cursor;
use wf_attr_schema::{FieldKind, Schema, FieldDescriptor};
use wf_oad::OadFile;

// ── PyField ───────────────────────────────────────────────────────────────────

/// Python-visible descriptor for one OAD field.
///
/// For `Float` fields, `default_display`, `min_display`, `max_display` are
/// the human-readable float values (raw / fp_scale).  Store and edit these
/// display values in Blender; the exporter converts them back to raw.
///
/// For all other field kinds, `default_display` equals `default_raw` cast to float.
#[pyclass(name = "Field")]
#[derive(Clone)]
struct PyField {
    #[pyo3(get)]
    key: String,
    #[pyo3(get)]
    label: String,
    /// `"Int"` | `"Float"` | `"Enum"` | `"Group"` | `"Str"` | `"Skip"`
    #[pyo3(get)]
    kind: String,
    #[pyo3(get)]
    help: String,
    #[pyo3(get)]
    group: String,
    #[pyo3(get)]
    min_raw: i32,
    #[pyo3(get)]
    max_raw: i32,
    #[pyo3(get)]
    default_raw: i32,
    /// Human-readable display value for the default.
    /// Float fields: default_raw / fp_scale.  Others: default_raw as f64.
    #[pyo3(get)]
    default_display: f64,
    #[pyo3(get)]
    min_display: f64,
    #[pyo3(get)]
    max_display: f64,
    /// Fixed-point divisor (e.g. 65536 for Fixed32).  0.0 for non-Float.
    #[pyo3(get)]
    fp_scale: f64,
    /// Serialized byte width (1, 2, or 4).  0 for Group/Skip/variable.
    #[pyo3(get)]
    byte_width: u8,
    /// Raw `visualRepresentation` from the OAD.
    /// 0=plain, 4/5=dropmenu, 6=hidden, 8=checkbox.
    #[pyo3(get)]
    show_as: u8,
    // Enum item labels (non-empty only for Enum fields).
    enum_items: Vec<String>,
}

#[pymethods]
impl PyField {
    /// For Enum fields, returns the list of choice labels.
    /// Returns an empty list for non-Enum fields.
    fn enum_items(&self) -> Vec<String> {
        self.enum_items.clone()
    }

    fn __repr__(&self) -> String {
        format!("Field(key={:?}, kind={:?})", self.key, self.kind)
    }
}

fn field_from_desc(desc: &FieldDescriptor) -> PyField {
    let (kind_tag, enum_items) = match &desc.kind {
        FieldKind::Enum { items } => ("Enum".to_owned(), items.clone()),
        other => (other.tag().to_owned(), Vec::new()),
    };
    let scale = if desc.fp_scale > 0.0 { desc.fp_scale } else { 1.0 };
    let (default_display, min_display, max_display) = if desc.fp_scale > 0.0 {
        (
            desc.default_raw as f64 / scale,
            desc.min_raw     as f64 / scale,
            desc.max_raw     as f64 / scale,
        )
    } else {
        (desc.default_raw as f64, desc.min_raw as f64, desc.max_raw as f64)
    };
    PyField {
        key:             desc.key.clone(),
        label:           desc.label.clone(),
        kind:            kind_tag,
        help:            desc.help.clone(),
        group:           desc.group.clone(),
        min_raw:         desc.min_raw,
        max_raw:         desc.max_raw,
        default_raw:     desc.default_raw,
        default_display,
        min_display,
        max_display,
        fp_scale:        desc.fp_scale,
        byte_width:      desc.byte_width,
        show_as:         desc.show_as,
        enum_items,
    }
}

// ── PySchema ──────────────────────────────────────────────────────────────────

/// Python-visible schema loaded from an OAD file.
#[pyclass(name = "Schema")]
struct PySchema {
    inner: Schema,
}

#[pymethods]
impl PySchema {
    /// The display name of the schema (e.g. "Room").
    #[getter]
    fn name(&self) -> &str {
        &self.inner.name
    }

    /// All fields, including Group headers and Skip entries.
    fn fields(&self) -> Vec<PyField> {
        self.inner.fields.iter().map(field_from_desc).collect()
    }

    /// Only the fields that should appear in the editor UI
    /// (excludes Group and Skip).
    fn visible_fields(&self) -> Vec<PyField> {
        self.inner.visible_fields().map(field_from_desc).collect()
    }

    fn __repr__(&self) -> String {
        format!(
            "Schema(name={:?}, fields={})",
            self.inner.name,
            self.inner.fields.len()
        )
    }
}

// ── validate ──────────────────────────────────────────────────────────────────

/// A single validation error or warning.
#[pyclass(name = "ValidationIssue")]
#[derive(Clone)]
struct PyValidationIssue {
    #[pyo3(get)]
    key: String,
    #[pyo3(get)]
    message: String,
    #[pyo3(get)]
    is_error: bool,
}

#[pymethods]
impl PyValidationIssue {
    fn __repr__(&self) -> String {
        let level = if self.is_error { "ERROR" } else { "WARN" };
        format!("{level}: {}: {}", self.key, self.message)
    }
}

/// Validate `values` against `schema`.
///
/// `values` is a Python dict mapping field keys to display values:
/// - `Int` fields: integer or float (truncated)
/// - `Float` fields: display float (e.g. 50.0 for Mass, not the raw 3276800)
/// - `Enum` fields: string item label (e.g. `"Physics"`) or int index
/// - `Str` fields: string
///
/// Returns a list of [`ValidationIssue`] objects (empty = all good).
#[pyfunction]
fn validate(
    schema: &PySchema,
    values: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<Vec<PyValidationIssue>> {
    let mut issues = Vec::new();

    for field in schema.inner.visible_fields() {
        let raw_val = values.get_item(field.key.as_str())?;
        let Some(raw_val) = raw_val else { continue };

        let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };

        match &field.kind {
            wf_attr_schema::FieldKind::Section
            | wf_attr_schema::FieldKind::Group
            | wf_attr_schema::FieldKind::GroupEnd => continue,
            wf_attr_schema::FieldKind::Int => {
                let v: i64 = raw_val.extract().unwrap_or(0);
                if v < field.min_raw as i64 || v > field.max_raw as i64 {
                    issues.push(PyValidationIssue {
                        key:      field.key.clone(),
                        message:  format!(
                            "value {} out of range [{}, {}]",
                            v, field.min_raw, field.max_raw
                        ),
                        is_error: true,
                    });
                }
            }
            wf_attr_schema::FieldKind::Float => {
                // values dict stores display float; convert back to raw for range check
                let display: f64 = raw_val.extract().unwrap_or(0.0);
                let raw = (display * scale).round() as i64;
                if raw < field.min_raw as i64 || raw > field.max_raw as i64 {
                    issues.push(PyValidationIssue {
                        key:     field.key.clone(),
                        message: format!(
                            "{:.4} out of range [{:.4}, {:.4}]",
                            display,
                            field.min_raw as f64 / scale,
                            field.max_raw as f64 / scale,
                        ),
                        is_error: true,
                    });
                }
            }
            wf_attr_schema::FieldKind::Enum { items } => {
                // Accept either a string label or an int index
                let idx: i64 = if let Ok(s) = raw_val.extract::<String>() {
                    items.iter().position(|i| i == &s).map(|p| p as i64).unwrap_or(-1)
                } else {
                    raw_val.extract().unwrap_or(-1)
                };
                if idx < 0 || idx as usize >= items.len() {
                    issues.push(PyValidationIssue {
                        key:      field.key.clone(),
                        message:  format!(
                            "invalid enum value {:?} — valid: {:?}",
                            raw_val.str().map(|s| s.to_string()).unwrap_or_default(),
                            items,
                        ),
                        is_error: true,
                    });
                }
            }
            _ => {}
        }
    }

    Ok(issues)
}

// ── export_iff_txt ────────────────────────────────────────────────────────────

/// Export `values` as an iffcomp-format `.iff.txt` artifact.
///
/// `values` is a Python dict mapping field keys to display values
/// (same contract as [`validate`]).  Missing fields use the schema default.
///
/// The output is valid iffcomp source: each field's value is serialized as
/// little-endian bytes on a `$XX $XX ...` line followed by a `// key` comment.
///
/// Returns the text as a Python string.
#[pyfunction]
fn export_iff_txt(
    schema: &PySchema,
    values: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<String> {
    // Build the FOURCC from the first 1–4 chars of the schema name, space-padded.
    let name_bytes = schema.inner.name.as_bytes();
    let mut fourcc = [b' '; 4];
    for (i, &b) in name_bytes.iter().take(4).enumerate() {
        fourcc[i] = b.to_ascii_uppercase();
    }
    let fourcc_str: String = fourcc.iter().map(|&b| b as char).collect();

    // Collect bytes for all visible fields.
    let mut payload: Vec<u8> = Vec::new();
    // Lines of iffcomp source for the payload.
    let mut lines: Vec<String> = Vec::new();

    for field in schema.inner.visible_fields() {
        let raw_val = values.get_item(field.key.as_str())?;
        let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };

        match &field.kind {
            wf_attr_schema::FieldKind::Section
            | wf_attr_schema::FieldKind::Group
            | wf_attr_schema::FieldKind::GroupEnd => continue,
            wf_attr_schema::FieldKind::Int | wf_attr_schema::FieldKind::Enum { .. } => {
                let raw: i32 = if let Some(v) = &raw_val {
                    // Enum: accept string label or int
                    if let wf_attr_schema::FieldKind::Enum { items } = &field.kind {
                        if let Ok(s) = v.extract::<String>() {
                            items.iter().position(|i| i == &s)
                                .map(|p| p as i32)
                                .unwrap_or(field.default_raw)
                        } else {
                            v.extract::<i32>().unwrap_or(field.default_raw)
                        }
                    } else {
                        v.extract::<i32>().unwrap_or(field.default_raw)
                    }
                } else {
                    field.default_raw
                };
                let bytes = raw.to_le_bytes();
                let width = field.byte_width.max(4) as usize;
                let b = &bytes[..width];
                payload.extend_from_slice(b);
                lines.push(format!(
                    "\t{}\t// {}",
                    b.iter().map(|x| format!("${:02X}", x)).collect::<Vec<_>>().join(" "),
                    field.key,
                ));
            }
            wf_attr_schema::FieldKind::Float => {
                let display: f64 = raw_val.as_ref()
                    .and_then(|v| v.extract::<f64>().ok())
                    .unwrap_or(field.default_raw as f64 / scale);
                let raw = (display * scale).round() as i32;
                let bytes = raw.to_le_bytes();
                let width = field.byte_width.max(4) as usize;
                let b = &bytes[..width];
                payload.extend_from_slice(b);
                lines.push(format!(
                    "\t{}\t// {} ({:.6})",
                    b.iter().map(|x| format!("${:02X}", x)).collect::<Vec<_>>().join(" "),
                    field.key,
                    display,
                ));
            }
            wf_attr_schema::FieldKind::Str => {
                // String: write raw bytes as-is; no fixed width in M1 export.
                let s: String = raw_val.as_ref()
                    .and_then(|v| v.extract::<String>().ok())
                    .unwrap_or_default();
                let b = s.as_bytes();
                payload.extend_from_slice(b);
                if !b.is_empty() {
                    lines.push(format!(
                        "\t{}\t// {}",
                        b.iter().map(|x| format!("${:02X}", x)).collect::<Vec<_>>().join(" "),
                        field.key,
                    ));
                } else {
                    lines.push(format!("\t// {} (empty string)", field.key));
                }
            }
            _ => {}
        }
    }

    let size = payload.len();
    let mut out = String::new();
    out.push_str("//=============================================================================\n");
    out.push_str(&format!(
        "// {} — exported by wf_core v{}\n",
        schema.inner.name,
        env!("CARGO_PKG_VERSION"),
    ));
    out.push_str("//=============================================================================\n");
    out.push_str(&format!("{{ '{}'\t\t// Size = {}\n", fourcc_str, size));
    for line in &lines {
        out.push_str(line);
        out.push('\n');
    }
    out.push_str("}\n");
    Ok(out)
}

// ── module entry point ────────────────────────────────────────────────────────

/// Load an OAD schema from `path` and return a [`Schema`] object.
#[pyfunction]
fn load_schema(path: &str) -> PyResult<PySchema> {
    let data = std::fs::read(path)
        .map_err(|e| pyo3::exceptions::PyIOError::new_err(format!("{path}: {e}")))?;
    let oad = OadFile::read(&mut Cursor::new(&data))
        .map_err(|e| pyo3::exceptions::PyValueError::new_err(format!("{path}: {e}")))?;
    let schema = wf_attr_schema::from_oad(&oad);
    Ok(PySchema { inner: schema })
}

/// World Foundry core library — OAD schema loading, validation, and export.
#[pymodule]
fn wf_core(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(load_schema, m)?)?;
    m.add_function(wrap_pyfunction!(validate, m)?)?;
    m.add_function(wrap_pyfunction!(export_iff_txt, m)?)?;
    m.add_class::<PySchema>()?;
    m.add_class::<PyField>()?;
    m.add_class::<PyValidationIssue>()?;
    Ok(())
}
