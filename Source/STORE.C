/*-------------------------------------------------------------------------*
 * File:  STORE.C
 *-------------------------------------------------------------------------*/
/**
 * Buying and selling items in the store goes here.
 * Store stock is also determined here.
 *
 * @addtogroup STORE
 * @brief Store User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BANNER.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "STATS.H"
#include "STORE.H"
#include "TXTBOX.H"

static T_graphicID G_backgroundPic=NULL;
static E_Boolean   G_houseMode=FALSE;
static T_TxtboxID  G_financeDisplays[4];
static E_Boolean   G_storeOpen=FALSE;

static T_void StoreUIUpdateGraphics (T_void);
static T_storeCallback G_controlCallback=NULL;
static T_inventoryItemStruct* G_itemToSell=NULL;
static T_inventoryItemStruct* G_itemToBuy=NULL;

/* enumed eboolean to determine if the current store will buy this type */
/* of object -- see StoreAddBuyType */
static E_Boolean G_storeWillBuy[EQUIP_OBJECT_TYPE_UNKNOWN];


T_void StoreSetUpInventory (T_word16 x1,
                            T_word16 y1,
                            T_word16 x2,
                            T_word16 y2,
                            T_word16 gridspacesx,
                            T_word16 gridspacesy)
{
    T_word16 i;
    DebugRoutine ("StoreSetUpInventory");

    G_storeOpen=TRUE;
    /* initialize dual inventory mode */
    InventorySetInventoryWindowLocation (INVENTORY_STORE,
                                         x1,
                                         y1,
                                         x2,
                                         y2,
                                         (T_byte8)gridspacesx,
                                         (T_byte8)gridspacesy,
                                         20);

    InventoryOpenStoreInventory();

    /* set banner to alternate mode */
//    BannerUIModeOn();

    /* clear out store will buy flags */
    for (i=0;i<EQUIP_OBJECT_TYPE_UNKNOWN;i++)
    {
        G_storeWillBuy[i]=FALSE;
    }

    DebugEnd();
}


T_void StoreCloseInventory (T_void)
{
    DebugRoutine ("StoreCloseInventory");

    G_storeOpen=FALSE;
    InventoryCloseStoreInventory();

    /* set banner back to normal mode */
//   BannerUIModeOff();

    G_houseMode=FALSE;

    /* clear control callback */
    StoreSetControlCallback (NULL);

    DebugEnd();
}


/* should be called when client recieves a packet instructing an item */
/* to be added to the store */
T_void StoreAddItem (T_word16 objectToAdd, T_word16 numberToAdd)
{
    T_word16 i;
    DebugRoutine ("StoreAddItem");

    for (i=0;i<numberToAdd;i++)
    {
		// TODO: Why is there a loop here AND a count passed in?
        InventoryAddObjectToInventory(INVENTORY_STORE,objectToAdd,(T_byte8)numberToAdd);
    }
//  InventoryReorder(INVENTORY_STORE,TRUE);
//  InventoryDrawInventoryWindow (INVENTORY_STORE);

    DebugEnd();
}


/* should be called when client recieves a packet instructing to remove */
/* an item from the store */
T_void StoreRemoveItem (T_word16 objectToRemove, T_word16 numberToRemove)
{
    T_word16 i;
    DebugRoutine ("StoreRemoveItem");
    for (i=0;i<numberToRemove;i++)
    {
		// TODO: Why is there a loop here AND a count passed in?
        InventoryRemoveObjectFromInventory (INVENTORY_STORE,objectToRemove,(T_byte8)numberToRemove);
    }
    InventoryReorder (INVENTORY_STORE,TRUE);
    InventoryDrawInventoryWindow (INVENTORY_STORE);

    DebugEnd();
}


