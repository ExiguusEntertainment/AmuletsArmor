/*-------------------------------------------------------------------------*
 * File:  CLIENT.C
 *-------------------------------------------------------------------------*/
/**
 * An interface between the player ("client" here) and originally the
 * server world is the Client system.  The game system has the player
 * make requests to the world through the Client interface.
 *
 * Much of this code is the player "top level" control and has a mix of
 * many pieces of code.
 *
 * The game keyboard controls are handled in ClientHandleKeyboard().
 *
 * @addtogroup CLIENT
 * @brief Client Player Actions
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include <ctype.h>
#include <malloc.h>
#include "3D_COLLI.H"
#include "ACTIVITY.H"
#include "AREASND.H"
#include "BANNER.H"
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONFIG.H"
#include "COMWIN.H"
#include "CONTROL.H"
#include "CMDQUEUE.H"
#include "CRELOGIC.H"
#include "CSYNCPCK.H"
#include "DOOR.H"
#include "ESCMENU.H"
#include "EFX.H"
#include "GENERAL.H"
#include "HARDFORM.H"
#include "MAP.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OVERHEAD.H"
#include "OVERLAY.H"
#include "PACKETDT.H"
#include "PEOPHERE.H"
#include "PICS.H"
#include "PLAYER.H"
#include "RANDOM.H"
#include "SCRFORM.H"
#include "SCRIPT.H"
#include "SMCPLAY.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SPELLS.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "TOWNUI.H"
#include "VIEW.H"

/*---------------------------------------------------------------------------
 * Constants:
 *--------------------------------------------------------------------------*/
#define PLAYER_ATTACK_HEIGHT      30
#define PLAYER_ATTACK_DISTANCE    40
#define PLAYER_ATTACK_RADIUS      10

#define CLIENT_NUM_TALK_MODES 3
#define TIME_BETWEEN_STEALS      140   /* 2 seconds */
#define TIME_BETWEEN_PICK_LOCKS  140   /* 2 seconds */

/*---------------------------------------------------------------------------
 * Types:
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Globals:
 *--------------------------------------------------------------------------*/
T_word32 G_syncCount = 0 ;

/* General flag noting if we are initilized. */
static E_Boolean G_clientInit = FALSE ;

/* Global mode of the client. */
static E_clientMode G_clientMode = CLIENT_MODE_UNKNOWN ;

/* Number of the adventure we are on. */
static T_word16 G_adventureNumber = 0 ;

/* Data telling where we are in the world. */
static T_word32 G_currentPlace = PLACE_NOWHERE ;
static T_word16 G_currentStartLocation = 0 ;
static T_word16 G_lastSector = 0xFFFF ;
static T_word32 G_currentServerID = SERVER_ID_NONE ;
static E_requestEnterStatus G_enterServerStatus =
                                REQUEST_ENTER_STATUS_WAITING ;

/* Various client stati */
static E_clientSaveCharStatus G_clientSaveCharStatus =
                                  CLIENT_SAVE_CHAR_STATUS_UNKNOWN ;
static E_checkPasswordStatus G_checkPasswordStatus =
                             CHECK_PASSWORD_STATUS_UNKNOWN ;
static E_changePasswordStatus G_changePasswordStatus =
                             CHANGE_PASSWORD_STATUS_UNKNOWN ;

static E_clientConnectionType G_clientConnectionType =
                                  CLIENT_CONNECTION_TYPE_SINGLE ;


/* What id does the server know us by? */
T_word16 G_loginId = 0xFFFF ;

/* Flag noting what direction we are facing. */
static E_Boolean G_logoutAttempted = FALSE ;
static E_Boolean G_clientIsActive = FALSE ;
static E_Boolean G_clientIsLogin = FALSE ;

/* Message being typed in for transmission/talking. */
static T_byte8 G_message[MAX_MESSAGE_LEN+2] ;
static T_word16 G_msgPos = 0 ;
static E_Boolean G_msgOn = FALSE ;
static E_Boolean G_deadState = FALSE ;

static T_3dObject *G_lastDrawTargetItem;

/* Flag telling if attack is completed. */
static E_Boolean G_attackComplete = TRUE ;

static E_Boolean G_pressedCastButton = FALSE;

/* Note if the fps is on or off */
static E_Boolean G_fpsOn = FALSE ;

static E_Boolean G_pauseGame = FALSE ;

#define ClientSetDeadState(state)  (G_deadState = (state))

static T_void ClientRevive(T_void) ;

/* Internal prototypes: */
T_void ClientHandleOverlay(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom) ;

T_void ClientUpdateHealth(T_void) ;

T_word16 ClientGetDelta(T_void) ;

T_void ClientShootFireball(T_void) ;

T_void IClientLoginAck(
                  T_word32 extraData,
                  T_packetEitherShortOrLong *p_packet) ;
T_void ClientGotoForm(T_word32 formNumber) ;

T_void ClientShootFireballAck (T_word32 extraData,
                               T_packetEitherShortOrLong *p_packet);


static T_void ClientOverlayDone (T_word16 animnum, T_byte8 flag);


static E_Boolean IClientGainExperienceIfHit(
                      T_3dObject *p_obj,
                      T_word32 damage) ;

static T_void IClientAttemptOpeningForwardWall(T_void) ;

#ifndef COMPILE_OPTION_FRAME_SPEED_OFF
static T_void IClientUpdateAndDrawFramesPerSecond(
                  T_word16 left,
                  T_word16 bottom) ;
#else
#define IClientUpdateAndDrawFramesPerSecond(l,b)
#endif

/* Next locations to go to. */
static T_word16 G_nextPlace = 0xFFFF ;
static T_byte8 G_nextStart = 0xFF ;

#ifndef NDEBUG
static T_void IClientDrawStatus(T_word16 left, T_word16 bottom) ;
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ClientInit
 *-------------------------------------------------------------------------*/
/**
 *  ClientInit starts up and cleans up any initial items needed by
 *  the client.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientInitMouseAndColor (T_void)
{
    T_word16 numPlayers ;
    T_word16 playerId ;
    FILE *fp ;

    DebugRoutine ("ClientInitMouseAndColor");

//    ControlInit(); /* Init control routines */
    /* initialize inventory */
//    InventoryInit();

    fp = fopen("player.cfg", "r") ;
    if (fp == NULL)  {
        printf("Cannot file PLAYER.CFG file!\n") ;
        exit(1) ;
    }
    fscanf(fp, "%d%d", &numPlayers, &playerId) ;
    fclose(fp) ;

    if (numPlayers == 1)  {
        G_clientConnectionType = CLIENT_CONNECTION_TYPE_SINGLE ;
    } else {
        G_clientConnectionType = CLIENT_CONNECTION_TYPE_SINGLE ;
//        G_clientConnectionType = CLIENT_CONNECTION_TYPE_DIRECT_IPX ;
    }
    ClientSyncSetNumberPlayers((T_byte8)numPlayers) ;
    ClientSetLoginId(playerId) ;


    ColorInit(); /* Init color mapping stuff */

    DebugEnd();
}

void OutputItems(void)
{
	FILE *fp;
	int count = 0;
	int i;
	T_byte8 name[100];
	T_byte8 string[1000];
	T_byte8 *p;
	T_resource res;
	T_word32 len;

	fp = fopen("items.txt", "w");
	fprintf(fp, "{| border=\"1\" cellspacing=\"0\" cellpadding=\"4\"\n");
	fprintf(fp, "! #\n! Identified name\n! Unidentified name\n");
	for (i=0; i<65536; i++) {
        sprintf((char *)name, "OBJDESC2/DES%05d.TXT", i);
		if (PictureExist(name)) {
			fprintf(fp, "|-\n| %d\n", i);
			p = PictureLockData(name, &res);
			len = ResourceGetSize(res);
			memcpy(string, p, len);
			string[len] = '\0';
			while (len) {
				len--;
				if ((string[len] == '\r') || (string[len] == '\n'))
					string[len] = '\0';
				else
					break;
			}
			fprintf(fp, "| %s\n", string);
			PictureUnlock(res);

			sprintf((char *)name, "OBJDESC/DES%05d.TXT", i);
			if (PictureExist(name)) {
				p = PictureLockData(name, &res);
				len = ResourceGetSize(res);
				memcpy(string, p, len);
				string[len] = '\0';
				while (len) {
					len--;
					if ((string[len] == '\r') || (string[len] == '\n'))
						string[len] = '\0';
					else
						break;
				}
				fprintf(fp, "| %s\n", string);
				PictureUnlock(res);
			} else {
				fprintf(fp, "| ?\n", p);
			}
		}
	}
	fprintf(fp, "|}\n");
	fclose(fp);
}

T_void ClientInit(T_void)
{
   T_cmdQActionRoutine callbacks[PACKET_COMMAND_MAX] =
   {
      NULL,                                       /* ACK */
      NULL,                                       /* LOGIN */
      ClientSyncReceiveRetransmitPacket,          /* RETRANSMIT */
      NULL,                                       /* MONSTER_MOVE */
      NULL,                                       /* PLAYER_ATTACK */
      ClientReceiveTownUIMessagePacket,           /* 5 TOWN_UI_MESSAGE */
      ClientReceivePlayerIDSelf,                  /* 6 PLAYER_ID_SELF */
      ClientReceiveRequestPlayerIDPacket,         /* 7 REQUEST_PLAYER_ID */
      ClientReceiveGameRequestJoinPacket,         /* 8 GAME_REQUEST_JOIN */
      ClientReceiveGameRespondJoinPacket,         /* 9 GAME_RESPOND_JOIN */
      ClientReceiveGameStartPacket,               /* 10 GAME_START */
      NULL,                                       /* FIREBALL_STOP */
      NULL,                                       /* MOVE_CREATURE */
      NULL,                                       /* SC_DAMAGE */
      NULL,                                       /* CREATURE_ATTACK */
      NULL,                                       /* CREATURE_HURT */
      NULL,                                       /* CREATURE_DEAD */
      NULL,                                       /* REVERSE_SECTOR */
      ClientReceiveSyncPacket,                    /* SYNC */
      NULL,                                       /* PICK_UP */
      ClientReceiveMessagePacket,                 /* MESSAGE */
      NULL,                                       /* OPEN_DOOR */
      NULL,                                       /* CANNED_SAYING */
      NULL,                                       /* SC_OBJECT_POSITION */
      NULL,                                       /* CS_REQUEST_TAKE */
      NULL,                                       /* SC_TAKE_REPLY */
      NULL,                                       /* CSC_ADD_OBJECT */
      NULL,                                       /* SC_DESTROY_OBJECT */
      NULL,                                       /* SC_SPECIAL_EFFECT */
      ClientReceivePlaceStartPacket,              /* SC_PLACE_START */
      ClientReceiveGotoPlacePacket,               /* CSC_GOTO_PLACE */
      NULL,                                       /* CS_GOTO_SUCCEEDED */
      NULL,                                       /* CSC_PROJECTILE_CREATE */

      NULL,                                       /* RT_REQUEST_FILE */
      NULL,                                       /* TR_START_TRANSFER */
      NULL,                                       /* TR_DATA_PACKET */
      NULL,                                       /* TR_FINAL_PACKET */
      NULL,                                       /* RT_RESEND_PLEASE */
      NULL,                                       /* TR_TRANSFER_COMPLETE */
      NULL,                                       /* RT_TRANSFER_CANCEL */
      NULL,                                       /* TR_FILE_NOT_HERE */

      NULL,                                       /* CSC_REQUEST_MEMORY_TRANSFER */
      NULL,                                       /* CSC_MEMORY_TRANSFER_READY */
      NULL,                                       /* CSC_MEMORY_TRANSFER_DATA */

      NULL,                                       /* CSC_CHANGE_BODY_PART */
      NULL,                                       /* CSC_PING */
      NULL,                                       /* SC_WALL_STATE_CHANGE */
      NULL,                                       /* SC_SIDE_STATE_CHANGE */
      NULL,                                       /* SC_SECTOR_STATE_CHANGE */
      NULL,                                       /* SC_GROUP_STATE_CHANGE */
      NULL,                                       /* SC_EXPERIENCE */
      NULL,                                       /* CS_REQUEST_SERVER_ID */
      ClientReceiveServerIDPacket,                /* SC_SERVER_ID */
      NULL,                                       /* CS_REQUEST_ENTER */
      ClientReceiveRequestEnterStatusPacket,      /* SC_REQUEST_ENTER_STATUS */
   };

    DebugRoutine("ClientInit") ;
    DebugCheck(G_clientInit == FALSE) ;

    KeyboardDebounce() ;

    /** Register my callback routines with the command queue. **/
    CmdQRegisterClientCallbacks (callbacks);

    ClientUpdateHealth() ;
    ClientGetDelta() ;
    KeyboardBufferOff() ;
    ViewSetOverlayHandler(ClientHandleOverlay) ;
    OverlaySetAnimation(0) ;  /* FIST */
    OverlaySetCallback (ClientOverlayDone); //JDA

    // No target yet
    G_lastDrawTargetItem = NULL;

    /* Prepare the people parts for the bitmaps. */

    /** Make note of the fact that we are now "up" **/
    G_clientInit = TRUE ;
//OutputItems();

     DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientShootFireballAck
 *-------------------------------------------------------------------------*/
/**
 *  ClientShootFireballAck confirms that a fireball was sent, and emits
 *  the fireball-shooting sound.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientShootFireballAck (T_word32 extraData,
                               T_packetEitherShortOrLong *p_packet)
{
   DebugRoutine ("ClientShootFireballAck");

   G_attackComplete = TRUE ;

   DebugEnd ();
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientShootFireball
 *-------------------------------------------------------------------------*/
/**
 *  ClientShootFireball creates a fireball object and requests that the
 *  server add it to the game.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientShootFireball(T_void)
{
    T_sword32 x, y ;
    T_sword16 heightTarget ;
    T_3dObject *target ;
    T_sword16 tx, ty ;
    T_sword32 distance ;
    T_sword32 deltaHeight;

    T_packetLong packet;
    T_projectileAddPacket *p_addPacket;


    DebugRoutine("ClientShootFireball") ;

    G_attackComplete = FALSE ;

    /** Get a pointer to our projectile-add packet **/
    p_addPacket = (T_projectileAddPacket *)(packet.data);

    p_addPacket->command = PACKET_COMMANDCSC_PROJECTILE_CREATE;

    /** We are requesting a fireball... **/
    p_addPacket->objectType = OBJECT_TYPE_FIREBALL;

    /** ... shot by this player ... **/
    ObjectGetForwardPosition(
         PlayerGetObject(),
         PLAYER_OBJECT_RADIUS + (PLAYER_OBJECT_RADIUS/2) + 22,
         &x,
         &y) ;
    p_addPacket->x = (x>>16) ;
    p_addPacket->y = (y>>16) ;
    p_addPacket->z = PlayerGetZ16() ;
    p_addPacket->z += 10 ;
    p_addPacket->vz = 0 ;
    p_addPacket->angle = PlayerGetAngle() ;

    /** ... with speed 50 ... **/
    p_addPacket->initialSpeed = 50;

    /** ... and Z velocity (don't let the 'angle' name fool you... :) **/
    /** based on whether we are targeting anything. **/

    /* See if there is a target in view. */
    target = ViewGetMiddleTarget() ;

    /* Check if there is a target. */
    if (target == NULL)  {
        /** No target. **/
        p_addPacket->target = 0;

        /* Change the angle based on the angle we are viewing. */
        p_addPacket->vz = (((T_sword16)View3dGetUpDownAngle())<<5)/100 ;

        p_addPacket->z += p_addPacket->vz >> 2 ;
    } else  {
        /** Target. **/
        p_addPacket->target = ObjectGetServerId (target);

        /* Find where the target is located. */
        tx = ObjectGetX16(target) ;
        ty = ObjectGetY16(target) ;

        /* Calculate the distance between here and there. */
        distance = CalculateDistance(x >> 16, y >> 16, tx, ty) ;

        /* How high is the target? */
        heightTarget = ObjectGetMiddleHeight(target) ;

        /* Calculate the steps necessary to draw a straight */
        /* line to the target. */
        deltaHeight = (((T_sword32)(heightTarget - PlayerGetZ16()))<<16) / distance ;
        deltaHeight *= 40 ;

        /* Don't allow more than 45 degrees up. */
        if (deltaHeight >= 0x320000)
            deltaHeight = 0x320000 ;

        /* Don't allow more than 45 degrees down. */
        if (deltaHeight <= -0x320000)
            deltaHeight = -0x320000 ;

        p_addPacket->vz = ((T_sword16)(deltaHeight>>16)) ;
    }

    /** Now we just send the packet. **/
    packet.header.packetLength = sizeof(T_projectileAddPacket) ;
    CmdQSendPacket ((T_packetEitherShortOrLong *)&packet, 140, 0, ClientShootFireballAck);

    DebugEnd ();

    return;
}

