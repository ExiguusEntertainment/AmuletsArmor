/****************************************************************************/
/*    FILE:  SPELLS.C                                                       */
/****************************************************************************/

#include "standard.h"

static T_spellStruct G_spells[NUM_SPELLS];
static T_byte8 G_curspell[4]={0,0,0,0};

static T_sword16 G_beaconX=0;
static T_sword16 G_beaconY=0;
static T_word16  G_facing=0;
/****************************************************************************/
/*  Routine:  SpellsInitSpells                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  Initializes variables associated with spell casting.                    */
/*  Must be called prior to client login.                                   */
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
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsInitSpells (T_void)
{
	T_word16 i,j;
	T_byte8 scode[5];

	DebugRoutine ("SpellsInitSpells");

	for (i=0;i<NUM_SPELLS;i++)
	{
		for (j=0;j<4;j++) G_spells[i].code[j]=0;
		G_spells[i].duration=0;
		G_spells[i].ineffect=FALSE;
		G_spells[i].spellflag=NONE;
		G_spells[i].filtr=0;
		G_spells[i].filtg=0;
		G_spells[i].filtb=0;
		G_spells[i].iconpicptr=NULL;
		sprintf (G_spells[i].iconname,"#");
	}

	G_spells[0].code[0]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[0].code[1]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[0].spellfunction=SpellsLeap;

	G_spells[1].duration=500;
	G_spells[1].code[0]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[1].code[1]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[1].code[2]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[1].code[3]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[1].spellfunction=SpellsFast;
	sprintf (G_spells[1].iconname,"SCIC0001");

	G_spells[2].duration=500;
	G_spells[2].code[0]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[2].code[1]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[2].spellfunction=SpellsShockAbsorb;
	sprintf (G_spells[2].iconname,"SCIC0002");

	G_spells[3].code[0]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[3].code[1]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[3].code[2]=KEY_SCAN_CODE_KEYPAD_3;
	G_spells[3].code[3]=KEY_SCAN_CODE_KEYPAD_4;
	G_spells[3].spellfunction=SpellsHeal;

	G_spells[4].code[0]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[4].code[1]=KEY_SCAN_CODE_KEYPAD_1;
	G_spells[4].code[2]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[4].spellfunction=SpellsFireball;

	G_spells[5].duration=500;
	G_spells[5].code[0]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[5].code[1]=KEY_SCAN_CODE_KEYPAD_3;
	G_spells[5].spellflag=SPELL_WATER_WALK;
	G_spells[5].spellfunction=SpellsToggle;
	G_spells[5].filtr=20;
	sprintf (G_spells[5].iconname,"SCIC0003");

	G_spells[6].duration=500;
	G_spells[6].code[0]=KEY_SCAN_CODE_KEYPAD_3;
	G_spells[6].code[1]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[6].spellflag=SPELL_LAVA_WALK;
	G_spells[6].spellfunction=SpellsToggle;
	G_spells[6].filtb=20;
	sprintf (G_spells[6].iconname,"SCIC0004");

	G_spells[7].duration=1000;
	G_spells[7].code[0]=KEY_SCAN_CODE_KEYPAD_4;
	G_spells[7].code[1]=KEY_SCAN_CODE_KEYPAD_6;
	G_spells[7].code[2]=KEY_SCAN_CODE_KEYPAD_4;
	G_spells[7].code[3]=KEY_SCAN_CODE_KEYPAD_6;
	G_spells[7].spellflag=SPELL_INVULNERABLE;
	G_spells[7].spellfunction=SpellsToggle;
	G_spells[7].filtr=10;
	G_spells[7].filtg=10;
	G_spells[7].filtb=10;
	sprintf (G_spells[7].iconname,"SCIC0005");

	G_spells[8].duration=1000;
	G_spells[8].code[0]=KEY_SCAN_CODE_KEYPAD_9;
	G_spells[8].code[1]=KEY_SCAN_CODE_KEYPAD_9;
	G_spells[8].code[2]=KEY_SCAN_CODE_KEYPAD_9;
	G_spells[8].code[3]=KEY_SCAN_CODE_KEYPAD_9;
	G_spells[8].spellfunction=SpellsDuration;
	sprintf (G_spells[8].iconname,"SCIC0006");

	G_spells[9].duration=1000;
	G_spells[9].code[0]=KEY_SCAN_CODE_KEYPAD_8;
	G_spells[9].code[1]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[9].code[2]=KEY_SCAN_CODE_KEYPAD_8;
	G_spells[9].code[3]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[9].spellfunction=SpellsRegenerate;
	sprintf (G_spells[9].iconname,"SCIC0007");

	G_spells[10].duration=500;
	G_spells[10].code[0]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[10].code[1]=KEY_SCAN_CODE_KEYPAD_2;
	G_spells[10].spellflag=SPELL_LO_GRAV;
	G_spells[10].spellfunction=SpellsToggle;
	G_spells[10].filtg=10;
	sprintf (G_spells[10].iconname,"SCIC0008");

	G_spells[11].duration=500;
	G_spells[11].code[0]=KEY_SCAN_CODE_KEYPAD_3;
	G_spells[11].code[1]=KEY_SCAN_CODE_KEYPAD_3;
	G_spells[11].spellflag=SPELL_FEATHER_FALL;
	G_spells[11].spellfunction=SpellsToggle;
	G_spells[11].filtr=8;
	G_spells[11].filtg=8;
	sprintf (G_spells[11].iconname,"SCIC0009");

	G_spells[12].duration=1000;
	G_spells[12].code[0]=KEY_SCAN_CODE_KEYPAD_4;
	G_spells[12].code[1]=KEY_SCAN_CODE_KEYPAD_4;
	G_spells[12].spellflag=SPELL_JOUST_FLY;
	G_spells[12].spellfunction=SpellsToggle;
	G_spells[12].filtr=8;
	G_spells[12].filtb=8;
	sprintf (G_spells[12].iconname,"SCIC0010");

	G_spells[13].duration=1000;
	G_spells[13].code[0]=KEY_SCAN_CODE_KEYPAD_5;
	G_spells[13].code[1]=KEY_SCAN_CODE_KEYPAD_5;
	G_spells[13].spellflag=SPELL_AIR_WALK;
	G_spells[13].spellfunction=SpellsToggle;
	G_spells[13].filtr=-8;
	G_spells[13].filtb=-8;
	sprintf (G_spells[13].iconname,"SCIC0011");

	G_spells[14].duration=1000;
	G_spells[14].code[0]=KEY_SCAN_CODE_KEYPAD_6;
	G_spells[14].code[1]=KEY_SCAN_CODE_KEYPAD_6;
	G_spells[14].spellflag=SPELL_STICKY_FEET;
	G_spells[14].spellfunction=SpellsToggle;
	G_spells[14].filtr=-8;
	G_spells[14].filtg=-8;
	sprintf (G_spells[14].iconname,"SCIC0012");

	G_spells[15].code[0]=KEY_SCAN_CODE_KEYPAD_7;
	G_spells[15].code[1]=KEY_SCAN_CODE_KEYPAD_7;
	G_spells[15].spellfunction=SpellsBeaconSet;

	G_spells[16].code[0]=KEY_SCAN_CODE_KEYPAD_8;
	G_spells[16].code[1]=KEY_SCAN_CODE_KEYPAD_8;
	G_spells[16].spellfunction=SpellsBeaconReturn;


	for (i=0;i<NUM_SPELLS;i++)
	{
		if (G_spells[i].iconname[0]!='#')
		{
			G_spells[i].iconpicptr=PictureFind(G_spells[i].iconname);
		}
	}

	DebugEnd();

}

