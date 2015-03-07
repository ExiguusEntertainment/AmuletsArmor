/*-------------------------------------------------------------------------*
 * File:  VIEW.C
 *-------------------------------------------------------------------------*/
/**
 * Top level control for drawing the current 3D view.
 *
 * @addtogroup VIEW
 * @brief View Drawing
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "AREASND.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONTROL.H"
#include "CRELOGIC.H"
#include "EFFECT.H"
#include "GENERAL.H"
#include "GRAPHICS.H"
#include "KEYMAP.H"
#include "OBJECT.H"
#include "OVERHEAD.H"
#include "OVERLAY.H"
#include "MAP.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "UPDATE.H"
#include "VIEW.H"

#define SECTOR_SHIFT_X_FLAG  4
#define SECTOR_SHIFT_Y_FLAG  8
#define SECTOR_DIP_FLAG      16
#define SECTOR_ANIMATE_FLOOR_FLAG 32
#define DISTANCE_TO_PICKUP   24

#define TIME_BETWEEN_SECTOR_DAMAGE 35     // Every half second

static E_Boolean G_viewInitialized = FALSE ;

static T_viewOverlayHandler F_viewHandler = NULL ;

static T_sword16 G_bobHeight = 0 ;
static T_word16 G_bobDistance = 0 ;

static E_Boolean G_earthquake = FALSE ;
static T_word32 G_earthquakeDuration = 0 ;
static T_word32 G_earthquakeLastUpdate = 0 ;
static T_sword16 G_earthquakeRollX = 0 ;
static T_sword16 G_earthquakeRollY = 0 ;
static T_sword16 G_earthquakeRollZ = 0 ;
static T_word16 G_earthQuakeSoundChannel = 0xFFFF;
static E_Boolean G_underwater = FALSE ;

/* Value of 0-255 telling how well the player sees in the dark. */
static T_sbyte8 G_darkSight = 0 ;

/* Internal prototypes: */
static T_void IViewUpdateEarthquake(T_void) ;

static T_word16 G_viewOffsetView = 0 ;

static E_Boolean G_updateFormOverView = FALSE;

/*-------------------------------------------------------------------------*
 * Routine:  ViewInitialize
 *-------------------------------------------------------------------------*/
/**
 *  ViewInitialize sets up all the variables necessary for the current
 *  3D view.  Calls are made appropriately to the 3D engine.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewInitialize(T_void)
{
    DebugRoutine("ViewInitialize") ;
    DebugCheck(G_viewInitialized == FALSE) ;

    G_viewInitialized = TRUE ;

    /* Put up a nice message. */
//    printf("Initializing 3D view.\n");

    View3dInitialize() ;
    ObjectsInitialize() ;

    View3dSetSize(312, 148) ;

    MapInitialize() ;
    OverheadInitialize() ;
    OverlayInitialize() ;

    G_viewOffsetView = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewFinish
 *-------------------------------------------------------------------------*/
/**
 *  ViewFinish is called to remove all items used by View that was not
 *  used by the map.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewFinish(T_void)
{
    DebugRoutine("ViewFinish") ;
    DebugCheck(G_viewInitialized == TRUE) ;

    ObjectsFinish() ;
    OverlayFinish() ;
    MapFinish() ;
    OverheadFinish() ;
    View3dFinish() ;

    G_viewInitialized = FALSE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewDraw
 *-------------------------------------------------------------------------*/
/**
 *  ViewDraw builds and draws the current view on the screen.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewDraw(T_void)
{
    T_sword16 x, y ;
    T_sword32 height ;
    T_word16 angle ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    DebugRoutine("ViewDraw") ;
    DebugTime(2) ;
    TICKER_TIME_ROUTINE_START() ;

    DebugCheck(G_viewInitialized == TRUE) ;

INDICATOR_LIGHT(93, INDICATOR_GREEN) ;
    /* Have the map change lighting accordingly. */
    if (!ClientIsPaused())
        MapUpdateLighting() ;
INDICATOR_LIGHT(93, INDICATOR_RED) ;

