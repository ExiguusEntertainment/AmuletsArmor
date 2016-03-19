/*-------------------------------------------------------------------------*
 * File:  TOWNUI.C
 *-------------------------------------------------------------------------*/
/**
 * Top level User Interface for being in town.
 *
 * @addtogroup TOWNUI
 * @brief Town User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BANNER.H"
#include "BUTTON.H"
#include "CLIENT.H"
#include "CLI_SEND.H"
#include "CSYNCPCK.H"
#include "DBLLINK.H"
#include "DITALK.H"
#include "FILE.H"
#include "GRAPHIC.H"
#include "HARDFORM.H"
#include "INIFILE.H"
#include "INVENTOR.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PEOPHERE.H"
#include "PROMPT.H"
#include "SOUND.H"
#include "STATS.H"
#include "TICKER.H"
#include "TOWNUI.H"
#include "TXTBOX.H"

#define TOWN_NUM_MESSAGES 15
#define TOWN_MESSAGE_SIZE 60
#define MAX_MAPS_PER_ADVENTURE 15
static T_word16 G_numQuestsAvailable = 0;
static T_word16 G_firstAdventureMap = 0;
static T_graphicID G_backgroundPic = NULL;
static T_buttonID G_menuButtons[4];
static T_buttonID G_userUpButton = NULL;
static T_buttonID G_userDnButton = NULL;
static T_buttonID G_userUpButton2 = NULL;
static T_buttonID G_userDnButton2 = NULL;
static T_buttonID G_userNextButton = NULL;
static T_buttonID G_userLastButton = NULL;
static T_graphicID G_userScrollBar = NULL;
static T_graphicID G_userScrollBar2 = NULL;
static T_TxtboxID G_userListBox = NULL;
static T_TxtboxID G_chatBox = NULL;
static T_TxtboxID G_listenBox = NULL;
static T_TxtboxID G_titleBox = NULL;
static T_TxtboxID G_goalBox = NULL;
static T_TxtboxID G_goldBox = NULL;
static T_TxtboxID G_expBox = NULL;
static T_TxtboxID G_descriptionBox = NULL;
static E_Boolean G_townIsOpen = FALSE;
static T_byte8* G_messages[TOWN_NUM_MESSAGES];
static T_byte8 G_messageLine;
static T_doubleLinkList G_chatList = DOUBLE_LINK_LIST_BAD;
static T_keyboardEventHandler G_oldKeyHandler = NULL;
static E_Boolean G_isOnePlayer;
static E_Boolean G_adventureComplete = FALSE;

/* internal routine prototypes */
static T_void TownUIUpdateGraphics(T_void);
static T_void TownUIGotoPlace(T_buttonID buttonID);
static T_void TownRedrawChatList(T_void);
static T_void TownUISendChatMessage(T_TxtboxID TxtboxID);
static T_void TownUIUpdateQuestInfo(T_void);
static T_void TownUINextQuest(T_buttonID buttonID);
static T_void TownUILastQuest(T_buttonID buttonID);
static T_void TownUIUserBoxUp(T_buttonID buttonID);
static T_void TownUIUserBoxDn(T_buttonID buttonID);
static T_void TownUIDescriptionBoxUp(T_buttonID buttonID);
static T_void TownUIDescriptionBoxDn(T_buttonID buttonID);

//static T_void TownUIHandleKey(E_keyboardEvent event, T_byte8 scankey);

