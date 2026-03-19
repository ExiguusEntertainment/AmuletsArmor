# macOS Build (Initial Port)

This is the first native macOS build path for Amulets & Armor.

## Scope

- Uses CMake as the build system.
- Uses the Windows source list as the baseline.
- Builds a playable binary with IPX networking disabled by default.

## Prerequisites

Install required tools:

```sh
brew install cmake ninja pkg-config sdl12-compat sdl2
```

Notes:
- `sdl12-compat` provides SDL 1.2 API compatibility on modern macOS.
- `sdl2` is the backend loaded at runtime by `sdl12-compat`.
- The default build (`AA_ENABLE_IPX=OFF`) expects only the pkg-config package `sdl`.
- If you later enable IPX, also install `sdl2_net` and adapt pkg-config lookup to your local package name.

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

## Manual CMake commands

If you prefer to drive CMake directly:

```sh
# Configure
cmake -S . -B out/macos -G Ninja -DAA_ENABLE_IPX=OFF -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build out/macos --target amulets-armor -j

# Run (must be launched from Exe/ so asset paths resolve)
cd Exe
../out/macos/amulets-armor
```
