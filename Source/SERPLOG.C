/****************************************************************************/
/*    FILE:  SERPLOG.C                                                      */
/****************************************************************************/
/*                                                                          */
/*  Module:  Server Packet Log                                              */
/*                                                                          */
/****************************************************************************/
#include "CMDQUEUE.H"
#include "HASH32.H"
#include "SERPLOG.H"

/* Internal prototypes: */
static T_word32 IGetObjectPositionPacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetObjectAddPacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetProjectileAddPacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetObjectDestroyPacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetChangeBodyPartPacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetWallStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetSideStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetSectorStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet) ;
static T_word32 IGetGroupStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet) ;

static T_void IPostProcessDestroyObject(
                  T_packetEitherShortOrLong *p_packet,
                  T_packetLog log,
                  T_packetLogPacket self) ;
static T_void IPostProcessChangeBodyPart(
                  T_packetEitherShortOrLong *p_packet,
                  T_packetLog log,
                  T_packetLogPacket self) ;

/* Global statics: */
static T_packetLog G_serverPacketLog ;
static E_Boolean G_isInit = FALSE ;
static T_hash32 G_playerPacketLogs ;

static T_packetLogParam G_serverPacketLogParams[PACKET_COMMAND_MAX] = {
    /* { type
         size
         p_postProcess
         p_getKeyCallback }, */
    /* 0 ACK */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 1 LOGIN */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 2 PLAYER_MOVE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 3 MONSTER_MOVE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 4 PLAYER_ATTACK */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 5 PLAYER_ACTIVATE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 6 PLAYER_JOIN */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 7 PLAYER_LOGOFF */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 8 PLAYER_CHANGE_WEAPON */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 9 PLAYER_ACTION */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 10 FIREBALL_MOVE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 11 FIREBALL_STOP */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 12 MOVE_CREATURE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 13 SC_DAMAGE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 14 CREATURE_ATTACK */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 15 CREATURE_HURT */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 16 CREATURE_DEAD */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 17 REVERSE_SECTOR */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 18 SYNC */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 19 PICK_UP */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 20 MESSAGE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 21 OPEN_DOOR */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 22 CANNED_SAYING */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 23 SC_OBJECT_POSITION */
    { PACKET_LOG_PARAM_TYPE_ONLY_MASTER_LOG,
      sizeof(T_objectPositionPacket),
      NULL,
      IGetObjectPositionPacketKey},

    /* 24 CS_REQUEST_TAKE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 25 SC_TAKE_REPLY */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 26 CSC_ADD_OBJECT */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_objectAddPacket),
      NULL,
      IGetObjectAddPacketKey },

    /* 27 SC_DESTROY_OBJECT */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_objectDestroyPacket),
      IPostProcessDestroyObject,
      IGetObjectDestroyPacketKey },

    /* 28 SC_SPECIAL_EFFECT */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 29 SC_PLACE_START */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 30 CSC_GOTO_PLACE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 31 CS_GOTO_SUCCEEDED */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 32 CSC_PROJECTILE_CREATE */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_projectileAddPacket),
      NULL,
      IGetProjectileAddPacketKey },

    /* 33 RT_REQUEST_FILE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 34 TR_START_TRANSFER */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 35 TR_DATA_PACKET */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 36 TR_FINAL_PACKET */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 37 RT_RESEND_PLEASE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 38 TR_TRANSFER_COMPLETE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 39 RT_TRANSFER_CANCEL */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 40 TR_FILE_NOT_HERE */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 41 CSC_REQUEST_MEMORY_TRANSFER */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 42 CSC_MEMORY_TRANSFER_READY */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 43 CSC_MEMORY_TRANSFER_DATA */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 44 CSC_CHANGE_BODY */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_changeBodyPartPacket),
      IPostProcessChangeBodyPart,
      IGetChangeBodyPartPacketKey },

    /* 45 CSC_PING */
    { PACKET_LOG_PARAM_TYPE_IGNORE,
      0,
      NULL,
      NULL },

    /* 46 SC_WALL_STATE_CHANGE */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_wallStateChangePacket),
      NULL,
      IGetWallStateChangePacketKey },

    /* 47 SC_SIDE_STATE_CHANGE */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_sideStateChangePacket),
      NULL,
      IGetSideStateChangePacketKey },

    /* 48 SC_SECTOR_STATE_CHANGE */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_sectorStateChangePacket),
      NULL,
      IGetSectorStateChangePacketKey },

    /* 49 SC_GROUP_STATE_CHANGE */
