/****************************************************************************/
/*    FILE:  STATS.C                                                        */
/****************************************************************************/
#include "BANNER.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "COMWIN.H"
#include "FILE.H"
#include "INVENTOR.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "NOTES.H"
#include "OBJECT.H"
#include "PICS.H"
#include "PLAYER.H"
#include "PROMPT.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SMCCHOOS.H"
#include "SPELLS.H"
#include "SPELTYPE.H"
#include "STATS.H"
#include "TICKER.H"
#include "VIEW.H"
#include <direct.h>

static E_Boolean G_exit=FALSE;
static E_Boolean G_statsLCExit=FALSE;
static E_Boolean G_success=FALSE;
static E_Boolean G_statsLCSuccess=FALSE;
static E_Boolean G_playNextHurtSound=TRUE;
static E_Boolean G_hitWasCritical=FALSE;
static E_statsClassType G_charTypeSelected=CLASS_CITIZEN;

#define NO_CHARACTER_LOADED   0xFF
static T_byte8 G_activeCharacter=NO_CHARACTER_LOADED;

/* LES: 06/25/96 */
static T_byte8 G_lastLoadedCharacter = NO_CHARACTER_LOADED ;

/* Current list of saved characters */
/* To be updated on startup */
static T_statsSavedCharacterID G_savedCharacters[MAX_CHARACTERS_PER_SERVER];

/* current server connection ID */
/* to be moved to client.c (probably...) */
static T_word32 G_serverID=0;

/* local routines */
static T_void StatsCalcClassStats (T_void);
static T_void StatsUpdateCreateCharacterUI (T_void);
static T_void StatsCalcPlayerMaxLoad (T_void);
static T_void StatsReorientPlayerView (T_void);
const T_byte8 *G_statsCharacterTypeNames[NUM_CLASSES]={"Citizen",
                                                  "Knight",
                                                  "Mage",
                                                  "Warlock",
                                                  "Priest",
                                                  "Rogue",
                                                  "Archer",
                                                  "Sailor",
                                                  "Paladin",
                                                  "Mercenary",
                                                  "Magician"};

const T_byte8 G_statsCharacterAttributes[NUM_CLASSES][NUM_ATTRIBUTES]=
                                                  {{25,25,25,25,25,25},
                                                   {35,15,10,20,15,35},
                                                   {10,25,35,25,20,15},
                                                   {25,20,30,20,15,20},
                                                   {15,20,35,15,20,25},
                                                   {15,30,15,25,30,15},
                                                   {20,20,15,35,20,20},
                                                   {25,25,10,20,15,35},
                                                   {30,20,25,15,10,30},
                                                   {30,20,10,25,20,25},
                                                   {15,30,30,20,20,15}};

const T_byte8 G_statsCharacterAdvancements[NUM_CLASSES][NUM_ATTRIBUTES]=
                                                  {{2,2,2,2,2,2},
                                                   {3,2,1,2,1,3},
                                                   {1,2,3,3,2,1},
                                                   {2,2,2,2,2,2},
                                                   {2,1,3,2,2,2},
                                                   {2,2,1,2,3,2},
                                                   {2,2,2,3,1,2},
                                                   {2,2,1,2,2,3},
                                                   {3,2,2,2,1,2},
                                                   {2,2,2,2,2,2},
                                                   {1,3,3,2,1,2}};


const T_byte8 *G_statsLevelTitles[NUM_CLASSES][NUM_TITLES_PER_CLASS]=
                                             {{"Serviceman",
                                               "Color Bearer",
                                               "King's Militia",
                                               "Loyalist",
                                               "Crown's Servant",
                                               "Crown's Fighter",
                                               "Warrior",
                                               "Defender",
                                               "Noble",
                                               "Royal Courtsman",
                                               "Brigade Elmore",
                                               "Righteous Band",
                                               "League of Aeneas",
                                               "Esias's Servant",
                                               "Exalted Guardian",
                                               "Hero",
                                               "Court Knight",
                                               "Guard Commander",
                                               "Supreme General",
                                               "The Mighty"},

                                               {"Page",
                                               "Trainee",
                                               "Squire",
                                               "Swordsman",
                                               "Enlisted Servant",
                                               "Tactical Student",
                                               "Swordsmaster",
                                               "Court Guard",
                                               "Armsmaster",
                                               "Knight",
                                               "Sect of Elmore",
                                               "Royal Captain",
                                               "Royal General",
                                               "King's Guardian",
                                               "Vanquisher",
                                               "Sword of Titas",
                                               "Glory of Aelia",
                                               "Horn of Domitian",
                                               "Title of Omega",
                                               "Dragonslayer"},

                                               {"Student",
                                               "Apprentice",
                                               "Spellcaster",
                                               "Mage",
                                               "Conjurer",
                                               "Enchanter",
                                               "Wizard",
                                               "Lore Master",
                                               "Dark Spirit",
                                               "Black Art Scribe",
                                               "Black Art Master",
                                               "Magi of the Fire",
                                               "Magi of the Star",
                                               "Sorcerer",
                                               "Order of Nu'ak",
                                               "Order of Tul",
                                               "Order of Ahnul",
                                               "Order of Baal",
                                               "Prophet of Baal",
                                               "Ancient One"},

                                               {"Disciple",
                                               "Apprentice",
                                               "Dark Artesian",
                                               "Warlock",
                                               "Mighty Sword",
                                               "Vanquisher",
                                               "Legion",
                                               "Destroyer",
                                               "Mystic Conquerer",
                                               "1st Circle Order",
                                               "2nd Circle Order",
                                               "3rd Circle Order",
                                               "Summoner",
                                               "Seeker",
                                               "Destructor",
                                               "Desolator",
                                               "Dragon Spirit",
                                               "Ruiner",
                                               "Death's Follower",
                                               "Death Incarnate"},

                                               {"Flock Tender",
                                               "Student",
                                               "Scribe",
                                               "Teacher",
                                               "Evangelist",
                                               "Cleric",
                                               "Elder",
                                               "The Annointed",
                                               "Parable Master",
                                               "Master of Canon",
                                               "Bishop",
                                               "Minor Prophet",
                                               "Cross Bearer",
                                               "Disciple",
                                               "Sacred Branch",
                                               "Witness",
                                               "Apostle",
                                               "Saint",
                                               "Herald of Glory",
                                               "Illuminated One"},

                                               {"Sneak",
                                               "Shadow",
                                               "Pickpocket",
                                               "Rogue",
                                               "Trickster",
                                               "Contriver",
                                               "Thief",
                                               "Knave",
                                               "Skilled Con",
                                               "Scoundrel",
                                               "Nighthawk",
                                               "Teacher",
                                               "1st Sect Member",
                                               "2nd Sect Member",
                                               "3rd Sect Member",
                                               "Master of the Art",
                                               "Stealth Master",
                                               "The Upright",
                                               "Master Thief",
                                               "Legendary Delver"},

                                               {"Fletcher",
                                               "Journeyman",
                                               "Page",
                                               "Squire",
                                               "Marksman",
                                               "Targetman",
                                               "Noble",
                                               "Sharpshooter",
                                               "Warrior",
                                               "Acurate",
                                               "Archer",
                                               "Forest Arrow",
                                               "CraftMaster",
                                               "Ancient Arrow",
                                               "Huntsman (1rd)",
                                               "Huntsman (2nd)",
                                               "Huntsman (3rd)",
                                               "Divine Missile",
                                               "Unerring One",
                                               "The Mighty"},

                                               {"Landlubber",
                                               "Trainee",
                                               "Galley Grunt",
                                               "Seaman",
                                               "Mate",
                                               "First Mate",
                                               "Sailor",
                                               "Veteran",
                                               "Captain",
                                               "Degree Hellespont",
                                               "Order of Salamis",
                                               "Golden Horn",
                                               "Seamaster",
                                               "Sextant Lord",
                                               "Admiral",
                                               "Water Magician",
                                               "Fury",
                                               "Order of Ulysses",
                                               "Prince of Havens",
                                               "Poseidon's Guide"},

                                               {"Page",
                                               "Squire",
                                               "Servant",
                                               "Good Knight",
                                               "The Annointed",
                                               "Blood Knight",
                                               "Warrior",
                                               "Son of Thunder",
                                               "Blessed Sword",
                                               "Crusader",
                                               "Champion",
                                               "Disciple",
                                               "Vanquisher",
                                               "Illuminated",
                                               "Cross Bearer",
                                               "Saint of Steel",
                                               "Soldier of God",
                                               "Avenger of God",
                                               "Wrath of God",
                                               "Hand of God"},

                                               {"Brawler",
                                               "Fighter",
                                               "Hireling",
                                               "Ransacker",
                                               "Wolf of War",
                                               "Soldier Fortune",
                                               "Destroyer",
                                               "Valorian Vitorix",
                                               "Legionaire",
                                               "Asassin",
                                               "Axe of Alesia",
                                               "Avenger",
                                               "Theo's Sword",
                                               "Siege Master",
                                               "Captain",
                                               "Son of Sparta",
                                               "The Janissary",
                                               "Hero",
                                               "ArmsMaster",
                                               "Casca"},

                                               {"Pupil",
                                               "Student",
                                               "Trickster",
                                               "Showman",
                                               "Charlatan",
                                               "Magician",
                                               "Deceiver",
                                               "Charmer",
                                               "Mystic",
                                               "Illusionist",
                                               "RuneMaster",
                                               "Seer",
                                               "Sage",
                                               "Dark Oracle",
                                               "Student of Rit",
                                               "Keeper of Rit",
                                               "Teacher of Rit",
                                               "Tower Artesian",
                                               "Master Sage",
                                               "Dark Prophet"}};

#define FLAG_ALLOW_TO_DIE

/* Current group of stats to use. */
T_playerStats *G_activeStats ;

T_void StatsInit (T_void)
{
	T_word16 i;

	DebugRoutine ("StatsInit");

   EffectRemoveAllPlayerEffects();
   InventoryRemoveEquippedEffects();

//   G_activeStats->HeartRate = 60;
   G_activeStats->MaxFallV = 31000;
   G_activeStats->playerisalive=FALSE;

   G_activeStats->ClassType=CLASS_CITIZEN;

   /* intial attributes */
   for (i=0;i<NUM_ATTRIBUTES;i++)
   G_activeStats->Attributes[i]=G_statsCharacterAttributes[G_activeStats->ClassType][i];

   G_activeStats->Food = 2000;
   G_activeStats->MaxFood=2000;
   G_activeStats->Water= 2000;
   G_activeStats->MaxWater=2000;
   G_activeStats->PoisonLevel = 0;
   G_activeStats->JumpPower = 75<<8;
   G_activeStats->Tallness = 50;
   G_activeStats->ClimbHeight = 40;

   G_activeStats->Coins[COIN_TYPE_COPPER]=0;
   G_activeStats->Coins[COIN_TYPE_SILVER]=0;
   G_activeStats->Coins[COIN_TYPE_GOLD]=0;
   G_activeStats->Coins[COIN_TYPE_PLATINUM]=0;
   G_activeStats->Bolts[BOLT_TYPE_NORMAL]=0;
   G_activeStats->Bolts[BOLT_TYPE_POISON]=0;
   G_activeStats->Bolts[BOLT_TYPE_PIERCING]=0;
   G_activeStats->Bolts[BOLT_TYPE_FIRE]=0;
   G_activeStats->Bolts[BOLT_TYPE_ELECTRICITY]=0;
   G_activeStats->Bolts[BOLT_TYPE_MANA_DRAIN]=0;
   G_activeStats->Bolts[BOLT_TYPE_ACID]=0;
   G_activeStats->SavedCoins[COIN_TYPE_COPPER]=0;
   G_activeStats->SavedCoins[COIN_TYPE_SILVER]=0;
   G_activeStats->SavedCoins[COIN_TYPE_GOLD]=0;
   G_activeStats->SavedCoins[COIN_TYPE_PLATINUM]=0;

   /* set armor values */
   for (i=0;i<EQUIP_NUMBER_OF_LOCATIONS;i++)
   {
     G_activeStats->ArmorValues[i]=0;
   }
   G_activeStats->ArmorLevel=0;

   G_activeStats->Load = 0;
   G_activeStats->JumpPowerMod=0;
   G_activeStats->Level = 1;
   G_activeStats->Experience=0;
   G_activeStats->ExpNeeded=2000;

   sprintf (G_activeStats->Name,"");

   /* clear attribute modifers */
   for (i=0;i<ATTRIBUTE_UNKNOWN;i++)
   {
    StatsClearPlayerAttributeMod (i);
   }

   /* clear carried runes */
   for (i=0;i<9;i++) G_activeStats->ActiveRunes[i]=0;

   /* clear houses owned */
   for (i=0;i<NUM_HOUSES;i++)
   {
        G_activeStats->HouseOwned[i]=FALSE;
   }

   /* clear journal notes */
   for (i=0;i<(MAX_NOTES/8)+1;i++)
   {
        G_activeStats->HasNotes[i]=0;
   }

   G_activeStats->NumNotes=0;

   /* clear identifiers */
   for (i=0;i<8193;i++)
   {
       G_activeStats->Identified[i]=0;
   }

   /* clear notes */
   strcpy (G_activeStats->Notes,"");

   /* clear inventory */
   InventoryClear (INVENTORY_STORE);
   InventoryClear (INVENTORY_PLAYER);

   G_activeStats->CompletedAdventure = 0;
   G_activeStats->CompletedMap = 0;
   G_activeStats->CurrentQuestNumber = 0;

   StatsCalcClassStats();

   G_lastLoadedCharacter = NO_CHARACTER_LOADED ;
   /* Clear out the history of past places */
   memset(&G_activeStats->pastPlaces, 0, sizeof(G_activeStats->pastPlaces)) ;

   DebugEnd();
}


