"""
World Foundry property panel — dynamic, driven by Rust schema descriptors.
"""

import bpy
import wf_core
from .operators import SCHEMA_PATH_KEY, _prop_key, _get_schema, _seed_defaults


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

        # ── header ────────────────────────────────────────────
        row = layout.row(align=True)
        if schema_path:
            try:
                schema = wf_core.load_schema(schema_path)
            except Exception as e:
                row.label(text=f"Schema error: {e}", icon='ERROR')
                row.operator("wf.attach_schema", text="", icon='FILE_FOLDER')
                return
            row.label(text=schema.name, icon='OBJECT_DATA')
        else:
            row.label(text="No schema attached", icon='QUESTION')

        row.operator("wf.attach_schema", text="", icon='FILE_FOLDER')
        row.operator("wf.detach_schema", text="", icon='X')

        if not schema_path:
            return

        # Ensure any fields added since last attach have defaults
        _seed_defaults(obj, schema)

        # ── field rows ────────────────────────────────────────
        current_box = None
        current_group_name = None

        for field in schema.fields():
            if field.kind == "Skip":
                continue

            if field.kind == "Group":
                current_box = layout.box()
                current_box.label(text=field.label, icon='DOWNARROW_HLT')
                current_group_name = field.label
                continue

            container = current_box if current_box is not None else layout
            prop_key  = _prop_key(field.key)

            if prop_key not in obj:
                container.label(text=f"{field.label}: (not set)", icon='ERROR')
                continue

            self._draw_field(container, obj, field, prop_key)

        # ── footer ────────────────────────────────────────────
        layout.separator()
        row = layout.row(align=True)
        row.operator("wf.validate",       text="Validate",        icon='CHECKMARK')
        row.operator("wf.export_iff_txt", text="Export .iff.txt", icon='EXPORT')

    @staticmethod
    def _draw_field(layout, obj, field, prop_key):
        kind    = field.kind
        show_as = field.show_as

        if kind in ("Int", "Float", "Str"):
            layout.prop(obj, f'["{prop_key}"]', text=field.label)

        elif kind == "Enum":
            items   = field.enum_items()
            current = obj.get(prop_key, "")

            if show_as == 8 and len(items) == 2:
                # ShowAs=8 → checkbox-style toggle (False / True)
                is_true = (current == items[1])
                row = layout.row()
                row.label(text=field.label + ":")
                op = row.operator(
                    "wf.set_enum",
                    text=items[1] if is_true else items[0],
                    icon='CHECKBOX_HLT' if is_true else 'CHECKBOX_DEHLT',
                    depress=is_true,
                )
                op.field_key  = field.key
                op.item_label = items[0] if is_true else items[1]

            else:
                # ShowAs=4/5 → row of labelled buttons (highlight selected)
                row = layout.row(align=True)
                row.label(text=field.label + ":")
                sub = row.row(align=True)
                for item in items:
                    op = sub.operator(
                        "wf.set_enum",
                        text=item,
                        depress=(item == current),
                    )
                    op.field_key  = field.key
                    op.item_label = item


# ── registration ──────────────────────────────────────────────────────────────

_CLASSES = [WF_PT_attributes]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
