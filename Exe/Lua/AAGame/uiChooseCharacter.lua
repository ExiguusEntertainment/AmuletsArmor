------------------------------------------------------------------------------
-- uiChooseCharacter is the code for putting up the form for players to
-- choose which character they want to use.  It is also at this point
-- that they can choose to create or delete characters.
--  
-- NOTE: In the original A&A code, this was called MAINUI
-- 
uiChooseCharacter = {}

local listChars;
local loadButton;
local form;

------------------------------------------------------------------------------
-- Create the choose a character form
------------------------------------------------------------------------------
uiChooseCharacter.createForm = function()
	form = Form.create(uiChooseCharacter.eventHandler);
	
	-- Graphic: Background
	form:addGraphic{id=100, x=0, y=0, picName="UI/LOGON/LOGON_BK"}
	
	-- Button: Load
	loadButton = form:addButton{id=304, x=162, y=49, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT, 
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

------------------------------------------------------------------------------
-- Handle form events here
------------------------------------------------------------------------------
uiChooseCharacter.eventHandler = function(form, event, obj)
	if (event ~= "none") then
		if (event == "select") then
			local selected = listChars:getSelection();
			stats.makeActive(selected);
			uiChooseCharacter.charSelected = selected;
			local charID = stats.getSavedCharacterIDStruct(selected);
			if (charID.status ~= "undefined") then
				uiChooseCharacter:showSelected();
			else
				uiChooseCharacter:clearPortrait();
			end
		elseif (event == "release") then
			if (obj.id == 304) then
				print("Release Load by ID")
			end
			if (obj == loadButton) then
				print("Release Load by Button")
			end
		end
	end
end

------------------------------------------------------------------------------
-- An empty slot was chosen, clear the portrait area with helpful info
------------------------------------------------------------------------------
function uiChooseCharacter:clearPortrait()
	local pic = pics.lockBitmap("UI/CREATEC/CHARINFO");
	color.update(0);
	graphics.drawPic(pic, 180, 79);
	pics.unlockAndUnfind(pic);
end

------------------------------------------------------------------------------
-- Load the list of characters and present
------------------------------------------------------------------------------
function uiChooseCharacter:updateCharacterListing()
	listChars:set("");
	listChars:cursorTop();
	chars = stats.getActiveCharacterList();
	for i=1,5 do
		listChars:append(chars[i].name .. "\r")
	end
	listChars:backspace();
	listChars:cursorTop();
end

------------------------------------------------------------------------------
-- Show the currently selected character's portrait
------------------------------------------------------------------------------
function uiChooseCharacter:showSelected()
	local c = self.charSelected;
	stats.makeActive(c);
	chardata = stats.getSavedCharacterIDStruct(c);
	if (chardata.status ~= "undefined") then
		stats.loadCharacter(c);
		stats.drawCharacterPortrait(180, 79);
	else
		-- do nothing?
--//        GrDrawRectangle(180,79,295,181,77);
	end		

	graphic.updateAllGraphics()
end

------------------------------------------------------------------------------
-- Initiliaze the uiChooseCharacter screen by creating a form
------------------------------------------------------------------------------
function uiChooseCharacter:init()
	self.createForm();
	self:updateCharacterListing()
	graphic.updateAllGraphics();
	self.charSelected = 0;
	self:showSelected();
	
	graphics.setCursor(5, 188)
	graphics.drawShadowedText(config.VERSION_TEXT, 210, 0);
end

------------------------------------------------------------------------------
-- Start is a short cut to get this started
------------------------------------------------------------------------------
uiChooseCharacter.start = function()
	uiChooseCharacter:init(uiChooseCharacter)
	form.start()
end

------------------------------------------------------------------------------
-- As the UI state machine is updated, run the form's ui
------------------------------------------------------------------------------
uiChooseCharacter.update = function()
	form:updateUI();
end

return uiChooseCharacter
