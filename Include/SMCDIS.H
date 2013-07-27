/****************************************************************************/
/*    FILE:  SM.H                                                           */
/****************************************************************************/
#ifndef _SMCDIS_H_
#define _SMCDIS_H_

#include "SMACHINE.H"

/* State constants: */
#define SMCDISCONNECT_HANG_UP_STATE                        0
#define SMCDISCONNECT_ERROR_MSG_STATE                      1
#define SMCDISCONNECT_SAVE_CHARACTER_STATE                 2
#define SMCDISCONNECT_EXIT_TO_CONNECT_STATE                3
#define SMCDISCONNECT_UNKNOWN_STATE                        4

#define NUMBER_SMCDISCONNECT_STATES    SMCDISCONNECT_UNKNOWN_STATE
#define SMCDISCONNECT_INITIAL_STATE    SMCDISCONNECT_HANG_UP_STATE

/* Conditional flags: */
#define SMCDISCONNECT_FLAG_HANG_UP_COMPLETE                    0
#define SMCDISCONNECT_FLAG_ERROR_MSG_COMPLETE                  1
#define SMCDISCONNECT_FLAG_SAVE_CHARACTER_COMPLETE             2
#define SMCDISCONNECT_FLAG_UNKNOWN                             3
#define NUMBER_SMCDISCONNECT_FLAGS    SMCDISCONNECT_FLAG_UNKNOWN

T_stateMachineHandle SMCDisconnectInitialize(T_void) ;

T_void SMCDisconnectFinish(T_void) ;

T_void SMCDisconnectUpdate(T_void) ;

E_Boolean SMCDisconnectCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMCDisconnectSetFlag(
           T_word16 flag,
           E_Boolean state) ;

#endif // _SMCDIS_H_

/****************************************************************************/
/*    END OF FILE:  SM.H                                                    */
/****************************************************************************/
