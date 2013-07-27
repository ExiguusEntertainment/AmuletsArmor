/****************************************************************************/
/*    FILE:  SCRIPT.H                                                       */
/****************************************************************************/
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "GENERAL.H"

typedef T_void *T_script ;
#define SCRIPT_BAD  NULL

typedef T_byte8 E_scriptDataType ;
#define SCRIPT_DATA_TYPE_8_BIT_NUMBER           0
#define SCRIPT_DATA_TYPE_16_BIT_NUMBER          1
#define SCRIPT_DATA_TYPE_32_BIT_NUMBER          2
#define SCRIPT_DATA_TYPE_STRING                 3
#define SCRIPT_DATA_TYPE_VARIABLE               4
#define SCRIPT_DATA_TYPE_EVENT_PARAMETER        5
#define SCRIPT_DATA_TYPE_FLAG                   6
#define SCRIPT_DATA_TYPE_NONE                   7
#define SCRIPT_DATA_TYPE_UNKNOWN                8

typedef T_byte8 E_scriptFlag ;
#define SCRIPT_FLAG_EQUAL                  0
#define SCRIPT_FLAG_NOT_EQUAL              1
#define SCRIPT_FLAG_LESS_THAN              2
#define SCRIPT_FLAG_NOT_LESS_THAN          3
#define SCRIPT_FLAG_GREATER_THAN           4
#define SCRIPT_FLAG_NOT_GREATER_THAN       5
#define SCRIPT_FLAG_LESS_THAN_OR_EQUAL     6
#define SCRIPT_FLAG_GREATER_THAN_OR_EQUAL  7
#define SCRIPT_FLAG_UNKNOWN                8

T_void ScriptInitialize(T_void) ;

T_void ScriptFinish(T_void) ;

T_script ScriptLock(T_word32 number) ;

T_void ScriptUnlock(T_script script) ;

E_Boolean ScriptEvent(
              T_script script,
              T_word16 eventNumber,
              E_scriptDataType type1,
              T_void *p_data1,
              E_scriptDataType type2,
              T_void *p_data2,
              E_scriptDataType type3,
              T_void *p_data3) ;

E_Boolean ScriptRunPlace(
              T_script script,
              T_word16 placeNumber) ;

T_word32 ScriptGetOwner(T_script script) ;

T_void ScriptSetOwner(T_script script, T_word32 owner) ;

#endif // _SCRIPT_H_

/****************************************************************************/
/*    END OF FILE:  SCRIPT.H                                                */
/****************************************************************************/
