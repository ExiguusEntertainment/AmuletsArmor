package.path = './?.lua;./Lua/?.lua;./Lua/AAEngine/?.lua;./Lua/Classes/?.lua;./Lua/Utilities/?.lua';

color = require "AAEngine/color"
display = require "AAEngine/display"
graphics = require "AAEngine/graphics"
keyboard = require "AAEngine/keyboard"
keymap = require "AAEngine/keymap"
mouse = require "AAEngine/mouse"
pics = require "AAEngine/pics"
sound = require "AAEngine/sound"
time = require "AAEngine/time"
ticker = require "AAEngine/ticker"
ui = require "AAEngine/ui"
view = require "AAEngine/view"

local titlescreen = require "AAGame/titlescreen"

local config = require "config"
local inspect = require "inspect"

local function startup()
	print "Hello "
end

startup();

require "AAGame/smMain" ;
for i=1,10 do
	smMain:update();
end
smChooseCharacter:set("EXIT")
smMain:set("LEAVE_GAME");
for i=1,10 do
	smMain:update();
end
