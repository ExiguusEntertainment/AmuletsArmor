#include "BANNER.H"
#include "BUTTON.H"
#include "CLIENT.H"
#include "CONFIG.H"
#include "CSYNCPCK.H"
#include "ESCMENU.H"
#include "FORM.H"
#include "INIFILE.H"
#include "KEYBOARD.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"
#include "MOUSEMOD.H"
#include "SPELLS.H"
#include "TEXT.H"
#include "TXTFLD.H"
#include "VIEW.H"

/*---------------------------------------------------------------------------
 * Constants:
 *--------------------------------------------------------------------------*/
#define NUMBER_KEY_PAGES 5
#define NUMBER_KEYS (NUMBER_KEY_PAGES*12)

#define ESC_MENU_OPTION_BOBBING             0
#define ESC_MENU_INVERT_MOUSE_Y             1
#define ESC_MENU_MOUSE_TURN_SPEED           2
#define ESC_MENU_KEYBOARD_TURN_SPEED        3
#define ESC_MENU_DYING_DROPS_ALL_ITEMS      4
#define ESC_MENU_NUM_OPTIONS                5

/*---------------------------------------------------------------------------
 * Types:
 *--------------------------------------------------------------------------*/
typedef struct {
    T_byte8 scankey ;
    const T_byte8 *p_string ;
} T_keyToStr ;

/*---------------------------------------------------------------------------
 * Prototypes:
 *--------------------------------------------------------------------------*/
void EscapeMenuEnterState(T_byte8 aState);
void EscapeMenuExitState(void);
void EscapeMenuGotoState(T_byte8 aState);
static const T_byte8 *EscMenuKeyToString(T_byte8 scankey);
static void EscapeMenuSetOptions(void);

/*---------------------------------------------------------------------------
 * Globals:
 *--------------------------------------------------------------------------*/
static T_iniFile G_iniFile = INIFILE_BAD ;
static E_Boolean G_escMenuIsOpen = FALSE;
static T_keyboardEventHandler G_previousKeyHandler;
static E_Boolean G_escKeyPressed = FALSE;
static T_byte8 G_keys[KEYMAP_NUM_KEYS_MAPPED] ;
static T_byte8 G_escMenuState = 0;
static T_byte8 G_escMenuDepth = 0;
static T_byte8 G_escMenuStateStack[10];
static T_word16 G_escMenuAssignKeyMap;
static E_Boolean G_escMenuAssignMode = FALSE;
static T_byte8 G_escMenuToggleOptions[ESC_MENU_NUM_OPTIONS];
static T_byte8 G_escMenuAbortCount = 0;

static const T_byte8 *G_keyboardMapping[] = {
    "Forward",
    "Backward",
    "Turn left",
    "Turn right",
    "Sidestep",
    "Run",
    "Jump",
    "Open door/activate wall",
    "Attack/Use readied item",
    "Look up",
    "Look down",
    "Readjust view",
    "Cast spell",
    "Abort spell",
    "Spell rune 1",
    "Spell rune 2",
    "Spell rune 3",
    "Spell rune 4",
    "Spell rune 5",
    "Spell rune 6",
    "Spell rune 7",
    "Spell rune 8",
    "Spell rune 9",
    "",
    "Inventory box        ALT+",
    "Equipment box        ALT+",
    "Statistics box       ALT+",
    "Options box          ALT+",
    "Communicate box      ALT+",
    "Finances box         ALT+",
    "Ammunition box       ALT+",
    "Notes box            ALT+",
    "Journal box          ALT+",
    "Talk",
    "Abort Level",
    "Pause game",
    "Canned message 1",
    "Canned message 2",
    "Canned message 3",
    "Canned message 4",
    "Canned message 5",
    "Canned message 6",
    "Canned message 7",
    "Canned message 8",
    "Canned message 9",
    "Canned message 10",
    "Canned message 11",
    "Canned message 12",
    "Map display",
    "Map rotate",
    "Map translucent",
    "Map see through",
    "Map smaller scale",
    "Map bigger scale",
    "Map position",
    "Scroll messages up",
    "Scroll messages down",
    "Time of day",
    "Gamma correct        ALT+",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
} ;

