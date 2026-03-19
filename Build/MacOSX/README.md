# macOS Build

This is the native macOS build path for Amulets & Armor.

## Scope

- Uses CMake as the build system.
- Uses the Windows source list as the baseline.
- IPX networking is always enabled.

## Prerequisites

```sh
brew install cmake ninja pkg-config sdl12-compat sdl2 sdl2_net
```

Notes:
- `sdl12-compat` provides SDL 1.2 API compatibility on modern macOS.
- `sdl2` is the backend loaded at runtime by `sdl12-compat`.
- `sdl2_net` provides UDP networking for IPX-over-UDP multiplayer.

## Build Scripts

Scripts live in `Build/MacOSX/` and can be run from anywhere; they resolve
the repo root automatically.  They mirror the role of `Build/DOS/make.bat`
and `Build/DOS/clean.bat`.

| Script | Description |
|---|---|
| `configure.sh [debug\|asan\|release]` | CMake configure step |
| `make.sh [debug\|asan\|release\|clean]` | Configure (if needed) + build |
| `clean.sh` | Remove all `out/macos*` build directories |
| `run.sh [debug\|asan\|release]` | Run the game from the `Exe` directory |

### Quick start (debug build)

```sh
./Build/MacOSX/make.sh
./Build/MacOSX/run.sh
```

### ASAN debug build (recommended while developing)

```sh
./Build/MacOSX/make.sh asan
./Build/MacOSX/run.sh asan
```

### Release build

```sh
./Build/MacOSX/make.sh release
./Build/MacOSX/run.sh release
```

### Clean

```sh
./Build/MacOSX/clean.sh
```

## Multiplayer

Pass the server IP as the first argument and an optional port as the second.
The port defaults to 213 (the original IPX-over-UDP port).

```sh
cd Exe
../out/macos-asan/amulets-armor <server-ip> [<port>]
```

Example against a local AAServer running on port 2130:

```sh
cd Exe
../out/macos-asan/amulets-armor 127.0.0.1 2130
```

## Manual CMake commands

If you prefer to drive CMake directly:

```sh
# Configure
cmake -S . -B out/macos -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build out/macos --target amulets-armor -j

# Run (must be launched from Exe/ so asset paths resolve)
cd Exe
../out/macos/amulets-armor
```
