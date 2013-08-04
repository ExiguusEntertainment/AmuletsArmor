/*-------------------------------------------------------------------------*
 * File:  DOOR.C
 *-------------------------------------------------------------------------*/
/**
 * The Door system controls the opening and closing of doors.  It also
 * handles locked doors (either opened through knock or by item).
 *
 * @addtogroup DOOR
 * @brief Door Control System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
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


/*-------------------------------------------------------------------------*
 * Routine:  DoorInitialize
 *-------------------------------------------------------------------------*/
/**
 *  DoorInitialize starts up all the information needed for handling
 *  doors.
 *
 *<!-----------------------------------------------------------------------*/
T_void DoorInitialize(T_void)
{
    DebugRoutine("DoorInitialize") ;
    DebugCheck(G_doorInit == FALSE) ;

    /* Make sure we have nothing in our list of sliders. */
    G_firstDoor = NULL ;

    G_doorInit = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoorFinish
 *-------------------------------------------------------------------------*/
/**
 *  DoorFinish cleans up the door module and unloads all memory.
 *
 *<!-----------------------------------------------------------------------*/
T_void DoorFinish(T_void)
{
    DebugRoutine("DoorFinish") ;
    DebugCheck(G_doorInit == TRUE) ;

    IDoorUnload() ;

    /* Note that we are done. */
    G_doorInit = FALSE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoorLoad
 *-------------------------------------------------------------------------*/
/**
 *  DoorLoad loads the appropriate map door file.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorLock
 *-------------------------------------------------------------------------*/
/**
 *  DoorLock locks a door.  If it is already locked, it stays locked.
 *
 *  NOTE: 
 *  A door is only locked when it is fully closed.  A locked open door
 *  reacts normally until fully closed -- then it is truly locked.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorUnlock
 *-------------------------------------------------------------------------*/
/**
 *  DoorUnlock unlocks a door.  If it is already unlocked, nothing
 *  happens.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorIsLock
 *-------------------------------------------------------------------------*/
/**
 *  DoorIsLock return TRUE if the door is locked, or return FALSE.
 *
 *  NOTE: 
 *  DoorIsLock pretty much assumes the given sector is a door, else
 *  will always return FALSE.
 *
 *  @return TRUE if locked, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorOpen
 *-------------------------------------------------------------------------*/
/**
 *  DoorOpen opens a door and starts the appropriate slider unless the
 *  the door is locked.
 *
 *  @param sector -- Sector with door to open.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorClose
 *-------------------------------------------------------------------------*/
/**
 *  DoorClose closes a door and starts the appropriate slider.
 *
 *  @param sector -- Sector with door to close
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorForceOpen
 *-------------------------------------------------------------------------*/
/**
 *  DoorForceOpen opens a door whether or not it is locked.
 *
 *  @param sector -- Sector with door to open.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  IDoorUnload
 *-------------------------------------------------------------------------*/
/**
 *  IDoorUnload returns all memory to the system used by the door module.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  IFindDoor
 *-------------------------------------------------------------------------*/
/**
 *  IFindDoor is a utility routine that returns a pointer to a door given
 *  the sector number.
 *
 *  @param sector -- Sector door is at.
 *
 *  @return Pointer to door or NULL if not found.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  IHandleDoorOpening
 *-------------------------------------------------------------------------*/
/**
 *  IHandleDoorOpening is called as a door opens up.
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New ceiling height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  IStartDoorClosing
 *-------------------------------------------------------------------------*/
/**
 *  IStartDoorClosing is called after a door has been opened.  This
 *  routine starts closing the door by creating another slider.
 *
 *  @param sector -- Sector that is done opening and now
 *      needs to close the door.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IStartDoorClosing(T_word32 sector)
{
    DebugRoutine("IStartDoorClosing") ;

    DoorClose((T_word16)sector) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IHandleDoorClosing
 *-------------------------------------------------------------------------*/
/**
 *  IHandleDoorClosing is called as a door closes.
 *
 *  @param sliderId -- Which sliding item.
 *  @param value -- New ceiling height
 *  @param isDone -- Flag telling if this is the last update
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  IHandleSoundCallback
 *-------------------------------------------------------------------------*/
/**
 *  IHandleSoundCallback tells the door module that one of the doors that
 *  created a sound is now done.  This routine nulls out the reference
 *  to the sound so it is not used later.
 *
 *  @param sound -- Sound that was created.
 *  @param data -- Data passed when sound created.
 *      In this case, a cast pointer to the
 *      door with the sound.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorIsAtSector
 *-------------------------------------------------------------------------*/
/**
 *  DoorIsAtSector checks to see if the given sector is a door.  If it
 *  is, TRUE is returned, else FALSE.
 *
 *  @param sector -- Sector to check if door
 *
 *  @return TRUE=is door, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorGetPercentOpen
 *-------------------------------------------------------------------------*/
/**
 *  DoorGetPercentOpen returns the percentage that the door is open in
 *  a range of 0 to 256 (0% to 100%).
 *
 *  @param sector -- Sector containing door
 *
 *  @return 0=0%, 256=100% open
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorDecreaseLock
 *-------------------------------------------------------------------------*/
/**
 *  DoorDecreaseLock makes the door have an easier chance to be opened.
 *
 *  @param sector -- Sector containing door
 *  @param amount -- Amount to lock the door.  A value of
 *      0-255 is expected, or 0xFFFF.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorIncreaseLock
 *-------------------------------------------------------------------------*/
/**
 *  DoorDecreaseLock makes the door have an easier chance to be opened.
 *
 *  @param sector -- Sector containing door
 *  @param amount -- Amount to open the door.  A value of
 *      0-255 is expected, or 0xFFFF.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorGetLockValue
 *-------------------------------------------------------------------------*/
/**
 *  DoorGetLockValue returns the lockedness of the door.
 *
 *  @param sector -- Sector containing door
 *
 *  @return Amount to door is locked. A value of
 *      0-255 is normal, else
 *      DOOR_ABSOLUTE_LOCK
 *
 *<!-----------------------------------------------------------------------*/
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


/*-------------------------------------------------------------------------*
 * Routine:  DoorCreate
 *-------------------------------------------------------------------------*/
/**
 *  DoorCreate makes another door out of a sector.
 *
 *  @param sector -- Sector containing door
 *  @param floor -- Floor height to go between
 *  @param ceiling -- Ceiling height to go between
 *  @param timeOpen -- Time taken to open and close door
 *  @param timePause -- Time to pause with door open
 *  @param lockLevel -- Level of lock, 0=unlock, 255=permanent
 *  @param soundX -- X Position of door sound
 *  @param soundY -- Y Position of door sound
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorSetRequiredItem
 *-------------------------------------------------------------------------*/
/**
 *  DoorSetRequiredItem makes a door that is locked become a door that
 *  requires a particular item.
 *
 *  @param doorSector -- Sector containing door
 *  @param itemType -- Required item
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  DoorGetRequiredItem
 *-------------------------------------------------------------------------*/
/**
 *  DoorGetRequiredItem returns the object type that the door requires
 *  or 0 if no object is required.
 *
 *  @param doorSector -- Sector containing door
 *
 *  @return Object type required, or 0 for none
 *
 *<!-----------------------------------------------------------------------*/
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

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  DOOR.C
 *-------------------------------------------------------------------------*/
