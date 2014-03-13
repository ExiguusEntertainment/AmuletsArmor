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
		if (obj.id == "ok") then
			form.action = "ok";
			form.exit = 1;
		elseif (obj.id == "cancel") then
			form.action = "cancel";
			form.exit = 1;
		end
	elseif (event == "changed") then
		form.enteredString = obj.get(); -- TODO: Make this work!
	end
end

prompt.displayMessage = function(message)
print("prompt.displayMessage");	
	Form.deleteAll();
	local form = Form.create(prompt.control);
	prompt.exit = false;
	prompt.action = "none";
	
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

return prompt;
