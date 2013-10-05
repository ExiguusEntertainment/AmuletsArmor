/*-------------------------------------------------------------------------*
 * File:  KEYBOARD.C
 *-------------------------------------------------------------------------*/
/**
 * Routines for tracking which keys are being pressed.  In DOS, we even
 * tried to make so we could tell how long a key was pressed.  This is
 * less of an issue these days.
 *
 * @addtogroup KEYBOARD
 * @brief Keyboad Controls
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "OPTIONS.H"
#if WIN32
#include <SDL.h>
#endif
#include <string.h>
#include "DBLLINK.H"
#include "KEYBOARD.H"
#include "KEYSCAN.H"
#include "TICKER.H"

#ifdef WATCOM
#define ALLOW_KEYBOARD_INTERRUPT
#endif

#ifdef ALLOW_KEYBOARD_INTERRUPT
#include <bios.h>

#define KEYBOARD_INTERRUPT_NUMBER 9
#define DISABLE_KEYBOARD 0xAD
#define ENABLE_KEYBOARD 0xAE
#define KEYBOARD_DATA 0x60
#define STATUS_PORT 0x64
#define INPUT_BUFFER_FULL 0x02
#endif

#define KEYBOARD_HIGHEST_SCAN_CODE 255

/* Number of characters that can be held in keyboard buffer. */
/* Must be a power of two. */
#define MAX_SCAN_KEY_BUFFER 128

/* Note if the keyboard is on: */
static E_Boolean F_keyboardOn = FALSE ;

/* Note if we are using the keyboard buffer: */
static E_Boolean F_keyboardBufferOn = TRUE ;

/* Table to keep track of which keys are pressed: */
static E_Boolean G_keyTable[256] ;

/* Also keep a table to track which events for what keys have occured: */
static E_Boolean G_keyEventTable[256] ;

/* Information about how long the key was pressed. */
static T_word32 G_keyHoldTime[256] ;
static T_word32 G_keyPressTime[256] ;

/* Internal buffer of press and release keys: */
static T_word16 G_scanKeyBuffer[MAX_SCAN_KEY_BUFFER] ;
static T_word16 G_scanKeyStart = 0 ;
static T_word16 G_scanKeyEnd = 0 ;

/* Number of keys being pressed simultaneously: */
static T_word16 G_keysPressed = 0 ;

/* Pointer to old BIOS key handler. */
#ifdef ALLOW_KEYBOARD_INTERRUPT
static T_void (__interrupt __far *IOldKeyboardInterrupt)(T_void);
#endif

/* Keep track of the event handler for the keyboard. */
static T_keyboardEventHandler G_keyboardEventHandler ;

/* Flag to tell if KeyboardGetScanCode works */
static E_Boolean G_allowKeyScans = TRUE ;

/* Internal routines: */
#ifdef ALLOW_KEYBOARD_INTERRUPT
static T_void __interrupt __far IKeyboardInterrupt(T_void);
#endif

static T_void IKeyboardClear(T_void) ;

static T_void IKeyboardSendCommand(T_byte8 keyboardCommand) ;

static E_Boolean G_escapeCode = FALSE ;

static T_doubleLinkList G_eventStack = DOUBLE_LINK_LIST_BAD ;

static T_word16 G_pauseLevel = 0 ;

#ifdef WIN32
char G_keyboardToASCII[256] = {
     '\0',  '\0',  '1',   '2',   '3',   '4',   '5',   '6',
     '7',   '8',   '9',   '0',   '-',   '=',   '\b',  '\t',
     'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',
     'o',   'p',   '[',   ']',   '\r',  '\0',  'a',   's',
     'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',
     '\'',  '`',  '\0',  '\\',  'z',   'x',   'c',   'v',
     'b',   'n',   'm',   ',',   '.',   '/',  '\0',  '\0',
     '\0',  '\040', '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '!',   '@',   '#',   '$',   '%',  '^',
     '&',   '*',   '(',   ')',   '_',   '+',   '\b',  '\t',
     'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',
     'O',   'P',   '{',   '}',   '\r',  '\0',  'A',   'S',
     'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',
     '\"',  '~',  '\0',  '|',   'Z',   'X',   'C',   'V',
     'B',   'N',   'M',   '<',   '>',   '?',  '\0',  '\0',
     '\0',  '\040', '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
} ;
#endif

#ifdef WATCOM
char G_keyboardToASCII[256] = {
     '\0',  '\0',  '1',   '2',   '3',   '4',   '5',   '6',
     '7',   '8',   '9',   '0',   '-',   '=',   '\b',  '\t',
     'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',
     'o',   'p',   '[',   ']',   '\r',  '\0',  'a',   's',
     'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',
     '\'',  '\`',  '\0',  '\\',  'z',   'x',   'c',   'v',
     'b',   'n',   'm',   ',',   '.',   '\/',  '\0',  '\0',
     '\0',  '\040', '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '!',   '@',   '#',   '$',   '\%',  '\^',
     '&',   '*',   '(',   ')',   '_',   '+',   '\b',  '\t',
     'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',
     'O',   'P',   '{',   '}',   '\r',  '\0',  'A',   'S',
     'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',
     '\"',  '\~',  '\0',  '|',   'Z',   'X',   'C',   'V',
     'B',   'N',   'M',   '<',   '>',   '?',  '\0',  '\0',
     '\0',  '\040', '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
     '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',  '\0',
} ;
#endif


