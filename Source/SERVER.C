/****************************************************************************/
/*    FILE:  SERVER.C                                                       */
/****************************************************************************/
#include "3D_COLLI.H"
#include "3D_TRIG.H"
#include "AREASND.H"
#include "CMDQUEUE.H"
#include "CRELOGIC.H"
#include "DOOR.H"
#include "MAP.H"
#include "MEMORY.H"
#include "MEMTRANS.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PLAYER.H"
#include "RANDOM.H"
#include "SERVER.H"
#include "SERVERSH.H"
#include "SMCCHOOS.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "VIEWFILE.H"

#define MAX_PLAYERS 4
#define SYNC_TIMING (70*1)
#define SERVER_PLAYER_NONE 0xFFFF

#define SERVER_TIME_BETWEEN_OBJECT_UPDATES 7

static E_Boolean G_serverInit = FALSE ;

E_Boolean G_serverActive = FALSE ;

static T_word32 G_nextMap = 1 ;

T_sword32 G_sourceX, G_sourceY, G_sourceZ ;

static T_word32 G_serverID = 1 ;

/* Internal prototypes: */
E_Boolean IServerCheckSectorSoundsForObject(T_3dObject *p_obj, T_word32 data) ;

static T_void IServerDataBlockSentNowFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData) ;

/****************************************************************************/
/*  Routine:  ServerInit                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerInit starts up the server and gets everything in order before   */
/*  any other actions can occur.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Must be called before all other ServerXXX commands.                   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*    AMT  09/??/95  Added support for master/slave callback sets, and for  */
/*                   re-initializing this module after a fork().            */
/*                                                                          */
/****************************************************************************/

