/****************************************************************************/
/*    FILE:  COLOR.H                                                        */
/****************************************************************************/
#ifndef _COLOR_H_
#define _COLOR_H_

#include "GENERAL.H"

T_void  ColorInit (T_void);
T_void  ColorStoreDefaultPalette (T_void);
T_void  ColorAddGlobal (T_sword16 red, T_sword16 green, T_sword16 blue);
T_void  ColorSetGlobal (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue);
T_void  ColorAddFilt (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue);
T_void  ColorSetFilt (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue);
T_void  ColorResetFilt (T_void);
T_void  ColorUpdate (T_word16 delta);
T_void  ColorGlowUpdate (T_void);
T_void  ColorSetColor (T_byte8 colornum, T_byte8 red, T_byte8 green, T_byte8 blue);
T_void  ColorRestore (T_void);
T_byte8 ColorGetRed (T_byte8 colornum);
T_byte8 ColorGetBlue (T_byte8 colornum);
T_byte8 ColorGetGreen (T_byte8 colornum);
T_void  ColorFadeTo (T_byte8 red, T_byte8 green, T_byte8 blue);

T_word16 ColorGammaAdjust(T_void) ;

#endif
