/****************************************************************************/
/*    FILE:  SMMAIN.C                                                       */
/****************************************************************************/
#include "CLIENT.H"
#include "MEMORY.H"
#include "SMCONNEC.H"
#include "SMCCHOOS.H"
#include "SMCDIS.H"
#include "SMCLEAVE.H"
#include "SMCLOGOF.H"
#include "SMCPLAY.H"
#include "SMMAIN.H"

//#undef DebugRoutine
//#define DebugRoutine(name) { puts(name) ; DebugAddRoutine(name, __FILE__, __LINE__) ; }

typedef struct {
    E_Boolean stateFlags[NUMBER_SMMAIN_FLAGS] ;
} T_smMainData ;

/* Internal prototypes: */
static T_smMainData *ISMMainGetExtraData(T_void) ;

/* Entry exit code: */
T_void SMMainConnectStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
T_void SMMainConnectEnd(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;
T_void SMMainConnectUpdate(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainChooseStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainChooseCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainChooseCharacterExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMMainLeaveServerStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainPlayGameStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainPlayGameIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainPlayGameExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMMainLogoffEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainLogoffIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainLogoffExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMMainDisconnectedEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainDisconnectedIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainDisconnectedExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMMainLeaveServerIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMMainLeaveServerExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

/***************************************************************************/
/*                             CONDITIONALS                                */
/***************************************************************************/
static T_stateMachineConditional SMMainConnectCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_CONNECT_EXIT,          /* extra data */
        SMMAIN_STATE_EXIT_GAME             /* next state */
    },
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_CONNECT_COMPLETE,      /* extra data */
        SMMAIN_STATE_CHOOSE_CHARACTER      /* next state */
    }
} ;

static T_stateMachineConditional SMMainChooseCharacterCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_LEAVE_SERVER,          /* extra data */
        SMMAIN_STATE_LEAVE_SERVER          /* next state */
    },
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_BEGIN_GAME,            /* extra data */
        SMMAIN_STATE_PLAY_GAME             /* next state */
    },
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_DROPPED,               /* extra data */
        SMMAIN_STATE_DISCONNECTED          /* next state */
    }
} ;

static T_stateMachineConditional SMMainPlayGameCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_END_GAME,              /* extra data */
        SMMAIN_STATE_LOGOFF_CHARACTER      /* next state */
//REMOVED FOR DEMO        SMMAIN_STATE_CHOOSE_CHARACTER      /* next state */
//        SMMAIN_STATE_EXIT_GAME
    },
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_DROPPED,               /* extra data */
        SMMAIN_STATE_DISCONNECTED          /* next state */
    }
} ;

static T_stateMachineConditional SMMainLogoffCharacterCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_LOGOFF_COMPLETE,       /* extra data */
        SMMAIN_STATE_CHOOSE_CHARACTER      /* next state */
    }
} ;

static T_stateMachineConditional SMMainLeaveServerCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_LEAVE_SERVER_COMPLETE, /* extra data */
        SMMAIN_STATE_CONNECT               /* next state */
    },
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_END_GAME,              /* extra data */
        SMMAIN_STATE_EXIT_GAME             /* next state */
    }
} ;

static T_stateMachineConditional SMMainDisconnectedCond[] = {
    {
        SMMainCheckFlag,                   /* conditional callback */
        SMMAIN_FLAG_DISCONNECT_COMPLETE,   /* extra data */
        SMMAIN_STATE_CONNECT               /* next state */
    }
} ;

/***************************************************************************/
/*                             STATES                                      */
/***************************************************************************/

