/*-------------------------------------------------------------------------*
 * File:  OBJGEN.C
 *-------------------------------------------------------------------------*/
/**
 * The object generator is a special place on a map that generates one
 * or more items or creatures when activated.  It uses the tell tale
 * teleporter sound as it generates objects.
 *
 * @addtogroup OBJGEN
 * @brief Object Generator
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "EFX.H"
#include "MAP.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "SYNCMEM.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "VIEWFILE.H"

#define GENERATOR_TAG           "OGn"
#define GENERATOR_DEAD_TAG      "DoG"

typedef struct _T_objectGenerator {
    T_sword16 x, y ;
    T_word16 angle ;
    T_word16 objectType ;
    T_word16 timeBetween ;
    T_word16 randomTimeBetween ;
    T_word16 maxObjectsOfThisType ;
    T_word16 maxLikeObjects ;
    T_word16 countDown ;
    T_sword16 maxGenerate ;   /* Value of -1 means unending. */
    E_Boolean isActive ;
    T_word16 id ;
    T_word16 specialEffect ;

    struct _T_objectGenerator *p_prev ;
    struct _T_objectGenerator *p_next ;
#ifndef NDEBUG
    T_byte8 tag[4] ;
#endif
} T_objectGenerator ;

static T_word16 G_numGenerators = 0 ;
static E_Boolean G_loaded = FALSE ;
static T_objectGenerator *G_generatorList = NULL ;
static T_word32 G_lastTimeUpdated = 0 ;
static T_word16 G_lastID = 0 ;

typedef struct {
    T_word16 numGenerators ;
    E_Boolean loaded ;
    T_objectGenerator *p_generatorList ;
    T_word32 lastTimeUpdated ;
    T_word16 lastID ;
} T_objectGeneratorHandleStruct ;

