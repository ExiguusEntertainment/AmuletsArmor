view = {}

local aaview = require "aaview";

-- Set to one of the default palettes.  A palette is a merely a list of colors
-- indexed from 0..255.  In the original A&A, these colors will shift and
-- change for special effects.  Going forward, the palette may end up being
-- a short hand of colors to use instead of full RGB references. 
function view.setPalette(pal)
	aaview.SetPalette(pal);
end

return view
