/****************************************************************************/
/*    FILE:  STOREDB.C                                                      */
/****************************************************************************/
#include "FILE.H"
#include "MEMORY.H"
#include "STOREDB.H"

#define MAX_DB_STORES             (FLATFILE_OPTIONAL_HEADER_MAX_SIZE / \
                               sizeof(T_flatFileIndex))  // 64
#define STORE_MAX_PER_ITEM     100

#define STORE_HANDLE_TAG        (*((T_word32 *)"StrH"))
#define STORE_HANDLE_DEAD_TAG   (*((T_word32 *)"DsTh"))

typedef struct {
    T_store *p_store ;
    T_word16 numAccessors ;
    T_flatFileIndex index ;
    T_word32 tag ;
} T_storeHandleStruct ;

/* Internal prototypes: */
static T_word32 StoreDBCreate(T_storeDB storeDB) ;

static T_void StoreDBDelete(
           T_storeDB storeDB,
           T_word32 storeID) ;

static T_void StoreDBPut(
           T_storeDB storeDB,
           T_word32 storeID,
           T_store *p_store) ;

static T_store *StoreDBGet(
               T_storeDB storeDB,
               T_word32 storeID) ;

/* Internal global variables: */
static T_word16 G_numAccessesOpen = 0 ;
static T_storeHandleStruct G_storeHandleArray[MAX_DB_STORES] ;

/****************************************************************************/
/*  Routine:  StoreDBOpen                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Opens up a given store database and returns a handle to it.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- Pointer to filename                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_storeDB               -- Handle to store database                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_storeDB StoreDBOpen(T_byte8 *p_filename)
{
    T_flatFile storeDB ;
    E_Boolean isCreated ;
    T_flatFileIndex *p_storeIDList ;
    T_word16 i ;

    DebugRoutine("StoreDBOpen") ;
    DebugCheck(G_numAccessesOpen == 0) ;

    /* Initialize the global variables. */
    G_numAccessesOpen = 0 ;
    memset(G_storeHandleArray, 0, sizeof(G_storeHandleArray)) ;

    /* Check if the database exists. */
    if (FileExist(p_filename))
        isCreated = FALSE ;
    else
        isCreated = TRUE ;

    /* Open or create the database. */
	storeDB = FlatFileOpenOrCreate(p_filename, sizeof(T_store)) ;
	DebugCheck(storeDB != FLATFILE_BAD) ;

    /* Get the header to all the indices for index info. */
    p_storeIDList = FlatFileGetOptionalHeader(storeDB) ;
    if (isCreated)  {
        /* Start up the new list with blank indexes. */
        for (i=0; i<MAX_DB_STORES; i++)
           p_storeIDList[i] = FLATFILE_INDEX_BAD ;

        FlatFileMarkDirty(storeDB) ;
    }

    /* Null out the handles. */
    for (i=0; i<MAX_DB_STORES; i++)
        G_storeHandleArray[i].tag = STORE_HANDLE_DEAD_TAG ;

	FlatFileRefresh(storeDB) ;

	DebugEnd() ;

    return storeDB ;
}

/****************************************************************************/
/*  Routine:  StoreDBClose                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Closes a previously opened store database.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to close                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StoreDBClose(T_storeDB storeDB)
{
    DebugRoutine("StoreDBClose") ;
    DebugCheck(G_numAccessesOpen == 0) ;

	FlatFileRefresh(storeDB) ;
	FlatFileClose(storeDB) ;

	DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StoreDBGet                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Gets a copy of a store in the database and returns it via a           */
