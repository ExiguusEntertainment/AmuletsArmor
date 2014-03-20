button = { 
	-- List of buttons currently active in the system indexed by handle.
	-- Used to convert between T_buttonID and Lua button objects quickly.
	index = {}  
}
button_mt = { __index = button };

local aabutton = require "aabutton";

function button.handleMouseEvent(event, x, y, buttons)
	aabutton.HandleMouseEvent(event, x, y, buttons)
end

function button.handleKeyEvent(event, scankey)
	aabutton.HandleKeyEvent(event, scankey)
end

-- Global event to handle any button event going to a button
function _buttonHandleEvent(handle, event)
	xpcall( -- protected action with backtrace on failues
		function() 
			local obj = button.index[handle];
			if (event == "press") then
				obj.funcPress(obj, event);
			elseif (event == "release") then
				obj.funcRelease(obj, event);
			end
		end, 
	AABacktrace)
end

function button.create(x, y, picName, toggleType, scankey1, scankey2, funcPress, funcRelease)
	local new_instance = {
		x = x,
		y = y,
		picName = picName,
		toggleType = toggleType,
		scankey1 = scankey1,
		scankey2 = scankey2,
		funcPress=funcPress,
		funcRelease=funcRelease
	}

	local toggle = 0;
	local hotkeys = scankey1 * 256 + scankey2
	if (toggleType == "toggle") then 
		toggle = 1
	end
    new_instance.handle = aabutton.Create(x, y, picName, toggle, hotkeys);
	setmetatable(new_instance, button_mt);
	button.index[new_instance.handle] = new_instance;
	
	return new_instance;
end

function button:delete()
print("button:delete");
print(inspect(self));	
print(inspect(self.handle));	
print(inspect(button.index));
	if (self.handle ~= nil) then	
		aabutton.Delete(self.handle);
		button.index[self.handle] = nil;
	end
	self.handle = nil;
end

return button
