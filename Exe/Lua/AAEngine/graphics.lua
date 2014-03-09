graphics = {}

local aagraphics = require "aagraphics";

function graphics.drawPic(pic, x, y)
	aagraphics.DrawBitmap(pic.bitmap, x, y);
end

function graphics.setCursor(x, y)
	aagraphics.SetCursor(x, y)
end

function graphics.drawShadowedText(text, palColor, shadedPalColor)
	aagraphics.DrawShadowedText(text, palColor, shadedPalColor)
end

function graphics.drawText(text, palColor)
	aagraphics.DrawText(text, palColor)
end

function graphics.fillRect(x1, y1, x2, y2, color)
	aagraphics.DrawRectangle(x1, y1, x2, y2, color)
end

function graphics.push()
	aagraphics.ScreenPush();
end

function graphics.pop()
	aagraphics.ScreenPop();
end

function graphics.shadeRect(x1, y1, x2, y2, shade)
	aagraphics.ShadeRectangle(x1, y1, x2, y2, shade)
end

return graphics
