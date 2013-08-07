
/*    FILE:  PACKETPR.C                                                     */
// Packet print utilities
#include <stdio.h>
#include "PACKET.H"
#include "CMDQUEUE.H"

const char *PacketName(unsigned char command)
{
	static const char *names[] = {
        "PACKET_COMMAND_ACK",                     /*  0 */
        "PACKET_COMMAND_LOGIN",                   /*  1 */
        "PACKET_COMMAND_RETRANSMIT",              /*  2 */
        "PACKET_COMMAND_MONSTER_MOVE",            /*  3  not used */
        "PACKET_COMMANDCS_PLAYER_ATTACK",         /*  4  not used */
        "PACKET_COMMAND_TOWN_UI_MESSAGE",         /*  5 */
        "PACKET_COMMAND_PLAYER_ID_SELF",          /*  6 */
        "PACKET_COMMAND_REQUEST_PLAYER_ID",       /*  7 */
        "PACKET_COMMAND_GAME_REQUEST_JOIN",       /*  8 */
        "PACKET_COMMAND_GAME_RESPOND_JOIN",       /*  9 */
        "PACKET_COMMAND_GAME_START",              /* 10 */
        "PACKET_COMMAND_FIREBALL_STOP",           /* 11  not used */
        "PACKET_COMMAND_MOVE_CREATURE",           /* 12  not used */
        "PACKET_COMMANDSC_DAMAGE",                /* 13  not used */
        "PACKET_COMMAND_CREATURE_ATTACK",         /* 14  not used */
        "PACKET_COMMAND_CREATURE_HURT",           /* 15  not used */
        "PACKET_COMMAND_CREATURE_DEAD",           /* 16  not used */
        "PACKET_COMMAND_REVERSE_SECTOR",          /* 17  not used */
        "PACKET_COMMAND_SYNC",                    /* 18 */
        "PACKET_COMMAND_PICK_UP",                 /* 19  not used */
        "PACKET_COMMAND_MESSAGE",                 /* 20 */
        "PACKET_COMMAND_OPEN_DOOR",               /* 21  not used */
        "PACKET_COMMAND_CANNED_SAYING",           /* 22  not used */
        "PACKET_COMMANDSC_OBJECT_POSITION",       /* 23 */
        "PACKET_COMMANDCS_REQUEST_TAKE",          /* 24  not used */
        "PACKET_COMMANDSC_TAKE_REPLY",            /* 25 */
        "PACKET_COMMANDCSC_ADD_OBJECT",           /* 26 */
        "PACKET_COMMANDSC_DESTROY_OBJECT",        /* 27  not used */
        "PACKET_COMMANDSC_SPECIAL_EFFECT",        /* 28  not used */
        "PACKET_COMMANDSC_PLACE_START",           /* 29 */
        "PACKET_COMMANDCSC_GOTO_PLACE",           /* 30 */
        "PACKET_COMMANDCS_GOTO_SUCCEEDED",        /* 31 */
        "PACKET_COMMANDCSC_PROJECTILE_CREATE",    /* 32  not used */

    /** File transfer packets.**/
    /**  RT = receiver->transmitter", TR = transmitter->receiver **/
        "PACKET_COMMANDRT_REQUEST_FILE",          /* 33 */
        "PACKET_COMMANDTR_START_TRANSFER",        /* 34 */
        "PACKET_COMMANDTR_DATA_PACKET",           /* 35 */
        "PACKET_COMMANDTR_FINAL_PACKET",          /* 36 */
        "PACKET_COMMANDRT_RESEND_PLEASE",         /* 37 */
        "PACKET_COMMANDRT_TRANSFER_COMPLETE",     /* 38 */
        "PACKET_COMMANDRT_TRANSFER_CANCEL",       /* 39 */
        "PACKET_COMMANDTR_FILE_NOT_HERE",         /* 40 */

        "PACKET_COMMANDCSC_REQUEST_MEMORY_TRANSFER", /* 41 */
        "PACKET_COMMANDCSC_MEMORY_TRANSFER_READY",   /* 42 */
        "PACKET_COMMANDCSC_MEMORY_TRANSFER_DATA",    /* 43 */

        "PACKET_COMMANDCSC_CHANGE_BODY_PART",      /* 44  not used */
        "PACKET_COMMANDCSC_PING",                  /* 45 */
        "PACKET_COMMANDSC_WALL_STATE_CHANGE",      /* 46  not used */
        "PACKET_COMMANDSC_SIDE_STATE_CHANGE",      /* 47  not used */
        "PACKET_COMMANDSC_SECTOR_STATE_CHANGE",    /* 48  not used */
        "PACKET_COMMANDSC_GROUP_STATE_CHANGE",     /* 49  not used */
        "PACKET_COMMANDSC_EXPERIENCE",             /* 50  not used */
        "PACKET_COMMANDCS_REQUEST_SERVER_ID",      /* 51 */
        "PACKET_COMMANDSC_SERVER_ID",              /* 52 */
        "PACKET_COMMANDCS_REQUEST_ENTER",          /* 53 */
        "PACKET_COMMANDSC_REQUEST_ENTER_STATUS",   /* 54 */
        "PACKET_COMMANDCS_REQUEST_CHAR_LIST",      /* 55 */
        "PACKET_COMMANDCS_LOAD_CHARACTER",         /* 56 */
        "PACKET_COMMANDSC_LOAD_CHARACTER_STATUS",  /* 57 */
        "PACKET_COMMANDCS_CREATE_CHARACTER",       /* 58 */
        "PACKET_COMMANDSC_CREATE_CHARACTER_STATUS",/* 59 */
        "PACKET_COMMANDCS_DELETE_CHARACTER",       /* 60 */

        "PACKET_COMMAND_UNKNOWN"
	};
	if (command > 74)
		command = 74;
	return names[command];
}

