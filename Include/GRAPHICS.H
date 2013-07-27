/****************************************************************************/
/*    FILE:  GRAPHICS.H                                                     */
/****************************************************************************/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "general.h"
#include "COLORIZE.H"

typedef T_byte8 *T_screen ;

typedef T_byte8 T_color ;

/* Standard colors: */
#define COLOR_BLACK       0
#define COLOR_BLUE        1
#define COLOR_GREEN       2
#define COLOR_LIGHT_GRAY  7
#define COLOR_DARK_GRAY   8
#define COLOR_LIGHT_BLUE  48
#define COLOR_LIGHT_GREEN 10
#define COLOR_YELLOW      32
#define COLOR_WHITE       31
#define COLOR_CLEAR       0

typedef struct {
    T_word16 sizex ;
    T_word16 sizey ;

/** Due to the parser of GCC, we must gracefully make this kludge. **/
#ifdef __GNUC__
    T_byte8 data[1] ;
#else
    T_byte8 data[];
#endif

} T_bitmap ;

typedef T_byte8 T_palette[256][3] ;

typedef struct {
   T_byte8 fontID[4] ;
   T_byte8 height ;
   T_byte8 widths[256] ;

/** Again, GCC will not take subscriptless values. **/
#ifdef __GNUC__
   T_byte8 p_data[1] ;
#else
   T_byte8 p_data[];
#endif
} T_bitfont ;

#ifndef SERVER_ONLY
//#define GRAPHICS_ACTUAL_SCREEN ((T_screen) 0xA0000)
extern T_screen GRAPHICS_ACTUAL_SCREEN ;
#define SCREEN_BAD NULL

#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 200

T_screen GrScreenAlloc(T_void) ;

T_screen GrScreenAllocPartial(T_word16 ySize) ;

T_void GrScreenFree(T_screen screen) ;

T_void GrScreenSet(T_screen screen) ;

T_screen GrScreenGet(T_void) ;

T_void GrDrawPixel(T_word16 x, T_word16 y, T_color color) ;

T_void GrDrawTranslucentPixel(T_word16 x, T_word16 y, T_color color);

T_void GrDrawVerticalLine(
           T_word16 x,
           T_word16 y_top,
           T_word16 y_bottom,
           T_color color) ;

T_void GrDrawHorizontalLine(
           T_word16 x_left,
           T_word16 y,
           T_word16 x_right,
           T_color color) ;

T_void GrDrawBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top) ;

T_void GrDrawCompressedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top) ;

T_void GrDrawCompressedBitmapAndClip(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top) ;

T_void GrDrawBitmapMasked(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top) ;

T_void GrDrawShadedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           T_byte8 shade) ;

T_void GrDrawShadedAndMaskedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           T_byte8 shade) ;


T_void GrTransferRectangle(
           T_screen destination,
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_word16 dest_x,
           T_word16 dest_y) ;

T_void GrGraphicsOn(T_void) ;

T_void GrGraphicsOff(T_void) ;

T_void GrDisplayScreen(T_void) ;

T_void GrDrawRectangle(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color) ;

T_void GrDrawFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color) ;

T_void GrDrawShadedFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color1,
           T_color color2) ;

T_void GrGetPalette(
           T_color start_color,
           T_word16 number_colors,
           T_palette p_palette) ;

T_void GrSetPalette(
           T_color start_color,
           T_word16 number_colors,
           T_palette *p_palette) ;

T_void GrSetBitFont(T_bitfont *p_bitfont) ;

T_void GrSetCursorPosition(T_word16 x_position, T_word16 y_position) ;

T_void GrDrawCharacter(T_byte8 character, T_color color) ;

T_void GrDrawShadowedText(T_byte8 *text, T_color color, T_color shadow) ;

T_void GrDrawText(T_byte8 *text, T_color color) ;

T_word16 GrGetCharacterWidth(T_byte8 character) ;

T_void GrTransferRasterTo(
           T_byte8 *whereTo,
           T_word16 x,
           T_word16 y,
           T_word16 length) ;

T_void GrTransferRasterFrom(
           T_byte8 *whereFrom,
           T_word16 x,
           T_word16 y,
           T_word16 length) ;

T_void GrInvertVerticalLine(
           T_word16 x,
           T_word16 y_top,
           T_word16 y_bottom) ;

T_void GrInvertHorizontalLine(
           T_word16 x_left,
           T_word16 y,
           T_word16 x_right) ;

T_void GrInvertFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom) ;

T_void GrDrawLine(
           T_sword16 x1,
           T_sword16 y1,
           T_sword16 x2,
           T_sword16 y2,
           T_color color) ;

T_void GrDrawCompressedBitmapAndClipAndColor(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top,
           E_colorizeTable colorTable) ;

T_void GrDrawCompressedBitmapAndColor(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           E_colorizeTable colorTable) ;

T_void GrDrawCompressedBitmapAndClipAndColorAndCenterAndResize(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top,
           E_colorizeTable colorTable,
           T_word16 roomWidth,
           T_word16 roomHeight) ;

#if (!defined(TARGET_UNIX) && !defined(WIN32))
T_void DrawTranslucentAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;
#pragma aux DrawTranslucentAsm parm [ESI] [EDI] [ECX]

T_void DrawTranslucentSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;
#pragma aux DrawTranslucentSeeThroughAsm parm [ESI] [EDI] [ECX]

T_void DrawSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;
#pragma aux DrawSeeThroughAsm parm [ESI] [EDI] [ECX]
#else
T_void DrawTranslucentAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;

T_void DrawTranslucentSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;

T_void DrawSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count) ;
#endif

T_void DrawAndShadeRaster(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count,
           T_byte8 shade) ;

T_void GrShadeRectangle(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_byte8 shade) ;

T_void GrActualScreenPush(T_void) ;

T_void GrActualScreenPop(T_void) ;

T_void GrInvalidateRect(
                  T_sword16 x_left,
                  T_sword16 y_top,
                  T_sword16 x_right,
                  T_sword16 y_bottom) ;

T_void GrInvalidateRectClipped(
                  T_sword16 x_left,
                  T_sword16 y_top,
                  T_sword16 x_right,
                  T_sword16 y_bottom) ;

T_void GrActivateColumn(T_word16 x);

#endif
#endif

/****************************************************************************/
/*    END OF FILE:  GRAPHICS.H                                              */
/****************************************************************************/
