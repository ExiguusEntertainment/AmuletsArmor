/*-------------------------------------------------------------------------*
 * File:  GUILDUI.C
 *-------------------------------------------------------------------------*/
/**
 * Used for players to meet each other and join when networking.
 *
 * @addtogroup GUILDUI
 * @brief Guild User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BUTTON.H"
#include "CLI_SEND.H"
#include "CSYNCPCK.H"
#include "GUILDUI.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "PEOPHERE.H"
#include "PICS.H"
#include "PROMPT.H"
#include "STATS.H"
#include "TXTBOX.H"

static T_graphicID G_backgroundPic=NULL;
static T_buttonID G_upButtons[4];
static T_buttonID G_dnButtons[4];
static T_graphicID G_scrollBars[4];
static T_TxtboxID G_displayBoxes[4];
static T_buttonID G_joinButton=NULL;
static T_buttonID G_createButton=NULL;
static T_doubleLinkList G_mapList=DOUBLE_LINK_LIST_BAD;
static T_doubleLinkList G_gameList=DOUBLE_LINK_LIST_BAD;
static T_doubleLinkList G_playerList=DOUBLE_LINK_LIST_BAD;

/* internal routine prototypes */
static T_void GuildUIUpdateGraphics (T_void);
static T_void GuildUIBuildMapList (T_void);
static E_Boolean GuildUIPlayerCanVisitMap (T_word16 which);
static T_void GuildDisplayMapInfo (T_TxtboxID TxtboxID);
static T_void GuildDisplayGameInfo (T_TxtboxID TxtboxID);
static T_void GuildUIUpDisplay (T_buttonID buttonID);
static T_void GuildUIDnDisplay (T_buttonID buttonID);
static T_void GuildUIDrawGameList (T_void);
static T_void GuildUIDrawPlayerList (T_void);
static T_void GuildUICancelCreateGame (T_buttonID buttonID);
static T_void GuildUIBeginGame (T_buttonID buttonID);
static T_void GuildUIJoinGame   (T_buttonID buttonID);
static T_void GuildUICreateGame (T_buttonID buttonID);
static T_void GuildUIRequestPlayerListForOpenGame (T_gameDescriptionStruct *p_game);
static T_void GuildUIRequestGameList(T_void);
static T_void GuildUIReset (T_void);
static T_mapDescriptionStruct* GuildUIGetMapDescription (T_word16 mapIndex);
static T_word16 GuildUIGetSelectedAdventure(T_void) ;

