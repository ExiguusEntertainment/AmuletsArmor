/****************************************************************************/
/*    FILE:  HOUSEUI.C                                                       */
/****************************************************************************/
#include "BANNER.H"
#include "BUTTON.H"
#include "CLIENT.H"
#include "GRAPHIC.H"
#include "HARDFORM.H"
#include "HOUSEUI.H"
#include "MESSAGE.H"
#include "STATS.H"
#include "STORE.H"
#include "TXTBOX.H"

static T_graphicID G_backgroundPic = NULL;
static T_TxtboxID G_financeDisplays[4];
static T_TxtboxID G_ownedByDisplay = NULL;
static T_TxtboxID G_salePriceDisplay = NULL;
static T_buttonID G_actionButton = NULL;
static T_buttonID G_forSaleToggle = NULL;
static T_buttonID G_increasePriceButton = NULL;
static T_buttonID G_decreasePriceButton = NULL;
static E_Boolean G_iOwnThisHouse = FALSE;
static E_Boolean G_houseInventoryIsOpen = FALSE;
static T_word32 G_currentFormNum = 0;
static T_word32 G_houseSaleValue = 0;
static E_Boolean G_houseIsForSale = FALSE;
static E_Boolean G_houseIsOccupied = FALSE;

static T_void HouseUILeaveGame(T_buttonID buttonID);

T_void HouseUIStart(T_word32 formNum)
{
    T_word16 i;
    T_byte8 stmp[32];

    DebugRoutine("HouseUIStart");

    /* set house number */
    G_currentFormNum = formNum;
    G_iOwnThisHouse = StatsIOwnHouse(formNum - HOUSES_START_HARD_FORM);

    /* load backdrop */
    G_backgroundPic = GraphicCreate(4, 3, "UI/HOUSE/HOUSBACK");

    for (i = 0; i < 4; i++) {
        /* set up current funds display */
        G_financeDisplays[i] = TxtboxCreate(163, 43 + (i * 11), 203,
                52 + (i * 11), "FontMedium", 0, 0, TRUE, Txtbox_JUSTIFY_CENTER,
                Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );

        sprintf(stmp, "%d", StatsGetPlayerCoins(3-i));
        TxtboxSetData(G_financeDisplays[i], stmp);

    }

    /* set up owned by field */
    G_ownedByDisplay = TxtboxCreate(54, 21, 203, 30, "FontMedium", 0, 0, FALSE,
            Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM, NULL );
    TxtboxSetData(G_ownedByDisplay, "");

    /* set up house sale value field */
    G_salePriceDisplay = TxtboxCreate(141, 122, 179, 131, "FontMedium", 0, 0,
            FALSE, Txtbox_JUSTIFY_CENTER, Txtbox_MODE_VIEW_NOSCROLL_FORM,
            NULL );
    TxtboxSetData(G_salePriceDisplay, "");

    /* ask server for house info packet */
    HouseRequestInfoPackage(formNum);

    if (G_iOwnThisHouse == TRUE) {
        /* get inventory info for this house */
        HouseRequestHouseInventory();
        /* set up 'leave game' button */
        G_actionButton = ButtonCreate(121, 89, "UI/HOUSE/LEAVE", FALSE, 0, NULL,
                HouseUILeaveGame);

    } else {
//        BannerUIModeOn();
//        MessageSetAlternateOutputOn();
    }

    BannerOpenForm(BANNER_FORM_INVENTORY);
    GraphicUpdateAllGraphics();

    DebugEnd();
}

T_void HouseUIUpdate(T_void)
{
    DebugRoutine("HouseUIUpdate");

    if (G_houseInventoryIsOpen) {
        StoreUIUpdate();
    }

    DebugEnd();
}

T_void HouseUIEnd(T_void)
{
    T_word16 i;
    DebugRoutine("HouseUIEnd");

    /* destroy ui objects */
    GraphicDelete(G_backgroundPic);
    for (i = 0; i < 4; i++) {
        TxtboxDelete(G_financeDisplays[i]);
    }

    TxtboxDelete(G_ownedByDisplay);
    TxtboxDelete(G_salePriceDisplay);
    if (G_actionButton != NULL )
        ButtonDelete(G_actionButton);
    if (G_forSaleToggle != NULL )
        ButtonDelete(G_forSaleToggle);
    if (G_increasePriceButton != NULL )
        ButtonDelete(G_increasePriceButton);
    if (G_decreasePriceButton != NULL )
        ButtonDelete(G_decreasePriceButton);

    if (G_houseInventoryIsOpen == TRUE) {
        StoreCloseInventory();
    } else {
//        BannerUIModeOff();
//        MessageSetAlternateOutputOff();
    }

    G_ownedByDisplay = NULL;
    G_salePriceDisplay = NULL;
    G_actionButton = NULL;
    G_forSaleToggle = NULL;
    G_currentFormNum = 0;
    G_iOwnThisHouse = FALSE;
    G_houseInventoryIsOpen = FALSE;
    G_houseSaleValue = 0;
    G_houseIsForSale = FALSE;
    G_houseIsOccupied = FALSE;

    DebugEnd();
}

