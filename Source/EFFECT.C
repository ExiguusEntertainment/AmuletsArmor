/*-------------------------------------------------------------------------*
 * File:  EFFECT.C
 *-------------------------------------------------------------------------*/
/**
 * The effect subsystem is where all the items get their properties
 * and affect the player and world around the player.  Effects have
 * a type of action, a trigger for that action, and up to 3 values
 * to customize the action.
 *
 * @addtogroup EFFECT
 * @brief Item and Spell Effects
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "BANNER.H"
#include "CLIENT.H"
#include "COLOR.H"
#include "CONTROL.H"
#include "CSYNCPCK.H"
#include "EFFECT.H"
#include "INVENTOR.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "OVERHEAD.H"
#include "PICS.H"
#include "PLAYER.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "SPELLS.H"
#include "STATS.H"
#include "TICKER.H"
#include "VIEW.H"
#include "VIEWFILE.H"

/* double link list for current player effects */
static T_doubleLinkList G_playerEffectsList=DOUBLE_LINK_LIST_BAD;

/* table containing power levels for current player effects */
static T_word16 G_playerEffectPowers[PLAYER_EFFECT_UNKNOWN];
static T_word16 G_playerEffectDisableFlags[PLAYER_EFFECT_UNKNOWN];

/* internal routine prototypes*/
static T_void EffectAddPlayerEffect (T_word16 data1,
                                     T_word16 data2,
                                     T_word16 data3,
                                     T_void*  p_owner);

static T_void EffectRemovePlayerEffect (T_void *p_owner);

static E_Boolean EffectDestroyElement (T_doubleLinkListElement killme);

static T_void EffectStartPlayerEffect (T_playerEffectStruct *p_effect);
static T_void EffectStopPlayerEffect (T_playerEffectStruct *p_effect);

static E_Boolean G_init = FALSE ;
static E_Boolean G_effectSoundOn=TRUE;

/*-------------------------------------------------------------------------*
 * Routine:  EffectInit
 *-------------------------------------------------------------------------*/
/**
 *  Initializes variables for player effects
 *
 *<!-----------------------------------------------------------------------*/
