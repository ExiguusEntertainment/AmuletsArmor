/*-------------------------------------------------------------------------*
 * File:  SMCCHOOS.C
 *-------------------------------------------------------------------------*/
/**
 * Handle the Main UI as a state machine.
 *
 * @addtogroup SMCCHOOS
 * @brief Choose Character State Machine
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONTROL.H"
#include "MAP.H"
#include "MAINUI.H"
#include "MEMORY.H"
#include "PACKET.H"
#include "PROMPT.H"
#include "SMCCHOOS.H"
#include "SMMAIN.H"
#include "STATS.H"

typedef struct {
    E_Boolean stateFlags[NUMBER_SMCCHOOSE_FLAGS] ;
    T_byte8 attemptPassword[MAX_SIZE_PASSWORD+100] ;
} T_SMCChooseData ;

/* Internal prototypes: */
T_void SMCChooseDataInit(T_stateMachineHandle handle) ;

T_void SMCChooseDataFinish(T_stateMachineHandle handle) ;

T_void SMCChooseWaitForListEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseWaitForListIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseChoicesEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseChoicesIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseChoicesExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMCChooseCreateEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseCreateExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMCChooseCreateIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseRequestCreateEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseRequestCreateIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseLoadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseLoadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDownloadCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDisplayStatsEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDisplayStatsIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDisplayStatsExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMCChooseCheckPasswordForLoadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseCheckPasswordForLoadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseChangePasswordEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseChangePasswordIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseEnableBeginEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDeleteEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDeleteExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMCChooseDeleteCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseDeleteCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChoosePlayGameEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseExitEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseCreateUploadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMCChooseCreateUploadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

static T_void ICreateUploadComplete(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData) ;

/* Internal global variables: */
static T_stateMachineHandle G_smHandle ;
static E_Boolean G_init = FALSE ;


static T_stateMachineConditional SMCChooseWaitForListCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_ENTER_COMPLETE,                /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseChoicesCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_EXIT,                          /* extra data */
        SMCCHOOSE_EXIT_STATE                          /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHOOSE_CREATE,                 /* extra data */
        SMCCHOOSE_CREATE_STATE                        /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHOOSE_LOAD,                   /* extra data */
        SMCCHOOSE_LOAD_STATE                          /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHOOSE_DELETE,                 /* extra data */
        SMCCHOOSE_DELETE_STATE                        /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHOOSE_REDRAW,                 /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseCreateCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_COMPLETE,               /* extra data */
        SMCCHOOSE_REQUEST_CREATE_STATE                /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_ABORT,                  /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseRequestCreateCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_STATUS_OK,              /* extra data */
        SMCCHOOSE_CREATE_UPLOAD_STATE                 /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_STATUS_ERROR,           /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_ABORT,                  /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseLoadCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_LOAD_STATUS_OK,                /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_LOAD_STATUS_INCORRECT,         /* extra data */
        SMCCHOOSE_DOWNLOAD_CHARACTER_STATE            /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseDownloadCharacterCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_DOWNLOAD_COMPLETE,             /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseDisplayStatsCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_BEGIN,                         /* extra data */
//        SMCCHOOSE_PLAY_GAME_STATE                     /* next state */
        SMCCHOOSE_CHECK_PASSWORD_FOR_LOAD_STATE       /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_PASSWORD_ENTERED,              /* extra data */
        SMCCHOOSE_CHECK_PASSWORD_FOR_LOAD_STATE       /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHANGE_PASSWORD,               /* extra data */
        SMCCHOOSE_CHANGE_PASSWORD_STATE               /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_EXIT,                          /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseCheckPasswordForLoadCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_PASSWORD_OK,                   /* extra data */
