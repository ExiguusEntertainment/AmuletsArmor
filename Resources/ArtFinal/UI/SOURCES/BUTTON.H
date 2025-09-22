/****************************************************************************/
/*    FILE:  BUTTON.H                                                       */
/****************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

#define MAX_BUTTONS 100

typedef T_void *T_buttonID;
typedef T_void (*T_buttonHandler)(T_buttonID buttonID) ;
typedef struct
{
   T_word16 locx;
   T_word16 locy;
   T_word16 width;
   T_word16 height;
   T_word16 scancode;
   E_Boolean toggle;
   E_Boolean pushed;
   E_Boolean enabled;
   E_Boolean changed;
   T_resource buttonpic;
   T_buttonHandler p_callback;

} T_buttonStruct ;


T_buttonID ButtonAdd (T_word16 lx,
                      T_word16 ly,
                      T_byte8 *bmname,
                      E_Boolean toggletype,
                      T_word16 keyassoc,
                      T_buttonHandler p_cbroutine);

T_buttonID ButtonInit (T_word16 lx,
                      T_word16 ly,
                      T_byte8 *bmname,
                      E_Boolean toggletype,
                      T_word16 keyassoc,
                      T_buttonHandler p_cbroutine);

T_void ButtonDel (T_buttonID buttonID);
T_void ButtonKill (T_buttonID buttonID);
T_void ButtonPush (T_buttonID buttonID);
T_void ButtonPushOnKey (T_void);
T_void ButtonPushLoc (T_word16 locx, T_word16 locy);
T_void ButtonDown (T_buttonID buttonID);
T_void ButtonUp (T_buttonID buttonID);
T_void ButtonEnable (T_buttonID buttonID);
T_void ButtonDisable (T_buttonID buttonID);
T_void ButtonDoCallback (T_buttonID buttonID);
T_void ButtonUpdateAllButtons (T_void);
T_void ButtonDrawAllButtons (T_void);
T_void ButtonDraw (T_buttonID buttonID);
T_void ButtonDrawAt (T_buttonID buttonID, T_word16 lx, T_word16 ly);
T_void ButtonUpdate (T_buttonID buttonID);
T_void ButtonCleanUp (T_void);
T_buttonID ButtonGetKey (T_word16 keycode);
T_buttonID ButtonGetLoc (T_word16 x, T_word16 y);

E_Boolean ButtonIsPushed (T_buttonID buttonID);
E_Boolean ButtonIsEnabled (T_buttonID buttonID);
E_Boolean ButtonIsAt (T_buttonID buttonID, T_word16 lx, T_word16 ly);

#endif
