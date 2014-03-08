sound = {}

local aasound = require "aasound";

function sound.play(sound, volume, looped)
	if (volume == nil) then
		volume = 1.0
	end
	if (looped == nil) then
		looped = false
	end
	if (looped) then
		return aasound.PlayLoop(sound, volume)
	else
		return aasound.Play(sound, volume)
	end
end

function sound.update()
	aasound.Update();
end

return sound
