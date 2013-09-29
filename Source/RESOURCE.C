/*-------------------------------------------------------------------------*
 * File:  RESOURCE.C
 *-------------------------------------------------------------------------*/
/**
 * Resource files are any .RES file that contains pictures, sounds,
 * animations etc.  Resources are also a bit sneakier because they are
 * locked in memory when needed and unlocked when not needed.  But they
 * are not discarded from memory immediately.  If memory becomes tight,
 * they are unloaded.  If memory does not become tight, on the next
 * lock, they are just marked as locked and are ready to use immediately.
 * Resources are loaded on an as needed basis.  A map load will typically
 * walk through all the resources desired and lock them in.
 *
 * @addtogroup RESOURCE
 * @brief Resource File System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include <ctype.h>
#include "MEMORY.H"
#include "RESOURCE.H"
#include "PICS.H"
#include "TICKER.H"

/* The following is done to ensure that the resource looks for its */
/* resources on the drive not just the resource. */

#ifdef NDEBUG
#    undef NDEBUG
#    define WAS_NDEBUG
#endif


#include "iresourc.h"

// Comment out the following define if you wish patches to be only read
// if in they do not exist in the resource file.
//#define RESOURCE_PATCHABLE

/* List of resouces available.  Initialize them on the free list. */
static T_resourceDirInfo G_resources[MAX_RESOURCE_FILES] = {
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 1},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 2},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 3},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 4},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 5},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 6},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 7},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 8},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, 9},
    { FILE_BAD, 0, NULL, RESOURCE_BAD, -1},
} ;

static T_byte8 G_resourceNames[MAX_RESOURCE_FILES][80] ;
static T_byte8 G_resourceCount[MAX_RESOURCE_FILES] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Keep track of who is on the free list of resource files. */
static T_sword16 G_firstFreeResourceFile = 0 ;

/* Keep track of the number of resource files open. */
static T_word16 G_numberOpenResourceFiles = 0 ;

/* Internal routine prototypes: */
static T_void IResourceMemCallback(void *p_block) ;
static T_void IResourceMemCallbackForDir(T_void *p_block) ;

static T_resource IPrimResourceFind(
                      T_resourceDirInfo *p_fileInfo,
                      T_byte8 *p_resourceName) ;

static T_resource IResourceFind(
               T_resourceDirInfo *p_dirInfo,
               T_byte8 *p_resourceName) ;

static T_void IPointAllToDir(
                  T_resourceDirInfo *p_dir,
                  T_resourceFile resourceFile) ;

static T_resourceDirInfo *IDirLock(T_resource dir) ;

static T_void IDirUnlock(T_resourceDirInfo *p_dir) ;

static T_resourceFile IFindOpenResource(T_byte8 *p_filename) ;

static T_void IDiscardEntries(T_resourceEntry *p_entry, T_word16 number) ;

#ifndef NDEBUG
static T_void ICheckDirEntries(T_resourceDirInfo *p_dir) ;
typedef struct T_loadLinkTag {
    char name[80] ;
    struct T_loadLinkTag *p_prev ;
    struct T_loadLinkTag *p_next ;
    T_resourceEntry *p_entry ;
} ;
typedef struct T_loadLinkTag T_loadLink ;
static T_loadLink *G_loadedEntries = NULL ;
#else
#define ICheckDirEntries(a)
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ResourceOpen
 *-------------------------------------------------------------------------*/
/**
 *  Open a resource file for locking and unlocking of blocks.  The
 *  resource file allows you to have several "files" grouped together into
 *  one real world file.  In addition, it also makes sure that blocks
 *  stay in memory as long as possible (based on the computer's available
 *  memory).
 *
 *  NOTE: 
 *  The biggest problem is not in the opening file, it is how the items
 *  are ordered in the resource.  All names used in a resource should
 *  alphabetical, or else searches for items may fail.
 *
 *  @param filename -- Name of resource file (may include
 *      the path to the file).
 *
 *  @return handle to resource block.
 *
 *<!-----------------------------------------------------------------------*/
T_resourceFile ResourceOpen(T_byte8 *filename)
{
    T_resourceFile resourceFile ;
    T_file file ;
    T_resourceFileHeader resourceHeader ;
    T_resourceEntry *p_index ;
    T_resourceDirInfo *p_info ;
    T_word16 i ;

    DebugRoutine("ResourceOpen") ;
    DebugCheck(filename != NULL) ;
    DebugCheck(G_numberOpenResourceFiles < MAX_RESOURCE_FILES) ;
    DebugCheck(G_firstFreeResourceFile != -1) ;

    resourceFile = IFindOpenResource(filename) ;
    if (resourceFile == RESOURCE_FILE_BAD)  {
        /* Go ahead and increment the number of resource files now open. */
        G_numberOpenResourceFiles++ ;

        /* Get a free resource file. */
        resourceFile = G_firstFreeResourceFile ;

        /* Store the filename so that future opens use this name. */
        strcpy(G_resourceNames[resourceFile], filename) ;

        /* Update the free resource file list so it has one less. */
        G_firstFreeResourceFile = G_resources[G_firstFreeResourceFile].nextResource ;

        /* Now open the file for the resource. */
        file = FileOpen(filename, FILE_MODE_READ) ;
        DebugCheck(file != FILE_BAD) ;

        /* Seek the beginning just in case we're not already there. */
        FileSeek(file, 0) ;

        /* Now read in the resource file header. */
        FileRead(file, &resourceHeader, sizeof(T_resourceFileHeader)) ;

        /* Check for the correct id */
        DebugCheck(resourceHeader.uniqueID == RESOURCE_FILE_UNIQUE_ID) ;

        /* Now allocate enough memory for the index. */
        p_index = MemAlloc(resourceHeader.indexSize) ;
        DebugCheck(p_index != NULL) ;

        /* Read in the index after positioning the read position. */
        FileSeek(file, resourceHeader.indexOffset) ;
        FileRead(file, p_index, resourceHeader.indexSize) ;

        /* Go through and mark all entries as from this file. */
        /* This helps with locking the blocks and not having to pass */
        /* a file handle all over the place.  (And since we open only */
        /* once for each resource file, there isn't much overhead). */
        for (i=0; i<resourceHeader.numEntries; i++)
            p_index[i].resourceFile = resourceFile ;

        /* Record all this information in the resource file list. */
        p_info = &G_resources[resourceFile] ;
        p_info->fileHandle = file ;
        p_info->numberEntries = resourceHeader.numEntries ;
        p_info->ownerRes = RESOURCE_BAD ;
        p_info->p_entries = p_index ;
        p_info->nextResource = -1 ;

        IPointAllToDir(p_info, resourceFile) ;
    }
    G_resourceCount[resourceFile]++ ;

    DebugCheck(resourceFile < MAX_RESOURCE_FILES) ;
    DebugEnd() ;

    /* Return the index into the resource list. */
    return resourceFile ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceClose
 *-------------------------------------------------------------------------*/
/**
 *  ResourceClose is used to close out the resource and remove all the
 *  blocks in memory.  You cannot call this routine on a resource that
 *  still has locked resource blocks, but you can call it on a resource
 *  that has discarded blocks.
 *
 *  @param resourceFile -- handle to resource file to close
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourceClose(T_resourceFile resourceFile)
{
    DebugRoutine("ResourceClose") ;
    DebugCheck(resourceFile < MAX_RESOURCE_FILES) ;
    DebugCheck(G_resources[resourceFile].fileHandle != FILE_BAD) ;

    /* Before we can close the resource file, we need to release */
    /* all discarded blocks from memory.  We start by looping through */
    /* all the resource blocks looking for discarded blocks. */

    /* Decrement the number of times the file is open. */
    G_resourceCount[resourceFile]-- ;

    /* See if the file has been closed enough to close it fully. */
    if (G_resourceCount[resourceFile] == 0)  {
//printf("--------------------- Resource Close: %s\n", G_resourceNames[resourceFile]) ;
        /* Get a pointer to the entry list. */
#ifndef NDEBUG
//        ResourceDumpIndex(resourceFile) ;
#endif
        ICheckDirEntries(&G_resources[resourceFile]) ;

        IDiscardEntries(
            G_resources[resourceFile].p_entries,
            G_resources[resourceFile].numberEntries) ;

        /* Now all the blocks have been freed when we get to here. */
        /* Close the resource file, free the index, and put this file */
        /* resource on the free list. */
        FileClose(G_resources[resourceFile].fileHandle) ;

        /* Make file as no longer valid. */
        G_resources[resourceFile].fileHandle = FILE_BAD ;

        /* Free index. */
        MemFree(G_resources[resourceFile].p_entries) ;
        G_resources[resourceFile].p_entries = NULL ;

        /* Decrement number of open resource files. */
        G_numberOpenResourceFiles-- ;

        /* Make this resource point to be beginning of the free list. */
        G_resources[resourceFile].nextResource = G_firstFreeResourceFile ;

        /* The name is now no longer valid. */
        G_resourceNames[resourceFile][0] = '\0' ;

        /* Make this the first free resource file. */
        G_firstFreeResourceFile = resourceFile ;
    }

    DebugEnd() ;
}

