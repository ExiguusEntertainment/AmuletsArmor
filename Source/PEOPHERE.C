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

/* List of T_playerIDSelf structures */
static T_doubleLinkList G_peopleList = DOUBLE_LINK_LIST_BAD ;

static E_Boolean G_init = FALSE ;

static T_playerIDState G_ourState = PLAYER_ID_STATE_NONE ;

static T_word16 G_ourAdventure = 0 ;

/* Internal prototypes: */
static T_void IClearList(T_void) ;
static T_playerIDSelf *IFindByName(T_byte8 *p_name) ;
static T_playerIDSelf *ICreatePlayerID(T_playerIDSelf *p_playerID) ;
static T_playerIDLocation IGetOurLocation(T_void) ;
static T_void ISetupGame(T_gameGroupID groupID) ;

#define MAX_PLAYERS_PER_GAME  4
static T_word16 G_numPeopleInGame = 0 ;
static T_directTalkUniqueAddress G_peopleInGame[MAX_PLAYERS_PER_GAME] ;
static T_byte8 G_peopleNames[MAX_PLAYERS_PER_GAME][STATS_CHARACTER_NAME_MAX_LENGTH] ;

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereInitialize
 *-------------------------------------------------------------------------*/
/**
 *  This routine starts up the people here module.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereInitialize(T_void)
{
    DebugRoutine("PeopleHereInitialize") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    /* Create a list of playerIDSelf structures. */
    G_peopleList = DoubleLinkListCreate() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  People Here finish
 *-------------------------------------------------------------------------*/
/**
 *  People here finish cleans up the people here module by removing
 *  the list of playerIDSelf structures.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereFinish(T_void)
{
    DebugRoutine("PeopleHereFinish") ;
    DebugCheck(G_init == TRUE) ;

    /* Empty the list. */
    IClearList() ;

    /* Get rid of the list itself. */
    DoubleLinkListDestroy(G_peopleList) ;

    G_init = FALSE ;

    DebugEnd() ;
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
    DebugRoutine("PeopleHereReset") ;
    DebugCheck(G_init == TRUE) ;

    /* Start by clearing the list. */
    IClearList() ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereGetNumInGame
 *-------------------------------------------------------------------------*/
/**
 *  PeopleHereGetNumInGame counts the number of people in a game group.
 *
 *  @param groupID -- Game group id
 *
 *  @return Number of players found
 *
 *<!-----------------------------------------------------------------------*/
T_word16 PeopleHereGetNumInGame(T_gameGroupID groupID)
{
    T_doubleLinkListElement element ;
    T_playerIDSelf *p_check ;
    T_word16 numFound = 0 ;

    DebugRoutine("PeopleHereGetNumInGame") ;

    element = DoubleLinkListGetFirst(G_peopleList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_check = (T_playerIDSelf *)DoubleLinkListElementGetData(element) ;

        if (CompareGameGroupIDs(groupID, p_check->groupID))
            numFound++ ;

        element = DoubleLinkListElementGetNext(element) ;
    }

    DebugEnd() ;

    return numFound ;
}


/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereFindPlayerGame
 *-------------------------------------------------------------------------*/
/**
 *  Search to find a player in a game.  Search the player list for a
 *  given name and return its group id (if any).
 *
 *  @param p_name -- Name to find
 *  @param p_groupID -- ID of player
 *
 *  @return TRUE if found, else FALSE.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean PeopleHereFindPlayerGame(
              T_byte8 *p_name,
              T_gameGroupID *p_groupID)
{
    T_playerIDSelf *p_find ;
    E_Boolean found = FALSE ;

    DebugRoutine("PeopleHereFindPlayerGame") ;

    /* Search for the name */
    p_find = IFindByName(p_name) ;

    /* If the name is found, give it a group id. */
    if (p_find)  {
        *p_groupID = p_find->groupID ;
        found = TRUE ;
    }

    DebugEnd() ;

    return found ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IClearList
 *-------------------------------------------------------------------------*/
