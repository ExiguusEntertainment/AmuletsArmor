#include "RESOURCE.H"
#include "IRESOURC.H"
#include <unzip.h>
#include "MEMORY.H"

#if 0
#define dprintf printf
#else
#define dprintf(...)
#endif

typedef struct _T_resourceFileInfo {
        struct _T_resourceFileInfo *iNext;
        void *iResourceList; // struct _T_resourceInfo *
        unzFile iZipFile;
        T_sword32 iCount;
        char iName[200];
        T_word32 iTag;
} T_resourceFileInfo;

typedef struct _T_resourceInfo {
        struct _T_resourceInfo *iNext;
        T_resourceFileInfo *iFile;
        char *iName;
        unz_file_pos iPosition;
        unsigned int iDataSize;
        unsigned int iLockCount;
        void *iData; // 0 if data is not locked
        T_word32 iTag;
} T_resourceInfo;

static T_resourceFileInfo *G_resourceFiles = 0;

void IResourceFree(T_resourceInfo *p_resource);
static T_resourceInfo *IResourceAlloc(
        T_resourceFileInfo *aFile,
        const char *aResourceName,
        unz_file_pos *aPosition,
        unz_file_info *aInfo);

#define RESOURCE_TAG            (*((T_word32 *)"RtAg"))
#define RESOURCE_FILE_TAG       (*((T_word32 *)"Rinf"))

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
static T_resourceFileInfo *IResourceFileFind(const char *p_filename)
{
    T_resourceFileInfo *p_file = G_resourceFiles;

    DebugRoutine("IResourceFileFind");

    for (p_file = G_resourceFiles; p_file; p_file = p_file->iNext) {
        if ((strcmp(p_file->iName, p_filename) == 0) && (p_file->iCount))
            break;
    }
    
    DebugEnd();

    return p_file;
}

static T_resourceFileInfo *IResourceFileAdd(
        const char *aFilename,
        unzFile aFile)
{
    T_resourceFileInfo *p_file;

    DebugRoutine("IResourceFileAdd");

    p_file = MemAlloc(sizeof(T_resourceFileInfo));
    DebugCheck(p_file != NULL);
    if (p_file) {
        strncpy(p_file->iName, aFilename, sizeof(p_file->iName));
        p_file->iCount = 1;
        p_file->iZipFile = aFile;
        p_file->iNext = G_resourceFiles;
        p_file->iResourceList = 0;
        p_file->iTag = RESOURCE_FILE_TAG;
        G_resourceFiles = p_file;
    }
    DebugEnd();

    return p_file;
}

static void IResourceFileRemove(T_resourceFileInfo *aFile)
{
    T_resourceFileInfo *p_file = G_resourceFiles;
    T_resourceFileInfo *p_prev = 0;

    DebugRoutine("IResourceFileRemove");
    DebugCheck(aFile->iCount == 0);
    DebugCheck(aFile->iZipFile == 0);
    DebugCheck(aFile->iName[0] != '\0');
    for (p_file = G_resourceFiles; p_file; p_file = p_file->iNext) {
        if (p_file == aFile)
            break;
        p_prev = p_file;
    }
    DebugCheck(p_file != 0);
    if (p_file) {
        // Unlink it from the list
        if (p_prev)
            p_prev->iNext = p_file->iNext;
        else
            G_resourceFiles = p_file->iNext;

        // Free it from member
        p_file->iNext = 0;
        p_file->iName[0] = '\0';
        p_file->iZipFile = 0;
        p_file->iCount = 0;
        p_file->iTag = 0;
        MemFree(p_file);
    }

    DebugEnd();

}

static void IResourceFilePullInDirectory(T_resourceFileInfo *p_file)
{
    int error;
    DebugRoutine("IResourceFilePullInDirectory");
    error = unzGoToFirstFile(p_file->iZipFile);
    while (error == UNZ_OK) {
        unz_file_info info;
        char name[4096];
        unz_file_pos pos;
        if (unzGetCurrentFileInfo(p_file->iZipFile, &info, name,
                sizeof(name), NULL, 0, NULL, 0) != UNZ_OK)
            break;
        if (unzGetFilePos(p_file->iZipFile, &pos) != UNZ_OK)
            break;
        dprintf("  adding %s : %s (@%d)\n", p_file->iName, name, pos.pos_in_zip_directory);
        IResourceAlloc(p_file, name, &pos, &info);

        error = unzGoToNextFile(p_file->iZipFile);
    }
    DebugEnd();
}