#ifndef NDEBUG
T_byte8 *JustEnd(T_byte8 *p_string)
{
    T_byte8 *p ;

    p = p_string + strlen(p_string) - 1 ;
    while (p != p_string)  {
        if (*p == '/')  {
            p++ ;
            break ;
        }
        p-- ;
    }

    return p ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ResourceFind
 *-------------------------------------------------------------------------*/
/**
 *  ResourceFind is used to find the index into the current resource
 *  file for a particular resource block (referenced by the given name).
 *  You use the returned resource handle to lock and unlock the resource.
 *  (Unlocking the resource will give you a pointer to where it is located).
 *
 *  NOTE: 
 *  Speed is the first problem.  Calling functions should try to use
 *  this routine rarely.
 *  Second, the resource file MUST have it's entries in alpabetical
 *  order.
 *
 *  @param resourceFile -- handle to resource file
 *  @param p_resourceName -- Pointer to resource block name
 *
 *  @return Handle to resource that is found,
 *      or returns RESOURCE_BAD.
 *
 *<!-----------------------------------------------------------------------*/
T_resource ResourceFind(
               T_resourceFile resourceFile,
               T_byte8 *p_resourceName)
{
    T_resource res ;
    T_resourceEntry *p_entry ;
    T_byte8 *p_data ;
    T_word32 size ;
#ifndef NDEBUG
    T_loadLink *p_loadLink ;
#endif

    DebugRoutine("ResourceFind") ;

//printf("ResourceFind: %s\n", p_resourceName) ;
//if (!isdigit(JustEnd(p_resourceName)[0]))
#ifdef RESOURCE_OUTPUT
printf("!A 1 find_%s\n", JustEnd(p_resourceName));
#endif

#ifndef RESOURCE_PATCHABLE
DebugRoutine("f") ;
    res = IResourceFind(&G_resources[resourceFile], p_resourceName) ;
DebugEnd() ;
    if (res == RESOURCE_BAD)  {
#endif
#ifndef NDEBUG
//printf("Loading %s\n", p_resourceName) ;
    if (FileExist(p_resourceName))  {
        p_loadLink = G_loadedEntries ;
        while (p_loadLink)  {
            if (strcmp(p_resourceName, p_loadLink->name) == 0)
                break ;
            p_loadLink = p_loadLink->p_next ;
        }

        if (!p_loadLink)  {
            p_data = FileLoad(p_resourceName, &size) ;
#ifdef RESOURCE_OUTPUT
printf("!A 1 file_%s\n", JustEnd(p_resourceName));
#endif
            p_entry = MemAlloc(sizeof(T_resourceEntry)) ;
            strcpy(p_entry->resID, "ReS") ;
            strcpy(p_entry->p_resourceName, "Loaded.") ;
            p_entry->fileOffset = 0 ;
            p_entry->p_data = p_data ;
            p_entry->size = size ;
            p_entry->lockCount = 1 ;
            p_entry->resourceType = RESOURCE_ENTRY_TYPE_FILE |
                                    RESOURCE_ENTRY_TYPE_MEMORY |
                                    RESOURCE_ENTRY_TYPE_FILE_LOADED ;
            p_entry->resourceFile = resourceFile ;
            p_entry->ownerDir =
                (T_void *)(p_loadLink =
                    (T_loadLink *)MemAlloc(sizeof(T_loadLink))) ;
            strcpy(p_loadLink->name, p_resourceName) ;
//printf("p_loadLink = %p\n", p_loadLink) ;
//printf("G_loadedEntries = %p\n", G_loadedEntries) ;
            p_loadLink->p_prev = NULL ;
            p_loadLink->p_next = G_loadedEntries ;
            p_loadLink->p_entry = p_entry ;
            if (G_loadedEntries != NULL)
                G_loadedEntries->p_prev = p_loadLink ;
            G_loadedEntries = p_loadLink ;
        } else {
            p_entry = p_loadLink->p_entry ;
            p_entry->lockCount++ ;
        }

        res = (T_resource)p_entry ;
    } else {
        res = IResourceFind(&G_resources[resourceFile], p_resourceName) ;
    }
#else
    /* Standard non-debug version only uses resource files. */
    res = IResourceFind(&G_resources[resourceFile], p_resourceName) ;
#endif

#ifndef RESOURCE_PATCHABLE
    }
#endif

    DebugEnd() ;

    return res ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceLock
 *-------------------------------------------------------------------------*/
/**
 *  If you need to get a resource, you should call ResourceLock first.
 *  If you don't already have a resource handle, you need to call
 *  ResourceFind first.  You then can call this routine to "lock" the block
 *  at a certain memory location.  This routine will take care of all
 *  cases of where the block may originally be (on disk, in memory, or
 *  discarded).
 *
 *  NOTE: 
 *  Not a whole lot.
 *
 *  @param resource -- handle to resource as returned by
 *      ResourceFind()
 *
 *  @return Pointer to memory block that is
 *      locked.
 *
 *<!-----------------------------------------------------------------------*/
T_void *ResourceLock(T_resource resource)
{
    T_resourceEntry *p_resource ;
    T_file file ;

    DebugRoutine("ResourceLock") ;
    DebugCheck(resource != RESOURCE_BAD) ;
    p_resource = resource ;
    DebugCheck(strcmp(p_resource->resID, "ReS")==0) ;
    DebugCheck(p_resource->resourceType < RESOURCE_ENTRY_TYPE_UNKNOWN) ;

    /* Depending on what type of resource this is, do the appropriate */
    /* locking action. */
//printf("ResourceLock: %s\n", p_resource->p_resourceName) ;
    switch(p_resource->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)  {
        case RESOURCE_ENTRY_TYPE_MEMORY:
            /* Do nothing if it is already in memory. */
//printf("  already in memory.\n") ;
            /* But for a quick check, make sure the lock count is */
            /* NOT 0.  A discardable block has a lock count of 0 */
            DebugCheck(p_resource->lockCount != 0) ;
            break ;
        case RESOURCE_ENTRY_TYPE_DISK:
            TickerPause() ;
//printf("  readding off the resource file\n") ;
            /* Resource is not loaded.  We will need to load it */
            /* into memory. */

            /* Start by allocating a spot for this resource. */
            p_resource->p_data = MemAlloc(p_resource->size+sizeof(T_resourceEntry *)) ;
            DebugCheck(p_resource->p_data) ;

            /* So we can have a backward reference to the resource handle */
            /* when given the data, we will store a prefix pointer back */
            /* to the reference handle.  However, we will return only a */
            /* pointer to the actual data. */
            *((T_resourceEntry **)(p_resource->p_data)) = p_resource ;
//printf("Reading in resource entry '%s'\n", p_resource->p_resourceName) ;
#ifdef RESOURCE_OUTPUT
printf("!A %ld lock_%s\n", p_resource->size, p_resource->p_resourceName) ;
#endif
            /* skip past the entry reference. */
            p_resource->p_data += sizeof(T_resourceEntry *) ;

            /* Then get the file location of where the resource is */
            /* stored. */
            file = G_resources[p_resource->resourceFile].fileHandle ;
            DebugCheck(file != FILE_BAD) ;

            FileSeek(file, p_resource->fileOffset) ;

            /* Read it in. */
            FileRead(file, p_resource->p_data, p_resource->size) ;
            /* Viola! Done.  Mark it now as being in memory. */
            p_resource->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
            p_resource->resourceType |= RESOURCE_ENTRY_TYPE_MEMORY ;

            TickerContinue() ;
            break ;
        case RESOURCE_ENTRY_TYPE_DISCARDED:
            /* The resource is in memory, but it has been declared */
            /* as discardable.  We need to get it off the discard */
            /* list and put it back into "normal" memory. */
//printf("  reclaiming resource\n") ;

            /* Reclaim the block. */
            MemReclaimDiscardable(((T_byte8 *)p_resource->p_data) -
                sizeof(T_resourceEntry *)) ;

            /* Note that the block is now in regular memory. */
            p_resource->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
            p_resource->resourceType |= RESOURCE_ENTRY_TYPE_MEMORY ;
            break ;
    }

    /* Now in memory.  Increment the lock count. */
    p_resource->lockCount++ ;

    /* Make sure the lock count didn't roll over. */
    DebugCheck(p_resource->lockCount != 0) ;

    DebugCheck(p_resource->p_data != NULL) ;
    DebugEnd() ;

    /* Return the pointer to data part (now that it is locked) */
    return (p_resource->p_data) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceUnlock
 *-------------------------------------------------------------------------*/
/**
 *  Unlock a resource that is in memory.  This doesn't actually destory
 *  the resource.  Instead the resource goes on the discardable memory
 *  list that allows it to be destroyed when more memory is needed.
 *  ResourceLock later to get the block back (or load from disk).
 *  Note, however, that if a resource has been multiply locked, there
 *  has to be an equal number of unlocks to allow the block to be
 *  discarded.
 *
 *  @param resource -- Resource you no longer need.
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourceUnlock(T_resource resource)
{
    T_resourceEntry *p_resource ;

    DebugRoutine("ResourceUnlock") ;
    DebugCheck(resource != RESOURCE_BAD) ;
    p_resource = resource ;
//printf("Unlocking resource %p\n", p_resource) ;
//ResourcePrint(stdout, resource) ;
#ifndef NDEBUG
if (strcmp(p_resource->resID, "ReS")!=0)  {
  ResourcePrint(stdout, resource) ;
}
#endif
    DebugCheck(strcmp(p_resource->resID, "ReS")==0) ;
    DebugCheck((p_resource->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) == RESOURCE_ENTRY_TYPE_MEMORY) ;
    DebugCheck(p_resource->lockCount > 0) ;

    /* Decrement the lock count on this resource. */
    p_resource->lockCount-- ;

    /* Check to see if the resource can be discarded. */
    if (p_resource->lockCount == 0)  {
        /* Yes, the block is no longer locked.  Let's make it discardable. */
        /* Also pass it a callback routine to allow us to finalize */
        /* the block entry when it is actually removed from memory. */
        /* You might recall that when we loaded the block from disk, */
        /* we added a pointer back to the resource table entry. */
        /* The callback routine will use this to mark the block as */
        /* no longer being in memory. */
//printf("resource unlock %s memmarkdiscard %p for %s\n", p_resource->p_resourceName, p_resource->p_data, DebugGetCallerName()) ;  fflush(stdout) ;
        MemMarkDiscardable(
            ((T_byte8 *)p_resource->p_data)-sizeof(T_resourceEntry *),
            IResourceMemCallback) ;
        p_resource->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
        p_resource->resourceType |= RESOURCE_ENTRY_TYPE_DISCARDED ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceGetSize
 *-------------------------------------------------------------------------*/
/**
 *  ResourceGetSize returns the size of the given resource.
 *
 *  @param resource -- Resource to get the size of
 *
 *  @return Size of resource
 *
 *<!-----------------------------------------------------------------------*/
T_word32 ResourceGetSize(T_resource resource)
{
    T_word32 size = 0;
    T_resourceEntry *p_resource ;

    DebugRoutine("ResourceGetSize") ;
    DebugCheck(resource != RESOURCE_BAD) ;
    p_resource = resource ;
    DebugCheck(strcmp(p_resource->resID, "ReS")==0) ;
    DebugCheck(p_resource->resourceType < RESOURCE_ENTRY_TYPE_UNKNOWN) ;

    size = p_resource->size ;

    DebugEnd() ;

    return size ;
}

/*** Internal functions to this module:                                   ***/


/*-------------------------------------------------------------------------*
 * Routine:  IResourceMemCallback
 *-------------------------------------------------------------------------*/
/**
 *  IResourceMemCallback is called for each discardable resource that
 *  is being removed from memory by the Memory Module.  This routine does
 *  not do the freeing of memory, but does mark the block in the resource
 *  index table as no longer in memory.  This is VERY important, since
 *  the Resource Manager needs a way to know when the resources are
 *  gone.
 *
 *  NOTE: 
 *  Can't think of any.
 *
 *  @param p_block -- Pointer to block being removed
 *
 *<!-----------------------------------------------------------------------*/
static T_void IResourceMemCallback(T_void *p_block)
{
    T_resourceEntry *p_entry ;

    DebugRoutine("IResourceMemCallback") ;
    DebugCheck(p_block != NULL) ;

    /* We don't actually need the data that we are receiving so much as */
    /* we need the data that we stored in front of this block.  Let's */
    /* start by getting that piece of informatin. */

//    p_entry = *((T_resourceEntry **)(((T_byte8 *)p_block)
//                                      -sizeof(T_resourceEntry *))) ;
    p_entry = *((T_resourceEntry **)p_block) ;

    /* Ok, just mark the entry as being no longer in memory (and our */
    /* only real choice is to say it is on disk, where it came from). */
#ifndef NDEBUG
/*
if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) != RESOURCE_ENTRY_TYPE_DISCARDED)  {
    fp = fopen("resstuff.txt", "a") ;
    fprintf(fp, "id: %-3s\nname: %-14s\noff: %d\nsize: %d\nlock: %d\ntype: %d\n",
                    p_entry->resID,
                    p_entry->p_resourceName,
                    p_entry->fileOffset,
                    p_entry->size,
                    p_entry->lockCount,
                    p_entry->resourceType) ;
    fclose(fp) ;
}
*/
#endif
//    DebugCheck(strcmp(p_entry->resID, "ReS")==0) ;
    DebugCheck((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) == RESOURCE_ENTRY_TYPE_DISCARDED) ;
    p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
    p_entry->resourceType |= RESOURCE_ENTRY_TYPE_DISK ;
    p_entry->p_data = NULL ;
#ifdef RESOURCE_OUTPUT
printf("!F %ld lock_%s\n", p_entry->size, p_entry->p_resourceName) ;
#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IPrimResourceFind
 *-------------------------------------------------------------------------*/
/**
 *  IPrimResourceFind is used to find the index into the current resource
 *  file for a particular resource block (referenced by the given name).
 *  You use the returned resource handle to lock and unlock the resource.
 *  (Unlocking the resource will give you a pointer to where it is located).
 *
 *  NOTE: 
 *  Speed is the first problem.  Calling functions should try to use
 *  this routine rarely.
 *  Second, the resource file MUST have it's entries in alpabetical
 *  order.
 *
 *  @param resourceFile -- handle to resource file
 *  @param p_resourceName -- Pointer to resource block name
 *
 *  @return Handle to resource that is found,
 *      or returns RESOURCE_BAD.
 *
 *<!-----------------------------------------------------------------------*/
static T_resource IPrimResourceFind(
                      T_resourceDirInfo *p_fileInfo,
                      T_byte8 *p_resourceName)
{
    T_resourceEntry *p_index ;
    T_word16 searchMin, searchMax, search ;
    T_sbyte8 compare ;
    T_resource resource = RESOURCE_BAD ;

    DebugRoutine("IPrimResourceFind") ;
    DebugCheck(p_resourceName != NULL) ;
    DebugCheck(p_fileInfo->fileHandle != FILE_BAD) ;
    DebugCheck(p_fileInfo->p_entries != NULL) ;

    /* We will need to first get a pointer to the entries and get */
    /* the number of entries in this list. */
    p_index = p_fileInfo->p_entries ;
    searchMax = p_fileInfo->numberEntries-1 ;
    searchMin = 0 ;

    while (searchMin <= searchMax) {
        /* Look at a point in between the min and max */
        search = (searchMax + searchMin)>>1 ;

        /* Compare the names (case sensitive) */
        compare = strcmp(p_resourceName, p_index[search].p_resourceName) ;
        if (compare == 0)
            break ;
        if (compare < 0)  {
            /* Must be in upper half of min/max */
            /* Move up the max to one before this one. */
            searchMax = search-1 ;
        } else {
            /* Must be in lwoer half of min/max */
            /* Move min down to one after this one. */
            searchMin = search+1 ;
        }
    }

    /* Check to see if the above loop was canceled due to min */
    /* and max overlapping. */
    if (searchMin <= searchMax)  {
        /* Since min and max don't overlap, we must of found a match. */
        /* Let the resource equal a pointer to the entry in the index. */
        resource = &p_index[search] ;
    }

    DebugEnd() ;
    return resource ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDirLock
 *-------------------------------------------------------------------------*/
/**
 *  IDirLock processes a resource handle and locks the appropriate
 *  directory structure for all future accesses to that directory.
 *
 *  NOTE: 
 *  Speed is the first problem.  Calling functions should try to use
 *  this routine rarely.
 *  Second, the resource dir. MUST have it's entries in alpabetical
 *  order.
 *
 *  @param dir -- handle to resource directory
 *
 *  @return Directory structure found (if found).
 *
 *<!-----------------------------------------------------------------------*/
static T_resourceDirInfo *IDirLock(T_resource dir)
{
    T_resourceFileHeader header ;
    T_resourceDirInfo *p_dir = NULL ;
    T_resourceEntry *p_entry ;
    T_file file ;

    DebugRoutine("IDirLock") ;

    p_entry = (T_resourceEntry *)dir ;
//printf("DirLock: %s\n", p_entry->p_resourceName) ;
//printf("!A 1 Dir:%s\n", p_entry->p_resourceName) ;

    /* Make sure we have a directory entry. */
    DebugCheck ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
                RESOURCE_ENTRY_TYPE_DIRECTORY) ;
    if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
        RESOURCE_ENTRY_TYPE_DIRECTORY)  {
        /* Is the directory already locked? */
        if (p_entry->lockCount == 0)  {
            /* No, it is not. */
            /* Is it discarded? */
            if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) ==
                RESOURCE_ENTRY_TYPE_DISCARDED)  {
//printf("Reclaiming directory %s\n", p_entry->p_resourceName) ;
//printf("!A 1 dir_%s\n", p_entry->p_resourceName) ;
                /* Reclaim the block. */
                p_dir = (T_resourceDirInfo *)p_entry->p_data ;
//printf("!F 1 disdir_%p\n", p_dir) ;  fflush(stdout) ;
                MemReclaimDiscardable(p_dir) ;

                p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
                p_entry->resourceType |= RESOURCE_ENTRY_TYPE_MEMORY ;
            } else {
                /* Not discarded.  Need to retrieve from disk. */
                /* Allocate memory for the directory info structure. */
                p_dir = MemAlloc(sizeof(T_resourceDirInfo)) ;

                DebugCheck(p_dir != NULL) ;
                if (p_dir != NULL)  {
                    file = G_resources[p_entry->resourceFile].fileHandle ;
                    /* If that went ok, read in the directory/file header. */
                    FileSeek(file, p_entry->fileOffset) ;
                    FileRead(
                        file,
                        &header,
                        sizeof(header)) ;

                    /* This better be correct. */
                    DebugCheck(header.uniqueID == RESOURCE_FILE_UNIQUE_ID) ;

                    /* Copy over info about the directory from the */
                    /* file or subdirectory. */
                    p_dir->ownerRes = dir ;
                    p_dir->fileHandle = file ;
                    p_dir->numberEntries = header.numEntries ;
                    p_dir->nextResource = -1 ;

                    /* Allocate space for this directory. */
                    p_dir->p_entries = MemAlloc(header.indexSize) ;
                    DebugCheck(p_dir->p_entries != NULL) ;

                    /* Find the file/directory's directory listing. */
                    FileSeek(file, header.indexOffset) ;

                    /* Read in the directory. */
                    FileRead(file, p_dir->p_entries, header.indexSize) ;

                    /* Ok, fix up the directory entries so that */
                    /* when we unlock the items, we have something to */
                    /* back track. */
                    IPointAllToDir(p_dir, p_entry->resourceFile) ;

                    /* Store the directory as the entry for the data. */
                    p_entry->p_data = (T_byte8 *)p_dir ;
                }
            }
            /* Note that the directory is in memory. */
            p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
            p_entry->resourceType |= RESOURCE_ENTRY_TYPE_MEMORY ;
        } else {
            /* Retrieve where this sub-directory can be found. */
            p_dir = (T_resourceDirInfo *)p_entry->p_data ;
        }

        /* Increment its reference count. */
        p_entry->lockCount++ ;
    }

    DebugCheck(p_dir != NULL) ;
    DebugEnd() ;

    return p_dir ;
}

#ifndef NDEBUG
#ifndef WAS_NDEBUG

/*-------------------------------------------------------------------------*
 * Routine:  ResourceDumpIndex
 *-------------------------------------------------------------------------*/
/**
 *  For debuggine purposes only, this can be used to dump out
 *  a list of indexes for a particular resource file.  Places list
 *  in external file, "RESOURDB.TXT"
 *
 *  @param fp -- file to scan through index.
 *  @param p_entry -- Starting entry in resource
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourceDumpDir(FILE *fp, T_resourceEntry *p_entry)
{
    T_resourceEntry *p_index ;
    T_word16 i ;
    T_resourceDirInfo *p_dir = NULL ;
    T_word16 num ;

    p_dir = (T_resourceDirInfo *)p_entry->p_data ;
    p_index = p_dir->p_entries ;
    num = p_dir->numberEntries ;

    fprintf(fp, "\n\nResource dump for %s:\n------------\n", p_entry->p_resourceName) ;
    fprintf(fp, "id:  Name:          Offset:  Size:    Lck T Pointer: File:\n");
    fprintf(fp, "---- -------------- -------- -------- --- - -------- -----\n") ;
    for (i=0; i<num; i++)  {
        fprintf(fp, "%-4s %-14s %8d %8d %3d %c %p %d\n",
            p_index[i].resID,
            p_index[i].p_resourceName,
            p_index[i].fileOffset,
            p_index[i].size,
            p_index[i].lockCount,
            ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                     == RESOURCE_ENTRY_TYPE_MEMORY)?'M':
                (((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                         ==RESOURCE_ENTRY_TYPE_DISK)?'D':
                    (((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                          ==RESOURCE_ENTRY_TYPE_DISCARDED)?'d':'?')),
            p_index[i].p_data,
            p_index[i].resourceFile) ;
    }

    /* Print out each subdir for each directory loaded in memory */
    for (i=0; i<num; i++)  {
        if ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) ==
                RESOURCE_ENTRY_TYPE_MEMORY)  {
            if ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
                RESOURCE_ENTRY_TYPE_DIRECTORY)  {
                ResourceDumpDir(fp, &p_index[i]) ;
            }
        }
    }
}

