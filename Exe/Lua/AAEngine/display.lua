local display = {}

local aadisplay = require "aadisplay";

function display.width()
	return aadisplay.GetWidth()
end

function display.height()
	return aadisplay.GetHeight()
end

return display
