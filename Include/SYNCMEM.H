/****************************************************************************/
/*    FILE:  SYNCMEM.H                                                      */
/****************************************************************************/
#ifndef _SYNCMEM_H_
#define _SYNCMEM_H_

#include "GENERAL.H"

#define SYNCMEM_SIZE    8000

#ifndef NDEBUG
T_void SyncMemAdd(char *p_name, T_word32 d1, T_word32 d2, T_word32 d3) ;
T_void SyncMemDump(T_void) ;
T_void SyncMemClear(T_void) ;
T_void SyncMemDumpOnce(T_void) ;
T_word16 SyncMemGetChecksum(T_void) ;
#else
#define SyncMemAdd(a, b, c, d)
#define SyncMemDump()
#define SyncMemClear()
#define SyncMemDumpOnce()
#define SyncMemGetChecksum() 0
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  ACTIVITY.H                                              */
/****************************************************************************/
