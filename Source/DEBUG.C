/*-------------------------------------------------------------------------*
 * File:  DEBUG.C
 *-------------------------------------------------------------------------*/
/**
 * Debugging routines to help with working on the code without a debugger.
 * It tracks stacks so when a problem occurs a call stack can be output.
 *
 * @addtogroup DEBUG
 * @brief Debug Call Stack System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "DEBUG.H"
#include "MEMORY.H"
#if defined(_DEBUG) && defined(WIN32)
#include <crtdbg.h>
#endif
#ifndef NDEBUG

#define DEBUG_NO_TIME 0
#define MAX_TIME_SLOTS  100
#define HEAP_CHECK_ON_ENTER_TOO     0
#define HEAP_CHECK_LESS_OFTEN       100 // number of times to skip between checks

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

//#define COMPILE_OPTION_DEBUG_CHECK_VECTOR_TABLE

/* The calling stack is a list of pointers to routine names defined
   else where. */
const char *G_CallStack[DEBUG_MAX_STACK_DEPTH] ;
const char *G_CallStackFile[DEBUG_MAX_STACK_DEPTH] ;
T_word16 G_CallStackLine[DEBUG_MAX_STACK_DEPTH] ;
T_word32 G_CallStackTime[DEBUG_MAX_STACK_DEPTH] ;
T_word16 G_CallStackTimeSlot[DEBUG_MAX_STACK_DEPTH] ;
void *G_CallStackLuaState[DEBUG_MAX_STACK_DEPTH] ;
T_word32 G_TimeSlots[MAX_TIME_SLOTS] ;

/* The following tells where the last access to the calling stack has
   been made. */
static T_word16 G_StackPosition = 0 ;

/* Are we checking the heap after each routine? */
static E_Boolean G_heapCheck = FALSE ;
//static E_Boolean G_heapCheck = TRUE ;

static E_Boolean G_stop = FALSE ;

static E_Boolean G_haveTimeSlots = FALSE ;

static E_Boolean G_vectorTableStored = FALSE ;
static T_byte8 G_vectorTable[64 * 4] ;
static T_byte8 G_savedPIC ;

T_void IDebugReportTimeSlots(T_void) ;
void LuaStackDump(void *state);

/*-------------------------------------------------------------------------*
 * Routine:  DebugAddRoutine
 *-------------------------------------------------------------------------*/
/**
 *  DebugAddRoutine is NOT called directly.  It is called by the use
 *  of the macro "DebugRoutine."  DebugRoutine is used to declare that
 *  the program is entering a section of code that needs to be debugged.
 *  The name of the routine is added to a call stack and will be removed
 *  later by DebugRemoveRoutine.
 *
 *  NOTE: 
 *  The current system only allows DEBUG_MAX_STACK_DEPTH levels of
 *  calling.  Should you go deeper, this routine will create an error.
 *  Also, this stack is only coherent as long as calls are made in each
 *  functions.
 *
 *  @param p_routineName -- String to add to call stack
 *      p_routineName is not NULL.
 *  @param p_filename -- String of source filename
 *  @param lineNum -- Source line number
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugAddRoutine(
           const char *p_routineName,
           const char *p_filename,
           long lineNum)
{
    /* We will place the routine's name on the call stack. */
    DebugCheck(p_routineName != NULL) ;

    /* First, determine if there is room on the stack for the name. */
    if (G_StackPosition == DEBUG_MAX_STACK_DEPTH)  {
        /* Error! Too many calls */
        DebugFail("Call stack too deep!", __FILE__, __LINE__) ;
    } else {
        /* OK, just add to the stack. */
        G_CallStackFile[G_StackPosition] = p_filename ;
        G_CallStackLine[G_StackPosition] = (T_word16)lineNum ;
        G_CallStack[G_StackPosition] = p_routineName ;
        G_CallStackTimeSlot[G_StackPosition] = DEBUG_NO_TIME ;
        G_CallStackLuaState[G_StackPosition] = 0;
#ifdef COMPILE_OPTION_DEBUG_WITH_TIME
        G_CallStackTime[G_StackPosition] = TickerGetAccruate() ;
#endif
        G_StackPosition++ ;
    }

