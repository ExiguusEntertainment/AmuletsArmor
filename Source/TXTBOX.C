/*-------------------------------------------------------------------------*
 * File:  TXTBOX.C
 *-------------------------------------------------------------------------*/
/**
 * User interface componet for handling large text components.  This
 * handles multi-line, read-only and modify fields, and
 * scrollbars.  And also handles colors.
 *
 * @addtogroup Txtbox
 * @brief Text Box User Interface Component
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "FORM.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "KEYSCAN.H"
#include "TXTBOX.H"

static T_TxtboxID G_TxtboxArray[MAX_TxtboxES];
static T_word16 G_currentTextBox = 0;
static E_TxtboxAction G_currentAction = Txtbox_ACTION_NO_ACTION;
static T_void TxtboxAppendKeyNoRepag (T_TxtboxID TxtboxID, T_byte8 scankey);
extern T_byte8 G_extendedColors[MAX_EXTENDED_COLORS];

/*-------------------------------------------------------------------------*
 * Routine:  TxtboxCreate/TxtboxInit (internal routine)
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/

T_TxtboxID TxtboxCreate (T_word16 x1,
                         T_word16 y1,
                         T_word16 x2,
                         T_word16 y2,
                         const char *fontname,
                         T_word32 maxlength,
                         T_word16 hotkey,
                         E_Boolean numericonly,
                         E_TxtboxJustify justify,
                         E_TxtboxMode boxmode,
                         T_TxtboxHandler callback)
{
    T_word16 i;
    T_word16 windowheight;
    T_word32 size;
    T_TxtboxStruct *p_Txtbox;
    T_graphicStruct *p_graphic;
    T_resourceFile res;
    T_bitfont *p_font;

    DebugRoutine ("TxtboxCreate");
    /* make sure coordinates for textbox are valid */
    DebugCheck (x1<=SCREEN_SIZE_X &&
                x2<=SCREEN_SIZE_X &&
                y1<=SCREEN_SIZE_Y &&
                y2<=SCREEN_SIZE_Y &&
                x1<x2 &&
                y1<y2);
    /* make sure we have a fontname */
    DebugCheck (fontname!=NULL);

    /* search through global Txtbox pointer list and find an empty slot */
    for (i=0;i<MAX_TxtboxES;i++)
    {

        if (G_TxtboxArray[i]==NULL)
        {
            /* found an empty slot, create a new text box */
            p_Txtbox = (T_TxtboxStruct *)G_TxtboxArray[i];
            size=sizeof (T_TxtboxStruct);
            p_Txtbox = (T_TxtboxID)MemAlloc(size);
            /* make sure the memory was allocated */
            DebugCheck (p_Txtbox != NULL);
            /* now, set the default variables */
            p_Txtbox->p_graphicID=NULL;
            p_Txtbox->p_graphicID=GraphicCreate (x1,y1,NULL);
            GraphicSetSize (p_Txtbox->p_graphicID,x2-x1,y2-y1);
            GraphicSetPostCallBack (p_Txtbox->p_graphicID, TxtboxDrawCallBack, i);
            DebugCheck (p_Txtbox->p_graphicID!=NULL);
            p_graphic=(T_graphicStruct *)p_Txtbox->p_graphicID;
            DebugCheck(p_graphic->png==PNG_BAD);

            /* allocate inital data char */
            p_Txtbox->data = MemAlloc(sizeof(T_byte8)*2);
            MemCheck (308);
            DebugCheck(p_Txtbox->data != NULL) ;
            p_Txtbox->data[0]='\0';

            /* allocate initial linestart/linewidth data space */
            p_Txtbox->linestarts=MemAlloc(sizeof(T_word32)*2);
            MemCheck (309);
            DebugCheck (p_Txtbox->linestarts != NULL);
            p_Txtbox->linestarts[0]=0;

            p_Txtbox->linewidths=MemAlloc(sizeof(T_word16)*2);
            MemCheck (310);
            DebugCheck (p_Txtbox->linewidths != NULL);
            p_Txtbox->linewidths[0]=0;

            /* copy passed in variables */
            p_Txtbox->lx1 = x1;
            p_Txtbox->lx2 = x2;
            p_Txtbox->ly1 = y1;
            p_Txtbox->ly2 = y2;
            p_Txtbox->mode = boxmode;
            p_Txtbox->Txtboxcallback = callback;
            p_Txtbox->justify=justify;
            p_Txtbox->maxlength=maxlength;
            p_Txtbox->hotkey=hotkey;
            p_Txtbox->numericonly=numericonly;
            p_Txtbox->isselected=FALSE;
            p_Txtbox->isfull=FALSE;
            p_Txtbox->sbupID=NULL;
            p_Txtbox->sbdnID=NULL;
            p_Txtbox->sbgrID=NULL;
            p_Txtbox->sbstart=0;
            p_Txtbox->sblength=0;
            if (p_Txtbox->mode==Txtbox_MODE_EDIT_FORM ||
                p_Txtbox->mode==Txtbox_MODE_EDIT_FIELD ||
                p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                DebugCheck (p_Txtbox->justify==Txtbox_JUSTIFY_LEFT);
            }


            /* set default colors (grey text box) */
/*
            p_Txtbox->textcolor=29;
            p_Txtbox->textshadow=6;
            p_Txtbox->backcolor=8;
            p_Txtbox->bordercolor1=11;
            p_Txtbox->bordercolor2=4;
            p_Txtbox->hbackcolor=9;
            p_Txtbox->hbordercolor1=12;
            p_Txtbox->hbordercolor2=5;
            p_Txtbox->htextcolor=31;
*/
            p_Txtbox->textcolor=210;
            p_Txtbox->textshadow=0;
            p_Txtbox->backcolor=68;
            p_Txtbox->bordercolor1=66;
            p_Txtbox->bordercolor2=70;
            p_Txtbox->hbackcolor=67;
            p_Txtbox->hbordercolor1=65;
            p_Txtbox->hbordercolor2=69;
            p_Txtbox->htextcolor=209;

/*
            switch (boxmode)
            {
                case Txtbox_MODE_SELECTION_BOX:
                p_Txtbox->textcolor=210;
                p_Txtbox->textshadow=0;
                p_Txtbox->backcolor=68;
                p_Txtbox->bordercolor1=66;
                p_Txtbox->bordercolor2=70;
                p_Txtbox->hbackcolor=67;
                p_Txtbox->hbordercolor1=65;
                p_Txtbox->hbordercolor2=69;
                p_Txtbox->htextcolor=209;
                p_Txtbox->textcolor=168;
                p_Txtbox->htextcolor=168;
                p_Txtbox->textshadow=0;
                p_Txtbox->backcolor=154;
                p_Txtbox->hbackcolor=154;
                p_Txtbox->bordercolor1=152;
                p_Txtbox->hbordercolor1=152;
                p_Txtbox->bordercolor2=156;
                p_Txtbox->hbordercolor2=156;

                break;

                case Txtbox_MODE_VIEW_NOSCROLL_FORM:
                case Txtbox_MODE_VIEW_SCROLL_FORM:
                p_Txtbox->textcolor=210;
                p_Txtbox->textshadow=0;
                p_Txtbox->backcolor=68;
                p_Txtbox->bordercolor1=66;
                p_Txtbox->bordercolor2=70;
                default:
                break;
            }
*/
            /* set the cursor to 0,0 */
            p_Txtbox->cursorl=0;
            p_Txtbox->cursorline=0;
            p_Txtbox->cursorx=0;
            p_Txtbox->cursory=0;
            p_Txtbox->windowstartline=0;
            p_Txtbox->totalrows=1;

            res = ResourceOpen ("sample.res");
            /* open up the selected font */
            p_Txtbox->font=ResourceFind (res,fontname);
            p_font=ResourceLock (p_Txtbox->font);
            GrSetBitFont (p_font);

            /* determine the height of the font */
            p_Txtbox->fontheight=p_font->height;

            /* close the font */
            ResourceUnlock (p_Txtbox->font);
            ResourceUnfind (p_Txtbox->font);
            ResourceClose (res);

            /* determine how many rows we can fit in the given area */
            windowheight=y2-y1;
            p_Txtbox->windowrows=windowheight/p_Txtbox->fontheight;

            /* we made a new textbox, exit from loop */
            G_TxtboxArray[i]=p_Txtbox;
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck (i<MAX_TxtboxES);
    DebugCheck (G_TxtboxArray[i]!=NULL);

    DebugEnd();
    return (G_TxtboxArray[i]);

}




/*-------------------------------------------------------------------------*
 * Routine:  TxtboxDelete/TxtboxCleanUp
 *-------------------------------------------------------------------------*/
/**
 *  TxtboxDelete removes all data associated with TxtboxID (passed in).
 *  TxtboxCleanup removes all data associated with all textboxes.
 *
 *<!-----------------------------------------------------------------------*/
T_void TxtboxDelete (T_TxtboxID TxtboxID)
{
    T_word16 i;
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxDelete");

    if (TxtboxID != NULL)
    {
        /* search for TxtboxID in list of current textboxes */
        for (i=0;i<MAX_TxtboxES;i++)
        {
            if (G_TxtboxArray[i]==TxtboxID)
            {
                if (G_currentTextBox==i) G_currentTextBox=-0;

                /* found it, now kill it */
                p_Txtbox=(T_TxtboxStruct *)TxtboxID;
                /* get rid of the data string */
                MemFree (p_Txtbox->data);
                MemCheck (300);
                p_Txtbox->data=NULL;
                /* get rid of the linestarts */
                MemFree (p_Txtbox->linestarts);
                MemCheck (305);
                p_Txtbox->linestarts=NULL;
                MemFree (p_Txtbox->linewidths);
                MemCheck (306);
                p_Txtbox->linewidths=NULL;
                /* get rid of the graphic */
                GraphicDelete (p_Txtbox->p_graphicID);
                /* get rid of the structure */
                MemFree (G_TxtboxArray[i]);
                MemCheck (301);
                G_TxtboxArray[i]=NULL;
                /* we found it, break */
                break;
            }
        }
    }

    /* make sure we found it */
//    DebugCheck (i<MAX_TxtboxES);

    DebugEnd();
}


T_void TxtboxCleanUp (T_void)
{
    T_word16 i;
    T_resourceFile res;

    DebugRoutine ("TxtboxCleanUp");

    for (i=0;i<MAX_TxtboxES;i++)
    {
        if (G_TxtboxArray[i]!=NULL) TxtboxDelete (G_TxtboxArray[i]);
        /* not efficient, but it works */
    }
    G_currentTextBox=0;
    res=ResourceOpen ("sample.res");
    ResourceClose (res);

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  TxtboxCursUp/TxtboxCursDn/TxtboxCursPgUp/TxtboxCursPgDn
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_void TxtboxCursTop  (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursTop");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    /* move cursor position to first character */
    p_Txtbox->cursorl=0;
    p_Txtbox->cursorline=0;
    /* move window position to first character */
    p_Txtbox->windowstartline=0;

    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursBot  (T_TxtboxID TxtboxID)
{
    T_word16 wrow;
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursBot");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    switch (p_Txtbox->mode)
    {
        case Txtbox_MODE_EDIT_FIELD:
        case Txtbox_MODE_EDIT_FORM:
        case Txtbox_MODE_FIXED_WIDTH_FIELD:
        /* move cursor position to last character */
        p_Txtbox->cursorl=strlen(p_Txtbox->data);
        p_Txtbox->cursorline=p_Txtbox->totalrows;

        /* calculate the window start line for new window positon */
        wrow=p_Txtbox->cursorline;

        /* back the window start row up a number of lines */
        /* equal to the height of the window */
        if (wrow>=p_Txtbox->windowrows) wrow-=p_Txtbox->windowrows;
        p_Txtbox->windowstartline=wrow;

        break;

        case Txtbox_MODE_VIEW_SCROLL_FORM:
        case Txtbox_MODE_SELECTION_BOX:

        /* move the cursor to the beginning of the last line */
        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->totalrows];
        p_Txtbox->cursorline=p_Txtbox->totalrows;
        /* calculate the window start line for new window positon */
        wrow=p_Txtbox->cursorline;

        /* back the window start row up a number of lines */
        /* equal to the height of the window */
        if (wrow>=p_Txtbox->windowrows) wrow-=p_Txtbox->windowrows;
        p_Txtbox->windowstartline=wrow;
        break;

        default:
        break;
    }
    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursHome  (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursHome");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    /* move cursor position x to leftmost screen position possible */
    p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, 0, 0);

    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}

T_void TxtboxCursEnd  (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursEnd");
    DebugCheck (TxtboxID != NULL);
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    if (p_Txtbox->mode < Txtbox_MODE_VIEW_SCROLL_FORM ||
        p_Txtbox->mode == Txtbox_MODE_FIXED_WIDTH_FIELD)
    /* move cursor to the rightmost screen position possible */
    p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, 0, 320);

    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursUp   (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursUp");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* if we are already on the first line */
    if (p_Txtbox->cursorline==0)
    {
        /* set the cursor to the first character */
        p_Txtbox->cursorl=0;
    } else
    {
        switch (p_Txtbox->mode)
        {
            case Txtbox_MODE_EDIT_FIELD:
            case Txtbox_MODE_EDIT_FORM:
            case Txtbox_MODE_FIXED_WIDTH_FIELD:
            p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, -1, p_Txtbox->cursorx);
            p_Txtbox->cursorline--;
            break;

            case Txtbox_MODE_VIEW_SCROLL_FORM:
            case Txtbox_MODE_SELECTION_BOX:
            p_Txtbox->cursorline--;
            p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
            break;

            default:
            break;
        }
    }

    /* check to see if we've moved the window */
    if (p_Txtbox->windowstartline > p_Txtbox->cursorline)
    {
        p_Txtbox->windowstartline--;
    }

    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursDn (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    DebugRoutine ("TxtboxCursDn");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    switch (p_Txtbox->mode)
    {
        case Txtbox_MODE_EDIT_FIELD:
        case Txtbox_MODE_FIXED_WIDTH_FIELD:
        case Txtbox_MODE_EDIT_FORM:
        if (p_Txtbox->cursorline>=p_Txtbox->totalrows)
        {
            /* we're already at the bottom line */
            /* move the cursor to the last character */
            p_Txtbox->cursorline=p_Txtbox->totalrows;
            p_Txtbox->cursorl=strlen (p_Txtbox->data);
        }
        else
        {
            p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, 1, p_Txtbox->cursorx);
            p_Txtbox->cursorline++;
        }
        break;

        case Txtbox_MODE_VIEW_SCROLL_FORM:
        /* move window down a click */
        if (p_Txtbox->windowstartline <= p_Txtbox->totalrows - p_Txtbox->windowrows)
        {
            p_Txtbox->windowstartline++;
            p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->windowstartline];
            p_Txtbox->cursorline=p_Txtbox->windowstartline;
        }
        break;

        case Txtbox_MODE_SELECTION_BOX:
        if (p_Txtbox->cursorline < p_Txtbox->totalrows)
        {
            p_Txtbox->cursorline++;
            p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
        }
        break;

        default:
        break;
    }

    /* check to see if we've moved the window */
    if (p_Txtbox->cursorline >= p_Txtbox->windowstartline + p_Txtbox->windowrows )
    {
        p_Txtbox->windowstartline++;
    }

    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursLeft   (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursLeft");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    /* subtract one from the cursor l position */
    if (p_Txtbox->cursorl>0)
    {
        p_Txtbox->cursorl--;
    }

    /* check to see if we've changed rows */
    if (p_Txtbox->cursorline > 0)
    {
        if (p_Txtbox->cursorl<p_Txtbox->linestarts[p_Txtbox->cursorline])
        {
            /* subtract one from the cursor line start */
            p_Txtbox->cursorline--;
            if (p_Txtbox->windowstartline > p_Txtbox->cursorline)
            {
                p_Txtbox->windowstartline--;
            }
        }
    }

    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursRight   (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursRight");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    /* add one to the cursor l position */
    if (p_Txtbox->cursorl < strlen (p_Txtbox->data))
    {
        p_Txtbox->cursorl++;
    }

    /* check to see if we've changed rows */
    if (p_Txtbox->cursorline < p_Txtbox->totalrows)
    {
        if (p_Txtbox->cursorl > p_Txtbox->linestarts[p_Txtbox->cursorline+1])
        {
            /* increment the cursor line */
            p_Txtbox->cursorline++;
            /* see if we've moved the window */
            if (p_Txtbox->cursorline >= p_Txtbox->windowstartline+p_Txtbox->windowrows)
            {
                p_Txtbox->windowstartline++;
            }
        }
    }

    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursPgUp (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursPgUp");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    switch (p_Txtbox->mode)
    {
        case Txtbox_MODE_EDIT_FIELD:
        case Txtbox_MODE_FIXED_WIDTH_FIELD:
        case Txtbox_MODE_EDIT_FORM:
        if (p_Txtbox->cursorline > p_Txtbox->windowrows)
        {
            p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, -p_Txtbox->windowrows, p_Txtbox->cursorx);
            p_Txtbox->cursorline-=p_Txtbox->windowrows;
            if (p_Txtbox->windowstartline < p_Txtbox->cursorline)
              p_Txtbox->windowstartline = p_Txtbox->cursorline;
        } else
        {
            p_Txtbox->cursorline=0;
            p_Txtbox->windowstartline=0;
            p_Txtbox->cursorl=0;
        }
        break;

        case Txtbox_MODE_VIEW_SCROLL_FORM:
        if (p_Txtbox->windowstartline > p_Txtbox->windowrows)
        {
            p_Txtbox->windowstartline -= p_Txtbox->windowrows;
        } else
        {
            p_Txtbox->windowstartline = 0;
        }
        p_Txtbox->cursorline = p_Txtbox->windowstartline;
        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
        break;

        case Txtbox_MODE_SELECTION_BOX:

        if (p_Txtbox->cursorline > p_Txtbox->windowrows)
        {
            p_Txtbox->cursorline -= p_Txtbox->windowrows;
            p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
        } else
        {
            p_Txtbox->cursorline=0;
            p_Txtbox->cursorl=0;
        }

        /* move viewing window */
        if (p_Txtbox->windowstartline < p_Txtbox->cursorline)
          p_Txtbox->windowstartline = p_Txtbox->cursorline;
        break;

        default:
        break;
    }
    /* update the view */
    TxtboxUpdate(TxtboxID);

    DebugEnd();
}


T_void TxtboxCursPgDn (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursPgDn");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    switch (p_Txtbox->mode)
    {
        case Txtbox_MODE_EDIT_FIELD:
        case Txtbox_MODE_FIXED_WIDTH_FIELD:
        case Txtbox_MODE_EDIT_FORM:
        if (p_Txtbox->cursorline+p_Txtbox->windowrows > p_Txtbox->totalrows)
        {
            p_Txtbox->cursorline=p_Txtbox->totalrows;
            p_Txtbox->windowstartline = p_Txtbox->cursorline;
            p_Txtbox->cursorl=strlen(p_Txtbox->data);
        } else
        {
            p_Txtbox->cursorl=TxtboxScanRow(TxtboxID, p_Txtbox->windowrows, p_Txtbox->cursorx);
            p_Txtbox->cursorline += p_Txtbox->windowrows;
            if (p_Txtbox->windowstartline < p_Txtbox->cursorline-p_Txtbox->windowrows)
              p_Txtbox->windowstartline = p_Txtbox->cursorline-p_Txtbox->windowrows;
        }
        break;

        case Txtbox_MODE_VIEW_SCROLL_FORM:
        /*move view down a whole screen */
        p_Txtbox->windowstartline += p_Txtbox->windowrows;
        if (p_Txtbox->windowstartline > p_Txtbox->totalrows - p_Txtbox->windowrows+1)
          p_Txtbox->windowstartline = p_Txtbox->totalrows - p_Txtbox->windowrows+1;
        p_Txtbox->cursorline = p_Txtbox->windowstartline;
        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
        break;

        case Txtbox_MODE_SELECTION_BOX:

        p_Txtbox->cursorline+=p_Txtbox->windowrows;
        if (p_Txtbox->cursorline > p_Txtbox->totalrows)
          p_Txtbox->cursorline=p_Txtbox->totalrows;

        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];

        /* move viewing window down if necessary */
        if (p_Txtbox->windowstartline + p_Txtbox->windowrows < p_Txtbox->cursorline)
        {
            p_Txtbox->windowstartline = p_Txtbox->cursorline - p_Txtbox->windowrows;
        }
        break;

        default:
        break;
    }

    /* update the view */
    TxtboxUpdate (TxtboxID);

    DebugEnd();
}


T_void TxtboxCursSetRow (T_TxtboxID TxtboxID, T_word16 row)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxCursSetRow");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    if (row > p_Txtbox->totalrows) row=p_Txtbox->totalrows;

    /* set row */
    p_Txtbox->cursorline=row;
    p_Txtbox->cursorl=p_Txtbox->linestarts[row];
    if (p_Txtbox->windowstartline > row)
     p_Txtbox->windowstartline=row;

    /* force update */
    TxtboxUpdate (TxtboxID);

    DebugEnd();
}



