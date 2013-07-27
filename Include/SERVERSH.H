#ifndef _SERVERSH_H_
#define _SERVERSH_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

typedef struct {
    T_word16 ownerID ;
    T_word16 damage ;
    T_byte8 type ;
} T_damageObjInfo ;

extern T_sword32 G_sourceX ;
extern T_sword32 G_sourceY ;
extern T_sword32 G_sourceZ ;

E_Boolean ServerDamageObjectXYZ(
              T_3dObject *p_obj,
              T_word32 data) ;

T_word16 ServerLockDoorsInArea(
           T_sword32 x,
           T_sword32 y,
           T_word16 radius) ;

T_word16 ServerUnlockDoorsInArea(
           T_sword32 x,
           T_sword32 y,
           T_word16 radius) ;

#endif // _SERVERSH_H_
