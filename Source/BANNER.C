/*-------------------------------------------------------------------------*
 * File:  BANNER.C
 *-------------------------------------------------------------------------*/
/**
 * The Banner UI system is the top level system for handling the player's
 * banner at the bottom of the screen.  It tracks the current sub-window
 * open as well as spells being cast.
 *
 * @addtogroup BANNER
 * @brief Player's Bottom Banner User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BANNER.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "COMWIN.H"
#include "CONFIG.H"
#include "CONTROL.H"
#include "GENERAL.H"
#include "GRAPHIC.H"
#include "HARDFORM.H"
#include "INIFILE.H"
#include "INVENTOR.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"
#include "LOOK.H"
#include "MESSAGE.H"
#include "NOTES.H"
#include "OBJECT.H"
#include "PICS.H"
#include "SOUND.H"
#include "SPELLS.H"
#include "STATS.H"
#include "TICKER.H"

#define MAX_POTION_PICS 8
#define MANA_BACKCOLOR 225
#define NUM_AMMO_SLOTS 7

typedef struct {
        T_word16 x;
        T_word16 y;
        T_byte8 *p_name;
        E_Boolean toggleType;
        T_word16 key;
        T_buttonHandler p_pressCallback;
        T_buttonHandler p_releaseCallback;
} T_buttonCreateParms;

static T_resource G_potionPics[MAX_POTION_PICS];
static E_Boolean G_potionCreated = FALSE;
static E_Boolean G_bannerStatsCreated = TRUE;
static T_word16 G_musicVol = 65000;
static T_word16 G_sfxVol = 65000;
static E_Boolean G_musicOn = TRUE;
static E_Boolean G_sfxOn = TRUE;
static E_Boolean G_bannerUIMode = FALSE;
static E_Boolean G_bannerIsOpen = FALSE;
static E_bannerFormType G_currentBannerFormType = BANNER_FORM_INVENTORY;
static E_bannerFormType G_bannerLastForm = BANNER_FORM_UNKNOWN;
static T_TxtboxID G_bannerStatBoxes[5];
#define NUMBER_BANNER_BUTTONS 11
#define NUMBER_RUNE_BUTTONS 9
static E_Boolean G_bannerButtonsCreated = FALSE;
static T_buttonID G_bannerButtons[NUMBER_BANNER_BUTTONS] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL };
static T_buttonID G_runeButtons[NUMBER_RUNE_BUTTONS] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL };
//static T_buttonID G_ammoButtons[NUM_AMMO_SLOTS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
//static T_graphicID G_ammoGraphics[NUM_AMMO_SLOTS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static T_byte8 G_ammoSelected = 0;
static T_word16 G_ammoTypeSelected = 0;
static T_byte8 G_numAmmoSlots = 0;

/* internal routines here */
static T_void BannerGetCoin(T_buttonID buttonID);
static T_void BannerSelectAmmo(T_buttonID buttonID);
static T_void BannerGetAmmo(T_buttonID buttonID);

/*-------------------------------------------------------------------------*
 * Routine:  BannerInit
 *-------------------------------------------------------------------------*/
/**
 *  BannerInit sets up all the parts necessary to display the banner
 *  at the bottom of the screen.
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerInit(T_void)
{
    T_bitmap *b1;
    T_resource r1;
//    static E_Boolean firstin=TRUE;
    DebugRoutine("BannerInit");

    /* Draw the boundaries of the screen. */
    b1 = (T_bitmap *)PictureLockData("UI/3DUI/MAINBACK", &r1);
    GrDrawBitmap(b1, 0, 0);
    PictureUnlock(r1);
    PictureUnfind(r1);

    /* draw a black box over button areas to fix slight graphic error */
    /* when re-drawing a 'pushed' button.. */
    GrDrawRectangle(4, 154, 55, 196, 0);

    /* create button controls */
    BannerCreateBottomButtons();
    /* initialize potions */
    PotionInit();

    /* draw status bars */
    BannerStatusBarInit();

    /* redraw any open menus */
    if (G_bannerIsOpen == TRUE)
        BannerOpenForm(G_currentBannerFormType);

    /* update status bar */
    BannerStatusBarUpdate();

    /* draw potions */
    PotionUpdate();

    /* draw mana display */
    BannerUpdateManaDisplay();

    /* redraw active runes */
    SpellsDrawRuneBox();

    /* draw ready area */
    InventoryDrawReadyArea();

//    firstin=FALSE;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  BannerUpdate
 *-------------------------------------------------------------------------*/
/**
 *  BannerUpdate updates all elements of the banner as necessary.
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerUpdate(T_void)
{
    static T_word32 lastupdate = 0;
    T_word32 time = 0;
    T_word32 delta = 0;

    DebugRoutine("BannerUpdate");

    time = TickerGet();
    delta = time - lastupdate;

    if (delta > 8) {
        PotionUpdate();
        BannerUpdateManaDisplay();
        lastupdate = TickerGet();
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  BannerFinish
 *-------------------------------------------------------------------------*/
/**
 *  BannerFinish closes out all the stuff that makes up the banner.
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerFinish(T_void)
{
//    FILE *fp;
    T_iniFile ini;
    char buffer[20];
    DebugRoutine("BannerFinish");

    BannerCloseForm();

    /* get rid of bottom buttons */
    BannerDestroyBottomButtons();

    /* get rid of potions */
    PotionFinish();

    /* get rid of status bar */
    BannerStatusBarFinish();

    /* write out music and sound volume options */
    ini = ConfigGetINIFile();
    sprintf(buffer, "%d", G_musicVol);
    INIFilePut(ini, "options", "musicVolume", buffer);
    sprintf(buffer, "%d", G_sfxVol);
    INIFilePut(ini, "options", "sfxVolume", buffer);
    sprintf(buffer, "%d", G_musicOn);
    INIFilePut(ini, "options", "musicOn", buffer);
    sprintf(buffer, "%d", G_sfxOn);
    INIFilePut(ini, "options", "sfxOn", buffer);
#if 0
    fp=fopen ("opts.dat","wb");

    if (fp != NULL)
    {
        fprintf (fp,"%d %d %d %d",G_musicVol, G_sfxVol, G_musicOn, G_sfxOn);
    }

    /* close file */
    fclose (fp);
#endif

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerOpenForm
 *-------------------------------------------------------------------------*/
/**
 *  BannerOpenForm changes the view to small screen and displays
 *  a form on the right side.  Also, BannerOpenForm creates 6 buttons
 *  for the 'main menu'
 *
 *  NOTE: 
 *  Currently only supports main menus (i.e. always adds main menu
 *  buttons on top of the form
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerOpenForm(E_bannerFormType formtype)
{
    T_byte8 stmp[48];
    T_byte8 *p_data;
    T_resource res;
    T_word16 i;
    T_buttonID buttonID;
    T_sliderID sliderID;
    T_TxtboxID TxtboxID;
    T_byte8 credits[2048];

    DebugRoutine("BannerOpenForm");
//  printf ("opening form%d\n",formtype);
//  fflush (stdout);

    DebugCheck(formtype < BANNER_FORM_UNKNOWN);

    if (BannerFormIsOpen(BANNER_FORM_NOTES))
        NotesCloseNotesPage();

    /* check to see if we are opening the default form */
    if (formtype == BANNER_FORM_CURRENT) {
        formtype = G_currentBannerFormType;
    }

    /* call any necessary de-initialization routines */
    switch (G_currentBannerFormType) {
        case BANNER_FORM_COMMUNICATE:
            ComwinCloseCommunicatePage();
            KeyboardAllowKeyScans();
            break;

        case BANNER_FORM_NOTES:
            KeyboardAllowKeyScans();
            break;

        default:
            break;
    }

    /* load the form file */
    sprintf(stmp, "BAN%5.5d.FRM", formtype);
    FormLoadFromFile(stmp);
//    MessageAdd ("form loaded\n");
    /* set the form callback routine */
    FormSetCallbackRoutine(BannerFormControl);

    /* set the window to half screen view */
    View3dClipCenter(205);