T_word32 TxtboxScanRow (T_TxtboxID TxtboxID, T_word16 rowinc, T_word16 ox)
{
    T_TxtboxStruct *p_Txtbox;
    T_bitfont *p_font;

    T_word16 startrow,startch,startx,targetx,wsize;

    T_word32 i=0,retvalue=0;

    DebugRoutine ("TxtboxScanRow");

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* open the font */
    p_font = ResourceLock(p_Txtbox->font) ;
	GrSetBitFont (p_font);

    wsize=GrGetCharacterWidth('W');

    startrow=p_Txtbox->cursorline+rowinc;
    if (startrow > p_Txtbox->totalrows) startrow=p_Txtbox->totalrows;

    startch=p_Txtbox->linestarts[startrow];
    startx=p_Txtbox->lx1;
    targetx=ox;

    /* scan the row of text for a near-x cursor positon */
    for (i=startch;i<strlen(p_Txtbox->data);i++)
    {
        if (p_Txtbox->data[i]==13)
        {
            /* we found a return, set the cursor here */
            retvalue=i;
            break;
        } else if (p_Txtbox->data[i]==9)
        {
            /* we have a tab here, advance to the nearest tab position */
            /* currently 3*wsize or 3 capital doubleyous :) */
            startx=((((startx-p_Txtbox->lx1)/(3*wsize))+1)*(3*wsize))+p_Txtbox->lx1;
            /* make sure we haven't reached the end of the line */
            if (startx+wsize>p_Txtbox->lx2)
            {
                /* out of room, set the cursor here */
                retvalue=i;
                break;
            }
        }
        else
        {   /* normal character */
            startx+=GrGetCharacterWidth (p_Txtbox->data[i]);
            if (startx+wsize>p_Txtbox->lx2)
            {
                /* reached end of line, drop down a row */
                retvalue=i;
                break;
            }
        }

        if (startx>targetx) /* here we are */
        {
            retvalue=i;
            break;
        }
    }

    if (retvalue>=strlen(p_Txtbox->data)) retvalue=strlen(p_Txtbox->data);


    /* check for total failure */
    if (retvalue==0 && startx < targetx) retvalue=strlen(p_Txtbox->data);

    /* close the font */
    ResourceUnlock (p_Txtbox->font);

    DebugEnd();

    return (retvalue);
}






