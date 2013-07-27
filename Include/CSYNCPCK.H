/****************************************************************************/
/*    FILE:  CSYNCPCK.H                                                      */
/****************************************************************************/
#ifndef _CSYNCPCK_H_
#define _CSYNCPCK_H_

#include "DITALK.H"
#include "EFFECT.H"
#include "GENERAL.H"
#include "GROUP.H"
#include "PACKET.H"
#include "SYNCPACK.H"
#include "VIEWFILE.H"

T_void ClientSyncPacketProcess(T_syncronizePacket *p_sync) ;

T_void ClientSyncSendActionChangeSelf(
           T_bodyPartLocation location,
           T_word16 newPart) ;

T_void ClientSyncSendActionMeleeAttack(
           T_word16 damageAmount,
           E_effectDamageType damageType,
           T_word16 target) ;

T_void ClientSyncSendActionMissileAttack(
           T_word16 missileType,
           T_word16 target) ;

T_void ClientSyncSendActionActivateForward(T_void) ;

T_void ClientSyncSendActionLeaveLevel(
       T_word16 newLevel) ;

T_void ClientSyncSendActionPickUpItem(
           T_word16 itemID,
           E_Boolean autoStore) ;

T_void ClientSyncSendActionThrowItem(
           T_word16 itemType,
           T_word16 angle,
           T_word16 throwSpeed,
           T_word16 objectAccData) ;

T_void ClientSyncSendActionSteal(
           T_word16 objectID,
           T_word16 stealerID,
           E_Boolean notifyOwner) ;

T_void ClientSyncSendActionStolen(
           T_word16 objectType,
           T_word16 stealerID) ;

T_void ClientSyncSendActionPickLock(
           T_word16 doorSector,
           T_word16 pickerID) ;

T_void ClientSyncSendActionAreaSound(
           T_word16 sound,
           T_word16 radius,
           E_Boolean isVoicing) ;

T_void ClientSyncSendActionDropAt(
           T_word16 objectType,
           T_sword16 x,
           T_sword16 y,
           T_word16 accData) ;

T_void ClientSyncSendIdSelf(T_byte8 *p_name) ;

T_void ClientSyncSendActionPauseGameToggle(T_void) ;

T_void ClientSyncSendActionAbortLevel(T_void) ;

T_void ClientSyncSendGotoPlace(T_word16 newPlace) ;

T_void ClientSyncUpdate(T_void) ;

T_void ClientSyncInit(T_void) ;

T_void ClientSyncFinish(T_void) ;

T_void ClientSyncEnsureSend(T_void) ;

T_void ClientSyncReceiveRetransmitPacket(
           T_packetEitherShortOrLong *p_packet) ;

T_void ClientSyncSetNumberPlayers(T_byte8 numPlayers) ;

T_byte8 ClientSyncGetNumberPlayers(T_void) ;

T_objMoveStruct *ClientSyncGetLastGoodMove(T_void) ;

T_void ClientSyncReset(T_void) ;

T_gameGroupID ClientSyncGetGameGroupID(T_void) ;

T_void ClientSyncSetGameGroupID(T_gameGroupID groupID) ;

T_void ClientSyncInitPlayersHere(T_void) ;

T_void ClientSyncSendIdSelf(T_byte8 *p_name) ;

#endif

/****************************************************************************/
/*    END OF FILE:  CSYNCPCK.H                                               */
/****************************************************************************/
