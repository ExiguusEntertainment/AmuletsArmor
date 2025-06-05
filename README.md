# Amulets & Armor

Amulets & Armor is a 1997 first person role playing game released as open source. The codebase is written primarily in C and builds for both DOS and Windows.

## Repository layout

- `Artwork/` – icons and related artwork
- `Build/` – build scripts for DOS (`MAKEFILE`) and Windows (Visual Studio projects)
- `Exe/` – packaged binaries, game data and forms
- `Include/` – header files for all modules
- `Lib/` – third-party libraries such as SDL and debug helpers
- `Source/` – main game source code
- `Utils/` – small helper tools

## Build system

The DOS version uses the legacy Watcom makefile found under `Build/DOS`. Windows builds are set up with Visual Studio projects in `Build/Windows/VC2010` and `VC2013`. SDL provides input, video and networking for the Windows port.

## Code structure

The entry point for the game is `Source/TESTME.C`. It initializes configuration, sets up subsystems, then runs the main loop. The source directory is divided into many modules such as graphics, input, networking and UI. Each `.c` file begins with a short header describing its purpose, and has a matching header in `Include/`.

### Networking

Networking originally supported modem, serial and IPX connections through a layer called "Direct Talk" (`DITALK.C`). Later a synchronization layer (`CSYNCPCK.C`) was added so all clients advance in lock step. Random numbers come from a shared table to keep simulations deterministic.

During a synchronization round the client waits until it has received a sync packet from every active player. Once all packets are present, `ClientSyncUpdateReceived()` processes them and advances the synchronized time.

### Synchronized time

`SYNCTIME.C` maintains a global synchronized clock. Other modules read it when scheduling events, and networking code advances it after each synchronization round.

### Modules

A large number of modules make up the game. Examples include 3D rendering (`3D_VIEW.C`), creature logic (`CRELOGIC.C`), object movement (`OBJMOVE.C`), the packet queue (`CMDQUEUE.C`) and the server/client code (`SERVER.C`, `CLIENT.C`). The Doxygen configuration in the repository can generate full documentation listing every module.

## Learning more

1. Run Doxygen using `Doxyfile` to browse module documentation.
2. Examine `Build/` to see how the DOS and Windows versions are compiled.
3. Start with `TESTME.C` and follow calls into the various subsystems to understand game flow.
4. Explore the assets under `Exe/` to learn how content is organized.