/*-------------------------------------------------------------------------*
 * Routine:  TxtboxAppendKey/TxtboxBackSpace
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/

T_void TxtboxAppendKey (T_TxtboxID TxtboxID, T_word16 scankey)
{
    T_TxtboxStruct *p_Txtbox;
    T_byte8 *newdata;
    T_word32 len;

    DebugRoutine ("TxtboxAppendKey");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* make sure the data string is valid */
    DebugCheck (p_Txtbox->data!=NULL);

    /* make sure we have room for another character */
    if ((p_Txtbox->isfull==FALSE) &&
        (p_Txtbox->numericonly==FALSE || (scankey>='0' && scankey<='9')) &&
        (strlen(p_Txtbox->data)<p_Txtbox->maxlength))
    {

        /* get the length of the current data string */
        len=strlen(p_Txtbox->data);

        /* create a new string with old length +1 */
        newdata=MemAlloc((sizeof(T_byte8))*(len+2));
        MemCheck (1);

        /* copy the old contents, up to the cursorl position to the new data area */
        memcpy (newdata,p_Txtbox->data,p_Txtbox->cursorl);

        /* insert the new character */
        newdata[p_Txtbox->cursorl]=(T_byte8)scankey;

        /* copy the rest of the old string to the new one */
        memcpy (newdata+p_Txtbox->cursorl+1,
            p_Txtbox->data+p_Txtbox->cursorl,len+1-p_Txtbox->cursorl);

        /* delete the old data string */
        MemFree (p_Txtbox->data);
        MemCheck (302);
        p_Txtbox->data=NULL;

        /* set the pointer */
        p_Txtbox->data=newdata;

        if ((scankey==13) &&
            (p_Txtbox->cursorline==p_Txtbox->windowstartline+p_Txtbox->windowrows-1))
            p_Txtbox->windowstartline++;

        if (scankey==9 || scankey==13 || scankey==31)
          TxtboxRepaginateAll (TxtboxID);
        /* repaginate the form */
        else TxtboxRepaginate(TxtboxID);

        /* add one to the cursor position */
        TxtboxCursRight (TxtboxID);

    }
    DebugEnd();
}



T_void TxtboxAppendKeyNoRepag (T_TxtboxID TxtboxID, T_byte8 scankey)
{
    T_TxtboxStruct *p_Txtbox;
    T_byte8 *newdata;
    T_word32 len;

    DebugRoutine ("TxtboxAppendKeyNoRepag");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* make sure the data string is valid */
    DebugCheck (p_Txtbox->data!=NULL);

    /* make sure we have room for another character */
    if ((p_Txtbox->isfull==FALSE) &&
        (p_Txtbox->numericonly==FALSE || (scankey>='0' && scankey<='9')) &&
        (strlen(p_Txtbox->data)<p_Txtbox->maxlength))
    {

        /* get the length of the current data string */
        len=strlen(p_Txtbox->data);

        /* create a new string with old length +1 */
        newdata=MemAlloc((sizeof(T_byte8))*(len+2));
        MemCheck (1);

        /* copy the old contents, up to the cursorl position to the new data area */
        memcpy (newdata,p_Txtbox->data,p_Txtbox->cursorl);

        /* insert the new character */
        newdata[p_Txtbox->cursorl]=scankey;

        /* copy the rest of the old string to the new one */
        memcpy (newdata+p_Txtbox->cursorl+1,
            p_Txtbox->data+p_Txtbox->cursorl,len+1-p_Txtbox->cursorl);

        /* delete the old data string */
        MemFree (p_Txtbox->data);
        p_Txtbox->data=NULL;

        /* set the pointer */
        p_Txtbox->data=newdata;

        if ((scankey==13) &&
            (p_Txtbox->cursorline==p_Txtbox->windowstartline+p_Txtbox->windowrows-1))
            p_Txtbox->windowstartline++;

        if (scankey==9 || scankey==13 || scankey==31)
          TxtboxRepaginate (TxtboxID);
        /* repaginate the form */
//        else TxtboxRepaginate(TxtboxID);

        /* add one to the cursor position */
        TxtboxCursRight (TxtboxID);

    }
    DebugEnd();
}



T_void TxtboxBackSpace (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_byte8 *newdata;
    T_word32 len;

    DebugRoutine ("TxtboxBackSpace");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* make sure the data string is valid */
    DebugCheck (p_Txtbox->data!=NULL);

    /* get the length of the current data string */
    len=strlen(p_Txtbox->data);

    /* make sure we have a character to delete ! */
    if (len>0 && p_Txtbox->cursorl>0)
    {

        /* create a new string with old length -1 */
        newdata=MemAlloc((sizeof(T_byte8))*(len+1));

        /* copy the old contents, up to the cursorl-1 position to the new data area */
        memcpy (newdata,p_Txtbox->data,p_Txtbox->cursorl-1);

        /* copy the rest of the old string to the new one */
        memcpy (newdata+p_Txtbox->cursorl-1,
                p_Txtbox->data+p_Txtbox->cursorl,
                len-p_Txtbox->cursorl+1);

        /* delete the old data string */
        MemFree (p_Txtbox->data);
        MemCheck (303);
        p_Txtbox->data=NULL;

        /* set the pointer */
        p_Txtbox->data=newdata;

        /* repaginate the form */
        TxtboxRepaginateAll(TxtboxID);

        /* subtract one to the cursor position */
        TxtboxCursLeft (TxtboxID);
    }

    DebugEnd();
}




T_void TxtboxAppendString (T_TxtboxID TxtboxID, const char *data)
{
    T_byte8 val;
    T_word16 i;

    DebugRoutine ("TxtboxAppendString");
    DebugCheck (TxtboxID != NULL);

    /* scan for control shifted (^) characters */
    for (i=0;i<strlen(data);i++)
    {
        if (data[i]=='^')
        {
            /* found a 'control' char, read in 3 digits */
            /* calculate the value */
            val=0;
            val+=((data[++i]-'0')*100);
            val+=((data[++i]-'0')*10);
            val+=(data[++i]-'0');

            TxtboxAppendKeyNoRepag(TxtboxID,val+128);
//tempstr2[j++]=val+128;
        } else TxtboxAppendKeyNoRepag (TxtboxID, data[i]);
//tempstr2[j++]=tempstr[i];
    }

//    TxtboxRepaginateAll (TxtboxID);
    DebugEnd();
}


