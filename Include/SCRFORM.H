/****************************************************************************/
/*    FILE:  SCRFORM.H                                                      */
/****************************************************************************/
#ifndef _SCRFORM_H_
#define _SCRFORM_H_

#include "FORM.H"
#include "GENERAL.H"

T_void ScriptFormStart(T_word32 uiFormNumber) ;
T_void ScriptFormEnd(T_void) ;
T_void ScriptFormCallback(
           E_formObjectType objtype,
		   T_word16 objstatus,
		   T_word32 objID) ;
T_void ScriptFormUpdate(T_void) ;

/* Commands to affect the current form. */
T_void ScriptFormTextBoxSetSelection(
           T_word16 id,
           T_word16 selection) ;

#endif // _SCRFORM_H_
/****************************************************************************/
/*    END OF FILE: SCRFORM.H                                                */
/****************************************************************************/
