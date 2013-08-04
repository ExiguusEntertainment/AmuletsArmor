/*-------------------------------------------------------------------------*
 * File:  PROMPT.C
 *-------------------------------------------------------------------------*/
/**
 * User Interface for handling simple yes/no prompts.
 *
 * @addtogroup Prompt
 * @brief Prompt Status Bar UI
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "COLOR.H"
#include "PROMPT.H"
#include "TICKER.H"

static E_Boolean G_exit=FALSE;
static E_promptAction G_action=PROMPT_ACTION_NO_ACTION;
static T_word16 G_statBaseRange=0;
static T_screen G_tempscreen;
static T_graphicID G_bargraphic=NULL;
static T_byte8 G_stringIn[60];

/*-------------------------------------------------------------------------*
 * Routine:  PromptStatusBarInit/Update/Close
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_void    PromptStatusBarInit (T_byte8 *prompt, T_word16 baserange)
{
    T_graphicID backgraphic;
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptStatusBarInit");
    G_statBaseRange=baserange;

    /* save current background */
    G_tempscreen=GrScreenAlloc();
    GrTransferRectangle (G_tempscreen,
                         0,
                         0,
                         319,
                         199,
                         0,
                         0);

    /* set up graphics */

    backgraphic=GraphicCreate (26,70,"UI/PROMPT/STAT_BK");
    TxtboxID=TxtboxCreate (129,86,
                           279,96,
                           "FontMedium",
                           0,
                           0,
                           FALSE,
                           Txtbox_JUSTIFY_CENTER,
                           Txtbox_MODE_VIEW_NOSCROLL_FORM,
                           NULL);

    G_bargraphic=GraphicCreate(39,105,"UI/PROMPT/STAT_GR");

    TxtboxSetData (TxtboxID,prompt);

    /* draw background */
    GraphicUpdateAllGraphics();

    /* draw black bar over bar graphic */
    GrDrawRectangle (41,105,279,113,0);
    /* free background graphic */
    GraphicDelete (backgraphic);
    /* free textbox */
    TxtboxDelete (TxtboxID);

    DebugEnd();
}


T_void    PromptStatusBarUpdate (T_word16 current)
{
    static T_word32 delta=0, lastupdate=0;
    T_word16 dx;
    float percentcomplete;
    T_screen tempscreen;
    DebugRoutine ("StatusBarUpdate");

    /* update color cycling */
	delta=TickerGet();
	/* update color every 4 ticks */
//	if (delta-lastupdate>4)
//	{
		ColorUpdate(delta);
		lastupdate=TickerGet();
//	}

    /* calculate percent complete */
    percentcomplete=(float)current/(float)G_statBaseRange;

    dx=(int)(238.0-(238.0*percentcomplete));

    /* draw bar graphic */
    /* double buffer */

    /* allocate hidden screen */
    tempscreen=GrScreenAlloc();

    /* copy current screen to hidden screen */
    GrTransferRectangle (tempscreen,
                         0,
                         0,
                         319,
                         199,
                         0,
                         0);

    /* set hidden screen to draw */
    GrScreenSet(tempscreen);

    /* draw bar graphic on background screen */
    GraphicDrawToCurrentScreen();
    DebugCheck (G_bargraphic != NULL);
    GraphicDrawAt (G_bargraphic,39,105);
    GraphicDrawToActualScreen();

    /* draw black bar over bar graphic */
    GrDrawRectangle (279-dx,105,279,113,0);

    /* copy hidden screen to visual one */
    MouseHide();
    GrTransferRectangle (GRAPHICS_ACTUAL_SCREEN,
                         0,
                         0,
                         319,
                         199,
                         0,
                         0);
    MouseShow();

    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);

    /* free the hidden screen */
    GrScreenFree(tempscreen);

    DebugEnd();
}


T_void    PromptStatusBarClose (T_void)
{
    T_screen tsc;

    DebugRoutine ("PromptStatusBarClose");

    /* restore background */
    tsc=GrScreenGet();

    GrScreenSet (G_tempscreen);
    GrTransferRectangle (tsc,
                         0,
                         0,
                         319,
                         199,
                         0,
                         0);
    GrScreenSet (tsc);
    GrScreenFree (G_tempscreen);

    /* delete bar graphic */
    GraphicDelete (G_bargraphic);
    G_bargraphic=NULL;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PromptForBoolean
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean PromptForBoolean (T_byte8 *prompt, E_Boolean defaultvalue)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptForBoolean");

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    GrActualScreenPush() ;

    GrShadeRectangle (37,89,301,123,125);
    GrShadeRectangle (36,88,302,124,125);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTBOOL.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();
    if (G_action==PROMPT_ACTION_NO_ACTION) return (defaultvalue);
    else if (G_action==PROMPT_ACTION_OK) return (TRUE);
    return (FALSE);
}


/*-------------------------------------------------------------------------*
 * Routine:  PromptNotify
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_void PromptNotify (T_byte8 *prompt)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptNotify");

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;
    /* load the prompt ui form */
	FormLoadFromFile ("PMPTNTFY.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();
}



