//! PyO3 Python extension module exposing World Foundry OAD schema to Python.
//!
//! Compiled as `wf_core.so` (abi3, Python ≥ 3.10).
//!
//! This crate is a thin boundary layer.  All schema logic lives in
//! `wf_attr_schema`, validation in `wf_attr_validate`, and serialization in
//! `wf_attr_serialize`.  This file converts between Python types and the
//! canonical Rust types, then delegates.
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
//!
//! issues = wf_core.validate(schema, {"hp": 5, "Mass": 1.5})
//! text   = wf_core.export_iff_txt(schema, {"hp": 5, "Mass": 1.5})
//! ```

use pyo3::prelude::*;
use std::io::Cursor;
use wf_attr_schema::{FieldDescriptor, FieldKind, FieldValue, Schema, Values};
use wf_oad::OadFile;

// ── PyField ───────────────────────────────────────────────────────────────────

/// Python-visible descriptor for one OAD field.
#[pyclass(name = "Field")]
#[derive(Clone)]
struct PyField {
    #[pyo3(get)]
    key: String,
    #[pyo3(get)]
    label: String,
    /// `"Int"` | `"Float"` | `"Enum"` | `"Section"` | `"Group"` | `"GroupEnd"` | `"Str"` | `"Skip"`
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
    /// Human-readable default: `default_raw / fp_scale` for Float, else `default_raw as f64`.
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
    /// Raw `visualRepresentation`: 0=plain, 4/5=dropmenu, 6=hidden, 8=checkbox.
    #[pyo3(get)]
    show_as: u8,
    enum_items: Vec<String>,
}

#[pymethods]
impl PyField {
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

#[pyclass(name = "Schema")]
struct PySchema {
    inner: Schema,
}

#[pymethods]
impl PySchema {
    #[getter]
    fn name(&self) -> &str {
        &self.inner.name
    }

    fn fields(&self) -> Vec<PyField> {
        self.inner.fields.iter().map(field_from_desc).collect()
    }

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

// ── PyValidationIssue ─────────────────────────────────────────────────────────

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

// ── dict → Values boundary conversion ────────────────────────────────────────

/// Convert a Python dict to the canonical `Values` map.
///
/// Enum fields accept either a string label or an integer index; both are
/// normalized to a string label here so the pure-Rust crates see a clean type.
fn dict_to_values(
    schema: &Schema,
    dict: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<Values> {
    let mut values = Values::new();

    for field in schema.visible_fields() {
        let Some(raw) = dict.get_item(field.key.as_str())? else {
            continue;
        };

        let fv = match &field.kind {
            FieldKind::Int => FieldValue::Int(raw.extract().unwrap_or(0)),
            FieldKind::Float => FieldValue::Float(raw.extract().unwrap_or(0.0)),
            FieldKind::Enum { items } => {
                let label = if let Ok(s) = raw.extract::<String>() {
                    if items.contains(&s) { s } else { items.first().cloned().unwrap_or_default() }
                } else {
                    let idx: usize = raw.extract().unwrap_or(0);
                    items.get(idx).cloned().unwrap_or_default()
                };
                FieldValue::Enum(label)
            }
            FieldKind::Str => FieldValue::Str(raw.extract().unwrap_or_default()),
            _ => continue,
        };

        values.insert(field.key.clone(), fv);
    }

    Ok(values)
}

// ── Python functions ──────────────────────────────────────────────────────────

/// Load an OAD schema from `path` and return a `Schema` object.
#[pyfunction]
fn load_schema(path: &str) -> PyResult<PySchema> {
    let data = std::fs::read(path)
        .map_err(|e| pyo3::exceptions::PyIOError::new_err(format!("{path}: {e}")))?;
    let oad = OadFile::read(&mut Cursor::new(&data))
        .map_err(|e| pyo3::exceptions::PyValueError::new_err(format!("{path}: {e}")))?;
    let schema = wf_attr_schema::from_oad(&oad);
    Ok(PySchema { inner: schema })
}

/// Validate `values` against `schema`.
///
/// `values`: Python dict mapping field keys to display values
/// (Int→int, Float→display float, Enum→string label or int index, Str→string).
/// Returns a list of `ValidationIssue` objects (empty = all good).
#[pyfunction]
fn validate(
    schema: &PySchema,
    values: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<Vec<PyValidationIssue>> {
    let values = dict_to_values(&schema.inner, values)?;
    Ok(wf_attr_validate::validate(&schema.inner, &values)
        .into_iter()
        .map(|i| PyValidationIssue { key: i.key, message: i.message, is_error: i.is_error })
        .collect())
}

/// Export `values` as an iffcomp-format `.iff.txt` string.
/// Missing fields use schema defaults.  Same value contract as `validate`.
#[pyfunction]
fn export_iff_txt(
    schema: &PySchema,
    values: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<String> {
    let values = dict_to_values(&schema.inner, values)?;
    Ok(wf_attr_serialize::to_iff_txt(&schema.inner, &values))
}

/// Import values from an iffcomp-format `.iff.txt` string.
///
/// Returns a plain Python dict mapping field keys to display values
/// (same shape as the dict accepted by `validate` and `export_iff_txt`).
/// Fields absent from the text are omitted from the dict; callers should
/// seed missing fields from schema defaults.
#[pyfunction]
fn import_iff_txt(
    py: Python<'_>,
    schema: &PySchema,
    text: &str,
) -> PyResult<pyo3::Py<pyo3::types::PyDict>> {
    let values = wf_attr_serialize::from_iff_txt(&schema.inner, text)
        .map_err(|e| pyo3::exceptions::PyValueError::new_err(e.message))?;

    let dict = pyo3::types::PyDict::new_bound(py);
    for (key, val) in &values {
        match val {
            FieldValue::Int(i)   => dict.set_item(key, i)?,
            FieldValue::Float(f) => dict.set_item(key, f)?,
            FieldValue::Enum(s)  => dict.set_item(key, s)?,
            FieldValue::Str(s)   => dict.set_item(key, s)?,
        }
    }
    Ok(dict.into())
}

// ── module entry point ────────────────────────────────────────────────────────

/// World Foundry core library — OAD schema loading, validation, and export.
#[pymodule]
fn wf_core(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(load_schema, m)?)?;
    m.add_function(wrap_pyfunction!(validate, m)?)?;
    m.add_function(wrap_pyfunction!(export_iff_txt, m)?)?;
    m.add_function(wrap_pyfunction!(import_iff_txt, m)?)?;
    m.add_class::<PySchema>()?;
    m.add_class::<PyField>()?;
    m.add_class::<PyValidationIssue>()?;
    Ok(())
}
