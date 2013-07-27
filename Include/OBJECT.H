/****************************************************************************/
/*    FILE:  OBJECT.H                                                       */
/****************************************************************************/
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "3D_IO.H"
#include "GRAPHICS.H"
#include "VIEWFILE.H"

#define STANCE_STAND  0
#define STANCE_WALK   1
#define STANCE_HURT   2
#define STANCE_DIE    3
#define STANCE_ATTACK 4
#define STANCE_ATTACK2 5
#define STANCE_UNKNOWN 6

typedef E_Boolean (*T_objectDoToAllCallback)(
                          T_3dObject *p_obj,
                          T_word32 data) ;

#define OBJECT_ATTR_PASSABLE         0x0001    /* ---- ---- ---- ---1 */
#define OBJECT_ATTR_GRABABLE         0x0002    /* ---- ---- ---- --1- */
#define OBJECT_ATTR_INVISIBLE        0x0004    /* ---- ---- ---- -1-- */
#define OBJECT_ATTR_NOT_USED1        0x0008    /* ---- ---- ---- 1--- */
#define OBJECT_ATTR_ADDABLE          0x0010    /* ---- ---- ---1 ---- */
#define OBJECT_ATTR_WEAPON           0x0020    /* ---- ---- --1- ---- */
#define OBJECT_ATTR_NO_SHADING       0x0040    /* ---- ---- -1-- ---- */
#define OBJECT_ATTR_EDIBLE           0x0080    /* ---- ---- 1--- ---- */
#define OBJECT_ATTR_TRANSLUCENT      0x0100    /* ---- ---1 ---- ---- */
#define OBJECT_ATTR_SLIDE_ONLY       0x0200    /* ---- --1- ---- ---- */
#define OBJECT_ATTR_PIECE_WISE       0x0400    /* ---- -1-- ---- ---- */
#define OBJECT_ATTR_MARK_IMPASSIBLE_WHEN_FREE 0x0800
                                               /* ---- 1--- ---- ---- */
#define OBJECT_ATTR_MARK_FOR_DESTROY 0x1000    /* ---1 ---- ---- ---- */
#define OBJECT_ATTR_FULLY_PASSABLE   0x2000    /* --1- ---- ---- ---- */
#define OBJECT_ATTR_BODY_PART        0x4000    /* -1-- ---- ---- ---- */
#define OBJECT_ATTR_STEALTHY         0x8000    /* 1--- ---- ---- ---- */

#define OBJECT_TYPE_NONE   0

#define ObjectGetX(p_obj)      ObjMoveGetX(&(p_obj)->objMove)
#define ObjectGetY(p_obj)      ObjMoveGetY(&(p_obj)->objMove)
#define ObjectGetZ(p_obj)      ObjMoveGetZ(&(p_obj)->objMove)
#define ObjectGetX16(p_obj)      (ObjMoveGetX(&(p_obj)->objMove)>>16)
#define ObjectGetY16(p_obj)      (ObjMoveGetY(&(p_obj)->objMove)>>16)
#define ObjectGetZ16(p_obj)      (ObjMoveGetZ(&(p_obj)->objMove)>>16)
#define ObjectGetRadius(p_obj) ObjMoveGetRadius(&(p_obj)->objMove)
#define ObjectGetHeight(p_obj)  ObjMoveGetHeight(&(p_obj)->objMove)
#define ObjectGetAttributes(p_obj)  ((p_obj)->attributes)
#define ObjectGetOrientation(p_obj) ((p_obj)->orientation)
#define ObjectGetAngle(p_obj)    (ObjMoveGetAngle(&(p_obj)->objMove))
#define ObjectGetHealth(p_obj)    ((p_obj)->health)
#define ObjectGetUniqueId(p_obj)        ((p_obj)->objUniqueId)
#define ObjectGetServerId(p_obj)        ((p_obj)->objServerId)
#define ObjectGetStance(p_obj)    ObjTypeGetStance((p_obj)->p_objType)
#define ObjectGetFrame(p_obj)    ObjTypeGetFrame((p_obj)->p_objType)
#define ObjectGetAnimData(p_obj)     ObjTypeGetAnimData((p_obj)->p_objType)
#define ObjectGetExtraData(p_obj)   ((p_obj)->extraData)
#define ObjectGetScript(p_obj)  ObjTypeGetScript((p_obj)->p_objType)
#define ObjectGetScriptHandle(p_obj)  ((p_obj)->script)
#define ObjectGetNumPackets(p_obj)  ((p_obj)->numPackets)
#define ObjectGetChainedObjects(p_obj)  ((p_obj)->p_chainedObjects)
#define ObjectGetScaleX(p_obj)          ((p_obj)->scaleX)
#define ObjectGetScaleY(p_obj)          ((p_obj)->scaleY)
#define ObjectGetIllumination(p_obj)    ((p_obj)->illumination)
#define ObjectGetOwnerID(p_obj)         ((p_obj)->ownerID)
#define ObjectUpdateZVel(p_obj, delta)  ObjMoveUpdateZVel( \
                                            &(p_obj)->objMove, \
                                            (delta))

