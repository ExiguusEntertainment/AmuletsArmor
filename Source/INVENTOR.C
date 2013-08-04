/*-------------------------------------------------------------------------*
 * File:  INVENTOR.C
 *-------------------------------------------------------------------------*/
/**
 * User Interface management and storing routines.
 *
 * @addtogroup INVENTORY
 * @brief Interface User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "BANNER.H"
#include "BUTTON.H"
#include "CLIENT.H"
#include "CONTROL.H"
#include "KEYSCAN.H"
#include "INVENTOR.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OVERLAY.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SPELLS.H"
#include "STATS.H"
#include "TXTBOX.H"

//static T_word16  G_inventoryWindowX1=213;
//static T_word16  G_inventoryWindowX2=315;
//static T_word16  G_inventoryWindowY1=16;
//static T_word16  G_inventoryWindowY2=129;

static T_word16  G_equipmentWindowX1=214;
static T_word16  G_equipmentWindowX2=314;
static T_word16  G_equipmentWindowY1=16;
static T_word16  G_equipmentWindowY2=85;

static T_buttonID G_lastStorePage=NULL;
static T_buttonID G_nextStorePage=NULL;
static T_TxtboxID G_storePageType=NULL;
static T_TxtboxID G_storePageNumber=NULL;
static E_Boolean  G_useResetNeeded=FALSE;

//static T_byte8   G_curinvpage=0;
static E_Boolean G_inventoryInventoryWindowOpen=FALSE;
static E_Boolean G_inventoryEquipmentWindowOpen=FALSE;
static E_inventoryType G_activeInventory=INVENTORY_PLAYER;
/* variables for area-overlap examination iteration (see InventoryFindOverlap) */
static T_byte8 G_x1,G_x2,G_y1,G_y2,G_page;
static T_doubleLinkListElement G_le;

/* Inventory structures for player / store (see inventor.h)  */
static T_inventoryStruct G_inventories[INVENTORY_UNKNOWN];
//static T_byte8 G_activeInventory = INVENTORY_PLAYER;
/* stored inventory items array */
//static T_doubleLinkList G_inventoryItemsList;

/* quick pointers to equipped item areas in G_inventories[INVENTORY_PLAYER]*/
static T_doubleLinkListElement G_inventoryLocations[EQUIP_NUMBER_OF_LOCATIONS];

/* coordinates for equipment 'boxes' i.e. amulets, rings, and other storage areas
/* for equipment pages*/
static T_word16 G_inventoryBoxCoordinates[EQUIP_NUMBER_OF_LOCATIONS][4];

static E_Boolean G_init = FALSE;

/* internal routines */
static E_Boolean InventoryDestroyElement (T_doubleLinkListElement killme);
static T_doubleLinkListElement InventoryIsItemInArea (E_inventoryType where, T_byte8 gx1, T_byte8 gx2, T_byte8 gy1, T_byte8 gy2, T_byte8 page);
static E_Boolean InventoryDrawElement (T_doubleLinkListElement element);
static E_Boolean InventoryFindOverlap (T_doubleLinkListElement element);
static T_void InventorySetPlayerColor (E_equipLocations location);
static E_Boolean InventoryDoItemEffect (T_inventoryItemStruct *p_inv, E_effectTriggerType trigger);

static T_void InventorySortByType(E_inventoryType which);
static T_void InventoryGroupItems (E_inventoryType whichInv);
static T_void InventoryClearCurrentLocations(E_inventoryType which);
static T_void InventorySetWindowSizeDefaults(T_void);
static T_doubleLinkListElement InventoryFindElement(E_inventoryType whichinv,
                                                    T_inventoryItemStruct *p_inv);
static T_void InventoryUpdateStoreItemTitlePage(T_void);
static T_void InventorySelectNextStoreInventoryPage (T_buttonID buttonID);
static T_void InventorySelectLastStoreInventoryPage (T_buttonID buttonID);

static T_void IInventoryWriteItem(
                  FILE *fp,
                  T_doubleLinkListElement element) ;

/* ----------------------------------------------------------------- */
static T_word16 G_armorToBodyPartTable[EQUIP_ARMOR_TYPE_UNKNOWN] =
{
    0,            /* EQUIP_ARMOR_TYPE_NONE */
    522,          /* EQUIP_ARMOR_TYPE_BRACING_CLOTH */
    532,          /* EQUIP_ARMOR_TYPE_BRACING_CHAIN */
    512,          /* EQUIP_ARMOR_TYPE_BRACING_PLATE */
    524,          /* EQUIP_ARMOR_TYPE_LEGGINGS_CLOTH */
    534,          /* EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN */
    514,          /* EQUIP_ARMOR_TYPE_LEGGINGS_PLATE */
    520,          /* EQUIP_ARMOR_TYPE_HELMET_CLOTH */
    530,          /* EQUIP_ARMOR_TYPE_HELMET_CHAIN */
    510,          /* EQUIP_ARMOR_TYPE_HELMET_PLATE */
    521,          /* EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH */
    531,          /* EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN */
    511,          /* EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE */
    526           /* EQUIP_ARMOR_TYPE_SHIELD */
} ;
/* ----------------------------------------------------------------- */
static T_word16 G_weaponToBodyPartTable[EQUIP_WEAPON_TYPE_UNKNOWN] =
{
    516,          /* EQUIP_WEAPON_TYPE_NONE */
    540,          /* EQUIP_WEAPON_TYPE_AXE */
    542,          /* EQUIP_WEAPON_TYPE_DAGGER */
    535,          /* EQUIP_WEAPON_TYPE_LONGSWORD */
    535,          /* EQUIP_WEAPON_TYPE_SHORTSWORD */
    543,          /* EQUIP_WEAPON_TYPE_MACE */
    544,          /* EQUIP_WEAPON_TYPE_STAFF */
    541,          /* EQUIP_WEAPON_TYPE_CROSSBOW */
    542,          /* EQUIP_WEAPON_TYPE_WAND */
} ;
/* ----------------------------------------------------------------- */
static T_word16 G_standardBodyParts[] =
{
    0,            /* EQUIP_LOCATION_MOUSE_HAND */
    515,          /* EQUIP_LOCATION_READY_HAND */
    520,          /* EQUIP_LOCATION_HEAD */
    523,          /* EQUIP_LOCATION_LEFT_ARM */
    522,          /* EQUIP_LOCATION_RIGHT_ARM */
    521,          /* EQUIP_LOCATION_CHEST */
    524,          /* EQUIP_LOCATION_LEGS */
    526           /* EQUIP_LOCATION_SHIELD_HAND */
} ;
/* ----------------------------------------------------------------- */

T_void InventoryInit (T_void)
{
	T_word16 i,j;

	DebugRoutine ("InventoryInit");
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    InventorySetWindowSizeDefaults();
    /* init player / store inventories */

/* LES: Remove following two lines because they were done in the above */
/*      call to SetWindowSizeDefaults() */
//    G_inventories[INVENTORY_PLAYER].itemslist=DoubleLinkListCreate();
//    G_inventories[INVENTORY_STORE].itemslist=DoubleLinkListCreate();


    /* set up initial link list for inventory items */
//    G_inventoryItemsList=DoubleLinkListCreate();
//    DebugCheck (G_inventoryItemsList!=DOUBLE_LINK_LIST_BAD);

    /* set 'quick' pointers to null */
    for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
    {
        G_inventoryLocations[i]=DOUBLE_LINK_LIST_ELEMENT_BAD;
    }

    /* set up coordinate box locations for equip page */
    for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
    {
        for (j=0;j<4;j++)
        {
            /* first init to max */
            G_inventoryBoxCoordinates[i][j]=0-1;
        }
    }

    G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1]=94;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1]=156;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X2]=129;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y2]=183;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_NECK][EQUIP_LOC_X1]=291;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_NECK][EQUIP_LOC_Y1]=20;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_NECK][EQUIP_LOC_X2]=306;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_NECK][EQUIP_LOC_Y2]=32;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_1][EQUIP_LOC_X1]=294;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_1][EQUIP_LOC_Y1]=36;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_1][EQUIP_LOC_X2]=304;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_1][EQUIP_LOC_Y2]=44;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_2][EQUIP_LOC_X1]=294;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_2][EQUIP_LOC_Y1]=48;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_2][EQUIP_LOC_X2]=304;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_2][EQUIP_LOC_Y2]=56;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_3][EQUIP_LOC_X1]=294;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_3][EQUIP_LOC_Y1]=60;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_3][EQUIP_LOC_X2]=304;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_3][EQUIP_LOC_Y2]=68;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_4][EQUIP_LOC_X1]=294;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_4][EQUIP_LOC_Y1]=72;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_4][EQUIP_LOC_X2]=304;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RING_4][EQUIP_LOC_Y2]=80;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_HEAD][EQUIP_LOC_X1]=242;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_HEAD][EQUIP_LOC_Y1]=19;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_HEAD][EQUIP_LOC_X2]=258;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_HEAD][EQUIP_LOC_Y2]=29;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_CHEST][EQUIP_LOC_X1]=244;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_CHEST][EQUIP_LOC_Y1]=29;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_CHEST][EQUIP_LOC_X2]=252;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_CHEST][EQUIP_LOC_Y2]=50;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEFT_ARM][EQUIP_LOC_X1]=230;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEFT_ARM][EQUIP_LOC_Y1]=28;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEFT_ARM][EQUIP_LOC_X2]=244;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEFT_ARM][EQUIP_LOC_Y2]=64;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RIGHT_ARM][EQUIP_LOC_X1]=253;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RIGHT_ARM][EQUIP_LOC_Y1]=28;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RIGHT_ARM][EQUIP_LOC_X2]=271;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_RIGHT_ARM][EQUIP_LOC_Y2]=53;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEGS][EQUIP_LOC_X1]=241;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEGS][EQUIP_LOC_Y1]=50;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEGS][EQUIP_LOC_X2]=258;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_LEGS][EQUIP_LOC_Y2]=82;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_SHIELD_HAND][EQUIP_LOC_X1]=258;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_SHIELD_HAND][EQUIP_LOC_Y1]=47;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_SHIELD_HAND][EQUIP_LOC_X2]=267;
    G_inventoryBoxCoordinates[EQUIP_LOCATION_SHIELD_HAND][EQUIP_LOC_Y2]=61;
	DebugEnd();
}


T_void InventoryClear (E_inventoryType which)
{
    T_word16 i;
    DebugRoutine ("InventoryClear");

    if (G_inventories[which].itemslist != DOUBLE_LINK_LIST_BAD)
    {
        DoubleLinkListTraverse (G_inventories[which].itemslist, InventoryDestroyElement);
    }

    if (which==INVENTORY_PLAYER)
    {
        /* clear out mouse pointer locations */
        for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++) G_inventoryLocations[i]=DOUBLE_LINK_LIST_ELEMENT_BAD;
    }

    DebugEnd();
}



/* routine cleans up all inventories for program exit */
T_void InventoryFinish (T_void)
{
    DebugRoutine ("InventoryFinish");
    DebugCheck(G_init == TRUE) ;

    /* traverse player / store inventories and destroy them */
    InventoryClear (INVENTORY_PLAYER);
    InventoryClear (INVENTORY_STORE);

    DoubleLinkListDestroy (G_inventories[INVENTORY_PLAYER].itemslist);
    DoubleLinkListDestroy (G_inventories[INVENTORY_STORE].itemslist);

    G_inventories[INVENTORY_PLAYER].itemslist=DOUBLE_LINK_LIST_BAD;
    G_inventories[INVENTORY_STORE].itemslist=DOUBLE_LINK_LIST_BAD;

    G_init = FALSE ;

    DebugEnd();
}


static E_Boolean InventoryDestroyElement (T_doubleLinkListElement killme)
{
    T_inventoryItemStruct *p_inv;
    T_word16 i;
    DebugRoutine ("InventoryDestroyElement");

    /* get element data */
    p_inv=(T_inventoryItemStruct *)DoubleLinkListRemoveElement (killme);

    /* check to see if this item is in an 'equip' location */
    for (i=0;i<EQUIP_LOCATION_UNKNOWN;i++)
    {
        if (G_inventoryLocations[i]==killme) G_inventoryLocations[i]=DOUBLE_LINK_LIST_ELEMENT_BAD;
    }

    /* get rid of it */
    if (p_inv->object != NULL)
    {
        ObjectDestroy (p_inv->object);
        p_inv->object=NULL;
    }

    /* kill element data */
    MemFree (p_inv);

    killme=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugEnd();

    return (TRUE);
}


T_void InventorySelectNextInventoryPage(E_inventoryType which)
{
    DebugRoutine ("InventorySelectNextInventoryPage");

    if (G_inventories[which].curpage+1 < G_inventories[which].maxpages)
    {
        G_inventories[which].curpage++;
        InventoryDrawInventoryWindow(which);
        if (which==INVENTORY_STORE) InventoryUpdateStoreItemTitlePage();
    }
    DebugEnd();
}


T_void InventorySelectLastInventoryPage(E_inventoryType which)
{
    DebugRoutine ("InventorySelectLastInventoryPage");
    if (G_inventories[which].curpage > 0)
    {
        G_inventories[which].curpage--;
        InventoryDrawInventoryWindow(which);
        if (which==INVENTORY_STORE) InventoryUpdateStoreItemTitlePage();
    }
    DebugEnd();
}

T_void InventoryFindInventoryItemPage (E_inventoryType which, T_inventoryItemStruct *thisitem)
{
    T_inventoryItemStruct *p_inv;
    T_doubleLinkListElement element;
    DebugRoutine ("InventoryFindInventoryItemPage");
    DebugCheck (thisitem != NULL);

    /* iterate through inventory list looking for first occurance of this item */
    element=DoubleLinkListGetFirst(G_inventories[which].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv==thisitem)
        {
            /* set page number and break */
            G_inventories[which].curpage=p_inv->storepage;
            InventoryDrawInventoryWindow(which);
            if (which==INVENTORY_STORE) InventoryUpdateStoreItemTitlePage();
            break;
        }
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
}

E_Boolean InventoryInventoryWindowIsAt (T_word16 locx, T_word16 locy)
{
    E_Boolean retvalue=FALSE;
    DebugRoutine ("InventoryWindowIsAt");

    if (InventoryFindInventoryWindow(locx,locy)!=INVENTORY_UNKNOWN) retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}

E_inventoryType InventoryFindInventoryWindow (T_word16 locx, T_word16 locy)
{
    E_inventoryType retvalue=INVENTORY_UNKNOWN;
    T_word16 i;

    DebugRoutine ("InventorFindInventoryWindow");

    for (i=INVENTORY_PLAYER;i<INVENTORY_UNKNOWN;i++)
    {
        if (locx>=G_inventories[i].locx1 &&
            locx<=G_inventories[i].locx2 &&
            locy>=G_inventories[i].locy1 &&
            locy<=G_inventories[i].locy2)
        {
            retvalue=i;
            break;
        }
    }

    if (i==INVENTORY_PLAYER)
    {
        /* check to make sure banenr is open */
        if (BannerFormIsOpen(BANNER_FORM_INVENTORY)!=TRUE) retvalue=INVENTORY_UNKNOWN;
    }

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryReadyBoxIsAt (T_word16 locx, T_word16 locy)
{
    E_Boolean retvalue=FALSE;

    DebugRoutine ("InventoryReadyBoxIsAt");
    if (locx >= G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1] &&
        locy >= G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1] &&
        locx <= G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X2] &&
        locy <= G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y2])
        retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryEquipmentWindowIsAt (T_word16 locx, T_word16 locy)
{
    E_Boolean retvalue=FALSE;

    DebugRoutine ("InventoryEquipmentWindowIsAt");

    if (locx>=G_equipmentWindowX1 &&
        locx<=G_equipmentWindowX2 &&
        locy>=G_equipmentWindowY1 &&
        locy<=G_equipmentWindowY2 &&
        BannerFormIsOpen(BANNER_FORM_EQUIPMENT)) retvalue=TRUE;

    DebugEnd();

    return (retvalue);
}


