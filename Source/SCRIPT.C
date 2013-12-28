/*-------------------------------------------------------------------------*
 * File:  SCRIPT.C
 *-------------------------------------------------------------------------*/
/**
 * All scripts on maps go through here.  The script system is fairly
 * simple and close to something like assembly language.
 * Each subroutine in a script is given a number and you only have
 * comments in the code to keep track of which is which.  Triggers numbers
 * on maps are used to call each script.
 *
 * @addtogroup SCRIPT
 * @brief Script System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_TRIG.H"
#include "AREASND.H"
#include "CLIENT.H"
#include "CRELOGIC.H"
#include "DOOR.H"
#include "EFFECT.H"
#include "FILE.H"
#include "MAP.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "PLAYER.H"
#include "SCHEDULE.H"
#include "SCRFORM.H"
#include "SCRIPT.H"
#include "SERVER.H"
#include "SLIDER.H"
#include "SOUND.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "VIEWFILE.H"

#define SCRIPT_TAG             (*((T_word32 *)"SpT"))
#define SCRIPT_TAG_BAD         (*((T_word32 *)"sBd"))
#define SCRIPT_TAG_DISCARDABLE (*((T_word32 *)"DsP"))

#define SCRIPT_INSTANCE_TAG             (*((T_word32 *)"SiT"))
#define SCRIPT_INSTANCE_TAG_BAD         (*((T_word32 *)"sIb"))

#define SCRIPT_MAX_STRING 80

#define OBJECT_SCRIPT_ATTR_X                   0
#define OBJECT_SCRIPT_ATTR_Y                   1
#define OBJECT_SCRIPT_ATTR_Z                   2
#define OBJECT_SCRIPT_ATTR_MAX_VELOCITY        3
#define OBJECT_SCRIPT_ATTR_STANCE              4
#define OBJECT_SCRIPT_ATTR_WAS_BLOCKED         5
#define OBJECT_SCRIPT_ATTR_X16                 6
#define OBJECT_SCRIPT_ATTR_Y16                 7
#define OBJECT_SCRIPT_ATTR_Z16                 8
#define OBJECT_SCRIPT_ATTR_ANGLE               9
#define OBJECT_SCRIPT_ATTR_UNKNOWN             10

typedef struct {
    T_byte8 length ;
    T_byte8 data[SCRIPT_MAX_STRING] ;
} T_scriptString ;

typedef union {
    T_sword32 number ;
    T_scriptString *p_string ;
} T_scriptNumberOrString ;

typedef struct {
    E_scriptDataType type ;
    T_scriptNumberOrString ns ;
} T_scriptDataItem ;

typedef struct T_scriptHeader_ {
    T_word16 highestEvent ;            /* Number of events in this script */
                                       /* that might be handled. */
    T_word16 highestPlace ;            /* Number of places in this script */
                                       /* that might be handled. */
    T_word32 sizeCode ;                /* size of code. */
    T_word32 reserved[6] ;             /* Reserved for future use. */
    T_word32 number ;                  /* Script number to identify it. */
    T_word32 tag ;                     /* Tag to tell its memory state. */
    struct T_scriptHeader_ *p_next ;          /* Pointer to next script. */
    struct T_scriptHeader_ *p_prev ;          /* Pointer to previous script. */
    T_word32 lockCount ;               /* Number of users of this script. */
                                       /* A lock count of zero means that */
                                       /* the script is discardable. */
    T_byte8 *p_code ;                  /* Pointer to code area. */
    T_word16 *p_events ;               /* Pointer to events list. */
    T_word16 *p_places ;               /* Pointer to places list. */
} T_scriptHeader ;

typedef struct {
    T_word32 instanceTag ;
    T_scriptHeader *p_header ;
    T_word32 owner ;
             /* Identifier of owner (may be pointer) */

    T_scriptDataItem vars[256] ;
} T_scriptInstance ;

typedef struct {
    T_scriptHeader *p_script ;
    T_word16 position ;
} T_continueData ;

typedef T_word16 (*T_scriptCommand)(
                          T_scriptHeader *script,
                          T_word16 position) ;

/* Accessor functions/macros. */
#define ScriptGetPrevious(p_script)  ((p_script)->p_prev)
#define ScriptGetNext(p_script)      ((p_script)->p_next)
#define ScriptGetLockCount(p_script) ((p_script)->lockCount)
#define ScriptGetCode(p_script)      ((p_script)->p_code)
#define ScriptGetPlaces(p_script)    ((p_script)->p_places)
#define ScriptGetEvents(p_script)    ((p_script)->p_events)
#define ScriptGetSizeCode(p_script)  ((p_script)->sizeCode)
#define ScriptGetHighestEvent(p_script)  ((p_script)->highestEvent)
#define ScriptGetHighestPlace(p_script)  ((p_script)->highestPlace)
#define ScriptGetTag(p_script)           ((p_script)->tag)
#define ScriptGetNumber(p_script)        ((p_script)->number)
#define ScriptGetEventPosition(p_script, eventNum)   \
            ((p_script)->p_events[(eventNum)])
#define ScriptGetPlacePosition(p_script, placeNum)   \
            ((p_script)->p_places[(placeNum)])
#define ScriptGetCodeByte(p_script, codeIndex)   \
            ((p_script)->p_code[(codeIndex)])
#define ScriptGetCodeWord(p_script, codeIndex)   \
            (*((T_word16 *)(&((p_script)->p_code[(codeIndex)]))))

#define ScriptInstanceGetHeader(p_inst)  ((p_inst)->p_header)
#define ScriptInstanceGetTag(p_inst)     ((p_inst)->instanceTag)
#define ScriptInstanceGetOwner(p_inst)     ((p_inst)->owner)

#define ScriptSetPrevious(p_script, prev)  \
            (((p_script)->p_prev) = (prev))
#define ScriptSetNext(p_script, next)      \
            (((p_script)->p_next) = (next))
#define ScriptSetLockCount(p_script, newLockCount) \
            (((p_script)->lockCount) = (newLockCount))
#define ScriptSetCode(p_script, code)          \
            (((p_script)->p_code) = (code))
#define ScriptSetPlaces(p_script, places)      \
            (((p_script)->p_places) = (places))
#define ScriptSetEvents(p_script, events)      \
            (((p_script)->p_events) = (events))
#define ScriptSetTag(p_script, newTag)              \
            (((p_script)->tag) = (newTag))
#define ScriptSetNumber(p_script, newnumber)              \
            (((p_script)->number) = (newnumber))

#define ScriptSetFirst(p_first) (G_firstScript = (p_first))
#define ScriptGetFirst() (G_firstScript)

#define ScriptIsInitialized()       (G_scriptInit)
#define ScriptMakeInitialized()     (G_scriptInit = TRUE)
#define ScriptMakeNotInitialized()  (G_scriptInit = FALSE)

#define ScriptInstanceSetHeader(p_inst, header)  \
            (((p_inst)->p_header) = (header))
#define ScriptInstanceSetTag(p_instance, tag)  \
            (((p_instance)->instanceTag) = (tag))
#define ScriptInstanceSetOwner(p_instance, newOwner)  \
            (((p_instance)->owner) = (newOwner))

#define ScriptHandleToInstance(p_handle) \
            ((T_scriptInstance *)p_handle)
#define ScriptInstanceToHandle(p_instance) \
            ((T_script)p_instance)

/* Internal prototype: */
static T_void IDestroyScriptList(T_void) ;
static T_void IRemoveScriptFromList(T_scriptHeader *p_script) ;
static T_void IDestroyScript(T_scriptHeader *p_script) ;
static T_scriptHeader *IFindScriptByNumber(T_word32 number) ;
static T_void IReclaimScript(T_scriptHeader *p_script) ;
static T_script IScriptInstantiate(T_scriptHeader *p_script) ;
static T_scriptHeader *IScriptLoad(T_word32 number) ;
static T_void IScriptMakeDiscardable(T_scriptHeader *p_script) ;
static T_void IDestroyScriptInstance(T_scriptInstance *p_instance) ;
static T_void IMemoryRequestDiscardScript(T_void *p_block) ;
static T_word16 IExecuteCode(T_scriptHeader *script, T_word16 position) ;
static T_scriptDataItem *ILookupVariable(
                            T_scriptHeader *p_script,
                            T_word16 varNumber) ;
static T_scriptDataItem *IScriptGetVariable(
                            T_scriptHeader *p_script,
                            T_word16 *position) ;
static T_void ICopyData(
                  T_scriptDataItem *dest,
                  T_scriptDataItem *source) ;
static T_void IPascalToCString(T_byte8 *p_cstring, T_scriptString *p_pstring) ;
static T_word16 IGetPlace(
                    T_scriptHeader *p_script,
                    T_scriptDataItem *p_value) ;
static T_sword16 IPascalStringCompare(
                     T_scriptString *p_string1,
                     T_scriptString *p_string2) ;

