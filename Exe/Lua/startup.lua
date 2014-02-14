package.path = './Lua/?.lua;./Lua/AAEngine/?.lua';

local pics = require "pics";

local function startup()
	print "Hello";
end

startup();
pics.foo();

local sound = require "sound";

-- Lighting sound!
sound.PlayByNumber(3501, 1.0);
