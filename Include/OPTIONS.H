/****************************************************************************/
/*    FILE:  OPTIONS.H                                                      */
/****************************************************************************/

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

/* All the following options are turned on and off by commenting out the */
/* #define. */

#ifndef WIN32
//#define WIN32 // TEMP for Eclipse
#endif
#ifndef _DEBUG
#define _DEBUG
#endif

/* Flag that determines if this is the shareware version */
//#define COMPILE_OPTION_SHAREWARE_DEMO
//#define COMPILE_OPTION_CLASSIC_VERSION
#define COMPILE_OPTION_CLASSIC_DEV_VERSION

/* Flag to turn off the frames per second. */
//#define COMPILE_OPTION_FRAME_SPEED_OFF

/* Flag that makes the F1 key turn on/off god mode. */
#define COMPILE_OPTION_ALLOW_GOD_MODE

/* Flag to make the player invincible in that he cannot die. */
//#define COMPILE_OPTION_CANNOT_DIE

/* Flag to turn off creatures. */
//#define COMPILE_OPTION_DONT_LOAD_CREATURES

// Option to allow a character to use all the spells. */
//#define COMPILE_OPTION_LOAD_ALL_SPELLS

// Option to load 16 bit sound effects.
//#define COMPILE_OPTION_16_BIT_SOUND

// Option to turn off all objects on all maps
//#define DONT_LOAD_OBJECTS

// Option to turn off all special objects on maps
//#define DONT_LOAD_SPECIALS

// Option to allow snow on the screen
//#define ALLOW_SNOWY_GRAPHICS

// Option to record all packets going to the client and self server
//#define COMPILE_OPTION_CREATE_PACKET_DATA_FILE

// Option to record all creature movement to ensure syncronized actions
//#define COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE

// Option to turn on indicator lights that show different
// areas of program execution
// Indicators go from left to right as follows:
//   -- Ticker interrupt
//   -- Communications interrupt
//   -- Keyboard interrupt
//   -- Sound interrupt

// The next four lights are in TESTME.C (main()'s main loop)
// NOTE: only one should be shown at any one time
//   --  CmdQ receives
//   --  CmdQ sends
//   --  UpdateOften
//   --  SMMainUpdate    <-- this one is one most of the time

// Next lights:
//   -- ClientUpdate
//   -- ServerUpdate
//   -- View3dDrawView

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
    #define VERSION_TEXT "Shareware Demo V1.2"
#elif defined(COMPILE_OPTION_CLASSIC_VERSION)
    #define VERSION_TEXT "Classic V1.04"
#elif defined(COMPILE_OPTION_CLASSIC_DEV_VERSION)
    #define VERSION_TEXT "Classic Dev V1.04a"
#else
    #define VERSION_TEXT "Registered V1.0"
#endif

#ifndef SERVER_ONLY
//#define INDICATOR_LIGHTS

#ifdef INDICATOR_LIGHTS
#define INDICATOR_LIGHT(p, c)       \
{       \
    ((char *)0xA0000)[p] = (c) ; \
    ((char *)0xA0000)[p+1] = (c)+4 ; \
    ((char *)0xA0000)[p+320] = (c)+4 ; \
    ((char *)0xA0000)[p+321] = (c)+4 ; \
}
#else
#define INDICATOR_LIGHT(p, c)
#endif
#endif

#define INDICATOR_GRAY         16
#define INDICATOR_GREEN        128
#define INDICATOR_RED          144
#define INDICATOR_YELLOW       160
#define INDICATOR_BLUE         48
#define INDICATOR_PURPLE       112

#define OPTIONS_COLORON
//#define OPTIONS_DARKCOLOR

/* Mip mapping is a technique that tends to remove moire patterns */
/* that are off in a distance.  It blurs things far off so they */
/* don't appear so dotty. */
#define MIP_MAPPING_ON

#define TIME_BETWEEN_DAMAGE_WALL_CHECKS  7

/* The following option makes all DebugRoutine and DebugEnd statements */
/* add a local variable that is checked for stack stability. */
//#define COMPILE_OPTION_DEBUG_CHECKS_STACK

/* Option to allow shifting of textures */
//#define COMPILE_OPTION_ALLOW_SHIFT_TEXTURES

/* Option to compile checks to see if the vectors go corrupt. */
//#define COMPILE_OPTION_DEBUG_CHECKS_VECTORS

/* Option to determine if the computers are out of sync */
/* based on their next ObjectCreate() object ids. */
#define COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS

/* Option to have player twist randomly when given a hard smack. */
//#define COMPILE_OPTION_ALLOW_REORIENT_SMACK

/* Optional file to output csync information. (CSYNC.DAT) */
//#define COMPILE_OPTION_RECORD_CSYNC_DAT_FILE

/* Option to turn on copy protection */
//#define COMPILE_OPTION_COPY_PROTECTION_ON

/** Player object characteristics. **/
#define PLAYER_OBJECT_HEIGHT  60
#define PLAYER_OBJECT_RADIUS  20
#define PLAYER_GRAB_DISTANCE  (10 * PLAYER_OBJECT_RADIUS)

/** Object type constants. **/
#define OBJECT_TYPE_TOMATO   12
#define OBJECT_TYPE_APPLE    19
#define OBJECT_TYPE_TORCH    27
#define OBJECT_TYPE_XBOW     28
#define OBJECT_TYPE_SWORD    29

#define OBJECT_TYPE_FIREBALL 31

/** Miscellaneous constants. **/
#define VIEW_WATER_DIP_LEVEL 20
#define SECTOR_SHIFT_X_FLAG  4
#define SECTOR_SHIFT_Y_FLAG  8
#define SECTOR_DIP_FLAG      16
#define SECTOR_ANIMATE_FLOOR_FLAG 32
#define DISTANCE_TO_PICKUP   24
#define PLAYER_TALLNESS      50

#define STORE_ID_START       20000

#define SPECIAL_OBJECTS_START 1500
#define SPECIAL_OBJECTS_END   1999

#define RandomNumber() SyncTimeGet()

#ifndef NULL
#define NULL 0
#endif

#define CompareGameGroupIDs(a, b) (((memcmp(&(a), &(b), sizeof(a)))==0)?TRUE:FALSE)
#define CompareUniqueNetworkIDs(a, b) (((memcmp(&(a), &(b), sizeof(a)))==0)?TRUE:FALSE)

#ifdef WIN32
	#define far
	#define near
	#define huge
	#define __interrupt
	#define __far
	#define __near
	#define COMPILE_OPTION_DIRECT_TALK_IS_DOS32
	#define WIN_IPX		 1 // off
#else
	#define COMPILE_OPTION_DIRECT_TALK_IS_DOS32
#endif

#ifdef WIN32
#pragma warning(disable:4200)
#pragma warning(disable:4100)
//#include <windows.h>
//#include <direct.h>
#include <sdl.h>
// Delay in a number of milliseconds to delay
#define delay(x) if (x < 14) { SDL_Delay(14); } else { SDL_Delay(x); }
/* The mouse driver is handled not in the mouse calls, but in the file 'winmouse.c' */
#define OUTSIDE_MOUSE_DRIVER
/* Remove C++ debug checks */
#define _NDEBUG
#define _disable()
#define _enable()
#endif

#endif

/****************************************************************************/
/*    END OF FILE: OPTIONS.H                                                */
/****************************************************************************/


/* Eric's Email : 71332,3644 */
