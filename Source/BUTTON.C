/*-------------------------------------------------------------------------*
 * File:  BUTTON.C
 *-------------------------------------------------------------------------*/
/**
 * Buttons are a type of UI element in the Form system.  As expected,
 * events can be attached to buttons and handled by the Form handler.
 * Buttons can be rendered with just a graphic or with text on it too.
 * Hot keys can also be assigned.
 *
 * @addtogroup BUTTON
 * @brief Button UI Component
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BUTTON.H"
#include "GENERAL.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "PICS.H"
#include "SOUND.H"
#include "SOUNDS.H"

#define BUTTON_TAG 12345

static T_buttonID G_buttonarray[MAX_BUTTONS];
static T_buttonID ButtonInit(
        T_word16 lx,
        T_word16 ly,
        const char *bmname,
        E_Boolean toggletype,
        T_word16 keyassoc,
        T_buttonHandler p_cbroutine,
        T_buttonHandler p_cbroutine2);

static T_buttonID G_buttonActedOn = NULL;
static E_buttonAction G_buttonAction = BUTTON_ACTION_NO_ACTION;
static T_buttonID G_mouseOverButton = NULL;
/*-------------------------------------------------------------------------*
 * Routine:  ButtonCreate
 *-------------------------------------------------------------------------*/
/**
 *  Adds a button to the current list of buttons for a form
 *
 *<!-----------------------------------------------------------------------*/
