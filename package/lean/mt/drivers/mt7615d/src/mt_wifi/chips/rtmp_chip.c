/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt_chip.c

	Abstract:
	Ralink Wireless driver CHIP related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

/*
========================================================================
Routine Description:
	write high memory.
	if firmware do not support auto high/low memory switching, we should switch to high memory by ourself.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteHighMemory(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN UINT32 Value,
	IN UINT8 Unit)
{
}


/*
========================================================================
Routine Description:
	write memory

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"
Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteMemory(
	IN	RTMP_ADAPTER	*pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit)
{
	switch (Unit) {
	case 1:
		RTMP_IO_WRITE8(pAd, Offset, Value);
		break;

	case 2:
		RTMP_IO_WRITE16(pAd, Offset, Value);
		break;

	case 4:
		RTMP_IO_WRITE32(pAd, Offset, Value);

	default:
		break;
	}
}

/*
========================================================================
Routine Description:
	Initialize normal beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
	pChipCap->BcnMaxHwNum = 8;
	pChipCap->BcnMaxNum = (pChipCap->BcnMaxHwNum - MAX_MESH_NUM - MAX_APCLI_NUM);
	pChipCap->BcnMaxHwSize = 0x1000;
	pChipCap->BcnBase[0] = 0x7800;
	pChipCap->BcnBase[1] = 0x7A00;
	pChipCap->BcnBase[2] = 0x7C00;
	pChipCap->BcnBase[3] = 0x7E00;
	pChipCap->BcnBase[4] = 0x7200;
	pChipCap->BcnBase[5] = 0x7400;
	pChipCap->BcnBase[6] = 0x5DC0;
	pChipCap->BcnBase[7] = 0x5BC0;

	/*
		If the MAX_MBSSID_NUM is larger than 6,
		it shall reserve some WCID space(wcid 222~253) for beacon frames.
		-	these wcid 238~253 are reserved for beacon#6(ra6).
		-	these wcid 222~237 are reserved for beacon#7(ra7).
	*/
	if (pChipCap->BcnMaxNum == 8)
		pChipCap->WcidHwRsvNum = 222;
	else if (pChipCap->BcnMaxNum == 7)
		pChipCap->WcidHwRsvNum = 238;
	else
		pChipCap->WcidHwRsvNum = 255;

	ops->BeaconUpdate = RtmpChipWriteMemory;
}

UINT8 NICGetBandSupported(RTMP_ADAPTER *pAd)
{
	if (BOARD_IS_5G_ONLY(pAd))
		return RFIC_5GHZ;
	else if (BOARD_IS_2G_ONLY(pAd))
		return RFIC_24GHZ;
	else if (RFIC_IS_5G_BAND(pAd))
		return RFIC_DUAL_BAND;
	else
		return RFIC_24GHZ;
}


INT WaitForAsicReady(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT AsicGetMacVersion(RTMP_ADAPTER *pAd)
{
	UINT32 reg = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* TODO: shiang-7603 */
	if (cap->hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return FALSE;
	}

	if (WaitForAsicReady(pAd) == TRUE) {
		RTMP_IO_READ32(pAd, reg, &pAd->MACVersion);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MACVersion[Ver:Rev]=0x%08x : 0x%08x\n",
				 pAd->MACVersion, pAd->ChipID));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() failed!\n", __func__));
		return FALSE;
	}
}


/*
========================================================================
Routine Description:
	Initialize chip related information.

Arguments:
	pCB				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
int RtmpChipOpsHook(VOID *pCB)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pCB;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	int ret = 0;

#ifdef MT_DFS_SUPPORT
	/* Initial new DFS channel list */
	DfsSetNewChInit(pAd);
#endif

	/* sanity check */
	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	/* TODO: shiang-7603 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7615(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(%d): Not support for HIF_MT yet! MACVersion=0x%x\n",
				  __func__, __LINE__, pAd->MACVersion));
	}

	if (pAd->MACVersion == 0xffffffff)
		return -1;

	/* default init */
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_LEGACY);
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
		/* wilsonl, need DE provide info */
#ifndef MT7622
		RTMP_SYS_IO_READ32(0xb000000c, &pAd->CommonCfg.CID);
		RTMP_SYS_IO_READ32(0xb0000000, &pAd->CommonCfg.CN);
#endif
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CN: %lx\tCID = %lx\n",
				 pAd->CommonCfg.CN, pAd->CommonCfg.CID));
	}

#endif /* RTMP_RBUS_SUPPORT */
	/*initial chip hook function*/
	WfSysPreInit(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipCap->bbpRegTbSize));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Chip VCO calibration mode = %d!\n", pChipCap->FlgIsVcoReCalMode));
#ifdef DOT11W_PMF_SUPPORT
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[PMF] Encryption mode = %d\n", pChipCap->FlgPMFEncrtptMode));
#endif /* DOT11W_PMF_SUPPORT */
	return ret;
}


