/*-------------------------------------------------------------------------*
 * File:  3D_IO.C
 *-------------------------------------------------------------------------*/
/**
 * Routines for loading maps are handled here.  This code differs from
 * the MAP code by handling the raw 3D information about a map.  It
 * knows about vertixes, wall segments, objects on the map, and any
 * of the 'raw' information about the world needed for rendering.
 * At the heart of it, it is loading DOOM PWAD/IWAD maps since
 * Deep 95 was used to make the maps.
 *
 * @addtogroup _3D_IO
 * @brief 3D Loading and unloading of maps
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
//#include "standard.h"
#include <ctype.h>
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "AREASND.H"
#include "DOOR.H"
#include "EFFECT.H"
#include "FILE.H"
#include "GENERAL.H"
#include "MAP.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "PICS.H"
#include "PROMPT.H"

#define OBJECT_TYPE_LIGHT   924

/* Map information telling how many of each item is available. */
T_word16             G_Num3dObjects ;
T_word16             G_Num3dSegs ;
T_word16             G_Num3dSides ;
T_word16             G_Num3dLines ;
T_word16             G_Num3dObjects ;
T_word16             G_Num3dNodes ;
T_word16             G_Num3dSectors ;
T_word16             G_Num3dSSectors ;
T_word16             G_Num3dVertexes ;
T_word16             G_BlockmapSize ;

/* Arrays to store the map information. */
T_3dSegment         *G_3dSegArray = NULL ;
T_3dSide            *G_3dSideArray = NULL ;
T_3dLine            *G_3dLineArray = NULL ;
T_3dObject          *G_First3dObject = NULL ;
T_3dObject          *G_Last3dObject = NULL ;
T_3dNode            *G_3dNodeArray = NULL ;
T_3dNode           **G_3dPNodeArray;
T_3dSector          *G_3dSectorArray = NULL ;
T_3dVertex          *G_3dVertexArray = NULL ;
T_3dSegmentSector   *G_3dSegmentSectorArray = NULL ;
T_3dBlockMap        *G_3dBlockMapArray = NULL ;
T_3dBlockMapHeader  *G_3dBlockMapHeader = NULL ;
T_byte8             *G_3dReject = NULL ;

T_sword16            G_3dRootBSPNode ;
T_resource          *G_3dUpperResourceArray = NULL ;
T_resource          *G_3dLowerResourceArray = NULL ;
T_resource          *G_3dMainResourceArray = NULL ;
T_resource          *G_3dFloorResourceArray = NULL ;
T_resource          *G_3dCeilingResourceArray = NULL ;

/* Player's location. */
T_sword16            G_3dPlayerX ;
T_sword16            G_3dPlayerY ;
T_sword32            G_3dPlayerHeight = 32<<16 ;

/* Direction player is facing. */
T_word16             G_3dPlayerAngle ;

/* Left angle of view (calculated from player's viewing angle). */
T_word16             G_3dPlayerLeftAngle ;
T_word16             G_3dPlayerRightAngle ;

/* Precomputed sine and cosine of the player's angle. */
T_sword32            G_3dCosPlayerAngle ;
T_sword32            G_3dSinPlayerAngle ;

/* Temporary variables to record the current height of the floor */
/* and ceiling. */
T_sword16            G_3dFloorHeight ;
T_sword16            G_3dCeilingHeight ;

/* Keep track of information about a sector */
T_3dSectorInfo      *G_3dSectorInfoArray ;

#ifdef MIP_MAPPING_ON
#if 0
T_hash32             G_textureHash = HASH32_BAD ;
#endif
#endif

