/* Copyright (C) 1992-2002, 2004, 2005, 2006, 2007, 2009, 2011, 2012
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_SYS_CDEFS_H
#define	_SYS_CDEFS_H	1

/* We are almost always included from features.h. */
#ifndef _FEATURES_H
# include <features.h>
#endif

/* The GNU libc does not support any K&R compilers or the traditional mode
   of ISO C compilers anymore.  Check for some of the combinations not
   anymore supported.  */
#if defined __GNUC__ && !defined __STDC__
# error "You need a ISO C conforming compiler to use the glibc headers"
#endif

/* Some user header file might have defined this before.  */
#undef	__P
#undef	__PMT

#ifdef __GNUC__

/* All functions, except those with callbacks or those that
   synchronize memory, are leaf functions.  */
# if __GNUC_PREREQ (4, 6) && !defined _LIBC
#  define __LEAF , __leaf__
#  define __LEAF_ATTR __attribute__ ((__leaf__))
# else
#  define __LEAF
#  define __LEAF_ATTR
# endif

/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this works only with
   gcc 2.8.x and egcs.  For gcc 3.2 and up we even mark C functions
   as non-throwing using a function attribute since programs can use
   the -fexceptions options for C code as well.  */
# if !defined __cplusplus && __GNUC_PREREQ (3, 3)
#  define __THROW	__attribute__ ((__nothrow__ __LEAF))
#  define __THROWNL	__attribute__ ((__nothrow__))
#  define __NTH(fct)	__attribute__ ((__nothrow__ __LEAF)) fct
# else
#  if defined __cplusplus && __GNUC_PREREQ (2,8)
#   define __THROW	throw ()
#   define __THROWNL	throw ()
#   define __NTH(fct)	__LEAF_ATTR fct throw ()
#  else
#   define __THROW
#   define __THROWNL
#   define __NTH(fct)	fct
#  endif
# endif

#else	/* Not GCC.  */

# define __inline		/* No inline functions.  */

# define __THROW
# define __THROWNL
# define __NTH(fct)	fct

#endif	/* GCC.  */

/* These two macros are not used in glibc anymore.  They are kept here
   only because some other projects expect the macros to be defined.  */
#define __P(args)	args
#define __PMT(args)	args

/* For these things, GCC behaves the ANSI way normally,
   and the non-ANSI way under -traditional.  */

#define __CONCAT(x,y)	x ## y
#define __STRING(x)	#x

/* This is not a typedef so `const __ptr_t' does the right thing.  */
#define __ptr_t void *
#define __long_double_t  long double


/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef	__cplusplus
# define __BEGIN_DECLS	extern "C" {
# define __END_DECLS	}
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif


/* The standard library needs the functions from the ISO C90 standard
   in the std namespace.  At the same time we want to be safe for
   future changes and we include the ISO C99 code in the non-standard
   namespace __c99.  The C++ wrapper header take case of adding the
   definitions to the global namespace.  */
#if defined __cplusplus && defined _GLIBCPP_USE_NAMESPACES
# define __BEGIN_NAMESPACE_STD	namespace std {
# define __END_NAMESPACE_STD	}
# define __USING_NAMESPACE_STD(name) using std::name;
# define __BEGIN_NAMESPACE_C99	namespace __c99 {
# define __END_NAMESPACE_C99	}
# define __USING_NAMESPACE_C99(name) using __c99::name;
#else
/* For compatibility we do not add the declarations into any
   namespace.  They will end up in the global namespace which is what
   old code expects.  */
# define __BEGIN_NAMESPACE_STD
# define __END_NAMESPACE_STD
# define __USING_NAMESPACE_STD(name)
# define __BEGIN_NAMESPACE_C99
# define __END_NAMESPACE_C99
# define __USING_NAMESPACE_C99(name)
#endif


/* Support for bounded pointers.  */
#ifndef __BOUNDED_POINTERS__
# define __bounded	/* nothing */
# define __unbounded	/* nothing */
# define __ptrvalue	/* nothing */
#endif


/* Fortify support.  */
#define __bos(ptr) __builtin_object_size (ptr, __USE_FORTIFY_LEVEL > 1)
#define __bos0(ptr) __builtin_object_size (ptr, 0)
#define __fortify_function __extern_always_inline __attribute_artificial__