/*  pointer.  When done with the store, just do a MemFree.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to get store                    */
/*                                                                          */
/*    T_word32 storeID        -- ID of store to fetch                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_store *StoreDBGet(
               T_storeDB storeDB,
               T_word32 storeID)
{
    T_store *p_store ;

    DebugRoutine("StoreDBGet") ;

    p_store = (T_store *)FlatFileGetRecord(storeDB, storeID);

	DebugEnd() ;

	return p_store ;
}

/****************************************************************************/
/*  Routine:  StoreDBPut                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Stores a the given store into the store database overwriting          */
/*  what is previously there.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to get store                    */
/*                                                                          */
/*    T_word32 storeID        -- ID of store to fetch                       */
/*                                                                          */
/*    T_store *p_store    -- Pointer to store to store                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void StoreDBPut(
           T_storeDB storeDB,
           T_word32 storeID,
           T_store *p_store)
{
    DebugRoutine("StoreDBPut") ;

	FlatFilePutRecord(storeDB, storeID, p_store) ;
	FlatFileRefresh(storeDB) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StoreDBDelete                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Deletes a previously created store.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to get store                    */
/*                                                                          */
/*    T_word32 storeID        -- ID of store to fetch                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void StoreDBDelete(
           T_storeDB storeDB,
           T_word32 storeID)
{
    DebugRoutine("StoreDBDelete") ;

	FlatFileDeleteRecord(storeDB, storeID) ;
	FlatFileRefresh(storeDB) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StoreDBCreate                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Creates a new, fresh, spanking store                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to create store in              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- store ID                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 StoreDBCreate(T_storeDB storeDB)
{
    T_word32 storeID ;

    DebugRoutine("StoreDBDelete") ;

	storeID = FlatFileCreateRecord(storeDB) ;
	FlatFileRefresh(storeDB) ;

	DebugEnd() ;

    return storeID ;
}

/****************************************************************************/
/*  Routine:  StoreDBStartAccess                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Starts up an access handle to one of the stores whether it is open    */
/*  or closed already.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to get access within            */
/*                                                                          */
/*    T_word32 storeID    -- Store number to access (0-63)                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_storeHandle               -- handle to store to access.             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_storeHandle StoreDBStartAccess(
                  T_storeDB storeDB,
                  T_word32 storeNum)
{
    T_storeHandleStruct *p_handle = NULL ;
    T_flatFileIndex index ;
    T_flatFileIndex *p_indexList ;
    T_store *p_store ;

    DebugRoutine("StoreDBStartAccess") ;
    DebugCheck(storeNum < MAX_DB_STORES) ;

    /* Make sure that this storeID is valid. */
    if (storeNum < MAX_DB_STORES)  {
        /* Get a handle reference. */
        p_handle = &G_storeHandleArray[storeNum] ;

        /* First, see if this record is already being accessed. */
        if (p_handle->numAccessors != 0)  {
            /* Yes, the store is already being accessed. */
            /* Increment the accessor count and return that handle. */
            p_handle->numAccessors++ ;
        } else {
            /* Nobody has accessed that store yet.  */
            /* Get the record index */
            p_indexList = (T_flatFileIndex *)
                              FlatFileGetOptionalHeader(storeDB) ;
            index = p_indexList[storeNum] ;
            /* Has this index been allocated? */
            if (index == FLATFILE_INDEX_BAD)  {
                /* No, we need a new store. */
                index = StoreDBCreate(storeDB) ;
                /* Save the index. */
                p_indexList[storeNum] = index ;

                /* Lock in the store and clear out the important */
                /* part. */
                p_store = StoreDBGet(storeDB, index) ;
                DebugCheck(p_store != NULL) ;
                if (p_store != NULL)   {
                    p_store->numItems = 0 ;
                    StoreDBPut(storeDB, index, p_store) ;
                    MemFree(p_store) ;
                }

                /* Dirty and save this change. */
                FlatFileMarkDirty(storeDB) ;
                FlatFileRefresh(storeDB) ;
            }

            /* Now that we have an index to a good store, */
            /* get it and attach it to an appropriate handle. */
            p_handle->p_store = StoreDBGet(storeDB, index) ;

            /* Record the index. */
            p_handle->index = index ;

            /* Tag the handle for use. */
            p_handle->tag = STORE_HANDLE_TAG ;

            /* Note that there is one person accessing this */
            /* store. */
            p_handle->numAccessors++ ;
        }
    }

    DebugEnd() ;

    return (T_storeHandle)p_handle ;
}

