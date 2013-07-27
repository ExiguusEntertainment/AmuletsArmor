/****************************************************************************/
/*    FILE:  EQUIP.H                                                        */
/****************************************************************************/
#ifndef _EQUIP_H_
#define _EQUIP_H_

#include "GENERAL.H"
#include "EFFECT.H"

#define EQUIP_NUMBER_OF_LOCATIONS 15
#define EQUIP_TOTAL_COIN_TYPES 4
#define EQUIP_TOTAL_BOLT_TYPES 7
#define EQUIP_LOC_X1 0
#define EQUIP_LOC_Y1 1
#define EQUIP_LOC_X2 2
#define EQUIP_LOC_Y2 3

#define USEABLE_BY_NONE      0
#define USEABLE_BY_CITIZEN   1
#define USEABLE_BY_KNIGHT    2
#define USEABLE_BY_MAGE      4
#define USEABLE_BY_WARLOCK   8
#define USEABLE_BY_PRIEST    16
#define USEABLE_BY_ROGUE     32
#define USEABLE_BY_ARCHER    64
#define USEABLE_BY_SAILOR    128
#define USEABLE_BY_PALADIN   256
#define USEABLE_BY_MERCENARY 512
#define USEABLE_BY_MAGICIAN  1024
#define USEABLE_BY_ALL       2047

typedef enum
{
    EQUIP_LOCATION_MOUSE_HAND,
    EQUIP_LOCATION_READY_HAND,
    EQUIP_LOCATION_HEAD,
    EQUIP_LOCATION_LEFT_ARM,
    EQUIP_LOCATION_RIGHT_ARM,
    EQUIP_LOCATION_CHEST,
    EQUIP_LOCATION_LEGS,
    EQUIP_LOCATION_SHIELD_HAND,
    EQUIP_LOCATION_NECK,
    EQUIP_LOCATION_RING_1,
    EQUIP_LOCATION_RING_2,
    EQUIP_LOCATION_RING_3,
    EQUIP_LOCATION_RING_4,
    EQUIP_LOCATION_UNKNOWN
//  EQUIP_LOCATION_BODY,
//  EQUIP_LOCATION_LEFT_LEG,
//  EQUIP_LOCATION_RIGHT_LEG,
} E_equipLocations;

typedef enum
{
    EQUIP_MATERIAL_TYPE_NONE,
    EQUIP_MATERIAL_TYPE_WOOD,
    EQUIP_MATERIAL_TYPE_RUSTY,
    EQUIP_MATERIAL_TYPE_BRONZE,
    EQUIP_MATERIAL_TYPE_IRON,
    EQUIP_MATERIAL_TYPE_SILVER,
    EQUIP_MATERIAL_TYPE_STEEL,
    EQUIP_MATERIAL_TYPE_HARDENED_STEEL,
    EQUIP_MATERIAL_TYPE_MITHRIL,
    EQUIP_MATERIAL_TYPE_OBSIDIAN,
    EQUIP_MATERIAL_TYPE_PYRINIUM,
    EQUIP_MATERIAL_TYPE_GOLD,
    EQUIP_MATERIAL_TYPE_UNDEFINED_1,
    EQUIP_MATERIAL_TYPE_UNDEFINED_2,
    EQUIP_MATERIAL_TYPE_UNDEFINED_3,
    EQUIP_MATERIAL_TYPE_UNDEFINED_4
} E_equipMaterialTypes;


typedef enum
{
    EQUIP_OBJECT_TYPE_WEAPON,
    EQUIP_OBJECT_TYPE_ARMOR,
    EQUIP_OBJECT_TYPE_AMULET,
    EQUIP_OBJECT_TYPE_RING,
    EQUIP_OBJECT_TYPE_POTION,
    EQUIP_OBJECT_TYPE_WAND,
    EQUIP_OBJECT_TYPE_SCROLL,
    EQUIP_OBJECT_TYPE_FOODSTUFF,
    EQUIP_OBJECT_TYPE_POWERUP,
    EQUIP_OBJECT_TYPE_THING,
    EQUIP_OBJECT_TYPE_COIN,
    EQUIP_OBJECT_TYPE_BOLT,
    EQUIP_OBJECT_TYPE_QUIVER,
    EQUIP_OBJECT_TYPE_UNKNOWN
} E_equipObjectTypes;

