/*-------------------------------------------------------------------------*
 * File:  MEMORY.C
 *-------------------------------------------------------------------------*/
/**
 * The Memory allocation system is here.  There is a fair amount of
 * specialized code going on here as we allocate and free memory.
 * When debug mode is enabled, memory can be tracked by who allocated it
 * and if there are any memory leaks.
 *
 * @addtogroup MEMORY
 * @brief Memory Allocation/Freeing
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#ifndef NDEBUG
#ifdef WIN32
///#define _MEM_CHECK_FULL_
///#define _MEM_RECORD_ROUTINES_
//#define COMPILE_OPTION_OUTPUT_ALLOCATION
#endif
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "GRAPHICS.H"
#include "MEMORY.H"

/* Turn this on to output !A and !F that tell who frees what */
//#define COMPILE_OPTION_OUTPUT_ALLOCATION

#define MEM_BLOCK_TAG_SIZE 4

/* This is a header block to attach at the beginning of all allocated */
/* blocks. */
struct T_memBlockHeader_tag {
    T_word32 blockId ;
    T_byte8 blockTag[MEM_BLOCK_TAG_SIZE] ;
    struct T_memBlockHeader_tag *p_nextBlock ;
    struct T_memBlockHeader_tag *p_prevBlock ;
    T_memDiscardCallbackFunc p_callback ;
    /* DEBUG! */
#ifdef _MEM_RECORD_ROUTINES_
    T_byte8 *routine ;   /* Calling routine and line number. */
    long line ;
#endif
    T_word32 size ;
} ;

typedef struct T_memBlockHeader_tag T_memBlockHeader ;

/* Number of block being allocated.  Start at one. */
static T_word32 G_allocCount = 1 ;
static T_word32 G_deallocCount = 0 ;

//#ifndef NDEBUG
static T_word32 G_sizeAllocated = 0 ;
static T_word32 G_maxSizeAllocated = 0 ;
//#endif

/* Pointer to beginning of discarded list. */
static T_memBlockHeader *P_startOfDiscardList = NULL ;

/* Pointer to end of discarded list.  The end represents the oldest */
/* items in the discard list and is where memory should be freed first. */
static T_memBlockHeader *P_endOfDiscardList = NULL ;

/* Internal Functions Prototypes: */
static E_Boolean IMemFindFreeSpace(T_void) ;

T_word32 FreeMemory(T_void) ;

#ifdef _MEM_CHECK_FULL_
#define MAX_BLOCK_LIST 200000
static T_memBlockHeader *G_blockList[MAX_BLOCK_LIST] ;
static T_word16 G_numBlocks = 0 ;
static T_word16 G_firstFree = 0xFFFF ;
static T_void ICheckAllocated(T_word16 noteNum) ;
static T_word16 IFindPointer(T_memBlockHeader *p_ptr) ;
#endif

/*-------------------------------------------------------------------------*
 * Routine:  MemAlloc
 *-------------------------------------------------------------------------*/
/**
 *  Currently, this routine is provided more for debugging than an
 *  actual utility.  However, this should be the one routine that everyone
 *  calls to allocate memory (no matter what system).
 *  Since we use this for debugging, we will assign a number to each
 *  piece of memory that is allocated and will attach a "tag" at the
 *  beginning to make the system have a way to check if we are later
 *  freeing a block, or just random memory.
 *  Space is also provided to allow the block to be declared as discard-
 *  able.  A discardable block needs to be able to be placed on a double
 *  link list and have a call back function (for when the block is actually
 *  discarded).  See MemMarkDiscardable for more details.
 *
 *  NOTE: 
 *  I'm sure what the size limitations for this will be, so we had
 *  best stick to 64K or smaller allocations.  Note that this routine is
 *  also made for blocks of typically larger than 256 bytes.  If you got
 *  alot of small parts, you might want to try doing it a different way.
 *
 *  @param size -- Amount of memory to allocate
 *
 *  @return Pointer or NULL to memory block.
 *
 *<!-----------------------------------------------------------------------*/
