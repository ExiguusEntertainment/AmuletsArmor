local mouse = {}

local aamouse = require "aamouse";

local mouseEventHandlers = {}

function _mouseHandleEvent(event, x, y, buttons)
	mouseEventHandlers[#mouseEventHandlers](event, x, y, buttons)
end

function mouse.pushEventHandler(func)
	table.insert(mouseEventHandlers, func);
	aamouse.PushEventHandler();
end

function mouse.popEventHandler(func)
	table.remove(mouseEventHandlers);
	aamouse.PopEventHandler();
end

function mouse.updateEvents()
	aamouse.UpdateEvents()
end

return mouse
