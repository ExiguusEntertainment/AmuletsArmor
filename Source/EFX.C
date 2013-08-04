/*-------------------------------------------------------------------------*
 * File:  EFX.C
 *-------------------------------------------------------------------------*/
/**
 * The EFX system is used to create blood splatters and chips.  It
 * should probably be called the "Particle" system.
 *
 * @addtogroup EFX
 * @brief Special Efx in the 3D World
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "AREASND.H"
#include "EFX.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "SOUNDS.H"
#include "TICKER.H"
#include "VIEWFILE.H"

#define MAX_EFX_OBJECTS 50

static T_word16 G_numEfxObjectsInWorld=0;

/* double link list of all efx in progress */
static T_doubleLinkList G_effectsInProgress=DOUBLE_LINK_LIST_BAD;

/* internal routine prototypes */
static T_void EfxCreateBloodSplat (T_efxID myID, E_Boolean translucent);
static T_void EfxUpdateBloodSplat (T_efxID myID);
static T_void EfxAddObjectToList  (T_doubleLinkList list, T_3dObject *object);
static T_void EfxCreateGenericShrapnel (T_word16 objectType,
                                        T_efxID myID,
                                        E_Boolean addToList,
                                        E_Boolean ignoreGravity);
static T_void EfxCreateGenericExplosion(T_word16 objectType,
                                        T_efxID myID,
                                        E_Boolean addToList,
                                        T_sword16 zPixelOffset);

static T_void EfxUpdateGeneric (T_efxID myID);

/* routine creates a linked list for tracking all effects in progress */
T_void EfxInit (T_void)
{
    DebugRoutine ("EfxInit");

    if (G_effectsInProgress != DOUBLE_LINK_LIST_BAD)
    {
        EfxFinish();
    }

    G_effectsInProgress=DoubleLinkListCreate();

    DebugEnd();
}

/* routine cleans up all efx in progress */
T_void EfxFinish (T_void)
{
    T_doubleLinkListElement element ;
    DebugRoutine ("EfxFinish");

    /* clean up effects linked list */
    if (G_effectsInProgress != DOUBLE_LINK_LIST_BAD)
    {
        element = DoubleLinkListGetFirst(G_effectsInProgress) ;
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            EfxDestroy(element) ;
            element = DoubleLinkListGetFirst(G_effectsInProgress) ;
        }

        DoubleLinkListDestroy (G_effectsInProgress);
        G_effectsInProgress=DOUBLE_LINK_LIST_BAD;
    }

    G_numEfxObjectsInWorld=0;

    DebugEnd();
}


/* routine creates an effect of type specified, at location x,y,z */
/* with the number of objects specified.  Duration is the number of ticks */
/* until all objects for this effect are automatically destroyed (via the */
/* scheduler), if 0 then the objects are 'permanent' */