T_buttonID ButtonCreate(
        T_word16 lx,
        T_word16 ly,
        const char *bmname,
        E_Boolean toggletype,
        T_word16 keyassoc,
        T_buttonHandler p_cbroutine,
        T_buttonHandler p_cbroutine2)
{
    T_word16 i;

    DebugRoutine("ButtonAdd");
    for (i = 0; i < MAX_BUTTONS; i++) {
        if (G_buttonarray[i] == NULL )  //add a button to list
        {
            G_buttonarray[i] = ButtonInit(lx, ly, bmname, toggletype, keyassoc,
                    p_cbroutine, p_cbroutine2);
            break;
        }
    }

    DebugCheck(i<MAX_BUTTONS);
    DebugEnd();
    return (G_buttonarray[i]);
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonInit (
 *-------------------------------------------------------------------------*/
/**
 *  Allocates memory and inits variables for a new button.
 *
 *<!-----------------------------------------------------------------------*/
static T_buttonID ButtonInit(
        T_word16 lx,
        T_word16 ly,
        const char *bmname,
        E_Boolean toggletype,
        T_word16 keyassoc,
        T_buttonHandler p_cbroutine,
        T_buttonHandler p_cbroutine2)
{
    T_word32 size;
    T_buttonStruct *myID;

    DebugRoutine("ButtonInit");
    DebugCheck(lx<=320 && ly<=200);
    DebugCheck(bmname!=NULL);

    size = sizeof(T_buttonStruct);
    myID = (T_buttonID)MemAlloc(size);

    DebugCheck(myID!=NULL);
    if (myID != NULL ) {
        myID->p_graphicID = NULL;
        myID->p_graphicID = GraphicCreate(lx, ly, bmname);
        myID->normalPNG = GraphicGetPNG(myID->p_graphicID);
        myID->selectPNG = PNG_BAD;
        DebugCheck(myID->p_graphicID != NULL);
        GraphicSetPreCallBack(myID->p_graphicID, ButtonDrawCallBack, 0);
        myID->toggle = toggletype;
        myID->pushed = FALSE;
        myID->p_callback = p_cbroutine;
        myID->p_callback2 = p_cbroutine2;
        myID->enabled = TRUE;
        myID->scancode = keyassoc;
        myID->textID = NULL;
        myID->data = 0;
        myID->subdata = 0;
        myID->tag = BUTTON_TAG;
    }
    DebugEnd();
    return (myID);
}

T_void ButtonSetSelectPic(T_buttonID buttonID, char *picname)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonSetSelectPic");
    DebugCheck(buttonID != NULL);
    DebugCheck(picname != NULL);

    /* get button struct */
    p_button = (T_buttonStruct *)buttonID;

    /* lock the resource */
    if (p_button->selectPNG != PNG_BAD) {
        /* free alternate picture */
        PNGUnlock(p_button->selectPNG);
    }

    p_button->selectPNG = PNGLock(picname);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonDelete / ButtonCleanUp
 *-------------------------------------------------------------------------*/
/**
 *  Releases memory allocated to a button
 *  Cleanup releases memory allocated to all 'buttons'
 *
 *<!-----------------------------------------------------------------------*/
T_void ButtonDelete(T_buttonID buttonID)
{
    T_word16 i;
    T_buttonStruct *p_button;

    DebugRoutine("ButtonDelete");
    if (buttonID != NULL ) {
        for (i = 0; i < MAX_BUTTONS; i++) {
            if (G_buttonarray[i] == buttonID) //found it, now kill it
                    {
                p_button = (T_buttonStruct *)buttonID;
                DebugCheck(p_button->tag==BUTTON_TAG);
                if (p_button->pushed == TRUE) {
                    ButtonUpNoAction(buttonID);
                }
                /* delete graphic */
                GraphicDelete(p_button->p_graphicID);
                if (p_button->selectPNG != PNG_BAD) {
                    /* free alternate picture */
                    PNGUnlock(p_button->selectPNG);
                    p_button->selectPNG = PNG_BAD;
                }
                if (p_button->textID != NULL )
                    TextDelete(p_button->textID);
                MemFree(G_buttonarray[i]);
                MemCheck(200);
                G_buttonarray[i] = NULL;
                break;
            }
        }
    }

    DebugEnd();
}

T_void ButtonCleanUp(T_void)
{
    T_word16 i;
    T_buttonStruct *p_button;

    DebugRoutine("ButtonCleanUp");

    for (i = 0; i < MAX_BUTTONS; i++)
        if (G_buttonarray[i] != NULL ) {
            p_button = (T_buttonStruct *)G_buttonarray[i];
            DebugCheck(p_button->tag==BUTTON_TAG);
            GraphicDelete(p_button->p_graphicID);
            if (p_button->textID != NULL )
                TextDelete(p_button->textID);
            if (p_button->selectPNG != PNG_BAD) {
                /* free alternate picture */
                PictureUnfind(p_button->selectPNG);
                p_button->selectPNG = PNG_BAD;
            }
            MemFree(G_buttonarray[i]);
            MemCheck(201);
            G_buttonarray[i] = NULL;
        }
    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonKeyControl/ButtonMouseControl
 *-------------------------------------------------------------------------*/
/**
 *  This routine acts as a generic keyboard and mouse handling routine
 *  for all active buttons
 *
 *<!-----------------------------------------------------------------------*/
T_void ButtonKeyControl(E_keyboardEvent event, T_word16 scankey)
{
    T_word16 keyscan;
    T_word16 i;
    T_buttonStruct *p_button;

    DebugRoutine("ButtonKeyControl");

    G_buttonActedOn = NULL;
    G_buttonAction = BUTTON_ACTION_NO_ACTION;
    switch (event) {
        case KEYBOARD_EVENT_PRESS:
            if (scankey == KEY_SCAN_CODE_ALT) {
                for (i = 0; i < MAX_BUTTONS; i++) {
                    if (G_buttonarray[i] != NULL ) {
                        p_button = (T_buttonStruct *)G_buttonarray[i];
                        DebugCheck(p_button->tag==BUTTON_TAG);
                        keyscan = p_button->scancode;
                        if ((!(keyscan >> 8)) && (p_button->toggle == FALSE)) {
                            ButtonUp(p_button);
                        }
                    }
                }
            } else {
                for (i = 0; i < MAX_BUTTONS; i++) {
                    if (G_buttonarray[i] != NULL ) {
                        p_button = (T_buttonStruct *)G_buttonarray[i];
                        DebugCheck(p_button->tag==BUTTON_TAG);
                        keyscan = p_button->scancode;
                        if ((keyscan >> 8)
                                && (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)
                                        == TRUE)) {
                            keyscan &= 0xFF;
                            if (keyscan == scankey) {
                                if (p_button->toggle == TRUE) {
                                    if (p_button->pushed == FALSE)
                                        ButtonDown(p_button);
                                    else {
                                        ButtonUp(p_button);
                                    }
                                } else
                                    ButtonDown(p_button);
                            }
                        } else if ((keyscan == scankey && keyscan > 0)
                                && (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)
                                        == FALSE)) {
                            if (p_button->toggle == TRUE) {
                                if (p_button->pushed == FALSE)
                                    ButtonDown(p_button);
                                else {
                                    ButtonUp(p_button);
                                }
                            } else
                                ButtonDown(p_button);
                        }
                    }
                }
            }
            break;

        case KEYBOARD_EVENT_RELEASE:
            if (scankey == KEY_SCAN_CODE_ALT) {
                for (i = 0; i < MAX_BUTTONS; i++) {
                    if (G_buttonarray[i] != NULL ) {
                        p_button = (T_buttonStruct *)G_buttonarray[i];
                        DebugCheck(p_button->tag==BUTTON_TAG);
                        keyscan = p_button->scancode;
                        if ((keyscan >> 8) && (p_button->toggle == FALSE)) {
                            ButtonUp(p_button);
                        }
                    }
                }
            } else {
                for (i = 0; i < MAX_BUTTONS; i++) {
                    if (G_buttonarray[i] != NULL ) {
                        p_button = (T_buttonStruct *)G_buttonarray[i];
                        DebugCheck(p_button->tag==BUTTON_TAG);
                        if (p_button->toggle == FALSE) {
                            keyscan = p_button->scancode;
                            if ((keyscan >> 8)
                                    && (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)
                                            == TRUE)) {
                                keyscan &= 0xFF;
                                if (keyscan == scankey) {
                                    ButtonUp(p_button);
                                }
                            } else if ((keyscan == scankey)
                                    && (KeyboardGetScanCode(KEY_SCAN_CODE_ALT)
                                            == FALSE)) {
                                ButtonUp(p_button);
                            }
                        }
                    }
                }
            }
            break;

        case KEYBOARD_EVENT_HELD:
            break;

        default:
            break;
    }

    DebugEnd();
}

