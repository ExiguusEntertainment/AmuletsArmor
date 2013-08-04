/*-------------------------------------------------------------------------*
 * File:  OBJTYPE.C
 *-------------------------------------------------------------------------*/
/**
 * This would probably be better called "Object Animation System".
 * ObjType is the code that handles animations of objects as they step
 * through pre-determined animation types.  So, you can have a creature
 * with a multi-step death scene yell and fall to the ground using this.
 * Objects have "Info" file resources that have all the animation
 * information compiled into them.
 *
 * @addtogroup OBJTYPE
 * @brief Object Types
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "AREASND.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "OBJTYPE.H"
#include "PICS.H"
#include "SYNCTIME.H"
#include "VIEW.H"
#include "VIEWFILE.H"

/* Internal defination:  Handle means that there is a copied resource, */
/* but not an actual one. */
#define INTERNAL_OBJECT_TYPE_COPIED_RESOURCE 0xFFFF0000
#define OBJECT_TYPE_PICTURE_IS_COPY          0x1000
#define OBJECT_TYPE_PICTURE_IS_ORIGINAL      0x2000
#define OBJECT_TYPE_PICTURE_NEED_DRAW        0x4000

static T_word16 G_frameResolution = 0 ;
static T_word16 G_realFrameResolution = 0 ;

typedef T_sword16 T_bodyParts[MAX_BODY_PARTS] ;

static E_Boolean G_somewhatLow = FALSE ;

/*-------------------------------------------------------------------------*
 * Enbumeration:  E_objectAnimateType
 *-------------------------------------------------------------------------*/
/**
 *  E_objectAnimateType declares the type of animation "movement" that
 *  an animate object undergoes.  Below is the different types.
 *
 *  @param ORDERED -- 1, 2, 3, 4, 1, 2, 3, 4, etc.
 *  @param BOUNCE -- 1, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, etc.
 *  @param RANDOM -- Like it says, bro.  1423424242E43144231123123
 *
 *<!-----------------------------------------------------------------------*/
/*
typedef enum T_word16 {
    OBJECT_ANIMATE_ORDERED=0,
    OBJECT_ANIMATE_BOUNCE,
    OBJECT_ANIMATE_RANDOM,
    OBJECT_ANIMATE_UNKNOWN
} E_objectAnimateType ;
*/

/** !!! Apparently, GNU CC does not allow you to specify the storage type **/
/** for an enum!  It doesn't issue a warning, it just makes all enums into **/
/** ints, which in our world are T_word32's.  Thus the structure no longer **/
/** works with the same res file.  For now, I just redefined things as **/
/** follows, to get it to work. **/

typedef T_word16 E_objectAnimateType;

#define OBJECT_ANIMATE_ORDERED 0
#define OBJECT_ANIMATE_BOUNCE  1
#define OBJECT_ANIMATE_RANDOM  2
#define OBJECT_ANIMATE_UNKNOWN 3

/*-------------------------------------------------------------------------*
 * Typedef:  T_objectStance
 *-------------------------------------------------------------------------*/
/**
 *  An animated object can be broken down into the number of stances
 *  that makes up the animation.  A stance is in itself an animation
 *  state (e.g., walking, talking, flying, etc.) or transition
 *  (e.g. landing, getting hit, taking off, etc.).
 *
 *  @param numFrames -- Number of frames that make up this stance.
 *  @param speed -- Interval in between animation frames.
 *      A speed of 0 means not to animate at all.  This
 *      is useful for objects with only one frame.
 *  @param type -- What type of animation
 *      (See E_objectAnimateType)
 *  @param nextStance -- Stance to go to after completion of animation.
 *  @param offsetFrameList -- Offset in bytes from the beginning of
 *      T_objectType structure that this stance is in to
 *      the list of frames that make up this stance.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_word16             numFrames          PACK ;
    T_word16             speed              PACK ;
    E_objectAnimateType  type               PACK ;
    T_word16             nextStance         PACK ;
    T_word16             offsetFrameList    PACK ;
} T_objectStance ;

/*-------------------------------------------------------------------------*
 * Typedef:  T_objectFrame
 *-------------------------------------------------------------------------*/
/**
 *  An object Frame is a part of a stance animation at one moment.
 *  A frame can have either 1 or 8 pictures for each angle.
 *
 *  @param numAngles -- how many views does this frame have.   Only
 *      1 or 8 is allowed.
 *  @param offsetPicList -- Offset in bytes from the beginning of
 *      T_objectType structure that this is in to the
 *      list of pics (T_objectPic) that make up this
 *      frame.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_byte8 numAngles                       PACK ;
    T_word16 offsetPicList                  PACK ;
    T_word16 soundNum     /* 0 = no sound */PACK ;
    T_word16 soundRadius                    PACK ;
    T_word16 objectAttributes               PACK ;
} T_objectFrame ;

/*-------------------------------------------------------------------------*
 * Typedef:  T_objectPic
 *-------------------------------------------------------------------------*/
/**
 *  Picture information block for one angle of a frame.
 *
 *  @param number -- Index number of picture to use in resource file
 *      for this object.
 *  @param resource -- Resource for picture when locked in memory.
 *      NULL when not locked in memory.
 *  @param p_pic -- Pointer to picture when locked in memory.
 *      NULL when not locked in memory.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_sword16 number                          PACK ;
    T_resource resource                       PACK ;
    T_void *p_pic                             PACK;
} T_objectPic ;

/*-------------------------------------------------------------------------*
 * Typedef:  T_objectType
 *-------------------------------------------------------------------------*/
/**
 *  T_objectType is the description structure for an object used in the
 *  game.  It describes everything that needs to be know about this object
 *  and how it interacts with the world.   This is basically the class
 *  description for the object, and instances are made from this type.
 *
 *  @param numStances -- Number of stances that make up this object
 *  @param lockCount -- How many times this type of object has been
 *      locked into memory.  This feature is used to try
 *      to keep as much art in memory as possible.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_word16 numStances               PACK ;
    T_word32 lockCount                PACK ;
    T_word16 radius                   PACK ;
    T_word16 attributes               PACK ;
    T_word16 weight                   PACK ;
    T_word16 value                    PACK ;
    T_word32 script                   PACK ;
    T_word16 health                   PACK ;
    T_word16 objMoveAttr              PACK ;
    T_objectStance stances[1]         PACK ;
} T_objectType ;

/*-------------------------------------------------------------------------*
 * Typedef:  T_objTypeInstanceStruct
 *-------------------------------------------------------------------------*/
/**
 *  For each object there is a T_objTypeInstanceStruct that tells what
 *  type of object it is, where the information about it is, and what
 *  state of animation it is in.
 *
 *  @param nextAnimationTime -- When to update the animation cycling.
 *  @param objTypeNum -- Number of the object type.
 *  @param resource -- Resource handle for the object type info block.
 *  @param p_objectType -- Pointer to the actual   object type info block.
 *  @param stanceNumber -- Current stance number
 *  @param frameNumber -- Current frameNumber
 *  @param stateData -- accessory data depending on the type
 *  @param p_obj -- pointer to object whose type I am
 *  @param T_areaSound -- area sound handle for my current sound.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_word32 nextAnimationTime ;
    T_word16 objTypeNum ;
    T_resource resource ;
    T_objectType *p_objectType ;
    T_word16 stanceNumber ;
    T_word16 frameNumber ;
    T_word16 stateData ;
    T_3dObject *p_obj;
    T_areaSound currSound;
    T_bodyParts *p_parts ;
    E_Boolean isActive ;
} T_objTypeInstanceStruct ;


/* Internal prototypes: */
static T_void IObjTypeLock(T_objectType *p_type, T_word16 typeNumber) ;
static T_void IObjTypeUnlock(T_objectType *p_type) ;
static T_void IObjTypeUpdateFrameChanges (T_objTypeInstanceStruct *p_objType);
static T_void IObjTypeFreePieces(T_objectType *p_type) ;
static T_void *IBuildView(
                   T_objTypeInstanceStruct *p_objType,
                   T_word16 stance,
                   T_word16 frame,
                   T_word16 angle) ;