E_Boolean InventoryObjectIsInMouseHand (T_void)
{
    E_Boolean retvalue=FALSE;

    DebugRoutine ("InventoryObjectIsInMouseHand");

    if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
      retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryObjectIsInReadyHand (T_void)
{
    E_Boolean retvalue=FALSE;

    DebugRoutine ("InventoryObjectIsInReadyHand");

    if (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
      retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryCanUseItemInReadyHand (T_void)
{
    E_Boolean canuse=FALSE;
    DebugRoutine ("InventoryCanUseItemInReadyHand");

    if (OverlayIsDone() && G_useResetNeeded==FALSE) canuse=TRUE;

    DebugEnd();
    return (canuse);
}


/* attempts to 'quick use' specified item from player inventory */
E_Boolean InventoryCanUseItem (T_inventoryItemStruct *p_inv)
{
    E_Boolean canUse=FALSE;
    DebugRoutine ("InventoryCanUseItem");

    if (p_inv != NULL &&
       (p_inv->itemdesc.type == EQUIP_OBJECT_TYPE_POTION ||
        p_inv->itemdesc.type == EQUIP_OBJECT_TYPE_SCROLL ||
        p_inv->itemdesc.type == EQUIP_OBJECT_TYPE_FOODSTUFF ||
        p_inv->itemdesc.type == EQUIP_OBJECT_TYPE_THING)) canUse=TRUE;
    DebugEnd();
    return (canUse);
}


/* calls use effect for item currently in ready area */
/* buttonID = buttonID of caller */
/* function is initiated by the 'use' button in the main banner area */
T_void InventoryUseItemInReadyHand (T_buttonID buttonID)
{
    T_inventoryItemStruct *p_inv=NULL;
    E_Boolean isweapon=FALSE;
    E_Boolean ispunch=FALSE;
    E_Boolean isobject=FALSE;
    T_byte8 bolttype;
    DebugRoutine ("InventoryUseItemInReadyHand");

    if ((!ClientIsPaused()) && (!ClientIsDead()))  {
        if (InventoryObjectIsInReadyHand())
        {
            /* get pointer to this object */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
              (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);

            DebugCheck (p_inv != NULL);

            /* see if it's a weapon */
            if (ObjectIsWeapon(p_inv->object)) {
                /* A crossbow is only a weapon if it has ammo. */
                if (ObjectGetBasicType(p_inv->object) == OBJECT_TYPE_XBOW)  {
                    bolttype=BannerGetSelectedAmmoType();
                    /* check to see if we have any bolts to fire */
                    if (StatsGetPlayerBolts(bolttype)>0)  {
                        isweapon = TRUE ;
                    } else {
                        isweapon = FALSE ;
                        SoundDing() ;
                        MessageAdd("You are out of ammo!") ;
                        /* Don't allow repeat keying */
                        G_useResetNeeded=TRUE;
                    }
                } else {
                    isweapon=TRUE;
                }
            } else {
                isobject=TRUE;
            }
        } else {
            ispunch=TRUE;
        }

        /* if we've a weapon or fist, start the overlay animation */
        if ((isweapon || ispunch) &&
             ClientIsInView())
        {
            /* make sure there's not an animation alreay in progress */
            if (OverlayIsDone() && G_useResetNeeded==FALSE)
            {
                /* create the effect use */
                InventoryDoEffect  (EFFECT_TRIGGER_USE, EQUIP_LOCATION_READY_HAND);

                /* initiate a new attack */
                PlayerSetStance(STANCE_ATTACK);
                ClientAttack();
                OverlayAnimate (StatsGetPlayerAttackSpeed());
                if (isweapon==TRUE)
                {
                    /* weapon attack */
                    InventoryPlayWeaponAttackSound();
                }
                else
                {
                    /* fist attack */
                    SoundPlayByNumber (SOUND_SWING_SET4+(rand()%3),200);
                }
            }
        }
        else if (isobject)
        {
            /* create the effect use */
            InventoryDoEffect  (EFFECT_TRIGGER_USE, EQUIP_LOCATION_READY_HAND);

            /* check for a destroy on use setting */
            if (InventoryDestroyOn (EFFECT_TRIGGER_USE,EQUIP_LOCATION_READY_HAND))
              InventoryDrawReadyArea();

            G_useResetNeeded=TRUE;
        }
    }

    DebugEnd();
}



/* attempts to 'quick use' specified item from player inventory */
T_void InventoryUseItem (T_inventoryItemStruct *p_inv)
{
    T_word16 i;
    T_word16 effecttype;
    DebugRoutine ("InventoryUseItem");
    DebugCheck (p_inv != NULL);

    if ((!ClientIsPaused()) && (!ClientIsDead()))  {
        /* create the effect use */
        /* scan through triggers to see if we find a match */
        for (i=0;i<MAX_ITEM_EFFECTS;i++)
        {
            if (p_inv->itemdesc.effectTriggerOn[i]==EFFECT_TRIGGER_USE)
            {
                /* we triggered an effect */
                Effect(p_inv->itemdesc.effectType[i],
                       EFFECT_TRIGGER_USE,
                       p_inv->itemdesc.effectData[i][0],
                       p_inv->itemdesc.effectData[i][1],
                       p_inv->itemdesc.effectData[i][2],
                       (T_void *)ObjectGetType(p_inv->object));

               /* should we identify ? */
               effecttype=p_inv->itemdesc.effectType[i];
               if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_POTION ||
                   p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_SCROLL ||
                   p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_FOODSTUFF ||
                   p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_THING)
               {
                    if ((effecttype != EFFECT_DISPLAY_CANNED_MESSAGE) &&
                        (effecttype != EFFECT_PLAY_LOCAL_SOUND) &&
                        (effecttype != EFFECT_PLAY_AREA_SOUND) &&
                        (effecttype != EFFECT_COLOR_FLASH))
                    {
//                        if ((rand()%100) < StatsGetPlayerAttribute(ATTRIBUTE_MAGIC))
//                        {
                            /* id this object */
                            StatsPlayerIdentify(ObjectGetType(p_inv->object));
    //                        MessageAdd ("Identified\n");
//                        }
                    }
               }
            }
        }

        /* check for a destroy on use setting */
        if (p_inv->itemdesc.objectDestroyOn==EFFECT_TRIGGER_USE)
        {
            /* we triggered a destroy object call.  remove the object */
            /* first, remove the weight */
            /* unless it's a coin or an ammo */
            if ((p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_COIN) &&
                (p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_BOLT))
            {
                StatsChangePlayerLoad (-ObjectGetWeight (p_inv->object));
            }
            if (p_inv->numitems > 1)
            {
                p_inv->numitems--;
            }
            else InventoryDestroyElement (p_inv->elementID);
            InventoryDrawInventoryWindow(INVENTORY_PLAYER);
        }
    }

    DebugEnd();
}


T_3dObject* InventoryCheckObjectInMouseHand (T_void)
{
    T_3dObject *retvalue=NULL;
    T_inventoryItemStruct *p_inv=NULL;

    DebugRoutine ("InventoryCheckObjectInMouseHand");

    /* do we have an object in our 'mouse hand'? */
    if (InventoryObjectIsInMouseHand())
    {
        p_inv=(T_inventoryItemStruct *)
          DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
        DebugCheck (p_inv != NULL);

        retvalue=p_inv->object;
    }

    DebugEnd();
    return (retvalue);
}


T_inventoryItemStruct* InventoryCheckItemInMouseHand (T_void)
{
    T_inventoryItemStruct *retvalue=NULL;
    T_inventoryItemStruct *p_inv=NULL;

    DebugRoutine ("InventoryCheckItemInMouseHand");

    /* do we have an object in our 'mouse hand'? */
    if (InventoryObjectIsInMouseHand())
    {
        p_inv=(T_inventoryItemStruct *)
          DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
        DebugCheck (p_inv != NULL);

        retvalue=p_inv;
    }

    DebugEnd();
    return (retvalue);
}


T_void InventoryDestroyItemInMouseHand (T_void)
{
    T_3dObject *p_obj;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroyItemInMouseHand");
    DebugCheck (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD);
    DebugCheck (InventoryObjectIsInMouseHand()==TRUE);

    p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
    p_obj=p_inv->object;

    /* if there is a 'drop' effect do it now */
    InventoryDoEffect(EFFECT_TRIGGER_DROP,EQUIP_LOCATION_MOUSE_HAND);

    /* change the player's load */
    StatsChangePlayerLoad (-ObjectGetWeight(p_obj));

    /* destroy the element */
    InventoryDestroyElement (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

    ControlSetDefaultPointer (CONTROL_MOUSE_POINTER_DEFAULT);
    MouseUseDefaultBitmap();

    DebugEnd();
}


T_void InventoryClearObjectInMouseHand (T_void)
{
    DebugRoutine ("inventoryClearObjectInMouseHand");
    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugEnd();
}

T_void InventorySetObjectInMouseHand (T_doubleLinkListElement element)
{
    DebugRoutine ("InventorySetObjectInMouseHand");

    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=element;

    DebugEnd();
}



T_3dObject* InventoryCheckObjectInReadyHand (T_void)
{
    T_3dObject *retvalue=NULL;
    T_inventoryItemStruct *p_inv=NULL;

    DebugRoutine ("InventoryCheckObjectInReadyHand");

    /* do we have an object in the 'ready hand' */
    if (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /*yes, return pointer to T_3dobject for it */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);
        retvalue=p_inv->object;
    }

    DebugEnd();
    return (retvalue);
}


T_inventoryItemStruct* InventoryCheckItemInReadyHand (T_void)
{
    T_inventoryItemStruct *retvalue=NULL;

    DebugRoutine ("InventoryCheckObjectInReadyHand");

    /* do we have an object in the 'ready hand' */
    if (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /*yes, return pointer to T_3dobject for it */
        retvalue=(T_inventoryItemStruct *)DoubleLinkListElementGetData (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);
    }

    DebugEnd();
    return (retvalue);
}


T_3dObject* InventoryCheckObjectInInventoryArea(T_word16 x, T_word16 y)
{
    T_3dObject *retvalue=NULL;
    T_inventoryItemStruct *p_inv;
    T_doubleLinkListElement temp;
    T_byte8 gsx,gsy;
    E_inventoryType invWin;
    DebugRoutine ("InventoryCheckObjectInInventoryArea");

    /* is the inventory window open ? */
    invWin=InventoryFindInventoryWindow(x,y);
    if (invWin != INVENTORY_UNKNOWN)
    {
        /* do we have an object in the inventory window area specified by x,y? */
        gsx=((x-G_inventories[invWin].locx1)/INVENTORY_GRID_WIDTH);
        gsy=((y-G_inventories[invWin].locy1)/INVENTORY_GRID_HEIGHT);
        temp=InventoryIsItemInArea(invWin,gsx,gsx+1,gsy,gsy+1,G_inventories[invWin].curpage);

        if (temp != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* yes, there is one here */
            /* get a pointer to it's structure and return the 3d object */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(temp);
            retvalue=p_inv->object;
        }
    }

    DebugEnd();
    return (retvalue);
}

/* same as above routine but returns the full inventory item structure if found */
T_inventoryItemStruct* InventoryCheckItemInInventoryArea(T_word16 x, T_word16 y)
{
    T_inventoryItemStruct *retvalue=NULL;
    T_inventoryItemStruct *p_inv;
    T_doubleLinkListElement temp;
    T_byte8 gsx,gsy;
    E_inventoryType invWin;
    DebugRoutine ("InventoryCheckItemInInventoryArea");
    fflush (stdout);

    /* is the inventory window open ? */
    invWin=InventoryFindInventoryWindow(x,y);
    if (invWin != INVENTORY_UNKNOWN)
    {
        /* do we have an object in the inventory window area specified by x,y? */
        gsx=((x-G_inventories[invWin].locx1)/INVENTORY_GRID_WIDTH);
        gsy=((y-G_inventories[invWin].locy1)/INVENTORY_GRID_HEIGHT);
        temp=InventoryIsItemInArea(invWin,gsx,gsx+1,gsy,gsy+1,G_inventories[invWin].curpage);

        if (temp != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* yes, there is one here */
            /* get a pointer to it's structure and return the 3d object */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(temp);
            retvalue=p_inv;
        }
    }

    DebugEnd();
    return (retvalue);
}



T_3dObject* InventoryCheckObjectInEquipmentArea(T_word16 x, T_word16 y)
{
    T_3dObject *retvalue=NULL;
    T_inventoryItemStruct *p_inv;
    T_word16 i;

    DebugRoutine ("InventoryCheckObjectInEquipmentArea");
    /* find location pointed at */
    for (i=EQUIP_LOCATION_HEAD;i<EQUIP_LOCATION_UNKNOWN;i++)
    {
        /* check to see if were in the coords for this area */
        if (x >= G_inventoryBoxCoordinates[i][EQUIP_LOC_X1] &&
            x <= G_inventoryBoxCoordinates[i][EQUIP_LOC_X2] &&
            y >= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1] &&
            y <= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y2])
        {
            /* we'ere here... is there something in this location? */
            if (G_inventoryLocations[i]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
            {
                /* something is here, return it's object */
                p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
                    G_inventoryLocations[i]);

                retvalue=p_inv->object;

                /* break from loop */
                break;
            }
        }
    }

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryCanTakeItem (T_3dObject *item)
{
    E_Boolean retvalue=FALSE;
    T_word16 objtype;
    T_byte8 stmp[64];

    DebugRoutine ("InventoryCanTakeItem");

    if (item != NULL)
    {
        /* get type of object */
        objtype=ObjectGetType (item);
        sprintf (stmp,"INVDESC/DES%05d",objtype);

        /* now, see if we have a resource file for it */
        if (PictureExist(stmp))
        {
            retvalue=TRUE;
        }
        else
        {
            sprintf (stmp,"OBJ ID file not found for obj ID=%05d",objtype);
            MessageAdd (stmp);
        }
    }
    DebugEnd();
    return (retvalue);
}



E_Boolean InventoryTakeItemFromWorld (T_3dObject *item, E_Boolean autoStore)
{
    T_inventoryItemStruct *p_inv;
    E_Boolean retvalue=FALSE;

    DebugRoutine ("InventoryTakeItemFromWorld");

    if (ClientIsPaused())  {
        MessageAdd("Cannot take item, game is paused") ;
    } else if (ClientIsDead())  {
        MessageAdd("Cannot take item, you are dead") ;
    } else {
        /* valid item? and nothing in mouse hand?*/
        if ((item != NULL) && (InventoryObjectIsInMouseHand() == FALSE))
        {
            /* remove from world */
            /* NOTE:  ObjectRemove MUST be called before */
            /*        ObjectSetServerId so that the object */
            /*        is removed from the correct hash table */
            /*        which is based on the object's ID */
            ObjectRemove (item);

            /* Make sure the item has no ID since it is no longer in the */
            /* world. */
            ObjectSetServerId(item, 0) ;

            p_inv=InventoryTakeObject (INVENTORY_PLAYER,item);

            /* set 'quick pointer' for mouse hand */
            G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=p_inv->elementID;
            /* force mouse hand picture update */
            ControlSetObjectPointer (p_inv->object);

            if (InventoryObjectIsInMouseHand()==TRUE)
            {
                /* item successfully taken */

                /* check for 'get effect' */
                InventoryDoEffect(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);

                /* check for 'destroy on get' indicator */
                if (InventoryShouldDestroyOn(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND)==TRUE)
                {
                    /* reset the mouse pointer to default */
                    ControlPointerReset();
                    /* destroy object */
                    InventoryDestroyOn (EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);
                    /* tell client.c to do nothing else */
                    retvalue=FALSE;
                }
                else
                {
                    /* set success flag */
                    retvalue=TRUE;

                    /* try to put into inventory on held shift */
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_SHIFT)==TRUE)
                    {
                        InventoryAutoStoreItemInMouseHand();
                    }
                    else if (autoStore)
                    {
                        if (InventoryAutoEquipItemInMouseHand()==FALSE)
                        {
                            /* failed to equip, just store */
                            InventoryAutoStoreItemInMouseHand();
                        }
                    }
                }

                if (BannerFormIsOpen (BANNER_FORM_INVENTORY)) InventoryDrawInventoryWindow(INVENTORY_PLAYER);

            }
        }
    }

    DebugEnd();
    return (retvalue);
}


T_inventoryItemStruct* InventoryTakeObject (E_inventoryType which, T_3dObject *item)
{
    T_word16 objtype;
    T_byte8 *desc1;
    T_resource res;
    E_Boolean destroyme=FALSE;
    E_spellsSpellSystemType spellsys;
    T_word32 size;
    E_statsClassType ourclass;
    T_inventoryItemStruct *p_inv=NULL;
    T_byte8 stmp[32];
    T_word16 weight;

    DebugRoutine ("InventoryTakeObject");

//    DebugCheck (InventoryObjectIsInMouseHand()==FALSE);
    DebugCheck (item != NULL);

    if (ClientIsPaused())  {
        MessageAdd("Cannot take item, game is paused") ;
    } else if (ClientIsDead())  {
        MessageAdd("Cannot take item, you are dead.") ;
    } else {
        /* get item type */
        objtype=ObjectGetType (item);

        /* check internal resource for description of item */
        sprintf (stmp,"INVDESC/DES%05d",objtype);
        DebugCheck (PictureExist(stmp));
        if (PictureExist(stmp))
        {
            /* create new object of this type in inventory struct */
            size=sizeof(T_inventoryItemStruct);
            p_inv=(T_inventoryItemStruct *)MemAlloc(size);

            /* init structure info */
            p_inv->locx=0;
            p_inv->locy=0;
            p_inv->picwidth=ObjectGetPictureWidth(item);
            p_inv->picheight=ObjectGetPictureHeight(item);
            p_inv->gridstartx=0;
            p_inv->gridstarty=0;
            p_inv->gridspacesx=((p_inv->picwidth+INVENTORY_GRID_WIDTH)/
                                (INVENTORY_GRID_WIDTH+1));
            p_inv->gridspacesy=((p_inv->picheight+INVENTORY_GRID_HEIGHT)/
                                (INVENTORY_GRID_HEIGHT+1));
            p_inv->object=item;
            p_inv->p_bitmap=(T_bitmap *)ObjectGetBitmap(item);
            p_inv->objecttype=ObjectGetType(item);
            p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
            p_inv->numitems=1;

            /* get description text */
            desc1=PictureLockData (stmp,&res);
            p_inv->itemdesc=*((T_equipItemDescription *)desc1);

            if (which==INVENTORY_PLAYER)
            {
    //            ObjectPrint(stdout,item);
                /* add object weight to load */
                weight=ObjectGetWeight(item);
                StatsChangePlayerLoad(weight);
            }
            else
            {
                /* check to see if the player can use this item, else */
                /* don't add it to the store */
                spellsys=StatsGetPlayerSpellSystem();
                destroyme=FALSE;
                ourclass=StatsGetPlayerClassType();
                if (InventoryIsUseableByClass(p_inv)==FALSE) destroyme=TRUE;
                if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON)
                {
                    if (InventoryCheckClassCanUseWeapon(p_inv,FALSE)==FALSE)
                    {
                        destroyme=TRUE;
                    }
                }
                else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_ARMOR)
                {
                    if (InventoryCheckClassCanUseArmor(p_inv,FALSE)==FALSE)
                    {
                        destroyme=TRUE;
                    }
                }
                else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_THING &&
                         p_inv->itemdesc.subtype==THING_TYPE_RUNE)
                {
                    if (spellsys==SPELL_SYSTEM_CLERIC)
                    {
                        if (ObjectGetType(p_inv->object)<309) destroyme=TRUE;
                    }
                    else if (spellsys==SPELL_SYSTEM_MAGE)
                    {
                        if (ObjectGetType(p_inv->object)>308) destroyme=TRUE;
                    }
                    else
                    {
                        if (ObjectGetType(p_inv->object)>303 &&
                            ObjectGetType(p_inv->object)<309) destroyme=TRUE;
                        if (ObjectGetType(p_inv->object)>313) destroyme=TRUE;
                    }
                }
                else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_BOLT ||
                         p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_QUIVER)
                {
                    if (ourclass==CLASS_MAGE ||
                        ourclass==CLASS_PRIEST ||
                        ourclass==CLASS_PALADIN ||
                        ourclass==CLASS_WARLOCK ||
                        ourclass==CLASS_MAGICIAN) destroyme=TRUE;
                }

                if (destroyme==TRUE)
                {
                    /* destroy this object */
                    ObjectDestroy(item);
                    MemFree(p_inv);
                    p_inv=NULL;
                }
            }

            if (p_inv != NULL)
            {
                /* add structure to linked list */
                p_inv->elementID=DoubleLinkListAddElementAtEnd(G_inventories[which].itemslist,p_inv);

                DebugCheck (p_inv->elementID != DOUBLE_LINK_LIST_ELEMENT_BAD);
            }

            PictureUnlockAndUnfind(res) ;
        }
    }
    DebugEnd();
    return (p_inv);
}


/* routine attempts to transfer item in mouse hand to inventory */
T_void InventoryAutoStoreItemInMouseHand (T_void)
{
    T_inventoryItemStruct *p_inv=NULL;
    T_inventoryItemStruct *temp=NULL;
    T_sword16 i,j;
    T_doubleLinkListElement element;
    T_byte8 pageno;
    E_Boolean success=FALSE;

    DebugRoutine ("InventoryAutoStoreItemInMouseHand");
    DebugCheck (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD);

    if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

        /* add item to player inventory */
        if (p_inv != NULL)
        {
            /* storage call successful, auto-place item in inventory somewhere */

            /* iterate through all items in inventory and look for a match */
            /* for this type */
            element=DoubleLinkListGetFirst (G_inventories[INVENTORY_PLAYER].itemslist);
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
            {
                temp=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
                if ((temp != p_inv) &&
                    ObjectGetType(p_inv->object) == ObjectGetType(temp->object))
                {
                    /* matched object types, see if we can stack it */
                    if (temp->itemdesc.numstackable > temp->numitems)
                    {
                        /* just increment one of these, destroy the */
                        /* item in the mouse hand, and then leave */
                        temp->numitems++;

                        /* if there is a 'get' effect do it now */
                        InventoryDoEffect(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);

                        /* change the player's load */
//                      StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));

                        /* remove from world */
//                      ObjectRemove (p_inv->object);

                        /* destroy the element */
                        InventoryDestroyElement (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

                        /* remove mouse hand graphic */
                        ControlPointerReset();
                        /* Inform the user of storage */
                        MessageAdd ("^007Item put in inventory");

                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                        if (BannerFormIsOpen(BANNER_FORM_INVENTORY))
                        {
                            InventoryDrawInventoryWindow(INVENTORY_PLAYER);
                        }

                        BannerStatusBarUpdate();

                        DebugEnd();
                        return;
                    }
                }
                element=DoubleLinkListElementGetNext(element);
            }
            /* failed to find a match for this item, */
            /* find storage space size needed */

            /* calculate number of grid squares it would cover */
            p_inv->gridspacesx=((p_inv->picwidth+INVENTORY_GRID_WIDTH)/
                                (INVENTORY_GRID_WIDTH+1));
            p_inv->gridspacesy=((p_inv->picheight+INVENTORY_GRID_HEIGHT)/
                                (INVENTORY_GRID_HEIGHT+1));

            /* loop through inventory spaces on this page and look for a fit */
            for (pageno=0;pageno<G_inventories[INVENTORY_PLAYER].maxpages;pageno++)
            {
                /* start at the bottom right corner of this window and look */
                /* backwards */
                for (i=G_inventories[INVENTORY_PLAYER].gridspacesx - p_inv->gridspacesx;
                     i>=0;
                     i--)
                {
                    for (j=G_inventories[INVENTORY_PLAYER].gridspacesy - p_inv->gridspacesy;
                         j>=0;
                         j--)
                    {
                        temp=NULL;
                        /*  Check for an object already at this location */
                        temp=InventoryIsItemInArea(INVENTORY_PLAYER,
                                                   (T_byte8)i,
                                                   i+p_inv->gridspacesx,
                                                   (T_byte8)j,
                                                   j+p_inv->gridspacesy,
                                                   pageno);
                        if (temp==NULL)
                        {
                            /* item will fit in this location, save and exit */
                            p_inv->gridstartx=(T_sbyte8)i;
                            p_inv->gridstarty=(T_sbyte8)j;
			                p_inv->locx=p_inv->gridstartx*INVENTORY_GRID_WIDTH+G_inventories[INVENTORY_PLAYER].locx1+1;
					        p_inv->locy=p_inv->gridstarty*INVENTORY_GRID_HEIGHT+G_inventories[INVENTORY_PLAYER].locy1+1;
                            p_inv->storepage=pageno;
                            p_inv->numitems=1;
                            success=TRUE;

                            /* remove mouse hand graphic */
                            ControlPointerReset();

                            /* Inform the user of storage */
                            MessageAdd ("^007Item put in inventory");

                            /* if there is a 'get' effect do it now */
                            InventoryDoEffect(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);

                            /* Remove from 'mouse hand */
                            G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                            /* redraw if necessary */
                            if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
                            {
                                InventoryDrawInventoryWindow(INVENTORY_PLAYER);
                            }

                            BannerStatusBarUpdate();
                            DebugEnd();
                            return;
                        }
                    }
                }
            }
        }
    }

    DebugEnd();
}

