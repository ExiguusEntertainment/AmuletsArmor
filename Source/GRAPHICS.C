/*-------------------------------------------------------------------------*
 * File:  GRAPHICS.C
 *-------------------------------------------------------------------------*/
/**
 * Routines for drawing 2D graphics.
 *
 * @addtogroup GRAPHICS
 * @brief Graphics Drawing System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include <stdio.h>
#include "3D_TRIG.H"
#include "DEBUG.H"
#include "DBLLINK.H"
#include "GRAPHICS.H"
#include "MEMORY.H"
#include "MOUSEMOD.H"
#include "TICKER.H"

#define MAX_PAGES 4

#if defined(WATCOM)
#pragma aux  ShadeMemAsm parm	[ESI] [EDI] [ECX] [EBX]
#endif
T_void ShadeMemAsm(
          T_byte8 *p_source,
          T_byte8 *p_destination,
          T_word32 count,
          T_byte8 *p_transTable) ;

/* f(a, b) = (a * b) >> 4 */
T_sword32 ColorChangeFastAsm(
            T_word16 port,
            T_byte8 *p_source,
            T_word32 number) ;
#if defined(WATCOM)
#pragma aux ColorChangeFastAsm = \
                "rep outsb" \
                parm [dx] [esi] [ecx] \
                modify [dx esi ecx] ;
#endif

/* Keep track of the state of the graphics system. */
static E_Boolean G_GraphicsIsOn = FALSE ;

/* Keep track of the active screen. */
static T_screen G_ActiveScreen = NULL ;
/*GRAPHICS_ACTUAL_SCREEN ; */

/* This tells what the current bit font is.  The default is none. */
static T_bitfont *G_CurrentBitFont = NULL ;

/* Position of the upper left hand corner of the font/cursor: */
static T_word16 G_cursorXPosition = 0 ;
static T_word16 G_cursorYPosition = 0 ;

T_word16 G_workingPage = 0 ;
T_byte8 *G_workingStart = ((T_byte8 *)(0xA0000)) ;

T_void GraphicsMode13X(T_void) ;


/* Vertical retrace interrupt information: */
#if defined(WATCOM)
static T_void (__interrupt __far *IOldRetraceInterrupt)();
#endif
static T_word32 G_verticalCount = 0 ;
static E_Boolean G_paletteChanged = FALSE ;
//static T_palette G_palette ;
static T_byte8 G_palette[768] ;

static T_void IInstallVerticalInterrupt(T_void) ;
static T_void IUninstallVerticalInterrupt(T_void) ;
#if defined(WATCOM)
static T_void __interrupt __far IVerticalRetrace(T_void) ;
#endif

static T_void ICheckPaletteChange(T_void) ;
static T_void ITransferPalette(T_void) ;
static T_void IConfirmPaletteChange(T_void) ;

static T_doubleLinkList G_screenStack = DOUBLE_LINK_LIST_BAD ;

T_screen GRAPHICS_ACTUAL_SCREEN ;

static T_sword16 G_lefts[SCREEN_SIZE_Y] ;
static T_sword16 G_rights[SCREEN_SIZE_Y] ;
static T_void IResetLeftsAndRights(T_void) ;

static T_byte8 G_lastPalette[256][3] ;

/*-------------------------------------------------------------------------*
 * Routine:  GrScreenAlloc
 *-------------------------------------------------------------------------*/
/**
 *  This routine is needed to allocate enough space for a screen.
 *  Although this current implementation only allocates a 320x200 region,
 *  future versions may wish to provide different size screens.
 *  To actually use this screen, you need to use GrScreenSet().  It
 *  will then make that screen the active screen.
 *
 *  @return Handle to a screen.
 *
 *<!-----------------------------------------------------------------------*/
T_screen GrScreenAlloc(T_void)
{
    T_screen screen ;

    DebugRoutine("GrScreenAlloc") ;

    /* Allocate enough memory for a 320x200 area. */
    screen = MemAlloc((T_word16)(320*200)) ;
    memset(screen, 0, 320*200) ;
    DebugCheck(screen != NULL) ;

    DebugEnd() ;

    return screen ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrScreenFree
 *-------------------------------------------------------------------------*/
/**
 *  When you are done with a screen, you can free it from memory.
 *  This routine removes the screen from memory.
 *
 *  NOTE: 
 *  A screen cannot be freed if it is the active screen.  You must either
 *  declare another screen as the active (or GRAPHICS_ACTUAL_SCREEN).
 *
 *  @param screen -- Screen to free.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrScreenFree(T_screen screen)
{
    DebugRoutine("GrScreenFree") ;
    DebugCheck(screen != G_ActiveScreen) ;
    MemFree(screen) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrScreenSet
 *-------------------------------------------------------------------------*/
/**
 *  Once you have a screen in allocated, in order to use it, you must
 *  declare it as the active screen.  To do so, use GrScreenSet and pass
 *  the screen handle.
 *
 *  NOTE: 
 *  None really.  Just need to make sure the screen is legal.  Of course,
 *  I can't tell if what is passed is actually a screen or just junk.
 *
 *  @param screen -- Screen to make the active screen
 *
 *<!-----------------------------------------------------------------------*/
T_void GrScreenSet(T_screen screen)
{
    DebugRoutine("GrScreenSet") ;
    DebugCheck(screen != NULL) ;

    G_ActiveScreen = screen ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrScreenGet
 *-------------------------------------------------------------------------*/
/**
 *  Use GrScreenGet to get the handle to the active screen.
 *
 *  @return Active screen
 *
 *<!-----------------------------------------------------------------------*/
T_screen GrScreenGet(T_void)
{
    T_screen screen ;

    DebugRoutine("GrScreenGet") ;

    screen = G_ActiveScreen ;

    DebugCheck(screen != NULL) ;
    DebugEnd() ;

    return screen ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawPixel
 *-------------------------------------------------------------------------*/
/**
 *  Perhaps the lowest level graphics command, this draws one dot on
 *  the screen of a specified color.
 *
 *  @param x -- position from left to right
 *  @param y -- position from top to bottom (top = 0)
 *  @param color -- color index into palette
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawPixel(T_word16 x, T_word16 y, T_color color)
{
    DebugRoutine("GrDrawPixel") ;
    DebugCheck(x < 320) ;
    DebugCheck(y < 200) ;

    GrInvalidateRect(
        x,
        y,
        x,
        y) ;

    /* Draw a dot on the active screen at the (x,y) location. */
    G_ActiveScreen[(y<<8) + (y<<6) + x] = color ;

    DebugEnd() ;
}

T_void GrDrawTranslucentPixel(T_word16 x, T_word16 y, T_color color)
{
    T_byte8 *p_destination;

    DebugRoutine("GrDrawTranslucentPixel") ;
    DebugCheck(x < 320) ;
    DebugCheck(y < 200) ;

    GrInvalidateRect(
        x,
        y,
        x,
        y) ;

    p_destination = G_ActiveScreen + (y<<8) + (y<<6) + x;

    /* Draw a dot on the active screen at the (x,y) location. */
    *p_destination = G_translucentTable[color][*p_destination];

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawVerticalLine
 *-------------------------------------------------------------------------*/
/**
 *  Draw a vertical line from a given (x, y) to a second y in, of
 *  course, your choice color.
 *
 *  NOTE: 
 *  The value of y_top MUST be a smaller value than y_bottom.
 *
 *  @param x -- column to draw line down
 *  @param y_top -- Top line to start drawing (inclusive)
 *  @param y_bottom -- Bottom line to end drawing (inclusive)
 *  @param color -- Color index to draw with
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawVerticalLine(
           T_word16 x,
           T_word16 y_top,
           T_word16 y_bottom,
           T_color color)
{
    T_word16 y_pos ;
    T_byte8 *p_dot ;

    DebugRoutine("GrDrawVerticalLine") ;
    DebugCheck(x < 320) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x,
        y_top,
        x,
        y_bottom) ;

    /* Find a pointer to the start point on the active screen. */
    p_dot = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x ;

    /* Loop through each pixel and draw a dot for each line. */
    for (y_pos = y_top; y_pos <= y_bottom; y_pos++, p_dot += 320)
        *p_dot = color ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawHorizontalLine
 *-------------------------------------------------------------------------*/
/**
 *  Drawing a horizontal line is useful in making boxes or underlines.
 *  In fact, some graphics are all done just by using this command.  But
 *  anyways, just pass the left and right position along with the line
 *  to which to draw it on.  Oh yeah, tell what color too.
 *
 *  NOTE: 
 *  None really.  Just keep the left on the left and the right on the
 *  right.
 *
 *  @param x_left -- Left position of line
 *  @param y -- Line position from the top
 *  @param x_right -- Right position of line (can equal
 *      the left position)
 *  @param color -- Index of color to use
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawHorizontalLine(
           T_word16 x_left,
           T_word16 y,
           T_word16 x_right,
           T_color color)
{
    DebugRoutine("GrDrawHorizontalLine") ;
    DebugCheck(y < 200) ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;

    GrInvalidateRect(
        x_left,
        y,
        x_right,
        y) ;
    /* Find a pointer to the start point on the active screen and set */
    /* a group of sequence bytes to the color. */
    memset(G_ActiveScreen+(y<<8)+(y<<6)+x_left,
           color,
           1+x_right-x_left) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawBitmap
 *-------------------------------------------------------------------------*/
/**
 *  When you need to draw a bitmap, call this routine.  It uses the
 *  standard (raw) format of bitmaps.  It will blip it on the active
 *  screen with NO masking.  (Sorry, black is black)
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_word16 y_count ;
    T_word16 x_size ;

    DebugRoutine("GrDrawBitmap") ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    DebugCheck(x_left+p_bitmap->sizex <= 320) ;
    DebugCheck(y_top+p_bitmap->sizey <= 200) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_left+p_bitmap->sizex,
        y_top+p_bitmap->sizey) ;

    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;
    p_bitmapData = p_bitmap->data ;
    x_size = p_bitmap->sizex ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (y_count=p_bitmap->sizey; y_count>0; y_count--)  {
        /* Copy the bitmap line from the picture to the screen. */
        memcpy(p_screen, p_bitmapData, x_size) ;

        /* Skip down to the next screen line. */
        p_screen += 320 ;

        /* Skip down to the next bitmap line. */
        p_bitmapData += x_size ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawShadedBitmap
 *-------------------------------------------------------------------------*/
/**
 *  When you need to draw a shaded bitmap, call this routine.  It uses
 *  the standard (raw) format of bitmaps.  It will blip it on the active
 *  screen with NO masking.  (Sorry, black is black)
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *  @param shade -- Shading value
 *      (0-255, 0=black, 255=normal)
 *
 *<!-----------------------------------------------------------------------*/
extern T_byte8 P_shadeIndex[16384] ;

T_void GrDrawShadedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           T_byte8 shade)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_word16 y_count ;
    T_word16 x_size ;
    T_word16 x ;
    T_byte8 *p_shadeLookup ;

    DebugRoutine("GrDrawShadedBitmap") ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    DebugCheck(x_left+p_bitmap->sizex <= 320) ;
    DebugCheck(y_top+p_bitmap->sizey <= 200) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_left+p_bitmap->sizex,
        y_top+p_bitmap->sizey) ;

    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;
    p_bitmapData = p_bitmap->data ;
    x_size = p_bitmap->sizex ;

    p_shadeLookup = &P_shadeIndex[((((T_word16)shade)>>2)&0x3F)<<8] ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (y_count=p_bitmap->sizey; y_count>0; y_count--)  {
        /* Draw the shaded version of each pixel on the screen. */
        for (x=0; x<x_size; x++)
            p_screen[x] = p_shadeLookup[p_bitmapData[x]] ;

        /* Skip down to the next screen line. */
        p_screen += 320 ;

        /* Skip down to the next bitmap line. */
        p_bitmapData += x_size ;
    }

    DebugEnd() ;
}


T_void GrDrawShadedAndMaskedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           T_byte8 shade)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_word16 y_count ;
    T_word16 x_size ;
    T_word16 x ;
    T_byte8 *p_shadeLookup ;

    DebugRoutine("GrDrawShadedBitmap") ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    DebugCheck(x_left+p_bitmap->sizex <= 320) ;
    DebugCheck(y_top+p_bitmap->sizey <= 200) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_left+p_bitmap->sizex,
        y_top+p_bitmap->sizey) ;

    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;
    p_bitmapData = p_bitmap->data ;
    x_size = p_bitmap->sizex ;

    p_shadeLookup = &P_shadeIndex[((((T_word16)shade)>>2)&0x3F)<<8] ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (y_count=p_bitmap->sizey; y_count>0; y_count--)  {
        /* Draw the shaded version of each pixel on the screen. */
        for (x=0; x<x_size; x++)
        {
            if (p_bitmapData[x]!=0x00)
              p_screen[x] = p_shadeLookup[p_bitmapData[x]] ;
        }

        /* Skip down to the next screen line. */
        p_screen += 320 ;

        /* Skip down to the next bitmap line. */
        p_bitmapData += x_size ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawBitmapMasked
 *-------------------------------------------------------------------------*/
/**
 *  Other bitmaps may wish the background to show through.  To accompany
 *  this, pictures should use the color 0   (0x00) to denote a clear
 *  pixel color (usually black).  All other pixels are drawn.
 *
 *  NOTE: 
 *  None, other than no clipping like GrDrawBitmap
 *
 *  @param p_bitmap -- Pointer to a bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawBitmapMasked(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_byte8 *p_bitmapPoint ;
    T_byte8 *p_screenPoint ;
    T_word16 y_count ;
    T_word16 x_count ;
    T_word16 x_size ;

    DebugRoutine("GrDrawBitmapMasked") ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    DebugCheck(x_left+p_bitmap->sizex <= 320) ;
    DebugCheck(y_top+p_bitmap->sizey <= 200) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_left+p_bitmap->sizex,
        y_top+p_bitmap->sizey) ;

    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;
    p_bitmapData = p_bitmap->data ;
    x_size = p_bitmap->sizex ;

    /* This is the fastest way to blip a masked bitmap on the screen */
    /* Without going into assembly language. */
    for (y_count=p_bitmap->sizey; y_count>0; y_count--)  {
        p_bitmapPoint = p_bitmapData ;
        p_screenPoint = p_screen ;

        /* Loop for each x pixel. */
        for (x_count=x_size; x_count>0; x_count--)  {
            /* Check if the bitmap pixel is a 0x00.  If it is not, */
            /* copy the pixel.  Otherwise, skip it. */
            if (*p_bitmapPoint != 0x00)
                *p_screenPoint = *p_bitmapPoint ;

            /* Go to the next pixel position. */
            p_screenPoint++ ;
            p_bitmapPoint++ ;
        }

        /* Skip down to the next screen line. */
        p_screen += 320 ;

        /* Skip down to the next bitmap line. */
        p_bitmapData += x_size ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrTransferRectangle
 *-------------------------------------------------------------------------*/
/**
 *  GrTransferRectangle is used to transfer only a section of the active
 *  screen to some other destination screen.  This routine requires the
 *  upper-left and lower-right corners of the region you wish copied and
 *  what coordinates you wish to copy too.
 *  Note that no masking is performed in this transfer.
 *
 *  NOTE: 
 *  No clipping is done.  Therefore, the region being copied must fit
 *  entirely on the destination screen.  If it doesn't you get a debug
 *  error.
 *  Also, the destination cannot equal the active source.
 *
 *  @param destination -- Screen to transfer to
 *  @param x_left -- Left of transfer rectangle
 *  @param y_top -- Top of transfer rectangle
 *  @param x_right -- Right of transfer rectangle
 *  @param y_bottom -- Bottom of tranfer rectangle
 *  @param dest_x -- Left of destination rectangle
 *  @param dest_y -- Top of destination rectangle
 *
 *<!-----------------------------------------------------------------------*/
T_void GrTransferRectangle(
           T_screen destination,
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_word16 dest_x,
           T_word16 dest_y)
{
    T_sword16 width ;
    T_sword16 height ;
    T_byte8 *p_screenFrom ;
    T_byte8 *p_screenTo ;

    DebugRoutine("GrTransferRectangle") ;
    DebugCheck(destination != G_ActiveScreen) ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    if (destination == GRAPHICS_ACTUAL_SCREEN)
        GrInvalidateRect(
            dest_x,
            dest_y,
            1+dest_x+x_right-x_left,
            1+dest_y+y_bottom-y_top) ;

//printf("%d\n", G_verticalCount) ;
    /* Calculate the width and height of this rectangle. */
    width = x_right - x_left + 1 ;
    height = y_bottom - y_top + 1 ;

    DebugCheck(dest_x+width <= 320) ;
    DebugCheck(dest_y+height <= 200) ;

    /* Find the starting point of where we are going to copy from. */
    p_screenFrom = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;

    /* Find the starting point of where we are going to transfer to. */
    p_screenTo = destination+(dest_y<<8)+(dest_y<<6)+dest_x;

    /* Loop through each line in the rectangle. */
    for (; height>0; height--)  {
//        ICheckPaletteChange() ;
        /* Copy each line's width that we have. */
        memcpy(p_screenTo, p_screenFrom, width) ;

        /* Update our line positions. */
        p_screenFrom += 320 ;
        p_screenTo += 320 ;
    }

//    IConfirmPaletteChange() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrGraphicsOn
 *-------------------------------------------------------------------------*/
/**
 *  Turn on the graphics screen and turn off the text screen.
 *
 *  NOTE: 
 *  None really.  Just the usually -- hey, no more text!
 *  Actually I just thought of one, this routine doesn't check to see
 *  if the computer can handle mode 0x13 graphics.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrGraphicsOn(T_void)
{
#if defined(DOS32)
	union REGS regs ;

    DebugRoutine("GrGraphicsOn") ;
    DebugCheck(G_GraphicsIsOn == FALSE) ;

    /* Turn on mode 13 graphics using the BIOS. */
#ifdef __386__
    regs.x.eax = 0x0013 ;
    int386(0x10, &regs, &regs) ;
#else
    regs.x.ax = 0x13 ;
    int86(0x10, &regs, &regs) ;
#endif

///    GraphicsMode13X() ;
    /* Note that graphics is now on. */
    G_GraphicsIsOn = TRUE ;

    G_ActiveScreen = GRAPHICS_ACTUAL_SCREEN = GrScreenAlloc() ;
    G_verticalCount = 0 ;
//IInstallVerticalInterrupt() ;

    G_screenStack = DoubleLinkListCreate() ;

    IResetLeftsAndRights() ;

    DebugEnd() ;
#elif defined(WIN32)
    DebugRoutine("GrGraphicsOn") ;
    DebugCheck(G_GraphicsIsOn == FALSE) ;
    G_GraphicsIsOn = TRUE ;
    //G_ActiveScreen = GRAPHICS_ACTUAL_SCREEN = GrScreenAlloc() ;
    G_verticalCount = 0 ;
//IInstallVerticalInterrupt() ;

    G_screenStack = DoubleLinkListCreate() ;

    IResetLeftsAndRights() ;
    DebugEnd() ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  GrGraphicsOff
 *-------------------------------------------------------------------------*/
/**
 *  Turn off graphics mode, and turn on text mode.  We will drop back
 *  to standard 80x25 color text.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrGraphicsOff(T_void)
{
#if defined(DOS32)
	union REGS regs ;

    DebugRoutine("GrGraphicsOff") ;
    DebugCheck(G_GraphicsIsOn == TRUE) ;

//IUninstallVerticalInterrupt() ;
    /* Turn off mode 13 graphics and turn on mode 3 using the BIOS. */
#ifdef __386__
    regs.x.eax = 0x0003 ;
    int386(0x10, &regs, &regs) ;
#else
    regs.x.ax = 0x03 ;
    int86(0x10, &regs, &regs) ;
#endif

    /* Note that graphics is now off. */
    G_GraphicsIsOn = FALSE ;
GrScreenSet((T_screen)((char *)0xA0000)) ;
GrScreenFree(GRAPHICS_ACTUAL_SCREEN) ;

#ifndef NDEBUG
    if (DoubleLinkListGetNumberElements(G_screenStack) != 0)
        puts("Warning!  GraphicsOff called before empty screen stack.") ;
#endif
    DoubleLinkListDestroy(G_screenStack) ;
    G_screenStack = DOUBLE_LINK_LIST_BAD ;

    DebugEnd() ;
#else
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDisplayScreen
 *-------------------------------------------------------------------------*/
/**
 *  None of the graphics commands take effect until this command is
 *  called.  When that happens, the active screen is copied over the
 *  actual screen.
 *
 *  NOTE: 
 *  Make sure the active screen is not the actual screen, or else
 *  this routine is made to bomb.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDisplayScreen(T_void)
{
    DebugRoutine("GrDisplayScreen") ;
    DebugCheck(G_ActiveScreen != GRAPHICS_ACTUAL_SCREEN) ;

    /* Memcpy is usually written to be VERY fast by the C compiler/library. */
    memcpy(GRAPHICS_ACTUAL_SCREEN, G_ActiveScreen, (T_word16)(320*200)) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawRectangle
 *-------------------------------------------------------------------------*/
/**
 *  To draw a solid rectangle of one color, use the GrDrawRectangle
 *  routine.  Pass to it the upper-left coordinate and the lower-right
 *  coordinate and the color.
 *
 *  NOTE: 
 *  All coordinates must be on the screen and the upper-left coordinate
 *  must be located to the left and above the lower-right corner.
 *
 *  @param x_left -- Left of rectangle
 *  @param y_top -- Top of rectangle
 *  @param x_right -- Right of rectangle
 *  @param y_bottom -- Bottom of rectangle
 *  @param color -- Color of the rectangle
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawRectangle(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color)
{
    T_word16 width ;
    T_word16 height ;
    T_byte8 *p_screen ;

    DebugRoutine("GrDrawRectangle") ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_right,
        y_bottom) ;
    width = x_right-x_left+1 ;
    height = y_bottom-y_top+1 ;

    /* Find the starting point of where we are going to fill. */
    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;

    /* Loop through each line and fill those lines. */
    for (; height > 0; height--, p_screen+=320)
        memset(p_screen, color, width) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawFrame
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawFrame will draw an empty frame in a choice color.  Just give
 *  the upper-left and lower-right corner of the frame.
 *
 *  NOTE: 
 *  All coordinates must be on the screen and the upper-left coordinate
 *  must be located to the left and above the lower-right corner.
 *
 *  @param x_left -- Left of frame
 *  @param y_top -- Top of frame
 *  @param x_right -- Right of frame
 *  @param y_bottom -- Bottom of frame
 *  @param color -- Color of the frame
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color)
{
    DebugRoutine("GrDrawFrame") ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_right,
        y_bottom) ;
    /* Draw the top and bottom */
    GrDrawHorizontalLine(x_left, y_top, x_right, color) ;
    GrDrawHorizontalLine(x_left, y_bottom, x_right, color) ;

    /* Draw left and right edges. */
    GrDrawVerticalLine(x_left, y_top, y_bottom, color) ;
    GrDrawVerticalLine(x_right, y_top, y_bottom, color) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawShadedFrame
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawShadedFrame will draw a frame that looks shaded.  The upper
 *  left corner is one color and the lower right corner is a different
 *  color.
 *
 *  NOTE: 
 *  All coordinates must be on the screen and the upper-left coordinate
 *  must be located to the left and above the lower-right corner.
 *
 *  @param x_left -- Left of frame
 *  @param y_top -- Top of frame
 *  @param x_right -- Right of frame
 *  @param y_bottom -- Bottom of frame
 *  @param color1 -- Color of the upper left corner
 *  @param color2 -- Color of the lower right corner
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawShadedFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_color color1,
           T_color color2)
{
    DebugRoutine("GrDrawShadedFrame") ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_right,
        y_bottom) ;
    /* Draw left and right edges. */
    GrDrawVerticalLine(x_left, y_top, y_bottom, color1) ;
    GrDrawVerticalLine(x_right, y_top, y_bottom, color2) ;

    /* Draw the top and bottom */
    GrDrawHorizontalLine(x_left, y_top, x_right, color1) ;
    GrDrawHorizontalLine(x_left, y_bottom, x_right, color2) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrSetPalette
 *-------------------------------------------------------------------------*/
/**
 *  GrSetPalette is used to change colors in the palette list.  Pass to
 *  this routine the starting color index and then the number of colors
 *  you wish changed.  Also pass a pointer to the new palette you want
 *  (where the 0'd item is the start color).
 *
 *  NOTE: 
 *  If the index of the start color plus the number of colors is >255
 *  The program will bomb.
 *
 *  @param start_color -- Index to start of colors to change
 *  @param number_colors -- Number of colors to change
 *  @param p_palette -- Pointer to palette list
 *
 *<!-----------------------------------------------------------------------*/
static T_void ITransferScreen(T_void)
{
    T_word16 i ;
    T_sword16 *p_left, *p_right ;
    T_byte8 *p_to ;
    T_byte8 *p_from ;

    TICKER_TIME_ROUTINE_PREPARE() ;
    TICKER_TIME_ROUTINE_START() ;
//    memcpy((char *)0xA0000, GRAPHICS_ACTUAL_SCREEN, 64000) ;
    p_left = G_lefts ;
    p_right = G_rights ;
    p_from = GRAPHICS_ACTUAL_SCREEN ;
    p_to = (char *)0xA0000 ;
    for (i=0; i<SCREEN_SIZE_Y; i++)  {
        if (*p_left != 0x7F7F)
            memcpy(p_to+*p_left, p_from+*p_left, 1+*p_right-*p_left) ;

        p_left++ ;
        p_right++ ;
        p_from += SCREEN_SIZE_X ;
        p_to += SCREEN_SIZE_X ;
    }

    TICKER_TIME_ROUTINE_ENDM("ITranserScreen", 1000);
    IResetLeftsAndRights() ;
}

T_void GrSetPalette(
           T_color start_color,
           T_word16 number_colors,
           T_palette *p_palette)
{
#ifdef WATCOM
    T_byte8 *p_color ;
    T_word16 i ;
    E_Boolean changed = FALSE ;
    static T_byte8 testvalue = 0 ;
    static T_word32 lastTime = 0 ;
    static T_byte8 countTime = 0 ;
    E_Boolean needSync = FALSE ;
    TICKER_TIME_ROUTINE_PREPARE() ;
    TICKER_TIME_ROUTINE_START() ;

    DebugRoutine("GrSetPalette") ;
    INDICATOR_LIGHT(943, INDICATOR_GREEN) ;
    DebugCheck(number_colors <= 256) ;
    DebugCheck(start_color + number_colors <= 256) ;
    /* Set the index color to start chaning. */
    outp(0x3C8, start_color) ;

    /* For each color that was passed to us, stored the RGB value */
    /* in the real palette. */
    /* Find where the first color is. */
    p_color = (T_byte8 *)p_palette ;

    /* See if the palette is changing. */
    i = start_color*3 ;
    if (memcmp(p_palette+i, G_lastPalette+i, number_colors*3))
        needSync = TRUE ;

    /* Record the last palette */
    memcpy(G_lastPalette+i, p_palette+i, number_colors*3) ;

    /* Wait for vertical retrace. */
#ifndef ALLOW_SNOWY_GRAPHICS
    if (needSync)
       while (!(inp(0x03DA) & 8))
          {}
#endif

    /* Multiply number of colors by 3 to get red, green, and blue */
    number_colors += (number_colors<<1) ;

    INDICATOR_LIGHT(946, INDICATOR_GREEN) ;
    if (needSync)
        ColorChangeFastAsm(0x03C9, p_color, number_colors) ;
    INDICATOR_LIGHT(946, INDICATOR_RED) ;
    INDICATOR_LIGHT(949, INDICATOR_GREEN) ;

    MouseDisallowUpdate() ;
    MouseDraw() ;
    INDICATOR_LIGHT(949, INDICATOR_RED) ;
    INDICATOR_LIGHT(952, INDICATOR_GREEN) ;
#ifndef INDICATOR_LIGHTS
//    memcpy((char *)0xA0000, GRAPHICS_ACTUAL_SCREEN, 64000) ;
    ITransferScreen() ;
#else
    if (TickerGet() & 64)  {
        GRAPHICS_ACTUAL_SCREEN[1280] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1281] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1282] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1283] = 25 ;
    } else {
        GRAPHICS_ACTUAL_SCREEN[1280] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1281] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1282] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1283] = 0 ;
    }
    testvalue++ ;
    if (testvalue & 1)  {
        GRAPHICS_ACTUAL_SCREEN[1284] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1285] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1286] = 25 ;
        GRAPHICS_ACTUAL_SCREEN[1287] = 25 ;
    } else {
        GRAPHICS_ACTUAL_SCREEN[1284] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1285] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1286] = 0 ;
        GRAPHICS_ACTUAL_SCREEN[1287] = 0 ;
    }
    memcpy((char *)0xA0000+1280, GRAPHICS_ACTUAL_SCREEN+1280, 62720) ;

    /* Check if we have been stuck in time */
    if (TickerGet() == lastTime)  {
        countTime++ ;
        if (countTime == 20)
            DebugCheck(FALSE) ;
    } else {
        lastTime = TickerGet() ;
        countTime = 0 ;
    }
