//! Serialization and deserialization of World Foundry attribute values.
//!
//! `.iff.txt` is a first-class format, fully interchangeable with binary `.iff`
//! via `iffcomp` (`.iff.txt` → `.iff`) and `iffdump -f-` (`.iff` → `.iff.txt`).
//!
//! # Export
//!
//! ```rust,ignore
//! use wf_attr_schema::{Schema, Values};
//! use wf_attr_serialize::to_iff_txt;
//!
//! let text = to_iff_txt(&schema, &values);
//! std::fs::write("enemy.iff.txt", &text).unwrap();
//! ```
//!
//! # Import
//!
//! ```rust,ignore
//! use wf_attr_serialize::from_iff_txt;
//!
//! let text = std::fs::read_to_string("enemy.iff.txt").unwrap();
//! let values = from_iff_txt(&schema, &text)?;
//! ```

use wf_attr_schema::{FieldKind, FieldValue, Schema, Values};

// ── to_iff_txt ────────────────────────────────────────────────────────────────

/// Serialize `values` as an iffcomp-compatible `.iff.txt` string.
///
/// Fields missing from `values` fall back to the schema default.
/// `Float` values must be display floats (e.g. `50.0`); the serializer converts
/// them back to raw fixed-point integers before writing.
///
/// The FOURCC is derived from the first 1–4 characters of the schema name,
/// uppercased and space-padded to exactly 4 bytes.
pub fn to_iff_txt(schema: &Schema, values: &Values) -> String {
    // FOURCC: first 4 chars of schema name, uppercased, space-padded.
    let name_bytes = schema.name.as_bytes();
    let mut fourcc = [b' '; 4];
    for (i, &b) in name_bytes.iter().take(4).enumerate() {
        fourcc[i] = b.to_ascii_uppercase();
    }
    let fourcc_str: String = fourcc.iter().map(|&b| b as char).collect();

    let mut payload: Vec<u8> = Vec::new();
    let mut lines:   Vec<String> = Vec::new();

    for field in schema.visible_fields() {
        let val = values.get(&field.key);
        let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };

        match &field.kind {
            FieldKind::Section | FieldKind::Group | FieldKind::GroupEnd => continue,

            FieldKind::Int => {
                let raw: i32 = match val {
                    Some(FieldValue::Int(i)) => *i as i32,
                    _                        => field.default_raw,
                };
                append_fixed_width(&mut payload, &mut lines, raw, field.byte_width, &field.key, None);
            }

            FieldKind::Enum { items } => {
                let raw: i32 = match val {
                    Some(FieldValue::Enum(label)) => items
                        .iter()
                        .position(|i| i == label)
                        .map(|p| p as i32)
                        .unwrap_or(field.default_raw),
                    _ => field.default_raw,
                };
                append_fixed_width(&mut payload, &mut lines, raw, field.byte_width, &field.key, None);
            }

            FieldKind::Float => {
                let display: f64 = match val {
                    Some(FieldValue::Float(f)) => *f,
                    _                          => field.default_raw as f64 / scale,
                };
                let raw = (display * scale).round() as i32;
                append_fixed_width(
                    &mut payload, &mut lines, raw, field.byte_width, &field.key,
                    Some(display),
                );
            }

            FieldKind::Str => {
                let s: &str = match val {
                    Some(FieldValue::Str(s)) => s.as_str(),
                    _                        => "",
                };
                let b = s.as_bytes();
                payload.extend_from_slice(b);
                if b.is_empty() {
                    lines.push(format!("\t// {} (empty string)", field.key));
                } else {
                    lines.push(format!(
                        "\t{}\t// {}",
                        hex_bytes(b),
                        field.key,
                    ));
                }
            }

            FieldKind::Skip => {}
        }
    }

    let size = payload.len();
    let mut out = String::new();
    out.push_str("//=============================================================================\n");
    out.push_str(&format!(
        "// {} — exported by wf_tools v{}\n",
        schema.name,
        env!("CARGO_PKG_VERSION"),
    ));
    out.push_str("//=============================================================================\n");
    out.push_str(&format!("{{ '{}'\t\t// Size = {}\n", fourcc_str, size));
    for line in &lines {
        out.push_str(line);
        out.push('\n');
    }
    out.push_str("}\n");
    out
}

// ── from_iff_txt ──────────────────────────────────────────────────────────────

