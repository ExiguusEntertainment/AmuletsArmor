textbox = {}

local aatextbox = require "aatextbox";

function textbox.handleMouseEvent(event, x, y, buttons)
	aatextbox.HandleMouseEvent(event, x, y, buttons)
end

return textbox