#endif
    INDICATOR_LIGHT(952, INDICATOR_RED) ;
    INDICATOR_LIGHT(955, INDICATOR_GREEN) ;

    MouseErase() ;
    MouseAllowUpdate() ;
    DebugEnd() ;
    INDICATOR_LIGHT(955, INDICATOR_RED) ;
    INDICATOR_LIGHT(943, INDICATOR_RED) ;
    TICKER_TIME_ROUTINE_ENDM("GrSetPalette", 1000)
#endif

#ifdef WIN32
        extern void _cdecl WindowsUpdate(char *p_screen, unsigned char *palette);
    T_word16 i ;
    /* See if the palette is changing. */
    i = start_color*3 ;

    /* Record the last palette */
    memcpy(G_lastPalette+i, p_palette+i, number_colors*3) ;
    MouseDraw() ;
    WindowsUpdate(GRAPHICS_ACTUAL_SCREEN, &G_lastPalette[0][0]) ;
    MouseErase() ;
    MouseAllowUpdate() ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  GrGetPalette
 *-------------------------------------------------------------------------*/
/**
 *  GrGetPalette is used to retrieve the color pallette list or a sub-
 *  part of it.  Works just like GrSetPalette.
 *
 *  NOTE: 
 *  If the index of the start color plus the number of colors is >255
 *  The program will bomb.
 *
 *  @param start_color -- Index to start of colors to change
 *  @param number_colors -- Number of colors to change
 *  @param p_palette -- Pointer to palette list
 *
 *<!-----------------------------------------------------------------------*/
