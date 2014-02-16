#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "AALua.h"
#include "File.h"
#include <COLOR.H>
#include <GRAPHICS.H>
#include <KEYBOARD.H>
#include <MOUSEMOD.H>
#include <PICS.H>
#include <SOUND.H>
#include <TICKER.H>
#include <VIEW.H>

static lua_State *L;

static int AASetLuaPath(lua_State* L, const char* path)
{
//    const char *cur_path;
    char newPath[1000];

    sprintf(newPath, "LUA_PATH=%s", path);
    _putenv ( newPath );
#if 0
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
    cur_path = lua_tostring( L, -1 ); // grab path string from top of stack
    sprintf(newPath, "%s;%s", cur_path, path);
    lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring(L, cur_path); // push the new one
    lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
    lua_pop(L, 1); // get rid of package table from top of stack
#endif

    return 0; // all done!
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
        error = luaL_loadbuffer(L, p_file, size, aFilename)
                || lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1); /* pop error message from the stack */
        }
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

int LUA_API luaopen_aasound(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Play", lua_SoundPlay },
            { "PlayLoop", lua_SoundPlayLoop },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//--------------------
static int lua_ColorAddGlobal(lua_State *L)
{
    T_byte8 red = (T_byte8)(lua_tonumber(L, 1));
    T_byte8 green = (T_byte8)(lua_tonumber(L, 2));
    T_byte8 blue = (T_byte8)(lua_tonumber(L, 3));
    ColorAddGlobal(red/4, green/4, blue/4);

    return 0;
}

static int lua_ColorStoreDefaultPalette(lua_State *L)
{
    ColorStoreDefaultPalette();

    return 0;
}

static int lua_ColorFadeTo(lua_State *L)
{
    T_byte8 red = (T_byte8)(lua_tonumber(L, 1));
    T_byte8 green = (T_byte8)(lua_tonumber(L, 2));
    T_byte8 blue = (T_byte8)(lua_tonumber(L, 3));
    ColorFadeTo(red, green, blue);

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
    DebugRoutine("lua_TickerContinue");

    TickerContinue();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aaticker(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "Get", lua_TickerGet },
            { "Pause", lua_TickerPause },
            { "Continue", lua_TickerContinue },
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


int LUA_API luaopen_aagraphics(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "DrawBitmap", lua_GraphicsDrawBitmap },
            { "DrawRectangle", lua_GraphicsDrawRectangle },
            { "DrawText", lua_GraphicsDrawText },
            { "DrawShadowedText", lua_GraphicsDrawShadowedText },
            { "SetCursor", lua_GraphicsSetCursorPosition },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
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

int LUA_API luaopen_aakeyboard(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "BufferOn", lua_KeyboardBufferOn },
            { "BufferOff", lua_KeyboardBufferOff },
            { "BufferGet", lua_KeyboardBufferGet },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
int LUA_API luaopen_aakeymap(lua_State *L)
{
    static struct luaL_Reg driver[] = {
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
        printf("IAALuaMouseEventHandler '%s' error:\n  %s\n", lua_tostring(L, -1));
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
    DebugRoutine("lua_KeyboardBufferOff");

    MouseUpdateEvents();

    DebugEnd();
    return 0;
}

int LUA_API luaopen_aamouse(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "PopEventHandler", lua_MousePopEventHandler },
            { "PushEventHandler", lua_MousePushEventHandler },
            { "UpdateEvents", lua_MouseUpdateEvents },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}

//-----------------
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

void AALuaInit(void)
{
    DebugRoutine("AALuaInit");

    //char buff[256];
    //int error;

    L = luaL_newstate();  /* create state */
    luaL_openlibs(L);

    luaL_requiref(L, "aacolor", luaopen_aacolor, 1);
    luaL_requiref(L, "aagraphics", luaopen_aagraphics, 1);
    luaL_requiref(L, "aakeyboard", luaopen_aakeyboard, 1);
    luaL_requiref(L, "aakeymap", luaopen_aakeymap, 1);
    luaL_requiref(L, "aamouse", luaopen_aamouse, 1);
    luaL_requiref(L, "aapics", luaopen_aapics, 1);
    luaL_requiref(L, "aasound", luaopen_aasound, 1);
    luaL_requiref(L, "aatime", luaopen_aatime, 1);
    luaL_requiref(L, "aaticker", luaopen_aaticker, 1);
    luaL_requiref(L, "aaview", luaopen_aaview, 1);

    //AASetLuaPath(L, "./Lua;./Lua/AAEngine");
    AALuaScriptLoadAndRun("startup.lua");

    DebugEnd();
}

void AALuaCallGlobalFunction0(const char *aFuncName)
{
    DebugRoutine("AALuaCallGlobalFunction0");

    lua_getglobal(L, aFuncName);
    if (lua_pcall(L, 0, 0, 0) != 0) {
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
