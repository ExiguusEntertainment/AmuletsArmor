/*-------------------------------------------------------------------------*
 * File:  CSYNCPCK.C
 *-------------------------------------------------------------------------*/
/**
 * When A&A was converted to the new communications system, this layer
 * of code was added underneath the current client/server system to allow
 * synchronized communications.
 *
 * @addtogroup CSYNCPCK
 * @brief Synchronized Communication Packet Handling
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_TRIG.H"
#include "ACTIVITY.H"
#include "AREASND.H"
#include "BANNER.H"
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CMDQUEUE.H"
#include "CONTROL.H"
#include "CRELOGIC.H"
#include "CSYNCPCK.H"
#include "DOOR.H"
#include "ESCMENU.H"
#include "EFX.H"
#include "GENERAL.H"
#include "MAP.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "PEOPHERE.H"
#include "PLAYER.H"
#include "SCHEDULE.H"
#include "SERVER.H"
#include "SERVERSH.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SPELLS.H"
#include "STATS.H"
#include "SYNCMEM.H"
#include "SYNCPACK.H"
#include "SYNCTIME.H"
#include "TICKER.H"

#define MAX_HISTORY_PACKETS  100
#define MAX_SYNC_PLAYERS     8

/* Number of players who started the game. */
static T_byte8 G_numPlayers = 1 ;

/* Number of players who actually currently playing */
static T_word16 G_numTruePlayers = 1 ;

static T_doubleLinkList G_waitingActionList = DOUBLE_LINK_LIST_BAD ;
static E_Boolean G_init = FALSE ;
static T_byte8 G_syncNumber = 0 ;
static T_word32 G_lastUpdate ;
static T_word32 G_lastTimeSyncSent = 0 ;

static E_Boolean G_haveLast = FALSE ;
static T_packetLong G_lastPacket ;

//#define COMPILE_OPTION_RECORD_DEMO
//#define COMPILE_OPTION_PLAY_DEMO

typedef struct {
    T_playerAction action ;
    T_word16 data[4] ;
} T_waitingSyncAction ;

#ifdef COMPILE_OPTION_RECORD_DEMO
T_file G_demoFile ;
#endif
#ifdef COMPILE_OPTION_PLAY_DEMO
T_file G_demoFile ;
#endif

T_byte8 G_syncAhead = 0 ;

static T_objMoveStruct G_lastGoodObjMove ;

/* The group of players in a game have a common group id stored below: */
/* NOTE: groupID of 0 means no group. */
static T_gameGroupID G_groupID ;
static E_Boolean G_isGroupIDInit = FALSE ;

/* Internal prototypes: */
static T_void IClientSyncDoPlayerAction(
                  T_3dObject *p_playerObj,
                  T_playerAction actionType,
                  T_word16 *p_actionData) ;

static T_void ClientSyncUpdateReceived(T_void) ;
/* stats */
T_word16 G_statSend = 0 ;
T_word16 G_statRecv = 0 ;

/* Keep a history of the last MAX_HISTORY_PACKETS packets */
static T_doubleLinkList G_packetHistory = DOUBLE_LINK_LIST_BAD ;

#define MAX_SYNC_AHEAD 3

/* Keep track of what packets have been received by other players. */
static T_doubleLinkList G_playerPacketArray[MAX_SYNC_PLAYERS] ;
static T_byte8 G_playerLastSyncNumArray[MAX_SYNC_PLAYERS] ;
static T_word32 G_playerPacketResyncTime[MAX_SYNC_PLAYERS] ;
static T_word32 G_playerSyncClock[MAX_SYNC_PLAYERS] ;
static E_Boolean G_playerLeft[MAX_SYNC_PLAYERS] ;
static E_Boolean G_playerRetransmitOccuring[MAX_SYNC_PLAYERS] ;
static T_objMoveStruct G_playerLastGoodPos[MAX_SYNC_PLAYERS] ;
static T_word16 G_lastAction[MAX_SYNC_PLAYERS] ;

/* Internal prototypes: */
static T_void IClientSyncDestroyPacketHistory(T_void) ;
static T_void IClientSyncAddPacketToHistory(T_packetLong *p_packet) ;

#ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
FILE *G_fp ;
#endif

static T_word16 G_lastGoodNextId = 0 ;
static T_word16 G_lastGoodNextIdSyncNum = 0 ;
static E_Boolean ICheckSyncIdHistory(T_word16 nextId, T_word16 syncNum) ;
static T_void ICheckCollides(T_void) ;

typedef struct {
    T_word16 nextId ;
    T_word16 syncNum ;
} T_syncObjectIdHistoryItem ;

#define MAX_OBJECT_ID_HISTORY 64
T_syncObjectIdHistoryItem G_syncObjectIdHistory[MAX_OBJECT_ID_HISTORY] ;
T_word16 G_syncObjectIdHistoryPos = 0 ;

T_void ClientSyncInit(T_void)
{
    T_word16 i ;

    DebugRoutine("ClientSyncInit") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    if (G_isGroupIDInit == FALSE)  {
        G_groupID = *DirectTalkGetNullBlankUniqueAddress() ;
        G_isGroupIDInit = TRUE ;
    }

    memset(G_syncObjectIdHistory, 0xFF, sizeof(G_syncObjectIdHistory)) ;
#ifdef COMPILE_OPTION_RECORD_DEMO
    G_demoFile = FileOpen("demo.dat", FILE_MODE_WRITE) ;
#endif
#ifdef COMPILE_OPTION_PLAY_DEMO
    G_demoFile = FileOpen("demo.dat", FILE_MODE_READ) ;
#endif
    /* Create a list to keep waiting actions. */
    G_waitingActionList = DoubleLinkListCreate() ;

    /* Set up the syncro number. */
    G_syncNumber = 0 ;
    G_lastUpdate = 0 ;
    G_lastTimeSyncSent = 0 ;

    G_packetHistory = DoubleLinkListCreate() ;
    DebugCheck(G_packetHistory != DOUBLE_LINK_LIST_BAD) ;

    /* Create the link list of other player packets. */
    for (i=0; i<MAX_SYNC_PLAYERS; i++)  {
        G_playerPacketArray[i] = DoubleLinkListCreate() ;
        /* Start at 255 so that it will roll over to 0 and be the */
        /* first sync number received by the other players. */
        G_playerLastSyncNumArray[i] = 255 ;
        G_playerPacketResyncTime[i] = 0 ;
        G_playerSyncClock[i] = 0 ;
    }

#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
    G_fp = fopen("csync.dat", "w") ;
#   endif

    DebugEnd() ;
}

T_void ClientSyncInitPlayersHere(T_void)
{
    T_word16 i ;

    DebugRoutine("ClientSyncInitPlayersHere") ;

    for (i=0; i<MAX_SYNC_PLAYERS; i++)  {
        G_playerLeft[i] = FALSE ;
        G_playerRetransmitOccuring[i] = FALSE ;
    }

    DebugEnd() ;
}


