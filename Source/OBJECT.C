/****************************************************************************/
/*  FILE:  OBJECT.H                                                         */
/****************************************************************************/
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/05/95  Removed all routines that try to change the picture of */
/*                   an object by any other way that ObjectSetStance or     */
/*                   ObjectSetType.                                         */
/*                      ObjectSetPictureByName                              */
/*                      ObjectSetPictureDirectly                            */
/*    LES  11/21/95  Added IMakeTempPassable and                            */
/*                   ObjectsMakeTemporarilyPassableAtXYRadius pair.         */
/*                   Went through all the code and replace G_First3dObject  */
/*                   references with ObjectsGetFirst().                     */
/*                   Did the same with nextObj and ObjectGetNext()          */
/*                   Did the same with prevObj and ObjectGetPrevious()      */
/*                                                                          */
/****************************************************************************/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "CRELOGIC.H"
#include "MAP.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SCRIPTEV.H"
#include "SYNCMEM.H"
#include "TICKER.H"

#define OBJECT_HASH_TABLE_SIZE 2048
#define OBJECT_HASH_TABLE_MASK (OBJECT_HASH_TABLE_SIZE-1)

typedef struct {
    T_3dObject *table[OBJECT_HASH_TABLE_SIZE] ;
} T_objectHashTable ;

static T_word16 G_lastObjectId = 30000;
static E_Boolean G_objectChainingAllow = TRUE ;
static T_objectHashTable *G_objectHashTable ;
static T_word32 G_numObjectsMarkedForDestroy = 0 ;

/* INTERNAL PROTOTYPES: */
static E_Boolean IMakeTempPassable(T_3dObject *p_obj, T_word32 data) ;
static T_3dObject *IObjectFindBodyPart(
                      T_3dObject *p_body,
                      T_bodyPartLocation location) ;
static T_void IObjectRemoveFromHashTable(T_3dObject *p_obj) ;
static T_void IObjectAddToHashTable(T_3dObject *p_obj) ;

/****************************************************************************/
/*  Routine:  ObjectsInitialize                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsInitialize starts up any object info that is needed by this    */
/*  Object Module.                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memset                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsInitialize(T_void)
{
    DebugRoutine("ObjectsInitialize") ;

    /* Set the object hash table to null. */
    G_objectHashTable = MemAlloc(sizeof(T_objectHashTable)) ;
    memset(G_objectHashTable->table, 0, sizeof(G_objectHashTable->table)) ;

    /* Starting fresh.  No objects are marked for destruction. */
    G_numObjectsMarkedForDestroy = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsFinish                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsFinish cleans up after itself.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsFinish(T_void)
{
    DebugRoutine("ObjectsFinish") ;

    G_numObjectsMarkedForDestroy = 0 ;
    MemFree(G_objectHashTable) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectGetPictureWidth                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetPictureWidth returns how wide the current picture for the    */
/*  given object is.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Warning!  Width may change as the object animates.                    */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get width of                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Width                                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 ObjectGetPictureWidth(T_3dObject *p_obj)
{
    T_word16 width ;

    DebugRoutine("ObjectGetPictureWidth") ;
#ifndef NDEBUG
    if (p_obj->p_picture == NULL)
        ObjectPrint(stdout, p_obj) ;
#endif

    DebugCheck(p_obj != NULL) ;
    DebugCheck(p_obj->p_picture != NULL) ;

    width = PictureGetWidth(p_obj->p_picture) ;

    DebugEnd() ;

    return width ;
}

/****************************************************************************/
/*  Routine:  ObjectGetPictureHeight                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetPictureHeight returns how tall the current picture for the   */
/*  given object is.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Warning!  Height may change as the object animates.                   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get height of                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Height                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 ObjectGetPictureHeight(T_3dObject *p_obj)
{
    T_word16 height ;

    DebugRoutine("ObjectGetPictureHeight") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(p_obj->p_picture != NULL) ;

    height = PictureGetHeight(p_obj->p_picture) ;

    DebugEnd() ;

    return height ;
}

/****************************************************************************/
/*  Routine:  ObjectGetPicture                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetPicture returns the pointer to the exact picture information */
/*  used for the object.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Warning!  Picture may change as the object animates.                  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get height of                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to picture                     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *ObjectGetPicture(T_3dObject *p_obj)
{
    T_byte8 *p_pic ;

    DebugRoutine("ObjectGetPicture") ;

    p_pic = p_obj->p_picture ;

    DebugEnd() ;

    return p_pic ;
}

/****************************************************************************/
/*  Routine:  ObjectGetBitmap                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetBitmap returns a pointer to the picture of the object in     */
/*  bitmap format.  It also grabs the front view.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get height of                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to picture                     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_bitmap *ObjectGetBitmap(T_3dObject *p_obj)
{
    T_bitmap *p_bitmap ;

    DebugRoutine("ObjectGetBitmap") ;

    p_bitmap = (T_bitmap *)(&(((T_sword16 *)p_obj->p_picture)[-2])) ;

    DebugEnd() ;

    return p_bitmap ;
}

/****************************************************************************/
/*  Routine:  ObjectFind                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectFind searches through the object lists for a matching object    */
/*  of the given id.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 objId              -- Id of object to find.                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Pointer to found object, or NULL       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*    LES  07/07/95  Changed to distinguish between id and SeverId          */
/*    AMT  07/12/95  Made it search the OutsideWorld list as well.          */
/*    LES  12/26/95  Changed to use a hashing table for fast accesses.      */
/*                                                                          */
/****************************************************************************/

T_3dObject *ObjectFind(T_word16 id)
{
    T_3dObject *p_found = NULL ;

    DebugRoutine("ObjectFind") ;

    p_found = G_objectHashTable->table[id & OBJECT_HASH_TABLE_MASK] ;

    /* If we still have an object and it is not the correct */
    /* id, move on to the next hashed object. */
    while ((p_found) && (p_found->objServerId != id))
        p_found = ObjectGetHashPointer(p_found) ;

    DebugEnd() ;

    return p_found ;
}

/****************************************************************************/
/*  Routine:  ObjectCreate                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectCreate     allocates a new object into the 3D engine and        */
/*  returns the pointer to that object.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Pointer to newly created object, or    */
/*                                   NULL.                                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dAllocateObject                                                  */
/*    ObjMoveInit                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/28/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*    AMT  07/18/95  Separated the "serverID" spaces of the client and servr*/
/*                                                                          */
/****************************************************************************/

T_3dObject *ObjectCreate(T_void)
{
    T_3dObject *p_obj ;

    DebugRoutine("ObjectCreate") ;

    SyncMemAdd("** ObjectCreate %d from %s\n", G_lastObjectId, (T_word32)DebugGetCallerName(), 0) ;
#ifndef NDEBUG
//    printf("ObjectCreate for Client number %d from %s\n", G_lastObjectId, DebugGetCallerName()) ;
#endif

    /* Just pass on the request and get a pointer to a new object. */
    p_obj = View3dAllocateObject() ;
    ObjMoveInit(&p_obj->objMove) ;
    p_obj->attributes = 0 ;
    p_obj->accessoryData = 0 ;
    p_obj->p_picture = NULL ;
    p_obj->picResource = RESOURCE_BAD ;
    p_obj->orientation = ORIENTATION_NORMAL ;
    p_obj->p_chainedObjects = NULL ;

    /** Keep the client and server on different ID spaces. **/
    p_obj->objServerId = G_lastObjectId++ ;

    p_obj->objUniqueId = 0 ;
    p_obj->p_objType = OBJECT_TYPE_INSTANCE_BAD ;
    p_obj->health = 0 ;
    p_obj->extraData = NULL ;
    p_obj->numPackets = 0;
    p_obj->scaleX = p_obj->scaleY = 65536L ;
    p_obj->script = SCRIPT_BAD ;
    ObjectSetColorizeTable(p_obj, COLORIZE_TABLE_NONE) ;
    p_obj->p_hash = NULL ;
    p_obj->illumination = 0 ;
    p_obj->ownerID = 0 ;
    p_obj->lastSectorSteppedOn = 0xFFFF ;

    strcpy(p_obj->tag, "Obj") ;
    p_obj->inWorld = FALSE ;
    p_obj->elementInObjCollisionList = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    p_obj->objCollisionGroup = OBJ_COLLISION_GROUP_NONE ;

//printf ("** ObjectCreate: ID %d by %s\n", p_obj->objServerId, DebugGetCallerName());

    DebugEnd() ;

    return p_obj ;
}

T_3dObject *ObjectCreateFake(T_void)
{
    T_3dObject *p_obj ;
    E_Boolean oldChaining ;

    DebugRoutine("ObjectCreateFake") ;


#ifndef NDEBUG
//    printf("ObjectCreateFake for Client number %d from %s\n", G_lastObjectId, DebugGetCallerName()) ;
#endif
    /* Just pass on the request and get a pointer to a new object. */
    oldChaining = G_objectChainingAllow ;

    G_objectChainingAllow = FALSE ;

    p_obj = View3dAllocateObject() ;
    ObjMoveInit(&p_obj->objMove) ;
    p_obj->attributes = 0 ;
    p_obj->accessoryData = 0 ;
    p_obj->p_picture = NULL ;
    p_obj->picResource = RESOURCE_BAD ;
    p_obj->orientation = ORIENTATION_NORMAL ;
    p_obj->p_chainedObjects = NULL ;

    p_obj->objUniqueId = 0 ;
    p_obj->p_objType = OBJECT_TYPE_INSTANCE_BAD ;
    p_obj->health = 0 ;
    p_obj->extraData = NULL ;
    p_obj->numPackets = 0;
    p_obj->scaleX = p_obj->scaleY = 65536L ;
    p_obj->script = SCRIPT_BAD ;
    p_obj->lastSectorSteppedOn = 0xFFFF ;
    ObjectSetColorizeTable(p_obj, COLORIZE_TABLE_NONE) ;

    strcpy(p_obj->tag, "Obj") ;
    p_obj->inWorld = FALSE ;
    p_obj->elementInObjCollisionList = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    p_obj->objCollisionGroup = OBJ_COLLISION_GROUP_NONE ;
//printf ("** ObjectCreateFake: ID %d (%p) by %s\n", p_obj->objServerId, p_obj, DebugGetCallerName());

    G_objectChainingAllow = oldChaining ;

    DebugEnd() ;

    return p_obj ;
}

/****************************************************************************/
/*  Routine:  ObjectAdd                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectAdd        attaches a new object to the list of objects in      */
/*  the 3d world.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You MUST have the object server Id defined in the object before       */
/*  calling this routine.  If you don't calls to ObjectFind may not work.   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to bring into world.            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dAddObject                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/26/94  Created                                                */
/*    LES  12/26/95  Added code to put the object on the hash table.        */
/*                                                                          */
/****************************************************************************/

T_void ObjectAdd(T_3dObject *p_obj)
{
    DebugRoutine("ObjectAdd") ;
    DebugCheck (p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
    DebugCheck(p_obj->inWorld == FALSE) ;


//if (ObjectGetServerId(p_obj))
//printf ("** ObjectAdd: ID %d (type %d) by %s\n", ObjectGetServerId (p_obj), ObjectGetType(p_obj),
//           DebugGetCallerName ());


    /* Do the normal without the history. */
    ObjectAddWithoutHistory(p_obj) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectAddWithoutHistory                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectAddWithoutHistory is the same as ObjectAdd, except it does      */
/*  not put the addition into the history hash tables (which are needed     */
/*  to save the game's state).                                              */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You MUST have the object server Id defined in the object before       */
/*  calling this routine.  If you don't calls to ObjectFind may not work.   */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to bring into world.            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/19/96  Created from ObjectAdd                                 */
/*                                                                          */
/****************************************************************************/

T_void ObjectAddWithoutHistory(T_3dObject *p_obj)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    DebugRoutine("ObjectAddWithoutHistory") ;
    DebugCheck (p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
    DebugCheck(p_obj->inWorld == FALSE) ;


//if (ObjectGetServerId(p_obj))
//printf ("** ObjectAddWithoutHistory: ID %d by %s\n", ObjectGetServerId (p_obj),
//           DebugGetCallerName ());


    /* Just pass on the request. */
    View3dAddObject(p_obj) ;

    /* Add the object to the hash table too. */
    IObjectAddToHashTable(p_obj) ;

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* Yes it is.  Is this a piecewise chained object? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* This is a piecewise chained list. */
            p_chainedList = (T_bodyPart *)p_chained ;
            for (i=1; i<MAX_BODY_PARTS; i++)  {
                if (p_chainedList[i].p_obj)  {
                    p_chainedList[i].p_obj->objMove = p_obj->objMove ;
                    ObjectAddWithoutHistory(p_chainedList[i].p_obj) ;
                }
            }
        } else {
            /* This is a chained list, but not piecewise. */
            ObjectAddWithoutHistory(p_chained) ;
        }
    }

    p_obj->inWorld = TRUE ;

    ObjectUpdateCollisionLink(p_obj) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectRemove                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectRemove   detaches an    object from the list of objects in      */
/*  the 3d world.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to bring into world.            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dRemoveObject                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/26/94  Created                                                */
/*    LES  12/26/95  Now removes the object from the hashing table          */
/*                                                                          */
/****************************************************************************/

T_void ObjectRemove(T_3dObject *p_obj)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;
    static E_Boolean isInsideObjectRemove = FALSE ;

    DebugRoutine("ObjectRemove") ;

    DebugCheck (p_obj != NULL) ;
#ifndef NDEBUG
    if (strcmp(p_obj->tag, "Obj") != 0)  {
        ObjectPrint(stdout, p_obj) ;
    }
#endif
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
#ifndef NDEBUG
    if (p_obj->inWorld == FALSE)  {
        ObjectPrint(stdout, p_obj) ;
    }
#endif
    DebugCheck(p_obj->inWorld == TRUE) ;


//if (ObjectGetServerId(p_obj))
//printf ("** ObjectRemove: ID %d by %s\n",
//        ObjectGetServerId (p_obj),
//        DebugGetCallerName ());


    /* Remove the object from the hashing table. */
    IObjectRemoveFromHashTable(p_obj) ;

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* Yes it is chained.  Is it piecewise? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* This is a piecewise chained object. */
            p_chainedList = (T_bodyPart *)p_chained ;

            /* Go through the list and remove the body parts. */
            isInsideObjectRemove = TRUE ;
            for (i=1; i<MAX_BODY_PARTS; i++)
                if (p_chainedList[i].p_obj)
                    ObjectRemove(p_chainedList[i].p_obj) ;
            isInsideObjectRemove = FALSE ;
        } else {
            /* This is not piecewise, but chained. */
            ObjectRemove(p_chained) ;
        }
    }

    /* Pass on the request. */
    View3dRemoveObject(p_obj) ;

    p_obj->inWorld = FALSE ;

    ObjectUnlinkCollisionLink(p_obj) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectDestroy                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectDestroy     frees up an object in memory given the object       */
