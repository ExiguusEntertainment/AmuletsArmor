/****************************************************************************/
/*    FILE:  SLIDER.H                                                       */
/****************************************************************************/
#ifndef _SLIDER_H_
#define _SLIDER_H_

#include "GENERAL.H"

typedef T_void *T_sliderHandle ;
#define SLIDER_HANDLE_BAD NULL

#define SLIDER_TYPE_FLOOR    1
#define SLIDER_TYPE_CEILING  2

typedef enum {
    SLIDER_RESPONSE_STOP,
    SLIDER_RESPONSE_CONTINUE,
    SLIDER_RESPONSE_PAUSE,
    SLIDER_RESPONSE_UNKNOWN
} T_sliderResponse ;

typedef T_sliderResponse (*T_sliderEventHandler)
                        (T_word32 sliderId,
                         T_sword32 value,
                         E_Boolean isDone) ;

T_void SliderInitialize(T_void) ;

T_void SliderFinish(T_void) ;

T_void SliderStart(
           T_word32 sliderId,
           T_sword32 start,
           T_sword32 end,
           T_sword32 time,
           T_sliderEventHandler handler,
           T_word16 finalActivity) ;

T_void SliderCancel(T_word32 sliderId) ;

T_void SliderDestroy(T_word32 sliderId) ;

T_void SliderReverse(T_word32 sliderId, T_word16 newActivity) ;

E_Boolean SliderExist(T_word32 sliderId) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SLIDER.H                                                */
/****************************************************************************/
