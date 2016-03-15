/*-------------------------------------------------------------------------*
 * File:  PEOPHERE.C
 *-------------------------------------------------------------------------*/
/**
 * List of people here over the network.
 *
 * @addtogroup PEOPHERE
 * @brief People Here
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CSYNCPCK.H"
#include "GUILDUI.H"
#include "HARDFORM.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "PEOPHERE.H"
#include "STATS.H"
#include "TOWNUI.H"

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define MAX_PLAYERS_IN_WORLD  256

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
//! What are the people in the town/guild?
static T_playerIDSelf G_peopleList[MAX_PLAYERS_IN_WORLD];
//! Have we initialized?
static E_Boolean G_init = FALSE;
//! What state are we in (relative to people in the groups
static T_playerIDState G_ourState = PLAYER_ID_STATE_NONE;
//! Which adventure id is being planned for this game?
static T_word16 G_ourAdventure = 0;
//! Number of players in the current game group
static T_word16 G_numPeopleInGame = 0;
//! Names of the players in the current synchronized game
static T_byte8 G_peopleNames[MAX_PLAYERS_PER_GAME][STATS_CHARACTER_NAME_MAX_LENGTH];

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
static T_playerIDSelf *ICreatePlayerID(T_playerIDSelf *p_playerID);
static T_playerIDSelf *IFindByName(T_byte8 *p_name);
static T_playerIDLocation IGetOurLocation(T_void);

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereInitialize
 *-------------------------------------------------------------------------*/
/**
 *  This routine starts up the people here module.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereInitialize(T_void)
{
    DebugRoutine("PeopleHereInitialize");
    DebugCheck(G_init == FALSE);

    G_init = TRUE;

    /* Clear list of playerIDSelf structures. */
    memset(G_peopleList, 0, sizeof(G_peopleList));
    G_ourState = PLAYER_ID_STATE_NONE;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereFinish
 *-------------------------------------------------------------------------*/
/**
 *  People here finish cleans up the people here module by removing
 *  the list of playerIDSelf structures.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereFinish(T_void)
{
    DebugRoutine("PeopleHereFinish");
    DebugCheck(G_init == TRUE);

    /* Clear the list */
    memset(G_peopleList, 0, sizeof(G_peopleList));

    G_init = FALSE;
    G_ourState = PLAYER_ID_STATE_NONE;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereReset
 *-------------------------------------------------------------------------*/
/**
 *  Reset the list of people here.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereReset(T_void)
{
    DebugRoutine("PeopleHereReset");
    DebugCheck(G_init == TRUE);

    /* Clear the list */
    memset(G_peopleList, 0, sizeof(G_peopleList));
    G_ourState = PLAYER_ID_STATE_NONE;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  IFindByName
 *-------------------------------------------------------------------------*/
/**
 *  IFindByName searches the people list for a person with the given
 *  name (case does matter).
 *
 *  @param p_name -- Name to search by
 *
 *  @return Found player ID pointer or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_playerIDSelf *IFindByName(T_byte8 *p_name)
{
    T_playerIDSelf *p_found = NULL;
    T_word16 i;
    T_playerIDSelf *p = G_peopleList;

    DebugRoutine("IFindByName");

    for (i = 0; i < MAX_PLAYERS_IN_WORLD; i++, p++) {
        if ((p->name[0]) && (strcmp(p_name, p->name) == 0)) {
            p_found = p;
            break;
        }
    }

    DebugEnd();

    return p_found;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICreatePlayerID
 *-------------------------------------------------------------------------*/