/* !!! Need some type of logic to "spread" out the actual group */
/* or something.  Will the order fix the problem? */
    { PACKET_LOG_PARAM_TYPE_ALL_LOGS,
      sizeof(T_groupStateChangePacket),
      NULL,
      IGetGroupStateChangePacketKey },
} ;

/****************************************************************************/
/*  Routine:  ServerPacketLogInitialize                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogInitialize starts up a packet log for the server       */
/*  part of the program.  All packets are logged in this packet log so that */
/*  new players into the map will get an update.                            */
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
/*    PacketLogCreate                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerPacketLogInitialize(T_void)
{
    DebugRoutine("ServerPacketLogInitialize") ;
    DebugCheck(G_isInit == FALSE) ;

    G_serverPacketLog = PacketLogCreate(
                            PACKET_COMMAND_MAX,
                            G_serverPacketLogParams) ;
    DebugCheck(G_serverPacketLog != PACKET_LOG_BAD) ;

    /* Create the hash tables that will be used to quickly */
    /* find the player packets. */
    G_playerPacketLogs = Hash32Create(256) ;
    DebugCheck(G_playerPacketLogs != HASH32_BAD) ;

    G_isInit = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerPacketLogFinish                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogFinish releases the previously created packet log      */
/*  for the server.                                                         */
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
/*    PacketLogDestroy                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerPacketLogFinish(T_void)
{
    DebugRoutine("ServerPacketLogFinish") ;
    DebugCheck(G_isInit == TRUE) ;

    PacketLogDestroy(G_serverPacketLog) ;

    Hash32Destroy(G_playerPacketLogs) ;

    G_isInit = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerPacketLogAddPacket                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogAddPacket quickly adds a packet to the packet log.     */
/*  All server based packet adds should come through here.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Packet to add to log           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogAddPacket                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerPacketLogAddPacket(T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ServerPacketLogAddPacket") ;
    DebugCheck(G_isInit == TRUE) ;

    /* Just pass on the request. */
    PacketLogAddPacket(G_serverPacketLog, p_packet) ;
    DebugEnd() ;
}

#ifndef NDEBUG
/****************************************************************************/
/*  Routine:  ServerPacketLogDump                     * DEBUG *             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogDump dumps the server's packet log to the given file.  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    FILE *fp                    -- File to dump to                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogDump                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerPacketLogDump(FILE *fp)
{
    DebugRoutine("ServerPacketLogDump") ;
    DebugCheck(G_isInit) ;

#if 0       /* Extra junk to test how well the packet log system works */
    PacketLogDump(G_serverPacketLog, fp) ;
subLog = PacketLogSplit(G_serverPacketLog) ;
testPacket.data[0] = PACKET_COMMANDCSC_PROJECTILE_CREATE ;
testPacket.data[1] = 0 ;
testPacket.data[2] = 4 ;   /* 1024 */
PacketLogAddPacket(G_serverPacketLog, &testPacket) ;
testPacket.data[0] = PACKET_COMMANDSC_OBJECT_POSITION ;
testPacket.data[1] = 1 ;
testPacket.data[2] = 4 ;   /* 1025 */
PacketLogAddPacket(G_serverPacketLog, &testPacket) ;
fprintf(fp, "---------------- SECOND DUMP ------------------\n") ;
fflush(fp) ;
PacketLogDump(G_serverPacketLog, fp) ;
fprintf(fp, "---------------- sub log     ------------------\n") ;
fflush(fp) ;
PacketLogDump(subLog, fp) ;
PacketLogDestroy(subLog) ;
#endif

    DebugEnd() ;
}

#endif

/****************************************************************************/
/*  Routine:  IGetObjectPositionPacketKey             * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetObjectPositionPacketKey returns the hash key for the given object */
/*  position packet.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj position */
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
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetObjectPositionPacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetObjectPositionPacketKey") ;

    key = (T_word32)(((T_objectPositionPacket *)(p_packet->data))->id) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetObjectAddPacketKey             * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetObjectAddPacketKey returns the hash key for the given object      */
