sound = {}

local aasound = require "aasound";

function sound.play(sound, volume, looped)
	if (volume == nil) then
		volume = 1.0
	end
	if (looped == nil) then
		looped = false
	end
	print("Playing sound "..sound.." now")
	if (looped) then
	    print("  sound is looped.")
		return aasound.PlayLoop(sound, volume)
	else
	    print("  sound is NOT looped")
		return aasound.Play(sound, volume)
	end
end

return sound