T_void EffectInit (T_void)
{
    T_word16 i;
    DebugRoutine ("EffectInit");
    DebugCheck(G_init == FALSE) ;

    G_init = TRUE ;

    /* init linked list for player effects */
    G_playerEffectsList=DoubleLinkListCreate();

    /* clear Powers table */
    for (i=0;i<PLAYER_EFFECT_UNKNOWN;i++)
    {
        G_playerEffectPowers[i]=0;
        G_playerEffectDisableFlags[i]=0;
    }

    DebugCheck (G_playerEffectsList!=DOUBLE_LINK_LIST_BAD);

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  EffectFinish
 *-------------------------------------------------------------------------*/
/**
 *  Cleans out the global double link list for player effects
 *
 *<!-----------------------------------------------------------------------*/
T_void EffectFinish (T_void)
{
    T_doubleLinkListElement node ;

    DebugRoutine ("EffectFinish");
    DebugCheck(G_init == TRUE) ;

    /* Clear the effect list */
    if (G_playerEffectsList != DOUBLE_LINK_LIST_BAD)  {
        /* Clear the list while we have elements. */
        node = DoubleLinkListGetFirst(G_playerEffectsList) ;
        while (node != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            EffectDestroyElement(node) ;
            node = DoubleLinkListGetFirst(G_playerEffectsList) ;
        }
    }

    DoubleLinkListDestroy(G_playerEffectsList) ;
    G_playerEffectsList = DOUBLE_LINK_LIST_BAD;

    G_init = FALSE ;

    DebugEnd();
}


/*-------------------------------------------------------------------------*
 * Routine:  Effect
 *-------------------------------------------------------------------------*/
/**
 *  Creates a specified effect
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean Effect(E_effectType effecttype,
                 E_effectTriggerType triggertype,
                 T_word16 data1,
                 T_word16 data2,
                 T_word16 data3,
                 T_void* p_owner)
{
    E_Boolean retvalue=TRUE;
    T_3dObject *p_object;
    T_word16 subtype,duration,power;
    T_byte8 bolttype;
    T_3dObject *p_obj;
    T_inventoryItemStruct *p_inv;
    DebugRoutine ("Effect");

//    printf ("\neffect called with type=%d data1=%d data2=%d data3=%d\n",effecttype,data1,data2,data3);
//    fflush (stdout);

    subtype=data1;
    duration=data2;
    power=data3;

    switch (effecttype)
    {
        case EFFECT_READY_WEAPON:
//        OverlaySetAnimation (data1);
        if (data2==0) StatsSetWeaponBaseDamage (2);
        else StatsSetWeaponBaseDamage ((T_sbyte8)data2);
        StatsSetWeaponBaseSpeed ((T_sbyte8)data3);

        /* play a 'ready weapon' sound effect here */
        p_inv=InventoryCheckItemInReadyHand();
        if (p_inv != NULL)  {
            DebugCheck(p_inv->object != NULL) ;
            DebugCheck(ObjectIsValid(p_inv->object)) ;

            /* make sure it's a weapon */
            // TODO: Does this OjbectGetScriptHandle work correctly?
            if (((T_word32)ObjectGetScriptHandle(p_inv->object))<100 && (data2!=0) && (G_effectSoundOn==TRUE))
            {
                switch (p_inv->itemdesc.subtype)
                {
                    case EQUIP_WEAPON_TYPE_DAGGER:
                    ClientSyncSendActionAreaSound(SOUND_SHEATH_DAGGER,500, FALSE);
                    //SoundPlayByNumber (SOUND_SHEATH_DAGGER,255);
                    break;

                    case EQUIP_WEAPON_TYPE_LONGSWORD:
                    ClientSyncSendActionAreaSound(SOUND_SHEATH_LONGSWD,500, FALSE);
                    //SoundPlayByNumber (SOUND_SHEATH_LONGSWD,255);
                    break;

                    case EQUIP_WEAPON_TYPE_SHORTSWORD:
                    ClientSyncSendActionAreaSound(SOUND_SHEATH_SHTSWD,500, FALSE);
                    //SoundPlayByNumber (SOUND_SHEATH_SHTSWD,255);
                    break;

                    case EQUIP_WEAPON_TYPE_NONE:
                    default:
                    /* bad weapon in hand! */
                    break;
                }
            }
        }
        break;

        case EFFECT_SET_ARMOR:
			/* location = data2, value = data1 */
			StatsSetArmorValue (data2,(T_byte8)data1);
			break;

        case EFFECT_TAKE_COIN:
			StatsAddCoin (data1,data2);
			break;

        case EFFECT_TAKE_AMMO:
        if (p_owner!=NULL)
        {
            p_object=(T_3dObject *)p_owner;
            if (ObjectGetAccData(p_object)==0)  {
                /* Check if this is a quiver or a bolt. */
                if ((ObjectGetType(p_object)>=144) && (ObjectGetType(p_object)<=150))
                    ObjectSetAccData(p_object, 12);    /* Quiver */
                else
                    ObjectSetAccData(p_object, 1) ;    /* bolt */
            }
            StatsAddBolt (data1,ObjectGetAccData(p_object));
        }
        else
        {
            StatsAddBolt (data1,data2);
        }

        break;

        case EFFECT_ADD_PLAYER_EFFECT:
        EffectAddPlayerEffect (subtype,duration,power,p_owner);
        break;

        case EFFECT_REMOVE_PLAYER_EFFECT:
        EffectRemovePlayerEffect (p_owner);
        break;

        case EFFECT_MOD_PLAYER_FOOD:
        StatsChangePlayerFood ((T_sword16)power);
        break;

        case EFFECT_MOD_PLAYER_WATER:
        StatsChangePlayerWater((T_sword16)power);
        break;

        case EFFECT_MOD_PLAYER_HEALTH:
        StatsChangePlayerHealth ((T_sword16)power);
        break;

        case EFFECT_MOD_PLAYER_MANA:
        StatsChangePlayerMana ((T_sword16)power);
        break;

        case EFFECT_MOD_PLAYER_EXPERIENCE:
        StatsChangePlayerExperience ((T_sword16)power);
        break;

        case EFFECT_REORIENT:
//        PlayerSetAngle(rand()) ;
        PlayerTurnLeft(rand()) ;
        PlayerMakeSoundLocal (SOUND_PLAYER_CONFUSED);
        MessageAdd("You feel dizzy") ;
        break ;

        case EFFECT_PLAYER_LEAP:
        PlayerJump (StatsGetJumpPower()+power);
        ColorAddGlobal (0,0,20);
//        SoundPlayByNumber (2000,255);
//        ClientSyncSendActionAreaSound(2000,500);
        PlayerMakeSoundGlobal(2000, 500) ;
        break;

        case EFFECT_CREATE_PROJECTILE:
//        ClientShootFireball();
        //* will be something like */
        // ClientCreateProjectile (E_effectMissileType type, duration, power);
        ClientCreateProjectile(data1, data2, data3) ;

        /* add a proper color effect */
        switch (data1)
        {
            case EFFECT_MISSILE_MAGIC_DART:
            case EFFECT_MISSILE_HOMING_DART:
            ColorAddGlobal (15,15,15);
 //         SoundPlayByNumber (1000,255);
            ClientSyncSendActionAreaSound(1000,500,FALSE);
            break;

            case EFFECT_MISSILE_ACID_BALL:
            case EFFECT_MISSILE_HOMING_ACID_BALL:
            ColorAddGlobal (0,15,0);
//          SoundPlayByNumber (1001,255);
            ClientSyncSendActionAreaSound(1001,500,FALSE);
            break;

            case EFFECT_MISSILE_FIREBALL:
            case EFFECT_MISSILE_HOMING_FIREBALL:
            ColorAddGlobal (15,0,0);
//          SoundPlayByNumber (1002,255);
            ClientSyncSendActionAreaSound(1002,500,FALSE);
            break;

            case EFFECT_MISSILE_HOMING_DEATHBALL:
            case EFFECT_MISSILE_PLASMA:
            ColorAddGlobal (30,0,0);
//          SoundPlayByNumber (1002,255);
            ClientSyncSendActionAreaSound(1003,500,FALSE);
            break;

            case EFFECT_MISSILE_DISPELL_MAGIC:
            case EFFECT_MISSILE_MANA_DRAIN:
            ColorAddGlobal (15,15,-15);
//          SoundPlayByNumber (1003,255);
            ClientSyncSendActionAreaSound(1004,500,FALSE);
            break;


            case EFFECT_MISSILE_EARTH_SMITE:
            case EFFECT_MISSILE_LIGHTNING_BOLT:
            ColorAddGlobal (15,15,-15);
//          SoundPlayByNumber (1003,255);
            ClientSyncSendActionAreaSound(1005,500,FALSE);
            break;

            case EFFECT_MISSILE_LOCK_DOOR:
            case EFFECT_MISSILE_UNLOCK_DOOR:
            ColorAddGlobal (-15,-15,-15);
//          SoundPlayByNumber (1004,255);
            ClientSyncSendActionAreaSound(1006,500,FALSE);
            break;

            case EFFECT_MISSILE_PUSH:
            case EFFECT_MISSILE_PULL:
            ColorAddGlobal (15,0,15);
//            SoundPlayByNumber (1005,255);
            ClientSyncSendActionAreaSound(1007,500,FALSE);
            break;

            case EFFECT_MISSILE_EARTHBIND:
            case EFFECT_MISSILE_POISON:
            ColorAddGlobal (0,15,0);
//            SoundPlayByNumber (1006,255);
            ClientSyncSendActionAreaSound(1008,500,FALSE);
            break;

            case EFFECT_MISSILE_CONFUSION:
            case EFFECT_MISSILE_BERSERK:
            ColorAddGlobal (15,20,-15);
            ClientSyncSendActionAreaSound(1009,500,FALSE);
            break;


            case EFFECT_MISSILE_SLOW:
            case EFFECT_MISSILE_PARALYZE:
            ColorAddGlobal (15,20,-15);
//            SoundPlayByNumber (1001,255);
            ClientSyncSendActionAreaSound(1010,500,FALSE);
            break;
            default:
            break;
        }
        break;

        case EFFECT_SET_PLAYER_BEACON:
        break;

        case EFFECT_RETURN_TO_PLAYER_BEACON:
        break;

        case EFFECT_ADD_RUNE:
        SpellsSetRune ((E_spellsRuneType)subtype);
        break;

        case EFFECT_REMOVE_RUNE:
        SpellsClearRune ((E_spellsRuneType)subtype);
        break;

        case EFFECT_MOD_PLAYER_HEALTH_RANDOM:
        StatsChangePlayerHealth ((T_sword16)power + (T_sword16) rand()%power);
        break;

        case EFFECT_MOD_PLAYER_MANA_RANDOM:
        StatsChangePlayerMana ((T_sword16)power + (T_sword16) rand()%power);
        break;

        case EFFECT_MOD_PLAYER_POISON_LEVEL:
        StatsChangePlayerPoisonLevel ((T_sbyte8)power);
        if ((T_sbyte8)power <0)
        break;

        case EFFECT_HALVE_PLAYER_POISON_LEVEL:
        if (StatsGetPlayerPoisonLevel() > 0)  {
            /* Removing poison, but not all */
            MessageAdd ("^009The poison in your body is less.");
        }
        StatsChangePlayerPoisonLevel (-(StatsGetPlayerPoisonLevel()>>1));
        /* add color effect and sound */
        ColorAddGlobal (25,25,25);
//        SoundPlayByNumber (2001,255);
        ClientSyncSendActionAreaSound(2001,500,FALSE);
        break;

        case EFFECT_SET_PLAYER_POISON_LEVEL:
        if ((power==0) && (StatsGetPlayerPoisonLevel() > 0))  {
            /* Removing poison. */
            /* Fully clensed. */
            MessageAdd ("^009Your body is cleansed of all poison.");
            ColorAddGlobal (25,25,25);
//          SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
        }
        StatsSetPlayerPoisonLevel ((T_byte8)power);
        /* Only say something if the poison is clensing any */
        break;

        case EFFECT_DISPLAY_CANNED_MESSAGE:
        MessageDisplayMessage ((T_word16)subtype);
        break;


        case EFFECT_DESTROY_RANDOM_EQUIPPED_ITEM:
        if (InventoryDestroyRandomEquippedItem() != 0)
        {
            ColorAddGlobal (60,60,60);
//          SoundPlayByNumber (4012,255);
            ClientSyncSendActionAreaSound(SOUND_SOMETHING_BREAKING,500,FALSE);
            MessageAdd ("^003You feel like something is missing");
        }
        break;

        case EFFECT_DESTROY_RANDOM_STORED_ITEM:
        if (InventoryDestroyRandomStoredItem() != 0)
        {
            /* put up a message */
            if (rand()%2==1)
               MessageAdd ("^003There is an acrid smell coming from your pack.");
            else
               MessageAdd ("^003You hear something shatter in your pack.");

            /* play a sfx */
//            SoundPlayByNumber (4013,255);
            ClientSyncSendActionAreaSound(SOUND_SOMETHING_SHATTERING,500,FALSE);
        }
        break;

        case EFFECT_DESTROY_RANDOM_ITEM:
        if (rand()%2 == 1)
        {
            if (InventoryDestroyRandomEquippedItem() != 0)
            {
                ColorAddGlobal (60,60,60);
                /* play a sfx */
//                SoundPlayByNumber (4013,255);
                ClientSyncSendActionAreaSound(SOUND_SOMETHING_BREAKING,500,FALSE);
                MessageAdd ("^003You are blinded by a flash of light!");
            }
        }
        else
        {
            if (InventoryDestroyRandomStoredItem() != 0)
            {
                MessageAdd ("^003Your pack feels lighter for some reason");
                /* play a sfx */
                ClientSyncSendActionAreaSound(SOUND_SOMETHING_SHATTERING,500,FALSE);
//              SoundPlayByNumber (4013,255);
            }
        }
        break;

        case EFFECT_TRIGGER_EARTHQUAKE:
        MessageAdd ("^003The ground shakes around you!!");
        ViewEarthquakeOn ((T_word32)duration);
        break;

        case EFFECT_REMOVE_RANDOM_SPELL:
        MessageAdd ("^003The air crackles around you!");
        ClientSyncSendActionAreaSound(2001,500,FALSE);
        EffectRemoveRandomPlayerEffect();
        break;


        case EFFECT_TAKE_NORMAL_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_NORMAL,(T_word16)power);
        break;

        case EFFECT_TAKE_FIRE_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_FIRE,(T_word16)power);
        break;

        case EFFECT_TAKE_ACID_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_ACID,(T_word16)power);
        break;

        case EFFECT_TAKE_POISON_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_POISON,(T_word16)power);
        break;

        case EFFECT_TAKE_ELECTRICITY_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_ELECTRICITY,(T_word16)power);
        break;

        case EFFECT_TAKE_PIERCING_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_PIERCING,(T_word16)power);
        break;

        case EFFECT_TAKE_MANA_DRAIN_DAMAGE:
        StatsTakeDamage (EFFECT_DAMAGE_MANA_DRAIN,(T_word16)power);
        break;

        case EFFECT_TAKE_MULTIPLE_DAMAGE:
        StatsTakeDamage ((T_word16)subtype,(T_word16)power);
        break;

        case EFFECT_PLAY_AREA_SOUND:
//        printf ("playing area sound #=%d radius=%d x=%d y=%d\n",
//                data1,
//                data2,
//                PlayerGetX16(),
//                PlayerGetY16());
//        fflush (stdout);

          /* doesn't work as expected? */
        ClientCreateGlobalAreaSound (PlayerGetX16(),
                                     PlayerGetY16(),
                                     data2, // around 500
                                     data1);
        break;

        case EFFECT_PLAY_LOCAL_SOUND:
        if (G_effectSoundOn)
        {
            SoundPlayByNumber (data1,data2);
        }
        break;

        case EFFECT_FIRE_WAND:
        /* check to see if we have enough mana */
        if (StatsGetPlayerMana()>data3)
        {
            /* subtract the mana */
            StatsChangePlayerMana (-data3);
            /* create the projectile effect */
            Effect (EFFECT_CREATE_PROJECTILE,
                    EFFECT_TRIGGER_USE,
                    data1,
                    data2,
                    data2,
                    0);
//           ClientCreateProjectile(data1, data2, data2);
        }
        else
        {
            /* fail, subtract some mana */
            StatsChangePlayerMana(-data3>>1);
            MessageAdd ("^014Nothing happens when you activate the wand.");
        }
        break;

        case EFFECT_FIRE_BOLT:
        bolttype=BannerGetSelectedAmmoType();
        /* check to see if we have any bolts to fire */
        if (bolttype < BOLT_TYPE_UNKNOWN)
        {
            if (StatsGetPlayerBolts(bolttype)>0)
            {
                /* we have a bolt */
                /* remove one of this type */
                if (StatsAddBolt (bolttype,-1))
                {
                    Effect (EFFECT_CREATE_PROJECTILE,
                            EFFECT_TRIGGER_USE,
                            bolttype+EFFECT_BOLT_NORMAL,
                            data3,
                            data3,
                            0);
                }
            }
        }
        break;

        case EFFECT_COLOR_FLASH:
        ColorAddGlobal ((T_sbyte8)data1,(T_sbyte8)data2,(T_sbyte8)data3);
        break;

        case EFFECT_JUMP_FORWARD:
        PlayerJumpForward(data3) ;
        ColorAddGlobal (0,0,20);
