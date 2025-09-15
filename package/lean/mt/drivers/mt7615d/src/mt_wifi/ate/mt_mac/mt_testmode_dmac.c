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
	mt_testmode_dmac.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif


INT mt_ate_mac_cr_restore(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		/* Data frame protection CR recover */
		MtTestModeRestoreCr(pAd, AGG_PCR);
		/* RTS threshold CR recover */
		MtTestModeRestoreCr(pAd, AGG_PCR1);
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeRestoreCr(pAd, AGG_PCR2);
		}
#endif /* DBDC_MODE */
		/* BA related CR recover */
		MtTestModeRestoreCr(pAd, AGG_AALCR0);
		MtTestModeRestoreCr(pAd, AGG_AALCR1);
		MtTestModeRestoreCr(pAd, AGG_AALCR2);
		MtTestModeRestoreCr(pAd, AGG_AALCR3);
		MtTestModeRestoreCr(pAd, AGG_AWSCR0);
		MtTestModeRestoreCr(pAd, AGG_AWSCR1);
		/* Station pause CR recover */
		MtTestModeRestoreCr(pAd, STATION_PAUSE0);
		MtTestModeRestoreCr(pAd, STATION_PAUSE1);
		MtTestModeRestoreCr(pAd, STATION_PAUSE2);
		MtTestModeRestoreCr(pAd, STATION_PAUSE3);
		/* Enable HW BAR feature */
		MtTestModeRestoreCr(pAd, AGG_MRCR);
		/* IPG related CR recover */
		MtTestModeRestoreCr(pAd, TMAC_TRCR0);
		MtTestModeRestoreCr(pAd, TMAC_ICR_BAND_0);
		MtTestModeRestoreCr(pAd, ARB_DRNGR0);
		MtTestModeRestoreCr(pAd, ARB_DRNGR1);
		MtTestModeRestoreCr(pAd, ARB_WMMAC01);
		MtTestModeRestoreCr(pAd, ARB_WMMAC11);
		MtTestModeRestoreCr(pAd, ARB_WMMAC21);
		MtTestModeRestoreCr(pAd, ARB_WMMAC31);
		MtTestModeRestoreCr(pAd, ARB_WMMALTX0);
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeRestoreCr(pAd, TMAC_TRCR1);
			MtTestModeRestoreCr(pAd, TMAC_ICR_BAND_1);
			MtTestModeRestoreCr(pAd, ARB_WMMALTX1);
		}
#endif /* DBDC_MODE */
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return 0;
}


INT mt_ate_mac_cr_backup_and_set(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	NdisZeroMemory(&ATECtrl->bk_cr, sizeof(struct _TESTMODE_BK_CR)*MAX_TEST_BKCR_NUM);
#if defined(MT7615) || defined(MT7622)
	/* TODO: check if following operation also need to do for other chips */
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		UINT32 val = 0;
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
#ifdef DBDC_MODE
		/* RTS threshold disable for band1 */
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeBkCr(pAd, AGG_PCR2, TEST_MAC_BKCR);
			MAC_IO_READ32(pAd, AGG_PCR2, &val);
			val = 0xFE0FFFFF;
			MAC_IO_WRITE32(pAd, AGG_PCR2, val);
		}
#endif /* DBDC_MODE */
		/* BA related CR backup */
		MtTestModeBkCr(pAd, AGG_AALCR0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AALCR1, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AALCR2, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AALCR3, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AWSCR0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AWSCR1, TEST_MAC_BKCR);
		/* Station pause CR backup for TX after reset WTBL */
		MtTestModeBkCr(pAd, STATION_PAUSE0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE1, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE2, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE3, TEST_MAC_BKCR);
		/* HW BAR feature */
		MtTestModeBkCr(pAd, AGG_MRCR, TEST_MAC_BKCR);
		MAC_IO_READ32(pAd, AGG_MRCR, &val);
		val &= ~BAR_TX_CNT_LIMIT_MASK;
		val |= BAR_TX_CNT_LIMIT(0);
		MAC_IO_WRITE32(pAd, AGG_MRCR, val);
		/* IPG related CR back up */
		MtTestModeBkCr(pAd, TMAC_TRCR0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, TMAC_ICR_BAND_0, TEST_MAC_BKCR);    /* IFS CR, for SIFS/SLOT time control */
		MtTestModeBkCr(pAd, ARB_DRNGR0, TEST_MAC_BKCR);         /* For fixing backoff random number */
		MtTestModeBkCr(pAd, ARB_DRNGR1, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, ARB_WMMAC01, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, ARB_WMMAC11, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, ARB_WMMAC21, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, ARB_WMMAC31, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, ARB_WMMALTX0, TEST_MAC_BKCR);
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeBkCr(pAd, TMAC_TRCR1, TEST_MAC_BKCR);
			MtTestModeBkCr(pAd, TMAC_ICR_BAND_1, TEST_MAC_BKCR);
			MtTestModeBkCr(pAd, ARB_WMMALTX1, TEST_MAC_BKCR);
		}
#endif /* DBDC_MODE */
	}
#endif /* defined(MT7615) || defined(MT7622) */
	return 0;
}


UINT32 agg_cnt_array[] = {AGG_AALCR0, AGG_AALCR1, AGG_AALCR2, AGG_AALCR3};
INT mt_ate_ampdu_ba_limit(RTMP_ADAPTER *pAd, UINT32 band_idx, UINT8 agg_limit)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 value;
	INT wdev_idx = 0;
	UCHAR WmmIdx = 0xFF;
#if !defined(COMPOS_TESTMODE_WIN)
	struct wifi_dev *pWdev = NULL;

	wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);
	pWdev = pAd->wdev_list[wdev_idx];

	if (pWdev) {
		WmmIdx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wmm_idx);

		if (WmmIdx > 3) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wdev_idx=%d, invalid WmmIdx=%d, reset to 0!\n",
				__func__, wdev_idx, WmmIdx));
			WmmIdx = 0xFF;
		}
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid WDEV, reset WmmIdx to 0!\n", __func__));
		WmmIdx = 0xFF;
	}

#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: wdev_idx=%d, WmmIdx=%d\n",
			  __func__, wdev_idx, WmmIdx));
	value = ((agg_limit & 0x3F) << 24)
		| ((agg_limit & 0x3F) << 16)
		| ((agg_limit & 0x3F) << 8)
		| ((agg_limit & 0x3F) << 0);

	if (WmmIdx <= 3)
		MAC_IO_WRITE32(pAd, agg_cnt_array[WmmIdx], value);
	else {
		MAC_IO_WRITE32(pAd, AGG_AALCR0, value);
		MAC_IO_WRITE32(pAd, AGG_AALCR1, value);
		MAC_IO_WRITE32(pAd, AGG_AALCR2, value);
		MAC_IO_WRITE32(pAd, AGG_AALCR3, value);
	}

	value = 0x0;
	MAC_IO_WRITE32(pAd, AGG_AWSCR0, value);

	return 0;
}


INT mt_ate_set_sta_pause_cr(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	UINT32 value;

	/* Set station pause CRs to 0 for TX after reset WTBL */
	/* The CR meaning in normal mode is that stop to TX packet when STA disconnect */
	value = 0x0;
	MAC_IO_WRITE32(pAd, STATION_PAUSE0, value);
	MAC_IO_WRITE32(pAd, STATION_PAUSE1, value);
	MAC_IO_WRITE32(pAd, STATION_PAUSE2, value);
	MAC_IO_WRITE32(pAd, STATION_PAUSE3, value);

	return ret;
}
