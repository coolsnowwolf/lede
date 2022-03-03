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
	hw_ctrl.c
*/
#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"
#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef WSC_LED_SUPPORT
#include "rt_led.h"
#endif /* WSC_LED_SUPPORT */
#endif


static NTSTATUS HwCtrlUpdateRtsThreshold(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct rts_thld *rts = (struct rts_thld *)CMDQelmt->buffer;

	AsicUpdateRtsThld(pAd, rts->wdev, rts->pkt_thld, rts->len_thld, rts->retry_limit);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlUpdateProtect(RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	UINT32 mode = 0, wdev_idx = 0;
	struct wifi_dev *wdev = NULL;
	MT_PROTECT_CTRL_T  protect;
#ifdef DBDC_MODE
	MT_PROTECT_CTRL_T  protect_5g;
#endif /* DBDC_MODE */
	os_zero_mem(&protect, sizeof(MT_PROTECT_CTRL_T));
#ifdef DBDC_MODE
	os_zero_mem(&protect_5g, sizeof(MT_PROTECT_CTRL_T));
#endif /* DBDC_MODE */

	if (arch_ops->archUpdateProtect == NULL) {
		AsicNotSupportFunc(pAd, __func__);
		return NDIS_STATUS_FAILURE;
	}

	do {
		wdev = pAd->wdev_list[wdev_idx];

		if (wdev == NULL)
			break;

		mode = wdev->protection;
#ifdef DBDC_MODE

		if ((pAd->CommonCfg.dbdc_mode == TRUE) && (HcGetBandByWdev(wdev) == DBDC_BAND1)) {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT

			if ((pAd->bApCliCertTest == TRUE) && (wdev->wdev_type == WDEV_TYPE_APCLI))
				os_zero_mem(&protect_5g, sizeof(MT_PROTECT_CTRL_T));

#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			if (mode & SET_PROTECT(ERP))
				protect_5g.erp_mask = ERP_OMAC_ALL;

			if (mode & SET_PROTECT(NON_MEMBER_PROTECT)) {
				protect_5g.mix_mode = 1;
				protect_5g.gf = 1;
				protect_5g.bw40 = 1;
			}

			if (mode & SET_PROTECT(HT20_PROTECT))
				protect_5g.bw40 = 1;

			if (mode & SET_PROTECT(NON_HT_MIXMODE_PROTECT)) {
				protect_5g.mix_mode = 1;
				protect_5g.gf = 1;
				protect_5g.bw40 = 1;
			}

			if (mode & SET_PROTECT(GREEN_FIELD_PROTECT))
				protect_5g.gf = 1;

			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
				protect_5g.long_nav = 1;

			if (mode & SET_PROTECT(LONG_NAV_PROTECT))
				protect_5g.long_nav = 1;

			if (mode & SET_PROTECT(RIFS_PROTECT)) {
				protect_5g.long_nav = 1;
				protect_5g.rifs = 1;
			}

			if (mode & SET_PROTECT(FORCE_RTS_PROTECT)) {
				arch_ops->archUpdateRtsThld(pAd, wdev, 0, 1, wlan_operate_get_rts_retry_limit(wdev));
				goto end;
			}

			protect_5g.band_idx = DBDC_BAND1;
		} else
#endif /* DBDC_MODE */
		{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT

			if ((pAd->bApCliCertTest == TRUE) && (wdev->wdev_type == WDEV_TYPE_APCLI))
				os_zero_mem(&protect, sizeof(MT_PROTECT_CTRL_T));

#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			if (mode & SET_PROTECT(ERP))
				protect.erp_mask = ERP_OMAC_ALL;

			if (mode & SET_PROTECT(NON_MEMBER_PROTECT)) {
				protect.mix_mode = 1;
				protect.gf = 1;
				protect.bw40 = 1;
			}

			if (mode & SET_PROTECT(HT20_PROTECT))
				protect.bw40 = 1;

			if (mode & SET_PROTECT(NON_HT_MIXMODE_PROTECT)) {
				protect.mix_mode = 1;
				protect.gf = 1;
				protect.bw40 = 1;
			}

			if (mode & SET_PROTECT(GREEN_FIELD_PROTECT))
				protect.gf = 1;

			/* if (mode & SET_PROTECT(RDG)) { */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
				protect.long_nav = 1;

			if (mode & SET_PROTECT(LONG_NAV_PROTECT))
				protect.long_nav = 1;

			if (mode & SET_PROTECT(RIFS_PROTECT)) {
				protect.long_nav = 1;
				protect.rifs = 1;
			}

			if (mode & SET_PROTECT(FORCE_RTS_PROTECT)) {
				arch_ops->archUpdateRtsThld(pAd, wdev, 0, 1, wlan_operate_get_rts_retry_limit(wdev));
				goto end;
			}
		}

		if (mode & SET_PROTECT(_NOT_DEFINE_HT_PROTECT)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("[ERROR] NOT Defined HT Protection!\n"));
		}

		wdev_idx++;
	} while (wdev_idx < WDEV_NUM_MAX);

	arch_ops->archUpdateProtect(pAd, &protect);
#ifdef DBDC_MODE

	if (pAd->CommonCfg.dbdc_mode == TRUE)
		arch_ops->archUpdateProtect(pAd, &protect_5g);

#endif /* DBDC_MODE */
end:
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlSetClientMACEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
	AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr, pInfo->IsBMC, pInfo->IsReset);
	return NDIS_STATUS_SUCCESS;
}


