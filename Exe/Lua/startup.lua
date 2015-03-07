package.path = './?.lua;./Lua/?.lua;./Lua/AAEngine/?.lua;./Lua/Classes/?.lua;./Lua/Utilities/?.lua';

require "Utilities/inspect"
require "Utilities/printf"
require "Utilities/tableUtils"

-- Global access tot he engine
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
JSON = require "Utilities/JSON";

require "AAGame/uiColors"
--require "AAGame/prompt"

local titlescreen = require "AAGame/titlescreen"
local config = require "config"
local inspect = require "inspect"