/* Internal prototypes: */
static T_void ILoadObjects(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadSegs(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadSides(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadLines(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadNodes(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadSectors(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadVertexes(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadSegmentSectors(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILoadReject(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void IDumpData(T_void) ;
static T_void IPrepareObjects(T_void) ;
static T_void ILoadBlockMap(
                  T_file file,
                  T_directoryEntry *p_dir) ;
static T_void ILockPictures(T_void) ;
static T_void IUnlockPictures(T_void) ;
static T_void ILoadSectorInfo(T_byte8 *mapName) ;

static T_sword16 G_mapStartsX[4] ;
static T_sword16 G_mapStartsY[4] ;
static T_word16 G_mapAngles[4] ;

/* Mip mapping capability goes here: */
#ifdef MIP_MAPPING_ON
static T_void IShrinkTexture(
                  T_byte8 *p_to,
                  T_byte8 *p_from,
                  T_word16 sizeX,
                  T_word16 sizeY) ;

#endif

static T_void IOutputReject(T_void) ;

static T_void IObjCollisionListsInit(T_void) ;
static T_void IObjCollisionListsFinish(T_void) ;

T_doubleLinkList G_specialObjectList = DOUBLE_LINK_LIST_BAD ;

/* Size of object collision hash table. */
T_word16             G_objCollisionNumX ;
T_word16             G_objCollisionNumY ;
T_word16             G_lastCollisionList ;
T_doubleLinkList    *G_3dObjCollisionLists ;

/*-------------------------------------------------------------------------*
 * Routine:  View3dLoadMap
 *-------------------------------------------------------------------------*/
/**
 *  View3dLoadMap loads into memory a precompiled map that contains
 *  all walls, sectors, lines, things, etc.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dLoadMap(T_byte8 *MapName)
{
    const T_word32 level = 0 ;
    T_file file ;
    T_wadHeader header ;
    T_directoryEntry directory ;
    T_word32 entryOffset ;
    T_word16 entriesRead ;
    T_byte8 newname[80] ;
    T_byte8 *ptr ;

    DebugRoutine("View3dLoadMap") ;

    file = FileOpen(MapName, FILE_MODE_READ) ;
    DebugCheck(file != FILE_BAD) ;

    FileRead(file, &header, sizeof(header)) ;

    if (strnicmp(header.signature, "IWAD", 4) == 0)  {
//puts("Reading IWAD file.\n") ;
        header.foffset += ((T_word32)6) * sizeof(T_directoryEntry) ;
    } else if (strnicmp(header.signature, "PWAD", 4) == 0)  {
//puts("Reading PWAD file.\n") ;
    } else {
puts("Not a valid file!\n") ;
        DebugCheck(FALSE) ;
    }

//printf("%d directory entries.\n\n", header.numEntries) ;
    entryOffset = header.foffset +
                      (level * ((T_word32)11) * sizeof(T_directoryEntry)) ;

    /* Setup a list of special objects to consider after almost */
    /* everything is loaded. */
    DebugCheck(G_specialObjectList == DOUBLE_LINK_LIST_BAD) ;
    G_specialObjectList = DoubleLinkListCreate() ;
    DebugCheck(G_specialObjectList != DOUBLE_LINK_LIST_BAD) ;

    /* Read part 1 stuff. */
    entriesRead = 0 ;
    while (entriesRead < 11)  {
        FileSeek(file, entryOffset) ;
        FileRead(file, &directory, sizeof(T_directoryEntry)) ;

        if (strnicmp(directory.name, "BLOCKMAP", 8) == 0)
            ILoadBlockMap(file, &directory) ;

        entriesRead++ ;
        entryOffset += sizeof(T_directoryEntry) ;
    }

    FileClose(file) ;

    /* Get the object collision data correctly prepared for when */
    /* we do load objects. */
    IObjCollisionListsInit() ;

    /* Read part 2 now. */
    file = FileOpen(MapName, FILE_MODE_READ) ;
    DebugCheck(file != FILE_BAD) ;
    entriesRead = 0 ;
    entryOffset = header.foffset +
                      (level * ((T_word32)11) * sizeof(T_directoryEntry)) ;
    while (entriesRead < 11)  {
        /* assumes status bar is active */
        PromptStatusBarUpdate (25+(entriesRead*3));
        FileSeek(file, entryOffset) ;
        FileRead(file, &directory, sizeof(T_directoryEntry)) ;

        if (strnicmp(directory.name, "THINGS", 6)==0)
            ILoadObjects(file, &directory) ;
        else if (strnicmp(directory.name, "LINEDEFS", 8) == 0)
            ILoadLines(file, &directory) ;
        else if (strnicmp(directory.name, "VERTEXES", 8) == 0)
            ILoadVertexes(file, &directory) ;
        else if (strnicmp(directory.name, "SIDEDEFS", 8) == 0)
            ILoadSides(file, &directory) ;
        else if (strnicmp(directory.name, "SEGS", 4) == 0)
            ILoadSegs(file, &directory) ;
        else if (strnicmp(directory.name, "SSECTORS", 8) == 0)
            ILoadSegmentSectors(file, &directory) ;
        else if (strnicmp(directory.name, "NODES", 5) == 0)
            ILoadNodes(file, &directory) ;
        else if (strnicmp(directory.name, "SECTORS", 7) == 0)
            ILoadSectors(file, &directory) ;
        else if (strnicmp(directory.name, "REJECT", 6) == 0)
            ILoadReject(file, &directory) ;

        entriesRead++ ;
        entryOffset += sizeof(T_directoryEntry) ;
    }

    FileClose(file) ;

//IOutputReject() ;

    strcpy(newname, MapName) ;
    ptr = strstr(newname, ".") ;
    if (ptr == NULL)
        ptr = newname+strlen(newname) ;
    strcpy(ptr, ".sec") ;

//printf("Loading sector info from '%s'\n", newname) ;
    /* Load in any other sector information that is needed. */
//    ILoadSectorInfo(newname) ;

//puts("Remapping sectors") ;
    View3dRemapSectors() ;

//puts("Preparing objects") ;
    /* Initialize any additional object data (like it's height) */
    IPrepareObjects() ;

//puts("Locking pictures") ;
#ifndef SERVER_ONLY
    ILockPictures() ;
#endif
///    IDumpData() ;

//puts("Loaded map.") ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dUnloadMap
 *-------------------------------------------------------------------------*/
/**
 *  View3dUnloadMap unallocates any memory used by the current map.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dUnloadMap(T_void)
{
    DebugRoutine("View3dUnloadMap") ;

    IObjCollisionListsFinish() ;

    IUnlockPictures() ;

    MemFree(G_3dSegArray) ;
    MemFree(G_3dLineArray) ;
    MemFree(G_3dSideArray) ;
    MemFree(G_3dNodeArray) ;
    MemFree(G_3dPNodeArray) ;
    MemFree(G_3dSectorArray) ;
    MemFree(G_3dVertexArray) ;
    MemFree(G_3dSegmentSectorArray) ;
    MemFree(G_3dSectorInfoArray) ;
    MemFree(G_3dBlockMapArray) ;
    MemFree(G_3dReject) ;

    G_3dSegArray = NULL ;
    G_3dLineArray = NULL ;
    G_3dSideArray = NULL ;
    G_3dNodeArray = NULL ;
    G_3dPNodeArray = NULL ;
    G_3dSectorArray = NULL ;
    G_3dVertexArray = NULL ;
    G_3dSegmentSectorArray = NULL ;
    G_3dSectorInfoArray = NULL ;
    G_3dBlockMapArray = NULL ;
    G_3dReject = NULL ;

#if 0
    /* Walk through the linked list and remove all of them. */
    p_obj = G_First3dObject ;
    while (p_obj != NULL)  {
        p_next = p_obj->nextObj ;
        View3dFreeObject(p_obj) ;
//        MemFree(p_obj) ;
        p_obj = p_next ;
    }
    DebugCheck(G_Num3dObjects == 0) ;
    G_First3dObject = G_Last3dObject = NULL ;
#endif

//puts("Unmapping sectors") ;
    View3dUnmapSectors() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadObjects
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of things from the given file.
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to objects
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadObjects(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    T_word16 i ;
    T_3dObject *p_obj ;
    T_3dObjectInFile objData ;
    T_word16 num ;
    T_3dObjectInFile *p_item ;
    E_Boolean isDay ;

    DebugRoutine("ILoadObjects") ;
    /* Allocate memory for the 3d objects. */

    /* How many objects does this become? */
    num = p_dir->size / sizeof(T_3dObjectInFile) ;

    isDay = MapIsDay() ;

    /* Seek and read in the objects. */
    FileSeek(file, p_dir->foffset) ;
//printf("Seek %ld\n", p_dir->foffset) ;
//printf("num: %ld\n", num) ;
    /* Look for the player #1 start. */
    for (i=0; i<num; i++)  {
        FileRead(file, &objData, sizeof(T_3dObjectInFile)) ;

//printf("%d) ObjType: %05d\n", i, objData.objectType) ;
//        color = objData.objectType>>12 ;
//        objData.objectType &= 0xFFF ;
        if (objData.objectType != 0)  {
            if ((objData.objectType >= OBJECT_TYPE_PLAYER_1_START) &&
                (objData.objectType <= OBJECT_TYPE_PLAYER_4_START)) {
                /* Determine the correct facing angle. */
                /* Move the viewing position to that location. */
    //printf("Jumping to %d, %d\n", objData.x, objData.y) ;
    /*
                View3dSetView(
                    objData.x,
                    objData.y,
                    40,
                    ((T_word16)objData.angle)<<13) ;
    */
                G_mapStartsX[objData.objectType-1] = objData.x ;
                G_mapStartsY[objData.objectType-1] = objData.y ;
                G_mapAngles[objData.objectType-1] = objData.angle ;
    //static T_sword16 G_mapStartsX[4] ;
    //static T_sword16 G_mapStartsY[4] ;
            } else if ((objData.objectType >= SPECIAL_OBJECTS_START) &&
                       (objData.objectType <= SPECIAL_OBJECTS_END))  {
#ifndef DONT_LOAD_SPECIALS
                p_item = MemAlloc(sizeof(T_3dObjectInFile)) ;
                DebugCheck(p_item != NULL) ;
                if (p_item)  {
                    *p_item = objData ;
                    DoubleLinkListAddElementAtEnd(G_specialObjectList, p_item) ;
                }
#endif
            } else {
#ifndef DONT_LOAD_OBJECTS
                /* Let's create the object we will be using. */
                p_obj = ObjectCreateFake() ;
                DebugCheck(p_obj != NULL) ;
                p_obj->objServerId = 1+i ;
                p_obj->orientation = ORIENTATION_NORMAL ;
                p_obj->p_picture = NULL ;

                p_obj->scaleX = p_obj->scaleY = 65536L ;
/* TESTING:  Stretch only trees. */
//if (objData.objectType == 22)  {
//    p_obj->scaleX =
//    p_obj->scaleY = 65000 + (rand() % 10000) * 4 ;
//}

                ObjMoveSetX(&p_obj->objMove, ((T_sword32)objData.x)<<16) ;
                ObjMoveSetY(&p_obj->objMove, ((T_sword32)objData.y)<<16) ;
                ObjMoveSetAngle(&p_obj->objMove, objData.angle<<13) ;
                p_obj->attributes = objData.attributes ;
                ObjectSetAccData(p_obj, objData.attributes) ;

                /* Create an object type instantiation for each object loaded. */
                ObjectSetType(p_obj, objData.objectType) ;
//                ObjectSetColorizeTable(p_obj, color) ;

                if (objData.objectType == OBJECT_TYPE_LIGHT)  {
                    p_obj->illumination = (T_sword16)(objData.attributes) ;
                    p_obj->attributes = OBJECT_ATTR_PASSABLE |
                                        OBJECT_ATTR_INVISIBLE ;
                }

                if (MapGetMapSpecial() == MAP_SPECIAL_DAY_NIGHT_CREATURES)  {
//printf("Considering object %d for day night\n", ObjectGetServerId(p_obj)) ;
                    /* Is it day or night? */
                    if (ObjectIsCreature(p_obj))  {
//printf("is %s\n", (isDay)?"day":"night") ;
                        /* Is it day or night. */
                        if (isDay)  {
                            if (ObjectGetAccData(p_obj) == 0)  {
                                /* Place daytime creature */
                                ObjectAddWithoutHistory(p_obj) ;
                            } else {
                                /* Don't place night time creatures */
                                /* when day time */
                                ObjectDestroy(p_obj) ;
                            }
                        } else {
                            if (ObjectGetAccData(p_obj) == 0)  {
                                /* Don't place day time creatures when */
                                /* night time */
                                ObjectDestroy(p_obj) ;
                            } else {
                                /* Place night time creature */
                                ObjectAddWithoutHistory(p_obj) ;
                            }
                        }
                    } else {
                        /* Not a creature, no special logic. */
                        /* Place it in our world. */
                        ObjectAddWithoutHistory(p_obj) ;
                    }
                } else {
                    /* Place it in our world. */
                    ObjectAddWithoutHistory(p_obj) ;
                }
#endif
            }
        }
/*
printf("  Object #%d loaded: type %d, x=%d, y=%d\n",
                i,
                objData.objectType,
                p_obj->objMove.X,
                p_obj->objMove.Y) ;
*/
    }

//printf("OBJECTS  : %d\n", G_Num3dObjects) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadSegs
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of line segments from the given file.
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to segments
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadSegs(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadSegs") ;

    /* Allocate memory for the 3d line segments. */
    G_3dSegArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dSegArray != NULL) ;

    /* How many segs are there? */
    G_Num3dSegs = p_dir->size / sizeof(T_3dSegment) ;

    /* Seek and read in the segments. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dSegArray, p_dir->size) ;

//printf("SEGMENTS : %d\n", G_Num3dSegs) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadSides
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of line sides from the given file.
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to sides
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadSides(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadSides") ;

    /* Allocate memory for the 3d line sides. */
    G_3dSideArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dSideArray != NULL) ;

    /* How many sides are there? */
    G_Num3dSides = p_dir->size / sizeof(T_3dSide) ;

    /* Seek and read in the sides. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dSideArray, p_dir->size) ;

//printf("SIDEDEFS : %d\n", G_Num3dSides) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadLines
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of lines from the given file
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to lines
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadLines(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadLines") ;

    /* Allocate memory for the 3d lines. */
    G_3dLineArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dLineArray != NULL) ;

    /* How many lines are there? */
    G_Num3dLines = p_dir->size / sizeof(T_3dLine) ;

    /* Seek and read in the lines. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dLineArray, p_dir->size) ;

//printf("LINEDEFS : %d\n", G_Num3dLines) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadNodes
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of nodes from the given file
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to lines
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadNodes(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    T_word16 i ;

    DebugRoutine("ILoadNodes") ;

    /* Allocate memory for the 3d lines. */
    G_3dNodeArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dNodeArray != NULL) ;

    /* How many nodes are there? */
    G_Num3dNodes = p_dir->size / sizeof(T_3dNode) ;

    /* Allocate memory for the pnodes. */
    G_3dPNodeArray = MemAlloc(sizeof(T_3dNode *) * G_Num3dNodes) ;

    /* Use the number to find the root BSP node. */
    G_3dRootBSPNode = G_Num3dNodes-1 ;

    /* Calculate a group of pointers to these nodes. */
    /* This should make accessing faster. */
    for (i=0; i<G_Num3dNodes; i++)
        G_3dPNodeArray[i] = &G_3dNodeArray[i] ;

    /* Seek and read in the nodes. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dNodeArray, p_dir->size) ;

//printf("NODES    : %d\n", G_Num3dNodes) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadSectors
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of sectors from the given file
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to sectors
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadSectors(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    T_word16 i ;

    DebugRoutine("ILoadSectors") ;

    /* Allocate memory for the 3d sectors. */
    G_3dSectorArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dSectorArray != NULL) ;

    /* How many nodes are there? */
    G_Num3dSectors = p_dir->size / sizeof(T_3dSector) ;

    /* Seek and read in the nodes. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dSectorArray, p_dir->size) ;

    G_3dSectorInfoArray =
        (T_3dSectorInfo *)MemAlloc(G_Num3dSectors * sizeof(T_3dSectorInfo)) ;

    memset(G_3dSectorInfoArray, 0, G_Num3dSectors * sizeof(T_3dSectorInfo)) ;

    for (i=0; i<G_Num3dSectors; i++)  {
        G_3dSectorInfoArray[i].gravity = 4 << 8 ;
        G_3dSectorInfoArray[i].friction = 12 ;
        G_3dSectorInfoArray[i].ceilingLimit = 0x7FFF ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ILoadVertexes
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of vertexes from the given file
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to vertexes
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadVertexes(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadVertexes") ;

    /* Allocate memory for the 3d sectors. */
    G_3dVertexArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dVertexArray != NULL) ;

    /* How many nodes are there? */
    G_Num3dVertexes = p_dir->size / sizeof(T_3dVertex) ;

    /* Seek and read in the nodes. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dVertexArray, p_dir->size) ;

//printf("VERTEXES : %d\n", G_Num3dVertexes) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadSegmentSectors
 *-------------------------------------------------------------------------*/
/**
 *  Loads a group of segment sectors from the given file.
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to segment
 *      sectors.
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadSegmentSectors(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadSegmentSectors") ;

    /* Allocate memory for the 3d sectors. */
    G_3dSegmentSectorArray = MemAlloc(p_dir->size) ;
    DebugCheck(G_3dSegmentSectorArray != NULL) ;

    /* How many nodes are there? */
    G_Num3dSSectors = p_dir->size / sizeof(T_3dSegmentSector) ;

    /* Seek and read in the nodes. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dSegmentSectorArray, p_dir->size) ;

//printf("SSECTORS : %d\n", G_Num3dSSectors) ;
    DebugEnd() ;
}


#ifndef NDEBUG
static T_void IDumpData(T_void)
{
    FILE *fp ;
    T_word32 i ;

    fp = fopen("dump.dat", "w") ;

    /* Segments */
    fprintf(fp, "\nSegments:\n") ;
    fprintf(fp, "####  from  to: ang: line liSi lOff\n") ;
    fprintf(fp, "----  ---- ---- ---- ---- ---- ----\n") ;
    for (i=0; i<G_Num3dSegs; i++)  {
        fprintf(fp, "%4X) %4X %4X %4X %4X %4X %4X\n",
            i,
            G_3dSegArray[i].from,
            G_3dSegArray[i].to,
            G_3dSegArray[i].angle,
            G_3dSegArray[i].line,
            G_3dSegArray[i].lineSide,
            G_3dSegArray[i].lineOffset) ;
    }

    fprintf(fp, "\nSides:\n") ;
    fprintf(fp, "####  tX   tY   upper:   lower:   main:    sect\n") ;
    fprintf(fp, "----  ---- ---- -------- -------- -------- ----\n") ;
    for (i=0; i<G_Num3dSides; i++)  {
        fprintf(fp, "%4X) %4X %4X %-8.8s %-8.8s %-8.8s %4X\n",
            i,
            G_3dSideArray[i].tmXoffset,
            G_3dSideArray[i].tmYoffset,
            G_3dSideArray[i].upperTx,
            G_3dSideArray[i].lowerTx,
            G_3dSideArray[i].mainTx,
            G_3dSideArray[i].sector) ;
    }

    fprintf(fp, "\nNodes:\n") ;
    fprintf(fp, "####  X    Y    dx:  dy:  left right\n") ;
    fprintf(fp, "----  ---- ---- ---- ---- ---- -----\n") ;
    for (i=0; i<G_Num3dNodes; i++)  {
        fprintf(fp, "%4X) %4X %4X %4X %4X %4X %4X\n",
            i,
            G_3dNodeArray[i].x&0xFFFF,
            G_3dNodeArray[i].y&0xFFFF,
            G_3dNodeArray[i].dx&0xFFFF,
            G_3dNodeArray[i].dy&0xFFFF,
            G_3dNodeArray[i].left&0xFFFF,
            G_3dNodeArray[i].right&0xFFFF) ;
    }

    fprintf(fp, "\nSectors:\n") ;
    fprintf(fp, "####  flrH ceiH floor    ceiling: lght type\n") ;
    fprintf(fp, "----  ---- ---- -------- -------- ---- ----\n") ;
    for (i=0; i<G_Num3dSectors; i++)  {
        fprintf(fp, "%4X) %4X %4X %-8.8s %-8.8s %4X %4X\n",
            i,
            G_3dSectorArray[i].floorHt&0xFFFF,
            G_3dSectorArray[i].ceilingHt&0xFFFF,
            G_3dSectorArray[i].floorTx,
            G_3dSectorArray[i].ceilingTx,
            G_3dSectorArray[i].light&0xFFFF,
            G_3dSectorArray[i].type&0xFFFF) ;
    }

    fprintf(fp, "\nSSectors:\n") ;
    fprintf(fp, "####  num  first\n") ;
    fprintf(fp, "----  ---- -----\n") ;
    for (i=0; i<G_Num3dSSectors; i++)  {
        fprintf(fp, "%4X) %4X %4X\n",
            i,
            G_3dSegmentSectorArray[i].numSegs&0xFFFF,
            G_3dSegmentSectorArray[i].firstSeg&0xFFFF) ;
    }

    fprintf(fp, "\nLines:\n") ;
    fprintf(fp, "####  from to:  sid0 sid1\n") ;
    fprintf(fp, "----  ---- ---- ---- ----\n") ;
    for (i=0; i<G_Num3dLines; i++)  {
        fprintf(fp, "%4X) %4X %4X %4X %4X\n",
            i,
            G_3dLineArray[i].from&0xFFFF,
            G_3dLineArray[i].to&0xFFFF,
            G_3dLineArray[i].side[0]&0xFFFF,
            G_3dLineArray[i].side[1]&0xFFFF) ;
    }

    fclose(fp) ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ILoadBlockMap
 *-------------------------------------------------------------------------*/
/**
 *  Loads the block map of the given file
 *
 *  @param file -- File to read from
 *  @param p_dir -- Directory entry refering to vertexes
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadBlockMap(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadBlockMap") ;

    /* Allocate memory for the 3d sectors. */
    (G_3dBlockMapArray = (T_sword16 *)MemAlloc(p_dir->size)) ;
    G_3dBlockMapHeader = (T_3dBlockMapHeader *)G_3dBlockMapArray ;
    DebugCheck(G_3dBlockMapArray != NULL) ;

    /* How many nodes are there? */
    G_BlockmapSize = p_dir->size ;

    /* Seek and read in the nodes. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dBlockMapArray, p_dir->size) ;

#ifndef NDEBUG
#if 0
fp = fopen("blockmap.dat", "w") ;
fprintf(fp, "x origin = %d\n", G_3dBlockMapHeader->xOrigin) ;
fprintf(fp, "y origin = %d\n", G_3dBlockMapHeader->yOrigin) ;
fprintf(fp, "columns = %d\n", G_3dBlockMapHeader->columns) ;
fprintf(fp, "rows = %d\n", G_3dBlockMapHeader->rows) ;

fprintf(fp, "\nDump:\n") ;
for (i=0; i<(p_dir->size/2); i++)  {
  fprintf(fp, "%4X: %4X\n", i, (G_3dBlockMapArray[i]&0xFFFF)) ;
}
fclose(fp) ;
#endif
#endif

//printf("BLOCKMAP: %d\n", p_dir->size) ;
    DebugEnd() ;
}

T_void IPrepareObjects(T_void)
{
    T_3dObject *p_obj ;
    T_sword16 x, y ;

    DebugRoutine("IPrepareObjects") ;

    p_obj = G_First3dObject ;
    while (p_obj != NULL)  {
        /* At first it may seem stupid to teleport to the same location */
        /* as you already are, but there is a very important list of */
        /* sectors that must be updated, and at this point that list */
        /* has not been updated.  Teleporting does it. */
        x = ObjMoveGetX(&p_obj->objMove)>>16 ;
        y = ObjMoveGetY(&p_obj->objMove)>>16 ;

        /* Collect information about sectors. */
        ObjectTeleportAlways(p_obj, x, y) ;
//        ObjectSetMoveFlags(p_obj, OBJMOVE_FLAG_FAST_MOVEMENT) ;
        ObjectClearMovedFlag(p_obj) ;

        p_obj = p_obj->nextObj ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILockPictures
 *-------------------------------------------------------------------------*/
/**
 *  ILockPictures goes through all the sides and sectors looking for
 *  textures and locks those textures into memory.  These textures will
 *  later be used to decide what are the floor, ceiling, and walls will
 *  look like.
 *
 *<!-----------------------------------------------------------------------*/
/* Hash pattern used for no textures */
static T_byte8 G_textureNone[] = {
    2, 0, 2, 0, 31, 226, 226, 31,
    2, 0, 2, 0, 31, 226, 226, 31,
    2, 0, 2, 0, 31, 226, 226, 31,
    2, 0, 2, 0, 31, 226, 226, 31,
    2, 0, 2, 0, 31, 226, 226, 31,
    2, 0, 2, 0, 31, 226, 226, 31,
} ;
static T_void ILockPictures(T_void)
{
    T_word16 i ;
    T_byte8 name[20] ;
    T_3dSide *p_side ;
    T_3dSector *p_sector ;

    DebugRoutine("ILockPictures") ;

    name[8] = '\0' ;
    name[9] = '\0' ;

    /* Allocate memory for texture resource handles. */
    G_3dUpperResourceArray = (T_resource *)
        MemAlloc(sizeof(T_resource) * G_Num3dSides) ;
    G_3dLowerResourceArray = (T_resource *)
        MemAlloc(sizeof(T_resource) * G_Num3dSides) ;
    G_3dMainResourceArray = (T_resource *)
        MemAlloc(sizeof(T_resource) * G_Num3dSides) ;
    G_3dFloorResourceArray = (T_resource *)
        MemAlloc(sizeof(T_resource) * G_Num3dSectors) ;
    G_3dCeilingResourceArray = (T_resource *)
        MemAlloc(sizeof(T_resource) * G_Num3dSectors) ;

//#ifdef MIP_MAPPING_ON
#if 0
    DebugCheck(G_textureHash == HASH32_BAD) ;
    G_textureHash = Hash32Create(256) ;

    /* Look for textures on sides. */
    for (i=0; i<G_Num3dSides; i++)  {
        p_side = &G_3dSideArray[i] ;

        if (p_side->upperTx[0] != '-')  {
            strncpy(name, p_side->upperTx, 8) ;
            p_texture =
                PictureLock(name, &G_3dUpperResourceArray[i]) ;
            *((T_byte8 **)(&p_side->upperTx[1])) =
                MipMap(p_texture) ;
        } else {
            G_3dUpperResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->upperTx[1])) = G_textureNone+4 ;
        }

        if (p_side->lowerTx[0] != '-')  {
            strncpy(name, p_side->lowerTx, 8) ;
            p_texture =
                PictureLock(name, &G_3dLowerResourceArray[i]) ;
            *((T_byte8 **)(&p_side->lowerTx[1])) =
                MipMap(p_texture) ;
        } else {
            G_3dLowerResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->lowerTx[1])) = G_textureNone+4 ;
        }

        if (p_side->mainTx[0] != '-')  {
            strncpy(name, p_side->mainTx, 8) ;
            p_texture =
                PictureLock(name, &G_3dMainResourceArray[i]) ;
            *((T_byte8 **)(&p_side->mainTx[1])) =
                MipMap(p_texture) ;
        } else {
            G_3dMainResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->mainTx[1])) = G_textureNone+4 ;
        }
    }

    /* Look for all the textures on the sectors. */
    for (i=0; i<G_Num3dSectors; i++)  {
        p_sector = &G_3dSectorArray[i] ;

        if (p_sector->floorTx[0] != '-')  {
            strncpy(name, p_sector->floorTx, 8) ;
            p_texture =
                PictureLock(name, &G_3dFloorResourceArray[i]) ;
            *((T_byte8 **)(&p_sector->floorTx[1])) =
                MipMap(p_texture) ;
        } else {
            G_3dFloorResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_sector->floorTx[1])) = G_textureNone+4 ;
        }
        if (p_sector->ceilingTx[0] != '-')  {
            strncpy(name, p_sector->ceilingTx, 8) ;
            p_texture =
                PictureLock(name, &G_3dCeilingResourceArray[i]) ;
            *((T_byte8 **)(&p_sector->ceilingTx[1])) =
                MipMap(p_texture) ;
        } else {
            G_3dCeilingResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_sector->ceilingTx[1])) = G_textureNone+4 ;
        }
    }
