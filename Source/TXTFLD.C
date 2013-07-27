/****************************************************************************/
/*    FILE:  TXTFLD.C                                                       */
/****************************************************************************/
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "TXTFLD.H"

static T_TxtfldID G_TxtfldArray[MAX_TXTFLDS];
static T_word16 G_currentTxtFld=0;
/****************************************************************************/
/*  Routine:  TxtfldCreate                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  Adds a text form (field) to the screen.                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
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
/*  X and Y boundary coordinates for the field, name of the font to use,    */
/*  the maximum length of the field (in chars), a hotkey, and a return      */
/*  and tab callback function                                               */
/*                                                                          */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_TxtfldID (id of the Txtfld created)                                     */
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

T_TxtfldID TxtfldCreate (T_word16 x1,
								 T_word16 y1,
								 T_word16 x2,
								 T_word16 y2,
								 T_byte8  *fontname,
								 T_byte8  maxfieldlen,
								 T_word16 key,
								 E_TxtfldDataType ndatatype,
								 T_TxtfldHandler retfunct,
								 T_TxtfldHandler tabfunct)
{
	T_word16 i,j,fheight;
	T_word32 size;
	T_TxtfldStruct *p_Txtfld;
	T_resourceFile res;
	T_bitfont *p_font;

	DebugRoutine ("TxtfldCreate");
	DebugCheck (x1<=SCREEN_SIZE_X &&
					x2<=SCREEN_SIZE_X &&
					y1<=SCREEN_SIZE_Y &&
					y2<=SCREEN_SIZE_Y &&
					x1<x2 &&
					y1<y2);

	for (i=0;i<MAX_TXTFLDS;i++)
	{
		if (G_TxtfldArray[i]==NULL) /* found an empty slot */
		{
			p_Txtfld = (T_TxtfldStruct *)G_TxtfldArray[i];
			/* we will now create a new text field structure */
			size=sizeof(T_TxtfldStruct);
			/* allocate memory for the form structure */
			p_Txtfld=(T_TxtfldID)MemAlloc(size);
			/* make sure memory was allocated */
			DebugCheck (p_Txtfld!=NULL);
			if (p_Txtfld!=NULL)
			{
				/* set the 'text graphic' to null */
				p_Txtfld->p_graphicID=NULL;
				/* make a 'text graphic' field */
				p_Txtfld->p_graphicID=GraphicCreate (x1, y1, NULL);
				/* set the size of the text window graphic */
				GraphicSetSize (p_Txtfld->p_graphicID, x2-x1,y2-y1);
				/* make sure it worked, and the graphic has been allocated */
				DebugCheck (p_Txtfld->p_graphicID != NULL);
				/* set the default fore and background colors */
				p_Txtfld->fcolor=31;
				p_Txtfld->bcolor=10;
				res = ResourceOpen("sample.res") ;
				/* make sure a font name was passed in */
				DebugCheck (fontname!=NULL);
				/* open the selected font */
				p_Txtfld->font=ResourceFind(res, fontname);
				p_font=ResourceLock (p_Txtfld->font);
				GrSetBitFont (p_font);
				/* determine the height of the font */
				fheight=p_font->height;
				/* make sure it will fit in the text field */
				DebugCheck (y2-y1>=fheight);
				/* set the maximum field length */
				p_Txtfld->fieldlength=maxfieldlen;
				/* set the hot key */
				p_Txtfld->hotkey=key;
				/* set the return and tab functions */
				p_Txtfld->retfunction=retfunct;
				p_Txtfld->tabfunction=tabfunct;
				/* set the cursor to the first character */
				p_Txtfld->cursorx=0;
				/* set the field full flag */
				p_Txtfld->fieldfull=FALSE;
				/* set the cursor on flag */
				p_Txtfld->cursoron=FALSE;
				/* set the data type enum */
				p_Txtfld->datatype=ndatatype;
				/* set the data area to nulls */
				for (j=0;j<MAX_TXTFLD_WIDTH;j++) p_Txtfld->data[j]='\0';
			}
			/* we made a new text field if we got this far, break from loop */
			G_TxtfldArray[i]=p_Txtfld;
			GraphicSetPostCallBack (p_Txtfld->p_graphicID,TxtfldTextDrawCallBack,i);
			break;
		}
	}

	DebugCheck (i<MAX_TXTFLDS);
	DebugEnd();
	return (G_TxtfldArray[i]);
}


