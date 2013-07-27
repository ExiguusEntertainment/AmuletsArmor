/****************************************************************************/
/*    FILE:  DOOR.C                                                         */
/****************************************************************************/
#include "AREASND.H"
#include "DOOR.H"
#include "MAP.H"
#include "MEMORY.H"
#include "SCHEDULE.H"
#include "SLIDER.H"
#include "SYNCTIME.H"

#define MAX_DOOR_LOCK         0xFF

#define DOOR_TAG        ((((T_word32)'D') << 0) | \
                         (((T_word32)'O') << 8) | \
                         (((T_word32)'o') << 16) | \
                         (((T_word32)'R') << 24))

#define DOOR_DELETE_TAG ((((T_word32)'D') << 0) | \
                         (((T_word32)'D') << 8) | \
                         (((T_word32)'e') << 16) | \
                         (((T_word32)'l') << 24))

typedef struct T_doorTag {
    T_word32 tag ;
    T_word16 sector ;
    T_sword16 low, high ;
    T_word16 speed ;
    T_word16 pause ;
    T_word16 locked ;
    struct T_doorTag *next ;
    T_sword16 soundX, soundY ;     /* Equicenter of sound effect. */
    T_areaSound sound ;
    T_word16 requiredItem ;
} T_door;

typedef struct {
    T_door *firstDoor ;
    E_Boolean doorInit ;
} T_doorGroupHandleStruct ;

static T_door *G_firstDoor = NULL ;
static E_Boolean G_doorInit = FALSE ;

#define DOOR_IS_UNLOCKED 0
#define DOOR_IS_LOCKED   DOOR_ABSOLUTE_LOCK
#define DOOR_SOUND_RADIUS 1500
#define DOOR_SOUND_NUMBER 5000
#define DOOR_SOUND_MAX_VOLUME 255