const char *PacketMACAsString(T_directTalkUniqueAddress *aAddress)
{
	static char string[20];

	sprintf(string, "@%02X%02X%02X%02X%02X%02X",
		aAddress->address[0],
		aAddress->address[1],
		aAddress->address[2],
		aAddress->address[3],
		aAddress->address[4],
		aAddress->address[5]);

	return string;
}

const char *PacketIDState(T_playerIDState aState)
{
	if (aState == PLAYER_ID_STATE_CREATING_GAME)
		return "Creating";
	if (aState == PLAYER_ID_STATE_JOINING_GAME)
		return "Joining";
	if (aState == PLAYER_ID_STATE_NONE)
		return "None";
	return "Unknown";
}

const char *PacketJoinResponse(E_respondJoin aResponse)
{
	if (aResponse == GAME_RESPOND_JOIN_OK)
		return "OK";
	if (aResponse == GAME_RESPOND_JOIN_FULL)
		return "Full";
	if (aResponse == GAME_RESPOND_JOIN_CANCELED)
		return "Cancelled";
	return "Unknown";
}

void PacketPrint(void *aData, unsigned int aSize)
{
	T_packetHeader *p_header = (T_packetHeader *)aData;
	T_packetEitherShortOrLong *p_packet = (T_packetEitherShortOrLong *)aData;
	int i;

	if (p_header->prefix != PACKET_PREFIX) {
		printf("Bad prefix!\n");
		return;
	}
	if (p_header->packetLength == SHORT_PACKET_LENGTH) {
		printf("S");
	} else if (p_header->packetLength == LONG_PACKET_LENGTH) {
		printf("L");
	} else {
		printf("Illegal packet length!\n");
		return;
	}
	printf("#%d: %s", p_header->id, PacketName(p_packet->data[0]));
	switch (p_packet->data[0]) {
		case PACKET_COMMAND_ACK:
			{
				T_ackPacket *p = (T_ackPacket *)p_packet->data;
				printf("(%s #%d)", PacketName(p->commandBeingAcked), p->packetIDBeingAcked);
			}
			break;
		case PACKET_COMMAND_TOWN_UI_MESSAGE:
			{
				T_townUIMessagePacket *p = (T_townUIMessagePacket *)p_packet->data;
				printf("(\"%s\" -> \"%s\")", p->name, p->msg);
			}
			break;
		case PACKET_COMMAND_PLAYER_ID_SELF:
			{
				T_playerIDSelfPacket *p = (T_playerIDSelfPacket *)p_packet->data;
				printf("(\"%s\" id=%s group=%d state=%s adv=%d)", p->id.name, PacketMACAsString(&p->id.uniqueAddress), PacketMACAsString(&p->id.groupID), PacketIDState(p->id.state), p->id.adventure);
			}
			break;
		case PACKET_COMMAND_GAME_REQUEST_JOIN:
			{
				T_gameRequestJoinPacket *p = (T_gameRequestJoinPacket *)p_packet->data;
				printf("(id=%s group=%d adv=%d)", PacketMACAsString(&p->uniqueAddress), PacketMACAsString(&p->groupID), p->adventure);
			}
			break;
		case PACKET_COMMAND_GAME_RESPOND_JOIN:
			{
				T_gameRespondJoinPacket *p = (T_gameRespondJoinPacket *)p_packet->data;
				printf("(id=%s group=%s adv=%d response=%s)", PacketMACAsString(&p->uniqueAddress), PacketMACAsString(&p->groupID), p->adventure, PacketJoinResponse(p->response));
			}
			break;
		case PACKET_COMMAND_GAME_START:
			{
				T_gameStartPacket *p = (T_gameStartPacket *)p_packet->data;
				printf("(group=%s adv=%d players={", PacketMACAsString(&p->groupID), p->groupID);
				for (i=0; i<p->numPlayers; i++) {
					if (i!=0)
						printf(" ");
					printf("%s", PacketMACAsString(&p->players[i]));
				}
				printf("} time=%d firstlevel=%d", p->timeOfDay, p->firstLevel);
			}
			break;
		case PACKET_COMMANDCS_GOTO_SUCCEEDED:
			{
				T_gotoSucceededPacket *p = (T_gotoSucceededPacket *)p_packet->data;
				printf("(location=%d place=%d)", p->startLocation, p->placeNumber);
			}
			break;
		case PACKET_COMMAND_REQUEST_PLAYER_ID:
			printf("()");
			break;
		default:
			printf("Unknown packet type #%d!\n", p_packet->data[0]);
			break;
	}
	printf("\n");
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PACKETPR.C
 *-------------------------------------------------------------------------*/