/*  pointer.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Pointer to object to destroy.          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    View3dFreeObject                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/28/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*    LES  09/22/95  Added code to destroy chained objects                  */
/*                                                                          */
/****************************************************************************/

T_void ObjectDestroy(T_3dObject *p_obj)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    DebugRoutine("ObjectDestroy") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
    DebugCheck(p_obj->inWorld == FALSE) ;

    ObjectUnlinkCollisionLink(p_obj) ;
//if (ObjectGetServerId(p_obj))
//printf ("** ObjectDestroy: ID %d (%d) by %s\n", p_obj->objServerId, ObjectGetType(p_obj), DebugGetCallerName ());


    /* Is this a chained object being destroyed? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* Yes, it is chained. */
        /* Is this a piecewise object? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* Yes, it is piecewise. */
            p_chainedList = (T_bodyPart *)p_chained ;

            /* Go through the list of body parts and destroy them all */
            for (i=1; i<MAX_BODY_PARTS; i++)
                if (p_chainedList[i].p_obj)  {
                    ObjectDestroy(p_chainedList[i].p_obj) ;
                    p_chainedList[i].p_obj = NULL ;
                }

            /* Get rid of the whole list. */
            MemFree(p_chainedList) ;
        } else {
            /* No, it is not piecewise. */
            /* Destroy its partner. */
            ObjectDestroy(p_chained) ;
            ObjectSetChainedObjects(p_obj, NULL) ;
        }
    }

    /* Remove the object's picture. */
    ObjectSetType(p_obj, OBJECT_TYPE_NONE) ;

    /* Decrement the count of requested objects to destroy */
    /* if this object was marked for destruction. */
    if (ObjectIsMarkedForDestroy(p_obj))
        G_numObjectsMarkedForDestroy-- ;

//#ifndef NDEBUG
    strcpy(p_obj->tag, "DOj") ;
