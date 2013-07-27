/****************************************************************************/
/*    FILE:  TICKER.C                                                       */
/****************************************************************************/
#include "SOUND.H"
#include "TICKER.H"

#ifdef DOS32
#ifndef HISPEED_TESTING
#  define TICKER_SPEED 70.0
#else
#  define TICKER_SPEED 560.0
#endif

#ifdef HISPEED_TESTING
#  define TICKER_SPEED_UP_RATE (TICKER_SPEED / 18.2)
#  define TICKER_DIVIDER ((T_word16)(65535.0 / TICKER_SPEED_UP_RATE))
#else
#  define TICKER_SPEED_UP_RATE (TICKER_SPEED / 18.2)
#  define TICKER_DIVIDER ((T_word16)(65535.0 / TICKER_SPEED_UP_RATE))
#endif

#define TICKER_INTERRUPT_NUMBER 0x8

/* Number of ticks that have been counted: */
static T_word32 G_tickCount = 0 ;

/* Note if the ticker is active: */
static E_Boolean F_tickerOn = FALSE ;

/* Keep track of how many ticks are needed until we must call the bios */
/* time of day counter. */
static T_byte8 G_subTickCount = 0 ;

/* Keep track of multiple levels of pausing the ticker. */
static T_sword16 G_pauseLevel = 0 ;

/* Keep track of what the old timer interrupt used to be.  We'll need */
/* it to update the BIOS timer correctly. */
static T_void (__interrupt __far *IOldTickerInterrupt)();

/* Internal routines: */
static T_void __interrupt __far ITickerInterrupt(T_void) ;
static T_void __interrupt __far ITickerInterruptSimple(T_void) ;

static E_Boolean G_installedAtExit = FALSE ;

T_void ITickerAtExit(T_void) ;

/****************************************************************************/
/*  Routine:  TickerOn                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Before any ticks progress in the system, you need to first turn on    */
/*  the Ticker by using TickerOn.  After that, the tick count is increment  */
/*  about 55 times a second.  Use TickerGet to get the current count.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Don't call this routine twice.                                        */
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
/*    _disable                                                              */
/*    _enable                                                               */
/*    outp                                                                  */
/*    _dos_getvect                                                          */
/*    _dos_setvect                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/22/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void TickerOn(T_void)
{
    T_word16 timerSpeed ;

    DebugRoutine("TickerOn") ;
    DebugCheck(F_tickerOn == FALSE) ;

    /* Note that the ticker is now on. */
    F_tickerOn = TRUE ;

    /* We are doing somewhat sensitive stuff, so turn off the interrupts. */
    _disable() ;

    /* First get the old timer we used to use. */
#if defined(DOS32)
    IOldTickerInterrupt = _dos_getvect(TICKER_INTERRUPT_NUMBER);
#endif
    /* Since we are piggy-backing the timer used by the sound system, */
    /* we need to know if there is sound on already.  If there is, */
    /* we'll use the timing of that timer already.  If the sound is */
    /* not there, we'll go ahead and set the timer to our speed (which */
    /* should be the same as the sound). */
