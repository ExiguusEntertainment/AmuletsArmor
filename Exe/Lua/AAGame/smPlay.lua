--local uiLoadCharacter = require "AAGame/uiLoadCharacter"
banner = require "AAGame/Banner/uiBanner"
hardformInst = require "AAGame/hardform"

--smPlay = StateMachine:create();
smPlay = {
	location = { type="hardform", place="town hall" }
}

local isprinted = false
function getAllData(t)
	local data = {}

	if isprinted then
		return
	end
		
	isprinted = true
	-- print all the attributes from t
	for k,v in pairs(t) do
		printf("%s - %s", k, v)
	end
end

function smPlayFunc()
	local result;
	
	print("SMPlayFunc")
	stats.saveCharacter(stats.getActive());
		
	mouseControl.InitForGamePlay();
	
	-- show standard palette, and stop showing black screen
	view.setPalette(0); 

	-- Startup the banner UI
    uiBanner:init();

    -- and add default rune buttons for current character class
    --SpellsInitSpells();

    -- clear the messages
    --MessageClear();

	if (stats.char.xp == 0) then
        --- Open 'journal to intro banner
         
        --EffectSoundOff();
        --NotesGotoPageID(0);
        --BannerOpenForm (BANNER_FORM_JOURNAL);
        --EffectSoundOn();
	end
		
	while (true) do	
		coroutine.yield();
		--smChooseCharacter.init();
		--printf("Try to update hardform %s", client.location.type);
		if (client.location.type == "hardform") then
			--printf("Do update hardform");
			--getAllData(hardform)
			--getAllData(hardformInst)
			hardformInst.update()
			
			-- Was Escape key pressed?
			if (keyboard.getScanCode(keyboard.KEY_SCAN_CODE_ESC)) then
				if (smPlay.location.place == "town hall") then
					-- Stop playing the game!
					-- TODO: MessageClear();
					break;
				else
					-- Otherwise, go back to town all
					-- TODO: MouseRelativeModeOff();
					-- TODO: client.GotoPlace({type="hardform", place="town hall"}); 
					smPlay.location.place = "town hall";
				end
			end
		end
-- TODO:		
--    /* Update all the regular game stuff. */
--    if (ClientIsActive())   {
--        ClientUpdate() ;
--        CreaturesCheck() ;
--        ClientSyncEnsureSend() ;
--    }

	end
		
	mouseControl.Finish();
	
	-- Done, leave the server
	return "leave_server";
end

smPlay.done = false;

smPlay.isDone = function (self)
	return self.done
end

smPlay.init = function()
	smPlay.co = coroutine.create(smPlayFunc);
end

smPlay.update = function()
	local succeeded, result = coroutine.resume(smPlay.co);
	if (not succeeded) then
		printf("smPlay.co coroutine backtrace: "..result);
		print(debug.traceback(smPlay.co));
		assert(false, "");
	else
		if (result == "leave_server") then
			smPlay.done = true;
			return "exit";
		end
	end
	return nil;
end

smPlay.finish = function()
	smPlay.co = nil;
end

smPlay.run = function()
	local result = nil;
	-- Just chosen to play the game, start up the flags and sub-state
	-- machine
	
	-- TODO: SMCPlayGameInitialize();
	-- TODO: MouseRelativeModeOff();
	client:gotoPlace("hardform", "town hall", 0);
	smPlay.location.type = "hardform";
	smPlay.location.place = "town hall"; -- was 20004
	smPlay:init();
	while (not smPlay:isDone()) do
		coroutine.yield();
		result = smPlay:update();
		if (result == "exit") then
			break;
		elseif (result == "leave_server") then
			break;
		end
	end

	smPlay:finish();
	return result;
end

return smPlay;
