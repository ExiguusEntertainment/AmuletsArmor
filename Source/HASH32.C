/****************************************************************************/
/*    FILE:  HASH32.C                                                       */
/****************************************************************************/
#include "DBLLINK.H"
#include "HASH32.H"
#include "MEMORY.H"

#define HASH32_TAG                      (*((T_word32 *)"Hs32"))
#define HASH32_DEAD_TAG                 (*((T_word32 *)"h32D"))

#define HASH32_ITEM_TAG                 (*((T_word32 *)"HaIt"))
#define HASH32_ITEM_DEAD_TAG            (*((T_word32 *)"DHiT"))

typedef struct {
    T_word32 tag ;
    T_word32 key ;
    T_void *p_data ;
    T_hash32 ownerHash ;
    T_doubleLinkListElement elementInBigList ;
    T_doubleLinkListElement elementInHashList ;
} T_hash32ItemStruct ;

typedef struct {
    T_word32 tag ;
    T_doubleLinkList itemList ;
    T_word32 keyMask ;
    T_word16 indexSize ;
    T_doubleLinkList *p_index ;
} T_hash32Struct ;

/****************************************************************************/
/*  Routine:  Hash32Create                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32Create creates a hash table that has all elements referenced    */
/*  by a unique 32 bit value.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Be careful on using too big of an index size.  Too big eats up too    */
/*  much memory.  For most cases, a value of 256 should be sufficient.      */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 indexSize          -- How big to create the initial table.   */
/*                                   Bigger = faster but uses more memory.  */
/*                                   MUST be a power of 2 (16 to 32768)     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_hash32                    -- Created hash32 table.                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    DoubleLinkListCreate                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_hash32 Hash32Create(T_word16 indexSize)
{
    T_hash32Struct *p_hash ;
    T_word16 i ;

    DebugRoutine("Hash32Create") ;
    DebugCheck((indexSize == 16) ||
               (indexSize == 32) ||
               (indexSize == 64) ||
               (indexSize == 128) ||
               (indexSize == 256) ||
               (indexSize == 512) ||
               (indexSize == 1024) ||
               (indexSize == 2048) ||
               (indexSize == 4096) ||
               (indexSize == 8192) ||
               (indexSize == 16384) ||
               (indexSize == 32768)) ;

    /* Allocate the header for the hash table. */
    p_hash = MemAlloc(sizeof(T_hash32Struct)) ;

    /* Make sure we got some memory. */
    DebugCheck(p_hash != NULL) ;
    if (p_hash)  {
        /* Allocate the space needed for the hash index. */
        p_hash->p_index = MemAlloc(sizeof(T_doubleLinkList *) * indexSize) ;
        DebugCheck(p_hash->p_index != NULL) ;
        if (p_hash->p_index)  {
            /* Go through the index and create a link list for each. */
            for (i=0; i<indexSize; i++)  {
                p_hash->p_index[i] = DoubleLinkListCreate() ;
                DebugCheck(p_hash->p_index[i] !=
                    DOUBLE_LINK_LIST_BAD) ;
            }

            /* Create a list of all the items. */
            p_hash->itemList= DoubleLinkListCreate() ;
            DebugCheck(p_hash->itemList != DOUBLE_LINK_LIST_BAD) ;

            /* Store the important size of the table. */
            p_hash->indexSize = indexSize ;
            p_hash->keyMask = indexSize-1 ;

            /* Tag this block as valid. */
            p_hash->tag = HASH32_TAG ;
        }
    }

    DebugEnd() ;

    return ((T_hash32)p_hash) ;
}

