/*-------------------------------------------------------------------------*
 * File:  SMCDIS.C
 *-------------------------------------------------------------------------*/
/**
 * Handle disconnect as a state machine.
 *
 * @addtogroup SMCDIS
 * @brief Disconnect State Machine
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "MEMORY.H"
#include "SMCDIS.H"

typedef struct {
    E_Boolean stateFlags[NUMBER_SMCDISCONNECT_FLAGS] ;
} T_SMCDisconnectData ;

/* Internal prototypes: */
T_void SMCDisconnectDataInit(T_stateMachineHandle handle) ;

T_void SMCDisconnectDataFinish(T_stateMachineHandle handle) ;

T_void SMCDisconnectHangUpEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCDisconnectErrorMsgEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCDisconnectSaveCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCDisconnectExitToConnectEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

/* Internal global variables: */
static T_stateMachineHandle G_smHandle ;
static E_Boolean G_init = FALSE ;


static T_stateMachineConditional SMCDisconnectHangUpCond[] = {
    {
        SMCDisconnectCheckFlag,                       /* conditional callback */
        SMCDISCONNECT_FLAG_HANG_UP_COMPLETE,          /* extra data */
        SMCDISCONNECT_ERROR_MSG_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCDisconnectErrorMsgCond[] = {
    {
        SMCDisconnectCheckFlag,                       /* conditional callback */
        SMCDISCONNECT_FLAG_ERROR_MSG_COMPLETE,        /* extra data */
        SMCDISCONNECT_SAVE_CHARACTER_STATE            /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCDisconnectSaveCharacterCond[] = {
    {
        SMCDisconnectCheckFlag,                       /* conditional callback */
        SMCDISCONNECT_FLAG_SAVE_CHARACTER_COMPLETE,   /* extra data */
        SMCDISCONNECT_EXIT_TO_CONNECT_STATE           /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/


static T_stateMachineState SMCDisconnectStates[] = {
    /* SMCDISCONNECT_HANG_UP_STATE */
    {
        SMCDisconnectHangUpEnter,               /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCDisconnectHangUpCond                 /* conditinal list. */
    },

    /* SMCDISCONNECT_ERROR_MSG_STATE */
    {
        SMCDisconnectErrorMsgEnter,             /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCDisconnectErrorMsgCond               /* conditinal list. */
    },

    /* SMCDISCONNECT_SAVE_CHARACTER_STATE */
    {
        SMCDisconnectSaveCharacterEnter,        /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCDisconnectSaveCharacterCond          /* conditinal list. */
    },

    /* SMCDISCONNECT_EXIT_TO_CONNECT_STATE */
    {
        SMCDisconnectExitToConnectEnter,        /* Enter callback */
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
static T_stateMachine SMCDisconnectStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMCDisconnectDataInit,        /* Init. callback */
    SMCDisconnectDataFinish,      /* Finish callback */
    NUMBER_SMCDISCONNECT_STATES,   /* Number states */
    SMCDisconnectStates           /* State list */
} ;



/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectInitialize
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectInitialize
 *
 *  @return Handle to state machine created
 *
 *<!-----------------------------------------------------------------------*/
T_stateMachineHandle SMCDisconnectInitialize(T_void)
{
    DebugRoutine("SMCDisconnectInitialize") ;

    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    G_smHandle = StateMachineCreate(&SMCDisconnectStateMachine) ;
    StateMachineGotoState(G_smHandle, SMCDISCONNECT_INITIAL_STATE) ;

    DebugEnd() ;

    return G_smHandle ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectFinish
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectFinish(T_void)
{
    DebugRoutine("SMCDisconnectFinish") ;

    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smHandle) ;
    G_smHandle = STATE_MACHINE_HANDLE_BAD ;

    G_init = FALSE ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectUpdate
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectUpdate
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectUpdate(T_void)
{
    DebugRoutine("SMCDisconnectUpdate") ;

    StateMachineUpdate(G_smHandle) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectDataInit
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectDataInit
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectDataInit(T_stateMachineHandle handle)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectDataInit") ;

    p_data = MemAlloc(sizeof(T_SMCDisconnectData)) ;

    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_SMCDisconnectData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectDataFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectDataFinish
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectDataFinish(T_stateMachineHandle handle)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectDataFinish") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;

    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectCheckFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectCheckFlag
 *
 *  @param handle -- Handle to state machine
 *  @param flag -- Flag to change
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SMCDisconnectCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectCheckFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < SMCDISCONNECT_FLAG_UNKNOWN) ;
    if (flag < SMCDISCONNECT_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;
    return stateFlag ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectSetFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectSetFlag
 *
 *  @param flag -- Flag to change
 *  @param state -- New state of flag
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectSetFlag(
           T_word16 flag,
           E_Boolean state)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectSetFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < SMCDISCONNECT_FLAG_UNKNOWN) ;
    if (flag < SMCDISCONNECT_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectHangUpEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectHangUpEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectHangUpEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectHangUpEnter") ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectSetFlag(
        SMCDISCONNECT_FLAG_HANG_UP_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectErrorMsgEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectErrorMsgEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectErrorMsgEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectErrorMsgEnter") ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectSetFlag(
        SMCDISCONNECT_FLAG_ERROR_MSG_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectSaveCharacterEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectSaveCharacterEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectSaveCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectSaveCharacterEnter") ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCDisconnectSetFlag(
        SMCDISCONNECT_FLAG_SAVE_CHARACTER_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCDisconnectExitToConnectEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCDisconnectExitToConnectEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCDisconnectExitToConnectEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCDisconnectData *p_data ;

    DebugRoutine("SMCDisconnectExitToConnectEnter") ;

    p_data = (T_SMCDisconnectData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SMCDIS.C
 *-------------------------------------------------------------------------*/
