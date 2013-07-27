/****************************************************************************/
/*    FILE:  VM.C                                                           */
/****************************************************************************/
#include "FILE.H"
#include "MEMORY.H"
#include "VM.H"

#define VM_BLOCK_ID ((((T_word32)'V')<<0) | \
                    (((T_word32)'m')<<8) | \
                    (((T_word32)'B')<<16))

#define VM_ENTRY_GROW_SIZE 2000
#define VM_SHIFT_BLOCK_SIZE 4000

typedef T_byte8 F_vmFlags ;

#define VM_FLAG_IN_MEMORY    0x80
#define VM_FLAG_DIRTY        0x40
#define VM_SEMAPHORE_READ    0x20
#define VM_SEMAPHORE_WRITE   0x10

typedef struct {
    T_word32 numEntries ;
    T_word32 numAllocatedEntries ;
    T_word32 numFreeEntries ;
    T_word32 numTotalEntries ;
    T_word32 firstFree ;              /* First index. */
    T_word32 lastByte ;               /* Last byte position. */
} T_vmHeader ;

typedef struct {
    T_word16 refCount ;
    F_vmFlags vmFlags ;
    T_byte8 lockCount ;
    T_word32 memoryOrNext ;
    T_word32 fileOffset ;
    T_word32 size ;
} T_vmEntry ;

typedef struct {
    T_vmHeader header ;
    T_vmEntry entries[] ;
} T_vmIndex ;

typedef struct {
    T_byte8 filenameIndex[80] ;
    T_file fileIndex ;
    T_byte8 filenameRecords[80] ;
    T_file fileRecords ;
    T_vmIndex *p_index ;
    T_word32 lockedRecords ;
    T_word32 numEntries ;
    E_Boolean opened ;
} T_vmFileInfo ;

typedef struct {
#ifndef NDEBUG
    T_word32 vmBlockId ;
#endif
    T_vmFile file ;
    T_vmBlock block ;
    T_vmEntry *p_entry ;
    T_byte8 data[] ;
} T_vmDataBlock ;

/* Internal prototypes: */
T_void IVMCreate(T_vmFileInfo *p_fileInfo) ;

T_void IVMUpdate(T_vmFileInfo *p_fileInfo) ;

T_vmBlock IVMFindFreeBlockOfSize(T_vmFile file, T_word32 size) ;

T_vmBlock IVMCreateBlockAtEnd(T_vmFile file, T_word32 size) ;

T_void IVMFree(T_vmFile file, T_vmBlock block) ;

T_void IVMRelease(T_vmFile file, T_vmBlock block) ;

T_vmBlock IVMFindLowestBlock(T_vmFile file, T_word32 place) ;

T_void IVMSaveIfDirty(T_vmFileInfo *p_fileInfo, T_vmEntry *p_entry) ;

T_void IVMSaveIndex(T_vmFileInfo *p_fileInfo) ;

T_word32 IVMAppendEntry(T_vmFile file) ;

T_void IVMShiftBlockDown(T_vmFile file, T_vmBlock block, T_word32 *place) ;

T_void IVMTruncate(T_vmFile file, T_word32 place) ;

