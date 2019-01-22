/* crypto/md5/md5_dgst.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/e_os2.h>
#include <openssl/md5.h>
#include <openssl/opensslv.h>

#ifndef MD5_LONG_LOG2
#define MD5_LONG_LOG2 2 /* default to 32 bits */
#endif


#define DATA_ORDER_IS_LITTLE_ENDIAN

typedef MD5_LONG HASH_LONG;
typedef MD5_CTX  HASH_CTX;
const int HASH_CBLOCK=MD5_CBLOCK;

static  int HASH_Update (HASH_CTX *c, const void *data, size_t len);
static void HASH_Transform (HASH_CTX *c, const unsigned char *data);
static  int HASH_FINAL (unsigned char *md, HASH_CTX *c);
static void HASH_MAKE_STRING (HASH_CTX *c, unsigned char *s);
static void HASH_BLOCK_DATA_ORDER (MD5_CTX *c, const void *p, size_t num);


//#include "md32_common.h"

/*
 * Engage compiler specific rotate intrinsic function if available.
 */
#undef ROTATE
#ifndef PEDANTIC
# if defined(_MSC_VER) || defined(__ICC)
#  define ROTATE(a,n)	_lrotl(a,n)
# elif defined(__MWERKS__)
#  if defined(__POWERPC__)
#   define ROTATE(a,n)	__rlwinm(a,n,0,31)
#  elif defined(__MC68K__)
    /* Motorola specific tweak. <appro@fy.chalmers.se> */
#   define ROTATE(a,n)	( n<24 ? __rol(a,n) : __ror(a,32-n) )
#  else
#   define ROTATE(a,n)	__rol(a,n)
#  endif
# elif defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
  /*
   * Some GNU C inline assembler templates. Note that these are
   * rotates by *constant* number of bits! But that's exactly
   * what we need here...
   * 					<appro@fy.chalmers.se>
   */
#  if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#   define ROTATE(a,n)	({ register unsigned int ret;	\
				asm (			\
				"roll %1,%0"		\
				: "=r"(ret)		\
				: "I"(n), "0"((unsigned int)(a))	\
				: "cc");		\
			   ret;				\
			})
#  elif defined(_ARCH_PPC) || defined(_ARCH_PPC64) || \
	defined(__powerpc) || defined(__ppc__) || defined(__powerpc64__)
#   define ROTATE(a,n)	({ register unsigned int ret;	\
				asm (			\
				"rlwinm %0,%1,%2,0,31"	\
				: "=r"(ret)		\
				: "r"(a), "I"(n));	\
			   ret;				\
			})
#  elif defined(__s390x__)
#   define ROTATE(a,n) ({ register unsigned int ret;	\
				asm ("rll %0,%1,%2"	\
				: "=r"(ret)		\
				: "r"(a), "I"(n));	\
			  ret;				\
			})
#  endif
# endif
#endif /* PEDANTIC */

#ifndef ROTATE
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

#ifndef PEDANTIC
# if defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
#  if ((defined(__i386) || defined(__i386__)) && !defined(I386_ONLY)) || \
      (defined(__x86_64) || defined(__x86_64__))
#   if !defined(B_ENDIAN)
    /*
     * This gives ~30-40% performance improvement in SHA-256 compiled
     * with gcc [on P4]. Well, first macro to be frank. We can pull
     * this trick on x86* platforms only, because these CPUs can fetch
     * unaligned data without raising an exception.
     */
#   define HOST_c2l(c,l)	({ unsigned int r=*((const unsigned int *)(c));	\
				   asm ("bswapl %0":"=r"(r):"0"(r));	\
				   (c)+=4; (l)=r;			})
#   define HOST_l2c(l,c)	({ unsigned int r=(l);			\
				   asm ("bswapl %0":"=r"(r):"0"(r));	\
				   *((unsigned int *)(c))=r; (c)+=4; r;	})