#else
    /* Look for textures on sides. */
    for (i=0; i<G_Num3dSides; i++)  {
        p_side = &G_3dSideArray[i] ;

        if (p_side->upperTx[0] != '-')  {
            strncpy(name, p_side->upperTx, 8) ;
            *((T_byte8 **)(&p_side->upperTx[1])) =
                PictureLock(name, &G_3dUpperResourceArray[i]) ;
//printf("!A 1 %s\n", name) ;
//printf("!A %ld %s_s\n", ResourceGetSize(G_3dUpperResourceArray[i]), name) ;
        } else {
            G_3dUpperResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->upperTx[1])) = G_textureNone+4 ;
        }

        if (p_side->lowerTx[0] != '-')  {
            strncpy(name, p_side->lowerTx, 8) ;
            *((T_byte8 **)(&p_side->lowerTx[1])) =
                PictureLock(name, &G_3dLowerResourceArray[i]) ;
//printf("!A 1 %s\n", name) ;
//printf("!A %ld %s_s\n", ResourceGetSize(G_3dLowerResourceArray[i]), name) ;
        } else {
            G_3dLowerResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->lowerTx[1])) = G_textureNone+4 ;
        }

        if (p_side->mainTx[0] != '-')  {
            strncpy(name, p_side->mainTx, 8) ;
            *((T_byte8 **)(&p_side->mainTx[1])) =
                PictureLock(name, &G_3dMainResourceArray[i]) ;
//printf("!A 1 %s\n", name) ;
//printf("!A %ld %s_s\n", ResourceGetSize(G_3dMainResourceArray[i]), name) ;
        } else {
            G_3dMainResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_side->mainTx[1])) = G_textureNone+4 ;
        }
    }

    /* Look for all the textures on the sectors. */
    for (i=0; i<G_Num3dSectors; i++)  {
        p_sector = &G_3dSectorArray[i] ;

        if (p_sector->floorTx[0] != '-')  {
            strncpy(name, p_sector->floorTx, 8) ;
            *((T_byte8 **)(&p_sector->floorTx[1])) =
                PictureLock(name, &G_3dFloorResourceArray[i]) ;
//printf("!A 1 %s\n", name) ;
//printf("!A %ld %s_s\n", ResourceGetSize(G_3dFloorResourceArray[i]), name) ;
        } else {
            G_3dFloorResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_sector->floorTx[1])) = G_textureNone+4 ;
        }
        if (p_sector->ceilingTx[0] != '-')  {
            strncpy(name, p_sector->ceilingTx, 8) ;
            // Open sky?
            if (strncmp(name, "F_SKY", 5)==0) {
                // Set the sky attribute
                p_sector->trigger |= 1;
            }
            *((T_byte8 **)(&p_sector->ceilingTx[1])) =
                PictureLock(name, &G_3dCeilingResourceArray[i]) ;
//printf("!A 1 %s\n", name) ;
//printf("!A %ld %s_s\n", ResourceGetSize(G_3dCeilingResourceArray[i]), name) ;
        } else {
            G_3dCeilingResourceArray[i] = RESOURCE_BAD ;
            *((T_byte8 **)(&p_sector->ceilingTx[1])) = G_textureNone+4 ;
        }
    }