T_void *MemAlloc(T_word32 size)
{
    T_byte8 *p_memory ;
    T_memBlockHeader *p_header ;
    E_Boolean memFound ;
#if _MEM_CHECK_FULL_
    T_word16 next ;
#endif
    const char *p_name ;
    long line ;

    DebugRoutine("MemAlloc") ;

DebugGetCaller(&p_name, &line) ;
//printf("|%s,%ld\.0\n", p_name, size) ;
#ifdef COMPILE_OPTION_OUTPUT_ALLOCATION
//printf("!A %d %s\n", size, DebugGetCallerFile()) ;
printf("!A %d %s:%s ", size, DebugGetCallerFile(), DebugGetCallerName()) ;
#endif
    /* Allocate memory and room for our tag. */
    do {
//DebugCheck(_heapchk() == _HEAPOK) ;
        p_memory = malloc(sizeof(T_memBlockHeader)+size) ;
//DebugCheck(_heapchk() == _HEAPOK) ;

        /* If the memory was not allocated, check to see if */
        /* Memory can be allocated. */
        if (p_memory == NULL)  {
            /* Check if there are blocks that we can discard. */
            if (IMemFindFreeSpace()==TRUE)  {
                /* If there is, note that we have not found memory */
                /* so the loop continues trying to allocate. */
                memFound = FALSE ;
            } else {
                /* If there is not, say we found memory to break */
                /* out of the loop (when we actually did not). */
                memFound = TRUE ;
            }
        } else {
            memFound = TRUE ;
        }
    } while (memFound == FALSE) ;

    /* Check to see if we actually got the memory. */
    if (p_memory != NULL)  {
        /* Initialize the header for this block. */
        p_header = (T_memBlockHeader *)p_memory ;

#ifdef _MEM_RECORD_ROUTINES_
       DebugGetCaller(&p_header->routine, &p_header->line) ;
#endif
#ifdef _MEM_CHECK_FULL_
       /* Place the block on the allocated block list. */
       if (G_firstFree == 0xFFFF)  {
           /* Use a fresh block. */
           DebugCheck(G_numBlocks < MAX_BLOCK_LIST) ;
           G_blockList[G_numBlocks++] = p_header ;
       } else {
           /* Take one off the free list. */
           next = (T_word32)G_blockList[G_firstFree] ;
           G_blockList[G_firstFree] = p_header ;
           G_firstFree = next ;
       }
#endif
        p_header->size = size ;
        /* Make sure the block has been tagged. */
        strcpy((char *)p_header->blockTag, "TaG") ;

        /* Make sure that the block has an id (for debugging). */
        p_header->blockId = G_allocCount++ ;

        /* Clear out the previous and next pointers. */
        p_header->p_nextBlock = p_header->p_prevBlock = NULL ;

        /* Make sure the callback routine points to no where. */
        p_header->p_callback = NULL ;

/* Get who called this routine. */
//DebugGetCaller(&p_header->routine, &p_header->line) ;

        /* Move pointer up past header. That way we can return a pointer
           to where the information really is. */
        p_memory += sizeof(T_memBlockHeader) ;
//DebugCheck(_heapchk() == _HEAPOK) ;
    }
//printf("ALLC: %d   \r", FreeMemory()) ;
//fflush(stdout) ;
//printf("alloc %d, ID: %d, @ %p, %s\n", size, p_header->blockId, p_memory, DebugGetCallerName()) ;
//fflush(stdout) ;

//DebugCheck(p_memory != NULL) ;

//#ifndef NDEBUG
    /* Add to the total amount of memory allocated this size. */
    G_sizeAllocated += sizeof(T_memBlockHeader)+size ;

    /* Is this the biggest it has ever been? */
    if (G_sizeAllocated > G_maxSizeAllocated)
        /* Yes, it is bigger.  Store this new maximum. */
        G_maxSizeAllocated = G_sizeAllocated ;
//#endif

    DebugEnd() ;

#ifndef REAL_MODE
    if (p_memory==NULL)
    {
        /* fail hard! out of memory ! */
        GrGraphicsOff();
        printf ("Out of memory error, exiting\n");
        MemDumpDiscarded() ;
        exit(-1);
    }
#endif
#ifdef COMPILE_OPTION_OUTPUT_ALLOCATION
printf("(@0x%08X)\n", p_memory) ;
#endif
    return p_memory ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MemFree
 *-------------------------------------------------------------------------*/
/**
 *  MemFree frees a block of memory that was previously allocated.
 *  It also checks the integrity of the pointer given to it.  If the
 *  pointer is NULL, it crashes.  If the pointer does not point to a
 *  block that was allocated, it crashes.
 *
 *  NOTE: 
 *  None that I can think of (except this will definitely slow down
 *  the system a little).
 *
 *  @param p_data -- Pointer to block to free
 *      Normally we can assume p_data points to data and is not NULL.
 *
 *<!-----------------------------------------------------------------------*/
T_void MemFree(T_void *p_data)
{
    T_byte8 *p_bytes ;
    T_memBlockHeader *p_header ;
#if _MEM_CHECK_FULL_
    T_word16 pos ;
#endif

    DebugRoutine("MemFree") ;
    DebugCheck(p_data != NULL) ;

    /* Back up from the pointer we are given and try to find the
       header tag. */
    p_bytes = p_data ;
    p_bytes -= sizeof(T_memBlockHeader) ;

    p_header = (T_memBlockHeader *)p_bytes ;
//printf("Freeing %p, ID: %d, for %s\n", p_data, p_header->blockId, DebugGetCallerName()) ;

//printf("free ID: %ld, @ %p, %s\n", p_header->blockId, p_data, DebugGetCallerName()) ;
//fflush(stdout) ;
#ifdef _MEM_CHECK_FULL_
    MemCheck(10210) ;
    /* Find the block on the list (if it is there). */
    pos = IFindPointer(p_header) ;

    if (pos == 0xFFFF)  {
        printf("bad attempt to free memory at %p\n", p_data) ;
        DebugCheck(FALSE) ;
    }
    /* Remove the block from the alloc list and put on the free list. */
    G_blockList[pos] = ((T_memBlockHeader *)((T_word32)G_firstFree)) ;

    G_firstFree = pos ;
#endif

//printf("MemFree: %s (%p)\n", ((T_memBlockHeader *)p_bytes)->blockTag, p_data) ;
    /* Make sure we are freeing one of our blocks. */
    DebugCheck(strcmp (((T_memBlockHeader *)p_bytes)->blockTag, "DaG") != 0) ;
    DebugCheck(strcmp (((T_memBlockHeader *)p_bytes)->blockTag, "TaG") == 0) ;

    /* Check if the tag is there, or bomb. */
    DebugCheck(strcmp(((T_memBlockHeader *)p_bytes)->blockTag, "TaG") == 0) ;
    strcpy(((T_memBlockHeader *)p_bytes)->blockTag, "!!!");

//#ifndef NDEBUG
    /* Note that the total is now less. */
    G_sizeAllocated -= sizeof(T_memBlockHeader) + p_header->size ;
#ifdef COMPILE_OPTION_OUTPUT_ALLOCATION
printf("!F %d %s (@0x%08X)\n", p_header->size, DebugGetCallerFile(), p_bytes) ;
printf("!F %d %s:%s\n", p_header->size, DebugGetCallerFile(), DebugGetCallerName()) ;
#endif

#ifndef NDEBUG
    /* Make sure we didn't roll under. */
    DebugCheck(G_sizeAllocated < 0xF0000000) ;
#endif
#ifndef NDEBUG
    memset(p_bytes, 0xCD, sizeof(T_memBlockHeader) + p_header->size) ;
#endif
    /* OK, free up the valid block. */
    free(p_bytes) ;
//puts("OK") ;
//printf("FREE: %d   \r", FreeMemory()) ;
//fflush(stdout) ;

    /* Cound how many are freed. */
    G_deallocCount++ ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MemMarkDiscardable
 *-------------------------------------------------------------------------*/
/**
 *  As an alternative to freeing a block of memory, you may choose to
 *  make the memory discardable.  Doing so allows you to keep the block
 *  in memory as long as possible without automatically get rid of it.
 *  This allows for other routines to keep blocks cached in memory instead
 *  of accessing the disk drive over and over again.
 *  To facilitate discardable blocks, a callback routine is also passed
 *  to tell the original caller that the block is no longer in memory.
 *  Typically the callback routine will remove the item from a list.
 *
 *  NOTE: 
 *  All routines that use memory have to use MemAlloc/MemFree or else
 *  several discarded blocks may be using up all the standard memory.  Only
 *  MemAlloc actually calls the routines to free up discarded memory.
 *
 *  @param p_data -- Pointer to the data block
 *  @param p_callback -- Pointer to the function that should
 *      be called when this block is freed
 *      from memory.
 *      You can use NULL if you don't care,
 *      but if you do, you might as well use
 *      MemFree since this is the link that
 *      allows you to know what the block
 *      status really is.
 *
 *<!-----------------------------------------------------------------------*/
T_void MemMarkDiscardable(
           T_void *p_data,
           T_memDiscardCallbackFunc p_callback)
{
    T_memBlockHeader *p_header ;

    /* All that needs to be done to make a memory block discardable is */
    /* to place it on the discard list.  This is a double linked list */
    /* that has the newest entries (youngest) at the front and the */
    /* oldest at then end of the list.  By doing so, we automatically */
    /* keep track of who to actually remove from memory based on age. */
    /* The double links are needed in case the block is reclaimed at */
    /* a later point by MemReclaimDiscardable().  */

    DebugRoutine("MemMarkDiscardable") ;
    DebugCheck(p_data != NULL) ;

    /* Start by accessing the header for this block. */
    p_header = (T_memBlockHeader *)
               (((T_byte8 *)p_data) - sizeof(T_memBlockHeader)) ;
//printf("MemMarkDiscardable %p %s\n", p_header, DebugGetCallerName()) ; fflush(stdout) ;

#ifndef NDEBUG
    if (strcmp(p_header->blockTag, "TaG") != 0)  {
        printf("Funny tag: %4.4s  (%02X %02X %02X %02X) for %p\n",
            p_header->blockTag,
            p_header->blockTag[0],
            p_header->blockTag[1],
            p_header->blockTag[2],
            p_header->blockTag[3],
            p_data) ;
        printf("Before: >>\n") ;
        fwrite(p_header-1000, 1000, 1, stdout) ;
        printf("\n<<\n\nAfter: >>\n") ;
        fwrite(p_header, 1000, 1, stdout) ;
        printf("\n<<\n\n") ;
    }
#endif

    DebugCheck(strcmp(p_header->blockTag, "TaG") == 0) ;

    /* Note that this is a discardable block by changing the tag. */
    p_header->blockTag[0] = 'D' ;

    /* Put this block on the discard list. */

    /* The next block is the current beginning of the list (or NULL) */
    p_header->p_nextBlock =
        (struct T_memBlockHeader_tag *)P_startOfDiscardList ;

    /* Note that we are the beginning by nulling out the previous field */
    p_header->p_prevBlock = NULL ;

    if (P_startOfDiscardList == NULL)  {
        /* If NULL start of discard list, initialize the end of the */
        /* discard list. */
        P_endOfDiscardList = p_header ;
    } else {
        /* If not NULL, then we need to make the first item */
        /* point to the new item. */
        P_startOfDiscardList->p_prevBlock =
            (struct T_memBlockHeader_tag *)p_header ;
    }
    /* In both cases, make the start of the discard list */
    /* point to this new item. */
    P_startOfDiscardList = p_header ;

    /* Finally, record the callback functions in the block itself. */
    p_header->p_callback = p_callback ;

#ifndef NDEBUG
/*
    p_data = (T_byte8 *)(p_header+1) ;
    for (i=0; i<p_header->size; i++)
       ((T_byte8 *)p_data)[i] ^= 0xFF ;
*/
#endif
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MemReclaimDiscardable
 *-------------------------------------------------------------------------*/
/**
 *  Should you need to use a block that you know you had allocated in
 *  in the past and later marked discardable, you can reclaim the block.
 *  By calling this routine, you remove the block from the discard list
 *  and make it act like a normal block.
 *
 *  NOTE: 
 *  The caller must keep up with what blocks are discarded and what their
 *  original pointers are.  The callback function provided in MemMarkDisc()
 *  provides a method to note when the block is finally removed.
 *
 *  @param p_data -- Pointer to originally discarded block
 *
 *<!-----------------------------------------------------------------------*/
T_void MemReclaimDiscardable(T_void *p_data)
{
    T_memBlockHeader *p_header ;

    /* All we have to do here is remove the block that was originally */
    /* discarded from the discard list.  Pretty simple, huh? */
    /* Since we used a double linked list, it is only a matter of */
    /* "mending" the links of the previous and next block. */

    DebugRoutine("MemReclaimDiscardable") ;
    DebugCheck(p_data != NULL) ;
//printf("Reclaim: %p\n", p_data) ; fflush(stdout) ;
    /* Start by accessing the header for this block. */
    p_header = (T_memBlockHeader *)(((T_byte8 *)p_data) - sizeof(T_memBlockHeader)) ;

    /* Make sure you are restoring a discarded block. */
    DebugCheck(strcmp(p_header->blockTag, "DaG") == 0) ;

    /* Take care of the previous block (if any) */

    /* Check if there is a previous block to the block we are removing. */
    if (p_header->p_prevBlock == NULL)  {
        /* There is not a previous block, so make the next block the */
        /* start of the discard list. */
        P_startOfDiscardList = p_header->p_nextBlock ;
    } else {
        /* OK, there is a previous block.  Make that previous block */
        /* linked to the next block. */
        p_header->p_prevBlock->p_nextBlock = p_header->p_nextBlock ;
    }

    /* Check if there is a next block to the block we are removing. */
    if (p_header->p_nextBlock == NULL)  {
        /* There is not a next block, so make the previous block the */
        /* end of the discard list. */
        P_endOfDiscardList = p_header->p_nextBlock ;
    } else {
        /* OK, there is a next block.  Make that next block */
        /* linked to the previous block. */
        p_header->p_nextBlock->p_prevBlock = p_header->p_prevBlock ;
    }

    /* Mark this block as free from the discard list (and make */
    /* it look just like a normal block. */
    p_header->blockTag[0] = 'T' ;

    /* We are now removed from the discard double link list. */

#ifndef NDEBUG
/*
    p_data = (T_byte8 *)(p_header+1) ;
    for (i=0; i<p_header->size; i++)
       ((T_byte8 *)p_data)[i] ^= 0xFF ;
*/
#endif
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IMemFindFreeSpace
 *-------------------------------------------------------------------------*/
/**
 *  FindFreeSpace is the routine called by MemAlloc when it just tried
 *  to allocate new memory and could not.  This routine is called to free
 *  up space use by discarded memory.  If there is memory freed up, this
 *  routine returns with a TRUE, otherwise FALSE.  It does this by looking
 *  at the discard list and freeing up the oldest block.  It will only
 *  discard one block and then return with a TRUE status.
 *  Also, before the block is freed, it's callback routine is called
 *  to note the block's disappearance.
 *
 *  NOTE: 
 *  A future version might want to free up a certain amount of space
 *  instead of being called several times.  However, when you are running
 *  out of memory, this method is probably just as good.
 *
 *  @return TRUE = memory was freed
 *      FALSE = no more memory can be freed.
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IMemFindFreeSpace(T_void)
{
    E_Boolean answer = FALSE ;
    T_memBlockHeader *p_header ;
#if _MEM_CHECK_FULL_
    T_word16 pos ;
#endif

    DebugRoutine("IMemFindFreeSpace") ;

//printf("Finding free space!\n") ;
//fprintf(stderr, "Finding free space!") ;
MemCheck(2925) ;
//MemDumpDiscarded() ;
//_heapmin() ;

/*
printf("List:\n") ;
p_header = P_startOfDiscardList ;
while (p_header)  {
    printf("  %p\n", ((T_byte8 *)p_header)+sizeof(T_memBlockHeader)) ;
    p_header = p_header->p_nextBlock ;
}
printf("End of list\n\n") ;
*/
    /* Do we have any blocks on the discard list? */
    if (P_startOfDiscardList != NULL)  {
        /* Yes, call it's callback routine -- it is about to be */
        /* freed from memory.  Make sure to pass a pointer to the */
        /* true data part (past header) of the memory block. */
        p_header = P_startOfDiscardList ;

        /* Make sure you are freeing a discarded block. */
        DebugCheck(strcmp(p_header->blockTag, "DaG") == 0) ;

//printf("Discard %p\n", (((T_byte8 *)p_header)+sizeof(T_memBlockHeader))) ;
        p_header->p_callback(((T_byte8 *)p_header)+sizeof(T_memBlockHeader)) ;

        /* Make sure you are freeing a discarded block. */
        DebugCheck(strcmp(p_header->blockTag, "DaG") == 0) ;

        /* Now that the block's owner has been notified, we */
        /* can continue. */

        /* We first need to note the previous block that the next block */
        /* is disappearing. */
#if 0
        if (p_header->p_nextBlock != NULL)  {
            p_header->p_nextBlock->p_prevBlock = NULL ;

            /* If there is a previous block, make it the new end. */
            P_startOfDiscardList = p_header->p_nextBlock ;
        } else {
            /* If there is no previous block, the list is now empty. */
            P_endOfDiscardList =
                P_startOfDiscardList = NULL ;
        }
#endif
        P_startOfDiscardList = p_header->p_nextBlock ;
        if (P_startOfDiscardList == NULL)
            P_endOfDiscardList = NULL ;
        else
            p_header->p_nextBlock->p_prevBlock = NULL ;

        /* Note that the total is now less. */
        G_sizeAllocated -= sizeof(T_memBlockHeader) + p_header->size ;
#ifdef COMPILE_OPTION_OUTPUT_ALLOCATION
printf("!F %d %s\n", p_header->size, DebugGetCallerFile()) ;
printf("!F %d %s:%s\n", p_header->size, DebugGetCallerFile(), DebugGetCallerName()) ;
#endif

#ifndef NDEBUG
        /* Make sure we didn't roll under. */
        DebugCheck(G_sizeAllocated < 0xF0000000) ;
#endif

#ifdef _MEM_CHECK_FULL_
        /* Find the block on the list (if it is there). */
        pos = IFindPointer(p_header) ;

        if (pos == 0xFFFF)  {
            printf("bad attempt to free memory at %p\n", p_header+1) ;
            DebugCheck(FALSE) ;
        }
        /* Remove the block from the alloc list and put on the free list. */
        G_blockList[pos] = ((T_memBlockHeader *)((T_word32)G_firstFree)) ;

        G_firstFree = pos ;
#endif
#ifndef NDEBUG
        /* Mark the block as fully freed. */
        strcpy(p_header->blockTag, "!!!") ;

        /* Fill the block with junk. */
        memset(((T_byte8 *)p_header)+sizeof(T_memBlockHeader), 0xCE, p_header->size) ;
#endif
#ifndef NDEBUG
    memset(p_header, 0xCD, sizeof(T_memBlockHeader) + p_header->size) ;
#endif
        /* Ok, now we can actually free the block. */
        free(p_header) ;

        /* Note that memory *was* freed. */
        answer = TRUE ;
    }

    DebugCheck(answer < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;
    return answer ;
}

#ifndef NDEBUG

/*-------------------------------------------------------------------------*
 * Routine:  MemDumpDiscarded
 *-------------------------------------------------------------------------*/
/**
 *  MemDumpDiscarded is for debugging purposes only (and only will
 *  be compiled in debug mode).  It will output a list of blocks to the
 *  file "debugmem.txt".
 *
 *  NOTE: 
 *  None really.
 *
 *<!-----------------------------------------------------------------------*/
T_void MemDumpDiscarded(T_void)
{
#ifndef REAL_MODE
    FILE *fp ;
    T_memBlockHeader *p_header ;
//extern T_word32 G_packetsAlloc ;
//extern T_word32 G_packetsFree ;
struct _heapinfo h_info ;
int heap_status ;
    T_word32 totalUsed = 0 ;
    T_word32 totalFree = 0 ;

    DebugRoutine("MemDumpDiscarded") ;

printf("Dumping discard info!\n") ;
fflush(stdout) ;
MemCheck(991) ;
    fp = fopen("debugmem.txt", "w") ;
//fp = stdout ;
    DebugCheck(fp != NULL) ;

    fprintf(fp, "\n\nList of discarded memory:\n") ;
    fprintf(fp,     "-------------------------\n") ;
    p_header = P_startOfDiscardList ;
    fprintf(fp, "##### tag next: prev: callback?\n") ;
    fflush(fp) ;
    fprintf(fp, "start: %p  end: %p\n", P_startOfDiscardList, P_endOfDiscardList) ;
    fprintf(fp, "Start: %5d\n",
        (P_startOfDiscardList == NULL)?0:
            P_startOfDiscardList->blockId) ;
    fflush(fp) ;
    fprintf(fp, "  End: %5d\n",
        (P_endOfDiscardList == NULL)?0:
            P_endOfDiscardList->blockId) ;
    fprintf(fp, "\n\n") ;
    fflush(fp) ;

    while (p_header != NULL)  {
        fprintf(fp, "p_header = %p\n", p_header) ;
        fflush(fp) ;
        fprintf(fp, "%5d %s %5d %5d %c %s\n",
            p_header->blockId,
            p_header->blockTag,
            (p_header->p_nextBlock==NULL)?0:
                p_header->p_nextBlock->blockId,
            (p_header->p_prevBlock==NULL)?0:
                p_header->p_prevBlock->blockId,
            (p_header->p_callback == NULL)?'N':'Y',
#ifdef _MEM_RECORD_ROUTINES_
                p_header->routine) ;
#else
                "") ; //            p_header->routine) ;
#endif
        p_header = p_header->p_nextBlock ;
        fflush(fp) ;
    }
    fprintf(fp, "%d blocks allocated\n", G_allocCount-1) ;
    fprintf(fp, "%d blocks freed\n\n", G_deallocCount) ;
    fprintf(fp, "%d free memory\n", FreeMemory()) ;
    fflush(fp) ;
//    fprintf(fp, "%d packets alloc\n", G_packetsAlloc) ;
//    fprintf(fp, "%d packets free\n", G_packetsFree) ;

fprintf(fp, "\n\nHeap walk:\n\n") ;
h_info._pentry = NULL ;
for(;;)  {
  heap_status = _heapwalk(&h_info) ;
  if (heap_status != _HEAPOK)
    break ;
  fprintf(fp, "  %s %Fp %8.8X\n",
    (h_info._useflag == _USEDENTRY?"USED":"FREE"),
    h_info._pentry, h_info._size) ;
  if (h_info._useflag == _USEDENTRY)
      totalUsed += h_info._size ;
  else
      totalFree += h_info._size ;
}
fprintf(fp, "  heap status: %d\n", heap_status) ;
fprintf(fp, "  Total used: %ld bytes\n", totalUsed) ;
fprintf(fp, "  Total free: %ld bytes\n", totalFree) ;

    fflush(fp) ;
    fclose(fp) ;

    DebugEnd() ;
#endif
}

T_void MemCheck(T_word16 num)
{

//return ;  /* Don't do. */
//    DebugRoutine("MemCheck") ;

#ifdef _MEM_CHECK_FULL_
    ICheckAllocated(num) ;
#endif
//    status = _heapchk() ;



/*
    status = _heapset(0xCCCC) ;

    if (status != 0)  {
        printf("memory fail: %d for %d\n", status, num) ;
    }
    DebugCheck(status == 0) ;
*/


/* Extreme memory checking. */
/*
ptr = malloc(1000) ;
status = _heapchk() ;
if (status != 0)  {
    printf("memory fail after test alloc: %d for %d\n", status, num) ;
}
free(ptr) ;
*/

//    DebugEnd() ;
}

#ifdef _MEM_CHECK_FULL_
static T_word16 IFindPointer(T_memBlockHeader *p_ptr)
{
    T_word16 pos ;

    DebugRoutine("IFindPointer") ;

    for (pos=0; pos<G_numBlocks; pos++)
        if (p_ptr == G_blockList[pos])
            break ;

    if (pos == G_numBlocks)
        pos = 0xFFFF ;

    DebugEnd() ;

    return pos ;
}

static T_void ICheckAllocated(T_word16 noteNum)
{
    T_word16 pos ;
    T_memBlockHeader *p_header ;

    for (pos=0; pos<G_numBlocks; pos++)  {
        if (((T_word32)G_blockList[pos]) >= 0x10000)  {
            /* Must be a pointer */
            p_header = G_blockList[pos] ;
            if ((strcmp(p_header->blockTag, "TaG") != 0) &&
                (strcmp(p_header->blockTag, "DaG") != 0))    {
                printf("Bad tag for block header %p, note %d\n",
                    p_header, noteNum) ;
                printf("Entry %d of %d\n", pos, G_numBlocks) ;
                printf("Block ID: %d\n", p_header->blockId) ;
                printf("Size: %ld\n", p_header->size) ;
                printf("Tag: %4.4s\n", p_header->blockTag) ;
                printf("Caller: %s (line %d)\n", p_header->routine, p_header->line) ;
                fflush(stdout) ;
                DebugCheck(FALSE) ;
            }
            if (p_header->size > 100000000L)  {
                printf("Bad block size (%ld) for block header %p, note %d\n",
                    p_header->size, p_header, noteNum) ;
                printf("Entry %d of %d\n", pos, G_numBlocks) ;
                printf("Block ID: %d\n", p_header->blockId) ;
                printf("Size: %ld\n", p_header->size) ;
                printf("Tag: %4.4s\n", p_header->blockTag) ;
                printf("Caller: %s (line %d)\n", p_header->routine, p_header->line) ;
                fflush(stdout) ;
                DebugCheck(FALSE) ;
            }
            if (p_header->blockId > G_allocCount)  {
                printf("Bad block ID (%d) for block header %p, note %d\n",
                    p_header->blockId, p_header, noteNum) ;
                printf("Entry %d of %d\n", pos, G_numBlocks) ;
                printf("Block ID: %d\n", p_header->blockId) ;
                printf("Size: %ld\n", p_header->size) ;
                printf("Tag: %4.4s\n", p_header->blockTag) ;
                printf("Caller: %s (line %d)\n", p_header->routine, p_header->line) ;
                fflush(stdout) ;
                DebugCheck(FALSE) ;
            }
        } else {
            /* Must be a free block, skip it. */
        }
    }
}

#endif

T_void MemCheckData(T_void *p_data)
{
    T_memBlockHeader *p_header ;
    T_byte8 *p_bytes ;

    DebugRoutine("MemCheckData") ;
    DebugCheck(p_data != NULL) ;

    p_bytes = p_data ;
    p_bytes -= sizeof(T_memBlockHeader) ;
    p_header = (T_memBlockHeader *)p_bytes ;

    if (strcmp (p_header->blockTag, "TaG") != 0)  {
        printf("bad memory block at %p\n", p_data) ;
        printf("tag: %-4s\n", p_header->blockTag) ;
        printf("id: %ld\n", p_header->blockId) ;
        printf("next: %p\n", p_header->p_nextBlock) ;
        printf("prev: %p\n", p_header->p_prevBlock) ;
        printf("disc: %p\n", p_header->p_callback) ;
#ifdef _MEM_RECORD_ROUTINES_
        printf("rout: %s\n", p_header->routine) ;
        printf("line: %d\n", p_header->line) ;
#endif
        printf("size: %ld\n", p_header->size) ;
    }
    DebugCheck(strcmp (p_header->blockTag, "TaG") == 0) ;

    DebugEnd() ;
}

#endif

/*-------------------------------------------------------------------------*
 * Routine:  MemGetAllocated
 *-------------------------------------------------------------------------*/
/**
 *  MemGetAllocated returns the total amount of memory allocated.
 *
 *  NOTE: 
 *  None really.
 *
 *  @return Number bytes allocated
 *
 *<!-----------------------------------------------------------------------*/
T_word32 MemGetAllocated(T_void)
{
    return G_sizeAllocated ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MemGetMaxAllocated
 *-------------------------------------------------------------------------*/
/**
 *  MemGetMaxAllocated returns the maximum amount of memory that has been
 *  allocated at any one time.
 *
 *  NOTE: 
 *  None really.
 *
 *  @return Max bytes allocated since program
 *      started.
 *
 *<!-----------------------------------------------------------------------*/
T_word32 MemGetMaxAllocated(T_void)
{
    return G_maxSizeAllocated ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MemFlushDiscardable
 *-------------------------------------------------------------------------*/
/**
 *  MemFlushDiscardable goes through the list of discardable memory
 *  and frees it out.  This is useful for checking memory leaks or just
 *  wanting the system to reset to the beginning.
 *
 *  NOTE: 
 *  None really.
 *
 *<!-----------------------------------------------------------------------*/
T_void MemFlushDiscardable(T_void)
{
    DebugRoutine("MemFlushDiscardable") ;

//puts("Flush START") ;
    /* Keep discarding memory until we no longer have anything to */
    /* discard. */
    while (IMemFindFreeSpace() == TRUE)
        {}
//puts("Flush END") ;

/*
printf("Flushed List:\n") ;
p_header = P_startOfDiscardList ;
while (p_header)  {
    printf("  %p\n", ((T_byte8 *)p_header)+sizeof(T_memBlockHeader)) ;
    p_header = p_header->p_nextBlock ;
}
printf("End of list\n\n") ;
*/
    DebugEnd() ;
}

#ifdef BORLAND_COMPILER
#  ifdef REAL_MODE
T_word32 FreeMemory(T_void)
{
    return farcoreleft() ;
}
#  endif
#else
T_word32 FreeMemory(T_void)
{
#if defined(DOS32)
	T_word32 memInfo[12] ;

    union REGS regs ;
    struct SREGS sregs ;

    regs.x.eax = 0x500 ;
    memset(&sregs, 0, sizeof(sregs)) ;
    sregs.es = FP_SEG(memInfo) ;
    regs.x.edi = FP_OFF(memInfo) ;
    int386x(0x31, &regs, &regs, &sregs) ;

    return (memInfo[0] + _memavl()) ;
#else
    return 100000000; // TODO: don't really know
#endif
}

#endif

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  MEMORY.C
 *-------------------------------------------------------------------------*/
