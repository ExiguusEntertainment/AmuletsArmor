graphic = {}
graphic_mt = { __index = graphic };

local aagraphic = require "aagraphic";

function graphic.create(x, y, picName)
	local new_instance = {
		x = x,
		y = y,
		picName = picName,
	}

	if (pics.exist(picName)) then
		new_instance.handle = aagraphic.Create(x, y, picName);
		return new_instance;
	else
		error(string.format("Graphic '%s' does not exist!", picName));
	end
end

function graphic:delete()
	aagraphic.Delete(self.handle);
	self.handle = nil;
end

function graphic.updateAllGraphics()
	aagraphic.UpdateAllGraphics()
end

return graphic
