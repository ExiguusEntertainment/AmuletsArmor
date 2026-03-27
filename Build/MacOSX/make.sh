#!/usr/bin/env bash
# Build the macOS target.
#
# Usage:
#   make.sh                -- configure (if needed) then build Debug
#   make.sh asan           -- configure (if needed) then build ASAN Debug
#   make.sh release        -- configure (if needed) then build Release
#   make.sh clean          -- remove all out/macos* build directories
#
# Mirrors the role of Build/DOS/make.bat for the macOS CMake path.

set -e

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SCRIPT_DIR="$REPO_ROOT/Build/MacOSX"

MODE="${1:-debug}"

if [ "$MODE" = "clean" ]; then
    echo "Make -- Clean"
    "$SCRIPT_DIR/clean.sh"
    echo "Make -- Done"
    exit 0
fi

case "$MODE" in
    asan)     BUILD_DIR="$REPO_ROOT/out/macos-asan" ;;
    release)  BUILD_DIR="$REPO_ROOT/out/macos-release" ;;
    debug|*)  BUILD_DIR="$REPO_ROOT/out/macos" ;;
esac

# Auto-configure if the build directory does not yet exist.
if [ ! -d "$BUILD_DIR" ]; then
    echo "Make -- Configuring ($MODE)"
    "$SCRIPT_DIR/configure.sh" "$MODE"
fi

echo "Make -- Building ($MODE)"
cmake --build "$BUILD_DIR" --target amulets-armor -j

echo "Make -- Done"
