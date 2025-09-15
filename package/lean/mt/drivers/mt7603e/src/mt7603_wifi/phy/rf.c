/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rf.c
*/

#include "rt_config.h"

VOID InitRFRegisters(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Init RF Registers\n", __FUNCTION__));
	if (pAd->chipOps.AsicRfInit)
		pAd->chipOps.AsicRfInit(pAd);
}

