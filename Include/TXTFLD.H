/****************************************************************************/
/*    FILE:  TXTFLD.H                                                       */
/****************************************************************************/
#ifndef _TXTFLD_H_
#define _TXTFLD_H_

#include "GENERAL.H"
#include "GRAPHIC.H"
#include "MOUSEMOD.H"
#include "KEYBOARD.H"

#define MAX_TXTFLDS 40
#define MAX_TXTFLD_WIDTH 100

typedef enum {
	 TXTFLD_NUMERIC_ONLY,
	 TXTFLD_ALPHANUMERIC,
     TXTFLD_NOEDIT,
	 TXTFLD_NUMERIC_UNKNOWN
} E_TxtfldDataType ;

typedef T_void *T_TxtfldID;
typedef T_void (*T_TxtfldHandler)(T_TxtfldID TxtfldID) ;

typedef struct
{
	/* a 'graphic' structure */
	T_graphicID 	 p_graphicID;
	/* the data string to be displayed in the box */
	T_byte8			 data[MAX_TXTFLD_WIDTH];
	/* an assignable hotkey for control use */
	T_word16        hotkey;
	/* the foreground color for text */
	T_byte8			 fcolor;
	T_byte8			 bcolor;
	/* the current cursor position in the box */
	T_byte8			 cursorx;
	/* the maximum field length */
	T_byte8         fieldlength;
	/* is the cursor on ? */
	E_Boolean       cursoron;
	/* is the field currently full ? */
	E_Boolean       fieldfull;
	/* is the field numeric only ? */
	E_TxtfldDataType datatype;
	/* font for the field */
	T_resource 	    font;
	/* callback function for the return key */
	T_TxtfldHandler retfunction;
	/* callback function for the tab key */
	T_TxtfldHandler tabfunction;
} T_TxtfldStruct;


T_TxtfldID TxtfldCreate (T_word16 x1,
								 T_word16 y1,
								 T_word16 x2,
								 T_word16 y2,
								 T_byte8  *fontname,
								 T_byte8  maxfieldlen,
								 T_word16 key,
								 E_TxtfldDataType datatype,
								 T_TxtfldHandler retfunct,
								 T_TxtfldHandler tabfunct);
T_void TxtfldDelete (T_TxtfldID TxtfldID);
T_void TxtfldCleanUp (T_void);
T_void TxtfldSetColor (T_TxtfldID TxtfldID,
								T_byte8 fcolor,
								T_byte8 bcolor);
T_void TxtfldSetData (T_TxtfldID txtfldID, T_byte8 *newdata);
T_byte8 *TxtfldGetData (T_TxtfldID txtfldID);

T_void TxtfldCursRight (T_TxtfldID TxtfldID);
T_void TxtfldCursLeft (T_TxtfldID TxtfldID);
T_void TxtfldBackSpace (T_TxtfldID TxtfldID);
T_void TxtfldDel (T_TxtfldID TxtfldID);
T_graphicID TxtfldGetTextGraphicID (T_TxtfldID TxtfldID);
T_TxtfldID TxtfldIsAt (T_word16 lx, T_word16 ly);
T_void TxtfldTextDrawCallBack (T_graphicID graphicID, T_word16 index);
T_byte8 *TxtfldGetLine (T_TxtfldID TxtfldID);
E_Boolean TxtfldUseKey (T_TxtfldID TxtfldID, T_byte8 key);
T_void TxtfldRedraw (T_TxtfldID TxtfldID);
T_void TxtfldNextField (T_void);
T_void TxtfldMouseControl (E_mouseEvent event, T_word16 x, T_word16 y, E_Boolean button);
T_void TxtfldKeyControl (E_keyboardEvent event, T_byte8 scankey);
#endif
