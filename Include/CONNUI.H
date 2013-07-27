/****************************************************************************/
/*    FILE:  CONNUI.H                                                       */
/****************************************************************************/
#ifndef _CONNUI_H_
#define _CONNUI_H_

#include "DBLLINK.H"
#include "FORM.H"
#include "GENERAL.H"

typedef struct
{
    T_byte8 servname[80];
    T_byte8 servphone[80];
    T_doubleLinkListElement This;
} T_connUIStruct;

E_Boolean ConnMainUI (T_void);

T_void ConnMainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

T_void ConnMainUIInit (T_void);

T_void ConnMainUIStart(T_void) ;

T_void ConnMainUIEnd(T_void) ;

T_void ConnMainUIUpdate(T_void) ;

#endif