/* routine attempts to equip item in mouse hand */
E_Boolean InventoryAutoEquipItemInMouseHand (T_void)
{
    T_inventoryItemStruct *p_inv;
    E_Boolean wasEquipped=FALSE;
    T_word16 i;
    E_equipLocations location=EQUIP_LOCATION_UNKNOWN;


    DebugRoutine ("InventoryAutoEquipItemInMouseHand");
    DebugCheck (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD);


    if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get item info */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

        /* make sure we can use this item */

        if (InventoryIsUseableByClass(p_inv))
        {
            /* figure out what type of item we have */
            switch (p_inv->itemdesc.type)
            {
                case EQUIP_OBJECT_TYPE_WEAPON:
                if (InventoryObjectIsInReadyHand()==FALSE)
                {
                    if (InventoryCheckClassCanUseWeapon(p_inv,FALSE)==TRUE)
                    {
                        /* go ahead and ready this weapon */
                        location=EQUIP_LOCATION_READY_HAND;
                        MessageAdd ("^007Weapon readied");
                        wasEquipped=TRUE;
                    }
                }
                break;

                case EQUIP_OBJECT_TYPE_ARMOR:
                if (InventoryCheckClassCanUseArmor (p_inv,FALSE)==TRUE)
                {
                    /* we can use this item, go ahead and see if */
                    /* there's a slot to equip it in */
                    switch (p_inv->itemdesc.subtype)
                    {
                        case EQUIP_ARMOR_TYPE_BRACING_CLOTH:
                        case EQUIP_ARMOR_TYPE_BRACING_CHAIN:
                        case EQUIP_ARMOR_TYPE_BRACING_PLATE:
                        if (G_inventoryLocations[EQUIP_LOCATION_LEFT_ARM]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                        {
                            location=EQUIP_LOCATION_LEFT_ARM;
                            wasEquipped=TRUE;
                            MessageAdd ("^007Bracing worn");
                        }
                        else if (G_inventoryLocations[EQUIP_LOCATION_RIGHT_ARM]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                        {
                            location=EQUIP_LOCATION_RIGHT_ARM;
                            wasEquipped=TRUE;
                            MessageAdd ("^007Bracing worn");
                        }
                        break;

                        case EQUIP_ARMOR_TYPE_LEGGINGS_CLOTH:
                        case EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN:
                        case EQUIP_ARMOR_TYPE_LEGGINGS_PLATE:
                        if (G_inventoryLocations[EQUIP_LOCATION_LEGS]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                        {
                            location=EQUIP_LOCATION_LEGS;
                            wasEquipped=TRUE;
                            MessageAdd ("^007Leggings worn");
                        }
                        break;

                        case EQUIP_ARMOR_TYPE_HELMET_CHAIN:
                        case EQUIP_ARMOR_TYPE_HELMET_PLATE:
                        if (G_inventoryLocations[EQUIP_LOCATION_HEAD]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                        {
                            location=EQUIP_LOCATION_HEAD;
                            wasEquipped=TRUE;
                            MessageAdd("^007Helm worn");
                        }
                        break;

                        case EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH:
                        case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                        case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                        if (G_inventoryLocations[EQUIP_LOCATION_CHEST]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                        {
                            location=EQUIP_LOCATION_CHEST;
                            wasEquipped=TRUE;
                            MessageAdd("^007Breastplate worn");
                        }
                        break;

                        case EQUIP_ARMOR_TYPE_SHIELD:
                        case EQUIP_ARMOR_TYPE_HELMET_CLOTH:
                        case EQUIP_ARMOR_TYPE_NONE:
                        case EQUIP_ARMOR_TYPE_UNKNOWN:
                        default:
                        /* Fail! currently illegal armor types */
                        DebugCheck(0);
                        break;
                    }
                }
                break;

                case EQUIP_OBJECT_TYPE_AMULET:
                if (G_inventoryLocations[EQUIP_LOCATION_NECK]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                {
                    /* stick the amulet here */
                    location=EQUIP_LOCATION_NECK;
                    wasEquipped=TRUE;
                    MessageAdd("^007Amulet worn");
                }
                break;

                case EQUIP_OBJECT_TYPE_RING:
                for (i=EQUIP_LOCATION_RING_1;i<=EQUIP_LOCATION_RING_4;i++)
                {
                    if (G_inventoryLocations[i]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                    {
                        /* put the ring here */
                        location=i;
                        wasEquipped=TRUE;
                        MessageAdd("Ring worn");
                        break;
                    }
                }
                break;

                default:
                break;
            }
        }

        if (wasEquipped==TRUE)
        {
            DebugCheck (location != EQUIP_LOCATION_UNKNOWN);
            /* move item */
            G_inventoryLocations[location]=p_inv->elementID;

            /* clear from mouse hand */
            G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;
            ControlPointerReset();

            /* check for a 'ready effect' */
            InventoryDoEffect(EFFECT_TRIGGER_READY,location);
            /* check for 'destroy on equip' indicator */
            InventoryDestroyOn(EFFECT_TRIGGER_READY,location);

            /* set the color for the equipped/unequipped location */
            InventorySetPlayerColor(location);

            /* redraw ready area */
            if (location==EQUIP_LOCATION_READY_HAND) InventoryDrawReadyArea();
            /* redraw equipment if needed */
            if (BannerFormIsOpen (BANNER_FORM_EQUIPMENT)) InventoryDrawEquipmentWindow();
        }

    }
    DebugEnd();

    return (wasEquipped);
}



E_Boolean InventoryThrowObjectIntoWorld (T_word16 x, T_word16 y)
{
    E_Boolean retvalue=FALSE;

    T_sword32 throwX, throwY, throwZ; /** Where a thrown object starts **/
    T_sword16 throwspeed,throwangle;
    T_word16 objectRadius;
    T_word16 oldangle ;
#if 0
    T_packetShort packet;
    T_projectileAddPacket *p_addPacket;
#endif
    T_inventoryItemStruct *p_inv;
    T_3dObject *p_object;

    DebugRoutine ("InventoryThrowObjectIntoWorld");

    /** first, get a pointer to the object in the 'mouse hand' */
    if (InventoryObjectIsInMouseHand())
    {

        if (ClientIsPaused())  {
            MessageAdd("Cannot throw item, game is paused") ;
        } else if (ClientIsDead())  {
            MessageAdd("Cannot throw item, you are dead.") ;
        } else {
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
            p_object=p_inv->object;
            DebugCheck (p_object != NULL);

            /** Throw speed is based on the height of the mouse cursor. **/
            throwspeed=VIEW3D_HEIGHT - (y >> 1);
            if (throwspeed<10) throwspeed = 10;

            /** Throw angle is based on the x of the mouse cursor. **/
            throwangle = (((VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT+1)/2) - x) << 6;

            /** Add it to my angle to get the absolute angle. **/
            throwangle += PlayerGetAngle ();

            /** Now I need to calculate the starting coordinates of **/
            /** the object being thrown.  Start Z = my Z. **/

            /** Add the delta Z to the player's height. **/
            throwZ = PlayerGetZ () + ((PLAYER_OBJECT_HEIGHT >> 1) << 16);

            /** I need to place the object's center a distance **/
            /** away from me equal to the sum of his and my radii. **/
            objectRadius = ObjectGetRadius(p_object);

            objectRadius += objectRadius >> 1;

            /** To calculate X and Y, I need to move out from my center **/
            /** along a line at the 'throwangle' angle.  How far?  Well, **/
            /** it should start on or outside the circle which **/
            /** circumscribes my bounding sqare.  This circle has **/
            /** radius sqrt(2) * halfwidth, so I'll use 1.5 * halfwidth **/

            oldangle = PlayerGetAngle() ;
            PlayerSetAngle(throwangle) ;
            ObjectGetForwardPosition( PlayerGetObject(),
                                      PLAYER_OBJECT_RADIUS + (PLAYER_OBJECT_RADIUS/2) +
//                                      2 * PLAYER_OBJECT_RADIUS +
                                      objectRadius,
                                      &throwX,
                                      &throwY) ;
            PlayerSetAngle(oldangle) ;

            /* Can the object exist there? **/
            if (ObjectCheckIfCollide (p_object, throwX, throwY, throwZ))
            {
                /** Tell the user the bad news. **/
                MessageAdd ("^011You can't throw that object in that direction.");
            }
            else
            {
                if (Collide3dObjectToXYCheckLineOfSightWithZ(
                      PlayerGetObject(),
                      throwX>>16,
                      throwY>>16,
                      throwZ>>16) == TRUE)  {
                    MessageAdd ("^011You can't throw that object in that direction.");
                } else {
                    /* if there is a 'drop' effect do it now */
                    InventoryDoEffect(EFFECT_TRIGGER_DROP,EQUIP_LOCATION_MOUSE_HAND);

                    retvalue=TRUE;
                    /** We need to send a projectile packet to the server. **/
#if 0
                    p_addPacket = (T_projectileAddPacket *)(packet.data);

                    p_addPacket->command = PACKET_COMMANDCSC_PROJECTILE_CREATE;
                    p_addPacket->sourceObject = ObjectGetServerId (PlayerGetObject ());
                    p_addPacket->objectType = ObjectGetType (p_object);
                    p_addPacket->initialSpeed = throwspeed;
                    p_addPacket->angle = throwangle;

                    CmdQSendShortPacket (&packet, 140, 0, NULL);
#endif
                    ClientThrowObject(p_object, throwspeed, throwangle) ;

                    /** And, now that the server is about to create this new object, **/
                    /** we need to destroy our old copy. **/
         //           ObjectDestroy (p_object);

                    /* also, get rid of this link list element */
                    /* subtract weight of object from stats player load */
                    StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));
                    if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
                        InventoryDrawInventoryWindow(INVENTORY_PLAYER);
                    InventoryDestroyElement (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
                    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                    /** The object is now out of our hands. **/
                }
            }
        }
    }
    DebugEnd ();

    return (retvalue);
}


T_void InventoryTransferToReadyHand (T_void)
{
    T_doubleLinkListElement temp=NULL;
    T_inventoryItemStruct *p_inv;
    T_word16 dx,dy;
    T_word16 value;
    T_byte8 stmp[64];

    DebugRoutine ("InventoryTransferToReadyHand");

    if (ClientIsPaused())  {
        MessageAdd("Cannot transfer item, game is paused") ;
    } else if (ClientIsDead())  {
        MessageAdd("Cannot transfer item, you are dead.") ;
    } else {
        /* is there something in our mouse hand right now? */
        if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND] != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* check to make sure our class can ready this item */
            p_inv=DoubleLinkListElementGetData (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
            DebugCheck (p_inv != NULL);
            if (InventoryIsUseableByClass (p_inv))
            {
                /* we can use it */
            } else
            {
                MessageAdd ("^005Your class can't use this item");
                DebugEnd();
                return;
            }

            /* internal hard code to test weapons/armor for classes */
            if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON)
            {
                if (InventoryCheckClassCanUseWeapon(p_inv,TRUE)==FALSE)
                {
                    /* can't use this armor, exit */
                    DebugEnd();
                    return;
                }
            }

            /* check to make sure it will fit here (size) */
            dx=G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X2] -
               G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1];
            dy=G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y2]-
               G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1];

            if (p_inv->picwidth >= dx || p_inv->picheight >= dy)
            {
                MessageAdd ("^005This item won't fit in your ready hand.");
                DebugEnd();
                return;
            }
        }

        /* check for 'unready' effect */
        if (G_inventoryLocations[EQUIP_LOCATION_READY_HAND] != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* make sure this isn't an armor piece, if it is don't do it's unready effect here */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
              (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);
            DebugCheck (p_inv != NULL);

            if (p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_ARMOR)
            {
                /* if there is a 'unready' effect for the item do it now */
                InventoryDoEffect(EFFECT_TRIGGER_UNREADY,EQUIP_LOCATION_READY_HAND);

                /* check for 'destroy on unready' indicator */
                InventoryDestroyOn(EFFECT_TRIGGER_UNREADY,EQUIP_LOCATION_READY_HAND);
            }

            /* notify store */
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


        /* now, swap the quick pointers for the mouse hand and ready hand */
        temp=G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND];
        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=G_inventoryLocations[EQUIP_LOCATION_READY_HAND];
        G_inventoryLocations[EQUIP_LOCATION_READY_HAND]=temp;

        /* check for 'ready' effect */
        if (G_inventoryLocations[EQUIP_LOCATION_READY_HAND] != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* make sure we don't do a ready armor effect here */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
              (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);
            DebugCheck (p_inv != NULL);

            if (p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_ARMOR)
            {
                /* if there is a 'ready' effect for the item do it now */
                InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_READY_HAND);

                /* check for 'destroy on ready' indicator */
                InventoryDestroyOn(EFFECT_TRIGGER_READY,EQUIP_LOCATION_READY_HAND);
            }
        }
        /* redraw ready area */
        InventoryDrawReadyArea();

        /* LES: 03/27/96 - Added to change the weapon on the */
        /* piecewise player. */
        InventorySetPlayerColor(EQUIP_LOCATION_READY_HAND) ;
        if (BannerFormIsOpen(BANNER_FORM_EQUIPMENT))
            InventoryDrawEquipmentWindow();
    }

    DebugEnd();
}


T_void InventoryTransferToInventory (T_word16 x, T_word16 y)
{
    E_Boolean success=FALSE;
    T_inventoryItemStruct *p_inv,*p_inv2;
    T_byte8 gsx,gsy;
    T_doubleLinkListElement temp=NULL;
    T_byte8 stmp[128];
    E_inventoryType thisInv;
    T_word16 value;


    DebugRoutine ("InventoryTransferToInventory");

    /* figure out which inventory we are looking at */
    thisInv=InventoryFindInventoryWindow (x,y);

    /* first, check to make sure the inventory window is open */
    if (thisInv != INVENTORY_UNKNOWN)
    {
        if (ClientIsPaused())  {
            MessageAdd("Cannot transfer item, game is paused") ;
        } else if (ClientIsDead())  {
            MessageAdd("Cannot transfer item, you are dead.") ;
        } else {
            /* second, check to see if there is an object in the mouse hand */
            if (InventoryObjectIsInMouseHand()==TRUE)
            {
                /* get the pointer to the struct */
                p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
                /* fill out the rest of the structure */
                /* calculate number of grid squares it would cover */
                p_inv->gridspacesx=((p_inv->picwidth+INVENTORY_GRID_WIDTH)/
                                    (INVENTORY_GRID_WIDTH+1));
                p_inv->gridspacesy=((p_inv->picheight+INVENTORY_GRID_HEIGHT)/
                                    (INVENTORY_GRID_HEIGHT+1));
                /* find the starting grid coordinate for this item */
                x-=p_inv->picwidth/2;
                y-=p_inv->picheight/2;
                p_inv->gridstartx=((x-G_inventories[thisInv].locx1)/INVENTORY_GRID_WIDTH);
                p_inv->gridstarty=((y-G_inventories[thisInv].locy1)/INVENTORY_GRID_HEIGHT);

                /* make sure the area covered is in bounds */
                if (p_inv->gridstartx+p_inv->gridspacesx<=G_inventories[thisInv].gridspacesx &&
                    p_inv->gridstarty+p_inv->gridspacesy<=G_inventories[thisInv].gridspacesy)
                {
                    /* seems it will fit.  Check for an object already here */

                    temp=InventoryIsItemInArea(thisInv,
                                               p_inv->gridstartx,
                                               p_inv->gridstartx+p_inv->gridspacesx,
                                               p_inv->gridstarty,
                                               p_inv->gridstarty+p_inv->gridspacesy,
                                               G_inventories[thisInv].curpage);
                    if (temp == DOUBLE_LINK_LIST_ELEMENT_BAD)
                    {
                        /* if we are moving to the alternate inventory, we need */
                        /* to remove this item from the main inventory linked list */
                        /* and add it to the alternate one */
                        if (thisInv!=INVENTORY_PLAYER)
                        {
                            InventoryTransferItemBetweenInventories(p_inv,INVENTORY_PLAYER,thisInv);
                        }
                        /* good to go.  set storage page id and transfer */
			            p_inv->locx=p_inv->gridstartx*INVENTORY_GRID_WIDTH+G_inventories[thisInv].locx1+1;
					    p_inv->locy=p_inv->gridstarty*INVENTORY_GRID_HEIGHT+G_inventories[thisInv].locy1+1;
                        p_inv->storepage=G_inventories[thisInv].curpage;
                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;
                        InventoryDrawInventoryWindow(thisInv);
                    } else
                    {
                        /* someting ees already here */
                        /* see if it's the same type */
                        /* und stack them */
                        p_inv2=(T_inventoryItemStruct *) DoubleLinkListElementGetData (temp);
                        if (ObjectGetType(p_inv->object)==ObjectGetType(p_inv2->object))
                        {
                          /* stack em */
                          /* increment counter for p_inv2 */
                            if (p_inv2->numitems < p_inv2->itemdesc.numstackable)
                            {
                                p_inv2->numitems++;

                                /* destroy object assoc with p_inv */
    //                          ObjectDestroy (p_inv->object);

                                /* destroy link list data for p_inv */
                                InventoryDestroyElement (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
                                G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                                InventoryDrawInventoryWindow(thisInv);
                            }
                        }
                    }
                }
            }
            else
            {
                /* we must be trying to grab an object */
                /* see what we can find here */
                /* 1st, calculate grid start x and y for mouse cursor */
                gsx=((x-G_inventories[thisInv].locx1)/INVENTORY_GRID_WIDTH);
                gsy=((y-G_inventories[thisInv].locy1)/INVENTORY_GRID_HEIGHT);

                /* now, let's see if an object is at these coordinates */
                temp=InventoryIsItemInArea(thisInv,gsx,gsx+1,gsy,gsy+1,G_inventories[thisInv].curpage);
                if (temp != DOUBLE_LINK_LIST_ELEMENT_BAD)
                {
                    /* found something */
                    /* get the inventory struct */
                    p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(temp);

                    /* if in store mode, notify player if the store will buy this item */
                    /* probably wrong place for this code, but should work ok */
                    if (StoreIsOpen()==TRUE &&
                        StoreHouseModeIsOn()==FALSE &&
                        thisInv==INVENTORY_PLAYER)
                    {
                        value=StoreGetBuyValue(p_inv);
                        /* find out if store will buy this type of item */
                        if (StoreWillBuy(p_inv->itemdesc.type) &&
                            value != 0 )
                        {
                            StoreConvertCurrencyToString(stmp,value);
                            MessagePrintf("^011I'll buy that for %s^011",stmp);
                        }
                        else
                        {
                            MessageAdd ("^004I don't want to buy that.");
                        }
                    }

                    if (p_inv->numitems > 1)
                    {
                        /* more than 1 item here, create a new item for mousehand */
                        p_inv->numitems--;
                        p_inv2=MemAlloc (sizeof(T_inventoryItemStruct));
                        *p_inv2=*p_inv;
                        p_inv2->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                        p_inv2->numitems=1;
                        p_inv2->object=ObjectDuplicate(p_inv->object);
                        p_inv2->elementID=DoubleLinkListAddElementAtEnd(G_inventories[thisInv].itemslist,p_inv2);
                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=p_inv2->elementID;
                        InventoryDrawInventoryWindow(thisInv);
                    } else
                    {
                        /* if this inventory is an alternate inventory */
                        /* we need to remove this item from the alternate */
                        /* inventory linked list and transfer it to the */
                        /* main inventory linked list */
                        if (thisInv!=INVENTORY_PLAYER)
                        {
                            temp=InventoryTransferItemBetweenInventories(p_inv,thisInv,INVENTORY_PLAYER);
                        }
                        /* set the mouse hand to this object */
                        p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=temp;
                        p_inv->numitems=1;
                        /* redraw */
                        InventoryDrawInventoryWindow(thisInv);
                    }
                }
            }
        }
    }

    DebugCompare ("InventoryTransferToInventory");
    DebugEnd();
}


T_void InventoryTransferToEquipment (T_word16 x, T_word16 y)
{
    T_word16 i;
    T_inventoryItemStruct *p_inv;
    E_Boolean canstore=FALSE;
    T_word16 location=EQUIP_LOCATION_UNKNOWN;
    T_doubleLinkListElement temp;
    E_statsClassType ourclass=CLASS_UNKNOWN;
    T_word16 value;
    T_byte8 stmp[128];

    DebugRoutine ("InventoryTransferToEquipment");

    /* make sure the equipment window is open */
    if (InventoryEquipmentWindowIsAt (x,y))
    {
        if (ClientIsPaused())
        {
            MessageAdd("Cannot transfer item, game is paused") ;
        }
        else if (ClientIsDead())
        {
            MessageAdd("Cannot transfer item, you are dead.") ;
        }
        else
        {
            /* are we trying to store an object? */
            if (InventoryObjectIsInMouseHand()==TRUE)
            {
                /* figure out it's type */
                p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
                       G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

                if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_ARMOR)
                {
                    switch (p_inv->itemdesc.subtype)
                    {
                        case EQUIP_ARMOR_TYPE_BRACING_CLOTH:
                        case EQUIP_ARMOR_TYPE_BRACING_CHAIN:
                        case EQUIP_ARMOR_TYPE_BRACING_PLATE:
                        if (x>248)
                        {
                            location=EQUIP_LOCATION_RIGHT_ARM;
                        }
                        else
                        {
                            location=EQUIP_LOCATION_LEFT_ARM;
                        }
                        break;

                        case EQUIP_ARMOR_TYPE_LEGGINGS_CLOTH:
                        case EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN:
                        case EQUIP_ARMOR_TYPE_LEGGINGS_PLATE:
                        location=EQUIP_LOCATION_LEGS;
                        break;

                        case EQUIP_ARMOR_TYPE_HELMET_CLOTH:
                        case EQUIP_ARMOR_TYPE_HELMET_CHAIN:
                        case EQUIP_ARMOR_TYPE_HELMET_PLATE:
                        location=EQUIP_LOCATION_HEAD;
                        break;

                        case EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH:
                        case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                        case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                        location=EQUIP_LOCATION_CHEST;
                        break;

                        default:
                        DebugCheck(0);
                        break;
                    }

                    canstore=TRUE;

                    /* it's an armor, see if we can use it */
                    /* check to make sure our class can use it */
                    if (!InventoryIsUseableByClass(p_inv))
                    {
                        MessageAdd ("^005Your class can't equip this item.");
                        canstore=FALSE;
                    }

                    ourclass=StatsGetPlayerClassType();
                    /* check armor allowances */
                    if (canstore==TRUE)
                    {
                        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_ARMOR)
                           canstore=InventoryCheckClassCanUseArmor (p_inv,TRUE);
                    }
                }
                else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_RING ||
                         p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_AMULET)
                {
                    for (i=EQUIP_LOCATION_NECK;i<=EQUIP_LOCATION_RING_4;i++)
                    {
                        if (x >= G_inventoryBoxCoordinates[i][EQUIP_LOC_X1] &&
                            x <= G_inventoryBoxCoordinates[i][EQUIP_LOC_X2] &&
                            y >= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1] &&
                            y <= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y2])
                        {
                            location=i;
                            if (location==EQUIP_LOCATION_NECK &&
                                p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_AMULET) canstore=TRUE;
                            else if (location>=EQUIP_LOCATION_RING_1 &&
                                     location<=EQUIP_LOCATION_RING_4 &&
                                     p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_RING) canstore=TRUE;
                            break;
                        }
                    }
                    if (!InventoryIsUseableByClass(p_inv))
                    {
                        MessageAdd ("^005Your class can't equip this item.");
                        canstore=FALSE;
                    }
                }
                else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON)
                {
                    if (InventoryCheckObjectInReadyHand()!=NULL)
                    {
                        MessageAdd("^005There is already something in your hand.");
                        SoundPlayByNumber(SOUND_DING,255);
                    }
                    else
                    {
                        /* move this to ready hand */
                        InventoryTransferToReadyHand();
                        canstore=FALSE;
                    }
                }
            }
            else
            {
                /* we have nothing in our hand, find a location */
                for (i=EQUIP_LOCATION_HEAD;i<=EQUIP_LOCATION_RING_4;i++)
                {
                    if (x >= G_inventoryBoxCoordinates[i][EQUIP_LOC_X1] &&
                        x <= G_inventoryBoxCoordinates[i][EQUIP_LOC_X2] &&
                        y >= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1] &&
                        y <= G_inventoryBoxCoordinates[i][EQUIP_LOC_Y2])
                    {
                        location=i;
                    }
                }
            }

            /* is there something in the mousehand right now? */
            if (InventoryObjectIsInMouseHand()==TRUE)
            {
                /* we are trying to store this object.  let's see */
                /* if it is legal here */
                /* get a pointer to the invstruct */

                if (canstore==TRUE && location != EQUIP_LOCATION_UNKNOWN)
                {
                    /* we are trying to equip a legal object */
                    /* is there something already here? */
                    if (G_inventoryLocations[location]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                    {
                        /* nope, equip object and remove from mouse hand*/
                        G_inventoryLocations[location]=G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND];
                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=DOUBLE_LINK_LIST_ELEMENT_BAD;
                    } else
                    {
                        /* something is here, swap them */
                        /* check for an 'unequip action' for item now in mouse hand (if any) */
                        InventoryDoEffect(EFFECT_TRIGGER_UNREADY,location);
                        /* check for 'destroy on unequip' indicator */
                        InventoryDestroyOn(EFFECT_TRIGGER_UNREADY,location);

                        temp=G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND];
                        G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=G_inventoryLocations[location];
                        G_inventoryLocations[location]=temp;
                    }

                    /* check for 'equip action' for equipped item (if any) */
                    /* if there is a 'equip' effect for the item do it now */

                    InventoryDoEffect(EFFECT_TRIGGER_READY,location);
                    /* check for 'destroy on equip' indicator */
                    InventoryDestroyOn(EFFECT_TRIGGER_READY,location);

                    /* set the color for the equipped/unequipped location */
                    InventorySetPlayerColor(location);

                    /* redraw screen */
                    InventoryDrawEquipmentWindow();
                }
            }
            else if (location != EQUIP_LOCATION_UNKNOWN)
            {
                /* nothing is in mouse hand, we must be trying to */
                /* unequip something here */
                if (G_inventoryLocations[location]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
                {
                    /* something is here, unequip it */
                    /* check for an 'unequip action' for item now in inv slot (if any) */
                    InventoryDoEffect(EFFECT_TRIGGER_UNREADY,location);
                    /* check for 'destroy on unequip' indicator */
                    InventoryDestroyOn(EFFECT_TRIGGER_UNREADY,location);

                    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=G_inventoryLocations[location];
                    G_inventoryLocations[location]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                    /* update color for player location specified by i */
                    InventorySetPlayerColor(location);
                    /* redraw screen */
                    InventoryDrawEquipmentWindow();

                    /* notify if store */
                    if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]!=DOUBLE_LINK_LIST_BAD &&
                        StoreIsOpen()==TRUE)
                    {
                        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);
                        value=StoreGetBuyValue(p_inv);
                        /* find out if store will buy this type of item */
                        if (StoreWillBuy(p_inv->itemdesc.type) &&
                            value>0)
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
        }
    }
    DebugEnd();
}



