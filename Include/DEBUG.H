/****************************************************************************/
/*    FILE:  DEBUG.H                                                        */
/****************************************************************************/

#ifndef _DEBUG_H_
#  define _DEBUG_H_

#include "GENERAL.H"
#include "OPTIONS.H"

#  define DEBUG_MAX_STACK_DEPTH 60

T_void DebugAddRoutine(
           const char *p_routineName,
           const char *p_filename,
           long lineNum) ;
T_void DebugFail(const char *p_msg, const char *p_file, long line) ;
T_void DebugRemoveRoutine(T_void) ;

#  ifdef NDEBUG
#    define DebugRoutine(str) ((T_void)0)
#    define DebugCheck(cond) ((T_void)0)
#    define DebugEnd() ((void)0)
#    define DebugGetCaller(file, line) ((void)0)
#    define DebugHeapOn() ((T_void)0)
#    define DebugHeapOff() ((T_void)0)
#    define DebugCompare(str)  ((T_void)0)
#    define DebugStop() ((T_void)0)
#    define DebugTime(x) ((T_void)0)
#    define DebugCheckVectorTable() ((T_void)0)
#    define DebugSaveVectorTable() ((T_void)0)
#    define DebugCheckValidStack()
#  else

#    define DebugCheck(cond)  ((cond)? (T_void)0 : \
                             DebugFail(#cond, __FILE__, __LINE__))
#    define DebugCompare(str)  DebugCompareCheck((str), __FILE__, __LINE__) ;
#    ifdef COMPILE_OPTION_DEBUG_CHECKS_STACK
#        define __STACK_IS_VALID_DEBUG_VAR   0xCCAABBDD
#        define DebugRoutine(name) T_word32 __debugCheckStackVar = __STACK_IS_VALID_DEBUG_VAR; DebugAddRoutine(name, __FILE__, __LINE__)
#        define DebugCheckValidStack()      DebugCheck(__debugCheckStackVar == __STACK_IS_VALID_DEBUG_VAR)
#        define DebugEnd()                  DebugCheckValidStack() ; DebugRemoveRoutine()
#    else
#        define DebugCheckValidStack()
#        define DebugRoutine(name) DebugAddRoutine(name, __FILE__, __LINE__)
#        define DebugEnd()         DebugRemoveRoutine()
#    endif

     T_void DebugGetCaller(const char **filename, long *line) ;
     const char *DebugGetCallerName(T_void) ;
     const char *DebugGetCallerFile(T_void) ;
     const char *DebugGetLastCalled(T_void) ;
     T_void DebugHeapOn(T_void) ;
     T_void DebugHeapOff(T_void) ;
     T_void DebugCompareCheck(T_byte8 *str, T_byte8 *p_file, T_word16 line) ;
     T_void DebugStop(T_void) ;
     T_void DebugTime(T_word16 timeSlot) ;
#    ifdef COMPILE_OPTION_DEBUG_CHECKS_VECTORS
     T_void DebugSaveVectorTable(T_void) ;
     T_void DebugCheckVectorTable(T_void) ;
#    else
#    define DebugCheckVectorTable() ((T_void)0)
#    define DebugSaveVectorTable() ((T_void)0)
#    endif
#  endif

#endif

/****************************************************************************/
/*    END OF FILE:  DEBUG.H                                                 */
/****************************************************************************/

