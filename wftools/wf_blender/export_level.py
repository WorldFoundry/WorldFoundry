"""
World Foundry level import / export operators.

  WF_OT_import_level  — read a .lev text IFF → populate Blender scene
  WF_OT_export_level  — write Blender scene → .lev text IFF

.lev format (text IFF):

  { 'LVL'
      { 'OBJ'
          { 'NAME' "objname" }
          { 'VEC3' { 'NAME' "Position" }             { 'DATA' x y z } }
          { 'EULR' { 'NAME' "Orientation" }           { 'DATA' a b c } }
          { 'BOX3' { 'NAME' "Global Bounding Box" }   { 'DATA' x0 y0 z0 x1 y1 z1 } }
          { 'STR'  { 'NAME' "Class Name" }            { 'DATA' "typename" } }
          { 'I32'  { 'NAME' "field" } { 'DATA' 0l }  { 'STR' "0" } }
          { 'FX32' { 'NAME' "field" } { 'DATA' 0.0(1.15.16) } { 'STR' "0.0" } }
          { 'STR'  { 'NAME' "field" } { 'DATA' "val" } }
      }
      ...
  }

Coordinate transform (Blender Z-up ↔ WF Y-up):
  export: wf_x = bl_x,  wf_y = bl_z,  wf_z = -bl_y
  import: bl_x = wf_x,  bl_y = -wf_z, bl_z = wf_y
"""

import math
import os
import re
import struct

import bpy
import bmesh
from bpy.props import StringProperty
from bpy_extras.io_utils import ExportHelper, ImportHelper

import wf_core

# ── helpers shared with operators.py ─────────────────────────────────────────

SCHEMA_PATH_KEY = "wf_schema_path"


def _prop_key(field_key: str) -> str:
    return f"wf_{field_key}"


def _seed_defaults(obj, schema):
    for field in schema.fields():
        if field.kind == "Section":
            sk = f"wf__open_{field.key}"
            if sk not in obj:
                obj[sk] = bool(field.default_raw)
            continue
        if field.kind in ("Group", "GroupEnd", "Skip", "Annotation"):
            continue
        if field.show_as == 6:
            continue
        key = _prop_key(field.key)
        if key in obj:
            continue
        if field.kind == "Float":
            obj[key] = field.default_display
        elif field.kind == "Bool":
            obj[key] = int(bool(field.default_raw))
        elif field.kind == "Enum":
            items = field.enum_items()
            idx = field.default_raw
            obj[key] = items[idx] if 0 <= idx < len(items) else (items[0] if items else "")
        elif field.kind in ("Str", "ObjRef", "FileRef"):
            obj[key] = ""
        else:
            obj[key] = field.default_raw


def _collect_values(obj, schema) -> dict:
    values = {}
    for field in schema.visible_fields():
        key = _prop_key(field.key)
        val = obj.get(key)
        if val is not None:
            values[field.key] = val
    return values


# ── text IFF parser ───────────────────────────────────────────────────────────

def _strip_line_comments(text: str) -> str:
    """Remove // comments that are not inside quoted strings."""
    result = []
    for line in text.splitlines():
        out = []
        in_str = False
        i = 0
        while i < len(line):
            c = line[i]
            if c == '"':
                in_str = not in_str
                out.append(c)
            elif not in_str and line[i:i+2] == '//':
                break
            else:
                out.append(c)
            i += 1
        result.append(''.join(out))
    return '\n'.join(result)


