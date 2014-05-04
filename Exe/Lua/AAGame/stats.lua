stats = {}

local aastats = require "aastats";

function stats.getCharacterList()
	-- Creates a table of characters with { name, password, status, mail }
	local chars = aastats.GetCharacterList()
print(inspect(chars));
	return chars;	
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

function stats.setPassword(c, newPassword)
	stats.char.password = newPassword;
end

-- Store the version of the character in Lua into the A&A engine underneath.
-- TODO: Remove this step
function stats.set()
	aastats.Set(stats.char);
end

-- Save the currently active character to the given save slot
function stats.saveCharacter(c)
	printf("SaveCharacter to slot %d with password %s", c, stats.char.password);
	stats.set();
	aastats.SaveCharacter(c);
	
	-- Update the list info
	stats.charList[c+1].name = stats.char.name;
	stats.charList[c+1].password = stats.char.password;
	stats.charList[c+1].mail = stats.char.mail;
	stats.charList[c+1].status = stats.char.status;
end

function stats.deleteCharacter(c)
	printf("DeleteCharacter slot %d", c);
	-- TODO: Enter code here!
end

return stats
