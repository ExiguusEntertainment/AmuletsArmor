#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <windows.h>

/* _NDEBUG definition will remove all exception handling calls */
/* from executable */
//#define _NDEBUG

/* _DEBUG_NOFAIL definintion will keep an exception from executing */
/* an exit or throw command */
//#define _DEBUG_NOFAIL

/* MAX_CALL_STACK depicts the number of _debug object calls that */
/* can be nested before automatic failure */
#define MAX_CALL_STACK 25

#ifdef _NDEBUG
#define _check(cond)     ((void)0)
#define _checkObj(cond)  ((void)0)
#define _debug(name)     ((void)0)
#else
#define _check(cond)     ((cond)?(void)0 : debug.fail((char *)#cond,(char *)__FILE__,(int)__LINE__))
#define _checkObj(cond)  ((cond)?(void)0 : debug.fail((char *)#cond,(char *)__FILE__,(int)__LINE__,this))
#define _debug(name)     C_debug debug((char *)name)
#endif

class C_debugObject
{
#ifndef _NDEBUG
    public:
    virtual void errorDump();
#endif
};

class C_debug
{
    public:

    C_debug (char *routinename);
    ~C_debug ();

    void fail (char *p_msg, char *p_file, int errline);
    void fail (char *p_msg, char *p_file, int errline, C_debugObject *myobj);
    void dump (void);
};

#endif
