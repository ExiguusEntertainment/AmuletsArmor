#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#define TICKER_INTERRUPT_NUMBER 0x8

typedef unsigned char T_byte8 ;
typedef unsigned int T_word16 ;
typedef void T_void ;
typedef T_byte8 T_palette[256][3] ;

T_palette palette ;

static T_void (__interrupt __far *IOldTickerInterrupt)(); ;

T_void GrSetPalette(
           T_byte8 start_color,
           T_word16 number_colors)
{
    T_word16 i ;
    T_byte8 color ;

    /* Set the index color to start chaning. */
    outp(0x3C8, start_color) ;

    /* For each color that was passed to us, stored the RGB value */
    /* in the real palette. */
    for (color=0, i=number_colors; i>0; i--, color++)  {
        outp(0x03C9, palette[color][0]) ;
        outp(0x03C9, palette[color][1]) ;
        outp(0x03C9, palette[color][2]) ;
    }
}


static T_void __interrupt __far ITickerInterruptSimple(T_void)
{
    T_word16 i ;
    static T_byte8 flag = 0 ;

    /* Do the old interrupt. */
    IOldTickerInterrupt() ;

    if (flag == 1)  {
        flag = 0 ;
        /* Change colors. */
        for (i=0; i<32; i++)  {
            palette[i][0] = rand()&63 ;
            palette[i][1] = rand()&63 ;
            palette[i][2] = rand()&63 ;
        }
        GrSetPalette(225, 32) ;
    } else {
        flag++ ;
    }


    /* Note that we are done. */
    outp(0x20, 0x20) ;
}


T_void main(T_void)
{
    puts("Installing color mapper.") ;
    IOldTickerInterrupt = _dos_getvect(TICKER_INTERRUPT_NUMBER);
    _dos_setvect(TICKER_INTERRUPT_NUMBER, ITickerInterruptSimple);
    puts("Installed.") ;
    keep(0, (_SS + (_SP/16) - _psp));
}
