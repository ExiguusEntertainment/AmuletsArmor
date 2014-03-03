mouse = {}

local aamouse = require "aamouse";

local mouseEventHandlers = {}

function _mouseHandleEvent(event, x, y, buttons)
	mouseEventHandlers[#mouseEventHandlers](event, x, y, buttons)
end

function mouse.pushEventHandler(func)
	table.insert(mouseEventHandlers, func);
	aamouse.PushEventHandler();
end

function mouse.popEventHandler()
	table.remove(mouseEventHandlers);
	aamouse.PopEventHandler();
end

function mouse.updateEvents()
	aamouse.UpdateEvents()
end

function mouse.setDefaultBitmap(pic, hotspot)
	if (pic == nil) then
		aamouse.SetDefaultBitmap(0, 0, nil)
	else
		aamouse.SetDefaultBitmap(hotspot.x, hotspot.y, pic.bitmap)
	end
end

function mouse.useDefaultBitmap()
	aamouse.UseDefaultBitmap()
end

return mouse
