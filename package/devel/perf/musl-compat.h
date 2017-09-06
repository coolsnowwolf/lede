#ifndef __PERF_MUSL_COMPAT_H
#define __PERF_MUSL_COMPAT_H

#ifndef __ASSEMBLER__

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <unistd.h>
#include <stdio.h>

#undef _IOWR
#undef _IOR
#undef _IOW
#undef _IOC
#undef _IO

#define _SC_LEVEL1_DCACHE_LINESIZE -1

static inline long sysconf_wrap(int name)
{
	FILE *f;
	int val;

	switch (name) {
	case _SC_LEVEL1_DCACHE_LINESIZE:
		f = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
		if (!f)
			return 0;

		if (fscanf(f, "%d", &val) != 1)
			return 0;

		fclose(f);
		return val;
	default:
		return sysconf(name);
	}
}

#define sysconf(_n) sysconf_wrap(_n)

#endif
#endif
