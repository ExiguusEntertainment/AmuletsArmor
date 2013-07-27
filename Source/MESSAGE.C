/****************************************************************************/
/*    FILE:  MESSAGE.C                                                      */
/****************************************************************************/
//#include "standard.h"
#include <stdarg.h>
#include "DEBUG.H"
#include "GENERAL.H"
#include "MESSAGE.H"
#include "MEMORY.H"
#include "PICS.H"
#include "TXTBOX.H"

#define MAX_NUM_MESSAGES 50
#define MAX_SIZE_MESSAGE 70
#define MAX_VIEWED_MESSAGES 4

static E_Boolean G_alternateOutput=FALSE;
static T_TxtboxID G_altOutputBox = NULL;

T_byte8 G_extendedColors[MAX_EXTENDED_COLORS] =
  {0,   /* 000 = black */
   31,  /* 001 = white */
   16,  /* 002 = gray */
   160, /* 003 = bt yellow */
   164, /* 004 = dk yellow */
   144, /* 005 = bt red */
   148, /* 006 = dk red */
   211, /* 007 = lt brown (default text on display Txtboxes) */
   82,  /* 008 = dk brown */
   128, /* 009 = lt green */
   132, /* 010 = dk green */
   177, /* 011 = lt blu */
   182, /* 012 = dk blu */
   34,  /* yellow to red = 13 - 17 */
   36,
   38,
   40,
   42,
   226, /* red glow = 18-25 */
   227,
   228,
   229,
   230,
   231,
   232,
   233,
   234, /* red flicker = 26-28 */
   235,
   236,
   237, /* orange flicker = 29 - 30 */
   238,
   99, /* yellow flicker = 31 - 32 */
   240,
   241, /* white flicker = 33*/
   242, /* greem flicker = 34*/
   243, /* blue flicker  = 35*/
   247, /* blue glow     = 36*/
   248, /* white glow   = 37*/
   208, /* platinum color= 38*/
   99 /* copper color  = 39*/
};

/* Here is a place to store the messages. */
static T_byte8 G_Messages[MAX_NUM_MESSAGES][MAX_SIZE_MESSAGE+1] ;

/* Note how many messages there are. */
static T_word16 G_numMessages = 0 ;

/* Keep track of where in the list we are (start point) */
static T_word16 G_currentMessage = 0 ;

/* Use pointers to keep the actual message list.  This keeps us from */
/* having to copy large chunks of data when scrolling.  We just move */
/* the pointers around. */
static T_byte8 *P_Messages[MAX_NUM_MESSAGES] ;

/* Keep track of where in the real message list we are when adding */
/* new messages.  */
static T_word16 G_realMessage = 0 ;

