#ifndef _DIRECT_H_
#define _DIRECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "MOUSEMOD.H"
#include "GENERAL.H"

void DirectDrawOn(void) ;
void DirectDrawOff(void) ;
void _cdecl WindowsUpdate(char *p_screen, unsigned char * palette) ;
void OutsideMouseDriverGet(T_word16 *xPos, T_word16 *yPos);
T_void DirectMouseSet(T_word16 newX, T_word16 newY) ;
T_buttonClick DirectMouseGetButton(T_void) ;
T_void DirectMouseSetButton(T_buttonClick click) ;
T_void DirectMouseClearButton(T_buttonClick click) ;

#ifdef __cplusplus
}
#endif

#endif // _DIRECT_H_