INDICATOR_LIGHT(97, INDICATOR_GREEN) ;
    if ((ViewIsEarthquakeOn()) && (!ClientIsPaused()))  {
        IViewUpdateEarthquake() ;
        View3dGetView(&x, &y, &height, &angle) ;
        x += ((MathSineLookup(G_earthquakeRollX) * 5) >> 16) ;
        y += ((MathSineLookup(G_earthquakeRollY) * 5) >> 16) ;
        height += (MathSineLookup(G_earthquakeRollZ) * 5) ;
        View3dSetView(x, y, height, angle) ;
    }
INDICATOR_LIGHT(97, INDICATOR_RED) ;



INDICATOR_LIGHT(101, INDICATOR_GREEN) ;
    UpdateFrame() ;
INDICATOR_LIGHT(101, INDICATOR_RED) ;

INDICATOR_LIGHT(105, INDICATOR_GREEN) ;
    /* Do an offset of the view if applicable. */
    if ((G_viewOffsetView != 0) && (!ClientIsPaused()))  {
        View3dGetView(&x, &y, &height, &angle) ;
        View3dSetView(x, y, height, angle + G_viewOffsetView) ;
        G_viewOffsetView = 0 ;
    }

    /* Update object angles to the view */
    ObjectsUpdateAnimation(0) ;

    /* Draw the current 3d view. */
    View3dDrawView() ;
INDICATOR_LIGHT(105, INDICATOR_RED) ;

INDICATOR_LIGHT(109, INDICATOR_GREEN) ;
    OverheadSetCenterPoint(PlayerGetX16(), PlayerGetY16()) ;
    OverheadDraw(0, 0, VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-1, VIEW3D_HEIGHT-1) ;

    /* Draw the overlay if there is a function to draw it. */
    /* !!! Note:  Shouldn't really be using VIEW3D_WIDTH and HEIGHT */
    if (F_viewHandler != NULL)
//        F_viewHandler(VIEW3D_CLIP_LEFT, 0, VIEW3D_CLIP_RIGHT-1, VIEW3D_HEIGHT-1) ;
        F_viewHandler(0, 0, VIEW3D_CLIP_RIGHT-VIEW3D_CLIP_LEFT-1, VIEW3D_HEIGHT-1) ;

    /* Display the screen. */
    View3dDisplayView() ;

    GrScreenSet(GRAPHICS_ACTUAL_SCREEN);

    if (G_updateFormOverView)
        GraphicUpdateAllGraphicsForced();

    TICKER_TIME_ROUTINE_ENDM("ViewDraw", 100) ;

INDICATOR_LIGHT(109, INDICATOR_RED) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewSetOverlayHandler
 *-------------------------------------------------------------------------*/
/**
 *  ViewSetOverlayHandler declares a pointer to a function that is to
 *  be called each time the view is drawn but not shown.  This allows the
 *  system to draw objects and images on top of the view as the view is
 *  moving.
 *
 *  @param handler -- Function to draw overlay
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewSetOverlayHandler(T_viewOverlayHandler handler)
{
    DebugRoutine("ViewSetOverlayHandler") ;
    DebugCheck(G_viewInitialized == TRUE) ;

    F_viewHandler = handler ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewUpdatePlayer
 *-------------------------------------------------------------------------*/