def _tokenize(text: str):
    """Yield tokens: '{', '}', ('str', value), ('tag', value), ('num', value), ('word', value)."""
    text = _strip_line_comments(text)
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c in ' \t\n\r':
            i += 1
        elif c == '{':
            yield ('{',)
            i += 1
        elif c == '}':
            yield ('}',)
            i += 1
        elif c == "'":
            # FOURCC tag: 'ABCD'
            j = i + 1
            while j < n and text[j] != "'":
                j += 1
            yield ('tag', text[i+1:j])
            i = j + 1
        elif c == '"':
            # Quoted string
            j = i + 1
            buf = []
            while j < n and text[j] != '"':
                if text[j] == '\\' and j + 1 < n:
                    buf.append(text[j+1])
                    j += 2
                else:
                    buf.append(text[j])
                    j += 1
            yield ('str', ''.join(buf))
            i = j + 1
        elif c in '-0123456789.' or (c == '-' and i + 1 < n and text[i+1].isdigit()):
            # Number (possibly with (1.15.16) suffix and/or trailing 'l')
            j = i
            while j < n and (text[j] in '-0123456789.eE+' or
                              (text[j] == '(' and '.' in text[j:j+10])):
                if text[j] == '(':
                    k = text.find(')', j)
                    j = k + 1 if k >= 0 else j + 1
                else:
                    j += 1
            # eat optional trailing 'l'
            if j < n and text[j] == 'l':
                j += 1
            raw = text[i:j]
            # strip (1.15.16) suffix to get the float
            raw_clean = re.sub(r'\([^)]*\)', '', raw).rstrip('l').strip()
            try:
                yield ('num', float(raw_clean))
            except ValueError:
                yield ('word', raw)
            i = j
        elif c.isalpha() or c == '_':
            j = i
            while j < n and (text[j].isalnum() or text[j] in '_'):
                j += 1
            yield ('word', text[i:j])
            i = j
        else:
            i += 1


class _Parser:
    def __init__(self, tokens):
        self._toks = list(tokens)
        self._pos = 0

    def _peek(self):
        return self._toks[self._pos] if self._pos < len(self._toks) else None

    def _next(self):
        t = self._toks[self._pos]
        self._pos += 1
        return t

    def parse_chunk(self):
        """Parse { 'TAG' ... } → {'tag': str, 'children': list, 'scalars': list}"""
        t = self._next()
        assert t == ('{',), f"expected {{ got {t}"
        tag_tok = self._next()
        assert tag_tok[0] == 'tag', f"expected tag, got {tag_tok}"
        tag = tag_tok[1]
        children = []
        scalars = []
        while True:
            p = self._peek()
            if p is None or p == ('}',):
                break
            if p == ('{',):
                children.append(self.parse_chunk())
            elif p[0] in ('str', 'num', 'word'):
                scalars.append(self._next())
            else:
                self._next()  # skip unexpected
        close = self._next()
        assert close == ('}',), f"expected }} got {close}"
        return {'tag': tag, 'children': children, 'scalars': scalars}

    def parse_top(self):
        chunks = []
        while self._peek() is not None:
            if self._peek() == ('{',):
                chunks.append(self.parse_chunk())
            else:
                self._next()
        return chunks


def _parse_lev(text: str):
    tokens = _tokenize(text)
    parser = _Parser(tokens)
    top = parser.parse_top()
    # find LVL chunk
    for chunk in top:
        if chunk['tag'].strip() == 'LVL':
            return chunk
    return top[0] if top else None


def _child_by_tag(chunk, tag):
    for c in chunk['children']:
        if c['tag'].strip() == tag:
            return c
    return None


def _children_by_tag(chunk, tag):
    return [c for c in chunk['children'] if c['tag'].strip() == tag]


def _data_scalars(chunk):
    data = _child_by_tag(chunk, 'DATA')
    if data:
        return [t[1] for t in data['scalars'] if t[0] in ('num', 'str', 'word')]
    return []


def _name_value(chunk):
    """Return the DATA or STR content of a named chunk."""
    name_chunk = _child_by_tag(chunk, 'NAME')
    if name_chunk is None:
        return None, None
    name = name_chunk['scalars'][0][1] if name_chunk['scalars'] else ''
    data = _data_scalars(chunk)
    if not data:
        # some STR chunks use a nested STR instead of DATA
        str_chunk = _child_by_tag(chunk, 'STR')
        if str_chunk and str_chunk['scalars']:
            data = [str_chunk['scalars'][0][1]]
        elif str_chunk and str_chunk.get('children'):
            pass
    return name, data


