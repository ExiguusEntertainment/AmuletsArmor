require "StateMachine"
require "AAGame/stats"
require "AAGame/mouseControl"
--require "AAGame/smMain"

smChooseCharacter = StateMachine:create();

-- The player has just connected to the server.  Let's get a list of
-- of characters (and wait for the update).
smChooseCharacter.WaitForList = function(self, event)
	if (event == "enter") then
		self:clear("ENTER_COMPLETE");
		-- In this current verison, there is no server.  Just fetch
		-- the list from the hard disk.
		listOfChars = stats.getCharacterList();
		stats.setActiveCharacterList(listOfChars);
		
		-- and immediately say we are complete
		self:set("ENTER_COMPLETE");
	elseif (event == "check") then
		self:check("ENTER_COMPLETE", smChooseCharacter.Choices);
	end
end

-- Waiting for choice of actions
smChooseCharacter.Choices = function(self, event)
	if (event == "enter") then
		self:clear({"EXIT", "CREATE", "LOAD", "DELETE", "REDRAW"});
		uiChooseCharacter.start();
	elseif (event == "check") then
		self:check("EXIT", smChooseCharacter.Exit);
		self:check("CREATE", smChooseCharacter.Create);
		self:check("LOAD", smChooseCharacter.Load);
		self:check("DELETE", smChooseCharacter.Delete);
		self:check("REDRAW", smChooseCharacter.Choices);
	elseif (event == "update") then
		uiChooseCharacter.update();
	elseif (event == "exit") then
		uiChooseCharacter.finish();
	end
end

-- Choosing to create a new character
smChooseCharacter.Create = function(self, event)
	if (event == "enter") then
		self:clear({"CREATE_COMPLETE", "CREATE_ABORT"})
		-- TODO: StatsCreateCharacterUIStart()
		-- TODO: MapSetDayOffset(0x2AAA8)
	elseif (event == "check") then
		self:check("CREATE_COMPLETE", smChooseCharacter.RequestCreate);
		self:check("CREATE_ABORT", smChooseCharacter.Choices);
	elseif (event == "update") then
		-- TODO: StatsCreateCharacterUIUpdate()
	elseif (event == "exit") then
		-- TODO: StatsCreateCharadcterUIEnd()
	end
end

-- Character creation is started, see if the server agrees after
-- entering a new character and password
smChooseCharacter.RequestCreate = function(self, event)
	if (event == "enter") then
		self:clear({"CREATE_STATUS_OK", "CREATE_STATUS_ERROR"})
		-- TODO: StatsSetPassword(StatsGetActive(), "")
	elseif (event == "check") then
		self:check("CREATE_STATUS_OK", smChooseCharacter.CreateUpload);
		self:check("CREATE_STATUS_ERROR", smChooseCharacter.Choices);
		self:check("CREATE_ABORT", smChooseCharacter.Choices);
	elseif (event == "update") then
		-- With no server, we immediately save the character
		-- and go on
		-- TODO: StatsSaveCharacter(StatsGetActive())
		self:set("CREATE_STATUS_OK");
	end
end

-- Character load/download is requested
smChooseCharacter.Load = function(self, event)
printf("STATE: smChooseCharacter.Load");
	if (event == "enter") then
		self:clear({"LOAD_STATUS_OK", "LOAD_STATUS_INCORRECT", "DOWNLOAD_COMPLETE"})
		-- TODO: MapSetDayOffset(0);
	elseif (event == "check") then
		self:check("LOAD_STATUS_OK", smChooseCharacter.DisplayStats);
		self:check("LOAD_STATUS_INCORRECT", smChooseCharacter.DownloadCharacter);
	elseif (event == "update") then
		-- Again, no server, loads are immediate and no need to download
		self:set("LOAD_STATUS_OK");
	end
end

-- Character is currently downloading
smChooseCharacter.DownloadCharacter = function(self, event)
	if (event == "enter") then
	elseif (event == "check") then
		self:check("DOWNLOAD_COMPLETE", smChooseCharacter.DisplayStats);
		-- TODO: Error on download?
	elseif (event == "update") then
	elseif (event == "exit") then
    end
end

-- Character has been shown.  Show stats.
smChooseCharacter.DisplayStats = function(self, event)
	if (event == "enter") then
