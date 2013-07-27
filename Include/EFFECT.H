/****************************************************************************/
/*    FILE:  EFFECT.H                                                       */
/****************************************************************************/

#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "DBLLINK.H"
#include "GENERAL.H"
#include "RESOURCE.H"

#define MAX_ITEM_EFFECTS 8
#define MAX_EFFECT_POWER 28000
#define MAX_EFFECT_DURATION 28000

typedef T_byte8 E_effectDamageType ;
#define EFFECT_DAMAGE_UNKNOWN 0
#define EFFECT_DAMAGE_NORMAL 1
#define EFFECT_DAMAGE_FIRE 2         /* 25% bonus damage */
#define EFFECT_DAMAGE_ACID 4         /* 1% chance per 100 hp lose item */
#define EFFECT_DAMAGE_POISON 8       /* 1 poison point per 10 pts damage */
#define EFFECT_DAMAGE_ELECTRICITY 16 /* light = +25% damage, heavy=+50% */
#define EFFECT_DAMAGE_PIERCING 32    /* ignore armor */
#define EFFECT_DAMAGE_MANA_DRAIN 64  /* 1 point mana loss per 1 pt damage */
#define EFFECT_DAMAGE_SPECIAL 128
#define EFFECT_DAMAGE_BAD 256
#define EFFECT_NUMBER_OF_DAMAGE_TYPES 7
#define EFFECT_DAMAGE_ALL (127)

#define EFFECT_DAMAGE_SPECIAL_LOCK               0
#define EFFECT_DAMAGE_SPECIAL_UNLOCK             1
#define EFFECT_DAMAGE_SPECIAL_DISPEL_MAGIC       2
#define EFFECT_DAMAGE_SPECIAL_PUSH               3
#define EFFECT_DAMAGE_SPECIAL_PULL               4
#define EFFECT_DAMAGE_SPECIAL_EARTHBIND          5
#define EFFECT_DAMAGE_SPECIAL_CONFUSE            6
#define EFFECT_DAMAGE_SPECIAL_BERSERK            7
#define EFFECT_DAMAGE_SPECIAL_SLOW               8
#define EFFECT_DAMAGE_SPECIAL_PARALYZE           9
#define EFFECT_DAMAGE_SPECIAL_UNKNOWN            10

typedef enum
{
    EFFECT_MISSILE_MAGIC_DART,
    EFFECT_MISSILE_HOMING_DART,
    EFFECT_MISSILE_ACID_BALL,
    EFFECT_MISSILE_HOMING_ACID_BALL,
    EFFECT_MISSILE_FIREBALL,
    EFFECT_MISSILE_HOMING_FIREBALL,
    EFFECT_MISSILE_HOMING_DEATHBALL,
    EFFECT_MISSILE_EARTH_SMITE,
    EFFECT_MISSILE_LIGHTNING_BOLT,
    EFFECT_MISSILE_LOCK_DOOR,
    EFFECT_MISSILE_UNLOCK_DOOR,
    EFFECT_MISSILE_DISPELL_MAGIC,
    EFFECT_MISSILE_MANA_DRAIN,
    EFFECT_MISSILE_PUSH,
    EFFECT_MISSILE_PULL,
    EFFECT_MISSILE_EARTHBIND,
    EFFECT_MISSILE_POISON,
    EFFECT_MISSILE_CONFUSION,
    EFFECT_MISSILE_BERSERK,
    EFFECT_MISSILE_SLOW,
    EFFECT_MISSILE_PARALYZE,
    EFFECT_BOLT_NORMAL,
    EFFECT_BOLT_POISON,
    EFFECT_BOLT_PIERCING,
    EFFECT_BOLT_FIRE,
    EFFECT_BOLT_ELECTRICITY,
    EFFECT_BOLT_MANA_DRAIN,
    EFFECT_BOLT_ACID,
    EFFECT_MISSILE_PLASMA,
    EFFECT_MISSILE_UNKNOWN
} E_effectMissileType;

