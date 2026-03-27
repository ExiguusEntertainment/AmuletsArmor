#!/usr/bin/env bash
# Run the game from the Exe directory so asset paths resolve correctly.
#
# Usage:
#   run.sh                 -- run the Debug build
#   run.sh asan            -- run the ASAN Debug build
#   run.sh release         -- run the Release build

set -e

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

MODE="${1:-debug}"

case "$MODE" in
    asan)     BINARY="$REPO_ROOT/out/macos-asan/amulets-armor" ;;
    release)  BINARY="$REPO_ROOT/out/macos-release/amulets-armor" ;;
    debug|*)  BINARY="$REPO_ROOT/out/macos/amulets-armor" ;;
esac

if [ ! -f "$BINARY" ]; then
    echo "Binary not found: $BINARY"
    echo "Run make.sh first."
    exit 1
fi

cd "$REPO_ROOT/Exe"
exec "$BINARY"
