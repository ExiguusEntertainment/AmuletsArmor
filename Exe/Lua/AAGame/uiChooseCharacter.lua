------------------------------------------------------------------------------
-- uiChooseCharacter is the code for putting up the form for players to
-- choose which character they want to use.  It is also at this point
-- that they can choose to create or delete characters.
--  
-- NOTE: In the original A&A code, this was called MAINUI
-- 
uiChooseCharacter = {}

local listChars;
local form;

uiChooseCharacter.createForm = function()
	form = Form.create(uiChooseCharacter.eventHandler);
	
	-- Graphic: Background
	form:addGraphic{id=100, x=0, y=0, picName="UI/LOGON/LOGON_BK"}
	
	-- Button: Load
	form:addButton{id=304, x=162, y=49, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT, 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_L, picName="UI/LOGON/LOGON_B3"}
		
	-- Button: Create
	form:addButton{id=305, x=200, y=49, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT, 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_C, picName="UI/LOGON/LOGON_B4"}
		
	-- Button: Delete
	form:addButton{id=306, x=238, y=49, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT, 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_D, picName="UI/LOGON/LOGON_B5"}
		
	-- Button: Exit
	form:addButton{id=307, x=276, y=49, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT, 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_E, picName="UI/LOGON/LOGON_B6"}
	
	-- Textbox: List of Characters (readonly scrolling list of text)
	listChars = form:addTextbox{id=502, x=162, y=4, width=153, height=44, readonly=1, 
		scrolling=1, font="FontMedium", mode="selection"};
		
	return form;
end

uiChooseCharacter.eventHandler = function(self, event, obj)
	print("uiChooseCharacter.eventHandler")
	print("objType = " .. obj.type)
	print("event = " .. event)
	print("objID = " .. obj.id)
	if (event == "select") then
		print("Selection = ")
		print(listChars:getSelection());
	end
end

uiChooseCharacter.updateCharacterListing = function()
print("update char listing")
--print(inspect(listChars))
	listChars:set("");
	listChars:cursorTop();
	chars = stats.getActiveCharacterList();
	for i=1,5 do
		listChars:append(chars[i].name .. "\r")
	end
	listChars:backspace();
	listChars:cursorTop();
end

uiChooseCharacter.showSelected = function()
--
--    /* character selected changed */
--    TxtboxID=FormGetObjID (MAINUI_CHARACTER_LIST);
--    DebugCheck (TxtboxID != NULL);
--    TxtboxCursSetRow (TxtboxID,G_characterSelected);
	--	local c = uiChooseCharacter.charSelected;
	--	listChars:cursorSetRow(c);
	--	stats.makeActive(c);
--	local chardata = stats.GetSavedCharacterIDStruct(c);
--
--    /* LES:  Make the selected item the active one. */
--    StatsMakeActive(G_characterSelected);
--    chardata=StatsGetSavedCharacterIDStruct (G_characterSelected);
--    if (chardata->status < CHARACTER_STATUS_UNDEFINED)
--    {
--        StatsLoadCharacter(G_characterSelected);
--        StatsDrawCharacterPortrait(180,79);
--    }
--    else
--    {
--//        GrDrawRectangle(180,79,295,181,77);
--    }
--
	graphic.updateAllGraphics()
end

function uiChooseCharacter:init()
print("uiChooseCharacter.init")
print(uiChooseCharacter.eventHandler)
	self.createForm();
	self.updateCharacterListing()
	graphic.updateAllGraphics();
	self.charSelected = 0;
	self.showSelected();
	
	graphics.setCursor(5, 188)
	graphics.drawShadowedText(config.VERSION_TEXT, 210, 0);
end

uiChooseCharacter.start = function()
print("uiChooseCharacter.start")
	uiChooseCharacter:init(uiChooseCharacter)
	form.start()
end

uiChooseCharacter.update = function()
	form:updateUI();
end

return uiChooseCharacter
