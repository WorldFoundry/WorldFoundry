//! Validation of World Foundry attribute values against a schema.
//!
//! # Usage
//!
//! ```rust,ignore
//! use wf_attr_schema::{Schema, FieldValue, Values};
//! use wf_attr_validate::validate;
//!
//! let issues = validate(&schema, &values);
//! for issue in &issues {
//!     println!("{}: {} (error={})", issue.key, issue.message, issue.is_error);
//! }
//! ```

use wf_attr_schema::{FieldKind, FieldValue, Schema, Values};

// ── ValidationIssue ───────────────────────────────────────────────────────────

/// A single validation problem found while checking a set of values.
#[derive(Debug, Clone, PartialEq)]
pub struct ValidationIssue {
    /// The field key the issue relates to.
    pub key: String,
    /// Human-readable description of the problem.
    pub message: String,
    /// `true` = hard error (export should be blocked); `false` = warning.
    pub is_error: bool,
}

// ── validate ──────────────────────────────────────────────────────────────────

/// Validate `values` against `schema`, returning any issues found.
///
/// Fields absent from `values` are silently skipped (no "required" check in M1).
/// `Float` values must be display floats (e.g. `50.0` for Mass, not the raw
/// fixed-point integer `3_276_800`).  `Enum` values must be string item labels.
pub fn validate(schema: &Schema, values: &Values) -> Vec<ValidationIssue> {
    let mut issues = Vec::new();

    for field in schema.visible_fields() {
        let Some(val) = values.get(&field.key) else {
            continue;
        };

        let scale = if field.fp_scale > 0.0 {
            field.fp_scale
        } else {
            1.0
        };

        match &field.kind {
            FieldKind::Int => {
                let FieldValue::Int(v) = val else { continue };
                if *v < field.min_raw as i64 || *v > field.max_raw as i64 {
                    issues.push(ValidationIssue {
                        key: field.key.clone(),
                        message: format!(
                            "value {} out of range [{}, {}]",
                            v, field.min_raw, field.max_raw
                        ),
                        is_error: true,
                    });
                }
            }

            FieldKind::Float => {
                let FieldValue::Float(display) = val else { continue };
                let raw = (display * scale).round() as i64;
                if raw < field.min_raw as i64 || raw > field.max_raw as i64 {
                    issues.push(ValidationIssue {
                        key: field.key.clone(),
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

            FieldKind::Enum { items } => {
                let FieldValue::Enum(label) = val else { continue };
                if !items.iter().any(|i| i == label) {
                    issues.push(ValidationIssue {
                        key: field.key.clone(),
                        message: format!(
                            "invalid enum value {:?} — valid: {:?}",
                            label, items
                        ),
                        is_error: true,
                    });
                }
            }

            // Str has no range constraints in M1.
            // Section / Group / GroupEnd / Skip are excluded by visible_fields().
            _ => {}
        }
    }

    issues
}

// ── tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;
    use wf_attr_schema::FieldValue;

    fn player_schema() -> wf_attr_schema::Schema {
        let path = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../wf_oad/tests/fixtures/player.oad");
        let data = std::fs::read(&path).expect("player.oad");
        let oad = wf_oad::OadFile::read(&mut Cursor::new(&data)).expect("parse");
        wf_attr_schema::from_oad(&oad)
    }

    #[test]
    fn no_issues_for_defaults() {
        let schema = player_schema();
        let mut values = Values::new();
        for field in schema.visible_fields() {
            let fv = match &field.kind {
                wf_attr_schema::FieldKind::Int => FieldValue::Int(field.default_raw as i64),
                wf_attr_schema::FieldKind::Float => {
                    let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };
                    FieldValue::Float(field.default_raw as f64 / scale)
                }
                wf_attr_schema::FieldKind::Enum { items } => {
                    let idx = field.default_raw as usize;
                    FieldValue::Enum(items.get(idx).cloned().unwrap_or_default())
                }
                wf_attr_schema::FieldKind::Str => FieldValue::Str(String::new()),
                _ => continue,
            };
            values.insert(field.key.clone(), fv);
        }
        let issues = validate(&schema, &values);
        assert!(issues.is_empty(), "default values should all pass: {:?}", issues);
    }

    #[test]
    fn out_of_range_int_is_error() {
        let schema = player_schema();
        let mut values = Values::new();
        // hp is an Int field; stuff a value well above max_raw
        values.insert("hp".to_owned(), FieldValue::Int(99_999));
        let issues = validate(&schema, &values);
        assert!(issues.iter().any(|i| i.key == "hp" && i.is_error));
    }

    #[test]
    fn invalid_enum_label_is_error() {
        let schema = player_schema();
        let mut values = Values::new();
        values.insert("Mobility".to_owned(), FieldValue::Enum("Warp10".to_owned()));
        let issues = validate(&schema, &values);
        assert!(issues.iter().any(|i| i.key == "Mobility" && i.is_error));
    }
}
