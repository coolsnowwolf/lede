/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All Related Structure & Definition for UBICOM platform.

	Only used in UTIL module.

***************************************************************************/

#ifndef __VR_UBICOM_H__
#define __VR_UBICOM_H__

#ifdef PLATFORM_UBM_IPX8

#include <asm/cachectl.h>

#undef RTMP_UTIL_DCACHE_FLUSH
#define RTMP_UTIL_DCACHE_FLUSH(__AddrStart, __Size)						\
	flush_dcache_range((ULONG)(__AddrStart),							\
					   (ULONG)(((UCHAR *)(__AddrStart)) + __Size - 1))

#endif /* PLATFORM_UBM_IPX8 */

#endif /* __VR_UBICOM_H__ */

/* End of vrut_ubm.h */

