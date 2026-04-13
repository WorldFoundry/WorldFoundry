//! Serialization of World Foundry attribute values to `.iff.txt` format.
//!
//! `.iff.txt` is a first-class format, fully interchangeable with binary `.iff`
//! via `iffcomp` (`.iff.txt` → `.iff`) and `iffdump -f-` (`.iff` → `.iff.txt`).
//!
//! # Usage
//!
//! ```rust,ignore
//! use wf_attr_schema::{Schema, Values};
//! use wf_attr_serialize::to_iff_txt;
//!
//! let text = to_iff_txt(&schema, &values);
//! std::fs::write("enemy.iff.txt", &text).unwrap();
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
}