/* routine will handle buying and selling of items for store or other */
/* alternate inventory */
T_void StoreHandleTransaction (E_mouseEvent event,
                                       T_word16 x,
                                       T_word16 y,
                                       T_buttonClick buttons)
{
    T_inventoryItemStruct* itemInHand=NULL;
    T_inventoryItemStruct* itemInStore=NULL;
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugRoutine ("StoreHandleTransaction");

    /* first, see if we've selected the alternate inventory */
    if (InventoryFindInventoryWindow(x,y)==INVENTORY_STORE)
    {
        /* ok, we clicked on it. */
        /* do we have something in the mouse hand currently? */
        itemInHand=InventoryCheckItemInMouseHand();
        if (itemInHand != NULL)
        {
            if (G_houseMode==FALSE)
            {
                /* we must be trying to sell this item */
                /* check to see if the store wants to buy it */
                if (StoreWillBuy(itemInHand->itemdesc.type) &&
                    StoreGetBuyValue(itemInHand)>0)
                {
                    /* notify the server that the store wants to buy an item */
                    /* from the player */
                    StoreRequestBuyItem(itemInHand);
                }
                else
                {
                    MessageAdd ("^005I don't want to buy that from you.");
                }
            }
            else
            {
                StoreRequestBuyItem(itemInHand);
            }
        }
        else
        {
            if (G_houseMode==FALSE)
            {
                /* we must be trying to buy something */
                /* is there an item in this location ? */
                itemInStore=InventoryCheckItemInInventoryArea(x,y);
                if (itemInStore != NULL)
                {
                    /* there's something here. Can we buy it? */
                    if (StatsGetPlayerTotalCarriedWealth()>=ObjectGetValue(itemInStore->object)
                        || EffectPlayerEffectIsActive(PLAYER_EFFECT_GOD_MODE))
                    {
                        /* notify the server that the store wants to sell */
                        /* an item to the player */
                        StoreRequestSellItem (itemInStore);
                    }
                    else
                    {
                        MessageAdd ("^005You can't afford that!");
                    }
                }
            }
            else
            {
                itemInStore=InventoryCheckItemInInventoryArea(x,y);
                if (itemInStore != NULL)
                {
                    StoreRequestSellItem(itemInStore);
                }
            }
        }
    }

    /* notify callback */
    if (G_controlCallback != NULL) G_controlCallback();

    DebugEnd();
}


/* called by HandleTransaction when user wants to buy a selected item */
T_void StoreRequestSellItem (T_inventoryItemStruct *p_inv)
{
    DebugRoutine ("StoreRequestSellItem");

    /* store is going to sell an item to the player */
    /* send a packet to the server stating such */
    G_itemToSell=p_inv;

    /* Request to sell/add item to store. */
    StoreConfirmSellItem() ;

    DebugEnd();
}

/* ConfirmSellItem is called when client recieves ok from server */
T_void StoreConfirmSellItem (T_void)
{
    T_word32 amount;
    T_byte8 stmp[64];
    T_byte8 stmp2[16];
    T_3dObject *p_obj;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("StoreConfirmSellItem");
    DebugCheck (G_itemToSell != NULL);

    /* move the item from the store inventory to the
       player inventory */
//    if (G_itemToSell->numitems > 1)
//    {
        if (G_itemToSell->itemdesc.type==EQUIP_OBJECT_TYPE_QUIVER)
        {
            /* don't make a new item */
            /* just add 12 arrows */
            Effect (EFFECT_TAKE_AMMO,
                    EFFECT_TRIGGER_GET,
                    G_itemToSell->itemdesc.subtype,
                    12,
                    0,
                    G_itemToSell->object);
        }
        else
        {
            p_obj=ObjectCreateFake();
            DebugCheck (p_obj != NULL);

            ObjectSetType (p_obj,ObjectGetType(G_itemToSell->object));
            ObjectSetAngle (p_obj,0x4000);

            p_inv=InventoryTakeObject (INVENTORY_PLAYER,p_obj);
            /* set 'quick pointer' for mouse hand */
            InventorySetMouseHandPointer (p_inv->elementID);
            InventoryDoEffect(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);
        }

        /* identify object type for player */
        StatsPlayerIdentify(ObjectGetType(p_obj));

        /* force mouse hand picture update */
//      ControlSetObjectPointer (p_inv->object);
//        StatsChangePlayerLoad(ObjectGetWeight(G_itemToSell->object));
//      G_itemToSell->numitems--;
//    }
//    else
//    {
//        element=InventoryTransferItemBetweenInventories(
//                  G_itemToSell,
//                  INVENTORY_STORE,
//                  INVENTORY_PLAYER);

//        DebugCheck (element != DOUBLE_LINK_LIST_ELEMENT_BAD);
        /* set the mouse hand pointer */
//        InventorySetMouseHandPointer (element);
//    }

    if (G_houseMode==FALSE)
    {
        /* go ahead and sell the item to the player */
        /* subtract the cost of this item */
        if (EffectPlayerEffectIsActive(PLAYER_EFFECT_GOD_MODE)==FALSE)
        {
            amount=ObjectGetValue(G_itemToSell->object);
            StoreConvertCurrencyToString (stmp,amount);
            if (G_itemToSell->itemdesc.type==EQUIP_OBJECT_TYPE_QUIVER)
            {
                switch (G_itemToSell->itemdesc.subtype)
                {
                    case BOLT_TYPE_NORMAL:
                    strcpy (stmp2,"normal");
                    break;
                    case BOLT_TYPE_POISON:
                    strcpy (stmp2,"green");
                    break;
                    case BOLT_TYPE_PIERCING:
                    strcpy (stmp2,"brown");
                    break;
                    case BOLT_TYPE_FIRE:
                    strcpy (stmp2,"red");
                    break;
                    case BOLT_TYPE_ELECTRICITY:
                    strcpy (stmp2,"yellow");
                    break;
                    case BOLT_TYPE_MANA_DRAIN:
                    strcpy (stmp2,"purple");
                    break;
                    case BOLT_TYPE_ACID:
                    strcpy (stmp2,"white");
                    break;
                    default:
                    DebugCheck (0); /* bad bolt! */
                    break;
                }
                MessagePrintf ("^009You bought 12 %s arrows for %s",stmp2,stmp);
            }
            else MessagePrintf ("^009Sold!! for %s",stmp);
            StatsChangePlayerTotalCarriedWealth(-(T_sword32)amount);
        }
        else
        {
            MessageAdd ("^009Well... You're a god... it's free.");
        }
//        StoreUIUpdateGraphics();

    }

    /* resort the inventory */
    InventoryReorder (INVENTORY_STORE,TRUE);

    /* redraw the inventory window */
    InventoryDrawInventoryWindow(INVENTORY_STORE);

    G_itemToSell=NULL;

    DebugEnd();
}


