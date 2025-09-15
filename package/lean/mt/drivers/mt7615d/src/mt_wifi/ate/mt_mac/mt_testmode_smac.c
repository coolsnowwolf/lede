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
	mt_testmode_smac.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif


INT mt_ate_mac_cr_restore(RTMP_ADAPTER *pAd)
{
	/* Data frame protection CR recover */
	MtTestModeRestoreCr(pAd, AGG_PCR);
	/* RTS threshold CR recover */
	MtTestModeRestoreCr(pAd, AGG_PCR1);

	return 0;
}


INT mt_ate_mac_cr_backup_and_set(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 val = 0;

	NdisZeroMemory(&ATECtrl->bk_cr, sizeof(struct _TESTMODE_BK_CR)*MAX_TEST_BKCR_NUM);
	/* Enable data frame protection for test mode */
	MtTestModeBkCr(pAd, AGG_PCR, TEST_MAC_BKCR);
	MAC_IO_READ32(pAd, AGG_PCR, &val);
	val = 0x80008;
	MAC_IO_WRITE32(pAd, AGG_PCR, val);
	/* RTS threshold need to change to 1 for test mode */
	MtTestModeBkCr(pAd, AGG_PCR1, TEST_MAC_BKCR);
	MAC_IO_READ32(pAd, AGG_PCR1, &val);
	/* val &= 0x0FFFFFFF; */
	/* val |= 0x10000000; */
	/* Setting RTS threshold to max value to aviod send RTS in test mode */
	val = 0xFE0FFFFF;
	MAC_IO_WRITE32(pAd, AGG_PCR1, val);

	return 0;
}