#define ObjectIsWeapon(p_obj)           (((p_obj)->attributes) & \
                                         OBJECT_ATTR_WEAPON)
#define ObjectIsBodyPart(p_obj)           (((p_obj)->attributes) & \
                                         OBJECT_ATTR_BODY_PART)
#define ObjectIsMarkedForDestroy(p_obj) (((p_obj)->attributes) & \
                                         OBJECT_ATTR_MARK_FOR_DESTROY)
#define ObjectIsFullyPassable(p_obj) (((p_obj)->attributes) & \
                                         OBJECT_ATTR_FULLY_PASSABLE)
#define ObjectMakeFullyPassable(p_obj) (((p_obj)->attributes) |= \
                                         OBJECT_ATTR_FULLY_PASSABLE)

#define ObjectMakeInvisible(p_obj)  \
            ((p_obj)->attributes |= OBJECT_ATTR_INVISIBLE)
#define ObjectMakeVisible(p_obj)  \
            ((p_obj)->attributes &= (~OBJECT_ATTR_INVISIBLE))
#define ObjectIsInvisible(p_obj) \
            (((p_obj)->attributes) & OBJECT_ATTR_INVISIBLE)

#define ObjectGetMaxVelocity(p_obj) \
            ObjMoveGetMaxVelocity(&(p_obj)->objMove)

#define ObjectMakeTranslucent(p_obj) \
            (((p_obj)->attributes) |= OBJECT_ATTR_TRANSLUCENT)
#define ObjectMakeSolid(p_obj) \
            (((p_obj)->attributes) &= (~OBJECT_ATTR_TRANSLUCENT))
#define ObjectIsTranslucent(p_obj) \
            (((p_obj)->attributes) & OBJECT_ATTR_TRANSLUCENT)

#define ObjectCheckMoved(p_obj)   ObjMoveCheckMovedFlag(&(p_obj)->objMove)
#define ObjectHasEverMoved(p_obj)  ObjMoveHasEverMoved(&(p_obj)->objMove)
#define ObjectClearMovedFlag(p_obj)   \
            ObjMoveClearMovedFlag(&(p_obj)->objMove)
#define ObjectClearBlockedFlag(p_obj) \
            ObjMoveClearBlockedFlag(&(p_obj)->objMove)
#define ObjectSetMovedFlag(p_obj) \
            ObjMoveSetMovedFlag(&(p_obj)->objMove)
#define ObjectSetX(p_obj, x)    \
            ObjMoveSetX(&(p_obj)->objMove, (x))
#define ObjectSetY(p_obj, y)    \
            ObjMoveSetY(&(p_obj)->objMove, (y))
#define ObjectSetZ(p_obj, z)    \
            ObjMoveSetZ(&(p_obj)->objMove, (z))