T_void ServerInit(T_void)
{
    DebugRoutine("ServerInit") ;
    DebugCheck(G_serverInit == FALSE) ;

    /** Register my callback routines. **/

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerFinish                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerFinish cleans up the server.                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerPacketLogFinish                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerFinish(T_void)
{
    DebugRoutine("ServerFinish") ;
//    DebugCheck(G_serverInit == TRUE) ;

    G_serverInit = FALSE ;

    DebugEnd() ;
}

/* 03/22/96 */
T_void ServerPlayerLeft(T_player player)
{
    DebugRoutine("ServerPlayerLeft") ;

//    StatsSaveCharacter(StatsGetActive()) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveProjectileAddPacket                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    A PROJECTILE_CREATE packet is used as an alternative to an            */
/*  OBJECT_CREATE when the new object is thrown, fired, or otherwise        */
/*  projected from a known origin object.  This allows the packet to be     */
/*  SHORT rather than LONG, since a good deal of positional information is  */
/*  implicit and doesn't need to be sent.  This routine handles these       */
/*  packets.                                                                */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- objectAdd packet               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *              -- Created object                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/01/95  Created                                                */
/*    LES  08/02/95  Corrected problem with fireballs and angles            */
/*                                                                          */
/****************************************************************************/
T_3dObject *ServerReceiveProjectileAddPacket (T_packetEitherShortOrLong *p_packet)
{
    T_projectileAddPacket *p_addPacket;
    T_3dObject *p_obj;
    T_sword32 startX, startY;
    E_Boolean creationOK = TRUE;

    DebugRoutine ("ServerReceiveProjectileAddPacket");
//printf("SRProjectileAdd: called by %s\n", DebugGetCallerName()) ;

    /** Get a pointer to the add packet data. **/
    p_addPacket = (T_projectileAddPacket *)(p_packet->data);

    /** Create a new object in my world. **/
    p_obj = ObjectCreate ();

    /** Set its type according to the packet instructions. **/
    ObjectSetType (p_obj, p_addPacket->objectType);

    /** Set up its movement parameters. **/
    ObjectSetX16(p_obj, p_addPacket->x) ;
    ObjectSetY16(p_obj, p_addPacket->y) ;
    ObjectSetZ16(p_obj, p_addPacket->z) ;
    ObjectSetAngle(p_obj, p_addPacket->angle) ;
    ObjectSetZVel(p_obj, (((T_sword32)p_addPacket->vz)<<16)) ;

    /* Attach an owner ID */
    ObjectSetOwnerID(p_obj, p_addPacket->ownerObjID) ;

    startX = (p_addPacket->x << 16) ;
    startY = (p_addPacket->y << 16) ;

    /** Did everything go alright? **/
    if (creationOK == TRUE)
    {
        /** Yes.  Now that it's ready, add it to the world. **/
        ObjectAdd (p_obj);
        ObjectSetUpSectors(p_obj);
        ObjectSetZ16(p_obj, p_addPacket->z) ;
        ObjectSetAngularVelocity (
            p_obj,
            p_addPacket->angle,
            (T_word16)p_addPacket->initialSpeed);

        /** That concludes my local object creation. **/
        /** To tell all the clients to create it too, I just send them all **/
        /** the same packet, plus my server ID value. **/
        p_addPacket->objectID = ObjectGetServerId (p_obj);

        /* Try setting up the target for this object. */
        if (p_addPacket->target != 0)
            ObjectSetTarget(p_obj, p_addPacket->target) ;
    }

    DebugEnd ();

    return p_obj ;
}

/****************************************************************************/
/*  Routine:  ServerShootProjectile                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerShootProjectile is a routine that will create a projectile      */
/*  by setting up a projectile packet and then sending it to itself.        */
/*  This will cause the packet to be sent to all the clients and the object */
/*  to be created (with an appropriate server id).                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_objSource     -- Object doing the shooting              */
/*                                                                          */
/*    T_word16 angle              -- Angle to shoot the projectile          */
/*                                                                          */
/*    T_word16 typeObj            -- Object type (fireball?)                */
/*                                                                          */
/*    T_byte8 initSpeed           -- How fast.  Note this is a small value  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Created projectile, NULL for none      */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerReceiveProjectileAddPacket                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_3dObject *ServerShootProjectile(
          T_3dObject *p_objSource,
          T_word16 angle,
          T_word16 typeObj,
          T_word16 initSpeed,
          T_3dObject *p_target)
{
    T_packetLong packet ;
    T_projectileAddPacket *p_packet ;
    T_sword32 distance ;
    T_sword32 x, y ;
    T_sword16 tx, ty ;
    T_sword32 deltaHeight;
    T_sword16 heightTarget ;
    T_3dObject *p_obj ;

    DebugRoutine("ServerShootProjectile") ;
    DebugCheck(p_objSource != NULL) ;
//printf("ServerShootProj: called by %s\n", DebugGetCallerName()) ;

    /* Get a quick pointer. */
    p_packet = (T_projectileAddPacket *)packet.data ;
    memset(&packet, 0, sizeof(packet)) ;

    /* Set up the short packet. */
    packet.header.packetLength = sizeof(T_projectileAddPacket) ;

    /* Fill it with the parameters. */
    /* Start with the command. */
    p_packet->command = PACKET_COMMANDCSC_PROJECTILE_CREATE;

    distance = (ObjectGetRadius(p_objSource) << 1) ;

    ObjectGetForwardPosition(
         p_objSource,
         distance,
         &x,
         &y) ;

    p_packet->x = (x>>16) ;
    p_packet->y = (y>>16) ;
    p_packet->z = ObjectGetZ16(p_objSource) ;
    p_packet->z += ((ObjectGetHeight(p_objSource)*2)/3) ;
    p_packet->z -= 10 ;
    p_packet->vz = 0 ;

    if (Collide3dObjectToXYCheckLineOfSightWithZ(
                      p_objSource,
                      p_packet->x,
                      p_packet->y,
                      p_packet->z) == TRUE)  {
        p_packet->x = ObjectGetX16(p_objSource) ;
        p_packet->y = ObjectGetY16(p_objSource) ;
    }

    p_packet->objectID = 0 ;  /* Filled by server. */

    p_packet->objectType = typeObj ;
    p_packet->initialSpeed = (T_byte8)initSpeed ;
    p_packet->angle = angle ;
    p_packet->ownerObjID = ObjectGetServerId(p_objSource) ;

    if (p_target != NULL)  {
        /** Target. **/
        p_packet->target = ObjectGetServerId (p_target);

        /* Find where the target is located. */
        tx = ObjectGetX16(p_target) ;
        ty = ObjectGetY16(p_target) ;

        /* Calculate the distance between here and there. */
        distance = CalculateDistance(x >> 16, y >> 16, tx, ty) ;

        /* How high is the target? */
        heightTarget = ObjectGetMiddleHeight(p_target)+10 ;

        /* Calculate the steps necessary to draw a straight */
        /* line to the target. */
        if (distance > 3)
            deltaHeight =
                (((T_sword32)
                    (heightTarget - p_packet->z
                        /* ObjectGetZ16(p_objSource) */))<<16) / distance ;
        else
            deltaHeight = 0 ;
        deltaHeight *= ((T_sword32)initSpeed) ;

        /* Don't allow more than 45 degrees up. */
        if (deltaHeight >= 0x400000)
            deltaHeight = 0x400000 ;

        /* Don't allow more than 45 degrees down. */
        if (deltaHeight <= -0x400000)
            deltaHeight = -0x400000 ;

        p_packet->vz = ((T_sword16)(deltaHeight>>16)) ;

        /* If the target is invisible or translucent, randomly turn a */
        /* little to make it harder to hit. */
        if (ObjectIsStealthy(p_target))  {
            p_packet->angle += (RandomValue() & 0x3FF) - 0x200 ;
        }
    } else {
    }

    /* This is a cheat, send it to ourself! */
    p_obj = ServerReceiveProjectileAddPacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;

    return p_obj ;
}

/****************************************************************************/
/*  Routine:  ServerUpdate                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerUpdate goes through all the code execution necessary to update  */
/*  the server actions in one time slice.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerUpdate(T_void)
{
    static T_word32 lastCount = 0 ;
    T_word32 delta;
    T_word16 lastbit = 0;
    T_word32 time ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("ServerUpdate") ;
    INDICATOR_LIGHT(38, INDICATOR_GREEN) ;

    /* Don't do any actions if the server is in a state where the level */
    /* is not ready. */
    if (G_serverActive == TRUE)  {
        /* Server will now do one iteration of whatever needs to be done */
        /* by the server. */

        /** Keep track of the deltas. **/
        if (lastCount == 0)
        {
           lastCount = SyncTimeGet ();
        }
        else
        {
           time = SyncTimeGet() ;
           delta = time - lastCount + lastbit;
           lastbit = delta & 1 ;
           delta = delta + (delta >> 1) ;
           lastCount = time ;

           /** Apply all velocities to the objects. **/
           ObjectsUpdateMovement (delta);
        }

        /* See if any objects have caused sector sounds and send them. */
        ServerCheckSectorSounds() ;

        /** Then, send out all necessary movement packets. **/
//        CreaturesUpdate() ;
//        ObjectGeneratorUpdate() ;

        /* Check for objects that need destroying. */
        ServerDestroyDestroyObjects() ;
    }

    DebugEnd() ;
    INDICATOR_LIGHT(38, INDICATOR_RED) ;
    TICKER_TIME_ROUTINE_END(stdout, "ServerUpdate", 500) ;
}

/****************************************************************************/
/*  Routine:  ServerGetNextPlayer                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerGetNextPlayer is used to traverse through all the players in    */
/*  the game.  Use -1 to get the first player and then continue.  When a    */
/*  -1 is returned, you know that you are at the end of the list.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 lastPlayer        -- Player to start from or -1 for first   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_sword16                   -- next player.                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_sword16 ServerGetNextPlayer(T_player lastPlayer)
{
    T_player nextPlayer ;
    T_3dObject *p_obj ;

    DebugRoutine("ServerGetNextPlayer") ;

/*
printf("\nlastplayer: %d\n", lastPlayer) ;

for (p_obj=ObjectsGetFirst(); p_obj!=NULL; p_obj=ObjectGetNext(p_obj))
  if (ObjectIsPlayer(p_obj))
    printf("p_obj: %p (%d %d) attr: %04X\n", p_obj, ObjectGetServerId(p_obj), ObjectGetType(p_obj), ObjectGetAttributes(p_obj)) ;
*/

    if (lastPlayer == PLAYER_BAD)  {
        /* Start at the beginning of the list. */
        p_obj = ObjectsGetFirst() ;
    } else {
        /* Find where we were. */
        p_obj = ObjectFind(lastPlayer) ;
        /* Skip immediately to the next one. */
        if (p_obj != NULL)
            p_obj = ObjectGetNext(p_obj) ;

    }

    /* Loop until we find a player. */
    while (p_obj != NULL)  {
//if (ObjectIsPiecewise(p_obj))  {
//    printf("piecewise p_obj: %p (%d %d) attr: %04X\n", p_obj, ObjectGetServerId(p_obj), ObjectGetType(p_obj), ObjectGetAttributes(p_obj)) ;
//}
//if (ObjectIsBodyPart(p_obj))  {
//    printf("bodyPart p_obj: %p (%d %d) attr: %04X\n", p_obj, ObjectGetServerId(p_obj), ObjectGetType(p_obj), ObjectGetAttributes(p_obj)) ;
//}
        if ((ObjectIsPlayerHead(p_obj)) && (ObjectGetStance(p_obj) != STANCE_DIE))  {
//puts("found") ;
            break ;
        }

        p_obj = ObjectGetNext(p_obj) ;
    }
/*
if (p_obj != NULL)  {
   printf("found: ") ;
    printf("p_obj: %p (%d %d) attr: %04X\n", p_obj, ObjectGetServerId(p_obj), ObjectGetType(p_obj), ObjectGetAttributes(p_obj)) ;
}
*/
    /* Get the id of this player (if there is one). */
    if (p_obj == NULL)
        nextPlayer = PLAYER_BAD ;
    else
        nextPlayer = ObjectGetServerId(p_obj) ;

/*
if (nextPlayer == 0)  {
    printf("\n\nfakemode: %s\n", PlayerInFakeMode()?"FAKE":"REAL") ;
    printf("%s player.\n", ObjectIsPlayer(p_obj)?"IS":"IS NOT") ;
    ObjectPrint(stdout, p_obj) ;
    fflush(stdout) ;
}
*/

    DebugCheck(nextPlayer != 0) ;
    DebugEnd() ;

    return nextPlayer ;
}

