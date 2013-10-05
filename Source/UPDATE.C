/*-------------------------------------------------------------------------*
 * File:  UPDATE.C
 *-------------------------------------------------------------------------*/
/**
 * This is various update code for controlling timing updates. It is
 * primarily to update animations.
 *
 * @addtogroup UPDATE
 * @brief Various Updates
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "BANNER.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CMDQUEUE.H"
#include "EFX.H"
#include "INVENTOR.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "NOTES.H"
#include "PEOPHERE.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SERVER.H"
#include "SOUND.H"
#include "SPELLS.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "VIEW.H"
#include "UPDATE.H"

static E_Boolean G_gameBegan = FALSE ;
static E_Boolean G_mapBegan = FALSE ;
static T_word32 G_lastFrameTime = 0 ;
static T_word32 G_lastFiveTime = 0 ;
static T_word32 G_lastOftenTime = 0 ;

#ifndef SERVER_ONLY
static T_playerStats G_playerStats ;
#endif

static    T_bitfont *G_p_font ;
static    T_bitfont *G_p_font2 ;
static    T_resource G_r_font ;
static    T_resource G_r_font2 ;
static    T_resourceFile G_res ;
static    T_sword16 G_musicChannel=0;

T_void TestPort(T_void) ;

/*-------------------------------------------------------------------------*
 * Routine:  UpdateGameBegin
 *-------------------------------------------------------------------------*/
/**
 *  UpdateGameBegin is one of the first routines called in the game.
 *  All game initialization goes here.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateGameBegin(T_void)
{
    DebugRoutine("UpdateGameBegin") ;
    DebugCheck(G_gameBegan == FALSE) ;

    G_gameBegan = TRUE ;

#ifndef SERVER_ONLY
    /* Set the active player stats. */
    memset(&G_playerStats, 0, sizeof(G_playerStats)) ;
    StatsSetActive(&G_playerStats) ;

//printf("Largest memory block: %u\n", FreeMemory()) ;
//printf("Loading config file ... \nEvaluation: ") ;
//fflush(stdout) ;
//puts("ConfigLoad") ;
//fflush(stdout) ;
//puts("SoundINitialize") ;
//fflush(stdout) ;
    SoundInitialize() ;

/* set sound options */
BannerInitSoundOptions();
    TickerOn() ;
/* TESTING */
SyncTimeSet(1) ;

    DebugTime(1) ;

    KeyboardOn() ;
    PicturesInitialize() ;
#ifndef SERVER_ONLY
    ColorizeInitialize() ;
#endif
    MouseInitialize() ;
//puts("Script Init") ;
//fflush(stdout) ;
    ScriptInitialize() ;

//printf("Largest memory block: %u\n", FreeMemory()) ;
//fflush(stdout) ;

//    CommReadConfigFile() ;
//puts("Active port") ;  fflush(stdout) ;

//    CommSetActivePortN(0) ;
//TestPort() ;

//puts("CmdQ Init") ;  fflush(stdout) ;
    CmdQInitialize() ;

    G_res = ResourceOpen("sample.res") ;
    DebugCheck(G_res != RESOURCE_FILE_BAD) ;

    GrGraphicsOn() ;
//puts("View Init") ; fflush(stdout) ;
    ViewInitialize() ;

//puts("Locking in fonts") ; fflush(stdout) ;
    G_r_font = ResourceFind(G_res, "FontNormal") ;
    G_p_font = ResourceLock(G_r_font) ;
    G_r_font2 = ResourceFind(G_res, "FontEngl") ;
    G_p_font2 = ResourceLock(G_r_font2) ;
    GrSetBitFont(G_p_font) ;

/*
    r2 = ResourceFind(G_res, "BPalette") ;
    p_pal = ResourceLock(G_r2) ;
    GrSetPalette(0, 256, p_pal) ;
    ResourceUnlock(G_r2) ;
    ResourceUnfind(G_r2) ;
*/

/*
//    r1 = ResourceFind(res, "T.Pic") ;
//    r2 = ResourceFind(res, "C.Pic") ;
*/
    GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;

/*
    r1 = ResourceFind(res, "Game.pal") ;
    ptr = ResourceLock(r1) ;
    GrSetPalette(0, 256, (T_palette *)ptr) ;
    ResourceUnlock(r1) ;
    ResourceUnfind(r1) ;
*/
//puts("Setting palette") ; fflush(stdout) ;
    ViewSetPalette(VIEW_PALETTE_STANDARD) ;