/****************************************************************************/
/*  Routine:  TxtfldDelete / TxtfldCleanUp                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Releases memory allocated to a Txtfld                                 */
/*    Cleanup releases memory allocated to all 'Txtflds'                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_TxtfldID TxtfldID / none                                                */
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
T_void TxtfldDelete (T_TxtfldID TxtfldID)
{
	T_word16 i;
	T_TxtfldStruct *p_Txtfld;

	DebugRoutine ("TxtfldDelete");

	if (TxtfldID!=NULL)
	{
		for (i=0;i<MAX_TXTFLDS;i++)
		{
			if (G_TxtfldArray[i]==TxtfldID) /*found it, now kill it */
			{
				p_Txtfld = (T_TxtfldStruct *)TxtfldID;
				GraphicDelete (p_Txtfld->p_graphicID);
				MemFree (p_Txtfld); /*now get rid of the whole thing*/
				G_TxtfldArray[i]=NULL;
				break;
			}
		}
	}

	DebugEnd();
}


T_void TxtfldCleanUp (T_void)
{
	T_word16 i;
	T_TxtfldStruct *p_Txtfld;
	T_resourceFile res;

	DebugRoutine ("TxtfldCleanUp");

	for (i=0;i<MAX_TXTFLDS;i++)
	if (G_TxtfldArray[i]!=NULL)
	{
		p_Txtfld = (T_TxtfldStruct *)G_TxtfldArray[i];
		/* release the text-graphic structure */
		GraphicDelete (p_Txtfld->p_graphicID);
		MemFree (p_Txtfld);  /* see ya! */
		G_TxtfldArray[i]=NULL;
	}
	res=ResourceOpen ("sample.res");
	ResourceClose (res);
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  TxtfldSetColor                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  Changes the current fore and background colors for TxtfldID             */
/*                                                                          */
/*                                                                          */
/*                                                                          */
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
/*    TxtfldID of the text field to be modified, and a fore and back color  */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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

T_void TxtfldSetColor (T_TxtfldID TxtfldID, T_byte8 fc, T_byte8 bc)
{
	T_TxtfldStruct *p_Txtfld;
    T_graphicStruct *p_graphic;
	DebugRoutine ("TxtfldSetColor");
	DebugCheck (TxtfldID != NULL);

	/* pretty self explanitory, here - get the pointer to the Txtfld structure
	and set the new values for color */

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;
	p_Txtfld->fcolor=fc;
	p_Txtfld->bcolor=bc;

    p_graphic=(T_graphicStruct*)p_Txtfld->p_graphicID;
    p_graphic->changed=TRUE;
	DebugEnd();
}


T_void TxtfldSetData (T_TxtfldID txtfldID, T_byte8 *newdata)
{
	T_TxtfldStruct *p_txtfld;
	T_word16 i;

	DebugRoutine ("TxtfldSetData");
	DebugCheck (txtfldID != NULL);
	DebugCheck (strlen(newdata)<MAX_TXTFLD_WIDTH);
	/* get a pointer to the txtfld structure and replace the data string */
	p_txtfld=(T_TxtfldStruct*) txtfldID;
	/* clear out the old data */
	for (i=0;i<MAX_TXTFLD_WIDTH;i++) p_txtfld->data[i]='\0';
	strcpy (p_txtfld->data,newdata);
	p_txtfld->cursorx=strlen(newdata);
	/* force an update */
	TxtfldRedraw (txtfldID);
	DebugEnd();
}


T_byte8 *TxtfldGetData (T_TxtfldID txtfldID)
{
	T_TxtfldStruct *p_txtfld;

	DebugRoutine ("TxtfldGetData");
	DebugCheck (txtfldID != NULL);
	p_txtfld=(T_TxtfldStruct*)txtfldID;
	DebugEnd();
	/* return the data string */
	return (p_txtfld->data);
}

/****************************************************************************/
/*  Routine:  TxtfldCursLeft/CursRight                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  Moves the cursor position in the Txtfld identified by TxtfldID          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
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
/*    TxtfldID of the text field to be modified                             */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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

T_void TxtfldCursLeft (T_TxtfldID TxtfldID)
{
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldCursLeft");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;
	p_Txtfld->cursorx--;
	if (p_Txtfld->cursorx>MAX_TXTFLD_WIDTH) p_Txtfld->cursorx=0;

	/* force update of text field */
	TxtfldRedraw (TxtfldID);
	DebugEnd();
}


T_void TxtfldCursRight (T_TxtfldID TxtfldID)
{
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldCursRight");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;
	p_Txtfld->cursorx++;

	if (p_Txtfld->cursorx>=p_Txtfld->fieldlength-1)
	  p_Txtfld->cursorx=p_Txtfld->fieldlength-1;

	if (p_Txtfld->cursorx>1)
	{
		if (p_Txtfld->data[p_Txtfld->cursorx-1]=='\0') p_Txtfld->cursorx--;
	}

	/* force update of text field */
	TxtfldRedraw (TxtfldID);
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  TxtfldBackSpace/Del                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  Removes a character from the data string in a text field                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
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
/*    TxtfldID of the text field to be modified                             */
/*                                                                          *//*                                                                          *//*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
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

T_void TxtfldBackSpace (T_TxtfldID TxtfldID)
{
	T_word16 i,j;
	T_byte8 tempstr[MAX_TXTFLD_WIDTH];
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldBackSpace");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;

	if (p_Txtfld->cursorx>0)
	{
		/* set the temporary string to nulls */
		for (i=0;i<MAX_TXTFLD_WIDTH;i++) tempstr[i]='\0';

		/* copy the current cursor line up to the x cursor-1 position into */
		/* a temporary variable */
		for (i=0;i<p_Txtfld->cursorx-1;i++)
		{
			tempstr[i]=p_Txtfld->data[i];
		}

		/* now skip a character and append the rest of the data line */
		/* to the temp variable */
		for (j=i+1;j<p_Txtfld->fieldlength-1;j++)
		{
			tempstr[j-1]=p_Txtfld->data[j];
		}

		/* now try to put the contents of the temporary variable into
		the current data block */

		for (i=0;i<p_Txtfld->fieldlength;i++)
		{
			p_Txtfld->data[i]=tempstr[i];
		}

		/* force update of text field */
		TxtfldRedraw(TxtfldID);
	}

	/* move the cursor back one space */
	p_Txtfld->cursorx--;
	if (p_Txtfld->cursorx>MAX_TXTFLD_WIDTH) p_Txtfld->cursorx=0;

	DebugEnd();
}