#if HEAP_CHECK_ON_ENTER_TOO
    if (G_heapCheck == TRUE) {
        MemCheck(998) ;
#if defined(_DEBUG) && defined(WIN32)
        _ASSERTE( _CrtCheckMemory( ) );
#endif
    }
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  DebugFail
 *-------------------------------------------------------------------------*/
/**
 *  Debug Fail is called when a DebugCheck macro finds an illegal
 *  assumption.  It will call this routine and expect the system to print
 *  out the error messages both to the screen and to an "ERROR.LOG."
 *  A list of routines is also printed out.
 *
 *  NOTE: 
 *  This current version does not change to text mode.
 *
 *  @param p_msg -- Message as to why it died/failed
 *  @param p_file -- Name of file it died in
 *  @param line -- Line number where failure occured
 *      All of the above are assumed to be valid pointers.
 *
 *  @return File containing explanation
 *      I'm assuming that we can still open a file and do a dump.
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugFail(const char *p_msg, const char *p_file, long line)
{
    FILE *fp ;

    /* Open a file for the error log. */
    fp = fopen("error.log", "a") ;
    fprintf(fp, "Failure: %s (FILE: %s  LINE: %d)\n", p_msg, p_file, line) ;
    fprintf(stderr, "Failure: %s (FILE: %s  LINE: %d)\n", p_msg, p_file, line) ;

    fprintf(fp, "Call stack:\n") ;
    fprintf(stderr, "Call stack:\n") ;

    /* Loop for each stack call. */
    while (G_StackPosition)  {
        /* Decrement the stack position. */
        G_StackPosition-- ;

        if (G_CallStackLuaState[G_StackPosition]) {
            /* Show/dump call item: */
            fprintf(
                fp,
                "  Lua file %s (FILE: %s  LINE: %d)\n",
                G_CallStack[G_StackPosition],
                G_CallStackFile[G_StackPosition],
                G_CallStackLine[G_StackPosition]) ;
            fprintf(
                stderr,
                "  Lua file %s (FILE: %s  LINE: %d)\n",
                G_CallStack[G_StackPosition],
                G_CallStackFile[G_StackPosition],
                G_CallStackLine[G_StackPosition]) ;
            LuaStackDump(G_CallStackLuaState[G_StackPosition]);
        } else {
            /* Show/dump call item: */
            fprintf(
                fp,
                "  %s (FILE: %s  LINE: %d)\n",
                G_CallStack[G_StackPosition],
                G_CallStackFile[G_StackPosition],
                G_CallStackLine[G_StackPosition]) ;
            fprintf(
                stderr,
                "  %s (FILE: %s  LINE: %d)\n",
                G_CallStack[G_StackPosition],
                G_CallStackFile[G_StackPosition],
                G_CallStackLine[G_StackPosition]) ;
        }
    }

    /* Done with dump. */
    fflush(fp) ;
    fclose(fp) ;

    /* Do a hard exit. */
//    abort();
    exit(3) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DebugRemoveRoutine
 *-------------------------------------------------------------------------*/
