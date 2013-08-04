/*-------------------------------------------------------------------------*
 * File:  SERVERSH.C
 *-------------------------------------------------------------------------*/
/**
 * In the past, there was a "shared" group of code between different
 * server types.  This code needs to be combined with SERVER.C.
 *
 * @addtogroup SERVERSH
 * @brief Server shared code
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_TRIG.H"
#include "CLIENT.H"
#include "CRELOGIC.H"
#include "DOOR.H"
#include "EFFECT.H"
#include "OBJECT.H"
#include "PLAYER.H"
#include "SERVER.H"
#include "SERVERSH.H"
#include "STATS.H"

/* Module:  Server Shared Routines (between client server and game server) */
extern T_sword32 G_sourceX, G_sourceY, G_sourceZ ;

/*-------------------------------------------------------------------------*
 * Routine:  ServerDamageObjectXYZ
 *-------------------------------------------------------------------------*/
/**
 *  ServerDamageObject does damage to any object.
 *
 *  @param p_obj -- Object to damage
 *  @param data -- Amount of damage to do and what type
 *      and owner
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ServerDamageObjectXYZ(
              T_3dObject *p_obj,
              T_word32 data)
{
    T_word16 angle ;
    T_damageObjInfo *p_damageInfo ;
    E_Boolean isFake ;

    DebugRoutine("ServerDamageObjectXYZ") ;

    p_damageInfo = (T_damageObjInfo *)data ;

    /* Don't hurt owner of damage (if a creature). */
    if ((!(ObjectIsCreature(p_obj))) || (p_damageInfo->ownerID != ObjectGetServerId(p_obj)))  {
        /* If it is a creature, damage it. */
        if (!ObjectIsPassable(p_obj))  {
            /* TESTING */
            if (!(p_damageInfo->type & EFFECT_DAMAGE_SPECIAL))  {
    //            printf("(%d) ServerDamageObjectXYZ %d for %d by %s\n", SyncTimeGet(), ObjectGetServerId(p_obj), p_damageInfo->damage, DebugGetCallerName()) ;
                if (ObjectIsCreature(p_obj))  {
                    CreatureGoSplat(
                        p_obj,
                        p_damageInfo->damage,
                        p_damageInfo->type) ;
                } else {
                    ClientMakeObjectGoSplat(
                        p_obj,
                        p_damageInfo->damage,
                        p_damageInfo->type,
                        0,
                        BLOOD_EFFECT_NORMAL) ;
                }
                /* Give experience to me if I hit a creature that */
                /* was not a special type. */
                if (p_damageInfo->ownerID == ObjectGetServerId(PlayerGetObject()))  {
                    if ((ObjectIsPlayer(p_obj) && (p_obj != PlayerGetObject())))  {
                        StatsChangePlayerExperience(p_damageInfo->damage);
                    }
                }
            }

            if (ObjectIsCreature(p_obj))  {
                angle = MathArcTangent(
                           (ObjectGetX(p_obj) - G_sourceX)>>16,
                           (ObjectGetY(p_obj) - G_sourceY)>>16) ;

                /* Push the creature from the damage. */
                if (p_damageInfo->type != (EFFECT_DAMAGE_SPECIAL |
                                EFFECT_DAMAGE_SPECIAL_CONFUSE))  {
                    if (p_damageInfo->type == (EFFECT_DAMAGE_SPECIAL |
                                     EFFECT_DAMAGE_SPECIAL_PULL))  {
                        /* Pull the creature. */
                        ObjectAccelFlat(
                            p_obj,
                            -(p_damageInfo->damage/90),
                            angle) ;
                        ObjectForceUpdate(p_obj) ;
                    } else {
                        /* Push the creature. */
                        ObjectAccelFlat(
                            p_obj,
                            p_damageInfo->damage/90,
                            angle) ;
                        ObjectForceUpdate(p_obj) ;
                    }
                }

                CreatureTakeDamage(
                    p_obj,
                    p_damageInfo->damage,
                    p_damageInfo->type,
                    p_damageInfo->ownerID) ;
            } else if (p_obj == PlayerGetObject())  {
                /* If it is me, hurt me (and push as well). */
                isFake = PlayerInFakeMode() ;
                if (isFake)
                    PlayerSetRealMode() ;

                p_obj = PlayerGetObject() ;

                angle = MathArcTangent(
                           (ObjectGetX(p_obj) - G_sourceX)>>16,
                           (ObjectGetY(p_obj) - G_sourceY)>>16) ;

                /* Push the creature from the damage. */
                if (p_damageInfo->type != (EFFECT_DAMAGE_SPECIAL |
                                EFFECT_DAMAGE_SPECIAL_CONFUSE))  {
                    if (p_damageInfo->type == (EFFECT_DAMAGE_SPECIAL |
                                     EFFECT_DAMAGE_SPECIAL_PULL))  {
                        /* Pull the creature. */
                        ObjectAccelFlat(
                            p_obj,
                            -(p_damageInfo->damage/90),
                            angle) ;
                        ObjectForceUpdate(p_obj) ;
                    } else {
                        /* Push the creature. */
                        ObjectAccelFlat(
                            p_obj,
                            p_damageInfo->damage/90,
                            angle) ;
                        ObjectForceUpdate(p_obj) ;
                    }
                }

                if (isFake)
                    PlayerSetFakeMode() ;

                StatsTakeDamage(
                    p_damageInfo->type,
                    p_damageInfo->damage) ;
            }
        }
    }

    /* If neither, do nothing. */
    DebugEnd() ;

    return FALSE ;
}