#   endif
#  endif
# endif
#endif
#if defined(__s390__) || defined(__s390x__)
# define HOST_c2l(c,l) ((l)=*((const unsigned int *)(c)), (c)+=4, (l))
# define HOST_l2c(l,c) (*((unsigned int *)(c))=(l), (c)+=4, (l))
#endif

#ifndef HOST_c2l
#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))<<24),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))    ),		\
			 l)
#endif
#ifndef HOST_l2c
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>16)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
			 *((c)++)=(unsigned char)(((l)    )&0xff),	\
			 l)
#endif

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

#ifndef PEDANTIC
# if defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
#  if defined(__s390x__)
#   define HOST_c2l(c,l)	({ asm ("lrv	%0,%1"			\
				   :"=d"(l) :"m"(*(const unsigned int *)(c)));\
				   (c)+=4; (l);				})
#   define HOST_l2c(l,c)	({ asm ("strv	%1,%0"			\
				   :"=m"(*(unsigned int *)(c)) :"d"(l));\
				   (c)+=4; (l);				})
#  endif
# endif
#endif
#if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
# ifndef B_ENDIAN
   /* See comment in DATA_ORDER_IS_BIG_ENDIAN section. */
#  define HOST_c2l(c,l)	((l)=*((const unsigned int *)(c)), (c)+=4, l)
#  define HOST_l2c(l,c)	(*((unsigned int *)(c))=(l), (c)+=4, l)
# endif
#endif

#ifndef HOST_c2l
#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))    ),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<<24),		\
			 l)
#endif
#ifndef HOST_l2c
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),	\
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>16)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>24)&0xff),	\
			 l)
#endif

#endif

/*
 * Time for some action:-)
 */
static
int HASH_UPDATE (HASH_CTX *c, const void *data_, size_t len)
	{
	const unsigned char *data=data_;
	unsigned char *p;
	HASH_LONG l;
	size_t n;

	if (len==0) return 1;

	l=(c->Nl+(((HASH_LONG)len)<<3))&0xffffffffUL;
	/* 95-05-24 eay Fixed a bug with the overflow handling, thanks to
	 * Wei Dai <weidai@eskimo.com> for pointing it out. */
	if (l < c->Nl) /* overflow */
		c->Nh++;
	c->Nh+=(HASH_LONG)(len>>29);	/* might cause compiler warning on 16-bit */
	c->Nl=l;

	n = c->num;
	if (n != 0)
		{
		p=(unsigned char *)c->data;

		if (len >= HASH_CBLOCK || len+n >= HASH_CBLOCK)
			{
			memcpy (p+n,data,HASH_CBLOCK-n);
			HASH_BLOCK_DATA_ORDER (c,p,1);
			n      = HASH_CBLOCK-n;
			data  += n;
			len   -= n;
			c->num = 0;
			memset (p,0,HASH_CBLOCK);	/* keep it zeroed */
			}
		else
			{
			memcpy (p+n,data,len);
			c->num += (unsigned int)len;
			return 1;
			}
		}

	n = len/HASH_CBLOCK;
	if (n > 0)
		{
		HASH_BLOCK_DATA_ORDER (c,data,n);
		n    *= HASH_CBLOCK;
		data += n;
		len  -= n;
		}

	if (len != 0)
		{
		p = (unsigned char *)c->data;
		c->num = (unsigned int)len;
		memcpy (p,data,len);
		}
	return 1;
	}

static
void HASH_TRANSFORM (HASH_CTX *c, const unsigned char *data)
	{
	HASH_BLOCK_DATA_ORDER (c,data,1);
	}