/****************************************************************************/
/*  Routine:  SpellsUpdateManaDisplay                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This routine updates the mana available display                         */
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
/*    GrDrawRectangle                                                       */
/*    GrDrawLine                                                            */
/*    StatsGetManaLeft                                                      */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsUpdateManaDisplay (T_void)
{
	T_sword16 Manaleft;
	T_sword16 i,j;
	const T_word16 dispx1=208;
	const T_word16 dispx2=260;
	const T_word16 dispy1=155;
	const T_word16 dispy2=178;
	const T_word16 deltax=260-208;
	const T_word16 deltay=178-155;
	static T_word16 updateon=0;
	static T_word16 plots[4][10];
	static E_Boolean firstin=TRUE;
	static T_word16 delta=0;
	static T_word32 last_update=0;
	T_sword16 r,g,b;
	T_word16 rnum;
	T_word16 color;
	T_word16 plotx1,ploty1,plotx2,ploty2;

	delta += (TickerGet() - last_update) ;
	last_update=TickerGet();
	if (delta<7) return;           //wait 7 ticks before updating.

	DebugRoutine ("SpellsUpdateManaDisplay");
	DebugCheck (firstin < BOOLEAN_UNKNOWN);

	delta=0;
	Manaleft=(StatsGetManaleft()+1)/100;
	if (Manaleft<1) Manaleft=1;


	if (firstin==TRUE) //initialize
	{
		firstin=FALSE;
		for (i=0;i<4;i++) for (j=0;j<10;j++) plots[i][j]=0;
//		ColorSetColor (MANA_BACKCOLOR,0,20,0);
		GrDrawRectangle (dispx1,dispy1,dispx2,dispy2,MANA_BACKCOLOR);
	}

	r=ColorGetRed (MANA_BACKCOLOR);
	g=ColorGetGreen (MANA_BACKCOLOR);
	b=ColorGetBlue(MANA_BACKCOLOR);

	if (r>0||g>20||b>0)
	{
		r-=2;b-=2;g-=2;
		if (r<0) r=0;
		if (b<0) b=0;
		if (g<20) g=20;
//		ColorSetColor (MANA_BACKCOLOR,r,g,b);
	}

	for (i=3;i>0;i--) for (j=0;j<10;j++) plots[i][j]=plots[i-1][j];

	for (j=0;j<10;j++)
	{
		rnum=rand();
		if (rnum%10>Manaleft) plots[0][j]+=(rnum%Manaleft+1);
		else plots[0][j]-=rnum%Manaleft;
        if (plots[0][j]>deltay) plots[0][j]=deltay;
	}

    for (i=3;i>=0;i--)
	{
		if (i==3) color=MANA_BACKCOLOR;
		else color=174-(i<<1)-Manaleft;
		for (j=1;j<9;j++)
		{
			plotx1=dispx1+((j-1)*6);
			plotx2=plotx1+9;
			ploty1=dispy1+plots[i][j-1];
			ploty2=dispy1+plots[i][j];
			GrDrawLine (plotx1,ploty1,plotx2,ploty2,color);
		}
	}

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsAddRune                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  Callback routine assigned to a rune button, adds rune to spell box      */
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
/*    T_buttonID (button ID which called this routine)                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    SpellsDrawRuneBox                                                     */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsAddRune (T_buttonID buttonID)
{
	T_buttonStruct *p_button;
	T_word16 i;
	T_word16 manasuck;

	DebugRoutine ("SpellsAddRune");
	DebugCheck (buttonID != NULL);

	if (G_curspell[3]!=0)
	{
//		ColorSetColor (MANA_BACKCOLOR,55,20,20);
	}
	else if (buttonID != NULL)
	{
		p_button=(T_buttonStruct *)buttonID;
		for (i=0;i<4;i++)
		{
			if (G_curspell[i]==0) //if slot is empty
			{
				manasuck=50;
				StatsChangeMana(-manasuck);
				if (StatsGetManaAvail()>0)
				{
//				  ColorSetColor (MANA_BACKCOLOR,0,50,0);
				  G_curspell[i]=p_button->scancode;   //add scancode to spell key
				  SpellsDrawRuneBox();
				}// else ColorSetColor (MANA_BACKCOLOR,55,20,20);
				break;
			}
		}
	}
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsClearRunes                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  Callback routine assigned to the rune clearbox button, clears runes     */
/*  in spell box                                                            */
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
/*    T_buttonID (button ID which called this routine)                      */
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
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsClearRunes (T_buttonID buttonID)
{
	T_word16 i;
	T_word16 num ;
	T_bitmap *pic ;
	T_resource res ;
	DebugRoutine ("SpellsClearRunes");

	for (i=0;i<4;i++) G_curspell[i]=0;

	pic = (T_bitmap *)PictureLockData("SPSTRIP", &res) ;
	DebugCheck(pic != NULL) ;

	if (pic != NULL)
	{
		GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
		GrDrawBitmap(pic, 176, 182) ;
		PictureUnlock(res) ;
	}

	DebugEnd();
}



/****************************************************************************/
/*  Routine:  SpellsBackSpace                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  Callback routine assigned to the rune backspace button, removes last    */
/*  tune entered.                                                           */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Currently same as SpellsClear                                         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_buttonID (button ID which called this routine)                      */
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
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Not Yet Functional                                     */
/*                                                                          */
/****************************************************************************/
T_void SpellsBackspace (T_buttonID buttonID)
{
	T_word16 i;
	T_word16 num ;
	T_bitmap *pic ;
	T_resource res ;
	DebugRoutine ("SpellsBackspace");

	for (i=0;i<4;i++) G_curspell[i]=0;

	pic = (T_bitmap *)PictureLockData("SPSTRIP", &res) ;
	DebugCheck(pic != NULL) ;

	if (pic != NULL)
	{
		GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
		GrDrawBitmap(pic, 176, 182) ;
		PictureUnlock(res) ;
	}

	DebugEnd();
}



/****************************************************************************/
/*  Routine:  SpellsDrawRuneBox                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This routine draws the selected icons for the current spell in the      */
/*  spell box                                                               */
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
/*    T_buttonID (button ID which called this routine)                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    GRDrawRectangle                                                       */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsDrawRuneBox (T_void)
{
	T_word16 i;
	T_buttonID runepic;

    DebugRoutine ("SpellsDrawRuneBox");

	for (i=0;i<4;i++)
	{
        if (G_curspell[i]!=0)
		{
			runepic=ButtonGetKey(G_curspell[i]);
			ButtonDrawAt (runepic,177+(i*16),183);
		}
	}


	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsCastSpell                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This routine attempts to cast the currently selected spell              */
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
/*    T_buttonID (button ID which called this routine)                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    GRDrawRectangle                                                       */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  05/30/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsCastSpell (T_buttonID buttonID)
{
	T_word16 i,j;
	E_Boolean success;

	DebugRoutine ("SpellsCastSpell");
//	if (G_curspell[0]!=0) ColorAddGlobal (0,0,30);

	for (i=0;i<NUM_SPELLS;i++)
	{
		success=TRUE;
		for (j=0;j<4;j++)
		{
			if (G_curspell[j]!=G_spells[i].code[j])
			{
				success=FALSE;
				break;
			}
		}

		if (success==TRUE)
		{
			if ((G_spells[i].spellfunction!=NULL) && (G_spells[i].ineffect==FALSE))
			{
				SpellsStart (&G_spells[i]);
			}
			break;
		}
	}

	SpellsClearRunes (buttonID);
	DebugEnd();
}



T_void SpellsStart (T_spellID spell)
{
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsStart");
	DebugCheck (spell != NULL);

	p_spell = (T_spellStruct *)spell;

	//start spell function
	p_spell->ineffect=TRUE;
	p_spell->spellfunction(p_spell);

	DebugEnd();
}



T_void SpellsStop (T_spellID spell)
{
	T_spellStruct *p_spell;

	DebugRoutine ("SpellsStop");
	DebugCheck (spell != NULL);

	p_spell = (T_spellStruct *)spell;
	p_spell->ineffect=FALSE;

	DebugEnd();
}



T_void SpellsDrawInEffectRunes (T_void)
{
	T_word16 i;
	T_sword16 y=138;
	T_word16 x=300;
	T_byte8 *p_pic;
	T_bitmap *p_bitmap;
	E_Boolean drawed=FALSE;

	DebugRoutine  ("SpellsDrawInEffectRunes");

	MouseHide();

	for (i=0;i<NUM_SPELLS;i++)
	{
		if (G_spells[i].ineffect==TRUE)
		{
			if (G_spells[i].iconpicptr!=NULL)
			{
				drawed=TRUE;
				p_pic=PictureLockQuick (G_spells[i].iconpicptr);
				p_bitmap = PictureToBitmap (p_pic);
			 //	GrDrawRectangle (x,y,x+15,y+12,0);
				GrDrawVerticalLine (x+16,y+1,y+13,0);
				GrDrawHorizontalLine (x+1,y+13,x+15,0);

				GrDrawBitmap (p_bitmap, x, y);
				y-=16;
				if (y<0)
				{
					y=138;
					x-=19;
				}
			}
		}
	}

	MouseShow();
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsLeap                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell causes the player to jump up in the air with 2x jump power   */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    StatsSetJumpPower                                                     */
/*    StatsGetJumpPower                                                     */
/*    ViewStartJump                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsLeap (T_spellID spell)
{
	T_word16 oldjumppower;
	DebugRoutine ("SpellsLeap");
	ColorAddGlobal (20,20,20);
	oldjumppower=StatsGetJumpPower();
	StatsSetJumpPower (oldjumppower<<1);
	ViewStartJump();
	StatsSetJumpPower (oldjumppower);
	SpellsStop (spell);
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsFast                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell doubles the movement rate of the player for a duration       */
/*  (duration is defined in spell structure)                                */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    StatsSetMaxVRunning, StatsSetMaxVWalking                             */
/*    StatsGetMaxVRunning, StatsGetMaxVWalking                              */
/*    ScheduleAddEvent                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsFast (T_spellID spell)
{
	static E_Boolean firstin=TRUE;
	static T_word16 old_move_walk;
	static T_word16 old_move_run;
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsFast");
	DebugCheck (spell != NULL);
	DebugCheck (firstin < BOOLEAN_UNKNOWN);
	p_spell = (T_spellStruct *)spell;

	if (firstin==TRUE)
	{
		if (spell!=NULL)
		{
			firstin=FALSE;
			ColorAddGlobal (0,30,0);
			old_move_walk=StatsGetMaxVWalking();
			old_move_run=StatsGetMaxVRunning();
			StatsSetMaxVWalking (old_move_walk<<1); //double max velocity
			StatsSetMaxVRunning (old_move_run<<1);
			ScheduleAddEvent (TickerGet()+p_spell->duration,(T_scheduleEventHandler)SpellsFast,(T_word32)spell);
		}
	} else
	{
		firstin=TRUE;
		SpellsStop (spell);   //turn off the spell 'in effect' icon
		StatsSetMaxVWalking (old_move_walk);//restore the old movement maximums
		StatsSetMaxVRunning (old_move_run);
	}
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsShockAbsorb                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell doubles the max velocity allowed before impact damage is     */
/*  taken. This effect lasts for a duration defined in the spell struct     */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    StatsSetMaxFallV                                                     */
/*    StatsGetMaxFallV                                                      */
/*    ScheduleAddEvent                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsShockAbsorb (T_spellID spell)
{
	static E_Boolean firstin=TRUE;
	static T_word16 old_max_fall_v;
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsShockAbsorb");
	DebugCheck (spell != NULL);
	DebugCheck (firstin < BOOLEAN_UNKNOWN);
	p_spell = (T_spellStruct *)spell;

	if (firstin==TRUE)
	{
		if (spell!=NULL)
		{
			ColorAddGlobal (0,0,30);
			firstin=FALSE;
			old_max_fall_v=StatsGetMaxFallV();
			StatsSetMaxFallV (old_max_fall_v<<1);
			ScheduleAddEvent (TickerGet()+p_spell->duration,(T_scheduleEventHandler)SpellsShockAbsorb,(T_word32)spell);
		}
	} else
	{
		firstin=TRUE;
		StatsSetMaxFallV (old_max_fall_v);
		SpellsStop (spell);
	}
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsHeal                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell Heals the player 200 life points                             */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    StatsHealPlayer                                                      */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsHeal (T_spellID spell)
{
	DebugRoutine ("SpellsHeal");
	StatsHealPlayer (200);
	SpellsStop (spell);
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsFireBall                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell creates a fireball                                           */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    ClientShootFireball                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsFireball (T_spellID spell)
{
	DebugRoutine ("SpellsFireball");
	ColorAddGlobal (30,15,15);
	ClientShootFireball();
	SpellsStop (spell);
	DebugEnd();
}

/****************************************************************************/
/*  Routine:  SpellsDuration                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell doubles the effective duration of all spells casted during   */
/*  the duration of this one.                                              */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                         */
/*    ScheduleAddEvent                                                      */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsDuration (T_spellID spell)
{
	static E_Boolean firstin=TRUE;
	T_spellStruct *p_spell ;
	static T_word16 olddur[NUM_SPELLS];
	T_word16 i;

	DebugRoutine ("SpellsDuration");
	DebugCheck (spell != NULL);
	DebugCheck (firstin < BOOLEAN_UNKNOWN);
	p_spell = (T_spellStruct *)spell;

	if (firstin==TRUE)
	{
		if (spell!=NULL)
		{
			firstin=FALSE;
			ColorAddGlobal (20,20,20);
			ScheduleAddEvent (TickerGet()+p_spell->duration,(T_scheduleEventHandler)SpellsDuration,(T_word32)spell);
			for (i=0;i<NUM_SPELLS;i++)
			{
				olddur[i]=G_spells[i].duration;
				G_spells[i].duration<<=1;
			}
		}
	} else
	{
		firstin=TRUE;
		SpellsStop(spell);
		for (i=0;i<NUM_SPELLS;i++)
		{
			G_spells[i].duration=olddur[i];
		}
	}
	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsRegenerate                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell regenerates character 50pts/sec over the duration of the     */
/*  spell.                                                                 */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                         */
/*    ScheduleAddEvent                                                      */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsRegenerate (T_spellID spell)
{
	static E_Boolean firstin=TRUE;
	static T_word16 dur;
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsRegenerate");
	DebugCheck (spell != NULL);
	DebugCheck (firstin < BOOLEAN_UNKNOWN);
	p_spell = (T_spellStruct *)spell;

	if (firstin==TRUE)
	{
		if (spell!=NULL)
		{
			dur=0;
			firstin=FALSE;
			ColorAddGlobal (20,20,20);
			ScheduleAddEvent (TickerGet()+70,(T_scheduleEventHandler)SpellsRegenerate,(T_word32)spell);
		}
	} else
	{
		dur+=70;
		if (dur>p_spell->duration)
		{
			firstin=TRUE;
			SpellsStop(spell);
		} else
		{
			StatsHealPlayer (50);
			ScheduleAddEvent (TickerGet()+70,(T_scheduleEventHandler)SpellsRegenerate,(T_word32)spell);
		}
	}
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  SpellsToggle                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This is a general spell function that will toggle a stats attribute     */
/*  in stats.c for a duration defined in the spell structure.               */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    StatsSetAttribute                                                    */
/*    ScheduleAddEvent                                                      */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/06/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsToggle (T_spellID spell)
{
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsToggle");
	DebugCheck (spell != NULL);
	p_spell = (T_spellStruct *)spell;
	DebugCheck (p_spell->spellflag < UNKNOWN && p_spell->spellflag > NONE);

	if (StatsGetAttribute(p_spell->spellflag)==FALSE) //not toggled
	{
		if (spell!=NULL)
		{
			StatsSetAttribute (p_spell->spellflag,TRUE);
			ScheduleAddEvent (TickerGet()+p_spell->duration,(T_scheduleEventHandler)SpellsToggle,(T_word32)spell);
//			ColorAddFilt (p_spell->filtr,p_spell->filtg,p_spell->filtb);
			ColorAddGlobal (p_spell->filtr,p_spell->filtg,p_spell->filtb);
		}
	} else
	{
		StatsSetAttribute (p_spell->spellflag,FALSE);
		SpellsStop (spell);
//		ColorAddFilt (-p_spell->filtr,-p_spell->filtg,-p_spell->filtb);
//		ColorAddGlobal (p_spell->filtr,p_spell->filtg,p_spell->filtb);
	}
	DebugEnd();
}



/****************************************************************************/
/*  Routine:  SpellsBeaconSet                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell will set a 'beacon' at the current player location.          */
/*  If BeaconReturn is cast, the player will return to the location.        */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*    ViewGetPOVLocation                                                   */
/*    ViewGetPOVFacingDir                                                   */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsBeaconSet (T_spellID spell)
{
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsBeaconSet");
	DebugCheck (spell != NULL);
	p_spell = (T_spellStruct *)spell;

	ViewGetPOVLocation (&G_beaconX, &G_beaconY);
	G_facing=ViewGetPOVFacingDir();

	printf ("Beacon set at %d %d facing=%d\r",G_beaconX,G_beaconY,G_facing);
	fflush (stdout);
	SpellsStop (spell);

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsBeaconReturn                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*  This spell will return to the beacon set by SpellBeaconSet              */
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
/*    T_spellID - spell structure which was cast                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                         */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  06/08/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void SpellsBeaconReturn (T_spellID spell)
{
	T_spellStruct *p_spell ;

	DebugRoutine ("SpellsBeaconReturn");
	DebugCheck (spell != NULL);
	p_spell = (T_spellStruct *)spell;


	printf ("Beacon set at %d %d facing=%d\r",G_beaconX,G_beaconY,G_facing);
	fflush (stdout);

	ViewTeleport (G_beaconX,G_beaconY,G_facing);
	SpellsStop (spell);

	DebugEnd();
}