/*-------------------------------------------------------------------------*
 * Routine:  PromptForInteger
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PromptForInteger  (T_byte8 *prompt,
                            T_word16 defaultvalue,
                            T_word16 min,
                            T_word16 max)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptForInteger");

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;
    /* load the prompt ui form */
	FormLoadFromFile ("PMPTINTG.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

    /* set entry field to be numeric only */
    TxtboxID=FormGetObjID (501);
    TxtboxSetNumericOnlyFlag (TxtboxID,TRUE);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();
    if (G_action==PROMPT_ACTION_NO_ACTION) return (defaultvalue);
    else if (G_action==PROMPT_ACTION_OK) return (TRUE);
    return (FALSE);
}






/*-------------------------------------------------------------------------*
 * Routine:  PromptControl
 *-------------------------------------------------------------------------*/
/**
 *  PromptControl is a callback assigned to the FormCallBack, it's purpose
 *  is to control all prompt functions
 *
 *<!-----------------------------------------------------------------------*/
T_void PromptControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
    T_TxtboxID TxtboxID;
	DebugRoutine ("PromptControl");

    if (objtype==FORM_OBJECT_BUTTON)
    {
        if (objstatus==BUTTON_ACTION_RELEASED)
        {
            if (objID==301)
            {
                /* cancel selected */
                G_action=PROMPT_ACTION_CANCEL;
                G_exit=TRUE;
            }
            else if (objID==300)
            {
                /* ok selected */
                G_action=PROMPT_ACTION_OK;
                G_exit=TRUE;
            }
        }
    }
    else if (objtype==FORM_OBJECT_TEXTBOX)
    {
        if (objID==501)
        {
            /* copy entered data into global field */
            /* for later usage */
            TxtboxID=FormGetObjID(501);
            if (TxtboxID != NULL)
            {
                strncpy (G_stringIn,TxtboxGetData(TxtboxID),TxtboxGetDataLength(TxtboxID));
                G_stringIn[TxtboxGetDataLength(TxtboxID)+1]='\0';
            }
        }
    }

	DebugEnd();
}


T_void PromptDisplayMessage (T_byte8 *prompt)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("Prompt DisplayMessage");

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    GrActualScreenPush() ;

    /* draw a shaded box */
//    GrShadeRectangle (0,0,319,199,190);
    GrShadeRectangle (37,89,301,123,125);
    GrShadeRectangle (36,88,302,124,125);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTMESG.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();
}



T_void PromptDisplayBulletin (T_byte8 *prompt)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptDisplayBulletin");

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    /* draw a shaded box */
//    GrShadeRectangle (0,0,319,199,190);
    GrShadeRectangle (37,59,301,154,125);
    GrShadeRectangle (36,58,302,155,125);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTBULL.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();
}


E_Boolean PromptDisplayDialogue (T_byte8 *prompt)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptDisplayDialogue");

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    /* draw a shaded box */
//    GrShadeRectangle (0,0,319,199,190);
    GrShadeRectangle (36,26,300,188,125);
    GrShadeRectangle (37,27,301,155,124);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTDLG.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();

    if (G_action==PROMPT_ACTION_OK) return (TRUE);

    return (FALSE);
}


E_Boolean PromptDisplayContinue (T_byte8 *prompt)
{
    T_TxtboxID TxtboxID;

    DebugRoutine ("PromptDisplayContinue");

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    /* draw a shaded box */
//    GrShadeRectangle (0,0,319,199,190);
    GrShadeRectangle (37,59,301,155,125);
    GrShadeRectangle (38,60,300,154,124);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTCONT.FRM");

    /* set up windows */
    TxtboxID=FormGetObjID (500);
    if (TxtboxID != NULL) TxtboxSetData (TxtboxID,prompt);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    GraphicUpdateAllGraphicsBuffered();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    GrActualScreenPop() ;

    DebugEnd();

    if (G_action==PROMPT_ACTION_OK) return (TRUE);

    return (FALSE);
}



/*-------------------------------------------------------------------------*
 * Routine:  PromptForString
 *-------------------------------------------------------------------------*/
/**
 *  Displays a prompt and requests a string of data.
 *  User has an accept/cancel option (returned in E_Boolean)
 *  Note that returned data pointer is only valid until a new form
 *  is loaded!!!!
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean PromptForString  (T_byte8 *prompt,
                            T_word16 maxlen,
                            T_byte8 *data)
{

    T_TxtboxID TxtboxID;
    E_Boolean accept=FALSE;

    DebugRoutine ("PromptForString");
    DebugCheck (maxlen<=60);

    GrActualScreenPush() ;

    G_exit=FALSE;
    G_action=PROMPT_ACTION_NO_ACTION;

    strcpy (G_stringIn,"");
    /* make a shadow */
    GrShadeRectangle (37,81,301,133,125);
    GrShadeRectangle (36,80,302,134,125);

    /* load the prompt ui form */
	FormLoadFromFile ("PMPTSTRN.FRM");

    /* display the prompt */
    TxtboxID=FormGetObjID(500);
    DebugCheck (TxtboxID != NULL);
    TxtboxSetData (TxtboxID,prompt);

    /* set maximum length of enterable field */
    TxtboxID=FormGetObjID(501);
    DebugCheck (TxtboxID != NULL);
    TxtboxSetMaxLength (TxtboxID,maxlen);

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (PromptControl);

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    /* get the return string */
    sprintf (data,"%s\0",G_stringIn);
//  data=G_stringIn;
    if (G_action==PROMPT_ACTION_OK) accept=TRUE;

    GrActualScreenPop() ;

    DebugEnd();
    return (accept);
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  PROMPT.C
 *-------------------------------------------------------------------------*/
