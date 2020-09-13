/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2005, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    wince.h

    Abstract:
    WinCE generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Rory Chen   04-14-2005    created

*/
#ifndef __WINCE_H__
#define __WINCE_H__

#ifdef UNDER_CE

#include	"giisr.h"
#include	<ddkreg.h>
#include	<ceddk.h>


#if DBG

#define ZONE_ERROR_BIT		0
#define ZONE_WARN_BIT		1
#define ZONE_FUNCTION_BIT	2
#define ZONE_INIT_BIT		3
#define ZONE_INTR_BIT		4
#define ZONE_RCV_BIT		5
#define ZONE_XMIT_BIT		6
#define ZONE_LINK_BIT		7

#define ZONE_ERROR_MASK		(1 << ZONE_ERROR_BIT)
#define ZONE_WARN_MASK		(1 << ZONE_WARN_BIT)
#define ZONE_FUNCTION_MASK	(1 << ZONE_FUNCTION_BIT)
#define ZONE_INIT_MASK		(1 << ZONE_INIT_BIT)
#define ZONE_INTR_MASK		(1 << ZONE_INTR_BIT)
#define ZONE_RCV_MASK		(1 << ZONE_RCV_BIT)
#define ZONE_XMIT_MASK		(1 << ZONE_XMIT_BIT)
#define ZONE_LINK_MASK		(1 << ZONE_LINK_BIT)

#define ZONE_ERROR		DEBUGZONE(ZONE_ERROR_BIT)
#define ZONE_WARN		DEBUGZONE(ZONE_WARN_BIT)
#define ZONE_FUNCTION	DEBUGZONE(ZONE_FUNCTION_BIT)
#define ZONE_INIT		DEBUGZONE(ZONE_INIT_BIT)
#define ZONE_INTR		DEBUGZONE(ZONE_INTR_BIT)
#define ZONE_RCV		DEBUGZONE(ZONE_RCV_BIT)
#define ZONE_XMIT		DEBUGZONE(ZONE_XMIT_BIT)
#define ZONE_LINK		DEBUGZONE(ZONE_LINK_BIT)

#endif /* DBG */


NDIS_STATUS LoadISR(
	IN	PRTMP_ADAPTER		pAd,
	IN	NDIS_HANDLE			WrapperConfigurationContext);

NDIS_STATUS UnloadISR(
	IN	PRTMP_ADAPTER		pAd);

VOID	RTMPCopyUnicodeString(
	IN OUT PUNICODE_STRING  DestinationString,
	IN PUNICODE_STRING  SourceString);

#endif	/*UNDER_CE */

#endif	/*__WINCE_H__ */