//#endif

    /* Pass on the request to free. */
    View3dFreeObject(p_obj) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectDeclareStatic                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectDeclareStatic takes a previously created object (by             */
/*  ObjectCreate) and fills out the information necessary to make           */
/*  it a static object on the map.  It uses the given x, y, and picture     */
/*  number to initialize it.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Pointer to object to declare           */
/*                                                                          */
/*    T_sword16 mapX              -- X coordinate on map                    */
/*                                                                          */
/*    T_sword16 mapY              -- Y coordinate on map                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/28/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  04/11/95  Added sector to object info array                      */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectDeclareStatic(
           T_3dObject *p_obj,
           T_sword16 mapX,
           T_sword16 mapY)
{
    T_word16 sector ;

    DebugRoutine("ObjectDeclareStatic") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    sector = View3dFindSectorNum(mapX, mapY) ;
    if (sector != 0xFFFF)  {
        ObjectSetX16(p_obj, mapX) ;
        ObjectSetY16(p_obj, mapY) ;
        ObjectSetAngle(p_obj, 0) ;
        ObjectSetType(p_obj, 0) ;
        ObjectSetAttributes(p_obj, 0) ;
        ObjectSetZ16(p_obj, MapGetWalkingFloorHeight(sector)) ;
        ObjectSetRadius(p_obj, 32) ;
        ObjectSetUpSectors(p_obj) ;
    }

    //// !!! Need to do something with the pictures!

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectDeclareMoveable                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectDeclareMoveable takes a previously created object (by           */
/*  ObjectCreate) and fills out the information necessary to make           */
/*  it a moveable object on the map.  It uses the given x, y, and picture   */
/*  number to initialize it.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Number of object to declare            */
/*                                                                          */
/*    T_word16 mapX               -- X accurate location on map             */
/*                                                                          */
/*    T_word16 mapY               -- Y accurate location on map             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/28/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  04/11/95  Added code to set object sector and water heights      */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectDeclareMoveable(
           T_3dObject *p_obj,
           T_word16 mapX,
           T_word16 mapY)
{
    T_word16 sector ;

    DebugRoutine("ObjectDeclareMoveable") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    sector = View3dFindSectorNum(mapX, mapY) ;
    if (sector != 0xFFFF)  {
        ObjectSetX16(p_obj, mapX) ;
        ObjectSetY16(p_obj, mapY) ;
        ObjectSetAngle(p_obj, 0) ;
        ObjectSetType(p_obj, 0) ;
        ObjectSetAttributes(p_obj, 0) ;
        ObjectSetZ16(p_obj, MapGetWalkingFloorHeight(sector)) ;
        ObjectSetRadius(p_obj, 32) ;
        ObjectSetUpSectors(p_obj) ;
    }

    //// !!! Need to do something with the pictures!

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectTeleport                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectTeleport is one of the key players in getting things to work.   */
/*  This nice routine moves any object (even static ones) to the new        */
/*  location.  Note that if you move a static object, it becomes a movable  */
/*  object (and is no longer static).  If you want to move a static object  */
/*  without changing its type, use two calls to ViewChangeObject -- one     */
/*  to erase the old object, one to add an object.                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum             -- Number of object to affect.            */
/*                                                                          */
/*    T_word16 x                  -- Accurate map X position to move to.    */
/*                                                                          */
/*    T_word16 y                  -- Accurate map Y position to move to.    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/04/94  Created                                                */
/*    LES  02/21/95  Modified for new 3D engine                             */
/*    LES  04/11/95  Added code to set object sector and water heights      */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectTeleport(T_3dObject *p_obj, T_sword16 x, T_sword16 y)
{
    T_word16 sector ;
    T_sword16 z ;

    DebugRoutine("ObjectTeleport") ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    sector = View3dFindSectorNum(x, y) ;

    /* Only teleport if going to a valid sector. */
    if (sector != 0xFFFF)  {
        z = MapGetWalkingFloorHeight(sector) ;
        View3dSetExceptObjectByPtr(NULL) ;

        /* Make sure no objects are in the way at that location. */
//        if (!ObjectCheckCollide(p_obj, x, y, z))  {
            if (!ObjectCheckIfCollide(p_obj, x<<16, y<<16, z<<16))  {
                ObjectSetX16(p_obj, x) ;
                ObjectSetY16(p_obj, y) ;
                ObjectSetUpSectors(p_obj) ;

                /* Make sure it is known that we zipped over somewhere. */
                ObjectSetMovedFlag(p_obj) ;
            }
//        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectTeleportAlways                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectTeleportAlways is just like ObjectTeleport, but the object      */
/*  goes to the position no matter if there is another object there.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum             -- Number of object to affect.            */
/*                                                                          */
/*    T_word16 x                  -- Accurate map X position to move to.    */
/*                                                                          */
/*    T_word16 y                  -- Accurate map Y position to move to.    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/31/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectTeleportAlways(T_3dObject *p_obj, T_sword16 x, T_sword16 y)
{
    T_word16 sector ;

    DebugRoutine("ObjectTeleportAlways") ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    sector = View3dFindSectorNum(x, y) ;
    if (sector != 0xFFFF)
        ObjectSetZ16(p_obj, MapGetFloorHeight(sector)) ;

/* Debug version of this routine. */
#ifndef NDEBUG
    if (sector != 0xFFFF)  {
        ObjectSetX16(p_obj, x) ;
        ObjectSetY16(p_obj, y) ;

        ObjectSetUpSectors(p_obj) ;

        /* Make sure it is known that we zipped over somewhere. */
        ObjectSetMovedFlag(p_obj) ;
    } else {
//            DebugCheck(FALSE) ;
    }
#else
/* Non-debug version of this routine. */
    ObjectSetX16(p_obj, x) ;
    ObjectSetY16(p_obj, y) ;

    ObjectSetUpSectors(p_obj) ;

    /* Make sure it is known that we zipped over somewhere. */
    ObjectSetMovedFlag(p_obj) ;
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectMakeImpassable                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectMakeImpassable     clears the passibility bit in the object.    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Ojbect to affect                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectMakeImpassable(T_3dObject *p_obj)
{
    DebugRoutine("ObjectMakeImpassable") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    p_obj->attributes &= (~OBJECT_ATTR_PASSABLE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectMakePassable                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectMakePassable     sets the passibility bit in the object.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objNum             -- object to affect                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectSetType                                                         */
/*    ObjectGetType                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectMakePassable(T_3dObject *p_obj)
{
    DebugRoutine("ObjectMakePassable") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    p_obj->attributes |= (OBJECT_ATTR_PASSABLE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectCheckCollide                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectCheckCollide     sees if the position given for an object will  */
/*  cause it to collide with any of the other objects.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject p_obj            -- object to check for object collision   */
/*                                                                          */
/*    T_sword16 x, y              -- position to check for collision        */
/*                                                                          */
/*    T_sword16 height            -- New height to check for                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectSetType                                                         */
/*    ObjectGetType                                                         */
/*    ObjectGetHeight                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*    LES  09/14/95  Modified to handle height as well as position          */
/*                                                                          */
/****************************************************************************/

E_Boolean ObjectCheckCollide(
              T_3dObject *p_obj,
              T_sword16 x,
              T_sword16 y,
              T_sword16 height)
{
    E_Boolean status = FALSE ;
    T_word16 radius ;
    T_sword16 zBottom, zTop ;
    T_objMoveStruct objMove ;

    DebugRoutine("ObjectCheckCollide") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    if (!ObjectIsFullyPassable(p_obj))  {
        objMove = p_obj->objMove ;

        zBottom = height ;
        zTop = height + ObjectGetHeight(p_obj) ;

        View3dSetExceptObjectByPtr(&(p_obj->objMove)) ;
        radius = ObjectGetRadius(p_obj) ;

        /* Check to see if hitting another object. */
        G_numHits = 0 ;
        status = View3dObjectHitFast(
                     x,
                     y,
                     radius,
                     x,
                     y,
                     zBottom,
                     zTop,
                     ObjectGetHeight(p_obj),
                     p_obj) ;

        p_obj->objMove = objMove ;
    }

    DebugEnd() ;

    return status ;
}

/****************************************************************************/
/*  Routine:  ObjectGetMiddleHeight                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetMiddleHeight calculates the height of the mid section of     */
/*  an object.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get middle height of         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_sword16                   -- middle height of object.               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_sword16 ObjectGetMiddleHeight(T_3dObject *p_obj)
{
    T_sword16 height ;

    DebugRoutine("ObjectGetMiddleHeight") ;
    DebugCheck(p_obj != NULL) ;
#ifndef NDEBUG
    if (strcmp(p_obj->tag, "Obj") != 0)
        printf("bad object %p\n", p_obj) ;
#endif
    if (strcmp(p_obj->tag, "Obj") != 0)
        p_obj = NULL ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    height = ObjectGetZ16(p_obj) + (ObjectGetHeight(p_obj)>>1) ;

    DebugEnd() ;

    return height ;
}

/****************************************************************************/
/*  Routine:  ObjectsUnload                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureUnlock                                                         */
/*    PictureUnload                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/11/95  Created                                                */
/*    LES  06/22/95  Moved from VIEW.C to OBJECT.C and renamed              */
/*                                                                          */
/****************************************************************************/

T_void ObjectsUnload(T_void)
{
    T_3dObject *p_obj ;
    T_3dObject *p_next ;

    DebugRoutine("ObjectsUnload") ;

    /* Destroy chained objects first. */
    p_obj = ObjectsGetFirst() ;
    while (p_obj != NULL)  {
        /* What's next before we destroy this object. */
        p_next = ObjectGetNext(p_obj) ;

        if (ObjectGetChainedObjects(p_obj))  {
            /* If there is any added data, destroy that. */
//            ObjectFreeExtraData(p_obj) ;
            if (ObjectGetScriptHandle(p_obj))
                ScriptUnlock(ObjectGetScriptHandle(p_obj)) ;

            /* Get rid of the object. */
            ObjectRemove(p_obj) ;
            ObjectDestroy(p_obj) ;

            /* start over. */
            p_obj = ObjectsGetFirst() ;
        } else {
            /* Next. */
            p_obj = p_next ;
        }
    }

    p_obj = ObjectsGetFirst() ;
    while (p_obj != NULL)  {
        /* If there is any added data, destroy that. */
//        if (p_obj->extraData != NULL)  {
/* !!! Need to free scripts !!!
            if (ObjectGetScript(p_obj))
                ScriptUnlock((T_script)p_obj->extraData) ;
            else
*/
//            ObjectFreeExtraData(p_obj) ;
//        }

        /* What's next before we destroy this object. */
        p_next = ObjectGetNext(p_obj) ;

        /* Get rid of the object. */
        ObjectRemove(p_obj) ;
        ObjectDestroy(p_obj) ;
        p_obj = ObjectsGetFirst() ;
    }

    /* Note that there are no more objects. */
    G_First3dObject = NULL ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectSetUpSectors                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectSetUpSectors determines the sectors that the object is over     */
/*  and transfers this information into the object.                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to set up the sector            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetUpSectors(T_3dObject *p_obj)
{
    DebugRoutine("ObjectSetUpSectors") ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    ObjMoveSetUpSectors(&p_obj->objMove) ;
    ObjectUpdateCollisionLink(p_obj) ;

    DebugEnd() ;
}


#ifndef NDEBUG
/****************************************************************************/
/*  Routine:  ObjectPrint                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectPrint dumps out the given object information to the given       */
/*  output io port.                                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    FILE *fp                    -- File to output object                  */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to print                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    fprintf                                                               */
/*    ObjMovePrint                                                          */
/*    PicturePrint                                                          */
/*    ResourcePrint                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectPrint(FILE *fp, T_3dObject *p_obj)
{
    DebugRoutine("ObjectPrint") ;

    if (strcmp(p_obj->tag, "Obj") != 0)
        fprintf(fp, "OBJECT %p IS BAD!!!\n", p_obj) ;

    fprintf(fp, "\n\nObject: %p\n", p_obj) ;
    fprintf(fp, "tag: %4.4s (%02X %02X %02X %02X)\n", p_obj->tag,
                      p_obj->tag[0],
                      p_obj->tag[1],
                      p_obj->tag[2],
                      p_obj->tag[3]) ;
    ObjMovePrint(fp, &p_obj->objMove) ;
    fprintf(fp, "  type: %p\n", p_obj->objectType) ;
    fprintf(fp, "  attr: %04X\n", p_obj->attributes) ;
    fprintf(fp, "  data: %04X\n", p_obj->accessoryData) ;
    fprintf(fp, "  pic:  %p\n",   p_obj->p_picture) ;
    fprintf(fp, "numPackets: %d\n",   p_obj->numPackets) ;
    if (p_obj->p_picture != NULL)
        PicturePrint(fp, p_obj->p_picture) ;
    else
        printf("    PICTURE BAD\n") ;
    fprintf(fp, "  res:  %p\n",   p_obj->picResource) ;
    if (p_obj->picResource != RESOURCE_BAD)
        ResourcePrint(fp, p_obj->picResource) ;
    else
        printf("    RESOURCE BAD\n") ;
    fprintf(fp, "  orie: %d\n", p_obj->orientation) ;
    fprintf(fp, "  server id:   %d\n", p_obj->objServerId) ;
    fprintf(fp, "  unique id:   %ld\n", p_obj->objUniqueId) ;
    fprintf(fp, "  next: %p\n", ObjectGetNext(p_obj)) ;
    fprintf(fp, "  prev: %p\n", ObjectGetPrevious(p_obj)) ;
    fprintf(fp, "  objType: %p\n", p_obj->p_objType) ;
    fprintf(fp, "  extraData: %p\n", p_obj->extraData) ;

    if (p_obj->inWorld)
        fprintf(fp, "  where: In world\n") ;
    else
        fprintf(fp, "  where: Out world\n") ;

    fflush(fp) ;

    DebugEnd() ;
}

#endif

/****************************************************************************/
/*  Routine:  ObjectsRemoveExtra                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsRemoveExtra removes and destroys any extra objects that are    */
/*  not in use on this level.  This is useful to get rid of extra monsters  */
/*  that are turned off.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectRemove                                                          */
/*    ObjectDestory                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/28/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsRemoveExtra(T_void)
{
    T_3dObject *p_obj ;
    T_3dObject *p_next ;

    DebugRoutine("ObjectsRemoveExtra") ;

    p_obj = ObjectsGetFirst() ;
    while (p_obj != NULL)  {
        p_next = ObjectGetNext(p_obj) ;

        /* Is this an intelligent creature, and has it been ignored? */
        if ((ObjectGetScript(p_obj) != 0) &&
            (!(ObjectGetAttributes(p_obj) & OBJECT_ATTR_WEAPON)) &&
            (p_obj->extraData == NULL))  {
            /* No, destroy the sucker. */
            ObjectRemove(p_obj) ;
            ObjectDestroy(p_obj) ;

            /* We have to start again since the list may have changed */
            /* greatly. */
            p_obj = ObjectsGetFirst() ;
        } else  {
            /* On to the next object. */
            p_obj = p_next ;
        }
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ObjectsUpdateMovement                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsUpdateMovement goes through all the objects and updates their  */
/*  object move structures and does the appropriate movement actions.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 delta              -- Delta of time since last update        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjMoveUpdate                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/29/95  Created                                                */
/*    AMT  07/18/95  Modified so it handles large deltas.                   */
/*                                                                          */
/****************************************************************************/

T_void ObjectsUpdateMovement(T_word32 delta)
{
    T_3dObject *p_obj ;
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;
    T_sword32 x, y, z ;
    T_3dObject *p_child ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    DebugRoutine("ObjectsUpdateMovement") ;

    TICKER_TIME_ROUTINE_START() ;

    if (delta != 0)  {
        /* Cap the amount of time that might of passed. */
        if (delta > 35)
            delta = 35 ;

        /** What if delta is too big? **/
        /** I'll break it up into pieces. **/
        if (delta > 20)
        {
            ObjectsUpdateMovement (delta >> 1);
            ObjectsUpdateMovement ((delta >> 1) + (delta & 1));
        }

        /* No more exceptions. */
        View3dSetExceptObjectByPtr(NULL) ;

        for (p_obj = ObjectsGetFirst();
             p_obj != NULL;
             p_obj = ObjectGetNext(p_obj))  {

/** Only valid for a client+server build. **/
#ifndef SERVER_ONLY
            if (PlayerGetObject() != p_obj)
#endif

            {
                /* Update the object's script if it has one. */
                if (ObjectGetScriptHandle(p_obj) != NULL)
                    ScriptEvent(
                        ObjectGetScriptHandle(p_obj),
                        SCRIPT_EVENT_TIME_UPDATE,
                        SCRIPT_DATA_TYPE_32_BIT_NUMBER,
                        &delta,
                        SCRIPT_DATA_TYPE_NONE,
                        NULL,
                        SCRIPT_DATA_TYPE_NONE,
                        NULL) ;

                /* Is this a creature or an object? */
                if (p_obj->extraData != NULL)  {
                    /* Creature based logic. */
                    if (CreatureIsMissile(p_obj))  {
                        /* Normal wall collision info. */
                        Collide3dSetWallDefinition(LINE_IS_IMPASSIBLE) ;
                    } else {
                        Collide3dSetWallDefinition(
                            LINE_IS_IMPASSIBLE |
                            LINE_IS_CREATURE_IMPASSIBLE) ;
                    }
                } else {
                    /* Normal wall collision info. */
                    Collide3dSetWallDefinition(LINE_IS_IMPASSIBLE) ;
                }

                ObjMoveUpdate(&p_obj->objMove, delta) ;

                /* Update the collision links. */
                ObjectUpdateCollisionLink(p_obj) ;

                /* If I made a sucessful step, make me impassible again. */
    /*
                if (!(ObjectWasBlocked(p_obj)))
                    if (ObjectIsMarkedMakeImpassibleWhenFree(p_obj))
                        ObjectMakeImpassable(p_obj) ;
    */

                /* Are there other objects chained to this one? */
                p_chained = ObjectGetChainedObjects(p_obj) ;
                if (p_chained)  {
                    /* Yes, there is a chain. */

                    /* Note where the root object is located. */
                    x = ObjectGetX(p_obj) ;
                    y = ObjectGetY(p_obj) ;
                    z = ObjectGetZ(p_obj) ;

                    /* Is this a piecewise object or */
                    /*   just a single chain object? */
                    if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
                        /* It is piecewise. */
                        p_chainedList = (T_bodyPart *)p_chained ;
                        /* Move all the chained objects to the root object. */
                        for (i=1; i<MAX_BODY_PARTS; i++)  {
                            p_child = p_chainedList[i].p_obj ;
                            /* Only bother with objects that are there. */
                            if (p_child)  {
                                /* Move it to the root location. */
                                ObjectSetX(p_child, x) ;
                                ObjectSetY(p_child, y) ;
                                ObjectSetZ(p_child, z) ;
                            }
                        }
                    } else {
                        /* Not piecewise -- therefore, must only be */
                        /* one object. */
                        ObjectSetX(p_chained, x) ;
                        ObjectSetY(p_chained, y) ;
                        ObjectSetZ(p_chained, z) ;
                    }
                }
            }
        }
    }

    TICKER_TIME_ROUTINE_ENDM("ObjectsUpdateMovement", 500) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectSetType                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectSetType changes the object type that is used for this object.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    ALL objects MUST have it's type set at some type to ensure that a     */
/*  picture be used with the object.  Otherwise, it doesn't have a pic.     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to set type of                  */
/*                                                                          */
/*    T_word16 type               -- Type of object to become               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/29/95  Created                                                */
/*    AMT  07/12/95  Made it store the object type number in the structure. */
/*    AMT  07/19/95  Added support for object move attributes in the type.  */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetType(T_3dObject *p_obj, T_word16 type)
{
    T_3dObject *p_chain ;
    T_bodyPart *p_chainList ;
    T_word16 i ;
    T_word16 basicType, basicObjectType ;
    T_word16 permanentAttribs ;

    DebugRoutine("ObjectSetType") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

#if 0
if (type == 0)  {
    printf("!F 1 %d\n", ObjectGetBasicType(p_obj)) ;
    printf("!F 1 OBJ:%s\n", DebugGetCallerName()) ;
} else {
    printf("!A 1 %d\n", type & OBJECT_TYPE_BASIC_MASK) ;
    printf("!A 1 OBJ:%s\n", DebugGetCallerName()) ;
}
#endif

//printf("Object set type: %d to %d by %s\n", ObjectGetServerId(p_obj), type, DebugGetCallerName()) ;  fflush(stdout) ;
//if (type == 27)   // Kludge to fix missing torches
//   type = 11 ;
//if ((type >= 511) && (type <= 516))  {
//   type += (type-510) * 4096 ;
//}
    basicObjectType = ObjectGetBasicType(p_obj) ;
    basicType = type & OBJECT_TYPE_BASIC_MASK ;

    if (basicObjectType != basicType)  {
        if (p_obj->p_objType != OBJECT_TYPE_INSTANCE_BAD)  {
            /* Get rid of that old object type. */
//printf("Destroy and null old %p\n", p_obj->p_objType) ;
            ObjTypeDestroy(p_obj->p_objType) ;
            p_obj->p_objType = NULL ;
        }

        /* If you have a script, remove it. */
        if (p_obj->script)  {
            ScriptUnlock(p_obj->script) ;
            p_obj->script = SCRIPT_BAD ;
        }

        /* Get rid of the extra data attached to this object. */
        if (!(p_obj->attributes & OBJECT_ATTR_PIECE_WISE))  {
            if (ObjectGetExtraData(p_obj) != NULL)
                CreatureDetachFromObject(p_obj) ;
        }

        ObjectFreeExtraData(p_obj) ;

        /* If we still have extra data, then it must be a creature. */
//        if (ObjectGetExtraData(p_obj) != NULL)
            /* Attach a creature structure. */
//            CreatureDetachFromObject(p_obj) ;

        /* Check to see if we are going to a legal type.  If not, we */
        /* WON'T bother with a new type. */
        if (type != OBJECT_TYPE_NONE)  {
//printf("type = %d\n", type) ;
            /* Create a new object type for the given number. */
            p_obj->p_objType = ObjTypeCreate(basicType, p_obj) ;
            DebugCheck(p_obj->p_objType != NULL) ;

            /* Get the object radius and attributes */
            ObjectSetRadius(p_obj, ObjTypeGetRadius(p_obj->p_objType)) ;

            /* Be sure to keep it as a body part if it is one. */
            permanentAttribs = ObjectGetAttributes(p_obj) & OBJECT_ATTR_BODY_PART ;
            ObjectSetAttributes(p_obj,
                permanentAttribs |
                ObjTypeGetAttributes(p_obj->p_objType)) ;

            /* Is this going to be a piecewise object? */
            if ((p_obj->attributes & OBJECT_ATTR_PIECE_WISE) &&
                (G_objectChainingAllow)) {
                /* Yes, it is piecewise. */

                /* Allocate memory for the piecewise description. */
                p_chainList = MemAlloc(sizeof(T_bodyPart) * MAX_BODY_PARTS) ;
                DebugCheck(p_chainList != NULL) ;
                /* Go through the list and initialize them all. */
                for (i=1; i<MAX_BODY_PARTS; i++)  {
                    p_chainList[i].number = -1 ;
                    p_chainList[i].p_obj = NULL ;
#ifndef SERVER_ONLY
                    p_chain = p_chainList[i].p_obj = ObjectCreateFake() ;
                    DebugCheck(p_chain != NULL) ;
                    ObjectSetType(p_chain, (T_word16)(type+i)) ;
//printf("Setting chained object at %d to %d (%p)\n", i, type+i, p_chain) ;

                    ObjectAddAttributes(p_chain,
                        OBJECT_ATTR_BODY_PART |
                        OBJECT_ATTR_INVISIBLE |
                        OBJECT_ATTR_PASSABLE) ;
                    ObjectRemoveAttributes(p_chain,
                        OBJECT_ATTR_PIECE_WISE) ;
#endif
                }
                ObjectSetChainedObjects(p_obj, ((T_3dObject *)p_chainList)) ;
            }

            ObjectSetMoveFlags (p_obj, ObjTypeGetMoveFlags (p_obj->p_objType));
            ObjectSetHeight(p_obj, ObjTypeGetHeight(p_obj->p_objType)) ;
            ObjectSetHealth(p_obj, ObjTypeGetHealth(p_obj->p_objType)) ;

            /** Store the object type number in the structure. **/
            p_obj->objectType = type ;

            /* Attach a new script. */
            if (ObjectGetScript(p_obj))  {
                /* Attach a creature structure. */
                p_obj->objectType = type ;
                CreatureAttachToObject(p_obj) ;
            }
        }
    }

    p_obj->objectType = type ;

    /* Now get the picture for the object type. */
//printf("p_obj->p_objType = %p\n", p_obj->p_objType) ;  fflush(stdout) ;
    if (p_obj->p_objType)  {
        ObjectSetStance(p_obj, 0) ;
        p_obj->p_picture = ObjTypeGetPicture(
                               p_obj->p_objType,
                               0,
                               &p_obj->orientation) ;
    }

#if 1
    if (ObjectGetType(p_obj) == 38)  /* TROPHY TORCH */
        ObjectSetIllumination(p_obj, 255) ;
    if (ObjectGetType(p_obj) == 27)  /* TORCH */
        ObjectSetIllumination(p_obj, 128) ;
    if (ObjectGetBasicType(p_obj) == 41)  /* BRAFIR */
        ObjectSetIllumination(p_obj, 64) ;

#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectSetTypeSimple                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectSetTypeSimple changes the object type that is used for this     */
/*  object.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    ALL objects MUST have it's type set at some type to ensure that a     */
/*  picture be used with the object.  Otherwise, it doesn't have a pic.     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to set type of                  */
/*                                                                          */
/*    T_word16 type               -- Type of object to become               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/29/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetTypeSimple(T_3dObject *p_obj, T_word16 type)
{
    T_word16 basicType, basicObjectType ;

    DebugRoutine("ObjectSetTypeSimple") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    basicObjectType = ObjectGetBasicType(p_obj) ;
    basicType = type & OBJECT_TYPE_BASIC_MASK ;

    if (basicObjectType != basicType)  {
        if (p_obj->p_objType != OBJECT_TYPE_INSTANCE_BAD)  {
            /* Get rid of that old object type. */
            ObjTypeDestroy(p_obj->p_objType) ;
            p_obj->p_objType = NULL ;
        }

        /* Get rid of the extra data attached to this object. */
        if (!(p_obj->attributes & OBJECT_ATTR_PIECE_WISE))  {
            if (ObjectGetExtraData(p_obj) != NULL)
                CreatureDetachFromObject(p_obj) ;
        }

        /* Check to see if we are going to a legal type.  If not, we */
        /* WON'T bother with a new type. */
        if (type != OBJECT_TYPE_NONE)  {
            /* Create a new object type for the given number. */
            p_obj->p_objType = ObjTypeCreate(basicType, p_obj) ;
            DebugCheck(p_obj->p_objType != NULL) ;

            /** Store the object type number in the structure. **/
            p_obj->objectType = type ;
        }
    }

    p_obj->objectType = type ;

    /* Now get the picture for the object type. */
    if (p_obj->p_objType)  {
//        ObjectSetStance(p_obj, 0) ;
        p_obj->p_picture = ObjTypeGetPicture(
                               p_obj->p_objType,
                               ObjectGetAngle(p_obj),
                               &p_obj->orientation) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsUpdateAnimation                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsUpdateAnimation goes through all the objects and updates their */
/*  animation structures.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 currentTime        -- The current time for the animation     */
/*                                   or 0 if you just want to update angles */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjMoveUpdate                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsUpdateAnimation(T_word32 currentTime)
{
    T_3dObject *p_obj ;
    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("ObjectsUpdateAnimation") ;

    for (p_obj = ObjectsGetFirst();
         p_obj != NULL;
         p_obj = ObjectGetNext(p_obj))  {
        /* Only update objects that are active. */
        if (ObjTypeIsActive(p_obj->p_objType))
            ObjectUpdateAnimation(p_obj, currentTime) ;
    }

    DebugEnd() ;
    TICKER_TIME_ROUTINE_ENDM("ObjectsUpdateAnimation", 500) ;
}

/****************************************************************************/
/*  Routine:  ObjectUpdateAnimation                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsUpdateAnimation goes through all the objects and updates their */
/*  animation structures.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 currentTime        -- The current time for the animation     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjMoveUpdate                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectUpdateAnimation(T_3dObject *p_obj, T_word32 currentTime)
{
    T_word16 angle ;

    DebugRoutine("ObjectUpdateAnimation") ;
    DebugCheck(p_obj != NULL) ;

    if (p_obj->p_objType != OBJECT_TYPE_INSTANCE_BAD)  {
        /* Update the animation for this object.  Also check to see if */
        /* there was a change. */
        if (currentTime != 0)
            ObjTypeAnimate(p_obj->p_objType, currentTime) ;

        /* A change occurred.  Let's get the picture, but first we */
        /* need the angle to the POV/player. */

        /* Compute the angle from the object to the player. */
        angle = MathArcTangent(
                    PlayerGetX16() - ObjectGetX16(p_obj),
                    ObjectGetY16(p_obj) - PlayerGetY16())
                        + ObjectGetAngle(p_obj) ;

        /* Now get the picture for the object type. */
        p_obj->p_picture = ObjTypeGetPicture(
                               p_obj->p_objType,
                               angle,
                               &p_obj->orientation) ;

#ifndef NDEBUG
        /* Are we about to bomb because we got a bad picture? */
        if (p_obj->p_picture == NULL)
            /* If so, print out the object's information. */
            ObjectPrint(stdout, p_obj) ;
        DebugCheck(p_obj->p_picture != NULL) ;
#endif
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectSetStance                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsSetStance declares the stance that the object should now       */
/*  become.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to change stance of             */
/*                                                                          */
/*    T_word16 stance             -- Numberical stance to change to         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjTypeSetStance                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetStance(T_3dObject *p_obj, T_word16 stance)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    DebugRoutine("ObjectSetStance") ;
    DebugCheck(p_obj != NULL) ;

//printf("ObjectSetStance %p (%d/%d) by %s\n", p_obj, ObjectGetServerId(p_obj), ObjectGetType(p_obj), DebugGetCallerName()) ;
/* TESTING */
if (strcmp(p_obj->tag, "Obj") == 0)  {
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    /* If we are changing stances, this has to be */
    /* reported to the client (if we aren't the client). */
#ifndef NDEBUG
    if (p_obj->p_objType == NULL)  {
        ObjectPrint(stdout, p_obj) ;
        DebugCheck(FALSE) ;
    }
#endif
    if (ObjTypeGetStance(p_obj->p_objType) != stance)  {
        ObjectSetMovedFlag(p_obj) ;
        ObjTypeSetStance(p_obj->p_objType, stance) ;
    }

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* This is a chained object. */
        /* Is it piecewise? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* It is piecewise. */
            /* Affect all the sub-parts. */
            p_chainedList = (T_bodyPart *)p_chained ;
            for (i=1; i<MAX_BODY_PARTS; i++)  {
                if (p_chainedList[i].p_obj)  {
//printf("set p_chain %p (%d)\n", p_chainedList[i].p_obj, i) ;  fflush(stdout) ;
                    p_chainedList[i].p_obj->objMove = p_obj->objMove ;
                    if (p_chainedList[i].p_obj)
                        if (ObjectGetType(p_chainedList[i].p_obj) != 0)
                            ObjectSetStance(p_chainedList[i].p_obj, stance) ;
                }
            }
        } else {
            /* Not piecewise.  Just affect the pair. */
            ObjectSetStance(p_chained, stance) ;
        }
    }
} /* TESTING */

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsDoToAll                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsDoToAll is a general routine to go through the list of objects */
/*  and call a callback for each object in the list.  In addition, if the   */
/*  callback returns a TRUE, the loop stops.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_objectDoToAllCallback *p_callback -- routine called for each object.*/
/*                                If routine returns TRUE, the loop stops.  */
/*                                Any other values (FALSE) continues.       */
/*                                                                          */
/*    T_word32 data               -- data to pass on to the callback.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsDoToAll(T_objectDoToAllCallback p_callback, T_word32 data)
{
    T_3dObject *p_obj, *p_objNext ;

    DebugRoutine("ObjectsDoToAll") ;
    DebugCheck(p_callback != NULL) ;

    /* Go through the list of all objects (in the world). */
    for (p_obj = ObjectsGetFirst(); p_obj!=NULL; p_obj=p_objNext)
    {
        DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
        /** To allow objects to destroy themselves in the callback, **/
        /** I'll make a copy of each next ptr before calling it. **/
        p_objNext = ObjectGetNext(p_obj) ;

        /* Call the callback. */
        if (p_callback(p_obj, data) == TRUE)
            /* If the callback returns TRUE, break out. */
            break ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ObjectsDoToAllXY                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsDoToAllXY is just like ObjectDoToAll, except it only calls the */
/*  callback routine if there is an object at the given location.           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 x, y               -- Position to test for object            */
/*                                                                          */
/*    T_objectDoToAllCallback *p_callback -- routine called for each object.*/
/*                                If routine returns TRUE, the loop stops.  */
/*                                Any other values (FALSE) continues.       */
/*                                                                          */
/*    T_word32 data               -- data to pass on to the callback.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsDoToAllAtXY(
           T_sword16 x,
           T_sword16 y,
           T_objectDoToAllCallback p_callback,
           T_word32 data)
{
    T_sword16 startHashX, startHashY ;
    T_3dObject *p_obj ;
    T_3dObject *p_objNext ;
    E_Boolean stop = FALSE ;

    DebugRoutine("ObjectsDoToAllAtXY") ;
    DebugCheck(p_callback != NULL) ;

    startHashX = ((x - G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((y - G_3dBlockMapHeader->yOrigin) >> 6) ;

#if 0
    for (hashY=-1; (hashY<=1) && (!stop); hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=-1; (hashX<=1) && (!stop); hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;
                DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

                if (ObjectIsAtXY(p_obj, x, y))  {
                    /* Call the callback. */
                    if (p_callback(p_obj, data) == TRUE)  {
                        /* If the callback returns TRUE, break out. */
                        stop = TRUE ;
                        break ;
                    }
                }
            }
        }
    }
#endif
#if 1
    /* Go through the list of all objects (in the world). */
    for (p_obj = ObjectsGetFirst(); p_obj!=NULL; p_obj = p_objNext)
    {
        DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
        /** To allow objects to destroy themselves in the callback, **/
        /** I'll make a copy of each next ptr before calling it. **/
        p_objNext = ObjectGetNext(p_obj);

        if (ObjectIsAtXY(p_obj, x, y))
            /* Call the callback. */
            if (p_callback(p_obj, data) == TRUE)
                /* If the callback returns TRUE, break out. */
                break ;
    }
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsDoToAllXYRadius                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsDoToAllXYRadius is similar to ObjectsDoToAllXY, but allows     */
/*  a "radius of effect."                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 x, y               -- Position to test for object            */
/*                                                                          */
/*    T_word16 radius             -- Radius around the point to check.      */
/*                                                                          */
/*    T_objectDoToAllCallback *p_callback -- routine called for each object.*/
/*                                If routine returns TRUE, the loop stops.  */
/*                                Any other values (FALSE) continues.       */
/*                                                                          */
/*    T_word32 data               -- data to pass on to the callback.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/17/95  Created (by cloning ObjectsDoToAllAtXY)                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsDoToAllAtXYRadius(
           T_sword16 x,
           T_sword16 y,
           T_word16 radius,
           T_objectDoToAllCallback p_callback,
           T_word32 data)
{
    T_3dObject *p_obj ;

    T_sword16 startHashX, startHashY ;
    E_Boolean stop = FALSE ;
    T_sword16 from, to ;
    T_3dObject *p_objNext ;

    DebugRoutine("ObjectsDoToAllAtXYRadius") ;
    DebugCheck(p_callback != NULL) ;
    startHashX = ((x - G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((y - G_3dBlockMapHeader->yOrigin) >> 6) ;
    to = 1+(radius>>6) ;
    from = -to ;

#if 0
    for (hashY=from; (hashY<=to) && (!stop); hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=from; (hashX<=to) && (!stop); hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;

                DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

                if (CalculateDistance (x,
                                       y,
                                       ObjectGetX16 (p_obj),
                                       ObjectGetY16 (p_obj)) <= (radius + ObjectGetRadius(p_obj)))
                    /* Call the callback. */
                    if (p_callback(p_obj, data) == TRUE)  {
                        /* If the callback returns TRUE, break out. */
                        stop = TRUE ;
                        break ;
                    }
            }
        }
    }
#endif
#if 1
    /* Go through the list of all objects (in the world). */
    for (p_obj = ObjectsGetFirst(); p_obj!=NULL; p_obj = p_objNext)
    {
        DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;
        /** To allow objects to destroy themselves in the callback, **/
        /** I'll make a copy of each next ptr before calling it. **/
        p_objNext = ObjectGetNext(p_obj) ;

        if (CalculateDistance (x,
                               y,
                               ObjectGetX16 (p_obj),
                               ObjectGetY16 (p_obj)) <= (radius + ObjectGetRadius(p_obj)))
            /* Call the callback. */
            if (p_callback(p_obj, data) == TRUE)
                /* If the callback returns TRUE, break out. */
                break ;
    }
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsDoToAllAtXYZRadius                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsDoToAllAtXYZRadius is similar to DoToALLAtXYRadius, except     */
/*  it takes into consideration the z vector.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x, y, z           -- Center point to test for object        */
/*                                                                          */
/*    T_word16 radius             -- Spherical radius to go around.         */
/*                                                                          */
/*    T_objectDoToAllCallback *p_callback -- routine called for each object.*/
/*                                If routine returns TRUE, the loop stops.  */
/*                                Any other values (FALSE) continues.       */
/*                                                                          */
/*    T_word32 data               -- data to pass on to the callback.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/16/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsDoToAllAtXYZRadius(
           T_sword16 x,
           T_sword16 y,
           T_sword16 z,
           T_word16 radius,
           T_objectDoToAllCallback p_callback,
           T_word32 data)
{
    T_3dObject *p_obj ;
    T_word16 xyPlaneDistance ;
    T_sword16 objZ ;
    E_Boolean stop = FALSE ;
    T_3dObject *p_objNext ;

    T_sword16 startHashX, startHashY ;
    T_sword16 to, from ;

    DebugRoutine("ObjectsDoToAllAtXYZRadius") ;
    DebugCheck(p_callback != NULL) ;
    startHashX = ((x - G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((y - G_3dBlockMapHeader->yOrigin) >> 6) ;
    to = 1+(radius>>6) ;
    from = -to ;


    startHashX = ((x - G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((y - G_3dBlockMapHeader->yOrigin) >> 6) ;

#if 0
    for (hashY=from; (hashY<=to) && (!stop); hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=from; (hashX<=to) && (!stop); hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;

                DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

                /* First see if we are close enough to be in the XY plane */
                /* distance. */
                xyPlaneDistance = CalculateDistance(
                                      x,
                                      y,
                                      ObjectGetX16(p_obj),
                                      ObjectGetY16(p_obj)) ;

                if (xyPlaneDistance <= (radius + ObjectGetRadius(p_obj)))  {
                    /* Now we will check the z heights. */
                    objZ = ObjectGetZ16(p_obj) ;

                    /* If the bottom of the object is below the top of the */
                    /* area, AND the top of the object is above the bottom */
                    /* of the area, then we have a collision. */
                    if ((objZ <= (z+radius)) &&
                        ((objZ+ObjectGetHeight(p_obj)) >= (z-radius)))  {
                        /* Call the callback. */
                        if (p_callback(p_obj, data) == TRUE)  {
                            /* If the callback returns TRUE, break out. */
                            stop = TRUE ;
                            break ;
                        }
                    }
                }
            }
        }
    }
