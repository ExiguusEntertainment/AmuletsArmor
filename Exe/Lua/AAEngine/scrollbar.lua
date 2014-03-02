scrollbar = {}

local aascrollbar = require "aascrollbar";

function scrollbar.handleMouseEvent(event, x, y, buttons)
	aascrollbar.HandleMouseEvent(event, x, y, buttons)
end

return scrollbar
