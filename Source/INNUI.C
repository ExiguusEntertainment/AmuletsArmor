/*-------------------------------------------------------------------------*
 * File:  INNUI.C
 *-------------------------------------------------------------------------*/
/**
 * Hard form where a player can play to rest up.
 *
 * @addtogroup INNUI
 * @brief Inn User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLIENT.H"
#include "GRAPHIC.H"
#include "INNUI.H"
#include "KEYSCAN.H"
#include "MESSAGE.H"
#include "STATS.H"
#include "TXTBOX.H"

static T_graphicID G_backgroundPic = NULL;
static T_buttonID G_rentButtons[4];
static T_TxtboxID G_financeDisplays[4];
static T_TxtboxID G_rentCostDisplays[4];

/* internal routine prototypes */
static T_void InnUIRent(T_buttonID buttonID);

T_void InnUIStart(T_word32 formNum)
{
    T_word16 i;
    T_byte8 stmp[32];
    T_word16 hotkey[4];

    DebugRoutine("InnUIStart");

    /* load backdrop */
    G_backgroundPic = GraphicCreate(4, 3, "UI/INN/INNBACK");

    hotkey[0] = KeyDual(KEY_SCAN_CODE_C,KEY_SCAN_CODE_ALT);
    hotkey[1] = KeyDual(KEY_SCAN_CODE_S,KEY_SCAN_CODE_ALT);
    hotkey[2] = KeyDual(KEY_SCAN_CODE_L,KEY_SCAN_CODE_ALT);
    hotkey[3] = KeyDual(KEY_SCAN_CODE_U,KEY_SCAN_CODE_ALT);

    for (i = 0; i < 4; i++) {
        /* place rent buttons */
        sprintf(stmp, "UI/INN/ROOMBTN%d", i + 1);

        G_rentButtons[i] = ButtonCreate(45, 24 + (i * 28), stmp,
        FALSE, hotkey[i],
        NULL, InnUIRent);

        ButtonSetData(G_rentButtons[i], i);

        /* set up rent cost displays */
        G_rentCostDisplays[i] = TxtboxCreate(45, 37 + (i * 28), 110,
                46 + (i * 28), "FontMedium", 0, 0,
                TRUE, Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM,
                NULL);

        /* set up current funds display */
        G_financeDisplays[i] = TxtboxCreate(164, 32 + (i * 11), 204,
                41 + (i * 11), "FontMedium", 0, 0,
                TRUE, Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM,
                NULL);

        sprintf(stmp, "%d", StatsGetPlayerCoins(3-i));
        TxtboxSetData(G_financeDisplays[i], stmp);

    }

    /* set rent prices */
    TxtboxSetData(G_rentCostDisplays[0], "free!");
    TxtboxSetData(G_rentCostDisplays[1], "2 gold");
    TxtboxSetData(G_rentCostDisplays[2], "1 platinum");
    TxtboxSetData(G_rentCostDisplays[3], "2 platinum");

    GraphicUpdateAllGraphics();

    /* welcome user */
    MessageAdd("^007Welcome to the inn!");

    /* add journal help page if necessary */
    if (StatsPlayerHasNotePage(31) == FALSE && StatsPlayerHasNotePage(0) == TRUE) {
        StatsAddPlayerNotePage(31);
    }

    DebugEnd();
}

T_void InnUIUpdate(T_void)
{
    DebugRoutine("InnUIUpdate");

    DebugEnd();
}

T_void InnUIEnd(T_void)
{
    T_word16 i;
    DebugRoutine("InnUIEnd");

    /* destroy ui objects */
    GraphicDelete(G_backgroundPic);
    for (i = 0; i < 4; i++) {
        ButtonDelete(G_rentButtons[i]);
        TxtboxDelete(G_financeDisplays[i]);
        TxtboxDelete(G_rentCostDisplays[i]);
    }

    DebugEnd();
}

