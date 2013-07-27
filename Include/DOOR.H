/****************************************************************************/
/*    FILE:  DOOR.H                                                         */
/****************************************************************************/
#ifndef _DOOR_H_
#define _DOOR_H_

#include "GENERAL.H"

#define DOOR_ABSOLUTE_LOCK       0xFFFF

typedef T_void *T_doorGroupHandle ;
#define DOOR_GROUP_HANDLE_BAD    NULL

T_void DoorInitialize(T_void) ;

T_void DoorFinish(T_void) ;

T_void DoorLoad(T_word32 mapNumber) ;

T_void DoorLock(T_word16 sector) ;

T_void DoorUnlock(T_word16 sector) ;

E_Boolean DoorIsLock(T_word16 sector) ;

E_Boolean DoorOpen(T_word16 sector) ;

T_void DoorClose(T_word16 sector) ;

T_void DoorForceOpen(T_word16 sector) ;

E_Boolean DoorIsAtSector(T_word16 sector) ;

T_word16 DoorGetPercentOpen(T_word16 sector) ;

T_void DoorDecreaseLock(T_word16 sector, T_word16 amount) ;

T_void DoorIncreaseLock(T_word16 sector, T_word16 amount) ;

T_word16 DoorGetLockValue(T_word16 sector) ;

T_void DoorCreate(
           T_word16 sector,
           T_sword16 floor,
           T_sword16 ceiling,
           T_word16 timeOpen,
           T_word16 timePause,
           T_word16 lockLevel,
           T_sword16 soundX,
           T_sword16 soundY) ;

T_void DoorSetRequiredItem(T_word16 doorSector, T_word16 itemType) ;

T_word16 DoorGetRequiredItem(T_word16 doorSector) ;

#endif

/****************************************************************************/
/*    END OF FILE:  DOOR.H                                                  */
/****************************************************************************/