/* Internal prototypes: */
static T_objectGenerator *IAddGenerator(
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

static T_void IStartUpGenerators() ;

static T_void IStartUpGenerator(T_objectGenerator *p_generator) ;

static T_void IDestroyGenerator(T_objectGenerator *p_generator) ;

static T_void IGeneratorGenerate(T_objectGenerator *p_generator) ;

static T_objectGenerator *IFindGenerator(T_word16 genID) ;

/*-------------------------------------------------------------------------*
 * Routine:  ObjectGeneratorLoad
 *-------------------------------------------------------------------------*/
/**
 *  ObjectGeneratorLoad loads up and starts the object generators for the
 *  given level.
 *
 *  @param mapNumber -- Number of the map to load obj gens for.
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjectGeneratorLoad(T_word32 mapNumber)
{
    FILE *fp ;
    T_byte8 filename[20] ;
    T_byte8 line[80] ;
    T_sword16 x, y ;
    T_word16 timeBetween ;
    T_word16 randomTimeBetween ;
    T_word16 maxObjects ;
    T_word16 objectType ;
    T_word16 angle ;
    T_word16 maxLikeObjects ;
    T_word16 isActive ;
    T_sword16 maxGenerate ;

    DebugRoutine("ObjectGeneratorLoad") ;
    DebugCheck(G_loaded == FALSE) ;

    /* Form the name of the generator file. */
    sprintf(filename, "L%ld.GEN", mapNumber) ;

    /* Open up the file. */
    fp = fopen(filename, "r") ;

    /* If the file cannot be opened, do nothing (except a warning). */
    if (fp)  {
        fgets(line, 80, fp) ;
        while (!feof(fp))  {
            if (line[0] == 'G')  {
                sscanf(line+1, "%u%d%d%u%u%u%u%u%u%d",
                    &objectType,
                    &x,
                    &y,
                    &angle,
                    &timeBetween,
                    &randomTimeBetween,
                    &maxObjects,
                    &maxLikeObjects,
                    &isActive,
                    &maxGenerate) ;
                IAddGenerator(
                    objectType,
                    x,
                    y,
                    angle,
                    timeBetween,
                    randomTimeBetween,
                    maxObjects,
                    maxLikeObjects,
                    (isActive)?TRUE:FALSE,
                    maxGenerate,
                    0) ;
            }
            fgets(line, 80, fp) ;
        }
        fclose(fp) ;

        IStartUpGenerators() ;
    } else {
#ifndef NDEBUG
        printf("Warning! Could not find file %s\n", filename) ;
#endif
    }

    /* Make all updates start based on now. */
DebugCheck(SyncTimeGet() == 1) ;
    G_lastTimeUpdated = SyncTimeGet() ;

    /* Note that we are loaded. */
    G_loaded = TRUE ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjectGeneratorUnload
 *-------------------------------------------------------------------------*/
/**
 *  ObjectGeneratorUnload gets rid of all generators for this object.
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjectGeneratorUnload(T_void)
{
    DebugRoutine("ObjectGeneratorUnload") ;
    DebugCheck(G_loaded == TRUE) ;

    while (G_generatorList)
        IDestroyGenerator(G_generatorList) ;

    /* We are no longer loaded. */
    G_loaded = FALSE ;

    /* Reset the generator ids */
    G_lastID = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IAddGenerator
 *-------------------------------------------------------------------------*/
/**
 *  IAddGenerator creates a new generator and adds to the list.
 *
 *  @param objectType -- Type of object generated
 *  @param y -- Where to place the new object
 *  @param angle -- Angle to face when created
 *  @param timeBetween -- Seconds between creation
 *  @param randomTimeBetween -- Addition random time between creation
 *  @param maxObjects -- Maximum number of objects in the map
 *  @param maxLikeObjects -- Maximum number of objects with the
 *      same basic type.
 *  @param specialEffect -- Special effect to occur when gen
 *      (0 = none)
 *
 *<!-----------------------------------------------------------------------*/
static T_objectGenerator *IAddGenerator(
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
                              T_word16 specialEffect)
{
    T_objectGenerator *p_generator = NULL ;
    DebugRoutine("IAddGenerator") ;

    /* Allocate memory for the generator. */
    p_generator = MemAlloc(sizeof(T_objectGenerator)) ;
    DebugCheck(p_generator != NULL) ;

    if (p_generator)  {
        /* Fill in the generator list. */
        p_generator->objectType = objectType ;
        p_generator->x = x ;
        p_generator->y = y ;
        p_generator->angle = angle ;
        p_generator->timeBetween = timeBetween ;
        p_generator->randomTimeBetween = randomTimeBetween ;
        p_generator->maxObjectsOfThisType = maxObjects ;
        p_generator->maxLikeObjects = maxLikeObjects ;
        p_generator->isActive = isActive ;
        p_generator->maxGenerate = maxGenerate ;
        p_generator->specialEffect = specialEffect ;
        p_generator->id = G_lastID++ ;     // Tag the generator

#ifndef NDEBUG
        /* Tag the generator. */
        strcpy(p_generator->tag, GENERATOR_TAG) ;
#endif

        /* Put the generator into the list. */
        p_generator->p_prev = NULL ;
        p_generator->p_next = G_generatorList ;
        G_generatorList = p_generator ;

        /* One more generator. */
        G_numGenerators++ ;
    }

    DebugEnd() ;

    return p_generator ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IStartUpGenerators
 *-------------------------------------------------------------------------*/
/**
 *  IStartUpGenerators goes through all generatores and starts them up.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IStartUpGenerators()
{
    T_objectGenerator *p_generator ;

    DebugRoutine("IStartUpGenerators") ;

    p_generator = G_generatorList ;
    while (p_generator)  {
        IStartUpGenerator(p_generator) ;
        p_generator = p_generator->p_next ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IStartUpGenerator
 *-------------------------------------------------------------------------*/
/**
 *  IStartUpGenerator reinitializes a generator to start creating.
 *
 *  @param p_generator -- Generator to start up
 *
 *<!-----------------------------------------------------------------------*/
static T_void IStartUpGenerator(T_objectGenerator *p_generator)
{
    DebugRoutine("IStartUpGenerator") ;
    DebugCheck(p_generator != NULL) ;
    DebugCheck(strcmp(p_generator->tag, GENERATOR_TAG) == 0) ;

    /* Determine the number of seconds until a generation will occur. */
    p_generator->countDown = p_generator->timeBetween ;
    if (p_generator->randomTimeBetween)
        p_generator->countDown +=
            (RandomNumber() % p_generator->randomTimeBetween) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IStartUpGenerator
 *-------------------------------------------------------------------------*/
/**
 *  IStartUpGenerator reinitializes a generator to start creating.
 *
 *  @param p_generator -- Generator to start up
 *
 *<!-----------------------------------------------------------------------*/
static T_void IDestroyGenerator(T_objectGenerator *p_generator)
{
    DebugRoutine("IDestroyGenerator") ;
    DebugCheck(p_generator != NULL) ;
    DebugCheck(strcmp(p_generator->tag, GENERATOR_TAG) == 0) ;

    /* Tag the item for deletion. */
#ifndef NDEBUG
    strcpy(p_generator->tag, GENERATOR_DEAD_TAG) ;
#endif

    /* Remove the generator from the list. */
    if (p_generator->p_prev)
        p_generator->p_prev->p_next = p_generator->p_next ;
    else
        G_generatorList = p_generator->p_next ;

    if (p_generator->p_next)
        p_generator->p_next->p_prev = p_generator->p_prev ;

    /* Remove the generator from memory. */
    MemFree(p_generator) ;

    /* One less generator. */
    G_numGenerators-- ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjectGeneratorUpdate
 *-------------------------------------------------------------------------*/
/**
 *  ObjectGeneratorUpdate checks to see if a second has gone by.  If one
 *  has, all the generators are updated.  Those generators that have had
 *  enough time go by will attempt to generate.  The process then
 *  starts over.
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjectGeneratorUpdate(T_void)
{
    T_word16 seconds ;
    T_word32 time ;
    T_objectGenerator *p_generator ;

    TICKER_TIME_ROUTINE_PREPARE() ;

    TICKER_TIME_ROUTINE_START() ;
    DebugRoutine("ObjectGeneratorUpdate") ;

    time = SyncTimeGet() ;
    seconds = (time - G_lastTimeUpdated) / ((T_word32) TICKS_PER_SECOND) ;
    SyncMemAdd("Gen timing: %ld %ld %d\n", time, G_lastTimeUpdated, seconds) ;
    if (seconds)  {
        /* Go through the list and update their counters. */
        p_generator = G_generatorList ;
        while (p_generator)  {
            /* Only update if it is active. */
            if (p_generator->isActive)  {
                /* Has this generator timed out? */
                if (p_generator->countDown <= seconds)  {
                    /* Time out! */
                    IGeneratorGenerate(p_generator) ;

                    /* Start over with this generator. */
                    IStartUpGenerator(p_generator) ;
                } else {
                    /* Still going. */
                    p_generator->countDown -= seconds ;
                }
            }

            /* Move through the list. */
            p_generator = p_generator->p_next ;
        }

        /* Note when was the last time we updated. */
        G_lastTimeUpdated += (seconds * ((T_word32) TICKS_PER_SECOND)) ;
    }

    DebugEnd() ;
    TICKER_TIME_ROUTINE_END(stdout, "ObjectGeneratorUpdate", 500) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IGeneratorGenerate
 *-------------------------------------------------------------------------*/
/**
 *  IGeneratorGenerate is called when it is time for a generator to
 *  generate a new object.  This routine does all the work for that object.
 *
 *  @param p_generator -- Generator to generate object
 *
 *<!-----------------------------------------------------------------------*/
static T_void IGeneratorGenerate(T_objectGenerator *p_generator)
{
    T_3dObject *p_obj ;

    DebugRoutine("IGeneratorGenerate") ;
    DebugCheck(p_generator != NULL) ;
    DebugCheck(strcmp(p_generator->tag, GENERATOR_TAG) == 0) ;

    SyncMemAdd("Gen stats %d %d %d\n", p_generator->maxGenerate, ObjectsCountType(p_generator->objectType), ObjectsCountBasicType(p_generator->objectType)) ;
    /* Can this generator create more? */
    if (p_generator->maxGenerate != 0)  {
        /* Are there already too many objects with same type? */
        if (ObjectsCountType(p_generator->objectType) <
            p_generator->maxObjectsOfThisType)  {
            /* Are there already too many basic objects? */
            if (ObjectsCountBasicType(p_generator->objectType) <
                p_generator->maxLikeObjects)  {
                p_obj = ObjectCreate() ;
                DebugCheck(p_obj != NULL) ;
                if (p_obj)  {
                    /* Initialize the object. */
                    ObjectDeclareStatic(
                        p_obj,
                        p_generator->x,
                        p_generator->y) ;
                    ObjectSetType(p_obj, p_generator->objectType) ;
                    ObjectSetAngle(p_obj, p_generator->angle) ;
                    ObjectSetUpSectors(p_obj) ;
                    ObjectSetZ16(
                        p_obj,
                        MapGetFloorHeight(ObjectGetAreaSector(p_obj))) ;

                    SyncMemAdd("Generator attempt %d %d\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj), 0) ;
                    /* See if there is anything in the way for creation. */
                    if (ObjectCheckCollideAny(
                            p_obj,
                            p_generator->x,
                            p_generator->y,
                            (T_sword16)(ObjectGetZ16(p_obj))))  {
//printf("Collision at %d %d %d\n", p_generator->x, p_generator->y, ObjectGetZ16(p_obj)) ;
                        /* Something is in the way, don't create. */
                        /* Destroy the object. */
                        ObjectDestroy(p_obj) ;
                    } else {
                        /* Nothing is in the way. */
                        /* Place it in our world. */
                        SyncMemAdd("Generator created %d %d\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj), 0) ;
//                        printf("Adding Monster\n") ;
                        ObjectAdd(p_obj) ;

                        /* Mark off one more as created. */
                        if (p_generator->maxGenerate != -1)  {
                            p_generator->maxGenerate-- ;

                            /* If no longer to generate, turn it off. */
                            if (p_generator->maxGenerate == 0)
                                p_generator->isActive = FALSE ;
                        }
                        EfxCreate(
                            EFX_TELEPORT,
                            ObjectGetX(p_obj),
                            ObjectGetY(p_obj),
                            (ObjectGetZ16(p_obj) +
                                (ObjectGetHeight(p_obj)>>1) - 64) << 16,
                            1,
                            FALSE,
                            0) ;
                    }
                }
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************
 *  Description:
 *
 *     The following routine is used to get a list of object generators of
 *  the given type.  Each generator is stored in a form of an x, y, and angle.
 *
 ****************************************************************************
 *  History:
 *  LES: 05/02/96 Created
 ****************************************************************************/
T_word16 ObjectGeneratorGetList(
             T_word16 objectType,
             T_objectGeneratorPosition *p_list,
             T_word16 maxList)
{
    T_objectGenerator *p_generator ;
    T_word16 count = 0 ;

    DebugRoutine("ObjectGeneratorGetList") ;

    /* Go through the list of generators and find the matches. */
    p_generator = G_generatorList ;
    while (p_generator)  {
        if (p_generator->objectType == objectType)  {
            p_list[count].x = p_generator->x ;
            p_list[count].y = p_generator->y ;
            p_list[count].angle = p_generator->angle ;

            /* That's one more.  Are we at the end? */
            count++ ;
            if (count == maxList)
                /* Yes, then stop looping. */
                break ;
        }

        p_generator = p_generator->p_next ;
    }

    DebugEnd() ;

    return count ;
}

/****************************************************************************
 *  Description:
 *
 *     IFindGenerator returns a pointer to the matching objgen for the given
 *  id.  Otherwise, NULL is returned.
 *
 ****************************************************************************
 *  History:
 *  LES: 05/14/96 Created
 ****************************************************************************/
static T_objectGenerator *IFindGenerator(T_word16 genID)
{
    T_objectGenerator *p_gen ;

    DebugRoutine("IFindGenerator") ;

    /* Search for a matching generator based on its id. */
    p_gen = G_generatorList ;
    while (G_generatorList != NULL)  {
        if (p_gen->id == genID)
            break ;
        p_gen = p_gen->p_next ;
    }

    DebugEnd() ;

    return p_gen ;
}

/****************************************************************************
 *  Description:
 *
 *     ObjectGeneratorActivate turns on a previously off generator.  If the
 *  generator is empty (maxed out), it will never reactivate.
 *
 ****************************************************************************
 *  History:
 *  LES: 05/14/96 Created
 ****************************************************************************/
T_void ObjectGeneratorActivate(T_word16 genID)
{
    T_objectGenerator *p_gen ;

    DebugRoutine("ObjectGeneratorActivate") ;

    /* First, find the generator in question. */
    p_gen = IFindGenerator(genID) ;
    if (p_gen)  {
        /* Only activate if the generator is not out. */
        if (p_gen->maxGenerate != 0)
            p_gen->isActive = TRUE ;
    } else {
        /* In the debug version, crash because there */
        /* is no generator of that type here. */
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;
}

/****************************************************************************
 *  Description:
 *
 *    ObjectGeneratorDeactivate turns off a generator by its id
 *
 ****************************************************************************
 *  History:
 *  LES: 05/14/96 Created
 ****************************************************************************/
T_void ObjectGeneratorDeactivate(T_word16 genID)
{
    T_objectGenerator *p_gen ;

    DebugRoutine("ObjectGeneratorDeactivate") ;

    /* First, find the generator in question. */
    p_gen = IFindGenerator(genID) ;
    if (p_gen)  {
        /* Deactive it. */
        p_gen->isActive = FALSE ;
    } else {
        /* In the debug version, crash because there */
        /* is no generator of that type here. */
        DebugCheck(FALSE) ;
    }

    DebugEnd() ;
}

/* LES: 05/27/96 Created */
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
           T_word16 specialEffect)
{
    DebugRoutine("GeneratorAddGenerator") ;

    IAddGenerator(
        objectType,
        x,
        y,
        angle,
        timeBetween,
        randomTimeBetween,
        maxObjects,
        maxLikeObjects,
        isActive,
        maxGenerate,
        specialEffect) ;

    DebugEnd() ;
}

/****************************************************************************
 *  Description:
 *
 *    ObjectGeneratorCreateHandle creates a context handle
 *
 ****************************************************************************
 *  History:
 *  LES: 06/07/96 Created
 ****************************************************************************/
T_objectGeneratorHandle ObjectGeneratorCreateHandle(T_void)
{
    T_objectGeneratorHandleStruct *p_handle ;

    DebugRoutine("ObjectGeneratorCreateHandle") ;

    p_handle = MemAlloc(sizeof(T_objectGeneratorHandleStruct)) ;
    DebugCheck(p_handle != NULL) ;
    if (p_handle)  {
        p_handle->numGenerators = 0 ;
        p_handle->loaded = FALSE ;
        p_handle->p_generatorList = NULL ;
        p_handle->lastTimeUpdated = 0 ;
        p_handle->lastID = 0 ;
    }

    DebugEnd() ;

    return (T_objectGeneratorHandle)p_handle ;
}

/****************************************************************************
 *  Description:
 *
 *    ObjectGeneratorDestroyHandle destroys a context handle
 *
 ****************************************************************************
 *  History:
 *  LES: 06/07/96 Created
 ****************************************************************************/
T_void ObjectGeneratorDestroyHandle(T_objectGeneratorHandle handle)
{
    DebugRoutine("ObjectGeneratorDestroyHandle") ;

    MemFree(handle) ;

    DebugEnd() ;
}

/****************************************************************************
 *  Description:
 *
 *    ObjectGeneratorSetHandle changes the context to the given handle
 *
 ****************************************************************************
 *  History:
 *  LES: 06/07/96 Created
 ****************************************************************************/
T_void ObjectGeneratorSetHandle(T_objectGeneratorHandle handle)
{
    T_objectGeneratorHandleStruct *p_handle ;

    DebugRoutine("ObjectGeneratorSetHandle") ;

    DebugCheck(handle != OBJECT_GENERATOR_HANDLE_BAD) ;
    p_handle = (T_objectGeneratorHandleStruct *)handle ;
    if (p_handle)  {
        G_numGenerators = p_handle->numGenerators ;
        G_loaded = p_handle->loaded ;
        G_generatorList = p_handle->p_generatorList ;
        G_lastTimeUpdated = p_handle->lastTimeUpdated ;
        G_lastID = p_handle->lastID ;
    }

    DebugEnd() ;
}

/****************************************************************************
 *  Description:
 *
 *    ObjectGeneratorGetHandle retrieves the current context into the handle
 *
 ****************************************************************************
 *  History:
 *  LES: 06/07/96 Created
 ****************************************************************************/
T_void ObjectGeneratorGetHandle(T_objectGeneratorHandle handle)
{
    T_objectGeneratorHandleStruct *p_handle ;

    DebugRoutine("ObjectGeneratorGetHandle") ;

    DebugCheck(handle != OBJECT_GENERATOR_HANDLE_BAD) ;
    p_handle = (T_objectGeneratorHandleStruct *)handle ;
    if (p_handle)  {
        p_handle->numGenerators = G_numGenerators ;
        p_handle->loaded = G_loaded ;
        p_handle->p_generatorList = G_generatorList ;
        p_handle->lastTimeUpdated = G_lastTimeUpdated ;
        p_handle->lastID = G_lastID ;
    }

    DebugEnd() ;
}


/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  OBJGEN.C
 *-------------------------------------------------------------------------*/
