
#ifndef _GENERAL_H_
#define _GENERAL_H_

#include "OPTIONS.H"

/* General typedefs declared  */
/** This file now supports either Watcom C32 OR Linux GCC. -- AMT, 8/2/95 **/

/** If I'm on a unix box, grab some system typedefs as well. **/
#ifdef TARGET_UNIX
#  include <sys/types.h>

typedef time_t T_time;

#endif

#if DOS32
/** DOS includes **/
/** C library and system call interface includes. **/
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <mem.h>
#  include <i86.h>
#  include <io.h>
#  include <dos.h>
#  include <conio.h>
#  include <math.h>
#endif

/** If under GCC, we need to specify that our packet structures are **/
/** packed. **/
#ifdef __GNUC__
#  define PACK __attribute__ ((packed))
#else
#  define PACK
#endif

/* Because many other #includes define these in bad ways. */
#undef TRUE
#undef FALSE

typedef void T_void ;

typedef unsigned char T_bit1 ;

typedef unsigned char T_nibble4 ;
typedef signed char T_snibble4 ;

typedef unsigned char T_byte8 ;
typedef signed char T_sbyte8 ;

typedef unsigned short T_word16 ;
typedef signed short T_sword16 ;

typedef unsigned int T_word32 ;
typedef signed int T_sword32 ;

typedef unsigned long T_word64 ;
typedef signed long T_sword64 ;

typedef T_byte8 E_Boolean ;
#define FALSE 0
#define TRUE 1
#define BOOLEAN_UNKNOWN 2

typedef union {
    T_word32 w32 ;
    struct {
        T_word16 low ;
        T_word16 high ;
    } w1616 ;
} T_word32as1616 ;

typedef union {
    T_word32 w32 ;
    struct {
        T_byte8 lowest ;
        T_byte8 low ;
        T_byte8 high ;
        T_byte8 highest ;
    } w8888 ;
} T_word32as8888 ;

#ifndef NULL
#define NULL 0
#endif

#include "DEBUG.H"

#endif