T_void ResourceDumpIndex(T_resourceFile resourceFile)
{
    T_resourceEntry *p_index ;
    T_word16 i ;
    FILE *fp ;

    DebugRoutine("ResourceDumpIndex") ;
    DebugCheck(resourceFile < MAX_RESOURCE_FILES) ;

    p_index = G_resources[resourceFile].p_entries ;
    DebugCheck(p_index != NULL) ;

    fp = fopen("resourdb.txt", "w") ;
    DebugCheck(fp != NULL) ;

    fprintf(fp, "Resource dump:\n------------\n") ;
    fprintf(fp, "id:  Name:          Offset:  Size:    Lck T Pointer: File:\n");
    fprintf(fp, "---- -------------- -------- -------- --- - -------- -----\n") ;
    for (i=0; i<G_resources[resourceFile].numberEntries; i++)  {
        fprintf(fp, "%-4s %-14s %8d %8d %3d %c %p %d\n",
            p_index[i].resID,
            p_index[i].p_resourceName,
            p_index[i].fileOffset,
            p_index[i].size,
            p_index[i].lockCount,
            ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                     == RESOURCE_ENTRY_TYPE_MEMORY)?'M':
                (((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                         ==RESOURCE_ENTRY_TYPE_DISK)?'D':
                    (((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)
                          ==RESOURCE_ENTRY_TYPE_DISCARDED)?'d':'?')),
            p_index[i].p_data,
            p_index[i].resourceFile) ;
    }

    /* Print out each subdir for each directory loaded in memory */
    for (i=0; i<G_resources[resourceFile].numberEntries; i++)  {
        if ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) ==
                RESOURCE_ENTRY_TYPE_MEMORY)  {
            if ((p_index[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
                RESOURCE_ENTRY_TYPE_DIRECTORY)  {
                ResourceDumpDir(fp, &p_index[i]) ;
            }
        }
    }

    fclose(fp) ;

    DebugEnd() ;
}

