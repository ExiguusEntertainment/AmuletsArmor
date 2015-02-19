-----------------------------------------------
--Overall frame for displaying shop forms
--Entered from character load/create
--Exited in INN, with X, or through quest
-----------------------------------------------
uiTown = {}

local form;

function uiTown.CreateForm()
	local char;
	
	char = stats.get();
	
	form = Form.create(uiTown.eventHandler);
end

function uiTown.start()
-- TODO: printf("uiTown.start");
end

function uiTown.finish()
-- TODO: printf("uiTown.finish");
end

function uiTown.update()
-- TODO: printf("uiTown.update");
end

------------------------------------------------------------------------------
-- Handle form events here
------------------------------------------------------------------------------
uiTown.eventHandler = function(form, obj, event)
end

return uiTown;

