/****************************************************************************/
/*    FILE:  CLIENT.C                                                       */
/****************************************************************************/

#include "standard.h"
#include <ctype.h>
#include <malloc.h>

/*
#include "ack3d.h"
#include "ackeng.h"
#include "kit.h"
*/

#define OBJECT_TYPE_MASK 0xFFF
#define CLIENT_NUM_TALK_MODES 3

//#define isFiring (*((E_Boolean *)0xA0001))
#define FIREBALL_NUM_FRAMES  4
#define PLAYER_HEALTH_MAX    100
#define HEALING_AMOUNT_APPLE 200
#define PLAYER_ATTACK_SPEED  70
#define MAX_CANNED_SAYINGS   10
#define CANNED_SAYINGS_FIRST_SOUND 23

static T_word16 fps = 0 ;
static T_word16 frames = 0 ;
static T_word32 nextfps = 0 ;
static E_Boolean moved = TRUE ;

/* Information about other players. */
static T_word16 G_numOtherPlayers = 0 ;
static T_sword16 G_playerObjects[4] ;
static T_sword16 G_playerFires[4] ;
static T_sword16 G_playerIds[4] ;
static T_word16 G_lastPlayerMapPos[4] ;
static T_word16 G_playerAngles[4] ;
static T_word32 G_lastMoveTime[4] ;
static T_word32 G_lastMoveTime2[4] ;
static T_word16 G_walkPhase[4] ;
static T_word16 G_moveAngle = 0 ;

/* This client's Fireball information. */
static T_sword16 G_fireballObject = -1 ;
static T_word16 G_fireballAngle ;
static T_word32 G_fireballLastMoveTime ;
static T_sword32 G_fireballHeight ;
static T_sword32 G_fireballDeltaHeight ;

static E_Boolean isFiring = FALSE ;
static T_sword16 playerHealth = 100 ;
static E_Boolean G_logoutAttempted = FALSE ;

static E_Boolean G_clientIsLogin = FALSE ;
T_byte8 G_loginId = 0xFF ;
static E_Boolean G_clientInit = FALSE ;
    T_word16 oldAngle = 0 ;
    T_word16 newAngle = 0 ;
    T_sword16 moveAmount = 0 ;
    T_word16 revAngle ;
    T_sword16 delta ;
    E_Boolean shift ;
    T_word32 time ;
    T_byte8 buffer[80] ;
    T_sword16 x, y ;
    T_sword16 newx, newy ;
    T_word16 i ;
    T_word16 action ;
	T_word32 lastAnim = 0 ;
    T_word16 onAlready = 0xFFFF ;
static T_word16 attackDir = 0 ;
static T_word32 attackTime = 0 ;
static T_word16 G_weapon = 0 ;
static T_word16 volume = 128 ;
static E_Boolean G_attackComplete = TRUE ;
static E_Boolean G_clientMoveToComplete = TRUE ;
static T_byte8 *G_fireballPics[FIREBALL_NUM_FRAMES] ;
static T_resource G_fireballRes[FIREBALL_NUM_FRAMES] ;
static T_byte8 *G_glowers[4] ;
static T_resource G_glowersRes[4] ;
static T_resource G_mouthRes[4] ;
static T_byte8 *P_mouthPics[4] ;

#define MAX_PENDING_OBJECTS 10
static T_word16 G_pendingObjects[MAX_PENDING_OBJECTS] ;
static T_word16 G_pendingCount ;

static T_byte8 G_message[MAX_MESSAGE_LEN+2] ;
static T_byte8 G_msgPos = 0 ;
static E_Boolean G_msgOn = FALSE ;

#define MAX_WEAPONS 3
#define WEAPON_FIST  0
#define WEAPON_SWORD 1
#define WEAPON_XBOW  2
static E_Boolean G_haveWeapons[MAX_WEAPONS] ;
static E_Boolean G_clientInitFirst = FALSE ;

extern  short   Resolution;

/* Talk and menu options. */
static E_Boolean G_talkBlock = FALSE ;
static T_resource R_talkBlock = RESOURCE_BAD ;
static T_word16 G_talkMode = 0 ;
static E_Boolean G_cannedBlock = FALSE ;

T_resource R_overlay = RESOURCE_BAD ;
T_byte8 *P_overlay = NULL ;
T_sword16 G_overlayOffset = 0 ;
T_word16 G_currentOverlay = 0xFFFF ;

/* !!! Change this later !!! */
T_byte8 *p_impPictures[53] ;
T_resource r_impPictures[53] ;
T_byte8 *p_shadowPictures[53] ;
T_resource r_shadowPictures[53] ;
T_byte8 *p_stefanPictures[53] ;
T_resource r_stefanPictures[53] ;

/* Internal prototypes: */
T_void ClientHandleOverlay(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom) ;

T_void ClientUpdateHealth(T_void) ;

T_word16 ClientGetDelta(T_void) ;

T_word16 QuickATan(T_sword16 dx, T_sword16 dy) ;

T_void ClientSetOverlay(T_word16 over) ;

T_word16 ClientGetPlayerObjectNum(T_word16 playerId) ;
T_void ClientFire(T_word16 x, T_word16 y, T_word16 angle) ;
T_void ClientRequestAction(T_word16 action) ;
T_word16 ClientCheckOtherPlayer(T_word16 x, T_word16 y) ;
T_void ClientRequestMoveTo(
           T_sword16 x,
           T_sword16 y,
           T_word16 angle,
           T_sword16 height) ;
T_void ClientFaceOtherPlayers(T_void) ;
T_void ClientCheckStances(T_void) ;
T_void ClientShootFireball(T_void) ;
T_void ClientUpdateFireball(T_void) ;
T_void ClientAnimateFireball(T_void) ;
T_void ClientPrepareFireballPictures(T_void) ;
T_void ClientReleaseFireballPictures(T_void) ;
T_void IClientAnimateDead(T_word32 data) ;
T_void ClientSendMessage(T_void) ;
T_void ClientHandleKeyboard(E_keyboardEvent event, T_byte8 scankey) ;



