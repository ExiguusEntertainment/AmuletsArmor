/*-------------------------------------------------------------------------*
 * File:  MAINUI.C
 *-------------------------------------------------------------------------*/
/**
 * The first main user interface for selecting/creating/delete characters
 * is here.
 *
 * @addtogroup MainUI
 * @brief Main UI for Character Creation
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BUTTON.H"
#include "COLOR.H"
#include "MAINUI.H"
#include "PICS.H"
#include "PROMPT.H"
#include "STATS.H"
#include "VIEW.H"

#define MAINUI_LOAD_CHARACTER_BUTTON    304
#define MAINUI_CREATE_CHARACTER_BUTTON  305
#define MAINUI_DELETE_CHARACTER_BUTTON  306
#define MAINUI_EXIT_BUTTON              307
//#define MAINUI_BULLETIN_TEXT            500
//#define MAINUI_BULLETIN_LIST            501
#define MAINUI_CHARACTER_LIST           502

static E_Boolean G_exit=FALSE;
static E_Boolean G_play=TRUE;
//static T_byte8 G_bulletinSelected=0;
static T_byte8 G_characterSelected=0;
static T_void MainUClearPortraitArea();
static T_void MainUIUpdateCharacterListing (T_void);

E_Boolean MainUI (T_void)
{

    DebugRoutine ("MainUI");

    G_play=TRUE;

    ViewSetPalette(VIEW_PALETTE_STANDARD) ;

    /* load the main ui form */
    MainUIInit ();

    /* go into generic control loop */
    FormGenericControl (&G_exit);

    DebugEnd();
    return (G_play);
}



T_void MainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
	T_byte8 tempstr[64];
    T_byte8 selected;
    T_statsSavedCharacterID* chardata;
	T_TxtboxID TxtboxID;

	DebugRoutine ("MainUIControl");

    if (objID==MAINUI_LOAD_CHARACTER_BUTTON &&
        objstatus==BUTTON_ACTION_RELEASED)
    {
        /* get the selection number of the character to be loaded */
        selected=(T_byte8)TxtboxGetSelectionNumber(FormGetObjID(MAINUI_CHARACTER_LIST));
//        DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);
        if (selected >= MAX_CHARACTERS_PER_SERVER) selected=0;

        /* LES:  Make the selected item the active one. */
        StatsMakeActive(G_characterSelected) ;

        ///* load character selected, go to the load character UI */
        //if (StatsLoadCharacter((T_byte8)selected)==TRUE)
        //{
        //    SMCChooseSetFlag(SMCCHOOSE_FLAG_CHOOSE_LOAD, TRUE) ;
        //}
        //else
        //{
        //    PromptDisplayMessage ("Character not available.");
        //    SMCChooseSetFlag(SMCCHOOSE_FLAG_CHOOSE_REDRAW, TRUE) ;
        //}
    }
    else if (objID==MAINUI_CREATE_CHARACTER_BUTTON &&
             objstatus==BUTTON_ACTION_RELEASED)
    {
        GraphicUpdateAllGraphics();
        /* create character selected, go to the create character UI */

        /* get the selection number of the character to be loaded */
        selected=(T_byte8)TxtboxGetSelectionNumber(
                     FormGetObjID(MAINUI_CHARACTER_LIST));
//        DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);
        if (selected >= MAX_CHARACTERS_PER_SERVER) selected=0;

        /* LES:  Make the selected item the active one. */
        StatsMakeActive(G_characterSelected) ;

        chardata=StatsGetSavedCharacterIDStruct (selected);

        FormCleanUp();

        //if (chardata->status==CHARACTER_STATUS_UNDEFINED)
        //{
        //    SMCChooseSetFlag(SMCCHOOSE_FLAG_CHOOSE_CREATE, TRUE) ;
        //}
        //else
        //{
        //    strcpy (tempstr,"^001Character slot filled - ^003Delete^001 character first.");
        //    PromptDisplayMessage (tempstr);
        //}

        MainUIInit();
    }
    else if (objID == MAINUI_DELETE_CHARACTER_BUTTON &&
             objstatus==BUTTON_ACTION_RELEASED)
    {
        selected=(T_byte8)TxtboxGetSelectionNumber(
                     FormGetObjID(MAINUI_CHARACTER_LIST));
//        DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);
        if (selected >= MAX_CHARACTERS_PER_SERVER) selected=0;

        /* LES:  Make the selected item the active one. */
        StatsMakeActive(G_characterSelected) ;
        chardata=StatsGetSavedCharacterIDStruct(StatsGetActive());
        if (chardata->status < CHARACTER_STATUS_UNDEFINED)
        {
            //SMCChooseSetFlag(SMCCHOOSE_FLAG_CHOOSE_DELETE, TRUE) ;
        } else {
            strcpy (tempstr,"^001Character slot not filled.");
            PromptDisplayMessage (tempstr);
            MainUIInit();
        }
#if 0
        /* delete selected character */
        selected=TxtboxGetSelectionNumber(FormGetObjID(MAINUI_CHARACTER_LIST));
        if (selected >= MAX_CHARACTERS_PER_SERVER) selected=0;

        /* LES:  Make the selected item the active one. */
        StatsMakeActive(G_characterSelected) ;

        /* check to make sure user wants to do this action */
        /* by prompting for the password for this character */
        /* check password code here */
        /* if password is wrong, say so. */

        /* get character id structure for selected character */
        chardata=StatsGetSavedCharacterIDStruct (selected);
        if (chardata->status < CHARACTER_STATUS_UNDEFINED)
        {
            /* remove current mainui form from memory so that prompt may display */
            FormCleanUp();

            /* get password */
            sprintf (tempstr,"^001Enter password for ^003%s^001 to confirm ^021delete",chardata->name);
            strcpy (password,"");
            if (PromptForString  (tempstr,12,password)==TRUE)
            {
                /* check to see if password is correct */
                /* fake server transaction code here */
                MainUIInit();
                GraphicUpdateAllGraphics();
                FormCleanUp();

                if (strcmp (password,chardata->password)==0)
                {
                    /* success. character will be deleted */
                    /*         delete code here           */
                    StatsDeleteCharacter (selected);
                }
                else
                {
                    strcpy (tempstr,"Improper password. Delete command canceled.");
                    PromptDisplayMessage(tempstr);
                    /* failure. return to mainui after notification of failure */
                }
            }

            /* restore MainUI */
            MainUIInit();
            MainUIUpdateCharacterListing();
            GraphicUpdateAllGraphics();
        }
#endif
    }
    else if (objID == MAINUI_EXIT_BUTTON &&
             objstatus==BUTTON_ACTION_RELEASED)
    {
        //SMCChooseSetFlag(SMCCHOOSE_FLAG_EXIT, TRUE) ;
    }