static T_stateMachineState SMMainStates[] = {
    /* SMMAIN_STATE_CONNECT */
    {
        SMMainConnectStart,                      /* Enter callback */
        SMMainConnectEnd,                        /* Exit callback */
        SMMainConnectUpdate,                     /* Idle callback */
        0,                                       /* Extra data */
        2,                                       /* Num conditionals */
        SMMainConnectCond                        /* conditional list. */
    },

    /* SMMAIN_STATE_CHOOSE_CHARACTER */
    {
        SMMainChooseStart,                       /* Enter callback */
        SMMainChooseCharacterExit,              /* Exit callback */
        SMMainChooseCharacterIdle,              /* Idle callback */
        0,                                       /* Extra data */
        3,                                       /* Num conditionals */
        SMMainChooseCharacterCond                /* conditional list. */
    },

    /* SMMAIN_STATE_PLAY_GAME */
    {
        SMMainPlayGameStart,                     /* Enter callback */
        SMMainPlayGameExit,                     /* Exit callback */
        SMMainPlayGameIdle,                     /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMMainPlayGameCond                      /* conditinal list. */
    },

    /* SMMAIN_STATE_LOGOFF_CHARACTER */
    {
        SMMainLogoffEnter,                      /* Enter callback */
        SMMainLogoffExit,                       /* Exit callback */
        SMMainLogoffIdle,                       /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMMainLogoffCharacterCond                /* conditional list. */
    },

    /* SMMAIN_STATE_LEAVE_SERVER */
    {
        SMMainLeaveServerStart,                  /* Enter callback */
        SMMainLeaveServerExit,                  /* Exit callback */
        SMMainLeaveServerIdle,                  /* Idle callback */
        0,                                       /* Extra data */
        2,                                       /* Num conditionals */
        SMMainLeaveServerCond                    /* conditional list. */
    },

    /* SMMAIN_STATE_EXIT_GAME  -- Last state, no exits */
    {
        NULL,                                    /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        0,                                       /* Num conditionals */
        NULL                                     /* conditional list. */
    },

    /* SMMAIN_STATE_DISCONNECTED */
    {
        SMMainDisconnectedEnter,                /* Enter callback */
        SMMainDisconnectedExit,                 /* Exit callback */
        SMMainDisconnectedIdle,                 /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMMainDisconnectedCond                   /* conditional list. */
    }
} ;

/***************************************************************************/
/*                             STATE MACHINE                               */
/***************************************************************************/

static T_stateMachine SMMainStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMMainInitData,                 /* Init. callback */
    SMMainFinishData,               /* Finish callback */
    NUMBER_SMMAIN_STATES,           /* Number states */
    SMMainStates                    /* State list */
} ;

static E_Boolean G_init = FALSE ;
static T_stateMachineHandle G_smMainHandle = STATE_MACHINE_HANDLE_BAD ;

/****************************************************************************/
/*  Routine:  SMMainCheckFlag                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainCheckFlag checks to see if one of the state flags      */
/*  of the client connect state machine is set.                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 flag                -- Flag to get value of                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                    -- State of flag                         */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StateMachineGetExtraData                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean SMMainCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_smMainData *p_data ;

    DebugRoutine("SMMainCheckFlag") ;
    DebugCheck(G_smMainHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < SMMAIN_FLAG_UNKNOWN) ;
    if (flag < SMMAIN_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;

    return stateFlag ;
}

/****************************************************************************/
/*  Routine:  SMMainSetFlag                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainSetFlag sets a flag for the main state                          */
/*  machine to process as part of its states.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 flag                -- Flag to set                           */
/*                                                                          */
/*    E_Boolean state              -- State to set flag to                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StateMachineGetExtraData                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainSetFlag(
              T_word32 flag,
              E_Boolean state)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainSetFlag") ;
    DebugCheck(G_smMainHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < SMMAIN_FLAG_UNKNOWN) ;
    if (flag < SMMAIN_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainInit                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainInit sets up the main state machine and its data.               */
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
/*    StateMachineCreate                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_stateMachineHandle SMMainInit(T_void)
{
    DebugRoutine("SMMainInit") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;
    G_smMainHandle = StateMachineCreate(&SMMainStateMachine) ;
    StateMachineGotoState(G_smMainHandle, SMMAIN_INITIAL_STATE) ;

    DebugEnd() ;

    return G_smMainHandle ;
}

/****************************************************************************/
/*  Routine:  SMMainFinish                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainFinish cleans up the main state machine's data.                 */
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
/*    StateMachineDestroy                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainFinish(T_void)
{
    DebugRoutine("SMMainFinish") ;
    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smMainHandle) ;
    G_smMainHandle = STATE_MACHINE_HANDLE_BAD ;

    G_init = FALSE ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainUpdate                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainUpdate updates the main state machine.                          */
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
/*    StateMachineUpdate                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainUpdate(T_void)
{
    DebugRoutine("SMMainUpdate") ;

    StateMachineUpdate(G_smMainHandle) ;
//    if (CommCheckClientAndServerExist())
//        ServerUpdate() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainIsDone                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainIsDone checks if the main state machine                         */
/*  has reached its end and is done.                                        */
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
/*    E_Boolean                   -- Done if TRUE                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StateMachineGetState                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean SMMainIsDone(T_void)
{
    T_word16 state;
    E_Boolean isDone = FALSE ;

    DebugRoutine("SMMainIsDone") ;

    state = StateMachineGetState(G_smMainHandle) ;

    if (state == SMMAIN_STATE_EXIT_GAME)
        isDone = TRUE ;

    DebugEnd() ;

    return isDone ;
}

/****************************************************************************/
/*  Routine:  SMMainInitData                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainInitData creates the data necessary to be associated            */
/*  to the MAIN STATE MACHINE.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle -- state machine                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    memset                                                                */
/*    StateMachineSetData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainInitData(T_stateMachineHandle handle)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainInitData") ;

    p_data = MemAlloc(sizeof(T_smMainData)) ;
    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_smMainData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainFinishData                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainFinishData destroys the data attached to the main state machine.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle -- state machine                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StateMachineGetData                                                   */