T_void TxtboxSetData (T_TxtboxID TxtboxID, const char *string)
{
    T_TxtboxStruct *p_Txtbox;
    T_word16 i,cnt=0;
    T_byte8 val;
    DebugRoutine ("TxtboxSetData");
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* delete old data string */
    MemFree (p_Txtbox->data);
    MemCheck (304);
    p_Txtbox->data=NULL;

    /* allocate a new data block the size of the string parameter */
    p_Txtbox->data= MemAlloc(sizeof(T_byte8)*strlen(string)+2);

    /* make sure it worked */
    DebugCheck (p_Txtbox->data != NULL);

    /* copy the data string */
    for (i=0;i<strlen(string);i++)
    {
        if (string[i]=='^')
        {
            /* found a 'control' char, read in 3 digits */
            /* calculate the value */
            val=0;
            val+=((string[++i]-'0')*100);
            val+=((string[++i]-'0')*10);
            val+=(string[++i]-'0');

            p_Txtbox->data[cnt++]=val+128;
        } else p_Txtbox->data[cnt++]=string[i];
    }
    p_Txtbox->data[cnt]='\0';

    /* move the cursor to the top */
    p_Txtbox->cursorl=0;
    p_Txtbox->cursorline=0;
    p_Txtbox->windowstartline=0;

    /* repaginate the form */
    TxtboxRepaginateAll (TxtboxID);

    /* force a screen update */
    TxtboxUpdate (TxtboxID);

    DebugEnd();
}

T_void TxtboxSetNData (T_TxtboxID TxtboxID, T_byte8 *string, T_word32 len)
{
    T_TxtboxStruct *p_Txtbox;
    T_word16 i,cnt=0;
    T_byte8 val;
    DebugRoutine ("TxtboxSetData");
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* delete old data string */
    MemFree (p_Txtbox->data);
    MemCheck (304);
    p_Txtbox->data=NULL;

    /* allocate a new data block the size of the string parameter */
    p_Txtbox->data= MemAlloc(sizeof(T_byte8)*len+2);

    /* make sure it worked */
    DebugCheck (p_Txtbox->data != NULL);

    /* copy the data string */
    for (i=0;i<len;i++)
    {
        if (string[i]=='^')
        {
            /* found a 'control' char, read in 3 digits */
            /* calculate the value */
            val=0;
            val+=((string[++i]-'0')*100);
            val+=((string[++i]-'0')*10);
            val+=(string[++i]-'0');

            p_Txtbox->data[cnt++]=val+128;
        } else p_Txtbox->data[cnt++]=string[i];
    }
    p_Txtbox->data[cnt]='\0';

    /* move the cursor to the top */
    p_Txtbox->cursorl=0;
    p_Txtbox->cursorline=0;
    p_Txtbox->windowstartline=0;

    /* repaginate the form */
    TxtboxRepaginateAll (TxtboxID);

    /* force a screen update */
    TxtboxUpdate (TxtboxID);

    DebugEnd();
}





T_byte8 *TxtboxGetData (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxGetData");
    DebugCheck (TxtboxID != NULL);
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    DebugEnd();
    return (p_Txtbox->data);
}

T_word32 TxtboxGetDataLength (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxGetDataLength");
    DebugCheck (TxtboxID != NULL);
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    DebugEnd();
    return (strlen(p_Txtbox->data));
}


T_void TxtboxSetColor (T_TxtboxID TxtboxID,
                       T_byte8 txtcolor,
                       T_byte8 bkcolor,
                       T_byte8 txtshadow,
                       T_byte8 bordclr1,
                       T_byte8 bordclr2)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxSetColor");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    p_Txtbox->textcolor=txtcolor;
    p_Txtbox->backcolor=bkcolor;
    p_Txtbox->textshadow=txtshadow;
    p_Txtbox->bordercolor1=bordclr1,
    p_Txtbox->bordercolor2=bordclr2;

    /* force an update */
    TxtboxUpdate (TxtboxID);

    DebugEnd();
}




/*-------------------------------------------------------------------------*
 * Routine:  TxtboxIsAt
 *-------------------------------------------------------------------------*/
/**
 *  Returns TRUE if a text box is at the given coordinates
 *  Otherwise returns FALSE
 *
 *<!-----------------------------------------------------------------------*/

E_Boolean TxtboxIsAt (T_TxtboxID TxtboxID, T_word16 x, T_word16 y)
{
    T_TxtboxStruct *p_Txtbox;
    E_Boolean retvalue=FALSE;

    DebugRoutine ("TxtboxIsAt");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    if ((x>=p_Txtbox->lx1) &&
        (x<=p_Txtbox->lx2) &&
        (y>=p_Txtbox->ly1) &&
        (y<=p_Txtbox->ly2)) retvalue=TRUE;

    DebugEnd();

    return (retvalue);
}

/*-------------------------------------------------------------------------*
 * Routine:  TxtboxHandleKey/TxtboxHandleMouse
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_void TxtboxKeyControl (E_keyboardEvent event, T_word16 scankey)
{

    T_TxtboxStruct *p_Txtbox;
    T_byte8 *data,*tempstr;
    DebugRoutine ("TxtboxKeyControl");

    p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[G_currentTextBox];

    G_currentAction = Txtbox_ACTION_NO_ACTION;

    switch (event)
    {
        case KEYBOARD_EVENT_BUFFERED:

        if (scankey==8)
        {
            /* recieved a backspace, see what to do with it */
            if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM ||
                p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                TxtboxBackSpace (G_TxtboxArray[G_currentTextBox]);
                G_currentAction = Txtbox_ACTION_DATA_CHANGED;
            }
        }
        else if (scankey>31 && scankey < 128)
        {
            if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM)
            {
                TxtboxAppendKey(G_TxtboxArray[G_currentTextBox],scankey);
                G_currentAction = Txtbox_ACTION_DATA_CHANGED;
            }
            else if (p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                /* see if the new character will fit */
                data=TxtboxGetData(G_TxtboxArray[G_currentTextBox]);
                tempstr=MemAlloc(strlen(data)+4);
                sprintf (tempstr,"%sWW",data);
                if (TxtboxCanFit (G_TxtboxArray[G_currentTextBox],tempstr)==strlen(tempstr))
                {
                    TxtboxAppendKey(G_TxtboxArray[G_currentTextBox],scankey);
                }
                MemFree(tempstr);
            }
        }
        else if (scankey==9)
        {
            /* we've got a tab, figure out what to do with it */
            switch (p_Txtbox->mode)
            {
                /* in these modes the tab key will advance a field */
                case Txtbox_MODE_EDIT_FIELD:
                case Txtbox_MODE_FIXED_WIDTH_FIELD:
                case Txtbox_MODE_VIEW_SCROLL_FORM:
                case Txtbox_MODE_SELECTION_BOX:
                if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_SHIFT)==TRUE ||
                    KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_SHIFT)==TRUE)
                {
                    /* shift is being held, back up a field */
                    TxtboxLastBox();
                }
                else
                {
                    TxtboxNextBox();
                }
                p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[G_currentTextBox];
                G_currentAction = Txtbox_ACTION_GAINED_FOCUS;
                break;

                /* in these modes, ctrl-shift is needed to advance a field */
                case Txtbox_MODE_EDIT_FORM:
                if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_CTRL)==TRUE ||
                    KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_CTRL)==TRUE)
                {
                    /* ctrl is being pushed, move to the next or last field */
                    if (KeyboardGetScanCode(KEY_SCAN_CODE_LEFT_SHIFT)==TRUE ||
                        KeyboardGetScanCode(KEY_SCAN_CODE_RIGHT_SHIFT)==TRUE)
                    {
                        /* shift is being held, back up a field */
                        TxtboxLastBox();  \
                    }
                    else
                    {
                        TxtboxNextBox();
                    }
                    p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[G_currentTextBox];
                    G_currentAction = Txtbox_ACTION_GAINED_FOCUS;
                }
                else  /* ctrl is not being held, add a tab to the field */
                {
                    TxtboxAppendKey(G_TxtboxArray[G_currentTextBox],scankey);
                    G_currentAction = Txtbox_ACTION_DATA_CHANGED;
                }
                break;
                default:
                break;
            }
        }
        else if (scankey==13)
        {
            /* we've got a return here, figure out what to do with it */
            switch (p_Txtbox->mode)
            {
                /* in these modes the return key will advance a field */
                case Txtbox_MODE_EDIT_FIELD:
                case Txtbox_MODE_FIXED_WIDTH_FIELD:
                case Txtbox_MODE_VIEW_SCROLL_FORM:
                G_currentAction = Txtbox_ACTION_ACCEPTED;
                if (p_Txtbox->Txtboxcallback != NULL)
                {
                    p_Txtbox->Txtboxcallback (G_TxtboxArray[G_currentTextBox]);
                }

//                TxtboxNextBox();
                p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[G_currentTextBox];
                G_currentAction = Txtbox_ACTION_GAINED_FOCUS;
                break;

                /* in these modes, return is sent as a character */
                case Txtbox_MODE_EDIT_FORM:
                TxtboxAppendKey(G_TxtboxArray[G_currentTextBox],scankey);
                G_currentAction = Txtbox_ACTION_DATA_CHANGED;
                default:
                break;
            }
        }
        case KEYBOARD_EVENT_HELD:
        break ;

        case KEYBOARD_EVENT_PRESS:

        if (scankey==KEY_SCAN_CODE_LEFT)
        {
            if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM ||
                p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                TxtboxCursLeft (G_TxtboxArray[G_currentTextBox]);
                G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
            }
        }
        else if (scankey==KEY_SCAN_CODE_RIGHT)
        {
            if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM ||
                p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                TxtboxCursRight (G_TxtboxArray[G_currentTextBox]);
                G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
            }
        }
        else if (scankey==KEY_SCAN_CODE_UP)
        {
            TxtboxCursUp (G_TxtboxArray[G_currentTextBox]);
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_DOWN)
        {
            TxtboxCursDn (G_TxtboxArray[G_currentTextBox]);
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_PGUP)
        {
            TxtboxCursPgUp (G_TxtboxArray[G_currentTextBox]);
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_PGDN)
        {
            TxtboxCursPgDn (G_TxtboxArray[G_currentTextBox]);
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_HOME)
        {
            if (KeyboardGetScanCode (KEY_SCAN_CODE_CTRL)==TRUE)
            {
               TxtboxCursTop (G_TxtboxArray[G_currentTextBox]);
            }
            else
            {
                TxtboxCursHome (G_TxtboxArray[G_currentTextBox]);
            }
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_END)
        {
            if (KeyboardGetScanCode (KEY_SCAN_CODE_CTRL)==TRUE)
            {
               TxtboxCursBot (G_TxtboxArray[G_currentTextBox]);
            }
            else
            {
                TxtboxCursEnd (G_TxtboxArray[G_currentTextBox]);
            }
            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        }
        else if (scankey==KEY_SCAN_CODE_DELETE)
        {
            if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM ||
                p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
            {
                /* make sure we have a character to delete */
                if (p_Txtbox->cursorl<strlen(p_Txtbox->data))
                {
                    /* move cursor right */
                    TxtboxCursRight (G_TxtboxArray[G_currentTextBox]);
                    /* backspace once */
                    TxtboxBackSpace (G_TxtboxArray[G_currentTextBox]);
                }
                G_currentAction = Txtbox_ACTION_DATA_CHANGED;
            }
        }
        break;

        default:
        break;
    }

    if (p_Txtbox->Txtboxcallback != NULL)
    {
        p_Txtbox->Txtboxcallback (G_TxtboxArray[G_currentTextBox]);
    }

    DebugEnd();
}


