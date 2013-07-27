/****************************************************************************/
/*    FILE:  BANKUI.C                                                       */
/****************************************************************************/
#include "BANKUI.H"
#include "BANNER.H"
#include "BUTTON.H"
#include "GENERAL.H"
#include "INVENTOR.H"
#include "MESSAGE.H"
#include "STATS.H"
#include "STORE.H"
#include "TXTBOX.H"

static T_graphicID G_backgroundPic=NULL;
static T_buttonID  G_depositButtons[4];
static T_buttonID  G_withdrawButtons[4];
static T_TxtboxID  G_financeDisplays[8];

/* internal routine prototypes */
static T_void BankUIDeposit (T_buttonID buttonID);
static T_void BankUIReorderSavedCoins();
static T_void BankUIWithdraw (T_buttonID buttonID);
static T_void BankUIUpdateGraphics (T_void);

T_void BankUIStart  (T_word32 formNum)
{
    T_word16 i;
    T_byte8 stmp[64];
    E_statsAdventureNumber adv=ADVENTURE_NONE;
    DebugRoutine ("BankUIStart");

    /* load backdrop */
    G_backgroundPic=GraphicCreate (4,3,"UI/BANK/BANKBACK");

    for (i=0;i<4;i++)
    {
        /* load deposit/withdraw buttons */
        G_depositButtons[i]=ButtonCreate(119,
                                         32+(i*11),
                                         "UI/COMMON/LTARROW",
                                         FALSE,
                                         0,
                                         NULL,
                                         BankUIDeposit);

        ButtonSetData(G_depositButtons[i],3-i);

        G_withdrawButtons[i]=ButtonCreate(131,
                                         32+(i*11),
                                         "UI/COMMON/RTARROW",
                                         FALSE,
                                         0,
                                         NULL,
                                         BankUIWithdraw);

        ButtonSetData(G_withdrawButtons[i],3-i);

        /* create financial display fields */
        G_financeDisplays[i]=TxtboxCreate(57,
                                          31+(i*11),
                                          109,
                                          40+(i*11),
                                          "FontMedium",
                                          0,
                                          0,
                                          TRUE,
                                          Txtbox_JUSTIFY_CENTER,
                                          Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                          NULL);

        sprintf (stmp,"%d",StatsGetPlayerSavedCoins(3-i));
        TxtboxSetData(G_financeDisplays[i],stmp);

        G_financeDisplays[i+4]=TxtboxCreate(148,
                                          31+(i*11),
                                          200,
                                          40+(i*11),
                                          "FontMedium",
                                          0,
                                          0,
                                          TRUE,
                                          Txtbox_JUSTIFY_CENTER,
                                          Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                          NULL);

        sprintf (stmp,"%d",StatsGetPlayerCoins(3-i));
        TxtboxSetData(G_financeDisplays[i+4],stmp);

    }

    GraphicUpdateAllGraphics();

    /* set up bank inventory */
    StoreSetUpInventory(10,82,112,124,6,3);

    /* tell store what items we will buy */
    StoreAddBuyType(EQUIP_OBJECT_TYPE_RING);
    StoreAddBuyType(EQUIP_OBJECT_TYPE_AMULET);

    adv=StatsGetCompletedAdventure();

    switch (adv)
    {
        case ADVENTURE_7:
        /* Ring of Strength 2 */
        StoreAddItem (408,1);
        /* Ring of Speed 2 */
        StoreAddItem (430,1);
        /* Amulet of Hiding */
        StoreAddItem (609,1);
        /* Amulet of Quickness */
        StoreAddItem (611,1);

        case ADVENTURE_6:
        /* Ring of Regen 1 */
        StoreAddItem (405,1);
        /* Amulet of Speed */
        StoreAddItem (602,1);
        /* Amulet of Mana Regeneration */
        StoreAddItem (619,1);

        case ADVENTURE_5:
        /* Amulet of Accuracy */
        StoreAddItem (605,1);
        /* Amulet of Stealth */
        StoreAddItem (603,1);
        /* Ring of Armor 1 */
        StoreAddItem (415,1);
        /* Amulet of Spell Focus */
        StoreAddItem (600,1);

        case ADVENTURE_4:
        /* Ring of Speed 1 */
        StoreAddItem (429,1);
        /* Ring of Accuracy 2 */
        StoreAddItem (421,1);

        case ADVENTURE_3:
        /* Ring of Strength 1 */
        StoreAddItem (407,1);
        /* Ring of Map */
        StoreAddItem (435,1);

        case ADVENTURE_2:
        /* Ring of Accuracy 1 */
        StoreAddItem (420,1);

        case ADVENTURE_1:
        /* Ring of Stealth */
        StoreAddItem (438,1);

        case ADVENTURE_NONE:
        default:
        /* Ring of Food Conservation */
        StoreAddItem (426,1);
        /* Ring of Water Conservation */
        StoreAddItem (427,1);
        /* Ring of Water Walk */
        StoreAddItem (400,1);
        /* Ring of Traction */
        StoreAddItem (432,1);
        break;

        case ADVENTURE_UNKNOWN:
        DebugCheck(0);
        break;
    }


    /* Amulet of Falling */
///  StoreAddItem (614,1);
    /* Ring of Regen 2 */
//    StoreAddItem (406,1);
    /* Ring of Strength 3 */
//    StoreAddItem (409,1);
    /* Ring of Strength 4 */
//    StoreAddItem (410,1);
    /* Ring of Strength 5 */
//    StoreAddItem (411,1);
    /* Ring of Armor 2 */
//    StoreAddItem (416,1);
    /* Ring of Armor 3 */
//    StoreAddItem (417,1);
    /* Ring of Armor 4 */
//    StoreAddItem (418,1);
    /* Ring of Armor 5 */
//    StoreAddItem (419,1);
    /* Ring of Accuracy 3 */
//  StoreAddItem (422,1);
    /* Ring of Accuracy 4 */
//  StoreAddItem (423,1);
    /* Ring of Accuracy 5 */
//  StoreAddItem (424,1);
    /* Ring of Speed 4 */
//  StoreAddItem (428,1);
    /* Ring of Speed 3 */
//  StoreAddItem (431,1);
    /* Ring of Jumping */
//  StoreAddItem (434,1);
    /* Amulet of Regeneration */
//  StoreAddItem (601,1);
    /* Amulet of Strength */
//  StoreAddItem (604,1);
    /* Amulet of Poison Protection */
//  StoreAddItem (606,1);
    /* Amulet of Fire Protection */
//  StoreAddItem (607,1);
    /* Amulet of Lava Walk */
//  StoreAddItem (608,1);
    /* Amulet of Eternal Nourishment */
//  StoreAddItem (610,1);
    /* Amulet of Jumping */
//  StoreAddItem (612,1);
    /* Amulet of Flying */
//  StoreAddItem (613,1);
    /* Amulet of Super Armor */
//  StoreAddItem (615,1);
    /* Amulet of Acid Protection */
//  StoreAddItem (616,1);
    /* Amulet of Electricity Protection */
//  StoreAddItem (617,1);
    /* Amulet of Mana Drain Protection */
//  StoreAddItem (618,1);

    InventoryReorder(INVENTORY_STORE,TRUE);
    InventoryDrawInventoryWindow (INVENTORY_STORE);

    /* set up store control callback */
    StoreSetControlCallback (BankUIUpdateGraphics);

    /* welcome user */
    MessageAdd ("^007Welcome to the bank!");

    /* add journal help page if necessary */
    if (StatsPlayerHasNotePage(33)==FALSE &&
        StatsPlayerHasNotePage(0)==TRUE)
    {
        StatsAddPlayerNotePage(33);
    }
    else
    {
        /* force loading of inventory banner display */
        BannerOpenForm(BANNER_FORM_INVENTORY);
    }

    DebugEnd();
}


