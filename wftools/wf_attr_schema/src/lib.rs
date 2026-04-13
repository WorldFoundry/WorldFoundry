//! Normalized attribute schema model for World Foundry OAD files.
//!
//! Takes a parsed [`wf_oad::OadFile`] and produces host-agnostic
//! [`FieldDescriptor`]s suitable for driving Blender UI or other editors.
//!
//! # Field kinds
//!
//! | [`FieldKind`]   | OAD `ButtonType`                          | Notes                              |
//! |-----------------|-------------------------------------------|------------------------------------|
//! | `Int`           | Int8 / Int16 / Int32 (no pipe items)      | raw i32 stored value               |
//! | `Float`         | Fixed16 / Fixed32                         | raw i32; scale = 65536 for Fixed32 |
//! | `Enum`          | Int8 / Int16 / Int32 + pipe-sep string    | items from `string` field          |
//! | `Group`         | PropertySheet / GroupStart                | section header, no stored value    |
//! | `Str`           | String / Filename                         | UTF-8 string value                 |
//!
//! Everything else (LevelconFlags, XData, ObjectReference, GroupStop, etc.)
//! is emitted as [`FieldKind::Skip`] and should be hidden from the editor UI.

use wf_oad::{ButtonType, OadFile};

// ── field kind ────────────────────────────────────────────────────────────────

/// The normalized kind of an editable field.
#[derive(Debug, Clone, PartialEq)]
pub enum FieldKind {
    /// Integer numeric field (Int8 / Int16 / Int32).
    Int,
    /// Fixed-point numeric field displayed as float (Fixed16 / Fixed32).
    /// Stored internally as raw i32; divide by 65536 to get float value.
    Float,
    /// Integer field with a discrete set of named choices.
    /// Items are pipe-separated from the OAD `string` field.
    Enum { items: Vec<String> },
    /// Section/group header — no stored value, used for UI grouping.
    Group,
    /// Free-text string field (String / Filename).
    Str,
    /// Not shown in the editor UI for milestone 1.
    Skip,
}

impl FieldKind {
    /// Short tag string used in the Python API.
    pub fn tag(&self) -> &'static str {
        match self {
            FieldKind::Int    => "Int",
            FieldKind::Float  => "Float",
            FieldKind::Enum { .. } => "Enum",
            FieldKind::Group  => "Group",
            FieldKind::Str    => "Str",
            FieldKind::Skip   => "Skip",
        }
    }
}

// ── field descriptor ──────────────────────────────────────────────────────────

/// Fixed-point scale factor for Fixed32 fields (2^16).
pub const FIXED32_SCALE: f64 = 65536.0;
/// Fixed-point scale factor for Fixed16 fields (2^8).
pub const FIXED16_SCALE: f64 = 256.0;

/// A normalized description of one field in a schema.
#[derive(Debug, Clone)]
pub struct FieldDescriptor {
    /// Internal key from the OAD `name` field (e.g. `"Mass"`).
    pub key: String,
    /// Human-readable label from the OAD `display_name` field, falling back
    /// to `key` if the display name is empty.
    pub label: String,
    /// Normalized field kind.
    pub kind: FieldKind,
    /// Help text from the OAD `helpMessage` field.
    pub help: String,
    /// Section/group this field belongs to (from the enclosing PropertySheet
    /// or GroupStart entry).
    pub group: String,
    /// Raw i32 minimum from OAD.
    pub min_raw: i32,
    /// Raw i32 maximum from OAD.
    pub max_raw: i32,
    /// Raw i32 default from OAD.
    pub default_raw: i32,
    /// Byte width of the serialized value (1, 2, or 4).
    /// Used by the exporter to pack the correct number of bytes.
    /// 0 for Group and Skip fields (they contribute no bytes).
    pub byte_width: u8,
    /// Fixed-point scale factor (for Float fields only).
    /// `display_value = raw_value / fp_scale`.
    /// 0.0 for non-Float fields.
    pub fp_scale: f64,
    /// Raw `visualRepresentation` byte from the OAD entry.
    /// Useful for rendering hints:
    ///   0 → plain numeric
    ///   4 → dropmenu (use dropdown)
    ///   5 → dropmenu (use dropdown)
    ///   6 → hidden / internal (consider skipping in UI)
    ///   8 → checkbox (render bool-style toggle)
    pub show_as: u8,
}