T_void ClientAnimate(T_void)
{
    T_word16 delta ;
    T_byte8 *ptr ;
    T_word16 i ;

    DebugRoutine("ClientAnimate") ;

    /* Warning!!! All hard coded junk! */

    /* Get how far we are. */
    delta = (TickerGet() & 0x1FF)>>1 ;
    for (i=978; i<=993; i++)
        ViewSetWallBitmapTextureXY(i, 0, delta&0x7F, 0) ;

    delta = (TickerGet() & 127) << 9 ;
    delta = (((T_sword32)90)*MathSineLookup(delta))>>16 ;

    ViewChangeSectorLighting(13, 160+delta) ;
    ViewChangeSectorLighting(14, 90+delta) ;
    ViewChangeSectorLighting(15, 90+delta) ;
    ViewChangeSectorLighting(254, 90+delta) ;
    ViewChangeSectorLighting(257, 90+delta) ;

    if (ViewGetObjectType(14) == 13)  {
        delta = (TickerGet() & 63) ;
        if (delta >= 32)
            delta = 63-delta ;
        delta >>= 3 ;
        ViewChangeObjectPictureDirectly(14, P_mouthPics[delta]) ;
    }

    delta = ((TickerGet() >> 3) & 7) ;
    if (delta > 3)
        delta = 7-delta ;

    for (i=52; i<=63; i++)
        ViewChangeObjectPictureDirectly(i, G_glowers[delta]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientPrepareFireballPictures                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientPrepareFireballPictures loads in all the different pictures     */
/*  for the fireball animation.                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientPrepareFireballPictures(T_void)
{
    T_word16 i ;
    static T_byte8 *pictures[FIREBALL_NUM_FRAMES] = {
        "FIREBALL1.PIC",
        "FIREBALL2.PIC",
        "FIREBALL3.PIC",
        "FIREBALL4.PIC"
    } ;

    DebugRoutine("ClientPrepareFireballPictures") ;

    for (i=0; i<FIREBALL_NUM_FRAMES; i++)
        G_fireballPics[i] = PictureLock(pictures[i], &G_fireballRes[i]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReleaseFireballPictures                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReleaseFireballPictures removes from memory all the fireball    */
/*  animation frames.                                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureUnlock                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReleaseFireballPictures(T_void)
{
    T_word16 i ;

    DebugRoutine("ClientReleaseFireballPictures") ;

    for (i=0; i<FIREBALL_NUM_FRAMES; i++)
        PictureUnlock(G_fireballRes[i]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientAnimateFireball                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientAnimateFireball cycles through the fireball images based        */
/*  on the time.                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientAnimateFireball(T_void)
{
    static T_word16 lastPicNum = 0xFFFF ;
    T_word16 newPicNum ;

    DebugRoutine("ClientAnimateFireball") ;

    /* Compute a picture based on what the timer is. */
    newPicNum = (TickerGet() & 0x18) >> 3 ;

    /* See if this is different than last time. */
    if (lastPicNum != newPicNum)  {
        /* Record the change. */
        lastPicNum = newPicNum ;

        /* Change the picture. */
        if (G_fireballObject != -1)
            ViewChangeObjectPictureDirectly(G_fireballObject, G_fireballPics[newPicNum]) ;
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientShootFireball                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientShootFireball checks to see if there is a fireball already      */
/*  in play, and if there is not, creates a new fireball, giving it         */
/*  the player's facing direction.                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    TickerGet                                                             */
/*    ViewCreateObject                                                      */
/*    ViewGetPOVLocation                                                    */
/*    ViewActiveObject                                                      */
/*    ViewDeclareMoveableObject                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientShootFireball(T_void)
{
    T_sword16 x, y ;
    T_sword16 height ;
    T_sword16 heightTarget ;
    T_word16 target ;
    T_sword16 tx, ty ;
    T_sword32 distance ;

    DebugRoutine("ClientShootFireball") ;

    /* Is another attack complete? */
    if (G_attackComplete == TRUE)  {
        /* Yes, it is.  How about is there a fireball object out there? */
        if (G_fireballObject == -1)  {
            /* Ok, no fireball currently.  Let's make one. */
            G_fireballObject = ViewCreateObject() ;

            /* Get the player's current location as a starting point. */
            ViewGetPOVLocation(&x, &y) ;

            /* Declare the object to be moveable and give it its */
            /* starting location. */
            ViewDeclareMoveableObject(
                G_fireballObject,
                x,
                y,
                0) ;
            ViewMoveObject(G_fireballObject, x, y) ;
            height = 2+ViewGetPlayerHeight() ;

            G_fireballHeight = ((T_sword32)height) << 16 ;

            /* See if there is a target in view. */
            target = ViewGetMiddleTarget() ;

            /* Check if there is a target. */
            if (target == VIEW_TARGET_NONE)  {
                /* There is none, just shoot straight. */
                G_fireballDeltaHeight = 0 ;
            } else  {
                /* Find where the target is located. */
                ViewGetObjectXY(target, &tx, &ty) ;

                /* How far is it? */
                distance = CalculateDistance(x, y, tx, ty) ;

                /* How high is the target? */
                heightTarget = ViewGetObjectMiddleHeight(target) ;

                /* Calculate the steps necessary to draw a straight */
                /* line to the target. */
                G_fireballDeltaHeight =
                    (((T_sword32)(heightTarget - height))<<16) / distance ;

                /* Don't allow more than 45 degrees up. */
                if (G_fireballDeltaHeight >= 0x10000)
                    G_fireballDeltaHeight = 0x10000 ;

                /* Don't allow more than 45 degrees down. */
                if (G_fireballDeltaHeight <= -0x10000)
                    G_fireballDeltaHeight = -0x10000 ;
            }

            height = G_fireballHeight >> 16 ;
            ViewSetObjectHeight(G_fireballObject, height) ;

            /* Declare the angle we are going to be shooting at. */
            G_fireballAngle = G_moveAngle ;

            /* Give the fireball a picture. */
            ViewChangeObjectPictureDirectly(G_fireballObject, G_fireballPics[0]) ;

            /* Make it active. */
            ViewObjectActivate(G_fireballObject) ;

            /* Note that we are in the middle of an attack. */
            G_attackComplete = FALSE ;

            /* Note when the fireball was started. */
            G_fireballLastMoveTime = TickerGet() ;

            /* Play shooting fireball sound. */
            SoundPlayByNumber(18) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientUpdateFireball                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientUpdateFireball moves the clients fireball (if there is one).    */
/*  It will also send out packets to the server and will identify when      */
/*  the fireball has hit a wall (or other object).                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    TickerGet                                                             */
/*    ViewStepObject                                                        */
/*    ViewGetObjectXY                                                       */
/*    CmdQSendShortPacket                                                   */
/*    ViewDeactivateObject                                                  */
/*    ViewRemoveObject                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientUpdateFireball(T_void)
{
    T_packetShort packet ;
    T_fireballMovePacket *p_fireballMove ;
    T_fireballStopPacket *p_fireballStop ;
    T_word32 delta ;
    T_word32 time ;
    T_sword16 status ;
    T_sword16 x, y ;
    T_sword16 height ;

    DebugRoutine("ClientUpdateFireball") ;

    /* Do we have a fireball to move? */
    if (G_fireballObject != -1)  {
        /* Yes.  Let's move it. */
        /* Get a delta time from when it last moved. */
        time = TickerGet() ;
        delta = G_fireballLastMoveTime - time ;
        G_fireballLastMoveTime = time ;

        delta <<= 4 ;
        if (delta > 63)
            delta = 63 ;

        if (delta == 0)
            delta = 10 ;

        if (delta != 0)  {
            /* Move the fireball at that delta (twice running speed) */
            status = ViewStepObject(
                         G_fireballObject,
                         G_fireballAngle,
                         (T_sword16)delta,
                         35) ;
            G_fireballHeight += G_fireballDeltaHeight * delta ;
            height = G_fireballHeight >> 16 ;
            ViewSetObjectHeight(G_fireballObject, height) ;

            /* Send out its new location. */
            p_fireballMove = (T_fireballMovePacket *)packet.data ;
            p_fireballMove->player = G_loginId ;
            ViewGetObjectXY(G_fireballObject, &x, &y) ;

//            if (ServerIsSynced() == TRUE)  {
                p_fireballMove->x = x ;
                p_fireballMove->y = y ;
                p_fireballMove->command = PACKET_COMMAND_FIREBALL_MOVE ;
                p_fireballMove->height = height ;
                CmdQSendShortPacket(&packet, 70, 0, NULL) ;
//            }

            /* Did it hit something? */
            if (status != 0)  {
                /* OK, must stop the fireball now.  Send a stop packet. */
                p_fireballStop = (T_fireballStopPacket *)packet.data ;
                p_fireballStop->command = PACKET_COMMAND_FIREBALL_STOP ;
                p_fireballStop->player = G_loginId ;
                CmdQSendShortPacket(&packet, 70, 0, NULL) ;

                /* Destroy the fireball so that it can no longer move. */
                ViewObjectDeactivate(G_fireballObject) ;
                ViewRemoveObject(G_fireballObject) ;
                G_fireballObject = -1 ;
                G_attackComplete = TRUE ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientChangeWeaponAck                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientChangeWeaponAck confirms that the last change weapon package    */
/*  was sent.  When it is, the picture of the current weapon changes.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetOverlay                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientChangeWeaponAck(
           T_word32 extraData,
           T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientChangeWeaponAck") ;

    G_weapon = extraData ;

    /* Change the overlay immediately. */
	ClientSetOverlay(G_weapon<<1) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ClientChangeWeapon                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientChangeWeapon changes the current weapon (and sends out a        */
/*  packet to tell everyone else).                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void ClientChangeWeapon(T_word16 weapon)
{
    T_packetShort packet ;
    T_changeWeaponPacket *p_change ;

    DebugRoutine("ClientChangeWeapon") ;

    /* Get a quick pointer. */
    p_change = (T_changeWeaponPacket *)packet.data ;

    /* Set up the packet for the new weapon. */
    p_change->command = PACKET_COMMAND_PLAYER_CHANGE_WEAPON ;
    p_change->weapon = weapon ;
    p_change->player = G_loginId ;

    /* Send out the packet. */
    CmdQSendShortPacket(&packet, 600, weapon, ClientChangeWeaponAck) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientAttackSent                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientAttackSent confirms that the attack was done.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientAttackSent(T_word32 extraData, T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientAttackSent") ;

    G_attackComplete = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendAttackPacket                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendAttackPacket sends a request to attack.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendAttackPacket(T_void)
{
    T_packetShort packet ;
    T_attackPacket *p_attack ;

    DebugRoutine("ClientSendAttackPacket") ;

    /* Get a quick pointer. */
    p_attack = (T_attackPacket *)packet.data ;

    p_attack->command = PACKET_COMMAND_PLAYER_ATTACK ;
    p_attack->player = G_loginId ;
    p_attack->weapon = G_weapon ;

    G_attackComplete = FALSE ;

    CmdQSendShortPacket(&packet, 600, G_weapon, ClientAttackSent) ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSetOverlay                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSetOverlay declares what picture to put on top of the rest      */
/*  of the screen.  Typically this is a hand, fist, gun, or other weapon.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 over               -- Number of overlay to use               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLock                                                           */
/*    PictureUnlock                                                         */
/*    SoundPlayByNumber                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSetOverlay(T_word16 over)
{
    static T_byte8 *overlays[] = {
	    "ARMover",
	    "ARM2over",
	    "DAGGERover",
	    "DAGGER2over",
	    "GUN1over",
	    "GUN2over",
	    "GUN3over",
	    "GUN4over",
	    "GUN5over",
	    "GUN6over",
	    "GUN7over",
	    "GUN8over",
	    "GUN9over"
    } ;

    DebugRoutine("ClientSetOverlay") ;

    if (over != G_currentOverlay)  {
	if (R_overlay != RESOURCE_BAD)
	    PictureUnlock(R_overlay) ;

	if (over == 1)
	    SoundPlayByNumber(9) ;
	if (over == 3)
	    SoundPlayByNumber(10) ;

	if (over < 13)  {
	    P_overlay = PictureLock(overlays[over], &R_overlay) ;
	    DebugCheck(P_overlay != NULL) ;
	} else {
            R_overlay = RESOURCE_BAD ;
        }
	G_currentOverlay = over ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientInit                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientInit starts up and cleans up any initial items needed by        */
/*  the client.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientInit(T_void)
{
    T_word16 i, j, k, kk ;
    static T_byte8 *torchNames[4] = {
        "GLOWER1.PIC",
        "GLOWER2.PIC",
        "GLOWER3.PIC",
        "GLOWER4.PIC"
    } ;
    static T_byte8 *mouthNames[4] = {
        "HEAD_1",
        "HEAD_2",
        "HEAD_3",
        "HEAD_4"
    } ;

    T_byte8 filename[20] ;

    DebugRoutine("ClientInit") ;
    DebugCheck(G_clientInit == FALSE) ;

//    if (G_clientInitFirst == FALSE)  {
        memset(G_playerObjects, 0xFF, sizeof(G_playerObjects)) ;
        memset(G_playerFires, 0xFF, sizeof(G_playerFires)) ;
        memset(G_lastMoveTime, 0, sizeof(G_lastMoveTime)) ;
        memset(G_lastMoveTime2, 0, sizeof(G_lastMoveTime2)) ;
        memset(G_walkPhase, 0, sizeof(G_walkPhase)) ;
        memset(G_playerIds, 0xFF, sizeof(G_playerIds)) ;
        G_numOtherPlayers = 0 ;
//    }

    ClientUpdateHealth() ;

    KeyboardDebounce() ;

    isFiring = FALSE ;

    if (G_clientInitFirst == FALSE)  {
        ColorInit(); /* Init color mapping stuff */
        ActivitiesRun(0) ;
    }

    StatsInit(); /* Init player statistics */
    ClientGetDelta() ;
    KeyboardBufferOff() ;
    GrDrawFrame(9, 9, 310, 151, 0) ;
    ViewSetOverlayHandler(ClientHandleOverlay) ;
    ClientSetOverlay(0) ;

    /* Prepare the people parts for the bitmaps. */
    PeopleBMInitialize() ;

    /* Load in the fireball pictures. */
    ClientPrepareFireballPictures() ;

    for (i=0; i<4; i++)
        G_glowers[i] = PictureLock(torchNames[i], &G_glowersRes[i]) ;


    /* Create a fireball object. */
///    G_fireballObjectNum = ViewCreateObject() ;
///    ViewDeclareStaticObject(G_fireballObjectNum, -10000, -10000, 0) ;

/* This is NOT the way I should be doing it !!! */
for (k=0; k<4; k++)  {
  kk = G_lowerCreatureSteps * (k/G_lowerCreatureSteps) ;
  for (i=0; i<8; i++)  {
    j = G_remapCreatureAngles[i] ;
    sprintf(filename, "IMP_%c%d.PIC", 'A'+kk, j+1) ;
    p_impPictures[i+(k<<3)] = PictureLock(filename, &r_impPictures[i+(k<<3)]) ;
    sprintf(filename, "SHAD_%c%d.PIC", 'A'+kk, j+1) ;
    p_shadowPictures[i+(k<<3)] = PictureLock(filename, &r_shadowPictures[i+(k<<3)]) ;
  }
}
/*
for (i=0; i<8; i++)  {
  j = G_remapAngles[i] ;
  sprintf(filename, "IMP_A%d.PIC", j+1) ;
  p_impPictures[i] = PictureLock(filename, &r_impPictures[i]) ;
}
for (i=0; i<8; i++)  {
  j = G_remapAngles[i] ;
  sprintf(filename, "IMP_B%d.PIC", j+1) ;
  p_impPictures[i+8] = PictureLock(filename, &r_impPictures[i+8]) ;
}
for (i=0; i<8; i++)  {
  j = G_remapAngles[i] ;
  sprintf(filename, "IMP_C%d.PIC", j+1) ;
  p_impPictures[i+16] = PictureLock(filename, &r_impPictures[i+16]) ;
}
for (i=0; i<8; i++)  {
  j = G_remapAngles[i] ;
  sprintf(filename, "IMP_D%d.PIC", j+1) ;
  p_impPictures[i+24] = PictureLock(filename, &r_impPictures[i+24]) ;
}
*/
for (i=0; i<8; i++)  {
  j = G_remapCreatureAngles[i] ;
  sprintf(filename, "IMP_E%d.PIC", j+1) ;
  p_impPictures[i+32] = PictureLock(filename, &r_impPictures[i+32]) ;
  sprintf(filename, "SHAD_E%d.PIC", j+1) ;
  p_shadowPictures[i+32] = PictureLock(filename, &r_shadowPictures[i+32]) ;
}
for (i=0; i<8; i++)  {
  j = G_remapCreatureAngles[i] ;
  sprintf(filename, "IMP_F%d.PIC", j+1) ;
  p_impPictures[i+40] = PictureLock(filename, &r_impPictures[i+40]) ;
  sprintf(filename, "SHAD_F%d.PIC", j+1) ;
  p_shadowPictures[i+40] = PictureLock(filename, &r_shadowPictures[i+40]) ;
}
p_impPictures[48] = PictureLock("IMP_DEAD.PIC", &r_impPictures[48]) ;
p_shadowPictures[48] = PictureLock("SHAD_DEA.PIC", &r_shadowPictures[48]) ;
for (i=0; i<4; i++)  {
  sprintf(filename, "IMP_DIE%d.PIC", i+1) ;
  p_impPictures[i+49] = PictureLock(filename, &r_impPictures[i+49]) ;
  sprintf(filename, "SHAD_DIE%d.PIC", i+1) ;
  p_shadowPictures[i+49] = PictureLock(filename, &r_shadowPictures[i+49]) ;
}

for (i=0; i<48; i++)  {
  j = i/6 ;
  j = G_remapCreatureAngles[j] ;
  j = G_lowerCreatureSteps*((i%6)/G_lowerCreatureSteps)+(j*6) ;
  sprintf(filename, "STEFAN%02d", j+1) ;
  p_stefanPictures[i] = PictureLock(filename, &r_stefanPictures[i]) ;
}
for (i=48; i<53; i++)  {
  sprintf(filename, "STEFAN%02d", i+1) ;
  p_stefanPictures[i] = PictureLock(filename, &r_stefanPictures[i]) ;
}


    for (i=0; i<4; i++)
        P_mouthPics[i] = PictureLock(mouthNames[i], &G_mouthRes[i]) ;

    if (G_clientInitFirst == FALSE)
        G_pendingCount = 0 ;

    KeyboardSetEventHandler(ClientHandleKeyboard) ;

    G_haveWeapons[WEAPON_FIST] = TRUE ;

    G_clientInitFirst = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientCheckScrolling                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientCheckScrolling checks to see if the player has pressed either   */
/*  the Page up or Page down keys to scroll the messages.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientCheckScrolling(T_void)
{
    static T_word32 nextAttempt = 0 ;
    T_word32 time ;
    T_word32 timeNext = 12 ;

    DebugRoutine("ClientCheckScrolling") ;

    time = TickerGet() ;

    if (time < nextAttempt)
        return ;

    /* If this routine is called several times (like a key being held) */
    /* repeat faster. */
    if ((time-nextAttempt) < 30)
        timeNext = 4 ;

    if (KeyboardGetScanCode(KEY_SCAN_CODE_PGUP)==TRUE)  {
        MessageScrollUp() ;
        /* Only allow the scrolling of about 4 items per second. */
        nextAttempt = time+timeNext ;
    }

    if (KeyboardGetScanCode(KEY_SCAN_CODE_PGDN)==TRUE)  {
        MessageScrollDown() ;
        /* Only allow the scrolling of about 4 items per second. */
        nextAttempt = time+timeNext ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientDrawOverlay                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientDrawOverlay draws the weapon on the overlay that goes on        */
/*  top of the 3D view.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientDrawOverlay(T_void)
{
    T_byte8 *p_screen ;
    T_byte8 *p_data ;
    T_word16 pos ;
    T_byte8 count ;
    T_word16 offset ;

    DebugRoutine("ClientDrawOverlay") ;
    DebugCheck(P_overlay != NULL) ;

    p_screen = (T_byte8 *)GrScreenGet() ;    /* Not a good thing to do! */
    p_screen += 16000-G_overlayOffset ;    /* Skip down 100 lines. */
    offset = 32560+G_overlayOffset ;

/*
if (G_currentOverlay>=4)
  p_screen += 110 ;
*/

    p_data = P_overlay ;
    while (*((T_word16 *)p_data) != 0xFFFF)  {
        pos = *((T_word16 *)p_data) ;
        if (pos >= offset) /* 48000 */
            break ;
        p_data+= 2 ;
        count = *(p_data++) ;
        memcpy(p_screen+pos, p_data, count) ;
        p_data += count ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientHandleOverlay                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientHandleOverlay is called when the 3D engine is done drawing      */
/*  the 3d view.  This routine draws everything that is to appear on top    */
/*  of the 3D view.                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left               -- left edge of view                      */
/*                                                                          */
/*    T_word16 top                -- top edge of view                       */
/*                                                                          */
/*    T_word16 right              -- right edge of view                     */
/*                                                                          */
/*    T_word16 bottom             -- bottom edge of view                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientHandleOverlay(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom)
{
    char buffer[10] ;
    T_bitmap *p_block ;
    T_resource res ;

    DebugRoutine("ClientHandleOverlay") ;
    DebugCheck(bottom < SCREEN_SIZE_Y) ;
    DebugCheck(right < SCREEN_SIZE_X) ;
    DebugCheck(top < bottom) ;
    DebugCheck(left < right) ;

    /* Draw the current weapon on the screen. */
    ClientDrawOverlay() ;

    /* Draw the frames per second we are getting. */
    itoa(fps, buffer, 10) ;
    GrSetCursorPosition(left+2, bottom-11) ;
    if ((TickerGet()&31)>15)
        GrDrawShadowedText(buffer, COLOR_YELLOW, COLOR_BLACK) ;

    /* Check if we are in message mode. */
    if (G_msgOn == TRUE)  {
        /* If so, we'll draw the message being entered now. */
        G_message[G_msgPos] = '_' ;
        G_message[G_msgPos+1] = '\0' ;

        GrSetCursorPosition(left+5, top+100) ;
        GrDrawShadowedText(G_message, COLOR_YELLOW, COLOR_BLACK) ;

        G_message[G_msgPos] = '\0' ;
    }

	SpellsDrawInEffectRunes();

	/* Draw the messages at the top. */
	MessageDraw(left+2, top+2, 10, COLOR_WHITE) ;

	/* See if we need to show the talk block on the screen. */
	if (G_talkBlock == TRUE)  {
		p_block = (T_bitmap *)PictureLockData("TALKBLOCK", &R_talkBlock) ;
		GrDrawBitmap(p_block,
			(SCREEN_SIZE_X - p_block->sizex)>>1,
			(SCREEN_SIZE_Y - p_block->sizey)>>2) ;
		PictureUnlock(R_talkBlock) ;
	}

	/* See if we need to show the canned sounds block on the screen. */
	if (G_cannedBlock == TRUE)  {
		p_block = (T_bitmap *)PictureLockData("CANNED", &res) ;
		GrDrawBitmap(p_block,
			(SCREEN_SIZE_X - p_block->sizex)>>1,
			(SCREEN_SIZE_Y - p_block->sizey)>>2) ;
		PictureUnlock(res) ;
	}

	/* update the 'spell in effect' icons */


    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientGetDelta                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientGetDelta returns the number of timer clicks (ticks) that have   */
/*  passed since this routine was last called.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Ticks since last call.                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

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

T_void ClientMain(T_void)
{
/*
MessageAdd("Lysle says, \"Are we ever going to get out of here?\"") ;
MessageAdd("Greg says, \"Yeah, eventually.  Let check something.\"") ;
MessageAdd("Eric says, \"You guys done yet?\"") ;
*/

///    KeyboardBufferOn() ;
}

/****************************************************************************/
/*  Routine:  ClientLogin                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientLogin requests to login into the server.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    It is assumed that the client has already attached to server and there*/
/*  is a data communications path open.                                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientLogin(T_void)
{
    T_packetShort packet ;

    DebugRoutine("ClientSendLoginPacket") ;
//putsflush("Sending login packet") ;

    G_logoutAttempted = FALSE ;

    CommSetActivePortN(0) ;
    CommClearPort() ;

    /* All I can do is ask for a login. */
    packet.data[0] = PACKET_COMMAND_LOGIN ;
    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientLogoffFinish                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientLogoffFinish declares the client no longer on.  This routine    */
/*  closes out the client and anything else that must be done.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientLogoffFinish(
           T_word32 extraData,
           T_packetEitherShortOrLong *p_packet)
{
    T_word16 i ;

    DebugRoutine("ClientLogoffFinish") ;

    /* Make sure we are logged on. */
    if (G_clientIsLogin == TRUE)  {
        /* We are no longer on. */
        G_clientIsLogin = FALSE ;

        /* No more overlay. */
        ClientSetOverlay(0xFFFF) ;

        /* No more people pictures. */
        PeopleBMFinish() ;

        /* Remove the fireball animation. */
        ClientReleaseFireballPictures() ;

        /* Note that we need to be re-inited when we re-login. */
        G_clientInit = FALSE ;

        for (i=0; i<4; i++)  {
            PictureUnlock(G_glowersRes[i]) ;
            G_glowersRes[i] = RESOURCE_BAD ;
        }

/* This is NOT the way I should be doing it !!! */
for (i=0; i<53; i++)  {
  PictureUnlock(r_impPictures[i]) ;
  PictureUnlock(r_shadowPictures[i]) ;
  PictureUnlock(r_stefanPictures[i]) ;
}

    for (i=0; i<4; i++)
        PictureUnlock(G_mouthRes[i]) ;

        for (i=0; i<4; i++)  {
            if (G_playerObjects[i] != -1)  {
                ViewRemoveObject(G_playerObjects[i]) ;
                G_playerObjects[i] = -1 ;
            }
            if (G_playerFires[i] != -1)  {
                ViewRemoveObject(G_playerFires[i]) ;
                G_playerFires[i] = -1 ;
            }
            G_playerIds[i] = -1 ;
        }

        KeyboardSetEventHandler(NULL) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientLogoff                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientLogoff tells the server that the client is leaving.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    It is assumed that the client has already attached to server and there*/
/*  is a data communications path open.                                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientLogoff(T_void)
{
    T_packetShort packet ;
    T_logoffPacket *p_packet ;
char *filename ;
T_word16 line ;

    DebugRoutine("ClientLogoff") ;

    if (G_logoutAttempted == FALSE)  {
//putsflush("Sending logoff packet") ;
        /* All I can do is ask for a logoff. */
        p_packet = (T_logoffPacket *)packet.data ;
        p_packet->command = PACKET_COMMAND_PLAYER_LOGOFF ;
        p_packet->player = G_loginId ;

        G_logoutAttempted = TRUE ;

        CmdQSendShortPacket(&packet, 140, 0, ClientLogoffFinish) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientIsLogin                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Simple put, "Am I logged into the server?"                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- FALSE = no, TRUE = yes                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ClientIsLogin(T_void)
{
    E_Boolean isLogin ;

    DebugRoutine("ClientIsLogin") ;

    isLogin = G_clientIsLogin ;

    DebugEnd() ;

    return G_clientIsLogin ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveLoginPacket                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveLoginPacket tells the client that the server has just    */
/*  allowed the client to login.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveLoginPacket(T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientReceiveLoginPacket") ;
//putsflush("Received Login packet") ;

    if (G_logoutAttempted == FALSE)  {
        /* Record the login id for future requests. */
        G_loginId = ((T_loginAnswerPacket *)(p_packet->data))->loginId ;

        /* Note that we are now logged in. */
        G_clientIsLogin = TRUE ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveFireballMovePacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveFireballMovePacket moves or creates a fireball shot      */
/*  by a player.                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- fireball move packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveFireballMovePacket(T_packetEitherShortOrLong *p_packet)
{
    T_fireballMovePacket *p_fireballMove ;
    T_word16 index ;
    T_word16 objNum ;
    T_sword32 dx, dy ;
    T_sword32 dist ;
    T_sword16 x, y ;

    DebugRoutine("ClientReceiveFireballMovePacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Start by getting a quick pointer to this data. */
        p_fireballMove = (T_fireballMovePacket *)p_packet->data ;

        /* Get the player's index who is using this. */
        index = ClientGetPlayerObjectNum(p_fireballMove->player) ;

        /* Make sure this is a good player. */
        if (index != 0xFFFF)  {
            /* Get the object number for this item. */
            objNum = G_playerFires[index] ;

            /* Is there already a fireball out there? */
            if (objNum == 0xFFFF)  {
                /* No, there is not one.  We need to create it. */
                objNum = G_playerFires[index] = ViewCreateObject() ;
                ViewDeclareMoveableObject(
                    objNum,
                    p_fireballMove->x,
                    p_fireballMove->y,
                    0) ;
                ViewChangeObjectPictureDirectly(objNum, G_fireballPics[0]) ;
            }

            /* Make sure the object is visible. */
            ViewObjectActivate(objNum) ;

            /* Move the object to the given location. */
            ViewMoveObject(objNum, p_fireballMove->x, p_fireballMove->y) ;
            ViewSetObjectHeight(objNum, p_fireballMove->height) ;

            /* How far is that ball to us? */
            ViewGetPOVLocation(&x, &y) ;
            dx = x - p_fireballMove->x ;
            dy = y - p_fireballMove->y ;

            /* See if point is touching us.  Do this by getting the distance. */
            dist = dx*dx + dy*dy ;

            /* See if that is a hit! */
            if (dist < 4000)  {
                /* Yes, a hit. */
                if (rand()&1)
                    SoundPlayByName("ImHit1") ;
                else
                    SoundPlayByName("ImHit2") ;
//                playerHealth -= 50 ;
                StatsHurtPlayer(500) ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveFireballStopPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveFireballStopPacket removes a fireball in play.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- fireball stop packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveFireballStopPacket(T_packetEitherShortOrLong *p_packet)
{
    T_fireballStopPacket *p_fireballStop ;
    T_word16 index ;
    T_word16 objNum ;

    DebugRoutine("ClientReceiveFireballStopPacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Start by getting a quick pointer to this data. */
        p_fireballStop = (T_fireballStopPacket *)p_packet->data ;

        /* Get the player's index who is using this. */
        index = ClientGetPlayerObjectNum(p_fireballStop->player) ;

        /* Make sure this is a good player. */
        if (index != 0xFFFF)  {
            /* Get the object number for this item. */
            objNum = G_playerFires[index] ;

            /* Is this a valid fireball object? */
            if (objNum != -1)  {
                /* Deactivate the object. */
                ViewObjectDeactivate(objNum) ;

                /* Destroy the object. */
                ViewRemoveObject(objNum) ;

                /* Note that we no longer have an object. */
                G_playerFires[index] = -1 ;
            }
        }
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerMovePacket                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerMovePacket takes in a movement request packet      */
/*  and either moves the player or one of the other players.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet containg move info.     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerMovePacket(T_packetEitherShortOrLong *p_packet)
{
    T_movePacketData *p_moveData ;
    T_word16 objNum ;
    E_stanceType stance ;

    DebugRoutine("ClientReceivePlayerMovePacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Get a quick pointer to the data in the packet. */
        p_moveData = (T_movePacketData *)p_packet->data ;

        /* See if this the client or some other player. */
        if (p_moveData->player == G_loginId)  {
            /* It is the client.  Jump to the told location. */
            ViewTeleport(
                p_moveData->x,
                p_moveData->y,
                p_moveData->angle) ;

            /* Note that we have moved (and when). */
            moved = TRUE ;

            /* Since we moved, check for doors. */
            ViewCheckDoor(p_moveData->angle) ;
        } else {
            /* Must be another player being moved. */

            /* Move his corresponding object. */
            /* Get the player's number from his id. */
            objNum = ClientGetPlayerObjectNum(p_moveData->player) ;

            if (objNum != 0xFFFF)  {
                /* See if we need to animate the walking. */
                if (G_lastMoveTime[objNum] < TickerGet())  {
                    /* Yes, we do.  Prepare next animate time. */
                    G_lastMoveTime[objNum] = TickerGet()+10 ;

                    /* Go to the next walk phase. */
                    G_walkPhase[objNum] = (G_walkPhase[objNum]+1)&3 ;

                    /* Convert phase to stance. */
                    switch(G_walkPhase[objNum])  {
                        case 0:
                            stance = STANCE_TYPE_STANDING ;
                            break ;
                        case 1:
                            stance = STANCE_TYPE_WALK_1 ;
                            break ;
                        case 2:
                            stance = STANCE_TYPE_STANDING ;
                            break ;
                        case 3:
                            stance = STANCE_TYPE_WALK_2 ;
                            break ;
                    }

                    PeopleBMSetStance(p_moveData->player, stance) ;
                }
                /* Store the new facing angle. */
                G_playerAngles[objNum] = p_moveData->angle ;

                /* Get the allocated object number. */
                objNum = G_playerObjects[objNum] ;
                if (ViewCheckObjectCollide(
                        objNum,
                        p_moveData->x,
                        p_moveData->y) == FALSE)  {
                    ViewMoveObject(
                        objNum,
                        p_moveData->x,
                        p_moveData->y) ;
                }
                ViewSetObjectHeight(objNum, p_moveData->height) ;
            }
        }
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerLogoffPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerLogoffPacket tells the client that one of the      */
/*  players have left this group.  Remove all the associated information.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- logoff packet                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientGetPlayerObjectNum                                              */
/*    memmove                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerLogoffPacket(T_packetEitherShortOrLong *p_packet)
{
    T_logoffPacket *p_logoff ;
    T_word16 index ;
    T_word16 len ;

    DebugRoutine("ClientReceivePlayerLogoffPacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_logoff = (T_logoffPacket *)p_packet->data ;
        index = ClientGetPlayerObjectNum(p_logoff->player) ;
        if (index != 0xFFFF)  {
            len = 3-index ;
            if (len > 0)  {
                /* Turn off the object. */
                ViewRemoveObject(G_playerObjects[index]) ;

                len *= sizeof(T_word16) ;
                /* Shrink up over the data. */
				memmove(G_playerObjects+index, G_playerObjects+index+1, len) ;
                memmove(G_playerFires+index, G_playerFires+index+1, len) ;
                memmove(G_playerIds+index, G_playerIds+index+1, len) ;
                memmove(G_lastPlayerMapPos+index, G_lastPlayerMapPos+index+1, len) ;
                memmove(G_playerAngles+index, G_playerAngles+index+1, len) ;
                memmove(G_walkPhase+index, G_walkPhase+index+1, len) ;

                len *= sizeof(T_word32)/sizeof(T_word16) ;
                memmove(G_lastMoveTime+index, G_lastMoveTime+index+1, len) ;
                memmove(G_lastMoveTime2+index, G_lastMoveTime2+index+1, len) ;

                G_numOtherPlayers-- ;
                G_playerIds[G_numOtherPlayers] = -1 ;
            }
            PeopleBMRemovePlayer(p_logoff->player) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerChangeWeaponPacket                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerChangeWeaponPacket reacts to the change weapon     */
/*  packet and changes the picture of the player with the weapon.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- change weapon packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientGetPlayerObjectNum                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerChangeWeaponPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_changeWeaponPacket *p_changeWeapon ;
    T_word16 index ;

    DebugRoutine("ClientReceivePlayerChangeWeaponPacket") ;

    if (G_logoutAttempted == FALSE)  {
		p_changeWeapon = (T_changeWeaponPacket *)p_packet->data ;

        /* Identify the person in our configuration. */
        index = ClientGetPlayerObjectNum(p_changeWeapon->player) ;

        /* Make sure it is a legal person. */
        if (index != 0xFFFF)  {
    /* !!! */
    if (p_changeWeapon->weapon >= 2)
      p_changeWeapon->weapon = 0 ;
            /* Change the weapon type for that person. */
            PeopleBMSetWeapon(G_playerIds[index], p_changeWeapon->weapon) ;

            /* Rebuild the image(s) */
            PeopleBMBuildStances() ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientGetPlayerObjectNum                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientGetPlayerObjectNum determines what object one of the other      */
/*  players are based on the given player ID number.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 playerId           -- Player ID to get object of             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- object index if sucessful, or 0xFFFF   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 ClientGetPlayerObjectNum(T_word16 playerId)
{
    T_word16 objNum ;

    DebugRoutine("ClientGetPlayerObjectNum") ;

    for (objNum=0; objNum < G_numOtherPlayers; objNum++)
        if (G_playerIds[objNum] == playerId)
            break ;

    if (objNum == G_numOtherPlayers)
        objNum = 0xFFFF ;

    DebugEnd() ;

    return objNum ;
}

/****************************************************************************/
/*  Routine:  ClientSentAction                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSentAction is called once the client has sent out to the server */
/*  an action packet.  This routine responds by doing the action.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 extraData          -- Action # to perform                    */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- action packet                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSentAction(
           T_word32 extraData,
           T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientSentAction") ;

    /* Perform the action. */
    ActivitiesRun(extraData /* action */) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientRequestAction                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRequestAction sends out a request for a certain activity to     */
/*  occur.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 action             -- Action #                               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientRequestAction(T_word16 action)
{
    T_packetShort packet ;
	T_actionPacket *p_action ;

    DebugRoutine("ClientRequestAction") ;

    if (action < 200)  {
        /* Get a quick pointer. */
        p_action = (T_actionPacket *)packet.data ;

        p_action->command = PACKET_COMMAND_PLAYER_ACTION ;
        p_action->action = action ;
        p_action->player = G_loginId ;

        CmdQSendShortPacket(&packet, 140, action, ClientSentAction) ;
    } else {
        ActivitiesRun(action) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientUpdate                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientUpdate does all the activities that a client needs to do for    */
/*  a single time slice.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientUpdate(T_void)
{
    static T_word16 lastSector = 0xFFFF ;
	static T_word16 lastbit = 0 ;
	static E_Boolean f_spaceIsHit = FALSE ;
	static T_word32 G_lastAttack = 0 ;
	static E_Boolean firstin=FALSE;
	T_word16 i;
	T_word16 sector, action ;
	T_word16 newWeapon ;

	DebugRoutine("ClientUpdate") ;

	if (firstin==FALSE)
	{
		firstin=TRUE;
		ButtonAdd ( 3,155,"AICON_1",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_7,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (20,155,"AICON_2",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_8,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (37,155,"AICON_3",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_9,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd ( 3,169,"AICON_4",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_4,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (20,169,"AICON_5",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_5,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (37,169,"AICON_6",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_6,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd ( 3,183,"AICON_7",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_1,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (20,183,"AICON_8",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_2,KEY_SCAN_CODE_ALT),NULL);
		ButtonAdd (37,183,"AICON_9",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_3,KEY_SCAN_CODE_ALT),NULL);

		ButtonAdd (266,155,"SICON_1",FALSE,KEY_SCAN_CODE_KEYPAD_7,SpellsAddRune);
		ButtonAdd (283,155,"SICON_2",FALSE,KEY_SCAN_CODE_KEYPAD_8,SpellsAddRune);
		ButtonAdd (300,155,"SICON_3",FALSE,KEY_SCAN_CODE_KEYPAD_9,SpellsAddRune);
		ButtonAdd (266,169,"SICON_4",FALSE,KEY_SCAN_CODE_KEYPAD_4,SpellsAddRune);
		ButtonAdd (283,169,"SICON_5",FALSE,KEY_SCAN_CODE_KEYPAD_5,SpellsAddRune);
		ButtonAdd (300,169,"SICON_6",FALSE,KEY_SCAN_CODE_KEYPAD_6,SpellsAddRune);
		ButtonAdd (266,183,"SICON_7",FALSE,KEY_SCAN_CODE_KEYPAD_1,SpellsAddRune);
		ButtonAdd (283,183,"SICON_8",FALSE,KEY_SCAN_CODE_KEYPAD_2,SpellsAddRune);
		ButtonAdd (300,183,"SICON_9",FALSE,KEY_SCAN_CODE_KEYPAD_3,SpellsAddRune);
		ButtonAdd (241,183,"SICON_B",FALSE,KEY_SCAN_CODE_KEYPAD_PERIOD,SpellsClearRunes);
		ButtonAdd (176,155,"SICON_C",FALSE,KEY_SCAN_CODE_KEYPAD_ENTER,SpellsCastSpell);

		MouseInitialize();
		SpellsInitSpells();
		MouseSetEventHandler (ClientCheckMouse);
		MouseShow();
	}

	AreaSoundCheck() ;
	if ((G_clientIsLogin == TRUE) && (G_logoutAttempted == FALSE))  {
		if (G_clientInit == FALSE)  {
			ClientInit() ;
			G_clientInit = TRUE ;
            G_lastAttack = 0 ;
        }

    //    while (KeyboardGetScanCode(KEY_SCAN_CODE_ESC)==FALSE)  {
            ClientAnimateFireball() ;
            ViewCheckFloorActivation() ;
            ScheduleUpdateEvents() ;
            KeyboardUpdateEvents() ;

            /* Update the frames per second. */
            time = TickerGet() ;
            if (nextfps <= time)  {
                nextfps = time+((T_word16)TICKS_PER_SECOND) ;
                fps = frames ;
                frames = 0 ;
            }

	        if (attackDir != 0)  {
	            attackDir = time-attackTime ;
	            if (attackDir == 0)
					attackDir++ ;

	            if (attackDir > 20)  {
		            G_overlayOffset = 0 ;
		            attackDir = 0 ;
		            ClientSetOverlay(0+(G_weapon<<1)) ;
	            } else {
		            if (G_weapon == /*2*/3)  {
						if (attackDir < 16)  {
			                if (attackDir < 3)
			                    SoundPlayByNumber(11) ;
			                ClientSetOverlay((G_weapon<<1)+(attackDir/2)) ;
						} else {
							ClientSetOverlay((G_weapon<<1)+8) ;
						}
					} else {
						if (attackDir >= 10)  {
							G_overlayOffset = 1280 * (20-attackDir) ;
						} else {
							G_overlayOffset = 1280 * attackDir ;
						}
						if ((attackDir > 5) && (attackDir < 15))  {
							ClientSetOverlay(1+(G_weapon<<1)) ;
						} else {
							ClientSetOverlay(0+(G_weapon<<1)) ;
						}
					}
				}
			}
			/* Get the XY location of the current location. */
			ViewGetPOVLocation(&x, &y) ;

			shift = FALSE ;

			if (((KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_SHIFT)==TRUE) ||
				 (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_SHIFT)==TRUE)) &&
				 (G_msgOn == FALSE) && (ViewIsAboveGround()==FALSE))  {
				shift = TRUE ;
//                *((char *)0xA0000) = 15 ;
			} else {
//                *((char *)0xA0000) = 0 ;
			}

			if ((KeyboardGetScanCode(KEY_SCAN_CODE_X)==TRUE) &&
				 (G_msgOn == FALSE))  {
				/* Do a jump manuever. */
				ViewStartJump() ;
			}

			ClientFaceOtherPlayers() ;

			/* Don't move unless we have received a message that we just */
			/* moved. */
			if (moved == TRUE)  {

//          for (i=0;i<9;i++) ButtonUp (buttons[i]);

/*            if ((KeyboardGetScanCode(KEY_SCAN_CODE_1)==TRUE)) ButtonDown(buttons[6]); else ButtonUp (buttons[6]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_2)==TRUE)) ButtonDown(buttons[7]); else ButtonUp (buttons[7]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_3)==TRUE)) ButtonDown(buttons[8]); else ButtonUp (buttons[8]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_4)==TRUE)) ButtonDown(buttons[3]); else ButtonUp (buttons[3]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_5)==TRUE)) ButtonDown(buttons[4]); else ButtonUp (buttons[4]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_6)==TRUE)) ButtonDown(buttons[5]); else ButtonUp (buttons[5]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_7)==TRUE)) ButtonDown(buttons[0]); else ButtonUp (buttons[0]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_8)==TRUE)) ButtonDown(buttons[1]); else ButtonUp (buttons[1]);
			if ((KeyboardGetScanCode(KEY_SCAN_CODE_9)==TRUE)) ButtonDown(buttons[2]); else ButtonUp (buttons[2]);
*/
			// JDA: ADDED 6/05/95

			if ((KeyBoardGetScanCode(KEY_SCAN_CODE_F1)==TRUE)) StatsChangeMana(1000);
            ButtonPushOnKey(); //JDA 05/31/95
            MouseUpdateEvents();
			GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
			ButtonUpdateAllButtons();

				ViewDraw() ;
				frames++ ;
//                delta = ClientGetDelta() ;
delta = ClientGetDelta() + lastbit ;
lastbit = delta&1 ;
delta = delta + (delta >> 1) ;

//delta *= 2 ;
//                moveAmount /= 2 ;
				moveAmount = (moveAmount+moveAmount+moveAmount)/4 ;

				oldAngle = G_moveAngle ;

				if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT) == TRUE)  {
				  if (ViewIsAboveGround()==FALSE || StatsGetAttribute (SPELL_AIR_WALK)) //added JDA 06/06/95
				  {
					if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT)==TRUE)  {
					ViewAccelDirection(G_moveAngle+INT_ANGLE_90, delta) ;
					}
					if (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT)==TRUE)  {
					ViewAccelDirection(G_moveAngle-INT_ANGLE_90, delta) ;
					}
				  }
				} else {
					if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT)==TRUE)  {
					G_moveAngle += (delta << 7);
					if (shift==TRUE)
						G_moveAngle += (delta << 7) ;
					}
					if (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT)==TRUE)  {
					G_moveAngle -= (delta << 7) ;
					if (shift==TRUE)
						G_moveAngle -= (delta << 7) ;
					}
				}

				if (ViewIsAboveGround()==FALSE || StatsGetAttribute (SPELL_AIR_WALK)) //added: JDA 06/06/95
				{
					if (KeyboardGetScanCode(KEY_SCAN_CODE_UP)==TRUE)
					ViewAccelDirection(G_moveAngle, delta<<(shift)) ;

					if (KeyboardGetScanCode(KEY_SCAN_CODE_DOWN)==TRUE)
					ViewAccelDirection(G_moveAngle, -(delta<<(shift))) ;
				}

	/*
				if (KeyboardGetScanCode(KEY_SCAN_CODE_CTRL)==TRUE)  {
					ClientFire(x, y, G_moveAngle) ;
				}
	*/
				if (KeyboardGetScanCode(KEY_SCAN_CODE_CTRL)==TRUE)  {
					if (TickerGet() >= G_lastAttack)  {
						G_lastAttack = TickerGet() + PLAYER_ATTACK_SPEED ;
						if (G_attackComplete == TRUE)  {
							if (attackDir == 0)  {
								attackDir = 1 ;
		                        attackTime = TickerGet() ;
	                        }
                            if (G_weapon < 2)
                                ClientSendAttackPacket() ;
                            else
                                ClientShootFireball() ;
                        }
                    }
	            }

                /* Make sure the angles and such are set. */
/*
                if (G_moveAngle >= INT_ANGLE_360)
                    G_moveAngle -= INT_ANGLE_360 ;
                else if (G_moveAngle < 0)
                    G_moveAngle += INT_ANGLE_360 ;
*/


//                if (moveAmount != 0)  {
//                    ViewStepDirection(G_moveAngle, moveAmount) ;
//                }
/*
                else if (moveAmount < 0) {
                    revAngle = G_moveAngle + INT_ANGLE_180 ;
                    ViewStepDirection(revAngle, moveAmount) ;
                }
*/

                /* Get our new location (after we hit walls and such) */
                ViewGetPOVLocation(&newx, &newy) ;

    //            onAlready = ClientCheckOtherPlayer(x, y) ;

                /* See if there is a player in the way. */
                ClientRequestMoveTo(
                    newx,
                    newy,
                    newAngle,
                    ViewGetPOVFootHeight()) ;

                newAngle = G_moveAngle ;

				ViewFaceDirection(G_moveAngle) ;
                ViewCheckDoor(G_moveAngle) ;
            }
            ViewUpdatePlayer() ;

/*
            if (KeyboardGetScanCode(KEY_SCAN_CODE_D)==TRUE)  {
                KeyboardDebounce() ;
                ViewToggleFloor() ;
                ViewToggleCeiling() ;
            }
            if (KeyboardGetScanCode(KEY_SCAN_CODE_M)==TRUE)  {
                KeyboardDebounce() ;
                sprintf(buffer, "Hello #%d", TickerGet()) ;
                MessageAdd(buffer) ;
            }
            if (KeyboardGetScanCode(KEY_SCAN_CODE_R)==TRUE)  {
                KeyboardDebounce() ;
                ActivitiesRun(0) ;
            }
            if (KeyboardGetScanCode(KEY_SCAN_CODE_F)==TRUE)  {
                KeyboardDebounce() ;
                ViewToggleFloorResolution() ;
            }
            if (KeyboardGetScanCode(KEY_SCAN_CODE_L)==TRUE)  {
                KeyboardDebounce() ;
                ViewToggleLightShading() ;
            }
*/
	        if ((KeyboardGetScanCode(KEY_SCAN_CODE_W)==TRUE) &&
                       (G_msgOn == FALSE)) {
	            if (attackDir == 0)  {
                    newWeapon = G_weapon ;
                    do {
                        newWeapon++ ;
                        if (newWeapon >= MAX_WEAPONS)
                            newWeapon = 0 ;
                    } while (G_haveWeapons[newWeapon] != TRUE) ;
                    if (newWeapon != G_weapon)
                        ClientChangeWeapon(newWeapon) ;
	            }
	            KeyboardDebounce() ;
	        }
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_MINUS)==TRUE) &&
                       (G_msgOn == FALSE))  {
	            if (volume > 16)  {
	                volume-= 16 ;
//	                KeyboardDebounce() ;
	            } else {
                    volume = 0 ;
                }
                SoundSetBackgroundVolume(volume) ;
            }
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_EQUAL)==TRUE) &&
                 (G_msgOn == FALSE))  {
	            if (volume < 240)  {
	                volume+= 16 ;
//	                KeyboardDebounce() ;
	            } else {
                    volume = 255 ;
                }
                SoundSetBackgroundVolume(volume) ;
            }
            ClientCheckScrolling() ;
            if (lastAnim+10 < TickerGet())  {
                lastAnim = TickerGet() ;
            }

        /* See if we need to do an action. */
        sector = ViewGetPlayerSector() ;

        if (sector != lastSector)  {
            /* We've gone to a different sector. */
            lastSector = sector ;

            /* Get the action for that sector. */
            action = ViewGetSectorAction(sector) ;

            /* Activate it (if it actually is something) */
            if (action != 0)
                ClientRequestAction(action) ;
        }

        /* Move the fireball (if there is one). */
        ClientUpdateFireball() ;

        ClientCheckStances() ;
        ClientAnimate() ;
        ClientUpdateHealth() ;
        SpellsUpdateManaDisplay();   //added 05/30/95 JDA

    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ClientCheckMouse                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientCheckMouse checks the status of the mouse and passes control    */
/*  if any buttons are selected                                             */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    Mouse control inputs                                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/31/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientCheckMouse (E_mouseEvent event,
                         T_word16 x,
                         T_word16 y,
                         E_Boolean button)

{
    static T_buttonID buttonpushed=NULL;
    DebugRoutine ("ClientCheckMouse");

    if (button==TRUE)
    {
        if (buttonpushed==NULL)
        {
            buttonpushed=ButtonGetLoc (x,y);
            if (buttonpushed!=NULL) ButtonDown (buttonpushed);
        }
        else
        {
            if (ButtonIsAt (buttonpushed,x,y)) ButtonDown (buttonpushed);
            else ButtonUp (buttonpushed);
        }
    }
    else
    {
        if (buttonpushed!=NULL)
        {
            if (ButtonIsAt (buttonpushed,x,y)) ButtonDoCallback (buttonpushed);
            buttonpushed=NULL;
        }
    }
    DebugEnd();
}


/****************************************************************************/
/*  Routine:  ClientSetupAngleTable                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSetupAngleTable setups a table of angles based on the relative  */
/*  position of two objects.  This is used to determine what direction      */
/*  a player in the view is facing.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 QuickATan(T_sword16 dx, T_sword16 dy)
{
    T_word16 angle ;

    DebugRoutine("QuickATan") ;

    if (dy > 0)  {
	    if (dx > 0)  {
	        if (dx > dy*2)  {
		        angle = 0 ;
	        } else if (dx*2 < dy)  {
		        angle = INT_ANGLE_90 ;
	        } else {
		        angle = INT_ANGLE_45 ;
	        }
	    } else {  /* dx < 0 */
	        if ((-dx) > dy*2)  {
		        angle = INT_ANGLE_180 ;
	        } else if ((-dx)*2 < dy)  {
		        angle = INT_ANGLE_90 ;
	        } else {
		        angle = INT_ANGLE_135 ;
	        }
	    }
        } else {  /* dy < 0 */
	    if (dx > 0)  {
	        if (dx > (-dy)*2)  {
		        angle = 0 ;
	        } else if (dx*2 < (-dy))  {
		        angle = INT_ANGLE_270 ;
	        } else {
		        angle = INT_ANGLE_315 ;
	        }
	    } else {  /* dx < 0 */
	        if ((-dx) > (-dy)*2)  {
		        angle = INT_ANGLE_180 ;
	        } else if ((-dx)*2 < (-dy))  {
		        angle = INT_ANGLE_270 ;
	        } else {
		        angle = INT_ANGLE_225 ;
	        }
	    }
    }

    DebugEnd() ;
    return angle ;
}

