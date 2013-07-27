/****************************************************************************/
/*    FILE:  VIEWREGN.H                                                     */
/****************************************************************************/

#ifndef _VIEWREGN_H_
#define _VIEWREGN_H_

#define VIEW_REGION_DEFAULT_X 200
#define VIEW_REGION_DEFAULT_Y 160
#define VIEW_REGION_HEIGHT 40
#define VIEW_REGION_WIDTH 40
#define VIEW_REGION_INTERIOR_HEIGHT (VIEW_REGION_HEIGHT-2)
#define VIEW_REGION_INTERIOR_WIDTH (VIEW_REGION_WIDTH-2)

T_void ViewRegionGoto(
           T_word16 mapX,
           T_word16 mapY,
           T_word16 width,
           T_word16 height) ;

T_void ViewRegionDraw(T_void) ;

T_void ViewRegionInitialize(T_void) ;

T_void ViewRegionFinish(T_void) ;

T_void ViewRegionMoveToMouse(T_word16 mouseX, T_word16 mouseY) ;

#endif

/****************************************************************************/
/*    END OF FILE:  VIEWREGN.H                                              */
/****************************************************************************/
