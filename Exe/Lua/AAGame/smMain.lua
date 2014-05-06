require "StateMachine";
local keyboard = require "keyboard";
local smChooseCharacter = require "AAGame/smChooseCharacter"
local uiChooseCharacter = require "AAGame/uiChooseCharacter"
local uiLoadCharacter = require "AAGame/uiLoadCharacter"

--smMain = StateMachine:create();
smMain = {}

function smMainFunc()
	local result;
	
	-- Connect
	-- Nothing special here

	while (true) do	
		-- Connected, choose character
		print("SMCChooseInitialize")
		smChooseCharacter.init();
	
		keyboard.bufferOn();		
	
		while (true) do
			coroutine.yield();
			result = smChooseCharacter.update();
			if (result ~= nil) then
				break;
			end
		end	
		smChooseCharacter.finish();
		keyboard.bufferOff();
		
		-- Start the game?
		if (result == "begin") then
			-- Start the game
				-- Just chosen to play the game, start up the flags and sub-state
				-- machine
				
				-- TODO: SMCPlayGameInitialize();
				print("SMCPlayGameInitialize")
				-- TODO: MouseRelativeModeOff();
				-- TODO: ClientGotoPlace(20004, 0);
				while (true) do
					-- Do nothing here!
					coroutine.yield();
				end
		elseif (result == "leave_server") then
			break;
		end
	end
		
	-- Done, leave the server
	return "leave_server";
end

-- Start in Connect mode
--smMain.state = smMain.Connect;
smMain.done = false;

smMain.isDone = function (self)
	return self.done
end

smMain.init = function()
	smMain.co = coroutine.create(smMainFunc);
end

smMain.update = function()
	local succeeded, result = coroutine.resume(smMain.co);
	if (not succeeded) then
		printf("smMain.co coroutine backtrace: "..result);
		print(debug.traceback(smMain.co));
		assert(false, "");
	else
		if (result == "leave_server") then
			smMain.done = true;
			return "exit";
		end
	end
	return nil;
end

smMain.finish = function()
	smMain.co = nil;
end

return smMain