T_void TxtfldDel (T_TxtfldID TxtfldID)
{
	T_word16 i,j;
	T_byte8 tempstr[MAX_TXTFLD_WIDTH];
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldDelete");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;

	/* set the temporary string to nulls */
	for (i=0;i<MAX_TXTFLD_WIDTH;i++) tempstr[i]='\0';

	/* copy the current cursor line up to the x cursor-1 position into */
	/* a temporary variable */
	for (i=0;i<p_Txtfld->cursorx;i++)
	{
		tempstr[i]=p_Txtfld->data[i];
	}

	/* now skip a character and append the rest of the data line */
	/* to the temp variable */
	for (j=i+1;j<p_Txtfld->fieldlength-1;j++)
	{
		tempstr[j-1]=p_Txtfld->data[j];
	}

	/* now try to put the contents of the temporary variable into
	the current data block */

	for (i=0;i<p_Txtfld->fieldlength;i++)
	{
		p_Txtfld->data[i]=tempstr[i];
	}

	/* force update of text field */
	TxtfldRedraw(TxtfldID);

	DebugEnd();
}




/****************************************************************************/
/*  Routine:  TxtfldDrawCallBack                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*    A routine that is called to draw the Txtfld (callback routine assigned  */
/*    to p_graphicID, called from GraphicDraw)                              */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_graphicID - the graphic that called this routine                    */
/*    T_word16 index - used to find which Txtfld string to display            */
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
T_void TxtfldTextDrawCallBack (T_graphicID graphicID, T_word16 index)
{
	T_graphicStruct *p_graphic;
	T_TxtfldStruct *p_Txtfld;
	T_bitfont *p_font;
	T_byte8 text[2];
	T_word16 i,j;
	T_word16 xstart=0,ystart=0;
	T_word16 curwidth=0;
    T_word16 fieldcntr=0;

	DebugRoutine ("TxtfldDrawCallBack");
	DebugCheck (graphicID != NULL);

	p_graphic=(T_graphicStruct*)graphicID;
	p_Txtfld=(T_TxtfldStruct*)G_TxtfldArray[index];

	p_font = ResourceLock(p_Txtfld->font) ;
	GrSetBitFont (p_font);

	GrDrawRectangle (p_graphic->locx-1,
						  p_graphic->locy,
						  p_graphic->locx+p_graphic->width,
						  p_graphic->locy+p_graphic->height,
						  p_Txtfld->bcolor);

    fieldcntr=p_graphic->locy+(p_graphic->height/2)-(p_font->height/2);
	GrSetCursorPosition (p_graphic->locx,fieldcntr);
	/* set the 'text' temp variable */
	text[0]=' ';
	text[1]='\0';

	/* loop through the text line, starting at the character designated by */
	/* viewstartx, and draw each character until we reach the width of the */
	/* text field window or the end of the data line*/

	for (i=0; i<p_Txtfld->fieldlength; i++)
	{
		/* draw the cursor */
		if (p_Txtfld->cursorx==i && p_Txtfld->cursoron==TRUE)
		{
			for (j=0;j<8;j++)
			{
				GrDrawHorizontalLine (p_graphic->locx+curwidth-1,
											 fieldcntr+j,
											 p_graphic->locx+curwidth-3+GrGetCharacterWidth('W'),
											 226+j);
			}
//			GrDrawRectangle (p_graphic->locx+curwidth-1,
//								  p_graphic->locy,
//								  p_graphic->locx+curwidth+GrGetCharacterWidth('W')-3,
//								  p_graphic->locy+p_font->height,
//								  235);
		}
		/* make sure we are not at the end of the data line */
		if ((p_Txtfld->data[i]!=0) && (p_Txtfld->data[i]!='\0'))
		{
			curwidth+=GrGetCharacterWidth(p_Txtfld->data[i]);
			/* if we haven't reached the end of the line, draw the character and */
			/* increment the width counter */
			if (curwidth+p_graphic->locx<=p_graphic->locx+p_graphic->width)
			{
				text[0]=p_Txtfld->data[i];
				if (p_Txtfld->cursorx==i && p_Txtfld->cursoron==TRUE)
					GrDrawShadowedText (text,240,0);
				else
					GrDrawShadowedText (text,p_Txtfld->fcolor,0);
//				GrDrawCharacter (p_Txtfld->data[i],p_Txtfld->fcolor-4);
			} else
			{
				/* no more data room in the window, notify that the field */
				/* is full */
				p_Txtfld->fieldfull=TRUE;
				break;
			}
		}
	}

	/* well, we drew the whole string, the field must not be full yet */
	if (i==p_Txtfld->fieldlength) p_Txtfld->fieldfull=FALSE;
	/* check for room for the next character */
	if (curwidth+GrGetCharacterWidth('W')>=p_graphic->width-1)
	  p_Txtfld->fieldfull=TRUE;

	/* force the graphic to update */
	p_graphic->changed=TRUE;

	/* close the font */
	ResourceUnlock (p_Txtfld->font);
	DebugEnd();
}