#ifdef TXBF_SUPPORT
static NTSTATUS HwCtrlSetClientBfCap(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMAC_TABLE_ENTRY pMacEntry;

	pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
	AsicUpdateClientBfCap(pAd, pMacEntry);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetBfRepeater(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMAC_TABLE_ENTRY pMacEntry;

	pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
#ifdef MAC_REPEATER_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	AsicTxBfReptClonedStaToNormalSta(pAd, pMacEntry->wcid, pMacEntry->MatchReptCliIdx);
#endif /* CONFIG_AP_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAdjBfSounding(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_BF_ADJ prMtStaBfAdj = NULL;
	UCHAR ucConnState;
	struct wifi_dev *wdev = NULL;

	prMtStaBfAdj = (PMT_STA_BF_ADJ)CMDQelmt->buffer;

	if (prMtStaBfAdj) {
		ucConnState = prMtStaBfAdj->ConnectionState;
		wdev = prMtStaBfAdj->wdev;
	}

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlTxBfTxApply(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PUCHAR pTxBfApply;

	pTxBfApply = (PUCHAR)CMDQelmt->buffer;
#ifdef BACKGROUND_SCAN_SUPPORT
	BfSwitch(pAd, *pTxBfApply);
#endif
	return NDIS_STATUS_SUCCESS;
}

#endif /* TXBF_SUPPORT */


static NTSTATUS HwCtrlDelAsicWcid(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;

	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (!VALID_WCID(SetAsicWcid.WCID) && (SetAsicWcid.WCID != WCID_ALL))
		return NDIS_STATUS_FAILURE;

	AsicDelWcidTab(pAd, SetAsicWcid.WCID);
	return NDIS_STATUS_SUCCESS;
}


#ifdef HTC_DECRYPT_IOT
static NTSTATUS HwCtrlSetAsicWcidAAD_OM(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;

	SetAsicAAD_OM = *((PRT_SET_ASIC_AAD_OM)(CMDQelmt->buffer));
	AsicSetWcidAAD_OM(pAd, SetAsicAAD_OM.WCID, SetAsicAAD_OM.Value);
	return NDIS_STATUS_SUCCESS;
}
#endif /* HTC_DECRYPT_IOT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
static NTSTATUS HwCtrlUpdate4Addr_HdrTrans(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_ASIC_4ADDR_HDR_TRANS Update_4Addr_Hdr_Trans;
	Update_4Addr_Hdr_Trans = *((PRT_ASIC_4ADDR_HDR_TRANS)(CMDQelmt->buffer));

	AsicSetWcid4Addr_HdrTrans(pAd, Update_4Addr_Hdr_Trans.Wcid, Update_4Addr_Hdr_Trans.Enable);

	return NDIS_STATUS_SUCCESS;
}
#endif
static void update_txop_level(UINT16 *dst, UINT16 *src,
							  UINT32 bitmap, UINT32 len)
{
	UINT32 prio;

	for (prio = 0; prio < len; prio++) {
		if (bitmap & (1 << prio)) {
			if (*(dst + prio) < *(src + prio))
				*(dst + prio) = *(src + prio);
		}
	}
}


static void tx_burst_arbiter(struct _RTMP_ADAPTER *pAd,
							 struct wifi_dev *curr_wdev,
							 UCHAR bss_idx)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	UINT32 idx = 0;
	UINT32 _prio_bitmap = 0;
	UINT16 txop_level = TXOP_0;
	UINT16 _txop_level[MAX_PRIO_NUM] = {0};
	UINT8 prio;
	UINT8 curr_prio = PRIO_DEFAULT;
	EDCA_PARM *edca_param = NULL;
	UCHAR wmm_idx = 0;

	edca_param = HcGetEdca(pAd, curr_wdev);

	if (edca_param == NULL)
		return;

	wmm_idx = edca_param->WmmSet;

	/* judge the final prio bitmap for specific BSS */
	do {
		if (wdev[idx] == NULL)
			break;

		if (wdev[idx]->bss_info_argument.ucBssIndex == bss_idx) {
			_prio_bitmap |= wdev[idx]->prio_bitmap;
			update_txop_level(_txop_level, wdev[idx]->txop_level,
							  _prio_bitmap, MAX_PRIO_NUM);
		}

		idx++;
	} while (idx < WDEV_NUM_MAX);

	/* update specific BSS's prio bitmap & txop_level array */
	curr_wdev->bss_info_argument.prio_bitmap = _prio_bitmap;
	memcpy(curr_wdev->bss_info_argument.txop_level, _txop_level,
		   (sizeof(UINT16) * MAX_PRIO_NUM));

	/* find the highest prio module */
	for (prio = 0; prio < MAX_PRIO_NUM; prio++) {
		if (_prio_bitmap & (1 << prio))
			curr_prio = prio;
	}

	txop_level = curr_wdev->bss_info_argument.txop_level[curr_prio];
	AsicSetWmmParam(pAd, wmm_idx, WMM_AC_BE, WMM_PARAM_TXOP, txop_level);
}

static void set_tx_burst(struct _RTMP_ADAPTER *pAd, struct _tx_burst_cfg *txop_cfg)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = NULL;
	UCHAR bss_idx = 0;

	if (txop_cfg->enable) {
		txop_cfg->wdev->prio_bitmap |= (1 << txop_cfg->prio);
		txop_cfg->wdev->txop_level[txop_cfg->prio] = txop_cfg->txop_level;
	} else
		txop_cfg->wdev->prio_bitmap &= ~(1 << txop_cfg->prio);

	bss_info = &txop_cfg->wdev->bss_info_argument;
	bss_idx = bss_info->ucBssIndex;
	tx_burst_arbiter(pAd, txop_cfg->wdev, bss_idx);
}

