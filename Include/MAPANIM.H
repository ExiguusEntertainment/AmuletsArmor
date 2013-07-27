/****************************************************************************/
/*    FILE:  MAPANIM.H                                                      */
/****************************************************************************/
#ifndef _MAPANIM_H_
#define _MAPANIM_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

typedef T_void *T_mapAnimation ;
#define MAP_ANIMATION_BAD NULL

T_mapAnimation MapAnimateLoad(T_word32 number) ;

T_void MapAnimateUnload(T_mapAnimation mapAnimation) ;

T_void MapAnimateUpdate(T_mapAnimation mapAnimation) ;

T_void MapAnimateStartSide(
           T_mapAnimation mapAnimation,
           T_word16 sideNum,
           T_word16 stateNumber) ;

T_void MapAnimateStartWall(
           T_mapAnimation mapAnimation,
           T_word16 wallNum,
           T_word16 stateNumber) ;

T_void MapAnimateStartSector(
           T_mapAnimation mapAnimation,
           T_word16 sectorNum,
           T_word16 stateNumber) ;

E_Boolean MapAnimateGetWallDamage(
              T_mapAnimation mapAnimation,
              T_3dObject *p_obj,
              T_word16 *p_damageAmount,
              T_byte8 *p_damageType) ;

#endif

/****************************************************************************/
/*    END OF FILE:  MAPANIM.H                                               */
/****************************************************************************/
