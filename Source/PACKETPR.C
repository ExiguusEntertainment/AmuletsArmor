
/*    FILE:  PACKETPR.C                                                     */
// Packet print utilities
#include <stdio.h>
#include "PACKET.H"
#include "CMDQUEUE.H"
#include "SYNCPACK.H"

const char *PacketName(unsigned char command)
{
	static const char *names[] = {
        "PACKET_COMMAND_ACK",                     /*  0 */
        "PACKET_COMMAND_RETRANSMIT",              /*  1 */
        "PACKET_COMMAND_TOWN_UI_MESSAGE",         /*  2 */
        "PACKET_COMMAND_PLAYER_ID_SELF",          /*  3 */
        "PACKET_COMMAND_GAME_REQUEST_JOIN",       /*  4 */
        "PACKET_COMMAND_GAME_RESPOND_JOIN",       /*  5 */
        "PACKET_COMMAND_GAME_START",              /*  6 */
        "PACKET_COMMAND_SYNC",                    /*  7 */
        "PACKET_COMMAND_MESSAGE",                 /*  8 */

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

const char *PacketSenderName(T_directTalkUniqueAddress addr)
{
    static char sender[20];

    sprintf(sender, "%02X%02X%02X%02X%02X%02X", addr.address[0],
            addr.address[1], addr.address[2], addr.address[3], addr.address[4],
            addr.address[5]);

    return sender;
}

void PacketSyncFPrintfAction(FILE *fp, T_playerAction type, T_word16 actionData[4])
{
    const char *actions[PLAYER_ACTION_UNKNOWN] = {
        "NONE", /* 0 */
        "CHANGE_SELF", /* 1 */
        "MELEE_ATTACK", /* 2 */
        "MISSILE_ATTACK", /* 3 */
        "ACTIVATE_FORWARD", /* 4 */
        "LEAVE_LEVEL", /* 5 */
        "PICKUP_ITEM", /* 6 */
        "THROW_ITEM", /* 7 */
        "STEAL", /* 8 */
        "STOLEN", /* 9 */
        "AREA_SOUND", /* 10 */
        "SYNC_NEW_PLAYER_WAIT", /* 11 */
        "SYNC_NEW_PLAYER_SEND", /* 12 */
        "SYNC_COMPLETE", /* 13 */
        "GOTO_PLACE", /* 14 */
        "DROP_AT", /* 15 */
        "PAUSE_GAME_TOGGLE", /* 16 */
        "ABORT_LEVEL", /* 17 */
        "PICK_LOCK", /* 18 */
        "ID_SELF", /* 19 */
    };
    if (type < PLAYER_ACTION_UNKNOWN)
        fprintf(fp, "%s", actions[type]);
    else
        fprintf(fp, "UNKNOWN#%d", type);

    fprintf(fp, "[%d, %d, %d, %d]", actionData[0], actionData[1], actionData[2], actionData[3]);
}

void PacketSyncFPrintf(FILE *fp, T_syncronizePacket *p)
{
    fprintf(fp,
            "(syncNumber=%d deltaTime=%d objID=%d xyz=%d,%d,%d angle=%d stance=0x%02X",
            p->syncNumber, p->deltaTime, p->playerObjectId, p->x, p->y, p->z,
            p->angle, p->stanceAndVisibility);
    if (p->fieldsAvailable & SYNC_PACKET_FIELD_ACTION) {
        fprintf(fp, " action=");
        PacketSyncFPrintfAction(fp, p->actionType, p->actionData);
    }
#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
    fprintf(fp, " nextid=%d nextwhen=%d", p->nextObjectId, p->nextObjectIdWhen);
#endif
    fprintf(fp, ")");
}

void PacketFPrint(FILE *fp, void *aData, unsigned int aSize)
{
	T_packetHeader *p_header = (T_packetHeader *)aData;
	T_packetEitherShortOrLong *p_packet = (T_packetEitherShortOrLong *)aData;
	int i;

	if (p_header->prefix != PACKET_PREFIX) {
		fprintf(fp, "Bad prefix!\n");
		return;
	}
	if (p_header->packetLength == SHORT_PACKET_LENGTH) {
		fprintf(fp, "S");
	} else if (p_header->packetLength == LONG_PACKET_LENGTH) {
		fprintf(fp, "L");
	} else {
	    // special csyncpacket
		fprintf(fp, "s");
	}
	fprintf(fp, "#%d: %s:%s", p_header->id, PacketSenderName(p_header->sender), PacketName(p_packet->data[0]));
	switch (p_packet->data[0]) {
		case PACKET_COMMAND_ACK:
			{
				T_ackPacket *p = (T_ackPacket *)p_packet->data;
				fprintf(fp, "(%s #%d)", PacketName(p->commandBeingAcked), p->packetIDBeingAcked);
			}
			break;
		case PACKET_COMMAND_TOWN_UI_MESSAGE:
			{
				T_townUIMessagePacket *p = (T_townUIMessagePacket *)p_packet->data;
				fprintf(fp, "(\"%s\" -> \"%s\")", p->name, p->msg);
			}
			break;
		case PACKET_COMMAND_PLAYER_ID_SELF:
			{
				T_playerIDSelfPacket *p = (T_playerIDSelfPacket *)p_packet->data;
				fprintf(fp, "(\"%s\" id=%s group=%d state=%s adv=%d)", p->id.name, PacketMACAsString(&p->id.uniqueAddress), PacketMACAsString(&p->id.groupID), PacketIDState(p->id.state), p->id.adventure);
			}
			break;
		case PACKET_COMMAND_GAME_REQUEST_JOIN:
			{
				T_gameRequestJoinPacket *p = (T_gameRequestJoinPacket *)p_packet->data;
				fprintf(fp, "(id=%s group=%d adv=%d)", PacketMACAsString(&p->uniqueAddress), PacketMACAsString(&p->groupID), p->adventure);
			}
			break;
		case PACKET_COMMAND_GAME_RESPOND_JOIN:
			{
				T_gameRespondJoinPacket *p = (T_gameRespondJoinPacket *)p_packet->data;
				fprintf(fp, "(id=%s group=%s adv=%d response=%s)", PacketMACAsString(&p->uniqueAddress), PacketMACAsString(&p->groupID), p->adventure, PacketJoinResponse(p->response));
			}
			break;
		case PACKET_COMMAND_GAME_START:
			{
				T_gameStartPacket *p = (T_gameStartPacket *)p_packet->data;
				fprintf(fp, "(group=%s adv=%d players={", PacketMACAsString(&p->groupID), p->groupID);
				for (i=0; i<p->numPlayers; i++) {
					if (i!=0)
						fprintf(fp, " ");
					fprintf(fp, "%s", PacketMACAsString(&p->players[i]));
				}
				fprintf(fp, "} time=%d firstlevel=%d", p->timeOfDay, p->firstLevel);
			}
			break;
		case PACKET_COMMAND_SYNC:
            {
                T_syncPacket *p_sync = (T_syncPacket *)p_packet->data;
                T_syncronizePacket *p = (T_syncronizePacket *)(p_sync->syncData) ;
                PacketSyncFPrintf(fp, p);
            }
		    break;
		default:
			fprintf(fp, "Unknown packet type #%d!", p_packet->data[0]);
			break;
	}
	fprintf(fp, "\n");
}

void PacketPrint(void *aData, unsigned int aSize)
{
#if WIN32
    //PacketFPrint(stdout, aData, aSize);
#elif 0
    FILE *fp;

    fp = fopen("packets.txt", "a");
    if (fp) {
        PacketFPrint(fp, aData, aSize);
        fclose(fp);
    }
#endif
}
/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PACKETPR.C
 *-------------------------------------------------------------------------*/