T_void ButtonMouseControl(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        T_buttonClick button)

{
    static T_buttonID buttonpushed = NULL;
    T_buttonStruct *p_button;

    DebugRoutine("ButtonMouseControl");
    DebugCheck(event<MOUSE_EVENT_UNKNOWN);

    if (!ButtonIsValid(buttonpushed))
        buttonpushed = NULL;

    if (buttonpushed != NULL )
        p_button = (T_buttonStruct *)buttonpushed;

    G_mouseOverButton = ButtonGetByLoc(x, y);

    switch (event) {
        case MOUSE_EVENT_IDLE:
            break;

        case MOUSE_EVENT_START:

            if (button == MOUSE_BUTTON_LEFT)
                buttonpushed = G_mouseOverButton;
            else
                buttonpushed = NULL;

            if (buttonpushed != NULL ) {
                p_button = (T_buttonStruct *)buttonpushed;
                DebugCheck(p_button->tag==BUTTON_TAG);
                if (p_button->toggle == TRUE) {
                    if (p_button->pushed == TRUE) {
                        ButtonUp(buttonpushed);
                    } else
                        ButtonDown(buttonpushed);
                } else {
                    ButtonDown(buttonpushed);
                }
            }
            break;

        case MOUSE_EVENT_HELD:
        case MOUSE_EVENT_DRAG:
            if (buttonpushed != NULL ) {
                DebugCheck(p_button->tag==BUTTON_TAG);
                if (p_button->toggle == FALSE) {
                    if (!(ButtonIsAt(buttonpushed, x, y))) /*release button if mouse*/
                    { /* was moved off of button */
                        if (p_button->pushed == TRUE)
                            ButtonUpNoAction(buttonpushed);
                    }
                }
            }
            break;

        default:
            if (buttonpushed != NULL ) {
                DebugCheck(p_button->tag==BUTTON_TAG);
                if (p_button->toggle == FALSE && p_button->pushed == TRUE) {
                    ButtonUp(buttonpushed);
                }

            }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonDown/ButtonUp routines
 *-------------------------------------------------------------------------*/
/**
 *  Sets the state (pushed and changed flags) of a button
 *
 *<!-----------------------------------------------------------------------*/
T_void ButtonDown(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;

    DebugRoutine("ButtonDown");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button->pushed == FALSE && p_button->enabled == TRUE) {
        GraphicClear(p_button->p_graphicID, 0);
        GraphicSetOffSet(p_button->p_graphicID, 1, 1);
        if (p_button->selectPNG != PNG_BAD) {
            GraphicSetPNG(p_button->p_graphicID, p_button->selectPNG);
        } else {
            GraphicSetShadow(p_button->p_graphicID, 150);
        }
        p_button->pushed = TRUE;
        G_buttonActedOn = buttonID;
        G_buttonAction = BUTTON_ACTION_PUSHED;
        if (p_button->textID != NULL ) {
            graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
            GraphicSetOffSet(graphicID, 1, 1); //shift text position
        }
        SoundPlayByNumber(SOUND_BUTTON_UP, 255);
        ButtonDoCallback(buttonID);
    }
    DebugEnd();
}

T_void ButtonDownNoAction(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;

    DebugRoutine("ButtonDownNoAction");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button->pushed == FALSE && p_button->enabled == TRUE) {
        GraphicClear(p_button->p_graphicID, 0);
        GraphicSetOffSet(p_button->p_graphicID, 1, 1);
        if (p_button->selectPNG != PNG_BAD) {
            GraphicSetPNG(p_button->p_graphicID, p_button->selectPNG);
        } else {
            GraphicSetShadow(p_button->p_graphicID, 150);
        }
        p_button->pushed = TRUE;
        if (p_button->textID != NULL ) {
            graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
            GraphicSetOffSet(graphicID, 1, 1); //shift text position
        }
    }

    DebugEnd();
}

