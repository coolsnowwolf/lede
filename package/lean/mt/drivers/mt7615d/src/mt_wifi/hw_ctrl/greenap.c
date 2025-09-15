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
	greenap.c

	Abstract:
	Ralink Wireless driver green ap related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

#ifdef GREENAP_SUPPORT
/*
*
*/
VOID greenap_init(struct greenap_ctrl *greenap)
{
	greenap->cap = FALSE;
	greenap->allow = FALSE;
	RTMP_SPIN_LOCK(&greenap->lock);
	greenap->suspend = 0;
	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*   update greenap on/off immediately
*/
static VOID greenap_update(
	RTMP_ADAPTER * pAd,
	BOOLEAN previous_greenap_active,
	BOOLEAN greenap_enter,
	UINT8 band_idx)
{
	struct greenap_on_off_ctrl greenap_on_off = {0};

	greenap_on_off.band_idx = band_idx;

	if (previous_greenap_active) {
		if (greenap_enter) {
			/* Do nothing */
		} else {
			/* Exit GreenAP */
			RTMP_CHIP_DISABLE_GREENAP(pAd, &greenap_on_off);
		}
	} else {
		if (greenap_enter) {
			/* Enter GreenAP */
			RTMP_CHIP_ENABLE_GREENAP(pAd, &greenap_on_off);
		} else {
			/* Do nothing */
		}
	}
}


/*
*
*/
VOID greenap_exit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct greenap_ctrl *greenap)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	RTMP_SPIN_LOCK(&greenap->lock);
	greenap_update(pAd, IsHcGreenAPActiveByWdev(wdev), FALSE, HcGetBandByWdev(wdev));
	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*
*/
VOID greenap_show(RTMP_ADAPTER *pAd, struct greenap_ctrl *greenap)
{
	UINT32  value = 0;
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev == NULL)
			continue;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tband(%d), wdev[%d], type(0x%x), up(%d)\n",
				  HcGetBandByWdev(wdev),
				  i,
				  wdev->wdev_type,
				  wdev->if_up_down_state));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tGREENAP::greenap_cap(%d), greenap_allow(%d), dbdc_mode(%d)\n",
			  greenap_get_capability(greenap),
			  greenap_get_allow_status(greenap),
			  pAd->CommonCfg.dbdc_mode));

	for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
		MAC_IO_READ32(pAd, ((band_idx == 0) ? RMAC_RMCR : RMAC_RMCR_BAND_1), &value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tGREENAP::band_idx(%d), greenap_active(%d), rx_stream(0x%x), mode(0x%x)\n",
				  band_idx,
				  IsHcGreenAPActiveByBand(pAd, band_idx),
				  ((band_idx == 0) ? (value >> 22) & 0xf : (value >> 22) & 0xf),
				  ((value >> 20) & 0x3)));
	}
}


/*
*   B/G mode
*/
static inline BOOLEAN greenap_rule_1(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN enetr_greenap = TRUE;

	if ((pEntry->MaxHTPhyMode.field.MODE == MODE_CCK) ||
		(pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM))
		enetr_greenap = TRUE;
	else
		enetr_greenap = FALSE;

#ifdef TXBF_SUPPORT

	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE)
		enetr_greenap = FALSE;

#endif /* TXBF_SUPPORT */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, enetr_greenap(%d), B(%d), G(%d)\n",
			  __func__,
			  enetr_greenap,
			  (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK) ? TRUE : FALSE,
			  (pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) ? TRUE : FALSE));
	return enetr_greenap;
}


