/****************************************************************************/
/*    FILE:  PLAYER.H                                                       */
/****************************************************************************/
#include "GENERAL.H"
#include "VIEWFILE.H"

#define PlayerGetX16()    (PlayerGetX()>>16)
#define PlayerGetY16()    (PlayerGetY()>>16)
#define PlayerGetZ16()    (PlayerGetZ()>>16)
#define PlayerSetX16(x)   (PlayerSetX((x) << 16))
#define PlayerSetY16(y)   (PlayerSetY((y) << 16))
#define PlayerSetZ16(z)   (PlayerSetZ((z) << 16))

T_void PlayerInitFirst(T_void) ;
T_void PlayerInit(T_3dObject *p_obj) ;
T_void PlayerFinish(T_void) ;

T_sword32 PlayerGetX(T_void) ;
T_sword32 PlayerGetY(T_void) ;
T_sword32 PlayerGetZ(T_void) ;
T_word16 PlayerGetAngle(T_void) ;
T_word16 PlayerGetAreaSector(T_void) ;
T_word16 PlayerGetCenterSector(T_void) ;

T_void PlayerSetX(T_sword32 x) ;
T_void PlayerSetY(T_sword32 y) ;
T_void PlayerSetZ(T_sword32 z) ;
T_void PlayerSetAngle(T_word16 angle) ;

T_void PlayerUpdatePosInfo(T_void) ;

E_Boolean PlayerIsAboveGround(T_void) ;

T_void PlayerJump(T_word16 jumpPower) ;
T_void PlayerSetMaxVelocity(T_sword32 maxVelocity) ;
T_word16 PlayerGetTravelDistance(T_void) ;
T_void PlayerStopMoving(T_void) ;
T_void PlayerAccelXYZ(T_sword32 accelX, T_sword32 accelY, T_sword32 accelZ) ;
T_word16 PlayerGetFriction(T_void) ;
T_word16 PlayerGetNumAreaSectors(T_void) ;
T_word16 PlayerGetNthAreaSector(T_word16 n) ;
T_void PlayerAccelDirection(T_word16 direction, T_sword16 accel) ;
T_void PlayerSetCameraView(T_void) ;
T_void PlayerTeleport(T_sword16 x, T_sword16 y, T_word16 angle) ;
T_void PlayerTeleportAlways(T_sword16 x, T_sword16 y, T_word16 angle) ;
T_void PlayerAddExternalVelocity(T_sword32 dx, T_sword32 dy, T_sword32 dz) ;

T_3dObject *PlayerGetObject(T_void) ;

T_word16 PlayerGetStance(T_void) ;
T_void PlayerSetStance(T_word16 stance) ;

T_void PlayerSetBodyPart(
           T_bodyPartLocation location,
           T_word16 newPart) ;

T_word16 PlayerGetBodyPart(T_bodyPartLocation location) ;

T_void PlayerDraw(T_word16 x, T_word16 y) ;

#define PLAYER_MOVE_NONE      0
#define PLAYER_MOVE_FULL      0x100
#define PLAYER_MOVE_DOUBLE    0x200
#define PLAYER_MOVE_MAXIMUM   0x8000

T_void PlayerTurnLeft(T_word16 fraction) ;
T_void PlayerTurnRight(T_word16 fraction) ;
T_void PlayerSlideLeft(T_word16 fraction) ;
T_void PlayerSlideRight(T_word16 fraction) ;
T_void PlayerMoveForward(T_word16 fraction) ;
T_void PlayerMoveBackward(T_word16 fraction) ;
T_void PlayerMoveNone(T_void) ;
T_word16 PlayerGetLastSector(T_void) ;
T_void PlayerSetLastSector(T_word16 lastSector) ;

T_void PlayerJumpForward(T_word16 distance) ;

T_void PlayerSetFakeMode(T_void) ;

T_void PlayerSetRealMode(T_void) ;

E_Boolean PlayerInFakeMode(T_void) ;

T_void PlayerFakeOverwriteCurrent(T_void) ;

T_void PlayerMakeSoundLocal(T_word16 soundNum) ;

T_void PlayerMakeSoundGlobal(T_word16 soundNum, T_word16 radius) ;

E_Boolean PlayerIsStealthy(T_void) ;

E_Boolean PlayerJustTeleported(T_void) ;

T_void PlayerTeleported(T_void) ;

T_void PlayerUpdate(T_word32 delta);

/****************************************************************************/
/*    END OF FILE: PLAYER.H                                                 */
/****************************************************************************/

