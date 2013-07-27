/****************************************************************************/
/*    FILE:  DBLLINK.H                                                      */
/****************************************************************************/

#ifndef _DBLLINK_H_
#define _DBLLINK_H_

#include "GENERAL.H"

typedef T_void *T_doubleLinkList ;
#define DOUBLE_LINK_LIST_BAD 0 /* NULL */

typedef T_void *T_doubleLinkListElement ;
#define DOUBLE_LINK_LIST_ELEMENT_BAD NULL

typedef E_Boolean (*T_doubleLinkListTraverseCallback)
                      (T_doubleLinkListElement element) ;

T_doubleLinkList DoubleLinkListCreate(T_void) ;

T_void DoubleLinkListDestroy(T_doubleLinkList linkList) ;

T_void DoubleLinkListFreeAndDestroy(T_doubleLinkList *linkList) ;

T_doubleLinkListElement DoubleLinkListAddElementAtEnd(
                            T_doubleLinkList linkList,
                            T_void *p_data) ;

T_doubleLinkListElement DoubleLinkListAddElementAtFront(
                            T_doubleLinkList linkList,
                            T_void *p_data) ;

T_doubleLinkListElement DoubleLinkListAddElementAfterElement(
                            T_doubleLinkListElement element,
                            T_void *p_data) ;

T_doubleLinkListElement DoubleLinkListAddElementBeforeElement(
                            T_doubleLinkListElement element,
                            T_void *p_data) ;

T_word32 DoubleLinkListGetNumberElements(T_doubleLinkList linkList) ;


T_void *DoubleLinkListRemoveElement(T_doubleLinkListElement element) ;

T_doubleLinkListElement DoubleLinkListTraverse(
                            T_doubleLinkList linkList,
                            T_doubleLinkListTraverseCallback callback) ;

T_void *DoubleLinkListElementGetData(T_doubleLinkListElement element) ;

T_doubleLinkListElement DoubleLinkListElementGetNext(
                            T_doubleLinkListElement element) ;

T_doubleLinkListElement DoubleLinkListElementGetPrevious(
                            T_doubleLinkListElement element) ;

T_doubleLinkListElement DoubleLinkListGetFirst(T_doubleLinkList linkList) ;

T_doubleLinkListElement DoubleLinkListGetLast(T_doubleLinkList linkList) ;

#ifndef NDEBUG
T_void DoubleLinkListDisplay(T_doubleLinkList linkList) ;
#else
#define DoubleLinkListDisplay(linklist)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  DBLLINK.H                                               */
/****************************************************************************/
