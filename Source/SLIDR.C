/*-------------------------------------------------------------------------*
 * File:  SLIDR.C
 *-------------------------------------------------------------------------*/
/**
 * User Interface sliders/scrollbars.
 *
 * @addtogroup SLIDR
 * @brief Scroll bars for UI
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "MEMORY.H"
#include "SLIDR.H"

static T_sliderID G_sliderArray[MAX_SLIDERS];
static T_void SliderDraw (T_sliderID sliderID);

/*-------------------------------------------------------------------------*
 * Routine:  SliderCreate
 *-------------------------------------------------------------------------*/
/**
 *  Adds a slider to the current list of sliders for a form
 *
 *<!-----------------------------------------------------------------------*/
T_sliderID SliderCreate (T_word16 lx1, T_word16 ly1, T_word16 lx2)
{
	T_word16 i;
   T_word16 size;
   T_sliderStruct *p_slider;

	DebugRoutine ("SliderCreate");

	for (i=0;i<MAX_SLIDERS;i++)
	{
		if (G_sliderArray[i]==NULL)  //add a slider to list
		{
         /* found an empty slot, allocate slider struct */
         p_slider = (T_sliderStruct *)G_sliderArray[i];
         size=sizeof (T_sliderStruct);
         p_slider = (T_sliderID)MemAlloc(size);
         /* check to make sure memory was allocated */
         DebugCheck (p_slider != NULL);
         p_slider->lx1=lx1;
         p_slider->ly1=ly1;
         p_slider->lx2=lx2;
         p_slider->sliderx=lx1;
         p_slider->callback=NULL;
         p_slider->curval=0;
         p_slider->knobgraphic=GraphicCreate (p_slider->sliderx,ly1-4,"UI/3DUI/SLIDER");
         G_sliderArray[i]=p_slider;
         break;
		}
	}

	DebugCheck (i<MAX_SLIDERS);
	DebugEnd();
	return (G_sliderArray[i]);
}


