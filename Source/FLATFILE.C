/****************************************************************************/
/*    FILE:  FLATFILE.C                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    The FlatFile Module is used for keeping record of many items of the   */
/*  same size in one file.  Typically each entry is one structure of a      */
/*  specific size.  Additional information can be stored in the header to   */
/*  create your own header for the data.                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/
#include "FILE.H"
#include "FLATFILE.H"
#include "MEMORY.H"

#define FLATFILE_TAG                   (*((T_word32 *)"FlFi"))
#define FLATFILE_DEAD_TAG              (*((T_word32 *)"Dflf"))

#define FLATFILE_CURRENT_VERSION       1001     /* Version 1.001 */

#define FLATFILE_NO_FREE_ENTRIES       0xFFFFFFFFL

/* I hope 1 billion indexes is enough! */
#define FLATFILE_MAX_INDEX             0x3FFFFFFFL

/* This is or'd to a deleted index to put a small two bit check */
/* on deleted indexes. */
#define FLATFILE_DELETED_INDEX         0xC0000000L

typedef struct {
    T_word32 tag ;                     /* Characters 'FlFi' */
    T_word32 version ;                 /* Should be 1001 */
    T_word32 firstFreeEntry ;          /* Index to first free slot. */
    T_word32 lastEntry ;               /* Largest index number. */
    T_word32 numberEntriesAllocated ;  /* Number allocated records. */
    T_word32 numberEntriesFree ;       /* Number entries freed. */ 
    T_word32 file ;                    /* Internal file handle */    
    T_word32 entrySize ;               /* Size of each entry. */
    T_word32 headerSize ;              /* Size of this header. */
    T_byte8 isDirty ;                  /* Flag saying if there is a change. */
    T_byte8 reserved[3] ;
    T_byte8 reserved2[24] ;
                                       /* Optional header block */
    T_byte8 optionalHeader[FLATFILE_OPTIONAL_HEADER_MAX_SIZE] ;           
} T_flatFileHeader ;

/* Internal prototype: */
static T_void IFlatFileSeekRecord(
                  T_flatFileHeader *p_header, 
                  T_word32 index) ;

/****************************************************************************/
/*  Routine:  FlatFileOpenOrCreate                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileOpenOrCreate opens up or creates a flatfile depending on      */
/*  if the file already exists (and that it is a flatfile database).        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    All entries are at least 4 bytes big.                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *filename           -- Filename with path to flat file        */
/*                                                                          */
/*    T_word32 entrySize          -- Size of each record, should match      */
/*                                   the size of the records if opening.    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_flatfile                  -- Created or opened flat file.  If       */
/*                                   FLATFILE_BAD, then you are trying      */
/*                                   to open a non-flatfile.                */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileExist                                                             */
/*    FileOpen                                                              */
/*    FileClose                                                             */
/*    FileSeek                                                              */
/*    MemAlloc                                                              */
/*    MemFree                                                               */
/*    FlatFileRefresh                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_flatFile FlatFileOpenOrCreate(
               T_byte8 *filename, 
               T_word32 entrySize)
{
    T_file file ;
    T_flatFileHeader *p_header = NULL ;
    E_Boolean isValid = TRUE ;

    DebugRoutine("FlatFileOpenOrCreate") ;

    /* Make sure that each record is at least four bytes. */
    if (entrySize < 4)
        entrySize = 4 ;

    /* Are we trying to open or create the flatfile? */
    if (FileExist(filename))  {
        /* Open an existing file. */

        /* First try to open it normal like. */
        file = FileOpen(filename, FILE_MODE_READ_WRITE) ;
        DebugCheck(file != FILE_BAD) ;
        if (file)  {
            /* Allocate memory for the header. */
            p_header = MemAlloc(sizeof(*p_header)) ;
            DebugCheck(p_header != NULL) ;
            if (p_header)  {
                /* Read in the header. */
                FileSeek(file, 0) ;
                FileRead(file, p_header, sizeof(*p_header)) ;
                p_header->file = file ;

                /* Is there the correct tag and version number? */
                DebugCheck(p_header->tag == FLATFILE_TAG) ;
                if (p_header->tag != FLATFILE_TAG)
                    isValid = FALSE ;

                DebugCheck(p_header->version == FLATFILE_CURRENT_VERSION) ;
                if (p_header->version != FLATFILE_CURRENT_VERSION)
                    isValid = FALSE ;
    
                /* Make sure we have the same sized entry size. */
                DebugCheck(p_header->entrySize == entrySize) ;
                if (p_header->entrySize != entrySize)
                    isValid = FALSE ;

                /* If any of the above checks failed, we have */
                /* to clean up. */                
                if (!isValid) {
                    /* Free the header. */
                    p_header->tag = FLATFILE_DEAD_TAG ;
                    MemFree(p_header) ;
                    p_header = NULL ;

                    /* Close the file. */
                    FileClose(file) ;
                } else {
                    /* Ok, the file is opened now and correct. */
                    /* Note that no changes have been made. */
                    p_header->isDirty = FALSE ;
                }
            }
        }
    } else {
        /* Create a new file. */

        /* Open/create the file to hold the records. */
        file = FileOpen(filename, FILE_MODE_READ_WRITE) ;
        DebugCheck(file != FILE_BAD) ;
        if (file)  {
            /* Allocate a new header. */
            p_header = MemAlloc(sizeof(*p_header)) ;
            DebugCheck(p_header != NULL) ;
            if (p_header)  {
                /* Initialize the header with data. */
                p_header->tag = FLATFILE_TAG ;
                p_header->version = FLATFILE_CURRENT_VERSION ;
                p_header->firstFreeEntry = FLATFILE_NO_FREE_ENTRIES ;
                p_header->lastEntry = 1 ;
                p_header->numberEntriesAllocated = 0 ;
                p_header->numberEntriesFree = 0 ;
                p_header->entrySize = entrySize ;
                p_header->file = file ;
                p_header->isDirty = TRUE ;
                p_header->headerSize = sizeof(*p_header) ;

                /* Save out the header info. */
                FlatFileRefresh(((T_flatFile)p_header)) ;
            }
        }
    }

    DebugEnd() ;

    return ((T_flatFile)p_header) ;
}