static T_word16 G_missileToObjectType[EFFECT_MISSILE_UNKNOWN] = {
    4011,
    4018,
    4004,
    4019,
    4005,
    4020,
    4003,
    4014,
    4006,
    4012,
    4013,
    4007,
    4001,
    31,
    4002,
    4008,
    4015,
    4010,
    4000,
    4009,
    4016,
    886, /* 206, */
    887, /* 222, */
    888, /* 229, */
    889, /* 218, */
    890, /* 241, */
    891, /* 242, */
    892, /* 240  */
    4017
} ;

/* LES: 06/12/96 Created. */
T_void ClientCreateProjectile (
           E_effectMissileType type,
           T_word16 duration,
           T_word16 power)
{
    T_word16 objectType ;
    T_3dObject *p_target ;
    T_word16 target ;

    DebugRoutine("ClientCreateProjectile") ;
    DebugCheck(type < EFFECT_MISSILE_UNKNOWN) ;

    if (ClientIsInView())
    {
        G_attackComplete = FALSE ;
        objectType = G_missileToObjectType[type] ;
        /* See if there is a target in view. */
        p_target = ViewGetMiddleTarget() ;
        /* If we have a target and it is passable, don't target it. */
        if (p_target)  {
            if (ObjectIsPassable(p_target))
                p_target = NULL ;
            if (!((ObjectIsCreature(p_target)) || (ObjectIsPlayer(p_target))))
                p_target = NULL ;
        }
        if (p_target)
            target = ObjectGetServerId(p_target) ;
        else
            target = 0 ;

        ClientSyncSendActionMissileAttack(
            objectType,
            target) ;
    }
    DebugEnd() ;
}

/* LES: 12/21/95 */
T_void ClientObjectShootObject(
           T_word16 objectType,
           T_word16 sourceObject,
           T_word16 angle,
           T_word16 velocity)
{
    T_packetShort packet;
    T_projectileAddPacket *p_addPacket;
    T_sword32 x, y ;

    DebugRoutine("ClientObjectShootObject") ;
//printf("OSO: %d %d %d %d\n", objectType, sourceObject, angle, velocity) ;
    /** Get a pointer to our projectile-add packet **/
    p_addPacket = (T_projectileAddPacket *)(packet.data);

    p_addPacket->command = PACKET_COMMANDCSC_PROJECTILE_CREATE;

    /** We are requesting a fireball... **/
    p_addPacket->objectType = objectType ;

    /** ... shot by this player ... **/
    ObjectGetForwardPosition(
         PlayerGetObject(),
         PLAYER_OBJECT_RADIUS + (PLAYER_OBJECT_RADIUS/2) + 20,
         &x,
         &y) ;
    p_addPacket->x = (T_sword16)(x>>16) ;
    p_addPacket->y = (T_sword16)(y>>16) ;
    p_addPacket->z = PlayerGetZ16() ;
    p_addPacket->z += 10 ;
    p_addPacket->vz = 0 ;
    p_addPacket->angle = PlayerGetAngle() ;

    /** ... with speed 50 ... **/
    p_addPacket->initialSpeed = (T_byte8)velocity ;

    /** Target. **/
    p_addPacket->angle = angle ;

    /** Now we just send the packet. **/
    CmdQSendShortPacket (&packet, 210, 0, NULL);

    DebugEnd ();
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientAttackSent
 *-------------------------------------------------------------------------*/
/**
 *  ClientAttackSent confirms that the attack was done.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientAttackSent(T_word32 extraData, T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientAttacklSent") ;

//puts("Attack complete") ;
    G_attackComplete = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IClientGainExperienceIfHit
 *-------------------------------------------------------------------------*/
/**
 *  IClientGainExperienceIfHit checks the given object and if it is a
 *  player or a creature, the given damage value is the amount of
 *  experience earned.
 *
 *  @param p_obj -- Object being hit to check
 *  @param damage -- Amount of damage done
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IClientGainExperienceIfHit(
                      T_3dObject *p_obj,
                      T_word32 damage)
{
    E_Boolean stopSearch = FALSE ;
    T_word32 x, y;
    T_word16 num ;
    T_wallListItem wallList[20] ;

    DebugRoutine("IClientGainExperienceIfHit") ;
    DebugCheck(p_obj != NULL) ;

    /* Does this thing have a script? */
    if (!ObjectIsPassable(p_obj))
        if ((ObjectGetScript(p_obj) != 0) ||
                (ObjectIsPiecewise(p_obj)))
        {
            /* Make sure there is nothing blocking the hit */
            num = Collide3dFindWallList(
                      PlayerGetX16(),
                      PlayerGetY16(),
                      ObjectGetX16(p_obj),
                      ObjectGetY16(p_obj),
                      PlayerGetZ16() + PLAYER_ATTACK_HEIGHT,
                      20,
                      wallList,
                      WALL_LIST_ITEM_UPPER |
                          WALL_LIST_ITEM_LOWER |
                              WALL_LIST_ITEM_MAIN) ;
            if (num == 0)  {
                /* No walls are in the way. */

                /* change experience */
                StatsChangePlayerExperience((T_sword16) damage) ;
                InventoryPlayWeaponHitSound();
                stopSearch = TRUE ;

                /* Send out the attack damage. */
                ClientSyncSendActionMeleeAttack(
                    StatsGetPlayerAttackDamage(),
                    StatsGetPlayerDamageType(),
                    ObjectGetServerId(p_obj)) ;

                if (StatsHitWasCritical())
                {
                    /* calc coords for critical hit splat */
                    x=PlayerGetX();
                    y=PlayerGetY();
                    x+=(ObjectGetX(p_obj)-PlayerGetX())/4;
                    y+=(ObjectGetY(p_obj)-PlayerGetY())/4;

                    EfxCreate (EFX_WALL_HIT,
                               x,
                               y,
                               45<<16,
                               (rand()%2)+1,
                               TRUE,
                               0);
                }
            }
        }
    DebugEnd() ;

    return stopSearch ;
}

