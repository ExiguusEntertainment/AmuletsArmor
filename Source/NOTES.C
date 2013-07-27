/****************************************************************************/
/*    FILE:  NOTES.C                                                        */
/****************************************************************************/
#include "BANNER.H"
#include "BUTTON.H"
#include "CONTROL.H"
#include "FORM.H"
#include "INVENTOR.H"
#include "OBJECT.H"
#include "NOTES.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "TXTBOX.H"

static T_word16 G_currentJournalPage=1;

T_void NotesOpenNotesPage (T_void)
{
    T_buttonID buttonID;
    T_TxtboxID TxtboxID;
    DebugRoutine ("NotesOpenNotesPage");

    buttonID=FormGetObjID(301);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,BannerOpenFormByButton);
    ButtonSetData (buttonID, BANNER_FORM_JOURNAL);

    /* set field data */
    TxtboxID=FormGetObjID(500);
    DebugCheck (TxtboxID != NULL);
    TxtboxSetData (TxtboxID,StatsGetPlayerNotes());

    /* select data entry textbox */
    TxtboxFirstBox();

    /* enable keyboard buffering while open */
    KeyboardBufferOn();

    DebugEnd();
}


T_void NotesCloseNotesPage (T_void)
{
    T_TxtboxID TxtboxID;
    DebugRoutine ("NotesCloseNotesPage");

    /* disable keyboard buffering */
    KeyboardBufferOff();

    TxtboxID=FormGetObjID(500);
    /* save Txtbox data */
    StatsSetPlayerNotes (TxtboxGetData(TxtboxID));

    DebugEnd();
}


T_void NotesOpenJournalPage (T_void)
{
    T_buttonID buttonID;
    DebugRoutine ("NotesOpenJournalPage");

    /* set 'to notes' button functionality */
    buttonID=FormGetObjID(301);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,BannerOpenFormByButton);
    ButtonSetData (buttonID,BANNER_FORM_NOTES);

    /* set rewind button functionality */
    buttonID=FormGetObjID(302);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,NotesChangeJournalPage);
    ButtonSetData (buttonID,-5);
    /* set back note button functionality */
    buttonID=FormGetObjID(303);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,NotesChangeJournalPage);
    ButtonSetData (buttonID,-1);
    /* set fwd note button functionality */
    buttonID=FormGetObjID(304);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,NotesChangeJournalPage);
    ButtonSetData (buttonID,1);
    /* set ffwd button functionality */
    buttonID=FormGetObjID(305);
    DebugCheck (buttonID != NULL);
    ButtonSetCallbacks (buttonID,NULL,NotesChangeJournalPage);
    ButtonSetData (buttonID,5);
    NotesUpdateJournalPage();

    DebugEnd();
}


T_void NotesUpdateJournalPage (T_void)
{
    T_TxtboxID TxtboxID;
    T_graphicID tempGraphic;
    T_word16 notePage;
    T_byte8 stmp[32];

    DebugRoutine ("NotesUpdateJournalPage");

    /* check current journal page bounds */
    if (G_currentJournalPage >= MAX_NOTES ||
        G_currentJournalPage == 0)
    {
        if (StatsGetNumberOfNotes()>0)
        {
            G_currentJournalPage=1;
        }
        else
        {
            G_currentJournalPage=0;
        }
    }

    if (G_currentJournalPage > StatsGetNumberOfNotes())
    {
        G_currentJournalPage = StatsGetNumberOfNotes();
    }

/*    if (StatsGetNumberOfNotes()>1)
    {

        if (G_currentJournalPage >= StatsGetNumberOfNotes())
            G_currentJournalPage = StatsGetNumberOfNotes()-1;
    }

    else G_currentJournalPage=0;
*/

    /* draw G_currentJournalPage page of notes */
    TxtboxID = FormGetObjID(500);
    notePage=StatsGetPlayerNotePageID(G_currentJournalPage);
    if (notePage != MAX_NOTES)
    {
        /* draw journal page */
        sprintf (stmp,"UI/NOTES/NOTE%04d",notePage);
   //     printf (stmp);
   //     printf ("\ncurrent jpage=%d notepage=%d\n",G_currentJournalPage,notePage);
   //     fflush (stdout);

        tempGraphic=GraphicCreate (214,16,stmp);
        GraphicUpdateAllGraphics();
        GraphicDelete (tempGraphic);

        /* update page number field */
        DebugCheck (TxtboxID != NULL);

        sprintf (stmp,"PG %d/%d",
                 G_currentJournalPage,
                 StatsGetNumberOfNotes());
        TxtboxSetData (TxtboxID,stmp);
    }
    else
    {
        GrDrawRectangle(214,16,314,127,75);
        TxtboxSetData(TxtboxID,"NO PAGES");
    }

    DebugEnd();
}