#if __GNUC_PREREQ (4,3)
# define __warndecl(name, msg) \
  extern void name (void) __attribute__((__warning__ (msg)))
# define __warnattr(msg) __attribute__((__warning__ (msg)))
# define __errordecl(name, msg) \
  extern void name (void) __attribute__((__error__ (msg)))
#else
# define __warndecl(name, msg) extern void name (void)
# define __warnattr(msg)
# define __errordecl(name, msg) extern void name (void)
#endif

/* Support for flexible arrays.  */
#if __GNUC_PREREQ (2,97)
/* GCC 2.97 supports C99 flexible array members.  */
# define __flexarr	[]
#else
# ifdef __GNUC__
#  define __flexarr	[0]
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __flexarr	[]
#  else
/* Some other non-C99 compiler.  Approximate with [1].  */
#   define __flexarr	[1]
#  endif
# endif
#endif


/* __asm__ ("xyz") is used throughout the headers to rename functions
   at the assembly language level.  This is wrapped by the __REDIRECT
   macro, in order to support compilers that can do this some other
   way.  When compilers don't support asm-names at all, we have to do
   preprocessor tricks instead (which don't have exactly the right
   semantics, but it's the best we can do).

   Example:
   int __REDIRECT(setpgrp, (__pid_t pid, __pid_t pgrp), setpgid); */

#if defined __GNUC__ && __GNUC__ >= 2

