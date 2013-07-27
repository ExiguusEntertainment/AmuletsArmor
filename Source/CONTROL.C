/****************************************************************************/
/*    FILE:  CONTROL.C                                                      */
/****************************************************************************/
#include "BANNER.H"
#include "BUTTON.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CONTROL.H"
#include "CRELOGIC.H"
#include "ESCMENU.H"
#include "GENERAL.H"
#include "HARDFORM.H"
#include "INVENTOR.H"
#include "KEYSCAN.H"
#include "LOOK.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "MOUSEMOD.H"
#include "NOTES.H"
#include "OBJECT.H"
#include "PEOPHERE.H"
#include "PICS.H"
#include "PLAYER.H"
#include "RESOURCE.H"
#include "SPELLS.H"
#include "STATS.H"
#include "TICKER.H"
#include "VIEW.H"
#include <ctype.h>

/*---------------------------------------------------------------------------
 * Constants:
 *--------------------------------------------------------------------------*/
#define JUMP_RESET_TIME 20

/*---------------------------------------------------------------------------
 * Types:
 *--------------------------------------------------------------------------*/
typedef struct {
    E_controlMouseModes mode; /* current mouse mode */
    T_buttonID buttonpushed; /* Is a button being pushed? */
    T_word16 hotx, hoty; /* Hot spot of mouse. */
    T_resource mouseRes; /* Current pointer resource */
    T_bitmap *bitmap; /* Current pointer bitmap **/
    E_controlMousePointerType type;
} T_controlMouseStruct;

/*---------------------------------------------------------------------------
 * Globals:
 *--------------------------------------------------------------------------*/
static T_controlMouseStruct G_mouse;
static E_Boolean G_resetNeeded = FALSE;
static E_Boolean G_spellWasCast = FALSE;
static T_sword16 G_lookAngle = 0;
static T_sword16 G_lookOffset = 0;
static E_Boolean G_autoID = FALSE;
static T_3dObject* ControlLookAt(T_word16 x, T_word16 y);
static T_void ControlExamineObject(T_3dObject *p_obj);
static T_word16 ControlSetMovePointer(T_word16 mx, T_word16 my);

/* LES:  Added to ensure no double init's */
static E_Boolean G_init = FALSE;

/*---------------------------------------------------------------------------
 * Prototypes:
 *--------------------------------------------------------------------------*/
static T_void ControlMouseControlForUI(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick button);
static T_void ControlMouseControlForGame(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick button);

/****************************************************************************/
/*  Routine:  ControlInitForGamePlay                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Sets up the mouse (& keyboard?) for control in the game.              */
/*    Initializes all necessary variables and default mouse bitmap, ect.    */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  11/30/95  Created                                                */
/*    LES  03/06/96  Renamed to ControlInitForGamePlay and copied.          */
/*                                                                          */
/****************************************************************************/