T_void NotesChangeJournalPage (T_buttonID buttonID)
{
    T_sword16 delta;

    DebugRoutine ("NotesChangeJournalPage");
    /* get number of pages to move */
    delta=ButtonGetData(buttonID);
    G_currentJournalPage+=delta;
    if (G_currentJournalPage < StatsGetNumberOfNotes() &&
        EffectSoundIsOn()==TRUE) SoundPlayByNumber (SOUND_NOTE_PAGE,255);

    NotesUpdateJournalPage();

    DebugEnd();
}


T_void NotesGotoPageID (T_word16 notePageID)
{
    T_word16 i;
    T_word16 notePage=0;
    DebugRoutine ("NotesGotoPageID");

    for (i=0;i<=StatsGetNumberOfNotes();i++)
    {
        if (StatsGetPlayerNotePageID(i)==notePageID)
        {
            if (EffectSoundIsOn()) SoundPlayByNumber (SOUND_NOTE_PAGE,255);
            notePage=i;
            break;
        }
    }

    G_currentJournalPage=notePage;
    if (G_currentJournalPage > StatsGetNumberOfNotes())
    {
        G_currentJournalPage = StatsGetNumberOfNotes();
    }

    DebugEnd();
}



/* Routine attempts to either add a journal page (if in the mouse hand)
   or remove one (if the mouse hand is empty) */
T_void NotesTransferJournalPage (T_void)
{
    T_word16 pageNum;
    T_word16 pageID;
    T_inventoryItemStruct *p_inv;
    T_word16 type;
    T_3dObject *p_obj;

    DebugRoutine ("NotesTransferJournalPage");

    if (InventoryObjectIsInMouseHand()==FALSE)
    {
        /* we're trying to grab a page */
        if (StatsGetNumberOfNotes()>0)
        {
            /* remove the current journal page and make a new object */
            /* for the 'mouse hand' location */
            pageNum=G_currentJournalPage;
            pageID=StatsGetPlayerNotePageID(pageNum);

            /* remove this page */
            StatsRemovePlayerNotePage(pageID);

            /* create a new scroll object for this notepage */
            p_obj=ObjectCreateFake();
            ObjectSetType (p_obj,373);
            ObjectSetAccData (p_obj,pageID);
            ObjectSetAngle (p_obj,0x4000);

            /* give it to the player's inventory (in the mouse hand) */
            p_inv=InventoryTakeObject (INVENTORY_PLAYER,p_obj);
            /* set 'quick pointer' for mouse hand */
            InventorySetObjectInMouseHand(p_inv->elementID);
            /* force mouse hand picture update */
            ControlSetObjectPointer (p_inv->object);
            if (EffectSoundIsOn()==TRUE) SoundPlayByNumber (SOUND_NOTE_PAGE,255);
        }
    }
    else
    {
        /* we're probably trying to add a page */
        p_inv=InventoryCheckItemInMouseHand();
        p_obj=p_inv->object;
        DebugCheck (p_obj != NULL);
        type=ObjectGetType(p_obj);
//        MessagePrintf ("Type=%d",type);
        if (type==373)
        {
            /* it's a scroll, get the acc data and destroy it */
            pageID=ObjectGetAccData(p_obj);
            InventoryDestroyItemInMouseHand();
            /* Add the journal entry */
            StatsAddPlayerNotePage(pageID);
//            SoundPlayByNumber (SOUND_NOTE_PAGE,255);
        }
    }


    DebugEnd();
}


/* Routine returns TRUE if the mouse is over a journal entry page */
E_Boolean NotesJournalWindowIsAt (T_word16 x, T_word16 y)
{
    E_Boolean isAt=FALSE;
    DebugRoutine ("NotesJournalWindowIsAt");

    if (BannerFormIsOpen(BANNER_FORM_JOURNAL) &&
        (x > 215) &&
        (x < 313) &&
        (y > 17) &&
        (y < 126)) isAt=TRUE;

    DebugEnd();
    return (isAt);
}


