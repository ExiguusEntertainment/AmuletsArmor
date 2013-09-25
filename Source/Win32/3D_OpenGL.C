/*-------------------------------------------------------------------------*
 * File:  3D_VIEW.C
 *-------------------------------------------------------------------------*/
/**
 * All 3D rendering is done through this section of code.
 *
 * @addtogroup _3D_VIEW
 * @brief 3D Rendering
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
//#define NDEBUG
#include <math.h>
#define M_PI        3.14159265358979323846
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "GRAPHICS.H"
#include "OBJECT.H"
#include "PLAYER.H"
#include "MAP.H"
#include "MEMORY.H"
#include "PICS.H"
#include "TICKER.H"
#include "VIEW.H"

#include "MESSAGE.H"

#include <Windows.h>
#include <SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

static T_word16 G_fromSector ;
static GLuint G_texture;

typedef struct {
    T_word16 offset ;
    T_byte8 start ;
    T_byte8 end ;
} T_pictureRaster ;

/** BEGIN VIEW-DRAWING-SPECIFIC BLOCK **/
#ifndef SERVER_ONLY

#ifndef NDEBUG
static T_void ITestMinMax(T_word16 where) ;
#else
#define ITestMinMax(where)
#endif

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
/* Coordinates of where the mouse is. */
static T_sword16 G_mouseAtX ;
static T_sword16 G_mouseAtY ;
static T_word16 G_textureSideNum ;
#endif

#if defined(WIN32)
/*
T_sword32 MultAndShift4(
           T_sword32 a,
           T_sword32 b) ;
T_sword32 MultAndShift6(
           T_sword32 a,
           T_sword32 b) ;
T_sword32 MultAndShift16(
           T_sword32 a,
           T_sword32 b) ;
T_sword32 MultAndShift22(
           T_sword32 a,
           T_sword32 b) ;
T_sword32 MultAndShift32(
           T_sword32 a,
           T_sword32 b) ;
*/
#define MultAndShift32(a, b)  ((T_sword32)(((((double)(a)) * ((double)(b))) / 65536.0) / 65536.0))
#define MultAndShift22(a, b)  ((T_sword32)((((double)(a)) * ((double)(b))) / 4194304.0))
#define MultAndShift16(a, b)  ((T_sword32)((((double)(a)) * ((double)(b))) / 65536.0))
#define MultAndShift6(a, b)  ((T_sword32)((((double)(a)) * ((double)(b))) / 64.0))
#define MultAndShift4(a, b)  ((T_sword32)((((double)(a)) * ((double)(b))) / 16.0))
#define Div32by32To1616Asm(a, b)  ((T_sword32)( (((double)(a)) * 65536.0) / ((double)(b))) )
#else
/* f(a, b) = (a * b) >> 4 */
T_sword32 MultAndShift6(
           T_sword32 a,
           T_sword32 b) ;
#pragma aux MultAndShift4 = \
            "imul ebx" \
            "shrd eax, edx, 4" \
            parm [eax] [ebx] \
            value [eax] \
            modify [eax edx] ;

/* f(a, b) = (a * b) >> 6 */
T_sword32 MultAndShift6(
           T_sword32 a,
           T_sword32 b) ;
#pragma aux MultAndShift6 = \
            "imul ebx" \
            "shrd eax, edx, 6" \
            parm [eax] [ebx] \
            value [eax] \
            modify [eax edx] ;

/* f(a, b) = (a * b) >> 16 */
T_sword32 MultAndShift16(
           T_sword32 a,
           T_sword32 b) ;
#pragma aux MultAndShift16 = \
            "imul ebx" \
            "shrd eax, edx, 16" \
            parm [eax] [ebx] \
            value [eax] \
            modify [eax edx] ;

/* f(a, b) = (a * b) >> 32 */
T_sword32 MultAndShift22(
           T_sword32 a,
           T_sword32 b) ;
#pragma aux MultAndShift22 = \
            "imul ebx" \
            "shrd eax, edx, 22" \
            parm [eax] [ebx] \
            value [edx] \
            modify [eax edx] ;


/* f(a, b) = (a * b) >> 32 */
T_sword32 MultAndShift32(
           T_sword32 a,
           T_sword32 b) ;
#pragma aux MultAndShift32 = \
            "imul ebx" \
            parm [eax] [ebx] \
            value [edx] \
            modify [eax edx] ;
#endif

#if defined(WATCOM)
#pragma aux FindInterXAsm parm [EDI]
#endif
T_void ClearSampleAsm(T_byte8 *ptr) ;

#if defined(WATCOM)
#pragma aux FindInterXAsm parm [ESI] [EAX] [EBX] [EDI]
#endif
T_sword32 FindInterXAsm(
             T_sword32 deltaX,
             T_sword32 deltaZ,
             T_sword32 tanViewAngle,
             T_sword32 zTop) ;

#if defined(WATCOM)
#pragma aux  DrawObjectColumnAsm   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawObjectColumnAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentObjectColumnAsm   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentObjectColumnAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm1   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm2   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm4   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm8   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm16   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm32   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm64   parm	[EBX] [ECX] [EDX] [ESI] [EDI] modify [ecx esi]
#endif
T_void DrawTextureColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm128   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureColumnAsm256   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTextureColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm1   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm2   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm4   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm8   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm16   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm32   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm64   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm128   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransparentColumnAsm256   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransparentColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm1   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm2   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm4   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm8   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm16   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm32   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm64   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm128   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTranslucentColumnAsm256   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTranslucentColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm1   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm2   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm4   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm8   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm16   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm32   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm64   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm128   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTextureRowAsm256   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTextureRowAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransColumnAsm   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawTransColumnAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm1   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm2   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm4   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm8   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm16   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm32   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm64   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm128   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
#pragma aux  DrawTransRowAsm256   parm	[EBX] [ECX] [ESI] [EDX] [EDI]
#endif
T_void DrawTransRowAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel) ;

#if defined(WATCOM)
//#pragma aux Div32by32To1616Asm parm [EAX] [EBX]
#endif
#if defined(WATCOM)
T_sword32 Div32by32To1616Asm(
           T_sword32 dividend,
           T_sword32 divider) ;
#pragma aux Div32by32To1616Asm = \
            "cdq" \
            "shld edx, eax, 16" \
            "sal eax, 16" \
            "idiv ebx" \
            parm [eax] [ebx] \
            value [eax] \
            modify [edx] ;
#endif

T_sword32 MultAndDivideAsm(
              T_sword32 multA,
              T_sword32 multB,
              T_sword32 divC) ;
#if defined(WATCOM)
#pragma aux MultAndDivideAsm = \
                "imul ecx" \
                "idiv ebx" \
                parm [eax] [ecx] [ebx] \
                value [eax] \
                modify [edx] ;
#endif

#endif /** SERVER_ONLY **/


static T_sword32 G_relativeFromZ ;
static T_sword32 G_relativeToZ ;
static T_sword32 G_relativeFromX ;
static T_sword32 G_relativeToX ;
static T_sword32 G_relativeFromXOld ;
static T_sword32 G_relativeFromZOld ;
static T_byte8 *G_doublePtrLookup[MAX_VIEW3D_HEIGHT] ;
static T_3dSide *P_sideFront ;
static T_3dSide *P_sideBack ;
static T_sword16 G_deltaFloors, G_deltaCeilings;
static T_3dSegment *P_segment ;
static T_sword16 G_xLeft, G_xRight, G_screenXLeft, G_screenXRight ;
static T_sword16 G_relativeTop ;
static T_sword16 G_relativeBottom ;
static T_sword16 G_eyeLevel ;
static T_sword32 G_eyeLevel32 ;
static E_Boolean G_newLine ;

/* Internal prototypes: */
E_Boolean IIsSegmentGood(T_word16 segmentIndex) ;
T_void ICalculateWallMatrix(T_void) ;
T_void IMarkOffWall(T_void) ;
T_void IAddMainWall(T_void) ;
T_void IAddLowerWall(T_void) ;
T_void IAddUpperWall(T_void) ;
T_byte8 *IDetermineShade(T_sword32 distance, T_word16 shadeStart) ;
T_void IDrawTextureColumnNew(
           T_byte8 *p_shade,
           T_word16 numPixels,
           T_word32 textureStep,
           T_word32 textureOffset,
           T_byte8 *p_pixel,
           T_byte8 shift) ;
T_sbyte8 G_darknessAdjustment ;

typedef struct {
    T_word16 left ;
    T_word16 right ;
    T_word16 sector ;
    T_word16 reserved ;        /* Make a multiple of 8 bytes. */
} T_horzFloorInfo ;

static T_void IAddVertFloor(T_word16 x, T_sword16 top, T_sword16 bottom, T_word16 sector) ;
T_void IDumpVertFloor(T_void) ;
T_void IDrawFloorRun(T_word16 y, T_horzFloorInfo *p_floor) ;
static T_void IConvertVertToHorzAndDraw(T_void) ;
static T_void IAddChainedObjects(T_void) ;
static T_void IAddChainedObject(T_3dObject *p_obj) ;
T_byte8 View3dOnRightByNodeWithXY(
            T_word16 nodeIndex,
            T_sword16 x,
            T_sword16 y) ;

/* Internal Defines: */
#define VIEW_3D_LEFT_ANGLE 0x2000

#define VIEW3D_CROSS_RATIO (65536 / VIEW3D_WIDTH)

#define MAX_INTERSECTIONS 50
//#define MAX_INTERSECTIONS 100
#define MAX_WALL_RUNS     8000
//#define MAX_WALL_RUNS     16000
#define MAX_FLOOR_INDEXES 40
//#define MAX_FLOOR_INDEXES 100
#define MAX_OBJECTS       500
#define MAX_OBJECT_COLUMNS 50

#define ZMIN            ((T_sword32) 10 << 16)

#define UPPER_TYPE      0
#define WALL_TYPE       1
#define LOWER_TYPE      2

T_sword16 VIEW3D_WIDTH = 300 ;
T_sword16 VIEW3D_HEIGHT = 140 ;
T_sword16 VIEW3D_HALF_WIDTH = 150 ;
T_sword16 VIEW3D_HALF_HEIGHT = 70 ;
T_sword16 VIEW3D_EVEN_HALF_HEIGHT = 70 ;
T_sword16 VIEW3D_CLIP_LEFT = 100 ;
T_sword16 VIEW3D_CLIP_RIGHT = 200 ;


///T_byte8 *GG_palette ;

T_byte8 *G_backdrop = NULL ;
T_word16 G_backdropOffset = 0 ;

typedef struct  {
    T_sword32 fromZ ;
    T_sword32 zTop ;
    T_sword32 interX, interZ ;
    T_sword32 a, c, d, e, f, g, i, eprime ;
    T_sword32 Bx, By, Bz ;
    T_sword32 Vx, Vz ;
    T_byte8 *p_texture ;
    T_byte8 *p_texture2 ;
    T_byte8 *textureFloor ;
    T_byte8 *textureCeiling ;
    T_word16 shadeIndex ;
    T_sword16 offX, offY ;
    T_word16 angle ;
    T_word16 sector ;
    T_word16 sectorBack ;
    T_byte8 type ;
    T_byte8 opaque ;
    T_byte8 transFlag ;
    T_byte8 reserved[3] ;              /* Makes 32 bit aligned. */
} T_3dWall;

typedef struct  {
    T_byte8 *textureFloor ;
    T_byte8 *textureCeiling ;
    T_sword16 height ;
    T_sword16 start ;
    T_sword16 end ;
    T_word16 shadeIndex ;
    T_word16 sector ;
    T_byte8 transparentFlag ;
    T_byte8 reserved[1] ;             /* Makes 32 bit aligned. */
} T_3dFloorRun ;

typedef struct  {
    T_sword32 realPixelTop ;      /* Accurate top pixel (<<16) */
    T_sword32 realPixelBottom ;      /* Accurate top pixel (<<16) */
    T_sword32 u ;
    T_sword32 du ;
    T_sword32 distance ;
    T_byte8 *p_texture ;
    T_sword16 top ;
    T_sword16 bottom ;
    T_sword16 absoluteTop ;       /* Non-relative height values. */
    T_sword16 absoluteBottom ;
    T_word16 textureColumn ;      /* What column to use in texture. */
    T_word16 shadeIndex ;
    T_sword16 offY ;

    T_word16 maskY ;              /* Information for texture masking. */
    T_word16 maskX ;
    T_byte8 shift ;

    T_byte8 reserved[1] ;         /* Roll over -- makes 32 bit aligned. */
    T_word32 pad[5] ;             /* Make a multiple of 256 per wall run. */
} T_3dWallRun ;

typedef struct {
    T_byte8 *p_picture ;
    T_3dObject *p_obj ;
    T_sword32 deltaTall ;
    T_sword32 distance ;
    T_sword16 top ;
    T_sword16 bottom ;
    T_sword16 realBottom ;
    T_sword16 light ;
    T_sword16 picHeight ;
    T_byte8 reserved[2] ;          /* Make 32 bit aligned. */
} T_3dObjectRunInfo ;

typedef struct {
    T_sword16 scrLeft, scrRight ;
    T_sword32 delta ;
    T_3dObjectRunInfo runInfo ;
} T_3dObjectRun ;

typedef struct {
    T_3dObjectRunInfo *p_runInfo ;
    T_sword16 clipTop, clipBottom ;
    T_sword16 column ;
    T_word16 next ;
    T_byte8 reserved[4] ;         /* Make 32 bit aligned and 16 bytes */
} T_3dObjectColRun ;

/* ---------------- WALL SLICE RELATED ------------------ */
typedef struct {
    T_byte8 *p_shade ;
    T_byte8 *p_pixel ;
    T_byte8 *p_texture ;
    T_word32 textureStep ;
    T_word32 textureOffset ;
    T_sword32 distance ;
    T_word16 numPixels ;
    T_byte8 shift ;
    T_byte8 type ;
    T_byte8 reserved[4] ;        /* Make 32 byte aligned. */
} T_3dWallSlice ;

#define MAX_WALL_SLICES_PER_COLUMN 10
T_3dWallSlice G_wallSlices[MAX_VIEW3D_WIDTH][MAX_WALL_SLICES_PER_COLUMN] ;
T_byte8 G_numWallSlices[MAX_VIEW3D_WIDTH] ;

static T_void IDrawTextureColumnLater(
                  T_word16 x,
                  T_byte8 *p_shade,
                  T_word16 numPixels,
                  T_word32 textureStep,
                  T_word32 textureOffset,
                  T_byte8 *p_pixel,
                  T_byte8 shift,
                  T_byte8 type,
                  T_sword32 distance,
                  T_byte8 *p_texture) ;

static T_void IDrawWallSliceColumn(T_3dWallSlice *p_slice) ;
/* ---------------- ---- ----- ------- ------------------ */


/* ---------------- FLOOR INFO RELATED ------------------ */
#define SECTOR_IS_WALL      0xFFFF
#define SECTOR_IS_UNKNOWN   0x8000
#define MAX_FLOOR_INFO      5000
#define NEXT_IS_NONE        0
#define SECTOR_NONE         0xFFFF

typedef struct {
    T_word16 x ;                  /* Screen X position (left of pixel) */
    T_word16 sectorLeft ;         /* Sector to left. */
    T_word16 sectorRight ;        /* Sector to right. */
    T_word16 next ;               /* Next in linked list of slices. */
} T_floorInfo ;

//T_word16 G_floorStarts[MAX_VIEW3D_HEIGHT] ;
//T_word16 G_numFloorInfo ;    /* MUST start at 1, not 0. */
//T_floorInfo G_floorInfo[MAX_FLOOR_INFO] ;

typedef struct {
    T_sword16 top ;
    T_sword16 bottom ;
    T_word16 sector ;
    T_word16 next ;
} T_vertFloorInfo ;
T_vertFloorInfo G_vertFloorInfo[MAX_FLOOR_INFO] ;
T_word16 G_vertFloorStarts[MAX_VIEW3D_WIDTH] ;
T_word16 G_numVertFloor ;   /* Always start at 1, ignore 0 */
/* ---------------- ------------------ ------------------ */

/* Array to remap sector ceilings and floors. */
T_word16 *G_remapSectorFloorArray ;
T_word16 *G_remapSectorCeilingArray ;

/* Wall columns are store in wall runs in this variable.  This list */
/* contains all the visible wall columns for this single viewing. */
//T_3dWallRun G_wallRuns[MAX_WALL_RUNS] ;

/* Like walls, there are also floor runs.  However, these go from */
/* left to right instead of up and down the screen. */
/*
T_3dFloorRun G_floorList[MAX_VIEW3D_HEIGHT][MAX_FLOOR_INDEXES] ;
T_word16 G_runCount[MAX_VIEW3D_HEIGHT] ;
*/
/* Keep track of the objects */
T_word16 G_objectCount ;
T_3dObjectRun G_objectRun[MAX_OBJECTS] ;

/* Keep track of the object columns. */
//T_3dObjectColRun G_objectColRun[MAX_VIEW3D_WIDTH][MAX_OBJECT_COLUMNS] ;
//T_word16 G_objectColCount[MAX_VIEW3D_WIDTH] ;

/* 0xFFFF means no objects in that column. */
T_word16 G_objectColStart[MAX_VIEW3D_WIDTH] ;
T_word16 G_allocatedColRun = 0 ;
#define MAX_OBJECT_COLUMN_RUNS  8000
T_3dObjectColRun G_objectColRunList[MAX_OBJECT_COLUMN_RUNS] ;

T_word16 G_screenObjectPosition[MAX_VIEW3D_WIDTH] ;
T_sword32 G_screenObjectDistance[MAX_VIEW3D_WIDTH] ;

/* Temporary variable to keep track of the current wall. */
T_3dWall             G_wall;

T_sword32 G_3dCosPlayerLeftAngle ;
T_sword32 G_3dSinPlayerLeftAngle ;
T_sword32 G_3dCosPlayerRightAngle ;
T_sword32 G_3dSinPlayerRightAngle ;

/* Keep track of the number of columns that have been drawn. */
T_word16 G_colCount ;

/* Note how many different walls we have found. */
T_word16 G_wallCount ;
T_word16 G_nodeCount ;
T_word16 G_segCount ;
T_word16 G_wallAttempt ;
#ifndef NDEBUG
E_Boolean G_didDrawWall ;
#endif

/* Store the number of wall runs in the wall run list. */
T_word16 G_wallRunCount ;

/* Keep track of the first segment sector. */
T_word16 G_firstSSector ;

/* Keep track if a screen column has been completely drawn. */
T_byte8 G_colDone[MAX_VIEW3D_WIDTH] ;

/* Keep a list of indexes into the wall run array, this way we can */
/* have multiple walls on one screen column. */
T_word16 G_intersections[MAX_INTERSECTIONS][MAX_VIEW3D_WIDTH] ;

/* How many wall runs are stored in the above columns. */
T_word16 G_intCount[MAX_VIEW3D_WIDTH] ;

/* Each column must keep track of the greatest and lowest Y that has */
/* been drawn -- and they are stored here. */
T_sword16 G_minY[MAX_VIEW3D_WIDTH] ;
T_sword16 G_maxY[MAX_VIEW3D_WIDTH] ;


/* Keep a double buffer for drawing to until we "splat" it on the screen. */
T_byte8 *P_doubleBuffer ;

/* Color palette index for shading. */
//T_byte8 *P_shadeIndex ;
T_byte8 P_shadeIndex[16384] ;

T_byte8 *G_CurrentTexturePos ;
T_sword32 G_textureStepX ;
T_sword32 G_textureStepY ;

T_word32 G_textureAndX=63, G_textureAndY=63 ;
T_word16 G_textureShift=6 ;

/* Arrays to record current wall information. */
/*
T_sword32 G_zArray[MAX_VIEW3D_WIDTH+10] ;
T_sword32 G_bottomArray[MAX_VIEW3D_WIDTH+10] ;
T_sword32 G_vArray[MAX_VIEW3D_WIDTH+10] ;
T_sword32 G_topuArray[MAX_VIEW3D_WIDTH+10] ;
T_sword32 G_duArray[MAX_VIEW3D_WIDTH+10] ;
*/

/* Also get a quick access to the vga buffer position. */
#define P_vgaBuffer ((T_byte8 *)0xA0000)

/* Internal prototypes: */
T_byte8 View3dRightSideInCone(T_word16 nodeIndex) ;
T_byte8 View3dLeftSideInCone(T_word16 nodeIndex) ;
T_byte8 View3dOnRightByVertices(T_word16 from, T_word16 to) ;
T_byte8 View3dOnRightByVerticesXY(
            T_word16 from,
            T_word16 to,
            T_sword16 x,
            T_sword16 y) ;
T_byte8 View3dOnRightByNode(T_word16 nodeIndex) ;
T_void IDrawColumn(
           T_word16 x,
           T_word16 top,
           T_word16 bottom,
           T_byte8 color) ;
T_void IDrawRow(
           T_word16 y,
           T_word16 left,
           T_word16 right,
           T_byte8 color) ;
T_void IDrawNode(T_word16 nodeIndex) ;
T_void IDrawSSector(T_word16 ssectorIndex) ;
T_void IDrawSegment(T_word16 segmentIndex) ;
T_void IAddWall(
           T_sword16 sx1,
           T_sword16 sx2,
           T_sword16 relativeBottom,
           T_sword16 relativeTop,
           T_sword16 relativeFromZ,
           T_sword16 relativeToZ) ;
T_void IDrawRuns(T_void) ;
T_void IDrawObjectAndWallRuns(T_void) ;

T_word16 IQuickSquareRoot(T_word32 value) ;

T_void IDrawTextureColumn(
           T_word16 x,
           T_3dWallRun *p_run) ;

T_void ISetupDistanceTable(T_void) ;

T_void IFindObjects(T_void) ;
E_Boolean IFindObject(T_3dObject *p_obj) ;

T_void ISortObjects(T_void) ;
T_void IDrawObjectColumn(T_word16 column, T_3dObjectColRun *p_objCol) ;
T_void IAddObjectSlice(
           T_sword32 zDist,
           T_sword16 x,
           T_sword16 clipTop,
           T_sword16 clipBottom) ;
T_word16 IFindSectorNum(T_sword16 x, T_sword16 y) ;
T_word32 IDetermineDistToLine(T_sword16 x, T_sword16 y, T_word16 lineNum) ;
T_byte8 IOnRightOfLine(T_sword16 x, T_sword16 y, T_word16 line) ;
T_sword32 IFindIntersectX(T_word16 line, T_sword16 y) ;
T_void IDrawPixel(T_byte8 *p_pixel, T_byte8 color) ;

E_Boolean ICheckValidRow(T_word16 row, T_word16 run) ;
E_Boolean ICheckValidRow2(T_word16 row, T_word16 run) ;

T_sword16 IObjectPartition(T_sword16 p, T_sword16 r) ;
T_void ISortObjectQuick(T_sword16 p, T_sword16 q) ;

/** BEGIN GRAPHICS-MODE (i.e. not req'd by server) FUNCTION DEFS **/
#ifndef SERVER_ONLY

/*-------------------------------------------------------------------------*
 * Routine:  View3dSetView
 *-------------------------------------------------------------------------*/
/**
 *  View3dSetView moves the current point of view to the new location
 *  and prepares the appropriate calculations.
 *
 *  @param x -- X position to move to.
 *  @param y -- Y position to move to.
 *  @param height -- Height to move to.
 *  @param angle -- Angle to face.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dSetView(
           T_sword16 x,
           T_sword16 y,
           T_sword32 height,
           T_word16 angle)
{
    G_3dPlayerX = x ;
    G_3dPlayerY = y ;
    G_3dPlayerHeight = height ;
    G_3dPlayerAngle = angle ;
    G_3dPlayerLeftAngle = G_3dPlayerAngle + VIEW_3D_LEFT_ANGLE ;
    G_3dPlayerRightAngle = G_3dPlayerAngle - VIEW_3D_LEFT_ANGLE ;
    G_3dSinPlayerAngle = MathSineLookup(-G_3dPlayerAngle) ;
    G_3dCosPlayerAngle = MathCosineLookup(-G_3dPlayerAngle) ;
    G_3dSinPlayerLeftAngle = MathSineLookup(-(G_3dPlayerLeftAngle)) ;
    G_3dCosPlayerLeftAngle = MathCosineLookup(-(G_3dPlayerLeftAngle)) ;
    G_3dSinPlayerRightAngle = MathSineLookup(-(G_3dPlayerRightAngle)) ;
    G_3dCosPlayerRightAngle = MathCosineLookup(-(G_3dPlayerRightAngle)) ;
    G_backdropOffset = (((((T_word32)(-angle))*(VIEW3D_WIDTH<<1))>>15)
                          - (VIEW3D_HALF_WIDTH))
                           % (VIEW3D_WIDTH<<1) ;
    G_fromSector = View3dFindSectorNum(x, y) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dSetHeight
 *-------------------------------------------------------------------------*/
/**
 *  View3dSetHeight sets the current point of view to the new location
 *  and prepares the appropriate calculations.
 *
 *  @param height -- Height to move to.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dSetHeight(T_sword32 height)
{
    G_3dPlayerHeight = height ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetView
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetView returns the position, angle, and height of the
 *  current point of view.
 *
 *  @param p_x -- X position
 *  @param p_y -- Y position
 *  @param p_height -- Height of position
 *  @param p_angle -- Angle being faced
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dGetView(
           T_sword16 *p_x,
           T_sword16 *p_y,
           T_sword32 *p_height,
           T_word16 *p_angle)
{
    *p_x = G_3dPlayerX ;
    *p_y = G_3dPlayerY ;
    *p_height = G_3dPlayerHeight ;
    *p_angle = G_3dPlayerAngle ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dRightSideInCone
 *-------------------------------------------------------------------------*/
/**
 *  View3dRightSideInCone checks to see if the given node (a right
 *  child node) is in the current viewing cone.
 *
 *  @param nodeIndex -- Number of node to check
 *
 *  @return TRUE if on right, FALSE if not
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 View3dRightSideInCone(T_word16 nodeIndex)
{
    /* Determine what quadrant the player is facing.  This is far */
    /* faster than getting exact angles ... but less nodes are */
    /* removed. */
    if (G_3dPlayerLeftAngle < 0x4000)  {
        if (G_3dPNodeArray[nodeIndex]->rx2 > G_3dPlayerX)
            return 1 ;
    } else if (G_3dPlayerLeftAngle < 0x8000)  {
        if (G_3dPNodeArray[nodeIndex]->ry2 > G_3dPlayerY)
            return 1 ;
    } else if (G_3dPlayerLeftAngle < 0xC000)  {
        if (G_3dPNodeArray[nodeIndex]->rx1 < G_3dPlayerX)
            return 1 ;
    } else {
        if (G_3dPNodeArray[nodeIndex]->ry1 < G_3dPlayerY)
            return 1 ;
    }
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dLeftSideInCone
 *-------------------------------------------------------------------------*/
/**
 *  View3dLeftSideInCone checks to see if the given node (a left
 *  child node) is in the current viewing cone.
 *
 *  @param nodeIndex -- Number of node to check
 *
 *  @return TRUE if on right, FALSE if not
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 View3dLeftSideInCone(T_word16 nodeIndex)
{
    /* Determine what quadrant the player is facing.  This is far */
    /* faster than getting exact angles ... but less nodes are */
    /* removed. */
    if (G_3dPlayerLeftAngle < 0x4000)  {
        if (G_3dPNodeArray[nodeIndex]->lx2 > G_3dPlayerX)
            return 1 ;
    } else if (G_3dPlayerLeftAngle < 0x8000)  {
        if (G_3dPNodeArray[nodeIndex]->ly2 > G_3dPlayerY)
            return 1;
    } else if (G_3dPlayerLeftAngle < 0xC000)  {
        if (G_3dPNodeArray[nodeIndex]->lx1 < G_3dPlayerX)
            return 1;
    } else {
        if (G_3dPNodeArray[nodeIndex]->ly1 < G_3dPlayerY)
            return 1;
    }
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dOnRightByVertices
 *-------------------------------------------------------------------------*/
