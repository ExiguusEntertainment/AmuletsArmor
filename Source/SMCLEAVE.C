/*-------------------------------------------------------------------------*
 * File:  SMCLEAVE.C
 *-------------------------------------------------------------------------*/
/**
 * Leave group/game state machine.
 *
 * @addtogroup SMLEAVE
 * @brief State machine for leaving group/game
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLIENT.H"
#include "MEMORY.H"
#include "SMCLEAVE.H"
#include "SMMAIN.H"
#include "TICKER.H"

typedef struct {
    E_Boolean stateFlags[NUMBER_SMCLEAVE_FLAGS] ;
    T_word32 logoffTimeout ;
} T_SMCLeaveData ;

/* Internal prototypes: */
T_void SMCLeaveDataInit(T_stateMachineHandle handle) ;

T_void SMCLeaveDataFinish(T_stateMachineHandle handle) ;

T_void SMCLeaveSendLogoffEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLeaveSendLogoffIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLeaveHangUpEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCLeaveLeaveCompleteEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

/* Internal global variables: */
static T_stateMachineHandle G_smHandle ;
static E_Boolean G_init = FALSE ;
extern void HangUp(void);


static T_stateMachineConditional SMCLeaveSendLogoffCond[] = {
    {
        SMCLeaveCheckFlag,                            /* conditional callback */
        SMCLEAVE_FLAG_LOGOFF_COMPLETE,                /* extra data */
        SMCLEAVE_HANG_UP_STATE                        /* next state */
    },
    {
        SMCLeaveCheckFlag,                            /* conditional callback */
        SMCLEAVE_FLAG_TIMEOUT,                        /* extra data */
        SMCLEAVE_HANG_UP_STATE                        /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCLeaveHangUpCond[] = {
    {
        SMCLeaveCheckFlag,                            /* conditional callback */
        SMCLEAVE_FLAG_HANGUP_COMPLETE,                /* extra data */
        SMCLEAVE_LEAVE_COMPLETE_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/


static T_stateMachineState SMCLeaveStates[] = {
    /* SMCLEAVE_SEND_LOGOFF_STATE */
    {
        SMCLeaveSendLogoffEnter,                /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCLeaveSendLogoffIdle,                 /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCLeaveSendLogoffCond                  /* conditinal list. */
    },

    /* SMCLEAVE_HANG_UP_STATE */
    {
        SMCLeaveHangUpEnter,                    /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCLeaveHangUpCond                      /* conditinal list. */
    },

    /* SMCLEAVE_LEAVE_COMPLETE_STATE */
    {
        SMCLeaveLeaveCompleteEnter,             /* Enter callback */
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
static T_stateMachine SMCLeaveStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMCLeaveDataInit,        /* Init. callback */
    SMCLeaveDataFinish,      /* Finish callback */
    NUMBER_SMCLEAVE_STATES,   /* Number states */
    SMCLeaveStates           /* State list */
} ;



/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveInitialize
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveInitialize
 *
 *  @return Handle to state machine created
 *
 *<!-----------------------------------------------------------------------*/
T_stateMachineHandle SMCLeaveInitialize(T_void)
{
    DebugRoutine("SMCLeaveInitialize") ;

    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    G_smHandle = StateMachineCreate(&SMCLeaveStateMachine) ;
    StateMachineGotoState(G_smHandle, SMCLEAVE_INITIAL_STATE) ;

    DebugEnd() ;

    return G_smHandle ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveFinish
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveFinish(T_void)
{
    DebugRoutine("SMCLeaveFinish") ;

    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smHandle) ;
    G_smHandle = STATE_MACHINE_HANDLE_BAD ;

    G_init = FALSE ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveUpdate
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveUpdate
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveUpdate(T_void)
{
    DebugRoutine("SMCLeaveUpdate") ;

    StateMachineUpdate(G_smHandle) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveDataInit
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveDataInit
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveDataInit(T_stateMachineHandle handle)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveDataInit") ;

    p_data = MemAlloc(sizeof(T_SMCLeaveData)) ;

    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_SMCLeaveData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveDataFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveDataFinish
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveDataFinish(T_stateMachineHandle handle)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveDataFinish") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;

    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveCheckFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveCheckFlag
 *
 *  @param handle -- Handle to state machine
 *  @param flag -- Flag to change
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SMCLeaveCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveCheckFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < SMCLEAVE_FLAG_UNKNOWN) ;
    if (flag < SMCLEAVE_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;
    return stateFlag ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveSetFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveSetFlag
 *
 *  @param flag -- Flag to change
 *  @param state -- New state of flag
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveSetFlag(
           T_word16 flag,
           E_Boolean state)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveSetFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < SMCLEAVE_FLAG_UNKNOWN) ;
    if (flag < SMCLEAVE_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveSendLogoffEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveSendLogoffEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveSendLogoffEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveSendLogoffEnter") ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLeaveSetFlag(
        SMCLEAVE_FLAG_LOGOFF_COMPLETE,
        FALSE) ;
    SMCLeaveSetFlag(
        SMCLEAVE_FLAG_TIMEOUT,
        FALSE) ;

//puts("Sending logoff\n") ;
    ClientLogoff() ;

    p_data->logoffTimeout = TickerGet() ;

//    PromptStatusBarInit("Leaving server ... ", 10 * TICKS_PER_SECOND) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveSendLogoffIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveSendLogoffIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveSendLogoffIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLeaveData *p_data ;
    T_word32 ticksTaken ;

    DebugRoutine("SMCLeaveSendLogoffIdle") ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    ticksTaken = TickerGet() - p_data->logoffTimeout ;
//    if (ticksTaken < 10 * TICKS_PER_SECOND)
//        PromptStatusBarUpdate(ticksTaken) ;

    if (ClientIsLogin() == FALSE)  {
        SMCLeaveSetFlag(SMCLEAVE_FLAG_LOGOFF_COMPLETE, TRUE) ;
//        PromptStatusBarClose() ;
    }

    if (TickerGet() > p_data->logoffTimeout)  {
        SMCLeaveSetFlag(SMCLEAVE_FLAG_LOGOFF_COMPLETE, TRUE) ;
//        PromptStatusBarClose() ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveHangUpEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveHangUpEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveHangUpEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveHangUpEnter") ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCLeaveSetFlag(
        SMCLEAVE_FLAG_HANGUP_COMPLETE,
        FALSE) ;

    HangUp() ;

    SMCLeaveSetFlag(
        SMCLEAVE_FLAG_HANGUP_COMPLETE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCLeaveLeaveCompleteEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCLeaveLeaveCompleteEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCLeaveLeaveCompleteEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCLeaveData *p_data ;

    DebugRoutine("SMCLeaveLeaveCompleteEnter") ;

    p_data = (T_SMCLeaveData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* Done with this state machine. */
    SMMainSetFlag(SMMAIN_FLAG_LEAVE_SERVER_COMPLETE, TRUE) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SMCLEAVE.C
 *-------------------------------------------------------------------------*/
