/****************************************************************************/
/*    FILE:  OVERLAY .C                                                     */
/****************************************************************************/
#include "OVERLAY.H"
#include "RESOURCE.H"
#include "PICS.H"
#include "TICKER.H"

#define MAX_IMAGES 32
#define MAX_LAYERS 3
#define MAX_IMAGE_LENGTH     140

#define ANIMATION_IMAGE_NONE 0xFF

#define ANIMATION_IMAGE_TRANSLUCENT   0x01


typedef struct {
    T_sword16 xOffset ;
    T_sword16 yOffset ;
    T_byte8 imageNumber ;
    T_byte8 flags ;
} T_animationFrame ;

typedef struct {
    T_word16 lengthAnimation ;
    T_byte8 imageNames[MAX_IMAGES][16] ;
    T_animationFrame animation[MAX_LAYERS][MAX_IMAGE_LENGTH] ;
} T_overlayAnimation ;

static T_byte8 *G_images[MAX_IMAGES] ;
static T_resource G_imageResources[MAX_IMAGES] ;

static T_overlayAnimation *G_animation ;
static T_resource G_animationResource ;

static E_Boolean G_haveAnimation = FALSE ;
static E_Boolean G_isAnimating = FALSE ;

static T_word16 G_frame = 0 ;
static T_sword32 G_frameFraction = 0 ;

/* Globals: */
static T_overlayCallback G_callback = NULL ;
static E_Boolean G_init = FALSE ;
static T_word32 G_speed = 65536 ;
static T_word32 G_lastTimeUpdate = 0 ;
static T_word16 G_animationNumber ;

/* Internal prototypes: */
static T_void IUnlockAnimation(T_void) ;
static T_void ILockAnimation(T_byte8 *prefix) ;
static T_void IDrawLayer(
                  T_word16 left,
                  T_word16 top,
                  T_word16 right,
                  T_word16 bottom,
                  T_sword16 xOffset,
                  T_sword16 yOffset,
                  T_animationFrame *p_frame) ;

static E_Boolean G_translucencyMode = FALSE ;

