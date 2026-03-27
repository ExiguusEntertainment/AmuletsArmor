#!/usr/bin/env bash
# Configure the macOS CMake build.
#
# Usage:
#   configure.sh           -- Debug build (out/macos)
#   configure.sh asan      -- Debug + AddressSanitizer (out/macos-asan)
#   configure.sh release   -- Release build (out/macos-release)
#
# Run from repo root or from Build/MacOSX; the script resolves the root itself.

set -e

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

MODE="${1:-debug}"

case "$MODE" in
    asan)
        BUILD_DIR="$REPO_ROOT/out/macos-asan"
        CMAKE_TYPE="Debug"
        EXTRA="-DAA_ENABLE_ASAN=ON"
        ;;
    release)
        BUILD_DIR="$REPO_ROOT/out/macos-release"
        CMAKE_TYPE="Release"
        EXTRA=""
        ;;
    debug|*)
        BUILD_DIR="$REPO_ROOT/out/macos"
        CMAKE_TYPE="Debug"
        EXTRA=""
        ;;
esac

echo "Configure -- mode=$MODE  dir=$BUILD_DIR"

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_TYPE" \
    $EXTRA

echo "Configure -- Done"
