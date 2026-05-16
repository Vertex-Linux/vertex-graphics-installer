#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
#  build.sh  –  Build arch-gpu-driver-installer
#
#  Usage:
#    chmod +x build.sh && ./build.sh
#    ./build.sh --run        launch after building
#    ./build.sh --install    install to /usr/local/bin (needs sudo)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BINARY="$BUILD_DIR/arch-gpu-driver-installer"

# ── Dependency check ──────────────────────────────────────────────────────────
missing_deps=()
for cmd in cmake make g++; do
    command -v "$cmd" &>/dev/null || missing_deps+=("$cmd")
done

if ! pkg-config --exists Qt6Widgets 2>/dev/null; then
    missing_deps+=("qt6-base")
fi

if [ ${#missing_deps[@]} -gt 0 ]; then
    echo "✗  Missing dependencies: ${missing_deps[*]}"
    echo ""
    echo "Install with:"
    echo "  sudo pacman -S --needed cmake make gcc qt6-base pciutils"
    exit 1
fi

echo "✓  All dependencies found"

# ── Configure ─────────────────────────────────────────────────────────────────
echo ""
echo "==> Configuring..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    --log-level=WARNING

# ── Compile ───────────────────────────────────────────────────────────────────
echo "==> Compiling with $(nproc) jobs..."
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

echo ""
echo "✓  Binary: $BINARY"
echo "   Size  : $(du -sh "$BINARY" | cut -f1)"

# ── Optional flags ────────────────────────────────────────────────────────────
for arg in "$@"; do
    case "$arg" in
        --run)
            echo ""
            echo "==> Launching..."
            exec "$BINARY"
            ;;
        --install)
            echo ""
            echo "==> Installing to /usr/local/bin..."
            sudo install -Dm755 "$BINARY" /usr/local/bin/arch-gpu-driver-installer
            echo "✓  Done. Run: arch-gpu-driver-installer"
            ;;
    esac
done

echo ""
echo "Run it:"
echo "  $BINARY"
echo "  # or to install system-wide:"
echo "  ./build.sh --install"
