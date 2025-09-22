/****************************************************************************/
/*    FILE:  BUTTON.C                                                       */
/****************************************************************************/

#include "standard.h"

static T_buttonID G_buttonarray[MAX_BUTTONS];

T_buttonID ButtonAdd (T_word16 lx,
                      T_word16 ly,
                      T_byte8 *bmname,
                      E_Boolean toggletype,
                      T_word16 keyassoc,
                      T_buttonHandler p_cbroutine)
{
    T_word16 i;

    DebugRoutine ("ButtonAdd");

    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]==NULL)  //add a button to list
        {
            G_buttonarray[i]=ButtonInit(lx,ly,bmname,toggletype,keyassoc,p_cbroutine);
            break;
        }
    }

    DebugCheck (i<MAX_BUTTONS);
    DebugEnd();
    return (G_buttonarray[i]);
}


T_buttonID ButtonInit (T_word16 lx,
                      T_word16 ly,
                      T_byte8 *bmname,
                      E_Boolean toggletype,
                      T_word16 keyassoc,
                      T_buttonHandler p_cbroutine)
{
    T_word32 size;
    T_buttonStruct *myID;
    T_byte8 *picptr;

    DebugRoutine ("ButtonInit");
    DebugCheck (lx<=320 && ly<=200);
    DebugCheck (bmname!=NULL);

    size=sizeof(T_buttonStruct);
    myID=(T_buttonID)MemAlloc(size);

    DebugCheck (myID!=NULL);
    if (myID!=NULL)
    {
        myID->buttonpic=PictureFind (bmname);
        picptr=PictureLockQuick (myID->buttonpic);
        PictureGetXYSize (picptr,&myID->width,&myID->height);

//        myID->width=15;
//        myID->height=11;
        PictureUnlock (myID->buttonpic);
        myID->locx=lx;
        myID->locy=ly;
        myID->toggle=toggletype;
        myID->pushed=FALSE;
        myID->p_callback=p_cbroutine;
        myID->enabled=TRUE;
        myID->changed=TRUE;
        myID->scancode=keyassoc;
    }
    DebugEnd();
    return (myID);
}


T_void ButtonDel (T_buttonID thisone)
{
    T_word16 i;

    DebugRoutine ("ButtonDel");
    DebugCheck (thisone != NULL);

    if (thisone!=NULL)
    {
        for (i=0;i<MAX_BUTTONS;i++)
        {
            if (G_buttonarray[i]==thisone) //found it, now kill it
            ButtonKill (thisone);
            break;
        }
    }

    DebugEnd();
}




T_void ButtonKill (T_buttonID buttonID)
{
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonKill");
    DebugCheck (buttonID != NULL);

    if (buttonID != NULL)
    {
        p_button = (T_buttonStruct *)buttonID ;
        MemFree (p_button);
    }

    DebugEnd();
}


T_void ButtonPush(T_buttonID buttonID)
{
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonPush");
    DebugCheck (buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID ;

    if (p_button->toggle==TRUE)
    {
        if (p_button->pushed==TRUE) p_button->pushed=FALSE;
        else p_button->pushed=TRUE;
        p_button->changed=TRUE;
    } else
    {
        if (p_button->pushed==FALSE)
        {
            p_button->pushed=TRUE;
            p_button->changed=TRUE;
        }
    }

    DebugEnd();
}


T_void ButtonPushOnKey (T_void)
{
    T_word16 keyscan;
    T_word16 i;
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonPushOnKey");
    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]!=NULL)
        {
            p_button = (T_buttonStruct *)G_buttonarray[i];
            keyscan= p_button->scancode;
            if (keyscan!=0)
            {
                if (KeyboardGetScanCode(keyscan)==TRUE)
                {
                  if ((p_button->pushed==FALSE) && (p_button->p_callback != NULL))
                  {
                    p_button->p_callback(G_buttonarray[i]);
                  }
				  ButtonDown (G_buttonarray[i]);
				  break;
                }
                else ButtonUp (G_buttonarray[i]);
            }
        }
    }

    DebugEnd();
}


T_void ButtonDown(T_buttonID buttonID)
{
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonDown");
    DebugCheck (buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID ;
    if (p_button->pushed==FALSE) p_button->changed=TRUE;
    p_button->pushed=TRUE;

    DebugEnd();
}


T_void ButtonUp(T_buttonID buttonID)
{
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonUp");
    DebugCheck (buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID ;
    if (p_button->pushed==TRUE) p_button->changed=TRUE;
    p_button->pushed=FALSE;

    DebugEnd();
}


T_void ButtonEnable (T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonEnable");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;
    if (p_button->enabled==FALSE) p_button->changed=TRUE;
    p_button->enabled=TRUE;

    DebugEnd();
}


T_void ButtonDisable (T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonDisable");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;
    if (p_button->enabled==TRUE) p_button->changed=TRUE;
    p_button->enabled=FALSE;

    DebugEnd();
}


T_void ButtonDoCallback (T_buttonID buttonID)
{
    T_buttonStruct *p_button ;

    DebugRoutine ("ButtonDoCallBack");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;

    if (p_button->p_callback != NULL) p_button->p_callback(buttonID);

    DebugEnd();
}


T_void ButtonUpdateAllButtons (T_void)
{
    T_word16 i;

    DebugRoutine ("ButtonUpdateAllButtons");

    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]!=NULL) ButtonUpdate(G_buttonarray[i]);
    }

    DebugEnd();
}