/* when this routine is called, if there is a coin in the mouse hand */
/* it will be added to the finances for the player */
T_void InventoryTransferToFinances (T_void)
{
    T_inventoryItemStruct *p_inv;
    T_word16 type=0;

    DebugRoutine ("InventoryTransferToFinances");

    /* check to see if the object in the mouse hand is a coin */
    p_inv=InventoryCheckItemInMouseHand();
    if (p_inv!=NULL)
    {
        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_COIN)
        {
            /* it's a coin.  Figure out it's type, add it to finances */
            /* and then clear the mouse out */
            type=ObjectGetType(p_inv->object);
            if (type==101 ||
                type==103 ||
                type==105 ||
                type==107)
            {
                StatsAddCoin (p_inv->itemdesc.subtype,5);
            }
            else
            {
                StatsAddCoin (p_inv-> itemdesc.subtype,1);
            }
            InventoryDestroyItemInMouseHand();

        }
    }

    DebugEnd();
}

/* when this routine is called, if there are any bolts in the mouse hand */
/* they will be added to the ammunition for the player */
T_void InventoryTransferToAmmo (T_void)
{
    T_inventoryItemStruct *p_inv;
    T_word16 type=0;
    T_word16 numbolts=0;

    DebugRoutine ("InventoryTransferToAmmo");
    /* check to see if the object in the mouse hand is a coin */
    p_inv=InventoryCheckItemInMouseHand();
    if (p_inv!=NULL)
    {
        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_BOLT)
        {
            /* it's a bolt.  Figure out it's type, add it to bolt list */
            /* and then clear the mouse out */
            StatsAddBolt (p_inv->itemdesc.subtype,1);
            InventoryDestroyItemInMouseHand();
        }
        else if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_QUIVER)
        {
            /* it's a quiver. */
            numbolts=ObjectGetAccData(p_inv->object);
            if(numbolts==0) numbolts=12;
            StatsAddBolt (p_inv->itemdesc.subtype,numbolts);
            InventoryDestroyItemInMouseHand();
        }
    }

    DebugEnd();
}

T_void InventoryDrawInventoryWindow (E_inventoryType which)
{
	T_word16 i,j;
    T_byte8 stmp[48];
    T_TxtboxID TxtboxID=NULL;
    T_byte8 invcnt=0;

    DebugRoutine ("InventoryDrawInventoryWindow");

    /* set graphics screen */
//    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);

    /* clear grid */
    for (i=0;i<G_inventories[which].gridspacesx;i++)
    {
      for (j=0;j<G_inventories[which].gridspacesy;j++)
      {

         GrDrawRectangle (G_inventories[which].locx1+(i*(INVENTORY_GRID_WIDTH)),
                          G_inventories[which].locy1+(j*(INVENTORY_GRID_HEIGHT)),
                          G_inventories[which].locx1+((i+1)*(INVENTORY_GRID_WIDTH)),
                          G_inventories[which].locy1+((j+1)*(INVENTORY_GRID_HEIGHT)),INVENTORY_BASE_COLOR);
         GrDrawFrame     (G_inventories[which].locx1+(i*(INVENTORY_GRID_WIDTH)),
                          G_inventories[which].locy1+(j*(INVENTORY_GRID_HEIGHT)),
                          G_inventories[which].locx1+((i+1)*(INVENTORY_GRID_WIDTH)),
                          G_inventories[which].locy1+((j+1)*(INVENTORY_GRID_HEIGHT)),INVENTORY_GRID_COLOR);
      }
    }

    /* now, iterate through all objects in list and draw objects assigned */
    /* to the current inventory page */
    G_activeInventory=which;
    DoubleLinkListTraverse (G_inventories[which].itemslist,InventoryDrawElement);
    G_activeInventory=INVENTORY_PLAYER;
    /* draw other info on inventory screen */
    if (which==INVENTORY_PLAYER)
    {
        sprintf (stmp,"LOAD:%3.1f kg",StatsGetPlayerLoad()/10.0);
        TxtboxID=FormGetObjID(500);
        if (TxtboxID != NULL)
        {
            TxtboxSetData (TxtboxID, stmp);
        }
    }

    if (which==INVENTORY_PLAYER)
    {
        sprintf (stmp,"PG:%d/%d",G_inventories[which].curpage+1,G_inventories[which].maxpages);
        TxtboxID=FormGetObjID(501);
    } else
    {
//        sprintf (stmp,"PAGE:%d of %d",G_inventories[which].curpage+1,G_inventories[which].maxpages);
//        TxtboxID=FormGetObjID(510);
    }

    if (TxtboxID!=NULL)
    {
        TxtboxSetData (TxtboxID,stmp);
    }

    DebugCompare ("InventoryDrawInventoryWindow");

    DebugEnd();
}


static E_Boolean InventoryDrawElement (T_doubleLinkListElement element)
{
    T_inventoryItemStruct *p_inv;
    E_Boolean retvalue=TRUE;
    T_resourceFile res;
    T_resource font;
    T_bitfont *p_font;
    T_byte8 stmp[32];
	T_word16 x1,x2,y1,y2;
	T_word16 xoff,yoff;

    DebugRoutine ("InventoryDrawElement");
    /* examine this element and see if it lies on the current inv page */
    p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
    DebugCheck (p_inv != NULL);

    if (p_inv->storepage==G_inventories[G_activeInventory].curpage)
    {
        /* draw this element */
        x1=G_inventories[G_activeInventory].locx1+(p_inv->gridstartx*INVENTORY_GRID_WIDTH);
		y1=G_inventories[G_activeInventory].locy1+(p_inv->gridstarty*INVENTORY_GRID_HEIGHT);
		x2=x1+(p_inv->gridspacesx*INVENTORY_GRID_WIDTH);
		y2=y1+(p_inv->gridspacesy*INVENTORY_GRID_HEIGHT);

		GrDrawRectangle (x1,y1,x2,y2,INVENTORY_BASE_COLOR-1);
		GrDrawFrame (x1,y1,x2,y2,INVENTORY_HIGHLIGHT_COLOR);
		xoff=((x2-x1)-p_inv->picwidth)/2;
		yoff=((y2-y1)-p_inv->picheight)/2;
		GrDrawCompressedBitmapAndColor( p_inv->p_bitmap,
                                        p_inv->locx+xoff,
                                        p_inv->locy+yoff,
                                        ObjectGetColorizeTable(p_inv->object));
        /* place the number of inventory items here in the upper left corner */
        if (p_inv->numitems>1)
        {
            /* open the font for drawing number of objects */
            res=ResourceOpen ("sample.res");
            font=ResourceFind (res,"FontTiny");
            p_font=ResourceLock (font);
            GrSetBitFont (p_font);

            sprintf (stmp,"%d",p_inv->numitems);
            GrSetCursorPosition (x1+2,y1+2);
            GrDrawShadowedText (stmp,INVENTORY_TEXT_COLOR,0);

            /* close the font */
            ResourceUnlock (font);
            ResourceClose (res);
        }
    }

    /* set load counter */
    DebugCompare ("InventoryDrawElement");
    DebugEnd();
    return (retvalue);
}


T_void InventoryDrawEquipmentWindow (T_void)
{
    T_word16 i,locx,locy;
    T_word16 shield=0, value=0;
    T_inventoryItemStruct *p_inv;
    T_TxtboxID TxtboxID;
    T_byte8 stmp[32];

    DebugRoutine ("InventoryDrawEquipmentWindow");

    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);
    /* draw rendering of player */
    GrDrawRectangle (217,19,280,82,INVENTORY_BASE_COLOR);
    PlayerDraw (217,19);

    /* draw amulets and rings */
    for (i=EQUIP_LOCATION_NECK;i<=EQUIP_LOCATION_RING_4;i++)
    {
        /* erase previous contents */
        GrDrawRectangle (G_inventoryBoxCoordinates[i][EQUIP_LOC_X1],
                         G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1],
                         G_inventoryBoxCoordinates[i][EQUIP_LOC_X2],
                         G_inventoryBoxCoordinates[i][EQUIP_LOC_Y2],
                         INVENTORY_BASE_COLOR);

        if (G_inventoryLocations[i]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* something is here, draw it */
            /* get pointer to inv struct */
            p_inv=DoubleLinkListElementGetData (G_inventoryLocations[i]);
            /* draw it */
            locx=((G_inventoryBoxCoordinates[i][EQUIP_LOC_X2] -
                   G_inventoryBoxCoordinates[i][EQUIP_LOC_X1])/2 +
                   G_inventoryBoxCoordinates[i][EQUIP_LOC_X1]);
            locx-=p_inv->picwidth/2;

            locy=((G_inventoryBoxCoordinates[i][EQUIP_LOC_Y2] -
                   G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1])/2 +
                   G_inventoryBoxCoordinates[i][EQUIP_LOC_Y1]);
            locy-=p_inv->picheight/2;

		    GrDrawCompressedBitmapAndColor(p_inv->p_bitmap,
                                           locx,
                                           locy,
                                           ObjectGetColorizeTable(p_inv->object));
        }
    }

    /* set up field values */
    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_SHIELD)==TRUE)
    {
        shield=EffectGetPlayerEffectPower(PLAYER_EFFECT_SHIELD);
    }

    TxtboxID=FormGetObjID (500);
    value=StatsGetArmorValue(EQUIP_LOCATION_HEAD);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_HEAD) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_HEAD) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (501);
    value=StatsGetArmorValue(EQUIP_LOCATION_CHEST);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_CHEST) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_CHEST) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (502);
    value=StatsGetArmorValue(EQUIP_LOCATION_LEFT_ARM);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_LEFT_ARM) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_LEFT_ARM) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (503);
    value=StatsGetArmorValue(EQUIP_LOCATION_RIGHT_ARM);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_RIGHT_ARM) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_RIGHT_ARM) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (504);
    value=StatsGetArmorValue(EQUIP_LOCATION_LEGS);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_LEGS) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_LEGS) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (505);
    value=StatsGetArmorValue(EQUIP_LOCATION_LEGS);
//    value=(StatsGetArmorValue(EQUIP_LOCATION_LEGS) > shield ?
//           StatsGetArmorValue(EQUIP_LOCATION_LEGS) : shield);
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (506);
    value= shield ;
    sprintf (stmp,"%d%%",value);
    TxtboxSetData (TxtboxID,stmp);

    /* calculate total armor value */

    TxtboxID=FormGetObjID (507);
    sprintf (stmp,"%d%%",StatsGetPlayerArmorValue());
    TxtboxSetData (TxtboxID,stmp);

    DebugEnd();
}



T_void InventoryDrawReadyArea (T_void)
{
    T_word16 x, y;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDrawReadyArea");

    /* draw a black box */
    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);
    GrDrawRectangle (G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1],
                     G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1],
                     G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X2],
                     G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y2],
                     INVENTORY_BASE_COLOR);

    if (InventoryObjectIsInReadyHand()==TRUE)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
          G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);

        x=(G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X2]-
           G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1])/2+
           G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_X1];
        x-=(p_inv->picwidth/2);

        y=(G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y2]-
           G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1])/2+
           G_inventoryBoxCoordinates[EQUIP_LOCATION_READY_HAND][EQUIP_LOC_Y1];
        y-=(p_inv->picheight/2);

        /* redraw inventory object */
		GrDrawCompressedBitmapAndColor( p_inv->p_bitmap,
                                        x,
                                        y,
                                        ObjectGetColorizeTable(p_inv->object));
    }

    /* check to see if what's in our hand is a weapon */
    /* if not, set the animation to null */

    if (InventoryObjectIsInReadyHand())
    {
        /* is it a weapon or does it have an animation ? */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
          (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);
        DebugCheck (p_inv != NULL);

        switch (p_inv->itemdesc.type)
        {
            case EQUIP_OBJECT_TYPE_WEAPON:
            switch (p_inv->itemdesc.subtype)
            {
                case EQUIP_WEAPON_TYPE_AXE:
                OverlaySetAnimation (ANIMATION_OVERLAY_AXE);
                break;
                case EQUIP_WEAPON_TYPE_DAGGER:
                OverlaySetAnimation (ANIMATION_OVERLAY_DAGGER);
                break;
                case EQUIP_WEAPON_TYPE_LONGSWORD:
                OverlaySetAnimation (ANIMATION_OVERLAY_SWORD);
                break;
                case EQUIP_WEAPON_TYPE_SHORTSWORD:
                OverlaySetAnimation (ANIMATION_OVERLAY_SWORD);
                break;
                case EQUIP_WEAPON_TYPE_MACE:
                OverlaySetAnimation (ANIMATION_OVERLAY_MACE);
                break;
                case EQUIP_WEAPON_TYPE_STAFF:
                OverlaySetAnimation (ANIMATION_OVERLAY_STAFF);
                break;
                case EQUIP_WEAPON_TYPE_CROSSBOW:
                OverlaySetAnimation (ANIMATION_OVERLAY_XBOW);
                break;
                case EQUIP_WEAPON_TYPE_WAND:
                OverlaySetAnimation (ANIMATION_OVERLAY_NULL);
                break;
                default:
                /* failed, bad weapon type! */
                DebugCheck (0);
            }
            break;

            case EQUIP_OBJECT_TYPE_WAND:
            OverlaySetAnimation (ANIMATION_OVERLAY_NULL);
            break;

            case EQUIP_OBJECT_TYPE_SCROLL:
            case EQUIP_OBJECT_TYPE_FOODSTUFF:
            case EQUIP_OBJECT_TYPE_POWERUP:
            case EQUIP_OBJECT_TYPE_THING:
            case EQUIP_OBJECT_TYPE_COIN:
            case EQUIP_OBJECT_TYPE_BOLT:
            case EQUIP_OBJECT_TYPE_ARMOR:
            case EQUIP_OBJECT_TYPE_AMULET:
            case EQUIP_OBJECT_TYPE_RING:
            case EQUIP_OBJECT_TYPE_POTION:
            case EQUIP_OBJECT_TYPE_QUIVER:
            OverlaySetAnimation (ANIMATION_OVERLAY_NULL);
            break;

            default:
            /* bad item ! */
            DebugCheck (0);
        }
    }
    else
    {
        /* nothing is in ready hand, use fists */
        OverlaySetAnimation (0);
    }

    DebugEnd();
}



