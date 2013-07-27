/****************************************************************************/
/*    FILE:  UITEXT.H                                                       */
/****************************************************************************/
#ifndef _UITEXT_H_
#define _UITEXT_H_

#include "UI.H"

#define UI_TEXT_MAX_NUMBER_OF_LINES     250
#define UI_TEXT_BAD                     NULL
#define UI_TEXT_MAX_CHARACTERS_PER_LINE 128
#define UI_TEXT_MAX_NUMBER_TABS         8

typedef T_void *T_UIText ;

typedef enum {
    TEXT_MODE_EDIT,
    TEXT_MODE_VIEW,
    TEXT_MODE_SELECT,
    TEXT_MODE_UNKNOWN
} E_UITextMode ;

T_UIText UITextCreate(
             T_UIGroup group,
             T_word16 left,
             T_word16 top,
             T_word16 right,
             T_word16 bottom,
             T_word16 maxNumberOfLines,
             E_UITextMode textMode,
             T_resource font,
             T_screen backgroundScreen,
             T_color textColor,
             T_color backgroundColor,
             T_color selectedTextColor,
             T_color selectedTextBackgroundColor) ;

T_void UITextDeleteAll(T_UIText uiText) ;

T_void UITextAppend(T_UIText uiText, T_byte8 *string) ;

T_void UITextSetTab(T_UIText uiText, T_word16 tabPosition) ;

T_void UITextScrollDown(T_UIText uiText) ;

T_void UITextScrollUp(T_UIText uiText) ;

T_void UITextGotoTop(T_UIText uiText) ;

T_void UITextGotoEnd(T_UIText uiText) ;

#endif

/****************************************************************************/
/*    END OF FILE:  UITEXT.H                                                */
/****************************************************************************/
