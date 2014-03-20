prompt = {}

--T_void PromptDisplayMessage (T_byte8 *prompt)
--{
--    T_TxtboxID TxtboxID;
--
--    DebugRoutine ("Prompt DisplayMessage");
--
--    G_exit=FALSE;
--    G_action=PROMPT_ACTION_NO_ACTION;
--
--    GrActualScreenPush() ;
--
--    /* draw a shaded box */
--//    GrShadeRectangle (0,0,319,199,190);
--    GrShadeRectangle (37,89,301,123,125);
--    GrShadeRectangle (36,88,302,124,125);
--
--    /* load the prompt ui form */
--	FormLoadFromFile ("PMPTMESG.FRM");
--
--    /* set up windows */
--    TxtboxID=FormGetObjID (500);
--    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);
--
--	/* set the form callback routine to MainUIControl */
--	FormSetCallbackRoutine (PromptControl);
--
--    GraphicUpdateAllGraphicsBuffered();
--
--    /* go into generic control loop */
--    FormGenericControl (&G_exit);
--
--    GrActualScreenPop() ;
--
--    DebugEnd();
--}

prompt.control = function(form, obj, event)
	if (event == "release") then
		if ((obj.id == "ok") or (obj.id == "yes")) then
			form.action = "ok";
			form.exit = 1;
		elseif ((obj.id == "cancel") or (obj.id == "no")) then
			form.action = "cancel";
			form.exit = 1;
		end
	elseif (event == "changed") then
		form.enteredString = obj:get();
	end
end

prompt.displayMessage = function(message)
	Form.deleteAll();
	local form = Form.create(prompt.control);
	form.exit = false;
	form.action = "none";
	
	graphics.push();
	
	graphics.shadeRect(37, 89, 301, 123, 125);
	graphics.shadeRect(36, 88, 302, 124, 125);
	
	form:addGraphic{id="background", x=27, y=79, picName="UI/PROMPT/PMPTMESG"};
	form:addButton{id="ok", x=131, y=98, picName="UI/PROMPT/OK",
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ENTER};
	form.field = form:addTextbox{id="message", x=32, y=84, width=254, height=9, readonly=1, 
		scrolling=0, font="FontMedium", mode="ro_textarea_noscroll", justify="center"};
	form.field:set(message);
	
	form:run();
	
	graphics.pop();
end

prompt.forString = function(message, maxLength)
	Form.deleteAll();
	local form = Form.create(prompt.control);
	form.exit = false;
	form.action = "none";
	form.enteredString = "";
	
	graphics.push();
	
	graphics.shadeRect(37, 81, 301, 133, 125);
	graphics.shadeRect(36, 80, 302, 134, 125);

	form:addGraphic{id="background", x=27, y=71, picName="UI/PROMPT/PMPTSTRN"};
	form:addButton{id="ok", x=89, y=108, picName="UI/PROMPT/ACCEPT", 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ENTER};
	form:addButton{id="cancel", x=173, y=108, picName="UI/PROMPT/CANCEL", 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ESC};
	form.message = form:addTextbox{id="message", x=32, y=76, width=254, height=10, 
		readonly=1, scrolling=0, font="FontMedium", mode="field", justify="center"};
	form.field = form:addTextbox{id="field", x=65, y=91, width=192, height=10, 
		readonly=0, scrolling=0, font="FontMedium", mode="field", justify="left"};
	form.message:set(message);
	form.field:setMaxLength(maxLength);

	form:run();
	
	graphics.pop();

	return form.action, form.enteredString;
end

------------------------------------------------------------------------------
-- Ask a yes/no question and return with true for yes, false for no.
-- The default is what ENTER selects in the prompt
------------------------------------------------------------------------------
prompt.question = function(question, default)
	Form.deleteAll();
	local form = Form.create(prompt.control);
	form.exit = false;
	form.action = "none";

	graphics.push();
	
	graphics.shadeRect(37, 89, 301, 123, 125);
	graphics.shadeRect(36, 88, 302, 124, 125);

	form:addGraphic{id="background", x=27, y=79, picName="UI/PROMPT/PMPTBOOL"};
	form:addButton{id="yes", x=89, y=98, picName="UI/PROMPT/YES", 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ENTER};
	form:addButton{id="no", x=173, y=98, picName="UI/PROMPT/NO", 
		scankey2=keyboard.scankeys.KEY_SCAN_CODE_ESC};
	form.question = form:addTextbox{id="question", x=32, y=84, width=254, height=10, 
		readonly=1, scrolling=0, font="FontMedium", mode="field", justify="center"};
	form.question:set(question);

	form:run();
	
	graphics.pop();

	if (form.action == "none") then
		return default;
	end

	if (form.action == "ok") then
		return true;
	end
	
	return false;
end

return prompt;