T_void ControlInitForGamePlay(T_void)
{
    DebugRoutine("ControlInitForGamePlay");
    DebugCheck(G_init == FALSE);

    G_init = TRUE;

    /** Initialize the mouse. **/
    G_mouse.buttonpushed = FALSE;
    G_mouse.hotx = 0;
    G_mouse.hoty = 0;
    G_mouse.mode = CONTROL_MOUSE_MODE_NORMAL;
    G_mouse.bitmap = (T_bitmap *)PictureLock("UI/MOUSE/DEFAULT",
            &G_mouse.mouseRes);
    DebugCheck(G_mouse.bitmap != NULL );
    MouseSetDefaultBitmap(0, 0, G_mouse.bitmap);
    MouseUseDefaultBitmap();

    /* set up control handler */
    MousePushEventHandler(ControlMouseControlForGame);

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ControlInitForJustUI                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Sets up the mouse (& keyboard?) for control in ui situations.         */
/*    Initializes all necessary variables and default mouse bitmap, ect.    */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Copied from old ControlInit                            */
/*                                                                          */
/****************************************************************************/

T_void ControlInitForJustUI(T_void)
{
    DebugRoutine("ControlInitForJustUI");
    DebugCheck(G_init == FALSE);

    G_init = TRUE;

    /** Initialize the mouse. **/
    G_mouse.buttonpushed = FALSE;
    G_mouse.hotx = 0;
    G_mouse.hoty = 0;
    G_mouse.mode = CONTROL_MOUSE_MODE_NORMAL;
    G_mouse.bitmap = (T_bitmap *)PictureLock("UI/MOUSE/DEFAULT",
            &G_mouse.mouseRes);
    DebugCheck(G_mouse.bitmap != NULL );
    MouseSetDefaultBitmap(0, 0, G_mouse.bitmap);
    MouseUseDefaultBitmap();

    /* set up control handler */
    MousePushEventHandler(ControlMouseControlForUI);

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ControlMouseControlForUI                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine does the standard mouse update for the ui.               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ControlMouseControlForUI(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick button)
{
    DebugRoutine("ControlMouseControlForUI");

    /* check buttons */
    ButtonMouseControl(event, x, y, button);
    /* check sliders */
    SliderMouseControl(event, x, y, button);
    /* check text boxes */
    TxtboxMouseControl(event, x, y, button);

    DebugCompare("ControlMouseControlForUI");
    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ControlMouseControlForGame                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This is the default (main) control loop for the mouse.                */
/*    It is set as the mouse control loop by ControlInit.                   */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  11/30/95  Created                                                */
/*    LES  03/06/96  Removed and put code into ControlMouseControlForUI     */
/*                                                                          */
/****************************************************************************/

static T_void ControlMouseControlForGame(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick button)
{
    T_3dObject *p_obj = NULL;
    T_word16 vx1, vy1, vx2, vy2, vcx, vcy;
    T_word16 dir;
    T_inventoryItemStruct *itemToUse;
    static T_word16 jumpTimer = 0;
    static T_word32 last_update = 0;
    T_word32 time_elapsed;
    DebugRoutine("ControlMouseControlForGame");

#   ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
    View3dTellMouseAt(x, y);
#   endif

    /* Do the standard ui stuff. */
    ControlMouseControlForUI(event, x, y, button);

    if (!EscapeMenuIsOpen()) {

    //    if (event!=MOUSE_EVENT_IDLE)
    //    {
    //        MessagePrintf("Mouse event:%d\n",event);
    //    }

        /* update delta timer */
        /* get time elapsed */
        time_elapsed = TickerGet() - last_update;
        last_update = TickerGet();
        jumpTimer -= time_elapsed;
        if (jumpTimer > JUMP_RESET_TIME)
            jumpTimer = 0;

        switch (event) {
            case MOUSE_EVENT_IDLE:
                break;

            case MOUSE_EVENT_START:
                G_resetNeeded = FALSE;
                /* which button was pressed ? */
                if (button == MOUSE_BUTTON_LEFT) {
                    /* check for a jump */
                    if (jumpTimer > 0) {
                        /* we're jumping */
                        jumpTimer = JUMP_RESET_TIME;
                        PlayerJump(StatsGetJumpPower());
                    }

                    /* ok, we are going to move/get/activate/throw, ect */
                    /* where are we at? */
                    if (ViewIsAt(x, y)) {
                        if (ClientGetMode() == CLIENT_MODE_HARD_CODED_FORM) {
                            HardFormHandleMouse(event, x, y, button);
                        } else {
                            /* is an object in mouse hand currently? */
                            if (InventoryObjectIsInMouseHand()) {
                                /* throw object into world */
                                InventoryThrowObjectIntoWorld(x, y);
                            } else {
                                /* no object currently in hand.  Try to grab one. */
                                p_obj = ViewGetXYTarget(x, y);
                                if (p_obj != NULL ) {
                                    /* hey, there is one there! */
                                    /* Ask the server politely to give me the object */
                                    /* see if it's a monster or another player */
                                    if (ObjectIsGrabable(p_obj)) {
                                        if (InventoryCanTakeItem(p_obj)) {
                                            ClientRequestTake(p_obj, KeyboardGetScanCode(KEY_SCAN_CODE_ALT));
                                        } else {
                                            /* some routine here to take object */
                                            /* as well as download description */
                                            /* of object at same time */
                                        }
                                    } else {
                                        G_mouse.mode = CONTROL_MOUSE_MODE_MOVE;
                                        ControlSetMovePointer(x, y);
                                    }
                                } else {
                                    /* no object there, set to move */
                                    G_mouse.mode = CONTROL_MOUSE_MODE_MOVE;
                                    /* set move pointer */
                                    ControlSetMovePointer(x, y);
                                }
                            }
                        }
                    } else if (InventoryInventoryWindowIsAt(x, y)) {
                        /* place item in inventory or get item */
                        InventoryTransferToInventory(x, y);
                    } else if (InventoryReadyBoxIsAt(x, y)) {
                        /* swap items with ready box */
                        InventoryTransferToReadyHand();
                    } else if (InventoryEquipmentWindowIsAt(x, y)) {
                        /* try to equip item */
                        InventoryTransferToEquipment(x, y);
                    } else if (NotesJournalWindowIsAt(x, y)) {
                        /* add or remove a journal page */
                        NotesTransferJournalPage();
                    } else if (BannerFinancesWindowIsAt(x, y)) {
                        InventoryTransferToFinances();
                    } else if (BannerAmmoWindowIsAt(x, y)) {
                        InventoryTransferToAmmo();
                    }

                    /* update mouse pointer */
                    if (G_mouse.mode == CONTROL_MOUSE_MODE_NORMAL) {
                        if (InventoryObjectIsInMouseHand()) {
                            ControlSetObjectPointer(
                                    InventoryCheckObjectInMouseHand());
                        } else {
                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_DEFAULT);
                            MouseUseDefaultBitmap();
                        }
                    }
                } else if (button == MOUSE_BUTTON_RIGHT) {
                    G_mouse.mode = CONTROL_MOUSE_MODE_LOOK;
                    /* see if we have a valid object */
                    p_obj = ControlLookAt(x, y);
                    if (p_obj != NULL ) {
                        /* set bitmap to good eye */
                        ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_LOOK);
                    } else {
                        /* set bitmap to bad eye */
                        ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_NOLOOK);
                    }
                }
                break;

            case MOUSE_EVENT_END: {
                /* reset use button for next attack */
                InventoryResetUse();

                G_resetNeeded = FALSE;
                G_lookAngle = 0;
                G_lookOffset = 0;
                /* mouse was released. set mode to normal */
                /* were we in look mode? */
                if (G_mouse.mode == CONTROL_MOUSE_MODE_LOOK) {
                    /* don't look if a spell was cast */
                    if (G_spellWasCast == FALSE) {
                        /* See if we have a valid object currently */
                        p_obj = ControlLookAt(x, y);
                        /* examine object (null is ok here) */
                        ControlExamineObject(p_obj);
                    } else
                        G_spellWasCast = FALSE;
                } else if (G_mouse.mode == CONTROL_MOUSE_MODE_MOVE) {
                    /* release mouse boundaries */
                    MouseReleaseBounds();
                }

                /* set mode to normal */
                G_mouse.mode = CONTROL_MOUSE_MODE_NORMAL;

                /* restore default mouse bitmap */
                ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_DEFAULT);

                /* check to see if there is an object in hand */
    //            if (ButtonIsMouseOverButton()==TRUE)
    //            {
    //                MouseUseDefaultBitmap();
    //            }
    //            else
    //            {
    //                p_obj=InventoryCheckObjectInMouseHand ();
    //                if (p_obj != NULL) ControlSetObjectPointer (p_obj);
    //                else MouseUseDefaultBitmap();
    //            }
            }
                break;

            case MOUSE_EVENT_HELD:
                /* short hack kludge, check for 'use button' held down */
                /* since there is no specific code to do this */
                if (BannerUseButtonIsDown()) {
                    if (InventoryCanUseItemInReadyHand())
                        InventoryUseItemInReadyHand(NULL );
                }

            case MOUSE_EVENT_DRAG:
            case MOUSE_EVENT_MOVE:

                /* what mode are we in ? */
                switch (G_mouse.mode) {
                    case CONTROL_MOUSE_MODE_NORMAL:
                        G_lookAngle = 0;
                        G_lookOffset = 0;
                        /* do nothing special */
                        if (ButtonIsMouseOverButton() == TRUE) {
                            MouseUseDefaultBitmap();
                        } else {
                            /* if we have an object in the hand set the pointer to it */
                            p_obj = InventoryCheckObjectInMouseHand();
                            if (p_obj != NULL )
                                ControlSetObjectPointer(p_obj);
                        }
                        break;

                    case CONTROL_MOUSE_MODE_LOOK:
                        /* see if we have a valid object */
                        p_obj = ControlLookAt(x, y);
                        if (p_obj != NULL ) {
                            /* set bitmap to good eye */
                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_LOOK);
                        } else {
                            /* set bitmap to bad eye */
                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_NOLOOK);
                        }

                        G_lookOffset = x - 3 - VIEW3D_HALF_WIDTH + VIEW3D_CLIP_LEFT;
                        G_lookAngle = (((VIEW3D_CLIP_RIGHT - VIEW3D_CLIP_LEFT + 1)
                                / 2) - x) << 6;
                        /* check for a spell cast command *left+right buttons* */
                        if (button
                                == 3&& G_resetNeeded==FALSE && HardFormIsOpen()==FALSE) {
                            if (ViewIsAt(x, y)) {
                                /* cast a spell */
                                /** Throw angle is based on the x of the mouse cursor. **/
                                SpellsCastSpell(NULL );
                                G_spellWasCast = TRUE;
                                G_resetNeeded = TRUE;
                            } else if (InventoryInventoryWindowIsAt(x, y)) {
                                /* try to use the selected object */
                                itemToUse = InventoryCheckItemInInventoryArea(x, y);
                                if (itemToUse != NULL ) {
                                    InventoryUseItem(itemToUse);
                                    G_spellWasCast = TRUE;
                                    G_resetNeeded = TRUE;
                                }
                            }
                        }
                        break;

                    case CONTROL_MOUSE_MODE_MOVE:
                        G_lookAngle = 0;
                        G_lookOffset = 0;
                        jumpTimer = JUMP_RESET_TIME;
                        /* find boundaries of view area */
                        vx1 = VIEW3D_UPPER_LEFT_X;
                        vy1 = VIEW3D_UPPER_LEFT_Y;
                        vx2 = VIEW3D_CLIP_RIGHT + VIEW3D_UPPER_LEFT_X
                                - VIEW3D_CLIP_LEFT + 1;
                        vy2 = VIEW3D_HEIGHT;

                        vcx = ((vx2 - vx1) / 2);
                        vcy = ((vy2 - vy1) / 2);

                        /* set mouse boundaries to view area */
                        MouseSetBounds(vx1, vy1, vx2, vy2);

                        /* change pointer to appropriate bitmap */
                        dir = ControlSetMovePointer(x, y);

                        /* should move character here based on mouse position */
                        if ((dir >= 1) && (dir <= 3))
                            PlayerTurnRight(((x - vcx) << 8) / (VIEW3D_WIDTH / 2));
                        if ((dir >= 5) && (dir <= 7))
                            PlayerTurnLeft(((vcx - x) << 8) / (VIEW3D_WIDTH / 2));
                        if ((dir == 0) || (dir == 1) || (dir == 7))
                            PlayerMoveForward(
                                    ((vcy - y) << 10) / (VIEW3D_HEIGHT / 2));
                        if ((dir >= 3) && (dir <= 5))
                            PlayerMoveBackward(
                                    ((y - vcy) << 10) / (VIEW3D_HEIGHT / 2));

                        if (button == 3) {
                            /* initiate an attack or use item */
                            if (InventoryCanUseItemInReadyHand())
                                InventoryUseItemInReadyHand(NULL );
                        }

                }
                break;

            default:
                break;
        }
    }

    DebugCompare("ControlMouseControlForGame");
    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ControlFinish                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Shuts down and cleans up control things.                              */