/**
 *  DebugRemoveRoutine is not called directly.  It is called by the
 *  macro "DebugEnd" which is used at the end of a debugged routine.
 *  This routine removes the text that was added to the calling stack
 *  by "DebugAddRoutine" (called via DebugRoutine).
 *
 *  NOTE: 
 *  None that I know of.
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugRemoveRoutine(T_void)
{
#if HEAP_CHECK_LESS_OFTEN
    static int lessOften = 0;
#endif
#ifdef COMPILE_OPTION_DEBUG_CHECK_VECTOR_TABLE
    DebugCheckVectorTable() ;
#endif
    if (G_stop != FALSE)  {
        G_stop = FALSE ;
        DebugCheck(FALSE) ;
    }

    if (G_heapCheck == TRUE) {
#if HEAP_CHECK_LESS_OFTEN
        if (lessOften > 0) {
            lessOften--;
        } else {
            lessOften = HEAP_CHECK_LESS_OFTEN;
#endif
        MemCheck(999) ;
#if defined(_DEBUG) && defined(WIN32)
        _ASSERTE( _CrtCheckMemory( ) );
#endif
#if HEAP_CHECK_LESS_OFTEN
        }
#endif
    }

    /* First see if we are allowed to pop of from the stack. */
    DebugCheck(G_StackPosition > 0) ;

    /* OK, it must be good. */
    G_StackPosition-- ;

    /* Add the difference in time. */
#ifdef COMPILE_OPTION_DEBUG_WITH_TIME
    G_TimeSlots[G_CallStackTimeSlot[G_StackPosition]] +=
        TickerGetAccurate() -
        G_CallStackTime[G_StackPosition] ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  DebugHeapOn
 *-------------------------------------------------------------------------*/
/**
 *  DebugHeapOn turns on heap checking after each routine is called
 *  (actually for each "DebugEnd" statement executed).  The heap is checked
 *  for consistency and bombs with an error message if corrupted.
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugHeapOn(T_void)
{
    DebugRoutine("DebugHeapOn") ;

    G_heapCheck = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DebugHeapOff
 *-------------------------------------------------------------------------*/
/**
 *  DebugHeapOff turns off heap checking after each routine is called
 *  (actually for each "DebugEnd" statement executed).  The heap is checked
 *  for consistency and bombs with an error message if corrupted.
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugHeapOff(T_void)
{
    DebugRoutine("DebugHeapOff") ;

    G_heapCheck = FALSE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DebugCompareCheck
 *-------------------------------------------------------------------------*/
/**
 *  DebugCompareCheck determines if the routine you are in is equal to
 *  the name you have given it.   If not, failure occurs.
 *
 *<!-----------------------------------------------------------------------*/
T_void DebugCompareCheck(const char *str, const char *p_file, T_word16 line)
{
    char msg[80] ;

    if (strcmp(str, G_CallStack[G_StackPosition-1]) != 0)  {
        sprintf(msg, "Routine name comparison (%s) failed!", str) ;
        DebugFail(msg, p_file, line) ;
    }
}

T_void DebugGetCaller(const char **filename, long *line)
{
    *filename = G_CallStack[G_StackPosition-2] ;
    *line = G_CallStackLine[G_StackPosition-2] ;
}

const char *DebugGetCallerName(T_void)
{
    return G_CallStack[G_StackPosition-2] ;
}

const char *DebugGetCallerFile(T_void)
{
    return G_CallStackFile[G_StackPosition-2] ;
}

const char *DebugGetLastCalled(T_void)
{
    return G_CallStack[G_StackPosition] ;
}

T_void DebugStop(T_void)
{
    G_stop = TRUE ;
}

T_void DebugTime(T_word16 timeSlot)
{
    DebugCheck(timeSlot < MAX_TIME_SLOTS) ;

    if (!G_haveTimeSlots)  {
        G_haveTimeSlots = TRUE ;
//        atexit(IDebugReportTimeSlots) ;
        memset(G_TimeSlots, 0, sizeof(G_TimeSlots)) ;
    }

    /* Tell where to record the time slot. */
    G_CallStackTimeSlot[G_StackPosition-1] = timeSlot ;
}

T_void IDebugReportTimeSlots(T_void)
{
    T_word16 i ;

    puts("Debug time slots: ---") ;
    for (i=0; i<MAX_TIME_SLOTS; i++)  {
        if (G_TimeSlots[i])
            printf("%3d) %ld\n", i, G_TimeSlots[i]) ;
    }
    puts("-----------------------") ;
}