T_efxID EfxCreate (E_efxType type,        // type of effect
                   T_word32  Xorigin,     // x,y,z location of effect
                   T_word32  Yorigin,
                   T_word32  Zorigin,
                   T_word16  numberOf,    // number of objects to create
                   E_Boolean transparent, // should objects be transparent?
                   T_word32  extraData)   // extra data for some efx
{
    T_doubleLinkListElement myElement=NULL;
    T_efxStruct *p_efx;

    DebugRoutine ("EfxCreate");
    DebugCheck (type < EFX_UNKNOWN);

    if (type < EFX_UNKNOWN)
    {
        /* create a new efx structure */
        p_efx=(T_efxStruct *)MemAlloc(sizeof(T_efxStruct));
        DebugCheck (p_efx != NULL);

        if (p_efx != NULL)
        {
            /* initialize a double linked list to hold all newly created */
            /* object pointers */
            p_efx->objectList=DOUBLE_LINK_LIST_BAD;
            p_efx->objectList=DoubleLinkListCreate();
            DebugCheck (p_efx->objectList != DOUBLE_LINK_LIST_BAD);
            if (p_efx->objectList != DOUBLE_LINK_LIST_BAD)
            {
                /* init extra data */
                p_efx->reserved=TickerGet();
                p_efx->updateCallback=NULL;
                p_efx->Xorigin=Xorigin;
                p_efx->Yorigin=Yorigin;
                p_efx->Zorigin=Zorigin;
                p_efx->numberOf=numberOf;
                p_efx->duration=0;

                /* add this efx to global list */
                p_efx->myID=DoubleLinkListAddElementAtEnd(G_effectsInProgress,p_efx);

                /* call the effect set up routine */
                switch (type)
                {
                    case EFX_BLOOD_SHRAPNEL:
                    EfxCreateBloodSplat (p_efx->myID,transparent);
                    break;

                    case EFX_WALL_HIT:
                    EfxCreateGenericShrapnel (OBJECT_TYPE_WALL_HIT,
                                              p_efx->myID,
                                              FALSE,
                                              TRUE);
                    break;

                    case EFX_TELEPORT:
                    p_efx->duration=100;
                    EfxCreateGenericExplosion (OBJECT_TYPE_TELEPORT,
                                               p_efx->myID,
                                               TRUE,
                                               0);
                    AreaSoundCreate(
                        p_efx->Xorigin>>16,
                        p_efx->Yorigin>>16,
                        500,
                        255,
                        AREA_SOUND_TYPE_ONCE,
                        0,
                        AREA_SOUND_BAD,
                        NULL,
                        0,
                        SOUND_TELEPORT) ;

                    break;

                    case EFX_POWER_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_POWER_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               0);
                    break;

                    case EFX_POISON_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_POISON_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               0);
                    break;

                    case EFX_ACID_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_ACID_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               0);
                    break;

                    case EFX_ELECTRIC_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_ELECTRIC_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               -20);
                    break;

                    case EFX_MAGIC_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_MAGIC_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               -20);
                    break;

                    case EFX_FIRE_EXPLOSION:
                    EfxCreateGenericExplosion (OBJECT_TYPE_FIRE_EXPLOSION,
                                               p_efx->myID,
                                               FALSE,
                                               -25);

                   EfxCreate (EFX_FLAME_SHRAPNEL,
                              p_efx->Xorigin,
                              p_efx->Yorigin,
                              p_efx->Zorigin,
                              3,
                              FALSE,
                              0);

                    break;


                    case EFX_FIRE_SHRAPNEL:
                    p_efx->duration=150;
                    EfxCreateGenericShrapnel(OBJECT_TYPE_FIRE_SHRAPNEL,
                                             p_efx->myID,
                                             TRUE,
                                             FALSE);
                    break;

                    case EFX_FLAME_SHRAPNEL:
                    p_efx->duration=80;
                    EfxCreateGenericShrapnel(OBJECT_TYPE_FLAME_SHRAPNEL,
                                             p_efx->myID,
                                             TRUE,
                                             FALSE);
                    break;

                    case EFX_BONE_SHRAPNEL:
                    p_efx->duration=80;
                    EfxCreateGenericShrapnel(OBJECT_TYPE_BONE_SHRAPNEL,
                                             p_efx->myID,
                                             TRUE,
                                             FALSE);
                    break;

                    case EFX_ACID_SHRAPNEL:
                    EfxCreateGenericShrapnel(OBJECT_TYPE_ACID_SHRAPNEL,
                                             p_efx->myID,
                                             FALSE,
                                             TRUE);
                    break;

                    case EFX_POISON_SHRAPNEL:
                    EfxCreateGenericShrapnel(OBJECT_TYPE_POISON_SHRAPNEL,
                                             p_efx->myID,
                                             FALSE,
                                             TRUE);
                    break;

                    case EFX_ELECTRIC_SHRAPNEL:
                    p_efx->duration=60;
                    EfxCreateGenericShrapnel(OBJECT_TYPE_ELECTRIC_SHRAPNEL,
                                             p_efx->myID,
                                             TRUE,
                                             FALSE);
                    break;

                    case EFX_MAGIC_SHRAPNEL:
                    p_efx->duration=60;
                    EfxCreateGenericShrapnel(OBJECT_TYPE_MANA_SHRAPNEL,
                                             p_efx->myID,
                                             TRUE,
                                             FALSE);
                    break;

                    default:
                    break;
                }
            }
        }
    }

    DebugEnd();
    return (p_efx->myID);
}