//        SoundPlayByNumber (2004,255);
        ClientSyncSendActionAreaSound(2004,500,FALSE);
        break ;

        case EFFECT_ADD_JOURNAL_PAGE_BY_OBJECT:
        p_object=(T_3dObject *)p_owner;
        if (StatsPlayerHasNotePage(ObjectGetAccData(p_object))==FALSE)
        {
            StatsAddPlayerNotePage(ObjectGetAccData(p_object));
            MessageAdd ("^009You found a journal entry");
        }
        else
        {
            StatsAddPlayerNotePage(ObjectGetAccData(p_object));
            MessageAdd ("^007You already had a copy of this note.");
        }

        break;

        case EFFECT_ADD_JOURNAL_PAGE:
        p_object=(T_3dObject *)p_owner;
        StatsAddPlayerNotePage(data3);
        break;

        case EFFECT_CREATE_OBJECT:
        if (!InventoryObjectIsInMouseHand())
        {
            p_obj = ObjectCreateFake() ;
            ObjectSetType(p_obj, power) ;
            ObjectSetAccData(p_obj, subtype) ;
            p_inv=InventoryTakeObject(INVENTORY_PLAYER,p_obj) ;
            InventorySetMouseHandPointer(p_inv->elementID);
            ControlSetObjectPointer (p_inv->object);
//            StatsChangePlayerLoad(ObjectGetWeight(p_obj)) ;
            ColorAddGlobal (20,10,5);
//            SoundPlayByNumber (2010,255);
            ClientSyncSendActionAreaSound(2010,500,FALSE);

        }
        else
        {
            MessageAdd("Can't create while item is in hand.") ;
        }
        break;

        case EFFECT_IDENTIFY_READIED:
        ColorAddGlobal (0,0,20);
        ClientSyncSendActionAreaSound(2001,500,FALSE);
        InventoryIdentifyReadied();
        MessageAdd ("^009You understand your equipment better!");
        break;

        case EFFECT_IDENTIFY_ALL:
        ColorAddGlobal (0,0,30);
        ClientSyncSendActionAreaSound(2001,500,FALSE);
        InventoryIdentifyAll();
        MessageAdd ("^009You understand your inventory better!");
        break;

        case EFFECT_ACTIVATE_THIEVING:
        if (StatsGetPlayerClassType() == CLASS_ROGUE ||
            StatsGetPlayerClassType() == CLASS_MAGICIAN ||
            StatsGetPlayerClassType() == CLASS_MERCENARY)
        {
            /* stealing or door opening code here */
            /* note: check for CLASS_MERCENARY when determining whether */
            /* the player can unlock a door. */
            /* Mercenaries can only pick pockets. */
            ClientDoSteal() ;
        }
        else
        {
            MessageAdd ("^005Your class doesn't have any thieving abilities.");
        }
        break;

        case EFFECT_MOD_PLAYER_WEALTH:
        if (p_owner!=NULL)
        {
            p_object=(T_3dObject *)p_owner;
            if (ObjectGetAccData(p_object)==0)
            {
                StatsAddCoin (COIN_TYPE_GOLD,10);
            }
            else
            {
                StatsAddCoin (COIN_TYPE_GOLD,ObjectGetAccData(p_object));
            }
        }
        else
        {
            StatsAddCoin (data1,data2);
        }
        break;

        default:
        retvalue=FALSE;
        break;
    }

    DebugEnd();
    return (retvalue);
}



/*-------------------------------------------------------------------------*
 * Routine:  EffectPlayerEffectIsActive
 *-------------------------------------------------------------------------*/
/**
 *  Returns true if the player effect specified is currently on
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean EffectPlayerEffectIsActive (E_playerEffectType type)
{
    E_Boolean retvalue=FALSE;
    DebugRoutine ("EffectPlayerEffectIsActive");
    DebugCheck (type < PLAYER_EFFECT_UNKNOWN);

    if (G_playerEffectPowers[type]>0) retvalue=TRUE;

    DebugEnd();
    return (retvalue);
}


/*-------------------------------------------------------------------------*
 * Routine:  EffectGetPlayerEffectPower
 *-------------------------------------------------------------------------*/
/**
 *  Returns a T_word16 'power level' for an effect in progress.
 *  Returns > 0 if effect is active (retvalue= combined power levels)
 *  Returns 0 if effect not found
 *
 *<!-----------------------------------------------------------------------*/
T_word16 EffectGetPlayerEffectPower (E_playerEffectType type)
{
    T_word16 retvalue;
    DebugRoutine ("EffectGetPlayerEffectPower");
    DebugCheck (type < PLAYER_EFFECT_UNKNOWN);

    retvalue=G_playerEffectPowers[type];

    DebugEnd();
    return (retvalue);
}


/*-------------------------------------------------------------------------*
 * Routine:  EffectUpdateAllPlayerEffects
 *-------------------------------------------------------------------------*/
/**
 *  this routine will search through the current linked list of player
 *  effects and update the duration. Also will call EffectRemovePlayerEffect
 *  if the current duration falls below zero.
 *
 *<!-----------------------------------------------------------------------*/
T_void EffectUpdateAllPlayerEffects (T_void)
{
    T_doubleLinkListElement element,nextelement;
    T_playerEffectStruct *p_effect;
    T_word32 time_elapsed;
    static T_word32 last_update=0;

    DebugRoutine ("EffectUpdateAllPlayerEffects");

    if (!ClientIsPaused())
    {
        /* get time elapsed */
        time_elapsed = TickerGet()-last_update;
        last_update= TickerGet();

        /* cycle through all effects and check durations */
        element=DoubleLinkListGetFirst (G_playerEffectsList);

        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            nextelement=DoubleLinkListElementGetNext(element);

            /* get structure for this element */
            p_effect = (T_playerEffectStruct *)DoubleLinkListElementGetData(element);

            /* check duration (0 here is permanent, > 0 is timed ) */
            /* check to see if we should remove this element */
            if (p_effect->duration > 0)
            {
                /* double duration if god mode */
                if (EffectPlayerEffectIsActive (PLAYER_EFFECT_GOD_MODE))
                {
                    time_elapsed=time_elapsed>>1;
                    if (time_elapsed < 1) time_elapsed=1;
                }

                p_effect->duration -= time_elapsed;
                if (p_effect->duration > MAX_EFFECT_DURATION || p_effect->duration==0)
                {
    //                printf ("Time expired for effect %d\n",p_effect->p_owner);
    //                fflush (stdout);
                    /* wrapped around, turn off spell */
                    EffectRemovePlayerEffect (p_effect->p_owner);
                }
            }

            /* get next element */
            element=nextelement;

        }
    }
    else
    {
        last_update=TickerGet();
    }

    DebugEnd();
}


