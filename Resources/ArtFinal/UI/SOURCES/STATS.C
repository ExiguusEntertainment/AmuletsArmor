/****************************************************************************/
/*    FILE:  STATS.C                                                        */
/****************************************************************************/

#include "standard.h"

static T_sword16 G_Health;
static T_sword16 G_Stamina;
static T_sword16 G_Mana;
static T_word16 G_RegenHealth;
static T_word16 G_RegenMana;
static T_word16 G_JumpPower;
static T_word16 G_Tallness;
static T_word16 G_ClimbHeight;
static T_word16 G_MaxVRunning;
static T_word16 G_MaxVWalking;
static T_word16 G_HeartRate;
static T_word16 G_MaxFallV;
static E_Boolean G_playerisalive;
static E_Boolean attributes[MAX_ATTRIBUTES];

T_void StatsInit (T_void)
{
	T_word16 i;

	DebugRoutine ("StatsInit");

	G_Health = 1000;
	G_Mana = 1000;
	G_RegenHealth = 5;
	G_RegenMana = 5;
	G_JumpPower = 75<<8;
	G_Tallness = 50;
	G_ClimbHeight = 40;
	G_MaxVRunning = 32;
	G_MaxVWalking = 24;
	G_HeartRate = 60;
	G_MaxFallV = 31000;
	G_playerisalive=FALSE;
	for (i=0;i<MAX_ATTRIBUTES;i++) attributes[i]=FALSE;
	StatsCalcHeartRate();

	DebugEnd();
}

T_void StatsSetAttribute (E_SpellFlag attribute, E_Boolean to)
{
	DebugRoutine ("StatsSetAttribute");
	DebugCheck (attribute != NONE);
	DebugCheck (attribute < UNKNOWN);
	DebugCheck (attribute < MAX_ATTRIBUTES);
	DebugCheck (to < BOOLEAN_UNKNOWN);

	attributes[attribute]=to;

	DebugEnd();
}


E_Boolean StatsGetAttribute (E_SpellFlag attribute)
{
	DebugRoutine ("StatsGetAttribute");
	DebugCheck (attribute != NONE);
	DebugCheck (attribute < UNKNOWN);
	DebugCheck (attribute < MAX_ATTRIBUTES);

	DebugEnd();
	return (attributes[attribute]);
}



T_sword16 StatsGetPlayerHealth (T_void)
{
	return (G_Health);
}

T_void StatsSetPlayerHealth (T_sword16 amount)
{
	DebugRoutine ("StatsSetPlayerHealth");

	DebugCheck (amount>MIN_HEALTH);
	DebugCheck (amount<MAX_HEALTH);

	G_Health = amount;
	G_playerisalive=FALSE;
	StatsCalcHeartRate();
	DebugEnd();

}


T_sword16 StatsGetManaAvail (T_void)
{
    return (G_Mana);
}


T_word16 StatsGetJumpPower (T_void)
{
    return (G_JumpPower);
}


T_void StatsSetJumpPower (T_word16 amt)
{
    G_JumpPower=amt;
}


T_word16 StatsGetTallness (T_void)
{
	return (G_Tallness);
}

T_word16 StatsGetClimbHeight (T_void)
{
	return (G_ClimbHeight);
}

T_word16 StatsGetMaxVRunning (T_void)
{
	return (G_MaxVRunning);
}

T_void StatsSetMaxVRunning (T_word16 amt)
{
	G_MaxVRunning=amt;
}

T_word16 StatsGetMaxVWalking (T_void)
{
	return (G_MaxVWalking);
}

T_void StatsSetMaxVWalking (T_word16 amt)
{
	G_MaxVWalking=amt;
}

T_word16 StatsGetHeartRate (T_void)
{
	return (G_HeartRate);
}

T_word16 StatsGetMaxFallV (T_void)
{
	return (G_MaxFallV);
}

T_void StatsSetMaxFallV (T_word16 amt)
{
	G_MaxFallV=amt;
}

T_word16 StatsGetManaLeft (T_void)
{
	return (G_Mana);
}

T_void StatsChangeMana (T_sword16 amt)
{
	G_Mana+=amt;
	if (G_Mana>1000) G_Mana=1000;
	if (G_Mana<0) G_Mana=0;
}

E_Boolean StatsPlayerIsAlive (T_void)
{
	return (G_playerisalive);
}


T_void StatsPlayerSetAlive (T_void)
{
	G_playerisalive=TRUE;
}


T_void StatsHurtPlayer (T_word16 amt)
{
	T_word16 i;

	DebugRoutine ("StatsHurtPlayer");
	DebugCheck (amt<=MAX_DAMAGEAMT);

	SoundPlayByName("ImHit1");
	if (StatsGetAttribute (SPELL_INVULNERABLE)==FALSE)
	{
		G_Health-=amt;
		ColorAddGlobal (amt>>1,-amt>>1,-amt>>1);
		StatsCalcHeartRate();
	}
	//code added 05/26/95 JDA

	if (StatsGetPlayerHealth() <= 0)
	{
	   StatsSetPlayerHealth (1000) ; //u died

	   for (i=0;i<30;i++)
	   {
			ColorAddGlobal (-5,-5,-5);
			ColorUpdate(1);
			delay(50);
	   }
	   ViewPlayerStop();
	   G_playerisalive=FALSE;
	   ClientLogoff();
	}

	DebugEnd();
}


T_void StatsHealPlayer (T_word16 amt)
{
	DebugRoutine ("StatsHealPlayer");

	if (G_Health<MAX_HEALTH)
	{
		G_Health+=amt;
		if (G_Health>MAX_HEALTH) G_Health=MAX_HEALTH;
		SoundPlayByNumber(19) ;
		ColorAddGlobal (amt>>2,amt>>2,amt>>2);
	}
	StatsCalcHeartRate();

	DebugEnd();
}


T_void StatsCalcHeartRate (T_void)
{
	T_word16 wvalue;

	DebugRoutine ("StatsCalcHeartRate");

	G_HeartRate = G_Health>>4;
	if (G_HeartRate < 5) G_HeartRate = 5;
	if (G_HeartRate > 63) G_HeartRate = 63;

	ColorSetColor(13,63-StatsGetHeartRate(),0,0);

	if (G_Health<=1000 && G_Health>=100) ColorSetColor (13,63-(G_Health/16),0,0);
	else if (G_Health>1000)
	{
		wvalue=(G_Health-1000)/8;
		ColorSetColor (13,wvalue,wvalue,wvalue);
	}

	DebugEnd();
}