//    GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;

    /* update the graphics */
    GraphicUpdateAllGraphics();

    /* set the current banner */
    G_currentBannerFormType = formtype;
    G_bannerIsOpen = TRUE;

    /* call any necessary initialization routines */
    switch (formtype) {
        case BANNER_FORM_INVENTORY:
            InventoryDrawInventoryWindow(INVENTORY_PLAYER);
            break;

        case BANNER_FORM_OPTIONS:
            buttonID = FormGetObjID(301);
            if (buttonID != NULL) {
                ButtonSetSelectPic(buttonID, "UI/CREATEC/CRC_TOG1");
                if (G_musicOn)
                    ButtonDownNoAction(buttonID);
            }
            buttonID = FormGetObjID(302);
            if (buttonID != NULL) {
                ButtonSetSelectPic(buttonID, "UI/CREATEC/CRC_TOG1");
                if (G_sfxOn)
                    ButtonDownNoAction(buttonID);
            }

            sliderID = FormGetObjID(600);
            DebugCheck(sliderID != NULL);
            SliderSetValue(sliderID, G_musicVol);
            sliderID = FormGetObjID(601);
            DebugCheck(sliderID != NULL);
            SliderSetValue(sliderID, G_sfxVol);
            buttonID = FormGetObjID(301);

            TxtboxID = FormGetObjID(500);

            /* set up credits list */
            strcpy(credits, "\r^036    Amulets and Armor\r\r");

            strcat(credits, "^009           EXIGUUS    \r");
            strcat(credits, "^009       ENTERTAINMENT  \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      DAVID WEBSTER   \r");
            strcat(credits, "^014      LYSLE SHIELDS   \r\r");

            strcat(credits, "^009         PRODUCERS    \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      DAVID WEBSTER   \r");
            strcat(credits, "^014       ERIC WEBSTER   \r\r");

            strcat(credits, "^009       PROGRAMMING    \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      LYSLE SHIELDS   \r");
            strcat(credits, "^014      JANUS ANDERSON  \r\r");

            strcat(credits, "^009             ART      \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013       JOEL THOMAS    \r");
            strcat(credits, "^014       JOHN HILEMAN   \r");
            strcat(credits, "^015        DEAN BEERS    \r\r");

            strcat(credits, "^009     SOUND AND MUSIC  \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      JANUS ANDERSON  \r");
            strcat(credits, "^014         BILLY FOX    \r\r");

            strcat(credits, "^009        MAP DESIGN    \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      EDWARD SINGER   \r");
            strcat(credits, "^014      TYCE TOOTHAKER  \r");
            strcat(credits, "^015      DAVID WEBSTER   \r\r");

            strcat(credits, "^009         STORY AND    \r");
            strcat(credits, "^009       GAME CONCEPT   \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      DAVID WEBSTER   \r\r");

            strcat(credits, "^009      GAME MECHANICS  \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      LYSLE SHIELDS   \r");
            strcat(credits, "^014      JANUS ANDERSON  \r\r");

            strcat(credits, "^009      USER INTERFACE  \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      JANUS ANDERSON  \r\r");

            strcat(credits, "^009      ADDITIONAL ART  \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013     TIFFANY WEBSTER  \r");
            strcat(credits, "^013      DAVID WEBSTER   \r");
            strcat(credits, "^014     TENLEY THURSTON  \r");
            strcat(credits, "^015      JANUS ANDERSON  \r");
            strcat(credits, "^016    MICHAEL KIRKBRIDE \r");
            strcat(credits, "^017       LYSLE SHIELDS  \r\r");

            strcat(credits, "^009      DOCUMENTATION   \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013      JANUS ANDERSON  \r\r");

            strcat(credits, "^009        ASSISTANCE    \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^014         GREG HAAS    \r");
            strcat(credits, "^015      MIKE TOOTHAKER  \r");
            strcat(credits, "^016      AARON TRICKEY   \r\r");

            strcat(credits, "^009        PLAYTESTERS   \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^013       ERIC SHAMLIN   \r");
            strcat(credits, "^014    CHARLES E. MARTIN \r");
            strcat(credits, "^015         TERRY LUNA   \r");
            strcat(credits, "^016       JASON HOLMES   \r");
            strcat(credits, "^017         BILL IRWIN   \r\r");
            strcat(credits, "^011   A Webster Brothers \r");
            strcat(credits, "^011         Production   \r\r");
            strcat(credits, "^011      (C)1996 EXIGUUS \r");
            strcat(credits, "^011       ENTERTAINMENT  \r");
            strcat(credits, "^011   ALL RIGHTS RESERVED\r\r");

            strcat(credits, "^009          WEB SITE   \r");
            strcat(credits, "^010    ----------------- \r");
            strcat(credits, "^036  amuletsandarmor.com\r\r");

            TxtboxSetData(TxtboxID, credits);
            TxtboxCursTop(TxtboxID);

            break;

        case BANNER_FORM_STATISTICS:
            StatsDisplayStatisticsPage();
            /* display the character graphic */
            sprintf(stmp, "UI/3DUI/SMPORT%02d", StatsGetPlayerClassType());

            if (PictureExist(stmp)) {
                res = PictureFind(stmp);
                p_data = PictureLockQuick(res);
                DebugCheck(p_data != NULL);
                GrDrawBitmap(PictureToBitmap(p_data), 215, 27);
                PictureUnlockAndUnfind(res);
            }
            break;

        case BANNER_FORM_COMMUNICATE:
            KeyboardDisallowKeyScans();
            ComwinDisplayCommunicatePage();
            break;

        case BANNER_FORM_EQUIPMENT:
            InventoryDrawEquipmentWindow();
            break;

        case BANNER_FORM_FINANCES:
            BannerDisplayFinancesPage();
            break;

        case BANNER_FORM_AMMO:
            /* set alternate button pictures and callbacks for buttons */
            for (i = 0; i < NUM_AMMO_SLOTS; i++) {
                buttonID = FormGetObjID(301 + i);
//            ButtonSetSelectPic (buttonID,"UI/3DUI/AMMSELDN");
                ButtonSetCallbacks(buttonID, BannerSelectAmmo,
                        ButtonDownNoAction);
                buttonID = FormGetObjID(308 + i);
                ButtonSetCallbacks(buttonID, NULL, BannerGetAmmo);
            }
            BannerDisplayAmmoPage();
            break;

        case BANNER_FORM_NOTES:
            KeyboardDisallowKeyScans();
            NotesOpenNotesPage();
            break;

        case BANNER_FORM_JOURNAL:
            NotesOpenJournalPage();
            break;

        case BANNER_FORM_LOOK:
            LookInit();
            break;

        case BANNER_FORM_CONTROL:
            ControlDisplayControlPage();
            break;

        default:
            break;
    }

    /* fix the menu buttons */
    if (G_bannerButtonsCreated == TRUE) {
        for (i = 0; i < 9; i++) {
            if (i + 1 != formtype)
                ButtonUpNoAction(G_bannerButtons[i]);
            else
                ButtonDownNoAction(G_bannerButtons[i]);
        }
    }

    DebugEnd();
}



/*-------------------------------------------------------------------------*
 * Routine:  BannerCloseForm
 *-------------------------------------------------------------------------*/
/**
 *  Closes any current form displayed on the right side of the window
 *  Deletes main menu buttons
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerCloseForm(T_void)
{
    T_graphicID graphic;

    DebugRoutine("BannerCloseForm");

    if (BannerFormIsOpen(BANNER_FORM_NOTES))
        NotesCloseNotesPage();

    /* delete any current form */
    FormCleanUp();
    FormSetCallbackRoutine(NULL);
    /* restore the display screen to full view */

    View3dClipCenter(312);
    if (HardFormIsOpen() == TRUE) {
        graphic = GraphicCreate(209, 0, "UI/3DUI/CLOSEDBA");
        GraphicUpdateAllGraphics();
        GraphicDelete(graphic);
    }