#ifdef HISPEED_TESTING
    if (TRUE)  {
#else
    if (SoundIsOn() == FALSE)  {
#endif
        /* Now declare our new interrupt to the timer. */
//puts("Turning on timer.") ;
#if defined(DOS32)
        _dos_setvect(TICKER_INTERRUPT_NUMBER, ITickerInterrupt);
#endif

        /* Speed up the timer chip so that it is about 3 times as fast: */
//       timerSpeed = ((T_word16)0xFFFF)/TICKER_SPEED_UP_RATE ;
        timerSpeed = TICKER_DIVIDER ;
        /* Access the timer chip for timer programming. */
        outp(0x43, 0x36) ;

        /* Change the low byte of the timer. */
        outp(0x40, timerSpeed&255) ;

        /* Chane the upper byte of the timer. */
        outp(0x40, (timerSpeed>>8)) ;

        /* See if we need an 'atexit' function to restore the timer. */
        if (G_installedAtExit == FALSE)  {
            G_installedAtExit = TRUE ;
            atexit(ITickerAtExit) ;
        }
    } else
    {
//puts("Not turning on timer") ;
        /* Now declare our new interrupt to the timer. */
//        _dos_setvect(TICKER_INTERRUPT_NUMBER, ITickerInterruptSimple);
    }

    /* Done twiddling with the hardware, turn back on the interrupts. */
    _enable() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  TickerOff                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    TickerOff will turn off the ticker counter.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine MUST be called before you end the program.  If you       */
/*  do not, DOS will crash at a later point (most likely).                  */
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
/*    _disable                                                              */
/*    _dos_setvect                                                          */
/*    _enable                                                               */
/*    outp                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/22/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void TickerOff(T_void)
{
    DebugRoutine("TickerOff") ;
    DebugCheck(F_tickerOn == TRUE) ;

    /* We are doing somewhat sensitive stuff, so turn off the interrupts. */
    _disable() ;

    /* Access the timer chip for timer programming. */
    outp(0x43, 0x36) ;

    /* Change the low byte of the timer to the default speed. */
    outp(0x40, 0xFF) ;

    /* Chane the upper byte of the timer to the default speed. */
    outp(0x40, 0xFF) ;

    /* Remove our ticker with the original ticker. */
    _dos_setvect(TICKER_INTERRUPT_NUMBER, IOldTickerInterrupt);

    /* Done twiddling with the hardware, turn back on the interrupts. */
    _enable() ;

    /* Note that the ticker is now off. */
    F_tickerOn = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  TickerGet                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    TickerGet returns the number of ticks that have passed since the      */
/*  TickerOn was called.  Note:  Don't worry about this timer rolling       */
/*  over because it will take about 2 years of the computer being           */
/*  constantly on and nobody reseting the computer.                         */
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
/*    T_word32                    -- Number of ticks                        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _disable                                                              */
/*    _enable                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/22/94  Created                                                */
/*    LES  09/19/95  Removed Debug lines to keep keyboard interrupt from    */
/*                   crashing (when memory checking is on).                 */
/*                                                                          */
/****************************************************************************/

T_word32 TickerGet(T_void)
{
    T_word32 count ;

/* Cannot have a DebugRoutine since called by Keyboard interrupt routine.
//    DebugRoutine("TickerGet") ;

    /* Before we get the count, let's make sure the system is not */
    /* in the middle of incrementing it. */
//    _disable() ;

    /* Ok, got the count. */
#ifdef HISPEED_TESTING
    count = (G_tickCount>>3) ;
#else
    count = G_tickCount ;
#endif

    /* Turn back on the interrupts. */
//    _enable() ;

//    DebugEnd() ;

    return count ;
}

T_word32 TickerGetAccurate(T_void)
{
    return G_tickCount ;
}
/****************************************************************************/
/*  Routine:  ITickerInterrupt                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ITickerInterrupt is the internal interrupt that counts ticks that     */
/*  have passed since TickerOn was called.  Note that this routine also     */
/*  makes sure to call the original BIOS routines to keep the time of       */
/*  day clock correct.                                                      */
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
/*    outp                                                                  */
/*    IOldTickerInterrupt                                                   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    outp                                                                  */
/*    IOldTickerInterrupt                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/22/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void __interrupt __far ITickerInterrupt(T_void)
{
    /* Check to see if we are allowed to increment the ticker count. */
    if (G_pauseLevel <= 0)
        /* Go ahead and increment the tick count. */
        G_tickCount++ ;

    /* Increment and check the sub tick count (the count that */
    /* tells us when to call the old BIOS routine(s)). */
    if ((++G_subTickCount) >= TICKER_SPEED_UP_RATE)  {
        /* Call the old BIOS routine. */
        IOldTickerInterrupt() ;

        /* Reset the sub tick count. */
        G_subTickCount = 0 ;
    } else {
        /* Do I need the next line? */
        /* _enable() ; */
        /* Signal the end of this interrupt. */
        outp(0x20, 0x20) ;
    }
}

/****************************************************************************/
/*  Routine:  ITickerInterruptSimple                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ITickerInterrupt is the internal interrupt that counts ticks that     */
/*  have passed since TickerOn was called.  Note that this routine also     */
/*  makes sure to call the original BIOS routines to keep the time of       */
/*  day clock correct.                                                      */
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
/*    outp                                                                  */
/*    IOldTickerInterrupt                                                   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    outp                                                                  */
/*    IOldTickerInterrupt                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/22/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void __interrupt __far ITickerInterruptSimple(T_void)
{
    INDICATOR_LIGHT(0, INDICATOR_GREEN) ;
    /* Go ahead and increment the tick count. */
    G_tickCount++ ;

    INDICATOR_LIGHT(0, INDICATOR_YELLOW) ;
    /* Do the old interrupt. */
    IOldTickerInterrupt() ;
    INDICATOR_LIGHT(0, INDICATOR_BLUE) ;

    /* Note that we are done. */
    outp(0x20, 0x20) ;
    INDICATOR_LIGHT(0, INDICATOR_RED) ;
}