static char G_asciiBuffer[32] ;
static T_word16 G_asciiStart = 0 ;
static T_word16 G_asciiEnd = 0 ;

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardOn
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardOn installs the keyboard functions of the following commands.
 *
 *  NOTE: 
 *  This routine can not be called twice in a row and must be called
 *  once for the rest of the routines to work correctly (Except the
 *  keyboard buffer commands).
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardOn(T_void)
{
    DebugRoutine("KeyboardOn") ;
    DebugCheck(F_keyboardOn == FALSE) ;

    /* Note that the keyboard is now on. */
    F_keyboardOn = TRUE ;

#ifdef ALLOW_KEYBOARD_INTERRUPT
    /* We are doing somewhat sensitive stuff, so turn off the interrupts. */
    _disable() ;

    /* First get the old keyboard interrupt we used to use. */
    IOldKeyboardInterrupt = _dos_getvect(KEYBOARD_INTERRUPT_NUMBER);

    /* Now declare our new interrupt to the timer. */
    _dos_setvect(KEYBOARD_INTERRUPT_NUMBER, IKeyboardInterrupt);

    /* Clear the keyboard and event keyboard before we let things go. */
    IKeyboardClear() ;

    /* Done twiddling with the hardware, turn back on the interrupts. */
    _enable() ;
#else
    /* Clear the keyboard and event keyboard before we let things go. */
    IKeyboardClear() ;
#endif
    G_eventStack = DoubleLinkListCreate() ;
    DebugCheck(G_eventStack != DOUBLE_LINK_LIST_BAD) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardOff
 *-------------------------------------------------------------------------*/
/**
 *  When you are done with the keyboard, you can turn it off by calling
 *  KeyboardOff.  This will remove the keyboard driver from memory.
 *
 *  NOTE: 
 *  This routine MUST be called before exiting or DOS will crash.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardOff(T_void)
{
    DebugRoutine("KeyboardOff") ;
    DebugCheck(F_keyboardOn == TRUE) ;

    DebugCheck(G_eventStack != DOUBLE_LINK_LIST_BAD) ;
    DoubleLinkListDestroy(G_eventStack) ;

#ifdef ALLOW_KEYBOARD_INTERRUPT
    /* We are doing somewhat sensitive stuff, so turn off the interrupts. */
    _disable() ;

    /* Replace the new driver with the old driver. */
    _dos_setvect(KEYBOARD_INTERRUPT_NUMBER, IOldKeyboardInterrupt);

    /* Done twiddling with the hardware, turn back on the interrupts. */
    _enable() ;
#endif

    /* Note that the keyboard is now off. */
    F_keyboardOn = FALSE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardGetScanCode
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardGetScanCode is used to check if the key for the corresponding
 *  scan code is being pressed.  If it is, a TRUE is returned, or else
 *  false is returned.
 *
 *  @param scanCode -- Scan code to check
 *
 *  @return TRUE  = Key is pressed
 *      FALSE = Key is not pressed
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IGetAdjustedKey(T_word16 scanCode)
{
    switch (scanCode)
    {
        case 0x38 /** Left alt key **/ :
        case 0xB8 /** Right alt key **/ :
           return G_keyTable[0x38] |
                  G_keyTable[0xB8] ;
           break;

        case KEY_SCAN_CODE_RIGHT_SHIFT:
        case KEY_SCAN_CODE_LEFT_SHIFT:
           return G_keyTable[KEY_SCAN_CODE_RIGHT_SHIFT] |
                  G_keyTable[KEY_SCAN_CODE_LEFT_SHIFT] ;
           break;

        case 0x9D /** Right control key **/ :
        case 0x1D /** Left control key **/ :
           return G_keyTable[0x9D] |
                  G_keyTable[0x1D] ;
           break;

        default:
           break;
    }
    return G_keyTable[scanCode] ;
}

E_Boolean KeyboardGetScanCode(T_word16 scanCodes)
{
    E_Boolean isPressed ;
    T_byte8 scanCode, scanCode2 ;

    DebugRoutine("KeyboardGetScanCode") ;
    DebugCheck(F_keyboardOn == TRUE) ;

    /* Check to see if key scans are allowed */
    /* (ESC and ALT and ENTER key break this rule) */
    /* Also function keys */
    if ((G_allowKeyScans == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_ESC) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_ENTER) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_ALT) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F1) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F2) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F3) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F4) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F5) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F6) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F7) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F8) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F9) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F10) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F11) == FALSE)
        && (IGetAdjustedKey(KEY_SCAN_CODE_F12) == FALSE))  {
        /* Pretend the key is not pressed if in this mode. */
        isPressed = FALSE ;
    } else {
        scanCode = scanCodes & 0xFF ;
        scanCode2 = (scanCodes >> 8) ;

        DebugCheck(scanCode <= KEYBOARD_HIGHEST_SCAN_CODE) ;
        DebugCheck(scanCode2 <= KEYBOARD_HIGHEST_SCAN_CODE) ;

        if (scanCode2 == 0)
            isPressed = IGetAdjustedKey(scanCode) ;
        else
            isPressed = IGetAdjustedKey(scanCode) |
                        IGetAdjustedKey(scanCode) ;
    }

    DebugCheck(isPressed < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return isPressed ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardDebounce
 *-------------------------------------------------------------------------*/
/**
 *  Sometimes it is useful to wait for all keys to be released.
 *  KeyboardDebounce is a simple routine that just waits until the user
 *  has released all pressed keys.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardDebounce(T_void)
{
    DebugRoutine("KeyboardDebounce") ;
    DebugCheck(F_keyboardOn == TRUE) ;

#if 0
    T_word32 time ;

    time = TickerGet() ;

//puts("Waiting") ;
    while (G_keysPressed != 0)
        { /* wait */
            if ((TickerGet() - time) > 210)
                break ;
        }
//puts("Done") ;

    /* Clear out the keyboard stats. */
//    IKeyboardClear() ;
#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardSetEventHandler
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardSetEventHandler declares the function to be called for
 *  each keyboard event as dispatched by the KeyboardUpdateEvents routine.
 *  If you no longer need a keyboard handler, give this routine an
 *  input parameter of NULL.
 *
 *  @param keyboardEventHandler -- function to call on events, or NULL
 *      for none.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardSetEventHandler(T_keyboardEventHandler keyboardEventHandler)
{
    DebugRoutine("KeyboardSetEventHandler") ;
    /* Nothing to do a DebugCheck on ... */

    /* Record the event handler for future use. */
    G_keyboardEventHandler = keyboardEventHandler ;

    DebugEnd() ;
}


T_keyboardEventHandler KeyboardGetEventHandler (T_void)
{
    return (G_keyboardEventHandler);
}



