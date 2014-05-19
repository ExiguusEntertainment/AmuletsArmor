require "StateMachine"
require "AAGame/stats"
require "AAGame/mouseControl"
require "AAGame/characterClasses"

--require "AAGame/smMain"

smChooseCharacter = { 
	activeStats = "Citizen", 
}

------------------------------------------------------------------------------
-- User has a current character selected and is trying to change the password.
-- Have the user enter the old password (if it exists), and if it matches
-- have the user enter a new password.
function smChangePassword()
	local chardata = stats.getSavedCharacterIDStruct(stats.getActive());
	local password = chardata.password;
	local correct = true;
	
	-- Do we need to check for the old password?
	if (#password > 0) then
		-- Okay, get the password and compare to the stored one
		local action, oldPassword = prompt.forString("Enter password", 12);
		if (action == "ok") then
			if ((#oldPassword > 0) and (oldPassword ~= password)) then
				-- Wrong password
				prompt.displayMessage("Incorrect password.");
				correct = false;
			end
		else
			-- aborted
			correct = false;
		end
	end
	
	-- Is the password correct (or blank)?
	if (correct) then
		-- Yes.  Let's try entering a new password.  Enter it twice
		local action, newPassword = prompt.forString("Enter new password", 12);
		if (action == "ok") then
			local action, newPassword2 = prompt.forString("Enter new password again", 12);
			if (action == "ok") then
				-- Do the two new passwords match?
				if (newPassword ~= newPassword2) then
					prompt.displayMessage("Passwords do not match!");
				else
					stats.setPassword(stats.getActive(), newPassword);
					stats.saveCharacter(stats.getActive());
					prompt.displayMessage("Password changed.");
				end
			end
		end
	end
end

------------------------------------------------------------------------------
-- User is choosing to load and play the current character.
-- Bring up the character summary screen and allow changing of the password.
function smLoadCharacter()
	local result = "exit";
	-- TODO: MapSetDayOffset(0);
	graphics.push();

	uiLoadCharacter.start();

	-- Let's bring up the load UI and stats of the character.
	-- We'll stay here until done.	
	while true do
		local loadresult = uiLoadCharacter.update();
		if (loadresult == "begin") then
			if ((#stats.char.password > 0) and (stats.char.password ~= uiLoadCharacter.passwordEntered)) then
				-- The password entered is incorrect.
				prompt.displayMessage("Incorrect password.  Try again.");
			else
				-- No password or correct password entered, go on
				color.fadeto({0, 0, 0});
				graphics.fillRect(0, 0, 319, 199, 0);
				result = "begin";
			end
			break;
		elseif (loadresult == "change_password") then
			-- User is opting to change the password, go there instead
			smChangePassword();
			break;
		elseif (loadresult == "exit") then
			-- Exit pressed?  Drop out of this loop.
			break;
		else
			-- Pause the state machine and let the system update
			coroutine.yield();
		end
	end
	
	uiLoadCharacter.finish();
	graphics.pop();
	
	return result;
end

------------------------------------------------------------------------------
-- User is choosing to delete the current character.
-- We don't want this to be a mistake, so if there is a password, first
-- ask for that.  Then confirm with a yes/no question.
function smDeleteCharacter()
	local chardata = stats.getSavedCharacterIDStruct(stats.getActive());
	local oldPassword = chardata.password;
	if (chardata.status ~= "undefined") then
		-- If there is a password, ask for the user to enter it.
		local action, password;
		if (#(chardata.password) > 0) then
			action, password = prompt.forString("^001Enter password to confirm ^021delete", 12);
		else
			action = "ok";
			password = "";
		end
		
		-- Is the password is correctly entered above (or blank)? 
		if (action == "ok") then
			if (password == chardata.password) then
				-- Password was entered correctly.  Let's ask to confirm.
				if (prompt.question("Are you sure you want to delete?", false) == true) then
					-- Yes.  Delete character.
				 	stats.deleteCharacter(stats.getActive());
					prompt.displayMessage("Character deleted.");
				else
					-- No.  Whew!  That was close!  Let the player know they didn't mess it up.
					prompt.displayMessage("Character NOT deleted.");
				end
			else
				-- Password is not okay
				prompt.displayMessage("Improper password. Delete command canceled.");
			end
		end
	end
end

------------------------------------------------------------------------------
-- User is choosing to create a whole new character
-- Bring up a user interface to select the type of character and enter a
-- name.
function smCreateCharacter()
	local result;
	
	-- Enter
	stats.init();
	
	graphics.push();

	uiCreateCharacter.start();		
	-- TODO: MapSetDayOffset(0x2AAA8)

	result = "";
	while true do
		coroutine.yield(nil);
		result = uiCreateCharacter.update();
if (result ~= nil) then
printf("Result=%s", result);
end		
		if (result == "exit") then
			result = "exit";
			break;
		elseif (result == "begin") then
			result = "begin";
			break;
		end
	end

	uiCreateCharacter.finish();
	
	graphics.pop();
	return result;
end

function smChooseCharacterFunc()
	local result;
	
	-- Wait for list here
	-- Start by getting the list of characters
	listOfChars = stats.getCharacterList();
	stats.setActiveCharacterList(listOfChars);

	-- Setup the UI for th list of characters and start it up
	uiChooseCharacter.start();
	result = "";
	while true do
		local redraw = 0;
		coroutine.yield(nil);
		result = uiChooseCharacter.update();
		if (result == "exit") then
			break;
		elseif (result == "create") then
			uiChooseCharacter.finish();
			result = smCreateCharacter();
			if (result == "begin") then
				return "begin";
			end
			redraw = 1;
			uiChooseCharacter.start(); 
		elseif (result == "load") then
			uiChooseCharacter.finish();
			result = smLoadCharacter();
			if (result == "begin") then
				return "begin";
			end
			uiChooseCharacter.start(); 
		elseif (result == "delete") then
			uiChooseCharacter.finish();
			-- Delete the character 
			smDeleteCharacter();
			redraw = 1;
			uiChooseCharacter.start(); 
		end
		if (redraw == 1) then
			uiChooseCharacter.finish();
			uiChooseCharacter.start(); 
		end
	end
	uiChooseCharacter.finish();
	return "exit";
end

smChooseCharacter.init = function()
	-- Start in Connect mode
--	smChooseCharacter.state = smChooseCharacter.WaitForList;
	smChooseCharacter.co = coroutine.create(smChooseCharacterFunc);
end

smChooseCharacter.update = function()
	local succeeded, result = coroutine.resume(smChooseCharacter.co);
	if (not succeeded) then
		printf("smChooseCharacter.co coroutine backtrace: "..result);
		print(debug.traceback(smChooseCharacter.co));
		assert(false, "");
	else
		if (result == "exit") then
--			smMain:set("LEAVE_SERVER");
			return "leave_server";
		elseif (result == "begin") then
--			smMain:set("BEGIN_GAME")l
			return "begin";
		end
	end
	return nil;
end

smChooseCharacter.finish = function()
	smChooseCharacter.co = nil; -- TODO: ???
end

return smChooseCharacter
