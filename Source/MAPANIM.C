/*-------------------------------------------------------------------------*
 * File:  MAPANIM.C
 *-------------------------------------------------------------------------*/
/**
 * Because we wanted more animation effects than provided in the Deep 97
 * editor, we created .ANI files to store additional information about
 * a map.  Light, wall, and floor animations are provided here.
 * It would be nice that when a text based editor is used, these are
 * loaded as just properties on walls/sectors.
 *
 * @addtogroup MAPANIM
 * @brief Animation for Maps Configuration
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_VIEW.H"
#include "EFFECT.H"
#include "MAP.H"
#include "MAPANIM.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "PICS.H"
#include "SYNCTIME.H"

#define MAP_ANIMATION_TAG         (*((T_word32 *)"MpAn"))
#define MAP_ANIMATION_DEAD_TAG    (*((T_word32 *)"dMaN"))

/* Structure for the rather complex .ANI file. */
typedef struct {
    T_byte8 textureFloor[10] ;  /* Texture for sectorFloor. */
    T_byte8 textureCeiling[10] ;  /* Texture for sector ceiling. */
    T_word16 flags ;            /* Flags for sector. */
    T_sword16 xOffset ;          /* Bitmap X offset. */
    T_sword16 yOffset ;          /* Bitmap Y offset. */
    T_sword16 xSpeed ;          /* Scroll X speed. */
    T_sword16 ySpeed ;          /* Scroll Y speed. */
    T_sword16 xVelAdd ;         /* New x velocity. */
    T_sword16 yVelAdd ;         /* New y velocity. */
    T_sword16 zVelAdd ;         /* New z velocity. */
    T_word16 gravity ;          /* Gravity amount. */
    T_word16 nextMap ;          /* Next map id. */
    T_word16 nextMapStart ;     /* Next map start position. */
    T_word16 lightType ;        /* Type of light animation (0x8000 = no change) */
    T_word16 lightRate ;        /* Light rate. */
    T_word16 lightCenter ;      /* Light center. */
    T_word16 lightRadius ;      /* Radius of light. */
    T_word16 damage ;           /* Damage amount. */
    T_sword16 floorHeight ;     /* New floor height to slide to. */
    T_sword16 ceilingHeight ;   /* New ceiling height to slide to. */
    T_word16 enterSound ;       /* Sound to make when entering. */
    T_word16 enterSoundRadius ; /* Distance of sound. */
    T_word16 areaSound ;        /* Area sound to activate when in this state. */
    T_word16 duration ;         /* Duration for animation. */
    T_word16 nextState ;        /* What the next sector state is. */
} T_mapAnimSectorState ;

typedef struct {
    T_word16 flags ;            /* New flags to be. */
    T_word16 activity ;         /* Activity script id. */
    T_word16 frontSideAnim ;    /* Animation for front side to become. */
    T_word16 backSideAnim ;     /* Animation for back side to become. */
    T_word16 damageAmount ;     /* Amount of damage this wall does. (0=none)*/
    T_word16 damageType ;        /* Type of damage this wall does. (0=none)*/
    T_word16 duration ;         /* How long this animation is. */
    T_word16 nextState ;        /* What is the next wall state. */
} T_mapAnimWallState ;

typedef struct {
    T_byte8 upperTexture[10] ; /* ?=non-define, -=none, XXXX=texture name. */
    T_byte8 mainTexture[10] ;  /* ?=non-define, -=none, XXXX=texture name. */
    T_byte8 lowerTexture[10] ; /* ?=non-define, -=none, XXXX=texture name. */
    T_word16 duration ;        /* Time in ticks until next state. */
    T_word16 nextState ;       /* Value of 0xFFFF means last state. */
    T_sword16 xOffset ;          /* Bitmap X offset. */
    T_sword16 yOffset ;          /* Bitmap Y offset. */
    T_sword16 xSpeed ;         /* X scroll speed (256 = 1 pixel/second) */
    T_sword16 ySpeed ;         /* Y scroll speed (256 = 1 pixel/second) */
} T_mapAnimSideState ;

typedef T_byte8 T_mapAnimInitStateType ;
#define MAP_ANIM_INIT_STATE_TYPE_SIDE            0
#define MAP_ANIM_INIT_STATE_TYPE_WALL            1
#define MAP_ANIM_INIT_STATE_TYPE_SECTOR          2
#define MAP_ANIM_INIT_STATE_TYPE_UNKNOWN         3

typedef struct {
    T_mapAnimInitStateType initType ;
    T_word16 number ;
    T_word16 state ;
} T_mapAnimInitState ;

typedef struct {
    T_word16 numWallStates ;                 /* Number of wall states defined. */
    T_word16 numSideStates ;                 /* Number of side states defined. */
    T_word16 numSectorStates ;               /* Number of sector states. */
    T_word16 numInitStates ;                 /* Number of states to start. */

    T_mapAnimWallState *p_wallStates ;       /* Where the wall states are. */
    T_mapAnimSideState *p_sideStates ;       /* Where the side states are. */
    T_mapAnimSectorState *p_sectorStates ;   /* Where the sector states are. */
    T_mapAnimInitState *p_initStates ;       /* Wehre the init states are. */

    T_byte8 p_rawData[] ;                    /* Raw data of wall and side */
                                             /* states follow. */
} T_mapAnimStates ;

/* Structures that keep the current state of the map. */
typedef struct {
    T_word32 xScroll ;           /* Amount scrolled in X. */
    T_word32 yScroll ;           /* Amount scrolled in Y. */
    T_word16 timePassed ;        /* Time passed in this state. */
    T_3dSide *p_side ;           /* Direct pointer to side being affected. */
    T_word16 sideNum ;           /* Number of side being affected. */
    T_mapAnimSideState *p_state; /* Side state being applied. */
} T_mapAnimSide ;

typedef struct {
    T_word16 timePassed ;        /* Time elapsed in this state. */
    T_3dLine *p_line ;           /* Direct pointer to wall. */
    T_word16 lineNum ;           /* Number of wall/line. */
    T_mapAnimWallState *p_state; /* Current wall state being applied. */
} T_mapAnimWall ;