static const T_byte8 *G_keyboardMapping2[] = {
    "Forward\t\t",
    "Backward\t\t",
    "Turn left\t\t",
    "Turn right\t",
    "Sidestep\t\t",
    "Run\t\t\t",
    "Jump\t\t",
    "Open/activate\t",
    "Attack/use\t",
    "Look up\t\t",
    "Look down\t",
    "Center view\t",
    "Cast spell\t",
    "Abort spell\t",
    "Spell rune 1\t",
    "Spell rune 2\t",
    "Spell rune 3\t",
    "Spell rune 4\t",
    "Spell rune 5\t",
    "Spell rune 6\t",
    "Spell rune 7\t",
    "Spell rune 8\t",
    "Spell rune 9\t",
    "",
    "INV menu ^003ALT\t",
    "EQP menu ^003ALT\t",
    "STA menu ^003ALT\t",
    "OPT menu ^003ALT\t",
    "COM menu ^003ALT\t",
    "FIN menu ^003ALT\t",
    "AMO menu ^003ALT\t",
    "NTS menu ^003ALT\t",
    "JNL menu ^003ALT\t",
    "Talk\t\t\t",
    "Abort Level\t",
    "Pause game\t",
    "Canned msg 1\t",
    "Canned msg 2\t",
    "Canned msg 3\t",
    "Canned msg 4\t",
    "Canned msg 5\t",
    "Canned msg 6\t",
    "Canned msg 7\t",
    "Canned msg 8\t",
    "Canned msg 9\t",
    "Canned msg 10\t",
    "Canned msg 11\t",
    "Canned msg 12\t",
    "Map display\t",
    "Map rotate\t",
    "Map trnslucnt\t",
    "Map see thru\t",
    "Map smaller\t",
    "Map bigger\t",
    "Map position\t",
    "Msgs up\t\t",
    "Msgs down\t",
    "Time of day\t",
    "Gamma ^003ALT\t",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
} ;

