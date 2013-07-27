/****************************************************************************/
/*    FILE:  PICS.C                                                         */
/****************************************************************************/
#include "PICS.H"

static T_resourceFile G_pictureResFile ;
static E_Boolean G_picturesActive = FALSE ;

/****************************************************************************/
/*  Routine:  PicturesInitialize                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PicturesInitialize opens up the picture database in preparation for   */
/*  all future picture locking and unlocking.                               */
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
/*    ResourceOpen                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PicturesInitialize(T_void)
{
    DebugRoutine("PicturesInitialize") ;
    DebugCheck(G_picturesActive == FALSE) ;

    /* Open up the resource file for future accesses. */
    G_pictureResFile = ResourceOpen(PICTURE_RESOURCE_FILENAME) ;

    /* Note that we are now active. */
    G_picturesActive = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PicturesFinish                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PicturesFinish is called when the pictures resource file is no        */
/*  longer needed (typically when exiting the program).  When this occurs,  */
/*  the resource file is closed out.                                        */
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
/*    ResourceClose                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PicturesFinish(T_void)
{
    DebugRoutine("PicturesFinish") ;
    DebugCheck(G_picturesActive == TRUE) ;

    /* Close the already open resource file. */
    ResourceClose(G_pictureResFile) ;

    /* Note that we are no longer active. */
    G_picturesActive = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: PictureLock                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureLock locks a picture out of the picture database into memory.  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *name               -- Name of picture to load                */
/*                                                                          */
/*    T_resource *res             -- Pointer to resource to record where    */
/*                                   the picture came from.  Is used        */
/*                                   by PictureUnlock.                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to picture data.               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceFind                                                          */
/*    ResourceLock                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*                                                                          */
/****************************************************************************/

typedef struct {
    T_byte8 resID[4] ;         /* Should contain "ReS"+'\0' id */
    T_byte8 p_resourceName[14] ; /* Case sensitive, 13 characters + '\0' */
    T_word32 fileOffset ;
    T_word32 size ;              /* Size in bytes. */
    T_word16 lockCount ;         /* 0 = unlocked. */
    T_byte8 resourceType ;
    T_byte8 *p_data ;
    T_resourceFile resourceFile ;      /* Resource file this is from. */
    T_void *ownerDir ;        /* Locked in owner directory (or NULL) */
} T_resourceEntry ;

T_byte8 *PictureLock(T_byte8 *name, T_resource *res)
{
    T_resource found ;
    T_byte8 *where = NULL ;

    DebugRoutine("PictureLock") ;
    DebugCheck(name != NULL) ;
    DebugCheck(res != NULL) ;
    DebugCheck(G_picturesActive == TRUE) ;

    /* Look up the picture in the index. */
//printf("> %s\n", name) ;
    found = ResourceFind(G_pictureResFile, name) ;
//printf("Locking pic %s (%p) for %s\n", name, found, DebugGetCallerName()) ;
    if (found == RESOURCE_BAD)  {
#ifndef NDEBUG
        printf("Cannot find picture named '%s'\n", name) ;
#endif
        found = ResourceFind(G_pictureResFile, "DRK42") ;
    }

DebugCheck(found != RESOURCE_BAD) ;

    /* If we found it, we need to lock it in memory. */
    if (found != RESOURCE_BAD)
        where = ResourceLock(found) ;

    /* Record the resource we got the data from.  Needed for unlocking. */
    *res = found ;

    DebugEnd() ;

    /* Return a pointer to the data part. */
    return (where+(2*sizeof(T_word16))) ;
}

/****************************************************************************/
/*  Routine: PictureLockData                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureLockData is the same as PictureLock except that it is used     */
/*  to lock non-pictures in the picture database.  Therefore, all of the    */
/*  data is available (pictures skip some bytes at the beginning).          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *name               -- Name of resource to load               */
/*                                                                          */
/*    T_resource *res             -- Pointer to resource to record where    */
/*                                   the resource came from.  Is used       */
/*                                   by PictureUnlock.                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to picture data.               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceFind                                                          */
/*    ResourceLock                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/17/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *PictureLockData(T_byte8 *name, T_resource *res)
{
    T_resource found ;
    T_byte8 *where = NULL ;

    DebugRoutine("PictureLockData") ;
    DebugCheck(name != NULL) ;
    DebugCheck(res != NULL) ;
    DebugCheck(G_picturesActive == TRUE) ;

    /* Look up the picture in the index. */
    found = ResourceFind(G_pictureResFile, name) ;
#ifndef NDEBUG
    if (found == RESOURCE_BAD)  {
        printf("Cannot find picture named '%s'\n", name) ;
        found = ResourceFind(G_pictureResFile, "DRK42") ;
    }
#endif

DebugCheck(found != RESOURCE_BAD) ;
    /* If we found it, we need to lock it in memory. */
    if (found != RESOURCE_BAD)
        where = ResourceLock(found) ;

    /* Record the resource we got the data from.  Needed for unlocking. */
    *res = found ;

    DebugEnd() ;

    /* Return a pointer to the data part. */
    return where ;
}