typedef struct {
    T_word16 timePassed ;           /* Time elapsed in this state. */
    T_3dSector *p_sector ;          /* Direct pointer to sector. */
    T_3dSectorInfo *p_sectorInfo ;  /* Additional sector info. */
    T_word16 sectorNum ;            /* Number of sector to change. */
    T_mapAnimSectorState *p_state;  /* Current sector state. */
    T_word32 xScroll ;              /* Amount scrolled on sector. */
    T_word32 yScroll ;              /* Amount scrolled on floor. */
} T_mapAnimSector ;

typedef struct {
    T_word32 tag ;                   /* Tag to identify this structure. */
    T_mapAnimStates *p_states ;      /* Pointer to all state data. */
    T_resource res ;                 /* Resource handle to this data. */
    T_word32 lastTimeUpdated ;       /* Last time this animation has */
                                     /* been updated. */
    T_mapAnimSide **p_sides ;        /* List of pointers of actively */
                                     /* changing sides. */
    T_mapAnimWall **p_walls ;        /* List of pointers of actively */
                                     /* changing walls. */
    T_mapAnimSector **p_sectors ;    /* List of pointers of actively */
                                     /* changing sectors. */
    T_doubleLinkList animSides ;     /* List of animating sides. */
    T_doubleLinkList animWalls ;     /* List of animating walls. */
    T_doubleLinkList animSectors ;   /* List of animating sectors. */
} T_mapAnimHeaderStruct ;

/* Internal prototypes: */
static T_void IMapUpdateSides(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta) ;

static T_void IMapUpdateWalls(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta) ;

static T_void IMapUpdateSectors(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta) ;

static T_void IInitAnimSideForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimSide *p_animSide,
                  T_word16 sideStateNumber) ;

static T_void IInitAnimWallForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimWall *p_animWall,
                  T_word16 wallStateNumber) ;

static T_void IInitAnimSectorForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimSector *p_animSector,
                  T_word16 sectorStateNumber) ;

static T_void IMapAnimDoInitStates(T_mapAnimHeaderStruct *p_mapAnimHeader) ;

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateLoad
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateLoad loads in a map animation record for the system.
 *
 *  NOTE: 
 *  This routine MUST be called AFTER loading the map, NOT BEFORE.
 *
 *  @param number -- Number of map animation to load
 *
 *  @return Map animation handle
 *
 *<!-----------------------------------------------------------------------*/