T_void GuildUIStart  (T_word32 formNum)
{
    T_word16 i;

    const T_word16 arrowCoordX[4]=   { 97,  97,  200, 200};
    const T_word16 upArrowCoordY[4]= { 28,  77,  28,  77 };
    const T_word16 dnArrowCoordY[4]= { 67,  117, 67,  117};
    const T_word16 scrollBarY[4]=    { 38,  87,  38,  87 };
    const T_word16 dispWindowX1[4]=  { 6,   6,   109, 109};
    const T_word16 dispWindowY1[4]=  { 28,  77,  28,  77 };
    const T_word16 dispWindowX2[4]=  { 95,  95,  198, 198};
    const T_word16 dispWindowY2[4]=  { 75,  125, 75,  125};

    const E_TxtboxMode dispWindowMode[4]={Txtbox_MODE_SELECTION_BOX,
                                          Txtbox_MODE_VIEW_SCROLL_FORM,
                                          Txtbox_MODE_SELECTION_BOX,
                                          Txtbox_MODE_VIEW_SCROLL_FORM};

    const E_TxtboxJustify dispWindowJustify[4]={Txtbox_JUSTIFY_LEFT,
                                                Txtbox_JUSTIFY_LEFT,
                                                Txtbox_JUSTIFY_CENTER,
                                                Txtbox_JUSTIFY_CENTER};

    DebugRoutine ("GuildUIStart");

    /* load backdrop */
    G_backgroundPic=GraphicCreate (4,3,"UI/GUILD/GLDBACK");

    for (i=GUILD_GAME_LIST;i<=GUILD_MAPS_DESC;i++)
    {
        /* set up display textboxes */
        /* scroll bars and control arrows */
        G_displayBoxes[i]=TxtboxCreate (dispWindowX1[i],
                                        dispWindowY1[i],
                                        dispWindowX2[i],
                                        dispWindowY2[i],
                                        "FontTiny",
                                        0,
                                        0,
                                        FALSE,
                                        dispWindowJustify[i],
                                        dispWindowMode[i],
                                        NULL);
        TxtboxSetData (G_displayBoxes[i]," ");


        G_upButtons[i]=ButtonCreate (arrowCoordX[i],
                                     upArrowCoordY[i],
                                     "UI/COMMON/UPARROW",
                                     FALSE,
                                     0,
                                     NULL,
                                     GuildUIUpDisplay);
        ButtonSetData (G_upButtons[i],i);

        G_dnButtons[i]=ButtonCreate (arrowCoordX[i],
                                     dnArrowCoordY[i],
                                     "UI/COMMON/DNARROW",
                                     FALSE,
                                     0,
                                     NULL,
                                     GuildUIDnDisplay);
        ButtonSetData (G_dnButtons[i],i);

        if (i%2==0)
        {
            G_scrollBars[i]=GraphicCreate (arrowCoordX[i],
                                           scrollBarY[i],
                                           "UI/GUILD/SB1");
        }
        else
        {
            G_scrollBars[i]=GraphicCreate (arrowCoordX[i],
                                           scrollBarY[i],
                                           "UI/GUILD/SB1");
        }

        TxtboxSetScrollBarObjIDs(G_displayBoxes[i],G_upButtons[i],G_dnButtons[i],G_scrollBars[i]);
    }

    /* set up control buttons */
    G_joinButton=ButtonCreate (6,
                               127,
                               "UI/GUILD/JOIN",
                               FALSE,
                               KeyDual(KEY_SCAN_CODE_J,KEY_SCAN_CODE_ALT),
                               NULL,
                               GuildUIJoinGame);

    G_createButton=ButtonCreate (109,
                                 127,
                                 "UI/GUILD/CREATE",
                                 FALSE,
                                 KeyDual(KEY_SCAN_CODE_C,KEY_SCAN_CODE_ALT),
                                 NULL,
                                 GuildUICreateGame);

    /* build map list */
    GuildUIBuildMapList();

    /* initialize game/player listings */
    G_gameList=DoubleLinkListCreate();
    G_playerList=DoubleLinkListCreate();

    /* set up control callbacks */
    TxtboxSetCallback (G_displayBoxes[GUILD_MAPS_LIST],GuildDisplayMapInfo);
    TxtboxSetCallback (G_displayBoxes[GUILD_GAME_LIST],GuildDisplayGameInfo);

    /* notify server to send a list of active games */
    GuildUIRequestGameList();

    /* update graphics */
    GraphicUpdateAllGraphics();

    /* Set up what we are out. */
    ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress()) ;
    PeopleHereSetOurAdventure(0) ;
    PeopleHereSetOurState(PLAYER_ID_STATE_NONE) ;

    /* Ask for people to show themselves. */
    PeopleHereReset() ;

    /* add journal help page if necessary */
    if (StatsPlayerHasNotePage(30)==FALSE &&
        StatsPlayerHasNotePage(0)==TRUE)
    {
        StatsAddPlayerNotePage(30);
    }

    DebugEnd();
}


T_void GuildUIUpdate (T_void)
{
    DebugRoutine ("GuildUIUpdate");

    DebugEnd();
}


T_void GuildUIEnd    (T_void)
{
    T_word16 i;
    T_mapDescriptionStruct *p_map;
    T_doubleLinkListElement element,nextElement;
    DebugRoutine ("GuildUIEnd");

    /* destroy ui objects */
    GraphicDelete (G_backgroundPic);
    G_backgroundPic=NULL;

    for (i=0;i<4;i++)
    {
        ButtonDelete(G_upButtons[i]);
        G_upButtons[i]=NULL;
        ButtonDelete(G_dnButtons[i]);
        G_dnButtons[i]=NULL;
        TxtboxDelete (G_displayBoxes[i]);
        GraphicDelete (G_scrollBars[i]);
        G_scrollBars[i]=NULL;
    }

    ButtonDelete (G_joinButton);
    ButtonDelete (G_createButton);
    G_joinButton=NULL;
    G_createButton=NULL;

    /* free up maps data */
    element=DoubleLinkListGetFirst(G_mapList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);
        p_map=(T_mapDescriptionStruct *)DoubleLinkListElementGetData(element);
        MemFree (p_map->name);
        MemFree (p_map->description);
        MemFree (p_map);
        element=nextElement;
    }
    DoubleLinkListDestroy (G_mapList);
    G_mapList=DOUBLE_LINK_LIST_BAD;

    /* free up game list data */
    DoubleLinkListFreeAndDestroy(&G_gameList) ;
#if 0
    element=DoubleLinkListGetFirst(G_gameList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);
        p_game=(T_gameDescriptionStruct *)DoubleLinkListElementGetData(element);
        MemFree(p_game);
        element=nextElement;
    }
    DoubleLinkListDestroy (G_gameList);
    G_gameList=DOUBLE_LINK_LIST_BAD;
#endif

    /* free up player list data */
//    GuildUIClearPlayers();
    DoubleLinkListFreeAndDestroy (&G_playerList);
//    G_playerList=DOUBLE_LINK_LIST_BAD;

    DebugEnd();
}


static T_void GuildUIUpdateGraphics (T_void)
{
    DebugRoutine ("GuildUIUpdateGraphics");

    GraphicUpdateAllGraphics();

    DebugEnd();
}

