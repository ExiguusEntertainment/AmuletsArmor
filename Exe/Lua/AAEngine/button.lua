button = {}

local aabutton = require "aabutton";

function button.handleMouseEvent(event, x, y, buttons)
	aabutton.HandleMouseEvent(event, x, y, buttons)
end

function button.handleKeyEvent(event, scankey)
	aabutton.HandleKeyEvent(event, scankey)
end

function button.create(x, y, picName, toggleType, scankey1, scankey2)
	local toggle = 0;
	local hotkeys = scankey1 * 256 + scankey2
	if (toggleType == "toggle") then 
		toggle = 1
	end
    return aabutton.Create(x, y, picName, toggle, hotkeys);
end

return button
