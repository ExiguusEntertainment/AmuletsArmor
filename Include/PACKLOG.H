/****************************************************************************/
/*    FILE:  PACKLOG.H                                                      */
/****************************************************************************/
#ifndef _PACKLOG_H_
#define _PACKLOG_H_

#include "GENERAL.H"
#include "PACKET.H"

typedef T_void *T_packetLog ;
#define PACKET_LOG_BAD NULL

typedef T_void *T_packetLogPacket ;
#define PACKET_LOG_PACKET_BAD NULL

typedef T_byte8 T_packetLogParamType ;
#define PACKET_LOG_PARAM_TYPE_IGNORE           0
#define PACKET_LOG_PARAM_TYPE_ONLY_MASTER_LOG  1
#define PACKET_LOG_PARAM_TYPE_ALL_LOGS         2

typedef T_word32 (*T_packetLogGetKeyCallback)
                       (T_packetEitherShortOrLong *p_packet) ;

typedef T_void (*T_packetLogPostProcess)
                       (T_packetEitherShortOrLong *p_packet,
                        T_packetLog log,
                        T_packetLogPacket self) ;

typedef struct {
    T_packetLogParamType type ;                   /* How to process this */
                                                  /* type of packet. */
    T_word16 size ;                               /* Byte size of packet. */
    T_packetLogPostProcess p_postProcess ;        /* Routine to possibly */
                                                  /* call after packet has */
                                                  /* been added. */
    T_packetLogGetKeyCallback p_getKeyCallback ;  /* Routine to call to */
                                                  /* return a unique 32 */
                                                  /* word. */
} T_packetLogParam ;

typedef T_void (*T_packetLogUpdateCallback)
                    (T_packetEitherShortOrLong *p_packet,
                     T_word32 extraData) ;

T_packetLog PacketLogCreate(
                T_word16 numPackets,
                T_packetLogParam *params) ;

T_void PacketLogDestroy(T_packetLog log) ;

T_packetLog PacketLogSplit(T_packetLog log) ;

T_byte8 *PacketLogGetUpdateBlock(T_packetLog log, T_word32 *p_size) ;

T_void PacketLogParseUpdateBlock(
           T_byte8 *p_block,
           T_word32 sizeBlock,
           T_word16 numPackets,
           T_packetLogParam *params,
           T_packetLogUpdateCallback p_callback,
           T_word32 extraData) ;

T_void PacketLogAddPacket(
           T_packetLog log,
           T_packetEitherShortOrLong *p_packet) ;

T_void PacketLogSave(T_packetLog log, T_byte8 *filename) ;

T_packetLog PacketLogLoad(
                T_byte8 *filename,
                T_word16 numPackets,
                T_packetLogParam *params) ;

/* Utility functions: */

T_packetLogPacket PacketLogPacketFind(
                      T_packetLog log,
                      T_word16 command,
                      T_word32 key) ;

T_byte8 *PacketLogPacketGetData(T_packetLogPacket logPacket) ;

T_void PacketLogPacketDestroy(T_packetLogPacket logPacket) ;

#ifndef NDEBUG
T_void PacketLogDump(T_packetLog log, FILE *fp) ;
#else
#define PacketLogDump(log, fp)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  PACKLOG.H                                               */
/****************************************************************************/
