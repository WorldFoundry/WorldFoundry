"""
World Foundry property panel.

Rendered dynamically from the Rust-provided schema descriptors — no
hardcoded field layout.  One panel per object in Properties > Object.
"""

import bpy
import wf_core
from .operators import SCHEMA_PATH_KEY, _prop_key, _get_schema

# ── WF_PT_attributes ─────────────────────────────────────────────────────────

FIXED32_SCALE = 65536.0  # 2^16


class WF_PT_attributes(bpy.types.Panel):
    bl_label       = "World Foundry"
    bl_idname      = "WF_PT_attributes"
    bl_space_type  = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context     = 'object'

    def draw(self, context):
        layout = self.layout
        obj = context.active_object
        if obj is None:
            return

        schema_path = obj.get(SCHEMA_PATH_KEY)

        # ── header row: schema name + attach/detach buttons ──
        row = layout.row(align=True)
        if schema_path:
            try:
                schema = wf_core.load_schema(schema_path)
                row.label(text=schema.name, icon='OBJECT_DATA')
            except Exception as e:
                row.label(text=f"Schema error: {e}", icon='ERROR')
                row.operator("wf.attach_schema", text="", icon='FILE_FOLDER')
                return
        else:
            row.label(text="No schema attached", icon='QUESTION')

        row.operator("wf.attach_schema",  text="", icon='FILE_FOLDER')
        row.operator("wf.detach_schema",  text="", icon='X')

        if not schema_path:
            return

        # ── field rows ───────────────────────────────────────
        current_group = None
        box = None

        for field in schema.fields():
            if field.kind == "Skip":
                continue

            if field.kind == "Group":
                # Start a new collapsible section
                box = layout.box()
                row = box.row()
                row.label(text=field.label, icon='DOWNARROW_HLT')
                current_group = field.label
                continue

            # Use the box if we're inside a group, otherwise the root layout
            target = box if box is not None else layout

            prop_key = _prop_key(field.key)
            if prop_key not in obj:
                # Field not seeded yet (schema changed?) — show placeholder
                target.label(text=f"{field.label}: (not set)", icon='ERROR')
                continue

            self._draw_field(target, obj, field, prop_key)

        # ── footer: validate + export buttons ────────────────
        layout.separator()
        row = layout.row(align=True)
        row.operator("wf.validate",        text="Validate",      icon='CHECKMARK')
        row.operator("wf.export_iff_txt",  text="Export .iff.txt", icon='EXPORT')

    @staticmethod
    def _draw_field(layout, obj, field, prop_key):
        """Render one field row appropriate to its kind."""
        kind = field.kind

        if kind == "Int":
            layout.prop(obj, f'["{prop_key}"]', text=field.label)

        elif kind == "Float":
            # Show raw fixed-point as a float via a custom draw
            row = layout.row()
            row.label(text=field.label)
            raw = obj.get(prop_key, 0)
            # Display as float; editing updates raw via modal operator (future).
            # For now show read-only float alongside the raw int prop.
            row.label(text=f"{raw / FIXED32_SCALE:.4f}")
            row.prop(obj, f'["{prop_key}"]', text="raw")

        elif kind == "Enum":
            items = field.enum_items()
            idx = obj.get(prop_key, 0)
            label = items[idx] if 0 <= idx < len(items) else f"? ({idx})"
            row = layout.row()
            row.label(text=field.label)
            row.label(text=label)
            # TODO: replace with a proper EnumProperty once schema is stable
            row.prop(obj, f'["{prop_key}"]', text="idx")

        elif kind == "Str":
            layout.prop(obj, f'["{prop_key}"]', text=field.label)

        # Group handled by caller; Skip never reaches here.


# ── registration ──────────────────────────────────────────────────────────────

_CLASSES = [WF_PT_attributes]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
