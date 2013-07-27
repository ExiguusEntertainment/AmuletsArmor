/****************************************************************************/
/*    FILE:  STOREDB.H                                                      */
/****************************************************************************/
#ifndef _STORE_DB_H_
#define _STORE_DB_H_

/* The database in this module is a database of all the stores. */
/* Each store is an independent record. */

#include "FLATFILE.H"
#include "GENERAL.H"

#define MAX_STORES   64

/* Handle to database. */
typedef T_flatFile T_storeDB ;
#define STOREDB_BAD   FLATFILE_BAD

typedef T_void *T_storeHandle ;
#define STORE_HANDLE_BAD         NULL

T_storeDB StoreDBOpen(T_byte8 *p_filename) ;

T_void StoreDBClose(T_storeDB storeDB) ;

T_storeHandle StoreDBStartAccess(
                  T_storeDB storeDB,
                  T_word32 storeNum) ;

T_void StoreDBEndAccess(
           T_storeDB storeDB,
           T_storeHandle storeHandle) ;

typedef T_byte8 T_storeAddResult ;
#define STORE_ADD_RESULT_OK                  0
#define STORE_ADD_RESULT_TOO_MANY_ITEMS      1
#define STORE_ADD_RESULT_TOO_MANY_OF_ITEM    2
#define STORE_ADD_RESULT_UNKNOWN             3

T_storeAddResult StoreDBAddItem(
           T_storeHandle storeHandle,
           T_word16 typeItem,
           T_word16 numberItem) ;

typedef T_byte8 T_storeRemoveResult ;
#define STORE_REMOVE_RESULT_OK                  0
#define STORE_REMOVE_RESULT_NONE_TO_TAKE        1
#define STORE_REMOVE_RESULT_NOT_ENOUGH          2
#define STORE_REMOVE_RESULT_UNKNOWN             3

T_storeRemoveResult StoreDBRemoveItem(
           T_storeHandle storeHandle,
           T_word16 typeItem,
           T_word16 numberItem) ;


#define STORE_MAX_ITEMS        100

typedef struct {
    T_word16 numItems ;
    T_word16 itemType ;
} T_storeItem ;

typedef struct {
    T_word16 numItems ;
    T_storeItem itemArray[STORE_MAX_ITEMS] ;
} T_store ;

T_store *StoreDBGetInventory(T_storeHandle storeHandle) ;

#endif

/****************************************************************************/
/*    END OF FILE:  STOREDB.H                                               */
/****************************************************************************/