/* routine builds a list of maps that the player can create */
static T_void GuildUIBuildMapList (T_void)
{
    T_byte8 stmp[64];
    T_byte8 stmp2[512];
    T_word16 mapIndex,mapKey;
    T_byte8 *dataIn;
    T_resource res;
    T_word32 size,tempSize;
    T_word32 inCnt,outCnt;
    T_byte8 charIn;
    T_word16 pass=0;
    T_word16 listcount=0;
    T_mapDescriptionStruct *p_mapStruct;
    T_byte8 *listdata=NULL,*temps=NULL;

    DebugRoutine ("GuildUIBuildMapList");

    /* initialize map listings */

    G_mapList=DoubleLinkListCreate();
    listcount=0;
    sprintf (stmp,"MAPDESC/DES%05d",listcount++);
    while (PictureExist(stmp))
    {
        /* alloc space for new map description block */
        p_mapStruct = MemAlloc (sizeof(T_mapDescriptionStruct));
        DebugCheck (p_mapStruct != NULL);

        /* lock in data 'text file' */
        dataIn=(T_byte8 *)PictureLockData (stmp,&res);
        size=ResourceGetSize(res);

        /* scan for newline */
        outCnt=inCnt=0;
        pass=0;

        while (inCnt < size)
        {
            /* get a character from input data */
            charIn=dataIn[inCnt++];
            stmp2[outCnt++]=charIn;

            /* check for temporary overflow */
            DebugCheck (outCnt<512);

            if (charIn=='\n')
            {
                /* reached 'end of line' */
                /* parse our temp string */
                stmp2[outCnt]='\0';
                outCnt=0;

                if (pass==0)
                {
                    /* getting map access (journal page needed ) */
                    sscanf (stmp2,"%d",&mapKey);
                    p_mapStruct->mapKey=mapKey;

                }
                else if (pass==1)
                {
                    /* getting map number */
                    sscanf (stmp2,"%d",&mapIndex);
                    p_mapStruct->mapIndex=mapIndex;

                }
                else if (pass==2)
                {
                    /* getting map name */
                    tempSize=strlen(stmp2);
                    /* alloc string */
                    p_mapStruct->name=MemAlloc(tempSize+1);
                    strcpy (p_mapStruct->name,stmp2);
                    p_mapStruct->name[strlen(p_mapStruct->name)-2]='\0';
                }
                else if (pass==3)
                {
                    /* getting map description */
                    tempSize=strlen(stmp2);
                    /* alloc string */
                    p_mapStruct->description=MemAlloc(tempSize+1);
                    strcpy (p_mapStruct->description,stmp2);

                }
                else
                {
                    break;
                }

                pass++;
            }
        }

        PictureUnlockAndUnfind(res);

        /* add our structure to global list */
        DoubleLinkListAddElementAtEnd (G_mapList,p_mapStruct);

        /* build our list of maps on the fly */
        if (GuildUIPlayerCanVisitMap(listcount))
        {
            sprintf (stmp,"^009%s",p_mapStruct->name);
        }
        else
        {
            sprintf (stmp,"^010%s",p_mapStruct->name);
        }

        /* add stmp to list */
        tempSize=strlen(stmp);
        if (listdata!=NULL) tempSize+=strlen(listdata);
        temps=MemAlloc(tempSize+2);
        if (listdata == NULL) sprintf (temps,"%s\r",stmp);
        else sprintf (temps,"%s%s\r",listdata,stmp);
        if (listdata != NULL) MemFree (listdata);
        listdata=temps;
        listdata[strlen(listdata)]='\0';

        /* increment map description file name */
        sprintf (stmp,"MAPDESC/DES%05d",listcount++);
    }

    TxtboxSetData (G_displayBoxes[GUILD_MAPS_LIST],listdata);
    TxtboxCursBot(G_displayBoxes[GUILD_MAPS_LIST]);
    TxtboxBackSpace (G_displayBoxes[GUILD_MAPS_LIST]);
    TxtboxCursTop(G_displayBoxes[GUILD_MAPS_LIST]);
    MemFree(listdata);

    GuildDisplayMapInfo (G_displayBoxes[GUILD_MAPS_LIST]);

    DebugEnd();
}