/****************************************************************************/
/*  Routine:  ClientFaceOtherPlayers                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientFaceOtherPlayers turn the objects so that the players face      */
/*  in the right direction.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientFaceOtherPlayers(T_void)
{
    T_word16 angle ;
    T_word16 i ;
    T_sword16 ox, oy ;
    T_sword16 x, y ;

    DebugRoutine("ClientFaceOtherPlayers") ;

    /* Get the this client's location. */
	ViewGetPOVLocation(&x, &y) ;

    /* Angle each of the other players. */
    for (i=0; i<G_numOtherPlayers; i++)  {
        /* Get the other player's position. */
        ViewGetObjectXY(G_playerObjects[i], &ox, &oy) ;

        /* Compute the angle to them. */
	    angle = MathArcTangent(
                    (T_sword16)x-(T_sword16)ox,
                    -((T_sword16)y-(T_sword16)oy))
                        + G_playerAngles[i]
                        + (INT_ANGLE_45/2) ;

        /* Make it a range from 0 to 7. */
	    angle /= INT_ANGLE_45 ;

        /* Turn the player. */
	    PeopleBMSetStanceAngle(G_playerIds[i], angle) ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ClientReceivePlayerJoinPacket                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerJoinPacket is a packet that is received when       */
/*  another player is joining the fray.  This routine sets up that player   */
/*  (using the defaults).                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- joinPacket being received.     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerJoinPacket(T_packetEitherShortOrLong *p_packet)
{
    T_joinPacket *p_join ;
    T_word16 player ;
    T_word16 i ;

    DebugRoutine("ClientReceivePlayerJoinPacket") ;

    if ((G_clientIsLogin == TRUE) && (G_logoutAttempted == FALSE))  {
        /* Get a quick pointer. */
        p_join = (T_joinPacket *)p_packet->data ;

        player = p_join->player ;

        for (i=0; i<G_numOtherPlayers; i++)
            if (G_playerIds[i] == player)
                break ;

        if (i == G_numOtherPlayers)  {
            /* Add and initialize that player. */
            /* Put the player on a list. */
            G_playerIds[G_numOtherPlayers] = player ;

            /* Create an object for the players. */
            G_playerObjects[G_numOtherPlayers] = ViewCreateObject() ;
            ViewDeclareMoveableObject(
                G_playerObjects[G_numOtherPlayers],
                1,
                1,
                255-G_numOtherPlayers) ;

            ViewChangeObjectsPicture(G_playerObjects[G_numOtherPlayers], 255-G_numOtherPlayers) ;

            /* Build the picture for that person. */
            PeopleBMAddPlayer(player, G_playerObjects[G_numOtherPlayers]) ;
            PeopleBMBuildStances() ;
            PeopleBMSetWeapon(player, WEAPON_TYPE_FIST) ;
            PeopleBMSetArmor(player, ARMOR_TYPE_CLOTHING) ;
            PeopleBMSetStanceAngle(player, 0) ;
            PeopleBMSetStance(player, STANCE_TYPE_STANDING) ;
            PeopleBMBuildStances() ;

            /* Increment the count of other players. */
            G_numOtherPlayers++ ;
    //DebugCheck(G_numOtherPlayers != 2) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerActionPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerActionPacket is received when another player has   */
/*  performed an activity (like pulling a switch).                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerActionPacket(T_packetEitherShortOrLong *p_packet)
{
    T_actionPacket *p_action ;

    DebugRoutine("ClientReceivePlayerActionPacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Get a quick pointer to the true action data. */
        p_action = (T_actionPacket *)p_packet->data ;

        /* Do the action (if in normal range). */
        if (p_action->action < 256)
            ActivitiesRun(p_action->action) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerAttackPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerAttackPacket is received when another player has   */
/*  attacked.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerAttackPacket(T_packetEitherShortOrLong *p_packet)
{
    T_attackPacket *p_attack ;
    T_word16 player, weapon ;
    T_word16 index ;
    T_word16 objNum ;
    T_sword16 oldx, oldy, newx, newy ;
    T_sword16 x, y ;
    T_sword32 dx, dy ;
    T_word32 dist ;

    DebugRoutine("ClientReceivePlayerAttackPacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Get a quick pointer. */
        p_attack = (T_attackPacket *)p_packet->data ;

        /* See who attacked and what weapon. */
        player = p_attack->player ;
        weapon = p_attack->weapon ;

        /* Make sure dagger or fist (for now).  !!! */
        if (weapon < 2)  {
            /* Get player index. */
            index = ClientGetPlayerObjectNum(player) ;

            /* Change the stance of the player. */
            PeopleBMSetStance(G_playerIds[index], STANCE_TYPE_ATTACK) ;

            /* Get true object num. */
            objNum = G_playerObjects[index] ;

            /* Get the old location of the player. */
            ViewGetObjectXY(objNum, &oldx, &oldy) ;

            /* Hit the area in front of the player by moving into it. */
            ViewStepObject(objNum, G_playerAngles[index], 50, 1) ;

            /* Get point in front of the player (where we are now). */
            ViewGetObjectXY(objNum, &newx, &newy) ;

            /* Move back to where we were. */
            ViewMoveObject(objNum, oldx, oldy) ;

            /* Get our location. */
            ViewGetPOVLocation(&x, &y) ;

            if (x > newx)
                dx = x-newx ;
            else
                dx = newx-x ;

            if (y > newy)
                dy = y-newy ;
            else
                dy = newy-y ;

            /* See if point is touching us.  Do this by getting the distance. */
            dist = dx*dx + dy*dy ;

            /* See if that is a hit! */
            if (dist < 4000)  {
                /* Yes, a hit. */
                /* Subtract the damage based on the weapon. */
                switch(weapon)  {
                    case 0:
                        if (rand()&1)
                            SoundPlayByName("ImHit1") ;
                        else
                            SoundPlayByName("ImHit2") ;
//                        playerHealth -= 10 ;
                        StatsHurtPlayer(100) ;
                        break ;
                    case 1:
                        if (rand()&1)
                            SoundPlayByName("ImHit1") ;
                        else
                            SoundPlayByName("ImHit2") ;
//                        playerHealth -= 20 ;
                        StatsHurtPlayer(200) ;
                        break ;
                }
            }

            G_lastMoveTime2[index] = TickerGet()+60 ;
        }
    }

    DebugEnd() ;
}

