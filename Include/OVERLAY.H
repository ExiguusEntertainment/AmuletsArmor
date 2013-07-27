/****************************************************************************/
/*    FILE:  OVERLAY .H                                                     */
/****************************************************************************/
#ifndef _OVERLAY_H_
#define _OVERLAY_H_

#include "GENERAL.H"

#define ANIMATION_NUMBER_UNKNOWN 10

#define ANIMATION_OVERLAY_FIST           0
#define ANIMATION_OVERLAY_SWORD          1
#define ANIMATION_OVERLAY_AXE            2
#define ANIMATION_OVERLAY_MAGIC          3
#define ANIMATION_OVERLAY_XBOW           4
#define ANIMATION_OVERLAY_MACE           5
#define ANIMATION_OVERLAY_DAGGER         6
#define ANIMATION_OVERLAY_STAFF          7
#define ANIMATION_OVERLAY_WAND           8
#define ANIMATION_OVERLAY_NULL           9

typedef T_void (*T_overlayCallback)(T_word16 animationNumber, T_byte8 flags) ;

T_void OverlayInitialize(T_void) ;

T_void OverlayFinish(T_void) ;

T_void OverlaySetAnimation(T_word16 animationNumber) ;

T_void OverlayAnimate(T_word32 speed) ;

T_void OverlaySetCallback(T_overlayCallback p_callback) ;

T_void OverlayUpdate(E_Boolean isPaused) ;

T_void OverlayDraw(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom,
           T_sword16 xOffset,
           T_sword16 yOffset) ;

E_Boolean OverlayIsDone(T_void);

T_void OverlaySetTranslucencyMode(E_Boolean mode) ;

#endif

/****************************************************************************/
/*    END OF FILE:  OVERLAY .H                                              */
/****************************************************************************/
