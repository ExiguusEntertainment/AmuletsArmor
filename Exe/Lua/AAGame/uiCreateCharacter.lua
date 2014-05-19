------------------------------------------------------------------------------
-- uiCreateCharacter is the code for putting up the form for players to
-- choose which character they want to use.  It is also at this point
-- that they can choose to create or delete characters.
--
-- NOTE: In the original A&A code, this was called MAINUI
--
uiCreateCharacter = {}

local form;

------------------------------------------------------------------------------
-- Create the choose a character form
------------------------------------------------------------------------------
uiCreateCharacter.createForm = function()
	form = Form.create(uiCreateCharacter.eventHandler);

	-- Graphic: Background
	form:addGraphic{id="background", x=0, y=0, picName="UI/CREATEC/CRC_BACK"}

	-- Buttons: Last, Next, Accept, and Cancel
	form:addButton{id="last", x=12, y=122, scankey1=0,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_LEFT, picName="UI/CREATEC/CRC_LST"}
	form:addButton{id="next", x=110, y=122, scankey1=0,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_RIGHT, picName="UI/CREATEC/CRC_NXT"}
	form:addButton{id="accept", x=133, y=182, scankey1=0,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ENTER, picName="UI/CREATEC/CRC_ACC"}
	form:addButton{id="cancel", x=252, y=182, scankey1=0,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ESC, picName="UI/CREATEC/CRC_CNC"}

	-- Textboxes: Description
	form:addTextbox{id="description", x=11, y=134, width=114, height=56, readonly=1,
		scrolling=0, font="FontTiny", mode="textarea", justify="center", text="Description goes here!\rNext line"};
	form:addTextbox{id="name", x=138, y=19, width=168, height=10, readonly=0,
		scrolling=0, font="FontMedium", mode="field", justify="left", text=""};
--	form:addTextbox{id="typed_desc", x=138, y=33, width=168, height=84, readonly=1,
--		scrolling=0, font="FontMedium", mode="textarea", justify="left", text="Typded description\rgoes here"};
	form:addTextbox{id="strength", x=202, y=135, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="constitution", x=202, y=148, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="accuracy", x=202, y=161, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="speed", x=275, y=135, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="magic", x=275, y=148, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="stealth", x=275, y=161, width=28, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};
	form:addTextbox{id="title", x=29, y=122, width=78, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=""};

	return form;
end

------------------------------------------------------------------------------
-- Show the text on the screen
------------------------------------------------------------------------------
uiCreateCharacter.updateText = function()
	local class = characterClassesArray[stats.char.class];
	form:find("description"):set(class.description);
	form:find("strength"):set(class.attributes.strength);
	form:find("constitution"):set(class.attributes.constitution);
	form:find("accuracy"):set(class.attributes.accuracy);
	form:find("speed"):set(class.attributes.speed);
	form:find("magic"):set(class.attributes.magic);
	form:find("stealth"):set(class.attributes.stealth);
	form:find("title"):set(class.class);
	stats.drawCharacterPortrait(10, 9);
end

------------------------------------------------------------------------------
-- Setup the stats of a character
------------------------------------------------------------------------------
uiCreateCharacter.setupStats = function(className)
	local class = characterClassesArray[className];
	stats.char.attributes = class.attributes;
	stats.char.class = className;
	uiCreateCharacter.updateText();
end

------------------------------------------------------------------------------
-- Handle form events here
------------------------------------------------------------------------------
uiCreateCharacter.eventHandler = function(form, obj, event)
	if (event ~= "none") then
		if (event == "release") then
--printf("objID: %s", obj.id);
			if (obj.id == "cancel") then
				form:setResponse("exit");
			elseif (obj.id == "last") then
				local prevClass = characterClasses.findPrevious(stats.char.class);
				uiCreateCharacter.setupStats(prevClass);
			elseif (obj.id == "next") then
				local nextClass = characterClasses.findNext(stats.char.class);
				uiCreateCharacter.setupStats(nextClass);
			elseif (obj.id == "accept") then
				local name = form:find("name"):get();
				if (#name > 0) then
					stats.char.name = name;
					-- InventorySetDefaultInventoryForClass();
                	-- ComwinInitCommunicatePage();
					form:setResponse("begin");
					-- TODO: Save character?
				else
					uiCreateCharacter.finish();
					prompt.displayMessage("Please enter a character name");
					uiCreateCharacter.start();
					uiCreateCharacter.setupStats(stats.char.class)
				end
			end
		end
	end
end

------------------------------------------------------------------------------
-- Initiliaze the uiCreateCharacter screen by creating a form
------------------------------------------------------------------------------
function uiCreateCharacter:init()
	self.createForm();
	uiCreateCharacter.setupStats(stats.char.class);
	form:draw();
end

------------------------------------------------------------------------------
-- Start is a short cut to get this started
------------------------------------------------------------------------------
uiCreateCharacter.start = function()
	uiCreateCharacter:init(uiCreateCharacter);
	form:start();
end

------------------------------------------------------------------------------
-- As the UI state machine is updated, run the form's ui
------------------------------------------------------------------------------
uiCreateCharacter.update = function()
	form:setResponse(nil);
	form:updateUI();
	local response = form:getResponse();
	return response;
end

uiCreateCharacter.finish = function()
	form:finish();
end

return uiCreateCharacter;
