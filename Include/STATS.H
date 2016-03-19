/****************************************************************************/
/*    FILE:  STATS.H                                                        */
/****************************************************************************/
#ifndef _STATS_H_
#define _STATS_H_

#include "EQUIP.H"
#include "FORM.H"
#include "GENERAL.H"
#include "PACKET.H"

/* UI items for Load/Display character UI form: */
#define LOAD_CHARACTER_BEGIN_BUTTON                 300
#define LOAD_CHARACTER_MAIL_BUTTON                  301
#define LOAD_CHARACTER_EXIT_BUTTON                  302
#define LOAD_CHARACTER_PASSWORD_TEXT                501

#define MIN_HEALTH -10
#define MAX_DAMAGEAMT 1500
#define MAX_ATTRIBUTES 50
#define NUM_CLASSES 11
#define NUM_TITLES_PER_CLASS 20
#define NUM_ATTRIBUTES 6
#define STATS_MAX_COINS 19999
#define NUM_HOUSES 100
#define MAX_NOTES 1024
#define MAX_JUMP_POWER 60000
#define MAX_NOTE_SIZE 1000

typedef enum
{
    ATTRIBUTE_STRENGTH,
    ATTRIBUTE_SPEED,
    ATTRIBUTE_MAGIC,
    ATTRIBUTE_ACCURACY,
    ATTRIBUTE_STEALTH,
    ATTRIBUTE_CONSTITUTION,
    ATTRIBUTE_UNKNOWN
} E_statsAttributeType;


typedef enum
{
    CLASS_CITIZEN,
    CLASS_KNIGHT,
    CLASS_MAGE,
    CLASS_WARLOCK,
    CLASS_PRIEST,
    CLASS_ROGUE,
    CLASS_ARCHER,
    CLASS_SAILOR,
    CLASS_PALADIN,
    CLASS_MERCENARY,
    CLASS_MAGICIAN,
    CLASS_UNKNOWN
} E_statsClassType;


typedef enum
{
    ADVENTURE_NONE,
    ADVENTURE_1,
    ADVENTURE_2,
    ADVENTURE_3,
    ADVENTURE_4,
    ADVENTURE_5,
    ADVENTURE_6,
    ADVENTURE_7,
    ADVENTURE_UNKNOWN
} E_statsAdventureNumber;

/* saved character ID list to be loaded from server */
#define MAX_CHARACTERS_PER_SERVER 5

/* list of character status(exactly the size of a byte, */
typedef T_byte8 E_statsCharacterStatus ;
#define CHARACTER_STATUS_OK                  0
#define CHARACTER_STATUS_DEAD                1
#define CHARACTER_STATUS_TAGGED_FOR_REMOVAL  2
#define CHARACTER_STATUS_UNAVAILABLE         3
#define CHARACTER_STATUS_UNDEFINED           4
#define CHARACTER_STATUS_UNKNOWN             5

#define STATS_CHARACTER_NAME_MAX_LENGTH      30 // includes null at end
#define MAX_SIZE_PASSWORD 12

typedef struct
{
    T_byte8 name[STATS_CHARACTER_NAME_MAX_LENGTH];
    T_byte8 password[MAX_SIZE_PASSWORD];
    E_statsCharacterStatus status;
    T_byte8 mail;
} T_statsSavedCharacterID;

/* LES 03/06/96: */
/* This places all the pertenent info into ONE structure that */
/* can be passed around without knowing how many elements are */
/* in the array. */
typedef struct
{
    T_statsSavedCharacterID chars[MAX_CHARACTERS_PER_SERVER] ;
} T_statsSavedCharArray ;

#define MAX_PAST_PLACES  30

typedef struct {
    T_word16 adventureNumber ;  /* Map id of first level of adventure */
                                /*(usually 1-100, never 0) */
    T_word16 lastLevelNumber ;  /* if 0, never explored past first level. */
} T_pastPlace ;

typedef struct  {
    T_word16 numInList ;
    T_pastPlace places[MAX_PAST_PLACES] ;
} T_pastPlaces ;