/**
 *  IClearList frees and removes all nodes in the double link list of
 *  T_playerIDSelf structures.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IClearList(T_void)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;

    DebugRoutine("IClearList") ;
    DebugCheck(G_peopleList != DOUBLE_LINK_LIST_BAD) ;

    /* Free all the elements in the list. */
    while ((element = DoubleLinkListGetFirst(G_peopleList)) !=
                 DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* Remove and free the element from the list. */
        MemFree(DoubleLinkListElementGetData(element)) ;
        DoubleLinkListRemoveElement(element) ;
    }

    DebugEnd() ;
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
    T_doubleLinkListElement element ;
    T_playerIDSelf *p_found = NULL ;
    T_playerIDSelf *p_check ;

    DebugRoutine("IFindByName") ;
    DebugCheck(G_peopleList != DOUBLE_LINK_LIST_BAD) ;

    /* Go through the current list and look for a match */
    element = DoubleLinkListGetFirst(G_peopleList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_check = (T_playerIDSelf *)DoubleLinkListElementGetData(element) ;
        if (strcmp(p_check->name, p_name) == 0)  {
            p_found = p_check ;
            break ;
        }

        element = DoubleLinkListElementGetNext(element) ;
    }

    DebugEnd() ;

    return p_found ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICreatePlayerID
 *-------------------------------------------------------------------------*/
/**
 *  ICreatePlayerID creates a new blank entry for a given person
 *
 *  @param p_name -- Name to search by
 *
 *  @return Found player ID pointer or NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_playerIDSelf *ICreatePlayerID(T_playerIDSelf *p_playerID)
{
    T_playerIDSelf *p_new = NULL ;

    DebugRoutine("ICreatePlayerID") ;

    /* Create a new node. */
    p_new = MemAlloc(sizeof(T_playerIDSelf)) ;
    DebugCheck(p_new != NULL) ;
    if (p_new)  {
        /* Fill out the new element with the defaults. */
        memset(p_new, 0, sizeof(T_playerIDSelf)) ;
        strncpy(p_new->name, p_playerID->name, 30) ;
        p_new->uniqueAddress = p_playerID->uniqueAddress ;

        /* Add to double link list, but make sure everything is ok. */
        if (DoubleLinkListAddElementAtEnd(G_peopleList, p_new) == DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            MemFree(p_new) ;
            p_new = NULL ;
        }
    }

    DebugEnd() ;

    return p_new ;
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
    T_playerIDLocation location = PLAYER_ID_LOCATION_NOWHERE ;
    T_word16 place ;

    DebugRoutine("IGetOurLocation") ;

    place = ClientGetCurrentPlace() ;

    switch(place)  {
        case HARDFORM_GOTO_PLACE_OFFSET+HARD_FORM_GUILD:
            location = PLAYER_ID_LOCATION_GUILD ;
            break ;
        case HARDFORM_GOTO_PLACE_OFFSET+HARD_FORM_TOWN:
            location = PLAYER_ID_LOCATION_TOWN ;
            break ;
    }

    if ((place < HARDFORM_GOTO_PLACE_OFFSET) && (place != 0))
        location = PLAYER_ID_LOCATION_GAME ;

    DebugEnd() ;

    return location ;
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
    DebugRoutine("PeopleHereGetPlayerIDSelfStruct") ;
    DebugCheck(p_self != NULL) ;

    strncpy(p_self->name, StatsGetName(), 30) ;
    DirectTalkGetUniqueAddress(&p_self->uniqueAddress) ;
    p_self->location = IGetOurLocation() ;
    p_self->state = G_ourState ;
    p_self->groupID = ClientSyncGetGameGroupID() ;
    p_self->adventure = PeopleHereGetOurAdventure() ;

    DebugEnd() ;
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
    T_gameGroupID groupID ;

    DebugRoutine("PeopleHereGetUniqueGroupID") ;

    /* Use our address as the unique address of the game. */
    DirectTalkGetUniqueAddress(&groupID) ;

    DebugEnd() ;

    return groupID ;
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
    G_ourState = state ;
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
    return G_ourState ;
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
    G_ourAdventure = adventure ;
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
    return G_ourAdventure ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereRequestJoin
 *-------------------------------------------------------------------------*/