//	 GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
//	 GrDrawFrame (3,2,316,151,71);

    if (G_currentBannerFormType
            > 0&& G_currentBannerFormType < 10 && G_bannerButtonsCreated==TRUE)ButtonUpNoAction
        (G_bannerButtons[G_currentBannerFormType - 1]);

    /* deinitialize */
    switch (G_currentBannerFormType) {
        case BANNER_FORM_COMMUNICATE:
            ComwinCloseCommunicatePage();
            KeyboardAllowKeyScans();
            break;

        case BANNER_FORM_NOTES:
            KeyboardAllowKeyScans();
            break;

        default:
            break;
    }

    G_bannerIsOpen = FALSE;

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerOpenFormsByButton
 *-------------------------------------------------------------------------*/
/**
 *  Calls BannerOpenForm with the formtype specified by Button->data
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerOpenFormByButton(T_buttonID buttonID)
{
    DebugRoutine("BannerOpenFormByButton");
    DebugCheck(buttonID != NULL);

    /* initialize form specified by data set in button */
    BannerOpenForm(ButtonGetData(buttonID));

    DebugEnd();
}


T_void BannerCloseFormByButton(T_buttonID buttonID)
{
    DebugRoutine("BannerCloseFormByButton");

    BannerCloseForm();

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerFormControl
 *-------------------------------------------------------------------------*/
/**
 *  FormCallBack routine set to control events for forms added
 *  to gaming window display area
 *
 *<!-----------------------------------------------------------------------*/

T_void BannerFormControl(
        E_formObjectType objtype,
        T_word16 objstatus,
        T_word32 objID)
{
    T_sliderID MusicVolSlider, SfxVolSlider;
    T_buttonID MusicVolButton, SfxVolButton;

    DebugRoutine("BannerFormControl");

//    printf ("in form control\n");
//    printf ("type=%d\n, status=%d\n, ID=%d\n",objtype,objstatus,objID);
//    fflush (stdout);

    if (objID == 300 && objtype == FORM_OBJECT_BUTTON
            && objstatus == BUTTON_ACTION_RELEASED) {
        BannerCloseForm();
    }

    else {
        switch (G_currentBannerFormType) {
            case BANNER_FORM_INVENTORY: {
                if (objstatus == BUTTON_ACTION_RELEASED) {
                    if (objID == 301) {
                        /* eat selected */
                        //ClientEatOn();
                    } else if (objID == 302) {
                        /* equipment page selected */
                        BannerOpenForm(BANNER_FORM_EQUIPMENT);
                    } else if (objID == 305) {
                        /* last inventory window selected */
                        InventorySelectLastInventoryPage(INVENTORY_PLAYER);
                    } else if (objID == 306) {
                        /* next inventory window selected */
                        InventorySelectNextInventoryPage(INVENTORY_PLAYER);
                    }
                }
            }
                break;

            case BANNER_FORM_OPTIONS:
                if (objtype == FORM_OBJECT_BUTTON
                        || objtype == FORM_OBJECT_SLIDER) {
                    if (objID == 301 || objID == 302 || objID == 600
                            || objID == 601) {
                        MusicVolButton = FormGetObjID(301);
                        SfxVolButton = FormGetObjID(302);
                        MusicVolSlider = FormGetObjID(600);
                        SfxVolSlider = FormGetObjID(601);

                        if (ButtonIsPushed(MusicVolButton) == TRUE)
                            G_musicOn = TRUE;
                        else
                            G_musicOn = FALSE;

                        if (ButtonIsPushed(SfxVolButton) == TRUE)
                            G_sfxOn = TRUE;
                        else
                            G_sfxOn = FALSE;
                        ;

                        G_musicVol = (SliderGetValue(MusicVolSlider));
                        G_sfxVol = (SliderGetValue(SfxVolSlider));

                        if (G_musicOn == TRUE)
                            SoundSetBackgroundVolume(G_musicVol >> 8);
                        else
                            SoundSetBackgroundVolume(0);

                        if (G_sfxOn == TRUE)
                            SoundSetEffectsVolume(G_sfxVol >> 8);
                        else
                            SoundSetEffectsVolume(0);
                    }
                }
                break;

            case BANNER_FORM_EQUIPMENT: {
                if (objtype == FORM_OBJECT_BUTTON
                        && objstatus == BUTTON_ACTION_RELEASED) {
                    if (objID == 301) {
                        /* to inventory page button selected */
                        BannerOpenForm(BANNER_FORM_INVENTORY);
                    }
                }
            }
                break;

            case BANNER_FORM_AMMO:
                break;

            default:
                break;
        }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  BannerCreateBottomButtons
 *-------------------------------------------------------------------------*/
/**
 *  BannerCreateBottomButtons creates all the buttons that make up
 *  the banner at the bottom of the screen.
 *
 *  NOTE: 
 *  Don't call if already created.
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerCreateBottomButtons(T_void)
{
    T_byte8 i ;
    T_byte8 spic[]="UI/3DUI/RUDBUT";

    static T_buttonCreateParms buttonParms[NUMBER_BANNER_BUTTONS] = {
        { 5,155,"UI/3DUI/MENUBUT1",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_7,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
        {22,155,"UI/3DUI/MENUBUT2",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_8,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
	    {39,155,"UI/3DUI/MENUBUT3",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_9,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
        { 5,169,"UI/3DUI/MENUBUT4",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_4,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
	    {22,169,"UI/3DUI/MENUBUT5",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_5,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
	    {39,169,"UI/3DUI/MENUBUT6",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_6,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
        { 5,183,"UI/3DUI/MENUBUT7",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_1,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
//	    {39,169,"UI/3DUI/MENUBUTN",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_6,KEY_SCAN_CODE_ALT),NULL,NULL},
//	    { 5,183,"UI/3DUI/MENUBUTN",FALSE,KeyDual(KEY_SCAN_CODE_KEYPAD_1,KEY_SCAN_CODE_ALT),NULL,NULL},
	    {22,183,"UI/3DUI/MENUBUT8",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_2,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
	    {39,183,"UI/3DUI/MENUBUT9",TRUE,KeyDual(KEY_SCAN_CODE_KEYPAD_3,KEY_SCAN_CODE_ALT),BannerOpenFormByButton,BannerCloseFormByButton},
	    {240,181,"UI/3DUI/CSTBUT2" ,FALSE,KEY_SCAN_CODE_KEYPAD_ENTER,SpellsCastSpell,NULL},
	    { 93,186,"UI/3DUI/MUSEBUT2" ,FALSE,KEY_SCAN_CODE_CTRL,NULL,NULL}
    } ;
    static T_byte8 buttonKeyMaps[NUMBER_BANNER_BUTTONS] = {
        KEYMAP_INVENTORY,
        KEYMAP_EQUIPMENT,
        KEYMAP_STATISTICS,
        KEYMAP_OPTIONS,
        KEYMAP_COMMUNICATE,
        KEYMAP_FINANCES,
        KEYMAP_AMMUNITION,
        KEYMAP_NOTES,
        KEYMAP_JOURNAL,
        KEYMAP_CAST_SPELL,
        KEYMAP_USE
    } ;

    DebugRoutine("BannerCreateBottomButtons");
    DebugCheck(G_bannerButtonsCreated == FALSE);

    /* LES: Fix up the correct keys for the buttons. */
    for (i = 0; i < NUMBER_BANNER_BUTTONS; i++) {
        if (i < 9) {
            buttonParms[i].key =
                    KeyDual(KeyMap(buttonKeyMaps[i]), KEY_SCAN_CODE_ALT);
        } else {
            buttonParms[i].key = KeyMap(buttonKeyMaps[i]);
        }
    }

    for (i = 0; i < NUMBER_BANNER_BUTTONS; i++) {
        DebugCheck(G_bannerButtons[i]==NULL);
        G_bannerButtons[i] = ButtonCreate(buttonParms[i].x, buttonParms[i].y,
                buttonParms[i].p_name, buttonParms[i].toggleType,
                buttonParms[i].key, buttonParms[i].p_pressCallback,
                buttonParms[i].p_releaseCallback);

        DebugCheck(G_bannerButtons[i]!=NULL);
    }
    for (i = 0; i < 9; i++)
        ButtonSetData(G_bannerButtons[i], i + 1);

    G_bannerButtonsCreated = TRUE;
    /* Make the buttons draw immediately. */
//    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);
    /* replace rune buttons if necessary */
    for (i = 0; i < NUMBER_RUNE_BUTTONS; i++) {
        if (StatsRuneIsAvailable(i)) {
            BannerRemoveSpellButton(i);
            BannerAddSpellButton(i);
        }
    }

    ButtonRedrawAllButtons();

    DebugEnd();
}


E_Boolean BannerUseButtonIsDown(T_void)
{
    E_Boolean isdown = FALSE;

    DebugRoutine("BannerUseButtonIsDown");

    if (ButtonIsPushed(G_bannerButtons[10]))
        isdown = TRUE;

    DebugEnd();
    return (isdown);
}

/*-------------------------------------------------------------------------*
 * Routine:  BannerDestoryBottomButtons
 *-------------------------------------------------------------------------*/
/**
 *  BannerDestroyBottomButtons gets rid of all the buttons needed by the
 *  bottom banner.
 *
 *  NOTE: 
 *  You can call this multiple times.
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerDestroyBottomButtons(T_void)
{
    T_word16 i;

    DebugRoutine("BannerDestoryBottomButtons");
    /* Destroy each button individually. */
    if (G_bannerButtonsCreated == TRUE) {
        for (i = 0; i < NUMBER_BANNER_BUTTONS; i++) {
            DebugCheck(G_bannerButtons[i]!=NULL);
            if (G_bannerButtons[i] != NULL) {
                ButtonDelete(G_bannerButtons[i]);
//                printf ("deleting banner button %d\n",i);
//                fflush (stdout);
                G_bannerButtons[i] = NULL;
            }
        }
        for (i = 0; i < NUMBER_RUNE_BUTTONS; i++) {
//          DebugCheck (G_runeButtons[i]!=NULL);
            if (G_runeButtons[i] != NULL) {
                ButtonDelete(G_runeButtons[i]);
//                printf ("deleting rune button %d\n",i);
//                fflush (stdout);
                G_runeButtons[i] = NULL;
            }
        }

        G_bannerButtonsCreated = FALSE;
    }
    DebugEnd();
}

T_void BannerRedrawBottomButtons(T_void)
{
    T_word16 i;

    DebugRoutine("BannerRedrawBottomButtons");

    if (G_bannerButtonsCreated == TRUE) {
        for (i = 0; i < NUMBER_BANNER_BUTTONS; i++)
            ButtonRedraw(G_bannerButtons[i]);
        for (i = 0; i < NUMBER_RUNE_BUTTONS; i++)
            ButtonRedraw(G_runeButtons[i]);
    }

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  PotionInit
 *-------------------------------------------------------------------------*/
/**
 *  PotionInit initializes all the pictures for the potion display on the
 *  banner.
 *
 *<!-----------------------------------------------------------------------*/
T_void PotionInit(T_void)
{
    T_byte8 potionName[] = "UI/3DUI/POTION?";
    T_word16 i;

    DebugRoutine("PotionInit");
    /* Find each of the pictures so we can lock them into memory. */
    for (i = 0; i < MAX_POTION_PICS; i++) {
        potionName[14] = i + '0';

        G_potionPics[i] = PictureFind(potionName);
    }

    G_potionCreated = TRUE;
    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PotionInit
 *-------------------------------------------------------------------------*/
/**
 *  PotionInit removes all pictures used by the potion display on the
 *  banner.
 *
 *<!-----------------------------------------------------------------------*/
T_void PotionFinish(T_void)
{
    T_word16 i;

    DebugRoutine("PotionFinish");

    if (G_potionCreated == TRUE) {
        /* Remove each picture's reference. */
        for (i = 0; i < MAX_POTION_PICS; i++)
            PictureUnfind(G_potionPics[i]);
        G_potionCreated = FALSE;
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PotionUpdate
 *-------------------------------------------------------------------------*/
/**
 *  PotionUpdate animates the potion
 *
 *<!-----------------------------------------------------------------------*/
T_void PotionUpdate(T_void)
{
    static T_byte8 potionFrame = 0;
    const float potiondepth = 192.0 - 157.0;
    static T_byte8 bubbled[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static T_byte8 surface[4] = { 0, 0, 0, 0 };
    const T_byte8 potioncolor[4] = { 147, 38, 134, 54 };
    static T_byte8 pdepth[4] = { 0, 0, 0, 0 };

    T_screen curscreen, tempscreen;
    T_byte8 *p_data;
    T_resource res;
    T_word16 i;
    T_byte8 randno;

    float pcts[4];
    T_byte8 depth;

    DebugRoutine("PotionUpdate");

    if (G_potionCreated == TRUE) {
        potionFrame++;

        if (rand() % 5 == 2)
            potionFrame += 2;

        if (potionFrame >= MAX_POTION_PICS)
            potionFrame = 0;

        /* allocate hidden screen */
        curscreen = GrScreenGet();
        tempscreen = GrScreenAllocPartial(43);
        GrScreenSet(tempscreen);

        /* Lock in and draw the next picture. */
        res = G_potionPics[potionFrame];
        p_data = PictureLockQuick(res);
        DebugCheck(p_data != NULL);
        if (p_data != NULL) {
            GrDrawBitmap(PictureToBitmap(p_data), 61, 0);   //155
            PictureUnlock(res);
        }

        /* now, draw background red over 'top' of potion to show
         levels */

        pcts[0] = (float)StatsGetPlayerHealth()
                / (float)StatsGetPlayerMaxHealth();
        pcts[1] = (float)StatsGetPlayerMana() / (float)StatsGetPlayerMaxMana();
        pcts[2] = (float)StatsGetPlayerFood() / (float)StatsGetPlayerMaxFood();
        pcts[3] =
                (float)StatsGetPlayerWater() / (float)StatsGetPlayerMaxWater();

        for (i = 0; i < 4; i++) {
            depth = (int)((1.0 - pcts[i]) * potiondepth);
            if (depth > pdepth[i])
                pdepth[i]++;
            else if (depth < pdepth[i])
                pdepth[i]--;

            GrDrawRectangle(65 + i * 7, 2, 66 + i * 7, 2 + pdepth[i],
                    BANNER_BASE_COLOR);
            GrDrawLine(64 + i * 7, 2, 67 + i * 7, 2, BANNER_BASE_COLOR);

            if (pdepth[i] > 32)
                GrDrawRectangle(64 + i * 7, 34, 67 + i * 7,
                        34 + (pdepth[i] - 32), BANNER_BASE_COLOR);

            /* draw 'bubbles' */

            randno = rand() % 20;
            if (randno == 1)
                bubbled[i * 2] = 1;
            if (randno == 2)
                bubbled[i * 2 + 1] = 1;

            if (bubbled[i * 2] > 0 && pdepth[i] < potiondepth) {
                GrDrawPixel(65 + i * 7, 3 + pdepth[i] - bubbled[i * 2],
                        potioncolor[i]);
                bubbled[i * 2]++;
                if (bubbled[i * 2] > 2)
                    bubbled[i * 2] = 0;
            }
            if (bubbled[i * 2 + 1] > 0 && pdepth[i] < potiondepth) {
                GrDrawPixel(66 + i * 7, 3 + pdepth[i] - bubbled[i * 2 + 1],
                        potioncolor[i]);
                bubbled[i * 2 + 1]++;
                if (bubbled[i * 2 + 1] > 2)
                    bubbled[i * 2 + 1] = 0;
            }

            /* draw 'surface' */
            if (randno > 15)
                surface[i] = rand() % 3;

            if (surface[i] > 0 && pdepth[i] < potiondepth) {
                GrDrawPixel(64 + i * 7 + surface[i], 3 + pdepth[i],
                        BANNER_BASE_COLOR);
            }

        }

        /* transfer hidden screen */
        GrTransferRectangle(GRAPHICS_ACTUAL_SCREEN, 61, 0, 91, 40, 61, 155);

        GrScreenSet(GRAPHICS_ACTUAL_SCREEN);
        GrScreenFree(tempscreen);

    }
    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerStatusBarInit
 *-------------------------------------------------------------------------*/
/**
 *  Sets up the mini-status bar displayed at the bottom of the 3d banner
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerStatusBarInit(T_void)
{
    DebugRoutine("BannerStatusBarInit");

    G_bannerStatsCreated = TRUE;

    G_bannerStatBoxes[0] = TxtboxCreate(140, 156, 168, 162, "FontTiny", 0, 0,
            FALSE, TRUE, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL);

    G_bannerStatBoxes[1] = TxtboxCreate(140, 164, 168, 170, "FontTiny", 0, 0,
            FALSE, TRUE, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL);

    G_bannerStatBoxes[2] = TxtboxCreate(140, 172, 168, 178, "FontTiny", 0, 0,
            FALSE, TRUE, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL);

    G_bannerStatBoxes[3] = TxtboxCreate(140, 180, 168, 186, "FontTiny", 0, 0,
            FALSE, TRUE, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL);

    G_bannerStatBoxes[4] = TxtboxCreate(140, 188, 168, 194, "FontTiny", 0, 0,
            FALSE, TRUE, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL);

//   TxtboxSetData (G_bannerStatBoxes[0]," ");
//   TxtboxSetData (G_bannerStatBoxes[1]," ");
//   TxtboxSetData (G_bannerStatBoxes[2]," ");
//   TxtboxSetData (G_bannerStatBoxes[3]," ");
//   TxtboxSetData (G_bannerStatBoxes[4]," ");

    /* update initial values */
    BannerStatusBarUpdate();

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerStatusBarUpdate
 *-------------------------------------------------------------------------*/
/**
 *  Checks to see if stats have changed and updates the mini-status bar
 *  accordingly
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerStatusBarUpdate(T_void)
{
    T_word16 Health, HealthMax;
    T_word16 Mana, ManaMax;
    T_word16 Food;
    T_word16 Water;
    float load;
    T_byte8 stmp[10];

    DebugRoutine("BannerStatusBarUpdate");

    if (G_bannerStatsCreated == TRUE) {
        Health = StatsGetPlayerHealth();
        HealthMax = StatsGetPlayerMaxHealth();
        sprintf(stmp, "%d/%d", ((Health + 99) / 100), ((HealthMax + 99) / 100));
        TxtboxSetData(G_bannerStatBoxes[0], stmp);

        Mana = StatsGetPlayerMana();
        ManaMax = StatsGetPlayerMaxMana();
        sprintf(stmp, "%d/%d", ((Mana + 99) / 100), ((ManaMax + 99) / 100));
        TxtboxSetData(G_bannerStatBoxes[1], stmp);

        Food = StatsGetPlayerFood();
        sprintf(stmp, "%d%%", ((Food + 19) / 20));
        TxtboxSetData(G_bannerStatBoxes[2], stmp);

        Water = StatsGetPlayerWater();
        sprintf(stmp, "%d%%", ((Water + 19) / 20));
        TxtboxSetData(G_bannerStatBoxes[3], stmp);

        load = (float)(StatsGetPlayerLoad() / 10.0);
        sprintf(stmp, "%3.1f KG", load);
        TxtboxSetData(G_bannerStatBoxes[4], stmp);
    }

//   GraphicUpdateAllGraphics();

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  BannerStatusBarFinish
 *-------------------------------------------------------------------------*/
/**
 *  Removes text boxes created for the mini status display
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerStatusBarFinish(T_void)
{
    T_word16 i;

    DebugRoutine("BannerStatusBarFinish");

    G_bannerStatsCreated = FALSE;

    for (i = 0; i < 5; i++)
        TxtboxDelete(G_bannerStatBoxes[i]);

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  BannerUpdateManaDisplay
 *-------------------------------------------------------------------------*/
/**
 *  This routine updates the mana available display
 *
 *<!-----------------------------------------------------------------------*/
T_void BannerUpdateManaDisplay(T_void)
{
    T_sword16 Manaleft;
    T_sword16 i, j;
    T_screen tempscreen;

    const T_word16 dispx1 = 177;
    const T_word16 dispx2 = 257;
    const T_word16 dispy1 = 156;
    const T_word16 dispy2 = 178;
    const T_word16 deltax = dispx2 - dispx1;
    const T_word16 deltay = dispy2 - dispy1;
    const T_word16 plotstep = deltax / 16;
    static T_byte8 plots[4][21];
    static E_Boolean firstin = TRUE;
    T_sword16 r, g, b;

    static float vel = 0;
    static float level = 9.0;

    T_byte8 color, color2;
    T_word16 cnt;
    static float pct = 0.0;
    float rpct;

    DebugRoutine("BannerUpdateManaDisplay");

    Manaleft = (StatsGetPlayerMana() + 1) / 1000;
    if (Manaleft < 1)
        Manaleft = 1;

    if (firstin == TRUE) //initialize
    {
        firstin = FALSE;
        for (i = 0; i < 4; i++)
            for (j = 0; j < 21; j++)
                plots[i][j] = deltay >> 1;
        ColorSetColor(MANA_BACKCOLOR, 10, 10, 10);
    }

    /* allocate hidden screen */

    tempscreen = GrScreenAllocPartial(deltay + 3);
    GrScreenSet(tempscreen);

    GrDrawRectangle(dispx1, 0, dispx2, deltay, MANA_BACKCOLOR);

    r = ColorGetRed(MANA_BACKCOLOR);
    g = ColorGetGreen(MANA_BACKCOLOR);
    b = ColorGetBlue(MANA_BACKCOLOR);

    /* update MANA_BACKCOLOR */
    if (r > 20 || g > 0 || b > 0) {
        r -= 2;
        b -= 2;
        g -= 2;
        if (r < 20)
            r = 20;
        if (b < 0)
            b = 0;
        if (g < 0)
            g = 0;
        ColorSetColor(MANA_BACKCOLOR, (T_byte8)r, (T_byte8)g, (T_byte8)b);
    }

    /* update plots */

    if (level > 0)
        vel -= 1.5; /* update 'sin' wave */
    else if (level < 0)
        vel += 1.5;
    level += vel;

    rpct = (float)StatsGetPlayerMana() / (float)StatsGetPlayerMaxMana();
    if (pct < rpct)
        pct += (float)0.05;
    else if (pct > rpct)
        pct -= (float)0.05;

    /* update plots */
    plots[0][0] = ((deltay >> 1) + (T_word16)((float)level * pct));

    if (plots[0][0] > deltay)
        plots[0][0] = (T_byte8)deltay;

    for (i = 20; i > 0; i--) {
        plots[3][i] = plots[2][i];
        plots[2][i] = plots[1][i];
        plots[1][i] = plots[0][i];
        plots[0][i] = plots[0][i - 1];
    }

    /* draw plots */
    cnt = dispx1;
    color = 44 - (T_sword32)((float)6.0 * pct);
    if (pct > 0.90)
        color2 = 32;
    else
        color2 = color - 3;

    for (i = 1; i < 20; i++) {
        GrDrawLine(cnt, plots[3][i], cnt + plotstep, plots[3][i + 1], color);
        GrDrawLine(cnt, plots[2][i], cnt + plotstep, plots[2][i + 1],
                color - 1);
        GrDrawLine(cnt, plots[1][i], cnt + plotstep, plots[1][i + 1],
                color - 2);
        GrDrawLine(cnt, plots[0][i], cnt + plotstep, plots[0][i + 1], color2);
        cnt += plotstep;
    }

#ifdef shit
    /* draw plots */
    for (i=3;i>0;i--) for (j=0;j<10;j++) plots[i][j]=plots[i-1][j];

    for (j=0;j<15;j++)
    {
        rnum=rand();
        if (rnum%15>Manaleft) plots[0][j]+=(rnum%Manaleft+1);
        else plots[0][j]-=rnum%Manaleft;
        if (plots[0][j]>deltay) plots[0][j]=deltay;
    }

    for (i=3;i>=0;i--)
    {
        if (i==3) color=MANA_BACKCOLOR;
        else color=167+i+(Manaleft>>1);
        for (j=1;j<14;j++)
        {
            plotx1=dispx1+((j-1)*6);
            plotx2=plotx1+9;
            ploty1=dispy1+plots[i][j-1];
            ploty2=dispy1+plots[i][j];
            GrDrawLine (plotx1,ploty1,plotx2,ploty2,color);
        }
    }
#endif

    /* put graphic on screen */
    MouseHide();
    GrTransferRectangle(GRAPHICS_ACTUAL_SCREEN, dispx1, 0, dispx2, deltay,
            dispx1, dispy1);
    MouseShow();
    GrScreenSet(GRAPHICS_ACTUAL_SCREEN);
    GrScreenFree(tempscreen);

    DebugEnd();
}



E_Boolean BannerIsOpen(T_void)
{
    E_Boolean retvalue = FALSE;

    DebugRoutine("BannerIsOpen");

    retvalue = G_bannerIsOpen;

    DebugEnd();

    return (retvalue);
}

E_Boolean BannerFormIsOpen(E_bannerFormType formtype)
{
    E_Boolean retvalue = FALSE;

    DebugRoutine("BannerFormIsOpen");

    if (BannerIsOpen() && G_currentBannerFormType == formtype)
        retvalue = TRUE;

    DebugEnd();

    return (retvalue);
}



T_void BannerDisplayFinancesPage(T_void)
{
    T_TxtboxID TxtboxID;
    T_buttonID buttonID;
    T_byte8 stmp[16];
    DebugRoutine("BannerDisplayFinancesPage");

    /* get Txtbox ID's for finances form */
    /* and fill them out */

    if (BannerFormIsOpen(BANNER_FORM_FINANCES)) {
        TxtboxID = FormGetObjID(500);
        sprintf(stmp, "%d", StatsGetPlayerCoins(COIN_TYPE_PLATINUM));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(501);
        sprintf(stmp, "%d", StatsGetPlayerCoins(COIN_TYPE_GOLD));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(502);
        sprintf(stmp, "%d", StatsGetPlayerCoins(COIN_TYPE_SILVER));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(503);
        sprintf(stmp, "%d", StatsGetPlayerCoins(COIN_TYPE_COPPER));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(504);
        sprintf(stmp, "%d", StatsGetPlayerSavedCoins(COIN_TYPE_PLATINUM));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(505);
        sprintf(stmp, "%d", StatsGetPlayerSavedCoins(COIN_TYPE_GOLD));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(506);
        sprintf(stmp, "%d", StatsGetPlayerSavedCoins(COIN_TYPE_SILVER));
        TxtboxSetData(TxtboxID, stmp);

        TxtboxID = FormGetObjID(507);
        sprintf(stmp, "%d", StatsGetPlayerSavedCoins(COIN_TYPE_COPPER));
        TxtboxSetData(TxtboxID, stmp);

        /* set callbacks for buttons */
        buttonID = FormGetObjID(301);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_PLATINUM);
        buttonID = FormGetObjID(302);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_GOLD);
        buttonID = FormGetObjID(303);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_SILVER);
        buttonID = FormGetObjID(304);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_COPPER);
        buttonID = FormGetObjID(305);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_FIVE + COIN_TYPE_PLATINUM);
        buttonID = FormGetObjID(306);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_FIVE + COIN_TYPE_GOLD);
        buttonID = FormGetObjID(307);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_FIVE + COIN_TYPE_SILVER);
        buttonID = FormGetObjID(308);
        ButtonSetCallbacks(buttonID, NULL, BannerGetCoin);
        ButtonSetData(buttonID, COIN_TYPE_FIVE + COIN_TYPE_COPPER);
    }

    DebugEnd();
}