/**
 *  View3dOnRightByVertices checks to see if the player is on the right
 *  of a line segment (given by their vertice indexes) and returns TRUE
 *  if so (otherwise, FALSE).
 *
 *  @param from -- From vertex
 *  @param to -- To vertex
 *
 *  @return TRUE if player on right of segment,
 *      FALSE if not.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 View3dOnRightByVertices(T_word16 from, T_word16 to)
{
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_vertex = &G_3dVertexArray[from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;
    p_vertex = &G_3dVertexArray[to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    if (((x1-x2)*(G_3dPlayerY - y2)) >=
        ((y1-y2) * (G_3dPlayerX - x2)))
        return 1 ;
    return 0 ;
}

T_byte8 View3dOnRightByVerticesXY(
            T_word16 from,
            T_word16 to,
            T_sword16 x,
            T_sword16 y)
{
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_vertex = &G_3dVertexArray[from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;
    p_vertex = &G_3dVertexArray[to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    if (((x1-x2)*(y - y2)) >=
        ((y1-y2) * (x - x2)))
        return 1 ;
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dOnRightByNode
 *-------------------------------------------------------------------------*/
/**
 *  View3dOnRightByNode checks to see if the player is on the right
 *  of a line segment (given by line's node number) and returns TRUE
 *  if so (otherwise, FALSE).
 *
 *  @param nodeIndex -- Node index to check
 *
 *  @return TRUE if player on right of segment,
 *      FALSE if not.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 View3dOnRightByNode(T_word16 nodeIndex)
{
    T_3dNode *p_node ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_node = G_3dPNodeArray[nodeIndex] ;
    x1 = p_node->x ;
    y1 = p_node->y ;
    x2 = x1 + p_node->dx ;
    y2 = y1 + p_node->dy ;

    if (((x1-x2)*(G_3dPlayerY - y2)) >=
        ((y1-y2) * (G_3dPlayerX - x2)))
        return 1 ;
    return 0 ;
}

T_byte8 View3dOnRightByNodeWithXY(
            T_word16 nodeIndex,
            T_sword16 x,
            T_sword16 y)
{
    T_3dNode *p_node ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_node = G_3dPNodeArray[nodeIndex] ;
    x1 = p_node->x ;
    y1 = p_node->y ;
    x2 = x1 + p_node->dx ;
    y2 = y1 + p_node->dy ;

    if (((x1-x2)*(y-y2)) >=
        ((y1-y2) * (x-x2)))
        return 1 ;
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dUpdateSectorLightAnimation
 *-------------------------------------------------------------------------*/
/**
 *  Updates all sector light animations.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dUpdateSectorLightAnimation(T_void)
{
   T_word16 i;
   static T_word32 lastTickCount = 0;
   T_word16 deltaTime;

   DebugRoutine ("View3dUpdateSectorLightAnimation");

   /** How long has it been since the last update? **/
   deltaTime = TickerGet () - lastTickCount;
   lastTickCount = TickerGet ();

   /** Loop through all the sectors. **/
   for (i = 0; i < G_Num3dSectors; i++)
   {
      /** Add the elapsed time to the light animation state. **/
      G_3dSectorInfoArray[i].lightAnimationState +=
         (deltaTime * G_3dSectorInfoArray[i].lightAnimationRate) << 3;

      /** Check the type of sector light animation for this sector. **/
      switch (G_3dSectorInfoArray[i].lightAnimationType)
      {
         case SECTOR_LIGHT_ANIM_NONE:
            /** No animation; no action necessary. **/
            break;

         case SECTOR_LIGHT_ANIM_OSCILLATE:
            /** Oscillate smoothly between the lower and upper bounds. **/
            /** This just involves mapping time to a sine wave. **/
            G_3dSectorArray[i].light =
               G_3dSectorInfoArray[i].lightAnimationCenter +
               MathXTimesSinAngle (G_3dSectorInfoArray[i].lightAnimationRadius,
                            G_3dSectorInfoArray[i].lightAnimationState);
            break;

         case SECTOR_LIGHT_ANIM_SAWTOOTH:
            /** This is pretty simple.  Just keep incrementing. **/
            G_3dSectorArray[i].light =
               G_3dSectorInfoArray[i].lightAnimationState;

            /** Did I exceed my limit? **/
            if (G_3dSectorArray[i].light >
                   G_3dSectorInfoArray[i].lightAnimationCenter +
                   G_3dSectorInfoArray[i].lightAnimationRadius)
            {
               /** Yes.  Set my value to the minimum. **/
               G_3dSectorArray[i].light =
                   G_3dSectorInfoArray[i].lightAnimationCenter -
                   G_3dSectorInfoArray[i].lightAnimationRadius;
            }
            break;

         case SECTOR_LIGHT_ANIM_RANDOM:
            /** Have at least two (scaled) seconds elapsed? **/
            if (G_3dSectorInfoArray[i].lightAnimationState > 140)
            {
               /** Yes.  It's time for a flicker.  Pick a random light **/
               /** value within the limits. **/
               G_3dSectorArray[i].light =
                  G_3dSectorInfoArray[i].lightAnimationCenter +
                   (rand() % G_3dSectorInfoArray[i].lightAnimationRadius) -
                   (rand() % G_3dSectorInfoArray[i].lightAnimationRadius);
            }
            break;

         case SECTOR_LIGHT_ANIM_MIMIC:
            /** Just copy the light value from another sector. **/
            G_3dSectorArray[i].light =
               G_3dSectorArray[
                  G_3dSectorInfoArray[i].lightAnimationCenter].light;
            break;
      }
   }

   DebugEnd ();
}

void PunchOut(void)
{
    unsigned char *p_where;
    int i;

    for (i=0; i<MAX_VIEW3D_HEIGHT; i++)  {
        p_where = G_doublePtrLookup[i];
        memset(p_where, 0x255, MAX_VIEW3D_WIDTH);
    }
}

void IRender(void)
{
    /* Our angle of rotation. */
    //static float angle = 0.0f;
    float px, py, pz;

    /*
     * EXERCISE:
     * Replace this awful mess with vertex
     * arrays and a call to glDrawElements.
     *
     * EXERCISE:
     * After completing the above, change
     * it to use compiled vertex arrays.
     *
     * EXERCISE:
     * Verify my windings are correct here ;).
     */
    static GLfloat v0[] = { -128.0f, -128.0f,  128.0f };
    static GLfloat v1[] = {  128.0f, -128.0f,  128.0f };
    static GLfloat v2[] = {  128.0f,  128.0f,  128.0f };
    static GLfloat v3[] = { -128.0f,  128.0f,  128.0f };
    static GLfloat v4[] = { -128.0f, -128.0f, -128.0f };
    static GLfloat v5[] = {  128.0f, -128.0f, -128.0f };
    static GLfloat v6[] = {  128.0f,  128.0f, -128.0f };
    static GLfloat v7[] = { -128.0f,  128.0f, -128.0f };
    static GLubyte red[]    = { 255,   0,   0, 255 };
    static GLubyte green[]  = {   0, 255,   0, 255 };
    static GLubyte blue[]   = {   0,   0, 255, 255 };
    static GLubyte white[]  = { 255, 255, 255, 255 };
    static GLubyte yellow[] = {   0, 255, 255, 255 };
    static GLubyte black[]  = {   0,   0,   0, 255 };
    static GLubyte orange[] = { 255, 255,   0, 255 };
    static GLubyte purple[] = { 255,   0, 255, 255 };

    /* Clear the color and depth buffers. */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable(GL_CULL_FACE); // testing

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_TEXTURE_2D);

    /* We don't want to modify the projection matrix. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective(90,1.0,1,15120.0);

    glRotatef( 180.0f+(PlayerGetAngle() * -360.0f)/65536.0f, 0.0, 1.0, 0.0 );

    /* Move down the z-axis. */
    px = PlayerGetX()/65536.0f;
    py = PlayerGetY()/65536.0f;
    pz = PlayerGetZ()/65536.0f;
    glTranslatef( -py, -(G_eyeLevel32/65536.0f), -px );
//    glTranslatef( -2416-py, pz, 2208-256-px );
MessagePrintf("%f, %f, %f %d", px, py, pz, PlayerGetAngle());

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    /* Rotate. */
//    if( 1 /*should_rotate*/ ) {
//        if( angle > 360.0f ) {
//            angle = 0.0f;
//        }
//    }
//    glRotatef( angle, 0.0, 1.0, 0.0 );
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dDrawView
 *-------------------------------------------------------------------------*/
/**
 *  View3dDrawView is the main routine to call when you wish to draw
 *  the whole view.  This routine calls all the other routines necessary.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dDrawView(T_void)
{
    T_word16 i ;
//static T_byte8 flippy = 0 ;

    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("View3dDrawView") ;
    INDICATOR_LIGHT(42, INDICATOR_GREEN) ;
//./printf("\n\n------------------------------------------------\n") ;
    /* Initialize and clear all the needed variables. */
    G_colCount = 0 ;
    G_wallCount = 0 ;
    G_nodeCount = 0 ;
    G_segCount = 0 ;
    G_wallAttempt = 0 ;
    G_wallRunCount = 0 ;
    G_firstSSector = 1 ;
    G_objectCount = 0 ;

    /* Compute the height that the eye of the player is looking. */
//    G_eyeLevel = PlayerGetZ16() + StatsGetTallness() ;
G_eyeLevel = G_3dPlayerHeight>>16 ;
//G_eyeLevel32 = PlayerGetZ() + (StatsGetTallness() << 16);
G_eyeLevel32 = G_3dPlayerHeight ;
//G_eyeLevel32 = G_eyeLevel << 16 ;

/* Clear out any floor information. */
//G_numFloorInfo = 1 ;
//memset(G_floorStarts, 0, sizeof(G_floorStarts)) ;

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
    /* Declare that we do not see a texture */
    G_textureSideNum = 0xFFFF ;
#endif

INDICATOR_LIGHT(114, INDICATOR_GREEN) ;
    /* Clear out all the vertical floor information. */
    G_numVertFloor = 1 ;   /* Always start at 1 */
    memset(G_vertFloorStarts, 0, sizeof(G_vertFloorStarts)) ;

    memset(G_colDone, 0, sizeof(G_colDone)) ;
//walls:    memset(G_intCount, 0, sizeof(G_intCount)) ;
    memset(G_minY, 0, sizeof(G_minY)) ;
//floors:    memset(G_runCount, 0, sizeof(G_runCount)) ;
//    memset(G_objectColCount, 0, sizeof(G_objectColCount)) ;
//memset(P_doubleBuffer, flippy^=15, VIEW3D_HEIGHT*MAX_VIEW3D_WIDTH) ;
    memset(G_numWallSlices, 0, sizeof(G_numWallSlices)) ;

    for (i=0; i<MAX_VIEW3D_WIDTH; i++)
        G_maxY[i] = VIEW3D_HEIGHT ;

/*
    for (i=0; i<MAX_VIEW3D_HEIGHT; i++)  {
        G_floorList[i][0].end = 0 ;
        G_floorList[i][0].start = MAX_VIEW3D_WIDTH ;
    }
*/

    if (G_fromSector != 0xFFFF)  {
        INDICATOR_LIGHT(114, INDICATOR_RED) ;
        INDICATOR_LIGHT(118, INDICATOR_GREEN) ;
        /* Find all the possible objects and sort them. */
//        IFindObjects() ;
        INDICATOR_LIGHT(118, INDICATOR_RED) ;

        INDICATOR_LIGHT(122, INDICATOR_GREEN) ;
        /* Compute all the visible walls and floors starting at the root node. */
///        PunchOut();
        GrDrawRectangle(4+0, 3+0, 4+(VIEW3D_CLIP_RIGHT - VIEW3D_CLIP_LEFT)-1, 3+VIEW3D_HEIGHT-1, 255) ;
        IRender();

        IDrawNode(G_3dRootBSPNode) ;

        INDICATOR_LIGHT(122, INDICATOR_RED) ;

        INDICATOR_LIGHT(126, INDICATOR_GREEN) ;
        ///IConvertVertToHorzAndDraw() ;
        INDICATOR_LIGHT(126, INDICATOR_RED) ;

        INDICATOR_LIGHT(130, INDICATOR_GREEN) ;
        ///IDrawObjectAndWallRuns() ;
        INDICATOR_LIGHT(130, INDICATOR_RED) ;
    } else {
        GrDrawRectangle(4+0, 3+0, 4+VIEW3D_WIDTH-1, 3+VIEW3D_HEIGHT-1, 15) ;
    }

    GrScreenSet((T_screen)P_doubleBuffer) ;

    GrInvalidateRect(
        0,
        0,
        VIEW3D_WIDTH+4,
        VIEW3D_HEIGHT+4) ;

    DebugEnd() ;
    INDICATOR_LIGHT(42, INDICATOR_RED) ;
    TICKER_TIME_ROUTINE_ENDM("View3dDrawView", 500) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dDisplayView
 *-------------------------------------------------------------------------*/
/**
 *  View3dDisplayView actually draws/flips to the actual screen to use.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dDisplayView(T_void)
{
    DebugRoutine("View3dDisplayView") ;

#ifndef NDEBUG
/*
    MessagePrintf(
        "attempt: %04d, drawn: %04d, nodes: %04d/%04d, segs: %04d",
        G_wallAttempt,
        G_wallCount,
        G_nodeCount,
        G_Num3dNodes,
        G_segCount) ;
*/
#endif
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawNode
 *-------------------------------------------------------------------------*/
/**
 *  IDrawNode is a self-calling recursive routine that travels through
 *  the BSP tree and determines the order that walls (and floors) should
 *  be drawn.
 *
 *  @param nodeIndex -- Index to head node of sub-BSP tree
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawNode(T_word16 nodeIndex)
{
    T_word16 count ;

#ifdef INDICATOR_LIGHTS
    static T_word16 indicatorLevel = 636 ;

    indicatorLevel+=4 ;
#endif

#ifdef INDICATOR_LIGHTS
    INDICATOR_LIGHT(indicatorLevel, INDICATOR_RED) ;
#endif

    count = VIEW3D_CLIP_RIGHT - VIEW3D_CLIP_LEFT ;
    if (nodeIndex & 0x8000)  {
        /* Found a segment sector, draw that sector. */
#ifdef INDICATOR_LIGHTS
        INDICATOR_LIGHT(indicatorLevel, INDICATOR_BLUE) ;
#endif
        IDrawSSector(nodeIndex & 0x7fff) ;
    } else {
        /* Still traveling nodes.  Keep going until enough columns  */
        /* on the screen are drawn. */
//        if (G_colCount < VIEW3D_WIDTH)  {
#ifndef NDEBUG
    G_nodeCount++ ;
#endif
#ifdef INDICATOR_LIGHTS
        INDICATOR_LIGHT(indicatorLevel, INDICATOR_GREEN) ;
#endif
        if (G_colCount < count)  {
            /* Are we on the right side of this node? */
            if (!View3dOnRightByNode(nodeIndex))  {
                /* Yes, we are.  Draw the left side first, and then */
                /* right (given that they are in the view). */
//                if (View3dLeftSideInCone(nodeIndex))
                    IDrawNode(G_3dPNodeArray[nodeIndex]->left) ;
//                if (View3dRightSideInCone(nodeIndex))
                    IDrawNode(G_3dPNodeArray[nodeIndex]->right) ;
            } else {
                /* No, we are not.  Draw the right side first, and then */
                /* left (given that they are in the view). */
//                if (View3dRightSideInCone(nodeIndex))
                    IDrawNode(G_3dPNodeArray[nodeIndex]->right) ;
//                if (View3dLeftSideInCone(nodeIndex))
                    IDrawNode(G_3dPNodeArray[nodeIndex]->left) ;
            }
        }
    }

#ifdef INDICATOR_LIGHTS
    INDICATOR_LIGHT(indicatorLevel, INDICATOR_GRAY) ;
    indicatorLevel-=4 ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawSSector
 *-------------------------------------------------------------------------*/
/**
 *  IDrawSSector draws all the facing sectors in a segment sector.
 *
 *  @param ssectorIndex -- Index to segment sector
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawSSector(T_word16 ssectorIndex)
{
    T_sword16 lineSide ;
    T_sword16 sector ;
    T_word16 segCount ;
    T_word16 firstSeg ;
    T_3dSegment *p_segment ;
    T_3dSector *p_sector ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;
    T_word32 index ;

    DebugRoutine("IDrawSSector") ;

INDICATOR_LIGHT(148, INDICATOR_GREEN) ;
    /* Get the first segment and the segment count. */
    firstSeg = G_3dSegmentSectorArray[ssectorIndex].firstSeg ;
    segCount = G_3dSegmentSectorArray[ssectorIndex].numSegs ;

    p_segment = &G_3dSegArray[firstSeg] ;
    lineSide = p_segment->lineSide ;

    sector = G_3dSideArray[
                 G_3dLineArray[p_segment->line].side[lineSide]].sector ;

    index = G_fromSector * G_Num3dSectors +
            sector ;
    if (G_3dReject[index>>3] & powers[index&7])  {
        /* Don't draw if can't see this sector. */
        DebugEnd() ;
        return ;
    }

    p_sector = &G_3dSectorArray[sector] ;

    G_3dFloorHeight = p_sector->floorHt ;
    G_3dCeilingHeight = p_sector->ceilingHt ;
    G_wall.shadeIndex = (p_sector->light>>2) ;
    G_wall.textureFloor = *((T_byte8 **)&p_sector->floorTx[1]) ;
    G_wall.textureCeiling = *((T_byte8 **)&p_sector->ceilingTx[1]) ;
//PictureCheck(G_wall.textureFloor) ;
//PictureCheck(G_wall.textureCeiling) ;
INDICATOR_LIGHT(148, INDICATOR_RED) ;
    G_wall.transFlag = p_sector->trigger&1 ;

    /* Draw each of the segments in the sector. */
INDICATOR_LIGHT(164+(segCount<<2), INDICATOR_BLUE) ;
    for (; segCount; segCount--, p_segment++, firstSeg++)  {
INDICATOR_LIGHT(160+(segCount<<2), INDICATOR_GREEN) ;
        if (View3dOnRightByVertices(
                 p_segment->from,
                 p_segment->to))  {
INDICATOR_LIGHT(152, INDICATOR_GREEN) ;
            /* Is the segment in the view? */
            if (IIsSegmentGood(firstSeg))  {
                /* Yes, draw it. */
INDICATOR_LIGHT(156, INDICATOR_GREEN) ;
                IDrawSegment(firstSeg) ;
INDICATOR_LIGHT(156, INDICATOR_RED) ;
            }
INDICATOR_LIGHT(152, INDICATOR_RED) ;
        }
INDICATOR_LIGHT(160+(segCount<<2), INDICATOR_RED) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawSegment
 *-------------------------------------------------------------------------*/
/**
 *  IDrawSegment does all the work to determine if the given segment
 *  can be drawn on the screen and if so, calls the appropriate routines
 *  to add the wall and floor runs.
 *
 *  @param segmentIndex -- Segment to draw
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawSegment(T_word16 segmentIndex)
{
    T_sword16 side ;
    T_3dLine *p_line ;


    INDICATOR_LIGHT(800, INDICATOR_GREEN) ;
G_wallAttempt++ ;
ITestMinMax(1010) ;
//./printf("  IDrawSegment %3d", segmentIndex) ;
    /* Calculate the matrix */
    ICalculateWallMatrix() ;

    /* Now we need to calculate all the screen coordintes for */
    /* this wall segment. */
    side = P_segment->lineSide ;

    p_line = &G_3dLineArray[P_segment->line] ;
    P_sideFront = &G_3dSideArray[p_line->side[side]] ;
    G_wall.offX = P_sideFront->tmXoffset + P_segment->lineOffset ;
    G_wall.offY = P_sideFront->tmYoffset ;

    /* Assume transparent first. */
    G_wall.opaque = 0 ;

    /* Note that we are now drawing a new wall/line. */
    G_newLine = TRUE ;

    G_wall.sector = P_sideFront->sector ;

    G_deltaCeilings = G_deltaFloors = 0 ;

    /* Get a pointer to the other side. */
    P_sideBack = &G_3dSideArray[p_line->side[!side]] ;

    G_wall.sectorBack = P_sideBack->sector ;

#ifndef NDEBUG
G_didDrawWall = FALSE ;
#endif
    /* Check to see if there is a main texture.  If there is, */
    /* there won't be an upper and lower, and we can exit then. */
//    if (P_sideFront->mainTx[0] != '-')  {
        INDICATOR_LIGHT(804, INDICATOR_GREEN) ;
        /* Check if it is single sided. */
        if (!(G_3dLineArray[P_segment->line].flags & LINE_IS_TWO_SIDED))  {
            /* If it is, mark off this section as drawn. */
            INDICATOR_LIGHT(808, INDICATOR_GREEN) ;
            G_wall.opaque = 1 ;
ITestMinMax(1013) ;
            IMarkOffWall() ;
ITestMinMax(1011) ;
            IAddMainWall() ;

            /* Done. */
            INDICATOR_LIGHT(808, INDICATOR_RED) ;
            INDICATOR_LIGHT(800, INDICATOR_RED) ;
            INDICATOR_LIGHT(804, INDICATOR_RED) ;
#ifndef NDEBUG
if (G_didDrawWall)
   G_wallCount++ ;
#endif
            return ;
        } else {
            if (P_sideFront->mainTx[0] != '-')  {
                if (G_3dLineArray[P_segment->line].flags & LINE_IS_TRANSLUCENT)  {
                    G_wall.opaque = 3 ;
                } else {
                    G_wall.opaque = 2 ;
                }

                ITestMinMax(1012) ;
                IAddMainWall() ;
            }
        }
        INDICATOR_LIGHT(804, INDICATOR_RED) ;
//    }

    /* If there is actually two sides to this side, */
    /* then calculate the difference in ceiling and wall heights. */
    if ((P_sideBack->sector != 0xFFFF) &&
        (P_sideFront->sector != 0xFFFF))  {
        G_deltaFloors =
            G_3dSectorArray[P_sideFront->sector].floorHt -
            G_3dSectorArray[P_sideBack->sector].floorHt ;
        G_deltaCeilings =
            G_3dSectorArray[P_sideFront->sector].ceilingHt -
            G_3dSectorArray[P_sideBack->sector].ceilingHt ;
    }
    /* If it doesn't have a main wall, then you can consider */
    /* it as having both an upper and lower wall -- its */
    /* just that they might be flat. */
    INDICATOR_LIGHT(812, INDICATOR_GREEN) ;
    IAddUpperWall() ;
    INDICATOR_LIGHT(812, INDICATOR_RED) ;

    INDICATOR_LIGHT(816, INDICATOR_GREEN) ;
    IAddLowerWall() ;
    INDICATOR_LIGHT(816, INDICATOR_RED) ;

    INDICATOR_LIGHT(800, INDICATOR_RED) ;

#ifndef NDEBUG
if (G_didDrawWall)
   G_wallCount++ ;
#endif
}

/*-------------------------------------------------------------------------*
 * Routine:  IIsSegmentGood
 *-------------------------------------------------------------------------*/
/**
 *  IIsSegmentGood determines if a segment is in the view and if it
 *  will be drawn on the screen.
 *
 *  @param segmentIndex -- Segment to check if need to draw.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean IIsSegmentGood(T_word16 segmentIndex)
{
/*LES2
    T_sword16 worldToX ;
    T_sword16 worldToY ;
    T_sword16 worldFromX ;
    T_sword16 worldFromY ;
*/
    T_sword32 worldToX ;
    T_sword32 worldToY ;
    T_sword32 worldFromX ;
    T_sword32 worldFromY ;
    T_sword32 tanAngle ;
    T_sword16 to ;
    T_sword16 from ;
    T_3dVertex *p_vertex ;
    T_word16 angle ;
    T_sword16 x ;

#ifndef NDEBUG
    G_segCount++ ;
#endif

    /* Get a quick pointer to the segment. */
    P_segment = &G_3dSegArray[segmentIndex] ;
//./printf("  Line #%4d -- side %d  ", P_segment->line, P_segment->lineSide) ;
    /* Determine what vertices this segment travels between. */

    to = P_segment->to ;
    from = P_segment->from ;
//./printf("draw. (%d, %d)\n", to, from) ;

    /* Compute the relative angle to the view. */
    G_wall.angle = angle = P_segment->angle - G_3dPlayerAngle ;

    /* Get the coordinates of the TO vertex. */
    p_vertex = &G_3dVertexArray[to] ;
    worldToX = p_vertex->x ;
    worldToY = p_vertex->y ;

    /* Get the coordinates of the FROM vertex. */
    p_vertex = &G_3dVertexArray[from] ;
    worldFromX = p_vertex->x ;
    worldFromY = p_vertex->y ;

    /* Rotate these world coodinates to get coordinates relative */
    /* to the direction the player is facing. */
    /* Note that we are now translating from world (x,y) coordinates */
    /* to relative (x, z) coordinates */
/*LES2
    G_relativeFromZ = (((worldFromX - G_3dPlayerX)*G_3dCosPlayerAngle) -
                     ((worldFromY - G_3dPlayerY)*G_3dSinPlayerAngle)) >> 16 ;
    G_relativeToZ = (((worldToX - G_3dPlayerX)*G_3dCosPlayerAngle) -
                   ((worldToY - G_3dPlayerY)*G_3dSinPlayerAngle)) >> 16 ;
G_relativeFromZ <<= 16 ;
G_relativeToZ <<= 16 ;
*/

/*
    G_relativeFromZ =
        MultAndShift16(
            ((worldFromX<<16) + 0x8000L - PlayerGetX()),
            G_3dCosPlayerAngle) -
        MultAndShift16(
            ((worldFromY<<16) + 0x8000L - PlayerGetY()),
            G_3dSinPlayerAngle) ;
    G_relativeToX =
        MultAndShift16(
            ((worldToX<<16) + 0x8000L - PlayerGetX()),
            G_3dCosPlayerAngle) -
        MultAndShift16(
            ((worldToY<<16) + 0x8000L - PlayerGetY()),
            G_3dSinPlayerAngle) ;
*/
    G_relativeFromZ =
        MultAndShift16(
            ((worldFromX<<16) + 0x8000L - PlayerGetX()),
            G_3dCosPlayerAngle) -
        MultAndShift16(
            ((worldFromY<<16) + 0x8000L - PlayerGetY()),
            G_3dSinPlayerAngle) ;
    G_relativeToZ =
        MultAndShift16(
            ((worldToX<<16) + 0x8000L - PlayerGetX()),
            G_3dCosPlayerAngle) -
        MultAndShift16(
            ((worldToY<<16) + 0x8000L - PlayerGetY()),
            G_3dSinPlayerAngle) ;

    /* If the segment is behind us, then stop processing. */
    if ((G_relativeFromZ < ZMIN) && (G_relativeToZ < ZMIN))  {
//./printf("behind.\n") ;
//TESTING        return FALSE ;
    }