T_void BankUIUpdate (T_void)
{
    DebugRoutine ("BankUIUpdate");

    DebugEnd();
}


T_void BankUIEnd    (T_void)
{
    T_word16 i;
    DebugRoutine ("BankUIEnd");

    /* destroy ui objects */
    GraphicDelete (G_backgroundPic);
    for (i=0;i<4;i++)
    {
        ButtonDelete(G_depositButtons[i]);
        ButtonDelete(G_withdrawButtons[i]);
        TxtboxDelete(G_financeDisplays[i]);
        TxtboxDelete(G_financeDisplays[i+3]);
    }

    /* close store inventory */
    StoreCloseInventory();

    DebugEnd();
}


static T_void BankUIDeposit (T_buttonID buttonID)
{
    E_equipCoinTypes coin;
    DebugRoutine ("BankUIDeposit");
    DebugCheck (buttonID != NULL);

    /* get coin type */
    coin=ButtonGetData(buttonID);
    DebugCheck (coin < COIN_TYPE_FIVE);

    StatsSetPlayerSavedCoins (coin,StatsGetPlayerCoins(coin)+StatsGetPlayerSavedCoins(coin));
    StatsSetPlayerCoins(coin,0);
    BankUIReorderSavedCoins();

    DebugEnd();
}


static T_void BankUIWithdraw (T_buttonID buttonID)
{
    E_equipCoinTypes coin;
    T_word16 trycoin;
    static T_word16 p10[]={1,10,100,1000};
    DebugRoutine ("BankUIWithdraw");

    /* get coin type */
    coin=ButtonGetData(buttonID);
    DebugCheck (coin < COIN_TYPE_FIVE);

    if (StatsGetPlayerSavedCoins(coin)>0)
    {
        StatsSetPlayerSavedCoins(coin,StatsGetPlayerSavedCoins(coin)-1);
        StatsSetPlayerCoins(coin,StatsGetPlayerCoins(coin)+1);
        BankUIReorderSavedCoins();
    }
    else
    {
        /* try to get 10 of this type of coin by making 'change' */
        trycoin=coin;
        while (trycoin<COIN_TYPE_PLATINUM)
        {
            trycoin++;
            if (StatsGetPlayerSavedCoins(trycoin)>0)
            {
                /* success, subtract one and reorder to coin */
                StatsSetPlayerSavedCoins(trycoin,StatsGetPlayerSavedCoins(trycoin)-1);
                StatsSetPlayerSavedCoins(coin,(p10[trycoin-coin])-1);
                StatsSetPlayerCoins (coin,StatsGetPlayerCoins(coin)+1);
                BankUIReorderSavedCoins();
                break;
            }
        }
    }
    DebugEnd();
}


