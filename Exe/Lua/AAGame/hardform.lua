uiTown = require "AAGame/HardForm/uiTown"

hardform = {
	currentForm = nil,
	isOpen = false,
	closeButton = nil,

	G_hardforms = {	
		["town hall"] = {
			start = uiTown.start, -- TownUIStart,
			update = uiTown.update, -- TODO: TownUIUpdate
			finish = uiTown.finish, -- TownUIEnd,
			handleMouse = nil,
		},
	},
}

function hardform.start(formName)
printf("hardform start %s", formName)
	hardform.isOpen = true

    -- All hard forms have an alternate output turned on.
    -- TODO: MessageSetAlternateOutputOn()
    -- TODO: BannerUIModeOn()

    if (true) then
        -- create hardform close button
        hardform.closeButton = button.create(196, 5,"UI/COMMON/CLOSEWIN","normal",0, 0, nil, hardform.exit)
    else
        hardform.closeButton = nil
    end

	hardform.currentForm = formName
	
    if (hardform.G_hardforms[formName].start ~= nil) then
        hardform.G_hardforms[formName].start(formName)
    end

    -- TODO: if (G_timeIDLastUpdated)
    -- TODO:    G_timeIDLastUpdated = TickerGet()
end

function hardform.update()
	--printf("Hardform Update")
	if (hardform.currentForm ~= nil) then
        -- TODO: GrScreenSet(GRAPHICS_ACTUAL_SCREEN)
        uiBanner:update()
        -- TODO: StatsUpdatePlayerStatistics()
        graphic.updateAllGraphics()
        -- TODO: ScheduleUpdateEvents()
        keyboard.updateEvents()
        mouse.updateEvents()
        if (hardform.G_hardforms[hardform.currentForm].update ~= nil) then
			--printf("CurrentFormUpdate")
            hardform.G_hardforms[hardform.currentForm].update()
        end
        -- TODO: HardFormNetworkUpdate()
	end
end

function hardform.finish()
    if (hardform.closeButton ~= nil) then
	    hardform.closeButton:delete()
	    hardform.closeButton = nil
    end
    -- TODO: MessageSetAlternateOutputOff()
    -- TODO: if (BannerIsOpen())
    -- TODO:    BannerUIModeOff()

	assert(hardform.currentForm ~= nil)

    if (hardform.G_hardforms[hardform.currentForm].finish ~= nil) then
        hardform.G_hardforms[hardform.currentForm].finish()
    end

	hardform.currentForm = nil

    hardform.isOpen = false
end

function hardform.exit()
printf("TODO: hardform.exit")
    -- Tell others that I'm no longer creating a game.
    -- TODO: PeopleHereSetOurState(PLAYER_ID_STATE_NONE)

    -- Send out a message that this is what we are doing.
    -- TODO: ClientSendPlayerIDSelf()

    -- TODO: PeopleHereSetOurAdventure(0)
    -- TODO: ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress())
    
    -- Send out a message that this is what we are doing.
    -- TODO: ClientSendPlayerIDSelf()

    hardform.closeButton:delete()
    hardform.closeButton = nil

-- TODO:
--    if (TownUIIsOpen()) then
--        -- Save the character!
--        StatsSaveCharacter(StatsGetActive())
--
--        -- Exit town
--        ClientSetNextPlace(0, 0)
--    else
--        ClientSetNextPlace(HARDFORM_GOTO_PLACE_OFFSET + HARD_FORM_TOWN, 0)
--    end
end

return hardform
