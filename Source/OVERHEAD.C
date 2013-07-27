/****************************************************************************/
/*    FILE:  OVERHEAD.C                                                     */
/****************************************************************************/
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "GENERAL.H"
#include "GRAPHICS.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "OVERHEAD.H"
#include "PLAYER.H"
#include "VIEW.H"

#define INTERNAL_MAX_OFFSET_Y 200
#define INTERNAL_MAX_OFFSET_X 320
#define INTERNAL_MAX_SIZE_X 320
#define INTERNAL_MAX_SIZE_Y 200

#define OVERHEAD_SOLID_LINE   31
#define OVERHEAD_PASSIBLE_LINE   47

static E_Boolean G_initialized = FALSE ;
static T_byte8 *G_overheadPages[OVERHEAD_MAX_PAGES] = { NULL, NULL, NULL, NULL };
static T_byte8 *G_workingPage ;
static T_word16 G_numPages ;
static T_word16 G_sizeX ;
static T_word16 G_sizeY ;
static E_overheadPosition G_position ;
static T_word16 G_offsetX ;
static T_word16 G_offsetY ;
static T_sword32 G_zoom ;
static F_overheadFeature G_features ;
static E_Boolean G_on ;
static T_sword32 G_centerX ;
static T_sword32 G_centerY ;
static T_byte8 G_page ;

static T_byte8 *P_currentPage ;

/* Internal prototypes: */
static T_void IAllocatePage(T_word16 num) ;
static T_void IFreePage(T_word16 num) ;
static T_void ICalculateUpperLeft(
                  T_word16 left,
                  T_word16 top,
                  T_word16 right,
                  T_word16 bottom,
                  T_word16 *x,
                  T_word16 *y) ;
static T_void IDrawSinglePage(T_word16 page) ;
static T_void IDrawWall(T_word16 numWall) ;
static T_void IOverheadDisplay(T_word16 left, T_word16 top) ;
static T_void ICompileView(T_void) ;
static T_void IDrawLine(
                  T_sword32 x1,
                  T_sword32 y1,
                  T_sword32 x2,
                  T_sword32 y2,
                  T_byte8 color) ;
static E_Boolean IDrawObject(
                     T_3dObject *p_obj,
                     T_word32 color) ;
static T_void IActuallyDrawObject(
                     T_3dObject *p_obj,
                     T_word32 color) ;

