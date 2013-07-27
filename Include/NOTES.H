/****************************************************************************/
/*    FILE:  NOTES.H                                                         */
/****************************************************************************/
#ifndef _NOTES_H_
#define _NOTES_H_

#include "BUTTON.H"
#include "GENERAL.H"

T_void NotesOpenNotesPage (T_void);
T_void NotesCloseNotesPage (T_void);

T_void NotesOpenJournalPage (T_void);
T_void NotesUpdateJournalPage (T_void);

T_void NotesChangeJournalPage (T_buttonID buttonID);
T_void NotesGotoPageID (T_word16 notePage);

T_void NotesTransferJournalPage (T_void);
E_Boolean NotesJournalWindowIsAt (T_word16 x, T_word16 y);

#endif