/* routine returns true if map listing number which is available */
/* to our current player */
static E_Boolean GuildUIPlayerCanVisitMap (T_word16 which)
{
    T_doubleLinkListElement element;
    E_Boolean canGo=FALSE;
    T_word16 count=0;
    T_mapDescriptionStruct *p_mapDesc;

    DebugRoutine ("GuildUIPlayerCanVisitMap");
    element=DoubleLinkListGetFirst (G_mapList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        if (count==which)
        {
            /* check this map */
            p_mapDesc=(T_mapDescriptionStruct *)DoubleLinkListElementGetData(element);
            if (p_mapDesc->mapKey==0) canGo=TRUE;
            else if (StatsPlayerHasNotePage(p_mapDesc->mapKey)) canGo=TRUE;
            break;
        }
        count++;
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
    return (canGo);
}


/* routine called when the map selection list is changed, displays text */
/* information block about an adventure */
static T_void GuildDisplayMapInfo (T_TxtboxID TxtboxID)
{
    T_word16 selection;
    T_mapDescriptionStruct *p_map;
    T_doubleLinkListElement element;

    DebugRoutine ("GuildDisplayMapInfo");

    selection=TxtboxGetSelectionNumber(TxtboxID);
    element=DoubleLinkListGetFirst(G_mapList);

    /* Record that adventure as the selected adventure. */
    StatsSetCurrentQuestNumber(selection);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        if (selection==0)
        {

            /* display this map information */
            p_map=DoubleLinkListElementGetData(element);
            TxtboxSetData (G_displayBoxes[GUILD_MAPS_DESC],p_map->description);
            break;
        }

        selection-=1;
        element=DoubleLinkListElementGetNext(element);
    }


    DebugEnd();
}


/* displays information about an open game */
static T_void GuildDisplayGameInfo (T_TxtboxID TxtboxID)
{
    T_sword16 selection;
    T_doubleLinkListElement element;
    T_gameDescriptionStruct *p_game;
    T_mapDescriptionStruct *p_map;
    T_gameDescriptionStruct blank = { 0, 0 } ;

    DebugRoutine ("GuildDisplayGameInfo");

    selection=TxtboxGetSelectionNumber(TxtboxID);
    element=DoubleLinkListGetFirst (G_gameList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* compare this game entry to the selection number */
        if (selection==0)
        {
            /* this is the one - */
            p_game=(T_gameDescriptionStruct *)DoubleLinkListElementGetData(element);
            /* find map */
            element=DoubleLinkListGetFirst (G_mapList);
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
            {
                p_map=(T_mapDescriptionStruct *)DoubleLinkListElementGetData(element);
                if (p_map->mapIndex==p_game->mapNumber)
                {
                    /* display this one */
                    TxtboxCursSetRow (G_displayBoxes[GUILD_MAPS_LIST],selection);
                    TxtboxSetData (G_displayBoxes[GUILD_MAPS_DESC],p_map->description);
                    break;
                }
                element=DoubleLinkListElementGetNext(element);
                selection++;
            }
            break;
        }
        element=DoubleLinkListElementGetNext(element);
        selection--;
    }

   /* request player list for this game from server here */
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        GuildUIRequestPlayerListForOpenGame(p_game);
    else
        GuildUIRequestPlayerListForOpenGame(&blank) ;

    DebugEnd();
}


/* handles scroll bar button 'up' functions for guildUI*/
static T_void GuildUIUpDisplay (T_buttonID buttonID)
{
    DebugRoutine ("GuildUIUpDisplay");

    TxtboxCursUp(G_displayBoxes[ButtonGetData(buttonID)]);
    if (ButtonGetData(buttonID)==GUILD_MAPS_LIST)  GuildDisplayMapInfo(G_displayBoxes[GUILD_MAPS_LIST]);
    else if (ButtonGetData(buttonID)==GUILD_GAME_LIST)  GuildDisplayGameInfo(G_displayBoxes[GUILD_GAME_LIST]);
    DebugEnd();
}

/* handles scroll bar button 'down' functions for guildUI*/
static T_void GuildUIDnDisplay (T_buttonID buttonID)
{
    DebugRoutine ("GuildUIDnDisplay");

    TxtboxCursDn(G_displayBoxes[ButtonGetData(buttonID)]);
    if (ButtonGetData(buttonID)==GUILD_MAPS_LIST)  GuildDisplayMapInfo(G_displayBoxes[GUILD_MAPS_LIST]);
    else if (ButtonGetData(buttonID)==GUILD_GAME_LIST)  GuildDisplayGameInfo(G_displayBoxes[GUILD_GAME_LIST]);

    DebugEnd();
}


/* adds a game to the list of active 'open' games */
T_void GuildUIAddGame(T_word16 mapNumber, T_gameGroupID groupID, T_word16 questNumber)
{
    T_gameDescriptionStruct *p_game;

    DebugRoutine ("GuildUIAddGame");
    p_game=MemAlloc(sizeof(T_gameDescriptionStruct));
    p_game->mapNumber=mapNumber;
    p_game->groupID = groupID ;
	p_game->questNumber = questNumber;
    DoubleLinkListAddElementAtEnd (G_gameList,p_game);

    /* Only redraw the list if we are not joining or creating a game. */
    if (PeopleHereGetOurState() == 0)
        GuildUIDrawGameList();

    /* If this is the first time entry, go to it. */
    if (DoubleLinkListGetNumberElements(G_gameList) == 1)
        TxtboxCursTop(G_displayBoxes[GUILD_GAME_LIST]) ;

    DebugEnd();
}


