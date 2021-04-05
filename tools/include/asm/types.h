/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */

#ifndef _ASM_GENERIC_INT_LL64_H
#define _ASM_GENERIC_INT_LL64_H

typedef __signed__ char __s8;

typedef __signed__ short __s16;

typedef __signed__ int __s32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
#else
typedef __signed__ long long __s64;
#endif

#endif /* _ASM_GENERIC_INT_LL64_H */
