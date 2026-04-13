"""
World Foundry operators:

  WF_OT_attach_schema   — file picker → load OAD, store path, seed defaults
  WF_OT_detach_schema   — remove WF custom properties from object
  WF_OT_set_enum        — set an enum field by item label (used by panel dropdown)
  WF_OT_validate        — range-check all values via Rust
  WF_OT_export_iff_txt  — gather values and write .iff.txt via Rust

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
from bpy_extras.io_utils import ExportHelper
import wf_core

# ── constants ─────────────────────────────────────────────────────────────────

SCHEMA_PATH_KEY = "wf_schema_path"


def _prop_key(field_key: str) -> str:
    """Custom property key for a field value."""
    return f"wf_{field_key}"


def _get_schema(obj):
    """Return a loaded wf_core.Schema or None."""
    path = obj.get(SCHEMA_PATH_KEY)
    if not path:
        return None
    try:
        return wf_core.load_schema(path)
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


def _seed_defaults(obj, schema):
    """Populate default values for all fields not yet set on obj."""
    for field in schema.visible_fields():
        key = _prop_key(field.key)
        if key in obj:
            continue
        if field.kind == "Float":
            obj[key] = field.default_display
        elif field.kind == "Enum":
            items = field.enum_items()
            idx = field.default_raw
            obj[key] = items[idx] if 0 <= idx < len(items) else (items[0] if items else "")
        elif field.kind == "Str":
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

        obj[SCHEMA_PATH_KEY] = self.filepath
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


# ── registration ──────────────────────────────────────────────────────────────

_CLASSES = [
    WF_OT_attach_schema,
    WF_OT_detach_schema,
    WF_OT_set_enum,
    WF_OT_validate,
    WF_OT_export_iff_txt,
]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
