/****************************************************************************/
/*    FILE:  TEXT.C                                                       */
/****************************************************************************/
#include "GRAPHICS.H"
#include "MEMORY.H"
#include "MOUSEMOD.H"
#include "TEXT.H"

static T_textID G_textarray[MAX_TEXTS];
static T_textID TextInit (T_word16 lx,
						  T_word16 ly,
						  const T_byte8 *string);

/****************************************************************************/
/*  Routine:  TextCreate                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Adds a text to the current list of texts for a form                   */
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
/*    location of text to be displayed, and a text string                   */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_textID (id of the text created)                                     */
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

T_textID TextCreate (T_word16 lx,
					  T_word16 ly,
					  const T_byte8 *string)
{
	T_word16 i;
	T_textStruct *p_text;

	DebugRoutine ("TextAdd");

	for (i=0;i<MAX_TEXTS;i++)
	{
		if (G_textarray[i]==NULL)  //add a text to list
		{
			G_textarray[i]=TextInit(lx,ly,string);
			p_text=(T_textStruct*)G_textarray[i];
			TextSetText (G_textarray[i],string);
			GraphicSetPreCallBack (p_text->p_graphicID,TextDrawCallBack,i);
			break;
		}
	}

	DebugCheck (i<MAX_TEXTS);
	DebugEnd();
	return (G_textarray[i]);
}



/****************************************************************************/
/*  Routine:  TextInit (* INTERNAL *)                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Allocates memory and inits variables for a new text.                */
/*    Basically, a text object creates a 'graphic' with a null picture      */
/*    and attaches a callback routine that will draw the text whenever      */
/*    the graphic is drawn.                                                 */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*    location of text to be displayed, and a text string                   */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_textID (id of the text created)                                     */
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
static T_textID TextInit (T_word16 lx,
								  T_word16 ly,
								  const T_byte8 *string)
{
	T_word32 size;
	T_resourceFile res;
	T_textStruct *myID;

	DebugRoutine ("TextInit");
	DebugCheck (lx<=320 && ly<=200);
	DebugCheck (string!=NULL);

	size=sizeof(T_textStruct);
	myID=(T_textID)MemAlloc(size);

	DebugCheck (myID!=NULL);

	if (myID!=NULL)
	{
		myID->p_graphicID=NULL;
		myID->p_graphicID=GraphicCreate (lx, ly, NULL);
		DebugCheck (myID->p_graphicID != NULL);
		myID->fcolor=31;
		myID->bcolor=0;
		res = ResourceOpen("sample.res") ;
        if (myID->font != RESOURCE_BAD)
            ResourceUnfind(myID->font) ;
		myID->font=ResourceFind(res, "FontNormal");
		ResourceClose (res);
	}
	DebugEnd();
	return (myID);
}


/****************************************************************************/
/*  Routine:  TextDelete / TextCleanUp                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Releases memory allocated to a text                                   */
/*    Cleanup releases memory allocated to all 'texts'                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_textID textID / none                                                */
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
T_void TextDelete (T_textID textID)
{
	T_word16 i;
	T_textStruct *p_text;

	DebugRoutine ("TextDelete");

	if (textID!=NULL)
	{
		for (i=0;i<MAX_TEXTS;i++)
		{
			if (G_textarray[i]==textID) //found it, now kill it
			{
				/* make sure font resource is closed */
				p_text = (T_textStruct *)textID;
				ResourceUnfind (p_text->font);
                p_text->font = RESOURCE_BAD ;
				GraphicDelete (p_text->p_graphicID);
				MemFree (p_text->data);
                MemCheck (500);
                p_text->data=NULL;
				MemFree (G_textarray[i]);
                MemCheck (501);
                G_textarray[i]=NULL;
				break;
			}
		}
	}

	DebugEnd();
}


T_void TextCleanUp (T_void)
{
	T_word16 i;
	T_textStruct *p_text;

	DebugRoutine ("TextCleanUp");

	for (i=0;i<MAX_TEXTS;i++)
	if (G_textarray[i]!=NULL)
	{
		p_text = (T_textStruct *)G_textarray[i];
		ResourceUnfind (p_text->font);
        p_text->font = RESOURCE_BAD ;
		GraphicDelete (p_text->p_graphicID);
		MemFree (p_text->data);
        MemCheck (502);
        p_text->data=NULL;
		MemFree (G_textarray[i]);
        MemCheck (503);
		G_textarray[i]=NULL;
	}
//	res=ResourceOpen ("sample.res");
//	ResourceClose (res);
	DebugEnd();
}