/*    Restores the mouse to the default bitmap.                             */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  11/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ControlFinish(T_void)
{
    DebugRoutine("ControlFinish");
    DebugCheck(G_init == TRUE);

    G_init = FALSE;

    MousePopEventHandler();

    /** free up the mouse pointer resource. **/
    ResourceUnlock(G_mouse.mouseRes);
    ResourceUnfind(G_mouse.mouseRes);

    /* Turn off the mouse */
    MouseSetDefaultBitmap(0, 0, NULL );
    MouseUseDefaultBitmap();

    DebugEnd();
}

static T_3dObject *ControlLookAt(T_word16 x, T_word16 y)
{
    T_3dObject *retvalue = NULL;
    E_inventoryType whichInv;
    DebugRoutine("ControlLookAt");

    /* so, where are we looking */
    if (ViewIsAt(x, y) && (ClientGetMode() != CLIENT_MODE_HARD_CODED_FORM)) {
        /* looking in view, check for object */
        retvalue = ViewGetXYTarget(x, y);
    } else if (InventoryInventoryWindowIsAt(x, y)) {
        whichInv = InventoryFindInventoryWindow(x, y);
        if (whichInv == INVENTORY_STORE) {
            G_autoID = TRUE;
        }
        retvalue = InventoryCheckObjectInInventoryArea(x, y);
        /* looking in inventory */
    } else if (InventoryReadyBoxIsAt(x, y)) {
        /* looking at ready hand */
        retvalue = InventoryCheckObjectInReadyHand();
    } else if (InventoryEquipmentWindowIsAt(x, y)) {
        /* looking in equipment area */
        retvalue = InventoryCheckObjectInEquipmentArea(x, y);
    }

    DebugEnd();

    return (retvalue);
}