T_graphicID TxtfldGetTextGraphicID (T_TxtfldID TxtfldID)
{
	DebugRoutine ("TxtfldGetTextGraphicID");
	DebugCheck (TxtfldID != NULL);


	DebugEnd();
	return (NULL);
}


T_TxtfldID TxtfldIsAt (T_word16 lx, T_word16 ly)
{
	T_word16 i;
	T_TxtfldStruct *p_Txtfld;
	T_graphicStruct *p_graphic;
	T_TxtfldID retvalue=NULL;
	DebugRoutine ("TxtfldIsAt");
	for (i=0;i<MAX_TXTFLDS;i++)
	{
		if (G_TxtfldArray[i]!=NULL)
		{
			p_Txtfld=(T_TxtfldStruct*)G_TxtfldArray[i];
			p_graphic=(T_graphicStruct*) p_Txtfld->p_graphicID;

			if (lx>=p_graphic->locx &&
				 lx<=p_graphic->locx+p_graphic->width &&
				 ly>=p_graphic->locy &&
				 ly<=p_graphic->locy+p_graphic->height)
			{
				retvalue=G_TxtfldArray[i];
				break;
			}
		}
	}
	DebugEnd();
	return (retvalue);
}


T_byte8 *TxtfldGetLine (T_TxtfldID TxtfldID)
{
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldGetLine");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;

	DebugEnd();
	return ((T_byte8 *)&p_Txtfld->data);
}


