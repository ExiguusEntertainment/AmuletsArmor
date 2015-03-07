/****************************************************************************/
/*    FILE:  GRAPHIC.H                                                      */
/****************************************************************************/

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include "GENERAL.H"
#include "RESOURCE.H"
#include "PNG.h"

#define MAX_GRAPHICS 200

typedef T_void *T_graphicID;
typedef T_void (*T_graphicHandler)(T_graphicID graphicID, T_word16 index) ;

typedef struct
{
	T_word16 locx;
	T_word16 locy;
	T_word16 xoff;
	T_word16 yoff;
	T_word16 width;
	T_word16 height;

	T_png png;

	E_Boolean visible;
	E_Boolean changed;
	T_byte8 shadow;
	T_graphicHandler predrawcallback;
	T_graphicHandler postdrawcallback;
	T_word16 predrawcbinfo;
	T_word16 postdrawcbinfo;

} T_graphicStruct ;

T_graphicID GraphicCreate (T_word16 lx,  T_word16 ly, const char *graphicname);

T_void GraphicSetPreCallBack (T_graphicID graphicID, T_graphicHandler graphichandler, T_word16 index);
T_void GraphicSetPostCallBack (T_graphicID graphicID, T_graphicHandler graphichandler, T_word16 index);

T_void GraphicDelete (T_graphicID graphicID);
T_void GraphicCleanUp (T_void);
T_void GraphicUpdate (T_graphicID graphicID);
T_void GraphicUpdateAllGraphics (T_void);
T_void GraphicUpdateAllGraphicsBuffered (T_void);
T_void GraphicUpdateAllGraphicsForced (T_void) ;
T_void GraphicDrawAt (T_graphicID graphicID, T_word16 lx, T_word16 ly);
T_void GraphicClear  (T_graphicID graphicID, T_byte8 clearcolor);

T_void GraphicHide (T_graphicID graphicID);
T_void GraphicShow (T_graphicID graphicID);

T_void GraphicSetShadow (T_graphicID graphicID, T_byte8 newshadow);
T_void GraphicSetOffSet (T_graphicID graphicID, T_word16 offx, T_word16 offy);
T_void GraphicSetSize (T_graphicID graphicID, T_word16 sizex, T_word16 sizey);

E_Boolean GraphicIsVisible (T_graphicID graphicID);
E_Boolean GraphicIsAt (T_graphicID graphicID, T_word16 lx, T_word16 ly);
E_Boolean GraphicIsShadowed (T_graphicID graphicID);
E_Boolean GraphicIsOffSet (T_graphicID graphicID);

T_void GraphicDrawToActualScreen (T_void);
T_void GraphicDrawToCurrentScreen (T_void);

//T_void GraphicSetResource (T_graphicID graphicID, T_resource newresource);
//T_resource GraphicGetResource (T_graphicID graphicID);

T_void GraphicSetPNG(T_graphicID graphicID, T_png aPNG);
T_png GraphicGetPNG(T_graphicID graphicID);

T_void *GraphicGetStateBlock(T_void) ;
T_void GraphicSetStateBlock(T_void *p_state) ;

#endif
