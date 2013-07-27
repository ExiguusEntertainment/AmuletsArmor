/****************************************************************************/
/*    FILE:  TICKER.H                                                       */
/****************************************************************************/
#ifndef _TICKER_H_
#define _TICKER_H_

#include "GENERAL.H"

/* Option to turn on 500/sec interrupting for accurate timing of routines. */
//#define HISPEED_TESTING

#define TICKS_PER_SECOND 70

T_void TickerOn(T_void) ;

T_void TickerOff(T_void) ;

T_word32 TickerGet(T_void) ;

T_word32 TickerGetAccurate(T_void) ;

T_void TickerPause(T_void) ;

T_void TickerContinue(T_void) ;

T_void TickerInc(T_void) ;

#ifdef HISPEED_TESTING
#  define TICKER_TIME_ROUTINE_PREPARE()         \
            static T_word16 __ttCount = 0 ;   \
            static T_word32 __ttTotal = 0 ;   \
            static T_word32 __ttStart

#  define TICKER_TIME_ROUTINE_START()     \
            __ttStart = TickerGetAccurate()

#  define TICKER_TIME_ROUTINE_END(f, s, n) \
            __ttTotal += TickerGetAccurate() - __ttStart ; \
            __ttCount++ ; \
            if (__ttCount >= (n))  { \
              fprintf((f), "%s: %8.3f\n", (s), (1000.0/560.0)*((double)__ttTotal)/((double)(n))) ; \
              __ttCount = 0 ; \
              __ttTotal = 0 ; \
            }

#  define TICKER_TIME_ROUTINE_ENDM(s, n) \
            __ttTotal += TickerGetAccurate() - __ttStart ; \
            __ttCount++ ; \
            if (__ttCount >= (n))  { \
              MessagePrintf("%s: %8.3f", (s), (1000.0/560.0)*((double)__ttTotal)/((double)(n))) ; \
              printf("%s: %8.3f\n", (s), (1000.0/560.0)*((double)__ttTotal)/((double)(n))) ; \
              __ttCount = 0 ; \
              __ttTotal = 0 ; \
            }
#else
#  define TICKER_TIME_ROUTINE_PREPARE()
#  define TICKER_TIME_ROUTINE_START()
#  define TICKER_TIME_ROUTINE_END(f, s, n)
#  define TICKER_TIME_ROUTINE_ENDM(s, n)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  TICKER.H                                                */
/****************************************************************************/
