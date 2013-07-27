/****************************************************************************/
/*    FILE:  LOOKUP32.H                                                     */
/****************************************************************************/
/*                                                                          */
/*    Lookup32 is a module that allows a way to store large conversion      */
/*  tables that take a 32 bit number and converts it to a different 32 bit  */
/*  number.  This is useful when looking up account records and such        */
/*  and you have a 32 bit value for the account number, but need to know    */
/*  what the corresponding record number is.                                */
/*                                                                          */
/*  NOTE: Requires the FlatFile module.                                     */
/*                                                                          */
/****************************************************************************/
#ifndef _LOOKUP32_H_
#define _LOOKUP32_H_

#include "FLATFILE.H"

typedef T_flatFile T_lookup32 ;
#define LOOKUP32_BAD FLATFILE_BAD

#define Lookup32Refresh(lookup) FlatFileRefresh(lookup) ;

T_lookup32 Lookup32OpenOrCreate(T_byte8 *filename) ;

T_void Lookup32Close(T_lookup32 lookup) ;

T_void Lookup32Put(T_lookup32 lookup, T_word32 index, T_word32 value) ;

T_word32 Lookup32Get(T_lookup32 lookup, T_word32 index) ;

#endif // _LOOKUP32_H_
/****************************************************************************/
/*    END OF FILE: LOOKUP32.H                                               */
/****************************************************************************/

