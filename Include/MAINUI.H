/****************************************************************************/
/*    FILE:  MAINUI.H                                                       */
/****************************************************************************/
#ifndef _MAINUI_H_
#define _MAINUI_H_

#include "FORM.H"
#include "GENERAL.H"
#include "TXTBOX.H"

E_Boolean MainUI (T_void);

T_void MainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

T_void MainUISetUpBulletins(T_TxtboxID TxtboxID);
T_void MainUIShowBulletin (T_TxtboxID TxtboxID, T_word16 number);
T_void MainUIInit (T_void);


/* LES: */
T_void MainUIStart(T_void) ;
T_void MainUIUpdate (T_void) ;
T_void MainUIEnd(T_void) ;
#endif
