/****************************************************************************/
/*    FILE:  UPDATE.H                                                       */
/****************************************************************************/
#ifndef _UPDATE_H_
#define _UPDATE_H_

#include "GENERAL.H"

T_void UpdateGameBegin(T_void) ;

T_void UpdateGameEnd(T_void) ;

T_void UpdateMapBegin(T_void) ;

T_void UpdateMapEnd(T_void) ;

T_void UpdateFrame(T_void) ;

T_void UpdateEveryFive(T_void) ;

T_void UpdateOften(T_void) ;

#ifndef SERVER_ONLY
T_void UpdateStart3dView(T_void) ;

T_void UpdateEnd3dView(T_void) ;
#else
#define UpdateStart3dView()  ((T_void) 0)
#define UpdateEnd3dView()  ((T_void) 0)
#endif

#endif // _UPDATE_H_

/****************************************************************************/
/*    END OF FILE:  UPDATE.H                                                */
/****************************************************************************/