//puts("Server init") ; fflush(stdout) ;
    ServerInit() ;

//puts("Inventory init"); fflush (stdout);
    InventoryInit() ;

//puts("Stats init");fflush(stdout);
    StatsInit(); /* Init player statistics */

//puts("Client Init Mouse And Color") ; fflush(stdout) ;
    ClientInitMouseAndColor ();

//puts("Player INit First") ; fflush(stdout) ;
    PlayerInitFirst() ;

    SoundSetBackgroundMusic("TITLE") ;

    MathInitialize(VIEW3D_WIDTH) ;

    /* initialize player effects */
//puts("Effect init") ;fflush (stdout);
    EffectInit();
    /* New calls go here. */
#endif

    PeopleHereInitialize() ;


//    G_musicChannel=SoundPlayLoopByNumber(1501,255);

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateGameEnd
 *-------------------------------------------------------------------------*/
/**
 *  UpdateGameEnd is the last routine called before the game is shut
 *  down.  Do your deinitialization here.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateGameEnd(T_void)
{
    DebugRoutine("UpdateGameEnd") ;

    PeopleHereFinish() ;

#ifndef SERVER_ONLY
//printf("Largest memory block: %u\n", FreeMemory()) ;
//    printf("MemAllocated: %ld\n", MemGetAllocated()) ;
//    printf("MemMaxAllocated: %ld\n", MemGetMaxAllocated()) ;


    /* New calls go here. */
    EffectFinish() ;
    InventoryFinish() ;

    /* write out the character stats */
//    StatsSaveCharacter (StatsGetActive());
    MathFinish() ;

/**/
//    HangUp() ;
    MouseHide() ;
    CmdQFinish() ;
//    CommCloseAll() ;
///    MemFlushDiscardable() ;

/**/
    /* Turn off the ticker as soon as possible to make sure if */
    /* we crash, the clock will return back to normal. */
    TickerOff() ;
    GrGraphicsOff() ;
    SoundFinish() ;
/**/

    ResourceUnlock(G_r_font) ;
    ResourceUnfind(G_r_font) ;

    ResourceUnlock(G_r_font2) ;
    ResourceUnfind(G_r_font2) ;

    ResourceClose(G_res) ;

//    PlayerFinish();
    MapUnload() ;
    ClientFinish ();
    ServerFinish() ;
    ViewFinish() ;
///    MemFlushDiscardable() ;
    ScriptFinish() ;
    MouseFinish() ;
#ifndef SERVER_ONLY
    ColorizeFinish() ;
#endif
    View3dCheckObjectListEmpty() ;
    PicturesFinish() ;
    KeyboardOff() ;
///    MemFlushDiscardable() ;

#endif
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateMapBegin
 *-------------------------------------------------------------------------*/
/**
 *  UpdateMapBegin is called immediately after a map has been loaded.
 *  Anything that must be initialized just before game play starts is
 *  done here.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateMapBegin(T_void)
{
    DebugRoutine("UpdateMapBegin") ;
    DebugCheck(G_mapBegan == FALSE) ;
/*
    if (G_musicChannel!=-1)
    {
        SoundStop(G_musicChannel);
        G_musicChannel=-1;
    }
*/
    G_mapBegan = TRUE ;
    G_lastFrameTime = TickerGet() ;
    G_lastFiveTime = TickerGet() ;
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
    if (ClientGetCurrentPlace()==1)
    {
        if (StatsPlayerHasNotePage(403)==FALSE)
        {
            EffectSoundOff();
            /* give player initial journal entry */
            StatsAddPlayerNotePage(403);
            EffectSoundOn();
        }
    }
#endif

#ifndef SERVER_ONLY
    /* initialize efx system */
    EfxInit();

#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateMapEnd
 *-------------------------------------------------------------------------*/
/**
 *  UpdateMapEnd is called immediately before the map is unloaded.
 *  Call anything that needs to be destroyed before the map is destroyed.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateMapEnd(T_void)
{
    DebugRoutine("UpdateMapEnd") ;
    DebugCheck(G_mapBegan == TRUE) ;

    G_mapBegan = FALSE ;

    /* close effects system */
    /* New calls go here. */
#ifndef SERVER_ONLY
    EfxFinish();
    PlayerFinish() ;
#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateFrame
 *-------------------------------------------------------------------------*/
