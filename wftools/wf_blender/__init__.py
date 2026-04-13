"""
World Foundry Blender Add-on
============================

Attaches OAD-driven schemas to Blender objects and provides:
  - dynamic property panel (rendered from Rust schema descriptors)
  - validation
  - .iff.txt export

Installation
------------
Copy this directory (wf_blender/) and wf_core.so into your Blender add-ons folder:
  ~/.config/blender/<version>/scripts/addons/wf_blender/

The wf_core.so lives in:
  wftools/wf_py/target/wheels/  (extract from wheel, or build with maturin)

Then enable "World Foundry" in Edit > Preferences > Add-ons.
"""

import sys
import os

bl_info = {
    "name":        "World Foundry",
    "author":      "World Foundry",
    "version":     (0, 1, 0),
    "blender":     (4, 0, 0),
    "location":    "Properties > Object > World Foundry",
    "description": "OAD schema-driven game object attributes",
    "category":    "Game Engine",
}

# ---------------------------------------------------------------------------
# wf_core import — the compiled Rust extension must live next to this file.
# ---------------------------------------------------------------------------

_addon_dir = os.path.dirname(__file__)
if _addon_dir not in sys.path:
    sys.path.insert(0, _addon_dir)

try:
    import wf_core  # noqa: F401  (used by sub-modules)
    _WF_CORE_OK = True
except ImportError as _e:
    _WF_CORE_OK = False
    _WF_CORE_ERROR = str(_e)

# ---------------------------------------------------------------------------
# Sub-module registration
# ---------------------------------------------------------------------------

from . import operators  # noqa: E402
from . import panels     # noqa: E402


def register():
    if not _WF_CORE_OK:
        import bpy
        def draw_error(self, context):
            self.layout.label(
                text=f"wf_core not found — copy wf_core.so into add-on folder: {_WF_CORE_ERROR}",
                icon='ERROR',
            )
        bpy.app.timers.register(
            lambda: bpy.context.window_manager.popup_menu(draw_error, title="World Foundry"),
            first_interval=0.1,
        )
        return

    operators.register()
    panels.register()


def unregister():
    if _WF_CORE_OK:
        panels.unregister()
        operators.unregister()