T_void ClientSyncFinish(T_void)
{
    T_word16 i ;

    DebugRoutine("ClientSyncFinish") ;
    DebugCheck(G_init == TRUE) ;

#ifdef COMPILE_OPTION_RECORD_DEMO
    FileClose(G_demoFile) ;
#endif
#ifdef COMPILE_OPTION_PLAY_DEMO
    FileClose(G_demoFile) ;
#endif
    /* Clear the waiting action list of any waiting actions. */
    DoubleLinkListFreeAndDestroy(&G_waitingActionList) ;

    G_init = FALSE ;

    IClientSyncDestroyPacketHistory() ;

    /* Clear out all the other player packets. */
    for (i=0; i<MAX_SYNC_PLAYERS; i++)
        /* Clear out the list of packets. */
        DoubleLinkListFreeAndDestroy(&G_playerPacketArray[i]) ;

#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
    fclose(G_fp) ;
#   endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSyncPacketEvaluate
 *-------------------------------------------------------------------------*/
/**
 *  ClientSyncPacketProcess processes a full sync packet that was received
 *  from the server.  This may or may not be refering to the player.
 *
 *  @param p_sync -- Syncronize packet to process
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSyncPacketEvaluate(T_syncronizePacket *p_sync)
{
    T_3dObject *p_playerObj ;

    T_syncPacketFieldsAvail fieldsAvailable ;
    T_sword16 x,  y, z ;
    T_word16 angle ;
    T_syncPacketStanceAndVis stanceAndVisibility ;
    T_playerAction actionType ;
    T_word16 *p_actionData ;
    T_byte8 *p_pos ;
    E_Boolean isPlayer ;
    T_byte8 stance ;
    T_byte8 visibility ;
    T_word16 sector ;
    T_word16 action ;
//    T_objMoveStruct oldObjMove ;
    T_sword16 lastX, lastY, lastZ ;
    T_word16 playerNum ;

    DebugRoutine("ClientSyncPacketEvaluate") ;
    DebugCheck(p_sync != NULL) ;

    if (ClientIsActive())  {
#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
    fprintf(G_fp, "%d #%3d d:%3d f:%02X p:%04d x:%04X y:%04X z:%04X a:%04X s:%02X at:%02X 0:%04X 1:%04X 2:%04X\n",
       SyncTimeGet(),
       p_sync->syncNumber,
       p_sync->deltaTime,
       p_sync->fieldsAvailable,
       p_sync->playerObjectId,
       p_sync->x,
       p_sync->y,
       p_sync->z,
       p_sync->angle,
       p_sync->stanceAndVisibility,
       p_sync->actionType,
       p_sync->actionData[0],
       p_sync->actionData[1],
       p_sync->actionData[2]) ;
    fflush(stdout) ;
#   endif

//printf("Process: sync number: %d\n", p_sync->syncNumber) ;
//printf("Process: sync delta: %d\n", p_sync->deltaTime) ;
#ifdef COMPILE_OPTION_RECORD_DEMO
    FileWrite(G_demoFile, p_sync, sizeof(T_syncronizePacket)) ;
#endif
#ifdef COMPILE_OPTION_PLAY_DEMO
    FileRead(G_demoFile, p_sync, sizeof(T_syncronizePacket)) ;
#endif
    /* Find the object that this packet is refering to. */
    p_playerObj = ObjectFind(p_sync->playerObjectId) ;

    /* Make sure the object exists before doing any actions related to it. */
#ifndef NDEBUG
    if (!p_playerObj)  {
        printf("Looking for player %d\n", p_sync->playerObjectId) ;
        printf("Player object id %d\n", ObjectGetServerId(PlayerGetObject())) ;
        fflush(stdout) ;
    }
#endif
    /* Create the player at our location if we cannot find him. */
    if (p_playerObj == NULL)  {
        p_playerObj = ServerCreateFakeObjectGlobal(
            510,
            PlayerGetX16(),
            PlayerGetY16(),
            PlayerGetZ16()) ;
        ObjectSetServerId(p_playerObj, p_sync->playerObjectId) ;
        ObjectAdd(p_playerObj) ;
    }

    DebugCheck(p_playerObj != NULL) ;
    if (p_playerObj)  {
        playerNum = ObjectGetServerId(p_playerObj) - 9000 ;

        /* Record the old location. */
        lastX = ObjectGetX16(p_playerObj) ;
        lastY = ObjectGetY16(p_playerObj) ;
        lastZ = ObjectGetZ16(p_playerObj) ;

        /* Is this the player or the object? */
        if (p_playerObj == PlayerGetObject())
            isPlayer = TRUE ;
        else
            isPlayer = FALSE ;

        /* Store the old location to see if by chance we ran into */
        /* something. */
//        oldObjMove = p_playerObj->objMove ;
        DebugCheck(playerNum < MAX_SYNC_PLAYERS) ;
        G_playerLastGoodPos[playerNum] = p_playerObj->objMove ;

        /* Break apart the packet. */
        fieldsAvailable = p_sync->fieldsAvailable ;

        p_pos = (T_byte8 *)(&p_sync->x) ;
        if (fieldsAvailable & SYNC_PACKET_FIELD_AVAIL_X)  {
            x = *((T_sword16 *)p_pos) ;
//printf("Found x = %d\n", x) ;
            p_pos += sizeof(T_sword16) ;
            ObjectSetX16(p_playerObj, x) ;
        }

        if (fieldsAvailable & SYNC_PACKET_FIELD_AVAIL_Y)  {
            y = *((T_sword16 *)p_pos) ;
            p_pos += sizeof(T_sword16) ;
            ObjectSetY16(p_playerObj, y) ;
        }

        if (fieldsAvailable & SYNC_PACKET_FIELD_AVAIL_Z)  {
            z = *((T_sword16 *)p_pos) ;
            p_pos += sizeof(T_sword16) ;
            ObjectSetZ16(p_playerObj, z) ;
        }

        if (fieldsAvailable & SYNC_PACKET_FIELD_ANGLE)  {
            angle = *((T_word16 *)p_pos) ;
            p_pos += sizeof(T_word16) ;

            /* Only set the angle if not in dead stance */
            if (ObjectGetStance(p_playerObj) != STANCE_DIE)
                ObjectSetAngle(p_playerObj, angle) ;
        }

        /* Set up the sectors for this objects -- if the object moved. */
        if (fieldsAvailable &
                (SYNC_PACKET_FIELD_AVAIL_X |
                 SYNC_PACKET_FIELD_AVAIL_Y |
                 SYNC_PACKET_FIELD_AVAIL_Z))  {
            z = ObjectGetZ16(p_playerObj) ;
            ObjectSetUpSectors(p_playerObj) ;
            ObjectSetZ16(p_playerObj, z) ;

            /* See if we or another player have stepped on */
            /* a pressure plate. */
            sector = ObjectGetAreaSector(p_playerObj) ;
            if (sector != ObjectGetLastSteppedOnSector(p_playerObj))  {
                ObjectSetLastSteppedOnSector(p_playerObj, sector) ;
                action = MapGetSectorAction(sector) ;

                /* Activate it (if it actually is something) */
                /* Don't do the action if it is 200 or higher or */
                /* it is this player. */
                if (action != 0)
                    if ((action < 200) || (isPlayer))
                        ActivitiesRun(action) ;
            }
#           ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
            fprintf(
                G_fp,
                "%d at %08lX %08lX %08lX\n",
                ObjectGetServerId(p_playerObj),
                ObjectGetX(p_playerObj),
                ObjectGetY(p_playerObj),
                ObjectGetZ(p_playerObj)) ;
#           endif
            /* Alert creatures near me of my body. */
            CreaturesHearSoundOfPlayer(p_playerObj, 50) ;
        }

        /* Change the stance accordingly as well. */
        if (fieldsAvailable & SYNC_PACKET_FIELD_STANCE)  {
            stanceAndVisibility = *p_pos ;
            p_pos++ ;
            stance = stanceAndVisibility &
                         SYNC_PACKET_STANCE_AND_VIS_STANCE_MASK ;
            visibility = stanceAndVisibility &
                         (~SYNC_PACKET_STANCE_AND_VIS_STANCE_MASK) ;

            if (!isPlayer)  {
                if (visibility & SYNC_PACKET_STANCE_AND_VIS_INVISIBLE)  {
                    ObjectAddAttributesToPiecewise(
                        p_playerObj,
                        OBJECT_ATTR_INVISIBLE) ;
                } else {
                    ObjectRemoveAttributesFromPiecewise(
                        p_playerObj,
                        OBJECT_ATTR_INVISIBLE) ;
                }

                if (visibility & SYNC_PACKET_STANCE_AND_VIS_TRANSLUCENT)  {
                    ObjectAddAttributesToPiecewise(
                        p_playerObj,
                        OBJECT_ATTR_TRANSLUCENT) ;
                } else  {
                    ObjectRemoveAttributesFromPiecewise(
                        p_playerObj,
                        OBJECT_ATTR_TRANSLUCENT) ;
                }
            }

            /* Are we feeling stealthy? */
            if (visibility & SYNC_PACKET_STANCE_AND_VIS_STEALTHY)  {
                /* yes. */
                ObjectMakeStealthy(p_playerObj) ;
            } else {
                /* Not stealthy. */
                ObjectMakeNotStealthy(p_playerObj) ;
            }

            if (visibility & SYNC_PACKET_STANCE_AND_VIS_TELEPORT)  {
                /* Old location teleport effect. */
                EfxCreate(
                    EFX_TELEPORT,
                    lastX<<16,
                    lastY<<16,
                    lastZ<<16,
                    1,
                    TRUE,
                    0) ;

                /* New location teleport effect */
                EfxCreate(
                    EFX_TELEPORT,
                    ObjectGetX(p_playerObj),
                    ObjectGetY(p_playerObj),
                    ObjectGetZ(p_playerObj),
                    1,
                    TRUE,
                    0) ;
            }
            /* Set the player stance */
//            if (!isPlayer)
            ObjectSetStance(p_playerObj, stance) ;
//printf("@@p:%d s:%d\n", ObjectGetServerId(p_playerObj), ObjectGetStance(p_playerObj)) ;
            /* Make sure passable when dead, impassible when not dead */
            if (stance == STANCE_DIE)  {
                ObjectMakePassable(p_playerObj) ;
            } else {
                ObjectMakeImpassable(p_playerObj) ;
            }

            if (stance == STANCE_ATTACK)
                CreaturesHearSoundOfPlayer(p_playerObj, 500) ;
        }

        if (fieldsAvailable & SYNC_PACKET_FIELD_ACTION)  {
            actionType = *((T_playerAction *)p_pos) ;
            p_pos += sizeof(T_playerAction) ;
            p_actionData = (T_word16 *)p_pos ;
            p_pos += 8 ;
            IClientSyncDoPlayerAction(
                p_playerObj,
                actionType,
                p_actionData) ;
        }

        SyncMemAdd("Player %d at %d %d\n", ObjectGetServerId(p_playerObj), x, y) ;
        SyncMemAdd("  vel %d %d %d\n", ObjectGetXVel(p_playerObj), ObjectGetYVel(p_playerObj), ObjectGetZVel(p_playerObj)) ;
        DebugCheck(playerNum < MAX_SYNC_PLAYERS) ;
        G_lastAction[playerNum] = actionType ;
    }


#ifdef COMPILE_OPTION_PLAY_DEMO
    PlayerFakeOverwriteCurrent() ;
#endif
    }
    DebugEnd() ;
}

