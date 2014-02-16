local keyboard = {}

local aakeyboard = require "aakeyboard";

function keyboard.bufferOn()
	aakeyboard.BufferOn();
end

function keyboard.bufferOff()
	aakeyboard.BufferOff();
end

function keyboard.bufferGet()
	return aakeyboard.BufferGet();
end

return keyboard
