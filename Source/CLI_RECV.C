/*-------------------------------------------------------------------------*
 * File:  CLI_RECV.C
 *-------------------------------------------------------------------------*/
/**
 * The client/server interface is broken into a send and receive side.
 * This is the receive/process side.  Responses from the server/world will
 * come back on the here and be fully processed.  It implies that the
 * actions have been synchronized with all the other players and is really
 * going to happen.
 *
 * @addtogroup CLI_RECV
 * @brief Client Receive Packet Communications
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CMDQUEUE.H"
#include "CSYNCPCK.H"
#include "GENERAL.H"
#include "GUILDUI.H"
#include "MAP.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PEOPHERE.H"
#include "PLAYER.H"
#include "SMCCHOOS.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "TOWNUI.H"

/*-------------------------------------------------------------------------*
 * Routine:  ClientReceivePlaceStartPacket
 *-------------------------------------------------------------------------*/
/**
 *  ClientReceiveLoginPacket tells the client that the server has just
 *  allowed the client to login.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientReceivePlaceStartPacket(T_packetEitherShortOrLong *p_packet)
{
    T_placeStartPacket *p_start ;
    extern E_Boolean G_serverActive ;

    DebugRoutine("ClientReceivePlaceStartPacket") ;

    DebugCheck (ClientIsInit() == TRUE);

    p_start = (T_placeStartPacket *)p_packet->data ;
    ClientStartPlayer(p_start->objectId, p_start->loginId) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientReceivePlayerLogoffPacket
 *-------------------------------------------------------------------------*/
/**
 *  ClientReceivePlayerLogoffPacket tells the client that one of the
 *  players have left this group.  Remove all the associated information.
 *
 *  @param p_packet -- logoff packet
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientReceivePlayerLogoffPacket(T_packetEitherShortOrLong *p_packet)
{
    T_logoffPacket *p_logoff ;
    T_3dObject *p_obj ;

    DebugRoutine("ClientReceivePlayerLogoffPacket") ;

    if ((ClientIsAttemptingLogout() == FALSE) &&
        (ClientIsActive() == TRUE))  {
        p_logoff = (T_logoffPacket *)p_packet->data ;

        /* Find the other player's object. */
        p_obj = ObjectFind(p_logoff->objectId) ;

        /* If we found it, remove and destroy it. */
        if (p_obj != NULL)  {
            ObjectRemove(p_obj) ;
            ObjectDestroy(p_obj) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientReceiveMessagePacket
 *-------------------------------------------------------------------------*/
/**
 *  ClientReceiveMessagePacket is called when someone sends a message
 *  about someone saying something.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientReceiveMessagePacket(T_packetEitherShortOrLong *p_packet)
{
    T_messagePacket *p_msg ;
    T_gameGroupID groupID ;

    DebugRoutine("ClientReceiveMessagePacket") ;

    if ((ClientIsAttemptingLogout() == FALSE) &&
        (ClientIsActive() == TRUE))  {
        /* Get a quick pointer. */
        p_msg = (T_messagePacket *)(p_packet->data) ;

        groupID = ClientSyncGetGameGroupID() ;
        if (CompareGameGroupIDs(p_msg->groupID, groupID))
        {
            MessageAdd(p_msg->message) ;
            SoundDing() ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientReceiveGotoPlacePacket
 *-------------------------------------------------------------------------*/
/**
 *  ClientReceiveGotoPlacePacket is the routine that stops the current
 *  map and goes to another.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientReceiveGotoPlacePacket(T_packetEitherShortOrLong *p_gotoPacket)
{
//    T_gotoPlacePacket *p_packet ;
//
//    DebugRoutine("ClientReceiveGotoPlacePacket") ;
//puts("ClientReceiveGotoPlacePacket");
//
//    if (ClientIsAttemptingLogout() == FALSE)  {
//        /* Get a quick pointer. */
//        p_packet = (T_gotoPlacePacket *)p_gotoPacket->data ;
//
//        ClientForceGotoPlace(p_packet->placeNumber, p_packet->startLocation) ;
//    }
//
//    DebugEnd() ;
}

/* LES: 06/12/96 */
T_void ClientReceiveSyncPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_syncPacket *p_sync ;
    T_syncronizePacket *p_syncro ;
    T_gameGroupID groupID ;

    DebugRoutine("ClientReceiveSyncPacket") ;

    if (ClientIsActive())  {
        p_sync = (T_syncPacket *)(p_packet->data) ;

        /* See if this is the correct group. */
        groupID = ClientSyncGetGameGroupID() ;
//printf("Us--- %02X:%02X:%02X:%02X:%02X:%02X\n", groupID.address[0],groupID.address[1],groupID.address[2],groupID.address[3],groupID.address[4],groupID.address[5]) ;
//printf("Them- %02X:%02X:%02X:%02X:%02X:%02X\n", p_sync->groupID.address[0],p_sync->groupID.address[1],p_sync->groupID.address[2],p_sync->groupID.address[3],p_sync->groupID.address[4],p_sync->groupID.address[5]) ;
        if (!(CompareGameGroupIDs(p_sync->groupID, *DirectTalkGetNullBlankUniqueAddress())))  {
            if (CompareGameGroupIDs(p_sync->groupID, groupID))  {
                p_syncro = (T_syncronizePacket *)(p_sync->syncData) ;

//puts("process") ;
                ClientSyncPacketProcess(p_syncro) ;
            }
        }
    }

    DebugEnd() ;
}

T_void ClientReceiveTownUIMessagePacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_townUIMessagePacket *p_msg ;

    DebugRoutine("ClientReceiveTownUIMessagePacket") ;

//puts("ClientReceiveTownUIMessagePacket") ;  fflush(stdout) ;
    /* Ignore unless we have the town ui screen up and running. */
    if (TownUIIsOpen() == TRUE)  {
        p_msg = (T_townUIMessagePacket *)(p_packet->data) ;

//printf("UI Msg: '%s' -- '%s'\n", p_msg->name, p_msg->msg) ;  fflush(stdout) ;
        /* Add the message. */
        TownUIAddMessage(p_msg->name, p_msg->msg);
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientReceivePlayerIDSelf
 *-------------------------------------------------------------------------*/
/**
 *  A player has sent a PLAYER_ID_SELF packet.  Update the state of that
 *  player.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientReceivePlayerIDSelf(T_packetEitherShortOrLong *p_packet)
{
    T_playerIDSelfPacket *p_self;

    DebugRoutine("ClientReceivePlayerIDSelf");

    p_self = (T_playerIDSelfPacket *)(p_packet->data);

    /* Update somebody. */
    PeopleHereUpdatePlayer(&p_self->id);

    DebugEnd();
}

T_void ClientReceiveGameRequestJoinPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameRequestJoinPacket *p_request ;

    DebugRoutine("ClientReceiveGameRequestJoinPacket") ;

    /* Get a quick pointer. */
    p_request = (T_gameRequestJoinPacket *)(p_packet->data) ;

//printf("Receive request join %d %d\n", p_request->groupID, p_request->adventure) ;  fflush(stdout) ;
    /* Just pass on the request to another routine (better suited). */
    PeopleHereRequestJoin(
        p_request->uniqueAddress,
        p_request->groupID,
        p_request->adventure) ;

    DebugEnd() ;
}

T_void ClientReceiveGameRespondJoinPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameRespondJoinPacket *p_request ;

    DebugRoutine("ClientReceiveGameRespondJoinPacket") ;

    /* Only process this packet if we are in the guild and are trying */
    /* to join a game. */
