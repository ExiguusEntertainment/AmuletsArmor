local pics = {}

local aapics = require "aapics";

function pics.lockBitmap(name)
	bitmap, res =  aapics.LockData(name);
	return { data=bitmap, bitmap=bitmap, res=res };
end

function pics.lockData(name)
	data, res =  aapics.LockData(name);
	return { data=data, res=res };
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
