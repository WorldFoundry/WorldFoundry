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
//! | `Str`           | String                                    | UTF-8 string value                 |
//! | `FileRef`       | Filename / MeshName                       | File path string with browse filter|
//! | `Bool`          | NoInstances / NoMesh / SingleInstance / … | 0/1 flag; serialized as 4 bytes    |
//!
//! Everything else (XData, GroupStop, EndCommon, etc.)
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
    /// Collapsible section header (`PropertySheet`).
    /// `default_raw` is 1 if the section starts open, 0 if closed.
    Section,
    /// Non-collapsible sub-group header (`GroupStart`).
    Group,
    /// Marks the end of the nearest enclosing `Group` box (`GroupStop`).
    GroupEnd,
    /// Free-text string field (String / Filename).
    Str,
    /// Level-compiler boolean flag (NoInstances / NoMesh / SingleInstance / …).
    ///
    /// Serialized as a 4-byte LE integer (0 = false, 1 = true).
    /// Shown in the editor as a checkbox regardless of `show_as`.
    Bool,
    /// File path field (Filename / MeshName).
    ///
    /// The value is a UTF-8 filename string (e.g. `"player.iff"`).
    /// `filter` is a semicolon-separated glob pattern derived from the OAD
    /// `lpstr_filter` field (e.g. `"*.iff;*.bmp;*.tga"`); empty if no filter.
    FileRef { filter: String },
    /// Reference to another game object (ObjectReference / ClassReference /
    /// CameraReference / LightReference).
    ///
    /// Stored in Blender as the referenced object's name (a string).
    /// Serialized to binary as a 4-byte LE integer index (0 = unresolved at
    /// edit time; resolved by the level compiler).
    ///
    /// `class_tag` is empty for a general object reference, or contains a
    /// class name (from the OAD `string` field) to filter the allowed types.
    ObjRef { class_tag: String },
    /// Not shown in the editor UI.
    Skip,
}

impl FieldKind {
    /// Short tag string used in the Python API.
    pub fn tag(&self) -> &'static str {
        match self {
            FieldKind::Int         => "Int",
            FieldKind::Float       => "Float",
            FieldKind::Enum { .. } => "Enum",
            FieldKind::Section     => "Section",
            FieldKind::Group       => "Group",
            FieldKind::GroupEnd    => "GroupEnd",
            FieldKind::Str           => "Str",
            FieldKind::Bool          => "Bool",
            FieldKind::FileRef { .. } => "FileRef",
            FieldKind::ObjRef { .. } => "ObjRef",
            FieldKind::Skip          => "Skip",
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
    /// Semicolon-separated glob patterns for `FileRef` fields (e.g. `"*.iff;*.bmp"`).
    /// Derived from the OAD `lpstr_filter` Windows file-dialog filter string.
    /// Empty string for all other field kinds.
    pub file_filter: String,
}

impl FieldDescriptor {
    /// Returns `true` if this field should be shown in the editor UI.
    ///
    /// Excludes [`FieldKind::Skip`] and fields with `show_as == 6`
    /// (hidden/internal in the original `attribedit`).
    /// [`FieldKind::Section`], [`FieldKind::Group`], and [`FieldKind::GroupEnd`]
    /// are structural and are included so the panel can render them.
    pub fn is_visible(&self) -> bool {
        if matches!(self.kind, FieldKind::Skip) {
            return false;
        }
        // show_as == 6 means "hidden in attribedit", but Bool/levelcon flags
        // use it as a default (all flags have show_as=6); always show them.
        if self.show_as == 6 && !matches!(self.kind, FieldKind::Bool) {
            return false;
        }
        true
    }