T_void ButtonDownNum(T_word16 num)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonDownNum");
    DebugCheck(num<MAX_BUTTONS);
    DebugCheck(G_buttonarray[num]!=NULL);

    p_button = (T_buttonStruct *)G_buttonarray[num];
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button != NULL )
        ButtonDownNoAction(G_buttonarray[num]);
    DebugEnd();
}

T_void ButtonUp(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;

    DebugRoutine("ButtonUp");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button->pushed == TRUE && p_button->enabled == TRUE) {
        GraphicClear(p_button->p_graphicID, 0);
        GraphicSetOffSet(p_button->p_graphicID, 0, 0);
        if (p_button->selectPNG != PNG_BAD) {
            GraphicSetPNG(p_button->p_graphicID, p_button->normalPNG);
        } else {
            GraphicClear(p_button->p_graphicID, 0);
            GraphicSetOffSet(p_button->p_graphicID, 0, 0);
            GraphicSetShadow(p_button->p_graphicID, 255);
        }
        p_button->pushed = FALSE;
        G_buttonActedOn = buttonID;
        G_buttonAction = BUTTON_ACTION_RELEASED;
        if (p_button->textID != NULL ) {
            graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
            GraphicSetOffSet(graphicID, 0, 0); //shift text position
        }
//        SoundPlayByNumber(SOUND_BUTTON_UP,255);
        ButtonDoCallback2(buttonID);
    }
    DebugEnd();
}

T_void ButtonUpNoAction(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;
    DebugRoutine("ButtonUpNoAction");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button->pushed == TRUE && p_button->enabled == TRUE) {
        GraphicClear(p_button->p_graphicID, 0);
        GraphicSetOffSet(p_button->p_graphicID, 0, 0);
        if (p_button->selectPNG != PNG_BAD) {
            GraphicSetPNG(p_button->p_graphicID, p_button->normalPNG);
        } else {
            GraphicSetShadow(p_button->p_graphicID, 255);
        }
        p_button->pushed = FALSE;
        if (p_button->textID != NULL ) {
            graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
            GraphicSetOffSet(graphicID, 0, 0); //shift text position
        }
    }

    DebugEnd();
}

T_void ButtonUpNum(T_word16 num)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonUpNum");
    DebugCheck(num<MAX_BUTTONS);
    DebugCheck(G_buttonarray[num]!=NULL);

    p_button = (T_buttonStruct *)G_buttonarray[num];
    DebugCheck(p_button->tag==BUTTON_TAG);
    if (p_button != NULL )
        ButtonUpNoAction(G_buttonarray[num]);
    DebugEnd();
}

T_void ButtonEnable(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;

    DebugRoutine("ButtonEnable");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    p_button->enabled = TRUE;
    DebugCheck(p_button->tag==BUTTON_TAG);

    /* put button into up position */
    GraphicClear(p_button->p_graphicID, 0);
    GraphicSetOffSet(p_button->p_graphicID, 0, 0);
    GraphicSetShadow(p_button->p_graphicID, 255);
    p_button->pushed = FALSE;
    if (p_button->textID != NULL ) {
        graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
        GraphicSetOffSet(graphicID, 0, 0); //shift text position
    }

    DebugEnd();
}

