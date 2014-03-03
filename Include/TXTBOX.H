/****************************************************************************/
/*    FILE:  Txtbox.H                                                       */
/****************************************************************************/

#ifndef _Txtbox_H_
#define _Txtbox_H_

#include "GENERAL.H"
#include "BUTTON.H"
#include "GRAPHIC.H"

#define MAX_TxtboxES 50

typedef T_void *T_TxtboxID;
typedef T_void (*T_TxtboxHandler) (T_TxtboxID TxtboxID);

typedef enum
{
    Txtbox_MODE_EDIT_FIELD,
    Txtbox_MODE_EDIT_FORM,
    Txtbox_MODE_VIEW_SCROLL_FORM,
    Txtbox_MODE_VIEW_NOSCROLL_FORM,
    Txtbox_MODE_SELECTION_BOX,
    Txtbox_MODE_FIXED_WIDTH_FIELD,
    Txtbox_MODE_UNKNOWN
} E_TxtboxMode;

typedef enum
{
    Txtbox_JUSTIFY_LEFT,
    Txtbox_JUSTIFY_CENTER,
    Txtbox_JUSTIFY_UNKNOWN
} E_TxtboxJustify;


typedef enum
{
    Txtbox_ACTION_NO_ACTION,
    Txtbox_ACTION_GAINED_FOCUS,
    Txtbox_ACTION_LOST_FOCUS,
    Txtbox_ACTION_ACCEPTED,
    Txtbox_ACTION_DATA_CHANGED,
    Txtbox_ACTION_SELECTION_CHANGED,
    Txtbox_ACTION_UNKNOWN
} E_TxtboxAction;


typedef struct
{

    T_graphicID     p_graphicID;

    /* current data string */
    T_byte8         *data;

    /* location in pixels for the window */
    T_word16        lx1;
    T_word16        ly1;
    T_word16        lx2;
    T_word16        ly2;

    /* window mode */
    E_TxtboxMode    mode;

    /* default window justification */
    E_TxtboxJustify justify;

    /* maximum length in bytes for data */
    T_word32        maxlength;

    /* hotkey for window selection */
    T_word16        hotkey;

    /* is data numeric or alpha-numeric? */
    E_Boolean       numericonly;

    /* window colors */
    T_byte8         textcolor;
    T_byte8         textshadow;
    T_byte8         backcolor;
    T_byte8         bordercolor1;
    T_byte8         bordercolor2;
    T_byte8         hbackcolor;
    T_byte8         hbordercolor1;
    T_byte8         hbordercolor2;
    T_byte8         htextcolor;

    /* cursor position (in characters) */
    T_word32        cursorl;
    /* cursor position (in rows) */
    T_word16        cursorline;
    /* cursor position (in pixels) */
    T_word16        cursorx;
    T_word16        cursory;

    /* row of data that display starts at */
    T_word16        windowstartline;
    /* number of rows visible in the window */
    T_word16        windowrows;
    /* total number of rows of data */
    T_word16        totalrows;

    /* callback routine to be activated when an event happens to this window */
    T_TxtboxHandler Txtboxcallback;

    /* window font */
    T_resource      font;

    /* height of font */
    T_byte8         fontheight;

    /* is this text box selected */
    E_Boolean       isselected;

    /* is this text box full */
    E_Boolean       isfull;

    /* parsing data */
    T_word32 *linestarts;
    T_word16 *linewidths;

    /* scroll bar data */
    T_buttonID sbupID;
    T_buttonID sbdnID;
    T_graphicID sbgrID;
    /* start y position of sbar box */
    T_word16 sbstart;
    /* length of sbar box */
    T_word16 sblength;

} T_TxtboxStruct;

T_TxtboxID TxtboxCreate (T_word16 x1,
                         T_word16 y1,
                         T_word16 x2,
                         T_word16 y2,
                         const char *fontname,
                         T_word32 maxlength,
                         T_word16 hotkey,
                         E_Boolean numericonly,
                         E_TxtboxJustify justify,
                         E_TxtboxMode boxmode,
                         T_TxtboxHandler callback);

T_void TxtboxDelete (T_TxtboxID TxtboxID);
T_void TxtboxCleanUp (T_void);

