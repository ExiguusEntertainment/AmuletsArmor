/*-------------------------------------------------------------------------*
 * File:  DBLLINK.C
 *-------------------------------------------------------------------------*/
/**
 * In my crazed days, I decided that having a double link list 'class'
 * would be useful and I would no longer have to write this code over
 * and over.  Amulets & Armor uses this code for all double linked lists.
 * This version has been optimized to not use malloc and free per node
 * pointer structure.
 *
 * @addtogroup DBLLINK
 * @brief Double Linked List
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "DBLLINK.H"
#include "GENERAL.H"
#include "GRAPHICS.H"
#include "MEMORY.H"

#define DOUBLE_LINK_LIST_TAG      (*((T_word32 *)"DbLi"))
#define DOUBLE_LINK_LIST_DEAD_TAG (*((T_word32 *)"DdBl"))

/* Flag to turn on output of all double link node creation */
//#define COMPILE_OPTION_DOUBLE_LINK_OUTPUT

typedef union {
    T_void *p_data ;
    T_word32 count ;
} T_dllCountOrData ;

typedef struct _T_doubleLinkListStruct {
    struct _T_doubleLinkListStruct *p_next ;
    struct _T_doubleLinkListStruct *p_previous ;
    struct _T_doubleLinkListStruct *p_head ;
    T_dllCountOrData countOrData ;
#ifndef NDEBUG
    T_word32 tag ;
#endif
} T_doubleLinkListStruct ;

/* Internal prototypes: */
static T_doubleLinkListStruct *ICreateNode(T_void) ;
static T_void IDestroyNode(T_doubleLinkListStruct *p_node) ;

static T_word32 G_numNodes = 0 ;
static T_word32 G_maxNodes = 0 ;
static E_Boolean G_doOutput = FALSE ;

#define MAX_ALLOCATED_NODES 30000
static T_doubleLinkListStruct G_nodes[MAX_ALLOCATED_NODES] ;
static T_word16 G_firstFreeNode = 0xFFFF ;
static T_word16 G_numAllocatedNodes = 0 ;

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListCreate
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListCreate creates a new double link list and sets up
 *  the list to receive new nodes.
 *
 *  @return Created double link list.
 *
 *<!-----------------------------------------------------------------------*/
#ifndef NDEBUG
T_void IDumpMaxCount(T_void)
{
    printf("Currently allocated nodes: %ld\n", G_numNodes) ;
    printf("Max double link nodes: %ld\n", G_maxNodes) ;
}
#endif