/****************************************************************************/
/*  Routine:  Hash32Destroy                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32Destroy destroys the given hash table.                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does NOT destroy each individual unit (the passed in     */
/*  T_void *) since they may not be blocks allocated by MemAlloc.  You      */
/*  MUST do that to clean up the memory (use Hash32GetFirstItem).           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32 hash               -- Hash 32 table to destroy.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListDestroy                                                 */
/*    DoubleLinkListElementGetData                                          */
/*    DoubleLinkListGetFirst                                                */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void Hash32Destroy(T_hash32 hash)
{
    T_word16 i ;
    T_hash32Struct *p_hash ;
    T_hash32Item hashItem ;
    T_doubleLinkListElement element ;
    DebugRoutine("Hash32Destroy") ;

    /* Make sure this is a valid hash table type. */
    DebugCheck(hash != HASH32_BAD) ;
    if (hash != HASH32_BAD)  {
        /* Get a quick pointer to the header. */
        p_hash = (T_hash32Struct *)hash ;

        /* Make sure this is a pointer to a not already dead */
        /* hash table. */
        DebugCheck(p_hash->tag == HASH32_TAG) ;
        if (p_hash->tag == HASH32_TAG)  {
            /* Go through the list of items and remove them all. */
            element = DoubleLinkListGetFirst(p_hash->itemList) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)   {
                hashItem =
                    (T_hash32Item)DoubleLinkListElementGetData(element) ;
                Hash32Remove(hashItem);
                element = DoubleLinkListGetFirst(p_hash->itemList) ;
            }

            /* Now destroy all the lists. */
            for (i=0; i<p_hash->indexSize; i++)
                DoubleLinkListDestroy(p_hash->p_index[i]) ;

            /* Free the list of link lists. */
            MemFree(p_hash->p_index) ;

            /* Now destroy the main list. */
            DoubleLinkListDestroy(p_hash->itemList) ;

            /* Tag this block as gone. */
            p_hash->tag = HASH32_DEAD_TAG ;

            /* Now get rid of the header. */
            MemFree(p_hash) ;

            /* Done. */
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  Hash32Add                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32Add adds an element to the given hash table with the given      */
/*  key.                                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does NOT check to see if the hash key is not being used. */
/*  If you need to do this, make a call to Hash32Find and then either Add   */
/*  or do a Replace.                                                        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32 hash               -- Hash 32 table to add item.             */
/*                                                                          */
/*    T_void *p_data              -- Pointer to data assigned.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_hash32Item                -- Hash entry created.                    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    DoubleLinkListAddElementAtEnd                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_hash32Item Hash32Add(T_hash32 hash, T_word32 key, T_void *p_data)
{
    T_hash32Struct *p_hash ;
    T_hash32ItemStruct *p_item = HASH32_ITEM_BAD ;

    DebugRoutine("Hash32Add") ;

    /* Make sure this is a valid hash table type. */
    DebugCheck(hash != HASH32_BAD) ;
    if (hash != HASH32_BAD)  {
        /* Get a quick pointer to the header. */
        p_hash = (T_hash32Struct *)hash ;

        /* Make sure this is a pointer to a not already dead */
        /* hash table. */
        DebugCheck(p_hash->tag == HASH32_TAG) ;
        if (p_hash->tag == HASH32_TAG)  {
            /* Allocate memory for the item. */
            p_item = (T_hash32ItemStruct *)
                MemAlloc(sizeof(T_hash32ItemStruct)) ;

            /* Make sure we got the memory. */
            DebugCheck(p_item != NULL) ;
            if (p_item)  {
                /* Fill out the item's info. */
                p_item->key = key ;
                p_item->p_data = p_data ;
                p_item->ownerHash = hash ;

                /* Put the item in the full listing. */
                p_item->elementInBigList =
                    DoubleLinkListAddElementAtEnd(
                        p_hash->itemList,
                        (T_void *)p_item) ;

                /* Put the item in the particular hash bucket. */
                p_item->elementInHashList =
                    DoubleLinkListAddElementAtEnd(
                        p_hash->p_index[key & p_hash->keyMask],
                        (T_void *)p_item) ;

                /* Done, mark valid. */
                p_item->tag = HASH32_ITEM_TAG ;
            }
        }
    }

    DebugEnd() ;

    /* Return what we created, or else BAD */
    return ((T_hash32Item)p_item) ;
}

/****************************************************************************/
/*  Routine:  Hash32Find                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32Find searches for an element by its key number.                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32 hash               -- Hash 32 table to search within.        */
/*                                                                          */
/*    T_word32 key                -- Key to search by.                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_hash32Item                -- Item found, else HASH32_ITEM_BAD       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*    DoubleLinkListElementGetNext                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_hash32Item Hash32Find(T_hash32 hash, T_word32 key)
{
    T_hash32Struct *p_hash ;
    T_hash32ItemStruct *p_item = HASH32_ITEM_BAD ;
    T_doubleLinkListElement element ;
    T_doubleLinkList list ;
    T_hash32ItemStruct *p_itemSearch ;

    DebugRoutine("Hash32Find") ;

    /* Make sure this is a valid hash table type. */
    DebugCheck(hash != HASH32_BAD) ;
    if (hash != HASH32_BAD)  {
        /* Get a quick pointer to the header. */
        p_hash = (T_hash32Struct *)hash ;

        /* Make sure this is a pointer to a not already dead */
        /* hash table. */
        DebugCheck(p_hash->tag == HASH32_TAG) ;
        if (p_hash->tag == HASH32_TAG)  {
            /* Find the list that the element is definitely in. */
            list = p_hash->p_index[key & p_hash->keyMask] ;

            /* Search the list for a matching key. */
            element = DoubleLinkListGetFirst(list) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                /* Get the item in this list. */
                p_itemSearch = (T_hash32ItemStruct *)
                    DoubleLinkListElementGetData(element) ;

                /* See if the keys match */
                if (p_itemSearch->key == key)  {
                    /* Grab it and break out of this linear search. */
                    p_item = p_itemSearch ;
                    break ;
                }

                /* Find the next element. */
                element = DoubleLinkListElementGetNext(element) ;
            }
        }
    }

    DebugEnd() ;

    /* Return what we found. */
    return ((T_hash32Item)p_item) ;
}