impl FieldDescriptor {
    /// Returns `true` if this field should be shown in the editor UI.
    pub fn is_visible(&self) -> bool {
        !matches!(self.kind, FieldKind::Skip | FieldKind::Group)
    }
}

// ── schema ────────────────────────────────────────────────────────────────────

/// A normalized schema derived from one OAD file.
#[derive(Debug, Clone)]
pub struct Schema {
    /// Display name from the OAD header (e.g. `"Room"`).
    pub name: String,
    /// All fields, including Group headers and Skip entries.
    pub fields: Vec<FieldDescriptor>,
}

impl Schema {
    /// Returns only the fields that should appear in the editor UI
    /// (excludes [`FieldKind::Skip`] and [`FieldKind::Group`]).
    pub fn visible_fields(&self) -> impl Iterator<Item = &FieldDescriptor> {
        self.fields.iter().filter(|f| f.is_visible())
    }
}

// ── OAD → Schema conversion ───────────────────────────────────────────────────

/// Convert a parsed [`OadFile`] into a normalized [`Schema`].
pub fn from_oad(oad: &OadFile) -> Schema {
    let name = oad.header.display_name().to_owned();
    let mut fields = Vec::with_capacity(oad.entries.len());
    let mut current_group = String::new();

    for entry in &oad.entries {
        let key   = entry.name_str().to_owned();
        // GroupStart entries carry `display_name = "displayName"` (a placeholder),
        // so use the key as the label for those.
        let raw_label = entry.display_name();
        let label = if raw_label.is_empty() || raw_label == "displayName" {
            key.clone()
        } else {
            raw_label.to_owned()
        };
        let help  = entry.help_str().to_owned();

        let kind = classify(entry.button_type, entry.string_str());

        // Track the current group from PropertySheet / GroupStart names.
        match entry.button_type {
            ButtonType::PropertySheet | ButtonType::GroupStart => {
                current_group = label.clone();
            }
            ButtonType::GroupStop | ButtonType::EndCommon => {
                current_group.clear();
            }
            _ => {}
        }

        let (byte_width, fp_scale) = field_layout(entry.button_type);

        fields.push(FieldDescriptor {
            key,
            label,
            kind,
            help,
            group:       current_group.clone(),
            min_raw:     entry.min,
            max_raw:     entry.max,
            default_raw: entry.def,
            byte_width,
            fp_scale,
            show_as:     entry.show_as,
        });
    }

    Schema { name, fields }
}

/// Return `(byte_width, fp_scale)` for a field's binary serialization layout.
/// `byte_width` is 0 for Group/Skip (no bytes in output).
/// `fp_scale` is the divisor to convert raw i32 to display float; 0.0 if not float.
fn field_layout(bt: ButtonType) -> (u8, f64) {
    match bt {
        ButtonType::Fixed16                            => (2, FIXED16_SCALE),
        ButtonType::Fixed32                            => (4, FIXED32_SCALE),
        ButtonType::Int8                               => (1, 0.0),
        ButtonType::Int16                              => (2, 0.0),
        ButtonType::Int32                              => (4, 0.0),
        ButtonType::String | ButtonType::Filename      => (0, 0.0), // variable; exporter handles
        ButtonType::PropertySheet | ButtonType::GroupStart => (0, 0.0),
        _                                              => (0, 0.0),
    }
}

/// Map an OAD `ButtonType` + `string` field content to a [`FieldKind`].
fn classify(bt: ButtonType, string_field: &str) -> FieldKind {
    match bt {
        // Numeric integer fields — check for pipe-separated enum items first.
        ButtonType::Int8 | ButtonType::Int16 | ButtonType::Int32 => {
            let items = parse_pipe_items(string_field);
            if items.is_empty() {
                FieldKind::Int
            } else {
                FieldKind::Enum { items }
            }
        }

        // Fixed-point numeric fields.
        ButtonType::Fixed16 | ButtonType::Fixed32 => FieldKind::Float,

        // Free-text string fields.
        ButtonType::String | ButtonType::Filename => FieldKind::Str,

        // Section / group headers.
        ButtonType::PropertySheet | ButtonType::GroupStart => FieldKind::Group,

        // Everything else is skipped in milestone 1.
        _ => FieldKind::Skip,
    }
}