/* routine destroys all objects associated with efxID */
T_void EfxDestroy (T_efxID efxElement)
{
    T_efxStruct *p_efx;
    T_3dObject *p_obj;
    T_doubleLinkListElement element,nextelement;

    DebugRoutine ("EfxDestroy");
    DebugCheck (efxElement != DOUBLE_LINK_LIST_ELEMENT_BAD);

    /* get efx structure */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData(efxElement);
    DebugCheck (p_efx->objectList != DOUBLE_LINK_LIST_BAD);

    /* traverse linked list and get all object pointers */
    element=DoubleLinkListGetFirst (p_efx->objectList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextelement=DoubleLinkListElementGetNext(element);
        p_obj=(T_3dObject *)DoubleLinkListElementGetData (element);
        DebugCheck (p_obj != NULL);
        if (p_obj != NULL)
        {
            DebugCheck (G_numEfxObjectsInWorld != 0);
            /* remove object from world */
//            ObjectRemove (p_obj);
            /* destroy object */
//            ObjectDestroy (p_obj);
            ObjectMarkForDestroy(p_obj) ;
            /* decrement counter */
            G_numEfxObjectsInWorld--;
        }

        DoubleLinkListRemoveElement(element);
        element=nextelement;
    }

    /* remove this element from global list */
    DoubleLinkListRemoveElement (efxElement);

    /* delete list of objects */
    DoubleLinkListDestroy (p_efx->objectList);
    p_efx->objectList = DOUBLE_LINK_LIST_BAD ;

    /* delete this efx structure */
    MemFree (p_efx);

    efxElement=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugEnd();
}


/* routine updates all efx in progress that need it */
/* called once per frame by update.c */
T_void EfxUpdate (T_void)
{
    T_doubleLinkListElement element,nextElement;
    T_efxStruct *p_efx;

    DebugRoutine ("EfxUpdate");

    /* traverse through all effects in progress and */
    /* update those that need it */

    if (G_effectsInProgress != DOUBLE_LINK_LIST_BAD)
    {
        element=DoubleLinkListGetFirst (G_effectsInProgress);

        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            nextElement=DoubleLinkListElementGetNext(element);
            p_efx=(T_efxStruct *)DoubleLinkListElementGetData(element);
            if (p_efx->updateCallback != NULL)
            {
                /* effect has an update routine assigned, call it */
                p_efx->updateCallback(element);
            }
            element=nextElement;
        }
    }

    DebugEnd();
}


/* sets up objects for a 'blood splat' effect */
static T_void EfxCreateBloodSplat (T_efxID myID, E_Boolean translucent)
{
    T_efxStruct *p_efx;
    T_3dObject *p_splooch;
    T_word16 i,j;
    T_sword32 genXVel, genYVel, genZVel;
    T_byte8 numSplatsInGroup;
    T_byte8 whichSplat;

    DebugRoutine ("EfxCreateBloodSplat");
    DebugCheck (myID != NULL);

    /* get pointer to our efx struct */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData (myID);

    /* set update callback routine */
    p_efx->updateCallback = EfxUpdateBloodSplat;

    /* set up all blood splat objects */
    for (i=0;i<p_efx->numberOf;i++)
    {
        /* create 'sub-groups' of random 1-4 splats */
        numSplatsInGroup=rand()%4;
        genXVel=(rand()*30) - (rand()*30);
        genYVel=(rand()*30) - (rand()*30);
        genZVel=(rand()%15000)-(rand()%5000);

        for (j=0;j<numSplatsInGroup;j++)
        {
            i++;

            p_splooch=ObjectCreateFake();
            whichSplat=rand()%3;
            if (whichSplat==0)
            {
                ObjectSetType (p_splooch,OBJECT_TYPE_BLOOD_SHRAPNEL_1);
            }
            else if (whichSplat==1)
            {
                ObjectSetType(p_splooch,OBJECT_TYPE_BLOOD_SHRAPNEL_2);
            }
            else
            {
                ObjectSetType (p_splooch,OBJECT_TYPE_BLOOD_SHRAPNEL_3);
            }

            ObjectMakeFullyPassable (p_splooch);
            ObjectSetX(p_splooch,p_efx->Xorigin);
            ObjectSetY(p_splooch,p_efx->Yorigin);
            ObjectSetZ(p_splooch,p_efx->Zorigin);

            ObjectSetUpSectors (p_splooch);
            ObjectSetStance (p_splooch,0);
            ObjectMakePassable (p_splooch);
            if (translucent==TRUE) ObjectMakeTranslucent(p_splooch);
            ObjectAdd(p_splooch);
            ObjectSetXVel (p_splooch,genXVel+(rand()*10));
            ObjectSetYVel (p_splooch,genYVel+(rand()*10));
            ObjectSetZVel (p_splooch,genZVel);
            /* reset Z */
            ObjectSetZ(p_splooch,p_efx->Zorigin+(rand()*10));
            ObjectForceUpdate (p_splooch);

            /* add our object to our linked list */

            EfxAddObjectToList(p_efx->objectList,p_splooch);
        }
    }

    DebugEnd();
}


