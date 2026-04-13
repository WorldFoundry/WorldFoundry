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
use wf_attr_schema::{FieldKind, Schema};
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
    // For Enum fields: the list of choice labels.
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

fn field_from_desc(desc: &wf_attr_schema::FieldDescriptor) -> PyField {
    let (kind_tag, enum_items) = match &desc.kind {
        FieldKind::Enum { items } => ("Enum".to_owned(), items.clone()),
        other => (other.tag().to_owned(), Vec::new()),
    };
    PyField {
        key:         desc.key.clone(),
        label:       desc.label.clone(),
        kind:        kind_tag,
        help:        desc.help.clone(),
        group:       desc.group.clone(),
        min_raw:     desc.min_raw,
        max_raw:     desc.max_raw,
        default_raw: desc.default_raw,
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
/// `values` is a Python dict mapping field keys to raw int/float/str values.
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

        match &field.kind {
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
                let v: i64 = raw_val.extract().unwrap_or(0);
                if v < field.min_raw as i64 || v > field.max_raw as i64 {
                    issues.push(PyValidationIssue {
                        key:      field.key.clone(),
                        message:  format!(
                            "raw value {} out of range [{}, {}]",
                            v, field.min_raw, field.max_raw
                        ),
                        is_error: true,
                    });
                }
            }
            wf_attr_schema::FieldKind::Enum { items } => {
                let v: i64 = raw_val.extract().unwrap_or(-1);
                if v < 0 || v as usize >= items.len() {
                    issues.push(PyValidationIssue {
                        key:      field.key.clone(),
                        message:  format!(
                            "enum index {} out of range [0, {}]",
                            v, items.len().saturating_sub(1)
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

/// Export `values` as a human-readable `.iff.txt` artifact.
///
/// `values` is a Python dict mapping field keys to raw int/str values.
/// Returns the text as a Python string.
///
/// This is a milestone-1 debug format: each visible field is listed as a
/// comment line with its key, kind, label, and current value.  Full binary
/// IFF serialization is deferred to milestone 2.
#[pyfunction]
fn export_iff_txt(
    schema: &PySchema,
    values: &pyo3::Bound<'_, pyo3::types::PyDict>,
) -> PyResult<String> {
    let mut out = String::new();
    out.push_str(&format!(
        "//=============================================================================\n"
    ));
    out.push_str(&format!(
        "// {} — exported by wf_core v{}\n",
        schema.inner.name,
        env!("CARGO_PKG_VERSION")
    ));
    out.push_str(
        "//=============================================================================\n",
    );
    out.push_str(&format!("{{ '{}'\n", &schema.inner.name[..4.min(schema.inner.name.len())]));
    out.push_str("//\t[kind]\tkey\t= value\n");

    for field in schema.inner.visible_fields() {
        let val = values.get_item(field.key.as_str())?;
        let val_str = match val {
            Some(v) => v.str().map(|s| s.to_string()).unwrap_or_else(|_| "?".to_owned()),
            None => "(default)".to_owned(),
        };
        out.push_str(&format!(
            "//\t[{}]\t{}\t= {}\n",
            field.kind.tag(),
            field.key,
            val_str
        ));
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