# ── coordinate transform ──────────────────────────────────────────────────────

def wf_to_bl(wf_x, wf_y, wf_z):
    """WF Y-up → Blender Z-up."""
    return (wf_x, -wf_z, wf_y)


def bl_to_wf(bl_x, bl_y, bl_z):
    """Blender Z-up → WF Y-up."""
    return (bl_x, bl_z, -bl_y)


# ── binary mesh reader ────────────────────────────────────────────────────────
#
# Parses a MODL .iff file and returns (vertices, faces, uvs).
#
# On-disk structs (all little-endian):
#   Vertex3DOnDisk  [24 bytes]: fixed32 u, v;  uint32 color;  fixed32 x, y, z
#   _TriFaceOnDisk  [ 8 bytes]: int16 v1, v2, v3, materialIndex
#
# fixed32 is 1.15.16 — divide raw int32 by 65536.0 to get float.

_FIXED32_SCALE = 1.0 / 65536.0
_VERTEX_SIZE   = 24   # sizeof(Vertex3DOnDisk)
_FACE_SIZE     =  8   # sizeof(_TriFaceOnDisk)


def _read_iff_chunks(data: bytes) -> dict:
    """Parse flat IFF children from a payload blob → {tag: bytes}."""
    chunks = {}
    off = 0
    while off + 8 <= len(data):
        tag  = data[off:off+4].decode('ascii', errors='replace').rstrip('\x00')
        size = struct.unpack_from('<I', data, off + 4)[0]
        off += 8
        payload = data[off:off + size]
        chunks[tag] = payload
        off += size
        if size % 4:
            off += 4 - (size % 4)
    return chunks


def _load_mesh_iff(filepath: str):
    """
    Load a World Foundry MODL .iff and return (verts, faces, uvs) ready for Blender.

    verts — list of (x, y, z) in Blender space (Z-up)
    faces — list of (v1, v2, v3) int indices
    uvs   — list of (u, v) per vertex (parallel to verts)
    Returns None if the file cannot be parsed.
    """
    try:
        data = open(filepath, 'rb').read()
    except OSError:
        return None

    if len(data) < 8:
        return None

    # Outer MODL wrapper
    outer_tag  = data[0:4].decode('ascii', errors='replace').rstrip('\x00')
    outer_size = struct.unpack_from('<I', data, 4)[0]
    if outer_tag != 'MODL':
        return None

    inner = data[8:8 + outer_size]
    chunks = _read_iff_chunks(inner)

    vrtx_data = chunks.get('VRTX', b'')
    face_data  = chunks.get('FACE', b'')

    if not vrtx_data or not face_data:
        return None

    n_verts = len(vrtx_data) // _VERTEX_SIZE
    n_faces = len(face_data) // _FACE_SIZE

    verts = []
    uvs   = []
    for i in range(n_verts):
        base = i * _VERTEX_SIZE
        u_raw, v_raw = struct.unpack_from('<ii', vrtx_data, base)
        # skip color (4 bytes at base+8)
        x_raw, y_raw, z_raw = struct.unpack_from('<iii', vrtx_data, base + 12)
        wf_x = x_raw * _FIXED32_SCALE
        wf_y = y_raw * _FIXED32_SCALE
        wf_z = z_raw * _FIXED32_SCALE
        # WF Y-up → Blender Z-up
        verts.append(wf_to_bl(wf_x, wf_y, wf_z))
        uvs.append((u_raw * _FIXED32_SCALE, v_raw * _FIXED32_SCALE))

    faces = []
    for i in range(n_faces):
        base = i * _FACE_SIZE
        v1, v2, v3, _mat = struct.unpack_from('<hhhh', face_data, base)
        faces.append((v1, v2, v3))

    return verts, faces, uvs


def _find_file_nocase(directory: str, filename: str) -> str | None:
    """Return the actual path for filename in directory, case-insensitively."""
    lower = filename.lower()
    try:
        for entry in os.listdir(directory):
            if entry.lower() == lower:
                return os.path.join(directory, entry)
    except OSError:
        pass
    return None


