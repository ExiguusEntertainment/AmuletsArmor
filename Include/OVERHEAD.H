/****************************************************************************/
/*    FILE:  OVERHEAD.H                                                     */
/****************************************************************************/
#ifndef _OVERHEAD_H_
#define _OVERHEAD_H_

typedef T_byte8 F_overheadFeature ;
#define OVERHEAD_FEATURE_ALL_WALLS     0x01        /* ---- ---1 */
#define OVERHEAD_FEATURE_OBJECTS       0x02        /* ---- --1- */
#define OVERHEAD_FEATURE_CREATURES     0x04        /* ---- -1-- */
#define OVERHEAD_FEATURE_ROTATE_VIEW   0x08        /* ---- 1--- */
#define OVERHEAD_FEATURE_TRANSPARENT   0x10        /* ---1 ---- */
#define OVERHEAD_FEATURE_PERSPECTIVE   0x20        /* --1- ---- */
#define OVERHEAD_FEATURE_TRANSLUCENT   0x40        /* -1-- ---- */
#define OVERHEAD_FEATURE_UNKNOWN       0x80        /* 11-- ---- */

#define OVERHEAD_FEATURE_DEFAULT (OVERHEAD_FEATURE_ALL_WALLS | \
                                  OVERHEAD_FEATURE_OBJECTS | \
                                  OVERHEAD_FEATURE_CREATURES)

#define OVERHEAD_SIZE_X_DEFAULT 75
#define OVERHEAD_SIZE_Y_DEFAULT 75

#define OVERHEAD_OFFSET_X_DEFAULT 3
#define OVERHEAD_OFFSET_Y_DEFAULT 3

#define OVERHEAD_ZOOM_DEFAULT 0x00002000L
#define OVERHEAD_ZOOM_MAX     0x00000200L
#define OVERHEAD_ZOOM_MIN     0x02000000L

#define OVERHEAD_MAX_PAGES 4
#define OVERHEAD_NUM_PAGES_DEFAULT 1

typedef enum {
    OVERHEAD_POSITION_UPPER_LEFT,
    OVERHEAD_POSITION_UPPER_RIGHT,
    OVERHEAD_POSITION_LOWER_RIGHT,
    OVERHEAD_POSITION_LOWER_LEFT,
    OVERHEAD_POSITION_CENTER,
    OVERHEAD_POSITION_BOTTOM_CENTER,
    OVERHEAD_POSITION_UNKNOWN
} E_overheadPosition ;

#define OVERHEAD_POSITION_DEFAULT   OVERHEAD_POSITION_UPPER_RIGHT

T_void OverheadInitialize(T_void) ;

T_void OverheadFinish(T_void) ;

T_void OverheadDraw(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom) ;

T_void OverheadSetSize(T_word16 sizeX, T_word16 sizeY) ;

T_word16 OverheadGetSizeX(T_void) ;

T_word16 OverheadGetSizeY(T_void) ;

T_void OverheadSetOffset(T_word16 offsetX, T_word16 offsetY) ;

T_word16 OverheadGetOffsetX(T_void) ;

T_word16 OverheadGetOffsetY(T_void) ;

T_void OverheadSetPosition(E_overheadPosition position) ;

E_overheadPosition OverheadGetPosition(T_void) ;

T_void OverheadSetFeatures(F_overheadFeature flags) ;

F_overheadFeature OverheadGetFeatures(T_void) ;

T_void OverheadAddFeatures(F_overheadFeature flags) ;

T_void OverheadRemoveFeatures(F_overheadFeature flags) ;

T_void OverheadSetZoomFactor(T_word32 zoom) ;

T_word32 OverheadGetZoomFactor(void) ;

T_void OverheadSetNumPages(void) ;

T_void OverheadToggle(T_void) ;

T_void OverheadOn(T_void) ;

T_void OverheadOff(T_void) ;

E_Boolean OverheadIsOn(T_void) ;

T_void OverheadSetCenterPoint(T_sword32 x, T_sword32 y) ;

/****************************************************************************/
/*    END OF FILE:  OVERHEAD.H                                              */
/****************************************************************************/

#endif // _OVERHEAD_H_
