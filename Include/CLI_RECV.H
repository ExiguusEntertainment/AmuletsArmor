/****************************************************************************/
/*    FILE:  CLI_RECV.H             Client Receive Packet Module            */
/****************************************************************************/
#ifndef _CLI_RECV_H_
#define _CLI_RECV_H_

#include "GENERAL.H"
#include "PACKET.H"

T_void ClientReceivePlaceStartPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerLogoffPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveMessagePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveGotoPlacePacket(T_packetEitherShortOrLong *p_gotoPacket) ;

T_void ClientReceiveServerIDPacket(T_packetEitherShortOrLong *p_packet);

T_void ClientReceiveSyncPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveTownUIMessagePacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerIDSelf(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveRequestPlayerIDPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveGameRequestJoinPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveGameRespondJoinPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveGameStartPacket(
           T_packetEitherShortOrLong *p_packet) ;

#endif

/****************************************************************************/
/*    END OF FILE: CLI_RECV.H       Client Send Packet Module               */
/****************************************************************************/