/* LES  07/30/96  Created */
T_void ClientMakeObjectGoSplat(
           T_3dObject *p_obj,
           T_word16 amount,
           E_effectDamageType damageType,
           E_effectDamageType defenseType,
           E_bloodEffect bloodType)
{
    T_sword16 numSplats;
    T_byte8 numSplatsThisTime;
    T_word32 x, y ;
    E_Boolean isTranslucent;
    T_byte8 splatType;
    E_efxType splatTypes[6]={EFX_UNKNOWN,
                             EFX_UNKNOWN,
                             EFX_UNKNOWN,
                             EFX_UNKNOWN,
                             EFX_UNKNOWN,
                             EFX_UNKNOWN};
    T_byte8   typeCount=0;

    DebugRoutine("ClientMakeObjectGoSplat") ;

    /* Determine what type of damage to do. */
    damageType &= (~defenseType) ;

    if (damageType & EFFECT_DAMAGE_NORMAL)
    {
        if (bloodType == BLOOD_EFFECT_NORMAL)
            splatTypes[typeCount++]=EFX_BLOOD_SHRAPNEL ;
        else if (bloodType == BLOOD_EFFECT_BONE)
            splatTypes[typeCount++]=EFX_BONE_SHRAPNEL ;
    }
    if (damageType & EFFECT_DAMAGE_POISON)
    {
        splatTypes[typeCount++]=EFX_POISON_SHRAPNEL;
    }
    if (damageType & EFFECT_DAMAGE_ACID)
    {
        splatTypes[typeCount++]=EFX_ACID_SHRAPNEL;
    }
    if (damageType & EFFECT_DAMAGE_FIRE)
    {
        splatTypes[typeCount++]=EFX_FIRE_SHRAPNEL;
    }
    if (damageType & EFFECT_DAMAGE_ELECTRICITY)
    {
        splatTypes[typeCount++]=EFX_ELECTRIC_SHRAPNEL;
    }
    if (damageType & EFFECT_DAMAGE_MANA_DRAIN)
    {
        splatTypes[typeCount++]=EFX_MAGIC_SHRAPNEL;
    }

    if (ObjectIsFullyPassable(p_obj))
        amount = 0 ;

    /* Check to see that we are doing any damage. */
    if ((damageType != 0) && (amount > 0)) {
        /* Only make an object splat if it is a creature or player. */
        if ((ObjectIsCreature(p_obj)) ||
            (ObjectIsPlayer(p_obj)))
        {
            isTranslucent=(ObjectGetAttributes(p_obj) & OBJECT_ATTR_TRANSLUCENT);

            x=ObjectGetX(p_obj);
            y=ObjectGetY(p_obj);

            numSplats = 1+(amount / 200) ;
            if (numSplats > 20)
                numSplats=20;

    //printf("Object %d go splat for %d splats from %s\n", ObjectGetServerId(p_obj), numSplats, DebugGetCallerName()) ;
            /* add effects based on damage type */
            if (typeCount==1)
            {
                EfxCreate (splatTypes[0],
                           x,
                           y,
                           ObjectGetZ(p_obj)+(ObjectGetHeight(p_obj)<<15),
                           numSplats,
                           isTranslucent,
                           0);
            }
            else while (numSplats > 0)
            {
                numSplatsThisTime=rand()%3+1;
                splatType=rand()%typeCount;
                EfxCreate (splatTypes[splatType],
                           x,
                           y,
                           ObjectGetZ(p_obj)+(ObjectGetHeight(p_obj)<<15),
                           numSplatsThisTime,
                           isTranslucent,
                           0);
                numSplats-=numSplatsThisTime;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientFinish
 *-------------------------------------------------------------------------*/
/**
 *  ClientFinish cleans up the client data and stuff.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientFinish (T_void)
{
   DebugRoutine ("ClientFinish");

   G_clientInit = FALSE ;

   DebugEnd ();
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientCheckScrolling
 *-------------------------------------------------------------------------*/
/**
 *  ClientCheckScrolling checks to see if the player has pressed either
 *  the Page up or Page down keys to scroll the messages.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientCheckScrolling(T_void)
{
    static T_word32 nextAttempt = 0 ;
    T_word32 time ;
    T_word32 timeNext = 12 ;

    DebugRoutine("ClientCheckScrolling") ;

    time = TickerGet() ;

    if (time >= nextAttempt)  {
        /* If this routine is called several times (like a key being held) */
        /* repeat faster. */
        if ((time-nextAttempt) < 30)
            timeNext = 4 ;

        if (KeyMapGetScan(KEYMAP_SCROLL_MSG_UP)==TRUE)  {
            MessageScrollUp() ;
            /* Only allow the scrolling of about 4 items per second. */
            nextAttempt = time+timeNext ;
        }

        if (KeyMapGetScan(KEYMAP_SCROLL_MSG_DOWN)==TRUE)  {
            MessageScrollDown() ;
            /* Only allow the scrolling of about 4 items per second. */
            nextAttempt = time+timeNext ;
        }
    }

    DebugEnd() ;
}

static void IDrawCrosshairs(T_word16 left, T_word16 top, T_word16 right, T_word16 bottom)
{
    /* Find center x and y */
    T_word16 x = (left+right)/2;
    T_word16 y = (top+bottom)/2;
    const T_byte8 crossHairColor = 31; /* almost white */

    GrDrawTranslucentPixel(x-2, y, crossHairColor) ;
    GrDrawTranslucentPixel(x-1, y, crossHairColor) ;
    GrDrawTranslucentPixel(x+1, y, crossHairColor) ;
    GrDrawTranslucentPixel(x+2, y, crossHairColor) ;

    GrDrawTranslucentPixel(x, y-2, crossHairColor) ;
    GrDrawTranslucentPixel(x, y-1, crossHairColor) ;
    GrDrawTranslucentPixel(x, y+1, crossHairColor) ;
    GrDrawTranslucentPixel(x, y+2, crossHairColor) ;
}

static T_3dObject *IDrawTargetItem(T_word16 x, T_word16 y, T_word16 drawX, T_word16 drawY)
{
    T_3dObject *p_obj = NULL;
    T_3dObject *targetObj = NULL;
    T_word16 objtype;
    T_byte8 stmp[32];
    T_resource res;
    T_byte8 *desc1;
    T_byte8 *desc2;
    T_word32 size;

    p_obj = ViewGetXYTarget(x, y);
    if (p_obj) {
        /* Turn body parts into their respective head. */
        if (ObjectIsBodyPart(p_obj))
            p_obj = ObjectFindBodyPartHead(p_obj) ;
        objtype=ObjectGetType(p_obj);
        if (ObjectIsGrabable(p_obj)) {
            if (StatsPlayerHasIdentified(objtype)) {
                // Get identified name
                sprintf((char *)stmp,"OBJDESC2/DES%05d.TXT",objtype);
            } else {
                // Get unidentified name
                sprintf((char *)stmp,"OBJDESC/DES%05d.TXT",objtype);
            }
            if (PictureExist(stmp)) {
                desc1 = PictureLockData(stmp, &res);
                size = ResourceGetSize(res);
                desc2=(T_byte8 *)MemAlloc(size+64);
                memcpy(desc2, desc1, size);
                desc2[size] = '\0';

                TextCleanString(desc2);
                ControlColorizeLookString(desc2);

                // TODO: This is hard code to the key E
                MessageDrawLine(drawX, drawY, (T_byte8 *)"E :", 31);
                MessageDrawLine(drawX+12, drawY, desc2, 31 /* white */);
                targetObj = p_obj;

                MemFree(desc2);
                PictureUnlockAndUnfind(res);
            }
        }
    }

    return targetObj;
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientHandleOverlay
 *-------------------------------------------------------------------------*/
/**
 *  ClientHandleOverlay is called when the 3D engine is done drawing
 *  the 3d view.  This routine draws everything that is to appear on top
 *  of the 3D view.
 *
 *  @param left -- left edge of view
 *  @param top -- top edge of view
 *  @param right -- right edge of view
 *  @param bottom -- bottom edge of view
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientHandleOverlay(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom)
{
    T_word16 ammocount=0;
    T_word16 ammotype=0;
    T_byte8 drawcolor=15;
    T_byte8 stmp[10];
    T_byte8 stmp2[10];
    E_Boolean isGrabDrawn = FALSE;

    DebugRoutine("ClientHandleOverlay") ;
    DebugCheck(bottom < SCREEN_SIZE_Y) ;
    DebugCheck(right < SCREEN_SIZE_X) ;
    DebugCheck(top < bottom) ;
    DebugCheck(left < right) ;

    /* Turn on translucency if player is translucent or invisible. */
    OverlaySetTranslucencyMode(
        (EffectPlayerEffectIsActive(
            PLAYER_EFFECT_TRANSLUCENT) |
         EffectPlayerEffectIsActive(
            PLAYER_EFFECT_INVISIBLE))?TRUE:FALSE) ;

    /* Draw the current weapon on the screen. */
    OverlayDraw(left, top, right, bottom, -VIEW3D_CLIP_LEFT, 0) ;
    OverlayUpdate(ClientIsPaused()) ;

    /* Draw crosshairs */
    if (MouseIsRelativeMode()) {
        IDrawCrosshairs(left, top, right, bottom);

        G_lastDrawTargetItem = IDrawTargetItem((left+right)/2, (top+bottom)/2, left+4, bottom-7);
    } else {
        G_lastDrawTargetItem = NULL;
    }

    /* Check if we are in message mode. */
    if (G_msgOn == TRUE)  {
        /* If so, we'll draw the message being entered now. */
        G_message[G_msgPos] = '_' ;
        G_message[G_msgPos+1] = '\0' ;

        GrSetCursorPosition(left+5, top+100) ;
        GrDrawShadowedText(G_message, COLOR_YELLOW, COLOR_BLACK) ;

        G_message[G_msgPos] = '\0' ;
    }

    EffectDrawEffectIcons (left, right, top, bottom);

    /* Draw the messages at the top. */
    MessageDraw(left+2, top+2, 10, COLOR_WHITE) ;

    /* if a bow is being used, draw ammo counter */
    if ((InventoryWeaponIsBow()) && (!isGrabDrawn))
    {
        ammotype=BannerGetSelectedAmmoType();
        if (ammotype==BOLT_TYPE_UNKNOWN) ammotype=BOLT_TYPE_NORMAL;

        switch (ammotype)
        {
            case BOLT_TYPE_NORMAL:
            drawcolor=211;
            break;

            case BOLT_TYPE_POISON:
            drawcolor=128;
            break;

            case BOLT_TYPE_PIERCING:
            drawcolor=31;
            break;

            case BOLT_TYPE_FIRE:
            drawcolor=144;
            break;

            case BOLT_TYPE_ELECTRICITY:
            drawcolor=160;
            break;

            case BOLT_TYPE_MANA_DRAIN:
            drawcolor=112;
            break;

            case BOLT_TYPE_ACID:
            drawcolor=83;
            break;

            default:
            DebugCheck(0);
            break;
        }

        ammocount=StatsGetPlayerBolts(BannerGetSelectedAmmoType());
        GrSetCursorPosition (left+4,bottom-7);
        strcpy (stmp,"AMMO:");
        sprintf (stmp2,"%d",ammocount);

        if (ammocount > 0)
        {
            GrDrawShadowedText (stmp,128,COLOR_BLACK);
            GrSetCursorPosition (left+28,bottom-7);
            GrDrawShadowedText (stmp2,drawcolor,COLOR_BLACK);
        }
        else
        {
            GrDrawShadowedText (stmp,144,COLOR_BLACK);
            GrSetCursorPosition (left+28,bottom-7);
            GrDrawShadowedText (stmp2,144,COLOR_BLACK);
        }
    }

#ifndef NDEBUG
#if 0
    GrSetCursorPosition(left+2, top+50) ;
    if (PlayerIsStealthy())  {
        GrDrawShadowedText(
             "STEALTH",
             COLOR_YELLOW,
             COLOR_BLACK) ;
    }
#endif
#endif

#if 0
/* TESTING ... */
n = PlayerGetNumAreaSectors() ;
GrSetCursorPosition(20, 120) ;
GrDrawShadowedText("Area sectors: ", COLOR_WHITE, COLOR_BLACK) ;
for (i=0; i<n; i++)  {
  sprintf(buffer, "%u ", PlayerGetNthAreaSector(i)) ;
  GrDrawShadowedText(buffer, COLOR_WHITE, COLOR_BLACK) ;
}
/* ... TESTING */
#endif

    /* Draw the frames per second we are getting. */
    IClientUpdateAndDrawFramesPerSecond(left, bottom) ;

    /* Draw the system status. */
#ifndef NDEBUG
    IClientDrawStatus(left, bottom) ;
#endif

    if (ClientIsPaused())  {
        if (SyncTimeGet() <= 1)  {
            if (ClientSyncGetNumberPlayers() > 1)  {
                GrSetCursorPosition(5+((VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-100)>>1), 80) ;
                GrDrawShadowedText("WAITING FOR OTHER PLAYERS", COLOR_WHITE, COLOR_BLACK) ;
            }
        } else {
            GrSetCursorPosition(5+((VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-50)>>1), 80) ;
            GrDrawShadowedText("GAME PAUSED", COLOR_WHITE, COLOR_BLACK) ;
        }
    }
    if (BannerFormIsOpen(BANNER_FORM_COMMUNICATE))  {
        GrSetCursorPosition(5+((VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-144)>>1), 90) ;
        GrDrawShadowedText("MOVEMENT BLOCKED WHILE COMMUNICATING", COLOR_WHITE, COLOR_BLACK) ;
    }
    if (BannerFormIsOpen(BANNER_FORM_NOTES))  {
        GrSetCursorPosition(5+((VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-140)>>1), 90) ;
        GrDrawShadowedText("MOVEMENT BLOCKED WHILE MAKING NOTES", COLOR_WHITE, COLOR_BLACK) ;
    }

    DebugEnd() ;
}

char G_clientSyncStatus[50] = "" ;

T_void ClientSetClientSyncStatus(T_byte8 *p_msg)
{
    strcpy(G_clientSyncStatus, p_msg) ;
}

static T_void IClientDrawStatus(T_word16 left, T_word16 bottom)
{
    GrSetCursorPosition(left+3, bottom-30) ;
    GrDrawShadowedText(G_clientSyncStatus, COLOR_WHITE, COLOR_BLACK) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetDelta
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetDelta returns the number of timer clicks (ticks) that have
 *  passed since this routine was last called.
 *
 *  @return Ticks since last call.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ClientGetDelta(T_void)
{
    static T_word32 lastTime = 0 ;
    T_word32 newTime ;
    T_word16 delta ;

    DebugRoutine("ClientGetDelta") ;

    newTime = TickerGet() ;
    delta = newTime - lastTime ;
    lastTime = newTime ;

    DebugEnd() ;

    return delta ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientLogin
 *-------------------------------------------------------------------------*/
/**
 *  ClientLogin requests to login into the server.
 *
 *  NOTE: 
 *  It is assumed that the client has already attached to server and there
 *  is a data communications path open.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientLogin(T_void)
{
    T_packetShort packet ;
    T_loginPacket *p_login;

    DebugRoutine("ClientLogin") ;

    /** Get a quick pointer. **/
    p_login = (T_loginPacket *)(packet.data);

    G_logoutAttempted = FALSE ;

    /** Request a login as the character of my choice. **/
    p_login->command = PACKET_COMMAND_LOGIN ;
    p_login->accountNum = 0; // not used

    CmdQSendShortPacket(&packet, 140, 0, IClientLoginAck) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IClientLoginAck
 *-------------------------------------------------------------------------*/
/**
 *  IClientLoginAck notes that the player is successfully connected to
 *  the server.
 *
 *<!-----------------------------------------------------------------------*/
T_void IClientLoginAck(
                  T_word32 extraData,
                  T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("IClientLoginAck") ;

    G_clientIsLogin = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientLogoffFinish
 *-------------------------------------------------------------------------*/
/**
 *  ClientLogoffFinish declares the client no longer on.  This routine
 *  closes out the client and anything else that must be done.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientLogoffFinish(
           T_word32 extraData,
           T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientLogoffFinish") ;

    /* Make sure we are logged on. */
    if (G_clientIsLogin == TRUE)  {
        /* We are no longer on. */
        G_clientIsLogin = FALSE ;

        KeyboardSetEventHandler(NULL) ;

        ButtonCleanUp() ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientLogoff
 *-------------------------------------------------------------------------*/
/**
 *  ClientLogoff tells the server that the client is leaving.
 *
 *  NOTE: 
 *  It is assumed that the client has already attached to server and there
 *  is a data communications path open.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientLogoff(T_void)
{
    DebugRoutine("ClientLogoff") ;

    if (G_logoutAttempted == FALSE)  {
//puts("Sending logoff packet") ;  fflush(stdout) ;
        /* All I can do is ask for a logoff. */
/*
        p_packet = (T_logoffPacket *)packet.data ;
        p_packet->command = PACKET_COMMAND_PLAYER_LOGOFF ;
        p_packet->player = ClientGetLoginId() ;

        G_logoutAttempted = TRUE ;

        CmdQSendShortPacket(&packet, 140, 0, ClientLogoffFinish) ;
*/
//ClientLogoffFinish(0, (T_packetEitherShortOrLong *)&packet) ;
//ServerReceivePlayerLogoffPacket((T_packetEitherShortOrLong *)&packet) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientIsLogin
 *-------------------------------------------------------------------------*/
/**
 *  Simple put, "Am I logged into the server?"
 *
 *  @return FALSE = no, TRUE = yes
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ClientIsLogin(T_void)
{
    E_Boolean isLogin ;

    DebugRoutine("ClientIsLogin") ;

    isLogin = G_clientIsLogin ;

    DebugEnd() ;

    return isLogin ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientUpdate
 *-------------------------------------------------------------------------*/
/**
 *  ClientUpdate does all the activities that a client needs to do for
 *  a single time slice.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientUpdate(T_void)
{
    T_word16 sector;
    T_sword32 timeHeld ;
    T_sword32 timeHeld2 ;
    T_word16 nextMap ;
    T_word16 nextMapPlace ;
    T_word16 playerMoveAngle = 0 ;
    T_word16 newAngle = 0 ;
    E_Boolean shift ;
    T_sword16 x, y, newx, newy ;
    T_word32 delta ;
    T_sword16 relx, rely;
    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;

    DebugRoutine("ClientUpdate") ;
    INDICATOR_LIGHT(34, INDICATOR_GREEN) ;
    DebugTime(3) ;

    DebugCompare("ClientUpdate") ;
    if ((G_clientIsActive == TRUE) && (G_logoutAttempted == FALSE))  {
        if (G_clientMode == CLIENT_MODE_GAME)  {
            AreaSoundCheck() ;
            PlayerSetRealMode() ;

            ViewUpdatePlayer() ;
            PlayerSetCameraView() ;
            ViewDraw() ;

            KeyboardUpdateEvents() ;

            if (G_clientMode != CLIENT_MODE_GAME)  {
                DebugEnd() ;
                TICKER_TIME_ROUTINE_ENDM("ClientUpdate", 100) ;
                return ;
            }
            DebugCompare("ClientUpdate") ;

            /* Get the XY location of the current location. */
            x = PlayerGetX16() ;
            y = PlayerGetY16() ;

            shift = FALSE ;
            if ((!KeyMapGetScan(KEYMAP_RUN)) &&
                 (G_msgOn == FALSE) && ((PlayerIsAboveGround()==FALSE) ||
                 (EffectPlayerEffectIsActive (PLAYER_EFFECT_FLY)==TRUE)))  {
                shift = TRUE ;
            }

            if ((KeyMapGetScan(KEYMAP_JUMP)==TRUE) && (!EscapeMenuIsOpen()) &&
                 (G_msgOn == FALSE))  {
                /* Do a jump manuever. */
                if (!ClientIsDead())
                    PlayerJump(StatsGetJumpPower()) ;
            }

            DebugCompare("ClientUpdate") ;

            GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
            GraphicUpdateAllGraphics();

            delta = ClientGetDelta() ;

#if 0
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_F5)==TRUE))  {
                MapSetOutsideLighting(MapGetOutsideLighting()-1) ;
                sprintf(buffer, "Light level: %d", MapGetOutsideLighting()) ;
                MessageAdd(buffer) ;
            }
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_F6)==TRUE))  {
                MapSetOutsideLighting(MapGetOutsideLighting()+1) ;
                sprintf(buffer, "Light level: %d", MapGetOutsideLighting()) ;
                MessageAdd(buffer) ;
            }