/* called by HandleTransaction when user wants to sell item in mousehand */
T_void StoreRequestBuyItem (T_inventoryItemStruct *p_item)
{
    DebugRoutine ("StoreRequestBuyItem");
    /* store is going to buy an item from the player */
    /* send packet to server stating such */

    G_itemToBuy=p_item;
    /* Request to buy/remove item from store. */
    StoreConfirmBuyItem() ;

    DebugEnd();
}

/* ConfirmBuyItem is called when client recieves ok from server */
T_void StoreConfirmBuyItem (T_void)
{
    T_word16 amount=0;
    T_byte8 stmp[64];

    DebugRoutine ("StoreConfirmBuyItem");
    DebugCheck (G_itemToBuy != NULL);

    /* go ahead and buy the item from the player */

    /* clear the object from the 'mousehand' pointer */
//    InventoryClearObjectInMouseHand();

    /* transfer it to the store inventory */
//    InventoryTransferItemBetweenInventories (G_itemToBuy,
//                                             INVENTORY_PLAYER,
//                                             INVENTORY_STORE);
    if (G_houseMode==FALSE)
    {
        /* give the player some money */
        amount=StoreGetBuyValue(G_itemToBuy);
        StoreConvertCurrencyToString (stmp,amount);
        MessagePrintf ("^009Thanks! Here's your %s",stmp);
        StatsChangePlayerTotalCarriedWealth (amount);
//        StoreUIUpdateGraphics();
    }

    /* destroy the 'bought' item */
    InventoryDestroyItemInMouseHand();
    /* reorder the store inventory */
//    InventoryReorder(INVENTORY_STORE,TRUE);
    /* go to the first page that this object was stored at */
//    InventoryFindInventoryItemPage(INVENTORY_STORE,G_itemToBuy);
    /* redraw the inventory window */
//    InventoryDrawInventoryWindow(INVENTORY_STORE);

    G_itemToBuy=NULL;

    DebugEnd();
}


/* sets a callback routine that will be called when store control */
/* intercepts a mouse event */

T_void StoreSetControlCallback (T_storeCallback callback)
{
    DebugRoutine ("StoreSetControlCallback");

    G_controlCallback = callback;

    DebugEnd();
}


/* determines if the store will buy a specific type of object */
E_Boolean StoreWillBuy (E_equipObjectTypes type)
{
    E_Boolean willbuy=FALSE;
    DebugRoutine ("StoreWillBuy");
    DebugCheck (type<EQUIP_OBJECT_TYPE_UNKNOWN);

    willbuy=G_storeWillBuy[type];

    DebugEnd();
    return (willbuy);
}

