package.path = './Lua/?.lua;./Lua/AAEngine/?.lua';

local color = require "color"
local graphics = require "graphics"
local keyboard = require "keyboard"
local keymap = require "keymap"
local mouse = require "mouse"
local pics = require "pics"
local sound = require "sound"
local time = require "time"
local ticker = require "ticker"
local view = require "view"

local VERSION_TEXT = "Lua version 0.01"

local function startup()
	print "Hello "
end

function showScreen(picName, pal, timeout, showTag, doFlash)
	local bypassed = false
	local wasGamma = false
	local showScreen_clicked = false
	
	stoptime = ticker.get() + timeout
	bitmap = pics.lockBitmap(picName)
	if (not bitmap) then
		print("Missing bitmap "..picName.."!")
		return;
	end
	
	mouse.pushEventHandler(function (event, x, y, buttons)
			if (buttons) then
				showScreen_clicked = true
			end
		end
	)
	
	view.setPalette(pal)
	color.storeDefaultPalette()
	if (doFlash) then
		color.addGlobal({255, 255, 255})
	end
	graphics.drawPic(bitmap, 0, 0)
	pics.unlockAndUnfind(bitmap)
	
	if (showTag) then
		graphics.setCursor(5, 188)
		graphics.drawShadowedText(VERSION_TEXT, 210, 0)
	end
	
	-- Check if a key was pressed on the keyboard and stop if pressed
	keyboard.bufferOn()
	while (ticker.get() < stoptime) do
		if (keyboard.bufferGet()) then
			bypassed = true
			break
		end 
	
		-- Check if a mouse button was pressed and stop if pressed
		mouse.updateEvents()
		if (showScreen_clicked) then 
			bypassed = true;
			break
		end
		
		color.update(1);
		
		-- At this point, the darkness can be adjusted with ALT-F11
		-- Check if the gamma key is pressed and adjust
		-- but don't keep adjusting until the key is released
--		if (keyboard.getScanCode("ALT")) then
--			if (keymap.getScan("gamma")) then
--				if (wasGamma ~= true) then
--					color.gammaAjusst()
--					color.update(1)
--					wasGamma = true
--				end
--			else
--				wasGamma = false;
--			end
--		else
--			wasGamma = false;
--		end
	end
	mouse.popEventHandler()

	return bypassed
end

function titlescreen()
	print("Time is now "..ticker.get());
	sound.play(3501, 1, 1);
	
	-- Show the company screen 
	if (showScreen("UI/SCREENS/COMPANY", "standard", 400, true, true) == false) then
		color.fadeto({0, 0, 0})
		graphics.fillRect(0, 0, 319, 199, 0)
		color.update(1)
		if (showScreen("UI/SCREENS/PRESENTS", "standard", 250, true, false) == false) then
			color.fadeto({0, 0, 0})
			graphics.fillRect(0, 0, 319, 199, 0)
			color.update(1)
			time.delayMS(350)
			if (showScreen("UI/SCREENS/BEGIN1", "standard", 1000, true, false) == false) then
				color.fadeto({0, 0, 0})
				graphics.fillRect(0, 0, 319, 199, 0)
				color.update(1)
			end
		end
	end
end

startup();

-- Lighting sound!
--sound.PlayByNumber(3501, 1.0);
--sound.PlayByName("snd#3501", 1.0);
--sound.play(3501, 1.0);
