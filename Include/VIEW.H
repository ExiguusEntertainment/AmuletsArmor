/****************************************************************************/
/*    FILE:  VIEW.H                                                         */
/****************************************************************************/

#ifndef _VIEW_H_
#define _VIEW_H_

#include "VIEWFILE.H"

#define MAX_PLAYER_STARTS 8

#define INT_ANGLE_0     0x0000
#define INT_ANGLE_45    0x2000
#define INT_ANGLE_90    0x4000
#define INT_ANGLE_135   0x6000
#define INT_ANGLE_180   0x8000
#define INT_ANGLE_225   0xA000
#define INT_ANGLE_270   0xC000
#define INT_ANGLE_315   0xE000

#define VIEW_TARGET_NONE 0x7FFF

#define VIEW3D_UPPER_LEFT_X  4
#define VIEW3D_UPPER_LEFT_Y  3

typedef T_void (*T_viewOverlayHandler)(T_word16 left,
                                       T_word16 top,
                                       T_word16 right,
                                       T_word16 bottom) ;

typedef T_byte8 T_viewPalette ;
#define VIEW_PALETTE_STANDARD          0
#define VIEW_PALETTE_WATER             1
#define VIEW_PALETTE_MAIN_TITLE        2
#define VIEW_PALETTE_UNKNOWN           3

T_void ViewInitialize(T_void) ;

T_void ViewFinish(T_void) ;

T_void ViewDraw(T_void) ;

T_void ViewSetOverlayHandler(T_viewOverlayHandler handler) ;

T_sword16 ViewCreateObject(T_void) ;

T_void ViewRemoveObject(T_word16 objectNum) ;

T_void ViewDeclareStaticObject(
           T_byte8 objNum,
           T_word16 mapX,
           T_word16 mapY,
           T_byte8 picNum) ;

T_void ViewChangeObjectPicture(T_word16 objectNum, T_byte8 *picName) ;

T_void ViewChangeObjectPictureDirectly(T_word16 objectNum, T_byte8 *pic) ;

T_void ViewMoveObject(T_3dObject *p_obj, T_sword16 x, T_sword16 y) ;

T_void ViewDeclareMoveableObject(
           T_3dObject *p_obj,
           T_word16 mapX,
           T_word16 mapY,
           T_byte8 picNum) ;

T_void ViewCheckFloorActivationAtXY(T_word16 x, T_word16 y) ;

T_word16 ViewGetForwardWallActivation(T_void) ;

T_sword16 ViewStepObject(T_word16 objNum, T_word16 angle, T_sword16 dist, T_word16 boundary) ;

T_void ViewGetObjectXY(T_word16 objNum, T_sword16 *p_x, T_sword16 *p_y) ;

T_void ViewGetObjectHeight(T_word16 objNum, T_sword16 *p_height) ;

T_void ViewSetObjectHeight(T_word16 objNum, T_sword16 height) ;

T_void ViewObjectStartAnimation(T_word16 objNum) ;

T_void ViewObjectStopAnimation(T_word16 objNum) ;

T_void ViewGetObjectAngle(T_word16 objNum, T_word16 *p_angle) ;

T_void ViewSetObjectAngle(T_word16 objNum, T_word16 angle) ;

T_word16 ViewGetObjectWidth(T_word16 objNum) ;

T_word16 ViewGetObjectType(T_word16 objNum) ;

T_void ViewSetObjectType(T_word16 objNum, T_word16 type) ;

T_void ViewMakeObjectImpassable(T_word16 objNum) ;

T_void ViewMakeObjectPassable(T_word16 objNum) ;

E_Boolean ViewCheckObjectCollide(T_word16 objNum, T_sword16 x, T_sword16 y) ;

T_byte8 *ViewGetObjectTypeName(T_word16 type) ;

T_void ViewUpdatePlayer(T_void) ;

T_sword16 ViewGetPlayerHeight(T_void) ;

T_3dObject *ViewGetMiddleTarget(T_void) ;

T_3dObject *ViewGetXYTarget(T_word16 x, T_word16 y) ;

T_void ViewChangeObjectOrientation(T_word16 objNum, T_orientation ori) ;

/* !!! */
T_void ViewHardCodedSetObject(T_word16 objNum) ;
T_void ViewHardCodedSetObject2(T_word16 objNum) ;

E_Boolean ViewIsAboveGround(T_void);

T_void ViewObjectsUnload(T_void) ;

E_Boolean ViewIsAt(T_word16 x, T_word16 y) ;

T_void ViewEarthquakeOn(T_word32 duration) ;

T_void ViewEarthquakeOff(T_void) ;

E_Boolean ViewIsEarthquakeOn(T_void) ;

T_void ViewSetPalette(T_viewPalette viewPalette) ;

T_void ViewSetDarkSight(T_sbyte8 darkSightValue) ;

T_sbyte8 ViewGetDarkSight(T_void) ;

T_void ViewOffsetView(T_word16 angle) ;

T_void ViewUpdateFormsOverViewEnable(void);

T_void ViewUpdateFormsOverViewDisable(void);

#endif

/****************************************************************************/
/*    END OF FILE:  VIEW.H                                                  */
/****************************************************************************/
