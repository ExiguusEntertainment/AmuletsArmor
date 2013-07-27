/****************************************************************************/
/*    FILE:  KEYMAP.H                                                       */
/****************************************************************************/
#ifndef _KEYMAP_H_
#define _KEYMAP_H_

#include "INIFILE.H"

extern T_byte8 G_keyMap[256] ;

#define KeyMap(v)    (G_keyMap[v])

#define KeyMapGetScan(v)  (KeyboardGetScanCode(G_keyMap[v]))
#if _WIN32
    #define KeyMapGetHeld(v)  ((KeyboardGetScanCode(G_keyMap[v]))?delta:0)
#else
    #define KeyMapGetHeld(v)  (KeyboardGetHeldTimeAndClear(G_keyMap[v]))
#endif

#define KEYMAP_FORWARD                      0
#define KEYMAP_BACKWARD                     1
#define KEYMAP_TURN_LEFT                    2
#define KEYMAP_TURN_RIGHT                   3
#define KEYMAP_SIDESTEP                     4
#define KEYMAP_RUN                          5
#define KEYMAP_JUMP                         6
#define KEYMAP_ACTIVATE                     7
#define KEYMAP_OPEN                         7
#define KEYMAP_ATTACK                       8
#define KEYMAP_USE                          8
#define KEYMAP_LOOK_UP                      9
#define KEYMAP_LOOK_DOWN                    10
#define KEYMAP_READJUST_VIEW                11
#define KEYMAP_CAST_SPELL                   12
#define KEYMAP_ABORT_SPELL                  13
#define KEYMAP_RUNE1                        14
#define KEYMAP_RUNE2                        15
#define KEYMAP_RUNE3                        16
#define KEYMAP_RUNE4                        17
#define KEYMAP_RUNE5                        18
#define KEYMAP_RUNE6                        19
#define KEYMAP_RUNE7                        20
#define KEYMAP_RUNE8                        21
#define KEYMAP_RUNE9                        22
#define KEYMAP_LOOK_MODE                    23
#define KEYMAP_INVENTORY                    24
#define KEYMAP_EQUIPMENT                    25
#define KEYMAP_STATISTICS                   26
#define KEYMAP_OPTIONS                      27
#define KEYMAP_COMMUNICATE                  28
#define KEYMAP_FINANCES                     29
#define KEYMAP_AMMUNITION                   30
#define KEYMAP_NOTES                        31
#define KEYMAP_JOURNAL                      32
#define KEYMAP_TALK                         33
#define KEYMAP_ABORT_LEVEL                  34
#define KEYMAP_PAUSE_GAME                   35
#define KEYMAP_CANNED_MESSAGE_1             36
#define KEYMAP_CANNED_MESSAGE_2             37
#define KEYMAP_CANNED_MESSAGE_3             38
#define KEYMAP_CANNED_MESSAGE_4             39
#define KEYMAP_CANNED_MESSAGE_5             40
#define KEYMAP_CANNED_MESSAGE_6             41
#define KEYMAP_CANNED_MESSAGE_7             42
#define KEYMAP_CANNED_MESSAGE_8             43
#define KEYMAP_CANNED_MESSAGE_9             44
#define KEYMAP_CANNED_MESSAGE_10            45
#define KEYMAP_CANNED_MESSAGE_11            46
#define KEYMAP_CANNED_MESSAGE_12            47
#define KEYMAP_MAP_DISPLAY                  48
#define KEYMAP_MAP_ROTATE                   49
#define KEYMAP_MAP_TRANSLUCENT              50
#define KEYMAP_MAP_SEE_THROUGH              51
#define KEYMAP_MAP_SMALLER_SCALE            52
#define KEYMAP_MAP_BIGGER_SCALE             53
#define KEYMAP_MAP_POSITION                 54

#define KEYMAP_SCROLL_MSG_UP                55
#define KEYMAP_SCROLL_MSG_DOWN              56
#define KEYMAP_TIME_OF_DAY                  57

#define KEYMAP_GAMMA_CORRECT                58
#define KEYMAP_UNKNOWN_EXTRA                59
#define KEYMAP_ACTIVATE_OR_TAKE             7

#define KEYMAP_NUM_KEYS_MAPPED              68

T_void KeyMapInitialize(T_iniFile iniFile);
T_void KeyMapFinish(T_void);
T_void KeyMapReinitialize(T_iniFile iniFile);

#endif /* _KEYMAP_H_ */

/****************************************************************************/
/*    END OF FILE:  KEYMAP.H                                                */
/****************************************************************************/

