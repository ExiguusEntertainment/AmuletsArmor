/*-------------------------------------------------------------------------*
 * File:  CRELOGIC.C
 *-------------------------------------------------------------------------*/
/**
 * All Creature AI goes here.  I would love to turn these into scripts.
 *
 * @addtogroup CRELOGIC
 * @brief Creature Logic and AI
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "AREASND.H"
#include "CLIENT.H"
#include "CRELOGIC.H"
#include "DOOR.H"
#include "EFFECT.H"
#include "EFX.H"
#include "EQUIP.H"
#include "GENERAL.H"
#include "MAP.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "OBJGEN.H"
#include "PLAYER.H"
#include "RANDOM.H"
#include "SERVER.H"
#include "STATS.H"
#include "SYNCMEM.H"
#include "SYNCTIME.H"
#include "TICKER.H"
#include "VIEW.H"

#define CRELOGIC_DFT_FLY_ACCELERATION  7     /* Usually same as UPDATE_TIME */
#define CRELOGIC_DFT_FLY_MAX_VELOCITY  20
#define CRELOGIC_DFT_UPDATE_TIME       7
#define CRELOGIC_DFT_STEP_SIZE         15
#define CRELOGIC_DFT_MAX_MELEE_RANGE   60
#define CRELOGIC_DFT_MIN_MISSILE_RANGE 250
#define CRELOGIC_DFT_MAX_MISSILE_RANGE 1000
#define CRELOGIC_DFT_MELEE_DELAY   10
#define CRELOGIC_DFT_MISSILE_DELAY 50
#define CRELOGIC_DFT_HIT_POINTS    2000
#define CRELOGIC_DFT_REGEN_RATE    0
#define CRELOGIC_DFT_MISSILE_TYPE  4011
#define CRELOGIC_DFT_DAMAGE_TYPE   EFFECT_DAMAGE_NORMAL
#define CRELOGIC_DFT_MELEE_DAMAGE  200
#define CRELOGIC_DFT_DAMAGE_RESIST 0
#define CRELOGIC_DFT_HURT_HEALTH   500
#define CRELOGIC_DFT_ARMOR_TYPE    0
#define CRELOGIC_DFT_TREASURE      TREASURE_TYPE_NONE
#define CRELOGIC_DFT_VISION_FIELD  INT_ANGLE_90
#define CRELOGIC_DFT_WANDER_SOUND  4005
#define CRELOGIC_DFT_ATTACK_SOUND  5014
#define CRELOGIC_DFT_ATTACK2_SOUND 5014
#define CRELOGIC_DFT_HURT_SOUND    5013
#define CRELOGIC_DFT_HURT2_SOUND   5013
#define CRELOGIC_DFT_TARGET_SOUND  4001
#define CRELOGIC_DFT_DIE_SOUND     5015
#define CRELOGIC_DFT_FACE_DELAY    6
#define CRELOGIC_DFT_EXPLODE_ON_COLILDE   FALSE
#define CRELOGIC_DFT_CAN_HURT_SELF        FALSE
#define CRELOGIC_DFT_DEATH_LOGIC   0
#define CRELOGIC_DFT_DECAY_OBJECT  0

/* All types except lava */
#define CRELOGIC_DFT_STAY_ON_SECTOR      (SECTOR_TYPE_DIRT| \
                                          SECTOR_TYPE_ICE| \
                                          SECTOR_TYPE_WATER| \
                                          SECTOR_TYPE_MUD)

#define CRELOGIC_DFT_EFX_AT_DEATH       0    // 0 = no effect

#define MAX_TELEPORTER_POSITIONS     20
#define TIME_BETWEEN_TELEPORTS       700
#define TIME_BETWEEN_SUMMONS         350
#define SUMMON_CREATURE_TYPE         1025      // Normal skeleton
#define MAX_CREATURES_FOR_SUMMON     50
#define SUMMON_DISTANCE              75

#define CRELOGIC_TIME_BETWEEN_SECTOR_DAMAGE 35     // Every half second

typedef struct {
    T_word32 lastCreatureUpdateTime ;
    T_doubleLinkList creatureList ;
    E_Boolean init ;
} T_creaturesStruct ;

typedef enum {
    NAV_LOGIC_PKG_NONE,
    NAV_LOGIC_PKG_BERSERKER_A,
    NAV_LOGIC_PKG_BERSERKER_B,
    NAV_LOGIC_PKG_BARBARIAN_GUARD,
    NAV_LOGIC_PKG_BARBARIAN_ARCHER,
    NAV_LOGIC_PKG_NEUTRAL_WANDERER,
    NAV_LOGIC_PKG_SCAREDY_CAT,
    NAV_LOGIC_PKG_ROAMING_GUARD,
    NAV_LOGIC_PKG_COMBO_FIGHTER_ARCHER,
    NAV_LOGIC_PKG_BARBARIAN_ARCHER_B,
    NAV_LOGIC_PKG_DIPPER,
    NAV_LOGIC_PKG_TELEPORTER,
    NAV_LOGIC_PKG_HOMING,
    NAV_LOGIC_PKG_STRAIGHT_LINE,
    NAV_LOGIC_PKG_CLOUD,
    NAV_LOGIC_PKG_APPROACH_AND_SHOOT,
    NAV_LOGIC_PKG_UNKNOWN
} E_navigateLogicPackage ;

typedef enum {
    TARGET_LOGIC_STATE_DO_NOTHING,
    TARGET_LOGIC_STATE_ATTACK_WHILE_IN_RANGE,
    TARGET_LOGIC_STATE_EXPLODE_WHEN_IN_RANGE,
    TARGET_LOGIC_STATE_EXPLODE_ON_COLLISION,
    TARGET_LOGIC_STATE_UNKNOWN
} E_targetLogicState ;

typedef enum {
    TARGET_LOGIC_PACKAGE_NONE,
    TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT,
    TARGET_LOGIC_PACKAGE_SUICIDE_EXPLOSION,
    TARGET_LOGIC_PACKAGE_EXPLODE_ON_COLLISION,
    TARGET_LOGIC_PACKAGE_SCREAM,
    TARGET_LOGIC_PACKAGE_SUMMON_CREATURE,
    TARGET_LOGIC_PACKAGE_CONSTANT_DAMAGE,
    TARGET_LOGIC_PACKAGE_UNKNOWN
} E_targetLogicPackage ;

typedef enum {
    DEATH_LOGIC_NONE,
    DEATH_LOGIC_SINK,
    DEATH_LOGIC_FAST_DISAPPEAR,
    DEATH_LOGIC_NORMAL_DECAY,
    DEATH_LOGIC_UNKNOWN
} E_deathLogic ;

typedef T_void (*T_monsterUpdateCallback)(T_void *p_data) ;

typedef enum {
    TREASURE_TYPE_NONE,
    TREASURE_TYPE_1_PLAT,
    TREASURE_TYPE_1_GOLD,
    TREASURE_TYPE_1_SILVER,
    TREASURE_TYPE_5_COPPER,
    TREASURE_TYPE_SHADOW,
    TREASURE_TYPE_LICH,
    TREASURE_TYPE_EXIGUUS, /* Cemetary wizard */
    TREASURE_TYPE_BAD_KNIGHT,
    TREASURE_TYPE_MITHRIL_BAD_KNIGHT,
    TREASURE_TYPE_EVIL_ORANGE_WIZARD,
    TREASURE_TYPE_BLUE_WIZARD,
    TREASURE_TYPE_DRUID,
    TREASURE_TYPE_POISON_DRUID,
    TREASURE_TYPE_EVIL_KNIGHT,
    TREASURE_TYPE_SILVER_EVIL_KNIGHT,
    TREASURE_TYPE_ELK,
    TREASURE_TYPE_ELF_ARCHER,
    TREASURE_TYPE_VITORIX,
    TREASURE_TYPE_JUGURTHA,
    TREASURE_TYPE_EVIL_ARCHER,
    TREASURE_TYPE_ELYMAS,
    TREASURE_TYPE_MIGHTY_HUNTER,
    TREASURE_TYPE_MATTAN,
    TREASURE_TYPE_MAMMA_DRAGON,
    TREASURE_TYPE_KOA_FOOTMAN,
    TREASURE_TYPE_KOA_HORSEMAN,
    TREASURE_TYPE_BRONZE_EVIL_KNIGHT,
    TREASURE_TYPE_TROJAN,
    TREASURE_TYPE_UNKNOWN
} E_treasureType ;

typedef struct {
    T_word32 objectType ;
    E_navigateLogicPackage navigateLogic ;
    E_targetLogicPackage targetLogic ;
    E_Boolean canFly ;
    E_Boolean ignoreWalls ;
    T_word32 acceleration ;
    T_word32 maxVelocity ;
    T_word32 updateTime ;
    T_word32 stepSize ;
    T_word32 maxMeleeRange ;
    T_word32 minMissileRange ;
    T_word32 maxMissileRange ;
    T_word32 meleeAttackDelay ;
    T_word32 missileAttackDelay ;
    T_word16 hitPoints ;
    T_word16 regenRate ;
//    E_effectMissileType missileType ;
    T_word16 missileType ;
    E_effectDamageType damageType ;
    T_word16 meleeDamage;
    T_monsterUpdateCallback callback ;
    E_effectDamageType damageResist ;
    T_word32 hurtHealth ;
    E_equipArmorTypes armorType ;
    E_treasureType treasureToDrop ;
    T_word32 visionField ;
    T_word32 wanderSound;
    T_word32 attackSound;
    T_word32 attackSound2;
    T_word32 hurtSound;
    T_word32 hurtSound2;
    T_word32 targetSound;
    T_word32 dieSound;
    T_word16 faceDelay ;
    E_Boolean explodeOnCollision ;
    E_Boolean canHurtSelf ;
    E_deathLogic deathLogic ;
    T_word16 decayObject ;
    T_word16 stayOnSectorType ;
    T_byte8 chanceAttackCreature ;
    T_byte8 chanceContinueAttack ;
    T_word16 efxAtDeath ;
    T_byte8 bloodEffect ;
    E_Boolean canOpenDoors ;
} T_creatureLogic ;

/* Include in the data of data that makes up the creature logic. */
#include "credata.h"

#define MAX_CREATURE_MOVES_PER_UPDATE  3
#define SCANB_TURN_RATE                10
#define NUM_UPDATES_DO_BLOCK_MOVEMENT  5

typedef enum {
    SCANB_FACING_LEFT_90,
    SCANB_FACING_LEFT_45,
    SCANB_FACING_MIDDLE,
    SCANB_FACING_RIGHT_45,
    SCANB_FACING_RIGHT_90,
    SCANB_FACING_UNKNOWN
} E_scanBFacing ;

static T_sword16 G_scanBFacingAngleArray[SCANB_FACING_UNKNOWN] = {
                     -INT_ANGLE_90,
                     -INT_ANGLE_45,
                     0,
                     INT_ANGLE_45,
                     INT_ANGLE_90
} ;

typedef enum {
    SCANB_DIR_TURN_LEFT,
    SCANB_DIR_TURN_RIGHT
} E_scanBDir ;

typedef enum {
    BLOCK_TURN_DIR_NONE,
    BLOCK_TURN_DIR_LEFT,
    BLOCK_TURN_DIR_RIGHT,
    BLOCK_TURN_DIR_UNKNOWN
} E_blockTurnDir ;

typedef struct {
    E_Boolean targetAcquired ;
    T_word32 targetID ;
    T_sword16 targetX, targetY, targetZ ;
    T_word32 lastTargetID ;
    T_word32 targetDistance ;
    T_word16 targetAngle ; /* Angle is based on map axis, */
                           /* not creature facing */
    E_Boolean canSeeTarget ;
    T_word16 health ;
    T_creatureLogic *p_logic ;
    T_word16 objectID ;
    T_3dObject *p_obj ;
    T_word32 lastUpdateTime ;
    E_scanBFacing scanBFacing ;
    E_scanBDir scanBDir ;
    T_byte8 scanBCount ;
    E_Boolean moveBlocked ;
    E_blockTurnDir blockTurnDir ;
    T_byte8 blockCount ;
    T_byte8 faceDelayCount ;
    E_Boolean targettingActive ;
    T_word16 meleeDelayCount ;
    T_word16 missileDelayCount ;
    T_word16 poisonLevel ;
    E_Boolean markedForDestroy ;

    /* The following flag is set by IUpdateTarget when the target */
    /* for the creature is at a position in front of the creature. */
    E_Boolean canMeleeHitTarget ;

    /* Flag that tells if the creature is running away. */
    E_Boolean isFleeing ;

    /* Flag telling if the creature is fully dipped into the */
    /* ground / water. */
    E_Boolean isDipped ;

    /* Flag telling if the creature is fully out of the water / ground. */
    E_Boolean isUndipped ;

    /* Note when we died for decaying objects. */
    T_word32 timeOfDeath ;

    /* Flag to say this creature is immune to all damage. */
    E_Boolean immuneToDamage ;

    /* Teleport information (if a teleporter) */
    T_objectGeneratorPosition *p_teleportList ;
    T_word16 lengthTeleportList ;
    T_word16 teleportPosition ;
    T_word32 lastTeleportTime ;

    /* Creature summoning info. */
    T_word32 timeToSummon ;

    /* Creature is slowed? and how long. */
    E_Boolean isSlowed ;
    T_word32 timeSlowEnds ;

    /* Creature is earthbound? and how long. */
    E_Boolean isEarthbound ;
    T_word32 timeEarthboundEnds ;

    /* Last time homing worked. */
    T_word32 timeLastHoming ;

    /* Last time checked for walls. */
    T_word32 lastWallCheck ;

    /* Area sound attached to object. */
    T_areaSound areaSound ;

    /* Time of last wandering sound. */
    T_word32 lastWanderSound ;

    /* Z location of where to hit enemy. */
    T_sword16 targetAttackZ ;

    /* Delayed attack time (in absolute time) */
    T_word32 delayedAttackTime ;

    /* Keep track of the last target's view info. */
    T_lineOfSightLast sight ;

    /* Last location. */
    T_sword16 lastX, lastY ;

    /* Flag stolen items from this guy. */
    E_Boolean wasStolenFrom ;

    /* Flag to say if gravity will make this creature fall. */
    E_Boolean allowFall ;

    /* Time to update berserk state */
    T_word32 timeCheckBerserk ;
} T_creatureState ;

/* Callback routine called each time a creature updates based on the */
/* type of navigation package attached to the creature type. */
typedef T_void (*T_creatureNavigateRoutine)(
                   T_creatureState *p_creature,
                   T_creatureLogic *p_logic,
                   T_3dObject *p_obj) ;

/* Callback routine called when creature's creatureState->targettingActive */
/* is true.  Called only per creature update. */
/* Returns TRUE if the creature was killed by his own doing. */
typedef E_Boolean (*T_creatureTargetRoutine)(
                       T_creatureState *p_creature,
                       T_creatureLogic *p_logic,
                       T_3dObject *p_obj) ;

/* Callback routine called when creature's health is zero. */
/* Returns TRUE when creature is truly dead (no more death throws). */
typedef E_Boolean (*T_creatureDeathRoutine)(
                       T_creatureState *p_creature,
                       T_creatureLogic *p_logic,
                       T_3dObject *p_obj) ;

#define NUM_CREATURE_LOGICS   \
                (sizeof(G_creatureLogics) / sizeof(T_creatureLogic))

/* Internal prototypes: */
static T_creatureLogic *ICreatureLogicFind(T_word16 objectType) ;
static T_doubleLinkListElement ICreatureFindViaObjectPtr(T_3dObject *p_obj) ;

