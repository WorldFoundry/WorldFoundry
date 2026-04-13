#!/usr/bin/env bash
# Install the wf_blender add-on into Blender's add-ons folder.
#
# Usage:
#   ./install.sh [/path/to/blender/scripts/addons]
#
# If no destination is given, defaults to ~/.config/blender/<latest>/scripts/addons/
#
# After running this, enable "World Foundry" in:
#   Blender > Edit > Preferences > Add-ons

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WF_PY_DIR="$(dirname "$SCRIPT_DIR")/wf_py"

# ── build if needed ───────────────────────────────────────────────────────────
WHEEL=$(find "$WF_PY_DIR/target/wheels" -name "wf_core*.whl" 2>/dev/null | sort | tail -1)
if [[ -z "$WHEEL" ]]; then
    echo "Building wf_core..."
    (cd "$WF_PY_DIR" && maturin build --release)
    WHEEL=$(find "$WF_PY_DIR/target/wheels" -name "wf_core*.whl" 2>/dev/null | sort | tail -1)
fi
if [[ -z "$WHEEL" ]]; then
    echo "Build failed — no wheel found"
    exit 1
fi
echo "Using wheel: $WHEEL"

# ── extract wf_core.so from wheel ─────────────────────────────────────────────
SO_TMP=$(mktemp /tmp/wf_core_XXXXXX.so)
python3 - <<EOF
import zipfile, sys
wheel = "$WHEEL"
with zipfile.ZipFile(wheel) as z:
    names = [n for n in z.namelist() if n.endswith('.so')]
    if not names:
        print(f"No .so found in {wheel}", file=sys.stderr)
        sys.exit(1)
    data = z.read(names[0])
with open("$SO_TMP", 'wb') as f:
    f.write(data)
EOF
SO="$SO_TMP"
echo "Extracted: $SO"

# ── resolve Blender addons dir ────────────────────────────────────────────────
if [[ $# -ge 1 ]]; then
    ADDONS_DIR="$1"
else
    # Find the highest versioned Blender config dir
    CONFIG_BASE="${XDG_CONFIG_HOME:-$HOME/.config}/blender"
    if [[ ! -d "$CONFIG_BASE" ]]; then
        echo "Blender config dir not found: $CONFIG_BASE"
        echo "Is Blender installed?  Try: sudo snap install blender --classic"
        exit 1
    fi
    BLENDER_VER=$(ls "$CONFIG_BASE" | sort -V | tail -1)
    ADDONS_DIR="$CONFIG_BASE/$BLENDER_VER/scripts/addons"
fi

DEST="$ADDONS_DIR/wf_blender"
mkdir -p "$DEST"

# ── copy add-on Python files ──────────────────────────────────────────────────
cp "$SCRIPT_DIR/__init__.py"  "$DEST/"
cp "$SCRIPT_DIR/operators.py" "$DEST/"
cp "$SCRIPT_DIR/panels.py"    "$DEST/"

# ── copy native library ───────────────────────────────────────────────────────
cp "$SO" "$DEST/wf_core.so"

echo ""
echo "Installed to: $DEST"
echo ""
echo "Next steps:"
echo "  1. Open Blender"
echo "  2. Edit > Preferences > Add-ons"
echo "  3. Search 'World Foundry' and enable it"
echo "  4. Select any mesh object"
echo "  5. Properties > Object > World Foundry panel"
echo "  6. Click the folder icon and choose a .oad file"