return TRUE ;

    /* Finish rotating the X coodinates (the above rotated the Z) */
    /* coordinates. */
/*LES2
    G_relativeFromX =
        (((worldFromX - G_3dPlayerX)*G_3dSinPlayerAngle) +
         ((worldFromY - G_3dPlayerY)*G_3dCosPlayerAngle)) >> 16 ;
    G_relativeToX =
        (((worldToX - G_3dPlayerX)*G_3dSinPlayerAngle) +
         ((worldToY - G_3dPlayerY)*G_3dCosPlayerAngle)) >> 16 ;
G_relativeFromX <<= 16 ;
G_relativeToX <<= 16 ;
*/
/*
    G_relativeFromX =
        MultAndShift16(
            ((worldFromX<<16) + 0x8000L - PlayerGetX()),
            G_3dSinPlayerAngle) +
        MultAndShift16(
            ((worldFromY<<16) + 0x8000L - PlayerGetY()),
            G_3dCosPlayerAngle) ;
    G_relativeToX =
        MultAndShift16(
            ((worldToX<<16) + 0x8000L - PlayerGetX()),
            G_3dSinPlayerAngle) +
        MultAndShift16(
            ((worldToY<<16) + 0x8000L - PlayerGetY()),
            G_3dCosPlayerAngle) ;
*/
    G_relativeFromX =
        MultAndShift16(
            ((worldFromX<<16) + 0x8000L - PlayerGetX()),
            G_3dSinPlayerAngle) +
        MultAndShift16(
            ((worldFromY<<16) + 0x8000L - PlayerGetY()),
            G_3dCosPlayerAngle) ;
    G_relativeToX =
        MultAndShift16(
            ((worldToX<<16) + 0x8000L - PlayerGetX()),
            G_3dSinPlayerAngle) +
        MultAndShift16(
            ((worldToY<<16) + 0x8000L - PlayerGetY()),
            G_3dCosPlayerAngle) ;

    /* See if the whole line (both ends) is outside the view to the right. */
    if ((G_relativeFromX > G_relativeFromZ) &&
        (G_relativeToX > G_relativeToZ))  {
//./printf("Too far right.\n") ;
//TESTING        return FALSE ;
    }

    /* See if the line is outside the view to the left. */
    if ((G_relativeFromX < (-G_relativeFromZ)) &&
        (G_relativeToX < (-G_relativeToZ)))  {
//./printf("Too far left.\n") ;
//TESTING        return FALSE ;
    }

    G_wall.fromZ = (G_relativeFromZ>>16) ;

G_relativeFromXOld = G_relativeFromX ;
G_relativeFromZOld = G_relativeFromZ ;

    /* Do Z-clipping. */
    tanAngle = MathTangentLookup(angle) ;

    /* Check to see if it needs to be clipped. */
    if (G_relativeFromX > G_relativeFromZ)  {
        /* Clip along the x = z line. */
        /* check if edge is facing us, then don't draw. */
        if (tanAngle == ((T_sword32)65536))  {
//./printf("edge.\n") ;
//TESTING            return FALSE ;
        }
    }

    /* Clip if z is too close for FROM coordinate. */
    if (G_relativeFromZ < ZMIN)  {
/*LES2
         G_relativeFromX = G_relativeFromX +
                         (((ZMIN - G_relativeFromZ) * tanAngle) >> 16) ;
*/
         G_relativeFromX += MultAndShift16(
                                (ZMIN - G_relativeFromZ),
                                tanAngle) ;
         G_relativeFromZ = ZMIN ;
    }

    /* Clip if z is too close for TO coordinate. */
    if (G_relativeToZ < ZMIN)  {
/*LES2
        G_relativeToX = G_relativeToX +
                      (((ZMIN - G_relativeToZ) * tanAngle) >> 16) ;
*/
        G_relativeToX += MultAndShift16(
                             (ZMIN - G_relativeToZ),
                             tanAngle) ;
        G_relativeToZ = ZMIN ;
    }

    /* Make sure we stay in bounds. */
    if (G_relativeFromZ > ((T_sword32)9999 << 16))
        G_relativeFromZ = ((T_sword32)9999 << 16) ;
    if (G_relativeToZ > ((T_sword32)9999 << 16))
        G_relativeToZ = ((T_sword32)9999 << 16);

    /* Now that all the clipping has been done, we can now compute */
    /* the X screen coordinates. */
/*LES2
    G_screenXLeft = G_xLeft = (T_sword16)(VIEW3D_HALF_WIDTH -
                  ((G_relativeFromX * MathInvDistanceLookup(G_relativeFromZ))
                      >> 16)) ;

    G_screenXRight = G_xRight = (T_sword16)(VIEW3D_HALF_WIDTH -
                  ((G_relativeToX * MathInvDistanceLookup(G_relativeToZ))
                      >> 16)) ;
*/
    G_screenXLeft = G_xLeft = (T_sword16)(VIEW3D_HALF_WIDTH -
                  (MultAndShift16(
                      G_relativeFromX,
                      MathInvDistanceLookup(G_relativeFromZ>>16)) >> 16)) ;
/* TESTING */
if (G_screenXLeft > 0)
  G_screenXLeft-- ;

    G_screenXRight = G_xRight = (T_sword16)(VIEW3D_HALF_WIDTH -
                  (MultAndShift16(
                      G_relativeToX,
                      MathInvDistanceLookup(G_relativeToZ>>16)) >> 16)) ;

    /* Make sure the wall is on the screen, or we'll just quit. */
    /* Nothing off the left. */
    if (G_screenXRight <= VIEW3D_CLIP_LEFT)  {
//./printf("Off left.\n") ;
        return FALSE ;
    }

    /* Nothing off the right. */
    if (G_screenXLeft >= VIEW3D_CLIP_RIGHT)  {
//./printf("Off right.\n") ;
        return FALSE ;
    }

    /* Clip to the screen edges. */
    if (G_xLeft < VIEW3D_CLIP_LEFT)
        G_xLeft = VIEW3D_CLIP_LEFT ;
    if (G_xRight > VIEW3D_CLIP_RIGHT)
        G_xRight = VIEW3D_CLIP_RIGHT ;

    /* Nothing paper thin. */
    if (G_screenXLeft==G_screenXRight)  {
        return FALSE ;
    }

    /* Now we need to see if any part of this wall can be seen. */
    /* If any part can, we will draw it.  If not, there is no */
    /* need to try drawing it. */
DebugCheck(G_xLeft <= VIEW3D_WIDTH) ;
DebugCheck(G_xRight <= VIEW3D_WIDTH) ;
    for (x=G_xLeft; x<G_xRight; x++)
        if (G_colDone[x] == 0)
            break ;

    if (x == G_xRight)  {
//./printf("blocked from view\n") ;
        /* None of the columns were free, quit bothering with */
        /* this segment. */
        return FALSE ;
    }

    return TRUE ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICalculateWallMatrix
 *-------------------------------------------------------------------------*/
/**
 *  ICalculateWallMatrix determines the matrix for a wall segment
 *  given that it has already been determined to be on the screen.
 *  Directly changes the matrix values in G_wall
 *
 *  NOTE: 
 *  Must call IIsSegmentGood any time before this routine.
 *
 *<!-----------------------------------------------------------------------*/
T_void ICalculateWallMatrix(T_void)
{
    T_sword32 Bx, By, Bz ;
    T_sword32 Vx, Vz ;
    T_sword16 angle ;
double dangle ;
double dVx, dVz ;
double calc, calc2 ;

    Bx = G_relativeFromXOld ;
    By = -G_eyeLevel32 ;
    Bz = -G_relativeFromZOld ;

    angle = G_wall.angle ;

//    Vx = MathSineLookup(angle) ;
//    Vx = -Vx ;
//    Vz = MathCosineLookup(angle) ;

/* Get radians from our angle notation */
//dangle = angle & 0xFF00 ;
dangle = angle ;
dangle *= 2 * M_PI ;
dangle /= 65536 ;

dVx = 65536 * sin(-dangle) ;
dVz = 65536 * cos(dangle) ;
Vx = (T_sword32)dVx ;
Vz = (T_sword32)dVz ;

 dVx = Vx ;
 dVz = Vz ;

G_wall.Vx = (T_sword32)dVx ;
G_wall.Vz = (T_sword32)dVz ;

calc = 65536 * cos(dangle) ;
G_wall.a = (T_sword32)calc ;
calc = 65536 * sin(dangle) ;
G_wall.c = (T_sword32)calc ;

calc = -(((double)By) * dVz) ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.d = (T_sword32)calc ;

calc = ((double)Bx) * dVz ;
calc2 = ((double)Bz) * dVx ;
calc -= calc2 ;
calc /= 4194304.0 ;
G_wall.e = (T_sword32)calc ;

calc = ((double)By) * dVx ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.f = (T_sword32)calc ;

/*
calc = -Bz ;
calc /= 1024.0 ;
G_wall.g = calc ;

calc = Bx ;
calc /= 1024.0 ;
G_wall.i = calc ;
*/
G_wall.g = -(Bz >> 10) ;
G_wall.i = Bx >> 10 ;

G_wall.c *= VIEW3D_HALF_WIDTH ;
G_wall.i *= VIEW3D_HALF_WIDTH ;
G_wall.f *= VIEW3D_HALF_WIDTH ;

G_wall.eprime = G_wall.e * (-VIEW3D_HALF_HEIGHT) ;

G_wall.By = By ;

/* ORIG
    G_wall.Vx = Vx ;
    G_wall.Vz = Vz ;

    G_wall.a = MathCosineLookup(angle) ;
    G_wall.c = MathSineLookup(angle) ;

    G_wall.d = -MultAndShift6(By, Vz) ;
    G_wall.e = MultAndShift6(((Bx+0x8000)>>16), Vz) -
                   MultAndShift6(Vx, ((Bz+0x8000)>>16)) ;
    G_wall.f = MultAndShift6(Vx, By) ;

    G_wall.g = -(Bz >> 10) ;
    G_wall.i = Bx >> 10 ;

    G_wall.c *= VIEW3D_HALF_WIDTH ;
    G_wall.i *= VIEW3D_HALF_WIDTH ;
    G_wall.f *= VIEW3D_HALF_WIDTH ;

    G_wall.eprime = G_wall.e * (-VIEW3D_HALF_HEIGHT) ;

    G_wall.By = By ;
*/
}

/*-------------------------------------------------------------------------*
 * Routine:  IMarkOffWall
 *-------------------------------------------------------------------------*/
/**
 *  IMarkOffWall goes through all the columns in a wall and marks off
 *  any empty slots.  This signifies that the column is done and is
 *  no longer going to have anything drawn onto it.
 *
 *<!-----------------------------------------------------------------------*/
T_void IMarkOffWall(T_void)
{
    T_word16 x ;
    T_byte8 *p_column ;

if (G_xLeft > G_xRight)  {
/*
    printf("Backwards on line %d\n", P_segment->line) ;
    fprintf(stderr, "Backwards on line %d\n", P_segment->line) ;
    DebugCheck(FALSE) ;
*/
    return ;
}
DebugCheck(G_xLeft <= G_xRight) ;
DebugCheck(G_xLeft >= 0) ;
DebugCheck(G_xRight >= 0) ;
DebugCheck(G_xLeft <= VIEW3D_WIDTH) ;
DebugCheck(G_xRight <= VIEW3D_WIDTH) ;
    x = G_xLeft ;
DebugCheck(x <= VIEW3D_WIDTH) ;
    p_column = G_colDone+x ;
    x = G_xRight-x ;

    for (; (x!=0); x--, p_column++)  {
ITestMinMax(2000+x+G_xLeft) ;
        if (*p_column == 0)  {
            G_colCount++ ;
            *p_column = 1 ;
        }
    }

/*
DebugCheck(G_xLeft <= G_xRight) ;
DebugCheck(G_xLeft >= 0) ;
DebugCheck(G_xRight >= 0) ;
DebugCheck(G_xLeft <= VIEW3D_WIDTH) ;
DebugCheck(G_xRight <= VIEW3D_WIDTH) ;
    for (x=G_xLeft; x<G_xRight; x++)  {
ITestMinMax(2000+x) ;
        if (G_colDone[x] == 0)  {
            G_colDone[x] = 1 ;
            G_colCount++ ;
        }
    }
*/
}

/*-------------------------------------------------------------------------*
 * Routine:  IAddMainWall
 *-------------------------------------------------------------------------*/
/**
 *  IAddMainWall sets up a main wall to be draw by the 3d engine.
 *
 *<!-----------------------------------------------------------------------*/