void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;

	if (wdev == NULL)
		return;

	txop_cfg.wdev = wdev;
	txop_cfg.prio = prio;
	txop_cfg.ac_type = ac_type;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("<caller: %pS>\n -%s: prio=%x, level=%x, enable=%x\n",
			  __builtin_return_address(0), __func__,
			  prio, level, enable));
	set_tx_burst(pAd, &txop_cfg);
}


static NTSTATUS HwCtrlSetTxBurst(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _tx_burst_cfg *txop_cfg = (struct _tx_burst_cfg *)CMDQelmt->buffer;

	if (txop_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	set_tx_burst(pAd, txop_cfg);
	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlAPAdjustEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME\n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x005400ca);
	}
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlAPRecoverEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME\n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x002400ca);
	}
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


static NTSTATUS HwCtrlUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d)\n", __func__, __LINE__));
	NICUpdateRawCounters(pAd);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAddRemoveKeyTab(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	ASIC_SEC_INFO *pInfo;

	pInfo = (PASIC_SEC_INFO) CMDQelmt->buffer;
	AsicAddRemoveKeyTab(pAd, pInfo);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MAC_REPEATER_SUPPORT
static NTSTATUS HwCtrlAddReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PADD_REPT_ENTRY_STRUC pInfo;
	struct wifi_dev *wdev = NULL;
	UCHAR *pAddr = NULL;

	pInfo = (PADD_REPT_ENTRY_STRUC)CMDQelmt->buffer;
	wdev = pInfo->wdev;
	pAddr = pInfo->arAddr;
	RTMPInsertRepeaterEntry(pAd, wdev, pAddr);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRemoveReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PREMOVE_REPT_ENTRY_STRUC pInfo;
	UCHAR func_tb_idx;
	UCHAR CliIdx;

	pInfo = (PREMOVE_REPT_ENTRY_STRUC)CMDQelmt->buffer;
	func_tb_idx = pInfo->func_tb_idx;
	CliIdx = pInfo->CliIdx;
	RTMPRemoveRepeaterEntry(pAd, func_tb_idx, CliIdx);
	return NDIS_STATUS_SUCCESS;
}
#endif /*MAC_REPEATER_SUPPORT*/

#ifdef MT_MAC
#ifdef BCN_OFFLOAD_SUPPORT
static NTSTATUS HwCtrlSetBcnOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_BCN_OFFLOAD pSetBcnOffload = (PMT_SET_BCN_OFFLOAD)CMDQelmt->buffer;
	struct wifi_dev *wdev = pAd->wdev_list[pSetBcnOffload->WdevIdx];
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	CMD_BCN_OFFLOAD_T *bcn_offload = NULL;
#else
	CMD_BCN_OFFLOAD_T bcn_offload;
#endif
	BCN_BUF_STRUC *bcn_buf = NULL;
#ifdef CONFIG_AP_SUPPORT
	TIM_BUF_STRUC *tim_buf = NULL;
#endif
	UCHAR *buf;
	PNDIS_PACKET *pkt = NULL;

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	os_alloc_mem(NULL, (PUCHAR *)&bcn_offload, sizeof(*bcn_offload));
	if (!bcn_offload) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("can not allocate bcn_offload\n"));
			return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(bcn_offload, sizeof(*bcn_offload));
#else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#endif

	if ((pSetBcnOffload->OffloadPktType == PKT_BCN)
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	|| (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
#endif
	) {
		bcn_buf = &wdev->bcn_buf;

		if (!bcn_buf) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): bcn_buf is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
			os_free_mem(bcn_offload);
#endif
			return NDIS_STATUS_FAILURE;
		}

		pkt = bcn_buf->BeaconPkt;
	}

#ifdef CONFIG_AP_SUPPORT
	else { /* tim pkt case in AP mode. */
		if (pAd->OpMode == OPMODE_AP)
			tim_buf = &wdev->bcn_buf.tim_buf;

		if (!tim_buf) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): tim_buf is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
			os_free_mem(bcn_offload);
#endif
			return NDIS_STATUS_FAILURE;
		}

		pkt = tim_buf->TimPkt;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	bcn_offload->ucEnable = pSetBcnOffload->Enable;
	bcn_offload->ucWlanIdx = 0;/* hardcode at present */
	bcn_offload->ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload->ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload->u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload->ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload->u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload->u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload->ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload->acPktContent, buf, pSetBcnOffload->WholeLength);
#else
	bcn_offload.ucEnable = pSetBcnOffload->Enable;
	bcn_offload.ucWlanIdx = 0;/* hardcode at present */
	bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload.ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
#endif
	MtCmdBcnOffloadSet(pAd, bcn_offload);

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	os_free_mem(bcn_offload);
#endif

	return NDIS_STATUS_SUCCESS;
}
#endif /*BCN_OFFLOAD_SUPPORT*/