static T_keyToStr  G_keyToStr[] = {
    { KEY_SCAN_CODE_ESC               ,"ESC"       },
    { KEY_SCAN_CODE_1                 ,"1"       },
    { KEY_SCAN_CODE_2                 ,"2"       },
    { KEY_SCAN_CODE_3                 ,"3"       },
    { KEY_SCAN_CODE_4                 ,"4"       },
    { KEY_SCAN_CODE_5                 ,"5"       },
    { KEY_SCAN_CODE_6                 ,"6"       },
    { KEY_SCAN_CODE_7                 ,"7"       },
    { KEY_SCAN_CODE_8                 ,"8"       },
    { KEY_SCAN_CODE_9                 ,"9"       },
    { KEY_SCAN_CODE_0                 ,"0"       },
    { KEY_SCAN_CODE_MINUS             ,"-"       },
    { KEY_SCAN_CODE_EQUAL             ,"="       },
    { KEY_SCAN_CODE_BACKSPACE         ,"<--"       },
    { KEY_SCAN_CODE_TAB               ,"TAB"       },
    { KEY_SCAN_CODE_Q                 ,"Q"       },
    { KEY_SCAN_CODE_W                 ,"W"       },
    { KEY_SCAN_CODE_E                 ,"E"       },
    { KEY_SCAN_CODE_R                 ,"R"       },
    { KEY_SCAN_CODE_T                 ,"T"       },
    { KEY_SCAN_CODE_Y                 ,"Y"       },
    { KEY_SCAN_CODE_U                 ,"U"       },
    { KEY_SCAN_CODE_I                 ,"I"       },
    { KEY_SCAN_CODE_O                 ,"O"       },
    { KEY_SCAN_CODE_P                 ,"P"       },
    { KEY_SCAN_CODE_SB_OPEN           ,"["       },
    { KEY_SCAN_CODE_SB_CLOSE          ,"]"       },
    { KEY_SCAN_CODE_ENTER             ,"ENTER"       },
    { KEY_SCAN_CODE_CTRL              ,"CTRL"       },
    { KEY_SCAN_CODE_A                 ,"A"       },
    { KEY_SCAN_CODE_S                 ,"S"       },
    { KEY_SCAN_CODE_D                 ,"D"       },
    { KEY_SCAN_CODE_F                 ,"F"       },
    { KEY_SCAN_CODE_G                 ,"G"       },
    { KEY_SCAN_CODE_H                 ,"H"       },
    { KEY_SCAN_CODE_J                 ,"J"       },
    { KEY_SCAN_CODE_K                 ,"K"       },
    { KEY_SCAN_CODE_L                 ,"L"       },
    { KEY_SCAN_CODE_SEMI_COLON        ,";"       },
    { KEY_SCAN_CODE_APOSTROPHE        ,"'"       },
    { KEY_SCAN_CODE_GRAVE             ,"`"       },
    { KEY_SCAN_CODE_LEFT_SHIFT        ,"SHIFT"       },
    { KEY_SCAN_CODE_BACKSLASH         ,"\\"       },
    { KEY_SCAN_CODE_Z                 ,"Z"       },
    { KEY_SCAN_CODE_X                 ,"X"       },
    { KEY_SCAN_CODE_C                 ,"C"       },
    { KEY_SCAN_CODE_V                 ,"V"       },
    { KEY_SCAN_CODE_B                 ,"B"       },
    { KEY_SCAN_CODE_N                 ,"N"       },
    { KEY_SCAN_CODE_M                 ,"M"       },
    { KEY_SCAN_CODE_COMMA             ,","       },
    { KEY_SCAN_CODE_PERIOD            ,"."       },
    { KEY_SCAN_CODE_SLASH             ,"/"       },
    { KEY_SCAN_CODE_RIGHT_SHIFT       ,"SHIFT"       },
    { KEY_SCAN_CODE_STAR              ,"*"       },
    { KEY_SCAN_CODE_ALT               ,"ALT"       },
    { KEY_SCAN_CODE_SPACE             ,"SPACE"       },
    { KEY_SCAN_CODE_CAPS_LOCK         ,"CAPLK"       },
    { KEY_SCAN_CODE_NUM_LOCK          ,"NUMLK"       },
    { KEY_SCAN_CODE_SCROLL_LOCK       ,"SCRLK"       },
    { KEY_SCAN_CODE_UNUSED1           ,"????"       },
    { KEY_SCAN_CODE_UNUSED2           ,"????"       },
    { KEY_SCAN_CODE_UNUSED3           ,"????"       },
    { KEY_SCAN_CODE_KEYPAD_0          ,"KPAD0"       },
    { KEY_SCAN_CODE_KEYPAD_1          ,"KPAD1"       },
    { KEY_SCAN_CODE_KEYPAD_2          ,"KPAD2"       },
    { KEY_SCAN_CODE_KEYPAD_3          ,"KPAD3"       },
    { KEY_SCAN_CODE_KEYPAD_4          ,"KPAD4"       },
    { KEY_SCAN_CODE_KEYPAD_5          ,"KPAD5"       },
    { KEY_SCAN_CODE_KEYPAD_6          ,"KPAD6"       },
    { KEY_SCAN_CODE_KEYPAD_7          ,"KPAD7"       },
    { KEY_SCAN_CODE_KEYPAD_8          ,"KPAD8"       },
    { KEY_SCAN_CODE_KEYPAD_9          ,"KPAD9"       },
    { KEY_SCAN_CODE_KEYPAD_PERIOD     ,"KPAD."       },
    { KEY_SCAN_CODE_KEYPAD_NUM_LOCK   ,"KNUML"       },
    { KEY_SCAN_CODE_KEYPAD_SLASH      ,"KPAD/"       },
    { KEY_SCAN_CODE_KEYPAD_STAR       ,"KPAD*"       },
    { KEY_SCAN_CODE_KEYPAD_MINUS      ,"KPAD-"       },
    { KEY_SCAN_CODE_KEYPAD_PLUS       ,"KPAD+"       },
    { KEY_SCAN_CODE_KEYPAD_ENTER      ,"K_ENT"       },
    { KEY_SCAN_CODE_KEYPAD_DELETE     ,"K_DEL"       },
    { KEY_SCAN_CODE_KEYPAD_INSERT     ,"K_INS"       },
    { KEY_SCAN_CODE_F1                ,"F1"       },
    { KEY_SCAN_CODE_F2                ,"F2"       },
    { KEY_SCAN_CODE_F3                ,"F3"       },
    { KEY_SCAN_CODE_F4                ,"F4"       },
    { KEY_SCAN_CODE_F5                ,"F5"       },
    { KEY_SCAN_CODE_F6                ,"F6"       },
    { KEY_SCAN_CODE_F7                ,"F7"       },
    { KEY_SCAN_CODE_F8                ,"F8"       },
    { KEY_SCAN_CODE_F9                ,"F9"       },
    { KEY_SCAN_CODE_F10               ,"F10"       },
    { KEY_SCAN_CODE_F11               ,"F11"       },
    { KEY_SCAN_CODE_F12               ,"F12"       },
    { KEY_SCAN_CODE_UP                ,"UP"       },
    { KEY_SCAN_CODE_DOWN              ,"DOWN"       },
    { KEY_SCAN_CODE_LEFT              ,"LEFT"       },
    { KEY_SCAN_CODE_RIGHT             ,"RIGHT"       },
    { KEY_SCAN_CODE_HOME              ,"HOME"       },
    { KEY_SCAN_CODE_PGUP              ,"PGUP"       },
    { KEY_SCAN_CODE_END               ,"END"       },
    { KEY_SCAN_CODE_PGDN              ,"PGDN"       },
    { KEY_SCAN_CODE_INSERT            ,"INS"       },
    { KEY_SCAN_CODE_DELETE            ,"DEL"       },
    { KEY_SCAN_CODE_MACRO             ,"MACRO"       },
    { KEY_SCAN_CODE_PAUSE             ,"PAUSE"       },
    { 0, "NONE" },
    { 0, "NONE" }
} ;

