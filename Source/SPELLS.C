/****************************************************************************/
/*    FILE:  SPELLS.C                                                       */
/****************************************************************************/
#include "BANNER.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "DBLLINK.H"
#include "EFFECT.H"
#include "KEYSCAN.H"
#include "MESSAGE.H"
#include "PICS.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SPELLS.H"
#include "STATS.H"

/* define if all spells available for any class */
#define MANA_BACKCOLOR 225

static T_byte8 G_curspell[4]={0,0,0,0};
static T_byte8 G_curSound=0;
static T_doubleLinkList G_spellsList;
static T_sword16 G_beaconX=0;
static T_sword16 G_beaconY=0;
static T_word16  G_facing=0;
static E_Boolean G_clearSpells=FALSE;
static E_Boolean G_isInit = FALSE ;

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
	T_word16 count=0;
    T_byte8 stmp[32];
    T_word16 i;
    T_resource res;
    T_spellStruct *p_spell;
    E_spellsSpellSystemType system;
    E_Boolean unload=FALSE;

	DebugRoutine ("SpellsInitSpells");
    DebugCheck(G_isInit == FALSE) ;

    G_isInit = TRUE ;

    /* now, create new linked list for storage of spells */
    G_spellsList=DoubleLinkListCreate();
    DebugCheck (G_spellsList!=DOUBLE_LINK_LIST_BAD);

    /* get current character spell system */
    system = StatsGetPlayerSpellSystem();

    /* scan through resource files and lock in all available spells */
    /* for current class type */
    sprintf (stmp,"SPELDESC/SPL%05d",count++);
    while (PictureExist(stmp))
    {
        /* lock in a new spell structure */
        p_spell = (T_spellStruct *)PictureLockData (stmp,&res);

        DebugCheck (p_spell != NULL);

        /* remap casting code to keyboard */
        for (i=0;i<4;i++)
        {
            if (p_spell->code[i]==1) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_1;
            else if (p_spell->code[i]==2) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_2;
            else if (p_spell->code[i]==3) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_3;
            else if (p_spell->code[i]==4) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_4;
            else if (p_spell->code[i]==5) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_5;
            else if (p_spell->code[i]==6) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_6;
            else if (p_spell->code[i]==7) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_7;
            else if (p_spell->code[i]==8) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_8;
            else if (p_spell->code[i]==9) p_spell->code[i]=KEY_SCAN_CODE_KEYPAD_9;
        }
/*
        printf ("\n\nloading spell:\n");
        printf ("code = %d,%d,%d,%d\n",p_spell->code[0],
                                       p_spell->code[1],
                                       p_spell->code[2],
                                       p_spell->code[3]);
        printf ("effect type =%d\n",p_spell->type);
        printf ("effect sub  =%d\n",p_spell->subtype);
        printf ("duration    =%d\n",p_spell->duration);
        printf ("duration mod=%d\n",p_spell->durationmod);
        printf ("power       =%d\n",p_spell->power);
        printf ("power mod   =%d\n",p_spell->powermod);
        printf ("cost        =%d\n",p_spell->cost);
        printf ("cost mod    =%d\n",p_spell->costmod);
        printf ("hardness    =%d\n",p_spell->hardness);
        printf ("hardness mod=%d\n",p_spell->hardnessmod);
        printf ("minimum lev =%d\n",p_spell->minlevel);
        printf ("system      =%d\n",p_spell->system);
        printf ("splash red  =%d\n",p_spell->filtr);
        printf ("splash grn  =%d\n",p_spell->filtg);
        printf ("splash blu  =%d\n",p_spell->filtb);
        printf ("\n\n\n");
        fflush (stdout);
*/
        /* determine if the character can cast this spell */
#ifndef COMPILE_OPTION_LOAD_ALL_SPELLS
        unload = TRUE;
        switch (system)
        {
            case SPELL_SYSTEM_MAGE:
            if (p_spell->system == SPELL_SYSTEM_MAGE ||
                p_spell->system == SPELL_SYSTEM_MAGE_AND_CLERIC ||
                p_spell->system == SPELL_SYSTEM_MAGE_AND_ARCANE ||
                p_spell->system == SPELL_SYSTEM_ANY) unload=FALSE;
            break;

            case SPELL_SYSTEM_CLERIC:
            if (p_spell->system == SPELL_SYSTEM_CLERIC ||
                p_spell->system == SPELL_SYSTEM_MAGE_AND_CLERIC ||
                p_spell->system == SPELL_SYSTEM_CLERIC_AND_ARCANE ||
                p_spell->system == SPELL_SYSTEM_ANY) unload=FALSE;
            break;

            case SPELL_SYSTEM_ARCANE:
            if (p_spell->system == SPELL_SYSTEM_ARCANE ||
                p_spell->system == SPELL_SYSTEM_MAGE_AND_ARCANE ||
                p_spell->system == SPELL_SYSTEM_CLERIC_AND_ARCANE ||
                p_spell->system == SPELL_SYSTEM_ANY) unload=FALSE;
            break;

            default:
            /* fail! improper spell system returned */
            DebugCheck (-1);
        }
#endif

        if (unload == TRUE)
        {
            /* nevermind, we don't want this spell anyway. */
//            printf ("unloading this spell, improper casting class\n");
//            fflush (stdout);
            PictureUnlockAndUnfind (res);
        }
        else
        {
//            printf ("saving this spell in list\n");
//            fflush (stdout);

            /* add spell pointer to linked list */
            DoubleLinkListAddElementAtEnd (G_spellsList, res);
        }
        sprintf (stmp,"SPELDESC/SPL%05d",count++);
    }

	DebugEnd();
}