/* adds a type of object that the store will buy to a list.  */
/* note that these flags are cleared when the store is initialized via */
/* call to StoreSetUpInventory */
T_void StoreAddBuyType (E_equipObjectTypes type)
{
    DebugRoutine ("StoreAddBuyTypes");
    DebugCheck (type<EQUIP_OBJECT_TYPE_UNKNOWN);

    G_storeWillBuy[type]=TRUE;

    DebugEnd();
}


/****************************/
/* begin store UI only code */
/****************************/


/* loads 'store ui' */
T_void StoreUIStart  (T_word32 formNum)
{
    T_word16 i;
    E_statsAdventureNumber adv=ADVENTURE_NONE;
    const T_word16 firstArmor=12988;
    const T_word16 firstWeapon=12488;
    /* bronze, silver, steel, hsteel */
    const T_word16 materialOffsets[4]={0,8192,12288,16384};
    DebugRoutine ("StoreUIStart");

    /* load backdrop */
    G_backgroundPic=GraphicCreate (4,3,"UI/STORE/STORBACK");

    for (i=0;i<4;i++)
    {
        /* create financial display fields */
        G_financeDisplays[i]=TxtboxCreate(164,
                                          32+(i*11),
                                          204,
                                          41+(i*11),
                                          "FontMedium",
                                          0,
                                          0,
                                          TRUE,
                                          Txtbox_JUSTIFY_CENTER,
                                          Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                          NULL);

    }

    StoreUIUpdateGraphics();
    GraphicUpdateAllGraphics();

    /* set up store inventory */
    StoreSetUpInventory(10,26,112,124,6,7);

    /* set up what items the store will buy */
    StoreAddBuyType (EQUIP_OBJECT_TYPE_WEAPON);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_ARMOR);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_POTION);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_WAND);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_SCROLL);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_THING);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_BOLT);
    StoreAddBuyType (EQUIP_OBJECT_TYPE_QUIVER);

    adv=StatsGetCompletedAdventure();

    switch (adv)
    {
        case ADVENTURE_7:
        /* Potion of Invisibility */
        StoreAddItem (815,1);
        /* Wand of Disintegrate */
        StoreAddItem (918,1);
        /* Mage rune 9 */
        StoreAddItem (308,1);
        /* Adiminium long sword */
        StoreAddItem (45260, 1);
        /* Adaminium dagger */
        StoreAddItem (45256, 1);
        /* Adaminium mace */
        StoreAddItem (45257, 1);

        case ADVENTURE_6:
        /* Axe of Power */
        StoreAddItem (232,1);
        /* Dagger of Piercing */
        StoreAddItem (215,1);
        /* Scroll of Health 3 */
        StoreAddItem (356,1);
        /* Scroll of Flying */
        StoreAddItem (360,1);
        /* Scroll of Invisibility */
        StoreAddItem (361,1);
        /* Potion of Healing 3 */
        StoreAddItem (802,1);
        /* Potion of Armor 5 */
        StoreAddItem (811,1);
        /* Wand of Homing Deathballs */
        StoreAddItem (905,1);
        /* Wand of Lightning Bolts */
        StoreAddItem (907,1);
        /* Mage rune 6, 8 */
        StoreAddItem (305, 1);
        StoreAddItem (307, 1);
        /* Obsidian dagger */
        StoreAddItem(37064, 1);
        /* Obsidian short sword */
        StoreAddItem(37067,1);

        case ADVENTURE_5:
        /* Long Sword of Striking */
        StoreAddItem (212,1);
        /* Potion of Flying */
        StoreAddItem (805,1);
        /* Potion of Armor 4 */
        StoreAddItem (810,1);
        /* Potion of Lava Walking */
        StoreAddItem (813,1);
        /* Potion of Translucency */
        StoreAddItem (816,1);
        /* Potion of Demon Strength */
        StoreAddItem (820,1);
        /* Wand of Homing Fireballs */
        StoreAddItem (904,1);
        /* Wand of Berserk */
        StoreAddItem (914,1);
        /* Quiver of piercing bolts */
        StoreAddItem (147,1);
        /* Mage rune 7 */
        StoreAddItem (306,1);
        /* Cleric rune 9 */
        StoreAddItem (317,1);
        /* Dagger of fire */
        StoreAddItem (214,1);
        /* Mithril staff */
        StoreAddItem(32970,1);
        /* Mithril short sword */
        StoreAddItem(32971,1);
        /* Mithril long sword */
        StoreAddItem(32972,1);
        /* Mithril heavy armor */
        StoreAddItem (33476,1);
        StoreAddItem (33477,1);
        StoreAddItem (33478,1);
        StoreAddItem (33479,1);


        case ADVENTURE_4:
        /* Dagger of Acid */
        StoreAddItem (230,1);
        /* Staff of Poison */
        StoreAddItem (219,1);
        /* Mace of Speed */
        StoreAddItem (224,1);
        /* Short Sword of Striking */
        StoreAddItem (211,1);
        /* Scroll of Health 2 */
        StoreAddItem (355,1);
        /* Potion of Regen 2 */
        StoreAddItem (804,1);
        /* Potion of Cure Poison 2 */
        StoreAddItem (807,1);
        /* Wand of Acid */
        StoreAddItem (902,1);
        /* Wand of Fireballs */
        StoreAddItem (903,1);
        /* Wand of Earthsmite */
        StoreAddItem (906,1);
        /* Wand of Slow */
        StoreAddItem (917,1);
        /* Quiver of electricity bolts */
        StoreAddItem (149,1);
        /* Hsteel long sword */
        StoreAddItem (28876, 1);
        /* Hsteel axe */
        StoreAddItem (28877, 1);
        /* Hsteel mace */
        StoreAddItem (28873, 1);
        /* Hsteel staff */
        StoreAddItem (28874, 1);
        /* Mithril light armor, hsteel heavy armor */
        StoreAddItem (33472, 1);
        StoreAddItem (33473, 1);
        StoreAddItem (33474, 1);
        StoreAddItem (33475, 1);
        StoreAddItem (29380, 1);
        StoreAddItem (29381, 1);
        StoreAddItem (29382, 1);
        StoreAddItem (29383, 1);

        case ADVENTURE_3:
        /* Axe of Accuracy */
        StoreAddItem (228,1);
        /* Staff of Speed */
        StoreAddItem (225,1);
        /* Scroll of identify all */
        StoreAddItem (351,1);
        /* Gain Strength Scroll */
        StoreAddItem (364,1);
        /* Gain Magic Scroll */
        StoreAddItem (367,1);
        /* Gain Speed Scroll */
        StoreAddItem (368,1);
        /* Potion of Holy Strength */
        StoreAddItem (819,1);
        /* Wand of Poison Missile */
        StoreAddItem (912,1);
        /* Quiver of poison bolts */
        StoreAddItem (146,1);
        /* Cleric runes 7, 8 */
        StoreAddItem (315, 1);
        StoreAddItem (316, 1);
        /* Steel mace */
        StoreAddItem(24777,1);
        /* Steel short sword */
        StoreAddItem(24779,1);
        /* Steel long sword */
        StoreAddItem(24780,1);
        /* Steel Axe */
        StoreAddItem(24781,1);
        /* Hsteel dagger */
        StoreAddItem(28872,1);
        /* steel heavy armor */
        StoreAddItem (25284, 1);
        StoreAddItem (25285, 1);
        StoreAddItem (25286, 1);
        StoreAddItem (25287, 1);
        /* Hsteel light armor */
        StoreAddItem (29376, 1);
        StoreAddItem (29377, 1);
        StoreAddItem (29378, 1);
        StoreAddItem (29379, 1);


        case ADVENTURE_2:
        /* Dagger of Speed */
        StoreAddItem (223,1);
        /* Scroll of Regeneration 1 */
        StoreAddItem (357,1);
        /* Gain Mana Scroll #2 */
        StoreAddItem (363,1);
        /* Gain Electric Attack Scroll */
        StoreAddItem (370,1);
        /* Potion of Armor 3 */
        StoreAddItem (809,1);
        /* Potion of Speed */
        StoreAddItem (814,1);
        /* Wand of Homing Darts */
        StoreAddItem (901,1);
        /* Quiver of acid bolts */
        StoreAddItem (148,1);
        /* mage runes 4, 5 */
        StoreAddItem (303,1);
        StoreAddItem (304,1);
        /* Cleric runes 4, 5, 6*/
        StoreAddItem (312,1);
        StoreAddItem (313,1);
        StoreAddItem (314,1);
        /* Silver long sword */
        StoreAddItem(20684, 1);
        /* Silver Axe */
        StoreAddItem(20685, 1);
        /* Steel dagger */
        StoreAddItem(24776, 1);
        /* Steel staff */
        StoreAddItem(24778, 1);
        /* Steel armor */
        StoreAddItem (25277, 1);
        StoreAddItem (25278, 1);
        StoreAddItem (25279, 1);
        StoreAddItem (25280, 1);
        StoreAddItem (25281, 1);
        StoreAddItem (25282, 1);
        StoreAddItem (25283, 1);


        case ADVENTURE_1:
        /* Dagger of Accuracy */
        StoreAddItem (227,1);
        /* Gain Accuracy Scroll */
        StoreAddItem (365,1);
        /* Gain Stealth Scroll */
        StoreAddItem (366,1);
        /* Gain Fire Attack Scroll */
        StoreAddItem (371,1);
        /* Potion of Healing 2 */
        StoreAddItem (801,1);
        /* Potion of Regen 1 */
        StoreAddItem (803,1);
        /* Potion of Cure Poison 1 */
        StoreAddItem (806,1);
        /* Potion of Feather Falling */
        StoreAddItem (817,1);
        /* Potion of Night Vision */
        StoreAddItem (823,1);
        /* Potion of Traction */
        StoreAddItem (824,1);
        /* Potion of Giant Strength */
        StoreAddItem (818,1);
        /* Wand of Darts */
        StoreAddItem (900,1);
        /* Wand of Confusion */
        StoreAddItem (913,1);
        /* Wand of Unlock Door */
        StoreAddItem (920,1);
        /* Quiver of fire bolts */
        StoreAddItem (145,1);
        /* mage runes 2 and 3 */
        StoreAddItem (301,1);
        StoreAddItem (302,1);
        /* Cleric rune 2 and 3 */
        StoreAddItem (310,1);
        StoreAddItem (311,1);
        /* Silver dagger */
        StoreAddItem(20680,1);
        /* Silver mace */
        StoreAddItem(20681,1);
        /* Silver short sword */
        StoreAddItem(20683,1);
        /* Silver studded leather */
        StoreAddItem (21181, 1);
        StoreAddItem (21182, 1);
        StoreAddItem (21183, 1);
        StoreAddItem (21184, 1);
        StoreAddItem (21185, 1);
        StoreAddItem (21186, 1);
        StoreAddItem (21187, 1);
        StoreAddItem (21188, 1);
        StoreAddItem (21189, 1);
        StoreAddItem (21190, 1);
        StoreAddItem (21191, 1);

        case ADVENTURE_NONE:
        default:
        /* apple */
        StoreAddItem (19,1);
        /* Berries */
        StoreAddItem (243,1);
        /* Carrot */
        StoreAddItem (244,1);
        /* Cherry */
        StoreAddItem (245,1);
        /* Water gourd */
        StoreAddItem (246,1);
        /* Chicken */
        StoreAddItem (247,1);
        /* Grape */
        StoreAddItem (248,1);
        /* Banana */
        StoreAddItem (249,1);
        /* Pineapple */
        StoreAddItem (251,1);
        /* crossbow */
        StoreAddItem (28,1);
        /* Iron Dagger */
        StoreAddItem (200,1);
        /* Iron Mace */
        StoreAddItem (201,1);
        /* Iron Staff */
        StoreAddItem (202,1);
        /* Iron Short Sword */
        StoreAddItem (203,1);
        /* Iron Long Sword */
        StoreAddItem (204,1);
        /* Iron Axe */
        StoreAddItem (205,1);
        /* Dagger of Striking */
        StoreAddItem (210,1);
        /* Scroll of identify readied */
        StoreAddItem (350,1);
        /* Scroll of Health 1 */
        StoreAddItem (354,1);
        /* Scroll of Armor */
        StoreAddItem (358,1);
        /* Gain Mana Scroll #1 */
        StoreAddItem (362,1);
        /* Potion of Healing 1 */
        StoreAddItem (800,1);
        /* Potion of Armor 1 */
        StoreAddItem (808,1);
        /* Potion of Water Walking */
        StoreAddItem (812,1);
        /* Potion of Food */
        StoreAddItem (821,1);
        /* Potion of Water */
        StoreAddItem (822,1);
        /* Wand of Earthbind */
        StoreAddItem (911,1);
        /* Wand of Pushing */
        StoreAddItem (915,1);
        /* Wand of Pulling */
        StoreAddItem (916,1);
        /* Leather Breast */
        StoreAddItem (701,1);
        /* Leather Arm */
        StoreAddItem (702,1);
        /* Leather Leg */
        StoreAddItem (703,1);
        /* Mail Helm */
        StoreAddItem (704,1);
        /* Mail Chest */
        StoreAddItem (705,1);
        /* Mail Arm */
        StoreAddItem (706,1);
        /* Mail Leg */
        StoreAddItem (707,1);
        /* Plate Helm */
        StoreAddItem (708,1);
        /* Plate Chest */
        StoreAddItem (709,1);
        /* Plate Arm */
        StoreAddItem (710,1);
        /* Plate Leg */
        StoreAddItem (711,1);
        /* Quiver of normal bolts */
        StoreAddItem (144,1);
        /* Thieving tools */
        StoreAddItem (143,1);
        /* Mage rune 1 */
        StoreAddItem (300,1);
        /* Cleric rune 1 */
        StoreAddItem (309,1);
        break;

        case ADVENTURE_UNKNOWN:
        DebugCheck(0);
        break;
    }

    /* loop weapons */
