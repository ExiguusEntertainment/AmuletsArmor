/****************************************************************************/
/*    FILE:  CLI_SEND.H             Client Send Packet Module               */
/****************************************************************************/
#ifndef _CLI_SEND_H_
#define _CLI_SEND_H_

#include "GENERAL.H"
#include "PACKET.H"

T_void ClientSendMessage(T_byte8 *message);

T_void ClientSendRequestServerIDPacket(T_void);

T_void ClientSendRequestEnterPacket(T_void);

T_void ClientSendRequestCharacterList(T_void);

T_void ClientSendRequestCharacterListing(T_void);

T_void ClientSendLoadCharacter(T_byte8 slot, T_word32 checksum);

T_void ClientSendCreateCharacter(
        T_byte8 slot,
        T_word32 checksum,
        T_byte8 *p_password);

T_void ClientSendCheckPassword(
        T_byte8 slot,
        T_byte8 password[MAX_SIZE_PASSWORD]);

T_void ClientRequestDataBlock(T_word16 dataBlockType, T_word32 extraData);

T_void ClientRequestTake(T_3dObject *p_obj, E_Boolean autoStore);

T_void ClientRequestRetransmit(
        T_byte8 player,
        T_byte8 transmitStart,
        T_gameGroupID groupID,
        T_word16 destination);

T_void ClientSendGotoSucceeded(T_word16 placeNumber, T_word16 startLocation);

T_void ClientSendTownUIAddMessage(T_byte8 *p_msg);

T_void ClientSendRequestPlayerID(T_void);

T_void ClientSendPlayerIDSelf(T_void);

T_void ClientSendRespondToJoinPacket(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure,
        E_respondJoin response);

T_void ClientSendRequestJoin(T_word16 map, T_gameGroupID instance);

T_void ClientSendGameStartPacket(
        T_gameGroupID groupID,
        T_word16 adventure,
        T_byte8 numPlayers,
        T_directTalkUniqueAddress *players,
        T_word16 firstLevel);
#endif

/****************************************************************************/
/*    END OF FILE: CLI_SEND.H       Client Send Packet Module               */
/****************************************************************************/