#endif
#if 0
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_F4)==TRUE))  {
                if (ViewIsUnderwater())  {
                    ViewUnderwaterOff() ;
                    MessageAdd("Underwater off.") ;
                }  else  {
                    ViewUnderwaterOn() ;
                    MessageAdd("Underwater on.") ;
                }
            }
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_F3)==TRUE))  {
                ViewSetDarkSight(ViewGetDarkSight()+4) ;
                sprintf(buffer, "Dark adjust: %d", ViewGetDarkSight()) ;
                MessageAdd(buffer) ;
            }
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_F2)==TRUE))  {
                ViewSetDarkSight(ViewGetDarkSight()-4) ;
                sprintf(buffer, "Dark adjust: %d", ViewGetDarkSight()) ;
                MessageAdd(buffer) ;
            }
#endif
            playerMoveAngle = PlayerGetAngle() ;

            if (!EscapeMenuIsOpen()) {
            if (MouseIsRelativeMode()) {
                // If in relative mouse mode, we turn using the mouse
                MouseRelativeRead(&relx, &rely);

                // Invert up/down if configured
                if (ConfigGetInvertMouseY())
                    rely = -rely;

                // If in relative mouse mode, we turn using the mouse
                if (ClientIsPaused()) {
                    relx = 0;
                    rely = 0;
                }
                if (relx < 0) {
                    PlayerTurnLeft(-relx) ;
                } else if (relx > 0) {
                    PlayerTurnRight(relx) ;
                }

                // Look up or down
                if (rely >= 0) {
                    View3dSetUpDownAngle(View3dGetUpDownAngle() - (3*rely*delta)/100) ;
                } else if (rely < 0) {
                    View3dSetUpDownAngle(View3dGetUpDownAngle() + (3*(-rely)*delta)/100) ;
                }

                if ((PlayerIsAboveGround()==FALSE) ||
                        (EffectPlayerEffectIsActive(PLAYER_EFFECT_FLY)==TRUE))
                {
                    timeHeld = KeyMapGetHeld(KEYMAP_TURN_LEFT)*8 ;
                    if (timeHeld)  {
                        timeHeld += (timeHeld>>1) ;
                        PlayerAccelDirection(playerMoveAngle+INT_ANGLE_90, timeHeld) ;
                    }

                    timeHeld = KeyMapGetHeld(KEYMAP_TURN_RIGHT)*8 ;
                    if (timeHeld)  {
                        timeHeld += (timeHeld>>1) ;
                        PlayerAccelDirection(playerMoveAngle-INT_ANGLE_90, timeHeld) ;
                    }
                } else {
                    /* Clear the keys */
                    timeHeld = KeyMapGetHeld(KEYMAP_TURN_LEFT) ;
                    timeHeld = KeyMapGetHeld(KEYMAP_TURN_RIGHT) ;
                }
            } else {
                if ((KeyMapGetScan(KEYMAP_SIDESTEP) == TRUE) && (!ClientIsDead()))  {
                    if ((PlayerIsAboveGround()==FALSE) ||
                          (EffectPlayerEffectIsActive(PLAYER_EFFECT_FLY)==TRUE))
                    {
                        timeHeld = KeyMapGetHeld(KEYMAP_TURN_LEFT)*8 ;
                        if (timeHeld)  {
                            timeHeld += (timeHeld>>1) ;
                            PlayerAccelDirection(playerMoveAngle+INT_ANGLE_90, timeHeld) ;
                        }

                        timeHeld = KeyMapGetHeld(KEYMAP_TURN_RIGHT)*8 ;
                        if (timeHeld)  {
                            timeHeld += (timeHeld>>1) ;
                            PlayerAccelDirection(playerMoveAngle-INT_ANGLE_90, timeHeld) ;
                        }
                    } else {
                        /* Clear the keys */
                        timeHeld = KeyMapGetHeld(KEYMAP_TURN_LEFT) ;
                        timeHeld = KeyMapGetHeld(KEYMAP_TURN_RIGHT) ;
                    }
                } else {
                    // If NOT in relative mouse mode, we turn with the keyboard
                    if (KeyMapGetScan(KEYMAP_TURN_LEFT)) {
                        PlayerTurnLeft((shift)?512:256) ;
                    }
                    if (KeyMapGetScan(KEYMAP_TURN_RIGHT)) {
                        PlayerTurnRight((shift)?512:256) ;
                    }
                }
                if (!G_msgOn)  {
                    if (KeyMapGetScan(KEYMAP_LOOK_DOWN)==TRUE)  {
                            /* Look down */
                            View3dSetUpDownAngle(View3dGetUpDownAngle() - 3*delta) ;
                    }
                    if (KeyMapGetScan(KEYMAP_LOOK_UP)==TRUE)  {
                            /* Look up */
                            View3dSetUpDownAngle(View3dGetUpDownAngle() + 3*delta) ;
                    }
                    if (KeyMapGetScan(KEYMAP_READJUST_VIEW)==TRUE)
                        View3dSetUpDownAngle(0) ;
                }
            }
              if ((PlayerIsAboveGround()==FALSE) ||
                (EffectPlayerEffectIsActive(PLAYER_EFFECT_FLY)==TRUE))
            {
                if (!ClientIsDead())  {
                    timeHeld = KeyMapGetHeld(KEYMAP_FORWARD)*8 ;
                    if (timeHeld)  {
                        timeHeld += (timeHeld>>1) ;
                        PlayerAccelDirection(playerMoveAngle, timeHeld<<(shift)) ;
                        PlayerSetStance(STANCE_WALK) ;
                    }
                    timeHeld2 = KeyMapGetHeld(KEYMAP_BACKWARD)*8 ;
                    if (timeHeld2)  {
                        timeHeld2 += (timeHeld2>>1) ;
                        PlayerAccelDirection(playerMoveAngle, -(timeHeld2<<(shift))) ;
                        PlayerSetStance(STANCE_WALK) ;
                    }
                    if ((!timeHeld) && (!timeHeld2))  {
                        PlayerSetStance(STANCE_STAND) ;
                    }
                } else {
                    timeHeld = KeyMapGetHeld(KEYMAP_FORWARD) ;
                    timeHeld2 = KeyMapGetHeld(KEYMAP_BACKWARD) ;
                }
            } else {
                timeHeld = KeyMapGetHeld(KEYMAP_FORWARD)*8 ;
                if ((timeHeld) && ((rand() % 5)==0))  {
                    PlayerAccelDirection(playerMoveAngle, timeHeld) ;
                    PlayerSetStance(STANCE_WALK) ;
                }
                timeHeld2 = KeyMapGetHeld(KEYMAP_BACKWARD)*8 ;
                if ((timeHeld2) && ((rand() % 5)==0))  {
                    PlayerAccelDirection(playerMoveAngle, -timeHeld) ;
                    PlayerSetStance(STANCE_WALK) ;
                }
                if ((!timeHeld) && (!timeHeld2))  {
                    PlayerSetStance(STANCE_STAND) ;
                }
            }

            if (MouseIsRelativeMode()) {
                // Look mode
                if (MouseGetButtonStatus() & MOUSE_BUTTON_LEFT) {
                    // Player clicked left mouse button.  Use item in hand
                    if (InventoryCanUseItemInReadyHand())
                        InventoryUseItemInReadyHand(NULL);
                }
                if (MouseGetButtonStatus() & MOUSE_BUTTON_RIGHT) {
                    // But only if we have release the mouse button at some point
                    if (G_pressedCastButton == FALSE) {
                        // Try to cast the active spell
                       SpellsCastSpell(0 /* not used */);
                       G_pressedCastButton = TRUE;
                    }
                } else {
                    G_pressedCastButton = FALSE;
                }
            } else {
                // Not casting spell
                G_pressedCastButton = FALSE;

                // Cursor mode
                MouseUpdateEvents();
            }
            } else {
                MouseUpdateEvents();
            }

            /* Get our new location (after we hit walls and such) */
            newx = PlayerGetX16() ;
            newy = PlayerGetY16() ;

            /* Tell the server where the player is. */
            newAngle = playerMoveAngle ;

            PlayerSetAngle(playerMoveAngle) ;
            DebugCompare("ClientUpdate") ;

            DebugCompare("ClientUpdate") ;

            ClientCheckScrolling() ;
            DebugCompare("ClientUpdate") ;

            /* See if we need to do an action. */
            sector = PlayerGetAreaSector() ;

            if (sector != PlayerGetLastSector())  {
                DebugCheck(sector < G_Num3dSectors) ;
                PlayerSetLastSector(sector) ;

                MapGetNextMapAndPlace(sector, &nextMap, &nextMapPlace) ;

                if (nextMap)  {
                    /* Tell everyone that I'm going to another level */
                    ClientSyncSendActionLeaveLevel(nextMap) ;
                }
            }

            DebugCompare("ClientUpdate") ;
            ClientUpdateHealth() ;

            DebugCompare("ClientUpdate") ;
            if (!ClientIsPaused())
            {
                BannerUpdate() ;
            }
            StatsUpdatePlayerStatistics();
            /* Back into fake mode */
            PlayerSetFakeMode() ;

        } else if (G_clientMode == CLIENT_MODE_SCRIPT_FORM)  {
            MouseHide() ;
              KeyboardUpdateEvents() ;
            MouseUpdateEvents();
            ScriptFormUpdate() ;
            MouseShow() ;
        } else if (ClientGetMode() == CLIENT_MODE_HARD_CODED_FORM)  {
            delta = ClientGetDelta() ;
            HardFormUpdate() ;

            /* Escape makes the user go back a ui screen, or log out */
            if (KeyboardGetScanCode(KEY_SCAN_CODE_ESC))  {
                if (ClientGetCurrentPlace() ==
                    (HARDFORM_GOTO_PLACE_OFFSET+HARD_FORM_TOWN))  {
                    MessageClear() ;
                    ClientGotoPlace(0, 0) ;
                } else {
                    MouseRelativeModeOff();

                    ClientGotoPlace(20004, 0) ;
                }
            }
        }

        if (G_nextPlace != 0xFFFF)  {
            ClientGotoPlace(G_nextPlace, G_nextStart) ;
        }
    }
    DebugCompare("ClientUpdate") ;
    DebugEnd() ;

    INDICATOR_LIGHT(34, INDICATOR_RED) ;
    TICKER_TIME_ROUTINE_ENDM("ClientUpdate", 500) ;
}

E_clientDialResult ClientDialIn(T_byte8 *p_init, T_byte8 *p_dial)
{
    return CLIENT_DIAL_RESULT_CONNECTED ;
}

