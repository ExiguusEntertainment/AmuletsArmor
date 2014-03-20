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

function stats.getActiveCharacterList()
	return stats.charList 
end

function stats.makeActive(charSelected)
	aastats.MakeActive(charSelected)
end

function stats.getActive()
	return aastats.GetActive();
end

function stats.getSavedCharacterIDStruct(c)
	return stats.charList[c+1];
end

function stats.get()
	return aastats.Get()
end

function stats.loadCharacter(c)
	local loadSuccessful = aastats.LoadCharacter(c);
	stats.char = aastats.Get();
	return loadSuccessful;
end

function stats.drawCharacterPortrait(x, y)
	if (stats.char) then
		local stmp = string.format("UI/CREATEC/CHAR%02d", stats.char.classType);
		graphics.fillRect(x, y, x+115, y+102, 0);
		local pic = pics.lockBitmap(stmp);
		graphics.drawPic(pic, x, y);
		color.update(0);
		pics.unlockAndUnfind(pic);
	end	
end

return stats
