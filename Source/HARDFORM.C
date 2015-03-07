/*-------------------------------------------------------------------------*
 * File:  HARDFORM.C
 *-------------------------------------------------------------------------*/
/**
 * Hard Forms are predefined forms in the game system that the player
 * can jump between.  Town UI, Bank UI, and Guild UI are all examples.
 * Going to a Hard Form causes the game to drop all previous forms
 * (except the banner UI) for this form.
 *
 * @addtogroup HARDFORM
 * @brief Hard Form UI Game System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BANNER.H"
#include "BANKUI.H"
#include "BUTTON.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CSYNCPCK.H"
#include "HARDFORM.H"
#include "HOUSEUI.H"
#include "INNUI.H"
#include "MESSAGE.H"
#include "PEOPHERE.H"
#include "SCHEDULE.H"
#include "STATS.H"
#include "STORE.H"
#include "TOWNUI.H"
#include "GUILDUI.H"
#include "TICKER.H"

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define PLAYER_ID_INTERVAL      (TICKS_PER_SECOND * 2) // once every 2 seconds

/*-------------------------------------------------------------------------*
 * Types:
 *-------------------------------------------------------------------------*/
typedef struct {
    T_hardFormStart start;
    T_hardFormEnd end;
    T_hardFormUpdate update;
    T_hardFormHandleMouse handleMouse;
} T_hardFormCallbackGroup;

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
static T_buttonID G_closeButton = NULL;
static T_void HardFormExit(T_buttonID buttonID);
static E_Boolean HardFormOpen = FALSE;
static T_word32 G_timeIDLastUpdated = 0;

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
static T_hardFormCallbackGroup G_callbackGroups[HARD_FORM_UNKNOWN] = {
    {
        BankUIStart,                      /* start */
        BankUIEnd,                        /* end */
        BankUIUpdate,                     /* update */
        StoreHandleTransaction           /* handle mouse */
    },
    {
        StoreUIStart,
        StoreUIEnd,
        StoreUIUpdate,
        StoreHandleTransaction
    },
    {   InnUIStart,
        InnUIEnd,
        InnUIUpdate,
        NULL
    },
    {
        HouseUIStart,
        HouseUIEnd,
        HouseUIUpdate,
        StoreHandleTransaction
    },
    {
        TownUIStart,
        TownUIEnd,
        TownUIUpdate,
        NULL
    },
    {
        GuildUIStart,
        GuildUIEnd,
        GuildUIUpdate,
        NULL
    }
};

static T_word32 G_currentForm = HARD_FORM_UNKNOWN;

T_void HardFormStart(T_word32 formNum)
{
    T_word32 formID;

    DebugRoutine("HardFormStart");

    HardFormOpen = TRUE;
    /* All hard forms have an alternate output turned on. */
    MessageSetAlternateOutputOn();
    BannerUIModeOn();

    formNum -= STORE_ID_START;
    formID = formNum;
    DebugCheck(formNum < HARD_FORM_UNKNOWN);
    DebugCheck(G_currentForm == HARD_FORM_UNKNOWN);

//    if (formID!=HARD_FORM_TOWN)
    if (1) {
        /* create hardform close button */
        G_closeButton = ButtonCreate(196, 5, "UI/COMMON/CLOSEWIN", FALSE, 0,
                NULL, HardFormExit);
    } else {
        G_closeButton = NULL;
    }

    if (formNum >= 20)
        formNum = HARD_FORM_HOUSE;

    G_currentForm = formNum;

    if (G_callbackGroups[G_currentForm].start != NULL )
        G_callbackGroups[G_currentForm].start(formID);

    if (G_timeIDLastUpdated)
        G_timeIDLastUpdated = TickerGet();

    DebugEnd();
}

T_void HardFormEnd(T_void)
{
    DebugRoutine("HardFormEnd");

    if (G_closeButton) {
        ButtonDelete(G_closeButton);
        G_closeButton = NULL;
    }
//    if (G_currentForm != HARD_FORM_UNKNOWN)  {
    MessageSetAlternateOutputOff();
    if (BannerIsOpen())
        BannerUIModeOff();

    DebugCheck(G_currentForm != HARD_FORM_UNKNOWN);
    if (G_callbackGroups[G_currentForm].end != NULL )
        G_callbackGroups[G_currentForm].end();

    G_currentForm = HARD_FORM_UNKNOWN;
//    }

    HardFormOpen = FALSE;

    DebugEnd();
}

static T_void HardFormExit(T_buttonID buttonID)
{
    DebugRoutine("HardFormExit");

    /* Tell others that I'm no longer creating a game. */
    PeopleHereSetOurState(PLAYER_ID_STATE_NONE);

    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf();

    PeopleHereSetOurAdventure(0);
    ClientSyncSetGameGroupID(*DirectTalkGetNullBlankUniqueAddress());
    /* Send out a message that this is what we are doing. */
    ClientSendPlayerIDSelf();

    ButtonDelete(G_closeButton);
    G_closeButton = NULL;
    if (TownUIIsOpen()) {
        // Save the character!
        StatsSaveCharacter(StatsGetActive());

        // Exit town
        ClientSetNextPlace(0, 0);
    } else {
        ClientSetNextPlace(HARDFORM_GOTO_PLACE_OFFSET + HARD_FORM_TOWN, 0);
    }

    DebugEnd();
}

T_void HardFormHandleMouse(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick buttons)
{
    DebugRoutine("HardFormHandleMouse");

    if (G_currentForm < HARD_FORM_UNKNOWN) {
        if (G_callbackGroups[G_currentForm].handleMouse != NULL )
            G_callbackGroups[G_currentForm].handleMouse(event, x, y, buttons);
    }

    DebugEnd();
}

void HardFormNetworkUpdate(void)
{
    T_word32 time;

    DebugRoutine("HardFormNetworkUpdate");

    // For players sitting in the town ui, send out an player ID
    // every so often.  If the list is not updated, players will be
    // dropped.
    time = TickerGet();
    if ((time - G_timeIDLastUpdated) >= PLAYER_ID_INTERVAL) {
        G_timeIDLastUpdated += PLAYER_ID_INTERVAL;
        ClientSendPlayerIDSelf();
    }

    DebugEnd();
}

T_void HardFormUpdate(T_void)
{
    DebugRoutine("HardFormUpdate");
    if (G_currentForm < HARD_FORM_UNKNOWN) {
        GrScreenSet(GRAPHICS_ACTUAL_SCREEN);
        BannerUpdate();
        StatsUpdatePlayerStatistics();
        GraphicUpdateAllGraphics();
        ScheduleUpdateEvents();
        KeyboardUpdateEvents();
        MouseUpdateEvents();
        if (G_callbackGroups[G_currentForm].update != NULL )
            G_callbackGroups[G_currentForm].update();
        HardFormNetworkUpdate();
    }

    DebugEnd();
}

E_Boolean HardFormIsOpen(T_void)
{
    return (HardFormOpen);
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  HARDFORM.C
 *-------------------------------------------------------------------------*/
