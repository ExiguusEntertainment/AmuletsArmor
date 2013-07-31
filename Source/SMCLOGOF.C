/****************************************************************************/
/*    FILE:  SMCLOGOF.C                                                     */
/****************************************************************************/
#include "CLIENT.H"
#include "MAP.H"
#include "MEMORY.H"
#include "PACKET.H"
#include "SMCLOGOF.H"
#include "SMMAIN.H"
#include "STATS.H"

//#undef DebugRoutine
//#define DebugRoutine(name) { puts(name) ; DebugAddRoutine(name, __FILE__, __LINE__) ; }
typedef struct {
    E_Boolean stateFlags[NUMBER_SMCLOGOFF_FLAGS] ;
} T_SMCLogoffData ;

/* Internal prototypes: */
T_void SMCLogoffDataInit(T_stateMachineHandle handle) ;

T_void SMCLogoffDataFinish(T_stateMachineHandle handle) ;

T_void SMCLogoffSaveCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLogoffSaveCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLogoffTimeoutEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLogoffReportErrorEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLogoffExitEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

static T_void ISaveUploadComplete(void) ;

/* Internal global variables: */
static T_stateMachineHandle G_smHandle ;
static E_Boolean G_init = FALSE ;


/****************************************************************************/
/*                             CONDITIONALS                                 */
/****************************************************************************/
static T_stateMachineConditional SMCLogoffSaveCharacterCond[] = {
    {
        SMCLogoffCheckFlag,                           /* conditional callback */
        SMCLOGOFF_FLAG_SAVE_COMPLETE,                 /* extra data */
        SMCLOGOFF_EXIT_STATE                          /* next state */
    },
    {
        SMCLogoffCheckFlag,                           /* conditional callback */
        SMCLOGOFF_FLAG_TIMEOUT,                       /* extra data */
        SMCLOGOFF_TIMEOUT_STATE                       /* next state */
    },
    {
        SMCLogoffCheckFlag,                           /* conditional callback */
        SMCLOGOFF_FLAG_SAVE_ERROR,                    /* extra data */
        SMCLOGOFF_REPORT_ERROR_STATE                  /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCLogoffReportErrorCond[] = {
    {
        SMCLogoffCheckFlag,                           /* conditional callback */
        SMCLOGOFF_FLAG_REPORT_ERROR_COMPLETE,         /* extra data */
        SMCLOGOFF_EXIT_STATE                          /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/


/****************************************************************************/
/*                              STATES                                      */
/****************************************************************************/
static T_stateMachineState SMCLogoffStates[] = {
    /* SMCLOGOFF_SAVE_CHARACTER_STATE */
    {
        SMCLogoffSaveCharacterEnter,            /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCLogoffSaveCharacterIdle,             /* Idle callback */
        0,                                      /* Extra data */
        3,                                      /* Num conditionals */
        SMCLogoffSaveCharacterCond              /* conditinal list. */
    },

    /* SMCLOGOFF_TIMEOUT_STATE */
    {
        SMCLogoffTimeoutEnter,                  /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        0,                                      /* Num conditionals */
        NULL                                    /* conditinal list. */
    },

    /* SMCLOGOFF_REPORT_ERROR_STATE */
    {
        SMCLogoffReportErrorEnter,              /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCLogoffReportErrorCond                /* conditinal list. */
    },

    /* SMCLOGOFF_EXIT_STATE */
    {
        NULL,                                   /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        0,                                      /* Num conditionals */
        NULL                                    /* conditinal list. */
    }
} ;

/***************************************************************************/
/*                             STATE MACHINE                               */
/***************************************************************************/
static T_stateMachine SMCLogoffStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMCLogoffDataInit,        /* Init. callback */
    SMCLogoffDataFinish,      /* Finish callback */
    NUMBER_SMCLOGOFF_STATES,   /* Number states */
    SMCLogoffStates           /* State list */
} ;



/****************************************************************************/
/*  Routine:  SMCLogoffInitialize                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffInitialize                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_stateMachineHandle           -- Handle to state machine created     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_stateMachineHandle SMCLogoffInitialize(T_void)
{
    DebugRoutine("SMCLogoffInitialize") ;

    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    G_smHandle = StateMachineCreate(&SMCLogoffStateMachine) ;
    StateMachineGotoState(G_smHandle, SMCLOGOFF_INITIAL_STATE) ;

    DebugEnd() ;

    return G_smHandle ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffFinish                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffFinish                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
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
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffFinish(T_void)
{
    DebugRoutine("SMCLogoffFinish") ;

    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smHandle) ;
    G_smHandle = STATE_MACHINE_HANDLE_BAD ;

    G_init = FALSE ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffUpdate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffUpdate                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
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
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffUpdate(T_void)
{
    DebugRoutine("SMCLogoffUpdate") ;

    StateMachineUpdate(G_smHandle) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffDataInit                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffDataInit                                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffDataInit(T_stateMachineHandle handle)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffDataInit") ;

    p_data = MemAlloc(sizeof(T_SMCLogoffData)) ;

    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_SMCLogoffData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffDataFinish                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffDataFinish                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffDataFinish(T_stateMachineHandle handle)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffDataFinish") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;

    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffCheckFlag                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffCheckFlag                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*    T_word32 flag                -- Flag to change                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                                                             */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean SMCLogoffCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffCheckFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < SMCLOGOFF_FLAG_UNKNOWN) ;
    if (flag < SMCLOGOFF_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;
    return stateFlag ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffSetFlag                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffSetFlag                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 flag                -- Flag to change                        */
/*                                                                          */
/*    E_Boolean state              -- New state of flag                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffSetFlag(
           T_word16 flag,
           E_Boolean state)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffSetFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < SMCLOGOFF_FLAG_UNKNOWN) ;
    if (flag < SMCLOGOFF_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffSaveCharacterEnter                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffSaveCharacterEnter                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffSaveCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffSaveCharacterEnter") ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLogoffSetFlag(
        SMCLOGOFF_FLAG_SAVE_COMPLETE,
        FALSE) ;
    SMCLogoffSetFlag(
        SMCLOGOFF_FLAG_TIMEOUT,
        FALSE) ;
    SMCLogoffSetFlag(
        SMCLOGOFF_FLAG_SAVE_ERROR,
        FALSE) ;

    /* No matter what type of server we are on, save this one. */
    StatsSaveCharacter(StatsGetActive()) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffSaveCharacterIdle                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffSaveCharacterIdle                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffSaveCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLogoffData *p_data ;
    DebugRoutine("SMCLogoffSaveCharacterIdle") ;

    /* Send the character. */
    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    ISaveUploadComplete();

    DebugEnd() ;
}

/****************************************************************************/
/* Note that a create character upload is complete. */
/* LES 06/24/96 Created */
static T_void ISaveUploadComplete(void)
{
    DebugRoutine("ISaveUploadComplete") ;

    /* Note that the upload is complete. */
    SMCLogoffSetFlag(
        SMCLOGOFF_FLAG_SAVE_COMPLETE,
        TRUE) ;
    SMMainSetFlag(
        SMMAIN_FLAG_LOGOFF_COMPLETE,
        TRUE) ;

    /* Note that we are nowhere -- logoff state. */
    ClientSetCurrentPlace(0) ;
    ClientSetCurrentStartLocation(0) ;
    MapUnload() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffTimeoutEnter                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffTimeoutEnter                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffTimeoutEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffTimeoutEnter") ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMCLogoffReportErrorEnter                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMCLogoffReportErrorEnter                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*                                                                          */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMCLogoffGetExtraData                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffReportErrorEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffReportErrorEnter") ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLogoffSetFlag(
        SMCLOGOFF_FLAG_REPORT_ERROR_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMCLogoffExitEnter                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*    SMCLogoffExitEnter                                                    */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_stateMachineHandle handle  -- Handle to state machine               */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMCLogoffExitEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLogoffData *p_data ;

    DebugRoutine("SMCLogoffExitEnter") ;

    p_data = (T_SMCLogoffData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMMainSetFlag(SMMAIN_FLAG_LOGOFF_COMPLETE, TRUE) ;
    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SM.C                                                    */
/****************************************************************************/
