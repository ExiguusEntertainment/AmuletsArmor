/****************************************************************************/
/*    FILE:  3D_IO.H                                                        */
/****************************************************************************/

#ifndef _3D_IO_H_
#define _3D_IO_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

typedef T_void *T_mapGroup ;
#define MAP_GROUP_BAD NULL

#define VIEW3D_MAX_OBJECTS 1000

#define OBJ_COLLISION_GROUP_NONE        0xFFFF

/* Map information telling how many of each item is available. */
extern T_word16             G_Num3dThings;
extern T_word16             G_Num3dSegs;
extern T_word16             G_Num3dSides;
extern T_word16             G_Num3dLines;
extern T_word16             G_Num3dNodes;
extern T_word16             G_Num3dSectors;
extern T_word16             G_Num3dSSectors;
extern T_word16             G_Num3dVertexes;
extern T_word16             G_BlockmapSize;
extern T_word16             G_Num3dObjects ;

extern T_word16             G_objCollisionNumX ;
extern T_word16             G_objCollisionNumY ;
extern T_word16             G_lastCollisionList ;

/* Arrays to store the map information. */
extern T_3dSegment         *G_3dSegArray;
extern T_3dSide            *G_3dSideArray;
extern T_3dLine            *G_3dLineArray;
extern T_3dNode            *G_3dNodeArray;
extern T_3dNode           **G_3dPNodeArray;
extern T_3dSector          *G_3dSectorArray;
extern T_3dVertex          *G_3dVertexArray;
extern T_3dSegmentSector   *G_3dSegmentSectorArray;
extern T_3dBlockMap        *G_3dBlockMapArray;
extern T_3dBlockMapHeader  *G_3dBlockMapHeader;
extern T_3dObject          *G_First3dObject ;
extern T_3dObject          *G_Last3dObject ;
extern T_sword16            G_3dRootBSPNode ;
extern T_byte8             *G_3dReject ;
extern T_doubleLinkList    *G_3dObjCollisionLists ;

/* Player's location. */
extern T_sword16            G_3dPlayerX ;
extern T_sword16            G_3dPlayerY ;
extern T_sword32            G_3dPlayerHeight ;

/* Direction player is facing. */
extern T_word16             G_3dPlayerAngle ;

/* Left angle of view (calculated from player's viewing angle). */
extern T_word16             G_3dPlayerLeftAngle ;
extern T_word16             G_3dPlayerRightAngle ;

/* Precomputed sine and cosine of the player's angle. */
extern T_sword32            G_3dCosPlayerAngle ;
extern T_sword32            G_3dSinPlayerAngle ;

extern T_resource          *G_3dUpperResourceArray ;
extern T_resource          *G_3dLowerResourceArray ;
extern T_resource          *G_3dMainResourceArray ;
extern T_resource          *G_3dFloorResourceArray ;
extern T_resource          *G_3dCeilingResourceArray ;

/* Keep track of information about a sector */
extern T_3dSectorInfo      *G_3dSectorInfoArray ;

#define OBJECT_TYPE_PLAYER_1_START      1
#define OBJECT_TYPE_PLAYER_4_START      4

T_void View3dLoadMap(T_byte8 *MapName) ;
T_void View3dUnloadMap(T_void) ;

T_void View3dGetStartLocation(
           T_word16 num,
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_word16 *p_angle) ;

#ifdef MIP_MAPPING_ON
T_byte8 *MipMap(T_byte8 *p_texture) ;
T_void ReleaseMipMap(T_byte8 *p_mipmap) ;
#else
#define MipMap(p_texture)  (p_texture)
#define ReleaseMipMap(p_mipmap)
#endif

T_void View3dResolveSpecialObjects(T_void) ;

T_sword16 View3dCalculateLowestCeiling(T_word16 sector) ;

T_void View3dSetSectorType(T_word16 sector, T_word16 type) ;

#endif

/* Context switching between maps is provided below: */
T_mapGroup View3dCreateMapGroup(T_void) ;

T_void View3dDestroyMapGroup(T_mapGroup p_mapGroup) ;

T_void View3dSetMapGroup(T_mapGroup p_mapGroup) ;

T_void View3dGetMapGroup(T_mapGroup p_mapGroup) ;


/****************************************************************************/
/*    END OF FILE:  3D_IO.H                                                 */
/****************************************************************************/