/****************************************************************************/
/*  Routine:  PictureUnlock                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureUnlock removes a picture that was in memory.                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_resource res              -- Resource to the picture.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResouceUnlock                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/20/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PictureUnlock(T_resource res)
{
    DebugRoutine("PictureUnlock") ;
    DebugCheck(res != RESOURCE_BAD) ;
    DebugCheck(G_picturesActive == TRUE) ;

//printf("Unlock %s (%p) by %s\n", ((T_resourceEntry *)res)->p_resourceName, res, DebugGetCallerName()) ;
    /* All we need to do at this point is unlock the resource. */
    ResourceUnlock(res) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: PictureExist                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureExist determines if a the given name corresponds to a picture  */
/*  in the picture resource file.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *name               -- Name of picture to check for existance */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = found, FALSE = not found        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceFind                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/29/94  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean PictureExist(T_byte8 *name)
{
    E_Boolean picExist ;
    T_resource res ;

    DebugRoutine("PictureExist") ;
    DebugCheck(name != NULL) ;
    DebugCheck(G_picturesActive == TRUE) ;

    /* Look up the picture in the index. */
    res = ResourceFind(G_pictureResFile, name) ;

    /* Check to see if it is a good resource. */
    picExist = (res == RESOURCE_BAD)?FALSE:TRUE ;

    /* Don't hold onto it, just wanted to know if it was there. */
    if (picExist)
        ResourceUnfind(res) ;

    DebugEnd() ;

    /* Return the boolean telling if the picture exists. */
    return (picExist) ;
}

/****************************************************************************/
/*  Routine: PictureGetXYSize                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureGetXY size gets the size of the picture and returns it by      */
/*  reference.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_picture           -- Pointer to the picture to get size of  */
/*                                                                          */
/*    T_word16 *sizeX             -- Get the size of the picture in the X   */
/*                                                                          */
/*    T_word16 *sizeY             -- Get the size of the picture in the Y   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing                                                               */
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
/*    LES  02/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PictureGetXYSize(T_void *p_picture, T_word16 *sizeX, T_word16 *sizeY)
{
    T_word16 *p_data ;

    DebugRoutine("PictureGetXYSize") ;
    DebugCheck(p_picture != NULL) ;
    DebugCheck(sizeX != NULL) ;
    DebugCheck(sizeY != NULL) ;

    /* Convert to 16 bit word pointer. */
    p_data = (T_word16 *)p_picture ;

    /* Get data behind this point. */
    *sizeX = p_data[-2] ;
    *sizeY = p_data[-1] ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: PictureFind                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureFind finds the corresponding resource handle for the given     */
/*  resource name.                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 * name              -- Name of resource to find.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_resource                  -- Corresponding resource handle or       */
/*                                   RESOURCE_BAD                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceFind                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_resource PictureFind(T_byte8 *name)
{
    T_resource res ;

    DebugRoutine("PictureFind") ;
    DebugCheck(name != NULL) ;

    /* Look up the picture in the index. */
    res = ResourceFind(G_pictureResFile, name) ;

    DebugEnd() ;

    return res ;
}

/****************************************************************************/
/*  Routine: PictureUnfind                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureUnfind removes all references to the given resource picture.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_resource res              -- Resource to unfind                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceUnfind                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PictureUnfind(T_resource res)
{
    DebugRoutine("PictureUnfind") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* Get rid of that picture. */
    ResourceUnfind(res) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: PictureUnlockAndUnfind                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureUnlockAndUnfind does a PictureUnlock and then a PictureUnfind. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_resource res              -- Resource to unlock and unfind          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceUnfind                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/27/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PictureUnlockAndUnfind(T_resource res)
{
    DebugRoutine("PictureUnlockAndUnfind") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* Get rid of that picture. */
    ResourceUnlock(res) ;
    ResourceUnfind(res) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine: PictureLockQuick                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureLockQuick is a simpler lock routine for a picture since it     */
/*  takes the resource handle of an already found picture (from             */
/*  PictureFind).                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Do NOT call GrDrawBitmap (or similar) with the returned pointer       */
/*  from this routine.  Use PictureToBitmap to get the correct pointer.     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_resource res              -- Resource of picture to lock.           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to resource data.              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceLock                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *PictureLockQuick(T_resource res)
{
    T_byte8 *p_where ;

    DebugRoutine("PictureLockQuick") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* If we found it, we need to lock it in memory. */
    if (res != RESOURCE_BAD)
        p_where = ResourceLock(res) ;

    DebugEnd() ;

    /* Return a pointer to the data part. */
    return (p_where+(2*sizeof(T_word16))) ;
}