/****************************************************************************/
/*  Routine:  SpellsFinish                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      SpellsFinish gets rid of any memory or resources associated with    */
/*  all the spells, either in effect, or able to be used.                   */
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
/*    PictureUnfind                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/28/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsFinish(T_void)
{
	T_word16 i ;
    T_doubleLinkListElement element, nextElement ;
    T_resource res ;

	DebugRoutine ("SpellsFinish");
    DebugCheck(G_isInit == TRUE) ;

    G_isInit = FALSE ;

//    printf ("spells finish called\n");
//    fflush (stdout);

    /* unlock and unfind all spell structure resources */

    element = DoubleLinkListGetFirst(G_spellsList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        nextElement = DoubleLinkListElementGetNext(element) ;

        res = DoubleLinkListElementGetData(element) ;

        PictureUnlockAndUnfind(res) ;
        DoubleLinkListRemoveElement(element) ;

        element = nextElement ;
    }

    /* clear globals */
    for (i=0;i<4;i++) G_curspell[i]=0;
    G_clearSpells=FALSE;

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

    if (G_clearSpells==TRUE)
    {
        G_clearSpells=FALSE;
    	SpellsClearRunes (buttonID);
    }

	if (G_curspell[3]!=0)
	{
		ColorSetColor (MANA_BACKCOLOR,55,0,0);
	}
	else if (buttonID != NULL)
	{
		p_button=(T_buttonStruct *)buttonID;
		for (i=0;i<4;i++)
		{
			if (G_curspell[i]==0) //if slot is empty
			{
				manasuck=25;
				StatsChangePlayerMana(-manasuck);
				if (StatsGetPlayerMana()>0)
				{
				  ColorSetColor (MANA_BACKCOLOR,30,0,30);
				  G_curspell[i]=(T_byte8)p_button->scancode;   //add scancode to spell key
				  SpellsDrawRuneBox();
				} else ColorSetColor (MANA_BACKCOLOR,55,0,0);
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
	T_bitmap *pic ;
	T_resource res ;
	DebugRoutine ("SpellsClearRunes");

	for (i=0;i<4;i++) G_curspell[i]=0;

	pic = (T_bitmap *)PictureLockData("UI/3DUI/SPSTRIP", &res) ;
	DebugCheck(pic != NULL) ;

	if (pic != NULL)
	{
		GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
        MouseHide();
		GrDrawBitmap(pic, 176, 181) ;
        MouseShow();
		PictureUnlock(res) ;
	PictureUnfind(res) ;
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
	T_bitmap *pic ;
	T_resource res ;
	DebugRoutine ("SpellsBackspace");

	for (i=0;i<4;i++) G_curspell[i]=0;

	pic = (T_bitmap *)PictureLockData("UI/3DUI/SPSTRIP", &res) ;
	DebugCheck(pic != NULL) ;

	if (pic != NULL)
	{
		GrScreenSet(GRAPHICS_ACTUAL_SCREEN) ;
		GrDrawBitmap(pic, 176, 181) ;
		PictureUnlock(res) ;
		PictureUnfind(res) ;
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
	T_buttonID runebutton;
    E_Boolean pushed=FALSE;
	T_graphicID runepic;

	DebugRoutine ("SpellsDrawRuneBox");

    if (BannerButtonsOk())
    {
	    for (i=0;i<4;i++)
	    {
		    if (G_curspell[i]!=0)
		    {
		         runebutton=ButtonGetByKey (G_curspell[i]);
                 pushed=ButtonIsPushed(runebutton);
                 if (pushed==FALSE)
                 {
                    ButtonDownNoAction (runebutton);
                 }
		         runepic=ButtonGetGraphic(runebutton);
                 GraphicSetShadow (runepic,255);
                 MouseHide();
		         GraphicDrawAt (runepic,177+(i*15),182);
                 MouseShow();
                 if (pushed==FALSE)
                 {
                    ButtonUpNoAction (runebutton);
                 }
		    }
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

#if 0
T_void SpellsCastSpell (T_buttonID buttonID)
{
    DebugRoutine ("SpellsCastSpell");

    /* start sound sequence */
    G_curSound=0;

    MessagePrintf ("code=%d %d %d %d",G_curspell[0],G_curspell[1],G_curspell[2],G_curspell[3]);

    SpellsMakeNextSound (NULL);

    DebugEnd();
}

T_void SpellsMakeNextSound (T_void *p_data)
{
    T_word16 soundNum;

    DebugRoutine ("SpellsMakeNextSound");


    if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_CLERIC )
    {
        if (G_curSound==3)
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_CLERIC_B_SET - 70;
        }
        else if (G_curspell[G_curSound+1]==0)
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_CLERIC_B_SET - 70;
        }
        else
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_CLERIC_A_SET - 70;
        }
    }
    else
    {
        if (G_curSound==3)
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_MAGE_B_SET - 70;
        }
        else if (G_curspell[G_curSound+1]==0)
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_MAGE_B_SET - 70;
        }
        else
        {
            soundNum=G_curspell[G_curSound]+SOUND_SPELL_MAGE_A_SET - 70;
        }
    }

    if (G_curSound == 3)
    {
        SoundPlayByNumber(soundNum,255);
        SpellsFinishSpell();
    }
    else if (G_curspell[G_curSound+1]==0)
    {
        SoundPlayByNumber(soundNum,255);
        SpellsFinishSpell();
    }
    else
    {
        MessagePrintf ("cs=%d G_cursp=%d",G_curSound,G_curspell[G_curSound]);
        MessagePrintf ("playing sound %d",soundNum);
        SoundPlayByNumberWithCallback(
            soundNum,
            255,
            SpellsMakeNextSound,
            NULL) ;
    }

    G_curSound++;

    DebugEnd();
}

#endif

T_void SpellsCastSpell (T_buttonID buttonID)
{
	E_Boolean success;
    T_doubleLinkListElement element;
    T_resource res;
    T_spellStruct *p_spell;
    T_word32 spellpower, spellduration, spellcost;
    T_sword16 spelldif;
    T_byte8 charlevel;
	T_word32 i;

	DebugRoutine ("SpellsCastSpell");

    /* search through spell list and see if any spell */
    /* matches current spell code */

    if (ClientIsPaused())  {
        MessageAdd("Cannot cast spells while paused.") ;
    } else if (ClientIsDead())  {
        MessageAdd("Dead players do not cast spells.") ;
    } else {
        element = DoubleLinkListGetFirst (G_spellsList);

        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* get a pointer to this spell's spell struct */
            res = DoubleLinkListElementGetData(element);
            p_spell = (T_spellStruct *)PictureLockDataQuick (res);
            DebugCheck (p_spell != NULL);

            /* check to see if code matches */
            success=TRUE;
            for (i=0;i<4;i++)
            {
                if (p_spell->code[i]!=G_curspell[i])
                {
                    success=FALSE;
                    break;
                }
            }

            if (success==TRUE)
            {
                /* get level of character */
                charlevel = StatsGetPlayerLevel();

                /* figure duration of spell */
                spellduration = p_spell->duration + (p_spell->durationmod*charlevel);
                if (spellduration > MAX_EFFECT_DURATION) spellduration = MAX_EFFECT_DURATION;

                /* figure power of spell */
                spellpower = p_spell->power + (p_spell->powermod*charlevel);
                if (spellpower > MAX_EFFECT_POWER) spellpower = MAX_EFFECT_POWER;

                /* figure casting cost of spell */
                spellcost = p_spell->cost + (p_spell->costmod*charlevel);

                /* figure difficulty of spell */
                spelldif = StatsGetPlayerAttribute(ATTRIBUTE_MAGIC) +
                           p_spell->hardness + (2 * charlevel);
                if (spelldif < 0) spelldif = 0;

                /* cast this spell if possible */
                if (((StatsGetManaLeft() > (T_sword32)spellcost) && (charlevel >= p_spell->minlevel)) ||
                    (EffectPlayerEffectIsActive (PLAYER_EFFECT_GOD_MODE) == TRUE))
                {
                    success=TRUE;

                    /* check for a d100 agains hardness for sucess */
                    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_GOD_MODE) == FALSE)
                    {
                        if (rand()%100 > spelldif)
                        {
                            /* failed roll */
    //                        success = FALSE;
                            MessageAdd ("Your spell fizzled");
                            ColorAddGlobal (10,10,-10);
                            SoundPlayByNumber (SOUND_SPELL_FIZZLE,255);
                            /* remove half the spell cost */
                            StatsChangePlayerMana (-p_spell->cost>>1);
                            PictureUnlock(res);
                            break;
                        }
                    }

                    if (success==TRUE)
                    {
                        if (EffectPlayerEffectIsActive(PLAYER_EFFECT_GOD_MODE)==FALSE)
                        {
                            StatsChangePlayerMana (-p_spell->cost);
                        }
                        /* do a color effect */
                        ColorAddGlobal ((T_sbyte8)p_spell->filtr>>1,(T_sbyte8)p_spell->filtg>>1,(T_sbyte8)p_spell->filtb>>1);

                        /* create spell effect */
                        Effect (p_spell->type,
                            EFFECT_TRIGGER_CAST,
                            p_spell->subtype,
                            (T_word16)spellduration,
                            (T_word16)spellpower,
                            p_spell);

                        /* trigger the spell sound */
                        if (p_spell->sound != 0)
                        {
                            SoundPlayByNumber (p_spell->sound,255);
                        }
                        PictureUnlock(res);
                        break;
                    }
                }
                else
                {
                    MessageAdd ("^003You are too exhausted to cast this spell.");
                    success=TRUE;
                    PictureUnlock(res);
                    break;
                }

                /* spell success, break from while loop */
                /* nevermind, check for other spells with same code */
    //			break;
		    }

            PictureUnlock(res);
            element = DoubleLinkListElementGetNext (element);
	    }
        if (success==FALSE && G_curspell[0]!=0)
        {
            MessageAdd ("^003You feel something is just not right.");
            SpellsClearRunes(NULL);
        }
        /* set flag to clear runes on next rune entered */
        else G_clearSpells=TRUE;
    }

	DebugEnd();
}