#define ObjectSetX16(p_obj, x)   ObjectSetX(p_obj, (((T_sword32)(x))<<16))
#define ObjectSetY16(p_obj, y)   ObjectSetY(p_obj, (((T_sword32)(y))<<16))
#define ObjectSetZ16(p_obj, z)   ObjectSetZ(p_obj, (((T_sword32)(z))<<16))
#define ObjectSetAttributes(p_obj, attr)  ((p_obj)->attributes = (attr))
#define ObjectAddAttributes(p_obj,attr)   ((p_obj)->attributes |= (attr))
#define ObjectRemoveAttributes(p_obj,attr)   ((p_obj)->attributes &= (~(attr)))
#define ObjectSetRadius(p_obj, rad)  \
             ObjMoveSetRadius(&(p_obj)->objMove, (rad))
#define ObjectSetUniqueId(p_obj, id)        ((p_obj)->objUniqueId = (id))
#define ObjectSetServerId(p_obj, id)        ((p_obj)->objServerId = (id))
/*
#define ObjectSetServerId(p_obj, id)        { ((p_obj)->objServerId = (id)) ; \
                                              printf("Object %p set to id %d\n",p_obj, id) ; }
*/

#define ObjectSetHeight(p_obj, height)  ObjMoveSetHeight(&(p_obj)->objMove, height)
#define ObjectSetClimbHeight(p_obj, height) \
            ObjMoveSetClimbHeight(&(p_obj)->objMove, height)
#define ObjectAddAngle(p_obj, angle)  \
            ObjMoveSetAngle(&(p_obj)->objMove, (angle) + \
                ObjMoveGetAngle(&(p_obj)->objMove))
#define ObjectSetChainedObjects(p_obj, objs) \
            ((p_obj)->p_chainedObjects = (objs))
#define ObjectForceUpdate(p_obj) \
            ObjMoveForceUpdate(&(p_obj)->objMove)
#define ObjectGetAccData(p_obj) ((p_obj)->accessoryData)
#define ObjectSetAccData(p_obj, data)  ((p_obj)->accessoryData = (data))
#define ObjectAddAccData(p_obj, data)  ((p_obj)->accessoryData += (data))
#define ObjectSetOrientation(p_obj, ori) ((p_obj)->orientation = (ori))
#define ObjectSetHealth(p_obj, value)   ((p_obj)->health = (value))
#define ObjectSetScript(p_obj, scr)    \
            ObjTypeSetScript((p_obj)->p_objType, (scr))
#define ObjectSetExtraData(p_obj, data)  ((p_obj)->extraData = (data))
#define ObjectSetScaleX(p_obj, sx)     (((p_obj)->scaleX) = (sx))
#define ObjectSetScaleY(p_obj, sy)     (((p_obj)->scaleY) = (sy))
#define ObjectSetIllumination(p_obj, illum)  \
              (((p_obj)->illumination) = (illum))
#define ObjectSetOwnerID(p_obj, id)         \
              (((p_obj)->ownerID) = (id))
#define ObjectSetHashPointer(p_obj, p_new) (((p_obj)->p_hash) = (p_new))

#define ObjectSetColorizeTable(p_obj, ct) \
             (((p_obj)->objectType) = \
              (((p_obj)->objectType) & (~OBJECT_TYPE_COLOR_MASK)) | \
             (((T_word16)(ct)) << OBJECT_TYPE_COLOR_OFFSET))

#define ObjectSetNumPackets(p_obj, num) ((p_obj)->numPackets = (num))
#define ObjectAddNumPackets(p_obj, num) ((p_obj)->numPackets += (num))

#define ObjectGetCenterSector(p_obj)  \
            ObjMoveGetCenterSector(&(p_obj)->objMove)

#define ObjectGetAreaCeilingSector(p_obj)  \
            ObjMoveGetAreaCeilingSector(&(p_obj)->objMove)

