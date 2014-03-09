graphic = {}

local aagraphic = require "aagraphic";

function graphic.create(x, y, picName)
	if (pics.exist(picName)) then
		return aagraphic.Create(x, y, picName)
	else
		error(string.format("Graphic '%s' does not exist!", picName));
	end
end

function graphic.updateAllGraphics()
	aagraphic.UpdateAllGraphics()
end

return graphic