T_void ButtonDisable(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicID graphicID;

    DebugRoutine("ButtonDisable");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    p_button->enabled = FALSE;
    DebugCheck(p_button->tag==BUTTON_TAG);

    /* put button into up and shadowed position */
    GraphicClear(p_button->p_graphicID, 0);
    GraphicSetOffSet(p_button->p_graphicID, 0, 0);
    GraphicSetShadow(p_button->p_graphicID, 100);
    p_button->pushed = FALSE;
    if (p_button->textID != NULL ) {
        graphicID = TextGetGraphicID(p_button->textID); //get ID of text graphic
        GraphicSetOffSet(graphicID, 0, 0); //shift text position
    }

    DebugEnd();
}
/*-------------------------------------------------------------------------*
 * Routine:  ButtonDoCallback/ButtonDoCallback2
 *-------------------------------------------------------------------------*/
/**
 *  Executes one of the callback routines pointed to in the button struct
 *
 *<!-----------------------------------------------------------------------*/
T_void ButtonDoCallback(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonDoCallBack");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);

    if (p_button->p_callback != NULL )
        p_button->p_callback(buttonID);

    DebugEnd();
}

T_void ButtonDoCallback2(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonDoCallBack");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);

    if (p_button->p_callback2 != NULL )
        p_button->p_callback2(buttonID);

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonIsPushed/ButtonIsEnabled/ButtonIsAt
 *-------------------------------------------------------------------------*/
/**
 *  Returns specific information about the status of a given button
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ButtonIsPushed(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonIsPushed");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);

    DebugEnd();

    return (p_button->pushed);
}

E_Boolean ButtonIsEnabled(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonIsEnabled");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    DebugCheck(p_button != NULL);

    DebugEnd();
    return (p_button->enabled);

}

E_Boolean ButtonIsAt(T_buttonID buttonID, T_word16 lx, T_word16 ly)
{
    T_buttonStruct *p_button;
    E_Boolean retvalue;

    DebugRoutine("ButtonIsAt");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    DebugCheck(p_button->p_graphicID != NULL);
    retvalue = GraphicIsAt(p_button->p_graphicID, lx, ly);
    DebugEnd();
    return (retvalue);
}

T_graphicID ButtonGetGraphic(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonGetGraphic");
    DebugCheck(buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    DebugCheck(p_button->p_graphicID != NULL);

    DebugEnd();
    return (p_button->p_graphicID);
}

T_buttonID ButtonGetByLoc(T_word16 x, T_word16 y)
{
    T_word16 i;
    T_buttonID retvalue = NULL;

    DebugRoutine("ButtonGetByLoc");

    for (i = 0; i < MAX_BUTTONS; i++) {
        if (G_buttonarray[i] != NULL ) {
            if (ButtonIsAt(G_buttonarray[i], x, y)) {
                retvalue = G_buttonarray[i];
                break;
            }
        }
    }
    DebugEnd();

    return (retvalue);
}

T_buttonID ButtonGetByKey(T_word16 keycode)
{
    T_word16 i;
    T_buttonStruct *p_button;
    T_buttonID retvalue = NULL;

    DebugRoutine("ButtonGetByKey");

    for (i = 0; i < MAX_BUTTONS; i++) {
        if (G_buttonarray[i] != NULL ) {
            p_button = (T_buttonStruct *)G_buttonarray[i];
            DebugCheck(p_button->tag==BUTTON_TAG);
            if (p_button->scancode == keycode) {
                retvalue = G_buttonarray[i];
                break;
            }
        }
    }
    DebugEnd();

    return (retvalue);
}

/*-------------------------------------------------------------------------*
 * Routine:  ButtonDrawCallBack
 *-------------------------------------------------------------------------*/
/**
 *  A routine that will be called once the first time a button is drawn..
 *  draws a 'shadow' behind the button, and then sets the graphic draw
 *  callback routine to null.
 *
 *<!-----------------------------------------------------------------------*/
T_void ButtonDrawCallBack(T_graphicID graphicID, T_word16 info)
{
    T_graphicStruct *p_graphic;

    DebugRoutine("ButtonDrawCallBack");
    DebugCheck(graphicID != NULL);

    p_graphic = (T_graphicStruct*)graphicID;
    MouseHide();
    GrDrawFrame(p_graphic->locx, p_graphic->locy,
            p_graphic->locx + p_graphic->width,
            p_graphic->locy + p_graphic->height, 0);
    MouseShow();
    GraphicSetPreCallBack(graphicID, NULL, 0);
    DebugEnd();
}

T_void ButtonDrawTextCallback(T_graphicID graphicID, T_word16 info)
{
    T_graphicStruct *p_graphic;
    T_buttonStruct *p_button;
    T_textStruct *p_text;
    T_resourceFile res;
    T_bitfont *p_font;
    T_graphicStruct *p_textgraphic;

    DebugRoutine("ButtonDrawCallBack");
    DebugCheck(graphicID != NULL);

    p_graphic = (T_graphicStruct*)graphicID;
    p_button = (T_buttonStruct *)G_buttonarray[info];
    p_text = (T_textStruct *)p_button->textID;
    p_textgraphic = (T_graphicStruct *)p_text->p_graphicID;

    res = ResourceOpen((T_byte8 *)"sample.res");
    p_font = (T_bitfont *)ResourceLock(p_text->font);

    GrSetBitFont(p_font);
    GrSetCursorPosition(p_textgraphic->locx + p_graphic->xoff,
            p_textgraphic->locy + p_graphic->yoff);
    MouseHide();
    if (p_text->bcolor == 0)
        GrDrawText(p_text->data, p_text->fcolor);
    else
        GrDrawShadowedText(p_text->data, p_text->fcolor, p_text->bcolor);
    MouseShow();
    ResourceUnlock(p_text->font);
    ResourceClose(res);

    DebugEnd();
}

T_word16 ButtonIDToIndex(T_buttonID buttonID)
{
    T_word16 i;

    for (i = 0; i < MAX_BUTTONS; i++) {
        if (G_buttonarray[i] == buttonID)
            break;
    }
    return i;
}

T_void ButtonSetText(T_buttonID buttonID, const T_byte8 *string, T_byte8 color)
{
    T_buttonStruct *p_button;
    T_textStruct *p_text;
    T_graphicStruct *p_buttongraphic, *p_textgraphic;
    T_word16 textx, texty;

    DebugRoutine("ButtonSetText");
    DebugCheck(buttonID != NULL);
    DebugCheck(string != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    p_buttongraphic = (T_graphicStruct *)p_button->p_graphicID;
    if (p_button->textID == NULL ) /* create a text object if one has not been defined */
    {
        p_button->textID = TextCreate(p_buttongraphic->locx,
                p_buttongraphic->locy, string);
        TextSetColor(p_button->textID, color, 1);
    } else /* one has already been created for this button, change text only */
    {
        TextSetText(p_button->textID, string);
        TextSetColor(p_button->textID, color, 1);
    }
    /* now, let's center the text */
    p_text = (T_textStruct*)p_button->textID;
    /* get text graphic struct info */
    p_textgraphic = (T_graphicStruct*)p_text->p_graphicID;

    /* first make sure that the text fits in the button!! */
//	DebugCheck (p_textgraphic->width<p_buttongraphic->width);
    /* now, calculate where the text should go */
    textx = p_buttongraphic->locx + (p_buttongraphic->width / 2)
            - (p_textgraphic->width / 2);
    texty = p_buttongraphic->locy + (p_buttongraphic->height / 2)
            - (p_textgraphic->height / 2);
    /* change the text position accordingly */
    TextMoveText(p_button->textID, textx, texty);
    GraphicSetPostCallBack(p_button->p_graphicID, ButtonDrawTextCallback,
            ButtonIDToIndex(buttonID));
    DebugEnd();
}

T_void ButtonSetFont(T_buttonID buttonID, T_byte8 *fntname)
{
    T_buttonStruct *p_button;
    T_textStruct *p_text;
    T_resourceFile res;

    DebugRoutine("ButtonSetFont");
    DebugCheck(buttonID!=NULL);
    DebugCheck(fntname!=NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    p_text = (T_textStruct *)p_button->textID;
    if (!p_text) {
        p_text = malloc(sizeof(*p_text));
        p_button->textID = p_text;
    }
    res = ResourceOpen("sample.res");
    p_text->font = ResourceFind(res, fntname);
    ResourceClose(res);

    DebugEnd();
}

T_void ButtonSetData(T_buttonID buttonID, T_word32 newdata)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonSetData");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    p_button->data = newdata;

    DebugEnd();
}

T_void ButtonSetSubData(T_buttonID buttonID, T_word32 newdata)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonSetSubData");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    p_button->subdata = newdata;

    DebugEnd();
}

T_void ButtonSetCallbacks(
        T_buttonID buttonID,
        T_buttonHandler cb1,
        T_buttonHandler cb2)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonSetCallbacks");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);

    p_button->p_callback = cb1;
    p_button->p_callback2 = cb2;

    DebugEnd();
}