T_void ControlPointerReset(T_void)
{
    G_mouse.type = CONTROL_MOUSE_POINTER_UNKNOWN;
    ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_DEFAULT);
}

T_void ControlSetDefaultPointer(E_controlMousePointerType type)
{
    T_byte8 stmp[30];
    T_word16 hotx, hoty;
    DebugRoutine("ControlSetDefaultPointer");

    if (G_mouse.type == type) {
        /* do nothing, return */
        DebugEnd();
        return;
    }

    else
        switch (type) {
            case CONTROL_MOUSE_POINTER_DEFAULT:
                strcpy(stmp, "UI/MOUSE/DEFAULT");
                hotx = 0;
                hoty = 0;
                break;

            case CONTROL_MOUSE_POINTER_LOOK:
                strcpy(stmp, "UI/MOUSE/LOOK");
                hotx = 6;
                hoty = 3;
                break;

            case CONTROL_MOUSE_POINTER_NOLOOK:
                strcpy(stmp, "UI/MOUSE/NOLOOK");
                hotx = 6;
                hoty = 3;
                break;

            case CONTROL_MOUSE_POINTER_MOVE_N:
                hotx = 6;
                hoty = 0;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_NE:
                hotx = 15;
                hoty = 0;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_E:
                hotx = 18;
                hoty = 3;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_SE:
                hotx = 15;
                hoty = 7;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_S:
                hotx = 6;
                hoty = 10;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_SW:
                hotx = 0;
                hoty = 7;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_W:
                hotx = 0;
                hoty = 3;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            case CONTROL_MOUSE_POINTER_MOVE_NW:
                hotx = 0;
                hoty = 0;
                sprintf(stmp, "UI/MOUSE/DIR%1.1d", type - 3);
                break;

            default:
                DebugCheck(-1); /* shouldn't get here */
        }

    ResourceUnlock(G_mouse.mouseRes);
    ResourceUnfind(G_mouse.mouseRes);
    G_mouse.bitmap = (T_bitmap *)PictureLock(stmp, &G_mouse.mouseRes);
    DebugCheck(G_mouse.bitmap != NULL );
    MouseSetDefaultBitmap(hotx, hoty, G_mouse.bitmap);
    MouseUseDefaultBitmap();
    G_mouse.type = type;

    DebugEnd();
}