/************** CLIENT/SERVER ROUTINES HERE ******************/
/* routine is currently called from HouseUIStart */
/* should send a packet to server requesting information about */
/* house number FormNum */
T_void HouseRequestInfoPackage(T_word32 formNum)
{
    DebugRoutine("HouseRequstInfoPackage");

    /* for now, we will fake an instantaneous transfer */
    /* later, client should call HouseSetup when server sends */
    /* info package */

    if (formNum == 20) {
        HouseSetup("Mikey", TRUE, 0, FALSE);
    } else if (formNum == 21) {
        HouseSetup("Keith", FALSE, 0, FALSE);
    } else if (formNum == 22) {
        HouseSetup("George", TRUE, 4000, FALSE);
    } else {
        HouseSetup("Pappy", FALSE, 10000, TRUE);
    }

    DebugEnd();
}

/* house setup should be called when client recieves house info packet */
T_void HouseSetup(
        T_byte8 *p_ownerName,
        E_Boolean isForSale,
        T_word32 salePrice,
        E_Boolean isOccupied)
{
    T_byte8 stmp[32];
    T_graphicID tempGraphic;
    DebugRoutine("HouseSetup");

    if (isOccupied == TRUE) {
        /* someone's already here, exit and inform user */
        MessageAdd("Someone is already in this house!");
        HouseUIEnd();
        ClientGotoPlace(1, 0);
    } else {
        G_houseSaleValue = salePrice;
        G_houseIsForSale = isForSale;
        G_houseIsOccupied = isOccupied;

        if (G_iOwnThisHouse == TRUE) {

            TxtboxSetData(G_ownedByDisplay, StatsGetName());

            /* add radio button toggle for sale on/off */
            G_forSaleToggle = ButtonCreate(134, 109, "UI/CREATEC/CRC_TOG2",
                    TRUE, 0, HouseChangeStatus, HouseChangeStatus);

            ButtonSetSelectPic(G_forSaleToggle, "UI/CREATEC/CRC_TOG1");

            /* set state of radio toggle button */
            if (isForSale) {
                sprintf(stmp, "%d\r", salePrice);
                TxtboxSetData(G_salePriceDisplay, stmp);
                ButtonDownNoAction(G_forSaleToggle);
            } else {
                ButtonUpNoAction(G_forSaleToggle);
            }

            /* add increase/decrease sale price button */
            G_increasePriceButton = ButtonCreate(121, 123, "UI/COMMON/UPARROW",
                    FALSE, 0, NULL, HouseChangeStatus);

            G_decreasePriceButton = ButtonCreate(131, 123, "UI/COMMON/DNARROW",
                    FALSE, 0, NULL, HouseChangeStatus);

        } else {
            /* set owner name */
            TxtboxSetData(G_ownedByDisplay, p_ownerName);

            if (isForSale) {
                sprintf(stmp, "%d\r", salePrice);
                TxtboxSetData(G_salePriceDisplay, stmp);
                tempGraphic = GraphicCreate(135, 110, "UI/CREATEC/CRC_TOG1");

                /* add 'buy house' button */
                DebugCheck(G_actionButton==NULL);
                G_actionButton = ButtonCreate(121, 89, "UI/HOUSE/BUY", FALSE, 0,
                        NULL, HouseChangeStatus);
            } else {
                tempGraphic = GraphicCreate(134, 109, "UI/CREATEC/CRC_TOG2");
            }

            GraphicUpdateAllGraphics();
            GraphicDelete(tempGraphic);
        }
    }
    DebugEnd();
}

/* called by HouseStart if player owns this house */
/* will later be called on successful steal */
T_void HouseRequestHouseInventory(T_void)
{
    T_graphicID tempGraphic;
    DebugRoutine("HouseRequestHouseInventory");

    /* open up house inventory display */
    /* replace 'door' graphic */
    tempGraphic = GraphicCreate(9, 39, "UI/HOUSE/HOUSEINV");
    GraphicUpdateAllGraphics();
    GraphicDelete(tempGraphic);

    /* set up store / house inventory */
    StoreSetUpInventory(10, 40, 112, 124, 6, 6);
    StoreSetHouseModeOn();
    G_houseInventoryIsOpen = TRUE;

    DebugEnd();
}

