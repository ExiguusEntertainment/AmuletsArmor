/*-------------------------------------------------------------------------*
 * File:  PLAYER.C
 *-------------------------------------------------------------------------*/
/**
 * The player is just another object in the game system.  These routines
 * handle the movement and animation of that player object.
 *
 * @addtogroup PLAYER
 * @brief Player Object
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONFIG.H"
#include "CSYNCPCK.H"
#include "EFFECT.H"
#include "MAP.H"
#include "OBJECT.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "TICKER.H"

/* Keep track of this module's initialization */
static E_Boolean G_init = FALSE ;

//static T_objMoveStruct G_playerMove ;
static T_3dObject *G_playerObject = NULL ;
static T_3dObject G_playerRealObject ;
static T_3dObject G_playerFakeObject ;
static T_objTypeInstance G_playerRealObjType = OBJECT_TYPE_INSTANCE_BAD ;
static T_objTypeInstance G_playerFakeObjType = OBJECT_TYPE_INSTANCE_BAD ;

static E_Boolean G_playerIsFake = FALSE ;
#define G_playerMove (G_playerObject->objMove)

static T_sword32 G_lastPlayerX ;
static T_sword32 G_lastPlayerY ;

#define easyabs(x) (((-x)<0)?(-(x)):(x))

static T_word16 G_playerBodyParts[MAX_BODY_PARTS] ;

static T_word16 G_bob = 0 ;

static T_word16 G_turnLeftFraction = PLAYER_MOVE_NONE ;
static T_word16 G_turnRightFraction = PLAYER_MOVE_NONE ;
static T_word16 G_slideLeftFraction = PLAYER_MOVE_NONE ;
static T_word16 G_slideRightFraction = PLAYER_MOVE_NONE ;
static T_word16 G_moveForwardFraction = PLAYER_MOVE_NONE ;
static T_word16 G_moveBackwardFraction = PLAYER_MOVE_NONE ;

static T_sword32 G_turnLeftTotal = 0 ;
static T_sword32 G_turnRightTotal = 0 ;
static T_sword32 G_slideLeftTotal = 0 ;
static T_sword32 G_slideRightTotal = 0 ;
static T_sword32 G_moveForwardTotal = 0 ;
static T_sword32 G_moveBackwardTotal = 0 ;

static E_Boolean G_playerIsMakingSound = FALSE ;

/* Last sector the player was in. */
static T_word16 G_lastSector = 0 ;

/* Keep track of whether or not the player is stealthy and when */
/* to check next. */
static E_Boolean G_playerIsStealthy = FALSE ;
static T_word32 G_nextStealthCheck = 0 ;
static T_void IPlayerUpdateStealth(T_void) ;

static E_Boolean G_playerTeleported = TRUE ;

/*-------------------------------------------------------------------------*
 * Routine:  PlayerInitFirst
 *-------------------------------------------------------------------------*/
/**
 *  PlayerInitFirst sets up the player's basic information that won't
 *  change from level to level.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerInitFirst(T_void)
{
    T_word16 i ;

    DebugRoutine("PlayerInitFirst") ;

    /* Initialize the player's body parts to the normal set. */
    /* Really this should load the body parts from either the disk */
    /* or the server. */
    for (i=0; i<MAX_BODY_PARTS; i++)
        G_playerBodyParts[i] = 520 + i ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerInit
 *-------------------------------------------------------------------------*/
/**
 *  PlayerInit sets up the player's position information.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerInit(T_3dObject *p_obj)
{
    DebugRoutine("PlayerInit") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

//printf("PlayerInit by %s: objectId: %d\n", DebugGetCallerName(), ObjectGetServerId(p_obj)) ;
    ObjectRemoveAttributes(p_obj, OBJECT_ATTR_BODY_PART) ;
    G_playerObject = p_obj ;
    G_playerFakeObject = *p_obj ;
    G_playerRealObject = *p_obj ;
    G_playerFakeObjType = ObjTypeCreate(ObjectGetType(p_obj), p_obj) ;
    G_playerRealObjType = ObjTypeCreate(ObjectGetType(p_obj), G_playerRealObjType) ;

    /* Won't need this object type declaration because */
    /* FakeObjType and RealObjType hold the ones we'll be using */
    ObjectSetType(p_obj, 0) ;
//    ObjTypeDestroy(p_obj->p_objType) ;
    ObjectSetServerId(&G_playerRealObject, 100+ObjectGetServerId(p_obj)) ;
//printf("Real Object = 100 + %d = %d\n", ObjectGetServerId(p_obj), ObjectGetServerId(&G_playerRealObject));

    G_lastPlayerX = G_lastPlayerY = 0 ;

    /* Is this needed? */
    ObjMoveSetZ(&G_playerMove, 700) ;

    PlayerMoveNone() ;

    PlayerSetRealMode() ;
    PlayerSetFakeMode() ;

    /* Initialize the "do not make the sound of 2 voices" variable. */
    G_playerIsMakingSound = FALSE ;

    /* Make sure to check the next stealth check. */
    G_nextStealthCheck = 0 ;

    /* Teleport into the game. */
    G_playerTeleported = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerFinish
 *-------------------------------------------------------------------------*/
/**
 *  PlayerFinish closes out anything that the player position was using.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerFinish(T_void)
{
    DebugRoutine("PlayerFinish") ;
    DebugCheck(G_init == TRUE) ;

    G_init = FALSE ;
    ObjTypeDestroy(G_playerRealObjType) ;
    ObjTypeDestroy(G_playerFakeObjType) ;
    G_playerRealObjType = OBJECT_TYPE_INSTANCE_BAD ;
    G_playerFakeObjType = OBJECT_TYPE_INSTANCE_BAD ;
    G_playerObject->p_objType = NULL ;
    ObjectDestroy(G_playerObject) ;

    /** Set the player object's type to zero to unlock the picture resources. **/