/* removes a game from the list of active 'open' games */
T_void GuildUIRemoveGame (T_word16 mapNumber, T_gameGroupID groupID)
{
    T_gameDescriptionStruct *p_game;
    T_doubleLinkListElement element,nextElement;
    DebugRoutine ("GuildUIRemoveGame");

    element=DoubleLinkListGetFirst(G_gameList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);
        p_game=(T_gameDescriptionStruct *)DoubleLinkListElementGetData(element);

        if ((p_game->mapNumber==mapNumber) &&
            (CompareGameGroupIDs(p_game->groupID, groupID)))
        {
            /* remove this one */
            MemFree(p_game);
            DoubleLinkListRemoveElement(element);
            break;
        }

        element=nextElement;
    }

    /* Only redraw the list if we are not joining or creating a game. */
    if (PeopleHereGetOurState() == 0)
        GuildUIDrawGameList();

    DebugEnd();
}


/* redraws the list of open games */
static T_void GuildUIDrawGameList(T_void)
{
    T_doubleLinkListElement element;
    T_gameDescriptionStruct *p_game=NULL;
    T_mapDescriptionStruct *p_map=NULL;
    T_byte8 *listData=NULL;
    T_byte8 *temp=NULL;
    T_byte8 stmp[64];
    T_word16 count=0;
    T_word32 size=0;

    DebugRoutine ("GuildUIDrawGameList");
    element=DoubleLinkListGetFirst (G_gameList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get this element's data */
        p_game=(T_gameDescriptionStruct *)DoubleLinkListElementGetData(element);

        /* find corresponding map information structure */
        p_map=GuildUIGetMapDescription(p_game->mapNumber);

        /* make this data line */
        sprintf (stmp,"%s\r",p_map->name);

        /* add to list */
        if (listData != NULL) size=strlen(listData);
        else size=0;
        size+=strlen(stmp);
        temp=MemAlloc(size+2);
        if (listData != NULL) sprintf (temp,"%s%s",listData,stmp);
        else strcpy (temp,stmp);
        if (listData != NULL) MemFree(listData);

        listData=temp;
        temp=NULL;

        element=DoubleLinkListElementGetNext(element);
    }

    if (listData != NULL)
    {
        /* get rid of last '\r' */
        listData[strlen(listData)-1]='\0';
        TxtboxSetData(G_displayBoxes[GUILD_GAME_LIST],listData);
        MemFree(listData);
    }
    else
    {
        TxtboxSetData(G_displayBoxes[GUILD_GAME_LIST],"");
    }
    DebugEnd();
}


/* draws current player list (G_playerList) */
static T_void GuildUIDrawPlayerList (T_void)
{
    T_doubleLinkListElement element;
    T_byte8* listData=NULL;
    T_byte8* temp=NULL;
    T_byte8* name=NULL;
    T_word16 size=0;

    DebugRoutine ("GuildUIDrawPlayerList");
    element=DoubleLinkListGetFirst (G_playerList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get this element's data */
        name=(T_byte8*)DoubleLinkListElementGetData(element);
        /* find corresponding map information structure */
        /* add to list */
        if (listData != NULL) size=strlen(listData);
        else size=0;
        size+=strlen(name);
        temp=MemAlloc(size+2);
        if (listData != NULL) sprintf (temp,"%s%s\r",listData,name);
        else sprintf (temp,"%s\r",name);

        if (listData != NULL) MemFree(listData);

        listData=temp;
        temp=NULL;

        element=DoubleLinkListElementGetNext(element);
    }

    TxtboxSetData(G_displayBoxes[GUILD_GAME_DESC],listData);
    if (listData != NULL) MemFree(listData);

    DebugEnd();
}


/* adds a player name to the list of players for the selected game */
T_void GuildUIAddPlayer (T_byte8 *playerName)
{
    T_byte8 *data;
    T_word32 size;

    DebugRoutine ("GuildUIAddPlayer");

    //size=TxtboxCanFit(G_displayBoxes[GUILD_GAME_DESC],playerName);
	size = strlen(playerName);

    data=NULL;
    data=MemAlloc(size+1);
    DebugCheck (data != NULL);
    strncpy (data,playerName,size);
    data[size]='\0';
    /* add personName to list */
    DoubleLinkListAddElementAtEnd (G_playerList,data);

    GuildUIDrawPlayerList();

    DebugEnd();
}

/* removes a player name from the list of players for the selected game */
T_void GuildUIRemovePlayer (T_byte8 *playerName)
{
    T_doubleLinkListElement element;
    T_byte8 *name;
    T_word32 size;
    DebugRoutine ("GuildUIRemovePlayer");

    size=TxtboxCanFit(G_displayBoxes[GUILD_GAME_DESC],playerName);
    element=DoubleLinkListGetFirst(G_playerList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        name=(T_byte8 *)DoubleLinkListElementGetData(element);
        if (strncmp(name,playerName,size)==0)
        {
            /* remove element */
            MemFree(name);
            DoubleLinkListRemoveElement(element);
            break;
        }
        element=DoubleLinkListElementGetNext(element);
    }

    GuildUIDrawPlayerList();

    DebugEnd();
}

