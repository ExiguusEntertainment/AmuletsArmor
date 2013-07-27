/****************************************************************************/
/*    FILE:  TXTFRM.H                                                       */
/****************************************************************************/

#ifndef _TXTFORM_H_
#define _TXTFORM_H_

#define MAX_TXTFRMS 100
#define MAX_TXTFRM_WIDTH 100
#define MAX_TXTFRM_LINES 100

typedef T_void *T_txtfrmID;
typedef T_void (*T_txtfrmHandler)(T_txtfrmID txtfrmID) ;

typedef enum {
	 TXTFRM_MODE_EDIT,
	 TXTFRM_MODE_VIEW,
	 TXTFRM_MODE_SELECT,
	 TXTFRM_MODE_UNKNOWN
} E_txtfrmType ;

typedef struct
{
	T_graphicID 	 p_graphicID;
	T_byte8			 *data[MAX_TXTFRM_LINES];
	E_Boolean       linechanged[MAX_TXTFRM_LINES];
	T_word16        hotkey;
	T_byte8			 fcolor;
	T_byte8			 bcolor;
	T_byte8			 cursorx;
	T_byte8         cursory;
	T_byte8         dispwiny;
	T_byte8			 maxvislines;
	E_txtfrmType    frmtype;
	T_resource 	    font ;
} T_txtfrmStruct;


T_txtfrmID txtfrmCreate (T_word16 x1,
								 T_word16 y1,
								 T_word16 x2,
								 T_word16 y2,
								 T_byte8 *fontname,
								 E_txtfrmType frmtype);
T_void txtfrmDelete (T_txtfrmID txtfrmID);
T_void txtfrmCleanUp (T_void);


/*T_void txtfrmSetFont (T_txtfrmID txtfrmID, T_resource newfont);*/
T_void txtfrmSetColor (T_txtfrmID txtfrmID, \
								T_byte8 fcolor,
								T_byte8 bcolor);
T_void txtfrmSetHotKey (T_txtfrmID txtfrmID, T_word16 hotkey);


T_void txtfrmInsert (T_txtfrmID txtfrmID, char *string);
T_void txtfrmAppend (T_txtfrmID txtfrmID, char *string);
T_byte8 *txtfrmGetLine (T_txtfrmID txtfrmID);


T_void txtfrmCursTop    (T_txtfrmID txtfrmID);
T_void txtfrmCursBottom (T_txtfrmID txtfrmID);
T_void txtfrmCursMove   (T_txtfrmID txtfrmID,
								  T_byte8 xchar,
								  T_byte8 ychar);
T_void txtfrmPgDn (T_txtfrmID txtfrmID);
T_void txtfrmPgUp (T_txtfrmID txtfrmID);
T_void txtfrmCursDn (T_txtfrmID txtfrmID);
T_void txtfrmCursUp (T_txtfrmID txtfrmID);
T_void txtfrmCursRight (T_txtfrmID txtfrmID);
T_void txtfrmCursLeft (T_txtfrmID txtfrmID);

T_void txtfrmNewLine (T_txtfrmID txtfrmID, T_byte8 yloc);
T_void txtfrmBackSpace (T_txtfrmID txtfrmID);

T_byte8  txtfrmXtoCX (T_txtfrmID txtfrmID, T_word16 xloc);
T_byte8  txtfrmYtoCY (T_txtfrmID txtfrmID, T_word16 yloc);
T_word16 txtfrmCXtoX (T_txtfrmID txtfrmID, T_byte8 xcurs);
T_word16 txtfrmCYtoY (T_txtfrmID txtfrmID, T_byte8 ycurs);


T_graphicID txtfrmGetTextGraphicID (T_txtfrmID txtfrmID);
T_graphicID txtfrmGetBackGraphicID (T_txtfrmID txtfrmID);


T_txtfrmID txtfrmIsAt (T_word16 lx, T_word16 ly);


T_void txtfrmTextDrawCallBack (T_graphicID graphicID, T_word16 index);

E_Boolean txtfrmKeyControl (E_keyboardEvent event, T_byte8 scankey);
T_void txtfrmMouseControl  (E_mouseEvent event,
									  T_word16 x,
									  T_word16 y,
									  E_Boolean button);
#endif
