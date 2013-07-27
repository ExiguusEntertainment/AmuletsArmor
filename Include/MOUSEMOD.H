/****************************************************************************/
/*    FILE:  MOUSEMOD.H                                                     */
/****************************************************************************/

#ifndef _MOUSEMOD_H_
#define _MOUSEMOD_H_

#include "GENERAL.H"
#include "GRAPHICS.H"

typedef enum {
    MOUSE_EVENT_START,
    MOUSE_EVENT_END,
    MOUSE_EVENT_MOVE,
    MOUSE_EVENT_DRAG,
    MOUSE_EVENT_IDLE,
    MOUSE_EVENT_HELD,
    MOUSE_EVENT_UNKNOWN
} E_mouseEvent ;

typedef T_byte8 T_buttonClick ;
#define MOUSE_BUTTON_LEFT 0x01       /* ---- ---1 */
#define MOUSE_BUTTON_RIGHT 0x02      /* ---- --1- */
#define MOUSE_BUTTON_MIDDLE 0x04     /* ---- -1-- */
#define MOUSE_BUTTON_UNKNOWN 0xF8    /* 1111 10-- */

#define MOUSE_EVENT_OPTION_HANDLE_START      0x01    /* -------1 */
#define MOUSE_EVENT_OPTION_HANDLE_END        0x02    /* ------1- */
#define MOUSE_EVENT_OPTION_HANDLE_MOVE       0x04    /* -----1-- */
#define MOUSE_EVENT_OPTION_HANDLE_DRAG       0x08    /* ----1--- */
#define MOUSE_EVENT_OPTION_HANDLE_IDLE       0x10    /* ---1---- */
#define MOUSE_EVENT_OPTION_HANDLE_HELD       0x20    /* --1----- */
#define MOUSE_EVENT_OPTION_UNKNOWN           0xC0    /* 11------ */

#define MOUSE_EVENT_DEFAULT_OPTIONS    \
            (MOUSE_EVENT_OPTION_HANDLE_START | \
             MOUSE_EVENT_OPTION_HANDLE_END   | \
             MOUSE_EVENT_OPTION_HANDLE_MOVE  | \
             MOUSE_EVENT_OPTION_HANDLE_DRAG)

typedef T_void (*T_mouseEventHandler)(E_mouseEvent event,
                                      T_word16 x,
                                      T_word16 y,
                                      T_buttonClick buttons) ;

typedef T_word16 *T_mousePicture ;

T_void MouseInitialize(T_void) ;

T_void MouseSetEventHandler(T_mouseEventHandler p_handler) ;
T_mouseEventHandler MouseGetEventHandler (T_void);

T_void MouseSetEventOptions(T_byte8 f_options) ;

E_Boolean MouseCheckInstalled(T_void) ;

T_void MouseShow(T_void) ;

T_void MouseHide(T_void) ;

T_void MouseMoveTo(T_word16 x, T_word16 y) ;

T_void MouseSetBounds(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom) ;

T_void MouseReleaseBounds(T_void) ;

T_void MouseSetBitmap(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_bitmap *p_bitmap) ;

T_void MouseGetBitmap(
          T_word16 *hot_spot_x,
          T_word16 *hot_spot_y,
          T_bitmap **p_bitmap) ;

T_void MouseSetDefaultBitmap(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_bitmap *p_bitmap) ;

T_void MouseUseDefaultBitmap(void) ;

T_void MouseSetPicture(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_mousePicture picture) ;

T_void MouseBlockEvents(T_void) ;

T_void MouseAllowEvents(T_void) ;

T_void MouseUpdateEvents(T_void) ;

T_void MouseFinish(T_void) ;

T_void MouseSetColorize(E_colorizeTable colorTable) ;

T_void MouseDraw(T_void) ;

T_void MouseErase(T_void) ;

T_void MouseDisallowUpdate(T_void) ;

T_void MouseAllowUpdate(T_void) ;

T_void MousePushEventHandler(T_mouseEventHandler p_handler) ;

T_void MousePopEventHandler(T_void) ;

// Functions for turning on relative mode
T_void MouseRelativeModeOn(T_void);
T_void MouseRelativeModeOff(T_void);
E_Boolean MouseIsRelativeMode(T_void);
T_void MouseRelativeRead(T_sword16 *aDeltaX, T_sword16 *aDeltaY);

T_buttonClick MouseGetButtonStatus(T_void);

#endif

/****************************************************************************/
/*    END OF FILE:  MOUSEMOD.H                                              */
/****************************************************************************/