#if 0
    else if (objID == MAINUI_BULLETIN_LIST &&
             objstatus == Txtbox_ACTION_SELECTION_CHANGED)
    {
        /* load new bulletin */
        TxtboxID=FormGetObjID (MAINUI_BULLETIN_LIST);
        showwindowID=FormGetObjID (MAINUI_BULLETIN_TEXT);
        DebugCheck (TxtboxID != NULL);
        DebugCheck (showwindowID != NULL);
        G_bulletinSelected=TxtboxGetSelectionNumber(TxtboxID);
        MainUIShowBulletin (showwindowID,G_bulletinSelected);
    }
#endif
    else if (objID == MAINUI_CHARACTER_LIST &&
             objstatus == Txtbox_ACTION_SELECTION_CHANGED)
    {
        /* character selected changed */
        TxtboxID=FormGetObjID (MAINUI_CHARACTER_LIST);
        DebugCheck (TxtboxID != NULL);

        selected=(T_byte8)TxtboxGetSelectionNumber(TxtboxID);
        if (G_characterSelected != selected)
        {
            G_characterSelected=selected;

            /* LES:  Make the selected item the active one. */
            StatsMakeActive(G_characterSelected);
            chardata=StatsGetSavedCharacterIDStruct (G_characterSelected);
            if (chardata->status < CHARACTER_STATUS_UNDEFINED)
            {
                StatsLoadCharacter(G_characterSelected);
                StatsDrawCharacterPortrait(180,79);
            }
            else
            {
                MainUClearPortraitArea();
//                GrDrawRectangle(180,79,295,181,77);
            }
        }
    }


	DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  MainUISetUpBulletins
 *-------------------------------------------------------------------------*/
/**
 *  This function requests a list of bulletins from the server
 *
 *  NOTE: 
 *  Doesn't do anything (yet)
 *
 *<!-----------------------------------------------------------------------*/
#if 0
T_void MainUISetUpBulletins (T_TxtboxID bulletinID)
{
    T_byte8 bulltitle[512];

    DebugRoutine ("MainUISetUpBulletins");

    DebugCheck (bulletinID != NULL);

    sprintf (bulltitle,"1. Greetings\r2. Update information\r3. Customer Support\r4. Coming Soon\r5. About LDS");
    TxtboxAppendString (bulletinID,bulltitle);
    TxtboxCursTop (bulletinID);

    DebugEnd();
}
#endif

#if 0
T_void MainUIShowBulletin (T_TxtboxID showwindowID, T_word16 number)
{
    FILE *fp;
    T_byte8 tempstr[130];
    T_byte8 filename[32];

    DebugRoutine ("MainUIShowBulletin");
    DebugCheck (showwindowID != NULL);
    DebugCheck (number <5);

    if (number==0) sprintf (filename,"greet.bul");
    else if (number==1) sprintf (filename,"update.bul");
    else if (number==2) sprintf (filename,"customs.bul");
    else if (number==3) sprintf (filename,"coming.bul");
    else if (number==4) sprintf (filename,"about.bul");

    /* clear current text in bulletin showing window */
    TxtboxSetData (showwindowID, "");

    /* open bulletin file */
    fp=fopen (filename,"r");

    if (fp != NULL)
    {
        while (feof(fp)==FALSE)
        {
            /* get a line from the bulletin file */
            fgets (tempstr,128,fp);

			/* strip last (newline) character */
			if (tempstr[strlen(tempstr)-1]=='\n') tempstr[strlen(tempstr)-1]='\r';

            /* display the line */
            TxtboxAppendString (showwindowID,tempstr);
        }
    }

    /* close file */
    fclose (fp);

    /* move cursor to top */
    TxtboxCursTop (showwindowID);

    DebugEnd();
}
#endif


