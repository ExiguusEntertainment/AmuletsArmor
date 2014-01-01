/****************************************************************************/
/*    FILE:  OBJMOVE.H                                                     */
/****************************************************************************/
#ifndef _OBJMOVE_H_
#define _OBJMOVE_H_

#include "GENERAL.H"

#define FAST_OBJMOVE        /* define flag that turns on faster objmove */

#define MAX_OBJECT_SECTORS 20
#define OBJMOVE_NORMAL_FRICTION        12

#define OBJMOVE_FLAG_BLOCKED               0x0001   /* -------- -------1 */
#define OBJMOVE_FLAG_PLEASE_UPDATE         0x0002   /* -------- ------1- */
#define OBJMOVE_FLAG_IGNORE_FRICTION       0x0004   /* -------- -----1-- */
#define OBJMOVE_FLAG_IGNORE_GRAVITY        0x0008   /* -------- ----1--- */
#define OBJMOVE_FLAG_FORCE_NORMAL_FRICTION 0x0010   /* -------- ---1---- */
#define OBJMOVE_FLAG_IGNORE_MAX_VELOCITY   0x0020   /* -------- --1----- */
#define OBJMOVE_FLAG_DO_NOT_CLIMB          0x0040   /* -------- -1------ */
#define OBJMOVE_FLAG_MOVED                 0x0080   /* -------- 1------- */
#define OBJMOVE_FLAG_DO_NOT_SINK           0x0100   /* -------1 -------- */
#define OBJMOVE_FLAG_STICK_TO_CEILING      0x0200   /* ------1- -------- */
#define OBJMOVE_FLAG_RAISED                0x0400   /* -----1-- -------- */
#define OBJMOVE_FLAG_BOUNCES               0x0800   /* ----1--- -------- */
#define OBJMOVE_FLAG_HAS_EVER_MOVED        0x1000   /* ---1---- -------- */
#define OBJMOVE_FLAG_IGNORE_Z_UPDATES      0x2000   /* --1----- -------- */
#define OBJMOVE_FLAG_DOES_NOT_FLOW         0x4000   /* -1------ -------- */
#define OBJMOVE_FLAG_LOW_GRAVITY           0x8000   /* 1------- -------- */
#define OBJMOVE_FLAG_FAST_MOVEMENT			0x0000 // not used

typedef struct
{
	T_sword32	X;
	T_sword32	Y;
	T_sword32	Z;

	T_sword32	XV;
	T_sword32	YV;
	T_sword32	ZV;

	T_sword32	XVExternal;
	T_sword32	YVExternal;
	T_sword32	ZVExternal;

	T_word16	Radius;
	T_sword16	Height;
	T_word16 	Angle;
	T_word16 	Flags;

	T_word16	AreaSector;
	T_word16	AreaCeilingSector;
	T_word16    CenterSector;
    T_word16    numOnSectors ;
	T_word16    OnSectors[MAX_OBJECT_SECTORS] ;

    T_word16    maxVelocity ;  /* LES */

    T_word16    lastSound ;

    T_sword32   lowestPoint ;           /* Based on all that is below */
                                        /* this is the lowest height allowed. */
    T_sword32   highestPoint ;          /* Based on all that is above */
                                        /* this is the highest height allowed. */
    T_word16    climbHeight ;           /* How big of a step can I climb? */
} T_objMoveStruct;

/*
#define ObjMoveGetFlags(p_objM) ((p_objM)->Flags)

#define ObjMoveSetFlags(p_objM, flags) ((p_objM)->Flags = (flags))
*/

#define ObjMoveClearMovedFlag(p_objM) \
            ((p_objM)->Flags &= (~OBJMOVE_FLAG_MOVED))

#define ObjMoveClearBlockedFlag(p_objM) \
            ((p_objM)->Flags &= (~OBJMOVE_FLAG_BLOCKED))

#define ObjMoveHasEverMoved(p_objM) \
            ((p_objM)->Flags &= (~OBJMOVE_FLAG_HAS_EVER_MOVED))

#define ObjMoveCheckMovedFlag(p_objM) \
            ((p_objM)->Flags & OBJMOVE_FLAG_MOVED)

#define ObjMoveWasBlocked(p_objM) \
            ((p_objM)->Flags & OBJMOVE_FLAG_BLOCKED)

#define ObjMoveSetXVel(p_objM, xv) ( \
             ((p_objM)->XV = (xv)) , \
             (p_objM)->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE \
        )

#define ObjMoveSetYVel(p_objM, yv) ( \
             ((p_objM)->YV = (yv)) , \
             (p_objM)->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE \
        )

#define ObjMoveSetZVel(p_objM, zv) ( \
             ((p_objM)->ZV = (zv)) , \
             (p_objM)->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE \
        )
#define ObjMoveSetClimbHeight(oms, height)  \
            (((oms)->climbHeight) = (height))


