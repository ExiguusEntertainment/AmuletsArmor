#include "RESOURCE.H"
#include "IRESOURC.H"
#include "minizip/unzip.h"

//typedef struct {
//    T_file fileHandle           PACK;
//    T_word16 numberEntries      PACK;
//    T_resourceEntry *p_entries  PACK;
//    T_resource ownerRes         PACK;
//    T_sword16 nextResource      PACK;
//} T_resourceDirInfo ;

//static T_resourceDirInfo G_resources[MAX_RESOURCE_FILES] = {
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 1},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 2},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 3},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 4},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 5},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 6},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 7},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 8},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, 9},
//    { FILE_BAD, 0, NULL, RESOURCE_BAD, -1},
//} ;
//
//static T_byte8 G_resourceNames[MAX_RESOURCE_FILES][200] ;
//static T_byte8 G_resourceCount[MAX_RESOURCE_FILES] = {
//    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
//};
//
/* Keep track of who is on the free list of resource files. */
static T_sword16 G_firstFreeResourceFile = 0 ;

/* Keep track of the number of resource files open. */
static T_word16 G_numberOpenResourceFiles = 0 ;



T_resourceFile ResourceOpen(const char *filename)
{
    // Open a resource by loading the .zip file directory
    char zipFilename[200];
    char *p;
    unzFile zipfile;
    DebugRoutine("ResourceOpen");
    DebugCheck(filename != NULL) ;
    DebugCheck(G_numberOpenResourceFiles < MAX_RESOURCE_FILES) ;
    DebugCheck(G_firstFreeResourceFile != -1) ;

    // Replace .RES on end with .zip and put in zipFilename
    strcpy(zipFilename, filename);
    p = strstr(zipFilename, ".RES");
    if (!p)
        p = strstr(zipFilename, ".res");
    if (p) {
        strcpy(p, ".zip");
    }
printf("Opening resource %s\n", zipFilename);

//    file = fopen(zipFilename, "rb");
//    DebugCheck(file != FILE_BAD) ;
    zipfile = unzOpen(zipFilename);
    DebugCheck(zipfile != NULL);

    DebugEnd();

    return RESOURCE_FILE_BAD;
}

T_void ResourceClose(T_resourceFile resourceFile)
{

}

T_resource ResourceFind(T_resourceFile resourceFile, const char *p_resourceName)
{
    return 0;
}

T_void ResourceUnfind(T_resource res)
{

}

T_void *ResourceLock(T_resource resource)
{
    return 0;
}

T_void ResourceUnlock(T_resource resource)
{

}

T_word32 ResourceGetSize(T_resource resource)
{
    return 0;
}


T_byte8 *ResourceGetName(T_void *p_data)
{
    return 0;
}

#ifndef NDEBUG
T_void ResourceDumpIndex(T_resourceFile resourceFile)
{

}
T_void ResourcePrint(FILE *fp, T_resource res)
{

}
T_void ResourceCheckByPtr(T_byte8 *p_resData)
{

}
#else
#define ResourceDumpIndex()
#define ResourcePrint(fp, res)
#define ResourceCheckByPtr(p_resData)
#endif