static T_void INavNone(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavBerserkerA(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavBerserkerB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavBarbarianGuard(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavBarbarianArcher(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavBarbarianArcherB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavNeutralWanderer(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavScaredyCat(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavRoamingGaurd(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavComboFighterArcher(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavDipper(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavTeleporter(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavHoming(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavStraightLine(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavCloud(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void INavApproachAndShoot(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

/* States of navigation: */
static T_void IFleeIndirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IFleeDirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IWander(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IWanderAndScan(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IApproachTargetDirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IMoveForwardViaFlying(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  E_Boolean flyAway) ;

static T_void IMoveForwardViaWalking(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IStepForward(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ICreatureTargetLogic(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj) ;

static T_void CreatureDropTreasure(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void IDoEfxAtDeath(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

/* Array of callbacks, one per type of navigation logic. */
static T_creatureNavigateRoutine G_creatureNavigateRoutineArray[] = {
    INavNone,                        /* NAV_LOGIC_PKG_NONE */
    INavBerserkerA,                  /* NAV_LOGIC_PKG_BERSERKER_A */
    INavBerserkerB,                  /* NAV_LOGIC_PKG_BERSERKER_B */
    INavBarbarianGuard,              /* NAV_LOGIC_PKG_BARBARIAN_GUARD */
    INavBarbarianArcher,             /* NAV_LOGIC_PKG_BARBARIAN_ARCHER */
    INavNeutralWanderer,             /* NAV_LOGIC_PKG_NEUTRAL_WANDERER */
    INavScaredyCat,                  /* NAV_LOGIC_PKG_SCAREDY_CAT */
    INavRoamingGaurd,                /* NAV_LOGIC_PKG_ROAMING_GUARD */
    INavComboFighterArcher,          /* NAV_LOGIC_PKG_COMBO_FIGHTER_ARCHER */
    INavBarbarianArcherB,            /* NAV_LOGIC_PKG_BARBARIAN_ARCHER_B */
    INavDipper,                      /* NAV_LOGIC_PKG_DIPPER */
    INavTeleporter,                  /* NAV_LOGIC_PKG_TELEPORTER */
    INavHoming,                      /* NAV_LOGIC_PKG_HOMING */
    INavStraightLine,                /* NAV_LOGIC_PKG_STRAIGHT_LINE */
    INavCloud,                       /* NAV_LOGIC_PKG_CLOUD */
    INavApproachAndShoot,            /* NAV_LOGIC_PKG_APPROACH_AND_SHOOT */
} ;

static E_Boolean ITargetNone(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetStandardHitOrShoot(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetSuicideExplosion(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetExplodeOnCollision(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetScream(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetSummonCreature(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static E_Boolean ITargetConstantDamage(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void IShootAtTarget(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;
static T_void IAttackTargetWithMelee(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_sword16 z) ;
static T_void IExplodeSelf(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_word16 damage) ;
static T_void IConsiderTargetChange(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_word16 proposedNewTargetID) ;
static T_word16 IFindClosestCreature(
                    T_sword16 x,
                    T_sword16 y,
                    T_creatureState *p_exclude) ;

/* Array of callbacks, one per type of target logic package. */
static T_creatureTargetRoutine G_creatureTargetRoutineArray[] = {
    ITargetNone,                     /* TARGET_LOGIC_PACKAGE_NONE */
    ITargetStandardHitOrShoot,       /* TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT */
    ITargetSuicideExplosion,         /* TARGET_LOGIC_PACKAGE_SUICIDE_EXPLOSION */
    ITargetExplodeOnCollision,       /* TARGET_LOGIC_PACKAGE_EXPLODE_ON_COLLISION */
    ITargetScream,                   /* TARGET_LOGIC_PACKAGE_SCREAM */
    ITargetSummonCreature,           /* TARGET_LOGIC_PACKAGE_SUMMON_CREATURE */
    ITargetConstantDamage,           /* TARGET_LOGIC_PACKAGE_CONSTANT_DAMAGE */
} ;

static E_Boolean IDeathNone(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj) ;
static E_Boolean IDeathSink(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj) ;
static E_Boolean IDeathFastDisappear(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj) ;
static E_Boolean IDeathDecay(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj) ;

static T_void ICreatureDelayedAttack(T_creatureState *p_creature) ;

static T_creatureDeathRoutine G_creatureDeathRoutine[] = {
    IDeathNone,                      /* DEATH_LOGIC_NONE */
    IDeathSink,                      /* DEATH_LOGIC_SINK */
    IDeathFastDisappear,             /* DEATH_LOGIC_FAST_DISAPPEAR */
    IDeathDecay,                     /* DEATH_LOGIC_NORMAL_DECAY */
} ;

/* Standard creature scan routine. */
static T_player ICreatureScanA(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void ICreatureScanB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void IUpdateTarget(T_creatureState *p_creature) ;

static T_void IHandleBlockedMove(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void ICreatureDip(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void ICreatureUndip(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

static T_void ICreatureMakeSound(
                  T_3dObject *p_obj,
                  T_word16 soundNum) ;

static T_void CreatureTakeSectorDamage(
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj) ;

/* Global variable definitions. */
static T_word32 G_lastCreatureUpdateTime = 0 ;

static T_doubleLinkList G_creatureList = DOUBLE_LINK_LIST_BAD ;

static E_Boolean G_init = FALSE ;

#ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
static FILE *G_fp ;
#endif

/*-------------------------------------------------------------------------*
 * Routine:  CreaturesInitialize
 *-------------------------------------------------------------------------*/
/**
 *  CreaturesInitialize sets up the list of creatures.
 *
 *<!-----------------------------------------------------------------------*/
T_void CreaturesInitialize(T_void)
{
    DebugRoutine("CreaturesInitialize") ;
    DebugCheck(G_init == FALSE) ;
    DebugCheck(G_creatureList == DOUBLE_LINK_LIST_BAD) ;

    /* Create a list to hold the creatures (or at least reference them) */
    G_init = TRUE ;
    G_creatureList = DoubleLinkListCreate() ;
    G_lastCreatureUpdateTime = 0 ;

    DebugCheck(G_creatureList != DOUBLE_LINK_LIST_BAD) ;

#   ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
    G_fp = fopen("crelogic.dat", "w") ;
#   endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  CreaturesFinish
 *-------------------------------------------------------------------------*/
/**
 *  CreaturesFinish unloads all creature based data.  All objects in the
 *  world should already be destroyed.
 *
 *<!-----------------------------------------------------------------------*/
T_void CreaturesFinish(T_void)
{
    DebugRoutine("CreaturesFinish") ;
    DebugCheck(G_init == TRUE) ;
    DebugCheck(DoubleLinkListGetNumberElements(G_creatureList) == 0) ;

    /* Destroy the list of creatures. */
    DoubleLinkListDestroy(G_creatureList) ;
    G_creatureList = DOUBLE_LINK_LIST_BAD ;

    G_init = FALSE ;

#   ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
    fclose(G_fp) ;
#   endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  CreatureAttachToObject
 *-------------------------------------------------------------------------*/
/**
 *  This routine is called to try to make a creature out of an object
 *  if the object type is that of a creature.  If not, nothing happens.
 *
 *<!-----------------------------------------------------------------------*/
T_void CreatureAttachToObject(T_3dObject *p_obj)
{
    T_creatureState *p_creature ;
    T_creatureLogic *p_logic ;

    DebugRoutine("CreatureAttachToObject") ;
    DebugCheck(p_obj != NULL) ;

#   ifndef COMPILE_OPTION_DONT_LOAD_CREATURES
    if (p_obj)  {
        /* Make sure this object has no data attached yet. */
        DebugCheck(ObjectGetExtraData(p_obj) == NULL) ;

        /* First, see if this is a creature in our database. */
        p_logic = ICreatureLogicFind(ObjectGetType(p_obj)) ;

        /* If no logic is found for the object type, then we */
        /* must not have a creature and the call to this command */
        /* is not needed. */
        if (p_logic != NULL)  {
            /* Each creature requires memory. */
            p_creature = MemAlloc(sizeof(T_creatureState)) ;

            /* If we don't get the memory.  Stop here and say this */
            /* is the last creature. */
            DebugCheck(p_creature != NULL) ;
            if (p_creature != NULL)  {
                /* Clear out the creature data before we start. */
                memset(p_creature, 0, sizeof(T_creatureState)) ;

                /* Make the object use this creature data as its extra */
                /* data. */
                ObjectSetExtraData(p_obj, p_creature) ;

                /* Initialize the default values of the object. */
                p_creature->targetAcquired = FALSE ;
                p_creature->health = p_logic->hitPoints ;
                p_creature->p_logic = p_logic ;
                p_creature->p_obj = p_obj ;
                p_creature->objectID = ObjectGetServerId(p_obj) ;
                p_creature->scanBFacing = SCANB_FACING_MIDDLE ;
                p_creature->markedForDestroy = FALSE ;
                p_creature->isFleeing = FALSE ;
                p_creature->isDipped = FALSE ;
                p_creature->isUndipped = TRUE ;
                p_creature->immuneToDamage = FALSE ;
                p_creature->p_teleportList = NULL ;
                p_creature->lengthTeleportList = 0 ;
                p_creature->teleportPosition = 0xFFFF ;
                p_creature->timeToSummon = 0 ;
                p_creature->lastX = p_creature->lastY = 0x7FFE ;
                p_creature->wasStolenFrom = FALSE ;
                p_creature->allowFall = TRUE ;

                Collide3dUpdateLineOfSightLast(
                    &p_creature->sight,
                    NULL) ;

                /* Mark time of death to be unknown. */
                p_creature->timeOfDeath = 0 ;

                /* Put creature in link list. */
                DoubleLinkListAddElementAtEnd(
                    G_creatureList,
                    p_creature) ;

                /* Make sure the creature uses our Z updates */
                /* instead of the object system. */
                if (!(p_logic->canFly))  {
                    ObjectSetMoveFlags(
                        p_obj,
                        OBJMOVE_FLAG_IGNORE_Z_UPDATES) ;
                    ObjectSetMaxVelocity(p_obj, 50) ;
                } else {
                    /* Set up the maximum velocity for the object. */
                    ObjectSetMaxVelocity(p_obj, p_logic->maxVelocity) ;

                    ObjectSetMoveFlags(
                        p_obj,
                        OBJMOVE_FLAG_IGNORE_GRAVITY) ;
                }
            }
        }
    }
#   endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  CreatureDetachFromObject
 *-------------------------------------------------------------------------*/
/**
 *  This routine is called to destroy creature data on an object.
 *  (making the object no longer a creature but an object).
 *
 *<!-----------------------------------------------------------------------*/
T_void CreatureDetachFromObject(T_3dObject *p_obj)
{
    T_doubleLinkListElement element ;
    T_creatureState *p_creature ;

    DebugRoutine("CreatureDetachFromObject") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(ObjectGetExtraData(p_obj) != NULL) ;

    element = ICreatureFindViaObjectPtr(p_obj) ;
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Yes, creature is here to detach. */
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;

        /* Make sure that if there is any extra teleport data here, to */
        /* free it. */
        if (p_creature->p_teleportList != NULL)
            MemFree(p_creature->p_teleportList) ;

        /* Free the memory and remove the element. */
        MemFree(p_creature) ;
        ObjectSetExtraData(p_obj, NULL) ;
        DoubleLinkListRemoveElement(element) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICreatureLogicFind
 *-------------------------------------------------------------------------*/
/**
 *  ICreatureLogicFind searches the list of logics for creatures.  If
 *  there is a match, a pointer to that logic is returned.
 *
 *<!-----------------------------------------------------------------------*/
static T_creatureLogic *ICreatureLogicFind(T_word16 objectType)
{
    T_creatureLogic *p_logic ;
    T_creatureLogic *p_foundLogic = NULL ;
    T_word16 i ;

    DebugRoutine("ICreatureLogicFind") ;

    for (i=0, p_logic=G_creatureLogics;
         i<NUM_CREATURE_LOGICS;
         i++, p_logic++)  {
        if (p_logic->objectType == objectType)  {
            p_foundLogic = p_logic ;
            break ;
        }
    }

    DebugEnd() ;

    return p_foundLogic ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICreatureFindViaObjectPtr
 *-------------------------------------------------------------------------*/
/**
 *  ICreatureFindViaObjectPtr searches the list of creatures for a
 *  creature that has the given object pointer.
 *
 *<!-----------------------------------------------------------------------*/
static T_doubleLinkListElement ICreatureFindViaObjectPtr(T_3dObject *p_obj)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_creatureState *p_creature ;

    DebugRoutine("ICreatureFindViaObjectPtr") ;

    /* Search until you find a match or find no more elements. */
    element = DoubleLinkListGetFirst(G_creatureList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        nextElement = DoubleLinkListElementGetNext(element) ;

        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        if (p_creature->p_obj == p_obj)
            break ;

        element = nextElement ;
    }

    DebugEnd() ;

    return element ;
}

/*-------------------------------------------------------------------------*
 * Routine:  OutputPlayerJunk
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
#ifndef NDEBUG
T_void OutputPlayerJunk(T_void)
{
    T_3dObject *p_obj ;
    T_word16 basic ;


    DebugRoutine("OutputPlayerJunk") ;

    p_obj = ObjectFind(9000) ;
    if (p_obj)  {
        basic = ObjectGetBasicType(p_obj) ;
        if ((ObjectIsPiecewise(p_obj)) || ((basic >= 700) && (basic < 800)))  {
            printf("Object %d at (%08lX %08lX %08lX)\n",
                ObjectGetServerId(p_obj),
                ObjectGetX(p_obj),
                ObjectGetY(p_obj),
                ObjectGetZ(p_obj)) ;
//ObjectPrint(stdout, p_obj) ;
        }
    }
    p_obj = ObjectFind(9001) ;
    if (p_obj)  {
        basic = ObjectGetBasicType(p_obj) ;
        if ((ObjectIsPiecewise(p_obj)) || ((basic >= 700) && (basic < 800)))  {
            printf("Object %d at (%08lX %08lX %08lX)\n",
                ObjectGetServerId(p_obj),
                ObjectGetX(p_obj),
                ObjectGetY(p_obj),
                ObjectGetZ(p_obj)) ;
//ObjectPrint(stdout, p_obj) ;
        }
    }

    p_obj = ObjectFind(9100) ;
    if (p_obj)  {
        basic = ObjectGetBasicType(p_obj) ;
        if ((ObjectIsPiecewise(p_obj)) || ((basic >= 700) && (basic < 800)))  {
            printf("Object %d at (%08lX %08lX %08lX)\n",
                ObjectGetServerId(p_obj),
                ObjectGetX(p_obj),
                ObjectGetY(p_obj),
                ObjectGetZ(p_obj)) ;
//ObjectPrint(stdout, p_obj) ;
        }
    }
    p_obj = ObjectFind(9101) ;
    if (p_obj)  {
        basic = ObjectGetBasicType(p_obj) ;
        if ((ObjectIsPiecewise(p_obj)) || ((basic >= 700) && (basic < 800)))  {
            printf("Object %d at (%08lX %08lX %08lX)\n",
                ObjectGetServerId(p_obj),
                ObjectGetX(p_obj),
                ObjectGetY(p_obj),
                ObjectGetZ(p_obj)) ;
//ObjectPrint(stdout, p_obj) ;
        }
    }

    DebugEnd() ;
}
#endif

T_void CreaturesUpdate(T_void)
{
    T_creatureLogic *p_logic ;
    T_creatureState *p_creature ;
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_word32 time ;
//    T_word16 updateCount ;
    T_3dObject *p_obj ;
    T_word32 delta ;
    T_word32 oldX, oldY, oldZ ;
    E_Boolean isGone = FALSE ;
    static T_word32 poisonTime = 0 ;
    E_Boolean doPoison = FALSE ;
    static T_word32 sectorDamageTime = 0 ;
    E_Boolean doSectorDamage = FALSE ;
    T_word32 regenValue ;
    T_word32 updateTime ;
    T_word16 damageAmount ;
    T_byte8 damageType ;
#if 0
T_sword32 lx, ly, lz ;
#endif
    TICKER_TIME_ROUTINE_PREPARE() ;

    DebugRoutine("CreaturesUpdate") ;
    TICKER_TIME_ROUTINE_START() ;

#   ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
    fprintf(G_fp, "CreaturesUpdate: t:%d, by %s\n", SyncTimeGet(), DebugGetCallerName()) ; fflush(stdout) ;
//OutputPlayerJunk() ;
#   endif

    /* Always allow dipping. */
//    View3dAllowDip() ;

    /* Walls are creature based. */
    Collide3dSetWallDefinition(
        LINE_IS_IMPASSIBLE |
        LINE_IS_CREATURE_IMPASSIBLE) ;

//puts("\nCreaturesUpdate") ;
    /* Get one time to sync all times. */
    time = SyncTimeGet() ;

    if (G_lastCreatureUpdateTime == 0)  {
        G_lastCreatureUpdateTime = time ;
        poisonTime = time ;
        sectorDamageTime = time ;
    }

    if (poisonTime < time)  {
        /* Update every 5 seconds. */
        poisonTime += 350 ;
        doPoison = TRUE ;
    }

    /* Sector damage time every 1/2 second. */
    if (sectorDamageTime < time)  {
        sectorDamageTime += CRELOGIC_TIME_BETWEEN_SECTOR_DAMAGE ;
        doSectorDamage = TRUE ;
    }

    delta = time - G_lastCreatureUpdateTime ;
    G_lastCreatureUpdateTime = time ;

    if (delta)  {
        for (element = DoubleLinkListGetFirst(G_creatureList);
                (element != DOUBLE_LINK_LIST_ELEMENT_BAD);
                    element = nextElement)  {
            nextElement = DoubleLinkListElementGetNext(element) ;

            /* Get the creature state info. */
            p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;

            /* Check for wall damage if time. */
            if ((time - p_creature->lastWallCheck) > TIME_BETWEEN_DAMAGE_WALL_CHECKS)  {
                p_creature->lastWallCheck = time ;
                if (MapGetWallDamage(
                        p_creature->p_obj,
                        &damageAmount,
                        &damageType))  {
                    /* Take damage. */
                    CreatureTakeDamage(
                        p_creature->p_obj,
                        damageAmount,
                        damageType,
                        0) ;
                }
            }

            if (p_creature->markedForDestroy == TRUE)  {
                /* Make sure the creature dies correctly. */
                if (ObjectGetStance(p_creature->p_obj) != STANCE_DIE)
                    ObjectSetStance(p_creature->p_obj, STANCE_DIE) ;

                /* Make the creature now fall and update like any normal object. */
                p_creature->p_obj->objMove.Flags &=
                     ~(OBJMOVE_FLAG_IGNORE_Z_UPDATES) ;

                /* Check to see if we need to really destroy this object. */
                if (G_creatureDeathRoutine[p_creature->p_logic->deathLogic](
                        p_creature,
                        p_creature->p_logic,
                        p_creature->p_obj) == TRUE)
                    CreatureDetachFromObject(p_creature->p_obj) ;
                /* NOTE:  The call to the DeathLogicRoutine above */
                /* may destroy the creature in hand. */
                /* Consider all references to p_creature past this */
                /* point to be invalid. */
            } else {
                p_logic = p_creature->p_logic ;
                DebugCheck(p_logic != NULL) ;
                p_obj = p_creature->p_obj ;
                DebugCheck(p_obj != NULL) ;
                DebugCheck(ObjectGetServerId(p_obj) != 0) ;
                DebugCheck(ObjectIsCreature(p_obj) == TRUE) ;
#ifndef NDEBUG
                SyncMemAdd("Creature %d at %d %d\n", ObjectGetServerId(p_obj), ObjectGetX16(p_obj), ObjectGetY16(p_obj)) ;
                if (p_creature->targetID != 0)
                    SyncMemAdd("  target %d\n", p_creature->targetID, 0, 0) ;
#endif

                if (p_obj->inWorld == TRUE)  {
                    ObjectSetClimbHeight(p_obj, (ObjectGetHeight(p_obj) >> 1)) ;

                    /* Do delayed attack if time. */
                    if ((p_creature->delayedAttackTime != 0) &&
                        (p_creature->delayedAttackTime <= SyncTimeGet()))  {
                        ICreatureDelayedAttack(p_creature) ;
                        p_creature->delayedAttackTime = 0 ;
                    }

                    /* Do poisoning if it is time. */
                    if (doPoison)  {
                        if (p_creature->poisonLevel > 0)  {
                            CreatureTakeDamage(
                                p_obj,
                                p_creature->poisonLevel*2,
                                EFFECT_DAMAGE_NORMAL,
                                0) ;

                            if (p_creature->poisonLevel > 5)
                                p_creature->poisonLevel -= 5 ;
                            else
                                p_creature->poisonLevel = 0 ;
                        }
                    }

                    /* Do sector damaging. */
                    if (doSectorDamage)
                        CreatureTakeSectorDamage(p_logic, p_obj) ;

                    /* First, update gravity if the creature cannot fly. */
                    if (p_logic->canFly == FALSE) {
                        if (ObjectGetZ(p_obj)
                                > (MapGetWalkingFloorHeight(&p_obj->objMove,
                                        ObjectGetAreaSector(p_obj)) << 16))
                            if (p_creature->allowFall == TRUE)
                                ObjectUpdateZVel(p_obj, delta);
                    } else {
                        if (p_creature->isEarthbound) {
                            if (time > p_creature->timeEarthboundEnds) {
                                /* Stop being earthbound. */
                                p_creature->isEarthbound = FALSE;
                                ObjectSetMoveFlags(
                                        p_obj,
                                        OBJMOVE_FLAG_IGNORE_GRAVITY);
                            }
                        } else {
                            ObjectClearMoveFlags(
                                    p_obj,
                                    OBJMOVE_FLAG_IGNORE_MAX_VELOCITY);
                        }
                    }

                    /* Creature just died.  Nothing else to */
                    /* do here. */
                    if (p_creature->markedForDestroy)
                        continue ;

                    /* Update wander sound (if it has one). */
                    if (p_logic->wanderSound != 0)  {
                        /* Is this a missile? */
                        if (!CreatureIsMissile(p_obj))  {
                            /* This is not a missile. */
                            /* Check if it is time to make wander sound. */
                            if (p_creature->lastWanderSound < SyncTimeGet())  {
                                p_creature->lastWanderSound = SyncTimeGet() + 700 + (RandomValue() % 700) ;
                                ICreatureMakeSound(p_obj, p_logic->wanderSound) ;
                            }
                        } else {
                            /* This is a missile. */
                            /* See if we have an area sound assigned yet. */
                            if (p_creature->areaSound == AREA_SOUND_BAD)  {
                                /* Create an area sound for the missile. */
                                p_creature->areaSound =
                                    AreaSoundCreate(
                                        ObjectGetX16(p_obj),
                                        ObjectGetY16(p_obj),
                                        500,
                                        255,
                                        AREA_SOUND_TYPE_LOOP,
                                        0,
                                        AREA_SOUND_BAD,
                                        NULL,
                                        0,
                                        p_logic->wanderSound) ;
                            } else {
                                /* Update the position of the area sound. */
                                AreaSoundMove(
                                    p_creature->areaSound,
                                    ObjectGetX16(p_obj),
                                    ObjectGetY16(p_obj)) ;
                            }
                        }
                    }

                    /* Make sure the object exists as given in the pointer. */
//                    DebugCheck(p_creature->p_obj == ObjectFind(p_creature->objectID)) ;

                    /* Update the creature's poison level. */

#if 0
                    /* If the last update time is zero, then this is the first time */
                    /* this creature is called to update.  Set the time to the */
                    /* current time. */
                    if (p_creature->lastUpdateTime == 0)  {
                        if (p_logic->updateTime == 0)  {
                            /* Missiles are always immediate */
                            p_creature->lastUpdateTime = 0 ;
                        } else {
                            p_creature->lastUpdateTime =
                                time +
                                (ObjectGetServerId(p_obj) %
                                    (1+p_logic->updateTime)) ;
                        }
                    }
#else
                    if (p_creature->lastUpdateTime == 0)
                        p_creature->lastUpdateTime = time ;
#endif

                    updateTime = p_logic->updateTime ;
/* TESTING */
updateTime += (updateTime>>1) ;
                    if (p_creature->isSlowed)  {
                        if (p_creature->timeSlowEnds < SyncTimeGet())  {
                            p_creature->isSlowed = FALSE ;
                            p_creature->timeSlowEnds = ((T_word32)(-1)) ;
                        } else {
                            updateTime = p_logic->updateTime*3 ;
                        }
                    }

                    /* Fleeing creatures move twice as fast. */
                    if (p_creature->isFleeing)
                        updateTime -= (updateTime>>2) ;

                    /* Check to see if it is time to update this creature. */
                    if (time >= (p_creature->lastUpdateTime + updateTime))  {
                        /* Yes, it is time to update the creature. */
                        /* Update the creature based on which logic package */
                        /* it is using. */
                        DebugCheck(p_logic->navigateLogic < NAV_LOGIC_PKG_UNKNOWN) ;

                        /* Check for creature being squished */
                        if (ObjectIsBeingCrushed(p_creature->p_obj))  {
                            CreatureTakeDamage(
                                p_creature->p_obj,
                                (updateTime)*30,
                                EFFECT_DAMAGE_NORMAL,
                                0);
                            /* Creature just died.  Nothing else to */
                            /* do here. */
                            if (p_creature->markedForDestroy)
                                continue ;
                        }

                        /* Update delay counts. */
                        if (p_creature->missileDelayCount != 0)
                            p_creature->missileDelayCount-- ;
                        if (p_creature->meleeDelayCount != 0)
                            p_creature->meleeDelayCount-- ;

                        /* Record where we were. */
                        oldX = ObjectGetX(p_obj) ;
                        oldY = ObjectGetY(p_obj) ;
                        oldZ = ObjectGetZ(p_obj) ;

                        if (((p_creature->moveBlocked) ||
                             (ObjectWasBlocked(p_obj)) ||
                             (p_creature->blockCount != 0)) &&
                            (!ObjectIsFullyPassable(p_obj))) {
                                /* The creature was blocked.  Handle that. */
                            if (p_logic->explodeOnCollision)  {
                                IExplodeSelf(
                                    p_creature,
                                    p_logic,
                                    p_obj,
                                    p_logic->meleeDamage) ;
                                isGone = TRUE ;
                            } else {
                                IHandleBlockedMove(
                                    p_creature,
                                    p_logic,
                                    p_obj) ;
                            }
                        } else {
                            /* Update the creature's target (if there is one). */
                            if (p_creature->targetAcquired)
                                IUpdateTarget(p_creature) ;

                            p_creature->targettingActive = FALSE ;

                            G_creatureNavigateRoutineArray
                                [p_logic->navigateLogic]
                                    (p_creature, p_logic, p_obj) ;

                            /* Reset the blocked direction to turn. */
                            p_creature->blockTurnDir = BLOCK_TURN_DIR_NONE ;

                            /* NOTE: You must strictly ahere to the use of */
                            /* the isGone flag.  If it is TRUE, this creature */
                            /* all pointers to it are now invalid! */
                            if (p_creature->targettingActive)
                                isGone = ICreatureTargetLogic(
                                             p_creature,
                                             p_logic,
                                             p_obj) ;
                        }

                        if (!isGone)  {
                            /* Step along the updates. */
                            p_creature->lastUpdateTime = time ;

                            /* Regenerate the creature */
                            regenValue = p_creature->health + p_logic->regenRate ;
                            if (regenValue > p_logic->hitPoints)
                                regenValue = p_logic->hitPoints ;
                            p_creature->health = regenValue ;

                            /* Update the stance based on the movement. */
                            /* But only do this if the creature is already in a walking */
                            /* or standing stance. */
                            if ((ObjectGetStance(p_obj) == STANCE_WALK) ||
                                (ObjectGetStance(p_obj) == STANCE_STAND))  {
                                if ((ObjectGetX(p_obj) != oldX) ||
                                    (ObjectGetY(p_obj) != oldY) ||
                                    (ObjectGetZ(p_obj) != oldZ) ||
                                    (p_logic->canFly == TRUE) ||
                                    (ObjectIsFullyPassable(p_obj)))  {
                                    ObjectSetStance(p_obj, STANCE_WALK) ;
                                } else {
                                    ObjectSetStance(p_obj, STANCE_STAND) ;
                                }
                            }
                        } else {
                            /* If the creature is gone, don't try to loop. */
//                            break ;
                        }
                    }
                }

#if 0
if ((ObjectGetX(p_obj)!=lx) ||
    (ly != ObjectGetY(p_obj)) ||
    (lz != ObjectGetZ(p_obj)))  {
  printf("Creature %d (%d) was at (%08X %08X %08X), angle: %04X\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj), lx, ly, lz, ObjectGetAngle(p_obj)) ;
  printf("Creature %d (%d) now at (%08X %08X %08X), angle: %04X\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj), ObjectGetX(p_obj), ObjectGetY(p_obj), ObjectGetZ(p_obj), ObjectGetAngle(p_obj)) ;
}
#endif
            }
        }
    }

    TICKER_TIME_ROUTINE_ENDM("CreaturesUpdate", 500) ;

    DebugEnd() ;
}

static T_void INavBerserkerA(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavBerserkerA") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        /* Yes, do we need to approach? */
        if (p_creature->canMeleeHitTarget == FALSE)  {
            IApproachTargetDirect(p_creature, p_logic, p_obj) ;
        } else {
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;

            /* Stop moving while you do this. */
            ObjectStopMoving(p_obj) ;
        }
    }

    DebugEnd() ;
}

static T_void INavNone(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavNone") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Always target. */
    p_creature->targettingActive = TRUE ;

    DebugEnd() ;
}

static T_void INavBerserkerB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavBerserkerB") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanB(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        /* Yes, do we need to approach? */
        if (p_creature->canMeleeHitTarget == FALSE)  {
            IApproachTargetDirect(p_creature, p_logic, p_obj) ;
        } else {
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
        }
    }

    DebugEnd() ;
}

static T_void INavBarbarianGuard(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavBarbarianGaurd") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanB(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        /* Yes, are we feeling strong enough to attack? */
        if (p_creature->health > p_logic->hurtHealth)  {
            /* Yes, do we need to approach? */
            if (p_creature->canMeleeHitTarget == FALSE)  {
                IApproachTargetDirect(p_creature, p_logic, p_obj) ;
            } else {
                /* Otherwise, navigation says to just sit there (and probably */
                /* keep attacking). */
                p_creature->targettingActive = TRUE ;
            }
            p_creature->isFleeing = FALSE ;
        } else {
            /* We are hurt and need to flee (if we are not already */
            /* fleeing). */
            /* Randomly choose between fleeing direct and indirect */
            if (SyncTimeGet() & 1)  {
                IFleeDirect(p_creature, p_logic, p_obj) ;
            } else {
                IFleeIndirect(p_creature, p_logic, p_obj) ;
            }
            p_creature->isFleeing = TRUE ;
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
        }
    }

    DebugEnd() ;
}

static T_void INavBarbarianArcherB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavBarbarianArcherB") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        if (p_creature->health < p_logic->hurtHealth)  {
            /* Then flee. */
            IFleeDirect(p_creature, p_logic, p_obj) ;
            /* Navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
            p_creature->isFleeing = TRUE ;
        } else {
            /* Yes, do we need to approach? */
            if (p_creature->targetDistance > p_logic->maxMissileRange)  {
                IApproachTargetDirect(p_creature, p_logic, p_obj) ;
            } else {
                /* Are we too close? */
                if (p_creature->targetDistance < p_logic->minMissileRange)  {
                    /* Run away !!! */
                    IFleeDirect(p_creature, p_logic, p_obj) ;
                }
                /* Otherwise, navigation says to just sit there (and probably */
                /* keep attacking). */
                p_creature->targettingActive = TRUE ;
            }
            p_creature->isFleeing = FALSE ;
        }
    }

    DebugEnd() ;
}

static T_void INavBarbarianArcher(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavBarbarianArcher") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        /* Yes, do we need to approach? */
        if (p_creature->targetDistance > p_logic->maxMissileRange)  {
            IApproachTargetDirect(p_creature, p_logic, p_obj) ;
        } else {
            /* Are we too close? */
            if (p_creature->targetDistance < p_logic->minMissileRange)  {
                /* Run away !!! */
                IFleeDirect(p_creature, p_logic, p_obj) ;
                p_creature->isFleeing = TRUE ;
            } else {
                p_creature->isFleeing = FALSE ;
            }
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
        }
    }

    DebugEnd() ;
}

static T_void INavNeutralWanderer(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavNeutralWanderer") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Are we being hit?  (NOTE: only get a target if hit by someone) */
    if (p_creature->targetAcquired)  {
       /* Yes, we have been hit.  Run around screaming. */
       IFleeDirect(p_creature, p_logic, p_obj) ;
       p_creature->isFleeing = TRUE ;
    } else {
       /* No, we are not hurt.  Just wander around. */
       IWander(p_creature, p_logic, p_obj) ;
       p_creature->isFleeing = FALSE ;
    }

    DebugEnd() ;
}

static T_void INavScaredyCat(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavScaredyCat") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* If there is something after the creature, run. */
    /* Otherwise, just wander and scan. */
    if (p_creature->targetAcquired)  {
        IFleeDirect(p_creature, p_logic, p_obj) ;
        p_creature->isFleeing = TRUE ;
    } else {
        IWanderAndScan(p_creature, p_logic, p_obj) ;
        p_creature->isFleeing = FALSE ;
    }

    DebugEnd() ;
}

static T_void INavRoamingGaurd(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavRoamingGaurd") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    if (p_creature->targetAcquired == FALSE)  {
        /* Not targget, Look for an enemy. */
        IWanderAndScan(p_creature, p_logic, p_obj) ;
    } else {
        /* Target, approach and attack. */
        /* Are we too far? */
        if (p_creature->canMeleeHitTarget == FALSE)  {
            IApproachTargetDirect(p_creature, p_logic, p_obj) ;
        } else {
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
        }
    }

    DebugEnd() ;
}

static T_void INavComboFighterArcher(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean inRange = TRUE ;

    DebugRoutine("INavComboFighterArcher") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_obj != NULL) ;

    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    if (p_creature->targetAcquired == TRUE)  {
        /* Are we hurt? */
        if (p_creature->health < p_logic->hurtHealth)  {
            /* Then flee. */
            IFleeDirect(p_creature, p_logic, p_obj) ;
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
            p_creature->isFleeing = TRUE ;
        } else {
            /* Are we in missile range? */
            if (p_creature->targetDistance > p_logic->maxMissileRange)  {
                /* Too far, move closer. */
                IApproachTargetDirect(p_creature, p_logic, p_obj) ;
                inRange = FALSE ;
            }

            /* Player is too close to shoot, but not close enough for */
            /* melee, move in. */
            if ((p_creature->canMeleeHitTarget == FALSE) &&
                (p_creature->targetDistance <= p_logic->minMissileRange))  {
                IApproachTargetDirect(p_creature, p_logic, p_obj) ;
                inRange = FALSE ;
            }

            if (inRange == TRUE)  {
                p_creature->targettingActive = TRUE ;
            }
            p_creature->isFleeing = FALSE ;
        }
    }

    DebugEnd() ;
}

static T_void INavApproachAndShoot(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean inRange = TRUE ;

    DebugRoutine("INavApproachAndShoot") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_obj != NULL) ;

    if (p_creature->targetAcquired == FALSE)  {
        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    if (p_creature->targetAcquired == TRUE)  {
        /* Are we hurt? */
        if (p_creature->health < p_logic->hurtHealth)  {
            /* Then flee. */
            IFleeDirect(p_creature, p_logic, p_obj) ;
            /* Otherwise, navigation says to just sit there (and probably */
            /* keep attacking). */
            p_creature->targettingActive = TRUE ;
            p_creature->isFleeing = TRUE ;
        } else {
            IApproachTargetDirect(p_creature, p_logic, p_obj) ;

            /* Are we in missile range? */
            if (p_creature->targetDistance > p_logic->maxMissileRange)  {
                /* Too far, move closer. */
                inRange = FALSE ;
            }

            /* Player is too close to shoot, but not close enough for */
            /* melee, move in. */
            if ((p_creature->canMeleeHitTarget == FALSE) &&
                (p_creature->targetDistance <= p_logic->minMissileRange))  {
                inRange = FALSE ;
            }

            if (inRange == TRUE)  {
                p_creature->targettingActive = TRUE ;
            }
            p_creature->isFleeing = FALSE ;
        }
    }

    DebugEnd() ;
}

static T_void INavDipper(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavDipper") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    p_creature->allowFall = FALSE ;

    /* Check to see if we have a target or the target is lost. */
    /* If we have a target, update info about that target. */
    if (p_creature->targetAcquired == FALSE)  {
        /* If we don't have a target, dip down into the ground. */
        ICreatureDip(p_creature, p_logic, p_obj) ;

        ICreatureScanA(p_creature, p_logic, p_obj) ;
        p_creature->meleeDelayCount = 5 ;
        p_creature->missileDelayCount = 5 ;
    }

    /* Check if we have a target (yet/still). */
    if (p_creature->targetAcquired == TRUE)  {
        /* Try to be fully visible. */
        ICreatureUndip(p_creature, p_logic, p_obj) ;

        /* Don't move unless we are fully up. */
        if (p_creature->isUndipped == TRUE)  {
            INavBarbarianGuard(p_creature, p_logic, p_obj) ;
        }
    }

    DebugEnd() ;
}

static T_void INavTeleporter(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 newPosition ;
    T_sword16 x, y ;
    T_word16 angle ;
    T_sword32 oldX, oldY, oldZ ;
    T_sword16 floor ;
    T_word32 time ;

    DebugRoutine("INavTeleporter") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    time = SyncTimeGet() ;
    /* Check if this is the first time this teleporter has */
    /* been called. */
    if (p_creature->p_teleportList == NULL)  {
        p_creature->p_teleportList =
            MemAlloc(sizeof(T_objectGeneratorPosition) *
                MAX_TELEPORTER_POSITIONS) ;
        DebugCheck(p_creature->p_teleportList) ;
        if (p_creature->p_teleportList != NULL)  {
            p_creature->lengthTeleportList =
                ObjectGeneratorGetList(
                    ObjectGetType(p_obj),
                    p_creature->p_teleportList,
                    MAX_TELEPORTER_POSITIONS) ;
            p_creature->lastTeleportTime = time ;
        }
    }

    /* Is it time to teleport? */
    if ((time - p_creature->lastTeleportTime) > TIME_BETWEEN_TELEPORTS)  {
        p_creature->lastTeleportTime = time ;
        /* Time to teleport */
        /* Only do this if there are places to teleport to. */
        if (p_creature->lengthTeleportList != 0)  {
            newPosition = RandomValue() % p_creature->lengthTeleportList ;

            /* Only teleport if we are going elsewhere. */
            if (newPosition != p_creature->teleportPosition)  {
                /* Determine where this new position is on the map. */
                x = p_creature->p_teleportList[newPosition].x ;
                y = p_creature->p_teleportList[newPosition].y ;
                angle = p_creature->p_teleportList[newPosition].angle ;

                /* Get the current position in case we are wrong. */
                oldX = ObjectGetX(p_obj) ;
                oldY = ObjectGetY(p_obj) ;
                oldZ = ObjectGetZ(p_obj) ;

                /* Try teleporting there. */
                ObjectTeleport(p_obj, x, y) ;

                /* Where is the floor at the new position? */
                floor = MapGetWalkingFloorHeightAtXY(&p_obj->objMove, x, y) ;

                /* Check to see if our feet is on the ground. */
                if (floor != ObjectGetZ16(p_obj))  {
                    /* Teleport too high, go back. */
                    ObjectSetX(p_obj, oldX) ;
                    ObjectSetY(p_obj, oldY) ;
                    ObjectSetZ(p_obj, oldZ) ;
                    ObjectSetUpSectors(p_obj) ;
                } else {
                    /* Otherwise, the move was complete. */
                    /* Face the right direction (new). */
                    ObjectSetAngle(p_obj, angle) ;
                    /* Record the new place. */
                    p_creature->teleportPosition = newPosition ;

                    /* Put in the teleport effects */
                    EfxCreate(EFX_TELEPORT, oldX, oldY, oldZ, 1, TRUE, 0) ;
                    EfxCreate(
                        EFX_TELEPORT,
                        ObjectGetX(p_obj),
                        ObjectGetY(p_obj),
                        ObjectGetZ(p_obj),
                        1, TRUE, 0) ;
                }
            } else {
                /* Not going anywhere -- same position. */
            }
        }
    }

    /* Always target. */
    p_creature->targettingActive = TRUE ;

    DebugEnd() ;
}

static T_void INavHoming(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 angle ;
    T_word16 faceAngle ;
    T_word16 diffAngle ;
    T_word16 stepAngle ;
    T_word32 time ;

    DebugRoutine("INavHoming") ;

    if (p_creature->targetAcquired == TRUE)  {
        time = SyncTimeGet() ;
        if (p_creature->timeLastHoming == 0)
            p_creature->timeLastHoming = time ;

        if (((time - p_creature->timeLastHoming) > p_logic->faceDelay) ||
               (p_logic->faceDelay == 0)) {
            p_creature->timeLastHoming += p_logic->faceDelay ;

            /* But first, we need to face the target if not already facing. */
            /* What is the angular difference? */
            angle = p_creature->targetAngle ;
            if (p_logic->faceDelay != 0)  {
                faceAngle = ObjectGetAngle(p_obj) ;

                diffAngle = angle - faceAngle ;

                stepAngle = INT_ANGLE_180 / 8 ;

                /* Turn left or right? */
                if (diffAngle < INT_ANGLE_180)  {
                    if (diffAngle > stepAngle)
                        diffAngle = stepAngle ;
                    ObjectSetAngle(p_obj, angle+diffAngle) ;
                } else {
                    diffAngle = -diffAngle ;
                    if (diffAngle > stepAngle)
                        diffAngle = stepAngle ;
                    ObjectSetAngle(p_obj, angle-diffAngle) ;
                }
            } else {
                /* Face opponent with no delay. */
                ObjectSetAngle(p_obj, angle) ;
            }
        }

        p_creature->faceDelayCount++ ;
        if (p_creature->faceDelayCount >= p_logic->faceDelay)
            p_creature->faceDelayCount = 0 ;

        ObjectSetMovedFlag(p_obj) ;
    }

    IMoveForwardViaFlying(
        p_creature,
        p_logic,
        p_obj,
        FALSE) ;

    DebugEnd() ;
}

static T_void INavStraightLine(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword32 newX, newY ;
    T_sword32 oldZ ;
    T_objMoveStruct oldPos ;

    DebugRoutine("INavStraightLine") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Make sure we are always fully passable. */
    ObjectMakeFullyPassable(p_obj) ;

    /* No movement on this object.  We do it. */
    ObjectStopMoving(p_obj) ;

    /* Check if this is the first time this straight liner has */
    /* been called. (use the teleport list since it is not being used). */
    if (p_creature->p_teleportList == NULL)  {
        p_creature->p_teleportList =
            MemAlloc(sizeof(T_objectGeneratorPosition) *
                MAX_TELEPORTER_POSITIONS) ;
        DebugCheck(p_creature->p_teleportList) ;
        if (p_creature->p_teleportList != NULL)  {
            p_creature->lengthTeleportList =
                ObjectGeneratorGetList(
                    ObjectGetType(p_obj),
                    p_creature->p_teleportList,
                    MAX_TELEPORTER_POSITIONS) ;
        }

        /* Make sure that there are only two entries on the list. */
        DebugCheck(p_creature->lengthTeleportList == 2) ;

        /* Declare the target end point. */
        p_creature->targetX = p_creature->p_teleportList[1].x ;
        p_creature->targetY = p_creature->p_teleportList[1].y ;
        p_creature->targetAcquired = FALSE ;
//printf("%d target at %d, %d\n", ObjectGetServerId(p_obj), p_creature->targetX, p_creature->targetY) ;
    }

    /* Are we close enough to the endpoint? */
    if (CalculateDistance(
            ObjectGetX16(p_obj),
            ObjectGetY16(p_obj),
            p_creature->p_teleportList[1].x,
            p_creature->p_teleportList[1].y) < (ObjectGetRadius(p_obj)>>1))  {
        /* We are close enough.  Yeah!  Let's teleport back to the start */
        /* after making some special effects. */
//puts("Close enough to teleport") ;
        /* Teleport from effect */
        EfxCreate(
            EFX_TELEPORT,
            p_creature->p_teleportList[1].x<<16,
            p_creature->p_teleportList[1].y<<16,
            ObjectGetZ(p_obj),
            1,
            TRUE,
            0) ;

        /* Teleport to new location and create effect. */
        ObjectTeleportAlways(
            p_obj,
            p_creature->p_teleportList[0].x,
            p_creature->p_teleportList[0].y) ;
        EfxCreate(
            EFX_TELEPORT,
            p_creature->p_teleportList[0].x<<16,
            p_creature->p_teleportList[0].y<<16,
            ObjectGetZ(p_obj),
            1,
            TRUE,
            0) ;
    } else {
//printf("%d stepping to %d %d\n", ObjectGetServerId(p_obj), p_creature->p_teleportList[1].x, p_creature->p_teleportList[1].y) ;
        /* Too far away.  Get closer. */
        /* Get the correct angle to the target. */
        p_creature->targetAngle =
            MathArcTangent(
                (T_sword16)(p_creature->p_teleportList[1].x - ObjectGetX16(p_obj)),
                (T_sword16)(p_creature->p_teleportList[1].y - ObjectGetY16(p_obj))) ;
        ObjectSetAngle(p_obj, p_creature->targetAngle) ;

        /* Step a step in that direction. */
        oldZ = ObjectGetZ(p_obj) ;
        oldPos = p_obj->objMove ;
        ObjectGetForwardPosition(
            p_obj,
            p_logic->stepSize,
            &newX,
            &newY) ;
        ObjectSetX(p_obj, newX) ;
        ObjectSetY(p_obj, newY) ;
        ObjectSetUpSectors(p_obj) ;
        /* Check for climb */
        /* Going up or down? */
//printf("Got to %d %d\n", newX>>16, newY>>16) ;
        if (ObjectGetZ(p_obj) > oldZ)  {
//puts("Stepping up") ;
            /* See if we are going up too fast */
            if (ObjectGetZ(p_obj) > oldZ+(48<<16))  {
//puts("Too high to step") ;
                /* Cannot step up that high.  Move back and wait/sit there. */
                p_obj->objMove = oldPos ;
            }
        } else {
//puts("Step same or down") ;
            /* Going down?  Let it fall down. */
            ObjectSetZ(p_obj, oldZ) ;
        }
    }

    /* Always target. */
    p_creature->targettingActive = TRUE ;

    /* Never blocked */
    p_creature->moveBlocked = FALSE ;
    ObjectClearBlockedFlag(p_obj) ;

    /* Is there an area sound to play. */
    if (p_logic->wanderSound)  {
        /* See if we have an area sound assigned yet. */
        if (p_creature->areaSound == AREA_SOUND_BAD)  {
            /* Create an area sound for the missile. */
            p_creature->areaSound =
                AreaSoundCreate(
                    ObjectGetX16(p_obj),
                    ObjectGetY16(p_obj),
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AREA_SOUND_BAD,
                    NULL,
                    0,
                    p_logic->wanderSound) ;
        } else {
            /* Update the position of the area sound. */
            AreaSoundMove(
                p_creature->areaSound,
                ObjectGetX16(p_obj),
                ObjectGetY16(p_obj)) ;
        }
    }

    /* Immune to all damage, always. */
    p_creature->immuneToDamage = TRUE ;

//printf("ITCD: by %d\n", ObjectGetServerId(p_obj)) ;
    /* Constant damage! */
    ServerDamageAtWithType(
        (T_sword16)ObjectGetX16(p_obj),
        (T_sword16)ObjectGetY16(p_obj),
        (T_sword16)(ObjectGetZ16(p_obj) + (ObjectGetHeight(p_obj) >> 1)),
        (T_word16)ObjectGetRadius(p_obj),
        (T_word16)p_logic->meleeDamage,
        ObjectGetServerId(p_obj),
        p_logic->damageType) ;

    DebugEnd() ;
}

static T_void INavCloud(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("INavCloud") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Make sure we are always fully passable. */
    ObjectMakeFullyPassable(p_obj) ;

    /* No movement on this object.  We do it. */
    ObjectStopMoving(p_obj) ;

    /* Always target. */
    p_creature->targettingActive = TRUE ;

    /* Never blocked */
    p_creature->moveBlocked = FALSE ;
    ObjectClearBlockedFlag(p_obj) ;

    /* Is there an area sound to play. */
    if (p_logic->wanderSound)  {
        /* See if we have an area sound assigned yet. */
        if (p_creature->areaSound == AREA_SOUND_BAD)  {
            /* Create an area sound for the missile. */
            p_creature->areaSound =
                AreaSoundCreate(
                    ObjectGetX16(p_obj),
                    ObjectGetY16(p_obj),
                    500,
                    255,
                    AREA_SOUND_TYPE_LOOP,
                    0,
                    AREA_SOUND_BAD,
                    NULL,
                    0,
                    p_logic->wanderSound) ;
        } else {
            /* Update the position of the area sound. */
            AreaSoundMove(
                p_creature->areaSound,
                ObjectGetX16(p_obj),
                ObjectGetY16(p_obj)) ;
        }
    }

    /* Immune to all damage, always. */
    p_creature->immuneToDamage = TRUE ;

//printf("Cloud: by %d\n", ObjectGetServerId(p_obj)) ;
    /* Constant damage! */
    ServerDamageAtWithType(
        (T_sword16)ObjectGetX16(p_obj),
        (T_sword16)ObjectGetY16(p_obj),
        (T_sword16)(ObjectGetZ16(p_obj) + (ObjectGetHeight(p_obj) >> 1)),
        (T_word16)ObjectGetRadius(p_obj),
        (T_word16)p_logic->meleeDamage,
        ObjectGetServerId(p_obj),
        p_logic->damageType) ;

    DebugEnd() ;
}

static T_void IUpdateTarget(T_creatureState *p_creature)
{
    T_3dObject *p_target ;                  /* Current target. */
    T_sword16 creatureX, creatureY ;        /* Creature's position. */
    T_sword16 targetX, targetY ;            /* Target's position. */
    T_sword16 dx, dy ;                      /* Difference between two. */
    T_creatureLogic *p_logic ;
    T_sword32 meleeX, meleeY ;

    DebugRoutine("IUpdateTarget") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_creature->targetAcquired == TRUE) ;
//    DebugCheck(p_creature->targetID != 0) ;

    /* Ignore zero target's (fake) */
/*
    if (p_creature->targetID == 0)  {
        p_creature->targetAcquired = FALSE ;
        DebugEnd() ;
        return ;
    }
*/
    /* Note that we can't hit the target unless further */
    /* in the code says we can. */
    p_creature->canMeleeHitTarget = FALSE ;

    /* Get the creature's main logic. */
    p_logic = p_creature->p_logic ;

    /* Find the target by its id. */
    p_target = ObjectFind((T_word16)p_creature->targetID) ;

    /* Consider our previous target. */
    if (p_creature->lastTargetID != 0)  {
        /* If the target no longer exists, or the current */
        /* target is a creature. */
        if ((!p_target) || (ObjectIsCreature(p_target)))  {
            /* See if we have lost patience and want to go back to a */
            /* previous target. */
            if ((!p_target) || (SyncTimeGet() > p_creature->timeCheckBerserk))  {
                if ((!p_target) || ((RandomValue() % 100) >= (p_logic->chanceContinueAttack)))  {
                    /* Go back to the old target and forget this one. */
                    p_creature->targetID = p_creature->lastTargetID ;
                    p_creature->lastTargetID = 0 ;

                    /* Find this target. */
                    p_target = ObjectFind((T_word16)p_creature->targetID) ;

                    /* Check in another 10 seconds */
                    p_creature->timeCheckBerserk += 700 ;
                }
            }
        }
    }

    /* Is the object still existing? */
    if (p_target)  {
        /* See if we can get its new location. */
        if (!ObjectIsStealthy(p_target))  {
            /* Target is NOT stealthy. */
            /* Only go there if you can see the target. */
            if (Collide3dObjectToObjectCheckLineOfSight(
                p_creature->p_obj,
                p_target) == FALSE)  {
                /* Get its location. */
                p_creature->targetX = ObjectGetX16(p_target) ;
                p_creature->targetY = ObjectGetY16(p_target) ;
                p_creature->targetZ = ObjectGetZ16(p_target) ;
                Collide3dUpdateLineOfSightLast(
                    &p_creature->sight,
                    p_target) ;

                p_creature->canSeeTarget = TRUE ;
            } else {
                /* Cannot see the target. */
                p_creature->canSeeTarget = FALSE ;
            }
        } else {
            /* Target IS stealthy. */
            /* Therefore, don't update the 'canSeeTarget' or */
            /* the target location.  Keep everything the same. */
            if (Collide3dObjectToObjectCheckLineOfSight(
                p_creature->p_obj,
                p_target) == FALSE)  {
                /* Can see the target's last location. */
                p_creature->canSeeTarget = TRUE ;
            } else {
                /* Cannot see the target's last location. */
                p_creature->canSeeTarget = TRUE ;
            }
        }

        /* Check to make sure that the target is not in the */
        /* dead stance. */
        if (ObjectGetStance(p_target) != STANCE_DIE)  {
            /* OK, target is still valid.  Compute distance and angle. */
            creatureX = ObjectGetX16(p_creature->p_obj) ;
            creatureY = ObjectGetY16(p_creature->p_obj) ;
            targetX = p_creature->targetX ;
            targetY = p_creature->targetY ;

            /* Calculate distance to target. */
            p_creature->targetDistance = CalculateDistance(
                                             creatureX,
                                             creatureY,
                                             targetX,
                                             targetY) ;

            /* Calculate the angle to the target. */
            dx = targetX - creatureX ;
            dy = targetY - creatureY ;

            p_creature->targetAngle =
                MathArcTangent(
                    (T_sword16)(targetX - creatureX),
                    (T_sword16)(targetY - creatureY)) ;
//printf("%d) Set target angle to %04X at (%d, %d) from (%d, %d) by %s\n", ObjectGetServerId(p_creature->p_obj), p_creature->targetAngle, targetX, targetY, creatureX, creatureY, DebugGetCallerName()) ;
//printf("     Target is %d at %d %d\n", p_creature->targetID, ObjectGetX16(p_target), ObjectGetY16(p_target)) ;
            /* Now determine if a melee attack would hit the */
            /* creature. */
            /* Don't even check if the radius is too small. */
            if (ObjectGetRadius(p_creature->p_obj) < p_logic->maxMeleeRange)  {
                /* See where the hit would be placed. */
                ObjectGetAngularPosition(
                    p_creature->p_obj,
                    p_creature->targetAngle,
                    (T_sword16)(p_logic->maxMeleeRange),
                    &meleeX,
                    &meleeY) ;

                /* See if the target is at that location. */
                if (ObjectIsAtXY(
                        p_target,
                        (T_sword16)(meleeX>>16),
                        (T_sword16)(meleeY>>16)) == TRUE)
                    p_creature->canMeleeHitTarget = TRUE ;
            }
        } else {
//printf("(%d) Target %d stance %d is dead.\n", ObjectGetServerId(p_creature->p_obj), ObjectGetServerId(p_target), ObjectGetStance(p_target)) ;
            /* Previous target is now dead. */
            p_creature->targetAcquired = FALSE ;
            p_creature->targetID = 0 ;
#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
            fprintf(G_fp, "%d IUpdateTarget 1: Creature %d target is now 0\n",
                SyncTimeGet(),
                p_creature->objectID) ;
            printf("%d IUpdateTarget 1: Creature %d target is now 0\n", SyncTimeGet(), p_creature->objectID) ;
#           endif
        }
    } else {
//printf("(%d) Target is dead or gone\n", ObjectGetServerId(p_creature->p_obj)) ;
        /* Previous target no longer exists. */
        p_creature->targetAcquired = FALSE ;
        p_creature->targetID = 0 ;

        /* Cannot see the target. */
        p_creature->canSeeTarget = FALSE ;
#       ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
        fprintf(G_fp, "%d IUpdateTarget 2: Creature %d target is now 0\n",
            SyncTimeGet(),
            p_creature->objectID) ;
        printf("%d IUpdateTarget 2: Creature %d target is now 0\n", SyncTimeGet(), p_creature->objectID) ;
#       endif
    }

    DebugEnd() ;
}

static T_player ICreatureScanA(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_player player ;
    T_player foundPlayer = PLAYER_BAD ;
    T_word16 playerDistance = 0xFFFF ;
    T_3dObject *p_playerObj ;
    T_sword16 playerX, playerY ;
    T_sword16 creatureX, creatureY ;
    T_sword16 dx, dy ;
    T_word16 angle ;
    T_word16 creatureAngle ;
    T_sword32 viewAngle ;
    T_word16 distance ;
    T_sword32 leftAngle, rightAngle ;

    DebugRoutine("ICreatureScanA") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Looks for an opponent player in the vision field of the */
    /* creature.  Opponents that are not visible are not considered. */
    /* If more than one player is visible, the closest one is targetted. */

    creatureX = ObjectGetX16(p_obj) ;
    creatureY = ObjectGetY16(p_obj) ;
    creatureAngle = ObjectGetAngle(p_obj) ;

    /* First, progress through the list of characters. */
    player = PlayersGetFirstPlayer() ;
    while (player != PLAYER_BAD)  {
        DebugCheck(player != 0) ;
        /* Can we see that player? */
        p_playerObj = PlayersGetPlayerObject(player) ;

        /* See where the player is located. */
        playerX = ObjectGetX16(p_playerObj) ;
        playerY = ObjectGetY16(p_playerObj) ;

        /* Calculate the angle to the player. */
        dx = playerX - creatureX ;
        dy = playerY - creatureY ;
        angle = MathArcTangent((T_sword16)dx, (T_sword16)dy) ;

        /* Is the player in the field of vision? */
        viewAngle = angle ;
        viewAngle -= creatureAngle ;
        rightAngle = p_logic->visionField>>1 ;
        leftAngle = -rightAngle ;

        if ((viewAngle <= rightAngle ) &&
            (viewAngle >= leftAngle ))  {
            /* Yes, inside the view.  A possibility now exists. */

            /* Is there a line of sight?  Can the creature */
            /* see the player without walls being in the way? */
            if (Collide3dObjectToObjectCheckLineOfSight(
                    p_obj,
                    p_playerObj) == FALSE)  {
#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
                fprintf(G_fp, "%d ScanA Line Of Sight: Creature %d to %d\n",
                    SyncTimeGet(),
                    p_creature->objectID,
                    ObjectGetServerId(p_playerObj)) ;
#           endif
                /* Yes, a line of sight exists. */
                /* Ok, last check.  Is this the closest player. */
                /* Calculate the distance. */
                distance =
                    CalculateDistance(
                        playerX,
                        playerY,
                        creatureX,
                        creatureY) ;

                /* Take the closer player and if the same, make sure */
                /* we get the lower numbered one (to avoid sync problems). */
                if ((distance < playerDistance) ||
                    ((distance == playerDistance) &&
                     (player < foundPlayer)))  {
                    /* Closer (or the first), record this */
                    foundPlayer = player ;
                    playerDistance = distance ;
                }
            }
        }

        player = PlayersGetNextPlayer(player) ;
    }

    /* Did we find a player? */
    if (foundPlayer != PLAYER_BAD)  {
        /* Yes, a player was found. */
        /* Make this the target and update its stats. */
        p_playerObj = PlayersGetPlayerObject(foundPlayer) ;
        DebugCheck(ObjectGetServerId(p_playerObj) != 0) ;
        if ((p_playerObj) &&
            (!ObjectIsStealthy(p_playerObj)) &&
            (ObjectGetStance(p_playerObj) != STANCE_DIE))  {
            p_creature->targetID = ObjectGetServerId(p_playerObj) ;

            /* Make sure we have the target location. */
            p_creature->targetX = ObjectGetX16(p_playerObj) ;
            p_creature->targetY = ObjectGetY16(p_playerObj) ;
            p_creature->targetZ = ObjectGetZ16(p_playerObj) ;
            Collide3dUpdateLineOfSightLast(
                &p_creature->sight,
                p_playerObj) ;

#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
            fprintf(G_fp, "%d ScanA: Creature %d target is now %d\n",
                SyncTimeGet(),
                p_creature->objectID,
                p_creature->targetID) ;
#           endif
//printf("%d ScanA: Creature %d target is now %d\n", SyncTimeGet(), p_creature->objectID, p_creature->targetID) ;
//printf("found player %d\n", foundPlayer) ;
            p_creature->targetAcquired = TRUE ;

            IUpdateTarget(p_creature) ;

            /* Yell for getting a target (if a sound). */
            if (p_logic->targetSound != 0)
                ICreatureMakeSound(p_obj, p_logic->targetSound) ;
        }
    }

    DebugEnd() ;

    return foundPlayer ;
}

static T_void ICreatureScanB(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_player foundPlayer ;
    T_word16 angle ;
    T_word16 randNum ;

    DebugRoutine("ICreatureScanB") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Just like Scan A except we do something different */
    /* if we don't find what we are looking for. */

    /* NOTE: Get the angle first since ScanA might change the angle. */
    angle = ObjectGetAngle(p_obj) ;
    foundPlayer = ICreatureScanA(p_creature, p_logic, p_obj) ;

    /* If still no player is found, see if we need to look around. */
    if (foundPlayer == PLAYER_BAD)  {
        /* Is it time to turn? */
        p_creature->scanBCount++ ;
        if (p_creature->scanBCount >= SCANB_TURN_RATE)  {
            /* Yes, time to turn. */
            /* Turn back to the 'middle' or starting position */
            /* when we started scanning. */
            DebugCheck(p_creature->scanBFacing < SCANB_FACING_UNKNOWN) ;
            angle -= G_scanBFacingAngleArray[p_creature->scanBFacing] ;

            /* Are we turning left or right? */
#if 0
            if (p_creature->scanBDir == SCANB_DIR_TURN_LEFT)  {
                /* Turn left. */
                p_creature->scanBFacing-- ;

                /* If we have turned all the way left, */
                /* next time we will turn right. */
                if (p_creature->scanBFacing == 0)
                    p_creature->scanBDir = SCANB_DIR_TURN_RIGHT ;
            } else {
                /* Turn right. */
                p_creature->scanBFacing++ ;

                /* If we have turned all the way right, */
                /* next time we will turn left. */
                if (p_creature->scanBFacing == SCANB_FACING_UNKNOWN-1)
                    p_creature->scanBDir = SCANB_DIR_TURN_LEFT ;
            }
#endif
            randNum = RandomValue() & 31 ;

            if (randNum < SCANB_FACING_UNKNOWN)  {
                p_creature->scanBFacing = randNum ;
                /* Readjust our facing to that of the new angle. */
                DebugCheck(p_creature->scanBFacing < SCANB_FACING_UNKNOWN) ;
                angle += G_scanBFacingAngleArray[p_creature->scanBFacing] ;
                ObjectSetAngle(p_obj, angle) ;
                ObjectSetMovedFlag(p_obj) ;
            }

            /* Reset the counter. */
            p_creature->scanBCount = 0 ;
        }
    } else {
        /* We found a player.  Scanning is complete (until player */
        /* disappears or is killed). */
        /* Next time we start scanning, we will be scanning from */
        /* the middle. */
        p_creature->scanBFacing = SCANB_FACING_MIDDLE ;
        p_creature->scanBCount = 0 ;
    }

    DebugEnd() ;
}

static T_void IApproachTargetDirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 angle ;              /* Facing of creature. */

    DebugRoutine("IApproachTargetDirect") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* At this point, the target is in sight and the creature wants */
    /* to move toward the target in a straight line. */
    /* Movement can be broken down into two types:  ground & flying */
    /* Therefore, this routine is split into two. */

    if (p_creature->faceDelayCount == 0)  {
        /* But first, we need to face the target if not already facing. */
        /* What is the angular difference? */
        angle = p_creature->targetAngle ;

        /* Round the angle to a multiple of 45 degrees. */
        /* This will make creatures appear a little smarter so that */
        /* walk along "invisible" lines of path. */
        angle += (INT_ANGLE_45>>1) ;
        angle /= INT_ANGLE_45 ;
        angle *= INT_ANGLE_45 ;

        /* Face that direction. */
//printf("Approach target at angle %04X\n", angle) ;
        ObjectSetAngle(p_obj, angle) ;
    }

    p_creature->faceDelayCount++ ;
    if (p_creature->faceDelayCount >= p_logic->faceDelay)
        p_creature->faceDelayCount = 0 ;

    ObjectSetMovedFlag(p_obj) ;

    /* Move ahead. */
    if (p_logic->canFly == TRUE)
        IMoveForwardViaFlying(
            p_creature,
            p_logic,
            p_obj,
            FALSE) ;
    else
        IMoveForwardViaWalking(
            p_creature,
            p_logic,
            p_obj) ;

    DebugEnd() ;
}

static T_void IMoveForwardViaWalking(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("IMoveForwardViaWalking") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Walking on the ground is a matter of taking definite steps */
    /* to places where there is nothing in the way.  In many ways */
    /* it is simpler than the ObjMove could in that it does not */
    /* have to figure out all the places in between, but does almost */
    /* the same thing.  NOTE:  Even though a creature "steps" in */
    /* this code, it is still affected by outside events that cause */
    /* the creature to slide from external events (except ice). */

    /* Since we are already facing the right direction, we just */
    /* need to take a step forward. */
    IStepForward(
        p_creature,
        p_logic,
        p_obj) ;
//    ObjectForceUpdate(p_obj) ;

    DebugEnd() ;
}

static T_void IFleeDirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 angle ;

    DebugRoutine("IFleeDirect") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Movement can be broken down into two types:  ground & flying */
    /* Therefore, this routine is split into two. */

    if (p_creature->faceDelayCount == 0)  {
        /* But first, we need to face away from the target. */
        angle = p_creature->targetAngle ;

        /* Face away */
        angle -= INT_ANGLE_180 ;

        /* Round the angle to a multiple of 45 degrees. */
        /* This will make creatures appear a little smarter so that */
        /* walk along "invisible" lines of path. */
        angle += (INT_ANGLE_45>>1) ;
        angle /= INT_ANGLE_45 ;
        angle *= INT_ANGLE_45 ;

        /* Face that direction. */
//printf("Flee target at angle %04X\n", angle) ;
        ObjectSetAngle(p_obj, angle) ;
    }

    /* Do the move. */
    if (p_logic->canFly == TRUE)
        IMoveForwardViaFlying(
            p_creature,
            p_logic,
            p_obj,
            TRUE) ;
    else
        IMoveForwardViaWalking(
            p_creature,
            p_logic,
            p_obj) ;

    DebugEnd() ;
}

static T_void IFleeIndirect(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 angle ;

    DebugRoutine("IFleeIndirect") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Flee indirect is just like flee direct, except it */
    /* veers to left or right a bit randomly. */

    if (p_creature->faceDelayCount == 0)  {
        /* But first, we need to face away from the target. */
        angle = p_creature->targetAngle ;

        /* Face away */
        angle -= INT_ANGLE_180 ;

        /* Round the angle to a multiple of 45 degrees. */
        /* This will make creatures appear a little smarter so that */
        /* walk along "invisible" lines of path. */
        angle += (INT_ANGLE_45>>1) ;
        angle /= INT_ANGLE_45 ;
        angle *= INT_ANGLE_45 ;

        /* Now randomly face left or right 45 degrees. */
        if (RandomValue() & 1)
            angle += INT_ANGLE_45 ;
        else
            angle -= INT_ANGLE_45 ;

        /* Face that direction. */
//printf("Flee indirect target at angle %04X\n", angle) ;
        ObjectSetAngle(p_obj, angle) ;
    }

    /* Do the move. */
    if (p_logic->canFly == TRUE)
        IMoveForwardViaFlying(
            p_creature,
            p_logic,
            p_obj,
            TRUE) ;
    else
        IMoveForwardViaWalking(
            p_creature,
            p_logic,
            p_obj) ;

    DebugEnd() ;
}

static T_void IWander(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("IWander") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Just move forward. */
    IMoveForwardViaWalking(
        p_creature,
        p_logic,
        p_obj) ;

    DebugEnd() ;
}

static T_void IWanderAndScan(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    DebugRoutine("IWanderAndScan") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Wander like usual. */
    IWander(p_creature, p_logic, p_obj) ;

    /* Then scan for target. */
    ICreatureScanA(p_creature, p_logic, p_obj) ;

    DebugEnd() ;
}

/* Routine that makes a creature step forward at stepSize distance
/* LES 04/15/96  Created */
static T_void IStepForward(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword32 newX, newY, newZ ;
    E_Boolean isBlocked ;
    T_sword32 oldX, oldY ;
    T_word16 i ;
    T_sword16 lowestFloor ;
    E_Boolean canWalkThere = TRUE ;
    T_sectorType sectorType ;
    T_word16 areaSector ;
    T_word16 stepSize ;
    T_objMoveStruct objMove ;
    E_Boolean stuckOnEdge ;

    DebugRoutine("IStepForward") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    SyncMemAdd("StepForward creature %d from %d %d\n", ObjectGetServerId(p_obj), ObjectGetX16(p_obj), ObjectGetY16(p_obj)) ;
#   ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
    fprintf(G_fp, "*SF: Creature %d at (%08X %08X %08X)\n",
        ObjectGetServerId(p_obj),
        ObjectGetX(p_obj),
        ObjectGetY(p_obj),
        ObjectGetZ(p_obj)) ;
    //printf("*SF: Creature %d at (%08X %08X %08X)\n", ObjectGetServerId(p_obj), ObjectGetX(p_obj), ObjectGetY(p_obj), ObjectGetZ(p_obj)) ;
#   endif
    /* When we step forward, we don't care about height ... stay the same. */
    newZ = ObjectGetZ(p_obj) ;
    oldX = ObjectGetX(p_obj) ;
    oldY = ObjectGetY(p_obj) ;

    objMove = p_obj->objMove ;

    /* Are we stuck on an edge right now? */
    stuckOnEdge = FALSE ;
    lowestFloor = (newZ >> 16) - ObjectGetHeight(p_obj) ;
    for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
         areaSector = ObjectGetNthAreaSector(p_obj, i) ;
         /* Check the floor under each sector we are now standing on. */
        if (MapGetWalkingFloorHeight(&p_obj->objMove, areaSector) <=lowestFloor)  {
            /* Floor we just stepped on is TOO far below ... */
            stuckOnEdge = TRUE ;
            break ;
        }

         /* If the creature is in a sector type that he is not supposed */
         /* to be in, use the same "stuck on edge" logic. */
         if (MapGetSectorType(areaSector) & (~p_creature->p_logic->stayOnSectorType))  {
//printf("Creature %d out of area %d, on %d\n", ObjectGetServerId(p_obj), p_creature->p_logic->stayOnSectorType, MapGetSectorType(areaSector)) ;
             stuckOnEdge = TRUE ;
             break ;
         }
    }

    /* No exceptions, we are moving. */
    View3dSetExceptObjectByPtr(NULL) ;

    /* Just move in a angular step out the way we are facing. */
    stepSize = p_logic->stepSize ;
/* TESTING */
stepSize += (stepSize>>1) ;
    ObjectGetForwardPosition(
        p_obj,
        stepSize,
        &newX,
        &newY) ;
//printf("*SF: %d Creature %d at (%08X %08X %08X) going to (%08X %08X)\n", SyncTimeGet(), ObjectGetServerId(p_obj), ObjectGetX(p_obj), ObjectGetY(p_obj), ObjectGetZ(p_obj), newX, newY) ;

    /* Only do if creature is not fully passable. */
    isBlocked = Collide3dMoveToXYZ(
                    p_obj,
                    newX,
                    newY,
                    newZ) ;
    /* Update the blocked flag on the object. */
    if (isBlocked)  {
        p_creature->moveBlocked = TRUE ;
    } else {
        p_creature->moveBlocked = FALSE ;
    }

    /* Put the z back so that the creature can fall. */
    newZ = ObjectGetZ(p_obj) ;
    ObjectSetUpSectors(p_obj) ;
    ObjectSetZ(p_obj, newZ) ;

    if (ObjectIsFullyPassable(p_obj))  {
        /* Keep going in that direction. */
        if (isBlocked)  {
            p_obj->objMove = objMove ;
            isBlocked = FALSE ;
            p_creature->moveBlocked = FALSE ;
            ObjectClearBlockedFlag(p_obj) ;
        }
    } else {
        if (isBlocked == FALSE)  {
            /* If we are stuck on an edge, but are not block, then we */
            /* can walk anywhere -- into a pit.  Its either that or */
            /* be stuck forever. */
            if (stuckOnEdge)  {
                /* Can always walk there. */
                canWalkThere = TRUE ;
            } else {
                /* Check to see if we just stepped onto a ledge that is too */
                /* far down to get back up.  If we are, go back to where */
                /* we were and consider this area as a wall. */
                canWalkThere = TRUE ;
                lowestFloor = (newZ >> 16) - ObjectGetHeight(p_obj) ;
                for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
                     /* Check the floor under each sector we are now standing on. */
                     if (MapGetWalkingFloorHeight(&p_obj->objMove,
                             ObjectGetNthAreaSector(p_obj, i)) <= lowestFloor)  {
                         /* Floor we just stepped on is TOO far below ... */
                         canWalkThere = FALSE ;
                         break ;
                     }
                }
            }

            if (canWalkThere != FALSE)  {
                if (p_logic->stayOnSectorType != 0)  {
                    for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
                        areaSector = ObjectGetNthAreaSector(p_obj, i) ;
                        sectorType = MapGetSectorType(areaSector) ;
//printf("check sec %d with %d and %d\n", areaSector, sectorType, p_logic->stayOnSectorType) ;
                        if ((sectorType & p_logic->stayOnSectorType) ||
                            ((sectorType==0) && (p_logic->stayOnSectorType & SECTOR_TYPE_DIRT)))  {
                            continue ;
                        } else  {
//printf("Creature %d cannot walk onto sector type %d\n", ObjectGetServerId(p_obj), sectorType) ;
                            canWalkThere = FALSE ;
                            p_obj->objMove = objMove ;
                            /* Always, this is a block. */
                            p_creature->moveBlocked = TRUE ;
                            break ;
                        }
                    }
                } else {
                    canWalkThere = TRUE ;
                }
            } else  {
                 /* back up and consider this a blocked path. */
    //printf("CWT *SF: %d Creature %d at (%08lX %08lX %08lX) going to (%08lX %08lX)\n", SyncTimeGet(), ObjectGetServerId(p_obj), oldX, oldY, ObjectGetZ(p_obj), newX, newY) ;
    /*
                 Collide3dMoveToXYZ(
                     p_obj,
                     oldX,
                     oldY,
                     newZ) ;
    */

                 p_obj->objMove = objMove ;
                 /* Always, this is a block. */
                 p_creature->moveBlocked = TRUE ;
            }

            /* Cause the server to update this creature. */
            ObjectSetMovedFlag(p_obj) ;

#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
            fprintf(G_fp, "Creature %d at (%08X %08X %08X)\n",
                ObjectGetServerId(p_obj),
                ObjectGetX(p_obj),
                ObjectGetY(p_obj),
                ObjectGetZ(p_obj)) ;
            //printf("Creature %d at (%08X %08X %08X)\n", ObjectGetServerId(p_obj), ObjectGetX(p_obj), ObjectGetY(p_obj), ObjectGetZ(p_obj)) ;
#           endif
        } else {
            p_obj->objMove = objMove ;
            SyncMemAdd("  Blocked\n", 0, 0, 0) ;
        }
        SyncMemAdd(" SF now creature %d from %d %d\n", ObjectGetServerId(p_obj), ObjectGetX16(p_obj), ObjectGetY16(p_obj)) ;
    }

    DebugEnd() ;
}

/* IHandleBlockedMove is called when the creature was blocked when it moved
/* the last move.
/* LES 04/15/96  Created */
static T_void IHandleBlockedMove(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_wallActivation type ;
    T_word16 wallData ;

    DebugRoutine("IHandleBlockedMove") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Clear appropriate flags */
    ObjectClearBlockedFlag(p_obj) ;
    p_creature->moveBlocked = FALSE ;

    /* Try opening a door if just blocked. */
    if (p_creature->blockCount == 0)  {
        if (p_logic->canOpenDoors)  {
            MapGetForwardWallActivationType(
                p_obj,
                &type,
                &wallData) ;

            if (type == WALL_ACTIVATION_DOOR)  {
                /* Found a door. */
                DebugCheck(DoorIsAtSector(wallData) == TRUE) ;
                /* Is the door locked? */
                if (!DoorIsLock(wallData)) {
                    DoorOpen(wallData);
                }
            }
        }
    }

    /* Were we blocked last time? */
    if (p_creature->blockTurnDir == BLOCK_TURN_DIR_NONE)  {
        if (RandomValue() & 1)  {
            p_creature->blockTurnDir = BLOCK_TURN_DIR_LEFT ;
        } else {
            p_creature->blockTurnDir = BLOCK_TURN_DIR_RIGHT ;
        }
    }

    if (p_creature->blockCount <= 1)  {
        if (p_creature->blockTurnDir == BLOCK_TURN_DIR_LEFT)  {
//printf("Blocked, move to angle %04X\n", (ObjectGetAngle(p_obj) + INT_ANGLE_45)) ;
            ObjectSetAngle(
                p_obj,
                ((T_word16)(ObjectGetAngle(p_obj) + INT_ANGLE_45))) ;
        }
        if (p_creature->blockTurnDir == BLOCK_TURN_DIR_RIGHT)  {
//printf("Blocked, move to angle %04X\n", (ObjectGetAngle(p_obj) - INT_ANGLE_45)) ;
            ObjectSetAngle(
                p_obj,
                ((T_word16)(ObjectGetAngle(p_obj) - INT_ANGLE_45))) ;
        }
    }

    /* Move ahead. */
    if (p_logic->canFly == TRUE)
        IMoveForwardViaFlying(
            p_creature,
            p_logic,
            p_obj,
            FALSE) ;
    else
        IMoveForwardViaWalking(
            p_creature,
            p_logic,
            p_obj) ;

    if ((p_creature->targetAcquired) && (!CreatureIsMissile(p_obj)))  {
        IUpdateTarget(p_creature) ;

        if ((p_creature->canMeleeHitTarget == TRUE) &&
            ((p_creature->isFleeing == FALSE) || (p_creature->blockCount==2)))  {
            p_creature->blockCount = NUM_UPDATES_DO_BLOCK_MOVEMENT ;
            ObjectSetAngle(p_obj, p_creature->targetAngle) ;
            p_creature->moveBlocked = FALSE ;
            ObjectClearBlockedFlag(p_obj) ;
        }
    }

    p_creature->blockCount++ ;
    if (p_creature->blockCount >= NUM_UPDATES_DO_BLOCK_MOVEMENT)  {
        p_creature->blockCount = 0 ;
        p_creature->blockTurnDir = BLOCK_TURN_DIR_NONE ;
    }

    DebugEnd() ;
}

static T_void IMoveForwardViaFlying(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  E_Boolean flyAway)
{
    T_sword16 floor = -0x7FFE ;
    T_sword16 ceiling = 0x7FFE ;
    T_sword16 targetHeight ;
    T_3dObject *p_target ;
    T_sword16 z ;
    T_sword16 origZ ;
    T_sword16 dz ;
    T_sword16 stepSize ;
    T_word16 sector ;
    T_word16 i;
    T_sword16 h ;

    DebugRoutine("IMoveForwardViaFlying") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

//printf("Flying creature %d at z %04X\n", ObjectGetServerId(p_obj), ObjectGetZ16(p_obj)) ;
    /* No exceptions, we are moving. */
    View3dSetExceptObjectByPtr(NULL) ;

    /* Make sure our movement doesn't keep pulling it down. */
    if (!p_creature->isEarthbound)  {
        ObjectSetMoveFlags(
            p_obj,
            OBJMOVE_FLAG_IGNORE_GRAVITY) ;
    }
    ObjectClearMoveFlags(p_obj, OBJMOVE_FLAG_FORCE_NORMAL_FRICTION) ;

    /* At this point, the creature is facing the player */
    /* and just needs to move forward.  Accelerate the creature */
    /* towards the target. */
    ObjectAccelFlat(
        p_obj,
        p_logic->acceleration,
        ObjectGetAngle(p_obj)) ;

    if (p_creature->isSlowed)
        ObjectSetMaxVelocity(p_obj, 2) ;
    else
        ObjectSetMaxVelocity(p_obj, p_logic->maxVelocity) ;

    if (p_creature->targetAcquired == TRUE)  {
        /* Now all that needs to happen is to have the creature */
        /* move up and down to "simulate" the flying experience */
        /* First we need to know what is the minimum and maximum heights */
        /* allowed to the flyer. */
        ObjectForceUpdate(p_obj) ;
//printf("  -- pre z %04X", ObjectGetZ16(p_obj)) ;
        ObjMoveUpdate(&p_obj->objMove, 0) ;
//printf("  -- post z %04X\n", ObjectGetZ16(p_obj)) ;
        floor = (ObjectGetLowestPoint(p_obj)>>16) ;
        ceiling = (ObjectGetHighestPoint(p_obj)>>16) ;
//printf("IMFVF---------: %d %d %d\n", ObjectGetServerId(p_obj), floor, ceiling) ;

        /* Take off the space for the height of the creature. */
        ceiling -= ObjectGetHeight(p_obj) ;
//printf("IMFVF: %d %d\n", ObjectGetServerId(p_obj), ceiling) ;

        /* Now determine how much we need to move up and down to */
        /* get at the target. */
        p_target = ObjectFind((T_word16)p_creature->targetID) ;
//printf("  target is %d\n", p_creature->targetID) ;
        if (p_target != NULL)  {
            targetHeight =
                ObjectGetZ16(p_target) +
                (ObjectGetHeight(p_target)>>1) -
                (ObjectGetHeight(p_obj)>>1) ;

            if (p_creature->lastX == 0x7FFE)  {
                p_creature->lastX = ObjectGetX16(p_obj) ;
                p_creature->lastY = ObjectGetY16(p_obj) ;
            }

            /* Now try to attain that height. */
//            z = p_creature->targetZ ;
            origZ = z = ObjectGetZ16(p_obj) ;
//printf("  %d %d\n", z, targetHeight) ;
            if (!p_creature->isEarthbound)  {
//                stepSize = p_logic->stepSize ;
                stepSize = CalculateDistance(
                               ObjectGetX16(p_obj),
                               ObjectGetY16(p_obj),
                               p_creature->lastX,
                               p_creature->lastY) ;

                /* Calculate slope to target */
                if (p_creature->targetDistance == 0)
                    dz = p_logic->stepSize ;
                else
                    dz = (((T_sword32)stepSize) * ((T_sword32)(((T_sword32)targetHeight) - ((T_sword32)z)))) / ((T_sword32)p_creature->targetDistance) ;

                if ((dz < 0) && (dz > -4))
                    dz = -4 ;
                if ((dz > 0) && (dz < 4))
                    dz = 4 ;
                if ((dz == 0) && (targetHeight != z))  {
                    if (targetHeight > z)
                        dz = 1 ;
                    else
                        dz = -1 ;
                }

//printf("creature %d steps %d along %d (delta z: %d -> %d) dist: %d\n", ObjectGetServerId(p_obj), dz, stepSize, targetHeight, z, p_creature->targetDistance) ;
            } else  {
                stepSize = 0 ;
                dz = 0 ;
            }

            p_creature->lastX = ObjectGetX16(p_obj) ;
            p_creature->lastY = ObjectGetY16(p_obj) ;
//printf("  dz: %d\n", dz) ;

            /* Are we flying towards or away from our target? */
//printf("z goes from %04X to ", z) ;
            if (flyAway == FALSE)
                z += dz ;
            else
                z -= dz ;
//printf("%04X\n", z) ;

            /* Make sure the Z is not out of bounds. */
            /* NOTE:  If for some reason the flying creature gets stuck */
            /* in an area to is too small for it (slowly smashing sector) */
            /* it will stay above the floor and go through the ceiling. */
            if (p_logic->explodeOnCollision)  {
                /* Missiles ignore objects when determine heights */
                floor = -0x7FFE ;
                ceiling = 0x7FFE ;

                for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
                    sector = ObjectGetNthAreaSector(p_obj, i) ;
                    h = MapGetWalkingFloorHeight(&p_obj->objMove, sector) ;
                    if (h > floor)
                        floor = h ;
                    h = MapGetCeilingHeight(sector) ;
                    if (h < ceiling)
                        ceiling = h ;
                }
            }

            /* Make sure our new z is good, else bump back */
            if (z > ceiling)
               z = origZ ;
            if (z < floor)
               z = origZ ;
//printf("IMFVF: %d newz %04X (oldz = %04X)\n", ObjectGetServerId(p_obj), z, ObjectGetZ16(p_obj)) ;
            /* See if we are going to have a problem with something */
            /* in the way. */
            if ((p_logic->explodeOnCollision) ||
                 (ObjectCheckIfCollide(
                    p_obj,
                    ObjectGetX(p_obj),
                    ObjectGetY(p_obj),
                    z<<16) == FALSE))  {
                /* Nothing in way, moving up or down. */
                ObjectSetZ16(p_obj, z) ;
//printf("  new Z %04X ok\n", z) ;
            } else {
//printf("  Z movement blocked (zobj=%04X)\n", ObjectGetZ16(p_obj));
            }
        }
    }
    ObjectForceUpdate(p_obj) ;

    /* That should conclude the flying. */

    DebugEnd() ;
}

static E_Boolean ICreatureTargetLogic(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    E_Boolean isGone = FALSE ;

    DebugRoutine("ICreatureTargetLogic") ;

    /* Make sure we have a target. */
    if (p_creature->targetAcquired == TRUE)  {
        /* Face target if we are not fleeing. */
        if (p_creature->isFleeing == FALSE)  {
//printf("Target logic angle %04X\n", p_creature->targetAngle) ;
            ObjectSetAngle(p_obj, p_creature->targetAngle) ;
        }

        /* Dispatch this call to one of the more appropriate calls. */
        DebugCheck(p_logic->targetLogic < TARGET_LOGIC_PACKAGE_UNKNOWN) ;
        isGone = G_creatureTargetRoutineArray
                     [p_logic->targetLogic]
                         (p_creature, p_logic, p_obj) ;
#if 0
    } else if (ObjectIsFullyPassable(p_obj)) {
printf("ICTL: fp %d for %d (%d)\n", p_logic->targetLogic, ObjectGetServerId(p_obj), ObjectGetType(p_obj)) ;
        /* Dispatch this call to one of the more appropriate calls. */
        DebugCheck(p_logic->targetLogic < TARGET_LOGIC_PACKAGE_UNKNOWN) ;
        isGone = G_creatureTargetRoutineArray
                     [p_logic->targetLogic]
                         (p_creature, p_logic, p_obj) ;
#endif
    }

    DebugEnd() ;

    return isGone ;
}

static E_Boolean ITargetNone(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    DebugRoutine("ITargetNone") ;
    DebugEnd() ;

    return FALSE ;
}

static E_Boolean ITargetStandardHitOrShoot(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 attackLow, attackHigh ;
    T_3dObject *p_target ;
    T_sword16 targetBottom, targetTop ;

    DebugRoutine("ITargetStandardHitOrShoot") ;
    p_target = ObjectFind((T_word16)p_creature->targetID) ;
    if (p_target != NULL)  {
        /* We need to choose if we are attacking with a weapon or a missile */
        /* Are we close enough to attack melee? */
        if (p_creature->canMeleeHitTarget == TRUE)  {
            /* Are we at the right height? */
            /* Attack over a range based on the attacker's height. */
            attackLow = ObjectGetZ16(p_obj) +
                        ((ObjectGetHeight(p_obj))>>2) ;
            attackHigh = ObjectGetZ16(p_obj) +
                             ObjectGetHeight(p_obj) ;
            targetBottom = p_creature->targetZ ;
            targetTop = targetBottom + ObjectGetHeight(p_target) ;

            if ((attackHigh >= targetBottom) &&
                 (attackLow <= targetTop))  {
                /* The target is at the right height, do an attack. */
                IAttackTargetWithMelee(
                    p_creature,
                    p_logic,
                    p_obj,
                    (T_sword16)((targetBottom+targetTop)>>1)) ;
            }
        } else {
            /* Do we have a missile to shoot? */
            if (p_logic->missileType != 0)  {
                /* Are we in missile distance? */
//printf("%d %d considering missile: targetDistance %d, target %d\n", SyncTimeGet(), ObjectGetServerId(p_obj), p_creature->targetDistance, ObjectGetServerId(p_target)) ;
                if ((p_creature->targetDistance >= p_logic->minMissileRange) ||
                    (p_creature->targetDistance <= p_logic->maxMissileRange))  {
                    IShootAtTarget(p_creature, p_logic, p_obj) ;
                }
            }
        }
    }
    DebugEnd() ;

    return FALSE ;
}

static E_Boolean ITargetSuicideExplosion(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean isGone ;

    DebugRoutine("ITargetSuicideExplosion") ;

    if ((p_creature->health < p_logic->hurtHealth) &&
        (p_creature->canMeleeHitTarget == TRUE))  {
        IExplodeSelf(
            p_creature,
            p_logic,
            p_obj,
            (T_word16)((100 + p_creature->health) * 3)) ;
        isGone = TRUE ;
    } else {
        /* Just do standard targetting. */
        ITargetStandardHitOrShoot(
            p_creature,
            p_logic,
            p_obj) ;
    }

    DebugEnd() ;

    return isGone ;
}

static E_Boolean ITargetExplodeOnCollision(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean isGone = FALSE ;

    DebugRoutine("ITargetExplodeOnCollision") ;

    /* If the object was blocked, then kablooy. */
    if ((p_creature->moveBlocked) ||
        (ObjectWasBlocked(p_obj)))  {

        /* Create the death explosion. */
        IDoEfxAtDeath(p_creature, p_logic, p_obj) ;

        /* Explode! */
        ServerDamageAtWithType(
            (T_sword16)ObjectGetX16(p_obj),
            (T_sword16)ObjectGetY16(p_obj),
            (T_sword16)(ObjectGetZ16(p_obj) + (ObjectGetHeight(p_obj) >> 1)),
            (T_word16)p_logic->maxMeleeRange,
            (T_word16)p_logic->meleeDamage,
            ObjectGetServerId(p_obj),
            p_logic->damageType) ;

        /* Check if this killed us. */
        if (ICreatureFindViaObjectPtr(p_obj) ==
            DOUBLE_LINK_LIST_ELEMENT_BAD)
                isGone = TRUE ;

        /* If this doesn't kill us, make sure it does. */
        if (!isGone)  {
            /* Death comes onto us. */
            ObjectSetStance(p_obj, STANCE_DIE) ;
            ICreatureMakeSound(p_obj, p_logic->dieSound) ;
            CreatureDropTreasure(p_creature, p_logic, p_obj) ;
            ObjectForceUpdate(p_obj) ;
            p_creature->health = 0 ;

            /* Remove the creature from our thinking list. */
//           CreatureDetachFromObject(p_obj) ;
            p_creature->markedForDestroy = TRUE ;
            isGone = TRUE ;
        }
/* Because missiles don't destroy themselves, we must do it ourself. */
/* !!! This should be changed in the resource file so that missiles */
/* have a nice disappearing explosion. */
        ObjectStopMoving(p_obj) ;
//        ObjectRemove(p_obj) ;
//        ObjectDestroy(p_obj) ;
        ObjectMarkForDestroy(p_obj) ;
    }

    DebugEnd() ;

    return isGone ;
}

static E_Boolean ITargetScream(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 creatureX, creatureY ;
    T_sword16 playerX, playerY ;
    T_3dObject *p_target ;
    E_Boolean wasImmune ;

    DebugRoutine("ITargetScream") ;

    /* Is it time to consider screaming? */
    if (p_creature->meleeDelayCount == 0)  {
        /* If in range, scream. */
        if (p_creature->targetDistance < (p_logic->maxMeleeRange-10))  {
            /* Also make sure that you can see your enemy. */
            creatureX = ObjectGetX16(p_obj) ;
            creatureY = ObjectGetY16(p_obj) ;
            p_target = ObjectFind((T_word16)p_creature->targetID) ;
            if (p_target != NULL)  {
                playerX = p_creature->targetX ;
                playerY = p_creature->targetY ;
/*
                if (Collide3dCheckLineOfSight(
                        creatureX,
                        creatureY,
                        playerX,
                        playerY) == FALSE)  {
*/
                if (Collide3dObjectToXYCheckLineOfSight(
                        p_obj,
                        &p_creature->sight,
                        playerX,
                        playerY) == FALSE)  {
                    /* Stop moving while you do this. */
                    ObjectStopMoving(p_obj) ;

                    /* Look like we are attacking. */
                    ObjectSetStance(p_obj, STANCE_STAND) ;
                    ObjectSetStance(p_obj, STANCE_ATTACK) ;
                    ICreatureMakeSound(p_obj, p_logic->attackSound) ;

                    /* Make this creature immune to the next damage. */
                    wasImmune = p_creature->immuneToDamage ;
                    p_creature->immuneToDamage = TRUE ;

                    /* Scream! */
                    ServerDamageAtWithType(
                        (T_sword16)ObjectGetX16(p_obj),
                        (T_sword16)ObjectGetY16(p_obj),
                        (T_sword16)(ObjectGetZ16(p_obj) + (ObjectGetHeight(p_obj) >> 1)),
                        (T_word16)p_logic->maxMeleeRange,
                        (T_word16)p_logic->meleeDamage,
                        ObjectGetServerId(p_obj),
                        p_logic->damageType) ;

                    p_creature->immuneToDamage = wasImmune ;

                    /* Reset the melee counter. */
                    p_creature->meleeDelayCount = p_logic->meleeAttackDelay ;
                }
            }
        }
    }

    DebugEnd() ;

    return FALSE ;
}

static E_Boolean ITargetSummonCreature(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean doStandard = TRUE ;
    T_word32 time ;
    T_3dObject *p_summoned ;
    T_sword32 forwardX, forwardY ;

    DebugRoutine("ITargetSummonCreature") ;

    if (p_creature->targetAcquired == TRUE)  {
        /* Have a target.  Is it time to summon? */
        time = SyncTimeGet() ;
        if (p_creature->timeToSummon == 0)
            p_creature->timeToSummon = time ;

        if ((time - p_creature->timeToSummon) > TIME_BETWEEN_SUMMONS)  {
            /* Time to attempt a summoning. */

            /* Don't summon if too many creatures. */
            if (DoubleLinkListGetNumberElements(G_creatureList) <
                MAX_CREATURES_FOR_SUMMON)  {
                /* There appears to be room.  Let's try adding another */
                p_summoned = ObjectCreate() ;
                DebugCheck(p_summoned != NULL) ;
                if (p_summoned)  {
                    /* Figure out where we are trying to put the summed */
                    /* creature. */
                    ObjectGetForwardPosition(
                        p_obj,
                        SUMMON_DISTANCE,
                        &forwardX,
                        &forwardY) ;

                    /* Initialize the object. */
                    ObjectDeclareStatic(
                        p_summoned,
                        (T_sword16)(forwardX>>16),
                        (T_sword16)(forwardY>>16)) ;
                    ObjectSetType(p_summoned, SUMMON_CREATURE_TYPE) ;
                    ObjectSetAngle(p_summoned, ObjectGetAngle(p_obj)) ;
                    ObjectSetUpSectors(p_summoned) ;
                    ObjectSetZ16(
                        p_summoned,
                        MapGetWalkingFloorHeight(&p_obj->objMove,
                            ObjectGetAreaSector(p_summoned))) ;

                    if ((ObjectCheckIfCollide(
                            p_summoned,
                            forwardX,
                            forwardY,
                            ObjectGetZ(p_summoned)) == FALSE) &&
                         (ObjectGetZ16(p_summoned) >= (ObjectGetZ16(p_obj)-16)) &&
                         (ObjectGetZ16(p_summoned) <= (ObjectGetZ16(p_obj)+30)))  {
                        ObjectAdd(p_summoned) ;
                        /* Don't do anything else for this creature. */
                        doStandard = FALSE ;

                        /* Look like we are attacking. */
                        if ((ObjectGetStance(p_obj) == STANCE_STAND) ||
                            (ObjectGetStance(p_obj) == STANCE_WALK))  {
                            ObjectSetStance(p_obj, STANCE_ATTACK) ;
                            ICreatureMakeSound(p_obj, p_logic->attackSound) ;
                        }

                        /* Note that we did a summoning. */
                        p_creature->timeToSummon = time ;
                    } else {
                        /* Nope.  Can't create it. */
                        ObjectDestroy(p_summoned) ;
                    }
                }
            }
        }
    }
    if (doStandard == TRUE)  {
        /* If I wasn't told not to, do the standard thing. */
        ITargetStandardHitOrShoot(p_creature, p_logic, p_obj) ;
    }

    DebugEnd() ;

    return FALSE ;
}

static E_Boolean ITargetConstantDamage(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    E_Boolean isGone = FALSE ;

    DebugRoutine("ITargetConstantDamage") ;

    DebugEnd() ;

    return FALSE ;
}

static T_void IShootAtTarget(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 creatureX, creatureY ;
    T_sword16 playerX, playerY ;
    T_3dObject *p_target ;

    DebugRoutine("IShootAtTarget") ;

    /* Don't shoot missiles every time. */
    if (p_creature->missileDelayCount == 0)  {
        /* Face the opponent exactly. */
//printf("Shoot at angle %04X\n", p_creature->targetAngle) ;
        ObjectSetAngle(p_obj, p_creature->targetAngle) ;

        /* Make sure that the target is in view before we */
        /* try to do any unnecessary firing. */
        creatureX = ObjectGetX16(p_obj) ;
        creatureY = ObjectGetY16(p_obj) ;
        p_target = ObjectFind((T_word16)p_creature->targetID) ;
        if (p_target != NULL)  {
            playerX = ObjectGetX16(p_target) ;
            playerY = ObjectGetY16(p_target) ;
            if (p_creature->canSeeTarget == TRUE)  {
                /* Look like we are attacking. */
                ObjectSetStance(p_obj, STANCE_STAND) ;
                ObjectSetStance(p_obj, STANCE_ATTACK) ;
                ICreatureMakeSound(p_obj, p_logic->attackSound) ;

#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
            fprintf(G_fp, "%d IShootAtTarget (%d at %d (%d %d))\n",
                SyncTimeGet(),
                p_creature->objectID,
                ObjectGetServerId(p_target),
                ObjectGetX16(p_target),
                ObjectGetY16(p_target)) ;
#           endif
//printf("%d IShootAtTarget (%d at %d (%d %d))\n", SyncTimeGet(), p_creature->objectID, ObjectGetServerId(p_target), ObjectGetX16(p_target), ObjectGetY16(p_target)) ;
//printf("creature %d (%d) shoots %d\n", ObjectGetServerId(p_obj), p_logic->objectType, p_logic->missileType) ;
                SyncMemAdd("Shooter %d (%d) shoots %d\n", ObjectGetServerId(p_obj), p_logic->objectType, p_logic->missileType) ;
                SyncMemAdd("  shoot at target %d at %d %d\n", ObjectGetServerId(p_target), ObjectGetX16(p_target), ObjectGetY16(p_target)) ;
                ServerShootProjectile(
                    p_obj,
                    p_creature->targetAngle,
                    p_logic->missileType,
                    30,
                    p_target) ;
            }
        }
        /* Set up the "wait" time for the next missile. */
        p_creature->missileDelayCount = p_logic->missileAttackDelay ;
    }

    DebugEnd() ;
}

static T_void IAttackTargetWithMelee(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_sword16 z)
{
    T_word16 num ;
    T_wallListItem wallList[20] ;
    T_sword32 frontX, frontY ;
    T_sword16 creatureX, creatureY ;

    DebugRoutine("IAttackTargetWithMelee") ;

    /* Slow down the attacks. */
    if (p_creature->meleeDelayCount == 0)  {
        /* !!! Attack !!! */
        /* Face the enemy at the exact angle, we don't want to miss. */
//printf("Melee at angle %04X\n", p_creature->targetAngle) ;
        ObjectSetAngle(p_obj, p_creature->targetAngle) ;

        /* Find where we are attacking. */
        ObjectGetForwardPosition(
            p_obj,
            (T_sword16)(p_logic->maxMeleeRange),
            &frontX,
            &frontY) ;

        p_creature->meleeDelayCount = p_logic->meleeAttackDelay ;

        /* Check to see if a wall is in the way of the player. */
        creatureX = ObjectGetX16(p_obj) ;
        creatureY = ObjectGetY16(p_obj) ;
        num = Collide3dFindWallList(
                  creatureX,
                  creatureY,
                  frontX>>16,
                  frontY>>16,
                  z,
                  20,
                  wallList,
                  WALL_LIST_ITEM_UPPER |
                      WALL_LIST_ITEM_LOWER |
                          WALL_LIST_ITEM_MAIN) ;
        if (num == 0)  {
            /* No walls are in the way. */
            /* Record the z to attack and schedule for the attack */
            /* to be a little bit later (to allow the animation */
            /* to occur at the right timing). */
            p_creature->targetAttackZ = z ;

            p_creature->delayedAttackTime = SyncTimeGet() + 18 ;

            /* Put the creature into an attack stance. */
            ObjectSetStance(p_obj, STANCE_STAND) ;
            ObjectSetStance(p_obj, STANCE_ATTACK) ;

            /* Make the appropriate sound. */
            if (p_logic->minMissileRange==0)
            {
                if (rand()%6==0) ICreatureMakeSound(p_obj, p_logic->attackSound);
                else ICreatureMakeSound(p_obj, p_logic->attackSound2);
            }
            else ICreatureMakeSound(p_obj, p_logic->attackSound2);
        }
    }

    DebugEnd() ;
}

static T_void ICreatureDelayedAttack(T_creatureState *p_creature)
{
    T_sword32 frontX, frontY ;
    T_3dObject *p_obj ;
    T_creatureLogic *p_logic ;
    T_word16 attackDist ;

    DebugRoutine("ICreatureDelayedAttack") ;
    DebugCheck(p_creature != NULL) ;

    if (p_creature)  {
        p_obj = p_creature->p_obj ;
        p_logic = p_creature->p_logic ;

        /* Attack up close if player is nearer */
        attackDist = p_logic->maxMeleeRange ;
        if (p_creature->targetDistance < attackDist)
            attackDist = p_creature->targetDistance ;

        /* Find where we are attacking. */
        ObjectGetForwardPosition(
            p_obj,
            (T_sword16)(p_logic->maxMeleeRange),
            &frontX,
            &frontY) ;

        /* Damage at that location. */
        ServerDamageAtWithType(
            (T_sword16)(frontX>>16),
            (T_sword16)(frontY>>16),
            p_creature->targetAttackZ,
            3,
            p_logic->meleeDamage,
            ObjectGetServerId(p_obj),
            p_logic->damageType) ;
    }

    DebugEnd() ;
}

/* ------------------------------------ Routines to consider later: ----- */
T_void CreatureTakeDamage(
           T_3dObject *p_obj,       /* Creature to affect. */
           T_word32 damage,         /* Amount of damage to do. */
           T_word16 type,            /* Type of damage to do. */
           T_word16 ownerID)        /* Who is doing the damage. */
{
    T_creatureState *p_creature ;
    T_creatureLogic *p_logic ;
    T_word32 damageAmt ;
    T_word16 numEffects ;
    T_word16 numResists ;

    DebugRoutine("CreatureTakeDamage") ;

    p_creature = ObjectGetExtraData(p_obj) ;
    DebugCheck(p_creature != NULL) ;
    if (p_creature != NULL)  {
        if ((p_creature->markedForDestroy == FALSE) &&
            (p_creature->immuneToDamage == FALSE) &&
            (!ObjectIsFullyPassable(p_obj)))  {
/*
printf("Creature %d (%d) takes damage %d (was health %d) by %s\n",
  ObjectGetServerId(p_obj),
  ObjectGetType(p_obj),
  damage,
  p_creature->health,
  DebugGetCallerName()) ;
*/
            p_logic = p_creature->p_logic ;
            DebugCheck(p_logic != NULL) ;

            /* Make sure this is not damage of our own (if the */
            /* flag that says so is TRUE */
            if ((ownerID != ObjectGetServerId(p_creature->p_obj)) ||
                (p_logic->canHurtSelf == TRUE))  {
                /* Consider the special damage amounts. */
                damageAmt = damage ;
                numEffects = 0 ;
                numResists = 0 ;

                /* Is this a special type of damage? */
                if (type & EFFECT_DAMAGE_SPECIAL)  {
                    switch(type & (~EFFECT_DAMAGE_SPECIAL))  {
                        case EFFECT_DAMAGE_SPECIAL_LOCK:
                        case EFFECT_DAMAGE_SPECIAL_UNLOCK:
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_PUSH:
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_PULL:
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_BERSERK:
                            /* Make the creature lose its target. */
                            p_creature->targetAcquired = FALSE ;
                            IConsiderTargetChange(
                                p_creature,
                                p_logic,
                                p_obj,
                                IFindClosestCreature(
                                    ObjectGetX16(p_obj),
                                    ObjectGetY16(p_obj),
                                    p_creature)) ;
                            /* Go berserk for 10 seconds */
                            p_creature->timeCheckBerserk = SyncTimeGet() + 700 ;
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_DISPEL_MAGIC:
                            /* Does not affect creatures. */
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_EARTHBIND:
                            /* Only effects flying creatures. */
                            if (p_logic->canFly == TRUE)  {
                                /* Make creature no longer fly */
                                p_creature->isEarthbound = TRUE ;
                                p_creature->timeEarthboundEnds =
                                    RandomValue() + 2100 ;
                                ObjectClearMoveFlags(
                                    p_obj,
                                    OBJMOVE_FLAG_IGNORE_GRAVITY) ;
                            }
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_CONFUSE:
                            ObjectSetAngle(p_creature->p_obj, (RandomValue()<<2)) ;
                            p_creature->targetID = 0 ;   /* Loose target */
                            p_creature->targetAcquired = FALSE ;
                            p_creature->moveBlocked = TRUE ;

                            /* You don't know who the owner is. */
                            ownerID = 0 ;
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_SLOW:
                            /* Make creature no longer fly */
                            p_creature->isSlowed = TRUE ;
                            p_creature->timeSlowEnds =
                                RandomValue() + 2100 ;
                            break ;
                        case EFFECT_DAMAGE_SPECIAL_PARALYZE:
                            /* Does not affect creatures. */
                            break ;
                        default:
                            DebugCheck(FALSE) ;
                            break ;
                    }

                    /* Do no physical damage. */
                    damageAmt = 0 ;
                } else {
                    /* No, just regular types of damages. */
                    /* Go through the different types of damage */
                    if (type & EFFECT_DAMAGE_NORMAL)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_NORMAL)
                            numResists++ ;
                    }

                    /* Check for fire based damages. */
                    if (type & EFFECT_DAMAGE_FIRE)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_FIRE)
                            numResists++ ;
                        else
                            /* Fire damage does +25% */
                            damageAmt += (damage>>2) ;
                    }

                    if (type & EFFECT_DAMAGE_ACID)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_ACID)
                            numResists++ ;
                        /* !!! Currently creatures have no harmful */
                        /* effect towards acid. */
                    }

                    if (type & EFFECT_DAMAGE_POISON)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_POISON)  {
                            numResists++ ;
                        } else {
                            /* Poison is transferred into the poison level. */
                            p_creature->poisonLevel += damage/10 ;
                        }
                    }

                    if (type & EFFECT_DAMAGE_ELECTRICITY)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_ELECTRICITY)  {
                            numResists++ ;
                        } else {
                            /* Electricity does additional damage based */
                            /* on the armor of the enemy. */
                            switch (p_logic->armorType)  {
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH:
                                    /* Cloth is normal. */
                                    break ;
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                                    /* Chain adds 25% damage. */
                                    damageAmt += (damage>>2) ;
                                    break ;
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                                    /* Plate adds 50% damage. */
                                    damageAmt += (damage>>1) ;
                                    break ;
                                default:
                                    /* Nothing special here. */
                                    break ;
                            }
                        }
                    }

                    if (type & EFFECT_DAMAGE_MANA_DRAIN)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_MANA_DRAIN)  {
                            numResists++ ;
                        } else {
                            /* !!! Warning! Mana drain does nothing to */
                            /* creatures currently. */
                        }
                    }

                    if (type & EFFECT_DAMAGE_PIERCING)  {
                        numEffects++ ;
                        if (p_logic->damageResist & EFFECT_DAMAGE_PIERCING)  {
                            numResists++ ;
                        } else {
                            /* Piercing usually depends on the amount */
                            /* of armor the opponent is wearing (and what */
                            /* type).  Based on the creature's toughness, */
                            /* piercing does better. */
                            switch (p_logic->armorType)  {
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_CLOTH:
                                    /* No armor to go through. */
                                    break ;
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_CHAIN:
                                    /* Add 2% per level of armor */
                                    damageAmt +=
                                        (damage * 2 *
                                        (2+ObjectGetColorizeTable(p_obj))) / 100 ;
                                    break ;
                                case EQUIP_ARMOR_TYPE_BREASTPLATE_PLATE:
                                    /* Add 4% per level of armor */
                                    damageAmt +=
                                        (damage * 4 *
                                        (2+ObjectGetColorizeTable(p_obj))) / 100 ;
                                    break ;
                                default:
                                    break ;
                            }
                        }
                    }

                    if (numEffects>0)  {
                        damageAmt -= (damageAmt * numResists) / numEffects ;
                    } else {
                        damageAmt = 0 ;
                    }

                    /* Can only do true damage up to the creatures health. */
                    if (damageAmt > p_creature->health)
                        damageAmt = p_creature->health ;

                    /* Check to see if this player did the damage and if */
                    /* so, give him the experience of the damage. */
                    /* Also, only bother if any damage. */
                    if (damageAmt)  {
//printf("%d actually done.\n", damageAmt) ;  fflush(stdout) ;
                        if (ownerID == ObjectGetServerId(PlayerGetObject()))  {
                            if (!(type & EFFECT_DAMAGE_SPECIAL))
                                StatsChangePlayerExperience(damageAmt);
                        }

                        /* Are we going to die? */
                        if (damageAmt >= p_creature->health)  {
                            /* Death comes onto us. */
                            ObjectSetStance(p_obj, STANCE_DIE) ;
                            ICreatureMakeSound(p_obj, p_logic->dieSound) ;
                            IDoEfxAtDeath(p_creature, p_logic, p_obj) ;
                            CreatureDropTreasure(p_creature, p_logic, p_obj) ;
                            p_creature->health = 0 ;
                            ObjectForceUpdate(p_obj) ;

                            /* Remove the creature from our thinking list. */
                            p_creature->markedForDestroy = TRUE ;

                            /* Make sure gravity takes effect. */
                            ObjectClearMoveFlags(
                                p_obj,
                                OBJMOVE_FLAG_IGNORE_GRAVITY) ;
                        } else {
                            /* Nope.  Still up and running. */
                            ObjectSetStance(p_obj, STANCE_HURT) ;
                            p_creature->health -= damageAmt ;
                            if ((rand()&1) == 0)  {
                                ICreatureMakeSound(p_obj, p_logic->hurtSound2) ;
                            } else {
                                ICreatureMakeSound(p_obj, p_logic->hurtSound) ;
                            }
                        }
                    }
                }

                /* Check if this damage is different than our current target */
                /* and is not 0. */
                if ((type != (EFFECT_DAMAGE_SPECIAL|EFFECT_DAMAGE_SPECIAL_CONFUSE)) &&
                    (type != (EFFECT_DAMAGE_SPECIAL|EFFECT_DAMAGE_SPECIAL_BERSERK))) {
                    if ((ownerID != 0) && (ownerID != p_creature->targetID))  {
                        /* Consider changing targets. */
                        IConsiderTargetChange(
                            p_creature,
                            p_logic,
                            p_obj,
                            ownerID) ;
                    }
                }
            }
            /* Creatures that are not allowed to walk on dirt (the hydra */
            /* for example) should not be moved around when taking damage */
            if (!(p_logic->stayOnSectorType & SECTOR_TYPE_DIRT))
                ObjectStopMoving(p_obj) ;
        }
    }


    DebugEnd() ;
}

/* LES 04/16/96 */
static T_void IExplodeSelf(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_word16 damage)
{
    E_Boolean isGone = FALSE ;

    DebugRoutine("IExplodeSelf") ;

//printf("IExplodeSelf %d called by %s\n", ObjectGetServerId(p_obj), DebugGetCallerName()) ;
    /* Put the creature into an hurt stance. */
    ObjectSetStance(p_obj, STANCE_STAND) ;
    ObjectSetStance(p_obj, STANCE_DIE) ;
    ICreatureMakeSound(p_obj, p_logic->dieSound) ;

    /* Create the death explosion. */
    IDoEfxAtDeath(p_creature, p_logic, p_obj) ;

    /* Explode!  Enemy is close and I'm hurt! */
    ServerDamageAtWithType(
        (T_sword16)ObjectGetX16(p_obj),
        (T_sword16)ObjectGetY16(p_obj),
        (T_sword16)(ObjectGetZ16(p_obj) + ((ObjectGetHeight(p_obj)) / 2)),
        (T_word16)p_logic->maxMeleeRange,
        (T_word16)damage,
        (T_word16)ObjectGetOwnerID(p_obj),
        (T_byte8)p_logic->damageType) ;

    /* Check if this killed us. */
    if (ICreatureFindViaObjectPtr(p_obj) ==
        DOUBLE_LINK_LIST_ELEMENT_BAD)
            isGone = TRUE ;

    if (!isGone)  {
//        CreatureDetachFromObject(p_obj) ;
        p_creature->markedForDestroy = TRUE ;
        isGone = TRUE ;
    }

/* NOTE: Since missiles don't destroy themselves, this routine */
/* will have to do it for now. */
    ObjectStopMoving(p_obj) ;
//    ObjectRemove(p_obj) ;
    ObjectMarkForDestroy(p_obj) ;
//    ObjectDestroy(p_obj) ;

    DebugEnd() ;
}


#ifdef SERVER_ONLY
T_3dObject *CreatureLookForPlayer(T_3dObject *p_obj)
{
    return NULL ;
}
#else
T_word16 CreatureLookForPlayer(T_3dObject *p_obj)
{
    return 0;  // TODO: Is this right?
}
#endif

T_void CreaturePlayerGone(T_player player)
{
}

static T_void IConsiderTargetChange(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj,
                  T_word16 proposedNewTargetID)
{
    T_word16 distance ;
    T_sword16 creatureX, creatureY ;
    T_sword16 targetX, targetY ;
    T_3dObject *p_target;
    T_3dObject *p_oldTarget ;
    E_Boolean isOldBlocked, isNewBlocked ;
    E_Boolean newTargetTaken = FALSE ;

    DebugRoutine("IConsiderTargetChange") ;

    /* Don't consider a change if I accidently hurt myself. */
    /* Don't consider a change if I'm fully passable. */
    if ((proposedNewTargetID != ObjectGetServerId(p_obj)) &&
        (!ObjectIsFullyPassable(p_obj)))  {
        /* Try identifying this target. */
        p_target = ObjectFind(proposedNewTargetID) ;

        /* Does it exist. */
        if (p_target != NULL)  {
            /* First, if I don't have a target, this will do. */
            if (p_creature->targetID == 0)  {
                newTargetTaken = TRUE ;
            } else {
                /* Find out who is in view and what is the distance. */
                creatureX = ObjectGetX16(p_obj) ;
                creatureY = ObjectGetY16(p_obj) ;
                targetX = ObjectGetX16(p_target) ;
                targetY = ObjectGetY16(p_target) ;

                p_oldTarget = ObjectFind((T_word16)p_creature->targetID) ;

                /* Is old target still there? */
                if (p_oldTarget != NULL)  {
                    /* Old target is still around. */
                    /* Who can we see? */
                    isOldBlocked = Collide3dObjectToObjectCheckLineOfSight(
                                        p_obj,
                                        p_oldTarget) ;
                    isNewBlocked = Collide3dObjectToObjectCheckLineOfSight(
                                        p_obj,
                                        p_target) ;

                    /* Can we see the old target? */
                    if (isOldBlocked == TRUE)  {
                        /* No, we cannot see the old target */
                        /* Can we see the new target? */
                        if (isNewBlocked == FALSE)  {
                            /* Yes, we can see the new target. */
                            /* Make this the new target. */
                            newTargetTaken = TRUE ;
                        } else {
                            /* No, since we can't see either */
                            /* target, keep with what the */
                            /* creature remembered -- the old target */
                        }
                    } else {
                        /* Yes, we can see the old target. */
                        if (isNewBlocked == FALSE)  {
                            /* We can also see the new target. */
                            /* Who is closer? */
                            distance = CalculateDistance(
                                           creatureX,
                                           creatureY,
                                           targetX,
                                           targetY) ;

                            /* If the new target is closer than */
                            /* the old, take that target. */
                            if (distance < p_creature->targetDistance)  {
                                newTargetTaken = TRUE ;
                            }
                        } else {
                            /* We cannot see the new target, but */
                            /* we can see the old target. */
                            /* Since the creature can't identify */
                            /* who hit him, keep going after */
                            /* the same old target. */
                        }
                    }
                } else {
                    /* Old target is not there.  This will be */
                    /* our target. */
                    newTargetTaken = TRUE ;
                }
            }
        }

        if ((newTargetTaken == TRUE) && (ObjectIsCreature(p_target)))  {
            /* We have just agreed to attack a fellow creature.  Is this */
            /* generally ok? */
            if ((RandomValue() % 100) >= p_logic->chanceAttackCreature)  {
                /* No.  Ignore the request to change. */
                newTargetTaken = FALSE ;
            } else {
                /* Go berserk for 10 seconds */
                p_creature->timeCheckBerserk = SyncTimeGet() + 700 ;
            }
        }

        if (newTargetTaken == TRUE)  {
            /* Record the old target.  We might go back to you. */
            p_creature->lastTargetID = p_creature->targetID ;

            /* This is our new target, go for it. */
            p_creature->targetID = proposedNewTargetID ;
#           ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
            fprintf(G_fp, "%d Change: Creature %d target is now %d\n",
                SyncTimeGet(),
                p_creature->objectID,
                p_creature->targetID) ;
#           endif
//printf("%d Change: Creature %d target is now %d\n", SyncTimeGet(), p_creature->objectID, p_creature->targetID) ;
            p_creature->targetAcquired = TRUE ;
            p_creature->targetX = ObjectGetX16(p_target) ;
            p_creature->targetY = ObjectGetY16(p_target) ;
            p_creature->targetZ = ObjectGetZ16(p_target) ;
            Collide3dUpdateLineOfSightLast(
                &p_creature->sight,
                p_target) ;

            IUpdateTarget(p_creature) ;
        }
    }

    DebugEnd() ;
}

#define CREATURE_DIP_AMOUNT_ABOVE_FLOOR 34
static T_void ICreatureDip(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 floor ;
    T_sword16 creatureHeight ;
    T_sword16 targetZ ;
    T_sword16 creatureZ ;
    T_word16 distance ;

    DebugRoutine("ICreatureDip") ;

    /* Stop moving so we can dip correctly. */
    ObjectStopMoving(p_obj) ;

    floor = MapGetWalkingFloorHeight(&p_obj->objMove, ObjectGetAreaSector(p_obj));
    creatureHeight = ObjectGetHeight(p_obj) ;
    creatureZ = ObjectGetZ16(p_obj) ;

    /* Dipping leaves about 30 pixels from the walking floor height */
    /* above the water. */
    targetZ = floor + CREATURE_DIP_AMOUNT_ABOVE_FLOOR - creatureHeight ;
//printf("c: %d, z: %d, t: %d, s: %d, h: %d\n", ObjectGetServerId(p_obj), creatureZ, targetZ,
//      ObjectGetAreaSector(p_obj),
//      MapGetWalkingFloorHeight(ObjectGetAreaSector(p_obj))) ;

    /* See what we need to do */
    if (targetZ > creatureZ)  {
        distance = targetZ - creatureZ ;
        if (distance > p_logic->stepSize)
            distance = p_logic->stepSize ;
        creatureZ += distance ;
    } else {
        distance = creatureZ - targetZ;
        if (distance > p_logic->stepSize)
            distance = p_logic->stepSize ;
        creatureZ -= distance ;
    }

    /* Move to the new height. */
    ObjectSetZ16(p_obj, creatureZ) ;

    /* Note if we are fully dipped. */
    if (creatureZ == targetZ)  {
        p_creature->isDipped = TRUE ;
    } else  {
        p_creature->isDipped = FALSE ;
        p_creature->isUndipped = FALSE ;
    }

//    ObjectSetMovedFlag(p_obj) ;

    DebugEnd() ;
}

static T_void ICreatureUndip(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 creatureZ ;
    T_sword16 targetZ ;
    T_word16 distance ;

    DebugRoutine("ICreatureUndip") ;

    targetZ = MapGetWalkingFloorHeight(&p_obj->objMove, ObjectGetAreaSector(p_obj));
    creatureZ = ObjectGetZ16(p_obj);

    /* See what we need to do */
    if (targetZ > creatureZ)  {
        distance = targetZ - creatureZ ;
        if (distance > p_logic->stepSize)
            distance = p_logic->stepSize ;
        creatureZ += distance ;
    } else {
        distance = creatureZ - targetZ;
        if (distance > p_logic->stepSize)
            distance = p_logic->stepSize ;
        creatureZ -= distance ;
    }

    /* Move to the new height. */
    ObjectSetZ16(p_obj, creatureZ) ;

    /* Note if we are fully dipped. */
    if (creatureZ == targetZ)  {
        p_creature->isUndipped = TRUE ;
    } else {
        p_creature->isDipped = FALSE ;
        p_creature->isUndipped = FALSE ;
    }

    ObjectSetMovedFlag(p_obj) ;

    DebugEnd() ;
}

/* IDeathNone is the standard death routine called that says, yeah, I'm dead.
/* LES 04/26/96  Created. */
static E_Boolean IDeathNone(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    E_Boolean deathIsDone = FALSE ;

    DebugRoutine("IDeathNone") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    deathIsDone = TRUE ;

    DebugEnd() ;

    return deathIsDone ;
}

static E_Boolean IDeathSink(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    E_Boolean deathIsDone = FALSE ;

    DebugRoutine("IDeathSink") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Dip the creature into the ground and if the creature is */
    /* dipped, destroy the object and tell that it is dead. */
    ICreatureDip(p_creature, p_logic, p_obj) ;
    if (p_creature->isDipped)  {
        ObjectMarkForDestroy(p_obj) ;
        deathIsDone = TRUE ;
    }

    DebugEnd() ;

    return deathIsDone ;
}

static E_Boolean IDeathFastDisappear(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    E_Boolean deathIsDone = FALSE ;

    DebugRoutine("IDeathFastDisappear") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Mark the creature's object for destruction. */
    ObjectMarkForDestroy(p_obj) ;
    deathIsDone = TRUE ;

    DebugEnd() ;

    return deathIsDone ;
}

#define DEATH_LOGIC_TIME_TO_DECAY      (60*70)  /* One minute to decay */
static E_Boolean IDeathDecay(
                     T_creatureState *p_creature,
                     T_creatureLogic *p_logic,
                     T_3dObject *p_obj)
{
    T_word32 time ;
    E_Boolean deathIsDone = FALSE ;

    DebugRoutine("IDeathDecay") ;
    DebugCheck(p_creature != NULL) ;
    DebugCheck(p_logic != NULL) ;
    DebugCheck(p_obj != NULL) ;

    /* Get the time and if this is the first time, record it. */
    time = SyncTimeGet() ;
    if (p_creature->timeOfDeath == 0)
        p_creature->timeOfDeath = time ;

    /* How long has it been?  Is it time to decay? */
    if ((time - p_creature->timeOfDeath) >= DEATH_LOGIC_TIME_TO_DECAY)  {
        /* Yes, it is time. */
        /* Do we change into something else? */
        if (p_logic->decayObject != 0)  {
            /* Yes, we change into the decay object. */
            /* !!! This will destroy the calling creature references !!! */
            ObjectSetType(p_obj, p_logic->decayObject) ;
            /* !!! After this point, all pointers are invalid. */
            /* Death doesn't apply to this case. */
            deathIsDone = FALSE ;
        } else {
            /* No, turn into nothing. */
            /* Standard destroy procedure. */
            SyncMemAdd("IDeathDecay: %d\n", ObjectGetServerId(p_obj), 0, 0) ;
            ObjectMarkForDestroy(p_obj) ;
            deathIsDone = TRUE ;
        }
    }

    DebugEnd() ;

    return deathIsDone ;
}

E_Boolean CreatureIsMissile(T_3dObject *p_obj)
{
    T_creatureState *p_creature ;
    T_creatureLogic *p_logic ;
    E_Boolean isMissile = FALSE ;

    DebugRoutine("CreatureIsMissile") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(ObjectGetExtraData(p_obj) != NULL) ;

    p_creature = (T_creatureState *)ObjectGetExtraData(p_obj) ;
    p_logic = p_creature->p_logic ;

    if (p_logic->explodeOnCollision == TRUE)
        isMissile = TRUE ;

    DebugEnd() ;

    return isMissile ;
}

static T_void CreatureDropTreasure(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_sword16 x, y, z ;
    T_3dObject *p_newobj ;
    T_word16 r ;
    static T_word16 badKnightList[12] = { 204, 204, 204, 204, 204, 204,
                                         818, 204, 708, 709, 710, 711 } ;
    static T_word16 mithrilBadKnightList[12] = { 32968, 32968, 32968, 204, 204, 204,
                                                818, 32968, 708, 709, 710, 711 } ;
    static T_word16 evilKnightList[12] = { 102, 102, 102, 204, 204, 204,
                                          102, 204, 708, 709, 710, 711 } ;
    static T_word16 silverEvilKnightList[12] = { 103, 103, 103, 204, 204, 204,
                                                103, 20684, 21184, 21185, 21186, 21187 } ;
    static T_word16 bronzeEvilKnightList[12] = { 12492, 12492, 12492, 102, 102, 102,
                                                102, 12492, 12996, 12997, 12998, 12999 } ;
    static T_word16 trojanList[12] = { 37067, 37067, 37067, 104, 104, 104,
                                                105, 12492, 12996, 12997, 12998, 12999 } ;

    DebugRoutine("CreatureDropTreasure") ;

    x = ObjectGetX16(p_obj) + (RandomValue() % 6) - 3;
    y = ObjectGetY16(p_obj) + (RandomValue() % 6) - 3;
    z = ObjectGetZ16(p_obj) ;
    r = RandomValue() ;

    /* Creatures that have their treasure stolen don't drop any. */
    if (p_creature->wasStolenFrom == FALSE)  {
        switch (p_logic->treasureToDrop)  {
            case TREASURE_TYPE_NONE:
                break ;
            case TREASURE_TYPE_1_PLAT:
                ServerCreateObjectGlobal(106, x, y, z) ;
                break ;
            case TREASURE_TYPE_1_GOLD:
                ServerCreateObjectGlobal(104, x, y, z) ;
                break ;
            case TREASURE_TYPE_1_SILVER:
                ServerCreateObjectGlobal(102, x, y, z) ;
                break ;
            case TREASURE_TYPE_5_COPPER:
                ServerCreateObjectGlobal(101, x, y, z) ;
                break ;
            case TREASURE_TYPE_SHADOW:
                break ;
            case TREASURE_TYPE_LICH:
                break ;
            case TREASURE_TYPE_ELK:
                ServerCreateObjectGlobal(247, x, y-2, z) ;  /* Piece of meat */
                ServerCreateObjectGlobal(247, x+2, y, z) ;  /* Piece of meat */
                ServerCreateObjectGlobal(247, x-2, y, z) ;  /* Piece of meat */
                ServerCreateObjectGlobal(247, x, y+2, z) ;  /* Piece of meat */
                break ;
            case TREASURE_TYPE_ELF_ARCHER:
                if (r&1)  {
                    /* Drop quiver of 6 arrows */
                    p_newobj = ServerCreateObjectGlobal(144, x-2, y, z) ;
                    ObjectSetAccData(p_newobj, 6) ;
                } else {
                    /* or drop 1 copper */
                    ServerCreateObjectGlobal(100, x, y, z) ;
                }
                break ;
            case TREASURE_TYPE_EXIGUUS: /* Cemetary wizard */
                /* Ring of armor 2 */
                ServerCreateObjectGlobal(416, x, y, z) ;
                /* Orb of the Undead */
                ServerCreateObjectGlobal(955, x, y, z) ;
//                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
//                ObjectSetAccData(p_newobj, 410) ;
                break ;
            case TREASURE_TYPE_BAD_KNIGHT:
                ServerCreateObjectGlobal(badKnightList[r%12], x, y, z) ;
                break ;
            case TREASURE_TYPE_MITHRIL_BAD_KNIGHT:
                ServerCreateObjectGlobal(mithrilBadKnightList[r%12], x, y, z) ;
                break ;
            case TREASURE_TYPE_EVIL_ORANGE_WIZARD:
                if (r&1)  {
                    p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
                    ObjectSetAccData(p_newobj, 407) ;
                } else  {
                    ServerCreateObjectGlobal(104, x, y, z) ;
                }
                break ;
            case TREASURE_TYPE_BLUE_WIZARD:
                if (r&1)
                    ServerCreateObjectGlobal(102, x, y, z) ;
                else
                    ServerCreateObjectGlobal(362, x, y, z) ;
                break ;
            case TREASURE_TYPE_DRUID:
                if (r&1)
                    ServerCreateObjectGlobal(202, x, y, z) ;
                else
                    ServerCreateObjectGlobal(822, x, y, z) ;
                break ;
            case TREASURE_TYPE_POISON_DRUID:
                if (r&1)
                    ServerCreateObjectGlobal(202, x, y, z) ;
                else
                    ServerCreateObjectGlobal(825, x, y, z) ;
                break ;
            case TREASURE_TYPE_EVIL_KNIGHT:
                ServerCreateObjectGlobal(evilKnightList[r%12], x, y, z) ;
                break ;
            case TREASURE_TYPE_SILVER_EVIL_KNIGHT:
                ServerCreateObjectGlobal(silverEvilKnightList[r%12], x, y, z) ;
                break ;
            case TREASURE_TYPE_VITORIX:
                /* Electric mace */
                ServerCreateObjectGlobal(234, x, y, z) ;
                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
                ObjectSetAccData(p_newobj, 672) ;
                break ;
            case TREASURE_TYPE_JUGURTHA:
                /* Ring of armor 1 */
                ServerCreateObjectGlobal(415, x, y, z) ;
                /* Electric mace */
                ServerCreateObjectGlobal(234, x, y, z) ;
                break ;
            case TREASURE_TYPE_EVIL_ARCHER:
                /* Drop quiver of 6 fire arrows */
                p_newobj = ServerCreateObjectGlobal(145, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 6) ;
                break ;
            case TREASURE_TYPE_ELYMAS:
                /* Ring of armor 3 */
                ServerCreateObjectGlobal(417, x, y, z) ;
                /* drops Elymus' notes */
                ServerCreateObjectGlobal(380, x, y, z) ;
                break ;
            case TREASURE_TYPE_MATTAN:
                /* Wand of mana drain */
                ServerCreateObjectGlobal(908, x, y, z) ;
                break ;
            case TREASURE_TYPE_MAMMA_DRAGON:
                /* Green Key to exit */
//                ServerCreateObjectGlobal(260, x, y, z) ;
                break ;
            case TREASURE_TYPE_KOA_FOOTMAN:
                /* Drop quiver of 12 arrows */
                p_newobj = ServerCreateObjectGlobal(144, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 12) ;
                break ;
            case TREASURE_TYPE_KOA_HORSEMAN:
                /* Drop quiver of 6 piercing arrows */
                p_newobj = ServerCreateObjectGlobal(147, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 6) ;
                break ;
            case TREASURE_TYPE_BRONZE_EVIL_KNIGHT:
                ServerCreateObjectGlobal(bronzeEvilKnightList[r%12], x, y, z) ;
                break ;
            case TREASURE_TYPE_TROJAN:
                ServerCreateObjectGlobal(trojanList[r%12], x, y, z) ;
                break ;
            default:
                break ;
        }
    } else {
        switch (p_logic->treasureToDrop)  {
            case TREASURE_TYPE_EXIGUUS: /* Cemetary wizard */
//                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
//                ObjectSetAccData(p_newobj, 410) ;
                /* Orb of the Undead */
                ServerCreateObjectGlobal(955, x, y, z) ;
                break ;
            case TREASURE_TYPE_VITORIX:
                /* Electric mace */
                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
                ObjectSetAccData(p_newobj, 672) ;
                break ;
            case TREASURE_TYPE_EVIL_ARCHER:
                /* Drop quiver of 6 fire arrows */
                p_newobj = ServerCreateObjectGlobal(145, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 6) ;
                break ;
            case TREASURE_TYPE_MIGHTY_HUNTER:
                /* Drop quiver of 6 electrical arrows */
                p_newobj = ServerCreateObjectGlobal(149, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 6) ;
                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
                ObjectSetAccData(p_newobj, 644) ;
                p_newobj = ServerCreateObjectGlobal(373, x, y, z) ;
                ObjectSetAccData(p_newobj, 647) ;
                break ;
            case TREASURE_TYPE_KOA_FOOTMAN:
                /* Drop quiver of 12 arrows */
                p_newobj = ServerCreateObjectGlobal(144, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 12) ;
                break ;
            case TREASURE_TYPE_KOA_HORSEMAN:
                /* Drop quiver of 6 piercing arrows */
                p_newobj = ServerCreateObjectGlobal(147, x-2, y, z) ;
                ObjectSetAccData(p_newobj, 6) ;
                break ;
            case TREASURE_TYPE_ELYMAS:
                /* drops Elymus' notes */
                ServerCreateObjectGlobal(380, x, y, z) ;
                break ;
            case TREASURE_TYPE_JUGURTHA:
                /* Electric mace */
                ServerCreateObjectGlobal(234, x, y, z) ;
                break ;
            default:
                break ;
        }
    }

    DebugEnd() ;
}

T_word16 CreatureStolenFrom(T_3dObject *p_obj)
{
    T_word16 item = 0 ;  /* No item */
    T_word16 r ;
    T_doubleLinkListElement element ;
    T_creatureState *p_creature ;
    T_creatureLogic *p_logic ;

    DebugRoutine("CreatureStolenFrom") ;

    element = ICreatureFindViaObjectPtr(p_obj) ;
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Yes, creature is here to detach. */
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        p_logic = p_creature->p_logic ;

        /* Creatures that have their treasure */
        /* stolen don't have anything to steal. */
        if (p_creature->wasStolenFrom == FALSE)  {
            r = RandomValue() ;

            switch (p_logic->treasureToDrop)  {
                case TREASURE_TYPE_NONE:
                    break ;
                case TREASURE_TYPE_1_PLAT:
                    item = 106 ;
                    break ;
                case TREASURE_TYPE_1_GOLD:
                    item = 104 ;
                    break ;
                case TREASURE_TYPE_1_SILVER:
                    item = 102 ;
                    break ;
                case TREASURE_TYPE_5_COPPER:
                    item = 101 ;
                    break ;
                case TREASURE_TYPE_SHADOW:
                    break ;
                case TREASURE_TYPE_LICH:
                    break ;
                case TREASURE_TYPE_ELK:
                    break ;
                case TREASURE_TYPE_ELF_ARCHER:
                    item = 100 ;    /* copper coin */
                    break ;
                case TREASURE_TYPE_EXIGUUS: /* Cemetary wizard */
                    item = 416 ;    /* ring */
                    break ;
                case TREASURE_TYPE_BAD_KNIGHT:
                    item = 818 ;    /* strength potion */
                    break ;
                case TREASURE_TYPE_MITHRIL_BAD_KNIGHT:
                    item = 32968 ;  /* Mithril dagger */
                    break ;
                case TREASURE_TYPE_EVIL_ORANGE_WIZARD:
                    item = 104 ;  /* gold piece */
                    break ;
                case TREASURE_TYPE_BLUE_WIZARD:
                    if (r&1)
                        item = 102 ;   /* Silver piece */
                    else
                        item = 362 ;   /* Gain mana #1 */
                    break ;
                case TREASURE_TYPE_DRUID:
                    if (r&1)
                        item = 202 ;   /* Iron staff */
                    else
                        item = 822 ;   /* Potion of water */
                    break ;
                case TREASURE_TYPE_POISON_DRUID:
                    if (r&1)
                        item = 202 ;   /* Iron staff */
                    else
                        item = 825 ;   /* Potion of poison */
                    break ;
                case TREASURE_TYPE_EVIL_KNIGHT:
                    item = 102 ;  /* Silver coin */
                    break ;
                case TREASURE_TYPE_SILVER_EVIL_KNIGHT:
                    item = 103 ;  /* 5 silver coin */
                    break ;
                case TREASURE_TYPE_VITORIX:
                    item = 234 ;  /* Electric mace */
                    break ;
                case TREASURE_TYPE_JUGURTHA:
                    /* Ring of armor 1 */
                    item = 415 ;
                    break ;
                case TREASURE_TYPE_ELYMAS:
                    /* Ring of armor 3 */
                    item = 417 ;
                    break ;
                case TREASURE_TYPE_MATTAN:
                    /* Wand of mana drain */
                    item = 908 ;
                    break ;
                case TREASURE_TYPE_MAMMA_DRAGON:
                    /* Green Key to exit */
//                    item = 260 ;
                    break ;
                case TREASURE_TYPE_KOA_FOOTMAN:
                    break ;
                case TREASURE_TYPE_KOA_HORSEMAN:
                    break ;
                case TREASURE_TYPE_BRONZE_EVIL_KNIGHT:
                    item = 202 ;
                    break ;
                case TREASURE_TYPE_TROJAN:
                    item = 202 ;
                    break ;
                default:
                    break ;
            }

            /* No further stealing allowed */
            p_creature->wasStolenFrom = TRUE ;
        }
    }

    DebugEnd() ;

    return item ;
}

static T_void IDoEfxAtDeath(
                  T_creatureState *p_creature,
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    /* Turn off the area sound attached to this missile. */
    if (p_creature->areaSound != AREA_SOUND_BAD)  {
        AreaSoundDestroy(p_creature->areaSound) ;
        p_creature->areaSound = AREA_SOUND_BAD ;
    }

    /* Don't do an efx if it is zero. */
    if (p_logic->efxAtDeath != 0)  {
        EfxCreate( p_logic->efxAtDeath,
                   ObjectGetX(p_obj),
                   ObjectGetY(p_obj),
                   ObjectGetZ(p_obj) + (((T_sword32)ObjectGetHeight(p_obj))<<15),
                   1,
                   FALSE,
                   0) ;
    }
}

/* 05/20/96: Find the creature closest to the given x and y position. */
static T_word16 IFindClosestCreature(
                    T_sword16 x,
                    T_sword16 y,
                    T_creatureState *p_exclude)
{
    T_doubleLinkListElement element ;
    T_creatureState *p_creature ;
    T_word16 distance = 0xFFFF ;
    T_word16 closest = 0 ;
    T_word16 newDistance ;

    DebugRoutine("IFindClosestCreature") ;

    element = DoubleLinkListGetFirst(G_creatureList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        if (p_creature != p_exclude)  {
            if (!CreatureIsMissile(p_creature->p_obj))  {
                newDistance =
                    CalculateDistance(
                        x,
                        y,
                        ObjectGetX16(p_creature->p_obj),
                        ObjectGetY16(p_creature->p_obj)) ;
                if (newDistance < distance)  {
                    distance = newDistance ;
                    closest = ObjectGetServerId(p_creature->p_obj) ;
                }
            }
        }

        element = DoubleLinkListElementGetNext(element) ;
    }
    DebugEnd() ;

    return closest ;
}

/* 05/21/96: Make a creature go to a particular target.               */
T_void CreatureSetTarget(T_3dObject *p_obj, T_word16 targetID)
{
    T_3dObject *p_target ;
    T_creatureState *p_creature ;
    T_doubleLinkListElement element ;

    DebugRoutine("CreatureSetTarget") ;

    element = ICreatureFindViaObjectPtr(p_obj) ;
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        if (p_creature)  {
            p_target = ObjectFind(targetID) ;
            if (p_target)  {
                p_creature->targetID = targetID ;
#               ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
                fprintf(G_fp, "%d SetTarget: Creature %d target is now %d\n",
                    SyncTimeGet(),
                    p_creature->objectID,
                    p_creature->targetID) ;
//printf("%d SetTarget: Creature %d target is now %d\n", SyncTimeGet(), p_creature->objectID, p_creature->targetID) ;
#               endif
                p_creature->targetAcquired = TRUE ;
                p_creature->targetX = ObjectGetX16(p_target) ;
                p_creature->targetY = ObjectGetY16(p_target) ;
                p_creature->targetZ = ObjectGetZ16(p_target) ;
                Collide3dUpdateLineOfSightLast(
                    &p_creature->sight,
                    p_target) ;

                IUpdateTarget(p_creature) ;
            } else {
                p_creature->targetAcquired = FALSE ;
                p_creature->targetID = 0 ;
#               ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
                fprintf(G_fp, "%d SetTarget 0: Creature %d target is now %d\n",
                    SyncTimeGet(),
                    p_creature->objectID,
                    p_creature->targetID) ;
#               endif
            }
//printf("%d SetTarget 0: Creature %d target is now %d\n", SyncTimeGet(), p_creature->objectID, p_creature->targetID) ;
        } else {
            DebugCheck(FALSE) ;
        }
    }

    DebugEnd() ;
}

/* LES 07/31/96 Created */
static T_void ICreatureMakeSound(
                  T_3dObject *p_obj,
                  T_word16 soundNum)
{
    /* Make a sound centered on the creature. */
    if (soundNum)
        AreaSoundCreate(
            ObjectGetX16(p_obj),
            ObjectGetY16(p_obj),
            800,
            255,
            AREA_SOUND_TYPE_ONCE,
            0,
            AREA_SOUND_BAD,
            NULL,
            0,
            soundNum) ;
}

/* Make a creature take damage based on the sector(s) he is in */
static T_void CreatureTakeSectorDamage(
                  T_creatureLogic *p_logic,
                  T_3dObject *p_obj)
{
    T_word16 i ;
    T_word16 areaSector ;
    T_word16 damage ;
    T_word16 damageType ;

    DebugRoutine("CreatureTakeSectorDamage") ;

    for (i=0; i<ObjectGetNumAreaSectors(p_obj); i++)  {
        areaSector = ObjectGetNthAreaSector(p_obj, i) ;
        damage = G_3dSectorInfoArray[areaSector].damage ;
        damageType = G_3dSectorInfoArray[areaSector].damageType ;

        if ((damage != 0) && (damage != ((T_word16)0x8000)))  {
            if ((ObjectGetZ16(p_obj)) <= G_3dSectorArray[areaSector].floorHt) {
                if (damageType == EFFECT_DAMAGE_UNKNOWN)
                    damageType = EFFECT_DAMAGE_FIRE ;

                /* Take the damage unless the creature is allowed */
                /* on these type of sectors. */
                if (MapGetSectorType(areaSector) & (~p_logic->stayOnSectorType))  {
                    CreatureTakeDamage(p_obj, damage, damageType, 0) ;
                }
            }
        }
    }

    DebugEnd() ;
}

/* LES 09/10/96 */
T_void CreaturesHearSoundOfPlayer(T_3dObject *p_player, T_word16 distance)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_creatureState *p_creature ;
    T_creatureLogic *p_logic ;
    T_3dObject *p_obj ;

    DebugRoutine("CreaturesHearSoundOfPlayer") ;


    element = DoubleLinkListGetFirst(G_creatureList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        nextElement = DoubleLinkListElementGetNext(element) ;

        /* Get the creature state info, logic, and object. */
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        p_logic = p_creature->p_logic ;
        p_obj = p_creature->p_obj ;

        /* Don't do missiles. */
        if (!CreatureIsMissile(p_obj))  {
            /* Does this creature have a target already? */
            if (p_creature->targetAcquired == FALSE)  {
                /* No. */
//printf("Consider creature %d\n", ObjectGetServerId(p_obj)) ;
                /* Is this creature in the estimate distance? */

                if (CalculateEstimateDistance(
                       ObjectGetX16(p_player),
                       ObjectGetY16(p_player),
                       ObjectGetX16(p_obj),
                       ObjectGetY16(p_obj)) <= distance)  {
//puts("within range") ;
                    /* Within range. */
                    /* Is there a line of sight? */
                    if (Collide3dObjectToObjectCheckLineOfSight(
                        p_obj,
                        p_player) == FALSE)  {
//puts("has line of sight ... targetted") ;
                        /* Yes, there is a visual line of sight. */

                        /* This is our new target, go for it. */
                        p_creature->targetID = ObjectGetServerId(p_player) ;

#                       ifdef COMPILE_OPTION_CREATE_CRELOGIC_DATA_FILE
                        fprintf(G_fp, "%d Change: Creature %d hears %d\n",
                            SyncTimeGet(),
                            p_creature->objectID,
                            p_creature->targetID) ;
#                       endif

                        p_creature->targetAcquired = TRUE ;

                        /* Record the last sighted location. */
                        p_creature->targetX = ObjectGetX16(p_player) ;
                        p_creature->targetY = ObjectGetY16(p_player) ;
                        p_creature->targetZ = ObjectGetZ16(p_player) ;
                        Collide3dUpdateLineOfSightLast(
                            &p_creature->sight,
                            p_player) ;

                        IUpdateTarget(p_creature) ;
                    }
                }
            }
        }
        element = nextElement ;
    }

    DebugEnd() ;
}


T_word32 CreaturesKillAll(T_void)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_creatureState *p_creature ;
    T_word32 total = 0 ;

    DebugRoutine("CreaturesKillAll") ;

    /* Search until you find a match or find no more elements. */
    element = DoubleLinkListGetFirst(G_creatureList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        nextElement = DoubleLinkListElementGetNext(element) ;

        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;
        if (!CreatureIsMissile(p_creature->p_obj))
            total += p_creature->health ;

        IExplodeSelf(p_creature, p_creature->p_logic, p_creature->p_obj, 0) ;

        element = nextElement ;
    }

    DebugEnd() ;

    return total ;
}

T_void CreaturesAllOnOneTarget(T_word16 targetId)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_creatureState *p_creature ;

    DebugRoutine("CreaturesAllOnOneTarget") ;

    /* Search until you find a match or find no more elements. */
    element = DoubleLinkListGetFirst(G_creatureList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        nextElement = DoubleLinkListElementGetNext(element) ;

        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;

        p_creature->targetID = targetId ;
        p_creature->targetAcquired = TRUE ;

        element = nextElement ;
    }

    DebugEnd() ;
}

#ifndef NDEBUG
T_void CreaturesCheck(T_void)
{
    T_doubleLinkListElement element ;
    T_doubleLinkListElement nextElement ;
    T_creatureState *p_creature ;
    T_3dObject *p_obj ;

    DebugRoutine("CreaturesCheck") ;

    /* Search until you find a match or find no more elements. */
    if (G_creatureList != DOUBLE_LINK_LIST_BAD)   {
        element = DoubleLinkListGetFirst(G_creatureList) ;
        while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            nextElement = DoubleLinkListElementGetNext(element) ;

            p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;

            DebugCheck(p_creature != NULL) ;

            p_obj = p_creature->p_obj ;
            DebugCheck(p_obj != NULL) ;
            DebugCheck(ObjectGetServerId(p_obj) != 0) ;

            element = nextElement ;
        }
    }

    DebugEnd() ;
}
#endif

T_void CreatureGoSplat(
           T_3dObject *p_obj,
           T_word16 amount,
           T_word16 damageType)
{
    T_doubleLinkListElement element ;
    T_creatureState *p_creature ;

    DebugRoutine("CreatureGoSplat") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(ObjectGetExtraData(p_obj) != NULL) ;

    element = ICreatureFindViaObjectPtr(p_obj) ;
    if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Yes, creature is here to go splat. */
        p_creature = (T_creatureState *)DoubleLinkListElementGetData(element) ;

        /* Take out any resistances (if not special) */
        if ((damageType & EFFECT_DAMAGE_SPECIAL) == 0)
            damageType &= (~(p_creature->p_logic->damageResist)) ;

        if (damageType != 0)  {
            /* Go ahead and go splat on everything else */
            ClientMakeObjectGoSplat(
                p_obj,
                amount,
                (E_effectDamageType)damageType,
                0,
                p_creature->p_logic->bloodEffect) ;
        }
    }
    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  CRELOGIC.C
 *-------------------------------------------------------------------------*/