T_void TownUIStart(T_word32 formNum)
{
    T_word16 i;
    T_byte8 stmp[32];
    E_Boolean iSucceeded;

    const T_word16 menuX[4] = { 6, 57, 107, 157 };
    const T_byte8 menuN[4][6] = { "QUEST", "INN", "STORE", "BANK" };
    const T_byte8 menuN2[4][6] = { "GUILD", "INN", "STORE", "BANK" };
    T_word16 menuHotKey[4];

    const T_word16 placeToGo[4] = {
            HARD_FORM_GUILD,
            HARD_FORM_INN,
            HARD_FORM_STORE,
            HARD_FORM_BANK };
    const T_word16 placeToGo2[4] = {
            0xFFFF,
            HARD_FORM_INN,
            HARD_FORM_STORE,
            HARD_FORM_BANK };

    DebugRoutine("TownUIStart");

    if (InventoryObjectIsInMouseHand() == TRUE) {
        InventoryAutoStoreItemInMouseHand();
    }

    if (DirectTalkGetServiceType() == DIRECT_TALK_SELF_SERVER)
        G_isOnePlayer = TRUE;
    else
        G_isOnePlayer = FALSE;

    /* load backdrop */
    if (G_isOnePlayer == TRUE) {
        G_backgroundPic = GraphicCreate(4, 3, "UI/TOWN/TWNBACK2");
    } else {
        G_backgroundPic = GraphicCreate(4, 3, "UI/TOWN/TWNBACK1");
    }

    /* set up menu button hot keys */
    if (G_isOnePlayer == TRUE)
        menuHotKey[0] = KeyDual(KEY_SCAN_CODE_Q,KEY_SCAN_CODE_ALT);
    else
        menuHotKey[0] = KeyDual(KEY_SCAN_CODE_G,KEY_SCAN_CODE_ALT);
    menuHotKey[1] = KeyDual(KEY_SCAN_CODE_I,KEY_SCAN_CODE_ALT);
    menuHotKey[2] = KeyDual(KEY_SCAN_CODE_S,KEY_SCAN_CODE_ALT);
    menuHotKey[3] = KeyDual(KEY_SCAN_CODE_B,KEY_SCAN_CODE_ALT);

    for (i = 0; i < 4 /* 5 */; i++) {
        /* load menu buttons */
        if (G_isOnePlayer)
            sprintf(stmp, "UI/TOWN/%s", menuN[i]);
        else
            sprintf(stmp, "UI/TOWN/%s", menuN2[i]);
        G_menuButtons[i] = ButtonCreate(menuX[i], 17, stmp, FALSE,
                menuHotKey[i], NULL, TownUIGotoPlace);

        if (G_isOnePlayer == TRUE) {
            ButtonSetData(G_menuButtons[i], placeToGo2[i]);
        } else {
            ButtonSetData(G_menuButtons[i], placeToGo[i]);
        }
    }

    if (G_isOnePlayer == FALSE) {


        /* set up display textboxes for multiplayer chat room */
        G_userListBox = TxtboxCreate(6, 35, 61, 137, "FontTiny", 0, 0, FALSE,
                Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_SCROLL_FORM, NULL );
        TxtboxSetData(G_userListBox, "");

        G_chatBox = TxtboxCreate(75, 35, 206, 42, "FontTiny", 40, 0, FALSE,
                Txtbox_JUSTIFY_LEFT, Txtbox_MODE_FIXED_WIDTH_FIELD,
                TownUISendChatMessage);
        TxtboxSetData(G_chatBox, "");

        G_listenBox = TxtboxCreate(75, 44, 206, 137, "FontTiny", 0, 0, FALSE,
                Txtbox_JUSTIFY_LEFT, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );
        TxtboxSetData(G_listenBox, "");

        /* set up scroll buttons */
        G_userUpButton = ButtonCreate(63, 35, "UI/COMMON/UPARROW", FALSE, 0,
                NULL, TownUIUserBoxUp);

        G_userDnButton = ButtonCreate(63, 129, "UI/COMMON/DNARROW", FALSE, 0,
                NULL, TownUIUserBoxDn);

        /* set up scroll bar */
        G_userScrollBar = GraphicCreate(63, 45, "UI/TOWN/ULSB");
        TxtboxSetScrollBarObjIDs(G_userListBox, G_userUpButton, G_userDnButton,
                G_userScrollBar);

        /* initialize list of people in chat area */
        G_chatList = DoubleLinkListCreate();

        /* allocate memory for chat messages */
        for (i = 0; i < TOWN_NUM_MESSAGES; i++) {
            G_messages[i] = MemAlloc(TOWN_MESSAGE_SIZE);
            strcpy(G_messages[i], "");
        }
        G_messageLine = 0;

        TxtboxFirstBox();
        TxtboxNextBox();
        KeyboardBufferOn();
    } else {
        /* single player mode, set up adventure choosing screen */
        G_userLastButton = ButtonCreate(41, 35, "UI/TOWN/LAST", FALSE, 0, NULL,
                TownUILastQuest);
        G_userNextButton = ButtonCreate(188, 35, "UI/TOWN/NEXT", FALSE, 0, NULL,
                TownUINextQuest);
        G_titleBox = TxtboxCreate(61, 35, 186, 44, "FontMedium", 0, 0, FALSE,
                Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );

        G_goalBox = TxtboxCreate(61, 46, 206, 53, "FontTiny", 0, 0, FALSE,
                Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );

        G_goldBox = TxtboxCreate(82, 55, 114, 62, "FontTiny", 0, 0, FALSE,
                Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );

        G_expBox = TxtboxCreate(156, 55, 206, 62, "FontTiny", 0, 0, FALSE,
                Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );

        G_descriptionBox = TxtboxCreate(6, 64, 198, 137, "FontMedium", 0, 0,
                FALSE, Txtbox_JUSTIFY_LEFT, Txtbox_MODE_VIEW_SCROLL_FORM,
                NULL );

        /* set up scroll buttons */
        G_userUpButton2 = ButtonCreate(200, 64, "UI/COMMON/UPARROW", FALSE, 0,
                NULL, TownUIDescriptionBoxUp);

        G_userDnButton2 = ButtonCreate(200, 129, "UI/COMMON/DNARROW", FALSE, 0,
                NULL, TownUIDescriptionBoxDn);
        /* set up scroll bar */
        G_userScrollBar2 = GraphicCreate(200, 74, "UI/TOWN/SB2");

        /* set up scroll bar control */
        TxtboxSetScrollBarObjIDs(G_descriptionBox, G_userUpButton2,
                G_userDnButton2, G_userScrollBar2);

        /* scan drive and count number of quests available */
        if (G_numQuestsAvailable == 0) {
            do {
                sprintf(stmp, "MAPDESC\\QUEST%d.ini", G_numQuestsAvailable++);
            } while (FileExist(stmp));
            G_numQuestsAvailable--;
            DebugCheck(G_numQuestsAvailable != 0);
        }

        TownUIUpdateQuestInfo();

    }

    G_townIsOpen = TRUE;
    MessageSetAlternateOutputOn();

//    G_oldKeyHandler=KeyboardGetEventHandler();
//    KeyboardSetEventHandler (TownUIHandleKey);

    GraphicUpdateAllGraphics();

    /* Set up what we are out. */
	//Code to quit current game.. need to defer until all clients confirm quest is over
    /*ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress());
    PeopleHereSetOurAdventure(0);
	PeopleHereSetOurState(PLAYER_ID_STATE_NONE);*/

    /* Ask for people to show themselves. 
		Only run here for single player or abort
		In multiplayer, players are not in town until they report quest complete*/
	if (G_isOnePlayer == TRUE || G_adventureComplete == FALSE)
		PeopleHereReset();

    /* check for adventure complete */
    if (G_adventureComplete == TRUE) 
	{
		iSucceeded = TownUIFinishedQuest(LEVEL_STATUS_STARTED, (T_byte8)PeopleHereGetNumInGroupGame(), StatsGetCurrentQuestNumber());//for multiplayer, assume we haven't won
        GraphicUpdateAllGraphicsForced();

        if (G_isOnePlayer)
            TownUIUpdateQuestInfo();		
    }

    BannerUIModeOn();
    GraphicUpdateAllGraphics();
    /*
     #ifdef COMPILE_OPTION_SHAREWARE_DEMO
     TownUIAddMessage(NULL, "^009Welcome to the Amulets and Armor shareware demo.") ;
     TownUIAddMessage(NULL, "") ;

     if (DirectTalkGetServiceType() != DIRECT_TALK_SELF_SERVER)  {
     TownUIAddMessage(NULL, "^010*^007 Select ^001GUILD^007 to begin") ;
     TownUIAddMessage(NULL, "   a multiplayer game.") ;
     } else  {
     ButtonDisable(G_menuButtons[0]) ;
     }

     TownUIAddMessage(NULL, "^010*^007 Select ^001INN^007 to leave") ;
     TownUIAddMessage(NULL, "   the game and rest.") ;
     TownUIAddMessage(NULL, "^010*^007 Select ^001STORE^007 to buy") ;
     TownUIAddMessage(NULL, "   and sell items.") ;
     TownUIAddMessage(NULL, "^010*^007 Select ^001BANK^007 to save") ;
     TownUIAddMessage(NULL, "   your earnings.") ;
     TownUIAddMessage(NULL, "") ;
     TownUIAddMessage(NULL, "^010*^007 Press ^026ESC^007 to start the") ;
     TownUIAddMessage(NULL, "   shareware demo adventure.") ;
     TownUIAddMessage(NULL, "") ;
     #endif
     */

    // Let everyone know we are here once every so much time
    // If this is not sent enough, we'll be dropped from the list
    ClientSendPlayerIDSelf();

    DebugEnd();
}

