/****************************************************************************/
/*    FILE:  SERVER.H                                                       */
/****************************************************************************/
#ifndef _SERVER_H_
#define _SERVER_H_

#include "GENERAL.H"
#include "PACKET.H"

#ifndef SERVER_ONLY
typedef T_word16 T_player ;
#define PLAYER_BAD 0xFFFF
#endif

T_void ServerUpdate(T_void) ;

T_void ServerInit(T_void) ;

T_void ServerFinish(T_void) ;

T_sword16 ServerGetNextPlayer(T_player lastPlayer) ;

T_3dObject *ServerGetPlayerObject(T_word16 playerId) ;

T_void ServerReceiveGotoSucceededPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ServerReceiveGotoPlacePacket(T_packetEitherShortOrLong *p_gotoPacket) ;

T_3dObject *ServerReceiveProjectileAddPacket (T_packetEitherShortOrLong *p_packet);

T_3dObject *ServerShootProjectile(
          T_3dObject *p_objSource,
          T_word16 angle,
          T_word16 typeObj,
          T_word16 initSpeed,
          T_3dObject *p_target) ;

T_void ServerGotoPlace (T_word32 placeNumber, T_word16 startLocation);

T_void ServerSetServerID(T_word32 newID) ;

T_word32 ServerGetServerID(T_void) ;

T_void ServerReceiveRequestCharacterListPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ServerReceiveLoadCharacterPacket(
           T_packetEitherShortOrLong *p_packet) ;

#define MESSAGE_RANGE_WHISPER 200
#define MESSAGE_RANGE_TALK    1500
#define MESSAGE_RANGE_SHOUT   5000

T_void *ServerFetchDataBlock(
            T_word16 dataBlockType,
            T_word32 extraData,
            T_word32 *p_size,
            E_Boolean *doMemFree) ;

T_void ServerReceiveRequestDataBlockPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ServerPlayerLeft(T_player player) ;

#define PlayersGetFirstPlayer()   (ServerGetNextPlayer(-1))
#define PlayersGetNextPlayer(p)   (ServerGetNextPlayer(p))
#define PlayersGetPlayerObject(p) (ServerGetPlayerObject(p))
#define PlayersFindPlayerByCommPort(commPort)  ((T_player)(commPort))
#define PlayersGetPlayerPlaceNumber(player)  (G_nextMap)

T_void ServerReceiveSyncPacket(T_packetEitherShortOrLong *p_packet) ;

T_3dObject *ServerCreateObjectGlobal(
           T_word16 objType,
           T_sword16 x,
           T_sword16 y,
           T_sword16 z) ;

T_3dObject * ServerCreateFakeObjectGlobal(
           T_word16 objType,
           T_sword16 x,
           T_sword16 y,
           T_sword16 z) ;

T_void ServerShootBasicProjectile(
           T_word16 objectType,
           T_sword32 x,
           T_sword32 y,
           T_sword32 z,
           T_sword32 targetX,
           T_sword32 targetY,
           T_sword32 targetZ,
           T_word16 initialSpeed) ;

T_void ServerDestroyDestroyObjects(T_void) ;

T_void ServerDamageAtWithType(
           T_sword16 x,
           T_sword16 y,
           T_sword16 z,
           T_word16 radius,
           T_word16 damage,
           T_word16 ownerID,
           T_byte8 type) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SERVER.H                                                */
/****************************************************************************/