/* routine displays all in effect icons */
T_void EffectDrawEffectIcons (T_word16 left,
                              T_word16 right,
                              T_word16 top,
                              T_word16 bottom)
{
    T_doubleLinkListElement element;
    T_playerEffectStruct *p_effect;
	T_byte8 *p_pic ;
    T_bitmap *p_bitmap;
	T_resource res ;
    T_word16 time;
    T_sword16 x,y;

    DebugRoutine ("EffectDrawEffectIcons");

    x=right-26;
    y=bottom-22;

    /* cycle through all effects and draw icons */
    element=DoubleLinkListGetFirst (G_playerEffectsList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* get structure for this element */
        p_effect = (T_playerEffectStruct *)DoubleLinkListElementGetData(element);

        /* draw 'in effect icon' if available */
        if (p_effect->p_effectpic != RESOURCE_BAD)
        {
	        res = p_effect->p_effectpic;
            p_pic = PictureLockQuick (res);
            p_bitmap=PictureToBitmap(p_pic);

//            printf ("l=%d r=%d t=%d b=%d\n",left,right,top,bottom);
//            printf ("drawing @ %d, %d\n",x,y);
//            fflush (stdout);


            /* duration < 700 ticks, flash icon */
            if (p_effect->duration < 700)
            {
                time=(700-p_effect->duration)/10;

                if (p_effect->duration % 55 > time )
                {
                    GrDrawShadedAndMaskedBitmap (p_bitmap, x+3, y+2, 0);
                    GrDrawBitmapMasked(p_bitmap, x, y) ;
                }
                else
                {
                    GrDrawShadedAndMaskedBitmap (p_bitmap, x+3, y+2, 0);
                    GrDrawShadedAndMaskedBitmap (p_bitmap, x, y, 100);
                }
            }
            else
            {
                GrDrawShadedAndMaskedBitmap (p_bitmap, x+3, y+2, 0);
                GrDrawBitmapMasked(p_bitmap, x, y) ;
            }
            y-=21;
            if (y<top)
            {
                y=bottom-22;
                x-=25;
            }
		    PictureUnlock(res) ;
        }
        element=DoubleLinkListElementGetNext(element);
    }
    DebugEnd();
}

/* internal routine: starts a player effect with ID p_owner */
/* if ID p_owner exits the effect will be 'restarted' with a new duration */
static T_void EffectAddPlayerEffect (T_word16 data1,
                                     T_word16 data2,
                                     T_word16 data3,
                                     T_void*  p_owner)
{
    T_word16 size;
    T_word16 type,duration,power;
    T_byte8 stmp[48];
    T_playerEffectStruct *p_effect;

    DebugRoutine ("EffectAddPlayerEffect");
    DebugCheck (p_owner != NULL);
    DebugCheck (data1 <= PLAYER_EFFECT_UNKNOWN);

    type=data1;
    duration=data2;
    power=data3;

    DebugCheck (duration < MAX_EFFECT_DURATION);
//    DebugCheck (power < MAX_EFFECT_POWER);

    if (duration > MAX_EFFECT_DURATION) duration = MAX_EFFECT_DURATION;
//  if (power > MAX_EFFECT_POWER) power = MAX_EFFECT_POWER;

    /* destroy any effect with p_owner is in progress */
    EffectRemovePlayerEffect (p_owner);

    /* make a new effect */
    size=sizeof(T_playerEffectStruct);
    p_effect=(T_playerEffectStruct *)MemAlloc(size);

    p_effect->type=(E_playerEffectType)type;
    p_effect->duration=duration;
    /* 1 is 'minimum' power */
    if (power==0) power=1;
    p_effect->power=power;
    p_effect->p_owner=p_owner;

 //   sprintf (stmp,"UI/EFXICONS/ICN%05d",p_effect->type);
    sprintf (stmp,"UI/EFICONS/ICO%05d",p_effect->type-1);
    if (PictureExist(stmp))
    {
        /* lock in spell in effect icon if available */
        p_effect->p_effectpic = PictureFind (stmp);
    } else
    {
//        printf ("effect icon not found [%s]\n",stmp);
//        fflush (stdout);
        p_effect->p_effectpic = RESOURCE_BAD;
    }

    /* add effect to linked list */
    p_effect->p_element = DoubleLinkListAddElementAtEnd(G_playerEffectsList,p_effect);
    DebugCheck (p_effect->p_element != NULL);

    /* add 'power' level to power list */
    G_playerEffectPowers[p_effect->type] += p_effect->power;

//    printf ("player effect started:\n");
//    printf ("type = %d\n",data1);
//    printf ("dur  = %d\n",data2);
//    printf ("pow  = %d\n",data3);
//    printf ("own  = %d\n",p_owner);

//    fflush (stdout);

    /* start the player effect */
    EffectStartPlayerEffect (p_effect);

    DebugEnd();
}


/* internal routine: ends a player effect ID p_owner */
static T_void EffectRemovePlayerEffect (T_void *p_owner)
{
    T_playerEffectStruct *p_effect;
    T_doubleLinkListElement element;

    DebugRoutine ("EffectRemovePlayerEffect");
    DebugCheck (p_owner != NULL);

    element=DoubleLinkListGetFirst (G_playerEffectsList);

    /* traverse through list and find this 'p_owner' */
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_effect=DoubleLinkListElementGetData (element);
        DebugCheck (p_effect != NULL);
        if (p_effect->p_owner == p_owner)
        {
//            printf ("removing effect %d\n",p_owner);
//            fflush (stdout);
            /* found it, break */
            break;
        }

        /* get next element */
        element=DoubleLinkListElementGetNext(element);
    }

    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        EffectStopPlayerEffect(p_effect);

        /* remove this element from the list */
        EffectDestroyElement (element);
    }

    DebugEnd();
}



