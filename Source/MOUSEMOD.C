/****************************************************************************/
/*    FILE:  MOUSEMOD.C                                                     */
/****************************************************************************/
#include "DBLLINK.H"
#include "GENERAL.H"
#include "MOUSEMOD.H"
#include "PICS.H"

#ifdef WIN32
#include "direct.h"
#endif

/* Flag that determines if the mouse module has been initialized. */
static T_byte8 F_MouseIsInitialized = FALSE ;

/* Keep track of how hidden/shown the mouse is. */
/*      > 0   -- shown     */
/*     <= 0   -- hidden    */
static T_sword16 G_mouseShowLevel = 0 ;

/* Note what events we will pass on: */
static T_byte8 F_mouseEventsHandled = MOUSE_EVENT_DEFAULT_OPTIONS ;

/* Keep the event handler:  (default = NONE) */
static T_mouseEventHandler P_mouseEventHandler = NULL ;

/* Note if events are being blocked */
static E_Boolean F_mouseBlockEvents = FALSE ;

#define MOUSE_STATE_IDLE   0
#define MOUSE_STATE_HELD   1

/* Keep track of what state the mouse is in. */
static T_word16 G_mouseState = MOUSE_STATE_IDLE ;
static T_word16 G_mouseLastX = 0 ;
static T_word16 G_mouseLastY = 0 ;

/* Where to center the mouse. */
static T_word16 G_hotSpotX = 0 ;
static T_word16 G_hotSpotY = 0 ;

/* COMPRESSED bitmap to use. */
static T_bitmap *G_bitmap = NULL ;

static T_word16 G_lastDrawX = 0 ;
static T_word16 G_lastDrawY = 0 ;

static T_word16 G_defaultHotSpotX = 0 ;
static T_word16 G_defaultHotSpotY = 0 ;
static T_bitmap *G_defaultBitmap = NULL ;

/* Flag to tell if the mouse callback routine has been installed. */
static E_Boolean G_callbackInstalled = FALSE ;

/* Flag to tell if atexit has been installed to make sure the */
/* mouse callback is turned off. */
static E_Boolean G_atexitInstalled = FALSE ;

#define MOUSE_BUTTON_LEFT   0x01       /* --------1 */
#define MOUSE_BUTTON_RIGHT  0x02       /* -------1- */
#define MOUSE_BUTTON_MIDDLE 0x04       /* ------1-- */

/* Internal prototypes: */

static T_buttonClick IMouseGetButtonStatus(T_void) ;

static T_void IMouseGetMousePosition(T_word16 *xPos, T_word16 *yPos) ;

static T_screen G_mouseScreen ;

T_void IMouseTransfer(T_word16 x, T_word16 y, T_screen from, T_screen to) ;

T_void IMouseGetUpperLeft(T_word16 *p_x, T_word16 *p_y) ;

T_void IMouseInstallCallback(T_void) ;

T_void IMouseUninstallCallback(T_void) ;

//static T_void _interrupt _loadds far IMouseCallback(T_word32 max, T_word32 mcx, T_word32 mdx) ;
#ifdef WATCOM
#pragma aux IMouseCallback parm [EAX] [ECX] [EDX]
static T_void _loadds __far IMouseCallback(T_word32 max, T_word32 mcx, T_word32 mdx) ;
#endif

static T_sword16 G_interX = 0 ;
static T_sword16 G_interY = 0 ;

static E_colorizeTable G_colorTable = COLORIZE_TABLE_NONE ;

static E_Boolean G_allowUpdate = TRUE ;

static T_doubleLinkList G_eventStack = DOUBLE_LINK_LIST_BAD ;

/* Relative mode registers */
static E_Boolean G_relativeMode = FALSE;
static T_word16 G_mousePreRelativeX;
static T_word16 G_mousePreRelativeY;
static T_word16 G_relativeSensitivity = 70;

/****************************************************************************/
/*  Routine:  MouseInitialize                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseInitialize sets up the mouse for all the different information   */
/*  it needs to hold.  It starts up with all default options and the cursor */
/*  starts up hidden.                                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You can only call this routine once.  Call MouseFinish first          */
/*  if you need to re-initialize the mouse (but you shouldn't).             */
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
/*    MouseHide                                                             */
/*    MouseAllowEvents                                                      */
/*    MouseReleaseBounds                                                    */
/*    MouseSetEventOptions                                                  */
/*    MouseMoveTo                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*    LES  02/28/96  Added construction of G_eventStack                     */
/*                                                                          */
/****************************************************************************/

