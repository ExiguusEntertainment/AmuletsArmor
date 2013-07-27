/****************************************************************************/
/*    FILE:  UIBUTTON.C                                                     */
/****************************************************************************/
#include "UIBUTTON.H"

#define UI_BUTTON_STATE_FOCUSED    0x01    /* -------1 */
#define UI_BUTTON_STATE_PRESSED     0x02    /* ------1- */
#define UI_BUTTON_STATE_UNKNOWN     0xFC    /* 111111-- */

typedef struct {
    T_resource focusPic ;
    T_resource notFocusPic ;
    T_resource pressedPic ;
    T_byte8 f_buttonState ;
    T_buttonHandler buttonHandler ;
    T_word16 x, y ;
    T_resource lastPic ;
    T_word32 accessoryData ;
} T_UIButtonStruct ;

/* Internal routines: */
static E_Boolean IUIButtonStandardEventHandler(
              T_UIObject object,
              E_UIEvent event,
              T_word16 data1,
              T_word16 data2) ;

static T_void IUIButtonDraw(T_UIButtonStruct *p_button) ;

/****************************************************************************/
/*  Routine:  UIButtonCreate                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIButtonCreate does all the work to set up a button to be displayed   */
/*  on the screen.  Just give it a group, 3 resource pictures, and a        */
/*  XY coordinate to place it on the screen.  The button will then appear   */
/*  when the group is drawn.                                                */
/*    Note:  The passed button resources are for masked bitmaps, so you     */
/*  will want the graphics drawn accordingly.  However, the button WILL     */
/*  be pressed if in the rectangular area (no checks are made to look into  */
/*  the bitmap).                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that the x & y coordinates leave enough room on the screen  */
/*  for the whole button.  Otherwise, it will bomb when it is drawn.        */
/*    Also, make sure that all your resource bitmaps are the same size.     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup group             -- Group to add button to.                */
/*                                                                          */
/*    T_resource focusPic        -- Picture to use when mouse is over      */
/*                                   button.                                */
/*                                                                          */
/*    T_resource notFocusPic     -- Picture to use when mouse is elsewhere */
/*                                                                          */
/*    T_resource pressedPic       -- Picture to use when mouse is pressing  */
/*                                   down on button.                        */
/*                                                                          */
/*    T_word16 x                  -- X position of button                   */
/*                                                                          */
/*    T_word16 y                  -- Y position of button                   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_UIButton                  -- Returned handle to button created      */
/*                                   or else will be UI_BUTTON_BAD          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceLock                                                          */
/*    ResourceUnlock                                                        */
/*    UIObjectCreate                                                        */
/*    UIObjectSetEventHandler                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Change parameter passing to UIObjectCreate             */
/*                                                                          */
/****************************************************************************/

T_UIButton UIButtonCreate(
               T_UIGroup group,
               T_resource focusPic,
               T_resource notFocusPic,
               T_resource pressedPic,
               T_word16 x,
               T_word16 y)
{
    T_UIButtonStruct *p_button ;
    T_UIButton button ;
    T_bitmap *bitmap ;

    DebugRoutine("UIButtonCreate") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(focusPic != RESOURCE_BAD) ;
    DebugCheck(notFocusPic != RESOURCE_BAD) ;
    DebugCheck(pressedPic != RESOURCE_BAD) ;
    DebugCheck(x < SCREEN_SIZE_X) ;
    DebugCheck(y < SCREEN_SIZE_Y) ;

    /* Create the button by createing a object with an add on size */
    /* equal to the structure we need to store. */
    p_button = button = UIObjectCreate(sizeof(T_UIButtonStruct)) ;

    /* Lock in the normally (not focusted) bitmap into memory. */
    bitmap = ResourceLock(notFocusPic) ;

    /* Set up the area that the button will occupy. */
    UIObjectSetArea(button, x, y, x+bitmap->sizex-1, y+bitmap->sizey-1) ;

    /* We are done with the bitmap, unlock it. */
    ResourceUnlock(notFocusPic) ;

    /* Set up the standard button event handler. */
    UIObjectSetEventHandler(button, IUIButtonStandardEventHandler) ;

    /* Set up the handler for this particular button.  Let's make it */
    /* NULL to signify that none has been set-up. */
    p_button->buttonHandler = NULL ;

    /* Store all the important information. */
    p_button->focusPic = focusPic ;
    p_button->notFocusPic = notFocusPic ;
    p_button->pressedPic = pressedPic ;

    /* Make sure the initial state of the button is normal. */
    p_button->f_buttonState = 0 ;

    /* Also note that the last picture we used for displaying */
    /* the button is illegal. */
    p_button->lastPic = RESOURCE_BAD ;

    /* Record the x & y location for easy access. */
    p_button->x = x ;
    p_button->y = y ;

    /* Now that we have the whole button created, let's add it */
    /* to the UI Group. */
    UIGroupAttachUIObject(group, button) ;

    DebugEnd() ;

    return button ;
}