T_void StatsCalcClassStats (T_void)
{
   DebugRoutine ("StatsCalcClassStats");

   /* calculated attributes */
   G_activeStats->Health = (10+(G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION]/2)) * 100 ;
   G_activeStats->MaxHealth = G_activeStats->Health;
   G_activeStats->Mana = G_activeStats->Attributes[ATTRIBUTE_MAGIC] * 100;
   G_activeStats->MaxMana = G_activeStats->Mana;

   /* calc max load */
   StatsCalcPlayerMaxLoad();

   /* calculate jump power */
   G_activeStats->JumpPower = 75<<8;

   /* calculate Tallness */
   G_activeStats->Tallness = 50;

   /* calculate Climbheight */
   G_activeStats->ClimbHeight = 40;

   /* calculate maxvrunning */
   /* calculate maxvwalking */
   StatsCalcPlayerMovementSpeed();

   G_activeStats->RegenHealth = G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION]*
                                (G_activeStats->Level+5);

   G_activeStats->RegenMana = G_activeStats->Attributes[ATTRIBUTE_MAGIC]*
                              (G_activeStats->Level+5);
   /* set some class specific bonuses */
   switch (G_activeStats->ClassType)
   {
        case CLASS_WARLOCK:
			G_activeStats->RegenMana=(G_activeStats->RegenMana*3)/2;
			break;

        case CLASS_PRIEST:
			G_activeStats->RegenMana*=3;
			break;

        case CLASS_ROGUE:
			G_activeStats->JumpPower=(G_activeStats->JumpPower*3)/2;
			break;

        case CLASS_ARCHER:
			break;

        case CLASS_SAILOR:
			break;

        case CLASS_PALADIN:
			break;

        case CLASS_MERCENARY:
			break;

        case CLASS_MAGICIAN:
			G_activeStats->RegenMana=(G_activeStats->RegenMana*3)/2;
			break;

        case CLASS_MAGE:
			G_activeStats->RegenMana *= 3;
			break;

        case CLASS_CITIZEN:
        case CLASS_UNKNOWN:
			default:
			break;
   }

   /* set class name / title */
   strcpy (G_activeStats->ClassName,G_statsCharacterTypeNames[G_activeStats->ClassType]);

   if (G_activeStats->Level < 21)
   {
        strcpy (G_activeStats->ClassTitle,G_statsLevelTitles[G_activeStats->ClassType][G_activeStats->Level-1]);
   }

   /* base for hand */
   StatsSetWeaponBaseDamage (3);
   StatsSetWeaponBaseSpeed (0);

   /* init spell casting type available for classtype */
   switch (G_activeStats->ClassType)
   {
        case CLASS_MAGE:
        G_activeStats->SpellSystem=SPELL_SYSTEM_MAGE;
        break;

        case CLASS_WARLOCK:
        G_activeStats->SpellSystem=SPELL_SYSTEM_MAGE;
        break;

        case CLASS_PRIEST:
        G_activeStats->SpellSystem=SPELL_SYSTEM_CLERIC;
        break;

        case CLASS_PALADIN:
        G_activeStats->SpellSystem=SPELL_SYSTEM_CLERIC;
        break;

        case CLASS_MAGICIAN:
        G_activeStats->SpellSystem=SPELL_SYSTEM_MAGE;
        break;

        case CLASS_UNKNOWN:
        default:
        G_activeStats->SpellSystem = SPELL_SYSTEM_ARCANE;
        break;
    }

   DebugEnd();
}



T_void StatsSetPlayerHealth (T_word16 amount)
{
	DebugRoutine ("StatsSetPlayerHealth");

    if (amount > G_activeStats->MaxHealth) amount = G_activeStats->MaxHealth;

	G_activeStats->Health = amount;
//	G_activeStats->playerisalive=FALSE;

	DebugEnd();
}

T_void StatsSetPlayerMana (T_word16 amount)
{
    DebugRoutine ("StatsSetPlayerMana");

    if (amount > G_activeStats->MaxMana) amount = G_activeStats->MaxMana;

    G_activeStats->Mana = amount;

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  StatsChange .................                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Modifies a statistics value.  These functions replace macros for      */
/*    purposes of error checking, displaying messages, clipping, ect.       */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  01/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsChangePlayerMana (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerMana");

    if (amt > 0 && (G_activeStats->Mana < G_activeStats->MaxMana))
    {
        /* mana gain */
        SoundPlayByNumber(2012, 128) ;
        MessageAdd ("^009You feel a surge of power!");
        ColorAddGlobal (amt>>6,amt>>6,0);
//      MessageAdd ("^036You feel a surge of power!");
        /* screen bright purple */
    } else if (amt < 0 && G_activeStats->Mana > 0)
    {
        /* mana drain */
//        ColorAddGlobal (0,amt>>6,0);
        /* screen dark purple */
    }

    G_activeStats->Mana += amt;

	if (G_activeStats->Mana>G_activeStats->MaxMana)
		G_activeStats->Mana = G_activeStats->MaxMana;
	if (G_activeStats->Mana<0)
       G_activeStats->Mana = 0;

    /* update the bottom banner status display */
    BannerStatusBarUpdate();

    DebugEnd();
}


T_void StatsChangePlayerHealth (T_sword16 amt)
{
    T_word16 real_damage;
    T_byte8 num;

    DebugRoutine ("StatsChangePlayerHealth");

    if (amt > 0)
    {
        /* must be a healing spell */
        SoundPlayByNumber(2012, 128) ;
	    ColorAddGlobal (amt>>5,amt>>5,amt>>5);
        MessageAdd ("^009You feel healthier!");

        G_activeStats->Health += amt;
		if (G_activeStats->Health>G_activeStats->MaxHealth) G_activeStats->Health=G_activeStats->MaxHealth;
    }
    else if (amt < 0)
    {
        amt=-amt;
        /* ouch! */
        /* use armor here */
//        real_damage_pct=100-G_activeStats->ArmorLevel;
//        real_damage=amt*real_damage_pct;
//        real_damage/=100;
//        if (real_damage > MAX_DAMAGEAMT) real_damage=MAX_DAMAGEAMT;
        real_damage=amt;

        if (real_damage != 0)
            PlayerSetStance(STANCE_HURT) ;

//        sprintf (stmp,"You took %d damage",real_damage);
//        MessageAdd(stmp);
        /* check for invulnerability */
	    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_INVULNERABLE)==FALSE)
	    {
            if (G_playNextHurtSound==TRUE)
            {
                if (real_damage < (G_activeStats->Health>>2))
                {
                    /* Play a hurt sound. */
                    num=rand()%4;
                    PlayerMakeSoundLocal (SOUND_PLAYER_HURT_SET+num);
                }
                else
                {
                    /* play a hurtbad sound */
                    num=rand()%4;
                    PlayerMakeSoundLocal (SOUND_PLAYER_HURTBAD_SET+num);
                }
            }
            else G_playNextHurtSound=TRUE;

/*
            switch (G_activeStats->ClassType)
            {
                case CLASS_CITIZEN:
                case CLASS_PRIEST:
                case CLASS_ARCHER:
                SoundPlayByNumber(SOUND_CITIZEN_HURT_SET+num,255);
                break;

                case CLASS_KNIGHT:
                case CLASS_PALADIN:
                SoundPlayByNumber(SOUND_PALADIN_HURT_SET+num,255);
                break;

                case CLASS_MAGE:
                case CLASS_MAGICIAN:
                case CLASS_WARLOCK:
                SoundPlayByNumber(SOUND_MAGE_HURT_SET+num,255);
                break;

                case CLASS_ROGUE:
                case CLASS_SAILOR:
                case CLASS_MERCENARY:
                SoundPlayByNumber(SOUND_ROGUE_HURT_SET+num,255);
                break;

                default:
                DebugCheck(0);
                break;
            }
*/
//            if (num==0) SoundPlayByNumber (2000,255);
//            else if (num==1) SoundPlayByNumber (2003,255);
//            else if (num==2) SoundPlayByNumber (2004,255);
//            else SoundPlayByNumber (1001,255);

//            if (rand() & 2)
//  		        SoundPlayByName("ImHit1", 170);
//            else
//  		        SoundPlayByName("ImHit2", 170);


            if (G_activeStats->Health >= real_damage)
            {
		        if (EffectPlayerEffectIsActive (PLAYER_EFFECT_GOD_MODE)==FALSE)
                   G_activeStats->Health-=real_damage;
            }
            else
            {
                G_activeStats->Health = 0 ;
                if (EffectPlayerEffectIsActive (PLAYER_EFFECT_DEATH_WARD))
                {
                    StatsChangePlayerMana (-real_damage);
                }
            }

//		    ColorAddGlobal (amt>>2,-amt>>2,-amt>>2);
//            if (real_damage > 50 && real_damage < 150) MessageAdd ("^014Ouch!");
//            else if (real_damage > 150 && real_damage < 499) MessageAdd ("^014That HURT!!");
//            else MessageAdd ("^014Aieeee!!! That hurt!!!");
	    }
        else
        {
//            MessageAdd ("^003Fortunately for you, you are invulnerable");
        }
	    //code added 05/26/95 JDA

#ifdef FLAG_ALLOW_TO_DIE
        if (EffectPlayerEffectIsActive (PLAYER_EFFECT_DEATH_WARD)==FALSE)
        {
	        if (StatsGetPlayerHealth() <= 0)
	        {
                PlayerMakeSoundLocal (SOUND_PLAYER_DEAD);
//                PlayerSetStance(STANCE_DIE) ;
                /* play taps here ;() */
	            StatsSetPlayerHealth (G_activeStats->MaxHealth) ; //u died
                StatsSetPlayerMana (G_activeStats->MaxMana/2);
//                StatsSetPlayerFood(1000);
//                StatsSetPlayerWater (1000);

                StatsSetPlayerPoisonLevel(0) ;

                ClientDied() ;
/*
		        for (i=0;i<30;i++)
		        {
			        ColorAddGlobal (-5,-5,-5);
			        ColorUpdate(1);
			        delay(50);
		        }
		        PlayerStopMoving();
		        G_activeStats->playerisalive=FALSE;
		        ClientLogoff();
*/
            }
        }
#endif
    }

    /* update the bottom banner status display */
    BannerStatusBarUpdate();

    DebugEnd();
}

/* specific routine to handle acid, fire, poison, electricity, ect */
/* type is the damage type, amt is the amount of damage to take */
/* note that all physical damage calls (i.e. lava, attacks, fireballs, */
/* eating a poison apple, ect. should use this routine. */