/* routine takes care of updating/displaying the Ammo page */
/* note that the form BAN00007.FRM must be loaded and active first */
T_void BannerDisplayAmmoPage(T_void)
{
    T_word16 i;
    T_byte8 slotcount = 0;
    T_byte8 stmp[64];
    const T_byte8 dy = 19;
    T_TxtboxID TxtboxID;
    T_buttonID buttonID;
    T_word16 lastBoltType = 0;

    DebugRoutine("BannerDisplayAmmoPage");
    /* iterate through all types of ammo, check to see if available */
    for (i = 0; i < BOLT_TYPE_UNKNOWN; i++) {
        if (StatsGetPlayerBolts(i)>0)
        {
            /* Record the last bolt position on the list. */
            lastBoltType = i;

            /* set the text for the type field */
            switch (i)
            {
                case BOLT_TYPE_NORMAL:
                strcpy (stmp,"Normal bolt");
                break;
                case BOLT_TYPE_POISON:
                strcpy (stmp,"Poison bolt");
                break;
                case BOLT_TYPE_PIERCING:
                strcpy (stmp,"Piercing bolt");
                break;
                case BOLT_TYPE_FIRE:
                strcpy (stmp,"Fire bolt");
                break;
                case BOLT_TYPE_ELECTRICITY:
                strcpy (stmp,"Charged bolt");
                break;
                case BOLT_TYPE_ACID:
                strcpy (stmp,"Acid bolt");
                break;
                case BOLT_TYPE_MANA_DRAIN:
                strcpy (stmp,"Mana drain bolt");
                break;
                default:
                DebugCheck (0);
                break;
            }

            TxtboxID=FormGetObjID(500+slotcount);
            TxtboxSetData (TxtboxID,stmp);

            /* set the field data for the number of ammo field */
            sprintf (stmp,"%d",StatsGetPlayerBolts(i));
            TxtboxID=FormGetObjID(507+slotcount);
            TxtboxSetData (TxtboxID,stmp);

            /* push selected button down, otherwise up */
            buttonID=FormGetObjID(301+slotcount);
            ButtonSetData (buttonID,slotcount);
            ButtonSetSubData (buttonID,i);

            if (G_ammoSelected==slotcount)
            {
                ButtonDownNoAction (buttonID);
                G_ammoTypeSelected=i;
            }
            else
            {
                ButtonUpNoAction (buttonID);
            }

            if (ButtonIsEnabled(buttonID)==FALSE)
            ButtonEnable(buttonID);

            buttonID=FormGetObjID(308+slotcount);
            ButtonSetData (buttonID,slotcount);
            ButtonSetSubData (buttonID,i);
            if (ButtonIsEnabled(buttonID)==FALSE)
            ButtonEnable (buttonID);

//          ButtonEnable (buttonID);
            /* increment slotcount */
            slotcount++;
        }
    }

    G_numAmmoSlots = slotcount;

    /* 'erase' the empty slots */
    for (i = slotcount; i < NUM_AMMO_SLOTS; i++) {
        TxtboxID = FormGetObjID(500 + i);
        TxtboxSetData(TxtboxID, "");

        TxtboxID = FormGetObjID(507 + i);
        TxtboxSetData(TxtboxID, "");

        buttonID = FormGetObjID(301 + i);
        if (ButtonIsEnabled(buttonID))
            ButtonDisable(buttonID);

        buttonID = FormGetObjID(308 + i);
        if (ButtonIsEnabled(buttonID))
            ButtonDisable(buttonID);
//        ButtonSetCallbacks (buttonID,NULL,NULL);
        if (G_ammoSelected == i) {
            /* move selected button */
            if (ButtonIsEnabled(buttonID) != TRUE) {
                ButtonEnable(buttonID);
            }
            ButtonUpNoAction(buttonID);
            GraphicUpdateAllGraphics();
            ButtonDisable(buttonID);

            /* Choose the selection over to the new one. */
            if (slotcount > 0) {
                G_ammoSelected = slotcount - 1;
                G_ammoTypeSelected = lastBoltType;
            } else {
                G_ammoSelected = 0;
                G_ammoTypeSelected = BOLT_TYPE_NORMAL;
            }
            buttonID = FormGetObjID(301 + G_ammoSelected);
            if (ButtonIsEnabled(buttonID) != TRUE)
                ButtonEnable(buttonID);
            ButtonDownNoAction(buttonID);
        }
    }

    DebugEnd();
}