#endif
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IPointAllToDir
 *-------------------------------------------------------------------------*/
/**
 *  IPointAllToDir makes all the entries in a directory point to the
 *  directory entry for that directory.
 *
 *  NOTE: 
 *  None
 *
 *  @param p_dir -- Directory to fix all pointers.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IPointAllToDir(
                  T_resourceDirInfo *p_dir,
                  T_resourceFile resourceFile)
{
    T_word16 num ;
    T_word16 i ;
    DebugRoutine("IPointAllToDir") ;

    DebugCheck(p_dir != NULL) ;

    num = p_dir->numberEntries ;
    for (i=0; i<num; i++)  {
        p_dir->p_entries[i].ownerDir = p_dir ;
        p_dir->p_entries[i].resourceFile = resourceFile ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IResourceFind
 *-------------------------------------------------------------------------*/
/**
 *  IResourceFind is used to find the index into the given directory or
 *  its subdirectory for a particular resource block (referenced by the
 *  given name).  You use the returned resource handle to lock and unlock
 *  the resource.  (Unlocking the resource will give you a pointer to where
 *  it is located).
 *
 *  NOTE: 
 *  Speed is the first problem.  Calling functions should try to use
 *  this routine rarely.
 *  Second, the resource file MUST have it's entries in alpabetical
 *  order.
 *
 *  @param resourceFile -- handle to resource file
 *  @param p_resourceName -- Pointer to resource block name
 *
 *  @return Handle to resource that is found,
 *      or returns RESOURCE_BAD.
 *
 *<!-----------------------------------------------------------------------*/
