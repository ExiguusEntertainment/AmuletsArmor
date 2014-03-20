mouse = { hotspot = { 0, 0 }, pic = { pic = nil, bitmap = nil, res = nil }}

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
printf("Mouse Set default bitmap %s %s", pic, hotspot)
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
printf("mouse use default bitmap");
	aamouse.UseDefaultBitmap()
end

function mouse.getBitmapAndHotspot()
printf("mouse get bitmap and hotspot: %s %s %s", mouse.hotspot.x, mouse.hotspot.y, mouse.pic);
	return { hotspot = mouse.hotspot, pic = mouse.pic };
end

return mouse