T_resourceFile ResourceOpen(const char *filename)
{
    // Open a resource by loading the .zip file directory
    char zipFilename[200];
    char *p;
    unzFile zipfile;
    T_resourceFileInfo *p_file = 0;
    DebugRoutine("ResourceOpen");
    DebugCheck(filename != NULL);

    dprintf("Opening resource file %s\n", filename);
    // Replace .RES on end with .zip and put in zipFilename
    strcpy(zipFilename, filename);
    p = strstr(zipFilename, ".RES");
    if (!p)
        p = strstr(zipFilename, ".res");
    if (p) {
        strcpy(p, ".zip");
    }
    dprintf("Opening resource %s\n", zipFilename);

    p_file = IResourceFileFind(filename);
    if (p_file == RESOURCE_FILE_BAD) {
        // Not in the list, let's try opening
        zipfile = unzOpen(zipFilename);
        DebugCheck(zipfile != NULL);
        if (zipfile) {
            p_file = IResourceFileAdd(filename, zipfile);
            DebugCheck(p_file != NULL);
            DebugCheck(p_file->iTag == RESOURCE_FILE_TAG);
        }

        // Create all the resource handles for all the files
        IResourceFilePullInDirectory(p_file);
    } else {
        // Found it, increment it's count
        p_file->iCount++;
    }

    DebugEnd();

    return (T_resource)p_file;
}

static T_void IResourceFileClear(T_resourceFileInfo *aFile)
{
    DebugRoutine("IResourceFileClear");
    DebugCheck(aFile->iTag == RESOURCE_FILE_TAG);

    while (aFile->iResourceList) {
        IResourceFree(aFile->iResourceList);
    }
    DebugEnd();
}

T_void ResourceClose(T_resourceFile resourceFile)
{
    // Just decrement the number of times the file has been opened or close
    // We really are just tracking who uses it currently, but later
    // we might free it from the list
    T_resourceFileInfo *p_file = (T_resourceFileInfo *)resourceFile;
    DebugCheck(p_file);
    if (p_file) {
        dprintf("Closing resource file %s\n", p_file->iName);
        DebugCheck(p_file->iTag == RESOURCE_FILE_TAG);
        if (p_file->iTag == RESOURCE_FILE_TAG) {
            // Should never go negative
            DebugCheck(p_file->iCount >= 0);

            // About to be cleared out?
            if (p_file->iCount == 1) {
                // Clear off all resources
                IResourceFileClear(p_file);
            }

            p_file->iCount--;

            // Are we no longer being used?
            if (p_file->iCount == 0) {
                // Then close it and remove it from the list
                unzClose(p_file->iZipFile);
                p_file->iZipFile = 0;

                // Not remove the resource file from the list
                IResourceFileRemove(p_file);
            }
        }
    }
}

static T_resourceInfo *IResourceAlloc(
        T_resourceFileInfo *aFile,
        const char *aResourceName,
        unz_file_pos *aPosition,
        unz_file_info *aInfo)
{
    T_resourceInfo *p_resource = 0;

    DebugRoutine("IResourceAdd");
    DebugCheck(aFile);
    DebugCheck(aFile->iTag == RESOURCE_FILE_TAG);
    DebugCheck(aResourceName != 0);
    DebugCheck(aPosition != 0);
    DebugCheck(aInfo != 0);

    p_resource = MemAlloc(sizeof(T_resourceInfo));
    DebugCheck(p_resource != 0);
    if (p_resource) {
        p_resource->iData = 0;
        p_resource->iDataSize = aInfo->uncompressed_size;
        p_resource->iFile = aFile;
        p_resource->iLockCount = 0;
        p_resource->iName = MemAlloc(strlen(aResourceName) + 1);
        DebugCheck(p_resource->iName != 0);
        strcpy(p_resource->iName, aResourceName);
        p_resource->iPosition = *aPosition;
        p_resource->iTag = RESOURCE_TAG;

        // Add to the list of resources on this file
        p_resource->iNext = (T_resourceInfo *)aFile->iResourceList;
        aFile->iResourceList = (void *)p_resource;
    }

    DebugEnd();

    return p_resource;
}