static T_resource IResourceFind(
               T_resourceDirInfo *p_dirInfo,
               T_byte8 *p_resourceName)
{
    T_resourceEntry *p_index ;
    T_sword16 searchMin, searchMax, search ;
    T_sbyte8 compare ;
    T_resource resource ;
    E_Boolean slashFound ;
    T_word16 i ;
    T_byte8 subName[20] ;

    DebugRoutine("IResourceFind") ;
    DebugCheck(p_resourceName != NULL) ;

    do {
        /* We first must extract a name into subName. */
        /* If there is a slash at the beginning, skip it ... not needed. */
        if (*p_resourceName == '/')
            p_resourceName++ ;

        /* See if there is another slash in the string. */
        for (slashFound = FALSE, i=0; p_resourceName[i] != '\0'; i++)  {
            subName[i] = p_resourceName[i] ;
            if (p_resourceName[i] == '/')  {
                slashFound = TRUE ;
                break ;
            }
        }
        subName[i] = '\0' ;
        p_resourceName += i ;
        /* Let's search for it! */

        /* We will need to first get a pointer to the entries and get */
        /* the number of entries in this list. */
        resource = RESOURCE_BAD ;
        p_index = p_dirInfo->p_entries ;
        searchMax = p_dirInfo->numberEntries-1 ;
        searchMin = 0 ;

        while (searchMin <= searchMax) {
            /* Look at a point in between the min and max */
            search = (searchMax + searchMin)>>1 ;
            /* Compare the names (case sensitive) */
            compare = strcmp(subName, p_index[search].p_resourceName) ;
            if (compare == 0)
                break ;
            if (compare < 0)  {
                /* Must be in upper half of min/max */
                /* Move up the max to one before this one. */
                searchMax = search-1 ;
            } else {
                /* Must be in lwoer half of min/max */
                /* Move min down to one after this one. */
                searchMin = search+1 ;
            }
        }

        /* Check to see if the above loop was canceled due to min */
        /* and max overlapping. */
        if (searchMin <= searchMax)  {
            /* Since min and max don't overlap, we must of found a match. */
            /* Let the resource equal a pointer to the entry in the index. */
            resource = &p_index[search] ;
        } else {
            /* We didn't find a match.  Stop looping.  Return a bad type. */
            resource = RESOURCE_BAD ;
            break ;
        }

        /* Are we supposed to be a sub directory or what? */
        if (slashFound == TRUE)  {
            /* If so, lock that sub-directory into memory */
            /* and get the directory pointer. */
            p_dirInfo = IDirLock(resource) ;

            /* Make sure that went well. */
            DebugCheck(p_dirInfo != NULL) ;
            if (p_dirInfo == NULL)
                break ;
        }

        /* Loop while we are going into directories. */
    } while (slashFound == TRUE) ;

    if ((p_dirInfo != NULL) && (resource == NULL))  {
        /* Didn't find the resource, but we need to unlock */
        /* the directories. */
        IDirUnlock(p_dirInfo) ;
    }
    DebugEnd() ;

    return resource ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceUnfind
 *-------------------------------------------------------------------------*/
/**
 *  ResourceUnfind is called after a resource has been used and no longer
 *  needs to be valid.  In effect, this routine closes out any items that
 *  might be locked from this resource.
 *
 *  NOTE: 
 *  Given T_resource must only have been Unfind once.  T_resource cannot
 *  be locked.
 *
 *  @param res -- resource to unfind
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourceUnfind(T_resource res)
{
    T_resourceEntry *p_entry ;
#ifndef NDEBUG
    T_loadLink *p_loadLink ;
#endif

    DebugRoutine("ResourceUnfind") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* Get the quick pointer. */
    p_entry = (T_resourceEntry *)res ;
//if (p_entry == RESOURCE_BAD)  {
//    printf("Bad resource %p\n", p_entry) ;
//}

    if (p_entry)  {
#ifdef RESOURCE_OUTPUT
printf("!F 1 find_%s\n", p_entry->p_resourceName) ;
#endif
        /* Check if legal. */
        DebugCheck(strcmp(p_entry->resID, "ReS")==0) ;
//printf("ResourceUnfind: %s\n", p_entry->p_resourceName) ;
//if (!isdigit(p_entry->p_resourceName[0]))
        DebugCheck(isprint(p_entry->p_resourceName[0])) ;
        /* Cannot unfind a locked resource. */
    //    DebugCheck(p_entry->lockCount == 0) ;

        /* Must be a file, not a directory or link or append type. */
        DebugCheck(((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) == RESOURCE_ENTRY_TYPE_FILE) ||
                     (p_entry->resourceType & RESOURCE_ENTRY_TYPE_FILE_LOADED)) ;

        /* If the item we are unfinding is discardable, we need to remove */
        /* it from memory totally. */
#if 0
        if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) ==
                  RESOURCE_ENTRY_TYPE_DISCARDED)  {
            /* Pretend that the memory module wants its memory back. */
            p_data = (p_entry->p_data)-sizeof(T_resourceEntry *) ;
            IResourceMemCallback(p_data) ;

            /* Get the block back and discard of it fully. */
            MemReclaimDiscardable(p_data) ;
            MemFree(p_data) ;
        }