T_void ButtonSetPicture(T_buttonID buttonID, T_byte8 *picname)
{
    T_buttonStruct *p_button;
    T_graphicStruct *p_graphic;
    T_word16 lx, ly;

    DebugRoutine("ButtonSetPicture");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    /* get the old graphic coordinates */
    p_graphic = (T_graphicStruct *)p_button->p_graphicID;
    lx = p_graphic->locx;
    ly = p_graphic->locy;
    /* delete the old graphic */
    GraphicDelete(p_button->p_graphicID);
    /* create a new one */
    p_button->p_graphicID = NULL;
    p_button->p_graphicID = GraphicCreate(lx, ly, picname);
    DebugCheck(p_button->p_graphicID != NULL);
    GraphicSetPreCallBack(p_button->p_graphicID, ButtonDrawCallBack, 0);
    /* all done */
    DebugEnd();
}

T_word32 ButtonGetData(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonGetData");
    DebugCheck(buttonID != NULL);
    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    DebugEnd();

    return (p_button->data);
}

T_word32 ButtonGetSubData(T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine("ButtonGetSubData");
    DebugCheck(buttonID != NULL);
    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->tag==BUTTON_TAG);
    DebugEnd();

    return (p_button->subdata);
}

E_buttonAction ButtonGetAction(T_void)
{
    E_buttonAction retvalue;
    DebugRoutine("ButtonGetAction");

    retvalue = G_buttonAction;
    G_buttonAction = BUTTON_ACTION_NO_ACTION;
    G_buttonActedOn = NULL;

    DebugEnd();
    return (retvalue);
}

