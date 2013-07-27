/****************************************************************************/
/*    FILE:  VM.H                                                           */
/****************************************************************************/
#ifndef _VM_H_
#define _VM_H_

#include "GENERAL.H"

typedef void *T_vmFile ;

typedef T_word32 T_vmBlock ;

#define VM_BLOCK_BAD ((T_vmBlock) 0)

typedef enum {
    VM_ACCESS_NONE,
    VM_ACCESS_READ,
    VM_ACCESS_WRITE,
    VM_ACCESS_READ_WRITE,
    VM_ACCESS_UNKNOWN
} F_vmAccess ;


T_vmFile VMOpen(T_byte8 *p_filename) ;

T_void VMClose(T_vmFile file) ;

T_vmBlock VMAlloc(T_vmFile file, T_word32 size) ;

T_void VMIncRefCount(T_vmFile file, T_vmBlock block) ;

T_void VMDecRefCount(T_vmFile file, T_vmBlock block) ;

T_word16 VMGetRefCount(T_vmFile file, T_vmBlock block) ;

void *VMLock(
         T_vmFile file,
         T_vmBlock block) ;

T_void VMUnlock(void *p_block) ;

T_void VMCleanUp(T_byte8 *p_filename) ;

T_void VMDirty(T_vmFile file, T_vmBlock block) ;

#endif

/****************************************************************************/
/*    END OF FILE:  VM.H                                                    */
/****************************************************************************/