/*    for (i=1;i<(3>adv?adv:3);i++)
    {
        for (j=0;j<6;j++)
        {
            item=firstWeapon+materialOffsets[i]+j;
            StoreAddItem(item,1);
        }
    }
*/

// multiplayer?? */
    /* Wand of Lock Door */
//  StoreAddItem (919,1);
    /* Wand of Dispell Magic 2 */
//  StoreAddItem (910,1);
    /* Quiver of mana drain bolts */
//  StoreAddItem (150,1);
    /* Gain Mana Drain Attack Scroll */
//  StoreAddItem (372,1);
    /* Wand of Mana Drain */
//  StoreAddItem (908,1);



    /* Leather Cap */
//  StoreAddItem (700,1);
    /* Wand of Dispell Magic 1 */
//    StoreAddItem (909,1);
    /* Potion of Poison */
//    StoreAddItem (825,1);
    /* Potion of Fire */
//    StoreAddItem (826,1);
    /* Potion of Hurt */
//    StoreAddItem (827,1);
    /* Gain Experience Scroll */
//  StoreAddItem (214,1);
    /* Short Sword of Piercing */
//  StoreAddItem (216,1);
    /* Long Sword of Piercing */
//  StoreAddItem (217,1);
    /* Dagger of Poison */
//  StoreAddItem (220,1);
    /* Short Sword of Poison */