static T_void BannerSelectAmmo(T_buttonID buttonID)
{
    DebugRoutine("BannerSelectAmmo");
    G_ammoSelected = ButtonGetData(buttonID);
    DebugCheck(G_ammoSelected <= G_numAmmoSlots);
    BannerDisplayAmmoPage();
    DebugEnd();
}

/* routine that returns the current bolt selection type */
E_equipBoltTypes BannerGetSelectedAmmoType(T_void)
{
    E_equipBoltTypes retvalue = BOLT_TYPE_UNKNOWN;
    T_word16 i;
    DebugRoutine("BannerGetSelectedAmmoType");

    if (StatsGetPlayerBolts(G_ammoTypeSelected)>0)
    {
        retvalue=G_ammoTypeSelected;
    }
    else
    {
        /* try to find next slot */
        for (i=BOLT_TYPE_NORMAL;i<BOLT_TYPE_UNKNOWN;i++)
        {
            if (StatsGetPlayerBolts(i)>0)
            {
                /* use these */
                G_ammoTypeSelected=i;
                retvalue=i;
                if (BannerFormIsOpen(BANNER_FORM_AMMO))
                {
                    BannerDisplayAmmoPage();
                }
                break;
            }
        }
    }
    DebugEnd();
    return (retvalue);
}


