/****************************************************************************/
/*    FILE:  SYNCPACK.H                                                      */
/****************************************************************************/

#ifndef _SYNCPACK_H_
#define _SYNCPACK_H_

typedef T_byte8 T_syncPacketFieldsAvail ;
#define SYNC_PACKET_FIELD_AVAIL_X    0x01      /* ---- ---1 */
#define SYNC_PACKET_FIELD_AVAIL_Y    0x02      /* ---- --1- */
#define SYNC_PACKET_FIELD_AVAIL_Z    0x04      /* ---- -1-- */
#define SYNC_PACKET_FIELD_ANGLE      0x08      /* ---- 1--- */
#define SYNC_PACKET_FIELD_STANCE     0x10      /* ---1 ---- */
#define SYNC_PACKET_FIELD_ACTION     0x20      /* --1- ---- */

typedef T_byte8 T_syncPacketStanceAndVis ;
#define SYNC_PACKET_STANCE_AND_VIS_INVISIBLE    0x80
#define SYNC_PACKET_STANCE_AND_VIS_TRANSLUCENT  0x40
#define SYNC_PACKET_STANCE_AND_VIS_STEALTHY     0x20
#define SYNC_PACKET_STANCE_AND_VIS_TELEPORT     0x10
#define SYNC_PACKET_STANCE_AND_VIS_STANCE_MASK  0x07

/* Different types of actions:

   Type:                     Data
   -----                     ------
   NO_NONE                   (no data)
   CHANGE_SELF               [0]Position and [1]body type
   MELEE_ATTACK              [0]Damage, [1]damage type, [2] target id
   MISSILE_ATTACK            [0]Type of missile, [1] target id
   ACTIVATE_FORWARD          (already check if door is valid)
   LEAVE_LEVEL               [0]Level (0=logoff)
   PICKUP_ITEM               [0]item id
   THROW_ITEM                [0]item type, [1]angle, [2]throwspeed
   STEAL                     [0]from who (player's id)
   STOLEN                    [0]item type
   AREA_SOUND                [0]area sound # [1] radius  [2] isVoicing
   GOTO_PLACE                [0]place number
   SYNC_NEW_PLAYER_WAIT      (no data)
   SYNC_NEW_PLAYER_SEND      (no data)
   SYNC_COMPLETE             (no data)
   DROP_AT                   [0]obj type [1] x pos [2] y pos
   PAUSE_GAME_TOGGLE         (no data)
   ABORT_LEVEL               (no data)
   PICK_LOCK                 [0]door sector  [1] picker ID
   ID_SELF                   [0..7] null terminated string that contains
                             part of name that is appended together with
                             previous ID_SELF packets.
*/

typedef T_byte8 T_playerAction ;
#define PLAYER_ACTION_NONE                  0
#define PLAYER_ACTION_CHANGE_SELF           1
#define PLAYER_ACTION_MELEE_ATTACK          2
#define PLAYER_ACTION_MISSILE_ATTACK        3
#define PLAYER_ACTION_ACTIVATE_FORWARD      4
#define PLAYER_ACTION_LEAVE_LEVEL           5
#define PLAYER_ACTION_PICKUP_ITEM           6
#define PLAYER_ACTION_THROW_ITEM            7
#define PLAYER_ACTION_STEAL                 8
#define PLAYER_ACTION_STOLEN                9
#define PLAYER_ACTION_AREA_SOUND            10
#define PLAYER_ACTION_SYNC_NEW_PLAYER_WAIT  11
#define PLAYER_ACTION_SYNC_NEW_PLAYER_SEND  12
#define PLAYER_ACTION_SYNC_COMPLETE         13
#define PLAYER_ACTION_GOTO_PLACE            14
#define PLAYER_ACTION_DROP_AT               15
#define PLAYER_ACTION_PAUSE_GAME_TOGGLE     16
#define PLAYER_ACTION_ABORT_LEVEL           17
#define PLAYER_ACTION_PICK_LOCK             18
#define PLAYER_ACTION_ID_SELF               19
#define PLAYER_ACTION_UNKNOWN               20

/* Fully available SYNC packet.  Only data elements that change are sent. */
typedef struct {
    /* 0-255, detects when out of sync.  Always starts at 0. */
    T_byte8 syncNumber ;

    /* Delta time since last update.  Sync clock always starts at 0. */
    T_byte8 deltaTime ;

    /* Tells which of the following data items are available. */
    T_syncPacketFieldsAvail fieldsAvailable ;

    /* Object Id of this player. */
    T_word16 playerObjectId ;

    /* Position and angle of this player. */
    T_sword16 x ;
    T_sword16 y ;
    T_sword16 z ;
    T_word16 angle ;

    /* Visibility of the player. */
    T_syncPacketStanceAndVis stanceAndVisibility ;

    /* action and data for action. */
    T_playerAction actionType ;
    T_word16 actionData[4] ;

#ifndef COMPILE_OPTION_DONT_CHECK_SYNC_OBJECT_IDS
    T_word16 nextObjectId ;      /* Server Id's must match */
    T_word16 nextObjectIdWhen ;
#endif
} T_syncronizePacket ;

#endif

/****************************************************************************/
/*    END OF FILE:  SYNCPACK.H                                               */
/****************************************************************************/