/* internal routine, constructs effects caused by a player effect */
static T_void EffectStartPlayerEffect (T_playerEffectStruct *p_effect)
{
    T_word16 type,duration,power;

    DebugRoutine ("EffectStartPlayerEffect");
    DebugCheck (p_effect != NULL);
    /* do effect 'startup' procedure here */
    type=p_effect->type;
    duration=p_effect->duration;
    power=p_effect->power;

    switch (type)
    {
        case PLAYER_EFFECT_INVULNERABLE:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel invulnerable!!!");
            /* play a sfx */
//          SoundPlayByNumber (2006,255);
            ClientSyncSendActionAreaSound(2016,500,FALSE);
            ColorAddGlobal (20,20,20);
        }
        break;

        case PLAYER_EFFECT_FLY:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel as light as the air around you!");
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2015,500,FALSE);
        }
        break;

        case PLAYER_EFFECT_LOW_GRAVITY:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel less weighty.");
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2008,255);
            ClientSyncSendActionAreaSound(2008,500,FALSE);
        }

        case PLAYER_EFFECT_LAVA_WALK:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009Your feet feel cold for some reason.");
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2009,255);
            ClientSyncSendActionAreaSound(2009,500,FALSE);
        }
        break;

        case PLAYER_EFFECT_WATER_WALK:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009Your feet feel warm and tingly.");
            ClientSyncSendActionAreaSound(2010,500,FALSE);
//            SoundPlayByNumber (2010,255);
            ColorAddGlobal (20,0,0);
        }
        break;

        case PLAYER_EFFECT_FEATHER_FALL:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel light headed.");
            ClientSyncSendActionAreaSound(2011,500,FALSE);
//            SoundPlayByNumber (2011,255);
            ColorAddGlobal (20,20,0);
        }
        break;

        case PLAYER_EFFECT_STICKY_FEET:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (20,20,20);
            ClientSyncSendActionAreaSound(2012,500,FALSE);
            MessageAdd ("^009Your feet are sticking to the ground!");
        }
        break;

        case PLAYER_EFFECT_SHOCK_ABSORB:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (20,0,-20);
            MessageAdd ("^009You lose your fear of heights.");
            ClientSyncSendActionAreaSound(2014,500,FALSE);
        }
        break;

        case PLAYER_EFFECT_SHIELD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009You feel protected!");
            ColorAddGlobal (20,20,-20);
//            SoundPlayByNumber (2012,255);
            ClientSyncSendActionAreaSound(2014,500,FALSE);
        }
        StatsCalcAverageArmorValue();
        break;

        case PLAYER_EFFECT_RESIST_POISON:
        case PLAYER_EFFECT_RESIST_FIRE:
        case PLAYER_EFFECT_RESIST_ACID:
        case PLAYER_EFFECT_RESIST_PIERCING:
        case PLAYER_EFFECT_RESIST_MANA_DRAIN:
        case PLAYER_EFFECT_RESIST_ELECTRICITY:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel protected.");
//            SoundPlayByNumber (2000,255);
            ClientSyncSendActionAreaSound(2015,500,FALSE);
            ColorAddGlobal (20,20,20);
        }
        break;


        case PLAYER_EFFECT_STRENGTH_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009You feel a surge of strength!");
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
            ColorAddGlobal (20,0,20);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_STRENGTH,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_ACCURACY_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009Your senses sharpen and things come into focus.");
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
            ColorAddGlobal (20,0,20);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_ACCURACY,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_SPEED_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009The world seems to be moving more slowly about you.");
            ColorAddGlobal (20,0,20);
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_SPEED,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_CONSTITUTION_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009You feel hardier!");
            ColorAddGlobal (20,0,20);
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_CONSTITUTION,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_MAGIC_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009Your mind clears and you feel more focused");
            ColorAddGlobal (20,0,20);
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_MAGIC,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_STEALTH_MOD:
        if (G_effectSoundOn)
        {
            if (power > 0) MessageAdd ("^009You feel like you are being watched");
//            SoundPlayByNumber (2001,255);
            ClientSyncSendActionAreaSound(2001,500,FALSE);
            ColorAddGlobal (20,0,20);
        }
        StatsChangePlayerAttributeMod (ATTRIBUTE_STEALTH,(T_sbyte8)power);
        break;

        case PLAYER_EFFECT_HEALTH_REGEN_MOD:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009A wonderful feeling courses through your body.");
//            SoundPlayByNumber (2002,255);
            ClientSyncSendActionAreaSound(2002,500,FALSE);
            ColorAddGlobal (20,0,20);
        }
        StatsChangePlayerHealthRegen ((T_sword16)power);
        break;

        case PLAYER_EFFECT_MANA_REGEN_MOD:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel a surge of power!");
//            SoundPlayByNumber (2002,255);
            ClientSyncSendActionAreaSound(2002,500,FALSE);
            ColorAddGlobal (20,0,20);
        }
        StatsChangePlayerManaRegen ((T_sword16)power);
        break;

        case PLAYER_EFFECT_TRANSLUCENT:
        case PLAYER_EFFECT_INVISIBLE:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009You feel airy.");
            ColorAddGlobal (-20,-20,-20);
            ClientSyncSendActionAreaSound(2004,500,FALSE);
        }
        break;

        case PLAYER_EFFECT_NIGHT_VISION:
        ViewSetDarkSight ((T_byte8)power);
        if (G_effectSoundOn)
        {
//            SoundPlayByNumber (2005,255);
            ClientSyncSendActionAreaSound(2005,500,FALSE);
            ColorAddGlobal (-30,-30,-30);
        }
        break;

        case PLAYER_EFFECT_MAGIC_MAP:
        if (G_effectSoundOn)
        {
//            SoundPlayByNumber (2004,255);
            ClientSyncSendActionAreaSound(2004,500,FALSE);
            ColorAddGlobal (20,20,20);
        }
        if (power==1)
        {
            /* visible walls only */
            OverheadOn();
            OverheadSetFeatures (OVERHEAD_FEATURE_ROTATE_VIEW |
                                 OVERHEAD_FEATURE_TRANSLUCENT);

        }
        else if (power==2)
        {
            /* visible walls objects and creatures*/
            OverheadOn();
            OverheadSetFeatures (OVERHEAD_FEATURE_ROTATE_VIEW |
                                 OVERHEAD_FEATURE_TRANSLUCENT |
                                 OVERHEAD_FEATURE_OBJECTS |
                                 OVERHEAD_FEATURE_CREATURES);

        }
        else if (power==3)
        {
            /* maps the entire map, walls */
            OverheadOn();
            OverheadSetFeatures (OVERHEAD_FEATURE_ROTATE_VIEW |
                                 OVERHEAD_FEATURE_TRANSLUCENT |
                                 OVERHEAD_FEATURE_ALL_WALLS);
        }
        else if (power==4)
        {
            /* super map, map everything */
            OverheadOn();
            OverheadSetFeatures (OVERHEAD_FEATURE_ROTATE_VIEW |
                                 OVERHEAD_FEATURE_TRANSLUCENT |
                                 OVERHEAD_FEATURE_ALL_WALLS |
                                 OVERHEAD_FEATURE_CREATURES |
                                 OVERHEAD_FEATURE_OBJECTS);
        }
        break;

        case PLAYER_EFFECT_MOD_JUMP_POWER:
        if (G_effectSoundOn)
        {
            MessageAdd ("^009Your legs feel much stronger!");
            ColorAddGlobal (20,-20,20);
            ClientSyncSendActionAreaSound(2005,500,FALSE);
//          SoundPlayByNumber (2005,255);
        }
        StatsModJumpPower((T_sword16)power);
        break;

        case PLAYER_EFFECT_DISABLE_PLAYER_EFFECT:
# ifdef NOT_IMPLEMENTED_YET
        DebugCheck (power < PLAYER_EFFECT_UNKNOWN));
        /* increment disable flag */
        G_playerEffectDisableFlags[power]++;
        /* iterate through all player effects, looking for effects */
        /* of (power) type in progress, and stop the effect caused by them */
        element=DoubleLinkListGetFirst (G_playerEffectsList);
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            /* get a pointer to this structure */
            p_effect=(T_playerEffectStruct *)DoubleLinkListElementGetData(element);
            /* check for matching types */
            if (p_effect->type==power)
            {
                /* same type specified, temporarily remove this effect */
                EffectStopPlayerEffect(p_effect);
            }
            element=DoubleLinkListElementGetNext(element);
        }
# endif
        break;

        case PLAYER_EFFECT_FOOD_CONSERVATION:
        case PLAYER_EFFECT_WATER_CONSERVATION:
        case PLAYER_EFFECT_ETERNAL_NOURISHMENT:
        if (G_effectSoundOn)
        {
//            SoundPlayByNumber (2006,255);
            ClientSyncSendActionAreaSound(2006,500,FALSE);
            ColorAddGlobal (0,20,0);
            MessageAdd ("^009You feel sustained.");
        }
        break;

        case PLAYER_EFFECT_POISON_ATTACK:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            MessageAdd ("^009A green mist begins to emanate from your hands.");
        }
        break;

        case PLAYER_EFFECT_ELECTRICITY_ATTACK:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            MessageAdd ("^009A yellow aura surrounds your hands.");
        }
        break;

        case PLAYER_EFFECT_PIERCING_ATTACK:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            MessageAdd ("^009A blue aura envelops your hands.");
        }
        break;

        case PLAYER_EFFECT_MANA_DRAIN_ATTACK:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            MessageAdd ("^009A purple liquid drips from your fingertips.");
        }
        break;

        case PLAYER_EFFECT_FIRE_ATTACK:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            MessageAdd ("^009Your hands begin to glow red hot!!");
        }
        break;

        case PLAYER_EFFECT_ACID_ATTACK:
        if (G_effectSoundOn)
        {
//            SoundPlayByNumber (2007,255);
            ClientSyncSendActionAreaSound(2007,500,FALSE);
            ColorAddGlobal (0,0,20);
            MessageAdd ("^009A white froth bubbles from your palms!");
        }
        break;

        case PLAYER_EFFECT_WATER_BREATHING:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (0,0,20);
//            SoundPlayByNumber (2008,255);
            ClientSyncSendActionAreaSound(2008,500,FALSE);
            MessageAdd ("^009You feel all warm inside.");
        }
        break;

        case PLAYER_EFFECT_DEATH_WARD:
        if (G_effectSoundOn)
        {
            ColorAddGlobal (20,20,20);
            ClientSyncSendActionAreaSound(2009,500,FALSE);
//            SoundPlayByNumber (2009,255);
            MessageAdd ("^009You feel an energy surge!");
        }
        break;

        default:
        break;
    }

    DebugEnd();
}