/****************************************************************************/
/*  Routine:  FlatFileClose                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileClose closes out a previously opened flat file.  Any records  */
/*  hanging around or header is flushed out and closed up.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file that is being closed         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FlatFileRefresh                                                       */
/*    FileClose                                                             */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FlatFileClose(T_flatFile flatFile)
{
    T_flatFileHeader *p_header ;
    DebugRoutine("FlatFileClose") ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Write out the header (if there has been any change). */
        FlatFileRefresh(flatFile) ;

        /* Close the file. */
        FileClose(p_header->file) ;

        /* Mark the header (in memory) as destroyed. */
        p_header->tag = FLATFILE_DEAD_TAG ;

        /* Free the header memory. */
        MemFree(p_header) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  FlatFileCreateRecord                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileCreateRecord sets up a new record for use and returns the     */
/*  index.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The initial record created contains random junk -- which might        */
/*  look legal since it may be a previously deleted record.                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file that is being closed         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    IFlatFileSeekRecord                                                   */
/*    FileRead                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_flatFileIndex FlatFileCreateRecord(T_flatFile flatFile)
{
    T_flatFileHeader *p_header ;
    T_flatFileIndex index = FLATFILE_INDEX_BAD ;

    DebugRoutine("FlatFileCreateRecord") ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Ok, header is good.  Now, do we have any free records? */
        if (p_header->firstFreeEntry == FLATFILE_NO_FREE_ENTRIES)  {
             /* No free entries exist, we need another. */
             index = p_header->lastEntry++ ;
             p_header->numberEntriesAllocated++ ;

             /* Mark the header as changed. */
             p_header->isDirty = TRUE ;
        } else {
             /* Pop off a free entry from the front of the list. */
             index = p_header->firstFreeEntry ;
#ifndef NDEBUG
             if ((index & FLATFILE_DELETED_INDEX) != FLATFILE_DELETED_INDEX)
                 printf("index: %08X\n", index) ;
#endif
             DebugCheck(
                 (index & FLATFILE_DELETED_INDEX) ==
                 FLATFILE_DELETED_INDEX) ;

             index &= (~FLATFILE_DELETED_INDEX) ;

             /* Seek where the record is. */
             IFlatFileSeekRecord(
                 p_header,
                 (index & (~FLATFILE_DELETED_INDEX))) ;

             /* Read in the link to the next free record. */
             FileRead(
                 p_header->file, 
                 &p_header->firstFreeEntry, 
                 sizeof(T_word32)) ;

             /* Make sure that is valid. */
             DebugCheck(
                 (p_header->firstFreeEntry & FLATFILE_DELETED_INDEX) ==
                 FLATFILE_DELETED_INDEX) ;

             /* Now we have remove an index record from the dead */
             /* list.  The value in 'index' is valid. */
             p_header->numberEntriesAllocated++ ;
             p_header->numberEntriesFree-- ;

             /* Mark the header as changed. */
             p_header->isDirty = TRUE ;
        }
    }

    DebugEnd() ;

    return index ;
}