/****************************************************************************/
/*  Routine:  SpellsStopAll                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*     SpellsStopAll turns off all the spells that are in effect.           */
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
/*    SpellsStop                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  09/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void SpellsStopAll (T_void)
{
    DebugRoutine("SpellsStopAll") ;

//    for (i=0; i<NUM_SPELLS; i++)
//        SpellsStop(G_spells+i) ;

	DebugEnd();
}



T_void SpellsDrawInEffectRunes (T_word16 left,
								T_word16 right,
								T_word16 top,
								T_word16 bottom)
{
	E_Boolean drawed=FALSE;

	DebugRoutine  ("SpellsDrawInEffectRunes");
	DebugEnd();
}

#ifdef shit
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
/*    PlayerJump                                                            */
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
	DebugRoutine ("SpellsLeap");
	ColorAddGlobal (20,20,20);

	PlayerJump(StatsGetJumpPower()<<1);

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
/*    StatsSetMaxVRunning, StatsSetMaxVWalking                              */
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
/*    StatsSetMaxFallV                                                      */
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
#ifdef shit
	DebugCheck (p_spell->spellflag < SPELL_UNKNOWN && p_spell->spellflag > SPELL_NONE);

	if (StatsGetAttribute(p_spell->spellflag)==FALSE) //not toggled
	{
		if (spell!=NULL)
		{
			StatsSetAttribute (p_spell->spellflag,TRUE);
			ScheduleAddEvent (TickerGet()+p_spell->duration,(T_scheduleEventHandler)SpellsToggle,(T_word32)spell);
//                      ColorAddFilt (p_spell->filtr,p_spell->filtg,p_spell->filtb);
			ColorAddGlobal (p_spell->filtr,p_spell->filtg,p_spell->filtb);
		}
	} else
	{
		StatsSetAttribute (p_spell->spellflag,FALSE);
		SpellsStop (spell);
//              ColorAddFilt (-p_spell->filtr,-p_spell->filtg,-p_spell->filtb);
//              ColorAddGlobal (p_spell->filtr,p_spell->filtg,p_spell->filtb);
	}