/// Split a pipe-separated string into a list of item labels.
/// Returns an empty Vec if the string is empty or contains no `|`.
fn parse_pipe_items(s: &str) -> Vec<String> {
    if s.is_empty() || !s.contains('|') {
        return Vec::new();
    }
    s.split('|').map(|item| item.to_owned()).collect()
}

// ── tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    fn load_fixture(name: &str) -> OadFile {
        let path = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../wf_oad/tests/fixtures")
            .join(name);
        let data = std::fs::read(&path)
            .unwrap_or_else(|e| panic!("failed to read {name}: {e}"));
        OadFile::read(&mut Cursor::new(&data))
            .unwrap_or_else(|e| panic!("failed to parse {name}: {e}"))
    }

    #[test]
    fn player_schema_name_and_field_count() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        assert_eq!(schema.name, "Player");
        // player.oad fixture has 93 entries (including Skip and Group)
        assert_eq!(schema.fields.len(), 93);
    }

    #[test]
    fn player_schema_has_float_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let mass = schema.fields.iter().find(|f| f.key == "Mass").expect("Mass field");
        assert!(matches!(mass.kind, FieldKind::Float));
        assert_eq!(mass.label, "Mass");
        assert_eq!(mass.min_raw, 0);
        assert_eq!(mass.max_raw, 6553600); // 100 * 65536
    }

    #[test]
    fn player_schema_has_int_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let hp = schema.fields.iter().find(|f| f.key == "hp").expect("hp field");
        assert!(matches!(hp.kind, FieldKind::Int));
    }

    #[test]
    fn player_schema_has_enum_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let field = schema.fields.iter()
            .find(|f| f.key == "Mobility")
            .expect("Mobility field");
        match &field.kind {
            FieldKind::Enum { items } => {
                assert_eq!(items, &["Anchored", "Physics", "Path", "Camera", "Follow"]);
            }
            other => panic!("expected Enum, got {other:?}"),
        }
    }

    #[test]
    fn player_schema_has_str_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let field = schema.fields.iter()
            .find(|f| f.key == "Mesh Name")
            .expect("Mesh Name field");
        assert!(matches!(field.kind, FieldKind::Str));
    }

    #[test]
    fn player_schema_has_group_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let group = schema.fields.iter()
            .find(|f| matches!(f.kind, FieldKind::Group) && f.label == "Movement")
            .expect("Movement group");
        assert!(matches!(group.kind, FieldKind::Group));
    }

    #[test]
    fn player_schema_group_start_uses_key_as_label() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        // BUTTON_GROUP_START entries have Display="displayName"; label should be key
        let group = schema.fields.iter()
            .find(|f| matches!(f.kind, FieldKind::Group) && f.label == "Path")
            .expect("Path group (from GROUP_START)");
        assert_eq!(group.label, "Path");
    }

    #[test]
    fn visible_fields_excludes_skip_and_group() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        for f in schema.visible_fields() {
            assert!(!matches!(f.kind, FieldKind::Skip | FieldKind::Group),
                "visible_fields should not include {}: {:?}", f.key, f.kind);
        }
    }

    #[test]
    fn group_tracking_sets_group_on_fields() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let mass = schema.fields.iter().find(|f| f.key == "Mass").expect("Mass");
        // Mass comes after the "Common" PropertySheet
        assert_eq!(mass.group, "Common");
    }

    #[test]
    fn parse_pipe_items_splits_correctly() {
        assert_eq!(
            super::parse_pipe_items("First Vertex|Min|Average|Max"),
            vec!["First Vertex", "Min", "Average", "Max"]
        );
        assert!(super::parse_pipe_items("").is_empty());
        assert!(super::parse_pipe_items("plain").is_empty());
    }
}