T_void StatsTakeDamage (T_word16 type, T_word16 amt)
{
    static T_word16 acidcount=0;
    T_word16 chance;
    T_byte8 resist=0; /* number of resistances to damage effects */
    T_byte8 effects=0; /* number of damage effects */
    T_sword32 real_damage;
    T_sword32 real_damage_pct;
    E_equipArmorTypes armortype;
    E_equipLocations where;
    float resistpct;

    DebugRoutine ("StatsTakeDamage");
    DebugCheck (type != EFFECT_DAMAGE_UNKNOWN);
//MessagePrintf("TakeDamage %d from %s", amt, DebugGetCallerName()) ;
//    printf ("TakeDamage called with type=%d amt=%d\n",type,amt);
//    fflush (stdout);

    /* please note that type are bitflags specified in effect.h */
    DebugCheck (type < EFFECT_DAMAGE_BAD);

    /* The dead players don't take damage. */
    if (ClientIsDead())  {
        DebugEnd() ;
        return ;
    }

    /* Check to see if we are doing a special type of "damage" */
    if (type & EFFECT_DAMAGE_SPECIAL)  {
        /* Do a special effect */
        switch(type & (~EFFECT_DAMAGE_SPECIAL))  {
            case EFFECT_DAMAGE_SPECIAL_LOCK:
                break ;
            case EFFECT_DAMAGE_SPECIAL_UNLOCK:
                break ;
            case EFFECT_DAMAGE_SPECIAL_PUSH:
                PlayerMakeSoundLocal (SOUND_PLAYER_CONFUSED);
                break ;
            case EFFECT_DAMAGE_SPECIAL_PULL:
                PlayerMakeSoundLocal (SOUND_PLAYER_CONFUSED);
                break ;
            case EFFECT_DAMAGE_SPECIAL_BERSERK:
//                MessageAdd("Berserk damage") ;
                PlayerMakeSoundLocal (SOUND_PLAYER_PISSED);
                /* These special types don't affect player stats. */
                break ;
            case EFFECT_DAMAGE_SPECIAL_DISPEL_MAGIC:
//                MessageAdd("Dispel magic damage") ;
                PlayerMakeSoundLocal (SOUND_PLAYER_DRAINED);
                /* Loose one of those magical effects. */
                Effect(
                    EFFECT_REMOVE_RANDOM_SPELL,
                    EFFECT_TRIGGER_NONE,
                    0,
                    0,
                    0,
                    NULL) ;
                break ;
            case EFFECT_DAMAGE_SPECIAL_EARTHBIND:
//                MessageAdd("earthbind magic damage") ;
                /* You don't fly anymore. */
                Effect(
                    EFFECT_REMOVE_SPECIFIC_SPELL,
                    EFFECT_TRIGGER_NONE,
                    PLAYER_EFFECT_FLY,
                    0,
                    0,
                    NULL) ;
                break ;
            case EFFECT_DAMAGE_SPECIAL_CONFUSE:
                PlayerMakeSoundLocal (SOUND_PLAYER_CONFUSED);
//                MessageAdd("Confuse magic damage") ;
                /* Spin around little buddy. */
                Effect(
                    EFFECT_REORIENT,
                    EFFECT_TRIGGER_NONE,
                    0,
                    0,
                    0,
                    NULL) ;
                break ;
            case EFFECT_DAMAGE_SPECIAL_SLOW:
//                MessageAdd("Slow damage") ;
                PlayerMakeSoundLocal (SOUND_PLAYER_POISONED);
                G_playNextHurtSound=FALSE;
                /* Slow down for a little while. */
                Effect(
                    EFFECT_ADD_PLAYER_EFFECT,
                    EFFECT_TRIGGER_NONE,
                    PLAYER_EFFECT_SPEED_MOD,
                    2100,    /* Slowed for 30 seconds. */
                    -20,
                    StatsTakeDamage) ;
                break ;
            case EFFECT_DAMAGE_SPECIAL_PARALYZE:
//                MessageAdd("Paralyze damage") ;
                PlayerMakeSoundLocal (SOUND_PLAYER_POISONED);
                /* Slow down to a halt. */
                Effect(
                    EFFECT_ADD_PLAYER_EFFECT,
                    EFFECT_TRIGGER_NONE,
                    PLAYER_EFFECT_SPEED_MOD,
                    700,    /* Paralyzed for 10 seconds. */
                    -120,
                    StatsTakeDamage) ;
                break ;
            default:
                DebugCheck(FALSE) ;
                break ;
        }
    } else if (EffectPlayerEffectIsActive(PLAYER_EFFECT_INVULNERABLE)==FALSE) {
        /* first, cause the effect specified */
        if (type & EFFECT_DAMAGE_NORMAL)
        {
            effects++;
        }
        if (type & EFFECT_DAMAGE_FIRE)
        {
  //        printf ("fire\n");
            effects++;
            /* 25% bonus damage for fire damage */
            /* check for fire resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_FIRE)==TRUE)
            {
                resist++;
            }
            else
            {
                /* 25% bonus damage */
                amt += amt>>2;
                /* add some extra red */
//              ColorAddGlobal (amt>>3,-amt>>3,-amt>>3);
            }
        }

        if (type & EFFECT_DAMAGE_ACID)
        {
    //        printf ("acid\n");
            effects++;

            /* 1% chance per 100 points of damage to lose an item */
            /* check for acid resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_ACID)==TRUE)
            {
                resist++;
            }
            else
            {
                /* add some extra white */
  //            ColorAddGlobal (amt>>3,amt>>3,amt>>3);
                /* acid special effect */
                acidcount += amt;
                chance=acidcount/50;
                if (chance > 0)
                {
                    acidcount=0;
                    /* here's a random chance to destroy an item */
                    /* 1% per 100 points of damage taken */
                    if (rand()%100 < chance)
                    {
                        /* item gets destroyed! */
                        Effect(EFFECT_DESTROY_RANDOM_ITEM,
                               EFFECT_TRIGGER_NONE,
                               0,0,0,NULL);
                    }
                }
            }
        }

        if (type & EFFECT_DAMAGE_POISON)
        {
            effects++;

            /* take 1 point of poison damage per 10 hp taken */
            /* check for poison resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_POISON)==TRUE)
            {
                resist++;
                StatsChangePlayerPoisonLevel (amt/40);
            }
            else
            {
                /* add some extra green */
//              ColorAddGlobal (-amt>>3,amt>>3,-amt>>3);
                /* poison effect */
                StatsChangePlayerPoisonLevel (amt/10);
            }
        }

        if (type & EFFECT_DAMAGE_ELECTRICITY)
        {
    //        printf ("electricity\n");
            effects++;

            /* check for light or heavy armor in a random location. */
            /* if light, damage+=25%.  if Heavy, damage+=50%.

            /* check for electricity resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_ELECTRICITY)==TRUE)
            {
                resist++;
            }
            else
            {
                /* electricity effect */
                /* roll a random location */
                /* add some extra yellow */
//                ColorAddGlobal (amt>>3,amt>>3,-amt>>3);
                where=rand()%6+EQUIP_LOCATION_HEAD;
                /* check to see what type of armor we have there */
                armortype = InventoryGetArmorType (where);
                switch (armortype)
                {
                    case EQUIP_ARMOR_TYPE_BRACING_CHAIN:
                    case EQUIP_ARMOR_TYPE_LEGGINGS_CHAIN:
                    case EQUIP_ARMOR_TYPE_HELMET_CHAIN:
                    case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                    /* chain armor, +25% damage */
                    amt += amt>>2;
                    break;

                    case EQUIP_ARMOR_TYPE_BRACING_PLATE:
                    case EQUIP_ARMOR_TYPE_LEGGINGS_PLATE:
                    case EQUIP_ARMOR_TYPE_HELMET_PLATE:
                    case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                    /* plate armor, +50% damage*/
                    amt += amt>>1;
                    break;

                    default:
                    break;
                }

            }

        }

        if (type & EFFECT_DAMAGE_MANA_DRAIN)
        {
    //        printf ("mana drain\n");
            effects++;

            /* removes 1 point of mana per point of damage */
            /* check for mana drain resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_MANA_DRAIN)==TRUE)
            {
                resist++;
                StatsChangePlayerMana(-amt/4);
            }
            else
            {
                /* add some extra purple */
//                ColorAddGlobal (amt>>3,-amt>>3,amt>>3);
                StatsChangePlayerMana (-amt);
            }
        }

        if (type & EFFECT_DAMAGE_PIERCING)
        {
    //        printf ("piercing\n");
            effects++;

            /* ignore armor when calculating damage */
            /* check for piercing resistance */
            if (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_PIERCING)==TRUE)
            {
                resist++;
            }
            else
            {
                /* add some extra blue */
//              ColorAddGlobal (0,0,amt>>3);
            }
        }

        /* now, apply damage */
        /* first, remove a percent of damage based on number of resists */
        /* skip if effects==0 (i.e. normal damage) */
    //    printf ("resist=%d effects=%d\n",resist,effects);
    //    fflush (stdout);

        if (effects>0)
        {
            resistpct=(float)resist/(float)effects;
            /* cap resistances to 80% of damage */
            if (resistpct > ((float)0.8))
				resistpct=((float)0.8);
            amt=(T_word16)((float)amt*(1.0-resistpct));
        }
        /* calculate armor effects */
        /* use armor here */
        if ((type & EFFECT_DAMAGE_PIERCING)&&
            (EffectPlayerEffectIsActive (PLAYER_EFFECT_RESIST_PIERCING)==FALSE))
        {
            /* ignore armor */
        }
        else
        {
            real_damage_pct=100-(G_activeStats->ArmorLevel>>1);
            real_damage=amt*real_damage_pct;
            real_damage/=100;
            amt=real_damage;
        }

    //    printf ("final amt=%d\n",amt);
    //    fflush (stdout);
        /* take damage specified by amt */
        StatsChangePlayerHealth (-amt);

        /* reorient the player's view a little */
//#if COMPILE_OPTION_ALLOW_REORIENT_SMACK
        if ((amt >= 400) &&
            ((rand()%1500 < amt) || (amt > (G_activeStats->Health>>2))))
        {
            /* smack!! */
            StatsReorientPlayerView();
        }
//#endif
        /* color effects */
        if (effects==0)
        {
            if (type & EFFECT_DAMAGE_NORMAL)
            {
                ColorAddGlobal (amt>>3,-amt>>3,-amt>>3);
            }
        }
        else
        {
            amt/=effects;

            if (type & EFFECT_DAMAGE_NORMAL)
            {
                ColorAddGlobal (amt>>3,-amt>>3,-amt>>3);
            }
            if (type & EFFECT_DAMAGE_FIRE)
            {
                ColorAddGlobal (amt>>3,-amt>>3,-amt>>3);
            }
            if (type & EFFECT_DAMAGE_ACID)
            {
                ColorAddGlobal (amt>>3,amt>>3,amt>>3);
            }
            if (type & EFFECT_DAMAGE_POISON)
            {
                ColorAddGlobal (-amt>>3,amt>>3,-amt>>3);
            }
            if (type & EFFECT_DAMAGE_ELECTRICITY)
            {
                ColorAddGlobal (amt>>3,amt>>3,-amt>>3);
            }
            if (type & EFFECT_DAMAGE_MANA_DRAIN)
            {
                ColorAddGlobal (amt>>3,-amt>>3,amt>>3);
            }
            if (type & EFFECT_DAMAGE_PIERCING)
            {
                ColorAddGlobal (0,0,amt>>3);
            }
        }
    }

    DebugEnd();
}



T_void StatsChangePlayerExperience (T_sword32 amt)
{
    T_word32 NextLevel;
    T_word16 i,j;
    T_sbyte8 where;
    T_byte8 old_attributes[ATTRIBUTE_UNKNOWN];

    DebugRoutine ("StatsChangePlayerExperience");
    if (amt > 0)
    {
        G_activeStats->Experience += amt;

        /* check for new level gained */
        while (G_activeStats->Experience >= G_activeStats->ExpNeeded)
        {
            /* gained a level! */
            G_activeStats->Level++;

            NextLevel = G_activeStats->ExpNeeded;
            NextLevel += 2000;
            NextLevel += 10000*(G_activeStats->Level);
            NextLevel += 3000*(G_activeStats->Level * G_activeStats->Level);

            G_activeStats->ExpNeeded=NextLevel;
            if (EffectSoundIsOn())
            {
                SoundPlayByNumber (6014,255);
                MessageAdd ("^036You feel more experienced!!!");
                ColorAddGlobal (40,40,40);
            }
            /* save old attributes */
            for (i=0;i<6;i++) old_attributes[i]=G_activeStats->Attributes[i];

            /* calculate bonuses for increasing level */

            /* add 6 points, 1 per attribute */
            for (i=0;i<6;i++)
            {
                StatsSetPlayerAttribute (i,G_activeStats->Attributes[i]+2);
            }

            for (i=0;i<6;i++)
            {
                /* add 6 points, distributed weighted randomly among */
                /* player attributes */
                where=rand()%13;
                for (j=0;j<6;j++)
                {
                    /* subtract G_statsCharacterAdvancements value */
                    /* for this attribute from value */
                    where -= G_statsCharacterAdvancements
                          [G_activeStats->ClassType][j];

                    if (where <= 0)
                    {
                        /* found our slot, add one to this attrib */
                        StatsSetPlayerAttribute (j,G_activeStats->Attributes[j]+1);
                       break;
                    }
                }
            }

            /* clip maximum values */
            for (i=0;i<6;i++)
            {
                if (G_activeStats->Attributes[i]>100) G_activeStats->Attributes[i]=100;
            }

            /* now that we have the new stat calculations, do some other updates */

            /* update max health */
            if (G_activeStats->MaxHealth < 30000)
            {
                G_activeStats->MaxHealth += ((G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION]+10)*10);
            }

            /* update max mana */
            if (G_activeStats->MaxMana < 30000)
            {
                G_activeStats->MaxMana += ((G_activeStats->Attributes[ATTRIBUTE_MAGIC]+10)*10);
            }

            if (G_activeStats->MaxHealth > 30000) G_activeStats->MaxHealth = 30000;
            if (G_activeStats->MaxMana > 30000) G_activeStats->MaxMana = 30000;

            /* update regeneration rates */
            G_activeStats->RegenHealth = G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION]*
                                         (G_activeStats->Level+5);

            G_activeStats->RegenMana = G_activeStats->Attributes[ATTRIBUTE_MAGIC]*
                                       (G_activeStats->Level+5);
            /* set some class specific bonuses */
            switch (G_activeStats->ClassType)
            {
                 case CLASS_WARLOCK:
                 case CLASS_MAGICIAN:
					 G_activeStats->RegenMana = (G_activeStats->RegenMana*3)/2;
					 break;

                 case CLASS_PRIEST:
                 case CLASS_MAGE:
					 G_activeStats->RegenMana *= 3;
					 break;

                 default:
					 break;
            }

            /* update max load */
            StatsCalcPlayerMaxLoad();

            /* update weapon damage */
            StatsCalcPlayerAttackDamage();

            /* update attack speed */
            StatsCalcPlayerAttackSpeed();

            /* update movement speed */
            StatsCalcPlayerMovementSpeed();

            /* update title */
            if (G_activeStats->Level < 21)
            {
                strcpy (G_activeStats->ClassTitle,G_statsLevelTitles[G_activeStats->ClassType][G_activeStats->Level-1]);
            }
        }
    }
    else
    {
        /* clip experience to 0 if necessary */
        amt = -amt;
        if ((T_word32)amt > G_activeStats->Experience) amt = G_activeStats->Experience;
//        sprintf (stmp,"^014You have lost %d experience.",amt);
        G_activeStats->Experience -= amt;
    }

//    MessageAdd (stmp);

    /* check to see if we need to update the stats banner */
    if (BannerFormIsOpen (BANNER_FORM_STATISTICS))
    {
        StatsDisplayStatisticsPage();
    }

    /* update bottom banner */
    BannerStatusBarUpdate();

    DebugEnd();
}


T_void StatsChangePlayerLoad (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerLoad");

    DebugCheck (amt < 5000 && amt > -5000);
    if (amt > 0)
    {
        G_activeStats->Load += amt;
    }
    else
    {
        amt = -amt;
        DebugCheck (G_activeStats->Load >= amt);
        if (G_activeStats->Load < amt) G_activeStats->Load=0;
        else G_activeStats->Load -= amt;
    }

    /* recalculate movement speed */
    StatsCalcPlayerMovementSpeed();
    /* update bottom status bar */
    if (BannerIsOpen())
    {
        BannerStatusBarUpdate();
    }

    /* if inventory window is open update load*/
    if (BannerFormIsOpen (BANNER_FORM_INVENTORY))
    {
        InventoryDrawInventoryWindow(INVENTORY_PLAYER);
    }

    DebugEnd();
}


T_void StatsChangePlayerFood (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerFood");

    DebugCheck (amt < 5000 && amt > -5000);
    if (amt > 0)
    {
        G_activeStats->Food += amt;
        if (G_activeStats->Food > G_activeStats->MaxFood)
        {
            MessageAdd ("^009You are full");
            G_activeStats->Food = G_activeStats->MaxFood;
        }
        else
        {
            MessageAdd ("^009You feel nourished");
        }
    }
    else
    {
        amt = -amt;
        if (G_activeStats->Food < amt) G_activeStats->Food=0;
        else G_activeStats->Food -= amt;
        if (amt > 50) MessageAdd ("^014Your stomach grumbles.");
    }

    /* update bottom status bar */
    BannerStatusBarUpdate();

    DebugEnd();
}


