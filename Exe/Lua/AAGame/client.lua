client = {
	gamePaused = false,
	location = {
		type = "nowhere",
		place = "nowhere",
		startPos = 0,
	},
	isInView = false,  -- View is 3D mode?
	
	-- Number of Escapes pressed trying to exit a level
	escCount = 0,
	
	-- Number of the adventure in this series (if any, 0 for none)
	adventureNumber = 0,
	
	isActive = false,
}

-- Make the client goto the given place now.
function client:forceGotoPlace(type, place, startPos)
    local timeToUpdate;
    local previousLocation;
    local goToTown = false;

    -- Reset count of esc's
    self.escCount = 0 ;

    -- TODO: MouseRelativeModeOff();

	if ((type ~= self.location.type) or (place ~= self.location.place)) then
        -- Stop bothering with this level.
        -- TODO: ClientSetInactive() ;

		-- Stop hard coded form?
		if (self.location.type == "hardform") then
			-- TODO: hardform.end();
		end

        -- Allow packets going out the port to get a chance to go.
        -- TODO: timeToUpdate = TickerGet() + 15 ;
        -- do {
        --     CmdQUpdateAllSends() ;
        -- } while (TickerGet() <= timeToUpdate) ;

        previousLocation = table.deepcopy(self.location);
        self.location.type = type;
        self.location.place = place;
        self.location.startPos = startPos;

        -- Unload the map.
        -- TODO: MapUnload() ;

        if ((type == "exit") or (place == 0))  then
            -- End mouse relative mode if still active
            -- TODO: MouseRelativeModeOff();

            -- TODO: SMCPlayGameSetFlag(SMCPLAY_GAME_FLAG_END_GAME, TRUE) ;
        else
            -- Get rid of the UI for the bottom of the screen (if any).
            -- If above 10000, must be a ui dedicated form.
            -- Otherwise, it is a map number.

            if (type == "hardform")  then
                -- TODO: We must of completed the adventure! (An abort command 
                -- resets G_adventureNumber to zero).
                if (self.adventureNumber ~= 0) then
--                    StatsUpdatePastPlace(
--                        G_adventureNumber,
--                        0) ;
--                    // Save the character!
--                    StatsSaveCharacter(StatsGetActive());
--                    TownUISetAdventureCompleted() ;
                end

				client:gotoForm(place);

                -- We are no longer in an adventure if in form places
                self.adventureNumber = 0 ;
            else
                -- Record that we have made it to here in the adventure.
                if (self.adventureNumber ~= 0) then
--                    StatsUpdatePastPlace(
--                        G_adventureNumber,
--                        G_currentPlace) ;
--
                    -- Save the character!
                    stats.saveCharacter(stats.getActive());

                    -- TODO: if (previousLocation < 10000)
                    --    goToTown = TownUICompletedMapLevel(previousLocation) ;
                end

                -- We have by passed to go to the town.
                if (goToTown) then
                    self.adventureNumber = 0 ;

                    -- TODO: MouseRelativeModeOff();

                    -- Save the character!
                    stats.saveCharacter(stats.getActive());

                    ClientSetCurrentPlace(HARD_FORM_TOWN+HARDFORM_GOTO_PLACE_OFFSET) ;
                    self.location.type = "hardform";
                    self.location.place = "town hall";
                    self.location.startPos = 0;
                    client:gotoForm("town hall");
                else
                	self.location.type = "map";

                    -- Resync the random numbers
                    -- TODO: RandomReset() ;

                    -- Reset the sync timer.
                    -- TODO: SyncTimeSet(1) ;

                    -- TODO: MapLoad(placeNumber) ;
                end
            end
        end

        -- Tell everyone about my new location.
        -- TODO: ClientSendPlayerIDSelf() ;
    end
end

function client:gotoPlace(type, place, startPos)
    -- Reset any previous requests to go elsewhere.  Either it is */
    -- a bad place or we are going to go to it real soon. */
    -- TODO: ClientSetNextPlace(0xFFFF, 0xFF) ;

    self.gamePaused = false;

	if ((type ~= self.location.type) or (place ~= self.location.place)) then
		local isValid = false;
		if (type == "hardform") then 
			isValid = true;
		end
		if ((type == "exit") or (place == 0)) then
			isValid = true;
		end
		-- TODO: if ((location == "map") and (map.exist(place))) then
		--			isValid = true;
		-- end
		if (isValid) then
            if (self.isInView)  then
                -- TODO: isFake = PlayerInFakeMode() ;
                -- TODO: if (!isFake)
                -- TODO:    PlayerSetFakeMode() ;
                -- TODO: if (PlayerGetObject()->inWorld == TRUE)
                -- TODO:    ObjectRemove(PlayerGetObject()) ;
                -- TODO: if (!isFake)
                -- TODO:    PlayerSetRealMode() ;
            end
            --  If zero, get out of playing the game.
            if ((type == "exit") or (place == 0)) then
                -- Leave this place
                self:forceGotoPlace(type, place, startPos) ;
--                SMCPlayGameSetFlag(
--                    SMCPLAY_GAME_FLAG_LEAVE_PLACE,
--                    TRUE) ;
            else
                self:forceGotoPlace(type, place, startPos) ;
                -- TODO: ClientStartPlayer(9000+G_loginId, G_loginId) ;
            end
        end
    end
end

function client:gotoForm(formName)
printf("Goto form %s", formName);
	self.type = "hard form";
	self.place = formName;
	hardform.start(formName);
	self.isActive = true;
end
