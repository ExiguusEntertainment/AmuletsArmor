/****************************************************************************/
/*    FILE:  AREASND.C                                                      */
/****************************************************************************/
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "AREASND.H"
#include "GENERAL.H"
#include "MEMORY.H"
#include "PLAYER.H"
#include "SOUND.H"
#include "TICKER.H"

#define AREA_SOUND_ID       ((((T_word32)'A')<<0) | \
                             (((T_word32)'S')<<8) | \
                             (((T_word32)'n')<<16) | \
                             (((T_word32)'d')<<24))

#define AREA_SOUND_ID_DONE  ((((T_word32)'F')<<0) | \
                             (((T_word32)'A')<<8) | \
                             (((T_word32)'S')<<16) | \
                             (((T_word32)'_')<<24))

typedef struct T_areaSoundStructTag {
    T_word32 tag ;
    T_sword16 x, y ;              /* Position of sound. */
    T_sword16 radius ;            /* Maximum radius to where sound is 0 */
    T_word16 maxVolume ;          /* Volume in center. */
    T_word16 currentVolume ;      /* Volume to player. 0 means not active. */
    E_areaSoundType type ;        /* Type:  to loop or not to loop. */
    T_word16 length ;             /* How long the sound is. */
    T_word16 groupId ;            /* Group id or 0 if independent. */
    T_sword16 soundNum ;          /* Number of sound to play. */
    T_sword16 channel ;           /* Number of channel sound is in, */
                                  /* or SOUND_BAD if none. */
    T_areaSoundFinishCallback p_callback ; /* Who to call when sound is done, */
                                           /* or NULL if nobody. */
                                           /* Only applies to type ONCE. */
    struct T_areaSoundStructTag *group ;  /* Link to group "leader". */
    struct T_areaSoundStructTag *next ;   /* Link to next area sound. */
    T_word32 data ;               /* Extra data to pass to the callback. */
    T_word32 id ;
    E_Boolean markedForDestroy ;
} ;

#define T_areaSoundStruct struct T_areaSoundStructTag

static E_Boolean G_areaSoundInit = FALSE ;
static T_areaSoundStruct *P_firstAreaSound = NULL ;
static T_areaSoundStruct *P_lastAreaSound = NULL ;
static T_word32 G_nextID = 0 ;

/* Internal prototypes: */
T_areaSound IFindGroupLeader(T_word16 groupId) ;
T_void ISetGroupLeaderAndId(
           T_areaSound areaSound,
           T_areaSound groupLeader,
           T_word16 groupId) ;
T_areaSoundStruct *IFindPrevSound(T_areaSoundStruct *p_sound) ;
T_void ICalculateAllVolumes(T_sword16 listenX, T_sword16 listenY) ;
T_areaSoundStruct *IFindLoudestNotChannelSound(T_void) ;
T_void IUpdateActiveSounds(T_sword16 listenX, T_sword16 listenY) ;
T_void IUpdateInactiveSounds(T_sword16 listenX, T_sword16 listenY) ;
T_word16 ICalculateVolume(
             T_areaSoundStruct *p_sound,
             T_sword16 listenX,
             T_sword16 listenY) ;
static T_word16 IAreaSoundDeterminePan(
                    T_areaSoundStruct *p_sound,
                    T_sword16 listenX,
                    T_sword16 listenY) ;
static T_void ISoundForAreaSoundIsDone(
                  T_void *p_data) ;
static T_areaSoundStruct *IFindAreaSoundByID(T_word32 id) ;
static T_void IDestroyMarked(T_void) ;


/****************************************************************************/
/*  Routine:  AreaSoundInitialize                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundInitialize starts up the area sound module.  Really not too  */
/*  much is done here.  Should things change in the future, more will be    */
/*  done.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundInitialize(T_void)
{
    DebugRoutine("AreaSoundInitialize") ;
    DebugCheck(G_areaSoundInit == FALSE) ;

    G_areaSoundInit = TRUE ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  AreaSoundFinish                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundFinish frees up all memory used by the area sound module     */