/* routine sets the mouse pointer to the object picture in p_obj */
T_void ControlSetObjectPointer(T_3dObject *p_obj)
{
    DebugRoutine("ControlSetObjectPointer");
    DebugCheck(p_obj != NULL );

    G_mouse.hotx = ObjectGetPictureWidth(p_obj) / 2;
    G_mouse.hoty = ObjectGetPictureHeight(p_obj) / 2;

    MouseSetColorize(ObjectGetColorizeTable(p_obj) );
    MouseSetBitmap(G_mouse.hotx, G_mouse.hoty,
            (T_bitmap *)ObjectGetPicture(p_obj));

    DebugEnd();
}

/* LES changed: */
static T_word16 ControlSetMovePointer(T_word16 mx, T_word16 my)
{
    T_sword16 x, y, hx, dx;
    T_word16 vx1, vx2, vy1, vy2;
    T_word16 qnum = 0;

    DebugRoutine("ControlSetMovePointer");

    /* here we are going to set the mouse default bitmap to one of 8 */
    /* arrows that correspond to the location of the mouse in the */
    /* main view. */

    /* get screen boundaries */
    vx1 = VIEW3D_UPPER_LEFT_X;
    vy1 = VIEW3D_UPPER_LEFT_Y;
    vx2 = VIEW3D_CLIP_RIGHT + VIEW3D_UPPER_LEFT_X - VIEW3D_CLIP_LEFT + 1;
    vy2 = VIEW3D_HEIGHT;

    /* calulate x and y of the mouse after moving the origin to the */
    /* center of the view */
    x = mx - ((vx2 - vx1) / 2);
    y = my - ((vy2 - vy1) / 2);

    /* multiply y by aspect ratio */
    y *= ((vx2 - vx1) / (vy2 - vy1));

    /* calculate 1/2x and 2x */
    hx = x / 2;
    dx = 2 * x;

    /* check to see if we are within a small rectangle in the center */
    /* of the screen */
//    if (x >-50 && x<50 && y>-50 && y<50)
//    {
//        qnum=0;
//        hotx=6;
//        hoty=0;
//    }
    /* otherwise */
    /* figure quadrant */
    if (y <= dx && y <= -dx)
        qnum = 0;
    else if (y > -dx && y <= -hx)
        qnum = 1;
    else if (y > -hx && y <= hx)
        qnum = 2;
    else if (y > hx && y <= dx)
        qnum = 3;
    else if (y > dx && y > -dx)
        qnum = 4;
    else if (y <= -dx && y > -hx)
        qnum = 5;
    else if (y <= -hx && y > hx)
        qnum = 6;
    else
        qnum = 7;

    /* set bitmap */
    ControlSetDefaultPointer(qnum + 3);

    /* all done */

    DebugEnd();

    /* LES changed: */
    return qnum;
}