//        SMCCHOOSE_ENABLE_BEGIN_STATE                  /* next state */
        SMCCHOOSE_PLAY_GAME_STATE                     /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_PASSWORD_NOT_OK,               /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseChangePasswordCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,      /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CHANGE_PASSWORD_ABORT,         /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseEnableBeginCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_ENABLE_COMPLETE,               /* extra data */
        SMCCHOOSE_DISPLAY_STATS_STATE                 /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseDeleteCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_DELETE_PASSWORD_OK,            /* extra data */
        SMCCHOOSE_DELETE_CHARACTER_STATE              /* next state */
    },
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_DELETE_PASSWORD_NOT_OK,        /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseDeleteCharacterCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_DELETE_COMPLETE,               /* extra data */
        SMCCHOOSE_CHOICES_STATE                       /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMCChooseCreateUploadCond[] = {
    {
        SMCChooseCheckFlag,                           /* conditional callback */
        SMCCHOOSE_FLAG_CREATE_UPLOAD_OK,              /* extra data */
        SMCCHOOSE_PLAY_GAME_STATE                     /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/


static T_stateMachineState SMCChooseStates[] = {
    /* SMCCHOOSE_WAIT_FOR_LIST_STATE */
    {
        SMCChooseWaitForListEnter,              /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseWaitForListIdle,               /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCChooseWaitForListCond                /* conditinal list. */
    },

    /* SMCCHOOSE_CHOICES_STATE */
    {
        SMCChooseChoicesEnter,                  /* Enter callback */
        SMCChooseChoicesExit,                   /* Exit callback */
        SMCChooseChoicesIdle,                   /* Idle callback */
        0,                                      /* Extra data */
        5,                                      /* Num conditionals */
        SMCChooseChoicesCond                    /* conditinal list. */
    },

    /* SMCCHOOSE_CREATE_STATE */
    {
        SMCChooseCreateEnter,                   /* Enter callback */
        SMCChooseCreateExit,                    /* Exit callback */
        SMCChooseCreateIdle,                    /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCChooseCreateCond                     /* conditinal list. */
    },

    /* SMCCHOOSE_REQUEST_CREATE_STATE */
    {
        SMCChooseRequestCreateEnter,            /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseRequestCreateIdle,             /* Idle callback */
        0,                                      /* Extra data */
        3,                                      /* Num conditionals */
        SMCChooseRequestCreateCond              /* conditinal list. */
    },

    /* SMCCHOOSE_LOAD_STATE */
    {
        SMCChooseLoadEnter,                     /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseLoadIdle,                      /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCChooseLoadCond                       /* conditinal list. */
    },

    /* SMCCHOOSE_DOWNLOAD_CHARACTER_STATE */
    {
        SMCChooseDownloadCharacterEnter,        /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCChooseDownloadCharacterCond          /* conditinal list. */
    },

    /* SMCCHOOSE_DISPLAY_STATS_STATE */
    {
        SMCChooseDisplayStatsEnter,             /* Enter callback */
        SMCChooseDisplayStatsExit,              /* Exit callback */
        SMCChooseDisplayStatsIdle,              /* Idle callback */
        0,                                      /* Extra data */
        4,                                      /* Num conditionals */
        SMCChooseDisplayStatsCond               /* conditinal list. */
    },

    /* SMCCHOOSE_CHECK_PASSWORD_FOR_LOAD_STATE */
    {
        SMCChooseCheckPasswordForLoadEnter,     /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseCheckPasswordForLoadIdle,      /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCChooseCheckPasswordForLoadCond       /* conditinal list. */
    },

    /* SMCCHOOSE_CHANGE_PASSWORD_STATE */
    {
        SMCChooseChangePasswordEnter,           /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseChangePasswordIdle,            /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCChooseChangePasswordCond             /* conditinal list. */
    },

    /* SMCCHOOSE_ENABLE_BEGIN_STATE */
    {
        SMCChooseEnableBeginEnter,              /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCChooseEnableBeginCond                /* conditinal list. */
    },

    /* SMCCHOOSE_DELETE_STATE */
    {
        SMCChooseDeleteEnter,                   /* Enter callback */
        SMCChooseDeleteExit,                    /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        2,                                      /* Num conditionals */
        SMCChooseDeleteCond                     /* conditinal list. */
    },

    /* SMCCHOOSE_DELETE_CHARACTER_STATE */
    {
        SMCChooseDeleteCharacterEnter,          /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseDeleteCharacterIdle,           /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCChooseDeleteCharacterCond            /* conditinal list. */
    },

    /* SMCCHOOSE_PLAY_GAME_STATE */
    {
        SMCChoosePlayGameEnter,                 /* Enter callback */
        NULL,                                   /* Exit callback */
        NULL,                                   /* Idle callback */
        0,                                      /* Extra data */
        0,                                      /* Num conditionals */
        NULL                                    /* conditinal list. */
    },

    /* SMCCHOOSE_CREATE_UPLOAD_STATE */
    {
        SMCChooseCreateUploadEnter,             /* Enter callback */
        NULL,                                   /* Exit callback */
        SMCChooseCreateUploadIdle,              /* Idle callback */
        0,                                      /* Extra data */
        1,                                      /* Num conditionals */
        SMCChooseCreateUploadCond               /* conditinal list. */
    },

    /* SMCCHOOSE_EXIT_STATE */
    {
        SMCChooseExitEnter,                     /* Enter callback */
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
static T_stateMachine SMCChooseStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMCChooseDataInit,        /* Init. callback */
    SMCChooseDataFinish,      /* Finish callback */
    NUMBER_SMCCHOOSE_STATES,   /* Number states */
    SMCChooseStates           /* State list */
} ;



/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseInitialize
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseInitialize
 *
 *  @return Handle to state machine created
 *
 *<!-----------------------------------------------------------------------*/
T_stateMachineHandle SMCChooseInitialize(T_void)
{
    DebugRoutine("SMCChooseInitialize") ;

    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    G_smHandle = StateMachineCreate(&SMCChooseStateMachine) ;
    StateMachineGotoState(G_smHandle, SMCCHOOSE_INITIAL_STATE) ;

    DebugEnd() ;

    return G_smHandle ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseFinish
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseFinish(T_void)
{
    DebugRoutine("SMCChooseFinish") ;

    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smHandle) ;
    G_smHandle = STATE_MACHINE_HANDLE_BAD ;

    G_init = FALSE ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseUpdate
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseUpdate
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseUpdate(T_void)
{
    DebugRoutine("SMCChooseUpdate") ;

    StateMachineUpdate(G_smHandle) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDataInit
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDataInit
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDataInit(T_stateMachineHandle handle)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDataInit") ;

    p_data = MemAlloc(sizeof(T_SMCChooseData)) ;

    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_SMCChooseData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDataFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDataFinish
 *
 *  @param handle -- Handle to state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDataFinish(T_stateMachineHandle handle)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDataFinish") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;

    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCheckFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCheckFlag
 *
 *  @param handle -- Handle to state machine
 *  @param flag -- Flag to change
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SMCChooseCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCheckFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < SMCCHOOSE_FLAG_UNKNOWN) ;
    if (flag < SMCCHOOSE_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;
    return stateFlag ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseSetFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseSetFlag
 *
 *  @param flag -- Flag to change
 *  @param state -- New state of flag
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseSetFlag(
           T_word16 flag,
           E_Boolean state)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseSetFlag") ;
    DebugCheck(G_smHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < SMCCHOOSE_FLAG_UNKNOWN) ;
    if (flag < SMCCHOOSE_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseWaitForListEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseWaitForListEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseWaitForListEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;
    T_statsSavedCharArray *p_charArray ;

    DebugRoutine("SMCChooseWaitForListEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* make stats get the current list of characters */
//    StatsGetSavedCharacterList();

    /* Note that enter is not complete. */
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_ENTER_COMPLETE,
        FALSE) ;

    /* Get the saved character listing. */
    p_charArray = StatsGetSavedCharacterList() ;

    /* Make this listing active. */
    StatsSetSavedCharacterList(p_charArray) ;

    /* Note that we are done "entering" the character list. */
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_ENTER_COMPLETE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseWaitForListIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseWaitForListIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseWaitForListIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseWaitForListIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseChoicesEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseChoicesEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseChoicesEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseChoicesEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_EXIT,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHOOSE_CREATE,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHOOSE_LOAD,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHOOSE_DELETE,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHOOSE_REDRAW,
        FALSE) ;

    ControlInitForJustUI() ;
    MainUIStart() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseChoicesIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseChoicesIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseChoicesIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseChoicesIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;
    MainUIUpdate() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseChoicesExit
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseChoicesExit
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- TRUE if state machine is being destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseChoicesExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseChoicesExit") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    ControlFinish() ;
    MainUIEnd() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCreateEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCreateEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCreateEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCreateEnter") ;
//puts("SMCChooseCreateEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_COMPLETE,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_ABORT,
        FALSE) ;

    ControlInitForJustUI() ;
    StatsCreateCharacterUIStart() ;

    /* Reset the time of day to 2:00 pm for created characters. */
    MapSetDayOffset(0x2AAA8) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCreateIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCreateIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCreateIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCreateIdle") ;
//puts("SMCChooseCreateIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    StatsCreateCharacterUIUpdate() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCreateExit
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCreateExit
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- TRUE if state machine is being destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCreateExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCreateExit") ;
//puts("SMCChooseCreateExit") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    StatsCreateCharacterUIEnd() ;
    ControlFinish() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseRequestCreateEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseRequestCreateEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseRequestCreateEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_byte8 password[14];
    T_SMCChooseData *p_data ;
    E_Boolean doAbort = FALSE ;

    DebugRoutine("SMCChooseRequestCreateEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_STATUS_OK,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_STATUS_ERROR,
        FALSE) ;

    /* No password on created characters */
    StatsSetPassword(StatsGetActive(), "") ;

    if (doAbort)  {
        SMCChooseSetFlag(SMCCHOOSE_FLAG_CREATE_ABORT, TRUE) ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseRequestCreateIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseRequestCreateIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseRequestCreateIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseRequestCreateIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    StatsSaveCharacter(StatsGetActive());
    SMCChooseSetFlag(SMCCHOOSE_FLAG_CREATE_STATUS_OK, TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseLoadEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseLoadEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseLoadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseLoadEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_LOAD_STATUS_OK,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_LOAD_STATUS_INCORRECT,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_DOWNLOAD_COMPLETE,
        FALSE) ;

    /* Reset the time of day to 6:00 am. */
    MapSetDayOffset(0) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseLoadIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseLoadIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseLoadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseLoadIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(SMCCHOOSE_FLAG_LOAD_STATUS_OK, TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDownloadCharacterEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDownloadCharacterEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDownloadCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDownloadCharacterEnter") ;
//puts("DownloadEnter") ;
    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDisplayStatsEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDisplayStatsEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDisplayStatsEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDisplayStatsEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_BEGIN,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_PASSWORD_ENTERED,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHANGE_PASSWORD,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_EXIT,
        FALSE) ;

    /* Record the screen */
    GrActualScreenPush() ;

    /* Load the character stats for the active character. */
    StatsLoadCharacter(StatsGetActive()) ;
    ControlInitForJustUI() ;
    StatsLoadCharacterUIStart() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDisplayStatsIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDisplayStatsIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDisplayStatsIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMCChooseDisplayStatsIdle") ;

    StatsLoadCharacterUIUpdate() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDisplayStatsExit
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDisplayStatsExit
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- TRUE if state machine is being destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDisplayStatsExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_SMCChooseData *p_data ;
    T_TxtboxID passwordID ;
    T_byte8 *p_password ;

    DebugRoutine("SMCChooseDisplayStatsExit") ;
//puts("SMCChooseDisplayStatsExit") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    passwordID = FormGetObjID(LOAD_CHARACTER_PASSWORD_TEXT);
    p_password = TxtboxGetData (passwordID);
//printf("StatsExit: %p (%-10.10s) %p (%-10.10s)\n", p_data->attemptPassword, p_data->attemptPassword, p_password, p_password) ; fflush(stdout) ;  delay(100) ;
    strcpy(p_data->attemptPassword, p_password) ;

    StatsLoadCharacterUIEnd() ;
    ControlFinish() ;

    GrActualScreenPop() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCheckPasswordForLoadEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCheckPasswordForLoadEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCheckPasswordForLoadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;
    T_byte8 password[20] ;

    DebugRoutine("SMCChooseCheckPasswordForLoadEnter") ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_PASSWORD_OK,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_PASSWORD_NOT_OK,
        FALSE) ;

    ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_UNKNOWN) ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    StatsGetPassword(StatsGetActive(), password) ;
    if (strnicmp(password, p_data->attemptPassword, MAX_SIZE_PASSWORD) == 0)  {
        ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_OK) ;
    } else {
        ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_WRONG) ;
    }

//    PromptStatusBarInit("Checking password ...", 100) ;
    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCheckPasswordForLoadIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCheckPasswordForLoadIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCheckPasswordForLoadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    E_checkPasswordStatus passwordStatus ;

    DebugRoutine("SMCChooseCheckPasswordForLoadIdle") ;

    passwordStatus = ClientGetCheckPasswordStatus() ;

    if (passwordStatus == CHECK_PASSWORD_STATUS_OK)  {
        SMCChooseSetFlag(
            SMCCHOOSE_FLAG_PASSWORD_OK,
            TRUE) ;
//        PromptStatusBarClose() ;
        /* Fade to black */
        ColorFadeTo(0,0,0);
        /* clear screen */
        GrDrawRectangle (0,0,319,199,0);

    } else if (passwordStatus == CHECK_PASSWORD_STATUS_WRONG)  {
        SMCChooseSetFlag(
            SMCCHOOSE_FLAG_PASSWORD_NOT_OK,
            TRUE) ;
//        PromptStatusBarClose() ;
        PromptDisplayMessage("Incorrect password.  Try again.") ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseChangePasswordEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseChangePasswordEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseChangePasswordEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;
    T_byte8 password[40] ;
    T_byte8 newPassword[40] ;
    T_byte8 oldPassword[40] ;

    DebugRoutine("SMCChooseChangePasswordEnter") ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CHANGE_PASSWORD_ABORT,
        FALSE) ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* Clear out the password entries first. */
    password[0] = '\0' ;
    newPassword[0] = '\0' ;

    /* Get the old password. */
    StatsGetPassword(StatsGetActive(), oldPassword) ;

    /* Ask to enter a password and then the new password. */
    /* If at any time, the user selects cancel, abort. */
    /* If both are entered, send up the packet. */
    if ((strlen(oldPassword) == 0) || (PromptForString(
            "Enter password",
            MAX_SIZE_PASSWORD,
            password) == TRUE))  {
        if (strnicmp(oldPassword, password, 12) != 0)  {
            /* passwords don't match up.  Bad password status */
            ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_WRONG) ;
        } else {
            /* ok, correct password.  Enter a new one. */
            if (PromptForString(
                    "Enter new password",
                    MAX_SIZE_PASSWORD,
                    newPassword) == TRUE)  {
                StatsSetPassword(StatsGetActive(), newPassword) ;
                StatsSaveCharacter(StatsGetActive()) ;
                ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_OK) ;
            } else {
                ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_ABORT) ;
            }
        }
    } else {
        ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_ABORT) ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseChangePasswordIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseChangePasswordIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseChangePasswordIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    E_changePasswordStatus passwordStatus ;

    DebugRoutine("SMCChooseChangePasswordIdle") ;

    passwordStatus = ClientGetChangePasswordStatus() ;
    if (passwordStatus == CHANGE_PASSWORD_STATUS_OK)  {
        PromptDisplayMessage("Password changed.") ;

        SMCChooseSetFlag(
            SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,
            TRUE) ;
    } else if (passwordStatus == CHANGE_PASSWORD_STATUS_WRONG)  {
        PromptDisplayMessage("Incorrect password.") ;

        SMCChooseSetFlag(
            SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,
            TRUE) ;
    } else if (passwordStatus == CHANGE_PASSWORD_STATUS_ABORT)  {
        SMCChooseSetFlag(SMCCHOOSE_FLAG_CHANGE_PASSWORD_ABORT, TRUE) ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseEnableBeginEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseEnableBeginEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseEnableBeginEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseEnableBeginEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_ENABLE_COMPLETE,
        FALSE) ;