/*-------------------------------------------------------------------------*
 * Routine:  KeyboardUpdateEvents
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardUpdateEvents should be called often for it to work correctly
 *  with a keyboard event handler.  Events are sent by this routine to
 *  tell what keys are being pressed and released.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardUpdateEvents(T_void)
{
    T_word16 scanCode ;

    DebugRoutine("KeyboardUpdateEvents") ;
    DebugCheck(F_keyboardOn == TRUE) ;

    while (G_scanKeyStart != G_scanKeyEnd)  {
        scanCode = G_scanKeyBuffer[G_scanKeyStart] ;
        /* Is this a press or a release? */
        if (scanCode & 0x100)  {
            /* THis is a release. */
            G_keyboardEventHandler(KEYBOARD_EVENT_RELEASE, scanCode & 0xFF) ;
        } else {
            G_keyboardEventHandler(KEYBOARD_EVENT_PRESS, scanCode) ;
        }

        /* Special case for the right ctrl key -- we want it to act */
        /* just like the left control key. */
        if ((scanCode & 0xFF) == KEY_SCAN_CODE_RIGHT_CTRL)  {
            if (scanCode & 0x100)  {
                /* THis is a release. */
                G_keyboardEventHandler(KEYBOARD_EVENT_RELEASE, KEY_SCAN_CODE_CTRL) ;
            } else {
                G_keyboardEventHandler(KEYBOARD_EVENT_PRESS, KEY_SCAN_CODE_CTRL) ;
            }
        }
        _disable();
        G_scanKeyStart = (G_scanKeyStart+1) & (MAX_SCAN_KEY_BUFFER-1) ;
        _enable();
    }

    /* Now we will look for all keys that are being held and send */
    /* events appropriately. */
    for (scanCode = 0; scanCode <= KEYBOARD_HIGHEST_SCAN_CODE; scanCode++)
        /* Check to see if key is being held. */
        if (G_keyTable[scanCode] == TRUE)
            /* We need to send a event. */
            G_keyboardEventHandler(KEYBOARD_EVENT_HELD, scanCode) ;

    /* Add an event for the right ctrl to be the left ctrl. */
    if (G_keyTable[KEY_SCAN_CODE_RIGHT_CTRL] == TRUE)
        G_keyboardEventHandler(KEYBOARD_EVENT_HELD, KEY_SCAN_CODE_CTRL) ;

    /* Check to see if we are in buffered mode. */
    if (F_keyboardBufferOn == TRUE)  {
        /* If we are, loop while there are characters in the buffer. */
        while (KeyboardBufferReady())  {
            /* Send a special buffered event for each */
            /* of these characters. */
            G_keyboardEventHandler(
                KEYBOARD_EVENT_BUFFERED,
                KeyboardBufferGet()) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardBufferOn
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardBufferOn will allow keys to be buffered up for use later.
 *  Although useful in word processors, when the buffer gets full it
 *  causes a beeping noise that is not wonderful in games.  Depending on
 *  your need, you may wnat the buffer on.
 *
 *  NOTE:
 *  KeyboardBufferOn CAN be called even if KeyboardOn is not.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardBufferOn(T_void)
{
    DebugRoutine("KeyboardBufferOn") ;

    F_keyboardBufferOn = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardBufferOff
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardBufferOff will not allow keys to be buffered up for use later.
 *  Although useful in word processors, when the buffer gets full it
 *  causes a beeping noise that is not wonderful in games.  Depending on
 *  your need, you may wnat the buffer off.
 *
 *  NOTE:
 *  KeyboardBufferOff CAN be called even if KeyboardOn is not.
 *  The default is for the buffer to be on.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardBufferOff(T_void)
{
    DebugRoutine("KeyboardBufferOff") ;

//    F_keyboardBufferOn = FALSE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardBufferGet
 *-------------------------------------------------------------------------*/
/**
 *  If you are using a buffer, you can get keystrokes from the buffer
 *  by using this routine.
 *
 *  @return ASCII character of gained character
 *      or 0 for none.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 KeyboardBufferGet(T_void)
{
    T_byte8 key ;
    T_word16 next ;

    DebugRoutine("KeyboardBufferGet") ;

#if 0
    if (_bios_keybrd(1))  {
        key = _bios_keybrd(0) ;
//        if (key == 0)
//            getch() ;
    } else
        key = 0 ;
#endif

    /* Are there any keys waiting? */
    if (G_asciiStart != G_asciiEnd)  {
        /* Find where the next position will be */
        next = (G_asciiStart+1)&31 ;

        /* Get this character */
        key = G_asciiBuffer[G_asciiStart] ;

        /* Advance in the queue. */
        G_asciiStart = next ;
    } else {
        /* No keys waiting. */
        key = 0 ;
    }

    DebugEnd() ;

    return key ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IKeyboardInterrupt
 *-------------------------------------------------------------------------*/
/**
 *  IKeyboardInterrupt is the routine that is called every time a key
 *  on the keyboard is either pressed or released.  If keyboard buffering
 *  is on, the regular BIOS routines are also called.
 *
 *<!-----------------------------------------------------------------------*/
#ifdef ALLOW_KEYBOARD_INTERRUPT
static T_void __interrupt __far IKeyboardInterrupt(T_void)
{
    T_byte8 portStatus ;
    T_byte8 scanCode ;
    E_Boolean newStatus ;
    T_word16 next ;
    T_word16 c ;

    /* We first must disable any new key strokes from occuring. */
    IKeyboardSendCommand(DISABLE_KEYBOARD) ;

    /* Turn off the standard interrupts. */
    _disable() ;

    /* Wait for the keyboard to prepare itself. */
	do {
		portStatus = inp(STATUS_PORT) ;
	} while ((portStatus & INPUT_BUFFER_FULL)==INPUT_BUFFER_FULL) ;

    /* Now that we are ready, let me get the scanCode. */
	scanCode = inp(KEYBOARD_DATA) ;

    /* Turn back on the interrupts. */
	_enable() ;

    if (scanCode == 0xE0)  {
        G_escapeCode = TRUE ;
    } else if (scanCode == 0xE1)  {
        /* Pause key was hit */
        G_pauseLevel++ ;
    } else {
        /* Store either a true or false based on if the key is pressed. */
        newStatus = (scanCode > 127) ? FALSE : TRUE ;

        /* Find reference into tables. */
        scanCode &= 127 ;

        if (G_escapeCode == TRUE)  {
            scanCode |= 128 ;
            G_escapeCode = FALSE ;
        }

        /* Check to see if there is a pause level */
        if ((scanCode == 0x45) || (scanCode == 0xC5))  {
            if (G_pauseLevel == 1)  {
                scanCode = KEY_SCAN_CODE_PAUSE ;
                newStatus = TRUE ;
            } else if (G_pauseLevel == 2)  {
                scanCode = KEY_SCAN_CODE_PAUSE ;
                newStatus = FALSE ;
                G_pauseLevel = 3 ;
            }
        }

        /* Store the key in the scan key buffer. */
        G_scanKeyBuffer[G_scanKeyEnd] = scanCode | ((newStatus)?0:0x100) ;
        next = (G_scanKeyEnd+1) & (MAX_SCAN_KEY_BUFFER-1) ;
        if (next != G_scanKeyStart)
            G_scanKeyEnd = next ;

        /* If a release and an ascii character, store that in the */
        /* keyboard buffer. */
        if ((newStatus) &&
            (G_keyTable[KEY_SCAN_CODE_ALT]==FALSE) &&
            (G_keyTable[KEY_SCAN_CODE_RIGHT_CTRL]==FALSE) &&
            (G_keyTable[KEY_SCAN_CODE_LEFT_CTRL]==FALSE))  {
            c = scanCode & 0xFF;

            /* Translate for the shift key */
            if (G_keyTable[KEY_SCAN_CODE_RIGHT_SHIFT] |
                  G_keyTable[KEY_SCAN_CODE_LEFT_SHIFT])
                c |= 0x80 ;

            /* What ascii character is this? */
            c = G_keyboardToASCII[c] ;

            /* Don't do if a null character */
//            if (c != 0)  {
                /* Where are we going to roll over to next */
                next = (G_asciiEnd+1)&31 ;

                /* If back at start, don't do */
                if (next != G_asciiStart)  {
                    /* Store in the buffer */
                    G_asciiBuffer[G_asciiEnd] = c ;
                    G_asciiEnd = next ;
                }
//            }
        }

        /* Check to see if keystroke changed from pressed to released or */
        /* visa-versa.  If it has, update the key count appropriately. */
        if ((G_keyTable[scanCode] == FALSE) && (newStatus == TRUE))  {
            G_keysPressed++ ;
            /* Note the time that key is pressed. */
            G_keyPressTime[scanCode] = TickerGet() ;
        } else if ((G_keyTable[scanCode] == TRUE) && (newStatus == FALSE))  {
            G_keysPressed-- ;
            /* Note how long the key has been held down. */
            G_keyHoldTime[scanCode] += TickerGet() - G_keyPressTime[scanCode] ;
        }

#if 0
        /* In any case, store the new state. */
        /** Make the right and left side codes for ALT, CTRL, and SHIFT **/
        /** the same. **/
        switch (scanCode)
        {
            case 0x38 /** Left alt key **/ :
            case 0xB8 /** Right alt key **/ :
               G_keyTable[KEY_SCAN_CODE_ALT] = newStatus;
               break;

            case KEY_SCAN_CODE_RIGHT_SHIFT:
            case KEY_SCAN_CODE_LEFT_SHIFT:
               G_keyTable[KEY_SCAN_CODE_RIGHT_SHIFT] =
                  G_keyTable[KEY_SCAN_CODE_LEFT_SHIFT] = newStatus;
               break;

            case 0x9D /** Right control key **/ :
            case 0x1D /** Left control key **/ :
               G_keyTable[KEY_SCAN_CODE_CTRL] = newStatus;
               break;

            default:
               G_keyTable[scanCode] = newStatus ;
               break;
        }
#endif
        G_keyTable[scanCode] = newStatus ;

    }

    /* Turn the keyboard back on. */
    IKeyboardSendCommand(ENABLE_KEYBOARD) ;

    /* If keyboard buffer is on, go ahead and let the BIOS also */
    /* process this keystroke. */
        /* Otherwise, signify end of interrupt */
    outp(0x20,0x20) ;

    if (G_pauseLevel == 3)
        G_pauseLevel = 0 ;


    /* If we are doing a debug compile, we want to have the capability */
    /* to press F12 and turn on the keyboard buffer. */
    /* This allows CTRL-ALT-DEL and CTRL-C to work correctly. */
#ifndef NDEBUG
    if (G_keyTable[KEY_SCAN_CODE_F12] == TRUE)
        F_keyboardBufferOn = TRUE ;

    /* As well, we also want to be able to break out of the program */
    /* when we hit CTRL-F12.  This should keep the system from */
    /* having to reboot. */
    if ((G_keyTable[KEY_SCAN_CODE_F12] == TRUE) &&
        (G_keyTable[KEY_SCAN_CODE_CTRL]==TRUE))  {
        DebugRoutine("IKeyboardInterrupt -- Keyboard break") ;
//        DebugCheck(FALSE) ;
        exit(1) ;
    }
    if ((G_keyTable[KEY_SCAN_CODE_F11] == TRUE) &&
        (G_keyTable[KEY_SCAN_CODE_CTRL]==TRUE))  {
        DebugStop() ;
    }
#endif
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IKeyboardSendCommand
 *-------------------------------------------------------------------------*/
/**
 *  IKeyboardSendCommand is used by IKeyboardInterrupt to tell the
 *  keyboard that certain events are taking place (most likely turning
 *  on and off the keyboard interrupts).
 *
 *  @param keyboardCommand -- command to send keyboard
 *
 *<!-----------------------------------------------------------------------*/
#ifdef ALLOW_KEYBOARD_INTERRUPT
static T_void IKeyboardSendCommand(T_byte8 keyboardCommand)
{
    T_byte8 statusPort ;

    /* cannot easily debug this routine since part of interrupt. */

    /* Don't allow any interrupts. */
	_disable() ;

    /* Wait for the status port to say we are ready. */
	do {
		statusPort = inp(STATUS_PORT) ;
	} while ((statusPort & INPUT_BUFFER_FULL) == INPUT_BUFFER_FULL) ;

    /* Send the command. */
	outp(STATUS_PORT, keyboardCommand) ;

    /* Turn back on interrupts. */
	_enable() ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IKeyboardClear
 *-------------------------------------------------------------------------*/
/**
 *  IKeyboardClear is a routine that is called to clear the whole
 *  key tables of their state and assume that all keys are NOT being
 *  pressed (even if they are).
 *
 *  NOTE: 
 *  If this routine is called and a key IS being pressed, it will be
 *  ignored until the key is released and then pressed again.  In other
 *  words, you should only call this routine on startup.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IKeyboardClear(T_void)
{
    T_word16 scanCode ;

    DebugRoutine("IKeyboardClear") ;

    _disable();

    /* Clear all scan codes stored in the key table and key event table. */
    for (scanCode = 0; scanCode <= KEYBOARD_HIGHEST_SCAN_CODE; scanCode++)  {
        G_keyTable[scanCode] = FALSE ;
        G_keyEventTable[scanCode] = FALSE ;
        G_keyHoldTime[scanCode] = 0 ;
        G_keyPressTime[scanCode] = 0 ;
    }

    memset(G_scanKeyBuffer, 0, sizeof(G_scanKeyBuffer)) ;
    G_scanKeyStart = G_scanKeyEnd = 0 ;

    /* Note that none of the keys are pressed. */
    G_keysPressed = 0 ;

    G_asciiStart = G_asciiEnd = 0 ;

    _enable();

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardGetHeldTimeAndClear
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardGetHeldTimeAndClear tells how long a key has been held
 *  down since the key was last cleared.  This capability is useful
 *  for getting accurate readings of how long the user desired the key
 *  to be pressed, and not based on when calls were able to be made.
 *
 *  NOTE: 
 *  Calling this routine can be a problem if it has been a long time
 *  since it was previously called on a key.  If you are unsure, be sure
 *  to make a call to KeyboardDebounce which clears the timing values.
 *
 *<!-----------------------------------------------------------------------*/
T_word32 KeyboardGetHeldTimeAndClear(T_word16 scanCode)
{
    T_word32 timeHeld ;
    T_word32 time ;

    DebugRoutine("KeyboardGetHeldTimeAndClear") ;

    /* No interrupts, please. */
    _disable() ;

    /* Is the key being currently pressed? */
    if (G_keyTable[scanCode])  {
        /* It is being pressed. */
        /* Return the time held in the past plus the time currently held */
        time = TickerGet() ;
        timeHeld = G_keyHoldTime[scanCode] + (time-G_keyPressTime[scanCode]) ;
        G_keyPressTime[scanCode] = time ;
    } else {
        /* It is not being pressed. */
        /* Return the time held in the register. */
        timeHeld = G_keyHoldTime[scanCode] ;
    }

    /* Always clear the time held. */
    G_keyHoldTime[scanCode] = 0 ;

    /* Allow interrupts again. */
    _enable() ;

    /* If no key scans are allowed, pretend the key was not hit. */
    if (G_allowKeyScans == FALSE)
        timeHeld = 0 ;

    DebugEnd() ;

    return timeHeld ;
}

/*-------------------------------------------------------------------------*
 * Routine:  KeyboardPushEventHandler
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardPushEventHandler removes the old handler by placing it on
 *  a stack and sets up the new event handler.  The old event handler
 *  will be restored when a call to KeyboardPopEventHandler is called.
 *
 *  @param keyboardEventHandler -- function to call on events, or NULL
 *      for none.
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardPushEventHandler(T_keyboardEventHandler keyboardEventHandler)
{
    DebugRoutine("KeyboardPushEventHandler") ;

    /* Store the old event handler. */
    DoubleLinkListAddElementAtFront(
        G_eventStack,
        (T_void *)KeyboardGetEventHandler) ;

    /* set up the new handler. */
    KeyboardSetEventHandler(keyboardEventHandler) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  KeyboardPopEventHandler
 *-------------------------------------------------------------------------*/
/**
 *  KeyboardPopEventHandler restores the last handler on the stack
 *  (if there is one).
 *
 *<!-----------------------------------------------------------------------*/
T_void KeyboardPopEventHandler(T_void)
{
    T_doubleLinkListElement first ;
    T_keyboardEventHandler handler ;

    DebugRoutine("KeyboardPopEventHandler") ;

    /* Get the old event handler. */
    first = DoubleLinkListGetFirst(G_eventStack) ;
    DebugCheck(first != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    if (first != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        handler = (T_keyboardEventHandler)
                      DoubleLinkListElementGetData(first) ;
        KeyboardSetEventHandler(handler) ;
        DoubleLinkListRemoveElement(first) ;
    }

    DebugEnd() ;
}

T_void KeyboardDisallowKeyScans(T_void)
{
    G_allowKeyScans = FALSE ;
}

T_void KeyboardAllowKeyScans(T_void)
{
    G_allowKeyScans = TRUE ;
}

E_Boolean KeyboardBufferReady(T_void)
{
    if (G_asciiStart != G_asciiEnd)
        return TRUE ;
    else
        return FALSE ;
}



#ifdef WIN32

// This maps the SDL keysym list to the list of scancodes in the original game
const uint8_t G_sdlToScancode[] = {
        0, // 0
        0, // 1
        0, // 2
        0, // 3
        0, // 4
        0, // 5
        0, // 6
        0, // 7
        KEY_SCAN_CODE_BACKSPACE, // SDLK_BACKSPACE		= 8,
        KEY_SCAN_CODE_TAB, // SDLK_TAB		= 9,
        0, // 10
        0, // 11
        0, // SDLK_CLEAR		= 12,
        KEY_SCAN_CODE_ENTER, // SDLK_RETURN		= 13,
        0, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        KEY_SCAN_CODE_PAUSE, // SDLK_PAUSE		= 19,
        0, // 20
        0, // 21
        0, // 22
        0, // 23
        0, // 24
        0, // 25
        0, // 26
        KEY_SCAN_CODE_ESC, // SDLK_ESCAPE		= 27,
        0, // 28
        0, // 29
        0, // 30
        0, // 31
        KEY_SCAN_CODE_SPACE, // SDLK_SPACE		= 32,
        0, // SDLK_EXCLAIM		= 33,
        0, // SDLK_QUOTEDBL		= 34,
        0, // SDLK_HASH		= 35,
        0, // SDLK_DOLLAR		= 36,
        0, // 37
        0, // SDLK_AMPERSAND		= 38,
        0, // SDLK_QUOTE		= 39,
        0, // SDLK_LEFTPAREN		= 40,
        0, // SDLK_RIGHTPAREN		= 41,
        0, // SDLK_ASTERISK		= 42,
        0, // SDLK_PLUS		= 43,
        KEY_SCAN_CODE_COMMA, // SDLK_COMMA		= 44,
        KEY_SCAN_CODE_MINUS, // SDLK_MINUS		= 45,
        KEY_SCAN_CODE_PERIOD, // SDLK_PERIOD		= 46,
        KEY_SCAN_CODE_SLASH, // SDLK_SLASH		= 47,
        KEY_SCAN_CODE_0, // SDLK_0			= 48,
        KEY_SCAN_CODE_1, // SDLK_1			= 49,
        KEY_SCAN_CODE_2, // SDLK_2          = 50,
        KEY_SCAN_CODE_3, // SDLK_3          = 51,
        KEY_SCAN_CODE_4, // SDLK_4          = 52,
        KEY_SCAN_CODE_5, // SDLK_5          = 53,
        KEY_SCAN_CODE_6, // SDLK_6          = 54,
        KEY_SCAN_CODE_7, // SDLK_7          = 55,
        KEY_SCAN_CODE_8, // SDLK_8          = 56,
        KEY_SCAN_CODE_9, // SDLK_9          = 57,
        0, // SDLK_COLON		= 58,
        KEY_SCAN_CODE_SEMI_COLON, // SDLK_SEMICOLON		= 59,
        0, // SDLK_LESS		= 60,
        KEY_SCAN_CODE_EQUAL, // SDLK_EQUALS		= 61,
        0, // SDLK_GREATER		= 62,
        0, // SDLK_QUESTION		= 63,
        0, // SDLK_AT			= 64,
        0, // 65
        0, // 66
        0, // 67
        0, // 68
        0, // 69
        0, // 70
        0, // 71
        0, // 72
        0, // 73
        0, // 74
        0, // 75
        0, // 76
        0, // 77
        0, // 78
        0, // 79
        0, // 80
        0, // 81
        0, // 82
        0, // 83
        0, // 84
        0, // 85
        0, // 86
        0, // 87
        0, // 88
        0, // 89
        0, // 90
	/*
	   Skip uppercase letters
	 */
        KEY_SCAN_CODE_SB_OPEN, // SDLK_LEFTBRACKET	= 91,
        KEY_SCAN_CODE_BACKSLASH, // SDLK_BACKSLASH		= 92,
        KEY_SCAN_CODE_SB_CLOSE, // SDLK_RIGHTBRACKET	= 93,
        0, // SDLK_CARET		= 94,
        0, // SDLK_UNDERSCORE		= 95,
        KEY_SCAN_CODE_GRAVE, // SDLK_BACKQUOTE		= 96,
        KEY_SCAN_CODE_A, // SDLK_a			= 97,
        KEY_SCAN_CODE_B, // SDLK_b			= 98,
        KEY_SCAN_CODE_C, // SDLK_c			= 99,
        KEY_SCAN_CODE_D, // SDLK_d			= 100,
        KEY_SCAN_CODE_E, // SDLK_e			= 101,
        KEY_SCAN_CODE_F, // SDLK_f			= 102,
        KEY_SCAN_CODE_G, // SDLK_g			= 103,
        KEY_SCAN_CODE_H, // SDLK_h			= 104,
        KEY_SCAN_CODE_I, // SDLK_i			= 105,
        KEY_SCAN_CODE_J, // SDLK_j			= 106,
        KEY_SCAN_CODE_K, // SDLK_k			= 107,
        KEY_SCAN_CODE_L, // SDLK_l			= 108,
        KEY_SCAN_CODE_M, // SDLK_m			= 109,
        KEY_SCAN_CODE_N, // SDLK_n			= 110,
        KEY_SCAN_CODE_O, // SDLK_o			= 111,
        KEY_SCAN_CODE_P, // SDLK_p			= 112,
        KEY_SCAN_CODE_Q, // SDLK_q			= 113,
        KEY_SCAN_CODE_R, // SDLK_r			= 114,
        KEY_SCAN_CODE_S, // SDLK_s			= 115,
        KEY_SCAN_CODE_T, // SDLK_t			= 116,
        KEY_SCAN_CODE_U, // SDLK_u			= 117,
        KEY_SCAN_CODE_V, // SDLK_v			= 118,
        KEY_SCAN_CODE_W, // SDLK_w			= 119,
        KEY_SCAN_CODE_X, // SDLK_x			= 120,
        KEY_SCAN_CODE_Y, // SDLK_y			= 121,
        KEY_SCAN_CODE_Z, // SDLK_z			= 122,
        0, // 123
        0, // 124
        0, // 125
        0, // 126
        KEY_SCAN_CODE_DELETE, // SDLK_DELETE		= 127,
        0, // 128
        0, // 129
        0, // 130
        0, // 131
        0, // 132
        0, // 133
        0, // 134
        0, // 135
        0, // 136
        0, // 137
        0, // 138
        0, // 139
        0, // 140
        0, // 141
        0, // 142
        0, // 143
        0, // 144
        0, // 145
        0, // 146
        0, // 147
        0, // 148
        0, // 149
        0, // 150
        0, // 151
        0, // 152
        0, // 153
        0, // 154
        0, // 155
        0, // 156
        0, // 157
        0, // 158
        0, // 159
	/* End of ASCII mapped keysyms */
        /*@}*/

	/** @name International keyboard syms */
        /*@{*/
	0, // SDLK_WORLD_0		= 160,		/* 0xA0 */
	0, // SDLK_WORLD_1		= 161,
	0, // SDLK_WORLD_2		= 162,
	0, // SDLK_WORLD_3		= 163,
	0, // SDLK_WORLD_4		= 164,
	0, // SDLK_WORLD_5		= 165,
	0, // SDLK_WORLD_6		= 166,
	0, // SDLK_WORLD_7		= 167,
	0, // SDLK_WORLD_8		= 168,
	0, // SDLK_WORLD_9		= 169,
	0, // SDLK_WORLD_10		= 170,
	0, // SDLK_WORLD_11		= 171,
	0, // SDLK_WORLD_12		= 172,
	0, // SDLK_WORLD_13		= 173,
	0, // SDLK_WORLD_14		= 174,
	0, // SDLK_WORLD_15		= 175,
	0, // SDLK_WORLD_16		= 176,
	0, // SDLK_WORLD_17		= 177,
	0, // SDLK_WORLD_18		= 178,
	0, // SDLK_WORLD_19		= 179,
	0, // SDLK_WORLD_20		= 180,
	0, // SDLK_WORLD_21		= 181,
	0, // SDLK_WORLD_22		= 182,
	0, // SDLK_WORLD_23		= 183,
	0, // SDLK_WORLD_24		= 184,
	0, // SDLK_WORLD_25		= 185,
	0, // SDLK_WORLD_26		= 186,
	0, // SDLK_WORLD_27		= 187,
	0, // SDLK_WORLD_28		= 188,
	0, // SDLK_WORLD_29		= 189,
	0, // SDLK_WORLD_30		= 190,
	0, // SDLK_WORLD_31		= 191,
	0, // SDLK_WORLD_32		= 192,
	0, // SDLK_WORLD_33		= 193,
	0, // SDLK_WORLD_34		= 194,
	0, // SDLK_WORLD_35		= 195,
	0, // SDLK_WORLD_36		= 196,
	0, // SDLK_WORLD_37		= 197,
	0, // SDLK_WORLD_38		= 198,
	0, // SDLK_WORLD_39		= 199,
	0, // SDLK_WORLD_40		= 200,
	0, // SDLK_WORLD_41		= 201,
	0, // SDLK_WORLD_42		= 202,
	0, // SDLK_WORLD_43		= 203,
	0, // SDLK_WORLD_44		= 204,
	0, // SDLK_WORLD_45		= 205,
	0, // SDLK_WORLD_46		= 206,
	0, // SDLK_WORLD_47		= 207,
	0, // SDLK_WORLD_48		= 208,
	0, // SDLK_WORLD_49		= 209,
	0, // SDLK_WORLD_50		= 210,
	0, // SDLK_WORLD_51		= 211,
	0, // SDLK_WORLD_52		= 212,
	0, // SDLK_WORLD_53		= 213,
	0, // SDLK_WORLD_54		= 214,
	0, // SDLK_WORLD_55		= 215,
	0, // SDLK_WORLD_56		= 216,
	0, // SDLK_WORLD_57		= 217,
	0, // SDLK_WORLD_58		= 218,
	0, // SDLK_WORLD_59		= 219,
	0, // SDLK_WORLD_60		= 220,
	0, // SDLK_WORLD_61		= 221,
	0, // SDLK_WORLD_62		= 222,
	0, // SDLK_WORLD_63		= 223,
	0, // SDLK_WORLD_64		= 224,
	0, // SDLK_WORLD_65		= 225,
	0, // SDLK_WORLD_66		= 226,
	0, // SDLK_WORLD_67		= 227,
	0, // SDLK_WORLD_68		= 228,
	0, // SDLK_WORLD_69		= 229,
	0, // SDLK_WORLD_70		= 230,
	0, // SDLK_WORLD_71		= 231,
	0, // SDLK_WORLD_72		= 232,
	0, // SDLK_WORLD_73		= 233,
	0, // SDLK_WORLD_74		= 234,
	0, // SDLK_WORLD_75		= 235,
	0, // SDLK_WORLD_76		= 236,
	0, // SDLK_WORLD_77		= 237,
	0, // SDLK_WORLD_78		= 238,
	0, // SDLK_WORLD_79		= 239,
	0, // SDLK_WORLD_80		= 240,
	0, // SDLK_WORLD_81		= 241,
	0, // SDLK_WORLD_82		= 242,
	0, // SDLK_WORLD_83		= 243,
	0, // SDLK_WORLD_84		= 244,
	0, // SDLK_WORLD_85		= 245,
	0, // SDLK_WORLD_86		= 246,
	0, // SDLK_WORLD_87		= 247,
	0, // SDLK_WORLD_88		= 248,
	0, // SDLK_WORLD_89		= 249,
	0, // SDLK_WORLD_90		= 250,
	0, // SDLK_WORLD_91		= 251,
	0, // SDLK_WORLD_92		= 252,
	0, // SDLK_WORLD_93		= 253,
	0, // SDLK_WORLD_94		= 254,
	0, // SDLK_WORLD_95		= 255,		/* 0xFF */
        /*@}*/

	/** @name Numeric keypad */
        /*@{*/
	KEY_SCAN_CODE_KEYPAD_0, // SDLK_KP0		= 256,
	KEY_SCAN_CODE_KEYPAD_1, // SDLK_KP1		= 257,
	KEY_SCAN_CODE_KEYPAD_2, // SDLK_KP2		= 258,
	KEY_SCAN_CODE_KEYPAD_3, // SDLK_KP3		= 259,
	KEY_SCAN_CODE_KEYPAD_4, // SDLK_KP4		= 260,
	KEY_SCAN_CODE_KEYPAD_5, // SDLK_KP5		= 261,
	KEY_SCAN_CODE_KEYPAD_6, // SDLK_KP6		= 262,
	KEY_SCAN_CODE_KEYPAD_7, // SDLK_KP7		= 263,
	KEY_SCAN_CODE_KEYPAD_8, // SDLK_KP8		= 264,
	KEY_SCAN_CODE_KEYPAD_9, // SDLK_KP9		= 265,
	KEY_SCAN_CODE_KEYPAD_PERIOD, // SDLK_KP_PERIOD		= 266,
	KEY_SCAN_CODE_KEYPAD_SLASH, // SDLK_KP_DIVIDE		= 267,
	KEY_SCAN_CODE_KEYPAD_STAR, // SDLK_KP_MULTIPLY	= 268,
	KEY_SCAN_CODE_KEYPAD_MINUS, // SDLK_KP_MINUS		= 269,
	KEY_SCAN_CODE_KEYPAD_PLUS, // SDLK_KP_PLUS		= 270,
	KEY_SCAN_CODE_KEYPAD_ENTER, // SDLK_KP_ENTER		= 271,
	0, // SDLK_KP_EQUALS		= 272,
        /*@}*/

	/** @name Arrows + Home/End pad */
        /*@{*/
	KEY_SCAN_CODE_UP, // SDLK_UP			= 273,
	KEY_SCAN_CODE_DOWN, // SDLK_DOWN		= 274,
	KEY_SCAN_CODE_RIGHT, // SDLK_RIGHT		= 275,
	KEY_SCAN_CODE_LEFT, // SDLK_LEFT		= 276,
	KEY_SCAN_CODE_INSERT, // SDLK_INSERT		= 277,
	KEY_SCAN_CODE_HOME, // SDLK_HOME		= 278,
	KEY_SCAN_CODE_END, // SDLK_END		= 279,
	KEY_SCAN_CODE_PGUP, // SDLK_PAGEUP		= 280,
	KEY_SCAN_CODE_PGDN, // SDLK_PAGEDOWN		= 281,
        /*@}*/

	/** @name Function keys */
        /*@{*/
	KEY_SCAN_CODE_F1, // SDLK_F1			= 282,
	KEY_SCAN_CODE_F2, // SDLK_F2			= 283,
	KEY_SCAN_CODE_F3, // SDLK_F3			= 284,
	KEY_SCAN_CODE_F4, // SDLK_F4			= 285,
	KEY_SCAN_CODE_F5, // SDLK_F5			= 286,
	KEY_SCAN_CODE_F6, // SDLK_F6			= 287,
	KEY_SCAN_CODE_F7, // SDLK_F7			= 288,
	KEY_SCAN_CODE_F8, // SDLK_F8			= 289,
	KEY_SCAN_CODE_F9, // SDLK_F9			= 290,
	KEY_SCAN_CODE_F10, // SDLK_F10		= 291,
	KEY_SCAN_CODE_F11, // SDLK_F11		= 292,
	KEY_SCAN_CODE_F12, // SDLK_F12		= 293,
	0, // SDLK_F13		= 294,
	0, // SDLK_F14		= 295,
	0, // SDLK_F15		= 296,
	0, // 297
	0, // 298
	0, // 299
        /*@}*/

	/** @name Key state modifier keys */
        /*@{*/
	KEY_SCAN_CODE_NUM_LOCK, // SDLK_NUMLOCK		= 300,
	KEY_SCAN_CODE_CAPS_LOCK, // SDLK_CAPSLOCK		= 301,
	KEY_SCAN_CODE_SCROLL_LOCK, // SDLK_SCROLLOCK		= 302,
	KEY_SCAN_CODE_RIGHT_SHIFT, // SDLK_RSHIFT		= 303,
	KEY_SCAN_CODE_LEFT_SHIFT, // SDLK_LSHIFT		= 304,
	KEY_SCAN_CODE_RIGHT_CTRL, // SDLK_RCTRL		= 305,
	KEY_SCAN_CODE_LEFT_CTRL, // SDLK_LCTRL		= 306,
	KEY_SCAN_CODE_ALT, // SDLK_RALT		= 307,
	KEY_SCAN_CODE_ALT, // SDLK_LALT		= 308,
	0, // SDLK_RMETA		= 309,
	0, // SDLK_LMETA		= 310,
	0, // SDLK_LSUPER		= 311,		/**< Left "Windows" key */
	0, // SDLK_RSUPER		= 312,		/**< Right "Windows" key */
	0, // SDLK_MODE		= 313,		/**< "Alt Gr" key */
	0, // SDLK_COMPOSE		= 314,		/**< Multi-key compose key */
        /*@}*/

	/** @name Miscellaneous function keys */
        /*@{*/
	0, // SDLK_HELP		= 315,
	0, // SDLK_PRINT		= 316,
	0, // SDLK_SYSREQ		= 317,
	0, // SDLK_BREAK		= 318,
	0, // SDLK_MENU		= 319,
	0, // SDLK_POWER		= 320,		/**< Power Macintosh power key */
	0, // SDLK_EURO		= 321,		/**< Some european keyboards */
	0, // SDLK_UNDO		= 322,		/**< Atari keyboard has Undo */

};

#include <direct.h>
#define KEY_IS_DOWN 0x80
#define KEY_IS_CHANGED 0x01
static T_byte8 G_lastKeyState[SDLK_LAST] ;
T_void KeyboardUpdate(E_Boolean updateBuffers)
{
    //T_byte8 keys[256] ;
    T_byte8 *keys;
    T_word32 time ;
    T_word16 scanCode ;
    T_byte8 c ;
    T_word16 next ;
    T_word16 i ;
    E_Boolean changed ;
    E_Boolean newValue ;

    DebugRoutine("KeyboardUpdate");
    DebugCheck(sizeof(G_sdlToScancode)==SDLK_LAST);
    time = TickerGet() ;
    //GetKeyboardState(keys) ;
    keys = SDL_GetKeyState(NULL);

    /* Only care about up/down status */
//    for (scanCode=0; scanCode<256; scanCode++)
//        keys[scanCode] &= KEY_IS_DOWN ;

    if (updateBuffers)  {
        for (i=1; i<SDLK_LAST; i++)  {
            // Skip keys we don't know how to process
            if (G_sdlToScancode[i] == 0)
                continue;

            changed = (keys[i] != G_lastKeyState[i])?TRUE:FALSE ;

            //scanCode = MapVirtualKey(i, 0) ; // old windows version
            scanCode = G_sdlToScancode[i];

            /* Record the state of the keypress */
            newValue = (keys[i])?TRUE:FALSE ;
            changed = (newValue != G_keyTable[scanCode])?TRUE:FALSE ;

            /* Find keys that have changed */
            if (changed)  {
printf("scancode %d = %d\n", scanCode, newValue);
                G_keyTable[scanCode] = newValue ;
                /* Store the key in the scan key buffer */
                G_scanKeyBuffer[G_scanKeyEnd] = scanCode | ((G_keyTable[scanCode])?0:0x100) ;
                next = (G_scanKeyEnd+1) & (MAX_SCAN_KEY_BUFFER-1) ;
                if (next != G_scanKeyStart)
                    G_scanKeyEnd = next ;

                /* If a press and an ascii character, store that in the */
                /* keyboard buffer. */
                if ((G_keyTable[scanCode] == TRUE) &&
                    (IGetAdjustedKey(KEY_SCAN_CODE_ALT) == FALSE) &&
                    (G_keyTable[KEY_SCAN_CODE_RIGHT_CTRL]==FALSE) &&
                    (G_keyTable[KEY_SCAN_CODE_LEFT_CTRL]==FALSE))  {
                    c = scanCode & 0xFF;

                    /* Translate for the shift key */
                    if (G_keyTable[KEY_SCAN_CODE_RIGHT_SHIFT] |
                          G_keyTable[KEY_SCAN_CODE_LEFT_SHIFT])
                        c |= 0x80 ;

                    /* What ascii character is this? */
                    c = G_keyboardToASCII[c] ;

                    /* Where are we going to roll over to next */
                    next = (G_asciiEnd+1)&31 ;

                    // Must be non-zero
                    if (c) {
                        /* If back at start, don't do */
                        if (next != G_asciiStart)  {
printf("  buffer key %d, %d (%c), i=%d\n", scanCode, c, c, i);
                            /* Store in the buffer */
                            G_asciiBuffer[G_asciiEnd] = c ;
                            G_asciiEnd = next ;
                        }
                    }
                }

                /* Check to see if keystroke changed from pressed to released or */
                /* visa-versa.  If it has, update the key count appropriately. */
                if (G_keyTable[scanCode] == TRUE)  {
                    G_keysPressed++ ;
                    /* Note the time that key is pressed. */
                    G_keyPressTime[scanCode] = time ;
                } else if (G_keyTable[scanCode] == FALSE)  {
                    G_keysPressed-- ;
                    /* Note how long the key has been held down. */
                    G_keyHoldTime[scanCode] += time - G_keyPressTime[scanCode] ;
                }
            }
        }
    }
    memcpy(G_lastKeyState, keys, sizeof(G_lastKeyState)) ;
    DebugEnd();
}
#endif


/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  KEYBOARD.C
 *-------------------------------------------------------------------------*/
