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
#define CRELOGIC_DFT_TREASURE      0
#define CRELOGIC_DFT_VISION_FIELD  INT_ANGLE_90
#define CRELOGIC_DFT_WANDER_SOUND  4005
#define CRELOGIC_DFT_ATTACK_SOUND  5014
#define CRELOGIC_DFT_HURT_SOUND    5013
#define CRELOGIC_DFT_TARGET_SOUND  4001
#define CRELOGIC_DFT_DIE_SOUND     5015
#define CRELOGIC_DFT_FACE_DELAY    6
#define CRELOGIC_DFT_EXPLODE_ON_COLILDE   FALSE
#define CRELOGIC_DFT_CAN_HURT_SELF        FALSE


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
    TARGET_LOGIC_PACKAGE_UNKNOWN
} E_targetLogicPackage ;

typedef T_void (*T_monsterUpdateCallback)(T_void *p_data) ;

typedef enum {
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
    T_word32 hurtSound;
    T_word32 targetSound;
    T_word32 dieSound;
    T_word16 faceDelay ;
    E_Boolean explodeOnCollision ;
    E_Boolean canHurtSelf ;
} T_creatureLogic ;