#endif
#if 1
    /* Go through the list of all objects (in the world). */
    for (p_obj = ObjectsGetFirst(); p_obj!=NULL; p_obj = p_objNext)
    {
        DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

        /** To allow objects to destroy themselves in the callback, **/
        /** I'll make a copy of each next ptr before calling it. **/
        p_objNext = ObjectGetNext(p_obj) ;

        /* First see if we are close enough to be in the XY plane */
        /* distance. */
        xyPlaneDistance = CalculateDistance(
                              x,
                              y,
                              ObjectGetX16(p_obj),
                              ObjectGetY16(p_obj)) ;

        if (xyPlaneDistance <= (radius + ObjectGetRadius(p_obj)))  {
            /* Now we will check the z heights. */
            objZ = ObjectGetZ16(p_obj) ;

            /* If the bottom of the object is below the top of the */
            /* area, AND the top of the object is above the bottom */
            /* of the area, then we have a collision. */
            if ((objZ <= (z+radius)) &&
                ((objZ+ObjectGetHeight(p_obj)) >= (z-radius)))  {
                /* Call the callback. */
                if (p_callback(p_obj, data) == TRUE)  {
                    /* If the callback returns TRUE, break out. */
                    break ;
                }
            }
        }
    }
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectSetAngle                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectSetAngle changes the angle and notes that the object has moved. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_objectDoToAllCallback *p_callback -- routine called for each object.*/
/*                                If routine returns TRUE, the loop stops.  */
/*                                Any other values (FALSE) continues.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetAngle(T_3dObject *p_obj, T_word16 angle)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    /* Does it change enough to constitute that it truly moved? */
    /* (this is purely to optimize and send packets as a creature */
    /*  turns). */
    if ((angle & 0xF800) != (ObjectGetAngle(p_obj) & 0xF800))
        /* Note that we have moved. */
        ObjectSetMovedFlag(p_obj) ;

    /* Change the angle. */
    ObjMoveSetAngle(&p_obj->objMove, angle) ;

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
//printf("Setting chained object %d\n", ObjectGetServerId(p_obj)) ; fflush(stdout) ;
        /* This is a chained object. */
        /* Is it piecewise? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* It is piecewise. */
            /* Affect all the sub-parts. */
            p_chainedList = (T_bodyPart *)p_chained ;
            for (i=1; i<MAX_BODY_PARTS; i++)  {
                if (p_chainedList[i].p_obj)  {
                    if (p_chainedList[i].p_obj)
                        if (ObjectGetType(p_chainedList[i].p_obj) != 0)  {
//printf("Facing chained object %d\n", i) ;
                            ObjectSetAngle(p_chainedList[i].p_obj, angle) ;
                        }
                }
            }
        } else {
            /* Not piecewise.  Just affect the pair. */
            ObjectSetAngle(p_chained, angle) ;
        }
    }

}

