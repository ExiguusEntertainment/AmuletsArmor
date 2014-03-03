------------------------------------------------------------------------
-- UI widget system for A&A.
-- 

ui = {}

require "AAEngine/button"
require "AAEngine/form"
require "AAEngine/graphic"
require "AAEngine/scrollbar"
require "AAEngine/textbox"

function ui.mouseEvent(event, x, y, buttons)
	button.handleMouseEvent(event, x, y, buttons)
	scrollbar.handleMouseEvent(event, x, y, buttons)
	textbox.handleMouseEvent(event, x, y, buttons)
end

return ui