/* routine looks at all blood splats and checks to see if they need to */
/* turn into puddles */

static T_void EfxUpdateBloodSplat (T_efxID myID)
{
    T_efxStruct *p_efx;
    T_3dObject *p_obj;
    T_word32 stance;
    T_word32 objType;
    T_doubleLinkListElement element;

    DebugRoutine ("EfxUpdateBloodSplat");
    DebugCheck (myID != DOUBLE_LINK_LIST_ELEMENT_BAD);

    /* get pointer to our struct */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData (myID);

    /* iterate through the list of drops and */
    /* update each one if needed */
    element=DoubleLinkListGetFirst (p_efx->objectList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_obj=DoubleLinkListElementGetData (element);
        objType=ObjectGetType(p_obj);

        /* check for different types */
        if (objType==OBJECT_TYPE_BLOOD_SHRAPNEL_1)
        {
            /* turn slash into drop if necessary */
            stance=ObjectGetStance(p_obj);
            if (stance==0)
            {
              /* wait 10 ticks before shifting to second stance */
                if (TickerGet()-p_efx->reserved > 10)
                {
                    ObjectSetStance (p_obj,1);
                }
            }
            else if (stance==1)
            {
                if (ObjectGetZVel(p_obj)<100 &&
                    ObjectGetZVel(p_obj)>-100)
                {
                    ObjectSetStance (p_obj,2);
                }
            }
        }
        else
        {
            stance=((TickerGet()-p_efx->reserved)/10);
            if (stance > 5) stance=5;
            ObjectSetStance(p_obj,stance);
        }
        element=DoubleLinkListElementGetNext(element);
    }

    DebugEnd();
}


/* routine will add object to a effects 'objects in world' list */
/* and count the total number of objects, removing the oldest groups */
/* if MAX_EFX_OBJS is exceeded */

static T_void EfxAddObjectToList (T_doubleLinkList list, T_3dObject *object)
{
    T_doubleLinkListElement element;
    T_efxStruct* efxStruct;

    DebugRoutine ("EfxAddObjectToList");
    DebugCheck (list != DOUBLE_LINK_LIST_BAD);
    DebugCheck (object != NULL);
    /* add this object to our link list */

    DoubleLinkListAddElementAtEnd (list,object);

    /* increment our numobjects counter */
    G_numEfxObjectsInWorld++;

    /* make sure we don't have too many */
    while (G_numEfxObjectsInWorld > MAX_EFX_OBJECTS)
    {
        /* destroy oldest (first) group in main list */
        element=DoubleLinkListGetFirst(G_effectsInProgress);
        DebugCheck (element != DOUBLE_LINK_LIST_ELEMENT_BAD);
        efxStruct=(T_efxStruct *)DoubleLinkListElementGetData(element);
        EfxDestroy(efxStruct->myID);
    }


    DebugEnd();
}


