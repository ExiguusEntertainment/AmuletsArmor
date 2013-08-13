
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
        "PACKET_COMMAND_TOWN_UI_MESSAGE",         /*  3 */
        "PACKET_COMMAND_PLAYER_ID_SELF",          /*  4 */
        "PACKET_COMMAND_GAME_REQUEST_JOIN",       /*  5 */
        "PACKET_COMMAND_GAME_RESPOND_JOIN",       /*  6 */
        "PACKET_COMMAND_GAME_START",              /*  7 */
        "PACKET_COMMAND_SYNC",                    /*  8 */
        "PACKET_COMMAND_MESSAGE",                 /*  9 */

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
		fprintf(fp, "Illegal packet length!\n");
		return;
	}
	fprintf(fp, "#%d: %s", p_header->id, PacketName(p_packet->data[0]));
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
//		case PACKET_COMMANDCS_GOTO_SUCCEEDED:
//			{
//				T_gotoSucceededPacket *p = (T_gotoSucceededPacket *)p_packet->data;
//				fprintf(fp, "(location=%d place=%d)", p->startLocation, p->placeNumber);
//			}
//			break;
		default:
			fprintf(fp, "Unknown packet type #%d!\n", p_packet->data[0]);
			break;
	}
	fprintf(fp, "\n");
}

void PacketPrint(void *aData, unsigned int aSize)
{
    FILE *fp;

    fp = fopen("packets.txt", "a");
    if (fp) {
        PacketFPrint(fp, aData, aSize);
        fclose(fp);
    }
}
/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PACKETPR.C
 *-------------------------------------------------------------------------*/