T_doubleLinkList DoubleLinkListCreate(T_void)
{
    T_doubleLinkListStruct *p_head ;

    DebugRoutine("DoubleLinkListCreate") ;

#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!A 1 list_%s\n", DebugGetCallerName()) ;
#endif
    p_head = ICreateNode() ;
    DebugCheck(p_head != NULL) ;

    if (p_head)  {
        p_head->p_next = p_head ;        /* Next is self. */
        p_head->p_previous = p_head ;    /* Previous is self. */
        p_head->p_head = p_head ;        /* Self points to self. */
        p_head->countOrData.count = 0 ;  /* No elements. */
    }

#ifndef NDEBUG
    if (G_doOutput==FALSE)  {
        G_doOutput = TRUE ;
        atexit(IDumpMaxCount) ;
    }
#endif

    DebugEnd() ;

    return (T_doubleLinkList) p_head ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListDestroy
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListDestroy goes through a list of double link list nodes
 *  and detaches the data with them and then deletes all the nodes.
 *
 *  NOTE:
 *  The calling routine must realize that this routine does not
 *  deallocate memory attached to a node, it is assumed that the calling
 *  routine will do this.
 *
 *  @param linkList -- Handle to link list to destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void DoubleLinkListDestroy(T_doubleLinkList linkList)
{
    T_doubleLinkListStruct *p_head ;

    DebugRoutine("DoubleLinkListDestroy") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!F 1 list_%s\n", DebugGetCallerName()) ;
#endif
    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Remove all items in the list.  Hopefully */
    /* all attached data is not allocated memory or is being */
    /* managed by someone else. */
    while (p_head->p_next != p_head)
        DoubleLinkListRemoveElement((T_doubleLinkListElement)p_head->p_next) ;

    /* Destroy this element now. */
    IDestroyNode(p_head) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListAddElementAtEnd
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListAddElementAtEnd appends a new element at the end of the
 *  link list.
 *
 *  @param linkList -- Handle to link list to append item
 *  @param p_data -- Pointer to element data
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListAddElementAtEnd(
                            T_doubleLinkList linkList,
                            T_void *p_data)
{
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListStruct *p_element ;

    DebugRoutine("DoubleLinkListAddElementAtEnd") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;
    DebugCheck(p_head->p_head == p_head) ;

    if (p_head)  {
        /* Create a new element. */
#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!A 1 node_%s\n", DebugGetCallerName()) ;
#endif
        p_element = ICreateNode() ;
        DebugCheck(p_element != NULL) ;

        if (p_element)  {
            /* Attach the data to the element. */
            p_element->countOrData.p_data = p_data ;
            p_element->p_previous = p_head->p_previous ;
            p_element->p_next = p_head ;
            p_element->p_head = p_head ;
            p_head->p_previous->p_next = p_element ;
            p_head->p_previous = p_element ;
            p_head->countOrData.count++ ;
        }
    }

    DebugEnd() ;

    return ((T_doubleLinkListElement)p_element) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListAddElementAtFront
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListAddElementAtFront adds a new element at the front of
 *  the link list.
 *
 *  @param linkList -- Handle to link list to insert item
 *  @param p_data -- Pointer to element data
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListAddElementAtFront(
                            T_doubleLinkList linkList,
                            T_void *p_data)
{
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListStruct *p_element ;

    DebugRoutine("DoubleLinkListAddElementAtFront") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;
    DebugCheck(p_head->p_head == p_head) ;

    if (p_head)  {
        /* Create a new element. */
#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!A 1 node_%s\n", DebugGetCallerName()) ;
#endif
        p_element = ICreateNode() ;
        DebugCheck(p_element != NULL) ;

        if (p_element)  {
            /* Attach the data to the element. */
            p_element->countOrData.p_data = p_data ;
            p_element->p_previous = p_head ;
            p_element->p_next = p_head->p_next ;
            p_element->p_head = p_head ;
            p_head->p_next->p_previous = p_element ;
            p_head->p_next = p_element ;
            p_head->countOrData.count++ ;
        }
    }

    DebugEnd() ;

    return ((T_doubleLinkListElement)p_element) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListAddElementAfterElement
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListAddElementAfterElement adds a new element after
 *  another element in a link list.
 *
 *  @param element -- handle of element to go after
 *  @param p_data -- Pointer to element data
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListAddElementAfterElement(
                            T_doubleLinkListElement element,
                            T_void *p_data)
{
    T_doubleLinkListStruct *p_after ;
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListStruct *p_element ;

    DebugRoutine("DoubleLinkListAddElementAfterElement") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_after = (T_doubleLinkListStruct *)element;
    DebugCheck(p_after->tag == DOUBLE_LINK_LIST_TAG) ;
    p_head = p_after->p_head ;
    DebugCheck(p_head != NULL) ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;
    DebugCheck(p_head->p_head == p_head) ;

    if ((p_head) && (p_after))  {
        /* Create a new element. */
#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!A 1 node_%s\n", DebugGetCallerName()) ;
#endif
        p_element = ICreateNode() ;
        DebugCheck(p_element != NULL) ;

        if (p_element)  {
            /* Attach the data to the element. */
            p_element->countOrData.p_data = p_data ;
            p_element->p_previous = p_after ;
            p_element->p_next = p_after->p_next ;
            p_element->p_head = p_head ;
            p_after->p_next->p_previous = p_element ;
            p_after->p_next = p_element ;
            p_head->countOrData.count++ ;
        }
    }

    DebugEnd() ;

    return ((T_doubleLinkListElement)p_element) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListAddElementBeforeElement
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListAddElementBeforeElement adds a new element before
 *  another element in a link list.
 *
 *  @param element -- handle of element to go before
 *  @param p_data -- Pointer to element data
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListAddElementBeforeElement(
                            T_doubleLinkListElement element,
                            T_void *p_data)
{
    T_doubleLinkListStruct *p_before ;
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListStruct *p_element ;

    DebugRoutine("DoubleLinkListAddElementBeforeElement") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_before = (T_doubleLinkListStruct *)element;
    DebugCheck(p_before->tag == DOUBLE_LINK_LIST_TAG) ;
    p_head = p_before->p_head ;
    DebugCheck(p_head != NULL) ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;
    DebugCheck(p_head->p_head == p_head) ;

    if ((p_head) && (p_before))  {
        /* Create a new element. */
#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!A 1 node_%s\n", DebugGetCallerName()) ;
#endif
        p_element = ICreateNode() ;
        DebugCheck(p_element != NULL) ;

        if (p_element)  {
            /* Attach the data to the element. */
            p_element->countOrData.p_data = p_data ;
            p_element->p_previous = p_before->p_previous ;
            p_element->p_next = p_before ;
            p_element->p_head = p_head ;
            p_before->p_previous->p_next = p_element ;
            p_before->p_previous = p_element ;
            p_head->countOrData.count++ ;
        }
    }

    DebugEnd() ;

    return ((T_doubleLinkListElement)p_element) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListGetNumberElements
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListGetNumberElements returns the number of elements in
 *  the given link list.
 *
 *  @param linkList -- Link list to get count of
 *
 *  @return Number of elements
 *
 *<!-----------------------------------------------------------------------*/
T_word32 DoubleLinkListGetNumberElements(T_doubleLinkList linkList)
{
    T_doubleLinkListStruct *p_head ;
    T_word32 count ;

    DebugRoutine("DoubleLinkListGetNumberElements") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    count = p_head->countOrData.count ;

    DebugEnd() ;

    return count ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListRemoveElement
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListRemoveElement removes  an element from a link list.
 *  This routine also returns the data that was attached to this element.
 *
 *  @param element --  removes the element from the list.
 *
 *  @return Previously attached node data
 *
 *<!-----------------------------------------------------------------------*/
T_void *DoubleLinkListRemoveElement(T_doubleLinkListElement element)
{
    T_doubleLinkListStruct *p_node ;
    T_void *p_data = NULL ;

    DebugRoutine("DoubleLinkListRemoveElement") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_node = (T_doubleLinkListStruct *)element;
    DebugCheck(p_node->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Make sure we are not trying to delete the head. */
    DebugCheck(p_node->p_head != p_node) ;

    if (p_node)  {
        /* Detach the node from the list. */
        p_node->p_next->p_previous = p_node->p_previous ;
        p_node->p_previous->p_next = p_node->p_next ;

        /* Get the attached data. */
        p_data = p_node->countOrData.p_data ;

        /* Decrement the count of items on this list. */
        p_node->p_head->countOrData.count-- ;

        /* Release the node from memory. */
#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!F 1 node_%s\n", DebugGetCallerName()) ;
#endif
        IDestroyNode(p_node) ;
    }

    DebugEnd() ;

    return p_data ;
}


/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListTraverse
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListTraverse travels along the link list and calls the
 *  given callback for each of the nodes.  In addition, the callback
 *  has the choice of aborting the routine.  If the traversal stops early
 *  that element's handle is returned, else DOUBLE_LINK_LIST_ELEMENT_BAD
 *  is returned.
 *
 *  @param linkList -- List to traverse
 *  @param callback -- callback to call on each
 *      element, returns FALSE
 *      to stop traversing.
 *
 *  @return Element that traverse stopped on,
 *      or DOUBLE_LINK_LIST_ELEMENT_NONE if
 *      completed list.
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListTraverse(
                            T_doubleLinkList linkList,
                            T_doubleLinkListTraverseCallback callback)
{
    T_doubleLinkListStruct *p_at = NULL ;
    T_doubleLinkListStruct *p_next = NULL ;
    T_doubleLinkListStruct *p_head ;

    DebugRoutine("DoubleLinkListTraverse") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;
    DebugCheck(callback != NULL) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    if (p_head)  {
        p_at = p_head->p_next ;
        while ((p_at != p_head) && (p_at != DOUBLE_LINK_LIST_ELEMENT_BAD))  {
            p_next = p_at->p_next ;
            if (callback(p_at) == FALSE)
                break ;
            p_at = p_next ;
        }
        if (p_at == p_head)
            p_at = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    }

    DebugEnd() ;

    return ((T_doubleLinkListElement)p_at) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListElementGetData
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListElementGetData pulls out the data pointer from a
 *  double link list element.
 *
 *  @param element -- Element to get data out of
 *
 *  @return Found data pointer on element
 *
 *<!-----------------------------------------------------------------------*/
T_void *DoubleLinkListElementGetData(T_doubleLinkListElement element)
{
    T_void *p_data ;
    T_doubleLinkListStruct *p_node ;

    DebugRoutine("DoubleLinkListElementGetData") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_node = (T_doubleLinkListStruct *)element;
    DebugCheck(p_node->tag != DOUBLE_LINK_LIST_DEAD_TAG);
    DebugCheck(p_node->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Make sure we are not trying to get data from the head. */
    DebugCheck(p_node->p_head != p_node) ;

    p_data = p_node->countOrData.p_data ;

    DebugEnd() ;

    return p_data ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListElementGetNext
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListElementGetData pulls out the next element from a
 *  double link list element.
 *
 *  @param element -- Element to get next element
 *
 *  @return Next element
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListElementGetNext(
                            T_doubleLinkListElement element)
{
    T_doubleLinkListElement nextElement ;
    T_doubleLinkListStruct *p_node ;

    DebugRoutine("DoubleLinkListElementGetNext") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_node = (T_doubleLinkListStruct *)element;
    DebugCheck(p_node->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Make sure we are not trying to get data from the head. */
    DebugCheck(p_node->p_head != p_node) ;

    nextElement = (T_doubleLinkListElement)p_node->p_next ;

    /* Check to see if we found the head. */
    if (p_node->p_next->p_head == p_node->p_next)
        nextElement = DOUBLE_LINK_LIST_ELEMENT_BAD ;

    DebugEnd() ;

    return nextElement ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListElementGetPrevious
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListElementGetPrevious pulls out the previous element from a
 *  double link list element.
 *
 *  @param element -- Element to get previous element
 *
 *  @return Previous element
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListElementGetPrevious(
                            T_doubleLinkListElement element)
{
    T_doubleLinkListElement previousElement ;
    T_doubleLinkListStruct *p_node ;

    DebugRoutine("DoubleLinkListElementGetPrevious") ;
    DebugCheck(element != DOUBLE_LINK_LIST_ELEMENT_BAD) ;

    /* Get a quick pointer. */
    p_node = (T_doubleLinkListStruct *)element;
    DebugCheck(p_node->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Make sure we are not trying to get data from the head. */
    DebugCheck(p_node->p_head != p_node) ;

    previousElement = (T_doubleLinkListElement)p_node->p_previous ;

    /* Check to see if we found the head. */
    if (p_node->p_previous->p_head == p_node->p_previous)
        previousElement = DOUBLE_LINK_LIST_ELEMENT_BAD ;

    DebugEnd() ;

    return previousElement ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListGetFirst
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListGetFirst returns the first element in a
 *  double link list.
 *
 *  @param linkList -- List to get first
 *
 *  @return First element, or
 *      DOUBLE_LINK_LIST_ELEMENT_BAD if none.
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListGetFirst(T_doubleLinkList linkList)
{
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListElement first = DOUBLE_LINK_LIST_ELEMENT_BAD ;

    DebugRoutine("DoubleLinkListGetFirst") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    if (p_head)  {
        if (p_head->p_next != p_head)  {
            first = (T_doubleLinkListElement)p_head->p_next ;
        }
    }

    DebugEnd() ;

    return first ;
}

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListGetLast
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListGetLast  returns the last  element in a
 *  double link list.
 *
 *  @param linkList -- List to get last element
 *
 *  @return Last  element, or
 *      DOUBLE_LINK_LIST_ELEMENT_BAD if none.
 *
 *<!-----------------------------------------------------------------------*/
T_doubleLinkListElement DoubleLinkListGetLast(T_doubleLinkList linkList)
{
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListElement last = DOUBLE_LINK_LIST_ELEMENT_BAD ;

    DebugRoutine("DoubleLinkListGetLast") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)linkList ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    if (p_head)  {
        if (p_head->p_previous != p_head)
            last = (T_doubleLinkListElement)p_head->p_previous ;
    }

    DebugEnd() ;

    return last ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICreateNode
 *-------------------------------------------------------------------------*/
/**
 *  ICreateNode is used to create a new node structure item and clean it
 *  up before processing.  All fields are set to zero.
 *
 *  @return Newly create node.
 *
 *<!-----------------------------------------------------------------------*/
static T_doubleLinkListStruct *ICreateNode(T_void)
{
    T_doubleLinkListStruct *p_node ;
    T_word16 newNode ;

    DebugRoutine("ICreateNode") ;

#if 1      /* List method */
        if (G_firstFreeNode == 0xFFFF)  {
            DebugCheck(G_numAllocatedNodes < MAX_ALLOCATED_NODES) ;
            if (G_numAllocatedNodes >= MAX_ALLOCATED_NODES)  {
                GrGraphicsOff() ;
                fprintf(stderr, "Out of node memory!\n") ;
                exit(1001) ;
            }
            newNode = G_numAllocatedNodes++ ;
        } else {
            newNode = G_firstFreeNode ;
            G_firstFreeNode = G_nodes[newNode].countOrData.count ;
        }
        p_node = G_nodes + newNode ;
        memset(p_node, 0, sizeof(T_doubleLinkListStruct)) ;
#ifndef NDEBUG
        p_node->tag = DOUBLE_LINK_LIST_TAG ;
#endif
        G_numNodes++ ;
        if (G_numNodes > G_maxNodes)
            G_maxNodes = G_numNodes ;
#else
    p_node = MemAlloc(sizeof(T_doubleLinkListStruct)) ;
    DebugCheck(p_node != NULL) ;
    if (p_node)  {
        memset(p_node, 0, sizeof(T_doubleLinkListStruct)) ;
#ifndef NDEBUG
        p_node->tag = DOUBLE_LINK_LIST_TAG ;
#endif
        G_numNodes++ ;
        if (G_numNodes > G_maxNodes)
            G_maxNodes = G_numNodes ;
    }
#endif

    DebugEnd() ;

    return p_node ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IDestroyNode
 *-------------------------------------------------------------------------*/
/**
 *  IDestroyNode gets rid of a previously created node.
 *
 *  @param p_node -- node to destroy
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDestroyNode(T_doubleLinkListStruct *p_node)
{
    T_word32 nodeNum ;

    DebugRoutine("IDestroyNode") ;
    DebugCheck(p_node != NULL) ;
    DebugCheck(p_node->tag == DOUBLE_LINK_LIST_TAG) ;

#if 1
    if (p_node)  {
# ifndef NDEBUG
        memset(p_node, 0, sizeof(T_doubleLinkListStruct)) ;
        p_node->tag = DOUBLE_LINK_LIST_DEAD_TAG ;
# endif
        nodeNum = p_node - G_nodes ;
        DebugCheck(nodeNum < MAX_ALLOCATED_NODES);
        G_nodes[nodeNum].countOrData.count = G_firstFreeNode ;
        G_firstFreeNode = nodeNum ;
        G_numNodes-- ;
        DebugCheck(G_numNodes != 0xFFFFFFFF) ;
    }
#else
    if (p_node)  {
# ifndef NDEBUG
        memset(p_node, 0, sizeof(T_doubleLinkListStruct)) ;
        p_node->tag = DOUBLE_LINK_LIST_DEAD_TAG ;
# endif
        MemFree(p_node) ;

        G_numNodes-- ;
        DebugCheck(G_numNodes != 0xFFFFFFFF) ;
    }
#endif

    DebugEnd() ;
}

/* LES: 12/17/95 */
#ifndef NDEBUG
T_void DoubleLinkListDisplay(T_doubleLinkList linkList)
{
    T_doubleLinkListStruct *p_start ;
    T_doubleLinkListStruct *p_place ;

    p_start = (T_doubleLinkListStruct *)linkList ;
    printf("\n\nList starting at %p:\n", p_start) ;
    if (p_start)  {
        p_place = p_start ;
        do {
            printf("  Node: %p\n", p_place) ;
            printf("    p_next: %p\n", p_place->p_next) ;
            printf("    p_prev: %p\n", p_place->p_previous) ;
            printf("    p_head: %p\n", p_place->p_head) ;
            if (p_place == p_place->p_head)  {
                printf("    count: %ld\n", p_place->countOrData.count) ;
            } else {
                printf("    data: %p\n", p_place->countOrData.p_data) ;
            }
            printf("    tag: %ld\n", p_place->tag) ;

            p_place = p_place->p_next ;

        } while (p_place != p_start) ;
    } else {
        printf("  NULL") ;
    }
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  DoubleLinkListFreeAndDestroy
 *-------------------------------------------------------------------------*/
/**
 *  DoubleLinkListFreeAndDestroy goes through a linked list and does a
 *  MemFree on each of the data elements and then calls Destroy on the
 *  whole list.
 *
 *  @param linkList -- Handle to link list to destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void DoubleLinkListFreeAndDestroy(T_doubleLinkList *linkList)
{
    T_doubleLinkListStruct *p_head ;
    T_doubleLinkListStruct *p_at ;
    T_doubleLinkListStruct *p_next ;

    DebugRoutine("DoubleLinkListFreeAndDestroy") ;
    DebugCheck(linkList != DOUBLE_LINK_LIST_BAD) ;

#ifdef COMPILE_OPTION_DOUBLE_LINK_OUTPUT
printf("!F 1 list_%s\n", DebugGetCallerName()) ;
#endif
    /* Get a quick pointer. */
    p_head = (T_doubleLinkListStruct *)(*linkList) ;
    DebugCheck(p_head->tag == DOUBLE_LINK_LIST_TAG) ;

    /* Remove all items in the list.  Hopefully */
    /* all attached data is not allocated memory or is being */
    /* managed by someone else. */
    if (p_head)  {
        p_at = p_head->p_next ;
        while ((p_at != p_head) && (p_at != DOUBLE_LINK_LIST_ELEMENT_BAD))  {
            p_next = p_at->p_next ;
            if (p_at->countOrData.p_data != NULL)  {
                MemFree(p_at->countOrData.p_data) ;
                p_at->countOrData.p_data = NULL ;
            }
            p_at = p_next ;
        }
    }

    DoubleLinkListDestroy(*linkList) ;
    linkList = DOUBLE_LINK_LIST_BAD ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  DBLLINK.C
 *-------------------------------------------------------------------------*/
