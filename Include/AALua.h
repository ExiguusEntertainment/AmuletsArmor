#ifndef LUA_H_
#define LUA_H_

void AALuaInit(void);
void AALuaFinish(void);
void AALuaScriptLoadAndRun(const char *aFilename);
void AALuaCallGlobalFunction0(const char *aFuncName);

#endif