# ── OAD dir helpers ───────────────────────────────────────────────────────────

def _default_oad_dirs() -> list[str]:
    """Candidate OAD directories to search if none is explicitly set."""
    # TODO: replace this hardcoded path with an addon preference once Blender
    # allows opening a directory picker while a file-selector dialog is already
    # open (currently raises "Cannot activate a file selector dialog, one
    # already open").
    _HARDCODED = "/home/will/SRC/WorldFoundry/wftools/wf_oad/tests/fixtures"
    candidates = [_HARDCODED] if os.path.isdir(_HARDCODED) else []

    # Walk up from this file's location to find wf_oad/tests/fixtures/
    here = os.path.dirname(os.path.abspath(__file__))
    d = here
    for _ in range(6):
        fixtures = os.path.join(d, "wf_oad", "tests", "fixtures")
        if os.path.isdir(fixtures) and fixtures not in candidates:
            candidates.append(fixtures)
        oad = os.path.join(d, "levels.src", "oad")
        if os.path.isdir(oad) and oad not in candidates:
            candidates.append(oad)
        d = os.path.dirname(d)
    return candidates


def _find_oad(typename: str, oad_dir: str) -> str | None:
    """Return path to <typename>.oad, searching oad_dir then default locations."""
    lower = typename.lower()
    dirs = []
    if oad_dir and os.path.isdir(oad_dir):
        dirs.append(oad_dir)
    dirs.extend(_default_oad_dirs())
    for d in dirs:
        for fname in os.listdir(d):
            if fname.lower() == lower + '.oad':
                return os.path.join(d, fname)
    return None


# ── import operator ───────────────────────────────────────────────────────────

