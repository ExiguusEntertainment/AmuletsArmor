/****************************************************************************/
/*    FILE:  GRAPHIC.C                                                       */
/****************************************************************************/
#include "DEBUG.H"
#include "GRAPHIC.H"
#include "MEMORY.H"
#include "PICS.H"

static T_graphicID GraphicInit (T_word16 lx, T_word16 ly, T_byte8 *bmname);
static T_graphicID G_graphicarray[MAX_GRAPHICS];
static E_Boolean G_drawToActualScreen=TRUE;

/****************************************************************************/
/*  Routine:  GraphicCreate                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Adds a graphic to the current list of graphics for a form             */
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
/*    T_word16 x location, y location, and a picture                        */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_graphicID (id of the graphic created)                               */
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
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_graphicID GraphicCreate (T_word16 lx, T_word16 ly, T_byte8 *bmname)
{
	T_word16 i;

	DebugRoutine ("GraphicCreate");

	for (i=0;i<MAX_GRAPHICS;i++)
	{
		if (G_graphicarray[i]==NULL)  //add a graphic to list
		{
			G_graphicarray[i]=GraphicInit(lx,ly,bmname);
			break;
		}
	}

	DebugCheck (i<MAX_GRAPHICS);
	DebugEnd();
	return (G_graphicarray[i]);
}

/****************************************************************************/
/*  Routine:  GraphicInit    * INTERNAL *                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Initializes a graphic as well as allocates memory for it.             */
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
/*    T_word16 x location, y location, and a picture                        */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_graphicID (ID of the graphic created)                               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PictureFind                                                           */
/*    PictureLockQuick                                                      */
/*    PictureUnfind                                                         */
/*    PictureGetXYSize                                                      */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_graphicID GraphicInit (T_word16 lx, T_word16 ly, T_byte8 *bmname)
{
	T_word32 size;
	T_graphicStruct *myID;
	T_byte8 *picptr;

	DebugRoutine ("GraphicInit");

//    printf ("loading graphic %s\n",bmname);
//    fflush (stdout);

	DebugCheck (lx<=SCREEN_SIZE_X && ly<=SCREEN_SIZE_Y);

	size=sizeof(T_graphicStruct);         /* allocate mem for graphic */
	myID=(T_graphicID)MemAlloc(size);

	DebugCheck (myID!=NULL);
	if (myID!=NULL)
	{
		if (bmname!=NULL)
		{
			myID->graphicpic=PictureFind (bmname);
//printf ("loading picture <%s>\n",bmname);
//fflush (stdout);
			picptr=PictureLockQuick (myID->graphicpic);
			PictureGetXYSize (picptr,&myID->width,&myID->height);
			PictureUnlock (myID->graphicpic);
		} else
		{
			myID->graphicpic=NULL;
			myID->width=0;
			myID->height=0;
		}
		myID->locx=lx;
		myID->locy=ly;
		myID->xoff=0;
		myID->yoff=0;
		myID->visible=TRUE;
		myID->changed=TRUE;
		myID->shadow=255;
		myID->predrawcallback=NULL;
		myID->postdrawcallback=NULL;
		myID->predrawcbinfo=0;
		myID->postdrawcbinfo=0;
		DebugCheck (lx+myID->width<=SCREEN_SIZE_X);
		DebugCheck (ly+myID->height<=SCREEN_SIZE_Y);
	}
	DebugEnd();
	return (myID);
}


T_void GraphicSetPreCallBack (T_graphicID graphicID,
										T_graphicHandler graphichandler,
										T_word16 cbinfo)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicSetCallBack");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID;
	p_graphic->predrawcallback=graphichandler;
	p_graphic->predrawcbinfo=cbinfo;

	DebugEnd();
}

T_void GraphicSetPostCallBack (T_graphicID graphicID,
										T_graphicHandler graphichandler,
										T_word16 cbinfo)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicSetCallBack");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID;
	p_graphic->postdrawcallback=graphichandler;
	p_graphic->postdrawcbinfo=cbinfo;

	DebugEnd();
}

