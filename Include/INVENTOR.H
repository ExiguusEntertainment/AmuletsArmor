/****************************************************************************/
/*    FILE:  INVENTORY.H                                                    */
/****************************************************************************/

#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include "3D_VIEW.H"
#include "BUTTON.H"
#include "DBLLINK.H"
#include "EQUIP.H"
#include "GENERAL.H"
#include "GRAPHICS.H"

#define INVENTORY_GRID_WIDTH 17
#define INVENTORY_GRID_HEIGHT 14
#define INVENTORY_BASE_COLOR 76
#define INVENTORY_GRID_COLOR 79
#define INVENTORY_HIGHLIGHT_COLOR 73
#define INVENTORY_TEXT_COLOR 144

#define INVENTORY_OFF_INVENTORY_PAGE 255
#define MAX_INVENTORY_SIZE 100

#define STORE_PAGE_TYPE_DISPLAY_X1 10
#define STORE_PAGE_TYPE_DISPLAY_Y1 126
#define STORE_PAGE_TYPE_DISPLAY_X2 62
#define STORE_PAGE_TYPE_DISPLAY_Y2 133
#define STORE_PAGE_NUMBER_DISPLAY_X1  84
#define STORE_PAGE_NUMBER_DISPLAY_Y1  126
#define STORE_PAGE_NUMBER_DISPLAY_X2  112
#define STORE_PAGE_NUMBER_DISPLAY_Y2  133
#define STORE_LAST_PAGE_BUTTON_X1  64
#define STORE_LAST_PAGE_BUTTON_Y1  126
#define STORE_NEXT_PAGE_BUTTON_X1  74
#define STORE_NEXT_PAGE_BUTTON_Y1  126

typedef T_void *T_inventoryItemID;
typedef T_void (*T_inventoryCallbackHandler)(T_inventoryItemID inventoryItemID);

typedef enum {
    INVENTORY_PLAYER, INVENTORY_STORE, INVENTORY_UNKNOWN
} E_inventoryType;

typedef struct {
    /* x and y location of inventory display */
    T_word16 locx1;
    T_word16 locy1;
    T_word16 locx2;
    T_word16 locy2;

    /* width and height of inventory display (in grid squares */
    T_byte8 gridspacesx;
    T_byte8 gridspacesy;

    /* current number of pages for this inventory */
    T_byte8 numpages;

    /* maximum number of pages allowed for this inventory (0=no limit) */
    T_byte8 maxpages;

    /* current visible page of this inventory */
    T_byte8 curpage;

    /* linked list containing current items in inventory */
    T_doubleLinkList itemslist;
} T_inventoryStruct;

typedef struct {
    /* x and y of current bitmap location */
    T_word16 locx;
    T_word16 locy;

    /* width and height of bitmap */
    T_word16 picwidth;
    T_word16 picheight;

    /* location of unit in inventory storage grid */
    T_sbyte8 gridstartx;
    T_sbyte8 gridstarty;

    /* number of inventory grid spaces this item takes up */
    T_sbyte8 gridspacesx;
    T_sbyte8 gridspacesy;

    /* pointer to stored object */
    T_3dObject *object;

    /* type number of stored object */
    T_word32 objecttype;

    /* pointer to stored object bitmap */
    T_bitmap *p_bitmap; //pointer to bitmap picture

    /* inventory page number stored on */
    T_byte8 storepage; //inventory page stored on

    /* number of objects of this type stored at this location */
    T_byte8 numitems; //number of objects stored at this location

    /* item description file (item effects, type, ect) */
    T_equipItemDescription itemdesc;

    /* double link list handler */
    T_doubleLinkListElement elementID;

} T_inventoryItemStruct;

/* global routine prototypes */
T_void InventoryInit(T_void);
T_void InventoryFinish(T_void);

T_void InventorySetActiveInventory(E_inventoryType active);

T_void InventorySelectNextInventoryPage(E_inventoryType which);
T_void InventorySelectLastInventoryPage(E_inventoryType which);
T_void InventoryFindInventoryItemPage(
        E_inventoryType which,
        T_inventoryItemStruct *thisitem);
E_Boolean InventoryInventoryWindowIsAt(T_word16 locx, T_word16 locy);
E_inventoryType InventoryFindInventoryWindow(T_word16 locx, T_word16 locy);
E_Boolean InventoryReadyBoxIsAt(T_word16 locx, T_word16 locy);
E_Boolean InventoryEquipmentWindowIsAt(T_word16 locx, T_word16 locy);
E_Boolean InventoryObjectIsInMouseHand(T_void);
E_Boolean InventoryObjectIsInReadyHand(T_void);
T_3dObject* InventoryCheckObjectInMouseHand(T_void);
T_inventoryItemStruct* InventoryCheckItemInMouseHand(T_void);
T_3dObject* InventoryCheckObjectInReadyHand(T_void);
T_inventoryItemStruct* InventoryCheckItemInReadyHand(T_void);
T_3dObject* InventoryCheckObjectInInventoryArea(T_word16 x, T_word16 y);
T_3dObject* InventoryCheckObjectInEquipmentArea(T_word16 x, T_word16 y);
T_inventoryItemStruct* InventoryCheckItemInInventoryArea(T_word16 x, T_word16 y);
E_Boolean InventoryCanUseItemInReadyHand(T_void);
T_void InventoryUseItemInReadyHand(T_buttonID buttonID);
E_Boolean InventoryCanUseItem(T_inventoryItemStruct *p_inv);
T_void InventoryUseItem(T_inventoryItemStruct *p_inv);