class WF_OT_import_level(bpy.types.Operator, ImportHelper):
    """Import a World Foundry .lev text IFF into the Blender scene"""
    bl_idname  = "wf.import_level"
    bl_label   = "Import WF Level (.lev)"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".lev"
    filter_glob: StringProperty(default="*.lev", options={'HIDDEN'})

    oad_dir: StringProperty(
        name="OAD Directory",
        description="Directory containing .oad schema files",
        default="",
        subtype='DIR_PATH',
    )

    def invoke(self, context, event):
        # Pre-fill oad_dir from scene if set
        scene_oad = context.scene.get("wf_oad_dir", "")
        if scene_oad and not self.oad_dir:
            self.oad_dir = scene_oad
        return super().invoke(context, event)

    def execute(self, context):
        try:
            with open(self.filepath, 'r', encoding='utf-8', errors='replace') as f:
                text = f.read()
        except OSError as e:
            self.report({'ERROR'}, f"Cannot read file: {e}")
            return {'CANCELLED'}

        oad_dir = bpy.path.abspath(self.oad_dir) if self.oad_dir else ""

        # Save oad_dir to scene for next time
        if oad_dir:
            context.scene["wf_oad_dir"] = self.oad_dir

        lvl = _parse_lev(text)
        if lvl is None:
            self.report({'ERROR'}, "No LVL chunk found in file")
            return {'CANCELLED'}

        imported = 0
        skipped = 0

        for obj_chunk in _children_by_tag(lvl, 'OBJ'):
            name_chunk = _child_by_tag(obj_chunk, 'NAME')
            obj_name = name_chunk['scalars'][0][1] if (name_chunk and name_chunk['scalars']) else "wf_obj"

            # Position (VEC3)
            pos_wf = (0.0, 0.0, 0.0)
            vec3 = _child_by_tag(obj_chunk, 'VEC3')
            if vec3:
                d = _data_scalars(vec3)
                if len(d) >= 3:
                    pos_wf = (float(d[0]), float(d[1]), float(d[2]))

            # Bounding box (BOX3)
            bb_min_wf = (-0.5, -0.5, -0.5)
            bb_max_wf = ( 0.5,  0.5,  0.5)
            box3 = _child_by_tag(obj_chunk, 'BOX3')
            if box3:
                d = _data_scalars(box3)
                if len(d) >= 6:
                    bb_min_wf = (float(d[0]), float(d[1]), float(d[2]))
                    bb_max_wf = (float(d[3]), float(d[4]), float(d[5]))

            # Class Name (STR with NAME "Class Name")
            typename = None
            for str_chunk in _children_by_tag(obj_chunk, 'STR'):
                n, d = _name_value(str_chunk)
                if n == "Class Name" and d:
                    typename = str(d[0])
                    break

            if typename is None:
                skipped += 1
                continue

            # Mesh Name field — look for a non-empty FILE/STR chunk named "Mesh Name"
            mesh_name = ""
            for chunk in obj_chunk['children']:
                tag = chunk['tag'].strip()
                if tag not in ('FILE', 'STR'):
                    continue
                nc = _child_by_tag(chunk, 'NAME')
                if nc and nc['scalars'] and nc['scalars'][0][1] == 'Mesh Name':
                    sc = _child_by_tag(chunk, 'STR')
                    if sc and sc['scalars']:
                        mesh_name = str(sc['scalars'][0][1]).strip()
                    break

            # Try to load actual mesh geometry (case-insensitive filename match)
            mesh_geo = None
            if mesh_name:
                level_dir = os.path.dirname(self.filepath)
                mesh_path = _find_file_nocase(level_dir, mesh_name)
                if mesh_path:
                    mesh_geo = _load_mesh_iff(mesh_path)

            # Derive Blender location from BB centre
            cx_wf = (bb_min_wf[0] + bb_max_wf[0]) * 0.5
            cy_wf = (bb_min_wf[1] + bb_max_wf[1]) * 0.5
            cz_wf = (bb_min_wf[2] + bb_max_wf[2]) * 0.5
            bl_loc = wf_to_bl(cx_wf, cy_wf, cz_wf)

            if mesh_geo:
                # Real mesh geometry — vertices already in Blender space
                verts, faces, uvs = mesh_geo
                mesh = bpy.data.meshes.new(obj_name)
                mesh.from_pydata(verts, [], faces)
                mesh.update()

                # UV layer
                if uvs:
                    uv_layer = mesh.uv_layers.new(name="UVMap")
                    for poly in mesh.polygons:
                        for li in poly.loop_indices:
                            vi = mesh.loops[li].vertex_index
                            uv_layer.data[li].uv = uvs[vi]

                blobj = bpy.data.objects.new(obj_name, mesh)
                blobj.location = bl_loc
            else:
                # Fallback: box sized to bounding box
                sx = bb_max_wf[0] - bb_min_wf[0]  # WF X → Blender X
                sy = bb_max_wf[2] - bb_min_wf[2]  # WF Z → Blender Y
                sz = bb_max_wf[1] - bb_min_wf[1]  # WF Y → Blender Z
                sx = max(sx, 0.01)
                sy = max(sy, 0.01)
                sz = max(sz, 0.01)

                mesh = bpy.data.meshes.new(obj_name)
                bm = bmesh.new()
                bmesh.ops.create_cube(bm, size=1.0)
                bm.to_mesh(mesh)
                bm.free()

                blobj = bpy.data.objects.new(obj_name, mesh)
                blobj.location = bl_loc
                blobj.scale = (sx, sy, sz)

            context.collection.objects.link(blobj)

            # Attach OAD schema
            oad_path = _find_oad(typename, oad_dir)
            if oad_path:
                blobj[SCHEMA_PATH_KEY] = oad_path
                try:
                    schema = wf_core.load_schema(oad_path)
                    _seed_defaults(blobj, schema)
                    # Populate field values from remaining chunks
                    _apply_field_chunks(blobj, schema, obj_chunk)
                except Exception as e:
                    self.report({'WARNING'}, f"{obj_name}: schema error: {e}")
            else:
                self.report({'WARNING'}, f"{obj_name}: no .oad for '{typename}'")

            imported += 1

        self.report({'INFO'}, f"Imported {imported} objects ({skipped} skipped)")
        return {'FINISHED'}


