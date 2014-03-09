pics = {}

local aapics = require "aapics";

function pics.exist(name)
	return aapics.Exist(name);
end

function pics.lockBitmap(name)
	if (pics.exist(name)) then
		bitmap, res =  aapics.LockData(name);
		return { data=bitmap, bitmap=bitmap, res=res };
	else
		error(string.format("Bitmap resource '%s' does not exist!", name))
	end
end

function pics.lockData(name)
	if (pics.exist(name)) then
		assert(pics.exist(name));
		data, res =  aapics.LockData(name);
		return { data=data, res=res };
	else
		error(string.format("Data resource '%s' does not exist!", name))
	end
end

function pics.unfind(pic)
	return aapics.Unfind(pics.res);
end

function pics.unlock(pic)
	return aapics.Unlock(pic.res);
end

function pics.unlockAndUnfind(pic)
	return aapics.UnlockAndUnfind(pic.res);
end

return pics;