E_Boolean InventoryCanTakeItem(T_3dObject *item);
E_Boolean InventoryTakeItemFromWorld(T_3dObject *item, E_Boolean autoStore);
T_void InventoryAutoTakeObject(T_3dObject *p_obj);
T_inventoryItemStruct* InventoryTakeObject(
        E_inventoryType which,
        T_3dObject *item);
E_Boolean InventoryThrowObjectIntoWorld(T_word16 x, T_word16 y);

T_void InventoryTransferToReadyHand(T_void);
T_void InventoryTransferToInventory(T_word16 x, T_word16 y);
T_void InventoryTransferToEquipment(T_word16 x, T_word16 y);
T_void InventoryTransferToFinances(T_void);
T_void InventoryTransferToAmmo(T_void);

T_void InventoryDrawInventoryWindow(E_inventoryType which);
T_void InventoryDrawEquipmentWindow(T_void);
T_void InventoryDrawReadyArea(T_void);
T_void InventoryReorder(E_inventoryType which, E_Boolean multipage);

E_equipArmorTypes InventoryGetArmorType(E_equipLocations location);

T_word16 InventoryDestroyRandomStoredItem(T_void);
T_word16 InventoryDestroyRandomEquippedItem(T_void);

E_Boolean InventoryIsUseableByClass(T_inventoryItemStruct *p_inv);

T_void InventoryPlayWeaponAttackSound(T_void);
T_void InventoryPlayWeaponHitSound(T_void);

T_void InventoryReadItemsList(FILE *fp);
T_void InventoryWriteItemsList(FILE *fp);

/* LES: 03/28/96  Routine to update all the weapons and items */
/* on a character. */
T_void InventoryUpdatePlayerEquipmentBodyParts(T_void);

T_void InventorySetInventoryWindowLocation(
        E_inventoryType type,
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_byte8 gx,
        T_byte8 gy,
        T_byte8 maxpages);

T_void InventoryMoveInventoryWindowLocation(
        E_inventoryType type,
        T_word16 dx,
        T_word16 dy);

T_void InventoryAddObjectToInventory(
        E_inventoryType which,
        T_word16 objID,
        T_byte8 numToAdd);

T_void InventoryRemoveObjectFromInventory(
        E_inventoryType which,
        T_word16 objID,
        T_byte8 numToRemove);

T_void InventoryDebugDump(E_inventoryType which);

T_doubleLinkListElement InventoryTransferItemBetweenInventories(
        T_inventoryItemStruct *p_inv,
        E_inventoryType source,
        E_inventoryType dest);

T_void InventorySetMouseHandPointer(T_doubleLinkListElement element);
T_void InventoryClearObjectInMouseHand(T_void);
T_void InventorySetObjectInMouseHand(T_doubleLinkListElement element);
T_void InventoryDestroyItemInMouseHand(T_void);
T_void InventoryAutoStoreItemInMouseHand(T_void);
E_Boolean InventoryAutoEquipItemInMouseHand(T_void);

T_void InventoryOpenStoreInventory(T_void);
T_void InventoryCloseStoreInventory(T_void);
T_void InventoryClear(E_inventoryType which);
T_void InventoryRemoveEquippedEffects(T_void);
T_void InventoryRestoreEquippedEffects(T_void);

T_word16 InventoryHasItem(E_inventoryType whichInventory, T_word16 objectType);
E_Boolean InventoryDestroySpecificItem(
        E_inventoryType whichInventory,
        T_word16 objectType,
        T_word16 numToDestroy);

T_void InventorySetDefaultInventoryForClass(T_void);

E_Boolean InventoryDestroyOn(
        E_effectTriggerType trigger,
        E_equipLocations location);
E_Boolean InventoryShouldDestroyOn(
        E_effectTriggerType trigger,
        E_equipLocations location);
T_void InventoryIdentifyReadied(T_void);
T_void InventoryIdentifyAll(T_void);
T_void InventoryResetUse(T_void);

/* Return TRUE to destroy */
typedef E_Boolean (*T_inventoryGoThroughAllCallback)(
        T_word16 num,
        T_word16 type,
        T_word16 accData);

T_void InventoryGoThroughAll(T_inventoryGoThroughAllCallback callback);
E_Boolean InventoryCheckClassCanUseWeapon(
        T_inventoryItemStruct *p_inv,
        E_Boolean showMessage);
E_Boolean InventoryCheckClassCanUseArmor(
        T_inventoryItemStruct *p_inv,
        E_Boolean showMessage);
E_Boolean InventoryCreateObjectInHand(T_word16 itemTypeNum);
E_Boolean InventoryWeaponIsBow(T_void);
E_Boolean InventoryDoEffect(
        E_effectTriggerType trigger,
        E_equipLocations location);

#endif

