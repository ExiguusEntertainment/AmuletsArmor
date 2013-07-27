/****************************************************************************/
/*    FILE:  SMCONNEC.H                                                     */
/****************************************************************************/
#ifndef _SMCONNEC_H_
#define _SMCONNEC_H_

#include "CONNUI.H"
#include "SMACHINE.H"

#define CLIENT_CONNECT_STATE_SELECT_SERVER         0
#define CLIENT_CONNECT_STATE_CONNECT_TO_SERVER     1
#define CLIENT_CONNECT_STATE_BUSY_MESSAGE          2
#define CLIENT_CONNECT_STATE_NO_CONNECT_MESSAGE    3
#define CLIENT_CONNECT_STATE_SEND_LOGIN_AND_WAIT   4
#define CLIENT_CONNECT_STATE_SERVER_NOT_ACK_LOGIN  5
#define CLIENT_CONNECT_STATE_GET_SERVER_ID         6
#define CLIENT_CONNECT_STATE_INCORRECT_SERVER_ID   7
#define CLIENT_CONNECT_STATE_GOTO_START            8
#define CLIENT_CONNECT_STATE_DISPLAY_EXIT_MESSAGE  9
#define CLIENT_CONNECT_STATE_ABORT_CONNECTION      10
#define CLIENT_CONNECT_STATE_NO_SERVER_ID          11
#define CLIENT_CONNECT_STATE_READY                 12
#define CLIENT_CONNECT_STATE_LOGOUT_AND_HANGUP     13
#define CLIENT_CONNECT_STATE_EXIT                  14
#define CLIENT_CONNECT_STATE_UNKNOWN               15

#define NUMBER_CLIENT_CONNECT_STATES     CLIENT_CONNECT_STATE_UNKNOWN
#define CLIENT_CONNECT_INITIAL_STATE     CLIENT_CONNECT_STATE_SELECT_SERVER

/* Conditional routines: */
#define CLIENT_CONNECT_FLAG_SERVER_SELECTED               0
#define CLIENT_CONNECT_FLAG_CONNECTED                     1
#define CLIENT_CONNECT_FLAG_CONNECTION_ABORTED            2
#define CLIENT_CONNECT_FLAG_LINE_BUSY                     3
#define CLIENT_CONNECT_FLAG_TIMEOUT                       4
#define CLIENT_CONNECT_FLAG_BUSY_MESSAGE_DONE             5
#define CLIENT_CONNECT_FLAG_NO_CONNECT_MESSAGE_DONE       6
#define CLIENT_CONNECT_FLAG_TIMEOUT_LOGIN                 7
#define CLIENT_CONNECT_FLAG_LOGIN_ACCEPTED                8
#define CLIENT_CONNECT_FLAG_NOT_ACK_LOGIN_DONE            9
#define CLIENT_CONNECT_FLAG_GET_SERVER_ID_TIMEOUT         10
#define CLIENT_CONNECT_FLAG_CORRECT_SERVER_ID             11
#define CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID           12
#define CLIENT_CONNECT_FLAG_ALLOWED_TO_ENTER              13
#define CLIENT_CONNECT_FLAG_NOT_ALLOWED_TO_ENTER          14
#define CLIENT_CONNECT_FLAG_EXIT_MESSAGE_DONE             15
#define CLIENT_CONNECT_FLAG_ABORT_COMPLETE                16
#define CLIENT_CONNECT_FLAG_LOGOUT_AND_HANGUP_DONE        17
#define CLIENT_CONNECT_FLAG_NO_SERVER_ID_DONE             18
#define CLIENT_CONNECT_FLAG_EXIT                          19
#define CLIENT_CONNECT_FLAG_INCORRECT_SERVER_ID_DONE      20
#define CLIENT_CONNECT_FLAG_UNKNOWN                       21

#define NUMBER_CLIENT_CONNECT_FLAGS     CLIENT_CONNECT_FLAG_UNKNOWN

E_Boolean SMClientConnectCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMClientConnectSetFlag(T_word32 flag, E_Boolean state) ;

T_stateMachineHandle SMClientConnectInit(T_void) ;

T_void SMClientConnectFinish(T_void) ;

T_void SMClientConnectUpdate(T_void) ;

E_Boolean SMClientConnectIsDone(T_void) ;

T_void SMClientConnectSetServerInfo(T_connUIStruct *p_server) ;

T_connUIStruct *SMClientConnectGetServerInfo(T_void) ;

#endif // _SMCONNEC_H_

/****************************************************************************/
/*    END OF FILE:  SMCONNEC.H                                              */
/****************************************************************************/