typedef enum
{
    EFFECT_TRIGGER_NONE,
    EFFECT_TRIGGER_GET,
    EFFECT_TRIGGER_USE,
    EFFECT_TRIGGER_READY,
    EFFECT_TRIGGER_UNREADY,
    EFFECT_TRIGGER_DROP,
    EFFECT_TRIGGER_CAST,
    EFFECT_TRIGGER_UNKNOWN
} E_effectTriggerType;

typedef enum
{
    EFFECT_NONE,
    EFFECT_READY_WEAPON,
    EFFECT_SET_ARMOR,
    EFFECT_TAKE_COIN,
    EFFECT_TAKE_AMMO,               // not implemeneted yet
    EFFECT_ADD_PLAYER_EFFECT,       // only 1 player effect allowed per item
    EFFECT_REMOVE_PLAYER_EFFECT,    // only powner used for this call
    EFFECT_MOD_PLAYER_FOOD,
    EFFECT_MOD_PLAYER_WATER,
    EFFECT_MOD_PLAYER_HEALTH,
    EFFECT_MOD_PLAYER_MANA,
    EFFECT_MOD_PLAYER_POISON_LEVEL,
    EFFECT_HALVE_PLAYER_POISON_LEVEL,
    EFFECT_SET_PLAYER_POISON_LEVEL,
    EFFECT_PLAYER_LEAP,
    EFFECT_CREATE_PROJECTILE,       // not fully implemeneted
    EFFECT_SET_PLAYER_BEACON,       // not implemented this version
    EFFECT_RETURN_TO_PLAYER_BEACON, // not implemented this version
    EFFECT_CLEAR_PLAYER_BEACON,     // not implemented this version
    EFFECT_ADD_RUNE,
    EFFECT_REMOVE_RUNE,
    EFFECT_CREATE_OIL_SLICK,        // need server functionality
    EFFECT_GOTO_PLACE,              // doesn't do anything yet (easy fix?)
    EFFECT_DISPLAY_CANNED_MESSAGE,  // see message.c for colors
    EFFECT_PLAY_AREA_SOUND,
    EFFECT_PLAY_LOCAL_SOUND,
    EFFECT_DESTROY_RANDOM_EQUIPPED_ITEM,
    EFFECT_DESTROY_RANDOM_STORED_ITEM,
    EFFECT_DESTROY_RANDOM_ITEM,
    EFFECT_TRIGGER_EARTHQUAKE,
    EFFECT_REMOVE_RANDOM_SPELL,
    EFFECT_REMOVE_ALL_SPELLS,
    EFFECT_REMOVE_SPECIFIC_SPELL,
    EFFECT_REMOVE_SPECIFIC_EFFECT,  // not implemented this version
    EFFECT_REMOVE_ALL_EFFECTS,      // not implemented this version
    EFFECT_MOD_PLAYER_HEALTH_RANDOM,
    EFFECT_MOD_PLAYER_MANA_RANDOM,
    EFFECT_MOD_PLAYER_EXPERIENCE,
    EFFECT_TAKE_NORMAL_DAMAGE,
    EFFECT_TAKE_FIRE_DAMAGE,
    EFFECT_TAKE_ACID_DAMAGE,
    EFFECT_TAKE_POISON_DAMAGE,
    EFFECT_TAKE_ELECTRICITY_DAMAGE,
    EFFECT_TAKE_PIERCING_DAMAGE,
    EFFECT_TAKE_MANA_DRAIN_DAMAGE,
    EFFECT_TAKE_MULTIPLE_DAMAGE,
    EFFECT_JUMP_FORWARD,            // not yet implemented
    EFFECT_REORIENT,                // not yet implemented
    EFFECT_TURN_UNDEAD,             // need server functionality
    EFFECT_AREA_SLOW,               // need server functionality
    EFFECT_FIRE_WAND,
    EFFECT_FIRE_BOLT,
    EFFECT_COLOR_FLASH,
    EFFECT_ADD_JOURNAL_PAGE_BY_OBJECT,
    EFFECT_CREATE_OBJECT,
    EFFECT_ADD_JOURNAL_PAGE,
    EFFECT_IDENTIFY_READIED,
    EFFECT_IDENTIFY_ALL,
    EFFECT_ACTIVATE_THIEVING,
    EFFECT_MOD_PLAYER_WEALTH,
    EFFECT_UNKNOWN
} E_effectType;