#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUnlockPictures
 *-------------------------------------------------------------------------*/
/**
 *  IUnlockPictures is the opposite to ILockPictures.  All data that
 *  was locked into memory must be released when done with the map.
 *  This routine goes through all sides and sectors and unlocks the
 *  pictures from memory.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUnlockPictures(T_void)
{
    T_word16 i ;
    T_3dSide *p_side ;
    T_3dSector *p_sector ;

    DebugRoutine("IUnlockPictures") ;

//#ifdef MIP_MAPPING_ON
#if 0
    /* Look for textures on sides. */
    for (i=0; i<G_Num3dSides; i++)  {
        p_side = &G_3dSideArray[i] ;

        if (p_side->upperTx[0] != '-')  {
            p_res = PictureLockQuick(G_3dUpperResourceArray[i]) ;
            PictureUnlock(G_3dUpperResourceArray[i]) ;
            PictureUnlockAndUnfind(G_3dUpperResourceArray[i]) ;
            ReleaseMipMap(p_res) ;
        }
        if (p_side->lowerTx[0] != '-')  {
            p_res = PictureLockQuick(G_3dLowerResourceArray[i]) ;
            PictureUnlock(G_3dLowerResourceArray[i]) ;
            PictureUnlockAndUnfind(G_3dLowerResourceArray[i]) ;
            ReleaseMipMap(p_res) ;
        }
        if (p_side->mainTx[0] != '-')  {
            p_res = PictureLockQuick(G_3dMainResourceArray[i]) ;
            PictureUnlock(G_3dMainResourceArray[i]) ;
            PictureUnlockAndUnfind(G_3dMainResourceArray[i]) ;
            ReleaseMipMap(p_res) ;
        }
    }

    /* Look for all the textures on the sectors. */
    for (i=0; i<G_Num3dSectors; i++)  {
        p_sector = &G_3dSectorArray[i] ;

        if (p_sector->floorTx[0] != '-')  {
            p_res = PictureLockQuick(G_3dFloorResourceArray[i]) ;
            PictureUnlock(G_3dFloorResourceArray[i]) ;
            PictureUnlockAndUnfind(G_3dFloorResourceArray[i]) ;
            ReleaseMipMap(p_res) ;
        }
        if (p_sector->ceilingTx[0] != '-')  {
            p_res = PictureLockQuick(G_3dCeilingResourceArray[i]) ;
            PictureUnlock(G_3dCeilingResourceArray[i]) ;
            PictureUnlockAndUnfind(G_3dCeilingResourceArray[i]) ;
            ReleaseMipMap(p_res) ;
        }
    }

    Hash32Destroy(G_textureHash) ;
    G_textureHash = HASH32_BAD ;
#else
    /* Look for textures on sides. */
    for (i=0; i<G_Num3dSides; i++)  {
        p_side = &G_3dSideArray[i] ;

        if (p_side->upperTx[0] != '-')
            PictureUnlockAndUnfind(G_3dUpperResourceArray[i]) ;
        if (p_side->lowerTx[0] != '-')
            PictureUnlockAndUnfind(G_3dLowerResourceArray[i]) ;
        if (p_side->mainTx[0] != '-')
            PictureUnlockAndUnfind(G_3dMainResourceArray[i]) ;
    }

    /* Look for all the textures on the sectors. */
    for (i=0; i<G_Num3dSectors; i++)  {
        p_sector = &G_3dSectorArray[i] ;

        if (p_sector->floorTx[0] != '-')
            PictureUnlockAndUnfind(G_3dFloorResourceArray[i]) ;
        if (p_sector->ceilingTx[0] != '-')
            PictureUnlockAndUnfind(G_3dCeilingResourceArray[i]) ;
    }
#endif

    MemFree(G_3dUpperResourceArray) ;
    MemFree(G_3dLowerResourceArray) ;
    MemFree(G_3dMainResourceArray) ;
    MemFree(G_3dCeilingResourceArray) ;
    MemFree(G_3dFloorResourceArray) ;

    DebugEnd() ;
}

