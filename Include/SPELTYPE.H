#ifndef _SPELTYPE_H_
#define _SPELTYPE_H_

#include "GENERAL.H"

typedef T_void *T_spellID;
typedef T_void (*T_spellFunct)(T_spellID spellID);

typedef enum
{
    SPELL_SYSTEM_NONE,
    SPELL_SYSTEM_MAGE,
    SPELL_SYSTEM_CLERIC,
    SPELL_SYSTEM_ARCANE,
    SPELL_SYSTEM_MAGE_AND_CLERIC,
    SPELL_SYSTEM_MAGE_AND_ARCANE,
    SPELL_SYSTEM_CLERIC_AND_ARCANE,
    SPELL_SYSTEM_ANY,
    SPELL_SYSTEM_UNKNOWN
} E_spellsSpellSystemType;

typedef enum
{
    RUNE_ARCANE_1,
    RUNE_ARCANE_2,
    RUNE_ARCANE_3,
    RUNE_ARCANE_4,
    RUNE_MAGE_1,
    RUNE_MAGE_2,
    RUNE_MAGE_3,
    RUNE_MAGE_4,
    RUNE_MAGE_5,
    RUNE_ARCANE_5,
    RUNE_ARCANE_6,
    RUNE_ARCANE_7,
    RUNE_ARCANE_8,
    RUNE_ARCANE_9,
    RUNE_CLERIC_1,
    RUNE_CLERIC_2,
    RUNE_CLERIC_3,
    RUNE_CLERIC_4,
    RUNE_UNKNOWN
} E_spellsRuneType;

typedef struct
{
    /* keypad combo (ex 4,4,3,2) code to access spell */
	T_byte8  code[4];
    /* effect type called when casting spell */
    T_word16 type;
    /* subtype of effect called (or data1 field for effect) */
    T_word16 subtype;

    /* duration of spell (or data2 field for effect) */
	T_word16 duration;
    /* this field modifies the duration of the spell by */
    /* character level * value */
    /* duration can never exceed typedef MAX_EFFECT_DURATION */
    /* (defined in effect.h) */
    T_sword16 durationmod;

    /* power of spell (or data3 field for effect) */
    T_word16 power;
    /* this field modifies the power level of the spell based on your */
    /* current character level.  Note that a negative value will lower */
    /* the power of the spell by your level * value */
    /* also note that power can never exceed typdef MAX_EFFECT_POWER */
    /* ( see effect.h ) */
    T_sword16 powermod;

    /* amount of mana deducted on sucessful casting */
    T_word16 cost;
    /* cost modifer is added to cost (player level * mod) */
    T_sword16 costmod;

    /* when casting spell, your percent chance for sucess = you magic attribute */
    /* this field will modify that chance, so that a positive number */
    /* increases your chance, and a neg. number decreases your chace */
    T_sbyte8 hardness;
    /* this field is multiplied by your level and added to the hardness */
    /* field, i.e. the higher the number the easier the spell is to cast */
    /* on gaining character levels */
    /* minimum level.  Note that a negitive value would actually make the */
    /* spell harder to cast if you are a higher level */
    T_sbyte8 hardnessmod;

    /* this is the minimum level of experience required to cast this spell */
    T_byte8 minlevel;
    /* type = type of spellsystem for this spell */
    T_byte8 system;
    /* note, should be a E_spellsSpellSystemType but is a t_byte8 for */
    /* purposes of the script-compiler written in borlandc */

    /* color splash when casting spell */
	T_sbyte8 filtr;
	T_sbyte8 filtg;
	T_sbyte8 filtb;

    /* sound effect to play on successful casting of spell */
    T_word16 sound;

} T_spellStruct;

#endif // _SPELTYPE_H_
