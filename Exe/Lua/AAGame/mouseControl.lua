mouseControl = {
}

local init = false; 
local mode = "normal";
local buttonpushed = false;
local hotspot = {x = 0, y = 0};
local mouseRes = nil;
local bitmap = nil;
local type = "default";

------------------------------------------------------------------------------
-- This routine does the standard mouse update for the ui.
------------------------------------------------------------------------------
function mouseControl.controlUI(event, x, y, buttons)
	-- Pass the mouse events on to the ui components
	ui.mouseEvent(event, x, y, buttons)
end

function mouseControl.controlForGame(event, x, y, buttons)
-- printf("mouseControl.controlForGame %s %s %s %s", event, x, y, buttons);
 
--    T_3dObject *p_obj = NULL;
--    T_word16 vx1, vy1, vx2, vy2, vcx, vcy;
--    T_word16 dir;
--    T_inventoryItemStruct *itemToUse;
--    static T_word16 jumpTimer = 0;
--    static T_word32 last_update = 0;
--    T_word32 time_elapsed;

    -- Do the standard ui stuff.
    --ControlMouseControlForUI(event, x, y, button);
    mouseControl.controlUI(event, x, y, buttons);

--    if (!EscapeMenuIsOpen()) {
--
--    //    if (event!=MOUSE_EVENT_IDLE)
--    //    {
--    //        MessagePrintf("Mouse event:%d\n",event);
--    //    }
--
--        /* update delta timer */
--        /* get time elapsed */
--        time_elapsed = TickerGet() - last_update;
--        last_update = TickerGet();
--        jumpTimer -= time_elapsed;
--        if (jumpTimer > JUMP_RESET_TIME)
--            jumpTimer = 0;
--
--        switch (event) {
--            case MOUSE_EVENT_IDLE:
--                break;
--
--            case MOUSE_EVENT_START:
--                G_resetNeeded = FALSE;
--                /* which button was pressed ? */
--                if (button == MOUSE_BUTTON_LEFT) {
--                    /* check for a jump */
--                    if (jumpTimer > 0) {
--                        /* we're jumping */
--                        jumpTimer = JUMP_RESET_TIME;
--                        PlayerJump(StatsGetJumpPower());
--                    }
--
--                    /* ok, we are going to move/get/activate/throw, ect */
--                    /* where are we at? */
--                    if (ViewIsAt(x, y)) {
--                        if (ClientGetMode() == CLIENT_MODE_HARD_CODED_FORM) {
--                            HardFormHandleMouse(event, x, y, button);
--                        } else {
--                            /* is an object in mouse hand currently? */
--                            if (InventoryObjectIsInMouseHand()) {
--                                /* throw object into world */
--                                InventoryThrowObjectIntoWorld(x, y);
--                            } else {
--                                /* no object currently in hand.  Try to grab one. */
--                                p_obj = ViewGetXYTarget(x, y);
--                                if (p_obj != NULL ) {
--                                    /* hey, there is one there! */
--                                    /* Ask the server politely to give me the object */
--                                    /* see if it's a monster or another player */
--                                    if (ObjectIsGrabable(p_obj)) {
--                                        if (InventoryCanTakeItem(p_obj)) {
--                                            ClientRequestTake(p_obj, KeyboardGetScanCode(KEY_SCAN_CODE_ALT));
--                                        } else {
--                                            /* some routine here to take object */
--                                            /* as well as download description */
--                                            /* of object at same time */
--                                        }
--                                    } else {
--                                        G_mouse.mode = CONTROL_MOUSE_MODE_MOVE;
--                                        ControlSetMovePointer(x, y);
--                                    }
--                                } else {
--                                    /* no object there, set to move */
--                                    G_mouse.mode = CONTROL_MOUSE_MODE_MOVE;
--                                    /* set move pointer */
--                                    ControlSetMovePointer(x, y);
--                                }
--                            }
--                        }
--                    } else if (InventoryInventoryWindowIsAt(x, y)) {
--                        /* place item in inventory or get item */
--                        InventoryTransferToInventory(x, y);
--                    } else if (InventoryReadyBoxIsAt(x, y)) {
--                        /* swap items with ready box */
--                        InventoryTransferToReadyHand();
--                    } else if (InventoryEquipmentWindowIsAt(x, y)) {
--                        /* try to equip item */
--                        InventoryTransferToEquipment(x, y);
--                    } else if (NotesJournalWindowIsAt(x, y)) {
--                        /* add or remove a journal page */
--                        NotesTransferJournalPage();
--                    } else if (BannerFinancesWindowIsAt(x, y)) {
--                        InventoryTransferToFinances();
--                    } else if (BannerAmmoWindowIsAt(x, y)) {
--                        InventoryTransferToAmmo();
--                    }
--
--                    /* update mouse pointer */
--                    if (G_mouse.mode == CONTROL_MOUSE_MODE_NORMAL) {
--                        if (InventoryObjectIsInMouseHand()) {
--                            ControlSetObjectPointer(
--                                    InventoryCheckObjectInMouseHand());
--                        } else {
--                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_DEFAULT);
--                            MouseUseDefaultBitmap();
--                        }
--                    }
--                } else if (button == MOUSE_BUTTON_RIGHT) {
--                    G_mouse.mode = CONTROL_MOUSE_MODE_LOOK;
--                    /* see if we have a valid object */
--                    p_obj = ControlLookAt(x, y);
--                    if (p_obj != NULL ) {
--                        /* set bitmap to good eye */
--                        ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_LOOK);
--                    } else {
--                        /* set bitmap to bad eye */
--                        ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_NOLOOK);
--                    }
--                }
--                break;
--
--            case MOUSE_EVENT_END: {
--                /* reset use button for next attack */
--                InventoryResetUse();
--
--                G_resetNeeded = FALSE;
--                G_lookAngle = 0;
--                G_lookOffset = 0;
--                /* mouse was released. set mode to normal */
--                /* were we in look mode? */
--                if (G_mouse.mode == CONTROL_MOUSE_MODE_LOOK) {
--                    /* don't look if a spell was cast */
--                    if (G_spellWasCast == FALSE) {
--                        /* See if we have a valid object currently */
--                        p_obj = ControlLookAt(x, y);
--                        /* examine object (null is ok here) */
--                        ControlExamineObject(p_obj);
--                    } else
--                        G_spellWasCast = FALSE;
--                } else if (G_mouse.mode == CONTROL_MOUSE_MODE_MOVE) {
--                    /* release mouse boundaries */
--                    MouseReleaseBounds();
--                }
--
--                /* set mode to normal */
--                G_mouse.mode = CONTROL_MOUSE_MODE_NORMAL;
--
--                /* restore default mouse bitmap */
--                ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_DEFAULT);
--
--                /* check to see if there is an object in hand */
--    //            if (ButtonIsMouseOverButton()==TRUE)
--    //            {
--    //                MouseUseDefaultBitmap();
--    //            }
--    //            else
--    //            {
--    //                p_obj=InventoryCheckObjectInMouseHand ();
--    //                if (p_obj != NULL) ControlSetObjectPointer (p_obj);
--    //                else MouseUseDefaultBitmap();
--    //            }
--            }
--                break;
--
--            case MOUSE_EVENT_HELD:
--                /* short hack kludge, check for 'use button' held down */
--                /* since there is no specific code to do this */
--                if (BannerUseButtonIsDown()) {
--                    if (InventoryCanUseItemInReadyHand())
--                        InventoryUseItemInReadyHand(NULL );
--                }
--
--            case MOUSE_EVENT_DRAG:
--            case MOUSE_EVENT_MOVE:
--
--                /* what mode are we in ? */
--                switch (G_mouse.mode) {
--                    case CONTROL_MOUSE_MODE_NORMAL:
--                        G_lookAngle = 0;
--                        G_lookOffset = 0;
--                        /* do nothing special */
--                        if (ButtonIsMouseOverButton() == TRUE) {
--                            MouseUseDefaultBitmap();
--                        } else {
--                            /* if we have an object in the hand set the pointer to it */
--                            p_obj = InventoryCheckObjectInMouseHand();
--                            if (p_obj != NULL )
--                                ControlSetObjectPointer(p_obj);
--                        }
--                        break;
--
--                    case CONTROL_MOUSE_MODE_LOOK:
--                        /* see if we have a valid object */
--                        p_obj = ControlLookAt(x, y);
--                        if (p_obj != NULL ) {
--                            /* set bitmap to good eye */
--                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_LOOK);
--                        } else {
--                            /* set bitmap to bad eye */
--                            ControlSetDefaultPointer(CONTROL_MOUSE_POINTER_NOLOOK);
--                        }
--
--                        G_lookOffset = x - 3 - VIEW3D_HALF_WIDTH + VIEW3D_CLIP_LEFT;
--                        G_lookAngle = (((VIEW3D_CLIP_RIGHT - VIEW3D_CLIP_LEFT + 1)
--                                / 2) - x) << 6;
--                        /* check for a spell cast command *left+right buttons* */
--                        if (button
--                                == 3&& G_resetNeeded==FALSE && HardFormIsOpen()==FALSE) {
--                            if (ViewIsAt(x, y)) {
--                                /* cast a spell */
--                                /** Throw angle is based on the x of the mouse cursor. **/
--                                SpellsCastSpell(NULL );
--                                G_spellWasCast = TRUE;
--                                G_resetNeeded = TRUE;
--                            } else if (InventoryInventoryWindowIsAt(x, y)) {
--                                /* try to use the selected object */
--                                itemToUse = InventoryCheckItemInInventoryArea(x, y);
--                                if (itemToUse != NULL ) {
--                                    InventoryUseItem(itemToUse);
--                                    G_spellWasCast = TRUE;
--                                    G_resetNeeded = TRUE;
--                                }
--                            }
--                        }
--                        break;
--
--                    case CONTROL_MOUSE_MODE_MOVE:
--                        G_lookAngle = 0;
--                        G_lookOffset = 0;
--                        jumpTimer = JUMP_RESET_TIME;
--                        /* find boundaries of view area */
--                        vx1 = VIEW3D_UPPER_LEFT_X;
--                        vy1 = VIEW3D_UPPER_LEFT_Y;
--                        vx2 = VIEW3D_CLIP_RIGHT + VIEW3D_UPPER_LEFT_X
--                                - VIEW3D_CLIP_LEFT + 1;
--                        vy2 = VIEW3D_HEIGHT;
--
--                        vcx = ((vx2 - vx1) / 2);
--                        vcy = ((vy2 - vy1) / 2);
--
--                        /* set mouse boundaries to view area */
--                        MouseSetBounds(vx1, vy1, vx2, vy2);
--
--                        /* change pointer to appropriate bitmap */
--                        dir = ControlSetMovePointer(x, y);
--
--                        /* should move character here based on mouse position */
--                        if ((dir >= 1) && (dir <= 3))
--                            PlayerTurnRight(((x - vcx) << 8) / (VIEW3D_WIDTH / 2));
--                        if ((dir >= 5) && (dir <= 7))
--                            PlayerTurnLeft(((vcx - x) << 8) / (VIEW3D_WIDTH / 2));
--
--                        // Can move only when not dead
--                        if (!ClientIsDead())  {
--                            if ((dir == 0) || (dir == 1) || (dir == 7))
--                                PlayerMoveForward(
--                                        ((vcy - y) << 10) / (VIEW3D_HEIGHT / 2));
--                            if ((dir >= 3) && (dir <= 5))
--                                PlayerMoveBackward(
--                                        ((y - vcy) << 10) / (VIEW3D_HEIGHT / 2));
--                        }
--
--                        if (button == 3) {
--                            /* initiate an attack or use item */
--                            if (InventoryCanUseItemInReadyHand())
--                                InventoryUseItemInReadyHand(NULL );
--                        }
--
--                }
--                break;
--
--            default:
--                break;
--        }
--    }
end