static T_void ILoadReject(
                  T_file file,
                  T_directoryEntry *p_dir)
{
    DebugRoutine("ILoadReject") ;

    /* Allocate memory for the reject data. */
    G_3dReject = (T_byte8 *)MemAlloc(p_dir->size) ;
    DebugCheck(G_3dReject != NULL) ;

    /* Seek and read in the reject data. */
    FileSeek(file, p_dir->foffset) ;
    FileRead(file, G_3dReject, p_dir->size) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILoadSectorInfo
 *-------------------------------------------------------------------------*/
/**
 *  ILoadSectorInfo reads in all the accompany information data that      *//*  was locked into memory must be released when done with the map.
 *  This routine goes through all sides and sectors and unlocks the
 *  pictures from memory.
 *
 *  @param mapName -- Name of .SEC file
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILoadSectorInfo(T_byte8 *mapName)
{
    T_word16 sector, sector2 ;
    T_byte8 buffer[81] ;
    T_word16 mode = 0 ;
    T_word16 submode = 0 ;
    T_sword32 var ;
    T_word16 lineNum = 0 ;
    T_word16 nextMap, nextMapStart ;
    T_sword16 ceilingLimit ;
    T_word16 sectorType ;
    T_word16 damageAmount, damageType ;
    FILE *fp ;

    DebugRoutine("ILoadSectorInfo") ;

    fp = fopen(mapName, "r") ;
    DebugCheck(fp != NULL) ;

    fgets(buffer, 80, fp) ;
    lineNum++ ;
    while (!feof(fp))  {
        if (toupper(buffer[0]) == 'E')
            break ;
        if (mode == 0)  {
            switch(toupper(buffer[0]))  {
                case 'S':          /* Sector info. */
                    sscanf(buffer+1, "%hd", &sector) ;
//printf ("Reading sector info for %d\n", sector);
                    DebugCheck(sector < G_Num3dSectors) ;
                    if (sector < G_Num3dSectors)  {
                        mode = 1 ;
                        submode = 0 ;
                    }
                    break ;
                case 'C':          /* Copy sector info. */
                    sscanf(buffer+1, "%hd%hd", &sector, &sector2) ;
//printf ("Copying sector info from %d to %d\n", sector, sector2);
                    DebugCheck(sector < G_Num3dSectors) ;
                    DebugCheck(sector2 < G_Num3dSectors) ;
                    if (sector < G_Num3dSectors)  {
                        if (sector2 < G_Num3dSectors)  {
                            G_3dSectorInfoArray[sector2] =
                                G_3dSectorInfoArray[sector] ;
                        }
                    }
                    break ;
                case 'L':       /* Link to other maps. */
                    sscanf(
                        buffer+1,
                        "%hd%hd%hd",
                        &sector,
                        &nextMap,
                        &nextMapStart) ;
                    DebugCheck(sector < G_Num3dSectors) ;
                    if (sector < G_Num3dSectors)  {
                        G_3dSectorInfoArray[sector].nextMap = nextMap ;
                        G_3dSectorInfoArray[sector].nextMapStart =
                            nextMapStart ;
                    }
                    break ;
                case 'U':       /* Upper limit to ceiling. */
                    sscanf(
                        buffer+1,
                        "%hd%hd",
                        &sector,
                        &ceilingLimit) ;
                    DebugCheck(sector < G_Num3dSectors) ;
                    if (sector < G_Num3dSectors)  {
                        G_3dSectorInfoArray[sector].ceilingLimit =
                            ceilingLimit ;
                    }
                    break ;
                case 'T':      /* Sector type (standard setups) */
                    sscanf(
                        buffer+1,
                        "%hd%hd",
                        &sector,
                        &sectorType) ;
                    View3dSetSectorType(sector, sectorType) ;
                    break ;
                case 'D':      /* Damage and damage type for sector. */
                    sscanf(
                        buffer+1,
                        "%hd%hd%hd",
                        &sector,
                        &damageAmount,
                        &damageType) ;
                    DebugCheck(sector < G_Num3dSectors) ;
                    if (sector < G_Num3dSectors)  {
                        G_3dSectorInfoArray[sector].damage = damageAmount ;
                        G_3dSectorInfoArray[sector].damageType = (T_byte8)damageType ;
                    }

                    break ;
                case '#':      /* Allow comments in the sector info file. */
                    break ;
                default:
#ifndef NDEBUG
                    printf("Illegal line %d in .SEC file\n", lineNum) ;
#endif
                    DebugCheck(FALSE) ;
                    break ;
            }
        } else {
            sscanf(buffer, "%d", &var) ;
            switch(submode)  {
                case 0:
                    G_3dSectorInfoArray[sector].textureXOffset = var ;
                    break ;
                case 1:
                    G_3dSectorInfoArray[sector].textureYOffset = var ;
                    break ;
                case 2:
                    G_3dSectorInfoArray[sector].xVelAdd = var ;
                    break ;
                case 3:
                    G_3dSectorInfoArray[sector].yVelAdd = var ;
                    break ;
                case 4:
                    G_3dSectorInfoArray[sector].zVelAdd = var ;
                    break ;
                case 5:
                    G_3dSectorInfoArray[sector].gravity = var ;
                    break ;
                case 6:
                    G_3dSectorInfoArray[sector].friction = var ;
                    break ;
                case 7:
                    G_3dSectorInfoArray[sector].damage = var ;
                    break ;
                case 8:
                    G_3dSectorInfoArray[sector].depth = var ;
                    break ;
                case 9:
                    G_3dSectorInfoArray[sector].enterSound = var ;
                    break ;
                case 10:
                    G_3dSectorInfoArray[sector].enterSoundRadius = var ;
                    break ;
                case 11:
                    G_3dSectorInfoArray[sector].lightAnimationType = var;
                    G_3dSectorInfoArray[sector].lightAnimationState = 0;
                    break;
                case 12:
                    G_3dSectorInfoArray[sector].lightAnimationRate = var;
                    break;
                case 13:
                    G_3dSectorInfoArray[sector].lightAnimationCenter = var;
                    break;
                case 14:
                    G_3dSectorInfoArray[sector].lightAnimationRadius = var;
                    /** End of sector description. **/
                    mode = 0 ;
                    break ;
            }
            submode++ ;
        }
        fgets(buffer, 80, fp) ;
    }

    fclose(fp) ;

    DebugEnd() ;
}

