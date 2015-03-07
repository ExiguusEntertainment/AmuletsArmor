#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "AALua.h"
#include "File.h"
#include <BUTTON.H>
#include <COLOR.H>
#include <GRAPHIC.H>
#include <GRAPHICS.H>
#include <KEYBOARD.H>
#include <KEYMAP.H>
#include <MOUSEMOD.H>
#include <PICS.H>
#include <SLIDR.H>
#include <SOUND.H>
#include <SPELTYPE.H>
#include <STATS.H>
#include <TICKER.H>
#include <TXTBOX.H>
#include <VIEW.H>

static lua_State *L;

#define MAP_STRING_TO_INT(str, x)   if(strcmp(aString, str)==0) return (x);
#define MAP_INT_START(v)            switch (v) {
#define MAP_INT_TO_STRING(x, str)       case (x): return (str); break;
#define MAP_INT_END()                default: break; }

static void ITableSetBoolean(lua_State* L, const char *name, E_Boolean value)
{
    lua_pushboolean(L, value?1:0);
    lua_setfield(L, -2, name);
}

static void ITableSetInt(lua_State* L, const char *name, int value)
{
    lua_pushnumber(L, (double)value);
    lua_setfield(L, -2, name);
}

static void ITableSetString(lua_State* L, const char *name, const char *string)
{
    lua_pushstring(L, string);
    lua_setfield(L, -2, name);
}

static void ITableSetTable(lua_State* L, const char *name)
{
    lua_setfield(L, -2, name);
}

static void ITableGetTable(lua_State* L, const char *name)
{
    lua_getfield(L, -1, name);
}

static void ITableReleaseTable(lua_State *L)
{
    lua_pop(L, 1);
}
//static void ITableSetDouble(lua_State* L, const char *name, double value)
//{
//    lua_pushstring(L, name);
//    lua_pushnumber(L, value);
//    lua_settable(L, -3);
//}

static int ITableGetInt(lua_State* L, const char *name, int *value, int defvalue)
{
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    if (!lua_isnumber(L, -1)) {
        *value = defvalue;
        return -1;
    }
    *value = (int)lua_tonumber(L, -1);
    lua_pop(L, 1);

    return 0;
}

static int ITableGetBoolean(lua_State* L, const char *name, E_Boolean *value, E_Boolean defvalue)
{
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    if (!lua_isboolean(L, -1)) {
        *value = defvalue;
        return -1;
    }
    *value = (int)lua_toboolean(L, -1);
    lua_pop(L, 1);

    return 0;
}

static int ITableGetString(lua_State* L, const char *name, char *value, int limit, char *defvalue)
{
    const char *s;
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    if (!lua_isstring(L, -1)) {
        strncpy(value, defvalue, limit);
        return -1;
    }
    s = lua_tostring(L, -1);
    strncpy(value, s, limit);
    lua_pop(L, 1);

    return 0;
}

static E_mouseEvent IMouseEventConvert(const char *aString)
{
    MAP_STRING_TO_INT("start", MOUSE_EVENT_START);
    MAP_STRING_TO_INT("end", MOUSE_EVENT_END);
    MAP_STRING_TO_INT("move", MOUSE_EVENT_MOVE);
    MAP_STRING_TO_INT("drag", MOUSE_EVENT_DRAG);
    MAP_STRING_TO_INT("idle", MOUSE_EVENT_IDLE);
    MAP_STRING_TO_INT("held", MOUSE_EVENT_HELD);
    return MOUSE_EVENT_UNKNOWN;
}

static E_keyboardEvent IKeyEventConvert(const char *aString)
{
    MAP_STRING_TO_INT("press", KEYBOARD_EVENT_PRESS);
    MAP_STRING_TO_INT("release", KEYBOARD_EVENT_RELEASE);
    MAP_STRING_TO_INT("buffered", KEYBOARD_EVENT_BUFFERED);
    MAP_STRING_TO_INT("held", KEYBOARD_EVENT_HELD);
    return KEYBOARD_EVENT_UNKNOWN;
}

static const char *ISpellSystemToString(T_byte8 aSpellSystem)
{
    MAP_INT_START(aSpellSystem);
    MAP_INT_TO_STRING(SPELL_SYSTEM_NONE, "none");
    MAP_INT_TO_STRING(SPELL_SYSTEM_MAGE, "mage");
    MAP_INT_TO_STRING(SPELL_SYSTEM_CLERIC, "cleric");
    MAP_INT_TO_STRING(SPELL_SYSTEM_ARCANE, "arcane");
    MAP_INT_TO_STRING(SPELL_SYSTEM_ANY, "any");
    MAP_INT_END();
    return "unknown";
}

static T_byte8 IStringToSpellSystem(const char *aString)
{
    MAP_STRING_TO_INT("none", SPELL_SYSTEM_NONE);
    MAP_STRING_TO_INT("mage", SPELL_SYSTEM_MAGE);
    MAP_STRING_TO_INT("cleric", SPELL_SYSTEM_CLERIC);
    MAP_STRING_TO_INT("arcane", SPELL_SYSTEM_ARCANE);
    MAP_STRING_TO_INT("any", SPELL_SYSTEM_ANY);
    return SPELL_SYSTEM_NONE;
}

static int traceback(lua_State *L) {
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return 1;
}

void AALuaScriptLoadAndRun(const char *aFilename)
{
    void *p_file;
    T_word32 size;
    char filename[200];
    int error;

    DebugRoutine("AALuaScriptLoadAndRun");

    sprintf(filename, "Lua/%s", aFilename);

    p_file = FileLoad(filename, &size);
    if (p_file) {
        DebugLuaStart(filename, L);
        lua_pushcfunction(L, traceback);
        error = luaL_loadbuffer(L, p_file, size, aFilename)
                || lua_pcall(L, 0, 0, lua_gettop(L) - 1);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1); /* pop error message from the stack */
        }
        DebugLuaEnd();
    } else {
        fprintf(stderr, "Could not load script %s!", filename);
    }

    DebugEnd();
}


//-----------------
static int lua_SoundPlay(lua_State *L)
{
    T_word16 volume = (T_word16)(255.0 * lua_tonumber(L, 2));

    DebugRoutine("lua_SoundPlay");

    if (lua_type(L, 1) == LUA_TNUMBER) {
        T_word16 soundNum = (T_word16)lua_tonumber(L, 1);

        SoundPlayByNumber(soundNum, volume);
    } else {
        const char *soundName = lua_tolstring(L, 1, NULL);

        SoundPlayByName(soundName, volume);
    }

    DebugEnd();
    return 0;
}