#define ObjMoveForceUpdate(p_objM) \
             ((p_objM)->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE)

#define ObjMoveGetLastSound(p_objM) \
             ((p_objM)->lastSound)

#define ObjMoveGetHighestPoint(p_objM) \
             ((p_objM)->highestPoint)

#define ObjMoveGetLowestPoint(p_objM) \
             ((p_objM)->lowestPoint)

#define ObjMoveSetLastSound(p_objM, sound) \
             (((p_objM)->lastSound) = (sound))
#define ObjMoveGetClimbHeight(oms)  ((oms)->climbHeight)

T_void ObjMoveUpdate (T_objMoveStruct *p_obj, T_sword32 delta) ;

T_void ObjMoveInit (T_objMoveStruct *ObjMoveStruct);

T_void ObjMoveSetXYZ (T_objMoveStruct *ObjMoveStruct,
						T_sword32 newx,
						T_sword32 newy,
						T_sword32 newz);
T_void ObjMoveGetXYZ (T_objMoveStruct *ObjMoveStruct,
						T_sword32 *xval,
						T_sword32 *yval,
						T_sword32 *zval);
T_void ObjMoveAccelXYZ    (T_objMoveStruct *ObjMoveStruct,
							 T_sword32 XACC,
							 T_sword32 YACC,
							 T_sword32 ZACC);
T_void ObjMoveAccelFlat   (T_objMoveStruct *ObjMoveStruct,
							 T_sword32 amount,
							 T_sword32 angle);

E_Boolean ObjMoveIsAboveGround (T_objMoveStruct *ObjMoveStruct);
T_void ObjMoveStopMoving(T_objMoveStruct *ObjMoveStruct) ;

T_void ObjMoveAddExternalVelocity(
           T_objMoveStruct *ObjMoveStruct,
           T_sword32 dVX,
           T_sword32 dVY,
           T_sword32 dVZ) ;

T_void ObjMoveSetUpSectors(T_objMoveStruct *ObjMoveStruct) ;

T_void ObjMoveSetAngularVelocity(
           T_objMoveStruct *p_objM,
           T_word16 angle,
           T_sword16 magnitude);

T_void ObjMoveUpdateZVel(
           T_objMoveStruct *ObjMoveStruct,
           T_word32 delta) ;


#ifdef FAST_OBJMOVE
#define ObjMoveSetX(ObjMoveStruct, newx)  ((ObjMoveStruct)->X = (newx))
#define ObjMoveSetY(ObjMoveStruct, newy)  ((ObjMoveStruct)->Y = (newy))
#define ObjMoveSetZ(ObjMoveStruct, newz)  ((ObjMoveStruct)->Z = (newz))
#define ObjMoveSetFlags(oms, flags) ((oms)->Flags = (flags))
#define ObjMoveSetAngle(oms, angle) ((oms)->Angle = (angle))
#define ObjMoveSetHeight(oms, height) (((oms)->Height = (height)), \
                                       ((oms)->climbHeight = (height>>2)))
#define ObjMoveSetRadius(oms, radius) ((oms)->Radius = (radius))
#define ObjMoveSetMaxVelocity(oms, maxVel) ((oms)->maxVelocity = (maxVel))

#define ObjMoveAddAngle(oms, angle) ((oms)->Angle += (angle))
#define ObjMoveAddX(oms, addx) ((oms)->X += (addx))
#define ObjMoveAddY(oms, addy) ((oms)->Y += (addy))
#define ObjMoveAddZ(oms, addz) ((oms)->Z += (addz))
#define ObjMoveSetExternalXVel(oms, xv)        (((oms)->XVExternal) = (xv))
#define ObjMoveSetExternalYVel(oms, yv)        (((oms)->YVExternal) = (yv))
#define ObjMoveSetExternalZVel(oms, zv)        (((oms)->ZVExternal) = (zv))

#define ObjMoveGetX(oms)           ((oms)->X)
#define ObjMoveGetY(oms)           ((oms)->Y)
#define ObjMoveGetZ(oms)           ((oms)->Z)
#define ObjMoveGetAngle(oms)       ((oms)->Angle)
#define ObjMoveGetHeight(oms)      ((oms)->Height)
#define ObjMoveGetRadius(oms)      ((oms)->Radius)
#define ObjMoveGetFlags(oms)       ((oms)->Flags)
#define ObjMoveGetMaxVelocity(oms) ((oms)->maxVelocity)
#define ObjMoveGetXVel(oms)        ((oms)->XV)
#define ObjMoveGetYVel(oms)        ((oms)->YV)
#define ObjMoveGetZVel(oms)        ((oms)->ZV)
#define ObjMoveGetExternalXVel(oms)        ((oms)->XVExternal)
#define ObjMoveGetExternalYVel(oms)        ((oms)->YVExternal)
#define ObjMoveGetExternalZVel(oms)        ((oms)->ZVExternal)
#define ObjMoveGetAreaSector(oms)  ((oms)->AreaSector)
#define ObjMoveGetAreaCeilingSector(oms)  ((oms)->AreaCeilingSector)
#define ObjMoveGetCenterSector(oms) ((oms)->CenterSector)
#define ObjMoveGetFriction(oms)     \
           (G_3dSectorInfoArray[(oms)->AreaSector].friction)
