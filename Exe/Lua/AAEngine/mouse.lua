mouse = { hotspot = { 0, 0 }, pic = { pic = nil, bitmap = nil, res = nil }}

local aamouse = require "aamouse";

mouseEventHandlers = {}

function _mouseHandleEvent(event, x, y, buttons)
	if (#mouseEventHandlers > 0) then
		mouseEventHandlers[#mouseEventHandlers](event, x, y, buttons)
	end
end

function mouse.pushEventHandler(func)
	assert (func ~= nil);
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
		mouse.hotspot.x = 0;
		mouse.hotspot.y = 0;
		mouse.pic = { pic = nil, bitmap = nil, res = nil };
	else
		aamouse.SetDefaultBitmap(hotspot.x, hotspot.y, pic.bitmap)
		mouse.hotspot.x = hotspot.x;
		mouse.hotspot.y = hotspot.y;
		mouse.pic = pic;
	end
end

function mouse.useDefaultBitmap()
	aamouse.UseDefaultBitmap()
end

function mouse.getBitmapAndHotspot()
	return { hotspot = mouse.hotspot, pic = mouse.pic };
end

return mouse