T_void TxtboxMouseControl (E_mouseEvent event, T_word16 x, T_word16 y, T_buttonClick button)
{
    T_TxtboxStruct *p_Txtbox,*p_Txtbox2;
    static T_TxtboxID *selected,*selectedsb;
    T_sword16 row;
    T_word16 i;

    DebugRoutine ("TxtboxMouseControl");


    switch (event)
    {
        case MOUSE_EVENT_START:
        selectedsb=NULL;
        selected=NULL;

        for (i=0;i<MAX_TxtboxES;i++)
        {
            if (G_TxtboxArray[i]!=NULL)
            {
                p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[i];
                if (p_Txtbox->sbgrID!=NULL)
                {
                    if (GraphicIsAt(p_Txtbox->sbgrID,x,y))
                    {
                        TxtboxMoveSB (G_TxtboxArray[i],y);
                        /* set selected scroll bar for dragging */
                        selectedsb=G_TxtboxArray[i];
                        G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
                        if (p_Txtbox->Txtboxcallback != NULL) p_Txtbox->Txtboxcallback (G_TxtboxArray[i]);
                    }
                }

                if (TxtboxIsAt(G_TxtboxArray[i],x,y))
                {
                    if (p_Txtbox->mode<Txtbox_MODE_VIEW_NOSCROLL_FORM ||
                        p_Txtbox->mode>Txtbox_MODE_VIEW_NOSCROLL_FORM)
                    {
                        /* select this text box */
                        p_Txtbox2=(T_TxtboxStruct *)G_TxtboxArray[G_currentTextBox];
                        p_Txtbox2->isselected=FALSE;
                        G_currentAction = Txtbox_ACTION_LOST_FOCUS;
                        if (p_Txtbox2->Txtboxcallback != NULL) p_Txtbox2->Txtboxcallback (G_TxtboxArray[G_currentTextBox]);

                        if (G_TxtboxArray[G_currentTextBox] != NULL)
                          TxtboxUpdate (G_TxtboxArray[G_currentTextBox]);
                        p_Txtbox->isselected=TRUE;
                        G_currentTextBox=i;
                        G_currentAction = Txtbox_ACTION_GAINED_FOCUS;
                        if (p_Txtbox->Txtboxcallback != NULL) p_Txtbox->Txtboxcallback (G_TxtboxArray[i]);

                        /* set selected */
                        selected=G_TxtboxArray[i];

                        /* move cursor to selected row or character */
                        switch (p_Txtbox->mode)
                        {
                            case Txtbox_MODE_EDIT_FORM:
                            case Txtbox_MODE_FIXED_WIDTH_FIELD:
                            case Txtbox_MODE_EDIT_FIELD:
                            /* determine row, col pointed at */
                            row=(y-p_Txtbox->ly1)/p_Txtbox->fontheight;
                            p_Txtbox->cursorline=p_Txtbox->windowstartline+row;
                            if (p_Txtbox->cursorline>p_Txtbox->totalrows) p_Txtbox->cursorline=p_Txtbox->totalrows;
                            p_Txtbox->cursorl=TxtboxScanRow (G_TxtboxArray[i],0,x);
                            break;

                            case Txtbox_MODE_SELECTION_BOX:
                            G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
                            /* determine the row pointed at */
                            row=(y-p_Txtbox->ly1)/p_Txtbox->fontheight;
                            p_Txtbox->cursorline=p_Txtbox->windowstartline+row;
                            if (p_Txtbox->cursorline>p_Txtbox->totalrows)
                              p_Txtbox->cursorline=p_Txtbox->totalrows;
                            p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
                            /* notify callback */
                            if (p_Txtbox->Txtboxcallback != NULL) p_Txtbox->Txtboxcallback (G_TxtboxArray[i]);
                            break;

                            default:
                            break;
                        }

                        if (G_TxtboxArray[i]!=NULL) TxtboxUpdate (G_TxtboxArray[i]);
                        break;
                    }
                }
            }
        }
        break;

        case MOUSE_EVENT_DRAG:
        case MOUSE_EVENT_HELD:
        if (selected != NULL)
        {
            if (!TxtboxValidateID (selected))
            {
                selected=NULL;
                break;
            }

            p_Txtbox=(T_TxtboxStruct *)selected;
            switch (p_Txtbox->mode)
            {
                case Txtbox_MODE_EDIT_FORM:
                case Txtbox_MODE_EDIT_FIELD:
                case Txtbox_MODE_FIXED_WIDTH_FIELD:
                break;

                case Txtbox_MODE_SELECTION_BOX:

                if (y < p_Txtbox->ly1)
                {
                    if (p_Txtbox->cursorline > p_Txtbox->windowstartline)
                      p_Txtbox->cursorline=p_Txtbox->windowstartline;

                    if (p_Txtbox->cursorline > 0)
                    {
                        p_Txtbox->cursorline--;
                        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
                        if (p_Txtbox->windowstartline > p_Txtbox->cursorline)
                          p_Txtbox->windowstartline --;
                    } else
                    {
                        p_Txtbox->cursorline=0;
                        p_Txtbox->cursorl=0;
                        if (p_Txtbox->windowstartline > 0) p_Txtbox->windowstartline--;
                    }
                }
                else if (y>p_Txtbox->ly2)
                {
                    if (p_Txtbox-> cursorline < p_Txtbox->windowstartline + p_Txtbox->windowrows+1)
                    {
                        p_Txtbox->cursorline = p_Txtbox->windowstartline + p_Txtbox->windowrows+1;
                        if (p_Txtbox->cursorline > p_Txtbox->totalrows) p_Txtbox->cursorline=p_Txtbox->totalrows;
                        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
                    }

                    if (p_Txtbox-> cursorline < p_Txtbox->totalrows)
                    {
                        p_Txtbox->cursorline++;
                        p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
                    }
                }
                else
                {
                    row=(y-p_Txtbox->ly1)/p_Txtbox->fontheight;
                    p_Txtbox->cursorline=p_Txtbox->windowstartline+row;
                    if (p_Txtbox->cursorline>p_Txtbox->totalrows)
                       p_Txtbox->cursorline=p_Txtbox->totalrows;
                    p_Txtbox->cursorl=p_Txtbox->linestarts[p_Txtbox->cursorline];
                }

                TxtboxUpdate (selected);
                /* notify callback */
                G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
                if (p_Txtbox->Txtboxcallback != NULL) p_Txtbox->Txtboxcallback (selected);
                break;

                default:
                break;
            }
        }

        if (selectedsb != NULL)
        {
            if (!TxtboxValidateID(selectedsb)) selectedsb=NULL;
            else
            /* move selected scroll bar */
            TxtboxMoveSB (selectedsb,y);
        }

        break;

        case MOUSE_EVENT_IDLE:
        default:
        selected=NULL;
        selectedsb=NULL;

        break;
    }
    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  TxtboxNextBox/TxtboxLastBox
 *-------------------------------------------------------------------------*/
/**
 *  Increments or decrements G_currentTextBox
 *
 *<!-----------------------------------------------------------------------*/
T_void TxtboxNextBox (T_void)
{
	T_word16 oldTxtbox;
	T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxNextBox");
	oldTxtbox=G_currentTextBox;
	/* turn off the cursor of the old text field */
	p_Txtbox=(T_TxtboxStruct*)G_TxtboxArray[oldTxtbox];

    /* report lost focus */
    G_currentAction = Txtbox_ACTION_LOST_FOCUS;

    if (p_Txtbox->Txtboxcallback != NULL)
    {
        p_Txtbox->Txtboxcallback (G_TxtboxArray[G_currentTextBox]);
    }

	p_Txtbox->isselected=FALSE;
	/* force redraw the old text field so the cursor goes away*/
	TxtboxUpdate (G_TxtboxArray[oldTxtbox]);
    /* increment the pointer to the active box */
	G_currentTextBox++;
	if (G_currentTextBox>=MAX_TxtboxES) G_currentTextBox=0;
	/* scan through Txtboxarray list until a valid Txtbox is found */
	/* or we pass the original point */
	while (G_TxtboxArray[G_currentTextBox]==NULL)
	{
		G_currentTextBox++;
		if (G_currentTextBox>=MAX_TxtboxES) G_currentTextBox=0;
		if (G_currentTextBox==oldTxtbox) break; /* we looped around */
	}

    /* skip this field if it is not editable */
    p_Txtbox=(T_TxtboxStruct*)G_TxtboxArray[G_currentTextBox];

    DebugEnd();

    if (p_Txtbox->mode==Txtbox_MODE_VIEW_NOSCROLL_FORM)
    {
        TxtboxNextBox();
        /* recursive call */
    } else
    {
        p_Txtbox->isselected=TRUE;
        TxtboxRepaginateAll (G_TxtboxArray[G_currentTextBox]);
        TxtboxUpdate (G_TxtboxArray[G_currentTextBox]);
    }
}


T_void TxtboxLastBox (T_void)
{
	T_word16 oldTxtbox;
	T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxLastBox");

	oldTxtbox=G_currentTextBox;
	/* turn off the cursor of the old text field */
	p_Txtbox=(T_TxtboxStruct*)G_TxtboxArray[oldTxtbox];

    /* report lost focus */
    G_currentAction = Txtbox_ACTION_LOST_FOCUS;

    if (p_Txtbox->Txtboxcallback != NULL)
    {
        p_Txtbox->Txtboxcallback (G_TxtboxArray[G_currentTextBox]);
    }

	p_Txtbox->isselected=FALSE;
	/* force redraw the old text field so the cursor goes away*/
	TxtboxUpdate (G_TxtboxArray[oldTxtbox]);
    /* increment the pointer to the active box */
	G_currentTextBox--;
	if (G_currentTextBox>=MAX_TxtboxES) G_currentTextBox=MAX_TxtboxES;
	/* scan through Txtboxarray list until a valid Txtbox is found */
	/* or we pass the original point */
	while (G_TxtboxArray[G_currentTextBox]==NULL)
	{
		G_currentTextBox--;
		if (G_currentTextBox>=MAX_TxtboxES) G_currentTextBox=MAX_TxtboxES;
		if (G_currentTextBox==oldTxtbox) break; /* we looped around */
	}

    /* skip this field if it is not editable */
    p_Txtbox=(T_TxtboxStruct*)G_TxtboxArray[G_currentTextBox];

    DebugEnd();

    if (p_Txtbox->mode==Txtbox_MODE_VIEW_NOSCROLL_FORM)
    {
        TxtboxLastBox();
        /* recursive call */
    } else
    {
        p_Txtbox->isselected=TRUE;
        TxtboxRepaginateAll (G_TxtboxArray[G_currentTextBox]);
        TxtboxUpdate (G_TxtboxArray[G_currentTextBox]);
    }
}


T_void TxtboxFirstBox (T_void)
{
    T_TxtboxStruct *p_Txtbox;
    T_word16 i;

    DebugRoutine ("TxtboxFirstBox");

    for (i=0;i<MAX_TxtboxES;i++)
    {
        if (G_TxtboxArray[i] != NULL)
        {
            p_Txtbox=(T_TxtboxStruct *)G_TxtboxArray[i];
            if (p_Txtbox->mode != Txtbox_MODE_VIEW_NOSCROLL_FORM)
            {
                p_Txtbox->isselected=TRUE;
                G_currentTextBox=i;
                break;
            }
        }
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  TxtboxUpdate
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_void TxtboxUpdate (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_graphicStruct *p_graphic;

    DebugRoutine ("TxtboxUpdate");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    p_graphic=(T_graphicStruct *)p_Txtbox->p_graphicID;

    p_graphic->changed=TRUE;

    DebugEnd();
}


T_void TxtboxDrawCallBack(T_graphicID graphicID, T_word16 index)
{
    T_word32 i,j,k;
    T_word16 wsize;
    T_word16 curposx, curposy;
    T_word16 startline,endline;
    T_word32 loopstart,loopend;
    T_word16 newcolor;
    T_TxtboxStruct *p_Txtbox;
    T_TxtboxID TxtboxID;
    T_bitfont *p_font;
    T_byte8 bcolor1, bcolor2, pcolor;
    T_byte8 tempstr[10];
    E_Boolean cursordrawn=FALSE;
    T_byte8 linespassed=0;

    DebugRoutine ("TxtboxDrawCallBack");
    DebugCheck (graphicID != NULL);

    TxtboxID=G_TxtboxArray[index];
    DebugCheck (TxtboxID != NULL);
    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* highlight if selected */
    if (p_Txtbox->isselected==TRUE)
    {
        bcolor1=p_Txtbox->hbordercolor1;
        bcolor2=p_Txtbox->hbordercolor2;
        pcolor=p_Txtbox->hbackcolor;
    } else
    {
        bcolor1=p_Txtbox->bordercolor1;
        bcolor2=p_Txtbox->bordercolor2;
        pcolor=p_Txtbox->backcolor;
    }

    /* allocate a temporary drawing field */
//    tempscreen=GrScreenAlloc();
//    oldscreen=GrScreenGet();
//    GrScreenSet (tempscreen);
//    GraphicDrawToCurrentScreen();
    /* first, draw the background box */
    GrDrawRectangle (p_Txtbox->lx1,
                     p_Txtbox->ly1,
                     p_Txtbox->lx2,
                     p_Txtbox->ly2,
                     pcolor);

    /* draw window border striping */
    GrDrawHorizontalLine (p_Txtbox->lx1,
                          p_Txtbox->ly1,
                          p_Txtbox->lx2-1,
                          bcolor1);

    GrDrawVerticalLine   (p_Txtbox->lx1,
                          p_Txtbox->ly1,
                          p_Txtbox->ly2-1,
                          bcolor1);

    GrDrawHorizontalLine (p_Txtbox->lx1+1,
                          p_Txtbox->ly2,
                          p_Txtbox->lx2,
                          bcolor2);

    GrDrawVerticalLine   (p_Txtbox->lx2,
                          p_Txtbox->ly1+1,
                          p_Txtbox->ly2,
                          bcolor2);

    /* open the font */
    p_font = ResourceLock(p_Txtbox->font) ;
	GrSetBitFont (p_font);

    wsize=GrGetCharacterWidth ('W');

    /* check to make sure the cursor is in the window */
    if (p_Txtbox->cursorline < p_Txtbox->windowstartline)
    {
        p_Txtbox->windowstartline=p_Txtbox->cursorline;
    } else if (p_Txtbox->cursorline >= (p_Txtbox->windowstartline+p_Txtbox->windowrows))
    {
        p_Txtbox->windowstartline++;
    }

    startline=p_Txtbox->windowstartline;
    endline=p_Txtbox->windowstartline+p_Txtbox->windowrows;
    if (endline>p_Txtbox->totalrows) endline=p_Txtbox->totalrows;

//    bcolor1=p_Txtbox->textcolor;
//    bcolor2=p_Txtbox->textshadow;

    /* figure out our last color */
    if (startline < endline) {
        for (i=0;i<p_Txtbox->linestarts[startline];i++)
        {
            if (p_Txtbox->data[i]>128)
            {
                newcolor=p_Txtbox->data[i]-128;
                if (newcolor < MAX_EXTENDED_COLORS)
                {
                    newcolor=G_extendedColors[newcolor];
                    p_Txtbox->textcolor=(T_byte8)newcolor;
                    p_Txtbox->htextcolor=(T_byte8)newcolor;
                 }
            }
        }
    } else {
        newcolor=G_extendedColors[1]; // white
        p_Txtbox->textcolor=(T_byte8)newcolor;
        p_Txtbox->htextcolor=(T_byte8)newcolor;
    }

    /* loop through the data lines, drawing each line */
    for (i=startline; i<=endline; i++)
    {
        /* set the y cursor start position */
        curposy=p_Txtbox->ly1+(i-startline)*p_Txtbox->fontheight;

        /* set the x cursor start position */
        if (p_Txtbox->justify==Txtbox_JUSTIFY_CENTER)
        {
            curposx=((p_Txtbox->lx2-p_Txtbox->lx1)/2)-(p_Txtbox->linewidths[i]/2)+p_Txtbox->lx1;
        } else
        {
            curposx=p_Txtbox->lx1;
        }

        /* loop through each character in the line, drawing as we go */
        loopstart=p_Txtbox->linestarts[i];
        if (i+1>p_Txtbox->totalrows) loopend=strlen(p_Txtbox->data);
        else loopend=p_Txtbox->linestarts[i+1];

        /* set the color if selection box */
//        if (p_Txtbox->mode==Txtbox_MODE_SELECTION_BOX && i==p_Txtbox->cursorline)
//        {
//            p_Txtbox->textcolor=47;
//            p_Txtbox->textshadow=53;
//        }

        if (i==endline) loopend++;

        for (j=loopstart;j<loopend;j++)
        {

            if (curposy+p_Txtbox->fontheight > p_Txtbox->ly2) break;

            /* draw the cursor if here */
            if (p_Txtbox->cursorl==j)
            {
                if (p_Txtbox->mode==Txtbox_MODE_SELECTION_BOX)
                {
                    /* draw a highlighted line */
                    GrDrawRectangle (p_Txtbox->lx1+1,
                                     curposy+1,
                                     p_Txtbox->lx2-1,
                                     curposy+p_Txtbox->fontheight,
                                     215);
                }
                else if (p_Txtbox->isselected==TRUE)
                {
                    if (p_Txtbox->mode<Txtbox_MODE_VIEW_SCROLL_FORM ||
                        p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD)
                    {
                        /* draw a cursor box */
                        for (k=0;k<p_Txtbox->fontheight;k++)
                        {
			                GrDrawHorizontalLine (curposx+1,
					                      curposy+k+1,
								          curposx+wsize,
								          226+k);
                        }
                    }
		        }
                p_Txtbox->cursorx=curposx;
                p_Txtbox->cursory=curposy;
                p_Txtbox->cursorline=i;
            }

            /* examine each character and draw */
            if (p_Txtbox->data[j]==9)
            {
                /* we have a tab here, advance to the nearest tab position */
                /* currently 3*wsize or 3 capital doubleyous :) */
                curposx=((((curposx-p_Txtbox->lx1)/(3*wsize))+1)*(3*wsize))+p_Txtbox->lx1;
            } else if (p_Txtbox->data[j]==13)
            {

            } else if (p_Txtbox->data[j]>31 && p_Txtbox->data[j]<128)
            {
                /* must be a normal character */
                tempstr[0]=p_Txtbox->data[j];
                tempstr[1]='\0';

                GrSetCursorPosition (curposx+1,curposy+1);

                if (j==p_Txtbox->cursorl && p_Txtbox->isselected==TRUE && (p_Txtbox->mode <= Txtbox_MODE_EDIT_FORM ||
                                                                           p_Txtbox->mode==Txtbox_MODE_FIXED_WIDTH_FIELD))
                {
                    /* cursor is over a character, change character color */
                    GrDrawShadowedText (tempstr,240,0);
                } else
                {
                    if (p_Txtbox->mode==Txtbox_MODE_SELECTION_BOX && i==p_Txtbox->cursorline)
                    {
                        GrDrawShadowedText (tempstr,29,0);
                    }
                    else if (p_Txtbox->isselected==TRUE)
                    {
                        GrDrawShadowedText (tempstr,p_Txtbox->htextcolor,p_Txtbox->textshadow);
                    } else GrDrawShadowedText (tempstr,p_Txtbox->textcolor,p_Txtbox->textshadow);
                }
                curposx+=GrGetCharacterWidth (p_Txtbox->data[j]);
            } else if (p_Txtbox->data[j]>128)
            {
                newcolor=p_Txtbox->data[j]-128;
                if (newcolor < MAX_EXTENDED_COLORS)
                {
                    newcolor=G_extendedColors[newcolor];
                    p_Txtbox->textcolor=(T_byte8)newcolor;
                    p_Txtbox->htextcolor=(T_byte8)newcolor;
                }
            }
        }

        /* restore the text colors */
//        p_Txtbox->textcolor=bcolor1;
//        p_Txtbox->textshadow=bcolor2;
    }

    /* close the font */
    /* close the font */
    ResourceUnlock (p_Txtbox->font);

    /* copy the contents of the temporary screen area to the visual one */
//    GrTransferRectangle (oldscreen,
//                         p_Txtbox->lx1,
//                         p_Txtbox->ly1,
//                         p_Txtbox->lx2,
//                         p_Txtbox->ly2,
//                         p_Txtbox->lx1,
//                         p_Txtbox->ly1);

//    GrScreenSet (oldscreen);
//    GrScreenFree (tempscreen);
//    GraphicDrawToActualScreen();
    /* update the scroll bar if applicable */
    if (p_Txtbox->sbgrID != NULL)
    {
        TxtboxUpdateSB (TxtboxID);
    }

    DebugEnd();
}



T_void TxtboxRepaginateAll (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_word16 templn;

    DebugRoutine ("TxtboxRepaginateAll");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    templn=p_Txtbox->cursorline;
    p_Txtbox->cursorline=0;
    TxtboxRepaginate (TxtboxID);
    p_Txtbox->cursorline=templn;

    DebugEnd();
}


T_void TxtboxRepaginate (T_TxtboxID TxtboxID)
{
    T_word16 i,j;
    T_word16 wsize;
    T_word16 linecnt;
    T_word16 curposx;
    T_TxtboxStruct *p_Txtbox;
    T_bitfont *p_font;
    static T_word16 cnt=0;
    T_word32 end,tcurposx;

    DebugRoutine ("TxtboxRepaginate");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    /* open the font */
    p_font = ResourceLock(p_Txtbox->font) ;
	GrSetBitFont (p_font);

    /* set the tab size using the character 'w' */

    wsize=GrGetCharacterWidth ('W');
    curposx=p_Txtbox->lx1;

    linecnt=p_Txtbox->cursorline;
    p_Txtbox->linestarts[0]=0;

    /* now format the data string */
    for (i=p_Txtbox->linestarts[p_Txtbox->cursorline];i<=strlen(p_Txtbox->data);i++)
    {
        /* examine each character and draw */
        if (p_Txtbox->data[i]==13)
        {
            /* we got a return here, advance a line */
            p_Txtbox->linewidths[linecnt]=curposx-p_Txtbox->lx1;
            curposx=p_Txtbox->lx1;
            if (linecnt >= p_Txtbox->totalrows) TxtboxAllocLine (TxtboxID);
            p_Txtbox->linestarts[++linecnt]=i+1;

        } else if (p_Txtbox->data[i]==9)
        {
            /* we have a tab here, advance to the nearest tab position */
            /* currently 3*wsize or 3 capital doubleyous :) */
            curposx=((((curposx-p_Txtbox->lx1)/(3*wsize))+1)*(3*wsize))+p_Txtbox->lx1;
            /* make sure we haven't reached the end of the line */
            if (curposx+wsize>p_Txtbox->lx2)
            {
                /* out of room, advance a line */
                p_Txtbox->linewidths[linecnt]=curposx-p_Txtbox->lx1;
                curposx=p_Txtbox->lx1;
                if (linecnt >= p_Txtbox->totalrows) TxtboxAllocLine (TxtboxID);
                p_Txtbox->linestarts[++linecnt]=i+1;

            }
        }
        else if (p_Txtbox->data[i]>31 && p_Txtbox->data[i]<128)
        {   /* normal character */
            curposx+=GrGetCharacterWidth (p_Txtbox->data[i]);
            if (curposx+wsize>p_Txtbox->lx2)
            {
                /* reached end of line, traverse backwards until we
                   find a space */
                if (linecnt>0) end=p_Txtbox->linestarts[linecnt-1];
                else end=0;

                tcurposx=curposx;

                for (j=i;j>end;j--)
                {
                    if (p_Txtbox->data[j]==32 || p_Txtbox->data[j]==9)
                    {
                        /* found a space, do it. */
                        break;
                    } else
                    {
                        tcurposx-=GrGetCharacterWidth (p_Txtbox->data[j]);
                        if (tcurposx<wsize)
                        {
                            j=end;
                            break;
                        }
                    }
                }
                if (j==end) /* wow, can't do it */
                {
                    p_Txtbox->linewidths[linecnt]=curposx-p_Txtbox->lx1;
                    curposx=p_Txtbox->lx1;
                    if (linecnt >= p_Txtbox->totalrows) TxtboxAllocLine (TxtboxID);
                    p_Txtbox->linestarts[++linecnt]=i+1;
                } else
                {
                    p_Txtbox->linewidths[linecnt]=tcurposx-p_Txtbox->lx1;
                    curposx=p_Txtbox->lx1;
                    if (linecnt >= p_Txtbox->totalrows) TxtboxAllocLine (TxtboxID);
                    p_Txtbox->linestarts[++linecnt]=j+1;
                    i=j+1;
                }
            }
        }
    }

    p_Txtbox->linewidths[linecnt]=curposx-p_Txtbox->lx1;
    p_Txtbox->totalrows=linecnt;

    /* close the font */
    ResourceUnlock (p_Txtbox->font);

    DebugEnd();
}


T_void TxtboxSetScrollBarObjIDs (T_TxtboxID TxtboxID,
                                 T_buttonID sbupID,
                                 T_buttonID sbdnID,
                                 T_graphicID sbgrID)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxSetScrollBarObjIDs");
    DebugCheck (TxtboxID != NULL);
    DebugCheck (sbupID != NULL);
    DebugCheck (sbdnID != NULL);
    DebugCheck (sbgrID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    p_Txtbox->sbupID=sbupID;
    p_Txtbox->sbdnID=sbdnID;
    p_Txtbox->sbgrID=sbgrID;

    DebugEnd();
}


T_void TxtboxHandleSBDn (T_buttonID buttonID)
{
    T_TxtboxID TxtboxID;
    T_buttonStruct *p_button;

    DebugRoutine ("TxtboxHandleSBDn");
    DebugCheck (buttonID != NULL);

    p_button=(T_buttonStruct *)buttonID;
    TxtboxID=FormGetObjID(p_button->data);
    DebugCheck (TxtboxID != NULL);
    TxtboxCursDn (TxtboxID);
    DebugEnd();
}


T_void TxtboxHandleSBUp (T_buttonID buttonID)
{
    T_TxtboxID TxtboxID;
    T_buttonStruct *p_button;

    DebugRoutine ("TxtboxHandleSBUp");
    DebugCheck (buttonID != NULL);

    p_button=(T_buttonStruct *)buttonID;
    TxtboxID=FormGetObjID(p_button->data);
    DebugCheck (TxtboxID != NULL);
    TxtboxCursUp (TxtboxID);
    DebugEnd();
}


T_void TxtboxUpdateSB (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_graphicStruct *p_graphic;

    T_word16 sbx1,sby1,sbx2,sby2;
    float ystart,ylength,yleft;
    float ratio;

    DebugRoutine ("TxtboxUpdateSB");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    DebugCheck (p_Txtbox->sbgrID != NULL);

   /* allocate a temporary drawing field */
//    oldscreen=GrScreenGet();
//    tempscreen=GrScreenAlloc();
//    GrScreenSet (tempscreen);
//    GraphicDrawToCurrentScreen();
    /* force redraw of the scroll bar graphic */
    p_graphic=(T_graphicStruct *)p_Txtbox->sbgrID;
    p_graphic->changed=TRUE;
    GraphicUpdate (p_Txtbox->sbgrID);

    /* get the loci of the scroll bar graphic */
    sbx1=p_graphic->locx+1;
    sbx2=p_graphic->locx+p_graphic->width-2;
    sby1=p_graphic->locy+1;
    sby2=p_graphic->locy+p_graphic->height-2;

    /* figure the percentage of the total document that is visible */
    if (p_Txtbox->totalrows < p_Txtbox->windowrows)
    {
        /* fully visible */
        ylength=(float)(sby2-sby1);
        ystart=sby1;
    } else
    {
        ratio=(float)p_Txtbox->windowrows/(float)(p_Txtbox->totalrows+1);

        /* partially visible document */
        ylength=(((float)(sby2-sby1))*ratio);

        yleft=(sby2-sby1)-ylength;
        if (p_Txtbox->mode==Txtbox_MODE_VIEW_SCROLL_FORM)
        {
            ratio=((float)p_Txtbox->windowstartline)/((float)p_Txtbox->totalrows+1-p_Txtbox->windowrows);
            ystart=(yleft*ratio)+sby1;
        }
        else
        {
            ratio=((float)p_Txtbox->cursorline)/((float)p_Txtbox->totalrows+1);
            ystart=(yleft*ratio)+sby1;
        }
    }

    /* draw the rectangle */
    GrDrawRectangle (sbx1,(int)ystart,sbx2,(int)ystart+(int)ylength,68);
    GrDrawFrame (sbx1,(int)ystart,sbx2,(int)ystart+(int)ylength,66);
    GrDrawHorizontalLine (sbx1,(int)ystart,sbx2,70);
    GrDrawVerticalLine (sbx1,(int)ystart,(int)ystart+(int)ylength,70);

    /* set structure records */
    p_Txtbox->sbstart=(int)ystart;
    p_Txtbox->sblength=(int)ylength;

    /* copy the contents of the temporary screen area to the visual one */
//    GrTransferRectangle (oldscreen,
//                         sbx1,
//                         sby1,
//                         sbx2,
//                         sby2,
//                         sbx1,
//                         sby1);

    /* restore buffer */
//    GrScreenSet (oldscreen);
//    GrScreenFree (tempscreen);
//    GraphicDrawToActualScreen();
    DebugEnd();
}


T_void TxtboxMoveSB (T_TxtboxID TxtboxID, T_word16 y)
{
    T_TxtboxStruct *p_Txtbox;
    T_graphicStruct *p_graphic;
    T_word16 center;
    T_word16 cnt;

    DebugRoutine ("TxtboxMoveSB");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    p_graphic=(T_graphicStruct *)p_Txtbox->sbgrID;

    if (y<p_Txtbox->sbstart) TxtboxCursPgUp (TxtboxID);
    else if (y>p_Txtbox->sbstart+p_Txtbox->sblength) TxtboxCursPgDn (TxtboxID);
    else
    {
        /* calculate center of scroll bar */
        center=p_Txtbox->sbstart+(p_Txtbox->sblength/2);
        if (y>center+1)
        {
            cnt=0;
            while (y>center+2)
            {
                cnt++;
                TxtboxCursDn (TxtboxID);
                TxtboxUpdateSB (TxtboxID);
                center=p_Txtbox->sbstart+(p_Txtbox->sblength/2);
                if (cnt>5) break;
            }
        }
        else if (y<center-1)
        {
            cnt=0;
            while (y<center-2)
            {
                cnt++;
                TxtboxCursUp (TxtboxID);
                TxtboxUpdateSB (TxtboxID);
                center=p_Txtbox->sbstart+(p_Txtbox->sblength/2);
                if (cnt>5) break;
            }
        }

        G_currentAction = Txtbox_ACTION_SELECTION_CHANGED;
        if (p_Txtbox->Txtboxcallback != NULL) p_Txtbox->Txtboxcallback (TxtboxID);

//          pratio=((float)(y-(p_Txtbox->sbstart)))/p_Txtbox->sblength;
//          pline=(int)(pratio*(float)p_Txtbox->totalrows);

        /* calc the center of the scroll bar */
//        center=p_Txtbox->sbstart+(p_Txtbox->sblength/2);

        /* calc the space around the edges */
//        spaceleft=(p_graphic->height-2) - p_Txtbox->sblength;

        /* calculate the distance moved */
//        dy=y-center;

        /* calc the new sb center */
//        center+=dy;

        /* limit the movement */
//        if (center - (p_Txtbox->sblength/2) < p_graphic->locy+1)
//          center=p_graphic->locy+(p_Txtbox->sblength/2);
//        else if ((center + p_Txtbox->sblength/2) > p_graphic->locy+p_graphic->height-1)
//          center=p_graphic->locy+p_graphic->height-1-(p_Txtbox->sblength/2);

        /* distance from top edge of sb graphic to top edge of sb is our ratio */
//        pratio=((center-(p_Txtbox->sblength/2))-(p_Txtbox->ly1+1))/spaceleft;
//        pline=(int)(pratio*(float)p_Txtbox->totalrows);

//
/*        switch (p_Txtbox->mode)
        {
            case Txtbox_MODE_EDIT_FIELD:
            case Txtbox_MODE_EDIT_FORM:

            break;

            case Txtbox_MODE_VIEW_SCROLL_FORM:
            case Txtbox_MODE_SELECTION_BOX:

                 p_Txtbox->cursorline=pline;
                 p_Txtbox->cursorl=p_Txtbox->linestarts[pline];
                 TxtboxUpdate (TxtboxID);
            break;

            default:
            break;
        }
*/
//        if (pline < p_Txtbox->cursorline) TxtboxCursUp (TxtboxID);
//        else TxtboxCursDn (TxtboxID);
    }

    /* force redraw */
    TxtboxUpdateSB (TxtboxID);

    DebugEnd();
}


T_void TxtboxAllocLine (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_word32 *newlinestarts;
    T_word16 *newlinewidths;
    T_word16 i;

    DebugRoutine ("TxtboxAllocLine");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    DebugCheck (p_Txtbox->linewidths != NULL);
    DebugCheck (p_Txtbox->linestarts != NULL);
    DebugCheck (p_Txtbox->data != NULL);

    p_Txtbox->totalrows++;

    /* allocate a new chunk for linewidths / linestarts */
    newlinestarts=MemAlloc (sizeof(T_word32)*(p_Txtbox->totalrows+1));
    MemCheck (311);
    DebugCheck (newlinestarts != NULL);

    newlinewidths=MemAlloc (sizeof(T_word16)*(p_Txtbox->totalrows+1));
    MemCheck (312);
    DebugCheck (newlinewidths != NULL);

    /* copy old data */
    for (i=0;i<p_Txtbox->totalrows;i++)
    {
        newlinestarts[i]=p_Txtbox->linestarts[i];
        newlinewidths[i]=p_Txtbox->linewidths[i];
    }
    newlinestarts[i]=0;
    newlinewidths[i]=0;

    /* delete old data */
    MemFree (p_Txtbox->linewidths);
    MemCheck (313);
    p_Txtbox->linewidths=newlinewidths;

    MemFree (p_Txtbox->linestarts);
    MemCheck (314);
    p_Txtbox->linestarts=newlinestarts;

    DebugEnd();
}


T_void TxtboxSetCallback (T_TxtboxID TxtboxID, T_TxtboxHandler newcallback)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxSetCallBack");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    p_Txtbox->Txtboxcallback = newcallback;

    DebugEnd();
}

T_void TxtboxSetNumericOnlyFlag (T_TxtboxID TxtboxID, E_Boolean newflag)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxSetNumericOnlyFlag");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    p_Txtbox->numericonly=newflag;

    DebugEnd();
}


E_TxtboxAction TxtboxGetAction (T_void)
{
    return (G_currentAction);
}

E_Boolean TxtboxValidateID (T_TxtboxID TxtboxID)
{
    T_word16 i;
    E_Boolean retvalue=FALSE;

    DebugRoutine ("TxtboxValidateID");
    for (i=0;i<MAX_TxtboxES;i++)
    {
        if (TxtboxID==G_TxtboxArray[i])
        {
            retvalue=TRUE;
            break;
        }
    }

    DebugEnd();
    return (retvalue);
}


T_word16 TxtboxGetSelectionNumber (T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_word16 retvalue;

    DebugRoutine ("TxtboxGetSelectionNumber");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    DebugCheck (p_Txtbox->mode==Txtbox_MODE_SELECTION_BOX);

    retvalue=p_Txtbox->cursorline;

    DebugEnd();

    return (retvalue);
}


E_Boolean TxtboxIsSelected (T_TxtboxID TxtboxID)
{
   T_TxtboxStruct *p_Txtbox;
   E_Boolean retvalue;

   DebugRoutine ("TxtboxIsSelected");
   DebugCheck (TxtboxID != NULL);

   p_Txtbox=(T_TxtboxStruct *)TxtboxID;

   retvalue=p_Txtbox->isselected;
   DebugEnd();
   return (retvalue);
}


T_void TxtboxSetMaxLength (T_TxtboxID TxtboxID,T_word32 newmaxlen)
{
    T_TxtboxStruct *p_Txtbox;

    DebugRoutine ("TxtboxSetMaxLength");
    DebugCheck (TxtboxID != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;

    p_Txtbox->maxlength=newmaxlen;

    DebugEnd();
}

/* routine will take stringToFit and return n where n is the number of */
/* characters from the left that will fit in TxtboxID's field width */
T_word16 TxtboxCanFit (T_TxtboxID TxtboxID, T_byte8 *stringToFit)
{
    T_word16 widthcnt=0;
    T_word16 boxwidth=0;
    T_word16 charcnt=0;
    T_word16 length;
    T_TxtboxStruct *p_Txtbox;
    DebugRoutine ("TxtboxCanFit");
    DebugCheck (TxtboxID != NULL);
    DebugCheck (stringToFit != NULL);

    p_Txtbox=(T_TxtboxStruct *)TxtboxID;
    boxwidth=p_Txtbox->lx2-p_Txtbox->lx1 - (GrGetCharacterWidth('W')+3);
    length=strlen(stringToFit);
/*
    printf ("string=%d\n",stringToFit);
    printf ("length=%d\n",length);
    printf ("boxwidth=%d\n",boxwidth);
*/
    while ((widthcnt < boxwidth) &&
           (charcnt < length))
    {
        widthcnt+=GrGetCharacterWidth(stringToFit[charcnt++]);
    }


    DebugEnd();
//    if (charcnt > 0) return (charcnt-1);
    return (charcnt);
}



T_void *TxtboxGetStateBlock(T_void)
{
    T_TxtboxID *p_Txtboxs;

    p_Txtboxs = MemAlloc(sizeof(G_TxtboxArray)) ;
    DebugCheck(p_Txtboxs != NULL) ;
    memcpy(p_Txtboxs, G_TxtboxArray, sizeof(G_TxtboxArray)) ;
    memset(G_TxtboxArray, 0, sizeof(G_TxtboxArray)) ;

    return p_Txtboxs ;
}

T_void TxtboxSetStateBlock(T_void *p_state)
{
    memcpy(G_TxtboxArray, p_state, sizeof(G_TxtboxArray)) ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  TXTBOX.C
 *-------------------------------------------------------------------------*/