T_void TextSetFont (T_textID textID, T_byte8 *fontname)
{
	T_textStruct *p_text;
	T_resourceFile res;
	DebugRoutine ("TextSetFont");
	DebugCheck (fontname!=NULL);

	p_text=(T_textStruct *)textID;
	res = ResourceOpen("sample.res") ;
    if (p_text->font != RESOURCE_BAD)
        ResourceUnfind(p_text->font) ;
	p_text->font=ResourceFind(res, fontname);
	ResourceClose(res);
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  TextDrawCallBack                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*    A routine that is called to draw the text (callback routine assigned  */
/*    to p_graphicID, called from GraphicDraw)                              */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID - the graphic that called this routine                    */
/*    T_word16 index - used to find which text string to display            */
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
T_void TextDrawCallBack (T_graphicID graphicID, T_word16 index)
{
	T_graphicStruct *p_graphic;
	T_resourceFile res;
	T_textStruct *p_text;
	T_bitfont *p_font;

	DebugRoutine ("TextDrawCallBack");
	DebugCheck (graphicID != NULL);

	p_graphic=(T_graphicStruct*)graphicID;
	p_text=(T_textStruct*)G_textarray[index];

	res = ResourceOpen("sample.res") ;
  	p_font = ResourceLock(p_text->font) ;

    GrSetBitFont (p_font);
    GrSetCursorPosition (p_graphic->locx+p_graphic->xoff
							  ,p_graphic->locy+p_graphic->yoff);
    MouseHide();
	if (p_text->bcolor==0) GrDrawText (p_text->data,p_text->fcolor);
	else GrDrawShadowedText (p_text->data,p_text->fcolor,p_text->bcolor);
    MouseShow();
	ResourceUnlock (p_text->font);
	ResourceClose (res);
	DebugEnd();
}


T_graphicID TextGetGraphicID (T_textID textID)
{
	T_textStruct *p_text;

	DebugRoutine ("TextGetGraphicID");
	DebugCheck (textID != NULL);
	/* this function will return the GraphicID created by a text object */

	p_text=(T_textStruct*)textID;
	DebugEnd();
	return (p_text->p_graphicID);
}


T_void TextSetText (T_textID textID, const T_byte8 *string)
{
	T_textStruct *p_text;
	T_graphicStruct *p_graphic;
	T_bitfont *p_font;
	T_word16 i;
	T_word16 len;

	DebugRoutine ("TextSetText");
	DebugCheck (textID != NULL);

	/* set the new text */
	p_text=(T_textStruct*)textID;
	p_text->data=NULL;
	p_text->data=MemAlloc (sizeof(T_byte8)*strlen(string)+1);
	if (p_text->data!=NULL) strcpy (p_text->data,string);
	/* get the graphic ID for the text object */
    p_text->p_graphicID = MemAlloc(sizeof(T_graphicStruct));
	p_graphic=(T_graphicStruct*)p_text->p_graphicID;

	/* lock in the font */
	p_font=ResourceLock (p_text->font);
	GrSetBitFont (p_font);

	/* find the length of the new text, for width calculations */
	len=strlen(p_text->data);

	/* calculate the width of the string */
	p_graphic->width=0;
	for (i=0;i<len;i++) p_graphic->width+=GrGetCharacterWidth(p_text->data[i]);
	/* get the height of the string */
	p_graphic->height=p_font->height;

    p_graphic->changed=TRUE;
    ResourceUnlock (p_text->font);
	DebugEnd();
}


T_void TextSetColor (T_textID textID, T_byte8 fc, T_byte8 bc)
{
	T_textStruct *p_text;

	DebugRoutine ("TextSetColor");
	DebugCheck (textID != NULL);

	/* pretty self explanitory, here - get the pointer to the text structure
	and set the new values for color */

	p_text=(T_textStruct*)textID;
	p_text->fcolor=fc;
	p_text->bcolor=bc;

	DebugEnd();
}

T_void TextMoveText(T_textID textID, T_word16 x, T_word16 y)
{
	T_textStruct *p_text;
	T_graphicStruct *p_graphic;

	DebugRoutine ("TextMoveText");
	DebugCheck (textID != NULL);

	/* pretty self explanitory, here - get the pointer to the text structure
		and set the new values for x and y location */

	p_text=(T_textStruct*)textID;
	p_graphic=p_text->p_graphicID;

	p_graphic->locx=x;
	p_graphic->locy=y;

	DebugEnd();
}

/* routine removes any returns or ending whitespace */
T_void TextCleanString (T_byte8 *stringToClean)
{
    T_word16 len,i;
    DebugRoutine ("TextCleanString");

    len=strlen(stringToClean);

    /* clean up end */
    for (i=len;i!=0;i--)
    {
        if (stringToClean[i]=='\r' ||
            stringToClean[i]=='\n')
        stringToClean[i]='\0';
    }

    DebugEnd();
}