static T_doubleLinkListElement InventoryIsItemInArea (E_inventoryType invWin, T_byte8 gx1, T_byte8 gx2, T_byte8 gy1, T_byte8 gy2, T_byte8 page)
{
    T_doubleLinkListElement retvalue=DOUBLE_LINK_LIST_ELEMENT_BAD;
    DebugRoutine ("InventoryIsItemInArea");

    /* cheap, but use global slots to pass coordinates to iterator function */
    G_x1=gx1;G_x2=gx2;G_y1=gy1;G_y2=gy2;G_page=page;

    /* iterate and look for this object */
    retvalue=DoubleLinkListTraverse (G_inventories[invWin].itemslist,InventoryFindOverlap);

    DebugEnd();
    return (retvalue);
}


static E_Boolean InventoryFindOverlap (T_doubleLinkListElement element)
{
    E_Boolean retvalue=FALSE;
    T_inventoryItemStruct *p_inv;
    T_byte8 T_x1,T_x2,T_y1,T_y2;

    DebugRoutine ("InventoryFindOverlap");
    /* get pointer to struct */
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData (element);
        /* test for overlap */
        /* assumes G_x1, G_y1, ... has been set by calling routine */

        T_x1=p_inv->gridstartx;
        T_x2=T_x1+p_inv->gridspacesx;
        T_y1=p_inv->gridstarty;
        T_y2=T_y1+p_inv->gridspacesy;

        if (G_x2 <= T_x1  ||
            G_x1 >= T_x2  ||
            G_y2 <= T_y1  ||
            G_y1 >= T_y2  ||
            p_inv->storepage != G_page) retvalue=TRUE;
    }
    DebugEnd();
    return (retvalue);
}


/* The following routine is intended to look at the given */
/* equipment location (ready hand included) and change the */
/* body part on the player to match the readied/unreadied item. */
/* The word "Color" is misleading because the armor type is */
/* also changed as well as the "color" (aka material type). */
/* update LES: 03/27/96 */
static T_void InventorySetPlayerColor (E_equipLocations location)
{
    T_byte8 color;
    T_inventoryItemStruct *p_inv;
    T_word16 bodyPart ;
    T_word16 subType ;
    T_word16 type ;

    DebugRoutine ("InventorySetPlayerColor");

    if (G_inventoryLocations[location] != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
            G_inventoryLocations[location]);
        color=ObjectGetColorizeTable(p_inv->object);

        /* LES: 03/27/96 */
        /* What subtype is this item? */
        subType = p_inv->itemdesc.subtype ;
        type = p_inv->itemdesc.type ;

        /* Are we equipping a weapon or armor? */
        if (location == EQUIP_LOCATION_READY_HAND)  {
            if (type == EQUIP_OBJECT_TYPE_WEAPON)  {
                /* Get the body object for the weapon. */
                bodyPart = G_weaponToBodyPartTable[subType] ;
            } else {
                bodyPart = G_standardBodyParts[EQUIP_LOCATION_READY_HAND] ;
            }
        } else {
            /* Get the body object for the armor. */
            bodyPart = G_armorToBodyPartTable[subType] ;
        }

        /* LES: 03/27/96 */
        /* Since the inventory description structure only */
        /* declares arming pieceing to be "bracings" and there */
        /* is a difference between left and right arms, we just */
        /* go to the next body part in the list if we want a right arm. */
        if (location == EQUIP_LOCATION_LEFT_ARM)
            bodyPart++ ;
    }
    else
    {
        /* nothing here  ... unequiping the item */
        color=14;
        if (location <= EQUIP_LOCATION_SHIELD_HAND)
            bodyPart = G_standardBodyParts[location] ;
        else
            bodyPart = 0 ;
    }

    /* Put the correct color into the body part. */
    bodyPart |= (((T_word16)color)<<12) ;

    switch(location)
    {
        case EQUIP_LOCATION_HEAD:
            PlayerSetBodyPart(BODY_PART_LOCATION_HEAD, bodyPart) ;
            break ;
        case EQUIP_LOCATION_RIGHT_ARM:
            PlayerSetBodyPart(BODY_PART_LOCATION_LEFT_ARM, bodyPart) ;
            break ;
        case EQUIP_LOCATION_LEFT_ARM:
            PlayerSetBodyPart(BODY_PART_LOCATION_RIGHT_ARM, bodyPart) ;
            break ;
        case EQUIP_LOCATION_CHEST:
            PlayerSetBodyPart(BODY_PART_LOCATION_CHEST, bodyPart) ;
            break ;
        case EQUIP_LOCATION_LEGS:
            PlayerSetBodyPart(BODY_PART_LOCATION_LEGS, bodyPart) ;
            break ;
        case EQUIP_LOCATION_SHIELD_HAND:
            PlayerSetBodyPart(BODY_PART_LOCATION_SHIELD, bodyPart) ;
            break ;
        case EQUIP_LOCATION_READY_HAND:
            PlayerSetBodyPart(BODY_PART_LOCATION_WEAPON, bodyPart) ;
            break ;
    }

/* LES: 03/27/96 -- commented out the old version that only changes */
/* the color. */
#if 0
    switch (location)
    {
        case EQUIP_LOCATION_HEAD:
            part = PlayerGetBodyPart(BODY_PART_LOCATION_HEAD) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_HEAD, part) ;
        break;

        case EQUIP_LOCATION_LEFT_ARM:
            part = PlayerGetBodyPart(BODY_PART_LOCATION_RIGHT_ARM) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_RIGHT_ARM, part) ;
        break;

        case EQUIP_LOCATION_RIGHT_ARM:
            part = PlayerGetBodyPart(BODY_PART_LOCATION_LEFT_ARM) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_LEFT_ARM, part) ;
        break;

        case EQUIP_LOCATION_CHEST:
            part = PlayerGetBodyPart(BODY_PART_LOCATION_CHEST) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_CHEST, part) ;
        break;

//        case EQUIP_LOCATION_BODY:
//        break;

        case EQUIP_LOCATION_LEGS:
        part = PlayerGetBodyPart(BODY_PART_LOCATION_LEGS) & 0x0FFF;
        part |= ((T_word16)color) << 12 ;
        PlayerSetBodyPart(BODY_PART_LOCATION_LEGS, part) ;
        break;

//        case EQUIP_LOCATION_LEFT_LEG:
/* CHANGED */
/*
            part = PlayerGetBodyPart(BODY_PART_LOCATION_RIGHT_LEG) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_RIGHT_LEG, part) ;
*/
//        break;

//        case EQUIP_LOCATION_RIGHT_LEG:
/* CHANGED */
/*
            part = PlayerGetBodyPart(BODY_PART_LOCATION_LEFT_LEG) & 0x0FFF;
            part |= ((T_word16)color) << 12 ;
            PlayerSetBodyPart(BODY_PART_LOCATION_LEFT_LEG, part) ;
*/

        case EQUIP_LOCATION_SHIELD_HAND:
        break;
    }
#endif

    DebugEnd();
}


static E_Boolean InventoryDoItemEffect (T_inventoryItemStruct *p_inv, E_effectTriggerType trigger)
{
    T_word16 i;
    T_word16 type;
    E_Boolean retvalue=FALSE;
    DebugRoutine ("InventoryDoItemEffect");

    /* scan through triggers to see if we find a match */
    for (i=0;i<MAX_ITEM_EFFECTS;i++)
    {
        if (p_inv->itemdesc.effectTriggerOn[i]==trigger)
        {
            if (p_inv->itemdesc.effectType[i]==EFFECT_SET_ARMOR)
            {
                /* Should not happen, fail */
                DebugCheck (0);
            }

            type=p_inv->itemdesc.effectType[i];
/*
            printf ("Calling effect - type=%d trig=%d dat=%d,%d,%d\n",
                    type,
                    trigger,
                    p_inv->itemdesc.effectData[i][0],
                    p_inv->itemdesc.effectData[i][1],
                    p_inv->itemdesc.effectData[i][2]);
            fflush (stdout);
*/

            /* we triggered an effect */
            if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_POTION ||
                p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_SCROLL)
            {
                Effect(type,
                   trigger,
                   p_inv->itemdesc.effectData[i][0],
                   p_inv->itemdesc.effectData[i][1],
                   p_inv->itemdesc.effectData[i][2],
                   (T_void *)ObjectGetType(p_inv->object));
            }
            else
            {
                Effect(type,
                   trigger,
                   p_inv->itemdesc.effectData[i][0],
                   p_inv->itemdesc.effectData[i][1],
                   p_inv->itemdesc.effectData[i][2],
                   (T_void *)p_inv->object);
            }

            /* test to see if we should identify this item */
            if ((trigger==EFFECT_TRIGGER_USE) &&
                (type != EFFECT_DISPLAY_CANNED_MESSAGE) &&
                (type != EFFECT_PLAY_LOCAL_SOUND) &&
                (type != EFFECT_PLAY_AREA_SOUND) &&
                (type != EFFECT_COLOR_FLASH))
            {
                if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_POTION ||
                    p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_SCROLL ||
                    p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_FOODSTUFF)
                {
//                    if ((rand()%100) < StatsGetPlayerAttribute(ATTRIBUTE_MAGIC))
//                    {
                        /* id this object */
                        StatsPlayerIdentify(ObjectGetType(p_inv->object));
//                    }
                }
            }
            retvalue=TRUE;
        }
    }

    DebugEnd();
    return (retvalue);

}




E_Boolean InventoryDoEffect  (E_effectTriggerType trigger, E_equipLocations location)
{
    E_Boolean retvalue=FALSE;
    T_inventoryItemStruct *p_inv;
    T_word16 i;
    T_word16 type;
    DebugRoutine ("InventoryDoEffect");

    if (G_inventoryLocations[location] != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get pointer to invstruct */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
            G_inventoryLocations[location]);

        /* scan through triggers to see if we find a match */
        for (i=0;i<MAX_ITEM_EFFECTS;i++)
        {
            if (p_inv->itemdesc.effectTriggerOn[i]==trigger)
            {
                if (p_inv->itemdesc.effectType[i]==EFFECT_SET_ARMOR)
                {
                    /* store location in effect data */
                    p_inv->itemdesc.effectData[i][1]=location;
                }

                type=p_inv->itemdesc.effectType[i];

                DebugCheck(p_inv != NULL) ;
                DebugCheck(p_inv->object != NULL) ;
                DebugCheck(ObjectIsValid(p_inv->object)) ;

                /* we triggered an effect */
                /* we triggered an effect */
                if ((p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_POTION ||
                    p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_SCROLL) &&
                    (type != EFFECT_ADD_JOURNAL_PAGE_BY_OBJECT))
                {
                    Effect(type,
                       trigger,
                       p_inv->itemdesc.effectData[i][0],
                       p_inv->itemdesc.effectData[i][1],
                       p_inv->itemdesc.effectData[i][2],
                       (T_void *)ObjectGetType(p_inv->object));
                }
                else
                {
                    Effect(type,
                       trigger,
                       p_inv->itemdesc.effectData[i][0],
                       p_inv->itemdesc.effectData[i][1],
                       p_inv->itemdesc.effectData[i][2],
                       (T_void *)p_inv->object);
                }

                /* test to see if we should identify this item */
                if ((trigger==EFFECT_TRIGGER_USE) &&
                    (type != EFFECT_DISPLAY_CANNED_MESSAGE) &&
                    (type != EFFECT_PLAY_LOCAL_SOUND) &&
                    (type != EFFECT_PLAY_AREA_SOUND) &&
                    (type != EFFECT_COLOR_FLASH))
                {
                    if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_POTION ||
                        p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_SCROLL ||
                        p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_FOODSTUFF)
                    {
//                        if ((rand()%100) < StatsGetPlayerAttribute(ATTRIBUTE_MAGIC))
//                        {
                            /* id this object */
                            StatsPlayerIdentify(ObjectGetType(p_inv->object));
//                        }
                    }
                }
                retvalue=TRUE;
            }
        }
    }

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryDestroyOn (E_effectTriggerType trigger, E_equipLocations location)
{
    E_Boolean retvalue=FALSE;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroyOn");
    /* make sure there is a valid item at location */
    if (G_inventoryLocations[location]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* something is here, check destroy on value to see if it matches */
        /* the current trigger */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
            G_inventoryLocations[location]);

        if (p_inv->itemdesc.objectDestroyOn==trigger && trigger > EFFECT_TRIGGER_NONE)
        {
            /* we triggered a destroy object call.  remove the object */
            /* first, remove the weight */
            /* unless it's a coin or an ammo */
            if ((p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_COIN) &&
                (p_inv->itemdesc.type != EQUIP_OBJECT_TYPE_BOLT))
            {
                StatsChangePlayerLoad (-ObjectGetWeight (p_inv->object));
            }
            InventoryDestroyElement (G_inventoryLocations[location]);
            G_inventoryLocations[location]=DOUBLE_LINK_LIST_ELEMENT_BAD;

            retvalue=TRUE;
        }
    }

    DebugEnd();
    return (retvalue);
}


E_Boolean InventoryShouldDestroyOn (E_effectTriggerType trigger, E_equipLocations location)
{
    E_Boolean retvalue=FALSE;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroyOn");
    /* make sure there is a valid item at location */
    if (G_inventoryLocations[location]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* something is here, check destroy on value to see if it matches */
        /* the current trigger */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
            G_inventoryLocations[location]);

        if (p_inv->itemdesc.objectDestroyOn==trigger && trigger > EFFECT_TRIGGER_NONE)
        {
            /* we triggered a destroy object call.  remove the object */
            retvalue=TRUE;
        }
    }

    DebugEnd();
    return (retvalue);
}


/* returns object type of object destroyed */
/* 0 if failed to destroy object */
T_word16 InventoryDestroyRandomEquippedItem (T_void)
{
    T_word16 i, numitems=0;
    T_word16 retvalue=0;
    T_word16 destroyme;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroyRandomEquippedItem");

    /* iterate through all equip locations, couting how many there are */
    /* skip MOUSE HAND location (1) */
    for (i=1;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
    {
        if (G_inventoryLocations[i]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* valid place, increment */
            numitems++;
        }
    }

    if (numitems > 0)
    {
        /* get a random number relating to the equipped item we're to destroy */
        destroyme=rand()%numitems+1;

        /* re iterate, and find this location */
        for (i=1;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
        {
            if (G_inventoryLocations[i]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
            {
                destroyme--;
                if (destroyme==0)
                {
                    /* found it, now kill it */
                    /* check for 'unequip effect' */
                    InventoryDoEffect(EFFECT_TRIGGER_UNREADY,i);

                    /* get pointer to invstruct */
                    p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
                                               (G_inventoryLocations[i]);
                    DebugCheck (p_inv != NULL);

                    /* subtract weight */
                    StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));

                    /* set return value */
                    retvalue=ObjectGetType (p_inv->object);

                    /* remove item */
                    InventoryDestroyElement (G_inventoryLocations[i]);
                    G_inventoryLocations[i]=DOUBLE_LINK_LIST_ELEMENT_BAD;

                    if (i==EQUIP_LOCATION_READY_HAND)
                    {
                        /* redraw ready area */
                        InventoryDrawReadyArea();
                    }

                    /* set the color for the equipped/unequipped location */
                    if (i>=EQUIP_LOCATION_READY_HAND && i<=EQUIP_LOCATION_SHIELD_HAND)
                       InventorySetPlayerColor(i);


                    /* if equip window is open redraw */
                    if (BannerFormIsOpen (BANNER_FORM_EQUIPMENT))
                       InventoryDrawEquipmentWindow();

                    /* redraw inventory screen if open (load change) */
                    if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
                       InventoryDrawInventoryWindow(INVENTORY_PLAYER);

                    /* redraw banner status display */
                    BannerStatusBarUpdate();

                    /* found it, break */
                    break;
                }
            }
        }
    }

    DebugEnd();
    return (retvalue);
}

/* returns object type of object destroyed */
/* 0 if failed to destroy object */
T_word16 InventoryDestroyRandomStoredItem (T_void)
{
    T_word16 retvalue=0;
    T_doubleLinkListElement element;
    T_word16 count=0;
    T_word16 numitems=0;
    T_word16 destroyme=0;
    E_Boolean success;
    T_word16 i;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroyRandomStoredItem");

//    printf ("in destroy random stored\n");

    /* we should be able to just destroy an element here */
    /* there should be no effects caused by destroying a random */
    /* stored item (hopefully... ) */

    /* first, count the number of items in the inventory */
    element=DoubleLinkListGetFirst (G_inventories[INVENTORY_PLAYER].itemslist);

    /* traverse through, the list and count the number of items */

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* scan through list of 'equipped' pointers and don't include */
        /* in count if same */
        /* note, not an efficient way but there are few EQUIP_NUMBER_OF*/
        success=TRUE;
        for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
        {
            if (element==G_inventoryLocations[i]) success=FALSE;
        }
        /* ignore runes */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_THING &&
            (p_inv->itemdesc.subtype==THING_TYPE_RUNE ||
            p_inv->itemdesc.subtype==THING_TYPE_KEY)) success=FALSE;

        if (success==TRUE) numitems++;

        element=DoubleLinkListElementGetNext (element);
    }

//    printf ("destroy: numitems=%d\n",numitems);
//    fflush (stdout);

    /* now, get a random number IDing the item to destroy */
    if (numitems>0)
    {
        destroyme=rand()%numitems+1;
        /* re-traverse through the list and find this element */
        element=DoubleLinkListGetFirst (G_inventories[INVENTORY_PLAYER].itemslist);
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
//            printf ("searching, destroyme=%d\n",destroyme);
//            fflush (stdout);
            success=TRUE;
            for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
            {
                if (element==G_inventoryLocations[i]) success=FALSE;
            }
            /* ignore runes */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
            if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_THING &&
            (p_inv->itemdesc.subtype==THING_TYPE_RUNE ||
            p_inv->itemdesc.subtype==THING_TYPE_KEY)) success=FALSE;

            if (success==TRUE)
            {
                destroyme--;
                if (destroyme==0)
                {
                    /* found this element, destroy it */
                    /* get data for this element first */
//                  p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
                    DebugCheck (p_inv != NULL);

//                    printf ("destroying object\n");
//                    fflush (stdout);
                    /* subtract the weight of this item */
                    StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));

                    /* get object type to return */
                    retvalue=ObjectGetType (p_inv->object);

                    /* cause 'drop effect' for this item */
                    InventoryDoItemEffect(p_inv,EFFECT_TRIGGER_DROP);

                    if (p_inv->numitems > 1)
                    {
                        /* more than 1 item, only destroy one */
                        p_inv->numitems--;
                    }
                    else
                    {
                        /* only 1 item, remove the whole thing */
                        InventoryDestroyElement (element);
                    }
                    /* we found it, update the inventory screen if open */
                    /* and then exit */
                    if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
                       InventoryDrawInventoryWindow(INVENTORY_PLAYER);
                    /* load has changed, update banner stats display */
                    BannerStatusBarUpdate();
                    /* exit */
                    break;
                }
            }
            element=DoubleLinkListElementGetNext(element);
        }
    }

    DebugEnd();
    return (retvalue);
}

/* returns true if player's class can use/equip this inventory item */
E_Boolean InventoryIsUseableByClass (T_inventoryItemStruct *p_inv)
{
    E_Boolean retvalue=FALSE;
    T_word16 ourclass;
    DebugRoutine ("InventoryIsUseableByClass");

    DebugCheck (p_inv != NULL);
    ourclass=1<<StatsGetPlayerClassType();
    if (ourclass&p_inv->itemdesc.useable)
    {
        retvalue=TRUE;
    }

    DebugEnd();
    return (retvalue);
}

/* function returns type of armor in location specified by location */
/* this function is called by stats when taking electrical damage */

E_equipArmorTypes InventoryGetArmorType (E_equipLocations location)
{
    T_inventoryItemStruct *p_inv;
    E_equipArmorTypes retvalue=EQUIP_ARMOR_TYPE_UNKNOWN;

    DebugRoutine ("InventoryGetArmorType");

    DebugCheck (location >= EQUIP_LOCATION_HEAD);
    DebugCheck (location <= EQUIP_LOCATION_SHIELD_HAND);

    /* get p_inv for item in this location */
    if (G_inventoryLocations[location]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
          (G_inventoryLocations[location]);

        /* check for armor type */
        retvalue=p_inv->itemdesc.subtype;
    }

    DebugEnd();
    return (retvalue);
}