/****************************************************************************/
/*  Routine:  Hash32Remove                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32Remove takes out a hash element item.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32Item hashItem       -- Item to remove from hash table.        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkList                                                        */
/*    DoubleLinkList                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void Hash32Remove(T_hash32Item hashItem)
{
    T_hash32ItemStruct *p_item ;

    DebugRoutine("Hash32Remove") ;

    /* Check to see if the item is bad. */
    DebugCheck(hashItem != HASH32_ITEM_BAD) ;
    if (hashItem != HASH32_ITEM_BAD)  {
        /* Get a quick pointer. */
        p_item = (T_hash32ItemStruct *)hashItem ;

        /* See if the tag is correct. */
        DebugCheck(p_item->tag == HASH32_ITEM_TAG) ;
        if (p_item->tag == HASH32_ITEM_TAG)  {
            /* Yes, this is a proper item, remove it. */
            DoubleLinkListRemoveElement(p_item->elementInBigList) ;
            DoubleLinkListRemoveElement(p_item->elementInHashList) ;

            /* Mark this object as no longer. */
            p_item->tag = HASH32_ITEM_DEAD_TAG ;

            /* Delete this item. */
            MemFree(p_item) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  Hash32GetFirstItem                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32GetFirstItem returns the first element in the whole hash table. */
/*  The order is in the order items were created.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32 hash               -- Hash table to search in.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_hash32Item                -- First item, else HASH32_ITEM_BAD       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_hash32Item Hash32GetFirstItem(T_hash32 hash)
{
    T_hash32Struct *p_hash ;
    T_hash32ItemStruct *p_item = HASH32_ITEM_BAD ;
    T_doubleLinkListElement element ;

    DebugRoutine("Hash32GetFirstItem") ;

    /* Make sure this is a valid hash table type. */
    DebugCheck(hash != HASH32_BAD) ;
    if (hash != HASH32_BAD)  {
        /* Get a quick pointer to the header. */
        p_hash = (T_hash32Struct *)hash ;

        /* Make sure this is a pointer to a not already dead */
        /* hash table. */
        DebugCheck(p_hash->tag == HASH32_TAG) ;
        if (p_hash->tag == HASH32_TAG)  {
            /* See if there is anything in that list. */
            element = DoubleLinkListGetFirst(p_hash->itemList) ;
            if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                /* Found something.  Get the hash item. */
                p_item = DoubleLinkListElementGetData(element) ;
            }
        }
    }

    DebugEnd() ;

    return ((T_hash32Item)p_item) ;
}

