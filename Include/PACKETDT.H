/****************************************************************************/
/*    FILE:  PACKET.H                                                       */
/****************************************************************************/
#if 0
#ifndef _PACKETDT_H_
#define _PACKETDT_H_

#define MAX_SIZE_PASSWORD 12

/* ------------------------------------------------------------------------ */
/* The following list is the types of blocks that are sent back and forth */
/* between client and server (mainly to the client). */

#define CLIENT_MEMORY_BLOCK_UPDATE_BLOCK       0
#define CLIENT_MEMORY_BLOCK_CHARACTER_LISTING  1
#define MEMORY_BLOCK_CHARACTER_DATA            2
#define MEMORY_BLOCK_ACCOUNT_INFO              3
#define MEMORY_BLOCK_TYPE_UNKNOWN              4

/* ------------------------------------------------------------------------ */

/* Data blocks to send: */
typedef struct {
    T_byte8 ownersName[60] ;
    T_byte8 street[80] ;
    T_byte8 city[40] ;
    T_byte8 state[20] ;
    T_byte8 zipcode[14] ;
    T_byte8 phoneHome[20] ;
    T_byte8 phoneWork[20] ;
    T_byte8 billingMethod[20] ;
    T_byte8 billingData[60] ;
    T_word32 accountID ;
} T_dataBlockAccountInfo ;

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
    T_byte8 syncData[1]          PACK ;
} T_syncPacket ;

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


/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
} T_requestEnterPacket ;

typedef T_byte8 E_requestEnterStatus ;
#define REQUEST_ENTER_STATUS_WAITING       0
#define REQUEST_ENTER_STATUS_OK            1
#define REQUEST_ENTER_STATUS_CLOSED        2
#define REQUEST_ENTER_STATUS_COMPLETED     3
#define REQUEST_ENTER_STATUS_FULL          4
#define REQUEST_ENTER_STATUS_UNKNOWN       5

typedef struct {
    T_byte8 command               PACK ;
    E_requestEnterStatus status   PACK ;
} T_requestEnterStatusPacket ;

typedef struct {
    T_byte8 command               PACK ;
} T_requestCharacterListPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_word32 checksum             PACK ;
    T_byte8 slot                  PACK ;
} T_loadCharacterPacket ;

typedef T_byte8 E_loadCharStatus ;
#define LOAD_CHARACTER_STATUS_INCORRECT      0
#define LOAD_CHARACTER_STATUS_CORRECT        1
#define LOAD_CHARACTER_STATUS_UNKNOWN        2

typedef struct {
    T_byte8 command               PACK ;
    E_loadCharStatus status       PACK ;
} T_loadCharacterStatusPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_byte8 slot                  PACK ;
    T_word32 checksum             PACK ;
    T_byte8 password[MAX_SIZE_PASSWORD] PACK ;
} T_createCharacterPacket ;

typedef T_byte8 E_createCharStatus ;
#define CREATE_CHARACTER_STATUS_OK             0
#define CREATE_CHARACTER_STATUS_ERROR          1
#define CREATE_CHARACTER_STATUS_UNKNOWN        2

typedef struct {
    T_byte8 command               PACK ;
    E_createCharStatus status     PACK ;
} T_createCharStatusPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_byte8 slot                  PACK ;
} T_deleteCharacterPacket ;

typedef T_byte8 E_deleteCharStatus ;
#define DELETE_CHARACTER_STATUS_OK             0
#define DELETE_CHARACTER_STATUS_ERROR          1
#define DELETE_CHARACTER_STATUS_UNKNOWN        2

typedef struct {
    T_byte8 command               PACK ;
    E_deleteCharStatus status     PACK ;
} T_deleteCharStatusPacket ;

/* ------------------------------------------------------------------------ */
/* The T_checkPasswordPacket and T_checkPasswordStatusPacket are used */
/* together to check a character's password and then to return a status */
/* telling if the password matches the character. */

typedef struct {
    T_byte8 command               PACK ;
    T_byte8 slot                  PACK ;
    T_byte8 password[MAX_SIZE_PASSWORD]  PACK ;
} T_checkPasswordPacket ;

typedef T_byte8 E_checkPasswordStatus ;
#define CHECK_PASSWORD_STATUS_OK             0
#define CHECK_PASSWORD_STATUS_WRONG          1
#define CHECK_PASSWORD_STATUS_UNKNOWN        2

typedef struct {
    T_byte8 command               PACK ;
    E_checkPasswordStatus status  PACK ;
} T_checkPasswordStatusPacket ;

/* ------------------------------------------------------------------------ */
/* The T_changePasswordPacket and T_changePasswordStatusPacket are used */
/* together to change a character's password and then to return a status */
/* telling if the password was successfully changed. */

typedef struct {
    T_byte8 command               PACK ;
    T_byte8 slot                  PACK ;
    T_byte8 password[MAX_SIZE_PASSWORD]  PACK ;
    T_byte8 newPassword[MAX_SIZE_PASSWORD]  PACK ;
} T_changePasswordPacket ;

typedef T_byte8 E_changePasswordStatus ;
#define CHANGE_PASSWORD_STATUS_OK             0
#define CHANGE_PASSWORD_STATUS_WRONG          1
#define CHANGE_PASSWORD_STATUS_ABORT          2
#define CHANGE_PASSWORD_STATUS_UNKNOWN        3

typedef struct {
    T_byte8 command               PACK ;
    E_changePasswordStatus status  PACK ;
} T_changePasswordStatusPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
    T_word16 dataBlockType        PACK ;
    T_word32 extraData            PACK ;
} T_requestDataBlockPacket ;

/* ------------------------------------------------------------------------ */

typedef struct {
    T_byte8 command               PACK ;
} T_requestPiecewiseListPacket ;

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
#endif

#endif
/****************************************************************************/
/*    END OF FILE:  PACKET.H                                                */
/****************************************************************************/
