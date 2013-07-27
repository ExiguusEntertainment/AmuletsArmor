/****************************************************************************/
/*    FILE:  SMMAIN.H                                                       */
/****************************************************************************/
#ifndef _SMMAIN_H_
#define _SMMAIN_H_

#include "SMACHINE.H"

typedef enum {
    SMMAIN_STATE_CONNECT=0,
    SMMAIN_STATE_CHOOSE_CHARACTER,
    SMMAIN_STATE_PLAY_GAME,
    SMMAIN_STATE_LOGOFF_CHARACTER,
    SMMAIN_STATE_LEAVE_SERVER,
    SMMAIN_STATE_EXIT_GAME,
    SMMAIN_STATE_DISCONNECTED,
    SMMAIN_STATE_UNKNOWN
} T_smMainState ;

#define NUMBER_SMMAIN_STATES    SMMAIN_STATE_UNKNOWN
#define SMMAIN_INITIAL_STATE    SMMAIN_STATE_CONNECT

typedef enum {
    SMMAIN_FLAG_CONNECT_COMPLETE=0,
    SMMAIN_FLAG_CONNECT_EXIT,
    SMMAIN_FLAG_BEGIN_GAME,
    SMMAIN_FLAG_LEAVE_SERVER,
    SMMAIN_FLAG_LEAVE_SERVER_COMPLETE,
    SMMAIN_FLAG_END_GAME,
    SMMAIN_FLAG_LOGOFF_COMPLETE,
    SMMAIN_FLAG_DROPPED,
    SMMAIN_FLAG_DISCONNECT_COMPLETE,
    SMMAIN_FLAG_UNKNOWN
} T_smMainStateFlags ;

#define NUMBER_SMMAIN_FLAGS     SMMAIN_FLAG_UNKNOWN

T_void SMMainInitData(T_stateMachineHandle handle) ;

T_void SMMainFinishData(T_stateMachineHandle handle) ;

E_Boolean SMMainCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMMainSetFlag(T_word32 flag, E_Boolean state) ;

T_stateMachineHandle SMMainInit(T_void) ;

T_void SMMainFinish(T_void) ;

T_void SMMainUpdate(T_void) ;

E_Boolean SMMainIsDone(T_void) ;

#endif // _SMMAIN_H_

/****************************************************************************/
/*    END OF FILE:  SMMAIN.H                                                */
/****************************************************************************/