T_void StatsChangePlayerWater(T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerWater");

    DebugCheck (amt < 5000 && amt > -5000);
    if (amt > 0)
    {
        G_activeStats->Water += amt;
        SoundPlayByNumber(6017,255);
        if (G_activeStats->Water > G_activeStats->MaxWater)
        {
            MessageAdd ("^009Your thirst is quenched.");
            G_activeStats->Water = G_activeStats->MaxWater;
        }
    }
    else
    {
        amt = -amt;
        if (G_activeStats->Water < amt) G_activeStats->Water=0;
        else G_activeStats->Water -= amt;
        if (amt > 50) MessageAdd ("^014Your feel your throat dry up.");
    }

    /* update bottom status bar */
    BannerStatusBarUpdate();

    DebugEnd();
}


T_void StatsChangePlayerHealthRegen (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerHealthRegen");

    G_activeStats->RegenHealth += amt;

    DebugEnd();
}



T_void StatsChangePlayerManaRegen (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerManaRegen");

    G_activeStats->RegenMana += amt;

    DebugEnd();
}


/* LES: 05/29/96 Changed input paramater from T_sbyte8 to T_sword16 */
/*               so that the HALVE POISON spell works correctly with */
/*               high levels of poison (255+). */
T_void StatsChangePlayerPoisonLevel (T_sword16 amt)
{
    DebugRoutine ("StatsChangePlayerPoisonLevel");

    /* LES:  Add a message to tell the state of the poison */
    /* especially since no poison indicator exists */
    /* Are we getting or removing poison? */
    if (amt > 0)  {
        /* Getting poisoned */
        /* Only add a message if we were not poisoned before. */
        if (G_activeStats->PoisonLevel == 0)  {
            MessageAdd ("^034You've been poisoned!!!");
        }
    }

    G_activeStats->PoisonLevel += amt;

    /* clip poison level to zero */
    if (G_activeStats->PoisonLevel < 0)
      G_activeStats->PoisonLevel = 0;

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  StatsUpdatePlayerStatistics                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine is responsible for changes to health, mana, food, and    */
/*    water due to time passing or spell effects such as poison             */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  01/11/96  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void StatsUpdatePlayerStatistics (T_void)
{
    T_word32 delta,time;
    T_word16 pReduction;
    float fratio,wratio,hregen,mregen;
    T_word16 plev;
    static T_word32 lastupdate=0;
    DebugRoutine ("StatsUpdatePlayerStatistics");

    /* calculate time passed delta */
    time=TickerGet();
    if (lastupdate==0)
    {
        delta=0;
        lastupdate=TickerGet();
    }
    else
    {
         delta=time-lastupdate;
    }

    if (ClientIsPaused()==FALSE &&
        ClientIsInView()==TRUE &&
        ClientIsDead()==FALSE )  {
        /* this routine triggers approx every 6 seconds */
        if (delta >= 420)
        {
            lastupdate=TickerGet();

            /* calculate water ratio */
            /* calculate food ratio */
            fratio = (float) G_activeStats->Food / (float) G_activeStats->MaxFood;
            wratio = (float) G_activeStats->Water / (float) G_activeStats->MaxWater;

            /* calculate mana regen rate */
            /* calculate health regen rate */
            mregen = (float)(G_activeStats->RegenMana>>2);
            mregen *= wratio;

            /* regen health 1/2 as fast as mana if rates are same */
            hregen = (float)(G_activeStats->RegenHealth>>3);
            hregen *= fratio;

//          MessagePrintf ("hregen=%f mregen=%f",hregen,mregen);

            G_activeStats->Health += (T_sword32)hregen;
            if (G_activeStats->Health > G_activeStats->MaxHealth)
              G_activeStats->Health = G_activeStats->MaxHealth;

            G_activeStats->Mana += (T_sword32)mregen;
            if (G_activeStats->Mana > G_activeStats->MaxMana)
              G_activeStats->Mana = G_activeStats->MaxMana;

            if (EffectPlayerEffectIsActive(PLAYER_EFFECT_ETERNAL_NOURISHMENT)==FALSE)
            {
                if (EffectPlayerEffectIsActive (PLAYER_EFFECT_FOOD_CONSERVATION)==FALSE)
                {
                    /* modify food level by -1 every 6 seconds */
                    StatsChangePlayerFood (-1);
                }

                if (EffectPlayerEffectIsActive (PLAYER_EFFECT_WATER_CONSERVATION)==FALSE)
                {
                    /* modify water level by -1 every 6 seconds */
                    StatsChangePlayerWater (-1);
                }
            }
    //        sprintf (stmp,"regen: M=%d H=%d\n",(T_sword16)mregen,(T_sword16)hregen);
    //        MessageAdd (stmp);

            /* check for poison */
            if (G_activeStats->PoisonLevel > 0 )
            {
                /* uh oh, poison time */
                if (EffectPlayerEffectIsActive(PLAYER_EFFECT_RESIST_POISON)==FALSE)
                {
                    MessageAdd ("^034Poison!!!");
                    PlayerMakeSoundLocal (SOUND_PLAYER_POISONED);
                    G_playNextHurtSound=FALSE;
                    plev = G_activeStats->PoisonLevel;
                    if (plev <= 255) ColorAddGlobal (-plev>>1,plev>>1,-plev>>1);
                    else ColorAddGlobal (-0,63,0);

                    StatsChangePlayerHealth (-plev*2);
                }
                /* reduce poison level */
                pReduction = StatsGetPlayerAttribute(ATTRIBUTE_CONSTITUTION)>>2;
                if (pReduction < 5) pReduction=5;
                StatsChangePlayerPoisonLevel (-pReduction);
            }

            /* modify last update by remainder */
            delta-=420;
            lastupdate-=delta;
        }
    }
    else
    {
        lastupdate=0;
    }

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  StatsSetActive                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StatsSetActive declares the active player stats.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_playerStats *p_stats      -- What player stats to use               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsSetActive(T_playerStats *p_stats)
{
    DebugRoutine("StatsSetActive") ;
    DebugCheck(p_stats != NULL) ;

    G_activeStats = p_stats ;

    DebugEnd() ;
}


T_void StatsSetName (T_byte8 *newname)
{
    DebugRoutine ("StatsSetName");
    DebugCheck (newname != NULL);
    DebugCheck (G_activeStats != NULL);
    DebugCheck (strlen (newname) < 30);
    strcpy (G_activeStats->Name, newname);

    DebugEnd();
}



/****************************************************************************/
/*  Routine:  StatsCreateCharacterUI                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    UI screen to create a new character                                   */
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
/*    E_Boolean TRUE if character successfully created                      */
/*              FALSE otherwise                                             */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  08/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/


E_Boolean StatsCreateCharacterUI (T_void)
{
	T_word32 delta=0,lastupdate=0;
    DebugRoutine ("StatsCreateCharacterUI");

    StatsCreateCharacterUIInit();

    FormGenericControl (&G_exit);

    DebugEnd();
    return (G_success);
}



/****************************************************************************/
/*  Routine:  StatsUpdateCreateCharacterUI                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Redraws the character creation UI screen                              */
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
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  08/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsUpdateCreateCharacterUI (T_void)
{
    T_byte8 stmp[64];
    T_TxtboxID TxtboxID;
    T_byte8 *description;
    T_resource res;

    DebugRoutine ("StatsUpdateCreateCharacterUI");

    StatsCalcClassStats();

    /* set up description field */
    TxtboxID=FormGetObjID (500);

    sprintf (stmp,"UI/CREATEC/DESC%2.2d.TXT",StatsGetPlayerClassType());
    description=PictureLockData (stmp,&res);
    TxtboxSetNData (TxtboxID,description,ResourceGetSize(res));
    PictureUnlockAndUnfind (res);

    /* set up attribute fields */
    TxtboxID=FormGetObjID (503);
    sprintf (stmp,"%d",StatsGetPlayerStrength());
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (504);
    sprintf (stmp,"%d",StatsGetPlayerConstitution());
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (505);
    sprintf (stmp,"%d",StatsGetPlayerAccuracy());
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (506);
    sprintf (stmp,"%d",StatsGetPlayerSpeed());
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (507);
    sprintf (stmp,"%d",StatsGetPlayerMagic());
    TxtboxSetData (TxtboxID,stmp);

    TxtboxID=FormGetObjID (508);
    sprintf (stmp,"%d",StatsGetPlayerStealth());
    TxtboxSetData (TxtboxID,stmp);

    /* set up title field */
    TxtboxID=FormGetObjID (509);
    sprintf (stmp,"%s",StatsGetPlayerClassName());
    TxtboxSetData (TxtboxID,stmp);


    /* load picture for this class */
    StatsDrawCharacterPortrait (10,9);

//    GraphicUpdateAllGraphics();

    DebugEnd();
}



/****************************************************************************/
/*  Routine:  StatsGetCharacterList                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Downloads a list of characters from the server                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Doesn't do anything yet.                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    *data [list of characters]                                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  08/11/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_byte8 *StatsGetCharacterList (T_void)
{
    static char data[256];
    sprintf (data,"1. Nirvana\r2. <@> Stone Temple Pilot\r3. <*> White Zombie\r4. Pantera\r5. <empty>");

    return (data);
}



T_void StatsCreateCharacterUIInit(T_void)
{
    DebugRoutine ("StatsCreateCharacterUIInit");

	/* load the form for this page */
	FormLoadFromFile ("CREATEC.FRM");

    /* set the success and exit flags */
    G_statsLCExit=FALSE;
    G_statsLCSuccess=FALSE;

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (StatsCreateCharacterControl);
	/* make sure that the color cycling will work */
    /* double buffer drawing */
    /* set up description fields */
//    StatsUpdateCreateCharacterUI();

    GraphicUpdateAllGraphics();

    StatsUpdateCreateCharacterUI();

    GraphicUpdateAllGraphics();

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  StatsDisplayStatisticsPage                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Fills out the fields for the banner show statistics option            */
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
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  11/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsDisplayStatisticsPage (T_void)
{
   T_byte8 stmp[64];
   T_TxtboxID TxtboxID;
   T_sword16 mod;

   DebugRoutine ("StatsDisplayStaticsticsPage");

   /* note, routine expects statistics form to be active */
   /* set name */
   TxtboxID=FormGetObjID(500);
   sprintf (stmp,"%s",StatsGetName());
   TxtboxSetData(TxtboxID,stmp);

   /* set level */
   TxtboxID=FormGetObjID(501);
   sprintf (stmp,"%d",StatsGetPlayerLevel());
   TxtboxSetData(TxtboxID,stmp);

   /* set strength */
   TxtboxID=FormGetObjID(502);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_STRENGTH);
   /* set field color to green for positive modification */
   if (mod > 0) strcpy (stmp,"^009");
   /* set field color to red for negative one */
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_STRENGTH));
   TxtboxSetData(TxtboxID,stmp);

   /* set constitution */
   TxtboxID=FormGetObjID(503);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_CONSTITUTION);
   if (mod > 0) strcpy (stmp,"^009");
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_CONSTITUTION));
   TxtboxSetData(TxtboxID,stmp);

   /* set accuracy */
   TxtboxID=FormGetObjID(504);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_ACCURACY);
   if (mod > 0) strcpy (stmp,"^009");
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_ACCURACY));
   TxtboxSetData(TxtboxID,stmp);

   /* set stealth*/
   TxtboxID=FormGetObjID(505);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_STEALTH);
   if (mod > 0) strcpy (stmp,"^009");
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_STEALTH));
   TxtboxSetData(TxtboxID,stmp);

   /* set magic */
   TxtboxID=FormGetObjID(506);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_MAGIC);
   if (mod > 0) strcpy (stmp,"^009");
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_MAGIC));
   TxtboxSetData(TxtboxID,stmp);

   /* set speed */
   TxtboxID=FormGetObjID(507);

   mod=StatsGetPlayerAttributeMod (ATTRIBUTE_SPEED);
   if (mod > 0) strcpy (stmp,"^009");
   else if (mod < 0) strcpy (stmp,"^014");
   else strcpy (stmp,"^007");

   sprintf (stmp,"%s%d",stmp,StatsGetPlayerAttribute(ATTRIBUTE_SPEED));
   TxtboxSetData(TxtboxID,stmp);

   /* set class name */
   TxtboxID=FormGetObjID(508);
   sprintf (stmp,"%s",StatsGetPlayerClassName());
   TxtboxSetData(TxtboxID,stmp);

   /* set class title */
   TxtboxID=FormGetObjID(509);
   sprintf (stmp,"%s",StatsGetPlayerClassTitle());
   TxtboxSetData(TxtboxID,stmp);

   /* set exp gained */
   TxtboxID=FormGetObjID(510);
   sprintf (stmp,"%d",StatsGetPlayerExperience());
   TxtboxSetData(TxtboxID,stmp);

   /* set exp needed */
   TxtboxID=FormGetObjID(511);
   sprintf (stmp,"%d",StatsGetPlayerExpNeeded());
   TxtboxSetData(TxtboxID,stmp);
/* moved to banner.c */
#if 0
   /* display the character graphic */
   sprintf (stmp,"UI/3DUI/SMPORT%02d",StatsGetPlayerClassType());

   if (PictureExist (stmp))
   {
       res=PictureFind(stmp);
       p_data=PictureLockQuick (res);
       DebugCheck (p_data != NULL);
       ColorUpdate(0) ;
       GrDrawBitmap (PictureToBitmap(p_data),215,27);
       PictureUnlockAndUnfind (res);
   }
#endif
   DebugEnd();
}



