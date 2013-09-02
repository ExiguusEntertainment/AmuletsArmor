/****************************************************************************/
/*    FILE:  PEOPHERE.H                                                     */
/****************************************************************************/
#ifndef _PEOPHERE_H_
#define _PEOPHERE_H_

#include "GENERAL.H"
#include "SERVER.H"

T_void PeopleHereInitialize(T_void) ;
T_void PeopleHereFinish(T_void) ;
T_void PeopleHereReset(T_void) ;
T_void PeopleHereUpdatePlayer(T_playerIDSelf *p_playerID) ;
E_Boolean PeopleHereFindPlayerGame(
              T_byte8 *p_name,
              T_gameGroupID *p_groupID) ;
T_void PeopleHereGetPlayerIDSelfStruct(T_playerIDSelf *p_self) ;
T_gameGroupID PeopleHereGetUniqueGroupID(T_void) ;

T_void PeopleHereSetOurState(T_playerIDState state) ;
T_playerIDState PeopleHereGetOurState(T_void) ;
T_void PeopleHereSetOurAdventure(T_word16 adventure) ;
T_word16 PeopleHereGetOurAdventure(T_void) ;

T_void PeopleHereGeneratePeopleInGame(T_gameGroupID groupID) ;

T_playerIDLocation PeopleHereGetOurLocation(T_void) ;

T_void PeopleHereStartGame(T_word16 firstLevel) ;

T_void PeopleHereIDPlayer(T_word16 playerNum, T_byte8 *p_name) ;
T_void PeopleHereResetPlayerIDs(T_void) ;
T_byte8 *PeopleHereGetPlayerIDName(T_word16 playerNum) ;

T_directTalkUniqueAddress *PeopleHereGetUniqueAddr(T_word16 playerNum) ;
T_void PeopleHereSetUniqueAddr(
           T_word16 playerNum,
           T_directTalkUniqueAddress *uaddr) ;
T_void PeopleHereRequestJoin(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure) ;
T_void PeopleHereRespondToJoin(
           T_directTalkUniqueAddress uniqueAddress,
           T_gameGroupID groupID,
           T_word16 adventure,
           E_respondJoin response);
T_word16 PeopleHereGetNumInGroupGame(void);
#endif

/****************************************************************************/
/*    END OF FILE:  PEOPHERE.H                                              */
/****************************************************************************/
