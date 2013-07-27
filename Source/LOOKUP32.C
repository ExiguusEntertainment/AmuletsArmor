/****************************************************************************/
/*    FILE:  LOOKUP32.C                                                     */
/****************************************************************************/
/*                                                                          */
/*    Lookup32 is a module that allows a way to store large conversion      */
/*  tables that take a 32 bit number and converts it to a different 32 bit  */
/*  number.  This is useful when looking up account records and such        */
/*  and you have a 32 bit value for the account number, but need to know    */
/*  what the corresponding record number is.                                */
/*                                                                          */
/*  NOTE: Requires the FlatFile module.                                     */
/*                                                                          */
/****************************************************************************/
#include "FILE.H"
#include "LOOKUP32.H"
#include "MEMORY.H"

#define ENTRIES_PER_TABLE 256

/* All entries in the table are either index to other tables, or */
/* some unknown 32 bit value. */
typedef union {
    T_flatFileIndex index ;
    T_word32 value ;
} T_lookupIndexOrValue ;

/* Table structure -- really just an array of entries. */
typedef struct {
    T_lookupIndexOrValue entryList[ENTRIES_PER_TABLE] ;
} T_lookupTable ;

/* The header is used to tell what is the first lookup table to find. */
/* This is always the start of the search. */
typedef struct {
    T_flatFileIndex first ;
} T_lookupHeader ;

/* Internal prototypes: */
static T_flatFileIndex IAllocateIndexTable(T_flatFile flatfile) ;

static T_lookupIndexOrValue IGetLookup(
                                T_flatFile flatfile,
                                T_flatFileIndex index,
                                T_byte8 pos) ;

static T_void IPutLookup(
                  T_flatFile flatfile,
                  T_flatFileIndex index,
                  T_byte8 pos,
                  T_lookupIndexOrValue indexOrValue) ;

/****************************************************************************/
/*  Routine:  Lookup32OpenOrCreate                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Lookup32OpenOrCreate opens up a 32 bit lookup table file at the given */
/*  filename and returns the handle (else LOOKUP32_BAD).                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename          -- Lookup 32 bit table file name         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_lookup32                   -- Handle to 32 bit lookup table         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_lookup32 Lookup32OpenOrCreate(T_byte8 *filename)
{
    T_flatFile lookupFile ;
    E_Boolean isCreated ;
    T_lookupHeader *p_header ;

    DebugRoutine("Lookup32OpenOrCreate") ;

    /* Check to see if this is being created. */
    isCreated = FlatFileExist(filename) ? FALSE : TRUE ;

    /* In either case, go ahead and open/create the file. */
    lookupFile = FlatFileOpenOrCreate(
                     filename,
                     sizeof(T_lookupTable)) ;
    DebugCheck(lookupFile != FLATFILE_BAD) ;

    /* Make sure we got something. */
    if (lookupFile != FLATFILE_BAD)  {
        /* If we got something AND we are creating */
        /* the lookup table, go ahead and initilize it. */

        if (isCreated)  {
            /* Get access to the header. */
            p_header = FlatFileGetOptionalHeader(lookupFile) ;

            /* Allocate the first block for the header. */
            p_header->first = IAllocateIndexTable(lookupFile) ;

            /* Make sure to store the header now. */
            FlatFileMarkDirty(lookupFile) ;
            FlatFileRefresh(lookupFile) ;
        }
    }

    DebugEnd() ;

    /* Return the handle to the flatfile, which is what */
    /* we currently use. */
    return ((T_lookup32)lookupFile) ;
}