/****************************************************************************/
/*  Routine:  GraphicDelete / GraphicCleanUp                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Releases memory allocated to a graphic                                */
/*    Cleanup releases memory allocated to all 'graphics'                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID / none                                          */
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
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void GraphicDelete (T_graphicID graphicID)
{
	T_word16 i;
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicDelete");
	if (graphicID!=NULL)
	{
		for (i=0;i<MAX_GRAPHICS;i++)
		{
			if (G_graphicarray[i]==graphicID) //found it, now kill it
			{
				p_graphic = (T_graphicStruct *)graphicID;
				if (p_graphic->graphicpic != RESOURCE_BAD)
				{
					PictureUnfind(p_graphic->graphicpic) ;
				}
				MemFree (G_graphicarray[i]);
                MemCheck (400);
                G_graphicarray[i]=NULL;
				break;
			}
		}
	}

	DebugEnd();
}


T_void GraphicCleanUp (T_void)
{
	T_word16 i;
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicCleanUp");

	for (i=0;i<MAX_GRAPHICS;i++)
	if (G_graphicarray[i]!=NULL)
	{
		p_graphic = (T_graphicStruct *)G_graphicarray[i];
		if (p_graphic->graphicpic != RESOURCE_BAD)
		{
			PictureUnfind(p_graphic->graphicpic) ;
		}
		MemFree (G_graphicarray[i]);
        MemCheck (401);
        G_graphicarray[i]=NULL;
	}
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  GraphicUpdate                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Draws a graphic on the screen at the stored locx+xoff, locy+yoff      */
/*    also will shadow if shadow is set to anything less than 255, and      */
/*    will not draw graphic if visible or changed are set to FALSE.         */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID                                                 */
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
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void GraphicUpdate (T_graphicID graphicID)
{
	T_byte8 *p_pic;
	T_bitmap *p_bitmap ;
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicUpdate");
	DebugCheck (graphicID!=NULL);

	p_graphic = (T_graphicStruct *)graphicID ;

	if (p_graphic->changed==TRUE && p_graphic->visible==TRUE)
	{
		if (p_graphic->predrawcallback != NULL) p_graphic->predrawcallback(graphicID, p_graphic->predrawcbinfo);

//		MouseHide();

		if (p_graphic->graphicpic != NULL)
		{

			p_pic=PictureLockQuick (p_graphic->graphicpic);
			p_bitmap = PictureToBitmap(p_pic) ;
//			if (G_drawToActualScreen==TRUE) GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
			if (p_graphic->shadow==255) //no shadowing needed
			{
				GrDrawBitmap (p_bitmap,
							  p_graphic->locx+p_graphic->xoff,
							  p_graphic->locy+p_graphic->yoff);
			} else
			{
				GrDrawShadedBitmap (p_bitmap,
									p_graphic->locx+p_graphic->xoff,
									p_graphic->locy+p_graphic->yoff,
									p_graphic->shadow);
			}
			PictureUnlock (p_graphic->graphicpic);
		}

		if (p_graphic->postdrawcallback != NULL) p_graphic->postdrawcallback(graphicID, p_graphic->postdrawcbinfo);
//		MouseShow();

		p_graphic->changed=FALSE;
	}
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  GraphicUpdateAllGraphics                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Calls GraphicUpdate for all graphics currently allocated.  Note that  */
/*    graphic will not draw if visible or changed are set to FALSE.         */
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
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void GraphicUpdateAllGraphics (T_void)
{
	T_word16 i;

	DebugRoutine ("GraphicUpdateAllGraphics");
	for (i=0;i<MAX_GRAPHICS;i++)
	{
		if (G_graphicarray[i]!=NULL) GraphicUpdate (G_graphicarray[i]);
	}
	DebugEnd();
}


T_void GraphicUpdateAllGraphicsForced (T_void)
{
	T_word16 i;
    T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicUpdateAllGraphicsForced");
	for (i=0;i<MAX_GRAPHICS;i++)
	{
		if (G_graphicarray[i]!=NULL)
        {
            p_graphic=(T_graphicStruct *)G_graphicarray[i];
            p_graphic->changed=TRUE;
            GraphicUpdate (G_graphicarray[i]);
        }
	}
	DebugEnd();
}


T_void GraphicUpdateAllGraphicsBuffered (T_void)
{
    DebugRoutine ("GraphicUpdateAllGraphicsBuffered");

 //   tempscreen=GrScreenAlloc();

    /* draw shading */
    /* temporary only */
 //   for (i=0;i<200;i+=2)
 //   {
 //       GrDrawHorizontalLine (0,i,319,0);
 //  }

    /* store current screen as shaded background */
//    GrTransferRectangle (tempscreen,
//                         0,
//                         0,
//                        319,
//                         199,
//                         0,
//                         0);

//    GrScreenSet (tempscreen);
//    GraphicDrawToCurrentScreen();
	GraphicUpdateAllGraphics();

//    MouseHide();
//    GrTransferRectangle (GRAPHICS_ACTUAL_SCREEN,
//                         0,
//                         0,
//                         319,
//                         199,
//                         0,
//                         0);
//    MouseShow();
//    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);
//    GrScreenFree (tempscreen);
//    GraphicDrawToActualScreen();

//    GraphicUpdateAllGraphics();
    DebugEnd();
}

/****************************************************************************/
/*  Routine:  GraphicDrawAt                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Draws the graphic at a specific x,y location passed in.               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID, locx, locy                                     */
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
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void GraphicDrawAt (T_graphicID graphicID, T_word16 lx, T_word16 ly)
{
	T_byte8 *p_pic;
	T_bitmap *p_bitmap ;
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicDrawAt");
	DebugCheck (graphicID!=NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	DebugCheck (p_graphic->locx+p_graphic->xoff+p_graphic->width<=SCREEN_SIZE_X);
	DebugCheck (p_graphic->locy+p_graphic->yoff+p_graphic->height<=SCREEN_SIZE_Y);

	if (p_graphic->graphicpic != NULL)
	{
		p_pic=PictureLockQuick (p_graphic->graphicpic);
		p_bitmap = PictureToBitmap(p_pic) ;
//		MouseHide();
//		if (G_drawToActualScreen==TRUE) GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
	    if (p_graphic->shadow==255) GrDrawBitmap (p_bitmap,lx,ly);
		else GrDrawShadedBitmap (p_bitmap,lx,ly,p_graphic->shadow);
//		MouseShow();
		PictureUnlock (p_graphic->graphicpic);
	}
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  GraphicClear                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Draws a box of the specified color over the graphic location          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID, color                                          */
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
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void GraphicClear (T_graphicID graphicID, T_byte8 clearcolor)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicClear");
	DebugCheck (graphicID!=NULL);

	p_graphic = (T_graphicStruct *)graphicID ;

//	MouseHide();
//	if (G_drawToActualScreen==TRUE) GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
	GrDrawFrame          (p_graphic->locx+p_graphic->xoff,
						  p_graphic->locy+p_graphic->yoff,
						  p_graphic->locx+p_graphic->xoff+p_graphic->width-1,
						  p_graphic->locy+p_graphic->yoff+p_graphic->height-1,
						  clearcolor);
//	MouseShow();
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  GraphicHide/GraphicShow/GraphicSetOffSet/GraphicSetShadow     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Sets variables in the Graphic structure                               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID , other params                                  */
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
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void GraphicHide (T_graphicID graphicID)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicHide");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	p_graphic->visible=FALSE;

	DebugEnd();
}


T_void GraphicShow (T_graphicID graphicID)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicShow");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	p_graphic->visible=TRUE;
	p_graphic->changed=TRUE;

	DebugEnd();
}


T_void GraphicSetShadow (T_graphicID graphicID, T_byte8 newshadow)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicSetShadow");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	p_graphic->shadow=newshadow;
	p_graphic->changed=TRUE;

	DebugEnd();
}


T_void GraphicSetOffSet (T_graphicID graphicID, T_word16 x, T_word16 y)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicSetOffSet");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	p_graphic->xoff=x;
	p_graphic->yoff=y;
	p_graphic->changed=TRUE;

	DebugCheck (p_graphic->locx+p_graphic->xoff+p_graphic->width<=SCREEN_SIZE_X);
	DebugCheck (p_graphic->locy+p_graphic->yoff+p_graphic->height<=SCREEN_SIZE_Y);

	DebugEnd();
}

