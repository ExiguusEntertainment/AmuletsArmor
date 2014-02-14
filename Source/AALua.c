#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "AALua.h"
#include "File.h"
#include <SOUND.H>

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
static int lua_SoundPlayByNumber(lua_State *L)
{
    T_word16 soundNum = (T_word16)lua_tonumber(L, 1);
    T_word16 volume = (T_word16)(255.0 * lua_tonumber(L, 2));

    printf("Playing number %d at volume %d!\n", soundNum, volume);
    SoundPlayByNumber(soundNum, volume);

    return 0;
}

int LUA_API luaopen_sound(lua_State *L)
{
    static struct luaL_Reg driver[] = {
            { "PlayByNumber", lua_SoundPlayByNumber },
            { NULL, NULL }, };
    luaL_newlib(L, driver);
    return 1;
}
//--------------------

void AALuaInit(void)
{
    DebugRoutine("AALuaInit");

    //char buff[256];
    //int error;

    L = luaL_newstate();  /* create state */
    luaL_openlibs(L);

    luaL_requiref(L, "sound", luaopen_sound, 1);

    //AASetLuaPath(L, "./Lua;./Lua/AAEngine");
    AALuaScriptLoadAndRun("startup.lua");

    DebugEnd();
}

void AALuaFinish(void)
{
    lua_close(L);
}
