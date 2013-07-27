/****************************************************************************/
/*    FILE:  MEMORY.H                                                       */
/****************************************************************************/
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "GENERAL.H"

typedef T_void (*T_memDiscardCallbackFunc)(T_void *p_block) ;

T_void *MemAlloc(T_word32 size) ;

T_void MemFree(T_void *p_data) ;

T_void MemMarkDiscardable(
           T_void *p_data,
           T_memDiscardCallbackFunc p_callback) ;

T_void MemReclaimDiscardable(T_void *p_data) ;

T_void MemFlushDiscardable(T_void) ;

T_word32 FreeMemory(T_void) ;

#ifndef NDEBUG
T_void MemDumpDiscarded(T_void) ;
T_void MemCheck(T_word16 num) ;
T_void MemCheckData(T_void *p_data) ;

/* Note:  These routines are ONLY available in the DEBUG version. */
T_word32 MemGetAllocated(T_void) ;
T_word32 MemGetMaxAllocated(T_void) ;
#else
#  define MemDumpDiscarded()
#  define MemCheck(num)
#  define MemCheckData(p_data)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  MEMORY.H                                                */
/****************************************************************************/