T_void TownUIUpdate(T_void)
{
    DebugRoutine("TownUIUpdate");

    DebugEnd();
}

T_void TownUIEnd(T_void)
{
    T_word16 i;
    T_doubleLinkListElement element;

    DebugRoutine("TownUIEnd");

    BannerUIModeOff();

    /* destroy ui objects */
    GraphicDelete(G_backgroundPic);
    G_backgroundPic = NULL;

    for (i = 0; i < 4 /* 5 */; i++) {
        ButtonDelete(G_menuButtons[i]);
        G_menuButtons[i] = NULL;
    }

    // Save the character!
    StatsSaveCharacter(StatsGetActive());

    if (G_isOnePlayer == FALSE) {
        /* clean up chat area */

        ButtonDelete(G_userUpButton);
        ButtonDelete(G_userDnButton);
        G_userUpButton = NULL;
        G_userDnButton = NULL;
        G_userScrollBar = NULL;
        GraphicDelete(G_userScrollBar);
        TxtboxDelete(G_userListBox);
        TxtboxDelete(G_chatBox);
        TxtboxDelete(G_listenBox);

        G_userListBox = NULL;
        G_chatBox = NULL;
        G_listenBox = NULL;

        /* destroy chat list */
        element = DoubleLinkListGetFirst(G_chatList);
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD ) {
            /* remove data string */
            MemFree(DoubleLinkListElementGetData(element));
            DoubleLinkListRemoveElement(element);
            element = DoubleLinkListGetFirst(G_chatList);
        }

        DoubleLinkListDestroy(G_chatList);
        G_chatList = DOUBLE_LINK_LIST_BAD;

        /* free message memory */
        for (i = 0; i < TOWN_NUM_MESSAGES; i++)
            MemFree(G_messages[i]);
        KeyboardBufferOff();
    } else {
        /* clean up quest selection display */
        TxtboxDelete(G_titleBox);
        TxtboxDelete(G_goalBox);
        TxtboxDelete(G_goldBox);
        TxtboxDelete(G_expBox);
        TxtboxDelete(G_descriptionBox);
        ButtonDelete(G_userNextButton);
        ButtonDelete(G_userLastButton);
        ButtonDelete(G_userUpButton2);
        ButtonDelete(G_userDnButton2);
        GraphicDelete(G_userScrollBar2);

        G_titleBox = NULL;
        G_goldBox = NULL;
        G_expBox = NULL;
        G_descriptionBox = NULL;
        G_userNextButton = NULL;
        G_userLastButton = NULL;
        G_userUpButton2 = NULL;
        G_userDnButton2 = NULL;
        G_userScrollBar2 = NULL;
    }

    G_townIsOpen = FALSE;