/* Internal prototypes: */
static T_void IDoorUnload(T_void) ;
static T_door *IFindDoor(T_word16 sector) ;
static T_sliderResponse IHandleDoorOpening(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;
static T_sliderResponse IHandleDoorClosing(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone) ;
static T_void IStartDoorClosing(T_word32 data) ;
static T_void IHandleSoundCallback(T_areaSound sound, T_word32 data) ;


/****************************************************************************/
/*  Routine:  DoorInitialize                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorInitialize starts up all the information needed for handling      */
/*  doors.                                                                  */
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
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorInitialize(T_void)
{
    DebugRoutine("DoorInitialize") ;
    DebugCheck(G_doorInit == FALSE) ;

    /* Make sure we have nothing in our list of sliders. */
    G_firstDoor = NULL ;

    G_doorInit = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorFinish                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorFinish cleans up the door module and unloads all memory.          */
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
/*    IDoorUnload                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorFinish(T_void)
{
    DebugRoutine("DoorFinish") ;
    DebugCheck(G_doorInit == TRUE) ;

    IDoorUnload() ;

    /* Note that we are done. */
    G_doorInit = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorLoad                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorLoad loads the appropriate map door file.                         */
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
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*    LES  04/27/95  Loads in equicenter sound data.                        */
/*    LES  05/08/95  Added memory tag for better checking.                  */
/*                                                                          */
/****************************************************************************/

T_void DoorLoad(T_word32 mapNumber)
{
    DebugRoutine("DoorLoad") ;
    DebugCheck(G_doorInit == TRUE) ;

    /* If there are doors loaded, remove them. */
    if (G_firstDoor != NULL)
        IDoorUnload() ;

#if 0
    /* Create the appropriate filename and try to open. */
    sprintf(filename, "l%ld.dor", mapNumber) ;
    fp = fopen(filename, "r") ;
    if (fp != NULL)  {
        /* If it is opened, get the number of doors found. */
        fscanf(fp, "%d", &numDoors) ;

        for (i=0; i<numDoors; i++)  {
            /* Allocate the needed memory. */
            p_door = MemAlloc(sizeof(T_door)) ;

            /* Read in the door's data. */
            fscanf(fp, "%d%d%d%d%d%d%d%d",
                &v1,        /* sector */
                &v2,        /* low */
                &v3,        /* high */
                &v4,        /* speed */
                &v5,        /* pause */
                &v6,        /* locked. */
                &v7,        /* soundX */
                &v8) ;      /* soundY */

            p_door->sector = v1 ;
            p_door->low = v2 ;
            p_door->high = v3 ;
            p_door->speed = v4 ;
            p_door->pause = v5 ;
            p_door->locked = v6 ;
            p_door->soundX = v7 ;
            p_door->soundY = v8 ;

            /* Tag this door. */
            p_door->tag = DOOR_TAG ;

            /* Note that we don't have any sound yet attached to the door */
            p_door->sound = NULL ;

            /* Link into the linked list (reversed order). */
            p_door->next = G_firstDoor ;
            G_firstDoor = p_door ;
        }
        fclose(fp) ;
    }
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorLock                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorLock locks a door.  If it is already locked, it stays locked.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    A door is only locked when it is fully closed.  A locked open door    */
/*  reacts normally until fully closed -- then it is truly locked.          */
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
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorLock(T_word16 sector)
{
    T_door *p_door ;

    DebugRoutine("DoorLock") ;
    DebugCheck(G_doorInit == TRUE) ;
//printf("Door %d lock\n", sector) ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;
    DebugCheck(p_door->tag == DOOR_TAG) ;

    /* If we found one, lock it. */
    if (p_door != NULL)
        p_door->locked = DOOR_IS_LOCKED ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorUnlock                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorUnlock unlocks a door.  If it is already unlocked, nothing        */
/*  happens.                                                                */
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
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorUnlock(T_word16 sector)
{
    T_door *p_door ;

    DebugRoutine("DoorUnlock") ;
    DebugCheck(G_doorInit == TRUE) ;
//printf("Door %d unlock\n", sector) ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;
    DebugCheck(p_door->tag == DOOR_TAG) ;

    /* If we found one, unlock it. */
    if (p_door != NULL)
        p_door->locked = DOOR_IS_UNLOCKED ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorIsLock                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorIsLock return TRUE if the door is locked, or return FALSE.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    DoorIsLock pretty much assumes the given sector is a door, else       */
/*  will always return FALSE.                                               */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE if locked, else FALSE             */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean DoorIsLock(T_word16 sector)
{
    T_door *p_door ;
    E_Boolean status = FALSE ;

    DebugRoutine("DoorIsLock") ;
    DebugCheck(G_doorInit == TRUE) ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;
    DebugCheck(p_door->tag == DOOR_TAG) ;

    /* If we found one, unlock it. */
    if (p_door != NULL)  {
        if (p_door->locked != 0)  {
            status = TRUE ;
        } else  {
            status = FALSE ;
        }
    }

    DebugEnd() ;

    return status ;
}

/****************************************************************************/
/*  Routine:  DoorOpen                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorOpen opens a door and starts the appropriate slider unless the    */
/*  the door is locked.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector with door to open.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*    LES  04/27/95  Added door sound effect                                */
/*    LES  12/22/95  Changed to return a boolean on if the door was opened  */
/*                                                                          */
/****************************************************************************/

E_Boolean DoorOpen(T_word16 sector)
{
    T_door *p_door ;
    T_sword16 doorHeight ;
    E_Boolean didOpen = FALSE ;

    DebugRoutine("DoorOpen") ;
    DebugCheck(G_doorInit == TRUE) ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;

    if (p_door != NULL)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        /* Now is the door locked? */
        if (p_door->locked == DOOR_IS_UNLOCKED)  {
            /* Door is unlocked. */
            /* Is door already opened? */
            doorHeight = MapGetCeilingHeight(sector) ;
            if (doorHeight <= p_door->high)  {
                 /* Door is at least partially closed, try opening */
                 /* it. */
                 SliderStart(
                     ((T_word32)sector) | (SLIDER_TYPE_CEILING<<16),
                     (doorHeight << 16),
                     ((T_sword32)p_door->high) << 16,
                     p_door->speed,
                     IHandleDoorOpening,
                     0xFFFF) ;
                 /* Do the door sound too. */
                 if (p_door->sound == NULL)  {
                     p_door->sound = AreaSoundCreate(
                                         p_door->soundX,
                                         p_door->soundY,
                                         DOOR_SOUND_RADIUS,
                                         DOOR_SOUND_MAX_VOLUME,
                                         AREA_SOUND_TYPE_ONCE,
                                         20,
                                         NULL,
                                         NULL /* IHandleSoundCallback */,
                                         (T_word32)p_door,
                                         DOOR_SOUND_NUMBER) ;
                 }

                 didOpen = TRUE ;
            }
        }
    }

    DebugEnd() ;

    return didOpen ;
}

/****************************************************************************/
/*  Routine:  DoorClose                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorClose closes a door and starts the appropriate slider.            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector with door to close              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*    SliderStart                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorClose(T_word16 sector)
{
    T_door *p_door ;
    T_sword16 doorHeight ;

    DebugRoutine("DoorClose") ;
    DebugCheck(G_doorInit == TRUE) ;
//printf("Door %d close\n", sector) ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;
    DebugCheck(p_door->tag == DOOR_TAG) ;

    if (p_door != NULL)  {
        /* Doesn't matter if door is locked, close the door. */
        doorHeight = MapGetCeilingHeight(sector) ;
        SliderStart(
            ((T_word32)sector) | (SLIDER_TYPE_CEILING<<16),
            (doorHeight << 16),
//            ((T_sword32)p_door->high) << 16,
            ((T_sword32)p_door->low) << 16,
            p_door->speed,
            IHandleDoorClosing,
            0xFFFF) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorForceOpen                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorForceOpen opens a door whether or not it is locked.               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector with door to open.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoorIsLock                                                            */
/*    DoorOpen                                                              */
/*    DoorLock                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorForceOpen(T_word16 sector)
{
    E_Boolean lock ;

    DebugRoutine("DoorForceOpen") ;
    DebugCheck(G_doorInit == TRUE) ;

    /* See if the door is locked. */
    lock = DoorIsLock(sector) ;

    /* If it is, unlock it. */
    if (lock == TRUE)
        DoorUnlock(sector) ;

    /* Open the door. */
    DoorOpen(sector) ;

    /* If it is supposed to be locked, lock it after initiating the */
    /* door to open.  This means that the door will go ahead and cycle */
    /* and when it finally closes, it then will be locked. */
    if (lock == TRUE)
        DoorLock(sector) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDoorUnload                        * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDoorUnload returns all memory to the system used by the door module. */
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
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*    LES  05/08/95  Make use of deleted tags.                              */
/*                                                                          */
/****************************************************************************/

static T_void IDoorUnload(T_void)
{
    T_door *p_door ;
    T_door *p_next ;
    T_word32 slider ;

    DebugRoutine("IDoorUnload") ;

    /* Get the first door. */
    p_door = G_firstDoor ;

    /* Go through the linked list and delete the door structures. */
    while (p_door != NULL)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        p_next = p_door->next ;

        /* Stop the slider for the door. */
        slider = ((T_word32)(p_door->sector)) | (SLIDER_TYPE_CEILING<<16) ;
        if (SliderExist(slider))  {
            SliderCancel(slider) ;
            SliderDestroy(slider) ;
        }

        /* Mark this door as deleted. */
        p_door->tag = DOOR_DELETE_TAG ;
        MemFree(p_door) ;
        p_door = p_next ;
    }

    /* Make the first door point to nothing. */
    G_firstDoor = NULL ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFindDoor                          * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFindDoor is a utility routine that returns a pointer to a door given */
/*  the sector number.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector door is at.                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_door *                    -- Pointer to door or NULL if not found.  */
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
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_door *IFindDoor(T_word16 sector)
{
    T_door *p_door ;

    DebugRoutine("IFindDoor") ;

    /* Start with the first door. */
    p_door = G_firstDoor ;

    /* Loop until we find a match or at the end of the list. */
    while (p_door != NULL)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        if (p_door->sector == sector)
            break ;
        p_door = p_door->next ;
    }

    DebugEnd() ;

    /* Return what we found. */
    return p_door ;
}

/****************************************************************************/
/*  Routine:  IHandleDoorOpening                 * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IHandleDoorOpening is called as a door opens up.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Which sliding item.                    */
/*                                                                          */
/*    T_sword32 value             -- New ceiling height                     */
/*                                                                          */
/*    E_Boolean isDone            -- Flag telling if this is the last update*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MapSetCeilingHeight                                                   */
/*    ScheduleAddEvent                                                      */
/*    IStartDoorClosing (indirectly)                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*    LES  04/27/95  Modified to stop door sound.                           */
/*                                                                          */
/****************************************************************************/

static T_sliderResponse IHandleDoorOpening(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;
    T_door *p_door ;
    DebugRoutine("IHandleDoorOpening") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* Change that ceiling's height. */
    MapSetCeilingHeight(sector, (T_sword16)(value>>16)) ;
    if (isDone == TRUE)  {
        p_door = IFindDoor(sector) ;
        DebugCheck(p_door != NULL) ;
        DebugCheck(p_door->tag == DOOR_TAG) ;

        if (p_door != NULL)  {
            if (TRUE)  {
                 ScheduleAddEvent(
                    SyncTimeGet() + p_door->pause,
                    IStartDoorClosing,
                    sector) ;
            }

            /* Stop the door opening sound if it is still there. */
            if (p_door->sound != NULL)  {
                /* Turn off the door sound. */
//                AreaSoundDestroy(p_door->sound) ;
                p_door->sound = NULL ;
            }
        }
    }

    DebugEnd() ;

    return SLIDER_RESPONSE_CONTINUE ;
}

/****************************************************************************/
/*  Routine:  IStartDoorClosing                  * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IStartDoorClosing is called after a door has been opened.  This       */
/*  routine starts closing the door by creating another slider.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sector             -- Sector that is done opening and now    */
/*                                   needs to close the door.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoorClose                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IStartDoorClosing(T_word32 sector)
{
    DebugRoutine("IStartDoorClosing") ;

    DoorClose((T_word16)sector) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IHandleDoorClosing                 * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IHandleDoorClosing is called as a door closes.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Which sliding item.                    */
/*                                                                          */
/*    T_sword32 value             -- New ceiling height                     */
/*                                                                          */
/*    E_Boolean isDone            -- Flag telling if this is the last update*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MapCheckCrush                                                         */
/*    MapSetCeilingHeight                                                   */
/*    DoorOpen                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_sliderResponse IHandleDoorClosing(
           T_word32 sliderId,
           T_sword32 value,
           E_Boolean isDone)
{
    T_word16 sector ;
    T_sliderResponse response ;

    DebugRoutine("IHandleDoorClosing") ;

    /* Extract what sector we are using. */
    sector = sliderId & 0xFFFF ;

    /* See if this is a crushing sector (by changing the ceiling height). */
    if (MapCheckCrushByCeiling(sector, (T_sword16)(value>>16)) == FALSE)  {
        /* Change that ceiling's height. */
        MapSetCeilingHeight(sector, (T_sword16)(value>>16)) ;

        response = SLIDER_RESPONSE_CONTINUE ;
    } else {
        DoorOpen(sector) ;

        response = SLIDER_RESPONSE_CONTINUE ;
    }

    DebugEnd() ;

    return response ;
}

/****************************************************************************/
/*  Routine:  IHandleSoundCallback               * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IHandleSoundCallback tells the door module that one of the doors that */
/*  created a sound is now done.  This routine nulls out the reference      */
/*  to the sound so it is not used later.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_areaSound sound           -- Sound that was created.                */
/*                                                                          */
/*    T_word32 data               -- Data passed when sound created.        */
/*                                   In this case, a cast pointer to the    */
/*                                   door with the sound.                   */
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
/*    LES  04/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IHandleSoundCallback(T_areaSound sound, T_word32 data)
{
    T_door *p_door ;

    DebugRoutine("IHandleSoundCallback") ;

    /* Get a quick pointer to the door. */
    p_door = (T_door *)data ;
    DebugCheck(p_door != NULL) ;
    DebugCheck(p_door->tag == DOOR_TAG) ;

    /* Null out its sound entry. */
    p_door->sound = NULL ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorIsAtSector                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorIsAtSector checks to see if the given sector is a door.  If it    */
/*  is, TRUE is returned, else FALSE.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector to check if door                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE=is door, else FALSE               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean DoorIsAtSector(T_word16 sector)
{
    T_door *p_door ;
    E_Boolean isDoor ;

    DebugRoutine("DoorIsAtSector") ;

    p_door = IFindDoor(sector) ;
    if (p_door == NULL)
        isDoor = FALSE ;
    else
        isDoor = TRUE ;

    DebugEnd() ;

    return isDoor ;
}

/****************************************************************************/
/*  Routine:  DoorGetPercentOpen                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorGetPercentOpen returns the percentage that the door is open in    */
/*  a range of 0 to 256 (0% to 100%).                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector containing door                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- 0=0%, 256=100% open                    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*    MapGetCeilingHeight                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 DoorGetPercentOpen(T_word16 sector)
{
    T_door *p_door ;
    T_word16 percent = 256 ;  /* 256 = 100 % open, 0 = 0% open */
    T_sword16 ceiling ;
    T_word32 openAmount ;
    T_word32 height ;

    DebugRoutine("DoorGetPercentOpen") ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
#ifndef NDEBUG
if (p_door == NULL)  {
    printf("Cannot find door %d\n", sector) ;
}
#endif
    DebugCheck(p_door != NULL) ;

    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        ceiling = MapGetCeilingHeight(p_door->sector) ;
        if (ceiling <= p_door->low)  {
            percent = 0 ;
        } else if (ceiling >= p_door->high)  {
            percent = 256 ;
        } else {
            openAmount = (ceiling - p_door->low) << 8;
            height = (p_door->high - p_door->low) ;
            percent = openAmount / height ;
        }
    }

    DebugEnd() ;

    return percent ;
}

