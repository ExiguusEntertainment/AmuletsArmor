/* process.h stub for macOS builds — SDL2 headers include this unconditionally
 * when WIN32 is defined. The functions declared here are never called on macOS
 * because SDL2 uses pthreads internally; the declarations are needed to satisfy
 * the preprocessor. */
#ifndef AA_PROCESS_H_STUB
#define AA_PROCESS_H_STUB

#include <pthread.h>

typedef unsigned long uintptr_t;

static inline uintptr_t _beginthreadex(
    void *security, unsigned stack_size,
    unsigned (*start_address)(void *), void *arglist,
    unsigned initflag, unsigned *thrdaddr)
{
    (void)security; (void)stack_size; (void)start_address;
    (void)arglist; (void)initflag; (void)thrdaddr;
    return 0;
}

static inline void _endthreadex(unsigned retval)
{
    (void)retval;
}

#endif
