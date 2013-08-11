/****************************************************************************/
/*    FILE:  PACKET.H                                                       */
/****************************************************************************/
#ifndef _PACKET_H_
#define _PACKET_H_

#include "GENERAL.H"
#include "GROUP.H"
#include "VIEWFILE.H"

#define MAX_SIZE_PASSWORD 12

/* ------------------------------------------------------------------------ */

#define SHORT_PACKET_LENGTH 10
#define LONG_PACKET_LENGTH 72

#define PACKET_PREFIX     0xCC

#define MAX_MESSAGE_LEN      40

typedef struct {
    T_byte8    prefix            PACK;
    T_byte8    packetLength      PACK;
    T_word32   id                PACK;
    T_word16   checksum          PACK;
} T_packetHeader ;

typedef struct {
    T_packetHeader header        PACK;
    T_byte8        data[SHORT_PACKET_LENGTH]
                                 PACK;
} T_packetShort ;

typedef struct {
    T_packetHeader header        PACK;
    T_byte8        data[LONG_PACKET_LENGTH]
                                 PACK;
} T_packetLong ;

typedef struct {
    T_packetHeader header        PACK;
    T_byte8        data[LONG_PACKET_LENGTH]       PACK;
} T_packetEitherShortOrLong ;

T_sword16 PacketSendShort(T_packetShort *p_shortPacket) ;

T_sword16 PacketSendLong(T_packetLong *p_longPacket) ;

T_sword16 PacketSendAnyLength(T_packetEitherShortOrLong *p_anyPacket) ;

T_sword16 PacketSend(T_packetEitherShortOrLong *p_packet) ;

T_sword16 PacketGet(T_packetLong *p_packet) ;

T_void PacketSetId (T_packetEitherShortOrLong *p_packet, T_word32 packetID);

T_void PacketReceiveData(T_void *p_data, T_word16 size);

/* !!! Needs to be moved! */
typedef struct {
    T_byte8 command              PACK;
    T_sword16 x                  PACK;
    T_sword16 y                  PACK;
    T_word16 angleAndStance      PACK;
    T_byte8 player               PACK;
    T_sword16 height             PACK;
} T_movePacketData ;

typedef struct {
    T_byte8 command              PACK;
    T_byte8 visage               PACK;
    T_byte8 demeanor             PACK;
    T_word32 accountNum          PACK ;
} T_loginPacket;

typedef struct {
    T_byte8 command              PACK;
    T_word16 player              PACK;
    T_word16 objectId            PACK;
} T_logoffPacket ;

typedef struct {
    T_byte8 command              PACK;
    T_word16 player              PACK;
    T_word16 weapon              PACK;
} T_changeWeaponPacket ;

typedef struct {
    T_byte8 command              PACK ;
    T_gameGroupID groupID        PACK ;
    T_byte8 syncData[1]          PACK ;
} T_syncPacket ;

typedef struct {
    T_byte8 command              PACK;
    T_word16 player              PACK;
    T_gameGroupID groupID        PACK;
    T_byte8 message[MAX_MESSAGE_LEN+1]
                                 PACK;
} T_messagePacket ;

typedef struct {
    T_byte8   command            PACK ;
    T_word16  id                 PACK ;
    T_sword16 x                  PACK ;
    T_sword16 y                  PACK ;
    T_sword16 z                  PACK ;
    T_byte8   angleAndStance     PACK ;
} T_objectPositionPacket ;

/* Define masks for the object position packet */
#define OBJECT_POSITION_PACKET_ANGLE  0xF8
#define OBJECT_POSITION_PACKET_STANCE 0x07
#define OBJECT_POS_ANGLE_AND_STANCE(ang, sta)  ((T_byte8)  \
            ( (((ang) & 0xF800) >> 8) | ((sta) & 7) ) )


typedef struct {
    T_byte8 command              PACK;
    T_word16 objectID            PACK;
} T_objectDestroyPacket;


/** Sent to perform a "special effect" **/
/** Current special effects are: **/

#define SPECIAL_EFFECT_AREA_SOUND   0  /** Area sound: id=sound # **/

typedef struct {
    T_byte8 command              PACK;
    T_byte8 effect               PACK;
    T_sword16 x                  PACK;
    T_sword16 y                  PACK;
    T_word16 radius              PACK;
    T_word16 id                  PACK;
} T_specialEffectPacket;


/** When a client drops an object, it needs to send the following **/
/** packet to the server, requesting the creation of an object. **/
/** Also, when the server creates an object, she needs to send this **/
/** packet to all her clients to tell them what she creates. **/

typedef struct {
    T_byte8 command              PACK;   /*   1 */ 
    T_word16 objectID            PACK;   /*   2 */  /* Ignored for C->S */
    T_word16 objectType          PACK;   /*   2 */
    T_word32 X                   PACK;   /*   4 */
    T_word32 Y                   PACK;   /*   4 */
    T_word32 Z                   PACK;   /*   4 */
    T_word32 XV                  PACK;   /*   4 */
    T_word32 YV                  PACK;   /*   4 */
    T_word32 ZV                  PACK;   /*   4 */
    T_word16 angle               PACK;   /* + 2 */

    T_word16 bodyParts[MAX_BODY_PARTS]           PACK;   /* body parts.  + 20 bytes */
} T_objectAddPacket;                     /*= 51 bytes = LONG packet. */