static
int HASH_FINAL (unsigned char *md, HASH_CTX *c)
	{
	unsigned char *p = (unsigned char *)c->data;
	size_t n = c->num;

	p[n] = 0x80; /* there is always room for one */
	n++;

	if (n > (HASH_CBLOCK-8))
		{
		memset (p+n,0,HASH_CBLOCK-n);
		n=0;
		HASH_BLOCK_DATA_ORDER (c,p,1);
		}
	memset (p+n,0,HASH_CBLOCK-8-n);

	p += HASH_CBLOCK-8;
#if   defined(DATA_ORDER_IS_BIG_ENDIAN)
	(void)HOST_l2c(c->Nh,p);
	(void)HOST_l2c(c->Nl,p);
#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
	(void)HOST_l2c(c->Nl,p);
	(void)HOST_l2c(c->Nh,p);
#endif
	p -= HASH_CBLOCK;
	HASH_BLOCK_DATA_ORDER (c,p,1);
	c->num=0;
	memset (p,0,HASH_CBLOCK);

	HASH_MAKE_STRING(c,md); 

	return 1;
	}

static
void HASH_MAKE_STRING (HASH_CTX *c, unsigned char *s)
	{
	unsigned long ll;
	ll=c->A;
	(void)HOST_l2c(ll,s);
	ll=c->B;
	(void)HOST_l2c(ll,s);
	ll=c->C;
	(void)HOST_l2c(ll,s);
	ll=c->D;
	(void)HOST_l2c(ll,s);
	return;
	}

static void md5_block_data_order (MD5_CTX *c, const void *p,size_t num);
static
void HASH_BLOCK_DATA_ORDER (MD5_CTX *c, const void *p, size_t num)
	{
	md5_block_data_order (c, p, num);
	return;
	}

#ifndef MD32_REG_T
#if defined(__alpha) || defined(__sparcv9) || defined(__mips)
#define MD32_REG_T long
/*
 * This comment was originaly written for MD5, which is why it
 * discusses A-D. But it basically applies to all 32-bit digests,
 * which is why it was moved to common header file.
 *
 * In case you wonder why A-D are declared as long and not
 * as MD5_LONG. Doing so results in slight performance
 * boost on LP64 architectures. The catch is we don't
 * really care if 32 MSBs of a 64-bit register get polluted
 * with eventual overflows as we *save* only 32 LSBs in
 * *either* case. Now declaring 'em long excuses the compiler
 * from keeping 32 MSBs zeroed resulting in 13% performance
 * improvement under SPARC Solaris7/64 and 5% under AlphaLinux.
 * Well, to be honest it should say that this *prevents* 
 * performance degradation.
 *				<appro@fy.chalmers.se>
 */
#else
/*
 * Above is not absolute and there are LP64 compilers that
 * generate better code if MD32_REG_T is defined int. The above
 * pre-processor condition reflects the circumstances under which
 * the conclusion was made and is subject to further extension.
 *				<appro@fy.chalmers.se>
 */
#define MD32_REG_T int
#endif
#endif
//end md32_common.h

/*
#define	F(x,y,z)	(((x) & (y))  |  ((~(x)) & (z)))
#define	G(x,y,z)	(((x) & (z))  |  ((y) & (~(z))))
*/

/* As pointed out by Wei Dai <weidai@eskimo.com>, the above can be
 * simplified to the code below.  Wei attributes these optimizations
 * to Peter Gutmann's SHS code, and he attributes it to Rich Schroeppel.
 */
#define	F(b,c,d)	((((c) ^ (d)) & (b)) ^ (d))
#define	G(b,c,d)	((((b) ^ (c)) & (d)) ^ (c))
#define	H(b,c,d)	((b) ^ (c) ^ (d))
#define	I(b,c,d)	(((~(d)) | (b)) ^ (c))

#define R0(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+F((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+G((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R2(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+H((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R3(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+I((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

const char MD5_version[]="MD5" OPENSSL_VERSION_PTEXT;

/* Implemented from RFC1321 The MD5 Message-Digest Algorithm
 */

#define INIT_DATA_A (unsigned long)0x67452301L
#define INIT_DATA_B (unsigned long)0xefcdab89L
#define INIT_DATA_C (unsigned long)0x98badcfeL
#define INIT_DATA_D (unsigned long)0x10325476L