/* creates a generic 'shrapnel' object with no update callback */
/* add to list is TRUE if the object does not remove itself */
/* timedKill is 0 if the object removes itself or is removed based */
/* on number of of effects objects in the world, otherwise, it's the */
/* number of ticks the object should stay in the world */
static T_void EfxCreateGenericShrapnel (T_word16 objectType,
                                        T_efxID myID,
                                        E_Boolean addToList,
                                        E_Boolean ignoreGravity)
{
    T_efxStruct *p_efx;
    T_3dObject *p_splooch;
    T_word16 i;
    T_sword32 genXVel, genYVel, genZVel;

    DebugRoutine ("EfxCreateGenericShrapnel");
    DebugCheck (myID != NULL);

    /* get pointer to our efx struct */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData (myID);

    /* set update callback routine */
    p_efx->updateCallback = EfxUpdateGeneric;

    /* set up all splat objects */
    for (i=0;i<p_efx->numberOf;i++)
    {
        genXVel=(rand()*30) - (rand()*30);
        genYVel=(rand()*30) - (rand()*30);
        genZVel=(rand());

        p_splooch=ObjectCreateFake();
        ObjectSetType (p_splooch,objectType);
        ObjectMakeFullyPassable (p_splooch);

        ObjectSetX(p_splooch,p_efx->Xorigin);
        ObjectSetY(p_splooch,p_efx->Yorigin);
        ObjectSetZ(p_splooch,p_efx->Zorigin);

        ObjectSetUpSectors (p_splooch);
        ObjectSetStance (p_splooch,0);
        ObjectMakePassable (p_splooch);

        if (ignoreGravity)
        {
            ObjectSetMoveFlags(p_splooch,OBJMOVE_FLAG_IGNORE_GRAVITY);
        }

        ObjectAddAttributes(p_splooch,OBJECT_ATTR_NO_SHADING);
        ObjectAdd(p_splooch);
        ObjectSetXVel (p_splooch,genXVel);
        ObjectSetYVel (p_splooch,genYVel);
        ObjectSetZVel (p_splooch,genZVel);
        ObjectSetZ(p_splooch,p_efx->Zorigin);

        ObjectForceUpdate (p_splooch);

        /* add our object to our linked list */

        if (addToList==TRUE)
        {
            EfxAddObjectToList(p_efx->objectList,p_splooch);
        }
    }

    DebugEnd();
}


static T_void EfxCreateGenericExplosion(T_word16 objectType,
                                        T_efxID myID,
                                        E_Boolean addToList,
                                        T_sword16 zPixelOffset)
{
    T_efxStruct *p_efx;
    T_3dObject *p_splooch;
    T_word16 i;

    DebugRoutine ("EfxCreateGenericExplosion");
    DebugCheck (myID != NULL);

    /* get pointer to our efx struct */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData (myID);

    /* set update callback routine */
    p_efx->updateCallback = EfxUpdateGeneric;

    /* set up all splat objects */
    for (i=0;i<p_efx->numberOf;i++)
    {
        p_splooch=ObjectCreateFake();
        ObjectSetType (p_splooch,objectType);
        ObjectMakeFullyPassable (p_splooch);

        ObjectSetX(p_splooch,p_efx->Xorigin);
        ObjectSetY(p_splooch,p_efx->Yorigin);
        ObjectSetZ(p_splooch,p_efx->Zorigin - (ObjectGetHeight(p_splooch)/2)
                                            + (zPixelOffset<<16));

        ObjectSetUpSectors (p_splooch);
        ObjectSetStance (p_splooch,0);
        ObjectMakePassable (p_splooch);
        ObjectAddAttributes(p_splooch,OBJECT_ATTR_NO_SHADING);

        ObjectAdd(p_splooch);
        ObjectSetXVel (p_splooch,0);
        ObjectSetYVel (p_splooch,0);
        ObjectSetZVel (p_splooch,0);
        ObjectSetZ(p_splooch,p_efx->Zorigin - (ObjectGetHeight(p_splooch)/2)
                                            + (zPixelOffset<<16));

        ObjectForceUpdate (p_splooch);

        /* add our object to our linked list */

        if (addToList==TRUE)
        {
            EfxAddObjectToList(p_efx->objectList,p_splooch);
        }
    }

    DebugEnd();
}


/* routine checks timer on efx (if any) and removes it if needed */
static T_void EfxUpdateGeneric (T_efxID myID)
{
    T_efxStruct *p_efx;
    T_word32 time_elapsed;

    DebugRoutine ("EfxUpdateGeneric");

    DebugCheck (myID != DOUBLE_LINK_LIST_ELEMENT_BAD);

    /* get pointer to our struct */
    p_efx=(T_efxStruct *)DoubleLinkListElementGetData (myID);

    if (p_efx->duration > 0)
    {
        /* get time elapsed */
        time_elapsed = TickerGet()-p_efx->reserved;

        /* subtract time elapsed from duration and destroy this effect */
        /* if out of time */

        if (p_efx->duration < time_elapsed)
        {
            /* remove this effect */
            EfxDestroy(myID);
        }
    }

    DebugEnd();
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  EFX.C
 *-------------------------------------------------------------------------*/