/*
*   1*1 HT & !eBF
*/
static inline BOOLEAN greenap_rule_2(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN enetr_greenap = TRUE;

	if ((pEntry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
		(pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) {
		if (pEntry->SupportHTMCS <= ((1 << MCS_8) - 1)) {
#ifdef TXBF_SUPPORT

			if (pAd->CommonCfg.RegTransmitSetting.field.TxBF == TRUE) {
				if (pEntry->eTxBfEnCond == 0)
					enetr_greenap = TRUE;
				else
					enetr_greenap = FALSE;
			} else
#endif /* TXBF_SUPPORT */
				enetr_greenap = TRUE;
		} else
			enetr_greenap = FALSE;

#ifdef TXBF_SUPPORT

		if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE)
			enetr_greenap = FALSE;

#endif /* TXBF_SUPPORT */
	} else
		enetr_greenap = FALSE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, enetr_greenap(%d), AP_e_i_BF(%d,%d), STA_eBF(%d)HT(%d)MSC(0x%x)\n",
			  __func__,
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  pAd->CommonCfg.RegTransmitSetting.field.TxBF,
			  pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn,
			  pEntry->eTxBfEnCond,
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  ((pEntry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
			   (pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) ? TRUE : FALSE,
			  ((pEntry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
			   (pEntry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) ? pEntry->SupportHTMCS : 0));
	return enetr_greenap;
}


/*
*   1*1 VHT & !eBF
*/
static inline BOOLEAN greenap_rule_3(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN enetr_greenap = TRUE;
#ifdef DOT11_VHT_AC

	if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) {
		if ((pEntry->SupportVHTMCS1SS != 0)
			&& (pEntry->SupportVHTMCS2SS == 0)
			&& (pEntry->SupportVHTMCS3SS == 0)
			&& (pEntry->SupportVHTMCS4SS == 0)) {
#ifdef TXBF_SUPPORT

			if (pAd->CommonCfg.RegTransmitSetting.field.TxBF == TRUE) {
				if (pEntry->eTxBfEnCond == 0)
					enetr_greenap = TRUE;
				else
					enetr_greenap = FALSE;
			} else
#endif /* TXBF_SUPPORT */
				enetr_greenap = TRUE;

			if ((pEntry->vht_cap_ie.vht_cap.ch_width == 1) || (pEntry->vht_cap_ie.vht_cap.ch_width == 2))
				enetr_greenap = FALSE;

		} else
			enetr_greenap = FALSE;

#ifdef TXBF_SUPPORT

		if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE)
			enetr_greenap = FALSE;

#endif /* TXBF_SUPPORT */
	} else
		enetr_greenap = FALSE;

#else
	enetr_greenap = FALSE;
#endif /* DOT11_VHT_AC */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, enetr_greenap(%d), AP_e_i_BF(%d,%d), STA_eBF(%d)VHT(%d)MCS(0x%x,0x%x,0x%x,0x%x)\n",
			  __func__,
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  pAd->CommonCfg.RegTransmitSetting.field.TxBF,
			  pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn,
			  pEntry->eTxBfEnCond,
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) ? TRUE : FALSE,
#ifdef DOT11_VHT_AC
			  pEntry->SupportVHTMCS1SS,
			  pEntry->SupportVHTMCS2SS,
			  pEntry->SupportVHTMCS3SS,
			  pEntry->SupportVHTMCS4SS
#else
			  0,
			  0,
			  0,
			  0
#endif /* DOT11_VHT_AC */
			 ));
	return enetr_greenap;
}


/*
*
*/
static BOOLEAN greenap_rule_check(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN enetr_greenap = FALSE;

	if (pEntry == NULL)
		return FALSE;

	enetr_greenap |= greenap_rule_1(pAd, pEntry);
	enetr_greenap |= greenap_rule_2(pAd, pEntry);
	enetr_greenap |= greenap_rule_3(pAd, pEntry);
	return enetr_greenap;
}


/*
*
*/
BOOLEAN greenap_check_when_if_down_up(RTMP_ADAPTER *pAd)
{
	BOOLEAN allow = FALSE;
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);
	struct greenap_ctrl *greenap = NULL;

	greenap = &pAd->ApCfg.greenap;

	if (greenap == NULL)
		return FALSE;

	if (greenap_get_capability(greenap)) {
		RTMP_SPIN_LOCK(&greenap->lock);

		if (greenap_get_suspend_status(greenap)) {
			RTMP_SPIN_UNLOCK(&greenap->lock);
			return TRUE;
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
		greenap_check_allow_status(pAd, greenap);
		allow = greenap_get_allow_status(greenap);

		for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
			greenap_check_peer_connection_status(
				pAd,
				band_idx,
				IsHcGreenAPActiveByBand(pAd, band_idx),
				allow);
		}

		RTMP_SPIN_UNLOCK(&greenap->lock);
	}

	return TRUE;
}


/*
*   For existed connection and update GreenAP state
*/
VOID greenap_check_peer_connection_status(
	RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	BOOLEAN previous_greenap_active,
	BOOLEAN greenap_allow)
{
	BOOLEAN greenap_enter = TRUE;
	UCHAR i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry)) &&
			(pEntry->ConnectionType == CONNECTION_INFRA_STA) &&
			(HcGetBandByWdev(pEntry->wdev) == band_idx)) {
			if (greenap_rule_check(pAd, pEntry))
				greenap_enter = TRUE;
			else {
				greenap_enter = FALSE;
				break;
			}
		} else {
			/* Do nothing and continue next pEntry */
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s::band_idx(%d), previous_greenap_active(%d),  greenap_allow(%d), greenap_enter(%d)\n",
			  __func__,
			  band_idx,
			  previous_greenap_active,
			  greenap_allow,
			  greenap_enter));
	greenap_update(pAd, previous_greenap_active, (greenap_allow & greenap_enter), band_idx);
}


