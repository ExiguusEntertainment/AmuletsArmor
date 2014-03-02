button = {}

local aabutton = require "aabutton";

function button.handleMouseEvent(event, x, y, buttons)
	aabutton.HandleMouseEvent(event, x, y, buttons)
end

return button