/****************************************************************************/
/*  Routine:  StoreDBEndAccess                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Ends a session with a store.  The handle is now invalid.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeDB storeDB   -- store database to end access with              */
/*                                                                          */
/*    T_storeHandle storeHandle -- Store number to access (0-63)            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_storeHandle               -- handle to store to access.             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StoreDBEndAccess(
           T_storeDB storeDB,
           T_storeHandle storeHandle)
{
    T_storeHandleStruct *p_handle ;

    DebugRoutine("StoreDBEndAccess") ;
    DebugCheck(storeHandle != STORE_HANDLE_BAD) ;

    /* Get a quick pointer. */
    p_handle = (T_storeHandleStruct *)storeHandle ;
    DebugCheck(p_handle->tag == STORE_HANDLE_TAG) ;

    /* Make the accessing one less. */
    DebugCheck(p_handle->numAccessors != 0) ;
    if (p_handle->numAccessors != 0)
        p_handle->numAccessors-- ;

    /* Save the store info. */
    StoreDBPut(
        storeDB,
        p_handle->index,
        p_handle->p_store) ;

    /* Release the record if no one is accessing the store. */
    if (p_handle->numAccessors == 0)  {
        p_handle->tag = STORE_HANDLE_DEAD_TAG ;
        MemFree(p_handle->p_store) ;
        p_handle->p_store = NULL ;
        p_handle->index = FLATFILE_INDEX_BAD ;
    }

    /* Make sure all changes have been saved. */
    FlatFileRefresh(storeDB) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StoreDBAddItem                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StoreDBAddItem puts a given number of items into the store.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeHandle storeHandle -- Handle to store to put items in          */
/*                                                                          */
/*    T_word16 typeItem         -- Type of item being added                 */
/*                                                                          */
/*    T_word16 numberItem       -- Number of items of this type             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_storeAddResult            -- Result of the add.                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_storeAddResult StoreDBAddItem(
           T_storeHandle storeHandle,
           T_word16 typeItem,
           T_word16 numberItem)
{
    E_Boolean foundItem = FALSE ;
    T_storeHandleStruct *p_handle ;
    T_store *p_store ;
    T_word16 i ;
    T_word32 total ;
    T_storeAddResult addResult = STORE_ADD_RESULT_OK ;

    DebugRoutine("StoreDBAddItem") ;
    DebugCheck(numberItem < STORE_MAX_PER_ITEM) ;

    /* Make sure the handle is valid. */
    DebugCheck(storeHandle != STORE_HANDLE_BAD) ;
    if (storeHandle != STORE_HANDLE_BAD)  {
        /* Get a quick pointer. */
        p_handle = (T_storeHandleStruct *)storeHandle ;

        /* Make sure we are accessing an active handle. */
        DebugCheck(p_handle->tag == STORE_HANDLE_TAG) ;
        if (p_handle->tag == STORE_HANDLE_TAG)  {
            /* Get a quick pointer to the store. */
            p_store = p_handle->p_store ;

            /* Search for a match in the store. */
            for (i=0; i<p_store->numItems; i++)  {
                if (p_store->itemArray[i].itemType == typeItem)  {
                    /* Found a match. */
                    /* Add up the total # of items. */
                    total = p_store->itemArray[i].numItems ;
                    total += numberItem ;

                    /* Is this too many? */
                    if (total > STORE_MAX_PER_ITEM)  {
                        addResult = STORE_ADD_RESULT_TOO_MANY_OF_ITEM ;
                    } else {
                        /* There is room, store this new count. */
                        p_store->itemArray[i].numItems = total ;
//printf("store %p item %d now %ld\n", storeHandle, i, total) ;
                    }

                    /* stop looping here. */
                    foundItem = TRUE ;
                    break ;
                }
            }

            /* Did we not find a matching item? */
            if (foundItem == FALSE)  {
                /* Can we add this to the list? */
                if (p_store->numItems == STORE_MAX_ITEMS)  {
                    /* Tell the caller that there is no room */
                    /* to add this type of item. */
                    addResult = STORE_ADD_RESULT_TOO_MANY_ITEMS ;
                } else {
                    /* Add the item to the list */
                    i = p_store->numItems++ ;
                    p_store->itemArray[i].numItems = numberItem ;
                    p_store->itemArray[i].itemType = typeItem ;
                }
            }
        }
    }

    DebugEnd() ;

    return addResult ;
}

