/****************************************************************************/
/*    FILE:  UIBUTTON.H                                                     */
/****************************************************************************/
#ifndef _UIBUTTON_H_
#define _UIBUTTON_H_

#include "BUTTON.H"
#include "UI.H"

typedef T_UIObject T_UIButton ;

typedef T_void (*T_buttonHandler)(T_UIButton button) ;

#define UI_BUTTON_BAD UI_OBJECT_BAD

T_UIButton UIButtonCreate(
               T_UIGroup group,
               T_resource targetPic,
               T_resource notTargetPic,
               T_resource pressedPic,
               T_word16 x,
               T_word16 y) ;

T_void UIButtonSetHandler(
           T_UIButton button,
           T_buttonHandler buttonHandler) ;

T_void UIButtonSetAccessoryData(T_UIButton button, T_word32 data) ;

T_word32 UIButtonGetAccessoryData(T_UIButton button) ;

#endif

/****************************************************************************/
/*    END OF FILE:  UIBUTTON.H                                              */
/****************************************************************************/