/*  after turning off all the sounds.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundFinish(T_void)
{
    DebugRoutine("AreaSoundFinish") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    AreaSoundCheck() ;

    /* Destroy all the sounds currently being used. */
    /* This also frees up all the memory. */
    while (P_firstAreaSound != NULL)
        AreaSoundDestroy(P_firstAreaSound) ;

    G_areaSoundInit = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AreaSoundLoad                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundLoad loads a whole level of area sounds for the given level  */
/*  number.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 mapNumber          -- Map/level sounds to load               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    sprintf                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundLoad(T_word32 mapNumber)
{
    DebugRoutine("AreaSoundLoad") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    AreaSoundCheck() ;

    /* First, destroy all the sounds currently being used. */
    /* This also frees up all the memory. */
    while (P_firstAreaSound != NULL)
        AreaSoundDestroy(P_firstAreaSound) ;

#if 0
    /* Identify the file we wish to load. */
    sprintf(filename, "l%ld.sou", mapNumber) ;

    /* Open it. */
    fp = fopen(filename, "r") ;

    /* If we open it, read the number of items and then all the items. */
    DebugCheck(fp != NULL) ;
    if (fp != NULL)  {
        /* Get the number of area sounds in this file. */
        fscanf(fp, "%d", &num) ;

        /* Loop and get each one. */
        for (i=0; i<num; i++)  {
            /* Read in a single set of numbers. */
            fscanf(fp, "%d%d%d%d%s%d%d%d",
                &v1,          /* x */
                &v2,          /* y */
                &v3,          /* radius */
                &v4,          /* volume */
                type,         /* type */
                &v5,          /* length */
                &v6,          /* group Id */
                &v7) ;        /* sound num */

            x = v1 ;
            y = v2 ;
            radius = v3 ;
            volume = v4 ;
            length = v5 ;
            groupId = v6 ;
            soundNum = v7 ;

            /* Identify the true type of sound. */
            if (type[0] == 'L')
                soundType = AREA_SOUND_TYPE_LOOP ;
            else
                soundType = AREA_SOUND_TYPE_ONCE ;

            if (groupId == 0)  {
                p_groupLeader = NULL ;
            } else  {
                p_groupLeader = IFindGroupLeader(groupId) ;
            }

            self = AreaSoundCreate(
                       x,
                       y,
                       radius,
                       volume,
                       soundType,
                       length,
                       p_groupLeader,
                       NULL,
                       0,
                       soundNum) ;
AreaSoundCheck() ;

            if (self != NULL)
                if ((groupId != 0) && (p_groupLeader == NULL))  {
                     ISetGroupLeaderAndId(self, self, groupId) ;
                }
        }
        fclose(fp) ;
    }
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AreaSoundCreate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundCreate creates and sets up a new area sound.  As well, it    */
/*  sets up the new location and placement of the area sound in reference   */
/*  to its group.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x, y              -- Location of area sound                 */
/*                                                                          */
/*    T_word16 radius             -- Radius at point zero sound             */
/*                                                                          */
/*    T_word16 volume             -- Maximum volume at center               */
/*                                                                          */
/*    E_areaSoundType type        -- One shot, or loop                      */
/*                                                                          */
/*    T_word16 length             -- How long is sound to play (not used    */
/*                                   if looping)                            */
/*                                                                          */
/*    T_areaSound p_groupLeader   -- Connect to which group?                */
/*                                                                          */
/*    T_areaSoundFinishCallback p_callback -- who to call when ONCE done    */
/*                                                                          */
/*    T_word16 soundNum           -- Number of sound to play                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_areaSound                 -- area sound handle that was created     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*    LES  04/27/95  Added delete flag and scheduled event if the sound     */
/*                   is not looping.                                        */
/*                   Added data for callback routine.                       */
/*                                                                          */
/****************************************************************************/