T_void ClientUpdateHealth()
{

    DebugRoutine("ClientUpdateHealth") ;

    if (StatsGetPlayerHealth()<100) //flash red behind heart
    {
        ColorSetGlobal(0,-15,-15);
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientTakeDamage
 *-------------------------------------------------------------------------*/
/**
 *  ClientTakeDamage is the one routine that should be called when
 *  a player takes damage.
 *
 *  @param amount -- Amount of damage to give
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientTakeDamage(T_word16 amount)
{
    DebugRoutine("ClientTakeDamage") ;

    StatsTakeDamage (EFFECT_DAMAGE_NORMAL,(T_sword16)amount);

    DebugEnd() ;
}

void IClientHandleMapKeys(T_word16 scankey)
{
    T_sword32 zoom;

    if (scankey == KeyMap(KEYMAP_MAP_SMALLER_SCALE)&& ClientIsGod()) {
        if (OverheadIsOn()) {
            zoom = OverheadGetZoomFactor();
            zoom -= 0x50;
            if (zoom >= OVERHEAD_ZOOM_MAX)
                OverheadSetZoomFactor(zoom);
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_BIGGER_SCALE)&& ClientIsGod()) {
        if (OverheadIsOn()) {
            zoom = OverheadGetZoomFactor();
            zoom += 0x50;
            if (zoom <= OVERHEAD_ZOOM_MIN)
                OverheadSetZoomFactor(zoom);
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_DISPLAY)) {
        if (EffectPlayerEffectIsActive(PLAYER_EFFECT_MAGIC_MAP)==TRUE) {
            OverheadToggle();
        } else {
            if (OverheadIsOn()) {
                OverheadToggle();
            } else {
                SoundDing();
                MessageAdd("Magic map only available with spell");
            }
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_SEE_THROUGH)) {
        if (OverheadIsOn()) {
            OverheadSetFeatures(
                    OverheadGetFeatures() ^
                    OVERHEAD_FEATURE_TRANSPARENT);
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_TRANSLUCENT)) {
        if (OverheadIsOn()) {
            OverheadSetFeatures(
                    OverheadGetFeatures() ^
                    OVERHEAD_FEATURE_TRANSLUCENT);
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_ROTATE))
    {
        if (OverheadIsOn()) {
            OverheadSetFeatures(
                    OverheadGetFeatures() ^
                    OVERHEAD_FEATURE_ROTATE_VIEW);
        }
    }
    if (scankey == KeyMap(KEYMAP_MAP_POSITION))
    {
        if (OverheadIsOn()) {
            OverheadSetPosition(
                    (OverheadGetPosition()+1) %
                    OVERHEAD_POSITION_UNKNOWN);
        }
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientHandleKeyboard
 *-------------------------------------------------------------------------*/
/**
 *  ClientHandleKeyboard takes care of all keyboard events created by
 *  the client.
 *
 *  @param event -- Keyboard event to process
 *  @param scanKey -- Key to go with event
 *
 *<!-----------------------------------------------------------------------*/
T_mapHandle G_mapHandleA = MAP_HANDLE_BAD ;
T_mapHandle G_mapHandleB = MAP_HANDLE_BAD ;
T_word16 G_mapAorB = 0 ;  /* 0 = A, 1 = B */
extern E_Boolean G_serverActive ;

static T_word16 G_escCount = 0 ;

T_void ClientHandleKeyboard(E_keyboardEvent event, T_word16 scankey)
{
    T_byte8 buffer[80] ;
    T_sword16 x, y ;
    T_word16 i;
    T_word16 angle ;
    T_sword16 height ;
    T_TxtboxID TxtboxID;
    static T_word32 lastTime = 0 ;
    T_buttonID temp=NULL;
    T_sword32 teleportX, teleportY ;
    T_word32 teleportAngle ;
    E_Boolean removedNotes=FALSE;
    FILE *fp ;
    static E_Boolean wasGamma = FALSE;

    DebugRoutine("ClientHandleKeyboard") ;

    /* branch some controls on special cases */
    /* check to see if communicate window is active */
    if (ComwinIsOpen() && ClientIsInView()) {
        /* pass key to comm window */
        TxtboxID = FormGetObjID(501);
        if (TxtboxIsSelected(TxtboxID)) {
            TxtboxKeyControl(event, scankey);
        }
    }

    if (BannerFormIsOpen(BANNER_FORM_NOTES) && ClientIsInView()) {
        TxtboxKeyControl(event, scankey);
    }

    if (HardFormIsOpen()) {
        TxtboxKeyControl(event, scankey);
        ButtonKeyControl(event, scankey);
    }

/****************************************************************/
/* These are keystrokes that apply only to game play (view up): */
/****************************************************************/
    if (ClientIsInView()) {
        /***********************/
        /* God mode only stuff */
        /***********************/
        if (ClientIsGod()) {
            switch (event) {
                case KEYBOARD_EVENT_HELD:
#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
                    switch(scankey)
                    {
                        case KEY_SCAN_CODE_Q: /* left */
                        MapShiftTextureLeft(1);
                        break;
                        case KEY_SCAN_CODE_W: /* right */
                        MapShiftTextureRight(1);
                        break;
                        case KEY_SCAN_CODE_A: /* up */
                        MapShiftTextureUp(1);
                        break;
                        case KEY_SCAN_CODE_Z: /* down */
                        MapShiftTextureDown(1);
                        break;
                    }
#endif
                break;


                case KEYBOARD_EVENT_BUFFERED:
                break;

                case KEYBOARD_EVENT_PRESS:
                    if (G_msgOn == FALSE) {
                        if (scankey == KEY_SCAN_CODE_8) {
                            ClientGotoPlace(1, 0);
                        }
                        if (scankey == KEY_SCAN_CODE_9) {
                            ClientGotoPlace(2, 0);
                        }
                        if (scankey == KEY_SCAN_CODE_0) {
                            ClientGotoPlace(10000, 0);
                        }
                        if (scankey == KEY_SCAN_CODE_F8) {
                            angle = PlayerGetAngle();
                            x = PlayerGetX() >> 16;
                            y = PlayerGetY() >> 16;
                            height = PlayerGetZ() >> 16;
                            sprintf(buffer, "X: %d  Y: %d  H: %d  angle: %d", x,
                                    y, height, angle);
                            MessageAdd(buffer);
                        }

                        /* Check to see if ALT-T is hit.  This teleports to */
                        /* a previously stored location. */
                        if (scankey == KEY_SCAN_CODE_T) {
                            if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT) == TRUE) {
                                fp = fopen("teleport.dat", "r");
                                fscanf(fp, "%d %d %u", &teleportX, &teleportY,
                                        &teleportAngle);
                                fclose(fp);
                                PlayerTeleport(teleportX, teleportY,
                                        teleportAngle);
                                MessageAdd("Teleport position retrieved.");
                            }
                        }
                        /* Check to see if ALT-T is hit.  This teleports to */
                        /* a previously stored location. */
                        if (scankey == KEY_SCAN_CODE_S) {
                            if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT) == TRUE) {
                                fp = fopen("teleport.dat", "w");
                                fprintf(fp, "%d %d %u", PlayerGetX16(),
                                        PlayerGetY16(), PlayerGetAngle());
                                fclose(fp);
                                MessageAdd("Teleport position stored.");
                            }
                        }

                        /* advance a 'adventure' number on key a */
                        if (scankey == KEY_SCAN_CODE_V) {
                            StatsSetCompletedAdventure(
                                    StatsGetCompletedAdventure()+1);
                            MessagePrintf("LastAdventureCompleted=%d",
                                    StatsGetCompletedAdventure());
                        }
                    }
                    break;

                case KEYBOARD_EVENT_RELEASE:
                    break;


                default:
                    break;
            }
        }

        /**************************/
        /* Normal (non-god) stuff */
        /**************************/
        switch (event)
        {
            case KEYBOARD_EVENT_HELD:
                break;

            case KEYBOARD_EVENT_BUFFERED:
                if (G_msgOn == TRUE) {
                    if (scankey == '\b') {
                        if (G_msgPos > 0) {
                            G_msgPos--;
                            G_message[G_msgPos] = '\0';
                        }
                    } else if ((scankey == '\r') || (scankey == '\n')) {
                        /*******************/
                        /* SPECIAL:::::::: */
                        /* God only stuff! */
                        /*******************/
                        if (ClientIsGod()) {
                            if (strncmp(G_message, "@goto", 5) == 0) {
                                sscanf(G_message + 5, "%d%d%d%d", &x, &y,
                                        &height, &angle);
                                PlayerTeleport(x, y, angle);
                            } else if (strncmp(G_message, "@jump", 5) == 0) {
                                sscanf(G_message + 5, "%d", &x);
                                ClientSetNextPlace(x, 0);
                            } else if (strncmp(G_message, "@play", 5) == 0) {
                                sscanf(G_message + 5, "%d", &x);
                                SoundPlayByNumber(x, 255);
                            } else if (strncmp(G_message, "@where", 6) == 0) {
                                char buf[40];
                                sprintf(buf, "Location: %d, %d, %d",
                                        PlayerGetX16 (), PlayerGetY16 (),
                                        PlayerGetZ16 ());
                                MessageAdd(buf);
                            } else if (strncmp(G_message, "@blam", 5) == 0) {
                                MessageAdd("Creatures destroyed");
                                MessagePrintf("Total XP: %ld",
                                        CreaturesKillAll());
                            } else if (strncmp(G_message, "@alleyes", 8) == 0) {
                                MessageAdd("Creatures on you");
                                CreaturesAllOnOneTarget(
                                        ObjectGetServerId(PlayerGetObject())
                                                - 100);
                            }
#if defined(_DEBUG)
                            else if (strncmp(G_message, "@heap", 5) == 0) {
                                sscanf(G_message + 5, "%d", &x);
                                if (x) {
                                    MessageAdd("Heap Check On");
                                    DebugHeapOn();
                                } else {
                                    MessageAdd("Heap Check Off");
                                    DebugHeapOff();
                                }
                            }
#endif
                            else if (strncmp(G_message, "@fps", 4) == 0) {
                                if (G_fpsOn) {
                                    MessageAdd("FPS turned off");
                                    G_fpsOn = FALSE;
                                } else {
                                    MessageAdd("FPS turned on");
                                    G_fpsOn = TRUE;
                                }
                            } else if (strncmp(G_message, "@create", 7) == 0) {
                                if (!InventoryObjectIsInMouseHand()) {
                                    y = 0;
                                    sscanf(G_message + 7, "%d%d", &x, &y);
                                    Effect(EFFECT_CREATE_OBJECT,
                                            EFFECT_TRIGGER_NONE, y, /* Object acc data */
                                            0, x, /* Object type */
                                            NULL );
                                } else {
                                    MessageAdd(
                                            "Can't create while item in hand.");
                                }
                            } else if (strncmp(G_message, "@freemem", 8) == 0) {
                                T_byte8 buf[50];

#if (WIN32)
                                sprintf(buf,
                                        "Free memory = <unknown for windows>");
                                MessageAdd(buf);
#else
                                T_word32 memInfo[12];
                                union REGS regs;
                                struct SREGS sregs;

                                regs.x.eax = 0x500;
                                memset(&sregs, 0, sizeof(sregs));
                                sregs.es = FP_SEG(memInfo);
                                regs.x.edi = FP_OFF(memInfo);
                                int386x(0x31, &regs, &regs, &sregs);

                                sprintf (buf, "Free memory = %lu", memInfo[0] + _memavl());
                                MessageAdd (buf);
#endif
                            } else /** Only ClientSendMessage if not interpreted **/
                            {
                                ClientSendMessage(G_message);
                            }
                            KeyboardBufferOff();
                            G_msgOn = FALSE;
                        } else {
                            /* client is not a god, just send message */
                            {
                                ClientSendMessage(G_message);
                            }
                            KeyboardBufferOff();
                            G_msgOn = FALSE;
                        }
                    } else if (scankey == 0x1b /* ESC */) {
                        G_msgPos = 0;
                        G_msgOn = FALSE;
                        KeyboardBufferOff();
                    } else if (((isalnum(scankey)) || (ispunct(scankey))
                            || (scankey == ' ')) && (scankey != 0)) {
                        if (G_msgPos < MAX_MESSAGE_LEN) {
                            G_message[G_msgPos++] = (T_byte8)scankey;
                        }
                    }
                } else {
                    /* Messages are off, update screen buttons */
                    ButtonKeyControl(event, scankey);  //JDA
                }
                break;


            case KEYBOARD_EVENT_PRESS:
                if (scankey == KeyMap(KEYMAP_LOOK_MODE)) {
                    // Toggle relative mouse mode
                    if (MouseIsRelativeMode()) {
                        // Turn off the mouse
                        MouseRelativeModeOff();
                    } else {
                        // Turn on the mouse
                        MouseRelativeModeOn();
                    }
                }
#if 0
                if (scankey == KEY_SCAN_CODE_GRAVE) {
                    if (BannerFormIsOpen(BANNER_FORM_ESC_MENU))
                        BannerCloseForm();
                    else
                        BannerOpenForm(BANNER_FORM_ESC_MENU);
                }
#endif
                if (scankey == KeyMap(KEYMAP_PAUSE_GAME))
                    ClientSyncSendActionPauseGameToggle();
                if (/*(KeyMapGetScan(KEYMAP_ABORT_LEVEL)==TRUE)*/ (scankey == KEY_SCAN_CODE_ESC)
                        && (!BannerIsOpen())) {
#if 0
                    if (G_escCount==1) {
                        ClientSyncSendActionAbortLevel();
                        MessageAdd("Requesting to abort level.");
                        G_escCount = 2;
                    } else if (G_escCount == 2) {
                        MessageAdd("Hit once more to force an abort...");
                        G_escCount = 3;
                    } else if (G_escCount == 3) {
                        MessageAdd("Forced abort!");
                        G_escCount = 4;

                        /* Declare this as no longer an adventure */
                        ClientSetAdventureNumber(0);
                        /* Return to guild. */
                        MouseRelativeModeOff();
                        ClientSetNextPlace(20004, 0);
                    } else {
                        MessageAdd("Hit again to abort level...");
                        G_escCount = 1;
                    }
#else
                    EscapeMenuOpen();
#endif
                } else {
                    if (G_escCount != 0) {
                        G_escCount = 0;
                        MessageAdd("Nevermind.");
                    }
                }
                if (KeyboardGetScanCode(KEY_SCAN_CODE_ESC)==TRUE) {
                    /* ESC key closes banner */
                    if (BannerIsOpen()) {
                        BannerCloseForm();
                    }
                }

                if (KeyMapGetScan(KEYMAP_TIME_OF_DAY)==TRUE)
                    MapOutputTimeOfDay();

                if (G_msgOn==FALSE) {
                    IClientHandleMapKeys(scankey);
                if (scankey == KeyMap(KEYMAP_OPEN))
                {
                     IClientAttemptOpeningForwardWall() ;
                }

                /* Grab item if relative mode */
                // TODO: Need to be able to remap key!
                if ((MouseIsRelativeMode()) && (G_lastDrawTargetItem)) {
                    if (KeyMapGetScan(KEYMAP_ACTIVATE_OR_TAKE)) {
                        if (InventoryCanTakeItem(G_lastDrawTargetItem)) {
                            ClientRequestTake(G_lastDrawTargetItem, TRUE);
                        }
                    }
                }

                /* Messages are off, update screen buttons */
                ButtonKeyControl (event,scankey);  //JDA
            }
            break;


            case KEYBOARD_EVENT_RELEASE:
            if (scankey == KEY_SCAN_CODE_LEFT_CTRL ||
                scankey == KEY_SCAN_CODE_RIGHT_CTRL)
            {
                /* reset held ctrl key */
                InventoryResetUse();
            }
            if (ClientIsDead())  {
                if (scankey == KEY_SCAN_CODE_SPACE)
                    ClientRevive() ;
            }
            if (G_msgOn == FALSE && (!BannerFormIsOpen(BANNER_FORM_NOTES)))
            {
                if (scankey == KeyMap(KEYMAP_TALK))
                {
                    if (ClientIsGod())  {
                        /* If in god mode, use the cheat line */
                        G_msgPos = 0 ;
                        G_message[G_msgPos] = '\0' ;
                        G_msgOn = TRUE ;

                        /* Don't do this with the com window open. */
                        if (ComwinIsOpen())
                            BannerCloseForm() ;
                    } else {
                        /* Otherwise, use the communicate form. */
                        if (!ComwinIsOpen())  {
                            BannerOpenForm(BANNER_FORM_COMMUNICATE);
                        }
                        TxtboxFirstBox();
                        TxtboxNextBox();
                    }
                }
            }
            break ;


            default:
            break;
        }
    }