T_void GraphicSetSize (T_graphicID graphicID, T_word16 sizex, T_word16 sizey)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicSetSize");
	DebugCheck (graphicID != NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	p_graphic->width=sizex;
	p_graphic->height=sizey;
	p_graphic->changed=TRUE;

	DebugCheck (p_graphic->locx+p_graphic->xoff+p_graphic->width<=SCREEN_SIZE_X);
	DebugCheck (p_graphic->locy+p_graphic->yoff+p_graphic->height<=SCREEN_SIZE_Y);

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  GraphicIsVisible/GraphicIsAt/GraphicIsShadowed/GraphicIsOffset*/
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Returns E_booleans about the state of the given graphic               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID graphicID                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                                                             */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  07/05/95  Created                                                */
/*                                                                          */
/****************************************************************************/


E_Boolean GraphicIsVisible (T_graphicID graphicID)
{
	T_graphicStruct *p_graphic;

	DebugRoutine ("GraphicIsVisible");
	DebugCheck (graphicID!=NULL);
	p_graphic = (T_graphicStruct *)graphicID ;
	DebugCheck (p_graphic->visible < BOOLEAN_UNKNOWN);
	DebugEnd();

	return (p_graphic->visible);
}


E_Boolean GraphicIsAt (T_graphicID graphicID, T_word16 lx, T_word16 ly)
{
	T_graphicStruct *p_graphic;
	E_Boolean retvalue=FALSE;

	DebugRoutine ("GraphicIsAt");
	DebugCheck (graphicID!=NULL);

	p_graphic = (T_graphicStruct *)graphicID ;
	if ( lx>=p_graphic->locx+p_graphic->xoff-1 &&
		 lx<=(p_graphic->locx+p_graphic->width+p_graphic->xoff) &&
		 ly>=p_graphic->locy+p_graphic->yoff-1 &&
		 ly<=(p_graphic->locy+p_graphic->height+p_graphic->yoff)) retvalue=TRUE;

	DebugEnd();
	return (retvalue);
}


E_Boolean GraphicIsShadowed (T_graphicID graphicID)
{
	T_graphicStruct *p_graphic;
	E_Boolean retvalue=FALSE;

	DebugRoutine ("GraphicIsShadowed");
	DebugCheck (graphicID!=NULL);
	p_graphic = (T_graphicStruct *)graphicID ;

	if (p_graphic->shadow<255) retvalue=TRUE;

	DebugEnd();

	return (retvalue);
}

E_Boolean GraphicIsOffSet (T_graphicID graphicID)
{
	T_graphicStruct *p_graphic;
	E_Boolean retvalue=FALSE;

	DebugRoutine ("GraphicIsOffSet");

	DebugCheck (graphicID!=NULL);
	p_graphic = (T_graphicStruct *)graphicID ;

	if (p_graphic->xoff!=0 || p_graphic->yoff!=0) retvalue=TRUE;

	DebugEnd();

	return (retvalue);
}

T_void GraphicDrawToActualScreen (T_void)
{
    G_drawToActualScreen=TRUE;
}

T_void GraphicDrawToCurrentScreen (T_void)
{
    G_drawToActualScreen=FALSE;
}


T_void GraphicSetResource (T_graphicID graphicID, T_resource newresource)
{
   T_graphicStruct *p_graphic;
   DebugRoutine ("GraphicSetResource");
   DebugCheck (graphicID != NULL);

   p_graphic=(T_graphicStruct *)graphicID;
   p_graphic->graphicpic=newresource;
   p_graphic->changed=TRUE;

   DebugEnd();
}


T_resource GraphicGetResource (T_graphicID graphicID)
{
   T_graphicStruct *p_graphic;
   T_resource retvalue;

   DebugRoutine ("GraphicGetResource");
   DebugCheck (graphicID != NULL);

   p_graphic=(T_graphicStruct *)graphicID;
   retvalue=p_graphic->graphicpic;

   DebugEnd();
   return (retvalue);
}

T_void *GraphicGetStateBlock(T_void)
{
    T_graphicID *p_graphics ;

    p_graphics = MemAlloc(sizeof(G_graphicarray)) ;
    DebugCheck(p_graphics != NULL) ;
    memcpy(p_graphics, G_graphicarray, sizeof(G_graphicarray)) ;
    memset(G_graphicarray, 0, sizeof(G_graphicarray)) ;

    return p_graphics ;
}

T_void GraphicSetStateBlock(T_void *p_state)
{
    memcpy(G_graphicarray, p_state, sizeof(G_graphicarray)) ;
}
