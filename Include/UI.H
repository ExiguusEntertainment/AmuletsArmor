/****************************************************************************/
/*    FILE:  UI.H                                                           */
/****************************************************************************/
#ifndef _UI_H_
#define _UI_H_

#include "GENERAL.H"
#include "GRAPHICS.H"
#include "RESOURCE.H"

#define UI_GROUP_BAD NULL
#define UI_OBJECT_BAD NULL

typedef enum {
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_START,
    UI_EVENT_MOUSE_END,
    UI_EVENT_MOUSE_DRAG,
    UI_EVENT_MOUSE_HELD,
    UI_EVENT_MOUSE_IDLE,
    UI_EVENT_KEY_START,
    UI_EVENT_KEY_END,
    UI_EVENT_KEY_BUFFERED,
    UI_EVENT_DRAW,
    UI_EVENT_GAINED_FOCUS,
    UI_EVENT_LOST_FOCUS,
    UI_EVENT_DESTROY,
    UI_EVENT_UNKNOWN
} E_UIEvent ;

typedef T_void *T_UIGroup ;

typedef T_void *T_UIObject ;

typedef E_Boolean (*T_uiEventHandler)(T_UIObject object,
                                      E_UIEvent event,
                                      T_word16 data1,
                                      T_word16 data2) ;

/* General UI: */

T_void UISetActiveGroup(T_UIGroup group) ;


/* UI Group: */

T_void UIGroupAttachUIObject(T_UIGroup group, T_UIObject object) ;

T_UIGroup UIGroupCreate(T_void) ;

T_void UIGroupDestroy(T_UIGroup group) ;

T_void UIGroupDraw(T_void) ;

T_void UIGroupSetScreen(T_UIGroup group, T_screen screen) ;

T_void UIGroupSetBackground(T_UIGroup group, T_resource background) ;


/* UI Object: */

T_UIObject UIObjectCreate(T_word16 size) ;

T_void UIObjectSetArea(
           T_UIObject object,
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom) ;

T_word16 UIObjectGetLeftPosition(T_UIObject object) ;

T_word16 UIObjectGetTopPosition(T_UIObject object) ;

T_word16 UIObjectGetRightPosition(T_UIObject object) ;

T_word16 UIObjectGetBottomPosition(T_UIObject object) ;

   /* Only use this if you are doing something special: */
T_void UIObjectSetEventHandler(
           T_UIObject object,
           T_uiEventHandler uiEventHandler) ;

#endif

/****************************************************************************/
/*    END OF FILE:  UI.H                                                    */
/****************************************************************************/
