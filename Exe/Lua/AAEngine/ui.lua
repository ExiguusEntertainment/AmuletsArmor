------------------------------------------------------------------------
-- UI widget system for A&A.
-- 

ui = {}

require "AAEngine/button"
require "AAEngine/scrollbar"
require "AAEngine/textbox"

function ui.controlForJustUI(event, x, y, buttons)
	button.handleMouseEvent(event, x, y, buttons)
	scrollbar.mouseEvent(event, x, y, buttons)
	textbox.mouseEvent(event, x, y, buttons)
end