//printf("Receive respond join ... \n") ;  fflush(stdout) ;
    if (PeopleHereGetOurState() == PLAYER_ID_STATE_JOINING_GAME)  {
        p_request = (T_gameRespondJoinPacket *)(p_packet->data) ;

        PeopleHereRespondToJoin(
            p_request->uniqueAddress,
            p_request->groupID,
            p_request->adventure,
            p_request->response) ;
    } else {
//puts("... nope") ;  fflush(stdout) ;
    }

    DebugEnd() ;
}

T_word16 G_CompletedPlayers;
T_word16 G_SuccessPlayers;

T_void ClientReceiveGameStartPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameStartPacket *p_start ;
    T_word16 i ;
    T_directTalkUniqueAddress ourAddress ;
    T_gameGroupID groupID ;
	T_word16 levelStatus = LEVEL_STATUS_STARTED;

    DebugRoutine("ClientReceiveGameStartPacket") ;

//puts("ClientReceiveGameStartPacket") ;
    p_start = (T_gameStartPacket *)(p_packet->data) ;

    /* Set up the time offset. */
    MapSetDayOffset(p_start->timeOfDay) ;

    /* Only consider the packet if we are in the same group. */
//printf("-- %d %d\n", ClientSyncGetGameGroupID(), p_start->groupID) ;
    groupID = ClientSyncGetGameGroupID() ;
    if (CompareGameGroupIDs(groupID, p_start->groupID))  {
        DirectTalkGetUniqueAddress(&ourAddress) ;

        /* See if we are in the list. */
        for (i=0; i<p_start->numPlayers; i++)  {
            if (memcmp(&ourAddress, &p_start->players[i], sizeof(ourAddress)) == 0)
                break ;
        }
		//Level complete packet
		if (p_start->firstLevel == LEVEL_STATUS_LEVEL_CODE_COMPLETE ||
			p_start->firstLevel == LEVEL_STATUS_LEVEL_CODE_SUCCESS)
		{
			G_CompletedPlayers++;
			if (p_start->firstLevel == LEVEL_STATUS_LEVEL_CODE_SUCCESS)
				G_SuccessPlayers++;

			//if we got complete packets from everyone
			if (G_CompletedPlayers == p_start->numPlayers)
			{
				levelStatus = LEVEL_STATUS_COMPLETE;
				if (G_SuccessPlayers > 0)
					levelStatus |= LEVEL_STATUS_SUCCESS;

				TownUIFinishedQuest(levelStatus, p_start->numPlayers, StatsGetCurrentQuestNumber());

				//once all players respond, quit the game
				ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress());
				PeopleHereSetOurAdventure(0);
				PeopleHereSetOurState(PLAYER_ID_STATE_NONE);
			}
		}
		/* Did we find a match? */
		else if (i != p_start->numPlayers)  
		{
			//Reset number of players completed
			G_CompletedPlayers = 0;
			G_SuccessPlayers = 0;

			//puts("Matched") ;
			/* Yes, we are on the list in the ith position. */
			/* Set up the game setup and go. */
			ClientSyncSetNumberPlayers(p_start->numPlayers);
			ClientSetLoginId(i);

			/* Set up the next jump */
			ClientSetNextPlace(p_start->firstLevel, 0);
			ClientSetAdventureNumber(p_start->adventure);
			ClientSyncInitPlayersHere();

			/* Clear all the messages */
			MessageClear();

			/* Copy all the player address over to the peophere module. */
			for (i = 0; i < p_start->numPlayers; i++)
			{
				PeopleHereSetUniqueAddr(i, (&p_start->players[i]));
			}
		}
		else 
		{
			//puts("Not Matched") ;
			/* Make sure we are still in the guild and if so drop us. */
			if ((PeopleHereGetOurLocation() == PLAYER_ID_LOCATION_GUILD) &&
				(PeopleHereGetOurState() == PLAYER_ID_STATE_JOINING_GAME))  {
				/* Cancel that game we were joining, it left without us. */
				GuildUICancelJoinGame(NULL);

				/* Nope, we got excluded. */
				MessageAdd("Game started without you.");
			}
		}
    }

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CLI_RECV.C
 *-------------------------------------------------------------------------*/
