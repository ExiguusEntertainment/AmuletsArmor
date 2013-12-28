/****************************************************************************/
/*    FILE:  MAP.H                                                          */
/****************************************************************************/
#ifndef _MAP_H_
#define _MAP_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

typedef T_void *T_mapHandle ;
#define MAP_HANDLE_BAD NULL

T_void MapInitialize(T_void) ;

T_void MapFinish(T_void) ;

T_void MapLoad(T_word32 mapNumber) ;

T_void MapSave(T_word32 mapNumber) ;

T_void MapUnload(T_void) ;

T_void MapSetBackdrop(T_byte8 *p_backdrop) ;

//T_word16 MapGetForwardWallActivation(T_3dObject *p_obj) ;

T_void MapSetWallBitmapTextureXY(
           T_word16 wallNum,
           T_word16 side,
           T_sword16 offX,
           T_sword16 offY) ;

T_word16 MapGetSectorAction(T_word16 sector) ;

T_void MapSetFloorHeight(T_word16 sector, T_sword16 height) ;

T_sword16 MapGetFloorHeight(T_word16 sector) ;

T_sword16 MapGetWalkingFloorHeight(T_objMoveStruct *p_objMove, T_word16 sector);

T_void MapSetCeilingHeight(T_word16 sector, T_sword16 height) ;

T_sword16 MapGetCeilingHeight(T_word16 sector) ;

T_void MapSetSectorLighting(T_word16 sector, T_byte8 lightLevel) ;

T_byte8 MapGetSectorLighting(T_word16 sector) ;

E_Boolean MapCheckCrush(T_word16 sector, T_sword16 newHeight) ;

T_void MapGetStartLocation(
           T_word16 num,
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_word16 *p_angle) ;


T_void MapOpenForwardWall(
           T_3dObject *p_obj,
           E_Boolean checkForItemRequired) ;

T_void MapGetNextMapAndPlace(
           T_word16 sector,
           T_word16 *p_nextMap,
           T_word16 *p_nextMapPlace) ;

T_void MapSetOutsideLighting(T_byte8 outsideLight) ;

T_byte8 MapGetOutsideLighting(T_void) ;

T_void MapUpdateLighting(T_void) ;

/* Need to be implemented: */
T_void MapSetUpperTextureForSide(T_word16 sideNum, T_byte8 *p_textureName) ;

T_void MapSetLowerTextureForSide(T_word16 sideNum, T_byte8 *p_textureName) ;

T_void MapSetMainTextureForSide(T_word16 sideNum, T_byte8 *p_textureName) ;

T_void MapSetSideState(T_word16 sideNum, T_word16 sideState) ;

T_void MapSetWallState(T_word16 wallNum, T_word16 wallState) ;

T_void MapSetSectorState(T_word16 sectorNum, T_word16 sectorState) ;

T_void MapSetFloorTextureForSector(
           T_word16 sectorNum,
           T_byte8 *p_textureName) ;

T_void MapSetCeilingTextureForSector(
           T_word16 sectorNum,
           T_byte8 *p_textureName) ;

E_Boolean MapExist(T_word32 num) ;

T_byte8 *MapGetUpperTextureName(T_word16 sideNum) ;

T_byte8 *MapGetLowerTextureName(T_word16 sideNum) ;

T_byte8 *MapGetMainTextureName(T_word16 sideNum) ;

T_byte8 *MapGetFloorTextureName(T_word16 sectorNum) ;

T_byte8 *MapGetCeilingTextureName(T_word16 sectorNum) ;

T_word16 MapGetTextureXOffset(T_word16 sideNum) ;

T_word16 MapGetTextureYOffset(T_word16 sideNum) ;

T_sword16 MapGetWalkingFloorHeightAtXY(
        T_objMoveStruct *p_objMove,
        T_sword16 x,
        T_sword16 y);

T_sectorType MapGetSectorType(T_word16 sectorNum) ;

E_Boolean MapGetWallDamage(
              T_3dObject *p_obj,
              T_word16 *p_damageAmount,
              T_byte8 *p_damageType) ;

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
T_void MapShiftTextureRight(T_sword16 amount) ;

T_void MapShiftTextureLeft(T_sword16 amount) ;

T_void MapShiftTextureUp(T_sword16 amount) ;

T_void MapShiftTextureDown(T_sword16 amount) ;

T_void MapShiftTextureCloseFile(T_void) ;

T_void MapShiftTextureOpenFile(T_void) ;
#endif

typedef T_byte8 E_wallActivation ;
#define WALL_ACTIVATION_NONE             0
#define WALL_ACTIVATION_DOOR             1
#define WALL_ACTIVATION_SCRIPT           2
#define WALL_ACTIVATION_UNKNOWN          3

T_void MapGetForwardWallActivationType(
           T_3dObject *p_obj,
           E_wallActivation *p_type,
           T_word16 *p_data) ;

T_void MapForceOpenForwardWall(T_3dObject *p_obj) ;

T_void MapSetWallTexture(T_word16 sideNum, T_byte8 *p_textureName) ;

T_void MapSetDayOffset(T_word32 offset) ;

T_word32 MapGetDayOffset(T_void) ;

T_void MapOutputTimeOfDay(T_void) ;

#define MAP_SPECIAL_NONE                0
#define MAP_SPECIAL_DAY_NIGHT_CREATURES 1
#define MAP_SPECIAL_UNKNOWN             2

T_word32 MapGetMapSpecial(T_void) ;

E_Boolean MapIsDay(T_void) ;

E_Boolean MapCheckCrushByFloor(T_word16 sector, T_sword16 newHeight);

E_Boolean MapCheckCrushByCeiling(T_word16 sector, T_sword16 newHeight);

#endif // _MAP_H_

/****************************************************************************/
/*    END OF FILE:  MAP.H                                                   */
/****************************************************************************/