static T_resourceInfo *IResourceFileDetachResource(T_resourceFileInfo *aFile, T_resourceInfo *aResource)
{
    T_resourceInfo *p_resource = 0;
    T_resourceInfo *p_prev = 0;

    DebugRoutine("IResourceFileDetachResource");
    DebugCheck(aFile != NULL);
    DebugCheck(aResource != NULL);

    if ((aFile != NULL) && (aResource != NULL)) {
        DebugCheck(aFile->iCount != 0);
        DebugCheck(aFile->iZipFile != 0);
        DebugCheck(aFile->iTag == RESOURCE_FILE_TAG);
        if ((aFile->iCount) && (aFile->iZipFile) && (aFile->iTag == RESOURCE_FILE_TAG)) {
            for (p_resource = (T_resourceInfo *)aFile->iResourceList; p_resource; p_resource=p_resource->iNext) {
                if (p_resource == aResource)
                    break;
                p_prev = p_resource;
            }
            DebugCheck(p_resource != 0);
            if (p_resource) {
                // Detach
                if (p_prev)
                    p_prev->iNext = p_resource->iNext;
                else
                    aFile->iResourceList = (T_void *)p_resource->iNext;
                p_resource->iNext = 0;
            }
        }
    }
    DebugEnd();

    // Return what was detached, or 0 if not found
    return p_resource;
}

static void IResourceFree(T_resourceInfo *p_resource)
{
    DebugRoutine("IResourceFree");
    DebugCheck(p_resource != 0);
    if (p_resource) {
        DebugCheck(p_resource->iFile != 0);
        if (p_resource->iFile) {
            p_resource = IResourceFileDetachResource(p_resource->iFile, p_resource);
            DebugCheck(p_resource != NULL);
            if (p_resource) {
                // Must be fully unlocked to free the resource!
                DebugCheck(p_resource->iLockCount == 0);
                p_resource->iTag = 0;
                MemFree(p_resource->iName);
                p_resource->iName = 0;

                // Free any memory
                if (p_resource->iData) {
                    // Clear the pointer in the data back to the resource info
                    *((T_resourceInfo **)p_resource->iData) = 0;

                    // Now free the data block itself
                    MemFree(p_resource->iData);
                    p_resource->iData = 0;
                }
                p_resource->iFile = 0;
                p_resource->iDataSize = 0;
                p_resource->iLockCount = 0;
                p_resource->iPosition.num_of_file = 0;
                p_resource->iPosition.pos_in_zip_directory = 0;
            }
        }
    }
    DebugEnd();
}

static void IResourceFreeData(T_resourceInfo *p_resource)
{
    DebugRoutine("IResourceFree");
    DebugCheck(p_resource != 0);
    if (p_resource) {
        // Free any data memory
        if (p_resource->iData) {
            // Clear the pointer in the data back to the resource info
            *((T_resourceInfo **)p_resource->iData) = 0;

            // Now free the data block itself
            MemFree(p_resource->iData);
            p_resource->iData = 0;
        }
    }
    DebugEnd();
}

static T_resourceInfo *IResourceFileFindResourceByName(
        T_resourceFileInfo *aFile,
        const char *aName)
{
    T_resourceInfo *p_resource = 0;
    DebugRoutine("IResourceFileFindResourceByName");
    DebugCheck(aFile != NULL);
    DebugCheck(aName != NULL);
    DebugCheck(aFile->iTag == RESOURCE_FILE_TAG);
    for (p_resource = (T_resourceInfo *)aFile->iResourceList; p_resource;
            p_resource = p_resource->iNext) {
        if (strcmp(p_resource->iName, aName) == 0) {
            break;
        }
    }
    DebugEnd();
    return p_resource;
}

T_resource ResourceFind(T_resourceFile resourceFile, const char *p_resourceName)
{
    T_resourceFileInfo *p_file = (T_resourceFileInfo *)resourceFile;
    T_resourceInfo *p_resource = 0;
    DebugRoutine("ResourceFind");
    DebugCheck(p_file);
    if (p_file) {
        DebugCheck(p_file->iZipFile);
        if (p_file->iZipFile) {
            dprintf("Finding %s : %s\n", p_file->iName, p_resourceName);
            // First, see if we have it already in our index
            p_resource = IResourceFileFindResourceByName(p_file,
                    p_resourceName);
            if (!p_resource) {
                // Not found, okay.  Let's look for it in the file
                if (unzLocateFile(p_file->iZipFile, p_resourceName, 1) == UNZ_OK) {
                    // Found!
                    unz_file_pos pos;
                    unz_file_info info;

                    // Get the position and information and add to the resource list
                    unzGetFilePos(p_file->iZipFile, &pos);
                    unzGetCurrentFileInfo(p_file->iZipFile, &info, NULL, 0, NULL,
                            0, NULL, 0);
                    p_resource = IResourceAlloc(p_file, p_resourceName, &pos,
                            &info);
                }
            }
        } else {
            // Not found (or error)
            // Just fall out
        }
    }
    DebugEnd();
    return (T_resource)p_resource;
}

