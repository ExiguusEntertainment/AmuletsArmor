graphic = {}

local aagraphic = require "aagraphic";

function graphic.create(x, y, picName)
	return aagraphic.Create(x, y, picName)
end

function graphic.updateAllGraphics()
	aagraphic.UpdateAllGraphics()
end

return graphic
