uiBanner = { 
	type = "inventory",
	buttonsCreated = false,
	bannerButtons = {},
}

function uiBanner.openFormByButton()
end

function uiBanner:createBottomButtons()
    local i ;

    self.bannerButtons[1] = button.create( 5,155,"UI/3DUI/MENUBUT1.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_INVENTORY),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[2] = button.create(22,155,"UI/3DUI/MENUBUT2.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_EQUIPMENT),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[3] = button.create(39,155,"UI/3DUI/MENUBUT3.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_STATISTICS),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[4] = button.create( 5,169,"UI/3DUI/MENUBUT4.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_OPTIONS),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[5] = button.create(22,169,"UI/3DUI/MENUBUT5.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_COMMUNICATE),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[6] = button.create(39,169,"UI/3DUI/MENUBUT6.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_FINANCES),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[7] = button.create( 5,183,"UI/3DUI/MENUBUT7.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_AMMUNITION),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[8] = button.create(22,183,"UI/3DUI/MENUBUT8.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_NOTES),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[9] = button.create(39,183,"UI/3DUI/MENUBUT9.png","toggle",keyboard.scankeys.KEY_SCAN_CODE_ALT, keymap.getMap(keymap.mapping.KEYMAP_JOURNAL),uiBanner.openFormByButton,uiBanner.closeFormByButton);
    self.bannerButtons[10] = button.create(240,181,"UI/3DUI/CSTBUT2.png" ,"normal",0,keymap.getMap(keymap.mapping.KEYMAP_CAST_SPELL),SpellsCastSpell,nil);
    self.bannerButtons[11] = button.create( 93,186,"UI/3DUI/MUSEBUT2.png" ,"normal",0,keymap.getMap(KEYMAP_USE),nil,nil);

    for i=1,9 do
        self.bannerButtons[i].setData(i);
    end

	uiBanner.buttonsCreated = true;

    -- replace rune buttons if necessary
    for i=1,9 do
    	if (stats.runeIsAvailable(i)) then
    		banner:remoteSpellButton(i);
    		banner:addSpellButton(i);
    	end
    end
    
    button.redrawAllButtons();
end

function uiBanner:init()
	local background = pics.lockBitmap("UI/3DUI/MAINBACK");
	graphics.drawPic(background, 0, 0);
	pics.unlockAndUnfind(background);
	
    -- draw a black box over button areas to fix slight graphic error
    -- when re-drawing a 'pushed' button.. 
	graphics.fillRect(4, 154, 55, 196, 0);

    -- create button controls
    uiBanner:createBottomButtons();

    -- initialize potions
    --PotionInit();

    -- draw status bars
    --BannerStatusBarInit();

    -- redraw any open menus
    --if (G_bannerIsOpen == TRUE)
    --    BannerOpenForm(self.type);

    -- update status bar
    --BannerStatusBarUpdate();

    -- draw potions
    --PotionUpdate();

    -- draw mana display
    --BannerUpdateManaDisplay();

    -- redraw active runes
    --SpellsDrawRuneBox();

    -- draw ready area
    --InventoryDrawReadyArea();
end

function uiBanner:update()
    -- static T_word32 lastupdate = 0;
    -- T_word32 time = 0;
    -- T_word32 delta = 0;

--    time = TickerGet();
--    delta = time - lastupdate;
--
--    if (delta > 8) {
--        PotionUpdate();
--        BannerUpdateManaDisplay();
--        lastupdate = TickerGet();
--    }
end
