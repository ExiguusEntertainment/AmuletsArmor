#ifndef AA_CONIO_SHIM_H
#define AA_CONIO_SHIM_H

#include <stdio.h>

static inline int kbhit(void)
{
    return 0;
}

static inline int getch(void)
{
    return getchar();
}

#endif
