#!/usr/bin/env bash
# =============================================================================
# gui/setup_imgui.sh  —  ECU Smart Home GUI Bootstrap
# Run once from the project root: bash gui/setup_imgui.sh
# Downloads Dear ImGui v1.91.9b into gui/imgui/ (SDL2 + OpenGL3 backend)
# =============================================================================
set -e

IMGUI_TAG="v1.91.9b"
BASE_URL="https://raw.githubusercontent.com/ocornut/imgui/${IMGUI_TAG}"
DEST="$(dirname "$0")/imgui"

echo "=== ECU Smart Home: ImGui Bootstrap ==="
echo "Target dir: $DEST"
mkdir -p "$DEST/backends"

DL() {
    local url="$1"
    local out="$2"
    echo "  Downloading: $(basename "$out")"
    if command -v curl &>/dev/null; then
        curl -fsSL "$url" -o "$out"
    elif command -v wget &>/dev/null; then
        wget -q "$url" -O "$out"
    else
        echo "ERROR: neither curl nor wget found."
        exit 1
    fi
}

# ---- Core ImGui sources ------------------------------------------------------
DL "${BASE_URL}/imconfig.h"            "$DEST/imconfig.h"
DL "${BASE_URL}/imgui.h"              "$DEST/imgui.h"
DL "${BASE_URL}/imgui_internal.h"     "$DEST/imgui_internal.h"
DL "${BASE_URL}/imgui.cpp"            "$DEST/imgui.cpp"
DL "${BASE_URL}/imgui_draw.cpp"       "$DEST/imgui_draw.cpp"
DL "${BASE_URL}/imgui_tables.cpp"     "$DEST/imgui_tables.cpp"
DL "${BASE_URL}/imgui_widgets.cpp"    "$DEST/imgui_widgets.cpp"
DL "${BASE_URL}/imgui_demo.cpp"       "$DEST/imgui_demo.cpp"
DL "${BASE_URL}/imstb_rectpack.h"     "$DEST/imstb_rectpack.h"
DL "${BASE_URL}/imstb_textedit.h"     "$DEST/imstb_textedit.h"
DL "${BASE_URL}/imstb_truetype.h"     "$DEST/imstb_truetype.h"

# ---- SDL2 + OpenGL3 backend --------------------------------------------------
DL "${BASE_URL}/backends/imgui_impl_sdl2.h"         "$DEST/backends/imgui_impl_sdl2.h"
DL "${BASE_URL}/backends/imgui_impl_sdl2.cpp"       "$DEST/backends/imgui_impl_sdl2.cpp"
DL "${BASE_URL}/backends/imgui_impl_opengl3.h"      "$DEST/backends/imgui_impl_opengl3.h"
DL "${BASE_URL}/backends/imgui_impl_opengl3.cpp"    "$DEST/backends/imgui_impl_opengl3.cpp"
DL "${BASE_URL}/backends/imgui_impl_opengl3_loader.h" "$DEST/backends/imgui_impl_opengl3_loader.h"

echo ""
echo "=== Done! ImGui ${IMGUI_TAG} vendored into ${DEST}/ ==="
echo "Now run: make"
