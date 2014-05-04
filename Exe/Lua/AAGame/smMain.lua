require "StateMachine";
local keyboard = require "keyboard";
local smChooseCharacter = require "AAGame/smChooseCharacter"
local uiChooseCharacter = require "AAGame/uiChooseCharacter"
local uiLoadCharacter = require "AAGame/uiLoadCharacter"

smMain = StateMachine:create();

-- All games start by going to the connect to server screen
-- In this current version, we do not have a server list to select.
smMain.Connect = function(self, event)
	if (event == "enter") then
		-- Connect to server here
		-- For now, we connect automatically (done outside of the game)
		self:set("CONNECT_COMPLETE");
	elseif (event == "check") then
		self:check("CONNECT_EXIT", smMain.ExitGame);
		self:check("CONNECT_COMPLETE", smMain.ChooseCharacter);
	end
end

-- At this point, the player has connected to the server
-- and must choose a character.  Do the choose a character screen.
smMain.ChooseCharacter = function (self, event)
	if (event == "enter") then
		self:clear("BEGIN_GAME");
		self:clear("LEAVE_SERVER");
		
		-- TODO: SMCChooseInitialize();
		print("SMCChooseInitialize")
		smChooseCharacter.init();

		keyboard.bufferOn();		
	elseif (event == "check") then		
		self:check("LEAVE_SERVER", smMain.LeaveServer);
		self:check("BEGIN_GAME", smMain.PlayGame);
		self:check("DROPPED", smMain.Disconnected);
	elseif (event == "update") then
		-- Sit here letting the player choose a character
		smChooseCharacter.update();
	elseif (event == "exit") then
		smChooseCharacter.finish();
		keyboard.bufferOff();
	end
end

-- The player is now playing the game (going into that server's town or
-- in a level).
smMain.PlayGame = function (self, event)
	if (event == "enter") then
		-- Just chosen to play the game, start up the flags and sub-state
		-- machine
		self:clear("DROPPED");
		self:clear("END_GAME");
		
		-- TODO: SMCPlayGameInitialize();
		print("SMCPlayGameInitialize")
		-- TODO: MouseRelativeModeOff();
		-- TODO: ClientGotoPlace(20004, 0);
	elseif (event == "check") then
		self:check("END_GAME", smMain.LogoffCharacter);
		self:check("DROPPED", smMain.Disconnected);
	elseif (event == "update") then
	end
end

-- The player is leaving the current character (and leaving town/game)
smMain.LogoffCharacter = function (self, event)
	if (event == "enter") then
		self:clear("DROPPED");
		self:clear("LOGOFF_COMPLETE");
	elseif (event == "update") then
		-- For now, there is no logoff procedure
		self:set("LOGOFF_COMPLETE");
	elseif (event == "exit") then
		-- SMCLogoffFinish()
		printf("SMCLogoffFinish");
	end
end

-- The player is now disconnected from the server harshly.  Report the 
-- dropped status.
smMain.Disconnected = function (self, event)
	if (event == "enter") then
		self:clear("DISCONNECT_COMPLETE");
	elseif (event == "check") then
		self:check("DISCONNECT_COMPLETE", smMain.Connect);
	elseif (event == "update") then
	elseif (event == "exit") then
		-- TODO: SMCDisconnectFinish()
		print("SMCDisconnectFinish")
	end
end

-- At this point, the player is leaving the server.  Show whatever
-- status needs to be shown (if any)
smMain.LeaveServer = function (self, event)
	if (event == "enter") then
		self:clear("LEAVE_SERVER_COMPLETE");
		
		-- For now, do nothing, we don't really leave the server in this code
		-- Just take us out of the game.  There is NO connect screen currently.
		self:set("END_GAME");	
	elseif (event == "check") then
		self:check("LEAVE_SERVER_COMPLETE", smMain.Connect);
		self:check("END_GAME", smMain.ExitGame);
	elseif (event == "update") then
	end
end

-- The player is now requesting to exit the game.  Go through this state
-- and clean up.
smMain.ExitGame = function (self, event)
	if (event == "update") then
		smMain.done = true
	end
end

-- Start in Connect mode
smMain.state = smMain.Connect;
smMain.done = false;

smMain.isDone = function (self)
	return self.done
end

return smMain
