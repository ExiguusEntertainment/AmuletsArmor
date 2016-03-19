#ifndef _TOWNUI_H_
#define _TOWNUI_H_

#include "GENERAL.H"

T_void TownUIStart    (T_word32 formNum);
T_void TownUIUpdate   (T_void);
T_void TownUIEnd      (T_void);
T_void TownAddPerson  (T_byte8 *personName);
T_void TownRemovePerson (T_byte8 *personName);
E_Boolean TownUIIsOpen  (T_void);
T_void TownUISetAdventureCompleted(T_void);
T_void TownUIAddMessage (T_byte8 *playerName, T_byte8 *message);
E_Boolean TownUICompletedMapLevel(T_word16 mapLevel) ;
E_Boolean TownUIFinishedQuest(T_word16 multiplayerStatus, T_byte8 numPlayers, T_word16 currentQuest);
E_Boolean TownPersonInChat(T_byte8 *personName);

#endif
