local sound = {}

local aasound = require "aasound";

function sound.play(sound, volume, looped)
	if (volume == nil) then
		volume = 1.0
	end
	if (looped == nil) then
		looped = 0
	end
	print("Playing sound "..sound.." now")
	if (looped) then
		return aasound.PlayLoop(sound, volume)
	else
		return aasound.Play(sound, volume)
	end
end

return sound
