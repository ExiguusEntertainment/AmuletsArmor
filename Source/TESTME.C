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

//#undef TRUE
//#undef FALSE

#define CAP_TICK_RATE   1

extern T_void PacketReceiveData(T_void *p_data, T_word16 size) ;

static T_void IShowScreenNextPage (E_mouseEvent event,
                         T_word16 x,
                         T_word16 y,
                         T_buttonClick button);
static E_Boolean G_mouseClicked=FALSE;

void HangUp(void)
{
}

#ifdef COMPILER_WATCOM
T_sword32 SetEAX(T_void) ;
#pragma aux SetEAX = \
            "mov eax, 0xFF" \
            "iret" \
            value [eax] \
            modify [eax] ;

T_void NoBreakPart2(T_void) ;
#pragma aux NoBreakPart2 = \
            "mov ax, 0x3301" \
            "mov dl, 0" \
            "int 0x21" \
            modify [eax edx] ;
void __interrupt __far NoBreak(void)
{
    SetEAX() ;
}
#endif

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

static E_Boolean IShowScreen(
                  T_byte8 *picName,
                  T_viewPalette pal,
                  T_word32 timeout,
                  E_Boolean showTag,
                  E_Boolean doFlash)
{
    T_bitmap *p_bitmap=NULL ;
    T_resource res ;
    T_word32 time ;
    E_Boolean bypassed = FALSE ;
    static E_Boolean wasGamma = FALSE;

    DebugRoutine("IShowScreen") ;
    MousePushEventHandler(IShowScreenNextPage);

    time = TickerGet() + timeout ;
    p_bitmap = (T_bitmap *)PictureLockData(picName, &res) ;
    DebugCheck (p_bitmap!=NULL);
    if (p_bitmap)  {
        ViewSetPalette(pal) ;
        ColorStoreDefaultPalette();
        if (doFlash)
            ColorAddGlobal(64, 64, 64) ;
//        else
//            ColorAddGlobal(-64, -64, -64) ;
        GrDrawBitmap(p_bitmap, 0, 0) ;
        PictureUnlockAndUnfind(res) ;

        if (showTag)
        {
            /* first time show version number */
            GrSetCursorPosition (5,188);
            GrDrawShadowedText (VERSION_TEXT,210,0);
        }
        KeyboardBufferOn() ;
        while (time > TickerGet())
        {
            if (KeyboardBufferGet() != 0)  {
                bypassed = TRUE ;
                break ;
            }
            MouseUpdateEvents();
            if (G_mouseClicked==TRUE) {
                bypassed = TRUE ;
                break;
            }
            ColorUpdate(1) ;
            if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT) == TRUE) {
                if (KeyMapGetScan(KEYMAP_GAMMA_CORRECT) == TRUE)  {
                    if (wasGamma == FALSE) {
                        MessagePrintf("Gamma level %d",
                            ColorGammaAdjust()) ;
                        ColorUpdate(1) ;
                        wasGamma = TRUE;
                    }
                } else {
                    wasGamma = FALSE;
                }
            } else {
                wasGamma = FALSE;
            }
        }

//        GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
    }
    MousePopEventHandler();

    DebugEnd() ;

    return bypassed ;
}


static T_void IShowScreenNextPage (E_mouseEvent event,
                         T_word16 x,
                         T_word16 y,
                         T_buttonClick button)
{
    DebugRoutine ("IShowScreenNextPage");

    if (button==MOUSE_BUTTON_LEFT &&
        event==MOUSE_EVENT_START)
    {
        G_mouseClicked=TRUE;
    }
    else
    {
        G_mouseClicked=FALSE;
    }

    DebugEnd();
}

T_void CheckCopyProtection(T_void)
{
    T_file file ;
    char filename[80] ;
    char code[20] ;
    E_Boolean good = FALSE ;
    char key ;

    DebugRoutine("CheckCopyProtection") ;

    do {
        sprintf(filename, "%c:\\morepics.res", ConfigGetCDROMDrive()) ;
        file = FileOpen(filename, FILE_MODE_READ) ;
        if (file != FILE_BAD)  {
            FileSeek(file, 333333333L) ;
            FileRead(file, code, 20) ;
            FileClose(file) ;

            if (strncmp("YcArIp", code, 7) == 0)
                good = TRUE ;
        }

        if (good == FALSE)  {
            printf("Please insert the Amulets & Armor CDROM into drive %c:\n", toupper(ConfigGetCDROMDrive())) ;
            puts("and press SPACE to continue.  If this is the wrong drive,") ;
            puts("press the letter of the drive to check.  Hit ESC to cancel.") ;

            do  {
                key = toupper(getch()) ;

                if (key == 0x1b)  {
                    puts("Aborting game.") ;
                    exit(1) ;
                }

                if ((key >= 'D') && (key <= 'Z'))  {
                    code[0] = key ;
                    code[1] = '\0' ;
                    INIFilePut(ConfigGetINIFile(), "cdrom", "drive", code) ;
                    ConfigClose() ;
                    ConfigOpen() ;
                    key = ' ' ;
                }
            } while (key != ' ') ;
        }
    } while (good == FALSE) ;

    DebugEnd() ;
}

//#define PULL_OUT

