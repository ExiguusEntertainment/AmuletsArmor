/****************************************************************************/
/*    FILE:  BUTTON.H                                                       */
/****************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "BUTTON.H"
#include "GENERAL.H"
#include "GRAPHIC.H"
#include "KEYBOARD.H"
#include "MOUSEMOD.H"
#include "TEXT.H"

#define MAX_BUTTONS 100

typedef T_void *T_buttonID;
typedef T_void (*T_buttonHandler)(T_buttonID buttonID) ;
typedef enum
{
	BUTTON_ACTION_NO_ACTION,
	BUTTON_ACTION_PUSHED,
	BUTTON_ACTION_RELEASED,
	BUTTON_ACTION_OTHER,
	BUTTON_ACTION_UNKNOWN
} E_buttonAction;

typedef struct
{
	T_graphicID p_graphicID;
   T_resource normalpic;
   T_resource selectpic;
	T_word16  scancode;
	E_Boolean toggle;
	E_Boolean pushed;
	E_Boolean enabled;
	T_textID textID;
	T_buttonHandler p_callback;
	T_buttonHandler p_callback2;
	E_Boolean mouseheld;
    T_word32 data;
    T_word16 subdata;
    T_word32 tag;
} T_buttonStruct ;


T_buttonID ButtonCreate (T_word16 lx,
					  T_word16 ly,
					  T_byte8 *bmname,
					  E_Boolean toggletype,
					  T_word16 keyassoc,
					  T_buttonHandler p_cbroutine,
					  T_buttonHandler p_cbroutine2);

T_void ButtonSetSelectPic (T_buttonID buttonID, char *picname);

T_void ButtonDelete (T_buttonID buttonID);
T_void ButtonCleanUp (T_void);

T_void ButtonKeyControl   (E_keyboardEvent event, T_word16 scankey);
T_void ButtonMouseControl (E_mouseEvent event,
									T_word16 x,
									T_word16 y,
									T_buttonClick button);

T_void ButtonDown (T_buttonID buttonID);
T_void ButtonDownNoAction (T_buttonID buttonID);
T_void ButtonDownNum (T_word16 num);
T_void ButtonUp (T_buttonID buttonID);
T_void ButtonUpNoAction (T_buttonID buttonID);
T_void ButtonUpNum (T_word16 num);

T_void ButtonEnable (T_buttonID buttonID);
T_void ButtonDisable (T_buttonID buttonID);
T_void ButtonDoCallback (T_buttonID buttonID);
T_void ButtonDoCallback2 (T_buttonID buttonID);

E_Boolean ButtonIsPushed (T_buttonID buttonID);
E_Boolean ButtonIsEnabled (T_buttonID buttonID);
E_Boolean ButtonIsAt (T_buttonID buttonID, T_word16 lx, T_word16 ly);
E_Boolean ButtonIsValid (T_buttonID buttonID);

T_graphicID ButtonGetGraphic (T_buttonID buttonID);
T_buttonID ButtonGetByLoc (T_word16 x, T_word16 y);
T_buttonID ButtonGetByKey (T_word16 keycode);
T_void ButtonDrawCallBack (T_graphicID graphicID, T_word16 info);

T_void ButtonRedrawAllButtons (T_void);
T_void ButtonRedraw (T_buttonID buttonID);

T_void ButtonSetText (T_buttonID buttonID, const T_byte8 *string, T_byte8 color);
T_void ButtonSetData (T_buttonID buttonID, T_word32 data);
T_void ButtonSetSubData (T_buttonID buttonID, T_word32 subdata);
T_void ButtonSetPicture (T_buttonID buttonID, T_byte8 *picname);
T_word32 ButtonGetData (T_buttonID buttonID);
T_word32 ButtonGetSubData (T_buttonID buttonID);
T_void ButtonSetFont (T_buttonID buttonID, T_byte8 *fntname);
T_void ButtonSetCallbacks (T_buttonID buttonID, T_buttonHandler callback1, T_buttonHandler callback2);
E_buttonAction ButtonGetAction (T_void);
E_Boolean ButtonIsMouseOverButton (T_void);

#if 0
T_void ButtonDisableAll(T_void) ;
T_void ButtonEnableAll(T_void) ;
#endif

T_void *ButtonGetStateBlock(T_void) ;
T_void ButtonSetStateBlock(T_void *p_state) ;

#endif