/****************************************************************************/
/*  Routine:  ServerSendSyncPacket                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Every so often a sync packet is used to make sure all the players are */
/*  together in the same time.  This "syncing" is executed by this command. */
/*  Players are then synced when G_syncCount equals zero.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerSendToAllFromWithCallback                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerSendSyncPacket(T_void)
{
    T_packetShort packet ;
    T_syncPacket *p_sync ;

    DebugRoutine("ServerSendSyncPacket") ;

    /* Get at the packet's data. */
    p_sync = (T_syncPacket *)&packet.data ;

    /* Put the command and length of the packet in the packet. */
    p_sync->command = PACKET_COMMAND_SYNC ;
    packet.header.packetLength = SHORT_PACKET_LENGTH ;

    /* Send this sync packet to all the players. */
    CmdQSendPacket(
        (T_packetEitherShortOrLong *)&packet,
        140, /* No real player being excluded, send to all */
        0,  /* No additional data. */
        NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerRecieveMessagePacket                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveMessagePacket is a called when a packet containing a     */
/*  message needs to be passed to all the other players.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- message packet                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerSendToAllFrom                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveMessagePacket(T_packetEitherShortOrLong *p_packet)
{
    T_messagePacket *p_msg ;
    T_word16 range ;

    DebugRoutine("ServerReceiveMessagePacket") ;

    p_msg = (T_messagePacket *)p_packet->data ;

    switch(p_msg->message[0])  {
        case '<':
            range = MESSAGE_RANGE_WHISPER ;
            break ;
        case '>':
            range = MESSAGE_RANGE_SHOUT ;
            break ;
        default:
            range = MESSAGE_RANGE_TALK ;
            break ;
    }

    /* Send out a notice to all the other players. */
//    ServerSendToAllFromInRange(p_packet, p_msg->player, range) ;
    CmdQSendPacket(p_packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerGetPlayerObject                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerGetPlayerObject finds a player and returns its object.  If none */
/*  is found, NULL is returned.                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 playerId           -- Player's Id                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Player's object, or NULL if not found  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_3dObject *ServerGetPlayerObject(T_word16 playerId)
{
    T_3dObject *p_obj ;

    DebugRoutine("ServerGetPlayerObject") ;

    /** Just always return the object pointer. **/
//    p_obj = players[playerId].p_obj ;
    p_obj = ObjectFind(playerId) ;

    DebugEnd() ;

    return p_obj ;
}

/****************************************************************************/
/*  Routine:  ServerCheckSectorSounds                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerCheckSectorSounds sees if all the objects have caused any       */
/*  sounds to occur.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine is to be immediately called after the ObjectsUpdateMove  */
/*  routine is called.  Don't send packets until this routine is called.    */
/*  The IsMoved flag on the objects is used to determine if a check is      */
/*  needed or not.                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectsDoToAll                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerCheckSectorSounds(T_void)
{
    DebugRoutine("ServerCheckSectorSounds") ;

    /* Check all the objects.  Pass 0 since we don't have any */
    /* other data to pass. */
    ObjectsDoToAll(IServerCheckSectorSoundsForObject, 0) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  IServerCheckSectorSoundsForObject                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IServerCheckSectorSoundsForObject sees if an object is on a group     */
/*  of sectors with the same "entry sound."  If it is, the sound is produced*/
/*  given that the object has not recently created the sound.  This is      */
/*  used for making splashing sounds.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Player object to identify              */
/*                                                                          */
/*    T_word32 data               -- This is currently always zero          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dGetSectorEnterSound                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean IServerCheckSectorSoundsForObject(T_3dObject *p_obj, T_word32 data)
{
    T_word16 i, sound1=0, sound2 ;
    T_word16 sector ;

    DebugRoutine("IServerCheckSectorSoundsForObject") ;
    DebugCheck(p_obj != NULL) ;

    /* Only bother with objects that have moved. */
    if (!ObjectIsBodyPart(p_obj))  {
        /* Also, only bother with objects that are not flying. */
//        if (ObjectIsAboveGround(p_obj) == FALSE)  {
        if (ObjectGetZ16(p_obj) < MapGetFloorHeight(ObjectGetAreaSector(p_obj)))  {
            /* See if there are sounds on each of the sectors. */
            sector = ObjectGetNthAreaSector(p_obj, 0) ;
            sound1 = View3dGetSectorEnterSound(sector) ;

            /* If this sound is zero, there is no need to go on. */
            if (sound1)  {
                /* Loop through all the other sectors and see if they */
                /* are the same sound.  Otherwise, stop immediately. */
                for (i=1; i<ObjectGetNumAreaSectors(p_obj); i++)  {
                    sound2 = View3dGetSectorEnterSound(
                                 ObjectGetNthAreaSector(p_obj, i)) ;
                    if (sound2 != sound1)  {
                        sound1 = 0 ;
                        break ;
                    }
                }

                /* Are we in agreement? (sound1 will be non-zero if we are) */
                if (sound1)  {
                    /* Is this sound non-zero and different than last time? */
                    if ((sound1) && (ObjectGetLastSound(p_obj) != sound1))  {
                        /* Play the sound at this location. */
                        AreaSoundCreate(
                            ObjectGetX16(p_obj),
                            ObjectGetY16(p_obj),
                            View3dGetSectorEnterSoundRadius(sector),
                            180,
                            AREA_SOUND_TYPE_ONCE,
                            0,
                            AREA_SOUND_BAD,
                            NULL,
                            0,
                            sound1) ;

                    }
                }
            }
            /* Note that this is the last sound (even if zero). */
            ObjectSetLastSound(p_obj, sound1) ;
        } else {
            ObjectSetLastSound(p_obj, 0) ;
        }
    }

    DebugEnd() ;

    return FALSE;
}

