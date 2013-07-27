/****************************************************************************/
/*    FILE:  ACCOUNT.C                                                      */
/****************************************************************************/

#include "ACCOUNT.H"
#include "FLATFILE.H"
#include "GENERAL.H"
#include "LOOKUP32.H"
#include "MEMORY.H"

#define ACCOUNT_DB_TAG                (*((T_word32 *)"AcDb"))
#define ACCOUNT_DB_DEAD_TAG           (*((T_word32 *)"DaCd"))

typedef struct {
    T_word32 tag ;
    T_flatFile data ;
    T_lookup32 index ;
} T_accountDBStruct ;

#define DBHandleToStruct(acct) ((T_accountDBStruct *)(acct))
#define DBStructToHandle(acct) ((T_accountDB)(acct))

/****************************************************************************/
/*  Routine:  AccountDBOpen                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Opens up a given account database and returns a handle to it.         */
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
/*    T_accountDB                 -- Handle to account database             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_accountDB AccountDBOpen(T_byte8 *p_filename)
{
    T_byte8 filename[80] ;
    T_accountDBStruct *p_accountDB = NULL ;

    DebugRoutine("AccountDBOpen") ;
    DebugCheck(p_filename != NULL) ;

	/* Allocate memory to hold the index and account database. */
    p_accountDB = MemAlloc(sizeof(T_accountDBStruct)) ;
    DebugCheck(p_accountDB != NULL) ;

    if (p_accountDB)  {
        /* Open up the account data file. */
        sprintf(filename, "%s.ADB", p_filename) ;
		p_accountDB->data = FlatFileOpenOrCreate(filename, sizeof(T_account)) ;
        DebugCheck(p_accountDB->data != FLATFILE_BAD) ;

        /* Open up the account info file. */
        sprintf(filename, "%s.AIX", p_filename) ;
        p_accountDB->index = Lookup32OpenOrCreate(filename) ;
        DebugCheck(p_accountDB->index != LOOKUP32_BAD) ;

        /* Check to see if both parts were opened. */
        if ((p_accountDB->data == FLATFILE_BAD) ||
            (p_accountDB->index == LOOKUP32_BAD))  {
            /* One or both is bad.  Drop the attempt to open. */
            if (p_accountDB->data != FLATFILE_BAD)
                FlatFileClose(p_accountDB->data) ;

            if (p_accountDB->index != LOOKUP32_BAD)
                Lookup32Close(p_accountDB->index) ;

            /* Free the previously allocated account db handle. */
            MemFree(p_accountDB) ;
            p_accountDB = NULL ;
        } else {
			/* Everything ok, mark the handle as good. */
			p_accountDB->tag = ACCOUNT_DB_TAG ;
		}
	}

	DebugEnd() ;

	return(DBStructToHandle(p_accountDB)) ;
}

