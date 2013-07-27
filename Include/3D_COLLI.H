/****************************************************************************/
/*    FILE:  3D_COLLI.H                                                     */
/****************************************************************************/
#ifndef _3D_COLLI_H_
#define _3D_COLLI_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

#define WALL_LIST_ITEM_UPPER       0x01  /* ---- ---1 */
#define WALL_LIST_ITEM_LOWER       0x02  /* ---- --1- */
#define WALL_LIST_ITEM_MAIN        0x04  /* ---- -1-- */

typedef struct {
    T_byte8 itemType ;
    T_word16 lineNumber ;
} T_wallListItem ;

extern T_word16 G_numHits ;

T_sword16 View3dMoveToXY(
              T_sword32 *oldX,
              T_sword32 *oldY,
              T_sword32 newx,
              T_sword32 newy,
              T_sword32 radius,
              T_sword32 foot,
              T_sword32 head,
              T_sword16 height,
              T_3dObject *p_obj) ;

E_Boolean View3dObjectHitFast(
              T_sword16 x,
              T_sword16 y,
              T_word16 radius,
              T_sword16 lastX,
              T_sword16 lastY,
              T_sword16 zBottom,
              T_sword16 zTop,
              T_sword16 height,
              T_3dObject *p_movingObject) ;

T_sword16 View3dMoveTo(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_word16 angle,
         T_sword32 step,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword16 height,
         T_3dObject *p_obj) ;

T_sword16 View3dMoveToFast(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_word16 angle,
         T_sword32 step,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword32 height,
         T_3dObject *p_obj) ;

T_void View3dGetFloorAndCeilingHeight(T_sword16 *floor, T_sword16 *ceiling) ;

T_void View3dGetFloorAbove(T_word16 *above) ;

T_void View3dGetCeilingBelow(T_word16 *below) ;

T_void View3dSetExceptObject(T_word16 exceptObject) ;

T_void View3dSetExceptObjectByPtr(T_objMoveStruct *p_object) ;

E_Boolean View3dObjectHit(T_sword16 x, T_sword16 y, T_word16 radius) ;

T_word16 *View3dGetSurroundingSectors(T_word16 *numSectors) ;

T_void View3dSetMoveAccuracy(T_word16 accuracy) ;

T_word16 View3dGetMoveAccuracy(T_void) ;

T_word16 View3dFindLineHits(
           T_sword16 x1,
           T_sword16 y1,
           T_sword16 x2,
           T_sword16 y2,
           T_word16 maxHits,
           T_sword16 *lines) ;

T_void View3dDisallowDip(T_void) ;

T_void View3dAllowDip(T_void) ;

T_word16 Collide3dGetWallsInBox(
             T_sword32 x,
             T_sword32 y,
             T_word16 radius,
             T_word16 maxWalls,
             T_word16 *p_walls) ;

E_Boolean Collide3dGetSectorsInBox(
              T_sword32 x,
              T_sword32 y,
              T_word16 radius,
              T_word16 maxSectors,
              T_word16 *p_sectors,
              T_word16 *p_numSectors) ;

E_Boolean Collide3dMoveToXYZ(
              T_3dObject *p_obj,
              T_sword32 newX,
              T_sword32 newY,
              T_sword32 newZ) ;

T_sword16 Collide3dCheckSegmentHitsSegment(
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_word16 lineNum) ;

T_byte8 Collide3dPointOnRight(
            T_sword32 lineX1,
            T_sword32 lineY1,
            T_sword32 lineX2,
            T_sword32 lineY2,
            T_sword32 pointX,
            T_sword32 pointY) ;

E_Boolean Collide3dCheckLineOfSight(
              T_sword16 sightStartX,
              T_sword16 sightStartY,
              T_sword16 sightEndX,
              T_sword16 sightEndY) ;

T_word16 Collide3dFindWallList(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2,
             T_sword16 z,
             T_word16 maxWalls,
             T_wallListItem *p_list,
             T_byte8 wallTypes) ;

E_Boolean View3dIsAllowDip(T_void) ;

T_void Collide3dSetWallDefinition(T_word16 lineFlags) ;

T_sword16 Collide3dCheckSegmentHitBox(
              T_word16 lineNum,
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2) ;

E_Boolean Collide3dObjectToObjectCheckLineOfSight(
              T_3dObject *p_from,
              T_3dObject *p_to) ;

typedef struct {
    T_sword16 x, y ;
    T_word16 sector ;
} T_lineOfSightLast ;

T_void Collide3dUpdateLineOfSightLast(
           T_lineOfSightLast *p_lastSight,
           T_3dObject *p_target) ;

E_Boolean Collide3dObjectToXYCheckLineOfSight(
              T_3dObject *p_from,
              T_lineOfSightLast *p_lastSight,
              T_sword16 x,
              T_sword16 y) ;

E_Boolean Collide3dObjectToXYCheckLineOfSightWithZ(
              T_3dObject *p_from,
              T_sword16 x,
              T_sword16 y,
              T_sword16 z) ;

#endif

/****************************************************************************/
/*    END OF FILE:  3D_COLLI.H                                              */
/****************************************************************************/