static T_void ControlExamineObject(T_3dObject *p_obj)
{
    T_word16 objtype;
    T_byte8 *desc1;
    T_byte8 *desc2;
    T_resource res;
    T_word32 size;
    T_word16 value;
    T_byte8 stmp[32];
    T_byte8 stmp2[256];

    DebugRoutine("ControlExamineObject");

    if (p_obj != NULL ) {
        /* Turn body parts into their respective head. */
        if (ObjectIsBodyPart(p_obj))
            p_obj = ObjectFindBodyPartHead(p_obj);

        objtype = ObjectGetType (p_obj);
//        MessagePrintf ("objtype=%d",objtype);

        /* check to see if object is idable */
        /* check internal resource for description of object */
        if ((ObjectIsGrabable(p_obj)
                && (StatsPlayerHasIdentified(ObjectGetType(p_obj))))||
        (G_autoID==TRUE)){
        G_autoID=FALSE;
        /* this object has been identified, use secondary */
        /* description for it */
        sprintf (stmp,"OBJDESC2/DES%05d.TXT",objtype);
        if (PictureExist(stmp))
        {
            /* show description file */
            desc1=PictureLockData (stmp,&res);
            size=ResourceGetSize(res);
            desc2=(T_byte8 *)MemAlloc(size+64);
            memcpy (desc2,desc1,size);
            desc2[size]='\0';
//                if (size > 1) desc2[size-1]='\0';
            TextCleanString (desc2);
            ControlColorizeLookString(desc2);
            if ((StoreIsOpen()==TRUE) && (StoreHouseModeIsOn()==FALSE))
            {
                /* add object value to message */
//                    if (size > 2) desc2[size-2]='\0';
                value=ObjectGetValue(p_obj);
                if (value > 0)
                {
                    StoreConvertCurrencyToString (stmp2,value);
                    MessagePrintf("^001%s ^007[%s]",desc2,stmp2);
                }
                else
                {
                    MessagePrintf("^001%s",desc2);
                }
            }
            else
            {
                MessagePrintf ("^001%s",desc2);
            }
            MemFree (desc2);
            PictureUnlockAndUnfind(res);
        }
        else
        {
            /* query server for description */
            MessageAdd ("^011Umm.. I forgot what that was.");
            /* should be a secondary description */
            DebugCheck (0);
        }
    }
    else
    {
        sprintf (stmp,"OBJDESC/DES%05d.TXT",objtype);
        if (PictureExist(stmp))
        {
            /* show description file */
            desc1=PictureLockData (stmp,&res);
            size=ResourceGetSize(res);
            desc2=(T_byte8 *)MemAlloc(size+64);
            memcpy (desc2,desc1,size);
//                if (size > 1) desc2[size-1]='\0';
            desc2[size]='\0';
            TextCleanString(desc2);
            ControlColorizeLookString(desc2);
            if ((StoreIsOpen()==TRUE) && (StoreHouseModeIsOn()==FALSE))
            {
                /* add object value to message */
//                    if (size > 2) desc2[size-2]='\0';
                value=ObjectGetValue(p_obj);
                if (value > 0)
                {
                    StoreConvertCurrencyToString (stmp2,value);
                    MessagePrintf("^007%s [%s]",desc2,stmp2);
                }
                else
                {
                    MessagePrintf("^007%s",desc2);
                }
            }
            else
            {
                /* don't display for creatures */
                if (ObjectIsCreature(p_obj)==FALSE)
                {
                    MessagePrintf ("^007%s",desc2);
                }
            }
            MemFree (desc2);
            PictureUnlockAndUnfind(res);
        }
        else
        {
            /* query server for description */
            if ((!ObjectIsCreature(p_obj)) && (!ObjectIsPlayer(p_obj)))
            MessageAdd ("^011Hmm.. I don't really know what that is.");
        }
    }

        if (p_obj) {
            /* open 'view player / monster' banner if appropriate */
            if ((ObjectIsCreature(p_obj) == TRUE)
                    && (!CreatureIsMissile(p_obj))) {
                BannerOpenForm(BANNER_FORM_LOOK);
                LookUpdateCreatureInfo(p_obj);
            } else if (ObjectIsPlayer(p_obj) == TRUE) {
//                BannerOpenForm (BANNER_FORM_LOOK);
//                LookRequestPlayerInfo (p_obj);
                MessagePrintf("^011Player ^009%s",
                        PeopleHereGetPlayerIDName(
                                ObjectGetServerId(p_obj) - 9000));
            }
        }
    } else {
        MessageAdd("^011I don't see anything special there.");
    }
    DebugEnd();
}