/**
 *  ICreatePlayerID creates a new blank entry for a given person
 *
 *  @param p_playerID -- ID of new player to add
 *
 *  @return Found player ID pointer or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_playerIDSelf *ICreatePlayerID(T_playerIDSelf *p_playerID)
{
    T_playerIDSelf *p_new = NULL;
    T_word16 i;
    T_playerIDSelf *p = G_peopleList;

    DebugRoutine("ICreatePlayerID");

    // Find a slot with no name
    for (i = 0; i < MAX_PLAYERS_IN_WORLD; i++, p++) {
        if (p->name[0] == '\0') {
            memcpy(p, p_playerID, sizeof(*p));

            // We want the transition from nowhere to somewhere, 
            // start at nowhere for a newly created character
            p->location = PLAYER_ID_LOCATION_NOWHERE;
            p->state = PLAYER_ID_STATE_NONE;
            p_new = p;
            break;
        }
    }

    DebugEnd();

    return p_new;
}

/*-------------------------------------------------------------------------*
 * Routine:  IGetOurLocation
 *-------------------------------------------------------------------------*/
/**
 *  IGetOurLocation determines this players general location
 *
 *  @return General player location
 *
 *<!-----------------------------------------------------------------------*/
static T_playerIDLocation IGetOurLocation(T_void)
{
    T_playerIDLocation location = PLAYER_ID_LOCATION_NOWHERE;
    T_word16 place;

    DebugRoutine("IGetOurLocation");

    place = ClientGetCurrentPlace();

    switch (place) {
        case HARDFORM_GOTO_PLACE_OFFSET + HARD_FORM_GUILD:
            location = PLAYER_ID_LOCATION_GUILD;
            break;
        case HARDFORM_GOTO_PLACE_OFFSET + HARD_FORM_TOWN:
            location = PLAYER_ID_LOCATION_TOWN;
            break;
    }

    if ((place < HARDFORM_GOTO_PLACE_OFFSET) && (place != 0))
        location = PLAYER_ID_LOCATION_GAME;

    DebugEnd();

    return location;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetPlayerIDSelfStruct
 *-------------------------------------------------------------------------*/
/**
 *  Used to fill out this player's own player ID structure
 *
 *  @param p_self -- Pointer to player id self to fill
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereGetPlayerIDSelfStruct(T_playerIDSelf *p_self)
{
    DebugRoutine("PeopleHereGetPlayerIDSelfStruct");
    DebugCheck(p_self != NULL);

    strncpy(p_self->name, StatsGetName(), 30);
    DirectTalkGetUniqueAddress(&p_self->uniqueAddress);
    p_self->location = IGetOurLocation();
    p_self->state = G_ourState;
    p_self->groupID = ClientSyncGetGameGroupID();
    p_self->adventure = PeopleHereGetOurAdventure();
	p_self->quest = StatsGetCurrentQuestNumber();

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetUniqueGroupID
 *-------------------------------------------------------------------------*/
/**
 *  Get a unique group ID for us.  We have one, it just so happens to
 *  equal our unique address.
 *
 *  @return Game group ID for this player
 *
 *<!-----------------------------------------------------------------------*/
T_gameGroupID PeopleHereGetUniqueGroupID(T_void)
{
    T_gameGroupID groupID;

    DebugRoutine("PeopleHereGetUniqueGroupID");

    /* Use our address as the unique address of the game. */
    DirectTalkGetUniqueAddress(&groupID);

    DebugEnd();

    return groupID;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereSetOurState
 *-------------------------------------------------------------------------*/
/**
 *  Accessor function to change our current state.
 *
 *  @param [in] state -- Our new game state (joining, creating, etc.)
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereSetOurState(T_playerIDState state)
{
    G_ourState = state;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereSetOurState
 *-------------------------------------------------------------------------*/
/**
 *  Accessor function to return our current state.
 *
 *  @return Our current game state (joining, creating, etc.)
 *
 *<!-----------------------------------------------------------------------*/
T_playerIDState PeopleHereGetOurState(T_void)
{
    return G_ourState;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereSetOurAdventure
 *-------------------------------------------------------------------------*/
/**
 *  Accessor function set the adventure ID.
 *
 *  @param [in] adventure -- 16-bit adventure id.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereSetOurAdventure(T_word16 adventure)
{
    G_ourAdventure = adventure;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetOurAdventure
 *-------------------------------------------------------------------------*/
/**
 *  Accessor function to get the adventure ID.
 *
 *  @return Current 16-bit adventure id.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PeopleHereGetOurAdventure(T_void)
{
    return G_ourAdventure;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereRequestJoin
 *-------------------------------------------------------------------------*/
/**
 *  Someone is requesting to join our game that we are creating!  Do
 *  we have room?  Are they requesting the right group and adventure id?
 *
 *  @param unqiueAddress -- unique address of person requesting to join
 *  @param groupID -- ID of group being requested to join
 *  @param adventure -- Number of adventure being requested to join
 *
 *  @return Current 16-bit adventure id.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereRequestJoin(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure)
{
    T_word16 i;
    T_gameGroupID ourGroupID;

    DebugRoutine("PeopleHereRequestJoin");

    ourGroupID = ClientSyncGetGameGroupID();

    /* In order to process this packet, we must be the */
    /* creator of the game and THIS particular group. */
    if ((CompareGameGroupIDs(groupID, ourGroupID))) {
        if ((PeopleHereGetOurState() == PLAYER_ID_STATE_CREATING_GAME)
                && (adventure == PeopleHereGetOurAdventure())
                && (IGetOurLocation() == PLAYER_ID_LOCATION_GUILD)) {
            if (G_numPeopleInGame < MAX_PLAYERS_PER_GAME) {
                /* Add this person to the list (if not already) */
                for (i = 0; i < G_numPeopleInGame; i++) {
                    /* Stop if we have seen this one before. */
                    if (memcmp(&uniqueAddress, &G_peopleNetworkIDInGame[i],
                            sizeof(uniqueAddress)) == 0)
                        break;
                }

                /* Are we at the end?  Then add the name. */
                if (i == G_numPeopleInGame) {
                    G_peopleNetworkIDInGame[i] = uniqueAddress;
                    G_numPeopleInGame++;
                }

                ClientSendRespondToJoinPacket(uniqueAddress, groupID, adventure,
                GAME_RESPOND_JOIN_OK);
            } else {
                ClientSendRespondToJoinPacket(uniqueAddress, groupID, adventure,
                GAME_RESPOND_JOIN_FULL);
            }
        } else {
            ClientSendRespondToJoinPacket(uniqueAddress, groupID, adventure,
            GAME_RESPOND_JOIN_CANCELED);
        }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereRespondToJoin
 *-------------------------------------------------------------------------*/
/**
 *  After requesting to join, this is the response handler.
 *
 *  @param unqiueAddress -- unique address of person requesting to join
 *  @param groupID -- ID of group being requested to join
 *  @param adventure -- Number of adventure being requested to join
 *  @param response -- Response to attempted join
 *
 *  @return Current 16-bit adventure id.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereRespondToJoin(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure,
        E_respondJoin response)
{
    T_directTalkUniqueAddress ourAddress;
    DebugRoutine("PeopleHereRespondToJoin");

    /* Make sure this refers to us. */
    DirectTalkGetUniqueAddress(&ourAddress);
    if (memcmp(&ourAddress, &uniqueAddress, sizeof(ourAddress)) == 0) {
        /* Yep, thats us. */
        switch (response) {
            case GAME_RESPOND_JOIN_OK:
                /* Tell others that I'm trying to join in the game. */
                PeopleHereSetOurState(PLAYER_ID_STATE_JOINING_GAME);
                PeopleHereSetOurAdventure(adventure);
                ClientSyncSetGameGroupID(groupID);

                /* we are in. */
                GuildUIConfirmJoinGame();

                /* Send out a message that this is what we are doing. */
                ClientSendPlayerIDSelf();

                PeopleHereGeneratePeopleInGame(groupID);
                break;
            case GAME_RESPOND_JOIN_FULL:
                PeopleHereSetOurState(PLAYER_ID_STATE_NONE);
                MessageAdd("Game is already full.");
                break;
            case GAME_RESPOND_JOIN_CANCELED:
                PeopleHereSetOurState(PLAYER_ID_STATE_NONE);
                MessageAdd("Game was cancelled.");
                break;
            default:
                DebugCheck(FALSE);
                break;
        }
    } else {
        /* Not us.  Just ignore. */
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereUpdatePlayer
 *-------------------------------------------------------------------------*/
/**
 *  PeopleHereUpdatePlayer is called per player self ID received.
 *  This routine updates the list and takes appropriate actions. The
 *  player may be moving from room to room or other actions.
 *
 *  @param p_playerID -- New player information
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereUpdatePlayer(T_playerIDSelf *p_playerID)
{
    T_playerIDSelf *p_find;
    T_playerIDLocation location;
    T_gameGroupID ourGroupID;

    DebugRoutine("PeopleHereUpdatePlayer");
    DebugCheck(p_playerID != NULL);
    DebugCheck(G_init == TRUE);

    /* What is our address again? */
    ourGroupID = ClientSyncGetGameGroupID();

    /* See what our location is.  If there is not a match, */
    /* we don't care about that location. */
    location = IGetOurLocation();

    /* First, try finding the player doing the action. */
    p_find = IFindByName(p_playerID->name);

    /* If not found, try creating the player.  Must have arrived. */
    if (p_find == NULL)
        p_find = ICreatePlayerID(p_playerID);

    /* If either work, go ahead and update the status. */
    if (p_find) {
        /* Check if the location is changing */
        if (p_find->location != p_playerID->location) {
            /* There is a difference, either coming or going. */

            /* Is the player entering or exiting? */
            if (p_playerID->location == location) {
                /* Entering */
                /* Do an action based on our location. */
                switch (location) {
                    case PLAYER_ID_LOCATION_NOWHERE:
                        /* We both are nowhere. nothing happens. */
                        break;
                    case PLAYER_ID_LOCATION_TOWN:
                        /* Update the town list. */
                        TownAddPerson(p_find->name);
                        break;
                    case PLAYER_ID_LOCATION_GUILD:
                        /* Action is taken care of below. */
                        break;
                    case PLAYER_ID_LOCATION_GAME:
                        /* Player is elsewhere.  do nothing. */
                        break;
                    default:
                        /* What? */
                        DebugCheck(FALSE);
                        break;
                }
            } else {
                /* Exiting */
                /* Do an action based on our location. */
                switch (location) {
                    case PLAYER_ID_LOCATION_NOWHERE:
                        /* We both are nowhere. nothing happens. */
                        break;
                    case PLAYER_ID_LOCATION_TOWN:
                        /* Update the town list. */
                        TownRemovePerson(p_find->name);
                        break;
                    case PLAYER_ID_LOCATION_GUILD:
                        /* Action is taken care of below. */
                        break;
                    case PLAYER_ID_LOCATION_GAME:
                        /* Player is elsewhere.  do nothing. */
                        break;
                    default:
                        /* What? */
                        DebugCheck(FALSE);
                        break;
                }
            }
        }

        // Was there a change of state?
        if (p_find->state != p_playerID->state) {
            /* Do guild related events */
            if (location == PLAYER_ID_LOCATION_GUILD) {
                if (!((PeopleHereGetOurState() == PLAYER_ID_STATE_CREATING_GAME)
                        || (PeopleHereGetOurState()
                                == PLAYER_ID_STATE_JOINING_GAME))) {
                    if (p_playerID->state == PLAYER_ID_STATE_CREATING_GAME) {
                        GuildUIAddGame(p_playerID->adventure,
                                p_playerID->groupID,
								p_playerID->quest);
                        PeopleHereGeneratePeopleInGame(p_playerID->groupID);
                    } else if (p_find->state == PLAYER_ID_STATE_CREATING_GAME) {
                        GuildUIRemoveGame(p_playerID->adventure,
                                p_playerID->groupID);
                        PeopleHereGeneratePeopleInGame(
                                *DirectTalkGetNullBlankUniqueAddress());
                    }
                } else {
                    /* Detected that a game is removed. */
                    if (p_find->state == PLAYER_ID_STATE_CREATING_GAME) {
                        /* Is it our game? */
                        if (CompareGameGroupIDs(p_find->groupID, ourGroupID)) {
                            /* Act like we pressed the cancel join button. */
                            GuildUICancelJoinGame(NULL);
                        }
                    }
                }
            }
        }

        // Are they in the guild and going to our game?
        if (location == PLAYER_ID_LOCATION_GUILD) {
            /* Update the list of players if relevant to this group. */
            if ((CompareGameGroupIDs(p_find->groupID, ourGroupID))
                    || (CompareGameGroupIDs(p_playerID->groupID, ourGroupID)))
                // Yes, in our game now
                PeopleHereGeneratePeopleInGame(ourGroupID);
        }

        /* Only process the state if it changes. */
        /* Record the state in all cases. */
        p_find->state = p_playerID->state;

        /* Store the location. */
        p_find->location = p_playerID->location;

        /* Store the new groupID */
        p_find->groupID = p_playerID->groupID;

        if (location == PLAYER_ID_LOCATION_GUILD) {
            /* Update the list of players if relevant to this group. */
            PeopleHereGeneratePeopleInGame(ClientSyncGetGameGroupID());
        }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGeneratePeopleInGame
 *-------------------------------------------------------------------------*/
/**
 *  Update the name of players in this game in the guild.  It clears
 *  out the old list, looks at the people here, and updates.
 *
 *  @param groupID -- Group ID of this game in the guild.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereGeneratePeopleInGame(T_gameGroupID groupID)
{
    T_word16 i;
    T_playerIDSelf *p = G_peopleList;

    DebugRoutine("PeopleHereGeneratePeopleInGame");

    GuildUIClearPlayers();

    if (IGetOurLocation() == PLAYER_ID_LOCATION_GUILD) {
        /* Go through the list and identify everyone in this group. */
        for (i = 0; i < MAX_PLAYERS_IN_WORLD; i++, p++) {
            if ((CompareGameGroupIDs(p->groupID, groupID))
                    && (p->location == PLAYER_ID_LOCATION_GUILD)) {
                GuildUIAddPlayer(p->name);
            }
        }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetOurLocation
 *-------------------------------------------------------------------------*/
/**
 *  Return our current location in the game world.
 *
 *  @return Player location identifier
 *
 *<!-----------------------------------------------------------------------*/
T_playerIDLocation PeopleHereGetOurLocation(T_void)
{
    return IGetOurLocation();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereStartGame
 *-------------------------------------------------------------------------*/
/**
 *  Let's start the game and go to the given first level.
 *
 *  @param firstLevel -- First level to go to
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereStartGame(T_word16 firstLevel)
{
    DebugRoutine("PeopleHereStartGame");

    /* Get the exact list of people going on this game. */
    ISetupGame(ClientSyncGetGameGroupID());

    ClientSendGameStartPacket(ClientSyncGetGameGroupID(),
            PeopleHereGetOurAdventure(), (T_byte8)G_numPeopleInGame,
            G_peopleNetworkIDInGame, firstLevel,
			LEVEL_STATUS_STARTED);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ISetupGame
 *-------------------------------------------------------------------------*/
/**
 *  ISetupGame creates a new game by counting up to the maximum number
 *  of players in the given groupID.  Sets G_numPeopleInGame.
 *
 *  @param groupID -- Game group id
 *
 *<!-----------------------------------------------------------------------*/
T_void ISetupGame(T_gameGroupID groupID)
{
    T_playerIDSelf *p = G_peopleList;
    T_word16 numFound = 0;
    T_word16 i;

    DebugRoutine("ISetupGame");

    for (i = 0; i < MAX_PLAYERS_IN_WORLD; i++, p++) {
        if (CompareGameGroupIDs(groupID, p->groupID)) {
            G_peopleNetworkIDInGame[numFound] = p->uniqueAddress;
            numFound++;

            /* Stop if we reached the limit.  Sorry to anybody else */
            if (numFound == MAX_PLAYERS_PER_GAME)
                break;
        }
    }

    G_numPeopleInGame = numFound;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereIDPlayer
 *-------------------------------------------------------------------------*/
/**
 *  PeopleHereIDPlayer sets the name of of the game given player by
 *  number in the list.  If already exists, appends more text.
 *
 *  @param playerNum -- Player number
 *  @param p_name -- Name of the player
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereIDPlayer(T_word16 playerNum, T_byte8 *p_name)
{
    DebugRoutine("PeopleHereIDPlayer");
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME);

    // Build a complete name using pieces of the name
    if (strlen(G_peopleNames[playerNum]) < 30)
        strcat(G_peopleNames[playerNum], p_name);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereResetPlayerIDs
 *-------------------------------------------------------------------------*/
/**
 *  Reset all the names of the players in the current synchronized game.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereResetPlayerIDs(T_void)
{
    DebugRoutine("PeopleHereResetPlayerIDs");

    memset(G_peopleNames, 0, sizeof(G_peopleNames));

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetPlayerIDName
 *-------------------------------------------------------------------------*/
/**
 *  Convert a player index (in the people here list) to a name.
 *
 *  @param [in] playerNum -- Index of name in people here list
 *
 *  @return Name of player
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *PeopleHereGetPlayerIDName(T_word16 playerNum)
{
    DebugRoutine("PeopleHereGetPlayerIDName");

    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME);

    DebugEnd();

    return G_peopleNames[playerNum];
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetUniqueAddr
 *-------------------------------------------------------------------------*/
/**
 *  Get the unique network ID for the given player index for a player in
 *  a synchronized game.
 *
 *  @param [in] playerNum -- Index of name in people here list
 *
 *  @return Name of player
 *
 *<!-----------------------------------------------------------------------*/
T_directTalkUniqueAddress *PeopleHereGetUniqueAddr(T_word16 playerNum)
{
    DebugRoutine("PeopleHereGetUniqueAddr");
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME);

    DebugEnd();

    return &G_peopleNetworkIDInGame[playerNum];
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereSetUniqueAddr
 *-------------------------------------------------------------------------*/
/**
 *  Set the unique network ID for the player in this game based on
 *  their index.
 *
 *  @param [in] playerNum -- Index of name in people here list
 *  @param [in] uaddr -- Network ID.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereSetUniqueAddr(
        T_word16 playerNum,
        T_directTalkUniqueAddress *uaddr)
{
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME);

    memcpy(&G_peopleNetworkIDInGame[playerNum], uaddr, 6);
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetNumInGroupGame
 *-------------------------------------------------------------------------*/
/**
 *  Return the number of people in the currently active game.
 *
 *  @return Number of people.  Usually always 1 -- you.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PeopleHereGetNumInGroupGame(void)
{
    return G_numPeopleInGame;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereSendPacketToGroup
 *-------------------------------------------------------------------------*/
/**
 *  Send a long packet to all members in a group.  Does not send to
 *  ourself.
 *
 *  @param groupID -- Group ID to send to (0=town UI, otherwise game ID)
 *  @param p_packet -- Pointer to packet to send
 *  @param retryTime -- Retry time for sending this packet
 *  @param extraData -- Extra data for this packet callback
 *  @param p_callback -- Callback when sent (0 if no callback)
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereSendPacketToGroup(
        T_gameGroupID groupID,
        T_packetLong *p_packet,
        T_word16 retryTime,
        T_word32 extraData,
        T_cmdQPacketCallback p_callback)
{
    T_word16 i;
    T_playerIDSelf *p = G_peopleList;
    T_directTalkUniqueAddress ourID;

    DebugRoutine("PeopleHereGeneratePeopleInGame");

    DirectTalkGetUniqueAddress(&ourID);

    /* Go through the list and identify everyone in this group. */
    for (i = 0; i < MAX_PLAYERS_IN_WORLD; i++, p++) {
        if ((p->name[0]) && (CompareGameGroupIDs(p->groupID, groupID))) {
            if (!CompareUniqueNetworkIDs(p->uniqueAddress, ourID)) {
                // Create a long packet request for EACH player
                CmdQSendLongPacket(p_packet, &p->uniqueAddress, retryTime, extraData,
                        p_callback);
            }
        }
    }

    DebugEnd();
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PEOPHERE.C
 *-------------------------------------------------------------------------*/
