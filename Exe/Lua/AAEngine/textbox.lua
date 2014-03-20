textbox = { index = {} }
textbox_mt = { __index = textbox};

local aatextbox = require "aatextbox";

function textbox.handleMouseEvent(event, x, y, buttons)
	aatextbox.HandleMouseEvent(event, x, y, buttons)
end

function textbox.handleKeyEvent(event, scankey)
	aatextbox.HandleKeyEvent(event, scankey)
end

function textbox:append(text)
	aatextbox.Append(self.handle, text)
end

function textbox:backspace()
	aatextbox.Backspace(self.handle);
end

function textbox:cursorSetRow(row)
	return aatextbox.CursorSetRow(self.handle, row)
end

function textbox:cursorTop()
	aatextbox.CursorTop(self.handle);
end

function textbox.firstBox()
	aatextbox.FirstBox();
end

function textbox:get()
	return aatextbox.GetData(self.handle);
end

function textbox:getSelection()
	return aatextbox.GetSelectionNumber(self.handle)
end

function textbox:setSelection(selection)
	self:cursorSetRow(selection);
end

function textbox:repaginate()
	aatextbox.Repaginate(self.handle);
end

function textbox:set(text)
	aatextbox.SetText(self.handle, text)
end

function textbox:setMaxLength(length)
	aatextbox.SetMaxLength(self.handle, length)
end

function _textboxHandleEvent(handle, event)
	xpcall(
		function() 
			local obj = textbox.index[handle];
			obj.callback(obj, event);
		end, 
	AABacktrace)
end

function textbox:delete()
	if (self.handle ~= nil) then
		aatextbox.Delete(self.handle);
		textbox.index[self.handle] = nil;
	end
	self.handle = nil;
end

function textbox.create(x, y, width, height, font, maxLength, scankey1, scankey2, numericOnly, justify, boxmode, callback)
	local hotkeys = scankey1 * 256 + scankey2
	local new_textbox = {
		type = "textbox",
		mode = boxmode,
		id=id, 
		x=x, 
		y=y,
		width=width,
		height=height,
		font=font,
		scankey1 = scankey1,
		scankey2 = scankey2,
		numericOnly = numericOnly,
		justify = justify, 
		callback = callback};

	setmetatable( new_textbox, textbox_mt );
	new_textbox.handle = aatextbox.Create(x, y, width, height, font, maxLength, hotkeys, 
		numericOnly, justify, boxmode);
	new_textbox:cursorTop();
	new_textbox:repaginate();
	new_textbox.firstBox();
	textbox.index[new_textbox.handle] = new_textbox;

	return new_textbox 
end

return textbox