/* clears all player names from the player list box */
T_void GuildUIClearPlayers (T_void)
{
    T_doubleLinkListElement element,nextElement;
    T_byte8 *p_name;
    T_byte8 *test;
    DebugRoutine ("GuildUIClearPlayers");

    element=DoubleLinkListGetFirst(G_playerList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);
        p_name=(T_byte8 *)DoubleLinkListElementGetData(element);
        MemFree(p_name);
        DoubleLinkListRemoveElement(element);
        element=nextElement;
    }

    test=TxtboxGetData(G_displayBoxes[GUILD_GAME_DESC]);
    if (test != NULL) TxtboxSetData(G_displayBoxes[GUILD_GAME_DESC],"");

    DebugEnd();
}

/* returns a pointer to a map information structure for map mapIndex */
static T_mapDescriptionStruct* GuildUIGetMapDescription (T_word16 mapIndex)
{
    T_mapDescriptionStruct *mapInfo;
    T_doubleLinkListElement element;
    DebugRoutine ("GuildUIGetMapDescription");

    element=DoubleLinkListGetFirst (G_mapList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        mapInfo=(T_mapDescriptionStruct *)DoubleLinkListElementGetData(element);
        if (mapInfo->mapIndex==mapIndex) break;
        else mapInfo=NULL;

        element=DoubleLinkListElementGetNext(element);
    }
    DebugCheck (mapInfo != NULL);

    DebugEnd();
    return (mapInfo);
}

/* routine resets the guild ui display back to normal */
/* after a join or create game command has been issued) */
static T_void GuildUIReset (T_void)
{
    DebugRoutine ("GuildUIReset");


    /* reset control buttons */
/*
    ButtonDelete (G_joinButton);
    if (G_createButton != NULL) ButtonDelete (G_createButton);
    G_joinButton=ButtonCreate (6,
                               127,
                               "UI/GUILD/JOIN",
                               FALSE,
                               0,
                               NULL,
                               GuildUIJoinGame);

    G_createButton=ButtonCreate (109,
                                 127,
                                 "UI/GUILD/CREATE",
                                 FALSE,
                                 0,
                                 NULL,
                                 GuildUICreateGame);

    TxtboxDelete (G_displayBoxes[GUILD_GAME_LIST]);
    ButtonSetCallbacks (G_upButtons[GUILD_GAME_LIST],NULL,GuildUIUpDisplay);
    ButtonSetCallbacks (G_dnButtons[GUILD_GAME_LIST],NULL,GuildUIDnDisplay);
    ButtonSetCallbacks (G_upButtons[GUILD_MAPS_LIST],NULL,GuildUIUpDisplay);
    ButtonSetCallbacks (G_dnButtons[GUILD_MAPS_LIST],NULL,GuildUIDnDisplay);
    TxtboxDelete (G_displayBoxes[GUILD_MAPS_LIST]);
*/

    GuildUIEnd();
    GuildUIStart(0);

    DebugEnd();
}

/*--------------------------------------------*/
/* server communication/control routines here */
/*--------------------------------------------*/

/* routine is called from GuildUIStart - requests a list of active */
/* open games to be sent */
static T_void GuildUIRequestGameList (T_void)
{
    DebugRoutine ("GuildUIRequestGameList");

/*
    GuildUIAddGame (1000,1);
    GuildUIAddGame (1000,2);
    GuildUIAddGame (1003,1);
    GuildUIAddGame (1001,1);
    GuildUIAddGame (1000,3);
*/

    GuildDisplayGameInfo (G_displayBoxes[GUILD_GAME_LIST]);

    DebugEnd();
}

/* routine should notify server that we need a list of players for the */
/* indicated open game */
static T_void GuildUIRequestPlayerListForOpenGame (T_gameDescriptionStruct *p_game)
{
    DebugRoutine ("GuildUIRequestPlayerListForOpenGame");

    /* clear current list */
//    GuildUIClearPlayers();

    /* Fill the list with appropriate people in the game. */
    PeopleHereGeneratePeopleInGame(p_game->groupID) ;

/*
    GuildUIAddPlayer    ("Joe Shmoe");
    GuildUIAddPlayer    ("Jack Sheet");
    GuildUIAddPlayer    ("Wonder Woman");
    GuildUIAddPlayer    ("Super Man");
    GuildUIRemovePlayer ("Wonder Woman");
*/

    DebugEnd();
}

/* button callback routine for when the 'create game' button is pressed */
/* should request a game creation from server */
static T_void GuildUICreateGame (T_buttonID buttonID)
{
    DebugRoutine ("GuildUICreateGame");

    /* fake it */
//    GuildUIConfirmCreateGame();

    /* real it */
    /* Tell others that I'm creating a game. */
    PeopleHereSetOurState(PLAYER_ID_STATE_CREATING_GAME) ;
    PeopleHereSetOurAdventure(GuildUIGetSelectedAdventure()) ;

    ClientSyncSetGameGroupID(PeopleHereGetUniqueGroupID()) ;

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf() ;

    GuildUIConfirmCreateGame();

    DebugEnd();
}