E_Boolean StatsAddCoin (E_equipCoinTypes type, T_sword16 num)
{
    E_Boolean retvalue=TRUE;
    T_byte8 stmp[64];
    T_word16 anum;

    DebugRoutine ("StatsAddCoin");
    DebugCheck (type < COIN_TYPE_UNKNOWN);

    /* get an abs value for num */
    anum=(T_word16)labs(num);

    //MessagePrintf ("type = %d, num=%d\n",type,num);

    switch (type)
    {
        case COIN_TYPE_COPPER:
        if (anum == 1) strcpy (stmp,"^009You got a copper coin.");
        else if (anum==5) strcpy (stmp,"^009You got a copper five-piece.");
        else sprintf (stmp,"^009 You got %d copper pieces.",anum);
        break;
        case COIN_TYPE_SILVER:
        if (anum == 1) strcpy (stmp,"^009You got a silver coin.");
        else if (anum==5) strcpy (stmp,"^009You got a silver five-piece.");
        else sprintf (stmp,"^009 You got %d silver pieces.",anum);
        break;
        case COIN_TYPE_GOLD:
        if (anum == 1) strcpy (stmp,"^009You got a gold coin.");
        else if (anum==5) strcpy (stmp,"^009You got a gold five-piece.");
        else sprintf (stmp,"^009 You got %d gold pieces.",anum);
        break;
        case COIN_TYPE_PLATINUM:
        if (anum == 1) strcpy (stmp,"^009You got a platinum coin.");
        else if (anum==5) strcpy (stmp,"^009You got a platinum five-piece.");
        else sprintf (stmp,"^009 You got %d platinum pieces.",anum);
        break;
        default:
        break;
    }

    if (num > 0)
    {
        G_activeStats->Coins[type]+=num;
        MessageAdd (stmp);
    } else
    {
        /* try to subtract coins here */
        if (G_activeStats->Coins[type]>=(-num)) G_activeStats->Coins[type]+=num;
        else retvalue=FALSE;
    }

    /* update banner display */
    if (BannerFormIsOpen (BANNER_FORM_FINANCES))
    {
        BannerDisplayFinancesPage();
    }

    DebugEnd();
    return (retvalue);
}


E_Boolean StatsAddBolt (E_equipBoltTypes type, T_sword16 num)
{
    E_Boolean retvalue=TRUE;
    T_byte8 stmp[64];
    T_word16 anum;

    DebugRoutine ("StatsAddBolt");
    DebugCheck (type < BOLT_TYPE_UNKNOWN);

    /* get an abs value for num */
    anum=(T_word16)labs(num);
    if (num>0)
    {
        switch (type)
        {
            case BOLT_TYPE_NORMAL:
            if (num==1) strcpy (stmp,"^009You got a bolt.");
            else sprintf (stmp,"^009You got a quiver of %d bolts.",num);
            break;
            case BOLT_TYPE_POISON:
            if (num==1) strcpy (stmp,"^009You got a green bolt.");
            else sprintf (stmp,"^009You got a quiver of %d green bolts.",num);
            break;
            case BOLT_TYPE_PIERCING:
            if (num==1) strcpy (stmp,"^009You got a white bolt.");
            else sprintf (stmp,"^009You got a quiver of %d white bolts.",num);
            break;
            case BOLT_TYPE_FIRE:
            if (num==1) strcpy (stmp,"^009You got a red bolt.");
            else sprintf (stmp,"^009You got a quiver of %d red bolts.",num);
            break;
            case BOLT_TYPE_ELECTRICITY:
            if (num==1) strcpy (stmp,"^009You got a yellow bolt.");
            else sprintf (stmp,"^009You got a quiver of %d yellow bolts.",num);
            break;
            case BOLT_TYPE_MANA_DRAIN:
            if (num==1) strcpy (stmp,"^009You got a violet bolt.");
            else sprintf (stmp,"^009You got a quiver of %d violet bolts.",num);
            break;
            case BOLT_TYPE_ACID:
            if (num==1) strcpy (stmp,"^009You got a brown bolt.");
            else sprintf (stmp,"^009You got a quiver of %d brown bolts.",num);
            break;
            default:
            break;
        }
        G_activeStats->Bolts[type]+=num;
        if (anum>0) MessageAdd (stmp);
    }
    else
    {
        /* try to subtract ammo here */
        if (G_activeStats->Bolts[type]>=(-num)) G_activeStats->Bolts[type]+=num;
        else
        {
            retvalue=FALSE;
//            MessageAdd ("Failed");
        }
    }

    /* update banner display */
    if (BannerFormIsOpen (BANNER_FORM_AMMO))
    {
        BannerDisplayAmmoPage();
    }

    DebugEnd();
    return (retvalue);
}



T_void StatsSetArmorValue (E_equipLocations location, T_byte8 value)
{
    DebugRoutine ("StatsSetArmorValue");
//    printf ("location=%d, value=%d\n",location,value);
//    fflush (stdout);
    DebugCheck (location < EQUIP_LOCATION_UNKNOWN);
//    DebugCheck (location >= EQUIP_LOCATION_HEAD);
//    DebugCheck (location <= EQUIP_LOCATION_SHIELD_HAND);

    if (location <= EQUIP_LOCATION_SHIELD_HAND &&
        location >= EQUIP_LOCATION_HEAD)
    {
        G_activeStats->ArmorValues[location]=value;
    }
    /* recalculate average armor level */

    StatsCalcAverageArmorValue();

    DebugEnd();
}


T_void StatsCalcAverageArmorValue (T_void)
{
    T_word16 Alev=0;
    T_word16 shield=0;

    Alev=0;
    if (EffectPlayerEffectIsActive(PLAYER_EFFECT_SHIELD)==TRUE)
    {
        shield=EffectGetPlayerEffectPower (PLAYER_EFFECT_SHIELD);
        if (shield > 90) shield=90;
    }

    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_HEAD]*2 > shield*2 ?
           G_activeStats->ArmorValues[EQUIP_LOCATION_HEAD]*2 : shield*2);

    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_RIGHT_ARM]*2 > shield*2 ?
           G_activeStats->ArmorValues[EQUIP_LOCATION_RIGHT_ARM]*2 : shield*2);

    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_LEFT_ARM]*2 > shield*2 ?
           G_activeStats->ArmorValues[EQUIP_LOCATION_LEFT_ARM]*2 : shield*2);

    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_LEGS]*4 > shield*4 ?
           G_activeStats->ArmorValues[EQUIP_LOCATION_LEGS]*4 : shield*4);

    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_CHEST]*4 > shield*4 ?
           G_activeStats->ArmorValues[EQUIP_LOCATION_CHEST]*4 : shield*4);

//    Alev+=(G_activeStats->ArmorValues[EQUIP_LOCATION_SHIELD_HAND]*2 > shield*2 ?
//           G_activeStats->ArmorValues[EQUIP_LOCATION_SHIELD_HAND]*2 : shield*2);

//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_RIGHT_ARM]*2;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_LEFT_ARM]*2;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_LEGS]*4;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_RIGHT_LEG]*3;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_LEFT_LEG]*3;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_CHEST]*4;
//    Alev+=G_activeStats->ArmorValues[EQUIP_LOCATION_SHIELD_HAND]*2;
    Alev/=14;

    G_activeStats->ArmorLevel = (T_byte8)Alev;
    /* redraw equip screen if necessary */
    if (BannerFormIsOpen (BANNER_FORM_EQUIPMENT)) InventoryDrawEquipmentWindow();
}


T_byte8 StatsGetArmorValue (T_byte8 location)
{
    T_byte8 retvalue=0;

    DebugRoutine ("StatsGetArmorValue");

    retvalue=G_activeStats->ArmorValues[location];

    DebugEnd();

    return (retvalue);
}


/****************************************************************************/
/*  Routine:  StatsCalcWeaponAttackSpeed / AttackDamage                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Performs calculations for attack speed and damage.                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_sbyte8 weaponmod - weapon modifier                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None (sets the statistics AttackSpeed and AttackDamage)               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  12/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/
/* routine sets base weapon damage */
T_void StatsSetWeaponBaseDamage (T_byte8 value)
{
    DebugRoutine ("StatsSetWeaponBaseDamage");

    if (value < 2) value=2;

    G_activeStats->WeaponBaseDamage = value;

    StatsCalcPlayerAttackDamage ();
    DebugEnd();
}

/* routine sets weapon base speed */
T_void StatsSetWeaponBaseSpeed (T_sbyte8 value)
{
    DebugRoutine ("StatsSetWeaponBaseSpeed");

    G_activeStats->WeaponBaseSpeed = value;

    StatsCalcPlayerAttackSpeed ();
    DebugEnd();
}


T_void StatsCalcPlayerAttackSpeed (T_void)
{
    const T_word32 slow=40000L;
    const T_word32 fast=150000L;
    const T_word32 step = (fast-slow)/100L;
    T_sbyte8 weaponmod;
    T_sword16 wspeed;
    DebugRoutine ("StatsCalcPlayerAttackSpeed");


    weaponmod = G_activeStats->WeaponBaseSpeed;
    //10=slow, 100=fast.

    wspeed=StatsGetPlayerAttribute(ATTRIBUTE_SPEED)+weaponmod;
    if (wspeed < 0) wspeed=0;
    if (wspeed > 100) wspeed=100;

    G_activeStats->AttackSpeed=slow+((T_word32)wspeed*step);
    if (G_activeStats->AttackSpeed > fast) G_activeStats->AttackSpeed=fast;

    DebugEnd();
}


T_void StatsCalcPlayerAttackDamage (T_void)
{
    T_word16 damage=0;
    T_byte8 weaponmod;
    DebugRoutine ("StatsCalcPlayerAttackDamage");

    weaponmod = G_activeStats->WeaponBaseDamage;

    damage=(StatsGetPlayerAttribute(ATTRIBUTE_STRENGTH)*weaponmod)/2;
    G_activeStats->AttackDamage = damage;

    /* check for class bonuses */
    switch (G_activeStats->ClassType)
    {
        case CLASS_KNIGHT:
        /* 50% melee weapon bonus */
        G_activeStats->AttackDamage+=(G_activeStats->AttackDamage/2);
        break;

        case CLASS_WARLOCK:
        /* 25% melee weapon bonus */
//        G_activeStats->AttackDamage+=(G_activeStats->AttackDamage/4);
        break;

        case CLASS_SAILOR:
        /* 25% melee weapon bonus */
        G_activeStats->AttackDamage+=(G_activeStats->AttackDamage/4);
        break;

        case CLASS_PALADIN:
        /* 25% melee weapon bonus */
//        G_activeStats->AttackDamage+=(G_activeStats->AttackDamage/4);
        break;

        case CLASS_MERCENARY:
        /* 25% melee weapon bonus */
        G_activeStats->AttackDamage+=(G_activeStats->AttackDamage/4);
        break;

        case CLASS_UNKNOWN:
        default:
        /* failed! */
        DebugCheck (-1);
        break;
    }
    DebugEnd();
}


T_word16 StatsGetPlayerAttackDamage (T_void)
{
    T_word16 retvalue;
    DebugRoutine ("StatsGetPlayerAttackDamage");

    /* figure base damage + random modifer */
    retvalue=G_activeStats->AttackDamage + (rand()%G_activeStats->AttackDamage);

    /* check for critical hit */
    if (rand()%200 < StatsGetPlayerAttribute (ATTRIBUTE_ACCURACY))
    {
        /* critical hit, double damage */
        G_hitWasCritical=TRUE;
        retvalue *=2;
    }
    else G_hitWasCritical=FALSE;

    DebugEnd();
    return (retvalue);
}


/* This routine draws a character portrait at the specified location */

T_void StatsDrawCharacterPortrait (T_word16 x1, T_word16 y1)
{
    T_resource res;
    T_byte8 stmp[64];
    T_byte8 *p_data;

    DebugRoutine ("StatsDrawCharacterPortrait");

    ViewSetPalette(VIEW_PALETTE_STANDARD);
    sprintf (stmp,"UI/CREATEC/CHAR%02d",StatsGetPlayerClassType());
//    printf ("Searching for %s\n",stmp);
    fflush (stdout);

    GrScreenSet (GRAPHICS_ACTUAL_SCREEN);
    GrDrawRectangle (x1,y1,x1+115,y1+102,0);
    if (PictureExist (stmp))
    {
        res=PictureFind(stmp);
        p_data=PictureLockQuick (res);
        DebugCheck (p_data != NULL);
        ColorUpdate(0) ;
        GrDrawBitmap (PictureToBitmap(p_data),x1,y1);
        PictureUnlockAndUnfind (res);
    } else
    {
        /* clear area */
        GrDrawRectangle (x1,y1,x1+115,y1+102,55);
    }

    /* load palette for this class picture */
/*    sprintf (stmp,"UI/CREATEC/PAL%02d",StatsGetPlayerClassType());
    if (PictureExist (stmp))
    {
        p_data=PictureLockData (stmp,&res);
        DebugCheck (p_data != NULL);
        GrSetPalette (0,256,(T_palette *)p_data);
        ColorInit();
        PictureUnlockAndUnfind (res);
    } else {
DebugCheck(FALSE) ;
    }
*/

    DebugEnd();
}


/* routine sets an attribute modifier (ex: a strength bonus or penalty) */
T_void StatsChangePlayerAttributeMod (E_statsAttributeType attribute, T_sbyte8 value)
{
    DebugRoutine ("StatSetPlayerAttributeMod");
    DebugCheck (attribute < ATTRIBUTE_UNKNOWN);

    G_activeStats->AttributeMods[attribute]+=value;

    /* display a message */
    /* and recalc any effects */
    switch (attribute)
    {
        case ATTRIBUTE_STRENGTH:

        /* recalculate attack damage */
        StatsCalcPlayerAttackDamage();
        /* recalculate max load */
        StatsCalcPlayerMaxLoad();

        /* redraw inventory screen to display new max load */
        if (BannerFormIsOpen (BANNER_FORM_INVENTORY)) InventoryDrawInventoryWindow(INVENTORY_PLAYER);
        break;

        case ATTRIBUTE_SPEED:
        /* recalculate attack speed */
        StatsCalcPlayerAttackSpeed();
        /* recalculate movement speed*/
        StatsCalcPlayerMovementSpeed();
        break;

        default:
        break;
    }

    /* redraw the statistics page if necessary */
    if (BannerFormIsOpen (BANNER_FORM_STATISTICS))
      StatsDisplayStatisticsPage();

    DebugEnd();
}

/* routine returns the current modifier for an attribute */
T_sword16 StatsGetPlayerAttributeMod (E_statsAttributeType attribute)
{
    T_sword16 retvalue;
    DebugRoutine ("StatsGetPlayerAttributeMod");
    DebugCheck (attribute < ATTRIBUTE_UNKNOWN);

    retvalue=G_activeStats->AttributeMods[attribute];

    DebugEnd();
    return (retvalue);
}