/**************************************************************************/
/* These are the keystrokes that always apply as long as the banner is up */
/**************************************************************************/


    {
        /* God mode stuff */
        if (ClientIsGod())
        {
            switch (event)
            {
                case KEYBOARD_EVENT_HELD:
                break;

                case KEYBOARD_EVENT_BUFFERED:
                break;

                case KEYBOARD_EVENT_PRESS:
                    /* Pressing ALT-J in god mode gives the player 80 bolts of each type */
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                        if (scankey == KEY_SCAN_CODE_J)
                        {
                            G_activeStats->Bolts[0] = 80 ;
                            G_activeStats->Bolts[1] = 80 ;
                            G_activeStats->Bolts[2] = 80 ;
                            G_activeStats->Bolts[3] = 80 ;
                            G_activeStats->Bolts[4] = 80 ;
                            G_activeStats->Bolts[5] = 80 ;
                            G_activeStats->Bolts[6] = 80 ;
                        }
                    }

                if (G_msgOn==FALSE) {
#ifndef NDEBUG
                    /* Pressing 'M' in Release version shows memory allocated, max, and free */
                    if (scankey == KEY_SCAN_CODE_M)
                    {
                        sprintf(buffer, "%ld alloc'd, %ld max, %ld free",
                            MemGetAllocated(),
                            MemGetMaxAllocated(),
                            FreeMemory()) ;
                        MessageAdd(buffer) ;
                    }
#endif
                    /* Pressing ALT-E gives the player 10,000 XP */
                    if (scankey == KEY_SCAN_CODE_E) {
                         if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE) {
                            StatsChangePlayerExperience (10000);
                         }
                    }

#ifndef NDEBUG
                    /* Pressing 'P' purges the memory of all discardable blocks */
                    if (scankey == KEY_SCAN_CODE_P)
                    {
                        MessageAdd("Purging free memory") ;
                        puts("Purge") ;
                        if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                            /* ALT-P not only purges but produces a report */
                            MemDumpDiscarded() ;
                        }
                        MemFlushDiscardable() ;
                        sprintf(buffer, "%ld alloc'd, %ld max, %ld free",
                            MemGetAllocated(),
                            MemGetMaxAllocated(),
                            FreeMemory()) ;
                        MessageAdd(buffer) ;
                        fflush(stdout) ;
                    }
#endif
                    /* Check to see if ALT-R is hit.  This */
                    /* outputs the resource file */
                    if (scankey == KEY_SCAN_CODE_R)
                    {
                         if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)
                         {
                               PicturesDump() ;
                               MemDumpDiscarded() ;
                               MessageAdd("Pics resource dumped") ;
                         }
                    }
                    /* Check to see if ALT-H is hit.  This turns on */
                    /* Heap checking in the debugger. */
                    if (scankey == KEY_SCAN_CODE_H)
                    {
                         if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)
                         {
                               DebugHeapOn() ;
                               MessageAdd("Heap checking turned ON") ;
                         }
                    }
                    /* Check to see if ALT-O is hit.  This turns off */
                    /* Heap checking in the debugger. */
                    if (scankey == KEY_SCAN_CODE_O)
                    {
                         if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)
                         {
                               DebugHeapOff() ;
                               MessageAdd("Heap checking turned OFF") ;
                         }
                    }
                }
                break;

                case KEYBOARD_EVENT_RELEASE:
                break;

                default:
                break;
            }
        }

        /*******************************/
        /* normal (non god mode) Stuff */
        /*******************************/
        switch (event) {
            case KEYBOARD_EVENT_HELD:
                if (!MouseIsRelativeMode()) {
                    /* use item in inventory */
                    if (scankey == KeyMap(KEYMAP_USE)) {
                        if (InventoryCanUseItemInReadyHand())
                            InventoryUseItemInReadyHand(temp);
                    }
                }
                break;

            case KEYBOARD_EVENT_BUFFERED:
                break;

            case KEYBOARD_EVENT_PRESS:
                if (KeyboardGetScanCode(KEY_SCAN_CODE_F1)==TRUE) {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_SHIFT)==TRUE) {
                        /* enter god mode */
    #ifdef COMPILE_OPTION_ALLOW_GOD_MODE
    #ifndef NDEBUG
                        StatsToggleGodMode();
    #endif
    #endif
                    } else {
                        if (BannerFormIsOpen(BANNER_FORM_CONTROL)) {
                            BannerCloseForm();
                        } else {
                            /* open help page */
                            BannerOpenForm(BANNER_FORM_CONTROL);
                        }
                    }
                }

                if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE) {
                    if (scankey==KEY_SCAN_CODE_J) {
                        if (ClientGetCurrentPlace()!=20005) {
                            for (i=0;i<23;i++) {
                                if (StatsPlayerHasNotePage(i)) {
                                    removedNotes=TRUE;
                                    StatsRemovePlayerNotePage (i);
                                }
                            }
                            for (i=30;i<34;i++) {
                                if (StatsPlayerHasNotePage(i)) {
                                    removedNotes=TRUE;
                                    StatsRemovePlayerNotePage (i);
                                }
                            }
                        }
                        if (removedNotes==TRUE) {
                            MessageAdd ("^007 Journal help pages removed");
                        }
                    }
                }


                if (G_msgOn==FALSE) {
                    /* flag to backspace spells */
                    if (KeyMapGetScan(KEYMAP_ABORT_SPELL)==TRUE)
                        SpellsBackspace(NULL);
                }

                if (G_msgOn==FALSE) {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE) {
                        if (KeyMapGetScan(KEYMAP_GAMMA_CORRECT) == TRUE) {
                            if (wasGamma == FALSE) {
                                wasGamma = TRUE;
                                MessagePrintf("Gamma level %d",
                                    ColorGammaAdjust()) ;
                            }
                        } else {
                            wasGamma = FALSE;
                        }
                    }
                }

                /* Check for canned sayings on the function keys */
                for (i=KEYMAP_CANNED_MESSAGE_1; i<=KEYMAP_CANNED_MESSAGE_12; i++)  {
                    if (KeyMapGetScan(i) == TRUE)  {
                        ComwinSayNth(i-KEYMAP_CANNED_MESSAGE_1) ;
                    }
                }

                break;

            case KEYBOARD_EVENT_RELEASE:
                /* update screen buttons on the keyboard events */
                ButtonKeyControl (event,scankey);  //JDA

                break;

            default:
                break;
        }
    }
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientCreateGlobalAreaSound
 *-------------------------------------------------------------------------*/
/**
 *  ClientCreateGlobalAreaSound causes an area sound to be created that
 *  all players hear.
 *
 *  @param x -- X center for area sound
 *  @param y -- Y center for area sound
 *  @param radius -- range of sound from center
 *  @param soundID -- sound number
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientCreateGlobalAreaSound(
           T_sword16 x,
           T_sword16 y,
           T_word16 radius,
           T_word16 soundID)
{
    DebugRoutine("ClientCreateGlobalAreaSound") ;

    if ((G_logoutAttempted == FALSE) &&
        (G_clientIsActive == TRUE))  {
        ClientSyncSendActionAreaSound(
            soundID,
            radius,
            FALSE) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGotoPlace
 *-------------------------------------------------------------------------*/
/**
 *  ClientGotoPlace tells the client to initiate a request to go to
 *  a different location.
 *
 *  @param locationNumber -- location number
 *  @param startLocation -- Location within map
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientGotoPlace(T_word32 locationNumber, T_word16 startLocation)
{
    T_packetShort packet ;
    T_gotoPlacePacket *p_packet ;
    E_Boolean isFake ;

    DebugRoutine("ClientGotoPlace") ;

    /* Reset any previous requests to go elsewhere.  Either it is */
    /* a bad place or we are going to go to it real soon. */
    ClientSetNextPlace(0xFFFF, 0xFF) ;

    ClientResetPauseGame() ;

