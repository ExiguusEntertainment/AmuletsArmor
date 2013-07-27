
#ifndef _GENERAL_H_
#define _GENERAL_H_
/* General typedefs declared (for Borland C 3.1 -- Normal DOS Mode): */

typedef void T_void ;

typedef unsigned char T_bit1 ;

typedef unsigned char T_nibble4 ;
typedef signed char T_snibble4 ;

typedef unsigned char T_byte8 ;
typedef signed char T_sbyte8 ;

typedef unsigned int T_word16 ;
typedef signed int T_sword16 ;

typedef unsigned long T_word32 ;
typedef signed long T_sword32 ;

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

/* For our reference, note that we are using the borland compiler */
#define BORLAND_COMPILER

#endif