#ifdef COMPILE_OPTION_DEBUG_CHECKS_VECTORS
T_void DebugSaveVectorTable(T_void)
{
    memcpy(G_vectorTable, (char *)0, sizeof(G_vectorTable)) ;
    G_savedPIC = inp(0x21) ;
    G_vectorTableStored = TRUE ;
}

T_void DebugCheckVectorTable(T_void)
{
    T_sword16 i ;
    if (G_vectorTableStored)  {
        i = memcmp(G_vectorTable, (char *)0, sizeof(G_vectorTable)) ;
        if (i)  {
            for (i=0; i<sizeof(G_vectorTable); i++)  {
                if (G_vectorTable[i] != ((char *)0)[i])  {
                    printf("DEBUG CHECK: Inconsistent vector at %d (0x%d?)\n", i, i>>2) ; fflush(stdout) ;
                    G_vectorTableStored = FALSE ;
                    DebugCheck(FALSE) ;
                }
            }
        }

        DebugCheck(G_savedPIC == inp(0x21)) ;
    }
}
#endif

static void LuaStackDump(void *state)
{
    lua_State *L = (lua_State *)state;
#if 0
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) { /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {

            case LUA_TSTRING: /* strings */
                printf("`%s'", lua_tostring(L, i));
                break;

            case LUA_TBOOLEAN: /* booleans */
                printf(lua_toboolean(L, i) ? "true" : "false");
                break;

            case LUA_TNUMBER: /* numbers */
                printf("%g", lua_tonumber(L, i));
                break;

            default: /* other types */
                printf("%s", lua_typename(L, t));
                if (strcmp(lua_typename(L, t), "thread") == 0) {
                    fprintf(stderr, "In thread:\n");
                    lua_getglobal(L, "debug");
                    lua_getfield(L, -1, "traceback");
                    lua_pushvalue(L, 1);
                    lua_pushinteger(L, 2);
                    lua_call(L, 2, 1);
                    fprintf(stderr, "%s\n", lua_tostring(L, -1));
                }
                break;

        }
        printf("  "); /* put a separator */
    }
    printf("\n"); /* end the listing */
#endif

//        lua_getglobal(L, "debug");
//        lua_getfield(L, -1, "tracebackWithCoRoutine");
        //lua_getfield(L, LUA_GLOBALSINDEX, "_ErrorHandler");
        lua_pushglobaltable(L);
        lua_getfield(L,-1, "AADumpWithCoRoutine");
        lua_remove(L,-2);
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_call(L, 2, 1);
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        //return 1;

}

void DebugLuaAdd(
        const char *p_routineName,
        void *state,
        const char *p_filename,
        long lineNum)
{
    /* We will place the routine's name on the call stack. */
    DebugCheck(p_routineName != NULL) ;

    /* First, determine if there is room on the stack for the name. */
    if (G_StackPosition == DEBUG_MAX_STACK_DEPTH)  {
        /* Error! Too many calls */
        DebugFail("Call stack too deep!", __FILE__, __LINE__) ;
    } else {
        /* OK, just add to the stack. */
        G_CallStackFile[G_StackPosition] = p_filename ;
        G_CallStackLine[G_StackPosition] = (T_word16)lineNum ;
        G_CallStack[G_StackPosition] = p_routineName ;
        G_CallStackLuaState[G_StackPosition] = state;
        G_CallStackTimeSlot[G_StackPosition] = DEBUG_NO_TIME ;
#ifdef COMPILE_OPTION_DEBUG_WITH_TIME
        G_CallStackTime[G_StackPosition] = TickerGetAccruate() ;
#endif
        G_StackPosition++ ;
    }

}
void DebugLuaRemove(void)
{
    DebugRemoveRoutine();
}
#endif // !NDEBUG

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  DEBUG.C
 *-------------------------------------------------------------------------*/