static T_void BannerGetAmmo(T_buttonID buttonID)
{
//  T_byte8 stmp[32];
    T_byte8 whichbolt;
    T_3dObject *object;
    T_word16 objnum;
    T_word16 value;
    T_byte8 stmp[48];
    T_inventoryItemStruct *p_inv;
    DebugRoutine("BannerGetAmmo");

    whichbolt = ButtonGetSubData(buttonID);
    DebugCheck(whichbolt < BOLT_TYPE_UNKNOWN);

    if (StatsGetPlayerBolts(whichbolt)>0 && InventoryObjectIsInMouseHand()==FALSE)
    {
        if (ClientIsPaused()) {
            MessageAdd("Cannot get ammo while paused.");
        } else if (ClientIsDead()) {
            MessageAdd("Dead people cannot get ammo.");
        } else {
            if (StatsGetPlayerBolts(whichbolt)>11)
            {
                /* remove a quiver */
                /* get obj id for bolt */
                switch (whichbolt)
                {
                    case BOLT_TYPE_NORMAL:
                    objnum=144;
                    break;
                    case BOLT_TYPE_POISON:
                    objnum=146;
                    break;
                    case BOLT_TYPE_PIERCING:
                    objnum=147;
                    break;
                    case BOLT_TYPE_FIRE:
                    objnum=145;
                    break;
                    case BOLT_TYPE_ELECTRICITY:
                    objnum=149;
                    break;
                    case BOLT_TYPE_MANA_DRAIN:
                    objnum=150;
                    break;
                    case BOLT_TYPE_ACID:
                    objnum=148;
                    break;
                    default:
                    DebugCheck(0);
                    break;
                }
                StatsAddBolt(whichbolt,-12);
            }
            else
            /* remove a bolt */
            {
                /* get obj id for bolt */
                switch (whichbolt)
                {
                    case BOLT_TYPE_NORMAL:
                    objnum=206;
                    break;
                    case BOLT_TYPE_POISON:
                    objnum=222;
                    break;
                    case BOLT_TYPE_PIERCING:
                    objnum=229;
                    break;
                    case BOLT_TYPE_FIRE:
                    objnum=218;
                    break;
                    case BOLT_TYPE_ELECTRICITY:
                    objnum=241;
                    break;
                    case BOLT_TYPE_MANA_DRAIN:
                    objnum=242;
                    break;
                    case BOLT_TYPE_ACID:
                    objnum=240;
                    break;
                    default:
                    DebugCheck(0);
                    break;
                }
                StatsAddBolt(whichbolt,-1);
            }

            /* make the object */
            object=ObjectCreateFake ();
            ObjectSetType (object,objnum);
            ObjectSetAngle (object,0x4000);
            p_inv=InventoryTakeObject (INVENTORY_PLAYER,object);
            InventorySetMouseHandPointer(p_inv->elementID);
            ControlSetObjectPointer (p_inv->object);

            if (StoreIsOpen()==TRUE &&
            StoreHouseModeIsOn()==FALSE)
            {
                value=StoreGetBuyValue(p_inv);
                /* find out if store will buy this type of item */
                if (StoreWillBuy(p_inv->itemdesc.type) &&
                value != 0)
                {
                    StoreConvertCurrencyToString(stmp,value);
                    MessagePrintf("^011I'll buy that for %s^011",stmp);
                }
                else
                {
                    MessageAdd ("^004I don't want to buy that.");
                }
            }

        }
    }

    BannerDisplayAmmoPage();

//  sprintf (stmp,"data=%d, subdata=%d\n bl=%d",ButtonGetData(buttonID),ButtonGetSubData(buttonID),StatsGetPlayerBolts(ButtonGetSubData(buttonID));
//  MessageAdd (stmp);
    DebugEnd();
}