------------------------------------------------------------------------------
-- Sets up the mouse (& keyboard?) for control in ui situations.
-- Initializes all necessary variables and default mouse bitmap, etc.
------------------------------------------------------------------------------
function mouseControl.InitForJustUI()
print("mouseControl.InitForJustUI");
	-- Old controller?
	if (init) then
		mouse.popEventHandler()
	end
	
	init = true
	hotspot = {x=0, y=0}
	mode = "normal"
	bitmap = pics.lockBitmap("UI/MOUSE/DEFAULT");
	assert(bitmap ~= nil)
	mouse.setDefaultBitmap(bitmap, hotspot)
	mouse.useDefaultBitmap()
	
	mouse.pushEventHandler(mouseControl.controlUI)
end

------------------------------------------------------------------------------
-- Sets up the mouse (& keyboard?) for control in game situations.
-- Initializes all necessary variables and default mouse bitmap, etc.
------------------------------------------------------------------------------
function mouseControl.InitForGamePlay()
	-- Old controller?
	if (init) then
		mouse.popEventHandler()
	end
	
	init = true

    -- Initialize the mouse.
    buttonpushed = false;
	hotspot = {x=0, y=0}
	mode = "normal"
	bitmap = pics.lockBitmap("UI/MOUSE/DEFAULT");
	assert(bitmap ~= nil)
	mouse.setDefaultBitmap(bitmap, hotspot)
	mouse.useDefaultBitmap()

    -- set up control handler
    mouse.pushEventHandler(mouseControl.controlForGame);
end

------------------------------------------------------------------------------
-- Shuts down and cleans up control things.
-- Restores the mouse to the default bitmap.
------------------------------------------------------------------------------
function mouseControl.Finish()
print("mouseControl.Finish");
	assert(init == true)
	init = false

	-- No longer need the bitmap	
	pics.unlockAndUnfind(bitmap)
	mouse.setDefaultBitmap(nil, {x=0, y=0})
	mouse.useDefaultBitmap()
	
	-- No longer handling mouse events (at least for this ui)
	mouse.popEventHandler()
end