typedef struct {
    T_byte8 command              PACK ;
    T_word16 loginId             PACK ;
    T_word16 objectId            PACK ;
    T_sword16 xPos               PACK ;
    T_sword16 yPos               PACK ;
    T_byte8 angle                PACK ;
} T_placeStartPacket ;

typedef struct {
    T_byte8 command              PACK ;
    T_word32 placeNumber         PACK ;
    T_word16 startLocation       PACK ;
} T_gotoPlacePacket ;

/** This packet now specifies exactly what goto_place it's responding to **/
typedef struct {
    T_byte8 command              PACK ;
    T_word32 placeNumber         PACK;
    T_word16 startLocation       PACK ;
} T_gotoSucceededPacket ;

typedef struct {
    T_byte8 command               PACK ;
    T_byte8 outsideLightLevel     PACK ;
} T_dayNightPacket ;

/* ------------------------------------------------------------------------ */

typedef T_byte8 E_checkPasswordStatus ;
#define CHECK_PASSWORD_STATUS_OK             0
#define CHECK_PASSWORD_STATUS_WRONG          1
#define CHECK_PASSWORD_STATUS_UNKNOWN        2

/* ------------------------------------------------------------------------ */

typedef T_byte8 E_changePasswordStatus ;
#define CHANGE_PASSWORD_STATUS_OK             0
#define CHANGE_PASSWORD_STATUS_WRONG          1
#define CHANGE_PASSWORD_STATUS_ABORT          2
#define CHANGE_PASSWORD_STATUS_UNKNOWN        3

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_word16 dataBlockType        PACK ;
    T_word32 extraData            PACK ;
} T_requestDataBlockPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_word16 objectID             PACK ;
    T_word16 damage               PACK ;
    T_byte8 damageType            PACK ;
    T_word16 ownerObjID           PACK ;
} T_damageObjectPacket ;

/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_word16 item ;
    T_word16 numItems ;
    T_word16 storeID ;            /* 0 to MAX_STORES (63) */
} T_requestStoreAddItemPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_word16 item ;
    T_word16 numItems ;
    T_word16 storeID ;
} T_requestStoreRemoveItemPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_byte8 removeResult ;
    T_word16 storeID ;
} T_storeRemoveResultPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_byte8 addResult ;
    T_word16 storeID ;
} T_storeAddResultPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_byte8 fromPlayer ;
    T_byte8 toPlayer ;
    T_byte8 transmitStart ;
    T_gameGroupID groupID ;
} T_retransmitPacket ;

/* ------------------------------------------------------------------------ */

/* Block for transmitting a character block (particularly to be stored) */
typedef struct {
    T_word16 slot ;
    T_byte8 password[MAX_SIZE_PASSWORD] ;
    T_word32 size ;
    T_byte8 charData[] ;
} T_characterBlock ;

/* ------------------------------------------------------------------------ */

/* Packet for town ui messages. */
typedef struct {
    T_byte8 command ;
    T_byte8 name[30] ;
    T_byte8 msg[40] ;
} T_townUIMessagePacket ;

/* ------------------------------------------------------------------------ */

typedef T_byte8 T_playerIDLocation ;
#define PLAYER_ID_LOCATION_NOWHERE           0
#define PLAYER_ID_LOCATION_TOWN              1
#define PLAYER_ID_LOCATION_GUILD             2
#define PLAYER_ID_LOCATION_GAME              3
#define PLAYER_ID_LOCATION_UNKNOWN           4

typedef T_byte8 T_playerIDState ;
#define PLAYER_ID_STATE_NONE                 0
#define PLAYER_ID_STATE_CREATING_GAME        1
#define PLAYER_ID_STATE_JOINING_GAME         2
#define PLAYER_ID_STATE_UNKNOWN              3

typedef struct {
    T_byte8 name[30] ;
    T_directTalkUniqueAddress uniqueAddress ;
    T_playerIDLocation location ;
    T_playerIDState state ;
    T_gameGroupID groupID ;
    T_word16 adventure ;
} T_playerIDSelf ;

typedef struct {
    T_byte8 command ;
    T_playerIDSelf id ;
} T_playerIDSelfPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_directTalkUniqueAddress uniqueAddress ;
    T_gameGroupID groupID ;
    T_word16 adventure ;
} T_gameRequestJoinPacket ;

/* ------------------------------------------------------------------------ */

typedef T_byte8 E_respondJoin ;
#define GAME_RESPOND_JOIN_OK         0
#define GAME_RESPOND_JOIN_FULL       1
#define GAME_RESPOND_JOIN_CANCELED   2
#define GAME_RESPOND_JOIN_UNKNOWN    3

typedef struct {
    T_byte8 command ;
    T_directTalkUniqueAddress uniqueAddress ;
    T_gameGroupID groupID ;
    T_word16 adventure ;
    E_respondJoin response ;
} T_gameRespondJoinPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command ;
    T_gameGroupID groupID ;
    T_word16 adventure ;
    T_byte8 numPlayers ;
    T_directTalkUniqueAddress players[4] ;
    T_word32 timeOfDay ;
    T_word16 firstLevel ;
} T_gameStartPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
	T_byte8 command;
	T_byte8 commandBeingAcked;
	T_word32 packetIDBeingAcked; 
} T_ackPacket;

#endif


/****************************************************************************/
/*    END OF FILE:  PACKET.H                                                */
/****************************************************************************/