#endif
#ifndef NDEBUG
        if (p_entry->resourceType & RESOURCE_ENTRY_TYPE_FILE_LOADED)   {
            p_loadLink = (T_loadLink *)(p_entry->ownerDir) ;
            p_entry = p_loadLink->p_entry ;
            DebugCheck(p_entry->lockCount != 0) ;
            p_entry->lockCount-- ;
            if (p_entry->lockCount == 0)  {
                if (p_loadLink->p_prev)  {
                    p_loadLink->p_prev->p_next = p_loadLink->p_next ;
                } else {
                    G_loadedEntries = p_loadLink->p_next ;
                }

                if (p_loadLink->p_next)
                    p_loadLink->p_next->p_prev = p_loadLink->p_prev ;

#ifdef RESOURCE_OUTPUT
printf("!F %ld file_%s\n", p_entry->size, p_entry->p_resourceName) ;
#endif
                MemFree(p_entry->p_data) ;
                MemFree(p_entry) ;
                MemFree(p_loadLink) ;
            }
        } else {
            /* Now that we have the memory of the resource thrown out, */
            /* we need to fix the directory. */
            IDirUnlock((T_resourceDirInfo *)(p_entry->ownerDir)) ;
        }
#endif
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDirUnlock
 *-------------------------------------------------------------------------*/