//    KeyboardSetEventHandler(G_oldKeyHandler);
    MessageSetAlternateOutputOff();
    DebugEnd();
}

static T_void TownUIUpdateGraphics(T_void)
{
    DebugRoutine("TownUIUpdateGraphics");

    GraphicUpdateAllGraphics();

    DebugEnd();
}

/* controls town button presses to go to other 'places' */
static T_void TownUIGotoPlace(T_buttonID buttonID)
{
    T_word16 mapNum = 30;
    T_iniFile idata;
    T_word16 currentQuest = 0;
    T_byte8 stmp[8192];
    E_Boolean accept = TRUE;

    DebugRoutine("TownUIGotoPlace");
    DebugCheck(buttonID != NULL);

    if (ButtonGetData(buttonID) == 0xFFFF) {
        /* We hit the start button.  Let's continue on */
        DebugCheck(G_firstAdventureMap!=0);

        mapNum = StatsFindPastPlace(G_firstAdventureMap);
        if (mapNum == 0) {
            mapNum = G_firstAdventureMap;
        }

        if (mapNum == G_firstAdventureMap) {
            /* show dialogue information */
            currentQuest = StatsGetCurrentQuestNumber();
            sprintf(stmp, "MAPDESC\\QUEST%d.INI", currentQuest);
            idata = INIFileOpen(stmp);

            /* read in data and set fields */
            INIFileGetString(idata, "main", "dialogue", stmp, 8192);
            FormPush();
            accept = PromptDisplayDialogue(stmp);
            FormPop();
            ButtonRedrawAllButtons();
        }

        ClientSetAdventureNumber(G_firstAdventureMap);

        if (mapNum) {
            if (accept) {
                ClientSetNextPlace(mapNum, 0);
            } else {
                /* Make sure it is know that no adventure was selected. */
                ClientSetAdventureNumber(0);
            }
        }

    } else {
        ClientSetNextPlace(HARDFORM_GOTO_PLACE_OFFSET + ButtonGetData(buttonID),
                0);
    }

    DebugEnd();
}

E_Boolean TownUIIsOpen(T_void)
{
    return (G_townIsOpen);
}


/* Checks if person is in chat area */
E_Boolean TownPersonInChat(T_byte8 *personName)
{
	T_doubleLinkListElement element, nextElement;
	T_byte8 *data;
	T_word32 tocmp;
	E_Boolean retval = FALSE;

	DebugRoutine("TownPersonInChat");

	if (G_isOnePlayer == FALSE) {
		DebugCheck(G_chatList != DOUBLE_LINK_LIST_BAD);
		/* search list for this string */
		element = DoubleLinkListGetFirst(G_chatList);
		while (element != DOUBLE_LINK_LIST_ELEMENT_BAD) {
			nextElement = DoubleLinkListElementGetNext(element);
			data = DoubleLinkListElementGetData(element);
			tocmp = TxtboxCanFit(G_userListBox, data);

			if (strncmp(data, personName, tocmp) == 0) {
				retval = TRUE;
				break;
			}

			element = nextElement;
		}
	}

	DebugEnd();

	return retval;
}

/* routine should be called by server - adds personName to list of people */
/* in town hall chat area */
T_void TownAddPerson(T_byte8 *personName)
{
    T_byte8 *data;
    T_word32 size;
    T_word32 tocpy;

    DebugRoutine("TownAddPerson");

    if (G_isOnePlayer == FALSE) {
        DebugCheck(G_chatList != DOUBLE_LINK_LIST_BAD);

		//Only add player if not already in list
		if (TownPersonInChat(personName) == FALSE)
		{
			/* alloc memory for string */
			size = strlen(personName);

			data = NULL;
			data = MemAlloc(size + 1);
			DebugCheck(data != NULL);
			//tocpy = TxtboxCanFit(G_userListBox, personName);
			tocpy = strlen(personName);
			strncpy(data, personName, tocpy);
			data[tocpy] = '\0';
			/* add personName to list */
			DoubleLinkListAddElementAtEnd(G_chatList, data);

			/* redraw list */
			TownRedrawChatList();
		}
    }

    DebugEnd();
}

