/****************************************************************************/
/*    FILE:  CMDQUEUE.H                                                      */
/****************************************************************************/

#ifndef _CMDQUEUE_H_
#define _CMDQUEUE_H_

#include "GENERAL.H"
#include "PACKET.H"

typedef enum {
    PACKET_COMMAND_ACK,                     /*  0 */
    PACKET_COMMAND_LOGIN,                   /*  1 */
    PACKET_COMMAND_RETRANSMIT,              /*  2 */
    PACKET_COMMAND_MONSTER_MOVE,            /*  3  not used */
    PACKET_COMMANDCS_PLAYER_ATTACK,         /*  4  not used */
    PACKET_COMMAND_TOWN_UI_MESSAGE,         /*  5 */
    PACKET_COMMAND_PLAYER_ID_SELF,          /*  6 */
    PACKET_COMMAND_REQUEST_PLAYER_ID,       /*  7 */           
    PACKET_COMMAND_GAME_REQUEST_JOIN,       /*  8 */
    PACKET_COMMAND_GAME_RESPOND_JOIN,       /*  9 */
    PACKET_COMMAND_GAME_START,              /* 10 */
    PACKET_COMMAND_FIREBALL_STOP,           /* 11  not used */
    PACKET_COMMAND_MOVE_CREATURE,           /* 12  not used */
    PACKET_COMMANDSC_DAMAGE,                /* 13  not used */
    PACKET_COMMAND_CREATURE_ATTACK,         /* 14  not used */
    PACKET_COMMAND_CREATURE_HURT,           /* 15  not used */
    PACKET_COMMAND_CREATURE_DEAD,           /* 16  not used */
    PACKET_COMMAND_REVERSE_SECTOR,          /* 17  not used */
    PACKET_COMMAND_SYNC,                    /* 18 */
    PACKET_COMMAND_PICK_UP,                 /* 19  not used */
    PACKET_COMMAND_MESSAGE,                 /* 20 */
    PACKET_COMMAND_OPEN_DOOR,               /* 21  not used */
    PACKET_COMMAND_CANNED_SAYING,           /* 22  not used */
    PACKET_COMMANDSC_OBJECT_POSITION,       /* 23 */
    PACKET_COMMANDCS_REQUEST_TAKE,          /* 24  not used */
    PACKET_COMMANDSC_TAKE_REPLY,            /* 25 */
    PACKET_COMMANDCSC_ADD_OBJECT,           /* 26 */
    PACKET_COMMANDSC_DESTROY_OBJECT,        /* 27  not used */
    PACKET_COMMANDSC_SPECIAL_EFFECT,        /* 28  not used */
    PACKET_COMMANDSC_PLACE_START,           /* 29 */
    PACKET_COMMANDCSC_GOTO_PLACE,           /* 30 */
    PACKET_COMMANDCS_GOTO_SUCCEEDED,        /* 31 */
    PACKET_COMMANDCSC_PROJECTILE_CREATE,    /* 32  not used */

    /** File transfer packets. **/
    /**  RT = receiver->transmitter, TR = transmitter->receiver **/
    //PACKET_COMMANDRT_REQUEST_FILE,          /* 33 */
    //PACKET_COMMANDTR_START_TRANSFER,        /* 34 */
    //PACKET_COMMANDTR_DATA_PACKET,           /* 35 */
    //PACKET_COMMANDTR_FINAL_PACKET,          /* 36 */
    //PACKET_COMMANDRT_RESEND_PLEASE,         /* 37 */
    //PACKET_COMMANDRT_TRANSFER_COMPLETE,     /* 38 */
    //PACKET_COMMANDRT_TRANSFER_CANCEL,       /* 39 */
    //PACKET_COMMANDTR_FILE_NOT_HERE,         /* 40 */

    PACKET_COMMAND_UNKNOWN=51
} E_packetCommand ;

typedef T_void *T_packetHandle ;

#define PACKET_COMMAND_MAX PACKET_COMMAND_UNKNOWN
#define BAD_PACKET_HANDLE NULL

typedef T_void (*T_cmdQActionRoutine)(T_packetEitherShortOrLong *p_packet) ;

typedef T_void (*T_cmdQPacketCallback)
                   (T_word32 extraData,
                    T_packetEitherShortOrLong *p_packet) ;

T_void CmdQInitialize(T_void) ;

T_void CmdQFinish(T_void) ;

T_void CmdQSetActivePortNum(T_word16 num) ;

T_word16 CmdQGetActivePortNum(T_void) ;

T_void CmdQSendShortPacket(
            T_packetShort *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

T_void CmdQSendLongPacket(
            T_packetLong *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

T_void CmdQSendPacket(
            T_packetEitherShortOrLong *p_packet,
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
