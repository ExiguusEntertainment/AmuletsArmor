--local uiLoadCharacter = require "AAGame/uiLoadCharacter"

--smPlay = StateMachine:create();
smPlay = {}

function smPlayFunc()
	local result;
	
		print("SMPlayFunc")
		stats.saveCharacter(stats.getActive());
		
	-- Connect
	-- Nothing special here

	while (true) do	
		-- Connected, choose character
		smChooseCharacter.init();
	end
		
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
	-- TODO: ClientGotoPlace(20004, 0);
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