/*    MemFree                                                               */
/*    StateMachineSetData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainFinishData(T_stateMachineHandle handle)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainFinishData") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ISMMainGetExtraData            * INTERNAL *                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISMMainGetExtraData returns a pointer to the current main SM          */
/*           global data area.                                              */
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
/*    T_smMainData *            -- SMMain's data                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StateMachineGetExtraData                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_smMainData *ISMMainGetExtraData(T_void)
{
    T_smMainData *p_data = NULL ;

    DebugRoutine("ISMMainGetExtraData") ;
    DebugCheck(G_init == TRUE) ;

    if (G_init)  {
        p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
        DebugCheck(p_data != NULL) ;
    }

    DebugEnd() ;

    return p_data ;
}

/****************************************************************************/
/*  Routine:  SMMainConnectStart                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainConnectStart                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainConnectStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainConnectStart") ;

    /* Initialize the client connect state machine. */
    if (ClientGetConnectionType() == CLIENT_CONNECTION_TYPE_SINGLE)  {
        SMMainSetFlag(SMMAIN_FLAG_CONNECT_COMPLETE, TRUE) ;
    } else {
        SMClientConnectInit() ;

        SMMainSetFlag(
            SMMAIN_FLAG_CONNECT_EXIT,
            FALSE) ;
        SMMainSetFlag(
            SMMAIN_FLAG_CONNECT_COMPLETE,
            FALSE) ;
    }

    KeyboardBufferOn() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainConnectEnd                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainConnectEnd                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