/*---------------------------------------------------------------------------
 * Routines:
 *--------------------------------------------------------------------------*/

T_void EscapeMenuFormControl(
        E_formObjectType objtype,
        T_word16 objstatus,
        T_word32 objID)
{
    T_TxtboxID textID;

    DebugRoutine("EscapeMenuFormControl");

//    printf("ESC: %d %d %d\n", objtype, objstatus, objID);

    if ((objtype == FORM_OBJECT_BUTTON) && (objstatus == 2)) {
        if ((objID >= 4001) && (objID <= 4009)) {
            // Go to a menu
            EscapeMenuGotoState(objID - 4000);
        } else if ((objID >= 2001) && (objID <= 2009)) {
            // Enter pages
            EscapeMenuEnterState(objID - 2000);
        } else if ((objID >= 1000) && (objID <= 1999)) {
            // Assign a key by enering into it and putting
            // the ESC code into an assignment mode
            G_escMenuAssignKeyMap = objID - 1000;
            EscapeMenuEnterState(6);
            G_escMenuAssignMode = TRUE;
        } else if ((objID >= 7000) && (objID < (7000+ESC_MENU_NUM_OPTIONS))) {
            if (objID == 7000+ESC_MENU_MOUSE_TURN_SPEED) {
                // Set the range from 1 to 10 with 5 as the default
                G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED]+=10;
                if (G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] > 200)
                    G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] = 20;
                EscapeMenuSetOptions();
            } else if (objID == 7000+ESC_MENU_KEYBOARD_TURN_SPEED) {
                    // Set the range from 1 to 10 with 5 as the default
                    G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED]+=10;
                    if (G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] > 200)
                        G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] = 20;
                    EscapeMenuSetOptions();
            } else {
                // On/Off option hit
                G_escMenuToggleOptions[objID - 7000] ^= 1;
                EscapeMenuSetOptions();
            }
        } else if (objID == 301) {
            // Pressed the "No Key" button
            // Assign a zero for no key selected
            G_keys[G_escMenuAssignKeyMap] = 0;

            // End the assignment state
            EscapeMenuExitState();
            G_escMenuAssignMode = FALSE;
        } else if (objID == 300) {
            EscapeMenuExitState();
        } else if (objID == 5000) {
            EscapeMenuEnterState(8);
            G_escMenuAbortCount = 1;
        } else if (objID == 5001) {
            textID = FormFindObjID(5002);
            if (G_escMenuAbortCount==1) {
                ClientSyncSendActionAbortLevel();
                TxtboxSetData(textID, "Requesting to abort level.");
                G_escMenuAbortCount = 2;
            } else if (G_escMenuAbortCount == 2) {
                TxtboxSetData(textID, "Hit once more to force an abort...");
                G_escMenuAbortCount = 3;
            } else if (G_escMenuAbortCount == 3) {
                TxtboxSetData(textID, "Forced abort!");
                G_escMenuAbortCount = 4;

                /* Declare this as no longer an adventure */
                ClientSetAdventureNumber(0);

                /* Return to guild. */
                MouseRelativeModeOff();
                ClientSetNextPlace(20004, 0);
            }
        } else if (objID == 5999) {
            // Directly stop immediately and once
            ClientSyncSendActionAbortLevel();
        }
    }

    DebugEnd();
}

