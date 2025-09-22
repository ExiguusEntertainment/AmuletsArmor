/****************************************************************************/
/*    FILE:  CLIENT.H                                                         */
/****************************************************************************/

#ifndef _CLIENT_H_
#define _CLIENT_H_

T_void ClientMain(T_void) ;

T_void ClientLogin(T_void) ;

E_Boolean ClientIsLogin(T_void) ;

T_void ClientUpdate(T_void) ;

T_void ClientCheckMouse (E_mouseEvent event,
                         T_word16 x,
                         T_word16 y,
                         E_Boolean button);

T_void ClientReceiveLoginPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerMovePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveFireballMovePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveFireballStopPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerJoinPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerAttackPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerActionPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerLogoffPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceivePlayerChangeWeaponPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientLogoff(T_void) ;

T_void ClientDialIn(T_void) ;

T_void ClientReceiveMonsterMovePacket(T_packetEitherShortOrLong *p_packet) ;
T_void ClientReceivePlayerActivatePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveCreatureMovePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveDamageAtPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveCreatureAttackPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveCreatureHurtPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveCreatureDeadPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveReverseSectorPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientSendReverseSectorPacket(T_word16 sector, T_word16 newActivity) ;

T_void ClientTakeDamage(T_word16 amount) ;

T_void ClientReceivePickUpPacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientRequestPickUpObject(T_word16 objNum) ;

T_void ClientIsOver(T_word16 objNum, T_word16 objectType) ;

T_void ClientHasGrabbedObject(T_word16 objNum) ;

T_void ClientRemovePendingObject(T_word16 objNum) ;

T_void ClientAddPendingObject(T_word16 objNum) ;

E_Boolean ClientIsObjectPending(T_word16 objNum) ;

T_void ClientReceiveMessagePacket(T_packetEitherShortOrLong *p_packet) ;

T_void ClientReceiveOpenDoorPacket(T_packetEitherShortOrLong *p_packet) ;

T_word16 ClientGetLoginId(T_void) ;

T_void ClientSendCannedSayingPacket(T_word16 saying) ;

T_void ClientReceiveCannedSayingPacket(T_packetEitherShortOrLong *p_packet) ;

T_word16 ClientGetTalkDistance(T_word16 talkMode) ;

#endif

/****************************************************************************/
/*    END OF FILE:  CLIENT.H                                                  */
/****************************************************************************/