/****************************************************************************/
/*  Routine:  ServerGotoPlace                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerGotoPlace is called to force all logged-in clients to go to     */
/*  a map or form.  It has the same effect as receiving a goto-place packet */
/*  from a client.                                                          */
/*                                                                          */
/*     New behavior (AMT, 9/13/95):  Place numbers fit into the following   */
/*  groups, specified by their high 2 bytes:                                */
/*     0000 - 00FF   Levels.  This allows for an absurd number of levels.   */
/*                   Whenever a master receives one, a new slave server     */
/*                   is forked off and begins an adventure.                 */
/*                   Whenever a slave receives one, it just advances to that*/
/*                   level.                                                 */
/*     0100 - 01FF   Forms.  Mail rooms, inns, etc.  Whenever one of these  */
/*                   is received by a slave, it terminates and returns      */
/*                   control to the master.  Masters handle normally.       */
/*     0200          Currently-running adventures.  I.e. currently active   */
/*                   slaves which the client wishes to join.                */
/*                   Slave receives and ignores.  Master receives and tells */
/*                   the appropriate slave.                                 */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 placeNumber          -- the place to go to.                  */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/09/95  Created                                                */
/*    AMT  09/13/95  Changed the face of forever.                           */
/*                                                                          */
/****************************************************************************/

