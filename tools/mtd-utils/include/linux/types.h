#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <mtd/ubi-media.h>

typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef __u16 __le16;
typedef __u32 __le32;
typedef __u64 __le64;
typedef __u64 off64_t;

typedef __u16  __sum16;
typedef __u32  __wsum;

#endif /* _LINUX_TYPES_H */