print("DisplayStats");
		self:clear({"BEGIN", "PASSWORD_ENTERED", "CHANGE_PASSWORD", "EXIT"})
		graphics.push();
		-- TODO: StatsLoadCharacter(StatsGetActive())
		uiLoadCharacter.start();
	elseif (event == "check") then
		self:check("BEGIN", smChooseCharacter.PasswordForLoad);
		self:check("PASSWORD_ENTERED", smChooseCharacter.PasswordForLoad);
		self:check("CHANGE_PASSWORD", smChooseCharacter.ChangePassword);
		self:check("EXIT", smChooseCharacter.Choices);
	elseif (event == "update") then
		uiLoadCharacter.update();
	elseif (event == "exit") then
		-- TODO: Convert this code
		--    passwordID = FormGetObjID(LOAD_CHARACTER_PASSWORD_TEXT);
		--    p_password = TxtboxGetData (passwordID);
		--//printf("StatsExit: %p (%-10.10s) %p (%-10.10s)\n", p_data->attemptPassword, p_data->attemptPassword, p_password, p_password) ; fflush(stdout) ;  delay(100) ;
		--    strcpy(p_data->attemptPassword, p_password) ;
		--
		uiLoadCharacter.finish();
		graphics.pop();
	end
end

-- Password for character load is being entered
smChooseCharacter.PasswordForLoad = function(self, event)
	if (event == "enter") then
		self:clear({"PASSWORD_OK", "PASSWORD_NOT_OK"})
		-- TODO: Convert this code:
		--    ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_UNKNOWN) ;
		--
		--    p_data = (T_SMCChooseData *)StateMachineGetExtraData(G_smHandle) ;
		--    DebugCheck(p_data != NULL) ;
		--
		--    StatsGetPassword(StatsGetActive(), password) ;
		--    if (strnicmp(password, p_data->attemptPassword, MAX_SIZE_PASSWORD) == 0)  {
		--        ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_OK) ;
		--    } else {
		--        ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_WRONG) ;
		--    }
		--		//    PromptStatusBarInit("Checking password ...", 100) ;
	elseif (event == "check") then
		self:check("PASSWORD_OK", smChooseCharacter.PlayGame);
		self:check("PASSWORD_NOT_OK", smChooseCharacter.DisplayStats);
	elseif (event == "update") then
		-- TODO: Convert this code
		--    passwordStatus = ClientGetCheckPasswordStatus() ;
		--
		--    if (passwordStatus == CHECK_PASSWORD_STATUS_OK)  {
		--        SMCChooseSetFlag(
		--            SMCCHOOSE_FLAG_PASSWORD_OK,
		--            TRUE) ;
		--//        PromptStatusBarClose() ;
		--        /* Fade to black */
		--        ColorFadeTo(0,0,0);
		--        /* clear screen */
		--        GrDrawRectangle (0,0,319,199,0);
		--
		--    } else if (passwordStatus == CHECK_PASSWORD_STATUS_WRONG)  {
		--        SMCChooseSetFlag(
		--            SMCCHOOSE_FLAG_PASSWORD_NOT_OK,
		--            TRUE) ;
		--//        PromptStatusBarClose() ;
		--        PromptDisplayMessage("Incorrect password.  Try again.") ;
		--    }
	elseif (event == "exit") then
	end
end

-- User is requesting to change their password
smChooseCharacter.ChangePassword = function(self, event)
	if (event == "enter") then
		self:clear({"CHANGE_PASSWORD_COMPLETE", "CHANGE_PASSWORD_ABORT"})
	elseif (event == "check") then
		self:check("CHANGE_PASSWORD_COMPLETE", smChooseCharacter.DisplayStats);
		self:check("CHANGE_PASSWORD_ABORT", smChooseCharacter.DisplayStats);
	
		-- TODO: Convert this code
		--    /* Clear out the password entries first. */
		--    password[0] = '\0' ;
		--    newPassword[0] = '\0' ;
		--
		--    /* Get the old password. */
		--    StatsGetPassword(StatsGetActive(), oldPassword) ;
		--
		--    /* Ask to enter a password and then the new password. */
		--    /* If at any time, the user selects cancel, abort. */
		--    /* If both are entered, send up the packet. */
		--    if ((strlen(oldPassword) == 0) || (PromptForString(
		--            "Enter password",
		--            MAX_SIZE_PASSWORD,
		--            password) == TRUE))  {
		--        if (strnicmp(oldPassword, password, 12) != 0)  {
		--            /* passwords don't match up.  Bad password status */
		--            ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_WRONG) ;
		--        } else {
		--            /* ok, correct password.  Enter a new one. */
		--            if (PromptForString(
		--                    "Enter new password",
		--                    MAX_SIZE_PASSWORD,
		--                    newPassword) == TRUE)  {
		--                StatsSetPassword(StatsGetActive(), newPassword) ;
		--                StatsSaveCharacter(StatsGetActive()) ;
		--                ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_OK) ;
		--            } else {
		--                ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_ABORT) ;
		--            }
		--        }
		--    } else {
		--        ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_ABORT) ;
		--    }
	elseif (event == "update") then
	    -- TODO: Convert this code
		--    passwordStatus = ClientGetChangePasswordStatus() ;
		--    if (passwordStatus == CHANGE_PASSWORD_STATUS_OK)  {
		--        PromptDisplayMessage("Password changed.") ;
		--
		--        SMCChooseSetFlag(
		--            SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,
		--            TRUE) ;
		--    } else if (passwordStatus == CHANGE_PASSWORD_STATUS_WRONG)  {
		--        PromptDisplayMessage("Incorrect password.") ;
		--
		--        SMCChooseSetFlag(
		--            SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE,
		--            TRUE) ;
		--    } else if (passwordStatus == CHANGE_PASSWORD_STATUS_ABORT)  {
		--        SMCChooseSetFlag(SMCCHOOSE_FLAG_CHANGE_PASSWORD_ABORT, TRUE) ;
		--    }
	
	elseif (event == "exit") then
	end