T_void IAddMainWall(T_void)
{
    T_sword16 backTop, backBottom ;

    DebugRoutine("IAddMainWall") ;

ITestMinMax(1002) ;
    G_wall.p_texture = *((T_byte8 **)(&P_sideFront->mainTx[1])) ;
//PictureCheck(G_wall.p_texture) ;
    DebugCheck(G_wall.p_texture != NULL) ;

    /* What are the relative position of the heights? */
    /* If we are dealing with a solid wall, we just */
    /* use the floor and ceiling height in front of the wall. */

    if (G_wall.opaque == 1)  {
        G_relativeBottom = G_eyeLevel - G_3dFloorHeight ;
        G_relativeTop = G_eyeLevel - G_3dCeilingHeight ;
    } else {
        /* If we are transparent or translucent, we compare the floor */
        /* and ceiling heights of the front and back sectors. */
        /* We want the section in the middle to be our wall. */
        backTop = G_3dSectorArray[G_wall.sectorBack].ceilingHt ;
        backBottom = G_3dSectorArray[G_wall.sectorBack].floorHt ;

        /* Take the lower of the ceilings. */
        if (backTop > G_3dCeilingHeight)  {
            G_relativeTop = G_eyeLevel - G_3dCeilingHeight ;
        } else {
            G_relativeTop = G_eyeLevel - backTop ;
        }

        /* Take the higher of the floors. */
        if (backBottom < G_3dFloorHeight)  {
            G_relativeBottom = G_eyeLevel - G_3dFloorHeight ;
        } else {
            G_relativeBottom = G_eyeLevel - backBottom ;
        }

        /* One last check to make sure the heights are valid. */
        /* Floors cannot be higher than the ceiling. */
        if (G_relativeBottom <= G_relativeTop)  {
            DebugEnd() ;
            return ;
        }
    }

    /* Note that this is a wall. */
    G_wall.type = WALL_TYPE ;

    /* Add the wall to the drawing buffers (run lists) */
//printf("Seg %4d - Line %4d - Side %d - ", segmentIndex, P_segment->line, P_segment->lineSide) ;
ITestMinMax(1007) ;
    IAddWall(
        G_screenXLeft,
        G_screenXRight,
        G_relativeBottom,
        G_relativeTop,
        (G_relativeFromZ>>16),
        (G_relativeToZ>>16)) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IAddLowerWall
 *-------------------------------------------------------------------------*/
/**
 *  IAddLowerWall sets up a lower wall to be draw by the 3d engine.
 *
 *<!-----------------------------------------------------------------------*/
T_void IAddLowerWall(T_void)
{
double calc, dVx, dVz ;
    DebugRoutine("IAddLowerWall") ;

ITestMinMax(1003) ;
    /* Since we are doing the lower part, the relative bottom is */
    /* equal to our current floor height. */
    G_relativeBottom = G_eyeLevel - G_3dFloorHeight ;

    /* Determine where the top is depending on wheter or not there */
    /* is a texture on the lower part (thus, we have a lower part) */
    /* and thus, do we use the floor height on this side, or on */
    /* the other side. */
#if 0
    if (P_sideFront->lowerTx[0] != '-')  {
#endif
        /* In addition, if there are two lower textures on both */
        /* sides of the wall, only the higher (taller floor) is the one */
        /* that is shown. */
#if 0
        if (P_sideBack->lowerTx[0] != '-')  {
            if (G_3dSectorArray[P_sideBack->sector].floorHt <=
                G_3dSectorArray[P_sideFront->sector].floorHt)  {
                G_relativeTop = G_eyeLevel - G_3dFloorHeight ;
                G_wall.opaque = 0 ;
            } else {
                G_wall.p_texture = *((T_byte8 **)(&P_sideFront->lowerTx[1])) ;
                DebugCheck(G_wall.p_texture != NULL) ;
//PictureCheck(G_wall.p_texture) ;
                G_relativeTop = G_eyeLevel -
                              G_3dSectorArray[P_sideBack->sector].floorHt ;
                G_wall.opaque = 1 ;
            }
        } else {
#endif
            G_wall.p_texture = *((T_byte8 **)(&P_sideFront->lowerTx[1])) ;
//PictureCheck(G_wall.p_texture) ;
            DebugCheck(G_wall.p_texture != NULL) ;
//            G_relativeTop = G_eyeLevel -
//                          G_3dSectorArray[P_sideBack->sector].floorHt ;
//            G_wall.opaque = 1 ;
            if (G_3dSectorArray[P_sideBack->sector].floorHt >
                G_3dSectorArray[P_sideFront->sector].floorHt)  {
                G_relativeTop = G_eyeLevel -
                              G_3dSectorArray[P_sideBack->sector].floorHt ;
                G_wall.opaque = 1 ;
            } else {
                G_relativeTop = G_eyeLevel -
                              G_3dSectorArray[P_sideFront->sector].floorHt ;
                G_wall.opaque = 0 ;
            }
#if 0
        }
#endif

        G_wall.By = -G_relativeTop ;
        G_wall.By <<= 16 ;

        if (G_eyeLevel32 > 0)
            G_wall.By -= (G_eyeLevel32 & 0xFFFF) ;
        else
            G_wall.By += ((-G_eyeLevel32) & 0xFFFF) ;

dVx = G_wall.Vx ;
dVz = G_wall.Vz ;
calc = -(((double)G_wall.By) * dVz) ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.d = (T_sword32)calc ;
//        G_wall.d = -(G_wall.By*G_wall.Vz) ;
//        G_wall.d = -MultAndShift6(G_wall.By, G_wall.Vz) ;
calc = ((double)G_wall.By) * dVx ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.f = (T_sword32)calc ;
G_wall.f *= VIEW3D_HALF_WIDTH ;
//        G_wall.f = (G_wall.Vx*G_wall.By) ;
//        G_wall.f = MultAndShift6(G_wall.Vx, G_wall.By) ;
//        G_wall.f *= VIEW3D_HALF_WIDTH ;

        /* Fix the top edge if it is above the ceiling on this side. */
        if (G_relativeTop < G_eyeLevel - G_3dSectorArray[P_sideFront->sector].ceilingHt)
             G_relativeTop = G_eyeLevel - G_3dSectorArray[P_sideFront->sector].ceilingHt ;
#if 0
    }  else  {
//        G_relativeTop = G_eyeLevel - G_3dFloorHeight ;
        G_relativeTop = G_relativeBottom ;
        G_wall.opaque = 0 ;
        G_wall.p_texture = NULL ;
    }
#endif

//printf("Seg %4d - Line %4d - Side %d - ", segmentIndex, P_segment->line, P_segment->lineSide) ;
    /* Add this wall section. */
    G_wall.type = LOWER_TYPE ;
ITestMinMax(1006) ;
    IAddWall(
        G_screenXLeft,
        G_screenXRight,
        G_relativeBottom,
        G_relativeTop,
        (G_relativeFromZ>>16),
        (G_relativeToZ>>16)) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IAddUpperWall
 *-------------------------------------------------------------------------*/
/**
 *  IAddUpperWall sets up a upper wall to be draw by the 3d engine.
 *
 *<!-----------------------------------------------------------------------*/
T_void IAddUpperWall(T_void)
{
double calc, dVx, dVz ;
    DebugRoutine("IAddUpperWall") ;

ITestMinMax(1004) ;
    G_wall.type = UPPER_TYPE ;
    G_relativeTop = G_eyeLevel - G_3dCeilingHeight ;

#if 1
    if (P_sideFront->upperTx[0] != '-')  {
#endif
        /* In addition, if there are two lower textures on both */
        /* sides of the wall, only the higher (taller floor) is the one */
        /* that is shown. */
#if 0
        if (P_sideBack->upperTx[0] != '-')  {
            if (G_3dSectorArray[P_sideBack->sector].ceilingHt <=
                G_3dSectorArray[P_sideFront->sector].ceilingHt)  {
                G_relativeBottom = G_eyeLevel - G_3dCeilingHeight ;
                G_wall.opaque = 0 ;
            } else {
                G_wall.p_texture = *((T_byte8 **)(&P_sideFront->upperTx[1])) ;
//PictureCheck(G_wall.p_texture) ;
                DebugCheck(G_wall.p_texture != NULL) ;
                G_relativeBottom = G_eyeLevel -
                              G_3dSectorArray[P_sideBack->sector].ceilingHt ;
                G_wall.opaque = 1 ;
            }
        } else {
#endif
            G_wall.p_texture = *((T_byte8 **)(&P_sideFront->upperTx[1])) ;
//PictureCheck(G_wall.p_texture) ;
            DebugCheck(G_wall.p_texture != NULL) ;
//            G_relativeBottom = G_eyeLevel -
//                          G_3dSectorArray[P_sideBack->sector].ceilingHt ;
//            G_wall.opaque = 1 ;
            if (G_3dSectorArray[P_sideBack->sector].ceilingHt <
                G_3dSectorArray[P_sideFront->sector].ceilingHt)  {
                G_wall.opaque = 1 ;
                G_relativeBottom = G_eyeLevel -
                              G_3dSectorArray[P_sideBack->sector].ceilingHt ;
            } else {
                G_wall.opaque = 0 ;
                G_relativeBottom = G_eyeLevel -
                              G_3dSectorArray[P_sideFront->sector].ceilingHt ;
            }
#if 0
        }
#endif

//        G_wall.By = -G_relativeBottom ;
//        G_wall.By <<= 16 ;
/*
        if (G_eyeLevel32 > 0)
            G_wall.By += (G_eyeLevel32 & 0xFFFF) ;
        else
            G_wall.By -= ((-G_eyeLevel32) & 0xFFFF) ;
*/
//LES1        G_wall.d = -(G_wall.By*G_wall.Vz) ;
//        G_wall.d = -MultAndShift6(G_wall.By, G_wall.Vz) ;
//LES1        G_wall.f = (G_wall.Vx*G_wall.By) ;
//        G_wall.f = MultAndShift6(G_wall.Vx, G_wall.By) ;
//        G_wall.f *= VIEW3D_HALF_WIDTH ;

        G_wall.By = -G_relativeBottom ;
        G_wall.By <<= 16 ;

        if (G_eyeLevel32 > 0)
            G_wall.By -= (G_eyeLevel32 & 0xFFFF) ;
        else
            G_wall.By += ((-G_eyeLevel32) & 0xFFFF) ;

dVx = G_wall.Vx ;
dVz = G_wall.Vz ;
calc = -(((double)G_wall.By) * dVz) ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.d = (T_sword32)calc ;
//        G_wall.d = -(G_wall.By*G_wall.Vz) ;
//        G_wall.d = -MultAndShift6(G_wall.By, G_wall.Vz) ;
calc = ((double)G_wall.By) * dVx ;
calc /= 64.0 ;
calc /= 65536.0 ;
G_wall.f = (T_sword32)calc ;
G_wall.f *= VIEW3D_HALF_WIDTH ;
        /* Fix the bottom edge if it is behind the floor on this side. */
        if (G_relativeBottom > G_eyeLevel - G_3dSectorArray[P_sideFront->sector].floorHt)
             G_relativeBottom = G_eyeLevel - G_3dSectorArray[P_sideFront->sector].floorHt ;

#if 1
    }  else  {
        G_relativeBottom = G_relativeTop ;
        G_wall.opaque = 0 ;
        G_wall.p_texture = NULL ;
    }
#endif

ITestMinMax(1005) ;
//printf("Seg %4d - Line %4d - Side %d - ", segmentIndex, P_segment->line, P_segment->lineSide) ;
    IAddWall(
        G_screenXLeft,
        G_screenXRight,
        G_relativeBottom,
        G_relativeTop,
        (G_relativeFromZ>>16),
        (G_relativeToZ>>16)) ;

//    if (G_3dSectorArray[P_sideBack->sector].floorHt >=
//        G_3dSectorArray[P_sideBack->sector].ceilingHt)  {
            /* If it is, mark off this section as drawn. */
//            G_wall.opaque = 1 ;
//            IMarkOffWall() ;
//    }
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IAddWall
 *-------------------------------------------------------------------------*/
/**
 *  IAddWall is called once a wall has been found to be on the screen,
 *  has been clipped, and the full coordinates have been located.  This
 *  routine "draws" the wall into the run lists.
 *
 *  @param sx1 -- Screen X1 coordinate
 *  @param sx2 -- Screen X2 coordinate
 *  @param relativeBottom -- Height of the wall at base
 *  @param relativeTop -- Height of the wall at top
 *  @param relativeFromZ -- Distance to first coordinate along Z
 *  @param relativeToZ -- Distance to second coordinate along Z
 *
 *<!-----------------------------------------------------------------------*/
T_void IAddWall(
           T_sword16 sx1,
           T_sword16 sx2,
           T_sword16 relativeBottom,
           T_sword16 relativeTop,
           T_sword16 relativeFromZ,
           T_sword16 relativeToZ)
{
    /* Texture variables. */
//    T_sword32 topu, topv, bottomCalc ;
//    T_sword32 u, v ;
//    T_sword32 du;
//    T_sword32 y1, y2, dy1, dy2 ;
//    T_sword32 scrYBottomLeft ;  // 1
//    T_sword32 scrYBottomRight ; // 2
//    T_sword32 scrYTopRight ;    // 3
//    T_sword32 scrYTopLeft ;     // 4
//    T_sword32 interZ ;   /* Intersection of view line to wall. */
//    T_sword32 cosineAngle ;
//    T_sword32 invDFromZ ;
//    T_sword32 invDToZ ;
//    T_sword16 top ;
//    T_sword16 bottom ;
//    T_sword16 minY ;
//    T_sword16 maxY ;
//    T_sword16 x ;
//    T_sword32 halfOffX ;
    T_sword16 absoluteTop ;
    T_sword16 absoluteBottom ;
//    T_word16 sizeX, sizeY ;
//    T_sword16 dx ;
//    T_sword16 leftEdge ;
//    T_byte8 shift ;
//    T_byte8 shiftOrig ;
//    T_word16 sizeXX, sizeYY ;
//    T_byte8 mipShift ;
//    T_byte8 mipLevel ;
    T_3dVertex *p_from;
    T_3dVertex *p_to;
    static GLubyte blue[]   = {   0,   0, 255, 255 };
    static GLubyte white[]   = {   255,   255, 255, 255 };
    static GLubyte gray[]   = {   128,   128, 128, 255 };
    static GLfloat v0[] = { -128.0f, -128.0f,  128.0f };
    static GLfloat v1[] = {  128.0f, -128.0f,  128.0f };
    static GLfloat v2[] = {  128.0f,  128.0f,  128.0f };
    static GLfloat v3[] = { -128.0f,  128.0f,  128.0f };

//T_sword32 testa, testb, testc, testd, teste ;

//T_sword32 relBot32 ;
//T_sword32 relTop32 ;

INDICATOR_LIGHT(821, INDICATOR_GREEN) ;

if (G_wall.p_texture)  {
  DebugRoutine("IAddWall") ;
//  PictureCheck(G_wall.p_texture) ;
  DebugEnd() ;
}

{
    float tw, th;
    float dx, dy;
    absoluteTop = G_eyeLevel - relativeTop ;
    absoluteBottom = G_eyeLevel - relativeBottom ;
    p_from = G_3dVertexArray+P_segment->from;
    p_to = G_3dVertexArray+P_segment->to;
    glEnable (GL_DEPTH_TEST);
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, G_texture);

#if 1
    glBegin( GL_QUADS );
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    dx = (float)p_from->x - (float)p_to->x;
    dy = (float)p_from->y - (float)p_to->y;
    tw = (float)sqrt(dx*dx + dy*dy)/32.0f;
    th = (float)fabs(absoluteBottom - absoluteTop)/32.0f;
    //glColor4ubv( blue );
    glTexCoord2f (0.0f, th);
    glVertex3i( p_from->y, absoluteBottom, p_from->x);
    glTexCoord2f (tw, th);
    glVertex3i( p_to->y, absoluteBottom, p_to->x);
    glTexCoord2f (tw, 0.0f);
    glVertex3i( p_to->y, absoluteTop, p_to->x);
    glTexCoord2f (0.0f, 0.0f);
    glVertex3i( p_from->y, absoluteTop, p_from->x);
    glEnd();
    //glColor4ubv( white );
//printf("Seg %d: (%d,%d,%d) -> (%d,%d,%d)\n", P_segment-G_3dSegArray, p_from->x, absoluteBottom, p_from->y, p_to->x, absoluteTop, p_to->y);
#endif

}

#if 0
//./printf("    IAddWall\n") ;
    /* LES:  Calculate the real world height values that we will be */
    /* storing in the wall runs.  These are pretty much constants */
    /* throughout this routine. */
    absoluteTop = G_eyeLevel - relativeTop ;
    absoluteBottom = G_eyeLevel - relativeBottom ;

relBot32 = (relativeBottom<<16) + (G_eyeLevel32 & 0xFFFF) ;
relTop32 = (relativeTop<<16) + (G_eyeLevel32 & 0xFFFF) ;

INDICATOR_LIGHT(825, INDICATOR_GREEN) ;
    /* Project the coordinates to determine the four y screen coordinates. */
    invDFromZ = MathInvDistanceLookup(relativeFromZ) ;
    invDToZ = MathInvDistanceLookup(relativeToZ) ;
//    scrYBottomLeft = VIEW3D_HALF_HEIGHT + ((relativeBottom*invDFromZ)>>16) ;
//    scrYBottomRight = VIEW3D_HALF_HEIGHT + ((relativeBottom*invDToZ)>>16) ;
scrYBottomLeft = (VIEW3D_HALF_HEIGHT<<16) + MultAndShift16(relBot32, invDFromZ) ;
scrYBottomRight = (VIEW3D_HALF_HEIGHT<<16) + MultAndShift16(relBot32, invDToZ) ;
INDICATOR_LIGHT(825, INDICATOR_RED) ;
    /* Don't do any wall pieces that are above the screen. */
    if ((scrYBottomLeft < 0) && (scrYBottomRight < 0))  {
        INDICATOR_LIGHT(821, INDICATOR_RED) ;
        return ;
    }

INDICATOR_LIGHT(829, INDICATOR_GREEN) ;
//    scrYTopLeft = VIEW3D_HALF_HEIGHT + ((relativeTop*invDFromZ)>>16) ;
//    scrYTopRight = VIEW3D_HALF_HEIGHT + ((relativeTop*invDToZ)>>16) ;
scrYTopLeft = (VIEW3D_HALF_HEIGHT<<16) + MultAndShift16(relTop32, invDFromZ) ;
scrYTopRight = (VIEW3D_HALF_HEIGHT<<16) + MultAndShift16(relTop32, invDToZ) ;
INDICATOR_LIGHT(829, INDICATOR_RED) ;

    /* Don't do any wall pieces that are below the screen. */
    if ((scrYTopLeft >= (VIEW3D_HEIGHT<<16)) &&
             (scrYTopRight >= (VIEW3D_HEIGHT<<16)))  {
        INDICATOR_LIGHT(821, INDICATOR_RED) ;
        return ;
    }

    /* Compute the step rates for "drawing a pair of lines" between the */
    /* top and bottom edges. */
    dx = sx2 - sx1 ;
//    y1 = scrYTopLeft << 16 ;
//    y2 = scrYBottomLeft << 16 ;
y1 = scrYTopLeft ;
y2 = scrYBottomLeft ;

INDICATOR_LIGHT(833, INDICATOR_GREEN) ;
//    dy1 = Div32by32To1616Asm(scrYTopRight - scrYTopLeft, dx) ;
//    dy2 = Div32by32To1616Asm(scrYBottomRight - scrYBottomLeft, dx) ;
dy1 = Div32by32To1616Asm(((scrYTopRight - scrYTopLeft)>>16), dx) ;
dy2 = Div32by32To1616Asm(((scrYBottomRight - scrYBottomLeft)>>16), dx) ;
INDICATOR_LIGHT(833, INDICATOR_RED) ;
INDICATOR_LIGHT(837, INDICATOR_GREEN) ;
    /* If we are off the egde, skip enough to get the right starting */
    /* location. */
    if (sx1 < VIEW3D_CLIP_LEFT)  {
        leftEdge = VIEW3D_CLIP_LEFT - sx1 ;
        y1 += leftEdge * dy1 ;
        y2 += leftEdge * dy2 ;

        sx1 = VIEW3D_CLIP_LEFT ;
    }

    /* Stop at the right. */
    if (sx2 > VIEW3D_CLIP_RIGHT)
        sx2 = VIEW3D_CLIP_RIGHT ;

    /* Set up the size for this texture. */
    if (G_wall.p_texture != NULL)  {
        PictureGetXYSize(G_wall.p_texture, &sizeY, &sizeX) ;
        shiftOrig = MathPower2Lookup(sizeY) ;
        sizeX-- ;
        sizeY-- ;
    } else  {
        sizeX = sizeY = 1 ;
    }
    sizeXX = sizeX ;
    sizeYY = sizeY ;
INDICATOR_LIGHT(837, INDICATOR_RED) ;

    /* Small speed up for calculations. */
    cosineAngle = MathCosineLookup(G_wall.angle) ;

    /* Go through each column and draw the appropriate column. */
    halfOffX = sx1 - VIEW3D_HALF_WIDTH ;

//G_newLine = TRUE ;
    /* Start up some calculations. */
//    if (G_newLine)  {
    bottomCalc = G_wall.a*halfOffX + G_wall.c ;
    topv = G_wall.g*halfOffX + G_wall.i ;

    topu = G_wall.d * halfOffX + G_wall.eprime + G_wall.f ;

//    topu = ((G_wall.d * halfOffX)>>1) + (G_wall.eprime>>1) + (G_wall.f>>1) ;

//    }

/* TESTING */
ITestMinMax(1000) ;

    for (x=sx1;
         x<sx2;
         x++, halfOffX++, y1 += dy1, y2 += dy2)  {
        sizeX = sizeXX ;
        sizeY = sizeYY ;
        shift = shiftOrig ;
INDICATOR_LIGHT(841, INDICATOR_GREEN) ;
/* TESTING */
//ITestMinMax(x) ;
/*
        if (G_newLine)  {
*/
//            bottomCalc = G_wall.a*halfOffX + G_wall.c ;
//            topv = G_wall.g*halfOffX + G_wall.i ;

        if (bottomCalc>>4)  {
            /* Compute the v, the texture column. */
//            v = Div32by32To1616Asm(topv, (bottomCalc>>4)) ;

            testa = topv ;
            testb = (bottomCalc>>4) ;
            testc = testa / testb ;
            testd = testa % testb ;
            teste = Div32by32To1616Asm(testd, testb) ;
            v = ((testc<<16)+teste) ;

            v >>= 10 ;
        } else {
            u = v = 0 ;
        }

INDICATOR_LIGHT(845, INDICATOR_GREEN) ;
        if (G_wall.opaque)  {
            if (bottomCalc>>4)  {
//                    topu = G_wall.d*halfOffX + G_wall.eprime + G_wall.f ;
//                    G_topuArray[x] = topu ;

/* Divide by zero error keeps occuring here!!!!!!!!!!! */
//                u = Div32by32To1616Asm(topu, (bottomCalc>>4)) << 2;

// Chopped up
//                  u = (((topu / (bottomCalc>>4)) << 2) << 16) ;

// Distorted lines, but uniform
//                  u = ((topu / bottomCalc)<<22) ;

// Chopped up (same as above)
//                  u = ((topu / (bottomCalc>>4)) << 18) ;

// Chopped up alot, but pits don't work
//                  u = (((topu<<4) / (bottomCalc>>4)) << 14) ;
                testa = (topu<<2) ;
                testb = (bottomCalc>>4) ;
                testc = testa / testb ;
                testd = testa % testb ;
                teste = Div32by32To1616Asm(testd, testb) ;
                u = ((testc<<16)+teste) ;
/*
printf("a: %08X  b: %08X  c: %08X  d: %08X  e: %08X  u: %08X\n",
    testa,
    testb,
    testc,
    testd,
    teste,
    u) ;
*/
//* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

//                u = Div32by32To1616Asm(topu, (bottomCalc>>4)) << 3;

/* DOES VERY LITTLE
                u = Div32by32To1616Asm(topu>>8, (bottomCalc>>4)) << 10;
*/

//                u = Div32by32To1616Asm(topu, bottomCalc) ;
//                u = Div32by32To1616Asm((topu<<2), (bottomCalc>>4)) ;
                du = Div32by32To1616Asm((G_wall.e)<<2, (bottomCalc>>4)) ;
//                du = Div32by32To1616Asm(G_wall.e, bottomCalc) << 6;
/*
printf("e: %08X  b: %08X  du: %08X\n",
    (G_wall.e<<2),
    (bottomCalc>>4),
    du) ;
*/
            } else {
                u = v = 0 ;
                du = 0 ;
            }
        }
INDICATOR_LIGHT(845, INDICATOR_RED) ;
INDICATOR_LIGHT(849, INDICATOR_GREEN) ;
//            G_duArray[x] = du ;
//            G_bottomArray[x] = bottomCalc ;
//            G_vArray[x] = v ;
//            G_zArray[x] =
              interZ =
                ((cosineAngle*(-v))>>16) + G_wall.fromZ ;

            /* Update the step rate. */
/*
            topu += stepU ;
            topv += stepV ;
            bottomCalc += stepB ;
*/
        topu += G_wall.d ;
        bottomCalc += G_wall.a ;
        topv += G_wall.g ;
/*
        } else {
            interZ = G_zArray[x] ;
            v = G_vArray[x] ;
            if (G_wall.opaque)  {
                bottomCalc = G_bottomArray[x] ;
                if (bottomCalc)  {
                    topu = G_topuArray[x] ;
//                    topu = G_wall.d*halfOffX + G_wall.eprime + G_wall.f ;
                    u = Div32by32To1616Asm(topu, bottomCalc) << 2;
                    du = Div32by32To1616Asm((G_wall.e)<<2, bottomCalc) ;
                } else {
                    du = u = 0 ;
                }
            }
        }
*/


        /* BTW, what are the min and max already drawn for this column. */
DebugCheck(x < VIEW3D_WIDTH) ;
        minY = G_minY[x] ;
DebugCheck(minY <= VIEW3D_HEIGHT) ;
        maxY = G_maxY[x] ;
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
        top = (T_sword16)((y1 + 0x8000) >> 16) ;
        bottom = (T_sword16)((y2 + 0x8000) >> 16) ;

        /* Check to see if there are any objects at this slice. */
        if ((G_newLine) && (interZ >= G_screenObjectDistance[x]))  {
            /* Yes, there might be.  Call the routine to add an object */
            /* slice.  Also pass the clipping parameters. */
            IAddObjectSlice(interZ, x, minY, maxY) ;
        }
        INDICATOR_LIGHT(849, INDICATOR_RED) ;

        INDICATOR_LIGHT(853, INDICATOR_GREEN) ;

        G_wall.p_texture2 = G_wall.p_texture ;
#ifdef MIP_MAPPING_ON
//SyncMemAdd("x: %d  sizeX: %d  sizeY: %d", x, sizeX, sizeY) ;
//SyncMemAdd("  shift: %d -> ", shift, 0, 0) ;
        mipShift = 0 ;
        if (sizeY != 255)  {
            for (mipLevel=0; mipLevel<4; mipLevel++)  {
                if ((sizeX <= 2) || (sizeY <= 2))
                    break ;

                if ((du <= -0x1C000)||(du >= 0x1C000))  {
                    du >>= 1 ;
                    u >>= 1 ;
                    mipShift++;
                    sizeX++ ;
                    sizeY++ ;
                    G_wall.p_texture2 += 4 + (sizeX * sizeY) ;
                    sizeX-- ;
                    sizeY-- ;
                    sizeX >>= 1 ;
                    sizeY >>= 1 ;
                    shift-- ;
                } else {
                    break ;
                }
            }
        }
//SyncMemAdd("du: %08X  sizeX: %d  sizeY: %d", du, sizeX, sizeY) ;
//SyncMemAdd("  shift: %d\n", shift, 0, 0) ;
#endif
        /* Time to do the wall now. */
//        if ((G_wall.opaque) && (minY < maxY))  {
        if (G_wall.opaque)  {
            if ((top < maxY) && (bottom > minY))  {
                /* Go to the points that make up the floor, not the wall. */

                if (top < minY)
                    top = minY ;
                if (bottom > maxY)
                    bottom = maxY ;

                if (top < bottom)  {
#if 0

                    /* Store the wall run information for later. */
                    G_wallRuns[G_wallRunCount].top = top ;
                    G_wallRuns[G_wallRunCount].bottom = bottom ;

                    /* LES:  Store the real world heights of the wall */
                    /* so we can correctly map the texture. */
                    G_wallRuns[G_wallRunCount].absoluteTop =
                        absoluteTop ;

                    G_wallRuns[G_wallRunCount].absoluteBottom =
                        absoluteBottom ;

                    /* LES:  Also note where the top and bottom */
                    /* pixel would have been if not clipped. */

                    G_wallRuns[G_wallRunCount].realPixelTop =
                        y1 >> 16 ;
                    G_wallRuns[G_wallRunCount].realPixelBottom =
                        y2 >> 16 ;

                    /* Store the distance to the wall segment. */
                    G_wallRuns[G_wallRunCount].distance = interZ ;

                    /* LES:  Store where along the texture we are located. */
                    G_wallRuns[G_wallRunCount].textureColumn =
                        G_wall.offX - v ;

                    G_wallRuns[G_wallRunCount].p_texture = G_wall.p_texture ;
                    G_wallRuns[G_wallRunCount].maskX = sizeX ;
                    G_wallRuns[G_wallRunCount].maskY = sizeY ;
                    G_wallRuns[G_wallRunCount].shift = shift ;
                    G_wallRuns[G_wallRunCount].offY = G_wall.offY ;
                    G_wallRuns[G_wallRunCount].u = -(u+0x8000) ;
                    G_wallRuns[G_wallRunCount].du = -du ;

                    G_wallRuns[G_wallRunCount].shadeIndex =
                        G_wall.shadeIndex ;

                    G_intersections[G_intCount[x]][x] = G_wallRunCount ;
                    G_intCount[x]++ ;
#endif
                    G_textureAndY = sizeY ;
                    G_CurrentTexturePos =
                        G_wall.p_texture2 +
                            ((((G_wall.offX - v)>>mipShift)&(sizeX))<<(shift)) ;
#if 0
#ifndef NDEBUG
//PictureCheck(G_wall.p_texture) ;
offsetCheck = ((((G_wall.offX - v)>>mipShift)&(sizeX))<<(shift)) ;
maxOffset = (PictureGetWidth(G_wall.p_texture2) * PictureGetHeight(G_wall.p_texture2)) ;
if (offsetCheck >= maxOffset)  {
    printf("offsetCheck = %d\nmaxOffset = %d\n", offsetCheck, maxOffset) ;
    printf("Texture name: '%s'\n", PictureGetName(G_wall.p_texture2)) ;
    fflush(stdout) ;
}
DebugCheck(offsetCheck < maxOffset) ;
#endif
#endif

G_3dLineArray[P_segment->line].flags |= 0x8000 ;
#ifndef NDEBUG
G_didDrawWall = TRUE ;
#endif
#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
                    if ((G_mouseAtX == x) &&
                        (G_mouseAtY >= top) &&
                        (G_mouseAtY <= bottom))  {
                        if (G_textureSideNum == 0xFFFF)  {
                            G_textureSideNum =
                                G_3dLineArray[P_segment->line].side[
                                    P_segment->lineSide] ;
                        }
                    }
#endif
                    if (G_wall.opaque == 1)  {
                        IDrawTextureColumnNew(
                            /* Shade pointer */
                            IDetermineShade(interZ, G_wall.shadeIndex),
                            /* number pixels. */
                            bottom - top,
                            /* texture step. */
                            -du,
                            /* texture offset. */
                            (-du * top) + (-(u+0x8000)) +
                                (((T_sword32)(G_wall.offY>>mipShift))<<16),
                            /* First pixel location. */
                            G_doublePtrLookup[top]+x,
                            /* Shift value of texture. */
                            shift) ;
                    } else {
                        IDrawTextureColumnLater(
                            x,
                            /* Shade pointer */
                            IDetermineShade(interZ, G_wall.shadeIndex),
                            /* number pixels. */
                            bottom - top,
                            /* texture step. */
                            -du,
                            /* texture offset. */
                            (-du * top) + (-(u+0x8000)) +
                                (((T_sword32)(G_wall.offY>>mipShift))<<16),
                            /* First pixel location. */
                            G_doublePtrLookup[top]+x,
                            /* Shift value of texture. */
                            shift,
                            /* transparent (2) or translucent (3) */
                            G_wall.opaque,
                            /* distance to slice */
                            interZ,
                            /* where in the texture. */
                            G_CurrentTexturePos) ;
                    }
                }
            }
        }

        if (top < 0)
            top = 0 ;
        if (top > VIEW3D_HEIGHT)
            top = VIEW3D_HEIGHT ;
        if (bottom > VIEW3D_HEIGHT)
            bottom = VIEW3D_HEIGHT ;
        if (bottom < 0)
            bottom = 0 ;

INDICATOR_LIGHT(853, INDICATOR_RED) ;
INDICATOR_LIGHT(857, INDICATOR_GREEN) ;
        /* Update the edges and color for the upper type wall. */
        switch(G_wall.type)  {
            case UPPER_TYPE:
                /* If the upper wall is opaque (and thus faces us), */
                /* we should run floor above that top edge. */
                if (G_wall.opaque)  {
                    if (minY < top)  {
DebugCheck(minY <= VIEW3D_HEIGHT) ;
DebugCheck(top <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, minY, top, G_wall.sector) ;
                    }
                }

                /* If the upper wall is transparent (and thus faces */
                /* away), we should run floor above the bottom edge. */
                if (!G_wall.opaque)  {
                    if (minY < bottom)  {
DebugCheck(minY <= VIEW3D_HEIGHT) ;
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, minY, bottom, G_wall.sector) ;
                    }
                }

                if (bottom > minY)  {
DebugCheck(x < VIEW3D_WIDTH) ;
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
                    G_minY[x] = minY = bottom ;
DebugCheck(G_minY[x] <= VIEW3D_HEIGHT) ;

//                    if (bottom >= VIEW3D_HEIGHT-1)  {
                     if (bottom >= G_maxY[x])  {
DebugCheck(x < VIEW3D_WIDTH) ;
                        if (!G_colDone[x])  {
                            G_colCount++ ;
                            G_colDone[x] = 1 ;
                        }
                    }
                }

                break ;
            case LOWER_TYPE:
                /* If the lower wall is opaque (thus facing us), */
                /* we should put a vertical floor run below the base. */
                if (G_wall.opaque)  {
                    if (bottom < maxY)  {
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, bottom, maxY, G_wall.sector) ;
                    }
                }

                /* If the lower wall is transparent (facing away), */
                /* we should run the floor down from the top. */
                /* This caps off the top of ledges. */
                if (!G_wall.opaque)  {
                    if (top < maxY)  {
DebugCheck(top <= VIEW3D_HEIGHT) ;
#ifndef NDEBUG
if (maxY > VIEW3D_HEIGHT)  {
    printf("maxY = %d, line=%d\n", maxY, P_segment->line) ;
    fprintf(stderr, "maxY = %d, line=%d\n", maxY, P_segment->line) ;
    fflush(stdout) ;
}
#endif
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, top, maxY, G_wall.sector) ;
                    }
                }

                if (top < maxY)  {
DebugCheck(top <= VIEW3D_HEIGHT) ;
DebugCheck(x < VIEW3D_WIDTH) ;
                    G_maxY[x] = maxY = top ;
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
                    if (top <= G_minY[x])  {
DebugCheck(x < VIEW3D_WIDTH) ;
                        if (!G_colDone[x])  {
                            G_colCount++ ;
                            G_colDone[x] = 1 ;
                        }
                    }
                }

                break ;
            case WALL_TYPE:
                /* Block off the drawn areas. */
                /* Note that for walls, this makes bottom greater than */
                /* top and visa-versa. */
                if (G_wall.opaque == 1)  {
                    if (minY < top)  {
DebugCheck(minY <= VIEW3D_HEIGHT) ;
DebugCheck(top <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, minY, top, G_wall.sector) ;
                    }
                    if (bottom < maxY)  {
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
                        IAddVertFloor(x, bottom, maxY, G_wall.sector) ;
                    }

                    if (bottom > minY)  {
DebugCheck(x < VIEW3D_WIDTH) ;
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
                        G_minY[x] = bottom ;
                    }
                    if (top < maxY)  {
DebugCheck(top <= VIEW3D_HEIGHT) ;
DebugCheck(x <= VIEW3D_WIDTH) ;
                        G_maxY[x] = top ;
                    }
DebugCheck(G_maxY[x] <= VIEW3D_HEIGHT) ;
                }

                break ;
        }
INDICATOR_LIGHT(857, INDICATOR_RED) ;

        G_wallRunCount++ ;
INDICATOR_LIGHT(841, INDICATOR_RED) ;
    }

    /* Note that we have drawn a wall and we can re-use many of our */
    /* calculations for similar walls. */
    G_newLine = FALSE ;

ITestMinMax(1001) ;
INDICATOR_LIGHT(821, INDICATOR_RED) ;
#endif
}


T_byte8 *IDetermineShade(T_sword32 distance, T_word16 shadeStart)
{
    T_byte8 *p_shade ;
    T_sword16 shadeIndex ;
    T_sword16 adjustedShade ;

    adjustedShade = ((T_sword16)shadeStart) + G_darknessAdjustment ;
    if (adjustedShade < 0)
        adjustedShade = 0 ;
    if (adjustedShade > 63)
        adjustedShade = 63 ;
    shadeStart = adjustedShade ;

    if (distance >= 171)  {
        /* Too far to have ambient lighting.  Just use regular lighting */
        p_shade = &P_shadeIndex[(shadeStart)<<8] ;
    } else {
        distance += (distance >> 1) ;
        shadeIndex = (256-distance)>>3 ;
        if (shadeIndex < 0)
            shadeIndex = 0 ;
        shadeIndex += shadeStart ;
        if (shadeIndex > 63)
            shadeIndex = 63 ;
        p_shade = &P_shadeIndex[shadeIndex<<8] ;
    }

    return p_shade ;
}

T_sword32 G_alpha = 0x0000 ;

T_void View3dSetUpDownAngle(T_sword32 alpha)
{
    DebugRoutine("View3dSetUpDownAngle") ;

    if (alpha > VIEW3D_EVEN_HALF_HEIGHT-2)
        alpha = VIEW3D_EVEN_HALF_HEIGHT-2 ;
    if (alpha < 2-VIEW3D_EVEN_HALF_HEIGHT)
        alpha = 2-VIEW3D_EVEN_HALF_HEIGHT ;

    G_alpha = alpha ;

    VIEW3D_HALF_HEIGHT = VIEW3D_EVEN_HALF_HEIGHT + alpha ;

    DebugEnd() ;
}

T_sword32 View3dGetUpDownAngle(T_void)
{
    DebugRoutine("View3dGetUpDownAngle") ;

    DebugEnd() ;

    return G_alpha ;
}