T_void ButtonDrawAllButtons (T_void)
{
    T_word16 i;

    DebugRoutine ("ButtonDrawAllButtons");

    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]!=NULL) ButtonDraw (G_buttonarray[i]);
    }

    DebugEnd();
}


T_void ButtonDraw (T_buttonID buttonID)
{
    T_byte8 *p_pic;
    T_bitmap *p_bitmap ;
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonDraw");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;
    p_pic=PictureLockQuick (p_button->buttonpic);
    p_bitmap = PictureToBitmap(p_pic) ;
    MouseHide();
    GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
    GrDrawRectangle (p_button->locx,p_button->locy,p_button->locx+p_button->width,p_button->locy+p_button->height,0);
    if (p_button->pushed)
    {
        GrDrawShadedBitmap (p_bitmap,p_button->locx+1,p_button->locy+1,160);
    }
    else
    {
        GrDrawBitmap (p_bitmap,p_button->locx,p_button->locy);
    }
    MouseShow();
    PictureUnlock (p_button->buttonpic);
    p_button->changed=FALSE;

    DebugEnd();
}


T_void ButtonDrawAt (T_buttonID buttonID, T_word16 lx, T_word16 ly)
{
    T_byte8 *p_pic;
    T_bitmap *p_bitmap ;
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonDrawAt");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;
    p_pic=PictureLockQuick (p_button->buttonpic);
    p_bitmap = PictureToBitmap(p_pic) ;
    MouseHide();
    GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
    GrDrawBitmap (p_bitmap,lx,ly);
    MouseShow();
    PictureUnlock (p_button->buttonpic);
    DebugEnd();
}


T_buttonID ButtonGetKey (T_word16 keycode)
{
    T_word16 i;
    T_buttonStruct *p_button;
    T_buttonID retvalue=NULL;

    DebugRoutine ("ButtonGetKey");

    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]!=NULL)
        {
            p_button=(T_buttonStruct *)G_buttonarray[i];
            if (p_button->scancode==keycode)
            {
                retvalue=G_buttonarray[i];
                break;
            }
        }
    }
    DebugEnd();

    return (retvalue);
}


T_buttonID ButtonGetLoc (T_word16 x, T_word16 y)
{
    T_word16 i;
    T_buttonStruct *p_button;
    T_buttonID retvalue=NULL;

    DebugRoutine ("ButtonGetLoc");

    for (i=0;i<MAX_BUTTONS;i++)
    {
        if (G_buttonarray[i]!=NULL)
        {
            if (ButtonIsAt (G_buttonarray[i],x,y))
            {
                retvalue=G_buttonarray[i];
                break;
            }
        }
    }
    DebugEnd();

    return (retvalue);
}


T_void ButtonUpdate (T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonUpdate");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;

    if (p_button->changed==TRUE) ButtonDraw (buttonID);

    DebugEnd();
}


T_void ButtonCleanUp (T_void)
{
    T_word16 i;

    DebugRoutine ("ButtonUpdate");
    for (i=0;i<MAX_BUTTONS;i++)
      if (G_buttonarray[i]!=NULL) ButtonKill (G_buttonarray[i]);
    DebugEnd();
}


E_Boolean ButtonIsPushed (T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonIsPushed");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;

    DebugEnd();
    return (p_button->pushed);
}


E_Boolean ButtonIsEnabled (T_buttonID buttonID)
{
    T_buttonStruct *p_button;

    DebugRoutine ("ButtonIsEnabled");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;

    DebugEnd();
    return (p_button->enabled);

}


E_Boolean ButtonIsAt (T_buttonID buttonID, T_word16 lx, T_word16 ly)
{
    T_buttonStruct *p_button;
    E_Boolean retvalue;

    DebugRoutine ("ButtonIsAt");
    DebugCheck (buttonID!=NULL);

    p_button = (T_buttonStruct *)buttonID ;

    if ( lx>=p_button->locx &&
         lx<=(p_button->locx+p_button->width) &&
         ly>=p_button->locy &&
         ly<=(p_button->locy+p_button->height)) retvalue=TRUE;
    else retvalue=FALSE;

    DebugEnd();
    return (retvalue);
}