/*
*
*/
VOID greenap_check_peer_connection_at_link_up_down(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct greenap_ctrl *greenap)
{
	UCHAR   band_idx = HcGetBandByWdev(wdev);
	BOOLEAN previous_greenap_active = IsHcGreenAPActiveByWdev(wdev);
	BOOLEAN greenap_allow = greenap_get_allow_status(greenap);

	RTMP_SPIN_LOCK(&greenap->lock);

	if (greenap_get_suspend_status(greenap)) {
		RTMP_SPIN_UNLOCK(&greenap->lock);
		return;
	}

	greenap_check_peer_connection_status(
		pAd,
		band_idx,
		previous_greenap_active,
		greenap_allow);
	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*
*/
BOOLEAN greenap_get_suspend_status(struct greenap_ctrl *greenap)
{
	return (greenap->suspend != 0) ? TRUE : FALSE;
}


/*
*
*/
VOID greenap_suspend(RTMP_ADAPTER *pAd, struct greenap_ctrl *greenap, UINT32 reason)
{
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, reason(0x%x)\n",
			  __func__,
			  reason));
	RTMP_SPIN_LOCK(&greenap->lock);
	greenap->suspend |= reason;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, greenap->suspend(0x%x)\n",
			  __func__,
			  greenap->suspend));

	for (band_idx = 0; band_idx < amount_of_band; band_idx++)
		greenap_update(pAd, IsHcGreenAPActiveByBand(pAd, band_idx), FALSE, band_idx);

	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*
*/
VOID greenap_resume(RTMP_ADAPTER *pAd, struct greenap_ctrl *greenap, UINT32 reason)
{
	BOOLEAN allow = FALSE;
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, reason(0x%x)\n",
			  __func__,
			  reason));
	RTMP_SPIN_LOCK(&greenap->lock);
	/* During scan, ifreface might be add or delete, check allow here again */
	greenap_check_allow_status(pAd, greenap);
	allow = greenap_get_allow_status(greenap);

	/* Check all pEntry if meet GreenAP mode */
	for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
		greenap_check_peer_connection_status(
			pAd,
			band_idx,
			IsHcGreenAPActiveByBand(pAd, band_idx),
			allow);
	}

	greenap->suspend &= ~reason;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s, greenap->suspend(0x%x)\n",
			  __func__,
			  greenap->suspend));
	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*
*/
VOID greenap_check_allow_status(RTMP_ADAPTER *pAd, struct greenap_ctrl *greenap)
{
	BOOLEAN ap_mode_if_up = FALSE;
	BOOLEAN non_ap_mode_if_up = FALSE;
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev == NULL)
			continue;

		if ((wdev->wdev_type == WDEV_TYPE_AP) && wdev->if_up_down_state) {
			ap_mode_if_up = TRUE;
			break;
		}
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev == NULL)
			continue;

		if ((wdev->wdev_type != WDEV_TYPE_AP) && wdev->if_up_down_state) {
			non_ap_mode_if_up = TRUE;
			break;
		}
	}

	greenap->allow = (ap_mode_if_up & (!non_ap_mode_if_up));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ap_mode_if_up(%d), non_ap_mode_if_up(%d) --> greenap_allow(%d)\n",
			  ap_mode_if_up,
			  non_ap_mode_if_up,
			  greenap_get_allow_status(greenap)));
}


/*
*
*/
BOOLEAN greenap_get_allow_status(struct greenap_ctrl *greenap)
{
	return greenap->allow;
}


/*
*
*/
VOID greenap_set_capability(struct greenap_ctrl *greenap, BOOLEAN greenap_cap)
{
	greenap->cap = greenap_cap;
}


/*
*
*/
BOOLEAN greenap_get_capability(struct greenap_ctrl *greenap)
{
	return greenap->cap;
}


/*
*
*/
VOID greenap_proc(RTMP_ADAPTER *pAd, struct greenap_ctrl *greenap, BOOLEAN greenap_cap_on)
{
	BOOLEAN allow = FALSE;
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);

	RTMP_SPIN_LOCK(&greenap->lock);

	if (greenap_get_suspend_status(greenap)) {
		RTMP_SPIN_UNLOCK(&greenap->lock);
		return;
	}

	if (!greenap_cap_on) {
		greenap_set_capability(greenap, FALSE);

		for (band_idx = 0; band_idx < amount_of_band; band_idx++)
			greenap_update(pAd, IsHcGreenAPActiveByBand(pAd, band_idx), FALSE, band_idx);
	} else if (greenap_cap_on) {
		greenap_set_capability(greenap, TRUE);
		greenap_check_allow_status(pAd, greenap);
		allow = greenap_get_allow_status(greenap);

		/* use case: No GreeAP on --> sta connected --> GreenAP on */
		for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
			greenap_check_peer_connection_status(
				pAd,
				band_idx,
				IsHcGreenAPActiveByBand(pAd, band_idx),
				allow);
		}
	}

	RTMP_SPIN_UNLOCK(&greenap->lock);
}