#define ObjectGetType(p_obj)   ((p_obj)->objectType)
#define ObjectGetBasicType(p_obj) \
            (((p_obj)->objectType) & (OBJECT_TYPE_BASIC_MASK))

#define ObjectGetAreaSector(p_obj) \
            ObjMoveGetAreaSector(&(p_obj)->objMove)

#define ObjectGetNumAreaSectors(p_obj) \
            ObjMoveGetNumAreaSectors(&(p_obj)->objMove)

#define ObjectSetNumAreaSectors(p_obj, num) \
            ((p_obj)->objMove.numOnSectors = (num))

#define ObjectGetNthAreaSector(p_obj, n) \
            ObjMoveGetNthAreaSector(&((p_obj)->objMove), (n))

#define ObjectAccelFlat(p_obj, amount, angle) \
            ObjMoveAccelFlat((&(p_obj)->objMove), (amount), (angle))

#define ObjectSetMoveFlags(p_obj, flags) \
            ObjMoveSetFlags(&(p_obj)->objMove, \
                (ObjMoveGetFlags(&(p_obj)->objMove) | (flags)))

#define ObjectClearMoveFlags(p_obj, flags) \
            ObjMoveSetFlags(&(p_obj)->objMove, \
                (ObjMoveGetFlags(&(p_obj)->objMove) & (~(flags))))

#define ObjectGetMoveFlags(p_obj) \
            ObjMoveGetFlags(&(p_obj)->objMove)

#define ObjectStoreMoveFlags(p_obj, flags) \
            ObjMoveSetFlags(&(p_obj)->objMove, (flags))

#define ObjectWasBlocked(p_obj) \
            ObjMoveWasBlocked(&(p_obj)->objMove)

#define ObjectSetXVel(p_obj, xv)  ObjMoveSetXVel(&(p_obj)->objMove, (xv))

#define ObjectSetYVel(p_obj, yv)  ObjMoveSetYVel(&(p_obj)->objMove, (yv))

#define ObjectSetZVel(p_obj, zv)  ObjMoveSetZVel(&(p_obj)->objMove, (zv))

#define ObjectGetXVel(p_obj)  ObjMoveGetXVel(&(p_obj)->objMove)

#define ObjectGetYVel(p_obj)  ObjMoveGetYVel(&(p_obj)->objMove)

#define ObjectGetZVel(p_obj)  ObjMoveGetZVel(&(p_obj)->objMove)

#define ObjectGetExternalXVel(p_obj)  \
            ObjMoveGetExternalXVel(&(p_obj)->objMove)

#define ObjectGetExternalYVel(p_obj)  \
            ObjMoveGetExternalYVel(&(p_obj)->objMove)

#define ObjectGetExternalZVel(p_obj)  \
            ObjMoveGetExternalZVel(&(p_obj)->objMove)

#define ObjectSetXVel16(p_obj, xv)  \
            ObjMoveSetXVel(&(p_obj)->objMove, ((xv)<<16))

#define ObjectSetYVel16(p_obj, yv)  \
            ObjMoveSetYVel(&(p_obj)->objMove, ((yv)<<16))

#define ObjectSetZVel16(p_obj, zv)  \
            ObjMoveSetZVel(&(p_obj)->objMove, ((zv)<<16))

#define ObjectSetAngularVelocity(p_obj, ang, mag) \
            ObjMoveSetAngularVelocity(&(p_obj)->objMove, (ang), (mag))

#define ObjectSetMaxVelocity(p_obj, max) \
            ObjMoveSetMaxVelocity(&(p_obj)->objMove, (max))

#define ObjectSetExternalXVel(p_obj, xv)  \
            ObjMoveSetExternalXVel(&(p_obj)->objMove, (xv))

#define ObjectSetExternalYVel(p_obj, yv)  \
            ObjMoveSetExternalYVel(&(p_obj)->objMove, (yv))

#define ObjectSetExternalZVel(p_obj, zv)  \
            ObjMoveSetExternalZVel(&(p_obj)->objMove, (zv))

