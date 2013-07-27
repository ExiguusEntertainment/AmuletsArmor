/****************************************************************************/
/*    FILE:  SMACHINE.C                                                     */
/****************************************************************************/
#include "MEMORY.H"
#include "SMACHINE.H"

#define STATE_MACHINE_INSTANCE_TAG      (*((T_word32 *)"SmIt"))
#define STATE_MACHINE_INSTANCE_DEAD_TAG (*((T_word32 *)"DsMi"))

typedef struct {
    T_word32 tag ;
    T_word16 currentState ;
    T_void *p_extraData ;
    T_stateMachine *p_stateMachine ;
} T_stateMachineInstance ;

/* A state of none means that no state has been declared yet */
/* for the state machine.  This should only occur at the */
/* very beginning of the state machine. */
#define STATE_MACHINE_STATE_NONE 0xFFFF

/* Internal prototypes: */
static T_stateMachineInstance *IIsValidStateMachine(
                                  T_stateMachineHandle handle) ;

/****************************************************************************/
/*  Routine:  StateMachineCreate                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineCreate starts up an instance of a state machine.  Just    */
/*  pass in the state machine you want to execute.  Then make calls to      */
/*  StateMachineUpdate.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachine *p_stateMachine -- "Form" of state machine to start up */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_stateMachineHandle         -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    memset                                                                */
/*    StateMachineGotoState                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_stateMachineHandle StateMachineCreate(T_stateMachine *p_stateMachine)
{
    T_stateMachineInstance *p_machine ;

    DebugRoutine("StateMachineCreate") ;
    DebugCheck(p_stateMachine != NULL) ;

    /* Allocate memory for the instance. */
    p_machine = (T_stateMachineInstance *)
                  MemAlloc(sizeof(T_stateMachineInstance)) ;
    DebugCheck(p_machine != NULL) ;
    if (p_machine)  {
        /* clear out the instance. */
        memset(p_machine, 0, sizeof(T_stateMachineInstance)) ;

        p_machine->tag = STATE_MACHINE_INSTANCE_TAG ;
        p_machine->currentState = STATE_MACHINE_STATE_NONE ;
        p_machine->p_extraData = NULL ;
        p_machine->p_stateMachine = p_stateMachine ;

//        printf("Created handle %p\n", p_stateMachine) ;

        if (p_stateMachine->initCallback)
            p_stateMachine->initCallback((T_stateMachineHandle)p_machine) ;

//        StateMachineGotoState(
//            (T_stateMachineHandle)p_machine,
//            STATE_MACHINE_INITIAL_STATE) ;
    }


//    DebugCompare("StateMachineCreate") ;
    DebugEnd() ;

    return ((T_stateMachineHandle)p_machine) ;
}

/****************************************************************************/
/*  Routine:  StateMachineDestroy                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineDestroy gets rid of all allocated memory related to a     */
/*  state machine after calling the finish callback.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does not do a MemFree on the extra data attached to the  */
/*  the state machine.  If there is allocated memory attached, it is up     */
/*  to the caller to maker sure it is either disposed before this routine   */
/*  is called or in the finish callback.                                    */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StateMachineDestroy(T_stateMachineHandle handle)
{
    T_stateMachine *p_stateMachine ;
    T_stateMachineInstance *p_machine ;

    DebugRoutine("StateMachineDestroy") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
        /* GEt a quick pointer to the state machine "form". */
        p_stateMachine = p_machine->p_stateMachine ;

        /* Move to the 'non' state of the system. */
        /* This is make the state machine close out the */
        /* current state. */
        StateMachineGotoState(handle, STATE_MACHINE_STATE_NONE) ;

        /* Call the finish callback if need be. */
        if (p_stateMachine->finishCallback)
            p_stateMachine->finishCallback(handle) ;

        /* Now destroy all the associated data. */
        p_machine->tag = STATE_MACHINE_INSTANCE_DEAD_TAG ;
        MemFree(p_machine) ;
    }

//    DebugCompare("StateMachineDestroy") ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StateMachineUpdate                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineUpdate does the "action" for a state.  It checks all the  */
/*  conditions that transition a state to another state.                    */
/*    Idle callbacks are only called when there was no state change.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine will only transition from one state to another.  It      */
/*  never does two at a time or string together transitions.  If multiple   */
/*  state changes is needed, call this routine multiple times, checking     */
/*  to see if the state changes after each call.                            */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StateMachineUpdate(T_stateMachineHandle handle)
{
    T_stateMachine *p_stateMachine ;
    T_stateMachineInstance *p_machine ;
    T_stateMachineState *p_state ;
    E_Boolean stateChangeFound = FALSE ;
    T_word16 i ;
    T_stateMachineConditional *p_cond ;

    DebugRoutine("StateMachineUpdate") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
        /* Get a quick pointer to the state machine "form". */
        p_stateMachine = p_machine->p_stateMachine ;

        /* Make sure we are not in the 'non' state. */
        if (p_machine->currentState != STATE_MACHINE_STATE_NONE)  {
//printf("State machine: update state: %d (%s)\n", p_machine->currentState, DebugGetCallerName()) ;
//fflush(stdout) ;
            /* Get a pointer to the state info. */
            p_state = p_stateMachine->p_stateList +
                          p_machine->currentState ;

            /* Go through the list of conditions and call each */
            /* looking for a TRUE (OK to change state). */
            /* If any one is found, do that state change. */
            /* If none are found, do an idle callback. */
            for (i=0; i<p_state->numConditionals; i++)  {
//printf("State macine: update cond: %d\n", i) ;
//fflush(stdout) ;
//DebugCompare("StateMachineUpdate") ;
                /* Find a conditional to check. */
                p_cond = p_state->p_conditionalList + i ;

                DebugCheck(p_cond->callback != NULL) ;
                if (p_cond->callback)  {
                    /* See if a state change is to be found. */
//printf("extra data: %ld\n", p_cond->extraData) ;
                    if (p_cond->callback(
                             handle,
                             p_cond->extraData) == TRUE)  {
                        stateChangeFound = TRUE ;

                        /* Change to the new state. */
                        StateMachineGotoState(
                            handle,
                            p_cond->nextState) ;

                        /* Stop looping. */
                        break ;
                    }
                }
            }

            if (!stateChangeFound)   {
                /* No state change, try doing an idle callback. */
                if (p_state->idleCallback)
                    p_state->idleCallback(
                        handle,
                        p_state->extraData) ;
            }
        }
    }

