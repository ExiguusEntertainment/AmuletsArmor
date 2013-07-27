/****************************************************************************/
/*    FILE:  CRELOGIC.H                                                      */
/****************************************************************************/
#ifndef _CRELOGIC_H_
#define _CRELOGIC_H_

#include "3D_VIEW.H"
#include "GENERAL.H"

typedef T_void *T_creaturesHandle ;
#define CREATURES_HANDLE_BAD NULL

T_void CreaturesInitialize(T_void) ;

T_void CreaturesFinish(T_void) ;

T_void CreatureTakeDamage(
           T_3dObject *p_obj,
           T_word32 damage,
           T_word16 type,
           T_word16 ownerID) ;

#ifdef SERVER_ONLY
T_3dObject *CreatureLookForPlayer(T_3dObject *p_obj) ;
T_void CreaturePlayerGone(T_player player) ;
#else
T_void CreaturePlayerGone(T_word16 playerId) ;
#endif

T_void CreaturesUpdate(T_void) ;

T_void CreatureAttachToObject(T_3dObject *p_obj) ;

T_void CreatureDetachFromObject(T_3dObject *p_obj) ;

E_Boolean CreatureIsMissile(T_3dObject *p_obj) ;

T_void CreatureSetTarget(T_3dObject *p_obj, T_word16 targetID) ;

T_void CreaturesHearSoundOfPlayer(T_3dObject *p_player, T_word16 distance) ;

T_word32 CreaturesKillAll(T_void) ;

T_word16 CreatureStolenFrom(T_3dObject *p_obj) ;

T_void CreatureGoSplat(
           T_3dObject *p_obj,
           T_word16 amount,
           T_word16 damageType) ;

T_void CreaturesAllOnOneTarget(T_word16 targetId);

#ifdef SERVER_ONLY
T_3dObject *CreatureLookForPlayer(T_3dObject *p_obj);
#else
T_word16 CreatureLookForPlayer(T_3dObject *p_obj);
#endif

#ifndef NDEBUG
T_void CreaturesCheck(T_void) ;
#else
#define CreaturesCheck()
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  CRELOGIC.H                                               */
/****************************************************************************/