typedef enum
{
    PLAYER_EFFECT_NONE,
    PLAYER_EFFECT_INVULNERABLE,
    PLAYER_EFFECT_FLY,
    PLAYER_EFFECT_LAVA_WALK,
    PLAYER_EFFECT_WATER_WALK,
    PLAYER_EFFECT_FEATHER_FALL,     // doesn't work
    PLAYER_EFFECT_LOW_GRAVITY,      // doesn't work
    PLAYER_EFFECT_STICKY_FEET,      // doesn't work
    PLAYER_EFFECT_SHOCK_ABSORB,     // doesn't work
    PLAYER_EFFECT_SHIELD,
    PLAYER_EFFECT_RESIST_POISON,
    PLAYER_EFFECT_RESIST_FIRE,
    PLAYER_EFFECT_RESIST_ACID,
    PLAYER_EFFECT_STRENGTH_MOD,
    PLAYER_EFFECT_ACCURACY_MOD,
    PLAYER_EFFECT_SPEED_MOD,
    PLAYER_EFFECT_CONSTITUTION_MOD,
    PLAYER_EFFECT_MAGIC_MOD,
    PLAYER_EFFECT_STEALTH_MOD,
    PLAYER_EFFECT_HEALTH_REGEN_MOD,
    PLAYER_EFFECT_MANA_REGEN_MOD,
    PLAYER_EFFECT_MAGIC_MAP,       // doesn't work properly
    PLAYER_EFFECT_TRANSLUCENT,     // doesn't work
    PLAYER_EFFECT_INVISIBLE,       // doesn't work
    PLAYER_EFFECT_NIGHT_VISION,
    PLAYER_EFFECT_ETERNAL_NOURISHMENT,
    PLAYER_EFFECT_GOD_MODE,
    PLAYER_EFFECT_DISABLE_PLAYER_EFFECT,
    PLAYER_EFFECT_RESIST_ELECTRICITY,
    PLAYER_EFFECT_RESIST_MANA_DRAIN,
    PLAYER_EFFECT_RESIST_PIERCING,
    PLAYER_EFFECT_POISON_ATTACK,
    PLAYER_EFFECT_ELECTRICITY_ATTACK,
    PLAYER_EFFECT_PIERCING_ATTACK,
    PLAYER_EFFECT_MANA_DRAIN_ATTACK,
    PLAYER_EFFECT_FIRE_ATTACK,
    PLAYER_EFFECT_ACID_ATTACK,
    PLAYER_EFFECT_FOOD_CONSERVATION,
    PLAYER_EFFECT_WATER_CONSERVATION,
    PLAYER_EFFECT_MOD_JUMP_POWER,
    PLAYER_EFFECT_WATER_BREATHING,
    PLAYER_EFFECT_DEATH_WARD,
    PLAYER_EFFECT_UNKNOWN,
} E_playerEffectType;

typedef struct
{
    /* type of player effect in action */
    E_playerEffectType      type;
    T_word16                duration;
    T_word16                power;
    T_void*                 p_owner;
    T_doubleLinkListElement p_element;
    T_resource              p_effectpic;
} T_playerEffectStruct;

T_void EffectInit (T_void);
T_void EffectFinish (T_void);

E_Boolean Effect(E_effectType type,
                 E_effectTriggerType triggertype,
                 T_word16 data1,
                 T_word16 data2,
                 T_word16 data3,
                 T_void*  p_owner);

E_Boolean EffectPlayerEffectIsActive (E_playerEffectType type);
T_word16  EffectGetPlayerEffectPower (E_playerEffectType type);

/* variables passed in represent 'overlay' coordinates for effect icons */
T_void EffectUpdateAllPlayerEffects (T_void);
T_void EffectRemoveAllPlayerEffects (T_void);
T_void EffectDrawEffectIcons (T_word16 left,
                              T_word16 right,
                              T_word16 top,
                              T_word16 bottom);

T_void EffectSoundOff (T_void);
T_void EffectSoundOn (T_void);
E_Boolean EffectSoundIsOn (T_void);
T_void EffectRemoveRandomPlayerEffect (T_void);
#endif