T_void ClientReceiveMonsterMovePacket(T_packetEitherShortOrLong *p_packet)
{
//putsflush("ClientRecieveMonsterMovePacket") ;
}
T_void ClientReceivePlayerActivatePacket(T_packetEitherShortOrLong *p_packet)
{
//putsflush("ClientReceivePlayerActivatePacket") ;
}

T_void ClientDialIn(T_void)
{
    T_byte8 buffer[80] ;
    T_byte8 call[80] ;
    T_word16 c ;
    T_word16 d = 0 ;
    T_word32 timeStart ;
    DebugRoutine("ClientDialIn") ;

//    GrGraphicsOff() ;
    KeyboardBufferOn() ;

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n") ;
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n") ;
    printf("Enter phone number or just hit <enter> if already connected:\n") ;
    printf("# > ") ;
    gets(buffer) ;
    if (strlen(buffer) != 0)  {
        timeStart = TickerGet()+5000 ;
        printf("Initializing modem\n") ;

        sprintf(call, "ATZ\r\n") ;

        while (CommGetReadBufferLength())  {
            c = CommReadByte() ;
            printf("J:%c ", c) ;
            fflush(stdout) ;
            if (TickerGet() > timeStart)  {
                printf("Sorry, connection took too long!") ;
                exit(1) ;
            }
        }

        CommSendData(call, strlen(call)) ;
        c = 0 ;
        do {
            if (CommGetReadBufferLength() > 0)  {
                c = CommReadByte() ;
                if (c != 0xFFFF)  {
                    printf("%c", c) ;
                    fflush(stdout) ;
                }
            }
            if (TickerGet() > timeStart)  {
                printf("Sorry, connection took too long!") ;
                exit(1) ;
            }
        } while (c != 'O') ;

        do {
            if (CommGetReadBufferLength() > 0)  {
                c = CommReadByte() ;
                if (c != 0xFFFF)  {
                    printf("%c", c) ;
                    fflush(stdout) ;
                }
            }
            if (TickerGet() > timeStart)  {
                printf("Sorry, connection took too long!\n") ;
                exit(1) ;
            }
        } while ((c != '\n') && (c != '\r')) ;

        printf("Modem initialized.  Calling now.\n") ;
fflush(stdout) ;

        sprintf(call, "ATDT%s\r\n", buffer) ;
        CommSendData(call, strlen(call)) ;

        do {
            if (CommGetReadBufferLength() > 0)  {
                c = CommReadByte() ;
                if (c != 0xFFFF)  {
                    printf("%c", c) ;
                    fflush(stdout) ;
                }
            }
            if (TickerGet() > timeStart)  {
                printf("Sorry, connection took too long!") ;
                exit(1) ;
            }
        } while ((c != 'C') && (c != 'B')) ;

        d = c ;

        do {
            if (CommGetReadBufferLength() > 0)  {
                c = CommReadByte() ;
                if (c != 0xFFFF)  {
                    printf("%c", c) ;
                    fflush(stdout) ;
                }
            }
            if (TickerGet() > timeStart)  {
                printf("Sorry, connection took too long!") ;
                exit(1) ;
            }
        } while ((c != '\n') && (c != '\r')) ;
    }

    if (d == 'B')  {
        printf("Cannot connect to busy line.  Try again later.") ;
        exit(1) ;
    }

///    KeyboardBufferOff() ;
//    GrGraphicsOn() ;

    DebugEnd() ;
}