T_void MainUIInit (T_void)
{
    T_TxtboxID TxtboxID;
    T_statsSavedCharacterID* chardata;

    DebugRoutine ("MainUIInit");

 	/* load the form for this page */
	FormLoadFromFile ("MAINUI.FRM");

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (MainUIControl);

    /* set up windows */

    /* show initial (greeting) bulletin page */
/*    TxtboxID=FormGetObjID (MAINUI_BULLETIN_TEXT);
    if (TxtboxID != NULL)
    {
        MainUIShowBulletin (TxtboxID, 0);
    }
*/
    /* bulletin listing */
/*    TxtboxID=FormGetObjID (MAINUI_BULLETIN_LIST);
    if (TxtboxID != NULL)
    {
        MainUISetUpBulletins(TxtboxID);
        TxtboxCursSetRow (TxtboxID,G_bulletinSelected);
        showwindowID=FormGetObjID (MAINUI_BULLETIN_TEXT);
        MainUIShowBulletin (showwindowID,G_bulletinSelected);
    }
*/
    MainUIUpdateCharacterListing();

    GraphicUpdateAllGraphics();

    /* character selected changed */
    TxtboxID=FormGetObjID (MAINUI_CHARACTER_LIST);
    DebugCheck (TxtboxID != NULL);
    TxtboxCursSetRow (TxtboxID,G_characterSelected);

    /* LES:  Make the selected item the active one. */
    StatsMakeActive(G_characterSelected);
    chardata=StatsGetSavedCharacterIDStruct (G_characterSelected);
    if (chardata->status < CHARACTER_STATUS_UNDEFINED)
    {
        StatsLoadCharacter(G_characterSelected);
        StatsDrawCharacterPortrait(180,79);
    }
    else
    {
//        GrDrawRectangle(180,79,295,181,77);
    }

    GraphicUpdateAllGraphics();

    GrSetCursorPosition(5, 188);
    GrDrawShadowedText(VERSION_TEXT, 210, 0);

    DebugEnd();
}


static T_void MainUIUpdateCharacterListing (T_void)
{
    T_word16 i;
    T_TxtboxID TxtboxID;
    T_statsSavedCharacterID* charID;
    DebugRoutine ("MainUIUpdateCharacterListing");


    TxtboxID=FormGetObjID (MAINUI_CHARACTER_LIST);

    if (TxtboxID != NULL)
    {
        /* loop through list of saved character slots */
        /* and create listing */
        TxtboxSetData (TxtboxID,"");
        TxtboxCursTop (TxtboxID);

        for (i=0;i<MAX_CHARACTERS_PER_SERVER;i++)
        {
            charID=StatsGetSavedCharacterIDStruct(i);
            TxtboxAppendString (TxtboxID,charID->name);
            TxtboxAppendKey (TxtboxID,'\r');
        }
        TxtboxBackSpace (TxtboxID);
        TxtboxCursTop (TxtboxID);
    }

    DebugEnd();
}


T_void MainUIStart(T_void)
{
    DebugRoutine ("MainUIStart");

    /* load the main ui form */
    MainUIInit ();

    /* go into generic control loop */
    FormGenericControlStart();

    DebugEnd();
}

T_void MainUIUpdate(T_void)
{

    DebugRoutine ("MainUIUpdate");

    /* go into generic control loop update */
    FormGenericControlUpdate();
    GraphicUpdateAllGraphics();

    DebugEnd();
}

T_void MainUIEnd(T_void)
{

    DebugRoutine ("MainUIEnd");

    /* go into generic control loop */
    FormGenericControlEnd() ;

    DebugEnd();
}



static T_void MainUClearPortraitArea()
{
    T_resource res;
    T_byte8 *p_data;
    const T_word16 x1=180;
    const T_word16 y1=79;

    DebugRoutine ("MainUIClearPortraitArea");

//    ViewSetPalette(VIEW_PALETTE_STANDARD);

//    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);

//    GrDrawRectangle (x1,y1,x1+115,y1+102,0);
    if (PictureExist ("UI/CREATEC/CHARINFO"))
    {
        res=PictureFind("UI/CREATEC/CHARINFO");
        p_data=PictureLockByResource (res);
        DebugCheck (p_data != NULL);
        ColorUpdate(0) ;
        GrDrawBitmap (PictureToBitmap(p_data),x1,y1);
        PictureUnlockAndUnfind (res);
    } else
    {
        /* clear area */
        GrDrawRectangle (x1,y1,x1+115,y1+102,55);
    }

    DebugEnd();
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  MAINUI.C
 *-------------------------------------------------------------------------*/