/****************************************************************************/
/*  Routine:  DoorDecreaseLock                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorDecreaseLock makes the door have an easier chance to be opened.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector containing door                 */
/*                                                                          */
/*    T_word16 amount             -- Amount to lock the door.  A value of   */
/*                                   0-255 is expected, or 0xFFFF.          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/16/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorDecreaseLock(T_word16 sector, T_word16 amount)
{
    T_door *p_door ;
    DebugRoutine("DoorDecreaseLock") ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;

    /* Make sure we have the door. */
    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        /* Unlock the door by the given amount down */
        /* to zero.  Note that if DOOR_ABSOLUTE_LOCK */
        /* is passed, it always will go to zero and */
        /* become passable. */
        if (p_door->locked > amount)  {
            if (p_door->locked != DOOR_ABSOLUTE_LOCK)
                p_door->locked -= amount ;
        } else {
            p_door->locked = 0 ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorIncreaseLock                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorDecreaseLock makes the door have an easier chance to be opened.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector containing door                 */
/*                                                                          */
/*    T_word16 amount             -- Amount to open the door.  A value of   */
/*                                   0-255 is expected, or 0xFFFF.          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/16/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorIncreaseLock(T_word16 sector, T_word16 amount)
{
    T_door *p_door ;

    DebugRoutine("DoorIncreaseLock") ;

    /* Find the sector door. */
    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;

    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        /* Make sure we are starting with a legal door lockedness. */
        DebugCheck(
            (p_door->locked <= MAX_DOOR_LOCK) ||
            (p_door->locked == DOOR_ABSOLUTE_LOCK)) ;
        if (amount == DOOR_ABSOLUTE_LOCK)  {
            /* Set the door so that it can not be unlocked */
            /* except by scripts. */
            p_door->locked = DOOR_ABSOLUTE_LOCK ;
        } else {
            if (p_door->locked != DOOR_ABSOLUTE_LOCK)  {
                /* Increase the locked amount. */
                p_door->locked += amount ;
                if (p_door->locked > MAX_DOOR_LOCK)
                    p_door->locked = MAX_DOOR_LOCK ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorGetLockValue                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorGetLockValue returns the lockedness of the door.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector containing door                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16 amount             -- Amount to door is locked. A value of   */
/*                                   0-255 is normal, else                  */
/*                                   DOOR_ABSOLUTE_LOCK                     */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindDoor                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/16/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 DoorGetLockValue(T_word16 sector)
{
    T_door *p_door ;
    T_word16 lockValue = DOOR_ABSOLUTE_LOCK ;

    DebugRoutine("DoorGetLockValue") ;

    p_door = IFindDoor(sector) ;
    DebugCheck(p_door != NULL) ;

    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        lockValue = p_door->locked ;
    }

    DebugCheck(
        (lockValue == DOOR_ABSOLUTE_LOCK) ||
        (lockValue <= MAX_DOOR_LOCK)) ;
    DebugEnd() ;

    return lockValue ;
}


/****************************************************************************/
/*  Routine:  DoorCreate                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorCreate makes another door out of a sector.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sector             -- Sector containing door                 */
/*                                                                          */
/*    T_sword16 floor, ceiling    -- Heights to go between                  */
/*                                                                          */
/*    T_word16 timeOpen           -- Time taken to open and close door      */
/*                                                                          */
/*    T_word16 timePause          -- Time to pause with door open           */
/*                                                                          */
/*    T_byte8 lockLevel           -- Level of lock, 0=unlock, 255=permanent */
/*                                                                          */
/*    T_sword16 soundX, soundY    -- Position of door sound                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  05/27/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorCreate(
           T_word16 sector,
           T_sword16 floor,
           T_sword16 ceiling,
           T_word16 timeOpen,
           T_word16 timePause,
           T_word16 lockLevel,
           T_sword16 soundX,
           T_sword16 soundY)
{
    T_door *p_door ;

    DebugRoutine("DoorCreate") ;
    DebugCheck(G_doorInit == TRUE) ;

    /* Allocate the needed memory. */
    p_door = MemAlloc(sizeof(T_door)) ;

    DebugCheck(p_door != NULL) ;
    if (p_door != NULL)  {
        p_door->sector = sector ;
        p_door->low = floor ;
        p_door->high = ceiling ;
        p_door->speed = timeOpen ;
        p_door->pause = timePause ;
        p_door->locked = lockLevel ;
        p_door->soundX = soundX ;
        p_door->soundY = soundY ;
        p_door->requiredItem = 0 ;

        /* Tag this door. */
        p_door->tag = DOOR_TAG ;

        /* Note that we don't have any sound yet attached to the door */
        p_door->sound = NULL ;

        /* Link into the linked list (reversed order). */
        p_door->next = G_firstDoor ;
        G_firstDoor = p_door ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorSetRequiredItem                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorSetRequiredItem makes a door that is locked become a door that    */
/*  requires a particular item.                                             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 doorSector         -- Sector containing door                 */
/*                                                                          */
/*    T_word16 itemType           -- Required item                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DoorSetRequiredItem(T_word16 doorSector, T_word16 itemType)
{
    T_door *p_door ;

    DebugRoutine("DoorSetRequiredItem") ;

    p_door = IFindDoor(doorSector) ;
    DebugCheck(p_door != NULL) ;

    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        p_door->requiredItem = itemType ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DoorGetRequiredItem                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    DoorGetRequiredItem returns the object type that the door requires    */
/*  or 0 if no object is required.                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 doorSector         -- Sector containing door                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Object type required, or 0 for none    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 DoorGetRequiredItem(T_word16 doorSector)
{
    T_word16 itemType = 0 ;
    T_door *p_door ;

    DebugRoutine("DoorGetRequiredItem") ;

    p_door = IFindDoor(doorSector) ;
    DebugCheck(p_door != NULL) ;

    if (p_door)  {
        DebugCheck(p_door->tag == DOOR_TAG) ;
        itemType = p_door->requiredItem ;
    }

    DebugEnd() ;

    return itemType ;
}

/****************************************************************************/
/*    END OF FILE:  DOOR.C                                                  */
/****************************************************************************/
