/****************************************************************************/
/*    FILE:  MAP.C                                                          */
/****************************************************************************/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "ACTIVITY.H"
#include "AREASND.H"
#include "CRELOGIC.H"
#include "CSYNCPCK.H"
#include "DOOR.H"
#include "FILE.H"
#include "LIGHT.H"
#include "MAP.H"
#include "MAPANIM.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "PICS.H"
#include "PROMPT.H"
#include "SCHEDULE.H"
#include "SLIDER.H"
#include "SOUND.H"
#include "SYNCMEM.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "UPDATE.H"

static E_Boolean G_mapLoaded = FALSE ;
static T_byte8 G_outsideLighting = 255 ;
static T_lightTable G_mapLightTable = LIGHT_TABLE_BAD ;
static T_mapAnimation G_mapAnimation = MAP_ANIMATION_BAD ;
static T_byte8 *G_realBackdrop = NULL ;
static T_word32 G_lastUpdateMapAnimate = 0 ;
static T_activitiesHandle G_activitiesHandle = ACTIVITY_HANDLE_BAD ;

static T_sword16 G_mapSetFloorHighest ;

static T_byte8 G_noName[] = "--------" ;

/* HARD CODED JUNK TO BE REMOVED! */
//static T_resource R_waterTexture[5] ;
//static T_byte8 *P_waterTexture[5] ;

/* Internal prototypes: */
static E_Boolean IMoveUpObjects(
                     T_3dObject *p_obj,
                     T_word32 heightAndSector) ;
static E_Boolean IMapSetFloorCheckObject(
                     T_3dObject *p_obj,
                     T_word32 heightAndSector) ;
static T_word32 G_dayTickOffset = 0 ;

static T_word32 G_mapSpecial = MAP_SPECIAL_NONE ;

static T_void IMapPushUpObjects(
                  T_3dObject *p_movingObject,
                  T_sword16 newHeight) ;

/****************************************************************************/
/*  Routine:  MapInitialize                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapInitialize is called to start up any data or code needed by the    */
/*  Map Module.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MapSetBackdrop                                                        */
/*    PictureUnlock                                                         */
/*    PictureUnfind                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapInitialize(T_void)
{
//    T_word16 i ;

    DebugRoutine("MapInitialize") ;

    /* Code to initialize map module. */

/* !!! Hard coded animation !!! */
//    for (i=0; i<5; i++)  {
//        sprintf(filename, "WATER_0%d", i+1) ;
//        P_waterTexture[i] = PictureLock(filename, &R_waterTexture[i]) ;
//    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapFinish                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapFinish is called to finish    up any data or code needed by the    */
/*  Map Module.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureUnlock                                                         */
/*    PictureUnfind                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapFinish(T_void)
{
    DebugRoutine("MapFinish") ;

    /* Code to initialize map module. */

/* !!! Hard coded animation!!! */
/*
    for (i=0; i<5; i++)  {
        PictureUnlock(R_waterTexture[i]) ;
        PictureUnfind(R_waterTexture[i]) ;
    }
*/

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapLoad                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapLoad     reads in the given file and uses it as the map for        */
/*  the current game.  This only controls the map and the objects within.   */
/*  Player position or other related information is not defined.            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 mapNumber          -- Number of the map to load              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    sprintf                                                               */
/*    View3dLoadMap                                                         */
/*    ActivitiesLoad                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  04/20/95  Added Door loading.                                    */
/*    LES  04/26/95  Added area sound loading                               */
/*    LES  06/21/95  Changed from ViewLoadMap to MapLoad                    */
/*    LES  04/15/96  Added call to CreaturesInitialize                      */
/*                                                                          */
/****************************************************************************/

