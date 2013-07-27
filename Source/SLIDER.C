/****************************************************************************/
/*    FILE:  SLIDER.C                                                       */
/****************************************************************************/
#include "ACTIVITY.H"
#include "MEMORY.H"
#include "SCHEDULE.H"
#include "SLIDER.H"
#include "SYNCTIME.H"

typedef struct T_sliderTag {
    T_byte8 tag[4] ;                 /* Tag to identify this block of data.*/
    T_word32 sliderId ;              /* Unique ID to identify which slider. */
    T_sword32 value ;                /* Current value. */
    T_sword32 start ;
    T_sword32 end ;                  /* Goal value. */
    T_sword32 delta ;                /* Step rate. */
    T_word16 finalActivity ;         /* Activity to run when done. */
    T_sliderEventHandler handler ;   /* Who to call when executed. */
    T_word32 lastTime ;              /* Time last updated. */
    E_Boolean f_cancel ;             /* Flag to request a cancel. */
    struct T_sliderTag *next ;       /* Links to items in the list. */
    struct T_sliderTag *prev ;
} T_slider;

/* So we can cancel some of these sliders, we have a slider linked list */
/* (actually doubly linked) and this pointer points to the first element. */
static T_slider *G_firstSlider ;

/* The following is a flag to identify whether or not the slider module */
/* has been initialized. */
static E_Boolean G_sliderInit = FALSE ;


typedef struct {
    T_slider *p_firstSlider ;
    T_word16 sliderCount ;
    E_Boolean sliderInit ;
} T_sliderHandleStruct ;

/* Internal prototypes: */
static T_void ISliderCallback(T_word32 data) ;

static T_void ISliderDestroy(T_slider *p_slider) ;

static T_slider *ISliderFind(T_word32 sliderId) ;



