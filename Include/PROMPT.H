/****************************************************************************/
/*    FILE:  PROMPT.H                                                       */
/****************************************************************************/
#ifndef _PROMPT_H_
#define _PROMPT_H_

#include "FORM.H"
#include "GENERAL.H"

typedef enum
{
    PROMPT_ACTION_NO_ACTION,
    PROMPT_ACTION_CANCEL,
    PROMPT_ACTION_OK
} E_promptAction;


T_void    PromptStatusBarInit (T_byte8 *prompt, T_word16 baserange);
T_void    PromptStatusBarUpdate (T_word16 current);
T_void    PromptStatusBarClose (T_void);

E_Boolean PromptForBoolean (T_byte8 *prompt, E_Boolean defaultvalue);
T_word16  PromptForInteger (T_byte8 *prompt,
                            T_word16 defaultvalue,
                            T_word16 minvalue,
                            T_word16 maxvalue);

T_void    PromptDisplayMessage (T_byte8 *prompt);
T_void    PromptDisplayBulletin (T_byte8 *prompt);
E_Boolean PromptDisplayDialogue (T_byte8 *data);
E_Boolean PromptDisplayContinue (T_byte8 *prompt);
E_Boolean PromptForString  (T_byte8 *prompt,
                            T_word16 maxlen,
                            T_byte8 *data);

T_void PromptControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID);

#endif