static NTSTATUS HwCtrlSetTREntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_SET_TR_ENTRY pInfo;
	MAC_TABLE_ENTRY *pEntry;

	pInfo = (PRT_SET_TR_ENTRY)CMDQelmt->buffer;
	pEntry = (MAC_TABLE_ENTRY *)pInfo->pEntry;
	TRTableInsertEntry(pAd, pInfo->WCID, pEntry);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateBssInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	BSS_INFO_ARGUMENT_T *pBssInfoArgs = (BSS_INFO_ARGUMENT_T *)CMDQelmt->buffer;
	UINT32 ret;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __func__));
	ret = AsicBssInfoUpdate(pAd, *pBssInfoArgs);
	return ret;
}


static NTSTATUS HwCtrlSetBaRec(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_BA_CTRL_T *pSetBaRec = (MT_BA_CTRL_T *)CMDQelmt->buffer;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __func__));
	AsicUpdateBASession(pAd, pSetBaRec->Wcid, pSetBaRec->Tid, pSetBaRec->Sn, pSetBaRec->BaWinSize, pSetBaRec->isAdd,
						pSetBaRec->BaSessionType);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlHandleUpdateBeacon(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_UPDATE_BEACON *prMtUpdateBeacon = (MT_UPDATE_BEACON *)CMDQelmt->buffer;
	struct wifi_dev *wdev = prMtUpdateBeacon->wdev;
	UCHAR UpdateReason = prMtUpdateBeacon->UpdateReason;
	UCHAR i;
	BOOLEAN UpdateAfterTim = FALSE;
	BCN_BUF_STRUC *pbcn_buf = NULL;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Update reason: %d\n", __func__, UpdateReason));

	switch (UpdateReason) {
	case BCN_UPDATE_INIT:
	case BCN_UPDATE_IF_STATE_CHG:
	case BCN_UPDATE_IE_CHG:
	case BCN_UPDATE_AP_RENEW: {
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (wdev != NULL) {
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
				UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, TRUE);
#else
				UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_BCN, TRUE);
#endif
			} else {
				/* Update/Renew all BSS if wdev = NULL */
				for (i = 0; i < WDEV_NUM_MAX; i++) {
					if (pAd->wdev_list[i] != NULL) {
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
					UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V2_BCN, TRUE);
#else
					UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_BCN, TRUE);
#endif
					}
				}
			}
		}
	}
	break;

	case BCN_UPDATE_ENABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;

			if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
				if (pbcn_buf->BcnUpdateMethod == BCN_GEN_BY_FW) {
					wdev->bcn_buf.bBcnSntReq = TRUE;
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, TRUE);
#else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_BCN, TRUE);
#endif
				} else
					AsicEnableBeacon(pAd, wdev);
			}
		}
	}
	break;

	case BCN_UPDATE_DISABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;

			if (WDEV_WITH_BCN_ABILITY(wdev)) {
				if (pbcn_buf->BcnUpdateMethod == BCN_GEN_BY_FW) {
					wdev->bcn_buf.bBcnSntReq = FALSE;
					/* No need to make beacon */
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, FALSE);
#else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_BCN, FALSE);
#endif
				} else
					AsicDisableBeacon(pAd, wdev);
			}
		}
	}
	break;

	case BCN_UPDATE_PRETBTT: {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef AP_QLOAD_SUPPORT
			ULONG UpTime;
			/* update channel utilization */
			NdisGetSystemUpTime(&UpTime);
			QBSS_LoadUpdate(pAd, UpTime);
#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
			RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
			UpdateAfterTim = TRUE;
			updateBeaconRoutineCase(pAd, UpdateAfterTim);
		}

#endif /* CONFIG_AP_SUPPORT */
	}
	break;

	case BCN_UPDATE_TIM: {
#ifdef CONFIG_AP_SUPPORT
		APMakeAllTimFrame(pAd);
#endif
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_ALL,
				 DBG_SUBCAT_ALL,
				 DBG_LVL_ERROR,
				 ("%s(): Wrong Update reason: %d\n",
				  __func__, UpdateReason));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


#ifdef ERR_RECOVERY

static INT ErrRecoveryMcuIntEvent(RTMP_ADAPTER *pAd, UINT32 status)
{
	UINT32 u4McuInitEvent = MT_MCU_INT_EVENT;
	UINT32 IntStatus = 0;

	IntStatus |= status;


	RTMP_IO_WRITE32(pAd, u4McuInitEvent, IntStatus); /* write 1 to clear */

#if defined(MT7622)
	if (IS_MT7622(pAd)) {
		mt7622_trigger_intr_to_mcu(TRUE);
		mt7622_trigger_intr_to_mcu(FALSE);
	}
#endif

	return TRUE;
}

static INT ErrRecoveryStopPdmaAccess(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_STOP_PDMA0;
	return TRUE;
}

static INT ErrRecoveryStopPdmaAccessDone(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_STOP_IDLE_DONE;
	return TRUE;
}

static INT ErrRecoveryReinitPdma(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_RESET_PDMA0;
	return TRUE;
}

static INT ErrRecoveryWaitN9Normal(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_WAIT_N9_NORMAL;
	return TRUE;
}

static INT ErrRecoveryEventReentry(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_EVENT_REENTRY;
	return TRUE;
}

static INT ErrRecoveryDone(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovState = ERR_RECOV_STOP_IDLE;
	return TRUE;
}

