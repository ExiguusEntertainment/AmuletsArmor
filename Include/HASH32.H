/****************************************************************************/
/*    FILE:  HASH32.H                                                       */
/****************************************************************************/
#ifndef _HASH32_H_
#define _HASH32_H_

#include "GENERAL.H"

typedef T_void *T_hash32 ;
#define HASH32_BAD NULL

typedef T_void *T_hash32Item ;
#define HASH32_ITEM_BAD NULL

T_hash32 Hash32Create(T_word16 indexSize) ;

T_void Hash32Destroy(T_hash32 hash) ;

T_hash32Item Hash32Add(T_hash32 hash, T_word32 key, T_void *p_data) ;

T_hash32Item Hash32Find(T_hash32 hash, T_word32 key) ;

T_void Hash32Remove(T_hash32Item hashItem) ;

T_hash32Item Hash32GetFirstItem(T_hash32 hash) ;

T_hash32Item Hash32ItemGetNext(T_hash32Item hashItem) ;

T_void *Hash32ItemGetData(T_hash32Item hashItem) ;

T_word32 Hash32GetNumberItems(T_hash32 hash) ;

T_word32 Hash32ItemGetKey(T_hash32Item hashItem) ;

#endif

/****************************************************************************/
/*    END OF FILE:  HASH32.H                                                */
/****************************************************************************/