void EscapeMenuKeyHandler(E_keyboardEvent event, T_word16 scankey)
{
    T_byte8 i;

    DebugRoutine("EscapeMenuKeyHandler");

//    printf("ESC-Key: %d %d\n", event, scankey);

    switch (event) {
        case KEYBOARD_EVENT_RELEASE:
            if (scankey == KEY_SCAN_CODE_ESC) {
                if (G_escKeyPressed) {
                    EscapeMenuExitState();
                    G_escKeyPressed = FALSE;
                }
            }
            break;
        case KEYBOARD_EVENT_PRESS:
            G_escKeyPressed = TRUE;
            if (G_escMenuAssignMode) {
                // Only assign if NOT escape key
                if (scankey != KEY_SCAN_CODE_ESC) {
                    // Unassign any keys already assigned with this
                    for (i = 0; i < sizeof(G_keys) / sizeof(G_keys[0]); i++) {
                        if (G_keys[i] == scankey) {
                            if ((strstr(G_keyboardMapping[i], "ALT+")==0) == (strstr(G_keyboardMapping[G_escMenuAssignKeyMap], "ALT+")==0)) {
                                G_keys[i] = 0;
                            }
                        }
                    }

                    // Assign the new key
                    G_keys[G_escMenuAssignKeyMap] = (T_byte8)scankey;

                    // End the assignment state
                    EscapeMenuExitState();
                    G_escMenuAssignMode = FALSE;
                }
            }
            break;
        default:
            break;
    }

    DebugEnd();
}

static const T_byte8 *EscMenuKeyToString(T_byte8 scankey)
{
    T_word16 i;
    const T_byte8 *p_string = "???";

    DebugRoutine("EscMenuKeyToString");

    i = 0;
    do {
        if (scankey == G_keyToStr[i].scankey) {
            p_string = G_keyToStr[i].p_string;
            break;
        }
    } while (G_keyToStr[i++].scankey);

    DebugEnd();

    return p_string;
}

static T_void EscMenuSaveKeys(T_void)
{
    T_word16 i, j;
    T_byte8 buffer[NUMBER_KEY_PAGES * 12 * 3 + 1];
    FILE *fp;
    j = 0;

    DebugRoutine("EscMenuSaveKeys");

    for (i = 0; (i < NUMBER_KEY_PAGES * 12) && (i < 34); i++) {
        sprintf(buffer + j, "%02X", G_keys[i]);
        j += 2;
    }
    INIFilePut(G_iniFile, "keyboard", "keys1", buffer);
    j = 0;
    buffer[0] = '\0';
    for (; (i < KEYMAP_NUM_KEYS_MAPPED); i++) {
        sprintf(buffer + j, "%02X", G_keys[i]);
        j += 2;
    }
    INIFilePut(G_iniFile, "keyboard", "keys2", buffer);

    fp = fopen("control.txt", "w");
    fprintf(fp, "\n^036   Amulets and Armor\n^003   Keyboard Commands\n^007\n");
    for (i = 0; i < NUMBER_KEY_PAGES * 12; i++) {
        if (G_keyboardMapping2[i][0]) {
            fprintf(fp, "%s^009:%s^007\n", G_keyboardMapping2[i],
                    EscMenuKeyToString(G_keys[i]));
        }
    }
//    fprintf(fp, "-- END OF LIST --") ;
    fclose(fp);

    DebugEnd();
}