/*  add packet.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj position */
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
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetObjectAddPacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetObjectAddPacketKey") ;

    key = (T_word32)(((T_objectAddPacket *)(p_packet->data))->objectID) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetProjectileAddPacketKey         * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetProjectileAddPacketKey returns the hash key for the given         */
/*  projectile add packet key.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing projectile   */
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
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetProjectileAddPacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetProjectileAddPacketKey") ;

    key = (T_word32)(((T_projectileAddPacket *)(p_packet->data))->objectID) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetObjectDestroyPacketKey         * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetObjectDestroyPacketKey returns the hash key for the given         */
/*  object destroy packet key.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj destroy  */
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
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetObjectDestroyPacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetObjectDestroyPacketKey") ;

    key = (T_word32)(((T_objectDestroyPacket *)(p_packet->data))->objectID) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IPostProcessDestroyObject          * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IPostProcessDestroyObject does all the work of deleting other packets */
/*  that are related to the object about to be destroyed.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj destroy  */
/*                                                                          */
/*    T_packetLog log             -- Log we are working in.                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogPacketFind                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IPostProcessDestroyObject(
                  T_packetEitherShortOrLong *p_packet,
                  T_packetLog log,
                  T_packetLogPacket self)
{
    E_Boolean foundCreatePacket = FALSE ;
    T_packetLogPacket packet ;
    T_word16 objectID ;

    DebugRoutine("IPostProcessDestroyObject") ;

    /* The key here is to find all the possible packets that */
    /* can be destroyed and get rid of them.  If no add or projectile */
    /* add packet is found, then the destroy must stay to get rid */
    /* of an object that is part of the original map object list. */

    /* Get the object's id that all references will be developed from. */
    objectID = ((T_objectDestroyPacket *)(p_packet->data))->objectID ;

    /* First, try to find the corresponding object position packet. */
    packet = PacketLogPacketFind(
                 log,
                 PACKET_COMMANDSC_OBJECT_POSITION,
                 objectID) ;
    if (packet != PACKET_LOG_PACKET_BAD)  {
        /* There is an object position packet for that. */
        /* Get rid of it. */
        PacketLogPacketDestroy(packet) ;
    }

    /* Second, try to find the corresponding object add/create packet. */
    packet = PacketLogPacketFind(
                 log,
                 PACKET_COMMANDCSC_ADD_OBJECT,
                 objectID) ;
    if (packet != PACKET_LOG_PACKET_BAD)  {
        /* There is an object create/add packet for that. */
        /* Get rid of it. */
        PacketLogPacketDestroy(packet) ;

        /* Note that we found the "pair" creation. */
        foundCreatePacket = TRUE ;
    }

    /* Third, try to find the corresponding projectile add/create packet. */
    packet = PacketLogPacketFind(
                 log,
                 PACKET_COMMANDCSC_PROJECTILE_CREATE,
                 objectID) ;
    if (packet != PACKET_LOG_PACKET_BAD)  {
        /* There is a projectile create packet for that. */
        /* Get rid of it. */
        PacketLogPacketDestroy(packet) ;

        /* Note that we found the "pair" creation. */
        foundCreatePacket = TRUE ;
    }

    /* If we found the "pair" creation, then we don't exist anymore ... */
    /* or at least we serve no purpose. */
    if (foundCreatePacket)  {
        PacketLogPacketDestroy(self) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ServerPacketLogGetUpdateBlockForPlayer                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogGetUpdateBlockForPlayer is the code that does all the  */
/*  work of keeping track of what player is updated when.  Just pass in a   */
/*  for the player and a place for the block size.  The key can be any      */
/*  32 bit value as long as it is unique per player.  A pointer to the      */
/*  player structure cast into a 32 bit value is the best choice.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 playerKey          -- Key to use for update                  */
/*                                                                          */
/*    T_word32 *p_blockSize       -- Pointer to place to put returned block  */
/*                                   size.                                  */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to created block, else NULL.   */
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
/*    LES  01/27/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *ServerPacketLogGetUpdateBlockForPlayer(
             T_word32 playerKey,
             T_word32 *p_blockSize)
{
    T_hash32Item hashItem ;
    T_byte8 *p_block = NULL ;
    T_packetLog playerLog ;

    DebugRoutine("ServerPacketLogGetUpdateBlockForPlayer") ;
    DebugCheck(p_blockSize != NULL) ;
    DebugCheck(G_isInit == TRUE) ;

    /* First, see if the player already has a packet log going. */
    hashItem = Hash32Find(G_playerPacketLogs, playerKey) ;

    if (hashItem == HASH32_ITEM_BAD)  {
        /* If no previous log has started, we are going to */
        /* start the player with an update.  His first update */
        /* will be from the master list -- so get that first. */
        p_block = PacketLogGetUpdateBlock(G_serverPacketLog, p_blockSize) ;

        /* If there is no block, we don't do anything ... there is */
        /* nothing to update and the player is not going to be */
        /* any different. */
        if (p_block)  {
            /* If there is a block, we need to start an update just */
            /* for this player.  All incremental updates will */
            /* go to this new log. */
            /* This new log is a split off the main server packet log. */
            playerLog = PacketLogSplit(G_serverPacketLog) ;

            /* Put it in the hash table so that the next call */
            /* to this routine will know where to carry on. */
            Hash32Add(G_playerPacketLogs, playerKey, (T_void *)playerLog) ;
        }
    } else {
        /* If there is already a player packet log in the hash */
        /* table, it is time to try to get the next update block. */
        /* Get the packet log out of the hash item. */
        playerLog = (T_packetLog)Hash32ItemGetData(hashItem) ;
        /* Get the update block. */
        p_block = PacketLogGetUpdateBlock(playerLog, p_blockSize) ;

        /* OK, now two things happened.  Either we got a block, or */
        /* did not. */
        if (p_block)  {
            /* If we did get the block, restart the log by destroying */
            /* the current one and creating another. */
            /* Remove it from the hash table. */
            Hash32Remove(hashItem) ;
            /* Destroy the packet log. */
            PacketLogDestroy(playerLog) ;
            /* Create the new empty one. */
            playerLog = PacketLogSplit(G_serverPacketLog) ;
            /* Put back into the hash table. */
            Hash32Add(G_playerPacketLogs, playerKey, (T_void *)playerLog) ;

            /* The block will be returned as non-NULL representing */
            /* that we have found changes and might actually */
            /* get more. */
        } else {
            /* If we did not get a block of data, then there are */
            /* no new updates.  Since we are apparently done with */
            /* updates currently, we will delete the packet log. */
            /* Remove it from the hash table. */
            Hash32Remove(hashItem) ;
            /* Destroy the packet log. */
            PacketLogDestroy(playerLog) ;
             
            /* The block will now return a NULL value saying */
            /* that nothing has changed since last time and */
            /* the update is complete.  If the calling routine */
            /* calls this again, the update will start from the */
            /* beginning state of the master log -- something */
            /* that should not be done after a complete update. */
        }
    }

    DebugEnd() ;

    return p_block ;
}

