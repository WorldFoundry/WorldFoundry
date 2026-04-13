"""
World Foundry operators:

  WF_OT_attach_schema   — file picker → load OAD, store path, seed defaults
  WF_OT_detach_schema   — remove WF custom properties from object
  WF_OT_set_enum        — set an enum field by item label (used by panel dropdown)
  WF_OT_validate        — range-check all values via Rust
  WF_OT_export_iff_txt  — gather values and write .iff.txt via Rust
  WF_OT_import_iff_txt  — read .iff.txt and populate object properties
  WF_OT_export_iff      — gather values and write binary .iff via Rust
  WF_OT_import_iff      — read binary .iff and populate object properties

Storage convention
------------------
  obj["wf_schema_path"]   — absolute path to the .oad file
  obj["wf_<key>"]         — field value:
    Int   → int
    Float → float (display value, e.g. 50.0; NOT the raw fixed-point int)
    Enum  → string label (e.g. "Physics")
    Str   → string
"""

import bpy
from bpy.props import StringProperty
from bpy_extras.io_utils import ExportHelper, ImportHelper
import wf_core

# ── constants ─────────────────────────────────────────────────────────────────

SCHEMA_PATH_KEY = "wf_schema_path"


def _prop_key(field_key: str) -> str:
    """Custom property key for a field value."""
    return f"wf_{field_key}"


def _resolve_schema_path(raw: str) -> str:
    """Resolve a stored schema path (possibly //-relative) to an absolute path."""
    return bpy.path.abspath(raw)


def _get_schema(obj):
    """Return a loaded wf_core.Schema or None."""
    path = obj.get(SCHEMA_PATH_KEY)
    if not path:
        return None
    try:
        return wf_core.load_schema(_resolve_schema_path(path))
    except Exception:
        return None


def _collect_values(obj, schema) -> dict:
    """Read all wf_ custom properties into a plain dict (display values)."""
    values = {}
    for field in schema.visible_fields():
        key = _prop_key(field.key)
        val = obj.get(key)
        if val is not None:
            values[field.key] = val
    return values


SECTION_OPEN_PREFIX = "wf__open_"


def _section_key(field_key: str) -> str:
    """Custom property key tracking whether a Section is expanded."""
    return f"{SECTION_OPEN_PREFIX}{field_key}"


def _seed_defaults(obj, schema):
    """Populate default values for all fields not yet set on obj."""
    for field in schema.fields():
        # Section open/closed state (default_raw: 1=open, 0=closed)
        if field.kind == "Section":
            sk = _section_key(field.key)
            if sk not in obj:
                obj[sk] = bool(field.default_raw)
            continue

        # Structural markers — no value to seed
        if field.kind in ("Group", "GroupEnd", "Skip"):
            continue

        # Skip hidden fields
        if field.show_as == 6:
            continue

        key = _prop_key(field.key)
        if key in obj:
            continue
        if field.kind == "Float":
            obj[key] = field.default_display
        elif field.kind == "Enum":
            items = field.enum_items()
            idx = field.default_raw
            obj[key] = items[idx] if 0 <= idx < len(items) else (items[0] if items else "")
        elif field.kind in ("Str", "ObjRef", "FileRef"):
            obj[key] = ""
        else:
            obj[key] = field.default_raw

        # Register UI properties (soft min/max, description)
        ui = obj.id_properties_ui(key)
        if field.kind == "Float":
            ui.update(
                min=field.min_display,
                max=field.max_display,
                soft_min=field.min_display,
                soft_max=field.max_display,
                description=field.help or field.label,
            )
        elif field.kind == "Int":
            ui.update(
                min=int(field.min_raw),
                max=int(field.max_raw),
                soft_min=int(field.min_raw),
                soft_max=int(field.max_raw),
                description=field.help or field.label,
            )


# ── WF_OT_attach_schema ───────────────────────────────────────────────────────

class WF_OT_attach_schema(bpy.types.Operator):
    """Load an OAD schema and attach it to the active object"""
    bl_idname  = "wf.attach_schema"
    bl_label   = "Attach Schema"
    bl_options = {'REGISTER', 'UNDO'}

    filepath: StringProperty(subtype='FILE_PATH')
    filter_glob: StringProperty(default="*.oad", options={'HIDDEN'})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}

        try:
            schema = wf_core.load_schema(self.filepath)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to load schema: {e}")
            return {'CANCELLED'}

        # Store a //-relative path when the blend file has been saved so the
        # association survives the blend file moving to a new location.
        # Fall back to absolute when the blend file is still unsaved.
        stored_path = (
            bpy.path.relpath(self.filepath)
            if bpy.data.is_saved
            else self.filepath
        )
        obj[SCHEMA_PATH_KEY] = stored_path
        _seed_defaults(obj, schema)

        visible = list(schema.visible_fields())
        self.report({'INFO'}, f"Attached '{schema.name}' — {len(visible)} fields")
        return {'FINISHED'}