/// Error returned by [`from_iff_txt`].
#[derive(Debug, Clone, PartialEq)]
pub struct ImportError {
    pub message: String,
}

impl std::fmt::Display for ImportError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.message)
    }
}

impl std::error::Error for ImportError {}

impl ImportError {
    fn new(msg: impl Into<String>) -> Self {
        ImportError { message: msg.into() }
    }
}

/// Parse an iffcomp-format `.iff.txt` string into a `Values` map.
///
/// Fields are matched by the `// key` comment that [`to_iff_txt`] emits on
/// every data line, so field order in the file does not have to match the
/// schema order and unknown keys are silently skipped.
///
/// Returns only the fields that were present in the file; callers should fill
/// in missing fields from schema defaults as needed.
pub fn from_iff_txt(schema: &Schema, text: &str) -> Result<Values, ImportError> {
    // Build a quick lookup: key → field descriptor.
    let field_map: std::collections::HashMap<&str, _> = schema
        .visible_fields()
        .map(|f| (f.key.as_str(), f))
        .collect();

    let mut values = Values::new();

    for line in text.lines() {
        let trimmed = line.trim();

        // Handle empty-string fields: `// key (empty string)`
        if trimmed.starts_with("//") {
            let comment = trimmed[2..].trim();
            if let Some(key) = comment.strip_suffix("(empty string)").map(str::trim) {
                if field_map.contains_key(key) {
                    values.insert(key.to_owned(), FieldValue::Str(String::new()));
                }
            }
            continue;
        }

        // Only process data lines: start with '$' after whitespace stripping.
        if !trimmed.starts_with('$') {
            continue;
        }

        // Split on '//' to separate the hex payload from the comment.
        // Format: `$XX $XX ...  // key` or `$XX $XX ...  // key (display)`
        let Some(comment_start) = trimmed.find("//") else {
            continue;
        };
        let hex_part     = trimmed[..comment_start].trim();
        let comment_part = trimmed[comment_start + 2..].trim();

        // Extract the key from the comment.  Float fields append " (display_value)"
        // so we strip from the first '(' and trim — this also handles multi-word
        // keys like "Render Type" or "Mesh Name" correctly.
        let key = if let Some(paren) = comment_part.find('(') {
            comment_part[..paren].trim()
        } else {
            comment_part.trim()
        };
        if key.is_empty() {
            continue;
        }

        let Some(field) = field_map.get(key) else {
            // Key not in schema — skip gracefully.
            continue;
        };

        // Parse the hex bytes.
        let bytes: Result<Vec<u8>, _> = hex_part
            .split_whitespace()
            .map(|token| {
                let digits = token.trim_start_matches('$');
                u8::from_str_radix(digits, 16)
                    .map_err(|_| ImportError::new(format!("bad hex token {:?}", token)))
            })
            .collect();
        let bytes = bytes?;

        let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };

        let fv = match &field.kind {
            FieldKind::Int => {
                let raw = le_bytes_to_i32(&bytes, &field.key)?;
                FieldValue::Int(raw as i64)
            }
            FieldKind::Float => {
                let raw = le_bytes_to_i32(&bytes, &field.key)?;
                FieldValue::Float(raw as f64 / scale)
            }
            FieldKind::Enum { items } => {
                let raw = le_bytes_to_i32(&bytes, &field.key)? as usize;
                let label = items.get(raw).cloned().ok_or_else(|| {
                    ImportError::new(format!(
                        "{}: enum index {} out of range (max {})",
                        field.key, raw, items.len().saturating_sub(1)
                    ))
                })?;
                FieldValue::Enum(label)
            }
            FieldKind::Str => {
                let s = String::from_utf8(bytes.clone()).map_err(|_| {
                    ImportError::new(format!("{}: invalid UTF-8 in string field", field.key))
                })?;
                FieldValue::Str(s)
            }
            _ => continue,
        };

        values.insert(field.key.clone(), fv);
    }

    Ok(values)
}

fn le_bytes_to_i32(bytes: &[u8], key: &str) -> Result<i32, ImportError> {
    match bytes.len() {
        1 => Ok(i8::from_le_bytes([bytes[0]]) as i32),
        2 => Ok(i16::from_le_bytes([bytes[0], bytes[1]]) as i32),
        4 => Ok(i32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]])),
        n => Err(ImportError::new(format!(
            "{}: expected 1/2/4 bytes, got {}", key, n
        ))),
    }
}

