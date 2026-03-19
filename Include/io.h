#ifndef AA_IO_SHIM_H
#define AA_IO_SHIM_H

#include <sys/stat.h>
#include <unistd.h>

static inline int filelength(int fd)
{
	struct stat st;
	if (fstat(fd, &st) != 0)
		return -1;
	return (int)st.st_size;
}

#endif