def _apply_field_chunks(blobj, schema, obj_chunk):
    """Populate wf_ custom properties from field chunks in an OBJ."""
    # Build a map from field name → field for quick lookup
    field_by_name = {}
    for field in schema.fields():
        field_by_name[field.label.lower()] = field
        field_by_name[field.key.lower()] = field

    skip_names = {"position", "orientation", "global bounding box", "class name"}

    for chunk in obj_chunk['children']:
        tag = chunk['tag'].strip()
        if tag not in ('I32', 'FX32', 'STR', 'F32'):
            continue
        name_chunk = _child_by_tag(chunk, 'NAME')
        if not name_chunk or not name_chunk['scalars']:
            continue
        field_name = str(name_chunk['scalars'][0][1])
        if field_name.lower() in skip_names:
            continue

        field = field_by_name.get(field_name.lower())
        if field is None:
            continue

        prop_key = _prop_key(field.key)
        data = _data_scalars(chunk)
        if not data:
            continue
        val = data[0]

        if field.kind == "Int" or field.kind == "Bool":
            try:
                blobj[prop_key] = int(float(val))
            except (ValueError, TypeError):
                pass
        elif field.kind == "Float":
            try:
                blobj[prop_key] = float(val)
            except (ValueError, TypeError):
                pass
        elif field.kind == "Enum":
            try:
                idx = int(float(val))
                items = field.enum_items()
                if 0 <= idx < len(items):
                    blobj[prop_key] = items[idx]
            except (ValueError, TypeError):
                pass
        elif field.kind in ("Str", "ObjRef", "FileRef"):
            blobj[prop_key] = str(val)


# ── export operator ───────────────────────────────────────────────────────────

