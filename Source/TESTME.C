/*-------------------------------------------------------------------------*
 * File:  TESTME.C
 *-------------------------------------------------------------------------*/
/**
 * TestMe is the old old old name for Main.
 *
 * @addtogroup TESTME
 * @brief Main Top Level Code
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include <ctype.h>
#if WIN32
#include <conio.h>
#include <SDL_net.h>
#endif
#include "CLIENT.H"
#include "CMDQUEUE.H"
#include "COLOR.H"
#include "CONFIG.H"
#include "FILE.H"
#include "GENERAL.H"
#include "KEYBOARD.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"
#include "MESSAGE.H"
#include "MOUSEMOD.H"
#include "PACKET.H"
#include "PICS.H"
#include "SOUND.H"
#include "SMMAIN.H"
#include "SYNCMEM.H"
#include "TICKER.H"
#include "UPDATE.H"
#include "VIEW.H"
#ifdef WIN32
#include "Win32\ipx_client.h"
#endif
#include "AALua.h"

#define CAP_TICK_RATE   1

extern T_void PacketReceiveData(T_void *p_data, T_word16 size) ;

void UpdateCmdqueue(void)
{
    TICKER_TIME_ROUTINE_PREPARE() ;
    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("UpdateCmdqueue") ;

    CmdQUpdateAllReceives() ;
    CmdQUpdateAllSends() ;

    DebugEnd() ;
    TICKER_TIME_ROUTINE_ENDM("UpdateCmdqueue", 500) ;
}

#ifdef WIN32
T_void game_main(T_word16 argc, char *argv[])
#else
T_void main(T_word16 argc, char *argv[])
#endif
{
    T_word32 i = 0 ;
    T_directTalkHandle handle ;
static T_word32 lastTick=0;
extern void SleepMS(T_word32 sleepMS);
    //    void (__interrupt __far *oldbreak)(); /* interrupt function pointer */

#ifdef WIN32
    /* Redirect the standard output to a file */
//    freopen("stdout.out", "w", stdout) ;
#endif
    TICKER_TIME_ROUTINE_PREPARE() ;

    DebugRoutine("main") ;

    ConfigOpen() ;
    ConfigLoad() ;

    puts("Amulets & Armor version 1.0 -- (C) 1996 United Software Artists") ;
    puts("---------------------------------------------------------------") ;

    SyncMemClear() ;

    if (MouseCheckInstalled() == FALSE)  {
        puts("-- ERROR --------------------------------------") ;
        puts("Amulets and Armor requires a mouse to play.") ;
        puts("No mouse was detected on this computer.") ;
        exit(1) ;
    }

#ifndef WIN32
    if (argc != 2)  {
        puts("USAGE: GAME <Direct Talk Handle>") ;
        DebugEnd() ;
        exit(1) ;
    }

    sscanf(argv[1], "%lX", &handle) ;
//    printf("DirectTalk Handle: 0x%08lX\n", handle) ;
#else
    if (argc == 1) {
        handle = 0;
    } else if (argc == 2)  {
#if WIN_IPX
        // An IP address is given.  Set it to connect there
        if(SDLNet_Init()==-1) {
            printf("SDLNet_Init: %s\n", SDLNet_GetError());
            exit(2);
        }
        if (!IPXConnectToServer(argv[1])) {
            printf("IPX Connection failed!\n");
            exit(3);
        }
        handle = 1; // TODO:
#else
        handle = 0;
#endif
    } else {
        puts("USAGE: GAME [<IP Address for network server>]") ;
        DebugEnd() ;
        exit(1) ;
    }
#endif

    DirectTalkInit(
         PacketReceiveData,
         NULL,
         NULL,
         NULL,
         handle) ;

    /* Initialize the game. */
    UpdateGameBegin() ;
    DebugSaveVectorTable() ;
    AALuaInit();

//#ifdef NDEBUG
    /* If this is NOT the debug version, redirect the output. */
#ifdef COMPILER_WATCOM
    freopen("stdout.out", "w", stdout) ;
#endif
    //    freopen("stderr.out", "w", stderr) ;
//#endif

    AALuaCallGlobalFunction0("titlescreen");

    GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
    ColorUpdate(1) ;
    ViewSetPalette(VIEW_PALETTE_STANDARD) ;
    ClientInit();

    SMMainInit() ;
    while (!SMMainIsDone())  {
        if ((TickerGet() - lastTick) < CAP_TICK_RATE) {
#ifdef WIN32
            SleepMS(1);
#endif
        } else {
            lastTick = TickerGet();
            TICKER_TIME_ROUTINE_START() ;
            DebugCompare("main") ;
            UpdateCmdqueue() ;
            DebugCompare("main") ;
            UpdateOften() ;
            DebugCompare("main") ;
            SMMainUpdate() ;
            DebugCompare("main") ;
            TICKER_TIME_ROUTINE_ENDM("main", 500) ;
        }
    }

    SMMainFinish() ;

    ClientFinish() ;

    UpdateGameEnd() ;

    DirectTalkFinish(handle) ;

//    _dos_setvect(0x23, oldbreak) ;

    ConfigClose() ;
    AALuaFinish();

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  TESTME.C
 *-------------------------------------------------------------------------*/