/* routine should be called by server - removes personName from list of */
/* people in town hall chat area */
T_void TownRemovePerson(T_byte8 *personName)
{
    T_doubleLinkListElement element, nextElement;
    T_byte8 *data;
    T_word32 tocmp;

    DebugRoutine("TownRemovePerson");

    if (G_isOnePlayer == FALSE) {
        DebugCheck(G_chatList != DOUBLE_LINK_LIST_BAD);
        /* search list for this string */
        element = DoubleLinkListGetFirst(G_chatList);
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD ) {
            nextElement = DoubleLinkListElementGetNext(element);
            data = DoubleLinkListElementGetData(element);
            tocmp = TxtboxCanFit(G_userListBox, data);

            if (strncmp(data, personName, tocmp) == 0) {
                /* found it, chunk it */
                MemFree(data);
                DoubleLinkListRemoveElement(element);
                break;
            }

            element = nextElement;
        }

        /* redraw list */
        TownRedrawChatList();
    }

    DebugEnd();
}

static T_void TownRedrawChatList(T_void)
{
    T_doubleLinkListElement element;
    T_byte8 *data = NULL;
    T_byte8 *list = NULL;
    T_byte8 *temp = NULL;
    T_byte8 newline[] = "\r";

    T_word32 size1, size2;
    DebugRoutine("TownRedrawChatList");
    DebugCheck(G_isOnePlayer==FALSE);

    /* parse list into long string for textbox */
    element = DoubleLinkListGetFirst(G_chatList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD ) {
        data = DoubleLinkListElementGetData(element);
        size1 = strlen(data);
        if (list != NULL ) {
            size2 = strlen(list);
        } else {
            size2 = 0;
        }
        temp = MemAlloc(size1 + size2 + 2);
        strcpy(temp, data);
        strcat(temp, newline);
        if (list != NULL ) {
            strcat(temp, list);
            MemFree(list);
        }
        list = temp;
        element = DoubleLinkListElementGetNext(element);
    }

    TxtboxSetData(G_userListBox, list);

    DebugEnd();
}

/* routine handles 'chat' control */
T_void TownUIAddMessage(T_byte8 *playerName, T_byte8 *message)
{
    T_byte8 *text;
    T_word16 i;
    T_word16 size = 0;
    T_word16 offset = 2;
    DebugRoutine("TownUIAddMessage");
    DebugCheck(strlen(message) < TOWN_MESSAGE_SIZE);
    DebugCheck(G_isOnePlayer==FALSE);

    if (playerName == NULL )
        offset = 1;

    if (G_messageLine < TOWN_NUM_MESSAGES - offset) {
        /* add message here */
        if (playerName != NULL ) {
            sprintf(G_messages[G_messageLine], "^003%s^007:\r", playerName);
            G_messageLine++;
        }

        sprintf(G_messages[G_messageLine++], "%s\r", message);
    } else {
        /* scroll up messages by 2 */
        for (i = 0; i < TOWN_NUM_MESSAGES - offset; i++) {
            strcpy(G_messages[i], G_messages[i + offset]);
        }

        /* add message here */
        if (playerName != NULL )
            sprintf(G_messages[TOWN_NUM_MESSAGES - 2], "^003%s^007:\r",
                    playerName);
        sprintf(G_messages[TOWN_NUM_MESSAGES - 1], "%s\r", message);
    }

    /* construct message block */
    for (i = 0; i <= G_messageLine; i++) {
        size += strlen(G_messages[i]) + 2;
    }

    text = MemAlloc(size);
    strcpy(text, "");

    for (i = 0; i <= G_messageLine; i++) {
        strcat(text, G_messages[i]);
    }

    TxtboxSetData(G_listenBox, text);
    TxtboxCursBot(G_listenBox);
    MemFree(text);

    DebugEnd();
}

static T_void TownUISendChatMessage(T_TxtboxID TxtboxID)
{
    E_TxtboxAction action;
    DebugRoutine("TownUISendChatMessage");
    DebugCheck(G_isOnePlayer==FALSE);

    action = TxtboxGetAction();
    if (action == Txtbox_ACTION_ACCEPTED) {
        /* send message to server */
        /* fake for now */
//        TownUIAddMessage(StatsGetName(),TxtboxGetData(G_chatBox));
        ClientSendTownUIAddMessage(TxtboxGetData(G_chatBox));

        /* clear chat area */
        TxtboxSetData(G_chatBox, "");
    }

    DebugEnd();
}