// ── helpers ───────────────────────────────────────────────────────────────────

fn hex_bytes(b: &[u8]) -> String {
    b.iter().map(|x| format!("${:02X}", x)).collect::<Vec<_>>().join(" ")
}

fn append_fixed_width(
    payload: &mut Vec<u8>,
    lines:   &mut Vec<String>,
    raw:     i32,
    byte_width: u8,
    key:     &str,
    display: Option<f64>,
) {
    let bytes = raw.to_le_bytes();
    let width = byte_width.max(4) as usize;
    let b = &bytes[..width];
    payload.extend_from_slice(b);
    let comment = match display {
        Some(f) => format!("\t{}\t// {} ({:.6})", hex_bytes(b), key, f),
        None    => format!("\t{}\t// {}", hex_bytes(b), key),
    };
    lines.push(comment);
}

// ── tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    fn player_schema() -> Schema {
        let path = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../wf_oad/tests/fixtures/player.oad");
        let data = std::fs::read(&path).expect("player.oad");
        let oad = wf_oad::OadFile::read(&mut Cursor::new(&data)).expect("parse");
        wf_attr_schema::from_oad(&oad)
    }

    #[test]
    fn to_iff_txt_produces_valid_header() {
        let schema = player_schema();
        let text = to_iff_txt(&schema, &Values::new());
        assert!(text.contains("{ 'PLAY'"), "FOURCC should be PLAY");
        assert!(text.contains("// Size ="), "should have size comment");
        assert!(text.ends_with("}\n"), "should end with closing brace");
    }

    #[test]
    fn to_iff_txt_defaults_produce_output() {
        let schema = player_schema();
        let text = to_iff_txt(&schema, &Values::new());
        // Every visible non-structural field should produce a line
        assert!(text.contains("// hp"), "hp field should appear");
        assert!(text.contains("// Mass"), "Mass field should appear");
    }

    #[test]
    fn float_value_written_as_raw_fixed_point() {
        let schema = player_schema();
        let mass = schema.visible_fields().find(|f| f.key == "Mass").unwrap();
        // Mass = 1.0 → raw = 65536 → $00 $00 $01 $00
        let mut values = Values::new();
        values.insert("Mass".to_owned(), FieldValue::Float(1.0));
        let text = to_iff_txt(&schema, &values);
        assert!(
            text.contains("$00 $00 $01 $00"),
            "Mass 1.0 should serialize as $00 $00 $01 $00 (raw 65536 LE): {}",
            &text[text.find("// Mass").unwrap_or(0)..],
        );
        let _ = mass; // suppress unused warning
    }

    #[test]
    fn round_trip_default_values() {
        let schema = player_schema();

        // Build a Values map from schema defaults.
        let mut original = Values::new();
        for field in schema.visible_fields() {
            let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };
            let fv = match &field.kind {
                wf_attr_schema::FieldKind::Int   => FieldValue::Int(field.default_raw as i64),
                wf_attr_schema::FieldKind::Float => FieldValue::Float(field.default_raw as f64 / scale),
                wf_attr_schema::FieldKind::Enum { items } => {
                    let idx = field.default_raw.max(0) as usize;
                    FieldValue::Enum(items.get(idx).cloned().unwrap_or_default())
                }
                wf_attr_schema::FieldKind::Str => FieldValue::Str(String::new()),
                _ => continue,
            };
            original.insert(field.key.clone(), fv);
        }

        // Export → import → compare.
        let text = to_iff_txt(&schema, &original);
        let imported = from_iff_txt(&schema, &text).expect("round-trip parse failed");

        for (key, orig_val) in &original {
            let imp_val = imported.get(key).unwrap_or_else(|| {
                panic!("field {:?} missing after round-trip", key)
            });
            match (orig_val, imp_val) {
                (FieldValue::Float(a), FieldValue::Float(b)) => {
                    // Fixed-point quantisation: allow ±1 ULP in raw space.
                    let field = schema.visible_fields().find(|f| &f.key == key).unwrap();
                    let scale = if field.fp_scale > 0.0 { field.fp_scale } else { 1.0 };
                    let diff = ((a - b) * scale).abs();
                    assert!(diff < 1.5, "Float {key}: {a} vs {b} (diff {diff} raw ULPs)");
                }
                _ => assert_eq!(orig_val, imp_val, "field {key} mismatch"),
            }
        }
    }
}