E_Boolean TxtfldUseKey (T_TxtfldID TxtfldID, T_byte8 key)
{
	T_TxtfldStruct *p_Txtfld;
	T_graphicStruct *p_graphic;
	E_Boolean retvalue=FALSE;
	T_word16 i,j;
	T_byte8 tempstr[MAX_TXTFLD_WIDTH];

	DebugRoutine ("TxtfldUseKey");
	DebugCheck (TxtfldID != NULL);
	p_Txtfld=(T_TxtfldStruct*)TxtfldID;

//	printf ("key=%d\r",key);
//	fflush (stdout);

	if ((key>31) &&
		 (p_Txtfld->fieldfull==FALSE) &&
		 ((p_Txtfld->datatype==TXTFLD_ALPHANUMERIC) || (key>47 && key<58)))

	{
		/* set the temporary string to nulls */
		for (i=0;i<MAX_TXTFLD_WIDTH;i++) tempstr[i]='\0';

		/* copy the current cursor line up to the x cursor position into */
		/* a temporary variable */
		for (i=0;i<p_Txtfld->cursorx;i++)
		{
			tempstr[i]=p_Txtfld->data[i];
		}
		if (i>=p_Txtfld->fieldlength-1)
		{
			/* append a null character */
			i=p_Txtfld->fieldlength-1;
			tempstr[i]='\0';
		}
		else
		{
			/* insert the key */
			tempstr[i++]=key;
			/* append the rest of the data line to the temp variable */
			for (j=i;j<p_Txtfld->fieldlength-1;j++)
			{
				tempstr[j]=p_Txtfld->data[j-1];
			}
			/* append a null character */
			tempstr[j]='\0';

			/* move the cursor right a character */
			p_Txtfld->cursorx++;

		}
		/* now try to put the contents of the temporary variable into
		the current data block */

		for (i=0;i<p_Txtfld->fieldlength;i++)
		{
			p_Txtfld->data[i]=tempstr[i];
		}

		/* check to make sure the field is not full */
		if (p_Txtfld->data[p_Txtfld->fieldlength-2]!='\0')
		  p_Txtfld->fieldfull=TRUE;

		/* force an update */
		TxtfldRedraw (TxtfldID);
	}
	else if (key==13)
	{
		/* return was hit, call the retfunction if available */
		if (p_Txtfld->retfunction != NULL) p_Txtfld->retfunction(TxtfldID);
	}
	else if (key==9)
	{
		/* tab was hit, call the tabfunction if available */
		if (p_Txtfld->tabfunction != NULL) p_Txtfld->tabfunction(TxtfldID);
	}
	else if (p_Txtfld->fieldfull==TRUE && key>32)
	{
		p_graphic=(T_graphicStruct*)p_Txtfld->p_graphicID;
		GrDrawRectangle (p_graphic->locx-1,
							  p_graphic->locy,
							  p_graphic->locx+p_graphic->width,
							  p_graphic->locy+p_graphic->height,
							  p_Txtfld->fcolor);
		TxtfldRedraw (TxtfldID);
		//TODO: sound (1000);
		delay (10);
		//TODO: nosound();
	}
//	printf ("key=%d\r",key);
//	fflush (stdout);
	DebugEnd();
	return (retvalue);
}