# ── WF_OT_detach_schema ───────────────────────────────────────────────────────

class WF_OT_detach_schema(bpy.types.Operator):
    """Remove all World Foundry custom properties from the active object"""
    bl_idname  = "wf.detach_schema"
    bl_label   = "Detach Schema"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        keys = [k for k in obj.keys() if k.startswith("wf_")]
        for k in keys:
            del obj[k]
        self.report({'INFO'}, f"Removed {len(keys)} World Foundry properties")
        return {'FINISHED'}


# ── WF_OT_toggle_section ─────────────────────────────────────────────────────

class WF_OT_toggle_section(bpy.types.Operator):
    """Expand or collapse a property-sheet section"""
    bl_idname  = "wf.toggle_section"
    bl_label   = "Toggle Section"
    bl_options = {'REGISTER', 'UNDO', 'INTERNAL'}

    section_key: StringProperty(options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            return {'CANCELLED'}
        current = bool(obj.get(self.section_key, True))
        obj[self.section_key] = not current
        for area in context.screen.areas:
            if area.type == 'PROPERTIES':
                area.tag_redraw()
        return {'FINISHED'}


# ── WF_OT_set_enum ────────────────────────────────────────────────────────────

class WF_OT_set_enum(bpy.types.Operator):
    """Set an enum field to a specific item label"""
    bl_idname  = "wf.set_enum"
    bl_label   = "Set Enum Value"
    bl_options = {'REGISTER', 'UNDO', 'INTERNAL'}

    field_key:  StringProperty(options={'HIDDEN'})
    item_label: StringProperty(options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            return {'CANCELLED'}
        obj[_prop_key(self.field_key)] = self.item_label
        # Force panel redraw
        for area in context.screen.areas:
            if area.type == 'PROPERTIES':
                area.tag_redraw()
        return {'FINISHED'}


# ── WF_OT_validate ────────────────────────────────────────────────────────────

class WF_OT_validate(bpy.types.Operator):
    """Validate attribute values against the attached schema"""
    bl_idname  = "wf.validate"
    bl_label   = "Validate"
    bl_options = {'REGISTER'}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        schema = _get_schema(obj)
        if schema is None:
            self.report({'ERROR'}, "No schema attached")
            return {'CANCELLED'}

        values = _collect_values(obj, schema)
        issues = wf_core.validate(schema, values)

        if not issues:
            self.report({'INFO'}, "Validation passed — no issues")
        else:
            for issue in issues:
                level = 'ERROR' if issue.is_error else 'WARNING'
                self.report({level}, f"{issue.key}: {issue.message}")

        return {'FINISHED'}


# ── WF_OT_export_iff_txt ──────────────────────────────────────────────────────

class WF_OT_export_iff_txt(bpy.types.Operator, ExportHelper):
    """Export object attributes as .iff.txt"""
    bl_idname  = "wf.export_iff_txt"
    bl_label   = "Export .iff.txt"
    bl_options = {'REGISTER'}

    filename_ext = ".iff.txt"
    filter_glob: StringProperty(default="*.iff.txt", options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        schema = _get_schema(obj)
        if schema is None:
            self.report({'ERROR'}, "No schema attached")
            return {'CANCELLED'}

        values = _collect_values(obj, schema)

        # Reject on validation errors
        issues = wf_core.validate(schema, values)
        errors = [i for i in issues if i.is_error]
        if errors:
            for e in errors:
                self.report({'ERROR'}, f"{e.key}: {e.message}")
            return {'CANCELLED'}

        text = wf_core.export_iff_txt(schema, values)
        with open(self.filepath, 'w', encoding='utf-8') as f:
            f.write(text)
        self.report({'INFO'}, f"Exported to {self.filepath}")
        return {'FINISHED'}


# ── WF_OT_import_iff_txt ─────────────────────────────────────────────────────

class WF_OT_import_iff_txt(bpy.types.Operator, ImportHelper):
    """Import attribute values from a .iff.txt file into this object"""
    bl_idname  = "wf.import_iff_txt"
    bl_label   = "Import .iff.txt"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".iff.txt"
    filter_glob: StringProperty(default="*.iff.txt", options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        schema = _get_schema(obj)
        if schema is None:
            self.report({'ERROR'}, "No schema attached — attach a schema first")
            return {'CANCELLED'}

        try:
            with open(self.filepath, 'r', encoding='utf-8') as f:
                text = f.read()
        except OSError as e:
            self.report({'ERROR'}, f"Could not read file: {e}")
            return {'CANCELLED'}

        try:
            imported = wf_core.import_iff_txt(schema, text)
        except Exception as e:
            self.report({'ERROR'}, f"Parse error: {e}")
            return {'CANCELLED'}

        # Seed defaults first so every field exists, then overwrite with
        # imported values.
        _seed_defaults(obj, schema)
        count = 0
        for key, val in imported.items():
            prop_key = _prop_key(key)
            if prop_key in obj:
                obj[prop_key] = val
                count += 1

        self.report({'INFO'}, f"Imported {count} fields from {self.filepath}")
        return {'FINISHED'}


# ── WF_OT_export_iff ─────────────────────────────────────────────────────────

class WF_OT_export_iff(bpy.types.Operator, ExportHelper):
    """Export object attributes as binary .iff"""
    bl_idname  = "wf.export_iff"
    bl_label   = "Export .iff"
    bl_options = {'REGISTER'}

    filename_ext = ".iff"
    filter_glob: StringProperty(default="*.iff", options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        schema = _get_schema(obj)
        if schema is None:
            self.report({'ERROR'}, "No schema attached")
            return {'CANCELLED'}

        values = _collect_values(obj, schema)

        # Reject on validation errors
        issues = wf_core.validate(schema, values)
        errors = [i for i in issues if i.is_error]
        if errors:
            for e in errors:
                self.report({'ERROR'}, f"{e.key}: {e.message}")
            return {'CANCELLED'}

        data = wf_core.export_iff(schema, values)
        with open(self.filepath, 'wb') as f:
            f.write(data)
        self.report({'INFO'}, f"Exported {len(data)} bytes to {self.filepath}")
        return {'FINISHED'}


# ── WF_OT_import_iff ─────────────────────────────────────────────────────────

class WF_OT_import_iff(bpy.types.Operator, ImportHelper):
    """Import attribute values from a binary .iff file into this object"""
    bl_idname  = "wf.import_iff"
    bl_label   = "Import .iff"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".iff"
    filter_glob: StringProperty(default="*.iff", options={'HIDDEN'})

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "No active object")
            return {'CANCELLED'}
        schema = _get_schema(obj)
        if schema is None:
            self.report({'ERROR'}, "No schema attached — attach a schema first")
            return {'CANCELLED'}

        try:
            with open(self.filepath, 'rb') as f:
                data = f.read()
        except OSError as e:
            self.report({'ERROR'}, f"Could not read file: {e}")
            return {'CANCELLED'}

        try:
            imported = wf_core.import_iff(schema, bytes(data))
        except Exception as e:
            self.report({'ERROR'}, f"Parse error: {e}")
            return {'CANCELLED'}

        # Seed defaults first so every field exists, then overwrite with
        # imported values.
        _seed_defaults(obj, schema)
        count = 0
        for key, val in imported.items():
            prop_key = _prop_key(key)
            if prop_key in obj:
                obj[prop_key] = val
                count += 1

        self.report({'INFO'}, f"Imported {count} fields from {self.filepath}")
        return {'FINISHED'}


# ── WF_OT_pick_file ──────────────────────────────────────────────────────────

class WF_OT_pick_file(bpy.types.Operator):
    """Browse for a file and store its path in a FileRef field"""
    bl_idname  = "wf.pick_file"
    bl_label   = "Pick File"
    bl_options = {'REGISTER', 'UNDO', 'INTERNAL'}

    field_key:   StringProperty(options={'HIDDEN'})
    filter_glob: StringProperty(default="*.iff", options={'HIDDEN'})
    filepath:    StringProperty(subtype='FILE_PATH')

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def execute(self, context):
        obj = context.active_object
        if obj is None:
            return {'CANCELLED'}
        prop_key = _prop_key(self.field_key)
        # Store //-relative path when the blend file is saved.
        stored = (
            bpy.path.relpath(self.filepath)
            if bpy.data.is_saved else self.filepath
        )
        obj[prop_key] = stored
        for area in context.screen.areas:
            if area.type == 'PROPERTIES':
                area.tag_redraw()
        return {'FINISHED'}


# ── registration ──────────────────────────────────────────────────────────────

_CLASSES = [
    WF_OT_attach_schema,
    WF_OT_detach_schema,
    WF_OT_toggle_section,
    WF_OT_set_enum,
    WF_OT_pick_file,
    WF_OT_validate,
    WF_OT_export_iff_txt,
    WF_OT_import_iff_txt,
    WF_OT_export_iff,
    WF_OT_import_iff,
]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