/* clears a player attribute modifier */
T_void StatsClearPlayerAttributeMod (E_statsAttributeType attribute)
{
    DebugRoutine ("StatsClearPlayerAttributeMod");
    DebugCheck (attribute < ATTRIBUTE_UNKNOWN);

    G_activeStats->AttributeMods[attribute]=0;

    DebugEnd();
}


/* routine returns the total value (with attribute mod) for attribute specified */
T_word16  StatsGetPlayerAttribute (E_statsAttributeType attribute)
{
    T_word16 retvalue;
    T_sword16 temp;
    DebugRoutine ("StatsGetPlayerAttribute");
    DebugCheck (attribute < ATTRIBUTE_UNKNOWN);

    temp=G_activeStats->Attributes[attribute]+G_activeStats->AttributeMods[attribute];

    /* clip calculation */
    if (temp < 10) temp = 10;
    if (temp > 120) temp= 120;

    retvalue = (T_byte8)temp;

    DebugEnd();
    return (retvalue);
}

T_word16  StatsGetPlayerAttributeNoMod (E_statsAttributeType attribute)
{
    T_word16 retvalue;
    DebugRoutine ("StatsGetPlayerAttributeNoMod");

    DebugCheck (attribute < ATTRIBUTE_UNKNOWN);
    retvalue=G_activeStats->Attributes[attribute];

    DebugEnd();
    return (retvalue);
}


static T_void StatsCalcPlayerMaxLoad (T_void)
{
    DebugRoutine ("StatsCalcPlayerMaxLoad");

    G_activeStats->MaxLoad = StatsGetPlayerAttribute(ATTRIBUTE_STRENGTH)*20;

    DebugEnd();
}


T_void StatsCalcPlayerMovementSpeed (T_void)
{
    T_word16 load,max;
    float ratio;
//    T_byte8 stmp[48];

    DebugRoutine ("StatsCalcPlayerMovementSpeed");

    G_activeStats->MaxVWalking = (StatsGetPlayerAttribute(ATTRIBUTE_SPEED)/2)+10;

    /* apply class bonuses */
    if (G_activeStats->ClassType == CLASS_ROGUE)
    {
        /* 25% movement speed bonus */
        G_activeStats->MaxVWalking+=(G_activeStats->MaxVWalking>>2);
    }
    else if (G_activeStats->ClassType == CLASS_SAILOR)
    {
        /* 13% movement speed bonus */
        G_activeStats->MaxVWalking+=(G_activeStats->MaxVWalking>>3);
    }

    /* apply encumberance */
    load=G_activeStats->Load;
    max=G_activeStats->MaxLoad;
//    sprintf (stmp,"load=%d, max=%d\n",load,max);
//    MessageAdd (stmp);
    if (load > max)
    {
        /* can't move ! */
        G_activeStats->MaxVWalking = 0;
    }
    else if (load > (max/2))
    {
        load = load-(max/2);
        load*=2;

        /* check encumberance */
        ratio = (float)(load)/(float)(max);

        if (ratio < 0.75) MessageAdd ("^003You are slowed by the weight of your pack");
        else MessageAdd ("^003You are carrying too much.");

//        sprintf (stmp,"rat=%f, Walking was %d\n",ratio,G_activeStats->MaxVWalking);
//        MessageAdd (stmp);
        G_activeStats->MaxVWalking =(T_word16)( (float)G_activeStats->MaxVWalking * (1.0-ratio)+1);
//        sprintf (stmp,"Walking now %d\n",G_activeStats->MaxVWalking);
//        MessageAdd (stmp);
    }

    G_activeStats->MaxVRunning = G_activeStats->MaxVWalking + (G_activeStats->MaxVWalking/2);
    G_activeStats->MaxVWalking /= 2; // new rule, make walking speed even slower

    DebugEnd();
}


/* toggles 'god mode' status */
T_void StatsToggleGodMode (T_void)
{
    T_word16 i;
    T_void *powner;

    DebugRoutine ("StatsToggleGodMode");

    /* use an address of this routine for spell create/destroy flag */
    powner = StatsToggleGodMode;

    if (EffectPlayerEffectIsActive (PLAYER_EFFECT_GOD_MODE)==TRUE)
    {
        /* remove god mode status */
        MessageAdd ("^018God mode off!");
        MessageAdd ("^014You feel normal again.");
        Effect (EFFECT_REMOVE_PLAYER_EFFECT,
                EFFECT_TRIGGER_NONE,
                0,
                0,
                0,
                powner);

        /* remove added spell runes */
        for (i=0;i<9;i++) StatsDecrementRuneCount ((T_byte8)i);
    }
    else
    {
        /* add god mode effect */
        StatsSetPlayerMana   (StatsGetPlayerMaxMana());
        StatsSetPlayerHealth (StatsGetPlayerMaxHealth());
        StatsSetPlayerPoisonLevel (0);
        ColorAddGlobal(30,30,30);
        SoundPlayByNumber(2014, 128) ;
        MessageAdd ("^009You feel superhuman!!");
        MessageAdd ("^018Welcome to God Mode");
        Effect (EFFECT_ADD_PLAYER_EFFECT,
                EFFECT_TRIGGER_NONE,
                PLAYER_EFFECT_GOD_MODE,
                0,
                0,
                powner);

        for (i=0;i<9;i++) StatsIncrementRuneCount ((T_byte8)i);

        GraphicUpdateAllGraphics();
    }

    DebugEnd();
}


/* function incremenets the number of 'active' runes in each slot */
/* valid args are 0-8.  This function is used to tally the number */
/* of effect calls to add a correct rune so that when all runes */
/* carried are dropped the spell button can be disabled */

T_void StatsIncrementRuneCount (T_byte8 which)
{
    DebugRoutine ("StatsIncrementRuneCount");
    DebugCheck (which <9);

    if (G_activeStats->ActiveRunes[which]<100)
    {
        G_activeStats->ActiveRunes[which]++;
    }

    /* if now 1, we must have added a new rune, update buttons accordingly */
    if (G_activeStats->ActiveRunes[which]==1)
      BannerAddSpellButton(which);

    DebugEnd();
}


T_void StatsDecrementRuneCount (T_byte8 which)
{
    DebugRoutine ("StatsDecrementRuneCount");
    DebugCheck (which <9);

    G_activeStats->ActiveRunes[which]--;
    /* if now 0, we must have removed a rune completely, */
    /* update buttons accordingly */

    if (G_activeStats->ActiveRunes[which]==0)
      BannerRemoveSpellButton(which);

    DebugCheck (G_activeStats->ActiveRunes[which]<MAX_RUNE_COUNT);

    DebugEnd();
}

/* returns true if G_activeStats->ActiveRunes[which]>0 */
/* i.e. if there is a rune in the keypad slot designated by which */

E_Boolean StatsRuneIsAvailable (T_byte8 which)
{
    E_Boolean retvalue=FALSE;
    DebugRoutine ("StatsRuneIsAvailable");

    if (G_activeStats->ActiveRunes[which]>0) retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}

/* returns player's jump power + player's jump power mod */
T_word32 StatsGetJumpPower()
{
    T_sword32 temp=0;
    T_word32 retvalue=0;
    DebugRoutine ("StatsGetJumpPower");

    temp=G_activeStats->JumpPower + G_activeStats->JumpPowerMod;

    if (temp < 0) temp=0;

    if (temp > MAX_JUMP_POWER) temp=MAX_JUMP_POWER;

    retvalue=temp;

    DebugEnd();
    return (retvalue);
}

/* modifies a player's jump power */
T_void StatsModJumpPower (T_sword16 mod)
{
    DebugRoutine ("StatsModJumpPower");

    G_activeStats->JumpPowerMod+=mod;

    DebugEnd();
}


/* routine will instruct the server to delete character number */
/* whichcharacter.  Currently only deletes file designated */
/* from client hard drive */
/* LES: Low level delete character:  Does not care about what */
/* the server is thinking. */
E_Boolean StatsDeleteCharacter (T_byte8 selected)
{
    E_Boolean success=FALSE;
    T_byte8 stmp[64];

    DebugRoutine ("StatsDeleteCharacter");

    /* make sure there is a character here to delete */
    if (G_savedCharacters[selected].status<CHARACTER_STATUS_UNDEFINED)
    {
        /* remove character file */
        sprintf (stmp,"S%07d//CHDATA%02d",G_serverID,selected);
        remove (stmp);

        G_savedCharacters[selected].status=CHARACTER_STATUS_UNDEFINED;
        strcpy (G_savedCharacters[selected].name,"<empty>");

        success=TRUE;
    }

    DebugEnd();
    return (success);
}


/* store the password */
T_void StatsSetPassword(
           T_byte8 selected,
           T_byte8 password[MAX_SIZE_PASSWORD])
{
    DebugRoutine ("StatsSetPassword");
    DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);
    DebugCheck(G_activeStats != NULL) ;

    if (G_activeStats != NULL)
    strncpy(
//        G_savedCharacters[selected].password,
        G_activeStats->password,
        password,
        MAX_SIZE_PASSWORD);

    DebugEnd();
}

/* Get the stored password. */
/* LES: 03/08/96 */
T_void StatsGetPassword(
           T_byte8 selected,
           T_byte8 password[MAX_SIZE_PASSWORD])
{
    DebugRoutine ("StatsGetPassword");
    DebugCheck(selected < MAX_CHARACTERS_PER_SERVER);
    DebugCheck(G_activeStats != NULL) ;

    strncpy(
        password,
        G_activeStats->password,
//        G_savedCharacters[selected].password,
        MAX_SIZE_PASSWORD);

    DebugEnd();
}


/* routine returns the saved character ID struct number selected */
T_statsSavedCharacterID* StatsGetSavedCharacterIDStruct (T_word16 selected)
{
    T_statsSavedCharacterID *retvalue=NULL;
    DebugRoutine ("StatsGetSavedCharacterIDStruct");
    DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);

    retvalue=&G_savedCharacters[selected];

    DebugEnd();
    return (retvalue);
}


T_byte8 StatsGetActive (T_void)
{
    return (G_activeCharacter);
}

/****************************************************************************/
/*  Routine:  StatsClearSavedCharacterList                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StatsClearSavedCharacterList initializes the list of saved characters */
/*  to null.                                                                */
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
/*    strcpy                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsClearSavedCharacterList(T_void)
{
    T_word16 i ;

    DebugRoutine ("StatsClearSavedCharacterList");

    for (i=0;i<MAX_CHARACTERS_PER_SERVER;i++)
    {
        strcpy (G_savedCharacters[i].name,"<empty>");
        strcpy (G_savedCharacters[i].password,"");
        G_savedCharacters[i].status=CHARACTER_STATUS_UNDEFINED;
        G_savedCharacters[i].mail=FALSE;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  StatsGetSavedCharacterList                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StatsGetSavedCharacterList lokos at the character files in the        */
/*  character directory (current server directory) and fills out the names  */
/*  and passwords of those characters.                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_statsSavedCharArray *     -- Pointer to character array             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    mkdir                                                                 */
/*    sprintf                                                               */
/*    strcpy                                                                */
/*    fopen                                                                 */
/*    fclose                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    JDA  ??/??/??  Created                                                */
/*    LES  03/06/96  Returns pointer to statsSavedCharArray now.            */
/*                                                                          */
/****************************************************************************/

T_statsSavedCharArray *StatsGetSavedCharacterList(T_void)
{
    T_word16 i;
    T_byte8 filename[32];
    FILE *fp;

    DebugRoutine ("StatsGetSavedCharacterList");

    StatsClearSavedCharacterList() ;

    /* Check and see which files are available on the drive */
    /* and fill the slots accordingly.  Later, the G_savedCharacters */
    /* array will be filled by the a server download */

    sprintf (filename,"S%07d",G_serverID);
    mkdir (filename);

    for (i=0;i<MAX_CHARACTERS_PER_SERVER;i++)
    {
        /* try to open the saved character file read only */
        sprintf (filename,"S%07d//CHDATA%02d",G_serverID,i);
	    fp = fopen (filename,"rb");
	    if (fp!=NULL)
        {
            /* file is available, saved char slot is avail for load */
            fread(
                G_savedCharacters[i].name,
                sizeof(G_savedCharacters[i].name),
                1,
                fp) ;
//            sprintf (G_savedCharacters[i].name,"<CHAR%02d>",i);
//            strcpy  (G_savedCharacters[i].password,"password");
            strcpy  (G_savedCharacters[i].password,"");
            G_savedCharacters[i].status=CHARACTER_STATUS_OK;
            fclose(fp);
        }
    }

    DebugEnd();

    return ((T_statsSavedCharArray *)(G_savedCharacters)) ;
}

/****************************************************************************/
/*  Routine:  StatsSetSavedCharacterList                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    StatsSetSavedCharacterList is called when the client receives a new   */
/*  list of characters.  The list of characters are replaced with this new  */
/*  block.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_statsSavedCharArray *p_chars -- Pointer to block of characters      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memcpy                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void StatsSetSavedCharacterList(T_statsSavedCharArray *p_chars)
{
    DebugRoutine("StatsSetSavedCharacterList") ;

    memcpy(
        G_savedCharacters,
        p_chars,
        sizeof(T_statsSavedCharArray)) ;

    DebugEnd() ;
}

E_Boolean StatsLoadCharacterUI (T_byte8 selection)
{
    E_Boolean success=FALSE;

    DebugRoutine ("StatsLoadCharacterUI");
    DebugCheck (selection < MAX_CHARACTERS_PER_SERVER);
    /* check to make sure this character is loadable */
    if (G_savedCharacters[selection].status==CHARACTER_STATUS_OK)
    {
        /* load this character here */

        /* simulate status bar */
/*
        sprintf (stmp,"Loading Character:%s",G_savedCharacters[selection].name);
        PromptStatusBarInit (stmp,100);
        for (i=0;i<100;i+=3)
        {
            PromptStatusBarUpdate (i);
            if (rand()%2==1) i+=6;
        }
        PromptStatusBarClose();
*/
        /* load the loadcharacter ui form */
        StatsLoadCharacterUIInit ();

        FormGenericControl (&G_statsLCExit);

        DebugEnd();
    }
    return (G_statsLCSuccess);
}