static T_void BankUIReorderSavedCoins()
{
    T_word16 i;

    DebugRoutine ("BankUIReorderSavedCoins");
    for (i=COIN_TYPE_COPPER;i<=COIN_TYPE_GOLD;i++)
    {
        StatsSetPlayerSavedCoins(i+1,StatsGetPlayerSavedCoins(i+1)+(StatsGetPlayerSavedCoins(i)/10));
        StatsSetPlayerSavedCoins(i,StatsGetPlayerSavedCoins(i)%10);
    }

    /* update graphics */
    BankUIUpdateGraphics();

    DebugEnd();
}


static T_void BankUIUpdateGraphics (T_void)
{
    T_byte8 stmp[64];
    T_word16 i;

    DebugRoutine ("BankUIUpdateGraphics");
    for (i=0;i<4;i++)
    {
        sprintf (stmp,"%d",StatsGetPlayerSavedCoins(3-i));
        TxtboxSetData(G_financeDisplays[i],stmp);

        sprintf (stmp,"%d",StatsGetPlayerCoins(3-i));
        TxtboxSetData(G_financeDisplays[i+4],stmp);
    }


    if (BannerFormIsOpen (BANNER_FORM_FINANCES)) BannerDisplayFinancesPage();
    GraphicUpdateAllGraphics();

    DebugEnd();
}