class WF_OT_export_level(bpy.types.Operator, ExportHelper):
    """Export the Blender scene as a World Foundry .lev text IFF"""
    bl_idname  = "wf.export_level"
    bl_label   = "Export WF Level (.lev)"
    bl_options = {'REGISTER'}

    filename_ext = ".lev"
    filter_glob: StringProperty(default="*.lev", options={'HIDDEN'})

    def execute(self, context):
        objects = [o for o in context.scene.objects if o.get(SCHEMA_PATH_KEY)]
        if not objects:
            self.report({'WARNING'}, "No WF objects in scene (attach schemas first)")
            return {'CANCELLED'}

        lines = ["{ 'LVL' "]
        for obj in objects:
            schema_path = obj[SCHEMA_PATH_KEY]
            # derive type name: strip directory, extension
            typename = os.path.splitext(os.path.basename(bpy.path.abspath(schema_path)))[0]

            # Bounding box in world space (Blender coords)
            corners_bl = [obj.matrix_world @ obj.bound_box[i].to_4d().to_3d()
                          if hasattr(obj.bound_box[0], 'to_4d')
                          else _corner(obj, i)
                          for i in range(8)]

            bl_min = [min(c[i] for c in corners_bl) for i in range(3)]
            bl_max = [max(c[i] for c in corners_bl) for i in range(3)]

            # Transform bbox corners to WF space
            wf_min_x, wf_min_y_raw, wf_min_z = bl_to_wf(bl_min[0], bl_min[1], bl_min[2])
            wf_max_x, wf_max_y_raw, wf_max_z = bl_to_wf(bl_max[0], bl_max[1], bl_max[2])
            # After transform, Y and Z axes may be swapped/negated — take real min/max
            wf_bb_min = (
                min(wf_min_x, wf_max_x),
                min(wf_min_y_raw, wf_max_y_raw),
                min(wf_min_z, wf_max_z),
            )
            wf_bb_max = (
                max(wf_min_x, wf_max_x),
                max(wf_min_y_raw, wf_max_y_raw),
                max(wf_min_z, wf_max_z),
            )

            # Object position (world-space origin → WF)
            loc = obj.matrix_world.to_translation()
            wf_pos = bl_to_wf(loc.x, loc.y, loc.z)

            # Orientation: Euler from world rotation (ZXY ≈ WF a,b,c)
            rot = obj.matrix_world.to_euler('XYZ')
            wf_rot = (rot.x, rot.z, -rot.y)

            def fp(v):
                return f"{v:.16f}(1.15.16)"

            lines.append("\t{ 'OBJ' ")
            lines.append(f'\t\t{{ \'NAME\' "{obj.name}" }}')
            lines.append(f"\t\t{{ 'VEC3' {{ 'NAME' \"Position\" }} {{ 'DATA' {fp(wf_pos[0])} {fp(wf_pos[1])} {fp(wf_pos[2])}  //x,y,z }} }}")
            lines.append(f"\t\t{{ 'EULR' {{ 'NAME' \"Orientation\" }} {{ 'DATA' {fp(wf_rot[0])} {fp(wf_rot[1])} {fp(wf_rot[2])}  //a,b,c }} }}")
            lines.append(
                f"\t\t{{ 'BOX3' {{ 'NAME' \"Global Bounding Box\" }} "
                f"{{ 'DATA' {fp(wf_bb_min[0])} {fp(wf_bb_min[1])} {fp(wf_bb_min[2])} "
                f"{fp(wf_bb_max[0])} {fp(wf_bb_max[1])} {fp(wf_bb_max[2])}  //min(x,y,z)-max(x,y,z) }} }}"
            )
            lines.append(f'\t\t{{ \'STR\' {{ \'NAME\' "Class Name" }} {{ \'DATA\' "{typename}" }} }}')

            # OAD fields
            try:
                schema = wf_core.load_schema(bpy.path.abspath(schema_path))
                values = _collect_values(obj, schema)
                oad_iff = wf_core.export_iff_txt(schema, values)
                # extract the inner field chunks from the OAD iff.txt
                # (it's wrapped in { 'TYPE' { 'NAME' ... } ... })
                for field_line in _extract_field_lines(oad_iff):
                    lines.append("\t\t" + field_line)
            except Exception as e:
                self.report({'WARNING'}, f"{obj.name}: OAD export error: {e}")

            lines.append("\t}")

        lines.append("}")
        text = "\n".join(lines) + "\n"

        try:
            with open(self.filepath, 'w', encoding='utf-8') as f:
                f.write(text)
        except OSError as e:
            self.report({'ERROR'}, f"Cannot write file: {e}")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Exported {len(objects)} objects to {self.filepath}")
        return {'FINISHED'}


def _corner(obj, i):
    """Return world-space corner i of obj.bound_box."""
    from mathutils import Vector
    local = Vector(obj.bound_box[i])
    return obj.matrix_world @ local


def _extract_field_lines(oad_iff_txt: str) -> list[str]:
    """
    Pull the inner field chunk lines out of a TYPE-wrapped OAD iff.txt block.
    Strips the outer { 'TYPE' ... } wrapper and the NAME chunk.
    """
    lines = []
    depth = 0
    in_body = False
    skip_name = False

    for line in oad_iff_txt.splitlines():
        stripped = line.strip()
        if not in_body:
            # Wait for the opening { 'TYPE'
            if "TYPE" in stripped and stripped.startswith("{"):
                in_body = True
                depth = 1
                skip_name = True  # next top-level chunk is { 'NAME' ... }, skip it
            continue

        opens = stripped.count('{')
        closes = stripped.count('}')

        if depth == 1 and stripped.startswith("}"):
            break  # end of TYPE chunk

        if skip_name and depth == 1 and stripped.startswith("{") and "'NAME'" in stripped:
            # single-line NAME chunk — skip
            if closes >= opens:
                skip_name = False
                continue
            skip_name = False

        depth += opens - closes
        if depth >= 1:
            lines.append(line)

    return lines


# ── registration ──────────────────────────────────────────────────────────────

_CLASSES = [WF_OT_import_level, WF_OT_export_level]


def register():
    for cls in _CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(_CLASSES):
        bpy.utils.unregister_class(cls)