T_void ButtonRedrawAllButtons(T_void)
{
    T_word16 i;

    DebugRoutine("ButtonRedrawAllButtons");
    for (i = 0; i < MAX_BUTTONS; i++) {
        if (G_buttonarray[i] != NULL ) {
            ButtonRedraw(G_buttonarray[i]);
        }
    }
    GraphicUpdateAllGraphics();
    DebugEnd();
}

T_void ButtonRedraw(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_graphicStruct *p_graphic;

    DebugRoutine("ButtonRedraw");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    DebugCheck(p_button->p_graphicID != NULL);
    DebugCheck(p_button->tag==BUTTON_TAG);
    p_graphic = (T_graphicStruct *)p_button->p_graphicID;
    p_graphic->changed = TRUE;

    DebugEnd();
}

E_Boolean ButtonIsValid(T_buttonID buttonID)
{
    T_word16 i;
    E_Boolean retvalue = FALSE;

    DebugRoutine("ButtonIsValid");

    for (i = 0; i < MAX_BUTTONS; i++) {
        if (buttonID == G_buttonarray[i]) {
            retvalue = TRUE;
            break;
        }
    }

    DebugEnd();
    return (retvalue);
}

E_Boolean ButtonIsMouseOverButton(T_void)
{
    E_Boolean retvalue = FALSE;
    DebugRoutine("MouseIsOverButton");

    if (G_mouseOverButton != NULL )
        retvalue = TRUE;

    DebugEnd();
    return (retvalue);
}

#if 0
T_void ButtonDisableAll(T_void)
{
    T_word16 i;

    for (i=0;i<MAX_BUTTONS;i++)
    if (G_buttonarray[i]!=NULL)
    ButtonDisable(G_buttonarray[i]);
}

T_void ButtonEnableAll(T_void)
{
    T_word16 i;

    for (i=0;i<MAX_BUTTONS;i++)
    if (G_buttonarray[i]!=NULL)
    ButtonEnable(G_buttonarray[i]);
}
#endif

T_void *ButtonGetStateBlock(T_void)
{
    T_buttonID *p_buttons;

    p_buttons = MemAlloc(sizeof(G_buttonarray));
    DebugCheck(p_buttons != NULL);
    memcpy(p_buttons, G_buttonarray, sizeof(G_buttonarray));
    memset(G_buttonarray, 0, sizeof(G_buttonarray));

    return p_buttons;
}

T_void ButtonSetStateBlock(T_void *p_state)
{
    memcpy(G_buttonarray, p_state, sizeof(G_buttonarray));
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  BUTTON.C
 *-------------------------------------------------------------------------*/
