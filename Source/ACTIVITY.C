/*-------------------------------------------------------------------------*
 * File:  ACTIVITY.C
 *-------------------------------------------------------------------------*/
/**
 * Scripts on the current map are processed through the Activities
 * system.  It really just a glorified 'globally loaded script'.
 *
 * @addtogroup ACTIVITY
 * @brief Map Script Activities
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
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

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  ACTIVITY.C
 *-------------------------------------------------------------------------*/