    /// Returns `true` if this field contributes bytes to the serialized payload.
    /// Structural fields (Section, Group, GroupEnd) do not.
    /// FileRef fields with max_raw == 0 also contribute no bytes (variable-width,
    /// handled by the level compiler rather than the Blender exporter).
    pub fn has_payload(&self) -> bool {
        !matches!(
            self.kind,
            FieldKind::Section | FieldKind::Group | FieldKind::GroupEnd | FieldKind::Skip
        )
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

        let kind = classify(entry.button_type, entry.string_str(), entry.lpstr_filter_bytes());

        // Track the current group from PropertySheet / GroupStart names.
        match entry.button_type {
            ButtonType::PropertySheet => {
                current_group = label.clone();
            }
            ButtonType::GroupStart => {
                // sub-group: keep the parent section name as group
            }
            ButtonType::GroupStop | ButtonType::EndCommon => {
                // GroupStop closes a GroupStart; EndCommon closes a CommonBlock.
                // We don't clear current_group here — the parent section is still active.
            }
            _ => {}
        }

        let (byte_width, fp_scale) = field_layout(entry.button_type);
        let file_filter = if let FieldKind::FileRef { filter } = &kind {
            filter.clone()
        } else {
            String::new()
        };

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
            file_filter,
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
        ButtonType::String | ButtonType::Filename | ButtonType::MeshName => (0, 0.0), // variable; exporter handles
        ButtonType::PropertySheet | ButtonType::GroupStart => (0, 0.0),
        // Bool levelcon flags serialize as a 4-byte LE integer (0 or 1).
        ButtonType::NoInstances
        | ButtonType::NoMesh
        | ButtonType::SingleInstance
        | ButtonType::Template
        | ButtonType::ExtractCamera
        | ButtonType::Room
        | ButtonType::ExtractCamera2
        | ButtonType::ExtractCameraNew
        | ButtonType::Waveform
        | ButtonType::ExtractLight
        | ButtonType::Shortcut                             => (4, 0.0),
        // Object/class/camera/light references serialize as a 4-byte LE integer index.
        ButtonType::ObjectReference
        | ButtonType::ClassReference
        | ButtonType::CameraReference
        | ButtonType::LightReference                   => (4, 0.0),
        _                                              => (0, 0.0),
    }
}

/// Extract semicolon-separated glob patterns from a Windows file-dialog filter byte string.
///
/// The filter format is null-separated pairs: `Desc\0Pattern\0Desc\0Pattern\0\0`.
/// This function returns the patterns (odd segments) joined by `;`.
fn parse_filter(raw: &[u8]) -> String {
    raw.split(|&b| b == 0)
        .filter(|seg| !seg.is_empty())
        .enumerate()
        .filter(|(i, _)| i % 2 == 1) // odd indices = patterns
        .map(|(_, seg)| String::from_utf8_lossy(seg).into_owned())
        .collect::<Vec<_>>()
        .join(";")
}

/// Map an OAD `ButtonType` + `string` field content to a [`FieldKind`].
fn classify(bt: ButtonType, string_field: &str, lpstr_filter: &[u8]) -> FieldKind {
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

        // Free-text string fields (no file filter).
        ButtonType::String => FieldKind::Str,

        // File-path fields — carry a browse filter from lpstr_filter.
        ButtonType::Filename | ButtonType::MeshName =>
            FieldKind::FileRef { filter: parse_filter(lpstr_filter) },

        // Collapsible section header (rollup in attribedit).
        ButtonType::PropertySheet => FieldKind::Section,

        // Non-collapsible sub-group header.
        ButtonType::GroupStart => FieldKind::Group,

        // End of a GroupStart box.
        ButtonType::GroupStop => FieldKind::GroupEnd,

        // Object / class / camera / light references.
        // The string field may carry a class-name filter (ClassReference);
        // for plain ObjectReference it is usually empty.
        ButtonType::ObjectReference
        | ButtonType::ClassReference
        | ButtonType::CameraReference
        | ButtonType::LightReference => FieldKind::ObjRef {
            class_tag: string_field.trim().to_owned(),
        },

        // Level-compiler boolean flags.
        ButtonType::NoInstances
        | ButtonType::NoMesh
        | ButtonType::SingleInstance
        | ButtonType::Template
        | ButtonType::ExtractCamera
        | ButtonType::Room
        | ButtonType::ExtractCamera2
        | ButtonType::ExtractCameraNew
        | ButtonType::Waveform
        | ButtonType::ExtractLight
        | ButtonType::Shortcut => FieldKind::Bool,

        // Everything else is not yet surfaced in the editor.
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

// ── field values ─────────────────────────────────────────────────────────────

/// A normalized, host-agnostic field value used for validation and serialization.
///
/// `Float` fields use *display* values (e.g. `50.0` for Mass), not raw fixed-point.
/// `Enum` fields use the string item label, already resolved from an index if needed.
#[derive(Debug, Clone, PartialEq)]
pub enum FieldValue {
    /// Integer value (`Int` fields).
    Int(i64),
    /// Human-readable float (`Float` fields; raw ÷ `fp_scale` = this).
    Float(f64),
    /// Enum item label (`Enum` fields).
    Enum(String),
    /// UTF-8 string (`Str` fields).
    Str(String),
}

/// Map from field key to value — the input type for validation and serialization.
pub type Values = std::collections::HashMap<String, FieldValue>;

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
    fn player_schema_has_fileref_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        let field = schema.fields.iter()
            .find(|f| f.key == "Mesh Name")
            .expect("Mesh Name field");
        match &field.kind {
            FieldKind::FileRef { filter } => {
                assert!(filter.contains("*.iff"), "filter should include *.iff: {filter}");
            }
            other => panic!("expected FileRef, got {other:?}"),
        }
        assert!(field.file_filter.contains("*.iff"),
            "file_filter shortcut should contain *.iff: {}", field.file_filter);
    }

    #[test]
    fn parse_filter_extracts_patterns() {
        // Simulate: "World Foundry IFF (*.iff)\0*.iff\0Targa (*.tga)\0*.tga\0\0"
        let mut raw = b"World Foundry IFF (*.iff)\0*.iff\0Targa (*.tga)\0*.tga\0\0".to_vec();
        raw.resize(512, 0); // pad to OAD field size
        let result = super::parse_filter(&raw);
        assert_eq!(result, "*.iff;*.tga");
    }

    #[test]
    fn player_schema_has_section_field() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        // PropertySheet → Section
        let section = schema.fields.iter()
            .find(|f| matches!(f.kind, FieldKind::Section) && f.label == "Movement")
            .expect("Movement section");
        assert!(matches!(section.kind, FieldKind::Section));
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
    fn visible_fields_excludes_skip_and_hidden() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        for f in schema.visible_fields() {
            assert!(!matches!(f.kind, FieldKind::Skip),
                "visible_fields should not include Skip field: {}", f.key);
            assert_ne!(f.show_as, 6,
                "visible_fields should not include show_as=6 field: {}", f.key);
        }
    }

    #[test]
    fn show_as_6_fields_not_visible() {
        let oad = load_fixture("player.oad");
        let schema = from_oad(&oad);
        // slopeA–slopeD are BUTTON_FIXED32 with show_as=6 — hidden/internal
        let slope = schema.fields.iter().find(|f| f.key == "slopeA").expect("slopeA");
        assert_eq!(slope.show_as, 6);
        assert!(!slope.is_visible(), "slopeA (show_as=6) should not be visible");
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