/****************************************************************************/
/*  Routine:  ServerPacketLogParseUpdateBlock                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ServerPacketLogParseUpdateBlock parses out a previously given packet  */
/*  log update into a group of functional calls to the given callback.      */
/*    NOte that this routine is practically the same as PacketLogParse...   */
/*  but this provides a central site to handle all the packet log stuff     */
/*  for our current applciation.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_block            -- Block to parse                         */
/*                                                                          */
/*    T_word32 blockSize          -- Size of the block to parse             */
/*                                                                          */
/*    T_packetLogUpdateCallback p_callback -- Callback to call on each pckt */
/*                                                                          */
/*    T_word32 extraData          -- Extra data to pass to the callback.    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogParseUpdateBlock                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/27/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ServerPacketLogParseUpdateBlock(
             T_byte8 *p_block,
             T_word32 blockSize,
             T_packetLogUpdateCallback p_callback,
             T_word32 extraData)
{
    DebugRoutine("ServerPacketLogParseUpdateBlock") ;

    /* Just pass on the request using the main list */
    /* of packet info. */
    PacketLogParseUpdateBlock(
        p_block,
        blockSize,
        PACKET_COMMAND_MAX,
        G_serverPacketLogParams,
        p_callback,
        extraData) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IGetChangeBodyPartPacketKey        * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetChangeBodyPartPacketKey returns the hash key for the given object */
