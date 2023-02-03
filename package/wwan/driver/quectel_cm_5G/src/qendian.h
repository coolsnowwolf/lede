#ifndef __QUECTEL_ENDIAN_H__
#define __QUECTEL_ENDIAN_H__
#include <endian.h>

#ifndef htole32 
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htole16(x) (uint16_t)(x)
#define le16toh(x) (uint16_t)(x)
#define letoh16(x) (uint16_t)(x)
#define htole32(x) (uint32_t)(x)
#define le32toh(x) (uint32_t)(x)
#define letoh32(x) (uint32_t)(x)
#define htole64(x) (uint64_t)(x)
#define le64toh(x) (uint64_t)(x)
#define letoh64(x) (uint64_t)(x)
#else
static __inline uint16_t __bswap16(uint16_t __x) {
    return (__x<<8) | (__x>>8);
}

static __inline uint32_t __bswap32(uint32_t __x) {
    return (__x>>24) | (__x>>8&0xff00) | (__x<<8&0xff0000) | (__x<<24);
}

static __inline uint64_t __bswap64(uint64_t __x) {
    return (__bswap32(__x)+0ULL<<32) | (__bswap32(__x>>32));
}

#define htole16(x) __bswap16(x)
#define le16toh(x) __bswap16(x)
#define letoh16(x) __bswap16(x)
#define htole32(x) __bswap32(x)
#define le32toh(x) __bswap32(x)
#define letoh32(x) __bswap32(x)
#define htole64(x) __bswap64(x)
#define le64toh(x) __bswap64(x)
#define letoh64(x) __bswap64(x)
#endif
#endif

#define le16_to_cpu(x) le16toh((uint16_t)(x))
#define le32_to_cpu(x) le32toh((uint32_t)(x))
#define le64_to_cpu(x) le64toh((uint64_t)(x))
#define cpu_to_le16(x) htole16((uint16_t)(x))
#define cpu_to_le32(x) htole32((uint32_t)(x))
#define cpu_to_le64(x) htole64((uint64_t)(x))

static __inline uint32_t ql_swap32(uint32_t __x) {
    return (__x>>24) | (__x>>8&0xff00) | (__x<<8&0xff0000) | (__x<<24);
}
#endif //__QUECTEL_ENDIAN_H__