T_void TxtfldRedraw (T_TxtfldID TxtfldID)
{
	T_graphicStruct *p_graphic;
	T_TxtfldStruct *p_Txtfld;
	DebugRoutine ("TxtfldRedraw");
	DebugCheck (TxtfldID != NULL);

	p_Txtfld=(T_TxtfldStruct*)TxtfldID;
	p_graphic=(T_graphicStruct*) p_Txtfld->p_graphicID;

	p_graphic->changed=TRUE;

	DebugEnd();
}


T_void TxtfldNextField (T_void)
{
	T_word16 oldtxtfld;
	T_TxtfldStruct *p_txtfld;

	DebugRoutine ("TxtfldNextField");
	oldtxtfld=G_currentTxtFld;
	/* turn off the cursor of the old text field */
	p_txtfld=(T_TxtfldStruct*)G_TxtfldArray[oldtxtfld];
	p_txtfld->cursoron=FALSE;
	/* force redraw the old text field so the cursor goes away*/
	TxtfldRedraw (G_TxtfldArray[oldtxtfld]);
	G_currentTxtFld+=1;
	if (G_currentTxtFld>=MAX_TXTFLDS) G_currentTxtFld=0;
	/* scan through txtfldarray list until a valid txtfld is found */
	/* or we pass the original point */
	while (G_TxtfldArray[G_currentTxtFld]==NULL)
	{
		G_currentTxtFld++;
		if (G_currentTxtFld>=MAX_TXTFLDS) G_currentTxtFld=0;
		if (G_currentTxtFld==oldtxtfld) break;
	}

    /* skip this field if it is not editable */
    p_txtfld=(T_TxtfldStruct*)G_TxtfldArray[G_currentTxtFld];

	DebugEnd();

    if (p_txtfld->datatype==TXTFLD_NOEDIT)
    {
        TxtfldNextField();
    }
}


T_void TxtfldKeyControl (E_keyboardEvent event, T_byte8 scankey)
{
	T_TxtfldStruct *p_txtfld;

	DebugRoutine ("TxtfldKeyControl");

	/* make sure the cursor is on */
	p_txtfld=(T_TxtfldStruct*)G_TxtfldArray[G_currentTxtFld];
	if (p_txtfld->cursoron==FALSE)
	{
		p_txtfld->cursoron=TRUE;
		TxtfldRedraw (G_TxtfldArray[G_currentTxtFld]);
	}
	switch (event)
	{
		case KEYBOARD_EVENT_BUFFERED:
		TxtfldUseKey (G_TxtfldArray[G_currentTxtFld],scankey);
		break;

		case KEYBOARD_EVENT_PRESS:
		if (scankey==KEY_SCAN_CODE_LEFT) TxtfldCursLeft(G_TxtfldArray[G_currentTxtFld]);
		if (scankey==KEY_SCAN_CODE_RIGHT) TxtfldCursRight(G_TxtfldArray[G_currentTxtFld]);
		if (scankey==KEY_SCAN_CODE_BACKSPACE) TxtfldBackSpace (G_TxtfldArray[G_currentTxtFld]);
		if (scankey==KEY_SCAN_CODE_DELETE) TxtfldDel (G_TxtfldArray[G_currentTxtFld]);
		default:
		break;
	}

	DebugEnd();
}


T_void  TxtfldMouseControl (E_mouseEvent event,
									 T_word16 x,
									 T_word16 y,
									 E_Boolean button)
{
	DebugRoutine ("TxtfldMouseControl");


	DebugEnd();
}