/**
 *  Someone is requesting to join our game that we are creating!  Do
 *  we have room?
 *
 *  @return Current 16-bit adventure id.
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereRequestJoin(
        T_directTalkUniqueAddress uniqueAddress,
        T_gameGroupID groupID,
        T_word16 adventure)
{
    T_word16 i ;
    T_gameGroupID ourGroupID ;

    DebugRoutine("PeopleHereRequestJoin") ;

    ourGroupID = ClientSyncGetGameGroupID() ;
/*
puts("PeopleHereRequestJoin") ;  fflush(stdout) ;
printf("groupID=%02X:%02X:%02X:%02X:%02X:%02X, get=%d\n", groupID.address[0], groupID.address[1], groupID.address[2], groupID.address[3], groupID.address[4], groupID.address[5], ClientSyncGetGameGroupID()) ;
printf("state=%d\n", PeopleHereGetOurState()) ;
printf("adventure=%d, advhere=%d\n", adventure, PeopleHereGetOurAdventure()) ;
printf("ourloc=%d\n", IGetOurLocation()) ;  fflush(stdout) ;
*/

    /* In order to process this packet, we must be the */
    /* creator of the game and THIS particular group. */
    if ((CompareGameGroupIDs(groupID, ourGroupID)))  {
        if ((PeopleHereGetOurState() == PLAYER_ID_STATE_CREATING_GAME) &&
            (adventure == PeopleHereGetOurAdventure()) &&
            (IGetOurLocation() == PLAYER_ID_LOCATION_GUILD))  {
            if (G_numPeopleInGame < MAX_PLAYERS_PER_GAME)  {
                /* Add this person to the list (if not already) */
                for (i=0; i<G_numPeopleInGame; i++)  {
                    /* Stop if we have seen this one before. */
                    if (memcmp(
                            &uniqueAddress,
                            &G_peopleInGame[i],
                            sizeof(uniqueAddress)) == 0)
                        break ;
                }

                /* Are we at the end?  Then add the name. */
                if (i==G_numPeopleInGame)  {
                    G_peopleInGame[i] = uniqueAddress ;
                    G_numPeopleInGame++ ;
                }

                ClientSendRespondToJoinPacket(
                    uniqueAddress,
                    groupID,
                    adventure,
                    GAME_RESPOND_JOIN_OK) ;
            } else {
                ClientSendRespondToJoinPacket(
                    uniqueAddress,
                    groupID,
                    adventure,
                    GAME_RESPOND_JOIN_FULL) ;
            }
        } else {
            ClientSendRespondToJoinPacket(
                uniqueAddress,
                groupID,
                adventure,
                GAME_RESPOND_JOIN_CANCELED) ;
        }
    }

    DebugEnd() ;
}