/**
 *  IDirUnlock starts with a resource and unlocks the directory structure
 *  in the tree.
 *
 *  @param p_dir -- directory to unlock
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDirUnlock(T_resourceDirInfo *p_dir)
{
    T_resourceEntry *p_entry ;
    T_resourceDirInfo *p_parent ;

    DebugRoutine("IDirUnlock") ;

    if (p_dir != NULL)  {
        do {
            DebugCheck(p_dir != NULL) ;
            p_entry = (T_resourceEntry *)(p_dir->ownerRes) ;
            if (p_entry == RESOURCE_BAD)
                break ;
            if (((T_word32)p_entry) == 0xFFFFFFFF)
                break ;
            p_parent = p_entry->ownerDir ;

            /* If there is no owner for this resource, then we */
            /* are at the root and we can quit. */
            if (p_entry == RESOURCE_BAD)
                break ;

            DebugCheck(p_entry != NULL) ;
//printf("DirUnlock: %s by %s (%d)\n", p_entry->p_resourceName, DebugGetCallerName(), p_entry->lockCount) ;

            /* Unlock the resource by one. */
            DebugCheck(p_entry->lockCount != 0) ;
            p_entry->lockCount-- ;
//printf("!F 1 Dir:%s\n", p_entry->p_resourceName) ;
            if (p_entry->lockCount == 0)  {
                /* Directory is free to move around. */
//printf("Discarding directory %s\n", p_entry->p_resourceName) ;
//printf("!F 1 dir_%s\n", p_entry->p_resourceName) ;
//printf("!A 1 disdir_%p\n", p_dir) ;
                MemMarkDiscardable(
                    p_dir /* p_dir */,
                    IResourceMemCallbackForDir) ;
                p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
                p_entry->resourceType |= RESOURCE_ENTRY_TYPE_DISCARDED ;
            }

            /* If the directory has a parent, unlock it too. */
            p_dir = p_parent ;
        } while (p_dir != NULL) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICheckDirEntries
 *-------------------------------------------------------------------------*/
/**
 *  ICheckDirEntries checks to see that all entries in a directory are
 *  freed from memory.
 *
 *  @param p_dir -- directory to check
 *
 *<!-----------------------------------------------------------------------*/