/* routine that retrieves a coin from the finance banner display */
static T_void BannerGetCoin(T_buttonID buttonID)
{
    T_word16 type;
    T_word16 objtype;
    T_3dObject *object;
    T_inventoryItemStruct *p_inv;

    E_Boolean fivepiece = FALSE;
    DebugRoutine("BannerGetCoin");

    if (ClientIsPaused()) {
        MessageAdd("Cannot get coin while paused.");
    }
    if (ClientIsDead()) {
        MessageAdd("The dead have no need of coins.");
    } else {
        /* get type of goin to retrieve */
        type = ButtonGetData(buttonID);

        /* get fivepiece flag */
        if (type >= COIN_TYPE_FIVE) {
            fivepiece = TRUE;
            type -= COIN_TYPE_FIVE;
        }

        /* see if the mouse hand is empty */
        if (!InventoryObjectIsInMouseHand()) {
            /* ok, retrieve the proper coin and put it in the mouse hand */
            switch (type) {
                case COIN_TYPE_COPPER:
                    objtype = 100;
                    break;

                case COIN_TYPE_SILVER:
                    objtype = 102;
                    break;

                case COIN_TYPE_GOLD:
                    objtype = 104;
                    break;

                case COIN_TYPE_PLATINUM:
                    objtype = 106;
                    break;

                default:
                    /* fail */
                    DebugCheck(0==1);
                    break;
            }

            /* subtract the number of coins from stats */
            if (StatsAddCoin(type, -1 - (fivepiece * 4)) == TRUE) {
                /* create a coin object */
                if (fivepiece == TRUE)
                    objtype++;
                object = ObjectCreateFake();
                ObjectSetType(object, objtype);

                /* temporarily subtract weight of this coin for balance purposes */
                //            StatsSetPlayerLoad (StatsGetPlayerLoad()-ObjectGetWeight(object));
                /* add object to inventory mouse hand */
                p_inv = InventoryTakeObject(INVENTORY_PLAYER, object);
                InventorySetMouseHandPointer(p_inv->elementID);
                ControlSetObjectPointer(p_inv->object);
            }
            /* all done */
        }
    }

    DebugEnd();
}


