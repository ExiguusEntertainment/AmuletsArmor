/****************************************************************************/
/*    FILE:  KEYBOARD.H                                                     */
/****************************************************************************/
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "GENERAL.H"

#define KeyDual(a, b) (((T_word16)a) | (((T_word16)b) << 8))

typedef enum {
    KEYBOARD_EVENT_NONE,
    KEYBOARD_EVENT_PRESS,
    KEYBOARD_EVENT_RELEASE,
    KEYBOARD_EVENT_BUFFERED,
    KEYBOARD_EVENT_HELD,
    KEYBOARD_EVENT_UNKNOWN
} E_keyboardEvent ;

typedef T_void (*T_keyboardEventHandler)(E_keyboardEvent event,
                                         T_word16 scankey) ;

/* Standard keyboard functions: */

T_void KeyboardOn(T_void) ;

T_void KeyboardOff(T_void) ;

E_Boolean KeyboardGetScanCode(T_word16 scancode) ;

T_void KeyboardDebounce(T_void) ;

T_void KeyboardSetEventHandler(T_keyboardEventHandler keyboardEventHandler) ;
T_keyboardEventHandler KeyboardGetEventHandler (T_void);

T_void KeyboardUpdateEvents(T_void) ;

/* Buffered keystroke commands. */

T_void KeyboardBufferOn(T_void) ;

T_void KeyboardBufferOff(T_void) ;

T_byte8 KeyboardBufferGet(T_void) ;

T_word32 KeyboardGetHeldTimeAndClear(T_word16 scanCode) ;

T_void KeyboardPushEventHandler(T_keyboardEventHandler keyboardEventHandler) ;

T_void KeyboardPopEventHandler(T_void) ;

T_void KeyboardDisallowKeyScans(T_void) ;

T_void KeyboardAllowKeyScans(T_void) ;

E_Boolean KeyboardBufferReady(T_void) ;

#endif

/****************************************************************************/
/*    END OF FILE:  KEYBOARD.H                                              */
/****************************************************************************/
