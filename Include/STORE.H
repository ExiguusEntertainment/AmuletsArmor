#ifndef _STORE_H_
#define _STORE_H_

#include "EQUIP.H"
#include "GENERAL.H"
#include "INVENTOR.H"
#include "MOUSEMOD.H"

typedef T_void (*T_storeCallback)(T_void);

T_void StoreSetUpInventory(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_word16 gridspacesx,
        T_word16 gridspacesy);

T_void StoreCloseInventory(T_void);

T_void StoreAddItem(T_word16 objectToAdd, T_word16 numberToAdd);
T_void StoreRemoveItem(T_word16 objectToRemove, T_word16 numberToRemove);

T_void StoreHandleTransaction(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick buttons);

T_void StoreRequestSellItem(T_inventoryItemStruct *p_inv);
T_void StoreConfirmSellItem(T_void);
T_void StoreRequestBuyItem(T_inventoryItemStruct *p_item);
T_void StoreConfirmBuyItem(T_void);

T_void StoreSetControlCallback(T_storeCallback callback);

T_void StoreUIStart(T_word32 formNum);
T_void StoreUIUpdate(T_void);
T_void StoreUIEnd(T_void);
T_void StoreAddBuyType(E_equipObjectTypes type);
E_Boolean StoreWillBuy(E_equipObjectTypes type);
T_void StoreSetHouseModeOn(T_void);
E_Boolean StoreHouseModeIsOn(T_void);
E_Boolean StoreIsOpen(T_void);
T_void StoreConvertCurrencyToString(T_byte8* tempString, T_word32 amount);
T_word16 StoreGetBuyValue(T_inventoryItemStruct *p_inv);

#endif
