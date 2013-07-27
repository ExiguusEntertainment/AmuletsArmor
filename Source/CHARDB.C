/****************************************************************************/
/*    FILE:  CHARDB.C                                                       */
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
#include "CHARDB.H"
#include "GENERAL.H"

/****************************************************************************/
/*  Routine:  CharacterDBOpen                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Opens up a given character database and returns a handle to it.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- Pointer to filename                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_characterDB               -- Handle to character database           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_characterDB CharacterDBOpen(T_byte8 *p_filename)
{
    T_flatFile characterDB ;

    DebugRoutine("CharacterDBOpen") ;

	characterDB = FlatFileOpenOrCreate(p_filename, sizeof(T_character)) ;
	DebugCheck(characterDB != FLATFILE_BAD) ;

	FlatFileRefresh(characterDB) ;

	DebugEnd() ;

    return characterDB ;
}

/****************************************************************************/
/*  Routine:  CharacterDBClose                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Closes a previously opened character database.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_characterDB characterDB   -- character database to close            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CharacterDBClose(T_characterDB characterDB)
{
    DebugRoutine("CharacterDBClose") ;

	FlatFileRefresh(characterDB) ;
	FlatFileClose(characterDB) ;

	DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CharacterDBGet                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Gets a copy of a character in the database and returns it via a       */
/*  pointer.  When done with the character, just do a MemFree.              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_characterDB characterDB   -- character database to get character    */
/*                                                                          */
/*    T_word32 characterID        -- ID of character to fetch               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_character *CharacterDBGet(
               T_characterDB characterDB,
               T_word32 characterID)
{
    T_character *p_character ;

    DebugRoutine("CharacterDBGet") ;

    p_character = (T_character *)FlatFileGetRecord(characterDB, characterID);

	DebugEnd() ;

	return p_character ;
}

/****************************************************************************/
/*  Routine:  CharacterDBPut                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Stores a the given character into the character database overwriting  */
/*  what is previously there.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_characterDB characterDB   -- character database to get character    */
/*                                                                          */
/*    T_word32 characterID        -- ID of character to fetch               */
/*                                                                          */
/*    T_character *p_character    -- Pointer to character to store          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CharacterDBPut(
           T_characterDB characterDB,
           T_word32 characterID,
           T_character *p_character)
{
    DebugRoutine("CharacterDBPut") ;

printf("CharacterDBPut: %08lX %08lX %08lX\n", characterDB, characterID, p_character) ;
	FlatFilePutRecord(characterDB, characterID, p_character) ;
    FlatFileMarkDirty(characterDB) ;
	FlatFileRefresh(characterDB) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CharacterDBDelete                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Deletes a previously created character.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_characterDB characterDB   -- character database to get character    */
/*                                                                          */
/*    T_word32 characterID        -- ID of character to fetch               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CharacterDBDelete(
           T_characterDB characterDB,
           T_word32 characterID)
{
    DebugRoutine("CharacterDBDelete") ;

	FlatFileDeleteRecord(characterDB, characterID) ;
	FlatFileRefresh(characterDB) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CharacterDBCreate                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Creates a new, fresh, spanking character                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_characterDB characterDB   -- character database to create char      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- character ID                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 CharacterDBCreate(T_characterDB characterDB)
{
    T_word32 characterID ;

    DebugRoutine("CharacterDBDelete") ;

	characterID = FlatFileCreateRecord(characterDB) ;
    FlatFileMarkDirty(characterDB) ;
	FlatFileRefresh(characterDB) ;

	DebugEnd() ;

    return characterID ;
}

/****************************************************************************/
/*    END OF FILE:  CHARDB.C                                                */
/****************************************************************************/