//  StoreAddItem (221,1);
    /* Long Sword of Speed */
//  StoreAddItem (226,1);
    /* Mace of Poison */
//  StoreAddItem (231,1);
    /* Staff of Power */
//  StoreAddItem (233,1);
    /* Mace of Electricity */
//  StoreAddItem (234,1);

    /* loop armors */
/*    for (i=1;i<(3>adv?adv:3);i++)
    {
        for (j=1;j<12;j++)
        {
            item=firstArmor+materialOffsets[i]+j;
            StoreAddItem(item,1);
        }
    } */
    /* secondary armor loop - hardened armor has no 'light' armor */
/*    if (adv >= 3)
    {
        for (j=4;j<12;j++)
        {
            item=firstArmor+materialOffsets[3]+j;
    //        printf ("adding armor item:%d\n",item);
    //        fflush (stdout);
            StoreAddItem (item,1);
    //        printf ("done\n");
    //        fflush (stdout);
        }
    }
*/
    InventoryReorder(INVENTORY_STORE,TRUE);
    InventoryDrawInventoryWindow (INVENTORY_STORE);

    /* set up store control callback */
    StoreSetControlCallback (StoreUIUpdateGraphics);

    /* welcome user */
    MessageAdd ("^007Welcome to the store!");

    /* add journal help page if necessary */
    if (StatsPlayerHasNotePage(32)==FALSE &&
        StatsPlayerHasNotePage(0)==TRUE)
    {
        StatsAddPlayerNotePage(32);
    }
    else
    {
        /* force loading of inventory banner display */
        BannerOpenForm(BANNER_FORM_INVENTORY);
    }

    DebugEnd();
}


