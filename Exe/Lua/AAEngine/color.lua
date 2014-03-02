color = {}

local aacolor = require "aacolor";

-- Modify the color palette by adding to the global RGB filter
function color.addGlobal(rgb)
	aacolor.AddGlobal(rgb[1], rgb[2], rgb[3])
end

-- Stop and fade to the given color over about 200 ms.
-- Used for title screens and transitions.  Do not use in regular game activity.
-- rgb = table [red (0..255), green (0..255), blue (0..255)]
function color.fadeto(rgb)
	aacolor.FadeTo(rgb[1], rgb[2], rgb[3])
end

-- Store the current palette as the default palette
function color.storeDefaultPalette()
	aacolor.StoreDefaultPalette()
end

-- Update the color palette for the given time
function color.update(duration)
	aacolor.Update(duration)
end

-- Adjust the gama
function color.gammaAdjust()
	aacolor.GammaAdjust()
end

return color