static T_void IAddVertFloor(
                  T_word16 x,
                  T_sword16 top,
                  T_sword16 bottom,
                  T_word16 sector)
{
    T_word16 index ;
    T_word16 prevIndex ;
    T_word16 newIndex ;
    T_vertFloorInfo *p_vert ;
    T_sword16 minY, maxY ;

    DebugRoutine("IAddVertFloorUp") ;
    DebugCheck(top < bottom) ;

#ifndef NDEBUG
if ((top > bottom) ||
    (top > VIEW3D_HEIGHT) ||
    (bottom> VIEW3D_HEIGHT))  {
    printf("3) top=%d  bottom=%d\n", top, bottom) ;
    fprintf(stderr, "3) top=%d  bottom=%d\n", top, bottom) ;
    fflush(stdout) ;
}
#endif
DebugCheck(top <= bottom ) ;
DebugCheck(top <= VIEW3D_HEIGHT) ;
DebugCheck(bottom <= VIEW3D_HEIGHT) ;
    minY = G_minY[x] ;
DebugCheck(minY <= VIEW3D_HEIGHT) ;
    maxY = G_maxY[x] ;
DebugCheck(maxY <= VIEW3D_HEIGHT) ;
    if (top < minY)
        top = minY ;
    if (bottom < minY)
        bottom = minY ;
    if (top > maxY)
        top = maxY ;
    if (bottom > maxY)
        bottom = maxY ;

/*
    if (top >= bottom)  {
//:        printf("bad vert\n") ;
        DebugEnd() ;
        return ;
    }
*/

/*
    if (top > VIEW3D_HALF_HEIGHT)  {
        sector = G_remapSectorFloorArray[sector] ;
    } else {
        sector = G_remapSectorCeilingArray[sector] ;
    }
*/
    /* Allocate a new entry. */
    newIndex = G_numVertFloor++ ;
    p_vert = G_vertFloorInfo+newIndex ;

    /* Insert sort this new entry. */
    /* Find a place to insert at. */
    index = G_vertFloorStarts[x] ;
    prevIndex = 0 ;
    while (index)  {
        if (top < G_vertFloorInfo[index].top)
            break ;
        prevIndex = index ;
        index = G_vertFloorInfo[index].next ;
    }

    /* Are we at the beginning? */
    if (prevIndex)  {
        /* Not at beginning.  Put us after another cell. */
        G_vertFloorInfo[prevIndex].next = newIndex ;
    } else {
        /* We are the new beginning. */
        G_vertFloorStarts[x] = newIndex ;
    }

    /* Fill out the rest. */
    p_vert->next = index ;
    p_vert->top = top ;
    p_vert->bottom = bottom ;
    p_vert->sector = sector ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawRuns
 *-------------------------------------------------------------------------*/
/**
 *  IDrawRuns does the actual work of drawing all the different parts
 *  onto the screen.
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawRuns(T_void)
{
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawColumn
 *-------------------------------------------------------------------------*/
/**
 *  IDrawColumn draws a vertical line at the given x position from
 *  the given top and bottom pixels.  The color is also passed in.
 *
 *  @param x -- X position on screen
 *  @param top -- Top Y position
 *  @param bottom -- Bottom Y position
 *  @param color -- Color to draw line in
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawColumn(
           T_word16 x,
           T_word16 top,
           T_word16 bottom,
           T_byte8 color)
{
    T_byte8 *p_pixel ;
    T_word16 y ;

    if (top >= MAX_VIEW3D_HEIGHT)
        top = 0 ;
    if (bottom > MAX_VIEW3D_HEIGHT)
        bottom = MAX_VIEW3D_HEIGHT ;

//    p_pixel = &P_doubleBuffer[((top << 6) + (top << 8)) + x] ;
    p_pixel = G_doublePtrLookup[top] + x ;
    for (y=top; y<bottom; y++, p_pixel += 320)
        *p_pixel = color ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawRow
 *-------------------------------------------------------------------------*/
/**
 *  IDrawColumn draws a vertical line at the given x position from
 *  the given top and bottom pixels.  The color is also passed in.
 *
 *  @param y -- Y position on screen
 *  @param left -- Left X position
 *  @param right -- Right Y position
 *  @param color -- Color to draw line in
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawRow(
           T_word16 y,
           T_word16 left,
           T_word16 right,
           T_byte8 color)
{
    T_byte8 *p_pixel ;

    if (left >= MAX_VIEW3D_WIDTH)
        left = 0 ;
    if (right > MAX_VIEW3D_WIDTH)
        right = MAX_VIEW3D_WIDTH ;

//    p_pixel = &P_doubleBuffer[((y << 6) + (y << 8)) + left] ;
    p_pixel = G_doublePtrLookup[y] + left ;
    memset(p_pixel, color, right-left) ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IDrawTextureColumn
 *-------------------------------------------------------------------------*/
/**
 *  IDrawTextureColumn draws a vertical line using a given run length
 *  and column.
 *
 *  @param x -- X position on screen
 *  @param p_run -- Wall run length to draw
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawTextureColumn(
           T_word16 x,
           T_3dWallRun *p_run)
{
    T_word32 textureOffset ;
    T_word32 textureStep ;
    T_sword32 top, bottom ;
    T_sword32 shadeIndex ;
    T_byte8 *p_shade ;
    T_byte8 *p_pixel ;
    T_sword16 distance ;

    top = p_run->top ;
    bottom = p_run->bottom ;

    /* Shouldn't have to clip, but we will for safety! */
/* LES???
    if (top >= MAX_VIEW3D_HEIGHT)
        top = 0 ;
    if (bottom > MAX_VIEW3D_HEIGHT)
        bottom = MAX_VIEW3D_HEIGHT ;
*/

    /* Calculate the shading for distance. */
    distance = p_run->distance ;
/*  Never occurs:
    if (distance <= -171)  {
        // Distance is too close ... just make the brightest. (negative?)
        p_shade = &P_shadeIndex[63<<8] ;
    } else
*/
    if (distance >= 171)  {
        /* Too far to have ambient lighting.  Just use regular lighting */
        p_shade = &P_shadeIndex[(p_run->shadeIndex)<<8] ;
    } else {
        distance += (distance >> 1) ;
        shadeIndex = (256-distance)>>3 ;
        if (shadeIndex < 0)
            shadeIndex = 0 ;
        shadeIndex += p_run->shadeIndex ;
        if (shadeIndex > 63)
            shadeIndex = 63 ;
        p_shade = &P_shadeIndex[shadeIndex<<8] ;
///        p_shade = &P_shadeIndex[p_run->shadeIndex<<8] ;
    }

    /* Find the location of the first pixel we are to draw. */
//    p_pixel = &P_doubleBuffer[((top << 6) + (top << 8)) + x] ;
    p_pixel = G_doublePtrLookup[top] + x ;
///    p_pixel = &P_doubleBuffer[((top << 2) + (top << 4)) + (x>>2)] ;

    /* Calculate the location in the texture and it's stepping rate. */
    /* First of all, we know the texture starts at the textureColumn. */
//    p_texture = p_run->p_texture+(((p_run->textureColumn)&63)<<6) ;
    G_CurrentTexturePos =
        p_run->p_texture+
            (((p_run->textureColumn)&(p_run->maskX))<<(p_run->shift)) ;
//PictureCheck(p_run->p_texture) ;

    textureStep = p_run->du ;
    textureOffset = p_run->u + (p_run->du * top) ;

    textureOffset += (((T_sword32)p_run->offY) << 16) ;

    G_textureAndY = p_run->maskY ;

    switch (p_run->shift)  {
        case 0:
            DrawTextureColumnAsm1(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 1:
            DrawTextureColumnAsm2(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 2:
            DrawTextureColumnAsm4(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 3:
            DrawTextureColumnAsm8(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 4:
            DrawTextureColumnAsm16(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 5:
            DrawTextureColumnAsm32(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 6:
            DrawTextureColumnAsm64(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 7:
            DrawTextureColumnAsm128(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 8:
            DrawTextureColumnAsm256(
                p_shade,
                bottom-top,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawTextureColumnNew
 *-------------------------------------------------------------------------*/
/**
 *  IDrawTextureColumnNew is the new version of IDrawTextureColumn.  The
 *  difference is that this routine JUST dispatches the data to the correct
 *  texture drawing routine.  No calculations are done on this level any
 *  more.
 *
 *  @param p_shade -- Pointer into shade table
 *  @param numPixels -- How many pixels high to draw
 *  @param textureStep -- How fast to step through the texture
 *  @param textureOffset -- Start position in texture
 *  @param p_pixel -- First position on the screen
 *  @param shift -- Texture power of 2 factor up and down
 *
 *<!-----------------------------------------------------------------------*/
T_void IDrawTextureColumnNew(
           T_byte8 *p_shade,
           T_word16 numPixels,
           T_word32 textureStep,
           T_word32 textureOffset,
           T_byte8 *p_pixel,
           T_byte8 shift)
{
    switch (shift)  {
        case 0:
            DrawTextureColumnAsm1(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 1:
            DrawTextureColumnAsm2(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 2:
            DrawTextureColumnAsm4(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 3:
            DrawTextureColumnAsm8(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 4:
            DrawTextureColumnAsm16(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 5:
            DrawTextureColumnAsm32(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 6:
            DrawTextureColumnAsm64(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 7:
            DrawTextureColumnAsm128(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
        case 8:
            DrawTextureColumnAsm256(
                p_shade,
                numPixels,
                textureStep,
                textureOffset,
                p_pixel) ;
            break ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  IFindObjects
 *-------------------------------------------------------------------------*/
/**
 *  IFindObjects locates all objects in the view and add them to the
 *  wall runs.
 *
 *<!-----------------------------------------------------------------------*/
T_void IFindObjects(T_void)
{
    T_word16 i ;
    T_sword32 closest ;
    T_3dObject *p_obj ;

    DebugRoutine("IFindObjects") ;

    memset(G_objectColStart, 0xFF, sizeof(G_objectColStart)) ;
    G_allocatedColRun = 0 ;

    p_obj = G_First3dObject ;
    while (p_obj != NULL)  {
        /* Make sure only visible objects are drawn. */
        if ((!(p_obj->attributes & OBJECT_ATTR_INVISIBLE)) &&
            (!(p_obj->attributes & OBJECT_ATTR_BODY_PART)))
            IFindObject(p_obj) ;

        p_obj = p_obj->nextObj ;
    }

    /* Add additional chained objects to those objects */
    /* already in the view. */
    IAddChainedObjects() ;

    /* Sort the objects in Z order. */
    ISortObjects() ;

    /* Set up the screen tables. */
    memset(G_screenObjectPosition, 0, sizeof(G_screenObjectPosition)) ;
    if (G_objectCount != 0)  {
        closest = G_objectRun[0].runInfo.distance ;

        for (i=0; i<VIEW3D_WIDTH; i++)
            G_screenObjectDistance[i] = closest ;
    } else {
        /* Set to close to maximum signed value. */
        memset(G_screenObjectDistance, 0x7F, sizeof(G_screenObjectDistance)) ;
    }

    DebugEnd() ;
}

E_Boolean IFindObject(T_3dObject *p_obj)
{
    T_sword32 relativeZ, relativeX, relativeHeight ;
    T_sword32 relX, relY ;
    T_sword32 width ;
    T_sword32 screenLeftX, screenRightX, screenBottomY, screenTopY ;
    T_sword32 screenRealBottomY ;
    T_word16 sector ;
    T_sword32 floor ;
    T_sword32 picHeight ;
    T_sword32 picWidth ;
    T_sword32 invD ;
    E_Boolean canSee = TRUE ;
    T_word32 index ;
    T_word16 i ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;

    DebugRoutine("IFindObject") ;

    canSee = FALSE ;
    for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
        index = G_fromSector * G_Num3dSectors + ObjectGetNthAreaSector(p_obj, i) ;
        if (!(G_3dReject[index>>3] & powers[index&7]))  {
            /* Draw if there is a partial part in view. */
            canSee = TRUE ;
            break ;
        }
    }

    /* Stop if we can't possibly see the object in the first place. */
    if (canSee == FALSE)  {
        DebugEnd() ;
        return FALSE ;
    }

    relX = ObjectGetX(p_obj) - PlayerGetX() ;
    relY = ObjectGetY(p_obj) - PlayerGetY() ;

    /* Find the relative Z from the player.  This will also act as */
    /* the approximate distance to the object. */
    relativeZ = MultAndShift16(relX, G_3dCosPlayerAngle) -
                 MultAndShift16(relY, G_3dSinPlayerAngle) ;
    /* Are we behind the player? */
    if (relativeZ < 0)  {
        /* Yeah, well ... don't do this one. */
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    /* Hmmm, well, where is it left to right? */
    relativeX = MultAndShift16(relX, G_3dSinPlayerAngle) +
                 MultAndShift16(relY, G_3dCosPlayerAngle) ;

    /* Get the half width of this object from the middle. */
//    picWidth = (ObjectGetPictureWidth(p_obj) << 16);
//picWidth = MultAndShift16(picWidth, 128000L) ;
    picWidth = ObjectGetPictureWidth(p_obj) * p_obj->scaleX ;
    width = (picWidth >> 1) ;


    /* See if the left edge is on the screen. */
    if ((relativeX-width) > relativeZ)  {
        /* Nope.  This one's no good. */
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    /* See if the right edge is on the screen. */
    if ((relativeX+width) < (-relativeZ))  {
        /* Nope.  This one's no good. */
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    /* Get the height of the object. */
    relativeHeight = G_eyeLevel32 - ObjectGetZ(p_obj) ;

    /* Get 1/z */
    invD = MathInvDistanceLookup(relativeZ>>16) ;

    /* Find where the left is on the screen. */
    screenRightX = VIEW3D_HALF_WIDTH -
                   MultAndShift32((relativeX-width), invD) ;
//    screenRightX = (VIEW3D_HALF_WIDTH - (((relativeX-width) * invD) >> 16)) ;

    /* Find where the right is on the screen. */
    screenLeftX = VIEW3D_HALF_WIDTH -
                   MultAndShift32((relativeX+width), invD) ;
//    screenLeftX = (VIEW3D_HALF_WIDTH - (((relativeX+width) * invD) >> 16)) ;

    /* Get rid of too thin objects. */
    if (screenRightX == screenLeftX)  {
        DebugEnd() ;
        return FALSE ;
    }

    /* Are we too far to the left? */
    if ((screenRightX < VIEW3D_CLIP_LEFT) || (screenRightX < 0))  {
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    /* Are we too far to the right? */
    if ((screenLeftX >= VIEW3D_CLIP_RIGHT) || (screenLeftX >= VIEW3D_WIDTH))  {
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    /* Calculate where the bottom edge is. */
    /* Also note if the floor is clipping the bottom of the object. */
    picHeight = ObjectGetPictureHeight(p_obj) * p_obj->scaleY ;

    /* What sector is this object in? */
    sector = ObjectGetCenterSector(p_obj) ;

    /* How high is that sector's floor? */
    floor = (MapGetFloorHeight(sector) << 16) ;

    if (ObjectGetZ(p_obj) >= floor)
        /* Object is higher (or equal to) floor */
        screenRealBottomY = screenBottomY = VIEW3D_HALF_HEIGHT +
            MultAndShift32(relativeHeight, invD) ;
    else  {
        /* Object is lower than floor. */
        screenRealBottomY = VIEW3D_HALF_HEIGHT +
            MultAndShift32(relativeHeight, invD) ;
        screenBottomY = VIEW3D_HALF_HEIGHT +
            MultAndShift32((G_eyeLevel32 - floor), invD) ;
    }

    /* Calculate where the top edge is. */
    screenTopY = VIEW3D_HALF_HEIGHT +
                    MultAndShift32((relativeHeight - picHeight), invD) ;

    /* Make sure it is at least partly on the screen. */
    /* Check the top. */
    if (screenTopY >= VIEW3D_HEIGHT)  {
        /* Top is below screen. */
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    if (screenBottomY < 0)  {
        /* Bottom is above the top of the screen. */
        DebugEnd() ;
//TESTING        return FALSE ;
    }

    if (screenTopY == screenBottomY)  {
        /* Too flat. */
        DebugEnd() ;
        return FALSE ;
    }

//printf("Found object: %d (%p) - %s\n", ObjectGetServerId(p_obj), p_obj, DebugGetCallerName()) ;
    /* Everything is fine.  Store the data. */
    G_objectRun[G_objectCount].scrLeft = screenLeftX ;
    G_objectRun[G_objectCount].scrRight = screenRightX ;
    G_objectRun[G_objectCount].runInfo.top = screenTopY ;
    G_objectRun[G_objectCount].runInfo.bottom = screenBottomY ;
    G_objectRun[G_objectCount].runInfo.realBottom = screenRealBottomY ;
    G_objectRun[G_objectCount].runInfo.distance = (relativeZ>>16) ;
    G_objectRun[G_objectCount].runInfo.p_obj = p_obj ;
    if (screenRealBottomY - screenTopY)  {
        G_objectRun[G_objectCount].runInfo.deltaTall =
            (ObjectGetPictureHeight(p_obj) << 16)/(screenRealBottomY - screenTopY) ;
    } else  {
        G_objectRun[G_objectCount].runInfo.deltaTall = 0 ;
    }
    G_objectRun[G_objectCount].runInfo.picHeight = ObjectGetPictureHeight(p_obj) ;
    /* (picHeight>>16) ; */
    G_objectRun[G_objectCount].runInfo.p_picture = ObjectGetPicture(p_obj) ;

    /* Figure out the step rate for the object's column. */
    G_objectRun[G_objectCount].delta = (ObjectGetPictureWidth(p_obj) << 16)/
                                           (screenRightX - screenLeftX) ;

    /** Apply lighting only if applicable. **/
    if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_NO_SHADING)
        G_objectRun[G_objectCount].runInfo.light = 255;
    else
        G_objectRun[G_objectCount].runInfo.light =
            MapGetSectorLighting(sector) ;

    G_objectCount++ ;

    DebugEnd() ;

    return TRUE ;
}


int ICompareObjectRuns(const void *first, const void *second)
{
    T_3dObjectRun *firstRun ;
    T_3dObjectRun *secondRun ;

    firstRun = (T_3dObjectRun *)first ;
    secondRun = (T_3dObjectRun *)second ;

    if (firstRun->runInfo.distance < secondRun->runInfo.distance)
        return -1 ;
    if (firstRun->runInfo.distance == secondRun->runInfo.distance)  {
        if (ObjectGetType(firstRun->runInfo.p_obj) <
            ObjectGetType(secondRun->runInfo.p_obj))
            return -1 ;
        if (ObjectGetType(firstRun->runInfo.p_obj) >
            ObjectGetType(secondRun->runInfo.p_obj))
            return 1 ;
        return 0 ;
    }
    return 1 ;
}

T_void ISortObjects(T_void)
{
    if (G_objectCount > 1)
        qsort(
            G_objectRun,
            G_objectCount,
            sizeof(T_3dObjectRun),
            ICompareObjectRuns) ;
}

T_word16 G_objColumnStart ;
T_word16 G_objColumnEnd ;

/* COLORIZE: */
static T_byte8 G_colorizedObject[2*MAX_VIEW3D_HEIGHT] ;

T_void IDrawObjectColumn(T_word16 x, T_3dObjectColRun *p_run)
{
    T_sword32 top, bottom ;
    T_sword32 clipTop, clipBottom ;
    T_sword32 delta ;
    T_byte8 *p_texture ;
    T_sword32 line ;
    T_byte8 *p_pixel ;
    T_byte8 *p_shade ;
    T_sword32 realBottom ;
    T_pictureRaster *p_entry ;
    T_3dObjectRunInfo *p_runInfo ;
    E_colorizeTable table ;
T_byte8 c ;

    /* Get a quick pointer to the common run info. */
    p_runInfo = p_run->p_runInfo ;
    DebugCheck(p_runInfo != NULL) ;

    /* Find the column in the texture. */
//    p_texture = &p_run->p_picture[p_run->column * p_run->picHeight] ;
    DebugCheck(p_run->p_runInfo->p_picture != NULL) ;
    p_entry = &((T_pictureRaster *)(p_runInfo->p_picture))[p_run->column] ;
    if (p_entry->start == 255)
        return ;

    G_objColumnStart = p_entry->start ;
    G_objColumnEnd = p_entry->end ;
//printf("column: %d  x: %d  start: %d  end: %d\n", p_run->column, x, G_objColumnStart, G_objColumnEnd) ;
    p_texture = &p_runInfo->p_picture[p_entry->offset-p_entry->start-4] ;
    line = 0 ;
    top = p_runInfo->top ;
    bottom = p_runInfo->bottom ;
    clipTop = p_run->clipTop ;
    clipBottom = p_run->clipBottom ;
    realBottom = p_runInfo->realBottom ;

    if (realBottom != top)  {
        /* Find the delta step for this object's column. */
        delta = (p_runInfo->picHeight*65536)/(realBottom-top) ;

        /* Clip the top. */
        if (top < clipTop)  {
            /* Skip some of the lines. */
            line += (clipTop-top)*delta ;

            top = clipTop ;
        }

        /* Clip the bottom. */
        if (bottom > clipBottom)
            bottom = clipBottom;

        if (bottom > top)  {
            p_shade = IDetermineShade(
                          p_runInfo->distance,
                          (p_runInfo->light>>2)) ;
            /* Find the location of the first pixel we are to draw. */
            p_pixel = G_doublePtrLookup[top]+x ;

            G_CurrentTexturePos = p_texture ;
/* TESTING:  Make everything colored. */
       if ((table = ObjectGetColorizeTable(p_runInfo->p_obj)) !=
               COLORIZE_TABLE_NONE)  {
/* TESTING for page faults */
c = *((T_byte8 *)p_texture) ;
c = *((T_byte8 *)G_colorizedObject) ;
//c = *((T_byte8 *)PictureGetHeight(p_runInfo->p_picture)) ;
           if (G_objColumnEnd >= G_objColumnStart)  {
               ColorizeMemory(
                   p_texture+p_entry->start,
                   G_colorizedObject+p_entry->start,
                   1 + G_objColumnEnd - G_objColumnStart,
    //               PictureGetHeight(p_runInfo->p_picture),
                   table) ;
           }
           G_CurrentTexturePos = G_colorizedObject ;
       }

DebugCheck(bottom >= 0) ;
DebugCheck(top >= 0) ;
DebugCheck(bottom < 200) ;
DebugCheck(top < 200) ;
DebugCheck(x < 320) ;
DebugCheck(bottom > top) ;
DebugCheck(p_shade != NULL) ;
DebugCheck(p_pixel != NULL) ;
DebugCheck(((T_word32)p_pixel) < (((T_word32)P_doubleBuffer)+64000)) ;
DebugCheck(((T_word32)p_pixel) >= ((T_word32)P_doubleBuffer)) ;

            if (ObjectGetAttributes(p_run->p_runInfo->p_obj) &
                    OBJECT_ATTR_TRANSLUCENT)  {
                /* transparent and translucent. */
                DrawTranslucentObjectColumnAsm(
                    p_shade,
                    bottom-top,
                    delta,
                    line,
                    p_pixel) ;
            } else {
                /* transparently opaque. */
                DrawObjectColumnAsm(
                    p_shade,
                    bottom-top,
                    delta,
                    line,
                    p_pixel) ;
            }
        }
    }
}

T_void IAddObjectSlice(
           T_sword32 zDist,
           T_sword16 x,
           T_sword16 clipTop,
           T_sword16 clipBottom)
{
    T_word16 objPos ;
    T_3dObjectRun *p_objRun ;
    T_word16 column ;
    T_3dObjectColRun *p_run ;
    T_word16 count ;
    T_3dObject *p_obj ;
    T_sword32 dist ;

    DebugRoutine("IAddObjectSlice") ;

    objPos = G_screenObjectPosition[x] ;
    if (objPos >= G_objectCount)  {
        DebugEnd() ;
        return ;
    }

    dist = G_screenObjectDistance[x] ;

    while (1==1)  {
        /* Check to see if we have a valid object at this screen column. */
//        if (objPos < G_objectCount)  {
            /* Yes, we do. */
            /* Is this x inside the left and right bounds of the object? */
            p_objRun = G_objectRun+objPos ;
            if ((x >= p_objRun->scrLeft) && (x < p_objRun->scrRight))  {
                /* Yes, the object is aligned over this column. */
                /* Add it's slice. */

                /* Find the column in it to use. */
                column = (p_objRun->delta *
                             ((T_sword32)(x -
                                 p_objRun->scrLeft))) >> 16 ;

                p_obj = p_objRun->runInfo.p_obj ;
                /* If this object is to be reversed, we need to flip */
                /* it around and make the column from the other side. */
                if (p_obj->orientation == ORIENTATION_REVERSE)  {
                    /* yes, flip left to right. */
                    column = (ObjectGetPictureWidth(p_obj) - column) - 1 ;
                }

                if (p_objRun->runInfo.top != p_objRun->runInfo.bottom)  {
//                    count = G_objectColCount[x]++ ;
                    count = G_allocatedColRun++ ;

                    /* Make sure our depth of objects is not too long. */
#ifndef NDEBUG
if (count >= MAX_OBJECT_COLUMN_RUNS)  {
    printf("Too deep on column %d!\n", x) ;
    printf("Count = %d\n", count) ;
    printf("Working object: (%d)\n", ObjectGetServerId(p_obj)) ;
    ObjectPrint(stdout, p_obj) ;
}
#endif
                    DebugCheck(count != MAX_OBJECT_COLUMN_RUNS) ;
                    if (count < MAX_OBJECT_COLUMN_RUNS)  {
                        p_run = &G_objectColRunList[count] ;
                        p_run->next = G_objectColStart[x] ;
                        G_objectColStart[x] = count ;
                        p_run->column = column ;
                        p_run->clipTop = clipTop ;
                        p_run->clipBottom = clipBottom ;

                        /* Copy over info. !!! */
                        p_run->p_runInfo = &p_objRun->runInfo ;
                    } else {
                        G_allocatedColRun-- ;
                    }
/*
                    p_run->top = p_objRun->scrTop ;
                    p_run->bottom = p_objRun->scrBottom ;
                    p_run->realBottom = p_objRun->scrRealBottom ;
                    p_run->distance = p_objRun->distance ;
                    p_run->light = p_objRun->light ;
                    p_run->picHeight = p_objRun->picHeight ;
                    p_run->p_picture = p_objRun->p_picture ;
                    p_run->p_obj = p_objRun->p_obj ;
                    p_run->deltaTall = p_objRun->deltaTall ;
*/
                }
            }

            /* Move to the next object. */
            objPos++ ;
            if (objPos < G_objectCount)  {
                dist = G_objectRun[objPos].runInfo.distance ;
                if (dist > zDist)  {
                    /* Stop looping. */
                    break ;
                } else {
                    /* Keep looping. */
                }
            }  else  {
                 /* Take the distance out to something rediculous. */
                 dist = 100000 ;

                 /* Stop looping. */
                 break ;
            }
//        } else {
            /* No, we don't.  Move the next location to twice the distance. */
//            G_screenObjectDistance[x] <<= 1 ;

            /* Stop looping. */
//            break ;
//        }
    }

    G_screenObjectPosition[x] = objPos ;
    G_screenObjectDistance[x] = dist ;

    DebugEnd() ;
}

T_void IDrawObjectAndWallRuns(T_void)
{
    T_sword16 x ;
    T_word16 i ;
    T_sword16 j ;
    T_sword32 distObj, distWall ;

    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    INDICATOR_LIGHT(862, INDICATOR_GREEN) ;

    for (x=0; x<VIEW3D_WIDTH; x++)  {
        /* How many wall slices are at this column? */
        j = G_numWallSlices[x] ;
        /* How many object slices are at this column? */
        i=G_objectColStart[x] ;

        /* Loop while there are objects and/or wall slices */
        while ((j) || (i != 0xFFFF))  {
            if (!j)  {
                /* Only objects. */
                INDICATOR_LIGHT(866, INDICATOR_GREEN) ;
                IDrawObjectColumn(x, &G_objectColRunList[i]) ;
                i = G_objectColRunList[i].next ;
                INDICATOR_LIGHT(866, INDICATOR_RED) ;
            } else if (i == 0xFFFF) {
                /* Only walls. */
                INDICATOR_LIGHT(870, INDICATOR_GREEN) ;
                IDrawWallSliceColumn(&G_wallSlices[x][--j]) ;
                INDICATOR_LIGHT(870, INDICATOR_RED) ;
            } else {
                /* Both */
                /* Which is further? */
                distWall = G_wallSlices[x][j-1].distance ;
                distObj = G_objectColRunList[i].p_runInfo->distance ;
                if (distWall >= distObj)  {
                    /* Wall is further--draw it first */
                    INDICATOR_LIGHT(874, INDICATOR_GREEN) ;
                    IDrawWallSliceColumn(&G_wallSlices[x][--j]) ;
                    INDICATOR_LIGHT(874, INDICATOR_RED) ;
                } else {
                    /* Object is further--draw it first. */
                    INDICATOR_LIGHT(878, INDICATOR_GREEN) ;
                    IDrawObjectColumn(x, &G_objectColRunList[i]) ;
                    i = G_objectColRunList[i].next
                    INDICATOR_LIGHT(878, INDICATOR_RED) ;
                }
            }
        }
    }

    INDICATOR_LIGHT(862, INDICATOR_RED) ;
    TICKER_TIME_ROUTINE_ENDM("IDrawObjectAndWallRuns", 500) ;
}

T_void IDrawPixel(T_byte8 *p_pixel, T_byte8 color)
{
    T_word32 pos ;
    T_word32 offset ;

    pos = (T_word32)p_pixel ;
    offset = (pos-0xA0000) >> 2 ;
    GrActivateColumn(pos&3) ;

    ((T_byte8 *)(0xA0000))[offset] = color ;
}

T_void View3dSetSize(T_word16 width, T_word16 height)
{
    T_byte8 *p_where ;
    T_word16 i ;

    DebugRoutine("View3dSetSize") ;
    DebugCheck(width <= MAX_VIEW3D_WIDTH) ;
    DebugCheck(height <= MAX_VIEW3D_HEIGHT) ;

    VIEW3D_WIDTH = width ;
    VIEW3D_HEIGHT = height ;
    VIEW3D_HALF_WIDTH = width >> 1 ;
    VIEW3D_EVEN_HALF_HEIGHT = VIEW3D_HALF_HEIGHT = height >> 1 ;
    VIEW3D_CLIP_LEFT = 0 ;
    VIEW3D_CLIP_RIGHT = VIEW3D_WIDTH ;

    for (p_where=P_doubleBuffer, i=0;
         i<MAX_VIEW3D_HEIGHT;
         i++, p_where+=MAX_VIEW3D_WIDTH)  {
        G_doublePtrLookup[i] = p_where-VIEW3D_CLIP_LEFT ;
    }

    MathInitialize(width) ;

/* !!! Hardcoded !!! */
/*
    p_backdrop = PictureLock("CLOUDS2.PIC", &res) ;
    MapSetBackdrop(p_backdrop) ;
    PictureUnlock(res) ;
    PictureUnfind(res) ;
*/

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dClipCenter
 *-------------------------------------------------------------------------*/
/**
 *  View3dClipCenter sets up the clipping left and right egdes based on
 *  the given view size already.  Just pass in how wide you want the final
 *  view to be.
 *
 *  NOTE: 
 *  Do not try to clip a width bigger than the width of the view.
 *
 *  @param centerWidth -- How width is the clipped view.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dClipCenter(T_word16 centerWidth)
{
    T_byte8 *p_where ;
    T_word16 i ;

    DebugRoutine("View3dClipCenter") ;
    DebugCheck(centerWidth <= VIEW3D_WIDTH) ;

    VIEW3D_CLIP_LEFT = ((VIEW3D_WIDTH - centerWidth)>>1) ;
    VIEW3D_CLIP_RIGHT = VIEW3D_CLIP_LEFT + centerWidth ;

    for (p_where=P_doubleBuffer, i=0;
         i<MAX_VIEW3D_HEIGHT;
         i++, p_where+=MAX_VIEW3D_WIDTH)  {
        G_doublePtrLookup[i] = p_where-VIEW3D_CLIP_LEFT ;
    }

    DebugEnd() ;
}

#if 0
E_Boolean ICheckValidRow(T_word16 row, T_word16 run)
{
    T_word16 i ;
    E_Boolean status = TRUE ;
    T_word16 start, end ;

    start = G_floorList[row][run].start ;
    end = G_floorList[row][run].end ;

    for (i=0; i<run; i++)  {
/*
             if (((start >= G_floorList[row][i].start) &&
                  (start <= (G_floorList[row][i].end-1))) ||
                 (((end-1) >= G_floorList[row][i].start) &&
                  ((end-1) <= (G_floorList[row][i].end-1))) ||
                 ((start >= G_floorList[row][i].start) &&
                  ((end-1) <= G_floorList[row][i].end-1)))
*/
             if (!((start >= G_floorList[row][i].end) ||
                 (end <= G_floorList[row][i].start)))
             {
            G_floorList[row][run].start = 0x7FFF ;
            G_floorList[row][run].end = 0x7FFF ;
            status = FALSE ;
            break ;
        }
    }

    return status ;
}

E_Boolean ICheckValidRow2(T_word16 row, T_word16 run)
{
    T_word16 i ;
    T_word16 start, end ;
    T_word16 rstart, rend ;
    T_3dFloorRun *p_new ;
    T_3dFloorRun *p_old ;
    E_Boolean status = TRUE ;

    p_new = &G_floorList[row][run] ;
    start = p_new->start ;
    end = p_new->end ;

    for (i=0; i<run; i++)  {
         p_old = &G_floorList[row][i] ;
         rstart = p_old->start ;
         rend = p_old->end ;


         if (start == rend)  {
             if ((p_new->height == p_old->height) &&
                 (p_new->shadeIndex == p_old->shadeIndex) &&
                 (p_new->transparentFlag == p_old->transparentFlag))  {
                 if (row < VIEW3D_HALF_HEIGHT)  {
                     if (p_new->textureCeiling == p_old->textureCeiling)  {
                         /* Combine these two. */
                         p_old->end = p_new->end ;
//                         return FALSE ;
                         status = FALSE ;
                         p_new->start = p_new->end+1 ;
                         p_new = p_old ;
                     }
                 } else {
                     if (p_new->textureFloor == p_old->textureFloor)  {
                         /* Combine these two. */
                         p_old->end = p_new->end ;
//                         return FALSE ;
                         status = FALSE ;
                         p_new->start = p_new->end+1 ;
                         p_new = p_old ;
                     }
                 }
             }
         }

         if (end == rstart)  {
             if ((p_new->height == p_old->height) &&
                 (p_new->shadeIndex == p_old->shadeIndex) &&
                 (p_new->transparentFlag == p_old->transparentFlag))  {
                 if (row < VIEW3D_HALF_HEIGHT)  {
                     if (p_new->textureCeiling == p_old->textureCeiling)  {
                         /* Combine these two. */
                         p_old->start = p_new->start ;
//                         return FALSE ;
                         status = FALSE ;
                         p_new->start = p_new->end+1 ;
                         p_new = p_old ;
                     }
                 } else {
                     if (p_new->textureFloor == p_old->textureFloor)  {
                         /* Combine these two. */
                         p_old->start = p_new->start ;
//                         return FALSE ;
                         status = FALSE ;
                         p_new->start = p_new->end+1 ;
                         p_new = p_old ;
                     }
                 }
             }
         }

         if (!((start >= rend) || (end <= rstart)))
;//TESTING            return FALSE ;
    }

//    return TRUE ;
    return status ;
}
#endif

static T_void IConvertVertToHorzAndDraw(T_void)
{
    T_horzFloorInfo floor[MAX_VIEW3D_HEIGHT] ;
    T_word16 leftTop ;
    T_word16 leftBottom ;
    T_word16 rightTop ;
    T_word16 rightBottom ;
    T_vertFloorInfo *p_left ;
    T_vertFloorInfo *p_right ;
    T_vertFloorInfo *p_leftStart ;
    T_vertFloorInfo *p_rightStart ;
    T_word16 x ;
    T_word16 y ;

    DebugRoutine("IConvertVertToHorzAndDraw") ;

    /* The first column has no left to speak of. */
    p_leftStart = NULL ;

    /* Loop through each of the vertical strips and convert. */
    for (x=0; x<=VIEW3D_WIDTH; x++)  {
//:printf("x=%d\n", x) ;
//:fflush(stdout) ;
        if (x == VIEW3D_WIDTH)
            /* If at far right, there isn't one past the end. */
            p_right = p_rightStart = NULL ;
        else  {
            if (G_vertFloorStarts[x])  {
                /* Start up the right side (if there is one). */
                p_right = p_rightStart =
                    G_vertFloorInfo + G_vertFloorStarts[x] ;
                rightTop = p_right->top ;
                rightBottom = p_right->bottom ;
            } else  {
                p_right = p_rightStart = NULL ;
            }

        }

        /* Start over the left side. */
        p_left = p_leftStart ;
        if (p_left)  {
            leftTop = p_left->top ;
            leftBottom = p_left->bottom ;
#ifndef NDEBUG
if ((leftTop > leftBottom) ||
    (leftTop > VIEW3D_HEIGHT) ||
    (leftBottom > VIEW3D_HEIGHT))  {
    printf("2) leftTop=%d  leftBottom=%d\n", leftTop, leftBottom) ;
    fprintf(stderr, "2) leftTop=%d  leftBottom=%d\n", leftTop, leftBottom) ;
    fflush(stdout) ;
}
#endif
DebugCheck(leftTop <= leftBottom) ;
DebugCheck(leftTop <= VIEW3D_HEIGHT) ;
DebugCheck(leftBottom <= VIEW3D_HEIGHT) ;
        }

        /* Keep looping while there are items to process. */
        while ((p_right) || (p_left))  {

//:puts(".") ;
            /* Our action depends on who exists. */
            /* Do we have a left side? */
            if (!p_left)  {
//:puts("no left") ;
//:printf("right goes %d to %d\n", rightTop, rightBottom) ;
                /* No, we don't have a left. */
                /* This means that everything in the */
                /* right side are starts of runs. */
                for (y=rightTop; y<rightBottom; y++)  {
                    floor[y].sector = p_right->sector ;
                    floor[y].left = x ;
                }
                /* End this right strip. */
                rightTop = y ;
            } else if (!p_right) {
//:puts("no right") ;
                /* Do we have a right side? */
                /* If we don't, then all the left sides are ends */
                /* or floor runs. */
#ifndef NDEBUG
if ((leftTop > leftBottom) ||
    (leftTop > VIEW3D_HEIGHT) ||
    (leftBottom > VIEW3D_HEIGHT))  {
    printf("1) leftTop=%d  leftBottom=%d\n", leftTop, leftBottom) ;
    fprintf(stderr, "1) leftTop=%d  leftBottom=%d\n", leftTop, leftBottom) ;
    fflush(stdout) ;
}
#endif
DebugCheck(leftTop <= leftBottom) ;
DebugCheck(leftTop <= VIEW3D_HEIGHT) ;
DebugCheck(leftBottom <= VIEW3D_HEIGHT) ;
                for (y=leftTop; y<leftBottom; y++)  {
                    floor[y].right = x ;
DebugCheck(y < 200) ;
                    IDrawFloorRun(y, floor+y) ;
                }
                /* end this left strip. */
                leftTop = y;
            } else  {
//:puts("Left and right") ;
//:printf("left top = %d, left bot=%d, right top = %d, right bot=%d\n", leftTop, leftBottom, rightTop, rightBottom) ;
                /* Both sides exist.  Now comes the fun part. */
                /* Who comes first? */
                if (leftTop < rightTop)  {
                    /* The left is sticking up more than the right. */
                    /* This means that the pixels that stick up */
                    /* are ending run. */
                    for (y=leftTop; (y<rightTop)&&(y<leftBottom); y++)  {
                        floor[y].right = x ;
DebugCheck(y < 200) ;
                        IDrawFloorRun(y, floor+y) ;
                    }
                    leftTop = y ;
                } else if (leftTop > rightTop) {
                    /* The right is sticking up more than the left. */
                    /* This means that the pixels that stick up */
                    /* are starting new runs. */
                    for (y=rightTop; (y<leftTop)&&(y<rightBottom); y++)  {
                        floor[y].sector = p_right->sector ;
                        floor[y].left = x ;
                    }
                    rightTop = y ;
                } else {
                    /* The top edges are the same. */
                    /* Are the sectors the same? */
                    if (p_left->sector != p_right->sector)  {
                        /* The sectors are different. */
                        /* End the left side, start the right side. */
                        for (y=leftTop; (y<leftBottom) && (y<rightBottom); y++)  {
                            /* End the left. */
                            floor[y].right = x ;
DebugCheck(y < 200) ;
                            IDrawFloorRun(y, floor+y) ;

                            /* Start the right. */
                            floor[y].sector = p_right->sector ;
                            floor[y].left = x ;
//:printf("dif sector at %d %d\n", x, y) ;
                        }
                        /* Stopped on either side, but positions */
                        /* are now the same. */
                        rightTop = leftTop = y ;
                    } else {
                        /* The sectors are the same, we just have */
                        /* to pass up one or the other. */
                        /* Skip to the topmost bottom. */
                        if (leftBottom < rightBottom)  {
                            rightTop = leftTop = leftBottom ;
                        } else {
                            rightTop = leftTop = rightBottom ;
                        }
                    }
                }
            }
            /* Are we done with the left strip? */
            if (p_left)  {
                if (leftTop == leftBottom)  {
                    /* Yes, we are.  Set up the next strip. */
                    if (p_left->next)  {
                        p_left = p_left->next + G_vertFloorInfo ;
                        if (p_left)  {
                            leftTop = p_left->top ;
                            leftBottom = p_left->bottom ;
                        }
                    } else {
                        p_left = NULL ;
                    }
                }
            }

            /* Are we done with the right strip? */
            if (p_right)  {
                if (rightTop == rightBottom)  {
                    /* Yes, we are.  Set up the next strip. */
                    if (p_right->next)  {
                        p_right = p_right->next + G_vertFloorInfo ;
                        if (p_right)  {
                            rightTop = p_right->top ;
                            rightBottom = p_right->bottom ;
                        }
                    } else {
                        p_right = NULL ;
                    }
                }
            }
        }
        p_leftStart = p_rightStart ;
    }

    fflush(stdout) ;

    DebugEnd() ;
}

#if 0
T_void IDrawFloorRun(T_word16 y, T_horzFloorInfo *p_floor)
{
/*    printf("drw: %d %d %d %d\n",
        y,
        p_floor->sector,
        p_floor->left,
        p_floor->right) ;
*/
    memset(P_doubleBuffer+y*320+p_floor->left, p_floor->sector, p_floor->right-p_floor->left) ;
}
#endif

T_void IDrawFloorRun(
           T_word16 row,
           T_horzFloorInfo *p_run)
{
    T_byte8 *p_pixel ;
    T_sword32 start, end ;
    T_sword32 x, y, dx, dy ;
    T_sword32 delta ;
    T_sword32 distance ;
    T_byte8 *p_shade ;
    T_word16 sizeX, sizeY ;
    T_word16 left, right, amount ;
    T_3dSector *p_sector ;
    T_3dSectorInfo *p_sectorInfo ;
    T_word16 transparentFlag ;
    T_byte8 *p_texture ;
    T_byte8 mipLevel ;

    p_sector = G_3dSectorArray + p_run->sector ;
    p_sectorInfo = G_3dSectorInfoArray + p_run->sector ;
DebugCheck(p_run->sector <= G_Num3dSectors) ;
    transparentFlag = p_sector->trigger & 1 ;

    if (row >= VIEW3D_HALF_HEIGHT)  {
        /* Floor */
        p_texture = *((T_byte8 **)&p_sector->floorTx[1]) ;
    } else {
        /* Ceiling */
        p_texture = *((T_byte8 **)&p_sector->ceilingTx[1]) ;
    }
    G_CurrentTexturePos = p_texture ;

DebugCheck(p_texture != NULL) ;
    PictureGetXYSize(p_texture, &sizeY, &sizeX) ;

    start = p_run->left ;
    end = p_run->right ;
    delta = 1+end-start ;

DebugCheck(row < 200) ;
DebugCheck(start < 320) ;
    p_pixel = G_doublePtrLookup[row] + start ;

    /* If sizeY = 0, then we must be drawing the sky. */
    /* If so, don't do any unnecessary texture calculations. */
    if (sizeY)  {
        if (row >= VIEW3D_HALF_HEIGHT)  {
            /* Floor */
//            distance = ((G_eyeLevel - p_sector->floorHt) *
//                        MathInvDistanceLookup(row - VIEW3D_HALF_HEIGHT)) ;
            distance = MultAndShift16(
                           G_eyeLevel32 - (p_sector->floorHt << 16),
                           MathInvDistanceLookup(row-VIEW3D_HALF_HEIGHT)) ;
        } else {
            /* Ceiling */
//            distance = -((G_eyeLevel - p_sector->ceilingHt) *
//                        MathInvDistanceLookup(VIEW3D_HALF_HEIGHT-row)) ;
            distance = -MultAndShift16(
                           G_eyeLevel32 - (p_sector->ceilingHt << 16),
                           MathInvDistanceLookup(VIEW3D_HALF_HEIGHT-row)) ;
        }

        /* Move out straight to the left edge. */
        distance = MultAndShift16(distance, MathInvCosineLookup(0x2000)) ;

        x = PlayerGetX() + MultAndShift16(distance, G_3dCosPlayerLeftAngle) ;
        y = PlayerGetY() + MultAndShift16(distance, (-G_3dSinPlayerLeftAngle)) ;

        dx = PlayerGetX() + MultAndShift16(distance, G_3dCosPlayerRightAngle) ;
        dy = PlayerGetY() + MultAndShift16(distance, (-G_3dSinPlayerRightAngle)) ;

        if (row >= VIEW3D_HALF_HEIGHT)  {
            x -= ((T_sword32)p_sectorInfo->textureXOffset) << 16 ;
            y -= ((T_sword32)p_sectorInfo->textureYOffset) << 16 ;

            dx -= ((T_sword32)p_sectorInfo->textureXOffset) << 16 ;
            dy -= ((T_sword32)p_sectorInfo->textureYOffset) << 16 ;
        }

        dx -= x ;
        dy -= y ;

        dx = dx / VIEW3D_WIDTH ;
        dy = dy / VIEW3D_WIDTH ;

        /* Skip over to the first pixel */
        x += dx * start ;
        y += dy * start ;

        /* Calculate the shading for distance. */
        p_shade = IDetermineShade(distance>>16, p_sector->light>>2) ;

#ifdef MIP_MAPPING_ON
        if ((sizeX >= 64) && (sizeY >= 64) && (sizeX != 256))  {
            for (mipLevel=0; mipLevel<4; mipLevel++)  {
                if ((dx <= -0x1C000)||(dx >= 0x1C000)||
                    (dy <= -0x1C000)||(dy >= 0x1C000))  {
                    dx >>= 1 ;
                    dy >>= 1 ;
                    x >>= 1 ;
                    y >>= 1 ;
                    G_CurrentTexturePos += 4 + (sizeX * sizeY) ;
                    sizeX >>= 1 ;
                    if (sizeX == 0)
                        sizeX = 1 ;
                    sizeY >>= 1 ;
                    if (sizeY == 0)
                        sizeY = 1 ;
                } else {
                    break ;
                }
            }
        }
#endif
    }

    if (start < end)  {
        if ((!transparentFlag) ||
            (row > VIEW3D_HALF_HEIGHT))  {
            if (sizeY)  {
                G_textureAndX = sizeX-1 ;
                G_textureAndY = sizeY-1 ;

                G_textureStepX = dx ;
                G_textureStepY = dy ;
                switch(MathPower2Lookup(sizeY))  {
                    case 0:
                        DrawTextureRowAsm1(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 1:
                        DrawTextureRowAsm2(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 2:
                        DrawTextureRowAsm4(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 3:
                        DrawTextureRowAsm8(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 4:
                        DrawTextureRowAsm16(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 5:
                        DrawTextureRowAsm32(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 6:
                        DrawTextureRowAsm64(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 7:
                        DrawTextureRowAsm128(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 8:
                        DrawTextureRowAsm256(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                }
            } else {
                /* Draw sky segment. */
                if (G_backdrop)  {
                    left = G_backdropOffset + start ;
                    right = G_backdropOffset + end ;
                    if (left >= (VIEW3D_WIDTH<<1))  {
                        left -= (VIEW3D_WIDTH<<1) ;
                        right -= (VIEW3D_WIDTH<<1) ;
                    }

    //                    memcpy(p_pixel, G_backdrop+(row*VIEW3D_WIDTH)+left, end-start) ;

                    if (right < (VIEW3D_WIDTH<<1))  {
                        if (end != start)  {
/*
                            memcpy(
                                p_pixel,
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                                end-start) ;
*/
                            DrawAndShadeRaster(
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                                p_pixel,
                                end-start,
                                MapGetOutsideLighting()) ;
                        }
                    } else {
                        amount = (VIEW3D_WIDTH<<1) - left ;
                        if (amount)  {
/*
                            memcpy(
                                p_pixel,
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                                amount) ;
*/
                            DrawAndShadeRaster(
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                                p_pixel,
                                amount,
                                MapGetOutsideLighting()) ;
                        }
                        if (((end-start)-amount) != 0)  {
                            DrawAndShadeRaster(
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1)),
                                p_pixel+amount,
                                (end-start)-amount,
                                MapGetOutsideLighting()) ;
/*
                            memcpy(
                                p_pixel+amount,
                                G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1)),
                                (end-start)-amount) ;
*/
                        }
                    }

                } else  {
                    memset(p_pixel, row, end-start) ;
                }
            }
        } else {
            if (G_backdrop)  {
                left = G_backdropOffset + start ;
                right = G_backdropOffset + end ;
                if (left >= (VIEW3D_WIDTH<<1))  {
                    left -= (VIEW3D_WIDTH<<1) ;
                    right -= (VIEW3D_WIDTH<<1) ;
                }

                if (right < (VIEW3D_WIDTH<<1))  {
                    if (end != start)  {
/*
                        memcpy(p_pixel, G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left, end-start) ;
*/
                        DrawAndShadeRaster(
                            G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                            p_pixel,
                            end-start,
                            MapGetOutsideLighting()) ;
                    }
                } else {
                    amount = (VIEW3D_WIDTH<<1) - left ;
                    if (amount != 0)  {
/*
                        memcpy(
                            p_pixel,
                            G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                            amount) ;
*/
                        DrawAndShadeRaster(
                            G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1))+left,
                            p_pixel,
                            amount,
                            MapGetOutsideLighting()) ;
                    }
                    if (((end-start)-amount) != 0)  {
/*
                        memcpy(
                            p_pixel+amount,
                            G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1)),
                            (end-start)-amount) ;
*/
                        DrawAndShadeRaster(
                            G_backdrop+((row-G_alpha)*(VIEW3D_WIDTH<<1)),
                            p_pixel+amount,
                            (end-start)-amount,
                            MapGetOutsideLighting()) ;
                    }
                }

            } else  {
                memset(p_pixel, row, end-start) ;
            }

            if (sizeY != 0)  {
                G_textureAndX = sizeX-1 ;
                G_textureAndY = sizeY-1 ;

                G_textureStepX = dx ;
                G_textureStepY = dy ;
                switch(MathPower2Lookup(sizeY))  {
                    case 0:
                        DrawTransRowAsm1(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 1:
                        DrawTransRowAsm2(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 2:
                        DrawTransRowAsm4(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 3:
                        DrawTransRowAsm8(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 4:
                        DrawTransRowAsm16(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 5:
                        DrawTransRowAsm32(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 6:
                        DrawTransRowAsm64(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 7:
                        DrawTransRowAsm128(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                    case 8:
                        DrawTransRowAsm256(p_shade, end-start, x, y, p_pixel) ;
                        break ;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawTextureColumnLater
 *-------------------------------------------------------------------------*/
/**
 *  IDrawTextureColumnLater puts the requested draw on a waiting list
 *  to be drawn.  Later, the items will be drawn in order of distance.
 *
 *  @param x -- X location on the screen
 *  @param p_shade -- Pointer into shade table
 *  @param numPixels -- How many pixels high to draw
 *  @param textureStep -- How fast to step through the texture
 *  @param textureOffset -- Start position in texture
 *  @param p_pixel -- First position on the screen
 *  @param shift -- Texture power of 2 factor up and down
 *  @param type -- Transparent (2) or translucent (3)
 *  @param distance -- Distance to slice
 *  @param p_texture -- Texture slice to use
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDrawTextureColumnLater(
                  T_word16 x,
                  T_byte8 *p_shade,
                  T_word16 numPixels,
                  T_word32 textureStep,
                  T_word32 textureOffset,
                  T_byte8 *p_pixel,
                  T_byte8 shift,
                  T_byte8 type,
                  T_sword32 distance,
                  T_byte8 *p_texture)
{
    T_3dWallSlice *p_slice ;

    DebugCheck((type == 2) || (type==3)) ;

    p_slice = G_wallSlices[x]+G_numWallSlices[x]++ ;
    DebugCheck(G_numWallSlices[x] <= MAX_WALL_SLICES_PER_COLUMN) ;

    p_slice->p_shade = p_shade ;
    p_slice->numPixels = numPixels ;
    p_slice->textureStep = textureStep ;
    p_slice->textureOffset = textureOffset;
    p_slice->p_pixel = p_pixel ;
    p_slice->shift = shift ;
    p_slice->type = type ;
    p_slice->distance = distance ;
    p_slice->p_texture = p_texture ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDrawWallSliceColumn
 *-------------------------------------------------------------------------*/
/**
 *  IDrawWallSliceColumn draws a previously stored wall column.  The
 *  wall slice can either be transparent or translucent.
 *
 *  @param p_slice -- Slice to draw
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDrawWallSliceColumn(T_3dWallSlice *p_slice)
{
    T_byte8 *p_shade ;
    T_word16 numPixels ;
    T_word32 textureStep ;
    T_word32 textureOffset ;
    T_byte8 *p_pixel ;
    T_byte8 shift ;

    p_shade = p_slice->p_shade ;
    numPixels = p_slice->numPixels ;
    textureStep = p_slice->textureStep ;
    textureOffset = p_slice->textureOffset ;
    p_pixel = p_slice->p_pixel ;
    shift = p_slice->shift ;
    G_CurrentTexturePos = p_slice->p_texture ;

    if (p_slice->type == 2)  {
        switch (shift)  {
            case 0:
                DrawTransparentColumnAsm1(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 1:
                DrawTransparentColumnAsm2(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 2:
                DrawTransparentColumnAsm4(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 3:
                DrawTransparentColumnAsm8(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 4:
                DrawTransparentColumnAsm16(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 5:
                DrawTransparentColumnAsm32(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 6:
                DrawTransparentColumnAsm64(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 7:
                DrawTransparentColumnAsm128(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 8:
                DrawTransparentColumnAsm256(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
        }
    } else {
        switch (shift)  {
            case 0:
                DrawTranslucentColumnAsm1(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 1:
                DrawTranslucentColumnAsm2(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 2:
                DrawTranslucentColumnAsm4(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 3:
                DrawTranslucentColumnAsm8(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 4:
                DrawTranslucentColumnAsm16(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 5:
                DrawTranslucentColumnAsm32(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 6:
                DrawTranslucentColumnAsm64(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 7:
                DrawTranslucentColumnAsm128(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
            case 8:
                DrawTranslucentColumnAsm256(
                    p_shade,
                    numPixels,
                    textureStep,
                    textureOffset,
                    p_pixel) ;
                break ;
        }
    }
}

#ifndef NDEBUG
T_void IDumpVertFloor(T_void)
{
    T_word16 x ;
    T_word16 index ;
    T_vertFloorInfo *p_vert ;
    for (x=0; x<VIEW3D_WIDTH; x++)   {
        index = G_vertFloorStarts[x] ;
        while (index)  {
            p_vert = G_vertFloorInfo + index ;
            printf("run: %3d   %3d %3d %3d\n", x, p_vert->top, p_vert->bottom, p_vert->sector) ;
            index = p_vert->next ;
        }
    }
}
#endif

/** END OF GRAPHICS-SPECIFIC (i.e. non-server) CODE **/

#endif /** SERVER_ONLY **/

#define TEXTURE_WIDTH   32
#define TEXTURE_HEIGHT  32

static unsigned char G_bytes32[TEXTURE_HEIGHT][TEXTURE_WIDTH][4];
static void *create_texture_32bit(void)
{
    unsigned char *p_bytes = G_bytes32[0][0];
    unsigned char *p = p_bytes;
    int x, y;

    for (y=0; y<TEXTURE_HEIGHT; y++) {
        for (x=0; x<TEXTURE_WIDTH; x++, p+=4) {
            if ((x==0) || (x==(TEXTURE_WIDTH-1)) || (y==0) || (y==(TEXTURE_HEIGHT-1))) {
                //edge of square
                // red
                p[0] = 255;
                p[1] = 255;
                p[2] = 255;
                p[3] = 255;
            } else {
                // internal of square
                // black
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
                p[3] = 255;
            }
        }
    }
    G_bytes32[0][0][1] = 0;
    G_bytes32[0][0][2] = 0;
    G_bytes32[1][0][1] = 0;
    G_bytes32[1][0][2] = 0;
    G_bytes32[0][1][1] = 0;
    G_bytes32[0][1][2] = 0;

    G_bytes32[0][31][1] = 0;
    G_bytes32[0][31][2] = 0;

    G_bytes32[TEXTURE_HEIGHT-1][TEXTURE_WIDTH-1][1] = 0;
    G_bytes32[TEXTURE_HEIGHT-1][TEXTURE_WIDTH-1][2] = 255;

    return p_bytes;
}

static void create_texture(void)
{
    void *p_texture = create_texture_32bit();

    // Create an unpacked alignment 1 structure
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, &G_texture);
    glBindTexture (GL_TEXTURE_2D, G_texture);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#if 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST /* GL_NEAREST */);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST /* GL_NEAREST */);
#elif 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR /* GL_NEAREST */);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR /* GL_NEAREST */);
#elif 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR /* GL_NEAREST */);
#else
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

//    gluBuild2DMipmaps (GL_TEXTURE_2D, 4, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, p_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, p_texture);
    glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void View3DInitOpenGL(void)
{
    create_texture();
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dInitialize
 *-------------------------------------------------------------------------*/
/**
 *  View3dInitialize starts up anything that relates to the 3d view.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dInitialize(T_void)
{
    T_byte8 *p_where ;
    T_word16 i ;

    DebugRoutine("View3dInitialize") ;

    G_3dSegArray = NULL ;
    G_3dSideArray = NULL ;
    G_3dLineArray = NULL ;
    G_3dNodeArray = NULL ;
    G_3dPNodeArray = NULL ;
    G_3dSectorArray = NULL ;
    G_3dVertexArray = NULL ;
    G_3dSegmentSectorArray = NULL ;
    G_3dBlockMapArray = NULL ;
    G_3dBlockMapHeader = NULL ;

#if AA_OPENGL
    View3DInitOpenGL();
#endif
/** Server-only build doesn't need graphics. **/
#ifndef SERVER_ONLY
////    P_doubleBuffer = MemAlloc(MAX_VIEW3D_WIDTH * MAX_VIEW3D_HEIGHT) ;
//    P_doubleBuffer = GRAPHICS_ACTUAL_SCREEN ;

P_doubleBuffer = GRAPHICS_ACTUAL_SCREEN+3*320+4 ;

//P_doubleBuffer = ((char *)0xA0000) ;
    /* Set up a lookup table that is a pointer to each of the lines */
    /* in the double buffer. */
    for (p_where=P_doubleBuffer, i=0;
         i<MAX_VIEW3D_HEIGHT;
         i++, p_where+=MAX_VIEW3D_WIDTH)  {
        G_doublePtrLookup[i] = p_where ;
    }
#endif


//    P_shadeIndex = (T_byte8 *)FileLoad("palindex.dat", &size) ;
//    DebugCheck(size > 100) ;

///    GG_palette = (T_byte8 *)FileLoad("game.pal", &size) ;
///    DebugCheck(size == 768) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dFinish
 *-------------------------------------------------------------------------*/
/**
 *  View3dFinish closes out all information that is stored in memory.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dFinish(T_void)
{
    DebugRoutine("View3dFinish") ;

//    MemFree(P_doubleBuffer) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IQuickSquareRoot
 *-------------------------------------------------------------------------*/
/**
 *  IQuickSquareRoot uses an approximation method to calculate the
 *  square root of a 32 bit number (into a 16 bit number).
 *
 *  @param value -- 32 bit number to take square root of
 *
 *  @return square root calculated
 *
 *<!-----------------------------------------------------------------------*/
T_word16 IQuickSquareRoot(T_word32 value)
{
    T_sword16 i;
    T_word16 result,tmp;
    T_word32 low,high;

    if (value <= 1L)
         return((T_word16)value);

    low = (T_word32)value;
    high = 0L;
    result = 0;

    for (i = 0; i < 16; i++)  {
        result += result;
        high = (high << 2) | ((low >>30) & 0x3);
        low <<= 2;

        tmp = result + result + 1;
        if (high >= tmp)  {
            result++;
            high -= tmp;
        }
    }

    return(result);
}

/*-------------------------------------------------------------------------*
 * Routine:  CalculateDistance
 *-------------------------------------------------------------------------*/
/**
 *  CalculateDistance determines the approximate distance between
 *  two coordinates.
 *
 *  @param x1 -- First X coordinate
 *  @param y1 -- First Y coordinate
 *  @param x2 -- Second X coordinate
 *  @param y2 -- Second Y coordinate
 *
 *  @return Approx. distance between too points.
 *
 *<!-----------------------------------------------------------------------*/
extern T_byte8 G_squareRootTable[16384] ;

T_word16 CalculateDistanceOld2(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2)
{
    x1 -= x2 ;
    if (x1 < 0)
        x1 = -x1 ;

    y1 -= y2 ;
    if (y1 < 0)
        y1 = -y1 ;

    if (x1 > y1)
       return x1 ;

    return y1 ;
}

T_word16 CalculateDistanceOld3(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2)
{
//T_byte8 *p_caller ;
//T_word16 line ;
    T_word16 shift = 0 ;

    DebugRoutine("CalculateDistance") ;


//DebugGetCaller(&p_caller, &line) ;
//printf("Caller: %s (%d)\n", p_caller, line) ;

    x1 -= x2 ;
    if (x1 < 0)
        x1 = -x1 ;

    y1 -= y2 ;
    if (y1 < 0)
        y1 = -y1 ;

    while ((x1 & 0xFF80) || (y1 & 0xFF80))  {
        x1 >>= 1 ;
        y1 >>= 1 ;
        shift++ ;
    }

    DebugEnd() ;

    return((G_squareRootTable[(y1<<7)|x1])<<shift) ;
}

T_word16 CalculateDistance(
             T_sword32 x1,
             T_sword32 y1,
             T_sword32 x2,
             T_sword32 y2)
{
    T_sword32 dx ;
    T_sword32 dy ;
    T_word16 shift = 0 ;

    dx = x1-x2 ;
    if (dx < 0)
       dx = -dx ;

    dy = y2 - y1 ;
    if (dy < 0)
        dy = -dy ;

    while ((dx & 0xFFFF0000) || (dy & 0xFFFF0000))  {
        dx >>= 2 ;
        dy >>= 2 ;
        shift += 2 ;
    }

    /* If delta x is zero, then the distance is from y1 to y2 */
    if (dx == 0)
        return (dy<<shift) ;
    if (dy == 0)
        return (dx<<shift) ;

    return (IQuickSquareRoot((dx*dx) + (dy*dy)) << shift) ;
}

#ifndef NDEBUG
T_word16 DebugIFindSectorNum(T_sword16 x, T_sword16 y)
{
    T_word32 index ;
    T_sword16 column, row ;
    T_word32 closestLine ;
    T_word32 closestDistance ;
    T_word32 distance ;
    T_word16 side ;
    T_word16 flag ;
    T_sword32 xInter ;
    T_sword16 midX, bestMidX ;
    T_word16 sector ;

    /* First, find the block map block that this point is located within. */
    column = (x - G_3dBlockMapHeader->xOrigin) >> 7 ;

    if ((column < 0) || (column > G_3dBlockMapHeader->columns))
        /* Out of bounds, return a bad one. */
        return -1 ;

    row = (y - G_3dBlockMapHeader->yOrigin) >> 7 ;

    if ((row < 0) || (row > G_3dBlockMapHeader->rows))
        /* Out of bounds, return a bad one. */
        return -1 ;
///puts("Here") ;
    flag = 0 ;
    do {
printf("column=%d, row=%d\n", column, row) ;
        /* Inbounds, find the index. */
        index = (row * G_3dBlockMapHeader->columns) + column ;
printf("index = %04X\n", index) ;

        /* Now translate the index into a position in the list of lines. */
printf("blockIndex = %04X\n", G_3dBlockMapHeader->blockIndexes[index]) ;
        index = 1+G_3dBlockMapHeader->blockIndexes[index] ;
printf("index new = %04X\n", index) ;

        /* Now we have a list of lines that end with a -1 line. */
        /* Make the first line the one we will assume to be the closest line. */
        closestLine = -1 ;

        /* Calculate the distance to that line and make it the closest */
        /* up to this point in time. */
        closestDistance = 0x7FFFFFFF ;
        bestMidX = 0x7FFE ;

        /* Compare each of the other lines and find which one is the closest. */
        while (G_3dBlockMapArray[index] != -1)  {
printf("++++ index = %d\n", index) ;
printf("[index] = %d\n", G_3dBlockMapArray[index]) ;
            xInter = IFindIntersectX(G_3dBlockMapArray[index], y) - x ;
            if (xInter < 0)
                distance = -xInter ;
            else
                distance = xInter ;
printf("xInter = %d, distance = %d, closest=%d\n", xInter, distance, closestDistance) ;

            if (distance < 500000)
                flag = 1 ;

            /* Is this line closer? */
            if (distance < closestDistance)  {
                /* Yes, it is. */
                closestDistance = distance ;
                closestLine = G_3dBlockMapArray[index] ;
                bestMidX = (G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].from].x +
                        G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].to].x) >> 1 ;
            } else if (distance == closestDistance)  {
                midX = (G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].from].x +
                        G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].to].x) >> 1 ;
                if (midX < bestMidX)  {
                    closestDistance = distance ;
                    closestLine = G_3dBlockMapArray[index] ;
                    bestMidX = midX ;
                }
            }

            index++ ;
        }
        if (flag == 0)  {
printf("stepping over a column\n") ;
            column++ ;
            if (column > G_3dBlockMapHeader->columns)
                return -1 ;
        }
printf("flag = %d\n", flag) ;
    } while (flag == 0) ;

printf("closestLine=%d\n", closestLine) ;
    /* Now that we have the closest line, let's determine the side we */
    /* are on of this line. */
    side = IOnRightOfLine(x, y, (T_word16)closestLine) ;
printf("Side=%d\n", side) ;

///printf("line=%d, side=%d", closestLine, side) ;
    /* Get the sector of that side and return it. */
    sector = (G_3dSideArray[G_3dLineArray[closestLine].side[side]].sector) ;

    return sector ;
}

#endif

/* Optional compile to output lots about IFindSectorNum */
//#define COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM

#define MAX_FIND_SECTOR_LINES 20

#if 0
T_word16 IFindSectorNum(T_sword16 x, T_sword16 y)
{
    T_word16 node ;
    T_word16 segCount ;
    T_word16 firstSeg ;
    T_sword16 lineSide ;
    T_word16 sector = 0xFFFF ;
    T_3dSegment *p_segment ;
    T_3dSegment *p_closest = NULL ;
    T_word16 distance ;
    T_word16 closestDist = 0x7FFF ;
    T_3dLine *p_line ;
    T_3dVertex *p_vertex ;

    node = G_3dRootBSPNode ;

    /* Traverse the BSP tree to find the appropriate location. */
    while ((node & 0x8000) == 0)  {
        if (View3dOnRightByNodeWithXY(node, x, y))  {
            node = G_3dPNodeArray[node]->right ;
        } else {
            node = G_3dPNodeArray[node]->left ;
        }
    }
    node &= 0x7FFF ;

    /* Get the first segment and the segment count. */
    firstSeg = G_3dSegmentSectorArray[node].firstSeg ;
    segCount = G_3dSegmentSectorArray[node].numSegs ;

    p_segment = &G_3dSegArray[firstSeg] ;

#if 0
    /* Find closest segment. */
    for (; segCount; segCount--, p_segment++)  {
        p_line = G_3dLineArray + p_segment->line ;
        p_vertex = G_3dVertexArray + p_line->from ;
        distance = CalculateEstimateDistance(
                      p_vertex->x,
                      p_vertex->y,
                      x,
                      y) ;
        if (distance < closestDist)  {
            closestDist = distance ;
            p_closest = p_segment ;
        } else {
            p_vertex = G_3dVertexArray + p_line->to ;
            distance = CalculateEstimateDistance(
                          p_vertex->x,
                          p_vertex->y,
                          x,
                          y) ;
            if (distance < closestDist)  {
                closestDist = distance ;
                p_closest = p_segment ;
            }
        }
    }

    if (p_closest)  {
        lineSide = p_closest->lineSide ;
        sector = G_3dSideArray[
                     G_3dLineArray[p_closest->line].side[lineSide]].sector ;
    }
#endif
    for (; segCount; segCount--, p_segment++)  {
        if (View3dOnRightByVerticesXY(p_segment->from, p_segment->to, x, y))  {
            lineSide = p_segment->lineSide ;
            sector = G_3dSideArray[
                         G_3dLineArray[p_segment->line].side[lineSide]].sector ;
            break ;
        }
    }
//printf("point %d, %d returns sector %d\n", x, y, sector) ;
    return sector ;
}
#endif

#if 1
T_word16 IFindSectorNum(T_sword16 x, T_sword16 y)
{
    T_word16 lastLine=0xFFFF, lastSide ;   /* Last line found */
    T_word16 sideFound = 0xFFFE ;          /* Last side found (0xFFFE = none) */
    T_sword16 column, row ;                /* Row and column in block map */
    T_word32 index ;                       /* Index into block map */
    T_word16 sideOfLine ;                  /* Side of line x,y is on */
                                           /* 0 = front, 1 = back */
    T_word16 line ;                        /* Current line number */
    E_Boolean newLineFound ;               /* Flag to say, "this line is closer" */
    T_3dLine *p_line ;                     /* Quick pointer to line. */
    T_word16 numLines = 0 ;
    T_word16 lastLines[MAX_FIND_SECTOR_LINES] ;
    T_byte8 lastSides[MAX_FIND_SECTOR_LINES] ;
    T_word16 i ;

#   ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
    printf("\n\nIFSN: Point %d, %d (%d, %d)\n", x, y, x-G_3dBlockMapHeader->xOrigin, y-G_3dBlockMapHeader->yOrigin) ;
#   endif

    /* First, find the block map block that this point is located within. */
    column = (x - G_3dBlockMapHeader->xOrigin) >> 7 ;

    if ((column < 0) || (column >= G_3dBlockMapHeader->columns))  {
        /* Out of bounds, return a bad one. */
//        DebugCheck(FALSE) ;
        return 0xFFFF ;
    }

    row = (y - G_3dBlockMapHeader->yOrigin) >> 7 ;

    if ((row < 0) || (row >= G_3dBlockMapHeader->rows))  {
        /* Out of bounds, return a bad one. */
//        DebugCheck(FALSE) ;
        return 0xFFFF ;
    }

    while (sideFound == 0xFFFE)  {
#       ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
        printf("IFSN: Block c:%d, r:%d\n", column, row) ;
#       endif
        /* Inbounds, find the index. */
        index = (row * G_3dBlockMapHeader->columns) + column ;

        /* Now translate the index into a position in the list of lines. */
        index = 1+G_3dBlockMapHeader->blockIndexes[index] ;

        /* Loop until we end the list of lines in that block */
        while ((line = G_3dBlockMapArray[index]) != ((T_word16)-1))  {
            /* Get a quick pointer to the line. */
            p_line = G_3dLineArray+line ;

#           ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
            printf("IFSN: Check line %d\n", line) ;  fflush(stdout) ;
#           endif

            /* Which side of that line are we on? */
            sideOfLine = IOnRightOfLine(x, y, line) ;

            /* See if we are right on the line. */
            if (sideOfLine == 2)  {
                /* Yes, we need to choose a real side. */
                /* Equal is the same as worse. */
                /* Is there are previously found side. */
                if (sideFound == 0xFFFE)  {
                    /* Then we are in front. */
                    sideOfLine = 0 ;
                } else {
                    /* Otherwise, choose the opposite side of the */
                    /* last line. */
                    if (lastSide == 0)
                        sideOfLine = 1 ;
                    else
                        sideOfLine = 0 ;
                }
            }

            /* Not sure about this line, don't take it yet. */
            newLineFound = FALSE ;

            /* Only bother with lines that have a side facing the x, y */
            if (p_line->side[sideOfLine] != -1)  {
#                   ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                printf("IFSN: Checking side of line %d (%d of line)\n", line, sideOfLine) ;  fflush(stdout) ;
#                   endif

                /* OK, is this a contending side? */
                if (sideFound != 0xFFFE)  {
                    /* A contender is better if either endpoint is in */
                    /* front of the last line. */
                    if (IOnRightOfLine(
                          G_3dVertexArray[p_line->from].x,
                          G_3dVertexArray[p_line->from].y,
                          lastLine) == lastSide)  {
#                           ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                        printf("IFSN: from point %d, %d on same side\n",
                            G_3dVertexArray[p_line->from].x,
                            G_3dVertexArray[p_line->from].y) ;  fflush(stdout) ;
#                           endif

                        /* From point is closer */
                        newLineFound = TRUE ;
                    } else if (IOnRightOfLine(
                                  G_3dVertexArray[p_line->to].x,
                                  G_3dVertexArray[p_line->to].y,
                                  lastLine) == lastSide)  {
#                           ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                        printf("IFSN: to point %d, %d on same side\n",
                            G_3dVertexArray[p_line->from].x,
                            G_3dVertexArray[p_line->from].y) ;  fflush(stdout) ;
#                           endif

                        /* to point is closer. */
                        newLineFound = TRUE ;
                    }
                } else {
                    /* No one else to contend.  This is the closest line. */
                    newLineFound = TRUE ;
                }
            }

            /* We think we have a better line.  Make sure in front */
            /* of all other lines. */
            for (i=0; i<numLines; i++)  {
#                   ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                printf("IFSN: checking reject line %d side %d\n", lastLines[i], lastSides[i]) ;  fflush(stdout) ;
#                   endif
                /* Check to see if this new line is in front of */
                /* all the old lines. */
                if ((IOnRightOfLine(
                      G_3dVertexArray[p_line->from].x,
                      G_3dVertexArray[p_line->from].y,
                      lastLines[i]) != lastSides[i]) &&
                    (IOnRightOfLine(
                              G_3dVertexArray[p_line->to].x,
                              G_3dVertexArray[p_line->to].y,
                              lastLines[i]) != lastSides[i]))  {
#                       ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                    printf("IFSN: rejected due to line %d side %d\n", lastLines[i], lastSides[i]) ;  fflush(stdout) ;
#                       endif
                    newLineFound = FALSE ;
                    break ;
                }
            }

            /* OK, got a better line.  Replace the old one and */
            /* get its coordinates for a little bit of speed. */
            if (newLineFound)  {
DebugCheck(numLines < MAX_FIND_SECTOR_LINES) ;
                /* Put the old one on the line history */
                if ((lastLine != 0xFFFF) &&
                        (numLines < MAX_FIND_SECTOR_LINES))  {
                    lastLines[numLines] = lastLine ;
                    lastSides[numLines] = (T_byte8)lastSide ;
                    numLines++ ;
                }
                sideFound = p_line->side[sideOfLine] ;
#                   ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                printf("IFSN: better line is %d (on side %d aka %d)\n", line, sideOfLine, sideFound) ;
#                   endif
                lastLine = line ;
                lastSide = sideOfLine ;
            }
            index++ ;
        }

        /* If no side is found yet, then advance to the next row. */
        if (sideFound == 0xFFFE)  {
            row++ ;

            /* If past edge, then don't know.  We're out of here. */
            if (row >= G_3dBlockMapHeader->rows)  {
#               ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
                DebugCheck(FALSE) ;
#               endif
                return 0xFFFF ;
            }
        }
    }

    if (sideFound == 0xFFFF)
        return 0xFFFF ;

#   ifdef COMPILE_OPTION_DEBUG_FIND_SECTOR_NUM
    DebugCheck(G_3dSideArray[sideFound].sector != 0xFFFF) ;
#   endif

    return G_3dSideArray[sideFound].sector ;
}
#endif

#if 0
T_word16 IFindSectorNum(T_sword16 x, T_sword16 y)
{
    T_word16 nodeIndex ;
    T_word16 lineSide ;

    nodeIndex = G_3dRootBSPNode ;

    for (;;)  {
        if (nodeIndex & 0x8000)  {
            /* Found a segment sector, draw that sector. */
            nodeIndex &= 0x7FFF ;

printf("IFSN: %d, %d near %d %d\n",
            x,
            y,
            G_3dSegArray[G_3dSegmentSectorArray[nodeIndex].firstSeg].line,
            G_3dSegArray[G_3dSegmentSectorArray[nodeIndex].firstSeg].lineSide) ;

            lineSide = G_3dSegArray[G_3dSegmentSectorArray[nodeIndex].firstSeg].lineSide ;
DebugCheck(G_3dSideArray[G_3dLineArray[G_3dSegArray[G_3dSegmentSectorArray[nodeIndex].firstSeg].line].side[lineSide]].sector < G_Num3dSectors) ;
            return G_3dSideArray[G_3dLineArray[G_3dSegArray[G_3dSegmentSectorArray[nodeIndex].firstSeg].line].side[lineSide]].sector ;
        } else {
            /* Are we on the right side of this node? */
            if (View3dOnRightByNodeWithXY(nodeIndex, x, y))  {
                nodeIndex = G_3dPNodeArray[nodeIndex]->right ;
            } else {
                nodeIndex = G_3dPNodeArray[nodeIndex]->left ;
            }
        }
    }
}
#endif

#if 0
T_word16 IFindSectorNumOld(T_sword16 x, T_sword16 y)
{
    T_word32 index ;
    T_sword16 column, row ;
    T_word32 closestLine ;
    T_word32 closestDistance ;
    T_word32 distance ;
    T_word16 side ;
    T_word16 flag ;
    T_sword32 xInter ;
    T_sword16 midX, bestMidX ;
    T_word16 sector ;

///puts("Here") ;
    flag = 0 ;
    do {
        /* Inbounds, find the index. */
        index = (row * G_3dBlockMapHeader->columns) + column ;

        /* Now translate the index into a position in the list of lines. */
        index = 1+G_3dBlockMapHeader->blockIndexes[index] ;

        /* Now we have a list of lines that end with a -1 line. */
        /* Make the first line the one we will assume to be the closest line. */
        closestLine = -1 ;

        /* Calculate the distance to that line and make it the closest */
        /* up to this point in time. */
        closestDistance = 0x7FFFFFFF ;
        bestMidX = 0x7FFE ;

        /* Compare each of the other lines and find which one is the closest. */
//printf("Checking: ") ;
        while (G_3dBlockMapArray[index] != -1)  {
            xInter = IFindIntersectX(G_3dBlockMapArray[index], y) - x ;
            if (xInter < 0)
                distance = -xInter ;
            else
                distance = xInter ;
//printf("(line %d @ %d) ", G_3dBlockMapArray[index], xInter) ;
///            distance = IDetermineDistToLine(x, y, G_3dBlockMapArray[index]) ;
///printf("distance=%d\n", distance) ;

            if (distance < 500000)
                flag = 1 ;

            /* Is this line closer? */
            if (distance < closestDistance)  {
                /* Yes, it is. */
                closestDistance = distance ;
                closestLine = G_3dBlockMapArray[index] ;
                bestMidX = (G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].from].x +
                        G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].to].x) >> 1 ;
            } else if (distance == closestDistance)  {
                midX = (G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].from].x +
                        G_3dVertexArray[
                           G_3dLineArray[
                               G_3dBlockMapArray[
                                   index]].to].x) >> 1 ;
                if (midX < bestMidX)  {
                    closestDistance = distance ;
                    closestLine = G_3dBlockMapArray[index] ;
                    bestMidX = midX ;
                }
            }

            index++ ;
        }
//puts(".") ;
        if (flag == 0)  {
            column++ ;
            if (column > G_3dBlockMapHeader->columns)
                return -1 ;
///printf("--> step over.") ;
        }
    } while (flag == 0) ;

    /* Now that we have the closest line, let's determine the side we */
    /* are on of this line. */
    side = IOnRightOfLine(x, y, (T_word16)closestLine) ;

    /* If this is a bad side, return a 'outside' sector value. */
    if (G_3dLineArray[closestLine].side[side] == -1)
        return 0xFFFF ;

///printf("line=%d, side=%d", closestLine, side) ;
    /* Get the sector of that side and return it. */
//#ifndef NDEBUG
if (side >= 2)  {
   return 0xFFFF ;
   printf("side = %d\n", side) ;
}
DebugCheck(side < 2) ;

if (closestLine >= G_Num3dLines)  {
   return 0xFFFF ;
   printf("closestLine = %d, G_numlines=%d\n", closestLine, G_Num3dLines) ;
}
DebugCheck(closestLine < G_Num3dLines) ;

if (G_3dLineArray[closestLine].side[side] >= G_Num3dSides)  {
   return 0xFFFF ;
   printf("num sides = %d, closestLine=%d, side=%d\n",
       G_3dLineArray[closestLine].side[side], closestLine, side) ;
}
DebugCheck(G_3dLineArray[closestLine].side[side] < G_Num3dSides) ;
//#endif

    sector = (G_3dSideArray[G_3dLineArray[closestLine].side[side]].sector) ;

#ifndef NDEBUG
if (sector >= G_Num3dSectors)  {
   return 0xFFFF ;
  printf("Bad sector %d\n", sector) ;
  printf("X: %d,  Y: %d\n", x, y) ;
  printf("From (side=%d), (closestLine=%d), (sideArray=%d)\n",
      side,
      closestLine,
      G_3dLineArray[closestLine].side[side]) ;
  DebugIFindSectorNum(x, y) ;
}
#endif

    DebugCheck(G_3dLineArray[closestLine].side[side] >= 0) ;
    DebugCheck(sector < G_Num3dSectors) ;
    return sector ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  View3dOnRightOfLine
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 IOnRightOfLine(T_sword16 x, T_sword16 y, T_word16 line)
{
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;
    T_word16 from, to ;
    T_sword32 calc ;

    from = G_3dLineArray[line].from ;
    to = G_3dLineArray[line].to ;

    p_vertex = &G_3dVertexArray[from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;
    p_vertex = &G_3dVertexArray[to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    calc = ((y1 - y2) * (x - x2)) - ((x1 - x2) * (y - y2)) ;

    if (calc < 0)
        return 0 ;
    if (calc > 0)
        return 1 ;

    return 2 ;
}

T_sword32 IFindIntersectX(T_word16 line, T_sword16 y)
{
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;
    T_word16 from, to ;
    T_sword32 deltaX, deltaY ;
    T_sword32 xInter ;
    T_sword32 dy ;

    from = G_3dLineArray[line].from ;
    to = G_3dLineArray[line].to ;

    p_vertex = &G_3dVertexArray[from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;
    p_vertex = &G_3dVertexArray[to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    deltaY = y2-y1 ;

    if (deltaY != 0)  {
        if (((y2 <= y) && (y1 >= y)) ||
            ((y1 <= y) && (y2 >= y)))  {
            deltaX = x2-x1 ;

            dy = y - y1 ;
            xInter = x1 + ((deltaX * dy) / deltaY) ;
//ver2            xInter = MultAndDivideAsm(deltaX, dy, deltaY) ;
//ver2            xInter += x1 ;
        } else {
            xInter = 0x7FFFFFFF ;
        }
    } else {
        xInter = 0x7FFFFFFF ;
    }

    return xInter ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dAllocateObject
 *-------------------------------------------------------------------------*/
/**
 *  View3dAllocateObject either finds an old object slot that is no
 *  longer being used or allocates a fresh one.  The number to the object
 *  is returned.
 *
 *  NOTE: 
 *  None of the fields are initialized, the object is just allocated.
 *  The fields need to be setup before another frame is drawn.
 *
 *  @return Pointer to the newly created object.
 *
 *<!-----------------------------------------------------------------------*/
T_3dObject *View3dAllocateObject(T_void)
{
    T_3dObject *p_obj ;

    DebugRoutine("View3dAllocateObject") ;

    /* Are we past the maximum? */
    if (G_Num3dObjects < VIEW3D_MAX_OBJECTS)  {
        /* Yes, we can add one.  allocate memory for that slot */
        /* and return it. */
        p_obj = MemAlloc(sizeof(T_3dObject)) ;
        DebugCheck(p_obj != NULL) ;

        /* Clear the object. */
        memset(p_obj, 0, sizeof(T_3dObject)) ;

        G_Num3dObjects++ ;

        /* Make sure the links are null. */
        p_obj->nextObj = p_obj->prevObj = NULL ;
    } else {
        /* No, we're out of luck.  Return nothing. */
        p_obj = NULL ;
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;

    return p_obj ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dFreeObject
 *-------------------------------------------------------------------------*/
/**
 *  View3dFreeObject deletes an object and updates the links assocaited
 *  with it.
 *
 *  @param p_obj -- pointer to object to free
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dFreeObject(T_3dObject *p_obj)
{
    DebugRoutine("View3dFreeObject") ;
    DebugCheck(p_obj->prevObj == NULL) ;
    DebugCheck(p_obj->nextObj == NULL) ;

    /* Free from memory now free from links. */
    MemFree(p_obj) ;

    /* Decrement the count of objects. */
    G_Num3dObjects-- ;

    DebugEnd() ;
}

T_void View3dCheckObjectListEmpty(T_void)
{
    DebugRoutine("View3dCheckObjectListEmpty") ;
    DebugCheck(G_Num3dObjects == 0) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetObjectAtColumn
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetObjectAtColumn is a utility routine that checks the sorted
 *  list of objects to see if there is an object at the given row.  The
 *  routine takes in the an integer of where in the list it is, a place
 *  to store the found object, and the column of where it is to search.
 *  It returns where in the list is the next object, or 0xFFFF if at end.
 *
 *  @param objPos -- Position in list to search.
 *  @param p_obj -- Object found (NULL if none)
 *  @param column -- Column on screen to search.
 *
 *  @return Next position to be searched, or
 *      0xFFFF if at end.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 View3dGetObjectAtColumn(
             T_word16 objPos,
             T_3dObject **p_obj,
             T_word16 column)
{
    T_word16 nextObjPos ;
    T_word16 colCount ;

    DebugRoutine("View3dGetObjectAtColumn") ;

    *p_obj = NULL ;

    /* How many items in this column? */
    colCount = G_objectColStart[column] ;
    if (colCount == 0xFFFF)  {
        DebugEnd() ;
        return 0xFFFF ;
    }

    while (G_objectColRunList[colCount].next != objPos)  {
        colCount = G_objectColRunList[colCount].next ;
        if (colCount == 0xFFFF)  {
            DebugEnd() ;
            return 0xFFFF ;
        }
    }

    *p_obj = G_objectColRunList[colCount].p_runInfo->p_obj ;
    nextObjPos = colCount ;

#if 0
    /* Make sure we are still in the list. */
    if (objPos < colCount)  {
        /* Go ahead and figure what is next in the list. */
        nextObjPos = objPos+1 ;

        /* If past the end, say "no more" */
        if (nextObjPos >= colCount)
            nextObjPos = 0xFFFF ;

        /* See if we are past the far wall. */
        /* Get a pointer to the current object list item */
        p_objRun = &G_objectColRun[column][objPos] ;

        /* Check to see if the object straddles the column. */
        /* Yep.  Store object number and stop looping. */
        *p_obj = p_objRun->p_runInfo->p_obj ;

        objPos = nextObjPos ;
    } else {
        /* We were called out of the list.  Tell them "no more" */
        nextObjPos = 0xFFFF ;
    }
#endif

    DebugEnd() ;

#ifndef NDEBUG
    if (*p_obj)
        DebugCheck(strcmp((*p_obj)->tag, "Obj")==0) ;
#endif
    return nextObjPos ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetObjectAtXY
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetObjectAtXY     is a utility routine that checks the sorted
 *  list of objects to see if there is an object at the given X&Y.  The
 *  routine takes in the an integer of where in the list it is, a place
 *  to store the found object, and the X & Y  of where it is to search.
 *  It returns where in the list is the next object, or 0xFFFF if at end.
 *
 *  @param objPos -- Position in list to search.
 *  @param p_obj -- Object found (NULL if none)
 *  @param x -- X (column) on screen to search
 *  @param y -- Y (row)    on screen to search
 *
 *  @return Next position to be searched, or
 *      0xFFFF if at end.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 View3dGetObjectAtXY(
             T_word16 objPos,
             T_3dObject **p_obj,
             T_word16 x,
             T_word16 y)
{
    T_word16 nextObjPos ;
    T_3dObjectColRun *p_objRun ;
    T_word16 colCount ;
    E_Boolean search = TRUE ;
    T_3dObjectRunInfo *p_runInfo ;
    T_word32 scrheight ;
    T_word32 imageheight ;
    T_word32 scrFromTop ;
    T_word32 imageFromTop ;
    T_byte8 *p_texture ;
    T_pictureRaster *p_entry ;

    DebugRoutine("View3dGetObjectAtXY") ;

    /* How many items in this x? */
    *p_obj = NULL ;

    do {
        nextObjPos = 0 ;

        /* How many items in this column? */
        colCount = G_objectColStart[x] ;
        if (colCount == 0xFFFF)  {
            DebugEnd() ;
            return 0xFFFF ;
        }

        /* Find previous item in list (since front to back) */
        while (G_objectColRunList[colCount].next != objPos)  {
            colCount = G_objectColRunList[colCount].next ;
            if (colCount == 0xFFFF)  {
                nextObjPos = 0xFFFF ;
                break ;
            }
        }

        if (nextObjPos == 0xFFFF)
            break ;

        nextObjPos = colCount ;

        /* See if we are past the far wall. */
        /* Get a pointer to the current object list item */
        p_objRun = &G_objectColRunList[nextObjPos] ;

        /* Yep.  Store object number and stop looping. */
        if ((p_objRun->p_runInfo->bottom >= y) &&
            (p_objRun->p_runInfo->top <= y))  {
            /* Search no more, we have found an object's slice */
            /* under the x & y location. */
            p_runInfo = p_objRun->p_runInfo ;
            scrheight = p_runInfo->realBottom - p_runInfo->top ;
            /* Avoid divide by zero problems. */
            if (scrheight == 0)
                scrheight = 1 ;
            imageheight = PictureGetHeight(p_runInfo->p_picture) ;
            scrFromTop = y - p_runInfo->top ;
            imageFromTop =(imageheight * scrFromTop) / scrheight ;
            /* imageFromTop is how far down the image where */
            /* we are pointing.  Examine the compressed bitmap. */
            p_entry = &((T_pictureRaster *)
                          (p_runInfo->p_picture))[p_objRun->column] ;
            p_texture = &p_runInfo->p_picture[
                            p_entry->offset-p_entry->start] ;
            if ((imageFromTop >= p_entry->start) &&
                (imageFromTop <= p_entry->end))  {
                /* Now check the pixel under the location. */
                if (p_texture[imageFromTop-p_entry->start] != 0)  {
                    *p_obj = p_runInfo->p_obj ;
                    search = FALSE ;
                } else {
                    /* Pixel is clear, so don't take, move on. */
                    objPos = nextObjPos ;
                }
            } else {
                /* Out of bounds, go on. */
                objPos = nextObjPos ;
            }

        } else {
            /* Progress to the next location. */
            objPos = nextObjPos ;
        }
    } while (search == TRUE) ;

    DebugEnd() ;

    return nextObjPos ;
}

/*-------------------------------------------------------------------------*
 * Routine:  CalculateEstimateDistance
 *-------------------------------------------------------------------------*/
/**
 *  CalculateEstimateDistance determines the quick and approximate
 *  distance between two points.
 *
 *  @param x1 -- First X coordinate
 *  @param y1 -- First Y coordinate
 *  @param x2 -- Second X coordinate
 *  @param y2 -- Second Y coordinate
 *
 *  @return Approx. distance between too points.
 *
 *<!-----------------------------------------------------------------------*/
extern T_byte8 G_squareRootTable[16384] ;

T_word16 CalculateEstimateDistance(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2)
{
    x1 -= x2 ;
    if (x1 < 0)
        x1 = -x1 ;

    y1 -= y2 ;
    if (y1 < 0)
        y1 = -y1 ;

    if (x1 > y1)
       return x1 ;

    return y1 ;
/*
    T_sword16 deltaX, deltaY ;
    T_word16 shift = 0 ;

    deltaX = x1 - x2 ;
    if (deltaX < 0)
        deltaX = -deltaX ;

    deltaY = y1 - y2 ;
    if (deltaY < 0)
        deltaY = -deltaY ;

    if (deltaX > deltaY)  {
        if (deltaX > (deltaY << 1))
            return deltaX ;
        else
            return (deltaX + (deltaX >> 1)) ;
    }

    if (deltaY > (deltaX << 1))
        return deltaY ;

    return (deltaY + (deltaY >> 1)) ;
*/
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dAddObject
 *-------------------------------------------------------------------------*/
/**
 *  View3dAddObject  attaches a new object to the list of objects in
 *  the 3d world.
 *
 *  @param p_obj -- Object to bring into world.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dAddObject(T_3dObject *p_obj)
{
    DebugRoutine("View3dAddObject") ;
    DebugCheck (p_obj != NULL) ;
    DebugCheck(p_obj->prevObj == NULL) ;
    DebugCheck(p_obj->nextObj == NULL) ;

    if (p_obj != NULL)  {
        /* Update links. */
        p_obj->prevObj = G_Last3dObject ;
        p_obj->nextObj = NULL ;
        if (G_Last3dObject != NULL)
            G_Last3dObject->nextObj = p_obj ;
        if (G_First3dObject == NULL)
            G_First3dObject = p_obj ;
        G_Last3dObject = p_obj ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dRemoveObject
 *-------------------------------------------------------------------------*/
/**
 *  View3dRemoveObject detaches an object from the list of objects in
 *  the 3d world.
 *
 *  @param p_obj -- Object to bring into world.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dRemoveObject(T_3dObject *p_obj)
{
    DebugRoutine("View3dRemoveObject") ;
    DebugCheck (p_obj != NULL) ;

    /* Remove it from the links. */
    /* Remove from previous link. */
    if (p_obj->prevObj != NULL)
        p_obj->prevObj->nextObj = p_obj->nextObj ;
    else
        G_First3dObject = p_obj->nextObj ;

    /* Remove next link. */
    if (p_obj->nextObj != NULL)
        p_obj->nextObj->prevObj = p_obj->prevObj ;
    else
        G_Last3dObject = p_obj->prevObj ;

    p_obj->nextObj = p_obj->prevObj = NULL ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dRemapSectors
 *-------------------------------------------------------------------------*/
/**
 *  View3dRemapSectors is called after a map is loaded.  This routine
 *  sets up optimization for floor and ceiling drawing by finding similar
 *  ceilings and floors and declares them to be the "same" ceiling and
 *  pattern.
 *
 *  NOTE: 
 *  This routine MUST be called before the wall textures are locked
 *  into memory.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dRemapSectors(T_void)
{
    T_word16 i, j ;
    T_3dSector *p_i, *p_j ;

    DebugRoutine("View3dRemapSectors") ;

    /* Allocate memory for the floor remapping. */
    G_remapSectorFloorArray = MemAlloc(sizeof(T_word16) * G_Num3dSectors) ;
    DebugCheck(G_remapSectorFloorArray != NULL) ;

    /* Allocate memory for the ceiling remapping. */
    G_remapSectorCeilingArray = MemAlloc(sizeof(T_word16) * G_Num3dSectors) ;
    DebugCheck(G_remapSectorCeilingArray != NULL) ;

    /* Determine similar floors. */
    p_i = G_3dSectorArray ;
    for (i=0; i<G_Num3dSectors; i++, p_i++)  {
        G_remapSectorFloorArray[i] = i ;
        p_j = G_3dSectorArray ;
        for (j=0; j<i; j++, p_j++)  {
            if ((p_i->floorHt == p_j->floorHt) &&
                (strncmp(p_i->floorTx, p_j->floorTx, 8) == 0) &&
                (p_i->light == p_j->light))  {
                G_remapSectorFloorArray[i] = j ;
                break ;
            }
        }
    }

    /* Determine similar ceilings. */
    p_i = G_3dSectorArray ;
    for (i=0; i<G_Num3dSectors; i++, p_i++)  {
        G_remapSectorCeilingArray[i] = i ;
        p_j = G_3dSectorArray ;
        for (j=0; j<i; j++, p_j++)  {
            if ((p_i->ceilingHt == p_j->ceilingHt) &&
                (strncmp(p_i->ceilingTx, p_j->ceilingTx, 8) == 0) &&
                (p_i->light == p_j->light))  {
                G_remapSectorCeilingArray[i] = j ;
                break ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dUnmapSectors
 *-------------------------------------------------------------------------*/
/**
 *  View3dUnmapSectors frees memory used by View3dRemapSectors.
 *  It is usually called when a map ends.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dUnmapSectors(T_void)
{
    DebugRoutine("View3dUnmapSectors") ;

    MemFree(G_remapSectorFloorArray) ;
    MemFree(G_remapSectorCeilingArray) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IAddChainedObjects
 *-------------------------------------------------------------------------*/
/**
 *  IAddChainedObjects goes through the list of objects to be considered
 *  for drawing and adds to the list the any chained objects that may be
 *  a part of the original object.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IAddChainedObjects(T_void)
{
    T_word16 i ;
    T_word16 num ;
    T_3dObjectRun *p_run ;
    T_3dObject *p_obj ;
    T_word16 endPos ;
    T_3dObject *p_chained ;

    T_word16 part ;
    T_bodyPart *p_chainedList ;
    T_word16 angle ;
    T_word16 stance ;
    T_word16 frame ;
    T_word16 oldOrdering ;

    static T_sword16 ordering[8][MAX_BODY_PARTS] = {
        { 0, -1, -3, -3,  0, -2, -4},
        { 0, -1,  1, -3,  0, -2,  2},
        { 0, -1,  1, -3,  0, -2,  2},
        { 0, -2,  1, -3,  0, -1,  2},
        { 0, -1,  1,  2,  0,  3,  2},
        { 0, -1, -2,  2,  0,  1, -3},
        { 0, -1, -2,  2,  0,  1, -3},
        { 0, -1, -3,  1,  0, -2, -4},
    } ;

    DebugRoutine("IAddChainedObjects") ;

    /* Get the number of objects in the list since the list MIGHT */
    /* change in number. */
    endPos = num = G_objectCount ;

    /* Go through the list of objects already scheduled to be drawn. */
    for (i=0, p_run=G_objectRun; i<num; i++, p_run++)  {
        /* Check each object and determine if it has a chained object. */
        p_obj = p_run->runInfo.p_obj ;

        p_chained = ObjectGetChainedObjects(p_obj) ;
        if (p_chained)  {
            /* Yes, there is a chain. */
            /* Is the chain for piecewise, or just a chain. */
            if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
                /* Yes, it is piecewise. */
                /* Compute the angle from the object to the player. */
	            angle = MathArcTangent(
                            PlayerGetX16() - ObjectGetX16(p_obj),
                            ObjectGetY16(p_obj) - PlayerGetY16())
                                + ObjectGetAngle(p_obj) ;
                angle = (((T_word16)(angle + ((INT_ANGLE_45/2)-1))) >> 13) ;

                if (ObjTypeIsLowPiecewiseRes())
                    if ((angle >= 1) && (angle <= 3))
                        angle = 8-angle ;

                stance = ObjectGetStance(p_obj) ;
                frame = ObjectGetFrame(p_obj) ;

                if (((stance == STANCE_ATTACK) ||
                     (stance == STANCE_HURT)) &&
                    (frame == 0))  {
                    oldOrdering = ordering[0][BODY_PART_LOCATION_WEAPON] ;
                    ordering[0][BODY_PART_LOCATION_WEAPON] = 3 ;
                }
                /* Calculate what pieces go in what order. */
                p_chainedList = (T_bodyPart *)p_chained ;
                for (part=1; part<MAX_BODY_PARTS; part++)  {
                    if (p_chainedList[part].p_obj)  {
                        p_chainedList[part].p_obj->objMove = p_obj->objMove ;
                        IFindObject(p_chainedList[part].p_obj) ;
                        G_objectRun[endPos].runInfo.distance =
                            p_run->runInfo.distance + ordering[angle][part] ;
                        endPos++ ;
                    }
                }

                if (((stance == STANCE_ATTACK) ||
                     (stance == STANCE_HURT)) &&
                    (frame == 0))  {
                    ordering[0][BODY_PART_LOCATION_WEAPON] = oldOrdering ;
                }
            } else {
                p_chained->objMove = p_obj->objMove ;
                /* No, it is not piecewise.  Just a normal chain. */
                /* Add the object to the drawing list. */
                IFindObject(p_chained) ;

                /* Modify the distance to be a bit closer */
                /* than the root. */
                G_objectRun[endPos].runInfo.distance =
                     p_run->runInfo.distance-1 ;
                endPos++ ;
            }
        }
    }

    DebugEnd() ;
}

#ifndef NDEBUG
/* TESTING */
static T_void ITestMinMax(T_word16 where)
{
#if 0
    T_word16 x ;
    DebugRoutine("ITestMinMax") ;

    for (x=0; x<VIEW3D_WIDTH; x++)  {
        if (G_minY[x] > VIEW3D_HEIGHT)  {
            printf("%d) G_minY[%d] = %d, out of bounds!\n", where, x, G_minY[x]) ;
            fprintf(stderr, "%d) G_minY[%d] = %d, out of bounds!\n", where, x, G_minY[x]) ;
            fflush(stdout) ;
            DebugCheck(FALSE) ;
        }
        if (G_maxY[x] > VIEW3D_HEIGHT)  {
            printf("%d) G_maxY[%d] = %d, out of bounds!", where, x, G_maxY[x]) ;
            fprintf(stderr, "%d) G_maxY[%d] = %d, out of bounds!", where, x, G_maxY[x]) ;
            fflush(stdout) ;
            DebugCheck(FALSE) ;
        }
    }

    DebugEnd() ;
#endif
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  View3dSetDarknessAdjustment
 *-------------------------------------------------------------------------*/
/**
 *  View3dSetDarknessAdjustment sets how much light to add to all
 *  floors and ceilings.  If negative, makes darker.
 *
 *  @param darkAdjust -- 0 = Normal, 63=perfect night vision,
 *      -63=Blind.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dSetDarknessAdjustment(T_sbyte8 darkAdjust)
{
    DebugRoutine("View3dSetDarknessAdjustment") ;

    G_darknessAdjustment = darkAdjust ;

    DebugEnd() ;
}

/* LES: 05/27/96 Created */
T_word16 View3dFindClosestLine(T_sword16 x, T_sword16 y)
{
    T_word32 dist ;
    T_word32 closestDist = 0xFFFFFFFF ;
    T_word16 closest = 0xFFFF ;
    T_word16 i ;

    DebugRoutine("View3dFindClosestLine") ;

    for (i=0; i<G_Num3dLines; i++)  {
        dist = IDetermineDistToLine(x, y, i) ;
        if (dist < closestDist)  {
            closest = i ;
            closestDist = dist ;
        }
    }

    DebugEnd() ;

    return closest ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDetermineDistToLine
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_word32 IDetermineDistToLine(T_sword16 x, T_sword16 y, T_word16 lineNum)
{
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;
    T_word16 from, to ;
    T_word16 angle ;
    T_sword32 dist ;
    T_sword32 xA, xB ;

    from = G_3dLineArray[lineNum].from ;
    to = G_3dLineArray[lineNum].to ;

    p_vertex = &G_3dVertexArray[from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;
    p_vertex = &G_3dVertexArray[to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    /* Find the angle of the line. */
    angle = -MathArcTangent(x2-x1, y2-y1) ;

    /* Find one of the X's of the line now. */
    xA = ((x1-(T_sword32)x) * MathCosineLookup(angle)) -
           ((y1-(T_sword32)y) * MathSineLookup(angle)) ;
    xA >>= 16 ;

    /* Find the other. */
    xB = ((x2-(T_sword32)x) * MathCosineLookup(angle)) -
           ((y2-(T_sword32)y) * MathSineLookup(angle)) ;
    xB >>= 16 ;

///printf("Line %d, xA=%d, xB=%d, angle=%d, dy=%d, dx=%d -- ", lineNum, xA, xB, angle, y2-y1, x2-x1) ;
    /* Are we "straddling" the X's? */
    if (((xA < 0) && (xB >= 0)) || ((xA >= 0) && (xB < 0)))  {
///puts("Yes") ;
        /* Rotate the from point around the given x, y point and thus */
        /* determine the distance to the line. */
        dist = ((x1-(T_sword32)x) * MathSineLookup(angle)) +
               ((y1-(T_sword32)y) * MathCosineLookup(angle)) ;
        dist >>= 16 ;

        /* Make sure it is positive. */
        if (dist < 0)
            dist = -dist ;
    } else {
///puts("No") ;
        /* Make it a big number! */
        dist = 1000000 ;
    }

    return dist ;
}

#ifdef COMPILE_OPTION_ALLOW_SHIFT_TEXTURES
T_void View3dTellMouseAt(T_sword16 x, T_sword16 y)
{
    G_mouseAtX = x - VIEW3D_UPPER_LEFT_X ;
    G_mouseAtY = y - VIEW3D_UPPER_LEFT_Y ;
}

T_word16 View3dGetTextureSideNum(T_void)
{
    return G_textureSideNum ;
}
#endif

T_word16 View3dGetSectorSide(T_word16 lineNum, T_sword16 x, T_sword16 y)
{
    T_3dLine *p_line ;
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_line = &G_3dLineArray[lineNum] ;
    p_vertex = &G_3dVertexArray[p_line->from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;

    p_vertex = &G_3dVertexArray[p_line->to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    if (((x1-x2)*(y - y2)) <
        ((y1-y2) * (x - x2)))
        return G_3dSideArray[p_line->side[1]].sector ;
    return G_3dSideArray[p_line->side[0]].sector ;
}

T_word16 View3dGetSide(T_word16 lineNum, T_sword16 x, T_sword16 y)
{
    T_3dLine *p_line ;
    T_3dVertex *p_vertex ;
    T_sword32 x1 ;
    T_sword32 y1 ;
    T_sword32 x2 ;
    T_sword32 y2 ;

    p_line = &G_3dLineArray[lineNum] ;
    p_vertex = &G_3dVertexArray[p_line->from] ;
    x1 = p_vertex->x ;
    y1 = p_vertex->y ;

    p_vertex = &G_3dVertexArray[p_line->to] ;
    x2 = p_vertex->x ;
    y2 = p_vertex->y ;

    if (((x1-x2)*(y - y2)) <
        ((y1-y2) * (x - x2)))  {
        return p_line->side[1] ;
    } else {
        return p_line->side[0] ;
    }
}

T_word16 View3dFindSide(T_sword16 x, T_sword16 y)
{
    T_word16 line ;

    line = View3dFindClosestLine(x, y) ;
    if (line == 0xFFFF)
        return line ;

    return View3dGetSide(line, x, y) ;
}

#if defined(WATCOM)
#pragma aux  DrawObjectColumnAsm   parm	[EBX] [ECX] [EDX] [ESI] [EDI]
#endif
T_void DrawObjectColumnAsm(
           T_byte8 *p_shade, // ebx
           T_word32 count, // ecx
           T_sword32 textureStep, // edx
           T_sword32 textureOffset, // esi
           T_byte8 *p_pixel) ; // edi


#ifdef NO_ASSEMBLY
T_void DrawObjectColumnAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_word32 x;
    T_byte8 c;

    while (count) {
        x = ((textureOffset>>16) & 0xFFFF);
        if (x >= G_objColumnStart)
            break;
        p_pixel += 320;
        textureOffset += textureStep;
        count--;
    }
    while (count) {
        x = (textureOffset>>16) & 0xFFFF;
        if (x > G_objColumnEnd)
            break;
        c = G_CurrentTexturePos[x];
        if (c)
            *p_pixel = p_shade[c];
        p_pixel += 320;
        textureOffset += textureStep;
        count--;
    }
}

T_void DrawTranslucentObjectColumnAsm(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_word32 x;
    T_byte8 c;

    while (count) {
        x = ((textureOffset>>16) & 0xFFFF);
        if (x >= G_objColumnStart)
            break;
        p_pixel += 320;
        textureOffset += textureStep;
        count--;
    }
    while (count) {
        x = (textureOffset>>16) & 0xFFFF;
        if (x > G_objColumnEnd)
            break;
        c = G_CurrentTexturePos[x];
        if (c)
            *p_pixel = G_translucentTable[p_shade[c]][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
        count--;
    }
}

T_void DrawTextureColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;
    c = p_shade[G_CurrentTexturePos[0]];
    while (count--) {
        *p_pixel = c;
        p_pixel += 320;
    }
}

T_void DrawTextureColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x01]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x03]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x07]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x0F]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x1F]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x3F]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0x7F]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *p_pixel = p_shade[G_CurrentTexturePos[(textureOffset>>16)&0xFF]];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c = G_CurrentTexturePos[0];

    if (c) {
        c = p_shade[c];
        while (count--) {
            *(p_pixel) = c;
            p_pixel+=320;
            textureOffset += textureStep;
        }
    }
}

T_void DrawTransparentColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x01];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x03];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x07];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x0F];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x1F];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x3F];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0x7F];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTransparentColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset>>16) & 0xFF];
        if (c)
            *(p_pixel) = p_shade[c];
        p_pixel+=320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    c = G_CurrentTexturePos[0];
    if (c) {
        while (count--) {
            *p_pixel = G_translucentTable[c][*p_pixel];
            p_pixel += 320;
            textureOffset += textureStep;
        }
    }
}

T_void DrawTranslucentColumnAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x01];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x03];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x07];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x0F];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x1F];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x3F];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0x7F];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTranslucentColumnAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 textureStep,
           T_sword32 textureOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(textureOffset >> 16) & 0xFF];
        if (c)
            *p_pixel = G_translucentTable[c][*p_pixel];
        p_pixel += 320;
        textureOffset += textureStep;
    }
}

T_void DrawTextureRowAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(xOffset >> 16) & G_textureAndX]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 1) | ((yOffset >> 16) & 0x01)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 2) | ((yOffset >> 16) & 0x03)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 3) | ((yOffset >> 16) & 0x07)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 4) | ((yOffset >> 16) & 0x0F)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 5) | ((yOffset >> 16) & 0x1F)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 6) | ((yOffset >> 16) & 0x3F)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 7) | ((yOffset >> 16) & 0x7F)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTextureRowAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    while (count--) {
        *(p_pixel++) = p_shade[G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 8) | ((yOffset >> 16) & 0xFF)]];
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm1(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(xOffset >> 16) & G_textureAndX];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm2(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 1) | ((yOffset >> 16) & 0x01)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm4(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 2) | ((yOffset >> 16) & 0x03)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm8(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 3) | ((yOffset >> 16) & 0x07)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm16(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 4) | ((yOffset >> 16) & 0x0F)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm32(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 5) | ((yOffset >> 16) & 0x1F)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm64(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 6) | ((yOffset >> 16) & 0x3F)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm128(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 7) | ((yOffset >> 16) & 0x7F)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}

T_void DrawTransRowAsm256(
           T_byte8 *p_shade,
           T_word32 count,
           T_sword32 xOffset,
           T_sword32 yOffset,
           T_byte8 *p_pixel)
{
    T_byte8 c;

    while (count--) {
        c = G_CurrentTexturePos[(((xOffset >> 16) & G_textureAndX) << 8) | ((yOffset >> 16) & 0xFF)];
        if (c)
            *p_pixel  = p_shade[c];
        p_pixel++;
        xOffset += G_textureStepX;
        yOffset += G_textureStepY;
    }
}
#endif

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  3D_VIEW.C
 *-------------------------------------------------------------------------*/
