stats = {}

local aastats = require "aastats";

function stats.getCharacterList()
	-- Creates a table of characters with { name, password, status, mail }
	return aastats.getCharacterList()
end

function stats.setActiveCharacterList(charList)
	stats.charList = charList 
	aastats.setSavedCharacterList(charList)
end

return stats