static T_word16 ICommandSet(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandPrint(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandIf(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGoto(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandAdd(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSubtract(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandMuliply(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDivide(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandIncrement(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDecrement(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandCompare(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSound(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandChangeSideTexture(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectSetType(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandTeleport(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDoorCycle(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDoorLock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDoorUnlock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandAreaSound(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGotoPlace(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDelay(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSlideFloor(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSlideCeiling(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGosub(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandRandom(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectSound(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectSet(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectGet(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandError(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandLookForPlayer(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandIfNot(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectGetAngleToObject(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandAbsolute(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandClear(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandNegate(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectDistanceToObject(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectAccelForward(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectDamageForward(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSubtract16(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandTextBoxSetSelection(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandObjectShootObject(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandPlayerObjectGet(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGetFloorHeight(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGetCeilingHeight(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDoorIncreaseLock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDoorDecreaseLock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSideState(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandWallState(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSectorState(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGivePlayerXP(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGiveAllPlayersXP(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandEffect(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSectorSetLight(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGenerateMissile(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandPlayerHasItem(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandIsEffectActive(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandActivateGenerator(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandDeactiveGenerator(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandGroupState(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandBlock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandUnblock(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandToggleSwitch(T_scriptHeader *script, T_word16 position) ;
static T_word16 ICommandSlideFloorNice(
                    T_scriptHeader *script,
                    T_word16 position) ;
static T_word16 ICommandSlideCeilingNice(
                    T_scriptHeader *script,
                    T_word16 position) ;
static T_word16 ICommandJournalEntry(T_scriptHeader *script, T_word16 position) ;

static T_void IContinueExecution(T_word32 data) ;

static T_sliderResponse IHandleSlidingCeiling(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;

static T_sliderResponse IHandleSlidingFloor(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;

static T_sliderResponse IHandleSlidingCeilingNice(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;

static T_sliderResponse IHandleSlidingFloorNice(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;

/* Global variables. */
static T_scriptHeader *G_firstScript = NULL ;
static E_Boolean G_scriptInit = FALSE ;

static E_scriptDataType G_parameter1Type ;
static T_void *         G_parameter1Data ;
static E_scriptDataType G_parameter2Type ;
static T_void *         G_parameter2Data ;
static E_scriptDataType G_parameter3Type ;
static T_void *         G_parameter3Data ;

static T_scriptDataItem G_systemFlags[SCRIPT_FLAG_UNKNOWN] ;
static T_scriptDataItem G_systemVars[2] ;

static E_Boolean G_pleaseStop = FALSE ;

#define SYSTEM_VAR_SELF          0
#define SYSTEM_VAR_TIME          1

/* Current instance being processed. */
static T_scriptInstance *G_instance ;

#define NUM_SCRIPT_COMMANDS       66
static T_scriptCommand G_commands[NUM_SCRIPT_COMMANDS] = {
    NULL,                            /* 0 Return */
    ICommandSet,                     /* 1 */
    ICommandPrint,                   /* 2 */
    ICommandIf,                      /* 3 */
    ICommandGoto,                    /* 4 */
    ICommandAdd,                     /* 5 */
    ICommandSubtract,                /* 6 */
    ICommandMuliply,                 /* 7 */
    ICommandDivide,                  /* 8 */
    ICommandIncrement,               /* 9 */
    ICommandDecrement,               /* 10 */
    ICommandCompare,                 /* 11 */
    ICommandSound,                   /* 12 */
    ICommandChangeSideTexture,       /* 13 */
    ICommandObjectSetType,           /* 14 */
    ICommandTeleport,                /* 15 */
    ICommandDoorCycle,               /* 16 */
    ICommandDoorLock,                /* 17 */
    ICommandDoorUnlock,              /* 18 */
    ICommandAreaSound,               /* 19 */
    ICommandGotoPlace,               /* 20 */
    ICommandDelay,                   /* 21 */
    ICommandSlideFloor,              /* 22 */
    ICommandSlideCeiling,            /* 23 */
    ICommandGosub,                   /* 24 */
    ICommandRandom,                  /* 25 */
    ICommandObjectSound,             /* 26 */
    ICommandObjectSet,               /* 27 */
    ICommandObjectGet,               /* 28 */
    ICommandError,                   /* 29 */
    ICommandLookForPlayer,           /* 30 */
    ICommandIfNot,                   /* 31 */
    ICommandObjectGetAngleToObject,  /* 32 */
    ICommandAbsolute,                /* 33 */
    ICommandClear,                   /* 34 */
    ICommandNegate,                  /* 35 */
    ICommandObjectDistanceToObject,  /* 36 */
    ICommandObjectAccelForward,      /* 37 */
    ICommandObjectDamageForward,     /* 38 */
    ICommandSubtract16,              /* 39 */
    ICommandTextBoxSetSelection,     /* 40 */
    ICommandObjectShootObject,       /* 41 */
    ICommandPlayerObjectGet,         /* 42 */
    ICommandGetFloorHeight,          /* 43 */
    ICommandGetCeilingHeight,        /* 44 */
    ICommandDoorIncreaseLock,        /* 45 */
    ICommandDoorDecreaseLock,        /* 46 */

    ICommandSideState,               /* 47 */
    ICommandWallState,               /* 48 */
    ICommandSectorState,             /* 49 */
    ICommandGivePlayerXP,            /* 50 */
    ICommandGiveAllPlayersXP,        /* 51 */
    ICommandEffect,                  /* 52 */
    ICommandSectorSetLight,          /* 53 */
    ICommandGenerateMissile,         /* 54 */
    ICommandPlayerHasItem,           /* 55 */
    ICommandIsEffectActive,          /* 56 */
    ICommandActivateGenerator,       /* 57 */
    ICommandDeactiveGenerator,       /* 58 */
    ICommandGroupState,              /* 59 */
    ICommandBlock,                   /* 60 */
    ICommandUnblock,                 /* 61 */
    ICommandToggleSwitch,            /* 62 */
    ICommandSlideFloorNice,          /* 63 */
    ICommandSlideCeilingNice,        /* 64 */
    ICommandJournalEntry             /* 65 */
} ;

/*-------------------------------------------------------------------------*
 * Routine:  ScriptInitialize
 *-------------------------------------------------------------------------*/
/**
 *  ScriptInitialize sets up the structures necessary to do all future
 *  script accesses.  Call this once at the beginning of the program.
 *
 *<!-----------------------------------------------------------------------*/
T_void ScriptInitialize(T_void)
{
    T_word16 i ;

    DebugRoutine("ScriptInitialize") ;
    DebugCheck(!ScriptIsInitialized()) ;

    if (!G_scriptInit)  {
        ScriptSetFirst(NULL) ;
        ScriptMakeInitialized() ;

        for (i=0; i<SCRIPT_FLAG_UNKNOWN; i++)  {
            G_systemFlags[i].ns.number = 0 ;
            G_systemFlags[i].type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptFinish
 *-------------------------------------------------------------------------*/
/**
 *  ScriptFinish closes out the scripting system.  All memory and
 *  variables must be returned to their normal state.
 *
 *  NOTE: 
 *  You MUST unlock all scripts before calling this command.
 *
 *<!-----------------------------------------------------------------------*/
T_void ScriptFinish(T_void)
{
    DebugRoutine("ScriptFinish") ;
    DebugCheck(ScriptIsInitialized()) ;

    if (G_scriptInit)  {
        IDestroyScriptList() ;
        ScriptMakeNotInitialized() ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptLock
 *-------------------------------------------------------------------------*/
/**
 *  ScriptLock loads a script file from disk and creates a handle to the
 *  script.  If the script is already in memory, a instance is made and
 *  the handle to that instance is returned.  All script commands require
 *  a script instance handle.
 *
 *  NOTE: 
 *  You MUST make a call to ScriptInitialize before using this routine.
 *
 *  @param number -- Number of script to lock.
 *
 *<!-----------------------------------------------------------------------*/
T_script ScriptLock(T_word32 number)
{
    T_scriptHeader *p_script ;
    T_word32 lockCount ;
    T_script script = SCRIPT_BAD ;

    DebugRoutine("ScriptLock") ;
    DebugCheck(ScriptIsInitialized()) ;

    /* First, try to find the script by its number. */
    p_script = IFindScriptByNumber(number) ;

    /* Did we find it. */
    if (p_script)  {
        /* Script was found. */
        /* Up its link number. */
        lockCount = ScriptGetLockCount(p_script) ;
        if (lockCount == 0)
            IReclaimScript(p_script) ;
        lockCount++ ;
        ScriptSetLockCount(p_script, lockCount) ;
        script = IScriptInstantiate(p_script) ;
    } else {
        /* Not found. */
        /* We'll have to load the script. */
        p_script = IScriptLoad(number) ;

        /* Append script to beginning of the list. */
        ScriptSetNext(p_script, ScriptGetFirst()) ;
        ScriptSetFirst(p_script) ;
        if (p_script)
            script = IScriptInstantiate(p_script) ;
    }

    DebugEnd() ;

    /* Return the found/created script instance handle. */
    return script ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptUnlock
 *-------------------------------------------------------------------------*/
/**
 *  ScriptUnlock releases a script that was previously locked.  If
 *  possible, the script code will try to stay around as long as possible
 *  to keep from do excess disk accesses.
 *
 *  NOTE: 
 *  For each ScriptUnlock you do, you must have already done just as many
 *  ScriptLocks.
 *
 *  @param script -- Previously locked script.
 *
 *<!-----------------------------------------------------------------------*/
T_void ScriptUnlock(T_script script)
{
    T_scriptInstance *p_instance ;
    T_scriptHeader *p_script ;
    T_word32 lockCount ;

    DebugRoutine("ScriptUnlock") ;
    DebugCheck(ScriptIsInitialized()) ;
    DebugCheck(script != SCRIPT_BAD) ;

    /* Get the instance data for this instance. */
    p_instance = ScriptHandleToInstance(script) ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    /* Get the script that goes with this instance. */
    p_script = ScriptInstanceGetHeader(p_instance) ;
    DebugCheck(p_script != NULL) ;
    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG) ;

    /* Lower the lock count for the script */
    lockCount = ScriptGetLockCount(p_script) ;
    DebugCheck((lockCount > 0) && (lockCount < 0x10000)) ;
    lockCount-- ;
    ScriptSetLockCount(p_script, lockCount) ;

    /* If the count goes to zero, make the script discardable. */
//    if (lockCount == 0)
//        IScriptMakeDiscardable(p_script) ;
    IRemoveScriptFromList(p_script) ;
    MemFree(p_script) ;

    /* Now destroy the instance for this script. */
    IDestroyScriptInstance(p_instance) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDestroyScriptList
 *-------------------------------------------------------------------------*/
/**
 *  IDestroyScriptList goes through the list of scripts and destroys each
 *  of them.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDestroyScriptList(T_void)
{
    DebugRoutine("IDestroyScriptList") ;

    while (ScriptGetFirst())
        IDestroyScript(ScriptGetFirst()) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDestroyScript
 *-------------------------------------------------------------------------*/
/**
 *  IDestroyScript removes a script from the script list and removes it
 *  from memory.
 *
 *  @param p_script -- script to destroy
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDestroyScript(T_scriptHeader *p_script)
{
    DebugRoutine("IDestroyScript") ;
    DebugCheck(p_script != NULL) ;
    DebugCheck(ScriptGetLockCount(p_script) == 0) ;
    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG_DISCARDABLE) ;

    /* Get it off the list. */
    IRemoveScriptFromList(p_script) ;

    /* Before truly freeing it, mark it as gone in case any body */
    /* still is illegally pointing to it. */
    ScriptSetTag(p_script, SCRIPT_TAG_BAD) ;

    /* Free the script from memory. */
    /* NOTE:  This may change latter to support resource files */
    /* (ResourceUnlock for instance). */
    MemReclaimDiscardable(p_script) ;
    MemFree(p_script) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IRemoveScriptFromList
 *-------------------------------------------------------------------------*/
/**
 *  IRemoveScriptFromList unlinks the script from the list of currently
 *  available scripts.
 *
 *  @param p_script -- script to remove from script list
 *
 *<!-----------------------------------------------------------------------*/
static T_void IRemoveScriptFromList(T_scriptHeader *p_script)
{
    T_scriptHeader *p_prev ;
    T_scriptHeader *p_next ;

    DebugRoutine("IRemoveScriptFromList") ;
    DebugCheck(p_script != NULL) ;
    DebugCheck((ScriptGetTag(p_script) == SCRIPT_TAG) ||
               (ScriptGetTag(p_script) == SCRIPT_TAG_DISCARDABLE)) ;

    /* Get the links we are about to delete. */
    p_prev = ScriptGetPrevious(p_script) ;
    p_next = ScriptGetNext(p_script) ;

    /* Check to see if there was a previous link. */
    if (p_prev)  {
        /* Previous link exists. */
        /* Make who is previous point its next to our next. */
        ScriptSetNext(p_prev, p_next) ;
    } else {
        /* No previous. */
        /* Make the first item our next item. */
        ScriptSetFirst(p_next) ;
    }

    /* Is there a next item? */
    if (p_next)  {
        /* Yes, there is a next item. */
        /* Point it to our previous. */
        ScriptSetPrevious(p_next, p_prev) ;
    }

    /* Declare this script as unlinked. */
    ScriptSetNext(p_script, NULL) ;
    ScriptSetPrevious(p_script, NULL) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IFindScriptByNumber
 *-------------------------------------------------------------------------*/
/**
 *  IFindScriptByNumber searches the list of scripts to find a matching
 *  number.  If found, a pointer to the script is returned, else NULL.
 *
 *  @param number -- Number of script to find
 *
 *  @return Found script, else NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_scriptHeader *IFindScriptByNumber(T_word32 number)
{
    T_scriptHeader *p_foundScript ;

    DebugRoutine("IFindScriptByNumber") ;

    p_foundScript = ScriptGetFirst() ;
    while (p_foundScript)  {
        if (ScriptGetNumber(p_foundScript) == number)
            break ;
        p_foundScript = ScriptGetNext(p_foundScript) ;
    }

    DebugEnd() ;

    return p_foundScript ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IReclaimScript
 *-------------------------------------------------------------------------*/
/**
 *  IReclaimScript takes a previous script declared to be discardable
 *  and makes it non-discardable.
 *
 *  @param p_script -- Script to reclaim
 *
 *<!-----------------------------------------------------------------------*/
static T_void IReclaimScript(T_scriptHeader *p_script)
{
    DebugRoutine("IReclaimScript") ;
    DebugCheck(p_script != NULL) ;
    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG_DISCARDABLE) ;

    /* Take it back. */
    MemReclaimDiscardable(p_script) ;

    /* Mark it as reclaimed (aka Normal tag). */
    ScriptSetTag(p_script, SCRIPT_TAG) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IScriptInstantiate
 *-------------------------------------------------------------------------*/
/**
 *  IScriptInstantiate makes a new script instance out of the given
 *  script item.  All data is initialized, too.
 *
 *  @param p_script -- Script to make instance of
 *
 *  @return Handle to new script instance
 *
 *<!-----------------------------------------------------------------------*/
static T_script IScriptInstantiate(T_scriptHeader *p_script)
{
    T_scriptInstance *p_instance ;

    DebugRoutine("IScriptInstantiate") ;
    DebugCheck(p_script != NULL) ;
    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG) ;

    /* Allocate a new instance structure. */
    p_instance = MemAlloc(sizeof(T_scriptInstance)) ;
    DebugCheck(p_instance != NULL) ;

    if (p_instance)  {
        /* Clear the structure. */
        memset(p_instance, 0, sizeof(T_scriptInstance)) ;

        /* Set up the data fields. */
        ScriptInstanceSetHeader(p_instance, p_script) ;
        ScriptInstanceSetTag(p_instance, SCRIPT_INSTANCE_TAG) ;
    }

    DebugEnd() ;

    return (ScriptInstanceToHandle(p_instance)) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IScriptLoad
 *-------------------------------------------------------------------------*/
/**
 *  IScriptLoad brings in a new script from disk and initializes the
 *  script data (as needed).
 *
 *  NOTE: 
 *  Debugging version will bomb if the script is not found.
 *
 *  @param number -- Number of script to load
 *
 *  @return Loaded script, or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_scriptHeader *IScriptLoad(T_word32 number)
{
    T_byte8 filename[40] ;
    T_word32 size ;
    T_byte8 *p_loaded ;
    T_scriptHeader *p_script ;
    T_byte8 *p_data ;

    DebugRoutine("IScriptLoad") ;

    /* Create the script name. */
    sprintf((char *)filename, "S%ld.SRP", number) ;

    /* Load the script. */
    p_loaded = (T_byte8 *)FileLoad(filename, &size) ;
    p_script = (T_scriptHeader *)p_loaded ;

    /* Bomb if we didn't load it. */
    DebugCheck(p_script != NULL) ;

    /* Did it load? */
    if (p_script)  {
        /* It did load. */
        /* Initialize it as best as we can. */
        ScriptSetNumber(p_script, number) ;
        ScriptSetTag(p_script, SCRIPT_TAG) ;
        ScriptSetNext(p_script, NULL) ;
        ScriptSetPrevious(p_script, NULL) ;
        ScriptSetLockCount(p_script, 1) ;

        /* Code is right after the header. */
        p_data = (T_byte8 *)(p_script+1) ;
        ScriptSetCode(p_script, p_data) ;

        /* Events are after code. */
        p_data += ScriptGetSizeCode(p_script) ;
        ScriptSetEvents(p_script, (T_word16 *)p_data) ;

        /* Places are after events. */
        p_data += ScriptGetHighestEvent(p_script) * sizeof(T_word16) ;
        ScriptSetPlaces(p_script, (T_word16 *)p_data) ;

        /* That should do it. */
    }

    DebugEnd() ;

    /* Return what we have. */
    return p_script ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IScriptMakeDiscardable
 *-------------------------------------------------------------------------*/
/**
 *  IScriptMakeDiscardable puts a script on the discardable list.
 *
 *  @param  -- Script to make discardable
 *
 *<!-----------------------------------------------------------------------*/
static T_void IScriptMakeDiscardable(T_scriptHeader *p_script)
{
    DebugRoutine("IScriptMakeDiscardable") ;
    DebugCheck(p_script != NULL) ;
    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG) ;
    DebugCheck(ScriptGetLockCount(p_script) == 0) ;

    /* Set up the tag. */
    ScriptSetTag(p_script, SCRIPT_TAG_DISCARDABLE) ;

    /* Tell the memory manager that the script can be deleted. */
    MemMarkDiscardable(p_script, IMemoryRequestDiscardScript) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDestroyScriptInstance
 *-------------------------------------------------------------------------*/
/**
 *  IDestroyScriptInstance gets rid of the instance data that goes with
 *  a script.  All links have been severed before this routine.
 *
 *  @param  -- Pointer to script instance
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDestroyScriptInstance(T_scriptInstance *p_instance)
{
    T_word16 i ;

    DebugRoutine("IDestroyScriptInstance") ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    /* Tag this instance as bad before we free it. */
    ScriptInstanceSetTag(p_instance, SCRIPT_INSTANCE_TAG_BAD) ;

    /* Make sure all string data is destroyed. */
    for (i=0; i<256; i++)
        if (p_instance->vars[i].type == SCRIPT_DATA_TYPE_STRING)
            MemFree(p_instance->vars[i].ns.p_string) ;

    /* Since there is no other attachments to the instance, just free it. */
    MemFree(p_instance) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IMemoryRequestDiscardScript
 *-------------------------------------------------------------------------*/
/**
 *  IMemoryRequestDiscardScript is called when the memory manager needs
 *  to free a script to make room for other things.  Basically this routine
 *  just unlinks the script from the list to allow it to leave.
 *
 *  @param p_block -- Pointer to data that is script to free
 *
 *<!-----------------------------------------------------------------------*/
static T_void IMemoryRequestDiscardScript(T_void *p_block)
{
    T_scriptHeader *p_script ;

    DebugRoutine("IMemoryRequestDiscardScript") ;
    DebugCheck(p_block != NULL) ;

    /* Get the script pointer. */
    p_script = p_block ;

    DebugCheck(ScriptGetTag(p_script) == SCRIPT_TAG_DISCARDABLE) ;
    DebugCheck(ScriptGetLockCount(p_script) == 0) ;

    /* Just unlink the script. */
    IRemoveScriptFromList(p_script) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ScriptSetOwner
 *-------------------------------------------------------------------------*/
/**
 *  ScriptSetOwner declares the owner of this script.
 *
 *  @param script -- Script to set owner of.
 *  @param owner -- General pointer to owner
 *
 *<!-----------------------------------------------------------------------*/
T_void ScriptSetOwner(T_script script, T_word32 owner)
{
    T_scriptInstance *p_instance ;

    DebugRoutine("ScriptSetOwner") ;
    DebugCheck(ScriptIsInitialized()) ;

    p_instance = ScriptHandleToInstance(script) ;
    DebugCheck(p_instance != NULL) ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    ScriptInstanceSetOwner(p_instance, owner) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptGetOwner
 *-------------------------------------------------------------------------*/
/**
 *  ScriptGetOwner returns the previously stored pointer to the owner of
 *  this script.
 *
 *  @param script -- Script to get owner of.
 *
 *  @return General pointer to owner
 *
 *<!-----------------------------------------------------------------------*/
T_word32 ScriptGetOwner(T_script script)
{
    T_scriptInstance *p_instance ;
    T_word32 owner ;

    DebugRoutine("ScriptGetOwner") ;
    DebugCheck(ScriptIsInitialized()) ;

    p_instance = ScriptHandleToInstance(script) ;
    DebugCheck(p_instance != NULL) ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    owner = ScriptInstanceGetOwner(p_instance) ;

    DebugEnd() ;

    return owner ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptEvent
 *-------------------------------------------------------------------------*/
/**
 *  ScriptEvent runs an event in a script file.  If the script has no
 *  event for the script, a FALSE code is returned.
 *
 *  NOTE: 
 *  For each ScriptUnlock you do, you must have already done just as many
 *  ScriptLocks.
 *
 *  @param script -- Script Instance to execute event
 *  @param eventNumber -- Number of event to execute
 *  @param type1 -- Type of parameter 1
 *  @param p_data1 -- Pointer to data parameter 1
 *  @param type2 -- Type of parameter 2
 *  @param p_data2 -- Pointer to data parameter 2
 *  @param type3 -- Type of parameter 3
 *  @param p_data3 -- Pointer to data parameter 3
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ScriptEvent(
              T_script script,
              T_word16 eventNumber,
              E_scriptDataType type1,
              T_void *p_data1,
              E_scriptDataType type2,
              T_void *p_data2,
              E_scriptDataType type3,
              T_void *p_data3)
{
    E_Boolean status = FALSE ;
    T_scriptInstance *p_instance ;
    T_scriptHeader *p_header ;
    T_word16 numEvents ;
    T_word16 exePosition ;

    DebugRoutine("ScriptEvent") ;
    DebugCheck(ScriptIsInitialized()) ;

    G_instance = p_instance = ScriptHandleToInstance(script) ;
    DebugCheck(p_instance != NULL) ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    /* Get ahold of the header to the actual script. */
    p_header = ScriptInstanceGetHeader(p_instance) ;
    DebugCheck(p_header != NULL) ;
    DebugCheck(ScriptGetTag(p_header) == SCRIPT_TAG) ;

    /* How many events can this script process? */
    numEvents = ScriptGetHighestEvent(p_header) ;

    /* Is the given event number in range? */
    if (eventNumber < numEvents)  {
        /* Yes, we can try to process this event. */
        /* Look the execution position up. */
        exePosition = ScriptGetEventPosition(p_header, eventNumber) ;

        /* Is this a non - position? */
        if (exePosition != 0xFFFF)  {
            /* Is this a valid position. */
            DebugCheck(exePosition < ScriptGetSizeCode(p_header)) ;

            /* 1, 2, and 3. */
            G_parameter1Type = type1 ;
            G_parameter1Data = p_data1 ;
            G_parameter2Type = type2 ;
            G_parameter2Data = p_data2 ;
            G_parameter3Type = type3 ;
            G_parameter3Data = p_data3 ;

            /* Run the code at that position. */
            IExecuteCode(p_header, exePosition) ;

            status = TRUE ;
        }
    }
    /* If no event was processed, oh well, stop caring about it. */

    DebugEnd() ;

    return status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ScriptRunPlace
 *-------------------------------------------------------------------------*/
/**
 *  ScriptRunPlace starts execution of a script file at the given
 *  place marker.
 *
 *  @param script -- Script Instance to execute place
 *  @param placeNumber -- Number of place to execute
 *
 *  @return FALSE if not executed
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ScriptRunPlace(
              T_script script,
              T_word16 placeNumber)
{
    E_Boolean status = FALSE ;
    T_scriptInstance *p_instance ;
    T_scriptHeader *p_header ;
    T_word16 numPlaces ;
    T_word16 exePosition ;

    DebugRoutine("ScriptRunPlace") ;
    DebugCheck(ScriptIsInitialized()) ;

    G_instance = p_instance = ScriptHandleToInstance(script) ;
    DebugCheck(p_instance != NULL) ;
    DebugCheck(ScriptInstanceGetTag(p_instance) == SCRIPT_INSTANCE_TAG) ;

    /* Get ahold of the header to the actual script. */
    p_header = ScriptInstanceGetHeader(p_instance) ;
    DebugCheck(p_header != NULL) ;
    DebugCheck(ScriptGetTag(p_header) == SCRIPT_TAG) ;

    /* How many events can this script process? */
    numPlaces = ScriptGetHighestPlace(p_header) ;
    /* Is the given event number in range? */
    if (placeNumber < numPlaces)  {
        /* Yes, we can try to process this event. */
        /* Look the execution position up. */
        exePosition = ScriptGetPlacePosition(p_header, placeNumber) ;

        /* Is this a non - position? */
        if (exePosition != 0xFFFF)  {
            /* Is this a valid position. */
            DebugCheck(exePosition < ScriptGetSizeCode(p_header)) ;

            /* 1, 2, and 3. */
            G_parameter1Type = SCRIPT_DATA_TYPE_NONE ;
            G_parameter2Type = SCRIPT_DATA_TYPE_NONE ;
            G_parameter3Type = SCRIPT_DATA_TYPE_NONE ;

            /* Run the code at that position. */
            IExecuteCode(p_header, exePosition) ;

            status = TRUE ;
        } else {
#ifndef NDEBUG
            fprintf(stderr, "Cannot execute place %d for script %p\n",
                placeNumber,
                script) ;
            printf("Cannot execute place %d for script %p\n",
                placeNumber,
                script) ;
            DebugCheck(FALSE) ;
#endif
        }
    } else {
#ifndef NDEBUG
        fprintf(stderr, "Cannot execute place %d for script %p\n",
            placeNumber,
            script) ;
        printf("Cannot execute place %d for script %p\n",
            placeNumber,
            script) ;
        DebugCheck(FALSE) ;
#endif
    }

    /* If no event was processed, oh well, stop caring about it. */

    DebugEnd() ;

    return status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IExecuteCode
 *-------------------------------------------------------------------------*/
/**
 *  IExecuteCode will process code at the given location until a return
 *  is reached.
 *
 *  NOTE: 
 *  For each ScriptUnlock you do, you must have already done just as many
 *  ScriptLocks.
 *
 *  @param script -- Script to execute code in
 *  @param exePosition -- execute position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IExecuteCode(T_scriptHeader *script, T_word16 position)
{
    T_byte8 command ;

    DebugRoutine("IExecuteCode") ;

    G_pleaseStop = FALSE ;
    while(G_pleaseStop == FALSE)  {
        command = ScriptGetCodeByte(script, position++) ;
        DebugCheck(command < NUM_SCRIPT_COMMANDS) ;

        if (command == 0)
            break ;

        DebugCheck(G_commands[command]) ;

        position = G_commands[command](script, position) ;

        if (command == 21 /* DELAY */)
            break ;
    }

    /* Make sure no one else stops because of this sub-execution */
    G_pleaseStop = FALSE ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILookupVariable
 *-------------------------------------------------------------------------*/
/**
 *  ILookupVariable finds a pointer to the correct variable based on
 *  the given script and variable number.
 *
 *  NOTE: 
 *  If you attempt to access a variable number that is not allowed, this
 *  routine bombs.
 *
 *  @param script -- Script to find variable in
 *  @param varNumber -- Number of variable
 *
 *<!-----------------------------------------------------------------------*/
static T_scriptDataItem *ILookupVariable(
                            T_scriptHeader *p_script,
                            T_word16 varNumber)
{
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("ILookupVariable") ;
    DebugCheck((varNumber < 256) ||
               ((varNumber >= 32768) && (varNumber <= 32769))) ;

    if (varNumber & 0x8000)  {
        p_var = G_systemVars + (varNumber & 0x7FFF) ;
        switch(varNumber & 0x7FFF)  {
            case SYSTEM_VAR_TIME:
                p_var->ns.number = SyncTimeGet() ;
                p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
                break;
            case SYSTEM_VAR_SELF:
                p_var->ns.number = G_instance->owner ;
                p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
                break ;
        }
    } else {
        /* For now, just do a direct lookup. */
        p_var = G_instance->vars+(varNumber&0x7FFF) ;
    }

    DebugEnd() ;

    return p_var ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IScriptGetVariable
 *-------------------------------------------------------------------------*/
/**
 *  IScriptGetVariable reads in the variable reference in the given code
 *  position and returns a pointer to the variable.
 *
 *  NOTE: 
 *  If you attempt to access a variable number that is not allowed, this
 *  routine bombs.
 *
 *  @param script -- Script to find variable in
 *  @param position -- Position to look for variable in code.
 *
 *  @return Pointer to found variable, or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_scriptDataItem *IScriptGetVariable(
                            T_scriptHeader *p_script,
                            T_word16 *position)
{
    E_scriptDataType type ;
    T_word16 value ;
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("IScriptGetVariable") ;

    type = ScriptGetCodeByte(p_script, (*position)++) ;
    DebugCheck(type == SCRIPT_DATA_TYPE_VARIABLE) ;

    if (type == SCRIPT_DATA_TYPE_VARIABLE)  {
        value = ScriptGetCodeWord(p_script, *position) ;
        (*position) += 2 ;

        p_var = ILookupVariable(p_script, value) ;
    }

    DebugEnd() ;

    return p_var ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IScriptGetAny
 *-------------------------------------------------------------------------*/
/**
 *  IScriptGetAny reads in any type of value reference in the given code
 *  position and returns a pointer to the data type.
 *
 *  NOTE: 
 *  If you attempt to access a variable number that is not allowed, this
 *  routine bombs.
 *
 *  @param script -- Script to find any in
 *  @param position -- Position to look for any in code.
 *
 *  @return Returned value
 *
 *<!-----------------------------------------------------------------------*/
static T_scriptDataItem IScriptGetAny(
                            T_scriptHeader *p_script,
                            T_word16 *position)
{
    E_scriptDataType type ;
    T_word16 value ;
//    T_scriptDataItem var = { SCRIPT_DATA_TYPE_NONE, 0 } ;
    T_scriptDataItem var ;
    T_byte8 *p_data ;
    T_scriptString *p_string ;
    T_sword32 *p_number ;

    DebugRoutine("IScriptGetAny") ;

    type = ScriptGetCodeByte(p_script, (*position)++) ;

    switch(type)  {
        case SCRIPT_DATA_TYPE_EVENT_PARAMETER:
            value = ScriptGetCodeByte(p_script, (*position)++) ;
            switch(value)  {
                case 1:
                    var.type = G_parameter1Type ;
                    var.ns.number = (T_word32)G_parameter1Data ;
                    break ;
                case 2:
                    var.type = G_parameter2Type ;
                    var.ns.number = (T_word32)G_parameter2Data ;
                    break ;
                case 3:
                    var.type = G_parameter3Type ;
                    var.ns.number = (T_word32)G_parameter3Data ;
                    break ;
                default:
                    DebugCheck(FALSE) ;
                    break ;
            }
            break ;
        case SCRIPT_DATA_TYPE_FLAG:
            value = ScriptGetCodeByte(p_script, (*position)++) ;
            DebugCheck(value < SCRIPT_FLAG_UNKNOWN) ;
            var = *(G_systemFlags + value) ;
            break ;
        case SCRIPT_DATA_TYPE_VARIABLE:
            value = ScriptGetCodeWord(p_script, *position) ;
            (*position) += 2 ;
            var = (*(ILookupVariable(p_script, value))) ;
            break ;
        case SCRIPT_DATA_TYPE_STRING:
            var.type = type ;
            p_data = (ScriptGetCode(p_script) + *position) ;
            p_string = (T_scriptString *)p_data ;
            var.ns.p_string = p_string ;

            *position += p_string->length + 1 ;
            break ;
        case SCRIPT_DATA_TYPE_8_BIT_NUMBER:
            var.type = type ;
            p_data = (ScriptGetCode(p_script) + *position) ;
            p_number = (T_sword32 *)((T_sbyte8 *)p_data) ;
            var.ns.number = *((T_sbyte8 *)p_number) ;
            *position += sizeof(T_sbyte8) ;
            break ;
        case SCRIPT_DATA_TYPE_16_BIT_NUMBER:
            var.type = type ;
            p_data = (ScriptGetCode(p_script) + *position) ;
            p_number = (T_sword32 *)((T_sbyte8 *)p_data) ;
            var.ns.number = *((T_sword16 *)p_number) ;
            *position += sizeof(T_sword16) ;
            break ;
        case SCRIPT_DATA_TYPE_32_BIT_NUMBER:
            var.type = type ;
            p_data = (ScriptGetCode(p_script) + *position) ;
            p_number = (T_sword32 *)((T_sbyte8 *)p_data) ;
            var.ns.number = *((T_sword32 *)p_number) ;
            *position += sizeof(T_sword32) ;
            break ;
        default:
            DebugCheck(FALSE) ;
            break ;
    }

    DebugEnd() ;

    return var ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICopyData
 *-------------------------------------------------------------------------*/
/**
 *  ICopyData copies one data item to another data item.
 *
 *  NOTE: 
 *  If the destination data item has a string, that destination must be
 *  allowed to do a MemFree on that string.
 *
 *  @param dest -- Destination to put copy
 *  @param source -- Source to copy from.
 *
 *<!-----------------------------------------------------------------------*/
static T_void ICopyData(
                  T_scriptDataItem *dest,
                  T_scriptDataItem *source)
{
    DebugRoutine("ICopyData") ;

    /* Is the memory at the destination a NONE type? */
    if (dest->type == SCRIPT_DATA_TYPE_STRING)  {
        /* We need to free its string. */
        MemFree(dest->ns.p_string) ;

        /* No more memory. */
        dest->ns.p_string = NULL ;
    }

    /* Are we copying a string or a number? */
    if (source->type == SCRIPT_DATA_TYPE_STRING)  {
        dest->ns.p_string = MemAlloc(source->ns.p_string->length+1) ;
        memcpy(
            dest->ns.p_string,
            source->ns.p_string,
            source->ns.p_string->length+1) ;
    } else {
        /* Must be copying something else that doesn't require */
        /* any extra memory. */
        dest->ns.number = source->ns.number ;
    }

    /* Be sure to copy the type. */
    dest->type = source->type ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IValueToCondition
 *-------------------------------------------------------------------------*/
/**
 *  IValueToCondition converts a value into a boolean value.
 *  Non-zero values are TRUE, else FALSE.  For strings, an empty string
 *  is false, else TRUE.
 *
 *  @param p_value -- Value to be converted to Boolean
 *
 *  @return TRUE or FALSE
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IValueToCondition(T_scriptDataItem *p_value)
{
    E_Boolean status ;

    DebugRoutine("IValueToCondition") ;

    if (p_value->type == SCRIPT_DATA_TYPE_STRING)  {
        if (p_value->ns.p_string->length)
            status = TRUE ;
        else
            status = FALSE ;
    } else {
        /* Assume it is a number. */
        if (p_value->ns.number)
            status = TRUE ;
        else
            status = FALSE ;
    }

    DebugEnd() ;

    return status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IPascalToCString
 *-------------------------------------------------------------------------*/
/**
 *  IPascalToCString converts a script string (with length and then data)
 *  into a normal C sytle (null terminated) string.
 *
 *  NOTE: 
 *  There needs to be SCRIPT_MAX_STRING+1 characters where you are
 *  storing the string.
 *
 *  @param p_cstring -- Place to store string
 *  @param p_pstring -- Pointer to script string to convert.
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_void IPascalToCString(T_byte8 *p_cstring, T_scriptString *p_pstring)
{
    T_word16 len ;
    DebugRoutine("IPascalToCString") ;
    len = p_pstring->length ;
    DebugCheck(len <= SCRIPT_MAX_STRING) ;

    memcpy(p_cstring, p_pstring->data, len) ;
    p_cstring[len] = '\0' ;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  IGetPlace
 *-------------------------------------------------------------------------*/
/**
 *  IGetPlace takes in a script and value and finds the position in the
 *  code for the gien value.
 *
 *  NOTE: 
 *  If the position given is out of bounds, this routine bombs.
 *
 *  @param p_script -- Pointer to the script
 *  @param p_value -- Pointer to value.
 *
 *  @return position found.
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IGetPlace(
                    T_scriptHeader *p_script,
                    T_scriptDataItem *p_value)
{
    T_word16 position ;
    T_word16 placeNum ;

    DebugRoutine("IGetPlace") ;
    DebugCheck(p_value->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    placeNum = p_value->ns.number ;

    DebugCheck(ScriptGetHighestPlace(p_script) > placeNum) ;

    position = (ScriptGetPlaces(p_script))[placeNum] ;

    DebugCheck(position != 0xFFFF) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IPascalStringCompare
 *-------------------------------------------------------------------------*/
/**
 *  IPascalStringCompare compares two script strings.
 *
 *  @param p_string1 -- First string
 *  @param p_string2 -- Second string
 *
 *  @return Positive = >, Negative = <, 0 = Equal
 *
 *<!-----------------------------------------------------------------------*/
static T_sword16 IPascalStringCompare(
                     T_scriptString *p_string1,
                     T_scriptString *p_string2)
{
    T_word16 len1, len2 ;
    T_byte8 *p_str1, *p_str2 ;
    T_sword16 diff ;

    DebugRoutine("IPascalStringCompare") ;
    DebugCheck(p_string1 != NULL) ;
    DebugCheck(p_string2 != NULL) ;

    len1 = p_string1->length ;
    len2 = p_string2->length ;

    p_str1 = p_string1->data ;
    p_str2 = p_string2->data ;

    while ((len1) || (len2))  {
        if (*p_str1 != *p_str2)
            break ;
        p_str1++ ;
        p_str2++ ;
        len1-- ;
        len2-- ;
    }

    if ((len1) || (len2))  {
        if ((len1 == 0) || (len2 == 0))  {
            if (len1 < len2)
                diff = -1 ;
            else
                diff = 1 ;
        } else {
            diff = ((T_sword16)*p_str1) - ((T_sword16)*p_str2) ;
        }
    }

    DebugEnd() ;

    return diff ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSet
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSet is called to evaluate "Set(var1, var2)"  or
 *  "var1 = var2".  The left side must be a variable, the right side must
 *  be a value or variabe.
 *
 *  NOTE: 
 *  If you attempt to access a variable number that is not allowed, this
 *  routine bombs.
 *
 *  @param script -- Script to find variable in
 *  @param position -- Position to look for variable in code.
 *
 *  @return Pointer to found variable, or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSet(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value ;

    DebugRoutine("ICommandSet") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;

    p_var->type = value.type ;
    ICopyData(p_var, &value) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandPrint
 *-------------------------------------------------------------------------*/
/**
 *  ICommandPrint displays a strring (currently, only a string) up in
 *  the message area.
 *
 *  @param script -- Script with print command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandPrint(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem value ;
    T_byte8 string[SCRIPT_MAX_STRING+1] ;

    DebugRoutine("ICommandPrint") ;

    value = IScriptGetAny(script, &position) ;

    /* Only strings for now. */
    DebugCheck(value.type <= SCRIPT_DATA_TYPE_STRING) ;

    if (value.type == SCRIPT_DATA_TYPE_STRING)  {
        IPascalToCString(string, value.ns.p_string) ;

    } else {
        sprintf(string, "%ld", value.ns.number) ;
    }

#ifndef SERVER_ONLY
    /* Put up the message. */
    MessageAdd(string) ;
#else
    /* Need something here. */
#endif

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandIf
 *-------------------------------------------------------------------------*/
/**
 *  ICommandIf processes the command "If(condition, place)" where if
 *  condition is true (non-zero), place is the next place executed.
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandIf(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem value ;
    T_scriptDataItem place ;
    E_Boolean condition ;

    DebugRoutine("ICommandIf") ;

    value = IScriptGetAny(script, &position) ;
    place = IScriptGetAny(script, &position) ;

    condition = IValueToCondition(&value) ;

    if (condition)
        position = IGetPlace(script, &place) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandGoto
 *-------------------------------------------------------------------------*/
/**
 *  ICommandGoto process the command "Goto(place)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandGoto(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem place ;

    DebugRoutine("ICommandGoto") ;

    place = IScriptGetAny(script, &position) ;
    position = IGetPlace(script, &place) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandAdd
 *-------------------------------------------------------------------------*/
/**
 *  ICommandAdd processes the command "Add(variable, value)" by adding
 *  value to variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandAdd(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value, value2 ;

    DebugRoutine("ICommandAdd") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Add. */
    p_var->ns.number = value.ns.number + value2.ns.number ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSubtract
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSubtract processes the command "Subtract(variable, value)" by
 *  subtracting value to variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSubtract(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value, value2 ;

    DebugRoutine("ICommandSubtract") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Subtract. */
    p_var->ns.number = value.ns.number - value2.ns.number ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandMultiply
 *-------------------------------------------------------------------------*/
/**
 *  ICommandMultiply processes the command "Multiply(variable, value)" by
 *  subtracting value to variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandMuliply(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value, value2 ;

    DebugRoutine("ICommandMultiply") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Multiply. */
    p_var->ns.number = value.ns.number * value2.ns.number ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandDivide
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDivide   processes the command "Divide(variable, value)" by
 *  dividing value to variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDivide(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value, value2 ;

    DebugRoutine("ICommandDivide") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Divide. */
    DebugCheck(value2.ns.number != 0) ;
    p_var->ns.number = value.ns.number / value2.ns.number ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandIncrement
 *-------------------------------------------------------------------------*/
/**
 *  ICommandIncrement processes the command "Increment(variable)" by
 *  incrementing the variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandIncrement(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;

    DebugRoutine("ICommandIncrement") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_var->ns.number++ ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandDecrement
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDecrement processes the command "Decrement(variable)" by
 *  decrementing the variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDecrement(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;

    DebugRoutine("ICommandDecrement") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_var->ns.number++ ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandCompare
 *-------------------------------------------------------------------------*/
/**
 *  ICommandCompare processes the command "Compare(val1, val2)".
 *  The two sides are compared and the system flags are set appropriately.
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandCompare(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem value1, value2 ;
    T_sword32 diff ;

    DebugRoutine("ICommandCompare") ;

    value1 = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    /* Are we comparing strings or numbers? */
    if (value1.type == SCRIPT_DATA_TYPE_STRING)  {
        DebugCheck(value2.type == SCRIPT_DATA_TYPE_STRING) ;

        diff = IPascalStringCompare(value1.ns.p_string, value2.ns.p_string) ;
    } else {
        DebugCheck(value1.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
        DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

        diff = value1.ns.number - value2.ns.number ;
    }

    G_systemFlags[SCRIPT_FLAG_EQUAL].ns.number = (diff == 0) ;
    G_systemFlags[SCRIPT_FLAG_NOT_EQUAL].ns.number = (diff != 0) ;
    G_systemFlags[SCRIPT_FLAG_LESS_THAN].ns.number = (diff < 0) ;
    G_systemFlags[SCRIPT_FLAG_NOT_LESS_THAN].ns.number = (!(diff < 0)) ;
    G_systemFlags[SCRIPT_FLAG_GREATER_THAN].ns.number = (diff > 0) ;
    G_systemFlags[SCRIPT_FLAG_NOT_GREATER_THAN].ns.number = (!(diff > 0)) ;
    G_systemFlags[SCRIPT_FLAG_LESS_THAN_OR_EQUAL].ns.number = (diff <= 0) ;
    G_systemFlags[SCRIPT_FLAG_GREATER_THAN_OR_EQUAL].ns.number = (diff >= 0) ;
    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSound
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSound   processes the command "Sound(soundNumber)".
 *  The sound is played on the client side.
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSound(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem value ;

    DebugRoutine("ICommandSound") ;

    value = IScriptGetAny(script, &position) ;
    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

#ifndef SERVER_ONLY
    SoundPlayByNumber(value.ns.number, 255) ;
#endif

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandChangeSideTexture
 *-------------------------------------------------------------------------*/
/**
 *  ICommandChangeSideTexture -> "ChangeSideTexture(wallNumber, texture)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandChangeSideTexture(T_scriptHeader *script, T_word16 position)
{
    T_byte8 name[SCRIPT_MAX_STRING+1] ;
    T_scriptDataItem value ;
    T_scriptDataItem string ;

    DebugRoutine("ICommandChangeSideTexture") ;

    value = IScriptGetAny(script, &position) ;
    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    string = IScriptGetAny(script, &position) ;
    DebugCheck(string.type == SCRIPT_DATA_TYPE_STRING) ;

    IPascalToCString(name, string.ns.p_string) ;

    MapSetWallTexture((T_word16)value.ns.number, name) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectSetType
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectSetType    -> "ObjectSetType(objectNum, typeNum)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectSetType(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem object, number;
    T_3dObject *p_obj ;

    DebugRoutine("ICommandObjectSetType") ;

    object = IScriptGetAny(script, &position) ;
    DebugCheck(object.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    number = IScriptGetAny(script, &position) ;
    DebugCheck(number.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_obj = ObjectFind((T_word16)object.ns.number) ;
    DebugCheck(p_obj != NULL) ;

    if (p_obj != NULL)
        ObjectSetType(p_obj, (T_word16)number.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandTeleport
 *-------------------------------------------------------------------------*/
/**
 *  ICommandTeleport         -> "Teleport(x16, y16, z16)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandTeleport(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem x, y, z;
    E_Boolean isFake ;

    DebugRoutine("ICommandTeleport") ;

    x = IScriptGetAny(script, &position) ;
    DebugCheck(x.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    y = IScriptGetAny(script, &position) ;
    DebugCheck(y.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    z = IScriptGetAny(script, &position) ;
    DebugCheck(z.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    isFake = PlayerInFakeMode() ;
    if (isFake)
        PlayerSetRealMode() ;
    PlayerTeleport(
        (T_sword16)x.ns.number,
        (T_sword16)y.ns.number,
        PlayerGetAngle()) ;
    if (isFake)
        PlayerSetFakeMode() ;
        
    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandDoorCycle
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDoorCycle        -> "DoorCycle(doorNum)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDoorCycle(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem doorNum ;

    DebugRoutine("ICommandDoorCycle") ;

    doorNum = IScriptGetAny(script, &position) ;
    DebugCheck(doorNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    DoorOpen((T_word16)doorNum.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandDoorLock
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDoorLock         -> "DoorLock (doorNum)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDoorLock(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem doorNum ;

    DebugRoutine("ICommandDoorLock") ;

    doorNum = IScriptGetAny(script, &position) ;
    DebugCheck(doorNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    DoorLock((T_word16)doorNum.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandDoorUnlock
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDoorUnlock       -> "DoorUnlock(doorNum)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDoorUnlock(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem doorNum ;

    DebugRoutine("ICommandDoorUnlock") ;

    doorNum = IScriptGetAny(script, &position) ;
    DebugCheck(doorNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    DoorUnlock((T_word16)doorNum.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandAreaSound
 *-------------------------------------------------------------------------*/
/**
 *  ICommandAreaSound        -> "AreaSound(soundNum, x16, y16, radius,
 *  volume)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandAreaSound(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem soundNum, x, y, radius, volume ;

    DebugRoutine("ICommandAreaSound") ;

    soundNum = IScriptGetAny(script, &position) ;
    DebugCheck(soundNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    x = IScriptGetAny(script, &position) ;
    DebugCheck(x.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    y = IScriptGetAny(script, &position) ;
    DebugCheck(y.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    radius = IScriptGetAny(script, &position) ;
    DebugCheck(radius.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    volume = IScriptGetAny(script, &position) ;
    DebugCheck(volume.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

#ifndef SERVER_ONLY
    AreaSoundCreate(
        x.ns.number, y.ns.number,
        radius.ns.number,
        volume.ns.number,
        AREA_SOUND_TYPE_ONCE,
        10,
        NULL,
        NULL,
        0,
        soundNum.ns.number) ;
#endif

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandGotoPlace
 *-------------------------------------------------------------------------*/
/**
 *  ICommandGotoPlace        -> "GotoPlace(placeNum)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandGotoPlace(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem placeNum ;

    DebugRoutine("ICommandGotoPlace") ;

    placeNum = IScriptGetAny(script, &position) ;
    DebugCheck(placeNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

#ifndef SERVER_ONLY
    /* !!! Goto place needs start location also passed in. */
    ClientGotoPlace(placeNum.ns.number, 0) ;
#endif

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandDelay
 *-------------------------------------------------------------------------*/
/**
 *  ICommandDelay            -> "Delay(time70)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandDelay(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem delayTime ;
    T_continueData *p_continueData ;

    DebugRoutine("ICommandDelay") ;

    delayTime = IScriptGetAny(script, &position) ;
    DebugCheck(delayTime.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_continueData = MemAlloc(sizeof(T_continueData)) ;
    DebugCheck(p_continueData != NULL) ;

    if (p_continueData)  {
        p_continueData->p_script = script ;
        p_continueData->position = position ;
        ScheduleAddEvent(
            SyncTimeGet() + delayTime.ns.number,
            IContinueExecution,
            (T_word32)p_continueData) ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDelayComplete
 *-------------------------------------------------------------------------*/
/**
 *  IDelayComplete is called by the scheduler when the delay command is
 *  done delaying.
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_void IContinueExecution(T_word32 data)
{
    T_continueData *p_continueData ;

    DebugRoutine("IDelayComplete") ;
    DebugCheck(data != 0) ;

    p_continueData = (T_continueData *)data ;

    IExecuteCode(p_continueData->p_script, p_continueData->position) ;

    MemFree(p_continueData) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSlideFloor
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSlideFloor       -> "SlideFloor(0)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSlideFloor(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem sector, start, end, over, next ;

    DebugRoutine("ICommandSlideFloor") ;

    sector = IScriptGetAny(script, &position) ;
    DebugCheck(sector.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    start = IScriptGetAny(script, &position) ;
    DebugCheck(start.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    end = IScriptGetAny(script, &position) ;
    DebugCheck(end.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    over = IScriptGetAny(script, &position) ;
    DebugCheck(over.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    next = IScriptGetAny(script, &position) ;
    DebugCheck(next.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    SliderStart(
        ((T_word32)sector.ns.number) | (SLIDER_TYPE_FLOOR<<16),
//        ((T_sword32)start.ns.number) << 16,
        ((T_sword32)MapGetFloorHeight((T_word16)sector.ns.number)) << 16,
        ((T_sword32)end.ns.number) << 16,
        (T_sword32)over.ns.number,
        IHandleSlidingFloor,
        ((T_word16)next.ns.number)) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSlideCeiling
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSlideCeiling     -> "SlideCeiling(...)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSlideCeiling(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem sector, start, end, over, next ;

    DebugRoutine("ICommandSlideCeiling") ;

    sector = IScriptGetAny(script, &position) ;
    DebugCheck(sector.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    start = IScriptGetAny(script, &position) ;
    DebugCheck(start.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    end = IScriptGetAny(script, &position) ;
    DebugCheck(end.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    over = IScriptGetAny(script, &position) ;
    DebugCheck(over.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    next = IScriptGetAny(script, &position) ;
    DebugCheck(next.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    SliderStart(
        ((T_word32)sector.ns.number) | (SLIDER_TYPE_CEILING<<16),
//        ((T_sword32)start.ns.number) << 16,
        ((T_sword32)MapGetCeilingHeight((T_word16)sector.ns.number)) << 16,
        ((T_sword32)end.ns.number) << 16,
        (T_sword32)over.ns.number,
        IHandleSlidingCeiling,
        (T_word16)next.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IHandleSlidingFloor
 *-------------------------------------------------------------------------*/
/**
 *  IHandleSlidingFloor           is called as a floor   slides up/down
 *  (as started by a slide floor   activity).
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New floor   height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
static T_sliderResponse IHandleSlidingFloor(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;

    DebugRoutine("IHandleSlidingFloor") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* Change that floor's height. */
    MapSetFloorHeight(sector, (T_sword16)(value>>16)) ;

    DebugEnd() ;

    return SLIDER_RESPONSE_CONTINUE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IHandleSlidingCeiling
 *-------------------------------------------------------------------------*/
/**
 *  IHandleSlidingCeiling         is called as a ceiling slides up/down
 *  (as started by a slide ceiling activity).
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New ceiling height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
static T_sliderResponse IHandleSlidingCeiling(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;

    DebugRoutine("IHandleSlidingCeiling") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* Change that floor's height. */
    MapSetCeilingHeight(sector, (T_sword16)(value>>16)) ;

    DebugEnd() ;

    return SLIDER_RESPONSE_CONTINUE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandGosub
 *-------------------------------------------------------------------------*/
/**
 *  ICommandGosub processes the command "Gosub(place)".  It will start
 *  another execution routine that goes until it hits return and then
 *  continues with the code at where it is.
 *
 *  @param script -- Script
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandGosub(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem place ;
    T_word16 subPosition ;

    DebugRoutine("ICommandGosub") ;

    place = IScriptGetAny(script, &position) ;
    DebugCheck(place.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    subPosition = IGetPlace(script, &place) ;

    IExecuteCode(script, subPosition) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandRandom
 *-------------------------------------------------------------------------*/
/**
 *  ICommandRandom processes the command "Random(var, maxValue)"
 *  by places a number from 0 to maxValue-1 into the given variable.
 *
 *  @param script -- Script with Random command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandRandom(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value ;

    DebugRoutine("ICommandRandom") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value.ns.number <= 32000) ;

    /* Get that random number */
    p_var->ns.number = (RandomNumber() % value.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectSound
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectSound processes the command
 *  "ObjectSound(objNum, soundNum, radius, volume)" by creating a sound
 *  that can be heard across the whole area.
 *
 *  NOTE: 
 *  This routine should only be called from a server script.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectSound(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem objectNum ;
    T_scriptDataItem soundNum ;
    T_scriptDataItem radius ;
    T_scriptDataItem volume ;
    T_3dObject *p_obj ;

    DebugRoutine("ICommandObjectSound") ;

    objectNum = IScriptGetAny(script, &position) ;
    soundNum  = IScriptGetAny(script, &position) ;
    radius    = IScriptGetAny(script, &position) ;
    volume    = IScriptGetAny(script, &position) ;

    DebugCheck(soundNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(objectNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(radius.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(volume.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Find the object. */
    p_obj = ObjectFind((T_word16)objectNum.ns.number) ;
    if (p_obj)  {
        /* Create the sound. */
        AreaSoundCreate(
            ObjectGetX16(p_obj),
            ObjectGetY16(p_obj),
            radius.ns.number,
            255,
            AREA_SOUND_TYPE_ONCE,
            0,
            AREA_SOUND_BAD,
            NULL,
            0,
            soundNum.ns.number) ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectSet
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectSet processes the command "ObjectSet(objectId,
 *  objAttr, value32).
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectSet(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem objectNum, objAttr, valueData ;
    T_sword32 value ;
    T_3dObject *p_obj ;

    DebugRoutine("ICommandObjectSet") ;

    objectNum = IScriptGetAny(script, &position) ;
    objAttr   = IScriptGetAny(script, &position) ;
    valueData = IScriptGetAny(script, &position) ;

    DebugCheck(objectNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(objAttr.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(valueData.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    value = valueData.ns.number ;

    /* Find the object. */
    p_obj = ObjectFind((T_word16)objectNum.ns.number) ;
    DebugCheck(p_obj != NULL) ;
    if (p_obj)  {
        switch(objAttr.ns.number)  {
            case OBJECT_SCRIPT_ATTR_X:
                ObjectSetX(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Y:
                ObjectSetY(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Z:
                ObjectSetZ(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_X16:
                ObjectSetX16(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Y16:
                ObjectSetY16(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Z16:
                ObjectSetZ16(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_MAX_VELOCITY:
                ObjectSetMaxVelocity(p_obj, value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_STANCE:
                ObjectSetStance(p_obj, (T_word16)value) ;
                break ;
            case OBJECT_SCRIPT_ATTR_WAS_BLOCKED:
                if (value)
                    DebugCheck(FALSE) ;
                else
                    ObjectClearBlockedFlag(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_ANGLE:
                ObjectSetAngle(p_obj, (T_word16)value) ;
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectGet
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectGet processes the command "ObjectGet(objectId,
 *  objAttr, var).
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectGet(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem objectNum, objAttr ;
    T_scriptDataItem *p_var = NULL ;
    T_sword32 value ;

    T_3dObject *p_obj ;

    DebugRoutine("ICommandObjectGet") ;

    objectNum = IScriptGetAny(script, &position) ;
    objAttr   = IScriptGetAny(script, &position) ;
    p_var     = IScriptGetVariable(script, &position) ;

    DebugCheck(objectNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(objAttr.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Find the object. */
    p_obj = ObjectFind((T_word16)objectNum.ns.number) ;
    DebugCheck(p_obj != NULL) ;
    if (p_obj)  {
        switch(objAttr.ns.number)  {
            case OBJECT_SCRIPT_ATTR_X:
                value = ObjectGetX(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Y:
                value = ObjectGetY(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Z:
                value = ObjectGetZ(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_X16:
                value = ObjectGetX16(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Y16:
                value = ObjectGetY16(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_Z16:
                value = ObjectGetZ16(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_MAX_VELOCITY:
                value = ObjectGetMaxVelocity(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_STANCE:
                value = ObjectGetStance(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_WAS_BLOCKED:
                value = ObjectWasBlocked(p_obj) ;
                break ;
            case OBJECT_SCRIPT_ATTR_ANGLE:
                value = ObjectGetAngle(p_obj) ;
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }
    }

    p_var->ns.number = value ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandError
 *-------------------------------------------------------------------------*/
/**
 *  ICommandError processes the command "Error(string)" by printing out
 *  the message and crashing.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandError(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem string ;

    DebugRoutine("ICommandError") ;

    string = IScriptGetAny(script, &position) ;
    DebugCheck(string.type == SCRIPT_DATA_TYPE_STRING) ;

    printf("Script error: %s\n (at %d in %p)",
        string.ns.p_string, position, script) ;
    fprintf(stderr, "Script error: %s (at %d in %p)\n",
        string.ns.p_string, position, script) ;
    DebugCheck(FALSE) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandLookForPlayer
 *-------------------------------------------------------------------------*/
/**
 *  ICommandLookForPlayer processes "LookForPlayer(lookingObj, targetVar)"
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandLookForPlayer(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem objectNum ;
    T_scriptDataItem *p_target = NULL ;
    T_3dObject *p_obj ;
    T_sword16 playerID ;

    DebugRoutine("ICommandLookForPlayer") ;

    objectNum = IScriptGetAny(script, &position) ;
    p_target  = IScriptGetVariable(script, &position) ;

    DebugCheck(objectNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_target->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_obj = ObjectFind((T_word16)objectNum.ns.number) ;
    DebugCheck(p_obj != NULL) ;

    p_target->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
    if (p_obj)  {
#ifdef SERVER_ONLY
        p_obj = CreatureLookForPlayer(p_obj) ;

        if (p_obj != NULL)  {
            DebugCheck(p_obj != NULL) ;

            if (p_obj)  {
                p_target->ns.number = ObjectGetServerId(p_obj) ;
            } else {
                p_target->ns.number = 0xFFFF ;
            }
        } else {
            p_target->ns.number = 0xFFFF ;
        }
#else
        playerID = CreatureLookForPlayer(p_obj) ;

        if (playerID != -1)  {
            p_obj = ServerGetPlayerObject(playerID) ;

            DebugCheck(p_obj != NULL) ;

            if (p_obj)  {
                p_target->ns.number = ObjectGetServerId(p_obj) ;
            } else {
                p_target->ns.number = 0xFFFF ;
            }
        } else {
            p_target->ns.number = 0xFFFF ;
        }
#endif
    } else {
        p_target->ns.number = 0xFFFF ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandIfNot
 *-------------------------------------------------------------------------*/
/**
 *  ICommandIfNot processes the command "IfNot(condition, place)" where if
 *  condition is true (non-zero), place is the next place executed.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandIfNot(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem value ;
    T_scriptDataItem place ;
    E_Boolean condition ;

    DebugRoutine("ICommandIf") ;

    value = IScriptGetAny(script, &position) ;
    place = IScriptGetAny(script, &position) ;

    condition = IValueToCondition(&value) ;

    if (!condition)
        position = IGetPlace(script, &place) ;

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectGetAngleToObject
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectGetAngleToObject processes the command
 *  "ObjectGetAngleToObject(targetAngle, sourceObject, targetObject)"
 *  and determines the angle between the target and source object and
 *  places the answer in targetAngle.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectGetAngleToObject(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sourceObjNum, targetObjNum ;
    T_scriptDataItem *p_targetAngle = NULL ;
    T_3dObject *p_source, *p_target ;

    DebugRoutine("ICommandObjectGetAngleToObject") ;

    p_targetAngle = IScriptGetVariable(script, &position) ;
    sourceObjNum = IScriptGetAny(script, &position) ;
    targetObjNum = IScriptGetAny(script, &position) ;

    DebugCheck(sourceObjNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(targetObjNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_targetAngle->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_source = ObjectFind((T_word16)sourceObjNum.ns.number) ;
    DebugCheck(p_source != NULL) ;

    p_target = ObjectFind((T_word16)targetObjNum.ns.number) ;
    DebugCheck(p_source != NULL) ;

    if ((p_source) && (p_target))  {
        p_targetAngle->ns.number =
            (T_sword32)MathArcTangent(
                (T_sword16)(ObjectGetX16(p_target) - ObjectGetX16(p_source)),
                (T_sword16)(ObjectGetY16(p_target) - ObjectGetY16(p_source))) ;
    } else {
        p_targetAngle->ns.number = 0 ;
    }
    p_targetAngle->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandAbsolute
 *-------------------------------------------------------------------------*/
/**
 *  ICommandAbsolute               processes the command
 *  "Absolute(var)"
 *  and takes the absolute of the var and puts it back in the var.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandAbsolute(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("ICommandAbsolute") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    if (p_var->ns.number < 0)
        p_var->ns.number = -p_var->ns.number ;

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandClear
 *-------------------------------------------------------------------------*/
/**
 *  ICommandClear                  processes the command
 *  "Clear(var)"
 *  and puts a zero number in the given variable.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandClear(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("ICommandClear") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_var->ns.number = 0 ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandNegate
 *-------------------------------------------------------------------------*/
/**
 *  ICommandNegate                 processes the command
 *  "Negate(var)"
 *  and flips the sign        the var and puts it back in the var.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandNegate(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("ICommandNegate") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_var->ns.number = -p_var->ns.number ;

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectDistanceToObject
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectDistanceToObject processes the command
 *  "ObjectDistanceToObject(distVar, obj1, obj2)"
 *  and determines the distance from one object to another.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectDistanceToObject(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sourceObjNum, targetObjNum ;
    T_scriptDataItem *p_dist = NULL ;
    T_3dObject *p_source, *p_target ;

    DebugRoutine("ICommandObjectDistanceToObject") ;

    p_dist = IScriptGetVariable(script, &position) ;
    sourceObjNum = IScriptGetAny(script, &position) ;
    targetObjNum = IScriptGetAny(script, &position) ;

    DebugCheck(sourceObjNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(targetObjNum.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_dist->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_source = ObjectFind((T_word16)(sourceObjNum.ns.number)) ;
    DebugCheck(p_source != NULL) ;

    p_target = ObjectFind((T_word16)(targetObjNum.ns.number)) ;
    DebugCheck(p_source != NULL) ;

    if ((p_source) && (p_target))  {
        p_dist->ns.number =
            CalculateDistance(
                ObjectGetX16(p_source),
                ObjectGetY16(p_source),
                ObjectGetX16(p_target),
                ObjectGetY16(p_target)) ;
    } else {
        p_dist->ns.number = 0x7FFF ;
    }
    p_dist->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectAccelForward
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectAccelForward     processes the command
 *  "ObjectAccelForward(object, accelAmount)"
 *  and determines the distance from one object to another.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectAccelForward(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem object, accelAmount;
    T_3dObject *p_obj ;

    DebugRoutine("ICommandObjectAccelForward") ;

    object = IScriptGetAny(script, &position) ;
    accelAmount = IScriptGetAny(script, &position) ;

    DebugCheck(object.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(accelAmount.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_obj = ObjectFind((T_word16)object.ns.number) ;
    DebugCheck(p_obj != NULL) ;

    if (p_obj)  {
        ObjectAccelFlat(p_obj, accelAmount.ns.number, ObjectGetAngle(p_obj)) ;
    }

    DebugEnd() ;

    return position ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICommandObjectDamageForward
 *-------------------------------------------------------------------------*/
/**
 *  ICommandObjectDamageForward    processes the command
 *  "ObjectDamageForward(object,distance,radius,typeDamage,damageAmount)"
 *  and does the amount of damage forward.
 *
 *  @param script -- Script with command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandObjectDamageForward(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem object, distance, typeDamage, radius, damageAmount ;
    T_3dObject *p_obj ;
    T_sword32 frontX, frontY ;

    DebugRoutine("ICommandObjectAccelForward") ;

    object = IScriptGetAny(script, &position) ;
    distance = IScriptGetAny(script, &position) ;
    radius = IScriptGetAny(script, &position) ;
    typeDamage = IScriptGetAny(script, &position) ;
    damageAmount = IScriptGetAny(script, &position) ;

    DebugCheck(object.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(distance.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(radius.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(typeDamage.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(damageAmount.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    p_obj = ObjectFind((T_word16)object.ns.number) ;
    DebugCheck(p_obj != NULL) ;

    if (p_obj)  {
        ObjectGetForwardPosition(
            p_obj,
            (T_sword16)(ObjectGetRadius(p_obj) + distance.ns.number),
            &frontX,
            &frontY) ;

#if 0
/* !!! To be modified later. */
        ServerDamageAt(
            ObjectGetX(p_obj),
            ObjectGetY(p_obj),
            frontX,
            frontY,
            (T_word16)radius.ns.number,
            (T_word16)damageAmount.ns.number,
            0) ;
#endif
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSubtract16
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSubtract processes the command "Subtract(variable, value)" by
 *  subtracting value to variable.  Variable must be a number (currently).
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSubtract16(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem value, value2 ;
    T_sword16 v1, v2 ;

    DebugRoutine("ICommandSubtract16") ;

    p_var = IScriptGetVariable(script, &position) ;
    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;

    DebugCheck(value.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(value2.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Subtract. */
    v1 = value.ns.number ;
    v2 = value2.ns.number ;

    v1 -= v2 ;
    p_var->ns.number = v1 ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandTextBoxSetSelection
 *-------------------------------------------------------------------------*/
/**
 *  ICommandTextBoxSetSelection changes the selected line in a selection
 *  text box.
 *  CommandProcessed ->  "TextBoxSetSelection(textBoxID, row)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandTextBoxSetSelection(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem textBoxID, row;

    DebugRoutine("ICommandTextBoxSetSelection") ;

    textBoxID = IScriptGetAny(script, &position) ;
    row = IScriptGetAny(script, &position) ;

    DebugCheck(row.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    DebugCheck(textBoxID.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Change the row. */
#ifndef SERVER_ONLY
    ScriptFormTextBoxSetSelection(
        textBoxID.ns.number,
        row.ns.number) ;
#else
    DebugCheck(FALSE) ;
#endif

    DebugEnd() ;

    return position ;
}

/* LES: 12/21/95 */
static T_word16 ICommandObjectShootObject(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_3dObject *p_obj ;
    T_scriptDataItem value, value2, value3 ;

    DebugRoutine("ICommandObjectShootObject") ;

    value = IScriptGetAny(script, &position) ;
    value2 = IScriptGetAny(script, &position) ;
    value3 = IScriptGetAny(script, &position) ;

    p_obj = ObjectFind((T_word16)value.ns.number) ;
#ifndef NDEBUG
    if (p_obj == NULL)  {
        printf("Unknown object %d\n", value.ns.number) ;
    }
#endif
    DebugCheck(p_obj != NULL) ;
    if (p_obj)  {
        // Shoot directly with the server/world system
        T_word16 objectType = value2.ns.number;
        T_word16 angle = ObjectGetAngle(p_obj) ;
        T_word16 velocity = value3.ns.number;
        ServerShootProjectile(p_obj, angle, objectType, velocity, 0);
    }

    DebugEnd() ;

    return position ;
}

/* LES: 12/21/95 */
static T_word16 ICommandPlayerObjectGet(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;

    DebugRoutine("ICommandPlayerObjectGet") ;

    p_var = IScriptGetVariable(script, &position) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

#ifdef SERVER_ONLY
    DebugCheck(FALSE) ;
#else
    p_var->ns.number = ObjectGetServerId(PlayerGetObject()) ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
#endif

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandGetFloorHeight
 *-------------------------------------------------------------------------*/
/**
 *  ICommandGetFloorHeight gets the height of a floor in a 16 bit value.
 *  CommandProcessed ->  "GetFloorHeight(sector, var)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandGetFloorHeight(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;
    T_scriptDataItem value ;

    DebugRoutine("ICommandGetFloorHeight") ;

    /* Get the sector for the floor. */
    value = IScriptGetAny(script, &position) ;

    /* Get where to store the value. */
    p_var = IScriptGetVariable(script, &position) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Store the height in the variable. */
    p_var->ns.number = MapGetFloorHeight((T_word16)value.ns.number) ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandGetCeilingHeight
 *-------------------------------------------------------------------------*/
/**
 *  ICommandGetCeilingHeight gets the height of a ceil. in a 16 bit value.
 *  CommandProcessed ->  "GetCeilingHeight(sector, var)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandGetCeilingHeight(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;
    T_scriptDataItem value ;

    DebugRoutine("ICommandGetCeilingHeight") ;

    /* Get the sector for the floor. */
    value = IScriptGetAny(script, &position) ;

    /* Get where to store the value. */
    p_var = IScriptGetVariable(script, &position) ;
    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Store the height in the variable. */
    p_var->ns.number = MapGetCeilingHeight((T_word16)value.ns.number) ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Door increase lock */
static T_word16 ICommandDoorIncreaseLock(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sector, amount ;

    DebugRoutine("ICommandDoorIncreaseLock") ;

    /* Get the sector for the floor. */
    sector = IScriptGetAny(script, &position) ;

    /* Get the amount to increase. */
    amount = IScriptGetAny(script, &position) ;

    DoorIncreaseLock(
        (T_word16)sector.ns.number,
        (T_word16)amount.ns.number) ;

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Door decrease lock */
static T_word16 ICommandDoorDecreaseLock(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sector, amount ;

    DebugRoutine("ICommandDoorDecreaseLock") ;

    /* Get the sector for the floor. */
    sector = IScriptGetAny(script, &position) ;

    /* Get the amount to decrease. */
    amount = IScriptGetAny(script, &position) ;

    DoorDecreaseLock(
        (T_word16)sector.ns.number,
        (T_word16)amount.ns.number) ;

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Start a side state */
static T_word16 ICommandSideState(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem sideNum, state ;

    DebugRoutine("ICommandSideState") ;

    /* Get the side number. */
    sideNum = IScriptGetAny(script, &position) ;

    /* Get the state to become. */
    state = IScriptGetAny(script, &position) ;

//    if (CommIsServer() == TRUE)  {
        MapSetSideState(sideNum.ns.number, state.ns.number) ;
//    }

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Start a wall state */
static T_word16 ICommandWallState(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem wallNum, state ;

    DebugRoutine("ICommandWallState") ;

    /* Get the wall number. */
    wallNum = IScriptGetAny(script, &position) ;

    /* Get the state to become. */
    state = IScriptGetAny(script, &position) ;

//    if (CommIsServer() == TRUE)  {
        MapSetWallState(wallNum.ns.number, state.ns.number) ;
//    }

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Start a sector state */
static T_word16 ICommandSectorState(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem sectorNum, state ;

    DebugRoutine("ICommandSectorState") ;

    /* Get the sector number. */
    sectorNum = IScriptGetAny(script, &position) ;

    /* Get the state to become. */
    state = IScriptGetAny(script, &position) ;

//    if (CommIsServer() == TRUE)  {
        MapSetSectorState(sectorNum.ns.number, state.ns.number) ;
//    }

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Give the player experience */
static T_word16 ICommandGivePlayerXP(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem xp ;

    DebugRoutine("ICommandGivePlayerXP") ;

    /* Get the amount of experience. */
    xp = IScriptGetAny(script, &position) ;

    StatsChangePlayerExperience((T_sword16)(xp.ns.number)) ;

    DebugEnd() ;

    return position ;
}

/* LES: 05/13/96 -- Give all players experience */
static T_word16 ICommandGiveAllPlayersXP(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem xp ;

    DebugRoutine("ICommandGiveAllPlayersXP") ;

    /* Get the amount of experience. */
    xp = IScriptGetAny(script, &position) ;

    StatsChangePlayerExperience((T_sword16)(xp.ns.number)) ;

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandEffect(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem effectType, data1, data2, data3 ;

    DebugRoutine("ICommandEffect") ;

    effectType = IScriptGetAny(script, &position) ;

    data1 = IScriptGetAny(script, &position) ;

    data2 = IScriptGetAny(script, &position) ;

    data3 = IScriptGetAny(script, &position) ;

#ifndef SERVER_ONLY
    /* Cause the effect. */
    Effect(
        (T_sword16)(effectType.ns.number),
        EFFECT_TRIGGER_NONE,
        (T_word16)(data1.ns.number),
        (T_word16)(data2.ns.number),
        (T_word16)(data3.ns.number),
        NULL) ;
#endif

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandSectorSetLight(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sector, lightLevel ;

    DebugRoutine("ICommandSectorSetLight") ;

    sector = IScriptGetAny(script, &position) ;
    lightLevel = IScriptGetAny(script, &position) ;

    MapSetSectorLighting(
        (T_word16)(sector.ns.number),
        (T_byte8)(lightLevel.ns.number)) ;

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandGenerateMissile(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem objType, x, y, z, targetX, targetY, targetZ, initSpeed ;

    DebugRoutine("ICommandGenerateMissile") ;

    objType = IScriptGetAny(script, &position) ;
    x = IScriptGetAny(script, &position) ;
    y = IScriptGetAny(script, &position) ;
    z = IScriptGetAny(script, &position) ;
    targetX = IScriptGetAny(script, &position) ;
    targetY = IScriptGetAny(script, &position) ;
    targetZ = IScriptGetAny(script, &position) ;
    initSpeed = IScriptGetAny(script, &position) ;

//    if (CommIsServer() == TRUE)  {
        ServerShootBasicProjectile(
            (T_word16)(objType.ns.number),
            (T_sword32)((x.ns.number)<<16),
            (T_sword32)((y.ns.number)<<16),
            (T_sword32)((z.ns.number)<<16),
            (T_sword32)((targetX.ns.number)<<16),
            (T_sword32)((targetY.ns.number)<<16),
            (T_sword32)((targetZ.ns.number)<<16),
            (T_word16)(initSpeed.ns.number)) ;
//    }

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandPlayerHasItem(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;
    T_scriptDataItem item ;

    DebugRoutine("ICommandPlayerHasItem") ;

    p_var = IScriptGetVariable(script, &position) ;
    item = IScriptGetAny(script, &position) ;

    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
/* !!! Need InventoryHasItem to be written */
/*    if (InventoryHasItem((T_word16)(item.ns.number)) == TRUE)  {
        p_var->ns.number = 1 ;
    } else */ {
        p_var->ns.number = 0 ;
    }

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandIsEffectActive(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var = NULL ;
    T_scriptDataItem effect ;

    DebugRoutine("ICommandIsEffectActive") ;

    p_var = IScriptGetVariable(script, &position) ;
    effect = IScriptGetAny(script, &position) ;

    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
#ifndef SERVER_ONLY
    if (EffectPlayerEffectIsActive(
            (E_playerEffectType)(effect.ns.number)) == TRUE)  {
        /* Player effect is active */
        p_var->ns.number = 1 ;
    } else {
        /* Player effect is not active. */
        p_var->ns.number = 0 ;
    }
#else
    /* When server only, always return zero. */
    p_var->ns.number = 0 ;
#endif

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandActivateGenerator(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem genID ;

    DebugRoutine("ICommandActivateGenerator") ;

    genID = IScriptGetAny(script, &position) ;

    ObjectGeneratorActivate((T_word16)(genID.ns.number)) ;

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandDeactiveGenerator(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem genID ;

    DebugRoutine("ICommandDeactivateGenerator") ;

    genID = IScriptGetAny(script, &position) ;

    ObjectGeneratorDeactivate((T_word16)(genID.ns.number)) ;

    DebugEnd() ;

    return position ;
}

static T_word16 ICommandGroupState(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem group ;

    DebugRoutine("ICommandGroupState") ;

    group = IScriptGetAny(script, &position) ;

    ObjectGeneratorDeactivate((T_word16)(group.ns.number)) ;

//    if (CommIsServer() == TRUE)  {
/* !!! Not implemented yet
        ServerSendGroupStateChange((T_word16)group.ns.number) ;
*/
//    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandBlock
 *-------------------------------------------------------------------------*/
/**
 *  ICommandBlock stops if the given variable is not zero, else goes
 *  on after setting the variable to 1.
 *
 *  @param script -- Script with Block command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandBlock(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;

    DebugRoutine("ICommandBlock") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    if (p_var->ns.number == 0)  {
        /* The given value is zero.  Make the value not zero and go on. */
        p_var->ns.number = 1 ;
        p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;
    } else {
        /* The given value is not zero.  We must immediately stop here. */
        G_pleaseStop = TRUE ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandUnblock
 *-------------------------------------------------------------------------*/
/**
 *  ICommandUnblock makes a variable previously set by a block command
 *  be zero to allow the block command to continue.
 *
 *  @param script -- Script with Block command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandUnblock(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem *p_var ;

    DebugRoutine("ICommandUnblock") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Unblock the variable. */
    p_var->ns.number = 0 ;
    p_var->type = SCRIPT_DATA_TYPE_32_BIT_NUMBER ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandToggleSwitch
 *-------------------------------------------------------------------------*/
/**
 *  ICommandToggleSwitch toggles a wall switch to go between up and down
 *  based on a given variable.  It toggles between the two given texture
 *  names.  If a script number other than -1 is given, then it goes to
 *  that script based on the state of the wall.
 *  ToggleSwitch(var, sidedef, "UP", "DOWN", scriptUp, scriptDown) ;
 *
 *  @param script -- Script with Block command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandToggleSwitch(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem *p_var ;
    T_scriptDataItem side ;
    T_scriptDataItem stringUp ;
    T_scriptDataItem stringDown ;
    T_scriptDataItem downScript ;
    T_scriptDataItem upScript ;
    T_byte8 name[SCRIPT_MAX_STRING+1] ;

    DebugRoutine("ICommandToggleSwitch") ;

    p_var = IScriptGetVariable(script, &position) ;

    DebugCheck(p_var->type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    side = IScriptGetAny(script, &position) ;
    DebugCheck(side.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    stringUp = IScriptGetAny(script, &position) ;
    DebugCheck(stringUp.type == SCRIPT_DATA_TYPE_STRING) ;

    stringDown = IScriptGetAny(script, &position) ;
    DebugCheck(stringDown.type == SCRIPT_DATA_TYPE_STRING) ;

    upScript = IScriptGetAny(script, &position) ;
    DebugCheck(upScript.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    downScript = IScriptGetAny(script, &position) ;
    DebugCheck(downScript.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    /* Is the switch up or down? */
    if (p_var->ns.number == 0)  {
        /* It is up.  Put it in the down position. */
        p_var->ns.number = 1 ;
        IPascalToCString(name, stringDown.ns.p_string) ;
        MapSetWallTexture((T_word16)side.ns.number, name) ;

        /* Go to the down script. */
        if (downScript.ns.number != -1)
            position = IGetPlace(script, &downScript) ;
    } else {
        /* It is down.  Put it in the up position. */
        p_var->ns.number = 0 ;
        IPascalToCString(name, stringUp.ns.p_string) ;
        MapSetWallTexture((T_word16)side.ns.number, name) ;

        /* Go to the down script. */
        if (upScript.ns.number != -1)
            position = IGetPlace(script, &upScript) ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSlideFloorNice
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSlideFloor       -> "SlideFloorNice(0)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSlideFloorNice(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sector, start, end, over, next ;

    DebugRoutine("ICommandSlideFloorNice") ;

    sector = IScriptGetAny(script, &position) ;
    DebugCheck(sector.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    start = IScriptGetAny(script, &position) ;
    DebugCheck(start.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    end = IScriptGetAny(script, &position) ;
    DebugCheck(end.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    over = IScriptGetAny(script, &position) ;
    DebugCheck(over.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    next = IScriptGetAny(script, &position) ;
    DebugCheck(next.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    SliderStart(
        ((T_word32)sector.ns.number) | (SLIDER_TYPE_FLOOR<<16),
        ((T_sword32)MapGetFloorHeight((T_word16)sector.ns.number)) << 16,
        ((T_sword32)end.ns.number) << 16,
        (T_sword32)over.ns.number,
        IHandleSlidingFloorNice,
        ((T_word16)next.ns.number)) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICommandSlideCeilingNice
 *-------------------------------------------------------------------------*/
/**
 *  ICommandSlideCeiling     -> "SlideCeilingNice(...)"
 *
 *  @param script -- Script with If command
 *  @param position -- Position in code
 *
 *  @return New position
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 ICommandSlideCeilingNice(
                    T_scriptHeader *script,
                    T_word16 position)
{
    T_scriptDataItem sector, start, end, over, next ;

    DebugRoutine("ICommandSlideCeilingNice") ;

    sector = IScriptGetAny(script, &position) ;
    DebugCheck(sector.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    start = IScriptGetAny(script, &position) ;
    DebugCheck(start.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    end = IScriptGetAny(script, &position) ;
    DebugCheck(end.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    over = IScriptGetAny(script, &position) ;
    DebugCheck(over.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;
    next = IScriptGetAny(script, &position) ;
    DebugCheck(next.type <= SCRIPT_DATA_TYPE_32_BIT_NUMBER) ;

    SliderStart(
        ((T_word32)sector.ns.number) | (SLIDER_TYPE_CEILING<<16),
        ((T_sword32)MapGetCeilingHeight((T_word16)sector.ns.number)) << 16,
        ((T_sword32)end.ns.number) << 16,
        (T_sword32)over.ns.number,
        IHandleSlidingCeilingNice,
        (T_word16)next.ns.number) ;

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IHandleSlidingFloorNice
 *-------------------------------------------------------------------------*/
/**
 *  IHandleSlidingFloorNice       is called as a floor   slides up/down
 *  (as started by a slide floor   activity).
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New floor   height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
static T_sliderResponse IHandleSlidingFloorNice(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;
    T_sliderResponse response ;

    DebugRoutine("IHandleSlidingFloorNice") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* Change that floor's height. */
    if (MapCheckCrushByFloor(sector, (T_sword16)(value>>16)) == FALSE)  {
        /* Not crushing, must be ok. */
        MapSetFloorHeight(sector, (T_sword16)(value>>16)) ;
        response = SLIDER_RESPONSE_CONTINUE ;
    } else {
        /* Crushing is a problem. */
        /* Hold this position until the obstruction is out of the way. */
        response = SLIDER_RESPONSE_PAUSE ;
    }

    DebugEnd() ;

    return response ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IHandleSlidingCeilingNice
 *-------------------------------------------------------------------------*/
/**
 *  IHandleSlidingCeiling         is called as a ceiling slides up/down
 *  (as started by a slide ceiling activity).
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New ceiling height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
static T_sliderResponse IHandleSlidingCeilingNice(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;
    T_sliderResponse response ;

    DebugRoutine("IHandleSlidingCeilingNice") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* Change that ceiling's height. */
    if (MapCheckCrushByCeiling(sector, (T_sword16)(value>>16)) == FALSE)  {
        /* Not crushing, must be ok. */
        MapSetCeilingHeight(sector, (T_sword16)(value>>16)) ;
        response = SLIDER_RESPONSE_CONTINUE ;
    } else {
        /* Crushing is a problem. */
        /* Hold this position until the obstruction is out of the way. */
        response = SLIDER_RESPONSE_PAUSE ;
    }

    DebugEnd() ;

    return response ;
}

static T_word16 ICommandJournalEntry(T_scriptHeader *script, T_word16 position)
{
    T_scriptDataItem entry ;

    DebugRoutine("ICommandJournalEntry") ;

    entry = IScriptGetAny(script, &position) ;

    StatsAddPlayerNotePage(entry.ns.number) ;

    DebugEnd() ;

    return position ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SCRIPT.C
 *-------------------------------------------------------------------------*/