/* LES: 03/06/96: Modified for state machine. */
T_void StatsLoadCharacterControl (E_formObjectType objtype,
					            	T_word16 objstatus,
					             	T_word32 objID)
{
    DebugRoutine ("StatsLoadCharacterControl");

    switch (objtype)
    {
        case FORM_OBJECT_BUTTON:
        if (objstatus==BUTTON_ACTION_RELEASED)
        {
            if (objID==LOAD_CHARACTER_BEGIN_BUTTON)
            {
                /* begin play button selected */
                /* strange place to do this, but we need to */
                /* initialize the canned saying list for this */
                /* character before the game begins*/
                ComwinInitCommunicatePage();


                SMCChooseSetFlag(
                    SMCCHOOSE_FLAG_BEGIN,
                    TRUE) ;
            }
            else if (objID==LOAD_CHARACTER_MAIL_BUTTON)
            {
//                /* mail button selected */
//                /* go to mail ui */
                /* change password button selected. */
                SMCChooseSetFlag(
                    SMCCHOOSE_FLAG_CHANGE_PASSWORD,
                    TRUE) ;
            }
            else if (objID==LOAD_CHARACTER_EXIT_BUTTON)
            {
                /* exit button selected */
                SMCChooseSetFlag(
                    SMCCHOOSE_FLAG_EXIT,
                    TRUE) ;
            }
        }
        break;
    }

    DebugEnd();
}


T_void StatsLoadCharacterUIInit (T_void)
{
    T_TxtboxID TxtboxID=NULL;
    T_byte8 tempstr[64];
    DebugRoutine ("StatsLoadCharacterUIInit");

    /* draw a window shadow */
    GrShadeRectangle (50,26,295,196,125);
	/* load the form for this page */
	FormLoadFromFile ("STATLCUI.FRM");

    /* set up some fields */
    /* set up name field */
    sprintf (tempstr,"%s",StatsGetName());
    TxtboxID=FormGetObjID(500);
    DebugCheck (TxtboxID != NULL);
    TxtboxSetData (TxtboxID,tempstr);

    /* set up attribute fields */
    TxtboxID=FormGetObjID(502);
    sprintf (tempstr,"%d",StatsGetPlayerLevel());
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(503);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_STRENGTH));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(504);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_CONSTITUTION));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(505);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_ACCURACY));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(506);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_STEALTH));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(507);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_MAGIC));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(508);
    sprintf (tempstr,"%d",StatsGetPlayerAttribute(ATTRIBUTE_SPEED));
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(509);
    sprintf (tempstr,"%s",StatsGetPlayerClassName());
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(510);
    sprintf (tempstr,"%s",StatsGetPlayerClassTitle());
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(511);
    sprintf (tempstr,"%d",StatsGetPlayerExperience());
    TxtboxSetData (TxtboxID,tempstr);

    TxtboxID=FormGetObjID(512);
    sprintf (tempstr,"%d",StatsGetPlayerExpNeeded());
    TxtboxSetData (TxtboxID,tempstr);

    /* temporarily disable mail button */
//    buttonID=FormGetObjID(LOAD_CHARACTER_MAIL_BUTTON);
//    ButtonDisable (buttonID);

//    buttonID=FormGetObjID(LOAD_CHARACTER_BEGIN_BUTTON);
//    ButtonDisable (buttonID);


    /* set the success and exit flags */
    G_statsLCExit=FALSE;
    G_statsLCSuccess=FALSE;

	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (StatsLoadCharacterControl);
	/* make sure that the color cycling will work */
    /* double buffer drawing */

    /* draw the character render */

    GraphicUpdateAllGraphicsBuffered();

    GrDrawRectangle (171,40,234,105,INVENTORY_BASE_COLOR);
    PlayerDraw (171,40);
    /* draw the character portrait */
    StatsDrawCharacterPortrait (48,42);

    DebugEnd();
}


/* LES: 03/06/96 */
T_void StatsLoadCharacterUIStart(T_void)
{
    DebugRoutine ("StatsLoadCharacterUIStart");

    StatsLoadCharacterUIInit();

    FormGenericControlStart();

    DebugEnd();
}

/* LES: 03/06/96 */
T_void StatsLoadCharacterUIUpdate(T_void)
{
    DebugRoutine ("StatsLoadCharacterUIUpdate");

    FormGenericControlUpdate();

    DebugEnd() ;
}

/* LES: 03/06/96 */
T_void StatsLoadCharacterUIEnd(T_void)
{
    DebugRoutine ("StatsLoadCharacterUIEnd");

    FormGenericControlEnd();

    DebugEnd() ;
}

/* LES: 03/06/96 */
T_void StatsMakeActive(T_byte8 selected)
{
    DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);
    G_activeCharacter = selected ;
}

/* LES: 03/06/96 */
T_word32 StatsComputeCharacterChecksum(T_void)
{
    T_word32 checksum = 0 ;
    T_word32 size ;
    T_byte8 *p_char ;
    T_word32 i ;
#if 0
T_file file ;
#endif

    DebugRoutine("StatsComputeCharacterChecksum") ;

    /* Grab the character. */
    p_char = StatsGetAsDataBlock(&size) ;

#if 0
file = FileOpen("checksum.dat", FILE_MODE_WRITE) ;
FileWrite(file, p_char, size) ;
FileClose(file) ;
#endif
    /* Make sure we got it. */
    DebugCheck(p_char != NULL) ;
    if (p_char)  {
        /* Create a progressive checksum. */
        for (i=0; i<size; i++)  {
            if (i & 1)  {
                checksum += p_char[i] ;
            } else {
                checksum ^= p_char[i] ;
            }
        }

        /* Let go of the character. */
        MemFree(p_char) ;
    }

    DebugEnd() ;

//printf("Calculate checksum %08lX\n", checksum) ;
    return checksum ;
}

/* LES: 03/06/96 */
T_void StatsReceiveCharacterData(T_byte8 *p_data, T_word32 size)
{
    T_byte8 filename[30];
    T_file file ;

    DebugRoutine("StatsReceiveCharacterData") ;

    sprintf (filename,"S%07d//CHDATA%02d",G_serverID,StatsGetActive());
    file = FileOpen(filename, FILE_MODE_WRITE) ;
    if (file != FILE_BAD)  {
        FileWrite(file, p_data, size) ;
        FileClose(file) ;
    } else {
        PromptDisplayMessage ("File I/O error saving downloaded character.");
    }

    DebugEnd() ;
}

/* LES: 03/07/96 */
T_void StatsCreateCharacterUIStart(T_void)
{
    T_word16 i;

    DebugRoutine ("StatsCreateCharacterUIStart");

//    InventoryRemoveEquippedEffects();
//    G_charTypeSelected=StatsGetPlayerClassType();
    StatsInit();
//    StatsSetPlayerClassType(CLASS_CITIZEN);
    for (i=0;i<NUM_ATTRIBUTES;i++)
    G_activeStats->Attributes[i]=G_statsCharacterAttributes[G_activeStats->ClassType][i];
    StatsCalcClassStats();

 	/* load the form for this page */
    StatsCreateCharacterUIInit();

    FormGenericControlStart();

    DebugEnd();
}

/* LES: 03/07/96 */
T_void StatsCreateCharacterUIUpdate(T_void)
{
    DebugRoutine ("StatsCreateCharacterUIUpdate");

    FormGenericControlUpdate();

    DebugEnd();
}

/* LES: 03/07/96 */
T_void StatsCreateCharacterUIEnd(T_void)
{
    DebugRoutine ("StatsCreateCharacterUIEnd");

    FormGenericControlEnd();

    DebugEnd();
}



/* LES: 03/07/96 Modified for state machine */
T_void StatsCreateCharacterControl (E_formObjectType objtype,
					            	T_word16 objstatus,
					             	T_word32 objID)
{
    T_TxtboxID TxtboxID;
    T_byte8 curclass;
    T_word16 i;

    DebugRoutine ("StatsCreateCharacterControl");

    if (objtype==FORM_OBJECT_BUTTON && objstatus==BUTTON_ACTION_RELEASED)
    {
        if (objID==301)
        {
            /* last character selected */
            curclass=StatsGetPlayerClassType();
            curclass--;
            if (curclass >= CLASS_UNKNOWN) curclass=CLASS_MAGICIAN;
            StatsSetPlayerClassType (curclass);
            for (i=0;i<NUM_ATTRIBUTES;i++)
                G_activeStats->Attributes[i]=G_statsCharacterAttributes[G_activeStats->ClassType][i];
            StatsUpdateCreateCharacterUI();
        }
        else if (objID==302)
        {
            /* next character selected */
            curclass=StatsGetPlayerClassType();
            curclass++;
            if (curclass >= CLASS_UNKNOWN) curclass=CLASS_CITIZEN;
            StatsSetPlayerClassType (curclass);
            for (i=0;i<NUM_ATTRIBUTES;i++)
                G_activeStats->Attributes[i]=G_statsCharacterAttributes[G_activeStats->ClassType][i];
            StatsUpdateCreateCharacterUI();
        }
        else if (objID==303)
        {
            /* Get character name */
            TxtboxID=FormGetObjID (501);

            /* Is there a name */
            if (strlen(TxtboxGetData(TxtboxID)) == 0)  {
                /* Character has no name */
                /* Put up a "that's bad prompt" */
                PromptDisplayMessage("Please enter a character name") ;

                /* Restart the create form */
                StatsCreateCharacterUIInit();
            } else {
                /* character accepted */

                /* set character name */
                StatsSetName (TxtboxGetData(TxtboxID));

                /* set character default inventory */
                InventorySetDefaultInventoryForClass();

                /* initialize canned sayings */
                ComwinInitCommunicatePage();

                /* set exit flags */
    //            G_success=TRUE;
    //            G_exit=TRUE;
                SMCChooseSetFlag(SMCCHOOSE_FLAG_CREATE_COMPLETE, TRUE) ;
            }
        }
        else if (objID==304)
        {
            /* character canceled */
//            G_success=FALSE;
//            G_exit=TRUE;
            SMCChooseSetFlag(SMCCHOOSE_FLAG_CREATE_ABORT, TRUE) ;
        }
    }

	DebugEnd();
}

/* LES: 03/12/96 Created */
T_void *StatsGetAsDataBlock(T_word32 *p_size)
{
    FILE *fout;
    T_void *p_data ;

    DebugRoutine("StatsGetAsDataBlock") ;
    DebugCheck(p_size != NULL) ;

    /* go ahead and write the stats structure + equip to disk for now */
    fout = fopen ("tempchar.$$$","wb");
    if (fout != NULL)
    {
        /* write player statistics */
        fwrite (G_activeStats,sizeof(T_playerStats),1,fout);

        /* write player inventory list */
        /* get up to 200 inventory items for player */
        /* and append each to file. */
        InventoryWriteItemsList(fout);

        fclose (fout);

        /* Load the file into memory. */
        p_data = FileLoad("tempchar.$$$", p_size) ;
        remove("tempchar.$$$") ;
    }
    else
    {
        /* inform user of error */
        PromptDisplayMessage ("File I/O error getting data block.");
    }

    DebugEnd() ;

    return p_data ;
}

/* routine returns the player's total saved in bank wealth in copper coins */
T_word32 StatsGetPlayerTotalSavedWealth (T_void)
{
    T_word32 wealth;

    DebugRoutine ("StatsGetPlayerTotalSavedWealth");
    wealth=(StatsGetPlayerSavedCoins(COIN_TYPE_PLATINUM)*1000) +
           (StatsGetPlayerSavedCoins(COIN_TYPE_GOLD)*100) +
           (StatsGetPlayerSavedCoins(COIN_TYPE_SILVER)*10) +
           (StatsGetPlayerSavedCoins(COIN_TYPE_COPPER)*1);

    DebugEnd();
    return (wealth);
}


/* routine returns the player's total carried wealth in copper coins */
T_word32 StatsGetPlayerTotalCarriedWealth (T_void)
{
    T_word32 wealth;
    DebugRoutine ("StatsGetPlayerTotalCarriedWealth");

    wealth=(StatsGetPlayerCoins(COIN_TYPE_PLATINUM)*1000) +
           (StatsGetPlayerCoins(COIN_TYPE_GOLD)*100) +
           (StatsGetPlayerCoins(COIN_TYPE_SILVER)*10) +
           (StatsGetPlayerCoins(COIN_TYPE_COPPER)*1);

    DebugEnd();
    return (wealth);
}


/* changes carried wealth based on amount in copper coins */
T_void StatsChangePlayerTotalCarriedWealth (T_sword32 amount)
{
    T_word16 i;
    T_sword16 platinum,gold,silver,copper;
    T_word32 change;
    DebugRoutine ("StatsChangePlayerTotalCarriedCoins");

    /* break value apart into fundimentals */
    platinum=amount/1000;
    amount-=(platinum*1000);
    gold=amount/100;
    amount-=(gold*100);
    silver=amount/10;
    amount-=(silver*10);
    copper=amount;


    /* add or subtract amount */
    StatsSetPlayerCoins(COIN_TYPE_PLATINUM,
                        StatsGetPlayerCoins(COIN_TYPE_PLATINUM)+platinum);
    StatsSetPlayerCoins(COIN_TYPE_GOLD,
                        StatsGetPlayerCoins(COIN_TYPE_GOLD)+gold);
    StatsSetPlayerCoins(COIN_TYPE_SILVER,
                        StatsGetPlayerCoins(COIN_TYPE_SILVER)+silver);
    StatsSetPlayerCoins(COIN_TYPE_COPPER,
                        StatsGetPlayerCoins(COIN_TYPE_COPPER)+copper);

    /* rebalance (i.e. make change) */
    for (i=COIN_TYPE_COPPER;i<COIN_TYPE_PLATINUM;i++)
    {
        if (StatsGetPlayerCoins(i)<0)
        {
            /* subtract one of higher value */
            StatsSetPlayerCoins((i+1),StatsGetPlayerCoins(i+1)-1);
            /* add 10 to this value */
            StatsSetPlayerCoins((i),StatsGetPlayerCoins(i)+10);
        }
        else if (StatsGetPlayerCoins(i)>=10)
        {
            /* rebalance coins upward */
            change=StatsGetPlayerCoins(i)/10;
            StatsSetPlayerCoins((i+1),StatsGetPlayerCoins(i+1)+change);
            StatsSetPlayerCoins(i,StatsGetPlayerCoins(i)-(change*10));
        }
    }

    DebugEnd();
}