T_void View3dSetSectorType(T_word16 sector, T_word16 sectorType)
{
    static T_3dSectorInfo sectorInfoTypes[NUM_SECTOR_TYPES] = {
        {     /* Type 1:  SECTOR_TYPE_DIRT */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              12,     /* friction  */
              0,      /* damage  */
              0,      /* damageType  */
              0,      /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              0,      /* enterSound  */
              0,      /* enterSoundRadius  */
              0,      /* lightAnimationType */
              0,      /* lightAnimationRate */
              0,      /* lightAnimationCenter */
              0,      /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_DIRT,      /* type  */
        },
        {     /* Type 2:  SECTOR_TYPE_ICE */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              2,      /* friction  */
              0,      /* damage  */
              0,      /* damageType  */
              0,      /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              0,      /* enterSound  */
              0,      /* enterSoundRadius  */
              0,      /* lightAnimationType */
              0,      /* lightAnimationRate */
              0,      /* lightAnimationCenter */
              0,      /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_ICE,      /* type  */
        },
        {     /* Type 4:  SECTOR_TYPE_LAVA */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              24,     /* friction  */
              200,    /* damage  */
              EFFECT_DAMAGE_NORMAL | EFFECT_DAMAGE_FIRE,  /* damageType  */
              20,     /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              0,      /* enterSound  */
              0,      /* enterSoundRadius  */
              1,      /* lightAnimationType */
              40,     /* lightAnimationRate */
              128,    /* lightAnimationCenter */
              64,     /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_LAVA,      /* type  */
        },
        {     /* Type 8:  SECTOR_TYPE_WATER */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              16,     /* friction  */
              0,      /* damage  */
              0,      /* damageType  */
              VIEW_WATER_DIP_LEVEL,   /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              6002,   /* enterSound  */
              500,    /* enterSoundRadius  */
              0,      /* lightAnimationType */
              0,      /* lightAnimationRate */
              0,      /* lightAnimationCenter */
              0,      /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_WATER,      /* type  */
        },
        {     /* Type 16:  SECTOR_TYPE_MUD */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              14,     /* friction  */
              0,      /* damage  */
              0,      /* damageType  */
              5,      /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              0,      /* enterSound  */
              0,      /* enterSoundRadius  */
              0,      /* lightAnimationType */
              0,      /* lightAnimationRate */
              0,      /* lightAnimationCenter */
              0,      /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_MUD,     /* type  */
        },
        {     /* Type 32:  SECTOR_TYPE_ACID */
              0,      /* xVelAdd  */
              0,      /* yVelAdd  */
              0,      /* zVelAdd  */
              1024,   /* gravity  */
              24,     /* friction  */
              200,    /* damage  */
              EFFECT_DAMAGE_NORMAL | EFFECT_DAMAGE_ACID,  /* damageType  */
              30,     /* depth  */
              0,      /* textureXOffset  */
              0,      /* textureYOffset  */
              0,      /* enterSound  */
              0,      /* enterSoundRadius  */
              1,      /* lightAnimationType */
              20,     /* lightAnimationRate */
              192,    /* lightAnimationCenter */
              32,     /* lightAnimationRadius */
              0,      /* lightAnimationState */
              0x7FFE, /* ceilingLimit  */
              0,      /* nextMap  */
              0,      /* nextMapStart  */
              SECTOR_TYPE_ACID,      /* type  */
        },
    } ;

    DebugRoutine("View3dSetSectorType") ;

    DebugCheck(sector < G_Num3dSectors) ;
    DebugCheck(sectorType < SECTOR_TYPE_UNKNOWN) ;
    if (sector < G_Num3dSectors)  {
        sectorType = MathPower2Lookup(sectorType) ;
        DebugCheck(sectorType < NUM_SECTOR_TYPES) ;
        G_3dSectorInfoArray[sector] =
            sectorInfoTypes[sectorType] ;
        G_3dSectorInfoArray[sector].type = 1<<sectorType ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetStartLocation
 *-------------------------------------------------------------------------*/
/**
 *  ILoadSectorInfo reads in all the accompany information data that      *//*  was locked into memory must be released when done with the map.
 *  This routine goes through all sides and sectors and unlocks the
 *  pictures from memory.
 *
 *  @param num -- Number of map location (0-3)
 *  @param p_y -- Pointer to coordinate values
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dGetStartLocation(
           T_word16 num,
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_word16 *p_angle)
{
    DebugRoutine("View3dGetStartLocation") ;
    DebugCheck(num < 4) ;
    DebugCheck(p_x != NULL) ;
    DebugCheck(p_y != NULL) ;
//printf("Getting start location %d (%s)\n", num, DebugGetCallerName()) ;

    *p_x = G_mapStartsX[num] ;
    *p_y = G_mapStartsY[num] ;
    *p_angle = G_mapAngles[num] * 0x2000 ;

    DebugEnd() ;
}

#ifdef MIP_MAPPING_ON
#if 0
/* Create a mip map texture from a pointer to a texture and return */
/* the pointer to the new mip map item. */
T_byte8 *MipMap(T_byte8 *p_texture)
{
    T_word16 sizeX, sizeY ;
    T_hash32Item item ;
    T_word32 memNeeded ;
    T_byte8 *p_mipTexture = NULL ;
    T_byte8 *p_work ;
    T_byte8 *p_work2 ;

    DebugRoutine("MipMap") ;
    DebugCheck(p_texture != NULL) ;
    DebugCheck(G_textureHash != HASH32_BAD) ;

//printf("!A 1 MipMap:%p\n", p_texture) ;
//printf("MipMap %p ", p_texture) ;
    /* First, see if there is already an entry in the hash table. */
    item = Hash32Find(G_textureHash, (T_word32)p_texture) ;
    if (item != HASH32_ITEM_BAD)  {
        /* Found a match! */
        p_mipTexture = Hash32ItemGetData(item) ;

        /* Skip the sizing info. */
        p_mipTexture+=4 ;
//puts("no") ;
    } else {
        /* No match.  We need to creature this texture entry. */

        /* We first need to know how big the original texture is. */
        PictureGetXYSize(p_texture, &sizeX, &sizeY) ;

        /* Calculate the space necessary. */
        memNeeded =
            200+ 20 + sizeX * sizeY +
            (sizeX>>1) * (sizeY>>1) +
            (sizeX>>2) * (sizeY>>2) +
            (sizeX>>3) * (sizeY>>3) +
            (sizeX>>4) * (sizeY>>4) ;
        p_mipTexture = MemAlloc(memNeeded) ;
        DebugCheck(p_mipTexture != NULL) ;
        if (p_mipTexture != NULL)  {
            p_work = p_mipTexture ;
            /* Copy over the main texture (with size info). */
            memcpy(p_work, p_texture-4, 4+(sizeX * sizeY)) ;
            p_work += 4 ;
            p_work2 = p_work + (sizeX * sizeY) ;

            /* Now create the shrunk image for #2. */
            memcpy(p_work2, p_texture-4, 4) ;
            p_work2+=4 ;
            IShrinkTexture(p_work2, p_work, sizeX, sizeY) ;

            p_work = p_work2 ;
            p_work2 += ((sizeX>>1) * (sizeY>>1)) ;

            /* Now create the shrunk image for #3. */
            memcpy(p_work2, p_texture-4, 4) ;
            p_work2+= 4;
            IShrinkTexture(p_work2, p_work, (sizeX>>1), (sizeY>>1)) ;

            p_work = p_work2 ;
            p_work2 += ((sizeX>>2) * (sizeY>>2)) ;

            /* Now create the shrunk image for #4. */
            memcpy(p_work2, p_texture-4, 4) ;
            p_work2+= 4;
            IShrinkTexture(p_work2, p_work, (sizeX>>2), (sizeY>>2)) ;

            p_work = p_work2 ;
            p_work2 += ((sizeX>>3) * (sizeY>>3)) ;

            /* Now create the shrunk image for #5. */
            memcpy(p_work2, p_texture-4, 4) ;
            p_work2+= 4;
            IShrinkTexture(p_work2, p_work, (sizeX>>3), (sizeY>>3)) ;

            /* Done with shrinks. */
            /* Add this mip map to the hash table. */
//printf("!A 1 Hash32Add:%p\n", p_texture) ;
            Hash32Add(G_textureHash, (T_word32)p_texture, p_mipTexture) ;

            /* Offset the texture a bit. */
            p_mipTexture += 4;
        }
//puts("yes") ;
//printf("!A %ld textureData \n", sizeX * sizeY) ;
    }

    DebugCheck(p_mipTexture != NULL) ;
    DebugEnd() ;

    return p_mipTexture ;
}

//#define SHRINK_SHOWS_AND_WAITS
static T_void IShrinkTexture(
                  T_byte8 *p_to,
                  T_byte8 *p_from,
                  T_word16 sizeX,
                  T_word16 sizeY)
{
    T_word16 x, y ;
    T_byte8 c1, c2, c3, c4, c ;
#ifdef SHRINK_SHOWS_AND_WAITS
    T_byte8 *p_screen = ((char *)(0xA0000 + 1284)) ;

    for (y=0; y<sizeY; y++)  {
        for (x=0; x<sizeX; x++)  {
            p_screen[y*320+x] = p_from[y*sizeX+x] ;
        }
    }
#endif

    for (y=0; y<sizeY; y+=2, p_from+=sizeX)  {
        for (x=0; x<sizeX; x+=2, p_from+=2, p_to++)  {
            c1 = p_from[0] ;
            c2 = p_from[1] ;
            c1 = G_translucentTable[c1][c2] ;
            c3 = p_from[sizeX] ;
            c4 = p_from[sizeX+1] ;
            c3 = G_translucentTable[c3][c4] ;
            c = G_translucentTable[c1][c3] ;
            *p_to = c ;
#ifdef SHRINK_SHOWS_AND_WAITS
            p_screen[x>>1] = c ;
#endif
        }
#ifdef SHRINK_SHOWS_AND_WAITS
        p_screen += 320 ;
#endif
    }

#ifdef SHRINK_SHOWS_AND_WAITS
    while (KeyboardBufferGet() == 0)
       {}
#endif
}

T_void ReleaseMipMap(T_byte8 *p_mipmap)
{
    T_hash32Item item ;
    T_byte8 *p_data ;

    DebugRoutine("ReleaseMipMap") ;
    DebugCheck(p_mipmap != NULL) ;
    DebugCheck(G_textureHash != HASH32_BAD) ;

//printf("!A 1 ReleaseMipMap:%p\n", p_mipmap) ;  fflush(stdout);
    item = Hash32Find(G_textureHash, (T_word32)p_mipmap) ;
    if (item != HASH32_ITEM_BAD)  {
        /* Found the item */
        p_data = (T_byte8 *)Hash32ItemGetData(item) ;
        MemFree(p_data) ;

        /* Remove the item from the list. */
        Hash32Remove(item) ;
    }

    DebugEnd() ;
}
#endif
#endif

/*-------------------------------------------------------------------------*
 * Routine:  View3dResolveSpecialObjects
 *-------------------------------------------------------------------------*/
/**
 *  View3dResolveSpecialObjects is used to actually create all the
 *  was locked into memory must be released when done with the map.
 *  special effects created by the special objects.  The special objects
 *  list is run through 3 times.  1) to place primary effects (doors,
 *  sector definitions, etc.), 2) to place modifiers/secondary effects
 *  (door goes higher, door has different sound), and 3) to delete the list.
 *
 *<!-----------------------------------------------------------------------*/
#define SPECIAL_OBJECT_DOOR                      1500
#define SPECIAL_OBJECT_LOCK_DOOR                 1501
#define SPECIAL_OBJECT_GENERATOR                 1502
#define SPECIAL_OBJECT_DIRT_SECTOR               1503
#define SPECIAL_OBJECT_ICE_SECTOR                1504
#define SPECIAL_OBJECT_WATER_SECTOR              1505
#define SPECIAL_OBJECT_LAVA_SECTOR               1506
#define SPECIAL_OBJECT_MUD_SECTOR                1507
#define SPECIAL_OBJECT_AREA_SOUND_RUMBLE         1508
#define SPECIAL_OBJECT_AREA_SOUND_FIREPLACE      1509
#define SPECIAL_OBJECT_AREA_SOUND_LAVA           1510
#define SPECIAL_OBJECT_AREA_SOUND_WIND_SOFT      1511
#define SPECIAL_OBJECT_AREA_SOUND_WIND_HARD      1512
#define SPECIAL_OBJECT_AREA_SOUND_STREAM         1513
#define SPECIAL_OBJECT_AREA_SOUND_WATER_FALL     1514
#define SPECIAL_OBJECT_AREA_SOUND_WATER_FOUNTAIN 1515
#define SPECIAL_OBJECT_AREA_SOUND_TELEPORTER     1516
#define SPECIAL_OBJECT_AREA_SOUND_ELECTRICITY    1517
#define SPECIAL_OBJECT_LEVEL_LINK                1518
#define SPECIAL_OBJECT_LEVEL_LINK_PLACE          1519
#define SPECIAL_OBJECT_GLASS_CEILING             1520
#define SPECIAL_OBJECT_DEPTH                     1521
#define SPECIAL_OBJECT_SECTOR_ANIMATION          1522
#define SPECIAL_OBJECT_FLOW_SLOW                 1523
#define SPECIAL_OBJECT_FLOW_MEDIUM               1524
#define SPECIAL_OBJECT_FLOW_FAST                 1525
#define SPECIAL_OBJECT_GLOW                      1526
#define SPECIAL_OBJECT_DOOR_REQUIRE_ITEM         1527
#define SPECIAL_OBJECT_AREA_SOUND_MACHINE_BUZZ   1528
#define SPECIAL_OBJECT_AREA_SOUND_TELEPORTER2    1529
#define SPECIAL_OBJECT_AREA_SOUND_LAVA_BOIL      1530
#define SPECIAL_OBJECT_AREA_SOUND_WARM_HUM       1531
#define SPECIAL_OBJECT_AREA_SOUND_HUM            1532
#define SPECIAL_OBJECT_AREA_SOUND_CRICKETS       1533
#define SPECIAL_OBJECT_AREA_SOUND_TUBE_HUM       1534
#define SPECIAL_OBJECT_AREA_SOUND_SHORELINE      1535
#define SPECIAL_OBJECT_AREA_SOUND_WATER_DRIP     1536

#define SPECIAL_OBJECT_FLOW_WALL_UP1             1537
#define SPECIAL_OBJECT_FLOW_WALL_UP2             1538
#define SPECIAL_OBJECT_FLOW_WALL_UP3             1539
#define SPECIAL_OBJECT_FLOW_WALL_DOWN1           1540
#define SPECIAL_OBJECT_FLOW_WALL_DOWN2           1541
#define SPECIAL_OBJECT_FLOW_WALL_DOWN3           1542

#define SPECIAL_OBJECT_ACID_SECTOR               1543

#define SOUND_MACHINE_BUZZ   3000
#define SOUND_ELECTRICITY    3001
#define SOUND_TELEPORTER2    3002
#define SOUND_STREAM         3003
#define SOUND_WIND_SOFT      3004
#define SOUND_WIND_HARD      3004
#define SOUND_TELEPORTER     3006
#define SOUND_WATER_FALL     3007
#define SOUND_FIREPLACE      3008
#define SOUND_LAVA           3009
#define SOUND_RUMBLE         3010
#define SOUND_LAVA_BOIL      3011
#define SOUND_WARM_HUM       3012
#define SOUND_HUM            3013
#define SOUND_CRICKETS       3014
#define SOUND_TUBE_HUM       3015
#define SOUND_SHORELINE      3016
#define SOUND_WATER_DRIP     3018
#define SOUND_WATER_FOUNTAIN 3019

/* Put the given animation on the side closest to the given x, y location */
T_void IAssignWallAt(T_sword16 x, T_sword16 y, T_word16 animNum)
{
    T_word16 side ;

    DebugRoutine("IAssignWallAt") ;

    side = View3dFindSide(x, y) ;
    if (side != 0xFFFF)
        MapSetSideState(side, animNum) ;

    DebugEnd() ;
}

T_void View3dResolveSpecialObjects(T_void)
{
    T_areaSound areaSound ;
    T_doubleLinkListElement element ;
    T_3dObjectInFile *p_object ;
    T_word16 sector ;
    T_sword16 x, y ;

    DebugRoutine("View3dResolveSpecialObjects") ;

#if 1
    /* Loop through the special objects list to get primary effects. */
    element = DoubleLinkListGetFirst(G_specialObjectList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_object = (T_3dObjectInFile *)DoubleLinkListElementGetData(element) ;
        x = p_object->x ;
        y = p_object->y ;
        switch(p_object->objectType)  {
            case SPECIAL_OBJECT_DOOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)  {
                    DoorCreate(
                        sector,
                        MapGetFloorHeight(sector),
                        View3dCalculateLowestCeiling(sector)-16,
                        105,
                        140,
                        0,
                        x,
                        y) ;
                    MapSetCeilingHeight(sector, MapGetFloorHeight(sector)) ;
                }
                break ;
            case SPECIAL_OBJECT_GENERATOR:
                GeneratorAddGenerator(
                    p_object->attributes,
                    x,
                    y,
                    p_object->angle << 13,
                    0,
                    1,
                    15,
                    15,
                    TRUE,
                    -1,
                    0) ;
                break ;
            case SPECIAL_OBJECT_DIRT_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_DIRT) ;
                break ;
            case SPECIAL_OBJECT_ICE_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_ICE) ;
                break ;
            case SPECIAL_OBJECT_WATER_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_WATER) ;
                break ;
            case SPECIAL_OBJECT_LAVA_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_LAVA) ;
                break ;
            case SPECIAL_OBJECT_MUD_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_MUD) ;
                break ;
            case SPECIAL_OBJECT_ACID_SECTOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    View3dSetSectorType(
                        sector,
                        SECTOR_TYPE_ACID) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_RUMBLE:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_RUMBLE) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_FIREPLACE:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_FIREPLACE) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_LAVA:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_LAVA) ;

                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WIND_SOFT:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    128,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WIND_SOFT) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WIND_HARD:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WIND_HARD) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_STREAM:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_STREAM) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WATER_FALL:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WATER_FALL) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WATER_FOUNTAIN:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WATER_FOUNTAIN) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_TELEPORTER:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_TELEPORTER) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_ELECTRICITY:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_ELECTRICITY) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_LEVEL_LINK:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    G_3dSectorInfoArray[sector].nextMap =
                        p_object->attributes ;
                break ;
            case SPECIAL_OBJECT_LEVEL_LINK_PLACE:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    G_3dSectorInfoArray[sector].nextMapStart =
                        p_object->attributes ;
                break ;
            case SPECIAL_OBJECT_GLASS_CEILING:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    G_3dSectorInfoArray[sector].ceilingLimit =
                        MapGetFloorHeight(sector) + p_object->attributes ;
                break ;
            case SPECIAL_OBJECT_DEPTH:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)  {
                    G_3dSectorInfoArray[sector].depth = (T_byte8)p_object->attributes ;

                    /* Make all creatures and things on top of the */
                    /* sector settle to their new correct height. */
                    MapSetFloorHeight(sector, MapGetFloorHeight(sector)) ;
                }
                break ;
            case SPECIAL_OBJECT_SECTOR_ANIMATION:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    MapSetSectorState(sector, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_FLOW_SLOW:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    MapSetSectorState(sector, p_object->angle) ;
                break ;
            case SPECIAL_OBJECT_FLOW_MEDIUM:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    MapSetSectorState(sector, 8+p_object->angle) ;
                break ;
            case SPECIAL_OBJECT_FLOW_FAST:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)
                    MapSetSectorState(sector, 16+p_object->angle) ;
                break ;
            case SPECIAL_OBJECT_GLOW:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)  {
                    G_3dSectorInfoArray[sector].lightAnimationType = 1 ;
                    G_3dSectorInfoArray[sector].lightAnimationRate = 70 ;
                    G_3dSectorInfoArray[sector].lightAnimationCenter = 190 ;
                    G_3dSectorInfoArray[sector].lightAnimationRadius = 60 ;
                }
                break ;
            case SPECIAL_OBJECT_DOOR_REQUIRE_ITEM:
                /* Make the door at the given x y location */
                /* require a certain item. */
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)  {
                    if (DoorIsAtSector(sector))  {
                        DoorSetRequiredItem(
                            sector,
                            p_object->attributes) ;
                        DoorLock(sector) ;
                    }
                }
                break ;

            case SPECIAL_OBJECT_AREA_SOUND_MACHINE_BUZZ:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_MACHINE_BUZZ) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_TELEPORTER2:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_TELEPORTER2) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_LAVA_BOIL:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_LAVA_BOIL) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WARM_HUM:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WARM_HUM) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_HUM:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_HUM) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_CRICKETS:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_CRICKETS) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_TUBE_HUM:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_TUBE_HUM) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_SHORELINE:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_SHORELINE) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_AREA_SOUND_WATER_DRIP:
                areaSound = AreaSoundCreate(
                    x,
                    y,
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AreaSoundFindGroupLeader(p_object->attributes),
                    NULL,
                    0,
                    SOUND_WATER_DRIP) ;
                AreaSoundSetGroupId(areaSound, p_object->attributes) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_UP1:
                IAssignWallAt(x, y, 0) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_UP2:
                IAssignWallAt(x, y, 1) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_UP3:
                IAssignWallAt(x, y, 2) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_DOWN1:
                IAssignWallAt(x, y, 3) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_DOWN2:
                IAssignWallAt(x, y, 4) ;
                break ;
            case SPECIAL_OBJECT_FLOW_WALL_DOWN3:
                IAssignWallAt(x, y, 5) ;
                break ;
        }
        element = DoubleLinkListElementGetNext(element) ;
    }

    /* Loop through the special objects list to get secondary effects. */
    element = DoubleLinkListGetFirst(G_specialObjectList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_object = (T_3dObjectInFile *)DoubleLinkListElementGetData(element) ;
        element = DoubleLinkListElementGetNext(element) ;
        x = p_object->x ;
        y = p_object->y ;
        switch(p_object->objectType)  {
            case SPECIAL_OBJECT_LOCK_DOOR:
                sector = View3dFindSectorNum(x, y) ;
                DebugCheck(sector != 0xFFFF) ;
                if (sector != 0xFFFF)  {
                    if (DoorIsAtSector(sector))  {
                        if (p_object->attributes == 255)  {
                            DoorLock(sector) ;
                        } else {
                            DoorIncreaseLock(sector, p_object->attributes) ;
                        }
                    }
                }
                break ;
        }
    }

    /* Loop through the special objects list to delete nodes. */
    element = DoubleLinkListGetFirst(G_specialObjectList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_object = (T_3dObjectInFile *)DoubleLinkListElementGetData(element) ;
        MemFree(p_object) ;
        DoubleLinkListRemoveElement(element) ;
        element = DoubleLinkListGetFirst(G_specialObjectList) ;
    }

    /* Free the special objects list. */
    DoubleLinkListDestroy(G_specialObjectList) ;
    G_specialObjectList = DOUBLE_LINK_LIST_BAD ;