T_void GrGetPalette(
           T_color start_color,
           T_word16 number_colors,
           T_palette p_palette)
{
#ifdef DOS32
    T_word16 i ;
    T_color color ;

    DebugRoutine("GrGetPalette") ;
    DebugCheck(number_colors <= 256) ;
    DebugCheck(start_color + number_colors <= 256) ;
    DebugCheck(p_palette != NULL) ;

    /* Set the index color to start chaning. */
    outp(0x3C7, start_color) ;

    /* For each color that was passed to us, stored the RGB value */
    /* in the real palette. */
    for (color=0, i=number_colors; i>0; i--, color++)  {
        p_palette[color][0] = inp(0x03C9) ;
        p_palette[color][1] = inp(0x03C9) ;
        p_palette[color][2] = inp(0x03C9) ;
    }
    DebugEnd() ;
#else
    int i;

    for (i=0; i<number_colors; i++) {
        p_palette[start_color+i][0] = G_lastPalette[start_color+i][0] & 63;
        p_palette[start_color+i][1] = G_lastPalette[start_color+i][1] & 63;
        p_palette[start_color+i][2] = G_lastPalette[start_color+i][2] & 63;
    }
    //memcpy(p_palette+(start_color*3), &G_lastPalette[start_color][0], (number_colors*3)) ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  GrSetBitFont
 *-------------------------------------------------------------------------*/
/**
 *  GrSetBitFont declares the bit font that the graphics module will
 *  use for all consecutive drawing of text.  Just pass a pointer to a
 *  bit font that has already been loaded into memory.
 *
 *  NOTE: 
 *  Note that if the font is removed from memory while in use, these
 *  routines have no way of knowing that the font is gone and will output
 *  garbage.  Hopefully this will not cause an error.
 *
 *  @param p_bitfont -- Pointer to loaded bit font
 *
 *<!-----------------------------------------------------------------------*/
T_void GrSetBitFont(T_bitfont *p_bitfont)
{
    DebugRoutine("GrSetBitFont") ;
    DebugCheck(p_bitfont != NULL) ;
#ifndef NDEBUG
    if (strcmp(p_bitfont->fontID, "Fon") != 0)  {
        printf("Bad font: %4.4s [%02X %02X %02X %02X]\n",
            p_bitfont->fontID,
            p_bitfont->fontID[0],
            p_bitfont->fontID[1],
            p_bitfont->fontID[2],
            p_bitfont->fontID[3]) ;
        fflush(stdout) ;
        DebugCheck(FALSE) ;
    }
#endif
    DebugCheck(strcmp(p_bitfont->fontID, "Fon")==0) ;
    DebugCheck(p_bitfont->height < 50) ;

    /* Make the given font the current bit font. */
    G_CurrentBitFont = p_bitfont ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrSetCursorPosition
 *-------------------------------------------------------------------------*/
/**
 *  GrSetCursorPosition moves the cursor to a new location on the screen.
 *  All subsequent character drawing will occur at that new location.
 *
 *  NOTE: 
 *  x_position and y_position must be inside the screen area for this
 *  command to work correctly.
 *
 *  @param x_position -- left side of where character is drawn
 *  @param y_position -- top side of where character is drawn
 *
 *<!-----------------------------------------------------------------------*/
T_void GrSetCursorPosition(T_word16 x_position, T_word16 y_position)
{
    DebugRoutine("GrSetCursorPosition") ;
    DebugCheck(x_position < 320) ;
    DebugCheck(y_position < 200) ;

    G_cursorXPosition = x_position ;
    G_cursorYPosition = y_position ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCharacter
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCharacter is the low level routine to draw a single character
 *  on the screen in the active font.  Just pass to it a character and it
 *  will be drawn at the current cursor position.  The cursor position will
 *  then be updated.
 *
 *  NOTE: 
 *  None.  Just make sure you used GrSetBitFont earlier.
 *
 *  @param character -- character to draw
 *  @param color -- color to draw character in
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawCharacter(T_byte8 character, T_color color)
{
    T_word16 height ;
    T_word16 width ;
    T_byte8 *p_line ;
    T_byte8 *p_dot ;
    T_word16 count ;
    T_byte8 bitmask ;
    T_byte8 *p_font_character ;

    DebugRoutine("GrDrawCharacter") ;
    DebugCheck(G_CurrentBitFont != NULL) ;
#ifndef NDEBUG
    if (strcmp(G_CurrentBitFont->fontID, "Fon") != 0)  {
        printf("Bad font: %4.4s [%02X %02X %02X %02X]\n",
            G_CurrentBitFont->fontID,
            G_CurrentBitFont->fontID[0],
            G_CurrentBitFont->fontID[1],
            G_CurrentBitFont->fontID[2],
            G_CurrentBitFont->fontID[3]) ;
        fflush(stdout) ;
        DebugCheck(FALSE) ;
    }
#endif
    DebugCheck(strcmp(G_CurrentBitFont->fontID, "Fon") == 0) ;

    /* First see if the character will fit on the screen.  If it does */
    /* not, we want to move it down to the next line. */
    width = G_CurrentBitFont->widths[character] ;
    height = G_CurrentBitFont->height ;

    if (G_cursorXPosition+width >= 320)  {
        /* We are too far to the right, let's move down to the next line. */
        G_cursorXPosition = 0 ;
        G_cursorYPosition += height ;
    }

    /* If the current character will go off the bottom, roll */
    /* around to the top. */
    if (G_cursorYPosition+height >= 200)
        G_cursorYPosition = 0;

    /* Ok, now we have room to draw the character. */
    /* Find a pointer to the current position for the character. */
    p_line = G_ActiveScreen+(G_cursorYPosition<<8)+
                            (G_cursorYPosition<<6)+
                            G_cursorXPosition ;

    GrInvalidateRect(
        G_cursorXPosition,
        G_cursorYPosition,
        G_cursorXPosition+8,
        G_cursorYPosition+height-1) ;
    /* Get a pointer to the one character in the font. */
    p_font_character = &G_CurrentBitFont->p_data[height * (T_word16)character] ;

    /* Go through each line in the font character. */
    for (; height>0; height--)  {
        /* Go through each pixel in the font character line and draw. */
        for (p_dot=p_line, bitmask = 0x80, count=width;
             count>0;
             count--, bitmask>>=1, p_dot++)  {
            /* If the bit is set in the font character, draw a pixel */
            if ((*p_font_character & bitmask)!=0)
                *p_dot = color ;
        }
        p_font_character++ ;
        p_line+=320 ;
    }

    /* Now update the x position of the character.  Since we know that */
    /* the character originally fits, we don't have to check to see */
    /* if we are off the screen now. */
    G_cursorXPosition += width ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawText
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawText draws a whole group of characters on the screen at once.
 *  It does NOT do any character filtering (such as newline characters).
 *
 *  @param text -- Pointer to string to draw
 *  @param color -- Color to draw text in
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawText(T_byte8 *text, T_color color)
{
    DebugRoutine("GrDrawText") ;
    DebugCheck(text != NULL) ;

    while (*text != '\0')
        GrDrawCharacter(*(text++), color) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawShadowedText
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawShadowed text is just like GrDrawText except that it draws
 *  a shadowed color of the text first and then places the normal characters
 *  on top.
 *
 *  @param text -- Pointer to string to draw
 *  @param color -- Color to draw text in
 *  @param shadow -- Color to draw shadow in
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawShadowedText(T_byte8 *text, T_color color, T_color shadow)
{
    T_word16 xPos ;
    T_word16 yPos ;
    T_byte8 *textPtr ;

    DebugRoutine("GrDrawText") ;
    DebugCheck(text != NULL) ;

    /* Get the current cursor position and prepare to draw the */
    /* shadow down one and over one. */
    xPos = G_cursorXPosition ;
    yPos = G_cursorYPosition ;
    GrSetCursorPosition(xPos+1, yPos+1) ;

    /* Draw the shadow first. */
    textPtr = text ;
    while (*textPtr != '\0')
        GrDrawCharacter(*(textPtr++), shadow) ;

    /* Draw the text letters on top of the shadow, but at the */
    /* original cursor position. */
    GrSetCursorPosition(xPos, yPos) ;
    textPtr = text ;
    while (*textPtr != '\0')
        GrDrawCharacter(*(textPtr++), color) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrGetCharacterWidth
 *-------------------------------------------------------------------------*/
/**
 *  Sometimes it is useful to know how wide a character is before you
 *  draw it (say for centering text).  To get this information, pass
 *  GrGetCharacterWidth the character you need info on.
 *
 *  NOTE: 
 *  None.  Just make sure a valid font is in memory.
 *
 *  @param character -- Character to get width of
 *
 *  @return Width of character
 *
 *<!-----------------------------------------------------------------------*/
T_word16 GrGetCharacterWidth(T_byte8 character)
{
    T_word16 width ;

    DebugRoutine("GrGetCharacterWidth") ;
    DebugCheck(G_CurrentBitFont != NULL) ;
    DebugCheck(strcmp(G_CurrentBitFont->fontID, "Fon") == 0) ;

    width = G_CurrentBitFont->widths[character] ;

    DebugCheck(width < 50) ;
    DebugEnd() ;

    return width ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrTransferRasterFrom
 *-------------------------------------------------------------------------*/
/**
 *  To help with high speed parts of the game, a transfer from memory
 *  to video routine has been provided.  Technically, this routine is
 *  only for one raster, but one raster can extend onto multiple lines.
 *  Just pass a pointer to where in memory you want to get an array of
 *  bytes, and x & y coordinate of where to put it, and the number of
 *  bytes (length) of the raster.
 *
 *  @param whereFrom -- Pointer to memory to transfer
 *  @param x -- Screen x coordinate
 *  @param y -- Screen y coordinate
 *  @param length -- Number of bytes to transfer
 *
 *<!-----------------------------------------------------------------------*/
T_void GrTransferRasterFrom(
           T_byte8 *whereFrom,
           T_word16 x,
           T_word16 y,
           T_word16 length)
{
    DebugRoutine("GrTransferRasterFrom") ;

    /* Only allow up the right side of the screen. */
    if (length+x >= SCREEN_SIZE_X)
        length = SCREEN_SIZE_X-x ;

    memcpy(G_ActiveScreen+(y<<6)+(y<<8)+x, whereFrom, length) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrTransferRasterTo
 *-------------------------------------------------------------------------*/
/**
 *  To help with high speed parts of the game, a transfer from video to
 *  memory routine has been provided.  Technically, this routine is
 *  only for one raster, but one raster can extend onto multiple lines.
 *  Just pass a pointer to where in memory you want to put an array of
 *  bytes, and x & y coordinate of where to get it, and the number of
 *  bytes (length) of the raster.
 *
 *  @param whereTo -- Pointer to memory
 *  @param x -- Screen x coordinate
 *  @param y -- Screen y coordinate
 *  @param length -- Number of bytes to transfer
 *
 *<!-----------------------------------------------------------------------*/
T_void GrTransferRasterTo(
           T_byte8 *whereTo,
           T_word16 x,
           T_word16 y,
           T_word16 length)
{
    DebugRoutine("GrTransferRasterTo") ;

    GrInvalidateRect(
        x,
        y,
        x+length-1,
        y) ;

    /* Only allow up the right side of the screen. */
    if (length+x >= SCREEN_SIZE_X)
        length = SCREEN_SIZE_X-x ;

    memcpy(whereTo, G_ActiveScreen+(y<<6)+(y<<8)+x, length) ;

    DebugEnd() ;
}

T_void GrInvertHorizontalLine(
           T_word16 x1,
           T_word16 y1,
           T_word16 x2) ;

T_void GrInvertVerticalLine(
           T_word16 x1,
           T_word16 y1,
           T_word16 y2) ;

T_void GrInvertFrame(
           T_word16 x1,
           T_word16 y1,
           T_word16 x2,
           T_word16 y2) ;

/*-------------------------------------------------------------------------*
 * Routine:  GrInvertVerticalLine
 *-------------------------------------------------------------------------*/
/**
 *  Invert a vertical line from a given (x, y) to a second y.
 *
 *  NOTE: 
 *  The value of y_top MUST be a smaller value than y_bottom.
 *
 *  @param x -- column to draw line down
 *  @param y_top -- Top line to start drawing (inclusive)
 *  @param y_bottom -- Bottom line to end drawing (inclusive)
 *
 *<!-----------------------------------------------------------------------*/
T_void GrInvertVerticalLine(
           T_word16 x,
           T_word16 y_top,
           T_word16 y_bottom)
{
    T_word16 y_pos ;
    T_byte8 *p_dot ;

    DebugRoutine("GrInvertVerticalLine") ;
    DebugCheck(x < 320) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x,
        y_top,
        x,
        y_bottom) ;

    /* Find a pointer to the start point on the active screen. */
    p_dot = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x ;

    /* Loop through each pixel and invert a dot for each line. */
    for (y_pos = y_top; y_pos <= y_bottom; y_pos++, p_dot += 320)
        *p_dot = *p_dot ^ 0xFF ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrInvertHorizontalLine
 *-------------------------------------------------------------------------*/
/**
 *  Invert a horizontal line from a given (x, y) to a second x.
 *
 *  NOTE: 
 *  None really.  Just keep the left on the left and the right on the
 *  right.
 *
 *  @param x_left -- Left position of line
 *  @param y -- Line position from the top
 *  @param x_right -- Right position of line (can equal
 *      the left position)
 *
 *<!-----------------------------------------------------------------------*/
T_void GrInvertHorizontalLine(
           T_word16 x_left,
           T_word16 y,
           T_word16 x_right)
{
    T_word16 x_pos ;
    T_byte8 *p_dot ;

    DebugRoutine("GrInvertHorizontalLine") ;
    DebugCheck(y < 200) ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;

    GrInvalidateRect(
        x_left,
        y,
        x_right,
        y) ;
    /* Find a pointer to the start point on the active screen. */
    p_dot = G_ActiveScreen+(y<<8)+(y<<6)+x_left ;

    /* Loop through each pixel and invert a dot for each column. */
    for (x_pos = x_left; x_pos <= x_right; x_pos++, p_dot++)
        *p_dot = *p_dot ^ 0xFF ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrInvertFrame
 *-------------------------------------------------------------------------*/
/**
 *  GrInvertFrame will invert an empty frame on the screen.  Just give
 *  the upper-left and lower-right corner of the frame.
 *
 *  NOTE: 
 *  All coordinates must be on the screen and the upper-left coordinate
 *  must be located to the left and above the lower-right corner.
 *
 *  @param x_left -- Left of frame
 *  @param y_top -- Top of frame
 *  @param x_right -- Right of frame
 *  @param y_bottom -- Bottom of frame
 *
 *<!-----------------------------------------------------------------------*/
T_void GrInvertFrame(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom)
{
    DebugRoutine("GrInvertFrame") ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(
        x_left,
        y_top,
        x_right,
        y_bottom) ;

    /* Invert the top and bottom */
    GrInvertHorizontalLine(x_left+1, y_top, x_right) ;
    GrInvertHorizontalLine(x_left, y_bottom, x_right-1) ;

    /* Invert left and right edges. */
    GrInvertVerticalLine(x_left, y_top, y_bottom-1) ;
    GrInvertVerticalLine(x_right, y_top+1, y_bottom) ;

    DebugEnd() ;
}

T_void GrSetWorkingPage(T_word16 page)
{
    DebugRoutine("GrSetWorkingPage") ;
    DebugCheck(page < MAX_PAGES) ;

    G_workingPage = page ;
    G_workingStart = ((T_byte8 *)0xA0000) + (page<<14) ;

    DebugEnd() ;
}

T_byte8 *GrGetWorkingPage(T_void)
{
    return G_workingStart ;
}

T_void GrSetVisualPage(T_word16 page)
{
    DebugRoutine("GrSetWorkingPage") ;
    DebugCheck(page < MAX_PAGES) ;
#ifdef DOS32
    outp(0x3D4, 0xC) ;
    outp(0x3D5, (page<<14)) ;
#endif
    DebugEnd() ;
}

T_void GrSelectPlanes(T_word16 planes)
{
#ifdef DOS32
    planes <<= 8 ;
    planes |= 2 ;

    outpw(0x3C4, planes) ;
#endif
}

T_void GrActivateColumn(T_word16 x)
{
    GrSelectPlanes(1 << (x & 3)) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawLine
 *-------------------------------------------------------------------------*/
/**
 *  Draw a vertical line from a given (x, y) to a second (x, y) in
 *  your choice color.
 *
 *  @param x1 -- X first point
 *  @param y1 -- Y first point
 *  @param x2 -- X second point.
 *  @param y2 -- Y second point.
 *  @param color -- Color index to draw with
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawLine(
           T_sword16 x1,
           T_sword16 y1,
           T_sword16 x2,
           T_sword16 y2,
           T_color color)
{
    T_sword16 fract_x ;
    T_sword16 fract_y ;
    T_sword16 dirx ;
    T_sword16 diry ;
    T_sword16 step_x ;
    T_sword16 step_y ;
    T_byte8 *p_screen = (T_byte8 *)G_ActiveScreen ;
    T_word16 offset ;
    T_sword16 ystep ;

    DebugRoutine("GrDrawLine") ;
    DebugCheck(x1 < SCREEN_SIZE_X) ;
    DebugCheck(y1 < SCREEN_SIZE_Y) ;
    DebugCheck(x2 < SCREEN_SIZE_X) ;
    DebugCheck(y2 < SCREEN_SIZE_Y) ;
    DebugCheck(x1 >= 0) ;
    DebugCheck(y1 >= 0) ;
    DebugCheck(x2 >= 0) ;
    DebugCheck(y2 >= 0) ;

    if (x1<x2)  {
        dirx = 1 ;
        step_x = x2-x1 ;
    } else {
        dirx = -1 ;
        step_x = x1-x2 ;
    }
    fract_x = 0 ;

    if (y1<y2)  {
        diry = 1 ;
        step_y = y2-y1 ;
    } else {
        diry = -1 ;
        step_y = y1-y2 ;
    }
    fract_y = 0 ;

    offset = y1*320 + x1 ;
    p_screen[offset] = color ;
    ystep = diry * 320 ;

    if (step_x > step_y)  {
        while (x1 != x2)  {
            fract_y += step_y ;
            if (fract_y >= step_x)  {
                fract_y -= step_x ;
                y1 += diry ;
                offset += ystep ;
            }
            x1 += dirx ;
            offset += dirx ;
            p_screen[offset] = color ;
        }
    } else {
        while (y1 != y2)  {
            fract_x += step_x ;
            if (fract_x >= step_y)  {
                fract_x -= step_y ;
                x1 += dirx ;
                offset += dirx ;
            }
            y1 += diry ;
            offset += ystep ;
            p_screen[offset] = color ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDoubleSizeTransfer
 *-------------------------------------------------------------------------*/
/**
 *  GrDoubleSizeTransfer draws a to the screen at double size.
 *
 *  NOTE: 
 *  No clipping is done.  Therefore, the region being copied must fit
 *  entirely on the destination screen.  If it doesn't you get a debug
 *  error.
 *  Also, the destination cannot equal the active source.
 *
 *  @param destination -- Screen to transfer to
 *  @param x_left -- Left of transfer rectangle
 *  @param y_top -- Top of transfer rectangle
 *  @param x_right -- Right of transfer rectangle
 *  @param y_bottom -- Bottom of tranfer rectangle
 *  @param dest_x -- Left of destination rectangle
 *  @param dest_y -- Top of destination rectangle
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDoubleSizeTransfer(
           T_screen destination,
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_word16 dest_x,
           T_word16 dest_y)
{
    T_sword16 width ;
    T_sword16 height ;
    T_sword16 x, i ;
    T_byte8 *p_screenFrom ;
    T_byte8 *p_screenFrom2 ;
    T_byte8 *p_screenTo ;
    T_byte8 *p_screenTo2 ;
    T_byte8 b ;

    DebugRoutine("GrDoubleSizeTransfer") ;
    DebugCheck(destination != G_ActiveScreen) ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    /* Calculate the width and height of this rectangle. */
    width = x_right - x_left + 1 ;
    height = y_bottom - y_top + 1 ;

    DebugCheck(dest_x+width <= 320) ;
    DebugCheck(dest_y+height <= 200) ;

    /* Find the starting point of where we are going to copy from. */
    p_screenFrom = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;

    /* Find the starting point of where we are going to transfer to. */
    p_screenTo = destination+(dest_y<<8)+(dest_y<<6)+dest_x;

    /* Loop through each line in the rectangle. */
    for (; height>0; height--)  {
        for (i=0; i<2; i++)  {
            p_screenFrom2 = p_screenFrom ;
            p_screenTo2 = p_screenTo ;

            for (x=0; x<width; x++, p_screenFrom2++)  {
                b = *p_screenFrom2 ;
                *(p_screenTo2++) = b ;
                *(p_screenTo2++) = b ;
            }
            p_screenTo += 320 ;
        }
        p_screenFrom += 320 ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCompressedBitmap
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCompressedBitmap is just like GrDrawBitmap, but takes in a
 *  compressed and rotated bitmap (.CPC file).
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a compressed bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_word16 offset ;
    T_byte8 start, end ;
} T_compressEntry ;

T_void GrDrawCompressedBitmap(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_word16 y_start ;
    T_word16 x_size ;
    T_compressEntry *p_entry ;
    T_word16 i, count ;
    T_word16 x ;
    T_byte8 pixel ;

    DebugRoutine("GrDrawCompressedBitmap") ;

    DebugCheck(p_bitmap != NULL) ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    /* NOTE:  It may look backward, but since it is rotated, */
    /* sizeX is sizeY and visa-versa. */
    DebugCheck(x_left+p_bitmap->sizey <= 320) ;
    DebugCheck(y_top+p_bitmap->sizex <= 200) ;

    GrInvalidateRectClipped(
        x_left,
        y_top,
        x_left+p_bitmap->sizey,
        y_top+p_bitmap->sizex) ;

    /* Get a quick x size. */
    /* NOTE:  It may look backward, but since it is rotated, */
    /* sizeX is sizeY and visa-versa. */
    x_size = p_bitmap->sizey ;

    /* Get into the index. */
    p_entry = (T_compressEntry *)(p_bitmap->data) ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (x=0; x<x_size; x++, p_entry++)  {
        if (p_entry->start != 255)  {
            /* Look up the bitmap entry. */
            p_bitmapData = &(((T_byte8 *)p_bitmap)[p_entry->offset]) ;

            /* Where on the screen does it go next? */
            y_start = y_top + p_entry->start ;
            p_screen = G_ActiveScreen+(y_start<<8)+(y_start<<6)+x+x_left ;

            /* Get the count of how many pixels to copy. */
            count = 1+p_entry->end - p_entry->start ;

            /* Loop over that line. */
            for (i=0; i<count; i++, p_bitmapData++)  {
                /* Copy the pixel to the screen. */
                pixel = *p_bitmapData ;
                if (pixel)
                    *p_screen = pixel ;

                /* Skip down to the next screen line. */
                p_screen += 320 ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCompressedBitmapAndColor
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCompressedBitmapAndColor is just like GrDrawCompressedBitmap
 *  except that you can also colorize the compressed bitmap with the
 *  given colorization table.
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a compressed bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *  @param colorTable -- colorization table to use.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawCompressedBitmapAndColor(
           T_bitmap *p_bitmap,
           T_word16 x_left,
           T_word16 y_top,
           E_colorizeTable colorTable)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_word16 y_start ;
    T_word16 x_size ;
    T_compressEntry *p_entry ;
    T_word16 i, count ;
    T_word16 x ;
    T_byte8 pixel ;
    T_byte8 *p_colorize ;

    DebugRoutine("GrDrawCompressedBitmapAndColor") ;

    DebugCheck(p_bitmap != NULL) ;
    DebugCheck(x_left < 320) ;
    DebugCheck(y_top < 200) ;
    /* NOTE:  It may look backward, but since it is rotated, */
    /* sizeX is sizeY and visa-versa. */
    DebugCheck(x_left+p_bitmap->sizey <= 320) ;
    DebugCheck(y_top+p_bitmap->sizex <= 200) ;

    if (colorTable == COLORIZE_TABLE_NONE)  {
        GrDrawCompressedBitmap(p_bitmap, x_left, y_top) ;
    } else {
        GrInvalidateRectClipped(
            x_left,
            y_top,
            x_left+p_bitmap->sizey,
            y_top+p_bitmap->sizex) ;
        /* Get a quick x size. */
        /* NOTE:  It may look backward, but since it is rotated, */
        /* sizeX is sizeY and visa-versa. */
        x_size = p_bitmap->sizey ;

        /* Get into the index. */
        p_entry = (T_compressEntry *)(p_bitmap->data) ;

        /* Get the colorization table. */
        p_colorize = ColorizeGetTable(colorTable) ;

        /* This is the fastest way to blip an unmasked bitmap on the screen */
        /* Without going into assembly language. */
        for (x=0; x<x_size; x++, p_entry++)  {
            if (p_entry->start != 255)  {
                /* Look up the bitmap entry. */
                p_bitmapData = &(((T_byte8 *)p_bitmap)[p_entry->offset]) ;

                /* Where on the screen does it go next? */
                y_start = y_top + p_entry->start ;
                p_screen = G_ActiveScreen+(y_start<<8)+(y_start<<6)+x+x_left ;

                /* Get the count of how many pixels to copy. */
                count = 1+p_entry->end - p_entry->start ;

                /* Loop over that line. */
                for (i=0; i<count; i++, p_bitmapData++)  {
                    /* Copy the pixel to the screen. */
                    pixel = *p_bitmapData ;
                    if (pixel)
                        *p_screen = p_colorize[pixel] ;

                    /* Skip down to the next screen line. */
                    p_screen += 320 ;
                }
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCompressedBitmapAndClip
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCompressedBitmapAndClip is just like GrDrawCompressedBitmap,
 *  but it clips to any coordinates on the screen.
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a compressed bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawCompressedBitmapAndClip(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_sword16 y_start ;
    T_word16 x_size ;
    T_compressEntry *p_entry ;
    T_sword16 i, count ;
    T_sword16 x ;
    T_byte8 pixel ;
    T_sword16 y ;

    DebugRoutine("GrDrawCompressedBitmapAndClip") ;

    DebugCheck(p_bitmap != NULL) ;

    GrInvalidateRectClipped(
        x_left,
        y_top,
        x_left+p_bitmap->sizey,
        y_top+p_bitmap->sizex) ;
    /* Get a quick x size. */
    /* NOTE:  It may look backward, but since it is rotated, */
    /* sizeX is sizeY and visa-versa. */
    x_size = p_bitmap->sizey ;

    /* Get into the index. */
    p_entry = (T_compressEntry *)(p_bitmap->data) ;

    x = 0 ;
    /* Clip to the left. */
    if (x_left < 0)  {
        p_entry -= x_left ;
        x = -x_left ;
    }

    /* Clip to the right. */
    if (x_left+x_size >= 320)
        x_size -= ((x_left+x_size)-320) ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (; x<x_size; x++, p_entry++)  {
        if (p_entry->start != 255)  {
            /* Look up the bitmap entry. */
            p_bitmapData = &(((T_byte8 *)p_bitmap)[p_entry->offset]) ;

            /* Where on the screen does it go next? */
            y_start = y_top + p_entry->start ;
            p_screen = G_ActiveScreen+(y_start<<8)+(y_start<<6)+x+x_left ;

            /* Get the count of how many pixels to copy. */
            count = 1+p_entry->end - p_entry->start ;

            /* Clip to the bottom. */
            if (count+y_start > 200)
                count -= ((count+y_start)-200) ;

            /* Loop over that line. */
            for (y=y_start, i=0; i<count; i++, p_bitmapData++, y++)  {
                if (y>=0)  {
                    /* Copy the pixel to the screen. */
                    pixel = *p_bitmapData ;
                    if (pixel)
                        *p_screen = pixel ;
                }

                /* Skip down to the next screen line. */
                p_screen += 320 ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCompressedBitmapAndClipAndColor
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCompressedBitmapAndClipAndColor is just like
 *  GrDrawCompressedBitmapAndClip, but it also includes a colorization.
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a compressed bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *  @param colorTable -- Color to transform into
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawCompressedBitmapAndClipAndColor(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top,
           E_colorizeTable colorTable)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_sword16 y_start ;
    T_word16 x_size ;
    T_compressEntry *p_entry ;
    T_sword16 i, count ;
    T_sword16 x ;
    T_byte8 pixel ;
    T_sword16 y ;
    T_byte8 *p_colorize ;

    DebugRoutine("GrDrawCompressedBitmapAndClipAndColor") ;

    DebugCheck(p_bitmap != NULL) ;

    if (colorTable == COLORIZE_TABLE_NONE)  {
        GrDrawCompressedBitmapAndClip(
            p_bitmap,
            x_left,
            y_top) ;
    } else {
        GrInvalidateRectClipped(
            x_left,
            y_top,
            x_left+p_bitmap->sizey,
            y_top+p_bitmap->sizex) ;

        p_colorize = ColorizeGetTable(colorTable) ;

        /* Get a quick x size. */
        /* NOTE:  It may look backward, but since it is rotated, */
        /* sizeX is sizeY and visa-versa. */
        x_size = p_bitmap->sizey ;

        /* Get into the index. */
        p_entry = (T_compressEntry *)(p_bitmap->data) ;

        x = 0 ;
        /* Clip to the left. */
        if (x_left < 0)  {
            p_entry -= x_left ;
            x = -x_left ;
        }

        /* Clip to the right. */
        if (x_left+x_size >= 320)
            x_size -= ((x_left+x_size)-320) ;

        /* This is the fastest way to blip an unmasked bitmap on the screen */
        /* Without going into assembly language. */
        for (; x<x_size; x++, p_entry++)  {
            if (p_entry->start != 255)  {
                /* Look up the bitmap entry. */
                p_bitmapData = &(((T_byte8 *)p_bitmap)[p_entry->offset]) ;

                /* Where on the screen does it go next? */
                y_start = y_top + p_entry->start ;
                p_screen = G_ActiveScreen+(y_start<<8)+(y_start<<6)+x+x_left ;

                /* Get the count of how many pixels to copy. */
                count = 1+p_entry->end - p_entry->start ;

                /* Clip to the bottom. */
                if (count+y_start > 200)
                    count -= ((count+y_start)-200) ;

                /* Loop over that line. */
                for (y=y_start, i=0; i<count; i++, p_bitmapData++, y++)  {
                    if (y>=0)  {
                        /* Copy the pixel to the screen. */
                        pixel = *p_bitmapData ;
                        if (pixel)
                            *p_screen = p_colorize[pixel] ;
                    }

                    /* Skip down to the next screen line. */
                    p_screen += 320 ;
                }
            }
        }
    }

    DebugEnd() ;
}

#define RETRACE_INTERRUPT_NUMBER 0x71
#define RETRACE_VERTICAL_INTERRUPT_BIT 0x02
#define RETRACE_VERTICAL_CLEAR_BIT 0x10

#define PIC2 0xA0

static T_void IInstallVerticalInterrupt(T_void)
{
#if defined(DOS32)
outp(0x3D4, 0x11) ;
printf("currently %02X\n", inp(0x3D5)) ;
    _disable() ;
outp(0x3D5, inp(0x3D5) & 0x7F) ;
    IOldRetraceInterrupt = _dos_getvect(RETRACE_INTERRUPT_NUMBER);
    _dos_setvect(RETRACE_INTERRUPT_NUMBER, IVerticalRetrace);

    /* Turn on the vertical interrupt. */
    outp(0x3D4, 0x11) ;
    outp(0x3D5, (inp(0x3D5) & (~RETRACE_VERTICAL_INTERRUPT_BIT))) ;
//    outp(PIC2+1, (inp(PIC2+1) | (RETRACE_VERTICAL_INTERRUPT_BIT)) ;
    outp(PIC2+1, 0) ;
//    outp(0x3D4, 0x11) ;
//    outp(0x3D5, (inp(0x3D5) | (RETRACE_VERTICAL_INTERRUPT_BIT))) ;
/*
    outp(0x3D4, 0x18) ;
    outp(0x3D5, 0x30) ;
    outp(0x3D4, 0x7) ;
    outp(0x3D5, inp(0x3D5) & 0x7F) ;
    outp(0x3D4, 0x9) ;
    outp(0x3D5, inp(0x3D5) & 0xBF) ;
*/
    inp(0x3C2) ;

    _enable() ;
outp(0x3D4, 0x11) ;
printf("currently %02X\n", inp(0x3D5)) ;
fprintf(stderr, "vertical on") ;
#elif defined(WIN32)
// TODO:
#endif
}

static T_void IUninstallVerticalInterrupt(T_void)
{
#if defined(DOS32)
    _disable() ;
    _dos_setvect(RETRACE_INTERRUPT_NUMBER, IOldRetraceInterrupt);

    /* Turn off the vertical interrupt. */
    outp(0x3D4, 0x11) ;
    outp(0x3D5, (inp(0x3D5) | (RETRACE_VERTICAL_INTERRUPT_BIT))) ;
    outp(0x21, (inp(0x21) | 0x04)) ;
    _enable() ;
fprintf(stderr, "vertical off") ;
#elif defined(WIN32)
#endif
}

#if defined(WATCOM)
static T_void __interrupt __far IVerticalRetrace(T_void)
{
    T_word16 i ;
    T_byte8 *p_color ;

    ((char *)0xA0000000)[320] ^= 0xFF ;

G_verticalCount++ ;
    if (inp(0x3C2) & 0x80)  {

        if (G_paletteChanged)  {
            /* Set the index color to start chaning. */
            outp(0x3C8, 0) ;

            p_color = (T_byte8 *)&G_palette ;

            for (i=768; i--;)  {
                outp(0x03C9, *(p_color++)) ;
                ((char *)0xA0000000)[i] ^= 0xFF ;
            }

            /* Don't change the palette the next time. */
            G_paletteChanged = FALSE ;
        }

        /* Clear the vertical retrace bit. */
        outp(0x3D4, 0x11) ;
        outp(0x3D5, (inp(0x3D5) & (~RETRACE_VERTICAL_CLEAR_BIT))) ;
    }

    /* Call the old BIOS routine. */
    IOldRetraceInterrupt() ;
}
#endif

static T_void ICheckPaletteChange(T_void)
{
    DebugRoutine("ICheckPaletteChange") ;

    if (G_paletteChanged)  {
        if (inp(0x03DA) & 8)  {
            ITransferPalette() ;
        }
    }

    DebugEnd() ;
}

static T_void ITransferPalette(T_void)
{
    T_byte8 *p_color ;
#ifdef DOS32
    int i;
#endif

    p_color = G_palette ;

#ifdef DOS32
    _disable() ;
    outp(0x3C8, 0) ;
    for (i=0; i<768; i++)
        outp(0x03C9, *(p_color++)) ;
    _enable() ;
#endif
}

static T_void IConfirmPaletteChange(T_void)
{
#ifdef DOS32
    if (G_paletteChanged)  {
        if (inp(0x03DA) & 8)  {
            ITransferPalette() ;
        }
    }
#else
    if (G_paletteChanged)  {
        ITransferPalette() ;
    }
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  GrScreenAllocPartial
 *-------------------------------------------------------------------------*/
/**
 *  This routine is needed to allocate enough space for a partial screen.
 *  To actually use this screen, you need to use GrScreenSet().  It
 *  will then make that screen the active screen.
 *
 *  NOTE: 
 *  Warning!  Since this routine returns a handle to a partial screen,
 *  you have to check that you do not draw out of bounds.
 *
 *  @return Handle to a partial screen.
 *
 *<!-----------------------------------------------------------------------*/
T_screen GrScreenAllocPartial(T_word16 ySize)
{
    T_screen screen ;

    DebugRoutine("GrScreenAllocPartial") ;
    DebugCheck(ySize < 200) ;

    /* Allocate enough memory for a 320x200 area. */
    screen = MemAlloc((T_word16)(320*ySize)) ;
    DebugCheck(screen != NULL) ;

    DebugEnd() ;

    return screen ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DrawAndShadeRaster
 *-------------------------------------------------------------------------*/
/**
 *  DrawAndShadeRaster copies a section of memory from one place to
 *  another and shades it in the process.
 *
 *  NOTE: 
 *  Warning!  Since this routine returns a handle to a partial screen,
 *  you have to check that you do not draw out of bounds.
 *
 *  @param p_source -- Where to copy from
 *  @param p_destination -- Where to copy to
 *  @param count -- How many bytes to copy and shade
 *  @param shade -- Shading value
 *      (0-255, 0=black, 255=normal)
 *
 *<!-----------------------------------------------------------------------*/
T_void DrawAndShadeRaster(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count,
           T_byte8 shade)
{
    T_byte8 *p_shadeLookup ;

    DebugRoutine("DrawAndShadeRaster") ;
    DebugCheck(p_source != NULL) ;
    DebugCheck(p_destination != NULL) ;

    if (count)  {
        p_shadeLookup = &P_shadeIndex[((((T_word16)shade)>>2)&0x3F)<<8] ;
        ShadeMemAsm(
            p_source,
            p_destination,
            count,
            p_shadeLookup) ;
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  GrShadeRectangle
 *-------------------------------------------------------------------------*/
/**
 *  GrShadeRectangle darkens a given rectangular area.
 *
 *  NOTE: 
 *  All coordinates must be on the screen and the upper-left coordinate
 *  must be located to the left and above the lower-right corner.
 *
 *  @param x_left -- Left of rectangle
 *  @param y_top -- Top of rectangle
 *  @param x_right -- Right of rectangle
 *  @param y_bottom -- Bottom of rectangle
 *  @param shade -- Amount to shade
 *
 *<!-----------------------------------------------------------------------*/
T_void GrShadeRectangle(
           T_word16 x_left,
           T_word16 y_top,
           T_word16 x_right,
           T_word16 y_bottom,
           T_byte8 shade)
{
    T_sword16 width ;
    T_sword16 height ;
    T_byte8 *p_screen ;

    DebugRoutine("GrShadeRectangle") ;
    DebugCheck(x_right < 320) ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_bottom < 200) ;
    DebugCheck(y_top <= y_bottom) ;

    GrInvalidateRect(x_left, y_top, x_right, y_bottom) ;

    width = x_right-x_left+1 ;
    height = y_bottom-y_top+1 ;

    /* Find the starting point of where we are going to fill. */
    p_screen = G_ActiveScreen+(y_top<<8)+(y_top<<6)+x_left ;

    /* Loop through each line and fill those lines. */
    for (; height > 0; height--, p_screen+=320)
        DrawAndShadeRaster(p_screen, p_screen, width, shade) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  GrActualScreenPush
 *-------------------------------------------------------------------------*/
/**
 *  GrActualScreenPush saves the current screen onto the screen stack.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrActualScreenPush(T_void)
{
    T_byte8 *p_screen ;
    DebugRoutine("GrActualScreenPush") ;

    p_screen = MemAlloc(64000) ;
    DebugCheck(p_screen != NULL) ;

    if (p_screen)
        memcpy(p_screen, ((char *)GRAPHICS_ACTUAL_SCREEN), 64000) ;

    DoubleLinkListAddElementAtFront(G_screenStack, p_screen) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrActualScreenPop
 *-------------------------------------------------------------------------*/
/**
 *  GrActualScreenPop restores the last screen on the screen stack.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrActualScreenPop(T_void)
{
    T_byte8 *p_screen ;
    T_doubleLinkListElement element ;

    DebugRoutine("GrActualScreenPop") ;

    element = DoubleLinkListGetFirst(G_screenStack) ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    p_screen = (T_byte8 *)DoubleLinkListElementGetData(element) ;

    if (p_screen)  {
#ifdef DOS32
        memcpy(((char *)0xA0000), p_screen, 64000) ;
#endif
        memcpy((char *)GRAPHICS_ACTUAL_SCREEN, p_screen, 64000) ;
        MemFree(p_screen) ;
    }

    DoubleLinkListRemoveElement(element) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  GrDrawCompressedBitmapAndClipAndColor
 *-------------------------------------------------------------------------*/
/**
 *  GrDrawCompressedBitmapAndClipAndColor is just like
 *  GrDrawCompressedBitmapAndClip, but it also includes a colorization.
 *
 *  NOTE: 
 *  No matter how much you want it to, this routine does NOT clip the
 *  bitmap at the edge on the screen and trying to will cause it to bomb.
 *
 *  @param p_bitmap -- Pointer to a compressed bitmap picture
 *  @param x_left -- Position of the left
 *  @param y_top -- Position of the top
 *  @param colorTable -- Color to transform into
 *  @param width -- Width of area to fill if too big,
 *      or center if too small.
 *  @param height -- Height of area to fill if too big,
 *      or center if too small.
 *
 *<!-----------------------------------------------------------------------*/
T_void GrDrawCompressedBitmapAndClipAndColorAndCenterAndResize(
           T_bitmap *p_bitmap,
           T_sword16 x_left,
           T_sword16 y_top,
           E_colorizeTable colorTable,
           T_word16 width,
           T_word16 height)
{
    T_byte8 *p_bitmapData ;
    T_byte8 *p_screen ;
    T_sword16 y_start ;
    T_word16 x_size ;
    T_word16 y_size ;
    T_compressEntry *p_entry ;
    T_compressEntry *p_entries ;
    T_sword16 i, count ;
    T_sword16 x ;
    T_byte8 pixel ;
    T_sword16 y ;
    T_byte8 *p_colorize ;
    T_sword32 deltaX, deltaY ;
    T_word16 newHeight, newWidth ;
    T_sword32 xFract, yFract ;
    T_word16 x_width;
    T_word16 yOffset ;

    DebugRoutine("GrDrawCompressedBitmapAndClipAndColorAndCenterAndResize") ;

    DebugCheck(p_bitmap != NULL) ;

    GrInvalidateRect(x_left, y_top, x_left+width-1, y_top+height-1) ;

    if (colorTable != COLORIZE_TABLE_NONE)
        p_colorize = ColorizeGetTable(colorTable) ;
    else
        p_colorize = NULL ;

    /* Get a quick x size and y size. */
    /* NOTE:  It may look backward, but since it is rotated, */
    /* sizeX is sizeY and visa-versa. */
    newWidth = x_size = p_bitmap->sizey ;
    newHeight = y_size = p_bitmap->sizex ;

    /* Is the x too big? */
    if (x_size >= width)  {
        /* Picture is wider or equal than width -- scale. */
        deltaX = (((T_sword32)x_size) << 16) / width ;
    } else {
        /* Picture is smaller than width -- center. */
        deltaX = 0x10000 ;
    }

    /* Is the y too big? */
    if (y_size >= height)  {
        /* Picture is wider or equal than width -- scale. */
        deltaY = (((T_sword32)y_size) << 16) / height ;
    } else {
        /* Picture is smaller than width -- center. */
        deltaY = 0x10000 ;
    }

    /* Since we want the scaling to be square, */
    /* we need to adjust for a smaller X or a smaller Y. */
    if (deltaX > deltaY)  {
        /* Taking bigger steps along x than y */
        /* Make the y smaller (by taking bigger steps) and shift the */
        /* image down to center it. */
        deltaY = deltaX ;
        newHeight = (((T_word32)y_size)<<16) / deltaY ;
    } else if (deltaY > deltaX)  {
        /* Taking bigger steps along y than x */
        deltaX = deltaY ;
        newWidth = (((T_word32)x_size)<<16) / deltaX ;
    }

    /* Center */
    if (newWidth < width)
        x_left += ((width - newWidth)>>1) ;
    if (newHeight < height)
        y_top += ((height - newHeight)>>1) ;

    /* Get into the index. */
    p_entries = (T_compressEntry *)(p_bitmap->data) ;

    x = 0 ;
    /* Clip to the left. */
    if (x_left < 0)  {
        //TODO: p_entry -= x_left ;
        x = -x_left ;
    }

    x_width = (((T_word32)x_size)<<16) / deltaX ;

    /* Clip to the right. */
    if (x_left+x_width >= 320)
        x_width -= ((x_left+x_width)-320) ;

    /* This is the fastest way to blip an unmasked bitmap on the screen */
    /* Without going into assembly language. */
    for (x=0, p_entry=p_entries, xFract = 0; x<x_width; x++)  {
        if (p_entry->start != 255)  {
            /* Look up the bitmap entry. */
            p_bitmapData = &(((T_byte8 *)p_bitmap)[p_entry->offset]) ;

            /* Where on the screen does it go next? */
            y_start = y_top + ((((T_sword32)p_entry->start)<<16) / deltaY) ;
            p_screen = G_ActiveScreen+(y_start<<8)+(y_start<<6)+x+x_left ;

            /* Get the count of how many pixels to copy. */
            count = 1+p_entry->end - p_entry->start ;

            /* Adjust for scaling */
            count = (((T_sword32)count)<<16)/deltaY ;

            /* Clip to the bottom. */
            if (count+y_start > 200)
                count -= ((count+y_start)-200) ;

            /* Loop over that line. */
            yFract = 0 ;
            yOffset = 0 ;
            for (y=y_start, i=0; i<count; i++, y++)  {
                if (y>=0)  {
                    /* Copy the pixel to the screen. */
                    pixel = p_bitmapData[yOffset] ;
                    if (pixel)  {
                        if (p_colorize)
                            *p_screen = p_colorize[pixel] ;
                        else
                            *p_screen = pixel ;
                    }
                }

                /* Skip down to the next screen line. */
                p_screen += 320 ;
                yFract += deltaY ;
                yOffset = (yFract>>16) ;
            }
        }
        xFract += deltaX ;
        p_entry = p_entries + (xFract>>16) ;
    }

    DebugEnd() ;
}

static T_void IResetLeftsAndRights(T_void)
{
    memset(G_lefts, 0x7F, sizeof(G_lefts)) ;
    memset(G_rights, 0xFF, sizeof(G_rights)) ;
}

T_void GrInvalidateRect(
                  T_sword16 x_left,
                  T_sword16 y_top,
                  T_sword16 x_right,
                  T_sword16 y_bottom)
{
    T_word16 i ;
    T_sword16 *p_edge ;

if (x_left < 0)
    x_left = 0 ;
if (y_top < 0)
    y_top = 0 ;
if (x_right >= SCREEN_SIZE_X)
    x_right = SCREEN_SIZE_X-1 ;
if (y_bottom >= SCREEN_SIZE_Y)
    y_bottom = SCREEN_SIZE_Y-1 ;

    DebugRoutine("GrInvalidateRect") ;
    DebugCheck(x_left <= x_right) ;
    DebugCheck(y_top <= y_bottom) ;
    DebugCheck(y_top >= 0) ;
    DebugCheck(x_left >= 0) ;
    DebugCheck(x_right < SCREEN_SIZE_X) ;
    DebugCheck(y_bottom < SCREEN_SIZE_Y) ;

    /* Check left egdes */
    i = 1+y_bottom - y_top ;
    p_edge = G_lefts + y_top ;
    while (i--)  {
        if (x_left < *p_edge)
            *p_edge = x_left ;
        p_edge++ ;
    }

    /* Check right egdes */
    i = 1+y_bottom - y_top ;
    p_edge = G_rights + y_top ;
    while (i--)  {
        if (x_right > *p_edge)
            *p_edge = x_right ;
        p_edge++ ;
    }

    DebugEnd() ;
}

T_void GrInvalidateRectClipped(
                  T_sword16 x_left,
                  T_sword16 y_top,
                  T_sword16 x_right,
                  T_sword16 y_bottom)
{
    DebugRoutine("GrInvalidateRectClipped") ;

    if (x_left < 0)
        x_left = 0 ;
    if (y_top < 0)
        y_top = 0 ;
    if (x_right >= SCREEN_SIZE_X)
        x_right = SCREEN_SIZE_X-1 ;
    if (y_bottom >= SCREEN_SIZE_Y)
        y_bottom = SCREEN_SIZE_Y-1 ;

    GrInvalidateRect(x_left, y_top, x_right, y_bottom) ;

    DebugEnd() ;
}

#ifdef WIN32
T_void DrawTranslucentAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count)
{
    while (count--)
        *(p_destination++) = G_translucentTable[*(p_source)++][*p_destination];
}

T_void DrawTranslucentSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count)
{
    T_byte8 c;

    while (count--) {
        c = *(p_source)++;
        if (c)
            *p_destination = G_translucentTable[c][*p_destination];
        p_destination++;
    }
}

T_void DrawSeeThroughAsm(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count)
{
    T_byte8 c;

    while (count--) {
        c = *(p_source)++;
        if (c)
            *p_destination = c;
        p_destination++;
    }
}

T_void ShadeMemAsm(
          T_byte8 *p_source,
          T_byte8 *p_destination,
          T_word32 count,
          T_byte8 *p_transTable)
{
    while (count--)
        *(p_destination++) = p_transTable[*(p_source)++];
}

#endif
/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  GRAPHICS.C
 *-------------------------------------------------------------------------*/
