textbox = {}
textbox_mt = { __index = textbox};

local aatextbox = require "aatextbox";

function textbox.handleMouseEvent(event, x, y, buttons)
	aatextbox.HandleMouseEvent(event, x, y, buttons)
end

function textbox.handleKeyEvent(event, scankey)
	aatextbox.HandleKeyEvent(event, scankey)
end

function textbox:backspace()
	aatextbox.Backspace(self.textboxID);
end

function textbox:cursorTop()
	aatextbox.CursorTop(self.textboxID);
end

function textbox:repaginate()
	aatextbox.Repaginate(self.textboxID);
end

function textbox.firstBox()
	aatextbox.FirstBox();
end

function textbox:set(text)
	aatextbox.SetText(self.textboxID, text)
end

function textbox:append(text)
	aatextbox.Append(self.textboxID, text)
end

function textbox:getSelection()
	return aatextbox.GetSelectionNumber(self.textboxID)
end

function textbox:cursorSetRow(row)
	return aatextbox.CursorSetRow(self.textboxID, row)
end

function textbox.create(x, y, width, height, font, maxLength, scankey1, scankey2, numericOnly, justify, boxmode, callback)
	local hotkeys = scankey1 * 256 + scankey2
	local textbox = {
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

	setmetatable( textbox, textbox_mt );
	textbox.textboxID = aatextbox.Create(x, y, width, height, font, maxLength, hotkeys, 
		numericOnly, justify, boxmode);
	textbox:cursorTop();
	textbox:repaginate();
	textbox.firstBox();
		
	return textbox 
end

return textbox