/****************************************************************************/
/*  Routine:  ObjectCheckIfCollide                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectCheckIfCollide checks to see if an object will fit at the given */
/*  location and not collide with anything else.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to test for                     */
/*                                                                          */
/*    T_sword32 x, y, z           -- new position                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE if blocked, FALSE if not.         */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetX                                                            */
/*    ObjectGetY                                                            */
/*    ObjectGetZ                                                            */
/*    ObjectSetX                                                            */
/*    ObjectSetY                                                            */
/*    ObjectSetZ                                                            */
/*    ObjMoveForceUpdate                                                    */
/*    ObjMoveUpdate                                                         */
/*    ObjectWasBlocked                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ObjectCheckIfCollide(
              T_3dObject *p_obj,
              T_sword32 x,
              T_sword32 y,
              T_sword32 z)
{
    E_Boolean status ;
    T_word16 sector;
    T_word16 i ;
    T_objMoveStruct objMove ;

    DebugRoutine("ObjectCheckIfCollide") ;

    /** Figure out which sector on the map the x-y coords point to **/
    sector = View3dFindSectorNum (
                 (T_sword16)(x >> 16),
                 (T_sword16)(y >> 16));

    /** Is the sector okay? **/
    if (sector != 0xFFFF)
    {
        /** Yes. **/
        /* Get the old position and state. */
        objMove = p_obj->objMove ;

        /* Make sure we have not been blocked. */
        ObjectClearBlockedFlag(p_obj) ;

        /* Go to the requested position. */
        ObjectSetX(p_obj, x) ;
        ObjectSetY(p_obj, y) ;
        ObjectSetZ(p_obj, z) ;

        /* Make sure we don't collide with ourself. */
        View3dSetExceptObjectByPtr(&p_obj->objMove) ;

        /* Force and do an update. */
        ObjMoveForceUpdate(&p_obj->objMove) ;
        ObjMoveUpdate(&p_obj->objMove, 0) ;

        /* Does that new position collide with anything? */
        status = (ObjectWasBlocked(p_obj)) ? TRUE : FALSE ;
//printf("status: %d\n", status) ;

        /* Check to see if the sectors are too high */
        for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
            if ((z>>16) < MapGetWalkingFloorHeight(ObjectGetNthAreaSector(p_obj, i)))  {
                status = TRUE ;
                ObjectSetMoveFlags(p_obj, OBJMOVE_FLAG_BLOCKED) ;
//printf("nth sector %d %d %d\n", i, ObjectGetNthAreaSector(p_obj, i), z>>16) ;
                break ;
            }
        }

        /* Go back to the old position and state. */
        p_obj->objMove = objMove ;
    }
    else
    {
        /** X,Y coordinates point to invalid sector. **/
        status = TRUE ;
//printf("invalid sector\n") ;
    }

    DebugCheck(status < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    /* Return if we hit something. */
    return status ;
}

