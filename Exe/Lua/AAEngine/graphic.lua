graphic = { index = {} }
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
		setmetatable(new_instance, graphic_mt);
		graphic.index[new_instance.handle] = new_instance;
		return new_instance;
	else
		error(string.format("Graphic '%s' does not exist!", picName));
	end
end

function graphic:delete()
	if (self.handle ~= nil) then
		aagraphic.Delete(self.handle);
		graphic.index[self.handle] = nil;
		self.handle = nil;
	end
end

function graphic.updateAllGraphics()
	aagraphic.UpdateAllGraphics()
end

function graphic.forceUpdateAllGraphics()
	aagraphic.ForceUpdateAllGraphics()
end

return graphic
