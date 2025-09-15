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

#ifndef __linux__
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobe16(x) bswap_16(x)
#define be16toh(x) bswap_16(x)
#define htobe32(x) bswap_32(x)
#define be32toh(x) bswap_32(x)
#define htobe64(x) bswap_64(x)
#define be64toh(x) bswap_64(x)
#define htole16(x) (uint16_t)(x)
#define le16toh(x) (uint16_t)(x)
#define htole32(x) (uint32_t)(x)
#define le32toh(x) (uint32_t)(x)
#define htole64(x) (uint64_t)(x)
#define le64toh(x) (uint64_t)(x)
#else
#define htobe16(x) (uint16_t)(x)
#define be16toh(x) (uint16_t)(x)
#define htobe32(x) (uint32_t)(x)
#define be32toh(x) (uint32_t)(x)
#define htobe64(x) (uint64_t)(x)
#define be64toh(x) (uint64_t)(x)
#define htole16(x) bswap_16(x)
#define le16toh(x) bswap_16(x)
#define htole32(x) bswap_32(x)
#define le32toh(x) bswap_32(x)
#define htole64(x) bswap_64(x)
#define le64toh(x) bswap_64(x)
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define betoh16(x) bswap_16(x)
#define betoh32(x) bswap_32(x)
#define betoh64(x) bswap_64(x)
#define letoh16(x) (uint16_t)(x)
#define letoh32(x) (uint32_t)(x)
#define letoh64(x) (uint64_t)(x)
#else
#define betoh16(x) (uint16_t)(x)
#define betoh32(x) (uint32_t)(x)
#define betoh64(x) (uint64_t)(x)
#define letoh16(x) bswap_16(x)
#define letoh32(x) bswap_32(x)
#define letoh64(x) bswap_64(x)
#endif
#endif
#endif

#endif
