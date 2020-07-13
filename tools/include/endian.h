#ifndef __endian_compat_h
#define __endian_compat_h

#if defined(__linux__) || defined(__CYGWIN__)
#include <byteswap.h>
#include_next <endian.h>
#elif defined(__APPLE__)
#include <netinet/in.h>
#include <libkern/OSByteOrder.h>
#define bswap_16(x) OSSwapInt16(x)
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#define bswap_16(x) bswap16(x)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#elif defined(__OpenBSD__)
#include <sys/types.h>
#define bswap_16(x) __swap16(x)
#define bswap_32(x) __swap32(x)
#define bswap_64(x) __swap64(x)
#else
#include <machine/endian.h>
#define bswap_16(x) swap16(x)
#define bswap_32(x) swap32(x)
#define bswap_64(x) swap64(x)
#endif

#ifndef __BYTE_ORDER
#define __BYTE_ORDER BYTE_ORDER
#endif
#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN BIG_ENDIAN
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#endif

#endif
