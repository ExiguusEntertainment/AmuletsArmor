/****************************************************************************/
/*    FILE:  CMDQUEUE.H                                                      */
/****************************************************************************/

#ifndef _CMDQUEUE_H_
#define _CMDQUEUE_H_

#include "GENERAL.H"
#include "PACKET.H"
#include "DITALK.H"

typedef enum {
    PACKET_COMMAND_ACK,                     /*  0 */
    PACKET_COMMAND_RETRANSMIT,              /*  1 */
    PACKET_COMMAND_TOWN_UI_MESSAGE,         /*  2 */
    PACKET_COMMAND_PLAYER_ID_SELF,          /*  3 */
    PACKET_COMMAND_GAME_REQUEST_JOIN,       /*  4 */
    PACKET_COMMAND_GAME_RESPOND_JOIN,       /*  5 */
    PACKET_COMMAND_GAME_START,              /*  6 */
    PACKET_COMMAND_SYNC,                    /*  7 */
    PACKET_COMMAND_MESSAGE,                 /*  8 */

    PACKET_COMMAND_UNKNOWN=10
} E_packetCommand ;

#define LEVEL_STATUS_STARTED				0
#define LEVEL_STATUS_COMPLETE				1
#define LEVEL_STATUS_SUCCESS				2
#define LEVEL_STATUS_LEVEL_CODE_COMPLETE	USHRT_MAX
#define LEVEL_STATUS_LEVEL_CODE_SUCCESS		(USHRT_MAX - 1)

typedef T_void *T_packetHandle ;

#define PACKET_COMMAND_MAX PACKET_COMMAND_UNKNOWN
#define BAD_PACKET_HANDLE NULL

typedef T_void (*T_cmdQActionRoutine)(T_packetEitherShortOrLong *p_packet) ;

typedef T_void (*T_cmdQPacketCallback)
                   (T_word32 extraData,
                    T_packetEitherShortOrLong *p_packet) ;

T_void CmdQInitialize(T_void) ;

T_void CmdQFinish(T_void) ;

T_void CmdQSendShortPacket(
            T_packetShort *p_packet,
            T_directTalkUniqueAddress *p_destination,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

T_void CmdQSendLongPacket(
            T_packetLong *p_packet,
            T_directTalkUniqueAddress *p_destination,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

T_void CmdQSendPacket(
            T_packetEitherShortOrLong *p_packet,
            T_directTalkUniqueAddress *p_destination,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

T_void CmdQUpdateAllSends(T_void) ;

T_void CmdQUpdateAllReceives(T_void) ;

T_void CmdQRegisterClientCallbacks (T_cmdQActionRoutine *callbacks);

T_void CmdQDisableClientPacket(T_word16 packetNum) ;

T_void CmdQClearAllPorts(T_void) ;
T_void CmdQClearPort(T_void) ;

T_void CmdQForcedReceive(T_packetEitherShortOrLong *p_packet) ;

#endif

/****************************************************************************/
/*    END OF FILE:  CMDQUEUE.H                                               */
/****************************************************************************/
