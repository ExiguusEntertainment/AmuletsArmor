/****************************************************************************/
/*    FILE:  COLOR.C                                                        */
/****************************************************************************/

#include "standard.h"
static T_sword16 G_rval,G_gval,G_bval;
static T_sword16 G_rfilt, G_gfilt, G_bfilt;
static T_byte8 G_colorvals[768];


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
	DebugRoutine ("ColorInit");

	G_rval=0;
	G_gval=0;
	G_bval=0;
	G_rfilt=0;
	G_gfilt=0;
	G_bfilt=0;
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
	DebugRoutine ("ColorStoreDefaultPalette");

	GrGetPalette (0,256,&G_colorvals);

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

T_void ColorAddGlobal (T_sbyte8 red, T_sbyte8 green, T_sbyte8 blue)
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
	static T_sword16 glowupdatetime=0;

	DebugRoutine ("ColorUpdate")

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
	if (glowupdatetime>8) //update glows every 8 ticks
	{
		ColorGlowUpdate();
		glowupdatetime-=8;
	}

	for (i=0;i<768;i+=3)
	{
		tempr=G_colorvals[i]+G_rval+G_rfilt;
		tempg=G_colorvals[i+1]+G_gval+G_gfilt;
		tempb=G_colorvals[i+2]+G_bval+G_bfilt;

		if (tempr<0) tempr=0;
		if (tempg<0) tempg=0;
		if (tempb<0) tempb=0;
		if (tempr>63) tempr=63;
		if (tempg>63) tempg=63;
		if (tempb>63) tempb=63;

		tempcolors[i]=tempr;
		tempcolors[i+1]=tempg;
		tempcolors[i+2]=tempb;
	 }

	GrSetPalette (0,256,&tempcolors);

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
	static T_sword16 glows[6]={0,0,0,0,0,0};
	static T_sword16 cycle[8]={0,0,0,0,0,0,0,0};
	static T_sword16 cyclecnt=0;
	static T_sbyte8 gv[3]={1,2,3};
	T_word16 i;
	T_sword16 colorval;

	DebugRoutine ("ColorGlowUpdate");

	glows[0]+=gv[0]; //these 'glows' bounce between 63 and 0
	glows[1]+=gv[1];
	glows[2]+=gv[2];
	glows[3]+=1;     //these cycle uni direction (0..63 then to 0)
	glows[4]+=2;
	glows[5]+=3;

	for (i=0;i<3;i++)
	{
		if (glows[i]>63)
		{
			glows[i]=63;
			gv[i]=-gv[i];
		}
		else if (glows[i]<0)
		{
			glows[i]=0;
			gv[i]=-gv[i];
		}
	}

	if (glows[3]>63) glows[3]=0;
	if (glows[4]>63) glows[4]=0;
	if (glows[5]>63) glows[5]=0;

	for (i=0;i<8;i++)            //full speed cycle[every 8 ticks]
	{
	  colorval=(i+1)*6+23;
	  //	if (colorval<63)
	  cycle[cyclecnt]=colorval;
/*		else
		{
			colorval=127-colorval;
			if (colorval<0) colorval=0;
			cycle[cyclecnt]=colorval;
		}          */
		cyclecnt++;
		if (cyclecnt>7) cyclecnt=0;
	}
	cyclecnt++;
	if (cyclecnt>7) cyclecnt=0;

//cycle[0]-[7] will contain a color cycle (sim. to Dpaint)

	ColorSetColor (2,glows[0],0,0);
	ColorSetColor (3,0,glows[1],0);
	ColorSetColor (4,0,0,glows[2]);
	ColorSetColor (5,glows[3],0,0);
	ColorSetColor (6,0,glows[4],0);
	ColorSetColor (7,0,0,glows[5]);
	ColorSetColor (8,cycle[0] ,0,0);
	ColorSetColor (9,cycle[1] ,0,0);
	ColorSetColor (10,cycle[2],0,0);
	ColorSetColor (11,cycle[3],0,0);
	ColorSetColor (12,cycle[4],0,0);
	ColorSetColor (13,cycle[5],0,0);
	ColorSetColor (14,cycle[6],0,0);
	ColorSetColor (15,cycle[7],0,0);

	DebugEnd();
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

	GrSetPalette (0,256,&G_colorvals);

    DebugEnd();
}