T_void StoreUIUpdate (T_void)
{
    DebugRoutine ("StoreUIUpdate");

    DebugEnd();
}


T_void StoreUIEnd    (T_void)
{
    T_word16 i;
    DebugRoutine ("StoreUIEnd");

    /* destroy ui objects */
    GraphicDelete (G_backgroundPic);
    for (i=0;i<4;i++)
    {
        TxtboxDelete(G_financeDisplays[i]);
    }

    /* close store inventory */
    StoreCloseInventory();

    DebugEnd();
}


static T_void StoreUIUpdateGraphics (T_void)
{
    T_byte8 stmp[32];
    T_word16 i;

    DebugRoutine ("StoreUIUpdateGraphics");

    for (i=0;i<4;i++)
    {
        sprintf (stmp,"%d",StatsGetPlayerCoins(3-i));
        TxtboxSetData(G_financeDisplays[i],stmp);
    }

    GraphicUpdateAllGraphics();

    DebugEnd();
}

T_void StoreSetHouseModeOn (T_void)
{
    DebugRoutine ("StoreSetHouseModeOn");
    G_houseMode=TRUE;
    DebugEnd();
}


E_Boolean StoreIsOpen(T_void)
{
    return (G_storeOpen);
}

E_Boolean StoreHouseModeIsOn(T_void)
{
    return (G_houseMode);
}


