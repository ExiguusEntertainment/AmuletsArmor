/****************************************************************************/
/*    FILE:  SM.H                                                           */
/****************************************************************************/
#ifndef _SMCLOGOF_H_
#define _SMCLOGOF_H_

#include "SMACHINE.H"

/* State constants: */
#define SMCLOGOFF_SAVE_CHARACTER_STATE                     0
#define SMCLOGOFF_TIMEOUT_STATE                            1
#define SMCLOGOFF_REPORT_ERROR_STATE                       2
#define SMCLOGOFF_EXIT_STATE                               3
#define SMCLOGOFF_UNKNOWN_STATE                            4

#define NUMBER_SMCLOGOFF_STATES    SMCLOGOFF_UNKNOWN_STATE
#define SMCLOGOFF_INITIAL_STATE    SMCLOGOFF_SAVE_CHARACTER_STATE

/* Conditional flags: */
#define SMCLOGOFF_FLAG_SAVE_COMPLETE                       0
#define SMCLOGOFF_FLAG_TIMEOUT                             1
#define SMCLOGOFF_FLAG_SAVE_ERROR                          2
#define SMCLOGOFF_FLAG_REPORT_ERROR_COMPLETE               3
#define SMCLOGOFF_FLAG_UNKNOWN                             4
#define NUMBER_SMCLOGOFF_FLAGS    SMCLOGOFF_FLAG_UNKNOWN

T_stateMachineHandle SMCLogoffInitialize(T_void) ;

T_void SMCLogoffFinish(T_void) ;

T_void SMCLogoffUpdate(T_void) ;

E_Boolean SMCLogoffCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMCLogoffSetFlag(
           T_word16 flag,
           E_Boolean state) ;

#endif // _SMCLOGOF_H_

/****************************************************************************/
/*    END OF FILE:  SM.H                                                    */
/****************************************************************************/