#define ObjectGetLastSound(p_obj) \
            ObjMoveGetLastSound(&(p_obj)->objMove)

#define ObjectSetLastSound(p_obj, sound) \
            ObjMoveSetLastSound(&(p_obj)->objMove, (sound))

#define ObjectIsAboveGround(p_obj) \
            ObjMoveIsAboveGround(&(p_obj)->objMove)

#define ObjectStopMoving(p_obj) \
            ObjMoveStopMoving(&(p_obj)->objMove)

#define ObjectGetHighestPoint(p_obj) \
            ObjMoveGetHighestPoint(&(p_obj)->objMove)

#define ObjectGetLowestPoint(p_obj) \
            ObjMoveGetLowestPoint(&(p_obj)->objMove)

#define ObjectGetClimbHeight(p_obj) \
            ObjMoveGetClimbHeight(&(p_obj)->objMove)

#define ObjectRebuildPiecewise(p_obj) \
            ObjTypeRebuildPieces((p_obj)->p_objType)

#define ObjectMarkImpassableWhenFree(p_obj) \
            ((p_obj)->attributes |= OBJECT_ATTR_MARK_IMPASSIBLE_WHEN_FREE)

#define ObjectIsMarkedMakeImpassibleWhenFree(p_obj) \
            ((p_obj)->attributes & OBJECT_ATTR_MARK_IMPASSIBLE_WHEN_FREE)

#define ObjectIsPassable(p_obj) \
            ((p_obj)->attributes & OBJECT_ATTR_PASSABLE)

#define ObjectIsGrabable(p_obj) \
            ((p_obj)->attributes & OBJECT_ATTR_GRABABLE)

#define ObjectIsPiecewise(p_obj) \
            ((p_obj)->attributes & OBJECT_ATTR_PIECE_WISE)

#define ObjectsGetFirst() (G_First3dObject)

#define ObjectsGetLast()  (G_Last3dObject)

#define ObjectGetNext(p_obj) ((p_obj)->nextObj)

#define ObjectGetPrevious(p_obj) ((p_obj)->prevObj)

#define ObjectGetColorizeTable(p_obj) \
            ((E_colorizeTable)((((p_obj)->objectType) & \
                OBJECT_TYPE_COLOR_MASK) >> \
                OBJECT_TYPE_COLOR_OFFSET))

#define ObjectGetHashPointer(p_obj) ((p_obj)->p_hash)

#define ObjectGetNextAnimationTime(p_obj)  \
            ObjTypeGetNextAnimationTime((p_obj)->p_objType)

#define ObjectSetNextAnimationTime(p_obj, time)  \
            ObjTypeGetNextAnimationTime((p_obj)->p_objType, (time))

#define ObjectDoesNotFlow(p_obj)   ObjMoveDoesNotFlow(&(p_obj)->objMove)

#define ObjectDoesFlow(p_obj)     ObjMoveDoesFlow(&(p_obj)->objMove)

#define ObjectLowGravity(p_obj)   ObjMoveLowGravity(&(p_obj)->objMove)

#define ObjectNormalGravity(p_obj)     ObjMoveNormalGravity(&(p_obj)->objMove)

#define ObjectForceNormalFriction(p_obj)   \
            ObjMoveForceNormalFriction(&(p_obj)->objMove)

#define ObjectRegularFriction(p_obj)     \
            ObjMoveRegularFriction(&(p_obj)->objMove)

T_word16 ObjectGetPictureWidth(T_3dObject *p_obj) ;

T_word16 ObjectGetPictureHeight(T_3dObject *p_obj) ;

T_byte8 *ObjectGetPicture(T_3dObject *p_obj) ;

T_3dObject *ObjectFind(T_word16 objServerId) ;

T_void ObjectSetUpSectors(T_3dObject *p_obj) ;

T_3dObject *ObjectCreate(T_void) ;

T_3dObject *ObjectCreateFake(T_void) ;