T_word32 FreeMemory(T_void) ;
T_void MapLoad(T_word32 mapNumber)
{
    T_byte8 filename[20] ;
//    T_word16 sector ;
    T_byte8 *p_backdrop ;
    T_resource res ;
    T_3dObject *p_obj ;
    T_byte8 G_songName[20] ;
    T_byte8 infoFileName[20] ;
    T_byte8 *p_infoFile ;
    T_resource r_infoFile ;
    T_byte8 backgroundName[20] ;
    T_byte8 backgroundFullName[30] ;
    extern E_Boolean G_serverActive ;
    T_word32 dummyvar ;

    DebugRoutine("MapLoad") ;

    SoundStopBackgroundMusic() ;

    PromptStatusBarInit ("Please wait .. loading level",100);

    /* Reset the sync memory. */
    SyncMemClear() ;

//printf("Loading level %d\n", mapNumber) ;  fflush(stdout) ;

    DebugCheck(mapNumber <= (T_word32)9999999) ;
    DebugCheck(G_mapLoaded == FALSE) ;

//puts("MapLoad: ClientSyncInit") ;  fflush(stdout) ;
    ClientSyncInit() ;

//puts("MapLoad: Check memory") ; fflush(stdout) ;
#if 0
p_memTest = MemAlloc(250000) ;
if (p_memTest == NULL)  {
    printf("Not enough memory to load map!") ;
    printf("Free memory: %ld\n", FreeMemory()) ;
    fflush(stdout) ;
//puts("MapLoad: DumpDiscarded") ; fflush(stdout) ;
//    MemDumpDiscarded() ;
    fflush(stdout) ;
    fprintf(stderr, "Not enough memory to load map!") ;
    fflush(stderr) ;
    exit(1) ;
}
MemFree(p_memTest) ;
#endif


//    puts("MapLoad: Loading backdrop and music") ;
    sprintf(infoFileName, "L%ld.I", mapNumber) ;
    p_infoFile = PictureLockData(infoFileName, &r_infoFile) ;
    strcpy(backgroundName, "CLOUDS2.PIC") ;
    if (p_infoFile)  {
        G_mapSpecial = MAP_SPECIAL_NONE ;
        sscanf(p_infoFile, "%s%s%d", G_songName, backgroundName, &G_mapSpecial) ;
        DebugCheck(G_mapSpecial < MAP_SPECIAL_UNKNOWN) ;
//        sprintf(G_songName, "L%ld.HMI", mapNumber) ;
        /** Change the tune. **/
//        sprintf(G_songName, "MUSIC/l%ld.hmi", mapNumber) ;
//        SoundSetBackgroundMusic(G_songName) ;

        PictureUnlockAndUnfind(r_infoFile) ;
    }

    TickerPause() ;
    PromptStatusBarUpdate (5);
    sprintf(filename, "l%u.map", mapNumber) ;

//puts("MapLoad: Door Init") ; fflush(stdout) ;
    DoorInitialize() ;
    PromptStatusBarUpdate (15);

//    puts("MapLoad: AreaSoundInit") ; fflush(stdout) ;
    AreaSoundInitialize() ;
    PromptStatusBarUpdate (25);

    SliderInitialize() ;

    /* Start up the creatures list -- it will be filled as */
    /* objects are created in the map (loaded). */
//puts("MapLoad: CreaturesInit") ; fflush(stdout) ;
    CreaturesInitialize() ;

    ObjectsResetIds() ;
    View3dLoadMap(filename) ;
    PromptStatusBarUpdate (60);
    CreaturesCheck() ;

//puts("MapLoad: DoorLoad") ; fflush(stdout) ;
    DoorLoad(mapNumber) ;
    PromptStatusBarUpdate (65);

    AreaSoundLoad(mapNumber) ;
    G_activitiesHandle = ActivitiesLoad(mapNumber) ;
    PromptStatusBarUpdate (75);

//    puts("MapLoad: Backdrop") ; fflush(stdout) ;
    sprintf(backgroundFullName, "BACKDROP/%s", backgroundName) ;
    p_backdrop = PictureLock(backgroundFullName, &res) ;
    MapSetBackdrop(p_backdrop) ;
    PictureUnlock(res) ;
    PictureUnfind(res) ;
    PromptStatusBarUpdate (80);

    /* Load in the creatures. */
//    CreaturesLoad(mapNumber) ;
    PromptStatusBarUpdate (90);

    ObjectsRemoveExtra() ;
    PromptStatusBarUpdate (99);

    /* Note that we now have a map in memory. */
    G_mapLoaded = TRUE ;

    PromptStatusBarClose();
    TickerContinue() ;
    ObjectGeneratorLoad(mapNumber) ;

    /* Load up the animations for this level. */
    G_mapAnimation = MapAnimateLoad(mapNumber) ;

    View3dResolveSpecialObjects() ;
    CreaturesCheck() ;

    /* Get our lighting table. */
    G_mapLightTable = LightTableLoad(mapNumber) ;
    LightTableRecalculate(G_mapLightTable, G_outsideLighting) ;

    UpdateMapBegin() ;

    G_serverActive = TRUE ;

#   ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
    MapShiftTextureOpenFile() ;
#   endif

    if (ClientSyncGetNumberPlayers() != 1)  {
        /* Cache in all types of piecewise objects. */
        /* But only do it if we are more than 1 person. */
        p_obj = ObjectCreateFake() ;
        ObjectSetType(p_obj, 510) ;
        ObjectDestroy(p_obj) ;
        p_obj = ObjectCreateFake() ;
        ObjectSetType(p_obj, 520) ;
        ObjectDestroy(p_obj) ;
        p_obj = ObjectCreateFake() ;
        ObjectSetType(p_obj, 530) ;
        ObjectDestroy(p_obj) ;
    }

    ActivitiesRun(0) ;
    CreaturesCheck() ;

    sprintf(infoFileName, "L%ld.I", mapNumber) ;
    p_infoFile = PictureLockData(infoFileName, &r_infoFile) ;
    if (p_infoFile)  {
        sscanf(p_infoFile, "%s%s%d", G_songName, backgroundName, &dummyvar) ;
        SoundSetBackgroundMusic(G_songName) ;

        PictureUnlockAndUnfind(r_infoFile) ;
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSave                                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSave     writes the given file and all the information out to      */
/*  the current map number.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Current 3d engine doesn't allow easy saving of the map.  (Not that    */
/*  it's impossible or anything -- just not yet).                           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 mapNumber          -- Number of the map to save              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/28/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/21/95  Move to the Map Module (ViewSaveMap -> MapSave)        */
/*                                                                          */
/****************************************************************************/

T_void MapSave(T_word32 mapNumber)
{
    DebugRoutine("MapSave") ;
    DebugCheck(mapNumber <= (T_word32)9999999) ;
    DebugCheck(G_mapLoaded == TRUE) ;

    /// Does nothing yet. !!!!

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: MapUnload                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    When the current map is no longer needed, call this routine to        */
/*  remove it.  All information and pictures concerning that map is         */
/*  removed from memory.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dUnloadMap                                                       */
/*    ActivitiesUnload                                                      */
/*    MapSetBackdrop                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/21/95  Changed from ViewUnloadMap to MapUnload                */
/*    LES  04/15/96  Added call to CreaturesFinish                          */
/*                                                                          */
/****************************************************************************/

T_void MapUnload(T_void)
{
    extern E_Boolean G_serverActive ;

    DebugRoutine("MapUnload") ;

    /* Only unload the map if there is a map to unload. */
    if (G_mapLoaded == TRUE)  {
        G_serverActive = FALSE ;
        /* Get rid of all the animations for this level. */
        MapAnimateUnload(G_mapAnimation) ;
        G_mapAnimation = MAP_ANIMATION_BAD ;

        /* Get rid of that previous light table. */
        LightTableUnload(G_mapLightTable) ;

        ObjectGeneratorUnload() ;


        ScheduleClearEvents() ;

        AreaSoundFinish() ;
        SoundStopAllSounds() ;

        DoorFinish() ;

        SliderFinish() ;

        ActivitiesUnload() ;

        ObjectsUnload() ;

        MapSetBackdrop(NULL) ;

        /* Unload everything that the map is using. */
        View3dUnloadMap() ;

        /* This call is made to destroy the creature list. */
        /* Since all objects are destroyed already, this */
        /* should be a minor clean-up. */
        CreaturesFinish() ;

        /* Note that a map is no longer in memory. */
        G_mapLoaded = FALSE ;

// LED 2/28/2013 -- Had a crash here when UpdateMapEnd called EfxFinish, it tried to 
// call EfxDestroy which called ObjectMarkForDestroy -- but the object was already destroyed!
// by ObjectsUnload?  What efx was in use?  Looks like I got hurt as I was exiting the level
// at the same time!  (EfxUpdateBloodSplat on updateCallback of p_efx (T_efxStruct *))
        UpdateMapEnd() ;

        ClientSyncFinish() ;
    }

    /* Get rid of everything discardable to clean up the memory. */
    MemFlushDiscardable() ;

#   ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
    MapShiftTextureCloseFile() ;
#   endif

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  MapSetBackdrop                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetBackdrop requires a pointer to a 320x100 (or bigger) picture    */
/*  and copies it into the backdrop to be used by the computer.             */
/*  (Actually, you pass in a 320x200 picture where the top 100 lines        */
/*  is half of the image and the next 100 lines are the right half of the   */
/*  image).                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_backdrop         -- Pointer to backdrop                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/21/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/21/95  Changed from ViewSetBackdrop to MapSetBackdrop         */
/*                                                                          */
/****************************************************************************/

T_void MapSetBackdrop(T_byte8 *p_backdrop)
{
    T_word16 sizeX, sizeY ;
    extern T_byte8 *G_backdrop ;
    T_word32 x, y, offset ;
    T_byte8 *p_back ;

    DebugRoutine("MapSetBackdrop") ;

//puts("Preparing backdrop") ;
    /* Free up the backdrop if needed. */
    if (G_realBackdrop != NULL)  {
        MemFree(G_realBackdrop) ;

        /* Note that the backdrop is gone. */
        G_backdrop = NULL ;
        G_realBackdrop = NULL ;
    }

#if 0
    if (p_backdrop != NULL)  {
        PictureGetXYSize(p_backdrop, &sizeX, &sizeY) ;

        /* Allocate memory for the resized backdrop. */
        p_back = G_backdrop = MemAlloc(VIEW3D_WIDTH*VIEW3D_HEIGHT) ;
        DebugCheck(G_backdrop != NULL) ;

        /* Resize the background. */
        for (y=0; y<(VIEW3D_HEIGHT>>1); y++)  {
            for (x=0; x<(VIEW3D_WIDTH<<1); x++, p_back++)  {
                if (x < VIEW3D_WIDTH)  {
                    offset = ((y*sizeY)/VIEW3D_HEIGHT)*sizeX +
                              ((x*sizeX)/VIEW3D_WIDTH) ;
                } else {
                    offset = ((y*sizeY)/VIEW3D_HEIGHT)*sizeX +
                              ((x*sizeX)/VIEW3D_WIDTH) +
                              ((T_word32)sizeX) * ((T_word32)(sizeY>>1));
                }
                *p_back = p_backdrop[offset] ;
            }
        }
    }
#endif
    if (p_backdrop != NULL)  {
        PictureGetXYSize(p_backdrop, &sizeX, &sizeY) ;

        /* Allocate memory for the resized backdrop. */
        p_back = G_backdrop = MemAlloc(2*VIEW3D_WIDTH*VIEW3D_HEIGHT) ;
        DebugCheck(G_backdrop != NULL) ;

        /* Resize the background. */
        for (y=0; y<(T_word16)VIEW3D_HEIGHT; y++)  {
            for (x=0; x<(T_word16)(VIEW3D_WIDTH<<1); x++, p_back++)  {
                if (x < (T_word16)VIEW3D_WIDTH)  {
                    offset = ((y*sizeY)/VIEW3D_HEIGHT)*sizeX +
                              ((x*sizeX)/VIEW3D_WIDTH) ;

                } else {
                    offset = ((y*sizeY)/VIEW3D_HEIGHT)*sizeX +
                              (((((VIEW3D_WIDTH<<1)-1)-x)
                               *sizeX)/VIEW3D_WIDTH) ;

                }
                *p_back = p_backdrop[offset] ;
            }
        }
    }

    G_realBackdrop = G_backdrop ;
    G_backdrop += VIEW3D_HEIGHT * VIEW3D_WIDTH ;

//puts("Done with backdrop") ;

    DebugEnd() ;
}


/** The following routines are not needed for the server-only build. **/
#ifndef SERVER_ONLY

/****************************************************************************/
/*  Routine:  MapGetForwardWallActivation                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetForwardWallActivation is used to get the activation number      */
/*  of the wall in front of the current POV.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The current 3d engine does not allow for wall activation.             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Activation number                      */
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
/*    LES  01/04/95  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/21/95  Removed from View.c and put into map.c and renamed.    */
/*    LES  06/12/96  Modified to work with passed in p_obj                  */
/*                                                                          */
/****************************************************************************/

T_word16 MapGetForwardWallActivation(T_3dObject *p_obj)
{
    T_word16 action = 0 ;
    T_sword32 newX, newY ;
    T_sword16 hitLines[40] ;
    T_word16 closest ;
    T_word16 i ;
    T_word16 num ;
    T_word16 dist ;

    DebugRoutine("MapGetForwardWallActivation") ;

    newX = ObjectGetX(p_obj) + MathCosineLookup(ObjectGetAngle(p_obj))*64L ;
    newY = ObjectGetY(p_obj) + MathSineLookup(ObjectGetAngle(p_obj))*64L ;

    if ((num = View3dFindLineHits(
                   ObjectGetX(p_obj)>>16,
                   ObjectGetY(p_obj)>>16,
                   newX>>16,
                   newY>>16,
                   20,
                   hitLines)) != 0)  {
        i = 0 ;
        num <<= 1 ;

        dist = hitLines[1] ;
        closest = hitLines[0] ;
        for (i=2; i<num; i+=2)  {
            if (hitLines[i+1] < dist)  {
                dist = hitLines[i+1] ;
                closest = hitLines[i] ;
            }
        }

        action = G_3dLineArray[closest].special ;
//MessagePrintf("Getting action %d off of line %d", action, closest) ;
    } else {
    }

    DebugEnd() ;

    return action ;
}

/****************************************************************************/
/*  Routine:  MapSetWallBitmapTextureXY                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetWallBitmapTextureXY changes the x and y offset of a wall's      */
/*  texture.  Just pass in what wall, side, and the new x & y offset.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 wallNum            -- Number of wall to change.              */
/*                                                                          */
/*    T_word16 side               -- Side of the wall (0 or 1)              */
/*                                                                          */
/*    T_sword16 offX              -- Offset in X (left to right) direction. */
/*                                                                          */
/*    T_sword16 offY              -- Offset in Y (top to bottom) direction. */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  02/23/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_void MapSetWallBitmapTextureXY(
           T_word16 wallNum,
           T_word16 side,
           T_sword16 offX,
           T_sword16 offY)
{
    DebugRoutine("MapSetWallBitmapTextureXY") ;
    DebugCheck(wallNum < G_Num3dLines) ;

    G_3dSideArray[G_3dLineArray[wallNum].side[side]].tmXoffset = offX ;
    G_3dSideArray[G_3dLineArray[wallNum].side[side]].tmYoffset = offY ;

    DebugEnd() ;
}

#endif /** SERVER_ONLY **/

/****************************************************************************/
/*  Routine:  MapGetSectorAction                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetSectorAction returns the corresponding activity number for the  */
/*  given sector.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector to get activity of              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Activation number for sector           */
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
/*    LES  02/27/94  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_word16 MapGetSectorAction(T_word16 sector)
{
    T_word16 activity ;

    DebugRoutine("MapGetSectorAction") ;
    DebugCheck(sector < G_Num3dSectors) ;

    activity = G_3dSectorArray[sector].type ;

    DebugEnd() ;

    return activity ;
}

/****************************************************************************/
/*  Routine:  MapSetFloorHeight                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetFloorHeight makes the given sector's floor height go to the     */
/*  given height (instantly).                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector's floor to change.              */
/*                                                                          */
/*    T_sword16 height            -- New height for floor.                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  02/27/94  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_void MapSetFloorHeight(T_word16 sector, T_sword16 height)
{
    DebugRoutine("MapSetFloorHeight") ;
    DebugCheck(sector < G_Num3dSectors) ;

    G_mapSetFloorHighest = 0x7FFF ;
    ObjectsDoToAll(
        IMapSetFloorCheckObject,
        (MapGetCeilingHeight(sector) << 16) | sector) ;
    if (height > G_mapSetFloorHighest)
        height = G_mapSetFloorHighest ;

    ObjectsDoToAll(
        IMoveUpObjects,
        (height<<16) | sector) ;
    G_3dSectorArray[sector].floorHt = height ;

    DebugEnd() ;
}

static E_Boolean IMoveUpObjects(
                     T_3dObject *p_obj,
                     T_word32 heightAndSector)
{
    T_word16 num ;
    T_word16 i ;
    T_sword16 height ;
    T_word16 sector ;

    height = ((T_sword32) heightAndSector)>>16 ;
    sector = heightAndSector & 0xFFFF ;

    /* Don't do objects that stick to the ceiling. */
    if (!(ObjectGetMoveFlags(p_obj) & OBJMOVE_FLAG_STICK_TO_CEILING))  {
        /* Get the number of area sectors that this object is over. */
        num = ObjectGetNumAreaSectors(p_obj) ;

        /* Go through the list of sectors. */
        for (i=0; i<num; i++)  {
            /* Look for a match.  Break if there is one. */
            if (sector == ObjectGetNthAreaSector(p_obj, i))
                break ;
        }

        /* Did we find a match? */
        if (i != num)  {
            /* Mark the object as possibly needing to move. */
            ObjectForceUpdate(p_obj) ;

            /* If object is under lift, force us up. */
            if (ObjectGetZ16(p_obj) < height)  {
                ObjectSetZ16(p_obj, height) ;
    //            if ((((T_sword32)height)<<16) >
    //                   p_obj->objMove.lowestPoint>>16)
                p_obj->objMove.lowestPoint = (((T_sword32)height)<<16) ;
                IMapPushUpObjects(p_obj, height) ;
            }
        }
    }

    return FALSE ;
}

static E_Boolean IMapSetFloorCheckObject(
                     T_3dObject *p_obj,
                     T_word32 ceilingAndSector)
{
    T_word16 num ;
    T_word16 i ;
    T_word16 sector ;
    T_sword16 objHeight ;
    T_sword16 z ;
    T_sword16 ceiling ;

    DebugRoutine("IMapSetFloorCheckObject") ;

    /* Get the sector out of the data passed in. */
    sector = ceilingAndSector & 0xFFFF ;

    /* Get the number of area sectors that this object is over. */
    num = ObjectGetNumAreaSectors(p_obj) ;

    /* Go through the list of sectors. */
    for (i=0; i<num; i++)  {
        /* Look for a match.  Break if there is one. */
        if (sector == ObjectGetNthAreaSector(p_obj, i))
            break ;
    }

    /* Did we find a match? */
    if (i != num)  {
        /* Yep, found a match. */
        ceiling = ((T_sword32)ceilingAndSector)>>16 ;

        /* Check its height. */
        z = ObjectGetZ16(p_obj) ;
        objHeight = ObjectGetHeight(p_obj) ;
        if (z + objHeight > ceiling)
            if (z < G_mapSetFloorHighest)
                G_mapSetFloorHighest = z ;
    }

    DebugEnd() ;

    return FALSE ;
}

static T_void IMapPushUpObjects(
                  T_3dObject *p_movingObject,
                  T_sword16 newHeight)
{
    T_sword16 newTop ;
    T_doubleLinkListElement element ;
    T_sword16 hashX, hashY ;
    T_sword16 startHashX, startHashY ;
    T_word16 group ;
    T_word16 groupWidth ;
    T_sword16 objX, objY ;
    T_word16 halfwidth ;
    T_3dObject *p_obj ;
    T_sword16 objBottom, objTop ;
    T_word16 radius ;
    T_sword16 x, y ;

    DebugRoutine("IMapPushUpObjects") ;

    /* Only do this whole routine if we are NOT passable. */
    if (ObjectIsPassable(p_movingObject))  {
        DebugEnd() ;
        return ;
    }

    /* Compute the new moving object's top. */
    newTop = newHeight + ObjectGetHeight(p_movingObject) ;
    x = ObjectGetX16(p_movingObject) ;
    y = ObjectGetY16(p_movingObject) ;
    radius = ObjectGetRadius(p_movingObject) ;

    /* Ok, push all objects above this object up, excluding this one */
    groupWidth = 2+(ObjectGetRadius(p_movingObject) >> 5) ;
    startHashX = ((ObjectGetX16(p_movingObject) -
                  G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((ObjectGetY16(p_movingObject) -
                  G_3dBlockMapHeader->yOrigin) >> 6) ;
    for (hashY=-groupWidth; hashY<=groupWidth; hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=-groupWidth; hashX<=groupWidth; hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;

                /* Skip this object */
                if (p_obj == p_movingObject)
                    continue ;

                /* Determine "square" distance to be in */
                halfwidth = ObjectGetRadius(p_obj) + radius ;
                objX = ObjectGetX16(p_obj) ;
                objY = ObjectGetY16(p_obj) ;

                /* See if the two objects collide in the x & y plane */
                if ((x <= objX+halfwidth) &&
                    (x >= objX-halfwidth) &&
                    (y <= objY+halfwidth) &&
                    (y >= objY-halfwidth))  {

                    /* Find the top and bottom of this object */
                    objBottom = ObjectGetZ16(p_obj) ;
                    objTop = objBottom + ObjectGetHeight(p_obj) ;

                    /* Move this object up if its bottom is below our */
                    /* moving top. */
                    if ((objBottom < newTop) && (objTop > newHeight))  {
                        /* Move that object up! */
                        ObjectSetZ16(p_obj, newTop) ;
                        p_obj->objMove.lowestPoint = (((T_sword32)newTop)<<16) ;

                        /* Move all objects on top of this up. */
                        /* NOTE:  This is recursive, but it should not */
                        /* get stuck because all objects below this */
                        /* object have already been processed ... thus, */
                        /* the computations are less (unless you have a 0 */
                        /* height object!). */
                        IMapPushUpObjects(p_obj, newTop) ;
                    }
                }
            }
        }
    }


    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetFloorHeight                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetFloorHeight returns the height of the given sector.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector's floor to get.                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  03/07/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_sword16 MapGetFloorHeight(T_word16 sector)
{
    T_sword16 height ;

    DebugRoutine("MapGetFloorHeight") ;
#ifndef NDEBUG
if (sector >= G_Num3dSectors)  {
 printf("Bad sector %d\n", sector) ;
}
#endif
    DebugCheck(sector < G_Num3dSectors) ;

    if (sector < G_Num3dSectors)
        height = G_3dSectorArray[sector].floorHt ;
    else
        height = -10000 ;

    DebugEnd() ;

    return height ;
}

/****************************************************************************/
/*  Routine:  MapGetWalkingFloorHeight                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetWalkingFloorHeight returns height of the floor if a thing       */
/*  is walking on it.  For water areas, this can be lower than the actual   */
/*  floor height.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector's floor to get.                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  04/11/95  Created                                                */
/*    LES  04/12/95  Fixed if/else mistake                                  */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_sword16 MapGetWalkingFloorHeight(T_word16 sector)
{
    T_sword16 height ;

    DebugRoutine("MapGetWalkingFloorHeight") ;
#ifndef NDEBUG
if (sector >= G_Num3dSectors)  {
 printf("Bad sector %d\n", sector) ;
 printf("Caller: %s\n", DebugGetCallerName()) ;
}
#endif
    DebugCheck(sector < G_Num3dSectors) ;

    if (sector < G_Num3dSectors)  {
        height = G_3dSectorArray[sector].floorHt ;
//        if (G_3dSectorArray[sector].trigger & SECTOR_DIP_FLAG)
//            height -= VIEW_WATER_DIP_LEVEL ;
        if (View3dIsAllowDip())
            height -= G_3dSectorInfoArray[sector].depth ;
    } else  {
        height = 10000 ;
    }

    DebugEnd() ;

    return height ;
}

/****************************************************************************/
/*  Routine:  MapSetCeilingHeight                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetCeilingHeight makes the given sector's floor height go to   */
/*  the given height (instantly).                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector's ceiling to change.            */
/*                                                                          */
/*    T_sword16 height            -- New height for ceiling.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  02/27/94  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_void MapSetCeilingHeight(T_word16 sector, T_sword16 height)
{
    T_3dObject *p_obj ;
    T_word16 i ;

    DebugRoutine("MapSetCeilingHeight") ;
    DebugCheck(sector < G_Num3dSectors) ;

    G_3dSectorArray[sector].ceilingHt = height ;

    /* Move any attached ceiling objects. */
    p_obj = ObjectsGetFirst() ;
    while (p_obj)  {
        /* Move objects that are told to stick to the ceiling. */
        if (ObjectGetMoveFlags(p_obj) & OBJMOVE_FLAG_STICK_TO_CEILING)  {
            /* See if this ceiling object is connected */
            /* to this ceiling. */
            for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
                if (ObjectGetNthAreaSector(p_obj, i) == sector)  {
                    /* Move the object up to its correct location. */
                    ObjectSetZ16(p_obj, height - ObjectGetHeight(p_obj)) ;
                    break ;
                }
            }
        }
        p_obj = ObjectGetNext(p_obj) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetCeilingHeight                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetCeilingHeight gets the given sector's ceiling height.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector's ceiling to change.            */
/*                                                                          */
/*    T_sword16 height            -- New height for ceiling.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  04/16/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_sword16 MapGetCeilingHeight(T_word16 sector)
{
    T_sword16 height ;

    DebugRoutine("MapGetCeilingHeight") ;
#ifndef NDEBUG
    if (sector >= G_Num3dSectors)  {
        printf("Bad sector #%d\n", sector) ;
    }
#endif
    DebugCheck(sector < G_Num3dSectors) ;

    height = G_3dSectorArray[sector].ceilingHt ;

    DebugEnd() ;

    return height ;
}

/****************************************************************************/
/*  Routine:  MapSetSectorLighting                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetSectorLighting makes a sector have a different light level.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Number of sector                       */
/*                                                                          */
/*    T_byte8 lightLevel          -- new lighting level                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
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
/*    LES  04/05/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_void MapSetSectorLighting(T_word16 sector, T_byte8 lightLevel)
{
    DebugRoutine("MapSetSectorLighting") ;
    DebugCheck(sector < G_Num3dSectors) ;

    G_3dSectorArray[sector].light = lightLevel ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetSectorLighting                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetSectorLighting gets  a sector have a different light level.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Number of sector                       */
/*                                                                          */
/*    T_byte8 lightLevel          -- new lighting level                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
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
/*    LES  04/05/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*                                                                          */
/****************************************************************************/

T_byte8 MapGetSectorLighting(T_word16 sector)
{
    T_byte8 light ;

    DebugRoutine("MapGetSectorLighting") ;
    DebugCheck(sector < G_Num3dSectors) ;

    light = (T_byte8)G_3dSectorArray[sector].light ;

    DebugEnd() ;

    return light ;
}

/****************************************************************************/
/*  Routine:  MapCheckCrush                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapCheckCrush sees if lowering the ceiling of the given sector to     */
/*  the new height will cause a character to be crushed.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector to consider                     */
/*                                                                          */
/*    T_sword16 newHeight         -- New ceiling height to consider         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = object or player is going to    */
/*                                   be crushed, or FALSE for ok.           */
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
/*    LES  04/21/95  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*    LES  07/30/96  Changed to not crush objects that are passible         */
/*                                                                          */
/****************************************************************************/

E_Boolean MapCheckCrushByCeiling(T_word16 sector, T_sword16 newHeight)
{
    E_Boolean status = FALSE ;
    T_word16 j ;
    T_sword16 height ;
    T_word16 num ;
    T_3dObject *p_obj ;

    DebugRoutine("MapCheckCrushByCeiling") ;

    p_obj = G_First3dObject ;
    while ((p_obj != NULL) && (!status))  {
        /* Only check non-passable objects. */
        if (!ObjectIsPassable(p_obj))  {
            num = ObjectGetNumAreaSectors(p_obj) ;
            for (j=0; j<num; j++)  {
                if (ObjectGetNthAreaSector(p_obj, j) == sector)  {
                    /* Where is the top of this object? */
                    height = ObjectGetZ16(p_obj) + ObjectGetHeight(p_obj) ;

                    /* Is this new height equal or less? */
                    if (newHeight <= height)  {
                        /* Yes, we are being crushed. */
                        status = TRUE ;
                        break ;
                    }
                }
            }
        }

        p_obj = p_obj->nextObj ;
    }

    DebugEnd() ;

    return status ;
}


/****************************************************************************/
/*  Routine:  MapCheckCrushByFloor                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapCheckCrushByFloor if raising the floor of a given sector    to     */
/*  the new height will cause a character to be crushed.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector to consider                     */
/*                                                                          */
/*    T_sword16 newHeight         -- New floor   height to consider         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = object or player is going to    */
/*                                   be crushed, or FALSE for ok.           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean MapCheckCrushByFloor(T_word16 sector, T_sword16 newHeight)
{
    T_word16 j ;
    T_sword16 height ;
    T_word16 num ;
    T_3dObject *p_obj ;
    T_sword16 bottom ;
    T_sword16 ceiling ;
    E_Boolean foundObj ;
    T_sword16 lowest ;
    T_word16 nth ;
    E_Boolean status ;
    char buffer[20] ;

    DebugRoutine("MapCheckCrushByFloor") ;

    status = FALSE ; /* Assume not being crushed */
    p_obj = ObjectsGetFirst() ;
    while ((p_obj != NULL) && (!status))  {
        /* Only check non-passable objects. */
        if (!ObjectIsPassable(p_obj))  {
            foundObj = FALSE ;
            num = ObjectGetNumAreaSectors(p_obj) ;
            for (j=0; j<num; j++)  {
                if (ObjectGetNthAreaSector(p_obj, j) == sector)  {
                    foundObj = TRUE ;
                    break ;
                }
            }
            if (foundObj == TRUE)  {
                /* Where is the bottom of this object? */
                bottom = ObjectGetZ16(p_obj) ;

                /* Where is the top of this object? */
                /* If moved up, change height too. */
                if (bottom < newHeight)  {
                    height = newHeight + ObjectGetHeight(p_obj) ;
                }  else  {
                    height = bottom + ObjectGetHeight(p_obj) ;
                }

                /* Find the lowest ceiling */
                lowest = 0x7FFE ;
                for (j=0; j<num; j++)  {
                    /* How far away is that ceiling? */
                    nth = ObjectGetNthAreaSector(p_obj, j) ;
                    ceiling = MapGetCeilingHeight(nth) ;
                    if (ceiling < lowest)  {
                        lowest = ceiling ;
//MessagePrintf("Check s:%d f:%d c:%d h:%d", ObjectGetNthAreaSector(p_obj, j), newHeight, ceiling, height) ;
                    }
                }

                /* Does the head of the object equal or go over the */
                /* ceiling?  If so, it crushes. */
                if (height >= lowest)  {
                    /* Yes, we are being crushed. */
                    status = TRUE ;
                    break ;
                }
            }
        }

        p_obj = ObjectGetNext(p_obj) ;
    }

    DebugEnd() ;

    sprintf(buffer, "%d", status) ;  // Don't remove this hack to get the compiler to correctly compile
    return status ;
}


#if 0
/****************************************************************************/
/*  Routine:  MapAnimate                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapAnimate goes through and updates any of the frames necessary for   */
/*  animation.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapAnimate(T_void)
{
    T_word32 delta ;
    T_word32 time ;
    T_word32 water ;
    T_sword16 distX, distY ;
    T_3dObject *p_obj ;
    T_word16 i ;

    DebugRoutine("MapAnimate") ;

    time = SyncTimeGet() ;
    delta = time - G_lastUpdateMapAnimate ;

    delta <<= 9 ;

    p_obj = G_First3dObject ;
    while (p_obj != NULL)  {
/*
        if ((ObjectGetType(p_obj) & 0xFFF) == OBJECT_TYPE_APPLE)  {
            ObjectSetZ(p_obj, ((ObjectGetZ(p_obj) >> 16) -
                (MathSineLookup(ObjectGetAccData(p_obj)) >> 12)) << 16) ;
            ObjectAddAccData(p_obj, delta) ;
            ObjectSetZ(p_obj, ((ObjectGetZ(p_obj) >> 16) +
                (MathSineLookup(ObjectGetAccData(p_obj)) >> 12)) << 16) ;
        }
*/

        /* How far away is the object in the X direction? */
        distX = ((ObjectGetX(p_obj) - PlayerGetX())>>16) ;

        if (distX < 0)
		    distX = -distX ;

        /* Is the X close enough for picking up? */
        if (distX < DISTANCE_TO_PICKUP)  {
            /* How far away is the object in the Y direction? */
		    distY = ((ObjectGetY(p_obj) - PlayerGetY()) >> 16) ;
            if (distY < 0)
		        distY = -distY ;

            /* Can the object be picked up? */
		    if (distY < DISTANCE_TO_PICKUP)  {
                /* Yes, the object is under us. */
//                ClientIsOver(p_obj, ObjectGetType(p_obj)) ;
            }
        }

        /* What is the next object. */
        p_obj = p_obj->nextObj ;
    }

    /* Animate the floors (water) */
    water = (time >> 4) & 7 ;
    if (water >= 5)
        water = 8-water ;

/*
    for (i=0; i<G_Num3dSectors; i++)  {
        if (G_3dSectorArray[i].trigger & SECTOR_ANIMATE_FLOOR_FLAG)  {
            *((T_byte8 **)(&G_3dSectorArray[i].floorTx[1])) =
                P_waterTexture[water] ;
        }
	if (G_3dSectorArray[i].trigger & SECTOR_SHIFT_X_FLAG)  {
	    G_3dSectorInfoArray[i].textureXOffset = time & 255 ;
        }
        if (G_3dSectorArray[i].trigger & SECTOR_SHIFT_Y_FLAG)  {
            G_3dSectorInfoArray[i].textureYOffset = (-time) & 255 ;
        }
    }
*/

    G_lastUpdateMapAnimate = time ;

    DebugEnd() ;
}
#endif


/****************************************************************************/
/*  Routine:  MapGetStartLocation                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetStartLocation uses the given number to return by indirection    */
/*  the location of the map's starting point.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 num                -- Number of starting point               */
/*                                                                          */
/*    T_sword16 *p_x, *p_y        -- X, Y starting location                 */
/*                                                                          */
/*    T_word16 *p_angle           -- Angle of starting location             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  07/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapGetStartLocation(
           T_word16 num,
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_word16 *p_angle)
{
    DebugRoutine("MapGetStartLocation") ;
    DebugCheck(p_x != NULL) ;
    DebugCheck(p_y != NULL) ;
    DebugCheck(p_angle != NULL) ;

//printf("MapGetStartLocation: %d (%s)\n", num, DebugGetCallerName()) ;
    /* Choose only 0-3 */
    num &= 3 ;
//    *p_x = 4368 ;
//    *p_y = 4873 ;
//    View3dGetView(p_x, p_y, &height, p_angle) ;
    View3dGetStartLocation(num, p_x, p_y, p_angle) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetNextMapAndPlace                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetNextMapAndPlace returns the map link for the given sector.      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector to get map link                 */
/*                                                                          */
/*    T_word16 *p_nextMap         -- Place to store next map number         */
/*                                                                          */
/*    T_word16 *p_nextMapPlace    -- Place to store next place number       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  11/16/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapGetNextMapAndPlace(
           T_word16 sector,
           T_word16 *p_nextMap,
           T_word16 *p_nextMapPlace)
{
    DebugRoutine("MapGetNextMapAndPlace") ;
    DebugCheck(sector < G_Num3dSectors) ;
    DebugCheck(p_nextMap != NULL) ;
    DebugCheck(p_nextMapPlace != NULL) ;

    if (sector < G_Num3dSectors)  {
        *p_nextMap = G_3dSectorInfoArray[sector].nextMap ;
        *p_nextMapPlace = G_3dSectorInfoArray[sector].nextMapStart ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetOutsideLighting                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetOutsideLighting declares how much light is outside the world.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 outsideLight        -- Light level of outside                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  01/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapSetOutsideLighting(T_byte8 outsideLight)
{
    G_outsideLighting = outsideLight ;
}

/****************************************************************************/
/*  Routine:  MapGetOutsideLighting                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetOutsideLighting returns how much light is outside.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 outsideLight        -- Light level of outside                 */
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
/*    LES  01/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 MapGetOutsideLighting(T_void)
{
    return G_outsideLighting ;
}

/****************************************************************************/
/*  Routine:  MapUpdateLighting                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapUpdateLighting updates all lighting effects for the map.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LightTableRecalculate                                                 */
/*    View3dUpdateSectorLightAnimation                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

#define TICKS_PER_DAY  0x7FFFFL

T_void MapUpdateLighting(T_void)
{
    T_sword32 tickInDay ;

    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("MapUpdateLighting") ;

    /* Recalculate all the lights. */
    tickInDay = (SyncTimeGet()+G_dayTickOffset) & TICKS_PER_DAY ;

    tickInDay >>= 3 ;
    tickInDay = ((180*MathSineLookup(tickInDay))>>16) ;
    if (tickInDay > 115)
        tickInDay = 115 ;
    if (tickInDay < -105)
        tickInDay = -105 ;
    tickInDay += 140 ;

    MapSetOutsideLighting(tickInDay) ;
    LightTableRecalculate(G_mapLightTable, G_outsideLighting) ;
    View3dUpdateSectorLightAnimation() ;
    MapAnimateUpdate(G_mapAnimation) ;

    DebugEnd() ;
    TICKER_TIME_ROUTINE_ENDM("MapUpdateLighting", 500) ;
}