/****************************************************************************/
/*  Routine:  Lookup32Close                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Lookup32Close closes out a 32 bit lookup table.                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_lookup32 lookup            -- Lookup 32 bit table handle            */
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
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void Lookup32Close(T_lookup32 lookup)
{
    DebugRoutine("Lookup32Close") ;

    FlatFileClose(lookup) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  Lookup32Put                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Lookup32Put sets up a index->value conversion in the given 32 bit     */
/*  lookup table.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_lookup32 lookup            -- Lookup table being affected           */
/*                                                                          */
/*    T_word32 index               -- Index to set up conversion for        */
/*                                                                          */
/*    T_word32 value               -- Value to convert to                   */
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
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void Lookup32Put(T_lookup32 lookup, T_word32 index, T_word32 value)
{
    T_byte8 level1, level2, level3, level4 ;
    T_lookupIndexOrValue indexOrValue ;
    T_lookupIndexOrValue nextIndexOrValue ;
    T_lookupHeader *p_header ;
    T_lookupIndexOrValue newIndex ;
    T_lookupIndexOrValue setValue ;

    DebugRoutine("Lookup32Put") ;

    /* Get the four parts that make up the 32 bit lookup. */
    level1 = ((index>>24)&0xFF) ;
    level2 = ((index>>16)&0xFF) ;
    level3 = ((index>> 8)&0xFF) ;
    level4 = ((index>> 0)&0xFF) ;

    /* Get access to the header. */
    p_header = FlatFileGetOptionalHeader(lookup) ;
    DebugCheck(p_header != NULL) ;

    indexOrValue.index = p_header->first ;

    /* Translate to get level 1 index. */
    nextIndexOrValue = IGetLookup(
                       lookup,
                       indexOrValue.index,
                       level1) ;

    /* Make sure that there is a sub index. */
    if (nextIndexOrValue.index == FLATFILE_INDEX_BAD)  {
        newIndex.index = IAllocateIndexTable(lookup) ;
        IPutLookup(lookup, indexOrValue.index, level1, newIndex) ;
        indexOrValue = newIndex ;
    } else {
        indexOrValue = nextIndexOrValue ;
    }

    /* Translate to the next level -- level 2. */
    nextIndexOrValue = IGetLookup(
                       lookup,
                       indexOrValue.index,
                       level2) ;

    /* Make sure that there is a sub index. */
    if (nextIndexOrValue.index == FLATFILE_INDEX_BAD)  {
        newIndex.index = IAllocateIndexTable(lookup) ;
        IPutLookup(lookup, indexOrValue.index, level2, newIndex) ;
        indexOrValue = newIndex ;
    } else {
        indexOrValue = nextIndexOrValue ;
    }

    /* Translate to the next level -- level 3. */
    nextIndexOrValue = IGetLookup(
                       lookup,
                       indexOrValue.index,
                       level3) ;

    /* Make sure that there is a sub index. */
    if (nextIndexOrValue.index == FLATFILE_INDEX_BAD)  {
        newIndex.index = IAllocateIndexTable(lookup) ;
        IPutLookup(lookup, indexOrValue.index, level3, newIndex) ;
        indexOrValue = newIndex ;
    } else {
        indexOrValue = nextIndexOrValue ;
    }

    /* Finally, store the value we have. */
    setValue.value = value ;
    IPutLookup(lookup, indexOrValue.index, level4, setValue) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  Lookup32Get                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Lookup32Get gets a value for a corresponding index in a 32 bit        */
/*  lookup table.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_lookup32 lookup            -- Lookup table being searched           */
/*                                                                          */
/*    T_word32 index               -- Index to find                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32 value               -- Value found, else 0                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 Lookup32Get(T_lookup32 lookup, T_word32 index)
{
    T_byte8 level1, level2, level3, level4 ;
    T_word32 found = 0 ;
    T_lookupHeader *p_header ;
    T_lookupIndexOrValue indexOrValue ;

    DebugRoutine("Lookup32Get") ;

    /* Get the four parts that make up the 32 bit lookup. */
    level1 = ((index>>24)&0xFF) ;
    level2 = ((index>>16)&0xFF) ;
    level3 = ((index>> 8)&0xFF) ;
    level4 = ((index>> 0)&0xFF) ;

    /* Get access to the header. */
    p_header = FlatFileGetOptionalHeader(lookup) ;
    DebugCheck(p_header != NULL) ;

    indexOrValue.index = p_header->first ;

    /* Translate to get level 1 index. */
    indexOrValue = IGetLookup(
                       lookup,
                       indexOrValue.index,
                       level1) ;
    if (indexOrValue.index != FLATFILE_INDEX_BAD)  {
        /* Translate to the next level -- level 2. */
        indexOrValue = IGetLookup(
                           lookup,
                           indexOrValue.index,
                           level2) ;

        if (indexOrValue.index != FLATFILE_INDEX_BAD)  {
            /* Translate to the next level -- level 3. */
            indexOrValue = IGetLookup(
                               lookup,
                               indexOrValue.index,
                               level3) ;

            if (indexOrValue.index != FLATFILE_INDEX_BAD)  {
                /* Translate to the next level -- level 4. */
                indexOrValue = IGetLookup(
                                   lookup,
                                   indexOrValue.index,
                                   level4) ;

                /* Get the found value. */
                found = indexOrValue.value ;
            }
        }
    }

    DebugEnd() ;

    return found ;
}


/****************************************************************************/
/*  Routine:  IAllocateIndexTable                     * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IAllocateIndexTable creates anotehr T_lookupTable item in the         */
/*  flatfile database, initializes the data to zero, and returns the        */
/*  index.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatfile          -- Flatfile to create table              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_flatFileIndex              -- index of created member               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_flatFileIndex IAllocateIndexTable(T_flatFile flatfile)
{
    T_flatFileIndex index ;
    T_lookupTable *p_table ;

    DebugRoutine("IAllocateIndexTable") ;

    /* Create the record. */
    index = FlatFileCreateRecord(flatfile) ;

    /* Make sure we got something. */
    DebugCheck(index != FLATFILE_INDEX_BAD) ;
    if (index != FLATFILE_INDEX_BAD)  {
        /* initialize the table. */
        p_table = (T_lookupTable *)FlatFileGetRecord(
                                       flatfile,
                                       index) ;

        /* clear the table. */
        memset(p_table, 0, sizeof(T_lookupTable)) ;

        /* Save the record back into the file. */
        FlatFilePutRecord(flatfile, index, p_table) ;

        /* Free the locked block from memory. */
        MemFree(p_table) ;
    }

    DebugEnd() ;

    return index ;
}

