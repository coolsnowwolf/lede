#ifndef __MUSL_COMPAT_STRING_H
#define __MUSL_COMPAT_STRING_H

#include_next <string.h>

/* Change XSI compliant version into GNU extension hackery */
static inline char *
gnu_strerror_r(int err, char *buf, size_t buflen)
{
	if (strerror_r(err, buf, buflen))
		return NULL;
	return buf;
}
#ifdef _GNU_SOURCE
#define strerror_r gnu_strerror_r
#endif

#endif
