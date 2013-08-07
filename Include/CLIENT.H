/****************************************************************************/
/*    FILE:  CLIENT.H                                                         */
/****************************************************************************/
#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "EFFECT.H"
#include "GENERAL.H"
#include "KEYBOARD.H"
#include "PACKET.H"
#include "STORE.H"

#define SERVER_ID_NONE        0
#define PLACE_NOWHERE         0xFFFFFFFF

typedef enum {
    CLIENT_MODE_GAME,
    CLIENT_MODE_SCRIPT_FORM,
    CLIENT_MODE_HARD_CODED_FORM,
    CLIENT_MODE_UNKNOWN
} E_clientMode;

typedef enum {
    CLIENT_DIAL_RESULT_CONNECTED,
    CLIENT_DIAL_RESULT_BUSY,
    CLIENT_DIAL_RESULT_ABORTED,
    CLIENT_DIAL_RESULT_TIMEOUT,
    CLIENT_DIAL_RESULT_UNKNOWN
} E_clientDialResult;

typedef enum {
    CLIENT_SAVE_CHAR_STATUS_ATTEMPTING,
    CLIENT_SAVE_CHAR_STATUS_OK,
    CLIENT_SAVE_CHAR_STATUS_ERROR,
    CLIENT_SAVE_CHAR_STATUS_UNKNOWN
} E_clientSaveCharStatus;

typedef T_byte8 E_clientConnectionType;
#define CLIENT_CONNECTION_TYPE_SINGLE              0
#define CLIENT_CONNECTION_TYPE_MODEM_CALL          1
#define CLIENT_CONNECTION_TYPE_MODEM_ANSWER        2
#define CLIENT_CONNECTION_TYPE_DIRECT_IPX          3
#define CLIENT_CONNECTION_TYPE_MODEM_TO_SERVER     4
#define CLIENT_CONNECTION_TYPE_UNKNOWN             5

T_void ClientMain(T_void);

T_void ClientLogin(T_void);

E_Boolean ClientIsLogin(T_void);

E_Boolean ClientIsActive(T_void);

T_void ClientUpdate(T_void);

T_void ClientCheckMouse(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        E_Boolean button);

T_void ClientLogoff(T_void);

E_clientDialResult ClientDialIn(T_byte8 *p_init, T_byte8 *p_dial);

T_void ClientTakeDamage(T_word16 amount);

T_void ClientHasGrabbedObject(T_word16 objNum);

T_word16 ClientGetTalkDistance(T_word16 talkMode);

T_word16 ClientGetLoginId(void);

T_void ClientCreateGlobalAreaSound(
        T_sword16 x,
        T_sword16 y,
        T_word16 radius,
        T_word16 soundID);

T_void ClientGotoPlace(T_word32 locationNumber, T_word16 startLocation);

T_void ClientHandleKeyboard(E_keyboardEvent event, T_word16 scankey);

T_void ClientInitMouseAndColor(T_void);
T_void ClientInit(T_void);

T_void ClientFinish(T_void);

T_void ClientObjectShootObject(
        T_word16 objectType,
        T_word16 sourceObject,
        T_word16 angle,
        T_word16 velocity);

T_void ClientThrowObject(
        T_3dObject *p_obj,
        T_sword16 throwspeed,
        T_word16 angle);

T_void ClientCreateProjectile(
        E_effectMissileType type,
        T_word16 duration,
        T_word16 power);

T_void ClientAttack(T_void);

/* Acknowledgements: */
T_void ClientAttackSent(T_word32 extraData, T_packetEitherShortOrLong *p_packet);

T_void ClientSentAction(T_word32 extraData, T_packetEitherShortOrLong *p_packet);

/* Accessor functions: */
T_word16 ClientGetSpeechLoudness(T_void);

E_Boolean ClientIsAttemptingLogout(T_void);

E_Boolean ClientIsActive(T_void);

T_void ClientSetActive(T_void);

T_void ClientSetInactive(T_void);

E_Boolean ClientIsInit(T_void);

E_clientMode ClientGetMode(T_void);

T_void ClientSetMode(E_clientMode newMode);

T_word16 ClientGetLoginId(T_void);

T_void ClientSetLoginId(T_word16 loginId);

T_word32 ClientGetCurrentPlace(T_void);

T_void ClientSetCurrentPlace(T_word32 newPlace);

T_word16 ClientGetCurrentStartLocation(T_void);

T_void ClientSetCurrentStartLocation(T_word16 newStartLocation);

T_word32 ClientGetCurrentServerID(T_void);

T_void ClientSetCurrentServerID(T_word32 newServerID);

T_void ClientSetServerEnterStatus(E_requestEnterStatus status);

E_requestEnterStatus ClientGetServerEnterStatus(T_void);

E_clientSaveCharStatus ClientGetSaveCharStatus(T_void);

T_void ClientSetSaveCharStatus(E_clientSaveCharStatus status);

E_loadCharStatus ClientGetLoadCharacterStatus(T_void);

T_void ClientSetLoadCharacterStatus(E_loadCharStatus status);

E_createCharStatus ClientGetCreateCharacterStatus(T_void);

T_void ClientSetCreateCharacterStatus(E_createCharStatus status);

E_checkPasswordStatus ClientGetCheckPasswordStatus(T_void);

T_void ClientSetCheckPasswordStatus(E_checkPasswordStatus status);

E_changePasswordStatus ClientGetChangePasswordStatus(T_void);

T_void ClientSetChangePasswordStatus(E_changePasswordStatus status);

E_Boolean ClientIsServerBased(T_void);

E_clientConnectionType ClientGetConnectionType(T_void);

T_void ClientSetConnectionType(E_clientConnectionType type);

T_void ClientStartPlayer(T_word16 objectId, T_word16 loginId);

T_void ClientForceGotoPlace(T_word16 placeNumber, T_word16 startLocation);

T_void ClientSetNextPlace(T_word16 nextPlace, T_byte8 nextStart);

typedef T_byte8 E_bloodEffect;
#define BLOOD_EFFECT_NORMAL       0
#define BLOOD_EFFECT_BONE         1
#define BLOOD_EFFECT_NONE         2

T_void ClientMakeObjectGoSplat(
        T_3dObject *p_obj,
        T_word16 amount,
        E_effectDamageType damageType,
        E_effectDamageType defenseType,
        E_bloodEffect bloodType);

T_void ClientSetClientSyncStatus(T_byte8 *p_msg);

/* returns TRUE if the banner is up */
E_Boolean ClientIsInGame(T_void);
/* returns TRUE if the 3dView is up */
E_Boolean ClientIsInView(T_void);
/* returns TRUE if the player is a god */
E_Boolean ClientIsGod(T_void);

T_void ClientResetPauseGame(T_void);
T_void ClientTogglePause(T_void);
E_Boolean ClientIsPaused(T_void);

/* Death related */
E_Boolean ClientIsDead(void);

T_void ClientSetAdventureNumber(T_word16 adventureNum);

T_void ClientDied(T_void);

T_void ClientDoSteal(T_void);

#endif

/****************************************************************************/
/*    END OF FILE:  CLIENT.H                                                  */
/****************************************************************************/
