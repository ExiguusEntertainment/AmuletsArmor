/****************************************************************************/
/*    FILE:  MESSAGE.H                                                      */
/****************************************************************************/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "GENERAL.H"
#include "GRAPHICS.H"

#define MAX_EXTENDED_COLORS 40
#define OUTPUT_BOX_X1 6
#define OUTPUT_BOX_Y1 139
#define OUTPUT_BOX_X2 206
#define OUTPUT_BOX_Y2 148

T_void MessageScrollUp(T_void) ;

T_void MessageScrollDown(T_void) ;

T_void MessageDraw(
           T_word16 x,
           T_word16 y,
           T_word16 interleave,
           T_color color) ;

T_void MessageAdd(T_byte8 *p_string) ;

T_void MessageClear(T_void) ;

T_void MessageDisplayMessage (T_word16 messagenum);

int MessagePrintf( char *fmt, ... ) ;

T_void MessageSetAlternateOutputOn(T_void);
T_void MessageSetAlternateOutputOff(T_void);

T_void MessageDrawLine(
        T_word16 x_pos,
        T_word16 y_pos,
        T_byte8 *line,
        T_color color);


#endif

/****************************************************************************/
/*    END OF FILE:  MESSAGE.H                                               */
/****************************************************************************/