#endif

    DebugEnd() ;
}

T_sword16 View3dCalculateLowestCeiling(T_word16 sector)
{
    T_word16 i ;
    T_3dLine *p_line ;
    T_3dSide *p_side1 ;
    T_3dSide *p_side2 ;
    T_sword16 lowest = 0x7FFE ;
    T_sword16 height ;

    p_line = G_3dLineArray ;
    for (i=0; i<G_Num3dLines; i++, p_line++)  {
        if (p_line->flags & LINE_IS_TWO_SIDED)  {
            p_side1 = G_3dSideArray + p_line->side[0] ;
            p_side2 = G_3dSideArray + p_line->side[1] ;
            if (p_side1->sector == sector)  {
                height = G_3dSectorArray[p_side2->sector].ceilingHt ;
                if (height < lowest)
                    lowest = height ;
            }
            if (p_side2->sector == sector)  {
                height = G_3dSectorArray[p_side1->sector].ceilingHt ;
                if (height < lowest)
                    lowest = height ;
            }
        }
    }

    return lowest ;
}

typedef struct {
    T_3dSegment *p_segArray ;
    T_3dSide *p_sideArray ;
    T_3dLine *p_lineArray ;
    T_3dObject *p_firstObject ;
    T_3dObject *p_lastObject ;
    T_3dNode *p_node ;
    T_3dNode **p_pnode ;
    T_3dSector *p_sectorArray ;
    T_3dVertex *p_vertexArray ;
    T_3dSegmentSector *p_segmentSectorArray ;
    T_3dBlockMap *p_blockMapArray ;
    T_3dBlockMapHeader *p_blockMapHeader ;
    T_sword16 rootBSPNode ;
    T_resource *p_upperResourceArray ;
    T_resource *p_lowerResourceArray ;
    T_resource *p_mainResourceArray ;
    T_resource *p_floorResourceArray ;
    T_resource *p_ceilingResourceArray ;
    T_word16 num3dObjects ;
    T_word16 num3dSegs ;
    T_word16 num3dSides ;
    T_word16 num3dLines ;
    T_word16 num3dNodes ;
    T_word16 num3dSectors ;
    T_word16 num3dSSectors ;
    T_word16 num3dVertexes ;
    T_word16 blockmapSize ;
    T_3dSectorInfo *p_sectorInfoArray ;
} T_mapGroupStruct ;