static T_void ICheckCollides(T_void)
{
    T_word16 i ;
    T_word16 actionType ;
    T_3dObject *p_playerObj ;
    E_Boolean jumpBack[MAX_SYNC_PLAYERS] ;

    for (i=0; i<G_numPlayers; i++)  {
        jumpBack[i] = FALSE ;
        if (G_playerLeft[i] == FALSE)  {
            actionType = G_lastAction[i] ;
            /* Is this new position colliding with something? */
            if ((actionType != PLAYER_ACTION_LEAVE_LEVEL) &&
                (actionType != PLAYER_ACTION_ABORT_LEVEL))  {
                p_playerObj = ObjectFind(9000 + i) ;
                if (p_playerObj)  {
                    if (ObjectCheckIfCollide(
                            p_playerObj,
                            ObjectGetX(p_playerObj),
                            ObjectGetY(p_playerObj),
                            ObjectGetZ(p_playerObj))) {
                        SyncMemAdd("Player %d collide at %d %d\n",
                            ObjectGetServerId(p_playerObj),
                            ObjectGetX16(p_playerObj),
                            ObjectGetY16(p_playerObj)) ;
                        /* Note that we need to jump back. */
                        jumpBack[i] = TRUE ;
                    }
                }
            }
        }
    }

    for (i=0; i<G_numPlayers; i++)  {
        p_playerObj = ObjectFind(9000 + i) ;
        if (jumpBack[i] == TRUE)  {
            /* Move this back to the old location NOW! */
            if (p_playerObj)
                p_playerObj->objMove = G_playerLastGoodPos[i] ;
        }
        if (i == ClientGetLoginId())
            G_lastGoodObjMove = p_playerObj->objMove ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  IClientSyncDoPlayerAction
 *-------------------------------------------------------------------------*/
/**
 *  IClientSyncDoAction is called to make an object do the action that
 *  another player or this player did.
 *
 *  @param p_playerObj -- Pointer to player object
 *  @param actionType -- Type of action to perform
 *  @param p_actionData -- Pointer to 4 T_word16's that are used
 *      for the action performed.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IClientSyncDoPlayerAction(
                  T_3dObject *p_playerObj,
                  T_playerAction actionType,
                  T_word16 *p_actionData)
{
    E_Boolean isPlayer = FALSE ;
    T_3dObject *p_target ;
    T_word16 angle ;
    T_damageObjInfo damageInfo ;
    extern T_sword32 G_sourceX, G_sourceY, G_sourceZ ;
    T_word16 action ;
    E_wallActivation wallType ;
    T_word16 wallData ;
    T_3dObject *p_obj ;
    T_word16 player ;
    E_Boolean isTargetMe ;
    T_word16 item ;
    T_word16 locked ;

    DebugRoutine("IClientSyncDoPlayerAction") ;
    DebugCheck(p_playerObj != NULL) ;
    DebugCheck(p_actionData != NULL) ;
    DebugCheck(actionType < PLAYER_ACTION_UNKNOWN) ;

    if (p_playerObj == PlayerGetObject())
        isPlayer = TRUE ;

    switch(actionType)  {
        case PLAYER_ACTION_NONE:
            /* Nothing to do?  Do nothing */
            break ;
        case PLAYER_ACTION_CHANGE_SELF:
            /* Change self body part. */
            if (isPlayer)  {
                /* Do nothing, we've already done it. */
            } else {
//                if (ObjectGetStance(p_playerObj) != STANCE_DIE)  {
                    ObjectSetBodyPartType(
                        p_playerObj,
                        p_actionData[0],
                        p_actionData[1]) ;
//                }
            }
            break ;
        case PLAYER_ACTION_MELEE_ATTACK:
            /* Find the target object to hit */
            p_target = ObjectFind(p_actionData[2]) ;
            if (p_target)  {
                /* Identify the source of the damage. */
                G_sourceX = ObjectGetX16(p_playerObj) << 16 ;
                G_sourceY = ObjectGetY16(p_playerObj) << 16 ;
                G_sourceZ = ObjectGetZ16(p_playerObj) << 16 ;
                damageInfo.damage = p_actionData[0] /* amount of damage. */ ;
                damageInfo.type = (T_byte8)p_actionData[1] /* damage type */ ;
                damageInfo.ownerID = ObjectGetServerId(p_playerObj) ;
                ServerDamageObjectXYZ(
                    p_target,
                    (T_word32)(&damageInfo)) ;
            }
            break ;
        case PLAYER_ACTION_MISSILE_ATTACK:
            /* Determine if we have a target and what angle to target. */
            if (p_actionData[1] != 0)  {
                p_target = ObjectFind(p_actionData[1]) ;
                if (p_target)  {
                    angle =
                        MathArcTangent(
                            ObjectGetX16(p_target) - ObjectGetX16(p_playerObj),
                            ObjectGetY16(p_target) - ObjectGetY16(p_playerObj)) ;
                } else {
                    angle = ObjectGetAngle(p_playerObj) ;
                }
            } else {
                p_target = NULL ;
                angle = ObjectGetAngle(p_playerObj) ;
            }

            /* Shoot a missile to the given target. */
            ServerShootProjectile(
                p_playerObj,
                angle,
                p_actionData[0], /* type of missile */
                50, /* initial speed */
                (p_actionData[1] != 0) ? ObjectFind(p_actionData[1]) : NULL) ;
            ObjectSetStance(p_playerObj, STANCE_ATTACK);

            /* Alert creatures of my attack. */
            CreaturesHearSoundOfPlayer(p_playerObj, 1500) ;
            break ;
        case PLAYER_ACTION_ACTIVATE_FORWARD:
            MapGetForwardWallActivationType(
                p_playerObj,
                &wallType,
                &wallData) ;
            if (wallType == WALL_ACTIVATION_SCRIPT)  {
                action = wallData ;
                if ((action != 0) && (action < 256))
                    ActivitiesRun(action) ;
            } else if (wallType == WALL_ACTIVATION_DOOR)  {
                /* Force the forward wall to open. */
                MapForceOpenForwardWall(p_playerObj) ;
            }
            ObjectSetStance(p_playerObj, STANCE_ATTACK);

            /* Alert creatures of my action. */
            CreaturesHearSoundOfPlayer(p_playerObj, 500) ;
            break ;
        case PLAYER_ACTION_LEAVE_LEVEL:
            /* Remove that other player.  He is no longer here. */
            if (!isPlayer)  {
                ObjectRemove(p_playerObj) ;
                ObjectDestroy(p_playerObj) ;
            } else {
                /* Add in the time for the clock to the time of day. */
                MapSetDayOffset(MapGetDayOffset() + SyncTimeGet()) ;
            }

            /* Note that we are about to leave. */
            ClientSetNextPlace(p_actionData[0], 0) ;
            break ;
        case PLAYER_ACTION_PICKUP_ITEM:
            /* Find the target item. */
            p_target = ObjectFind(p_actionData[0] /* item id */) ;

            /* Are we taking the item or someone else? */
            if (isPlayer)  {
                /* We are taking it. */
                /* Does the item exist? */
                if (p_target)  {
                    /* Item exists.  Take and put it in the inventory. */
                    if (InventoryTakeItemFromWorld(p_target, p_actionData[1]?TRUE:FALSE))  {
//                        MessageAdd("Item taken.") ;
                    }
                } else {
                    /* Item no longer exists.  Someone else must of */
                    /* swiped it. */
                    MessageAdd("Object has already been taken.");
                }
            } else {
                /* Someone else is taking the item. */
                /* All we care is that the object no longer exists. */
                if (p_target)  {
                    ObjectRemove(p_target) ;
                    ObjectDestroy(p_target) ;
                }
            }
            break ;
        case PLAYER_ACTION_THROW_ITEM:
            p_obj = ServerShootProjectile(
                        p_playerObj,
                        p_actionData[1], /* angle */
                        p_actionData[0], /* type of object */
                        p_actionData[2], /* throw speed */
                        NULL) ;          /* no aimed target. */
            ObjectSetStance(p_playerObj, STANCE_ATTACK);
            ObjectSetAccData(p_obj, p_actionData[3]) ;
            break ;
        case PLAYER_ACTION_STEAL:
            /* Who is being stolen from. */
            p_target = ObjectFind(p_actionData[0] /* victum id */) ;
            if (p_target == PlayerGetObject())
                isTargetMe = TRUE ;
            else
                isTargetMe = FALSE ;

            if (isPlayer)  {
                /* I'm doing the stealing. */
                if (isTargetMe == TRUE)  {
                    /* How am I stealing from myself? */
                    DebugCheck(FALSE) ;
                } else {
                    /* Stealing from another.  Is it a creature */
                    /* or another player? */
                    if (ObjectIsCreature(p_target))  {
                        if (p_actionData[2] == 0)  {
                            /* Just take it from the creature. */
                            item = CreatureStolenFrom(p_target) ;
                            if (item != 0)  {
                                SoundDing() ;
                                MessagePrintf("Stole item!", item) ;
                                if (InventoryCreateObjectInHand(item) == FALSE)  {
                                    MessageAdd("Stolen item dropped.") ;
                                    ClientSyncSendActionDropAt(
                                        item,
                                        ObjectGetX16(p_playerObj),
                                        ObjectGetY16(p_playerObj),
                                        0) ;
                                }
                            } else {
                                SoundDing() ;
                                MessageAdd("Nothing to steal!") ;
                            }
                        } else {
                            /* Alert creatures of my attempt. */
                            CreaturesHearSoundOfPlayer(p_playerObj, 100) ;
                        }
                    } else {
                        /* We'll have to wait for that player to */
                        /* "give" us his object. */
                    }
                }
            } else {
                /* Somebody else is doing the stealing. */
                if (isTargetMe == TRUE)  {
                    /* I'm being stolen from. */
                    /* Give something back at random. */
                    if (p_actionData[2] /* notifyOwner */!= 0)  {
                        /* Notify me if the robber failed badly. */
                        SoundDing() ;
                        MessageAdd("^026Somebody failed to steal from you!") ;
                    } else {
                        /* Robber did it correctly. */
                        item = InventoryDestroyRandomStoredItem();
                        ClientSyncSendActionStolen(
                            item,
                            p_actionData[1] /* stealer */) ;
                    }
                } else {
                    /* Someone else is stealing somebody else. */
                    /* I only care if this is a creature being */
                    /* stolen so he drops only the correct treausre. */
                    if (ObjectIsCreature(p_target))
                        CreatureStolenFrom(p_target) ;
                }
            }
            break ;
        case PLAYER_ACTION_STOLEN:
            /* Not currently implemented. */
            if (ObjectGetServerId(PlayerGetObject()) ==
                    p_actionData[1] /* stealer */)  {
                /* This refers to me, I did the stealing, */
                /* I get the reward. */
                if (p_actionData[0] != 0)  {
                    SoundDing() ;
                    MessagePrintf("Stole item!") ;
                    if (InventoryCreateObjectInHand(p_actionData[0]) == FALSE)  {
                        MessageAdd("Stolen item dropped.") ;
                        ClientSyncSendActionDropAt(
                            p_actionData[0],
                            ObjectGetX16(p_playerObj),
                            ObjectGetY16(p_playerObj),
                            0) ;
                    }
                } else {
                    SoundDing() ;
                    MessageAdd("Nothing to steal!") ;
                }
            }
            break ;
        case PLAYER_ACTION_PICK_LOCK:
            if (DoorIsAtSector(p_actionData[0] /* sector */))  {
                locked = DoorGetLockValue(p_actionData[0] /* door sector */) ;
                if (locked == DOOR_ABSOLUTE_LOCK)  {
                    /* Door is unlocked no matter what, miracle move. */
                    DoorUnlock(p_actionData[0]) ;

                    /* Report the fact to the thief. */
                    if (ObjectGetServerId(PlayerGetObject()) ==
                         p_actionData[1] /* picker ID */)  {
                        SoundDing() ;
                        MessageAdd("Door jimmied open without key!") ;
                    }
                } else {
                    /* Door was picked, decrease lock on it by 10. */
                    DoorDecreaseLock(p_actionData[0], 10) ;

                    /* Report the fact to the thief. */
                    if (ObjectGetServerId(PlayerGetObject()) ==
                         p_actionData[1] /* picker ID */)  {
                        if (DoorGetLockValue(p_actionData[0]) != 0)  {
                            MessageAdd("Lock on door is decreased") ;
                        } else {
                            SoundDing() ;
                            MessageAdd("Door is now unlocked") ;
                        }
                    }
                }
            }
            break ;
        case PLAYER_ACTION_AREA_SOUND:
            /* Play an area sound if it is not this player or the */
            /* sound is not a voicing. */
            if ((!isPlayer) || (p_actionData[2] /* isVoicing */ == 0))  {
                AreaSoundCreate(
                       ObjectGetX16(p_playerObj),
                       ObjectGetY16(p_playerObj),
                       p_actionData[1], /* radius */
                       AREA_SOUND_MAX_VOLUME,
                       AREA_SOUND_TYPE_ONCE,
                       20,
                       AREA_SOUND_BAD,
                       NULL,
                       0,
                       p_actionData[0] /* sound */) ;
            } else {
                /* Otherwise, play it locally and make sure no other */
                /* sounds are being heard. */
                PlayerMakeSoundLocal(p_actionData[0]) ;
            }

            /* Alert creatures of my sound. */
            CreaturesHearSoundOfPlayer(p_playerObj, 500) ;
            break ;
        case PLAYER_ACTION_SYNC_NEW_PLAYER_WAIT:
            /* Not currently implemented. */
            break ;
        case PLAYER_ACTION_SYNC_NEW_PLAYER_SEND:
            /* Not currently implemented. */
            break ;
        case PLAYER_ACTION_SYNC_COMPLETE:
            /* Not currently implemented. */
            break ;
        case PLAYER_ACTION_GOTO_PLACE:
            ClientSetNextPlace(p_actionData[0], 0) ;
            break ;
        case PLAYER_ACTION_DROP_AT:
            /* Drop a new object of the given type at the given */
            /* location. */
            p_obj = ObjectCreate() ;
            DebugCheck(p_obj != NULL) ;
            if (p_obj)  {
                ObjectSetType(p_obj, p_actionData[0] /* obj type */) ;
                ObjectSetX16(p_obj, p_actionData[1] /* x */) ;
                ObjectSetY16(p_obj, p_actionData[2] /* y */) ;
                ObjectAdd(p_obj) ;
                ObjectSetUpSectors(p_obj) ;

                /* Force object onto floor. */
                ObjectSetZ16(p_obj,
                    MapGetFloorHeight(ObjectGetAreaSector(p_obj))) ;
                ObjectSetAccData(p_obj, p_actionData[3] /* accData */) ;
            }
            break ;
        case PLAYER_ACTION_PAUSE_GAME_TOGGLE:
            ClientTogglePause() ;
            break ;
        case PLAYER_ACTION_ABORT_LEVEL:
            if (isPlayer)  {
                /* Close the escape menu */
                if (EscapeMenuIsOpen())
                    EscapeMenuClose();

                /* Close out the banner to keep confusion down. */
                if (BannerIsOpen())
                    BannerCloseForm() ;

                /* Reload this character. */
                StatsLoadCharacter(StatsGetActive()) ;

                /* Redraw everything on the screen that may be open. */
                InventoryDrawReadyArea();

                /* Set up the pointer correctly. */
                ControlPointerReset() ;

                /* Remove any spells in progress .. may not have the runes */
                SpellsBackspace(NULL);

                /* Fix the buttons */
                BannerFinish() ;
                BannerInit() ;
                BannerUpdate() ;

                /* Make sure the status bar is correct. */
                BannerStatusBarUpdate() ;

                /* Make sure to save this character. */
//                StatsSaveCharacter(StatsGetActive()) ;
                /* Declare this as no longer an adventure */
                ClientSetAdventureNumber(0) ;

                MouseRelativeModeOff();

                /* Jump out of the system. */
                ClientSetNextPlace(20004, 0) ;

                /* Add in the time for the clock to the time of day. */
                MapSetDayOffset(MapGetDayOffset() + SyncTimeGet()) ;
            } else {
                /* Somebody is leaving. */
                player = ObjectGetServerId(p_playerObj) - 9000 ;
                DebugCheck(player < MAX_SYNC_PLAYERS) ;
                /* Make sure player is around before we remove him */
                if (G_playerLeft[player] == FALSE)  {
                    G_numTruePlayers-- ;
                    G_playerLeft[player] = TRUE ;
                    MessagePrintf("Player left the game.") ;
                    ObjectRemove(p_playerObj) ;
                    ObjectDestroy(p_playerObj) ;
                }
            }
            break ;
        case PLAYER_ACTION_ID_SELF:
            /* Append this information to the player data names. */
            PeopleHereIDPlayer(
                ObjectGetServerId(p_playerObj) - 9000,
                (T_byte8 *)p_actionData) ;
            break ;
    }


    DebugEnd() ;
}