/* function returns last 'look angle' when look mode is activated */
T_sword16 ControlGetLookAngle(T_void)
{
//    MessagePrintf ("look angle is %d",G_lookAngle);
    return (G_lookAngle);
}

/* function returns last 'look offset' when look mode is activated */
T_sword16 ControlGetLookOffset(T_void)
{
//    MessagePrintf ("look offset is %d",G_lookOffset);
    return (G_lookOffset);
}

T_void ControlColorizeLookString(T_byte8 *string)
{
    T_word16 len, newlen;
    T_word16 i;
    T_word16 cpos;
    E_Boolean cflag = FALSE;
    T_byte8 newstring[128];
    DebugRoutine("ControlColorizeLookString");
    DebugCheck(string != NULL );

    /* scan string for color changes */
    len = strlen(string);
    newlen = len + 8;

    for (i = 0; i < len; i++) {
        if (string[i] == '[' || string[i] == ']' || string[i] == '+'
                || isdigit(string[i]))
            newlen += 4;
    }

    /* allocate new string */
    cpos = 0;
    /* make new 'colored' string */
    for (i = 0; i < len; i++) {
        if (string[i] == '[' || string[i] == ']') {
            /* change to light brown */
            newstring[cpos++] = '^';
            newstring[cpos++] = '0';
            newstring[cpos++] = '0';
            newstring[cpos++] = '7';
        } else if (string[i] == '+' || isdigit(string[i])) {
            /* change to light green */
            newstring[cpos++] = '^';
            newstring[cpos++] = '0';
            newstring[cpos++] = '0';
            newstring[cpos++] = '9';
            cflag = TRUE;
        } else if (cflag == TRUE) {
            /* revert to light blue */
            newstring[cpos++] = '^';
            newstring[cpos++] = '0';
            newstring[cpos++] = '1';
            newstring[cpos++] = '1';
            cflag = FALSE;
        }
        newstring[cpos++] = string[i];
    }
    newstring[cpos++] = '\0';

    strcpy(string, newstring);

    DebugEnd();
}

/* Routine displays control listing banner form */
/* Called from BannerOpen                       */
T_void ControlDisplayControlPage(T_void)
{
    T_TxtboxID *windowID = NULL;
    FILE *fp;
    T_byte8 tempstr[140];

    DebugRoutine("ControlDisplayControlPage");

    /* get window */
    windowID = FormGetObjID(500);

    /* clear current text in bulletin showing window */
    TxtboxSetData(windowID, "");

    /* open bulletin file */
    fp = fopen("control.txt", "r");

    if (fp != NULL ) {
        while (feof(fp) == FALSE) {
            /* get a line from the bulletin file */
            fgets(tempstr, 128, fp);

            /* strip last (newline) character */
            if (tempstr[strlen(tempstr) - 1] == '\n')
                tempstr[strlen(tempstr) - 1] = '\r';

            /* display the line */
            TxtboxAppendString(windowID, tempstr);
        }
        /* close file */
        fclose(fp);
    }

    /* move cursor to top */
    TxtboxCursTop(windowID);

    DebugEnd();
}