/*  change body part packet                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj position */
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
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetChangeBodyPartPacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetChangeBodyPartPacketKey") ;

    key = (T_word32)(((T_changeBodyPartPacket *)
                       (p_packet->data))->objServerID) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetWallStateChangePacketKey       * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetWallStateChangePacketKey returns the hash key for the given object*/
/*  change body part packet                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing state        */
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
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetWallStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetWallStateChangePacketKey") ;

    key = (T_word32)(((T_wallStateChangePacket *)
                       (p_packet->data))->wallNum) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetSideStateChangePacketKey       * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetSideStateChangePacketKey returns the hash key for the given object*/
/*  change body part packet                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing state        */
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
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetSideStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetSideStateChangePacketKey") ;

    key = (T_word32)(((T_sideStateChangePacket *)
                       (p_packet->data))->sideNum) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetSectorStateChangePacketKey       * INTERNAL *             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetSectorStateChangePacketKey returns the hash key for the given object*/
/*  change body part packet                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing state        */
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
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetSectorStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetSectorStateChangePacketKey") ;

    key = (T_word32)(((T_sectorStateChangePacket *)
                       (p_packet->data))->sectorNum) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IGetGroupStateChangePacketKey       * INTERNAL *              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IGetGroupStateChangePacketKey returns the hash key for the given object*/
/*  change body part packet                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing state        */
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
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word32 IGetGroupStateChangePacketKey(
                    T_packetEitherShortOrLong *p_packet)
{
    T_word32 key ;

    DebugRoutine("IGetGroupStateChangePacketKey") ;

    key = (T_word32)(((T_groupStateChangePacket *)
                       (p_packet->data))->groupNum) ;

    DebugEnd() ;

    return key ;
}

/****************************************************************************/
/*  Routine:  IPostProcessChangeBodyPart         * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IPostProcessChangeBodyPart does all the work of merging this change   */
/*  body part packet with any previous object add packets.  If there is     */
/*  none, then this packet stays.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containing obj destroy  */
/*                                                                          */
/*    T_packetLog log             -- Log we are working in.                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogPacketFind                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/05/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IPostProcessChangeBodyPart(
                  T_packetEitherShortOrLong *p_packet,
                  T_packetLog log,
                  T_packetLogPacket self) 
{
    E_Boolean foundCreatePacket = FALSE ;
    T_packetLogPacket packet ;
    T_word16 objectID ;
    T_objectAddPacket *p_addPacket ;
    T_changeBodyPartPacket *p_bodyPartPacket ;

    DebugRoutine("IPostProcessChangeBodyPart") ;

    /* All that needs to be done is to check to see if there */
    /* is a matching add object packet.  If there is, apply the */
    /* change body part and then destroy ourself, otherwise */
    /* keep this packet. */

    /* Get the object's id that all references will be developed from. */
    p_bodyPartPacket = ((T_changeBodyPartPacket *)(p_packet->data)) ;
    objectID = p_bodyPartPacket->objServerID ;

puts("PPCBP: Looking for old Add Object") ;
    /* First, try to find the corresponding object position packet. */
    packet = PacketLogPacketFind(
                 log,
                 PACKET_COMMANDCSC_ADD_OBJECT,
                 objectID) ;
    if (packet != PACKET_LOG_PACKET_BAD)  {
puts("PPCBP: found") ;
        /* There is an object add packet for that. */
        /* Change the packet to reflect the change body part. */
        p_addPacket = (T_objectAddPacket *)
                           PacketLogPacketGetData(packet) ;

        p_addPacket->bodyParts[p_bodyPartPacket->bodyLocation] = 
            p_bodyPartPacket->newType ;
        if (p_bodyPartPacket->bodyLocation == BODY_PART_LOCATION_HEAD)
            p_addPacket->objectID = p_bodyPartPacket->objServerID ;

        /* Now destroy ourself.  We are no longer needed. */
        PacketLogPacketDestroy(self) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SERPLOG.C                                               */
/****************************************************************************/