end

-- TODO: Check!  Server said you can come in!
smChooseCharacter.EnableBegin = function(self, event)
	if (event == "enter") then
		self:clear("ENABLE_COMPLETE");
		
		-- For now, always enable
		self:set("ENABLE_COMPLETE");
	elseif (event == "check") then
		self:check("ENABLE_COMPLETE", smChooseCharacter.DisplayStats);
	elseif (event == "update") then
	elseif (event == "exit") then
	end
end

-- User is choosing to delete their character.  They have to enter the
-- password first!
smChooseCharacter.Delete = function(self, event)
	if (event == "enter") then
		self:clear({"DELETE_PASSWORD_OK", "DELETE_PASSWORD_NOT_OK"})
		
		local chardata = stats.getSavedCharacterIDStruct(stats.getActive());
		local oldPassword = chardata.password;
		if (chardata.status ~= "undefined") then
			local action, password;
			if (#(chardata.password) > 0) then
				action, password = prompt.forString("^001Enter password to confirm ^021delete", 12);
			else
				action = "ok";
				password = "";
			end
			if (action == "ok") then
				if (password == chardata.password) then
					self:set("DELETE_PASSWORD_OK");
				else
					prompt.displayMessage("Improper password. Delete command canceled.");
					self:set("DELETE_PASSWORD_NOT_OK");
				end
			else
				self:set("DELETE_PASSWORD_NOT_OK");
			end
		end
	elseif (event == "check") then
		self:check("DELETE_PASSWORD_OK", smChooseCharacter.DeleteCharacter);
		self:check("DELETE_PASSWORD_NOT_OK", smChooseCharacter.Choices);
	elseif (event == "update") then
	elseif (event == "exit") then
	end
end

-- Character is being deleted
smChooseCharacter.DeleteCharacter = function(self, event)
	if (event == "enter") then
		self:clear("DELETE_COMPLETE");
	elseif (event == "check") then
		self:check("DELETE_COMPLETE", smChooseCharacter.Choices);
	elseif (event == "update") then
		if (prompt.question("Are you sure you want to delete?", false) == true) then
		 	-- TODO: StatsDeleteChaacter(StatsGetActive());
			prompt.displayMessage("Character deleted.");
		else
			prompt.displayMessage("Character NOT deleted.");
		end
		self:set("DELETE_COMPLETE");
	elseif (event == "exit") then
	end
end

-- Play Game -- yeah!  We're in!
smChooseCharacter.PlayGame = function(self, event)
	if (event == "enter") then
		-- Call the upper state machine to begin the game 
		-- (this state machine will end and need a reset to continue)
		smMain.set("BEGIN_GAME")
	end
end

-- Player has chosen to exit the choose character screen (and start
-- exiting the server)
smChooseCharacter.Exit = function(self, event)
	if (event == "enter") then
		-- Player has chosen to exit the list of characters.  Let's
		-- tell the main state machine we need to leave this server.
		-- (this state machine will end and need a reset to continue)
		smMain:set("LEAVE_SERVER")
	end
end

-- Create a new character.  Show the uploading
smChooseCharacter.CreateUpload = function(self, event)
	if (event == "enter") then
		self:clear("CREATE_UPLOAD_OK")
		-- No server, so go ahead and always say the upload is okay!
		self:set("CREATE_UPLOAD_OK")
	elseif (event == "check") then
		-- If the upload is good, go straight into playing the game
		self:check("CREATE_UPLOAD_OK", smChooseCharacter.PlayGame);
		-- TODO: What about failure to upload here?
	elseif (event == "update") then
	elseif (event == "exit") then
	end
end

smChooseCharacter.init = function()
	-- Start in Connect mode
	smChooseCharacter.state = smChooseCharacter.WaitForList;
end

smChooseCharacter.finish = function()
	-- Mark this as done and avoid any more calls (nothing else to do currently)
	smChooseCharacter.state = nil;
end

smChooseCharacter.init();

return smChooseCharacter