/****************************************************************************/
/*  Routine:  Hash32ItemGetNext                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32ItemGetNext looks up the next item in the hash table as sorted  */
/*  in order of creation.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32Item hashItem       -- Hash item to find next of              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_hash32Item                -- First item, else HASH32_ITEM_BAD       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_hash32Item Hash32ItemGetNext(T_hash32Item hashItem)
{
    T_hash32ItemStruct *p_item ;
    T_hash32ItemStruct *p_next ;
    T_doubleLinkListElement element ;

    DebugRoutine("Hash32ItemGetNext") ;

    /* Check to see if the item is bad. */
    DebugCheck(hashItem != HASH32_ITEM_BAD) ;
    if (hashItem != HASH32_ITEM_BAD)  {
        /* Get a quick pointer. */
        p_item = (T_hash32ItemStruct *)hashItem ;

        /* See if the tag is correct. */
        DebugCheck(p_item->tag == HASH32_ITEM_TAG) ;
        if (p_item->tag == HASH32_ITEM_TAG)  {
            /* Item is good.  Look up the next item. */
            element = DoubleLinkListElementGetNext(p_item->elementInBigList) ;
            if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
                p_next = DoubleLinkListElementGetData(element) ;
            else
                p_next = HASH32_ITEM_BAD ;
        }
    }

    DebugEnd() ;

    return ((T_hash32Item) p_next) ;
}

/****************************************************************************/
/*  Routine:  Hash32ItemGetData                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32ItemGetData looks up the data associated with a hash table      */
/*  entry.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32Item hashItem       -- Hash item to get data                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_void *                    -- Found data pointer                     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *Hash32ItemGetData(T_hash32Item hashItem)
{
    T_hash32ItemStruct *p_item ;
    T_void *p_data ;

    DebugRoutine("Hash32ItemGetData") ;

    /* Check to see if the item is bad. */
    DebugCheck(hashItem != HASH32_ITEM_BAD) ;
    if (hashItem != HASH32_ITEM_BAD)  {
        /* Get a quick pointer. */
        p_item = (T_hash32ItemStruct *)hashItem ;

        /* See if the tag is correct. */
        DebugCheck(p_item->tag == HASH32_ITEM_TAG) ;
        if (p_item->tag == HASH32_ITEM_TAG)  {
            /* Get the data. */
            p_data = p_item->p_data ;
        }
    }

    DebugEnd() ;

    return p_data ;
}

/****************************************************************************/
/*  Routine:  Hash32ItemGetKey                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32ItemGetKey returns the key used to index this hash item.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash32Item hashItem       -- Hash item to get data                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Found key                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/19/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 Hash32ItemGetKey(T_hash32Item hashItem)
{
    T_hash32ItemStruct *p_item ;
    T_word32 key = 0 ;

    DebugRoutine("Hash32ItemGetKey") ;

    /* Check to see if the item is bad. */
    DebugCheck(hashItem != HASH32_ITEM_BAD) ;
    if (hashItem != HASH32_ITEM_BAD)  {
        /* Get a quick pointer. */
        p_item = (T_hash32ItemStruct *)hashItem ;

        /* See if the tag is correct. */
        DebugCheck(p_item->tag == HASH32_ITEM_TAG) ;
        if (p_item->tag == HASH32_ITEM_TAG)  {
            /* Get the data. */
            key = p_item->key ;
        }
    }

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  Hash32GetNumberItems                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Hash32GetNumberItems tells how many items are in the hash table.      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_hash hash                 -- Hash table                             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Number items in table                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetNumberElements                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/24/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 Hash32GetNumberItems(T_hash32 hash)
{
    T_word32 numItems = 0 ;
    T_hash32Struct *p_hash ;

    DebugRoutine("Hash32GetNumberItems") ;

    /* Make sure this is a valid hash table type. */
    DebugCheck(hash != HASH32_BAD) ;
    if (hash != HASH32_BAD)  {
        /* Get a quick pointer to the header. */
        p_hash = (T_hash32Struct *)hash ;

        /* Make sure this is a pointer to a not already dead */
        /* hash table. */
        DebugCheck(p_hash->tag == HASH32_TAG) ;
        if (p_hash->tag == HASH32_TAG)  {
            numItems = DoubleLinkListGetNumberElements(
                           p_hash->itemList) ;
        }
    }

    DebugEnd() ;

    return numItems ;
}