static T_void EscMenuLoadKeys(T_void)
{
    T_word16 i, j;
    T_byte8 *buffer;
    int c;

    DebugRoutine("EscMenuLoadKeys");

    buffer = INIFileGet(G_iniFile, "keyboard", "keys1");
    if (buffer) {
        j = 0;
        for (i = 0; i < (NUMBER_KEY_PAGES * 12) && (i < 34); i++) {
            sscanf(buffer + j, "%02X", &c);
            G_keys[i] = (T_byte8)c;
            j += 2;
        }
    }
    buffer = INIFileGet(G_iniFile, "keyboard", "keys2");
    if (buffer) {
        j = 0;
        for (; (i < KEYMAP_NUM_KEYS_MAPPED); i++) {
            sscanf(buffer + j, "%02X", &c);
            G_keys[i] = (T_byte8)c;
            j += 2;
        }
    }

    DebugEnd();
}

static T_void EscMenuSaveSettings(void)
{
    char value[10];
    DebugRoutine("EscMenuSaveSettings");

    EscMenuSaveKeys();

    INIFilePut(G_iniFile, "options", "boboff",
            G_escMenuToggleOptions[ESC_MENU_OPTION_BOBBING] ? "0" : "1");
    INIFilePut(G_iniFile, "options", "invertmousey",
            G_escMenuToggleOptions[ESC_MENU_INVERT_MOUSE_Y] ? "1" : "0");
    sprintf(value, "%d", G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED]);
    INIFilePut(G_iniFile, "options", "mouseturnspeed", value);
    sprintf(value, "%d", G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED]);
    INIFilePut(G_iniFile, "options", "keyturnspeed", value);
    INIFilePut(G_iniFile, "options", "dyingdropsitems",
            G_escMenuToggleOptions[ESC_MENU_DYING_DROPS_ALL_ITEMS] ? "1" : "0");

    DebugEnd();
}

static T_void EscMenuLoadSettings(void)
{
    char *p;

    DebugRoutine("EscMenuLoadSettings");

    EscMenuLoadKeys();

    memset(G_escMenuToggleOptions, 0, sizeof(G_escMenuToggleOptions));
    p = (char *)INIFileGet(G_iniFile, "options", "boboff");
    if (p)
        G_escMenuToggleOptions[ESC_MENU_OPTION_BOBBING] = (atoi(p)==1)?0:1;

    G_escMenuToggleOptions[ESC_MENU_INVERT_MOUSE_Y] = 0;
    p = (char *)INIFileGet(G_iniFile, "options", "invertmousey");
    if (p)
        G_escMenuToggleOptions[ESC_MENU_INVERT_MOUSE_Y] = (atoi(p)==1)?1:0;

    G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] = 100;
    p = (char *)INIFileGet(G_iniFile, "options", "mouseturnspeed");
    if (p)
        G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] = atoi(p);
    if (G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] > 200)
        G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] = 200;
    if (G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] < 20)
        G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED] = 20;

    G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] = 100;
    p = (char *)INIFileGet(G_iniFile, "options", "keyturnspeed");
    if (p)
        G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] = atoi(p);
    if (G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] > 200)
        G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] = 200;
    if (G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] < 20)
        G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED] = 20;

    G_escMenuToggleOptions[ESC_MENU_DYING_DROPS_ALL_ITEMS] = 1;
    p = (char *)INIFileGet(G_iniFile, "options", "dyingdropsitems");
    if (p)
        G_escMenuToggleOptions[ESC_MENU_DYING_DROPS_ALL_ITEMS] = (atoi(p)==1)?1:0;

    DebugEnd();
}

static void EscapeMenuSetOptionButtonText(T_byte8 aOptionIndex, const char *aText, T_byte8 aColor)
{
    T_buttonID buttonID;

    DebugRoutine("EscapeMenuSetOptionButtonText");

    buttonID = FormFindObjID(7000+aOptionIndex);
    if (buttonID)
        ButtonSetText(buttonID, (const T_byte8 *)aText, aColor);

    DebugEnd();
}

