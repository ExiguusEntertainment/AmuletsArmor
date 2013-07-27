/****************************************************************************/
/*    FILE:  MAPDIFF.H                                                      */
/****************************************************************************/

typedef T_byte8 E_changedObjectAction ;
#define CHANGED_OBJECT_ADD                   0
#define CHANGED_OBJECT_DESTROY               1
#define CHANGED_OBJECT_TYPE                  2
#define CHANGED_OBJECT_POSITION              3
#define CHANGED_OBJECT_UNKNOWN               4

typedef struct {
    T_word16 lastChangeLevel ;          /* Time line of last change */
                                        /* Only for server. */
    T_word16 numTries ;                 /* How many updates have been tried. */
    T_word16 numDiffWalls ;
    T_word16 numDiffSectors ;
} T_changeHeader ;

#define TEXTURE_NAME_LEN 8

typedef struct {
    T_word16 sideNum ;
    T_byte8 upperTexture[TEXTURE_NAME_LEN] ;
    T_byte8 lowerTexture[TEXTURE_NAME_LEN] ;
    T_byte8 mainTexture[TEXTURE_NAME_LEN] ;
    T_word16 textureXOffset ;
    T_word16 textureYOffset ;
} T_mapDiffSide ;

typedef struct {
    T_3dSector basicData ;
    T_3dSectorInfo moreData ;
} T_mapDiffSector ;

typedef struct {
    E_changedObjectAction action ;
    T_word16 serverID ;
    T_word32 uniqueID ;
    T_word16 X, Y, Z ;
    T_word16 type ;
    T_word32 scaleX, scaleY ;
    T_word16 health ;
    T_word16 bodyParts[MAX_BODY_PARTS] ;
} T_changeObjectAdd ;

typedef struct {
    E_changedObjectAction action ;
    T_word16 serverID ;
} T_changeObjectDestroy ;

typedef struct {
    E_changedObjectAction action ;
    T_word16 serverID ;
    T_word16 type ;
    T_word32 scaleX, scaleY ;
    T_word16 bodyParts[MAX_BODY_PARTS] ;
} T_changeObjectType ;

typedef struct {
    E_changedObjectAction action ;
    T_word16 serverID ;
    T_sword16 X, Y, Z ;
} T_changeObjectPosition ;

T_void MapDiffInitialize(T_word16 numSides, T_word16 numSectors) ;

T_void MapDiffFinish(T_void) ;

T_void MapDiffSector(T_word16 sectorNum) ;

T_void MapDiffSide(T_word16 sideNum) ;

T_void MapDiffCreatedObject(T_3dObject *p_obj) ;

T_void MapDiffDestroyingObject(T_3dObject *p_obj) ;

T_void MapDiffChangeTypeObject(T_3dObject *p_obj) ;

T_void *MapDiffCreateUpdateBlock(
            T_void *lastUpdateBlock, 
            T_word32 *size) ;

/****************************************************************************/
/*    END OF FILE: MAPDIFF.H                                                */
/****************************************************************************/