// Unfind can really be thought of as 'unload'
T_void ResourceUnfind(T_resource res)
{
    // This releases the memory for the res
    T_resourceInfo *p_resource = (T_resourceInfo *)res;
    DebugRoutine("ResourceUnfind");
    DebugCheck(p_resource);
    if (p_resource) {
        DebugCheck(p_resource->iTag == RESOURCE_TAG);
        // Cannot unfind resources that are locked
        //DebugCheck(p_resource->iLockCount == 0);
        if (p_resource->iLockCount == 0) {
            // We can unload this resource now, just detach it fully
            // from the cached resource file information
            IResourceFreeData(p_resource);
        }
    }
    DebugEnd();
}

// Lock really loads the resource from the resource file
// if not in memory, or just marks it for use again
T_void *ResourceLock(T_resource resource)
{
    T_resourceInfo *p_resource = (T_resourceInfo *)resource;
    T_void *p_data = 0;
    DebugRoutine("ResourceLock");
    DebugCheck(p_resource);
    if (p_resource) {
        DebugCheck(p_resource->iTag == RESOURCE_TAG);
        if (p_resource->iTag == RESOURCE_TAG) {
            // Has the resource been loaded yet?
            if (p_resource->iData == 0) {
                dprintf("Locking %s : %s\n", p_resource->iFile->iName, p_resource->iName);
                // Allocate memory to hold the resource
                p_resource->iData = MemAlloc(sizeof(T_resourceInfo *) + p_resource->iDataSize);
                *((T_resourceInfo **)p_resource->iData) = p_resource;
                DebugCheck(p_resource->iData != 0);
                if (p_resource->iData) {
                    // Seek the file opsition
                    if (unzGoToFilePos(p_resource->iFile->iZipFile,
                            &p_resource->iPosition) == UNZ_OK) {
                        int error = 0;
                        if (unzOpenCurrentFile(p_resource->iFile->iZipFile) != UNZ_OK) {
                            printf("Could not open file %s : %s!\n", p_resource->iFile->iName, p_resource->iName);
                            MemFree(p_resource->iData);
                            p_resource->iData = 0;
                        } else {
                            error = unzReadCurrentFile(p_resource->iFile->iZipFile,
                                    sizeof(T_resourceInfo *)
                                            + (T_byte8 *)p_resource->iData,
                                    p_resource->iDataSize);
                            unzCloseCurrentFile(p_resource->iFile->iZipFile);
                            if (error < 0) {
                                printf("Failed to read resource %s : %s!\n",
                                        p_resource->iFile->iName, p_resource->iName);
                                MemFree(p_resource->iData);
                                p_resource->iData = 0;
                            } else {
                                p_data = sizeof(T_resourceInfo *)
                                        + (T_byte8 *)p_resource->iData;
                            }
                        }
                    } else {
                        printf("Failed to locate resource %s : %s!\n",
                                p_resource->iFile->iName, p_resource->iName);
                        MemFree(p_resource->iData);
                        p_resource->iData = 0;
                    }
                }
            } else {
                p_data = sizeof(T_resourceInfo *)
                        + (T_byte8 *)p_resource->iData;
            }
            // Note we have locked this resource
            p_resource->iLockCount++;
        }
    }

    DebugEnd();
    return p_data;
}

T_void ResourceUnlock(T_resource resource)
{
    T_resourceInfo *p_resource = (T_resourceInfo *)resource;

    DebugRoutine("ResourceUnlock");
    DebugCheck(p_resource);

    if (p_resource) {
        DebugCheck(p_resource->iTag == RESOURCE_TAG);
        if (p_resource->iTag == RESOURCE_TAG) {
            // Decrement the lock count.  We don't unload here yet.
            DebugCheck(p_resource->iLockCount > 0);
            if (p_resource->iLockCount > 0)
                p_resource->iLockCount--;
        }
    }

    DebugEnd();
}

T_word32 ResourceGetSize(T_resource resource)
{
    T_resourceInfo *p_resource = (T_resourceInfo *)resource;
    T_word32 size = 0;

    DebugRoutine("ResourceGetSize");
    DebugCheck(p_resource);

    if (p_resource) {
        DebugCheck(p_resource->iTag == RESOURCE_TAG);
        if (p_resource->iTag == RESOURCE_TAG) {
            // Return the size (even if not loaded)
            size = p_resource->iDataSize;
        }
    }

    DebugEnd();

    return size;
}