static void EscapeMenuSetOptions(void)
{
    char value[10];
    DebugRoutine("EscapeMenuSetOptions");

    EscapeMenuSetOptionButtonText(ESC_MENU_OPTION_BOBBING,
            G_escMenuToggleOptions[ESC_MENU_OPTION_BOBBING] ? "ON" : "OFF", 31);
    EscapeMenuSetOptionButtonText(ESC_MENU_INVERT_MOUSE_Y,
            G_escMenuToggleOptions[ESC_MENU_INVERT_MOUSE_Y] ? "YES" : "NO", 31);
    sprintf(value, "%d %%", G_escMenuToggleOptions[ESC_MENU_MOUSE_TURN_SPEED]);
    EscapeMenuSetOptionButtonText(ESC_MENU_MOUSE_TURN_SPEED, value, 31);
    sprintf(value, "%d %%", G_escMenuToggleOptions[ESC_MENU_KEYBOARD_TURN_SPEED]);
    EscapeMenuSetOptionButtonText(ESC_MENU_KEYBOARD_TURN_SPEED, value, 31);
    EscapeMenuSetOptionButtonText(ESC_MENU_DYING_DROPS_ALL_ITEMS,
            G_escMenuToggleOptions[ESC_MENU_DYING_DROPS_ALL_ITEMS] ? "YES" : "NO", 31);

    DebugEnd();
}

void EscapeMenuLoadState(void)
{
    char form[20];
    T_word32 i;
    T_buttonID buttonID;

    DebugRoutine("EscapeMenuLoadState");

    // Setup the form
    sprintf(form, "ESCMENU%d.FRM", G_escMenuState);
    FormLoadFromFile(form);
    FormSetCallbackRoutine(EscapeMenuFormControl);

    for (i=0; i<NUMBER_KEYS; i++) {
        buttonID = FormFindObjID(1000+i);
        if (buttonID) {
            // Found a button on the page, replace it
            ButtonSetText(buttonID, EscMenuKeyToString(G_keys[i]), 31);
        }
    }

    // Set the options
    EscapeMenuSetOptions();

    G_escKeyPressed = FALSE;

    DebugEnd();
}

void EscapeMenuEnterState(T_byte8 aState)
{
    DebugRoutine("EscapeMenuEnterState");

    // Save the old state and load the new state
    G_escMenuStateStack[G_escMenuDepth++] = G_escMenuState;
    G_escMenuState = aState;
    EscapeMenuLoadState();

    DebugEnd();
}

void EscapeMenuExitState(void)
{
    DebugRoutine("EscapeMenuExitState");

    if (G_escMenuState == 0) {
        // Exiting the ESC Menu
        EscapeMenuClose();
    } else {
        G_escMenuState = G_escMenuStateStack[--G_escMenuDepth];
        EscapeMenuLoadState();
    }

    DebugEnd();
}

void EscapeMenuGotoState(T_byte8 aState)
{
    DebugRoutine("EscapeMenuGotoState");

    // Jump to this new state
    G_escMenuState = aState;
    EscapeMenuLoadState();

    DebugEnd();
}

void EscapeMenuOpen(void)
{
    DebugRoutine("EscapeMenuOpen");

    if (!G_escMenuIsOpen) {
        // We are now open (or will be soon)
        G_escMenuIsOpen = TRUE;

        ConfigClose();

        G_iniFile = ConfigOpen(); // INIFileOpen("config.ini");

        EscMenuLoadSettings();

        // Force the mouse out of relative mode
        MouseRelativeModeOff();

        // Save the current form
        FormPush();

        // Tell the view we want to update form graphics per update
        ViewUpdateFormsOverViewEnable();

        G_previousKeyHandler = KeyboardGetEventHandler();
        KeyboardSetEventHandler(EscapeMenuKeyHandler);

        G_escMenuState = 0;
        EscapeMenuLoadState();
    }
    DebugEnd();
}

void EscapeMenuClose(void)
{
    DebugRoutine("EscapeMenuClose");

    if (G_escMenuIsOpen) {
        // We are now open (or will be soon)
        G_escMenuIsOpen = FALSE;

        // Go back to the previous form setup
        FormPop();

        // Tell the view we want to no longer update
        // form graphics per update
        ViewUpdateFormsOverViewDisable();

        KeyboardSetEventHandler(G_previousKeyHandler);

        // Save all the key strokes
        EscMenuSaveSettings();

        // Setup the new key controls
        KeyMapReinitialize(G_iniFile);

        // Use any new settings
        ConfigReadOptions(G_iniFile);

        //INIFileClose("config.ini", G_iniFile);

        // Force a save
        ConfigClose();
        ConfigOpen();

        BannerFinish();
        BannerInit();

        SpellsClearRunes(NULL);
        SpellsFinish();
        SpellsInitSpells();
    }

    DebugEnd();
}

E_Boolean EscapeMenuIsOpen(void)
{
    return G_escMenuIsOpen;
}

