/****************************************************************************/
/*    FILE:  COLOR.C                                                        */
/****************************************************************************/
#include "COLOR.H"
#include "CONFIG.H"
#include "GENERAL.H"
#include "GRAPHICS.H"
#include "INIFILE.H"
#include "SOUND.H"
#include "TICKER.H"

static T_sword16 G_rval,G_gval,G_bval;
static T_sword16 G_rfilt, G_gfilt, G_bfilt;
static T_byte8 G_colorvals[768];

static T_word16 G_gamma ;

/****************************************************************************/
/*  Routine:  ColorInit                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorInit inits variables used by routines in color.c                  */
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
/*  ColorStoreDefaultPalette                                                                        */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorInit (T_void)
{
    T_iniFile iniFile ;
    T_byte8 *buffer ;

	DebugRoutine ("ColorInit");

	G_rval=0;
	G_gval=0;
	G_bval=0;
	G_rfilt=0;
	G_gfilt=0;
	G_bfilt=0;
    iniFile = ConfigGetINIFile() ;
    buffer = INIFileGet(iniFile, "video", "gamma") ;
    if (buffer)
        G_gamma = atoi(buffer) ;
    else
        G_gamma = 0 ;
	ColorStoreDefaultPalette();

	DebugEnd();
}



/****************************************************************************/
/*  Routine:  ColorStoreDefaultPalette                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorStoreDefaultPalette stores the current palette for use with other  */
/*  routines in color.c                                                     */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorStoreDefaultPalette (T_void)
{
    T_byte8 p[256][3];
	DebugRoutine ("ColorStoreDefaultPalette");

	GrGetPalette (0, 256, p);

    memcpy(&G_colorvals, p, sizeof(T_palette));
//    *((T_palette *)&G_colorvals) = *p;

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  ColorAddGlobal                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorAddGlobal adds an amount of R,G,B to all values in the palette.    */
/*  Used to indicate damage or effects in the game.                         */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorAddGlobal (T_sword16 red, T_sword16 green, T_sword16 blue)
{
	DebugRoutine ("ColorAddGlobal");

	G_rval+=red;
	G_gval+=green;
	G_bval+=blue;

	if (G_rval>63) G_rval=63;
	if (G_gval>63) G_gval=63;
	if (G_bval>63) G_bval=63;
	if (G_rval<-63) G_rval=-63;
	if (G_gval<-63) G_gval=-63;
	if (G_bval<-63) G_bval=-63;

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  ColorSetGlobal                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorAddGlobal sets an amount of R,G,B to all values in the palette.    */
/*  Used to indicate damage or effects in the game.                         */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorSetGlobal (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue)
{
	DebugRoutine ("ColorSetGlobal");

	G_rval=red;
	G_gval=green;
	G_bval=blue;

	if (G_rval>63) G_rval=63;
	if (G_gval>63) G_gval=63;
	if (G_bval>63) G_bval=63;
	if (G_rval<-63) G_rval=-63;
	if (G_gval<-63) G_gval=-63;
	if (G_bval<-63) G_bval=-63;

	DebugEnd();
}

/****************************************************************************/
/*  Routine:  ColorAddFilt                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorAddFilt adds an amount of r,g,b to a 'filter' value that affects   */
/*  all colors in the palette.                                              */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorAddFilt (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue)
{
	DebugRoutine ("ColorAddGlobal");

	G_rfilt+=red;
	G_gfilt+=green;
	G_bfilt+=blue;

	if (G_rfilt>63) G_rfilt=63;
	if (G_gfilt>63) G_gfilt=63;
	if (G_bfilt>63) G_bfilt=63;
	if (G_rfilt<-63) G_rfilt=-63;
	if (G_gfilt<-63) G_gfilt=-63;
	if (G_bfilt<-63) G_bfilt=-63;

	DebugEnd();
}



/****************************************************************************/
/*  Routine:  ColorAddFilt                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorAddFilt sets the filter to an amount of r,g,b - filter affects     */
/*  all colors in the palette.                                              */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void ColorSetFilt (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue)
{
	DebugRoutine ("ColorSetFilt");

	G_rfilt=red;
	G_gfilt=green;
	G_bfilt=blue;

	if (G_rfilt>63) G_rfilt=63;
	if (G_gfilt>63) G_gfilt=63;
	if (G_bfilt>63) G_bfilt=63;
	if (G_rfilt<-63) G_rfilt=-63;
	if (G_gfilt<-63) G_gfilt=-63;
	if (G_bfilt<-63) G_bfilt=-63;

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  ColorResetFilt                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorResetFilt resets the value of the filter to 0,0,0                  */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void ColorResetFilt (T_void)
{
	DebugRoutine ("ColorResetFilt");

	G_rfilt=0;
	G_gfilt=0;
	G_bfilt=0;
	DebugEnd();
}

/****************************************************************************/
/*  Routine:  ColorUpdate                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorUpdate moves the entire palette spectrum from current colors       */
/*  towards colors stored in ColorStoreDefaultPalette.                      */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorUpdate (T_word16 delta)
{
	T_byte8 tempcolors[768];
	T_word16 i;
	T_sword16 tempr,tempg,tempb;
	static T_sword32 glowupdatetime=0;
    static T_word32 lastTime = 0 ;

	DebugRoutine ("ColorUpdate") ;
    INDICATOR_LIGHT(933, INDICATOR_GREEN) ;

    delta = TickerGet() - lastTime ;
    lastTime = TickerGet() ;

    if (delta)
        SoundUpdateOften() ;

	if (G_rval<0)
	{
		G_rval+=delta;
		if (G_rval>0) G_rval=0;
	}

	else if (G_rval>0)
	{
		G_rval-=delta;
		if (G_rval<0) G_rval=0;
	}

	if (G_bval<0)
	{
		G_bval+=delta;
		if (G_bval>0) G_bval=0;
	}

	else if (G_bval>0)
	{
		G_bval-=delta;
		if (G_bval<0) G_bval=0;
	}

	if (G_gval<0)
	{
		G_gval+=delta;
		if (G_gval>0) G_gval=0;
	}

	else if (G_gval>0)
	{
		G_gval-=delta;
		if (G_gval<0) G_gval=0;
	}

	glowupdatetime+=delta;
	if (glowupdatetime>=8)
	{
		ColorGlowUpdate();
		glowupdatetime=0;
	 }
    INDICATOR_LIGHT(936, INDICATOR_GREEN) ;
	for (i=0;i<768;i+=3)
	{
/* TESTING!!! Color is getting in the way */
#ifdef OPTIONS_COLORON
   	    tempr=G_colorvals[i]+G_rval+G_rfilt+G_gamma;
		tempg=G_colorvals[i+1]+G_gval+G_gfilt+G_gamma;
		tempb=G_colorvals[i+2]+G_bval+G_bfilt+G_gamma;
#else
		tempr=G_colorvals[i] ;
		tempg=G_colorvals[i+1] ;
		tempb=G_colorvals[i+2] ;
#endif
		if (tempr<0) tempr=0;
		if (tempg<0) tempg=0;
		if (tempb<0) tempb=0;
		if (tempr>63) tempr=63;
		if (tempg>63) tempg=63;
		if (tempb>63) tempb=63;

		tempcolors[i]=(T_byte8)tempr;
		tempcolors[i+1]=(T_byte8)tempg;
		tempcolors[i+2]=(T_byte8)tempb;
	 }
    INDICATOR_LIGHT(936, INDICATOR_RED) ;
    INDICATOR_LIGHT(939, INDICATOR_GREEN) ;

	GrSetPalette (0,256, (T_palette *)&tempcolors);

    INDICATOR_LIGHT(939, INDICATOR_RED) ;

    INDICATOR_LIGHT(933, INDICATOR_RED) ;
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  ColorGlowUpdate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorGlowUpdate updates the 'glow colors' defined in the palette        */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/13/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorGlowUpdate (T_void)
{
	static int glows[3]={20,20,20};
	static int cycle[8]={0,0,0,0,0,0,0,0};
	static int cyclecnt=0;
	static int gv[3]={1,2,3};
	int i;
	int colorval;
	static char timeflag=0;
	static int n1=0,n2=0,n3=0,n4=0,n5=0,n6=0,n7=0,n8=0,n9=0,n10=0,n11=0,n12=0;

	glows[0]+=gv[0];
	glows[1]+=gv[1];
	glows[2]+=gv[2];

	for (i=0;i<3;i++)
	{
		if (glows[i]>63)
		{
			glows[i]=63;
			gv[i]=-gv[i];
		}
		else if (glows[i]<20)
		{
			glows[i]=20;
			gv[i]=-gv[i];
		}
	}
	n1=(rand()&31)+32;
	n2=(rand()&31)+32;
	n3=(rand()&31)+32;
	n4=(rand()&31)+32;
	n5=(rand()&31)+32;
	n6=(rand()&31)+32;
	n7=(rand()&31)+32;
	n8=(rand()&31)+32;
	n9=rand()&63;

	if (n7==32)
	{
		n10=63;
	}

	if (n8==32)
	{
		n11=63;
	}

	if (n9==32)
	{
		n12=63;
	}

	for (i=0;i<8;i++)
	{
		colorval=(i+1)*6+23;
		cycle[cyclecnt]=colorval;
		cyclecnt++;
		if (cyclecnt>7) cyclecnt=0;
	}
	cyclecnt++;
	if (cyclecnt>7) cyclecnt=0;

	if (n10>0) n10-=4;
	if (n11>0) n11-=6;
	if (n12>0) n12-=4;
	if (n10<0) n10=0;
	if (n11<0) n11=0;
	if (n12<0) n12=0;

	ColorSetColor (226,cycle[0],0,0);
	ColorSetColor (227,cycle[1],0,0);
	ColorSetColor (228,cycle[2],0,0);
	ColorSetColor (229,cycle[3],0,0);
	ColorSetColor (230,cycle[4],0,0);
	ColorSetColor (231,cycle[5],0,0);
	ColorSetColor (232,cycle[6],0,0);
	ColorSetColor (233,cycle[7],0,0);

	ColorSetColor (234,n1			,0			,0);
	ColorSetColor (235,n2			,0			,0);
	ColorSetColor (236,n3			,0			,0);
	ColorSetColor (237,n4			,n4/2		,0);
	ColorSetColor (238,n5			,n5/2		,0);
	ColorSetColor (239,n7			,n7-10		,0);
	ColorSetColor (240,n8			,n8-10		,0);
	ColorSetColor (241,n1			,n1			,n1);
	ColorSetColor (242,0			,n4			,0);
	ColorSetColor (243,0			,0			,n5);

	ColorSetColor (244,n10			,n10		,0);
	ColorSetColor (245,n11			,n11		,n11);
	ColorSetColor (246,n12			,n12		,0);

	ColorSetColor (247,glows[0]		,glows[1]	,glows[2]);
	ColorSetColor (248,glows[2]		,glows[2]	,glows[2]);
	ColorSetColor (249,glows[0]		,0			,glows[0]);
	ColorSetColor (250,glows[0]     ,0          ,0);
	ColorSetColor (251,glows[2]		,0			,glows[0]);
	ColorSetColor (252,0			,glows[1]	,0);
	ColorSetColor (253,0			,glows[1]	,glows[2]);
	ColorSetColor (254,0			,0			,glows[2]);
}




/****************************************************************************/
/*  Routine:  ColorSetColor                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorSetColor permanently sets a color palette value to r,g,b           */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorSetColor (T_byte8 colornum, T_byte8 red, T_byte8 green, T_byte8 blue)
{
	static T_byte8 rgb[3]={0,0,0};

	DebugRoutine ("ColorSetColor");

	if (red>63) red=63;
	if (green>63) green=63;
	if (blue>63) blue=63;

	rgb[0]=red;
	rgb[1]=green;
	rgb[2]=blue;

	G_colorvals[colornum*3]=red;
	G_colorvals[colornum*3+1]=green;
	G_colorvals[colornum*3+2]=blue;

//  GrSetPalette (colornum,1,&rgb);
	DebugEnd();
}


T_byte8 ColorGetRed (T_byte8 colornum)
{
    return (G_colorvals[colornum*3]);
}

T_byte8 ColorGetGreen (T_byte8 colornum)
{
    return (G_colorvals[colornum*3+1]);
}

T_byte8 ColorGetBlue (T_byte8 colornum)
{
    return (G_colorvals[colornum*3+2]);
}



/****************************************************************************/
/*  Routine:  ColorRestore                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  ColorRestore restores the default palette.                              */
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
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/26/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ColorRestore (T_void)
{
    DebugRoutine ("ColorRestore");

	GrSetPalette (0, 256, (T_palette *)&G_colorvals);

    DebugEnd();
}

T_void  ColorFadeTo (T_byte8 red, T_byte8 green, T_byte8 blue)
{

	T_word16 i,j,k;
    DebugRoutine ("ColorFadeTo");

    /* get current palette */
    ColorStoreDefaultPalette();

    /* loop and change palette */
    for (i=0;i<31;i++)
    {
        for (j=0;j<768;j+=3)
        {
            for (k=0;k<2;k++)
            {
                if (G_colorvals[j]<red) G_colorvals[j]++;
                else if (G_colorvals[j]>red) G_colorvals[j]--;

                if (G_colorvals[j+1]<green) G_colorvals[j+1]++;
                else if (G_colorvals[j+1]>green) G_colorvals[j+1]--;

                if (G_colorvals[j+2]<blue) G_colorvals[j+2]++;
                else if (G_colorvals[j+2]>blue) G_colorvals[j+2]--;
            }
        }
	    GrSetPalette (0,256, (T_palette *)&G_colorvals);
        delay (2);
    }

    DebugEnd();
}

T_word16 ColorGammaAdjust(T_void)
{
    T_iniFile iniFile ;
    T_byte8 buffer[10] ;

    G_gamma += 2 ;
    if (G_gamma >= 16)
        G_gamma = 0 ;

    iniFile = ConfigGetINIFile() ;
    sprintf(buffer, "%d", G_gamma) ;
    INIFilePut(iniFile, "video", "gamma", buffer) ;

    return G_gamma/2 ;
}
