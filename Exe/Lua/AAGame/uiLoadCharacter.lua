------------------------------------------------------------------------------
-- uiLoadCharacter is the code for putting up the form for players to
-- choose which character they want to use.  It is also at this point
-- that they can choose to create or delete characters.
--
-- NOTE: In the original A&A code, this was called MAINUI
--
uiLoadCharacter = {}

local form;

------------------------------------------------------------------------------
-- Create the choose a character form
------------------------------------------------------------------------------
uiLoadCharacter.createForm = function()
	local t;
	local char;
	
	stats.set();
	char = stats.get();
print(inspect(char));
	form = Form.create(uiLoadCharacter.eventHandler);

	-- Graphic: Background
	form:addGraphic{id="background", x=36, y=16, picName="UI/LOADC/LOADC_BK"}

	-- Button: Begin
	form:addButton{id="begin", x=45, y=167, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_B, picName="UI/LOADC/LOADC_B1"}

	-- Button: Set password
	form:addButton{id="set_password", x=122, y=167, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_S, picName="UI/LOADC/LOADC_B2"}

	-- Button: Exit
	form:addButton{id="exit", x=199, y=167, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT,
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_E, picName="UI/LOADC/LOADC_B3"}

	-- Textbox: Name
	form:addTextbox{id="name", x=45, y=24, width=226, height=10, readonly=1,
		scrolling=0, font="FontMedium", mode="field", justify="center", text=char.name};
	uiLoadCharacter.passwordField = form:addTextbox{id="password", x=104, y=153, width=167, height=10, readonly=0,
		scrolling=0, font="FontMedium", mode="field"};
	form:addTextbox{id="level", x=255, y=39, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.level};
	form:addTextbox{id="str", x=255, y=49, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.strength};
	form:addTextbox{id="con", x=255, y=59, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.constitution};
	form:addTextbox{id="acc", x=255, y=69, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.accuracy };
	form:addTextbox{id="stl", x=255, y=79, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.stealth};
	form:addTextbox{id="mag", x=255, y=89, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.magic};
	form:addTextbox{id="spd", x=255, y=99, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.speed};
	form:addTextbox{id="class", x=196, y=109, width=75, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.class};
	form:addTextbox{id="title", x=196, y=119, width=75, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.title};
	form:addTextbox{id="xp", x=219, y=129, width=52, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.xp};
	form:addTextbox{id="xpNeeded", x=219, y=139, width=52, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text=char.xpNeeded};

	return form;
end

------------------------------------------------------------------------------
-- Handle form events here
------------------------------------------------------------------------------
uiLoadCharacter.eventHandler = function(form, obj, event)
--printf("uiLoadCharacter.eventHandler %s %s %s", form, event, obj);
	if (event ~= "none") then
		if (event == "release") then
			if (obj.id == "exit") then
				form:setResponse("exit");
			elseif (obj.id == "set_password") then
				form:setResponse("change_password");
			elseif (obj.id == "begin") then
                -- ComwinInitCommunicatePage();
                uiLoadCharacter.passwordEntered =  uiLoadCharacter.passwordField:get();
				form:setResponse("begin");
			end
		end
	end
end

------------------------------------------------------------------------------
-- Initiliaze the uiLoadCharacter screen by creating a form
------------------------------------------------------------------------------
function uiLoadCharacter:init()
	graphics.shadeRect(50, 26, 295, 196, 125);
	self.createForm();
	form:draw();
	graphics.fillRect(171, 40, 234, 105, uiColors.inventoryBaseColor);

    -- PlayerDraw (171,40);
	stats.drawCharacterPortrait(48, 42);
end

------------------------------------------------------------------------------
-- Start is a short cut to get this started
------------------------------------------------------------------------------
uiLoadCharacter.start = function()
	uiLoadCharacter:init(uiLoadCharacter)
	form:start()
end

------------------------------------------------------------------------------
-- As the UI state machine is updated, run the form's ui
------------------------------------------------------------------------------
uiLoadCharacter.update = function()
	form:setResponse(nil);
	form:updateUI();
	return form:getResponse();
end

uiLoadCharacter.finish = function()
	form:finish();
end

return uiLoadCharacter