T_word16 ClientCheckOtherPlayer(T_word16 x, T_word16 y)
{
    T_word16 code = 0xFFFF ;
    T_word16 dist ;
    T_word16 i ;
    T_sword16 d ;
    T_sword16 px, py ;

    DebugRoutine("ClientCheckOtherPlayer") ;

    for (i=0; i<6; i++)  {
        if (G_playerObjects[i] != -1)  {
            ViewGetObjectXY(G_playerObjects[i], &px, &py) ;
            d = px - x ;
            if (d < 0)
                d = -d ;
            if (d < 255)  {
                dist = d*d ;
                d = py - y ;
                if (d < 0)
                    d = -d ;
                    if (d < 255)  {
                        dist += d*d ;
                        if (dist < 25*25)  {
                            /* Hit! */
                            code = i ;
                            break ;
                        }
                    }
            }
        }
    }


    DebugEnd() ;

    return code ;
}

T_void ClientUpdateHealth()
{

    static T_sword16 old_health = 0xFFFF ;
    static T_word32 last_update = 0;
    static T_word16 heart_frame = 0;
    static T_word32 heart_time = 0;
//    static T_byte8 rgb[3]={0,0,0};
    T_byte8 shieldName[] = "HEART_?" ;
    T_word16 num ;
    T_bitmap *pic ;
    T_resource res ;

    DebugRoutine("ClientUpdateHealth") ;

    heart_time += (TickerGet() - last_update) ;
    last_update = TickerGet();

    while (heart_time >= StatsGetHeartRate())
    {
       heart_time-=StatsGetHeartRate();

       /* draw other frame of heart */
       heart_frame = 1-heart_frame;
       shieldName[6] = '1'+heart_frame;

       pic = (T_bitmap *)PictureLockData(shieldName, &res) ;
       DebugCheck(pic != NULL) ;
       if (pic != NULL)
       {
          GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
          GrDrawBitmap(pic, 60, 156) ;
          PictureUnlock(res) ;
       }

    }

    if (StatsGetPlayerHealth()<100) //flash red behind heart
    {                              //05/26/95 JDA
        ColorSetGlobal(0,-15,-15);
    }


    //code moved to stats.c 05/26/95 JDA
//    if (StatsGetPlayerHealth() <= 0)
//    {
//       StatsSetPlayerHealth (1000) ; //u died
//
//       ClientLogoff();
//    }

    DebugEnd() ;
}

