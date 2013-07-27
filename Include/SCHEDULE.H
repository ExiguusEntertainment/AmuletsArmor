/****************************************************************************/
/*    FILE:  SCHEDULE.H                                                     */
/****************************************************************************/
#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include "GENERAL.H"

typedef T_void *T_scheduleHandle ;
#define SCHEDULE_HANDLE_BAD NULL

typedef T_void (*T_scheduleEventHandler)(T_word32 data) ;

T_void ScheduleAddEvent(
           T_word32 when,
           T_scheduleEventHandler handler,
           T_word32 data) ;

T_void ScheduleClearEvents(T_void) ;

E_Boolean ScheduleUpdateEvents(T_void) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SCHEDULE.H                                              */
/****************************************************************************/