//    DebugCompare("StateMachineUpdate") ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StateMachineGetExtraData                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineGetExtraData returns a pointer to the data associated     */
/*  with this instance of the state machine.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_void *                     -- A pointer to the attached data        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *StateMachineGetExtraData(T_stateMachineHandle handle)
{
    T_void *p_data = NULL ;      /* Default answer. */
    T_stateMachineInstance *p_machine ;

    DebugRoutine("StateMachineGetExtraData") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
        /* Get the extra data. */
        p_data = p_machine->p_extraData ;
    }

    DebugEnd() ;

    return p_data ;
}


/****************************************************************************/
/*  Routine:  StateMachineSetExtraData                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineSetExtraData declares what piece of data is attached      */
/*  to the given state machine.                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StateMachineSetExtraData(T_stateMachineHandle handle, T_void *p_data)
{
    T_stateMachineInstance *p_machine ;

    DebugRoutine("StateMachineSetExtraData") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
        /* Set the extra data. */
        p_machine->p_extraData = p_data ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StateMachineGetState                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineGetState returns the current state number of the given    */
/*  state machine.                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You should try not to call this routine unless you have very specific */
/*  states.  Should you edit the order of states in the state list, your    */
/*  state indexes/ids are not invalid.                                      */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                     -- State number                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 StateMachineGetState(T_stateMachineHandle handle)
{
    T_word16 currentState ;
    T_stateMachineInstance *p_machine ;

    DebugRoutine("StateMachineGetState") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
        /* Get the current state. */
        currentState = p_machine->currentState ;
    }

    DebugEnd() ;

    return currentState ;
}

/****************************************************************************/
/*  Routine:  StateMachineGotoState                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StateMachineGotoState does the work of switching between states and   */
/*  executing callback routines as necessary.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to state machine being worked  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IIsValidStateMachine                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StateMachineGotoState(
           T_stateMachineHandle handle,
           T_word16 stateNumber)
{
    T_stateMachine *p_stateMachine ;
    T_stateMachineInstance *p_machine ;
    T_stateMachineState *p_state ;
    E_Boolean isDestroyed = FALSE ;

    DebugRoutine("StateMachineGotoState") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    if ((p_machine = IIsValidStateMachine(handle)) != NULL)  {
//printf("State Machine %p going to state %d\n",
//  handle,
//  stateNumber) ;
//fflush(stdout) ;

        /* GEt a quick pointer to the state machine "form". */
        p_stateMachine = p_machine->p_stateMachine ;

        /* Check if this is a valid state */
        if ((stateNumber == STATE_MACHINE_STATE_NONE) ||
            (stateNumber < p_stateMachine->numStates))  {
            /* This is a valid state to go to. */

            /* Are we going to the none state and thus */
            /* are being destroyed. */
            if (stateNumber == STATE_MACHINE_STATE_NONE)
                isDestroyed = TRUE ;

            /* Close out the old state we were in. */
            if (p_machine->currentState != STATE_MACHINE_STATE_NONE)  {
                /* Get a pointer to the old state info. */
                p_state = p_stateMachine->p_stateList +
                              p_machine->currentState ;

                /* Call the exit state callback if available. */
                if (p_state->exitCallback)
                    p_state->exitCallback(
                        handle,
                        p_state->extraData,
                        isDestroyed) ;
            }

            /* Record this to be in the new state. */
            p_machine->currentState = stateNumber ;

            /* Start up the new state we are going to. */
            if (stateNumber != STATE_MACHINE_STATE_NONE)  {
                /* Get a pointer to the new state info. */
                p_state = &(p_stateMachine->p_stateList[stateNumber]) ;

                /* Call the enter state callback if availabale. */
                if (p_state->enterCallback)
                    p_state->enterCallback(
                        handle,
                        p_state->extraData) ;
            }
        } else {
            DebugCheck(FALSE) ;
        }
    }

//    DebugCompare("StateMachineGotoState") ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IIsValidStateMachine               * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IIsValidStateMachine checks the given handle and sees if it is        */
/*  actually a handle to a state machine.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_stateMachineHandle handle  -- Handle to validate                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_stateMachineInstance *     -- Pointer to the actual instance, else  */
/*                                    NULL.                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_stateMachineInstance *IIsValidStateMachine(
                                  T_stateMachineHandle handle)
{
    T_stateMachineInstance *p_machine = NULL ;

    DebugRoutine("IIsValidStateMachine") ;
    DebugCheck(handle != STATE_MACHINE_HANDLE_BAD) ;
    if (handle != STATE_MACHINE_HANDLE_BAD)  {
        p_machine = (T_stateMachineInstance *)handle ;
        DebugCheck(p_machine->tag == STATE_MACHINE_INSTANCE_TAG) ;
        DebugCheck(p_machine->p_stateMachine != NULL) ;
    }

    DebugEnd() ;

    return p_machine ;
}

/****************************************************************************/
/*    END OF FILE:  SMACHINE.C                                              */
/****************************************************************************/
