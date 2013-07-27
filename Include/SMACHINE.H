/****************************************************************************/
/*    FILE:  SMACHINE.H                                                     */
/****************************************************************************/

#ifndef _SMACHINE_H_
#define _SMACHINE_H_

#include "GENERAL.H"

//#define STATE_MACHINE_TAG (*((T_word32 *)"StMa"))
#define STATE_MACHINE_TAG ((T_word32)('S' << 0) | \
                                     ('t' << 8) | \
                                     ('M' << 16) | \
                                     ('a' << 24))

/* The initialize state of a state machine is always the first */
/* state in the list. */
#define STATE_MACHINE_INITIAL_STATE 0

typedef T_void *T_stateMachineHandle ;
#define STATE_MACHINE_HANDLE_BAD NULL

typedef E_Boolean (*T_stateMachineConditionalCallback)(
                       T_stateMachineHandle handle,
                       T_word32 extraData) ;

typedef T_void (*T_stateMachineStateEnterCallback)(
                     T_stateMachineHandle handle,
                     T_word32 extraData) ;

typedef T_void (*T_stateMachineStateExitCallback)(
                     T_stateMachineHandle handle,
                     T_word32 extraData,
                     E_Boolean isDestroyed) ;

typedef T_void (*T_stateMachineStateIdleCallback)(
                     T_stateMachineHandle handle,
                     T_word32 extraData) ;

typedef T_void (*T_stateMachineInitializeCallback)(
                     T_stateMachineHandle handle) ;

typedef T_void (*T_stateMachineFinishCallback)(
                     T_stateMachineHandle handle) ;

typedef struct {
    T_stateMachineConditionalCallback callback ;
    T_word32 extraData ;
    T_word16 nextState ;
} T_stateMachineConditional ;

typedef struct {
    T_stateMachineStateEnterCallback enterCallback ;
    T_stateMachineStateExitCallback exitCallback ;
    T_stateMachineStateIdleCallback idleCallback ;
    T_word32 extraData ;
    T_word16 numConditionals ;
    T_stateMachineConditional *p_conditionalList ;
} T_stateMachineState ;

typedef struct {
    T_word32 tag ;
    T_stateMachineInitializeCallback initCallback ;
    T_stateMachineFinishCallback finishCallback ;
    T_word16 numStates ;
    T_stateMachineState *p_stateList ;
} T_stateMachine ;

T_stateMachineHandle StateMachineCreate(T_stateMachine *p_stateMachine) ;

T_void StateMachineDestroy(T_stateMachineHandle handle) ;

T_void StateMachineUpdate(T_stateMachineHandle handle) ;

T_void *StateMachineGetExtraData(T_stateMachineHandle handle) ;

T_void StateMachineSetExtraData(T_stateMachineHandle handle, T_void *p_data) ;

T_word16 StateMachineGetState(T_stateMachineHandle handle) ;

T_void StateMachineGotoState(
           T_stateMachineHandle handle,
           T_word16 stateNumber) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SMACHINE.H                                              */
/****************************************************************************/