#ifdef PULL_OUT
T_void PullOut(T_void)
{
    int i ;
    char filename[80] ;
    char ofilename[80] ;
    T_word32 size ;
    T_resource res ;
    T_bitmap *p_bitmap ;
    FILE *fp ;

    DebugRoutine("PullOut") ;

    i=0;

    while (1)  {
        sprintf(filename, "UI/EFICONS/ICO%05d", i) ;
        if (!PictureExist(filename))
            break ;

        p_bitmap = PictureToBitmap(PictureLock(filename, &res)) ;
        sprintf(ofilename, "ICONS\\ICO%05d", i) ;
        puts(ofilename) ;
        fp = fopen(ofilename, "wb") ;
        fwrite(p_bitmap, ResourceGetSize(res), 1, fp) ;
        fclose(fp) ;
        i++ ;
    }
    i = 0 ;
    while (1)  {
        sprintf(filename, "UI/3DUI/RUNBUT%02d", i);
        if (!PictureExist(filename))
            break ;

        p_bitmap = PictureToBitmap(PictureLock(filename, &res)) ;
        sprintf(ofilename, "\\manual\\runes\\rune%02d.pic", i) ;
        puts(ofilename) ;
        fp = fopen(ofilename, "wb") ;
        fwrite(p_bitmap, ResourceGetSize(res), 1, fp) ;
        fclose(fp) ;
        i++ ;
    }
    DebugEnd() ;
}
#endif

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
#ifdef COMPILE_OPTION_COPY_PROTECTION_ON
    CheckCopyProtection() ;
#endif

    SyncMemClear() ;

    /* Do not allow CTRL-Break */
//    oldbreak = _dos_getvect(0x23);
//    _dos_setvect(0x23, NoBreak) ;
//    NoBreakPart2() ;

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

#ifdef PULL_OUT
    PullOut() ;
#endif

//#ifdef NDEBUG
    /* If this is NOT the debug version, redirect the output. */
#ifdef COMPILER_WATCOM
    freopen("stdout.out", "w", stdout) ;
#endif
    //    freopen("stderr.out", "w", stderr) ;
//#endif

//#ifdef NDEBUG
    SoundPlayByNumber(3501, 0) ;
//    if (IShowScreen("UI/SCREENS/USA1", VIEW_PALETTE_MAIN_TITLE, 14, FALSE, FALSE) == FALSE)  {
        ColorUpdate(1) ;
//        delay(200) ;
        SoundPlayByNumber(3501, 255) ;
        //if (IShowScreen("UI/SCREENS/USA1", VIEW_PALETTE_MAIN_TITLE, 400, FALSE, TRUE) == FALSE)  {
        if (IShowScreen("UI/SCREENS/COMPANY", VIEW_PALETTE_STANDARD, 400, TRUE, TRUE) == FALSE)  {
            ColorFadeTo(0,0,0);
            GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
            ColorUpdate(1) ;
            ViewSetPalette(VIEW_PALETTE_STANDARD) ;
            //if (IShowScreen("UI/SCREENS/USA2", VIEW_PALETTE_MAIN_TITLE, 250, FALSE, FALSE) == FALSE)  {
            if (IShowScreen("UI/SCREENS/PRESENTS", VIEW_PALETTE_STANDARD, 250, TRUE, FALSE) == FALSE)  {
                ColorFadeTo(0,0,0);
                GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
                ColorUpdate(1) ;
                ViewSetPalette(VIEW_PALETTE_STANDARD) ;
//                SoundPlayByNumber(3501, 255) ;
//                if (IShowScreen("UI/SCREENS/BEGIN1", VIEW_PALETTE_STANDARD, 14, TRUE, TRUE) == FALSE)  {
                    delay(350) ;
//                    SoundPlayByNumber(3501, 255) ;
                    if (IShowScreen("UI/SCREENS/BEGIN1", VIEW_PALETTE_STANDARD, 1000, TRUE, FALSE) == FALSE)  {
                        ColorFadeTo(0,0,0);
                        GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
                        ColorUpdate(1) ;
                        ViewSetPalette(VIEW_PALETTE_STANDARD) ;
#                   ifdef COMPILE_OPTION_SHAREWARE_DEMO
                        IShowScreen("UI/SCREENS/BEGIN2", VIEW_PALETTE_STANDARD, 350, TRUE, FALSE);
                        ColorFadeTo(0,0,0);
#                   endif
                    }
//                }
            }
        }
//    }
//#endif

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

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
    ColorFadeTo(0,0,0);
    GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
    ColorUpdate(1) ;
    IShowScreen("UI/SCREENS/END1", VIEW_PALETTE_STANDARD, 350, FALSE, FALSE) ;
    ColorFadeTo(0,0,0);
    GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, 0) ;
    ColorUpdate(1) ;
    IShowScreen("UI/SCREENS/END2", VIEW_PALETTE_STANDARD, 350, FALSE, FALSE) ;
    ColorFadeTo(0,0,0);
#endif

    UpdateGameEnd() ;

    DirectTalkFinish(handle) ;

//    _dos_setvect(0x23, oldbreak) ;

    ConfigClose() ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  TESTME.C
 *-------------------------------------------------------------------------*/
