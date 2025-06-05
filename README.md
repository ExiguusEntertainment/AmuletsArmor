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

- `3D_COLLI.C`   - 3D collision routines
- `3D_IO.C`      - 3D map loading/unloading
- `3D_TRIG.C`    - 3D math utilities
- `3D_VIEW.C`    - 3D rendering system
- `ACTIVITY.C`   - Processing of map script activities
- `AREASND.C`    - Area-based sound effects
- `BANKUI.C`     - Bank user interface
- `BANNER.C`     - Bottom banner UI
- `BUTTON.C`     - Generic UI button component
- `CLIENT.C`     - Player actions and top-level control
- `CLI_RECV.C`   - Client side packet reception
- `CLI_SEND.C`   - Client side packet sending
- `CMDQUEUE.C`   - Networking command/packet queue
- `COLOR.C`      - Palette color control
- `COLORIZE.C`   - Palette colorization helpers
- `COMWIN.C`     - Communications window UI
- `CONFIG.C`     - CONFIG.INI handling
- `CONTROL.C`    - Mouse input in game
- `CRELOGIC.C`   - Creature logic/AI routines
- `CSYNCPCK.C`   - Synchronized packet handling
- `DBLLINK.C`    - Double linked list utilities
- `DEBUG.C`      - Debug call stack system
- `DEBUGBOR.C`   - Borland C debug utilities
- `DITALK.C`     - Direct Talk system
- `DOOR.C`       - Door control code
- `EFFECT.C`     - Item and spell effects
- `EFX.C`        - Special effects in 3D world
- `FILE.C`       - File I/O helpers
- `FORM.C`       - Generic form UI component
- `GRAPHIC.C`    - Graphic UI component helpers
- `GRAPHICS.C`   - Low level drawing system
- `GUILDUI.C`    - Guild user interface
- `HARDFORM.C`   - Hard form UI game system
- `HOUSEUI.C`    - Housing user interface
- `INIFILE.C`    - INI file parser
- `INNUI.C`      - Inn user interface
- `INVENTOR.C`   - Inventory user interface
- `KEYBOARD.C`   - Keyboard controls
- `KEYMAP.C`     - Key remapping system
- `LIGHT.C`      - Map lighting tables
- `LOOK.C`       - "Look" user interface
- `MAINUI.C`     - Character creation main UI
- `MAP.C`        - Map interface
- `MAPANIM.C`    - Map animation configuration
- `MEMORY.C`     - Memory allocation/freeing
- `MESSAGE.C`    - Message rendering
- `MOUSEMOD.C`   - OS-level mouse interface
- `NOTES.C`      - Notes banner window
- `OBJECT.C`     - Objects in the world
- `OBJGEN.C`     - Object generator
- `OBJMOVE.C`    - Object movement subsystem
- `OBJTYPE.C`    - Object type definitions
- `OVERHEAD.C`   - Overhead map view
- `OVERLAY.C`    - Overlay animations
- `PACKETDT.C`   - Packet communications
- `PEOPHERE.C`   - "People here" window
- `PICS.C`       - Picture resource loader
- `PLAYER.C`     - Player object
- `PROMPT.C`     - Prompt status bar UI
- `RANDOM.C`     - Random number generator
- `RESOURCE.C`   - Resource file system
- `SCHEDULE.C`   - Event scheduler
- `SCRFORM.C`    - Script-based forms
- `SCRIPT.C`     - Script system
- `SERVER.C`     - Server/world code
- `SERVERSH.C`   - Code shared with servers
- `SLIDER.C`     - Sliding wall/door system
- `SLIDR.C`      - UI scroll bar component
- `SMACHINE.C`   - Generic state machine system
- `SMCCHOOS.C`   - Character selection state machine
- `SMCPLAY.C`    - In-game state machine
- `SMMAIN.C`     - State machine top level
- `SOUND.C`      - Sound system
- `SPELLS.C`     - Spell logic
- `STATS.C`      - Player statistics
- `STORE.C`      - Store user interface
- `SYNCMEM.C`    - Synchronization memory utilities
- `SYNCTIME.C`   - Synchronized time module
- `TESTME.C`     - Main entry point and loop
- `TEXT.C`       - Text UI component
- `TICKER.C`     - Timer/ticker system
- `TOWNUI.C`     - Town user interface
- `TXTBOX.C`     - UI text box component
- `TXTFLD.C`     - UI text field component
- `UI.C`         - UI top-level control
- `UIBUTTON.C`   - UI button implementation
- `UITEXT.C`     - UI text helpers
- `UPDATE.C`     - Miscellaneous updates
- `VIEW.C`       - Map view drawing
- `VM.C`         - Virtual memory helpers
- `DOS/DOSDTALK.C`   - DOS implementation of Direct Talk
- `Win32/WINDTALK.C` - Windows implementation of Direct Talk
- `Win32/ipx_client.*` - IPX networking over UDP (Windows)

## Learning more

1. Run Doxygen using `Doxyfile` to browse module documentation.
2. Examine `Build/` to see how the DOS and Windows versions are compiled.
3. Start with `TESTME.C` and follow calls into the various subsystems to understand game flow.
4. Explore the assets under `Exe/` to learn how content is organized.