/****************************************************************************/
/*  Routine:  ObjectIsAtXY                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectIsAtXY checks to see if an object is at the given x, y location.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to test for                     */
/*                                                                          */
/*    T_sword16 x, y              -- Position to test                       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE if ther, FALSE if not.            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetX16                                                          */
/*    ObjectGetY16                                                          */
/*    ObjectGetRadius                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ObjectIsAtXY(T_3dObject *p_obj, T_sword16 x, T_sword16 y)
{
    T_word16 radius ;
    T_sword16 dist ;
    E_Boolean status = FALSE ;

    DebugRoutine("ObjectIsAtXY") ;
    DebugCheck(p_obj != NULL) ;

    /* What is the object's radius? */
    radius = ObjectGetRadius(p_obj) ;

    /* What is the distance along the x axis (positive only, please). */
    dist = x - ObjectGetX16(p_obj) ;
    if (dist < 0)
        dist = -dist ;

    /* Is that close enough? */
    if (dist <= radius)  {
        /* How about the distance along the y? */
        dist = y - ObjectGetY16(p_obj) ;
        if (dist < 0)
            dist = -dist ;

        /* If that is close enough, then this is here. */
        if (dist <= radius)
            status = TRUE ;
    }

    DebugEnd() ;

    return status ;
}

/****************************************************************************/
/*  Routine:  ObjectGetForwardPosition                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetForwardPosition determines what the x and y coordinate is    */
/*  in front of an object by a given distance.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to find position in front of    */
/*                                                                          */
/*    T_word16 dist               -- Distance in front of object            */
/*                                                                          */
/*    T_sword32 *p_x, *p_y        -- X and Y pointers for found location.   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetX                                                            */
/*    ObjectGetY                                                            */
/*    ObjectGetAngle                                                        */
/*    MathCosineLookup                                                      */
/*    MathSineLookup                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectGetForwardPosition(
           T_3dObject *p_obj,
           T_word16 dist,
           T_sword32 *p_x,
           T_sword32 *p_y)
{
    T_word16 angle ;

    DebugRoutine("ObjectGetForwardPosition") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(p_x != NULL) ;
    DebugCheck(p_y != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    /* What angle are we facing? */
    angle = ObjectGetAngle(p_obj) ;

    /* Calculate the position in front of the object's X */
    *p_x = ObjectGetX(p_obj) +
               (MathCosineLookup(angle) * ((T_sword32)dist)) ;

    /* Calculate the position in front of the object's Y */
    *p_y = ObjectGetY(p_obj) +
               (MathSineLookup(angle) * ((T_sword32)dist)) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectMakeTemporarilyPassableAtXYRadius                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine is called when a group of objects touching a circular    */
/*  area needs to be make passable until they move again without touching   */
/*  anything.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x, y              -- Center location of circular area       */
/*                                                                          */
/*    T_word16 radius             -- Radius of circular area                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectsDoToAllAtXY                                                    */
/*    IMakeTempPassable (indirectly)                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectsMakeTemporarilyPassableAtXYRadius(
           T_sword16 x,
           T_sword16 y,
           T_word16 radius,
           T_sword16 zBottom,
           T_sword16 zTop)
{
    DebugRoutine("ObjectsMakeTemporarilyPassableAtXYRadius") ;

    ObjectsDoToAllAtXYRadius(x, y, radius, IMakeTempPassable, (((T_sword32)zBottom)<<16)|zTop) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IMakeTempPassable                       * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IMakeTempPassable is a callback routine called to declare an object   */
/*  as passable until free.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to make temporarily passable    */
/*                                                                          */
/*    T_word32 data               -- [Not used] required for callback       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectMakePassable                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static E_Boolean IMakeTempPassable(T_3dObject *p_obj, T_word32 data)
{
    T_sword16 zBottom, zTop ;

    DebugRoutine("IMakeTempPassable") ;
    DebugCheck(p_obj != NULL) ;

    zTop = (data & 0xFFFF) ;
    zBottom = (data >> 16) ;

//    if ((ObjectGetZ16(p_obj) >= zBottom) &&
//        ((ObjectGetZ16(p_obj) + ObjectGetHeight(p_obj)) <= zTop))  {
//printf("     Make trans %d (%d, %d) (%d, %d)\n", ObjectGetServerId(p_obj), ObjectGetX16(p_obj), ObjectGetY16(p_obj), ObjectGetZ16(p_obj), ObjectGetZ16(p_obj) + ObjectGetHeight(p_obj)) ;
        ObjectMakePassable(p_obj) ;
        ObjectMakeTranslucent(p_obj) ;
        ObjectMarkImpassableWhenFree(p_obj) ;
//    } else {
//printf("     Failed: %d %d %d %d\n", zBottom, zTop, ObjectGetZ16(p_obj), ObjectGetHeight(p_obj)) ;
//    }

    DebugEnd();

    return FALSE ;
}

/** !!! AMT FOR DEBUGGING PURPOSES ONLY **/
/*
T_void ObjectSetServerId (T_3dObject *p_obj, T_word16 id)
{
   DebugRoutine ("ObjectSetServerId");

   DebugCheck (ObjectFind (id) == NULL);
   p_obj->objServerId = id;

   DebugEnd ();
}
*/

/****************************************************************************/
/*  Routine:  ObjectsCountType                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsCountType goes through the list of objects in the map and      */
/*  counts how many have the same type.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objectType         -- Type to count                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Number found                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectsGetFirst                                                       */
/*    ObjectGetNext                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/28/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 ObjectsCountType(T_word16 objectType)
{
    T_word32 count = 0 ;
    T_3dObject *p_obj ;

    DebugRoutine("ObjectsCountType") ;

    p_obj = ObjectsGetFirst() ;
    while (p_obj)  {
        if (ObjectGetType(p_obj) == objectType)
            count++ ;
        p_obj = ObjectGetNext(p_obj) ;
    }

    DebugEnd() ;

    return count ;
}

/****************************************************************************/
/*  Routine:  ObjectsCountBasicType                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsCountBasicType goes through the list of objects in the map and */
/*  counts how many have the same basic type.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 objectType         -- Type to count                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Number found                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectsGetFirst                                                       */
/*    ObjectGetNext                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/28/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 ObjectsCountBasicType(T_word16 objectType)
{
    T_word32 count = 0 ;
    T_3dObject *p_obj ;

    DebugRoutine("ObjectsCountBasicType") ;

    /* Strip out the confusing part. */
    objectType &= OBJECT_TYPE_BASIC_MASK ;

    p_obj = ObjectsGetFirst() ;
    while (p_obj)  {
        if (ObjectGetBasicType(p_obj) == objectType)
            count++ ;
        p_obj = ObjectGetNext(p_obj) ;
    }

    DebugEnd() ;

    return count ;
}

/****************************************************************************/
/*  Routine:  ObjectDuplicate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectDuplicate takes one object and make a copy of it.               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to duplicate                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Duplicate object, else NULL            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectCreate                                                          */
/*    ObjectSetType                                                         */
/*    ObjectGetType                                                         */
/*    ObjectTypeGetPicture                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/01/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_3dObject *ObjectDuplicate(T_3dObject *p_obj)
{
    T_3dObject *p_new ;

    DebugRoutine("ObjectDuplicate") ;
    DebugCheck(p_obj != NULL) ;

    if (p_obj)  {
        if (ObjectGetServerId(p_obj) == 0)
            p_new = ObjectCreateFake() ;
        else
            p_new = ObjectCreate() ;
        ObjectSetType(p_new, ObjectGetType(p_obj)) ;
        DebugCheck(p_new != NULL) ;
        if (p_new)  {
            p_new->objMove = p_obj->objMove ;
            p_new->objectType = p_obj->objectType ;
            p_new->attributes = p_obj->attributes ;
            p_new->accessoryData = p_obj->accessoryData ;
            p_new->picResource = p_obj->picResource ;
            p_new->p_picture = ObjTypeGetPicture(
                                   p_new->p_objType,
                                   ObjectGetAngle(p_obj),
                                   &p_new->orientation) ;
            p_new->health = p_obj->health ;
            p_new->scaleX = p_obj->scaleX ;
            p_new->scaleY = p_obj->scaleY ;

        }
    }

    DebugEnd() ;

    return p_new ;
}

/****************************************************************************/
/*  Routine:  ObjectSetBodyPartType                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectSetBodyPartType changes the body part on an object.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to change body part on          */
/*                                                                          */
/*    T_bodyPartLocation location -- Location of body part                  */
/*                                                                          */
/*    T_word16 objType            -- Type of new body part                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IObjectFindBodyPart                                                   */
/*    ObjectSetType                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectSetBodyPartType(
           T_3dObject *p_obj,
           T_bodyPartLocation location,
           T_word16 objType)
{
    T_3dObject *p_bodyPart ;

    DebugRoutine("ObjectSetBodyPartType") ;

    p_bodyPart = IObjectFindBodyPart(p_obj, location) ;
//    DebugCheck(p_bodyPart != NULL) ;
    if (p_bodyPart)
        ObjectSetTypeSimple(p_bodyPart, objType) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectGetBodyPartType                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetBodyPartType gets    the body part on an object.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to get    body part of          */
/*                                                                          */
/*    T_bodyPartLocation location -- Location of body part                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- type of part, or 0                     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IObjectFindBodyPart                                                   */
/*    ObjectGetType                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 ObjectGetBodyPartType(
             T_3dObject *p_obj,
             T_bodyPartLocation location)
{
    T_3dObject *p_bodyPart ;
    T_word16 objType = 0 ;

    DebugRoutine("ObjectGetBodyPartType") ;

    p_bodyPart = IObjectFindBodyPart(p_obj, location) ;

    if (p_bodyPart)
        objType = ObjectGetType(p_bodyPart) ;

    DebugEnd() ;

    return objType ;
}


/****************************************************************************/
/*  Routine:  IObjectFindBodyPart                     * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IObjectFindBodyPart looks up a player's object part.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_body          -- Object that has a body                 */
/*                                                                          */
/*    T_bodyPartLocation location -- Location of body part                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_3dObject *                -- Pointer to object part, else NULL      */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IObjectFindBodyPart                                                   */
/*    ObjectGetType                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/07/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_3dObject *IObjectFindBodyPart(
                      T_3dObject *p_body,
                      T_bodyPartLocation location)
{
    T_bodyPart *p_chainedList ;
    T_3dObject *p_part = NULL ;

    DebugRoutine("IObjectFindBodyPart") ;
    DebugCheck(p_body != NULL) ;
    DebugCheck(strcmp(p_body->tag, "Obj") == 0) ;
    DebugCheck(location < BODY_PART_LOCATION_UNKNOWN) ;

//puts("IObjectFindBodyPart") ;
//printf("p_body = %p (%d)\n", p_body, ObjectGetServerId(p_body)) ;  fflush(stdout) ;
    if (ObjectGetAttributes(p_body) & OBJECT_ATTR_PIECE_WISE)  {
        if (location)  {
            /* Not the head, look it up in the list. */
            p_chainedList = (T_bodyPart *)ObjectGetChainedObjects(p_body) ;
