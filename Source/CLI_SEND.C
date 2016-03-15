/*-------------------------------------------------------------------------*
 * File:  CLI_SEND.C
 *-------------------------------------------------------------------------*/
/**
 * The client/server interface is broken into a send and receive side.
 * This is the send/request side.  Responses from the server/world will
 * come back on the receive side later when ready to be processed.
 *
 * @addtogroup CLI_SEND
 * @brief Client Send Packet Communications
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "CMDQUEUE.H"
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CSYNCPCK.H"
#include "DITALK.H"
#include "EQUIP.H"
#include "GENERAL.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PACKET.H"
#include "PEOPHERE.H"
#include "PLAYER.H"
#include "SMCCHOOS.H"
#include "STATS.H"

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendMessage
 *-------------------------------------------------------------------------*/
/**
 *  ClientSendMessage sends the message that the user just typed in across
 *  the network and to everyone else in range.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendMessage(T_byte8 *message)
{
    T_packetLong packet;
    T_messagePacket *p_msg;
    char buffer[100];

    DebugRoutine("ClientSendMessage");

    /* Get a quick pointer. */
    p_msg = (T_messagePacket *)packet.data;

    /* Put the message in the packet. */
    p_msg->command = PACKET_COMMAND_MESSAGE;
    p_msg->player = ClientGetLoginId();
    p_msg->groupID = ClientSyncGetGameGroupID();
//    strcpy(p_msg->message, message) ;
    sprintf(buffer, "%s: %s", StatsGetName(), message);
    sprintf(p_msg->message, "%-50.50s", buffer);

    /* Send the whole packet to several people */
    PeopleHereSendPacketToGroup(ClientSyncGetGameGroupID(), &packet, 280, 0, NULL);

    /* Send it back to ourselves so we have a running log. */
    ClientReceiveMessagePacket((T_packetEitherShortOrLong *)&packet);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientRequestTake
 *-------------------------------------------------------------------------*/
/**
 *  ClientRequestTake queues up a request to take an item.  It isn't
 *  taken until the csync packet goes through.  There will be a small
 *  delay until that happens.
 *
 *  @param p_obj -- Object to request taking.
 *  @param autoStore -- Flag TRUE if Automatically store the item
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientRequestTake(T_3dObject *p_obj, E_Boolean autoStore)
{
    T_word16 dist;
    T_sword16 z;
    T_word16 num;
    T_wallListItem wallList[20];

    DebugRoutine("ClientRequestTake");

    /* See if the object exists and is grabable. */
    if ((p_obj != NULL)
            && (ObjectGetAttributes (p_obj) & OBJECT_ATTR_GRABABLE)) {
        /* Object is not grabable. */
        /* Is the object close enough? */
        if ((dist = CalculateDistance(ObjectGetX16(p_obj),
        ObjectGetY16(p_obj),
        PlayerGetX16(),
        PlayerGetY16())) < PLAYER_GRAB_DISTANCE) {
            /* Find a mutual z height. */
            z = ObjectGetZ16(p_obj);
            if (z < PlayerGetZ16())
                z += ObjectGetHeight(p_obj);

            /* Make sure there is nothing blocking the hit */
            num = Collide3dFindWallList(
            PlayerGetX16(),
            PlayerGetY16(),
            ObjectGetX16(p_obj),
            ObjectGetY16(p_obj), z, 20, wallList,
            WALL_LIST_ITEM_MAIN);
            if (num == 0) {
                /* No walls are in the way. */

                /* Object is close enough.  Attempt to take it. */
                ClientSyncSendActionPickUpItem(ObjectGetServerId(p_obj),
                        autoStore);
            } else {
                MessageAdd("Cannot grab through wall.");
            }
        } else {
            /* Object is too far. */
            MessageAdd("Object too far away to take.");
        }
    } else {
        /* Not grabbable, tell the player. */
        MessageAdd("Object can not be taken.");
    }
    DebugEnd ();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientRequestRetransmit
 *-------------------------------------------------------------------------*/
/**
 *  ClientRequestRetransmit sends out a request to get back in sync
 *  with a particular player.
 *
 *  @param player -- Player being request to retrans
 *  @param transmitStart -- Packet to start retransmiting from
 *  @param groupID -- ID of this game group
 *  @param destination -- Desired destination
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientRequestRetransmit(
        T_byte8 player,
        T_byte8 transmitStart,
        T_gameGroupID groupID,
        T_word16 destination)
{
    T_packetShort packet;
    T_retransmitPacket *p_request;

    DebugRoutine("ClientRequestRetransmit");

    /* Fill out the request. */
    p_request = (T_retransmitPacket *)(packet.data);
    p_request->command = PACKET_COMMAND_RETRANSMIT;
    p_request->fromPlayer = ObjectGetServerId(PlayerGetObject()) - 9000;
    p_request->toPlayer = player;
    p_request->transmitStart = transmitStart;
    p_request->groupID = groupID;

    /* Send out the request. */
//    CmdQSendShortPacket(&packet, 140, 0, NULL) ;
    DirectTalkSetDestination(PeopleHereGetUniqueAddr(destination));
    PacketSendShort(&packet);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendTownUIAddMessage
 *-------------------------------------------------------------------------*/