//    buttonID = FormGetObjID(LOAD_CHARACTER_BEGIN_BUTTON);
//    ButtonEnable(buttonID);

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_ENABLE_COMPLETE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDeleteEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDeleteEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDeleteEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;
    T_statsSavedCharacterID* chardata;
	T_byte8 tempstr[64];
    T_byte8 password[14];
    T_byte8 oldPassword[64] ;

    DebugRoutine("SMCChooseDeleteEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_DELETE_PASSWORD_OK,
        FALSE) ;
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_DELETE_PASSWORD_NOT_OK,
        FALSE) ;

    StatsGetPassword(StatsGetActive(), oldPassword) ;

    /* get character id structure for selected character */
    chardata=StatsGetSavedCharacterIDStruct (StatsGetActive());
    DebugCheck(chardata->status < CHARACTER_STATUS_UNDEFINED) ;
    if (chardata->status < CHARACTER_STATUS_UNDEFINED)
    {
        /* get password */
        sprintf(tempstr, "^001Enter password to confirm ^021delete") ;
        strcpy (password,"");

        if ((strlen(oldPassword)==0) ||
            (PromptForString(tempstr,12,password)==TRUE))  {
            /* check to see if password is correct */
            if (stricmp (password, oldPassword)==0)  {
                /* success. character will be deleted */
                /*         delete code here           */
                SMCChooseSetFlag(
                    SMCCHOOSE_FLAG_DELETE_PASSWORD_OK,
                    TRUE) ;
            } else {
                strcpy (tempstr,"Improper password. Delete command canceled.");
                PromptDisplayMessage(tempstr);
                /* failure. return to mainui after notification of failure */

                SMCChooseSetFlag(
                    SMCCHOOSE_FLAG_DELETE_PASSWORD_NOT_OK,
                    TRUE) ;
            }
        } else {
            /* Go back, user aborted. */
            SMCChooseSetFlag(
                SMCCHOOSE_FLAG_DELETE_PASSWORD_NOT_OK,
                TRUE) ;
        }
    }
    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDeleteExit
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDeleteExit
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- TRUE if state machine is being destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDeleteExit(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDeleteExit") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDeleteCharacterEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDeleteCharacterEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDeleteCharacterEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDeleteCharacterEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_DELETE_COMPLETE,
        FALSE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseDeleteCharacterIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseDeleteCharacterIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseDeleteCharacterIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseDeleteCharacterIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    /* OK, we can delete the character. */
    if (PromptForBoolean("Are you sure you want to delete?", FALSE) == TRUE)  {
        StatsDeleteCharacter(StatsGetActive());

        /* inform user of deletion */
        PromptDisplayMessage ("Character deleted.");
    } else {
        PromptDisplayMessage ("Character NOT deleted.");
    }
    SMCChooseSetFlag(SMCCHOOSE_FLAG_DELETE_COMPLETE, TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMCChoosePlayGameEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChoosePlayGameEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChoosePlayGameEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChoosePlayGameEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMMainSetFlag(SMMAIN_FLAG_BEGIN_GAME, TRUE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseExitEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseExitEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseExitEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseExitEnter") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

//puts("ExitEnter") ; fflush(stdout) ;
    SMMainSetFlag(SMMAIN_FLAG_LEAVE_SERVER, TRUE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCreateUploadEnter
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCreateUploadEnter
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCreateUploadEnter(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCreateUploadEnter") ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_UPLOAD_OK,
        FALSE) ;

    /* Send the character. */
    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_UPLOAD_OK,
        TRUE) ;

    DebugEnd() ;
}

/* Note that a create character upload is complete. */
/* LES 06/24/96 Created */
static T_void ICreateUploadComplete(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData)
{
    DebugRoutine("ICreateUploadComplete") ;

    /* Let go of the uploaded character. */
    MemFree(p_data) ;

    /* Note that the upload is complete. */
    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_CREATE_UPLOAD_OK,
        TRUE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMCChooseCreateUploadIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMCChooseCreateUploadIdle
 *
 *  @param handle -- Handle to state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMCChooseCreateUploadIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_SMCChooseData *p_data ;

    DebugRoutine("SMCChooseCreateUploadIdle") ;

    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SMCCHOOS.C
 *-------------------------------------------------------------------------*/