# define __REDIRECT(name, proto, alias) name proto __asm__ (__ASMNAME (#alias))
# ifdef __cplusplus
#  define __REDIRECT_NTH(name, proto, alias) \
     name proto __THROW __asm__ (__ASMNAME (#alias))
#  define __REDIRECT_NTHNL(name, proto, alias) \
     name proto __THROWNL __asm__ (__ASMNAME (#alias))
# else
#  define __REDIRECT_NTH(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROW
#  define __REDIRECT_NTHNL(name, proto, alias) \
     name proto __asm__ (__ASMNAME (#alias)) __THROWNL
# endif
# define __ASMNAME(cname)  __ASMNAME2 (__USER_LABEL_PREFIX__, cname)
# define __ASMNAME2(prefix, cname) __STRING (prefix) cname

/*
#elif __SOME_OTHER_COMPILER__

# define __REDIRECT(name, proto, alias) name proto; \
	_Pragma("let " #name " = " #alias)
*/
#endif

/* GCC has various useful declarations that can be made with the
   `__attribute__' syntax.  All of the ways we use this do fine if
   they are omitted for compilers that don't understand it. */
#if !defined __GNUC__ || __GNUC__ < 2
# define __attribute__(xyz)	/* Ignore */
#endif

/* At some point during the gcc 2.96 development the `malloc' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */
#if __GNUC_PREREQ (2,96)
# define __attribute_malloc__ __attribute__ ((__malloc__))
#else
# define __attribute_malloc__ /* Ignore */
#endif

/* At some point during the gcc 2.96 development the `pure' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */
#if __GNUC_PREREQ (2,96)
# define __attribute_pure__ __attribute__ ((__pure__))
#else
# define __attribute_pure__ /* Ignore */
#endif

/* This declaration tells the compiler that the value is constant.  */
#if __GNUC_PREREQ (2,5)
# define __attribute_const__ __attribute__ ((__const__))
#else
# define __attribute_const__ /* Ignore */
#endif

/* At some point during the gcc 3.1 development the `used' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.  */
#if __GNUC_PREREQ (3,1)
# define __attribute_used__ __attribute__ ((__used__))
# define __attribute_noinline__ __attribute__ ((__noinline__))
#else
# define __attribute_used__ __attribute__ ((__unused__))
# define __attribute_noinline__ /* Ignore */
#endif

/* gcc allows marking deprecated functions.  */
#if __GNUC_PREREQ (3,2)
# define __attribute_deprecated__ __attribute__ ((__deprecated__))
#else
# define __attribute_deprecated__ /* Ignore */
#endif

/* At some point during the gcc 2.8 development the `format_arg' attribute
   for functions was introduced.  We don't want to use it unconditionally
   (although this would be possible) since it generates warnings.
   If several `format_arg' attributes are given for the same function, in
   gcc-3.0 and older, all but the last one are ignored.  In newer gccs,
   all designated arguments are considered.  */
#if __GNUC_PREREQ (2,8)
# define __attribute_format_arg__(x) __attribute__ ((__format_arg__ (x)))
#else
# define __attribute_format_arg__(x) /* Ignore */
#endif

/* At some point during the gcc 2.97 development the `strfmon' format
   attribute for functions was introduced.  We don't want to use it
   unconditionally (although this would be possible) since it
   generates warnings.  */
#if __GNUC_PREREQ (2,97)
# define __attribute_format_strfmon__(a,b) \
  __attribute__ ((__format__ (__strfmon__, a, b)))
#else
# define __attribute_format_strfmon__(a,b) /* Ignore */
#endif

/* The nonull function attribute allows to mark pointer parameters which
   must not be NULL.  */
#if __GNUC_PREREQ (3,3)
# define __nonnull(params) __attribute__ ((__nonnull__ params))
#else
# define __nonnull(params)
#endif

/* If fortification mode, we warn about unused results of certain
   function calls which can lead to problems.  */
#if __GNUC_PREREQ (3,4)
# define __attribute_warn_unused_result__ \
   __attribute__ ((__warn_unused_result__))
# if __USE_FORTIFY_LEVEL > 0
#  define __wur __attribute_warn_unused_result__
# endif
#else
# define __attribute_warn_unused_result__ /* empty */
#endif
#ifndef __wur
# define __wur /* Ignore */
#endif

/* Forces a function to be always inlined.  */
#if __GNUC_PREREQ (3,2)
# define __always_inline __inline __attribute__ ((__always_inline__))
#else
# define __always_inline __inline
#endif

/* Associate error messages with the source location of the call site rather
   than with the source location inside the function.  */
#if __GNUC_PREREQ (4,3)
# define __attribute_artificial__ __attribute__ ((__artificial__))
#else
# define __attribute_artificial__ /* Ignore */
#endif

/* GCC 4.3 and above with -std=c99 or -std=gnu99 implements ISO C99
   inline semantics, unless -fgnu89-inline is used.  */
#if !defined __cplusplus || __GNUC_PREREQ (4,3)
# if defined __GNUC_STDC_INLINE__ || defined __cplusplus
#  define __extern_inline extern __inline __attribute__ ((__gnu_inline__))
#  define __extern_always_inline \
  extern __always_inline __attribute__ ((__gnu_inline__))
# else
#  define __extern_inline extern __inline
#  define __extern_always_inline extern __always_inline
# endif
#endif

/* GCC 4.3 and above allow passing all anonymous arguments of an
   __extern_always_inline function to some other vararg function.  */
#if __GNUC_PREREQ (4,3)
# define __va_arg_pack() __builtin_va_arg_pack ()
# define __va_arg_pack_len() __builtin_va_arg_pack_len ()
#endif

/* It is possible to compile containing GCC extensions even if GCC is
   run in pedantic mode if the uses are carefully marked using the
   `__extension__' keyword.  But this is not generally available before
   version 2.8.  */
#if !__GNUC_PREREQ (2,8)
# define __extension__		/* Ignore */
#endif

/* __restrict is known in EGCS 1.2 and above. */
#if !__GNUC_PREREQ (2,92)
# define __restrict	/* Ignore */
#endif

/* ISO C99 also allows to declare arrays as non-overlapping.  The syntax is
     array_name[restrict]
   GCC 3.1 supports this.  */
#if __GNUC_PREREQ (3,1) && !defined __GNUG__
# define __restrict_arr	__restrict
#else
# ifdef __GNUC__
#  define __restrict_arr	/* Not supported in old GCC.  */
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __restrict_arr	restrict
#  else
/* Some other non-C99 compiler.  */
#   define __restrict_arr	/* Not supported.  */
#  endif
# endif
#endif

#if __GNUC__ >= 3
# define __glibc_unlikely(cond) __builtin_expect((cond), 0)
#else
# define __glibc_unlikely(cond) (cond)
#endif

#endif	 /* sys/cdefs.h */