T_byte8 *ResourceGetName(T_void *p_data)
{
    T_resourceInfo *p_resource = ((T_resourceInfo **)p_data)[-1];
    T_byte8 *p_name = 0;

    DebugRoutine("ResourceGetSize");
    DebugCheck(p_data);
    DebugCheck(p_resource);

    if ((p_data) && (p_resource)) {
        // Make sure the resource IS a resource
        DebugCheck(p_resource->iTag == RESOURCE_TAG);

        // Ensure we are linked to each other
        DebugCheck(p_resource->iData == &((T_resourceInfo *)p_data)[-1]);

        if (p_resource->iTag == RESOURCE_TAG) {
            // Return the full name path
            p_name = p_resource->iName;
        }
    }

    DebugEnd();

    return p_name;
}

#ifndef NDEBUG
T_void ResourceDumpIndex(T_resourceFile resourceFile)
{
    T_resourceFileInfo *p_file = (T_resourceFileInfo *)resourceFile;
    T_resourceInfo *p_resource ;
    FILE *fp ;

    DebugRoutine("ResourceDumpIndex") ;
    DebugCheck(p_file != NULL) ;
    if (p_file) {
        DebugCheck(p_file->iTag == RESOURCE_FILE_TAG);
        if (p_file->iTag == RESOURCE_FILE_TAG) {
            p_resource = (T_resourceInfo *)p_file->iResourceList;

            fp = fopen("resourdb.txt", "w") ;
            DebugCheck(fp != NULL) ;

            fprintf(fp, "Resource dump of file %p:\n------------\n", p_file) ;
            fprintf(fp, "id:  Name:          Offset:  Size:    Lck Pointer: File:\n");
            fprintf(fp, "---- -------------- -------- -------- --- -------- -----\n") ;
            for (; p_resource; p_resource = p_resource->iNext)  {
                fprintf(fp, "%-4.4s %-14s %8d %8d %3d %p %p\n",
                    (char *)&p_resource->iTag,
                    p_resource->iName,
                    p_resource->iPosition.pos_in_zip_directory,
                    p_resource->iDataSize,
                    p_resource->iLockCount,
                    p_resource->iData,
                    p_resource->iFile);
            }

            fclose(fp) ;
        }
    }

    DebugEnd() ;
}
T_void ResourcePrint(FILE *fp, T_resource res)
{
    T_resourceInfo *p_resource = (T_resourceInfo *)res;

    DebugRoutine("ResourcePrint") ;

//    DebugCheck(res != RESOURCE_BAD) ;
    if (p_resource->iTag == RESOURCE_TAG)  {
//    DebugCheck(strcmp(p_resource->resID, "ReS")==0) ;
//    DebugCheck(p_resource->resourceType < RESOURCE_ENTRY_TYPE_UNKNOWN) ;

        fprintf(fp, "Resource: %p\n", p_resource) ;
        fprintf(fp, "  id    : %-4.4s\n", (char *)&p_resource->iTag) ;
        fprintf(fp, "  name  : %14.14s\n", p_resource->iName) ;
        fprintf(fp, "  offset: %ld\n", p_resource->iPosition.pos_in_zip_directory) ;
        fprintf(fp, "  size  : %ld\n", p_resource->iDataSize) ;
        fprintf(fp, "  lockCo: %d\n", p_resource->iLockCount) ;
        fprintf(fp, "  p_data: %p\n", p_resource->iData) ;
        fprintf(fp, "  Rfile : %p\n", p_resource->iFile) ;
    } else {
        fprintf(fp, "RESOURCE %p IS BAD!!!\n", p_resource) ;
        fprintf(fp, "Tag: %-4s (%02X %02X %02X %02X)\n",
            p_resource->iTag,
            ((char *)&p_resource->iTag)[0],
            ((char *)&p_resource->iTag)[1],
            ((char *)&p_resource->iTag)[2],
            ((char *)&p_resource->iTag)[3]) ;
    }
    fflush(fp) ;

    DebugEnd() ;
}
T_void ResourceCheckByPtr(T_byte8 *p_resData)
{
    DebugRoutine("ResourceCheckByPtr") ;
    DebugCheck(p_resData != NULL) ;

    MemCheckData(p_resData - sizeof(T_resourceInfo *)) ;

    DebugEnd() ;
}
#else
#define ResourceDumpIndex()
#define ResourcePrint(fp, res)
#define ResourceCheckByPtr(p_resData)
#endif