E_Boolean StatsIOwnHouse (T_word32 houseNum)
{
    E_Boolean iOwn=FALSE;
    DebugRoutine ("StatsIOwnThisHouse");
    DebugCheck (houseNum < NUM_HOUSES);
//    if (houseNum==HARD_FORM_HOUSE1) iOwn=TRUE;
    if (G_activeStats->HouseOwned[houseNum]==TRUE) iOwn=TRUE;

    DebugEnd();
    return (iOwn);
}

/* sets the ownership state of housenum */
T_void    StatsSetIOwnHouse (T_word32 houseNum, E_Boolean to)
{
    DebugRoutine ("StatsSetIOwnHouse");

    DebugCheck (houseNum < NUM_HOUSES);

    G_activeStats->HouseOwned[houseNum]=to;

    DebugEnd();
}


/* returns true if player has journal note page whichpage */
E_Boolean StatsPlayerHasNotePage    (T_word16 whichpage)
{
    E_Boolean haspage=FALSE;
    T_word16 byte;
    T_byte8 bit;

    DebugRoutine ("StatsPlayerHasNotePage");
    DebugCheck (whichpage < MAX_NOTES);

    byte=whichpage>>3;
    bit=1 << (whichpage & 7);

    /* return TRUE if bit is set */
    haspage=(G_activeStats->HasNotes[byte]&bit) ? TRUE : FALSE;

    DebugEnd();
    return (haspage);
}


/* gives player journal note page whichpage */
T_void    StatsAddPlayerNotePage    (T_word16 whichpage)
{
    T_word16 byte;
    T_byte8 bit;

    DebugRoutine ("statsAddPlayerNotePage");
    DebugCheck (whichpage < MAX_NOTES);

    if (!StatsPlayerHasNotePage(whichpage))
    {
        byte=whichpage>>3;
        bit=1 << (whichpage & 7);

        /* set bit */
        G_activeStats->HasNotes[byte] |= bit;
        /* set counter */
        G_activeStats->NumNotes++;
    }
    else
    {
        MessageAdd ("^007 You already have this journal entry");
    }

    NotesGotoPageID (whichpage);
    if (BannerFormIsOpen (BANNER_FORM_JOURNAL))
    {
        NotesUpdateJournalPage();
    }
    else
    {
        /* force opening of banner */
        if (ClientIsInGame())
        {
            BannerOpenForm (BANNER_FORM_JOURNAL);
            NotesUpdateJournalPage();
        }
    }

    DebugEnd();
}


/* removes player journal note page whichpage */
T_void StatsRemovePlayerNotePage (T_word16 whichpage)
{
    T_word16 byte;
    T_byte8 bit;

    DebugRoutine ("StatsRemovePlayerNotePage");
    DebugCheck (whichpage < MAX_NOTES);

    if (StatsPlayerHasNotePage(whichpage))
    {
        byte=whichpage>>3;
        bit=1 << (whichpage & 7);

        /* clear bit */
        G_activeStats->HasNotes[byte] &= (~bit);

        /* decrement counter */
        G_activeStats->NumNotes--;
        DebugCheck (G_activeStats->NumNotes < MAX_NOTES);

        /* force opening of banner */
        if (BannerFormIsOpen (BANNER_FORM_JOURNAL))
        {
            NotesUpdateJournalPage();
        }
    }

    DebugEnd();
}


T_word16 StatsGetNumberOfNotes (T_void)
{
    return (G_activeStats->NumNotes);
}


/* returns index conversion where pageIndex is the nth page in all */
/* player owned notes -- returns MAXNOTES if player currently has 0 */
/* notes */
T_word16  StatsGetPlayerNotePageID  (T_word16 pageIndex)
{
    T_word16 pageID=MAX_NOTES;
    T_word16 i=0,count=0;
    T_word16 byte;
    T_byte8 bit;

    DebugRoutine ("StatsGetPlayerNotePageID");

    if (pageIndex>0 && StatsGetNumberOfNotes()>0)
    {
        for (i=0;i<MAX_NOTES;i++)
        {
            byte=i>>3;
            bit=1<<(i&7);
            if (G_activeStats->HasNotes[byte]&bit)
            {
                count++;
                if (count == pageIndex)
                {
                    break;
                }
            }
        }
        pageID=i;
    }

    DebugEnd();
    return (pageID);
}


/* returns player 'notes' data for notes display field */
T_byte8*  StatsGetPlayerNotes       (T_void)
{
    return (G_activeStats->Notes);
}

/* saves player 'notes field' data */
T_void    StatsSetPlayerNotes       (T_byte8 *fieldData)
{
    DebugRoutine ("StatsSetPlayerNotes");
    DebugCheck (strlen(fieldData)<1000);

    /* copy data to stats area */
    strcpy (G_activeStats->Notes,fieldData);

    DebugEnd();
}

/* returns true if the last hit rolled 'critical' */
E_Boolean StatsHitWasCritical (T_void)
{
    return (G_hitWasCritical);
}


/* returns the type(s) of damage the player currently does with melee */
E_effectDamageType StatsGetPlayerDamageType (T_void)
{
    /* start with normal damage */
    E_effectDamageType damageType=1;
    DebugRoutine ("StatsGetPlayerDamageType");

    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_FIRE_ATTACK) ? EFFECT_DAMAGE_FIRE : 0);
    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_ACID_ATTACK) ? EFFECT_DAMAGE_ACID : 0);
    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_POISON_ATTACK) ? EFFECT_DAMAGE_POISON : 0);
    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_ELECTRICITY_ATTACK) ? EFFECT_DAMAGE_ELECTRICITY : 0);
    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_PIERCING_ATTACK) ? EFFECT_DAMAGE_PIERCING : 0);
    damageType+=(EffectPlayerEffectIsActive(PLAYER_EFFECT_MANA_DRAIN_ATTACK) ? EFFECT_DAMAGE_MANA_DRAIN : 0);

    DebugEnd();
    return (damageType);
}


static T_void StatsReorientPlayerView (T_void)
{
    T_sword32 angle;
    DebugRoutine ("StatsReorientPlayerView");

    angle=rand()-rand();
    angle=angle>>5;

//    MessagePrintf ("Reorienting - angle=%d\n",angle);


    if (angle < 0)
    {
//        PlayerTurnLeft(-angle);
        ViewOffsetView(-angle) ;
    }
    else
    {
//        PlayerTurnRight(angle);
        ViewOffsetView(angle) ;
    }

    DebugEnd();
}

/* routine returns TRUE if player has successfully identified object */
/* type number objType */
E_Boolean StatsPlayerHasIdentified (T_word16 objType)
{
    T_word16 byte=0;
    T_byte8 bit=0;
    E_Boolean hasIdentified=FALSE;
    DebugRoutine ("StatsPlayerHasIdentified");

    byte=objType>>3;
    bit=1<<(objType&7);
    hasIdentified= (G_activeStats->Identified[byte]&bit) ? TRUE:FALSE;

    DebugEnd();
    return (hasIdentified);
}

/* routine sets itentify flag TRUE for object type number objType */
T_void    StatsPlayerIdentify (T_word16 objType)
{
    T_word16 byte=0;
    T_byte8 bit=0;
    DebugRoutine ("StatsPlayerIdentify");

    byte=objType>>3;
    bit=1<<(objType&7);

    DebugCheck (byte <= 8192);

    G_activeStats->Identified[byte]|=bit;

    DebugEnd();
}

T_void StatsUpdatePastPlace(
           T_word16 adventureNumber,
           T_word16 mapNumber)
{
    T_word16 i, j ;
    T_pastPlaces *p_places ;

    DebugRoutine("StatsUpdatePastPlace") ;

    p_places = StatsGetPastPlaces() ;

    /* See if there is a previous adventure already stored. */
    for (i=0; i<p_places->numInList; i++)  {
        /* if there is a match, stop. */
        if (p_places->places[i].adventureNumber == adventureNumber)
            break ;
    }

    /* If never found a place, make room for another */
    if ((i==p_places->numInList) && (i != MAX_PAST_PLACES))
        p_places->numInList++ ;

    /* Close up the spot (found or not found) */
    for (j=i; j>0; j--)  {
        if (j != MAX_PAST_PLACES)
            p_places->places[j] = p_places->places[j-1] ;
    }

    /* This makes the top of the list the newest entry */
    /* Fill it with the given info. */
    p_places->places[0].adventureNumber = adventureNumber ;
    p_places->places[0].lastLevelNumber = mapNumber ;

//printf("adv %d stored %d (%d spots)\n", adventureNumber, mapNumber, p_places->numInList) ;
    DebugEnd() ;
}

T_word16 StatsFindPastPlace(T_word16 adventureNumber)
{
    T_word16 i ;
    T_pastPlaces *p_places ;
    T_word16 level ;

    DebugRoutine("StatsFindPastPlace") ;

    p_places = StatsGetPastPlaces() ;

    for (i=0; i<p_places->numInList; i++)  {
        /* if there is a match, stop. */
        if (p_places->places[i].adventureNumber == adventureNumber)
            break ;
    }

    /* If we got to the end of list, no level was found. */
    if (i==p_places->numInList)
        level = 0 ;
    else
        level = p_places->places[i].lastLevelNumber ;

    /* If the level equals the adventure number, return no level. */
    /* NOTE:  The adventure number is always the number of the */
    /*        first level (thus, if the last level was the first */
    /*        level, the player has never gone past the first level */
    /*        and is not able to bypass previous levels). */
    if (level == adventureNumber)
        level = 0 ;

    DebugEnd() ;

//printf("Find %d, got %d\n", adventureNumber, level) ;
    return level ;
}


/* routine loads a character (currently from client hard drive) */
/* character loaded is designated by selected and G_serverID (client.c) */
/* LES: Low level load character:  Don't worry about what server thinks. */
E_Boolean StatsLoadCharacter (T_byte8 selected)
{
    E_Boolean success=FALSE;
    T_byte8 filename[30];
    FILE *fin;

    DebugRoutine ("StatsLoadCharacter");
    DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);

    /* Make sure that there is no character in memory. */
    StatsInit();
//    StatsUnloadCharacter() ;
//    InventoryRemoveEquippedEffects();

    /* check to make sure there is an ok character in this slot */
    if (G_savedCharacters[selected].status==CHARACTER_STATUS_OK)
    {
        /* right now, get the blocks of data off of client drive */
        sprintf (filename,"S%07d\\CHDATA%02d",G_serverID,selected);
//printf("Loading char '%s'\n", filename) ;
        fin = fopen (filename,"rb");
        if (fin!=NULL)
        {
            success=TRUE;
            /* file is available, load character */
            /* get the statistics */
            fread (G_activeStats,sizeof(T_playerStats),1,fin);
            /* get the inventory items */
            InventoryReadItemsList(fin);
            fclose (fin);
            G_activeCharacter=selected;
            G_lastLoadedCharacter = selected ;

            /* LES: 03/28/96 */
            /* Update all the equiped body parts. */
            InventoryUpdatePlayerEquipmentBodyParts() ;

            /* LES: 06/25/96 */
            /* Restore equipped effects. */
            InventoryRestoreEquippedEffects();

        } else {
            G_activeCharacter = NO_CHARACTER_LOADED ;
            G_lastLoadedCharacter = NO_CHARACTER_LOADED ;
        }
    } else {
        G_activeCharacter = NO_CHARACTER_LOADED ;
        G_lastLoadedCharacter = NO_CHARACTER_LOADED ;
    }
    DebugEnd();
    return (success);
}



/* routine saves character, file is designated by the stats */
/* LES:  Low level save character:  Just saves the character without */
/* respect to the server. */
E_Boolean StatsSaveCharacter (T_byte8 selected)
{
    E_Boolean success=FALSE;
    FILE *fout;
    T_byte8 filename[30];
    T_word16 objcnt=0;
    E_Boolean isGod ;

    DebugRoutine ("StatsSaveCharacter");

    /* Make sure god mode is turned off. */
    isGod = ClientIsGod() ;
    if (isGod)  {
        EffectSoundOff() ;
        StatsToggleGodMode();
        EffectSoundOn() ;
    }

    InventoryRemoveEquippedEffects();
    EffectRemoveAllPlayerEffects();

    if (selected != NO_CHARACTER_LOADED)  {
        DebugCheck (selected < MAX_CHARACTERS_PER_SERVER);

        /* go ahead and write the stats structure + equip to disk for now */
        sprintf (filename,"S%07d//CHDATA%02d",G_serverID,selected);
        fout = fopen (filename,"wb");
        if (fout != NULL)
        {
            /* write player statistics */
            fwrite (G_activeStats,sizeof(T_playerStats),1,fout);
            /* write player inventory list */
            /* get up to 200 inventory items for player */
            /* and append each to file. */
            InventoryWriteItemsList(fout);

            fclose (fout);
            strncpy (G_savedCharacters[selected].name,G_activeStats->Name,30);
            G_savedCharacters[selected].status=CHARACTER_STATUS_OK;
            success=TRUE;
            G_activeCharacter=selected;
        }
        else
        {
            /* inform user of error */
            PromptDisplayMessage ("File I/O error saving character.");
            success=FALSE;
        }
    }


    /* restore equipped effects */
    InventoryRestoreEquippedEffects();

    /* Turn god mode back on */
    if (isGod)  {
        EffectSoundOff() ;
        StatsToggleGodMode();
        EffectSoundOn() ;
    }

    /* Make sure that there is no character in memory. */
//    StatsUnloadCharacter() ;

    DebugEnd();

    return (success);
}
