/****************************************************************************/
/*    FILE:  MAILUI.H                                                       */
/****************************************************************************/
#ifndef _MAILUI_H_
#define _MAILUI_H_

#include "FORM.H"
#include "GENERAL.H"

T_void MailMainUI (T_void);

T_void MailMainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

T_void MailMainUIInit (T_void);


T_void MailWriteUI (T_void);

T_void MailWriteUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

T_void MailWriteUIInit (T_void);


T_void MailReadUI (T_void);

T_void MailReadUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

T_void MailReadUIInit (T_void);


#endif