/* this function is a callback that intercepts a click on the house for */
/* sale toggle or the 'buy house' button */
T_void HouseChangeStatus(T_buttonID buttonID)
{
    T_byte8 stmp[32];
    DebugRoutine("HouseChangeStatus");

    /* determine if we selected the toggle or buy house button */
    if (buttonID == G_actionButton) {
        /* buy house button selected */
        /* does the player have enough money ? */
        if (StatsGetPlayerTotalCarriedWealth() >= G_houseSaleValue * 100) {
            /* deduct price */
            StatsChangePlayerTotalCarriedWealth(-((T_sword32)G_houseSaleValue) * 100);

            /* change ownership of house */
            StatsSetIOwnHouse(G_currentFormNum - HOUSES_START_HARD_FORM, TRUE);
            MessageAdd("^009Congrats! You now own this fine house.");
            /* get inventory / open house inventory */
            HouseRequestHouseInventory();
            /* clear 'house for sale' flag */
            G_houseIsForSale = FALSE;
            G_iOwnThisHouse = TRUE;
            /* put player's name in owner box */
            TxtboxSetData(G_ownedByDisplay, StatsGetName());

            /* add radio button toggle for sale control */
            G_forSaleToggle = ButtonCreate(134, 109, "UI/CREATEC/CRC_TOG2",
                    TRUE, 0, HouseChangeStatus, HouseChangeStatus);

            ButtonSetSelectPic(G_forSaleToggle, "UI/CREATEC/CRC_TOG1");

            /* change action button to leave game */
            ButtonDelete(G_actionButton);
            G_actionButton = ButtonCreate(121, 89, "UI/HOUSE/LEAVE", FALSE, 0,
                    NULL, HouseUILeaveGame);

            /* add increase/decrease sale price button */
            G_increasePriceButton = ButtonCreate(121, 123, "UI/COMMON/UPARROW",
                    FALSE, 0, NULL, HouseChangeStatus);

            G_decreasePriceButton = ButtonCreate(131, 123, "UI/COMMON/DNARROW",
                    FALSE, 0, NULL, HouseChangeStatus);

        } else {
            MessageAdd("^005You don't have the money to buy it");
        }
    } else if (buttonID == G_forSaleToggle) {
        /* toggle button selected */
        DebugCheck(G_iOwnThisHouse==TRUE);

        if (G_houseIsForSale == FALSE) {
            /* prompt user for sale amount */
            G_houseIsForSale = TRUE;
            sprintf(stmp, "%d", G_houseSaleValue);
            TxtboxSetData(G_salePriceDisplay, stmp);
            MessageAdd("^007 Your house is now up for sale");
        } else {
            G_houseIsForSale = FALSE;
            /* clear sale amount box */
            TxtboxSetData(G_salePriceDisplay, "");
            MessageAdd("^007 Your house is no longer up for sale");
        }
        /* notify server that house sell/buy status has changed */
    } else if (buttonID == G_increasePriceButton) {
        if (G_houseIsForSale) {
            G_houseSaleValue += 25;
            if (G_houseSaleValue > MAX_HOUSE_SALE_VALUE)
                G_houseSaleValue = MAX_HOUSE_SALE_VALUE;
            sprintf(stmp, "%d", G_houseSaleValue);
            TxtboxSetData(G_salePriceDisplay, stmp);
        }
    } else if (buttonID == G_decreasePriceButton) {
        if (G_houseIsForSale) {
            if (G_houseSaleValue >= MIN_HOUSE_SALE_VALUE + 25) {
                G_houseSaleValue -= 25;
            } else
                G_houseSaleValue = MIN_HOUSE_SALE_VALUE;
            sprintf(stmp, "%d", G_houseSaleValue);
            TxtboxSetData(G_salePriceDisplay, stmp);
        }
    } else {
        /* fail, unknown control button! */
        DebugCheck(0);
    }
    DebugEnd();
}

/* callback routine for G_actionButton 'leave game' */
static T_void HouseUILeaveGame(T_buttonID buttonID)
{
    DebugRoutine("HouseUILeaveGame");

    HouseUIEnd();
    ClientGotoPlace(0, 0);

    DebugEnd();
}