/* updates the town hall view in single player mode */
static T_void TownUIUpdateQuestInfo(T_void)
{
    T_word16 currentQuest;
    T_graphicID objBackPic = NULL;
    T_word16 objType;
    T_bitmap *p_bitmap;
    T_3dObject *p_obj;
    T_word16 xdraw, ydraw;
    T_word16 nummaps;
    T_word16 mapnumber;
    T_word16 mapNum;
    T_word16 i;
    T_word16 lastMapCompleted;
    T_iniFile idata;

    T_byte8 stmp[4096];
    T_byte8 stmp2[512];
    T_byte8 stmp3[16000];
    DebugRoutine("TownUIUpdateQuestInfo");

    /* get the current quest number */
    currentQuest = StatsGetCurrentQuestNumber();
    sprintf(stmp, "MAPDESC\\QUEST%d.INI", currentQuest);
    idata = INIFileOpen(stmp);

    /* read in data and set fields */
    INIFileGetString(idata, "main", "title", stmp, 4096);
    TxtboxSetData(G_titleBox, stmp);

    INIFileGetString(idata, "main", "goal", stmp, 4096);
    TxtboxSetData(G_goalBox, stmp);

    INIFileGetString(idata, "main", "gold", stmp, 4096);
    TxtboxSetData(G_goldBox, stmp);

    INIFileGetString(idata, "main", "experience", stmp, 4096);
    TxtboxSetData(G_expBox, stmp);

    INIFileGetString(idata, "main", "firstmap", stmp, 4096);
    G_firstAdventureMap = atoi(stmp);

    /* append ongoing quest information */
    INIFileGetString(idata, "main", "description", stmp, 4096);
    strcpy(stmp3, stmp);

    INIFileGetString(idata, "main", "nummaps", stmp, 4096);
    nummaps = atoi(stmp);
    sprintf(stmp2, "^007     Number of maps :\t^009%d\r", nummaps);
    strcat(stmp3, stmp2);
    mapNum = StatsFindPastPlace(G_firstAdventureMap);

    if (mapNum == 0) {
        mapNum = G_firstAdventureMap;
    }

    if (mapNum == G_firstAdventureMap) {
        strcat(stmp3, "^007     Quest is currently :\t^009Available\r");
    } else {
        strcat(stmp3, "^007     Quest is currently :\t^005In progress\r");
        lastMapCompleted = 0;
        for (i = 1; i <= nummaps; i++) {
            sprintf(stmp2, "map%d", i);
            strcpy(stmp, "");
            INIFileGetString(idata, stmp2, "mapnumber", stmp, 4096);
            mapnumber = atoi(stmp);
            if (mapnumber == mapNum) {
                lastMapCompleted = i - 1;
            }
        }

        sprintf(stmp, "^007     Maps Completed:\t^009%d of %d^007\r",
                lastMapCompleted, nummaps);
        strcat(stmp3, stmp);
    }

    INIFileGetString(idata, "main", "description2", stmp, 4096);
    strcat(stmp3, "\r");
    strcat(stmp3, stmp);

    if (mapNum != G_firstAdventureMap) {
        /* show 'completed maps' data */
        for (i = lastMapCompleted; i >= 1; i--) {
            sprintf(stmp, "^011Notes for map %d:\r", i);
            strcat(stmp3, stmp);
            sprintf(stmp2, "map%d", i);
            strcpy(stmp, "");
            INIFileGetString(idata, stmp2, "description", stmp, 4096);
            strcat(stmp3, stmp);
            strcat(stmp3, "^011\r\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r^007");
        }
    }

    TxtboxSetData(G_descriptionBox, stmp3);

    /* draw the quest object picture background*/
    objBackPic = GraphicCreate(6, 35, "UI/TOWN/BLOCK");
    GraphicUpdateAllGraphics();
    GraphicDelete(objBackPic);

    /* draw the quest object */
    INIFileGetString(idata, "main", "questobj", stmp, 4096);
    objType = atoi(stmp);
    p_obj = ObjectCreateFake();
    ObjectSetType(p_obj, objType);
    p_bitmap = (T_bitmap *)ObjectGetBitmap(p_obj);
    xdraw = 22 - (ObjectGetPictureWidth(p_obj) / 2);
    ydraw = 48 - (ObjectGetPictureHeight(p_obj) / 2);
//    GrDrawShadedAndMaskedBitmap(p_bitmap,xdraw+2,ydraw+2,0);
    GrDrawCompressedBitmap(p_bitmap, xdraw, ydraw);
    ObjectDestroy(p_obj);

    INIFileClose(NULL, idata);
    DebugEnd();
}

/* selects the next available quest */
static T_void TownUINextQuest(T_buttonID buttonID)
{
    T_word16 currentQuest;
    DebugRoutine("TownUINextQuest");

    currentQuest = StatsGetCurrentQuestNumber() + 1;
    if (currentQuest == G_numQuestsAvailable) {
        currentQuest = 0;
    }

    StatsSetCurrentQuestNumber(currentQuest);
    TownUIUpdateQuestInfo();
    DebugEnd();
}