/* changes amount of copper into a display string */
/* ex: 15320 copper = 15p3g2s */
T_void StoreConvertCurrencyToString (T_byte8* tempString, T_word32 amount)
{
    T_word16 platinum,gold,silver,copper;
    T_byte8 stmp[32];
    DebugRoutine ("StoreConvertCurrencyToString");

    platinum=amount/1000;
    amount-=(platinum*1000);
    gold=amount/100;
    amount-=(gold*100);
    silver=amount/10;
    amount-=(silver*10);
    copper=amount;

    strcpy (tempString,"");
    if (platinum > 0)
    {
        sprintf (stmp,"^038%dp",platinum);
        strcat (tempString,stmp);
    }
    if (gold > 0)
    {
        sprintf (stmp,"^003%dg",gold);
        strcat (tempString,stmp);
    }
    if (silver > 0)
    {
        sprintf (stmp,"^001%ds",silver);
        strcat (tempString,stmp);
    }
    if (copper > 0)
    {
        sprintf (stmp,"^031%dc",copper);
        strcat (tempString,stmp);
    }
    strcat (tempString,"^007");

    DebugEnd();
}



T_word16 StoreGetBuyValue (T_inventoryItemStruct *p_inv)
{
    T_word16 buyValue=0;
    T_word16 objtype;
    DebugRoutine ("StoreGetBuyValue");

    if (StoreWillBuy(p_inv->itemdesc.type))
    {
        objtype=ObjectGetType(p_inv->object);
        if (objtype==60 ||
            objtype==117)
        {
            /* bag of gold */
            if (ObjectGetAccData(p_inv->object)!=0)
            {
                buyValue=ObjectGetAccData(p_inv->object)*100;
            }
            else
            {
                buyValue=1000;
            }
        }
        else if ((objtype>=110 &&
                 objtype<=113) ||
                 objtype==56)
        {
            /* gems */
            buyValue=ObjectGetValue(p_inv->object);
        }
        else if (objtype==412 ||
                 objtype==413 ||
                 objtype==414 ||
                 objtype==425 ||
                 objtype==433)
        {
            /* treasure rings */
            buyValue=ObjectGetValue(p_inv->object);
        }
        else if (objtype>=144 &&
                 objtype<=150)
        {
            /* quivers */
            if (ObjectGetAccData(p_inv->object)!=0)
            {
                buyValue=ObjectGetValue(p_inv->object)*
                     ObjectGetAccData(p_inv->object)/3;
            }
            else
            {
                buyValue=ObjectGetValue(p_inv->object)/3;
            }
        }
        else if (objtype >= 300 &&
                 objtype <= 317)
        {
            /* won't buy runes */
            buyValue=0;
        }
        else
        {
            buyValue=(ObjectGetValue(p_inv->object))/3;
        }
    }

    DebugEnd();
    return (buyValue);
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  STORE.C
 *-------------------------------------------------------------------------*/
