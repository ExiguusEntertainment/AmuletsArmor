/****************************************************************************/
/*    FILE:  SCHEDULE.C                                                     */
/****************************************************************************/
#include "MEMORY.H"
#include "SCHEDULE.H"
#include "SYNCTIME.H"
#include "TICKER.H"

typedef struct {
    T_word32 when ;
    T_scheduleEventHandler handler ;
    T_word32 data ;
    T_void *next ;
} T_scheduleEvent ;

static T_scheduleEvent *G_firstScheduleEvent = NULL;

typedef struct {
    T_scheduleEvent *p_firstScheduleEvent ;
} T_scheduleHandleStruct ;

/* Internal prototypes: */
T_void IScheduleInsertSortEvent(T_scheduleEvent *p_newEvent, T_word32 when) ;

/****************************************************************************/
/*  Routine:  ScheduleAddEvent                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScheduleAddEvent adds an event to the schedule list based on the      */
/*  ticker clock.  When this event goes off, the handler is called and      */
/*  the passed in data is passed to the handler.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 when               -- Ticker time for event to occur         */
/*                                                                          */
/*    T_scheduleEventHandler handler -- Callback routine for event          */
/*                                                                          */
/*    T_word32 data               -- Data to store with event (note that    */
/*                                   this could be a pointer cast as a      */
/*                                   32 bit value).                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    IScheduleInsertSortEvent                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScheduleAddEvent(
           T_word32 when,
           T_scheduleEventHandler handler,
           T_word32 data)
{
    T_scheduleEvent *p_event ;

    DebugRoutine("ScheduleAddEvent") ;

    /* Allocate memory for this new event. */
    p_event = MemAlloc(sizeof(T_scheduleEvent)) ;

    DebugCheck(p_event != NULL) ;

    /* Make sure it was allocated. */
    if (p_event != NULL)  {
        /* Store the data in the structure. */
        p_event->when = when ;
        p_event->handler = handler ;
        p_event->data = data ;

        /* Insert sort this item into our linked list of events. */
        IScheduleInsertSortEvent(p_event, when) ;
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ScheduleClearEvents                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScheduleClearEvents removes all events from the schedule.  This is    */
/*  useful when you need to quit processing those events.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScheduleClearEvents(T_void)
{
    T_scheduleEvent *p_event ;
    DebugRoutine("ScheduleClearEvents") ;

    /* Loop and free each event until all are gone. */
    while (G_firstScheduleEvent != NULL)  {
        p_event = G_firstScheduleEvent->next ;
        MemFree(G_firstScheduleEvent) ;
        G_firstScheduleEvent = p_event ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ScheduleUpdateEvents                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScheduleUpdateEvents looks at the beginning of the schedule list      */
/*  and sees if there is an action it needs to perform.  If there is, that  */
/*  action is taken and the appropriate callback routine is called.         */
/*  The event is then discarded and another attempt is made.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- Flag that tells if an event occured    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemFree                                                               */
/*    SyncTimeGet                                                             */
/*    Handler of schedule events                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ScheduleUpdateEvents(T_void)
{
    T_scheduleEvent *p_event ;
    E_Boolean f_eventOccured = FALSE ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;

    DebugRoutine("ScheduleUpdateEvents") ;

    /* Loop while there are events that can be processed. */
    while ((G_firstScheduleEvent != NULL) &&
           (G_firstScheduleEvent->when <= SyncTimeGet()))  {
        /* An event is now going off.  Get that event. */
        p_event = G_firstScheduleEvent ;

        /* Call the handler for this event. */
        DebugCheck(p_event->handler != NULL) ;
        p_event->handler(p_event->data) ;

        /* Remove the event from the list. */
        G_firstScheduleEvent = p_event->next ;
        MemFree(p_event) ;

        /* Note that some event occured. */
        f_eventOccured = TRUE ;
    }

    DebugEnd() ;

    TICKER_TIME_ROUTINE_END(stdout, "ScheduleUpdateEvents", 500) ;
    return f_eventOccured ;
}

/****************************************************************************/
/*  Routine:  IScheduleInsertSortEvent           * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IScheduleInsertSortEvent adds a new event to the event list and       */
/*  tries to find a position where the event is in order from earliest to   */
/*  latest.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_scheduleEvent *p_event    -- Event to add                           */
/*                                                                          */
/*    T_word32 when               -- When the event is to occur             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IScheduleInsertSortEvent(T_scheduleEvent *p_newEvent, T_word32 when)
{
    T_scheduleEvent *p_prev = NULL ;
    T_scheduleEvent *p_event ;

    DebugRoutine("IScheduleInsertSortEvent") ;

    /* Start at the beginning. */
    p_event = G_firstScheduleEvent ;

    while (p_event != NULL)  {
        /* See if the current event is older than our event. */
        /* If it is, stop looping. */
        if (p_event->when > when)
            break ;
        /* If not, make this the last event we got and go forward. */
        p_prev = p_event ;
        p_event = p_event->next ;
    }

    /* Now link up the previous and next links. */
    /* The next is easy, just make it to wherever we got to (even a NULL). */
    p_newEvent->next = p_event ;

    /* The previous link is a little tricky.  If the previous link is */
    /* NULL, then we are at the beginning of the list and need to */
    /* make it the first event.  Otherwise, we make that previous event */
    /* point to this event. */
    if (p_prev == NULL)  {
        G_firstScheduleEvent = p_newEvent ;
    } else {
        p_prev->next = p_newEvent ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SCHEDULE.C                                              */
/****************************************************************************/
