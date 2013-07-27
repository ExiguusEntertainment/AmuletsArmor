/****************************************************************************/
/*    FILE:  3D_VIEW.H                                                      */
/****************************************************************************/

#ifndef _3D_VIEW_H_
#define _3D_VIEW_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

/*
#define VIEW3D_WIDTH 300
#define VIEW3D_HEIGHT 140
#define VIEW3D_HALF_WIDTH (VIEW3D_WIDTH/2)
#define VIEW3D_HALF_HEIGHT (VIEW3D_HEIGHT/2)
*/
#define MAX_VIEW3D_WIDTH  320
#define MAX_VIEW3D_HEIGHT 200

#define VIEW3D_PASSABLE_BIT 0x1000

extern T_sword16 VIEW3D_WIDTH ;
extern T_sword16 VIEW3D_HEIGHT ;
extern T_sword16 VIEW3D_HALF_WIDTH ;
extern T_sword16 VIEW3D_HALF_HEIGHT ;
extern T_sword16 VIEW3D_CLIP_LEFT ;
extern T_sword16 VIEW3D_CLIP_RIGHT ;

extern T_byte8 P_shadeIndex[16384] ;

T_void View3dInitialize(T_void) ;

T_void View3dSetView(
           T_sword16 x,
           T_sword16 y,
           T_sword32 height,
           T_word16 angle) ;

T_void View3dGetView(
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_sword32 *p_height,
           T_word16 *p_angle) ;

T_void View3dDrawView(T_void) ;

T_void View3dDisplayView(T_void) ;

T_word16 CalculateDistance(
             T_sword32 x1,
             T_sword32 y1,
             T_sword32 x2,
             T_sword32 y2) ;

T_word16 CalculateEstimateDistance(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2) ;

#define View3dFindSectorNum(x, y) IFindSectorNum(x, y)

T_word16 IFindSectorNum(T_sword16 x, T_sword16 y) ;

T_void View3dSetSize(T_word16 width, T_word16 height) ;

T_void View3dClipCenter(T_word16 centerWidth) ;

T_3dObject *View3dAllocateObject(T_void) ;

T_void View3dFreeObject(T_3dObject *p_obj) ;

T_void View3dSetHeight(T_sword32 height) ;

T_word16 View3dGetObjectAtColumn(
             T_word16 objPos,
             T_3dObject **p_obj,
             T_word16 column) ;

T_word16 View3dGetObjectAtXY(
             T_word16 objPos,
             T_3dObject **p_obj,
             T_word16 x,
             T_word16 y) ;

T_sword32 View3dGetUpDownAngle(T_void) ;

T_void View3dSetUpDownAngle(T_sword32 alpha) ;

T_void View3dAddObject(T_3dObject *p_obj) ;

T_void View3dRemoveObject(T_3dObject *p_obj) ;

T_void View3dUpdateSectorLightAnimation(T_void) ;

#define View3dGetSectorEnterSound(sector) \
            (G_3dSectorInfoArray[sector].enterSound)

#define View3dGetSectorEnterSoundRadius(sector) \
            (G_3dSectorInfoArray[sector].enterSoundRadius)

#define LINE_IS_IMPASSIBLE           0x0001
#define LINE_IS_CREATURE_IMPASSIBLE  0x0002
#define LINE_IS_TWO_SIDED            0x0004
#define LINE_IS_TRANSLUCENT          0x0020
#define LINE_IS_ALWAYS_SOLID         0x0040
#define LINE_IS_INVISIBLE            0x0080
#define LINE_IS_AUTOMAPPED           0x0100
#define LINE_HAS_BEEN_SEEN           0x8000

T_word16 View3dFindClosestLine(T_sword16 x, T_sword16 y) ;

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
T_void View3dTellMouseAt(T_sword16 x, T_sword16 y) ;

T_word16 View3dGetTextureSideNum(T_void) ;
#endif

T_word16 View3dGetSectorSide(T_word16 lineNum, T_sword16 x, T_sword16 y) ;

T_word16 View3dFindSide(T_sword16 x, T_sword16 y) ;

T_void View3dCheckObjectListEmpty(T_void) ;

T_void View3dFinish(T_void);

T_void View3dSetDarknessAdjustment(T_sbyte8 darkAdjust);

T_void View3dRemapSectors(T_void);

T_void View3dUnmapSectors(T_void);


#endif

/****************************************************************************/
/*    END OF FILE:  3D_VIEW.H                                                 */
/****************************************************************************/