T_void ClientHandleDamagePacket(T_damagePacket *p_damage)
{
    DebugRoutine("ClientHandleDamagePacket") ;

    if (playerHealth >= 0)  {
//        playerHealth -= 10 ;
//        ClientUpdateHealth() ;
        StatsHurtPlayer(100) ;
    }

    DebugEnd() ;
}

T_void ClientHandleFirePacket(T_firePacketData *p_fire)
{
    T_word16 objNum ;

    DebugRoutine("ClientHandleFirePacket") ;
    DebugCheck(p_fire != NULL) ;
    DebugCheck(p_fire->command == GAME_PACKET_PERFORM_FIRE) ;

    objNum = G_playerFires[p_fire->player-1] ;

    /* Is the fire being cancelled? */
    if (p_fire->x == 0xFFFF)  {
        /* Yes, it is.  Make the object inactive. */
        ViewObjectDeactivate(objNum) ;

        if (p_fire->player-1 == G_loginId)
            isFiring = FALSE ;
    } else {
        /* No, it isn't.  Make sure it is active and move it */
        /* to it's new location. */
        ViewObjectActivate(objNum) ;
        ViewMoveObject(objNum, p_fire->x, p_fire->y) ;
    }

    DebugEnd() ;
}

T_void ClientFire(T_word16 x, T_word16 y, T_word16 angle)
{
    T_packetShort packet ;
    T_firePacketData *p_fireData ;
    static T_word32 lastFireTime = 0 ;

    DebugRoutine("ClientFire") ;

    if (isFiring == TRUE)  {
        if (lastFireTime+300 < TickerGet())
            isFiring = FALSE ;
    }

    if (isFiring == FALSE)  {
        isFiring = TRUE ;

        lastFireTime = TickerGet() ;

        ViewMoveObject(G_playerFires[G_loginId], x, y) ;

        /* Get a quick pointer. */
        p_fireData = (T_firePacketData *)packet.data ;

        p_fireData->command = GAME_PACKET_REQUEST_FIRE ;
        p_fireData->x = x ;
        p_fireData->y = y ;
        p_fireData->angle = angle ;
        p_fireData->player = G_loginId ;

//        PacketSendShort(&packet) ;
    }

    DebugEnd() ;
}

T_void ClientMoveToComplete(
           T_word32 extraData,
           T_packetEitherShortOrLong *p_packet)
{
    G_clientMoveToComplete = TRUE ;
}

T_void ClientRequestMoveTo(
           T_sword16 x,
           T_sword16 y,
           T_word16 angle,
           T_sword16 height)
{
    T_packetShort packet ;
    T_movePacketData *p_moveData ;
    static T_sword16 last_x = 0x8000;
    static T_sword16 last_y = 0x8000 ;
    static T_word16 last_angle = 0x8003 ;
    static T_word16 last_height = 0x8000 ;

    static T_sword32 nextMoveTime = 0 ;
    T_word32 time ;

    DebugRoutine("ClientRequestMoveTo") ;

    /* See if we can move yet. */
    time = TickerGet() ;
    if (time >= nextMoveTime)  {
        nextMoveTime = time+7 ;

        if ((G_clientMoveToComplete == TRUE) && (ServerIsSynced() == TRUE))  {
            if ((x != last_x) ||
                (y != last_y) ||
                (angle != last_angle) ||
                (height != last_height))  {

                last_x = x ;
                last_y = y ;
                last_angle = angle ;
                last_height = height ;

                /* Get a quick pointer. */
                p_moveData = (T_movePacketData *)packet.data ;

                p_moveData->command = PACKET_COMMAND_PLAYER_MOVE ;
                p_moveData->x = x ;
                p_moveData->y = y ;
                p_moveData->angle = angle ;
                p_moveData->height = height ;
                p_moveData->player = G_loginId ;

                G_clientMoveToComplete = FALSE ;
                CmdQSendShortPacket(&packet, 60, 0, ClientMoveToComplete) ;
            }
        }
    }

    DebugEnd() ;
}

T_void ClientCheckStances(T_void)
{
    T_word16 i ;
    T_word32 time ;
    E_stanceType stance ;

    for (i=0; i<G_numOtherPlayers; i++)  {
        time = TickerGet() ;
        stance = PeopleBMGetStance(G_playerIds[i]) ;
        switch(stance)  {
            case STANCE_TYPE_STANDING:
                break ;
            case STANCE_TYPE_WALK_1:
            case STANCE_TYPE_WALK_2:
                if (time > G_lastMoveTime2[i])  {
                    G_lastMoveTime2[i] = time+700000 ;
                    G_walkPhase[i] = 0 ;
                    PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_STANDING) ;
                }
                break ;
            case STANCE_TYPE_ATTACK:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_2) ;
                break ;
            case STANCE_TYPE_ATTACK_2:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_3) ;
                break ;
            case STANCE_TYPE_ATTACK_3:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_4) ;
                break ;
            case STANCE_TYPE_ATTACK_4:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_5) ;
                break ;
            case STANCE_TYPE_ATTACK_5:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_6) ;
                break ;
            case STANCE_TYPE_ATTACK_6:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_ATTACK_7) ;
                break ;
            case STANCE_TYPE_ATTACK_7:
                PeopleBMSetStance(G_playerIds[i], STANCE_TYPE_STANDING) ;
                break ;
        }
    }
}

/****************************************************************************/
/*  Routine:  ClientReceiveCreatureMovePacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- move object packet             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ViewMoveObject                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCreatureMovePacket(T_packetEitherShortOrLong *p_packet)
{
    T_moveCreaturePacket *p_moveCreature ;
    T_sword16 playerX, playerY ;
    T_word16 angle ;
    T_void *p_creature ;
    T_word16 type ;
    T_word16 newangle ;

    DebugRoutine("ClientReceiveCreatureMovePacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_moveCreature = (T_moveCreaturePacket *)p_packet->data ;

        if (CreatureIsActive(p_moveCreature->object) == TRUE)  {
            if (ViewCheckObjectCollide(
                    p_moveCreature->object,
                    p_moveCreature->x,
                    p_moveCreature->y) == FALSE)  {
                ViewMoveObject(
                    p_moveCreature->object,
                    p_moveCreature->x,
                    p_moveCreature->y) ;
            }

            ViewSetObjectHeight(
                p_moveCreature->object,
                p_moveCreature->height) ;

/*
            ViewSetObjectAngle(
                p_moveCreature->object,
                p_moveCreature->angle&0xF800) ;
*/

            ViewSetObjectAngle(
                p_moveCreature->object,
                (p_moveCreature->angleAndStepping&0xF0)<<8) ;


            /* Now update that creature's picture. */
            ViewGetPOVLocation(&playerX, &playerY) ;


            angle = -MathArcTangent(
                        (playerX - (p_moveCreature->x)),
                         -(playerY - (p_moveCreature->y)) )
                            - ((p_moveCreature->angleAndStepping&0xF0) << 8)
                            + (INT_ANGLE_45/2) ;

/*
            angle = -MathArcTangent(
                        (playerX - (p_moveCreature->x)),
                         -(playerY - (p_moveCreature->y)) )
                            - (p_moveCreature->angle & 0xF800)
                            + (INT_ANGLE_45/2) ;
*/
            angle >>= 13 ;
            angle &= 7 ;

            p_creature = CreatureGetByObject(p_moveCreature->object) ;
            if (p_creature != NULL)  {
                type = CreatureGetType(p_creature) ;

                if (type == 2)  {
                    newangle = (8-angle)&7 ;
                    angle = newangle ;
                    newangle *= 6 ;
                    newangle += ((TickerGet()>>3)%6) ;
                    ViewChangeObjectPictureDirectly(
                        p_moveCreature->object,
                        p_stefanPictures[newangle]) ;
                } else if (type == 1) {
                    switch((p_moveCreature->angleAndStepping & 0xC)>>2)  {
                        case 0:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_shadowPictures[angle]) ;
                            break ;
                        case 1:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_shadowPictures[angle+8]) ;
                            break ;
                        case 2:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_shadowPictures[angle+16]) ;
                            break ;
                        case 3:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_shadowPictures[angle+24]) ;
                            break ;
                    }
                } else {
                    switch((p_moveCreature->angleAndStepping & 0xC)>>2)  {
                        case 0:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_impPictures[angle]) ;
                            break ;
                        case 1:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_impPictures[angle+8]) ;
                            break ;
                        case 2:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_impPictures[angle+16]) ;
                            break ;
                        case 3:
                            ViewChangeObjectPictureDirectly(
                                p_moveCreature->object,
                                p_impPictures[angle+24]) ;
                            break ;
                    }
                }
                ViewChangeObjectOrientation(
                    p_moveCreature->object,
                    G_creatureOrientation[angle]) ;
            }