/****************************************************************************/
/*  Routine:  FlatFileGetRecord                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileGetRecord retrieves a copy of a record that is in the         */
/*  flatfile.                                                               */
/*    When you are done with the returned record, just do a MemFree.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    It is up to the users of the FlatFile Module to decide on how they    */
/*  want to handle file/record locking.  The FlatFile Module does *nothing* */
/*  to make sure that records are handled correctly.                        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to read record from.         */
/*                                                                          */
/*    T_flatFileIndex index       -- Index to look up.                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_void *                    -- Pointer to memory of copy of record.   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    IFlatFileSeekRecord                                                   */
/*    FileRead                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *FlatFileGetRecord(
            T_flatFile flatFile, 
            T_flatFileIndex index)
{
    T_flatFileHeader *p_header ;
    T_void *p_record = NULL ;

    DebugRoutine("FlatFileGetRecord") ;
    DebugCheck(flatFile != FLATFILE_BAD) ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
#ifndef NDEBUG
        if (index >= p_header->lastEntry)  {
            printf("index = %ld, p_header->lastEntry = %ld\n",
                index,
                p_header->lastEntry) ;
        }
#endif
        DebugCheck(index < p_header->lastEntry) ;
        if (index < p_header->lastEntry)  {
            /* Allocate memory to hold the found record. */
            p_record = MemAlloc(p_header->entrySize) ;
            DebugCheck(p_record != NULL) ;

            /* Make sure we got the memory. */
            if (p_record)  {
                /* Find the record. */
                IFlatFileSeekRecord(p_header, index) ;

                /* Read in the record. */
                FileRead(p_header->file, p_record, p_header->entrySize) ;

                /* Got the record, just pass it back. */
            }
        }
    }

    DebugEnd() ;

    return p_record ;
}