/****************************************************************************/
/*  Routine:  OverlayInitialize                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlayInitialize sets up the basics for the overlay manager.         */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlayInitialize(T_void)
{
    DebugRoutine("OverlayInitialize") ;
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;
    G_callback = NULL ;
    G_haveAnimation = FALSE ;
    G_isAnimating = FALSE ;
    OverlaySetTranslucencyMode(FALSE) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlayFinish                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlayFinish cleans up the overlay module afterwards.                */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlayFinish(T_void)
{
    DebugRoutine("OverlayFinish") ;
    DebugCheck(G_init == TRUE) ;

    IUnlockAnimation() ;

    G_init = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlaySetCallback                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlaySetCallback declares the function to call when the overlay     */
/*  has completed the animation.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_overlayCallback p_callback -- Callback to call when done            */
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
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlaySetCallback(T_overlayCallback p_callback)
{
    DebugRoutine("OverlaySetCallback") ;

    G_callback = p_callback ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlaySetAnimation                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlaySetAnimation declares the animation to be used for the         */
/*  next overlay cycle and the overlay at the beginning.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 animationNumber     -- Number of the animation to use        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    sprintf                                                               */
/*    PictureExist                                                          */
/*    IUnlockAnimation                                                      */
/*    PictureLockData                                                       */
/*    ILockAnimation                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlaySetAnimation(T_word16 animationNumber)
{
    static char *names[ANIMATION_NUMBER_UNKNOWN] = {
        "FIST",
        "SWORD",
        "AXE",
        "MAGIC",
        "XBOW",
        "MACE",
        "DAGGER",
        "STAFF",
        "WAND",
        "NULL"
    } ;

    char filename[80] ;
    char prefix[80] ;

    DebugRoutine("OverlaySetAnimation") ;
    DebugCheck(animationNumber < ANIMATION_NUMBER_UNKNOWN) ;
    DebugCheck(G_init == TRUE) ;

    sprintf(
        prefix,
        "WEAPONS/%s",
        names[animationNumber]) ;
    sprintf(
        filename,
        "WEAPONS/%s/Info",
        names[animationNumber],
        names[animationNumber]) ;

    /* If in the middle of an animation, immediately call the */
    /* callback routine to say that the attack is complete. */
    if (G_frame != 0)
        if (G_callback)
            G_callback(G_animationNumber, 0) ;

    IUnlockAnimation() ;
    G_animation = (T_overlayAnimation *)
                      PictureLockData(filename, &G_animationResource) ;
    ILockAnimation(prefix) ;
    G_animationNumber = animationNumber ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlayAnimate                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlayAnimate starts an animation cycle.                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 speed              -- Speed of animation where 65536 is      */
/*                                   normal speed, 32768 is half, and       */
/*                                   131072 is double.                      */
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
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlayAnimate(T_word32 speed)
{
    DebugRoutine("OverlayAnimate") ;
    DebugCheck(G_init == TRUE) ;
    DebugCheck(speed > 256) ;

    /* Set the flags to allow the animation to go. */
    if (G_haveAnimation)  {
        G_frame = 0 ;
        G_frameFraction = FALSE ;
        G_isAnimating = TRUE ;
        G_lastTimeUpdate = TickerGet() ;
        G_speed = speed ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlayUpdate                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlayUpdate updates the animation of the overlay (if there is one). */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlayUpdate(E_Boolean isPaused)
{
    T_sword32 delta ;
    T_sword32 time ;
    T_word16 lastFrame ;

    DebugRoutine("OverlayUpdate") ;

    /* Do we have an animation and are we to animate? */
    if ((G_haveAnimation) && (G_isAnimating))  {
        time = TickerGet() ;
        delta = time - G_lastTimeUpdate ;
        G_lastTimeUpdate = time ;

        if (!isPaused) {
            /* If on a slow computer, we'll slow it down a little. */
            if (delta > 35)
                delta = 35 ;

            /* Multiply our speed factor. */
            delta *= G_speed ;

            /* Add in our timing. */
            G_frameFraction += delta ;

            /* Get the current frame and hold onto it. */
            lastFrame = G_frame ;

            /* Pull out what frame we now are on. */
            G_frame = (G_frameFraction >> 16) ;

            /* Clip to the upper limit. */
            if (G_frame >= G_animation->lengthAnimation)
                G_frame = G_animation->lengthAnimation ;

            /* Check for flags in the middle of where */
            /* we were and where we are about to go. */
            if (G_callback)  {
                for (; (lastFrame < G_frame) &&
                         (lastFrame < MAX_IMAGE_LENGTH);
                       lastFrame++)  {
                    if (G_animation->animation[0][lastFrame].flags)  {
                        G_callback(
                            G_animationNumber,
                            G_animation->animation[0][lastFrame].flags) ;
    //MessageAdd("Overlay with flags") ;
                    }
                }
            }

            /* If we are over the limit, end the animation. */
            /* Otherwise we are done. */
            if (G_frame >= G_animation->lengthAnimation)  {
                G_frame = 0 ;
                G_isAnimating = FALSE ;

                /* Tell them about the end of the animation, if */
                /* there is a "they" */
                if (G_callback)
                    G_callback(G_animationNumber, 0) ;
    //MessageAdd("Overlay done.") ;
            }
            }
    }

    DebugEnd() ;
}

/* added, jda */
E_Boolean OverlayIsDone(T_void)
{
    return (!G_isAnimating);
}


/****************************************************************************/
/*  Routine:  IUnlockAnimation                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUnlockAnimation releases all the graphics needed for an animation.   */
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
/*    PictureUnfindAndUnlock                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IUnlockAnimation(T_void)
{
    T_word16 i ;

    DebugRoutine("IUnlockAnimation") ;

    if (G_haveAnimation)  {
        for (i=0; i<MAX_IMAGES; i++)  {
            if (G_images[i])  {
//printf("Unlocking image %d: %s, %p\n",i, G_animation->imageNames[i], G_imageResources[i]) ;
//fflush(stdout) ;
                PictureUnlockAndUnfind(G_imageResources[i]) ;
                G_imageResources[i] = RESOURCE_BAD ;
                G_images[i] = NULL ;
            }
        }

        PictureUnlockAndUnfind(G_animationResource) ;

        G_haveAnimation = FALSE ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ILockAnimation                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IUnlockAnimation brings in all the graphics needed for an animation.  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *prefix             -- Prefix directory to find resources     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLockData                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ILockAnimation(T_byte8 *prefix)
{
    T_word16 i ;
    char filename[80] ;

    DebugRoutine("ILockAnimation") ;

    for (i=0; i<MAX_IMAGES; i++)  {
        if (G_animation->imageNames[i][0])  {
            sprintf(
                filename,
                "%s/%s",
                prefix,
                G_animation->imageNames[i]) ;
//printf("%d) Image %s\n", i, filename) ;
            G_images[i] =
                PictureLockData(
                    filename,
                    &G_imageResources[i]) ;
//printf("    %p %p\n", G_images[i], G_imageResources[i]) ;
        }
    }
    G_haveAnimation = TRUE ;
    G_frame = 0 ;
    G_isAnimating = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverlayDraw                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverlayDraw is called to draw the current frame on the screen at the  */
/*  given offset and with the give bounds.                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left, top          -- Upper left of bounding view            */
/*                                                                          */
/*    T_word16 right, bottom      -- Lower right of bounding view           */
/*                                                                          */
/*    T_word16 xOffset, yOffset   -- Offset of picture                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureLockData                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverlayDraw(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom,
           T_sword16 xOffset,
           T_sword16 yOffset)
{
    T_word16 layer ;
    T_animationFrame *p_frame ;

    DebugRoutine("OverlayDraw") ;
    DebugCheck(G_frame < G_animation->lengthAnimation) ;

    if (G_haveAnimation)  {
        for (layer=0; layer < MAX_LAYERS; layer++)  {
            p_frame = &G_animation->animation[layer][G_frame] ;

            IDrawLayer(
                left,
                top,
                right,
                bottom,
                xOffset,
                yOffset,
                p_frame) ;
        }
    }

    DebugEnd() ;
}

static T_void IDrawLayer(
                  T_word16 left,
                  T_word16 top,
                  T_word16 right,
                  T_word16 bottom,
                  T_sword16 xOffset,
                  T_sword16 yOffset,
                  T_animationFrame *p_frame)
{
    T_byte8 *p_data ;
    T_sword16 x ;
    T_sword16 y ;
    T_word16 entry ;
    T_word16 size ;
    T_sword16 drawSize ;
    T_sword16 start ;
    T_byte8 *p_screen ;

    if (p_frame->imageNumber != 255)  {
        p_data = G_images[p_frame->imageNumber] ;

        p_screen = (T_byte8 *)GrScreenGet() ;

        if (p_data)  {
            entry = *((T_word16 *)p_data) ;
            p_data += 2 ;
            while (entry != 0xFFFF)  {
                if (entry & 0x8000)  {
                    y = p_frame->yOffset + (entry & 0x7fff) + yOffset ;
                } else {
                    size = *((T_word16 *)p_data) ;
                    p_data += 2 ;
                    if (y > bottom)
                        break ;

                    if (y >= top)   {
                        x = p_frame->xOffset + entry + xOffset ;
                        drawSize = size ;
                        start = 0 ;
                        if (x < left)  {
                            start = left-x ;
                            drawSize -= start ;
                            x = left ;
                        }
                        if ((x + drawSize) > right+1)  {
                            drawSize -= ((x + drawSize) - (right+1)) ;
                        }
#if 0
#ifndef NDEBUG
if (drawSize >= 300)  {
  printf("G_frame = %d\n", G_frame) ;
  printf("imageNumber = %d\n", p_frame->imageNumber) ;
  printf("len = %d\n", G_animation->lengthAnimation) ;
  printf("G_animation = %p\n", G_animation) ;
  printf("x = %d, y=%d, xOffset=%d, yOffset=%d\n", x, y, xOffset, yOffset) ;
  printf("frameXOffset=%d, frameYOffset=%d\n", p_frame->xOffset, p_frame->yOffset) ;
  fflush(stdout) ;
  DebugCheck(drawSize < 300) ;
}
#endif
#endif

                        if (drawSize > 0)  {
                            if (G_translucencyMode)  {
                                DrawTranslucentAsm(
                                    p_data + start,
                                    p_screen + y*SCREEN_SIZE_X + x,
                                    drawSize) ;
                            } else {
                                memcpy(
                                    p_screen + y*SCREEN_SIZE_X + x,
                                    p_data + start,
                                    drawSize) ;
                            }
                        }
                    }
                    p_data += size ;
                }
                entry = *((T_word16 *)p_data) ;
                p_data += 2 ;
            }
        }
    }
}

T_void OverlaySetTranslucencyMode(E_Boolean mode)
{
    G_translucencyMode = mode ;
}


/****************************************************************************/
/*    END OF FILE:  OVERLAY .C                                              */
/****************************************************************************/
