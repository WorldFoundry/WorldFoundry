"""
World Foundry operators:

  WF_OT_attach_schema   — file picker → load OAD, store path, seed defaults
  WF_OT_detach_schema   — remove WF custom properties from object
  WF_OT_validate        — range-check all values via Rust
  WF_OT_export_iff_txt  — gather values and write .iff.txt via Rust
"""

import bpy
from bpy.props import StringProperty
from bpy_extras.io_utils import ExportHelper
import wf_core

# ── helpers ──────────────────────────────────────────────────────────────────

SCHEMA_PATH_KEY = "wf_schema_path"


def _prop_key(field_key: str) -> str:
    """Custom property key for a field value."""
    return f"wf_{field_key}"


def _get_schema(obj) -> "wf_core.Schema | None":
    path = obj.get(SCHEMA_PATH_KEY)
    if not path:
        return None
    try:
        return wf_core.load_schema(path)
    except Exception:
        return None


def _collect_values(obj, schema) -> dict:
    """Read all wf_ custom properties into a plain dict."""
    values = {}
    for field in schema.visible_fields():
        key = _prop_key(field.key)
        val = obj.get(key)
        if val is not None:
            values[field.key] = val
    return values


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

        # Store schema path
        obj[SCHEMA_PATH_KEY] = self.filepath

        # Seed defaults for all visible fields (only if not already set)
        for field in schema.visible_fields():
            key = _prop_key(field.key)
            if key not in obj:
                if field.kind == "Str":
                    obj[key] = ""
                else:
                    obj[key] = field.default_raw

        self.report({'INFO'}, f"Attached schema '{schema.name}' ({len(list(schema.visible_fields()))} fields)")
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
            self.report({'ERROR'}, "No schema attached (use 'Attach Schema' first)")
            return {'CANCELLED'}

        values = _collect_values(obj, schema)
        issues = wf_core.validate(schema, values)

        if not issues:
            self.report({'INFO'}, "Validation passed — no issues found")
        else:
            errors   = [i for i in issues if i.is_error]
            warnings = [i for i in issues if not i.is_error]
            for issue in errors:
                self.report({'ERROR'}, f"{issue.key}: {issue.message}")
            for issue in warnings:
                self.report({'WARNING'}, f"{issue.key}: {issue.message}")

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
            self.report({'ERROR'}, "No schema attached (use 'Attach Schema' first)")
            return {'CANCELLED'}

        # Validate before export
        values = _collect_values(obj, schema)
        issues = wf_core.validate(schema, values)
        errors = [i for i in issues if i.is_error]
        if errors:
            for e in errors:
                self.report({'ERROR'}, f"Validation error — {e.key}: {e.message}")
            self.report({'ERROR'}, "Fix validation errors before exporting")
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
    WF_OT_validate,
    WF_OT_export_iff_txt,
]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
