/****************************************************************************/
/*    FILE:  UI.C                                                           */
/****************************************************************************/
#include "KEYBOARD.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "MOUSEMOD.H"
#include "UI.H"

#define MAX_UI_OBJECTS_IN_GROUP 20

#define GROUP_FOCUS_BAD (-1)

typedef struct {
    T_UIObject list[MAX_UI_OBJECTS_IN_GROUP] ;
    T_word16 numInList ;
    T_sword16 activeObject ;
    T_resource background ;
    T_screen screen ;
    T_word16 lastFocus ;
} T_UIGroupStruct ;

typedef struct {
    T_word16 left ;
    T_word16 top ;
    T_word16 right ;
    T_word16 bottom ;
    T_uiEventHandler uiEventHandler ;
} T_UIObjectStruct ;

/* Keep track of which UI group is active: */
static T_UIGroup G_ActiveUIGroup = UI_GROUP_BAD ;

/* Internal prototypes: */
static T_void IUIGroupMouseHandler(
           E_mouseEvent event,
           T_word16 x,
           T_word16 y,
           E_Boolean button) ;

static T_void IUIGroupKeyboardHandler(
           E_keyboardEvent event,
           T_word16 key) ;

static E_Boolean IUIObjectHandleEvent(
                    T_UIObject object,
                    E_UIEvent event,
                    T_word16 data1,
                    T_word16 data2) ;

static T_void IUIObjectDestroy(T_UIObject obj) ;

static E_Boolean IUIObjectIsInside(T_UIObject obj, T_word16 x, T_word16 y) ;

static T_void IUIGroupChangeFocus(T_sword16 indexObject) ;