static UINT32 ErrRecoveryTimeDiff(UINT32 time1, UINT32 time2)
{
	UINT32 timeDiff = 0;

	if (time1 > time2)
		timeDiff = (0xFFFFFFFF - time1 + 1) + time2;
	else
		timeDiff = time2 - time1;

	return timeDiff;
}

void SerTimeLogDump(RTMP_ADAPTER *pAd)
{
	UINT32 idx = 0;
	UINT32 *pSerTimes = NULL;

	if (pAd == NULL)
		return;

	pSerTimes = &pAd->HwCtrl.ser_times[0];

	for (idx = SER_TIME_ID_T0; idx < SER_TIME_ID_END; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s,::E  R  , Time[%d](us)=%u\n", __func__, idx,
				  pSerTimes[idx]));
	}

	for (idx = SER_TIME_ID_T0; idx < (SER_TIME_ID_END - 1); idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s,::E  R  , T%d - T%d(us)=%u\n", __func__,
				  idx + 1, idx, ErrRecoveryTimeDiff(pSerTimes[idx],
						  pSerTimes[idx + 1])));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s,::E  R  , Total Time(us)=%u\n", __func__,
			  ErrRecoveryTimeDiff(pSerTimes[SER_TIME_ID_T0],
								  pSerTimes[SER_TIME_ID_T7])));
}


VOID ser_sys_reset(RTMP_STRING *arg)
{
#ifdef SDK_TIMER_WDG
	/*kernel_restart(NULL);*/
	panic(arg); /* trigger SDK WATCHDOG TIMER */
#endif /* SDK_TIMER_WDG */
}


static void ErrRecoveryEndDriverRestore(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;
	struct tm_ops *tm_ops = pAd->tm_hif_ops;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#if defined(RTMP_MAC_PCI) || defined(RTMP_MAC_USB)
	tm_ops->schedule_task(pAd, TR_DONE_TASK);
#endif /* defined(RTMP_MAC_PCI) || defined(RTMP_MAC_USB) */
}

NTSTATUS HwRecoveryFromError(RTMP_ADAPTER *pAd)
{
	UINT32 Status;
	UINT32 Stat;
	ERR_RECOVERY_CTRL_T *pErrRecoveryCtrl;
	UINT32 Highpart, Lowpart;
	UINT32 *pSerTimes = NULL;

	if (!pAd)
		return NDIS_STATUS_INVALID_DATA;

#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Ser():The driver is in ATE mode now\n"));
		return NDIS_STATUS_SUCCESS;
	}

#endif /* CONFIG_ATE */
	pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
	Status = pAd->HwCtrl.ser_status;
	Stat = ErrRecoveryCurStat(pErrRecoveryCtrl);
	pSerTimes = &pAd->HwCtrl.ser_times[0];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ser                       ,::E  R  , stat=0x%08X\n", Stat));
	/*wlan hook for ser*/
	WLAN_HOOK_CALL(WLAN_HOOK_SER, pAd, pErrRecoveryCtrl);

	switch (Stat) {
	case ERR_RECOV_STOP_IDLE:
	case ERR_RECOV_EVENT_REENTRY:
		if ((Status & ERROR_DETECT_STOP_PDMA) == ERROR_DETECT_STOP_PDMA) {
			os_zero_mem(pSerTimes,
						(sizeof(pSerTimes[SER_TIME_ID_T0]) * SER_TIME_ID_END));
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T0] = Lowpart;
			/* Stop access PDMA. */
			ErrRecoveryStopPdmaAccessDone(pErrRecoveryCtrl);
			/* send PDMA0 stop to N9 through interrupt. */
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_STOP_DONE);
			/* all mmio need to be stop till hw reset done. */
			pAd->bPCIclkOff = TRUE;
			RtmpusecDelay(100 * 1000); /* delay for 100 ms to wait reset done. */
			pAd->bPCIclkOff = FALSE;
			/*re-call for change status to stop dma0*/
			HwRecoveryFromError(pAd);
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T1] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STOP_PDMA0:
		if ((Status & ERROR_DETECT_RESET_DONE) == ERROR_DETECT_RESET_DONE) {
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T2] = Lowpart;
			tm_exit(pAd);
			qm_exit(pAd);
#ifdef RESOURCE_PRE_ALLOC
			RTMPResetTxRxRingMemory(pAd);
