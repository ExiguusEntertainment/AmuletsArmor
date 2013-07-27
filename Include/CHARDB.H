/****************************************************************************/
/*    FILE:  CHARDB.H                                                       */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*  The Character DB module is intended to provide low level, non-file      */
/*  locking capabilities to handle records of characters by their           */
/*  character ID.  If two processes are using the same set of routines on   */
/*  the same file, beware!  This module uses extensive use of the Flat File */
/*  Database Module.  In addition, knowledge is necessary for the data type */
/*  T_playerStats.                                                          */
/*                                                                          */
/****************************************************************************/

#ifndef _CHARDB_H_
#define _CHARDB_H_

#include "FLATFILE.H"
#include "GENERAL.H"
#include "STATS.H"

/* Handle to database. */
typedef T_flatFile T_characterDB ;
#define CHARACTERDB_BAD   FLATFILE_BAD

typedef union {
    T_playerStats stats ;
    T_byte8 reserved[16384] ;
} T_character ;

T_characterDB CharacterDBOpen(T_byte8 *p_filename) ;

T_void CharacterDBClose(T_characterDB characterDB) ;

T_word32 CharacterDBCreate(T_characterDB characterDB) ;

T_void CharacterDBDelete(
           T_characterDB characterDB,
           T_word32 characterID) ;

T_character *CharacterDBGet(
               T_characterDB characterDB,
               T_word32 characterID) ;

T_void CharacterDBPut(
           T_characterDB characterDB,
           T_word32 characterID,
           T_character *p_character) ;

#endif

/****************************************************************************/
/*    END OF FILE:  CHARDB.H                                                */
/****************************************************************************/
