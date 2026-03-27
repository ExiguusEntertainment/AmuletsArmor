#ifndef AA_DIRECT_SHIM_H
#define AA_DIRECT_SHIM_H

#include <sys/stat.h>

static inline int _mkdir(const char *path)
{
    return mkdir(path, 0777);
}

#ifndef mkdir
#define mkdir(path) _mkdir(path)
#endif

#endif
