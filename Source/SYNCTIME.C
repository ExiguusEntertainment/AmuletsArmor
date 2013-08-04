/*-------------------------------------------------------------------------*
 * File:  SYNCTIME.C
 *-------------------------------------------------------------------------*/
/**
 * When doing network gaming, a separate Synchronized time is kept.
 *
 * @addtogroup SYNCTIME
 * @brief Syncrhonized Time Module
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "SYNCTIME.H"

/* The actual sync time value */
T_word32 G_syncTime = 1;

/*-------------------------------------------------------------------------*
 * Routine:  SyncTimeGet
 *-------------------------------------------------------------------------*/
/**
 *  Return the global sync time.
 *
 *  @return sync time
 *
 *<!-----------------------------------------------------------------------*/
T_word32 SyncTimeGet(T_void)
{
    return G_syncTime ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SyncTimeSet
 *-------------------------------------------------------------------------*/
/**
 *  Sets the global sync time.
 *
 *  @param time -- new sync time
 *
 *<!-----------------------------------------------------------------------*/
T_void SyncTimeSet(T_word32 time)
{
    G_syncTime = time ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SYNCTIME.C
 *-------------------------------------------------------------------------*/
