/****************************************************************************/
/*    FILE:  ACCOUNT.H                                                      */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*  The Account DB module is intended to provide low level, non-file locking*/
/*  capabilities to handle records of accounts by their account numbers.    */
/*  If two processes are using these same set of routines on the same file, */
/*  beware.  This module uses extensive use of the Flat File Database       */
/*  Module and the Lookup32 Module.  In addition, knowledge is necessary    */
/*  for the data type T_account of the character list as provided in the    */
/*  Stats Module (the .H file).                                             */
/*                                                                          */
/****************************************************************************/

#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#include "GENERAL.H"
#include "STATS.H"

/* Handle to database. */
typedef T_void *T_accountDB ;
#define ACCOUNT_DB_BAD   NULL

#define PRIVILEDGE_LEVEL_STANDARD     1024

/* Structure of the records: */
typedef struct {
    T_byte8 ownersName[60] ;
    T_byte8 street[80] ;
    T_byte8 city[40] ;
    T_byte8 state[20] ;
    T_byte8 zipcode[14] ;
    T_byte8 billingMethod[20] ;
    T_byte8 billingData[60] ;
    T_byte8 phoneHome[20] ;
    T_byte8 phoneWork[20] ;
    T_word32 accountID ;
    T_word32 nextAccount ;
    T_word32 prevAccount ;
    T_word16 priviledgeLevel ;
    E_Boolean inUse ;
    T_byte8 status ;
    T_statsSavedCharArray characters ;
    T_word32 characterIDs[MAX_CHARACTERS_PER_SERVER] ;
} T_account ;

T_accountDB AccountDBOpen(T_byte8 *p_filename) ;

T_void AccountDBClose(T_accountDB accountDB) ;

T_account *AccountDBGet(
               T_accountDB accountDB,
               T_word32 accountNumber) ;

T_void AccountDBPut(
           T_accountDB accountDB,
           T_word32 accountNumber,
           T_account *p_account) ;

T_void AccountDBDelete(
           T_accountDB accountDB,
           T_word32 accountNumber) ;

#endif

/****************************************************************************/
/*    END OF FILE:  ACCOUNT.H                                               */
/****************************************************************************/
