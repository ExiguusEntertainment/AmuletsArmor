stats = {}

local aastats = require "aastats";

function stats.getCharacterList()
	-- Creates a table of characters with { name, password, status, mail }
	return aastats.GetCharacterList()
end

function stats.setActiveCharacterList(charList)
	stats.charList = charList 
	aastats.SetSavedCharacterList(charList)
end

return stats
