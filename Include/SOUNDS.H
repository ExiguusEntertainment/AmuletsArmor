/* Sounds in the current sound file */
#define SOUND_SWING_SET1                4000
#define SOUND_SWING_SET2                4003
#define SOUND_SWING_SET3                4006
#define SOUND_SWING_SET4                4045

#define SOUND_XBOW                      4021
#define SOUND_SWORDHIT                  4012
#define SOUND_DAGGERHIT                 4015
#define SOUND_MACEHIT                   4018
#define SOUND_FISTHIT                   4042

#define SOUND_EARTHQUAKE                3005
#define SOUND_MUNCH                     6000

#define SOUND_PALADIN_HURT_SET          4023
#define SOUND_ROGUE_HURT_SET            4027
#define SOUND_MAGE_HURT_SET             4031
#define SOUND_CITIZEN_HURT_SET          4035
#define SOUND_PALADIN_WAR_CRY           4022
#define SOUND_ROGUE_WAR_CRY             4026
#define SOUND_MAGE_WAR_CRY              4030
#define SOUND_CITIZEN_WAR_CRY           4034

#define SOUND_SPELL_FIZZLE              2013

#define SOUND_PALADIN_JUMP              4038
#define SOUND_THIEF_JUMP                4039
#define SOUND_CITIZEN_JUMP              4040
#define SOUND_MAGE_JUMP                 4041

#define SOUND_SHEATH_LONGSWD            4010
#define SOUND_SHEATH_SHTSWD             4009
#define SOUND_SHEATH_DAGGER             4011

#define SOUND_BUTTON_UP                 6003
#define SOUND_BUTTON_DOWN               6004
#define SOUND_SOMETHING_BREAKING        6005
#define SOUND_SOMETHING_SHATTERING      6006
#define SOUND_NOTE_PAGE                 6016

#define SOUND_DOOR_1                    5000
#define SOUND_DOOR_2                    5001
#define SOUND_DOOR_3                    5002

#define SOUND_TELEPORT                  6001

#define SOUND_DING                      6011

#define SOUND_PLAYER_WAR_CRY_SET        9000
#define SOUND_PLAYER_HURT_SET           9010
#define SOUND_PLAYER_HURTBAD_SET        9014
#define SOUND_PLAYER_JUMP_SET           9020
#define SOUND_PLAYER_CONFUSED           9031
#define SOUND_PLAYER_POISONED           9033
#define SOUND_PLAYER_PISSED             9034
#define SOUND_PLAYER_DRAINED            9030
#define SOUND_PLAYER_DEAD               9032

#define SOUND_SPELL_CLERIC_A_SET        2020
#define SOUND_SPELL_CLERIC_B_SET        2029
#define SOUND_SPELL_MAGE_A_SET          2038
#define SOUND_SPELL_MAGE_B_SET          2047


/* currently bad mappings */

#define SOUND_NONE                      0
#define SOUND_PUNCH                     9
#define SOUND_PUNCH2                    1035
#define SOUND_CRUNCH                    4026
#define SOUND_SMASH                     4027
#define SOUND_HIT                       1034
#define SOUND_CLANG                     4019

#define SOUND_ELECTRIC1                 4022
#define SOUND_ELECTRIC2                 4023
#define SOUND_LATCH                     1036

#define SoundDing()                     \
              SoundPlayByNumber(SOUND_DING, VOLUME_LEVEL_100);