/* LES: 05/21/96 Created */
/* Returns new door lock level */
/*     (0=unlocked, 0x100 means no change, other=unlocked more) */
T_word16 ServerLockDoorsInArea(
           T_sword32 x,
           T_sword32 y,
           T_word16 radius)
{
    T_word16 numSectors ;
    T_word16 sectors[20] ;
    T_word16 i ;
    T_word16 bestLevel = 0x100 ;
    T_word16 level ;
    T_word16 oldlevel ;

    DebugRoutine("ServerLockDoorsInArea") ;

    Collide3dGetSectorsInBox(
        x,
        y,
        radius,
        20,
        sectors,
        &numSectors) ;

    for (i=0; i<numSectors; i++)  {
        if (DoorIsAtSector(sectors[i]))  {
            oldlevel = DoorGetLockValue(sectors[i]) ;
            DoorIncreaseLock(sectors[i], 10) ;
            level = DoorGetLockValue(sectors[i]) ;
            if (((level > bestLevel) || (bestLevel == 0x100)) &&
                (level != oldlevel))
                bestLevel = level ;
        }
    }

    DebugEnd() ;

    return bestLevel ;
}

/* LES: 05/21/96 Created */
/* Returns new door lock level */
/*     (0=unlocked, 0x100 means no change, other=unlocked more) */
T_word16 ServerUnlockDoorsInArea(
           T_sword32 x,
           T_sword32 y,
           T_word16 radius)
{
    T_word16 numSectors ;
    T_word16 sectors[20] ;
    T_word16 i ;
    T_word16 bestLevel = 0x100 ;
    T_word16 level ;

    DebugRoutine("ServerUnlockDoorsInArea") ;

    Collide3dGetSectorsInBox(
        x,
        y,
        radius,
        20,
        sectors,
        &numSectors) ;

    for (i=0; i<numSectors; i++)  {
        if (DoorIsAtSector(sectors[i]))  {
            if (DoorIsLock(sectors[i]))  {
                DoorDecreaseLock(sectors[i], 10) ;
                level = DoorGetLockValue(sectors[i]) ;
                if (level < bestLevel)
                    bestLevel = level ;
            }
        }
    }

    DebugEnd() ;

    return bestLevel ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SERVERSH.C
 *-------------------------------------------------------------------------*/
