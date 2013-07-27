/****************************************************************************/
/*    FILE:  FORM.H                                                         */
/****************************************************************************/

#ifndef _FORM_H_
#define _FORM_H_

#include "GENERAL.H"
#include "SLIDR.H"
#include "TXTBOX.H"
#include "TXTFLD.H"

/*
 #define MAX_FORM_BUTTONS  100
 #define MAX_FORM_FIELDS   50
 #define MAX_FORM_GRAPHICS 200
 #define MAX_FORM_TEXTS    50
 #define MAX_FORM_BOXES    50
 #define MAX_FORM_SLIDERS  15
 */
#define MAX_FORM_OBJECTS  400

typedef enum {
    FORM_OBJECT_UNDEFINED=0,
    FORM_OBJECT_GRAPHIC=1,
    FORM_OBJECT_TEXT=2,
    FORM_OBJECT_BUTTON=3,
    FORM_OBJECT_TEXTBUTTON=4,
    FORM_OBJECT_TEXTBOX=5,
    FORM_OBJECT_TEXTFIELD=6,
    FORM_OBJECT_SLIDER=7,
    FORM_OBJECT_UNKNOWN
} E_formObjectType;

typedef T_void *T_formObjectID;
typedef T_void (*T_formCallBackRoutine)(
        E_formObjectType objtype,
        T_word16 objstatus,
        T_word32 objID);

typedef struct {
    E_formObjectType objtype;
    T_formObjectID objID;
    T_word16 status;
    T_word32 numID;
} T_formObjectStruct;

T_formObjectID FormAddButton(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *picturename,
        E_Boolean toggletype,
        T_word16 hotkey,
        T_word32 idnum);

T_formObjectID FormAddTextButton(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *data,
        T_byte8 *picturename,
        T_byte8 *fontname,
        T_byte8 fcolor,
        T_byte8 bcolor,
        E_Boolean toggletype,
        T_word16 hotkey,
        T_word32 idnum);

T_formObjectID FormAddText(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *data,
        T_byte8 *fontname,
        T_byte8 fcolor,
        T_byte8 bcolor,
        T_word32 idnum);

T_formObjectID FormAddGraphic(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *picturename,
        T_word32 idnum);

T_formObjectID FormAddTextField(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_byte8 *fontname,
        T_byte8 maxfieldlength,
        E_TxtfldDataType datatype,
        T_word16 hotkey,
        T_word32 idnum);

T_formObjectID FormAddTextBox(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_byte8 *fontname,
        T_word32 maxlength,
        T_byte8 hotkey,
        E_Boolean numericonly,
        E_TxtboxJustify justify,
        E_TxtboxMode boxmode,
        T_word32 idnum);

T_formObjectID FormAddSlider(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word32 idnum);

T_void FormReportButton(T_buttonID buttonID);
T_void FormReportField(T_TxtfldID TxtfldID);
T_void FormReportTextBox(T_TxtboxID TxtboxID);
T_void FormReportSlider(T_sliderID sliderID);

T_void FormLoadFromFile(T_byte8 *filename);
T_void FormDeleteObject(T_formObjectID objectID);
T_void FormCleanUp(T_void);
T_void FormHandleMouse(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        E_Boolean button);
T_void FormHandleKey(E_keyboardEvent event, T_word16 scankey);
T_void FormSetCallbackRoutine(T_formCallBackRoutine newcallback);

T_formObjectID FormGetObjID(T_word32 numID);
T_formObjectID FormFindObjID(T_word32 numID);
T_void FormGenericControl(E_Boolean *exitflag);

/* LES: 02/28/96 */
T_void FormGenericControlStart(T_void);
T_void FormGenericControlEnd(T_void);
T_void FormGenericControlUpdate(T_void);

T_void FormPush(T_void);
T_void FormPop(T_void);

#endif