T_void   InventoryPlayWeaponAttackSound(T_void)
{
    T_inventoryItemStruct *p_inv;
    T_byte8 randSound;

    DebugRoutine ("InventoryPlayWeaponAttackSound");

    /* randomize swing sounds */
    randSound=rand()%3;

    if (InventoryObjectIsInReadyHand())
    {
        /* get item in ready hand */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
          (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);

        /* make sure it's a weapon */
        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON)
        {
            /* emit an occasional war cry */
            if (rand()%10==1 && p_inv->itemdesc.subtype!=EQUIP_WEAPON_TYPE_CROSSBOW)
            {
                SoundPlayByNumber(SOUND_PLAYER_WAR_CRY_SET+(rand()%3),255);
/*               switch (StatsGetPlayerClassType())
                {
                    case CLASS_CITIZEN:
                    case CLASS_PRIEST:
                    case CLASS_ARCHER:
                    SoundPlayByNumber(SOUND_CITIZEN_WAR_CRY,255);
                    break;

                    case CLASS_KNIGHT:
                    case CLASS_PALADIN:
                    SoundPlayByNumber(SOUND_PALADIN_WAR_CRY,255);
                    break;

                    case CLASS_MAGE:
                    case CLASS_MAGICIAN:
                    case CLASS_WARLOCK:
                    SoundPlayByNumber(SOUND_MAGE_WAR_CRY,255);
                    break;

                    case CLASS_ROGUE:
                    case CLASS_SAILOR:
                    case CLASS_MERCENARY:
                    SoundPlayByNumber(SOUND_ROGUE_WAR_CRY,255);
                    break;

                    default:
                    DebugCheck(0);
                    break;
                }
*/
            }

            /* play an attack sound */
            switch (p_inv->itemdesc.subtype)
            {

                case EQUIP_WEAPON_TYPE_DAGGER:
                SoundPlayByNumber (SOUND_SWING_SET3+randSound,200);
                break;

                case EQUIP_WEAPON_TYPE_AXE:
                case EQUIP_WEAPON_TYPE_LONGSWORD:
                SoundPlayByNumber (SOUND_SWING_SET2+randSound,200);
                break;

                case EQUIP_WEAPON_TYPE_SHORTSWORD:
                case EQUIP_WEAPON_TYPE_MACE:
                case EQUIP_WEAPON_TYPE_STAFF:
                SoundPlayByNumber (SOUND_SWING_SET1+randSound,200);
                break;

                case EQUIP_WEAPON_TYPE_CROSSBOW:
                SoundPlayByNumber (SOUND_XBOW,255);
                break;

                case EQUIP_WEAPON_TYPE_WAND:
                SoundPlayByNumber (SOUND_ELECTRIC1,255);
                break;

                case EQUIP_WEAPON_TYPE_NONE:
                default:
                /* bad weapon in hand! */
                DebugCheck (0);
                break;
            }
        }
    }
    DebugEnd();
}


T_void InventoryPlayWeaponHitSound(T_void)
{
    T_inventoryItemStruct *p_inv;
    T_byte8 randSound;
    DebugRoutine ("InventoryPlayWeaponHitSound");

    /* randomize hit sounds */
    randSound=rand()%3;

    if (InventoryObjectIsInReadyHand())
    {
        /* get item in ready hand */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData
          (G_inventoryLocations[EQUIP_LOCATION_READY_HAND]);

        /* make sure it's a weapon */
        if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON)
        {
            /* play an attack sound */
            switch (p_inv->itemdesc.subtype)
            {

                case EQUIP_WEAPON_TYPE_DAGGER:
                SoundPlayByNumber(SOUND_DAGGERHIT+randSound,255);
                break;

                case EQUIP_WEAPON_TYPE_STAFF:
                case EQUIP_WEAPON_TYPE_MACE:
                SoundPlayByNumber (SOUND_MACEHIT+randSound,255);
                break;

                case EQUIP_WEAPON_TYPE_AXE:
                case EQUIP_WEAPON_TYPE_LONGSWORD:
                case EQUIP_WEAPON_TYPE_SHORTSWORD:
                SoundPlayByNumber (SOUND_SWORDHIT+randSound,255);
                break;

                default:
                SoundPlayByNumber (SOUND_FISTHIT+randSound,255);
                break;
            }
        }
    }
    else
    {
        SoundPlayByNumber (SOUND_FISTHIT+randSound,255);
    }
    DebugEnd();
}



/* routine reads inventory items into active inventory structure */

T_void InventoryReadItemsList(FILE *fp)
{
    T_inventoryItemStruct *p_inv;
    T_inventoryItemStruct blank;
    T_word32 size;
    T_word16 i;
    T_word16 cntr=0;

    DebugRoutine ("InventoryReadPlayerItemsList");
    DebugCheck (fp != NULL);

    /* calculate size of record */
    size=sizeof(T_inventoryItemStruct);

    /* clean out a 'blank' structure for compare */
    memset (&blank,0,size);

    /* Remove any old effects. */
//    InventoryRemoveEquippedEffects() ;

    /* delete all current inventory items */
    InventoryClear(INVENTORY_PLAYER);
//    InventoryInit();

    /* now, read in our 'equipped' item list, and add them */
    /* to our items list if they are not null */
    for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
    {
        /* allocate a new chunk for read */
        p_inv=(T_inventoryItemStruct *)MemAlloc(size);

        /* clean it */
        memset (p_inv,0,size);

        /* read in an item */
        if (!feof(fp)) fread (p_inv,size,1,fp);

        /* see if it's a valid entry */
        if ((p_inv->objecttype != 0) && (!feof(fp)))
        {
            /* recreate the object */
            p_inv->object=ObjectCreateFake ();
            DebugCheck(p_inv->object != NULL) ;
//printf("inv object: %d\n", p_inv->objecttype) ;
            ObjectSetType (p_inv->object,p_inv->objecttype);
            ObjectSetAngle (p_inv->object,0x4000);

            /* fix the object bitmap pointer */
            p_inv->p_bitmap=(T_bitmap *)ObjectGetBitmap(p_inv->object);

            /* set the inventory and equip list location pointer */
            /* and add it to the list */
            p_inv->elementID=DoubleLinkListAddElementAtEnd(G_inventories[INVENTORY_PLAYER].itemslist,p_inv);
            G_inventoryLocations[i]=p_inv->elementID;

            /* trigger any 'equip' effects for item */
//          InventoryDoEffect (EFFECT_TRIGGER_READY,i);
        }
        else
        {
            /* it's blank, throw it away */
            MemFree (p_inv);
        }
    }

    /* get the rest of the items */
    while (!feof(fp))
    {
        /* allocate a new chunk for read */
        p_inv=(T_inventoryItemStruct *)MemAlloc(size);

        /* read in the block */
        fread (p_inv,size,1,fp);

        /* see if it's a valid entry */
        if ((p_inv->objecttype != 0) && (!feof(fp)))
        {
            /* recreate the object */
            p_inv->object=ObjectCreateFake ();
//printf("inv object2: %d\n", p_inv->objecttype) ;
            ObjectSetType (p_inv->object,p_inv->objecttype);
            ObjectSetAngle (p_inv->object,0x4000);

            /* fix the object bitmap pointer */
            p_inv->p_bitmap=(T_bitmap *)ObjectGetBitmap(p_inv->object);

            /* set the inventory and equip list location pointer */
            /* and add it to the list */
            p_inv->elementID=DoubleLinkListAddElementAtEnd(G_inventories[INVENTORY_PLAYER].itemslist,p_inv);
        }
        else
        {
            /* it's blank, throw it away */
            MemFree (p_inv);
        }
    }

    DebugEnd();
}



T_void InventoryWriteItemsList (FILE *fp)
{
    T_word16 i;
    E_Boolean beenwritten;
    T_doubleLinkListElement element;

    DebugRoutine ("InventoryWritePlayerItemsList");
    DebugCheck (fp != NULL);

    /* first, write out our 'equipped' items in order */
    /* and pad empty spots with 'blank' items */
    for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
    {
        /* write the item associated with the equip slot */
        IInventoryWriteItem(fp, G_inventoryLocations[i]) ;
    }

    /* now, traverse through the entire list and write out all items *
    /* except for the 'equipped' ones */

    element=DoubleLinkListGetFirst (G_inventories[INVENTORY_PLAYER].itemslist);
    /* traverse through the list and write each item */

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {

        /* has this element already been written ? */
        beenwritten=FALSE;
        for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
        {
            if (element==G_inventoryLocations[i])
            {
                beenwritten=TRUE;
                break;
            }
        }

        if (!beenwritten)
        {
            /* Write the non-equipped item out to the file */
            IInventoryWriteItem(fp, element) ;
        }

        /* get the next element */
        element=DoubleLinkListElementGetNext (element);
    }

    DebugEnd();
}

static T_void IInventoryWriteItem(
                  FILE *fp,
                  T_doubleLinkListElement element)
{
    T_inventoryItemStruct itemCopy ;
    T_inventoryItemStruct *p_item = NULL ;

    DebugRoutine("IInventoryWriteItem") ;
    DebugCheck(fp != NULL) ;

    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_item =
            (T_inventoryItemStruct *) DoubleLinkListElementGetData(element) ;
    }

    if (p_item)  {
        itemCopy = *p_item ;
        itemCopy.object = NULL ;
        itemCopy.p_bitmap = NULL ;
        itemCopy.elementID = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    } else {
        memset(&itemCopy, 0, sizeof(itemCopy)) ;
    }

    fwrite(&itemCopy, sizeof(itemCopy), 1, fp);

    DebugEnd() ;
}

/* routine reorders location and page of all inventory items */
/* if multipage=true then then each type of item will be stored on */
/* a separate page */

T_void InventoryReorder (E_inventoryType which, E_Boolean multipage)
{
    T_doubleLinkListElement element;
    T_doubleLinkListElement temp;
    T_inventoryItemStruct *p_inv;
    E_equipObjectTypes currentInvObjectType=0;
    T_byte8 gridx=0;
    T_byte8 gridy=0;
    T_byte8 gsizex=0,gsizey=0;
    T_byte8 curpage=0;

    DebugRoutine ("InventoryReorder");


    if (which==INVENTORY_STORE)
    {
        /*'group' like items */
        InventoryGroupItems(which);
        /* and sort store inventory by type */
        InventorySortByType(which);
    }

    /* clear out locational information */
    InventoryClearCurrentLocations(which);

    /* get first inventory element */
    element=DoubleLinkListGetFirst (G_inventories[which].itemslist);

    /* set current object type for bypage sort */
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData (element);
        currentInvObjectType=p_inv->itemdesc.type;
    }

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {

        /* get the size (in gridspaces) of this item */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        gsizex=p_inv->gridspacesx;
        gsizey=p_inv->gridspacesy;

        /* check to see if this item demands a new page */
        if (p_inv->itemdesc.type > currentInvObjectType)
        {
            /* advance a page if multipage flag is set */
            currentInvObjectType=p_inv->itemdesc.type;
            if (multipage==TRUE)
            {
                gridx=0;
                gridy=0;
                curpage++;
            }
        }

        /* is there an item in this area? */
        temp=DOUBLE_LINK_LIST_ELEMENT_BAD;
        temp=InventoryIsItemInArea(which,gridx,gridx+gsizex,gridy,gridy+gsizey,curpage);

        if (temp == DOUBLE_LINK_LIST_ELEMENT_BAD &&
            gridy+gsizey<=G_inventories[which].gridspacesy &&
            gridx+gsizex<=G_inventories[which].gridspacesx)
        {
            /* nothing is here, move this item to this location */
            p_inv->gridstartx=gridx;
            p_inv->gridstarty=gridy;
	        p_inv->locx=p_inv->gridstartx*INVENTORY_GRID_WIDTH+G_inventories[which].locx1+1;
			p_inv->locy=p_inv->gridstarty*INVENTORY_GRID_HEIGHT+G_inventories[which].locy1+1;
            p_inv->storepage=curpage;
            /* increment to next location to test */
            gridx+=gsizex;

            /* get next element */
            element=DoubleLinkListElementGetNext(element);
        }
        else
        {
            /* increment to next location to test */
            gridx+=1;
        }

        /* check to see if we ran out of space */
        if (gridx>=G_inventories[which].gridspacesx)
        {
            gridx=0;
            gridy++;
            /* advance a row */
            if (gridy>=G_inventories[which].gridspacesy)
            {
                gridy=0;
                curpage++;
                /* advance a page */
                if (curpage >= G_inventories[which].maxpages+1)
                {
                    DebugCheck(0);
                    break;
                }
           }
        }
    }

//    if (BannerFormIsOpen (BANNER_FORM_INVENTORY)) InventoryDrawInventoryWindow(which);

    if (which==INVENTORY_STORE)
    {
        G_inventories[which].maxpages=curpage+1;
        if (G_inventories[which].curpage > curpage) G_inventories[which].curpage=curpage;

        InventoryUpdateStoreItemTitlePage();
    }
//    else
//    {
        /* clear equipped item pointers */
//        for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
//        {
//            G_inventoryLocations[i]=DOUBLE_LINK_LIST_ELEMENT_BAD;
//        }

        /* make sure mouse pointer is ok */
//        ControlSetDefaultPointer (CONTROL_MOUSE_POINTER_DEFAULT);
//    }
    DebugEnd();
}


/* routine sorts linked list inventory order based on invdesc.type */
static T_void InventorySortByType(E_inventoryType which)
{
    T_doubleLinkList list1,list2;
    T_doubleLinkListElement element,nextelement;
    T_inventoryItemStruct *p_inv;
    E_equipObjectTypes comparetype=EQUIP_OBJECT_TYPE_WEAPON;
    DebugRoutine ("InventorySortByType");
    /* simple 'apple-pick' sort */
    /* get list to sort handle */
    list1=G_inventories[which].itemslist;

    /* create list to sort to */
    list2=DoubleLinkListCreate();

    /* iterate through list1, removing all items of invdesc.type comparetype */
    /* each time and moving to list2 */

    for (comparetype=EQUIP_OBJECT_TYPE_WEAPON;
         comparetype < EQUIP_OBJECT_TYPE_UNKNOWN;
         comparetype++)
    {
        element=DoubleLinkListGetFirst(list1);

        /* traverse list 1 */
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            nextelement=DoubleLinkListElementGetNext(element);
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
            if (p_inv->itemdesc.type==comparetype)
            {
                /* move this element to list2 */
                DoubleLinkListRemoveElement (element);
                p_inv->elementID=DoubleLinkListAddElementAtEnd (list2,p_inv);
            }
            element=nextelement;
        }
    }

    /* remove empty list */
    DoubleLinkListDestroy (list1);
    G_inventories[which].itemslist=list2;

    fflush (stdout);
    DebugEnd();
}

/* routine looks through the inventory and groups like items */
static T_void InventoryGroupItems (E_inventoryType whichInv)
{
    T_doubleLinkListElement elm1,elm2;
    T_doubleLinkList tempList;
    T_inventoryItemStruct *p_item1,*p_item2;
    E_Boolean match=FALSE;
    DebugRoutine ("InventoryGroupItems");

    elm1=DoubleLinkListGetFirst(G_inventories[whichInv].itemslist);
    tempList=DoubleLinkListCreate();

    while (elm1 != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_item1=(T_inventoryItemStruct *)DoubleLinkListElementGetData(elm1);

        elm2=DoubleLinkListGetFirst(tempList);
        match=FALSE;
        while (elm2 != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* iterate through temporary list and compare types */
            p_item2=(T_inventoryItemStruct *)DoubleLinkListElementGetData(elm2);
            if (p_item1->objecttype==p_item2->objecttype)
            {
                /* match found, increment numitems counter */
                p_item2->numitems+=p_item1->numitems;
                match=TRUE;
                break;
            }

            elm2=DoubleLinkListElementGetNext(elm2);
        }
        if (match!=TRUE)
        {
            /* no duplicates found, add item to second list */
            DoubleLinkListAddElementAtEnd(tempList,p_item1);
        }

        elm1=DoubleLinkListElementGetNext(elm1);
    }

    DoubleLinkListDestroy(G_inventories[whichInv].itemslist);
    G_inventories[whichInv].itemslist=tempList;

    DebugEnd();
}


/* routine clears x and y locations of all inventory items */
/* called only by InventoryReorder() */
static T_void InventoryClearCurrentLocations(E_inventoryType which)
{
    T_doubleLinkListElement element;
    T_inventoryItemStruct *p_inv;
    DebugRoutine ("InventoryClearCurrentLocations");

    element=DoubleLinkListGetFirst(G_inventories[which].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData (element);
        p_inv->locx=0;
        p_inv->locy=0;
        p_inv->gridstartx=127;
        p_inv->gridstarty=127;
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
}


/* LES: 03/28/96  Routine to update all the weapons and items */
/* on a character. */
T_void InventoryUpdatePlayerEquipmentBodyParts(T_void)
{
    E_equipLocations location ;

    /* Update the color/body part per location of the character */
    /* from hand position (first) to shield position (last). */
    for (location = EQUIP_LOCATION_READY_HAND;
         location <= EQUIP_LOCATION_SHIELD_HAND;
         location++)
    {
        InventorySetPlayerColor(location) ;
    }

    /* Redraw the equipment banner form if it is open. */
    /* Typically, it is not. */
    if (BannerFormIsOpen(BANNER_FORM_EQUIPMENT))
        InventoryDrawEquipmentWindow();
}


static T_void InventorySetWindowSizeDefaults(T_void)
{
    DebugRoutine ("InventorySetWindowSizeDefaults");

    G_inventories[INVENTORY_PLAYER].locx1=213;
    G_inventories[INVENTORY_PLAYER].locy1=16;
    G_inventories[INVENTORY_PLAYER].locx2=315;
    G_inventories[INVENTORY_PLAYER].locy2=129;
    G_inventories[INVENTORY_PLAYER].gridspacesx=6;
    G_inventories[INVENTORY_PLAYER].gridspacesy=8;
    G_inventories[INVENTORY_PLAYER].maxpages=5;
    G_inventories[INVENTORY_PLAYER].numpages=5;
    G_inventories[INVENTORY_PLAYER].curpage=0;
    G_inventories[INVENTORY_PLAYER].itemslist=DoubleLinkListCreate();

    /* default 'store' inventory */
    G_inventories[INVENTORY_STORE].locx1=0;
    G_inventories[INVENTORY_STORE].locy1=0;
    G_inventories[INVENTORY_STORE].locx2=0;
    G_inventories[INVENTORY_STORE].locy2=0;
    G_inventories[INVENTORY_STORE].gridspacesx=6;
    G_inventories[INVENTORY_STORE].gridspacesy=8;
    G_inventories[INVENTORY_STORE].maxpages=0;
    G_inventories[INVENTORY_STORE].numpages=0;
    G_inventories[INVENTORY_STORE].curpage=0;
    G_inventories[INVENTORY_STORE].itemslist=DoubleLinkListCreate();

    DebugEnd();
}


T_void InventorySetInventoryWindowLocation (E_inventoryType type,
                                            T_word16        x1,
                                            T_word16        y1,
                                            T_word16        x2,
                                            T_word16        y2,
                                            T_byte8         gx,
                                            T_byte8         gy,
                                            T_byte8         maxpages)
{
    DebugRoutine ("InventorySetInventoryWindowLocation");

    DebugCheck (type < INVENTORY_UNKNOWN);

    G_inventories[type].locx1=x1;
    G_inventories[type].locx2=x2;
    G_inventories[type].locy1=y1;
    G_inventories[type].locy2=y2;
    G_inventories[type].gridspacesx=gx;
    G_inventories[type].gridspacesy=gy;
    G_inventories[type].maxpages=maxpages;

    DebugEnd();
}

T_void InventoryMoveInventoryWindowLocation (E_inventoryType type,
                                             T_word16 dx,
                                             T_word16 dy)
{
    T_doubleLinkListElement element;
    T_inventoryItemStruct *p_item;

    DebugRoutine ("InventoryMoveInventoryWindowLocation");
    DebugCheck (type < INVENTORY_UNKNOWN);

    /* translate window area first */
    G_inventories[type].locx1+=dx;
    G_inventories[type].locx2+=dx;
    G_inventories[type].locy1+=dy;
    G_inventories[type].locy2+=dy;

    /* now translate location of all objects (x,y) in list */
    if (G_inventories[type].itemslist != DOUBLE_LINK_LIST_BAD)
    {
        element=DoubleLinkListGetFirst (G_inventories[type].itemslist);

        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            p_item=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
            p_item->locx+=dx;
            p_item->locy+=dy;
            element=DoubleLinkListElementGetNext(element);
        }
    }

    DebugEnd();
}



