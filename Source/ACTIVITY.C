/****************************************************************************/
/*    FILE:  ACTIVITY.C                                                     */
/****************************************************************************/

#include "ACTIVITY.H"
#include "GENERAL.H"
#include "SCRIPT.H"

/* Note where the table, strings, and program data are located */
static T_script G_activityScript = SCRIPT_BAD ;

/* LES 01/04/96 */
/* LES 06/05/96 -- Added use of activities handle. */
T_activitiesHandle ActivitiesLoad(T_word32 number)
{
    DebugRoutine("ActivitiesLoad") ;

    G_activityScript = ScriptLock(number) ;

    DebugEnd() ;

    return (T_activitiesHandle)G_activityScript ;
}

/* LES 01/04/96 */
T_void ActivitiesUnload(T_void)
{
    DebugRoutine("ActivitiesUnload") ;

    ScriptUnlock(G_activityScript) ;

    DebugEnd() ;
}

/* LES 01/04/96 */
T_void ActivitiesRun(T_word16 numberOfActivity)
{
    DebugRoutine("ActivitiesRun") ;

    ScriptRunPlace(G_activityScript, numberOfActivity) ;

    DebugEnd() ;
}

/* LES 06/05/96 */
T_void ActivitiesSetActive(T_activitiesHandle handle)
{
    DebugRoutine("ActivitiesSetActive") ;

    G_activityScript = (T_script)handle ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  ACTIVITY.C                                              */
/****************************************************************************/
