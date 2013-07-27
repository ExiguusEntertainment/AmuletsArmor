/****************************************************************************/
/*    FILE:  KEYMAP.C                                                       */
/****************************************************************************/
#include "GENERAL.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"

/* Global variables. */
T_byte8 G_keyMap[256] ;               /* Array of keyscan codes */
static E_Boolean G_init = FALSE ;     /* Flag to determine if init'd */

/****************************************************************************/
/*  Routine:  KeyMapInitialize                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    KeyMapInitialize is called after a iniFile is opened.  This routine   */
/*  pulls the key configuration out of the keyboard group in the ini file.  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_iniFile iniFile           -- .ini file with keyboard group          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void KeyMapInitialize(T_iniFile iniFile)
{
    T_word16 i, j ;
    T_byte8 *buffer ;
    T_word16 c ;
    T_word16 len;

    DebugRoutine("KeyMapInitialize") ;
    DebugCheck(G_init == FALSE) ;
    G_init = TRUE ;

    // Setup default keys in case .ini file is missing or incorrect
    memset(G_keyMap, 0, sizeof(G_keyMap));
//    G_keyMap[KEYMAP_ACTIVATE_OR_TAKE] = KEY_SCAN_CODE_E;

    buffer = INIFileGet(iniFile, "keyboard", "keys1") ;
    if (buffer)  {
        for (j=i=0; i<34; i++)  {
            sscanf(buffer+j, "%02X", &c) ;
            G_keyMap[i] = (T_byte8)c ;
            j+=2 ;
        }
    }
    buffer = INIFileGet(iniFile, "keyboard", "keys2") ;
    if (buffer)  {
        len = strlen(buffer);
        for (j=0; i<KEYMAP_NUM_KEYS_MAPPED; i++)  {
            if (j < len) {
                sscanf(buffer+j, "%02X", &c) ;
                G_keyMap[i] = (T_byte8)c ;
                j+=2 ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  KeyMapFinish                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    KeyMapFinish releases any resources it may be using.                  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_iniFile iniFile           -- .ini file with keyboard group          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void KeyMapFinish(T_void)
{
    DebugRoutine("KeyMapFinish") ;
    DebugCheck(G_init == TRUE) ;

    G_init = FALSE ;

    /* Do nothing at this time. */

    DebugEnd() ;
}

T_void KeyMapReinitialize(T_iniFile iniFile)
{
    DebugRoutine("KeyMapReinitialize");
    KeyMapFinish();
    KeyMapInitialize(iniFile);
    DebugEnd();
}
/****************************************************************************/
/*    END OF FILE:  KEYMAP.C                                                */
/****************************************************************************/