T_mapAnimation MapAnimateLoad(T_word32 number)
{
    T_byte8 name[40] ;
    T_mapAnimHeaderStruct *p_mapAnimHeader ;
    T_mapAnimStates *p_states ;
    T_byte8 *p_data ;

    DebugRoutine("MapAnimateLoad") ;

    /* Allocate a structure for a map animation handle. */
    p_mapAnimHeader = MemAlloc(sizeof(T_mapAnimHeaderStruct)) ;
    DebugCheck(p_mapAnimHeader != NULL) ;

    /* Make sure we got the handle. */
    if (p_mapAnimHeader)  {
        /* Get the appropriate name. */
//        sprintf(name, "L%ld.ANI", number) ;
        strcpy(name, "map.ani") ;

        /* Lock the data into memory. */
        if (PictureExist(name))  {
            p_states =
                p_mapAnimHeader->p_states =
                    (T_mapAnimStates *)
                        PictureLockData(
                            name,
                            &p_mapAnimHeader->res) ;
            DebugCheck(p_mapAnimHeader->p_states != NULL) ;
        } else {
            DebugCheck(FALSE) ;
            p_mapAnimHeader->p_states = NULL ;
        }

        if (p_mapAnimHeader->p_states == NULL)  {
            /* If we didn't get it, drop the handle */
            /* and return a bad handle. */
            MemFree(p_mapAnimHeader) ;
            p_mapAnimHeader = NULL ;
        } else {
            /* Everything is fine, mark the handle tag as good. */
            p_mapAnimHeader->tag = MAP_ANIMATION_TAG ;

            /* Set up the linked lists. */
            p_mapAnimHeader->animSides = DoubleLinkListCreate() ;
            p_mapAnimHeader->animWalls = DoubleLinkListCreate() ;
            p_mapAnimHeader->animSectors = DoubleLinkListCreate() ;

            /* Start the update counter from the current time. */
            p_mapAnimHeader->lastTimeUpdated = 0 ;

            /* Allocate memory for the quick pointers to actively */
            /* changing areas.  Be sure to init to NULL. */
            p_mapAnimHeader->p_sides =
                MemAlloc(G_Num3dSides * sizeof(T_mapAnimSide *)) ;
            memset(
                p_mapAnimHeader->p_sides,
                0,
                G_Num3dSides * sizeof(T_mapAnimSide *)) ;
            p_mapAnimHeader->p_walls =
                MemAlloc(G_Num3dLines * sizeof(T_mapAnimWall *)) ;
            memset(
                p_mapAnimHeader->p_walls,
                0,
                G_Num3dLines * sizeof(T_mapAnimWall *)) ;
            p_mapAnimHeader->p_sectors =
                MemAlloc(G_Num3dSectors * sizeof(T_mapAnimSector *)) ;
            memset(
                p_mapAnimHeader->p_sectors,
                0,
                G_Num3dSectors * sizeof(T_mapAnimSector *)) ;

            /* Fix up all the pointers. */
            p_data = (T_byte8 *)p_mapAnimHeader->p_states ;

            /* Skip the header. */
            p_data += sizeof(T_mapAnimStates) ;

            /* This is where the wall states start. */
            p_states->p_wallStates = (T_mapAnimWallState *)p_data ;

            /* Skip past the wall states. */
            p_data += sizeof(T_mapAnimWallState) * p_states->numWallStates ;

            /* This is where the side states start. */
            p_states->p_sideStates = (T_mapAnimSideState *)p_data ;

            /* Skip past the side states. */
            p_data += sizeof(T_mapAnimSideState) * p_states->numSideStates ;

            /* This is where the sector states start. */
            p_states->p_sectorStates = (T_mapAnimSectorState *)p_data ;

            /* Skip past the sector states. */
            p_data += sizeof(T_mapAnimSectorState) * p_states->numSectorStates ;

            /* This is where the initialize states occur. */
            p_states->p_initStates = (T_mapAnimInitState *)p_data ;

            IMapAnimDoInitStates(p_mapAnimHeader) ;
        }
    }

    DebugEnd() ;

    return ((T_mapAnimation)p_mapAnimHeader) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateUnload
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateUnload deletes all data associated with this map animation.
 *
 *  @param mapAnimation -- Map animation to unload
 *
 *<!-----------------------------------------------------------------------*/
T_void MapAnimateUnload(T_mapAnimation mapAnimation)
{
    T_mapAnimHeaderStruct *p_mapAnimHeader ;
    T_doubleLinkListElement element ;

    DebugRoutine("MapAnimateUnload") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        /* Dereference the handle. */
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;

        MemFree(p_mapAnimHeader->p_sides) ;
        MemFree(p_mapAnimHeader->p_walls) ;
        MemFree(p_mapAnimHeader->p_sectors) ;

        /* Make sure we have the correct handle to what we want. */
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;
        if (p_mapAnimHeader->tag == MAP_ANIMATION_TAG)  {
            PictureUnlockAndUnfind(p_mapAnimHeader->res) ;

            if (p_mapAnimHeader->animSides != DOUBLE_LINK_LIST_BAD)  {
                element = DoubleLinkListGetFirst(p_mapAnimHeader->animSides) ;
                while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
//printf("Destroy side) %p\n", DoubleLinkListElementGetData(element)) ;
                    MemFree(DoubleLinkListElementGetData(element)) ;
                    DoubleLinkListRemoveElement(element) ;
                    element = DoubleLinkListGetFirst(p_mapAnimHeader->animSides) ;
                }
                DoubleLinkListDestroy(p_mapAnimHeader->animSides) ;
                p_mapAnimHeader->animSides = DOUBLE_LINK_LIST_BAD ;
            }

            if (p_mapAnimHeader->animWalls != DOUBLE_LINK_LIST_BAD)  {
                element = DoubleLinkListGetFirst(p_mapAnimHeader->animWalls) ;
                while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
//printf("Destroy wall) %p\n", DoubleLinkListElementGetData(element)) ;
                    MemFree(DoubleLinkListElementGetData(element)) ;
                    DoubleLinkListRemoveElement(element) ;
                    element = DoubleLinkListGetFirst(p_mapAnimHeader->animWalls) ;
                }
                DoubleLinkListDestroy(p_mapAnimHeader->animWalls) ;
                p_mapAnimHeader->animWalls = DOUBLE_LINK_LIST_BAD ;
            }

            if (p_mapAnimHeader->animSectors != DOUBLE_LINK_LIST_BAD)  {
                element = DoubleLinkListGetFirst(p_mapAnimHeader->animSectors) ;
                while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
//printf("Destroy sect) %p\n", DoubleLinkListElementGetData(element)) ;
                    MemFree(DoubleLinkListElementGetData(element)) ;
                    DoubleLinkListRemoveElement(element) ;
                    element = DoubleLinkListGetFirst(p_mapAnimHeader->animSectors) ;
                }
                DoubleLinkListDestroy(p_mapAnimHeader->animSectors) ;
                p_mapAnimHeader->animSectors = DOUBLE_LINK_LIST_BAD ;
            }

            p_mapAnimHeader->tag = MAP_ANIMATION_DEAD_TAG ;
            MemFree(p_mapAnimHeader) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateUpdate
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateUpdate goes through its list of things to animate and does
 *  them.
 *
 *  @param mapAnimation -- Map animation to update
 *
 *<!-----------------------------------------------------------------------*/