//printf("Player object: %d\n", ObjectGetServerId(G_playerObject)) ;
//ObjectPrint(stdout, G_playerObject) ;
//fflush(stdout) ;
//    ObjectSetType (G_playerObject, 0);

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetX
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetX gets the x location of the current player location.
 *
 *  @return X location of player
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 PlayerGetX(T_void)
{
    T_sword32 x ;

    x = ObjMoveGetX(&G_playerMove) ;

    return x ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetY
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetY gets the y location of the current player location.
 *
 *  @return Y location of player
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 PlayerGetY(T_void)
{
    return ObjMoveGetY(&G_playerMove) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetZ
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetZ gets the z location of the current player location.
 *
 *  @return Z location of player
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 PlayerGetZ(T_void)
{
    return ObjMoveGetZ(&G_playerMove) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetAngle
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetAngle gets the facing angle of the current player's location.
 *
 *  @return facing angle of player
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetAngle(T_void)
{
    return ObjMoveGetAngle(&G_playerMove);
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetAreaSector
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetAreaSector returns the sector that the player is standing
 *  on top of.
 *
 *  @return sector being stood on.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetAreaSector(T_void)
{
    T_word16 sector ;

    DebugRoutine("PlayerGetAreaSector") ;

    sector = ObjMoveGetAreaSector(&G_playerMove) ;

    DebugCheck(sector < G_Num3dSectors) ;
    DebugEnd() ;

    return sector ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetCenterSector
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetCenterSector returns the sector that is under the middle
 *  of the player.
 *
 *  @return sector under center of player.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetCenterSector(T_void)
{
    T_word16 sector ;

    DebugRoutine("PlayerGetCenterSector") ;

    sector = ObjMoveGetCenterSector(&G_playerMove) ;

    DebugCheck(sector < G_Num3dSectors) ;
    DebugEnd() ;

    return sector ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetX
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetX sets the x location of the current player location.
 *
 *  @param x -- new X location of player
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetX(T_sword32 x)
{
    /* Note that this is the last location. */
    G_lastPlayerX = x ;

    ObjMoveSetX(&G_playerMove, x) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetY
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetY sets the y location of the current player location.
 *
 *  @param y -- new Y location of player
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetY(T_sword32 y)
{
    /* Note the last player Y location. */
    G_lastPlayerY = y ;

    ObjMoveSetY(&G_playerMove, y) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetZ
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetZ sets the z location of the current player location.
 *
 *  @param z -- new Z location of player
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetZ(T_sword32 z)
{
    ObjMoveSetZ(&G_playerMove, z) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetAngle
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetAngle changes the facing angle of the player.
 *
 *  @param angle -- new angle of the player
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetAngle(T_word16 angle)
{
    ObjMoveSetAngle(&G_playerMove, angle) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerUpdatePosInfo
 *-------------------------------------------------------------------------*/
/**
 *  PlayerUpdatePosInfo forces the player information to update the
 *  sector and related sector information (like height).
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerUpdatePosInfo(T_void)
{
    DebugRoutine("PlayerUpdatePosInfo") ;

    /* Tug the player just a tiny bit in the x direction. */
//    G_playerMove.XV++ ;

    /* Record where we were. */
    G_lastPlayerX = PlayerGetX() ;
    G_lastPlayerY = PlayerGetY() ;

//printf("Player set up over %d, %d\n", PlayerGetX16(), PlayerGetY16()) ;
//    ObjMoveUpdate(&G_playerMove, 0) ;
    ObjMoveSetUpSectors(&G_playerMove) ;
    ObjectUnlinkCollisionLink(G_playerObject) ;

    /* Tug the player back the other way a bit. */
//    G_playerMove.XV-- ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerIsAboveGround
 *-------------------------------------------------------------------------*/
/**
 *  PlayerIsAboveGround checks to see if the player is above the ground
 *  (and probably falling).
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean PlayerIsAboveGround(T_void)
{
    E_Boolean isAbove ;

    DebugRoutine("PlayerIsAboveGround") ;

    isAbove = ObjMoveIsAboveGround(&G_playerMove) ;

    DebugCheck(isAbove < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return isAbove ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerJump
 *-------------------------------------------------------------------------*/
/**
 *  PlayerJump causes the player to jump by moving the player up a little
 *  and accelerating in the up direction.
 *
 *  @param jumpPower -- Jump by how much
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerJump(T_word16 jumpPower)
{
    E_Boolean isFlying ;
    E_Boolean isAboveGround ;

    DebugRoutine("PlayerJump") ;

    isFlying = EffectPlayerEffectIsActive (PLAYER_EFFECT_FLY) ;
    isAboveGround = PlayerIsAboveGround() ;

    if (!isAboveGround)  {
        /* Move feet off of floor */
        PlayerSetZ(PlayerGetZ()+1) ;

        /* Accelerate the Z velocity by the given jump power. */
        ObjMoveAccelXYZ(&G_playerMove, 0, 0, jumpPower) ;

        if (rand()%2==0)
        {
            /* play a jump 'exertion' sound */
            PlayerMakeSoundGlobal(SOUND_PLAYER_JUMP_SET+(rand()%3),500);
        }
/*
        switch (StatsGetPlayerClassType())
        {
            case CLASS_CITIZEN:
            case CLASS_PRIEST:
            case CLASS_ARCHER:
//          SoundPlayByNumber(SOUND_CITIZEN_JUMP,255);
            ClientSyncSendActionAreaSound(SOUND_CITIZEN_JUMP,500);
            break;

            case CLASS_KNIGHT:
            case CLASS_PALADIN:
//            SoundPlayByNumber(SOUND_PALADIN_JUMP,255);
            ClientSyncSendActionAreaSound(SOUND_PALADIN_JUMP,500);
            break;

            case CLASS_MAGE:
            case CLASS_MAGICIAN:
            case CLASS_WARLOCK:
            ClientSyncSendActionAreaSound(SOUND_MAGE_JUMP,500);
//            SoundPlayByNumber(SOUND_MAGE_JUMP,255);
            break;

            case CLASS_ROGUE:
            case CLASS_SAILOR:
            case CLASS_MERCENARY:
            ClientSyncSendActionAreaSound(SOUND_THIEF_JUMP,500);
//            SoundPlayByNumber(SOUND_THIEF_JUMP,255);
            break;

            default:
            DebugCheck(0);
            break;
        }
*/
    } else if (isFlying)  {
        ObjMoveAccelXYZ(&G_playerMove, 0, 0, (jumpPower>>2)+(jumpPower>>3)) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetMaxVelocity
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetMaxVelocity sets up the player's movement maximum velocity.
 *
 *  @param maxVelocity -- Jump by how much
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetMaxVelocity(T_sword32 maxVelocity)
{
    DebugRoutine("PlayerSetMaxVelocity") ;

    ObjMoveSetMaxVelocity(&G_playerMove, maxVelocity) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerUpdate
 *-------------------------------------------------------------------------*/
/**
 *  PlayerUpdate updates all the movement for the player over the given
 *  delta of time.
 *
 *  @param delta -- Number of timer ticks that have passed.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerUpdate(T_word32 delta)
{
    T_sword16 walkingHeight ;
    T_sword32 oopsX, oopsY, oopsZ ;
    T_word16 oldAngle ;

    DebugRoutine("PlayerUpdate") ;
//printf("PlayerUpdate (%08lX, %08lX, %04X)\n>>>\n", PlayerGetX(), PlayerGetY(), PlayerGetZ16()) ;

    ObjectForceUpdate(G_playerObject) ;
    ObjMoveUpdate(&G_playerObject->objMove, 0) ;

    if (G_playerObject->inWorld != TRUE)  {
        /* If we are not in the world, don't update */
        DebugEnd() ;
        return ;
    }

    /* Update the player's animation. */
    ObjectUpdateAnimation(G_playerObject, SyncTimeGet()) ;

    ObjectUnlinkCollisionLink(G_playerObject) ;
    /* Allow the player to dip. */
    View3dAllowDip() ;
    ObjectClearMoveFlags(G_playerObject, OBJMOVE_FLAG_DO_NOT_SINK) ;

    /* Make sure we are above the walking ground level. */
    walkingHeight = MapGetWalkingFloorHeight(PlayerGetAreaSector()) ;
    if (PlayerGetZ16() < walkingHeight)
        PlayerSetZ16(walkingHeight) ;

    /* TESTING:  Ignore this feature on the player. */
    if (ObjectIsMarkedMakeImpassibleWhenFree(G_playerObject))  {
        ObjectMakeImpassable(G_playerObject) ;
        ObjectMakeSolid(G_playerObject) ;
    }

    ObjectAddAttributes(G_playerObject, OBJECT_ATTR_SLIDE_ONLY) ;

    /** What if delta is too big? **/
    /** I'll break it up into pieces. **/
    if (delta > 20)
    {
        PlayerUpdate (delta >> 1);
        PlayerUpdate ((delta >> 1) + (delta & 1));
    }

    /* Record where we were. */
    G_lastPlayerX = PlayerGetX() ;
    G_lastPlayerY = PlayerGetY() ;

    /* Are we feather falling? */
    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_FEATHER_FALL) != FALSE)  {
        /* Need to clip Z velocity. */
        if (ObjectGetZVel(PlayerGetObject())<-15000)
          ObjectSetZVel (PlayerGetObject(),-15000);

    }

    ObjectNormalGravity(PlayerGetObject()) ;
    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_FLY) != FALSE)  {
        /* Need to clip Z velocity. */
        if (ObjectGetZVel(PlayerGetObject())>15000)
          ObjectSetZVel (PlayerGetObject(),15000);

    } else {
        /* Resist gravity only works when fly is off */
        /* Are we under the effect of a low gravity spell? */
        if (EffectPlayerEffectIsActive(PLAYER_EFFECT_LOW_GRAVITY))  {
            /* Low gravity in effect */
            ObjectLowGravity(PlayerGetObject()) ;
        }
    }

    /* Are we water walking and in water? */
    if ((EffectPlayerEffectIsActive(PLAYER_EFFECT_WATER_WALK)) &&
        (MapGetSectorType(PlayerGetAreaSector()) == SECTOR_TYPE_WATER))  {
        /* Yes, don't sink and don't flow */
        ObjectSetMoveFlags(PlayerGetObject(), OBJMOVE_FLAG_DO_NOT_SINK) ;
        ObjectDoesNotFlow(PlayerGetObject()) ;
    /* Are we lava walking and in lava? */
    } else if ((EffectPlayerEffectIsActive(PLAYER_EFFECT_LAVA_WALK)) &&
        (MapGetSectorType(PlayerGetAreaSector()) == SECTOR_TYPE_LAVA))  {
        /* Yes, don't sink and don't flow */
        ObjectSetMoveFlags(PlayerGetObject(), OBJMOVE_FLAG_DO_NOT_SINK) ;
        ObjectDoesNotFlow(PlayerGetObject()) ;
    } else {
        /* Yes, sink and flow */
        ObjectClearMoveFlags(PlayerGetObject(), OBJMOVE_FLAG_DO_NOT_SINK) ;
        ObjectDoesFlow(PlayerGetObject()) ;
    }

    /* Are we under the effect of a sticky feet spell? */
    if (EffectPlayerEffectIsActive(PLAYER_EFFECT_STICKY_FEET))  {
        /* sticky feet in effect */
        ObjectForceNormalFriction(PlayerGetObject()) ;
    } else {
        /* Just regular friction */
        ObjectRegularFriction(PlayerGetObject()) ;
    }

#if 0
    p_obj = ObjectsGetFirst() ;
    while (p_obj)  {
        if (ObjectIsMarkedMakeImpassibleWhenFree(p_obj))  {
            ObjectMakeImpassable(p_obj) ;
            ObjectMakeSolid(p_obj) ;
        }
        p_obj = ObjectGetNext(p_obj) ;
    }
#endif

    if ((ObjectCheckIfCollide(
        PlayerGetObject(),
        PlayerGetX(),
        PlayerGetY(),
        PlayerGetZ())) ||
       (ObjectIsBeingCrushed(G_playerObject)))  {
        oopsX = PlayerGetX() ;
        oopsY = PlayerGetY() ;
        oopsZ = PlayerGetZ() ;
        /* Move the player back to the last gauranteed good move */
        /* to ensure that he is not inside something. */
        /* But keep the correct angle */
        oldAngle = PlayerGetAngle() ;
        G_playerObject->objMove = *(ClientSyncGetLastGoodMove()) ;
        PlayerSetAngle(oldAngle) ;

        /* Try to move in a straight line from the last good location */
        /* to where we were. */
        Collide3dMoveToXYZ(
              G_playerObject,
              oopsX,
              oopsY,
              oopsZ) ;
    } else {
        if ((EffectPlayerEffectIsActive (PLAYER_EFFECT_FLY)==TRUE) &&
            (PlayerIsAboveGround()))
            ObjMoveAccelXYZ(&G_playerMove, 0, 0, 800 * delta) ;

        ObjectForceUpdate(PlayerGetObject()) ;
        ObjectAddAttributes(PlayerGetObject(), OBJECT_ATTR_SLIDE_ONLY) ;
        ObjMoveUpdate(&G_playerMove, delta) ;
    }

    G_turnLeftTotal += ((delta * G_turnLeftFraction)<<8) +
                       ((delta * G_turnLeftFraction)<<7) ;
    G_turnRightTotal += ((delta * G_turnRightFraction)<<8) +
                        ((delta * G_turnRightFraction)<<7) ;
    G_slideLeftTotal += (delta * G_slideLeftFraction) ;
    G_slideRightTotal += (delta * G_slideRightFraction) ;
    G_moveForwardTotal += (delta * G_moveForwardFraction) ;
    G_moveBackwardTotal += (delta * G_moveBackwardFraction) ;

    if (G_turnLeftTotal >= 0x100)  {
        PlayerSetAngle(
            PlayerGetAngle() +
            (G_turnLeftTotal >> 8)) ;
        G_turnLeftTotal &= 0xFF ;
    }

    if (G_turnRightTotal >= 0x100)  {
        PlayerSetAngle(
            PlayerGetAngle() -
            (G_turnRightTotal >> 8)) ;
        G_turnRightTotal &= 0xFF ;
    }

    if (G_moveForwardTotal >= 0x100)  {
        PlayerAccelDirection(
            PlayerGetAngle(),
            (G_moveForwardTotal >> 8)) ;
        PlayerSetStance(STANCE_WALK) ;
        G_moveForwardTotal &= 0xFF ;
    }

    if (G_moveBackwardTotal >= 0x100)  {
        PlayerAccelDirection(
            PlayerGetAngle(),
            -(G_moveBackwardTotal >> 9)) ;
        PlayerSetStance(STANCE_WALK) ;
        G_moveBackwardTotal &= 0xFF ;
    }

    if (G_slideLeftTotal >= 0x100)  {
    }

    if (ObjectIsMarkedMakeImpassibleWhenFree(G_playerObject))  {
        ObjectMakeImpassable(G_playerObject) ;
        ObjectMakeSolid(G_playerObject) ;
    }

    /* All fractional movement goes to zero. */
    G_turnLeftFraction = PLAYER_MOVE_NONE ;
    G_turnRightFraction = PLAYER_MOVE_NONE ;
    G_slideLeftFraction = PLAYER_MOVE_NONE ;
    G_slideRightFraction = PLAYER_MOVE_NONE ;
    G_moveForwardFraction = PLAYER_MOVE_NONE ;
    G_moveBackwardFraction = PLAYER_MOVE_NONE ;

/*
foundVoid = Collide3dGetSectorsInBox(
    PlayerGetX(),
    PlayerGetY(),
    ObjectGetRadius(PlayerGetObject()),
    50,
    sectors,
    &numSectors) ;
G_playerObject->objMove.numOnSectors = numSectors ;
for (i=0; i<numSectors; i++)  {
    G_playerObject->objMove.OnSectors[i] = sectors[i] ;
}
*/
    /* Check to see if we are being crushed! */
    if (ObjectIsBeingCrushed(G_playerObject))
//        StatsChangePlayerHealth(delta * -10) ;
        StatsTakeDamage (EFFECT_DAMAGE_NORMAL,(delta*30));

    IPlayerUpdateStealth() ;
    ObjectUnlinkCollisionLink(G_playerObject) ;

//puts("<<<\n") ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetTravelDistance
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetTravelDistance determines how far the player traveled on the
 *  last call to PlayerUpdate.  Teleportation does not do this.
 *
 *  @return distance traveled in unaccurate terms.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetTravelDistance(T_void)
{
    T_word16 distance ;

    DebugRoutine("PlayerGetTravelDistance") ;

    distance = CalculateDistance(
                   G_lastPlayerX >> 16,
                   G_lastPlayerY >> 16,
                   PlayerGetX() >> 16,
                   PlayerGetY() >> 16) ;

    DebugEnd() ;

    return distance ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerStopMoving
 *-------------------------------------------------------------------------*/
/**
 *  PlayerStopMoving causes the player to come to a complete halt.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerStopMoving(T_void)
{
    DebugRoutine("PlayerStopMoving") ;

    ObjMoveStopMoving(&G_playerMove) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerAccelXYZ
 *-------------------------------------------------------------------------*/
/**
 *  PlayerAccelXYZ gives the player a push in a given direction.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerAccelXYZ(T_sword32 accelX, T_sword32 accelY, T_sword32 accelZ)
{
    DebugRoutine("PlayerAccelXYZ") ;

    ObjMoveAccelXYZ(&G_playerMove, accelX, accelY, accelZ) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetFriction
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetFriction returns the friction of the surface under the
 *  player.
 *
 *  @return Friction under player.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetFriction(T_void)
{
    T_word16 friction ;

    DebugRoutine("PlayerGetFriction") ;

    /* Are we under the effect of a sticky feet spell? */
    if (EffectPlayerEffectIsActive(PLAYER_EFFECT_STICKY_FEET))  {
        /* sticky feet in effect */
        friction = OBJMOVE_NORMAL_FRICTION ;
    } else {
        /* Just regular friction */
        friction = ObjMoveGetFriction(&G_playerMove) ;
    }

    DebugEnd() ;

    return friction ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetNumAreaSectors
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetNumAreaSectors tells how many area sectors the player is
 *  standing over.
 *
 *  @return number of area sectors
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetNumAreaSectors(T_void)
{
    T_word16 num ;

    DebugRoutine("PlayerGetNumAreaSectors") ;

//    num = ObjMoveGetNumAreaSectors(&G_playerMove) ;
    num = ObjectGetNumAreaSectors(G_playerObject) ;

    DebugEnd() ;

    return num ;
}


/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetNthAreaSector
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetNthAreaSecotr gets one of the sectors in the sector list
 *  of sectors that they are over.
 *
 *  @return Sector in list
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetNthAreaSector(T_word16 n)
{
    T_word16 sector ;

    DebugRoutine("PlayerGetNthAreaSector") ;

//    sector = ObjMoveGetNthAreaSector(&G_playerMove, n) ;
    sector = ObjectGetNthAreaSector(G_playerObject, n) ;

    DebugEnd() ;

    return sector ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerAccelDirection
 *-------------------------------------------------------------------------*/
/**
 *  PlayerAccelDirection pushes the player in a given facing direction
 *  with the given amount of acceleration.
 *
 *  @param direction -- Direction to move toward
 *  @param accel -- Acceleration amount
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerAccelDirection(T_word16 direction, T_sword16 accel)
{
    T_sword32 xAccel, yAccel ;

    DebugRoutine ("PlayerAccelDirection");

    xAccel = (MathCosineLookup(direction)*accel*PlayerGetFriction())>>3;
    yAccel = (MathSineLookup(direction)*accel*PlayerGetFriction())>>3;
    PlayerAccelXYZ(xAccel, yAccel, 0) ;

//if (PlayerIsAboveGround() == FALSE)
//  G_bob += accel ;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetCameraView
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetCameraView does the work of transferring the player's
 *  coordinates and facing direction to the actual 3d view.  Until this
 *  is done, the view stays like it normally does.
 *
 *<!-----------------------------------------------------------------------*/
static T_sword32 G_eyeBall = 0 ;
static T_word32 G_lastEyed = 0 ;

T_void PlayerSetCameraView(T_void)
{
    T_sword16 bobHeight ;
    T_sword32 bobHeight32 ;
    T_word16 sector ;
    T_sword16 ceiling ;
    T_sword16 floor ;
    T_sword16 eyeLevel ;
    T_sword32 eyeLevel32 ;
    T_word16 num ;
    T_word16 i ;
    T_sword32 diff ;
    T_sword32 delta ;
    T_sword32 step ;

    DebugRoutine("PlayerSetCameraView") ;

    if (PlayerIsAboveGround() == FALSE)

    if (!ClientIsPaused())
        G_bob+=PlayerGetTravelDistance();

    if (ConfigGetBobOffFlag())  {
        bobHeight = 0 ;
        bobHeight32 = 0 ;
    } else {
        bobHeight = (4 * MathSineLookup(G_bob << 9)) >> 15 ;
        bobHeight32 = (4 * MathSineLookup(G_bob << 9)) << 1 ;
    }
    ObjectSetClimbHeight(PlayerGetObject(), StatsGetClimbHeight()) ;

    if (ClientIsDead())  {
        eyeLevel = PlayerGetZ16() + 30;
        eyeLevel32 = PlayerGetZ() + (30<<16) ;
    } else {
        eyeLevel = PlayerGetZ16() + StatsGetTallness() + bobHeight ;
        eyeLevel32 = PlayerGetZ() + (StatsGetTallness() << 16) + bobHeight32 ;
    }

    if ((G_lastEyed > TickerGet()) || (G_lastEyed == 0))  {
        /* Reset everything. */
        G_eyeBall = eyeLevel32 ;
    } else {
        /* Follow the eye to where it needs to go. */
#if 0
        delta = TickerGet() - G_lastEyed ;
        diff = eyeLevel32 - G_eyeBall ;
        step = (delta * diff) / 10 ;
        if ((step < (delta * 0x10000)) && (step > (-delta * 0x10000)))  {
            step = delta * 0x10000 ;
            if (diff < 0)
                step = -step ;
        }
        if (diff == 0)
            step = 0 ;

MessagePrintf("delta: %d, diff: %d, step: %d, eye: %d\n", delta, diff>>16, step, G_eyeBall>>16) ;
        if ((eyeLevel32 > G_eyeBall) && ((G_eyeBall + step) > eyeLevel32))
            G_eyeBall = eyeLevel32 ;
        else if ((eyeLevel32 < G_eyeBall) && ((G_eyeBall + step) < eyeLevel32))
            G_eyeBall = eyeLevel32 ;
        else
            G_eyeBall += step ;
#endif
        delta = TickerGet() - G_lastEyed ;
        if (delta > 19)
            delta = 19 ;
        diff = eyeLevel32 - G_eyeBall ;
        if (delta == 0)
            step = 0 ;
        else
            step = (7 * diff) / (20 - delta) ;
        if (step < 0)
            step *= 2 ;

        if ((eyeLevel32 > G_eyeBall) && ((G_eyeBall + step) > eyeLevel32))
            G_eyeBall = eyeLevel32 ;
        else if ((eyeLevel32 < G_eyeBall) && ((G_eyeBall + step) < eyeLevel32))
            G_eyeBall = eyeLevel32 ;
        else
            G_eyeBall += step ;
    }

#if 0
    num = PlayerGetNumAreaSectors() ;
    for (i=0; i<num; i++)  {
        sector = PlayerGetNthAreaSector(i) ;
        ceiling = MapGetCeilingHeight(sector) ;
        floor = MapGetFloorHeight(sector) ;

        if (eyeLevel+3 >= ceiling)  {
            eyeLevel = ceiling-3 ;
            eyeLevel32 = eyeLevel<<16 ;
        }
        if (eyeLevel-3 <= floor)  {
            eyeLevel = floor+3 ;
            eyeLevel32 = eyeLevel<<16 ;
        }
        if (eyeLevel >= ceiling)  {
            eyeLevel = ceiling ;
            eyeLevel32 = eyeLevel<<16 ;
        }
    }
#endif
    /* Make sure we are below the ceiling and above the floor. */
    eyeLevel = G_eyeBall >> 16 ;
    num = PlayerGetNumAreaSectors() ;
    for (i=0; i<num; i++)  {
        sector = PlayerGetNthAreaSector(i) ;
        ceiling = MapGetCeilingHeight(sector) ;
        floor = MapGetFloorHeight(sector) ;

        if (eyeLevel+3 >= ceiling)  {
            eyeLevel = ceiling-3 ;
            G_eyeBall = eyeLevel<<16 ;
        }
        if (eyeLevel-3 <= floor)  {
            eyeLevel = floor+3 ;
            G_eyeBall = eyeLevel<<16 ;
        }
        if (eyeLevel >= ceiling)  {
            eyeLevel = ceiling ;
            G_eyeBall = eyeLevel<<16 ;
        }
    }

    G_lastEyed = TickerGet() ;

    View3dSetView(
        PlayerGetX16(),
        PlayerGetY16(),
        G_eyeBall,
        PlayerGetAngle()) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerTeleport
 *-------------------------------------------------------------------------*/
/**
 *  PlayerTeleport is the major move all solution to moving the current
 *  POV.  It moves the POV to the x, y, and angle as passed into the
 *  routine.
 *
 *  @param x -- Accurate X coordinate on map
 *  @param y -- Accurate Y coordinate on map
 *  @param angle -- Angle to face when moved.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerTeleport(T_sword16 x, T_sword16 y, T_word16 angle)
{
    T_word16 sector ;

    DebugRoutine("PlayerTeleport") ;

    sector = View3dFindSectorNum(x, y) ;
    if (sector != 0xFFFF)  {
//printf("Player teleporting to %d %d\n", x, y) ;
        ObjectTeleport(G_playerObject, x, y) ;
        G_lastEyed = 0 ;
//puts("Player set angle") ; fflush(stdout) ;
        PlayerSetAngle(angle) ;
        PlayerUpdatePosInfo() ;
        PlayerSetCameraView() ;

        /* If we actually teleported, note it. */
        if ((PlayerGetX16() == x) && (PlayerGetY16() == y))
            PlayerTeleported() ;
    } else {
#ifndef NDEBUG
        printf("Bad sector to teleport to! %d %d\n", x, y) ;
#endif
    }

    ColorAddGlobal(63, 63, 0);

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetObject
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetObject tells what the player object is.
 *
 *<!-----------------------------------------------------------------------*/
T_3dObject *PlayerGetObject(T_void)
{
#ifdef SERVER_ONLY
    DebugCheck(FALSE) ;
    return NULL ;
#endif
    return G_playerObject ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerAddExternalObject
 *-------------------------------------------------------------------------*/
/**
 *  PlayerAddExternalObject moves the player along the x, y, and z dir.
 *
 *  @param dx -- X External velocity
 *  @param dy -- Y External velocity
 *  @param dz -- Z External velocity
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerAddExternalVelocity(T_sword32 dx, T_sword32 dy, T_sword32 dz)
{
    ObjMoveAddExternalVelocity(
        &G_playerObject->objMove,
        dx,
        dy,
        dz) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetStance
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetStance returns what stance the player is at.
 *
 *  @return stance number
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetStance(T_void)
{
    T_word16 stance ;
    DebugRoutine("PlayerGetStance") ;

    stance = ObjectGetStance(PlayerGetObject()) ;

    DebugCheck(stance < STANCE_UNKNOWN) ;
    DebugEnd() ;

    return stance ;
}


/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetStance
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetStance puts the player in another stance.  If the player was
 *  hurt recently, it will time out after awhile.
 *
 *  @param stance -- stance number
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetStance(T_word16 stance)
{
    DebugRoutine("PlayerSetStance") ;
    DebugCheck(stance < STANCE_UNKNOWN) ;

    /* Only do if we are in the game. */
    if (ClientIsInView())  {
        /* Are we allowed to change the stance yet? Always show attack stances. */
        if ((PlayerGetStance() == STANCE_STAND) ||
            (PlayerGetStance() == STANCE_WALK) ||
            (stance == STANCE_DIE) ||
            ((PlayerGetStance() == STANCE_DIE) && (!ClientIsDead())))  {
//printf("PlayerSetStance %d\n", stance) ;
            /* Set the new stance. */
            ObjectSetStance(PlayerGetObject(), stance) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSetBodyPart
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSetBodyPart    changes a part on the body and sends it to all
 *  the other machines.
 *
 *  @param location -- location of body part
 *  @param newPart -- Number of new part
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSetBodyPart(
           T_bodyPartLocation location,
           T_word16 newPart)
{
    DebugRoutine("PlayerSetBodyPart") ;
    DebugCheck(location < BODY_PART_LOCATION_UNKNOWN) ;

    if (G_playerBodyParts[location] != newPart)  {
        G_playerBodyParts[location] = newPart ;

//printf("PlayerSetBodyPart: %d %d\n", location, newPart) ;
        ClientSyncSendActionChangeSelf(
            location,
            newPart) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetBodyPart
 *-------------------------------------------------------------------------*/
/**
 *  PlayerGetBodyPart reports the type of part for the given body
 *  location on the player.
 *
 *  @param location -- location of body part
 *
 *  @return body type
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetBodyPart(T_bodyPartLocation location)
{
    T_word16 part ;

    DebugRoutine("PlayerChangeBodyPart") ;
    DebugCheck(location < BODY_PART_LOCATION_UNKNOWN) ;

    part = G_playerBodyParts[location] ;

    DebugEnd() ;

    return part ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerDraw
 *-------------------------------------------------------------------------*/
/**
 *  PlayerDraw draws what the player looks like at the given location
 *  on the current screen.
 *
 *  @param x -- X Location on the screen
 *  @param y -- Y Location on the screen
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerDraw(T_word16 x, T_word16 y)
{
    T_word16 i ;
    T_bitmap *p_pic ;
    static T_byte8 reorder[MAX_BODY_PARTS] = {
        BODY_PART_LOCATION_CHEST,
        BODY_PART_LOCATION_HEAD,
        BODY_PART_LOCATION_LEGS,
        BODY_PART_LOCATION_LEFT_ARM,
        BODY_PART_LOCATION_RIGHT_ARM,
        BODY_PART_LOCATION_WEAPON,
        BODY_PART_LOCATION_SHIELD,
    } ;
    T_byte8 name[80] ;
    T_resource res ;

    DebugRoutine("PlayerDraw") ;

    for (i=0; i<MAX_BODY_PARTS; i++)  {
        if (G_playerBodyParts[i])   {
            sprintf(name, "OBJS/%05d/32000", G_playerBodyParts[i] & 0x0FFF) ;
//printf("Drawing body part %s at %d\n", name, i) ; fflush(stdout) ;
            p_pic = (T_bitmap *)PictureLockData(name, &res) ;
            DebugCheck(p_pic != NULL) ;
//printf("Size (%d, %d)\n", p_pic->sizex, p_pic->sizey) ; fflush(stdout) ;
            GrDrawCompressedBitmapAndClipAndColor(
                p_pic,
                x+32-(p_pic->sizey>>1),
                y+64-p_pic->sizex,
                G_playerBodyParts[i]>>12) ;
            PictureUnlockAndUnfind(res) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerTeleportAlways
 *-------------------------------------------------------------------------*/
/**
 *  PlayerTeleportAlways is just like PlayerTeleport buts doesn't care
 *  if there is something in the way.
 *
 *  @param x -- Accurate X coordinate on map
 *  @param y -- Accurate Y coordinate on map
 *  @param angle -- Angle to face when moved.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerTeleportAlways(T_sword16 x, T_sword16 y, T_word16 angle)
{
    T_word16 sector ;
    T_sword16 height ;

    DebugRoutine("PlayerTeleportAlways") ;

//printf("Teleport always to %d %d\n", x, y) ;
    sector = View3dFindSectorNum(x, y) ;
    if (sector != 0xFFFF)  {
        /* Get the height of the sector. */
        height = G_3dSectorArray[sector].floorHt;

        ObjectsMakeTemporarilyPassableAtXYRadius(
            x,
            y,
            40,
            PlayerGetZ16(),
            PlayerGetZ16() + ObjectGetHeight(PlayerGetObject())) ;

        G_lastEyed = 0 ;
        ObjectTeleportAlways(G_playerObject, x, y) ;
        PlayerSetAngle(angle) ;
        PlayerSetZ16(height) ;

        /* Go directly to jail, do not pass GO ... */
        PlayerSetCameraView() ;

        sector = PlayerGetAreaSector() ;
        G_lastSector = sector ;
//printf("TeleportAlways: Player at %d %d\n", PlayerGetX16(), PlayerGetY16()) ;
    } else {
#ifndef NDEBUG
        printf("TeleportALways: Bad sector to teleport to! %d %d\n", x, y) ;
#endif
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerTurnLeft
 *-------------------------------------------------------------------------*/
/**
 *  PlayerTurnLeft turns the player left by the given fractional amount.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerTurnLeft(T_word16 fraction)
{
    DebugRoutine("PlayerTurnLeft") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_turnLeftFraction = fraction ;
    if (fraction == 0)
        G_turnLeftTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerTurnRight
 *-------------------------------------------------------------------------*/
/**
 *  PlayerTurnRight turns the player right the given fractional speed.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerTurnRight(T_word16 fraction)
{
    DebugRoutine("PlayerTurnRight") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_turnRightFraction = fraction ;
    if (fraction == 0)
        G_turnRightTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSlideLeft
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSlideLeft slides the player left the given speed.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSlideLeft(T_word16 fraction)
{
    DebugRoutine("PlayerSlideLeft") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_slideLeftFraction = fraction ;
    if (fraction == 0)
        G_slideLeftTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerSlideRight
 *-------------------------------------------------------------------------*/
/**
 *  PlayerSlideRight slides the player right the given speed.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerSlideRight(T_word16 fraction)
{
    DebugRoutine("PlayerSlideRight") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_slideRightFraction = fraction ;
    if (fraction == 0)
        G_slideRightTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerMoveForward
 *-------------------------------------------------------------------------*/
/**
 *  PlayerMoveForward moves the player forward the given speed.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerMoveForward(T_word16 fraction)
{
    DebugRoutine("PlayerMoveForward") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_moveForwardFraction = fraction ;
    if (fraction == 0)
        G_moveForwardTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerMoveBackward
 *-------------------------------------------------------------------------*/
/**
 *  PlayerMoveBackward moves the player backward the given speed.
 *
 *  @param fraction -- fractional amount allowed
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerMoveBackward(T_word16 fraction)
{
    DebugRoutine("PlayerMoveBackward") ;
    DebugCheck(fraction < PLAYER_MOVE_MAXIMUM) ;

    G_moveBackwardFraction = fraction ;
    if (fraction == 0)
        G_moveBackwardTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerMoveNone
 *-------------------------------------------------------------------------*/
/**
 *  PlayerMoveNone cancels all declared movement for this frame.
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerMoveNone(T_void)
{
    DebugRoutine("PlayerMoveNone") ;

    G_turnLeftFraction = PLAYER_MOVE_NONE ;
    G_turnRightFraction = PLAYER_MOVE_NONE ;
    G_slideLeftFraction = PLAYER_MOVE_NONE ;
    G_slideRightFraction = PLAYER_MOVE_NONE ;
    G_moveForwardFraction = PLAYER_MOVE_NONE ;
    G_moveBackwardFraction = PLAYER_MOVE_NONE ;

    G_turnLeftTotal = 0 ;
    G_turnRightTotal = 0 ;
    G_slideLeftTotal = 0 ;
    G_slideRightTotal = 0 ;
    G_moveForwardTotal = 0 ;
    G_moveBackwardTotal = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerGetLastSector
 *-------------------------------------------------------------------------*/
/**
 *  PlayerMoveNone cancels all declared movement for this frame.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PlayerGetLastSector(T_void)
{
    return G_lastSector ;
}

T_void PlayerSetLastSector(T_word16 lastSector)
{
    G_lastSector = lastSector ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PlayerJumpForward
 *-------------------------------------------------------------------------*/
/**
 *  PlayerJumpForward makes the player take a step forward regardless
 *  of anything else except for colllisions.
 *
 *  @param distance -- How far to jump forward
 *
 *<!-----------------------------------------------------------------------*/
T_void PlayerJumpForward(T_word16 distance)
{
    T_sword32 newX, newY, newZ ;

    newZ = PlayerGetZ() ;
    ObjectGetForwardPosition(
        PlayerGetObject(),
        distance,
        &newX,
        &newY) ;
    Collide3dMoveToXYZ(
        PlayerGetObject(),
        newX,
        newY,
        newZ) ;
    newZ = PlayerGetZ() ;
    ObjectSetUpSectors(PlayerGetObject()) ;
    ObjectUnlinkCollisionLink(G_playerObject) ;
    PlayerSetZ(newZ) ;
    ObjectForceUpdate(PlayerGetObject()) ;
    ObjectSetMovedFlag(PlayerGetObject()) ;
    ObjectUpdateZVel(PlayerGetObject(), 1) ;
}

E_Boolean PlayerInFakeMode(T_void)
{
    return G_playerIsFake;
}

T_void PlayerSetRealMode(T_void)
{
    DebugRoutine("PlayerSetRealMode") ;

//printf("SetRealMode by %s\n", DebugGetCallerName()) ;  fflush(stdout) ;
//    DebugCheck(G_playerIsFake) ;
    if (G_playerIsFake)  {
        DebugCheck(G_playerObject != NULL) ;
        if (G_playerObject != NULL)  {
            G_playerFakeObject.objMove = G_playerObject->objMove ;
            G_playerObject->objMove = G_playerRealObject.objMove ;
            G_playerObject->objServerId = G_playerRealObject.objServerId ;
DebugCheck(G_playerObject->objServerId != 0) ;
            G_playerObject->p_objType = G_playerRealObjType ;
            G_playerIsFake = FALSE ;
        }
    }

    DebugEnd() ;
}

T_void PlayerSetFakeMode(T_void)
{
    DebugRoutine("PlayerSetFakeMode") ;

//printf("SetFakeMode by %s\n", DebugGetCallerName()) ;  fflush(stdout) ;
    DebugCheck(!G_playerIsFake) ;
    if (!G_playerIsFake)  {
        DebugCheck(G_playerObject != NULL) ;
        if (G_playerObject != NULL)  {
            G_playerRealObject.objMove = G_playerObject->objMove ;
            G_playerObject->objMove = G_playerFakeObject.objMove ;
            G_playerObject->objServerId = G_playerFakeObject.objServerId ;
DebugCheck(G_playerObject->objServerId != 0) ;
            G_playerObject->p_objType = G_playerFakeObjType ;
            G_playerIsFake = TRUE ;
        }
    }

    DebugEnd() ;
}

T_void PlayerFakeOverwriteCurrent(T_void)
{
    DebugRoutine("PlayerFakeOverwriteCurrent") ;

//printf("FakeOverwriteCurrent by %s\n", DebugGetCallerName()) ;  fflush(stdout) ;
    DebugCheck(G_playerObject != NULL) ;
//    G_playerObject->objMove = G_playerFakeObject.objMove ;
    G_playerRealObject.objMove = G_playerObject->objMove ;

    DebugEnd() ;
}

T_void IPlayerStoppedMakingSound(T_void *p_data)
{
    G_playerIsMakingSound = FALSE ;
}

/* Play a sound locally, but don't play it if we are still playing */
/* another voicing. */
T_void PlayerMakeSoundLocal(T_word16 soundNum)
{
    DebugRoutine("PlayerMakeSoundLocal") ;

    /* Don't do anything if we are already making a sound. */
    if (G_playerIsMakingSound == FALSE)  {
        G_playerIsMakingSound = TRUE ;
        SoundPlayByNumberWithCallback(
            soundNum,
            255,
            IPlayerStoppedMakingSound,
            NULL) ;
    }

    DebugEnd() ;
}

/* Make a sound by send a create sound action to all the other players. */
/* In this case, it is declared to be a "voicing" which means that */
/* when the sound is to be played, it will be played with */
/* PlayerMakeSoundLocal and not to be doubly played. */
T_void PlayerMakeSoundGlobal(T_word16 soundNum, T_word16 radius)
{
    DebugRoutine("PlayerMakeSoundGlobal") ;

    /* Don't do anything if we are already making a sound. */
    if (G_playerIsMakingSound == FALSE)  {
        ClientSyncSendActionAreaSound(soundNum, radius, TRUE) ;
    }

    DebugEnd() ;
}

E_Boolean PlayerIsStealthy(T_void)
{
    return G_playerIsStealthy ;
}

static T_void IPlayerUpdateStealth(T_void)
{
    T_word16 stealthLevel ;

    /* Update our stealth checks. */
    if (TickerGet() >= G_nextStealthCheck)  {
        /* Are we moving or standing still? */
        if ((ObjectGetXVel(G_playerObject) != 0) ||
            (ObjectGetYVel(G_playerObject) != 0) ||
            (ObjectGetZVel(G_playerObject) != 0))  {
            /* Moving around. */
            /* Check again in a second. */
            G_nextStealthCheck = TickerGet() + 70 ;
        } else {
            /* Standing still. */
            /* Check again in three seconds. */
            G_nextStealthCheck = TickerGet() + 210 ;
        }

        /* Do our random check based on if we are translucent */
        /* or invisible. */
        stealthLevel = StatsGetPlayerStealth()>>1 ;

        /* If invisible, minimum stealth level of 240. */
        if ((EffectPlayerEffectIsActive(PLAYER_EFFECT_INVISIBLE)) &&
                 (stealthLevel < 190))
            stealthLevel = 190 ;

        /* If translucent, minimum stealth level of 120. */
        if ((EffectPlayerEffectIsActive(PLAYER_EFFECT_TRANSLUCENT)) &&
                 (stealthLevel < 100))
            stealthLevel = 100 ;

        G_playerIsStealthy = ((rand() % 200) < stealthLevel) ? TRUE : FALSE ;
    }
}

E_Boolean PlayerJustTeleported(T_void)
{
    E_Boolean justTele = G_playerTeleported ;

    G_playerTeleported = FALSE ;

    return justTele ;
}

T_void PlayerTeleported(T_void)
{
    G_playerTeleported = TRUE ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PLAYER.C
 *-------------------------------------------------------------------------*/