T_void ObjectAdd(T_3dObject *p_obj) ;

T_void ObjectRemove(T_3dObject *p_obj) ;

T_void ObjectDestroy(T_3dObject *p_obj) ;

T_void ObjectDeclareStatic(
           T_3dObject *p_obj,
           T_sword16 mapX,
           T_sword16 mapY) ;

//T_void ObjectSetPictureDirectly(T_3dObject *p_obj, T_byte8 *p_pic) ;

T_void ObjectTeleport(T_3dObject *p_obj, T_sword16 x, T_sword16 y) ;

T_void ObjectTeleportAlways(T_3dObject *p_obj, T_sword16 x, T_sword16 y) ;

T_void ObjectMakeImpassable(T_3dObject *p_obj) ;

T_void ObjectMakePassable(T_3dObject *p_obj) ;

T_void ObjectMakePassable(T_3dObject *p_obj) ;

T_sword16 ObjectGetMiddleHeight(T_3dObject *p_obj) ;

T_word16 ObjectGetTypeRadius(T_word16 type) ;

T_bitmap *ObjectGetBitmap(T_3dObject *p_obj) ;

T_void ObjectsRemoveExtra(T_void) ;

T_void ObjectsUpdateMovement(T_word32 delta) ;

T_void ObjectSetType(T_3dObject *p_obj, T_word16 type) ;

T_void ObjectSetTypeSimple(T_3dObject *p_obj, T_word16 type) ;

T_void ObjectsUpdateAnimation(T_word32 currentTime) ;

T_void ObjectUpdateAnimation(T_3dObject *p_obj, T_word32 currentTime) ;

T_void ObjectSetStance(T_3dObject *p_obj, T_word16 stance) ;

T_void ObjectDoToAll(T_objectDoToAllCallback p_callback, T_word32 data) ;

T_void ObjectsDoToAllAtXY(
           T_sword16 x,
           T_sword16 y,
           T_objectDoToAllCallback p_callback,
           T_word32 data) ;

T_void ObjectsDoToAllAtXYRadius(
           T_sword16 x,
           T_sword16 y,
           T_word16 radius,
           T_objectDoToAllCallback p_callback,
           T_word32 data);

T_void ObjectsDoToAllAtXYZRadius(
           T_sword16 x,
           T_sword16 y,
           T_sword16 z,
           T_word16 radius,
           T_objectDoToAllCallback p_callback,
           T_word32 data) ;

T_void ObjectSetAngle(T_3dObject *p_obj, T_word16 angle) ;

E_Boolean ObjectCheckIfCollide(
              T_3dObject *p_obj,
              T_sword32 x,
              T_sword32 y,
              T_sword32 z) ;

E_Boolean ObjectCheckCollide(
              T_3dObject *p_obj,
              T_sword16 x,
              T_sword16 y,
              T_sword16 height) ;

E_Boolean ObjectIsAtXY(T_3dObject *p_obj, T_sword16 x, T_sword16 y) ;

T_void ObjectGetForwardPosition(
           T_3dObject *p_obj,
           T_word16 dist,
           T_sword32 *p_x,
           T_sword32 *p_y) ;

T_void ObjectGetAngularPosition(
           T_3dObject *p_obj,
           T_word16 angle,
           T_sword16 dist,
           T_sword32 *p_x,
           T_sword32 *p_y) ;

T_void ObjectsMakeTemporarilyPassableAtXYRadius(
           T_sword16 x,
           T_sword16 y,
           T_word16 radius,
           T_sword16 zBottom,
           T_sword16 zTop) ;

T_word32 ObjectsCountType(T_word16 objectType) ;

T_word32 ObjectsCountBasicType(T_word16 objectType) ;

T_3dObject *ObjectDuplicate(T_3dObject *p_obj) ;

T_void ObjectSetBodyPartType(
           T_3dObject *p_obj,
           T_bodyPartLocation location,
           T_word16 objType) ;