T_void MapAnimateUpdate(T_mapAnimation mapAnimation)
{
    T_mapAnimHeaderStruct *p_mapAnimHeader ;
    T_word32 delta ;
    T_word32 time ;

    DebugRoutine("MapAnimateUpdate") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        /* Dereference the handle. */
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;

        /* Make sure we have the correct handle to what we want. */
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;
        if (p_mapAnimHeader->tag == MAP_ANIMATION_TAG)  {
            /* Ok, now we can start animating. */
            if (p_mapAnimHeader->lastTimeUpdated == 0)  {
                p_mapAnimHeader->lastTimeUpdated = SyncTimeGet() ;
            } else {
                time = SyncTimeGet() ;
                delta = time - p_mapAnimHeader->lastTimeUpdated ;
                p_mapAnimHeader->lastTimeUpdated = time ;
                IMapUpdateSides(p_mapAnimHeader, delta) ;
                IMapUpdateWalls(p_mapAnimHeader, delta) ;
                IMapUpdateSectors(p_mapAnimHeader, delta) ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IMapUpdateSides
 *-------------------------------------------------------------------------*/
/**
 *  IMapUpdateSides goes through the list of actively changing sides
 *  and updates them.
 *
 *  @param mapAnimation -- Map animation containing sides
 *
 *<!-----------------------------------------------------------------------*/
static T_void IMapUpdateSides(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_word32 timeTotal ;
    E_Boolean stateChange = FALSE ;
    T_word32 timeChange ;
    T_word32 timeRemaining ;
    T_word32 duration ;
    T_word32 timeOrig ;

    T_mapAnimSide *p_animSide ;
    T_mapAnimSideState *p_state ;
    T_3dSide *p_side ;

    DebugRoutine("IMapUpdateSides") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

    element = DoubleLinkListGetFirst(p_mapAnimHeader->animSides) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Get the next element before we do anything */
        /* "funky" to this element. */
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* Get the animation side referenced by this element. */
        p_animSide = (T_mapAnimSide *)DoubleLinkListElementGetData(element) ;
        timeChange = delta ;

        /* Loop while we keep changing state structures. */
        do {
            /* Get the original time slice of this animation */
            /* since last animation. */
            timeOrig = p_animSide->timePassed ;
            /* Add the time that has gone by to the total */
            p_animSide->timePassed += timeChange ;

            /* Record this in a local variable. */
            timeTotal = p_animSide->timePassed ;

            /* Get a quick pointer to the state. */
            p_state = p_animSide->p_state ;

            /* Get a quick pointer to the side. */
            p_side = p_animSide->p_side ;

            /* Get the duration for the state we are in. */
            duration = p_state->duration ;

            /* Check if it is time to change states. */
            if (timeTotal >= duration)  {
                /* State is changing. */
                /* Note that there is a state change. */
                stateChange = TRUE ;

                /* Calculate the amount of time it took */
                /* up to this exact state change. */
                timeChange =  duration - timeOrig ;

                /* Calculate the amount of time remaining */
                /* after this state change. */
                timeRemaining = timeTotal - duration ;
            } else {
                /* It is not time to change states.  Make a note. */
                stateChange = FALSE ;
            }

            /* Regardless if we are changing state, we still */
            /* need to update the delta animations. */
            p_animSide->xScroll += (((T_sword32)p_state->xSpeed) * timeChange) ;
            p_animSide->yScroll += (((T_sword32)p_state->ySpeed) * timeChange) ;
            /* Store the x and y offset on the wall. */
            p_side->tmXoffset = (p_animSide->xScroll >> 6) & 0xFF ;
            p_side->tmYoffset = (p_animSide->yScroll >> 6) & 0xFF ;

            /* Check to see if the above says it is time to */
            /* change state. */
            if (stateChange)  {
                /* Do the state change. */
                /* Is this the last state? */
                if (p_state->nextState == 0xFFFF)  {
                    /* Yep, last state.  Destroy it. */

                    /* Take it off the list first. */
                    DoubleLinkListRemoveElement(element) ;
                    /* Remove it from the list of active sides. */
                    p_mapAnimHeader->p_sides[p_animSide->sideNum] = NULL ;
                    /* Now destroy it. */
//printf("free side) %p\n", p_animSide) ;
                    MemFree(p_animSide) ;

                    /* Note that we don't need to continue with */
                    /* the state change. */
                    stateChange = FALSE ;
                } else {
                    /* Nope, another state we need to be. */

                    /* Go to the next state. */
                    IInitAnimSideForState(
                        p_mapAnimHeader,
                        p_animSide,
                        p_state->nextState) ;

                    /* However, roll over the remaining state info. */
//                    p_animSide->timePassed = timeRemaining ;
                    timeChange = timeRemaining ;
                }
            }
        } while (stateChange == TRUE) ;

        element = nextElement ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IInitAnimSideForState
 *-------------------------------------------------------------------------*/
/**
 *  IInitAnimSideForState takes the given animation side and a new
 *  side animation state and makes the change to that state.
 *
 *  NOTE: 
 *  It is assumed that p_animSide is already created and holds the sideNum
 *  and p_side variables.
 *
 *  @param p_mapAnimHeader -- Which group of animations
 *  @param p_animSide -- Pointer to animation side in effect
 *  @param sideStateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IInitAnimSideForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimSide *p_animSide,
                  T_word16 sideStateNumber)
{
    T_mapAnimSideState *p_state ;
    T_3dSide *p_side ;

    DebugRoutine("IInitAnimSideForState") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;
    DebugCheck(p_animSide->p_side != NULL) ;

    /* Determine which state we are referring to. */
    DebugCheck(sideStateNumber < p_mapAnimHeader->p_states->numSideStates) ;
    p_state = &p_mapAnimHeader->p_states->p_sideStates[sideStateNumber] ;
    p_animSide->p_state = p_state ;
    p_side = p_animSide->p_side ;

    /* Reset the time. */
    p_animSide->timePassed = 0 ;

    /* Change the textures of the wall. */
    if (p_state->upperTexture[0] != '?')
        MapSetUpperTextureForSide(
            p_animSide->sideNum,
            p_state->upperTexture) ;
    if (p_state->lowerTexture[0] != '?')
        MapSetLowerTextureForSide(
            p_animSide->sideNum,
            p_state->lowerTexture) ;
    if (p_state->mainTexture[0] != '?')
        MapSetMainTextureForSide(
            p_animSide->sideNum,
            p_state->mainTexture) ;

    if (p_state->xOffset != (T_sword16)0x8000)  {
        p_side->tmXoffset = p_state->xOffset ;
        p_animSide->xScroll = p_state->xOffset << 6 ;
    }

    if (p_state->yOffset != (T_sword16)0x8000)  {
        p_side->tmYoffset = p_state->yOffset ;
        p_animSide->yScroll = p_state->yOffset << 6 ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateStartSide
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateStartSide prepares a side to go into the given animation
 *  state.  If the side is already animating, this routine also aborts
 *  the previous state for the new given state.
 *
 *  NOTE: 
 *  You have to be careful when you have this called.  Repeat calls
 *  DO reinitialize the state to zero.
 *
 *  @param mapAnimation -- Aninmationp of animations
 *  @param sideNum -- Pointer to animation side in effect
 *  @param stateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
T_void MapAnimateStartSide(
           T_mapAnimation mapAnimation,
           T_word16 sideNum,
           T_word16 stateNumber)
{
    T_mapAnimSide *p_animSide ;
    T_3dSide *p_side ;
    T_mapAnimHeaderStruct *p_mapAnimHeader ;

    DebugRoutine("MapAnimateStartSide") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

        /* Make sure the side num is valid. */
        DebugCheck(sideNum < G_Num3dSides) ;
        if (sideNum < G_Num3dSides)  {
            /* Get a quick pointer to the T_3dSide */
            p_side = G_3dSideArray + sideNum ;

            /* Make sure the state number is valid. */
            DebugCheck(stateNumber < p_mapAnimHeader->p_states->numSideStates) ;
            if (stateNumber < p_mapAnimHeader->p_states->numSideStates)  {
                /* See if this side is already animating. */
                p_animSide = p_mapAnimHeader->p_sides[sideNum] ;

                if (!p_animSide)  {
                    /* It is not already animating.  We'll have to create it. */
                    p_animSide = MemAlloc(sizeof(T_mapAnimSide)) ;
                    DebugCheck(p_animSide != NULL) ;

                    if (p_animSide)  {
                        /* Initialize the new structure. */
                        p_animSide->xScroll = p_side->tmXoffset << 6 ;
                        p_animSide->yScroll = p_side->tmYoffset << 6 ;
                        p_animSide->timePassed = 0 ;
                        p_animSide->p_side = p_side ;
                        p_animSide->sideNum = sideNum ;
                    }

                    /* Add this element to the update list. */
                    DoubleLinkListAddElementAtEnd(
                        p_mapAnimHeader->animSides,
                        p_animSide) ;
//printf("side) Add %p\n", p_animSide) ;
                }

                /* Initialize the state related data. */
                IInitAnimSideForState(
                    p_mapAnimHeader,
                    p_animSide,
                    stateNumber) ;

                /* Add it from the list of active sides. */
                p_mapAnimHeader->p_sides[p_animSide->sideNum] = p_animSide ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IMapAnimDoInitStates
 *-------------------------------------------------------------------------*/
/**
 *  IMapAnimDoInitStates goes through the list of initial states and
 *  starts each state up.
 *
 *  @param p_mapAnimHeader -- Aninmationp of animations
 *
 *<!-----------------------------------------------------------------------*/
static T_void IMapAnimDoInitStates(T_mapAnimHeaderStruct *p_mapAnimHeader)
{
    T_word16 numInit ;
    T_mapAnimInitState *p_init ;

    DebugRoutine("IMapAnimDoInitStates") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

    p_init = p_mapAnimHeader->p_states->p_initStates ;
    DebugCheck(p_init != NULL) ;
    numInit = p_mapAnimHeader->p_states->numInitStates ;

    while (numInit)  {
        switch(p_init->initType)  {
            case MAP_ANIM_INIT_STATE_TYPE_SIDE:
                MapAnimateStartSide(
                    (T_mapAnimation)p_mapAnimHeader,
                    p_init->number,
                    p_init->state) ;
                break ;
            case MAP_ANIM_INIT_STATE_TYPE_WALL:
                MapAnimateStartWall(
                    (T_mapAnimation)p_mapAnimHeader,
                    p_init->number,
                    p_init->state) ;
                break ;
            case MAP_ANIM_INIT_STATE_TYPE_SECTOR:
                MapAnimateStartSector(
                    (T_mapAnimation)p_mapAnimHeader,
                    p_init->number,
                    p_init->state) ;
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }

        numInit-- ;
        p_init++ ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IMapUpdateWalls
 *-------------------------------------------------------------------------*/
/**
 *  IMapUpdateWalls goes through the list of actively changing walls
 *  and updates them.
 *
 *  @param mapAnimation -- Map animation containing walls
 *
 *<!-----------------------------------------------------------------------*/
static T_void IMapUpdateWalls(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_word32 timeTotal ;
    E_Boolean stateChange = FALSE ;
    T_word32 timeChange ;
    T_word32 timeRemaining ;
    T_word32 duration ;
    T_word32 timeOrig ;

    T_mapAnimWall *p_animWall ;
    T_mapAnimWallState *p_state ;
    T_3dLine *p_line ;

    DebugRoutine("IMapUpdateWalls") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

    element = DoubleLinkListGetFirst(p_mapAnimHeader->animWalls) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Get the next element before we do anything */
        /* "funky" to this element. */
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* Get the animation wall referenced by this element. */
        p_animWall = (T_mapAnimWall *)DoubleLinkListElementGetData(element) ;
        timeChange = delta ;

        /* Loop while we keep changing state structures. */
        do {
            /* Get the original time slice of this animation */
            /* since last animation. */
            timeOrig = p_animWall->timePassed ;

            /* Add the time that has gone by to the total */
            p_animWall->timePassed += timeChange ;

            /* Record this in a local variable. */
            timeTotal = p_animWall->timePassed ;

            /* Get a quick pointer to the state. */
            p_state = p_animWall->p_state ;

            /* Get a quick pointer to the wall/line. */
            p_line = p_animWall->p_line ;

            /* Get the duration for the state we are in. */
            duration = p_state->duration ;

            /* Check if it is time to change states. */
            if (timeTotal >= duration)  {
                /* State is changing. */
                /* Note that there is a state change. */
                stateChange = TRUE ;

                /* Calculate the amount of time it took */
                /* up to this exact state change. */
                timeChange =  duration - timeOrig ;

                /* Calculate the amount of time remaining */
                /* after this state change. */
                timeRemaining = timeTotal - duration ;
            } else {
                /* It is not time to change states.  Make a note. */
                stateChange = FALSE ;
            }

            /* Check to see if the above says it is time to */
            /* change state. */
            if (stateChange)  {
                /* Do the state change. */
                /* Is this the last state? */
                if (p_state->nextState == 0xFFFF)  {
                    /* Yep, last state.  Destroy it. */

                    /* Take it off the list first. */
                    DoubleLinkListRemoveElement(element) ;
                    /* Remove it from the list of active walls. */
                    p_mapAnimHeader->p_walls[p_animWall->lineNum] = NULL ;
                    /* Now destroy it. */
//printf("free wall) %p\n", p_animWall) ;
                    MemFree(p_animWall) ;

                    /* Note that we don't need to continue with */
                    /* the state change. */
                    stateChange = FALSE ;
                } else {
                    /* Nope, another state we need to be. */

                    /* Go to the next state. */
                    IInitAnimWallForState(
                        p_mapAnimHeader,
                        p_animWall,
                        p_state->nextState) ;

                    /* However, roll over the remaining state info. */
                    timeChange = timeRemaining ;
                }
            }
        } while (stateChange == TRUE) ;

        element = nextElement ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IInitAnimWallForState
 *-------------------------------------------------------------------------*/
/**
 *  IInitAnimWallForState takes the given animation wall and a new
 *  wall animation state and makes the change to that state.
 *
 *  NOTE: 
 *  It is assumed that p_animWall is already created and holds the lineNum
 *  and p_line variables.
 *
 *  @param p_mapAnimHeader -- Which group of animations
 *  @param p_animWall -- Pointer to animation wall in effect
 *  @param wallStateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IInitAnimWallForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimWall *p_animWall,
                  T_word16 wallStateNumber)
{
    T_mapAnimWallState *p_state ;
    T_3dLine *p_line ;

    DebugRoutine("IInitAnimWallForState") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;
    DebugCheck(p_animWall->p_line != NULL) ;

    /* Determine which state we are referring to. */
    DebugCheck(wallStateNumber < p_mapAnimHeader->p_states->numWallStates) ;
    p_state = &p_mapAnimHeader->p_states->p_wallStates[wallStateNumber] ;
    p_animWall->p_state = p_state ;

    /* Reset the time. */
    p_animWall->timePassed = 0 ;

    /* Change the flags if need be. */
    p_line = p_animWall->p_line;
    if (p_state->flags != 0xFFFF)
        p_line->flags = p_state->flags ;
    if (p_state->activity != 0xFFFF)
        p_line->tag = p_state->activity ;

    /* Start up any wall animations if necessary. */
    if (p_state->frontSideAnim != 0xFFFF)
        MapAnimateStartSide(
            (T_mapAnimation)p_mapAnimHeader,
            p_line->side[0],
            p_state->frontSideAnim) ;
    if (p_state->backSideAnim != 0xFFFF)
        MapAnimateStartSide(
            (T_mapAnimation)p_mapAnimHeader,
            p_line->side[1],
            p_state->backSideAnim) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateStartWall
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateStartWall prepares a wall to go into the given animation
 *  state.  If the wall is already animating, this routine also aborts
 *  the previous state for the new given state.
 *
 *  NOTE: 
 *  You have to be careful when you have this called.  Repeat calls
 *  DO reinitialize the state to zero.
 *
 *  @param mapAnimation -- Aninmationp of animations
 *  @param wallNum -- Pointer to animation wall in effect
 *  @param stateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
T_void MapAnimateStartWall(
           T_mapAnimation mapAnimation,
           T_word16 wallNum,
           T_word16 stateNumber)
{
    T_mapAnimWall *p_animWall ;
    T_3dLine *p_line ;
    T_mapAnimHeaderStruct *p_mapAnimHeader ;

    DebugRoutine("MapAnimateStartWall") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

        /* Make sure the wall num is valid. */
        DebugCheck(wallNum < G_Num3dLines) ;
        if (wallNum < G_Num3dLines)  {
            /* Get a quick pointer to the T_3dLine */
            p_line = G_3dLineArray + wallNum ;

            /* Make sure the state number is valid. */
            DebugCheck(stateNumber < p_mapAnimHeader->p_states->numWallStates) ;
            if (stateNumber < p_mapAnimHeader->p_states->numWallStates)  {
                /* See if this wall is already animating. */
                p_animWall = p_mapAnimHeader->p_walls[wallNum] ;

                if (!p_animWall)  {
                    /* It is not already animating.  We'll have to create it. */
                    p_animWall = MemAlloc(sizeof(T_mapAnimWall)) ;
                    DebugCheck(p_animWall != NULL) ;

                    if (p_animWall)  {
                        /* Initialize the new structure. */
                        p_animWall->timePassed = 0 ;
                        p_animWall->p_line = p_line ;
                        p_animWall->lineNum = wallNum ;
                    }

                    /* Add this element to the update list. */
                    DoubleLinkListAddElementAtEnd(
                        p_mapAnimHeader->animWalls,
                        p_animWall) ;
//printf("wall) Add %p\n", p_animWall) ;
                }

                /* Initialize the state related data. */
                IInitAnimWallForState(
                    p_mapAnimHeader,
                    p_animWall,
                    stateNumber) ;

                /* Add it from the list of active walls. */
                p_mapAnimHeader->p_walls[wallNum] = p_animWall ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IMapUpdateSectors
 *-------------------------------------------------------------------------*/
/**
 *  IMapUpdateSectors goes through the list of actively changing sectors
 *  and updates them.
 *
 *  @param mapAnimation -- Map animation containing sectors
 *
 *<!-----------------------------------------------------------------------*/
static T_void IMapUpdateSectors(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_word32 delta)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_word32 timeTotal ;
    E_Boolean stateChange = FALSE ;
    T_word32 timeChange ;
    T_word32 timeRemaining ;
    T_word32 duration ;
    T_word32 timeOrig ;

    T_mapAnimSector *p_animSector ;
    T_mapAnimSectorState *p_state ;
    T_3dSector *p_sector ;
    T_3dSectorInfo *p_sectorInfo ;

    DebugRoutine("IMapUpdateSectors") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

    element = DoubleLinkListGetFirst(p_mapAnimHeader->animSectors) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Get the next element before we do anything */
        /* "funky" to this element. */
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* Get the animation sector referenced by this element. */
        p_animSector = (T_mapAnimSector *)DoubleLinkListElementGetData(element) ;
        timeChange = delta ;

        /* Loop while we keep changing state structures. */
        do {
            /* Get the original time slice of this animation */
            /* since last animation. */
            timeOrig = p_animSector->timePassed ;
            /* Add the time that has gone by to the total */
            p_animSector->timePassed += timeChange ;

            /* Record this in a local variable. */
            timeTotal = p_animSector->timePassed ;

            /* Get a quick pointer to the state. */
            p_state = p_animSector->p_state ;

            /* Get a quick pointer to the sector. */
            p_sector = p_animSector->p_sector ;
            p_sectorInfo = p_animSector->p_sectorInfo ;

            /* Get the duration for the state we are in. */
            duration = p_state->duration ;

            /* Check if it is time to change states. */
            if (timeTotal >= duration)  {
                /* State is changing. */
                /* Note that there is a state change. */
                stateChange = TRUE ;

                /* Calculate the amount of time it took */
                /* up to this exact state change. */
                timeChange =  duration - timeOrig ;

                /* Calculate the amount of time remaining */
                /* after this state change. */
                timeRemaining = timeTotal - duration ;
            } else {
                /* It is not time to change states.  Make a note. */
                stateChange = FALSE ;
            }

            /* Regardless if we are changing state, we still */
            /* need to update the delta animations. */
            p_animSector->xScroll += ((((T_sword32)p_state->xSpeed) * timeChange)) ;
            p_animSector->yScroll += ((((T_sword32)p_state->ySpeed) * timeChange)) ;

            /* Store the x and y offset on the wall. */
            p_sectorInfo->textureXOffset = (p_animSector->xScroll >> 6) & 0xFF ;
            p_sectorInfo->textureYOffset = (p_animSector->yScroll >> 6) & 0xFF ;

            /* Check to see if the above says it is time to */
            /* change state. */
            if (stateChange)  {
                /* Do the state change. */
                /* Is this the last state? */
                if (p_state->nextState == 0xFFFF)  {
                    /* Yep, last state.  Destroy it. */

                    /* Take it off the list first. */
                    DoubleLinkListRemoveElement(element) ;
                    /* Remove it from the list of active sectors. */
                    p_mapAnimHeader->p_sectors[p_animSector->sectorNum] = NULL ;
                    /* Now destroy it. */
//printf("free sect) %p\n", p_animSector) ;
                    MemFree(p_animSector) ;

                    /* Note that we don't need to continue with */
                    /* the state change. */
                    stateChange = FALSE ;
                } else {
                    /* Nope, another state we need to be. */

                    /* Go to the next state. */
                    IInitAnimSectorForState(
                        p_mapAnimHeader,
                        p_animSector,
                        p_state->nextState) ;

                    /* However, roll over the remaining state info. */
                    timeChange = timeRemaining ;
                }
            }
        } while (stateChange == TRUE) ;

        element = nextElement ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IInitAnimSectorForState
 *-------------------------------------------------------------------------*/
/**
 *  IInitAnimSectorForState takes the given animation sector and a new
 *  sector animation state and makes the change to that state.
 *
 *  NOTE: 
 *  It is assumed that p_animSector is already created and holds the
 *  sectorNum and p_sector variables.
 *
 *  @param p_mapAnimHeader -- Which group of animations
 *  @param p_animSector -- Pointer to animation sector in
 *      effect
 *  @param sectorStateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IInitAnimSectorForState(
                  T_mapAnimHeaderStruct *p_mapAnimHeader,
                  T_mapAnimSector *p_animSector,
                  T_word16 sectorStateNumber)
{
    T_mapAnimSectorState *p_state ;
    T_3dSector *p_sector ;
    T_3dSectorInfo *p_sectorInfo ;

    DebugRoutine("IInitAnimSectorForState") ;
    DebugCheck(p_mapAnimHeader != NULL) ;
    DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;
    DebugCheck(p_animSector->p_sector != NULL) ;

    /* Determine which state we are referring to. */
    DebugCheck(sectorStateNumber < p_mapAnimHeader->p_states->numSectorStates) ;
    p_state = &p_mapAnimHeader->p_states->p_sectorStates[sectorStateNumber] ;
    p_animSector->p_state = p_state ;

    p_sector = G_3dSectorArray + p_animSector->sectorNum ;
    p_sectorInfo = G_3dSectorInfoArray + p_animSector->sectorNum ;

    /* Reset the time. */
    p_animSector->timePassed = 0 ;

    /* Change the textures of the wall. */
    if (p_state->textureFloor[0] != '?')
        MapSetFloorTextureForSector(
            p_animSector->sectorNum,
            p_state->textureFloor) ;
    if (p_state->textureCeiling[0] != '?')
        MapSetCeilingTextureForSector(
            p_animSector->sectorNum,
            p_state->textureCeiling) ;
//    if (p_state->flags != 0xFFFF)
//        p_sector->type = p_state->flags ;

    if (p_state->xOffset != (T_sword16)0x8000)  {
        p_sectorInfo->textureXOffset = (T_sbyte8)p_state->xOffset ;
        p_animSector->xScroll = p_state->xOffset << 6 ;
    }
    if (p_state->yOffset != (T_sword16)0x8000)  {
        p_sectorInfo->textureYOffset = (T_sbyte8)p_state->yOffset ;
        p_animSector->yScroll = p_state->yOffset << 6 ;
    }

    if (p_state->xVelAdd != (T_sword16)0x8000)
        p_sectorInfo->xVelAdd = (T_sbyte8)p_state->xVelAdd ;
    if (p_state->yVelAdd != (T_sword16)0x8000)
        p_sectorInfo->yVelAdd = (T_sbyte8)p_state->yVelAdd ;
    if (p_state->zVelAdd != (T_sword16)0x8000)
        p_sectorInfo->zVelAdd = (T_sbyte8)p_state->zVelAdd ;
    if (p_state->gravity != 0xFFFF)
        p_sectorInfo->gravity = p_state->gravity ;
    if (p_state->nextMap != 0xFFFF)
        p_sectorInfo->nextMap = p_state->nextMap ;
    if (p_state->nextMapStart != 0xFFFF)
        p_sectorInfo->nextMapStart = p_state->nextMapStart ;
    if (p_state->lightType != 0xFFFF)
        p_sectorInfo->lightAnimationType = (T_byte8)p_state->lightType ;
    if (p_state->lightRate != 0xFFFF)
        p_sectorInfo->lightAnimationRate = (T_byte8)p_state->lightRate ;
    if (p_state->lightCenter != 0xFFFF)
        p_sectorInfo->lightAnimationCenter = (T_byte8)p_state->lightCenter ;
    if (p_state->lightRadius != 0xFFFF)
        p_sectorInfo->lightAnimationRadius = (T_byte8)p_state->lightRadius ;
    if (p_state->damage != (T_word16)0x8000)  {
        p_sectorInfo->damage = p_state->damage ;
    }

    /* !!! Note:  This should be a slider!!! */
    if (p_state->floorHeight != (T_sword16)0x8000)
        p_sector->floorHt = p_state->floorHeight ;
    if (p_state->ceilingHeight != (T_sword16)0x8000)
        p_sector->ceilingHt = p_state->ceilingHeight ;
    if (p_state->enterSound != 0xFFFF)
        p_sectorInfo->enterSound = p_state->enterSound ;
    if (p_state->enterSoundRadius != 0xFFFF)
        p_sectorInfo->enterSoundRadius = p_state->enterSoundRadius ;


    /* Area sound. */
    if (p_state->areaSound != 0xFFFF)  {
        /* Need code here to activate area sound. */
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateStartSector
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateStartSector prepares a sector to go into the given animation
 *  state.  If the sector is already animating, this routine also aborts
 *  the previous state for the new given state.
 *
 *  NOTE: 
 *  You have to be careful when you have this called.  Repeat calls
 *  DO reinitialize the state to zero.
 *
 *  @param mapAnimation -- Aninmationp of animations
 *  @param sectorNum -- Pointer to animation sector in
 *      effect
 *  @param stateNumber -- State number to become.
 *
 *<!-----------------------------------------------------------------------*/
T_void MapAnimateStartSector(
           T_mapAnimation mapAnimation,
           T_word16 sectorNum,
           T_word16 stateNumber)
{
    T_mapAnimSector *p_animSector ;
    T_3dSector *p_sector ;
    T_3dSectorInfo *p_sectorInfo ;
    T_mapAnimHeaderStruct *p_mapAnimHeader ;

    DebugRoutine("MapAnimateStartSector") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

        /* Make sure the sector num is valid. */
        DebugCheck(sectorNum < G_Num3dSectors) ;
        if (sectorNum < G_Num3dSectors)  {
            /* Get a quick pointer to the T_3dSector */
            p_sector = G_3dSectorArray + sectorNum ;
            p_sectorInfo = G_3dSectorInfoArray + sectorNum ;

            /* Make sure the state number is valid. */
            DebugCheck(stateNumber < p_mapAnimHeader->p_states->numSectorStates) ;
            if (stateNumber < p_mapAnimHeader->p_states->numSectorStates)  {
                /* See if this sector is already animating. */
                p_animSector = p_mapAnimHeader->p_sectors[sectorNum] ;

                if (!p_animSector)  {
                    /* It is not already animating.  We'll have to create it. */
                    p_animSector = MemAlloc(sizeof(T_mapAnimSector)) ;
                    DebugCheck(p_animSector != NULL) ;

                    if (p_animSector)  {
                        /* Initialize the new structure. */
                        p_animSector->xScroll = p_sectorInfo->textureXOffset << 6 ;
                        p_animSector->yScroll = p_sectorInfo->textureYOffset << 6 ;
                        p_animSector->timePassed = 0 ;
                        p_animSector->p_sector = p_sector ;
                        p_animSector->p_sectorInfo = p_sectorInfo ;
                        p_animSector->sectorNum = sectorNum ;
                    }

                    /* Add this element to the update list. */
                    DoubleLinkListAddElementAtEnd(
                        p_mapAnimHeader->animSectors,
                        p_animSector) ;
//printf("sect) Add %p\n", p_animSector) ;
                }

                /* Initialize the state related data. */
                IInitAnimSectorForState(
                    p_mapAnimHeader,
                    p_animSector,
                    stateNumber) ;

                /* Add it from the list of active sectors. */
                p_mapAnimHeader->p_sectors[p_animSector->sectorNum] = p_animSector ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MapAnimateGetWallDamage
 *-------------------------------------------------------------------------*/
/**
 *  MapAnimateGetWallDamage takes an object and returns a damage type and
 *  damage amount.  The routine is used to determine if an object is on
 *  a damage wall.  If it is, this routine returns TRUE.
 *
 *  NOTE: 
 *  The passed in object must have its area sectors correctly resolved.
 *
 *  @param mapAnimation -- MapAnimation to check
 *  @param p_obj -- Pointer to object to consider
 *  @param p_damageAmount -- where to return found damage amount
 *  @param p_damageType -- Returned damage type
 *
 *  @return TRUE if any type of damage here.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean MapAnimateGetWallDamage(
              T_mapAnimation mapAnimation,
              T_3dObject *p_obj,
              T_word16 *p_damageAmount,
              T_byte8 *p_damageType)
{
    T_mapAnimHeaderStruct *p_mapAnimHeader ;
    T_doubleLinkListElement element ;
    T_mapAnimWall *p_wall ;
    T_mapAnimWallState *p_state ;
    T_3dLine *p_line ;
    E_Boolean isFound = FALSE ;

    DebugRoutine("MapAnimateGetWallDamage") ;
    DebugCheck(mapAnimation != MAP_ANIMATION_BAD) ;

    if (mapAnimation)  {
        p_mapAnimHeader = (T_mapAnimHeaderStruct *)mapAnimation ;
        DebugCheck(p_mapAnimHeader->tag == MAP_ANIMATION_TAG) ;

        element = DoubleLinkListGetFirst(p_mapAnimHeader->animWalls) ;

        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            p_wall = (T_mapAnimWall *)DoubleLinkListElementGetData(element) ;
            p_line = p_wall->p_line ;
            /* Only bother with 2 sided animatiion walls. */
            if (p_line->flags & LINE_IS_TWO_SIDED)  {
                p_state = p_wall->p_state ;
                /* Only bother if wall is going to do damage. */
                if ((p_state->damageAmount != 0) &&
                    (p_state->damageType != 0))  {
                    if (Collide3dCheckSegmentHitBox(
                            p_wall->lineNum,
                            ObjectGetX16(p_obj) - ObjectGetRadius(p_obj),
                            ObjectGetY16(p_obj) - ObjectGetRadius(p_obj),
                            ObjectGetX16(p_obj) + ObjectGetRadius(p_obj),
                            ObjectGetY16(p_obj) + ObjectGetRadius(p_obj)) != FALSE)  {
                        *p_damageAmount = p_state->damageAmount ;
                        *p_damageType = (T_byte8)p_state->damageType ;
                        isFound = TRUE ;
                        break ;
                    }
                }
            }
            element = DoubleLinkListElementGetNext(element) ;
        }
    }

    DebugEnd() ;

    return isFound ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  MAPANIM.C
 *-------------------------------------------------------------------------*/