/****************************************************************************/
/*  Routine:  UISetActiveGroup                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Once a UIGroup object has been declared and set-up with all of its    */
/*  UIObjects, you can declare the UIGroup to be the active group.          */
/*  Use this routine to make the UIGroup connect into the system and        */
/*  receive mouse and keyboard events.  If you no longer need to have an    */
/*  active UIGroup, send UI_GROUP_BAD to this routine.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup group             -- Group to make active or use            */
/*                                   UI_GROUP_BAD for none.                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    KeyboardSetEventHandler                                               */
/*    MouseSetEventHandler                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void UISetActiveGroup(T_UIGroup group)
{
    DebugRoutine("UISetActiveGroup") ;
    /* I can't think of any DebugChecks that I can do at this point. */

    /* Make the given group the active group. */
    G_ActiveUIGroup = group ;

    /* Check to see that the group is not UI_GROUP_BAD. */
    if (group != UI_GROUP_BAD)  {
        /* Turn on the keyboard and mouse handlers. */
        KeyboardSetEventHandler(IUIGroupKeyboardHandler) ;
        MouseSetEventHandler(IUIGroupMouseHandler) ;
    } else {
        /* Turn off the keyboard and mouse handlers. */
        KeyboardSetEventHandler(NULL) ;
        MouseSetEventHandler(NULL) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIGroupAttachUIObject                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupAttachUIObject is used to add objects to a UI Group.  Each     */
/*  added object represents another piece of UI on the screen that will     */
/*  be passed UI Events.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    I can't really tell if the group and object are truly legal, but      */
/*  as long as they are not BAD,  you should be ok.                         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup group             -- Group to add object to                 */
/*                                                                          */
/*    T_UIObject object           -- Object to add                          */
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
/*                                                                          */
/****************************************************************************/

T_void UIGroupAttachUIObject(T_UIGroup group, T_UIObject object)
{
    T_UIGroupStruct *p_group ;

    DebugRoutine("UIGroupAttachUIGroup") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(object != UI_GROUP_BAD) ;

    /* Get pointer to UI group structure. */
    p_group = group ;

    /* Make sure we have not gone over the limit. */
    DebugCheck(p_group->numInList != MAX_UI_OBJECTS_IN_GROUP) ;

    /* Add the object to the group list and increment the count. */
    p_group->list[p_group->numInList++] = object ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIGroupCreate                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupCreate is used to allocate memory for a new UI Group.          */
/*  If there is not enough memory, a UI_GROUP_BAD is returned.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that when you are returned a T_UIGroup that you check       */
/*  to see if it is UI_GROUP_BAD.  In many cases, it is best just to bomb.  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_UIGroup                   -- UIGroup or UI_GROUP_BAD if out of      */
/*                                   memory.                                */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_UIGroup UIGroupCreate(T_void)
{
    T_UIGroupStruct *p_group ;

    DebugRoutine("UIGroupCreate") ;

    /* Allocate the memory for a group. */
    p_group = MemAlloc(sizeof(T_UIGroupStruct)) ;

    /* Check to see if it was allocated. */
    if (p_group == NULL)  {
        /* If it was not allocated, make sure we return a BAD */
        p_group = UI_GROUP_BAD ;
    } else {
        /* If it was allocated, initialize the structure data. */
        /* Declare the group to be empty. */
        p_group->numInList = 0 ;

        /* Declare the active object to be none. */
        p_group->activeObject = GROUP_FOCUS_BAD ;

        /* Make the background image undeclared. */
        p_group->background = RESOURCE_BAD ;

        /* Make the screen for this group undeclared. */
        p_group->screen = SCREEN_BAD ;
    }

    DebugEnd() ;

    /* Return whatever was/wasn't allocated. */
    return ((T_UIGroup)p_group) ;
}

/****************************************************************************/
/*  Routine:  UIGroupDestroy                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupDestroy detaches, deallocates, and cleans up all the objects   */
/*  under and including the UI Group.  This routine is a all in one         */
/*  technique.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The UIGroup being destroy MUST NOT be the active UI Group.  Make      */
/*  to call UISetActiveGroup(UI_GROUP_BAD) before executing this command.   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup                   -- Group you want destroyed               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IUIObjectDestroy                                                      */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void UIGroupDestroy(T_UIGroup group)
{
    T_UIGroupStruct *p_group ;
    T_word16 i ;

    DebugRoutine("UIGroupDestroy") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(group != G_ActiveUIGroup) ;

    /* Get structure pointer to group. */
    p_group = group ;

    /* Loop through all the objects in the group list and destroy */
    /* all those objects. */
    for (i=0; i<p_group->numInList; i++)
        /* Then we will actually free up the memory used by that object. */
        IUIObjectDestroy(p_group->list[i]) ;

    /* Now that all the objects underneath are destroyed, */
    /* Destroy our self. */
    MemFree(p_group) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIGroupDraw                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupDraw draws the background image and all the objects that are   */
/*  part of this group.  Note that it only does this to the active group.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Be sure to call UISetActiveGroup and UIGroupSetScreen before using    */
/*  this command.                                                           */
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
/*    GrDrawBitmap                                                          */
/*    GrDrawRectangle                                                       */
/*    GrScreenSet                                                           */
/*    ResourceLock                                                          */
/*    ResourceUnlock                                                        */
/*    IUIObjectHandleEvent                                                  */
/*    MouseHide                                                             */
/*    MouseShow                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void UIGroupDraw(T_void)
{
    T_UIGroupStruct *p_group ;
    T_bitmap *p_background ;
    T_word16 i ;

    DebugRoutine("UIGroupDraw") ;
    DebugCheck(G_ActiveUIGroup != UI_GROUP_BAD) ;

    /* Get a pointer to the structure of the active group. */
    p_group = G_ActiveUIGroup ;

    /* Make sure the screen is declared. */
    DebugCheck(p_group->screen != SCREEN_BAD) ;

    /* Make this screen the active screen. */
    GrScreenSet(p_group->screen) ;

    /* Hide the mouse from everything we draw. */
    MouseHide() ;

    /* Start by drawing a black background over everything. */
    GrDrawRectangle(0, 0, SCREEN_SIZE_X-1, SCREEN_SIZE_Y-1, COLOR_BLACK) ;

    /* Check to see if we have a background resource. */
    if (p_group->background != RESOURCE_BAD)  {
        /* Let's bring in the background. */
        p_background = ResourceLock(p_group->background) ;

        /* Draw it in the upper left hand corner. */
        GrDrawBitmap(p_background, 0, 0) ;

        /* Unlock the resource. */
        ResourceUnlock(p_group->background) ;
    }

    /* Now let's draw each of the objects underneath this UI Group. */
    /* To do so, requires that we send a draw event to each object. */
    for (i=0; i<p_group->numInList; i++)
        IUIObjectHandleEvent(p_group->list[i], UI_EVENT_DRAW, 0, 0) ;

    /* Done drawing, let the mouse appear again. */
    MouseShow() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:                                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupSetScreen tells what screen to draw all the objects to         */
/*  under a UI Group.  Usually this will be a screen that was allocated     */
/*  for the purpose of being drawn once before being displayed.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Screen must not be a NULL or SCREEN_BAD.                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup group             -- Group to change screen                 */
/*                                                                          */
/*    T_screen screen             -- Screen to use                          */
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
/*                                                                          */
/****************************************************************************/

T_void UIGroupSetScreen(T_UIGroup group, T_screen screen)
{
    T_UIGroupStruct *p_group ;

    DebugRoutine("UIGroupSetScreen") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(screen != SCREEN_BAD) ;

    /* Get a pointer to the ui group structure. */
    p_group = group ;

    /* Change the screen. */
    p_group->screen = screen ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIGroupSetBackground                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIGroupSetBackground declares the background of the group you         */
/*  passed.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Note that the background is not drawn until a UIGroupDraw command     */
/*  is issued.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIGroup group             -- Group to change background of          */
/*                                                                          */
/*    T_resource background       -- Background resource to use             */
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
/*                                                                          */
/****************************************************************************/

T_void UIGroupSetBackground(T_UIGroup group, T_resource background)
{
    T_UIGroupStruct *p_group ;

    DebugRoutine("UIGroupSetScreen") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(background != RESOURCE_BAD) ;

    /* Get a pointer to the ui group structure. */
    p_group = group ;

    /* Change the background. */
    p_group->background = background ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIObjectCreate                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectCreate creates a general object that can be added to a        */
/*  UI Group.  In addition, you can allocate additional memory for whatever */
/*  use you see fit (usually for the specific UI objects like buttons).     */
/*  Just tell it how much space you need.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 size               -- Size for ui object to allocate in      */
/*                                   addition to the normal amount.         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_UIObject                  -- Pointer to whole object.               */
/*                                   Returns UI_OBJECT_BAD if it count not  */
/*                                   allocate enough memory.                */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_UIObject UIObjectCreate(T_word16 size)
{
    T_UIObjectStruct *p_object ;

    DebugRoutine("UIObjectCreate") ;

    /* Allocate the memory for the structure and the add on */
    p_object = MemAlloc(sizeof(T_UIObjectStruct) + size) ;

    /* Check to see if we were able to allocate the memory. */
    if (p_object == NULL)  {
        /* No good, we'll have to return a bad object. */
        p_object = UI_OBJECT_BAD ;
    } else {
        /* We got our memory, let's initialize it. */
        /* Let's make sure there is not an event handler for the object. */
        p_object->uiEventHandler = NULL ;
        /* Since we don't know the size of the object, let's give it */
        /* an very small size.  This will make the object practically */
        /* unselectable. */
        p_object->left = 1 ;
        p_object->top = 1 ;
        p_object->right = 1 ;
        p_object->bottom = 1 ;
    }

    DebugEnd() ;

    return ((T_UIObject)(((T_UIObjectStruct *)p_object)+1)) ;
}

/****************************************************************************/
/*  Routine:  UIObjectSetArea                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectSetArea declares the region in which the UI object exists.    */
/*  In general, UI objects should never overlap, and if they do, they       */
/*  will have unpredictable results.  Think of the ui objects as tiles,     */
/*  and just keep them that way.                                            */
/*    For this routine, pass the bounds of the rectangle along with the     */
/*  object handle to change.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure all sides are inside the screen and that left is left of    */
/*  right and top is above bottom.                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_UIObject object           -- Object to change bounds of             */
/*                                                                          */
/*    left                        -- Left edge of object                    */
/*                                                                          */
/*    top                         -- Top edge of object                     */
/*                                                                          */
/*    right                       -- Right edge of object                   */
/*                                                                          */
/*    bottom                      -- Bottom edge of object                  */
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
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_void UIObjectSetArea(
           T_UIObject object,
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom)
{
    T_UIObjectStruct *p_object ;

    DebugRoutine("UIObjectSetArea") ;
    DebugCheck(object != UI_OBJECT_BAD) ;
    DebugCheck(right < SCREEN_SIZE_X) ;
    DebugCheck(left < right) ;
    DebugCheck(bottom < SCREEN_SIZE_Y) ;
    DebugCheck(top < bottom ) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Set the object's bounds: */
    p_object->left = left ;
    p_object->right = right ;
    p_object->top = top ;
    p_object->bottom = bottom ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIObjectSetEventHandler                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Most objects will need an event handler to control the functionality  */
/*  of the UI object.  For example, buttons need to press inward when       */
/*  clicked and windows need to scroll.  This routine declares the handler  */
/*  for the ui object.  Passing a NULL will also turn off the event         */
/*  handler.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_uiEventHandler uiEventHandler  -- Routine to handle UI events.      */
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
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_void UIObjectSetEventHandler(
           T_UIObject object,
           T_uiEventHandler uiEventHandler)
{
    T_UIObjectStruct *p_object ;

    DebugRoutine("UIObjectSetEventHandler") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Set the object's ui event handler: */
    p_object->uiEventHandler = uiEventHandler ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  UIObjectGetLeftPosition                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectGetLeftPosition returns the left location of the UI Object.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to get left of                  */
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
/*    LES  11/23/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_word16 UIObjectGetLeftPosition(T_UIObject object)
{
    T_UIObjectStruct *p_object ;
    T_word16 left ;

    DebugRoutine("UIObjectGetLeftPosition") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Get the left side */
    left = p_object->left ;

    DebugCheck(left < SCREEN_SIZE_X) ;
    DebugEnd() ;

    return left ;
}

/****************************************************************************/
/*  Routine:  UIObjectGetTopPosition                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectGetTopPosition returns the top location of the UI Object.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to get top of                   */
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
/*    LES  11/23/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_word16 UIObjectGetTopPosition(T_UIObject object)
{
    T_UIObjectStruct *p_object ;
    T_word16 top ;

    DebugRoutine("UIObjectGetTopPosition") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Get the top side */
    top = p_object->top ;

    DebugCheck(top < SCREEN_SIZE_Y) ;
    DebugEnd() ;

    return top ;
}

/****************************************************************************/
/*  Routine:  UIObjectGetRightPosition                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectGetRightPosition returns the right location of the UI Object. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to get right of                 */
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
/*    LES  11/23/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_word16 UIObjectGetRightPosition(T_UIObject object)
{
    T_UIObjectStruct *p_object ;
    T_word16 right ;

    DebugRoutine("UIObjectGetRightPosition") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Get the left side */
    right = p_object->right ;

    DebugCheck(right < SCREEN_SIZE_X) ;
    DebugEnd() ;

    return right ;
}

/****************************************************************************/
/*  Routine:  UIObjectGetBottomPosition                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UIObjectGetBottomPosition returns the bottom location of the UI       */
/*  Object.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to get bottom of                */
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
/*    LES  11/23/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

T_word16 UIObjectGetBottomPosition(T_UIObject object)
{
    T_UIObjectStruct *p_object ;
    T_word16 bottom ;

    DebugRoutine("UIObjectGetBottomPosition") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object's structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Get the bottom side */
    bottom = p_object->bottom ;

    DebugCheck(bottom < SCREEN_SIZE_Y) ;
    DebugEnd() ;

    return bottom ;
}


/****************************************************************************/
/*  Routine:  IUIGroupMouseHandler               * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIGroupMouseHandler is an internal function that controls and        */
/*  interprets the mouse clicks entered by the user while a UIGroup is up.  */
/*  Each ui object inside the group is checked to see if the action         */
/*  pertains to that object.                                                */
/*    Technically, IUIGroupMouseHandler is a fitler that converts from      */
/*  mouse events to appropriate UI events (including focus events).         */
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
/*    IUIObjectHandleEvent                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/25/94  Modified to use keep track of the last legally focused */
/*                   item.                                                  */
/*                                                                          */
/****************************************************************************/

static T_void IUIGroupMouseHandler(
                  E_mouseEvent event,
                  T_word16 x,
                  T_word16 y,
                  E_Boolean button)
{
    T_sword16 indexObject ;
    T_UIGroupStruct *p_group ;
    T_UIObject object ;
    E_UIEvent uiEvent ;

    DebugRoutine("IUIGroupMouseHandler") ;
    DebugCheck(G_ActiveUIGroup != UI_GROUP_BAD) ;
    DebugCheck(event < MOUSE_EVENT_UNKNOWN) ;

    /* Get a pointer to the currently active ui group. */
    p_group = G_ActiveUIGroup ;

    /* If we are IDLE, MOVEing, or doing a START, we need to know */
    /* where the object under the cursor is. */
    if ((event == MOUSE_EVENT_START) ||
        (event == MOUSE_EVENT_MOVE) ||
        (event == MOUSE_EVENT_IDLE))  {
        /* First we will have to check for lost/gained focuses. */
        /* Find who the mouse is currently over. */
        for (indexObject = 0; indexObject<p_group->numInList; indexObject++)  {
            /* Get an object out of the list. */
            object = p_group->list[indexObject] ;

            /* Check if the mouse is inside the object.  If it is */
            /* break out of this loop. */
            if (IUIObjectIsInside(object, x, y) == TRUE)
                break ;
        }

        /* Check to see if the mouse was over anything  */
        if (indexObject == p_group->numInList)  {
            /* Not over anything, just note that none was selected. */
            object = UI_OBJECT_BAD ;
            indexObject = GROUP_FOCUS_BAD ;
        }
    } else {
        /* None of the above, assume that we are over the object */
        /* that is active.  Why?  So it let's go. */
        indexObject = p_group->activeObject ;
        object = p_group->list[indexObject] ;
    }

    /* However only move and start mouse events actually change */
    /* the focus. */
    if ((event == MOUSE_EVENT_START) || (event == MOUSE_EVENT_MOVE))
        IUIGroupChangeFocus(indexObject) ;

    /* Now we want to handle the rest mouse events. */
    /* But these events are only useful if we are over a UI object. */
    if (indexObject != GROUP_FOCUS_BAD)  {
        /* Translate the mouse events: */
        switch(event)  {
            case MOUSE_EVENT_START:
                uiEvent = UI_EVENT_MOUSE_START ;
                break ;
            case MOUSE_EVENT_END:
                uiEvent = UI_EVENT_MOUSE_END ;
                break ;
            case MOUSE_EVENT_MOVE:
                uiEvent = UI_EVENT_MOUSE_MOVE ;
                break ;
            case MOUSE_EVENT_DRAG:
                uiEvent = UI_EVENT_MOUSE_DRAG ;
                break ;
            case MOUSE_EVENT_IDLE:
                uiEvent = UI_EVENT_MOUSE_IDLE ;
                break ;
            case MOUSE_EVENT_HELD:
                uiEvent = UI_EVENT_MOUSE_HELD ;
                break ;
        }

        /* Send out the message to the active focus. */
        IUIObjectHandleEvent(object, uiEvent, x, y) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IUIGroupKeyboardHandler            * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIGroupKeyboardHandler handles all the events that are sent from     */
/*  the keyboard module.  The main thing that this routine does is handle   */
/*  TAB, SHIFT-TAB, and the sending of keys down to the active focus.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_keyboardEvent event       -- Keyboard event to process              */
/*                                                                          */
/*    T_byte16 key                -- Key to process                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    KeyboardGetScanCode                                                   */
/*    KeyboardBufferGet                                                     */
/*    IUIGroupChangeFocus                                                   */
/*    IUIObjectHandleEvent                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/25/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IUIGroupKeyboardHandler(
                  E_keyboardEvent event,
                  T_word16 key)
{
    T_UIObject object ;
    E_UIEvent uiEvent ;
    T_sword16 focus ;
    T_UIGroupStruct *p_group ;
    E_Boolean usedKey ;

    DebugRoutine("IUIGroupKeyboardHandler") ;
    DebugCheck(G_ActiveUIGroup != UI_GROUP_BAD) ;
    DebugCheck(event < KEYBOARD_EVENT_UNKNOWN) ;

    /* Get a pointer to the currently active ui group. */
    p_group = G_ActiveUIGroup ;

    /* If there are no members in this group, we don't even */
    /* do anything.  In the debugging version, I see no reason to let */
    /* this occur. */
    DebugCheck(p_group->numInList != 0) ;
    if (p_group->numInList != 0)  {
        /* Before we do anything, see if this is a TAB/SHIFT-TAB situation. */
        if (KeyboardGetScanCode(KEY_SCAN_CODE_TAB)==TRUE)  {
            /* TAB ... hmmm, we need to change the current focus. */
            /* What is the original focus. */
            focus = p_group->activeObject ;

            /* Check to see if that is a valid focus.  If it doesn't appear */
            /* to be one, let us use the last one we used before. */
            if (focus == GROUP_FOCUS_BAD)
                focus = p_group->lastFocus ;

            /* Are doing a SHIFT-TAB? */
            if ((KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_SHIFT)==TRUE) ||
                (KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_SHIFT)==TRUE))  {
               /* It's a SHIFT-TAB, move backwards through the list. */
               focus-- ;

               /* Make sure we check to roll around. */
               if (focus == -1)
                   focus = p_group->numInList-1 ;
            } else {
               /* We are doing a plain TAB, need to move forwards */
               /* in the list. */
               focus++ ;

               /* Make sure we check for roll around. */
               if (focus == p_group->numInList)
                   focus = 0 ;
            }

            /* Change to this new focus. */
            IUIGroupChangeFocus(focus) ;

            /* Now, to make sure we don't have the TAB in the buffer, */
            /* we will remove it.  (Note that this works even if the */
            /* buffer is turned off). */
            KeyboardBufferGet() ;
        } else {
            /* Not a TAB key combination.  Let's see if we have an */
            /* active focus. */
            object = p_group->list[p_group->activeObject] ;

            if (object != UI_OBJECT_BAD)  {
                /* Yes, there is an active object. */
                /* We need to send it an appropriate message. */
                /* Choose the type of message to send. */
                switch(event)  {
                    case KEYBOARD_EVENT_PRESS:
                        uiEvent = UI_EVENT_KEY_START ;
                        break ;
                    case KEYBOARD_EVENT_RELEASE:
                        uiEvent = UI_EVENT_KEY_END ;
                        break ;
                    case KEYBOARD_EVENT_BUFFERED:
                        uiEvent = UI_EVENT_KEY_BUFFERED ;
                        break ;
                }
                /* Send out the message to the active focus. */
                usedKey = IUIObjectHandleEvent(object, uiEvent, key, 0) ;
            } else {
                /* Note that the key was not used. */
                usedKey = FALSE ;
            }

            /* If the key was never used */
            /* remove it from the keyboard buffer. */
            if (usedKey == FALSE)
                KeyboardBufferGet() ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IUIObjectHandleEvent               * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    All events sent to an object group IUIObjectHandleEvent.  This        */
/*  routine checks to see if the given object can handle the given event.   */
/*  If no event handler is found, the event is discarded and ignored.       */
/*  If there is, the handler is called with the given event.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to have event processed         */
/*                                                                          */
/*    T_UIEvent event             -- UI event to process                    */
/*                                                                          */
/*    T_word16 data1              -- Accessory data.  Usually mouse X coord */
/*                                                                          */
/*    T_word16 data2              -- Accessory data.  Usually mouse Y coord */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- Flag to tell if event was processed.   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Object's ui event handler (if defined).                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

static E_Boolean IUIObjectHandleEvent(
                    T_UIObject object,
                    E_UIEvent event,
                    T_word16 data1,
                    T_word16 data2)
{
    T_UIObjectStruct *p_object ;
    E_Boolean returnStatus ;

    DebugRoutine("IUIObjectHandleEvent") ;
    DebugCheck(object != UI_OBJECT_BAD) ;
    DebugCheck(event < UI_EVENT_UNKNOWN) ;

    /* Get a pointer to the object structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Check to see if this object has a UI handler. */
    if (p_object->uiEventHandler != NULL)  {
        /* If there exists an event, go ahead and pass the event along. */
        returnStatus = p_object->uiEventHandler(object, event, data1, data2) ;
    }

    DebugCheck(returnStatus < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return returnStatus ;
}

/****************************************************************************/
/*  Routine:  IUIObjectDestroy                   * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIObjectDestory will finish out and destroy an object that is        */
/*  no longer is needed.  Note that it passes the UI_EVENT_DESTROY event    */
/*  to the object before destruction.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Note:  The reason that this routine is internal is that it is         */
/*           ONLY called by UIGroupDestroy.  If you have an object that     */
/*           you need to destroy and not a member of a group, you have      */
/*           done something very wrong or against protocol.                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object                                                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IUIObjectHandleEvent                                                  */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/21/94  Created                                                */
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

static T_void IUIObjectDestroy(T_UIObject object)
{
    T_UIObjectStruct *p_object ;

    DebugRoutine("IUIObjectDestroy") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* First, send a destroy message to the object before actually */
    /* destorying it. */
    IUIObjectHandleEvent(object, UI_EVENT_DESTROY, 0, 0) ;

    /* Get a pointer to the object structure. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Ok, now we can destroy the object.  This is done by freeing it. */
    MemFree(p_object) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IUIObjectIsInside                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIObjectIsInside is used to determine if a passed coordinated        */
/*  is inclusively located inside of the given object.  This is typically   */
/*  used by a UI Group to determine which object the mouse is over.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_UIObject object           -- Object to check if inside              */
/*                                                                          */
/*    T_word16 x                  -- X Coordinate to check                  */
/*                                                                          */
/*    T_word16 y                  -- Y Coordinate to check                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE  = coordinate is inside           */
/*                                   FALSE = coordinate is outside          */
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
/*    LES  11/23/94  Removed need for UIObjectGetAddOn by moving pointers.  */
/*                                                                          */
/****************************************************************************/

static E_Boolean IUIObjectIsInside(T_UIObject object, T_word16 x, T_word16 y)
{
    E_Boolean inside ;
    T_UIObjectStruct *p_object ;

    DebugRoutine("IUIObjectIsInside") ;
    DebugCheck(object != UI_OBJECT_BAD) ;

    /* Get a pointer to the object. */
    p_object = ((T_UIObjectStruct *)object)-1 ;

    /* Assume that we are inside.  However, if any of the following */
    /* conditions fail, we are outside. */
    inside = TRUE ;
    if ((x < p_object->left) || (x > p_object->right) ||
            (y < p_object->top) || (y > p_object->bottom))
        inside = FALSE ;

    DebugCheck(inside < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return inside ;
}

/****************************************************************************/
/*  Routine:  IUIGroupChangeFocus                * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUIGroupChangeFocus changes the focus of the active object in the     */
/*  currently active group.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 indexObject       -- Number of item in current group        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IUIObjectHandleEvent                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/25/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IUIGroupChangeFocus(T_sword16 indexObject)
{
    T_UIGroupStruct *p_group ;

    DebugRoutine("IUIGroupChangeFocus") ;
    DebugCheck(G_ActiveUIGroup != UI_GROUP_BAD) ;

    /* Get a pointer to the currently active ui group. */
    p_group = G_ActiveUIGroup ;
    DebugCheck(indexObject < p_group->numInList) ;

    /* Check if the new focus is different from the old one */
    /* (even if the "new focus" or "old focus" is invalid). */
    if (indexObject != p_group->activeObject)  {
        /* Different focus, let's change. */
        /* Check if there was an old focus */
        if (p_group->activeObject != GROUP_FOCUS_BAD)  {
            /* There was an old focus, therefore, send it a */
            /* focus lost event message. */
            IUIObjectHandleEvent(
                p_group->list[p_group->activeObject],
                UI_EVENT_LOST_FOCUS, 0, 0) ;
        }

        /* Check to see if we have a new focus. */
        if (indexObject != GROUP_FOCUS_BAD)  {
            /* Since we do, send it a focus gained message. */
            IUIObjectHandleEvent(
                p_group->list[indexObject],
                UI_EVENT_GAINED_FOCUS, 0, 0) ;

            /* Also note that this is the last valid focus. */
            p_group->lastFocus = indexObject ;
        }
        /* No matter where we are, */
        /* declare this object as the active object. */
        p_group->activeObject = indexObject ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  UI.C                                                    */
/****************************************************************************/
