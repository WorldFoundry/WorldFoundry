"""
World Foundry property panel — dynamic, driven by Rust schema descriptors.

Section rendering
-----------------
PropertySheet → collapsible section (TRIA_DOWN / TRIA_RIGHT toggle).
               Fields up to the next PropertySheet are shown/hidden together.
GroupStart    → non-collapsible box within the current section.
GroupStop     → closes the GroupStart box.
"""

import os
import bpy
import wf_core
from .operators import (
    SCHEMA_PATH_KEY, SECTION_OPEN_PREFIX,
    _prop_key, _section_key, _get_schema, _seed_defaults,
    _resolve_schema_path,
)


class WF_PT_attributes(bpy.types.Panel):
    bl_label       = "World Foundry"
    bl_idname      = "WF_PT_attributes"
    bl_space_type  = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context     = 'object'

    def draw(self, context):
        layout = self.layout
        obj    = context.active_object
        if obj is None:
            return

        schema_path = obj.get(SCHEMA_PATH_KEY)

        # ── header row ────────────────────────────────────────
        row = layout.row(align=True)
        if schema_path:
            resolved = _resolve_schema_path(schema_path)
            try:
                schema = wf_core.load_schema(resolved)
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

        # Show the stored path so the user can verify it persisted correctly.
        path_row = layout.row()
        path_row.label(text=resolved, icon='FILE')

        # ── field loop ────────────────────────────────────────
        # State:
        #   section_open  — whether the current PropertySheet is expanded
        #   section_layout — the column inside the section box (or None = top-level)
        #   group_box      — inner box for a GroupStart (None when not inside one)

        section_open   = True   # fields before any PropertySheet are always shown
        section_layout = layout
        group_box      = None

        for field in schema.fields():
            kind = field.kind

            # ── Section (PropertySheet) — collapsible rollup ──
            if kind == "Section":
                sk      = _section_key(field.key)
                is_open = bool(obj.get(sk, True))

                # Always draw at the root layout, not inside any group box
                group_box = None

                box = layout.box()
                row = box.row(align=True)
                icon = 'TRIA_DOWN' if is_open else 'TRIA_RIGHT'
                op = row.operator(
                    "wf.toggle_section",
                    text=field.label,
                    icon=icon,
                    emboss=False,
                )
                op.section_key = sk

                section_open   = is_open
                section_layout = box.column() if is_open else None
                continue

            # ── GroupStart — non-collapsible sub-box ──────────
            if kind == "Group":
                if section_open and section_layout is not None:
                    group_box = section_layout.box()
                    group_box.label(text=field.label, icon='DOWNARROW_HLT')
                continue

            # ── GroupEnd — close sub-box ──────────────────────
            if kind == "GroupEnd":
                group_box = None
                continue

            # ── Skip / hidden fields ──────────────────────────
            if kind == "Skip" or field.show_as == 6:
                continue

            # If section is collapsed, don't draw any fields
            if not section_open or section_layout is None:
                continue

            # Choose target: inside group box or direct in section
            target    = group_box if group_box is not None else section_layout
            prop_key  = _prop_key(field.key)

            if prop_key not in obj:
                target.label(text=f"{field.label}: (not set)", icon='ERROR')
                continue

            self._draw_field(target, obj, field, prop_key)

        # ── footer ────────────────────────────────────────────
        layout.separator()
        row = layout.row(align=True)
        row.operator("wf.validate",       text="Validate",        icon='CHECKMARK')

        row = layout.row(align=True)
        row.operator("wf.export_iff_txt", text="Export .iff.txt", icon='EXPORT')
        row.operator("wf.import_iff_txt", text="Import .iff.txt", icon='IMPORT')

        row = layout.row(align=True)
        row.operator("wf.export_iff",     text="Export .iff",     icon='EXPORT')
        row.operator("wf.import_iff",     text="Import .iff",     icon='IMPORT')

    @staticmethod
    def _draw_field(layout, obj, field, prop_key):
        kind    = field.kind
        show_as = field.show_as

        if kind == "ObjRef":
            # Searchable object picker.  prop_search with a custom string ID
            # property lets the user type or select from all scene objects.
            row = layout.row(align=True)
            row.prop_search(obj, f'["{prop_key}"]', bpy.data, "objects",
                            text=field.label)
            # Warn if the named object doesn't exist in the scene.
            name = obj.get(prop_key, "")
            if name and name not in bpy.data.objects:
                row.label(text="", icon='ERROR')

        elif kind == "Bool":
            is_on = bool(obj.get(prop_key, 0))
            row = layout.row()
            row.label(text=field.label + ":")
            op = row.operator(
                "wf.toggle_bool",
                text="",
                icon='CHECKBOX_HLT' if is_on else 'CHECKBOX_DEHLT',
                depress=is_on,
            )
            op.field_key = field.key

        elif kind == "FileRef":
            # Text field + file-browser button.
            row = layout.row(align=True)
            row.prop(obj, f'["{prop_key}"]', text=field.label)
            op = row.operator("wf.pick_file", text="", icon='FILE_FOLDER')
            op.field_key   = field.key
            op.filter_glob = field.file_filter or "*.iff"

        elif kind in ("Int", "Float", "Str"):
            layout.prop(obj, f'["{prop_key}"]', text=field.label)

        elif kind == "Enum":
            items   = field.enum_items()
            current = obj.get(prop_key, "")

            if show_as == 8 and len(items) == 2:
                # Checkbox-style toggle: label left, icon button right
                is_true = (current == items[1])
                row = layout.row()
                row.label(text=field.label + ":")
                op = row.operator(
                    "wf.set_enum",
                    text="",
                    icon='CHECKBOX_HLT' if is_true else 'CHECKBOX_DEHLT',
                    depress=is_true,
                )
                op.field_key  = field.key
                op.item_label = items[0] if is_true else items[1]

            else:
                # Button row: one button per choice, highlight current
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