/****************************************************************************/
/*  Routine:  MessageScrollUp                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MessageScrollUp scrolls up the message display by one line.           */
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
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MessageScrollUp(T_void)
{
    DebugRoutine("MessageScrollUp") ;

    /* Scroll the message up one line. */
    if (G_currentMessage > 0)
        G_currentMessage-- ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MessageScrollDown                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MessageScrollDown moves one item down the message list.               */
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
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MessageScrollDown(T_void)
{
    DebugRoutine("MessageScrollDown") ;

    if ((G_currentMessage+1) < G_numMessages)
        G_currentMessage++ ;

    DebugEnd() ;
}

T_void MessageDrawLine(
        T_word16 x_pos,
        T_word16 y_pos,
        T_byte8 *line,
        T_color color)
{
    T_word16 j;
    T_word16 len;
    char c;
    T_byte8 val;

    len = strlen((char *)line);
    for (j = 0; j < len; j++) {
        c = line[j];
        if (c == '^') {
            /* skip caret */
            j++;

            /* get color value */
            val = 0;
            val += ((line[j++] - '0') * 100);
            if (j < len)
                val += ((line[j++] - '0') * 10);
            if (j < len)
                val += ((line[j] - '0'));

            DebugCheck(val < MAX_EXTENDED_COLORS);
            if (val >= MAX_EXTENDED_COLORS)
                val = MAX_EXTENDED_COLORS - 1;

            /* change color */
            color = G_extendedColors[val];
        } else {
            GrSetCursorPosition(x_pos + 1, y_pos + 1);
            GrDrawCharacter(c, 0);
            GrSetCursorPosition(x_pos, y_pos);
            GrDrawCharacter(c, color);
            x_pos += GrGetCharacterWidth(c);
        }
    }
}

/****************************************************************************/
/*  Routine:  MessageDraw                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MessageDraw draws MAX_VIEWED_MESSAGES of message lines on the         */
/*  currently active screen at given x and y coordinate.  The amount of     */
/*  space from line to line is also given by an interleave variable.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    It is assumed that the currently active screen is where the messages  */
/*  need to be drawn.                                                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 x                  -- Starting x to draw                     */
/*                                                                          */
/*    T_word16 y                  -- Starting y to draw                     */
/*                                                                          */
/*    T_word16 interleave         -- How far down to next line              */
/*                                                                          */
/*    T_color color               -- Color of text                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    GrSetCursorPosition                                                   */
/*    GrDrawShadowedText                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MessageDraw(
           T_word16 x,
           T_word16 y,
           T_word16 interleave,
           T_color color)
{
    T_word16 i ;
    T_word16 current ;
    T_word16 y_pos;
    T_word16 x_pos;
    T_byte8 ncolor;
    T_resourceFile res;
    T_resource font;
    T_bitfont *p_font;

    DebugRoutine("MessageDraw") ;

    /* lock in font */
    res = ResourceOpen ("sample.res");
    font=ResourceFind (res,"FontTiny");
    p_font=ResourceLock (font);
    GrSetBitFont (p_font);

    /* Loop through up to MAX_VIEWED_MESSAGES, but stop if we reach the */
    /* end of the message list.  Keep track of the current message and */
    /* also the position on the screen. */
    for (i=0, current=G_currentMessage, y_pos = y;
         (i<MAX_VIEWED_MESSAGES) && (current < G_numMessages);
         i++, current++, y_pos+=interleave)
    {
        /* draw each character, checking for embedded color controls */
        x_pos=0;
        ncolor=G_extendedColors[7];
        MessageDrawLine(x_pos, y_pos, P_Messages[current], ncolor);
    }

#if 0
        if (P_Messages[current][0]=='^')
        {
            /* special color imbedded in string */
            val=0;
            val+=((P_Messages[current][1]-'0')*100);
            val+=((P_Messages[current][2]-'0')*10);
            val+=((P_Messages[current][3]-'0'));

            if (val < MAX_EXTENDED_COLORS)
            {
                ncolor = G_extendedColors[val];
            }
            else DebugCheck (0);
            /* hide color code chars */
            temp+=4;
        }
        GrSetCursorPosition(x, y_pos) ;
        GrDrawShadowedText(temp, ncolor, COLOR_BLACK) ;
#endif

    /* unlock the font */
    ResourceUnlock (font);
    ResourceClose (res);

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MessageAdd                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MessageAdd appends a text line to the bottom of the message list.     */
/*  If needed, the messages are scroll up to make room (and one message     */
/*  is scroll off the list).                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_string           -- String to add to message list          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memmove                                                               */
/*    strcpy                                                                */
/*    MessageScrollDown                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MessageAdd(T_byte8 *p_string)
{
    DebugRoutine("MessageAdd") ;
//    DebugCheck(strlen(p_string) <= MAX_SIZE_MESSAGE) ;

    if (G_alternateOutput==TRUE)
    {
        /* spit output to alternate textbox */
        TxtboxSetData(G_altOutputBox,p_string);
    }

    /* First see if we need to scroll up the messages. */
    if (G_numMessages == MAX_NUM_MESSAGES)  {
        /* Yes, we need to scroll. */
        /* Scroll up the messages. */
        memmove(
            P_Messages,
            P_Messages+1,
            sizeof(P_Messages)-sizeof(T_byte8 *)) ;

        /* Note that we are one message less now. */
        G_numMessages-- ;
    }

    /* Add the new message. */
    /* Get a pointer to next message slot. */
    P_Messages[G_numMessages] = G_Messages[G_realMessage++] ;

    /* Copy the message into the slot. */
    strncpy(P_Messages[G_numMessages], p_string, MAX_SIZE_MESSAGE-1) ;

    /* Add one to the total number of messages. */
    G_numMessages++ ;

    /* If the real message location is past the end, wrap around. */
    if (G_realMessage == MAX_NUM_MESSAGES)
        G_realMessage = 0 ;

    /* If the new message is out of our view, scroll down one to try */
    /* to follow it. */
    if (((T_sword16)(G_numMessages-G_currentMessage)) > MAX_VIEWED_MESSAGES)
        MessageScrollDown() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MessageClear                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MessageClear clears all the messages in the list and starts with      */