/* button callback to rent a space at the inn */
static T_void InnUIRent(T_buttonID buttonID)
{
    T_word32 whichRoom;
    T_word16 newhealth;
    T_word16 newmana;

    DebugRoutine("InnUIRent");
    DebugCheck(buttonID != NULL);

    /* figure out which room we want to rent */
    whichRoom = ButtonGetData(buttonID);

    switch (whichRoom) {
        case INN_ROOM_COMMONS:
            ClientSetNextPlace(0, 0);
            break;

        case INN_ROOM_SMALL:
            /* deduct 12 silver */
            if (StatsGetPlayerTotalCarriedWealth() >= 200) {
                StatsChangePlayerTotalCarriedWealth(-200);
                /* Give player 25% Health, 25% Mana, 50% food and Water */
                StatsChangePlayerFood(1000);
                StatsChangePlayerWater(1000);

                newhealth = StatsGetPlayerHealth();
                newhealth += StatsGetPlayerMaxHealth() / 4;
                if (newhealth > StatsGetPlayerMaxHealth())
                    newhealth = StatsGetPlayerMaxHealth();
                newmana = StatsGetPlayerMana();
                newmana += StatsGetPlayerMaxMana() / 4;
                if (newmana > StatsGetPlayerMaxMana())
                    newmana = StatsGetPlayerMaxMana();
                StatsSetPlayerHealth(newhealth);
                StatsSetPlayerMana(newmana);

                /* log off */
                ClientSetNextPlace(0, 0);
            } else {
                MessageAdd("^005Gotta have the money first, pal.");
            }
            break;

        case INN_ROOM_LARGE:
            /* deduct 2 gold */
            if (StatsGetPlayerTotalCarriedWealth() >= 1000) {
                StatsChangePlayerTotalCarriedWealth(-1000);
                /* Give player 33% health, 33% mana, 100% food and water */
                StatsChangePlayerFood(2000);
                StatsChangePlayerWater(2000);

                newhealth = StatsGetPlayerHealth();
                newhealth += StatsGetPlayerMaxHealth() / 3;
                if (newhealth > StatsGetPlayerMaxHealth())
                    newhealth = StatsGetPlayerMaxHealth();
                newmana = StatsGetPlayerMana();
                newmana += StatsGetPlayerMaxMana() / 3;
                if (newmana > StatsGetPlayerMaxMana())
                    newmana = StatsGetPlayerMaxMana();
                StatsSetPlayerHealth(newhealth);
                StatsSetPlayerMana(newmana);

                /* Halve Poison */
                StatsSetPlayerPoisonLevel(StatsGetPlayerPoisonLevel()/2);
                /* log off */
                ClientSetNextPlace(0, 0);
            } else {
                MessageAdd("^005You don't look like you can afford it.");
            }
            break;

        case INN_ROOM_SUITE:
            /* deduct 1 platinum */
            if (StatsGetPlayerTotalCarriedWealth() >= 2000) {
                StatsChangePlayerTotalCarriedWealth(-2000);

                /* Give player 100% health, 100% mana, 100% food, 100% water */
                StatsChangePlayerFood(2000);
                StatsChangePlayerWater(2000);

//            newhealth=StatsGetPlayerHealth();
//            newhealth+=StatsGetPlayerMaxHealth();
//            if (newhealth > StatsGetPlayerMaxHealth())
//              newhealth=StatsGetPlayerMaxHealth();
//            newmana=StatsGetPlayerMana();
//            newmana+=StatsGetPlayerMaxMana()/4;
//            if (newmana > StatsGetPlayerMaxMana())
//              newmana=StatsGetPlayerMaxMana();
                newhealth = StatsGetPlayerMaxHealth();
                newmana = StatsGetPlayerMaxMana();
                StatsSetPlayerHealth(newhealth);
                StatsSetPlayerMana(newmana);

                /* Cure poison */
                StatsSetPlayerPoisonLevel(0);

                /* log off */
                ClientSetNextPlace(0, 0);
            } else {
                MessageAdd("^005What, are you kidding??");
            }
            break;

        default:
            /* fail! */
            DebugCheck(0);
    }

    DebugEnd();
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  INNUI.C
 *-------------------------------------------------------------------------*/
