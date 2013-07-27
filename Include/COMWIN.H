/****************************************************************************/
/*    FILE:  COMWIN.H                                                         */
/****************************************************************************/
#ifndef _COMWIN_H_
#define _COMWIN_H_

#include "BUTTON.H"
#include "GENERAL.H"

typedef enum
{
    COMWIN_SAMPLE_SET_CITIZEN,
    COMWIN_SAMPLE_SET_WIZARD,
    COMWIN_SAMPLE_SET_FIGHTER,
    COMWIN_SAMPLE_SET_ROGUE,
    COMWIN_SAMPLE_SET_UNKNOWN
} E_comwinSampleType;


/*typedef struct
{
   T_byte8 *text;
   E_Boolean tagged;
} T_comwinCannedSayingStruct;
*/
T_void ComwinInitCommunicatePage(T_void);
T_void ComwinDisplayCommunicatePage(T_void);
T_void ComWinCloseCommunicatePage(T_void);
E_Boolean ComwinIsOpen (T_void);
T_void ComwinSay (T_buttonID buttonID);
T_void ComwinSayNth(T_word16 saynum) ;
T_void ComwinCloseCommunicatePage (T_void);

#endif