/****************************************************************************/
E_Boolean MapIsDay(T_void)
{
    E_Boolean isDay = TRUE ;
    T_sword32 tickInDay ;
    T_word16 hour ;
    DebugRoutine("MapIsDay") ;

    /* 0 = 6 am */
    tickInDay = (SyncTimeGet()+G_dayTickOffset) & TICKS_PER_DAY ;
    tickInDay *= 1440 ;
    tickInDay /= TICKS_PER_DAY ;
    hour = tickInDay / 60 ;

    /* 14 is 8:00 pm,  28 is 4:00 am,  14 to 28 is night time */
    if ((hour >= 14) && (hour <= 28))
        isDay = FALSE ;

    DebugEnd() ;

    return isDay ;
}

/****************************************************************************/
T_void MapOutputTimeOfDay(T_void)
{
    T_sword32 tickInDay ;
    T_word16 hour ;
    T_word16 min ;
    DebugRoutine("MapOutputTImeOfDay") ;

    /* 0 = 6 am */
    tickInDay = ((SyncTimeGet()+G_dayTickOffset)+(TICKS_PER_DAY/4)) & TICKS_PER_DAY ;

    tickInDay *= 1440 ;
    tickInDay /= TICKS_PER_DAY ;

    hour = tickInDay / 60 ;
    min = tickInDay % 60 ;
    if (hour > 12)
        hour -= 12 ;
    if (hour == 0)
        hour = 12 ;

    MessagePrintf("game time: %d:%02d %s",
        hour,
        min,
        ((tickInDay/60)>=12)?"pm":"am") ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetMainTextureForSide                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetMainTexture changes the texture used on the middle part         */
/*  of walls.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Side definition to affect.             */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlockAndUnfind                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/27/94  Created                                                */
/*    LES  06/21/95  Change from View module to map module                  */
/*    LES  01/09/96  Cleaned up and renamed                                 */
/*                                                                          */
/****************************************************************************/

T_void MapSetMainTextureForSide(T_word16 sideNum, T_byte8 *p_textureName)
{
    T_3dSide *p_side ;

    DebugRoutine("MapSetMainTextureForSide") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = &G_3dSideArray[sideNum] ;

    if (p_side->mainTx[0] != '-')  {
        PictureUnlockAndUnfind(G_3dMainResourceArray[sideNum]) ;
    }

    p_side->mainTx[0] = p_textureName[0] ;

    if (p_textureName[0] != '-')
        *((T_byte8 **)(&p_side->mainTx[1])) =
                PictureLock(p_textureName, &G_3dMainResourceArray[sideNum]) ;
    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  MapSetLowerTextureForSide                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetLowerTextureForSide changes the texture used on the lower part  */
/*  of walls.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Side definition to affect.             */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlockAndUnfind                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/09/96  Created (based off MapSetMainTextureForSide)           */
/*                                                                          */
/****************************************************************************/

T_void MapSetLowerTextureForSide(T_word16 sideNum, T_byte8 *p_textureName)
{
    T_3dSide *p_side ;

    DebugRoutine("MapSetLowerTextureForSide") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = &G_3dSideArray[sideNum] ;

    if (p_side->lowerTx[0] != '-')  {
        PictureUnlockAndUnfind(G_3dLowerResourceArray[sideNum]) ;
    }

    p_side->lowerTx[0] = p_textureName[0] ;

    if (p_textureName[0] != '-')
        *((T_byte8 **)(&p_side->lowerTx[1])) =
                PictureLock(p_textureName, &G_3dLowerResourceArray[sideNum]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetUpperTextureForSide                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetUpperTextureForSide changes the texture used on the lower part  */
/*  of walls.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Side definition to affect.             */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlockAndUnfind                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/09/96  Created (based off MapSetMainTextureForSide)           */
/*                                                                          */
/****************************************************************************/

T_void MapSetUpperTextureForSide(T_word16 sideNum, T_byte8 *p_textureName)
{
    T_3dSide *p_side ;

    DebugRoutine("MapSetUpperTextureForSide") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = &G_3dSideArray[sideNum] ;

    if (p_side->upperTx[0] != '-')  {
        PictureUnlockAndUnfind(G_3dUpperResourceArray[sideNum]) ;
    }

    p_side->upperTx[0] = p_textureName[0] ;

    if (p_textureName[0] != '-')
        *((T_byte8 **)(&p_side->upperTx[1])) =
                PictureLock(p_textureName, &G_3dUpperResourceArray[sideNum]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetWallTexture                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetWallTexture tries to be the all-in-wall way to change wall      */
/*  textures.  If called on a wall, it will only change one part of the     */
/*  wall based on what exists based on the following priority:              */
/*  Main, Lower, & Upper.                                                   */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Side definition to affect.             */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/27/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapSetWallTexture(T_word16 sideNum, T_byte8 *p_textureName)
{
    T_3dSide *p_side ;

    DebugRoutine("MapSetWallTexture") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = &G_3dSideArray[sideNum] ;

    /* Try in this order:  main, lower, & upper */
    if (p_side->mainTx[0] != '-')  {
        MapSetMainTextureForSide(sideNum, p_textureName) ;
    } else if (p_side->lowerTx[0] != '-')  {
        MapSetLowerTextureForSide(sideNum, p_textureName) ;
    } else if (p_side->upperTx[0] != '-')  {
        MapSetUpperTextureForSide(sideNum, p_textureName) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetFloorTextureForSector                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetFloorTextureForSector declares the texture to be used on the    */
/*  given sector floor.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sectorNum          -- Sector to affect.                      */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlockAndUnfind                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/11/96  Created (based off MapSetMainTextureForSide)           */
/*                                                                          */
/****************************************************************************/

T_void MapSetFloorTextureForSector(
           T_word16 sectorNum,
           T_byte8 *p_textureName)
{
    T_3dSector *p_sector ;

    DebugRoutine("MapSetFloorTextureForSector") ;
    DebugCheck(sectorNum < G_Num3dSectors) ;

    p_sector = &G_3dSectorArray[sectorNum] ;

    PictureUnlockAndUnfind(G_3dFloorResourceArray[sectorNum]) ;

    *((T_byte8 **)(&p_sector->floorTx[1])) =
        PictureLock(p_textureName, &G_3dFloorResourceArray[sectorNum]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapSetCeilingTextureForSector                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapSetCeilingTextureForSector declares the texture to be used on the  */
/*  given sector ceiling.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sectorNum          -- Sector to affect.                      */
/*                                                                          */
/*    T_byte8 *p_textureName      -- Name of texture to use.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlockAndUnfind                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/11/96  Created (based off MapSetMainTextureForSide)           */
/*                                                                          */
/****************************************************************************/

T_void MapSetCeilingTextureForSector(
           T_word16 sectorNum,
           T_byte8 *p_textureName)
{
    T_3dSector *p_sector ;

    DebugRoutine("MapSetCeilingTextureForSector") ;
    DebugCheck(sectorNum < G_Num3dSectors) ;

    p_sector = &G_3dSectorArray[sectorNum] ;

    PictureUnlockAndUnfind(G_3dCeilingResourceArray[sectorNum]) ;

    *((T_byte8 **)(&p_sector->ceilingTx[1])) =
        PictureLock(p_textureName, &G_3dCeilingResourceArray[sectorNum]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapExist                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapExist tells if a map exists.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void                                                                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE=Map does exist                    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileExist                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/14/96  Created (based off MapSetMainTextureForSide)           */
/*                                                                          */
/****************************************************************************/

E_Boolean MapExist(T_word32 num)
{
    E_Boolean mapExist = FALSE ;
    T_byte8 filename[20] ;

    DebugRoutine("MapExist") ;
    sprintf(filename, "l%u.map", num) ;

    if (FileExist(filename))  {
        mapExist = TRUE ;
    }
#ifndef NDEBUG
    else  {
#ifndef SERVER_ONLY
        MessageAdd("Map does NOT exist!") ;
#else
        printf("Map does NOT exist!") ;
        DebugCheck(FALSE) ;
#endif
    }
#endif

    DebugEnd() ;

    return mapExist ;
}

/****************************************************************************/
/*  Routine:  MapGetUpperTextureName                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetUpperTextureName  returns the name of the upper texture name    */
/*  on a given side of a wall.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Number of side                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to 8 characters for name       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureGetName                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *MapGetUpperTextureName(T_word16 sideNum)
{
    T_byte8 *p_pic ;
    T_byte8 *p_name ;
    T_3dSide *p_side ;

    DebugRoutine("MapGetUpperTextureName") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = G_3dSideArray + sideNum ;
    if (p_side->upperTx[0] == '-')  {
        p_name = G_noName ;
    } else {
        p_pic = *((T_byte8 **)(p_side->upperTx + 1)) ;
        p_name = PictureGetName(p_pic) ;
    }

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*  Routine:  MapGetLowerTextureName                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetLowerTextureName  returns the name of the lower texture name    */
/*  on a given side of a wall.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Number of side                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to 8 characters for name       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureGetName                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *MapGetLowerTextureName(T_word16 sideNum) 
{
    T_byte8 *p_pic ;
    T_byte8 *p_name ;
    T_3dSide *p_side ;

    DebugRoutine("MapGetLowerTextureName") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = G_3dSideArray + sideNum ;
    if (p_side->lowerTx[0] == '-')  {
        p_name = G_noName ;
    } else {
        p_pic = *((T_byte8 **)(p_side->lowerTx + 1)) ;
        p_name = PictureGetName(p_pic) ;
    }

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*  Routine:  MapGetMainTextureName                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetMainTextureName   returns the name of the main  texture name    */
/*  on a given side of a wall.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Number of side                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to 8 characters for name       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureGetName                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *MapGetMainTextureName(T_word16 sideNum) 
{
    T_byte8 *p_pic ;
    T_byte8 *p_name ;
    T_3dSide *p_side ;

    DebugRoutine("MapGetMainTextureName") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = G_3dSideArray + sideNum ;
    if (p_side->mainTx[0] == '-')  {
        p_name = G_noName ;
    } else {
        p_pic = *((T_byte8 **)(p_side->mainTx + 1)) ;
        p_name = PictureGetName(p_pic) ;
    }

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*  Routine:  MapGetFloorTextureName                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetFloorTextureName  returns the name of the floor texture name    */
/*  in a given sector.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sectorNum          -- Number of sector                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to 8 characters for name       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureGetName                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *MapGetFloorTextureName(T_word16 sectorNum) 
{
    T_3dSector *p_sector ;
    T_byte8 *p_pic ;
    T_byte8 *p_name ;

    DebugRoutine("MapGetFloorTextureName") ;
    DebugCheck(sectorNum < G_Num3dSectors) ;

    p_sector = G_3dSectorArray + sectorNum ;
    p_pic = *((T_byte8 **)(p_sector->floorTx + 1)) ;
    p_name = PictureGetName(p_pic) ;

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*  Routine:  MapGetCeilingTextureName                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetCeilingTextureName returens the name of the ceiling texture     */
/*  in a given sector.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sectorNum          -- Number of sector                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to 8 characters for name       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureGetName                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *MapGetCeilingTextureName(T_word16 sectorNum) 
{
    T_3dSector *p_sector ;
    T_byte8 *p_pic ;
    T_byte8 *p_name ;

    DebugRoutine("MapGetCeilingTextureName") ;
    DebugCheck(sectorNum < G_Num3dSectors) ;

    p_sector = G_3dSectorArray + sectorNum ;
    p_pic = *((T_byte8 **)(p_sector->ceilingTx + 1)) ;
    p_name = PictureGetName(p_pic) ;

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*  Routine:  MapGetTextureXOffset                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetTextureXOffset returns the x offset of a texture on a wall side.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Number of wall side                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Texture shift in X direction           */
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
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 MapGetTextureXOffset(T_word16 sideNum) 
{
    T_3dSide *p_side ;
    T_word16 xOffset ;

    DebugRoutine("MapGetTextureXOffset") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = G_3dSideArray + sideNum ;
    xOffset = p_side->tmXoffset ;

    DebugEnd() ;

    return xOffset ;
}

/****************************************************************************/
/*  Routine:  MapGetTextureYOffset                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetTextureYOffset returns the y offset of a texture on a wall side.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sideNum            -- Number of wall side                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Shift in texture along y direction     */
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
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 MapGetTextureYOffset(T_word16 sideNum)
{
    T_3dSide *p_side ;
    T_word16 yOffset ;

    DebugRoutine("MapGetTextureYOffset") ;
    DebugCheck(sideNum < G_Num3dSides) ;

    p_side = G_3dSideArray + sideNum ;
    yOffset = p_side->tmYoffset ;

    DebugEnd() ;

    return yOffset ;
}

/****************************************************************************/
/*  Routine:  MapGetWalkingFloorHeightAtXY                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetWalkingFloorHeightAtXY returns the height of the floor given    */
/*  a map coordinate.                                                       */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x, y              -- Position on map.                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_sword16                   -- Height there, or -32767                */
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
/*    LES  05/02/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_sword16 MapGetWalkingFloorHeightAtXY(T_sword16 x, T_sword16 y)
{
    T_sword16 floor ;
    T_word16 sector ;

    DebugRoutine("MapGetWalkingFloorHeightAtXY") ;

    sector = View3dFindSectorNum(x, y) ;
    if (sector < G_Num3dSectors)  {
        floor = MapGetWalkingFloorHeight(sector) ;
    } else {
        floor = -32767 ;
    }

    DebugEnd() ;

    return floor ;
}

/* LES: 05/09/96 -- Return what type of sector this is. */
T_sectorType MapGetSectorType(T_word16 sectorNum)
{
    T_sectorType type = 0 ;

    DebugRoutine("MapGetSectorType") ;

    if (sectorNum < G_Num3dSectors)  {
        type = G_3dSectorInfoArray[sectorNum].type ;
    } else  {
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;

    return type ;
}

/* LES: 05/13/96 -- Make a side go through a different animation */
T_void MapSetSideState(T_word16 sideNum, T_word16 sideState)
{
    DebugRoutine("MapSetSideState") ;

    MapAnimateStartSide(
        G_mapAnimation,
        sideNum,
        sideState) ;

    DebugEnd() ;
}

/* LES: 05/13/96 -- Make a wall go through a different animation */
T_void MapSetWallState(T_word16 wallNum, T_word16 wallState)
{
    DebugRoutine("MapSetWallState") ;

    MapAnimateStartWall(
        G_mapAnimation,
        wallNum,
        wallState) ;

    DebugEnd() ;
}

/* LES: 05/13/96 -- Make a sector go through a different animation */
T_void MapSetSectorState(T_word16 sectorNum, T_word16 sectorState)
{
    DebugRoutine("MapSetSectorState") ;

    MapAnimateStartSector(
        G_mapAnimation,
        sectorNum,
        sectorState) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetWallDamage                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetWallDamage is called to determine if an object is being damaged */
/*  by a wall animation.                                                    */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to check                        */
/*                                                                          */
/*    T_word16 *p_damageAmount    -- Amount of damage                       */
/*                                                                          */
/*    E_effectDamageType *p_damageType -- Type of damage to be done         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE=found damaging wall               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean MapGetWallDamage(
              T_3dObject *p_obj,
              T_word16 *p_damageAmount,
              T_byte8 *p_damageType)
{
    E_Boolean didGet = FALSE ;

    DebugRoutine("MapGetWallDamage") ;
    DebugCheck(G_mapAnimation != MAP_ANIMATION_BAD) ;

    didGet = MapAnimateGetWallDamage(
                 G_mapAnimation,
                 p_obj,
                 p_damageAmount,
                 p_damageType) ;

    DebugEnd() ;

    return didGet ;
}

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
static FILE *G_textureShiftFile ;

T_void MapShiftTextureOpenFile(T_void)
{
    DebugRoutine("MapShiftTextureOpenFile") ;

    G_textureShiftFile = fopen("texshift.dat", "w") ;

    DebugEnd() ;
}

T_void MapShiftTextureCloseFile(T_void)
{
    DebugRoutine("MapShiftTextureCloseFile") ;

    fclose(G_textureShiftFile) ;

    DebugEnd() ;
}

T_void MapShiftTextureRight(T_sword16 amount)
{
    T_word16 sideNum ;

    DebugRoutine("MapShiftTextureRight") ;

    sideNum = View3dGetTextureSideNum() ;
    if (sideNum != 0xFFFF)  {
        G_3dSideArray[sideNum].tmXoffset-- ;
        fprintf(
            G_textureShiftFile,
            "%d %d %d\n", sideNum,
            G_3dSideArray[sideNum].tmXoffset,
            G_3dSideArray[sideNum].tmYoffset) ;
    }

    DebugEnd() ;
}

T_void MapShiftTextureLeft(T_sword16 amount)
{
    T_word16 sideNum ;

    DebugRoutine("MapShiftTextureLeft") ;

    sideNum = View3dGetTextureSideNum() ;
    if (sideNum != 0xFFFF)  {
        G_3dSideArray[sideNum].tmXoffset++ ;
        fprintf(
            G_textureShiftFile,
            "%d %d %d\n", sideNum,
            G_3dSideArray[sideNum].tmXoffset,
            G_3dSideArray[sideNum].tmYoffset) ;
    }

    DebugEnd() ;
}

T_void MapShiftTextureUp(T_sword16 amount)
{
    T_word16 sideNum ;

    DebugRoutine("MapShiftTextureUp") ;

    sideNum = View3dGetTextureSideNum() ;
    if (sideNum != 0xFFFF)  {
        G_3dSideArray[sideNum].tmYoffset++ ;
        fprintf(
            G_textureShiftFile,
            "%d %d %d\n", sideNum,
            G_3dSideArray[sideNum].tmXoffset,
            G_3dSideArray[sideNum].tmYoffset) ;
    }

    DebugEnd() ;
}

T_void MapShiftTextureDown(T_sword16 amount)
{
    T_word16 sideNum ;

    DebugRoutine("MapShiftTextureDown") ;

    sideNum = View3dGetTextureSideNum() ;
    if (sideNum != 0xFFFF)  {
        G_3dSideArray[sideNum].tmYoffset-- ;
        fprintf(
            G_textureShiftFile,
            "%d %d %d\n", sideNum,
            G_3dSideArray[sideNum].tmXoffset,
            G_3dSideArray[sideNum].tmYoffset) ;
    }

    DebugEnd() ;
}
#endif

/****************************************************************************/
/*  Routine:  MapOpenForwardWall                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapOpenForwardWall tries to open a door attached to the forward       */
/*  wall.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/10/95  Created                                                */
/*    LES  06/12/96  Changed to work with p_obj                             */
/*                                                                          */
/****************************************************************************/

T_void MapOpenForwardWall(
           T_3dObject *p_obj,
           E_Boolean checkForItemRequired)
{
    T_word16 num ;
    T_wallListItem wallList[20] ;
    T_sword32 newX, newY ;
    T_sword32 reach = 64 ;
    T_word16 side ;
    T_word16 door = 0 ;
    T_word16 j ;
    T_word16 stepSize = 32 ;

    do {
        newX = ObjectGetX(p_obj) + MathCosineLookup(ObjectGetAngle(p_obj))*reach ;
        newY = ObjectGetY(p_obj) + MathSineLookup(ObjectGetAngle(p_obj))*reach ;
        num = Collide3dFindWallList(
                  (T_sword16)(ObjectGetX16(p_obj)),
                  (T_sword16)(ObjectGetY16(p_obj)),
                  (T_sword16)(newX>>16),
                  (T_sword16)(newY>>16),
                  (T_sword16)(ObjectGetZ16(p_obj)+(ObjectGetHeight(p_obj)>>1)),
                  20,
                  wallList,
                  WALL_LIST_ITEM_UPPER) ;

        /* No walls were hit. */
        if ((num == 0) && (stepSize != 0))  {
            /* If we have tried reaching out the maximum distance, quit. */
            if (reach == 64)
                break ;
            /* Try to reach further. */
            reach += stepSize ;
        } else if ((num > 1) && (stepSize != 0)) {
            /* Too many lines, try cutting it shorter. */
            reach -= stepSize ;
        } else {
            /* We are only touching one.  Must be the only */
            /* possible door. */
            for (j=0; j<2; j++)  {
                side = G_3dLineArray[wallList[0].lineNumber].side[j] ;
                if (side != 0xFFFF)  {
                    door = G_3dSideArray[side].sector ;
                    if (DoorIsAtSector(door))  {
                        if (!DoorIsLock(door))  {
                            DoorOpen(door) ;
                        }
                    }
                }
            }

            /* Stop looping. */
            break ;
        }
        stepSize>>=1 ;
        /* Loop while our reach is still good. */
    } while (stepSize > 0) ;
}

/****************************************************************************/
/*  Routine:  MapForceOpenForwardWall                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapForceOpenForwardWall always opens the forward wall.  If the door   */
/*  is locked and/or requires an item, the door is unlocked and gets rid    */
/*  of the item.                                                            */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object doing the open                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapForceOpenForwardWall(T_3dObject *p_obj)
{
    T_word16 num ;
    T_wallListItem wallList[20] ;
    T_sword32 newX, newY ;
    T_sword32 reach = 64 ;
    T_word16 side ;
    T_word16 door = 0 ;
    T_word16 j ;
    T_word16 stepSize = 32 ;

    DebugRoutine("MapForceOpenForwardWall") ;

    do {
        newX = ObjectGetX(p_obj) + MathCosineLookup(ObjectGetAngle(p_obj))*reach ;
        newY = ObjectGetY(p_obj) + MathSineLookup(ObjectGetAngle(p_obj))*reach ;
        num = Collide3dFindWallList(
                  (T_sword16)(ObjectGetX16(p_obj)),
                  (T_sword16)(ObjectGetY16(p_obj)),
                  (T_sword16)(newX>>16),
                  (T_sword16)(newY>>16),
                  (T_sword16)(ObjectGetZ16(p_obj)+(ObjectGetHeight(p_obj)>>1)),
                  20,
                  wallList,
                  WALL_LIST_ITEM_UPPER) ;

        /* No walls were hit. */
        if ((num == 0) && (stepSize != 0))  {
            /* If we have tried reaching out the maximum distance, quit. */
            if (reach == 64)
                break ;
            /* Try to reach further. */
            reach += stepSize ;
        } else if ((num > 1) && (stepSize != 0)) {
            /* Too many lines, try cutting it shorter. */
            reach -= stepSize ;
        } else {
            /* We are only touching one.  Must be the only */
            /* possible door. */
            for (j=0; j<2; j++)  {
                side = G_3dLineArray[wallList[0].lineNumber].side[j] ;
                if (side != 0xFFFF)  {
                    door = G_3dSideArray[side].sector ;
                    if (DoorIsAtSector(door))  {
                        /* Unlock and remove any required items. */
                        DoorUnlock(door) ;
                        DoorSetRequiredItem(door, 0) ;

                        DoorOpen(door) ;
                    }
                }
            }

            /* Stop looping. */
            break ;
        }
        stepSize>>=1 ;
        /* Loop while our reach is still good. */
    } while (stepSize > 0) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MapGetForwardWallActivationType                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MapGetForwardWallActivationType returns the type of action that will  */
/*  occur (including specific data) if the forward wall is activated.       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Thing opening door                     */
/*                                                                          */
/*    E_Boolean checkForItemRequired -- TRUE if you need this checked.      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE if can open, else false           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MapGetForwardWallActivationType(
           T_3dObject *p_obj,
           E_wallActivation *p_type,
           T_word16 *p_data)
{
    T_word16 num ;
    T_wallListItem wallList[20] ;
    T_sword32 newX, newY ;
    T_sword32 reach = 64 ;
    T_word16 side ;
    T_word16 door = 0 ;
    T_word16 j ;
    T_word16 stepSize = 32 ;

    DebugRoutine("MapGetForwardWallActivationType") ;

    *p_type = WALL_ACTIVATION_NONE ;
    do {
        /* Find any line that is solid. */
        newX = ObjectGetX(p_obj) + MathCosineLookup(ObjectGetAngle(p_obj))*reach ;
        newY = ObjectGetY(p_obj) + MathSineLookup(ObjectGetAngle(p_obj))*reach ;
        num = Collide3dFindWallList(
                  (T_sword16)(ObjectGetX16(p_obj)),
                  (T_sword16)(ObjectGetY16(p_obj)),
                  (T_sword16)(newX>>16),
                  (T_sword16)(newY>>16),
                  (T_sword16)(ObjectGetZ16(p_obj)+(ObjectGetHeight(p_obj)>>1)),
                  20,
                  wallList,
                  WALL_LIST_ITEM_UPPER |
                      WALL_LIST_ITEM_LOWER |
                          WALL_LIST_ITEM_MAIN) ;

        /* No walls were hit. */
        if ((num == 0) && (stepSize != 0))  {
            /* If we have tried reaching out the maximum distance, quit. */
            if (reach == 64)
                break ;
            /* Try to reach further. */
            reach += stepSize ;
        } else if ((num > 1) && (stepSize != 0)) {
            /* Too many lines, try cutting it shorter. */
            reach -= stepSize ;
        } else {
            /* We are only touching one.  Must be the only */
            /* possible door or wall. */
            /* Check if there is a script on the line. */
            if (G_3dLineArray[wallList[0].lineNumber].special != 0)  {
                /* A script is found.  Report this. */
                *p_type = WALL_ACTIVATION_SCRIPT ;
                *p_data = G_3dLineArray[wallList[0].lineNumber].special ;
            } else {
                /* Only consider it to be a door if it is an upper. */
                if (wallList[0].itemType == WALL_LIST_ITEM_UPPER)  {
                    /* Check if there is a door attached. */
                    for (j=0; j<2; j++)  {
                        side = G_3dLineArray[wallList[0].lineNumber].side[j] ;
                        if (side != 0xFFFF)  {
                            door = G_3dSideArray[side].sector ;
                            if (DoorIsAtSector(door))  {
                                /* Found a door on the line. */
                                *p_type = WALL_ACTIVATION_DOOR ;
                                *p_data = door ;
                            }
                        }
                    }
                }
            }

            /* Stop looping. */
            break ;
        }
        stepSize>>=1 ;
        /* Loop while our reach is still good. */
    } while (stepSize > 0) ;

    DebugEnd() ;
}

T_void MapSetDayOffset(T_word32 offset)
{
    G_dayTickOffset = offset ;
}

T_word32 MapGetDayOffset(T_void)
{
    return G_dayTickOffset ;
}

T_word32 MapGetMapSpecial(T_void)
{
    return G_mapSpecial ;
}

/****************************************************************************/
/*    END OF FILE:  MAP.C                                                   */
/****************************************************************************/