int MD5_Init(MD5_CTX *c)
	{
	memset (c,0,sizeof(*c));
	c->A=INIT_DATA_A;
	c->B=INIT_DATA_B;
	c->C=INIT_DATA_C;
	c->D=INIT_DATA_D;
	return 1;
	}

#if !defined(MD5_ASM)
#ifdef X
#undef X
#endif
static
void md5_block_data_order (MD5_CTX *c, const void *data_, size_t num)
	{
	const unsigned char *data=data_;
	register unsigned MD32_REG_T A,B,C,D,l;
#ifndef MD32_XARRAY
	/* See comment in crypto/sha/sha_locl.h for details. */
	unsigned MD32_REG_T	XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
				XX8, XX9,XX10,XX11,XX12,XX13,XX14,XX15;
# define X(i)	XX##i
#else
	MD5_LONG XX[MD5_LBLOCK];
# define X(i)	XX[i]
#endif

	A=c->A;
	B=c->B;
	C=c->C;
	D=c->D;

	for (;num--;)
		{
	HOST_c2l(data,l); X( 0)=l;		HOST_c2l(data,l); X( 1)=l;
	/* Round 0 */
	R0(A,B,C,D,X( 0), 7,0xd76aa478L);	HOST_c2l(data,l); X( 2)=l;
	R0(D,A,B,C,X( 1),12,0xe8c7b756L);	HOST_c2l(data,l); X( 3)=l;
	R0(C,D,A,B,X( 2),17,0x242070dbL);	HOST_c2l(data,l); X( 4)=l;
	R0(B,C,D,A,X( 3),22,0xc1bdceeeL);	HOST_c2l(data,l); X( 5)=l;
	R0(A,B,C,D,X( 4), 7,0xf57c0fafL);	HOST_c2l(data,l); X( 6)=l;
	R0(D,A,B,C,X( 5),12,0x4787c62aL);	HOST_c2l(data,l); X( 7)=l;
	R0(C,D,A,B,X( 6),17,0xa8304613L);	HOST_c2l(data,l); X( 8)=l;
	R0(B,C,D,A,X( 7),22,0xfd469501L);	HOST_c2l(data,l); X( 9)=l;
	R0(A,B,C,D,X( 8), 7,0x698098d8L);	HOST_c2l(data,l); X(10)=l;
	R0(D,A,B,C,X( 9),12,0x8b44f7afL);	HOST_c2l(data,l); X(11)=l;
	R0(C,D,A,B,X(10),17,0xffff5bb1L);	HOST_c2l(data,l); X(12)=l;
	R0(B,C,D,A,X(11),22,0x895cd7beL);	HOST_c2l(data,l); X(13)=l;
	R0(A,B,C,D,X(12), 7,0x6b901122L);	HOST_c2l(data,l); X(14)=l;
	R0(D,A,B,C,X(13),12,0xfd987193L);	HOST_c2l(data,l); X(15)=l;
	R0(C,D,A,B,X(14),17,0xa679438eL);
	R0(B,C,D,A,X(15),22,0x49b40821L);
	/* Round 1 */
	R1(A,B,C,D,X( 1), 5,0xf61e2562L);
	R1(D,A,B,C,X( 6), 9,0xc040b340L);
	R1(C,D,A,B,X(11),14,0x265e5a51L);
	R1(B,C,D,A,X( 0),20,0xe9b6c7aaL);
	R1(A,B,C,D,X( 5), 5,0xd62f105dL);
	R1(D,A,B,C,X(10), 9,0x02441453L);
	R1(C,D,A,B,X(15),14,0xd8a1e681L);
	R1(B,C,D,A,X( 4),20,0xe7d3fbc8L);
	R1(A,B,C,D,X( 9), 5,0x21e1cde6L);
	R1(D,A,B,C,X(14), 9,0xc33707d6L);
	R1(C,D,A,B,X( 3),14,0xf4d50d87L);
	R1(B,C,D,A,X( 8),20,0x455a14edL);
	R1(A,B,C,D,X(13), 5,0xa9e3e905L);
	R1(D,A,B,C,X( 2), 9,0xfcefa3f8L);
	R1(C,D,A,B,X( 7),14,0x676f02d9L);
	R1(B,C,D,A,X(12),20,0x8d2a4c8aL);
	/* Round 2 */
	R2(A,B,C,D,X( 5), 4,0xfffa3942L);
	R2(D,A,B,C,X( 8),11,0x8771f681L);
	R2(C,D,A,B,X(11),16,0x6d9d6122L);
	R2(B,C,D,A,X(14),23,0xfde5380cL);
	R2(A,B,C,D,X( 1), 4,0xa4beea44L);
	R2(D,A,B,C,X( 4),11,0x4bdecfa9L);
	R2(C,D,A,B,X( 7),16,0xf6bb4b60L);
	R2(B,C,D,A,X(10),23,0xbebfbc70L);
	R2(A,B,C,D,X(13), 4,0x289b7ec6L);
	R2(D,A,B,C,X( 0),11,0xeaa127faL);
	R2(C,D,A,B,X( 3),16,0xd4ef3085L);
	R2(B,C,D,A,X( 6),23,0x04881d05L);
	R2(A,B,C,D,X( 9), 4,0xd9d4d039L);
	R2(D,A,B,C,X(12),11,0xe6db99e5L);
	R2(C,D,A,B,X(15),16,0x1fa27cf8L);
	R2(B,C,D,A,X( 2),23,0xc4ac5665L);
	/* Round 3 */
	R3(A,B,C,D,X( 0), 6,0xf4292244L);
	R3(D,A,B,C,X( 7),10,0x432aff97L);
	R3(C,D,A,B,X(14),15,0xab9423a7L);
	R3(B,C,D,A,X( 5),21,0xfc93a039L);
	R3(A,B,C,D,X(12), 6,0x655b59c3L);
	R3(D,A,B,C,X( 3),10,0x8f0ccc92L);
	R3(C,D,A,B,X(10),15,0xffeff47dL);
	R3(B,C,D,A,X( 1),21,0x85845dd1L);
	R3(A,B,C,D,X( 8), 6,0x6fa87e4fL);
	R3(D,A,B,C,X(15),10,0xfe2ce6e0L);
	R3(C,D,A,B,X( 6),15,0xa3014314L);
	R3(B,C,D,A,X(13),21,0x4e0811a1L);
	R3(A,B,C,D,X( 4), 6,0xf7537e82L);
	R3(D,A,B,C,X(11),10,0xbd3af235L);
	R3(C,D,A,B,X( 2),15,0x2ad7d2bbL);
	R3(B,C,D,A,X( 9),21,0xeb86d391L);

	A = c->A += A;
	B = c->B += B;
	C = c->C += C;
	D = c->D += D;
		}
	}
#endif /* !defined(MD5_ASM) */

#if defined(MD5_ASM)
#error "TODO: The MD5_ASM code for x86 will be ported in njit-client sooner or later"
# if defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__INTEL__) || \
     defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64) || \
     defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
static
void md5_block_data_order (MD5_CTX *c, const void *p,size_t num)
	{
	md5_block_asm_data_order(c, p, num);
	return;
	}
# else
# error "md5_block_asm_data_order() has not been supported on your platform"
# endif
#endif /* defined(MD5_ASM) */

int MD5_Update (HASH_CTX *c, const void *data, size_t len)
	{
	return HASH_UPDATE (c, data, len);
	}

void MD5_Transform (HASH_CTX *c, const unsigned char *data)
	{
	HASH_TRANSFORM (c, data);
	return;
	}

int MD5_Final (unsigned char *md, HASH_CTX *c)
	{
	return HASH_FINAL (md, c);
	}

