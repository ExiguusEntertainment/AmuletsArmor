/****************************************************************************/
/*    FILE:  SERPLOG.H                                                      */
/****************************************************************************/
#ifndef _SERPLOG_H_
#define _SERPLOG_H_

#include "GENERAL.H"
#include "PACKET.H"
#include "PACKLOG.H"

T_void ServerPacketLogInitialize(T_void) ;

T_void ServerPacketLogFinish(T_void) ;

T_void ServerPacketLogAddPacket(T_packetEitherShortOrLong *p_packet) ;

T_byte8 *ServerPacketLogGetUpdateBlockForPlayer(
             T_word32 playerKey,
             T_word32 *p_blockSize) ;

T_void ServerPacketLogParseUpdateBlock(
             T_byte8 *p_block,
             T_word32 blockSize,
             T_packetLogUpdateCallback p_callback,
             T_word32 extraData) ;

#ifndef NDEBUG
T_void ServerPacketLogDump(FILE *fp) ;
#else
#define ServerPacketLogDump(fp)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  SERPLOG.H                                               */
/****************************************************************************/