/*                                                                          */
/*    T_word32 extraData           -- Not used                              */
/*                                                                          */
/*    E_Boolean isDestroy          -- Flag noting if destroying             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainConnectEnd(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    DebugRoutine("SMMainConnectEnd") ;

    if (ClientGetConnectionType() != CLIENT_CONNECTION_TYPE_SINGLE)  {
        /* Code goes here. */
        SMClientConnectFinish() ;
    }

    KeyboardBufferOff() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainConnectUpdate                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainConnectUpdate                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainConnectUpdate(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainConnectStart") ;

    if (ClientGetConnectionType() != CLIENT_CONNECTION_TYPE_SINGLE)  {
        /* Update the client state machine. */
        SMClientConnectUpdate() ;
    }

    if (SMClientConnectIsDone())
        SMMainSetFlag(
            SMMAIN_FLAG_CONNECT_EXIT,
            TRUE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainChooseStart                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainChooseStart brings up the main screen of text and info and      */
/*  lets the player create/destroy/play a given character.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
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
/*    SMMainSetFlag                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainChooseStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainChooseStart") ;

    SMMainSetFlag(
        SMMAIN_FLAG_BEGIN_GAME,
        FALSE) ;
    SMMainSetFlag(
        SMMAIN_FLAG_LEAVE_SERVER,
        FALSE) ;

    /* Initialize the client connect state machine. */
    SMCChooseInitialize() ;

    KeyboardBufferOn() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainChooseCharacterIdle                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainChooseCharacterIdle                                               */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainChooseCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainChooseCharacterIdle") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseUpdate() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainChooseCharacterExit                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainChooseCharacterExit                                               */
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
/*    E_Boolean isDestroyed        -- TRUE if state machine is being destroy*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainChooseCharacterExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainChooseCharacterExit") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseFinish() ;

    KeyboardBufferOff() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLeaveServerStart                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainLeaveServerStart starts the transaction leaving the server.     */
/*  To be polite, the system "logs off" from the server.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
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
/*    SMMainSetFlag                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLeaveServerStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainLeaveServerStart") ;

    SMMainSetFlag(SMMAIN_FLAG_LEAVE_SERVER_COMPLETE, FALSE) ;

    if (ClientGetConnectionType() == CLIENT_CONNECTION_TYPE_SINGLE)  {
        /* Leave as soon as possible. */
        SMMainSetFlag(SMMAIN_FLAG_END_GAME, TRUE) ;

        /* Got to exit. */
//        StateMachineGotoState(handle, SMMAIN_STATE_EXIT_GAME) ;
    } else {
        SMCLeaveInitialize() ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLeaveServerIdle                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainLeaveServerIdle                                                   */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLeaveServerIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainLeaveServerIdle") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    if (ClientGetConnectionType() != CLIENT_CONNECTION_TYPE_SINGLE)  {
        SMCLeaveUpdate() ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLeaveServerExit                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainLeaveServerExit                                                   */
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
/*    E_Boolean isDestroyed        -- TRUE if state machine is being destroy*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLeaveServerExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainLeaveServerExit") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    if (ClientGetConnectionType() != CLIENT_CONNECTION_TYPE_SINGLE)  {
        SMCLeaveFinish() ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainPlayGameStart                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SMMainPlayGameStart gets the 3d engine going and appropriate game     */
/*  state machine.                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to PHASE A client state machine*/
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
/*    SMMainSetFlag                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainPlayGameStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainPlayGameStart") ;

    SMMainSetFlag(
        SMMAIN_FLAG_DROPPED,
        FALSE) ;
    SMMainSetFlag(
        SMMAIN_FLAG_END_GAME,
        FALSE) ;

    /* Start up the play game state. */
    SMCPlayGameInitialize() ;

    MouseRelativeModeOff();

    /* Go to the first allowable place. */
    ClientGotoPlace(20004, 0) ;
//    ClientGotoPlace(1, 0) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SMMainPlayGameIdle                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainPlayGameIdle                                                      */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainPlayGameIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMMainPlayGameIdle") ;

    /* Play the game. */
    SMCPlayGameUpdate() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainPlayGameExit                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainPlayGameExit                                                      */
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
/*    E_Boolean isDestroyed        -- TRUE if state machine is being destroy*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainPlayGameExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainPlayGameExit") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

/* TESTING */
//ControlFinish() ;
    /* Leave the play game state totally. */
    SMCPlayGameFinish() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLogoffEnter                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainLogoffEnter                                                       */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLogoffEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainLogoffEnter") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLogoffInitialize() ;

    SMMainSetFlag(
        SMMAIN_FLAG_DROPPED,
        FALSE) ;
    SMMainSetFlag(
        SMMAIN_FLAG_LOGOFF_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLogoffIdle                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainLogoffIdle                                                        */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLogoffIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainLogoffIdle") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLogoffUpdate() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainLogoffExit                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainLogoffExit                                                        */
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
/*    E_Boolean isDestroyed        -- TRUE if state machine is being destroy*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainLogoffExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainLogoffExit") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLogoffFinish() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainDisconnectedEnter                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainDisconnectedEnter                                                 */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainDisconnectedEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainDisconnectedEnter") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectInitialize() ;

    SMMainSetFlag(
        SMMAIN_FLAG_DISCONNECT_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainDisconnectedIdle                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainDisconnectedIdle                                                  */
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
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainDisconnectedIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainDisconnectedIdle") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectUpdate() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  SMMainDisconnectedExit                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  SMMainDisconnectedExit                                                  */
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
/*    E_Boolean isDestroyed        -- TRUE if state machine is being destroy*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISMMainGetExtraData                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SMMainDisconnectedExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smMainData *p_data ;

    DebugRoutine("SMMainDisconnectedExit") ;

    p_data = (T_smMainData *)StateMachineGetExtraData(G_smMainHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectFinish() ;

    DebugEnd() ;
}


/****************************************************************************/
/*    END OF FILE:  SMMAIN.C                                                */
/****************************************************************************/