T_void ServerGotoPlace(T_word32 placeNumber, T_word16 startLocation)
{
    T_packetShort packet ;
    T_gotoPlacePacket *p_packet ;

    DebugRoutine("ServerGotoPlace") ;

//printf("ServerGotoPlace: %ld %d\n", placeNumber, startLocation) ; fflush(stdout) ;

    /** Normal mode. **/
    /** Is this a next-map instruction? **/
//printf("Server attempting to go to %d\n", placeNumber) ;

//printf("Server going to %d (was %d)\n", placeNumber, G_nextMap) ;
    G_nextMap = placeNumber ;

    /* Mark updates for the current level as inactive. */
    G_serverActive = FALSE ;

    /** Create a GOTO_PLACE packet. **/
    packet.header.packetLength = SHORT_PACKET_LENGTH;
    p_packet = (T_gotoPlacePacket *)packet.data;

    p_packet->command = PACKET_COMMANDCSC_GOTO_PLACE;
    p_packet->placeNumber = placeNumber;
    p_packet->startLocation = startLocation ;
//printf("start location set to %d\n", startLocation) ;

    /** Just send an identical packet to each client. **/
    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ServerReceiveGotoPlacePacket                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveGotoPlacePacket is called when a client reqests a map    */
/*  or form change.  It basically loads in the new map and sends a copy     */
/*  of the request to all the other clients.                                */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_gotoPacket -- the goto place packet.     */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/21/95  Created                                                */
/*    AMT  09/13/95  Made it just call ServerGotoPlace                      */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveGotoPlacePacket(T_packetEitherShortOrLong *p_gotoPacket)
{
    T_gotoPlacePacket *p_packet ;

    DebugRoutine("ServerReceiveGotoPlacePacket") ;
//puts("ServerReceiveGotoPlacePacket") ;

    /* Get a quick pointer. */
    p_packet = (T_gotoPlacePacket *)p_gotoPacket->data ;

    ServerGotoPlace(p_packet->placeNumber, p_packet->startLocation);

    if (p_packet->placeNumber == 0)
        ServerPlayerLeft(CmdQGetActivePortNum()) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveGotoSucceededPacket                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveGotoSucceededPacket is called when a client has success- */
/*  fully complied with a GotoPlace packet.  This lets me know when every-  */
/*  body is ready.                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- the goto success packet.       */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/21/95  Created                                                */
/*    LES  07/21/95  Finished creating by handling object creation          */
/*                   correctly.  Most of the code was taken from            */
/*                   ServerAckLoginPacket.                                  */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveGotoSucceededPacket(T_packetEitherShortOrLong *p_packet)
{
    T_word16 player ;
    T_packetShort startPacket ;
    T_placeStartPacket *p_start ;
    T_word16 angle ;
    T_gotoSucceededPacket *p_succeeded;

    DebugRoutine("ServerReceiveGotoSucceededPacket") ;
//puts("ServerReceiveGotoSucceededPacket") ;

    /** First! Ensure that this packet corresponds to the place change in **/
    /** progress. **/
    p_succeeded = (T_gotoSucceededPacket *)(p_packet->data);
    G_nextMap = p_succeeded->placeNumber ;
    if (p_succeeded->placeNumber != G_nextMap)
    {
        printf ("Warning: Received incorrect gts packet (gts for %d,"
                " nextMap = %d\n", p_succeeded->placeNumber, G_nextMap);
    }
    else
    {
        /* Find which player this is and mark him as active (if inactive). */
        player = CmdQGetActivePortNum() ;

        /* Clear all past port info. */
        CmdQClearAllPorts() ;

        if (G_nextMap < 10000)  {
            /** Yes!  Make the server update loop active again. **/
            G_serverActive = TRUE;

            /** Yes. **/
            /** Send everyone a place start packet and positional */
            /* information based on their login Id. **/
            p_start = (T_placeStartPacket *)(startPacket.data);

            CmdQSetActivePortNum(1) ;

            /* Set up the command. */
            p_start->command = PACKET_COMMANDSC_PLACE_START ;

            /* What is the client to be known by. */
            p_start->loginId = player ;

            /* What is the object's id. */
            p_start->objectId = 9000 ;

            /* Where is the player to start? */
            MapGetStartLocation(
                p_succeeded->startLocation,
                &p_start->xPos,
                &p_start->yPos,
                &angle) ;
            p_start->angle = angle>>8 ;

            CmdQSendShortPacket(&startPacket, 210, 0, NULL) ;
        } else {
            /* Must be doing a form. */
            /* Just send a place succeeded packet. */
            p_start = (T_placeStartPacket *)(startPacket.data);

            CmdQSetActivePortNum(1) ;

            /* Set up the command. */
            p_start->command = PACKET_COMMANDSC_PLACE_START ;

            /* What is the client to be known by. */
            p_start->loginId = player ;

            /* What is the object's id. */
            /* (No object id) */
            p_start->objectId = 0 ;

            CmdQSendShortPacket(&startPacket, 210, 0, NULL) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveRequestServerIDPacket                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveRequestServerIDPacket is called when a client process    */
/*  wants to know what the unique id of this server is.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- request server id packet       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveRequestServerIDPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_serverIDPacket *p_serverID ;

    DebugRoutine("ServerReceiveRequestServerIDPacket") ;

    p_serverID = (T_serverIDPacket *)(packet.data) ;
    p_serverID->command = PACKET_COMMANDSC_SERVER_ID ;
    p_serverID->serverID = ServerGetServerID() ;
    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerGetServerID                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerGetServerID returns the server id of this server.               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Server id                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 ServerGetServerID(T_void)
{
    return G_serverID ;
}

/****************************************************************************/
/*  Routine:  ServerSetServerID                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerSetServerID sets    the server id of this server.               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32                    -- Server id                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerSetServerID(T_word32 newID)
{
    G_serverID = newID ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveRequestEnterPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveRequestEnterPacket is called when a client is wanting    */
/*  to receive info about if he can enter.                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- request enter packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveRequestEnterPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_requestEnterStatusPacket *p_status ;

    DebugRoutine("ServerReceiveRequestEnterPacket") ;

    p_status = (T_requestEnterStatusPacket *)(packet.data) ;
    p_status->command = PACKET_COMMANDSC_REQUEST_ENTER_STATUS ;
    p_status->status = REQUEST_ENTER_STATUS_OK ;
    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveRequestCharacterListPacket                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveRequestCharacterListPacket is called when the client     */
/*  wishes to know what characters are available for this account.          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.  It just  */
/*  makes a call to StatsGetSavedCharacterList and starts a memory transfer.*/
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- request enter packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    StatsGetSavedCharacterList                                            */
/*    MemoryTransfer                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveRequestCharacterListPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_statsSavedCharArray *p_charArray ;

    DebugRoutine("ServerReceiveRequestCharacterListPacket") ;

    p_charArray = StatsGetSavedCharacterList() ;
/*
    MemoryTransfer(
        p_charArray,
        sizeof(T_statsSavedCharArray),
        NULL,
        CLIENT_MEMORY_BLOCK_CHARACTER_LISTING) ;
*/
    StatsSetSavedCharacterList(p_charArray) ;

    SMCChooseSetFlag(
        SMCCHOOSE_FLAG_ENTER_COMPLETE,
        TRUE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveLoadCharacterPacket                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveLoadCharacterPacket is called when the client            */
/*  wishes to either see if a character needs to be downloaded or if        */
/*  the checksums are correct, noted.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load char     packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveLoadCharacterPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_loadCharacterStatusPacket *p_status ;
    T_loadCharacterPacket *p_load ;
    T_word32 checksum = 0 ;
    T_word32 serverChecksum = 0 ;

    DebugRoutine("ServerReceiveLoadCharacterPacket") ;

    p_load = (T_loadCharacterPacket *)(p_packet->data) ;
    checksum = p_load->checksum ;

    p_status = (T_loadCharacterStatusPacket *)packet.data ;
    p_status->command = PACKET_COMMANDSC_LOAD_CHARACTER_STATUS ;

/* Code needed here to actually compare checksums. */
/*
    serverChecksum = ServerGetCharacterChecksum(p_status->slot) ;
*/

//    if (serverChecksum != checksum)  {
//        p_status->status = LOAD_CHARACTER_STATUS_INCORRECT ;
        /* Immediately start sending the character. */
/* Code needed on full server. */
/*
        ServerSendCharacter(p_status->slot) ; <-- basically a MemoryTransfer
*/

//    } else {
        p_status->status = LOAD_CHARACTER_STATUS_CORRECT ;
//    }

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveCreateCharacterPacket                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveCreateCharacterPacket is called when the client          */
/*  wishes to create a new character in the given slot.  At the same time   */
/*  the character will be uploaded.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- craete char   packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveCreateCharacterPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_createCharStatusPacket *p_status ;
    T_createCharacterPacket *p_create ;
    T_word32 checksum = 0 ;
    E_Boolean createOk = TRUE ;

    DebugRoutine("ServerReceiveCreateCharacterPacket") ;

    p_create = (T_createCharacterPacket *)(p_packet->data) ;
    checksum = p_create->checksum ;

    p_status = (T_createCharStatusPacket *)packet.data ;
    p_status->command = PACKET_COMMANDSC_CREATE_CHARACTER_STATUS ;

    if (createOk)  {
        p_status->status = CREATE_CHARACTER_STATUS_OK ;
    } else {
        p_status->status = CREATE_CHARACTER_STATUS_ERROR ;
    }

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveDeleteCharacterPacket                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveDeleteCharacterPacket is called when the client          */
/*  wishes to delete a new character in the given slot.  At the same time   */
/*  the character will be uploaded.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- delete char   packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/07/96  Deleted                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveDeleteCharacterPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_deleteCharStatusPacket *p_status ;
    T_deleteCharacterPacket *p_delete ;
    T_word32 checksum = 0 ;
    E_Boolean deleteOk = TRUE ;

    DebugRoutine("ServerReceiveDeleteCharacterPacket") ;
//puts("ServerReceiveDeleteCharacterPacket") ;

    p_delete = (T_deleteCharacterPacket *)(p_packet->data) ;

    p_status = (T_deleteCharStatusPacket *)packet.data ;
    p_status->command = PACKET_COMMANDSC_DELETE_CHARACTER_STATUS ;

/*
deleteOk = DeleteCharacterForAccount(???, p_delete->slot) ;
*/
    if (deleteOk)  {
        p_status->status = DELETE_CHARACTER_STATUS_OK ;
    } else {
        p_status->status = DELETE_CHARACTER_STATUS_ERROR ;
    }

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveCheckPasswordPacket                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveCheckPasswordPacket is called when the client            */
/*  wishes to check the password on a character of a given slot.            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- check passwordpacket           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/96  Deleted                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveCheckPasswordPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_checkPasswordStatusPacket *p_status ;
    T_checkPasswordPacket *p_check ;
    T_word32 checksum = 0 ;
    E_Boolean isMatch = TRUE ;
T_byte8 password[MAX_SIZE_PASSWORD] ;

    DebugRoutine("ServerReceiveCheckPasswordPacket") ;

    p_check = (T_checkPasswordPacket *)(p_packet->data) ;

    p_status = (T_checkPasswordStatusPacket *)packet.data ;
    p_status->command = PACKET_COMMANDSC_CHECK_PASSWORD_STATUS ;

    StatsGetPassword(p_check->slot, password) ;
    if (strnicmp(password, p_check->password, MAX_SIZE_PASSWORD) == 0)
       isMatch = TRUE ;
    else
       isMatch = FALSE ;

    if (isMatch)  {
        p_status->status = CHECK_PASSWORD_STATUS_OK ;
    } else {
        p_status->status = CHECK_PASSWORD_STATUS_WRONG ;
    }

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveChangePasswordPacket                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerReceiveChangePasswordPacket is called when the client           */
/*  wishes to change her password on the system.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This code is intended for the Self Server mode ONLY.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- change password packet         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/96  Deleted                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveChangePasswordPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetShort packet ;
    T_changePasswordStatusPacket *p_status ;
    T_changePasswordPacket *p_change ;
    T_word32 changesum = 0 ;
    E_Boolean isMatch = TRUE ;
    T_byte8 password[MAX_SIZE_PASSWORD] ;

    DebugRoutine("ServerReceiveChangePasswordPacket") ;
//puts("ServerReceiveChangePasswordPacket") ;

    p_change = (T_changePasswordPacket *)(p_packet->data) ;

    p_status = (T_changePasswordStatusPacket *)packet.data ;
    p_status->command = PACKET_COMMANDSC_CHANGE_PASSWORD_STATUS ;

    StatsGetPassword(p_change->slot, password) ;
    if (strnicmp(password, p_change->password, MAX_SIZE_PASSWORD) == 0)
        isMatch = TRUE ;
    else
        isMatch = FALSE ;

    if (isMatch)  {
        p_status->status = CHANGE_PASSWORD_STATUS_OK ;

        StatsSetPassword(p_change->slot, p_change->newPassword) ;
//        StatsSaveCharacter(p_change->slot) ;
    } else {
        p_status->status = CHANGE_PASSWORD_STATUS_WRONG ;
    }

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerReceiveRequestDataBlockPacket                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*      ServerReceiveRequestDataBlockPacket is called when the client wants */
/*    a particular data block to be sent.                                   */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packeEitherShortOrLong *p_packet -- packet for data block           */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerReceiveRequestDataBlockPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_word32 size ;
    E_Boolean doMemFree ;
    T_requestDataBlockPacket *p_request ;
    T_void *p_data ;

    DebugRoutine("ServerReceiveRequestDataBlockPacket") ;

    p_request = (T_requestDataBlockPacket *)(p_packet->data) ;
    p_data = ServerFetchDataBlock(
                 p_request->dataBlockType,
                 p_request->extraData,
                 &size,
                 &doMemFree) ;

    /* Check that the data is now available. */
    if (p_data != NULL)
        /* Note if we need to free the memory. */
        if (doMemFree)
            MemoryTransfer(
                p_data,
                size,
                IServerDataBlockSentNowFree,
                p_request->dataBlockType) ;
        else
            MemoryTransfer(
                p_data,
                size,
                NULL,
                p_request->dataBlockType) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IServerDataBlockSentNowFree                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*      Called after a memory transfer, it frees the memory block just      */
/*    sent.                                                                 */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_void *p_data              -- data block sent.                       */
/*    T_word32 size               -- Size of block (not used)               */
/*    T_word32 extraData          -- acc. data (not used)                   */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IServerDataBlockSentNowFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData)
{
    DebugRoutine("IServerDataBlockSentNowFree") ;

    MemFree(p_data) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerFetchDataBlock                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*      ServerFetchDataBlock retrieves one of the basic types of data       */
/*    blocks from where it needs to be fetched.                             */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word16 dataBlockType      -- Type of block to fetch                 */
/*    T_word32 extraData          -- Extra info to process                  */
/*    T_word32 *p_size            -- Size of data block                     */
/*    E_Boolean *doMemFree        -- Flag to note if block needs to be freed*/
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *ServerFetchDataBlock(
            T_word16 dataBlockType,
            T_word32 extraData,
            T_word32 *p_size,
            E_Boolean *doMemFree)
{
    T_void *p_data ;

    DebugRoutine("ServerFetchDataBlock") ;

    /* The default is a zero size null block that does */
    /* not need to be freed. */
    *doMemFree = FALSE ;
    *p_size = 0 ;
    p_data = NULL ;

/* !!! Warning! This is Self Server only (currently) */
    switch(dataBlockType)  {
        case CLIENT_MEMORY_BLOCK_UPDATE_BLOCK:
            /* This block is created under special conditions */
            /* (aka jumping from level to level -- its not */
            /* fetched. */
            break ;

        case CLIENT_MEMORY_BLOCK_CHARACTER_LISTING:
            p_data = MemAlloc(sizeof(T_statsSavedCharArray)) ;
            StatsSetSavedCharacterList(p_data) ;
            *doMemFree = TRUE ;
            break ;

        case MEMORY_BLOCK_CHARACTER_DATA:
            /* Get character stats and inventory block */
            /* and tell to free the block afterwards. */
            StatsLoadCharacter(extraData) ;
            p_data = StatsGetAsDataBlock(p_size) ;
            *doMemFree = TRUE ;
            break ;

        case MEMORY_BLOCK_ACCOUNT_INFO:
            /* Do nothing, this is sent to the server. */
            break ;

        default:
#ifndef NDEBUG
            printf("Unknown data block type: %d\n", dataBlockType) ;
            fflush(stdout) ;
#endif
            DebugCheck(FALSE) ;
            break ;
    }

    DebugEnd() ;

    return p_data ;
}

T_void ServerReceiveSyncPacket(T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ServerReceiveSyncPacket") ;

    /* For the self server, just send it back. */
//    ServerSendToAll(p_packet) ;
    CmdQSendPacket(p_packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/* LES: 05/15/96 -- Create an object anywhere in the world. */
T_3dObject * ServerCreateObjectGlobal(
           T_word16 objType,
           T_sword16 x,
           T_sword16 y,
           T_sword16 z)
{
    T_3dObject *p_obj ;

    DebugRoutine("ServerCreateObjectGlobal") ;

    p_obj = ObjectCreate() ;
    if (p_obj)  {
        ObjectDeclareStatic(
            p_obj,
            x,
            y) ;
        ObjectSetType(p_obj, objType) ;
        ObjectSetAngle(p_obj, 0) ;
        ObjectSetUpSectors(p_obj) ;
        ObjectForceUpdate(p_obj) ;
        ObjectSetZ16(p_obj, z) ;

        ObjectAdd(p_obj) ;
    } else {
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;

    return p_obj ;
}

/* LES: 05/15/96 -- Create an object anywhere in the world. */
T_3dObject * ServerCreateFakeObjectGlobal(
           T_word16 objType,
           T_sword16 x,
           T_sword16 y,
           T_sword16 z)
{
    T_3dObject *p_obj ;

    DebugRoutine("ServerCreateFakeObjectGlobal") ;

    p_obj = ObjectCreateFake() ;
    if (p_obj)  {
        ObjectDeclareStatic(
            p_obj,
            x,
            y) ;
        ObjectSetType(p_obj, objType) ;
        ObjectSetAngle(p_obj, 0) ;
        ObjectSetUpSectors(p_obj) ;
        ObjectForceUpdate(p_obj) ;
        ObjectSetZ16(p_obj, z) ;

//        ObjectAdd(p_obj) ;
    } else {
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;

    return p_obj ;
}

/****************************************************************************/
/*  Routine:  ServerShootBasicProjectile                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerShootBasicProjectile is called to create an object at a         */
/*  specific location and with specific velocities.                         */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objectType         -- Type of object projectile              */
/*                                                                          */
/*    T_sword32 x, y, z           -- Position to start projectile           */
/*                                                                          */
/*    T_sword32 targetX, Y, Z     -- Target position for missile            */
/*                                                                          */
/*    T_word16 initialSpeed       -- How fast to shoot the missile          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerReceiveProjectileAddPacket                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/13/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerShootBasicProjectile(
           T_word16 objectType,
           T_sword32 x,
           T_sword32 y,
           T_sword32 z,
           T_sword32 targetX,
           T_sword32 targetY,
           T_sword32 targetZ,
           T_word16 initialSpeed)
{
    T_packetLong packet ;
    T_projectileAddPacket *p_packet ;
    T_sword16 tx, ty ;
    T_sword32 deltaHeight;
    T_sword16 heightTarget ;
    T_word16 angle ;
    T_word16 distance ;

    DebugRoutine("ServerShootBasicProjectile") ;

    /* Get a quick pointer. */
    p_packet = (T_projectileAddPacket *)packet.data ;

    /* Set up the short packet. */
    packet.header.packetLength = sizeof(T_projectileAddPacket) ;

    /* Fill it with the parameters. */
    /* Start with the command. */
    p_packet->command = PACKET_COMMANDCSC_PROJECTILE_CREATE;

    p_packet->x = (x>>16) ;
    p_packet->y = (y>>16) ;
    p_packet->z = (z>>16) ;

    angle = MathArcTangent(
                (T_sword16)((targetX-x)>>16),
                (T_sword16)((targetY-y)>>16)) ;
    p_packet->angle = angle ;
    tx = (targetX >> 16) ;
    ty = (targetY >> 16) ;

    p_packet->objectID = 0 ;  /* Filled by server. */

    p_packet->objectType = objectType ;
    p_packet->initialSpeed = (T_byte8)initialSpeed ;
    p_packet->ownerObjID = 0 ;
    p_packet->target = 0 ;

    /* Find where the target is located. */
    tx = (targetX>>16) ;
    ty = (targetY>>16) ;

    /* Calculate the distance between here and there. */
    distance = CalculateDistance(x >> 16, y >> 16, tx, ty) ;

    /* How high is the target? */
    heightTarget = (targetZ >> 16) ;

    /* Calculate the steps necessary to draw a straight */
    /* line to the target. */
    if (distance != 0)
        deltaHeight =
            (((T_sword32)
                (heightTarget - p_packet->z))<<16) / distance ;
    else
        deltaHeight = 0 ;
    deltaHeight *= 40 ;

    /* Don't allow more than 45 degrees up. */
    if (deltaHeight >= 0x320000)
        deltaHeight = 0x320000 ;

    /* Don't allow more than 45 degrees down. */
    if (deltaHeight <= -0x320000)
        deltaHeight = -0x320000 ;

    p_packet->vz = ((T_sword16)(deltaHeight>>16)) ;

    /* This is a cheat, send it to ourself! */
    ServerReceiveProjectileAddPacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerDestroyDestroyObjects                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerDestroyDestroyObjects goes through the list of objects and      */
/*  checks if there any objects that are requesting self-destruction.  If   */
/*  there is, an appropriate packet is sent and the object is eliminated.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectsGetFirst                                                       */
/*    ObjectGetNext                                                         */
/*    ServerDestroyObjectGlobal                                             */
/*    ObjectIsMarkedForDestroy                                              */
/*    ObjectsGetNumMarkedForDestroy                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerDestroyDestroyObjects(T_void)
{
    T_3dObject *p_obj ;
    T_3dObject *p_objNext ;

    DebugRoutine("ServerDestroyDestroyObjects") ;

    p_obj = ObjectsGetFirst() ;
    while ((ObjectsGetNumMarkedForDestroy() != 0) &&
           (p_obj != NULL))  {
        p_objNext = ObjectGetNext(p_obj) ;

        if (ObjectIsMarkedForDestroy(p_obj))  {
            ObjectRemove(p_obj) ;
            ObjectDestroy(p_obj) ;
        }

        p_obj = p_objNext ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerDamageAtWithType                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerDamageAtWithType does damage to a given location.               */
/*  If the given radius is nonzero, does damage to anything within the      */
/*  radius.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x, y, z           -- Exact location                         */
/*                                                                          */
/*    T_word16 radius             -- Sphereical radius                      */
/*                                                                          */
/*    T_word16 damage             -- Amount of damage to do                 */
/*                                                                          */
/*    T_word16 ownerID            -- Owner of damage being done             */
/*                                                                          */
/*    E_effectDamageType type     -- Type of damage to do                   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectDoToAllAtXYZRadius                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/16/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerDamageAtWithType(
           T_sword16 x,
           T_sword16 y,
           T_sword16 z,
           T_word16 radius,
           T_word16 damage,
           T_word16 ownerID,
           T_byte8 type)
{
    T_damageObjInfo damageInfo ;
    T_word16 locked ;

    DebugRoutine("ServerDamageAtWithType") ;

    G_sourceX = (((T_sword32)x)<<16) ;
    G_sourceY = (((T_sword32)y)<<16) ;
    G_sourceZ = (((T_sword32)z)<<16) ;

    damageInfo.ownerID = ownerID ;
    damageInfo.damage = damage ;
    damageInfo.type = type ;

    if (type == (EFFECT_DAMAGE_SPECIAL | EFFECT_DAMAGE_SPECIAL_LOCK)) {
        locked = ServerLockDoorsInArea(
                     G_sourceX,
                     G_sourceY,
                     40) ;
        if (ownerID == ObjectGetServerId(PlayerGetObject()))  {
            if ((locked) && (locked != 0x100))  {
                MessageAdd("Lock on door is increased") ;
            } else {
                MessageAdd("Cannot lock") ;
            }
        }
    } else if (type == (EFFECT_DAMAGE_SPECIAL | EFFECT_DAMAGE_SPECIAL_UNLOCK))  {
        locked = ServerUnlockDoorsInArea(
            G_sourceX,
            G_sourceY,
            40) ;
        if (ownerID == ObjectGetServerId(PlayerGetObject()))  {
            if ((locked == 0x100) || (locked == DOOR_ABSOLUTE_LOCK))  {
                MessageAdd("Cannot unlock") ;
            } else if ((locked) &&
                (locked != DOOR_ABSOLUTE_LOCK) &&
                (locked != 0x100))  {
                MessageAdd("Lock on door is decreased") ;
            } else if (locked == 0)  {
                SoundDing() ;
                MessageAdd("Door is now unlocked") ;
            }
        }
    } else {
        ObjectsDoToAllAtXYZRadius(
            x,
            y,
            z,
            radius,
            ServerDamageObjectXYZ,
            (T_word32)(&damageInfo));
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SERVER.C                                                */
/****************************************************************************/