/****************************************************************************/
/*  Routine:  OverheadInitialize                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadInitialize clears out all the memory that is needed for the   */
/*  first use of this module.                                               */
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
/*    IAllocatePage                                                         */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadInitialize(T_void)
{
    T_word16 i ;

    DebugRoutine("OverheadInitialize") ;
    DebugCheck(G_initialized == FALSE) ;

    /* Initialize as much of the data as we can. */
    G_sizeX = OVERHEAD_SIZE_X_DEFAULT ;
    G_sizeY = OVERHEAD_SIZE_Y_DEFAULT ;
    G_numPages = OVERHEAD_NUM_PAGES_DEFAULT ;
    for (i=0; i<G_numPages; i++)
        IAllocatePage(i) ;
    G_offsetX = OVERHEAD_OFFSET_X_DEFAULT ;
    G_offsetY = OVERHEAD_OFFSET_Y_DEFAULT ;
    G_zoom = OVERHEAD_ZOOM_DEFAULT ;
    G_features = OVERHEAD_FEATURE_DEFAULT ;
    G_on = FALSE ;
    G_position = OVERHEAD_POSITION_DEFAULT ;
    G_centerX = G_centerY = 0 ;
    G_page = 0 ;
    G_workingPage = MemAlloc(G_sizeX * G_sizeY) ;
    DebugCheck(G_workingPage != NULL) ;

    G_initialized = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadFinish                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadFinish undo's all the bad things remainging with the          */
/*  overhead.                                                               */
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
/*    IFreePage                                                             */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadFinish(T_void)
{
    T_word16 i ;

    DebugRoutine("OverheadFinish") ;
    DebugCheck(G_initialized == TRUE) ;

    /* All we HAVE to do is remove all the pages. */
    for (i=0; i<G_numPages; i++)
        IFreePage(i) ;

    MemFree(G_workingPage) ;

    G_initialized = FALSE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadSetNumPages                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetNumPages changes the number of pages that are "smeared"    */
/*  together when drawing the overhead view.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Changing the number of pages clears all the old pages.                */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 numPages                                                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFreePage                                                             */
/*    IAllocatePage                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetNumPages(void)
{
    T_word16 i ;

    DebugRoutine("OverheadSetNumPages") ;

    /* Delete all the old pages. */
    for (i=0; i<G_numPages; i++)
        IFreePage(i) ;

    /* Create the new pages. */
    for (i=0; i<G_numPages; i++)
        IAllocatePage(i) ;

    /* Start the page position at the beginning. */
    G_page = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadSetSize                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetSize changes the height and width of the overhead view.    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Setting a new size clears any previous pages.                         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 sizeX              -- Width of overhead view                 */
/*                                                                          */
/*    T_word16 sizeY              -- Height of the overhead view            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    OverheadSetNumPages                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetSize(T_word16 sizeX, T_word16 sizeY)
{
    DebugRoutine("OverheadSetSize") ;
    DebugCheck(sizeX < INTERNAL_MAX_SIZE_X) ;
    DebugCheck(sizeY < INTERNAL_MAX_SIZE_Y) ;

    G_sizeX = sizeX ;
    G_sizeY = sizeY ;

    /* Call SetNumPages to cause it to re-allocate the memory */
    /* necessary for the new size. */
    OverheadSetNumPages() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadGetSizeY                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetSizeY returns the height of the overhead view.             */
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
/*    T_word16                    -- Height of the overhead view            */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 OverheadGetSizeY(T_void)
{
    T_word16 sizeY ;

    DebugRoutine("OverheadGetSizeY") ;

    sizeY = G_sizeY ;

    DebugCheck(sizeY < INTERNAL_MAX_SIZE_Y) ;
    DebugEnd() ;

    return sizeY ;
}

/****************************************************************************/
/*  Routine:  OverheadGetSizeX                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetSizeX returns the width of the overhead view.              */
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
/*    T_word16                    -- Width of the overhead view             */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 OverheadGetSizeX(T_void)
{
    T_word16 sizeX ;

    DebugRoutine("OverheadGetSizeY") ;

    sizeX = G_sizeX ;

    DebugCheck(sizeX < INTERNAL_MAX_SIZE_Y) ;
    DebugEnd() ;

    return sizeX ;
}

/****************************************************************************/
/*  Routine:  OverheadSetOffset                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetOffset tells the overhead module how many pixels off the   */
/*  left or right (x) edge and how many pixels off the top or bottom (y)    */
/*  edge.  The offset depends on the position of the view (see              */
/*  OverheadSetPosition).                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 offsetX            -- Pixels off the left or right           */
/*                                                                          */
/*    T_word16 offsetY            -- Pixels off the top or bottom           */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetOffset(T_word16 offsetX, T_word16 offsetY)
{
    DebugRoutine("OverheadSetOffset") ;
    DebugCheck(offsetX < INTERNAL_MAX_OFFSET_X) ;
    DebugCheck(offsetY < INTERNAL_MAX_OFFSET_Y) ;

    G_offsetX = offsetX ;
    G_offsetY = offsetY ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadGetOffsetX                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetOffsetX returns the number of pixels to place overhead     */
/*  away from the left or right edge.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number pixels from left or right edge  */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 OverheadGetOffsetX(T_void)
{
    T_word16 offsetX ;

    DebugRoutine("OverheadGetOffsetX") ;

    offsetX = G_offsetX ;

    DebugCheck(offsetX < INTERNAL_MAX_OFFSET_X) ;
    DebugEnd() ;

    return offsetX ;
}

/****************************************************************************/
/*  Routine:  OverheadGetOffsetY                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetOffsetY returns the number of pixels to place overhead     */
/*  away from the top or bottom edge.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number pixels from top or bottom edge  */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 OverheadGetOffsetY(T_void)
{
    T_word16 offsetY ;

    DebugRoutine("OverheadGetOffsetY") ;

    offsetY = G_offsetY ;

    DebugCheck(offsetY < INTERNAL_MAX_OFFSET_Y) ;
    DebugEnd() ;

    return offsetY ;
}

/****************************************************************************/
/*  Routine:  OverheadSetPosition                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetPosition tells where to place the overhead relative to the */
/*  view.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_overheadPosition position -- What location the overhead should be   */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetPosition(E_overheadPosition position)
{
    DebugRoutine("OverheadSetPosition") ;
    DebugCheck(position < OVERHEAD_POSITION_UNKNOWN) ;

    G_position = position ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadGetPosition                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetPosition tells where the overhead is placed relative to    */
/*  the view.                                                               */
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
/*    E_overheadPosition          -- What location the overhead should be   */
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
/*    LES  09/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_overheadPosition OverheadGetPosition(T_void)
{
    E_overheadPosition position ;

    DebugRoutine("OverheadGetPosition") ;

    position = G_position ;

    DebugCheck(position < OVERHEAD_POSITION_UNKNOWN) ;
    DebugEnd() ;

    return position ;
}

/****************************************************************************/
/*  Routine:  OverheadAddFeatures                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadAddFeatures sets all the features to be used by the overhead. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    F_overheadFeature flags     -- Flags to be on                         */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetFeatures(F_overheadFeature flags)
{
    DebugRoutine("OverheadAddFeatures") ;
    DebugCheck(!(flags & OVERHEAD_FEATURE_UNKNOWN)) ;

    G_features = flags ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadGetFeatures                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetFeatures gets all the features used by the overhead.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    F_overheadFeature flags     -- Flags to be on                         */
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
/*    LES  09/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

F_overheadFeature OverheadGetFeatures(T_void)
{
    F_overheadFeature flags ;

    DebugRoutine("OverheadAddFeatures") ;

    flags = G_features ;

    DebugCheck(!(flags & OVERHEAD_FEATURE_UNKNOWN)) ;
    DebugEnd() ;

    return flags ;
}

/****************************************************************************/
/*  Routine:  OverheadAddFeatures                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadAddFeatures turns on     some of the features being used.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    F_overheadFeature flags     -- Flags to be added                      */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadAddFeatures(F_overheadFeature flags)
{
    DebugRoutine("OverheadAddFeatures") ;
    DebugCheck(!(flags & OVERHEAD_FEATURE_UNKNOWN)) ;

    G_features |= flags ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadRemoveFeatures                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadRemoveFeatures turns off some of the features being used.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    F_overheadFeature flags     -- Flags to be cleared                    */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadRemoveFeatures(F_overheadFeature flags)
{
    DebugRoutine("OverheadRemoveFeatures") ;
    DebugCheck(!(flags & OVERHEAD_FEATURE_UNKNOWN)) ;

    G_features &= (~flags) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadSetCenterPoint                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetCenterPoint declares where the view should be centered     */
/*  over.  This routine should be called before every call to OverheadDraw. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword32 x, y              -- Point to center view over              */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetCenterPoint(T_sword32 x, T_sword32 y)
{
    DebugRoutine("OverheadSetCenterPoint") ;

    G_centerX = x ;
    G_centerY = y ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadSetZoomFactor                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadSetZoomFactor sets the scaling factor used when drawing the   */
/*  the view.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword32 zoom              -- Zoom factor, smaller value is more     */
/*                                   detail.                                */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadSetZoomFactor(T_word32 zoom)
{
    DebugRoutine("OverheadSetZoomFactor") ;
    DebugCheck(zoom >= OVERHEAD_ZOOM_MAX) ;
    DebugCheck(zoom <= OVERHEAD_ZOOM_MIN) ;

    G_zoom = zoom ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadGetZoomFactor                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadGetZoomFactor gets the scaling factor used when drawing the   */
/*  the view.                                                               */
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
/*    T_sword32 zoom              -- Zoom factor, smaller value is more     */
/*                                   detail.                                */
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
/*    LES  09/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 OverheadGetZoomFactor(T_void)
{
    T_sword32 zoom ;
    DebugRoutine("OverheadSetZoomFactor") ;

    zoom = G_zoom ;

    DebugCheck(zoom >= OVERHEAD_ZOOM_MAX) ;
    DebugCheck(zoom <= OVERHEAD_ZOOM_MIN) ;
    DebugEnd() ;

    return zoom ;
}

/****************************************************************************/
/*  Routine:  OverheadToggle                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadToggle turns on the overview if it is off, and visa-versa.    */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadToggle(T_void)
{
    DebugRoutine("OverheadToggle") ;
    DebugCheck(G_on < BOOLEAN_UNKNOWN) ;

    G_on = (G_on)?FALSE:TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadOn                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadOn  turns on the overhead view.                               */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadOn(T_void)
{
    DebugRoutine("OverheadOn") ;
    DebugCheck(G_on < BOOLEAN_UNKNOWN) ;

    G_on = TRUE ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadOff                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadOff turns of the overhead view.                               */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadOff(T_void)
{
    DebugRoutine("OverheadOff") ;
    DebugCheck(G_on < BOOLEAN_UNKNOWN) ;

    G_on = FALSE ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  OverheadIsOn                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadIsOn tells if overhead view is on.                            */
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
/*    LES  10/09/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean OverheadIsOn(T_void)
{
    return G_on ;
}

/****************************************************************************/
/*  Routine:  IAllocatePage                      * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IAllocatePage sets up a page of memory for the overhead drawing and   */
/*  animation.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Will bomb if you try to allocate a non-free page.                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 num                -- Page number to allocate                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IAllocatePage(T_word16 num)
{
    T_byte8 *p_page ;

    DebugRoutine("IAllocatePage") ;
    DebugCheck(num < G_numPages) ;
    DebugCheck(num < OVERHEAD_MAX_PAGES) ;
    DebugCheck(G_overheadPages[num] == NULL) ;

    G_overheadPages[num] = p_page = MemAlloc(G_sizeX * G_sizeY) ;

    DebugCheck(p_page != NULL) ;

    /* Clear the page. */
    memset(p_page, 0, G_sizeX * G_sizeY) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFreePage                          * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFreePage removes a page of the animation from memory.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Will bomb if you try to free a free page.                             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 num                -- Page number to remove                  */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IFreePage(T_word16 num)
{
    DebugRoutine("IFreePage") ;
    DebugCheck(num < G_numPages) ;
    DebugCheck(num < OVERHEAD_MAX_PAGES) ;
    DebugCheck(G_overheadPages[num] != NULL) ;

    MemFree(G_overheadPages[num]) ;
    G_overheadPages[num] = NULL ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ICalculateUpperLeft                * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadDraw is the heart of the overhead drawing.  This routine is   */
/*  should be part of the 3d view callback routine and have the correct     */
/*  boundaries of the view.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left               -- Left of allowed view                   */
/*                                                                          */
/*    T_word16 top                -- Top of allowed  view                   */
/*                                                                          */
/*    T_word16 right              -- Right of allowed view                  */
/*                                                                          */
/*    T_word16 bottom             -- Bottom of allowed view                 */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ICalculateUpperLeft(
                  T_word16 left,
                  T_word16 top,
                  T_word16 right,
                  T_word16 bottom,
                  T_word16 *x,
                  T_word16 *y)
{
    T_sword16 xLeft ;
    T_sword16 yTop ;

    DebugRoutine("ICalculateUpperLeft") ;

    /* The top left is dependent on the position of the window. */
    switch(G_position)  {
        case OVERHEAD_POSITION_UPPER_LEFT:
            xLeft = left + G_offsetX ;
            yTop = top + G_offsetY ;
            break ;
        case OVERHEAD_POSITION_UPPER_RIGHT:
            xLeft = right - G_offsetX - G_sizeX ;
            yTop = top + G_offsetY ;
            break ;
        case OVERHEAD_POSITION_LOWER_RIGHT:
            xLeft = right - G_offsetX - G_sizeX ;
            yTop = bottom - G_offsetY - G_sizeY ;
            break ;
        case OVERHEAD_POSITION_LOWER_LEFT:
            xLeft = left + G_offsetX ;
            yTop = bottom - G_offsetY - G_sizeY ;
            break ;
        case OVERHEAD_POSITION_CENTER:
            xLeft = left+((right - left - G_sizeX)>>1) ;
            yTop = top + ((bottom - top - G_sizeY)>>1) ;
            break ;
        case OVERHEAD_POSITION_BOTTOM_CENTER:
            xLeft = left + G_offsetX ;
            yTop = bottom - G_offsetY - G_sizeY ;
            break ;
    }

    DebugCheck(xLeft > 0) ;
    DebugCheck(yTop > 0) ;
    DebugCheck(xLeft > left) ;
    DebugCheck(yTop > top) ;
    DebugCheck(xLeft + G_sizeX <= right) ;
    DebugCheck(yTop + G_sizeY <= bottom) ;

    *x = xLeft ;
    *y = yTop ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  OverheadDraw                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    OverheadDraw is the heart of the overhead drawing.  This routine is   */
/*  should be part of the 3d view callback routine and have the correct     */
/*  boundaries of the view.                                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left               -- Left of allowed view                   */
/*                                                                          */
/*    T_word16 top                -- Top of allowed  view                   */
/*                                                                          */
/*    T_word16 right              -- Right of allowed view                  */
/*                                                                          */
/*    T_word16 bottom             -- Bottom of allowed view                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ICalculateUpperLeft                                                   */
/*    IDrawSinglePage                                                       */
/*    ICompileView                                                          */
/*    IOverheadDisplay                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void OverheadDraw(
           T_word16 left,
           T_word16 top,
           T_word16 right,
           T_word16 bottom)
{
    T_word16 x, y ;
    DebugRoutine("OverheadDraw") ;

    if (G_on)  {
        memset(G_workingPage, 0, G_sizeX * G_sizeY) ;
        /* First things first, calculate where the upper left hand corner */
        /* for this view is going to be. */
        ICalculateUpperLeft(left, top, right, bottom, &x, &y) ;

        /* Draw a page. */
        IDrawSinglePage(G_page) ;

        /* Progress to the next page. */
        G_page++ ;
        if (G_page == G_numPages)
            G_page = 0 ;

        /* Compile the view. */
        ICompileView() ;

        /* Draw the page(s) on the screen. */
        IOverheadDisplay(x, y) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IOverheadDisplay                   * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IOverheadDisplay places the overhead view in the 3d view as required  */
/*  by the feature list (transparent, perspective, etc.)                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 left               -- Top left corner                        */
/*                                                                          */
/*    T_word16 top                -- Top left corner                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memcpy                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IOverheadDisplay(T_word16 left, T_word16 top)
{
    T_byte8 *p_screen ;
    T_word16 i ;
    T_byte8 *p_from ;

    DebugRoutine("IOverheadDisplay") ;

    /* Draw the view in its simplest form. */

    /* Where do we draw? */
    p_screen = (T_byte8 *)GrScreenGet() ;
    p_screen += (top * 320) + left ;

    p_from = G_workingPage ;

    if (OverheadGetFeatures() & OVERHEAD_FEATURE_TRANSPARENT)  {
        if (OverheadGetFeatures() & OVERHEAD_FEATURE_TRANSLUCENT)  {
            for (i=0; i<G_sizeY; i++, p_from += G_sizeX, p_screen+=320)  {
                DrawTranslucentSeeThroughAsm(p_from, p_screen, G_sizeX) ;
            }
        } else {
            for (i=0; i<G_sizeY; i++, p_from += G_sizeX, p_screen+=320)
                DrawSeeThroughAsm(p_from, p_screen, G_sizeX) ;
        }
    } else {
        if (OverheadGetFeatures() & OVERHEAD_FEATURE_TRANSLUCENT)  {
            for (i=0; i<G_sizeY; i++, p_from += G_sizeX, p_screen+=320)  {
                DrawTranslucentAsm(p_from, p_screen, G_sizeX) ;
            }
        } else {
            for (i=0; i<G_sizeY; i++, p_from += G_sizeX, p_screen+=320)
                memcpy(p_screen, p_from, G_sizeX) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDrawSinglePage                    * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDrawSinglePage creates the display for one page.                     */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IDrawSinglePage(T_word16 page)
{
    T_word16 i ;

    DebugRoutine("IDrawSinglePage") ;

    P_currentPage = G_overheadPages[page] ;
    DebugCheck(P_currentPage != NULL) ;
    memset(P_currentPage, 0, G_sizeX * G_sizeY) ;
    for (i=0; i<G_Num3dLines; i++)
        IDrawWall(i) ;
    if (G_features & OVERHEAD_FEATURE_OBJECTS)  {
        ObjectsDoToAll(IDrawObject, 31) ;
    }
    IActuallyDrawObject(PlayerGetObject(), 160) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDrawSinglePage                    * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDrawSinglePage creates the display for one page.                     */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IDrawWall(T_word16 numWall)
{
    T_sword32 fromX, fromY, toX, toY ;
    T_sword32 rotX, rotY ;
    T_sword32 sine, cosine ;
    T_word16 angle ;
    T_sword32 zoom ;
    T_3dLine *p_wall ;

    DebugRoutine("IDrawWall") ;

    p_wall = G_3dLineArray+numWall ;

    /* Only draw lines that have been seen or is automatically mapped */
    if ((p_wall->flags & (LINE_HAS_BEEN_SEEN | LINE_IS_AUTOMAPPED)) ||
        (G_features & OVERHEAD_FEATURE_ALL_WALLS))  {
        /* Don't draw invisible ones. */
        if (!(p_wall->flags & LINE_IS_INVISIBLE))  {
            /* Get the end points of the line. */
            fromX = (T_sword32)((T_sword16)G_3dVertexArray[p_wall->from].x) ;
            fromY = (T_sword32)((T_sword16)G_3dVertexArray[p_wall->from].y) ;
            toX = (T_sword32)((T_sword16)G_3dVertexArray[p_wall->to].x) ;
            toY = (T_sword32)((T_sword16)G_3dVertexArray[p_wall->to].y) ;

            /* Translate to the center. */
            fromX -= G_centerX  ;
            fromY -= G_centerY ;
            toX -= G_centerX ;
            toY -= G_centerY ;

            if (OverheadGetFeatures() & OVERHEAD_FEATURE_ROTATE_VIEW)  {
                angle = INT_ANGLE_90-PlayerGetAngle() ;
                sine = MathSineLookup(angle) ;
                cosine = MathCosineLookup(angle) ;

                rotX = (fromX * cosine - fromY * sine) >> 16 ;
                rotY = (fromX * sine   + fromY * cosine) >> 16 ;

                fromX = rotX ;
                fromY = rotY ;

                rotX = (toX * cosine - toY * sine) >> 16 ;
                rotY = (toX * sine   + toY * cosine) >> 16 ;

                toX = rotX ;
                toY = rotY ;
            }

            /* Scale the points to the correct zoom left. */
            /* Flip the Y's */
            zoom = G_zoom ;

            fromX = ((fromX * zoom)>>16) ;
            fromY = -((fromY * zoom)>>16) ;
            toX = ((toX * zoom)>>16) ;
            toY = -((toY * zoom)>>16) ;

            fromX += (G_sizeX>>1) ;
            fromY += (G_sizeY>>1) ;
            toX += (G_sizeX>>1) ;
            toY += (G_sizeY>>1) ;

            /* Draw the line. */
        //    IDrawLine(fromX, fromY, toX, toY, 31) ;

            /* Draw it solid unless it is impassible or "hidden" */
            if ((p_wall->flags & LINE_IS_IMPASSIBLE) ||
                (p_wall->flags & LINE_IS_ALWAYS_SOLID))
                IDrawLine(fromX, fromY, toX, toY, OVERHEAD_SOLID_LINE) ;
            else
                IDrawLine(fromX, fromY, toX, toY, OVERHEAD_PASSIBLE_LINE) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDrawLine                          * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDrawLine draw a straight line on the current page given the two      */
/*  end points.  (If the line is on the page).                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sword16 x1, y1             -- First point                           */
/*                                                                          */
/*    T_sword16 x2, y2             -- Second point                          */
/*                                                                          */
/*    T_byte8 coor                 -- Color to draw with                    */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IDrawLine(
                  T_sword32 x1,
                  T_sword32 y1,
                  T_sword32 x2,
                  T_sword32 y2,
                  T_byte8 color)
{
    T_sword32 dx, dy ;
    T_sword32 sx, sy ;
    T_sword32 step ;
    T_sword32 fract ;
    T_byte8 *p_pos ;
T_sword32 ox1, oy1, ox2, oy2 ;
ox1=x1; oy1=y1; ox2=x2; oy2=y2 ;

    dx = x2 - x1 ;
    dy = y2 - y1 ;

    /* Look for sides to clip. */
    /* Clip left edge. */
    if ((x1 < 0) && (x2 >= 0))  {
        y1 += (-x1 * dy)/dx ;
        x1 = 0 ;
    }
    if ((x2 < 0) && (x1 >= 0))  {
        y2 += (-x2 * dy)/dx ;
        x2 = 0 ;
    }

    /* Clip right edge. */
    if ((x1 >= G_sizeX) && (x2 < G_sizeX))  {
        y1 += ((G_sizeX-1 - x1) * dy) / dx ;
        x1 = G_sizeX-1 ;
    }
    if ((x2 >= G_sizeX) && (x1 < G_sizeX))  {
        y2 += ((G_sizeX-1 - x2) * dy) / dx ;
        x2 = G_sizeX-1 ;
    }

    /* Clip bottom edge. */
    if ((y1 >= G_sizeY) && (y2 < G_sizeY))  {
        x1 += ((G_sizeY-1 - y1) * dx) / dy ;
        y1 = G_sizeY-1 ;
    }
    if ((y2 >= G_sizeY) && (y1 < G_sizeY))  {
        x2 += ((G_sizeY-1 - y2) * dx) / dy ;
        y2 = G_sizeY-1 ;
    }

    /* Clip top edge. */
    if ((y1 < 0) && (y2 >= 0))  {
        x1 += (-y1 * dx)/dy ;
        y1 = 0 ;
    }
    if ((y2 < 0) && (y1 >= 0))  {
        x2 += (-y2 * dx)/dy ;
        y2 = 0 ;
    }

    /* Check to see if the line is in the view. */
    if ((x1 >= G_sizeX) && (x2 >= G_sizeX))
        return ;
    if ((x1 < 0) && (x2 < 0))
        return ;
    if ((y1 >= G_sizeY) && (y2 >= G_sizeY))
        return ;
    if ((y1 < 0) && (y2 < 0))
        return ;

    /* Re-compute the deltas. */
    dx = x2 - x1 ;
    dy = y2 - y1 ;

    /* Determine where we are going to draw. */
    p_pos = G_workingPage + y1 * G_sizeX + x1 ;

    /* Is this a dot? */
    if ((dx == 0) && (dy == 0))  {
DebugCheck(x1 >= 0) ;
DebugCheck(x1 < G_sizeX) ;
DebugCheck(y1 >= 0) ;
DebugCheck(y1 < G_sizeY) ;
        /* Then just draw a dot. */
        *p_pos = color ;
    }

    /* Now we actually have a line we can go along. */
    /* Do either a X or Y stepping algorithm. */
    if (dx < 0)
        sx = -dx ;
    else
        sx = dx ;

    if (dy < 0)
        sy = -dy ;
    else
        sy = dy ;


    if (x1 < 0)
        x1 = 0 ;
    if (x1 >= G_sizeX)
        x1 = G_sizeX-1 ;

    if (x2 < 0)
        x2 = 0 ;
    if (x2 >= G_sizeX)
        x2 = G_sizeX-1 ;

    if (y1 < 0)
        y1 = 0 ;
    if (y1 >= G_sizeY)
        y1 = G_sizeY-1 ;

    if (y2 < 0)
        y2 = 0 ;
    if (y2 >= G_sizeY)
        y2 = G_sizeY-1 ;

    /* Is X bigger than Y? */
    if (sx > sy)  {
        /* X is bigger than Y.  Draw along X. */
        if (dx == 0)
            step = 0 ;
        else
            step = (dy<<16) / dx ;
        fract = (y1<<16) ;
        if (x1 < x2)  {
            for (; x1<=x2; x1++, fract += step)  {
DebugCheck(x1 >= 0) ;
DebugCheck(x1 < G_sizeX) ;
DebugCheck((fract>>16) >= 0) ;
DebugCheck((fract>>16) < G_sizeY) ;
                G_workingPage[(fract>>16)*G_sizeX + x1] = color ;
            }
        } else {
            for (; x1>=x2; x1--, fract -= step)  {
DebugCheck(x1 >= 0) ;
DebugCheck(x1 < G_sizeX) ;
DebugCheck((fract>>16) >= 0) ;
DebugCheck((fract>>16) < G_sizeY) ;
                G_workingPage[(fract>>16)*G_sizeX + x1] = color ;
            }
        }
    } else {
        /* Y is bigger than X.  Draw along Y. */
        if (dx == 0)
            step = 0 ;
        else
            step = (dx<<16) / dy ;
        fract = (x1<<16) ;

        if (y1 < y2)  {
            for (; y1<=y2; y1++, fract+=step)  {
/*
DebugCheck(y1 >= 0) ;
DebugCheck(y1 < G_sizeY) ;
DebugCheck((fract>>16) >= 0) ;
DebugCheck((fract>>16) < G_sizeX) ;
*/
if (y1 < 0)
    y1 = 0 ;
if (y1 >= G_sizeY)
    y1 = G_sizeY-1 ;
if ((fract>>16) < 0)
    fract = 0 ;
if ((fract>>16) >= G_sizeX)
    fract = (G_sizeX-1)<<16 ;
                G_workingPage[(fract>>16) + y1*G_sizeX] = color ;
            }
        } else {
            for (; y1>=y2; y1--, fract-=step)  {
/*
DebugCheck(y1 >= 0) ;
DebugCheck(y1 < G_sizeY) ;
DebugCheck((fract>>16) >= 0) ;
DebugCheck((fract>>16) < G_sizeX) ;
*/
if (y1 < 0)
    y1 = 0 ;
if (y1 >= G_sizeY)
    y1 = G_sizeY-1 ;
if ((fract>>16) < 0)
    fract = 0 ;
if ((fract>>16) >= G_sizeX)
    fract = (G_sizeX-1)<<16 ;
                G_workingPage[(fract>>16) + y1*G_sizeX] = color ;
            }
        }
    }
}


/****************************************************************************/
/*  Routine:  ICompileView                       * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICompileView merges or blurs the pages together into one.             */
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
/*    LES  09/19/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ICompileView(T_void)
{
    DebugRoutine("ICompileView") ;

    /* For now, do nothing -- its all in the working page. */

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDrawObject                        * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDrawObject draws an object onto the view.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_3dObject *p_obj           -- Object to draw                         */
/*                                                                          */
/*    T_word32 color              -- Color to draw object in                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ObjectGetX16                                                          */
/*    ObjectGetY16                                                          */
/*    OverheadGetFeatures                                                   */
/*    PlayerGetAngle                                                        */
/*    MathSineLookup                                                        */
/*    MathCosineLookup                                                      */
/*    IDrawLine                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/22/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static E_Boolean IDrawObject(
                     T_3dObject *p_obj,
                     T_word32 color)
{
    DebugRoutine("IDrawObject") ;

    if (ObjectGetServerId(p_obj))  {
        if (!(ObjectGetAttributes(p_obj) & OBJECT_ATTR_INVISIBLE))  {
            if (!(ObjectIsCreature(p_obj)) ||
                 (G_features & OVERHEAD_FEATURE_CREATURES))  {
                IActuallyDrawObject(p_obj, color) ;
            }
        }
    }

    DebugEnd() ;

    return FALSE ;
}

/* LES: 10/23/96 */
static T_void IActuallyDrawObject(
                     T_3dObject *p_obj,
                     T_word32 color)
{
    T_sword32 x, y ;
    T_sword32 rotX, rotY ;
    T_sword32 zoom ;
    T_sword32 sine, cosine ;
    T_word16 angle ;
    T_sword32 radius ;

    x = ObjectGetX16(p_obj) ;
    y = ObjectGetY16(p_obj) ;

    /* Translate to the center. */
    x -= G_centerX  ;
    y -= G_centerY ;

    if (OverheadGetFeatures() & OVERHEAD_FEATURE_ROTATE_VIEW)  {
        angle = INT_ANGLE_90-PlayerGetAngle() ;
        sine = MathSineLookup(angle) ;
        cosine = MathCosineLookup(angle) ;

        rotX = (x * cosine - y * sine) >> 16 ;
        rotY = (x * sine   + y * cosine) >> 16 ;

        x = rotX ;
        y = rotY ;
    }

    /* Scale the points to the correct zoom left. */
    /* Flip the Y's */
    zoom = G_zoom ;

    radius = ObjectGetRadius(p_obj) ;
    radius = ((radius * zoom) >> 16) ;
    x = ((x * zoom)>>16) ;
    y = -((y * zoom)>>16) ;

    x += (G_sizeX>>1) ;
    y += (G_sizeY>>1) ;

    /* Draw the line. */
    if (ObjectGetExtraData(p_obj))
        IDrawLine(x, y, x, y, color+16) ;
    else
        IDrawLine(x, y, x, y, color) ;

    IDrawLine(x-radius, y-radius, x+radius, y-radius, 31) ;
    IDrawLine(x-radius, y+radius, x+radius, y+radius, 31) ;
    IDrawLine(x-radius, y-radius, x-radius, y+radius, 31) ;
    IDrawLine(x+radius, y-radius, x+radius, y+radius, 31) ;
}

/****************************************************************************/
/*    END OF FILE:  OVERHEAD.C                                              */
/****************************************************************************/