T_areaSound AreaSoundCreate(
                T_sword16 x,
                T_sword16 y,
                T_word16 radius,
                T_word16 volume,
                E_areaSoundType type,
                T_word16 length,
                T_areaSound p_groupLeader,
                T_areaSoundFinishCallback p_callback,
                T_word32 data,
                T_word16 soundNum)
{
    T_areaSoundStruct *p_sound ;

    DebugRoutine("AreaSoundCreate") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    AreaSoundCheck() ;

    /* First check to see if sound is turned on. */
    if (SoundIsOn()==TRUE)  {
        /* Allocate memory for the sound. */
        p_sound = MemAlloc(sizeof(T_areaSoundStruct)) ;

        /* If sound is allocated ... */
        DebugCheck(p_sound != NULL) ;
        if (p_sound != NULL)  {
            /* Initialize the fields as we can. */
            p_sound->tag = AREA_SOUND_ID ;
            p_sound->x = x ;
            p_sound->y = y ;
            p_sound->radius = radius ;
            p_sound->maxVolume = volume ;
            p_sound->currentVolume = 0 ;
            p_sound->type = type ;
            p_sound->length = length ;
            p_sound->group = (T_areaSoundStruct *)p_groupLeader ;
            p_sound->data = data ;
            p_sound->id = G_nextID++ ;
            p_sound->markedForDestroy = FALSE;
            if (p_groupLeader != NULL)  {
                /* Make the group id match the group leader (in case */
                /* the group leader is destroyed. */
                DebugCheck(p_sound->group->tag == AREA_SOUND_ID) ;
                p_sound->groupId = p_sound->group->groupId ;
            } else {
                /* Must be an independent. */
                p_sound->groupId = 0 ;
            }
            p_sound->p_callback = p_callback ;
            p_sound->soundNum = soundNum ;
            /* No channel given to this sound yet. */
            p_sound->channel = SOUND_BAD ;

            /* Place on list of sounds. */
            if (P_lastAreaSound != NULL)  {
                P_lastAreaSound->next = p_sound ;
            } else  {
                P_firstAreaSound = p_sound ;
            }
            P_lastAreaSound = p_sound ;
            p_sound->next = NULL ;

            DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
        }
    } else {
        p_sound = NULL ;
    }

    DebugEnd() ;

    return((T_areaSound)p_sound) ;
}