#endif
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
/*    PlayerGetX                                                            */
/*    PlayerGetY                                                            */
/*    PlayerGetAngle                                                        */
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

	G_beaconX = (PlayerGetX()>>16) ;
	G_beaconY = (PlayerGetY()>>16) ;
	G_facing = PlayerGetAngle() ;

//      printf ("Beacon set at %d %d facing=%d\r",G_beaconX,G_beaconY,G_facing);
//      fflush (stdout);
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


//      printf ("Beacon set at %d %d facing=%d\r",G_beaconX,G_beaconY,G_facing);
//      fflush (stdout);

	PlayerTeleport (G_beaconX,G_beaconY,G_facing);
	SpellsStop (spell);

	DebugEnd();
}

#endif


T_void SpellsSetRune (E_spellsRuneType type)
{
    DebugRoutine ("SpellsAddRune");


    /* check to see if this rune is valid for the characters */
    /* current spell system */
    if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_MAGE)
    {
        /* valid runes are >=RUNE_ARCANE_1, <=RUNE_MAGE_5 */
        if (type >=RUNE_ARCANE_1 && type <= RUNE_MAGE_5)
        {
            /* valid rune, increment counter */
            StatsIncrementRuneCount (type);
        }
    } else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_CLERIC)
    {
        if (type >= RUNE_ARCANE_5 && type <= RUNE_CLERIC_4)
        {
            StatsIncrementRuneCount (type - RUNE_ARCANE_5);
        }
    } else
    {
        /* arcane spell system */
        if (type >= RUNE_ARCANE_1 && type <= RUNE_ARCANE_4)
        {
            StatsIncrementRuneCount (type - RUNE_ARCANE_1);
        }
        else if (type >= RUNE_ARCANE_5 && type <= RUNE_ARCANE_9)
        {
            StatsIncrementRuneCount (type - RUNE_ARCANE_5 + 4);
        }
    }

    DebugEnd();
}


T_void SpellsClearRune (E_spellsRuneType type)
{
    DebugRoutine ("SpellsClearRune");

    /* check to see if this rune is valid for the characters */
    /* current spell system */
    if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_MAGE)
    {
        /* valid runes are >=RUNE_ARCANE_1, <=RUNE_MAGE_5 */
        if (type >=RUNE_ARCANE_1 && type <= RUNE_MAGE_5)
        {
            /* valid rune, increment counter */
            StatsDecrementRuneCount (type);
        }
    } else if (StatsGetPlayerSpellSystem()==SPELL_SYSTEM_CLERIC)
    {
        if (type >= RUNE_ARCANE_5 && type <= RUNE_CLERIC_4)
        {
            StatsDecrementRuneCount (type - RUNE_ARCANE_5);
        }
    } else
    {
        /* arcane spell system */
        if (type >= RUNE_ARCANE_1 && type <= RUNE_ARCANE_4)
        {
            StatsDecrementRuneCount (type - RUNE_ARCANE_1);
        }
        else if (type >= RUNE_ARCANE_5 && type <= RUNE_ARCANE_9)
        {
            StatsDecrementRuneCount (type - RUNE_ARCANE_5 + 4);
        }
    }

    DebugEnd();
}