/*
            if (p_moveCreature->angleAndStepping & 4)
                ViewChangeObjectPictureDirectly(
                    p_moveCreature->object,
                    p_impPictures[angle]) ;
            else
                ViewChangeObjectPictureDirectly(
                    p_moveCreature->object,
                    p_impPictures[angle+8]) ;
*/
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveDamageAtPacket                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveDamageAtPacket is a general notification that all things */
/*  in the given area should take damage.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- damage packet                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ServerSendToAllFrom                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveDamageAtPacket(T_packetEitherShortOrLong *p_packet)
{
    T_damageAtPacket *p_damageAt ;

    DebugRoutine("ServerReceiveDamageAtPacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_damageAt = (T_damageAtPacket *)p_packet->data ;

        // !!! Do whatever needs to be done.
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCreatureAttackPacket                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- move object packet             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ViewMoveObject                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCreatureAttackPacket(T_packetEitherShortOrLong *p_packet)
{
    T_creatureAttackPacket *p_creatureAttack ;
    T_sword16 playerX, playerY ;
    T_sword16 objX, objY ;
    T_word16 angle ;
    T_sword16 attackX, attackY ;
    T_void *p_creature ;
    T_word16 type ;

    DebugRoutine("ClientReceiveCreatureAttackPacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_creatureAttack = (T_creatureAttackPacket *)p_packet->data ;

        if (CreatureIsActive(p_creatureAttack->object) == TRUE)  {
            /* Now update that creature's picture. */
            ViewGetPOVLocation(&playerX, &playerY) ;

            ViewGetObjectXY(
                p_creatureAttack->object,
                &objX,
                &objY) ;

            angle = -MathArcTangent(
                        (playerX - objX),
                         -(playerY - objY) )
                            - p_creatureAttack->angle
                            + (INT_ANGLE_45/2) ;
            angle >>= 13 ;
            angle &= 7 ;

            p_creature = CreatureGetByObject(p_creatureAttack->object) ;
            if (p_creature != NULL)  {
                type = CreatureGetType(p_creature) ;

                if (type == 1)  {
                    ViewChangeObjectPictureDirectly(
                        p_creatureAttack->object,
                        p_shadowPictures[angle+32]) ;
                } else {
                    ViewChangeObjectPictureDirectly(
                        p_creatureAttack->object,
                        p_impPictures[angle+32]) ;
                }
            }

            attackX = objX + MathXTimesCosAngle(40, p_creatureAttack->angle) ;
            attackY = objY + MathXTimesSinAngle(40, p_creatureAttack->angle) ;

            attackX -= playerX ;
            attackY -= playerY ;

            /* See if point is touching us.  Do this by getting the distance. */
            if (((attackX*attackX) + (attackY*attackY)) < 4000)  {
                if (rand()&1)
                    SoundPlayByName("ImHit1") ;
                else
                    SoundPlayByName("ImHit2") ;
    //            playerHealth -= 5 ;
                StatsHurtPlayer(75) ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCreatureHurtPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveCreatureHurtPacket makes the picture of the creature     */
/*  look drawn back (as if hit).                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- creature hurt packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ViewGetPOVLocation                                                    */
/*    ViewGetObjectXY                                                       */
/*    ViewGetObjectAngle                                                    */
/*    MathArcTangent                                                        */
/*    ViewChangeObjectPictureDirectly                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCreatureHurtPacket(T_packetEitherShortOrLong *p_packet)
{
    T_creatureHurtPacket *p_creatureHurt ;
    T_sword16 playerX, playerY ;
    T_sword16 objX, objY ;
    T_word16 angle ;
    T_void *p_creature ;
    T_word16 type ;

    DebugRoutine("ClientReceiveCreatureHurtPacket") ;

    if (G_logoutAttempted == FALSE)  {
        if (CreatureIsActive(p_creatureHurt->object) == TRUE)  {
            p_creatureHurt = (T_creatureHurtPacket *)p_packet->data ;

            /* Now update that creature's picture. */
            ViewGetPOVLocation(&playerX, &playerY) ;

            ViewGetObjectXY(
                p_creatureHurt->object,
                &objX,
                &objY) ;

            ViewGetObjectAngle(
                p_creatureHurt->object,
                &angle) ;

            angle = -MathArcTangent(
                        (playerX - objX),
                         -(playerY - objY))
                            - angle
                            + (INT_ANGLE_45/2) ;
            angle >>= 13 ;
            angle &= 7 ;

            p_creature = CreatureGetByObject(p_creatureHurt->object) ;
            if (p_creature != NULL)  {
                type = CreatureGetType(p_creature) ;

                if (type == 2)  {
                    ViewChangeObjectPictureDirectly(
                        p_creatureHurt->object,
                        p_stefanPictures[angle+40]) ;
                } else if (type == 1)  {
                    ViewChangeObjectPictureDirectly(
                        p_creatureHurt->object,
                        p_shadowPictures[angle+40]) ;
                } else {
                    ViewChangeObjectPictureDirectly(
                        p_creatureHurt->object,
                        p_impPictures[angle+40]) ;
                }
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCreatureDeadPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveCreatureDeadPacket is the notice received when a         */
/*  creature has died.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- creature dead packet           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ViewChangeObjectPictureDirectly                                       */
/*    CreatureGone                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCreatureDeadPacket(T_packetEitherShortOrLong *p_packet)
{
    T_creatureDeadPacket *p_creatureDead ;
    T_void *p_creature ;
    T_word16 type ;

    DebugRoutine("ClientReceiveCreatureDeadPacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_creatureDead = (T_creatureDeadPacket *)p_packet->data ;

        /* Now update that creature's picture.  (Dead picture) */
/*
        ViewChangeObjectPictureDirectly(
            p_creatureDead->object,
            p_impPictures[48]) ;
        ViewMakeObjectPassable(p_creatureDead->object) ;
*/
        p_creature = CreatureGetByObject(p_creatureDead->object) ;
        if (p_creature != NULL)  {
            type = CreatureGetType(p_creature) ;

            if (type == 2)  {
                ViewChangeObjectPictureDirectly(
                    p_creatureDead->object,
                    p_stefanPictures[49]) ;
            } else if (type == 1)  {
                ViewChangeObjectPictureDirectly(
                    p_creatureDead->object,
                    p_shadowPictures[49]) ;
            } else {
                ViewChangeObjectPictureDirectly(
                    p_creatureDead->object,
                    p_impPictures[49]) ;
            }
        }

        ScheduleAddEvent(
            TickerGet()+10,
            IClientAnimateDead,
            p_creatureDead->object | (49 << 16)) ;
    }

    DebugEnd() ;
}

/*    LES  05/30/95  Added CreatureGone call to finish creature death.      */
T_void IClientAnimateDead(T_word32 data)
{
    T_word16 object ;
    T_word16 frame ;
    T_word16 type ;
    T_void *p_creature ;

    object = data & 0xFF ;
    frame = (data >> 16) ;

//printf("obj: %d\n", object) ;
    p_creature = CreatureGetByObject(object) ;
    if (p_creature != NULL)  {
        type = CreatureGetType(p_creature) ;
    } else {
        type = 0 ;
    }
//printf("AnimateDead: %d\n", type) ;
    frame++ ;
    if (frame == 53)  {
        if (type == 1)  {
            ViewChangeObjectPictureDirectly(
                object,
                p_shadowPictures[48]) ;
        } else if (type == 2)  {
            ViewChangeObjectPictureDirectly(
                object,
                p_stefanPictures[48]) ;
        } else {
            ViewChangeObjectPictureDirectly(
                object,
                p_impPictures[48]) ;
        }

        ViewMakeObjectPassable(object) ;

        /* Put the object on the ground. */
        ViewStepObject(object, 0, 0, ViewGetObjectWidth(object)>>1) ;

        /* Cancel the creature movement now. */
        CreatureGone(object) ;
    } else {
        if (type == 1)  {
            ViewChangeObjectPictureDirectly(
                object,
                p_shadowPictures[frame]) ;
        } else if (type == 2)  {
            ViewChangeObjectPictureDirectly(
                object,
                p_stefanPictures[frame]) ;
        } else {
            ViewChangeObjectPictureDirectly(
                object,
                p_impPictures[frame]) ;
        }

        ScheduleAddEvent(
            TickerGet()+10,
            IClientAnimateDead,
            object | (((T_word32)frame) << 16)) ;
    }
}

/****************************************************************************/
/*  Routine:  ClientReceiveReverseSectorPacket                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveReverseSectorPacket reverses any sliding floors or       */
/*  ceilings in a sector.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- reverse sector packet          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    SliderExist                                                           */
/*    SliderReverse                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveReverseSectorPacket(T_packetEitherShortOrLong *p_packet)
{
    T_reverseSectorPacket *p_reverseSector ;
    T_word16 sector ;
    T_word32 sliderId ;

    DebugRoutine("ClientReceiveReverseSectorPacket") ;

    if (G_logoutAttempted == FALSE)  {
        p_reverseSector = (T_reverseSectorPacket *)p_packet->data ;
        sector = p_reverseSector->sector ;

        /* Try to reverse the floor slider at this sector. */
        /* Get the id. */
//        sliderId = ((T_word32)sector) | (SLIDER_TYPE_FLOOR<<16) ;

        /* If there is one, reverse the slider. */
//        if (SliderExist(sliderId) == TRUE)
//            SliderReverse(sliderId, p_reverseSector->activity) ;


        /* Try to reverse the ceiling slider at this sector. */
        /* Get the id. */
        sliderId = ((T_word32)sector) | (SLIDER_TYPE_CEILING<<16) ;

        /* If there is one, reverse the slider. */
        if (SliderExist(sliderId) == TRUE)  {
//            SliderReverse(sliderId, p_reverseSector->activity) ;
            DoorOpen(sliderId & 0xFFFF) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendReverseSectorPacket                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendReverseSectorPacket sends out a request to the server       */
/*  to reverse the given sector.  Since this request is needed immediately, */
/*  a call to ClientReceiveReverseSectorPacket is made immediately before   */
/*  the packet is actually sent.                                            */
/*    The server also knows NOT to send a reverse packet back to the        */
/*  player.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector                     -- reversing sector               */
/*                                                                          */
/*    T_word16 newActivity                -- new activity for slider        */
/*                                           completion                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQSendShortPacket                                                   */
/*    ClientReceiveReverseSectorPacket                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendReverseSectorPacket(T_word16 sector, T_word16 newActivity)
{
    T_packetShort reversePacket ;
    T_reverseSectorPacket *p_reverseSector ;

    DebugRoutine("ClientSendReverseSectorPacket") ;

    /* Fill the packet with the given data. */
    p_reverseSector = (T_reverseSectorPacket *)reversePacket.data ;
    p_reverseSector->command = PACKET_COMMAND_REVERSE_SECTOR ;
    p_reverseSector->sector = sector ;
    p_reverseSector->activity = newActivity ;
    p_reverseSector->player = G_loginId ;

    /* Send it out. */
    CmdQSendShortPacket(&reversePacket, 70, 0, NULL) ;

    /* Immediately call ourselves to make the reverse start. */
    ClientReceiveReverseSectorPacket(
        (T_packetEitherShortOrLong *)&reversePacket) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientTakeDamage                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientTakeDamage is the one routine that should be called when        */
/*  a player takes damage.                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 amount                     -- Amount of damage to give       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientTakeDamage(T_word16 amount)
{
    DebugRoutine("ClientTakeDamage") ;

    StatsHurtPlayer(amount) ;
//    playerHealth -= amount ;
//    if (playerHealth < 0)
//        playerHealth = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientIsOver                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientIsOver is called when the player moves over an object and       */
/*  might want to pick up or take some action for "touching" this object.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object being touched           */
/*                                                                          */
/*    T_word16 objectType                 -- object type being touched      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*    LES  04/19/95  Handle weapons too.                                    */
/*                                                                          */
/****************************************************************************/

T_void ClientIsOver(T_word16 objNum, T_word16 objectType)
{
    DebugRoutine("ClientIsOver") ;

    if (ClientIsObjectPending(objNum) == FALSE)  {
        ClientAddPendingObject(objNum) ;

        switch(objectType & OBJECT_TYPE_MASK)  {
            case OBJECT_TYPE_APPLE:
                if (StatsGetPlayerHealth() < MAX_HEALTH)
                    ClientRequestPickUpObject(objNum) ;
                break ;
            case OBJECT_TYPE_XBOW:
                if (G_haveWeapons[WEAPON_XBOW] != TRUE)
                    ClientRequestPickUpObject(objNum) ;
                break ;
            case OBJECT_TYPE_SWORD:
                if (G_haveWeapons[WEAPON_SWORD] != TRUE)
                    ClientRequestPickUpObject(objNum) ;
                break ;
        } ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientRequestPickUpObject                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRequestPickUpObject sends the server a request to take a        */
/*  object.  The server will send back a player takes package as a          */
/*  response.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object requested for taking    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientRequestPickUpObject(T_word16 objNum)
{
    T_packetShort packet ;
    T_pickUpPacket *p_pickUp ;

    DebugRoutine("ClientRequestPickUpObject") ;

    /* Get a quick pointer. */
    p_pickUp = (T_pickUpPacket *)packet.data ;

    p_pickUp->command = PACKET_COMMAND_PICK_UP ;
    p_pickUp->objNum = objNum ;
    p_pickUp->player = G_loginId ;

    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePickUpPacket                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePickUpPacket is called when a pick up of an object is    */
/*  received.  Either this player or another player/creature is picking     */
/*  up an object.  This routine takes care of all that.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object requested for taking    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePickUpPacket(T_packetEitherShortOrLong *p_packet)
{
    T_pickUpPacket *p_pickUp ;

    DebugRoutine("ClientReceivePickUpPacket") ;

    /* Get a quick pointer. */
    p_pickUp = (T_pickUpPacket *)(p_packet->data) ;

    if (G_loginId == p_pickUp->player)  {
        /* Finally!  I got the object! */
        ClientHasGrabbedObject(p_pickUp->objNum) ;
    }

    /* The object is now picked up. */
    ClientRemovePendingObject(p_pickUp->objNum) ;
    ViewRemoveObject(p_pickUp->objNum) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientHasGrabbedObject                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientHasGrabbedObject is called once a confirmed object grab has     */
/*  been executed.  The player can now do whatever is needed for picking    */
/*  up this object.                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object requested for taking    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientHasGrabbedObject(T_word16 objNum)
{
    T_word16 typeObj ;
    DebugRoutine("ClientHasGrabbedObject") ;

    /* What type of object is this? */
    typeObj = ViewGetObjectType(objNum) ;

    switch(typeObj & OBJECT_TYPE_MASK)  {
        case OBJECT_TYPE_APPLE:
            StatsHealPlayer (HEALING_AMOUNT_APPLE);
            SoundPlayByNumber(19) ;
            break ;
        case OBJECT_TYPE_SWORD:
            G_haveWeapons[WEAPON_SWORD] = TRUE ;
            SoundPlayByNumber(19) ;
            ColorAddGlobal (20, 20, 20);
            break ;
        case OBJECT_TYPE_XBOW:
            G_haveWeapons[WEAPON_XBOW] = TRUE ;
            SoundPlayByNumber(19) ;
            ColorAddGlobal (20, 20, 20);
            break ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientRemovePendingObject                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRemovePendingObject removes an object from the pending object   */
/*  list.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object on pending list         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientRemovePendingObject(T_word16 objNum)
{
    T_word16 i, j, count ;

    DebugRoutine("ClientRemovePendingObject") ;

    count = G_pendingCount ;
    for (i=j=0; i<count; i++, j++)  {
        if (G_pendingObjects[i] == objNum)  {
            i++ ;
            G_pendingCount-- ;
        }
        G_pendingObjects[j] = G_pendingObjects[i] ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientAddPendingObject                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientAddPendingObject adds an object onto the pending list.          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object on pending list         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientAddPendingObject(T_word16 objNum)
{
    T_word16 i, j, count ;

    DebugRoutine("ClientAddPendingObject") ;

    DebugCheck(G_pendingObjects != MAX_PENDING_OBJECTS) ;

    if (G_pendingObjects != MAX_PENDING_OBJECTS)
        G_pendingObjects[G_pendingCount++] = objNum ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientIsObjectPending                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientIsObjectPending checks to see if an object is on the pending    */
/*  for pickup list.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum                     -- object on pending list         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ClientIsObjectPending(T_word16 objNum)
{
    T_word16 i ;
    E_Boolean status = FALSE ;

    DebugRoutine("ClientIsObjectPending") ;

    for (i=0; i<G_pendingCount; i++)  {
        if (G_pendingObjects[i] == objNum)  {
            status = TRUE ;
            break ;
        }
    }

    DebugEnd() ;

    return status ;
}

/****************************************************************************/
/*  Routine:  ClientHandleKeyboard                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientHandleKeyboard takes care of all keyboard events created by     */
/*  the client.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_keyboardEvent event               -- Keyboard event to process      */
/*                                                                          */
/*    T_byte8 scanKey                     -- Key to go with event           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientHandleKeyboard(E_keyboardEvent event, T_byte8 scankey)
{
    DebugRoutine("ClientHandleKeyboard") ;

    switch(event)  {
        case KEYBOARD_EVENT_BUFFERED:
            if (G_msgOn == TRUE)  {
                if (scankey == '\b')  {
                    if (G_msgPos > 0)  {
                        G_msgPos-- ;
                        G_message[G_msgPos] = '\0' ;
                    }
                } else if ((scankey == '\r') || (scankey == '\n'))  {
                    ClientSendMessage() ;
                    KeyboardBufferOff() ;
                    G_msgOn = FALSE ;
                } else if (scankey == 0x1b /* ESC */)  {
                    G_msgPos = 0 ;
                    G_msgOn = FALSE ;
                    KeyboardBufferOff() ;
                } else if (((isalnum(scankey)) ||
                           (ispunct(scankey)) ||
                           (scankey == ' ')) &&
                           (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT)!=TRUE) &&
                           (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT)!=TRUE) &&
                           (KeyboardGetScanCode(KEY_SCAN_CODE_UP)!=TRUE) &&
                           (KeyboardGetScanCode(KEY_SCAN_CODE_DOWN)!=TRUE))  {
                   if (G_msgPos < MAX_MESSAGE_LEN)
                        G_message[G_msgPos++] = scankey ;
                }
            } else if (G_talkBlock == TRUE)  {
                switch (scankey)  {
                    case '1':
                    case '2':
                    case '3':
                       G_talkMode = scankey - '1' ;
                       G_talkBlock = FALSE ;
                       KeyboardBufferOff() ;
                       break ;
                    case 0x1b /* ESC */:
                       KeyboardBufferOff() ;
                       G_talkBlock = FALSE ;
                       break ;
                }
            } else if (G_cannedBlock == TRUE)  {
                /* Don't care if upper or lower case. */
                scankey = toupper(scankey) ;
                /* See if A to I. */
                if ((scankey >= 'A') && (scankey <= 'J'))  {
                    /* Got one.  Send the canned message. */
                    ClientSendCannedSayingPacket(scankey-'A') ;

                    /* Now react finish by doing an ESCAPE. */
                    scankey = 0x1b ;
                }

                /* Hitting the Escape?  Get rid of the block. */
                if (scankey == 0x1b /* ESC */)  {
                    KeyboardBufferOff() ;
                    G_cannedBlock = FALSE ;
                }
            }