/** Player statistics structure **/
/** AMT - 7/11/95 **/
typedef struct _T_playerStats
{
   T_byte8             Name[STATS_CHARACTER_NAME_MAX_LENGTH];
   T_byte8             ClassName[30];
   T_byte8             ClassTitle[35];

   T_sword32           Health;
   T_sword32           MaxHealth;
   T_sword32           Mana;
   T_sword32           MaxMana;

   T_sword16           Food;
   T_sword16           MaxFood;
   T_sword16           Water;
   T_sword16           MaxWater;

   T_sword16           PoisonLevel;
   T_sword16           RegenHealth;
   T_sword16           RegenMana;

   T_word16            JumpPower;
   T_word16            JumpPowerMod;
   T_word16            Tallness;
   T_word16            ClimbHeight;
   T_word16            MaxVRunning;
   T_word16            MaxVWalking;

   T_word16            HeartRate;
   T_word16            MaxFallV;

   T_byte8             WeaponBaseDamage;
   T_sbyte8            WeaponBaseSpeed;
   T_word32            AttackSpeed;
   T_word16            AttackDamage;

   E_Boolean           playerisalive;

   T_byte8 /*E_statsClassType*/    ClassType;
   T_byte8             Attributes[NUM_ATTRIBUTES];
   T_sword16           AttributeMods[NUM_ATTRIBUTES];
   T_sword16           Coins[EQUIP_TOTAL_COIN_TYPES];
   T_sword16           Bolts[EQUIP_TOTAL_BOLT_TYPES];
   T_sword16           SavedCoins[EQUIP_TOTAL_COIN_TYPES];
   /* specific armor values for each location */
   T_byte8             ArmorValues[EQUIP_NUMBER_OF_LOCATIONS];
   /* total armor level */
   T_byte8             ArmorLevel;
   T_word16            Load;
   T_word16            MaxLoad;

   T_byte8             Level;
   T_word32            Experience;
   T_word32            ExpNeeded;

   T_byte8 /*E_spellsSpellSystemType*/ SpellSystem;
   T_byte8             ActiveRunes[9];

   E_Boolean           HouseOwned[NUM_HOUSES];
   T_byte8             HasNotes[(MAX_NOTES/8)+1];
   T_word16            NumNotes;

   T_byte8             Notes[MAX_NOTE_SIZE];
   T_byte8             Identified[8193];
   T_byte8             password[MAX_SIZE_PASSWORD];

   T_byte8             CompletedAdventure;
   T_word16            CompletedMap;
   T_word16            CurrentQuestNumber;

   T_pastPlaces        pastPlaces ;
} T_playerStats;

extern T_playerStats *G_activeStats ;

/** Convenience macros **/

