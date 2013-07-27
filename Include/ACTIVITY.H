/****************************************************************************/
/*    FILE:  ACTIVITY.H                                                     */
/****************************************************************************/

#ifndef _ACTIVITY_H_
#define _ACTIVITY_H_

#include "GENERAL.H"

typedef T_void *T_activitiesHandle ;
#define ACTIVITY_HANDLE_BAD SCRIPT_BAD

T_activitiesHandle ActivitiesLoad(T_word32 number) ;

T_void ActivitiesUnload(void) ;

T_void ActivitiesRun(T_word16 numberOfActivity) ;

T_void ActivitiesSetActive(T_activitiesHandle handle) ;

#endif

/****************************************************************************/
/*    END OF FILE:  ACTIVITY.H                                              */
/****************************************************************************/