/**
 *  UpdateFrame is called once per frame animation of the 3D view.
 *  Anything that is remotedly related to the frame rate ought be called.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateFrame(T_void)
{
    T_word32 delta ;
    T_word32 deltaFive ;
    T_word32 time ;

    DebugRoutine("UpdateFrame") ;
    DebugCheck(G_mapBegan == TRUE) ;

    time = TickerGet() ;
    delta = time - G_lastFrameTime ;
    G_lastFrameTime = delta ;

    /* Don't update if no time has gone by. */
    if (delta)  {

        /* Put new routines here. */

        /* update effects in process */
#ifndef SERVER_ONLY
        EffectUpdateAllPlayerEffects();
        EfxUpdate();
#endif
    }


    /* ***************** Update the Every five counter. */
    deltaFive = time - G_lastFiveTime ;
    if (deltaFive >= (350 /* 5 seconds */))  {
        /* Record the time corrected for the remainder. */
        G_lastFiveTime = time - (deltaFive - 350) ;

        /* ***************** */
        /* NO !!! ROUTINES !!! Go here other than */
        /* UpdateEveryFive(). */
        UpdateEveryFive() ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateEveryFive
 *-------------------------------------------------------------------------*/
/**
 *  UpdateEveryFive is called every five seconds while the 3D view is
 *  active.
 *
 *  NOTE: 
 *  If you need something called every five seconds while a form is
 *  on the screen, it does NOT go here.
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateEveryFive(T_void)
{
    DebugRoutine("UpdateEveryFive") ;
    DebugCheck(G_mapBegan == TRUE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UpdateOften
 *-------------------------------------------------------------------------*/
/**
 *  UpdateOften executes any code that needs to be called in a regular
 *  basis (typically as fast as possible).
 *
 *<!-----------------------------------------------------------------------*/
T_void UpdateOften(T_void)
{
    T_word32 delta, time ;
    TICKER_TIME_ROUTINE_PREPARE() ;
    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("UpdateOften") ;

    time = TickerGet() ;
    delta = time - G_lastOftenTime ;
    G_lastOftenTime = time ;

#ifndef SERVER_ONLY
    SoundUpdate() ;
    ColorUpdate(delta) ;
#endif
    /* New routines go here. */

    DebugEnd() ;
    TICKER_TIME_ROUTINE_ENDM("UpdateOften", 500) ;
}

#ifndef SERVER_ONLY
/* 03/22/96 */
T_void UpdateStart3dView(T_void)
{
    DebugRoutine("UpdateStart3dView") ;

    /* Start up the banner. */
    BannerInit();

    /* and add default rune buttons for current character class */
    SpellsInitSpells();

    /* clear the messages */
    MessageClear();

    if (StatsGetPlayerExperience()==0)
    {
        /* Open 'journal to intro banner */
        EffectSoundOff();
        NotesGotoPageID(0);
        BannerOpenForm (BANNER_FORM_JOURNAL);
        EffectSoundOn();
    }

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
    /* If the player is 1st level, advance to level 8 */
    if (StatsGetPlayerExperience()==0)
    {
        EffectSoundOff();
        /* Give the player 8 levels of experience */
        StatsChangePlayerExperience (32000);
        StatsChangePlayerExperience (32000);
        StatsChangePlayerExperience (32000);
        StatsChangePlayerExperience (32000);
//      StatsChangePlayerExperience (32000);
//      StatsChangePlayerExperience (32000);

        /* Heal Player / Give Max Mana */
        StatsSetPlayerHealth (StatsGetPlayerMaxHealth());
        StatsSetPlayerMana (StatsGetPlayerMaxMana());
        BannerStatusBarUpdate();

        EffectSoundOn();
    }
#endif

    DebugEnd() ;
}


/* 03/22/96 */
T_void UpdateEnd3dView(T_void)
{
    DebugRoutine("UpdateEnd3dView") ;

    /* close down the banner */
    BannerFinish() ;

    SpellsFinish();  /* LES */

    DebugEnd() ;
}

#if 0
T_void TestPort(T_void)
{
    T_word16 key ;
    T_word16 c ;

    DebugRoutine("TestPort") ;

    do {
        key = 0 ;
        if (kbhit())  {
            key = _bios_keybrd(0) ;
            CommSendByte(key) ;
        }
        c = CommReadByte() ;
        if (c != 0xFFFF)  {
            printf("%c", c) ;
            fflush(stdout) ;
        }
    } while (key != 0x11b) ;

    DebugEnd() ;
}
#endif
#endif

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  UPDATE.C
 *-------------------------------------------------------------------------*/
