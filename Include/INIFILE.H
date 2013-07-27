/****************************************************************************/
/*    FILE:  INIFILE.H                                                      */
/****************************************************************************/
#ifndef _INIFILE_H_
#define _INIFILE_H_

#include "GENERAL.H"

typedef T_void *T_iniFile ;
#define INIFILE_BAD      NULL

T_iniFile INIFileOpen(T_byte8 *p_filename) ;

T_void INIFileClose(T_byte8 *p_filename, T_iniFile iniFile) ;

T_byte8 *INIFileGet(
             T_iniFile iniFile,
             T_byte8 *p_category,
             T_byte8 *p_key) ;

T_void INIFileGetString(
             T_iniFile iniFile,
             T_byte8 *p_category,
             T_byte8 *p_key,
             T_byte8 *p_string,
             T_word16 maxLength) ;

T_void INIFilePut(
           T_iniFile iniFile,
           T_byte8 *p_category,
           T_byte8 *p_key,
           T_byte8 *p_value) ;

#endif

/****************************************************************************/
/*    END OF FILE:  INIFILE.H                                               */
/****************************************************************************/