#endif
			cut_through_deinit((PKT_TOKEN_CB **)&pAd->PktTokenCb);

			WfHifInit(pAd);
			qm_init(pAd);
			tm_init(pAd);
			RT28XXDMAEnable(pAd);
			HIF_IO_WRITE32(pAd, MT_WPDMA_MEM_RNG_ERR, 0);
			ErrRecoveryReinitPdma(pErrRecoveryCtrl);
			/* send PDMA0 reinit done to N9 through interrupt. */
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_INIT_DONE);
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T3] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_RESET_PDMA0:
		if ((Status & ERROR_DETECT_RECOVERY_DONE)
			== ERROR_DETECT_RECOVERY_DONE) {
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T4] = Lowpart;
			ErrRecoveryWaitN9Normal(pErrRecoveryCtrl);
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_RECOVERY_DONE);
			pSerTimes[SER_TIME_ID_T5] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("!!! SER CurStat=%u Event=%x!!!\n",
					  ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_WAIT_N9_NORMAL:
		if ((Status & ERROR_DETECT_N9_NORMAL_STATE)
			== ERROR_DETECT_N9_NORMAL_STATE) {
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;
			ErrRecoveryDone(pErrRecoveryCtrl);
			/* update Beacon frame if operating in AP mode. */
			UpdateBeaconHandler(
				pAd,
				NULL,
				BCN_UPDATE_AP_RENEW);
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;
			/*print out ser log timing*/
			SerTimeLogDump(pAd);
			ErrRecoveryEndDriverRestore(pAd);
		} else if ((Status & ERROR_DETECT_STOP_PDMA)
				   == ERROR_DETECT_STOP_PDMA) {
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("!!! ERROR SER re-entry  CurStat=%u Event=%x!!!\n",
					  ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;
			/*print out ser log timing*/
			SerTimeLogDump(pAd);
			ErrRecoveryEventReentry(pErrRecoveryCtrl);
			HwRecoveryFromError(pAd);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STOP_IDLE_DONE:
		ErrRecoveryStopPdmaAccess(pErrRecoveryCtrl);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* ERR_RECOVERY */

#endif

/*STA part*/

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
static NTSTATUS hw_ctrl_host_resume_done_ack(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->HostResumeDoneAck != NULL)
		ops->HostResumeDoneAck(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */



static NTSTATUS HwCtrlNICUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	NICUpdateRawCounters(pAd);
	return NDIS_STATUS_SUCCESS;
}

/*Pheripheral Handler*/
static NTSTATUS HwCtrlCheckGPIO(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

#ifdef LED_CONTROL_SUPPORT
static NTSTATUS HwCtrlSetLEDStatus(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR LEDStatus = *((PUCHAR)(CMDQelmt->buffer));

	RTMPSetLEDStatus(pAd, LEDStatus);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: CMDTHREAD_SET_LED_STATUS (LEDStatus = %d)\n",
			 __func__, LEDStatus));
	return NDIS_STATUS_SUCCESS;
}
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS HwCtrlLEDWPSMode10(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT WPSLedMode10 = *((PUINT)(CMDQelmt->buffer));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", WPSLedMode10));

	switch (WPSLedMode10) {
	case LINK_STATUS_WPS_MODE10_TURN_ON:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_ON);
		break;

	case LINK_STATUS_WPS_MODE10_FLASH:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_FLASH);
		break;

	case LINK_STATUS_WPS_MODE10_TURN_OFF:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_OFF);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10:: No this status %d!!!\n", WPSLedMode10));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRR(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_VOW_STA_GROUP *pVoW  =  (MT_VOW_STA_GROUP *)(CMDQelmt->buffer);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: group %d, staid %d\n", __func__, pVoW->GroupIdx,
			 pVoW->StaIdx));
	vow_set_client(pAd, pVoW->GroupIdx, pVoW->StaIdx);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetStaDWRRQuantum(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	INT32 ret;
	MT_VOW_STA_QUANTUM *pVoW  =  (MT_VOW_STA_QUANTUM *)(CMDQelmt->buffer);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\x1b[31m%s: restore %d, quantum %d\x1b[m\n", __func__, pVoW->restore, pVoW->quantum));

	if (pVoW->restore) {
		if (vow_watf_is_enabled(pAd)) {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
		} else {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
		}
	} else {
		UINT8 ac;
		/* 4 ac with the same quantum */
		for (ac = 0; ac < WMM_NUM_OF_AC; ac++)
			pAd->vow_cfg.vow_sta_dwrr_quantum[ac] = pVoW->quantum;
	}

	ret = vow_set_sta(pAd, 0x0, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\x1b[31m%s: ret %d\x1b[m\n", __func__, ret));

	return NDIS_STATUS_SUCCESS;
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

#ifdef THERMAL_PROTECT_SUPPORT
static NTSTATUS HwCtrlThermalProtRadioOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:\n", __func__));
	/* Set Radio off Process*/
	Set_RadioOn_Proc(pAd, "0");
	return NDIS_STATUS_SUCCESS;
}
#endif /* THERMAL_PROTECT_SUPPORT */

static NTSTATUS HwCtrlUpdateRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RssiUpdate(pAd);
	return NDIS_STATUS_SUCCESS;
}


#ifdef ETSI_RX_BLOCKER_SUPPORT
static NTSTATUS HwCtrlCheckRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	/* Check RSSI on evey 100 ms */
	CheckRssi(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

static NTSTATUS HwCtrlGetTemperature(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT32 temperature = 0;
	/*ActionIdx 0 means get temperature*/
	MtCmdGetThermalSensorResult(pAd, 0, &temperature);
	os_move_mem(CMDQelmt->RspBuffer, &temperature, CMDQelmt->RspBufferLen);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlGetTxStatistic(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	TX_STAT_STRUC *pTxStat = (PTX_STAT_STRUC)CMDQelmt->buffer;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	UCHAR dbdc_idx = 0;

	if (!VALID_UCAST_ENTRY_WCID(pAd, pTxStat->Wcid))
		return NDIS_STATUS_SUCCESS;

	pEntry = &pAd->MacTab.Content[pTxStat->Wcid];

	if (IS_ENTRY_NONE(pEntry))
		return NDIS_STATUS_SUCCESS;

	wdev = pEntry->wdev;

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	dbdc_idx = HcGetBandByWdev(wdev);

	tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];

	if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
		return NDIS_STATUS_SUCCESS;

	mt_cmd_get_sta_tx_statistic(pAd, pTxStat->Wcid, dbdc_idx, pTxStat->Field);
#endif
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRadioOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRADIO_ON_OFF_T pRadioOnOff = (PRADIO_ON_OFF_T)CMDQelmt->buffer;

	AsicRadioOnOffCtrl(pAd, pRadioOnOff->ucDbdcIdx, pRadioOnOff->ucRadio);
	return NDIS_STATUS_SUCCESS;
}


#ifdef GREENAP_SUPPORT
static NTSTATUS HwCtrlGreenAPOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PGREENAP_ON_OFF_T pGreenAP = (PGREENAP_ON_OFF_T)CMDQelmt->buffer;

	AsicGreenAPOnOffCtrl(pAd, pGreenAP->ucDbdcIdx, pGreenAP->ucGreenAPOn);
	return NDIS_STATUS_SUCCESS;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
static NTSTATUS hw_ctrl_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	P_PCIE_ASPM_DYM_CTRL_T ppcie_aspm_dym_ctrl = (P_PCIE_ASPM_DYM_CTRL_T)CMDQelmt->buffer;

	asic_pcie_aspm_dym_ctrl(
		pAd,
		ppcie_aspm_dym_ctrl->ucDbdcIdx,
		ppcie_aspm_dym_ctrl->fgL1Enable,
		ppcie_aspm_dym_ctrl->fgL0sEnable);

	return NDIS_STATUS_SUCCESS;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

static NTSTATUS HwCtrlSetSlotTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	SLOT_CFG *pSlotCfg = (SLOT_CFG *)CMDQelmt->buffer;

	AsicSetSlotTime(pAd, pSlotCfg->bUseShortSlotTime, pSlotCfg->Channel, pSlotCfg->wdev);
	return NDIS_STATUS_SUCCESS;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
/*
*
*/
static NTSTATUS HwCtrlSetPbc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct pbc_ctrl *pbc = (struct pbc_ctrl *)CMDQelmt->buffer;
	INT32 ret = 0;
	UINT8 bssid = (pbc->wdev) ?  (pbc->wdev->bss_info_argument.ucBssIndex) : PBC_BSS_IDX_FOR_ALL;
	UINT16 wcid = (pbc->entry) ? (pbc->entry->wcid) : PBC_WLAN_IDX_FOR_ALL;

	ret = MtCmdPktBudgetCtrl(pAd, bssid, wcid, pbc->type);
	return ret;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

static NTSTATUS HwCtrlSetEdca(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	INT32 ret = 0;
	PEDCA_PARM pedca_param = (PEDCA_PARM)CMDQelmt->buffer;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	ASSERT(arch_ops->archSetEdcaParm);
	arch_ops->archSetEdcaParm(pAd, pedca_param);
	return ret;
}

/*
*
*/
static NTSTATUS HwCtrlWifiSysOpen(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_open)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_open(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysClose(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_close)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_close(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerUpdate(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

#ifdef MBO_SUPPORT
static NTSTATUS HwCtrlBssTermination(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct wifi_dev *wdev = (struct wifi_dev *)CMDQelmt->buffer;
	UCHAR RfIC = 0;

	RfIC = wmode_2_rfic(wdev->PhyMode);

		if (!wdev->if_up_down_state) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>HwCtrlBssTermination (%s) but IF is done, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx));
			return TRUE;
		}

		if (IsHcRadioCurStatOffByChannel(pAd, wdev->channel)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>HwCtrlBssTermination (%s) equal to current state, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx));
			return TRUE;
		}

		MlmeRadioOff(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>HwCtrlBssTermination (OFF)\n"));

	return NDIS_STATUS_SUCCESS;
}
#endif

/*HWCMD_TYPE_RADIO*/
static HW_CMD_TABLE_T HwCmdRadioTable[] = {
	{HWCMD_ID_UPDATE_DAW_COUNTER, HwCtrlUpdateRawCounters, 0},
#ifdef MT_MAC
	{HWCMD_ID_SET_CLIENT_MAC_ENTRY, HwCtrlSetClientMACEntry, 0},
#ifdef TXBF_SUPPORT
	{HWCMD_ID_SET_APCLI_BF_CAP, HwCtrlSetClientBfCap, 0},
	{HWCMD_ID_SET_APCLI_BF_REPEATER, HwCtrlSetBfRepeater, 0},
	{HWCMD_ID_ADJUST_STA_BF_SOUNDING, HwCtrlAdjBfSounding, 0},
	{HWCMD_ID_TXBF_TX_APPLY_CTRL, HwCtrlTxBfTxApply, 0},
#endif
	{HWCMD_ID_SET_TR_ENTRY, HwCtrlSetTREntry, 0},
	{HWCMD_ID_SET_BA_REC, HwCtrlSetBaRec, 0},
	{HWCMD_ID_UPDATE_BSSINFO, HwCtrlUpdateBssInfo, 0},
	{HWCMD_ID_UPDATE_BEACON, HwCtrlHandleUpdateBeacon, 0},
	{HWCMD_ID_SET_TX_BURST, HwCtrlSetTxBurst, 0},
#endif /*MT_MAC*/
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_AP_ADJUST_EXP_ACK_TIME, HwCtrlAPAdjustEXPAckTime, 0},
	{HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,	HwCtrlAPRecoverEXPAckTime, 0},
#endif
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_SET_STA_DWRR, HwCtrlSetStaDWRR, 0},
	{HWCMD_ID_SET_STA_DWRR_QUANTUM, HwCtrlSetStaDWRRQuantum, 0},
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
	{HWCMD_ID_UPDATE_RSSI, HwCtrlUpdateRssi, 0},
	{HWCMD_ID_GET_TEMPERATURE, HwCtrlGetTemperature, 0},
	{HWCMD_ID_SET_SLOTTIME, HwCtrlSetSlotTime, 0},
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{HWCMD_RX_CHECK_RSSI, HwCtrlCheckRssi, 0},
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
#ifdef BCN_OFFLOAD_SUPPORT
	{HWCMD_ID_SET_BCN_OFFLOAD, HwCtrlSetBcnOffload, 0},
#endif
#ifdef MAC_REPEATER_SUPPORT
	{HWCMD_ID_ADD_REPT_ENTRY, HwCtrlAddReptEntry, 0},
	{HWCMD_ID_REMOVE_REPT_ENTRY, HwCtrlRemoveReptEntry, 0},
#endif
#ifdef THERMAL_PROTECT_SUPPORT
	{HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, HwCtrlThermalProtRadioOff, 0},
#endif /* THERMAL_PROTECT_SUPPORT */
	{HWCMD_ID_RADIO_ON_OFF, HwCtrlRadioOnOff, 0},
#ifdef GREENAP_SUPPORT
	{HWCMD_ID_GREENAP_ON_OFF, HwCtrlGreenAPOnOff, 0},
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	{HWCMD_ID_BSS_TERMINATION, HwCtrlBssTermination, 0},
#endif /* MBO_SUPPORT */
#ifdef MBSS_AS_WDS_AP_SUPPORT
	{HWCMD_ID_UPDATE_4ADDR_HDR_TRANS, HwCtrlUpdate4Addr_HdrTrans, 0},
#endif
    {HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_SECURITY*/
static HW_CMD_TABLE_T HwCmdSecurityTable[] = {
	{HWCMD_ID_ADDREMOVE_ASIC_KEY, HwCtrlAddRemoveKeyTab, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PERIPHERAL*/
static HW_CMD_TABLE_T HwCmdPeripheralTable[] = {
	{HWCMD_ID_GPIO_CHECK, HwCtrlCheckGPIO, 0},
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	{HWCMD_ID_LED_WPS_MODE10, HwCtrlLEDWPSMode10, 0},
#endif
#endif
#ifdef LED_CONTROL_SUPPORT
	{HWCMD_ID_SET_LED_STATUS, HwCtrlSetLEDStatus, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_HT_CAP*/
static HW_CMD_TABLE_T HwCmdHtCapTable[] = {
	{HWCMD_ID_DEL_ASIC_WCID, HwCtrlDelAsicWcid, 0},
#ifdef HTC_DECRYPT_IOT
	{HWCMD_ID_SET_ASIC_AAD_OM, HwCtrlSetAsicWcidAAD_OM, 0},
#endif /* HTC_DECRYPT_IOT */
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PS*/
static HW_CMD_TABLE_T HwCmdPsTable[] = {
#ifdef MT_MAC
#endif
	{HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, HwCtrlNICUpdateRawCounters, 0},
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	{HWCMD_ID_HOST_RESUME_DONE_ACK, hw_ctrl_host_resume_done_ack, 0},
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{HWCMD_ID_PCIE_ASPM_DYM_CTRL, hw_ctrl_pcie_aspm_dym_ctrl, 0},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	{HWCMD_ID_END, NULL, 0}
};


/*HWCMD_TYPE_WIFISYS*/
static HW_CMD_TABLE_T HwCmdWifiSysTable[] = {
	{HWCMD_ID_WIFISYS_LINKDOWN, HwCtrlWifiSysLinkDown, 0},
	{HWCMD_ID_WIFISYS_LINKUP, HwCtrlWifiSysLinkUp, 0},
	{HWCMD_ID_WIFISYS_OPEN, HwCtrlWifiSysOpen, 0},
	{HWCMD_ID_WIFISYS_CLOSE, HwCtrlWifiSysClose, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKDOWN, HwCtrlWifiSysPeerLinkDown, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKUP, HwCtrlWifiSysPeerLinkUp, 0},
	{HWCMD_ID_WIFISYS_PEER_UPDATE, HwCtrlWifiSysPeerUpdate, 0},
	{HWCMD_ID_GET_TX_STATISTIC, HwCtrlGetTxStatistic, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_WMM*/
static HW_CMD_TABLE_T HwCmdWmmTable[] = {
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{HWCMD_ID_PBC_CTRL, HwCtrlSetPbc, 0},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
	{HWCMD_ID_SET_EDCA, HwCtrlSetEdca, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PROTECT*/
static HW_CMD_TABLE_T HwCmdProtectTable[] = {
	{HWCMD_ID_RTS_THLD, HwCtrlUpdateRtsThreshold, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*Order can't be changed, follow HW_CMD_TYPE order definition*/
HW_CMD_TABLE_T *HwCmdTable[] = {
	HwCmdRadioTable,
	HwCmdSecurityTable,
	HwCmdPeripheralTable,
	HwCmdHtCapTable,
	HwCmdPsTable,
	HwCmdWifiSysTable,
	HwCmdWmmTable,
	HwCmdProtectTable,
	NULL
};



HW_FLAG_TABLE_T HwFlagTable[] = {
	{HWFLAG_ID_UPDATE_PROTECT, HwCtrlUpdateProtect, 0},
	{HWFLAG_ID_END, NULL, 0}
};