#define ObjMoveGetNumAreaSectors(oms)     ((oms)->numOnSectors)
#define ObjMoveGetNthAreaSector(oms, n)    ((oms)->OnSectors[(n)])

#else
T_void ObjMoveSetX   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 newx);
T_void ObjMoveSetY   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 newy);
T_void ObjMoveSetZ   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 newz);
T_void ObjMoveSetFlags (T_objMoveStruct *ObjMoveStruct,
						  T_word16 newflags);
T_void ObjMoveSetAngle	(T_objMoveStruct *ObjMoveStruct, T_word16 newAngle);
T_void ObjMoveSetHeight	(T_objMoveStruct *ObjMoveStruct, T_sword32 newHeight);
T_void ObjMoveSetRadius (T_objMoveStruct *ObjMoveStruct, T_word16 newRadius);
T_void ObjMoveSetMaxVelocity(
           T_objMoveStruct *ObjMoveStruct,
           T_word16 maxVelocity) ;

T_void ObjMoveAddAngle  (T_objMoveStruct *ObjMoveStruct, T_word16 addAngle);
T_void ObjMoveAddX   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addx);
T_void ObjMoveAddY   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addy);
T_void ObjMoveAddZ   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addz);

T_sword32 ObjMoveGetX      (T_objMoveStruct *ObjMoveStruct);
T_sword32 ObjMoveGetY      (T_objMoveStruct *ObjMoveStruct);
T_sword32 ObjMoveGetZ      (T_objMoveStruct *ObjMoveStruct);
T_word16  ObjMoveGetAngle  (T_objMoveStruct *ObjMoveStruct);
T_word16  ObjMoveGetHeight (T_objMoveStruct *ObjMoveStruct);
T_word16  ObjMoveGetRadius (T_objMoveStruct *ObjMoveStruct);
T_word16   ObjMoveGetFlags  (T_objMoveStruct *ObjMoveStruct);
T_word16 ObjMoveGetMaxVelocity(T_objMoveStruct *ObjMoveStruct) ;
T_sword32 ObjMoveGetXVel (T_objMoveStruct *ObjMoveStruct);
T_sword32 ObjMoveGetYVel (T_objMoveStruct *ObjMoveStruct);
T_sword32 ObjMoveGetZVel (T_objMoveStruct *ObjMoveStruct);

T_word16 ObjMoveGetAreaSector (T_objMoveStruct *ObjMoveStruct) ;
T_word16 ObjMoveGetAreaCeilingSector (T_objMoveStruct *ObjMoveStruct) ;
T_word16 ObjMoveGetCenterSector (T_objMoveStruct *ObjMoveStruct) ;
T_word16 ObjMoveGetFriction(T_objMoveStruct *ObjMoveStruct) ;
T_word16 ObjMoveGetNumAreaSectors(T_objMoveStruct *ObjMoveStruct) ;
T_word16 ObjMoveGetNthAreaSector(
             T_objMoveStruct *ObjMoveStruct,
             T_word16 n) ;
#endif
T_void ObjMoveSetMovedFlag(T_objMoveStruct *ObjMoveStruct) ;


#define ObjMoveDoesNotFlow(oms) ((oms)->Flags |= OBJMOVE_FLAG_DOES_NOT_FLOW)

#define ObjMoveDoesFlow(oms) ((oms)->Flags &= (~OBJMOVE_FLAG_DOES_NOT_FLOW))

#define ObjMoveLowGravity(oms) ((oms)->Flags |= OBJMOVE_FLAG_LOW_GRAVITY)

#define ObjMoveNormalGravity(oms) ((oms)->Flags &= (~OBJMOVE_FLAG_LOW_GRAVITY))


#define ObjMoveForceNormalFriction(oms) \
            ((oms)->Flags |= OBJMOVE_FLAG_FORCE_NORMAL_FRICTION)

#define ObjMoveRegularFriction(oms) \
            ((oms)->Flags &= (~OBJMOVE_FLAG_FORCE_NORMAL_FRICTION))

#ifndef NDEBUG

T_void ObjMovePrint(FILE *fp, T_objMoveStruct *p_om) ;
#else
#define ObjMovePrint(fp, p_om)
#endif

#endif

/****************************************************************************/
/*    END OF FILE: OBJMOVE.H                                                */
/****************************************************************************/

