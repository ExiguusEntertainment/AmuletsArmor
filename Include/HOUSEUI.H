#ifndef _HOUSEUI_H_
#define _HOUSEUI_H_

#include "BUTTON.H"
#include "GENERAL.H"

#define MIN_HOUSE_SALE_VALUE 100
#define MAX_HOUSE_SALE_VALUE 10000

T_void HouseUIStart    (T_word32 houseNum);

T_void HouseUIUpdate   (T_void);

T_void HouseUIEnd      (T_void);


T_void HouseRequestInfoPackage(T_word32 houseNum);

T_void HouseSetup (T_byte8   *p_ownerName,
                   E_Boolean isForSale,
                   T_word32  saleValue,
                   E_Boolean isOccupied);

T_void HouseRequestHouseInventory(T_void);

T_void HouseChangeStatus (T_buttonID buttonID);

#endif
