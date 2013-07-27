/****************************************************************************/
/*    FILE:  OBJTYPE.H                                                      */
/****************************************************************************/
#ifndef _OBJTYPE_H_
#define _OBJTYPE_H_

#include "GENERAL.H"

/*---------------------------------------------------------------------------
 * Constants:
 *--------------------------------------------------------------------------*/
#define OBJECT_TYPE_INSTANCE_BAD NULL

/*---------------------------------------------------------------------------
 * Types:
 *--------------------------------------------------------------------------*/
typedef enum {
    ORIENTATION_NORMAL,
    ORIENTATION_REVERSE,
    ORIENTATION_UNKNOWN
} T_orientation ;

//typedef void *T_3dObject; /* Unknown here */

struct T_3dObject_;
typedef T_void *T_objTypeInstance ;

/*---------------------------------------------------------------------------
 * Prototypes:
 *--------------------------------------------------------------------------*/

T_objTypeInstance ObjTypeCreate(T_word16 objTypeNum, struct T_3dObject_ *p_obj) ;

T_void ObjTypeDestroy(T_objTypeInstance objTypeInst) ;

E_Boolean ObjTypeAnimate(
              T_objTypeInstance objTypeInst,
              T_word32 currentTime) ;

T_word16 ObjTypeGetRadius(T_objTypeInstance objTypeInst) ;

T_word16 ObjTypeGetAttributes(T_objTypeInstance objTypeInst) ;

T_void *ObjTypeGetPicture(
           T_objTypeInstance objTypeInst,
           T_word16 angle,
           T_orientation *p_orient) ;

T_void ObjTypesSetResolution(T_word16 resolution) ;

T_word16 ObjTypesGetResolution(T_void) ;

T_void ObjTypeSetStance(T_objTypeInstance objTypeInst, T_word16 stance) ;

T_word16 ObjTypeGetStance(T_objTypeInstance objTypeInst) ;

T_word32 ObjTypeGetScript(T_objTypeInstance objTypeInst) ;

T_void ObjTypeSetScript(T_objTypeInstance objTypeInst, T_word32 script) ;

T_word16 ObjTypeGetHealth (T_objTypeInstance objTypeInst);

T_word16 ObjTypeGetWeight (T_objTypeInstance objTypeInst);

T_word16 ObjTypeGetValue (T_objTypeInstance objTypeInst);

T_word16 ObjTypeGetMoveFlags (T_objTypeInstance objTypeInst);

T_word16 ObjTypeGetHeight(T_objTypeInstance objTypeInst) ;

T_word16 ObjTypeGetFrame(T_objTypeInstance objTypeInst) ;

T_word32 ObjTypeGetNextAnimationTime(T_objTypeInstance objTypeInst) ;

T_void ObjTypeSetNextAnimationTime(
             T_objTypeInstance objTypeInst,
             T_word32 time) ;

T_word16 ObjTypeGetAnimData(T_objTypeInstance objTypeInst) ;

T_void ObjTypeSetAnimData(T_objTypeInstance objTypeInst, T_word16 data) ;

#ifndef NDEBUG
T_void ObjTypePrint(FILE *fp, T_objTypeInstance objTypeInst) ;
#else
#define ObjTypePrint(fp, obj) ((T_void)0)
#endif

E_Boolean ObjTypeIsActive(T_objTypeInstance objTypeInst) ;

T_void *ObjTypeGetFrontFirstPicture(T_objTypeInstance objTypeInst) ;

T_void ObjTypeDeclareSomewhatLowOnMemory(T_void) ;

E_Boolean ObjTypeIsLowPiecewiseRes(T_void) ;

#endif

/****************************************************************************/
/*    END OF FILE:  OBJTYPE.H                                               */
/****************************************************************************/