#ifndef NDEBUG
static T_void ICheckDirEntries(T_resourceDirInfo *p_dir)
{
    T_word16 i ;
    T_word16 num ;

    DebugRoutine("ICheckDirEntries") ;
    DebugCheck(p_dir != NULL) ;
    DebugCheck(p_dir->p_entries != NULL) ;
    num = p_dir->numberEntries ;
    for (i=0; i<num; i++)  {
        DebugCheck(p_dir != NULL) ;

        if (p_dir->p_entries[i].lockCount!=0)  {
            puts("Cannot close resource file because of following resource:") ;
            ResourcePrint(stdout, &p_dir->p_entries[i]) ;
            PicturesDump() ;

            if ((p_dir->p_entries[i].resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) == RESOURCE_ENTRY_TYPE_DIRECTORY) {
                ICheckDirEntries((T_resourceDirInfo *)(p_dir->p_entries[i].p_data));
            }
        }
        DebugCheck(p_dir->p_entries[i].lockCount==0) ;
    }

    DebugEnd() ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IFindOpenResource
 *-------------------------------------------------------------------------*/
/**
 *  IFindOpenResource searches the list of open files to find a matching
 *  already opened resource file.  If none is found, a bad result is
 *  returned.
 *
 *  @param p_filename -- Pointer to the filename to find
 *
 *  @return Matching resource file, or
 *      RESOURCE_FILE_BAD
 *
 *<!-----------------------------------------------------------------------*/
static T_resourceFile IFindOpenResource(T_byte8 *p_filename)
{
    T_word16 i ;
    T_resourceFile file = RESOURCE_FILE_BAD ;

    DebugRoutine("IFindOpenResource") ;

    /* Check all the file slots. */
    for (i=0; i<MAX_RESOURCE_FILES; i++)  {
        /* See if the slot is being used for a file. */
        if (G_resourceCount[i] != 0)  {
            /* If it is, compare the filenames. */
            if (strcmp(G_resourceNames[i], p_filename) == 0)  {
                /* If they are the same, this is the same resource */
                /* file. */
                break ;
            }
        }
    }

    if (i!=MAX_RESOURCE_FILES)
        file = (T_resourceFile)i ;

    DebugEnd() ;

    return file ;
}

#ifndef NDEBUG
#ifndef WAS_NDEBUG
/*-------------------------------------------------------------------------*
 * Routine:  ResourcePrint
 *-------------------------------------------------------------------------*/
/**
 *  ResourcePrint prints out the information tied to the given resource
 *  to the given output.
 *
 *  @param fp -- File to output picture info
 *  @param res -- Resource to print
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourcePrint(FILE *fp, T_resource res)
{
    T_resourceEntry *p_resource ;

    DebugRoutine("ResourcePrint") ;

//    DebugCheck(res != RESOURCE_BAD) ;
    p_resource = res ;
    if (strcmp(p_resource->resID, "ReS") == 0)  {
//    DebugCheck(strcmp(p_resource->resID, "ReS")==0) ;
//    DebugCheck(p_resource->resourceType < RESOURCE_ENTRY_TYPE_UNKNOWN) ;

        fprintf(fp, "Resource: %p\n", p_resource) ;
        fprintf(fp, "  id    : %4.4s\n", p_resource->resID) ;
        fprintf(fp, "  name  : %14.14s\n", p_resource->p_resourceName) ;
        fprintf(fp, "  offset: %ld\n", p_resource->fileOffset) ;
        fprintf(fp, "  size  : %ld\n", p_resource->size) ;
        fprintf(fp, "  lockCo: %d\n", p_resource->lockCount) ;
        fprintf(fp, "  resTyp: 0x%02X\n", p_resource->resourceType) ;
        fprintf(fp, "  p_data: %p\n", p_resource->p_data) ;
        fprintf(fp, "  Rfile : %d\n", p_resource->resourceFile) ;
        fprintf(fp, "  ownDir: %p\n", p_resource->ownerDir) ;
    } else {
        fprintf(fp, "RESOURCE %p IS BAD!!!\n", p_resource) ;
        fprintf(fp, "Tag: %-4s (%02X %02X %02X %02X)\n",
            p_resource->resID,
            p_resource->resID[0],
            p_resource->resID[1],
            p_resource->resID[2],
            p_resource->resID[3]) ;
    }
    fflush(fp) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceCheckByPtr
 *-------------------------------------------------------------------------*/
/**
 *  ResourceCheckByPtr is a debugging routine used to validate that the
 *  given pointer points to data that is a resource block.
 *
 *  @param p_resData -- Pointer to res data (not res handle)
 *
 *<!-----------------------------------------------------------------------*/
T_void ResourceCheckByPtr(T_byte8 *p_resData)
{
    DebugRoutine("ResourceCheckByPtr") ;
    DebugCheck(p_resData != NULL) ;

    MemCheckData(p_resData - sizeof(T_resourceEntry *)) ;

    DebugEnd() ;
}

#endif
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IResourceMemCallbackForDir
 *-------------------------------------------------------------------------*/
/**
 *  IResourceMemCallback is called for each discardable resource that
 *  is being removed from memory by the Memory Module.  This routine does
 *  not do the freeing of memory, but does mark the block in the resource
 *  index table as no longer in memory.  This is VERY important, since
 *  the Resource Manager needs a way to know when the resources are
 *  gone.
 *
 *  NOTE: 
 *  Can't think of any.
 *
 *  @param p_block -- Pointer to block being removed
 *
 *<!-----------------------------------------------------------------------*/
static T_void IResourceMemCallbackForDir(T_void *p_block)
{
    T_resourceEntry *p_entry ;
    T_resourceDirInfo *p_dir = NULL ;

    DebugRoutine("IResourceMemCallbackForDir") ;
    DebugCheck(p_block != NULL) ;

//printf("IResourceMemCallbackForDir %p by %s\n", p_block, DebugGetCallerName()) ;  fflush(stdout) ;
    /* We don't actually need the data that we are receiving so much as */
    /* we need the data that we stored in front of this block.  Let's */
    /* start by getting that piece of informatin. */
    p_dir = (T_resourceDirInfo *)p_block ;
    p_entry = (T_resourceEntry *)(p_dir->ownerRes) ;

//printf("Checking dir entries in dir %s (%p)\n", p_entry->p_resourceName, p_dir) ;  fflush(stdout) ;
    /* Check to make sure all entries are correct. */
    ICheckDirEntries(p_dir) ;

//printf("Discarding entries in dir %s\n", p_entry->p_resourceName) ;  fflush(stdout) ;
    /* Go through the list of entries for this directory */
    /* and make sure that everything that should be discarded, is. */
    IDiscardEntries(p_dir->p_entries, p_dir->numberEntries) ;

//puts("Freeing memory") ;  fflush(stdout) ;
    /* !!! Need check here to see if all directory entries */
    /* are unlocked before deletion. */
//printf("Freeing memory at %p\n", p_dir->p_entries) ;
    MemFree(p_dir->p_entries) ;
    p_dir->p_entries = NULL ;

    /* Mark the entry as now on disk. */
    DebugCheck((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE) == RESOURCE_ENTRY_TYPE_DISCARDED) ;
    p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
    p_entry->resourceType |= RESOURCE_ENTRY_TYPE_DISK ;
    p_entry->p_data = NULL ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDiscardEntries
 *-------------------------------------------------------------------------*/
/**
 *  IDiscardEntries goes through a list of entries and discards them
 *  appropriately AND checks to see if all the items are unlocked.
 *
 *  NOTE: 
 *  Can't think of any.
 *
 *  @param p_entry -- Pointer to first entry of array
 *  @param number -- Number of entries to discard
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDiscardEntries(T_resourceEntry *p_entry, T_word16 number)
{
    T_word16 index ;
    T_byte8 *p_oldData ;

    DebugRoutine("IDiscardEntries") ;
    DebugCheck(p_entry != NULL) ;

    /* Loop through all entries in the index. */
    for (index=0;
         index < number;
         index++, p_entry++)  {
        switch(p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_WHERE)  {
            case RESOURCE_ENTRY_TYPE_DISCARDED:
                /* If we have found a resource that is marked as discarded, */
                /* reclaim the block and then free it. */
                if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
                    RESOURCE_ENTRY_TYPE_DIRECTORY)  {
                    /* Do whatever is needed before freeing the */
                    /* directory. */
                    p_oldData = p_entry->p_data ;
                    IResourceMemCallbackForDir(p_entry->p_data) ;

                    /* Free it. */
                    if (p_oldData != NULL)  {
                        MemReclaimDiscardable(p_oldData) ;
                        MemFree(p_oldData) ;
                    }
                } else {
#ifdef RESOURCE_OUTPUT
printf("!F %ld lock_%s\n", p_entry->size, p_entry->p_resourceName) ;
#endif
                    MemReclaimDiscardable(((T_byte8 *)p_entry->p_data) -
                        sizeof(T_resourceEntry *)) ;
                    MemFree(((T_byte8 *)p_entry->p_data) -
                        sizeof(T_resourceEntry *)) ;
                }
                p_entry->p_data = NULL ;
                p_entry->resourceType &= (~RESOURCE_ENTRY_TYPE_MASK_WHERE) ;
                p_entry->resourceType |= RESOURCE_ENTRY_TYPE_DISK ;
                break ;
            case RESOURCE_ENTRY_TYPE_MEMORY:
               /* If we have a block that is/was in memory, make sure it */
               /* has a lock count of zero and has a null pointer. */
#ifndef NDEBUG
#ifndef WAS_NDEBUG
                if (p_entry->lockCount != 0)  {
                    puts("Following resource is still locked!") ;
                    ResourcePrint(stdout, p_entry) ;
                }
#endif
#endif
                DebugCheck(p_entry->lockCount == 0) ;
                DebugCheck(p_entry->p_data == NULL) ;
                break ;
            case RESOURCE_ENTRY_TYPE_DISK:
                /* OK. */
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }

        /* In any case, invalidate the res tag at the beginning of the */
        /* entries to make sure that the resource can no longer be used. */
        p_entry->resID[0] = '\0' ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ResourceGetName
 *-------------------------------------------------------------------------*/
/**
 *  ResourceGetName finds the name that goes with the corresponding
 *  data pointer.
 *
 *  @param p_data -- Pointer to data to find name of
 *
 *  @return Pointer to name
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *ResourceGetName(T_void *p_data)
{
    T_resourceEntry *p_entry ;
    T_byte8 *p_name ;

    DebugRoutine("ResourceGetName") ;
    DebugCheck(p_data != NULL) ;
    p_entry = *((T_resourceEntry **)
                   (((T_byte8 *)p_data) - sizeof(T_resourceEntry *))) ;
    DebugCheck(p_entry != NULL) ;
    DebugCheck(strcmp(p_entry->resID, "ReS")==0) ;
    p_name = p_entry->p_resourceName ;

    DebugEnd() ;

    return p_name ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  RESOURCE.C
 *-------------------------------------------------------------------------*/