/****************************************************************************/
/*  Routine:  TickerPause                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    TickerPause stops the counter from counting so that future actions    */
/*  that depend on the ticker think that it is not updating (although       */
/*  the interrupts will still occur).                                       */
/*    Note that you can call this routine more than once, but if you do,    */
/*  you must call TickerContinue an equal number of times for it to         */
/*  return to its previous state.                                           */
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
/*    LES  05/09/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void TickerPause(T_void)
{
    DebugRoutine("TickerPause") ;

    /* Add one to the pause level. */
    G_pauseLevel++ ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  TickerContinue                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    TickerContinue continues the counter from a previous call to          */
/*  TickerPause.                                                            */
/*    Note that you can call this routine more than once, but if you do,    */
/*  you must call (or have called) TickerPause an equal number of times.    */
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
/*    LES  05/09/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void TickerContinue(T_void)
{
    DebugRoutine("TickerContinue") ;

    /* Subtract one from the pause level. */
    G_pauseLevel-- ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ITickerAtExit                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ITickerAtExit is the routine called to restore the timer if it        */
/*  has not already done so.  This is mainly for when a crash occurs.       */
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
/*    TickerOff                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  05/09/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ITickerAtExit(T_void)
{
    if (F_tickerOn == TRUE)  {
#ifndef NDEBUG
        puts("Restoring timer") ;
#endif
        TickerOff() ;
    }
}

#ifdef HISPEED_TESTING
T_void TickerInc(T_void)
{
    ((char *)0xA0000)[288+(G_tickCount & 0x1F)]^=15 ;
}
#else
T_void TickerInc(T_void)
{
    INDICATOR_LIGHT(0, INDICATOR_GREEN) ;
    if (G_pauseLevel <= 0)
        G_tickCount++ ;
//    ((char *)0xA0000)[288+(G_tickCount & 0x1F)]+=3 ;
    INDICATOR_LIGHT(0, INDICATOR_RED) ;
}
#endif

#endif


#ifdef WIN32
//#include <time.h>
#include <SDL.h>
static T_word32 G_lastMillisecondCount ;
static T_word32 G_tickMilli ;
static T_word32 G_tickCount ;
static E_Boolean F_tickerOn = FALSE ;
static T_sword16 G_pauseLevel = 0 ;

T_void TickerOn(T_void)
{
    DebugRoutine("TickerOn") ;
    DebugCheck(F_tickerOn == FALSE) ;

    /* Note that the ticker is now on. */
    F_tickerOn = TRUE ;
    G_lastMillisecondCount = SDL_GetTicks(); // clock() ;
    G_tickMilli = 0 ;

    DebugEnd() ;
}
T_void TickerOff(T_void)
{
    DebugRoutine("TickerOff") ;
    DebugCheck(F_tickerOn == TRUE) ;

    /* Note that the ticker is now off. */
    F_tickerOn = FALSE ;

    DebugEnd() ;
}

T_void TickerUpdate(T_void)
{
    T_word32 time ;

    time = SDL_GetTicks(); // clock() ;
    G_tickMilli += time - G_lastMillisecondCount ;
    G_lastMillisecondCount = time ;

    G_tickCount = G_tickMilli / 14 ;
}

T_word32 TickerGet(T_void)
{
    /* Update the tick count if we are not paused */
    if (!G_pauseLevel)
        TickerUpdate();
    return G_tickCount ;
}

T_word32 TickerGetAccurate(T_void)
{
    return G_tickCount ;
}

T_void TickerPause(T_void)
{
    /* Add one to the pause level. */
    G_pauseLevel++ ;
}

T_void TickerContinue(T_void)
{
    /* Sub one from the pause level. */
    G_pauseLevel-- ;
}
#endif


/****************************************************************************/
/*    END OF FILE:  TICKER.C                                                */
/****************************************************************************/
