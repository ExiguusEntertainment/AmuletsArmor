/****************************************************************************/
/*    FILE:  STATS.H                                                        */
/****************************************************************************/
#ifndef _STATS_H_
#define _STATS_H_

#define MIN_HEALTH -10
#define MAX_HEALTH 1500
#define MAX_DAMAGEAMT 1500
#define MAX_ATTRIBUTES 30

typedef enum
{
	NONE,
	SPELL_WATER_WALK,
	SPELL_LAVA_WALK,
	SPELL_INVULNERABLE,
	SPELL_FEATHER_FALL,
	SPELL_LO_GRAV,
	SPELL_JOUST_FLY,
	SPELL_AIR_WALK,
	SPELL_STICKY_FEET,
	UNKNOWN
} E_SpellFlag;

T_void StatsInit (T_void);
T_sword16 StatsGetPlayerHealth (T_void);
T_sword16 StatsGetManaAvail (T_void);
T_void StatsSetPlayerHealth (T_sword16 amount);
T_word16 StatsGetJumpPower (T_void);
T_void StatsSetJumpPower (T_word16 amt);
T_word16 StatsGetTallness (T_void);
T_word16 StatsGetClimbHeight (T_void);
T_word16 StatsGetMaxVRunning (T_void);
T_void StatsSetMaxVRunning (T_word16 amt);
T_word16 StatsGetMaxVWalking (T_void);
T_void StatsSetMaxVWalking (T_word16 amt);
T_word16 StatsGetHeartRate (T_void);
T_word16 StatsGetMaxFallV (T_void);
T_void StatsSetMaxFallV (T_word16 amt);
T_word16 StatsGetManaLeft (T_void);

T_void StatsSetAttribute (E_SpellFlag attribute, E_Boolean to);
E_Boolean StatsGetAttribute (E_SpellFlag attribute);

T_void StatsChangeMana (T_sword16 amt);
T_void StatsHurtPlayer (T_word16 amt);
T_void StatsHealPlayer (T_word16 amt);
T_void StatsCalcHeartRate (T_void);
E_Boolean StatsPlayerIsAlive (T_void);
T_void StatsPlayerSetAlive (T_void);

#endif