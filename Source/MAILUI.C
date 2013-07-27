/****************************************************************************/
/*    FILE:  MailUI.C                                                       */
/****************************************************************************/
#include "MAILUI.H"

static E_Boolean G_mainExit = FALSE;
static E_Boolean G_writeExit = FALSE;
static E_Boolean G_readExit = FALSE;


T_void MailMainUI (T_void)
{
    T_word32 delta=0, lastupdate=0;

    DebugRoutine ("MailMainUI");

    /* load the mail main ui form */
    MailMainUIInit ();

    /* go into generic control loop */
    FormGenericControl(&G_mainExit);

    DebugEnd();
}


T_void MailMainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
    T_TxtboxID TxtboxID;
    T_word16 selectnum;

    DebugRoutine ("MailMainUIControl");

    if (objstatus==BUTTON_ACTION_RELEASED && objtype==FORM_OBJECT_BUTTON)
    {
        if (objID==300)
        {
            /* delete button pressed */
        }
        else if (objID==301)
        {
            /* read button pressed */
            FormCleanUp();
            MailReadUI();
            MailMainUIInit();
        }
        else if (objID==302)
        {
            /* create button pressed */
            FormCleanUp();
            MailWriteUI();
            MailMainUIInit();
        }
        else if (objID==303)
        {
            /* reply button pressed */
            FormCleanUp();
            MailWriteUI();
            MailMainUIInit();
        }
        else if (objID==304)
        {
            /* forward button pressed */
            FormCleanUp();
            MailWriteUI();
            MailMainUIInit();
        }
        else if (objID==305)
        {
            G_mainExit=TRUE;
            /* exit button pressed */
        }
        else if (objID==306)
        {
            /* sort by Date */
        }
        else if (objID==307)
        {
            /* sort by from */
        }
        else if (objID==308)
        {
            /* sort by subject */
        }
    }

    else if (objtype==FORM_OBJECT_TEXTBOX)
    {
        if (objID>=500 && objID <=502)
        {
            /* mail listing row selected */
            TxtboxID=FormGetObjID(objID);
            selectnum=TxtboxGetSelectionNumber (TxtboxID);

            /* set highlights to same column on all */
            TxtboxID=FormGetObjID(500);
            TxtboxCursSetRow(TxtboxID,selectnum);
            TxtboxID=FormGetObjID(501);
            TxtboxCursSetRow(TxtboxID,selectnum);
            TxtboxID=FormGetObjID(502);
            TxtboxCursSetRow(TxtboxID,selectnum);
        }
    }

    DebugEnd();
}



T_void MailMainUIInit (T_void)
{
    DebugRoutine ("MailMainUIInit");

	/* load the form for this page */
	FormLoadFromFile ("MAILMAIN.FRM");

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (MailMainUIControl);
	/* make sure that the color cycling will work */
    /* double buffer drawing */

    GraphicUpdateAllGraphicsBuffered ();
    DebugEnd();
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/



T_void MailWriteUI (T_void)
{
    T_word32 delta=0, lastupdate=0;

    DebugRoutine ("MailWriteUI");

    /* load the mail main ui form */
    MailWriteUIInit ();

    /* go into generic control loop */
    FormGenericControl(&G_writeExit);

    DebugEnd();
}


T_void MailWriteUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
    DebugRoutine ("MailWriteUIControl");

    if (objstatus==BUTTON_ACTION_RELEASED && objtype==FORM_OBJECT_BUTTON)
    {
        if (objID==300)
        {
            /* send button pressed */
        }
        else if (objID==301)
        {
            /* cancel button pressed */
            G_writeExit=TRUE;
        }
    }

    else if (objtype==FORM_OBJECT_TEXTBOX)
    {

    }

    DebugEnd();
}



T_void MailWriteUIInit (T_void)
{
    DebugRoutine ("MailWriteUIInit");

	/* load the form for this page */
	FormLoadFromFile ("MAILWRIT.FRM");

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (MailWriteUIControl);
	/* make sure that the color cycling will work */
    /* double buffer drawing */

    GraphicUpdateAllGraphicsBuffered ();
    DebugEnd();
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


T_void MailReadUI (T_void)
{
    T_word32 delta=0, lastupdate=0;

    DebugRoutine ("MailReadUI");

    /* load the mail main ui form */
    MailReadUIInit ();

    /* go into generic control loop */
    FormGenericControl(&G_readExit);

    DebugEnd();
}


T_void MailReadUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
    DebugRoutine ("MailReadUIControl");

    if (objstatus==BUTTON_ACTION_RELEASED && objtype==FORM_OBJECT_BUTTON)
    {
        if (objID==303)
        {
            /* exit button pressed */
            G_readExit=TRUE;
        }
    }

    else if (objtype==FORM_OBJECT_TEXTBOX)
    {

    }

    DebugEnd();
}



T_void MailReadUIInit (T_void)
{
    DebugRoutine ("MailReadUIInit");

	/* load the form for this page */
	FormLoadFromFile ("MAILREAD.FRM");

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (MailReadUIControl);
	/* make sure that the color cycling will work */
    /* double buffer drawing */

    GraphicUpdateAllGraphicsBuffered ();
    DebugEnd();
}