/* selects the last quest */
static T_void TownUILastQuest(T_buttonID buttonID)
{
    T_word16 currentQuest;
    DebugRoutine("TownUILastQuest");

    currentQuest = StatsGetCurrentQuestNumber() - 1;
    if (currentQuest > G_numQuestsAvailable) {
        currentQuest = G_numQuestsAvailable - 1;
    }

    StatsSetCurrentQuestNumber(currentQuest);
    TownUIUpdateQuestInfo();
    DebugEnd();
}

/* notifies townui that the current adventure has been completed */
T_void TownUISetAdventureCompleted(T_void)
{
    DebugRoutine("TownUISetAdventureCompleted");

    G_adventureComplete = TRUE;

    DebugEnd();
}

/* presents reward or failure info for completed quest */
E_Boolean TownUIFinishedQuest(T_word16 multiplayerStatus, T_byte8 numPlayers, T_word16 currentQuest)
{
    E_Boolean iSucceeded = FALSE;
    T_byte8 stmp[8192];
    T_word16 rewardGold = 0;
    T_word32 rewardExp = 0;
    T_word16 rewardItem = 0;
    T_word16 questItem = 0;
	T_word16 sendStatus = 0;
    E_Boolean keepItem = TRUE;
    T_iniFile idata;
    T_byte8 vol;

    DebugRoutine("TownUIFinishedQuest");

    BannerCloseForm();

    G_adventureComplete = FALSE;
    /* determine success or failure for this quest */

    /* open ini file */
    sprintf(stmp, "MAPDESC\\QUEST%d.INI", currentQuest);
    idata = INIFileOpen(stmp);

    /* get quest item */
    INIFileGetString(idata, "main", "questobj", stmp, 8192);
    questItem = atoi(stmp);

    /* see if the item is in the player's inventory */
    if (questItem == 0) {
        iSucceeded = TRUE;
    } else {
		if (InventoryHasItem(INVENTORY_PLAYER, questItem) > 0)
            iSucceeded = TRUE;
    }

	if (iSucceeded == TRUE || multiplayerStatus & LEVEL_STATUS_SUCCESS) {
        /* get reward info */
        INIFileGetString(idata, "main", "gold", stmp, 8192);
        rewardGold = atoi(stmp);
        INIFileGetString(idata, "main", "experience", stmp, 8192);
        rewardExp = atoi(stmp);
        INIFileGetString(idata, "main", "rewarditem", stmp, 8192);
        rewardItem = atoi(stmp);
        INIFileGetString(idata, "main", "keepobj", stmp, 8192);
        if (strcmp(stmp, "FALSE") == 0)
            keepItem = FALSE;
        else
            keepItem = TRUE;
        /* display success info */
        if (G_isOnePlayer || multiplayerStatus & LEVEL_STATUS_SUCCESS) {
            INIFileGetString(idata, "success", "description", stmp, 8192);
            FormPush();
            PromptDisplayBulletin(stmp);
            FormPop();

            FormPush();
            vol = SoundGetBackgroundVolume();
            SoundSetBackgroundVolume((vol > 2) ? 2 : vol);
            sprintf(stmp, "^036Your quest has been completed!");
            SoundPlayByNumber(1500, 255);
            PromptDisplayMessage(stmp);
            FormPop();
            GraphicUpdateAllGraphicsForced();
            SoundSetBackgroundVolume(vol);

            /* remove quest item from player's inventory if necessary */
            if (keepItem == FALSE) {
                INIFileGetString(idata, "main", "keepobjtext", stmp, 8192);
                FormPush();
                PromptDisplayMessage(stmp);
                FormPop();
                InventoryDestroySpecificItem(INVENTORY_PLAYER, questItem, 1);
                GraphicUpdateAllGraphicsForced();
            }

			//Divvy out based on number of players
			if (G_isOnePlayer == FALSE)
			{
				rewardExp = (T_word32)(rewardExp / numPlayers);
				rewardGold = (T_word32)(rewardGold / numPlayers);
			}

            /* give player rewards */
            if (rewardGold != 0) {
                INIFileGetString(idata, "main", "rewardgoldtext", stmp, 8192);
                FormPush();
                SoundPlayByNumber(6018, 255);
                PromptDisplayMessage(stmp);
                FormPop();
                StatsAddCoin(COIN_TYPE_GOLD, rewardGold);
                GraphicUpdateAllGraphicsForced();
            }

            /*        if (rewardItem != 0)
             {
             INIFileGetString(idata,"main","rewarditemtext",stmp,8192);
             FormPush() ;
             SoundPlayByNumber(2014,255);
             PromptDisplayMessage(stmp);
             FormPop() ;
             if (InventoryObjectIsInMouseHand())
             {
             InventoryAutoStoreItemInMouseHand();
             }
             InventoryCreateObjectInHand(rewardItem);
             GraphicUpdateAllGraphicsForced();
             }
             */
            if (rewardExp != 0) {				
                sprintf(stmp, "^009You have gained ^018%d ^009experience!",
                        rewardExp);
                FormPush();
                SoundPlayByNumber(2014, 255);
                PromptDisplayMessage(stmp);
                FormPop();
                StatsChangePlayerExperience(rewardExp);
                GraphicUpdateAllGraphicsForced();
            }

			//Only open up next store if success or multiplayer
			if (iSucceeded || G_isOnePlayer == FALSE) 
			{
				currentQuest = 1 + StatsGetCurrentQuestNumber();
				//modulos down to a normal quest number
				// We just have to assume they are in the same order. 
				//  i.e. Quest 8 = same level as Quest 1
				//       Quest 14 = same level as Quest 7
				while (currentQuest >= ADVENTURE_UNKNOWN)
				{
					currentQuest -= (ADVENTURE_UNKNOWN - 1);//step back by 7 quest numbers
				}
				if (currentQuest != (T_word16)-1)
				{ // TODO: Is this comparison correct?
					if ((currentQuest > StatsGetCompletedAdventure()) &&
						(currentQuest < ADVENTURE_UNKNOWN))
					{
						StatsSetCompletedAdventure((T_byte8)currentQuest);
						MessageAdd("More items are available in the store");
					}
				}
			}
        } else {
            /* Make sure they don't have the quest item. */
            //InventoryDestroySpecificItem(INVENTORY_PLAYER, questItem, 1);
        }
    } else {
		if (G_isOnePlayer || multiplayerStatus & LEVEL_STATUS_COMPLETE) {
            /* display failure info */
            INIFileGetString(idata, "failure", "description", stmp, 8192);
            FormPush();
            PromptDisplayBulletin(stmp);
            FormPop();

            FormPush();
            sprintf(stmp, "^005You have failed your quest.");
            PromptDisplayMessage(stmp);
            FormPop();
        }
    }

	//If multiplayer, sync the results of a completed quest
	if (G_isOnePlayer == FALSE && multiplayerStatus == LEVEL_STATUS_STARTED)
	{
		sendStatus = LEVEL_STATUS_COMPLETE;
		if (iSucceeded)
			sendStatus |= LEVEL_STATUS_SUCCESS;

		/* Get the exact list of people that were in this game. */
		ISetupGame(ClientSyncGetGameGroupID());

		//use "start" packet to signal an end and the result
		ClientSendGameStartPacket(ClientSyncGetGameGroupID(),
			PeopleHereGetOurAdventure(), (T_byte8)PeopleHereGetNumInGroupGame(),
			G_peopleNetworkIDInGame, 0,
			sendStatus);
	}
	else
	{
		//Completed quest has been reported by all players
		//	Reset them and show them in town
		PeopleHereReset();
		TownRedrawChatList();
	}

//  GraphicUpdateAllGraphicsForced();

    DebugEnd();

    return iSucceeded;
}

