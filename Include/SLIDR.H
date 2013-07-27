/****************************************************************************/
/*    FILE:  SLIDR.H                                                        */
/****************************************************************************/
#ifndef _SLIDR_H_
#define _SLIDR_H_

#include "GENERAL.H"
#include "GRAPHIC.H"
#include "MOUSEMOD.H"

#define MAX_SLIDERS 10
#define SLIDER_LINE1_COLOR 213
#define SLIDER_LINE2_COLOR 218
#define SLIDER_BACKGROUND_COLOR 76

typedef T_void *T_sliderID;
typedef T_void (*T_sliderHandler)(T_sliderID sliderID) ;

typedef struct
{
   T_word16 lx1;
   T_word16 ly1;
   T_word16 lx2;

   T_word16 sliderx;
   T_word16 curval;
   T_sliderHandler callback;

   T_graphicID knobgraphic;
} T_sliderStruct ;

T_sliderID SliderCreate (T_word16 lx1, T_word16 ly1, T_word16 lx2);

T_void SliderSetCallBack (T_sliderID sliderID, T_sliderHandler sliderhandler);

T_void SliderDelete (T_sliderID sliderID);
T_void SliderCleanUp (T_void);

T_void SliderUpdate (T_sliderID sliderID);
T_void SliderUpdateAllSliders (T_void);

E_Boolean SliderIsAt (T_sliderID sliderID, T_word16 lx, T_word16 ly);

T_void SliderSetValue (T_sliderID sliderID, T_word16 value);
T_word16 SliderGetValue (T_sliderID sliderID);

T_void SliderMouseControl (E_mouseEvent event,
								   T_word16 x,
								   T_word16 y,
								   T_buttonClick button);
T_void *SliderGetStateBlock(T_void) ;
T_void SliderSetStateBlock(T_void *p_state) ;

#endif
