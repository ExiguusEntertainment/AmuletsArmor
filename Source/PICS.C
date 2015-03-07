/*-------------------------------------------------------------------------*
 * File:  PICS.C
 *-------------------------------------------------------------------------*/
/**
 * All graphics are stored in the PICS resource file.  This is the accessor
 * code to all those pictures.  Pictures can be locked as a picture or
 * locked as just a raw data file.
 *
 * @addtogroup PICS
 * @brief Picture Resource File
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "PICS.H"

static T_resourceFile G_pictureResFile ;
static E_Boolean G_picturesActive = FALSE ;

/*-------------------------------------------------------------------------*
 * Routine:  PicturesInitialize
 *-------------------------------------------------------------------------*/
/**
 *  PicturesInitialize opens up the picture database in preparation for
 *  all future picture locking and unlocking.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PicturesFinish
 *-------------------------------------------------------------------------*/
/**
 *  PicturesFinish is called when the pictures resource file is no
 *  longer needed (typically when exiting the program).  When this occurs,
 *  the resource file is closed out.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureLock
 *-------------------------------------------------------------------------*/
/**
 *  PictureLock locks a picture out of the picture database into memory.
 *
 *  @param name -- Name of picture to load
 *  @param res -- Pointer to resource to record where
 *      the picture came from.  Is used
 *      by PictureUnlock.
 *
 *  @return Pointer to picture data.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *PictureLock(T_byte8 *name, T_resource *res)
{
    T_resource found ;
    T_byte8 *where = NULL ;
    char pngName[200];
    char *p;

    DebugRoutine("PictureLock") ;
    DebugCheck(name != NULL) ;
    DebugCheck(res != NULL) ;
    DebugCheck(G_picturesActive == TRUE) ;

    // Convert all pictures into .png file names
    strcpy(pngName, name);
    p = strchr(pngName, '.');
    if (!p)
        p = pngName + strlen(pngName);
    strcpy(p, ".png");

    /* Look up the picture in the index. */
//printf("> %s\n", name) ;
    found = ResourceFind(G_pictureResFile, pngName) ;