T_void SliderSetCallBack (T_sliderID sliderID, T_sliderHandler sliderhandler)
{
	T_sliderStruct *p_slider;

	DebugRoutine ("SliderSetCallBack");
	DebugCheck (sliderID != NULL);

	p_slider = (T_sliderStruct *)sliderID;
	p_slider->callback=sliderhandler;

	DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  SliderDelete / SliderCleanUp
 *-------------------------------------------------------------------------*/
/**
 *  Releases memory allocated to a slider
 *  Cleanup releases memory allocated to all 'sliders'
 *
 *<!-----------------------------------------------------------------------*/
T_void SliderDelete (T_sliderID sliderID)
{
	T_word16 i;
	T_sliderStruct *p_slider;

	DebugRoutine ("SliderDelete");
	if (sliderID!=NULL)
	{
		for (i=0;i<MAX_SLIDERS;i++)
		{
			if (G_sliderArray[i]==sliderID) //found it, now kill it
			{
            p_slider=(T_sliderStruct *)sliderID;
            GraphicDelete (p_slider->knobgraphic);
				MemFree (G_sliderArray[i]);
            MemCheck (600);
            G_sliderArray[i]=NULL;
				break;
			}
		}
	}

	DebugEnd();
}


T_void SliderCleanUp (T_void)
{
	T_word16 i;
	T_sliderStruct *p_slider;

	DebugRoutine ("SliderCleanUp");

	for (i=0;i<MAX_SLIDERS;i++)
	if (G_sliderArray[i]!=NULL)
	{
      p_slider = (T_sliderStruct *)G_sliderArray[i];
      GraphicDelete (p_slider->knobgraphic);
      MemFree (G_sliderArray[i]);
      MemCheck (601);
      G_sliderArray[i]=NULL;
	}
	DebugEnd();
}



/*-------------------------------------------------------------------------*
 * Routine:  SliderUpdate
 *-------------------------------------------------------------------------*/
/**
 *  Draws a slider on the screen at the stored locx+xoff, locy+yoff
 *  also will shadow if shadow is set to anything less than 255, and
 *  will not draw slider if visible or changed are set to FALSE.
 *
 *<!-----------------------------------------------------------------------*/
T_void SliderUpdate (T_sliderID sliderID)
{
   T_sliderStruct *p_slider;

   DebugRoutine ("SliderUpdate");
   /* redraw slider */
   SliderDraw (sliderID);

   p_slider=(T_sliderStruct *)sliderID;
   /* call callback if set */
   if (p_slider->callback != NULL)
   {
      p_slider->callback(sliderID);
   }

   /* force graphic update */
   GraphicUpdateAllGraphics();

   DebugEnd();
}


static T_void SliderDraw (T_sliderID sliderID)
{
    T_word16 offset2,dx;
    float offset;
	T_sliderStruct *p_slider;
    T_graphicStruct *p_graphic;

    DebugRoutine ("SliderDraw");
	DebugCheck (sliderID!=NULL);

	p_slider = (T_sliderStruct *)sliderID ;
    p_graphic = (T_graphicStruct *)p_slider->knobgraphic;

    /* set the slider location */
    dx=((p_slider->lx2-p_graphic->width+2)-p_slider->lx1);
    offset2=0-1;
    offset=p_slider->curval;
    offset/=(float)offset2;
    offset*=(float)dx;
    offset+=p_slider->lx1;

    if (offset > p_slider->lx2-p_graphic->width+1)
      offset=(float)(p_slider->lx2-p_graphic->width+1);
    else if (offset < p_slider->lx1)
      offset=p_slider->lx1;

    p_graphic->locx=(T_word16)offset;

    /* draw background */
    GrDrawRectangle (p_slider->lx1,p_slider->ly1-4,p_slider->lx2,p_slider->ly1+6,SLIDER_BACKGROUND_COLOR);

    /* draw lines */
    GrDrawHorizontalLine (p_slider->lx1,p_slider->ly1,p_slider->lx2,SLIDER_LINE1_COLOR);
    GrDrawHorizontalLine (p_slider->lx1,p_slider->ly1+1,p_slider->lx2,SLIDER_LINE2_COLOR);
    GrDrawHorizontalLine (p_slider->lx1+1,p_slider->ly1+2,p_slider->lx2+1,0);

    /* update graphic */
    p_graphic=(T_graphicStruct *)p_slider->knobgraphic;
    DebugCheck (p_graphic != NULL);
    p_graphic->changed=TRUE;

    DebugEnd();
}



/*-------------------------------------------------------------------------*
 * Routine:  SliderUpdateAllSliders
 *-------------------------------------------------------------------------*/
/**
 *  Calls SliderUpdate for all sliders currently allocated.  Note that
 *  slider will not draw if visible or changed are set to FALSE.
 *
 *<!-----------------------------------------------------------------------*/
T_void SliderUpdateAllSliders (T_void)
{
	T_word16 i;

	DebugRoutine ("SliderUpdateAllSliders");
	for (i=0;i<MAX_SLIDERS;i++)
	{
		if (G_sliderArray[i]!=NULL) SliderUpdate (G_sliderArray[i]);
	}
	DebugEnd();
}



T_void SliderSetValue (T_sliderID sliderID, T_word16 value)
{
   T_sliderStruct *p_slider;

   DebugRoutine ("SliderSetValue");
   DebugCheck (sliderID != NULL);

   p_slider=(T_sliderStruct *)sliderID;

   p_slider->curval=value;
   SliderDraw (sliderID);

   DebugEnd();
}


T_word16 SliderGetValue (T_sliderID sliderID)
{
   T_sliderStruct *p_slider;

   DebugRoutine ("SliderSetValue");
   DebugCheck (sliderID != NULL);

   p_slider=(T_sliderStruct *)sliderID;
   DebugEnd();

   return (p_slider->curval);
}



E_Boolean SliderIsAt (T_sliderID sliderID, T_word16 lx, T_word16 ly)
{
	T_sliderStruct *p_slider;
	E_Boolean retvalue=FALSE;

	DebugRoutine ("SliderIsAt");
	DebugCheck (sliderID!=NULL);

	p_slider = (T_sliderStruct *)sliderID ;

    retvalue=GraphicIsAt(p_slider->knobgraphic,lx,ly);

	DebugEnd();
	return (retvalue);
}


T_void SliderMouseControl (E_mouseEvent event,
								   T_word16 x,
								   T_word16 y,
								   T_buttonClick button)
{
   T_word16 i;
   T_word16 mvalue,dx,dx2;
   T_graphicStruct *p_graphic=NULL;
   T_sliderStruct *p_slider=NULL;
   static T_sliderID selected=NULL;

   DebugRoutine ("SliderMouseControl");

   switch (event)
   {
      case MOUSE_EVENT_START:
      selected=NULL;

      for (i=0;i<MAX_SLIDERS;i++)
      {
         if (G_sliderArray[i]!=NULL)
         {
            /* found a slider, check to see if the mouse is on it */
            if (SliderIsAt (G_sliderArray[i],x,y))
            {
               /* assign this slider as selected */
               selected=G_sliderArray[i];
               break;
            }
         }
      }
      break;

      case MOUSE_EVENT_DRAG:
      case MOUSE_EVENT_HELD:
      if (selected!=NULL)
      {
         p_slider=(T_sliderStruct *)selected;
         p_graphic=(T_graphicStruct *)p_slider->knobgraphic;
         /* drag slider knob */
         p_slider->sliderx=x-4;
         /* check bounds */
         if (p_slider->sliderx>p_slider->lx2-p_graphic->width+1)
            p_slider->sliderx=p_slider->lx2-p_graphic->width+1;
         else if (p_slider->sliderx < p_slider->lx1)
            p_slider->sliderx=p_slider->lx1;
         /* calculate the new slider value */
            dx=((p_slider->lx2-p_graphic->width+1)-p_slider->lx1);
            dx2=(p_slider->sliderx-p_slider->lx1);
            mvalue=0-1;
            mvalue/=dx;
            mvalue*=dx2;
            p_slider->curval=mvalue;
         /* redraw this slider */
//       p_graphic->locx=p_slider->sliderx;
         SliderUpdate (selected);
      }
      break;

      default:
      selected=NULL;
      break;
   }

   DebugEnd();
}


T_void *SliderGetStateBlock(T_void)
{
    T_sliderID *p_sliders;

    p_sliders = MemAlloc(sizeof(G_sliderArray)) ;
    DebugCheck(p_sliders != NULL) ;
    memcpy(p_sliders, G_sliderArray, sizeof(G_sliderArray)) ;
    memset(G_sliderArray, 0, sizeof(G_sliderArray)) ;

    return p_sliders ;
}

T_void SliderSetStateBlock(T_void *p_state)
{
    memcpy(G_sliderArray, p_state, sizeof(G_sliderArray)) ;
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  SLIDR.C
 *-------------------------------------------------------------------------*/