T_doubleLinkListElement InventoryTransferItemBetweenInventories(T_inventoryItemStruct *p_inv,
                                                                E_inventoryType source,
                                                                E_inventoryType dest)
{
    T_inventoryItemStruct *testInv;
    T_doubleLinkListElement retvalue=DOUBLE_LINK_LIST_ELEMENT_BAD;
    T_doubleLinkListElement element;
    E_Boolean success=FALSE;

    DebugRoutine ("InventoryTransferBetweenInventories");
    DebugCheck (p_inv!=NULL);

    /* remove structure from old linked list */
    /* traverse list searching for this pointer */
    element=DoubleLinkListGetFirst(G_inventories[source].itemslist);

    while (element!=DOUBLE_LINK_LIST_BAD)
    {
        testInv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (testInv==p_inv)
        {
            DoubleLinkListRemoveElement(element);
            success=TRUE;
            break;
        }

        element=DoubleLinkListElementGetNext(element);
    }

    DebugCheck (success==TRUE);

    /* add structure to linked list */
    p_inv->elementID=DoubleLinkListAddElementAtEnd(G_inventories[dest].itemslist,p_inv);
    retvalue=p_inv->elementID;

    /* clear location structures */
    p_inv->locx=0;
    p_inv->locy=0;
    p_inv->gridstartx=0;
    p_inv->gridstarty=0;
    p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;

    /* modify player load */
    if (source==INVENTORY_PLAYER)
    {
        /* subtract weight */
        StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));
    }
    else
    {
        /* add weight */
        StatsChangePlayerLoad (ObjectGetWeight(p_inv->object));
    }


    DebugEnd();
    return (retvalue);

}



T_void InventoryAddObjectToInventory(E_inventoryType which,
                                     T_word16 objID,
                                     T_byte8 numToAdd)
{
    T_3dObject *p_object=NULL;
    T_word16 i;
    DebugRoutine ("InventoryAddObjectToInventory");

    for (i=0;i<numToAdd;i++)
    {
        /* create an object of this type */
        /* get the invdesc structure for it and add it to */
        /* the inventory linked list */
        p_object=ObjectCreateFake();
        DebugCheck (p_object != NULL);

        ObjectSetType (p_object,objID);
        ObjectSetAngle (p_object,0x4000);

        InventoryTakeObject(which,p_object);
    }
    DebugEnd();
}


T_void InventoryRemoveObjectFromInventory (E_inventoryType which,
                                           T_word16 objID,
                                           T_byte8 numToRemove)
{
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;
    T_inventoryItemStruct *p_inv;
    E_Boolean success=FALSE;

    DebugRoutine ("InventoryRemoveObjectFromInventory");
    DebugCheck (which<INVENTORY_UNKNOWN);

    element=DoubleLinkListGetFirst(G_inventories[which].itemslist);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv->objecttype==objID)
        {
            /* remove this inventory item or subtract one from the */
            /* number here */
            if (p_inv->numitems > numToRemove)
            {
                p_inv->numitems-=numToRemove;
                success=TRUE;
            }
            else
            {
                DebugCheck (p_inv->numitems==numToRemove);
                InventoryDestroyElement (element);
                success=TRUE;
            }
            break;
        }
        element=DoubleLinkListElementGetNext(element);
    }

    DebugCheck (success==TRUE);
    DebugEnd();
}



T_void InventoryDebugDump (E_inventoryType which)
{
    T_inventoryItemStruct *p_inv;
    T_word16 count=0;
    T_doubleLinkListElement element;
    DebugRoutine ("InventoryDebugDump");

    printf ("mouse hand addr=%d\n",G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]);

    element=DoubleLinkListGetFirst(G_inventories[which].itemslist);

    while (element!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        printf ("item #%d: type=%d\n",count++,p_inv->itemdesc.type);
        fflush (stdout);
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
}

static T_doubleLinkListElement InventoryFindElement(E_inventoryType whichinv,
                                                    T_inventoryItemStruct *thisinv)
{
    T_doubleLinkListElement element,retvalue=DOUBLE_LINK_LIST_ELEMENT_BAD;
    T_inventoryItemStruct *p_inv;
    DebugRoutine ("InventoryFindElement");
    DebugCheck (whichinv<INVENTORY_UNKNOWN);

    element=DoubleLinkListGetFirst(G_inventories[whichinv].itemslist);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv==thisinv)
        {
            retvalue=element;
            break;
        }
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
    return (retvalue);
}


static T_void InventoryUpdateStoreItemTitlePage(T_void)
{
    T_doubleLinkListElement element;
    T_inventoryItemStruct *p_inv;
    T_byte8 stmp[64];
    DebugRoutine ("InventoryUpdateStoreItemTitlePage");
    /* find first item on this page */
    element=DoubleLinkListGetFirst(G_inventories[INVENTORY_STORE].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* is this item on current page? */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv->storepage==G_inventories[INVENTORY_STORE].curpage)
        {
            /* success, break */
            break;
        }

        element=DoubleLinkListElementGetNext(element);
    }

    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* set field to this item type */
        switch (p_inv->itemdesc.type)
        {
            case EQUIP_OBJECT_TYPE_WEAPON:
            strcpy (stmp,"Weapons");
            break;
            case EQUIP_OBJECT_TYPE_ARMOR:
            strcpy (stmp,"Armor");
            break;
            case EQUIP_OBJECT_TYPE_AMULET:
            strcpy (stmp,"Amulets");
            break;
            case EQUIP_OBJECT_TYPE_RING:
            strcpy (stmp,"Rings");
            break;
            case EQUIP_OBJECT_TYPE_POTION:
            strcpy (stmp,"Potions");
            break;
            case EQUIP_OBJECT_TYPE_WAND:
            strcpy (stmp,"Wands");
            break;
            case EQUIP_OBJECT_TYPE_SCROLL:
            strcpy (stmp,"Scrolls");
            break;
            case EQUIP_OBJECT_TYPE_FOODSTUFF:
            strcpy (stmp,"Food Items");
            break;
            case EQUIP_OBJECT_TYPE_POWERUP:
            DebugCheck (0); /* fail! */
            break;
            case EQUIP_OBJECT_TYPE_THING:
            strcpy (stmp,"Assorted Items");
            break;
            case EQUIP_OBJECT_TYPE_COIN:
            DebugCheck (0); /* fail! */
            break;
            case EQUIP_OBJECT_TYPE_BOLT:
            strcpy (stmp,"Bolts");
            break;
            case EQUIP_OBJECT_TYPE_QUIVER:
            strcpy (stmp,"Arrows");
            break;
            case EQUIP_OBJECT_TYPE_UNKNOWN:
            DebugCheck (0); /* fail! */
            break;
            default:
            DebugCheck (0); /* fail! */
            break;
        }

        /* set field data */
        TxtboxSetData(G_storePageType,stmp);
    }
    else
    {
        TxtboxSetData(G_storePageType,"");
    }

    /* set page number field */
    sprintf (stmp,"pg:%d/%d",G_inventories[INVENTORY_STORE].curpage+1,
                             G_inventories[INVENTORY_STORE].maxpages);

    TxtboxSetData(G_storePageNumber,stmp);

    DebugEnd();
}


T_void InventorySetMouseHandPointer(T_doubleLinkListElement element)
{
    DebugRoutine ("InventorySetMouseHandPointer");
    DebugCheck (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]==DOUBLE_LINK_LIST_ELEMENT_BAD);
    DebugCheck (element != DOUBLE_LINK_LIST_ELEMENT_BAD);

    G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=element;

    DebugEnd();
}


T_void InventoryOpenStoreInventory (T_void)
{
    DebugRoutine ("InventoryOpenStoreInventory");

    /* set up last/next page button */
    G_lastStorePage = ButtonCreate (STORE_LAST_PAGE_BUTTON_X1,
                                    STORE_LAST_PAGE_BUTTON_Y1,
                                    "UI/COMMON/LTARROW",
                                    FALSE,
                                    0,
                                    NULL,
                                    InventorySelectLastStoreInventoryPage);

    G_nextStorePage = ButtonCreate (STORE_NEXT_PAGE_BUTTON_X1,
                                    STORE_NEXT_PAGE_BUTTON_Y1,
                                    "UI/COMMON/RTARROW",
                                    FALSE,
                                    0,
                                    NULL,
                                    InventorySelectNextStoreInventoryPage);

    /* set up page type/page number displays */
    G_storePageType = TxtboxCreate (STORE_PAGE_TYPE_DISPLAY_X1,
                                    STORE_PAGE_TYPE_DISPLAY_Y1,
                                    STORE_PAGE_TYPE_DISPLAY_X2,
                                    STORE_PAGE_TYPE_DISPLAY_Y2,
                                    "FontTiny",
                                    0,
                                    0,
                                    FALSE,
                                    Txtbox_JUSTIFY_CENTER,
                                    Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                    NULL);

    G_storePageNumber = TxtboxCreate (STORE_PAGE_NUMBER_DISPLAY_X1,
                                    STORE_PAGE_NUMBER_DISPLAY_Y1,
                                    STORE_PAGE_NUMBER_DISPLAY_X2,
                                    STORE_PAGE_NUMBER_DISPLAY_Y2,
                                    "FontTiny",
                                    0,
                                    0,
                                    FALSE,
                                    Txtbox_JUSTIFY_CENTER,
                                    Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                    NULL);

    G_inventories[INVENTORY_STORE].curpage=0;

    /* set up alternate messaging mode */
//    MessageSetAlternateOutputOn();

    /* update item title/ page number */
    InventoryUpdateStoreItemTitlePage();

    /* draw inventory window */
    InventoryDrawInventoryWindow(INVENTORY_STORE);

    DebugEnd();
}


T_void InventoryCloseStoreInventory (T_void)
{
    DebugRoutine ("InventoryCloseStoreInventory");

    ButtonDelete (G_lastStorePage);
    ButtonDelete (G_nextStorePage);
    TxtboxDelete (G_storePageType);
    TxtboxDelete (G_storePageNumber);

    /* turn off alternate messaging mode */
//    MessageSetAlternateOutputOff();

    /* delete current store inventory */
    InventoryClear (INVENTORY_STORE);

    DebugEnd();
}


static T_void InventorySelectNextStoreInventoryPage (T_buttonID buttonID)
{
    DebugRoutine ("InventorySelectNextStoreInventoryPage");

    InventorySelectNextInventoryPage(INVENTORY_STORE);

    DebugEnd();
}


static T_void InventorySelectLastStoreInventoryPage (T_buttonID buttonID)
{
    DebugRoutine ("InventorySelectLastStoreInventoryPage");

    InventorySelectLastInventoryPage(INVENTORY_STORE);

    DebugEnd();
}

static T_sword16 G_equipEffectLevel = 0 ;

/* removes all player effects associated with equipped inventory items */
T_void InventoryRemoveEquippedEffects (T_void)
{
    T_word16 i;
    DebugRoutine ("InventoryRemoveEquippedEffects");

    G_equipEffectLevel=0 ;
//printf("- InventoryRemoveEquippedEffects by %s\n", DebugGetCallerName()) ;
//fflush (stdout);
//  if (G_equipEffectLevel == 0)
    {
//printf("InventoryRemoveEquippedEffects by %s\n", DebugGetCallerName()) ;
        /* turn off effect sound/flashes for this procedure */
        EffectSoundOff();
        for (i=EQUIP_LOCATION_READY_HAND;i<EQUIP_LOCATION_UNKNOWN;i++)
        {
            InventoryDoEffect (EFFECT_TRIGGER_UNREADY,i);
        }
        EffectSoundOn();
    }

    DebugEnd();
}

/* restores all player effects associated with equipped inventory items */
T_void InventoryRestoreEquippedEffects (T_void)
{
    T_word16 i;
    DebugRoutine ("InventoryRestoreEquippedEffects");
    G_equipEffectLevel++ ;

    DebugCheck (G_equipEffectLevel < 2);
//printf("+ InventoryRestoreEquippedEffects by %s\n", DebugGetCallerName()) ;
//fflush (stdout);
///    if (G_equipEffectLevel==1)
    {
//printf("InventoryRestoreEquippedEffects by %s\n", DebugGetCallerName()) ;
        /* turn off effect sound/flashes for this procedure */
        EffectSoundOff();
        for (i=EQUIP_LOCATION_READY_HAND;i<EQUIP_LOCATION_UNKNOWN;i++)
        {
            InventoryDoEffect (EFFECT_TRIGGER_READY,i);
        }
        EffectSoundOn();
    }

    DebugEnd();
}


/* returns the number of objectType found in the inventory.  If there */
/* aren't any, it returns 0 */
T_word16 InventoryHasItem (E_inventoryType whichInventory, T_word16 objectType)
{
    T_doubleLinkListElement element;
    T_word16 numFound=0;
    T_inventoryItemStruct *p_inv;
    DebugRoutine ("InventoryHasItem");
    DebugCheck (whichInventory < INVENTORY_UNKNOWN);

    element=DoubleLinkListGetFirst (G_inventories[whichInventory].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get this item structure */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv->objecttype==objectType)
        {
            /* increment numFound counter */
            numFound+=p_inv->numitems;
        }

        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();

    return (numFound);
}

/* attemps to destroy numToDestroy objectType in the player/store inventory. */
/* on success, returns TRUE, else FALSE */
E_Boolean InventoryDestroySpecificItem (E_inventoryType whichInventory,
                                        T_word16 objectType,
                                        T_word16 numToDestroy)
{
    E_Boolean success=FALSE;
    T_doubleLinkListElement element,nextElement;
    T_inventoryItemStruct *p_inv;

    DebugRoutine ("InventoryDestroySpecificItem");
    DebugCheck (whichInventory < INVENTORY_UNKNOWN);

    element=DoubleLinkListGetFirst (G_inventories[whichInventory].itemslist);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);

        /* get this item structure */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        if (p_inv->objecttype==objectType)
        {
            /* found some of objectType */
            /* destroy them */
            if (p_inv->numitems > numToDestroy)
            {
                p_inv->numitems -= numToDestroy;
                /* change the player's load */
                StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));
                success=TRUE;
                break;
            }
            else if (p_inv->numitems == numToDestroy)
            {
                /* just remove this one */
                if (G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]==element)
                {
                    /* clear mouse hand pointer */
                    InventoryDestroyItemInMouseHand();
                }
                else
                {
                    /* change the player's load */
                    StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));
                    InventoryDestroyElement (element);
                }
                success=TRUE;
                break;
            }
            else
            {
                /* remove this one and keep going */
                numToDestroy-=p_inv->numitems;
                /* change the player's load */
                StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));
                InventoryDestroyElement (element);
            }
        }

        element=nextElement;
    }
    DebugEnd();

    if (BannerFormIsOpen(BANNER_FORM_INVENTORY)) InventoryDrawInventoryWindow(INVENTORY_PLAYER);
    if (BannerFormIsOpen(BANNER_FORM_EQUIPMENT)) InventoryDrawEquipmentWindow();
    InventoryDrawReadyArea();

    return (success);
}




/* this function will equip the player with some basic equipment depending */
/* on the player's class type */
T_void InventorySetDefaultInventoryForClass(T_void)
{
    T_word16 i;
    T_word16 whichFood;
    E_statsClassType classType;
    T_doubleLinkListElement element,nextElement;
    T_inventoryItemStruct *p_inv;
    DebugRoutine ("InventorySetDefaultInventoryForClass");

    /* get the player's class */
    classType=StatsGetPlayerClassType();

    /* give the player some water gourds */
    InventoryAddObjectToInventory (INVENTORY_PLAYER,246,1);

    /* give the player some food */
    for (i=0;i<5;i++)
    {
        whichFood=rand()%8;
        if (whichFood==0)
        {
            /* give em an apple */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,19,1);
        }
        else if (whichFood==1)
        {
            /* berries */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,243,1);
        }
        else if (whichFood==2)
        {
            /* carrot */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,244,1);
        }
        else if (whichFood==3)
        {
            /* cherry */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,245,1);
        }
        else if (whichFood==4)
        {
            /* chicken */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,247,1);
        }
        else if (whichFood==5)
        {
            /* grape */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,248,1);
        }
        else if (whichFood==6)
        {
            /* bannana */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,249,1);
        }
        else
        {
            /* pinapple */
            InventoryAddObjectToInventory (INVENTORY_PLAYER,251,1);
        }
    }


#ifdef COMPILE_OPTION_SHAREWARE_DEMO
    /* Give the player 2 extra medium healing potions */
    InventoryAddObjectToInventory (INVENTORY_PLAYER,801,2);
#endif


    /* Give the player 5 silver */
    StatsAddCoin (COIN_TYPE_GOLD,4);

    /* Give the player 1 Identify All Scroll */
    InventoryAddObjectToInventory (INVENTORY_PLAYER,351,1);


    switch (classType)
    {
        case CLASS_CITIZEN:
             /* 2 healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* short sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,203,1);
             /* leather armor */
//           InventoryAddObjectToInventory (INVENTORY_PLAYER,700,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_KNIGHT:
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,3);
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* long sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,204,1);
             /* chain armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,704,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,705,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,706,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,707,1);
        break;

        case CLASS_MAGE:
             /* armor scrolls */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,809,3);
             /* gain mana */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,362,3);
             /* speed */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,814,3);
             /* Mage runes */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,304,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,305,1);
             /* dagger */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,200,1);

             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_WARLOCK:
             /* gain mana */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,362,1);
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);
             /* Mage runes */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all mage runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,304,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,305,1);
#endif
             /* short sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,203,1);

             /* leather armor */
//           InventoryAddObjectToInventory (INVENTORY_PLAYER,700,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
             break;

        case CLASS_PRIEST:
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,3);
             /* cure poison potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,807,3);
             /* Cleric runes */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,314,1);

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player extra cleric  runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,315,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,316,1);
#endif

             /* mace */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,201,1);

             /* Leather armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_ROGUE:
             /* speed */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,814,2);
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* rogue's tools */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,143,1);
             /* dagger */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,200,1);

             /* leather armor */
//           InventoryAddObjectToInventory (INVENTORY_PLAYER,700,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);

        break;

        case CLASS_ARCHER:
             /* crossbow */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,28,1);
             /* short sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,203,1);

             /* bolts -- special case */
//           InventoryAddObjectToInventory (INVENTORY_PLAYER,206,40);
             Effect(EFFECT_TAKE_AMMO,
                    EFFECT_TRIGGER_NONE,
                    0,
                    80,
                    0,
                    NULL);
             Effect(EFFECT_TAKE_AMMO,
                    EFFECT_TRIGGER_NONE,
                    1,
                    24,
                    0,
                    NULL);
             Effect(EFFECT_TAKE_AMMO,
                    EFFECT_TRIGGER_NONE,
                    3,
                    24,
                    0,
                    NULL);

             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* Leather armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_SAILOR:
             /* strength potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,818,2);
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* leather armor */
//           InventoryAddObjectToInventory (INVENTORY_PLAYER,700,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
             /* long sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,204,1);

        break;

        case CLASS_PALADIN:
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);

             /* Cleric runes */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player extra cleric  runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,314,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,315,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,316,1);
#endif
             /* long sword */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,204,1);
             /* chain armor */
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,704,1);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,705,1);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,706,2);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,707,1);
             /* Leather armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_MERCENARY:
#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all arcane runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,309,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,310,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,311,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,312,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,313,1);
#endif
             /* rogue's tools */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,143,1);
             /* axe */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,205,1);
             /* strength potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,818,2);
             /* chain armor */
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,704,1);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,705,1);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,706,2);
//             InventoryAddObjectToInventory (INVENTORY_PLAYER,707,1);

             /* Leather armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        case CLASS_MAGICIAN:
             /* speed */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,814,1);
             /* healing potions */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,800,2);
             /* gain mana */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,362,2);
             /* Mage Runes */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,300,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,301,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,302,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,303,1);
             /* rogue's tools */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,143,1);

#ifdef COMPILE_OPTION_SHAREWARE_DEMO
             /* Give player all mage runes necessary for demo */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,304,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,305,1);