/* LES 06/03/96 Created */
T_mapGroup View3dCreateMapGroup(T_void)
{
    T_mapGroupStruct *p_mapGroup = MAP_GROUP_BAD ;

    DebugRoutine("View3dCreateMapGroup");

    p_mapGroup = MemAlloc(sizeof(T_mapGroupStruct)) ;
    DebugCheck(p_mapGroup != NULL) ;
    if (p_mapGroup)  {
        memset(p_mapGroup, 0, sizeof(T_mapGroupStruct)) ;
    }

    DebugEnd() ;

    return (T_mapGroup *)p_mapGroup ;
}

/* LES 06/03/96 Created */
T_void View3dDestroyMapGroup(T_mapGroup p_mapGroup)
{
    DebugRoutine("View3dDestroyMapGroup") ;
    DebugCheck(p_mapGroup != MAP_GROUP_BAD) ;

    MemFree(p_mapGroup) ;

    DebugEnd() ;
}

/* LES 06/03/96 Created */
T_void View3dSetMapGroup(T_mapGroup p_mapGroup)
{
    T_mapGroupStruct *p_map ;

    DebugRoutine("View3dSetMapGroup") ;
    DebugCheck(p_mapGroup != MAP_GROUP_BAD) ;

    p_map = (T_mapGroupStruct *)p_mapGroup ;

    G_3dSegArray = p_map->p_segArray ;
    G_3dSideArray = p_map->p_sideArray ;
    G_3dLineArray = p_map->p_lineArray ;
    G_First3dObject = p_map->p_firstObject ;
    G_Last3dObject = p_map->p_lastObject ;
    G_3dNodeArray = p_map->p_node ;
    G_3dPNodeArray = p_map->p_pnode ;
    G_3dSectorArray = p_map->p_sectorArray ;
    G_3dVertexArray = p_map->p_vertexArray ;
    G_3dSegmentSectorArray = p_map->p_segmentSectorArray ;
    G_3dBlockMapArray = p_map->p_blockMapArray ;
    G_3dBlockMapHeader = p_map->p_blockMapHeader ;
    G_3dRootBSPNode = p_map->rootBSPNode ;
    G_3dUpperResourceArray = p_map->p_upperResourceArray ;
    G_3dLowerResourceArray = p_map->p_lowerResourceArray ;
    G_3dMainResourceArray = p_map->p_mainResourceArray ;
    G_3dLowerResourceArray = p_map->p_floorResourceArray ;
    G_3dCeilingResourceArray = p_map->p_ceilingResourceArray ;
    G_Num3dObjects = p_map->num3dObjects ;
    G_Num3dSegs = p_map->num3dSegs ;
    G_Num3dLines = p_map->num3dLines ;
    G_Num3dObjects = p_map->num3dObjects ;
    G_Num3dNodes = p_map->num3dNodes ;
    G_Num3dSectors = p_map->num3dSectors ;
    G_Num3dSSectors = p_map->num3dSSectors ;
    G_Num3dVertexes = p_map->num3dVertexes ;
    G_BlockmapSize = p_map->blockmapSize ;
    G_3dSectorInfoArray = p_map->p_sectorInfoArray ;

    DebugEnd() ;
}

/* LES 06/03/96 Created */
T_void View3dGetMapGroup(T_mapGroup p_mapGroup)
{
    T_mapGroupStruct *p_map ;

    DebugRoutine("View3dGetMapGroup") ;
    DebugCheck(p_mapGroup != MAP_GROUP_BAD) ;

    p_map = (T_mapGroupStruct *)p_mapGroup ;

    p_map->p_segArray = G_3dSegArray ;
    p_map->p_sideArray = G_3dSideArray ;
    p_map->p_lineArray = G_3dLineArray ;
    p_map->p_firstObject = G_First3dObject ;
    p_map->p_lastObject = G_Last3dObject ;
    p_map->p_node = G_3dNodeArray ;
    p_map->p_pnode = G_3dPNodeArray ;
    p_map->p_sectorArray = G_3dSectorArray ;
    p_map->p_vertexArray = G_3dVertexArray ;
    p_map->p_segmentSectorArray = G_3dSegmentSectorArray ;
    p_map->p_blockMapArray = G_3dBlockMapArray ;
    p_map->p_blockMapHeader = G_3dBlockMapHeader ;
    p_map->rootBSPNode = G_3dRootBSPNode ;
    p_map->p_upperResourceArray = G_3dUpperResourceArray ;
    p_map->p_lowerResourceArray = G_3dLowerResourceArray ;
    p_map->p_mainResourceArray = G_3dMainResourceArray ;
    p_map->p_floorResourceArray = G_3dLowerResourceArray ;
    p_map->p_ceilingResourceArray = G_3dCeilingResourceArray ;
    p_map->num3dObjects = G_Num3dObjects ;
    p_map->num3dSegs = G_Num3dSegs ;
    p_map->num3dLines = G_Num3dLines ;
    p_map->num3dObjects = G_Num3dObjects ;
    p_map->num3dNodes = G_Num3dNodes ;
    p_map->num3dSectors = G_Num3dSectors ;
    p_map->num3dSSectors = G_Num3dSSectors ;
    p_map->num3dVertexes = G_Num3dVertexes ;
    p_map->blockmapSize = G_BlockmapSize ;
    p_map->p_sectorInfoArray = G_3dSectorInfoArray ;

    DebugEnd() ;
}

static T_void IOutputReject(T_void)
{
    FILE *fp ;
    T_word16 i, j ;
    T_word16 width ;
    T_word32 index ;
//    static powers[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 } ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;

    fp = fopen("reject.dat", "w") ;
    width = (G_Num3dSectors+7)>>3 ;

    fprintf(fp, "Num sectors = %d\n", G_Num3dSectors) ;
    fprintf(fp, "width = %d\n", width) ;
    for (i=0, index=0; i<G_Num3dSectors; i++)  {
        for (j=0; j<G_Num3dSectors; j++, index++)  {
            fprintf(fp, "%d", ((G_3dReject[index>>3] & powers[index & 7]) == 0) ? 0 : 1) ;
        }
        fprintf(fp, "\n") ; ;
    }

    fclose(fp) ;
}

static T_void IObjCollisionListsInit(T_void)
{
    T_word16 size ;
    T_word16 i ;

    DebugRoutine("IObjCollisionListsInit") ;

    G_objCollisionNumX = G_3dBlockMapHeader->columns * 2 ;
    G_objCollisionNumY = G_3dBlockMapHeader->rows * 2 ;
    G_lastCollisionList = G_objCollisionNumX * G_objCollisionNumY ;

    /* Allocate the memory just for the list handles. */
    size = 1+G_lastCollisionList ;
    G_3dObjCollisionLists = MemAlloc(size * sizeof(T_doubleLinkList)) ;

    /* Initialize all the lists */
    for (i=0; i<size; i++)  {
        G_3dObjCollisionLists[i] = DoubleLinkListCreate() ;
        DebugCheck(G_3dObjCollisionLists[i] != DOUBLE_LINK_LIST_BAD) ;
    }

    DebugEnd() ;
}

static T_void IObjCollisionListsFinish(T_void)
{
    T_word16 size ;
    T_word16 i ;
    T_doubleLinkListElement element ;

    DebugRoutine("IObjCollisionListsFinish") ;

    size = 1+G_lastCollisionList ;

    /* Get rid of all the lists */
    for (i=0; i<size; i++)  {
        DebugCheck(G_3dObjCollisionLists[i] != DOUBLE_LINK_LIST_BAD) ;
        /* Remove all the elements in the list. */
        while ((element = DoubleLinkListGetFirst(G_3dObjCollisionLists[i])) !=
                DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            DoubleLinkListRemoveElement(element) ;
        }

        /* Destroy the list itself. */
        DoubleLinkListDestroy(G_3dObjCollisionLists[i]) ;
        G_3dObjCollisionLists[i] = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    }

    /* Free the whole handle list from memory. */
    MemFree(G_3dObjCollisionLists) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  3D_IO.C
 *-------------------------------------------------------------------------*/
