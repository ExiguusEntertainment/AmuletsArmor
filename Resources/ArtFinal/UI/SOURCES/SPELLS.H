/****************************************************************************/
/*    FILE:  SPELLS.H                                                       */
/****************************************************************************/

#ifndef _SPELLS_H_
#define _SPELLS_H_

#define NUM_SPELLS 17
#define MANA_BACKCOLOR 6

typedef T_void *T_spellID;
typedef T_void (*T_spellFunct)(T_spellID spellID);

typedef struct
{
	T_byte8  code[4];  //rune code to access spell
	T_word32 duration; //length in clicks of effect
	E_Boolean ineffect; //true if the spell is in action
	T_spellFunct spellfunction;
	E_SpellFlag spellflag;
	T_sbyte8 filtr;
	T_sbyte8 filtg;
	T_sbyte8 filtb;
	T_byte8  iconname[10];
	T_byte8  *iconpicptr;
} T_spellStruct;

T_void SpellsInitSpells (T_void);
T_void SpellsUpdateManaDisplay(T_void);
T_void SpellsAddRune (T_buttonID buttonID);
T_void SpellsClearRunes (T_buttonID buttonID);
T_void SpellsCastSpell (T_buttonID buttonID);
T_void SpellsDrawRuneBox (T_void);
T_void SpellsLeap (T_spellID spell);
T_void SpellsFast (T_spellID spell);
T_void SpellsShockAbsorb (T_spellID spell);
T_void SpellsHeal (T_spellID spell);
T_void SpellsToggle (T_spellID spell);
T_void SpellsFireball (T_spellID spell);
T_void SpellsLavaWalk (T_spellID spell);
T_void SpellsWaterWalk (T_spellID spell);
T_void SpellsInvulnerable (T_spellID spell);
T_void SpellsDuration (T_spellID spell);
T_void SpellsRegenerate (T_spellID spell);
T_void SpellsFeatherFall (T_spellID spell);
T_void SpellsLoGrav (T_spellID spell);
T_void SpellsBeaconSet (T_spellID spell);
T_void SpellsBeaconReturn (T_spellID spell);
T_void SpellsStart (T_spellID spell);
T_void SpellsStop (T_spellID spell);
T_void SpellsDrawInEffectIcons (T_void);
#endif