#define StatsGetPlayerHealth()(G_activeStats->Health)
#define StatsGetPlayerMaxHealth()(G_activeStats->MaxHealth)
#define StatsGetPlayerMana()(G_activeStats->Mana)
#define StatsGetPlayerMaxMana()(G_activeStats->MaxMana)
#define StatsGetPlayerFood()(G_activeStats->Food)
#define StatsGetPlayerMaxFood()(G_activeStats->MaxFood)
#define StatsGetPlayerWater()(G_activeStats->Water)
#define StatsGetPlayerMaxWater()(G_activeStats->MaxWater)
#define StatsGetPlayerCoins(type)(G_activeStats->Coins[(type)])
#define StatsGetPlayerBolts(type)(G_activeStats->Bolts[(type)])
#define StatsGetPlayerSavedCoins(type)(G_activeStats->SavedCoins[(type)])
#define StatsGetPlayerArmorValue()(G_activeStats->ArmorLevel)
#define StatsGetPlayerLoad()(G_activeStats->Load)
#define StatsGetPlayerMaxLoad()(G_activeStats->MaxLoad)
#define StatsGetTallness()(G_activeStats->Tallness)
#define StatsGetClimbHeight()(G_activeStats->ClimbHeight)
#define StatsGetMaxVRunning()(G_activeStats->MaxVRunning)
#define StatsGetMaxVWalking()(G_activeStats->MaxVWalking)
#define StatsGetMaxFallV()(G_activeStats->MaxFallV)
#define StatsGetManaLeft()(G_activeStats->Mana)
#define StatsGetName()(G_activeStats->Name)
#define StatsGetPlayerClassType()(G_activeStats->ClassType)
#define StatsGetPlayerLevel()(G_activeStats->Level)
#define StatsGetPlayerStrength()(G_activeStats->Attributes[ATTRIBUTE_STRENGTH])
#define StatsGetPlayerConstitution()(G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION])
#define StatsGetPlayerAccuracy()(G_activeStats->Attributes[ATTRIBUTE_ACCURACY])
#define StatsGetPlayerStealth()(G_activeStats->Attributes[ATTRIBUTE_STEALTH])
#define StatsGetPlayerMagic()(G_activeStats->Attributes[ATTRIBUTE_MAGIC])
#define StatsGetPlayerSpeed()(G_activeStats->Attributes[ATTRIBUTE_SPEED])
#define StatsGetPlayerClassName()(G_activeStats->ClassName)
#define StatsGetPlayerClassTitle()(G_activeStats->ClassTitle)
#define StatsGetPlayerExperience()(G_activeStats->Experience)
#define StatsGetPlayerExpNeeded()(G_activeStats->ExpNeeded)
#define StatsGetPlayerAttackSpeed()(G_activeStats->AttackSpeed)
#define StatsGetPlayerSpellSystem()(G_activeStats->SpellSystem)
#define StatsGetCompletedAdventure()(G_activeStats->CompletedAdventure)
#define StatsGetCompletedMap()(G_activeStats->CompletedMap)
#define StatsGetCurrentQuestNumber()(G_activeStats->CurrentQuestNumber)
#define StatsSetJumpPower(amt)(G_activeStats->JumpPower =(amt))
#define StatsSetMaxVRunning(amt)(G_activeStats->MaxVRunning =(amt))
#define StatsSetMaxVWalking(amt)(G_activeStats->MaxVWalking =(amt))
#define StatsSetMaxFallV(amt)(G_activeStats->MaxFallV =(amt))
#define StatsPlayerIsAlive()(G_activeStats->playerisalive)
#define StatsPlayerSetAlive()(G_activeStats->playerisalive = TRUE)
#define StatsSetPlayerClassType(amt)(G_activeStats->ClassType=(amt))
#define StatsSetPlayerStrength(amt)(G_activeStats->Attributes[ATTRIBUTE_STRENGTH] =(amt))
#define StatsSetPlayerMaxHealth(amt)(G_activeStats->MaxHealth+=(amt))
#define StatsSetPlayerConstitution(amt)(G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION] =(amt))
#define StatsSetPlayerAccuracy(amt)(G_activeStats->Attributes[ATTRIBUTE_ACCURACY] =(amt))
#define StatsSetPlayerStealth(amt)(G_activeStats->Attributes[ATTRIBUTE_STEALTH] =(amt))
#define StatsSetPlayerMagic(amt)(G_activeStats->Attributes[ATTRIBUTE_MAGIC] =(amt))
#define StatsSetPlayerSpeed(amt)(G_activeStats->Attributes[ATTRIBUTE_SPEED] =(amt))
#define StatsSetPlayerAttribute(num,amt)(G_activeStats->Attributes[(num)]=(amt))
#define StatsSetPlayerCoins(type,amt)(G_activeStats->Coins[(type)] =(amt))
#define StatsSetPlayerSavedCoins(type,amt)(G_activeStats->SavedCoins[(type)] =(amt))
#define StatsGetPlayerPoisonLevel()(G_activeStats->PoisonLevel)
#define StatsSetPlayerPoisonLevel(amt)(G_activeStats->PoisonLevel =(amt))
#define StatsSetPlayerLoad(amt)(G_activeStats->Load =(amt))
#define StatsSetPlayerFood(amt)(G_activeStats->Food =(amt))
#define StatsSetPlayerWater(amt)(G_activeStats->Water =(amt))
#define StatsGetPastPlaces()(&G_activeStats->pastPlaces)
#define StatsSetCompletedAdventure(amt)(G_activeStats->CompletedAdventure =(amt))
#define StatsSetCompletedMap(amt)(G_activeStats->CompletedMap =(amt))
#define StatsSetCurrentQuestNumber(amt)(G_activeStats->CurrentQuestNumber =(amt))

/** Function prototypes **/
T_void StatsInit(T_void);
T_void StatsSetPlayerMana(T_word16 value);
T_void StatsSetPlayerHealth(T_word16 value);
T_void StatsChangePlayerMana(T_sword16 amt);
T_void StatsChangePlayerHealth(T_sword16 amt);
T_void StatsTakeDamage(T_word16 type, T_word16 amt);
T_void StatsChangePlayerExperience(T_sword32 amt);
T_void StatsChangePlayerLoad(T_sword16 amt);
T_void StatsChangePlayerFood(T_sword16 amt);
T_void StatsChangePlayerWater(T_sword16 amt);
T_void StatsChangePlayerHealthRegen(T_sword16 amt);
T_void StatsChangePlayerManaRegen(T_sword16 amt);
T_void StatsChangePlayerPoisonLevel(T_sword16 amt) ;

T_void StatsUpdatePlayerStatistics(T_void);
//T_void StatsHurtPlayer(T_word16 amt);
//T_void StatsHealPlayer(T_word16 amt);
E_Boolean StatsIOwnHouse(T_word32 houseNum);
T_void    StatsSetIOwnHouse(T_word32 houseNum, E_Boolean to);

T_void StatsSetActive(T_playerStats *p_stats) ;

T_byte8 *StatsGetCharacterList(T_void);

E_Boolean StatsPlayerHasNotePage(T_word16 whichpage);
T_void    StatsAddPlayerNotePage(T_word16 whichpage);
T_void    StatsRemovePlayerNotePage(T_word16 whichpage);
T_word16  StatsGetNumberOfNotes(T_void);
T_word16  StatsGetPlayerNotePageID(T_word16 pageIndex);
T_byte8*  StatsGetPlayerNotes(T_void);
T_void    StatsSetPlayerNotes(T_byte8 *fieldData);

