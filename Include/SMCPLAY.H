/****************************************************************************/
/*    FILE:  SM.H                                                           */
/****************************************************************************/
#ifndef _SMCPLAY_H_
#define _SMCPLAY_H_

#include "SMACHINE.H"

/* State constants: */
#define SMCPLAY_GAME_WAIT_FOR_GO_TO_STATE                  0
#define SMCPLAY_GAME_GO_TO_PLACE_STATE                     1
#define SMCPLAY_GAME_DO_GAME_STATE                         2
#define SMCPLAY_GAME_TIMED_OUT_STATE                       3
#define SMCPLAY_GAME_END_GAME_STATE                        4
#define SMCPLAY_GAME_UNKNOWN_STATE                         5

#define NUMBER_SMCPLAY_GAME_STATES    SMCPLAY_GAME_UNKNOWN_STATE
#define SMCPLAY_GAME_INITIAL_STATE    SMCPLAY_GAME_WAIT_FOR_GO_TO_STATE

/* Conditional flags: */
#define SMCPLAY_GAME_FLAG_GOTO_RECEIVED                       0
#define SMCPLAY_GAME_FLAG_TIMED_OUT                           1
#define SMCPLAY_GAME_FLAG_END_GAME                            2
#define SMCPLAY_GAME_FLAG_START_GAME                          3
#define SMCPLAY_GAME_FLAG_LEAVE_PLACE                         4
#define SMCPLAY_GAME_FLAG_UNKNOWN                             5
#define NUMBER_SMCPLAY_GAME_FLAGS    SMCPLAY_GAME_FLAG_UNKNOWN

T_stateMachineHandle SMCPlayGameInitialize(T_void) ;

T_void SMCPlayGameFinish(T_void) ;

T_void SMCPlayGameUpdate(T_void) ;

E_Boolean SMCPlayGameCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMCPlayGameSetFlag(
           T_word16 flag,
           E_Boolean state) ;

#endif // _SMCPLAY_H_

/****************************************************************************/
/*    END OF FILE:  SM.H                                                    */
/****************************************************************************/