#endif
             /* dagger */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,200,1);

             /* Leather armor */
             InventoryAddObjectToInventory (INVENTORY_PLAYER,701,1);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,702,2);
             InventoryAddObjectToInventory (INVENTORY_PLAYER,703,1);
        break;

        default:
        /* fail! */
        DebugCheck(0);
        break;
    }

    InventoryReorder(INVENTORY_PLAYER,FALSE);

    /* iterate through the player's inventory and equip anything that */
    /* needs to be equipped */

    G_inventoryLocations[EQUIP_LOCATION_LEFT_ARM]=DOUBLE_LINK_LIST_ELEMENT_BAD;

    element=DoubleLinkListGetFirst(G_inventories[INVENTORY_PLAYER].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement=DoubleLinkListElementGetNext(element);
        /* get an item */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);

        switch (p_inv->itemdesc.type)
        {
            case EQUIP_OBJECT_TYPE_WEAPON:
            /* ready this weapon */
            if (InventoryObjectIsInReadyHand()==FALSE)
            {
                G_inventoryLocations[EQUIP_LOCATION_READY_HAND]=p_inv->elementID;
                /* move it off the inventory display page */
                p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                /* if there is a 'ready' effect for the item do it now */
                InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_READY_HAND);
            }
            break;

            case EQUIP_OBJECT_TYPE_THING:
            /* trigger take effect (used to equip runes) */
            InventoryDoItemEffect (p_inv,EFFECT_TRIGGER_GET);
            break;

            case EQUIP_OBJECT_TYPE_ARMOR:
            /* equip this armor */
            /* figure out which 'type' of armor this is */
            switch (p_inv->itemdesc.subtype)
            {
                case EQUIP_ARMOR_TYPE_BRACING_CLOTH:
                case EQUIP_ARMOR_TYPE_BRACING_CHAIN:
                case EQUIP_ARMOR_TYPE_BRACING_PLATE:
                /* put it on a free arm */
                if (G_inventoryLocations[EQUIP_LOCATION_LEFT_ARM]==DOUBLE_LINK_LIST_ELEMENT_BAD)
                {
//                    printf ("on left arm\n");
                    fflush (stdout);
                    /* put it on the left arm */
                    G_inventoryLocations[EQUIP_LOCATION_LEFT_ARM]=p_inv->elementID;
                    InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_LEFT_ARM);
                    InventorySetPlayerColor(EQUIP_LOCATION_LEFT_ARM);
                }
                else
                {
//                    printf ("on right arm\n");
                    fflush (stdout);
                    /* put it on the right arm */
                    G_inventoryLocations[EQUIP_LOCATION_RIGHT_ARM]=p_inv->elementID;
                    InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_RIGHT_ARM);
                    InventorySetPlayerColor(EQUIP_LOCATION_RIGHT_ARM);
                }
                /* hide it in the inventory */
                p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                break;

                case EQUIP_ARMOR_TYPE_LEGGINGS_CLOTH:
                case EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN:
                case EQUIP_ARMOR_TYPE_LEGGINGS_PLATE:
                /* put it on the legs */
                G_inventoryLocations[EQUIP_LOCATION_LEGS]=p_inv->elementID;
                InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_LEGS);
                InventorySetPlayerColor(EQUIP_LOCATION_LEGS);
                /* hide it in the inventory */
                p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                break;

                case EQUIP_ARMOR_TYPE_HELMET_CLOTH:
                case EQUIP_ARMOR_TYPE_HELMET_CHAIN:
                case EQUIP_ARMOR_TYPE_HELMET_PLATE:
                /* put it on the head */
                G_inventoryLocations[EQUIP_LOCATION_HEAD]=p_inv->elementID;
                InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_HEAD);
                InventorySetPlayerColor(EQUIP_LOCATION_HEAD);
                /* hide it in the inventory */
                p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                break;

                case EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH:
                case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                /* put it on the chest */
                G_inventoryLocations[EQUIP_LOCATION_CHEST]=p_inv->elementID;
                InventoryDoEffect(EFFECT_TRIGGER_READY,EQUIP_LOCATION_CHEST);
                InventorySetPlayerColor(EQUIP_LOCATION_CHEST);
                /* hide it in the inventory */
                p_inv->storepage=INVENTORY_OFF_INVENTORY_PAGE;
                break;

                default:
                DebugCheck(0); /* fail !*/
                break;
            }
            break;

            default:
            /* ignore other things */
            break;
        }
        element=nextElement;
    }

    /* Give player starting Journal Entries */
    /* turn off effect sounds temporarily so that we don't */
    /* hear 100 page flips simultaneously */
    EffectSoundOff();
    /* Add 'intro' pages */
    for (i=0;i<23;i++) StatsAddPlayerNotePage (i);

    /* Add 'shpeel' page */
//    StatsAddPlayerNotePage (98);
    /* Add 'spell group listing' page */
    StatsAddPlayerNotePage(99);

#ifndef COMPILE_OPTION_SHAREWARE_DEMO
    /* spells for regular version */
    if (StatsGetPlayerSpellSystem() == SPELL_SYSTEM_MAGE)
    {
        StatsAddPlayerNotePage(100); /* Deflect */
        StatsAddPlayerNotePage(101); /* Magic Dart */
        StatsAddPlayerNotePage(102); /* Feather Fall */
        StatsAddPlayerNotePage(104); /* Disorient */
        StatsAddPlayerNotePage(105); /* Attract */
        StatsAddPlayerNotePage(106); /* Wizard lock */
        StatsAddPlayerNotePage(109); /* Repulse */
        StatsAddPlayerNotePage(110); /* Angry word */
        StatsAddPlayerNotePage(111); /* Force Door */
    }
    else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_CLERIC)
    {
        StatsAddPlayerNotePage(300); /* Rejuvinate */
        StatsAddPlayerNotePage(305); /* Confuse */
        StatsAddPlayerNotePage(306); /* Knock */
        StatsAddPlayerNotePage(307); /* Cast of Sand */
        StatsAddPlayerNotePage(308); /* Attract */
        StatsAddPlayerNotePage(309); /* Repel */
    }
    else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_ARCANE)
    {
        StatsAddPlayerNotePage(202); /* Cast of Sand */
        StatsAddPlayerNotePage(204); /* Pull */
        StatsAddPlayerNotePage(206); /* Push */
        StatsAddPlayerNotePage(208); /* Knock */
    }
#else
    /* buy me note */
    StatsAddPlayerNotePage(98);
    /* Spells for shareware demo */
    if (StatsGetPlayerSpellSystem() == SPELL_SYSTEM_MAGE)
    {
        StatsAddPlayerNotePage(100); /* Deflect */
        StatsAddPlayerNotePage(101); /* Magic Dart */
        StatsAddPlayerNotePage(103); /* Magic Missile */
        StatsAddPlayerNotePage(104); /* Disorient */
        StatsAddPlayerNotePage(105); /* Attact */
        StatsAddPlayerNotePage(109); /* Repulse */
        StatsAddPlayerNotePage(111); /* Force Door */
        StatsAddPlayerNotePage(113); /* Magic Map */
        StatsAddPlayerNotePage(121); /* Enhance Vision */
    }
    else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_CLERIC)
    {
        StatsAddPlayerNotePage(300); /* Rejuvinate */
        StatsAddPlayerNotePage(301); /* Waterwalk */
        StatsAddPlayerNotePage(302); /* Purify Blood */
        StatsAddPlayerNotePage(303); /* Resist Impact */
//      StatsAddPlayerNotePage(304); /* Earthsmite */
        StatsAddPlayerNotePage(306); /* Knock */
        StatsAddPlayerNotePage(308); /* Attract */
        StatsAddPlayerNotePage(309); /* Repel */
        StatsAddPlayerNotePage(310); /* Magic Map */
        StatsAddPlayerNotePage(317); /* Glow */
        StatsAddPlayerNotePage(318); /* Life Water */
        StatsAddPlayerNotePage(320); /* Wolf Speed */
    }
    else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_ARCANE)
    {
        StatsAddPlayerNotePage(202); /* Cast of Sand */
        StatsAddPlayerNotePage(204); /* Pull */
        StatsAddPlayerNotePage(206); /* Push */
        StatsAddPlayerNotePage(208); /* Knock */
        StatsAddPlayerNotePage(209); /* Magic Map */
        StatsAddPlayerNotePage(214); /* Glow */
    }
#endif

    EffectSoundOn();

    /* update equip location colors */
    for (i=0;i<=EQUIP_LOCATION_LEGS;i++) InventorySetPlayerColor(i);

    /* identify all inventory items */
    InventoryIdentifyAll();

    DebugEnd();
}


/* Routine identifies all equipped items */
T_void InventoryIdentifyReadied (T_void)
{
    T_word16 i;
    T_inventoryItemStruct *p_inv;
    T_word16 objnum;
    E_Boolean hasId=FALSE;
    DebugRoutine ("InventoryIdentifyReadied");
    for (i=0;i<EQUIP_LOCATION_UNKNOWN;i++)
    {
        if (G_inventoryLocations[i]!=DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* get this item */
            p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(
                G_inventoryLocations[i]);

            objnum=ObjectGetType(p_inv->object);

            if (StatsPlayerHasIdentified(objnum)==FALSE)
            {
                hasId=TRUE;
                StatsPlayerIdentify(objnum);
            }
        }
    }
    DebugEnd();

//    if (hasId==TRUE) MessageAdd ("^009You have gained insight about your equipment!");

}


/* Routine identifies all items in inventory */
T_void InventoryIdentifyAll (T_void)
{
    T_doubleLinkListElement element;
    T_inventoryItemStruct *p_inv;
    T_word16 objnum;
    E_Boolean hasId=FALSE;
    DebugRoutine ("InventoryIdentifyAll");

    /* loop through player's inventory */
    element=DoubleLinkListGetFirst(G_inventories[INVENTORY_PLAYER].itemslist);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        objnum=ObjectGetType(p_inv->object);
        if (StatsPlayerHasIdentified(objnum)==FALSE)
        {
            hasId=TRUE;
            StatsPlayerIdentify(objnum);
        }
        element=DoubleLinkListElementGetNext(element);
    }

//    if (hasId==TRUE) MessageAdd ("^009You have gained insight about your equipment!");
//    else MessageAdd ("^005Nothing seemed to happen");

    DebugEnd();
}


T_void InventoryResetUse (T_void)
{
    G_useResetNeeded=FALSE;
}

T_void InventoryGoThroughAll(T_inventoryGoThroughAllCallback callback)
{
    E_Boolean destroyme;
    T_inventoryItemStruct *p_inv;
    T_doubleLinkListElement nextElement ;
    T_doubleLinkListElement element ;
    T_word16 equipLocation;
    E_Boolean isEquipped ;
    E_Boolean equipDestroyed = FALSE ;
    E_Boolean invDestroyed = FALSE ;

    DebugRoutine ("InventoryGoThroughAll");
    DebugCheck(callback != NULL) ;

    /* Iterate through the whole player inventory. */
    element=DoubleLinkListGetFirst (G_inventories[INVENTORY_PLAYER].itemslist);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Get the next element just in case this one is destroyed. */
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* get pointer to invstruct */
        p_inv=(T_inventoryItemStruct *)DoubleLinkListElementGetData(element);
        DebugCheck(p_inv != NULL);
//printf("p_inv=%p, p_inv->obj=%p\n", p_inv, p_inv->object) ;
        DebugCheck(p_inv->object != NULL) ;
        DebugCheck(ObjectIsValid(p_inv->object)) ;

        /* Call per item in equipment */
        destroyme = callback(p_inv->numitems, p_inv->objecttype, ObjectGetAccData(p_inv->object)) ;

        /* Check if request to destroy this item. */
        if (destroyme == TRUE)  {
            /* Yep, destroy it. */

            /* Determine if and where this item is equipped */
            isEquipped = FALSE ;
            for (equipLocation=EQUIP_LOCATION_READY_HAND;
                   equipLocation<EQUIP_NUMBER_OF_LOCATIONS;
                     equipLocation++)  {
                if (G_inventoryLocations[equipLocation] == element)  {
                    isEquipped = TRUE ;
                    break ;
                }
            }

            /* JDA - Call 'drop' effect for item if necessary */
            InventoryDoItemEffect (p_inv, EFFECT_TRIGGER_DROP);


            /* If it is equipped, unready the item and */
            /* delete it from the equipped locations. */
            if (isEquipped)  {
                /* check for 'unequip effect' */
                if (equipLocation != EQUIP_LOCATION_MOUSE_HAND)
                    InventoryDoEffect(EFFECT_TRIGGER_UNREADY,equipLocation);
                G_inventoryLocations[equipLocation]=DOUBLE_LINK_LIST_ELEMENT_BAD;
                equipDestroyed = TRUE ;
            } else {
                invDestroyed = TRUE ;
            }

            /* subtract weight */
//            StatsChangePlayerLoad (-ObjectGetWeight(p_inv->object));

            /* remove item */
            InventoryDestroyElement(element);

            if (equipLocation==EQUIP_LOCATION_READY_HAND)
            {
                /* redraw ready area */
                InventoryDrawReadyArea();
            }

            /* set the color for the equipped/unequipped location */
            if ((equipLocation>=EQUIP_LOCATION_HEAD) &&
                (equipLocation<=EQUIP_LOCATION_SHIELD_HAND))
               InventorySetPlayerColor(equipLocation);
        }

        element = nextElement ;
    }

    /* if equip window is open and equipment changed redraw */
    if ((BannerFormIsOpen (BANNER_FORM_EQUIPMENT)) && (equipDestroyed))
        InventoryDrawEquipmentWindow();

    /* redraw inventory screen if open (load change) and inv changed */
    if ((BannerFormIsOpen (BANNER_FORM_INVENTORY)) && (invDestroyed))
        InventoryDrawInventoryWindow(INVENTORY_PLAYER);

    /* redraw banner status display */
    if ((equipDestroyed) || (invDestroyed))
        BannerStatusBarUpdate();

    DebugEnd();
}



E_Boolean InventoryCheckClassCanUseWeapon (T_inventoryItemStruct *p_inv,
                                           E_Boolean showMessage)
{
    E_Boolean canUse=TRUE;
    E_statsClassType ourclass=CLASS_UNKNOWN;

    DebugRoutine ("InventoryCheckClassCanUseWeapon");

    ourclass=StatsGetPlayerClassType();

    DebugCheck (ourclass < CLASS_UNKNOWN);
    DebugCheck (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_WEAPON);

    switch (p_inv->itemdesc.subtype)
    {
        case EQUIP_WEAPON_TYPE_DAGGER:
        if (ourclass==CLASS_PRIEST)
        {
            if (showMessage==TRUE) MessageAdd ("You aren't allowed to use daggers.");
            canUse=FALSE;
        }
        break;

        case EQUIP_WEAPON_TYPE_AXE:
        if (ourclass==CLASS_MAGE ||
            ourclass==CLASS_PRIEST ||
            ourclass==CLASS_MAGICIAN ||
            ourclass==CLASS_ROGUE)
        {
            if (showMessage==TRUE) MessageAdd ("^005You aren't proficient with axes.");
            canUse=FALSE;
        }
        break;

        case EQUIP_WEAPON_TYPE_LONGSWORD:
        if (ourclass==CLASS_ROGUE ||
            ourclass==CLASS_MAGICIAN)
        {
            if (showMessage==TRUE) MessageAdd("^005You aren't proficient with longswords.");
            canUse=FALSE;
        }

        case EQUIP_WEAPON_TYPE_SHORTSWORD:
        if (ourclass==CLASS_MAGE ||
            ourclass==CLASS_PRIEST)
        {
            if (showMessage==TRUE) MessageAdd("^005You aren't proficient with swords.");
            canUse=FALSE;
        }
        break;

        case EQUIP_WEAPON_TYPE_MACE:
        if (ourclass==CLASS_MAGE ||
            ourclass==CLASS_MERCENARY ||
            ourclass==CLASS_MAGICIAN)
        {
            if (showMessage==TRUE) MessageAdd ("^005You aren't proficient with maces.");
            canUse=FALSE;
        }
        break;

        case EQUIP_WEAPON_TYPE_CROSSBOW:
        if (ourclass==CLASS_MAGE ||
            ourclass==CLASS_WARLOCK ||
            ourclass==CLASS_PRIEST ||
            ourclass==CLASS_PALADIN ||
            ourclass==CLASS_MAGICIAN)
        {
            if (showMessage==TRUE) MessageAdd ("^005You aren't proficient with bows.");
            canUse=FALSE;
        }
        break;

        default:
        break;
    }

    DebugEnd();

    return (canUse);
}


E_Boolean InventoryCheckClassCanUseArmor (T_inventoryItemStruct *p_inv,
                                          E_Boolean showMessage)
{
    E_Boolean canUse=TRUE;
    E_statsClassType ourclass=CLASS_UNKNOWN;

    DebugRoutine ("InventoryCheckClassCanUseArmor");

    ourclass=StatsGetPlayerClassType();

    DebugCheck (ourclass < CLASS_UNKNOWN);
    DebugCheck (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_ARMOR);

    /* check armor allowances */
    if (p_inv->itemdesc.type==EQUIP_OBJECT_TYPE_ARMOR)
    {
        switch (p_inv->itemdesc.subtype)
        {
            case EQUIP_ARMOR_TYPE_LEGGINGS_PLATE:
            case EQUIP_ARMOR_TYPE_BRACING_PLATE:
            case EQUIP_ARMOR_TYPE_HELMET_PLATE:
            case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
            if (ourclass==CLASS_WARLOCK ||
                ourclass==CLASS_PRIEST ||
                ourclass==CLASS_ARCHER ||
                ourclass==CLASS_SAILOR ||
                ourclass==CLASS_MAGE ||
                ourclass==CLASS_ROGUE ||
                ourclass==CLASS_MAGICIAN)
            {
                if (showMessage==TRUE) MessageAdd ("^005You can't wear heavy armor");
                canUse=FALSE;
            }
            break;

            case EQUIP_ARMOR_TYPE_BRACING_CHAIN:
            case EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN:
            case EQUIP_ARMOR_TYPE_HELMET_CHAIN:
            case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
            if (ourclass==CLASS_MAGE ||
                ourclass==CLASS_ROGUE ||
                ourclass==CLASS_MAGICIAN)
            {
                if (showMessage==TRUE) MessageAdd ("^005That armor is too heavy for you to wear.");
                canUse=FALSE;
            }
            break;

            default:
            break;
        }
    }


    DebugEnd();

    return (canUse);
}


E_Boolean InventoryCreateObjectInHand(T_word16 itemTypeNum)
{
    T_inventoryItemStruct *p_inv;
    E_Boolean retvalue=FALSE;
    T_3dObject *item ;

    DebugRoutine ("InventoryCreateObjectInHand");
    DebugCheck(itemTypeNum != 0) ;

    /* Do if nothing in mouse hand and item is real */
    if ((itemTypeNum != 0) && (InventoryObjectIsInMouseHand() == FALSE))  {
        item = ObjectCreateFake() ;
        /* valid item? */
        if (item != NULL)  {
            /* Make correct type */
            ObjectSetType(item, itemTypeNum) ;

            p_inv=InventoryTakeObject (INVENTORY_PLAYER,item);

            /* set 'quick pointer' for mouse hand */
            G_inventoryLocations[EQUIP_LOCATION_MOUSE_HAND]=p_inv->elementID;

            /* force mouse hand picture update */
            ControlSetObjectPointer (p_inv->object);

            if (InventoryObjectIsInMouseHand()==TRUE)
            {
                /* item successfully taken */

                /* check for 'get effect' */
                InventoryDoEffect(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);

                /* check for 'destroy on get' indicator */
                if (InventoryShouldDestroyOn(EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND)==TRUE)
                {
                    /* reset the mouse pointer to default */
                    ControlPointerReset();

                    /* destroy object */
                    InventoryDestroyOn (EFFECT_TRIGGER_GET,EQUIP_LOCATION_MOUSE_HAND);

                }

                /* Tell the caller that this is a good object. */
                retvalue=TRUE;
            } else {
                /* Don't leak the memory. */
                ObjectDestroy(item) ;
            }
        }
    }

    DebugEnd();
    return (retvalue);
}


/* Routine returns true if current weapon in hand is a bow */
E_Boolean InventoryWeaponIsBow (T_void)
{
    E_Boolean isBow=FALSE;
    T_3dObject *p_obj;
    DebugRoutine ("InventoryWeaponIsBow");

    p_obj=InventoryCheckObjectInReadyHand();
    if (p_obj) {
        if (ObjectIsWeapon(p_obj) &&
            ObjectGetBasicType(p_obj)==OBJECT_TYPE_XBOW)
        {
            isBow=TRUE;
        }
    }

    DebugEnd();
    return (isBow);
}


/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  INVENTOR.C
 *-------------------------------------------------------------------------*/