typedef enum
{
    EQUIP_WEAPON_TYPE_NONE,
    EQUIP_WEAPON_TYPE_AXE,
    EQUIP_WEAPON_TYPE_DAGGER,
    EQUIP_WEAPON_TYPE_LONGSWORD,
    EQUIP_WEAPON_TYPE_SHORTSWORD,
    EQUIP_WEAPON_TYPE_MACE,
    EQUIP_WEAPON_TYPE_STAFF,
    EQUIP_WEAPON_TYPE_CROSSBOW,
    EQUIP_WEAPON_TYPE_WAND,
    EQUIP_WEAPON_TYPE_UNKNOWN
} E_equipWeaponTypes;

typedef enum
{
    EQUIP_ARMOR_TYPE_NONE,
    EQUIP_ARMOR_TYPE_BRACING_CLOTH,
    EQUIP_ARMOR_TYPE_BRACING_CHAIN,
    EQUIP_ARMOR_TYPE_BRACING_PLATE,
    EQUIP_ARMOR_TYPE_LEGGINGS_CLOTH,
    EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN,
    EQUIP_ARMOR_TYPE_LEGGINGS_PLATE,
    EQUIP_ARMOR_TYPE_HELMET_CLOTH,
    EQUIP_ARMOR_TYPE_HELMET_CHAIN,
    EQUIP_ARMOR_TYPE_HELMET_PLATE,
    EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH,
    EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN,
    EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE,
    EQUIP_ARMOR_TYPE_SHIELD,
    EQUIP_ARMOR_TYPE_UNKNOWN
} E_equipArmorTypes;


typedef enum
{
    COIN_TYPE_COPPER,
    COIN_TYPE_SILVER,
    COIN_TYPE_GOLD,
    COIN_TYPE_PLATINUM,
    COIN_TYPE_FIVE,
    COIN_TYPE_UNKNOWN
} E_equipCoinTypes;


typedef enum
{
    BOLT_TYPE_NORMAL,
    BOLT_TYPE_POISON,
    BOLT_TYPE_PIERCING,
    BOLT_TYPE_FIRE,
    BOLT_TYPE_ELECTRICITY,
    BOLT_TYPE_MANA_DRAIN,
    BOLT_TYPE_ACID,
    BOLT_TYPE_UNKNOWN
} E_equipBoltTypes;


typedef enum
{
    THING_TYPE_RUNE,
    THING_TYPE_KEY,
    THING_TYPE_THING,
    THING_TYPE_UNKNOWN
} E_equipThingTypes;


typedef struct
{
    /* equipment type of item */
    T_byte8 /*E_equipObjectTypes*/ type;

    /* equipment sub-type of item */
    T_byte8 subtype;

    /* number of items stackable */
    T_byte8 numstackable;

    /* effect enums */
    T_byte8 /*E_effectTriggerType*/  effectTriggerOn[MAX_ITEM_EFFECTS];
    T_byte8 /*E_effectType */        effectType[MAX_ITEM_EFFECTS];
    T_word16             effectData[MAX_ITEM_EFFECTS][3];

    /* object destroy after action enum */
    T_byte8 /*E_effectTriggerType*/ objectDestroyOn;

    /* useable by class bitflags */
    T_word16 useable;

    /* unique flag */
    T_byte8 unique;
} T_equipItemDescription;


typedef struct
{
    /* equipment type of item */
    T_byte8 type;

    /* equipment sub-type of item */
    T_byte8 subtype;

    /* number of items stackable */
    T_byte8 numstackable;

    /* effect enums */
    T_byte8              effectTriggerOn[MAX_ITEM_EFFECTS];
    T_byte8              effectType[MAX_ITEM_EFFECTS];
    T_word16             effectData[MAX_ITEM_EFFECTS][3];

    /* object destroy after action enum */
    T_byte8 objectDestroyOn;

    /* useable by class bitflags */
    T_word16 useable;

    /* unique flag */
    T_byte8 unique;

} T_equipItemDescriptionBorland;



#endif

