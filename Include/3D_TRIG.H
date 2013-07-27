/****************************************************************************/
/*    FILE:  3D_TRIG.H                                                      */
/****************************************************************************/

#ifndef _3D_TRIG_H_
#define _3D_TRIG_H_

#include "3D_VIEW.H"
#include "GENERAL.H"

#define MATH_MAX_DISTANCE           10000
#define MATH_MAX_ANGLE              1024
#define MATH_ANGLE_ACCURACY         64

#define MATH_ANGLE_360              1024
#define MATH_ANGLE_90               256
#define MATH_ANGLE_45               128

#define SIN(X)   G_sinTable[ (X) & 0x3FF ]
#define TAN(X)   G_tanTable[ (X) & 0x3FF ]
#define COS(X)   G_cosTable[ (X) & 0x3FF ]
#define ICOS(X)  G_invCosTable[ (X) & 0x3FF ]

#if 0
#define MathSineLookup(x)         SIN( (x) >> 6 )
#define MathCosineLookup(x)       COS( (x) >> 6 )
#else
#include "math.h"
#define M_PI	3.14159265358979323846
//#define MathSineLookup(x)         SIN( (x) >> 6 )
#define MathSineLookup(x)			((T_sword32)(sin(((double)(x))*2.0*M_PI/65536.0)*65536.0))
//#define MathCosineLookup(x)       COS( (x) >> 6 )
#define MathCosineLookup(x)			((T_sword32)(cos(((double)(x))*2.0*M_PI/65536.0)*65536.0))
#endif
#define MathTangentLookup(x)      TAN( (x) >> 6 )
#define MathInvCosineLookup(x)    ICOS( (x) >> 6 )
#define MathInvDistanceLookup(x)  (G_invDistTable[x])
#define MathViewTanTableLookup(x) (G_viewTanTable[x])
#define MathPower2Lookup(x)       (G_power2table[x])

extern T_sword32 G_sinTable[MATH_MAX_ANGLE];
extern T_sword32 G_tanTable[MATH_MAX_ANGLE];
extern T_sword32 G_cosTable[MATH_MAX_ANGLE];
extern T_sword32 G_invCosTable[MATH_MAX_ANGLE];
extern T_word16 G_distanceTable[256][256] ;
extern T_byte8 G_translucentTable[256][256] ;

/*
extern T_sword32 *G_sinTable ;
extern T_sword32 *G_tanTable ;
extern T_sword32 *G_cosTable ;
extern T_sword32 *G_invCosTable ;
extern T_word16 *G_distanceTable ;
*/
//extern T_byte8 *G_palIndex ;
extern T_sword32 G_invDistTable[MATH_MAX_DISTANCE];

extern T_sword32 G_viewTanTable[MAX_VIEW3D_WIDTH] ;
extern T_byte8 G_power2table[257] ;

T_sword32 MathCosine(T_word16 angle) ;

T_void MathInitializeInvDistTable(T_word32 screenWidth) ;

T_void MathInitialize(T_word32 screenWidth) ;

T_void MathFinish(void) ;

T_sword32 MathInvCosine(T_word16 angle) ;

T_sword32 MathSine(T_word16 angle) ;

T_sword32 MathTangent(T_word16 angle) ;

T_sword16 MathXTimesCosAngle(
              T_sword16 x,
              T_word16 angle) ;

T_sword16 MathXTimesSinAngle(
              T_sword16 x,
              T_word16 angle) ;

T_word16 MathArcTangent(T_sword32 y, T_sword32 x) ;

T_word16 MathArcTangent32(T_sword32 y, T_sword32 x) ;


#endif

/****************************************************************************/
/*    END OF FILE:  3D_TRIG.H                                                 */
/****************************************************************************/
