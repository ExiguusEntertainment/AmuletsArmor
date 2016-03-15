#ifndef _GUILDUI_H_
#define _GUILDUI_H_

#include "BUTTON.H"
#include "GENERAL.H"
#include "SERVER.H"

typedef enum
{
    GUILD_GAME_LIST,
    GUILD_GAME_DESC,
    GUILD_MAPS_LIST,
    GUILD_MAPS_DESC
} E_guildWindowNames;

typedef struct
{
    T_byte8 *name;
    T_byte8 *description;
    T_word16 mapKey;
    T_word16 mapIndex;
} T_mapDescriptionStruct;

typedef struct
{
    T_word16 mapNumber;
    T_gameGroupID groupID ;
	T_word16 questNumber;
} T_gameDescriptionStruct;

T_void GuildUIStart    (T_word32 formNum);
T_void GuildUIUpdate   (T_void);
T_void GuildUIEnd      (T_void);

T_void GuildUIRemoveGame (T_word16 mapNumber, T_gameGroupID groupID);
T_void GuildUIAddGame (T_word16 mapNumber, T_gameGroupID groupID, T_word16 questNumber) ;
T_void GuildUIAddPlayer (T_byte8 *playerName);
T_void GuildUIRemovePlayer (T_byte8 *playerName);
T_void GuildUIClearPlayers (T_void) ;

T_void GuildUIConfirmCreateGame (T_void);
T_void GuildUIConfirmJoinGame (T_void);
T_void GuildUICancelJoinGame (T_buttonID buttonID);

T_void GuildUIGetSelectedGame(
                  T_word16 *p_mapNumber,
                  T_gameGroupID *p_groupID,
				  T_word16 *p_quest);

#endif