T_void MouseInitialize(T_void)
{
    DebugRoutine("MouseInitialize") ;
    DebugCheck(MouseCheckInstalled() == TRUE) ;
    DebugCheck(F_MouseIsInitialized == FALSE) ;

    F_MouseIsInitialized = TRUE ;

    /* Note that the mouse module has been initialized. */
    G_mouseShowLevel = 0 ;

    /* Set the default options. */
    MouseSetEventOptions(MOUSE_EVENT_DEFAULT_OPTIONS) ;

    /* Allow the mouse to move all over the screen. */
    MouseReleaseBounds() ;

    /* Note that the current mouse state is idle. */
    G_mouseState = MOUSE_STATE_IDLE ;

    /* Put the mouse in the upper left hand corner. */
    MouseMoveTo(0, 0) ;

    /* Allocate a place to put stuff behind the mouse. */
    G_mouseScreen = GrScreenAlloc() ;

#ifdef DOS32
    IMouseInstallCallback() ;
#endif

    G_eventStack = DoubleLinkListCreate() ;
    DebugCheck(G_eventStack != DOUBLE_LINK_LIST_BAD) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseSetEventHandler                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    In order to use the mouse for anything, you MUST have an event        */
/*  handler.  This handler will receive events to process each time         */
/*  MouseUpdateEvents() is called (but not until then).                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    There is no way to tell if you are giving this routine a correct      */
/*  mouse event handler, so it is up to you to make sure you are doing      */
/*  it right.                                                               */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_mouseEventHandler p_handler  -- Pointer to function that will       */
/*                                      handle mouse events.  Pass NULL     */
/*                                      if there is no longer an event      */
/*                                      handler.                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetEventHandler(T_mouseEventHandler p_handler)
{
    DebugRoutine("MouseSetEventHandler") ;

    /* Just copy it over.  Since NULL is also allowed, there is no */
    /* type checking or anything that we can do to make sure the */
    /* handler is good.  Hopefully the compiler will be enough. */
    P_mouseEventHandler = p_handler ;

    DebugEnd() ;
}

T_mouseEventHandler MouseGetEventHandler (T_void)
{
    return (P_mouseEventHandler);
}

/****************************************************************************/
/*  Routine:  MouseSetEventOptions                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Several different events can be enabled and disabled.  To do so,      */
/*  use this MouseSetEventOptions to tell what events you want the current  */
/*  mouse event handler to handle.                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 f_options          -- Or'd list of MOUSE_EVENT_OPTION's      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetEventOptions(T_byte8 f_options)
{
    DebugRoutine("MouseSetEventOptions") ;
    DebugCheck((f_options & MOUSE_EVENT_OPTION_UNKNOWN) == 0) ;

    /* Just record the flags. */
    F_mouseEventsHandled = f_options ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseCheckInstalled                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseCheckInstalled checks to see if a mouse is available.  If there  */
/*  is one, TRUE is returned.  If not, a FALSE is returned.                 */
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
/*    E_Boolean                   -- TRUE = Mouse driver installed          */
/*                                   FALSE= No mouse driver found           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset                                                                */
/*    intr                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean MouseCheckInstalled(T_void)
{
#ifdef DOS32
    E_Boolean f_installed ;
    union REGPACK regs;

    DebugRoutine("MouseCheckInstalled") ;

    memset(&regs,0,sizeof(union REGPACK));
    intr(0x33,&regs);
    if (regs.w.ax == ((T_word16)-1))
        f_installed = TRUE ;
    else
        f_installed = FALSE ;

    DebugCheck(f_installed < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return(f_installed);
#else
    return TRUE ;
#endif
}

/****************************************************************************/
/*  Routine:  MouseShow                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseShow turns on the mouse cursor.  If the mouse was already on,    */
/*  it increments the level of showing.  Therefore, it will take an         */
/*  equal number of MouseHide calls to turn the mouse off.                  */
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
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseShow(T_void)
{
#if 0
    union REGPACK regs;

    DebugRoutine("MouseShow") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    if (G_mouseShowLevel == 0)  {
        /* If the mouse was just hidden (and soon won't be), */
        /* turn on the mouse. */
/*
        memset(&regs,0,sizeof(union REGPACK));
        regs.w.ax = 1;
        intr(0x33,&regs);
*/
        MouseDraw() ;
    }

    /* Increment the hide level */
    G_mouseShowLevel++ ;

    /* Limit to 10 shows */
    DebugCheck(G_mouseShowLevel < 10) ;
    DebugEnd() ;
#endif
}