static int lua_SoundPlayLoop(lua_State *L)
{
    T_word16 volume = (T_word16)(255.0 * lua_tonumber(L, 2));
    T_sword16 channel;

    DebugRoutine("lua_SoundPlayLoop");

    if (lua_type(L, 1) == LUA_TNUMBER) {
        T_word16 soundNum = (T_word16)lua_tonumber(L, 1);

        channel = SoundPlayLoopByNumber(soundNum, volume);
        if (channel < 0)
            lua_pushnil(L);
        else
            lua_pushnumber(L, channel);
    } else {
        // TODO: Put in the right code here!
    }

    DebugEnd();
    return 1;
}

static int lua_SoundUpdate(lua_State *L)
{
    DebugRoutine("lua_SoundUpdate");

    SoundUpdate();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aasound(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Play", lua_SoundPlay },
            { "PlayLoop", lua_SoundPlayLoop },
            { "Update", lua_SoundUpdate },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//--------------------
static int lua_ColorAddGlobal(lua_State *L)
{
    T_byte8 red, green, blue;

    DebugRoutine("lua_ColorAddGlobal");

    red = (T_byte8)(lua_tonumber(L, 1));
    green = (T_byte8)(lua_tonumber(L, 2));
    blue = (T_byte8)(lua_tonumber(L, 3));
    ColorAddGlobal(red/4, green/4, blue/4);

    DebugEnd();

    return 0;
}

static int lua_ColorGammaAdjust(lua_State *L)
{
    T_word16 v;

    DebugRoutine("lua_ColorAdjustGamma");

    v = ColorGammaAdjust();
    lua_pushnumber(L, v);

    DebugEnd();

    return 1;
}

static int lua_ColorStoreDefaultPalette(lua_State *L)
{
    ColorStoreDefaultPalette();

    return 0;
}

static int lua_ColorFadeTo(lua_State *L)
{
    T_byte8 red, green, blue;

    DebugRoutine("lua_ColorFadeTo");

    red = (T_byte8)(lua_tonumber(L, 1));
    green = (T_byte8)(lua_tonumber(L, 2));
    blue = (T_byte8)(lua_tonumber(L, 3));
    ColorFadeTo(red, green, blue);

    DebugEnd();
    return 0;
}

static int lua_ColorUpdate(lua_State *L)
{
    T_word16 duration = (T_word16)(lua_tonumber(L, 1));

    DebugRoutine("lua_ColorUpdate");

    ColorUpdate(duration);

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aacolor(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "AddGlobal", lua_ColorAddGlobal },
            { "GammaAdjust", lua_ColorGammaAdjust },
            { "FadeTo", lua_ColorFadeTo },
            { "StoreDefaultPalette", lua_ColorStoreDefaultPalette },
            { "Update", lua_ColorUpdate },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//-----------------

//-----------------
static int lua_TickerGet(lua_State *L)
{
    T_word32 ticks;

    DebugRoutine("lua_TickerGet");

    ticks = TickerGet();
    lua_pushnumber(L, (double)ticks);

    DebugEnd();
    return 1;
}

static int lua_TickerPause(lua_State *L)
{
    DebugRoutine("lua_TickerPause");

    TickerPause();

    DebugEnd();
    return 0;
}

static int lua_TickerContinue(lua_State *L)
{
#ifdef WIN32
    extern void SleepMS(T_word32 aMS);
#endif
    DebugRoutine("lua_TickerContinue");

#ifdef WIN32
    SleepMS((T_word32)lua_tonumber(L, 1));
#endif

    DebugEnd();
    return 0;
}

static int lua_TickerSleepMS(lua_State *L)
{
    DebugRoutine("lua_TickerSleepMS");

    TickerContinue();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aaticker(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Continue", lua_TickerContinue },
            { "Get", lua_TickerGet },
            { "Pause", lua_TickerPause },
            { "SleepMS", lua_TickerSleepMS },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_SetPalette(lua_State *L)
{
    T_viewPalette palette = (T_viewPalette)lua_tonumber(L, 1);

    DebugRoutine("lua_SetPalette");

    ViewSetPalette(palette);

    DebugEnd();
    return 0;
}


int LUA_API luaopen_aaview(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "SetPalette", lua_SetPalette },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_GraphicsDrawBitmap(lua_State *L)
{
    T_bitmap *p_bitmap;
    T_word16 x, y;

    DebugRoutine("lua_GraphicsDrawBitmap");

    p_bitmap = (T_bitmap *)lua_touserdata(L, 1);
    x = (T_word16)lua_tonumber(L, 2);
    y = (T_word16)lua_tonumber(L, 3);

    GrDrawBitmap(p_bitmap, x, y);

    DebugEnd();
    return 0;
}

static int lua_GraphicsDrawRectangle(lua_State *L)
{
    T_word16 x1, y1, x2, y2;
    T_byte8 color;

    DebugRoutine("lua_GraphicsDrawRectangle");

    x1 = (T_word16)lua_tonumber(L, 1);
    y1 = (T_word16)lua_tonumber(L, 2);
    x2 = (T_word16)lua_tonumber(L, 3);
    y2 = (T_word16)lua_tonumber(L, 4);
    color = (T_byte8)lua_tonumber(L, 5);

    GrDrawRectangle(x1, y1, x2, y2, color);

    DebugEnd();
    return 0;
}


static int lua_GraphicsDrawShadowedText(lua_State *L)
{
    const char *p_text;
    T_byte8 color;
    T_byte8 colorShadow;

    DebugRoutine("lua_GraphicsDrawShadowedText");

    p_text = lua_tostring(L, 1);
    color = (T_byte8)lua_tonumber(L, 2);
    colorShadow = (T_byte8)lua_tonumber(L, 3);

    GrDrawShadowedText((char *)p_text, color, colorShadow);

    DebugEnd();
    return 0;
}

static int lua_GraphicsDrawText(lua_State *L)
{
    const char *p_text;
    T_byte8 color;

    DebugRoutine("lua_GraphicsDrawShadowedText");

    p_text = lua_tostring(L, 1);
    color = (T_byte8)lua_tonumber(L, 2);

    GrDrawText((char *)p_text, color);

    DebugEnd();
    return 0;
}

static int lua_GraphicsScreenPop(lua_State *L)
{
    DebugRoutine("lua_GraphicsScreenPop");

    GrActualScreenPop();

    DebugEnd();
    return 0;
}

static int lua_GraphicsScreenPush(lua_State *L)
{
    DebugRoutine("lua_GraphicsScreenPush");

    GrActualScreenPush();

    DebugEnd();
    return 0;
}

static int lua_GraphicsSetCursorPosition(lua_State *L)
{
    T_word16 x, y;

    DebugRoutine("lua_GraphicsSetCursorPosition");

    x = (T_word16)lua_tonumber(L, 1);
    y = (T_word16)lua_tonumber(L, 2);

    GrSetCursorPosition(x, y);

    DebugEnd();
    return 0;
}

static int lua_GraphicsShadeRectangle(lua_State *L)
{
    T_word16 x1, y1, x2, y2;
    T_byte8 shade;

    DebugRoutine("lua_GraphicsShadeRectangle");

    x1 = (T_word16)lua_tonumber(L, 1);
    y1 = (T_word16)lua_tonumber(L, 2);
    x2 = (T_word16)lua_tonumber(L, 3);
    y2 = (T_word16)lua_tonumber(L, 4);
    shade = (T_byte8)lua_tonumber(L, 5);

    GrShadeRectangle(x1, y1, x2, y2, shade);

    DebugEnd();
    return 0;
}


int LUA_API luaopen_aagraphics(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "DrawBitmap", lua_GraphicsDrawBitmap },
            { "DrawRectangle", lua_GraphicsDrawRectangle },
            { "DrawText", lua_GraphicsDrawText },
            { "DrawShadowedText", lua_GraphicsDrawShadowedText },
            { "ScreenPop", lua_GraphicsScreenPop },
            { "ScreenPush", lua_GraphicsScreenPush },
            { "ShadeRectangle", lua_GraphicsShadeRectangle },
            { "SetCursor", lua_GraphicsSetCursorPosition },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static T_void IAALuaKeyboardEventHandler(E_keyboardEvent event, T_word16 scankey)
{
    const char *p_event;

    switch (event) {
        case KEYBOARD_EVENT_NONE:
            p_event = "none";
            break;
        case KEYBOARD_EVENT_PRESS:
            p_event = "press";
            break;
        case KEYBOARD_EVENT_RELEASE:
            p_event = "release";
            break;
        case KEYBOARD_EVENT_BUFFERED:
            p_event = "buffered";
            break;
        case KEYBOARD_EVENT_HELD:
            p_event = "held";
            break;
        default:
        case KEYBOARD_EVENT_UNKNOWN:
            p_event = "unknown";
            break;
    }
    lua_getglobal(L, "_keyboardHandleEvent");
    lua_pushstring(L, p_event);
    lua_pushnumber(L, scankey);
    if (lua_pcall(L, 2, 0, 0) != 0) {
        printf("IAALuaKeyboardEventHandler error:\n  %s\n", lua_tostring(L, -1));
        DebugCheck(FALSE);
    }
}

static int lua_KeyboardBufferOn(lua_State *L)
{
    DebugRoutine("lua_KeyboardBufferOn");

    KeyboardBufferOn();

    DebugEnd();
    return 0;
}

static int lua_KeyboardBufferOff(lua_State *L)
{
    DebugRoutine("lua_KeyboardBufferOff");

    KeyboardBufferOff();

    DebugEnd();
    return 0;
}

static int lua_KeyboardBufferGet(lua_State *L)
{
    T_byte8 key;

    DebugRoutine("lua_KeyboardBufferGet");

    key = KeyboardBufferGet();
    if (key == 0)
        lua_pushnil(L);
    else
        lua_pushnumber(L, key);

    DebugEnd();
    return 1;
}

static int lua_KeyboardGetScanCode(lua_State *L)
{
    T_word16 scankey;

    DebugRoutine("lua_KeyboardGetScanKey");

    scankey = (T_word16)lua_tonumber(L, 1);

    if (KeyboardGetScanCode(scankey)) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    DebugEnd();
    return 1;
}

static int lua_KeyboardUpdateEvents(lua_State *L)
{
    DebugRoutine("lua_KeyboardUpdateEvents");

    KeyboardUpdateEvents();

    DebugEnd();
    return 0;
}

static int lua_KeyboardDebounce(lua_State *L)
{
    DebugRoutine("lua_KeyboardDebounce");

    KeyboardDebounce();

    DebugEnd();
    return 0;
}

static int lua_KeyboardPopEventHandler(lua_State *L)
{
    DebugRoutine("lua_KeyboardPopEventHandler");

    KeyboardPopEventHandler();

    DebugEnd();
    return 0;
}

static int lua_KeyboardPushEventHandler(lua_State *L)
{
    DebugRoutine("lua_KeyboardPushEventHandler");

    KeyboardPushEventHandler(IAALuaKeyboardEventHandler);

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aakeyboard(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "BufferOn", lua_KeyboardBufferOn },
            { "BufferOff", lua_KeyboardBufferOff },
            { "BufferGet", lua_KeyboardBufferGet },
            { "Debounce", lua_KeyboardDebounce },
            { "GetScanCode", lua_KeyboardGetScanCode },
            { "PopEventHandler", lua_KeyboardPopEventHandler },
            { "PushEventHandler", lua_KeyboardPushEventHandler },
            { "UpdateEvents", lua_KeyboardUpdateEvents },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------

static int lua_KeymapGetScan(lua_State *L)
{
    T_word16 keymapping;

    DebugRoutine("lua_KeymapGetScan");

    keymapping = (T_word16)lua_tonumber(L, 1);

    if (KeyMapGetScan(keymapping)) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    DebugEnd();
    return 1;
}

static int lua_KeymapGetMap(lua_State *L)
{
    T_word16 keymapping;

    DebugRoutine("lua_KeymapGetMap");

    keymapping = (T_word16)lua_tonumber(L, 1);

    lua_pushnumber(L, KeyMap(keymapping));
    DebugEnd();
    return 1;
}


int LUA_API luaopen_aakeymap(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "GetScan", lua_KeymapGetScan },
            { "GetMap", lua_KeymapGetMap },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static T_void IAALuaMouseEventHandler(E_mouseEvent event,
                                      T_word16 x,
                                      T_word16 y,
                                      T_buttonClick buttons)
{
    const char *p_event;

    switch (event) {
        case MOUSE_EVENT_START:
            p_event = "start";
            break;
        case MOUSE_EVENT_END:
            p_event = "end";
            break;
        case MOUSE_EVENT_MOVE:
            p_event = "move";
            break;
        case MOUSE_EVENT_DRAG:
            p_event = "drag";
            break;
        case MOUSE_EVENT_IDLE:
            p_event = "idle";
            break;
        case MOUSE_EVENT_HELD:
            p_event = "held";
            break;
        case MOUSE_EVENT_UNKNOWN:
        default:
            p_event = "unknown";
            break;
    }
    lua_getglobal(L, "_mouseHandleEvent");
    lua_pushstring(L, p_event);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    if (buttons)
        lua_pushnumber(L, buttons);
    else
        lua_pushnil(L);
    if (lua_pcall(L, 4, 0, 0) != 0) {
        printf("IAALuaMouseEventHandler error:\n  %s\n", lua_tostring(L, -1));
        DebugCheck(FALSE);
    }
}

static int lua_MousePopEventHandler(lua_State *L)
{
    DebugRoutine("lua_MousePopEventHandler");

    MousePopEventHandler();

    DebugEnd();
    return 0;
}

static int lua_MousePushEventHandler(lua_State *L)
{
    DebugRoutine("lua_MousePushEventHandler");

    MousePushEventHandler(IAALuaMouseEventHandler);

    DebugEnd();
    return 0;
}

static int lua_MouseUpdateEvents(lua_State *L)
{
    DebugRoutine("lua_MouseUpdateEvents");

    MouseUpdateEvents();

    DebugEnd();
    return 0;
}

static int lua_MouseSetDefaultBitmap(lua_State *L)
{
    T_bitmap *p_bitmap;
    T_word16 x, y;
    DebugRoutine("lua_MouseSetDefaultBitmap");

    x = (T_word16)lua_tonumber(L, 1);
    y = (T_word16)lua_tonumber(L, 2);
    if (lua_isnil(L, 3)) {
        p_bitmap = NULL;
	    MouseSetDefaultBitmap(x, y, 0);
	} else {
        p_bitmap = (T_bitmap *)lua_touserdata(L, 3);
        MouseSetDefaultBitmap(x, y, &p_bitmap[1]);
	}

    DebugEnd();
    return 0;
}

static int lua_MouseUseDefaultBitmap(lua_State *L)
{
    DebugRoutine("lua_MouseUseDefaultBitmap");

    MouseUseDefaultBitmap();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aamouse(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "PopEventHandler", lua_MousePopEventHandler },
            { "PushEventHandler", lua_MousePushEventHandler },
            { "UpdateEvents", lua_MouseUpdateEvents },
            { "SetDefaultBitmap", lua_MouseSetDefaultBitmap },
            { "UseDefaultBitmap", lua_MouseUseDefaultBitmap },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_PicsExist(lua_State *L)
{
    const char *picName;
    E_Boolean result;

    DebugRoutine("lua_PicsExist");

    picName = lua_tolstring(L, 1, NULL);
    result = PictureExist(picName);
    lua_pushboolean(L, result?1:0);

    DebugEnd();
    return 1;
}

static int lua_PicsLockData(lua_State *L)
{
    T_byte8 *p_data;
    T_resource res;
    const char *picName;

    DebugRoutine("lua_PicsLockData");

    picName = lua_tolstring(L, 1, NULL);

    p_data = PictureLockData((char *)picName, &res);

    lua_pushlightuserdata(L, p_data);
    lua_pushlightuserdata(L, res);

    DebugEnd();
    return 2;
}

static int lua_PicsUnfind(lua_State *L)
{
    T_resource res;

    DebugRoutine("lua_PicsUnfind");

    res = (T_resource)lua_touserdata(L, 1);

    PictureUnfind(res);

    DebugEnd();
    return 0;
}

static int lua_PicsUnlock(lua_State *L)
{
    T_resource res;

    DebugRoutine("lua_PicsUnlock");

    res = (T_resource)lua_touserdata(L, 1);

    PictureUnlock(res);

    DebugEnd();
    return 0;
}

static int lua_PicsUnlockAndUnfind(lua_State *L)
{
    T_resource res;

    DebugRoutine("lua_PicsUnlockAndUnfind");

    res = (T_resource)lua_touserdata(L, 1);

    PictureUnlockAndUnfind(res);

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aapics(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Exist", lua_PicsExist },
            { "LockData", lua_PicsLockData },
            { "Unfind", lua_PicsUnfind },
            { "Unlock", lua_PicsUnlock },
            { "UnlockAndUnfind", lua_PicsUnlockAndUnfind },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_DelayMS(lua_State *L)
{
    T_word32 t;

    DebugRoutine("lua_PicsUnlock");

    t = (T_word32)lua_tonumber(L, 1);

    delay(t);

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aatime(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "DelayMS", lua_DelayMS },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//-----------------

static int lua_DisplayGetWidth(lua_State *L)
{
    DebugRoutine("lua_DisplayGetWidth");

    lua_pushnumber(L, SCREEN_SIZE_X);

    DebugEnd();
    return 1;
}

static int lua_DisplayGetHeight(lua_State *L)
{
    DebugRoutine("lua_DisplayGetHeight");

    lua_pushnumber(L, SCREEN_SIZE_Y);

    DebugEnd();
    return 1;
}

int LUA_API luaopen_aadisplay(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "GetWidth", lua_DisplayGetWidth },
            { "GetHeight", lua_DisplayGetHeight },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//-----------------
static int lua_StatsDeleteCharacter(lua_State *L)
{
    T_byte8 selected;
    E_Boolean result;

    DebugRoutine("lua_StatsDeleteCharacter");
    selected = (T_byte8)lua_tonumber(L, 1);
    result = StatsDeleteCharacter(selected);
    DebugEnd();

    return 1;
}

static int lua_StatsGetSavedCharacterList(lua_State *L)
{
    T_statsSavedCharArray *p_charArray;
    T_statsSavedCharacterID *p_char;
    const char *status;
    int i;
    DebugRoutine("lua_StatsGetSavedCharacterList");

    p_charArray = StatsGetSavedCharacterList();
    p_char = p_charArray->chars;
    lua_newtable(L); // array
    for (i=0; i<MAX_CHARACTERS_PER_SERVER; i++, p_char++) {
        lua_newtable(L);
        ITableSetString(L, "name", p_char->name);
        switch (p_char->status) {
            case CHARACTER_STATUS_OK:
                status = "ok";
                break;
            case CHARACTER_STATUS_DEAD:
                status = "dead";
                break;
            case CHARACTER_STATUS_TAGGED_FOR_REMOVAL:
                status = "removal";
                break;
            case CHARACTER_STATUS_UNAVAILABLE:
                status = "unavailable";
                break;
            case CHARACTER_STATUS_UNDEFINED:
                status = "undefined";
                break;
            default:
                status = "unknown";
                break;
        }
        ITableSetString(L, "status", status);
        ITableSetString(L, "password", p_char->password);
        ITableSetInt(L, "mail", p_char->mail);

        // Add to the array
        lua_rawseti(L, -2, i+1);
    }

    DebugEnd();
    return 1;
}

static int lua_StatsSetSavedCharacterList(lua_State *L)
{
    int i;
    T_statsSavedCharArray charArray;
    T_statsSavedCharacterID *p_char = charArray.chars;
    char status[30];
    int mail;
    DebugRoutine("lua_StatsSetSavedCharacterList");

    // Ensure we have a table/array being passed in
    luaL_checktype(L, 1, LUA_TTABLE);

    if (lua_istable(L, 1)) {
        lua_pushnil(L);
        for (i=1; i<=MAX_CHARACTERS_PER_SERVER; i++, p_char++) {
            // Walk the list (1..n)
            if (lua_next(L, 1) == 0) {
                printf("Missing entry!");
                DebugCheck(FALSE);
            }
            // Get the sub-table of the character
            ITableGetString(L, "name", p_char->name, sizeof(p_char->name) - 1,
                    "<undefined>");
            ITableGetString(L, "password", p_char->password,
                    sizeof(p_char->password) - 1, "");
            ITableGetInt(L, "mail", &mail, 0);
            p_char->mail = (T_byte8)mail;
            ITableGetString(L, "status", status, sizeof(status) - 1,
                    "<undefined>");
            if (strcmp(status, "ok") == 0)
                p_char->status = CHARACTER_STATUS_OK;
            else if (strcmp(status, "dead") == 0)
                p_char->status = CHARACTER_STATUS_DEAD;
            else if (strcmp(status, "removal"))
                p_char->status = CHARACTER_STATUS_TAGGED_FOR_REMOVAL;
            else if (strcmp(status, "unavailable")==0)
                p_char->status = CHARACTER_STATUS_UNAVAILABLE;
            else if (strcmp(status, "undefined"))
                p_char->status = CHARACTER_STATUS_UNDEFINED;
            else
                p_char->status = CHARACTER_STATUS_UNKNOWN;

            lua_pop(L, 1);
        }
    }

    DebugEnd();
    return 0;
}

static int lua_StatsGet(lua_State *L)
{
    T_byte8 selected;

    selected = (T_byte8)lua_tonumber(L, 1);

    lua_newtable(L);
    ITableSetString(L, "name", G_activeStats->Name);
    ITableSetString(L, "class", G_activeStats->ClassName);
    ITableSetString(L, "title", G_activeStats->ClassTitle);
    ITableSetInt(L, "health", G_activeStats->Health);
    ITableSetInt(L, "healthMax", G_activeStats->MaxHealth);
    ITableSetInt(L, "mana", G_activeStats->Mana);
    ITableSetInt(L, "manaMax", G_activeStats->MaxMana);
    ITableSetInt(L, "food", G_activeStats->Food);
    ITableSetInt(L, "foodMax", G_activeStats->MaxFood);
    ITableSetInt(L, "water", G_activeStats->Water);
    ITableSetInt(L, "waterMax", G_activeStats->MaxWater);
    ITableSetInt(L, "poison", G_activeStats->PoisonLevel);
    ITableSetInt(L, "regenHealth", G_activeStats->RegenHealth);
    ITableSetInt(L, "regenMana", G_activeStats->RegenMana);
    ITableSetInt(L, "jumpPower", G_activeStats->JumpPower);
    ITableSetInt(L, "jumpPowerMod", G_activeStats->JumpPowerMod);
    ITableSetInt(L, "tallness", G_activeStats->Tallness);
    ITableSetInt(L, "climbHeight", G_activeStats->ClimbHeight);
    ITableSetInt(L, "velRunningMax", G_activeStats->MaxVRunning);
    ITableSetInt(L, "velWalkingMax", G_activeStats->MaxVWalking);
    ITableSetInt(L, "heartRate", G_activeStats->HeartRate);
    ITableSetInt(L, "velFallingMax", G_activeStats->MaxFallV);
    ITableSetInt(L, "weaponBaseDamage", G_activeStats->WeaponBaseDamage);
    ITableSetInt(L, "weaponBaseSpeed", G_activeStats->WeaponBaseSpeed);
    ITableSetInt(L, "attackSpeed", G_activeStats->AttackSpeed);
    ITableSetInt(L, "attackDamage", G_activeStats->AttackDamage);
    ITableSetBoolean(L, "isAlive", G_activeStats->playerisalive);
    ITableSetInt(L, "classType", G_activeStats->ClassType);
    ITableSetInt(L, "armorLevel", G_activeStats->ArmorLevel);
    ITableSetInt(L, "load", G_activeStats->Load);
    ITableSetInt(L, "loadMax", G_activeStats->MaxLoad);
    ITableSetInt(L, "level", G_activeStats->Level);
    ITableSetInt(L, "xp", G_activeStats->Experience);
    ITableSetInt(L, "xpNeeded", G_activeStats->ExpNeeded);
    ITableSetString(L, "spellSystem", ISpellSystemToString(G_activeStats->SpellSystem));
    ITableSetString(L, "password", (char *)G_activeStats->password);
    lua_newtable(L);
    ITableSetInt(L, "strength", G_activeStats->Attributes[ATTRIBUTE_STRENGTH]);
    ITableSetInt(L, "speed", G_activeStats->Attributes[ATTRIBUTE_SPEED]);
    ITableSetInt(L, "magic", G_activeStats->Attributes[ATTRIBUTE_MAGIC]);
    ITableSetInt(L, "accuracy", G_activeStats->Attributes[ATTRIBUTE_ACCURACY]);
    ITableSetInt(L, "stealth", G_activeStats->Attributes[ATTRIBUTE_STEALTH]);
    ITableSetInt(L, "constitution", G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION]);
    ITableSetTable(L, "attributes");

    return 1;
}

static int lua_StatsSet(lua_State *L)
{
    T_byte8 selected;
    int v;
    char spellsystem[20];
    E_Boolean b;

    selected = (T_byte8)lua_tonumber(L, 1);

    //lua_newtable(L);
    ITableGetString(L, "name", G_activeStats->Name, sizeof(G_activeStats->Name), "Missing");
    ITableGetString(L, "class", G_activeStats->ClassName, sizeof(G_activeStats->ClassName), "Citizen");
    ITableGetString(L, "title", G_activeStats->ClassTitle, sizeof(G_activeStats->ClassTitle), "---");
    ITableGetInt(L, "health", &v, 20);
    G_activeStats->Health = v;
    ITableGetInt(L, "healthMax", &v, 20);
    G_activeStats->MaxHealth = v;
    ITableGetInt(L, "mana", &v, 0);
    G_activeStats->Mana = v;
    ITableGetInt(L, "manaMax", &v, 0);
    G_activeStats->MaxMana = v;
    ITableGetInt(L, "food", &v, 0);
    G_activeStats->Food = v;
    ITableGetInt(L, "foodMax", &v, 0);
    G_activeStats->MaxFood = v;
    ITableGetInt(L, "water", &v, 0);
    G_activeStats->Water = v;
    ITableGetInt(L, "waterMax", &v, 0);
    G_activeStats->MaxWater = v;
    ITableGetInt(L, "poison", &v, 0);
    G_activeStats->PoisonLevel = v;
    ITableGetInt(L, "regenHealth", &v, 0);
    G_activeStats->RegenHealth = v;
    ITableGetInt(L, "regenMana", &v, 0);
    G_activeStats->RegenMana = v;
    ITableGetInt(L, "jumpPower", &v, 0);
    G_activeStats->JumpPower = v;
    ITableGetInt(L, "jumpPowerMod", &v, 0);
    G_activeStats->JumpPowerMod = v;
    ITableGetInt(L, "tallness", &v, 0);
    G_activeStats->Tallness = v;
    ITableGetInt(L, "climbHeight", &v, 0);
    G_activeStats->ClimbHeight = v;
    ITableGetInt(L, "velRunningMax", &v, 0);
    G_activeStats->MaxVRunning = v;
    ITableGetInt(L, "velWalkingMax", &v, 0);
    G_activeStats->MaxVWalking = v;
    ITableGetInt(L, "heartRate", &v, 0);
    G_activeStats->HeartRate = v;
    ITableGetInt(L, "velFallingMax", &v, 0);
    G_activeStats->MaxFallV = v;
    ITableGetInt(L, "weaponBaseDamage", &v, 0);
    G_activeStats->WeaponBaseDamage = v;
    ITableGetInt(L, "weaponBaseSpeed", &v, 0);
    G_activeStats->WeaponBaseSpeed = v;
    ITableGetInt(L, "attackSpeed", &v, 0);
    G_activeStats->AttackSpeed = v;
    ITableGetInt(L, "attackDamage", &v, 0);
    G_activeStats->AttackDamage = v;
    ITableGetBoolean(L, "isAlive", &b, TRUE);
    G_activeStats->playerisalive = b;
    ITableGetInt(L, "classType", &v, 0);
    G_activeStats->ClassType = v;
    ITableGetInt(L, "armorLevel", &v, 0);
    G_activeStats->ArmorLevel = v;
    ITableGetInt(L, "load", &v, 0);
    G_activeStats->Load = v;
    ITableGetInt(L, "loadMax", &v, 0);
    G_activeStats->MaxLoad = v;
    ITableGetInt(L, "level", &v, 0);
    G_activeStats->Level = v;
    ITableGetInt(L, "xp", &v, 0);
    G_activeStats->Experience = v;
    ITableGetInt(L, "xpNeeded", &v, 0);
    G_activeStats->ExpNeeded = v;
    ITableGetString(L, "spellSystem", spellsystem, sizeof(spellsystem), "none");
    G_activeStats->SpellSystem = IStringToSpellSystem(spellsystem);
    ITableGetString(L, "password", (char *)G_activeStats->password, sizeof(G_activeStats->password), "");
    
    ITableGetTable(L, "attributes");

    ITableGetInt(L, "strength", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_STRENGTH] = v;
    ITableGetInt(L, "speed", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_SPEED] = v;
    ITableGetInt(L, "magic", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_MAGIC] = v;
    ITableGetInt(L, "accuracy", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_ACCURACY] = v;
    ITableGetInt(L, "stealth", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_STEALTH] = v;
    ITableGetInt(L, "constitution", &v, 20);
    G_activeStats->Attributes[ATTRIBUTE_CONSTITUTION] = v;

    ITableReleaseTable(L);

    return 0;
}

static int lua_StatsGetActive(lua_State *L)
{
    lua_pushnumber(L, StatsGetActive());

    return 1;
}

static int lua_StatsInit(lua_State *L)
{
    StatsInit();

    return 0;
}

static int lua_StatsMakeActive(lua_State *L)
{
    T_byte8 selected;

    selected = (T_byte8)lua_tonumber(L, 1);

    StatsMakeActive(selected);
    return 0;
}

static int lua_StatsLoadCharacter(lua_State *L)
{
    T_byte8 selected;
    E_Boolean result;

    selected = (T_byte8)lua_tonumber(L, 1);

    result = StatsLoadCharacter(selected);
    lua_pushboolean(L, (result)?1:0);

    return 1;
}

static int lua_StatsSaveCharacter(lua_State *L)
{
    T_byte8 selected;
    E_Boolean result;

    selected = (T_byte8)lua_tonumber(L, 1);

    result = StatsSaveCharacter(selected);

    return 0;
}

int LUA_API luaopen_aastats(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "DeleteCharacter", lua_StatsDeleteCharacter },
            { "GetCharacterList", lua_StatsGetSavedCharacterList },
            { "Get", lua_StatsGet },
            { "GetActive", lua_StatsGetActive },
            { "Init", lua_StatsInit },
            { "LoadCharacter", lua_StatsLoadCharacter },
            { "MakeActive", lua_StatsMakeActive },
            { "SaveCharacter", lua_StatsSaveCharacter },
            { "Set", lua_StatsSet },
            { "SetSavedCharacterList", lua_StatsSetSavedCharacterList },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static T_void IAALuaButtonEventHandler(T_buttonID buttonID)
{
    const char *p_event;
    DebugRoutine("IAALuaButtonEventHandler");

    switch (ButtonGetAction()) {
        default:
        case BUTTON_ACTION_NO_ACTION:
            p_event = "none";
            break;
        case BUTTON_ACTION_PUSHED:
            p_event = "press";
            break;
        case BUTTON_ACTION_RELEASED:
            p_event = "release";
            break;
    }
    lua_getglobal(L, "_buttonHandleEvent");
//    lua_pushnumber(L, (T_word32)buttonID);
    lua_pushlightuserdata(L, buttonID);
    lua_pushstring(L, p_event);
    if (lua_pcall(L, 2, 0, 0) != 0) {
        const char *errMsg = lua_tostring(L, -1);
        printf("IAALuaButtonEventHandler error:\n  %s\n", errMsg);
        DebugCheck(FALSE);
    }

    DebugEnd();
}

static int lua_ButtonCreate(lua_State *L)
{
    T_word16 x, y;
    T_word16 hotkeys;
    const char *picName;
    E_Boolean toggleType;
    T_buttonID buttonID;
    DebugRoutine("lua_ButtonCreate");

    x = (T_word16)lua_tonumber(L, 1);
    y = (T_word16)lua_tonumber(L, 2);
    picName = lua_tostring(L, 3);
    toggleType = lua_tonumber(L, 4)?TRUE:FALSE;
    hotkeys = (T_word16)lua_tonumber(L, 5);
    buttonID = ButtonCreate(x, y, picName, toggleType, hotkeys,
            IAALuaButtonEventHandler, IAALuaButtonEventHandler);
//    lua_pushnumber(L, (T_word32)buttonID);
    lua_pushlightuserdata(L, buttonID);

    DebugEnd();
    return 1;
}

static int lua_ButtonDelete(lua_State *L)
{
    T_buttonID buttonID;
    DebugRoutine("lua_ButtonDelete");

    buttonID = (T_buttonID)lua_touserdata(L, 1);
    ButtonDelete(buttonID);

    DebugEnd();
    return 0;
}

static int lua_ButtonKeyControl(lua_State *L)
{
    E_keyboardEvent event;
    T_word16 scankey;
    DebugRoutine("lua_ButtonKeyControl");

    event = IKeyEventConvert(lua_tostring(L, 1));
    scankey = (T_byte8)lua_tonumber(L, 2);
    ButtonKeyControl(event, scankey);

    DebugEnd();

    return 0;
}

static int lua_ButtonMouseControl(lua_State *L)
{
    E_mouseEvent event;
    T_word16 x, y;
    T_buttonClick button;
    DebugRoutine("lua_ButtonMouseControl");

    event = IMouseEventConvert(lua_tostring(L, 1));
    x = (T_word16)lua_tonumber(L, 2);
    y = (T_word16)lua_tonumber(L, 3);
    button = (T_byte8)lua_tonumber(L, 4);
    ButtonMouseControl(event, x, y, button);

    DebugEnd();

    return 0;
}

static int lua_ButtonRedrawAllButtons(lua_State *L)
{
    DebugRoutine("lua_ButtonRedrawAllButtons");

    ButtonRedrawAllButtons();

    DebugEnd();

    return 0;
}

int LUA_API luaopen_aabutton(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Create", lua_ButtonCreate },
            { "Delete", lua_ButtonDelete },
            { "HandleKeyEvent", lua_ButtonKeyControl },
            { "HandleMouseEvent", lua_ButtonMouseControl },
            { "RedrawAllButtons", lua_ButtonRedrawAllButtons },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_ScrollbarMouseControl(lua_State *L)
{
    E_mouseEvent event;
    T_word16 x, y;
    T_buttonClick button;
    DebugRoutine("lua_ScrollbarMouseControl");

    event = IMouseEventConvert(lua_tostring(L, 1));
    x = (T_word16)lua_tonumber(L, 2);
    y = (T_word16)lua_tonumber(L, 3);
    button = (T_byte8)lua_tonumber(L, 4);
    SliderMouseControl(event, x, y, button);

    DebugEnd();

    return 0;
}

int LUA_API luaopen_aascrollbar(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "HandleMouseEvent", lua_ScrollbarMouseControl },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static E_TxtboxMode ITxtboxModeConvert(const char *aString)
{
    MAP_STRING_TO_INT("field", Txtbox_MODE_EDIT_FIELD);
    MAP_STRING_TO_INT("textarea", Txtbox_MODE_EDIT_FIELD);
    MAP_STRING_TO_INT("ro_textarea", Txtbox_MODE_VIEW_SCROLL_FORM);
    MAP_STRING_TO_INT("ro_field", Txtbox_MODE_VIEW_NOSCROLL_FORM);
    MAP_STRING_TO_INT("ro_textarea_noscroll", Txtbox_MODE_VIEW_NOSCROLL_FORM);
    MAP_STRING_TO_INT("selection", Txtbox_MODE_SELECTION_BOX);
    MAP_STRING_TO_INT("fixed_field", Txtbox_MODE_FIXED_WIDTH_FIELD);
    return Txtbox_MODE_UNKNOWN;
}

static T_void IAALuaTxtboxEventHandler(T_TxtboxID textboxID)
{
    const char *p_event;
    DebugRoutine("IAALuaTxtboxEventHandler");

    switch (TxtboxGetAction()) {
        default:
        case Txtbox_ACTION_NO_ACTION:
            p_event = "none";
            break;
        case Txtbox_ACTION_GAINED_FOCUS:
            p_event = "focus";
            break;
        case Txtbox_ACTION_LOST_FOCUS:
            p_event = "lost";
            break;
        case Txtbox_ACTION_ACCEPTED:
            p_event = "accepted";
            break;
        case Txtbox_ACTION_DATA_CHANGED:
            p_event = "changed";
            break;
        case Txtbox_ACTION_SELECTION_CHANGED:
            p_event = "select";
            break;
    }
    lua_getglobal(L, "_textboxHandleEvent");
    lua_pushlightuserdata(L, textboxID);
    lua_pushstring(L, p_event);
    if (lua_pcall(L, 2, 0, 0) != 0) {
        const char *errMsg = lua_tostring(L, -1);
        printf("IAALuaTxtboxEventHandler error:\n  %s\n", errMsg);
        DebugCheck(FALSE);
    }

    DebugEnd();
}

static int lua_TxtboxCreate(lua_State *L)
{
    T_word16 x1, y1;
    T_word16 width, height;
    T_word16 x2, y2;
    T_word16 hotkeys;
    const char *picName;
    T_TxtboxID textboxID;
    T_word32 maxLength;
    const char *fontName;
    E_Boolean numericOnly;
    E_TxtboxJustify justify;
    const char *p_justify;
    E_TxtboxMode boxMode;
    DebugRoutine("lua_ButtonCreate");

    x1 = (T_word16)lua_tonumber(L, 1);
    y1 = (T_word16)lua_tonumber(L, 2);
    width = (T_word16)lua_tonumber(L, 3);
    height = (T_word16)lua_tonumber(L, 4);
    fontName = lua_tostring(L, 5);
    maxLength = (T_word16)lua_tonumber(L, 6);
    hotkeys = (T_word16)lua_tonumber(L, 7);
    numericOnly = lua_tonumber(L, 8)?TRUE:FALSE;
    p_justify = lua_tostring(L, 9);
    justify = (strcmp(p_justify, "center")==0)?Txtbox_JUSTIFY_CENTER:Txtbox_JUSTIFY_LEFT;
    boxMode = ITxtboxModeConvert(lua_tostring(L, 10));
    x2 = x1 + width - 1;
    y2 = y1 + height - 1;

    picName = lua_tostring(L, 3);
//    textboxID = TxtboxCreate(x1, y1, picName, toggleType, hotkeys,
//            IAALuaButtonEventHandler, IAALuaButtonEventHandler);
    textboxID = TxtboxCreate (x1, y1, x2, y2, fontName, maxLength, hotkeys, numericOnly, justify, boxMode, IAALuaTxtboxEventHandler);
    lua_pushlightuserdata(L, textboxID);

    DebugEnd();
    return 1;
}

static int lua_TxtboxDelete(lua_State *L)
{
    T_TxtboxID textboxID;
    DebugRoutine("lua_TxtboxDelete");

    textboxID = (T_TxtboxID)lua_touserdata(L, 1);
    TxtboxDelete(textboxID);

    DebugEnd();
    return 0;
}

static int lua_TxtboxMouseControl(lua_State *L)
{
    E_mouseEvent event;
    T_word16 x, y;
    T_buttonClick button;
    DebugRoutine("lua_TxtboxMouseControl");

    event = IMouseEventConvert(lua_tostring(L, 1));
    x = (T_word16)lua_tonumber(L, 2);
    y = (T_word16)lua_tonumber(L, 3);
    button = (T_byte8)lua_tonumber(L, 4);
    TxtboxMouseControl(event, x, y, button);

    DebugEnd();

    return 0;
}

static int lua_TxtboxRepaginate(lua_State *L)
{
    DebugRoutine("lua_TxtboxRepaginate");
    TxtboxRepaginate((T_TxtboxID)lua_touserdata(L, 1));
    DebugEnd();

    return 0;
}

static int lua_TxtboxCursorSetRow(lua_State *L)
{
    DebugRoutine("lua_TxtboxCursorTop");
    TxtboxCursSetRow((T_TxtboxID)lua_touserdata(L, 1), (T_word16)lua_tonumber(L, 2));
    DebugEnd();

    return 0;
}
static int lua_TxtboxCursorTop(lua_State *L)
{
    DebugRoutine("lua_TxtboxCursorTop");
    TxtboxCursTop((T_TxtboxID)lua_touserdata(L, 1));
    DebugEnd();

    return 0;
}

static int lua_TxtboxFirstBox(lua_State *L)
{
    DebugRoutine("lua_TxtboxFirstBox");
    TxtboxFirstBox();
    DebugEnd();

    return 0;
}

static int lua_TxtboxAppend(lua_State *L)
{
    DebugRoutine("lua_TxtboxAppend");
    TxtboxAppendString((T_TxtboxID)lua_touserdata(L, 1), lua_tostring(L, 2));
    DebugEnd();

    return 0;
}

static int lua_TxtboxBackspace(lua_State *L)
{
    DebugRoutine("lua_TxtboxBackspace");
    TxtboxBackSpace((T_TxtboxID)lua_touserdata(L, 1));
    DebugEnd();

    return 0;
}

static int lua_TxtboxSetMaxLength(lua_State *L)
{
    DebugRoutine("lua_TxtboxSetMaxLength");
    TxtboxSetMaxLength((T_TxtboxID)lua_touserdata(L, 1), (T_word32)lua_tonumber(L, 2));
    DebugEnd();

    return 0;
}
static int lua_TxtboxSetText(lua_State *L)
{
    DebugRoutine("lua_TxtboxAppend");
    TxtboxSetData((T_TxtboxID)lua_touserdata(L, 1), lua_tostring(L, 2));
    DebugEnd();

    return 0;
}

static int lua_TxtboxGetData(lua_State *L)
{
    DebugRoutine("lua_TxtboxGetData");
    lua_pushstring(L, TxtboxGetData((T_TxtboxID)lua_touserdata(L, 1)));
    DebugEnd();

    return 1;
}

static int lua_TxtboxGetSelectionNumber(lua_State *L)
{
    DebugRoutine("lua_TxtboxGetSelectionNumber");
    lua_pushnumber(L, TxtboxGetSelectionNumber((T_TxtboxID)lua_touserdata(L, 1)));
    DebugEnd();

    return 1;
}

static int lua_TxtboxKeyControl(lua_State *L)
{
    E_keyboardEvent event;
    T_word16 scankey;
    DebugRoutine("lua_TxtboxKeyControl");

    event = IKeyEventConvert(lua_tostring(L, 1));
    scankey = (T_word16)lua_tonumber(L, 2);
    TxtboxKeyControl(event, scankey);

    DebugEnd();

    return 0;
}

int LUA_API luaopen_aatextbox(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Append", lua_TxtboxAppend },
            { "Backspace", lua_TxtboxBackspace },
            { "Create", lua_TxtboxCreate },
            { "CursorSetRow", lua_TxtboxCursorSetRow },
            { "CursorTop", lua_TxtboxCursorTop },
            { "Delete", lua_TxtboxDelete },
            { "FirstBox", lua_TxtboxFirstBox },
            { "GetData", lua_TxtboxGetData },
            { "GetSelectionNumber", lua_TxtboxGetSelectionNumber },
            { "HandleKeyEvent", lua_TxtboxKeyControl },
            { "HandleMouseEvent", lua_TxtboxMouseControl },
            { "Repaginate", lua_TxtboxRepaginate },
            { "SetMaxLength", lua_TxtboxSetMaxLength },
            { "SetText", lua_TxtboxSetText },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
static int lua_GraphicCreate(lua_State *L)
{
    T_word16 x, y;
    const char *picName;
    T_graphicID graphicID;;
    DebugRoutine("lua_GraphicCreate");

    x = (T_word16)lua_tonumber(L, 1);
    y = (T_word16)lua_tonumber(L, 2);
    picName = lua_tostring(L, 3);
    graphicID = GraphicCreate(x, y, picName);
    lua_pushlightuserdata(L, graphicID);

    DebugEnd();

    return 1;
}

static int lua_GraphicDelete(lua_State *L)
{
    T_graphicID graphicID;;
    DebugRoutine("lua_GraphicDelete");

    graphicID = (T_graphicID)lua_touserdata(L, 1);
    GraphicDelete(graphicID);

    DebugEnd();

    return 0;
}

static int lua_GraphicUpdateAllGraphics(lua_State *L)
{
    DebugRoutine("lua_GraphicUpdateAllGraphics");

    GraphicUpdateAllGraphics();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aagraphic(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Create", lua_GraphicCreate },
            { "Delete", lua_GraphicDelete },
            { "UpdateAllGraphics", lua_GraphicUpdateAllGraphics },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
void AALuaInit(void)
{
    DebugRoutine("AALuaInit");

    //char buff[256];
    //int error;

    L = luaL_newstate();  /* create state */
    luaL_openlibs(L);

    luaL_requiref(L, "aabutton", luaopen_aabutton, 1);
    luaL_requiref(L, "aacolor", luaopen_aacolor, 1);
    luaL_requiref(L, "aadisplay", luaopen_aadisplay, 1);
    luaL_requiref(L, "aagraphic", luaopen_aagraphic, 1);
    luaL_requiref(L, "aagraphics", luaopen_aagraphics, 1);
    luaL_requiref(L, "aakeyboard", luaopen_aakeyboard, 1);
    luaL_requiref(L, "aakeymap", luaopen_aakeymap, 1);
    luaL_requiref(L, "aamouse", luaopen_aamouse, 1);
    luaL_requiref(L, "aapics", luaopen_aapics, 1);
    luaL_requiref(L, "aascrollbar", luaopen_aascrollbar, 1);
    luaL_requiref(L, "aasound", luaopen_aasound, 1);
    luaL_requiref(L, "aastats", luaopen_aastats, 1);
    luaL_requiref(L, "aatextbox", luaopen_aatextbox, 1);
    luaL_requiref(L, "aatime", luaopen_aatime, 1);
    luaL_requiref(L, "aaticker", luaopen_aaticker, 1);
    luaL_requiref(L, "aaview", luaopen_aaview, 1);

    AALuaScriptLoadAndRun("startup.lua");

    DebugEnd();
}

void AALuaCallGlobalFunction0(const char *aFuncName)
{
    DebugRoutine("AALuaCallGlobalFunction0");

    lua_pushcfunction(L, traceback);
    lua_getglobal(L, aFuncName);
    if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != 0) {
        printf("Lua Function '%s' error:\n  %s\n", aFuncName, lua_tostring(L, -1));
        DebugCheck(FALSE);
    }

    DebugEnd();
}

void AALuaFinish(void)
{
    DebugRoutine("AALuaFinish");

    lua_close(L);

    DebugEnd();
}