/****************************************************************************/
/*  Routine: PictureToBitmap                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureToBitmap converts a picture pointer (from PictureLock) into    */
/*  a corresponding bitmap pointer for use by the graphic drawing           */
/*  routines.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure you ONLY pass a pointer from PictureLock or PictureLockQuick*/
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *pic                -- Pointer to picture                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_bitmap *                  -- Pointer to bitmap                      */
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
/*    LES  06/02/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_bitmap *PictureToBitmap(T_byte8 *pic)
{
    T_bitmap *p_bitmap ;

    DebugRoutine("PictureToBitmap") ;
    DebugCheck(pic != NULL) ;

    p_bitmap = (T_bitmap *)pic ;

    DebugEnd() ;

    return(&p_bitmap[-1]) ;
}

/****************************************************************************/
/*  Routine: PictureGetWidth                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureGetWidth returns the pictures with.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_picture           -- Pointer to the picture to get size of  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- picture width                          */
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
/*    LES  02/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 PictureGetWidth(T_void *p_picture)
{
    T_word16 width ;
    T_word16 *p_data ;

    DebugRoutine("PictureGetWidth") ;
    DebugCheck(p_picture != NULL) ;

    /* Convert to 16 bit word pointer. */
    p_data = (T_word16 *)p_picture ;

    /* Get data behind this point. */
    width = p_data[-1] ;

    DebugEnd() ;

    return width ;
}


/****************************************************************************/
/*  Routine: PictureGetHeight                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureGetHeight returns the picture height                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_picture           -- Pointer to the picture to get size of  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- picture height                         */
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
/*    LES  02/21/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 PictureGetHeight(T_void *p_picture)
{
    T_word16 height ;
    T_word16 *p_data ;

    DebugRoutine("PictureGetHeight") ;
    DebugCheck(p_picture != NULL) ;

    /* Convert to 16 bit word pointer. */
    p_data = (T_word16 *)p_picture ;

    /* Get data behind this point. */
    height = p_data[-2] ;

    DebugEnd() ;

    return height ;
}

#ifndef NDEBUG
/****************************************************************************/
/*  Routine:  PicturePrint                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PicturePrint prints out the structure related to the given picture    */
/*  to the given output.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    FILE *fp                    -- File to output picture info            */
/*                                                                          */
/*    T_void *p_pic               -- Picture to print                       */
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
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PicturePrint(FILE *fp, T_void *p_pic)
{
    T_word16 *p_data ;

    DebugRoutine("PicturePrint") ;

    p_data = p_pic ;

    fprintf(fp, "Picture: %p\n", p_pic) ;
    fprintf(fp, "  width: %d\n", p_data[-1]) ;
    fprintf(fp, "  heigh: %d\n", p_data[-2]) ;
    fflush(fp) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PicturesDump                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PicturesDump outputs the picture index file.                          */
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
/*    ResourceDumpIndex                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/24/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PicturesDump(T_void)
{
    DebugRoutine("PicturesDump") ;

    ResourceDumpIndex(G_pictureResFile) ;

    DebugEnd() ;
}

T_void PictureCheck(T_void *p_picture)
{
#if 0
    T_byte8 *p_where ;

    DebugRoutine("PictureCheck") ;
    DebugCheck(p_picture != NULL) ;

    p_where = (((T_byte8 *)p_picture)-(2*sizeof(T_word16))) ;

    ResourceCheckByPtr(p_where) ;

    DebugEnd() ;
#endif
}

#endif

/****************************************************************************/
/*  Routine: PictureLockDataQuick                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureLockQuick is a simpler lock routine for a picture since it     */
/*  takes the resource handle of an already found picture (from             */
/*  PictureFind).                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Do NOT call GrDrawBitmap (or similar) with the returned pointer       */
/*  from this routine.  Use PictureToBitmap to get the correct pointer.     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_resource res              -- Resource of picture to lock.           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to resource data.              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceLock                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *PictureLockDataQuick(T_resource res)
{
    T_byte8 *p_where ;

    DebugRoutine("PictureDataLockQuick") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* If we found it, we need to lock it in memory. */
    if (res != RESOURCE_BAD)
        p_where = ResourceLock(res) ;

    DebugEnd() ;

    /* Return a pointer to the data part. */
    return (p_where) ;
}

/****************************************************************************/
/*  Routine:  PictureGetName                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PictureGetName returns a pointer to the picture's stub name (without  */
/*  sub-directory info.)                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_picture           -- Pointer to the picture                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Found name                             */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ResourceGetName                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *PictureGetName(T_void *p_picture)
{
    T_byte8 *p_name ;

    DebugRoutine("PictureGetName") ;
    DebugCheck(p_picture != NULL) ;

    /* Convert the picture into its basic data form */
    /* and the resource name. */
    p_name = ResourceGetName(PictureToBitmap((T_byte8 *)p_picture)) ;

    DebugEnd() ;

    return p_name ;
}

/****************************************************************************/
/*    END OF FILE:  PICS.C                                                  */
/****************************************************************************/
