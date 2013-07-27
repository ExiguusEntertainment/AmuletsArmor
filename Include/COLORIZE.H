/****************************************************************************/
/*    FILE:  COLORIZE.H                                                     */
/****************************************************************************/
#ifndef _COLORIZE_H_
#define _COLORIZE_H_

#include "GENERAL.H"

typedef T_byte8 E_colorizeTable ;
#define COLORIZE_TABLE_NONE           0
#define COLORIZE_TABLE_WOOD           1
#define COLORIZE_TABLE_RUSTY          2
#define COLORIZE_TABLE_BRONZE         3
#define COLORIZE_TABLE_IRON           4
#define COLORIZE_TABLE_SILVER         5
#define COLORIZE_TABLE_STEEL          6
#define COLORIZE_TABLE_HARDEN_STEEL   7
#define COLORIZE_TABLE_MITHRIL        8
#define COLORIZE_TABLE_OBSIDIAN       9
#define COLORIZE_TABLE_PYRINIUM       10
#define COLORIZE_TABLE_ADAMINIUM      11
#define COLORIZE_TABLE_ORANGE_TO_RED  12
#define COLORIZE_TABLE_RED_TO_PURPLE  13
#define COLORIZE_TABLE_NO_ARMOR       14

#define MAX_COLORIZE_TABLES  15

T_void ColorizeInitialize(T_void) ;
T_void ColorizeFinish(T_void) ;

T_void ColorizeMemory(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count,
           E_colorizeTable table) ;

T_byte8 *ColorizeGetTable(E_colorizeTable colorTable) ;

#endif

/****************************************************************************/
/*    END OF FILE:  COLORIZE.H                                              */
/****************************************************************************/
