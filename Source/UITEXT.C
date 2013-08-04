/*-------------------------------------------------------------------------*
 * File:  UITEXT.C
 *-------------------------------------------------------------------------*/
/**
 * DEPRECATED!
 *
 * @addtogroup UITEXT
 * @brief User Interface for Text
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "MEMORY.H"
#include "MOUSEMOD.H"
#include "UITEXT.H"

typedef struct {
    T_word16 index ;
    T_word16 length ;
    E_Boolean changed ;
    T_word16 startSelect ;
    T_word16 endSelect ;
} T_UITextLine ;

typedef struct {
    T_byte8 *p_data ;
    T_word16 maxSize ;
    T_word16 totalSize ;
    E_UITextMode textMode ;
    T_resource font ;
    T_word16 fontHeight ;
    T_screen backgroundScreen ;
    T_color textColor ;
    T_color backgroundColor ;
    T_color selectedTextColor ;
    T_color selectedTextBackgroundColor ;
    T_UITextLine lines[UI_TEXT_MAX_NUMBER_OF_LINES] ;
    T_word16 width ;
    T_byte8 numberLines ;
    T_byte8 numberLinesInView ;
    T_byte8 topLine ;
    T_byte8 cursorLine ;
    T_byte8 cursorIndex ;
    T_word16 tabPositions[UI_TEXT_MAX_NUMBER_TABS] ;
    T_byte8 numberTabs ;
    E_Boolean cursorOn ;
    T_byte8 endSelectLine ;
    T_byte8 endSelectIndex ;
} T_UITextStruct ;

/* Internal routine prototypes: */
static T_void IUITextDrawChanges(T_UIText uiText) ;

static T_void IUITextDrawAll(T_UIText uiText) ;

static T_void IUITextAppendCharacter(T_UIText uiText, T_byte8 character) ;

static T_void IUITextDrawLine(T_UIText uiText, T_byte8 lineIndex) ;

static T_word16 IUITextCalculateOffsetPosition(
                    T_UIText uiText,
                    T_word16 indexLine,
                    T_word16 charIndex) ;

static T_word16 IUIGetClosestTabPosition(T_UIText uiText, T_word16 offset) ;

static E_Boolean IUITextStandardEventHandler(
              T_UIObject object,
              E_UIEvent event,
              T_word16 data1,
              T_word16 data2) ;

static T_void IUITextPositionCursor(
                  T_UIText uiText,
                  T_word16 posX,
                  T_word16 posY) ;

static T_word16 IUITextCalculateCursorInLine(
                    T_UIText uiText,
                    T_word16 line,
                    T_word16 posX) ;

static T_void IUITextDragCursor(
                  T_UIText uiText,
                  T_word16 posX,
                  T_word16 posY) ;

static T_void IUITextSelectArea(T_UIText uiText) ;

static T_void IUITextSelectNone(T_UIText uiText) ;

static T_void IUITextClipCoordinates(
                  T_UIText uiText,
                  T_word16 *posX,
                  T_word16 *posY) ;

static T_void IUITextInsertKeystroke(T_UIText uiText, T_byte8 character) ;

static T_void IUITextInsertCharacter(T_UIText uiText, T_byte8 character) ;

static T_void IUITextMoveCursorRight(T_UIText uiText) ;

static T_void IUITextMoveCursorLeft(T_UIText uiText) ;

static T_word16 IUITextGetCharacterWidth(
                    T_UIText uiText,
                    T_word16 offset,
                    T_byte8 character) ;

static T_void IUITextFormat(T_UIText uiText) ;

static T_void IUITextMoveCursorToIndex(
                  T_UIText uiText,
                  T_word16 cursorIndex) ;

static T_word16 IUITextFindLineNumber(
                    T_UIText uiText,
                    T_word16 index) ;

static T_void IUITextBackspace(T_UIText uiText) ;

/*-------------------------------------------------------------------------*
 * Routine:  UITextCreate
 *-------------------------------------------------------------------------*/
/**
 *  UITextCreate will do everything that needs to be done to create
 *  and set up a ui text object that will be used in a UI Group.  Just
 *  pass it a long list of parameters that declare what type of text object
 *  you want.  From this point on, all the options will be in full force.
 *
 *  @param group -- UI Group to add ui text to.
 *  @param left -- Left border of text.
 *  @param top -- Top border of text.
 *  @param right -- Right border of text.
 *  @param bottom -- Bottom border of text.
 *  @param maxSize -- Tells what is the maximum number of
 *      characters allowed in the text region.
 *  @param textMode -- Choose between text you can edit,
 *      text to view, or text where each
 *      line is selectable.
 *  @param font -- Font text will be in.
 *  @param backgroundScreen -- Background picture to use when
 *      characters are erased.  If you specify
 *      NULL, no background will be used.
 *  @param textColor -- Color of the text.
 *  @param backgroundColor -- Color of the background.  Use
 *      COLOR_CLEAR if you are using a
 *      background screen.
 *  @param selectedTextColor -- Color of text that is selected.
 *      T_color selectedTextBackgroundColor
 *  @param selectedTextBackgroundColor -- Color of background when text is
 *      selected.  Pass COLOR_CLEAR if you do
 *      not want a background selected color.
 *
 *  @return Create ui text object is returned.
 *
 *<!-----------------------------------------------------------------------*/