/****************************************************************************/
/*  Routine:  MouseHide                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseHide turns off the mouse and can be called multiple times.       */
/*  Multiple hides require an equal number of calls to MouseShow to make    */
/*  the mouse reappear.                                                     */
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
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseHide(T_void)
{
#if 0
    union REGPACK regs;

    DebugRoutine("MouseHide") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    if (G_mouseShowLevel == 1)  {
        /* If the mouse was just hidden (and soon won't be), */
        /* turn on the mouse. */
/*
        memset(&regs,0,sizeof(union REGPACK));
        regs.w.ax = 2;
        intr(0x33,&regs);
*/
        MouseErase() ;
    }

    /* Decrement the hide level */
    G_mouseShowLevel-- ;

    /* Limit to 10 shows */
    DebugCheck(G_mouseShowLevel < 10) ;
    DebugEnd() ;
#endif
}

/****************************************************************************/
/*  Routine:  MouseMoveTo                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    To move the mouse to a different location, pass an X & Y position     */
/*  to MouseMoveTo().  The mouse will then be at that new location.         */
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
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseMoveTo(T_word16 x, T_word16 y)
{
#ifdef DOS32
    union REGPACK regs;

    DebugRoutine("MouseMoveTo") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(x < 320) ;
    DebugCheck(y < 200) ;

    /* Move the mouse. */
    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 4;
    regs.w.dx = y;
    regs.w.cx = x<<1;
    intr(0x33,&regs);

    /* Make this position the position we've always been in. */
    G_mouseLastX = x ;
    G_mouseLastY = y ;

    DebugEnd() ;
#else
    extern void OutsideMouseDriverSet(T_word16 xPos, T_word16 yPos);
    OutsideMouseDriverSet(x, y);
#endif
}

/****************************************************************************/
/*  Routine:  MouseSetBounds                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseSetBounds declares a box area in which the mouse is not allowed  */
/*  to move out of.                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    I don't know what happens when the mouse is outside and the box is    */
/*  then declared.  You should probably do a MouseMoveTo after setting      */
/*  the bounds.                                                             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left               -- Left side of box                       */
/*                                                                          */
/*    T_word16 top                -- top edge of box                        */
/*                                                                          */
/*    T_word16 right              -- Right side of box                      */
/*                                                                          */
/*    T_word16 bottom             -- Bottom edge of box                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetBounds(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom)
{
#ifdef DOS32
    union REGPACK regs;

    DebugRoutine("MouseSetBounds") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(right < 320) ;
    DebugCheck(left <= right) ;
    DebugCheck(bottom < 200) ;
    DebugCheck(top <= bottom ) ;

    /* Declare the left and right edge. */
    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 7;
    regs.w.cx = left<<1 ;
    regs.w.dx = right<<1 ;
    intr(0x33,&regs);

    /* Declare the top and bottom edge. */
    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 8;
    regs.w.cx = top ;
    regs.w.dx = bottom ;
    intr(0x33,&regs);

    DebugEnd() ;
#endif
}

/****************************************************************************/
/*  Routine:  MouseSetPicture                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseSetPicture is used to change the look of the mouse cursor        */
/*  and where it's hot spot (pointing spot) is located.  Just pass to       */
/*  it a mouse shape pointer and the position of the hot spot.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.  Just make sure the hot spot is within the cursor.              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 hot_spot_x         -- left to right position of hot spot     */
/*                                                                          */
/*    T_word16 hot_spot_y         -- top to bottom position of hot spot     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetPicture(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_mousePicture picture)
{
#ifdef DOS32
//    union REGPACK regs;
    struct SREGS sregs ;
    union REGS inregs ;

    DebugRoutine("MouseSetPicture") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(hot_spot_x < 16) ;
    DebugCheck(hot_spot_y < 16) ;
    DebugCheck(picture != NULL) ;

/*
    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 9;
    regs.w.dx = FP_OFF(picture) ;
    regs.w.es = FP_SEG(picture) ;
    regs.w.bx = hot_spot_x ;
    regs.w.cx = hot_spot_y ;
    intr(0x33,&regs);
*/
    segread(&sregs) ;

    inregs.w.ax = 0x09 ;
    inregs.w.bx = hot_spot_x ;
    inregs.w.cx = hot_spot_y ;
    sregs.es = FP_SEG(picture) ;
#ifdef __386__
    inregs.x.edx = FP_OFF(picture) ;
    int386x(0x33, &inregs, &inregs, &sregs) ;
#else
    inregs.x.dx = FP_OFF(picture) ;
    int86x(0x33, &inregs, &inregs, &sregs) ;
#endif

    DebugEnd() ;
#endif
}

