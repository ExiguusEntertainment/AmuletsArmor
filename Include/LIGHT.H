/****************************************************************************/
/*    FILE:  LIGHT.H                                                        */
/****************************************************************************/
#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "GENERAL.H"

typedef T_void *T_lightTable ;
#define LIGHT_TABLE_BAD NULL

#ifdef SERVER_ONLY

#define LightTableLoad(number)          NULL
#define LightTableUnload(light)
#define LightTableRecalculate(light, out)

#else
T_lightTable LightTableLoad(T_word32 number) ;

T_void LightTableUnload(T_lightTable light) ;

T_void LightTableRecalculate(T_lightTable light, T_byte8 outsideLighting) ;
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  LIGHT.H                                                 */
/****************************************************************************/
