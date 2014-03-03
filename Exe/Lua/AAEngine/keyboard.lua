keyboard = {}

keyboard.scankeys = {
	KEY_SCAN_CODE_NONE = 0,
	KEY_SCAN_CODE_ESC = 1,
	KEY_SCAN_CODE_1 = 2,
	KEY_SCAN_CODE_2 = 3,
	KEY_SCAN_CODE_3 = 4,
	KEY_SCAN_CODE_4 = 5,
	KEY_SCAN_CODE_5 = 6,
	KEY_SCAN_CODE_6 = 7,
	KEY_SCAN_CODE_7 = 8,
	KEY_SCAN_CODE_8 = 9,
	KEY_SCAN_CODE_9 = 10,
	KEY_SCAN_CODE_0 = 11,
	KEY_SCAN_CODE_MINUS = 12,
	KEY_SCAN_CODE_EQUAL = 13,
	KEY_SCAN_CODE_BACKSPACE = 14,
	KEY_SCAN_CODE_TAB = 15,
	KEY_SCAN_CODE_Q = 16,
	KEY_SCAN_CODE_W = 17,
	KEY_SCAN_CODE_E = 18,
	KEY_SCAN_CODE_R = 19,
	KEY_SCAN_CODE_T = 20,
	KEY_SCAN_CODE_Y = 21,
	KEY_SCAN_CODE_U = 22,
	KEY_SCAN_CODE_I = 23,
	KEY_SCAN_CODE_O = 24,
	KEY_SCAN_CODE_P = 25,
	KEY_SCAN_CODE_SB_OPEN = 26,
	KEY_SCAN_CODE_SB_CLOSE = 27,
	KEY_SCAN_CODE_ENTER = 28,
	KEY_SCAN_CODE_CTRL = 29,
	KEY_SCAN_CODE_LEFT_CTRL = 29,
	KEY_SCAN_CODE_RIGHT_CTRL = 157,
	KEY_SCAN_CODE_A = 30,
	KEY_SCAN_CODE_S = 31,
	KEY_SCAN_CODE_D = 32,
	KEY_SCAN_CODE_F = 33,
	KEY_SCAN_CODE_G = 34,
	KEY_SCAN_CODE_H = 35,
	KEY_SCAN_CODE_J = 36,
	KEY_SCAN_CODE_K = 37,
	KEY_SCAN_CODE_L = 38,
	KEY_SCAN_CODE_SEMI_COLON = 39,
	KEY_SCAN_CODE_APOSTROPHE = 40,
	KEY_SCAN_CODE_GRAVE = 41,
	KEY_SCAN_CODE_LEFT_SHIFT = 42,
	KEY_SCAN_CODE_BACKSLASH = 43,
	KEY_SCAN_CODE_Z = 44,
	KEY_SCAN_CODE_X = 45,
	KEY_SCAN_CODE_C = 46,
	KEY_SCAN_CODE_V = 47,
	KEY_SCAN_CODE_B = 48,
	KEY_SCAN_CODE_N = 49,
	KEY_SCAN_CODE_M = 50,
	KEY_SCAN_CODE_COMMA = 51,
	KEY_SCAN_CODE_PERIOD = 52,
	KEY_SCAN_CODE_SLASH = 53,
	KEY_SCAN_CODE_RIGHT_SHIFT = 54,
	KEY_SCAN_CODE_STAR = 55,
	KEY_SCAN_CODE_ALT = 56,
	KEY_SCAN_CODE_SPACE = 57,
	KEY_SCAN_CODE_CAPS_LOCK = 58,
	KEY_SCAN_CODE_NUM_LOCK = 69,
	KEY_SCAN_CODE_SCROLL_LOCK = 70,
	
	KEY_SCAN_CODE_UNUSED1 = 84,
	KEY_SCAN_CODE_UNUSED2 = 85,
	KEY_SCAN_CODE_UNUSED3 = 86,
	
	-- Keypad keys:
	KEY_SCAN_CODE_KEYPAD_0 = 82,
	KEY_SCAN_CODE_KEYPAD_1 = 79,
	KEY_SCAN_CODE_KEYPAD_2 = 80,
	KEY_SCAN_CODE_KEYPAD_3 = 81,
	KEY_SCAN_CODE_KEYPAD_4 = 75,
	KEY_SCAN_CODE_KEYPAD_5 = 76,
	KEY_SCAN_CODE_KEYPAD_6 = 77,
	KEY_SCAN_CODE_KEYPAD_7 = 71,
	KEY_SCAN_CODE_KEYPAD_8 = 72,
	KEY_SCAN_CODE_KEYPAD_9 = 73,
	KEY_SCAN_CODE_KEYPAD_PERIOD = 83,
	KEY_SCAN_CODE_KEYPAD_NUM_LOCK = 69,
	KEY_SCAN_CODE_KEYPAD_SLASH = 181,
	KEY_SCAN_CODE_KEYPAD_STAR = 55,
	KEY_SCAN_CODE_KEYPAD_MINUS = 74,
	KEY_SCAN_CODE_KEYPAD_PLUS = 78,
	KEY_SCAN_CODE_KEYPAD_ENTER = 156,
	KEY_SCAN_CODE_KEYPAD_DELETE = 83,
	KEY_SCAN_CODE_KEYPAD_INSERT = 82,
	
	-- Function keys:
	KEY_SCAN_CODE_F1 = 59,
	KEY_SCAN_CODE_F2 = 60,
	KEY_SCAN_CODE_F3 = 61,
	KEY_SCAN_CODE_F4 = 62,
	KEY_SCAN_CODE_F5 = 63,
	KEY_SCAN_CODE_F6 = 64,
	KEY_SCAN_CODE_F7 = 65,
	KEY_SCAN_CODE_F8 = 66,
	KEY_SCAN_CODE_F9 = 67,
	KEY_SCAN_CODE_F10 = 68,
	KEY_SCAN_CODE_F11 = 87,
	KEY_SCAN_CODE_F12 = 88,
	
	-- Standard arrow keys:
	KEY_SCAN_CODE_UP = 200,
	KEY_SCAN_CODE_DOWN = 208,
	KEY_SCAN_CODE_LEFT = 203,
	KEY_SCAN_CODE_RIGHT = 205,
	
	-- Other editing keys:
	KEY_SCAN_CODE_HOME = 199,
	KEY_SCAN_CODE_PGUP = 201,
	KEY_SCAN_CODE_END = 207,
	KEY_SCAN_CODE_PGDN = 209,
	KEY_SCAN_CODE_INSERT = 210,
	KEY_SCAN_CODE_DELETE = 211,
	
	-- Special keys:
	KEY_SCAN_CODE_MACRO = 239,
	KEY_SCAN_CODE_PAUSE = 220,
}


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

-- Determine if the given scan key is being pressed
function keyboard.getScanCode(scankey)
	return aakeyboard.GetScanCode(scankey);
end

function keyboard.updateEvents()
	return aakeyboard.UpdateEvents();
end

function keyboard.debounce()
	return aakeyboard.Debounce()
end

keyboard.eventHandlers = {}

function keyboard.pushEventHandler(func)
	table.insert(keyboard.eventHandlers, func);
	aakeyboard.PushEventHandler();
end

function keyboard.popEventHandler()
	table.remove(keyboard.eventHandlers);
	aakeyboard.PopEventHandler();
end

function _keyboardHandleEvent(event, scankey)
	keyboard.eventHandlers[#keyboard.eventHandlers](event, scankey)
end

return keyboard
