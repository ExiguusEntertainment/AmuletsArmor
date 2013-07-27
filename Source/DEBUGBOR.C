/****************************************************************************/
/*    FILE:  DEBUG.C                                                        */
/****************************************************************************/

#include "standard.h"

#ifndef NDEBUG

/* The calling stack is a list of pointers to routine names defined
   else where. */
static T_byte8 *G_CallStack[DEBUG_MAX_STACK_DEPTH] ;
static T_byte8 *G_CallStackFile[DEBUG_MAX_STACK_DEPTH] ;
static T_word16 G_CallStackLine[DEBUG_MAX_STACK_DEPTH] ;

/* The following tells where the last access to the calling stack has
   been made. */
static T_word16 G_StackPosition = 0 ;

/****************************************************************************/
/*  Routine: DebugAddRoutine                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DebugAddRoutine is NOT called directly.  It is called by the use      */
/*  of the macro "DebugRoutine."  DebugRoutine is used to declare that      */
/*  the program is entering a section of code that needs to be debugged.    */
/*  The name of the routine is added to a call stack and will be removed    */
/*  later by DebugRemoveRoutine.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The current system only allows DEBUG_MAX_STACK_DEPTH levels of        */
/*  calling.  Should you go deeper, this routine will create an error.      */
/*  Also, this stack is only coherent as long as calls are made in each     */
/*  functions.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_routineName      -- String to add to call stack            */
/*                                                                          */
/*    Assumptoings:                                                         */
/*        p_routineName is not NULL.                                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    DebugCheck                                                            */
/*    DebugFail                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*    LES  12/12/94  Modified to handle file name and line numbers          */
/*                                                                          */
/****************************************************************************/

T_void DebugAddRoutine(
           T_byte8 *p_routineName,
           T_byte8 *p_filename,
           T_word16 lineNum)
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
        G_CallStackLine[G_StackPosition] = lineNum ;
        G_CallStack[G_StackPosition++] = p_routineName ;
    }
}

/****************************************************************************/
/*  Routine:                                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*     Debug Fail is called when a DebugCheck macro finds an illegal        */
/*  assumption.  It will call this routine and expect the system to print   */
/*  out the error messages both to the screen and to an "ERROR.LOG."        */
/*  A list of routines is also printed out.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*     This current version does not change to text mode.                   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*     T_byte8 *p_msg             -- Message as to why it died/failed       */
/*                                                                          */
/*     T_byte8 *p_file            -- Name of file it died in                */
/*                                                                          */
/*     T_word16 line              -- Line number where failure occured      */
/*                                                                          */
/*     Assumptions:                                                         */
/*        All of the above are assumed to be valid pointers.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*     "ERROR.LOG"                -- File containing explanation            */
/*                                                                          */
/*     Assumptions:                                                         */
/*        I'm assuming that we can still open a file and do a dump.         */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*     fclose                                                               */
/*     fflush                                                               */
/*     fopen                                                                */
/*     fprintf                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*    LES  12/12/94  Added to stack trace all the files and line numbers.   */
/*                                                                          */
/****************************************************************************/

T_void DebugFail(T_byte8 *p_msg, T_byte8 *p_file, T_word16 line)
{
    FILE *fp ;

    /* Open a file for the error log. */
    fp = fopen("error.log", "w") ;
    fprintf(fp, "Failure: %s (FILE: %s  LINE: %d)\n", p_msg, p_file, line) ;
    fprintf(stderr, "Failure: %s (FILE: %s  LINE: %d)\n", p_msg, p_file, line) ;

    fprintf(fp, "Call stack:\n") ;
    fprintf(stderr, "Call stack:\n") ;

    /* Loop for each stack call. */
    while (G_StackPosition)  {
        /* Decrement the stack position. */
        G_StackPosition-- ;

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

    /* Done with dump. */
    fflush(fp) ;
    fclose(fp) ;

    /* Do a hard exit. */
    abort();
}

/****************************************************************************/
/*  Routine:  DebugRemoveRoutine                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*     DebugRemoveRoutine is not called directly.  It is called by the      */
/*  macro "DebugEnd" which is used at the end of a debugged routine.        */
/*  This routine removes the text that was added to the calling stack       */
/*  by "DebugAddRoutine" (called via DebugRoutine).                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None that I know of.                                                  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DebugRemoveRoutine(T_void)
{
    /* First see if we are allowed to pop of from the stack. */
    DebugCheck(G_StackPosition > 0) ;

    /* OK, it must be good. */
    G_StackPosition-- ;
}

T_void DebugGetCaller(T_byte8 **filename, T_word16 *line)
{
    *filename = G_CallStack[G_StackPosition-2] ;
    *line = G_CallStackLine[G_StackPosition-2] ;
}

#endif
/****************************************************************************/
/*    END OF FILE:  DEBUG.C                                                 */
/****************************************************************************/