static T_void TownUIUserBoxUp(T_buttonID buttonID)
{
    TxtboxCursUp(G_userListBox);
}

static T_void TownUIUserBoxDn(T_buttonID buttonID)
{
    TxtboxCursDn(G_userListBox);
}

static T_void TownUIDescriptionBoxUp(T_buttonID buttonID)
{
    TxtboxCursUp(G_descriptionBox);
}

static T_void TownUIDescriptionBoxDn(T_buttonID buttonID)
{
    TxtboxCursDn(G_descriptionBox);
}

E_Boolean TownUICompletedMapLevel(T_word16 mapLevel)
{
    T_iniFile idata;
    T_word16 currentQuest = 0;
    T_byte8 stmp[8192];
    T_byte8 stmp2[9000];
    T_byte8 stmp3[512];
    T_byte8 section[40];
    T_word16 i;
    T_word16 mapNum;
    T_word16 nummaps;
    E_Boolean goToTown = FALSE;
    T_byte8 *p_map;

    /* determine which quest we are doing */
    currentQuest = StatsGetCurrentQuestNumber();
    /* determine success or failure for this quest */

    /* open ini file */
    sprintf(stmp, "MAPDESC\\QUEST%d.INI", currentQuest);
    /* If one player, put up a message explaining your end. */
    if (G_isOnePlayer) {
        idata = INIFileOpen(stmp);

        nummaps = atoi(INIFileGet(idata, "main", "nummaps"));

        stmp[0] = '\0';
        for (i = 0; i < nummaps; i++) {
            sprintf(section, "map%d", i);
            p_map = INIFileGet(idata, section, "mapnumber");
            if (p_map)
                mapNum = atoi(p_map);
            else
                mapNum = 0;
            if (mapNum == mapLevel) {
                INIFileGetString(idata, section, "description", stmp, 8192);
                break;
            }
        }

        INIFileGetString(idata, "main", "title", stmp3, 512);
        sprintf(stmp2,
                "^003Map %d of ^009%s ^003completed!\r^011~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\r^007%s",
                i, stmp3, stmp);

        /* Only display if there truly is text. */
        if (stmp[0] != '\0') {
            FormPush();
            goToTown = !(PromptDisplayContinue(stmp2));
            FormPop();
        }
    }

    return goToTown;
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  TOWNUI.C
 *-------------------------------------------------------------------------*/
