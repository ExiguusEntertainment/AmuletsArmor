require "StateMachine";
require "keyboard";

smmain = StateMachine:create();
smmain.state = "Connect";

-- All games start by going to the connect to server screen
-- In this current version, we do not have a server list to select.
smmain.state_funcs["Connect"] = function(self, event)
	if (event == "enter") then
		-- Connect to server here
		-- For now, we connect automatically (done outside of the game)
		self:set("CONNECT_COMPLETE");
	elseif (event == "check") then
		self:check("CONNECT_EXIT", "ExitGame");
		self:check("CONNECT_COMPLETE", "ChooseCharacter");
	end
end

-- At this point, the player has connected to the server
-- and must choose a character.  Do the choose a character screen.
smmain.state_funcs["ChooseCharacter"] = function (self, event)
	if (event == "enter") then
		self:clear("BEGIN_GAME");
		self:clear("LEAVE_SERVER");
		
		-- TODO: SMCChooseInitialize();
		print("SMCChooseInitialize")

		keyboard.bufferOn();		
	elseif (event == "check") then		
		self:check("LEAVE_SERVER", "LeaveServer");
		self:check("BEGIN_GAME", "PlayGame");
		self:check("DROPPED", "Disconnected");
	elseif (event == "update") then
		-- TODO: SMCChooseUpdate();
		print("SMCChooseUpdate")
	elseif (event == "exit") then
		-- TODO: SMCChooseFinish();
		
		keyboard.bufferOff();
	end
end

-- The player is now playing the game (going into that server's town or
-- in a level).
smmain.state_funcs["PlayGame"] = function (self, event)
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
		self:check("END_GAME", "LogoffCharacter");
		self:check("DROPPED", "Disconnected");
	elseif (event == "update") then
	end
end

-- The player is leaving the current character (and leaving town/game)
smmain.state_funcs["LogoffCharacter"] = function (self, event)
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
smmain.state_funcs["Disconnected"] = function (self, event)
	if (event == "enter") then
		self:clear("DISCONNECT_COMPLETE");
	elseif (event == "check") then
		self:check("DISCONNECT_COMPLETE", "Connect");
	elseif (event == "update") then
	elseif (event == "exit") then
		-- TODO: SMCDisconnectFinish()
		print("SMCDisconnectFinish")
	end
end

-- At this point, the player is leaving the server.  Show whatever
-- status needs to be shown (if any)
smmain.state_funcs["LeaveServer"] = function (self, event)
	if (event == "enter") then
		self:clear("LEAVE_SERVER_COMPLETE");
		
		-- For now, do nothing, we don't really leave the server in this code
		-- Just take us out of the game.  There is NO connect screen currently.
		self:set("END_GAME");	
	elseif (event == "check") then
		self:check("LEAVE_SERVER_COMPLETE", "Connect");
		self:check("END_GAME", "ExitGame");
	elseif (event == "update") then
	end
end

-- The player is now requesting to exit the game.  Go through this state
-- and clean up.
smmain.state_funcs["ExitGame"] = function (self, event)
	if (event == "update") then
	end
end

return smmain
