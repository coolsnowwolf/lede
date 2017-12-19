#ifndef __MUSL_GLIBC_TYPES_H
#define __MUSL_GLIBC_TYPES_H

#include <sys/cdefs.h>

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#if __WORDSIZE == 64
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
#else
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
#endif

#define __off64_t off_t
#define __loff_t off_t
typedef char *__caddr_t;
#define __locale_t locale_t

#define __gid_t gid_t
#define __uid_t uid_t

#endif