#if 0
/* The following is a routine that you can use to confirm that */
/* the hash table is functionaly correct.  Call it a 'unit tester'. */
T_void TestHash(T_void)
{
    T_hash32 hashA ;
    T_hash32 hashB ;
    T_hash32 hashItem ;
    T_hash32 hashItemNext ;

    DebugRoutine("TestHash") ;

puts("Creating") ; fflush(stdout) ;
    hashA = Hash32Create(256) ;
    hashB = Hash32Create(512) ;
    Hash32Destroy(hashB) ;
    hashB = Hash32Create(512) ;

puts("Adding items") ; fflush(stdout) ;
    Hash32Add(hashA, 522, (T_void *)522) ;
    Hash32Add(hashA, 512, (T_void *)512) ;
    Hash32Add(hashA, 256, (T_void *)256) ;
    Hash32Add(hashA, 266, (T_void *)266) ;
    Hash32Add(hashA, 10, (T_void *)10) ;

    printf("1) In hashA:\n") ;  fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            hashItem = Hash32ItemGetNext(hashItem) ;
        } while (hashItem) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    hashItem = Hash32Find(hashA, 256) ;
    printf("R %p\n", hashItem) ;  fflush(stdout) ;
    Hash32Remove(hashItem) ;

    printf("2) In hashA:\n") ;   fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            hashItem = Hash32ItemGetNext(hashItem) ;
        } while (hashItem) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    hashItem = Hash32Find(hashA, 10) ;
    printf("R %p\n", hashItem) ;  fflush(stdout) ;
    Hash32Remove(hashItem) ;

    printf("3) In hashA:\n") ;  fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            hashItem = Hash32ItemGetNext(hashItem) ;
        } while (hashItem) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    hashItem = Hash32Find(hashA, 522) ;
    printf("R %p\n", hashItem) ;  fflush(stdout) ;
    Hash32Remove(hashItem) ;

    printf("4) In hashA:\n") ;  fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            hashItem = Hash32ItemGetNext(hashItem) ;
        } while (hashItem) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    hashItem = Hash32Add(hashA, 257, (T_void *)257) ;
    printf("Added %p.\n", hashItem) ;  fflush(stdout) ;

    printf("5) In hashA:\n") ;  fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            hashItemNext = Hash32ItemGetNext(hashItem) ;
            Hash32Remove(hashItem) ;
            hashItem = hashItemNext;
        } while (hashItem) ;  fflush(stdout) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    printf("6) In hashA:\n") ;  fflush(stdout) ;
    if (hashItem = Hash32GetFirstItem(hashA))  {
        do {
            printf("%p %p\n", hashItem, Hash32ItemGetData(hashItem)) ;
            fflush(stdout) ;
            Hash32Remove(hashItem) ;
            hashItem = Hash32ItemGetNext(hashItem) ;
        } while (hashItem) ;
    }
    printf("---\n") ;  fflush(stdout) ;

    Hash32Destroy(hashB) ;
    Hash32Destroy(hashA) ;

    DebugEnd() ;
}
#endif

/****************************************************************************/
/*    END OF FILE:  HASH32.C                                                */
/****************************************************************************/