/* internal routine, destructs effects caused by a player effect */
static T_void EffectStopPlayerEffect (T_playerEffectStruct *p_effect)
{
    T_word16 type,duration,power;

    DebugRoutine ("EffectStopPlayerEffect");
    DebugCheck (p_effect != NULL);

    type=p_effect->type;
    duration=p_effect->duration;
    power=p_effect->power;

    /* do the player effect 'shut down routine' here */
     switch (type)
     {
         case PLAYER_EFFECT_INVULNERABLE:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The feeling of invulnerability passes.");
         break;

         case PLAYER_EFFECT_FLY:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014You feel gravity once again take hold.");
         break;

         case PLAYER_EFFECT_LOW_GRAVITY:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014You feel gravity once again take hold.");
         break;

         case PLAYER_EFFECT_LAVA_WALK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014Your feet no longer feel cold.");
         break;

         case PLAYER_EFFECT_WATER_WALK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014Your feet no longer feel warm.");
         break;

         case PLAYER_EFFECT_FEATHER_FALL:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The feeling of lightheadedness passes.");
         break;

         case PLAYER_EFFECT_STICKY_FEET:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014Your feet no longer stick to everything.");
         break;

         case PLAYER_EFFECT_SHOCK_ABSORB:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014You are once again afraid of high places.");
         break;

         case PLAYER_EFFECT_RESIST_POISON:
         case PLAYER_EFFECT_RESIST_FIRE:
         case PLAYER_EFFECT_RESIST_ACID:
         case PLAYER_EFFECT_RESIST_PIERCING:
         case PLAYER_EFFECT_RESIST_MANA_DRAIN:
         case PLAYER_EFFECT_RESIST_ELECTRICITY:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014You no longer feel protected.");
         break;

         case PLAYER_EFFECT_SHIELD:
         G_playerEffectPowers[type]-=power;
         StatsCalcAverageArmorValue();
         G_playerEffectPowers[type]+=power;
         break;

         case PLAYER_EFFECT_STRENGTH_MOD:
         if (power > 0) MessageAdd ("^014You feel somewhat weaker.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_STRENGTH,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_ACCURACY_MOD:
         if (power > 0) MessageAdd ("^014You feel clumsier.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_ACCURACY,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_SPEED_MOD:
         if (power > 0) MessageAdd ("^014Hmm... everything sped up for some reason.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_SPEED,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_CONSTITUTION_MOD:
         if (power > 0) MessageAdd ("^014You feel somewhat weakened.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_CONSTITUTION,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_MAGIC_MOD:
         if (power > 0) MessageAdd ("^014The feeling of mental acuity passes.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_MAGIC,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_STEALTH_MOD:
         if (power > 0) MessageAdd ("^014The jittery feeling passes.");
         StatsChangePlayerAttributeMod (ATTRIBUTE_STEALTH,-(T_sbyte8)power);
         break;

         case PLAYER_EFFECT_HEALTH_REGEN_MOD:
         if (power > 0) MessageAdd("^014You feel different somehow.");
         StatsChangePlayerHealthRegen (-(T_sword16)power);
         break;

         case PLAYER_EFFECT_MANA_REGEN_MOD:
         if (power > 0) MessageAdd("^014You feel different somehow.");
         StatsChangePlayerManaRegen (-(T_sword16)power);
         break;

         case PLAYER_EFFECT_TRANSLUCENT:
         case PLAYER_EFFECT_INVISIBLE:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The airy feeling passes.");
         break;

         case PLAYER_EFFECT_NIGHT_VISION:
         ViewSetDarkSight (0);
         break;

         case PLAYER_EFFECT_MAGIC_MAP:
         /*only turn off map if this is the last spell */
         if (G_playerEffectPowers[type]==1) OverheadOff();
         break;

         case PLAYER_EFFECT_MOD_JUMP_POWER:
         MessageAdd ("^014Your legs feel weaker.");
         StatsModJumpPower(-(T_sword16)power);
         break;

         case PLAYER_EFFECT_FOOD_CONSERVATION:
         case PLAYER_EFFECT_WATER_CONSERVATION:
         case PLAYER_EFFECT_ETERNAL_NOURISHMENT:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014You no longer feel sustained.");
         break;

         case PLAYER_EFFECT_POISON_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The green mist surrounding your hands fades.");
         break;

         case PLAYER_EFFECT_ELECTRICITY_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The yellow aura dissipates.");
         break;

         case PLAYER_EFFECT_PIERCING_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The blue aura dissipates.");
         break;

         case PLAYER_EFFECT_MANA_DRAIN_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The purple liquid dries up from your fingertips.");
         break;

         case PLAYER_EFFECT_FIRE_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014Your hands stop glowing red.");
         break;

         case PLAYER_EFFECT_ACID_ATTACK:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The white froth on your hands disappears.");
         break;

         case PLAYER_EFFECT_WATER_BREATHING:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The warm feeling subsides.");
         break;

         case PLAYER_EFFECT_DEATH_WARD:
         if (G_playerEffectPowers[type]==1) MessageAdd ("^014The energy surge wears off.");
         break;

         default:
         break;
    }
    DebugEnd();
}



/* internal routine, frees mem used by a player effect element */
static E_Boolean EffectDestroyElement (T_doubleLinkListElement killme)
{
    T_playerEffectStruct *p_effect;
    DebugRoutine ("EffectDestroyElement");

    /* get element data */
    p_effect=(T_playerEffectStruct *)DoubleLinkListRemoveElement (killme);

    /* subtract 'power' from power lookup table */
    G_playerEffectPowers[p_effect->type]-=p_effect->power;

    /* unlock resource for icon pic if necessary */
    if (p_effect->p_effectpic != RESOURCE_BAD)
    {
        PictureUnfind (p_effect->p_effectpic);
    }

    DebugCheck (G_playerEffectPowers[p_effect->type] <= MAX_EFFECT_POWER);

//    if (G_playerEffectPowers[p_effect->type] > MAX_EFFECT_POWER)
//      G_playerEffectPowers[p_effect->type]=0;

    /* kill element data */
    MemFree (p_effect);

    killme=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugEnd();

    return (TRUE);
}


/*-------------------------------------------------------------------------*
 * Routine:  EffectRemoveAllPlayerEffects
 *-------------------------------------------------------------------------*/
/**
 *  Removes ALL player effects in progress.  Note that you will need to
 *  restart equipment effects if you continue to play after a call to this
 *  function.
 *
 *<!-----------------------------------------------------------------------*/
T_void EffectRemoveAllPlayerEffects (T_void)
{
    T_doubleLinkListElement element,nextElement;
    T_playerEffectStruct *p_effect;
    DebugRoutine ("EffectRemoveAllPlayerEffects");

    if (G_playerEffectsList != DOUBLE_LINK_LIST_BAD)
    {
        element=DoubleLinkListGetFirst(G_playerEffectsList);
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            nextElement=DoubleLinkListElementGetNext(element);
            p_effect=DoubleLinkListElementGetData(element);
            EffectStopPlayerEffect(p_effect);
            EffectDestroyElement (element);
            element=nextElement;
        }
        EffectFinish() ;
        EffectInit();
    }

//    G_init=FALSE;

    DebugEnd();
}


/* turns off sound effects for effects */
T_void EffectSoundOff (T_void)
{
    DebugRoutine ("EffectSoundOff");

    G_effectSoundOn=FALSE;

    DebugEnd();
}


/* turns on sound effects for effects */
T_void EffectSoundOn (T_void)
{
    DebugRoutine ("EffectSoundOn");

    G_effectSoundOn=TRUE;

    DebugEnd();
}


E_Boolean EffectSoundIsOn (T_void)
{
    return (G_effectSoundOn);
}

/* removes a random player effect for Dispell Magic effect */
T_void EffectRemoveRandomPlayerEffect (T_void)
{
    T_doubleLinkListElement element;
    T_word16 count=0;
    T_word16 toRemove;
    T_playerEffectStruct *p_effect;

    DebugRoutine ("EffectRemoveRandomPlayerEffect");

    /* remove the equipped player effects so that they don't interfere */
    /* with our choice of spell to remove */
    InventoryRemoveEquippedEffects();

    /* iterate through player effects and count */
    DebugCheck (G_playerEffectsList != DOUBLE_LINK_LIST_BAD);

    element=DoubleLinkListGetFirst (G_playerEffectsList);

    while (element!=DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        count++;
        element=DoubleLinkListElementGetNext(element);
    }

    if (count > 0)
    {
        toRemove=rand()%count;
        element=DoubleLinkListGetFirst(G_playerEffectsList);
        while (toRemove > 0 && element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            element=DoubleLinkListElementGetNext(element);
            toRemove--;
        }
        /* remove this element */
        if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
        {
            p_effect=DoubleLinkListElementGetData(element);
            EffectRemovePlayerEffect(p_effect->p_owner);
        }
    }

    /* restore equipped effects */
    InventoryRestoreEquippedEffects();
    DebugEnd();
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  EFFECT.C
 *-------------------------------------------------------------------------*/