#ifndef SERVER_ONLY
E_Boolean StatsCreateCharacterUI(T_void);
T_void StatsCreateCharacterControl(E_formObjectType objtype,
					            	T_word16 objstatus,
					             	T_word32 objID);
T_void StatsCreateCharacterUIInit(T_void);


E_Boolean StatsLoadCharacterUI(T_byte8 selection);
T_void StatsLoadCharacterControl(E_formObjectType objtype,
					            	T_word16 objstatus,
					             	T_word32 objID);
T_void StatsLoadCharacterUIInit(T_void);

T_void StatsDisplayStatisticsPage(T_void);
E_Boolean StatsAddCoin(E_equipCoinTypes type, T_sword16 num);
E_Boolean StatsAddBolt(E_equipBoltTypes type, T_sword16 num);
T_void StatsSetArmorValue(E_equipLocations location, T_byte8 value);
T_void StatsCalcAverageArmorValue(T_void);
T_byte8 StatsGetArmorValue(T_byte8 location);

T_void StatsSetWeaponBaseDamage(T_byte8 damage);
T_void StatsSetWeaponBaseSpeed(T_sbyte8 speed);

T_void StatsCalcPlayerAttackSpeed(T_void);
T_void StatsCalcPlayerAttackDamage(T_void);

T_word16 StatsGetPlayerAttackDamage(T_void);

T_void   StatsChangePlayerAttributeMod(E_statsAttributeType attribute, T_sbyte8 value);
T_void   StatsClearPlayerAttributeMod(E_statsAttributeType attribute);
T_sword16 StatsGetPlayerAttributeMod(E_statsAttributeType attribute);
T_word16  StatsGetPlayerAttribute(E_statsAttributeType attribute);
T_word16  StatsGetPlayerAttributeNoMod(E_statsAttributeType attribute);
T_void StatsToggleGodMode(T_void);
E_Boolean StatsGodModeIsOn(T_void);
T_void StatsIncrementRuneCount(T_byte8 which);
T_void StatsDecrementRuneCount(T_byte8 which);
E_Boolean StatsRuneIsAvailable(T_byte8 which);

T_word32 StatsGetJumpPower(void);
T_void StatsModJumpPower(T_sword16 mod);

E_Boolean StatsLoadCharacter(T_byte8 selected);
E_Boolean StatsSaveCharacter(T_byte8 selected);
E_Boolean StatsDeleteCharacter(T_byte8 selected);
T_void StatsUnloadCharacter(T_void) ;

T_void StatsSetPassword(
           T_byte8 selected,
           T_byte8 password[MAX_SIZE_PASSWORD]) ;
T_void StatsGetPassword(
           T_byte8 selected,
           T_byte8 password[MAX_SIZE_PASSWORD]) ;

T_byte8 StatsGetActive(T_void);
T_void StatsMakeActive(T_byte8 selected) ;
T_statsSavedCharacterID* StatsGetSavedCharacterIDStruct(T_word16 selected);

T_statsSavedCharArray *StatsGetSavedCharacterList(T_void) ;
T_void StatsSetSavedCharacterList(T_statsSavedCharArray *p_chars) ;
T_void StatsClearSavedCharacterList(T_void) ;

T_void StatsLoadCharacterUIStart(T_void) ;
T_void StatsLoadCharacterUIUpdate(T_void) ;
T_void StatsLoadCharacterUIEnd(T_void) ;

T_word32 StatsComputeCharacterChecksum(T_void) ;

T_void StatsCreateCharacterUIStart(T_void) ;
T_void StatsCreateCharacterUIEnd(T_void) ;
T_void StatsCreateCharacterUIUpdate(T_void) ;

T_word32 StatsGetPlayerTotalSavedWealth(T_void);
T_word32 StatsGetPlayerTotalCarriedWealth(T_void);

T_void StatsChangePlayerTotalCarriedWealth(T_sword32 amount);
T_void StatsDrawCharacterPortrait(T_word16 x1, T_word16 y1);

E_Boolean StatsHitWasCritical(T_void);
E_effectDamageType StatsGetPlayerDamageType(T_void);

E_Boolean StatsPlayerHasIdentified(T_word16 objType);
T_void    StatsPlayerIdentify(T_word16 objType);
T_void StatsCalcPlayerMovementSpeed(T_void);

/* Past place information stored in the current character. */
T_void StatsUpdatePastPlace(
           T_word16 adventureNumber,
           T_word16 mapNumber) ;
T_word16 StatsFindPastPlace(T_word16 adventureNumber) ;

char* StatsGetClassTitle(T_byte8 classType);

#endif /* SERVER_ONLY */

#endif
