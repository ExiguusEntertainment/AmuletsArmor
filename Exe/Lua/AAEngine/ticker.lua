ticker = {}

local aaticker = require "aaticker";

-- Get the time since the application started in 70ths of a second
-- Returns: Ticks since start of application
function ticker.get()
	return aaticker.Get();
end

-- Pause the ticker clock
function ticker.pause()
	aaticker.Pause();
end

-- Continue the ticker clock
function ticker.continue()
	aaticker.Continue();
end

function ticker.sleep(milliseconds)
	aaticker.SleepMS(milliseconds)
end

return ticker
