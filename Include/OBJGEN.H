/****************************************************************************/
/*    FILE:  OBJGEN.H                                                       */
/****************************************************************************/
#ifndef _OBJGEN_H_
#define _OBJGEN_H_

#include "GENERAL.H"

typedef T_void *T_objectGeneratorHandle ;
#define OBJECT_GENERATOR_HANDLE_BAD NULL
 
T_void ObjectGeneratorLoad(T_word32 mapNumber) ;

T_void ObjectGeneratorUnload(T_void) ;

T_void ObjectGeneratorUpdate(T_void) ;

T_void ObjectGeneratorActivate(T_word16 genID) ;

T_void ObjectGeneratorDeactivate(T_word16 genID) ;

typedef struct {
    T_sword16 x, y ;
    T_word16 angle ;
} T_objectGeneratorPosition ;

T_word16 ObjectGeneratorGetList(
             T_word16 objectType,
             T_objectGeneratorPosition *p_list,
             T_word16 maxList) ;

T_void GeneratorAddGenerator(
           T_word16 objectType,
           T_sword16 x,
           T_sword16 y,
           T_word16 angle,
           T_word16 timeBetween,
           T_word16 randomTimeBetween,
           T_word16 maxObjects,
           T_word16 maxLikeObjects,
           E_Boolean isActive,
           T_sword16 maxGenerate,
           T_word16 specialEffect) ;

/* Context handle routines: */
T_objectGeneratorHandle ObjectGeneratorCreateHandle(T_void) ;

T_void ObjectGeneratorDestroyHandle(T_objectGeneratorHandle handle) ;

T_void ObjectGeneratorSetHandle(T_objectGeneratorHandle handle) ;

T_void ObjectGeneratorGetHandle(T_objectGeneratorHandle handle) ;

#endif // _OBJGEN_H_

/****************************************************************************/
/*    END OF FILE:  OBJGEN.H                                                */
/****************************************************************************/