/* this routine adds a rune button for selection of a spell */
/* Spellsystem dictates the picture locked, slot indicates where */
T_void BannerAddSpellButton(T_byte8 slot)
{
    const T_word16 xloc[NUMBER_RUNE_BUTTONS] = {
            265,
            282,
            299,
            265,
            282,
            299,
            265,
            282,
            299 };
    const T_word16 yloc[NUMBER_RUNE_BUTTONS] = {
            183,
            183,
            183,
            169,
            169,
            169,
            155,
            155,
            155 };
    E_spellsSpellSystemType spellSystem;

    T_byte8 keycode, picno;
    T_byte8 stmp[32];
    DebugRoutine("BannerAddSpellButton");

    spellSystem = StatsGetPlayerSpellSystem();

//    DebugCheck (G_bannerButtonsCreated==TRUE);

    if (G_bannerButtonsCreated == TRUE) {
        switch (spellSystem) {
            case SPELL_SYSTEM_MAGE:
                picno = slot;
                break;

            case SPELL_SYSTEM_CLERIC:
                picno = slot + 9;
                break;

            case SPELL_SYSTEM_ARCANE:
                if (slot < 4)
                    picno = slot;
                else
                    picno = slot + 5;
                break;

            default:
                printf("Bad class type in bannerAddSpellButton");
                DebugCheck(-1);
                /* fail! */
        }

        if (slot == 0)
            keycode = KeyMap(KEYMAP_RUNE1);
            else if (slot==1) keycode=KeyMap(KEYMAP_RUNE2);
            else if (slot==2) keycode=KeyMap(KEYMAP_RUNE3);
            else if (slot==3) keycode=KeyMap(KEYMAP_RUNE4);
            else if (slot==4) keycode=KeyMap(KEYMAP_RUNE5);
            else if (slot==5) keycode=KeyMap(KEYMAP_RUNE6);
            else if (slot==6) keycode=KeyMap(KEYMAP_RUNE7);
            else if (slot==7) keycode=KeyMap(KEYMAP_RUNE8);
            else if (slot==8) keycode=KeyMap(KEYMAP_RUNE9);

            /* add button for slot */
        sprintf(stmp, "UI/3DUI/RUNBUT%02d", picno);

        DebugCheck(G_runeButtons[slot]==NULL);
        G_runeButtons[slot] = ButtonCreate(xloc[slot], yloc[slot], stmp, FALSE,
                keycode, SpellsAddRune, NULL);
        DebugCheck(G_runeButtons[slot]!=NULL);
        sprintf(stmp, "UI/3DUI/RUDBUT%02d", picno);
        ButtonSetSelectPic(G_runeButtons[slot], stmp);

//        printf ("added rune button slot=%d\n",slot);
//        fflush (stdout);
    }

    DebugEnd();

}

/* this routine removes a rune button for spell selection */
T_void BannerRemoveSpellButton(T_byte8 slot)
{
    T_graphicID keyback;
    const T_word16 xloc[NUMBER_RUNE_BUTTONS] = {
            265,
            282,
            299,
            265,
            282,
            299,
            265,
            282,
            299 };
    const T_word16 yloc[NUMBER_RUNE_BUTTONS] = {
            183,
            183,
            183,
            169,
            169,
            169,
            155,
            155,
            155 };

    DebugRoutine("BannerRemoveSpellButton");
//  DebugCheck (G_runeButtons[slot]!=NULL);
    if (G_bannerButtonsCreated == TRUE) {
//      DebugCheck (G_runeButtons[slot]!=NULL);
        if (G_runeButtons[slot] != NULL) {
            ButtonDelete(G_runeButtons[slot]);
            G_runeButtons[slot] = NULL;

            /* fix background of deleted button */
            keyback = GraphicCreate(xloc[slot], yloc[slot], "UI/3DUI/KEYBACK");
            GraphicUpdateAllGraphics();
            GraphicDelete(keyback);
//            printf ("Deleting rune slot %d\n",slot);
//            fflush (stdout);
        }
    }
    DebugEnd();
}


/* routine disables some banner functionality for */
/* UI screens */
T_void BannerUIModeOn(T_void)
{
    T_graphicID graphic;

    DebugRoutine("BannerUIModeOn");
    G_bannerUIMode = TRUE;

    if (BannerIsOpen()) {
        G_bannerLastForm = G_currentBannerFormType;
    } else {
        G_bannerLastForm = BANNER_FORM_UNKNOWN;
        /* open 'blank' banner scren */
        graphic = GraphicCreate(209, 0, "UI/3DUI/CLOSEDBA");
        GraphicUpdateAllGraphics();
        GraphicDelete(graphic);
    }

    /* disable some buttons */
//    ButtonDisable (G_bannerButtons[BANNER_BUTTON_COM]);
//    ButtonDisable (G_bannerButtons[BANNER_BUTTON_FIN]);
    /* reset control button callbacks */
//    for (i=0;i<9;i++)
//    {
//        ButtonSetCallbacks (G_bannerButtons[i],BannerOpenFormByButton,BannerOpenFormByButton);
//    }
    DebugEnd();
}

/* routine restores disabled banner functionality */
T_void BannerUIModeOff(T_void)
{
    DebugRoutine("BannerUIModeOff");

    G_bannerUIMode = FALSE;
//    ButtonEnable (G_bannerButtons[BANNER_BUTTON_COM]);
//    ButtonEnable (G_bannerButtons[BANNER_BUTTON_FIN]);

    /* reset control button callbacks */
//    for (i=0;i<9;i++)
//    {
//        ButtonSetCallbacks (G_bannerButtons[i],BannerOpenFormByButton,BannerCloseFormByButton);
//    }
    /* restore old banner form */
//    if (G_bannerLastForm!=BANNER_FORM_UNKNOWN)
//    {
//        if (BannerIsOpen()) BannerOpenForm(G_bannerLastForm);
//    }
//    else if (BannerIsOpen()) BannerCloseForm();
    GraphicUpdateAllGraphics();

    DebugEnd();
}



T_void BannerInitSoundOptions(T_void)
{
    T_iniFile ini;
    T_byte8 *p_value;

    DebugRoutine("BannerInitSoundOptions");

    /* load music and sound volume options */
    ini = ConfigGetINIFile();
    p_value = INIFileGet(ini, "options", "musicVolume");
    if (p_value)
        G_musicVol = atoi(p_value);
    else
        G_musicVol = 65000;
    p_value = INIFileGet(ini, "options", "sfxVolume");
    if (p_value)
        G_sfxVol = atoi(p_value);
    else
        G_sfxVol = 65000;
    p_value = INIFileGet(ini, "options", "musicOn");
    if (p_value)
        G_musicOn = atoi(p_value);
    else
        G_musicOn = TRUE;
    p_value = INIFileGet(ini, "options", "sfxOn");
    if (p_value)
        G_sfxOn = atoi(p_value);
    else
        G_sfxOn = TRUE;

#if 0
    fp=fopen ("opts.dat","rb");

    if (fp != NULL)
    {
        fscanf (fp,"%d %d %d %d",&G_musicVol, &G_sfxVol, &G_musicOn, &G_sfxOn);
    }

    /* close file */
    fclose (fp);
#endif

    /* set initial sfx and music volumes */
    if (G_musicOn == TRUE) {
        SoundSetBackgroundVolume(G_musicVol >> 8);
    } else {
        SoundSetBackgroundVolume(0);
    }

    if (G_sfxOn == TRUE) {
        SoundSetEffectsVolume(G_sfxVol >> 8);
    } else {
        SoundSetEffectsVolume(0);
    }

    DebugEnd();
}


E_Boolean BannerButtonsOk(T_void)
{
    return (G_bannerButtonsCreated);
}

E_Boolean BannerFinancesWindowIsAt(T_word16 x, T_word16 y)
{
    E_Boolean isAt = FALSE;
    DebugRoutine("BannerFinancesWindowIsAt");

    if (BannerFormIsOpen(BANNER_FORM_FINANCES) && (x > 214) && (x < 315)
            && (y > 18) && (y < 147)) {
        isAt = TRUE;
    }

    DebugEnd();
    return (isAt);
}

E_Boolean BannerAmmoWindowIsAt(T_word16 x, T_word16 y)
{
    E_Boolean isAt = FALSE;
    DebugRoutine("BannerAmmoWindowIsAt");

    if (BannerFormIsOpen(BANNER_FORM_AMMO) && (x > 214) && (x < 315) && (y > 18)
            && (y < 147)) {
        isAt = TRUE;
    }

    DebugEnd();
    return (isAt);
}

T_void BannerOpenLast(T_void)
{
    DebugRoutine("BannerOpenLast");
    if (G_bannerLastForm < BANNER_FORM_UNKNOWN)
        BannerOpenForm(G_bannerLastForm);
    DebugEnd();
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  BANNER.C
 *-------------------------------------------------------------------------*/