T_UIText UITextCreate(
             T_UIGroup group,
             T_word16 left,
             T_word16 top,
             T_word16 right,
             T_word16 bottom,
             T_word16 maxSize,
             E_UITextMode textMode,
             T_resource font,
             T_screen backgroundScreen,
             T_color textColor,
             T_color backgroundColor,
             T_color selectedTextColor,
             T_color selectedTextBackgroundColor)
{
    T_UITextStruct *p_text ;
    T_UIText uiText ;
    T_word16 i ;
    T_bitfont *p_font ;

    DebugRoutine("UITextCreate") ;
    DebugCheck(group != UI_GROUP_BAD) ;
    DebugCheck(right < SCREEN_SIZE_X) ;
    DebugCheck(left < right) ;
    DebugCheck(bottom < SCREEN_SIZE_Y) ;
    DebugCheck(top < bottom) ;
    DebugCheck(textMode < TEXT_MODE_UNKNOWN) ;
    DebugCheck(font != RESOURCE_BAD) ;

    /* Create the button by createing a object with an add on size */
    /* equal to the structure we need to store. */
    uiText = UIObjectCreate(sizeof(T_UITextStruct)) ;

    /* Get a pointer to the structure of a ui text object. */
    p_text = uiText ;

    DebugCheck(uiText != UI_OBJECT_BAD) ;

    /* Make sure we could allocate that object.  If not, don't use it. */
    if (uiText != UI_OBJECT_BAD)  {
        /* Allocate the memory for the text data. */
        p_text->p_data = MemAlloc(maxSize) ;

        DebugCheck(p_text->p_data != NULL) ;

        /* Check to see that we could allocate the memory. */
        if (p_text->p_data == NULL)  {
            /* If we can't, return a bad response. */
            uiText = UI_TEXT_BAD ;
        } else {
            /* Set up the area that the ui text will occupy. */
            UIObjectSetArea(uiText, left, top, right, bottom) ;

            /* Set up the standard ui text event handler. */
            UIObjectSetEventHandler(uiText, IUITextStandardEventHandler) ;

            /* Record all the pertinent information about this UI text object. */
            p_text->maxSize = maxSize;
            p_text->textMode = textMode ;
            p_text->font = font ;
            p_text->textColor = textColor ;
            p_text->backgroundColor = backgroundColor ;
            p_text->backgroundScreen = backgroundScreen ;
            p_text->selectedTextColor = selectedTextColor ;
            p_text->selectedTextBackgroundColor = selectedTextBackgroundColor ;

            /* Determine how many lines can be displayed given the particular, */
            /* font and height of the ui text area. */
            p_font = ResourceLock(font) ;

            /* Also, record the font height so we don't have to keep locking */
            /* the resource to get a the font height. */
            p_text->fontHeight = p_font->height ;

            p_text->numberLinesInView = (1+bottom-top)/p_text->fontHeight ;
            ResourceUnlock(font) ;

            /* Initialize the internal data elements: */
            p_text->totalSize = 0 ;
            p_text->cursorLine = 0 ;
            p_text->cursorIndex = 0 ;
            p_text->numberTabs = 0 ;
            p_text->topLine = 0 ;
            p_text->cursorOn = FALSE ;

            /* Calculate the width of the text area: */
            p_text->width = 1+right-left ;

            /* You always have one line to edit: */
            p_text->numberLines = 1 ;

            /* Each of the lines now have to be initialized. */
            for (i=0; i<UI_TEXT_MAX_NUMBER_OF_LINES; i++)  {
                /* Note that the offset into the text is zero. */
                p_text->lines[i].index = 0 ;

                /* Make the length zero too. */
                p_text->lines[i].length = 0 ;

                /* No selection is declared on any of the lines. */
                p_text->lines[i].startSelect = 0 ;
                p_text->lines[i].endSelect = 0 ;

                /* Declare the text object as not changed. */
                p_text->lines[i].changed = FALSE ;
            }

            /* Now that we have the whole ui text created, let's add it */
            /* to the UI Group. */
            UIGroupAttachUIObject(group, uiText) ;
        }
    }
    DebugEnd() ;

    return uiText ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextDeleteAll
 *-------------------------------------------------------------------------*/
/**
 *  UITextDeleteAll will delete all text from the current UI Text object.
 *  It will draw the differences on the screen as well.
 *
 *  @param uiText -- Text to have all deleted from.
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextDeleteAll(T_UIText uiText)
{
    T_word16 i ;
    T_UITextStruct *p_text ;

    DebugRoutine("UITextDeleteAll") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Go through all the text lines and make them point to nothing. */
    for (i=0; i<UI_TEXT_MAX_NUMBER_OF_LINES; i++)  {
        /* Note that the offset into the text is zero. */
        p_text->lines[i].index = 0 ;

        /* Make the length zero too. */
        p_text->lines[i].length = 0 ;

        /* Obviously we have changed something. */
        p_text->lines[i].changed = TRUE ;
    }

    /* Declare the length of the text to be zero. */
    p_text->totalSize = 0 ;

    /* Home the cursor */
    p_text->cursorLine = 0 ;
    p_text->cursorIndex = 0 ;

    /* Home the selection. */
    p_text->endSelectLine = 0 ;
    p_text->endSelectIndex = 0 ;

    /* Have the program redraw all the lines and note the changes. */
    IUITextDrawChanges(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextAppend
 *-------------------------------------------------------------------------*/
/**
 *  UITextAppend adds a given string to the end of the current.
 *  Any text that is past the end of the allowed range is lost.
 *
 *  @param uiText -- UI Text object to append text onto.
 *  @param string -- String of characters to append
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextAppend(T_UIText uiText, T_byte8 *string)
{
    DebugRoutine("UITextAppend") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;
    DebugCheck(string != NULL) ;

    /* Really this routine will seem quite simple, we just append each */
    /* character in the string one at a time. */
    while (*string != '\0')
        IUITextAppendCharacter(uiText, *(string++)) ;

    /* Now that the text has been added, we need to format the text. */
    IUITextFormat(uiText) ;

    /* Now that everything has been appended, let's draw the changes. */
    IUITextDrawChanges(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextSetTab
 *-------------------------------------------------------------------------*/
/**
 *  UITextSetTab is used to declare the position of tabs within a
 *  text line (actually, all text lines in a ui text), so that when a tab
 *  character (0x9) is found, the characters move to tab location.
 *
 *  NOTE: 
 *  In the debuging version, trying to create more than 8 tabs will bomb.
 *  Also, all tabs should be defined before adding text or being drawn.
 *  If you don't, unpredictable results may occur.
 *
 *  @param uiText -- Text object to get tab
 *  @param tabPosition -- Tab position from left of object
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextSetTab(T_UIText uiText, T_word16 tabPosition)
{
    T_UITextStruct *p_text ;
    T_word16 i, oldTab, tab ;

    DebugRoutine("UITextSetTab") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check the current number of tabs. */
    DebugCheck(p_text->numberTabs != UI_TEXT_MAX_NUMBER_TABS) ;

    /* First check to see if the tab is already existing. */
    for (i=0; i<p_text->numberTabs; i++)
        if (p_text->tabPositions[i] == tabPosition)
            tabPosition = 0 ;

    /* If there was a tabPosition found that is identical, tabPosition */
    /* now equals zero (if it doesn't already).  If not, go ahead and */
    /* do an insert sort for the new tab. */
    if (tabPosition != 0)  {
        /* Hold in left hand the tab we are trying to place. */
        oldTab = tabPosition ;

        for (i=0; i<p_text->numberTabs; i++)  {
            /* Put in right hand the tab currently in the list. */
            tab = p_text->tabPositions[i] ;

            /* See if left hand is a tab position more to the left */
            /* than the tab in the right hand. */
            if (oldTab < tab)  {
                /* If it is, replace tab in list with tab in left */
                /* hand, */
                p_text->tabPositions[i] = oldTab ;

                /* and put tab in right hand into left hand. */
                oldTab = tab ;
            }
        }
        /* Loop until all tabs in the list have been checked and we */
        /* are left with a tab in our left hand.  This tab must be */
        /* one that goes on the end.  Since i is equal to the number */
        /* of tabs now, place this left over tab. */
        p_text->tabPositions[i] = oldTab ;

        /* Increment the number of tabs now available. */
        p_text->numberTabs++ ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextScrollDown
 *-------------------------------------------------------------------------*/
/**
 *  UITextScrollDown will scroll the display down one line (if it can).
 *  It will always try to leave one line on the display.
 *
 *  @param uiText -- Text object to have scrolled.
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextScrollDown(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("UITextScrollDown") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check if the top line is at the bottom of the written text or what. */
    if (p_text->topLine+p_text->numberLinesInView < (p_text->numberLines-1))  {
        /* If it is not, change the top line of the display. */
        p_text->topLine++ ;

        /* Since the whole thing has changed on the screen, just draw */
        /* the whole text object over. */
        IUITextDrawAll(uiText) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextScrollUp
 *-------------------------------------------------------------------------*/
/**
 *  UITextScrollUp will scroll the display up one line (if it can).
 *
 *  @param uiText -- Text object to have scrolled.
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextScrollUp(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("UITextScrollUp") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check if the top line is at the top. */
    if (p_text->topLine > 0)  {
        /* If it is not, change the top line of the display. */
        p_text->topLine-- ;

        /* Since the whole thing has changed on the screen, just draw */
        /* the whole text object over. */
        IUITextDrawAll(uiText) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextGotoTop
 *-------------------------------------------------------------------------*/
/**
 *  UITextGotoTop will home the given text ui so that the top of the
 *  document is being shown.
 *
 *  @param uiText -- Text object to go to the top.
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextGotoTop(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("UITextGotoTop") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check if the top line is at the top. */
    if (p_text->topLine > 0)  {
        /* If it is not, change the top line of the display so the */
        /* display is homed */
        p_text->topLine = 0 ;

        /* Since the whole thing has changed on the screen, just draw */
        /* the whole text object over. */
        IUITextDrawAll(uiText) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  UITextGotoEnd
 *-------------------------------------------------------------------------*/
/**
 *  UITextGotoEnd will move the display to the very end and show text
 *  as much of the text as possible.
 *
 *  @param uiText -- Text object to go to the top.
 *
 *<!-----------------------------------------------------------------------*/
T_void UITextGotoEnd(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("UITextGotoEnd") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check if the bottom is off the screen. */
    if (p_text->numberLines > p_text->topLine + p_text->numberLinesInView)  {
        /* If it is, change the top line of the display so the */
        /* display is at the end. */
        p_text->topLine = p_text->numberLines-p_text->numberLinesInView ;

        /* Since the whole thing has changed on the screen, just draw */
        /* the whole text object over. */
        IUITextDrawAll(uiText) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextDrawAll
 *-------------------------------------------------------------------------*/
/**
 *  IUITextDrawAll marks all the lines in a UI Text object as being
 *  changed (even it it was not) and then makes a call to
 *  IUITextDrawChanges.
 *
 *  @param uiText -- Text object to draw
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextDrawAll(T_UIText uiText)
{
    T_word16 i ;
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextDrawAll") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Go through all the text lines and mark them as changed. */
    for (i=0; i<UI_TEXT_MAX_NUMBER_OF_LINES; i++)
        p_text->lines[i].changed = TRUE ;

    /* Have the program redraw all the lines. */
    IUITextDrawChanges(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextDrawChanges
 *-------------------------------------------------------------------------*/
/**
 *  IUITextDrawChanges searches the ui text object given for changes
 *  that have occured and calls IUITextDrawLine for each line that has
 *  been changed.
 *
 *  @param uiText -- Text object to have changes drawn
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextDrawChanges(T_UIText uiText)
{
    T_UITextStruct *p_text ;
    T_word16 i ;

    DebugRoutine("IUITextDrawChanges") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Several changes are going to be made, make the mouse disappear. */
    MouseHide() ;

    /* Loop through all the lines and find lines that have been */
    /* changed. */
    for (i=0; i<UI_TEXT_MAX_NUMBER_OF_LINES; i++)
        if (p_text->lines[i].changed == TRUE)
            IUITextDrawLine(uiText, (T_byte8)i) ;

    /* Done with changes. */
    MouseShow() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextDrawLine
 *-------------------------------------------------------------------------*/
/**
 *  IUITextDrawLine does the brute work of determining if a line is
 *  on the screen, and if it is, draws the whole line and text inside
 *  it.
 *
 *  @param uiText -- Text object to draw line from
 *  @param lineIndex -- Line to draw within text object
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextDrawLine(T_UIText uiText, T_byte8 lineIndex)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_word16 left ;
    T_word16 top ;
    T_word16 right ;
    T_word16 bottom ;
    T_bitfont *p_font ;
    T_word16 selectPixelLeft ;
    T_word16 selectPixelRight ;
    T_word16 i ;
    T_word16 offset ;
    T_color charColor ;

    DebugRoutine("IUITextDrawLine") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;
    DebugCheck(lineIndex < UI_TEXT_MAX_NUMBER_OF_LINES) ;

    /* We need to make sure the mouse is hidden if it is not already. */
    MouseHide() ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Doesn't matter if line is on screen or not, go ahead */
    /* and mark the line as drawn (we'll get to it in a moment). */
    p_text->lines[lineIndex].changed = FALSE ;

    /* Check to see if the line is on the screen. */
    if ((lineIndex >= p_text->topLine) &&
        (lineIndex < (p_text->topLine + p_text->numberLinesInView)))  {
        /* If the line is on the screen, we need to draw it. */
        /* Lock in the font. */
        p_font = ResourceLock(p_text->font) ;

        /* Declare this font to be the active font. */
        GrSetBitFont(p_font) ;

        /* Compute the location on the screen for this line. */

        /* The top is a number of font height multiples down the */
        /* screen. */
        top = UIObjectGetTopPosition(uiText) +
              (lineIndex - p_text->topLine) * p_text->fontHeight ;

        /* The bottom is a font height down (minus one since all the */
        /* drawing routines are inclusive. */
        bottom = top + p_font->height - 1 ;

        /* The left and the right are in the same places for all */
        /* lines. */
        left = UIObjectGetLeftPosition(uiText) ;
        right = UIObjectGetRightPosition(uiText) ;

        /* Let's draw the background first. */
        /* Two different options are allowed: */
        /*    1) Background screen is background of text. */
        /*    2) Background is a solid color. */
        /* Check for which one: */
        if (p_text->backgroundScreen == NULL)  {
            /* No background screen is defined, therefore, we will */
            /* draw a solid colored background. */
            GrDrawRectangle(
                left,
                top,
                right,
                bottom,
                p_text->backgroundColor) ;
        } else {
            /* A background screen is defined, we will transfer */
            /* the background screen to the active screen. */
            GrScreenSet(p_text->backgroundScreen) ;
            GrTransferRectangle(
                GRAPHICS_ACTUAL_SCREEN,
                left,
                top,
                right,
                bottom,
                left,
                top) ;
            GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
        }

        /* Until now, we didn't need any particular line information. */
        /* Now we do.  Get a pointer to the particular line we are */
        /* processing. */
        p_line = &p_text->lines[lineIndex] ;

        /* Now that the background is drawn, let's check to see */
        /* if we need to draw a selected area. */
        if (p_line->startSelect != p_line->endSelect)  {
            /* The start and end is different, thus signifying a selected */
            /* area.  Let's see if the whole line is selected. */
            if ((p_line->startSelect == 0) &&
                (p_line->endSelect >= p_line->length))  {
                /* The whole line is selected, let's draw the background */
                /* selection color over the whole line. */
                GrDrawRectangle(
                    left,
                    top,
                    right,
                    bottom,
                    p_text->selectedTextBackgroundColor) ;
            } else {
                /* Only part of the line is selected, let's see if we can */
                /* calculate the region used. */

                /* Let's check the left side.  */
                if (p_line->startSelect == 0)  {
                    /* If the left is 0, we already know without */
                    /* calculating where the left pixel is located */
                    selectPixelLeft = left ;
                } else {
                    /* Otherwise, we need to do a little calculation. */
                    selectPixelLeft = left + IUITextCalculateOffsetPosition(
                                                 uiText,
                                                 lineIndex,
                                                 p_line->startSelect) ;
                }

                /* Now let's calculate the right side.  Check to see */
                /* if the right half of the line is fully selected. */
                if (p_line->endSelect >= p_line->length)  {
                    /* We're all the way over, so make the ending */
                    /* equal the right edge of the ui text object. */
                    selectPixelRight = right ;
                } else {
                    /* Let's calculate the right position. */
                    selectPixelRight = left + IUITextCalculateOffsetPosition(
                                                 uiText,
                                                 lineIndex,
                                                 p_line->endSelect) ;
                }

                /* Now that we know where the left and right of the */
                /* selected area is, fill it. */
                GrDrawRectangle(
                    selectPixelLeft,
                    top,
                    selectPixelRight-1,
                    bottom,
                    p_text->selectedTextBackgroundColor) ;
            }
        }

        /* Before we draw the text, let's draw the cursor.  Although */
        /* the cursor is considered behind the text, it should be */
        /* between the characters at all times. */

        /* Check to see if we are in edit mode. */
        if (p_text->textMode == TEXT_MODE_EDIT)  {
            /* We are, therefore, we should draw a cursor. */
            /* Check to see if the cursor is located on this line and if */
            /* the cursor is allowed to be on. */
            if ((p_text->cursorLine == lineIndex) &&
                (p_text->cursorOn == TRUE))  {
                /* Yes, the cursor is on this line. */
                /* Calculate the location within the line. */
                offset = left+IUITextCalculateOffsetPosition(
                                    uiText,
                                    lineIndex,
                                    p_text->cursorIndex -
                                    p_text->lines[lineIndex].index) ;

                /* Now draw the cursor in the selection background */
                /* color. */
                GrDrawVerticalLine(
                    offset-1,
                    top,
                    bottom,
                    p_text->selectedTextBackgroundColor) ;
            }
        }

        /* We are finally ready to draw the text.  Let's draw the text. */
        /* Start by declaring where the text is to be drawn. */
        /* Leave one pixel for the cursor. */
        GrSetCursorPosition(left+1, top) ;

        /* Loop through the line and draw each character individually. */
        for (i=0; i<p_line->length; i++)  {
            /* Check to see if the character is a tab. */
            if (p_text->p_data[i+p_line->index] == '\t')  {
                /* It is a tab, so we need to move over a bit. */
                offset = left + IUITextCalculateOffsetPosition(
                                    uiText,
                                    lineIndex,
                                    i+1) ;
                /* Now that we know where the next character starts */
                /* lets move over there. */
                GrSetCursorPosition(offset, top) ;
            } else {
                /* It is not a tab, therefore we can draw it line normal. */
                /* But first we must determine what color to make it. */
                /* If the character is in the selection range, we need */
                /* to draw it in the selected color. */
                if ((i >= p_line->startSelect) &&
                    (i < p_line->endSelect))  {
                    /* If the character is within a selection area, */
                    /* choose the selection color. */
                    charColor = p_text->selectedTextColor ;
                } else {
                    /* If not in a selection area, use the regular */
                    /* text color. */
                    charColor = p_text->textColor ;
                }

                /* Got the character and the color, draw it. */
                /* But if it is a newline character, don't draw it. */
                if (p_text->p_data[i+p_line->index] != '\n')
                    GrDrawCharacter(
                        p_text->p_data[i+p_line->index],
                        charColor) ;
            }
        }

        /* We are now done, release the font. */
        ResourceUnlock(p_text->font) ;
    }

    /* Return the mouse to its previous hidden state. */
    MouseShow() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextCalculateOffsetPosition
 *-------------------------------------------------------------------------*/
/**
 *  IUITextCalculateOffsetPosition is used to determine how far from the
 *  left border in pixels is a certain location of a character in a
 *  particular text line.
 *
 *  NOTE: 
 *  This routine assumes that the currently active font is the font
 *  that the ui text object is using.
 *
 *  @param uiText -- Text object to find position within
 *  @param lineIndex -- Line to find position within
 *  @param charIndex -- Character index from start of line
 *      to find position of.
 *
 *  @return Number of pixels from left edge
 *      til reaches the character.
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IUITextCalculateOffsetPosition(
                    T_UIText uiText,
                    T_word16 lineIndex,
                    T_word16 charIndex)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_word16 offset ;
    T_word16 i ;

    DebugRoutine("IUITextCalculateOffsetPosition") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;
    DebugCheck(lineIndex < UI_TEXT_MAX_NUMBER_OF_LINES) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Get a pointer to the actual line in the text. */
    p_line = &p_text->lines[lineIndex] ;

    /* Start the offset at 1.  This leaves room for the cursor */
    /* in front of the line. */
    offset = 1 ;

    /* Loop through all the characters until we reach the character */
    /* index. */
    for (i=0; i<charIndex; i++)  {
        /* Move over for each character (special or whatnot) */
        offset += IUITextGetCharacterWidth(
                      uiText,
                      offset,
                      p_text->p_data[p_line->index+i]) ;
    }

    DebugEnd() ;

    return offset ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUIGetClosestTabPosition
 *-------------------------------------------------------------------------*/
/**
 *  IUIGetClosestTabPosition uses the given pixel location from the
 *  left of a uiText object to determine where to tab over to.
 *
 *  @param uiText -- Text object to find tab within
 *  @param offset -- Number of pixels already over from
 *      left.
 *
 *  @return Position of next tab.  If 0, there
 *      was no next tab.
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IUIGetClosestTabPosition(T_UIText uiText, T_word16 offset)
{
    T_word16 i ;
    T_UITextStruct *p_text ;
    T_word16 position ;

    DebugRoutine("IUIGetClosestTabPosition") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Go through all the tabs and find the tab that just goes over */
    /* the given offset. */
    for (i=0; i<p_text->numberTabs; i++)  {
        /* If we have found a tab that is further over than where */
        /* we are currently, then that is the next tab. */
        if (p_text->tabPositions[i] > offset)
            break ;
    }

    /* Check to see if we found a new tab. */
    if (i != p_text->numberTabs)  {
        /* Found a tab, use it. */
        position = 1+p_text->tabPositions[i] ;
    } else {
        /* No such luck.  We'll roll the line around for now. */
        /* We don't want the text to go everywhere. */
        position = 0 ;
    }

    DebugEnd() ;

    return position ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextAppendCharacter
 *-------------------------------------------------------------------------*/
/**
 *  IUITextAppendCharacter adds one character to the end of the given
 *  text object.
 *
 *  NOTE: 
 *  This routine appends the character but the character will not appear
 *  until a IUITextFormat command is issued.
 *
 *  @param uiText -- Text object to append to
 *  @param character -- Character to append to text object
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextAppendCharacter(T_UIText uiText, T_byte8 character)
{
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextAppendCharacter") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check to see if there is room for the character. */
    if (p_text->totalSize < p_text->maxSize)  {
        /* There is room.  Append the character. */
        p_text->p_data[p_text->totalSize++] = character ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextStandardEventHandler
 *-------------------------------------------------------------------------*/
/**
 *
 *  @param object -- This particular ui object/button
 *  @param event -- The event to process
 *  @param data1 -- Mouse X coordinate (not used)
 *  @param data2 -- Mouse Y coordinate (not used)
 *
 *  @return TRUE  = Processed event
 *      FALSE = Event ignored
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IUITextStandardEventHandler(
              T_UIObject object,
              E_UIEvent event,
              T_word16 data1,
              T_word16 data2)
{
    T_UITextStruct *p_text ;
    E_Boolean handled = FALSE ;

    DebugRoutine("IUITextStandardEventHandler") ;
    DebugCheck(object != UI_OBJECT_BAD) ;
    DebugCheck(event < UI_EVENT_UNKNOWN) ;

    /* First get the text part of the ui object passed in. */
    p_text = object ;

    /* OK, what event were we given? */
    switch(event)  {
        case UI_EVENT_GAINED_FOCUS:
            /* Gained the focus, make the cursor appear. */
            p_text->cursorOn = TRUE ;

            /* Make the line with the cursor get drawn. */
            p_text->lines[p_text->cursorLine].changed = TRUE ;
            IUITextDrawChanges(object) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_LOST_FOCUS:
            /* Lost the focus, make the cursor disappear. */
            p_text->cursorOn = FALSE ;

            /* Make the line with the cursor get drawn. */
            p_text->lines[p_text->cursorLine].changed = TRUE ;
            IUITextDrawChanges(object) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_MOUSE_START:
            /* If there was anything selected, we need to get rid of */
            /* it.  Get rid of it. */
            IUITextSelectNone(object) ;

            /* The mouse is wanting to move the cursor.  Let's move the */
            /* cursor to a new location based on the mouse position. */
            IUITextPositionCursor(
                 object,  /* Our text object. */
                 data1,   /* Mouse X */
                 data2) ; /* Mouse Y */

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_MOUSE_END:
            break ;
        case UI_EVENT_MOUSE_DRAG:
            IUITextDragCursor(
                object,   /* Text object */
                data1,    /* Mouse X */
                data2) ;  /* MOuse Y */
            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_DRAW:
            /* Draw the whole text object. */
            IUITextDrawAll(object) ;

            /* Note that this event was processed. */
            handled = TRUE ;
            break ;
        case UI_EVENT_KEY_BUFFERED:
            /* Something is being pressed on the keyboard, must be */
            /* a key to insert.  */
            IUITextInsertKeystroke(object, (T_byte8)data1) ;

            /* We handle all the keys */
            handled = TRUE ;
            break ;
    }

    DebugCheck(handled < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return handled ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextPositionCursor
 *-------------------------------------------------------------------------*/
/**
 *  IUITextPositionCursor moves the cursor to the given coordinates
 *  on the screen (and the given text object).
 *
 *  @param uiText -- Text object to have cursor positioned.
 *  @param posX -- Screen position X
 *  @param posY -- Screen position Y
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextPositionCursor(
                  T_UIText uiText,
                  T_word16 posX,
                  T_word16 posY)
{
    T_UITextStruct *p_text ;
    T_word16 top ;
    T_word16 newCursorLine ;
    T_word16 oldCursorLine ;

    DebugRoutine("IUITextPositionCursor") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Get the top line of the text. */
    top = UIObjectGetTopPosition(uiText) ;

    /* Calculate what line this position is at. */
    newCursorLine = p_text->topLine +
               ((posY - top) / p_text->fontHeight) ;

    DebugCheck(newCursorLine < UI_TEXT_MAX_NUMBER_OF_LINES) ;

    /* Hold onto where the cursorLine was located before. */
    oldCursorLine = p_text->cursorLine ;

    /* Change the cursorLine. */
    p_text->cursorLine = (T_byte8)newCursorLine ;

    /* Now we need to change the position of the cursor within */
    /* the actual text line.  Let's call a routine to do this. */
    p_text->cursorIndex = (T_byte8)
        IUITextCalculateCursorInLine(uiText, newCursorLine, posX) ;

    /* Now let's draw the differences.  Check to see if the new and */
    /* the old lines are different. */
    if (newCursorLine != oldCursorLine)  {
        /* If the cursor lines are different, we need to redraw both. */
        /* Inside this 'if', we'll redraw the original line. */
        IUITextDrawLine(uiText, (T_byte8)oldCursorLine) ;
    }

    /* In either case, the new line must be drawn. */
    IUITextDrawLine(uiText, (T_byte8)newCursorLine) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextDragCursor
 *-------------------------------------------------------------------------*/
/**
 *  IUITextDragCursor determines the end point of the area of text that
 *  is about to be selected and selects it.
 *
 *  @param uiText -- Text object to have cursor positioned.
 *  @param posX -- Screen position X
 *  @param posY -- Screen position Y
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextDragCursor(
                  T_UIText uiText,
                  T_word16 posX,
                  T_word16 posY)
{
    T_UITextStruct *p_text ;
    T_word16 top ;

    DebugRoutine("IUITextPositionCursor") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Get the top line of the text. */
    top = UIObjectGetTopPosition(uiText) ;

    /* Since drag x,y coordinates can be out of bounds, let */
    /* us clip them to the inside. */
    IUITextClipCoordinates(uiText, &posX, &posY) ;

    /* Calculate what line this position is at. */
    p_text->endSelectLine = p_text->topLine +
                              ((posY - top) / p_text->fontHeight) ;
    DebugCheck(p_text->endSelectLine < UI_TEXT_MAX_NUMBER_OF_LINES) ;

    /* Let's only do lines that are on the screen.  This sometimes happens */
    /* when the vertical size of the text object is not a multiple of */
    /* the font height. */
    if (p_text->endSelectLine >= p_text->topLine + p_text->numberLinesInView)
        p_text->endSelectLine-- ;

    /* Now we need to calculate where the end of the selection area */
    /* is on the particular line. */
    p_text->endSelectIndex = (T_byte8)
        IUITextCalculateCursorInLine(uiText, p_text->endSelectLine, posX) ;

    /* Finally, we need the text area to actually be selected. */
    IUITextSelectArea(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextCalculateCursorInLine
 *-------------------------------------------------------------------------*/
/**
 *  IUITextPositionCursorInLine moves the cursor to a new location
 *  in a UI text line based on a given x position and the line.
 *
 *  NOTE: 
 *  The font of the line is assumed to be the active font.
 *
 *  @param uiText -- Text object to have cursor positioned.
 *  @param line -- Text line within the text object
 *  @param posY -- Screen position Y
 *
 *  @return Character offset into the given line.
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IUITextCalculateCursorInLine(
                    T_UIText uiText,
                    T_word16 line,
                    T_word16 posX)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_word16 i ;
    T_word16 total ;
    T_word16 left ;
    T_word16 offset ;
    T_word16 width ;

    DebugRoutine("IUITextCalculateCursorInLine") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;
    DebugCheck(posX < SCREEN_SIZE_X) ;
    DebugCheck(line < UI_TEXT_MAX_NUMBER_OF_LINES) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Get a pointer to the line in the text structure. */
    p_line = &p_text->lines[line] ;

    /* Determine the left side of this line for reference. */
    left = UIObjectGetLeftPosition(uiText) ;

    /* Ok, let's see if we can find a place to put the cursor. */
    /* Loop through all the characters in the line and determine */
    /* what character the cursor goes before. */
    for (width=1, i=0; i<p_line->length; i++)  {
        width += IUITextGetCharacterWidth(
                     uiText,
                     width,
                     p_text->p_data[p_line->index+i]) ;
        total = left+width ;
        if (total > posX)
            break ;
    }

    /* Wherever i ends up is where the cursor position must be before. */
    offset = i+p_line->index ;

    DebugEnd() ;

    return offset ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextSelectArea
 *-------------------------------------------------------------------------*/
/**
 *  IUITextSelectArea goes through and selects all the text that is
 *  between the cursor and the end of the selection range (determined by
 *  where the mouse was dragged).
 *
 *  @param uiText -- Text object to have area selected.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextSelectArea(T_UIText uiText)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_byte8 newSelectionStart ;
    T_byte8 newSelectionEnd ;
    T_byte8 line ;
    T_byte8 startLine ;
    T_byte8 startIndex ;
    T_byte8 endLine ;
    T_byte8 endIndex ;

    DebugRoutine("IUITextSelectArea") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Before we even start going through the lines, let's get our */
    /* bearing and figure out if the cursor or the end of the selection */
    /* area is first. */
    if (p_text->cursorIndex <= p_text->endSelectIndex)  {
        /* The cursor is before the end select area.  Let's hold onto */
        /* the start and end this way. */
        startLine = p_text->cursorLine ;
        startIndex = p_text->cursorIndex ;
        endLine = p_text->endSelectLine ;
        endIndex = p_text->endSelectIndex ;
    } else {
        /* Hmmm, it's the other way around.  The end of the selection */
        /* area is before the cursor. */
        startLine = p_text->endSelectLine ;
        startIndex = p_text->endSelectIndex ;
        endLine = p_text->cursorLine ;
        endIndex = p_text->cursorIndex ;
    }

    /* Let's go through all the text line by line.  We don't know if */
    /* previous areas are no longer selected that were, so we might */
    /* have to change those lines. */
    for (line=0; p_text->lines[line].length != 0; line++)  {
        /* Get a pointer to the line in the text structure. */
        p_line = &p_text->lines[line] ;

        /* Let's first determine if the whole line is between the two */
        /* start and end areas. */
        if ((line > startLine) && (line < endLine))  {
            /* It is between the positions.  Therefore, we will declare */
            /* start and end points on this line to be the very first */
            /* and last positions. */
            newSelectionStart = 0 ;
            newSelectionEnd = (T_byte8)p_line->length ;
        } else {
            /* Well, it looks like the line in question is either */
            /* outside, on the start line, or on the end line.  Let's */
            /* find out which. */
            if ((line == startLine) || (line == endLine))  {
                /* Until we know which side has been slighted, we */
                /* will assume the whole line is selected. */
                newSelectionStart = 0 ;
                newSelectionEnd = (T_byte8)p_line->length ;

                if (line == startLine)  {
                    /* We're on the starting line.  Let us declare the */
                    /* starting side of the line. */
                    newSelectionStart = startIndex -
                                        p_text->lines[startLine].index ;
                }
                if (line == endLine)  {
                    /* We're on the ending side. */
                    newSelectionEnd = endIndex -
                                      p_text->lines[endLine].index;
                }
            } else {
                /* Hmmm, we're not selected at all.  Make it so. */
                newSelectionStart = 0 ;
                newSelectionEnd = 0 ;
            }
        }

        /* Now that we know the new selection start and end, let us */
        /* see if they are different than what we had. */
        if ((newSelectionStart != p_line->startSelect) ||
            (newSelectionEnd != p_line->endSelect))  {
            /* We have a different selection area than before.  Record */
            /* the selection area and mark the line as changed. */
            p_line->startSelect = newSelectionStart ;
            p_line->endSelect = newSelectionEnd ;
            p_line->changed = TRUE ;
        }
    }

    /* OK, the selection area should now be different.  Draw any */
    /* changes that need to be done. */
    IUITextDrawChanges(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextSelectNone
 *-------------------------------------------------------------------------*/
/**
 *  IUITextSelectNone makes the given text no longer selected and removes
 *  the selection block from the screen.  The cursor returns to where it
 *  was originally when the selection area was made.
 *
 *  @param uiText -- Text object to have area selected.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextSelectNone(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextSelectArea") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* We can cheat for the most part.  All we really need to do is */
    /* make a selection area of zero size.  We do this by making the */
    /* end of the selection area equal the start (which was where the */
    /* cursor was originally.  We then call the IUITextSelectArea and */
    /* it removes the selection area--thinking there is some other area. */
    p_text->endSelectLine = p_text->cursorLine ;
    p_text->endSelectIndex = p_text->cursorIndex ;

    /* Viola!  There ya go. */
    IUITextSelectArea(uiText) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextClipCoordinates
 *-------------------------------------------------------------------------*/
/**
 *  IUITextClipCoordinates looks at a text object and two coordinates
 *  and determines if the coodinates are outside the text object, and if
 *  they are, it clips them to stay in the box.
 *
 *  @param uiText -- Text object to clip coordinates to.
 *  @param posX -- Pointer to X position to clip.
 *  @param posY -- Pointer to Y position to clip.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextClipCoordinates(
                  T_UIText uiText,
                  T_word16 *posX,
                  T_word16 *posY)
{
    T_word16 left ;
    T_word16 right ;
    T_word16 top ;
    T_word16 bottom ;

    DebugRoutine("IUITextClipCoordinates") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get the left position of the text object. */
    left = UIObjectGetLeftPosition(uiText) ;

    /* Check to see if the x coordinate is less than the left. */
    if (*posX < left)  {
        /* If it is, change the x coordinate to be the left. */
        *posX = left ;
    }

    /* Get the right position of the text object. */
    right = UIObjectGetRightPosition(uiText) ;

    /* Check to see if the x coordinate is greater than the right. */
    if (*posX > right)  {
        /* If it is, change the x coordinate to be the right. */
        *posX = right ;
    }

    /* Get the top position of the text object. */
    top = UIObjectGetTopPosition(uiText) ;

    /* Check to see if the y coordinate is less than the top. */
    if (*posY < top)  {
        /* If it is, change the y coordinate to be the top. */
        *posY = top ;
    }

    /* Get the bottom position of the text object. */
    bottom = UIObjectGetBottomPosition(uiText) ;

    /* Check to see if the y coordinate is greater than the bottom. */
    if (*posY > bottom)  {
        /* If it is, change the y coordinate to be the bottom. */
        *posY = bottom ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextInsertKeystroke
 *-------------------------------------------------------------------------*/
/**
 *  Perhaps one of the most important routines, this routine takes
 *  keys off the keyboard and inserts them at the current cursor location.
 *
 *  @param uiText -- Text object to send keystrokes
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextInsertKeystroke(T_UIText uiText, T_byte8 character)
{
    DebugRoutine("IUITextInsertKeystroke") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Check to see if we got a key from the keyboard. */
    if (character != 0)  {
        /* Yes, we got a legal character. */
        /* Is the character a backspace? */
        if (character == '\b')  {
            IUITextBackspace(uiText) ;
        } else {
            /* Try inserting it. */
            /* But before we do, check to see if it is a carriage return. */
            if (character == '\r')  {
                /* If it is, we want to turn it into a newline character. */
                character = '\n' ;
            }
            IUITextInsertCharacter(uiText, character) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextInsertCharacter
 *-------------------------------------------------------------------------*/
/**
 *  IUITextInsertCharacter does the actual job of inserting new
 *  characters at the cursor location.  Word wrap is also checked.
 *
 *  NOTE: 
 *  It is assumed that the font of the text is the active graphics
 *  font.
 *
 *  @param uiText -- Text object to insert char into.
 *  @param character -- ASCII character to insert.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextInsertCharacter(T_UIText uiText, T_byte8 character)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_byte8 *p_character ;

    DebugRoutine("IUITextInsertCharacter") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check to see if there is room for a character. */
    if (p_text->totalSize < p_text->maxSize)  {
        /* There is room for the character. */
        /* Get a pointer to the current cursor line. */
        p_line = &p_text->lines[p_text->cursorLine] ;

        /* Find the exact position to the character. */
        p_character = p_text->p_data+p_text->cursorIndex ;

        /* Insert the character within the line (moving down the old */
        /* characters first). */
        memmove(
            p_character+1,
            p_character,
            p_text->totalSize - p_text->cursorIndex) ;
        *p_character = character ;

        /* Increment the total number of characters. */
        p_text->totalSize++ ;

        /* Mark this line as changed (since it has). */
        p_line->changed = TRUE ;

        /* Now that the character has been inserted, we have to check for */
        /* word wrap on this line.  Reformat all the information below it. */
        IUITextFormat(uiText) ;

        /* Now we need to move the cursor right so that it stays to */
        /* the right of inserted characters. */
        IUITextMoveCursorRight(uiText) ;

        /* Now that all the changes have been made, draw them. */
        IUITextDrawChanges(uiText) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextMoveCursorRight
 *-------------------------------------------------------------------------*/
/**
 *  IUITextMoveCursorRight moves the cursor one character to the right.
 *
 *  NOTE: 
 *  IUITextMoveCursorRight does not draw the changed line on the screen.
 *  However, this can be useful if you wish to call this routine multiple
 *  times.
 *
 *  @param uiText -- Text to have cursor moved right.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextMoveCursorRight(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextMoveCursorRight") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check to see if cursor is before the end. */
    if ((p_text->cursorIndex+1) < p_text->totalSize)  {
        /* No, it is not.  Change the cursor position. */
        IUITextMoveCursorToIndex(uiText, p_text->cursorIndex+1) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextMoveCursorLeft
 *-------------------------------------------------------------------------*/
/**
 *  IUITextMoveCursorLeft  moves the cursor one character to the left.
 *
 *  NOTE: 
 *  IUITextMoveCursorLeft  does not draw the changed line on the screen.
 *  However, this can be useful if you wish to call this routine multiple
 *  times.
 *
 *  @param uiText -- Text to have cursor moved right.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextMoveCursorLeft(T_UIText uiText)
{
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextMoveCursorLeft") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check to see if cursor is past the beginning. */
    if (p_text->cursorIndex > 0)  {
        /* No, it is not.  Change the cursor position. */
        IUITextMoveCursorToIndex(uiText, p_text->cursorIndex-1) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextGetCharacterWidth
 *-------------------------------------------------------------------------*/
/**
 *  IUITextGetCharacterWidth determines how wide a character in the
 *  current text is.  The routine uses the given x position to determine
 *  what special characters like tab and return will do.
 *
 *  NOTE: 
 *  It is assumed that the current font is the correct font.
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IUITextGetCharacterWidth(
                    T_UIText uiText,
                    T_word16 offset,
                    T_byte8 character)
{
    T_word16 width ;
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextGetCharacterWidth") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* See if the character is a tab. */
    if (character == '\t')  {
        /* If the character is a tab, we need to get the distance to */
        /* the closest tab position.  */
        width = IUIGetClosestTabPosition(uiText, offset)-offset ;
    } else if (character == '\n')  {
        /* Carriage returns are always zero in length. */
        width = 0 ;
    } else {
        width = GrGetCharacterWidth(character) ;
    }

    DebugEnd() ;

    return width ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextMoveCursorToIndex
 *-------------------------------------------------------------------------*/
/**
 *  IUITextMoveCursorToIndex will change where the cursor is currently
 *  located.  It will move it appropriately and update it's line index.
 *
 *  NOTE: 
 *  Not that the cursor will not update on the screen until a
 *  IUITextDrawChanges call is made.
 *
 *  @param uiText -- Text object with cursor to move
 *  @param cursorIndex -- Index location of new cursor.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextMoveCursorToIndex(
                  T_UIText uiText,
                  T_word16 cursorIndex)
{
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextMoveCursorToIndex") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Note that the cursor is moving. */
    p_text->lines[p_text->cursorLine].changed = TRUE ;

    /* Find what line the cursor is located on. */
    p_text->cursorLine = (T_byte8)IUITextFindLineNumber(uiText, cursorIndex) ;

    /* Mark the new cursor line for redrawing. */
    p_text->lines[p_text->cursorLine].changed = TRUE ;

    /* Save the line number in the end of the selection area, too. */
    p_text->endSelectLine = p_text->cursorLine ;

    /* Save the index in both cursor and end selection index. */
    p_text->cursorIndex = (T_byte8)cursorIndex ;
    p_text->endSelectIndex = p_text->cursorLine ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextFindLineNumber
 *-------------------------------------------------------------------------*/
/**
 *  IUITextFindLineNumber uses the given index to determine what line
 *  the index is found on.  The number is returned.  If too far, the
 *  last line is returned.
 *
 *  @param uiText -- Text object to find line within
 *  @param index -- Index into data
 *
 *  @return line number of text
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IUITextFindLineNumber(
                    T_UIText uiText,
                    T_word16 index)
{
    T_word16 line ;
    T_word16 lastLine ;
    T_UITextStruct *p_text ;

    DebugRoutine("IUITextFindLineNumber") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    DebugCheck(p_text->numberLines <= UI_TEXT_MAX_NUMBER_OF_LINES) ;

    for (lastLine=line=0; line<p_text->numberLines; lastLine=line, line++)  {
        /* Stop if the length of the line is zero (meaning no more */
        /* text in the ui text object. */
        if (p_text->lines[line].length == 0)
            break ;

        /* Stop if we found an index that is greater than the index */
        /* we are looking for.  The last line will be the line we */
        /* want to return. */
        if (p_text->lines[line].index > index)
            break ;
    }

    DebugCheck(lastLine < UI_TEXT_MAX_NUMBER_OF_LINES) ;
    DebugEnd() ;

    return lastLine ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextFormat
 *-------------------------------------------------------------------------*/
/**
 *  IUITextFormat goes through and formats the lines of the text
 *  so that characters are where they should be.  This includes word
 *  wrapping and such.
 *
 *  @param uiText -- Text object to format
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextFormat(T_UIText uiText)
{
    T_UITextStruct *p_text ;
    T_UITextLine *p_line ;
    T_word16 line = 0 ;
    T_word16 index = 0 ;
    T_word16 lastIndex = 0 ;
    T_word16 width = 1 ;
    T_word16 lastCut = 0 ;
    T_word16 lastWidth = 0 ;
    T_word16 charWidth ;
    T_word16 newLength = 0 ;
    T_word16 newIndex = 0 ;

    DebugRoutine("IUITextFormat") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Loop through all the characters in the ui text object. */
    for (index = 0; index < p_text->totalSize; index++)  {
        /* Check if we are past the first character. */
        if ((index != 0)||(p_text->p_data[index]=='\n'))  {
            /* We now can check to see if we have gone from */
            /* a space to a non-space character.  Also, if this */
            /* is a carriage return, we have found a place to */
            /* word wrap. */
            if ((p_text->p_data[index] == '\n') ||
                 ((isspace(p_text->p_data[index-1])) &&
                 (!isspace(p_text->p_data[index]))))  {
                /* We're good to go.  Note that this is the best point */
                /* to do a word wrap. */
                lastCut = index ;
                lastWidth = width ;
            }
        }

        /* Find the width of the current character. */
        charWidth = IUITextGetCharacterWidth(
                        uiText,
                        width,
                        p_text->p_data[index]) ;

        /* Can this character fit? Or is this a carriage return? */
        if ((charWidth+width > p_text->width) ||
            (p_text->p_data[index] == '\n'))  {
            /* No, it cannot fit.  We'll have to do the word wrap. */
            /* However, check to see if there was ever found a place */
            /* to do a cut. */
            if (lastWidth == 0)  {
                /* No, we didn't find a single space.  This means */
                /* there are so many non-spaces that we have no choice */
                /* but to split the word. */
                newIndex = lastIndex ;

                newLength = index-lastIndex ;

                /* Note that this is now the last index used. */
                lastIndex = index ;

                /* If this is a carriage return, add it to the line. */
                /* Also add one to the length.  Also note that we cut */
                /* after the \n. */
                if (p_text->p_data[index] == '\n')  {
                    newIndex++ ;
                }

                /* Start the width over. */
                width = 1 ;
            } else {
                /* Yes, we did find a space.  Let's chop it up at that */
                /* point. */
                /* Let's first record all the information we need about */
                /* this line. */
                newIndex = lastIndex ;
                newLength = lastCut-lastIndex ;

                /* Note that the cut is where the last index occured. */
                lastIndex = lastCut ;

                /* Subtract off the part we used off the width. */
                width -= lastWidth ;

                /* Clear out the lastWidth. */
                lastWidth = 0 ;
            }

            /* Now that we know where to do the word wrap, we */
            /* need to record the line information and move to the */
            /* next line.  However, if all the data is the same, */
            /* we won't do anything to keep the text from being */
            /* redrawn. */
            p_line = &p_text->lines[line] ;
            /* Check to see if either the index or the length is */
            /* different. */
            if ((p_line->index != newIndex) ||
                (p_line->length != newLength))  {
                /* Yes, there is a difference.  Makes the changes */
                /* and mark the line as changed. */
                p_line->index = newIndex ;
                p_line->length = newLength ;
                p_line->changed = TRUE ;
            }

            /* Done with the line.  Move down one line. */
            line++ ;
            if (line == UI_TEXT_MAX_NUMBER_OF_LINES)
                break ;
        }

        /* Place the character on the width. */
        width += charWidth ;
    }

    /* Done looping.  Do we have any information left to store? */
    if (lastIndex != index)  {
        /* OK, the last line needs to be recorded. */
        newIndex = lastIndex ;
        newLength = index-lastIndex ;

        /* Now that we know where to do the word wrap, we */
        /* need to record the line information and move to the */
        /* next line.  However, if all the data is the same, */
        /* we won't do anything to keep the text from being */
        /* redrawn. */
        p_line = &p_text->lines[line] ;

        /* Check to see if either the index or the length is */
        /* different. */
        if ((p_line->index != newIndex) ||
            (p_line->length != newLength))  {
            /* Yes, there is a difference.  Makes the changes */
            /* and mark the line as changed. */
            p_line->index = newIndex ;
            p_line->length = newLength ;
            p_line->changed = TRUE ;
        }

        /* Done with the line.  Move down one line. */
        line++ ;
    }

    /* Record the number of lines in the text. */
    p_text->numberLines = (T_byte8)line ;

    /* OK, last step.  Clear out the rest of the line lengths with zero. */
    for (; line<UI_TEXT_MAX_NUMBER_OF_LINES; line++)  {
        if (p_text->lines[line].length != 0)  {
            p_text->lines[line].length = 0 ;
            p_text->lines[line].index = index ;
            p_text->lines[line].changed = TRUE ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUITextBackspace
 *-------------------------------------------------------------------------*/
/**
 *  IUITextBackspace deletes one character to the left of the cursor
 *  location.  If the cursor is at the start, no characters are deleted.
 *
 *  @param uiText -- Text object to delete character
 *
 *<!-----------------------------------------------------------------------*/
static T_void IUITextBackspace(T_UIText uiText)
{
    T_UITextStruct *p_text ;
    T_word16 index ;

    DebugRoutine("IUITextBackspace") ;
    DebugCheck(uiText != UI_TEXT_BAD) ;

    /* Get a pointer to the actual text structure */
    p_text = uiText ;

    /* Check to see if cursor is NOT at the beginning. */
    if ((index = p_text->cursorIndex) != 0)  {
        /* If the cursor is not at the start, then there must be a */
        /* character to its left that can be deleted.  Delete that */
        /* character. */
        memmove(
            p_text->p_data+index-1,
            p_text->p_data+index,
            p_text->totalSize-index) ;

        /* There is now one less character. */
        p_text->totalSize-- ;

        /* Now that the character has been deleted, we have to reformat */
        /* the text. */
        IUITextFormat(uiText) ;

        /* Move the cursor to the left one to make it move with the */
        /* text. */
        IUITextMoveCursorLeft(uiText) ;

        /* Now that all the changes have been made, draw them. */
        IUITextDrawChanges(uiText) ;
    }

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  UITEXT.C
 *-------------------------------------------------------------------------*/