//printf("ClientGotoPlace by %s: %ld %d %d\n", DebugGetCallerName(), locationNumber, startLocation, G_adventureNumber) ;
    if (locationNumber != G_currentPlace)  {
        if ((locationNumber >= 10000) ||
            (locationNumber == 0) ||
            (MapExist(locationNumber)))  {
            if (ClientIsInView())  {
                isFake = PlayerInFakeMode() ;
                if (!isFake)
                    PlayerSetFakeMode() ;
                if (PlayerGetObject()->inWorld == TRUE)
                    ObjectRemove(PlayerGetObject()) ;
                if (!isFake)
                    PlayerSetRealMode() ;
            }
            if (ClientGetConnectionType() == CLIENT_CONNECTION_TYPE_SINGLE)  {
                /* If zero, get out of playing the game. */
                if (locationNumber == 0)  {
                    /* Leave this place */
                    ClientForceGotoPlace(locationNumber, startLocation) ;
                    SMCPlayGameSetFlag(
                        SMCPLAY_GAME_FLAG_LEAVE_PLACE,
                        TRUE) ;
                } else  {
                    ClientForceGotoPlace(locationNumber, startLocation) ;
//printf("Starting player at %d\n", G_loginId) ;   fflush(stdout) ;
                    ClientStartPlayer(9000+G_loginId, G_loginId) ;
                }
            } else {
                /* Get a quick pointer. */
                p_packet = (T_gotoPlacePacket *)packet.data ;

                p_packet->command = PACKET_COMMANDCSC_GOTO_PLACE ;
                p_packet->placeNumber = locationNumber ;
                p_packet->startLocation = startLocation ;

                CmdQSendShortPacket(&packet, 200, 0, NULL) ;

                /* Stop bothering with this level. */
                G_clientIsActive = FALSE ;

                /* If zero, get out of playing the game. */
                if (locationNumber == 0)  {
//puts("bye ... here") ; fflush(stdout) ;
                    /* Leave this place */
                    SMCPlayGameSetFlag(
                        SMCPLAY_GAME_FLAG_LEAVE_PLACE,
                        TRUE) ;
                }
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGotoForm
 *-------------------------------------------------------------------------*/
/**
 *  ClientGotoForm starts up a form that is either hardcoded or
 *  communications based.
 *
 *  @param formNumber -- Number of form to load.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientGotoForm(T_word32 formNumber)
{
    T_packetShort packet ;
    T_gotoSucceededPacket *p_succeed ;

    DebugRoutine("ClientGotoForm") ;

//printf("Going to form %ld\n", formNumber) ;
    if (formNumber >= 20000)  {
        /* This is a hardcoded form. */
        ClientSetMode(CLIENT_MODE_HARD_CODED_FORM) ;

        p_succeed = (T_gotoSucceededPacket *)packet.data;
        p_succeed->command = PACKET_COMMANDCS_GOTO_SUCCEEDED ;
        p_succeed->placeNumber = formNumber;

        CmdQSendShortPacket(
            &packet,
            600,
            0,
            NULL /* IGotoSucceededForIntro */) ;

        HardFormStart(formNumber) ;
    } else {
        G_clientMode = CLIENT_MODE_SCRIPT_FORM ;

        /* Standard scripting form. */
        ScriptFormStart(formNumber) ;

        p_succeed = (T_gotoSucceededPacket *)packet.data;
        p_succeed->command = PACKET_COMMANDCS_GOTO_SUCCEEDED ;
        p_succeed->placeNumber = formNumber;

        CmdQSendShortPacket(
            &packet,
            600,
            0,
            NULL /* IGotoSucceededForIntro */) ;
    }

    G_clientIsActive = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientOverlayDone
 *-------------------------------------------------------------------------*/
/**
 *  ClientOverlayDone is a callback for OverlayAnimate.  It is called
 *  when an attack is done or when a flag is encounter within the anim.
 *
 *  @param animnum -- Number of animation
 *  @param flag -- Flag found, else 0 if end of anim.
 *
 *<!-----------------------------------------------------------------------*/
static T_void ClientOverlayDone (T_word16 animnum, T_byte8 flag)
{

#define ANIMATION_OVERLAY_FIST           0
#define ANIMATION_OVERLAY_SWORD          1
#define ANIMATION_OVERLAY_AXE            2
#define ANIMATION_OVERLAY_MAGIC          3
#define ANIMATION_OVERLAY_XBOW           4
#define ANIMATION_OVERLAY_MACE           5
#define ANIMATION_OVERLAY_DAGGER         6
#define ANIMATION_OVERLAY_STAFF          7
#define ANIMATION_OVERLAY_WAND           8
#define ANIMATION_OVERLAY_NULL           9

    T_sword32 frontX, frontY ;

    DebugRoutine ("ClientOverlayDone");

    if (flag == 0)
    {
        G_attackComplete = TRUE;
    } else if (flag&1) {
        /* Look at the position ahead and try to hit anything */
        /* that is there. */
        ObjectGetForwardPosition(
            PlayerGetObject(),
            PLAYER_ATTACK_DISTANCE,
            &frontX,
            &frontY) ;

        ObjectsDoToAllAtXYZRadius (
            (T_sword16)(frontX>>16),
            (T_sword16)(frontY>>16),
            (T_sword16)(PlayerGetZ16() + PLAYER_ATTACK_HEIGHT),
            PLAYER_ATTACK_RADIUS,
            IClientGainExperienceIfHit,
            StatsGetPlayerAttackDamage()>>2) ;

        /* Force the attack to be cut short */
        PlayerSetStance(STANCE_STAND) ;
    }

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientThrowObject
 *-------------------------------------------------------------------------*/
/**
 *  ClientThrowObject is called to throw an object at a given speed
 *  and angle.
 *
 *  @param p_obj -- Object to throw.
 *  @param throwspeed -- Speed to throw object
 *  @param angle -- Angle to throw along.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientThrowObject(
           T_3dObject *p_obj,
           T_sword16 throwspeed,
           T_word16 angle)
{
    DebugRoutine("ClientThrowObject") ;

    ClientSyncSendActionThrowItem(
        ObjectGetType(p_obj),
        angle,
        throwspeed,
        ObjectGetAccData(p_obj)) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IClientUpdateAndDrawFramesPerSecond
 *-------------------------------------------------------------------------*/
/**
 *  IClientUpdateAndDrawFramesPerSecond is a routine that calculates
 *  how many frames per second the engine is getting based on the speed
 *  of the calls to this routine.  Also, it draws the rate on the screen.
 *
 *  @param left -- Left edge of window with 3d view
 *  @param bottom -- Bottom edge of window with 3d view
 *
 *<!-----------------------------------------------------------------------*/
#ifndef COMPILE_OPTION_FRAME_SPEED_OFF
static T_void IClientUpdateAndDrawFramesPerSecond(
                  T_word16 left,
                  T_word16 bottom)
{
    /* Frame rate information. */
    static T_word16 fps = 0 ;
    static T_word16 frames = 0 ;
    static T_word32 nextfps = 0 ;
    static T_word16 syncPerSec = 0 ;
    T_word32 time ;
    char buffer[20] ;
extern T_word16 G_numSoundsPlaying;
    /* Update the frames per second. */
    time = TickerGet() ;
    if (nextfps <= time)  {
        nextfps = time+((T_word16)TICKS_PER_SECOND) ;
        fps = frames ;
        frames = 0 ;
        syncPerSec = G_syncCount ;
        G_syncCount = 0  ;
    }
    frames++ ;

    if (G_fpsOn)  {
        itoa(fps, buffer, 10) ;
        GrDrawRectangle(left+1, bottom-12, left+30, bottom-1, COLOR_YELLOW) ;
        GrDrawRectangle(left+2, bottom-11, left+29, bottom-2, COLOR_BLACK) ;
        GrSetCursorPosition(left+4, bottom-10) ;
        if ((TickerGet()&31)>15)  {
            GrDrawShadowedText(buffer, COLOR_YELLOW, COLOR_BLACK) ;
//            itoa(syncPerSec, buffer, 10) ;
itoa(G_numSoundsPlaying, buffer, 10) ;
            GrDrawShadowedText("/", COLOR_YELLOW, COLOR_BLACK) ;
            GrDrawShadowedText(buffer, COLOR_YELLOW, COLOR_BLACK) ;
        }
    }
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ClientAttack
 *-------------------------------------------------------------------------*/
/**
 *  ClientAttack puts the client in the motions of doing an attack.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientAttack(T_void)
{
    G_attackComplete = FALSE ;

//    ClientSendAttackPacket() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientIsAttemptingLogout
 *-------------------------------------------------------------------------*/
/**
 *  ClientIsAttemptingLogout returns true if the client is logging out
 *  of the system.
 *
 *  @return TRUE = attempting logout
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ClientIsAttemptingLogout(T_void)
{
    return G_logoutAttempted ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientIsActive
 *-------------------------------------------------------------------------*/
/**
 *  Simple put, "Am I logged into the server?"
 *
 *  @return FALSE = no, TRUE = yes
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ClientIsActive(T_void)
{
    E_Boolean isActive ;

    DebugRoutine("ClientIsActive") ;

    isActive = G_clientIsActive ;

    DebugEnd() ;

    return isActive ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetActive
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetActive declares the client to now be active.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetActive(T_void)
{
    G_clientIsActive = TRUE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetInactive
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetInactive declares the client to not be active.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetInactive(T_void)
{
    G_clientIsActive = FALSE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientIsInit
 *-------------------------------------------------------------------------*/
/**
 *  ClientIsActive returns TRUE if the client is initialized.
 *
 *  @return TRUE = client is initialized
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ClientIsInit(T_void)
{
    return G_clientInit ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetMode
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetMode returns the mode that the client is in.
 *
 *  @return mode client is in.
 *
 *<!-----------------------------------------------------------------------*/
E_clientMode ClientGetMode(T_void)
{
    return G_clientMode ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetMode
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetMode declares the mode that the client is in.
 *
 *  @param newMode -- mode client is in.
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetMode(E_clientMode newMode)
{
//printf("G_clientMode = %d\n", newMode) ;  fflush(stdout) ;
    G_clientMode = newMode ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetLoginId
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetLoginId retrieves the 'login ID' of the client.
 *
 *  @return login ID (0xFFFF if not logged in)
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ClientGetLoginId(T_void)
{
   return G_loginId;
}


/*-------------------------------------------------------------------------*
 * Routine:  ClientSetLoginId
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetLoginId stores    the 'login ID' of the client.
 *
 *  @param loginId -- login ID
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetLoginId(T_word16 loginId)
{
    G_loginId = loginId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetCurrentPlace
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetCurrentPlace returns the number of the current place.
 *
 *  @return Number of current place
 *
 *<!-----------------------------------------------------------------------*/
T_word32 ClientGetCurrentPlace(T_void)
{
    return G_currentPlace ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetCurrentPlace
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetCurrentPlace declares where the current place is.
 *
 *  @param newPlace -- Number of current place
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetCurrentPlace(T_word32 newPlace)
{
    G_currentPlace = newPlace ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetCurrentStartLocation
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetCurrentStartLocation returns what position within a place
 *  client is at.
 *
 *  @return Number of current start location
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ClientGetCurrentStartLocation(T_void)
{
    return G_currentStartLocation ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetCurrentStartLocation
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetCurrentStartLocation returns what position within a place
 *  client is at.
 *
 *  @param newStartLocation -- Number of current start location
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetCurrentStartLocation(T_word16 newStartLocation)
{
    G_currentStartLocation = newStartLocation ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetCurrentServerID
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetCurrentServerID returns the id of the current server.
 *
 *  @return Server ID
 *
 *<!-----------------------------------------------------------------------*/
T_word32 ClientGetCurrentServerID(T_void)
{
    return G_currentServerID ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetCurrentServerID
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetCurrentServerID sets    the id of the current server.
 *
 *  @param newServerID -- Server ID
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetCurrentServerID(T_word32 newServerID)
{
    G_currentServerID = newServerID ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetServerEnterStatus
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetServerEnterStatus sets what state in enter the server we
 *  are in.
 *
 *  @param status -- New status
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetServerEnterStatus(E_requestEnterStatus status)
{
    G_enterServerStatus = status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientGetServerEnterStatus
 *-------------------------------------------------------------------------*/
/**
 *  ClientGetServerEnterStatus gets what state in enter the server we
 *  are in.
 *
 *  @return status
 *
 *<!-----------------------------------------------------------------------*/
E_requestEnterStatus ClientGetServerEnterStatus(T_void)
{
    return G_enterServerStatus ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetSaveCharStatus
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetSaveCharStatus sets what status the last attempt to save
 *  a character is in.
 *
 *  @return Current save char status
 *
 *<!-----------------------------------------------------------------------*/
E_clientSaveCharStatus ClientGetSaveCharStatus(T_void)
{
    return G_clientSaveCharStatus ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ClientSetSaveCharStatus
 *-------------------------------------------------------------------------*/
/**
 *  ClientSetSaveCharStatus sets what status the last attempt to save
 *  a character is in.
 *
 *  @param status -- New save char status
 *
 *<!-----------------------------------------------------------------------*/
T_void ClientSetSaveCharStatus(E_clientSaveCharStatus status)
{
    G_clientSaveCharStatus = status ;
}

/* LES: 03/08/96 */
E_checkPasswordStatus ClientGetCheckPasswordStatus(T_void)
{
    return G_checkPasswordStatus ;
}

/* LES: 03/08/96 */
T_void ClientSetCheckPasswordStatus(E_checkPasswordStatus status)
{
    G_checkPasswordStatus = status ;
}

/* LES: 03/08/96 */
E_changePasswordStatus ClientGetChangePasswordStatus(T_void)
{
    return G_changePasswordStatus ;
}

/* LES: 03/08/96 */
T_void ClientSetChangePasswordStatus(E_changePasswordStatus status)
{
    G_changePasswordStatus = status ;
}

/* LES: 07/11/96 */
static T_void IClientAttemptOpeningForwardWall(T_void)
{
    E_wallActivation type ;
    T_word16 wallData ;
    T_word16 required ;

    DebugRoutine("IClientAttemptOpeningForwardWall") ;

    MapGetForwardWallActivationType(
        PlayerGetObject(),
        &type,
        &wallData) ;

    switch (type)  {
        case WALL_ACTIVATION_NONE:
            /* No action -- no wall found. */
            break ;
        case WALL_ACTIVATION_DOOR:
            /* Found a door. */
            DebugCheck(DoorIsAtSector(wallData) == TRUE) ;
            /* Is the door locked? */
            if (DoorIsLock(wallData))  {
                /* Check if the door needs a required item. */
                required = DoorGetRequiredItem(wallData) ;
                if (required != 0)  {
                    /* Door requires an item, so we need to check */
                    /* if the player has that item. */
                    if (InventoryHasItem(INVENTORY_PLAYER, required) != 0)  {
                        /* Player does have the item in question. */
                        /* Destroy it from the inventory and tell */
                        /* everyone to open the door. */
                        InventoryDestroySpecificItem(
                            INVENTORY_PLAYER,
                            required,
                            1) ;

                        SoundDing() ;
                        MessageAdd("Door took item.") ;

                        ClientSyncSendActionActivateForward() ;

                        /* Don't forget to unlock the door and make it */
                        /* no longer require an item. */
                        DoorSetRequiredItem(wallData, 0) ;
                        DoorUnlock(wallData) ;
                    } else {
                        /* Player does not have the item necessary. */
                        SoundDing() ;
                        MessageAdd("Door requires key to be opened.") ;
                    }
                } else {
                    /* Since the door is locked and doesn't require */
                    /* an item, we can't open the door -- so do */
                    /* nothing. */
                    SoundDing() ;
                    MessageAdd("Door is locked.") ;
                }
            } else {
                /* Door is not locked.  We are free to open it. */
                ClientSyncSendActionActivateForward() ;
            }
            break ;
        case WALL_ACTIVATION_SCRIPT:
            /* Found a script to activate. */
            /* Activate the forward wall. */
            if (wallData < 200)
                ClientSyncSendActionActivateForward() ;
            else
                ActivitiesRun(wallData) ;
            break ;
    }

    DebugEnd() ;
}

/* LES: 07/23/96 */
E_clientConnectionType ClientGetConnectionType(T_void)
{
    DebugCheck(G_clientConnectionType < CLIENT_CONNECTION_TYPE_UNKNOWN) ;
    return G_clientConnectionType ;
}

/* LES: 07/23/96 */
T_void ClientSetConnectionType(E_clientConnectionType type)
{
    DebugRoutine("ClientSetConnectionType") ;
    DebugCheck(type < CLIENT_CONNECTION_TYPE_UNKNOWN) ;

    G_clientConnectionType = type ;

    DebugEnd() ;
}

/* LES: 07/23/96 */
T_void ClientStartPlayer(T_word16 objectId, T_word16 loginId)
{
    T_3dObject *p_obj;
    T_sword16 xPos ;
    T_sword16 yPos ;
    T_word16 angle ;
    T_word16 loc ;
    extern E_Boolean G_serverActive ;
    T_word16 bodyPartLoc ;

    DebugRoutine("ClientStartPlayer") ;
    DebugCheck (ClientIsInit() == TRUE);
//printf("ClientStartPlayer called by %s\n", DebugGetCallerName()) ;

//printf("\n---------------------------- PLACE_START --------------------\n\n") ;
//puts("ClientReceivePlaceStartPacket");
    if ((ClientIsAttemptingLogout() == FALSE) && (ClientIsActive() == FALSE))  {
        /* Record the login id for future requests. */
        ClientSetLoginId(loginId) ;

//printf("Client mode: %d\n", ClientGetMode()) ; fflush(stdout) ;
        if (ClientGetMode() == CLIENT_MODE_GAME)  {
//puts("Client mode game start") ;  fflush(stdout) ;
            /* Be sure to save the character here. */
            StatsSaveCharacter(StatsGetActive()) ;
            MessageClear() ;
            MessageAdd("^037Character saved!") ;

            ClientSyncReset() ;
            ClientSetClientSyncStatus("") ;  /* SYNC OK */

            /** Do I share a machine with the server? **/
            p_obj = ObjectCreate() ;
            DebugCheck(p_obj != NULL) ;
            ObjectSetServerId (p_obj, objectId) ;

            ObjectSetType (p_obj, 510);  /** Player type. **/

            /** Make the object invisible, so I don't have my view blocked **/
            /** by myself. **/
            ObjectMakeInvisible(p_obj) ;

            /** Initialize the movement of the player. **/
            PlayerInit(p_obj) ;

            loc = ClientGetCurrentStartLocation() ;
            MapGetStartLocation(
//                loc,
                loginId,  /* Start location depends on login id */
                &xPos,
                &yPos,
                &angle) ;
            PlayerTeleportAlways(xPos, yPos, angle) ;
            //PlayerGetAngle()) ;

            /* Set up the player's location and all. */
            PlayerUpdatePosInfo() ;
            PlayerSetCameraView() ;
            G_serverActive = TRUE ;

            /** Initialize again to ensure the correct location. **/
            ObjectSetAngle(p_obj, angle) ;
//            PlayerInit(p_obj) ;
            PlayerFakeOverwriteCurrent() ;
            PlayerUpdatePosInfo() ;

            ObjectAdd(p_obj) ;

            /* Reset the list of names between players. */
            PeopleHereResetPlayerIDs() ;

            /* Send out what we look like to other players. */
            for (bodyPartLoc=0; bodyPartLoc < MAX_BODY_PARTS; bodyPartLoc++)  {
                ClientSyncSendActionChangeSelf(
                    bodyPartLoc,
                    PlayerGetBodyPart(bodyPartLoc)) ;
            }

            /* Send out our character name. */
            ClientSyncSendIdSelf(StatsGetName()) ;
        } else if (ClientGetMode() == CLIENT_MODE_HARD_CODED_FORM)  {
//            HardFormStart(ClientGetCurrentPlace()) ;
            /* Player completed the adventure, reset the adventure. */
            if (G_adventureNumber != 0)  {
                StatsUpdatePastPlace(G_adventureNumber, 0) ;
            }
        }

        /* Note that we are now logged in. */
        ClientSetActive() ;
    } else {
    }

    DebugEnd() ;
}

/* LES: 07/23/96 */
/* Make the client goto the given place now. */
T_void ClientForceGotoPlace(
           T_word16 placeNumber,
           T_word16 startLocation)
{
    T_word16 timeToUpdate ;
    T_word16 previousLocation;
    E_Boolean goToTown = FALSE ;

    DebugRoutine("ClientForceGotoPlace") ;

    /* Reset count of esc's */
    G_escCount = 0 ;

    MouseRelativeModeOff();

//printf("ClientGotoPlace by %s: %d %d\n", DebugGetCallerName(), placeNumber, startLocation) ;
//printf("Going to place %ld\n", placeNumber) ;  fflush(stdout) ;
    if (placeNumber != ClientGetCurrentPlace())  {
//printf("2) Going to place %ld\n", placeNumber) ;  fflush(stdout) ;
//printf("Client mode: %d\n", ClientGetMode()) ;
        /* Stop bothering with this level. */
        ClientSetInactive() ;

        if (ClientGetMode() == CLIENT_MODE_SCRIPT_FORM)
            /* Stop processing the form and destroy it. */
            ScriptFormEnd() ;

        if (ClientGetMode() == CLIENT_MODE_HARD_CODED_FORM)  {
            HardFormEnd() ;
        }

        /* Get rid of the current map. */
//        CmdQClearAllPorts() ;
        /* Allow packets going out the port to get a chance to go. */
        timeToUpdate = TickerGet() + 15 ;
        do {
            CmdQUpdateAllSends() ;
        } while (TickerGet() <= timeToUpdate) ;

        CmdQSetActivePortNum(0) ;
//        CommClearPort() ;

        previousLocation = ClientGetCurrentPlace() ;
        ClientSetCurrentPlace(placeNumber) ;
        ClientSetCurrentStartLocation(startLocation) ;

        /* Unload the map. */
        MapUnload() ;

        /* OK, we are nowhere now.  No known mode. */
        ClientSetMode(CLIENT_MODE_UNKNOWN) ;

        if (placeNumber == 0)  {
            // End mouse relative mode if still active
            MouseRelativeModeOff();

            SMCPlayGameSetFlag(SMCPLAY_GAME_FLAG_END_GAME, TRUE) ;
        } else {
            /* Get rid of the UI for the bottom of the screen (if any). */
            /* If above 10000, must be a ui dedicated form. */
            /* Otherwise, it is a map number. */
//printf("Received force goto place %ld\n", placeNumber) ;  fflush(stdout) ;

            if (placeNumber >= 10000)  {
                /* We must of completed the adventure! (An abort command */
                /* resets G_adventureNumber to zero). */
                if (G_adventureNumber != 0)  {
                    StatsUpdatePastPlace(
                        G_adventureNumber,
                        0) ;
                    TownUISetAdventureCompleted() ;
                }

                ClientGotoForm(placeNumber) ;

                /* We are no longer in an adventure if in form places */
                G_adventureNumber = 0 ;
            } else {
                /* Record that we have made it to here in the adventure. */
                if (G_adventureNumber != 0)  {
                    StatsUpdatePastPlace(
                        G_adventureNumber,
                        G_currentPlace) ;

                    if (previousLocation < 10000)
                        goToTown = TownUICompletedMapLevel(previousLocation) ;
                }

                /* We have by passed to go to the town. */
                if (goToTown == TRUE)  {
                    G_adventureNumber = 0 ;

                    MouseRelativeModeOff();

                    ClientSetCurrentPlace(HARD_FORM_TOWN+HARDFORM_GOTO_PLACE_OFFSET) ;
                    ClientSetCurrentStartLocation(0) ;
                    ClientGotoForm(HARD_FORM_TOWN+HARDFORM_GOTO_PLACE_OFFSET) ;
                } else {
                    ClientSetMode(CLIENT_MODE_GAME) ;

                    /* Load up the new map. */
                    ViewSetPalette(VIEW_PALETTE_STANDARD) ;

                    /* Resync the random numbers */
                    RandomReset() ;

                    /* Reset the sync timer. */
                    SyncTimeSet(1) ;

                    MapLoad(placeNumber) ;

                    ClientSendGotoSucceeded(
                         ClientGetCurrentPlace(),
                         ClientGetCurrentStartLocation()) ;
                }
            }
        }

        /* Tell everyone about my new location. */
        ClientSendPlayerIDSelf() ;
    }

    DebugEnd() ;
}

/* LES: 07/23/96 */
/* Declare next place to go to. */
T_void ClientSetNextPlace(T_word16 nextPlace, T_byte8 nextStart)
{
    E_Boolean isFake ;

    DebugRoutine("ClientSetNextPlace") ;

    if ((ClientGetCurrentPlace() != 0) && (ClientGetCurrentPlace() < 10000))  {
        ClientSetDeadState(FALSE) ;
        isFake = PlayerInFakeMode() ;
        if (isFake)
            PlayerSetRealMode() ;
        PlayerSetStance(STANCE_STAND) ;
        if (isFake)
            PlayerSetFakeMode() ;
    }

    ColorResetFilt() ;
    G_nextPlace = nextPlace ;
    G_nextStart = nextStart ;

    DebugEnd() ;
}


/* returns TRUE if the banner is up */
E_Boolean ClientIsInGame (T_void)
{
    E_Boolean isInGame=FALSE;
    DebugRoutine ("ClientIsInGame");

    if (ClientGetMode()==CLIENT_MODE_GAME ||
        ClientGetMode()==CLIENT_MODE_HARD_CODED_FORM)
    {
        isInGame=TRUE;
    }

    DebugEnd();

    return (isInGame);
}

/* returns TRUE if the 3dView is up */
E_Boolean ClientIsInView (T_void)
{
    E_Boolean isInView=FALSE;
    DebugRoutine ("ClientIsInView");

    if (ClientGetMode()==CLIENT_MODE_GAME &&
        HardFormIsOpen()==FALSE) isInView=TRUE;

    DebugEnd();
    return (isInView);
}

/* returns TRUE if the player is a god */
E_Boolean ClientIsGod (T_void)
{
    E_Boolean isGod=FALSE;
    DebugRoutine ("ClientIsGod");

#ifdef COMPILE_OPTION_ALLOW_GOD_MODE
    if (EffectPlayerEffectIsActive(PLAYER_EFFECT_GOD_MODE)==TRUE)
    {
        isGod=TRUE;
    }
#endif

    DebugEnd();
    return (isGod);
}

E_Boolean ClientDropInventoryItem(T_word16 numItems, T_word16 itemType, T_word16 accData)
{
    E_Boolean doDestroy = FALSE ;
    T_word16 i ;

    DebugRoutine("ClientDropInventoryItem") ;

//printf("ClientDropItem: %d of %d\n", numItems, itemType) ;

    /* don't drop runes */
    if (itemType < 300 || itemType > 317)
    {
        /* No more than 3. */
        if (numItems > 3)
            numItems = 3 ;

        /* If in acid, don't drop items, just destroy them. */
        if (!(MapGetSectorType(PlayerGetAreaSector()) & SECTOR_TYPE_ACID))  {
            /* Drop the item(s) */
            for (i=0; i<numItems; i++)
                ClientSyncSendActionDropAt(itemType, PlayerGetX16(), PlayerGetY16(), accData) ;
        }

        doDestroy = TRUE ;
    }
    DebugEnd() ;

    return doDestroy ;
}

T_void ClientDied(T_void)
{
    T_word16 i ;
    T_sword16 num ;
    static T_word16 boltToQuiver[EQUIP_TOTAL_BOLT_TYPES] = {
        144, /* NORMAL */
        146, /* POISON */
        147, /* PIERCING */
        145, /* FIRE */
        149, /* ELECTRICITY */
        150, /* MANA DRAIN */
        148  /* ACID */
    } ;
    static T_word16 coinTypeToCoin[EQUIP_TOTAL_COIN_TYPES] = {
        101, /* COPPER */
        103, /* SILVER */
        105, /* GOLD */
        107, /* PLATINUM */
    } ;
    E_Boolean isFake ;

    DebugRoutine("ClientDied") ;
    /* This player just died!  We need to take appropriate actions. */

    if (ClientIsInView())  {
        /* Drop all his equipment here. */
        InventoryGoThroughAll(ClientDropInventoryItem) ;

        /* Drop bolts (if got any) */
        for (i=0; i<EQUIP_TOTAL_BOLT_TYPES; i++)  {
            num = StatsGetPlayerBolts(i) ;
            if (num)  {
                if (!(MapGetSectorType(PlayerGetAreaSector()) & SECTOR_TYPE_ACID))  {
                    /* Drop quiver full. */
                    ClientSyncSendActionDropAt(boltToQuiver[i], PlayerGetX16(), PlayerGetY16(), num) ;
                }
                StatsAddBolt(i, -num) ;
            }
        }

        /* Drop coins (if got any) */
        for (i=0; i<EQUIP_TOTAL_COIN_TYPES; i++)  {
            num = StatsGetPlayerCoins(i) ;
            if (num)  {
                /* Only drop 5 pieces */
                if (num >= 5)  {
                    if (!(MapGetSectorType(PlayerGetAreaSector()) & SECTOR_TYPE_ACID))  {
                        ClientSyncSendActionDropAt(coinTypeToCoin[i], PlayerGetX16(), PlayerGetY16(), 0) ;
                    }
                }
                StatsAddCoin(i, -num);
            }
        }

        /* Set weight allowance to zero */
        StatsSetPlayerLoad(0);

        /* Recalculate movemement speed */
        StatsCalcPlayerMovementSpeed();

        /* update banner */
        if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
        {
            InventoryDrawInventoryWindow(INVENTORY_PLAYER);
        }
        /* Get rid of any remaining spell that was being attempted. */
        SpellsBackspace(NULL) ;

        /* Since we lost all the armor, update the parts */
        InventoryUpdatePlayerEquipmentBodyParts() ;

        /* Declare the player dead */
        ClientSetDeadState(TRUE) ;

        /* Make sure nothing is in the mouse hand. */
        ControlSetDefaultPointer (CONTROL_MOUSE_POINTER_DEFAULT);
        MouseUseDefaultBitmap() ;

        MessageAdd("^005You are dead.") ;
        MessageAdd("^003Press SPACE to revive your character.") ;

        isFake = PlayerInFakeMode() ;
        if (isFake)
            PlayerSetRealMode() ;
        PlayerSetStance(STANCE_DIE) ;
        if (isFake)
            PlayerSetFakeMode() ;
        ColorSetFilt(0, -63, -63);
    } else {
        /* No matter how bad it is, you don't die while not in the */
        /* game. */
        StatsSetPlayerHealth(100);
    }

    DebugEnd() ;
}

T_void ClientRevive(T_void)
{
    T_sword16 xPos, yPos ;
    T_word16 angle ;
    T_word16 i ;
    E_Boolean isFake ;

    DebugRoutine("ClientRevive") ;

    if ((isFake = PlayerInFakeMode()) == TRUE)
        PlayerSetRealMode() ;

    /* Try to going to one of the four start locations (which one */
    /* is free) */
    for (i=0; i<4; i++)  {
        MapGetStartLocation(
            ClientGetLoginId(),  /* Start location depends on login id */
            &xPos,
            &yPos,
            &angle) ;

        PlayerTeleport(xPos, yPos, angle) ;
        /* Stop if we made it. */
        if ((PlayerGetX16() == xPos) &&
            (PlayerGetY16() == yPos))
            break ;
    }
    if (isFake)
        PlayerSetFakeMode() ;

    ColorResetFilt() ;
    ClientSetDeadState(FALSE) ;
    isFake = PlayerInFakeMode() ;
    if (isFake)
        PlayerSetRealMode() ;
    PlayerSetStance(STANCE_STAND) ;
    if (isFake)
        PlayerSetFakeMode() ;

    MessageAdd("^005You are revived from the dead.") ;

    DebugEnd() ;
}

E_Boolean ClientIsDead()
{
    return G_deadState ;
}

T_void ClientResetPauseGame(T_void)
{
    G_pauseGame = FALSE ;
}

T_void ClientTogglePause(T_void)
{
    if (G_pauseGame)  {
        G_pauseGame = FALSE ;
    } else {
        G_pauseGame = TRUE ;
    }
}

E_Boolean ClientIsPaused(T_void)
{
    if (!ClientIsInGame())
        return FALSE ;
    if ((SyncTimeGet() <= 1) && (!HardFormIsOpen()))
        return TRUE ;
    return G_pauseGame ;
}

static T_3dObject *G_stealFromObject ;

static E_Boolean IClientFindSteal(
                      T_3dObject *p_obj,
                      T_word32 dummy)
{
    E_Boolean stopSearch = FALSE ;

    DebugRoutine("IClientFindSteal") ;

    if ((ObjectIsPiecewise(p_obj)) && (!ObjectIsBodyPart(p_obj)))  {
        G_stealFromObject = p_obj ;
        stopSearch = TRUE ;
    }
    if (ObjectIsCreature(p_obj))  {
        if (!CreatureIsMissile(p_obj))  {
            G_stealFromObject = p_obj ;
            stopSearch = TRUE ;
        }
    }

    DebugEnd() ;

    return stopSearch ;
}

T_void ClientDoSteal(T_void)
{
    T_sword32 frontX, frontY ;
    T_sword16 stealPickSkill ;
    static T_word32 lastTime = 0 ;
    E_wallActivation type ;
    T_word16 doorSector ;
    T_word16 doorLock ;

    DebugRoutine("ClientDoSteal") ;

    /* Stealing/Picking skill for mercenary and magician is half chance. */
    stealPickSkill = StatsGetPlayerStealth() ;
    if ((StatsGetPlayerClassType() == CLASS_MERCENARY) ||
        (StatsGetPlayerClassType() == CLASS_MAGICIAN))  {
        stealPickSkill >>= 1 ;
    }

    /* Look at the position ahead and try to hit anything */
    /* that is there. */
    ObjectGetForwardPosition(
        PlayerGetObject(),
        50,
        &frontX,
        &frontY) ;

    G_stealFromObject = NULL ;

    ObjectsDoToAllAtXYZRadius (
        (T_sword16)(frontX>>16),
        (T_sword16)(frontY>>16),
        (T_sword16)(PlayerGetZ16() + 30),
        15,
        IClientFindSteal,
        0) ;

    if (G_stealFromObject == NULL)  {
        MapGetForwardWallActivationType(
            PlayerGetObject(),
            &type,
            &doorSector) ;
        if (type == WALL_ACTIVATION_DOOR)  {
            DebugCheck(DoorIsAtSector(doorSector)) ;
            if (lastTime > TickerGet())
                lastTime = TickerGet() ;
            doorLock = DoorGetLockValue(doorSector) ;
            if (doorLock == 0)  {
                /* Door isn't locked.  Don't bother. */
                MessageAdd("Door is already unlocked.") ;
            } else {
                if ((lastTime + TIME_BETWEEN_PICK_LOCKS) > TickerGet())  {
                    MessageAdd("Cannot pick lock right now.  Wait a moment.") ;
                } else {
                    lastTime = TickerGet() ;
                    if ((rand() % 100) < stealPickSkill)  {
                        /* Is the door absolutely locked? */
                        if (doorLock == DOOR_ABSOLUTE_LOCK)  {
                            /* Give a 1% chance to open door without key. */
                            if ((rand() % 100) == 0)  {
                                MessageAdd("Picking lock ... ") ;
                                ClientSyncSendActionPickLock(
                                    doorSector,
                                    ObjectGetServerId(PlayerGetObject())-100) ;
                            } else {
                                MessageAdd("^005Door appears to have a superior lock") ;
                            }
                        } else {
                            MessageAdd("Picking lock ... ") ;
                            ClientSyncSendActionPickLock(
                                doorSector,
                                ObjectGetServerId(PlayerGetObject())-100) ;
                        }
                    } else {
                        MessageAdd("^005You failed to pick the lock") ;
                    }
                }
            }
        } else {
            MessageAdd("^006No door to unlock or person to steal.") ;
        }
    } else {
        /* Don't allow stealing when item is in mouse hand. */
        if (InventoryObjectIsInMouseHand() == TRUE)  {
            SoundDing() ;
            MessageAdd("^026Cannot steal with item in mouse hand!") ;
        } else {
            if (lastTime > TickerGet())
                lastTime = TickerGet() ;
            if ((lastTime+TIME_BETWEEN_STEALS) > TickerGet())  {
                MessageAdd("Cannot steal right now.  Wait a moment.") ;
            } else {
                lastTime = TickerGet() ;
                if ((rand() % 100) < stealPickSkill)  {
                    /* Successful. */
                    /* Send out a request for this steal to occur. */
                    ClientSyncSendActionSteal(
                        ObjectGetServerId(G_stealFromObject),
                        ObjectGetServerId(PlayerGetObject())-100,
                        FALSE) ;
                } else {
                    /* See if we failed badly. */
                    if (((rand() % 100)*4) < (120-stealPickSkill))  {
                        /* Failed badly. */
                        ClientSyncSendActionSteal(
                            ObjectGetServerId(G_stealFromObject),
                            ObjectGetServerId(PlayerGetObject())-100,
                            TRUE) ;
                        SoundDing() ;
                        MessageAdd("^005Oh oh!  You failed to steal and were noticed!") ;
                    } else {
                        MessageAdd("^005You failed to steal") ;
                    }
                }
            }
        }
    }

    DebugEnd() ;
}

T_void ClientSetAdventureNumber(T_word16 adventureNum)
{
    G_adventureNumber = adventureNum ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CLIENT.C
 *-------------------------------------------------------------------------*/