/****************************************************************************/
/*  Routine:  SliderInitialize                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderInitialize is called to start up the slider system.  No other   */
/*  routines can be called until this first step is taken.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    All other slider routines will bomb unless this is called.  Also,     */
/*  you must call SliderFinish before calling this routine again.           */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SliderInitialize(T_void)
{
    DebugRoutine("SliderInitialize") ;
    DebugCheck(G_sliderInit == FALSE) ;

    /* Make sure we have nothing in our list of sliders. */
    G_firstSlider = NULL ;

    G_sliderInit = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SliderFinish                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderFinish is to be called when the system is done and no longer    */
/*  needs sliders.  All remaining sliders are discarded from memory.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that no calls to ScheduleUpdateEvents occur again after     */
/*  SliderFinish is executed.  This insures that no remaining scheduled     */
/*  items with callbacks to the slider module are called.                   */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SliderFinish(T_void)
{
    DebugRoutine("SliderFinish") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* Destroy all the sliders in the current slider list. */
    while (G_firstSlider)
        ISliderDestroy(G_firstSlider) ;

    /* Note that we are done. */
    G_sliderInit = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SliderStart                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderStart is called to add another slider event to the slider list. */
/*  A slider is an event that occurs over several points in time and will   */
/*  range over a start and end value over a given period of time.  Each     */
/*  time the slider changes the value, a callback routine is called to      */
/*  handle the new value.  In addition, a optional script activity can      */
/*  be called once the sliding value has reached its goal value.            */
/*    In addition, a unique ID is passed to the routine.  Should there      */
/*  exist a slider already in existance, the slider will redirect its       */
/*  direction and go to the new value.                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Unique ID                              */
/*                                                                          */
/*    T_sword32 start, end        -- Starting and ending values             */
/*                                                                          */
/*    T_word16 time               -- Time to take to change                 */
/*                                                                          */
/*    T_sliderEventHandle handler -- Callback routine for each value change */
/*                                                                          */
/*    T_word16 finalActivity      -- Activity to call when done             */
/*                                   0xFFFF = none.                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*    LES  07/30/96  Changed sliders so that start at given start even      */
/*                   though they already exist and are being redirected.    */
/*                                                                          */
/****************************************************************************/

T_void SliderStart(
           T_word32 sliderId,
           T_sword32 start,
           T_sword32 end,
           T_sword32 time,
           T_sliderEventHandler handler,
           T_word16 finalActivity)
{
    T_slider *p_slider ;

    DebugRoutine("SliderStart") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* First, check to see if there is a slider with this id. */
    p_slider = ISliderFind(sliderId) ;

    /* Did we find one? */
    if (p_slider == NULL)  {
        /* No, we didn't find one. */
        /* Create a structure for the slider. */
        p_slider = MemAlloc(sizeof(T_slider)) ;
        DebugCheck(p_slider != NULL) ;

        /* Fill out the slider structure.  Be sure to also calculate */
        /* the change of structure information. */
        strcpy(p_slider->tag, "Sli") ;
        p_slider->sliderId = sliderId ;
        p_slider->value = start ;
        p_slider->start = start ;
        p_slider->end = end ;
        p_slider->delta = (end-start)/time ;
        p_slider->finalActivity = finalActivity ;
        p_slider->handler = handler ;
        p_slider->lastTime = SyncTimeGet() ;
        p_slider->f_cancel = FALSE ;

        /* Attach the slider to the double linked list. */
        p_slider->next = G_firstSlider ;
        p_slider->prev = NULL ;
        if (G_firstSlider != NULL)
            G_firstSlider->prev = p_slider ;
        G_firstSlider = p_slider ;

        /* Add the slider to the schedule. */
        ScheduleAddEvent(
            p_slider->lastTime+1,
            ISliderCallback,
//            (T_word32)p_slider) ;
            sliderId) ;
    } else {
        /* Yes, there is a slider with this id.  Since it is always */
        /* tied to a scheduled item, we need to update the data. */
        /* Calculate a new delta time based on the given start */
        /* and end data. */
        p_slider->start = start ;
        p_slider->value = start ;
        p_slider->delta = (end-start)/time ;
        p_slider->end = end ;

        /* Change any other appropriate information. */
        p_slider->finalActivity = finalActivity ;
        p_slider->handler = handler ;

        /* The Schedule module will now do the rest. */
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SliderCancel                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderCancel stops a slider that is in progress.  The slider is       */
/*  noted for canceling and on the next call to ScheduleUpdateEvents,       */
/*  it will be removed/deleted.                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Don't call this routine unless you KNOWN a slider is still active.    */
/*  A bomb will occur if there is no slider.                                */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Slider to cancel.                      */
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
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SliderCancel(T_word32 sliderId)
{
    T_slider *p_slider ;

    DebugRoutine("SliderCancel") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* First, check to see if there is a slider with this id. */
    p_slider = ISliderFind(sliderId) ;
    DebugCheck(p_slider != NULL) ;
    DebugCheck(strcmp(p_slider->tag, "Sli")==0) ;

    /* Let's note that the slider is to be canceled. */
    p_slider->f_cancel = TRUE ;

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  SliderReverse                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderReverse changes the direction that a slider is going to back    */
/*  its original value.  If a different value is wanted, just use           */
/*  SliderCancel and SliderStart than this routine.  In addition, an option */
/*  new final activity can be supplied.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Don't call this routine unless you KNOWN a slider is still active.    */
/*  A bomb will occur if there is no slider.                                */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Slider to cancel.                      */
/*                                                                          */
/*    T_word16 newActivity        -- New activity to run, or 0x8000 for no  */
/*                                   change.  Also, a value of -1 (0xFFFF)  */
/*                                   sets up a non-activity.                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISliderFind                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SliderReverse(T_word32 sliderId, T_word16 newActivity)
{
    T_slider *p_slider ;
    T_sword32 temp ;

    DebugRoutine("SliderReverse") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* First, check to see if there is a slider with this id. */
    p_slider = ISliderFind(sliderId) ;
    DebugCheck(p_slider != NULL) ;
    DebugCheck(strcmp(p_slider->tag, "Sli")==0) ;

    /* Let's note that the slider is to be canceled. */
    if (p_slider != NULL)  {
        /* Swap start and end values. */
        temp = p_slider->end ;
        p_slider->end = p_slider->start ;
        p_slider->start = temp ;

        /* Reverse the direction of the slide. */
        p_slider->delta = -p_slider->delta ;

        /* See if there is a request for a new slider activity. */
        if (newActivity != 0x8000)
            /* Yes, new activity requested. */
            p_slider->finalActivity = newActivity ;
    }

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  SliderExist                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SliderExist checks to see if the given slider exists.                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId           -- Slider to cancel.                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISliderFind                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean SliderExist(T_word32 sliderId)
{
    T_slider *p_slider ;
    E_Boolean status ;

    DebugRoutine("SliderExist") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* Check to see if there is a slider with this id. */
    p_slider = ISliderFind(sliderId) ;
    if (p_slider == NULL)
        status = FALSE ;
    else
        status = TRUE ;

    DebugEnd();

    return status ;
}

/****************************************************************************/
/*  Routine:  ISliderCallback                    * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISliderCallback is the routine called when a slider updates each      */
/*  time slice.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 data               -- Casted pointer to the slider struct.   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    slider handler                                                        */
/*    ScheduleAddEvent                                                      */
/*    ISliderDestroy                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ISliderCallback(T_word32 data)
{
    T_slider *p_slider ;
    E_Boolean f_done ;
    T_word32 time ;
    T_sword32 deltaTime ;
    T_sliderResponse status ;
    T_word16 finalAct ;
    T_sword32 prevValue ;

    DebugRoutine("ISliderCallback") ;
    DebugCheck(G_sliderInit == TRUE) ;

//    p_slider = (T_slider *)data ;
    p_slider = ISliderFind(data /* sliderID */) ;
    if (p_slider)  {
        DebugCheck(strcmp(p_slider->tag, "Sli")==0) ;

        /* First check to see if we are canceling this slider. */
        if (p_slider->f_cancel == TRUE)  {
            /* We ARE canceling. */
            /* Just destroy this item and do nothing else. */
            ISliderDestroy(p_slider) ;
        } else {
            /* We are NOT canceling. */
            /* Get the time and time since the last update. */
            time = SyncTimeGet() ;
            deltaTime = time - p_slider->lastTime ;
            p_slider->lastTime = time ;

            /* Update the sliding value. */
            prevValue = p_slider->value ;
            p_slider->value += p_slider->delta * deltaTime ;

            /* Now we need to know if we hit the end (or went past). */
            /* Which direction are we going? */
            if (p_slider->delta >= 0)  {
                /* Positive. */
                if (p_slider->value > p_slider->end)
                    p_slider->value = p_slider->end ;
            } else {
                /* Negative. */
                if (p_slider->value < p_slider->end)
                    p_slider->value = p_slider->end ;
            }

            /* Are we done? */
            if (p_slider->end == p_slider->value)
                f_done = TRUE ;
            else
                f_done = FALSE ;

            /* Update the callback no matter what. */
            status = p_slider->handler(
                         p_slider->sliderId,
                         p_slider->value,
                         f_done) ;
            DebugCheck(status < SLIDER_RESPONSE_UNKNOWN) ;

            if (status == SLIDER_RESPONSE_PAUSE)  {
                p_slider->value = prevValue ;

                /* Can't possibly be done. */
                f_done = FALSE ;
            }


            /* Make sure we are not canceled. */
            if (p_slider->f_cancel == FALSE)  {
                /* If we are done, we need to try to run a new activity. */
                if (f_done == TRUE)  {
                    /* Get the final activity for this slider. */
                    finalAct = p_slider->finalActivity ;

                    /* If we are done, we need to delete this slider. */
                    ISliderDestroy(p_slider) ;

                    /* If there was a final activity, run it. */
                    if (finalAct != 0xFFFF)
                        ActivitiesRun(finalAct) ;
                } else {
                    /* If we are not done, we need to schedule for the next */
                    /* time.  Use the same slider structure as the data. */
                    /* Also, make sure the handler thinks we need to keep */
                    /* going. */
                    if (status != SLIDER_RESPONSE_STOP)
                        ScheduleAddEvent(
                            time+2,
                            ISliderCallback,
                            data) ;
                }
            } else {
                ISliderDestroy(p_slider) ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ISliderDestroy                     * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISliderDestroy is called to remove a slider from the slider list      */
/*  and memory.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    It is assumed that the slider is no longer being pointed to by any    */
/*  other elements and also is not on the scheduler.                        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_slider *p_slider          -- slider to destroy                      */
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
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ISliderDestroy(T_slider *p_slider)
{
    DebugRoutine("ISliderDestroy") ;
    DebugCheck(G_sliderInit == TRUE) ;
    DebugCheck(G_firstSlider != NULL) ;
    DebugCheck(strcmp(p_slider->tag, "Sli")==0) ;

    /* Remove the slider from the double linked list. */
    /* Are we the first slider? */
    if (p_slider->prev == NULL)  {
        /* Yes, we are. */
        /* Change the front then. */
        G_firstSlider = p_slider->next ;
    } else {
        /* Change the previous element to point to the next element. */
        p_slider->prev->next = p_slider->next ;
    }

    /* Is there an element after us? */
    if (p_slider->next != NULL)  {
        /* Make the next element point to our previous. */
        p_slider->next->prev = p_slider->prev ;
    }

    /* Scrub the tag to make sure that if we have a pointer pointing */
    /* to this slider, the checks will find it.  The new tag should */
    /* be Fli. */
    p_slider->tag[0] = 'F' ;

    /* Free it from memory now that it is no longer linked. */
    MemFree(p_slider) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ISliderFind                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISliderFind is used to find a slider by its unique ID.  If a slider   */
/*  is not found, a NULL is returned.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 sliderId                                                     */
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
/*    LES  03/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_slider *ISliderFind(T_word32 sliderId)
{
    T_slider *p_slider ;

    DebugRoutine("ISliderFind") ;
    DebugCheck(G_sliderInit == TRUE) ;

    /* Start at the beginning of the list and look for a match. */
    p_slider = G_firstSlider ;

    /* Stop either if we find a match or we are at the end of the list. */
    while (p_slider != NULL)  {
        if (p_slider->sliderId == sliderId)
            break ;
        p_slider = p_slider->next ;
    }

    DebugEnd() ;

    /* We'll return NULL if we run out of items to check. */
    return p_slider ;
}

/* LES: 04/01/96 -- Routine to get rid of a slider in the slider list. */
T_void SliderDestroy(T_word32 sliderId)
{
    T_slider *p_slider ;

    DebugRoutine("SliderDestroy") ;
    DebugCheck(G_sliderInit == TRUE) ;

    p_slider = ISliderFind(sliderId) ;
    if (p_slider)
        ISliderDestroy(p_slider) ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SLIDER.C                                                */
/****************************************************************************/