/*
*
*/
VOID enable_greenap(RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("band_idx(%d), enable_greenap\n", greenap->band_idx));
	HcSetGreenAPActiveByBand(pAd, greenap->band_idx, TRUE);
	RTMP_GREENAP_ON_OFF_CTRL(pAd, greenap->band_idx, TRUE);
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(pAd, greenap->band_idx, TRUE, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
}


/*
*
*/
VOID disable_greenap(RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("band_idx(%d), disable_greenap\n", greenap->band_idx));
	HcSetGreenAPActiveByBand(pAd, greenap->band_idx, FALSE);
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(pAd, greenap->band_idx, FALSE, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	RTMP_GREENAP_ON_OFF_CTRL(pAd, greenap->band_idx, FALSE);
}

VOID EnableAPMIMOPSv2(RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	bbp_set_mmps(pAd, greenap->reduce_core_power);
	rtmp_mac_set_mmps(pAd, greenap->reduce_core_power);
	HcSetGreenAPActiveByBand(pAd, 0, TRUE);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("EnableAPMIMOPSNew, 30xx changes the # of antenna to 1\n"));
}


VOID DisableAPMIMOPSv2(RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off)
{
	bbp_set_mmps(pAd, FALSE);
	rtmp_mac_set_mmps(pAd, FALSE);
	HcSetGreenAPActiveByBand(pAd, 0, FALSE);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("DisableAPMIMOPSNew, 30xx reserve only one antenna\n"));
}


VOID EnableAPMIMOPSv1(
	IN RTMP_ADAPTER *pAd,
	IN struct greenap_on_off_ctrl *greenap_on_off)
{
	ULONG TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/
	struct wifi_dev *wdev = get_default_wdev(pAd);

	if (wdev->channel > 14)
		TxPinCfg = 0x00050F05;

	TxPinCfg &= 0xFFFFFFF3;
	TxPinCfg &= 0xFFFFF3FF;
	HcSetGreenAPActiveByBand(pAd, 0, TRUE);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Run with BW_20\n"));
	/* Set BBP registers to BW20 */
	wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);

	/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
	if (pAd->Antenna.field.RxPath > 1 || pAd->Antenna.field.TxPath > 1) {
		/*Tx/Rx Stream*/
		bbp_set_txdac(pAd, 0);
		bbp_set_rxpath(pAd, 1);
		/* Need to check in RT chip */
		/*RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);*/
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n"));
}


VOID DisableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd,
	IN struct greenap_on_off_ctrl *greenap_on_off)
{
	ULONG	TxPinCfg = 0x00050F0A; /* Gary 2007/08/09 0x050A0A */
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
	UCHAR ht_bw = wlan_config_get_ht_bw(wdev);

	if (wdev->channel > 14)
		TxPinCfg = 0x00050F05;

	/* Turn off unused PA or LNA when only 1T or 1R*/
	if (pAd->Antenna.field.TxPath == 1)
		TxPinCfg &= 0xFFFFFFF3;

	if (pAd->Antenna.field.RxPath == 1)
		TxPinCfg &= 0xFFFFF3FF;

	HcSetGreenAPActiveByBand(pAd, 0, FALSE);

	if ((ht_bw == BW_40) && (wdev->channel != 14)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Run with BW_40\n"));

		/* Set CentralChannel to work for BW40 */
		if (ext_cha == EXTCHA_ABOVE) {
			ext_cha = EXTCHA_ABOVE;
		} else if ((wdev->channel > 2) && (ext_cha == EXTCHA_BELOW)) {
			ext_cha = EXTCHA_BELOW;
		}

		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}

	/*Tx Stream*/
	if (WMODE_CAP_N(wdev->PhyMode) && (pAd->Antenna.field.TxPath == 2))
		bbp_set_txdac(pAd, 2);
	else
		bbp_set_txdac(pAd, 0);

	/*Rx Stream*/
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("DisableAPMIMOPS, 305x/28xx reserve only one antenna\n"));
}
#endif /* GREENAP_SUPPORT */