T_void PeopleHereRespondToJoin(
           T_directTalkUniqueAddress uniqueAddress,
           T_gameGroupID groupID,
           T_word16 adventure,
           E_respondJoin response)
{
    T_directTalkUniqueAddress ourAddress ;
    DebugRoutine("PeopleHereRespondToJoin") ;

    /* Make sure this refers to us. */
    DirectTalkGetUniqueAddress(&ourAddress) ;
    if (memcmp(&ourAddress, &uniqueAddress, sizeof(ourAddress)) == 0)  {
        /* Yep, thats us. */
        switch(response)  {
            case GAME_RESPOND_JOIN_OK:
                /* Tell others that I'm trying to join in the game. */
                PeopleHereSetOurState(PLAYER_ID_STATE_JOINING_GAME) ;
                PeopleHereSetOurAdventure(adventure) ;
                ClientSyncSetGameGroupID(groupID) ;

                /* we are in. */
                GuildUIConfirmJoinGame();

                /* Send out a message that this is what we are doing. */
                ClientSendPlayerIDSelf() ;

                PeopleHereGeneratePeopleInGame(groupID) ;
                break ;
            case GAME_RESPOND_JOIN_FULL:
                PeopleHereSetOurState(PLAYER_ID_STATE_NONE) ;
                MessageAdd("Game is already full.") ;
                break ;
            case GAME_RESPOND_JOIN_CANCELED:
                PeopleHereSetOurState(PLAYER_ID_STATE_NONE) ;
                MessageAdd("Game was cancelled.") ;
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }
    } else {
        /* Not us.  Just ignore. */
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PeopleHereUpdatePlayer
 *-------------------------------------------------------------------------*/
/**
 *  PeopleHereUpdatePlayer is called per player self ID received.
 *  This routine updates the list and takes appropriate actions.
 *
 *  @param p_playerID -- New player action
 *
 *<!-----------------------------------------------------------------------*/
T_void PeopleHereUpdatePlayer(T_playerIDSelf *p_playerID)
{
    T_playerIDSelf *p_find ;
    T_playerIDLocation location ;
    T_gameGroupID ourGroupID ;

    DebugRoutine("PeopleHereUpdatePlayer") ;
    DebugCheck(p_playerID != NULL) ;
    DebugCheck(G_init == TRUE) ;

    /* What is our address again? */
    ourGroupID = ClientSyncGetGameGroupID() ;

/*
printf("'%s' %d state, %d location, ", p_playerID->name, p_playerID->state, p_playerID->location) ;
DirectTalkPrintAddress(stdout, &p_playerID->groupID) ;
printf(" groupID\n") ;
*/

    /* See what our location is.  If there is not a match, */
    /* we don't care about that location. */
    location = IGetOurLocation() ;

    /* First, try finding the name. */
    p_find = IFindByName(p_playerID->name) ;

    /* If not found, try creating the player. */
    if (p_find == NULL)
        p_find = ICreatePlayerID(p_playerID) ;

    /* If either work, go ahead and update the status. */
    if (p_find)  {
        /* Check if the location is changing */
        if (p_find->location != p_playerID->location)  {
            /* There is a difference, either coming or going. */

            /* Is the player entering or exiting? */
            if (p_playerID->location == location)  {
                /* Entering */
                /* Do an action based on our location. */
                switch(location)  {
                    case PLAYER_ID_LOCATION_NOWHERE:
                        /* We both are nowhere. nothing happens. */
                        break ;
                    case PLAYER_ID_LOCATION_TOWN:
                        /* Update the town list. */
                        TownAddPerson(p_find->name);
                        break ;
                    case PLAYER_ID_LOCATION_GUILD:
                        /* Action is taken care of below. */
/*
                        if (ClientSyncGetGameGroupID() != 0)
                            if (p_find->groupID == ClientSyncGetGameGroupID())
                                PeopleHereGeneratePeopleInGame() ;
*/
                        break ;
                    case PLAYER_ID_LOCATION_GAME:
                        /* Player is elsewhere.  do nothing. */
                        break ;
                    default:
                        /* What? */
                        DebugCheck(FALSE) ;
                        break ;
                }
            } else {
                /* Exiting */
                /* Do an action based on our location. */
                switch(location)  {
                    case PLAYER_ID_LOCATION_NOWHERE:
                        /* We both are nowhere. nothing happens. */
                        break ;
                    case PLAYER_ID_LOCATION_TOWN:
                        /* Update the town list. */
                        TownRemovePerson(p_find->name);
                        break ;
                    case PLAYER_ID_LOCATION_GUILD:
                        /* Action is taken care of below. */
/*
                        if (ClientSyncGetGameGroupID() != 0)
                            if (p_find->groupID == ClientSyncGetGameGroupID())
                                PeopleHereGeneratePeopleInGame() ;
*/
                        break ;
                    case PLAYER_ID_LOCATION_GAME:
                        /* Player is elsewhere.  do nothing. */
                        break ;
                    default:
                        /* What? */
                        DebugCheck(FALSE) ;
                        break ;
                }
            }
        }

        if (p_find->state != p_playerID->state)  {
            /* Do guild related events */
            if (location == PLAYER_ID_LOCATION_GUILD)  {
                if (!((PeopleHereGetOurState() == PLAYER_ID_STATE_CREATING_GAME) ||
                    (PeopleHereGetOurState() == PLAYER_ID_STATE_JOINING_GAME)))  {
                    if (p_playerID->state == PLAYER_ID_STATE_CREATING_GAME)  {
                        GuildUIAddGame(
                            p_playerID->adventure,
                            p_playerID->groupID) ;
                        PeopleHereGeneratePeopleInGame(p_playerID->groupID) ;
                    } else if (p_find->state == PLAYER_ID_STATE_CREATING_GAME)  {
                        GuildUIRemoveGame(
                            p_playerID->adventure,
                            p_playerID->groupID) ;
                        PeopleHereGeneratePeopleInGame(*DirectTalkGetNullBlankUniqueAddress()) ;
                    }
                } else {
                    /* Detected that a game is removed. */
                    if (p_find->state == PLAYER_ID_STATE_CREATING_GAME)  {
                        /* Is it our game? */
                        if (CompareGameGroupIDs(p_find->groupID, ourGroupID))  {
                            /* Act like we pressed the cancel join button. */
                            GuildUICancelJoinGame(NULL);
                        }
                    }
                }
            }
        }

        if (location == PLAYER_ID_LOCATION_GUILD)  {
            /* Update the list of players if relevant to this group. */
            if ((CompareGameGroupIDs(p_find->groupID, ourGroupID)) ||
                (CompareGameGroupIDs(p_playerID->groupID, ourGroupID)))
                PeopleHereGeneratePeopleInGame(ourGroupID) ;
        }

        /* Only process the state if it changes. */
        /* Record the state in all cases. */
        p_find->state = p_playerID->state ;

        /* Store the location. */
        p_find->location = p_playerID->location ;

        /* Store the new groupID */
        p_find->groupID = p_playerID->groupID ;

        if (location == PLAYER_ID_LOCATION_GUILD)  {
            /* Update the list of players if relevant to this group. */
            PeopleHereGeneratePeopleInGame(ClientSyncGetGameGroupID()) ;
        }
    }

    DebugEnd() ;
}

T_void PeopleHereGeneratePeopleInGame(T_gameGroupID groupID)
{
    T_doubleLinkListElement element ;
    T_playerIDSelf *p_check ;

    DebugRoutine("PeopleHereGeneratePeopleInGame") ;

    GuildUIClearPlayers() ;

    if (IGetOurLocation() == PLAYER_ID_LOCATION_GUILD)  {
        /* Go through the list and identify everyone in this group. */
        element = DoubleLinkListGetFirst(G_peopleList) ;
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            p_check = (T_playerIDSelf *)DoubleLinkListElementGetData(element) ;
            DebugCheck(p_check != NULL) ;

            if ((CompareGameGroupIDs(p_check->groupID, groupID)) &&
                (p_check->location == PLAYER_ID_LOCATION_GUILD))  {
                GuildUIAddPlayer(p_check->name) ;
            }

            element = DoubleLinkListElementGetNext(element) ;
        }
    }

    DebugEnd() ;
}

T_playerIDLocation PeopleHereGetOurLocation(T_void)
{
    return IGetOurLocation() ;
}

T_void PeopleHereStartGame(T_word16 firstLevel)
{
    DebugRoutine("PeopleHereStartGame") ;

    /* Get the exact list of people going on this game. */
    ISetupGame(ClientSyncGetGameGroupID()) ;

//printf("Starting game at %d\n", PeopleHereGetOurAdventure()) ; fflush(stdout) ;
    ClientSendGameStartPacket(
        ClientSyncGetGameGroupID(),
        PeopleHereGetOurAdventure(),
        (T_byte8)G_numPeopleInGame,
        G_peopleInGame,
        firstLevel) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ISetupGame
 *-------------------------------------------------------------------------*/
/**
 *  PeopleHereGetNumInGame counts the number of people in a game group.
 *
 *  @param game -- Game group id
 *
 *  @param T_word16 -- Number of players found
 *
 *<!-----------------------------------------------------------------------*/
static T_void ISetupGame(T_gameGroupID groupID)
{
    T_doubleLinkListElement element ;
    T_playerIDSelf *p_check ;
    T_word16 numFound = 0 ;

    DebugRoutine("ISetupGame") ;

    element = DoubleLinkListGetFirst(G_peopleList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_check = (T_playerIDSelf *)DoubleLinkListElementGetData(element) ;

        if (CompareGameGroupIDs(groupID, p_check->groupID))  {
            G_peopleInGame[numFound] = p_check->uniqueAddress ;
            numFound++ ;

            /* Stop if we reached the limit. */
            if (numFound == MAX_PLAYERS_PER_GAME)
                break ;
        }

        element = DoubleLinkListElementGetNext(element) ;
    }

    G_numPeopleInGame = numFound ;

    DebugEnd() ;
}

T_void PeopleHereIDPlayer(T_word16 playerNum, T_byte8 *p_name)
{
    DebugRoutine("PeopleHereIDPlayer") ;
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME) ;

    if (strlen(G_peopleNames[playerNum]) < 30)
        strcat(G_peopleNames[playerNum], p_name) ;
//printf("Player %d now named '%s'\n", playerNum, G_peopleNames[playerNum]) ;

    DebugEnd() ;
}

T_void PeopleHereResetPlayerIDs(T_void)
{
    DebugRoutine("PeopleHereResetPlayerIDs") ;

    memset(G_peopleNames, 0, sizeof(G_peopleNames)) ;

    DebugEnd() ;
}

T_byte8 *PeopleHereGetPlayerIDName(T_word16 playerNum)
{
    DebugRoutine("PeopleHereGetPlayerIDName") ;

    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME) ;

    DebugEnd() ;

    return G_peopleNames[playerNum] ;
}

T_directTalkUniqueAddress *PeopleHereGetUniqueAddr(T_word16 playerNum)
{
    DebugRoutine("PeopleHereGetUniqueAddr") ;
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME) ;

    DebugEnd() ;

    return &G_peopleInGame[playerNum] ;
}

T_void PeopleHereSetUniqueAddr(
           T_word16 playerNum,
           T_directTalkUniqueAddress *uaddr)
{
    DebugCheck(playerNum < MAX_PLAYERS_PER_GAME) ;

    memcpy(&G_peopleInGame[playerNum], uaddr, 6) ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PEOPHERE.C
 *-------------------------------------------------------------------------*/