/****************************************************************************/
/*  Routine:  FlatFilePutRecord                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFilePutRecord stores the given data over a pre-existing record.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does not check to see if the index is correct.           */
/*  Overwriting in the wrong slot is deadly.  Be sure to keep the index     */
/*  with the previously read block.                                         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to store record to           */
/*                                                                          */
/*    T_flatFileIndex index       -- Index of record to store               */
/*                                                                          */
/*    T_void *p_record            -- Where record is to store               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    IFlatFileSeekRecord                                                   */
/*    FileWrite                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FlatFilePutRecord(
           T_flatFile flatFile, 
           T_flatFileIndex index, 
           T_void *p_record)
{
    T_flatFileHeader *p_header ;

    DebugRoutine("FlatFilePutRecord") ;
    DebugCheck(flatFile != FLATFILE_BAD) ;
    DebugCheck(p_record != NULL) ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Make sure the index is correct. */
        DebugCheck(index < p_header->lastEntry) ;
        if (index < p_header->lastEntry)  {
            /* Find the record. */
            IFlatFileSeekRecord(p_header, index) ;

            /* Write out the record. */
            FileWrite(p_header->file, p_record, p_header->entrySize) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  FlatFileDeleteRecord                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileDeleteRecord deletes the given record and puts it in the      */
/*  free list.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does not check to see if the index is correct.           */
/*  Deleting the wrong record may cause problems (especially if it is       */
/*  already free).                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to delete record             */
/*                                                                          */
/*    T_flatFileIndex index       -- Index of record to delete              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFlatFileSeekRecord                                                   */
/*    FileWrite                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FlatFileDeleteRecord(
           T_flatFile flatFile,
           T_flatFileIndex index)
{
    T_word32 nextIndex ;
    T_flatFileHeader *p_header ;

    DebugRoutine("FlatFileDeleteRecord") ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Make sure the index is correct. */
        DebugCheck(index < p_header->lastEntry) ;
        if (index < p_header->lastEntry)  {
            /* Find the record. */
            IFlatFileSeekRecord(p_header, index) ;

            /* Store at the beginning the link to the next */
            /* record. */
            nextIndex = p_header->firstFreeEntry ;
            nextIndex |= FLATFILE_DELETED_INDEX ;
            FileWrite(p_header->file, &nextIndex, sizeof(nextIndex)) ;

            /* Make this index the next free index. */
            p_header->firstFreeEntry = index | FLATFILE_DELETED_INDEX ;
            p_header->numberEntriesAllocated-- ;
            p_header->numberEntriesFree++ ;

            p_header->isDirty = TRUE ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  FlatFileRefresh                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileRefresh writes out the header of a flatfile if it has         */
/*  changed.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to refresh                   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileSeek                                                              */
/*    FileWrite                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FlatFileRefresh(T_flatFile flatFile)
{
    T_flatFileHeader *p_header ;

    DebugRoutine("FlatFileRefresh") ;
    DebugCheck(flatFile != FLATFILE_BAD) ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Check if there has been a change to the header. */
        if (p_header->isDirty)  {
            FileSeek(p_header->file, 0) ;
            FileWrite(p_header->file, p_header, sizeof(*p_header)) ;
            p_header->isDirty = FALSE ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFlatFileSeekRecord                     * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFlatFileSeekRecord positions the file pointer over the given         */
/*  record index.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFileHeader *p_header  -- Header to seek position into           */
/*                                                                          */
/*    T_word32 index              -- Index to seek                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileSeek                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IFlatFileSeekRecord(
                  T_flatFileHeader *p_header, 
                  T_word32 index)
{
    DebugRoutine("IFlatFileSeekRecord") ;
    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    DebugCheck(p_header->file != FILE_BAD) ;
    DebugCheck(index <= FLATFILE_MAX_INDEX) ;
    DebugCheck(index < p_header->lastEntry) ;

    FileSeek(
        p_header->file, 
        sizeof(T_flatFileHeader) + 
            index * p_header->entrySize) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  FlatFileGetOptionalHeader                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileGetOptionalHeader returns a pointer to a portion of the       */
/*  header that you can change.  Note that this area is restricted to       */
/*  only FLATFILE_OPTIONAL_HEADER_MAX_SIZE bytes.                           */
/*    Calls to FlatFileMarkDirty and then FlatFileRefresh will save this    */
/*  data.  Failure to do so can lead to loss of data.                       */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Writing outside the given pointer will cause unexpected results.      */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to get optional data         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *FlatFileGetOptionalHeader(T_flatFile flatFile)
{
    T_void *p_optionalHeader ;
    T_flatFileHeader *p_header ;

    DebugRoutine("FlatFileGetOptionalHeader") ;
    DebugCheck(flatFile != FLATFILE_BAD) ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;

    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Just return a pointer to the work area. */
        p_optionalHeader = p_header->optionalHeader ;
    }

    DebugEnd() ;

    return p_optionalHeader ;
}

/****************************************************************************/
/*  Routine:  FlatFileMarkDirty                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FlatFileMarkDirty is called to say that some part of the flatfile     */
/*  header has changed and to make sure that it is saved on the next        */
/*  FlatFileRefresh or FlatFileClose.                                       */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_flatFile flatFile         -- Flat file to mark dirty                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FlatFileMarkDirty(T_flatFile flatFile)
{
    T_flatFileHeader *p_header ;

    DebugRoutine("FlatFileMarkDirty") ;
    DebugCheck(flatFile != FLATFILE_BAD) ;

    /* Get a pointer to the flat file header. */
    p_header = (T_flatFileHeader *)flatFile ;

    DebugCheck(p_header->tag == FLATFILE_TAG) ;
    if (p_header->tag == FLATFILE_TAG)   {
        /* Mark the header as dirty. */
        p_header->isDirty = TRUE ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE: FLATFILE.H                                               */
/****************************************************************************/