static T_byte8 *ICompressPicture(T_byte8 *p_picture) ;
static T_void IOverlayPicture(T_byte8 *p_picture, T_byte8 *p_workArea) ;




/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeCreate
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeCreate instantiates a structure for the given object type
 *  number. In addition, all pictures are locked into memory as needed.
 *
 *  @param objTypeNum -- Type of object to instantiate
 *  @param p_obj -- Pointer to 3D Object to link.
 *
 *  @return Handle to created object instance, or
 *      OBJECT_TYPE_INSTANCE_BAD
 *
 *<!-----------------------------------------------------------------------*/
T_objTypeInstance ObjTypeCreate(T_word16 objTypeNum, T_3dObject *p_obj)
{
    T_objTypeInstanceStruct *p_objType ;
    T_byte8 resName[80] ;
    T_objectType *p_type ;
    T_objectFrame *p_frame ;

    DebugRoutine("ObjTypeCreate") ;

    /* Create the instance data structure. */
    p_objType = MemAlloc(sizeof(T_objTypeInstanceStruct)) ;
//printf("ObjTypeCreate %d for %p (created %p)\n", objTypeNum, p_obj, p_objType) ;
    DebugCheck(p_objType != NULL) ;

    if (p_objType != NULL)  {
        /* Initialize the instance data. */
        memset(p_objType, 0, sizeof(T_objTypeInstanceStruct)) ;
        p_objType->objTypeNum = objTypeNum ;
        p_objType->resource = RESOURCE_BAD ;

        /* Find and lock the object type information into memory. */
        sprintf(resName, "OBJS/%05d/Info", objTypeNum) ;
//printf("!A 1 OBJ%05d\n", objTypeNum) ;

        /* Note that PictureLockData does a PictureFind */
        p_type = p_objType->p_objectType =
           (T_objectType *)PictureLockData(resName, &p_objType->resource) ;
        DebugCheck(p_objType->resource != RESOURCE_BAD) ;

        /* Does this instance/type need to be a piece-wise object */
        /* with its own objtype? */
#ifndef SERVER_ONLY
#if 0
        if (p_type->attributes & OBJECT_ATTR_PIECE_WISE)  {
            /* Make a copy of this type iff it is not the first */
            /* object type. */
//            if (p_type->lockCount != 0)  {
                /* How big is the original? */
                sizeObjType = ResourceGetSize(p_objType->resource) ;

                /* Allocate room for a copy. */
                p_typeCopy = MemAlloc(sizeObjType) ;
                DebugCheck(p_typeCopy != NULL) ;
                if (p_typeCopy)  {
                    /* Do the copy. */
                    memcpy(p_typeCopy, p_type, sizeObjType) ;

                    /* Make this copy the one we want to use. */
                    p_type = p_typeCopy ;
                    p_objType->p_objectType = p_typeCopy ;

                    /* Clear out the resource handle to say that */
                    /* this is a copy. */
                    p_objType->resource =
                        (T_resource)INTERNAL_OBJECT_TYPE_COPIED_RESOURCE ;

                    /* Reset the lock count for this type. */
                    p_type->lockCount = 0 ;
                }

//            }

            /* Allocate memory for the piece-wise information. */
            p_objType->p_parts = MemAlloc(sizeof(T_bodyParts)) ;
            DebugCheck(p_objType->p_parts != NULL) ;
            if (p_objType->p_parts)  {
                memset(p_objType->p_parts, 0, sizeof(T_bodyParts)) ;
//(*p_objType->p_parts)[2] = -1 ;
            }
        }
#endif
#endif

        /* See if we need to lock the object type pictures into memory. */
//printf("Lock count: %d\n", p_type->lockCount) ;
        if (p_type->lockCount == 0)  {
            /* Go through and lock them all in. */
#if 0
            if (!(p_type->attributes & OBJECT_ATTR_PIECE_WISE))
#endif
                IObjTypeLock(p_type, objTypeNum) ;
        }

        /* Make an object type not active unless the object */
        /* has multiple stances or multiple frames in its one stance. */
        p_frame = (T_objectFrame *)
                     (&((T_byte8 *)p_type)[p_type->stances[0].offsetFrameList]) ;
        if ((p_type->numStances > 1) || (p_type->stances[0].numFrames > 1) || (p_frame->numAngles > 1))
            p_objType->isActive = TRUE ;
        else
            p_objType->isActive = FALSE ;

        /* Increment the lock count.  This is necessary for determining */
        /* when to unlock all the object type pictures. */
        p_type->lockCount++ ;

        /** Clear out the current sound handle. **/
        p_objType->currSound = AREA_SOUND_BAD;

        /** And, finally, set the object pointer. **/
        p_objType->p_obj = p_obj;

    } else {
        DebugCheck(FALSE) ;
    }

/* TESTING:  Make piecewise person a creature with BIFF intelligence */
//if (objTypeNum == 510)  {
//   p_type->script = 2 ;
//}
/**/
    DebugEnd() ;

    /* Return the instance data as the handle to all future accesses. */
    return (T_objTypeInstance)(p_objType) ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeDestroy
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeDestroy removes all references to the object type of an object
 *  and, if necessary, unlocks all the pictures attached to it.
 *
 *  @param objTypeInst -- Type of object to destroy
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypeDestroy(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;

    DebugRoutine("ObjTypeDestroy") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

//printf("!F 1 OBJ%05d\n", p_objType->objTypeNum) ;

    /* Make sure this instance is attached to some info block. */
    DebugCheck(p_objType->resource != RESOURCE_BAD) ;
    DebugCheck(p_objType->p_objectType != NULL) ;

    /* Get the pointer to the type. */
    p_type = p_objType->p_objectType ;
    DebugCheck(p_type != NULL) ;

    /* Decrement the lock count. */
    p_type->lockCount-- ;
//printf("Lock count now: %d\n", p_type->lockCount) ;

    /* Make sure we didn't roll under. */
    DebugCheck(p_type->lockCount != ((T_word32)-1)) ;

    /* Is the lock count now zero? */
    if (p_type->lockCount == 0)  {
        /* Yes.  We are now free to release this object type structure and */
        /* all the pictures.  */
        /* If this is a piecewise object, don't unlock the resources */
        /* (there are none), just free the memory. */
#ifndef SERVER_ONLY
#if 0
        if (p_type->attributes & OBJECT_ATTR_PIECE_WISE)
            IObjTypeFreePieces(p_type) ;
        else
#endif
#endif
            IObjTypeUnlock(p_type) ;
    }

    /* If piecewise, free the body pieces information block from */
    /* memory. */
    if (p_type->attributes & OBJECT_ATTR_PIECE_WISE)
        if (p_objType->p_parts)  {
            MemFree(p_objType->p_parts) ;
        }

    /* Release any connections. */
    if (p_objType->resource != (T_resource)INTERNAL_OBJECT_TYPE_COPIED_RESOURCE)  {
        /* This is the real thing.  Release it. */
        PictureUnlockData(p_objType->resource) ;
        PictureUnfind(p_objType->resource) ;
    } else {
        /* This is a copy in memory.  Just free it from memory. */
        MemFree(p_objType->p_objectType) ;
#ifndef NDEBUG
        memset(p_objType->p_objectType, 0xCC, sizeof(T_objectType)) ;
#endif
    }

#ifndef NDEBUG
    memset(p_objType, 0x11, sizeof(T_objTypeInstanceStruct)) ;
#endif
    /* Now we can free the instance block. */
    MemFree(p_objType) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeAnimate
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeAnimate updates the animation state for the object.  However,
 *  an object must call ObjTypeGetPicture followed with a call to
 *  ObjectSetPicture to get the image to update.  This routine is ONLY
 *  to change the state.
 *
 *  @param objTypeInst -- Handle to the instance to update
 *  @param currentTime -- current time to update for
 *
 *  @return TRUE if there is a change
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean ObjTypeAnimate(
              T_objTypeInstance objTypeInst,
              T_word32 currentTime)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_objectStance *p_stance ;
    T_word16 stanceNum;
    E_Boolean changed = FALSE ;
    T_objectFrame *p_frame ;

    DebugRoutine("ObjTypeAnimate") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Is it time to animate?   Keep animating if we have to */
    /* catch up.  */
    while ((currentTime >= p_objType->nextAnimationTime) ||
           (p_objType->nextAnimationTime == 0))  {
        /* Make sure this instance is attached to some info block. */
        DebugCheck(p_objType->resource != RESOURCE_BAD) ;
        DebugCheck(p_objType->p_objectType != NULL) ;

        /* Get the pointer to the type. */
        p_type = p_objType->p_objectType ;

        /* Get the stance number. */
        stanceNum = p_objType->stanceNumber ;

        /* Get the stance pointer for the current stance. */
        p_stance = &p_type->stances[stanceNum] ;

        /* Calculate the next update time. */
        if (p_objType->nextAnimationTime == 0)
            p_objType->nextAnimationTime = currentTime ;
        else
            p_objType->nextAnimationTime += p_stance->speed ;

        /* Check to see if the current frame is not above the limit */
 #ifndef NDEBUG
 if (p_objType->frameNumber >= p_stance->numFrames)  {
  printf("frame#: %d, max: %d\n",
      p_objType->frameNumber,
      p_stance->numFrames) ;
  ObjTypePrint(stdout, p_objType) ;
 }
 #endif
        DebugCheck(p_objType->frameNumber < p_stance->numFrames) ;

        /* If we have a speed of zero, don't update this one. */
        if (p_stance->speed == 0)  {
            p_frame = (T_objectFrame *)
                         (&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;
            p_frame += p_objType->frameNumber ;
            if (p_frame->numAngles != 1)
                changed = TRUE ;
            break ;
        }

        /* Check to see if this stance can have any animation */
        /* tied to it. */
 //        if (p_stance->numFrames > 1)  {
            /* Now, based on the state, update the frame code */
            switch (p_stance->type)  {
                /* ---------------------------------------------------- */
                case OBJECT_ANIMATE_ORDERED:
                    /* Go sequentially through the list. 1, 2, 3, ... */
                    p_objType->frameNumber++ ;

                    /* Have we gone past the end of the list. */
                    if (p_objType->frameNumber >= p_stance->numFrames)  {
                        /* Yes, we have. */
                        /* Is there another stance to go to? */
                        if (p_stance->nextStance != 0xFFFF)  {
                            /* Move to the next stance and its first frame. */
                            p_objType->stanceNumber = p_stance->nextStance ;
                            p_objType->frameNumber = 0 ;
                        } else {
                            /* Otherwise, stop animating by setting the */
                            /* clock to a REALLY BIG value (if you */
                            /* are worried about this, I'll let you */
                            /* know that it takes over a year of continuous */
                            /* play before this time is reached. */
                            p_objType->nextAnimationTime = 0xFFFFFFFF ;

                            /* Stay on the last frame of animation. */
                            p_objType->frameNumber-- ;
                        }

                        /* To be clean, clear out the state data. */
                        p_objType->stateData = 0 ;
                    }
                    break ;
                /* ---------------------------------------------------- */
                case OBJECT_ANIMATE_BOUNCE:
                    /* Bounce is like sequential, but goes back and forth. */
                    /* The nextStance does not matter in this case. */
                    /* The state data is either 0 or 1 for this type of */
                    /* animation. */
                    if (p_objType->stateData == 0)  {
                        /* Go 1, 2, 3, ... */
                        p_objType->frameNumber++ ;

                        /* Do we need to bounce? */
                        if (p_objType->frameNumber >= (p_stance->numFrames-1))
                            /* Now go in reverse. */
                            p_objType->stateData = 1 ;
                    } else if (p_objType->stateData == 1)  {
                        /* Go 3, 2, 1, ... */
                        p_objType->frameNumber-- ;

                        /* Do we need to bounce? */
                        if (p_objType->frameNumber == 0)
                            /* Now go in order. */
                            p_objType->stateData = 0 ;
                    } else {
                        DebugCheck(p_objType->stateData < 2) ;
                    }
                    break ;
                /* ---------------------------------------------------- */
                case OBJECT_ANIMATE_RANDOM:
                    /* Random does like it says, picking randomly from */
                    /* the list of frames. */
                    p_objType->frameNumber = (rand() % (p_stance->numFrames)) ;
                    break ;
                /* ---------------------------------------------------- */
                default:
                    /* Better check for illegal types. */
 #ifndef NDEBUG
                    if (p_stance->type >= OBJECT_ANIMATE_UNKNOWN)  {
                        printf("Bad animation stance %d (%p)\n", p_stance->type, p_stance) ;
                        ObjTypePrint(stdout, objTypeInst) ;
                    }
 #endif
                    DebugCheck(p_stance->type < OBJECT_ANIMATE_UNKNOWN) ;

                    /* Let through any that are not handled from above. */
                    break ;
            }
 //        }


        /** Now that we've updated the stances, we need to take care of **/
        /** the sounds and attributes associated with the current frame. **/

        /* Make sure we have a legal frame number. */
 #ifndef NDEBUG
        if (p_objType->frameNumber >= p_stance->numFrames)  {
            printf("Bad animation frame %d\n", p_objType->frameNumber) ;
            ObjTypePrint(stdout, objTypeInst) ;
        }
 #endif
        DebugCheck(p_objType->frameNumber < p_stance->numFrames) ;

        /* Note that a change occured. */
        changed = TRUE ;
    }

    /** Check to see if any change occurred in the stance or frame. **/
    if (changed == TRUE)
    {
        /** Yes.  Let's see if any attributes or sounds are attached. **/
        IObjTypeUpdateFrameChanges (p_objType);
    }

    DebugEnd() ;

    return changed ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IObjTypeUpdateFrameChanges
 *-------------------------------------------------------------------------*/
/**
 *  This routine checks the current stance/frame of the given object type
 *  instance, and if applicable, creates sounds and changes attributes as
 *  directed by the .oaf description file.
 *
 *  @param p_objType -- pointer to obj.type instance
 *
 *<!-----------------------------------------------------------------------*/
static T_void IObjTypeUpdateFrameChanges (T_objTypeInstanceStruct *p_objType)
{
   T_objectType *p_type;
   T_objectStance *p_stance;
   T_objectFrame *p_frame;
   T_word16 attribs ;
   T_word16 permanentAttribs ;

   DebugRoutine ("ObjTypeUpdateFrameChanges");

   /** Get a pointer to the appropriate frame list. **/
   p_type = p_objType->p_objectType;
   p_stance = &p_type->stances[p_objType->stanceNumber] ;
   p_frame = (T_objectFrame *)
                (&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;

/** Only client+server machines can even make sounds. **/
   /** Is there a sound? **/
#if 0  /* Do not play sounds -- this is probably going to be removed totally. */
   if (p_frame[p_objType->frameNumber].soundNum != 0)
   {
       /** Make the new sound via the area sound facility. **/
       p_objType->currSound = AreaSoundCreate(
           ObjectGetX16 (p_objType->p_obj),
           ObjectGetY16 (p_objType->p_obj),
           p_frame->soundRadius,
           AREA_SOUND_MAX_VOLUME,
           AREA_SOUND_TYPE_ONCE,
           20,
           NULL,
           NULL,
           0,
           p_frame->soundNum) ;
   }
#endif

   /** Is there an attribute change? **/
   attribs = p_frame[p_objType->frameNumber].objectAttributes ;
   if (attribs != 0)
   {
       if (attribs & OBJECT_ATTR_MARK_FOR_DESTROY)
           ObjectMarkForDestroy(p_objType->p_obj) ;

       /** Yes.  Just set the attribute value (for now) **/
       permanentAttribs = ObjectGetAttributes(p_objType->p_obj) /* &
                              OBJECT_ATTR_BODY_PART ; */ ;
       ObjectSetAttributes (
           p_objType->p_obj,
           permanentAttribs | attribs);
   }

   DebugEnd ();
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetRadius
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetRadius returns the defined radius for the given object
 *  type's instance data.
 *
 *  @param objTypeInst -- Handle to the instance to update
 *
 *  @return Radius of object type
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetRadius(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_word16 radius ;

    DebugRoutine("ObjTypeGetRadius") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the pointer to the type. */
    p_type = p_objType->p_objectType ;

    /* Get the radius */
    radius = p_type->radius ;

    /* Make sure it is not a rediculous value. */
    DebugCheck(radius < 500) ;
    DebugEnd() ;

    return radius ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetAttributes
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetAttributes returns the set of predefined characteristic
 *  flags defined for this object type.
 *
 *  @param objTypeInst -- Handle to the instance to update
 *
 *  @return Attributes of object type
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetAttributes(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_word16 attributes ;

    DebugRoutine("ObjTypeGetAttributes") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the pointer to the type. */
    p_type = p_objType->p_objectType ;

    /* Get the radius */
    attributes = p_type->attributes ;

    /* Make sure it is not a rediculous value. */
    DebugEnd() ;

    return attributes ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetPicture
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetPicture returns the pointer to a picture that is appropriate
 *  for the current stance, frame, and angle.
 *
 *  @param objTypeInst -- Handle to the instance to update
 *  @param angle -- Angle that object is facing relative
 *      to the view.
 *  @param p_orient -- Pointer to picture orientation
 *
 *  @return Attributes of object type
 *
 *<!-----------------------------------------------------------------------*/
T_void *ObjTypeGetPicture(
           T_objTypeInstance objTypeInst,
           T_word16 angle,
           T_orientation *p_orient)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectPic *p_pic ;
    T_byte8 *p_picData ;
    T_word16 oldAngle ;

    DebugRoutine("ObjTypeGetPicture") ;
    DebugCheck(objTypeInst != NULL) ;

    oldAngle = angle ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the pointer to the type. */
    p_type = p_objType->p_objectType ;

    /* Make sure we have a legal stance number. */
#ifndef NDEBUG
    if (p_objType->stanceNumber >= p_type->numStances)  {
        printf("Bad animation stance %d\n", p_objType->stanceNumber) ;
        ObjTypePrint(stdout, objTypeInst) ;
    }
#endif
    DebugCheck(p_objType->stanceNumber < p_type->numStances) ;

    /* Get a pointer to the stance. */
    p_stance = &p_type->stances[p_objType->stanceNumber] ;

    /* Make sure we have a legal frame number. */
#ifndef NDEBUG
    if (p_objType->frameNumber >= p_stance->numFrames)  {
        printf("Bad animation frame %d\n", p_objType->frameNumber) ;
        ObjTypePrint(stdout, objTypeInst) ;
    }
#endif
    DebugCheck(p_objType->frameNumber < p_stance->numFrames) ;

    /* Get a pointer to the appropriate frame list. */
    p_frame = (T_objectFrame *)
                 (&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;

    /* Get a pointer to the picture list. */
    p_pic = (T_objectPic *)
                 (&((T_byte8 *)p_type)
                     [p_frame[p_objType->frameNumber].offsetPicList]) ;

    /* Determine the angle lookup. */
    if (p_frame->numAngles == 8)  {
        angle = (((T_word16)(angle + ((INT_ANGLE_45/2)-1))) >> 13) ;
    } else if (p_frame->numAngles == 4)  {
        angle = (((T_word16)(angle + ((INT_ANGLE_90/2)-1))) >> 14) ;
    }  else  {
        angle = 0 ;
    }

#ifndef NDEBUG
    if (angle >= p_frame->numAngles)  {
        printf("angle: %04X  numAng: %d  ", oldAngle, p_frame->numAngles) ;
        printf("stanceNum: %d  frameNum: %d\n", p_objType->stanceNumber, p_objType->frameNumber) ;
        printf("trying angle %d\n", angle) ;
        ObjTypePrint(stdout, objTypeInst) ;
    }
#endif
    DebugCheck(angle < p_frame->numAngles) ;

    /* Set up the orientation for that picture. */
    if (p_pic[angle].number < 0)
        *p_orient = ORIENTATION_REVERSE ;
    else
        *p_orient = ORIENTATION_NORMAL ;

if ((p_pic[angle].resource == RESOURCE_BAD) &&
    (p_pic[angle].number == OBJECT_TYPE_PICTURE_NEED_DRAW))  {
    p_pic[angle].p_pic = IBuildView(
        p_objType,
        p_objType->stanceNumber,
        p_objType->frameNumber,
        angle) ;
    p_pic[angle].number = OBJECT_TYPE_PICTURE_IS_ORIGINAL ;
}
    /* Look up that picture. */
    p_picData = p_pic[angle].p_pic ;

#ifndef NDEBUG
    if (p_picData == NULL)
        ObjTypePrint(stdout, objTypeInst) ;
#endif
    DebugCheck(p_picData != NULL) ;
    DebugEnd() ;

    return p_picData ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypesSetResolution
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypesSetResolution declares what power of two to cut out pictures
 *  from animation frames.
 *
 *  @param resolution -- Power of 2 loss (0 = none, 1=half,
 *      2=fourth, 3=eighth)
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypesSetResolution(T_word16 resolution)
{
    DebugRoutine("ObjTypesSetResolution") ;
    DebugCheck(resolution < 4) ;

    G_realFrameResolution = resolution ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypesGetResolution
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypesGetResolution retruns  what power of two to cut out pictures
 *  from animation frames.
 *
 *  @return Power of 2 loss (0 = none, 1=half,
 *      2=fourth, 3=eighth)
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypesGetResolution(T_void)
{
    T_word16 resolution ;
    DebugRoutine("ObjTypesGetResolution") ;

    resolution = G_realFrameResolution ;

    DebugCheck(resolution < 4) ;
    DebugEnd() ;

    return resolution ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IObjTypeLock
 *-------------------------------------------------------------------------*/
/**
 *  IObjTypeLock goes through all the pictures in an object type and locks
 *  all the pictures into memory.
 *
 *  @param p_type -- Pointer to object type to lock
 *  @param typeNumber -- Object index type
 *
 *<!-----------------------------------------------------------------------*/
static T_void IObjTypeLock(T_objectType *p_type, T_word16 typeNumber)
{
    T_byte8 resName[80] ;
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectFrame *p_frameLookup ;
    T_objectFrame *p_frameList ;
    T_objectPic *p_pic ;
    T_objectPic *p_picList ;
    T_objectPic *p_picListLookup ;
    T_objectPic *p_picLookup ;
    T_word16 frameNum ;
    T_word16 stanceNum ;
    T_word16 picNum ;
    T_word16 i ;
    T_sword16 number ;
//    T_word16 picLookup ;
    T_word16 frameLookup ;
    E_Boolean flip ;

    DebugRoutine("IObjTypeLock") ;

    G_frameResolution = G_realFrameResolution ;
    if ((G_somewhatLow == TRUE) && (G_frameResolution == 0))  {
        if ((typeNumber/100)==5)
           G_frameResolution = 1 ;
    }

    /* All objects will have a radius of at least 5 */
    if (p_type->radius < 5)
        p_type->radius = 5 ;
//printf("\nLocking type %d  %p  %d\n", typeNumber, p_type, G_frameResolution) ;
/*
printf("\nLocking type %d  %p\n", typeNumber, p_type) ;
printf("numStances = %d\n", p_type->numStances) ;
printf("lockCount = %d\n", p_type->lockCount) ;
printf("radius = %d\n", p_type->radius) ;
printf("attributes = %d\n", p_type->attributes) ;
fflush(stdout) ;
*/
    /* Go through each stance in the object type. */
    for (stanceNum = 0, p_stance = p_type->stances;
         stanceNum < p_type->numStances;
         stanceNum++, p_stance++)  {
/*
printf("Stance %d of %d\n", stanceNum, p_type->numStances) ;
printf(" numFrames = %d\n", p_stance->numFrames) ;
printf(" speed = %d\n", p_stance->speed) ;
printf(" type = %d\n", p_stance->type) ;
printf(" nextStance = %d\n", p_stance->nextStance) ;
printf(" offset = %d\n", p_stance->offsetFrameList) ;
fflush(stdout) ;
*/
        /* Find the appropriate frame list for this stance. */
        p_frameList = p_frame = (T_objectFrame *)
                      (&(((T_byte8 *)p_type)[p_stance->offsetFrameList])) ;

        for (frameNum=0;
             frameNum<p_stance->numFrames;
             frameNum++)  {
             frameLookup =
                 ((frameNum >> G_frameResolution)
                 << G_frameResolution) ;

             /* Make sure we always get the last frame */
             if ((G_frameResolution > 0) && (frameNum==7))
                 frameLookup = 7 ;

             p_frameLookup = p_frameList+frameLookup ;
             p_frame = p_frameList + frameNum ;

//printf("frame: %d\n", frameNum) ;  fflush(stdout);
             /* Find the appropriate picture list. */
             p_picList = (T_objectPic *)
                             (&(((T_byte8 *)p_type)[p_frame->offsetPicList])) ;
             p_picListLookup = (T_objectPic *)
                             (&(((T_byte8 *)p_type)[p_frameLookup->offsetPicList])) ;

             /* Only 1, 4, or 8 angles per frame. */
             DebugCheck((p_frameLookup->numAngles == 1) ||
                        (p_frameLookup->numAngles == 8) ||
                        (p_frameLookup->numAngles == 4)) ;

             /* Map one frame onto the other. */
             p_frame->numAngles = p_frameLookup->numAngles ;

             for (picNum=0, p_pic = p_picList, p_picLookup = p_picListLookup;
                  picNum < p_frame->numAngles;
                  picNum++, p_pic++, p_picLookup++)  {
                 i = picNum ;

                 /* If the resolution is less than perfect, */
                 /* flip the images for off angles. */
                 flip = FALSE ;
                 if (G_frameResolution > 0)  {
                     if (p_frame->numAngles == 4)  {
                         if (i==1)  {
                             flip = TRUE ;
                             i = 3 ;
                         }
                     } else if (p_frame->numAngles == 8)  {
                         if ((i >= 1) && (i <= 3))  {
                             flip = TRUE ;
                             i = 8-i ;
                         }
                     }
                 }

                 if (G_frameResolution == 3) {
                     i = 0 ;
                     flip = FALSE ;
                 }

                 p_picLookup = &p_picListLookup[i] ;
//printf("%p = %p[%d]\n", p_picLookup, p_picListLookup, i) ; fflush(stdout) ;
                 number = p_picLookup->number ;
                 if ((flip) && (number >= 0))
                     number = -number ;
                 /* Get the name of the picture. */
                 if (number >= 0)  {
                     sprintf(resName,
                         "OBJS/%05d/%05d",
                         typeNumber,
                         number) ;
                 } else {
                     sprintf(resName,
                         "OBJS/%05d/%05d",
                         typeNumber,
                         -number) ;
                 }
//printf("s:%d f:%d a:%d -- %s (%s) [%d]\n", stanceNum, frameNum, i, resName, (flip)?"FLIP":"NO FLIP", number) ; fflush(stdout);
                 /* Store the number (it may now be different, or flipped */
                 p_pic->number = number ;

                 /* Alright, lock in that picture into the slot. */
                 p_pic->p_pic = PictureLock(resName, &p_pic->resource) ;
                 DebugCheck(p_pic->p_pic != NULL) ;
//printf("p_pic->p_pic = %p\n", p_pic->p_pic) ; fflush(stdout) ;
             }
        }
    }

/* TESTING */
//if ((typeNumber == 530) || (typeNumber == 520) || (typeNumber == 510))  {
//  p_type->script = 2 ;
//  p_type->health = 1500 ;
//}

/* Hardcoded TEMPORARY FIX to make shadow creature's dead body */
/* stop floating around -- this flag will make the object */
/* get destroyed on the last frame of animation (if this is */
/* done correctly. */
#if 1
    if (typeNumber == 1003 /* SHADOW CREATURE */)  {
        p_stance = p_type->stances + STANCE_DIE ;
        p_frameList = (T_objectFrame *)
                      (&(((T_byte8 *)p_type)[p_stance->offsetFrameList])) ;
        for (frameNum = 4; frameNum < p_stance->numFrames; frameNum++)  {
            p_frame = p_frameList + frameNum ;
            p_frame->objectAttributes |= OBJECT_ATTR_MARK_FOR_DESTROY ;
        }
    }
#endif
//puts("End IObjLock") ; fflush(stdout) ;
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IObjTypeUnlock
 *-------------------------------------------------------------------------*/
/**
 *  IObjTypeUnlock goes through all the pictures in an object type and
 *  unlocks them from memory.
 *
 *  @param p_type -- Pointer to object type to lock
 *
 *<!-----------------------------------------------------------------------*/
static T_void IObjTypeUnlock(T_objectType *p_type)
{
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectPic *p_pic ;
    T_word16 frameNum ;
    T_word16 stanceNum ;
    T_word16 picNum ;

    DebugRoutine("IObjTypeUnlock") ;

    /* Go through each stance in the object type. */
    for (stanceNum = 0, p_stance = p_type->stances;
         stanceNum < p_type->numStances;
         stanceNum++, p_stance++)  {
        /* Find the appropriate frame list for this stance. */
        p_frame = (T_objectFrame *)
                      (&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;

        for (frameNum=0;
             frameNum<p_stance->numFrames;
             frameNum++, p_frame++)  {
             /* Find the appropriate picture list. */
             p_pic = (T_objectPic *)
                             (&((T_byte8 *)p_type)[p_frame->offsetPicList]) ;

             for (picNum=0 ;
                  picNum < p_frame->numAngles;
                  picNum++, p_pic++)  {
                 /* Alright, unlock that picture. */
                 PictureUnlock(p_pic->resource) ;
                 PictureUnfind(p_pic->resource) ;

                 /* Clear the slots in case it is locked again. */
                 p_pic->resource = RESOURCE_BAD ;
                 p_pic->p_pic = NULL ;
             }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeSetStance
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeSetStance changes the stance of the given object type instance
 *  to new declare stance.  The frame of the stance is set to zero.
 *
 *  @param objTypeInst -- object type instance to set stance of
 *  @param stance -- Stance to be
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypeSetStance(T_objTypeInstance objTypeInst, T_word16 stance)
{
    T_objTypeInstanceStruct *p_objType ;

    DebugRoutine("ObjTypeSetStance") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Make sure this is a valid stance. */
//    DebugCheck(stance < p_objType->p_objectType->numStances) ;

/* !!! Perhaps due to bad communications, I believe that we have */
/* to protect the system from bad stances.  Unfortunately, to make */
/* sure it always runs correctly, we will ignore any bad stances */
/* that might be applied here. */
    if (stance < p_objType->p_objectType->numStances)  {
        /* First see if we are not already in that stance. */
        if (p_objType->stanceNumber != stance)  {
            /* Change the stance. */
            p_objType->stanceNumber = stance ;

            /* Must start the animation over. */
            p_objType->frameNumber = 0 ;

            /* Set when the next frame is to occur. */
            p_objType->nextAnimationTime = SyncTimeGet() +
                p_objType->p_objectType->stances[p_objType->stanceNumber].speed ;
        }

        /** Create sounds/change attributes/whatever. **/
        IObjTypeUpdateFrameChanges (p_objType);
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetStance
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetStance gets the current stance of the object.
 *
 *  @param objTypeInst -- object type instance to set stance of
 *
 *  @return object stance
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetStance(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_word16 stance ;

    DebugRoutine("ObjTypeGetStance") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the stance. */
    stance = p_objType->stanceNumber ;

    /* Make sure this is a valid stance. */
    DebugCheck(stance < p_objType->p_objectType->numStances) ;

    DebugEnd() ;

    return stance ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetScript
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetScript gets the script associated with the given object
 *  type instance.
 *
 *  @param objTypeInst -- object type instance to get script of
 *
 *  @return object script
 *
 *<!-----------------------------------------------------------------------*/
T_word32 ObjTypeGetScript(T_objTypeInstance objTypeInst)
{
    DebugCheck((T_objTypeInstanceStruct *)objTypeInst != NULL) ;
    DebugCheck(((T_objTypeInstanceStruct *)objTypeInst)->p_objectType != NULL) ;

    return ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->script ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeSetScript
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetScript sets the script associated for  the given object
 *  type instance.
 *
 *  @param objTypeInst -- object type instance to set script of
 *  @param script -- object script
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypeSetScript(T_objTypeInstance objTypeInst, T_word32 script)
{
    DebugCheck((T_objTypeInstanceStruct *)objTypeInst != NULL) ;
    DebugCheck(((T_objTypeInstanceStruct *)objTypeInst)->p_objectType != NULL) ;

    ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->script = script ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetHealth
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetHealth retreives the health value for the object type.
 *
 *  @param objTypeInst -- object type instance to set script of
 *
 *  @return health value
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetHealth (T_objTypeInstance objTypeInst)
{
    return ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->health;
}


/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetWeight
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetHealth retreives the weight value for the object type.
 *
 *  @param objTypeInst -- object type instance to set script of
 *
 *  @return health value
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetWeight (T_objTypeInstance objTypeInst)
{
    return ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->weight;
}



/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetValue
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetHealth retreives the weight value for the object type.
 *
 *  @param objTypeInst -- object type instance to set script of
 *
 *  @return health value
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetValue (T_objTypeInstance objTypeInst)
{
    return ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->value;
}



/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetMoveFlags
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetMoveFlags retreives the movement flags associated with the
 *  given type instance.
 *
 *  @param objTypeInst -- object type instance to set script of
 *
 *  @return health value
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetMoveFlags (T_objTypeInstance objTypeInst)
{
    return ((T_objTypeInstanceStruct *)objTypeInst)->p_objectType->objMoveAttr;
}

#ifndef NDEBUG
/*-------------------------------------------------------------------------*
 * Routine:  ObjTypePrint
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypePrint is a utility routine to print out the instance
 *  for a type of object.
 *
 *  @param fp -- output device
 *  @param objTypeInst -- instance to dispaly
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypePrint(FILE *fp, T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectPic *p_pic ;
    T_word16 frameNum ;
    T_word16 stanceNum ;
    T_word16 picNum ;

    DebugRoutine("ObjTypePrint");

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the pointer to the type. */
    p_type = p_objType->p_objectType ;

//fprintf(fp, ">>\n") ;
//fwrite(p_type, 1000, 1, fp) ;
//fprintf(fp, "<<\n") ;
    fprintf(fp, "ObjTypeInst: %p\n", objTypeInst) ;
    fprintf(fp, "  nextAnimationTime: %d\n", p_objType->nextAnimationTime) ;
    fprintf(fp, "  objTypeNum:        %d\n", p_objType->objTypeNum) ;
    fprintf(fp, "  resource:          %p\n", p_objType->resource) ;
    fprintf(fp, "  stanceNumber:      %d\n", p_objType->stanceNumber) ;
    fprintf(fp, "  frameNumber:       %d\n", p_objType->frameNumber) ;
    fprintf(fp, "  stateData:         %d\n", p_objType->stateData) ;

    fprintf(fp, "  p_objectType: %p\n", p_type) ;
    fprintf(fp, "    numStances: %d\n", p_type->numStances) ;
    fprintf(fp, "    lockCount:  %d\n", p_type->lockCount) ;
    fprintf(fp, "    radius:     %d\n", p_type->radius) ;
    fprintf(fp, "    attributes: %d\n", p_type->attributes) ;
    fprintf(fp, "    weight:     %d\n", p_type->weight) ;
    fprintf(fp, "    value:      %d\n", p_type->value) ;
    fprintf(fp, "    script:     %ld\n", p_type->script) ;
    fprintf(fp, "    health:     %d\n", p_type->health) ;
    fprintf(fp, "    objMoveAttr:%d\n\n", p_type->objMoveAttr) ;

    /* Go through each stance in the object type. */
    for (stanceNum = 0, p_stance = p_type->stances;
         stanceNum < p_type->numStances;
         stanceNum++, p_stance++)  {
        fprintf(fp, "      stance %d\n", stanceNum) ;
        fprintf(fp, "        speed:   %d\n", p_stance->speed) ;
        fprintf(fp, "        type:    %d\n", p_stance->type) ;
        fprintf(fp, "        nextStan:%d\n", p_stance->nextStance) ;
        fprintf(fp, "        offsetFr:%d\n", p_stance->offsetFrameList) ;

        /* Find the appropriate frame list for this stance. */
        p_frame = (T_objectFrame *)
                      (&(((T_byte8 *)p_type)[p_stance->offsetFrameList])) ;

        for (frameNum=0;
             frameNum<p_stance->numFrames;
             frameNum++, p_frame++)  {
             fprintf(fp, "        frame %d\n", frameNum) ;
             fprintf(fp, "          numAng:    %d\n", p_frame->numAngles) ;
             fprintf(fp, "          offsetPic: %d\n", p_frame->offsetPicList) ;
             fprintf(fp, "          soundNum:  %d\n", p_frame->soundNum) ;
             fprintf(fp, "          soundRad:  %d\n", p_frame->soundRadius) ;
             fprintf(fp, "          objAttr:   %d\n", p_frame->objectAttributes) ;
             /* Find the appropriate picture list. */
             p_pic = (T_objectPic *)
                             (&(((T_byte8 *)p_type)[p_frame->offsetPicList])) ;

             /* Only 1, 4, or 8 angles per frame. */
             DebugCheck((p_frame->numAngles == 1) ||
                        (p_frame->numAngles == 8) ||
                        (p_frame->numAngles == 4)) ;

             for (picNum=0 ;
                  picNum < p_frame->numAngles;
                  picNum++, p_pic++)  {
                 fprintf(fp, "          angle %d\n", picNum) ;
                 fprintf(fp, "            p_pic: %p\n", p_pic) ;
                 fprintf(fp, "              picture: %p\n", p_pic->p_pic) ;
                 fprintf(fp, "              number:  %d\n", p_pic->number) ;
                 fprintf(fp, "              resource:%p\n", p_pic->resource) ;
             }
        }
    }

    DebugEnd() ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeGetHeight
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeGetHeight looks at the first frame of the first stance and
 *  determines the picture's height.
 *
 *  @param objTypeInst -- instance to find height of
 *
 *  @return height
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjTypeGetHeight(T_objTypeInstance objTypeInst)
{
    T_void *p_picture ;
    T_word16 height ;
    T_orientation orient ;

    DebugRoutine("ObjTypeGetHeight") ;
    DebugCheck(objTypeInst != OBJECT_TYPE_INSTANCE_BAD) ;

    p_picture = ObjTypeGetPicture(objTypeInst, 0, &orient) ;
    height = PictureGetHeight(p_picture) ;

    DebugEnd() ;

    return height ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IObjTypeFreePieces
 *-------------------------------------------------------------------------*/
/**
 *  IObjTypeFreePieces goes through all the pictures in a piecewise
 *  object type and frees them from memory.
 *
 *  @param p_type -- Pointer to object type to lock
 *
 *<!-----------------------------------------------------------------------*/
static T_void IObjTypeFreePieces(T_objectType *p_type)
{
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectPic *p_pic ;
    T_word16 frameNum ;
    T_word16 stanceNum ;
    T_word16 picNum ;

    DebugRoutine("IObjTypeFreePieces") ;
//printf("ObjTypeFreePieces %p\n", p_type) ;
    /* Go through each stance in the object type. */
    for (stanceNum = 0, p_stance = p_type->stances;
         stanceNum < p_type->numStances;
         stanceNum++, p_stance++)  {
        /* Find the appropriate frame list for this stance. */
        p_frame = (T_objectFrame *)
                      (&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;

        for (frameNum=0;
             frameNum<p_stance->numFrames;
             frameNum++, p_frame++)  {
             /* Find the appropriate picture list. */
             p_pic = (T_objectPic *)
                             (&((T_byte8 *)p_type)[p_frame->offsetPicList]) ;

             for (picNum=0 ;
                  picNum < p_frame->numAngles;
                  picNum++, p_pic++)  {
                 /* Alright, free the picture (if it is an orignal). */
                 if ((p_pic->number == OBJECT_TYPE_PICTURE_IS_ORIGINAL) ||
                     (p_pic->number == -OBJECT_TYPE_PICTURE_IS_ORIGINAL))
                     if (p_pic->p_pic)
                         MemFree(((T_byte8 *)p_pic->p_pic)-4) ;

                 /* Clear the slots in case it is used again. */
                 p_pic->resource = RESOURCE_BAD ;
                 p_pic->p_pic = NULL ;
                 p_pic->number = OBJECT_TYPE_PICTURE_IS_COPY ;
             }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjTypeRebuildPiecewise
 *-------------------------------------------------------------------------*/
/**
 *  ObjTypeRebuildPiecewise goes through all the frames in an object type
 *  and builds the appropriate picture out of parts as based on this
 *  instance of the object.
 *
 *  @param objTypeInst -- Pointer to object type to lock
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjTypeRebuildPieces(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectFrame *p_frameLookup ;
    T_objectFrame *p_frameList ;
    T_objectPic *p_pic ;
    T_objectPic *p_picList ;
    T_objectPic *p_picListLookup ;
    T_objectPic *p_picLookup ;
    T_word16 frameNum ;
    T_word16 stanceNum ;
    T_word16 picNum ;
    T_word16 i ;
    T_word16 frameLookup ;
    E_Boolean flip ;

    DebugRoutine("ObjTypeRebuildPiecewise") ;
    DebugCheck(objTypeInst != NULL) ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;
    p_type = p_objType->p_objectType ;

    /* First, free any pieces that were originally there. */
    IObjTypeFreePieces(p_type) ;

    /* Go through each stance in the object type. */
    for (stanceNum = 0, p_stance = p_type->stances;
         stanceNum < p_type->numStances;
         stanceNum++, p_stance++)  {
        /* Find the appropriate frame list for this stance. */
        p_frameList = p_frame = (T_objectFrame *)
                      (&(((T_byte8 *)p_type)[p_stance->offsetFrameList])) ;

        for (frameNum=0;
             frameNum<p_stance->numFrames;
             frameNum++)  {
             frameLookup =
                 ((frameNum >> G_frameResolution)
                 << G_frameResolution) ;
             p_frameLookup = p_frameList+frameLookup ;
             p_frame = p_frameList + frameNum ;

             /* Find the appropriate picture list. */
             p_picList = (T_objectPic *)
                             (&(((T_byte8 *)p_type)[p_frame->offsetPicList])) ;
             p_picListLookup = (T_objectPic *)
                             (&(((T_byte8 *)p_type)[p_frameLookup->offsetPicList])) ;

             /* Only 1, 4, or 8 angles per frame. */
             DebugCheck((p_frameLookup->numAngles == 1) ||
                        (p_frameLookup->numAngles == 8) ||
                        (p_frameLookup->numAngles == 4)) ;

             /* Map one frame onto the other. */
             p_frame->numAngles = p_frameLookup->numAngles ;

             for (picNum=0, p_pic = p_picList, p_picLookup = p_picListLookup;
                  picNum < p_frame->numAngles;
                  picNum++, p_pic++, p_picLookup++)  {
                 i = picNum ;

                 /* If the resolution is less than perfect, */
                 /* flip the images for off angles. */
                 flip = FALSE ;
                 if (G_frameResolution > 0)  {
                     if (p_frame->numAngles == 4)  {
                         if (i==1)  {
                             flip = TRUE ;
                             i = 3 ;
                         }
                     } else if (p_frame->numAngles == 8)  {
                         if ((i >= 1) && (i <= 3))  {
                             flip = TRUE ;
                             i = 8-i ;
                         }
                     }
                 }
                 p_picLookup = &p_picListLookup[i] ;

                 /* No matter what the picture is, there is no resource. */
                 p_pic->resource = RESOURCE_BAD ;

#if 0
                 /* Is this a copy or an original? */
                 if ((i != picNum) || (frameNum != frameLookup))  {
                     /* It is a copy. */
                     /* Copy over the data. */
                     p_pic->p_pic = p_picLookup->p_pic ;

                     /* Make sure to mark it as a copy. */
                     p_pic->number = OBJECT_TYPE_PICTURE_IS_COPY ;
                 } else {
                     /* It is an original. */
                     /* Build the view. */
                     p_pic->p_pic = IBuildView(
                                        p_objType,
                                        stanceNum,
                                        frameNum,
                                        picNum * (8/(p_frame->numAngles))) ;

                     /* Make sure to mark it as an original. */
                     p_pic->number = OBJECT_TYPE_PICTURE_IS_ORIGINAL ;
                 }
                 /* If this image is flipped, flip it. */
                 if (flip)
                     p_pic->number = -p_pic->number ;
#else
                 p_pic->number = OBJECT_TYPE_PICTURE_NEED_DRAW ;
                 p_pic->p_pic = NULL ;
#endif
             }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IBuildView
 *-------------------------------------------------------------------------*/
/**
 *  IBuildView is the routine that creates one view of a piecewise object.
 *  Just pass in what stance, frame, and angle to build and the picture
 *  is returned (compressed) in a newly allocated memory block.
 *
 *  @param p_objType -- objType to build pic for
 *  @param stance -- Number of stance to draw for
 *  @param frame -- Frame to draw for
 *  @param angle -- Angle of view to draw for
 *
 *  @return Pointer to compressed bitmap created.
 *
 *<!-----------------------------------------------------------------------*/
static T_void *IBuildView(
                   T_objTypeInstanceStruct *p_objType,
                   T_word16 stance,
                   T_word16 frame,
                   T_word16 angle)
{
    /* Table telling what order to place body parts based on */
    /* what angle is being faced. */
    static T_byte8 ordering[8][MAX_BODY_PARTS] = {
        { 0, 4, 5, 1, 3, 2},
        { 3, 5, 4, 0, 1, 2},
        { 3, 5, 4, 0, 1, 2},
        { 3, 5, 4, 0, 1, 2},
        { 2, 3, 4, 5, 0, 1},
        { 2, 4, 5, 0, 1, 3},
        { 2, 4, 5, 0, 1, 3},
        { 2, 4, 5, 0, 1, 3}
    } ;
    T_word16 i ;
    T_word16 part ;
    T_byte8 *p_workArea ;
    T_byte8 partName[80] ;
    T_word16 partNumber ;
    T_resource res ;
    T_byte8 *p_compress ;
    T_byte8 *p_pic ;

    DebugRoutine("IBuildView") ;
    DebugCheck(p_objType != NULL) ;
    DebugCheck(stance < 50) ;
    DebugCheck(frame < 50) ;
    DebugCheck(angle < 8) ;
/* For standing, use first frame of walking. */
if (stance == STANCE_STAND)
  stance = STANCE_WALK ;
/* Use first frame of dying for getting hurt. */
if (stance == STANCE_HURT)
  stance = STANCE_DIE ;
/* Repeat any death scenes. */
if ((stance == STANCE_DIE) && (frame >= 4))
  frame = 3 ;

angle = (7-angle) ;
angle = (2+angle) & 7 ;
    p_workArea = MemAlloc(64*64+1000) ;
//p_workArea = (char *)0xA0000 ;
    DebugCheck(p_workArea != NULL) ;
    memset(p_workArea, 0, 64*64) ;

    for (i=0; i<MAX_BODY_PARTS; i++)  {
        part = ordering[(angle-1)&7][i] ;
        partNumber = ((T_word16 *)((p_objType->p_parts)))[part] ;
        /* Don't do parts that are declared missing. */
        if (partNumber != (T_word16)-1)  { // TODO: Is this comparison correct?
            sprintf(partName, "/Parts/%02d/%05d/%02d%02d%d",
                part,
                partNumber,
                stance,
                frame,
                angle) ;
            p_pic = PictureLock(partName, &res) ;
//            if (PictureExist(partName))  {
            if (p_pic)  {
                DebugCheck(p_pic != NULL) ;

                IOverlayPicture(p_pic, p_workArea) ;

                PictureUnlock(res) ;
                PictureUnfind(res) ;
            } else {
#ifndef NDEBUG
                printf("Locking non-existant part: %s\n", partName) ;
                DebugCheck(FALSE) ;
#endif
            }
        }
    }

    p_compress = ICompressPicture(p_workArea) ;

    MemFree(p_workArea) ;

    DebugEnd() ;

    return p_compress+4 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IOverlayPicture
 *-------------------------------------------------------------------------*/
/**
 *  IOverlayPicture takes a mpress'd picture and overlays it into a
 *  work area that is correctly sized for the mpression.
 *
 *  @param p_picture -- Pointer to mpress'd picture
 *  @param p_workArea -- Number of stance to draw for
 *
 *<!-----------------------------------------------------------------------*/
static T_void IOverlayPicture(T_byte8 *p_picture, T_byte8 *p_workArea)
{
    T_word16 pos ;
    T_byte8 count ;

    DebugRoutine("IOverlayPicture") ;
    DebugCheck(p_picture != NULL) ;
    DebugCheck(p_workArea != NULL) ;

    /* Keep overlaying until we find the end marker. */
    while (*((T_word16 *)p_picture) != 0xFFFF)  {
        /* Get the position withing the 64x64 bitmap. */
        pos = *((T_word16 *)p_picture) ;
        p_picture += 2 ;

        /* Get the count of bytes to copy. */
        count = *(p_picture++) ;

        /* Copy them. */
        memcpy(p_workArea+pos, p_picture, count) ;

        /* Skip over to the next stream. */
        p_picture += count ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICompressPicture
 *-------------------------------------------------------------------------*/
/**
 *  ICompressPicture takes a section of memory and compresses it into
 *  the .CPC format that is already used by the other object pictures.
 *
 *  NOTE: 
 *  Note that the pointer returned by this routine is a pointer to the
 *  start of the compression table, just past the x & y size information.
 *
 *  @param p_picture -- Pointer to picture to compress.
 *
 *  @return Pointer to compressed picture.
 *
 *<!-----------------------------------------------------------------------*/
typedef struct {
    T_word16 offset ;
    T_byte8 start ;
    T_byte8 end ;
} T_compressionEntry ;

static T_byte8 *ICompressPicture(T_byte8 *p_picture)
{
    T_word16 offset ;
    T_word16 len ;
    T_sword16 yy, xx1, xx2 ;
    T_byte8 *p_work ;
    T_byte8 *p_line ;
    const T_word16 x = 64 ;
    const T_word16 y = 64 ;
    T_word16 linesize ;
    T_compressionEntry table[256] ;
    T_word16 totalSize ;
    char *p_final ;

    DebugRoutine("ICompressPicture") ;
    /* Allocate memory to work in. */
    p_work = MemAlloc(x*y+1000) ;
    DebugCheck(p_work != NULL) ;

    offset = 4*(1 + y) ;
    len = 0 ;

    for (yy=0; yy<y; yy++)  {
        p_line = p_picture + yy*x ;

        /* Find the first non transparent pixel. */
        for (xx1=0; xx1<x; xx1++)  {
            if (p_line[xx1] != 0)
                break ;
        }
        if (xx1 != x)  {
            /* Find the last non transparent pixel. */
            for (xx2=x-1; xx2 >= 0; xx2--)
                if (p_line[xx2] != 0)
                    break ;
            linesize = 1+xx2-xx1 ;
        } else {
            xx2 = xx1 = 255 ;
            linesize = 0 ;
        }

        /* Store this table entry. */
        table[yy].offset = offset ;
        table[yy].start = (T_byte8)xx1 ;
        table[yy].end = (T_byte8)xx2 ;

        /* Copy the data (if any) */
        if (linesize != 0)  {
            memcpy(p_work+len, p_line+xx1, linesize) ;

            /* And update the offset. */
            offset += linesize ;
            len += linesize ;
        }
    }

    /* Done compressing into the work space. */
    /* How big is the total? */
    totalSize = 4 + sizeof(T_compressionEntry) * y + len ;

    /* Allocate the block that we will return */
    p_final = MemAlloc(totalSize) ;

    /* Make the correct formatted item. */
    /* Copy over the size. */
    ((T_word16 *)p_final)[0] = x ;
    ((T_word16 *)p_final)[1] = y ;
    p_final += 4 ;

    /* Copy over the table. */
    memcpy(p_final, table, sizeof(T_compressionEntry) * y) ;

    /* Copy over the actual data. */
    memcpy(p_final + sizeof(T_compressionEntry) * y, p_work, len) ;

    /* No longer need that work space any more. */
    MemFree(p_work) ;

    DebugEnd() ;

    /* Return a pointer that is past the x & y values. */
    return p_final-4 ;
}

/* LES: 03/28/96 */
T_word16 ObjTypeGetFrame(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_word16 frame ;

    DebugRoutine("ObjTypeGetFrame") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the stance. */
    frame = p_objType->frameNumber ;

    DebugEnd() ;

    return frame ;
}

/* LES: 06/20/96 */
T_word32 ObjTypeGetNextAnimationTime(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_word32 time ;

    DebugRoutine("ObjTypeGetNextAnimationTime") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the next time. */
    time = p_objType->nextAnimationTime ;

    DebugEnd() ;

    return time ;
}

/* LES: 06/20/96 */
T_void ObjTypeSetNextAnimationTime(
             T_objTypeInstance objTypeInst,
             T_word32 time)
{
    T_objTypeInstanceStruct *p_objType ;

    DebugRoutine("ObjTypeSetNextAnimationTime") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the next time. */
    p_objType->nextAnimationTime = time ;

    DebugEnd() ;
}

/* LES: 06/20/96 */
T_word16 ObjTypeGetAnimData(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_word16 data ;

    DebugRoutine("ObjTypeGetAnimData") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the next time. */
    data = p_objType->stateData ;

    DebugEnd() ;

    return data ;
}

/* LES: 06/20/96 */
T_void ObjTypeSetAnimData(T_objTypeInstance objTypeInst, T_word16 data)
{
    T_objTypeInstanceStruct *p_objType ;

    DebugRoutine("ObjTypeSetAnimData") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get the next time. */
    p_objType->stateData = data ;

    DebugEnd() ;
}

/* LES: 09/06/96 */
E_Boolean ObjTypeIsActive(T_objTypeInstance objTypeInst)
{
    return ((T_objTypeInstanceStruct *)objTypeInst)->isActive ;
}

/* LES: 09/12/96 */
T_void *ObjTypeGetFrontFirstPicture(T_objTypeInstance objTypeInst)
{
    T_objTypeInstanceStruct *p_objType ;
    T_objectType *p_type ;
    T_objectStance *p_stance ;
    T_objectFrame *p_frame ;
    T_objectPic *p_pic ;
    T_void *p_picture ;

    DebugRoutine("ObjectTypeGetFrontFirstPicture") ;

    /* Get the correct type of pointer. */
    p_objType = (T_objTypeInstanceStruct *)objTypeInst ;

    /* Get a quick pointer to the type. */
    p_type = p_objType->p_objectType ;

    /* Get the stance pointer for the first stance. */
    p_stance = &p_type->stances[0] ;

    /* Get the frame pointer. */
    p_frame = (T_objectFrame *)(&((T_byte8 *)p_type)[p_stance->offsetFrameList]) ;

    /* Get a pointer to the picture list. */
    p_pic = (T_objectPic *)
                 (&((T_byte8 *)p_type)
                     [p_frame[0].offsetPicList]) ;

    /* Get the pointer to the artwork for the first angle. */
    p_picture = p_pic->p_pic ;

    DebugEnd() ;

    return p_picture ;
}

T_void ObjTypeDeclareSomewhatLowOnMemory(T_void)
{
    G_somewhatLow = TRUE ;
}

E_Boolean ObjTypeIsLowPiecewiseRes(T_void)
{
    return G_somewhatLow ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  OBJTYPE.C
 *-------------------------------------------------------------------------*/
