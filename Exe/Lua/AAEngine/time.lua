local time = {}

local aatime = require "aatime";

-- Delay a number of milliseconds. NOTE: Not to be used during a game
function time.delayMS(ms)
	aatime.DelayMS(ms);
end

return time
