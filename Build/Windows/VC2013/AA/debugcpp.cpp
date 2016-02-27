#include <Windows.h>
#include "debug.hpp"
#include <fstream>

using namespace std;

static char *G_callStack[MAX_CALL_STACK];
static char G_lastCall=0;
static ofstream outfile("debug.out") ;

#ifndef _NDEBUG
void C_debugObject::errorDump (void)
{
    outfile << "object address="<<this<<"\n";
}
#endif

C_debug::C_debug (char *routinename)
{
    int i;
    /* init callstack if G_lastCall=0 */
    if (G_lastCall==0)
    for (i=0;i<MAX_CALL_STACK;i++) G_callStack[i]=NULL;

    /* push filename/routinename on stack */
    if (G_lastCall>=MAX_CALL_STACK)
    {
        /* error, too many calls */
        outfile << "Debug error: call stack too deep!\n";
        dump();
#ifndef _DEBUG_NOFAIL
//        exit (-1);
        throw (this);
#endif
        outfile.flush() ;
    }
    else
    {
        G_callStack[G_lastCall++]=routinename;
    }
}


C_debug::~C_debug()
{
    /* pop calling stack and test for stack error */
    G_callStack[G_lastCall--]=NULL;
    if (G_lastCall>MAX_CALL_STACK)
    {
        outfile << "Debug error: Call Stack Undeflow\n";
#ifndef _DEBUG_NOFAIL
//        exit (-1);
        throw (this);
#endif
        outfile.flush() ;
    }
}


void C_debug::fail (char *p_msg, char *p_file, int errline)
{
    outfile << "*******************************************************************************\n";
    outfile << "_check failure:["<<p_msg<<"] in file ["<<p_file<<"], line ["<<errline<<"]\n";
    dump();
    outfile << "*******************************************************************************\n";
#ifndef _DEBUG_NOFAIL
    outfile << "_DEBUG_NOFAIL unset - exiting.\n";
//    exit (-1);
    throw (this);
#endif
    outfile.flush() ;
}



void C_debug::fail (char *p_msg, char *p_file, int errline, C_debugObject *errobj)
{
    outfile << "*******************************************************************************\n";
    outfile << "_checkObj failure:["<<p_msg<<"] in file ["<<p_file<<"], line ["<<errline<<"]\n";
    dump();
    outfile << "object failure errorDump:\n";
#ifndef _NDEBUG
    errobj->errorDump();
#endif
    outfile << "*******************************************************************************\n";
#ifndef _DEBUG_NOFAIL
    outfile << "_DEBUG_NOFAIL unset - exiting.\n";
//    exit (-1);
    throw (this);
#endif
    outfile.flush() ;
}

void C_debug::dump (void)
{
    int i;

    i=G_lastCall-1;
    if (i<MAX_CALL_STACK)
    {
        if (G_callStack[i]!=NULL)
        {
            for (;i>=0;i--)
            {
                outfile << "      called from ["<<G_callStack[i]<<"]\n";
            }
        }
    }
    else
    {
        outfile << "debug call stack failure - undefined or empty stack\n";
#ifndef _DEBUG_NOFAIL
//        exit (-1);
        throw (this);
#endif
    }
    outfile.flush() ;
}