//printf("p_chainedList = %p\n", p_chainedList) ;  fflush(stdout) ;
            if (p_chainedList)  {
                p_part = p_chainedList[location].p_obj ;
//printf("p_part = %p (%d)\n", p_part, ObjectGetServerId(p_part)) ;  fflush(stdout) ;
            }
        } else {
            /* The head is the main part of the body. */
            /* Change it and everything changes. */
            p_part = p_body ;
        }
    }

    DebugEnd() ;

    return p_part ;
}

/****************************************************************************/
/*  Routine:  IObjectRemoveFromHashTable              * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IObjectRemoveFromhashTable checks to see if the object is on the      */
/*  hash table, and if it is, removes it from that table.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to remove from hash table       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetServerId                                                     */
/*    ObjectGetHashPointer                                                  */
/*    ObjectSetHashPointer                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IObjectRemoveFromHashTable(T_3dObject *p_obj)
{
    T_3dObject *p_hash ;
    T_3dObject *p_nextHash ;
    T_word16 hash ;


    DebugRoutine("IObjectRemoveFromHashTable") ;

    hash = ObjectGetServerId(p_obj) & OBJECT_HASH_TABLE_MASK ;
    p_hash = G_objectHashTable->table[hash] ;
    DebugCheck(p_hash != NULL) ;

    /* Is the first entry of the hash table pointing to the object? */
    if (p_hash == p_obj)  {
        /* Yes, the first is the object. */
        G_objectHashTable->table[hash] = ObjectGetHashPointer(p_obj) ;
    } else {
        /* No, the object is elsewhere. */
        /* Search through the hash list for a match. */
        do {
            DebugCheck(p_hash != NULL) ;
            p_nextHash = ObjectGetHashPointer(p_hash) ;

            /* Is the next hash entry the object we are trying to */
            /* remove? */
            if (p_nextHash == p_obj)  {
                /* Yes, it is. */
                /* Remove the entry from the hash list. */
                ObjectSetHashPointer(p_hash,
                    ObjectGetHashPointer(p_obj)) ;
            } else {
                /* No, keep searching. */
                p_hash = p_nextHash ;
            }
        } while (p_hash != NULL) ;
    }

    /* The object is no longer on the hash table. */
    ObjectSetHashPointer(p_obj, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IObjectAddToHashTable                   * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IObjectAddToHashTable adds the given object onto the hash table.      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to add to the  hash table       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetServerId                                                     */
/*    ObjectGetHashPointer                                                  */
/*    ObjectSetHashPointer                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IObjectAddToHashTable(T_3dObject *p_obj)
{
    T_word16 hash ;

    DebugRoutine("IObjectAddToHashTable") ;

    /* Add the object to the hash table. */
    hash = ObjectGetServerId(p_obj) & OBJECT_HASH_TABLE_MASK ;
//printf("Add to hash table %d at hash %d\n", ObjectGetServerId(p_obj), hash) ;
    ObjectSetHashPointer(p_obj, G_objectHashTable->table[hash]) ;
    G_objectHashTable->table[hash] = p_obj ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectAllocExtraData                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectAllocExtraData allocates memory onto an object for whatever     */
/*  use is needed by the caller.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to add memory to                */
/*                                                                          */
/*    T_word32 sizeData           -- Amount of memory needed.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_void *                    -- POinter to memory                      */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/03/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *ObjectAllocExtraData(T_3dObject *p_obj, T_word32 sizeData)
{
    T_void *p_data = NULL ;

    DebugRoutine("ObjectAllocExtraData") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(ObjectGetExtraData(p_obj) == NULL) ;

    p_data = MemAlloc(sizeData) ;
    ObjectSetExtraData(p_obj, p_data);

    DebugEnd() ;

    DebugCheck(p_data != NULL) ;
    return p_data ;
}

/****************************************************************************/
/*  Routine:  ObjectFreeExtraData                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectFreeExtraData frees preivous allocated memory attached to a     */
/*  given object.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This routine checks to see if there IS memory to free first    */
/*  and doesn't mind being called with no extra data.                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to free memory from.            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/03/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectFreeExtraData(T_3dObject *p_obj)
{
    T_void *p_data = NULL ;

    DebugRoutine("ObjectFreeExtraData") ;
    DebugCheck(p_obj != NULL) ;
//    DebugCheck(ObjectGetExtraData(p_obj) == NULL) ;

    p_data = ObjectGetExtraData(p_obj);
    if (p_data)  {
        MemFree(p_data) ;
        ObjectSetExtraData(p_obj, NULL) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectIsBeingCrushed                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectIsBeingCrushed checks to see if the given object has a bigger   */
/*  height than the height it is in.                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to check if being crushed       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE=yes, crushed, else FALSE          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MapGetCeilingHeight                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/04/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ObjectIsBeingCrushed(T_3dObject *p_obj)
{
    T_word16 i, num ;
    T_word16 sector ;
    T_sword16 height ;
    T_sword16 head ;
    E_Boolean isCrushed = FALSE ;

    DebugRoutine("ObjectIsBeingCrushed") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    /* Go through the list of sectors that we are standing over. */
    /* For each sector, check to see if our head is in the ceiling. */
    /* If so, return TRUE. */
    num = ObjectGetNumAreaSectors(p_obj) ;
    head = ObjectGetZ16(p_obj) + ObjectGetHeight(p_obj) ;
    for (i=0; i<num; i++)  {
        sector = ObjectGetNthAreaSector(p_obj, i) ;
        DebugCheck(sector < G_Num3dSectors) ;
        height = MapGetCeilingHeight(sector) ;
        if (head > height)  {
            isCrushed = TRUE ;
            break ;
        }
    }

    DebugEnd() ;

    return isCrushed ;
}


/****************************************************************************/
/*  Routine:  ObjectRemoveScript                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectRemoveScript checks to see if an object has a script, and if it */
/*  does, turns off and removes that script.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to remove script from.          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ScriptUnlock                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/04/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectRemoveScript(T_3dObject *p_obj)
{
    DebugRoutine("ObjectRemoveScript") ;
    DebugCheck (p_obj != NULL);

    /* If you have a script, remove it. */
    if (p_obj->script)  {
        ScriptUnlock(p_obj->script) ;
        p_obj->script = SCRIPT_BAD ;

        /* Declare no script here. */
//        ObjectSetScript(p_obj, 0) ;
    }

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ObjectGetAngularPosition                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectGetAngularPosition determines what the x and y coordinate is    */
/*  in a given direction and distance from a given object                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to find position in front of    */
/*                                                                          */
/*    T_word16 angle              -- Angle from the object                  */
/*                                                                          */
/*    T_word16 dist               -- Distance in front of object            */
/*                                                                          */
/*    T_sword32 *p_x, *p_y        -- X and Y pointers for found location.   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetX                                                            */
/*    ObjectGetY                                                            */
/*    MathCosineLookup                                                      */
/*    MathSineLookup                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectGetAngularPosition(
           T_3dObject *p_obj,
           T_word16 angle,
           T_sword16 dist,
           T_sword32 *p_x,
           T_sword32 *p_y)
{
    DebugRoutine("ObjectGetAngularPosition") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(p_x != NULL) ;
    DebugCheck(p_y != NULL) ;

    /* Calculate the position in front of the object's X */
    *p_x = ObjectGetX(p_obj) +
               (MathCosineLookup(angle) * ((T_sword32)dist)) ;

    /* Calculate the position in front of the object's Y */
    *p_y = ObjectGetY(p_obj) +
               (MathSineLookup(angle) * ((T_sword32)dist)) ;

    DebugEnd() ;
}

T_void ObjectChainingOff(T_void)
{
    G_objectChainingAllow = FALSE ;
}

T_void ObjectChainingOn(T_void)
{
    G_objectChainingAllow = TRUE ;
}

/****************************************************************************/
/*  Routine:  ObjectMarkForDestroy                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectMarkForDestroy declares an object needs to be destroyed by      */
/*  outside routines.  The object system does not automatically destroy.    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to mark for destruction         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ObjectMarkForDestroy(T_3dObject *p_obj)
{
    DebugRoutine("ObjectMarkForDestroy") ;
    DebugCheck(p_obj != NULL) ;

//if (ObjectGetServerId(p_obj))
//printf("Mark object %d (%d) for destroy by %s\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj), DebugGetCallerName()) ;
    /* Check to see if the object has been previously marked. */
    if (!(p_obj->attributes & OBJECT_ATTR_MARK_FOR_DESTROY))  {
        /* If not, mark it for destruction and increment */
        /* the count. */
        p_obj->attributes |= OBJECT_ATTR_MARK_FOR_DESTROY ;
        G_numObjectsMarkedForDestroy++ ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectsGetNumMarkedForDestroy                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsGetNumMarkedForDestroy returns the number of objects that      */
/*  have set their MARK_FOR_DESTROY attribute.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/26/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 ObjectsGetNumMarkedForDestroy(T_void)
{
    return G_numObjectsMarkedForDestroy ;
}

/****************************************************************************/
/*  Routine:  ObjectsUpdateMovementForFake                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectsUpdateMovementForFake is just like ObjectsUpdateMovement but   */
/*  it only updates fake objects.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 delta              -- Delta of time since last update        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjMoveUpdate                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/17/96  Created from ObjectsUpdateMovement                     */
/*                                                                          */
/****************************************************************************/

T_void ObjectsUpdateMovementForFake(T_word32 delta)
{
    T_3dObject *p_obj ;
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;
    T_sword32 x, y, z ;
    T_3dObject *p_child ;

    DebugRoutine("ObjectsUpdateMovement") ;

    /** What if delta is too big? **/
    /** I'll break it up into pieces. **/
    if (delta > 20)
    {
        ObjectsUpdateMovementForFake(delta >> 1);
        ObjectsUpdateMovementForFake((delta >> 1) + (delta & 1));
    }

    /* No more exceptions. */
    View3dSetExceptObjectByPtr(NULL) ;

    for (p_obj = ObjectsGetFirst();
         p_obj != NULL;
         p_obj = ObjectGetNext(p_obj))  {

        if (ObjectGetServerId(p_obj) == 0)  {
    /** Only valid for a client+server build. **/
    #ifndef SERVER_ONLY
            if (PlayerGetObject() != p_obj)
    #endif

            {
                /* Update the object's script if it has one. */
                if (ObjectGetScriptHandle(p_obj) != NULL)
                    ScriptEvent(
                        ObjectGetScriptHandle(p_obj),
                        SCRIPT_EVENT_TIME_UPDATE,
                        SCRIPT_DATA_TYPE_32_BIT_NUMBER,
                        &delta,
                        SCRIPT_DATA_TYPE_NONE,
                        NULL,
                        SCRIPT_DATA_TYPE_NONE,
                        NULL) ;

                ObjMoveUpdate(&p_obj->objMove, delta) ;

                /* If I made a sucessful step, make me impassible again. */
    /*
                if (!(ObjectWasBlocked(p_obj)))
                    if (ObjectIsMarkedMakeImpassibleWhenFree(p_obj))
                        ObjectMakeImpassable(p_obj) ;
    */

                /* Are there other objects chained to this one? */
                p_chained = ObjectGetChainedObjects(p_obj) ;
                if (p_chained)  {
                    /* Yes, there is a chain. */

                    /* Note where the root object is located. */
                    x = ObjectGetX(p_obj) ;
                    y = ObjectGetY(p_obj) ;
                    z = ObjectGetZ(p_obj) ;

                    /* Is this a piecewise object or */
                    /*   just a single chain object? */
                    if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
                        /* It is piecewise. */
                        p_chainedList = (T_bodyPart *)p_chained ;
                        /* Move all the chained objects to the root object. */
                        for (i=1; i<MAX_BODY_PARTS; i++)  {
                            p_child = p_chainedList[i].p_obj ;
                            /* Only bother with objects that are there. */
                            if (p_child)  {
                                /* Move it to the root location. */
                                ObjectSetX(p_child, x) ;
                                ObjectSetY(p_child, y) ;
                                ObjectSetZ(p_child, z) ;
                            }
                        }
                    } else {
                        /* Not piecewise -- therefore, must only be */
                        /* one object. */
                        ObjectSetX(p_chained, x) ;
                        ObjectSetY(p_chained, y) ;
                        ObjectSetZ(p_chained, z) ;
                    }
                }
            }
        }
    }

    DebugEnd() ;
}