/*  a fresh and emtpy list.  This is useful when some people are tired      */
/*  of seeing the messages.                                                 */
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
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MessageClear(T_void)
{
    DebugRoutine("MessageClear") ;

    /* Luckily, we don't have to do anything to clear out the messages */
    /* but change the number of messages in the list and where we are. */
    G_numMessages = 0 ;
    G_currentMessage = 0 ;
    G_realMessage = 0 ;

    DebugEnd() ;
}



/****************************************************************************/
/*  Routine:  MessageDisplayMessage                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  MessageDisplayMessage searches for a text file in the resource          */
/*  identified by MESSAGES/MSG##### where ##### is the message number       */
/*  to display.                                                             */
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
/*    JDA  01/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void MessageDisplayMessage (T_word16 messagenum)
{
    T_byte8 *desc1;
    T_byte8 *desc2;
    T_resource res;
    T_word32 size;
    T_byte8 stmp[32];
    DebugRoutine ("MessageDisplayMessage");

    sprintf (stmp,"MESSAGES/MSG%05d.TXT",messagenum);
    DebugCheck (PictureExist(stmp));

    if (PictureExist(stmp))
    {
        /* show description file */
        desc1=PictureLockData (stmp,&res);
        size=ResourceGetSize(res);
        desc2=(T_byte8 *)MemAlloc(size+2);
        memcpy (desc2,desc1,size);
        desc2[size-1]='\0';
        MessageAdd (desc2);
        MemFree (desc2);
        PictureUnlockAndUnfind(res) ;
    }

    DebugEnd();
}

int MessagePrintf( char *fmt, ... )
{
  va_list  argptr;			/* Argument list pointer	*/
  char str[140];			/* Buffer to build sting into	*/
  int cnt;				/* Result of SPRINTF for return */

  va_start( argptr, fmt );		/* Initialize va_ functions	*/

  cnt = vsprintf( str, fmt, argptr );	/* prints string to buffer	*/
  MessageAdd(str) ;

  va_end( argptr );			/* Close va_ functions		*/

  return( cnt );			/* Return the conversion count	*/

}

/* routine enables 'alternate output' -- will open up a textbox */
/* in the view area and force output to it.  Currently only used */
/* for store / bank / inn ui screens */
T_void MessageSetAlternateOutputOn(T_void)
{
    DebugRoutine ("MessageSetAlternateOutputOn");

    if (G_alternateOutput==FALSE)
    {
        /* create alternate textbox */
        G_altOutputBox = TxtboxCreate  (OUTPUT_BOX_X1,
                                        OUTPUT_BOX_Y1,
                                        OUTPUT_BOX_X2,
                                        OUTPUT_BOX_Y2,
                                        "FontMedium",
                                        0,
                                        0,
                                        FALSE,
                                        Txtbox_JUSTIFY_CENTER,
                                        Txtbox_MODE_VIEW_NOSCROLL_FORM,
                                        NULL);

        TxtboxSetData(G_altOutputBox,"");
        G_alternateOutput=TRUE;
    }

    DebugEnd();
}

/* routine disables 'alternate output' */
T_void MessageSetAlternateOutputOff(T_void)
{
    DebugRoutine ("MessageSetAlternateOutputOff");

    if (G_alternateOutput==TRUE)
    {
        G_alternateOutput=FALSE;
        TxtboxDelete(G_altOutputBox);
    }

    DebugEnd();
}


/****************************************************************************/
/*    END OF FILE:  MESSAGE.C                                               */
/****************************************************************************/