/****************************************************************************/
/*  Routine:  UIButtonSetHandler                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIButtonSetHandler will declare the handler that will be used for     */
/*  this button.  Basically, the handler will be called when the button     */
/*  is pressed.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_buttonHandler             -- Function to call when button is pressed*/
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
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Got rid of need to use UIObjectGetAddOn routine        */
/*                                                                          */
/****************************************************************************/

T_void UIButtonSetHandler(T_UIButton button, T_buttonHandler buttonHandler)
{
    T_UIButtonStruct *p_button ;

    DebugRoutine("UIButtonSetHandler") ;
    DebugCheck(buttonHandler != NULL) ;
    DebugCheck(button != UI_BUTTON_BAD) ;

    /* First get the button part of the ui object passed in. */
    p_button = button ;

    /* Change the handler. */
    p_button->buttonHandler = buttonHandler ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IUIButtonStandardEventHandler      * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIButtonStandardEventHandler is the standard ui handler that is      */
/*  used for all buttons.  Most applications will never need to change      */
/*  this.  It basically does all the processing of a button including       */
/*  the changing of pictures.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- This particular ui object/button       */
/*                                                                          */
/*    E_UIEvent event             -- The event to process                   */
/*                                                                          */
/*    T_word16 data1              -- Mouse X coordinate (not used)          */
/*                                                                          */
/*    T_word16 data2              -- Mouse Y coordinate (not used)          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE  = Processed event                */
/*                                   FALSE = Event ignored                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IUIButtonDraw                                                         */
/*    Whatever handle is defined in T_UIButtonStruct                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Got rid of need to use UIObjectGetAddOn routine        */
/*                                                                          */
/****************************************************************************/

static E_Boolean IUIButtonStandardEventHandler(
              T_UIObject object,
              E_UIEvent event,
              T_word16 data1,
              T_word16 data2)
{
    T_UIButtonStruct *p_button ;
    E_Boolean handled = FALSE ;

    DebugRoutine("IUIButtonStandardEventHandler") ;
    DebugCheck(object != UI_OBJECT_BAD) ;
    DebugCheck(event < UI_EVENT_UNKNOWN) ;

    /* First get the button part of the ui object passed in. */
    p_button = object ;

    /* OK, what event were we given? */
    switch(event)  {
        case UI_EVENT_GAINED_FOCUS:
            /* The mouse is over us, change our state. */
            p_button->f_buttonState |= UI_BUTTON_STATE_FOCUSED ;

            /* Draw a different picture. */
            IUIButtonDraw(p_button) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_LOST_FOCUS:
            /* The mouse is no longer over us, change our state. */
            p_button->f_buttonState &= (~UI_BUTTON_STATE_FOCUSED) ;

            /* Draw a different picture. */
            IUIButtonDraw(p_button) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_MOUSE_START:
            /* The mouse is pressing down on us, change our state. */
            p_button->f_buttonState |= UI_BUTTON_STATE_PRESSED ;

            /* Draw a different picture. */
            IUIButtonDraw(p_button) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_MOUSE_END:
            /* The mouse is no longer pressing down on us, change */
            /* our state. */
            p_button->f_buttonState &= (~UI_BUTTON_STATE_PRESSED) ;

            /* Draw a different picture. */
            IUIButtonDraw(p_button) ;

            /* Since the button was fully pressed, let's go ahead */
            /* and tell the handler to process this button press */
            /* (if there is a handler). */
            if (p_button->buttonHandler != NULL)
                p_button->buttonHandler((T_UIButton)p_button) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_DRAW:
            /* Draw a the picture. */
            IUIButtonDraw(p_button) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
    }

    DebugCheck(handled < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return handled ;
}

/****************************************************************************/
/*  Routine:  IUIButtonDraw                      * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIButtonDraw draws the appropriate picture based on the state of     */
/*  the button.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIButtonStructure *p_button  -- Pointer to the actual button struct */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    GrDrawBitmapMasked                                                    */
/*    ResourceLock                                                          */
/*    ResourceUnlock                                                        */
/*    MouseHide                                                             */
/*    MouseShow                                                             */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  12/12/94  Added missing Debug statements.                        */
/*                                                                          */
/****************************************************************************/

static T_void IUIButtonDraw(T_UIButtonStruct *p_button)
{
    T_bitmap *p_bitmap ;
    T_resource r_pic ;

    DebugRoutine("IUIButtonDraw") ;
    DebugCheck(p_button != NULL) ;

    /* Get the resource based on its current state. */
    switch(p_button->f_buttonState)  {
        case (0):
           /* Nothing active, use none-focus pic. */
            r_pic = p_button->notFocusPic ;
            break ;

        case (UI_BUTTON_STATE_FOCUSED):
            /* Mouse is over us, use the focus pic. */
            r_pic = p_button->focusPic ;
            break ;

        case (UI_BUTTON_STATE_PRESSED):
        case (UI_BUTTON_STATE_FOCUSED | UI_BUTTON_STATE_PRESSED):
            /* We're being pressed and probably focused, use pressed pic. */
            r_pic = p_button->pressedPic ;
            break ;
    }

    /* Now, is this new picture any different than what we last */
    /* put up? */
    if (r_pic != p_button->lastPic)  {
        /* Yes, it is.  Ok, let's lock in the bitmap and draw it. */
        p_bitmap = ResourceLock(r_pic) ;
        DebugCheck(p_bitmap != NULL) ;

        MouseHide() ;
        GrDrawBitmapMasked(p_bitmap, p_button->x, p_button->y) ;
        MouseShow() ;

        ResourceUnlock(r_pic) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIButtonSetAccessoryData                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Every button has a slot available for storing one 32 bit word.        */
/*  This data slot can be used for anything that the calling module         */
/*  wishes.  Most of the time, it is used to record a button ID.  The       */
/*  ID allows the programmer to use the same button handler for multiple    */
/*  buttons.  This routine is used to store that data.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIButton button           -- Button to store data within            */
/*                                                                          */
/*    T_word32 data               -- Data to store                          */
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
/*    LES  12/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void UIButtonSetAccessoryData(T_UIButton button, T_word32 data)
{
    T_UIButtonStruct *p_button ;

    DebugRoutine("UIButtonSetAccessoryData") ;
    DebugCheck(button != UI_BUTTON_BAD) ;

    /* First get the button part of the ui object passed in. */
    p_button = button ;

    /* Store the data. */
    p_button->accessoryData = data ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIButtonGetAccessoryData                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Every button has a slot available for storing one 32 bit word.        */
/*  This data slot can be used for anything that the calling module         */
/*  wishes.  Most of the time, it is used to record a button ID.  The       */
/*  ID allows the programmer to use the same button handler for multiple    */
/*  buttons.  This routine is used to retrieve that data.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIButton button           -- Button to retrieve data from           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32 data               -- Data that was stored                   */
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
/*    LES  12/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 UIButtonGetAccessoryData(T_UIButton button)
{
    T_UIButtonStruct *p_button ;
    T_word32 data ;

    DebugRoutine("UIButtonGetAccessoryData") ;
    DebugCheck(button != UI_BUTTON_BAD) ;

    /* First get the button part of the ui object passed in. */
    p_button = button ;

    /* Retrieve the data. */
    data = p_button->accessoryData ;

    DebugEnd() ;

    return data ;
}

/****************************************************************************/
/*    END OF FILE:  UIBUTTON.C                                              */
/****************************************************************************/