/* button callback routine for when the 'join game' button is pressed */

static T_void GuildUIJoinGame   (T_buttonID buttonID)
{
    T_word16 map ;
    T_gameGroupID groupID ;
	T_word16 quest;

    DebugRoutine ("GuildUIJoinGame");

    /* fake it */
//    GuildUIConfirmJoinGame();

    /* real it */
    GuildUIGetSelectedGame(&map, &groupID, &quest) ;

    if (map != 0)  {
        /* Request to join in the fun. */
        PeopleHereSetOurState(PLAYER_ID_STATE_JOINING_GAME) ;
        ClientSendRequestJoin(
            map,
            groupID) ;
		StatsSetCurrentQuestNumber(quest);
    } else {
        MessageAdd("No game session selected.") ;
    }

    DebugEnd();
}

/* button callback routine for when the 'begin game' button is pressed */
/* only active after 'create game' has been selected */
/* should notify the server that the created game should start with the */
/* current listed members */
static T_void GuildUIBeginGame (T_buttonID buttonID)
{
    T_word16 firstLevel ;
    E_Boolean doSkip ;

    DebugRoutine ("GuildUIBeginGame");

    firstLevel = StatsFindPastPlace(PeopleHereGetOurAdventure()) ;
    if (firstLevel != 0)  {
//        if (BannerIsOpen())
//            BannerCloseForm() ;
        FormPush() ;
        doSkip = PromptForBoolean("Continue adventure from last save point?", TRUE);
        FormPop() ;
//        ButtonRedrawAllButtons() ;
        if (!doSkip)
            firstLevel = 0 ;
    }

    /* The wonder command */
    PeopleHereStartGame(firstLevel) ;

    DebugEnd();
}

/* button callback routine for when the 'cancel' button is pressed */
/* only active after 'create game' has been selected */
/* should notify the server that the created game has been canceled */
static T_void GuildUICancelCreateGame (T_buttonID buttonID)
{
    DebugRoutine ("GuildUICancelCreateGame");

    /* Go back to nowhere. */
    /* Tell others that I'm no longer creating a game. */
    PeopleHereSetOurState(PLAYER_ID_STATE_NONE) ;

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf() ;

    PeopleHereSetOurAdventure(0) ;
    ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress()) ;

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf() ;

    /* reset display */
    GuildUIReset();

    DebugEnd();
}

/* button callback routine for when the 'cancel' button is pressed */
/* after a 'join game' command has been issued */
T_void GuildUICancelJoinGame (T_buttonID buttonID)
{
    DebugRoutine ("GuildUICancelJoinGame");

    ButtonDelete (G_joinButton);

    /* Go back to nowhere. */
    /* Tell others that I'm no longer creating a game. */
    PeopleHereSetOurState(PLAYER_ID_STATE_NONE) ;

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf() ;

    PeopleHereSetOurAdventure(0) ;
    ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress()) ;

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf() ;

    /* reset display */
    GuildUIReset();

    DebugEnd();
}

/* server callback, should be notified after a 'create game' packet has */
/* been recieved, puts player in state of monitering player list for created */
/* game, and either issuing a 'begin' or 'cancel' command for the created game */
T_void GuildUIConfirmCreateGame (T_void)
{
    DebugRoutine ("GuildUIConfirmCreateGame");

    /* switch control buttons to 'begin game' and 'cancel game' */
    ButtonDelete (G_joinButton);
    ButtonDelete (G_createButton);
    G_joinButton=ButtonCreate (6,
                               127,
                               "UI/GUILD/BEGIN",
                               FALSE,
                               KeyDual(KEY_SCAN_CODE_B,KEY_SCAN_CODE_ALT),
                               NULL,
                               GuildUIBeginGame);

    G_createButton=ButtonCreate (109,
                                 127,
                                 "UI/GUILD/CANCEL",
                                 FALSE,
                                 KeyDual(KEY_SCAN_CODE_C,KEY_SCAN_CODE_ALT),
                                 NULL,
                                 GuildUICancelCreateGame);


    /* delete adventure list box and replace with information */
    /* concerning status */
    TxtboxDelete (G_displayBoxes[GUILD_GAME_LIST]);
    TxtboxDelete (G_displayBoxes[GUILD_MAPS_LIST]);
    ButtonSetCallbacks (G_upButtons[GUILD_GAME_LIST],NULL,NULL);
    ButtonSetCallbacks (G_dnButtons[GUILD_GAME_LIST],NULL,NULL);
    ButtonSetCallbacks (G_upButtons[GUILD_MAPS_LIST],NULL,NULL);
    ButtonSetCallbacks (G_dnButtons[GUILD_MAPS_LIST],NULL,NULL);

    G_displayBoxes[GUILD_GAME_LIST]=TxtboxCreate (6,
                                                  28,
                                                  95,
                                                  75,
                                                  "FontTiny",
                                                  0,
                                                  0,
                                                  FALSE,
                                                  Txtbox_JUSTIFY_CENTER,
                                                  Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                                  NULL);

    TxtboxSetData (G_displayBoxes[GUILD_GAME_LIST],
                   "^003Creating game:^007\r\rplease wait for\rother players\r\r^005Select begin button\rTo start game.");


    DebugEnd();
}


