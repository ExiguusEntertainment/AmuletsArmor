mouseControl = {}

local init = false; 
local mode = "normal";
local buttonpushed = false;
local hotspot = {x = 0, y = 0};
local mouseRes = nil;
local bitmap = nil;
local type = "default";

------------------------------------------------------------------------------
-- This routine does the standard mouse update for the ui.
------------------------------------------------------------------------------
function mouseControl.controlForJustUI(event, x, y, buttons)
	-- Pass the mouse events on to the ui components
	ui.mouseEvent(event, x, y, buttons)
end

------------------------------------------------------------------------------
-- Sets up the mouse (& keyboard?) for control in ui situations.
-- Initializes all necessary variables and default mouse bitmap, etc.
------------------------------------------------------------------------------
function mouseControl.InitForJustUI()
print("mouseControl.InitForJustUI");
	assert(init == false)
	init = true
	hotspot = {x=0, y=0}
	mode = "normal"
	bitmap = pics.lockBitmap("UI/MOUSE/DEFAULT");
	assert(bitmap ~= nil)
	mouse.setDefaultBitmap(bitmap, hotspot)
	mouse.useDefaultBitmap()
	
	mouse.pushEventHandler(mouseControl.controlForJustUI)
end

------------------------------------------------------------------------------
-- Shuts down and cleans up control things.
-- Restores the mouse to the default bitmap.
------------------------------------------------------------------------------
function mouseControl.Finish()
print("mouseControl.Finish");
	assert(init == true)
	init = false

	-- No longer need the bitmap	
	pics.unlockAndUnfind(bitmap)
	mouse.setDefaultBitmap(nil, {x=0, y=0})
	mouse.useDefaultBitmap()
	
	-- No longer handling mouse events (at least for this ui)
	mouse.popEventHandler()
end