static T_void IClientSyncSendAction(
                  T_playerAction action,
                  T_word16 data1,
                  T_word16 data2,
                  T_word16 data3,
                  T_word16 data4)
{
    T_waitingSyncAction *p_action ;

    DebugRoutine("IClientSyncSendAction") ;
    DebugCheck(action < PLAYER_ACTION_UNKNOWN) ;
//    DebugCheck(G_init == TRUE) ;

    /* Only do these commands if the system is up and running. */
    if (G_init == TRUE)  {
        DebugCheck(G_waitingActionList != NULL) ;
        /* For now, just queue up the request.  When we go to send a sync */
        /* packet, we will pull out items from this list. */
        p_action = MemAlloc(sizeof(T_waitingSyncAction)) ;
        DebugCheck(p_action != NULL) ;
        if (p_action != NULL)  {
            p_action->action = action ;
            p_action->data[0] = data1 ;
            p_action->data[1] = data2 ;
            p_action->data[2] = data3 ;
            p_action->data[3] = data4 ;
            DoubleLinkListAddElementAtEnd(G_waitingActionList, p_action) ;
        }
    }

    DebugEnd() ;
}

T_void ClientSyncSendActionChangeSelf(
           T_bodyPartLocation location,
           T_word16 newPart)
{
    DebugRoutine("ClientSyncSendActionChangeSelf") ;

    IClientSyncSendAction(
        PLAYER_ACTION_CHANGE_SELF,
        (T_word16)location,
        newPart,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionMeleeAttack(
           T_word16 damageAmount,
           E_effectDamageType damageType,
           T_word16 targetId)
{
    DebugRoutine("ClientSyncSendActionMeleeAttack") ;

    IClientSyncSendAction(
        PLAYER_ACTION_MELEE_ATTACK,
        damageAmount,
        (T_word16)damageType,
        targetId,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionMissileAttack(
           T_word16 missileType,
           T_word16 target)
{
    DebugRoutine("ClientSyncSendActionMissileAttack") ;

    IClientSyncSendAction(
        PLAYER_ACTION_MISSILE_ATTACK,
        missileType,
        target,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionActivateForward(T_void)
{
    DebugRoutine("ClientSyncSendActionActivateForward") ;

    IClientSyncSendAction(
        PLAYER_ACTION_ACTIVATE_FORWARD,
        0,
        0,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionPickLock(
           T_word16 doorSector,
           T_word16 pickerID)
{
    DebugRoutine("ClientSyncSendActionPickLock") ;

    IClientSyncSendAction(
        PLAYER_ACTION_PICK_LOCK,
        doorSector,
        pickerID,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionDropAt(
           T_word16 objectType,
           T_sword16 x,
           T_sword16 y,
           T_word16 accData)
{
    DebugRoutine("ClientSyncSendActionDropAt") ;

    IClientSyncSendAction(
        PLAYER_ACTION_DROP_AT,
        objectType,
        x,
        y,
        accData) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionLeaveLevel(
       T_word16 newLevel)
{
    DebugRoutine("ClientSyncSendActionLeaveLevel") ;

    IClientSyncSendAction(
        PLAYER_ACTION_LEAVE_LEVEL,
        newLevel,
        0,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionPickUpItem(
           T_word16 itemID,
           E_Boolean autoStore)
{
    DebugRoutine("ClientSyncSendActionPickUpItem") ;

    IClientSyncSendAction(
        PLAYER_ACTION_PICKUP_ITEM,
        itemID,
        autoStore,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionThrowItem(
           T_word16 itemType,
           T_word16 angle,
           T_word16 throwSpeed,
           T_word16 objectAccData)
{
    DebugRoutine("ClientSyncSendActionThrowItem") ;

    IClientSyncSendAction(
        PLAYER_ACTION_THROW_ITEM,
        itemType,
        angle,
        throwSpeed,
        objectAccData) ;

    DebugEnd() ;
}

T_void ClientSyncSendGotoPlace(T_word16 newPlace)
{
    DebugRoutine("ClientSyncSendGotoPlace") ;

    IClientSyncSendAction(
        PLAYER_ACTION_GOTO_PLACE,
        newPlace,
        0,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionSteal(
           T_word16 objectID,
           T_word16 stealerID,
           E_Boolean notifyOwner)
{
    DebugRoutine("ClientSyncSendActionSteal") ;

    IClientSyncSendAction(
        PLAYER_ACTION_STEAL,
        objectID,
        stealerID,
        (notifyOwner==TRUE)?1:0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionStolen(
           T_word16 objectType,
           T_word16 stealerID)
{
    DebugRoutine("ClientSyncSendActionStolen") ;

    IClientSyncSendAction(
        PLAYER_ACTION_STOLEN,
        objectType,
        stealerID,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionAreaSound(
           T_word16 sound,
           T_word16 radius,
           E_Boolean isVoicing)
{
    DebugRoutine("ClientSyncSendActionAreaSound") ;

    IClientSyncSendAction(
        PLAYER_ACTION_AREA_SOUND,
        sound,
        radius,
        (isVoicing == TRUE)?1:0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionPauseGameToggle(T_void)
{
    DebugRoutine("ClientSyncSendActionPauseGameToggle") ;

    IClientSyncSendAction(
        PLAYER_ACTION_PAUSE_GAME_TOGGLE,
        0,
        0,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncSendActionAbortLevel(T_void)
{
    DebugRoutine("ClientSyncSendActionAbortLevel") ;

    IClientSyncSendAction(
        PLAYER_ACTION_ABORT_LEVEL,
        0,
        0,
        0,
        0) ;

    DebugEnd() ;
}

T_void ClientSyncUpdate(T_void)
{
    T_doubleLinkListElement element ;
    T_syncronizePacket *p_syncro ;
    T_syncPacket *p_sync ;
    T_packetLong packet ;
    T_word32 time ;
    T_waitingSyncAction *p_action ;
    static T_word32 lastTime = 0 ;
//    static T_word16 G_maxAllowed = 1 ;
    T_word16 i ;
    T_byte8 player ;

    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("ClientSyncUpdate") ;
    DebugCheck(G_init == TRUE) ;

    time = TickerGet() ;

    /* Update any retransmit requests. */
    if (G_numTruePlayers != 1)   {
        for (player=0; player<G_numPlayers; player++)  {
            if (G_playerRetransmitOccuring[player] == TRUE)  {
                if (G_playerLeft[player] == FALSE)  {
                    if ((time - G_playerPacketResyncTime[player]) > ((T_word32)(2*TICKS_PER_SECOND)))  {
//printf("Again, Requesting retrans at %d", G_playerLastSyncNumArray[player]+1) ;
                        if (G_playerLeft[player] == FALSE)  {
                            ClientRequestRetransmit(
                                player,
                                G_playerLastSyncNumArray[player]+1,
                                ClientSyncGetGameGroupID(),
                                player) ;
                        }
                        G_playerPacketResyncTime[player] = TickerGet() ;
                    }
                }
            }
        }
    }

    if (G_syncAhead >= MAX_SYNC_AHEAD)  {
        /* Repeat last packet, but do it slowly. */
        if ((time - lastTime) >= 15)  {
            packet = G_lastPacket ;
//            lastTime += 70 ;
            lastTime = time ;
//puts("Sending resend") ;
/*
            CmdQSetActivePortNum(0) ;
            CmdQSendPacket(
               (T_packetEitherShortOrLong *)&packet,
               140,
               0,
               NULL) ;
*/
            /* Resend packet to all our friends. */
            for (i=0; i<G_numPlayers; i++)  {
                if ((ObjectGetServerId(PlayerGetObject())-9000) != i)  {
                    if (G_playerLeft[i] == FALSE)  {
                        DirectTalkSetDestination(PeopleHereGetUniqueAddr(i)) ;
                        PacketSend((T_packetEitherShortOrLong *)(&packet)) ;
                    }
                }
            }
        }
//        if (G_maxAllowed > 1)
//            G_maxAllowed-- ;
    } else {
        if (((G_syncAhead < MAX_SYNC_AHEAD)
/*             && ((time-lastTime) >= 5) */
            ) ||
             (G_numTruePlayers == 1) ||
             (lastTime == 0))  {
//            if ((G_syncAhead == 0) && (G_maxAllowed < 20))
//                G_maxAllowed++ ;

            PlayerSetRealMode() ;

            p_sync = (T_syncPacket *)(&packet.data) ;
            p_sync->groupID = G_groupID ;
            p_syncro = (T_syncronizePacket *)(p_sync->syncData) ;

            p_syncro->syncNumber = G_syncNumber++ ;

            p_syncro->deltaTime = time - G_lastUpdate ;
//            p_syncro->deltaTime = 3 ;
            G_lastUpdate = time ;

            p_syncro->fieldsAvailable =
                SYNC_PACKET_FIELD_AVAIL_X |
                SYNC_PACKET_FIELD_AVAIL_Y |
                SYNC_PACKET_FIELD_AVAIL_Z |
                SYNC_PACKET_FIELD_ANGLE   |
                SYNC_PACKET_FIELD_STANCE  ;

            p_syncro->playerObjectId = ObjectGetServerId(PlayerGetObject())-100 ;
            p_syncro->x = PlayerGetX16() ;
            p_syncro->y = PlayerGetY16() ;
        //printf("x=%d, y=%d\n", p_syncro->x, p_syncro->y) ;
            p_syncro->z = PlayerGetZ16() ;
            p_syncro->angle = PlayerGetAngle() + ControlGetLookAngle() ;

            /* Update translucency. */
            p_syncro->stanceAndVisibility = (T_syncPacketStanceAndVis)PlayerGetStance() ;
            if (EffectPlayerEffectIsActive(PLAYER_EFFECT_TRANSLUCENT))
                p_syncro->stanceAndVisibility |=
                   SYNC_PACKET_STANCE_AND_VIS_TRANSLUCENT ;

            /* Update invisibility. */
            if (EffectPlayerEffectIsActive(PLAYER_EFFECT_INVISIBLE))
                p_syncro->stanceAndVisibility |=
                   SYNC_PACKET_STANCE_AND_VIS_INVISIBLE ;

            if (PlayerIsStealthy())  {
                p_syncro->stanceAndVisibility |=
                   SYNC_PACKET_STANCE_AND_VIS_STEALTHY ;
            }

            /* Note if we just teleported so that everyone */
            /* has the same effect. */
            if (PlayerJustTeleported())
                p_syncro->stanceAndVisibility |=
                    SYNC_PACKET_STANCE_AND_VIS_TELEPORT ;

            element = DoubleLinkListGetFirst(G_waitingActionList) ;
            if ((PlayerGetObject() != NULL) &&
                (ObjectGetServerId(PlayerGetObject()) != 0) &&
                (element != DOUBLE_LINK_LIST_ELEMENT_BAD))  {
                p_syncro->fieldsAvailable |= SYNC_PACKET_FIELD_ACTION ;
                p_action = (T_waitingSyncAction *)
                                 DoubleLinkListElementGetData(element) ;
                DebugCheck(p_action != NULL) ;
                p_syncro->actionType = p_action->action ;
                p_syncro->actionData[0] = p_action->data[0] ;
                p_syncro->actionData[1] = p_action->data[1] ;
                p_syncro->actionData[2] = p_action->data[2] ;
                p_syncro->actionData[3] = p_action->data[3] ;

                /* Get rid of this action.  It is now out the door. */
                MemFree(p_action) ;
                DoubleLinkListRemoveElement(element) ;
            }

#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
            p_syncro->nextObjectId = G_lastGoodNextId ;
            p_syncro->nextObjectIdWhen = G_lastGoodNextIdSyncNum ;
#endif
            packet.header.packetLength =
               sizeof(T_syncPacket) +
               sizeof(T_syncronizePacket) +
               sizeof(T_syncronizePacket) ;
            packet.data[0] = PACKET_COMMAND_SYNC ;
            if (G_haveLast == TRUE)
                p_syncro[1] = *((T_syncronizePacket *)
                            (((T_syncPacket *)G_lastPacket.data)->syncData)) ;
            else
                p_syncro[1] = p_syncro[0] ;
            G_syncAhead++ ;

            /* NOTE:  MUST set to Fake mode before doing */
            /*        ClientReceiveSyncPacket */
            PlayerSetFakeMode() ;

//puts("Sending sync packet") ;
            if (G_numTruePlayers != 1)  {
/*
                CmdQSetActivePortNum(0) ;
                CmdQSendPacket(
                   (T_packetEitherShortOrLong *)&packet,
                   140,
                   0,
                   NULL) ;
*/
                /* Send packet to all our friends. */
                for (i=0; i<G_numPlayers; i++)  {
                    if ((ObjectGetServerId(PlayerGetObject())-9000) != i)  {
                        if (G_playerLeft[i] == FALSE)  {
    //printf("  to %d\n", i) ;
                            DirectTalkSetDestination(PeopleHereGetUniqueAddr(i)) ;
                            PacketSend((T_packetEitherShortOrLong *)(&packet)) ;
                        }
                    }
                }
            } else {
                /* If only one player playing, force this packet */
                /* to come right back.  This is purely an optimization */
                /* to not cause any hardware to be worked out. */
                /* NOTE:  Must be in fake mode */
                ClientReceiveSyncPacket((T_packetEitherShortOrLong *)&packet) ;
            }

            IClientSyncAddPacketToHistory(&packet) ;
            G_statSend++ ;
            G_lastPacket = packet ;
            G_haveLast = TRUE ;

            /* Send the packet to ourselves. */
//puts("Process self"); fflush(stdout) ;
            ClientSyncPacketProcess(p_syncro) ;

            G_lastTimeSyncSent = TickerGet() ;
            lastTime = time ;
        }
    }
//MessagePrintf("sync ahead %d %d %d %d", G_syncAhead, G_statSend, G_statRecv, G_maxAllowed) ;
    TICKER_TIME_ROUTINE_ENDM("ClientSyncUpdate", 500) ;

    DebugEnd() ;
}

T_void ClientSyncEnsureSend(T_void)
{
    TICKER_TIME_ROUTINE_PREPARE() ;
    DebugRoutine("ClientSyncEnsureSend") ;

    TICKER_TIME_ROUTINE_START() ;
    /* Only do client syncing if the sync is turned on. */
    if (G_init == TRUE)  {
        ClientSyncUpdateReceived() ;
        ClientSyncUpdate() ;
        CreaturesCheck() ;
    }
    TICKER_TIME_ROUTINE_ENDM("ClientSyncEnsureSend", 500) ;

    DebugEnd() ;
}

/* LES  06/17/96  Created */
/* Adds a packet to the end of the history and removes packets if */
/* the history is too long. */
static T_void IClientSyncAddPacketToHistory(T_packetLong *p_packet)
{
    T_packetLong *p_newHistoryPacket ;
    T_doubleLinkListElement frontElement ;
    T_void *p_oldPacket ;

    DebugRoutine("IClientSyncAddPacketToHistory") ;

    /* Make a copy of the given packet so we can store it */
    /* in the linked list. */
    p_newHistoryPacket = MemAlloc(sizeof(T_packetLong)) ;
    *p_newHistoryPacket = *p_packet ;

    /* Put the new packet into the history */
    DoubleLinkListAddElementAtEnd(G_packetHistory, p_newHistoryPacket) ;

    /* Check if we have gone over the limit of packets in the history. */
    while (DoubleLinkListGetNumberElements(G_packetHistory) >
            MAX_HISTORY_PACKETS)  {
        /* Too many packets in history.  Remove one. */
        frontElement = DoubleLinkListGetFirst(G_packetHistory) ;
        DebugCheck(frontElement != DOUBLE_LINK_LIST_ELEMENT_BAD) ;
        p_oldPacket = DoubleLinkListRemoveElement(frontElement) ;
        /* Get rid of the attached packet. */
        MemFree(p_oldPacket) ;
    }

    DebugEnd() ;
}

/* LES  06/17/96  Created */
/* Destroys the packet history. */
static T_void IClientSyncDestroyPacketHistory(T_void)
{
    DebugRoutine("IClientSyncDestroyPacketHistory") ;
    DebugCheck(G_packetHistory != DOUBLE_LINK_LIST_BAD) ;

    /* Free all the old history packets. */
    DoubleLinkListFreeAndDestroy(&G_packetHistory) ;

    DebugEnd() ;
}

/* LES: 06/17/06  Created */
/* The client has just received a sync packet.  We need to put it */
/* into the right list.  Also check if the sync data is correct. */
T_void ClientSyncPacketProcess(T_syncronizePacket *p_sync)
{
    T_word16 player ;
    T_syncronizePacket *p_syncCopy ;
    T_word16 syncNum ;
    T_word16 diffSync ;
    T_word16 lastSync ;

    DebugRoutine("ClientSyncPacketProcess") ;
    DebugCheck(p_sync != NULL) ;

    if (G_init)  {
        /* Get the player id */
//printf("playerObjectId: %d\n", p_sync->playerObjectId) ;
        player = p_sync->playerObjectId - 9000 ;
#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
//        fprintf(G_fp, "Packet process from %d at %ld  ", player, SyncTimeGet()) ;
#   endif

        DebugCheck(player < MAX_SYNC_PLAYERS) ;
        if (player < MAX_SYNC_PLAYERS)  {
            syncNum = p_sync->syncNumber ;
            lastSync = G_playerLastSyncNumArray[player] ;

#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
//        fprintf(G_fp, "(syncNum = %d, lastSync = %d)", syncNum, lastSync) ;
#   endif
//            printf("p%d: (syncNum = %d, lastSync = %d)", player, syncNum, lastSync) ;

            /* Calculate how off the sync count is. */
            if (syncNum >= lastSync)  {
                diffSync = syncNum - lastSync ;
            } else {
                diffSync = 256+syncNum - lastSync ;
            }

#       ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
//            fprintf(G_fp, "-> diffSync = %d ... ", diffSync) ;
#       endif
//            printf("-> diffSync = %d ... ", diffSync) ;

            /* Check if we have lost a packet. */
            if (diffSync == 1)  {
                /* No packet has been lost.  Add this one. */
                G_playerLastSyncNumArray[player]++ ;

                /* Copy this packet. */
                p_syncCopy = MemAlloc(sizeof(T_syncronizePacket)) ;
                *p_syncCopy = *p_sync ;

//    printf("add pack - %d\n", p_syncCopy->syncNumber) ;
                /* Put this packet on the list. */
                DoubleLinkListAddElementAtEnd(G_playerPacketArray[player], p_syncCopy) ;

                /* We got a good packet, so make sure not to continue resyncing. */
                G_playerPacketResyncTime[player] = TickerGet() ;

                /* Got what we needed, no retransmits needed. */
                G_playerRetransmitOccuring[player] = FALSE ;
            } else if (diffSync == 2) {
                /* We have lost one packet.  Ok.  We can recover from that. */
                /* Fix the missing packet. */
                G_playerLastSyncNumArray[player]++ ;

                /* Copy this packet. */
                p_syncCopy = MemAlloc(sizeof(T_syncronizePacket)) ;
                *p_syncCopy = p_sync[1] ;

//    printf("add pack A %d\n", p_syncCopy->syncNumber) ;
                /* Put this packet on the list. */
                DoubleLinkListAddElementAtEnd(G_playerPacketArray[player], p_syncCopy) ;

                /* Do the normal packet now. */
                G_playerLastSyncNumArray[player]++ ;

                /* Copy this packet. */
                p_syncCopy = MemAlloc(sizeof(T_syncronizePacket)) ;
                *p_syncCopy = *p_sync ;

//    printf("add pack B %d\n", p_syncCopy->syncNumber) ;
                /* Put this packet on the list. */
                DoubleLinkListAddElementAtEnd(G_playerPacketArray[player], p_syncCopy) ;

                /* We got a good packet, so make sure not to continue resyncing. */
                G_playerPacketResyncTime[player] = TickerGet() ;

                /* Got what we needed, no retransmits needed. */
                G_playerRetransmitOccuring[player] = FALSE ;
            } else if ((diffSync == 0) || (diffSync > 50)) {
                /* Either we got a repeat of a previous packet, or */
                /* Someone is retransmitting old packets.  If so, ignore. */
                /* Ignore. */
//    puts("ignore") ;
//    MessageAdd("ignore") ;
            } else {
                /* Hmmm ... we are really messed up now.  More than 1 packet */
                /* was lost and I don't think it is retransmits. */
                /* Send out a request to retransmit. */
                /* But don't send another if we are still resyncing. */
                if ((TickerGet() - G_playerPacketResyncTime[player]) >
                       ((T_word32)TICKS_PER_SECOND/4))  {
//                    CmdQSetActivePortNum(0) ;
//    printf("Requesting retrans at %d\n", G_playerLastSyncNumArray[player]+1) ;
//    MessagePrintf("Requesting retrans at %d", G_playerLastSyncNumArray[player]+1) ;
                    /* Resend packet to all our friends. */
//                    for (i=0; i<G_numPlayers; i++)  {
                        if (G_playerLeft[player] == FALSE)  {
//                            if (G_playerRetransmitOccuring[player] == FALSE)  {
                                ClientRequestRetransmit(
                                    (T_byte8)player,
                                    G_playerLastSyncNumArray[player]+1,
                                    ClientSyncGetGameGroupID(),
                                    player) ;
                                G_playerRetransmitOccuring[player] = TRUE ;
//                            }
                        }
//                    }
                    G_playerPacketResyncTime[player] = TickerGet() ;
                } else {
                }
            }
        }
    }

    DebugEnd() ;
}

/* LES: 06/17/06  Created */
T_void ClientSyncReceiveRetransmitPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_retransmitPacket *p_retrans ;
    T_doubleLinkListElement element ;
    T_packetLong *p_packetHistory ;
    T_syncPacket *p_sync ;
    T_syncronizePacket *p_syncro ;

    DebugRoutine("ClientSyncReceiveRetransmitPacket") ;

    /* Get a quick pointer to the data. */
    p_retrans = (T_retransmitPacket *)(p_packet->data) ;

//printf("Receive Retrans looking for %d\n", p_retrans->transmitStart) ;
#   ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
//MessageAdd("Receive Retrans") ;
//fprintf(G_fp, "Receive Retrans looking for %d\n", p_retrans->transmitStart) ;
//fprintf(G_fp, "we are object %d, looking for %d\n", ObjectGetServerId(PlayerGetObject()), 9000+p_retrans->toPlayer) ;
#   endif

    /* Only consider the message if in this group. */
    if ((CompareGameGroupIDs(p_retrans->groupID, G_groupID)) &&
        (!(CompareGameGroupIDs(p_retrans->groupID, (*DirectTalkGetNullBlankUniqueAddress())))))  {
        /* See if it is us being request to retransmit. */
        if (p_retrans->toPlayer == (ObjectGetServerId(PlayerGetObject())-9000))  {
            /* Yes, the request is to us. */
            /* Search our history to find the start of the request. */
            element = DoubleLinkListGetFirst(G_packetHistory) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_packetHistory =
                    (T_packetLong *)DoubleLinkListElementGetData(element) ;
                p_sync = (T_syncPacket *)(p_packetHistory->data) ;
                p_syncro = (T_syncronizePacket *)(p_sync->syncData) ;

                /* If we find a match, stop here. */
                if (p_syncro->syncNumber == p_retrans->transmitStart)
                    break ;
                element = DoubleLinkListElementGetNext(element) ;
            }

            /* Did we find a match? */
            if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                /* Yes, a match was found. */
                /* Send all the elements of the history again. */
                while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                    /* resend that packet as is. */
                    p_packetHistory =
                        (T_packetLong *)DoubleLinkListElementGetData(element) ;
                    p_sync = (T_syncPacket *)(p_packetHistory->data) ;
                    p_syncro = (T_syncronizePacket *)(p_sync->syncData) ;

                    /* Note that this is a retransmitted packet. */
//                    p_syncro->fieldsAvailable |= SYNC_PACKET_RETRANSMIT_PACKET ;
//printf("Sending retransmit %d\n", p_syncro->syncNumber) ;
/*
                    CmdQSendPacket(
                       (T_packetEitherShortOrLong *)
                           DoubleLinkListElementGetData(element),
                       140,
                       0,
                       NULL) ;
*/
                    /* Only send to that person. */
                    DirectTalkSetDestination(
                        PeopleHereGetUniqueAddr(
                            p_retrans->fromPlayer)) ;
                    PacketSend((T_packetEitherShortOrLong *)
                        DoubleLinkListElementGetData(element)) ;
                    element = DoubleLinkListElementGetNext(element) ;
                }
            } else {
//puts("no match found") ;
    #           ifdef COMPILE_OPTION_RECORD_CSYNC_DAT_FILE
    //MessageAdd("---------- No match found") ;
    //fprintf(G_fp, "  No match found\n") ;
    #           endif
            }
        }
    }
    DebugEnd() ;
}

static T_void IAddToSyncObjectIdHistory(T_word16 nextId, T_word16 syncNum)
{
    G_syncObjectIdHistory[G_syncObjectIdHistoryPos].nextId = nextId ;
    G_syncObjectIdHistory[G_syncObjectIdHistoryPos].syncNum = syncNum ;
    G_syncObjectIdHistoryPos++ ;
    if (G_syncObjectIdHistoryPos == MAX_OBJECT_ID_HISTORY)
        G_syncObjectIdHistoryPos = 0 ;
}

static E_Boolean ICheckSyncIdHistory(T_word16 nextId, T_word16 syncNum)
{
    T_word16 i ;
    T_byte8 buffer[80] ;

    /* Check to see if sync history is different. */
    for (i=0; i<MAX_OBJECT_ID_HISTORY; i++)  {
        if (G_syncObjectIdHistory[i].syncNum == syncNum)  {
            if (G_syncObjectIdHistory[i].nextId != 0xFFFF)  {
                if (G_syncObjectIdHistory[i].nextId != nextId)  {
                    if (nextId)  {
#if 1
                        sprintf(buffer, "OBJECT SYNC ERROR!!! (%d | %d)",
                            G_syncObjectIdHistory[i].nextId,
                            nextId) ;
                        ClientSetClientSyncStatus(buffer) ;
                        SyncMemDumpOnce() ;
#endif
                        return FALSE ;
                    }
                }
            }
        }
    }

    return TRUE ;
}

/* LES: 06/17/06  Created */
static T_void ClientSyncUpdateReceived(T_void)
{
    T_word16 player ;
    E_Boolean isEveryoneHere ;
    T_word16 slowestSync ;
    T_syncronizePacket *p_sync ;
    T_doubleLinkListElement element ;
    T_word32 delta ;
    extern T_word32 G_syncCount ;
    E_Boolean firstFound ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("ClientSyncUpdateReceived") ;

    /* Do this as many times as necessary to catch up. */
//TESTING    do {
        isEveryoneHere = TRUE ;

        /* First, see if everyone is here. */
        for (player=0; player<G_numPlayers; player++)  {
            if ((DoubleLinkListGetNumberElements(
                    G_playerPacketArray[player]) == 0) &&
                    (G_playerLeft[player] == FALSE))  {
                isEveryoneHere = FALSE ;
                break ;
            }
        }

        /* Is everyone ready with a sync packet? */
        if (isEveryoneHere == TRUE)  {
            /* Ok, let's process everyone's move and calculate the next */
            /* sync time update (equal to the slowest sync) */
#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
            for (player=0; player<G_numPlayers; player++)  {
                /* Skip the player that has left */
                if (G_playerLeft[player] == TRUE)
                    continue ;

                element = DoubleLinkListGetFirst(G_playerPacketArray[player]) ;
                DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;
                if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                    p_sync = (T_syncronizePacket *)
                                 DoubleLinkListElementGetData(element) ;
                    DebugCheck(p_sync != NULL) ;
                    if (p_sync != NULL)  {
                        ICheckSyncIdHistory(
                            p_sync->nextObjectId,
                            p_sync->nextObjectIdWhen) ;
                    }
                }
            }
#endif
            slowestSync = 0 ;
            for (player=0; player<G_numPlayers; player++)  {
                /* Skip the player that has left */
                if (G_playerLeft[player] == TRUE)
                    continue ;

                firstFound = FALSE ;
                element = DoubleLinkListGetFirst(G_playerPacketArray[player]) ;
                DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;
                if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                    p_sync = (T_syncronizePacket *)
                                 DoubleLinkListElementGetData(element) ;
                    DebugCheck(p_sync != NULL) ;
                    if (p_sync != NULL)  {
//printf("  d%d: %d, n: %d\n", player, p_sync->deltaTime, p_sync->syncNumber) ;
#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
                        if (firstFound == FALSE)  {
                            syncNum = p_sync->syncNumber ;
                            firstFound = TRUE ;
                        } else  {
                            if (p_sync->syncNumber != syncNum)  {
                                sprintf(buffer, "sync numbers diff: %d and %d", syncNum, p_sync->syncNumber) ;
                                ClientSetClientSyncStatus(buffer) ;
                            }
                        }
#endif
                        if (p_sync->deltaTime > slowestSync)
                            slowestSync = p_sync->deltaTime ;
                        ClientSyncPacketEvaluate(p_sync) ;

#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
                        G_lastGoodNextId = ObjectGetNextId() ;
//                        G_lastGoodNextId = SyncMemGetChecksum() ;
                        G_lastGoodNextIdSyncNum = p_sync->syncNumber ;
#endif
                        /* Destroy the packet off the list. */
                        MemFree(p_sync) ;
                    }
                    DoubleLinkListRemoveElement(element) ;
                }
            }

            /* Check for player collisions */
            ICheckCollides() ;

#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
            IAddToSyncObjectIdHistory(G_lastGoodNextId, G_lastGoodNextIdSyncNum) ;
#endif

//printf("  slow: %d\n", slowestSync) ;

            /* Update the sync timer. */
            /* Don't go over 1/2 second */
            if (slowestSync > 35)
                slowestSync = 35 ;

            /* Update the clock. */
            if ((!ClientIsPaused()) || (SyncTimeGet()<=1))  {
                delta = slowestSync ;
                delta += SyncTimeGet() ;
                SyncTimeSet(delta) ;
                SyncMemAdd("------------------- SYNC %d -----------------\n", SyncTimeGet(), 0, 0) ;
//printf("\n---------------------------- SYNC %d ---------------------------\n\n", SyncTimeGet()) ;
    //printf("Z1: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
                CreaturesUpdate() ;
    //printf("Z2: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
                ObjectGeneratorUpdate() ;
    //printf("Z3: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
                ServerUpdate() ;
    //printf("Z4: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
                ScheduleUpdateEvents() ;
    //printf("Z5: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
                ObjectsUpdateAnimation(SyncTimeGet()) ;
    //printf("Z6: %08lX\n", ObjectGetZ(ObjectFind(6))) ;
            }

            /* Note that we have synced. */
            G_syncAhead-- ;
            G_syncCount++ ;
        } else {
            /* Sorry, not all packets have arrived.  Just wait. */
            /* Don't do anything. */
        }
//TESTING    } while (isEveryoneHere == TRUE) ;

    DebugEnd() ;
    TICKER_TIME_ROUTINE_ENDM("ClientSyncUpdateReceived", 500) ;
}

T_void ClientSyncSetNumberPlayers(T_byte8 numPlayers)
{
    G_numPlayers = numPlayers ;
    G_numTruePlayers = numPlayers ;
}

T_byte8 ClientSyncGetNumberPlayers(T_void)
{
    return (T_byte8)G_numPlayers;
}

T_objMoveStruct *ClientSyncGetLastGoodMove(T_void)
{
    return &G_lastGoodObjMove ;
}

T_void ClientSyncReset(T_void)
{
    G_syncAhead = 0 ;
    G_lastTimeSyncSent = 0 ;
}


T_gameGroupID ClientSyncGetGameGroupID(T_void)
{
    return G_groupID ;
}

T_void ClientSyncSetGameGroupID(T_gameGroupID groupID)
{
    DebugRoutine("ClientSyncSetGameGroupID") ;
/*
printf("G_groupID now is ") ;
DirectTalkPrintAddress(stdout, &groupID) ;
printf(" by %s\n", DebugGetCallerName()) ;
*/
    G_isGroupIDInit = TRUE ;
    G_groupID = groupID ;

    DebugEnd() ;
}

T_void ClientSyncSendIdSelf(T_byte8 *p_name)
{
    T_word16 i ;
    T_word16 len ;
    T_byte8 buffer[40] ;

    DebugRoutine("ClientSyncSendIdSelf") ;

    strncpy(buffer, p_name, 35) ;
    len = strlen(buffer) ;
    p_name = buffer ;
    for (i=0; i<len; i+=7, p_name+=7)  {
        /* Send out 7 characters at a time, with a null at the end. */
        IClientSyncSendAction(
            PLAYER_ACTION_ID_SELF,
            *((T_word16 *)(p_name + 0)),
            *((T_word16 *)(p_name + 2)),
            *((T_word16 *)(p_name + 4)),
            *((T_byte8 *)(p_name + 6))) ;
    }

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CSYNCPCK.C
 *-------------------------------------------------------------------------*/