/****************************************************************************/
/*  Routine:  VMOpen                                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMOpen opens a file for VM file access.  If the file does not exist,  */
/*  it is created.  Minimal setup is used when first created.               */
/*  Pass to this routine the filename WITHOUT an extension.  A .idx postfix */
/*  is added for the index and A .rec postfix is added for the records      */
/*  file.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- File to open for VM access.            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_vmFile file               -- File that was opened.                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    strcpy                                                                */
/*    strcat                                                                */
/*    FileOpen                                                              */
/*    FileLoad                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_vmFile VMOpen(T_byte8 *p_filename)
{
    T_vmFileInfo *p_fileInfo ;
    T_word32 indexSize ;

    DebugRoutine("VMOpen") ;
    DebugCheck(p_filename != NULL) ;
    DebugCheck(strlen(p_filename) > 0) ;

    /* Allocate a file structure. */
    p_fileInfo = (T_vmFileInfo *)MemAlloc(sizeof(T_vmFileInfo)) ;

    /* Create the two filenames we are going to use. */
    strcpy(p_fileInfo->filenameIndex, p_filename) ;
    strcpy(p_fileInfo->filenameRecords, p_filename) ;
    strcat(p_fileInfo->filenameIndex, ".idx") ;
    strcat(p_fileInfo->filenameRecords, ".rec") ;

    /* Try opening the file for reading (and writing). */
    p_fileInfo->fileIndex = FileOpen(
                                p_fileInfo->filenameIndex,
                                FILE_MODE_READ) ;

    /* Were we able to actually get the index file? */
    if (p_fileInfo->fileIndex == FILE_BAD)  {
        /* Does not exist.  Create the files. */
        IVMCreate(p_fileInfo) ;
    } else {
        FileClose(p_fileInfo->fileIndex) ;
    }

    p_fileInfo->fileIndex = FileOpen(
                                p_fileInfo->filenameIndex,
                                FILE_MODE_READ_WRITE) ;
    DebugCheck(p_fileInfo->fileIndex != FILE_BAD) ;

    /* At this point we should have a valid index file. */
    DebugCheck(p_fileInfo->fileIndex != FILE_BAD) ;

    /* Try getting a file handle to the records file. */
    p_fileInfo->fileRecords = FileOpen(
                                 p_fileInfo->filenameRecords,
                                 FILE_MODE_READ_WRITE) ;

    /* Make sure the records file exists. */
    if (p_fileInfo->fileRecords == FILE_BAD)
        /* Something has gone wrong here.  You should never have an index */
        /* file without a records file (or visa-versa). */
        DebugCheck(FALSE) ;

    /* Load in the index. */
    p_fileInfo->p_index =
        (T_vmIndex *)FileLoad(
                         p_fileInfo->filenameIndex,
                         &indexSize) ;

    DebugCheck(indexSize != 0) ;

    /* Initialize the other fields for this session. */
    p_fileInfo->lockedRecords = 0 ;
    p_fileInfo->numEntries = indexSize / sizeof(T_vmEntry) ;
    p_fileInfo->opened = TRUE ;

    DebugEnd() ;

    return((T_vmFile)p_fileInfo) ;
}

/****************************************************************************/
/*  Routine:  VMClose                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMClose closes out all of the records opened (but waiting to be       */
/*  written).  All corresponding junk that is not needed is cleared         */
/*  from memory.                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File that is to be closed.             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IVMUpdate                                                             */
/*    FileClose                                                             */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMClose(T_vmFile file)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("VMClose") ;
    DebugCheck(file != NULL) ;

    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure we are not trying to close twice. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

    /* Mark as closed. */
    p_fileInfo->opened = FALSE ;

    /* Make sure we don't have any locked records. */
    DebugCheck(p_fileInfo->lockedRecords == 0) ;

    /* Save any changes to the index and records. */
    IVMUpdate(p_fileInfo) ;

    /* Close out the files. */
    FileClose(p_fileInfo->fileIndex) ;
    FileClose(p_fileInfo->fileRecords) ;

    /* Free the index in memory. */
    MemFree(p_fileInfo->p_index) ;

    /* Free the memory used by the file handle. */
    MemFree(p_fileInfo) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  VMAlloc                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMAlloc does all the work of finding a block of the given size in the */
/*  current virtual memory file.  If no space can be found, a VM_BLOCK_BAD  */
/*  is returned.                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to allocate vm block within       */
/*                                                                          */
/*    T_word32 size               -- Size to make the vm block              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_vmBlock                   -- block handle to block, or VM_BLOCK_BAD */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_vmBlock VMAlloc(T_vmFile file, T_word32 size)
{
    T_vmBlock block ;

    DebugRoutine("VMAlloc") ;
    DebugCheck(file != NULL) ;
    DebugCheck(size != 0) ;

    /* Try to find a block on the free list that meets this size */
    /* requirement. */
    block = IVMFindFreeBlockOfSize(file, size) ;
    if (block == VM_BLOCK_BAD)
        /* If no block was found, just add another one to the end. */
        /* NOTE:  This routine can return a bad block if the disk */
        /* is out of space. */
        block = IVMCreateBlockAtEnd(file, size) ;

    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Immediately set the reference count. */
    ((T_vmFileInfo *)file)->p_index->entries[block].refCount = 1 ;

    DebugEnd() ;

    return block ;
}