/****************************************************************************/
/*  Routine:  AreaSoundDestroy                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundDestroy turns off a given sound AND removes it from the list */
/*  of sounds.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSound areaSound       -- Area sound to destroy                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemFree                                                               */
/*    IFindPrevSound                                                        */
/*    TriggerOff                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*    LES  04/27/95  Added callback call (with check)                       */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundDestroy(T_areaSound areaSound)
{
    T_areaSoundStruct *p_sound ;
    T_areaSoundStruct *p_prev ;
    static E_Boolean insideHere = FALSE ;
    T_word32 timeOut ;

    DebugRoutine("AreaSoundDestroy") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    DebugCheck(areaSound != NULL) ;
    DebugCheck(((T_areaSoundStruct *)areaSound)->tag == AREA_SOUND_ID) ;
    AreaSoundCheck() ;

    if (insideHere == TRUE)  {
        DebugEnd() ;
        return ;
    }
    /* Get a quick pointer to the sound. */
    p_sound = (T_areaSoundStruct *)areaSound ;

    DebugCheck(p_sound->channel != ((T_sword16)0xCECE)) ;
    DebugCheck(p_sound->channel != ((T_sword16)0xCDCD)) ;
    DebugCheck(p_sound->channel != ((T_sword16)0xCCCC)) ;
    /* Turn off the sound if it is playing. */
    if (p_sound->channel != SOUND_BAD)  {
        if (SoundIsOn())  {
            /* You have 1/2 second to stop the sound */
            timeOut = TickerGet() + 35 ;
            while ((p_sound->channel != SOUND_BAD) && (TickerGet() < timeOut))  {
                insideHere = TRUE ;
                SoundStop(p_sound->channel) ;
                SoundUpdate() ;
                insideHere = FALSE ;
            }
        } else {
            /* If no sound, make sure we have no channel assigned. */
            p_sound->channel = SOUND_BAD ;
        }
        /* If we are looping, destroy us NOW. */
        if (p_sound->type == AREA_SOUND_TYPE_LOOP)
            AreaSoundDestroy(areaSound) ;
    } else {
        /* If there is a callback routine, call it before we destroy it. */
        if (p_sound->p_callback != NULL)
            p_sound->p_callback(p_sound, p_sound->data) ;

        /* Remove the sound from the list. */
        p_prev = IFindPrevSound(p_sound) ;
        if (p_prev == NULL)  {
            P_firstAreaSound = p_sound->next ;
        } else {
            p_prev->next = p_sound->next ;
        }
        if (P_lastAreaSound == p_sound)
            P_lastAreaSound = p_prev ;

        /* Tag the sound as truly freed. */
        p_sound->tag = AREA_SOUND_ID_DONE ;

        /* Now that we are freed from the link, remove the sound from memory. */
        MemFree(p_sound) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AreaSoundMove                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundMove changes the placement of the given area sound to the    */
/*  new location.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSound areaSound       -- area sound to move                     */
/*                                                                          */
/*    T_sword16 newX, newY        -- New XY location of sound               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundMove(T_areaSound areaSound, T_sword16 newX, T_sword16 newY)
{
    DebugRoutine("AreaSoundMove") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    DebugCheck(((T_areaSoundStruct *)areaSound)->tag == AREA_SOUND_ID) ;
    AreaSoundCheck() ;

    ((T_areaSoundStruct *)areaSound)->x = newX ;
    ((T_areaSoundStruct *)areaSound)->y = newY ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AreaSoundUpdate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundUpdate updates the volumes and activity of all sounds        */
/*  based on the sound areas and the given listening location.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position where sounds are heard.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IUpdateActiveSounds                                                   */
/*    IUpdateInactiveSounds                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundUpdate(T_sword16 listenX, T_sword16 listenY)
{
    DebugRoutine("AreaSoundUpdate") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    AreaSoundCheck() ;

//    IDestroyMarked() ;
    ICalculateAllVolumes(listenX, listenY) ;
    IUpdateActiveSounds(listenX, listenY) ;
    IUpdateInactiveSounds(listenX, listenY) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFindGroupLeader                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFindGroupLeader searches through the list of area sounds and finds   */
/*  the first instance of a given areaSound.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 groupId            -- Group id to find leader of.            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_areaSound                 -- Found group leader, or AREA_SOUND_BAD  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_areaSound IFindGroupLeader(T_word16 groupId)
{
    T_areaSoundStruct *p_areaSound ;

    DebugRoutine("IFindGroupLeader") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    AreaSoundCheck() ;

    p_areaSound = P_firstAreaSound ;
    while (p_areaSound != NULL)  {
        DebugCheck(p_areaSound->tag == AREA_SOUND_ID) ;
        if (p_areaSound->groupId == groupId)
            break ;
        p_areaSound = p_areaSound->next ;
    }

    DebugEnd() ;

    return((T_areaSound)p_areaSound) ;
}

/****************************************************************************/
/*  Routine:  ISetGroupLeaderAndId                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISetGroupLeaderAndId changes the group leader and id for the given    */
/*  instance.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSound areaSound       -- area sound to change group leader      */
/*                                                                          */
/*    T_areaSound groupLeader     -- Group leader to change to              */
/*                                                                          */
/*    T_word16 groupId            -- Group Id to change to                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ISetGroupLeaderAndId(
           T_areaSound areaSound,
           T_areaSound groupLeader,
           T_word16 groupId)
{
    DebugRoutine("ISetGroupLeaderAndId") ;
    DebugCheck(G_areaSoundInit == TRUE) ;
    DebugCheck(((T_areaSoundStruct *)areaSound)->tag == AREA_SOUND_ID) ;
    AreaSoundCheck() ;

    ((T_areaSoundStruct *)areaSound)->group = groupLeader ;
    ((T_areaSoundStruct *)areaSound)->groupId = groupId ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFindPrevSound                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFindPrevSound searches in the linked list for the previous sound     */
/*  to the given sound.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSoundStruct *p_sound  -- Sound to find previous of              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_areaSoundStruct *         -- Previous sound, or NULL                */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_areaSoundStruct *IFindPrevSound(T_areaSoundStruct *p_sound)
{
    T_areaSoundStruct *p_prev ;
    T_areaSoundStruct *p_where ;

    DebugRoutine("IFindPrevSound") ;
#ifndef NDEBUG
    if(p_sound->tag != AREA_SOUND_ID) {
        printf("Bad sound %p\n", p_sound);  fflush(stdout) ;
        if (p_sound != NULL)  {
            printf("tag %4.4.s (%08lX)\n", p_sound, p_sound->tag) ;  fflush(stdout) ;
        }
    }
#endif
    DebugCheck(p_sound != NULL) ;
    DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
    AreaSoundCheck() ;

    /* Keep track of the previous and the current location. */
    p_prev = NULL ;
    p_where = P_firstAreaSound ;

    /* Search through the list until we find the match.  Prev is one */
    /* element behind. */
    while (p_where != NULL)  {
        if (p_where == p_sound)
             break ;
        p_prev = p_where ;
        p_where = p_where->next ;
    }

    DebugEnd() ;

    return p_prev ;
}


/****************************************************************************/
/*  Routine:  IUpdateActiveSounds                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUpdateActiveSounds go through the listof area sounds and identifies  */
/*  those sounds that are active and determines their new volume level      */
/*  and if they should be turned off.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position where sounds are heard.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    TriggerStatus                                                         */
/*    ICalculateVolume                                                      */
/*    TriggerVolume                                                         */
/*    AreaSoundDestroy                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*    LES  04/27/95  Added check to see if sound needs to be deleted.       */
/*    LES  04/08/96  Stopped using SoundIsDone                              */
/*                                                                          */
/****************************************************************************/

T_void IUpdateActiveSounds(T_sword16 listenX, T_sword16 listenY)
{
    T_areaSoundStruct *p_sound ;
    T_word16 volume ;
    T_word16 pan ;

    DebugRoutine("IUpdateActiveSounds") ;
    AreaSoundCheck() ;

    p_sound = P_firstAreaSound ;
    while (p_sound != NULL)  {
        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;

        /* Look for area sounds that are assigned to a channel. */
        if (p_sound->channel != SOUND_BAD)  {
            /* OK, found one. */
            volume = p_sound->currentVolume ;

            if (volume != 0)  {
                /* Change the volume level. */
                SoundSetVolume(p_sound->channel, ((volume * SoundGetEffectsVolume())>>8)) ;
                pan = IAreaSoundDeterminePan(p_sound, listenX, listenY) ;
                SoundSetStereoPanLocation(p_sound->channel, pan) ;
            } else {
                /* The volume is zero, there is no need to play */
                /* this sound! */
                SoundStop(p_sound->channel) ;
            }
        }
        p_sound = p_sound->next ;
    }

    /* Sound being updated. */
    SoundUpdate() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IUpdateInactiveSounds                                         */
/****************************************************************************/
/*                                                                           */
/*  Description:                                                            */
/*                                                                          */
/*    IUpdateInactiveSounds does the work of determining if there are new   */
/*  sounds that can be played or "take over" other sounds.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position where sounds are heard.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IUpdateInactiveSounds(T_sword16 listenX, T_sword16 listenY)
{
    T_areaSoundStruct *p_sound ;
    E_Boolean allowFreqShift ;

    DebugRoutine("IUpdateInactiveSounds") ;
    AreaSoundCheck() ;

    /* Keep playing area sounds if there is room for them in the */
    /* sound system. */
    while (1)  {
        /* What sound is the loudest and is not being played. */
        p_sound = IFindLoudestNotChannelSound() ;

        /* If no sound was found, stop doing this processing.  */
        /* There is nothing more useful to do. */
        if (p_sound == NULL)  {
            break ;
        } else {
        }

        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
        /* Otherwise, we HAVE found another sound that is not being */
        /* played that needs to be played.  So, let's play it. */
        allowFreqShift = SoundGetAllowFreqShift() ;
        if (p_sound->type == AREA_SOUND_TYPE_ONCE)
            SoundSetAllowFreqShift(TRUE) ;
        else
            SoundSetAllowFreqShift(FALSE) ;
        if (p_sound->type == AREA_SOUND_TYPE_ONCE)  {
            p_sound->channel =
                SoundPlayByNumberWithCallback(
                    p_sound->soundNum,
                    p_sound->currentVolume,
                    ISoundForAreaSoundIsDone,
                    (T_void *)(p_sound->id)) ;
        } else  {
            p_sound->channel =
                SoundPlayLoopByNumberWithCallback(
                    p_sound->soundNum,
                    p_sound->currentVolume,
                    ISoundForAreaSoundIsDone,
                    (T_void *)(p_sound->id)) ;
        }
        SoundSetAllowFreqShift(allowFreqShift) ;

        /* Stop if the channel could not be played. */
        if (p_sound->channel == SOUND_BAD)  {
            ISoundForAreaSoundIsDone((T_void *)(p_sound->id));
            break ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ICalculateVolume                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICalculateVolume determines the volume level of a sound given the     */
/*  listening location.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSoundStruct *p_sound  -- Pointer to sound to calc. volume.      */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position where sounds are heard.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CalculateDistance                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/25/95  Created                                                */
/*    LES  04/26/95  Fixed if/else bug.  (left off the else)                */
/*                                                                          */
/****************************************************************************/

T_word16 ICalculateVolume(
             T_areaSoundStruct *p_sound,
             T_sword16 listenX,
             T_sword16 listenY)
{
    T_word16 volume ;
    T_word16 distance ;

    DebugRoutine("ICalculateVolume") ;
    DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
    AreaSoundCheck() ;

    /* Are far is the sound source? */
    distance = CalculateDistance(
                    p_sound->x,
                    p_sound->y,
                    listenX,
                    listenY) ;

    /* If the distance is too far, volume is zero. */
    if (distance >= p_sound->radius)
        if (p_sound->type == AREA_SOUND_TYPE_ONCE)
            volume = 10 ;
        else
            volume = 0 ;
    else
        /* If not, use the distance to scale the max sound. */
        volume = (((T_word32)p_sound->maxVolume) *
                     ((T_word32)(p_sound->radius - distance))) /
                         p_sound->radius ;
    DebugEnd() ;

    return volume ;
}

/****************************************************************************/
/*  Routine:  ICalculateAllVolumes                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICalculateAllVolumes is a routine that determines volume levels       */
/*  for all the area sounds.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position where sounds are heard.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ICalculateVolume                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ICalculateAllVolumes(T_sword16 listenX, T_sword16 listenY)
{
    T_areaSoundStruct *p_sound ;

    DebugRoutine("ICalculateAllVolumes") ;
    AreaSoundCheck() ;

    p_sound = P_firstAreaSound ;
    while (p_sound != NULL)  {
        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
        /* Calculate the sounds. */
        p_sound->currentVolume =
            ICalculateVolume(p_sound, listenX, listenY) ;

        /* If in a group, make the leader equal the louder of the two. */
        if (p_sound->group != NULL)  {
             DebugCheck(p_sound->group->tag == AREA_SOUND_ID) ;
             if (p_sound->currentVolume > p_sound->group->currentVolume)
                  p_sound->group->currentVolume = p_sound->currentVolume;
        }
        p_sound = p_sound->next ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFindLoudestNotChannelSound                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFindLoudestNotChannelSound searches throught the area sounds to      */
/*  find the loudest audible sound that is not being played.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_areaSoundStruct *         -- loudest non-playing sound, or NULL     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_areaSoundStruct *IFindLoudestNotChannelSound(T_void)
{
    T_areaSoundStruct *p_sound ;
    T_areaSoundStruct *p_soundLoudest = NULL ;
    T_word16 loudestVolume = 0 ;

    DebugRoutine("IFindLoudestNotChannelSound") ;
    AreaSoundCheck() ;

    p_sound = P_firstAreaSound ;
    while (p_sound != NULL)  {
        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
        /* Pick only sounds that are NOT being played. */
        if (p_sound->channel == SOUND_BAD)  {
            /* Pick only group leaders or non group members. */
            if ((p_sound->group == NULL) ||
                (p_sound->group == p_sound))  {
                /* Compare volumes as we go through the list. */
                if (p_sound->currentVolume >= loudestVolume)  {
                    /* Check if we are at volume 0.  If we are, */
                    /* volume 0 is played for one time sounds, but */
                    /* not for area snds. */
                    if ((p_sound->currentVolume != 0) ||
                         (p_sound->type == AREA_SOUND_TYPE_ONCE))  {
                        loudestVolume = p_sound->currentVolume ;
                        p_soundLoudest = p_sound ;
                    }
                }
            }
        }

        p_sound = p_sound->next ;
    }

    DebugEnd() ;

    return p_soundLoudest ;
}

/****************************************************************************/
/*  Routine:  AreaSoundCheck                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundCheck is a routine only used in debugging to check to        */
/*  see that the sounds have not been corrupted.  If there is a problem,    */
/*  it will crash the program.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/27/95  Created                                                */
/*    LES  05/08/95  Added more checks.                                     */
/*                                                                          */
/****************************************************************************/

#ifndef NDEBUG
T_void AreaSoundCheck(T_void)
{
    T_areaSoundStruct *p_sound ;

    DebugRoutine("AreaSoundCheck") ;
    DebugCheck(G_areaSoundInit == TRUE) ;

    /* Convert the data into a pointer to the sound to cancel. */
    p_sound = P_firstAreaSound ;

    while (p_sound != NULL)  {
        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;
        DebugCheck(p_sound->maxVolume < 256) ;
        DebugCheck(p_sound->currentVolume < 256) ;
        DebugCheck(p_sound->currentVolume <= p_sound->maxVolume) ;
        DebugCheck(p_sound->type < AREA_SOUND_TYPE_UNKNOWN) ;
        DebugCheck((p_sound->channel >= SOUND_BAD) && (p_sound->channel <= 31)) ;
        DebugCheck(((T_word16)p_sound->channel) != 0xCDCD) ;
        DebugCheck(p_sound->soundNum >= 0) ;
        p_sound = p_sound->next ;
    }

    DebugEnd() ;
}
#endif

/****************************************************************************/
/*  Routine:  IAreaSoundDeterminePan                  * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IAreaSoundDeterminePan is called to calculate the panning value of    */
/*  the sound in reference to the listening x, y position.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSoundStruct *p_sound  -- Sound to calculate pan                 */
/*                                                                          */
/*    T_sword16 listenX, listenY  -- Position of listener                   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MathArcTangent                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/18/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 IAreaSoundDeterminePan(
                    T_areaSoundStruct *p_sound,
                    T_sword16 listenX,
                    T_sword16 listenY)
{
    T_sword16 dx, dy ;
    T_word16 pan, angle ;

    DebugRoutine("IAreaSoundDeterminePan") ;
    DebugCheck(p_sound != NULL) ;

    if (p_sound)  {
        DebugCheck(p_sound->tag == AREA_SOUND_ID) ;

        dx = p_sound->x - listenX ;
        dy = p_sound->y - listenY ;
        angle = MathArcTangent(dx, dy) ;
        angle -= 0x4000 ;
        angle -= PlayerGetAngle() ;

        if (angle & 0x8000)  {
            pan = 1+(0xFFFF-angle) ;
        } else {
            pan = angle ;
        }
        pan <<= 1 ;
    }

    DebugEnd() ;

    return pan ;
}

/****************************************************************************/
/*  Routine:  ISoundForAreaSoundIsDone                * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISoundForAreaSoundIsDone is called to say, "your sound is complete"   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine is called by an interrupt, therefore, no debug routines  */
/*  are allowed.  DONT add them.                                            */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_data              -- Pointer to the area sound              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ISoundForAreaSoundIsDone(
                  T_void *p_data)
{
    T_areaSoundStruct *p_areaSound ;
    T_word32 id ;

    DebugRoutine("ISoundForAreaSoundIsDone") ;

    id = (T_word32)p_data ;
//printf("Area sound %d is done\n", id) ; fflush(stdout) ;
    p_areaSound = IFindAreaSoundByID(id) ;

    if (p_areaSound)  {
        DebugCheck(p_areaSound->tag == AREA_SOUND_ID) ;

        /* Mark this sound has having no channel. */
        p_areaSound->channel = SOUND_BAD ;

        /* If the sound is done and the type is once, destroy it. */
        if (p_areaSound->type == AREA_SOUND_TYPE_ONCE)  {
            p_areaSound->markedForDestroy = TRUE ;
            AreaSoundDestroy((T_areaSound)p_areaSound) ;
        }
    }

    DebugEnd() ;
}

/* LES: 04/09/96  Search for area sound in list by its id */
static T_areaSoundStruct *IFindAreaSoundByID(T_word32 id)
{
    T_areaSoundStruct *p_find ;

    DebugRoutine("IFindAreaSoundByID") ;

    /* Go through the link list and stop if you find a */
    /* matching id, otherwise, progress to the end (NULL) */
    p_find = P_firstAreaSound ;
    while (p_find != NULL)  {
        if (p_find->id == id)
            break ;
        p_find = p_find->next ;
    }

    DebugEnd() ;

    return p_find ;
}

/****************************************************************************/
/*  Routine:  AreaSoundFindGroupLeader                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundFindGroupLeader finds the group leader for a given group id. */
/*  returns AREA_SOUND_BAD if no group is found, or groupID is zero.        */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 groupID            -- ID of group                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_areaSound                 -- identifier for area sound              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_areaSound AreaSoundFindGroupLeader(T_word16 groupID)
{
    T_areaSound groupLeader = AREA_SOUND_BAD ;

    DebugRoutine("AreaSoundFindGroupLeader") ;

    if (groupID != 0)
        groupLeader = IFindGroupLeader(groupID) ;

    DebugEnd() ;

    return groupLeader ;
}


/****************************************************************************/
/*  Routine:  AreaSoundSetGroupId                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    AreaSoundSetGroupId changes the group id on an area sound.            */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSound areaSound       -- Area sound to affect                   */
/*                                                                          */
/*    T_word16 groupID            -- ID of the group to attach sound        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AreaSoundSetGroupId(T_areaSound areaSound, T_word16 groupID)
{
    T_areaSoundStruct *p_sound ;

    DebugRoutine("AreaSoundSetGroupId") ;
//    DebugCheck(areaSound != NULL) ;
    if (areaSound != NULL)  {
        DebugCheck(G_areaSoundInit == TRUE) ;
        DebugCheck(((T_areaSoundStruct *)areaSound)->tag == AREA_SOUND_ID) ;
        AreaSoundCheck() ;

        /* Get a quick pointer to the sound. */
        p_sound = (T_areaSoundStruct *)areaSound ;

        p_sound->groupId = groupID ;
    }
    DebugEnd() ;
}

#if 0
static T_void IDestroyMarked(T_void)
{
    T_areaSoundStruct *p_find ;

    DebugRoutine("IDestroyMarked") ;

    /* Go through the link list and stop if you find a */
    /* matching id, otherwise, progress to the end (NULL) */
    p_find = P_firstAreaSound ;
    while (p_find != NULL)  {
        if (p_find->markedForDestroy == TRUE)
            AreaSoundDestroy((T_areaSound)p_find) ;

        p_find = p_find->next ;
    }

    DebugEnd() ;
}
#endif

/****************************************************************************/
/*    END OF FILE:  AREASND.C                                               */
/****************************************************************************/
