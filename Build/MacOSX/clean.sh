#!/usr/bin/env bash
# Remove all macOS CMake build output.
# Mirrors the role of Build/DOS/clean.bat for the macOS CMake path.

set -e

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

echo "Cleaning!"

for dir in "$REPO_ROOT/out/macos" \
           "$REPO_ROOT/out/macos-asan" \
           "$REPO_ROOT/out/macos-release"; do
    if [ -d "$dir" ]; then
        rm -rf "$dir"
        echo "  removed $dir"
    fi
done

echo "Clean -- Done"