/*    LES  06/20/96  Created                                                */
T_void ObjectsResetIds(T_void)
{
    DebugRoutine("ObjectsResetIds") ;

    G_lastObjectId = 30000 ;

    DebugEnd() ;
}

T_word32 ObjectGetNextId(T_void)
{
    return G_lastObjectId ;
}

/****************************************************************************/
T_void ObjectAddAttributesToPiecewise(T_3dObject *p_obj, T_word16 attr)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    /* Set the attributes for this object. */
    ObjectAddAttributes(p_obj, attr) ;

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* Yes it is.  Is this a piecewise chained object? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* This is a piecewise chained list. */
            p_chainedList = (T_bodyPart *)p_chained ;
            for (i=1; i<MAX_BODY_PARTS; i++)  {
//printf("Chained object %d (%p) of %p attr added %02X was %04X\n",  i, p_chainedList[i].p_obj, p_obj, attr, ObjectGetAttributes(p_chainedList[i].p_obj)) ;
                if (p_chainedList[i].p_obj)  {
//puts("add attr OK") ;
                    ObjectAddAttributes(p_chainedList[i].p_obj, attr) ;
                }
            }
        } else {
            /* This is a chained list, but not piecewise. */
            ObjectAddAttributes(p_chained, attr) ;
        }
    }
}

/****************************************************************************/
T_void ObjectRemoveAttributesFromPiecewise(T_3dObject *p_obj, T_word16 attr)
{
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;
    T_word16 i ;

    /* Set the attributes for this object. */
    ObjectRemoveAttributes(p_obj, attr) ;

    /* Is this a chained object? */
    p_chained = ObjectGetChainedObjects(p_obj) ;
    if (p_chained)  {
        /* Yes it is.  Is this a piecewise chained object? */
        if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PIECE_WISE)  {
            /* This is a piecewise chained list. */
            p_chainedList = (T_bodyPart *)p_chained ;
            for (i=1; i<MAX_BODY_PARTS; i++)  {
                if (p_chainedList[i].p_obj)  {
                    p_chainedList[i].p_obj->objMove = p_obj->objMove ;
                    ObjectRemoveAttributes(p_chainedList[i].p_obj, attr) ;
                }
            }
        } else {
            /* This is a chained list, but not piecewise. */
            ObjectRemoveAttributes(p_chained, attr) ;
        }
    }
}

/****************************************************************************/
T_word16 ObjectGetWeight(T_3dObject *p_obj)
{
    T_word16 weight ;
    T_word16 color ;

    /* Weight adjustments based on object color type */
    static T_word32 weightAdjust[16] = {
        100,    /* NONE           */
        100,    /* WOOD           */
        100,    /* RUSTY          */
        125,    /* BRONZE         */

        100,    /* IRON           */
        100,    /* SILVER         */
        100,    /* STEEL          */
        125,    /* HARDEN_STEEL   */

         50,    /* MITHRIL        */
        100,    /* OBSIDIAN       */
         75,    /* PYRINIUM       */
         50,    /* ADAMINIUM      */

        100,    /* UNKNOWN_1      */
        100,    /* UNKNOWN_2      */
        100,    /* UNKNOWN_3      */
        100,    /* UNKNOWN_4      */
    } ;

    weight = ObjTypeGetWeight((p_obj)->p_objType) ;
    color = ObjectGetColorizeTable(p_obj) ;

    return (((T_word32)weight) * weightAdjust[color]) / 100 ;
}

/****************************************************************************/
T_word16 ObjectGetValue(T_3dObject *p_obj)
{
    T_word16 value ;
    T_word16 color ;

    /* Value adjustments based on object color type */
    static T_word32 valueAdjust[16] = {
        100,    /* NONE           */
          5,    /* WOOD           */
         25,    /* RUSTY          */
         50,    /* BRONZE         */

        100,    /* IRON           */
        150,    /* SILVER         */
        200,    /* STEEL          */
        300,    /* HARDEN_STEEL   */

        400,    /* MITHRIL        */
        600,    /* OBSIDIAN       */
        800,    /* PYRINIUM       */
       1200,    /* ADAMINIUM      */

        100,    /* UNKNOWN_1      */
        100,    /* UNKNOWN_2      */
        100,    /* UNKNOWN_3      */
        100,    /* UNKNOWN_4      */
    } ;

    value = ObjTypeGetValue((p_obj)->p_objType) ;
    color = ObjectGetColorizeTable(p_obj) ;

    return (((T_word32)value) * valueAdjust[color]) / 100 ;
}

/****************************************************************************/
T_void ObjectDrawFrontScaled(
           T_3dObject *p_obj,
           T_sword16 x,
           T_sword16 y,
           T_word16 width,
           T_word16 height)
{
    T_void *p_picture ;

    DebugRoutine("ObjectDrawFrontScaled") ;
    DebugCheck (p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    /* Get the front picture */
    p_picture = ObjTypeGetFrontFirstPicture(p_obj->p_objType) ;

    DebugCheck(p_picture != NULL) ;

    if (p_picture)
        GrDrawCompressedBitmapAndClipAndColorAndCenterAndResize(
            (T_bitmap *)(&(((T_sword16 *)p_picture)[-2])),
            x,
            y,
            ObjectGetColorizeTable(p_obj),
            width,
            height) ;

    DebugEnd() ;
}

/****************************************************************************/
T_void ObjectUpdateCollisionLink(T_3dObject *p_obj)
{
    T_word16 group ;

    DebugRoutine("ObjectUpdateCollisionLink") ;

    /* Determine group object should be in. */
    group = ((ObjectGetY16(p_obj) - G_3dBlockMapHeader->yOrigin) >> 6) *
                G_objCollisionNumX +
                    ((ObjectGetX16(p_obj) - G_3dBlockMapHeader->xOrigin) >> 6) ;
    if (group > G_lastCollisionList)
        group = G_lastCollisionList ;

    /* Has the object moved from one list to another? */
    if (p_obj->objCollisionGroup != group)  {
        /* If so, take it off the old list (if ever on one) */
        if (p_obj->objCollisionGroup != OBJ_COLLISION_GROUP_NONE)  {
            DoubleLinkListRemoveElement(p_obj->elementInObjCollisionList) ;
            p_obj->elementInObjCollisionList = DOUBLE_LINK_LIST_ELEMENT_BAD ;
            p_obj->objCollisionGroup = OBJ_COLLISION_GROUP_NONE ;
        }

        if ((!ObjectIsPassable(p_obj)) && (!ObjectIsFullyPassable(p_obj)))  {
/*
printf("p_obj %p at (%d, %d) put in group %d (list %p)\n",
  p_obj,
  ObjectGetX16(p_obj),
  ObjectGetY16(p_obj),
  group,
  G_3dObjCollisionLists[group]) ;
*/
            /* Put the object on the new list. */
            p_obj->elementInObjCollisionList =
                DoubleLinkListAddElementAtFront(
                    G_3dObjCollisionLists[group],
                    (T_void *)p_obj) ;
            p_obj->objCollisionGroup = group ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
T_void ObjectUnlinkCollisionLink(T_3dObject *p_obj)
{
    DebugRoutine("ObjectUnlinkCollisionLink") ;

    if (p_obj->objCollisionGroup != OBJ_COLLISION_GROUP_NONE)  {
        DoubleLinkListRemoveElement(p_obj->elementInObjCollisionList) ;
        p_obj->elementInObjCollisionList = DOUBLE_LINK_LIST_ELEMENT_BAD ;
        p_obj->objCollisionGroup = OBJ_COLLISION_GROUP_NONE ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ObjectCheckCollideAny                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ObjectCheckCollideAny  sees if the position given for an object will  */
/*  cause it to collide with any of the other objects, passible or not.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject p_obj            -- object to check for object collision   */
/*                                                                          */
/*    T_sword16 x, y              -- position to check for collision        */
/*                                                                          */
/*    T_sword16 height            -- New height to check for                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = collided, else FALSE            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/30/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean ObjectCheckCollideAny(
              T_3dObject *p_obj,
              T_sword16 x,
              T_sword16 y,
              T_sword16 height)
{
    E_Boolean status = FALSE ;
    T_word16 radius ;
    T_sword16 zBottom, zTop ;
    T_sword16 czBottom, czTop ;
    T_3dObject *p_compare ;
    T_sword16 x1, x2 ;
    T_sword16 y1, y2 ;
    T_sword16 cx1, cx2 ;
    T_sword16 cy1, cy2 ;

    DebugRoutine("ObjectCheckCollideAny") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(strcmp(p_obj->tag, "Obj") == 0) ;

    zBottom = height ;
    zTop = height + ObjectGetHeight(p_obj) ;
    radius = ObjectGetRadius(p_obj) ;
    x1 = ObjectGetX16(p_obj) - radius ;
    x2 = ObjectGetX16(p_obj) + radius ;
    y1 = ObjectGetY16(p_obj) - radius ;
    y2 = ObjectGetY16(p_obj) + radius ;

    p_compare = ObjectsGetFirst() ;
    for (; p_compare; p_compare = ObjectGetNext(p_compare))  {
        if (p_compare == p_obj)
            continue ;

        if (ObjectIsFullyPassable(p_compare))
            continue ;

        /* Compare the x edges */
        radius = ObjectGetRadius(p_compare) ;
        cx1 = ObjectGetX16(p_compare) - radius ;
        cx2 = ObjectGetX16(p_compare) + radius ;
        if ((x1 <= cx2) && (x2 >= cx1))  {
            /* Compare the y edges */
            cy1 = ObjectGetY16(p_compare) - radius ;
            cy2 = ObjectGetY16(p_compare) + radius ;
            if ((y1 <= cy2) && (y2 >= cy1))  {
                /* Compare the z edges */
                czBottom = ObjectGetZ16(p_compare) ;
                czTop = czBottom + ObjectGetHeight(p_compare) ;
                if ((zBottom <= czTop) && (zTop >= czBottom))  {
                    SyncMemAdd("Object %d collides with %d\n",
                        ObjectGetServerId(p_obj),
                        ObjectGetServerId(p_compare),
                        0) ;
                    status = TRUE ;
                    break ;
                }
            }
        }
    }

    DebugEnd() ;

    return status ;
}

/****************************************************************************/
/* Given an object that is pointing to a body part, find a matching */
/* head that is owning that body part. */
T_3dObject *ObjectFindBodyPartHead(T_3dObject *p_part)
{
    T_3dObject *p_search ;
    T_word16 i ;
    T_3dObject *p_chained ;
    T_bodyPart *p_chainedList ;

    DebugRoutine("ObjectFindBodyPartHead") ;
    DebugCheck(ObjectIsBodyPart(p_part)) ;

    p_search = ObjectsGetFirst() ;
    while (p_search != NULL)  {
        if (ObjectIsPlayer(p_search))  {
            p_chained = ObjectGetChainedObjects(p_search) ;
            if (p_chained)  {
                p_chainedList = (T_bodyPart *)p_chained ;
                for (i=1; i<MAX_BODY_PARTS; i++)  {
                    if (p_chainedList[i].p_obj == p_part)  {
                        DebugEnd() ;
                        return p_search ;
                    }
                }
            }
        }
        p_search = ObjectGetNext(p_search) ;
    }

    DebugEnd() ;

    return NULL ;
}

/****************************************************************************/
/*    END OF FILE:  OBJECT.C                                                */
/****************************************************************************/
