/****************************************************************************/
/*    FILE:  SMLEAVE.H                                                           */
/****************************************************************************/
#ifndef _SMCLEAVE_H_
#define _SMCLEAVE_H_

#include "SMACHINE.H"

/* State constants: */
#define SMCLEAVE_SEND_LOGOFF_STATE                         0
#define SMCLEAVE_HANG_UP_STATE                             1
#define SMCLEAVE_LEAVE_COMPLETE_STATE                      2
#define SMCLEAVE_UNKNOWN_STATE                             3

#define NUMBER_SMCLEAVE_STATES    SMCLEAVE_UNKNOWN_STATE
#define SMCLEAVE_INITIAL_STATE    SMCLEAVE_SEND_LOGOFF_STATE

/* Conditional flags: */
#define SMCLEAVE_FLAG_LOGOFF_COMPLETE                     0
#define SMCLEAVE_FLAG_TIMEOUT                             1
#define SMCLEAVE_FLAG_HANGUP_COMPLETE                     2
#define SMCLEAVE_FLAG_UNKNOWN                             3
#define NUMBER_SMCLEAVE_FLAGS    SMCLEAVE_FLAG_UNKNOWN

T_stateMachineHandle SMCLeaveInitialize(T_void) ;

T_void SMCLeaveFinish(T_void) ;

T_void SMCLeaveUpdate(T_void) ;

E_Boolean SMCLeaveCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMCLeaveSetFlag(
           T_word16 flag,
           E_Boolean state) ;

#endif /* _SMCLEAVE_H_ */

/****************************************************************************/
/*    END OF FILE:  SM.H                                                    */
/****************************************************************************/
