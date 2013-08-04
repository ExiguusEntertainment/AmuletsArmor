/*-------------------------------------------------------------------------*
 * File:  CONFIG.C
 *-------------------------------------------------------------------------*/
/**
 * Loading and saving of the configuration file CONFIG.INI is handled
 * here.  Some global configuration settings also are grouped here.
 *
 * @addtogroup CONFIG
 * @brief CONFIG.INI File
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CONFIG.H"
#include "GENERAL.H"
#include "KEYMAP.H"
#include "MEMORY.H"

#define MEMORY_SOMEWHAT_LOW_LEVEL          ((T_word32)7000000)

#define MEMORY_LOW_LEVEL                   ((T_word32)2400000)
#define MEMORY_LOW_CREATURE_FRAMES         2
#define MEMORY_LOW_PLAYER_FRAMES           2

#define MEMORY_VERY_LOW_LEVEL              ((T_word32)1800000)
#define MEMORY_VERY_LOW_CREATURE_FRAMES    7
#define MEMORY_VERY_LOW_PLAYER_FRAMES      7

#define MEMORY_NOT_ENOUGH_LEVEL            ((T_word32)1200000)

static E_Boolean G_configLoaded = FALSE ;
static T_iniFile G_configINIFile = INIFILE_BAD ;
static E_Boolean G_bobOffFlag = FALSE ;
static E_Boolean G_invertMouseY = FALSE;

T_word32 FreeMemory(T_void) ;

/*-------------------------------------------------------------------------*
 * Routine:  ConfigOpen
 *-------------------------------------------------------------------------*/
/**
 *  ConfigOpen opens up the config file for reading and changes.
 *
 *<!-----------------------------------------------------------------------*/
T_iniFile ConfigOpen(T_void)
{
    DebugRoutine("ConfigOpen") ;
    DebugCheck(G_configINIFile == INIFILE_BAD) ;

    G_configINIFile = INIFileOpen("config.ini") ;
    DebugCheck(G_configINIFile != INIFILE_BAD) ;

    KeyMapInitialize(G_configINIFile) ;

    DebugEnd() ;

    return G_configINIFile;
}

/*-------------------------------------------------------------------------*
 * Routine:  ConfigClose
 *-------------------------------------------------------------------------*/
/**
 *  ConfigClose closes the config file and writes out any changes.
 *
 *<!-----------------------------------------------------------------------*/
T_void ConfigClose(T_void)
{
    DebugRoutine("ConfigClose") ;
    DebugCheck(G_configINIFile != INIFILE_BAD) ;

    if (G_configINIFile != INIFILE_BAD)  {
        KeyMapFinish() ;
        INIFileClose("config.ini", G_configINIFile) ;
        G_configINIFile = INIFILE_BAD;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ConfigLoad
 *-------------------------------------------------------------------------*/
/**
 *  ConfigLoad reads in a variety of variables on how the game is to
 *  operate.  In particular, the GAME.CFG file determines the amount of
 *  graphic detail is to be shown.
 *
 *<!-----------------------------------------------------------------------*/
T_void ConfigLoad(T_void)
{
    T_word32 memFree ;
    T_word16 neededResolution ;
    T_byte8 *p_stepdown ;
    T_word16 stepdown = 0 ;

    DebugRoutine("ConfigLoad") ;
    DebugCheck(G_configLoaded == FALSE) ;

    /* Get the amount of free memory as soon as we can. */
    memFree = FreeMemory() ;

    if (memFree < MEMORY_SOMEWHAT_LOW_LEVEL)  {
    /* If you are low on memory, cut down the frames per player */
        ObjTypeDeclareSomewhatLowOnMemory() ;
        puts("Lowering piecewise animations") ;
    }
    /* If you are low on memory, cut down the frames per player and creature */
    if (memFree < MEMORY_NOT_ENOUGH_LEVEL)  {
        puts("NOT ENOUGH MEMORY!!!\n") ;
        puts("Sorry, you do not have enough memory available in either conventional") ;
        puts("or expanded memory.  Please do not use SMARTDRV if you are using it.") ;
        puts("Please remove any unneeded TSR's or memory eating devices.  You must") ;
        puts("have at least 2500K free of expanded memory to run this program, ") ;
        puts("and you should have at least 3000K to run it adequately.\n") ;
        puts("PROGRAM ABORTED") ;
        exit(1) ;
    } else if (memFree < MEMORY_VERY_LOW_LEVEL)  {
        puts("VERY LOW memory system!  Lowering graphical resolution ALOT.") ;
        neededResolution = 2 ;
    } else if (memFree < MEMORY_LOW_LEVEL)  {
        puts("LOW memory system!  Lowering graphical resolution.") ;
        neededResolution = 1 ;
    } else {
        puts("Ahhh ... plenty of memory to run in.") ;
        neededResolution = 0 ;
    }
    p_stepdown = INIFileGet(G_configINIFile, "engine", "stepdown") ;
//printf("INI got: '%s'\n", p_stepdown) ;
    if (p_stepdown)  {
        stepdown = sscanf(p_stepdown, "%d", &stepdown) ;
stepdown = (*p_stepdown) - '0' ;
//printf("stepdown = %d\n", stepdown) ;
    } else  {
        stepdown = 0 ;
    }

    if (neededResolution < stepdown)
        neededResolution = stepdown ;

//printf("Needed resolution: %d (%d)\n", neededResolution, stepdown) ;
    ObjTypesSetResolution(neededResolution) ;

    G_configLoaded = TRUE ;

    ConfigReadOptions(G_configINIFile);

    DebugEnd() ;
}

T_iniFile ConfigGetINIFile(T_void)
{
    return G_configINIFile ;
}

E_musicType ConfigGetMusicType(T_void)
{
    E_musicType type ;

    DebugRoutine("ConfigGetMusicType") ;

    type = atoi(INIFileGet(G_configINIFile, "options", "musicType")) ;

    DebugCheck(type < MUSIC_TYPE_UNKNOWN) ;

    /* If a strange type, then don't do it. */
    if (type >= MUSIC_TYPE_UNKNOWN)
        type = MUSIC_TYPE_NONE ;

    DebugEnd() ;

    return type ;
}

T_byte8 ConfigGetCDROMDrive(T_void)
{
    return *INIFileGet(G_configINIFile, "cdrom", "drive") ;
}

E_Boolean ConfigGetBobOffFlag(T_void)
{
    return G_bobOffFlag ;
}

E_Boolean ConfigGetInvertMouseY(T_void)
{
    return G_invertMouseY;
}

void ConfigReadOptions(T_iniFile iniFile)
{
    char *p_value;

    DebugRoutine("ConfigReadOptions");

    /* Load the bob off flag */
    p_value = INIFileGet(iniFile, "options", "boboff");
    if (p_value) {
        if (atoi(p_value)==1)  {
            G_bobOffFlag = TRUE ;
        } else {
            G_bobOffFlag = FALSE ;
        }
    } else {
        G_bobOffFlag = FALSE ;
    }

    p_value = INIFileGet(iniFile, "options", "invertmousey");
    if (p_value) {
        if (atoi(p_value)==1)  {
            G_invertMouseY = TRUE ;
        } else {
            G_invertMouseY = FALSE ;
        }
    } else {
        G_invertMouseY = FALSE ;
    }

    DebugEnd();
}
/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CONFIG.C
 *-------------------------------------------------------------------------*/
