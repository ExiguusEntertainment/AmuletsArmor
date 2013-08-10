/*-------------------------------------------------------------------------*
 * File:  SMCONNEC.C
 *-------------------------------------------------------------------------*/
/**
 * Connect to server state machine.  Deprecated?
 *
 * @addtogroup SMCONNEC
 * @brief State Machine for connecting to server
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONTROL.H"
#include "FILE.H"
#include "MEMORY.H"
#include "PICS.H"
#include "PROMPT.H"
#include "SMCONNEC.H"
#include "SMMAIN.H"
#include "SOUND.H"
#include "TICKER.H"

typedef struct {
    E_Boolean stateFlags[NUMBER_CLIENT_CONNECT_FLAGS] ;
    T_connUIStruct serverInfo ;
    T_word32 loginTimeout ;
    T_word32 lookingForServerID ;
    T_word32 getServerIDTimeout ;
} T_smClientData ;

/* Prototypes for init and finish routines: */
T_void SMClientConnectInitData(T_stateMachineHandle handle) ;
T_void SMClientConnectFinishData(T_stateMachineHandle handle) ;

/* Prototypes for State entry & exit routines: */
T_void SMClientConnectSelectServer(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
T_void SMClientConnectServerSelected(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;
T_void SMClientConnectSelectServerIdle(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectConnectToServer(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
T_void SMClientConnectConnectionResolved(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed) ;

T_void SMClientConnectBusyMessage(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectNoConnectMessage(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectSendLogin(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
T_void SMClientConnectWaitForLogin(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectServerNotAckLogin(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectGetServerID(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
T_void SMClientConnectLookForServerID(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectIncorrectServerID(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectGotoStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectDisplayExitMessage(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectAbortConnection(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectNoServerID(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectReady(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectLogoutAndHangup(
           T_stateMachineHandle handle,
           T_word32 extraData) ;

T_void SMClientConnectWaitForStart(
           T_stateMachineHandle handle,
           T_word32 extraData) ;
extern void HangUp(void);

static T_stateMachineHandle G_smClientHandle ;
static E_Boolean G_init = FALSE ;

/* Internal prototypes: */
static T_smClientData *ISMClientConnectGetExtraData(T_void) ;

/***************************************************************************/
/*                             CONDITIONALS                                */
/***************************************************************************/
static T_stateMachineConditional SMClientConnectSelectServerCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_SERVER_SELECTED,       /* extra data */
        CLIENT_CONNECT_STATE_CONNECT_TO_SERVER     /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_EXIT,                  /* extra data */
        CLIENT_CONNECT_STATE_EXIT                  /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectConnectToServerCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_CONNECTED,             /* extra data */
        CLIENT_CONNECT_STATE_SEND_LOGIN_AND_WAIT   /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_CONNECTION_ABORTED,    /* extra data */
        CLIENT_CONNECT_STATE_ABORT_CONNECTION      /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_LINE_BUSY,             /* extra data */
        CLIENT_CONNECT_STATE_BUSY_MESSAGE          /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_TIMEOUT,               /* extra data */
        CLIENT_CONNECT_STATE_NO_CONNECT_MESSAGE    /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectBusyMessageCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_BUSY_MESSAGE_DONE,     /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectNoConnectMessageCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_NO_CONNECT_MESSAGE_DONE,  /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectSendLoginAndWaitCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_LOGIN_ACCEPTED,        /* extra data */
        CLIENT_CONNECT_STATE_GET_SERVER_ID         /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_TIMEOUT_LOGIN,         /* extra data */
        CLIENT_CONNECT_STATE_SERVER_NOT_ACK_LOGIN  /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectServerNotAckLoginCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_NOT_ACK_LOGIN_DONE,    /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectGetServerIDCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_GET_SERVER_ID_TIMEOUT, /* extra data */
        CLIENT_CONNECT_STATE_NO_SERVER_ID          /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_CORRECT_SERVER_ID,     /* extra data */
        CLIENT_CONNECT_STATE_GOTO_START            /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID,   /* extra data */
        CLIENT_CONNECT_STATE_INCORRECT_SERVER_ID   /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectIncorrectServerIDCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID_DONE, /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectGotoStartCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_ALLOWED_TO_ENTER,      /* extra data */
        CLIENT_CONNECT_STATE_READY                 /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_NOT_ALLOWED_TO_ENTER,  /* extra data */
        CLIENT_CONNECT_STATE_LOGOUT_AND_HANGUP     /* next state */
    },
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_TIMEOUT_LOGIN,         /* extra data */
        CLIENT_CONNECT_STATE_SERVER_NOT_ACK_LOGIN  /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectDisplayExitMessageCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_EXIT_MESSAGE_DONE,     /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectAbortConnectionCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_ABORT_COMPLETE,        /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectNoServerIDCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_NO_SERVER_ID_DONE,     /* extra data */
        CLIENT_CONNECT_STATE_SELECT_SERVER         /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/
static T_stateMachineConditional SMClientConnectLogoutAndHangupCond[] = {
    {
        SMClientConnectCheckFlag,                  /* conditional callback */
        CLIENT_CONNECT_FLAG_LOGOUT_AND_HANGUP_DONE,/* extra data */
        CLIENT_CONNECT_STATE_DISPLAY_EXIT_MESSAGE  /* next state */
    }
} ;
/*-------------------------------------------------------------------------*/

/***************************************************************************/
/*                             STATES                                      */
/***************************************************************************/

static T_stateMachineState SMClientConnectStates[] = {
    /* CLIENT_CONNECT_STATE_SELECT_SERVER */
    {
        SMClientConnectSelectServer,             /* Enter callback */
        SMClientConnectServerSelected,           /* Exit callback */
        SMClientConnectSelectServerIdle,         /* Idle callback */
        0,                                       /* Extra data */
        2,                                       /* Num conditionals */
        SMClientConnectSelectServerCond          /* conditional list. */
    },

    /* CLIENT_CONNECT_STATE_CONNECT_TO_SERVER */
    {
        SMClientConnectConnectToServer,          /* Enter callback */
        SMClientConnectConnectionResolved,       /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        4,                                       /* Num conditionals */
        SMClientConnectConnectToServerCond       /* conditional list. */
    },

    /* CLIENT_CONNECT_STATE_BUSY_MESSAGE */
    {
        SMClientConnectBusyMessage,              /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectBusyMessageCond           /* conditional list. */
    },

    /* CLIENT_CONNECT_STATE_NO_CONNECT_MESSAGE */
    {
        SMClientConnectNoConnectMessage,         /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectNoConnectMessageCond      /* conditional list. */
    },

    /* CLIENT_CONNECT_STATE_SEND_LOGIN_AND_WAIT */
    {
        SMClientConnectSendLogin,                /* Enter callback */
        NULL,                                    /* Exit callback */
        SMClientConnectWaitForLogin,             /* Idle callback */
        0,                                       /* Extra data */
        2,                                       /* Num conditionals */
        SMClientConnectSendLoginAndWaitCond      /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_SERVER_NOT_ACK_LOGIN */
        SMClientConnectServerNotAckLogin,        /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectServerNotAckLoginCond     /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_GET_SERVER_ID */
        SMClientConnectGetServerID,              /* Enter callback */
        NULL,                                    /* Exit callback */
        SMClientConnectLookForServerID,          /* Idle callback */
        0,                                       /* Extra data */
        3,                                       /* Num conditionals */
        SMClientConnectGetServerIDCond           /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_INCORRECT_SERVER_ID */
        SMClientConnectIncorrectServerID,        /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectIncorrectServerIDCond     /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_GOTO_START */
        SMClientConnectGotoStart,                /* Enter callback */
        NULL,                                    /* Exit callback */
        SMClientConnectWaitForStart,             /* Idle callback */
        0,                                       /* Extra data */
        3,                                       /* Num conditionals */
        SMClientConnectGotoStartCond             /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_DISPLAY_EXIT_MESSAGE */
        SMClientConnectDisplayExitMessage,       /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectDisplayExitMessageCond    /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_ABORT_CONNECTION */
        SMClientConnectAbortConnection,          /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectAbortConnectionCond       /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_NO_SERVER_ID */
        SMClientConnectNoServerID,               /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectNoServerIDCond            /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_READY */
        SMClientConnectReady,                    /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        0,                                       /* Num conditionals */
        NULL                                     /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_LOGOUT_AND_HANGUP */
        SMClientConnectLogoutAndHangup,          /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        1,                                       /* Num conditionals */
        SMClientConnectLogoutAndHangupCond       /* conditional list. */
    },
    {  /* CLIENT_CONNECT_STATE_EXIT */
        NULL,                                    /* Enter callback */
        NULL,                                    /* Exit callback */
        NULL,                                    /* Idle callback */
        0,                                       /* Extra data */
        0,                                       /* Num conditionals */
        NULL                                     /* conditional list. */
    }
} ;

/***************************************************************************/
/*                             STATE MACHINE                               */
/***************************************************************************/

static T_stateMachine SMClientConnectStateMachine = {
    STATE_MACHINE_TAG,              /* Tag to identify the structure. */
    SMClientConnectInitData,        /* Init. callback */
    SMClientConnectFinishData,      /* Finish callback */
    NUMBER_CLIENT_CONNECT_STATES,   /* Number states */
    SMClientConnectStates           /* State list */
} ;

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectSelectServer
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectSelectServer is called at the beginning of the
 *  SELECT_SERVER state.  This routine brings up the UI for choosing
 *  a server.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectSelectServer(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;
    T_palette *p_palette ;
    T_resource res ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    SoundSetBackgroundVolume (255);
    SoundSetBackgroundMusic("LOGON.HMI") ;
    p_palette = (T_palette *)PictureLockData("UI/CONNECT/CONNECTPAL", &res) ;

    GrSetPalette(0, 256, (T_palette *)p_palette);
    ColorInit();
    PictureUnlockAndUnfind(res) ;

    ConnMainUIStart() ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_SERVER_SELECTED,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_EXIT,
        FALSE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectSelectServerIdle
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectSelectServerIdle is called while in the state of
 *  SELECT_SERVER.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectSelectServerIdle(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServerIdle") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    ConnMainUIUpdate() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectServerSelected
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectServerSelected is called after the conn ui screen
 *  has been shown and is now done.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- End flag
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectServerSelected(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectServerSelected") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    ConnMainUIEnd() ;

/* !!! This needs to be changed to actually get the selected server id. */
    p_data->lookingForServerID = 1 ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnect
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnect
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectConnectToServer(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;
    E_clientDialResult dialResult ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Initialize the flags from the possible outcomes. */
    SMClientConnectSetFlag(
        CLIENT_CONNECT_STATE_CONNECT_TO_SERVER,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_LINE_BUSY,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_CONNECTION_ABORTED,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_TIMEOUT,
        FALSE) ;

    /* Set up other variables we will need later. */
    ClientSetCurrentPlace(PLACE_NOWHERE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_ALLOWED_TO_ENTER,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NOT_ALLOWED_TO_ENTER,
        FALSE) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    /* Start up a connection and see what happens. */
    dialResult = ClientDialIn(
                     "ATZ",
//                     p_data->serverInfo.servinit,
                     p_data->serverInfo.servphone);

    /* based on one of the results, tell this to the state machine. */
    switch(dialResult)  {
        case CLIENT_DIAL_RESULT_CONNECTED:
            SMClientConnectSetFlag(
                CLIENT_CONNECT_STATE_CONNECT_TO_SERVER,
                TRUE) ;
            break ;
        case CLIENT_DIAL_RESULT_BUSY:
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_LINE_BUSY,
                TRUE) ;
            break ;
        case CLIENT_DIAL_RESULT_ABORTED:
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_CONNECTION_ABORTED,
                TRUE) ;
            break ;
        case CLIENT_DIAL_RESULT_TIMEOUT:
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_TIMEOUT,
                TRUE) ;
            break ;
        default:
            DebugCheck(FALSE) ;
            break ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnect
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnect
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *  @param isDestroyed -- Flag if state machine destroyed
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectConnectionResolved(
           T_stateMachineHandle handle,
           T_word32 extraData,
           E_Boolean isDestroyed)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_STATE_CONNECT_TO_SERVER,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_LINE_BUSY,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_CONNECTION_ABORTED,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_TIMEOUT,
        FALSE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectBusyMessage
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectBusyMessage states that the connection failed because
 *  the line was busy.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectBusyMessage(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMClientConnectSelectServer") ;

    HangUp() ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_BUSY_MESSAGE_DONE,
        FALSE) ;

    PromptDisplayMessage("Unable to connect.  Line is busy.") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_BUSY_MESSAGE_DONE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectNoConnectMessage
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectNoConnectMessage states that a time out occured
 *  and a connection could not be established.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectNoConnectMessage(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMClientConnectSelectServer") ;

    HangUp() ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NO_CONNECT_MESSAGE_DONE,
        FALSE) ;

    PromptDisplayMessage("Timeout.  No connection established.") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NO_CONNECT_MESSAGE_DONE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectSendLogin
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectSendLogin starts up the actual login process with the
 *  server.  A login packet is sent and the state will wait until it times
 *  out or receives login acknowledgement.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectSendLogin(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

//    PromptDisplayMessage("Connection!") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_LOGIN_ACCEPTED,
        FALSE) ;
    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_TIMEOUT_LOGIN,
        FALSE) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    PromptStatusBarInit("Requesting login ...", 30*TICKS_PER_SECOND) ;
    ClientLogin() ;
    p_data->loginTimeout = TickerGet() + 2100 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectWaitForLogin
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectWaitForLogin is the idle routine for the state of
 *  SEND_LOGIN_AND_WAIT.  This routine checks to see if the client is
 *  logged in or times out.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectWaitForLogin(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;
    T_sword32 timeLeft ;

    DebugRoutine("SMClientConnectWaitForLogin") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    timeLeft = p_data->loginTimeout - TickerGet() ;
    if (timeLeft < 0)
        timeLeft = 0 ;

    PromptStatusBarUpdate(2100 - timeLeft) ;

    if (ClientIsLogin())  {
        SMClientConnectSetFlag(
            CLIENT_CONNECT_FLAG_LOGIN_ACCEPTED,
            TRUE) ;
        PromptStatusBarClose() ;
    } else if (timeLeft <= 0)  {
        SMClientConnectSetFlag(
            CLIENT_CONNECT_FLAG_TIMEOUT_LOGIN,
            TRUE) ;
        PromptStatusBarClose() ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectServerNotAckLogin
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnect
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectServerNotAckLogin(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectServerNotAckLogin") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    HangUp() ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NOT_ACK_LOGIN_DONE,
        FALSE) ;

    PromptDisplayMessage("Timeout. Server did not respond to login.") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NOT_ACK_LOGIN_DONE,
        TRUE) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectGetServerID
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectGetServerID starts a request to the server for its
 *  unique server id.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectGetServerID(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

//    PromptDisplayMessage("Login accepted.") ;

//puts("Login accepted.") ;
    ClientSetCurrentServerID(SERVER_ID_NONE) ;
    ClientSendRequestServerIDPacket() ;

    /* Determine when we will take long. */
    p_data->getServerIDTimeout = TickerGet() + TICKS_PER_SECOND * 10 ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectLookForServerID
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectLookForServerID sits and constantly checks to see
 *  if the client has gotten a server id.  When it gets it, it compares
 *  to see if this is what was supposed to be gotten.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectLookForServerID(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;
    T_word32 serverID ;

    DebugRoutine("SMClientConnectLookForServerID") ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    serverID = ClientGetCurrentServerID() ;
//printf("Got serverID: %d\n", serverID) ;
    if (serverID != SERVER_ID_NONE)  {
        if (serverID == p_data->lookingForServerID)  {
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_CORRECT_SERVER_ID,
                TRUE) ;
        } else  {
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID,
                TRUE) ;
        }
    } else {
        if (TickerGet() > p_data->getServerIDTimeout)  {
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_GET_SERVER_ID_TIMEOUT,
                TRUE) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectIncorrectServerID
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectIncorrectServerID is called when the server responds
 *  with a different server ID than what is expected.  The client then
 *  logs off with an error message.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectIncorrectServerID(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID_DONE,
        FALSE) ;

    HangUp() ;
    PromptDisplayMessage("Error! Server ID Mismatch!") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID_DONE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectGotoStart
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectGotoStart requests to enter the server.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectGotoStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    /* Start the timeout timer for about 10 seconds. */
    p_data->loginTimeout = TickerGet() + 1400 ;
//    PromptStatusBarInit("Asking for directions ...", 1400) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectDisplayExitMessage
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectDisplayExitMessage displays the exit message that
 *  the server sent down when it exited.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectDisplayExitMessage(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_byte8 *p_msg ;
    T_word32 size ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_EXIT_MESSAGE_DONE,
        FALSE) ;

    p_msg = FileLoad("exit.msg", &size) ;

    PromptDisplayBulletin(p_msg) ;

    MemFree(p_msg) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_EXIT_MESSAGE_DONE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectAbortConnection
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectAbortConnection
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectAbortConnection(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMClientConnectAbortConnection") ;

    HangUp() ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_ABORT_COMPLETE,
        FALSE) ;

    PromptDisplayMessage("Aborted connection.") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_ABORT_COMPLETE,
        TRUE) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectNoServerID
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectNoServerID is called when the request for the server
 *  ID times out and doesn't get a response.  This routine politely hangs
 *  up the phone and returns to the main screen.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectNoServerID(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NO_SERVER_ID_DONE,
        FALSE) ;

    HangUp() ;
    PromptDisplayMessage("Timeout.  Server not responding with server ID!  Hung up the phone.  You should probably call the sysop of this system.") ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_NO_SERVER_ID_DONE,
        TRUE) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnect
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnect
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectReady(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSelectServer") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnect
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnect
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectLogoutAndHangup(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectLogoutAndHangup") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectCheckFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectCheckFlag checks to see if one of the state flags
 *  of the client connect state machine is set.
 *
 *  @param handle -- State machine
 *  @param flag -- Flag to get value of
 *
 *  @return State of flag
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SMClientConnectCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag)
{
    E_Boolean stateFlag = FALSE ;        /* Return status will default */
                                         /* to FALSE. */
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectCheckFlag") ;
    DebugCheck(G_smClientHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(G_smClientHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid flag, get the state */
    DebugCheck(flag < CLIENT_CONNECT_FLAG_UNKNOWN) ;
    if (flag < CLIENT_CONNECT_FLAG_UNKNOWN)
        stateFlag = p_data->stateFlags[flag] ;

    DebugEnd() ;

    return stateFlag ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectSetFlag
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectSetFlag sets a flag for the client connect state
 *  machine to process as part of its states.
 *
 *  @param flag -- Flag to set
 *  @param state -- State to set flag to
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectSetFlag(
              T_word32 flag,
              E_Boolean state)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSetFlag") ;
    DebugCheck(G_smClientHandle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(G_smClientHandle) ;
    DebugCheck(p_data != NULL) ;

    /* If a valid index, set to the new state */
    DebugCheck(flag < CLIENT_CONNECT_FLAG_UNKNOWN) ;
    if (flag < CLIENT_CONNECT_FLAG_UNKNOWN)
        p_data->stateFlags[flag] = state ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectInit
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectInit sets up the state machine and its data.
 *
 *<!-----------------------------------------------------------------------*/
T_stateMachineHandle SMClientConnectInit(T_void)
{
    DebugRoutine("SMClientConnectInit") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;
    G_smClientHandle = StateMachineCreate(&SMClientConnectStateMachine) ;
    StateMachineGotoState(G_smClientHandle, CLIENT_CONNECT_INITIAL_STATE) ;
    ControlInitForJustUI() ;

    DebugEnd() ;

    return G_smClientHandle ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectFinish
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectFinish cleans up the client connect state machine's
 *  data.
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectFinish(T_void)
{
    DebugRoutine("SMClientConnectFinish") ;
    DebugCheck(G_init == TRUE) ;

    /* Destroy the state machine. */
    StateMachineDestroy(G_smClientHandle) ;
    G_smClientHandle = STATE_MACHINE_HANDLE_BAD ;
    ControlFinish() ;

    G_init = FALSE ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectUpdate
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectUpdate updates the client connect state machine.
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectUpdate(T_void)
{
    DebugRoutine("SMClientConnectUpdate") ;

    StateMachineUpdate(G_smClientHandle) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectIsDone
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectIsDone checks if the client connect state machine
 *  has reached its end and is done.
 *
 *  @return Done if TRUE
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SMClientConnectIsDone(T_void)
{
    T_word16 state;
    E_Boolean isDone = FALSE ;

    DebugRoutine("SMClientConnectIsDone") ;

    state = StateMachineGetState(G_smClientHandle) ;

    if (state == CLIENT_CONNECT_STATE_READY)  {
//puts("State for connect is correct to go on!") ;
        SMMainSetFlag(
            SMMAIN_FLAG_CONNECT_COMPLETE,
            TRUE) ;
    } else if (state == CLIENT_CONNECT_STATE_EXIT)  {
//puts("State for conenct is to exit.") ;
        isDone = TRUE ;
    }

    DebugEnd() ;

    return isDone ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectInitData
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectInitData creates the data necessary to be associated
 *  to the client connect state machine.
 *
 *  @param handle -- state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectInitData(T_stateMachineHandle handle)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectInitData") ;

    p_data = MemAlloc(sizeof(T_smClientData)) ;
    DebugCheck(p_data != NULL) ;
    memset(p_data, 0, sizeof(T_smClientData)) ;

    StateMachineSetExtraData(handle, p_data) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectFinishData
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectFinishData destroys the data attached to the client
 *  connect state machine.
 *
 *  @param handle -- state machine
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectFinishData(T_stateMachineHandle handle)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectFinishData") ;

    /* Destroy the extra data attached to the state machine. */
    p_data = (T_smClientData *)StateMachineGetExtraData(G_smClientHandle) ;
    DebugCheck(p_data != NULL) ;
    MemFree(p_data) ;
    StateMachineSetExtraData(handle, NULL) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectSetServerInfo
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectSetServerInfo declares which server the connection
 *  will occur with.
 *
 *  @param p_server -- Server info to use
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectSetServerInfo(T_connUIStruct *p_server)
{
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectSetServerInfo") ;
    DebugCheck(G_init == TRUE) ;

    p_data = ISMClientConnectGetExtraData() ;

    if (p_data)
        p_data->serverInfo = *p_server ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectGetServerInfo
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectGetServerInfo returns a pointer to the connection ui
 *  server structure.
 *
 *  @return POinter to structure
 *
 *<!-----------------------------------------------------------------------*/
T_connUIStruct *SMClientConnectGetServerInfo(T_void)
{
    T_connUIStruct *p_server = NULL ;
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectGetServerInfo") ;

    p_data = ISMClientConnectGetExtraData() ;
    if (p_data)
        p_server = &(p_data->serverInfo) ;

    DebugEnd() ;

    return p_server ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ISMClientConnectGetExtraData
 *-------------------------------------------------------------------------*/
/**
 *  ISMClientConnectGetExtraData returns a pointer to the current SM
 *  Client's global data area.
 *
 *  @return SM Client's data
 *
 *<!-----------------------------------------------------------------------*/
static T_smClientData *ISMClientConnectGetExtraData(T_void)
{
    T_smClientData *p_data = NULL ;

    DebugRoutine("ISMClientConnectGetExtraData") ;
    DebugCheck(G_init == TRUE) ;

    if (G_init)  {
        p_data = (T_smClientData *)StateMachineGetExtraData(G_smClientHandle) ;
        DebugCheck(p_data != NULL) ;
    }

    DebugEnd() ;

    return p_data ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SMClientConnectWaitForStart
 *-------------------------------------------------------------------------*/
/**
 *  SMClientConnectWaitForState waits until it is decided where the
 *  player will start the game.
 *
 *  @param handle -- Handle to PHASE A client state machine
 *  @param extraData -- Not used
 *
 *<!-----------------------------------------------------------------------*/
T_void SMClientConnectWaitForStart(
           T_stateMachineHandle handle,
           T_word32 extraData)
{
    T_sword32 timeLeft ;
    T_smClientData *p_data ;

    DebugRoutine("SMClientConnectWaitForStart") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    p_data = (T_smClientData *)StateMachineGetExtraData(handle) ;
    DebugCheck(p_data != NULL) ;

    SMClientConnectSetFlag(
        CLIENT_CONNECT_FLAG_ALLOWED_TO_ENTER,
        TRUE) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SMCONNEC.C
 *-------------------------------------------------------------------------*/