/**
 *  ViewUpdatePlayer is the routine called to update all the exterior
 *  events that happen, even when a player is standing still.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewUpdatePlayer(T_void)
{
    static T_word32 lowerBit = 0;
    static T_word32 lastUpdate ;
    static T_word32 nextTimeToDamage = 0 ;
    static T_word32 lastWallCheck = 0 ;

    T_word16 flags ;
    T_word32 delta ;
    T_sword32 maxvelocity;
    T_sword16 playerHeight ;
    T_word32 time ;
    T_word16 areaSector ;
    T_word16 damageAmount ;
    T_byte8 damageType ;

    DebugRoutine("ViewUpdatePlayer") ;

    delta = (time = TickerGet()) - lastUpdate ;
    /* Clip delta -- no need to get rediculous */
    if (delta > 25)
        delta = 25 ;

    if (!ClientIsPaused())  {
        if (lastWallCheck == 0)
            lastWallCheck = time ;

        /* 'clip' velocity values; */
        if (KeyMapGetScan(KEYMAP_RUN) != TRUE)  {
            maxvelocity=StatsGetMaxVRunning();
        } else  {
            maxvelocity=StatsGetMaxVWalking();
        }

        /* Halve the speed if going backward. */
        if (KeyMapGetScan(KEYMAP_BACKWARD)==TRUE)
            maxvelocity>>=1;

        /* Set the maximum velocity. */
        PlayerSetMaxVelocity(maxvelocity) ;

        /* Update movement for the player. */
        PlayerUpdate(delta) ;
        playerHeight = (PlayerGetZ()>>16) + StatsGetTallness() ;

        /* Calculate 'bob' */
        G_bobDistance += (((T_sword32) PlayerGetTravelDistance()) << 7) ;
    //    G_bobDistance += (((T_sword32)CalculateDistance(0, 0, playerMove.XV>>16, playerMove.YV>>16)) << 7) ;
    //    G_bobDistance += ((T_sword32)velocitymag << 7) ;
        G_bobHeight = MathXTimesSinAngle(4, G_bobDistance) ;

        //HEY! You need to fix this!
        DebugCheck(PlayerGetAreaSector() < G_Num3dSectors) ;

        areaSector = PlayerGetAreaSector() ;
        if (PlayerGetAreaSector() < G_Num3dSectors)  {
	        flags = G_3dSectorArray[PlayerGetAreaSector()].trigger ;

#if 1
            if (flags & 2)  {
                G_3dSectorInfoArray[PlayerGetAreaSector()].damage = 250 ;
                G_3dSectorInfoArray[PlayerGetAreaSector()].damageType = EFFECT_DAMAGE_FIRE ;
            }
#else
            if ((flags&2) == 2 && (PlayerGetZ()>>16) <=
                    G_3dSectorArray[PlayerGetAreaSector()].floorHt)
            {
               if (StatsPlayerIsAlive())
               {
               /* lava damage */
    //               StatsChangePlayerHealth (-delta<<3);
                     if (EffectPlayerEffectIsActive (PLAYER_EFFECT_LAVA_WALK)==FALSE)
                     {
                        StatsTakeDamage (EFFECT_DAMAGE_FIRE,(delta<<3));
                     }
               }
               else
                   StatsPlayerSetAlive();
            }
#endif

	        /* Is this a sector where there is water flowing? */
	        if ((flags & 4) == 4)  {
	            if ((PlayerGetZ() >> 16) <=
                     G_3dSectorArray[PlayerGetAreaSector()].floorHt)  {
                     PlayerAddExternalVelocity((delta<<16), 0, 0) ;
		        /* Move the player in the +X direction. */
    /*
                newX = PlayerGetX() ;
                newY = PlayerGetY() ;
		        View3dMoveToFast(
        	        &newX,
		            &newY,
		            0,
		            delta,
		            20,
		            playerHeight) ;
                PlayerSetX(newX) ;
                PlayerSetY(newY) ;
    */
                }
	        }

	        /* Is this a sector where there is water flowing? */
	        if ((flags & 8) == 8)  {
	            if ((PlayerGetZ()>>16) <=
                     G_3dSectorArray[PlayerGetAreaSector()].floorHt)  {
                     PlayerAddExternalVelocity(0, -((T_sword32)(delta<<16)), 0) ;
		        /* Move the player in the +Y direction. */
    /*
                newX = PlayerGetX() ;
                newY = PlayerGetY() ;
		        View3dMoveToFast(
        	        &newX,
		            &newY,
		            0xC000,
		            delta*3,
		            20,
		            playerHeight) ;
                PlayerSetX(newX) ;
                PlayerSetY(newY) ;
    */
                }
            }

            lastUpdate = time ;

            if (time > nextTimeToDamage)  {
                /* Are we lava walking and in lava? */
                if ((EffectPlayerEffectIsActive(PLAYER_EFFECT_LAVA_WALK)) &&
                    (MapGetSectorType(PlayerGetAreaSector()) == SECTOR_TYPE_LAVA))  {
                    /* Yes, lava walking and in lava.  Take no damage. */
                } else {
                /* No, we're some where else.  Do whatever is appropriate. */
                    if (G_3dSectorInfoArray[areaSector].damage != 0)  {
                        if (G_3dSectorInfoArray[areaSector].damage != ((T_sword16)0x8000))  {
                            if ((PlayerGetZ()>>16) <=
                                    G_3dSectorArray[PlayerGetAreaSector()].floorHt) {
                                if (G_3dSectorInfoArray[areaSector].damageType ==
                                     EFFECT_DAMAGE_UNKNOWN)  {
                                    StatsTakeDamage(
                                        EFFECT_DAMAGE_FIRE,
                                        G_3dSectorInfoArray[areaSector].damage) ;
                                } else {
                                    StatsTakeDamage(
                                        G_3dSectorInfoArray[areaSector].damageType,
                                        G_3dSectorInfoArray[areaSector].damage) ;
                                }
                            }
                        }
                    }
                }

                nextTimeToDamage = time + TIME_BETWEEN_SECTOR_DAMAGE ;
            }
        }

        AreaSoundUpdate(PlayerGetX()>>16, PlayerGetY()>>16) ;

        if ((time - lastWallCheck) > TIME_BETWEEN_DAMAGE_WALL_CHECKS)  {
            lastWallCheck = time ;
            if (MapGetWallDamage(
                    PlayerGetObject(),
                    &damageAmount,
                    &damageType))
                StatsTakeDamage(damageType, damageAmount) ;
        }
    } else {
        lastUpdate = TickerGet() ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewGetMiddleTarget
 *-------------------------------------------------------------------------*/
/**
 *  ViewGetMiddleTarget finds the first non-passable target in front
 *  of the player.
 *
 *  @return Middle target object pointer or
 *      NULL if none.
 *
 *<!-----------------------------------------------------------------------*/
T_3dObject *ViewGetMiddleTarget(T_void)
{
    T_3dObject *p_object ;
    T_word16 objectPos ;
    T_sword32 width ;
    E_Boolean found = FALSE ;
    T_sword16 i;
    T_word16 x ;

    DebugRoutine("ViewGetMiddleTarget") ;

    p_object = NULL ;

    width = StatsGetPlayerAccuracy() ;
    width = width*(VIEW3D_HALF_WIDTH-1)/260L ;
    for (i=0; (i<width) && (found == FALSE); i++)  {
        objectPos = 0xFFFF ;
        x = VIEW3D_HALF_WIDTH - i + ControlGetLookOffset() ;
        if ((x > VIEW3D_CLIP_LEFT) && (x < VIEW3D_CLIP_RIGHT))  {
            do {
                objectPos = View3dGetObjectAtColumn(
                                objectPos,
                                &p_object,
                                x) ;
                if (p_object != NULL)  {
                    if ((!ObjectIsPassable(p_object)) &&
                        (((ObjectIsCreature(p_object)) &&
                           (!CreatureIsMissile(p_object))) ||
                         (ObjectIsPlayer(p_object))))  {
                        found = TRUE ;
                        break ;
                    }
                }
            } while (objectPos != 0xFFFF) ;
        }

        if (found == TRUE)
            break ;

        objectPos = 0xFFFF ;
        x = VIEW3D_HALF_WIDTH + i + ControlGetLookOffset() ;
        if ((x > VIEW3D_CLIP_LEFT) && (x < VIEW3D_CLIP_RIGHT))  {
            do {
                objectPos = View3dGetObjectAtColumn(
                                objectPos,
                                &p_object,
                                x) ;
                if (p_object != NULL)  {
                    if ((!ObjectIsPassable(p_object)) &&
                        (((ObjectIsCreature(p_object)) &&
                           (!CreatureIsMissile(p_object))) ||
                         (ObjectIsPlayer(p_object))))  {
                        found = TRUE ;
                        break ;
                    }
                }
            } while (objectPos != 0xFFFF) ;
        }

        if (found == TRUE)
            break ;
    }

#ifndef NDEBUG
    if (p_object)
        DebugCheck(strcmp(p_object->tag, "Obj")==0) ;
#endif

    DebugEnd() ;

    return p_object ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewGetXYTarget
 *-------------------------------------------------------------------------*/
/**
 *  ViewGetXYTarget finds the first passable or non-passable object
 *  under the given x & y pixel location on the screen (relative to the
 *  upper left hand corner of the screen)..
 *
 *  @param x -- x location from the view 's upperleft
 *  @param y -- y location from the view 's upperleft
 *
 *  @return XY target object pointer or
 *      NULL if none.
 *
 *<!-----------------------------------------------------------------------*/
T_3dObject *ViewGetXYTarget(T_word16 x, T_word16 y)
{
    T_3dObject *p_object ;
    T_word16 objectPos = 0xFFFF ;
    T_sword16 dx, dy ;

    DebugRoutine("ViewGetXYTarget") ;

    /* Try middle first -- make sure to get it all. */
    do {
        objectPos = View3dGetObjectAtXY(
                        objectPos,
                        &p_object,
                        x-VIEW3D_UPPER_LEFT_X+VIEW3D_CLIP_LEFT,
                        y-VIEW3D_UPPER_LEFT_Y) ;
    } while ((p_object == NULL) && (objectPos != 0xFFFF)) ;

    /* If didn't get an object, try a bigger area (5x5 pixel area) */
    if (p_object == NULL)  {
        for (dy=-2; (dy<=2)&&(p_object == NULL); dy++)  {
            for (dx=-2; (dx<=2)&&(p_object == NULL); dx++)  {
                objectPos = 0xFFFF ;
                do {
                    objectPos = View3dGetObjectAtXY(
                                    objectPos,
                                    &p_object,
                                    x-VIEW3D_UPPER_LEFT_X+VIEW3D_CLIP_LEFT+dx,
                                    y-VIEW3D_UPPER_LEFT_Y+dy) ;
                } while ((p_object == NULL) && (objectPos != 0xFFFF)) ;
            }
        }
    }

    DebugEnd() ;

    return p_object ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewIsAt
 *-------------------------------------------------------------------------*/
/**
 *  ViewIsAt checks to see if mouse based XY is over the 3d view.
 *
 *  @param x -- x location of mouse X
 *  @param y -- y location of mouse Y
 *
 *  @return TRUE = over, FALSE = else
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ViewIsAt(T_word16 x, T_word16 y)
{
    E_Boolean at = FALSE ;

	DebugRoutine("ViewIsAt") ;

	x -= VIEW3D_UPPER_LEFT_X ;
	y -= VIEW3D_UPPER_LEFT_Y ;

	if ((x < (VIEW3D_CLIP_RIGHT - VIEW3D_CLIP_LEFT)) && (y <= VIEW3D_HEIGHT))
		at = TRUE ;

    DebugEnd() ;

    return at ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewEarthquakeOn
 *-------------------------------------------------------------------------*/
/**
 *  ViewEarthquakeOn makes the view shake randomly.
 *
 *  @param duration -- How long the earthquake should occur
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewEarthquakeOn(T_word32 duration)
{
    DebugRoutine("ViewEarthquakeOn") ;

    G_earthquakeLastUpdate = SyncTimeGet() ;
    G_earthquakeDuration = G_earthquakeLastUpdate + duration ;
    G_earthquake = TRUE ;
    G_earthquakeRollX = 0 ;
    G_earthquakeRollY = 0 ;
    G_earthquakeRollZ = 0 ;

    if (G_earthQuakeSoundChannel==0xFFFF)
      G_earthQuakeSoundChannel=SoundPlayLoopByNumber(3005,255);
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewEarthquakeOff
 *-------------------------------------------------------------------------*/
/**
 *  ViewEarthquakeOff stops the view shake randomly.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewEarthquakeOff(T_void)
{
    DebugRoutine("ViewEarthquakeOff") ;

    G_earthquakeDuration = 0 ;
    G_earthquake = FALSE ;
    G_earthquakeRollX = 0 ;
    G_earthquakeRollY = 0 ;
    G_earthquakeRollZ = 0 ;
    if (G_earthQuakeSoundChannel!=0xFFFF)
    {
        SoundStop(G_earthQuakeSoundChannel);
        G_earthQuakeSoundChannel=0xFFFF;
    }
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewIsEarthquakeOn
 *-------------------------------------------------------------------------*/
/**
 *  ViewIsEarthquakeOn tells if the earthquake is happening.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ViewIsEarthquakeOn(T_void)
{
    return G_earthquake ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IViewUpdateEarthquake
 *-------------------------------------------------------------------------*/
/**
 *  IViewUpdateEarthquake updates whatever activity is occuring during
 *  an earthquake.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IViewUpdateEarthquake(T_void)
{
    T_word32 time ;
    T_word32 delta ;

    DebugRoutine("IViewUpdateEarthquake") ;

    time = SyncTimeGet() ;
    delta = time - G_earthquakeLastUpdate ;

    if (time > G_earthquakeDuration)  {
        ViewEarthquakeOff() ;
    } else {
        G_earthquakeRollZ += RandomNumber() % 20000 ;
        G_earthquakeRollY += RandomNumber() % 15000 ;
        G_earthquakeRollX += RandomNumber() % 10000 ;

        PlayerAccelDirection(rand(), delta) ;
    }

    G_earthquakeLastUpdate = delta ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewSetPalette
 *-------------------------------------------------------------------------*/
/**
 *  ViewSetPalette changes the palette used in the view.
 *
 *  @param viewPalette -- Palette to use in view
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewSetPalette(T_viewPalette viewPalette)
{
    T_resource res ;
    T_palette *p_palette ;
    T_byte8 name[40] ;

    DebugRoutine("ViewSetPalette") ;
    DebugCheck(viewPalette < VIEW_PALETTE_UNKNOWN) ;

    sprintf(name, "Palettes/VIEW%02d.PAL", viewPalette) ;
    p_palette = (T_palette *)PictureLockData(name, &res) ;

#ifdef OPTIONS_DARKCOLOR
    /* Darken the palette some. */
    p_pal = (T_byte8 *)p_palette ;
    for (i=0; i<768; i++, p_pal++)  {
        c = *p_pal ;
        c &= 63 ;
        c *= 7 ;
        c /= 8 ;
        *p_pal = c ;
    }
#endif
    GrSetPalette (0,224,(T_palette *)p_palette);
    ColorInit();

    PictureUnlockAndUnfind(res) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewUnderwaterOn
 *-------------------------------------------------------------------------*/
/**
 *  ViewUnderwaterOn changes the view to look like it is underwater.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewUnderwaterOn(T_void)
{
    DebugRoutine("ViewUnderwaterOn") ;

    if (!G_underwater)  {
        ViewSetPalette(VIEW_PALETTE_WATER) ;
        G_underwater = TRUE ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewUnderwaterOff
 *-------------------------------------------------------------------------*/
/**
 *  ViewUnderwaterOff changes the view back into the regular palette.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewUnderwaterOff(T_void)
{
    DebugRoutine("ViewUnderwaterOff") ;

    if (G_underwater)  {
        ViewSetPalette(VIEW_PALETTE_STANDARD) ;
        G_underwater = FALSE ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewIsUnderwater
 *-------------------------------------------------------------------------*/
/**
 *  ViewIsUnderwater tells if the view is under the water.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ViewIsUnderwater(T_void)
{
    DebugRoutine("ViewIsUnderwater") ;
    DebugEnd() ;

    return G_underwater ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewSetDarkSight
 *-------------------------------------------------------------------------*/
/**
 *  ViewSetDarkSight sets how well the player sees in the dark.
 *
 *  @param darkSightValue -- 0 = Normal, 127=perfect night vision,
 *      -127=Blind.
 *
 *<!-----------------------------------------------------------------------*/
T_void ViewSetDarkSight(T_sbyte8 darkSightValue)
{
    DebugRoutine("ViewSetDarkSight") ;

    View3dSetDarknessAdjustment(darkSightValue >> 1) ;
    G_darkSight = darkSightValue ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ViewSetDarkSight
 *-------------------------------------------------------------------------*/
/**
 *  ViewSetDarkSight returns how well the player sees in the dark.
 *
 *  @return 0 = Normal, 127=perfect night vision,
 *      -127=Blind.
 *
 *<!-----------------------------------------------------------------------*/
T_sbyte8 ViewGetDarkSight(T_void)
{
    DebugRoutine("ViewGetDarkSight") ;
    DebugEnd() ;

    return G_darkSight ;
}

T_void ViewOffsetView(T_word16 angle)
{
    G_viewOffsetView = angle ;
}

T_void ViewUpdateFormsOverViewEnable(void)
{
    G_updateFormOverView = TRUE;
}
T_void ViewUpdateFormsOverViewDisable(void)
{
    G_updateFormOverView = FALSE;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  VIEW.C
 *-------------------------------------------------------------------------*/