T_word16 ObjectGetBodyPartType(
             T_3dObject *p_obj,
             T_bodyPartLocation location) ;

T_void ObjectChainingOff(T_void) ;

T_void ObjectChainingOn(T_void) ;

T_void ObjectsInitialize(T_void) ;

T_void ObjectsFinish(T_void) ;

T_void *ObjectAllocExtraData(T_3dObject *p_obj, T_word32 sizeData) ;

T_void ObjectFreeExtraData(T_3dObject *p_obj) ;

E_Boolean ObjectIsBeingCrushed(T_3dObject *p_obj) ;

T_void ObjectMarkForDestroy(T_3dObject *p_obj) ;

T_word32 ObjectsGetNumMarkedForDestroy(T_void) ;

#ifndef NDEBUG

T_void ObjectPrint(FILE *fp, T_3dObject *p_obj) ;
#else
#define ObjectPrint(fp, p_obj)
#endif

T_void ObjectRemoveScript(T_3dObject *p_obj) ;

T_void ObjectsUpdateMovementForFake(T_word32 delta) ;

T_void ObjectAddWithoutHistory(T_3dObject *p_obj) ;

#define ObjectSetTarget(p_obj, targetID) CreatureSetTarget(p_obj, targetID)
#endif

#define ObjectIsPlayer(p_obj)   (ObjectIsPiecewise(p_obj)?TRUE:FALSE)
#define ObjectIsPlayerHead(p_obj)   ((ObjectIsPiecewise(p_obj) && \
                                 (!ObjectIsBodyPart(p_obj)))?TRUE:FALSE)
#define ObjectIsCreature(p_obj)  ((ObjectGetExtraData(p_obj)==NULL)?FALSE:TRUE)

#define ObjectGetLastSteppedOnSector(p_obj)   ((p_obj)->lastSectorSteppedOn)
#define ObjectSetLastSteppedOnSector(p_obj, last)   \
             ((p_obj)->lastSectorSteppedOn = (last))

#define ObjectMakeStealthy(p_obj)    \
            ((p_obj)->attributes |= OBJECT_ATTR_STEALTHY)
#define ObjectMakeNotStealthy(p_obj) \
            ((p_obj)->attributes &= (~OBJECT_ATTR_STEALTHY))
#define ObjectIsStealthy(p_obj)      \
            ((p_obj)->attributes & OBJECT_ATTR_STEALTHY)
#define ObjectIsValid(p_obj)         \
            ((strcmp((p_obj)->tag, "Obj") == 0)?TRUE:FALSE)

T_void ObjectsCreateSaveBlock(T_void) ;

T_void ObjectsResetIds(T_void) ;

T_word32 ObjectGetNextId(T_void) ;

T_void ObjectAddAttributesToPiecewise(T_3dObject *p_obj, T_word16 attr) ;

T_void ObjectRemoveAttributesFromPiecewise(T_3dObject *p_obj, T_word16 attr) ;

T_word16 ObjectGetWeight(T_3dObject *p_obj) ;

T_word16 ObjectGetValue(T_3dObject *p_obj) ;

T_void ObjectDrawFrontScaled(
           T_3dObject *p_obj,
           T_sword16 x,
           T_sword16 y,
           T_word16 width,
           T_word16 height) ;

T_void ObjectUpdateCollisionLink(T_3dObject *p_obj) ;

T_void ObjectUnlinkCollisionLink(T_3dObject *p_obj) ;

E_Boolean ObjectCheckCollideAny(
              T_3dObject *p_obj,
              T_sword16 x,
              T_sword16 y,
              T_sword16 height) ;

T_3dObject *ObjectFindBodyPartHead(T_3dObject *p_part) ;

T_void ObjectsDoToAll(T_objectDoToAllCallback p_callback, T_word32 data);

T_void ObjectsUnload(T_void);

/****************************************************************************/
/*    END OF FILE:  OBJECT.H                                                */
/****************************************************************************/