/* server callback, should be notified after a 'join game' packet has been */
/* recieved, puts player in a wait state, waiting on the open game creator */
/* to select 'begin game'.  Player has the option to cancel the join */
T_void GuildUIConfirmJoinGame (T_void)
{
    DebugRoutine ("GuildUIConfirmJoinGame");

    /* switch control buttons to 'cancel' and 'null' */
    ButtonDelete (G_joinButton);
    ButtonDownNoAction (G_createButton);
    GraphicUpdateAllGraphics ();
    ButtonDelete (G_createButton);
    G_createButton=NULL;

    G_joinButton=ButtonCreate (6,
                               127,
                               "UI/GUILD/CANCEL",
                               FALSE,
                               KeyDual(KEY_SCAN_CODE_C,KEY_SCAN_CODE_ALT),
                               NULL,
                               GuildUICancelJoinGame);


    /* delete adventure list box and replace with information */
    /* concerning status */
    TxtboxDelete (G_displayBoxes[GUILD_GAME_LIST]);
    TxtboxDelete (G_displayBoxes[GUILD_MAPS_LIST]);
    ButtonSetCallbacks (G_upButtons[GUILD_GAME_LIST],NULL,NULL);
    ButtonSetCallbacks (G_dnButtons[GUILD_GAME_LIST],NULL,NULL);
    ButtonSetCallbacks (G_upButtons[GUILD_MAPS_LIST],NULL,NULL);
    ButtonSetCallbacks (G_dnButtons[GUILD_MAPS_LIST],NULL,NULL);

    G_displayBoxes[GUILD_GAME_LIST]=TxtboxCreate (6,
                                                  28,
                                                  95,
                                                  75,
                                                  "FontTiny",
                                                  0,
                                                  0,
                                                  FALSE,
                                                  Txtbox_JUSTIFY_CENTER,
                                                  Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                                  NULL);

    TxtboxSetData (G_displayBoxes[GUILD_GAME_LIST],
                   "^003Joining game:^007\r\rplease wait for\rother players\r\r^005Select cancel button\rTo cancel action.");

    DebugEnd();
}


static T_word16 GuildUIGetSelectedAdventure(T_void)
{
    T_word16 adventure ;
    T_word16 i, index ;
    T_mapDescriptionStruct *p_map ;
    T_doubleLinkListElement element ;

    DebugRoutine("GuildUIGetSelectedAdventure") ;

    /* Just the appropriate text line on the list. */
    index = TxtboxGetSelectionNumber(G_displayBoxes[GUILD_MAPS_LIST]) ;
    StatsSetCurrentQuestNumber(index);

    element = DoubleLinkListGetFirst(G_mapList) ;
    for (i=0; i<index; i++)  {
        element = DoubleLinkListElementGetNext(element) ;
    }
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_map = (T_mapDescriptionStruct *)
                    DoubleLinkListElementGetData(element) ;
        adventure = p_map->mapIndex ;
    } else {
        /* Element not found? */
        DebugCheck(FALSE) ;
        adventure = 0 ;
    }

    DebugEnd() ;

    return adventure ;
}

T_void GuildUIGetSelectedGame(
                  T_word16 *p_mapNumber,
                  T_gameGroupID *p_groupID,
				  T_word16 *p_quest)
{
    T_word16 i, index ;
    T_gameDescriptionStruct *p_game ;
    T_doubleLinkListElement element ;

    DebugRoutine("GuildUIGetSelectedGame") ;

    /* Just the appropriate text line on the list. */
    index = TxtboxGetSelectionNumber(G_displayBoxes[GUILD_GAME_LIST]) ;

    element = DoubleLinkListGetFirst(G_gameList) ;
    for (i=0; i<index; i++)  {
        element = DoubleLinkListElementGetNext(element) ;
    }
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_game = (T_gameDescriptionStruct *)
                    DoubleLinkListElementGetData(element) ;
        *p_mapNumber = p_game->mapNumber ;
        *p_groupID = p_game->groupID ;
		*p_quest = p_game->questNumber;
    } else {
        /* Element not found?  Return zero. */
        *p_mapNumber = 0 ;
        *p_groupID = *DirectTalkGetNullBlankUniqueAddress() ;
		*p_quest = 0 ;
    }

    DebugEnd() ;
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  GUILDUI.C
 *-------------------------------------------------------------------------*/