/****************************************************************************/
/*  Routine:  AccountDBClose                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Closes a previously opened account database.                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_accountDB account         -- Account db to close                    */
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
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AccountDBClose(T_accountDB accountDB)
{
    T_accountDBStruct *p_accountDB ;

    DebugRoutine("AccountDBClose") ;
    DebugCheck(accountDB != ACCOUNT_DB_BAD) ;

    /* Get a quick pointer to the account db data structure. */
    p_accountDB = DBHandleToStruct(accountDB) ;

    if (p_accountDB)  {
        /* Make sure this is not already closed (aka valid). */
        DebugCheck(p_accountDB->tag == ACCOUNT_DB_TAG) ;
        if (p_accountDB->tag == ACCOUNT_DB_TAG)  {
            /* Close the index side. */
            Lookup32Close(p_accountDB->index) ;

            /* Close the data side. */
            FlatFileClose(p_accountDB->data) ;

            /* Mark the header as no longer valid. */
            p_accountDB->tag = ACCOUNT_DB_DEAD_TAG ;

            /* Free up the handle. */
            MemFree(p_accountDB) ;
        }
	}

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AccountDBGet                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Gets a copy of the current account.  Use MemFree when you are done    */
/*  to get rid of the account record.  It is just a copy.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_accountDB account         -- Account db to get account              */
/*                                                                          */
/*    T_word32 accountNumber      -- Number to use as index                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_account *                 -- Pointer to allocated memory, else NULL */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_account *AccountDBGet(
               T_accountDB accountDB,
               T_word32 accountNumber)
{
    T_accountDBStruct *p_accountDB ;
	T_account *p_account = NULL ;
    T_word32 index ;
    T_flatFileIndex dataIndex ;

    DebugRoutine("AccountDBGet") ;
    DebugCheck(accountDB != ACCOUNT_DB_BAD) ;

    /* Get a quick pointer to the account db data structure. */
    p_accountDB = DBHandleToStruct(accountDB) ;

    if (p_accountDB)  {
        /* Make sure this is a valid account db. */
        DebugCheck(p_accountDB->tag == ACCOUNT_DB_TAG) ;
        if (p_accountDB->tag == ACCOUNT_DB_TAG)  {
            /* Ok, find the flat file index for the given account number */
            index = Lookup32Get(p_accountDB->index, accountNumber) ;
            /* Convert the types. */
            dataIndex = index ;

            /* See if that record exists. */
            if (dataIndex != FLATFILE_INDEX_BAD)  {
                /* Yep, it exists.  Now get the record itself. */
                p_account = FlatFileGetRecord(p_accountDB->data, dataIndex) ;

                /* If the record was retrieved, we should not be NULL. */
                /* However, the index has it, so the account should exist. */
                DebugCheck(p_account != NULL) ;
            }
        }
    }

    DebugEnd() ;

    return p_account ;
}

/****************************************************************************/
/*  Routine:  AccountDBPut                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Put a copy of the current account into the account database.          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_accountDB account         -- Account db to put account              */
/*                                                                          */
/*    T_word32 accountNumber      -- Number to use as index                 */
/*                                                                          */
/*    T_account *                 -- Pointer to account record              */
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
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AccountDBPut(
           T_accountDB accountDB,
           T_word32 accountNumber,
           T_account *p_account)
{
    T_accountDBStruct *p_accountDB ;
    T_word32 index ;
    T_flatFileIndex dataIndex ;

	DebugRoutine("AccountDBPut") ;

    /* Get a quick pointer to the account db data structure. */
    p_accountDB = DBHandleToStruct(accountDB) ;

    if (p_accountDB)  {
        /* Make sure this is a valid account db. */
        DebugCheck(p_accountDB->tag == ACCOUNT_DB_TAG) ;
        if (p_accountDB->tag == ACCOUNT_DB_TAG)  {
			/* Ok, find the flat file index for the given account number */
            index = Lookup32Get(p_accountDB->index, accountNumber) ;
            /* Convert the types. */
            dataIndex = index ;

            /* See if that record exists. */
            if (dataIndex == FLATFILE_INDEX_BAD)  {
                /* Nope, create it. */
                dataIndex = FlatFileCreateRecord(p_accountDB->data) ;
                DebugCheck(dataIndex != FLATFILE_INDEX_BAD) ;

                /* Convert to 32 bit value. */
                index = dataIndex ;

                /* Link up the record to the index for the */
                /* account number. */
				Lookup32Put(p_accountDB->index, accountNumber, index) ;
				Lookup32Refresh(p_accountDB->index) ;
            }

            /* Write out the record. */
            if (dataIndex != FLATFILE_INDEX_BAD)  {
				FlatFilePutRecord(p_accountDB->data, dataIndex, p_account) ;
                FlatFileRefresh(p_accountDB->data) ;
			}
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  AccountDBDelete                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Delete the entry for the account.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_accountDB account         -- Account db to delete account           */
/*                                                                          */
/*    T_word32 accountNumber      -- Number to use as index                 */
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
/*    LES  03/10/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void AccountDBDelete(
           T_accountDB accountDB,
           T_word32 accountNumber)
{
    T_accountDBStruct *p_accountDB ;
    T_word32 index ;
    T_flatFileIndex dataIndex ;

	DebugRoutine("AccountDBDelete") ;

    /* Get a quick pointer to the account db data structure. */
    p_accountDB = DBHandleToStruct(accountDB) ;

    if (p_accountDB)  {
        /* Make sure this is a valid account db. */
        DebugCheck(p_accountDB->tag == ACCOUNT_DB_TAG) ;
        if (p_accountDB->tag == ACCOUNT_DB_TAG)  {
            /* Ok, find the flat file index for the given account number */
            index = Lookup32Get(p_accountDB->index, accountNumber) ;

            /* Convert the types. */
            dataIndex = index ;

			/* Delete the record from the data file. */
			FlatFileDeleteRecord(p_accountDB->data, dataIndex) ;
			FlatFileRefresh(p_accountDB->data) ;

            /* Now delete the index entry. */
			Lookup32Put(
                p_accountDB->index,
                accountNumber,
				FLATFILE_INDEX_BAD) ;
			Lookup32Refresh(p_accountDB->index) ;
		}
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  ACCOUNT.C                                               */
/****************************************************************************/