/****************************************************************************/
/*  Routine:  MouseBlockEvents                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseBlockEvents keeps the mouse module from sending out events.      */
/*  Use MouseAllowEvents to re-enable blocked events.                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You can only call this routine once before having to call             */
/*  MouseAllowEvents.                                                       */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseBlockEvents(T_void)
{
    DebugRoutine("MouseBlockEvents") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(F_mouseBlockEvents == FALSE) ;

    /* Turn on block. */
    F_mouseBlockEvents = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseAllowEvents                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Use MouseAllowEvents to allow events to occur by the mouse module.    */
/*  Usually MouseBlockEvents will have been called.                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You can only call this routine after MouseBlockEvents was called.     */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseAllowEvents(T_void)
{
    DebugRoutine("MouseAllowEvents") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(F_mouseBlockEvents == TRUE) ;

    /* Turn off block. */
    F_mouseBlockEvents = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseUpdateEvents                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Whenever you wish the mouse to be updated and events to be sent       */
/*  to the event handler (if any), call this routine.                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine must be called several times a second to be really       */
/*  efficient.  I could just tie this routine in with the mouse driver,     */
/*  but I figured it prudent to have an upper level that controls the       */
/*  mouse updates.                                                          */
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
/*    P_mouseEventHandler                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseUpdateEvents(T_void)
{
    T_word16 newX ;
    T_word16 newY ;
    T_buttonClick buttonStatus ;

    DebugRoutine("MouseUpdateEvents") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    if (!MouseIsRelativeMode()) {
        /* No matter what we do, we might as well get the button status */
        /* and the mouse x & y locations. */
        buttonStatus = IMouseGetButtonStatus() ;
        IMouseGetMousePosition(&newX, &newY) ;

        /* Let the current state of the mouse determine our actions */
        switch(G_mouseState)  {
            case MOUSE_STATE_IDLE:
                /* Check to see if we have moved. */
                if ((newX != G_mouseLastX) || (newY != G_mouseLastY))  {
                    /* We have.  Send mouse move event if there is a mouse */
                    /* handler and events are not blocked and if the option */
                    /* for receiving move events are there. */
                    if (F_mouseBlockEvents == FALSE)
                        if (F_mouseEventsHandled &&
                                (MOUSE_EVENT_OPTION_HANDLE_MOVE != 0))
                            P_mouseEventHandler(
                                MOUSE_EVENT_MOVE,
                                newX,
                                newY,
                                buttonStatus) ;
                } else if (buttonStatus != 0)  {
                    /* Check to see if the button is pressed */
                    /* We are changing state, send out a mouse start event. */
                    /* But make sure it is not being blocked. */
                    if (P_mouseEventHandler != NULL)
                        if (F_mouseBlockEvents == FALSE)
                            if (F_mouseEventsHandled &&
                                    (MOUSE_EVENT_OPTION_HANDLE_START != 0))
                                P_mouseEventHandler(
                                    MOUSE_EVENT_START,
                                    newX,
                                    newY,
                                    buttonStatus) ;

                    /* Go to the button held state. */
                    G_mouseState = MOUSE_STATE_HELD ;
                } else {
                    /* None of the above, try sending a MOUSE_EVENT_IDLE. */
                    if (F_mouseBlockEvents == FALSE)
                        if (F_mouseEventsHandled &&
                                (MOUSE_EVENT_OPTION_HANDLE_IDLE != 0))
                            P_mouseEventHandler(
                                MOUSE_EVENT_IDLE,
                                newX,
                                newY,
                                buttonStatus) ;
                }
                break ;
            case MOUSE_STATE_HELD:
                if (buttonStatus == 0)  {
                    /* Check to see if the button is pressed */
                    /* We are changing state, send out a mouse end event. */
                    /* But make sure it is not being blocked. */
                    if (P_mouseEventHandler != NULL)
                        if (F_mouseBlockEvents == FALSE)
                            if (F_mouseEventsHandled &&
                                    (MOUSE_EVENT_OPTION_HANDLE_END != 0))
                                P_mouseEventHandler(
                                    MOUSE_EVENT_END,
                                    newX,
                                    newY,
                                    buttonStatus) ;

                    /* Go to the mouse idle state. */
                    G_mouseState = MOUSE_STATE_IDLE ;
                } else if ((newX != G_mouseLastX) || (newY != G_mouseLastY))  {
                    /* Check to see if we have moved. */
                    /* We have.  Send mouse move event if there is a mouse */
                    /* handler and events are not blocked and if the option */
                    /* for receiving drag events are there. */
                    if (F_mouseBlockEvents == FALSE)
                        if (F_mouseEventsHandled &&
                                (MOUSE_EVENT_OPTION_HANDLE_DRAG != 0))
                            P_mouseEventHandler(
                                MOUSE_EVENT_DRAG,
                                newX,
                                newY,
                                buttonStatus) ;
                } else {
                    /* None of the above, try sending a MOUSE_EVENT_HELD. */
                    if (F_mouseBlockEvents == FALSE)
                        if (F_mouseEventsHandled &&
                                (MOUSE_EVENT_OPTION_HANDLE_HELD != 0))
                            P_mouseEventHandler(
                                MOUSE_EVENT_HELD,
                                newX,
                                newY,
                                buttonStatus) ;
                }
                break ;
        } ;

        G_mouseLastX = newX ;
        G_mouseLastY = newY ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseFinish                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseFinish is called when you are done with the mouse module         */
/*  and no longer need it.  All hooks and variables are changed to non-     */
/*  active mode and you can consider it no longer being used.               */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*    LES  02/28/96  Added reconstruction of G_eventStack                   */
/*                                                                          */
/****************************************************************************/

T_void MouseFinish(T_void)
{
    DebugRoutine("MouseFinish") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;
    DebugCheck(G_mouseShowLevel < 2) ;

    DebugCheck(G_eventStack != DOUBLE_LINK_LIST_BAD) ;
    DoubleLinkListDestroy(G_eventStack) ;

    /* Hide the mouse. */
    MouseHide() ;

    /* Turn off the event handler. */
    MouseSetEventHandler(NULL) ;

    /* Set the default options. */
    MouseSetEventOptions(MOUSE_EVENT_DEFAULT_OPTIONS) ;

    /* Allow the mouse to move all over the screen. */
    MouseReleaseBounds() ;

    /* Note that the mouse module is no longer in use. */
    F_MouseIsInitialized = FALSE ;

    /* Free up the mouse background screen. */
    GrScreenFree(G_mouseScreen) ;
#ifdef DOS32
    IMouseUninstallCallback() ;
#endif
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseReleaseBounds                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseReleaseBounds lets the mouse now roam the whole screen.          */
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
/*    MouseSetBounds                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseReleaseBounds(T_void)
{
    DebugRoutine("MouseReleaseBounds") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    /* Set the bounds to the size of the screen. */
    MouseSetBounds(0, 0, 319, 199) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IMouseGetButtonStatus              * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IMouseGetButtonStatus checks to see if any of the mouse buttons       */
/*  are being held and returns the status of those buttons.                 */
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
/*    T_word16                    -- the bit combination of the mouse       */
/*                                   mouse buttons.                         */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_buttonClick IMouseGetButtonStatus(T_void)
{
#if DOS32
    union REGPACK regs;
    T_word16 buttonStatus ;

    DebugRoutine("IMouseGetButtonStatus") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    /* Call the int 0x33 for the button status of the mouse. */
    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 3;
    intr(0x33,&regs);
    buttonStatus = regs.w.bx;

    DebugEnd() ;
    return buttonStatus ;
#else
	extern T_buttonClick DirectMouseGetButton(T_void) ;

    return DirectMouseGetButton() ;
#endif
}

/****************************************************************************/
/*  Routine:  IMouseGetMousePosition             * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IMouseGetMousePosition uses the passed in pointers to return the      */
/*  x (column) and y (row) coordinate of where the mouse's hotspot is       */
/*  located.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 *xPos              -- Pointer to x position to store into    */
/*                                                                          */
/*    T_word16 *yPos              -- Pointer to y position to store into    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset, intr                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/18/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IMouseGetMousePosition(T_word16 *xPos, T_word16 *yPos)
{
#ifdef DOS32
    union REGPACK regs;

    DebugRoutine("IMouseGetMousePosition") ;
    DebugCheck(F_MouseIsInitialized == TRUE) ;

    /* Get the x & y position */

    memset(&regs,0,sizeof(union REGPACK));
    regs.w.ax = 3;
    intr(0x33,&regs);
    *xPos = regs.w.cx>>1 ;
    *yPos = regs.w.dx ;
/*
    *xPos = G_interX ;
    *yPos = G_interY ;
*/
    DebugEnd() ;
#else
    extern void OutsideMouseDriverGet(T_word16 *xPos, T_word16 *yPos);
    OutsideMouseDriverGet(xPos, yPos);
#endif
}

/****************************************************************************/
/*  Routine:  MouseSetBitmap                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseSetBitmap changes the picture used for the mouse and its hotspot.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 hot_spot_x, y      -- x, y position for hot spot             */
/*                                                                          */
/*    T_bitmap *p_bitmap          -- COMPRESSED Bitmap to use               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetBitmap(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_bitmap *p_bitmap)
{
    DebugRoutine("MouseSetBitmap") ;
//    DebugCheck(p_bitmap != NULL) ;

    G_hotSpotX = hot_spot_x ;
    G_hotSpotY = hot_spot_y ;

//    if (G_mouseShowLevel > 0)
//        MouseErase() ;

    MouseHide();
    G_bitmap = p_bitmap ;
    MouseShow() ;
//    if (G_mouseShowLevel > 0)
//        MouseDraw() ;

    DebugEnd() ;
}

T_void MouseGetBitmap(
          T_word16 *hot_spot_x,
          T_word16 *hot_spot_y,
          T_bitmap **p_bitmap)
{
    *hot_spot_x = G_hotSpotX ;
    *hot_spot_y = G_hotSpotY ;
    *p_bitmap = G_bitmap ;
}

/****************************************************************************/
/*  Routine:  MouseSetDefaultBitmap                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseSetDefaultBitmap changes the picture's default bitmap.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 hot_spot_x, y      -- x, y position for hot spot             */
/*                                                                          */
/*    T_bitmap *p_bitmap          -- COMPRESSED Bitmap to use               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/15/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetDefaultBitmap(
          T_word16 hot_spot_x,
          T_word16 hot_spot_y,
          T_bitmap *p_bitmap)
{
    DebugRoutine("MouseSetDefaultBitmap") ;
//    DebugCheck(p_bitmap != NULL) ;

    G_defaultHotSpotX = hot_spot_x ;
    G_defaultHotSpotY = hot_spot_y ;

    G_defaultBitmap = p_bitmap ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseDraw                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseDraw is used to force the mouse to display on the screen.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 hot_spot_x, y      -- x, y position for hot spot             */
/*                                                                          */
/*    T_bitmap *p_bitmap          -- COMPRESSED Bitmap to use               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseDraw(T_void)
{
    T_word16 mx, my ;
    T_screen was ;

    DebugRoutine("MouseDraw") ;
//    DebugCheck(G_bitmap != NULL) ;

    // Draw if we have a bitmap AND we are not in relative mouse mode
    if ((G_bitmap) && (MouseIsRelativeMode() == FALSE))  {
        IMouseGetUpperLeft(&mx, &my) ;

        /* First, save what is behind the mouse. */
        IMouseTransfer(mx, my, GRAPHICS_ACTUAL_SCREEN, G_mouseScreen) ;

        was = GrScreenGet() ;
        GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;

        GrDrawCompressedBitmapAndClipAndColor(
            PictureToBitmap((T_byte8 *)G_bitmap),
            mx,
            my,
            G_colorTable) ;

        G_lastDrawX = mx ;
        G_lastDrawY = my ;

        GrScreenSet(was) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseErase                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseErase removes the mouse from the display using the hidden data.  */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseErase(T_void)
{
    DebugRoutine("MouseErase") ;

    if (G_bitmap != NULL)
        /* Draw what was behind the mouse back on the screen. */
        IMouseTransfer(G_lastDrawX, G_lastDrawY, G_mouseScreen, GRAPHICS_ACTUAL_SCREEN) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IMouseTransfer                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseErase removes the mouse from the display using the hidden data.  */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IMouseTransfer(T_word16 x, T_word16 y, T_screen from, T_screen to)
{
    T_screen was ;
    T_sword16 top, bottom, left, right ;

    DebugRoutine("IMouseTransfer") ;
    DebugCheck(G_bitmap != NULL) ;

    /* Transfer the mouse */
    was = GrScreenGet() ;
    GrScreenSet(from) ;

    top = y ;
    bottom = y + PictureGetHeight(G_bitmap)-1 ;
    left = x ;
    right = x + PictureGetWidth(G_bitmap)-1 ;

    if (top < 0)
        top = 0 ;
    if (bottom >= 200)
        bottom = 199 ;
    if (left < 0)
        left = 0 ;
    if (right >= 320)
        right = 319 ;

    GrTransferRectangle(
           to,
           left,
           top,
           right,
           bottom,
           left,
           top) ;

    GrScreenSet(was) ;

    DebugEnd() ;
}

T_void IMouseGetUpperLeft(T_word16 *p_x, T_word16 *p_y)
{
    T_word16 mx, my ;
    T_word16 sx, sy ;
    T_sword16 gx, gy ;

    DebugRoutine("IMouseGetUpperLeft") ;

    /* Get the mouse coordinates */
    IMouseGetMousePosition(&mx, &my) ;

    /* Get the picture size. */
    PictureGetXYSize(G_bitmap, &sy, &sx) ;

    gx = mx ;
    gy = my ;
    gx -= G_hotSpotX ;
    gy -= G_hotSpotY ;

/*
    if (gx < 0)
        gx = 0 ;
    else if (gx+sx >= 320)
        gx = 319-sx ;

    if (gy < 0)
        gy = 0 ;
    else if (gy+sy >= 200)
        gy = 199-sy ;
*/

    *p_x = gx ;
    *p_y = gy ;

/*
    DebugCheck(gx < 320) ;
    DebugCheck(gy < 200) ;
    DebugCheck(gx+sx < 320) ;
    DebugCheck(gy+sy < 200) ;
*/

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MouseUseDefaultBitmap                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseUseDefaultBitmap returns the mouse to its default picture.       */
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
/*    MouseSetBitmap                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/15/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseUseDefaultBitmap()
{
    DebugRoutine("MouseUseDefaultBitmap") ;
//    DebugCheck(G_defaultBitmap != NULL) ;

    /* Revert to no colorization. */
    G_colorTable = COLORIZE_TABLE_NONE ;

    /* Change to the default bitmap picture. */
    MouseSetBitmap(G_defaultHotSpotX, G_defaultHotSpotY, G_defaultBitmap) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IMouseInstallCallback                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IMouseInstallCallback puts sets up the mouse driver to call the       */
/*  IMouseCallback routine every time something is moved.                   */
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
/*    segread                                                               */
/*    int386x                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/15/94  Created                                                */
/*                                                                          */
/****************************************************************************/
#ifdef DOS32
T_void IMouseInstallCallback(T_void)
{
    struct SREGS sregs ;
    union REGS inregs ;
    union REGS outregs ;

/* Can't get to work correctly -- take it out. */
//return ;

    DebugRoutine("IMouseInstallCallback") ;

    if (G_callbackInstalled == FALSE)  {
puts("Installing mouse callback") ;
        segread(&sregs) ;
        inregs.w.ax = 0xC ;
        inregs.w.cx = 0x000F ;
        inregs.x.edx = FP_OFF(IMouseCallback) ;
        sregs.es = FP_SEG(IMouseCallback) ;
        int386x(0x33, &inregs, &outregs, &sregs) ;

        G_callbackInstalled = TRUE ;

        /* Make sure the callback is removed at some time. */
        if (G_atexitInstalled == FALSE)  {
            G_atexitInstalled = TRUE ;
            atexit(IMouseUninstallCallback) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IMouseUninstallCallback                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IMouseUninstallCallback turns off the mouse driver callback capability*/
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
/*    int386                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/15/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void IMouseUninstallCallback(T_void)
{
    union REGS inregs ;
    union REGS outregs ;

/* Can't get to work correctly -- take it out. */
//return ;

    DebugRoutine("IMouseUninstallCallback") ;

    if (G_callbackInstalled == TRUE)  {
puts("IMouseUninstallCallback") ;
fflush(stdout) ;
        inregs.w.ax = 0x0 ;
        int386(0x33, &inregs, &outregs) ;

        G_callbackInstalled = FALSE ;
    }

    DebugEnd() ;
}

#pragma off (check_stack)
//static T_void _interrupt _loadds far IMouseCallback(T_word32 max, T_word32 mcx, T_word32 mdx)
//static T_void __interrupt __far IMouseCallback(T_word32 max, T_word32 mcx, T_word32 mdx)
static T_void _loadds __far IMouseCallback(T_word32 max, T_word32 mcx, T_word32 mdx)
{
    INDICATOR_LIGHT(281, INDICATOR_GREEN) ;
    G_interX = mcx>>1 ;
    G_interY = mdx ;
    INDICATOR_LIGHT(281, INDICATOR_RED) ;
//    if (G_mouseShowLevel > 0)  {
/*
    if (G_allowUpdate)  {
        IMouseTransfer(G_lastDrawX, G_lastDrawY, G_mouseScreen, ((char *)0xA0000)) ;
        GrDrawCompressedBitmapAndClip(PictureToBitmap((T_byte8 *)G_bitmap), mcx, mdx) ;
        IMouseTransfer(mcx, mdx, ((char *)0xA0000), G_mouseScreen) ;
        G_lastDrawX = mcx ;
        G_lastDrawY = mdx ;
    }
*/
}

#pragma on (check_stack)
#endif

/****************************************************************************/
/*  Routine:  MouseSetColorize                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MouseSetColorize sets the colorization table to be used with the      */
/*  picture of the mouse.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Colorize does not work with the default cursor.                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_colorizeTable colorTable  -- Colorization table to use              */
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
/*    LES  11/27/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MouseSetColorize(E_colorizeTable colorTable)
{
    DebugRoutine("MouseSetColorize") ;
    DebugCheck(colorTable < MAX_COLORIZE_TABLES) ;

    G_colorTable = colorTable ;

    DebugEnd() ;
}

T_void MouseDisallowUpdate(T_void)
{
    G_allowUpdate = FALSE ;
}

T_void MouseAllowUpdate(T_void)
{
    G_allowUpdate = TRUE ;
}

/****************************************************************************/
/*  Routine:  MousePopEventHandler                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MousePopEventHandler restores the last handler on the stack           */
/*  (if there is one).                                                      */
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
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*    MouseSetEventHandler                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MousePopEventHandler(T_void)
{
    T_doubleLinkListElement first ;
    T_mouseEventHandler handler ;

    DebugRoutine("MousePopEventHandler") ;

    /* Get the old event handler. */
    first = DoubleLinkListGetFirst(G_eventStack) ;
    DebugCheck(first != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    if (first != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        handler = (T_mouseEventHandler)
                      DoubleLinkListElementGetData(first) ;

        MouseSetEventHandler(handler) ;
        DoubleLinkListRemoveElement(first) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MousePushEventHandler                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MousePushEventHandler removes the old handler by placing it on        */
/*  a stack and sets up the new event handler.  The old event handler       */
/*  will be restored when a call to MousePopEventHandler is called.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_keyboardEventHandler keyboardEventHandler                           */
/*                                -- function to call on events, or NULL    */
/*                                   for none.                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListAddElementAtFront                                       */
/*    MouseSetEventHandler                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MousePushEventHandler(T_mouseEventHandler mouseEventHandler)
{
    DebugRoutine("MousePushEventHandler") ;

    /* Store the old event handler. */
    DoubleLinkListAddElementAtFront(
        G_eventStack,
        (T_void *)MouseGetEventHandler) ;

    /* set up the new handler. */
    MouseSetEventHandler(mouseEventHandler) ;

    DebugEnd() ;
}

T_void MouseRelativeModeOn(T_void)
{
    DebugRoutine("MouseRelativeModeOn");

    if (!G_relativeMode) {
        G_relativeMode = TRUE;
        // Remember where the mouse is located (we'll return there when we exit)
        IMouseGetMousePosition(&G_mousePreRelativeX, &G_mousePreRelativeY);

        // Center the mouse
        MouseMoveTo(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2);
    }

    DebugEnd();
}

T_void MouseRelativeModeOff(T_void)
{
    DebugRoutine("MouseRelativeModeOff");

    if (G_relativeMode) {
        // Restore the original mouse location
        MouseMoveTo(G_mousePreRelativeX, G_mousePreRelativeY);

        // End relative mouse movement
        G_relativeMode = FALSE;
    }

    DebugEnd();
}

E_Boolean MouseIsRelativeMode(T_void)
{
    return G_relativeMode;
}

T_void MouseSetRelativeSensitivity(T_word16 sensitivity)
{
    DebugRoutine("MouseSetRelativeSensitivity");
    DebugCheck(sensitivity > 0);
    DebugCheck(sensitivity < 100); // arbitrary maximum for now

    G_relativeSensitivity = sensitivity;

    DebugEnd();
}

T_void MouseRelativeRead(T_sword16 *aDeltaX, T_sword16 *aDeltaY)
{
    T_word16 x, y;
    DebugRoutine("MouseRelativeRead");

    if (G_relativeMode) {
        // Read the mouse position and then recenter the mouse
        IMouseGetMousePosition(&x, &y);

        // How far off did the mouse move from the middle?
        *aDeltaX = ((T_sword16)(x - SCREEN_SIZE_X/2))*G_relativeSensitivity;
        *aDeltaY = ((T_sword16)(y - SCREEN_SIZE_Y/2))*G_relativeSensitivity;

        // Center the mouse
        MouseMoveTo(SCREEN_SIZE_X/2, SCREEN_SIZE_Y/2);
    } else {
        // Not in relative mouse mode, just return 0's
        *aDeltaX = *aDeltaY = 0;
    }

    DebugEnd();
}

T_buttonClick MouseGetButtonStatus(T_void)
{
    T_buttonClick buttonStatus;

    DebugRoutine("MouseGetButtonStatus");

    buttonStatus = IMouseGetButtonStatus() ;

    DebugEnd();

    return buttonStatus;
}

/****************************************************************************/
/*    END OF FILE:  MOUSEMOD.C                                              */
/****************************************************************************/
