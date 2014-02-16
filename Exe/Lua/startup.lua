package.path = './Lua/?.lua;./Lua/AAEngine/?.lua';

local color = require "color"
local display = require "display"
local graphics = require "graphics"
local keyboard = require "keyboard"
local keymap = require "keymap"
local mouse = require "mouse"
local pics = require "pics"
local sound = require "sound"
local time = require "time"
local ticker = require "ticker"
local titlescreen = require "titlescreen"
local view = require "view"

local VERSION_TEXT = "Lua version 0.01"

local function startup()
	print "Hello "
end

startup();

