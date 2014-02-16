local color = require "AAEngine/color"
local display = require "AAEngine/display"
local graphics = require "AAEngine/graphics"
local keyboard = require "AAEngine/keyboard"
local keymap = require "AAEngine/keymap"
local mouse = require "AAEngine/mouse"
local pics = require "AAEngine/pics"
local sound = require "AAEngine/sound"
local time = require "AAEngine/time"
local ticker = require "AAEngine/ticker"
local view = require "AAEngine/view"
local config = require "config"

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
		if (keyboard.getScanCode(keyboard.scankeys.KEY_SCAN_CODE_ALT)) then
			if (keymap.getScanCode(keymap.mapping.KEYMAP_GAMMA_CORRECT)) then
				if (wasGamma ~= true) then
					color.gammaAdjust()
					color.update(1)
					wasGamma = true
				end
			else
				wasGamma = false;
			end
		else
			wasGamma = false;
		end
	end
	mouse.popEventHandler()

	if (not bypassed) then
		color.fadeto({0, 0, 0})
	end		
	graphics.fillRect(0, 0, display.width()-1, display.height()-1, 0)
	color.update(1)

	return bypassed
end

function titlescreen()
	print("Time is now "..ticker.get());
	sound.play(3501, 1, 1);
	
	-- Show the company screen 
	if (showScreen("UI/SCREENS/COMPANY", "standard", 400, true, true)) then return end;
	
	-- Show "presents"
	if (showScreen("UI/SCREENS/PRESENTS", "standard", 250, false, false)) then return end;
	time.delayMS(350)
	
	-- Show A&A Logo screen
	if (showScreen("UI/SCREENS/BEGIN1", "standard", 1000, true, false)) then return end;
end