/****************************************************************************/
/*  Routine:  StoreDBRemoveItem                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StoreDBRemoveItem gets a given number of items from the store.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeHandle storeHandle -- Handle to store to get items from        */
/*                                                                          */
/*    T_word16 typeItem         -- Type of item being removed               */
/*                                                                          */
/*    T_word16 numberItem       -- Number of items of this type             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_storeAddResult            -- Result of the add.                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_storeRemoveResult StoreDBRemoveItem(
           T_storeHandle storeHandle,
           T_word16 typeItem,
           T_word16 numberItem)
{
    E_Boolean foundItem = FALSE ;
    T_storeHandleStruct *p_handle ;
    T_store *p_store ;
    T_word16 i, j ;
    T_storeRemoveResult removeResult = STORE_REMOVE_RESULT_OK ;

    DebugRoutine("StoreDBRemoveItem") ;

    /* Make sure the handle is valid. */
    DebugCheck(storeHandle != STORE_HANDLE_BAD) ;
    if (storeHandle != STORE_HANDLE_BAD)  {
        /* Get a quick pointer. */
        p_handle = (T_storeHandleStruct *)storeHandle ;

        /* Make sure we are accessing an active handle. */
        DebugCheck(p_handle->tag == STORE_HANDLE_TAG) ;
        if (p_handle->tag == STORE_HANDLE_TAG)  {
            /* Get a quick pointer to the store. */
            p_store = p_handle->p_store ;

            /* Search for a match in the store. */
            for (i=0; i<p_store->numItems; i++)  {
                if (p_store->itemArray[i].itemType == typeItem)  {
                    /* Found a match. */

                    /* Is there enough here? */
                    if (p_store->itemArray[i].numItems >= numberItem)  {
                        /* Yes, there is enough. */
                        /* Remove the items. */
                        p_store->itemArray[i].numItems -= numberItem ;
//printf("store %p item %d now %d\n", p_handle, i, p_store->itemArray[i].numItems) ;
                        /* See if we need to squeeze the list. */
                        if (p_store->itemArray[i].numItems == 0)  {
                            /* Yes, squeeze the list. */
                            for (j=i; j<p_store->numItems; j++)
                                p_store->itemArray[j] =
                                    p_store->itemArray[j+1] ;

                            /* One less item in the list now. */
                            p_store->numItems-- ;
                        }
                    } else {
                        /* No, there is not enough.  Note it. */
                        removeResult = STORE_REMOVE_RESULT_NOT_ENOUGH ;
                    }

                    /* stop looping here. */
                    foundItem = TRUE ;
                    break ;
                }
            }

            /* Did we not find a matching item? */
            if (foundItem == FALSE)  {
                /* Tell the caller that the item does not exist. */
                removeResult = STORE_REMOVE_RESULT_NONE_TO_TAKE ;
            }
        }
    }

    DebugEnd() ;

    return removeResult ;
}

/****************************************************************************/
/*  Routine:  StoreDBGetInventory                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StoreDBGetInventory returns a pointer to a currently active store     */
/*  inventory.  Note that this data will change if you keep the pointer.    */
/*  The returned pointer especially is no longer valid when you end         */
/*  the store access.                                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_storeHandle storeHandle -- Handle to store to get inventory         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_store *                   -- Store inventory                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_store *StoreDBGetInventory(T_storeHandle storeHandle)
{
    T_storeHandleStruct *p_handle ;
    T_store *p_store = NULL ;

    DebugRoutine("StoreDBGetInventory") ;

    if (storeHandle != STORE_HANDLE_BAD)  {
        /* Get a quick pointer. */
        p_handle = (T_storeHandleStruct *)storeHandle ;
        p_store = p_handle->p_store ;
    } else {
        p_store = NULL ;
    }

    DebugEnd() ;

    return p_store ;
}

/****************************************************************************/
/*    END OF FILE:  STOREDB.C                                               */
/****************************************************************************/