//printf("Locking pic %s (%p) for %s\n", name, found, DebugGetCallerName()) ;
    if (found == RESOURCE_BAD)  {
#ifndef NDEBUG
        printf("Cannot find picture named '%s'\n", pngName) ;
#endif
        found = ResourceFind(G_pictureResFile, "DRK42.png") ;
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureLockData
 *-------------------------------------------------------------------------*/
/**
 *  PictureLockData is the same as PictureLock except that it is used
 *  to lock non-pictures in the picture database.  Therefore, all of the
 *  data is available (pictures skip some bytes at the beginning).
 *
 *  @param name -- Name of resource to load
 *  @param res -- Pointer to resource to record where
 *      the resource came from.  Is used
 *      by PictureUnlock.
 *
 *  @return Pointer to picture data.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *PictureLockData(const char *name, T_resource *res)
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

T_byte8 *PictureLockPNGAsPIC(T_byte8 *name, T_resource *res)
{
    T_resource found ;
    T_byte8 *where = NULL ;
    char pngName[200];
    char *p;

    DebugRoutine("PictureLockData") ;
    DebugCheck(name != NULL) ;
    DebugCheck(res != NULL) ;
    DebugCheck(G_picturesActive == TRUE) ;

    // Convert all pictures into .png file names
    strcpy(pngName, name);
    p = strchr(pngName, '.');
    if (!p)
        p = pngName + strlen(pngName);
    strcpy(p, ".png");

    /* Look up the picture in the index. */
    found = ResourceFind(G_pictureResFile, pngName) ;
#ifndef NDEBUG
    if (found == RESOURCE_BAD)  {
        printf("Cannot find picture named '%s'\n", pngName) ;
        found = ResourceFind(G_pictureResFile, "DRK42.png") ;
    }
#endif

    DebugCheck(found != RESOURCE_BAD) ;

    /* If we found it, we need to lock it in memory. */
    if (found != RESOURCE_BAD)
        where = ResourceLock(found);

    /* Record the resource we got the data from.  Needed for unlocking. */
    *res = found ;

    DebugEnd() ;

    /* Return a pointer to the data part. */
    return where ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PictureUnlock
 *-------------------------------------------------------------------------*/
/**
 *  PictureUnlock removes a picture that was in memory.
 *
 *  @param res -- Resource to the picture.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureExist
 *-------------------------------------------------------------------------*/
/**
 *  PictureExist determines if a the given name corresponds to a picture
 *  in the picture resource file.
 *
 *  @param name -- Name of picture to check for existance
 *
 *  @return TRUE = found, FALSE = not found
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean PictureExist(const char *name)
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureGetXYSize
 *-------------------------------------------------------------------------*/
/**
 *  PictureGetXY size gets the size of the picture and returns it by
 *  reference.
 *
 *  @param p_picture -- Pointer to the picture to get size of
 *  @param sizeX -- Get the size of the picture in the X
 *  @param sizeY -- Get the size of the picture in the Y
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureFind
 *-------------------------------------------------------------------------*/
/**
 *  PictureFind finds the corresponding resource handle for the given
 *  resource name.
 *
 *  @param name -- Name of resource to find.
 *
 *  @return Corresponding resource handle or
 *      RESOURCE_BAD
 *
 *<!-----------------------------------------------------------------------*/
T_resource PictureFind(const char *name)
{
    T_resource res ;

    DebugRoutine("PictureFind") ;
    DebugCheck(name != NULL) ;

    /* Look up the picture in the index. */
    res = ResourceFind(G_pictureResFile, name) ;

    DebugEnd() ;

    return res ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PictureUnfind
 *-------------------------------------------------------------------------*/
/**
 *  PictureUnfind removes all references to the given resource picture.
 *
 *  @param res -- Resource to unfind
 *
 *<!-----------------------------------------------------------------------*/
T_void PictureUnfind(T_resource res)
{
    DebugRoutine("PictureUnfind") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* Get rid of that picture. */
    ResourceUnfind(res) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PictureUnlockAndUnfind
 *-------------------------------------------------------------------------*/
/**
 *  PictureUnlockAndUnfind does a PictureUnlock and then a PictureUnfind.
 *
 *  @param res -- Resource to unlock and unfind
 *
 *<!-----------------------------------------------------------------------*/
T_void PictureUnlockAndUnfind(T_resource res)
{
    DebugRoutine("PictureUnlockAndUnfind") ;
    DebugCheck(res != RESOURCE_BAD) ;

    /* Get rid of that picture. */
    ResourceUnlock(res) ;
    ResourceUnfind(res) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PictureLockQuick
 *-------------------------------------------------------------------------*/
/**
 *  PictureLockQuick is a simpler lock routine for a picture since it
 *  takes the resource handle of an already found picture (from
 *  PictureFind).
 *
 *  NOTE:
 *  Do NOT call GrDrawBitmap (or similar) with the returned pointer
 *  from this routine.  Use PictureToBitmap to get the correct pointer.
 *
 *  @param res -- Resource of picture to lock.
 *
 *  @return Pointer to resource data.
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *PictureLockByResource(T_resource res)
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureToBitmap
 *-------------------------------------------------------------------------*/
/**
 *  PictureToBitmap converts a picture pointer (from PictureLock) into
 *  a corresponding bitmap pointer for use by the graphic drawing
 *  routines.
 *
 *  NOTE: 
 *  Make sure you ONLY pass a pointer from PictureLock or PictureLockQuick
 *
 *  @param pic -- Pointer to picture
 *
 *  @return Pointer to bitmap
 *
 *<!-----------------------------------------------------------------------*/
T_bitmap *PictureToBitmap(T_byte8 *pic)
{
    T_bitmap *p_bitmap ;

    DebugRoutine("PictureToBitmap") ;
    DebugCheck(pic != NULL) ;

    p_bitmap = (T_bitmap *)pic ;

    DebugEnd() ;

    return(&p_bitmap[-1]) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PictureGetWidth
 *-------------------------------------------------------------------------*/
/**
 *  PictureGetWidth returns the pictures with.
 *
 *  @param p_picture -- Pointer to the picture to get size of
 *
 *  @return picture width
 *
 *<!-----------------------------------------------------------------------*/
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


/*-------------------------------------------------------------------------*
 * Routine:  PictureGetHeight
 *-------------------------------------------------------------------------*/
/**
 *  PictureGetHeight returns the picture height
 *
 *  @param p_picture -- Pointer to the picture to get size of
 *
 *  @return picture height
 *
 *<!-----------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------*
 * Routine:  PicturePrint
 *-------------------------------------------------------------------------*/
/**
 *  PicturePrint prints out the structure related to the given picture
 *  to the given output.
 *
 *  @param fp -- File to output picture info
 *  @param p_pic -- Picture to print
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PicturesDump
 *-------------------------------------------------------------------------*/
/**
 *  PicturesDump outputs the picture index file.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureLockDataQuick
 *-------------------------------------------------------------------------*/
/**
 *  PictureLockQuick is a simpler lock routine for a picture since it
 *  takes the resource handle of an already found picture (from
 *  PictureFind).
 *
 *  NOTE: 
 *  Do NOT call GrDrawBitmap (or similar) with the returned pointer
 *  from this routine.  Use PictureToBitmap to get the correct pointer.
 *
 *  @param res -- Resource of picture to lock.
 *
 *  @return Pointer to resource data.
 *
 *<!-----------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------*
 * Routine:  PictureGetName
 *-------------------------------------------------------------------------*/
/**
 *  PictureGetName returns a pointer to the picture's stub name (without
 *  sub-directory info.)
 *
 *  @param p_picture -- Pointer to the picture
 *
 *  @return Found name
 *
 *<!-----------------------------------------------------------------------*/
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

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PICS.C
 *-------------------------------------------------------------------------*/