/**
 *  ClientSendTownUIAddMessage tells everyone in the pub/town to add a
 *  message.
 *
 *  @param p_msg -- Message to send
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendTownUIAddMessage(T_byte8 *p_msg)
{
    T_townUIMessagePacket *p_msgPacket;
    T_packetLong packet;
    T_directTalkUniqueAddress townGroup;

    DebugRoutine("ClientSendTownUIAddMessage");

    townGroup = *DirectTalkGetNullBlankUniqueAddress();

    p_msgPacket = (T_townUIMessagePacket *)(packet.data);
    memset(p_msgPacket, 0, sizeof(T_townUIMessagePacket));
    p_msgPacket->command = PACKET_COMMAND_TOWN_UI_MESSAGE;
    strncpy((char *)p_msgPacket->name, (const char *)StatsGetName(), 29);
    strncpy((char *)p_msgPacket->msg, (char *)p_msg, 39);
//    CmdQSendLongPacket(&packet, 0, 600, 0, NULL);
    PeopleHereSendPacketToGroup(townGroup, &packet, 140, 0, NULL);

    ClientReceiveTownUIMessagePacket((T_packetEitherShortOrLong *)&packet);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendPlayerIDSelf
 *-------------------------------------------------------------------------*/
/**
 *  ClientSendPlayerIDSelf is used for several things.  Mainly it is used
 *  to identify a player's location at particular times.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendPlayerIDSelf(T_void)
{
    T_packetLong packet;
    T_playerIDSelfPacket *p_self;

    DebugRoutine("ClientSendPlayerIDSelf");

    memset(&packet, 0, sizeof(packet));
    p_self = (T_playerIDSelfPacket *)(packet.data);
    PeopleHereGetPlayerIDSelfStruct(&p_self->id);
    p_self->command = PACKET_COMMAND_PLAYER_ID_SELF;

    CmdQSendLongPacket(&packet, 0, 600, 0, NULL);

    // Also, put ourself on the list
    ClientReceivePlayerIDSelf((T_packetEitherShortOrLong *)&packet);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendRespondToJoinPacket
 *-------------------------------------------------------------------------*/
/**
 *  This routine is used by a creator of a game to respond to a players
 *  request to enter an already existing game.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendRespondToJoinPacket(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure,
        E_respondJoin response)
{
    T_packetLong packet;
    T_gameRespondJoinPacket *p_respond;

    DebugRoutine("ClientSendRespondToJoinPacket");

    /* Make a packet and fill it out. */
    p_respond = (T_gameRespondJoinPacket *)packet.data;
    p_respond->command = PACKET_COMMAND_GAME_RESPOND_JOIN;
    p_respond->uniqueAddress = uniqueAddress;
    p_respond->groupID = groupID;
    p_respond->adventure = adventure;
    p_respond->response = response;

//printf("Send respond to join %d\n", response) ;
    CmdQSendLongPacket(&packet, &uniqueAddress, 140, 0, NULL);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendRequestJoin
 *-------------------------------------------------------------------------*/
/**
 *  This routine is used by a player wanting to join a game already
 *  created.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendRequestJoin(T_word16 map, T_gameGroupID instance)
{
    T_packetLong packet;
    T_gameRequestJoinPacket *p_request;

    DebugRoutine("ClientSendRequestJoin");

    /* Fill a packet to request joining a game. */
    p_request = (T_gameRequestJoinPacket *)packet.data;
    p_request->command = PACKET_COMMAND_GAME_REQUEST_JOIN;
    DirectTalkGetUniqueAddress(&p_request->uniqueAddress);
    p_request->groupID = instance;
    p_request->adventure = map;

//printf("Send request to join %d ", map) ;
//DirectTalkPrintAddress(stdout, &instance) ;
//puts(".") ;

    CmdQSendLongPacket(&packet, &instance, 140, 0, NULL);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSendGameStartPacket
 *-------------------------------------------------------------------------*/
/**
 *  This routine is used by a player starting up a game that has already
 *  been constructed.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSendGameStartPacket(
        T_gameGroupID groupID,
        T_word16 adventure,
        T_byte8 numPlayers,
        T_directTalkUniqueAddress *players,
        T_word16 firstLevel,
		T_word16 levelstatus)
{
    T_packetLong packet;
    T_gameStartPacket *p_start;

    DebugRoutine("ClientSendGameStartPacket");

    p_start = (T_gameStartPacket *)(packet.data);

    p_start->command = PACKET_COMMAND_GAME_START;
    p_start->groupID = groupID;
    p_start->adventure = adventure;
    p_start->numPlayers = numPlayers;
    p_start->players[0] = players[0];
    p_start->players[1] = players[1];
    p_start->players[2] = players[2];
    p_start->players[3] = players[3];
    p_start->timeOfDay = MapGetDayOffset();
    if (firstLevel == 0)
        firstLevel = adventure;
	if (levelstatus & LEVEL_STATUS_COMPLETE)
	{
		if (levelstatus & LEVEL_STATUS_SUCCESS)
			p_start->firstLevel = LEVEL_STATUS_LEVEL_CODE_SUCCESS;
		else
			p_start->firstLevel = LEVEL_STATUS_LEVEL_CODE_COMPLETE;		
	}
	else
	{
		p_start->firstLevel = firstLevel;
	}

//printf("Send request to start %d %d\n", groupID, adventure) ;
    PeopleHereSendPacketToGroup(groupID, &packet, 140, 0, NULL);

    ClientReceiveGameStartPacket((T_packetEitherShortOrLong *)&packet);

    DebugEnd();
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CLI_SEND.C
 *-------------------------------------------------------------------------*/