/****************************************************************************/
/*  Routine:  IGetLookup                              * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetLookup searches for the given table and gets the value of the     */
/*  given index position.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatfile          -- Flatfile containing table             */
/*                                                                          */
/*    T_flatFileIndex index        -- Index to table                        */
/*                                                                          */
/*    T_byte8 pos                  -- Position within table                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_lookupIndexOrValue         -- found value                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_lookupIndexOrValue IGetLookup(
                                T_flatFile flatfile,
                                T_flatFileIndex index,
                                T_byte8 pos)
{
    T_lookupTable *p_table ;
    T_lookupIndexOrValue found ;

    DebugRoutine("IGetLookup") ;

//printf("IGetLookup: index=%08lX, pos=%d\n", index, pos) ;
    /* Zero out the found value/index. */
    found.value = 0 ;

    /* Get access to the given index. */
    p_table = (T_lookupTable *)FlatFileGetRecord(
                                   flatfile,
                                   index) ;
    DebugCheck(p_table != NULL) ;

    /* Check that we have level access. */
    if (p_table)  {
        /* Lookup that position. */
        found = p_table->entryList[pos] ;

        /* Free the table. */
        MemFree(p_table) ;
    }

    DebugEnd() ;

    return found ;
}

/****************************************************************************/
/*  Routine:  IPutLookup                              * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IPutLookup searches for the given table and sets the value of the     */
/*  given index position.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatfile          -- Flatfile containing table             */
/*                                                                          */
/*    T_flatFileIndex index        -- Index to table                        */
/*                                                                          */
/*    T_byte8 pos                  -- Position within table                 */
/*                                                                          */
/*    T_lookupIndexOrValue         -- Value to become                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IPutLookup(
                  T_flatFile flatfile,
                  T_flatFileIndex index,
                  T_byte8 pos,
                  T_lookupIndexOrValue indexOrValue)
{
    T_lookupTable *p_table ;

    DebugRoutine("IPutLookup") ;
    DebugCheck(index != FLATFILE_INDEX_BAD) ;

    /* Get access to the given index. */
    p_table = (T_lookupTable *)FlatFileGetRecord(
                                   flatfile,
                                   index) ;
    DebugCheck(p_table != NULL) ;

    /* Check that we have level access. */
    if (p_table)  {
        /* Lookup that position and store the value. */
        p_table->entryList[pos] = indexOrValue;

        /* Store the record back out. */
        FlatFilePutRecord(
            flatfile,
            index,
            p_table) ;

        /* Free the table. */
        MemFree(p_table) ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*    END OF FILE: LOOKUP32.C                                               */
/****************************************************************************/

