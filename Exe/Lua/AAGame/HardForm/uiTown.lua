-----------------------------------------------
--Overall frame for displaying shop forms
--Entered from character load/create
--Exited in INN, with X, or through quest
-----------------------------------------------
uiTown = {
	bgPic = nil
	
}

local form

uiTown.createForm = function()
	local char
	
	--char = stats.get()
	
	form = Form.create(uiTown.eventHandler)
	uiTown.bgPic = pics.lockBitmap("UI/TOWN/TOWNBACK")	

	--form:addGraphic{id="twnbackground", x=4, y=44, picName="UI/TOWN/TOWNBACK"}

	printf("Townhall Opened 44")
	-- Textbox: Name
	--form:addTextbox{id="name", x=125, y=73, width=148, height=14, readonly=1,
	--	scrolling=0, font="FontMedium", mode="field", justify="center", text="Elmore's Retreat"}
	form:addTextbox{id="str", x=255, y=49, width=16, height=8, readonly=1,
		scrolling=0, font="FontTiny", mode="field", justify="center", text="TEST!!!!"}
	-- Button: Begin
	--form:addButton{id="begin", x=45, y=167, scankey1=keyboard.scankeys.KEY_SCAN_CODE_ALT,
		--scankey2=keyboard.scankeys.KEY_SCAN_CODE_B, picName="UI/LOADC/LOADC_B1"}

	--uiLoadCharacter.passwordField = form:addTextbox{id="password", x=104, y=153, width=167, height=10, readonly=0,
		--scrolling=0, font="FontMedium", mode="field"}
	--form:addTextbox{id="level", x=255, y=39, width=16, height=8, readonly=1,
		--scrolling=0, font="FontTiny", mode="field", justify="center", text=char.level}
	--form:addTextbox{id="str", x=255, y=49, width=16, height=8, readonly=1,
		--scrolling=0, font="FontTiny", mode="field", justify="center", text=char.attributes.strength}

	return form
end

--------------------------------------
-- Draw Graphics
--------------------------------------
function uiTown:DrawGraphics()
	--printf("Drawing %s", self.bgPic)
	graphics.drawPic(self.bgPic, 0, 0)
end

------------------------------------------------------------------------------
-- Initiliaze the uiTown screen by creating a form
------------------------------------------------------------------------------
function uiTown:init()
	printf("uitown init")
	self.createForm()
	form:draw()
end

-----------------------------------------------------
-- Entry point for form
-----------------------------------------------------
uiTown.start = function()
	printf("uiTown start")
	uiTown:init(uiTown)
	form:start()
end

uiTown.finish = function()
	printf("uitown finish")
	form:finish()
	pics.unlockAndUnfind(self.bgPic)
end

uiTown.update = function()
	--printf("uitown update")
	form:setResponse(nil)
	form:updateUI()
	
	uiTown:DrawGraphics()
	
	local response = form:getResponse()	
	return response
end

------------------------------------------------------------------------------
-- Handle form events here
------------------------------------------------------------------------------
uiTown.eventHandler = function(form, obj, event)
	printf("uitown eventHandler")
	if (event ~= "none") then
		if (event == "release") then
		end
	end
end

return uiTown