/*
            if (G_msgOn == FALSE)  {
                if (scankey == ' ')  {
                    action = ViewGetForwardWallActivation() ;
                    if (action != 0)
                        ClientRequestAction(action) ;
                }
            }
*/
            break ;
        case KEYBOARD_EVENT_PRESS:
            if ((G_msgOn == FALSE) &&
                (G_talkBlock == FALSE) &&
                (G_cannedBlock == FALSE))  {

                if (KeyboardGetScanCode(KEY_SCAN_CODE_ESC)==TRUE)  {
                    ClientLogoff() ;
                }
                if (scankey == KEY_SCAN_CODE_D)  {
                    MemDumpDiscarded() ;
                    KeyboardOff() ;
                    system("C:\COMMAND.COM") ;
                    KeyboardOn() ;
                }
                if (scankey == KEY_SCAN_CODE_SPACE)  {
                    action = ViewGetForwardWallActivation() ;
                    if (action != 0)
                        ClientRequestAction(action) ;
                }
                if (scankey == KEY_SCAN_CODE_M)  {
                    G_talkBlock = TRUE ;
                    KeyboardBufferOn() ;
                    G_msgPos = 0 ;
                    G_message[G_msgPos] = '\0' ;
                }
#ifndef NDEBUG
                /* Check to see if ALT-H is hit.  This turns on */
                /* Heap checking in the debugger. */
                if (scankey == KEY_SCAN_CODE_H)  {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                        DebugHeapOn() ;
                        MessageAdd("Heap checking turned ON") ;
                    }
                }
                /* Check to see if ALT-O is hit.  This turns off */
                /* Heap checking in the debugger. */
                if (scankey == KEY_SCAN_CODE_O)  {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                        DebugHeapOff() ;
                        MessageAdd("Heap checking turned OFF") ;
                    }
                }
                /* Check to see if ALT-1 or ALT-2 is hit.  Teleport */
                /* to a hard coded location. */
                if (scankey == KEY_SCAN_CODE_1)  {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                        MessageAdd("Teleport!") ;
                        ViewTeleport(3872, 3552, ViewGetPOVFacingDir()) ;
                    }
                }
                if (scankey == KEY_SCAN_CODE_2)  {
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)==TRUE)  {
                        MessageAdd("Teleport!") ;
                        ViewTeleport(1344, 3744, ViewGetPOVFacingDir()) ;
                    }
                }
#endif
            }
            break ;
        case KEYBOARD_EVENT_RELEASE:
            if ((G_msgOn == FALSE) &&
                (G_talkBlock == FALSE) &&
                (G_cannedBlock == FALSE))  {

                if (scankey == KEY_SCAN_CODE_SLASH)  {
                    G_msgOn = TRUE ;
                    KeyboardBufferOn() ;
                    G_msgPos = 0 ;
                    G_message[G_msgPos] = '\0' ;
                }

                if (scankey == KEY_SCAN_CODE_F5)  {
                    KeyboardBufferOn() ;
                    G_cannedBlock = TRUE ;
                }
            }
            break ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendMessage                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendMessage sends the message that the user just typed in across*/
/*  the network and to everyone else in range.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQSendLongPacket                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendMessage(T_void)
{
    T_packetLong packet ;
    T_messagePacket *p_msg ;

    DebugRoutine("ClientSendMessage") ;

    /* Get a quick pointer. */
    p_msg = (T_messagePacket *)packet.data ;

    /* Put the message in the packet. */
    p_msg->command = PACKET_COMMAND_MESSAGE ;
    p_msg->player = G_loginId ;
    p_msg->mode = G_talkMode ;
    strcpy(p_msg->message, G_message) ;

    /* Send the whole packet. */
    CmdQSendLongPacket(&packet, 280, 0, NULL) ;

    /* Send it back to ourselves so we have a running log. */
    ClientReceiveMessagePacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveMessagePacket                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveMessagePacket is called when someone sends a message     */
/*  about someone saying something.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveMessagePacket(T_packetEitherShortOrLong *p_packet)
{
    T_byte8 buffer[80] ;
    static T_byte8 *whom[10] = {
        "Server",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I"
    } ;
    T_messagePacket *p_msg ;

    DebugRoutine("ClientReceiveMessagePacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Get a quick pointer. */
        p_msg = (T_messagePacket *)(p_packet->data) ;

        if (p_msg->message[0] == '>')  {
            sprintf(buffer, "%s shouts, \"%s\"",
                whom[p_msg->player],
                p_msg->message+1) ;
        } else if (p_msg->message[0] == '<')  {
            sprintf(buffer, "%s whispers, \"%s\"",
                whom[p_msg->player],
                p_msg->message+1) ;
        } else if (p_msg->mode == 1)  {
            sprintf(buffer, "%s shouts, \"%s\"",
                whom[p_msg->player],
                p_msg->message) ;
        } else if (p_msg->mode == 2)  {
            sprintf(buffer, "%s whispers, \"%s\"",
                whom[p_msg->player],
                p_msg->message) ;
        } else {
            sprintf(buffer, "%s says, \"%s\"",
                whom[p_msg->player],
                p_msg->message) ;
        }
        MessageAdd(buffer) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveOpenDoorPacket                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveOpenDoorPacket is received when some door somewhere is   */
/*  being forced opened (usually under crushing circumstances).             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- open door packet.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoorForceOpen                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveOpenDoorPacket(T_packetEitherShortOrLong *p_packet)
{
    T_openDoorPacket *p_openDoor ;

    DebugRoutine("ClientReceiveOpenDoorPacket") ;

    if (G_logoutAttempted == FALSE)  {
        /* Get a quick pointer to the true action data. */
        p_openDoor = (T_openDoorPacket *)p_packet->data ;

        /* Force open the door. */
        DoorForceOpen(p_openDoor->sector) ;
    }

    DebugEnd() ;
}

T_word16 ClientGetLoginId(T_void)
{
    return G_loginId ;
}

/****************************************************************************/
/*  Routine:  ClientSendCannedSayingPacket                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendCannedSaying puts out a packet for the given saying.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 saying             -- Saying to say.                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendCannedSayingPacket(T_word16 saying)
{
    T_packetShort packet ;
    T_cannedSayingPacket *p_can ;

    DebugRoutine("ClientSendCannedSaying") ;
    DebugCheck(saying < MAX_CANNED_SAYINGS) ;

    /* Get a quick pointer. */
    p_can = (T_cannedSayingPacket *)packet.data ;

    /* Put the message in the packet. */
    p_can->command = PACKET_COMMAND_CANNED_SAYING ;
    p_can->player = G_loginId ;
    p_can->saying = saying ;
    p_can->talkMode = G_talkMode ;
    ViewGetPOVLocation(&p_can->x, &p_can->y) ;

    /* Send the packet. */
    CmdQSendShortPacket(&packet, 280, 0, NULL) ;

    /* Send it back to ourselves immediately. */
    ClientReceiveCannedSayingPacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCannedSayingPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveCannedSaying handles a canned saying packet and plays    */
/*  it in the area.                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- canned saying packet.          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MessageAdd                                                            */
/*    ViewGetPOVLocation                                                    */
/*    AreaSoundCreate                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCannedSayingPacket(T_packetEitherShortOrLong *p_packet)
{
    static T_byte8 *cannedSayings[MAX_CANNED_SAYINGS] = {
        "Where are you?",
        "Help!",
        "Turn around",
        "Aargh!",
        "Yikes!",
        "Hello",
        "Defend yourself",
        "<< rasberry sound effect >>",
        "Look out!",
        "Have ya' had enough yet?"
    } ;
    T_cannedSayingPacket *p_can ;
    T_sword16 x, y ;
//T_sword16 px, py ;

    DebugRoutine("ClientReceiveCannedSayingPacket") ;

    /* Get a quick pointer. */
    p_can = (T_cannedSayingPacket *)p_packet->data ;

    /* Make sure this is a valid packet by seeing if the canned */
    /* saying is a known saying. */
    if (p_can->saying < MAX_CANNED_SAYINGS)  {
        /* Play the sound effect at that location. */
        AreaSoundCreate(
            p_can->x, p_can->y,
            ClientGetTalkDistance(p_can->talkMode),
            255,
            AREA_SOUND_TYPE_ONCE,
            30,
            NULL,
            NULL,
            0,
            CANNED_SAYINGS_FIRST_SOUND + p_can->saying) ;

//ViewGetPOVLocation(&px, &py) ;
//AreaSoundUpdate(px, py) ;

        /* For the deaf, put up a message. */
        MessageAdd(cannedSayings[p_can->saying]) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientGetTalkDistance                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientGetTalkDistance determines how far a sound is from a player     */
/*  for the given talk mode.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 talkMode           -- Type of talk mode                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/15/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 ClientGetTalkDistance(T_word16 talkMode)
{
    static talkDistances[CLIENT_NUM_TALK_MODES] = {
        MESSAGE_RANGE_TALK,
        MESSAGE_RANGE_SHOUT,
        MESSAGE_RANGE_WHISPER
    } ;

    DebugRoutine("ClientGetTalkDistance") ;
    DebugCheck(G_talkMode < CLIENT_NUM_TALK_MODES) ;

    DebugEnd() ;

    return talkDistances[talkMode] ;
}

/****************************************************************************/
/*    END OF FILE:  CLIENT.C                                                */
/****************************************************************************/
