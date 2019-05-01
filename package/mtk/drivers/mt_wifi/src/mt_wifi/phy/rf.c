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
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Init RF Registers\n", __func__));
	if (ops->AsicRfInit)
		ops->AsicRfInit(pAd);
}