T_void TxtboxCursTop  (T_TxtboxID TxtboxID);
T_void TxtboxCursBot  (T_TxtboxID TxtboxID);
T_void TxtboxCursHome (T_TxtboxID TxtboxID);
T_void TxtboxCursEnd  (T_TxtboxID TxtboxID);
T_void TxtboxCursUp   (T_TxtboxID TxtboxID);
T_void TxtboxCursDn   (T_TxtboxID TxtboxID);
T_void TxtboxCursLeft (T_TxtboxID TxtboxID);
T_void TxtboxCursRight(T_TxtboxID TxtboxID);
T_void TxtboxCursPgUp (T_TxtboxID TxtboxID);
T_void TxtboxCursPgDn (T_TxtboxID TxtboxID);
T_void TxtboxCursSetRow (T_TxtboxID TxtboxID, T_word16 row);

T_void TxtboxCursMoveTo (T_TxtboxID TxtboxID, T_word16 x, T_word16 y);

T_void TxtboxAppendKey (T_TxtboxID TxtboxID, T_word16 scankey);
T_void TxtboxAppendString (T_TxtboxID TxtboxID, const char *data);
T_void TxtboxSetData (T_TxtboxID TxtboxID, const char *data);
T_void TxtboxSetNData (T_TxtboxID TxtboxID, T_byte8 *data, T_word32 len);

T_byte8 *TxtboxGetData (T_TxtboxID TxtboxID);
T_word32 TxtboxGetDataLength (T_TxtboxID TxtboxID);

T_void TxtboxSetColor (T_TxtboxID TxtboxID,
                       T_byte8 txtcolor,
                       T_byte8 bkcolor,
                       T_byte8 txtshadow,
                       T_byte8 bordclr1,
                       T_byte8 bordclr2);
T_void TxtboxBackSpace (T_TxtboxID TxtboxID);

T_void TxtboxKeyControl (E_keyboardEvent event, T_word16 scankey);
T_void TxtboxMouseControl (E_mouseEvent event, T_word16 x, T_word16 y, T_buttonClick button);
T_void TxtboxRepaginate (T_TxtboxID TxtboxID);
T_void TxtboxRepaginateAll (T_TxtboxID TxtboxID);
T_word32 TxtboxScanRow (T_TxtboxID TxtboxID, T_word16 rowinc, T_word16 ox);
T_void TxtboxDrawCallBack (T_graphicID graphicID, T_word16 index);
T_void TxtboxUpdate (T_TxtboxID TxtboxID);
T_void TxtboxNextBox (T_void);
T_void TxtboxLastBox (T_void);
T_void TxtboxFirstBox (T_void);
E_Boolean TxtboxIsAt (T_TxtboxID TxtboxID, T_word16 x, T_word16 y);
E_Boolean TxtboxIsSelected (T_TxtboxID TxtboxID);

T_void TxtboxSetScrollBarObjIDs (T_TxtboxID TxtboxID,
                                 T_buttonID sbupID,
                                 T_buttonID sbdnID,
                                 T_graphicID sbgrID);

T_void TxtboxHandleSBDn (T_buttonID buttonID);
T_void TxtboxHandleSBUp (T_buttonID buttonID);
T_void TxtboxUpdateSB (T_TxtboxID TxtboxID);
T_void TxtboxMoveSB (T_TxtboxID TxtboxID, T_word16 x);
T_void TxtboxAllocLine (T_TxtboxID TxtboxID);
T_void TxtboxSetCallback (T_TxtboxID TxtboxID, T_TxtboxHandler newcallback);
T_void TxtboxSetNumericOnlyFlag (T_TxtboxID TxtboxID, E_Boolean newflag);
E_TxtboxAction TxtboxGetAction (T_void);
E_Boolean TxtboxValidateID (T_TxtboxID TxtboxID);
T_word16 TxtboxGetSelectionNumber (T_TxtboxID TxtboxID);
T_void TxtboxSetMaxLength (T_TxtboxID TxtboxID,T_word32 newmaxlen);
T_word16 TxtboxCanFit (T_TxtboxID TxtboxID, T_byte8 *stringToFit);

T_void *TxtboxGetStateBlock(T_void) ;
T_void TxtboxSetStateBlock(T_void *p_state) ;

#endif
