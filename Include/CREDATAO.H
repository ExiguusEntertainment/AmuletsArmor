static T_creatureLogic G_creatureLogics[] =
{
/*      DEFAULT TEMPLATE
        --------------------------------------------------------------
    {
        1000,                           //objectType
        NAV_LOGIC_PKG_NEUTRAL_WANDERER, //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        CRELOGIC_DFT_MELEE_DAMAGE,      //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self 
    },
        ---------------------------------------------------------------
*/
    {                                   //stefan
        1001,                           //objectType
        NAV_LOGIC_PKG_NEUTRAL_WANDERER, //monster logic package type
        TARGET_LOGIC_PACKAGE_NONE,      //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        CRELOGIC_DFT_MELEE_DAMAGE,      //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //blue wizard
        1002,                           //objectType
        NAV_LOGIC_PKG_BARBARIAN_ARCHER, //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        0,                              //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        EFFECT_MISSILE_FIREBALL,        //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        CRELOGIC_DFT_MELEE_DAMAGE,      //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //poltergeist
        1003,                           //objectType
        NAV_LOGIC_PKG_BERSERKER_A, //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        TRUE,                           //can fly
        TRUE,                           //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        0,                              //minimum missile range
        0,                              //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        EFFECT_MISSILE_FIREBALL,        //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        100,                            //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //wyvern
        1005,                           //objectType
        NAV_LOGIC_PKG_ROAMING_GUARD,    //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        EFFECT_MISSILE_LIGHTNING_BOLT,  //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        600,                            //amount of melee damage done per hit
        NULL,                           //special update callback routine
        EFFECT_DAMAGE_ALL,              //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //orange wizard
        1006,                           //objectType
        NAV_LOGIC_PKG_COMBO_FIGHTER_ARCHER,//monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        EFFECT_MISSILE_ACID_BALL,       //type of missile to shoot
        EFFECT_DAMAGE_ACID,             //type of damage done in melee
        CRELOGIC_DFT_MELEE_DAMAGE,      //amount of melee damage done per hit
        NULL,//CrelogicSummonCreature?? //special update callback routine
        EFFECT_DAMAGE_ACID,             //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //gargoyle
        1007,                           //objectType
        NAV_LOGIC_PKG_BERSERKER_A,      //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        0,                              //minimum missile range
        0,                              //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        EFFECT_DAMAGE_POISON,           //type of damage done in melee
        150,                            //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //elk
        1016,                           //objectType
        NAV_LOGIC_PKG_SCAREDY_CAT,      //monster logic package type
        TARGET_LOGIC_PACKAGE_STANDARD_HIT_OR_SHOOT, //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        CRELOGIC_DFT_MELEE_DAMAGE,      //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //Druid -- TESTING WALKING B
        1019,                           //objectType
        NAV_LOGIC_PKG_COMBO_FIGHTER_ARCHER,  //monster logic package type
        TARGET_LOGIC_PACKAGE_SUICIDE_EXPLOSION,     //monster target pkg
        FALSE,                          //can fly
        FALSE,                          //ignore walls
        CRELOGIC_DFT_FLY_ACCELERATION,  //monster acceleration for flying
        CRELOGIC_DFT_FLY_MAX_VELOCITY,  //maximum velocity for flying
        CRELOGIC_DFT_UPDATE_TIME,       //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        CRELOGIC_DFT_MAX_MELEE_RANGE,   //maximum melee hit range
        CRELOGIC_DFT_MIN_MISSILE_RANGE, //minimum missile range
        CRELOGIC_DFT_MAX_MISSILE_RANGE, //maximum missile range
        CRELOGIC_DFT_MELEE_DELAY,       //number of updates per melee atk
        CRELOGIC_DFT_MISSILE_DELAY,     //number of updates per missile atk
        CRELOGIC_DFT_HIT_POINTS,        //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        EFFECT_DAMAGE_POISON,           //type of damage done in melee
        150,                            //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        CRELOGIC_DFT_EXPLODE_ON_COLILDE,//TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
    {                                   //Druid -- TESTING MISSILE
        4011,                           //objectType
        NAV_LOGIC_PKG_NONE,             //monster logic package type
        TARGET_LOGIC_PACKAGE_NONE,      //monster target pkg
        TRUE,                           //can fly
        FALSE,                          //ignore walls
        30,                             //monster acceleration for flying
        30,                             //maximum velocity for flying
        0,                              //time between creature updates
        CRELOGIC_DFT_STEP_SIZE,         //number of pixels per step
        100,                            //maximum melee hit range
        0,                              //minimum missile range
        0,                              //maximum missile range
        0,                              //number of updates per melee atk
        0,                              //number of updates per missile atk
        32767,                          //amount of health this creature has
        CRELOGIC_DFT_REGEN_RATE,        //amount healed per update
        CRELOGIC_DFT_MISSILE_TYPE,      //type of missile to shoot
        CRELOGIC_DFT_DAMAGE_TYPE,       //type of damage done in melee
        100,                            //amount of melee damage done per hit
        NULL,                           //special update callback routine
        CRELOGIC_DFT_DAMAGE_RESIST,     //special dmg resistances for monster
        CRELOGIC_DFT_HURT_HEALTH,       //below this health creatures runs
        CRELOGIC_DFT_ARMOR_TYPE,        //type of armor for elec. based attacks
        CRELOGIC_DFT_TREASURE,          //type of treasure carried
        CRELOGIC_DFT_VISION_FIELD,      //angular area monster can view
        CRELOGIC_DFT_WANDER_SOUND,      //sound to use while wandering
        CRELOGIC_DFT_ATTACK_SOUND,      //sound to use when attacking
        CRELOGIC_DFT_HURT_SOUND,        //sound to use when hit
        CRELOGIC_DFT_TARGET_SOUND,      //'recognise' sound effect
        CRELOGIC_DFT_DIE_SOUND,         //sound to use when killed
        CRELOGIC_DFT_FACE_DELAY,        //number updates before face opponent
        TRUE,                           //TRUE=explode on collision, else FALSE
        CRELOGIC_DFT_CAN_HURT_SELF,     //TRUE=missile and melee damage hurts self
    },
};