/****************************************************************************/
/*  Routine:  VMIncRefCount                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMIncRefCount increments the count of references to this block.  This */
/*  number is very important since it tells whether or not the block is     */
/*  free.  Accurate reference counts is a must.  Note that newly VMAlloc'd  */
/*  blocks start with a reference count of 1.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File containing vm block to increment  */
/*                                                                          */
/*    T_vmBlock block             -- Block to increment in vm file.         */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMIncRefCount(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("VMIncRefCount") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure the file is opened. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

    /* Make sure this isn't a free block. */
    DebugCheck(p_fileInfo->p_index->entries[block].refCount != 0) ;

    /* Increment the block's reference count. */
    p_fileInfo->p_index->entries[block].refCount++ ;

    /* Make sure we didn't roll over. */
    DebugCheck(p_fileInfo->p_index->entries[block].refCount != 0) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  VMDecRefCount                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMDecRefCount decrements the count of references to this block.  This */
/*  number is very important since it tells whether or not the block is     */
/*  free.  Accurate reference counts is a must.  Note that newly VMAlloc'd  */
/*  blocks start with a reference count of 1.  A reference count of 0 is    */
/*  a free block.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    VMDecRefCount may destroy all data related to the block if the        */
/*  reference count goes to zero.  Make VMDecRefCount the last action       */
/*  performed on any block.                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File containing vm block to decrement  */
/*                                                                          */
/*    T_vmBlock block             -- Block to decrement in vm file.         */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMDecRefCount(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;
    T_word16 refCount ;

    DebugRoutine("VMDecRefCount") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure the file is opened. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

    /* Make sure this isn't a free block. */
    DebugCheck(p_fileInfo->p_index->entries[block].refCount != 0) ;

    /* Decrement the block's reference count. */
    refCount = --(p_fileInfo->p_index->entries[block].refCount) ;

    /* If the reference count is now zero, then we can free it. */
    if (refCount == 0)
        IVMFree(file, block) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  VMGetRefCount                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMGetRefCount returns the number of references to this block.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File containing vm block to decrement  */
/*                                                                          */
/*    T_vmBlock block             -- Block to decrement in vm file.         */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 VMGetRefCount(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;
    T_word16 refCount ;

    DebugRoutine("VMGetRefCount") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure the file is opened. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

    /* Get the block's reference count. */
    refCount = p_fileInfo->p_index->entries[block].refCount ;

    DebugEnd() ;

    return refCount ;
}

/****************************************************************************/
/*  Routine:  VMLock                                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMLock is used to retrieve a block from the vm file.  In addition,    */
/*  semaphoring is done to check for read/write access priviledges.  A      */
/*  block can be locked with a semaphore on reading or writing.  If locked  */
/*  for writing semaphore, no other locking actions for write access are    */
/*  allowed.  If the lock requests read priviledge, but the block is        */
/*  locked with read semaphored, then the lock is stopped.  In addition,    */
/*  a flag is added to allow for thread blocking.  If TRUE, the calling     */
/*  will wait until the block is free.  If FALSE, the calling process will  */
/*  not wait and will not return with a block pointer.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    In this current version, the routine does not make memory write       */
/*  protected even though the semaphores may request read only.  It is up   */
/*  to the programmer to make sure read and writes are appropriate to the   */
/*  semaphore accesses.                                                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File containing vm block to decrement  */
/*                                                                          */
/*    T_vmBlock block             -- Block to decrement in vm file.         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    void *                      -- Pointer to data block, or NULL         */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileRead                                                              */
/*    VMIncRefCount                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

void *VMLock(
         T_vmFile file,
         T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;
    T_vmDataBlock *p_block ;
    T_vmEntry *p_entry ;

    DebugRoutine("VMLock") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure the file is opened. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

    /* Get a pointer to the entry for this block. */
    p_entry = &p_fileInfo->p_index->entries[block] ;

    /* Make sure we have a block that we can lock (not free). */
    DebugCheck(p_entry->refCount != 0) ;

    /* Is this block in memory or on disk? */
    if ((p_entry->vmFlags & VM_FLAG_IN_MEMORY)==0)  {
        /* Block is NOT in memory and is still on disk. */
        /* Allocate memory for this block plus a bit of header. */
        p_block = MemAlloc(p_entry->size) ;

        /* Make sure we can allocate a block. */
        DebugCheck(p_block != NULL) ;
        if (p_block == NULL)
            return NULL ;

        /* Read in the block from the file into the newly declared memory. */
        FileRead(p_fileInfo->fileRecords, p_block->data, p_entry->size) ;

        /* Fill out the block's header. (Useful when unlocking) */
        p_block->file = file ;
        p_block->block = block ;
        p_block->p_entry = p_entry ;
#ifndef NDEBUG
        p_block->vmBlockId = VM_BLOCK_ID ;
#endif

        /* Record where the block now is in memory. */
        p_entry->vmFlags |= VM_FLAG_IN_MEMORY ;
printf("Locked in at %p\n", p_block) ;
        p_entry->memoryOrNext = (T_word32)p_block ;
    } else {
        /* Block is already in memory, just use this pointer. */
        p_block = (T_vmDataBlock *)p_entry->memoryOrNext ;
    }

    /* Increment the reference count. */
    VMIncRefCount(file, block) ;

    /* Increment the lock count. */
    p_entry->lockCount++ ;

    DebugEnd() ;

    /* Return a pointer to the data of this block. */
    return((T_void *)p_block->data) ;
}

/****************************************************************************/
/*  Routine:  VMUnlock                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMUnlock releases the vm block from memory.  If multiple locks have   */
/*  been executed, an equal number of unlocks must be performed to remove   */
/*  the block from memory.                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *                    -- Pointer to block previously locked     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    VMDecRefCount                                                         */
/*    IVMRelease                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMUnlock(void *block)
{
    T_vmDataBlock *p_block ;
    T_vmEntry *p_entry ;

    DebugRoutine("VMUnlock") ;
    DebugCheck(block != NULL) ;

    /* Get a pointer to the block's header. */
    p_block = (T_vmDataBlock *)(((T_byte8 *)block)-sizeof(T_vmDataBlock)) ;

    /* Make sure we are pointing at a block. */
    DebugCheck(p_block->vmBlockId == VM_BLOCK_ID) ;

    /* Now that we have a pointer, extract the pointer to the entry. */
    p_entry = p_block->p_entry ;
    DebugCheck(p_entry != NULL) ;

    /* Decrement the lock count and see if we are now no longer locked */
    /* in memory. */
    if ((--p_entry->lockCount) == 0)
        /* Not locked any more.  Release from memory. */
        IVMRelease(p_block->file, p_block->block) ;

    /* Decrement the reference count on this block. */
    VMDecRefCount(p_block->file, p_block->block) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  VMCleanUp                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    From time to time, files will become 'pitted' with free blocks that   */
/*  nothing will use.  When this happens, all the free spots need to be     */
/*  compressed and expulged.  All free spots are removed and everything is  */
/*  relocated correctly.  Major files should have this routine called       */
/*  on them at least once a day.  Note that this routine should only be     */
/*  called on FULLY CLOSED vm files.  If you don't the file will be         */
/*  destroyed since two versions will exist (one in memory, one on disk).   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The VM file that is being cleaned-up should NOT be opened at this     */
/*  time.  No checks are made to confirm this.                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- Name of file to clean-up               */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMCleanUp(T_byte8 *p_filename)
{
    T_vmFile file ;
    T_vmBlock block ;
    T_word32 place ;
    T_vmHeader *p_header ;

    DebugRoutine("VMCleanUp") ;
    DebugCheck(p_filename != NULL) ;
    DebugCheck(strlen(p_filename) != 0) ;

    /* First block starts at zero. */
    place = 0 ;

    /* Open the file to clean up. */
    file = VMOpen(p_filename) ;
    DebugCheck(file != NULL) ;

    /* Loop through all the blocks, picking them out in order of their */
    /* placement in the file. */
    while ((block = IVMFindLowestBlock(file, place)) != 0)  {
        /* Check to see if this is a free block. */
        if ((((T_vmFileInfo *)file)->p_index->entries[block].refCount) != 0)
            /* Shift the block down and return where the end of the block */
            /* is. */
            IVMShiftBlockDown(file, block, &place) ;
    }

    /* We're done with all the blocks.  Truncate the extra size off. */
    IVMTruncate(file, place) ;

    /* Change the index header so there is no free entries anymore. */
    p_header = &(((T_vmFileInfo *)file)->p_index->header) ;
    p_header->numEntries = p_header->numAllocatedEntries ;
    p_header->numTotalEntries = p_header->numAllocatedEntries ;
    p_header->numFreeEntries = 0 ;
    p_header->firstFree = 0 ;
    p_header->lastByte = 0 ;

    VMClose(file) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  VMDirty                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    VMDirty marks a block dirty so the block is saved to disk.            */
/*  Otherwise, the block is never saved and any changes are lost.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File containing newly dirty block      */
/*                                                                          */
/*    T_vmBlock block             -- Block in file to mark dirty            */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void VMDirty(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("VMDirty") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure the file is opened. */
    DebugCheck(p_fileInfo->opened == TRUE) ;

printf("Mark dirty block %d\n", block) ;
    /* Set the dirty bit in the flags for that block. */
    p_fileInfo->p_index->entries[block].vmFlags |= VM_FLAG_DIRTY ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMCreate                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMCreate opens up a new vm file and vm records file.  Any previous   */
/*  data is lost.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFileInfo *p_fileInfo    -- Pointer to file info structure to      */
/*                                   create.                                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileOpen                                                              */
/*    FileSeek                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMCreate(T_vmFileInfo *p_fileInfo)
{
    T_file file ;
    T_vmHeader header ;

    DebugRoutine("IVMCreate") ;
    DebugCheck(p_fileInfo != NULL) ;
puts("IVMCreate") ;

    /* Open up the new file (an old file doesn't matter since we are */
    /* over writing the important information). */
    p_fileInfo->fileIndex = file =
        FileOpen(p_fileInfo->filenameIndex, FILE_MODE_READ_WRITE) ;

    /* Make sure the file is good. */
    DebugCheck(file != FILE_BAD) ;
    if (file != FILE_BAD)  {
        header.numEntries = 0 ;
        header.numTotalEntries = 0 ;
        header.numAllocatedEntries = 0 ;
        header.numFreeEntries = 0 ;
        header.firstFree = 0 ;
        header.lastByte = 0 ;
        FileSeek(file, 0) ;
        FileWrite(file, &header, sizeof(T_vmHeader)) ;
        FileSeek(file, 0) ;
        FileClose(file) ;
    }

    /* Open up the other file. */
    p_fileInfo->fileRecords = file =
        FileOpen(p_fileInfo->filenameRecords, FILE_MODE_READ_WRITE) ;
    DebugCheck(file != FILE_BAD) ;

    FileSeek(file, 0) ;
    FileClose(file) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMUpdate                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMUpdate saves all dirty records, mark them as clean, and then saves */
/*  the whole index.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFileInfo *p_fileInfo    -- Pointer to file info structure to      */
/*                                   update.                                */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMUpdate(T_vmFileInfo *p_fileInfo)
{
    T_word32 i ;
    T_word32 num ;

    DebugRoutine("IVMUpdate") ;
    DebugCheck(p_fileInfo != NULL) ;

    /* Get the number of items in the list. */
    num = p_fileInfo->p_index->header.numEntries ;

    /* Go through the list of blocks and update each one if */
    /* they are dirty. */
    for (i=1; i<num; i++)
        IVMSaveIfDirty(p_fileInfo, &p_fileInfo->p_index->entries[i]) ;

    /* Now save the whole index. */
    IVMSaveIndex(p_fileInfo) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  IVMFindFreeBlockOfSize                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMFindFreeBlockOfSize searches for the first block that meets the    */
/*  given size requirement.  In addition, it splits the blocks accordingly. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Based on the way this routine works, it is fast, but causes alot      */
/*  of fragmentation.                                                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to search for block within        */
/*                                                                          */
/*    T_word32 size               -- Size to find                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IVMAppendEntry                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_vmBlock IVMFindFreeBlockOfSize(T_vmFile file, T_word32 size)
{
    T_vmFileInfo *p_fileInfo ;
    T_vmBlock block = VM_BLOCK_BAD ;
    T_word32 entry ;
    T_word32 previous ;
    T_sword32 diffSize ;
    T_word32 newEntry ;

    DebugRoutine("IVMFindFreeBlockOfSize") ;
    DebugCheck(file != NULL) ;
    DebugCheck(size != 0) ;

    /* Get a quick pointer to the file structure. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Search through the free list. */
    previous = 0 ;
    entry = p_fileInfo->p_index->header.firstFree ;
    while (entry != 0)  {
        /* this had better be a free block. */
        DebugCheck(p_fileInfo->p_index->entries[entry].refCount == 0) ;

        /* Find the difference in the sizes. */
        diffSize = (T_sword32)size -
                       (T_sword32)p_fileInfo->p_index->entries[entry].size ;

        /* See if this block is big enough. */
        if (diffSize >= 0)  {
            /* We found one.  Is it the exact size, or do we need */
            /* to cut it apart? */
            if (diffSize == 0)  {
                /* Same size. */
                /* Remove the block from the list. */
                if (previous == 0)  {
                    /* No previous free entry, make the next one past this */
                    /* one be the next free. */
                    p_fileInfo->p_index->header.firstFree =
                        p_fileInfo->p_index->entries[entry].memoryOrNext ;
                } else {
                    /* Make the previous one point to the next one. */
                    p_fileInfo->p_index->entries[previous].memoryOrNext =
                        p_fileInfo->p_index->entries[entry].memoryOrNext ;
                }
                newEntry = entry ;
            } else {
                /* Bigger. */
                /* We need to split the block into two halves, the part */
                /* we need and the part that is free.  Just make this block */
                /* smaller and add a new entry in the index for the part */
                /* that is to be used. */
                p_fileInfo->p_index->entries[entry].size -= diffSize ;

                /* Now create a new entry at the end. */
                newEntry = IVMAppendEntry(file) ;

                /* Note where the block starts in the record file. */
                /* (This, in effect, splits the free block and makes */
                /* the first part the free half and the rest is the */
                /* new block).  The great thing about doing it this way */
                /* is that the links are still the same. */
                p_fileInfo->p_index->entries[newEntry].fileOffset =
                    p_fileInfo->p_index->entries[entry].fileOffset+diffSize ;
            }

            /* Prepare this new block. */
            p_fileInfo->p_index->entries[newEntry].refCount = 0 ;
            p_fileInfo->p_index->entries[newEntry].vmFlags = 0 ;
            p_fileInfo->p_index->entries[newEntry].memoryOrNext = 0 ;
            p_fileInfo->p_index->entries[newEntry].size = size ;
            p_fileInfo->p_index->entries[newEntry].lockCount = 0 ;

            /* Note this is the block we want to return. */
            block = (T_vmBlock)newEntry ;
            break ;
        }

        /* Haven't found the right size block yet. */
        previous = entry ;
        entry = p_fileInfo->p_index->entries[entry].memoryOrNext ;
    }

    DebugEnd() ;

    return block ;
}

/****************************************************************************/
/*  Routine:  IVMCreateBlockAtEnd                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMCreateBlockAtEnd assumes that the best place for a new block is    */
/*  at the end of the records file (and at the end of the index).           */
/*  A new block is created there.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to append block to                */
/*                                                                          */
/*    T_word32 size               -- Size of block                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IVMAppendEntry                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_vmBlock IVMCreateBlockAtEnd(T_vmFile file, T_word32 size)
{
    T_vmFileInfo *p_fileInfo ;
    T_word32 newEntry ;

    DebugRoutine("IVMCreateBlockAtEnd") ;
    DebugCheck(file != NULL) ;
    DebugCheck(size != 0) ;

    /* Find a new entry in the file. */
    newEntry = IVMAppendEntry(file) ;

    /* Get a quick file pointer. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Prepare this new block. */
    p_fileInfo->p_index->entries[newEntry].refCount = 0 ;
    p_fileInfo->p_index->entries[newEntry].vmFlags = 0 ;
    p_fileInfo->p_index->entries[newEntry].memoryOrNext = 0 ;
    p_fileInfo->p_index->entries[newEntry].size = size ;
    p_fileInfo->p_index->entries[newEntry].lockCount = 0 ;

    /* Now allocate space at the end of the file. */
    p_fileInfo->p_index->entries[newEntry].fileOffset =
        p_fileInfo->p_index->header.lastByte ;

    /* Move where the end is now located. */
    p_fileInfo->p_index->header.lastByte += size ;

    DebugEnd() ;

    return newEntry ;
}

/****************************************************************************/
/*  Routine:  IVMFree                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMFree takes a originally reference allocated block and frees it     */
/*  and puts it on the linked list of free blocks.                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Obviously you should not call this on an already freed block.         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to free block within              */
/*                                                                          */
/*    T_vmBlock block             -- Block that is being freed              */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMFree(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("IVMFree") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != 0) ;

    /* Get a quick file pointer. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Make sure this is considered a to-be-freed block. */
    DebugCheck(p_fileInfo->p_index->entries[block].refCount == 0) ;

    /* If there is any memory attached to this block, then we need to */
    /* dispose of it.  No saving is necessary since the block is being */
    /* freed. */
printf("Free %p\n", (T_void *)p_fileInfo->p_index->entries[block].memoryOrNext) ;
    if (p_fileInfo->p_index->entries[block].vmFlags & VM_FLAG_IN_MEMORY)
        MemFree((T_void *)p_fileInfo->p_index->entries[block].memoryOrNext) ;

    /* Just put the given block on the free list.  Since order does */
    /* not matter, put the newly freed block on the beginning of the */
    /* free list. */
    p_fileInfo->p_index->entries[block].memoryOrNext =
        p_fileInfo->p_index->header.firstFree ;

    p_fileInfo->p_index->header.firstFree = block ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMRelease                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMRelease makes a block that was in memory be released to disk (and  */
/*  saves if necessary).                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Obviously you should not call this on an already freed block.         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to release block within           */
/*                                                                          */
/*    T_vmBlock block             -- Block that is being released           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IVMSaveIfDirty                                                        */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMRelease(T_vmFile file, T_vmBlock block)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("IVMRelease") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != 0) ;

    /* Get a quick file pointer. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* !!! For now, we will just save the block to disk and remove it */
    /* from memory.  However, later I want to make this use the */
    /* discardable list capability so that it can be kept in memory */
    /* as long as possible. */

    /* Check to see if the block needs to be saved, and if so, save it. */
    IVMSaveIfDirty(p_fileInfo, &p_fileInfo->p_index->entries[block]) ;

    /* Now that it is saved, we can remove it from memory. */
    MemFree((T_void *)p_fileInfo->p_index->entries[block].memoryOrNext) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMFindLowestBlock                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMFindLowestBlock searches the index to find the lowest block that   */
/*  is above the given file offset.   If none is found, a zero is returned. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to release block within           */
/*                                                                          */
/*    T_vmBlock block             -- Block that is being released           */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_vmBlock IVMFindLowestBlock(T_vmFile file, T_word32 place)
{
    T_vmFileInfo *p_fileInfo ;
    T_word32 i ;
    T_word32 bestBlock = 0 ;
    T_word32 bestOffset = 0xFFFFFFFF ;
    T_vmEntry *p_entry ;
    T_word32 num ;

    DebugRoutine("IVMFindLowestBlock") ;
    DebugCheck(file != NULL) ;

    /* Get a quick file pointer. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Find out how many entries are in use. */
    num = p_fileInfo->p_index->header.numEntries ;

    /* Loop through all entries looking for the lowest. */
    for (i=1; i<num; i++)  {
        /* Get a quick pointer to this entry. */
        p_entry = &p_fileInfo->p_index->entries[i] ;

        /* Are we above the limit and below the last found block? */
        if ((p_entry->fileOffset >= place) &&
            (p_entry->fileOffset < bestOffset))  {
            /* Yes, we are.  Make this the best one so far. */
            bestOffset = p_entry->fileOffset ;
            bestBlock = i ;
        }
    }

    DebugEnd() ;

    /* Return who was the best block. */
    return bestBlock ;
}

/****************************************************************************/
/*  Routine:  IVMSaveIfDirty                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMSaveIfDirty checks the given block if is dirty, and if so, saves   */
/*  it to disk.  Otherwise, nothing happens.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmEntry *p_entry          -- Pointer to entry to check to save      */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMSaveIfDirty(T_vmFileInfo *p_fileInfo, T_vmEntry *p_entry)
{
    DebugRoutine("IVMSaveIfDirty") ;
    DebugCheck(p_fileInfo != NULL) ;
    DebugCheck(p_entry != NULL) ;

    if ((p_entry->vmFlags & VM_FLAG_DIRTY) != 0)  {
puts("Saving block") ;
        /* Block is dirty, save it. */

        /* Seek the location to save. */
        FileSeek(p_fileInfo->fileRecords, p_entry->fileOffset) ;

        /* Write out all of the data. */
        FileWrite(
            p_fileInfo->fileRecords,
            (T_void *)p_entry->memoryOrNext,
            p_entry->size) ;

        /* Clear the dirty flag. */
        p_entry->vmFlags &= (~VM_FLAG_DIRTY) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMSaveIndex                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMSaveIndex saves the index file.                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFileInfo *p_fileInfo    -- Pointer to the file information struct */
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
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMSaveIndex(T_vmFileInfo *p_fileInfo)
{
    DebugRoutine("IVMSaveIndex") ;
    DebugCheck(p_fileInfo != NULL) ;

    /* Move back to the start. */
    FileSeek(p_fileInfo->fileIndex, 0) ;

    /* Save the index. */
    FileWrite(
        p_fileInfo->fileIndex,
        p_fileInfo->p_index,
        p_fileInfo->p_index->header.numTotalEntries * sizeof(T_vmEntry) +
            sizeof(T_vmHeader)) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMAppendEntry                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMAppendEntry adds another entry at the bottom of the index.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to append an entry.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IVMUpdate                                                             */
/*    MemAlloc                                                              */
/*    memcpy                                                                */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 IVMAppendEntry(T_vmFile file)
{
    T_vmFileInfo *p_fileInfo ;
    T_word32 block ;
    T_vmIndex *p_newIndex ;

    DebugRoutine("IVMAppendEntry") ;

    /* Get a quick file pointer. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* In any case, we are taking the last one on the list. */
    block = p_fileInfo->p_index->header.numEntries ;

    /* Get rid of zero as the first case. */
    if (block == 0)
        block = 1 ;

    /* Is there enough room for another entry? */
    if (block < p_fileInfo->p_index->header.numTotalEntries)  {
        /* Enough space in the current version.  Just take one more. */
        p_fileInfo->p_index->header.numEntries = block+1 ;
    } else {
        /* Not enough space.  Now we have a big problem.  We need to */
        /* expand the whole list. */

        /* Grow the total size. */
        p_fileInfo->p_index->header.numTotalEntries += VM_ENTRY_GROW_SIZE ;

        /* Now we need to allocate more memory for this routine. */
        /* To be safe, go ahead and save everything. */
        IVMUpdate(p_fileInfo) ;

        /* Now allocate memory for another index. */
        p_newIndex = MemAlloc(
            p_fileInfo->p_index->header.numTotalEntries * sizeof(T_vmEntry) +
            sizeof(T_vmHeader)) ;

        /* Make sure we go the memory. */
        DebugCheck(p_newIndex != NULL) ;

        /* Copy the index over. */
        memcpy(
            p_newIndex,
            p_fileInfo->p_index,
            sizeof(T_vmHeader) +
                p_fileInfo->p_index->header.numEntries * sizeof(T_vmEntry)) ;

        /* Free the old index. */
        MemFree(p_fileInfo->p_index) ;

        /* Declare the new index. */
        p_fileInfo->p_index = p_newIndex ;

        /* Increment the number of entries. */
        p_fileInfo->p_index->header.numEntries = block+1 ;
    }

    DebugEnd() ;

    return block ;
}

/****************************************************************************/
/*  Routine:  IVMShiftBlockDown                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMShiftBlockDown moves a vm block within the records file to the     */
/*  given place.  The byte after the block is also returned.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to shift block within             */
/*                                                                          */
/*    T_vmBlock block             -- Block to shift                         */
/*                                                                          */
/*    T_word32 *place             -- Place to shift down to (and variable   */
/*                                   to record end of block).               */
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
/*    FileSeek                                                              */
/*    FileRead                                                              */
/*    FileWrite                                                             */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMShiftBlockDown(T_vmFile file, T_vmBlock block, T_word32 *place)
{
    T_vmFileInfo *p_fileInfo ;
    T_word32 where ;
    T_word32 size ;
    T_file recordsFile ;
    T_byte8 *tempBlock ;
    T_word32 newPlace ;

    DebugRoutine("IVMShiftBlockDown") ;
    DebugCheck(file != NULL) ;
    DebugCheck(block != VM_BLOCK_BAD) ;
    DebugCheck(place != NULL) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* Find where the block is currently at. */
    where = p_fileInfo->p_index->entries[block].fileOffset ;

    /* What is the handle to the records file. */
    recordsFile = p_fileInfo->fileRecords ;

    /* How big is the block? */
    size = p_fileInfo->p_index->entries[block].size ;

    /* Copy where we are trying to go to. */
    newPlace = *place ;

    /* Allocate some memory for transfering data. */
    tempBlock = MemAlloc(VM_SHIFT_BLOCK_SIZE) ;
    DebugCheck(tempBlock != NULL) ;

    /* Somethings a block is bigger than one shift, so we must keep */
    /* shifting a group at a time. */
    while (size >= VM_SHIFT_BLOCK_SIZE)  {
        FileSeek(recordsFile, where) ;
        FileRead(recordsFile, tempBlock, VM_SHIFT_BLOCK_SIZE) ;
        FileSeek(recordsFile, *place) ;
        FileWrite(recordsFile, tempBlock, VM_SHIFT_BLOCK_SIZE) ;
        size -= VM_SHIFT_BLOCK_SIZE ;
        *place += VM_SHIFT_BLOCK_SIZE ;
        where += VM_SHIFT_BLOCK_SIZE ;
    }

    /* Now shift the last block if it is not zero in size. */
    if (size != 0)  {
        FileSeek(recordsFile, where) ;
        FileRead(recordsFile, tempBlock, size) ;
        FileSeek(recordsFile, *place) ;
        FileWrite(recordsFile, tempBlock, size) ;
        *place += size ;
        where += size ;
    }

    /* Free the temporary transfer block. */
    MemFree(tempBlock) ;

    /* Note where the block can now be found. */
    p_fileInfo->p_index->entries[block].fileOffset = newPlace ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IVMTruncate                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IVMTruncate clips the end of the vm file so that the file takes up    */
/*  less disk space.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_vmFile file               -- File to truncate                       */
/*                                                                          */
/*    T_word32 place              -- where to truncate the file             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IVMTruncate(T_vmFile file, T_word32 place)
{
    T_vmFileInfo *p_fileInfo ;

    DebugRoutine("IVMTruncate") ;
    DebugCheck(file != NULL) ;

    /* Get a pointer to the file information. */
    p_fileInfo = (T_vmFileInfo *)file ;

    /* !!! At this point, I do not know what the truncate code should be. */

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  VM.C                                                    */
/****************************************************************************/
