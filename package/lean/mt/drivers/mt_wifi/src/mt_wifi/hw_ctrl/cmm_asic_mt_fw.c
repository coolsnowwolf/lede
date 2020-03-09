/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic_mt.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#include "hdev/hdev.h"

/* DEV Info */
INT32 MtAsicSetDevMacByFw(
	RTMP_ADAPTER * pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	return CmdExtDevInfoUpdate(pAd,
							   OwnMacIdx,
							   OwnMacAddr,
							   BandIdx,
							   Active,
							   EnableFeature);
}


/* BSS Info */
INT32 MtAsicSetBssidByFw(
	RTMP_ADAPTER * pAd,
	BSS_INFO_ARGUMENT_T bss_info_argument)
{
	return CmdExtBssInfoUpdate(pAd, bss_info_argument);
}

/* STARec Info */
INT32 MtAsicSetStaRecByFw(
	RTMP_ADAPTER * pAd,
	STA_REC_CFG_T StaCfg)
{
	return CmdExtStaRecUpdate(pAd, StaCfg);
}

INT32 MtAsicUpdateStaRecBaByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
	return CmdExtStaRecBaUpdate(pAd, StaRecBaCfg);
}


VOID MtSetTmrCRByFw(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	CmdExtSetTmrCR(pAd, enable, BandIdx);
}


VOID MtAsicAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout)
{
	CmdAutoBATrigger(pAd, Enable, Timeout);
}


VOID MtAsicDelWcidTabByFw(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid_idx)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, --->\n", __func__));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, wcid_idx(%d)\n", __func__, wcid_idx));

	if (wcid_idx == WCID_ALL)
		CmdExtWtblUpdate(pAd, 0, RESET_ALL_WTBL, NULL, 0);
	else
		CmdExtWtblUpdate(pAd, wcid_idx, RESET_WTBL_AND_SET, NULL, 0);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, <---\n", __func__));
}


#ifdef HTC_DECRYPT_IOT
VOID MtAsicSetWcidAAD_OMByFw(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid_idx,
	IN UCHAR value)
{
	UINT32 mask = 0xfffffff7;
	UINT32 val;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, wcid_idx(%d), value(%d)\n", __func__, wcid_idx, value));

	if (value) {
		val = 0x8;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	} else {
		val = 0x0;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, <---\n", __func__));
}
#endif /* HTC_DECRYPT_IOT */



/* MT7615 */
/* need to ready first, Carter, wilsonl */
VOID MtAsicUpdateRxWCIDTableByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo)
{
	NDIS_STATUS					Status = NDIS_STATUS_SUCCESS;
	UCHAR						*pTlvBuffer = NULL;
	UCHAR						*pTempBuffer = NULL;
	UINT32						u4TotalTlvLen = 0;
	UCHAR						ucTotalTlvNumber = 0;
	/* Tag = 0, Generic */
	CMD_WTBL_GENERIC_T		rWtblGeneric = {0};
	/* Tage = 1, Rx */
	CMD_WTBL_RX_T				rWtblRx = {0};
#ifdef DOT11_N_SUPPORT
	/* Tag = 2, HT */
	CMD_WTBL_HT_T				rWtblHt = {0};
#ifdef DOT11_VHT_AC
	/* Tag = 3, VHT */
	CMD_WTBL_VHT_T			rWtblVht = {0};
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	/* Tag = 5, TxPs */
	CMD_WTBL_TX_PS_T			rWtblTxPs = {0};
#if defined(HDR_TRANS_TX_SUPPORT) || defined(HDR_TRANS_RX_SUPPORT)
	/* Tag = 6, Hdr Trans */
	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};
#endif /* HDR_TRANS_TX_SUPPORT */
	/* Tag = 7, Security Key */
	CMD_WTBL_SECURITY_KEY_T	rWtblSecurityKey = {0};
	/* Tag = 9, Rdg */
	CMD_WTBL_RDG_T			rWtblRdg = {0};
#ifdef TXBF_SUPPORT
	/* Tag = 12, BF */
	CMD_WTBL_BF_T           rWtblBf = {0};
#endif /* TXBF_SUPPORT */
	/* Tag = 13, SMPS */
	CMD_WTBL_SMPS_T			rWtblSmPs = {0};
	/* Tag = 16, SPE */
	CMD_WTBL_SPE_T          rWtblSpe = {0};
	/* Allocate TLV msg */
	Status = os_alloc_mem(pAd, (UCHAR **)&pTlvBuffer, MAX_BUF_SIZE_OF_WTBL_INFO);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): %d,%d,%d,%d,%d,%d,%d,%d,%d,(%x:%x:%x:%x:%x:%x),%d,%d,%d,%d,%d,%d)\n", __func__,
			 WtblInfo.Wcid,
			 WtblInfo.Aid,
			 WtblInfo.BssidIdx,
			 WtblInfo.MacAddrIdx,
			 WtblInfo.SmpsMode,
			 WtblInfo.MaxRAmpduFactor,
			 WtblInfo.MpduDensity,
			 WtblInfo.WcidType,
			 WtblInfo.aad_om,
			 PRINT_MAC(WtblInfo.Addr),
			 WtblInfo.CipherSuit,
			 WtblInfo.PfmuId,
			 WtblInfo.SupportHT,
			 WtblInfo.SupportVHT,
			 WtblInfo.SupportRDG,
			 WtblInfo.SupportQoS));

	if ((Status != NDIS_STATUS_SUCCESS) || (pTlvBuffer == NULL))
		goto error;

	pTempBuffer = pTlvBuffer;
	rWtblRx.ucRv   = WtblInfo.rv;
	rWtblRx.ucRca2 = WtblInfo.rca2;

	if (WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST) {
		/* prevent BMC ICV message dumped during GTK rekey */
		if (HcGetWcidLinkType(pAd, WtblInfo.Wcid) == WDEV_TYPE_APCLI)
			rWtblRx.ucRcid = 1;
	}

	/* Manipulate TLV msg */
	if (WtblInfo.WcidType == MT_WCID_TYPE_BMCAST) {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = 0x0e;
		/* Tag = 1 */
		rWtblRx.ucRv = 1;
		rWtblRx.ucRca1 = 1;
		/* if (pAd->OpMode == OPMODE_AP) */
		{
			rWtblRx.ucRca2 = 1;
		}
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WTBL_CIPHER_NONE;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		if (pAd->OpMode == OPMODE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		}

#endif
	} else {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = WtblInfo.MacAddrIdx;
		rWtblGeneric.ucQos = (WtblInfo.SupportQoS) ? 1 : 0;
		rWtblGeneric.u2PartialAID = WtblInfo.Aid;
		rWtblGeneric.ucAadOm = WtblInfo.aad_om;

		/* Tag = 1 */
		if ((WtblInfo.WcidType == MT_WCID_TYPE_APCLI) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_REPEATER) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_AP) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST))
			rWtblRx.ucRca1 = 1;

		rWtblRx.ucRv = 1;
		rWtblRx.ucRca2 = 1;
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WtblInfo.CipherSuit;
		rWtblSecurityKey.ucRkv = (WtblInfo.CipherSuit != WTBL_CIPHER_NONE) ? 1 : 0;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		switch (WtblInfo.WcidType) {
		case MT_WCID_TYPE_AP:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
			break;

		case MT_WCID_TYPE_CLI:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
			break;

		case MT_WCID_TYPE_APCLI:
		case MT_WCID_TYPE_REPEATER:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
			break;

		case MT_WCID_TYPE_WDS:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: Unknown entry type(%d) do not support header translation\n",
					  __func__, WtblInfo.WcidType));
			break;
		}

#endif /* HDR_TRANS_TX_SUPPORT */
#ifdef HDR_TRANS_RX_SUPPORT

		if (WtblInfo.DisRHTR)
			rWtblHdrTrans.ucDisRhtr = 1;
		else
			rWtblHdrTrans.ucDisRhtr = 0;

#endif /* HDR_TRANS_RX_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (WtblInfo.SupportHT) {
			/* Tag = 0 */
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucBafEn = 0;
			/* Tag = 2 */
			rWtblHt.ucHt = 1;
			rWtblHt.ucMm = WtblInfo.MpduDensity;
			rWtblHt.ucAf = WtblInfo.MaxRAmpduFactor;

			/* Tga = 9 */
			if (WtblInfo.SupportRDG) {
				rWtblRdg.ucR = 1;
				rWtblRdg.ucRdgBa = 1;
			}

			/* Tag = 13*/
			if (WtblInfo.SmpsMode == MMPS_DYNAMIC)
				rWtblSmPs.ucSmPs = 1;
			else
				rWtblSmPs.ucSmPs = 0;

#ifdef DOT11_VHT_AC

			/* Tag = 3 */
			if (WtblInfo.SupportVHT) {
				rWtblVht.ucVht = 1;

				if (WtblInfo.dyn_bw == BW_SIGNALING_DYNAMIC) {
					rWtblVht.ucDynBw = 1;
				}
			}

#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
	}

	/* Tag = 0 */
	os_move_mem(rWtblGeneric.aucPeerAddress, WtblInfo.Addr, MAC_ADDR_LEN);
	/* Tag = 5 */
	rWtblTxPs.ucTxPs = 0;
#ifdef TXBF_SUPPORT
	/* Tag = 0xc */
	rWtblBf.ucGid     = WtblInfo.gid;
	rWtblBf.ucPFMUIdx = WtblInfo.PfmuId;
	rWtblBf.ucTiBf    = WtblInfo.fgTiBf;
	rWtblBf.ucTeBf    = WtblInfo.fgTeBf;
	rWtblBf.ucTibfVht = WtblInfo.fgTibfVht;
	rWtblBf.ucTebfVht = WtblInfo.fgTebfVht;
#endif /* TXBF_SUPPORT */
	/* Tag = 0x10 */
	rWtblSpe.ucSpeIdx = WtblInfo.spe_idx;
	/* Append TLV msg */
	pTempBuffer = pTlvAppend(
					  pTlvBuffer,
					  (WTBL_GENERIC),
					  (sizeof(CMD_WTBL_GENERIC_T)),
					  &rWtblGeneric,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RX),
					  (sizeof(CMD_WTBL_RX_T)),
					  &rWtblRx,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_N_SUPPORT
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HT),
					  (sizeof(CMD_WTBL_HT_T)),
					  &rWtblHt,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RDG),
					  (sizeof(CMD_WTBL_RDG_T)),
					  &rWtblRdg,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SMPS),
					  (sizeof(CMD_WTBL_SMPS_T)),
					  &rWtblSmPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_VHT_AC
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_VHT),
					  (sizeof(CMD_WTBL_VHT_T)),
					  &rWtblVht,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_TX_PS),
					  (sizeof(CMD_WTBL_TX_PS_T)),
					  &rWtblTxPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#if defined(HDR_TRANS_RX_SUPPORT) || defined(HDR_TRANS_TX_SUPPORT)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HDR_TRANS),
					  (sizeof(CMD_WTBL_HDR_TRANS_T)),
					  &rWtblHdrTrans,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* HDR_TRANS_RX_SUPPORT || HDR_TRANS_TX_SUPPORT */
	if (WtblInfo.SkipClearPrevSecKey == FALSE)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SECURITY_KEY),
					  (sizeof(CMD_WTBL_SECURITY_KEY_T)),
					  &rWtblSecurityKey,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef TXBF_SUPPORT

	if (pAd->rStaRecBf.u2PfmuId != 0xFFFF) {
		pTempBuffer = pTlvAppend(
						  pTempBuffer,
						  (WTBL_BF),
						  (sizeof(CMD_WTBL_BF_T)),
						  &rWtblBf,
						  &u4TotalTlvLen,
						  &ucTotalTlvNumber);
	}

#endif /* TXBF_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SPE),
					  (sizeof(CMD_WTBL_SPE_T)),
					  &rWtblSpe,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	/* Send TLV msg*/
	if (WtblInfo.IsReset == TRUE) {
			if (WtblInfo.SkipClearPrevSecKey == TRUE)
				CmdExtWtblUpdate(pAd, (UINT8)WtblInfo.Wcid, SET_WTBL, pTlvBuffer, u4TotalTlvLen);
			else
				CmdExtWtblUpdate(pAd, (UINT8)WtblInfo.Wcid, RESET_WTBL_AND_SET, pTlvBuffer, u4TotalTlvLen);
	}
	else
		CmdExtWtblUpdate(pAd, (UINT8)WtblInfo.Wcid, SET_WTBL, pTlvBuffer, u4TotalTlvLen);

	/* Free TLV msg */
	if (pTlvBuffer)
		os_free_mem(pTlvBuffer);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Ret = %d)\n", __func__, Status));
}



VOID MtAsicUpdateBASessionByWtblTlv(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR *ba_range = cap->ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown tid(%d)\n", __func__, BaCtrl.Tid));
		return;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			CmdExtWtblUpdate(pAd, (UINT8)BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/*get ba win size from range */
			while (ba_range[idx] < BaCtrl.BaWinSize) {
				if (idx == 7)
					break;

				idx++;
			};

			if (ba_range[idx] > BaCtrl.BaWinSize)
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaSize = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaSize = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		CmdExtWtblUpdate(pAd, (UINT8)BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}
}


INT32 MtAsicUpdateBASessionByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_BA_CTRL_T BaCtrl)
{
	INT32				Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR *ba_range = cap->ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown tid(%d)\n", __func__, BaCtrl.Tid));
		return Status;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			rWtblBa.ucBandIdx = BaCtrl.band_idx;
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			Status = CmdExtWtblUpdate(pAd, (UINT8)BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}

		/* TODO: Hanmin 7615, need rWtblBa.ucBaEn=0 for delete? */
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/* Get ba win size from range */
			while (BaCtrl.BaWinSize > ba_range[idx]) {
				if (idx == (MT_DMAC_BA_AGG_RANGE - 1))
					break;

				idx++;
			};

			if ((idx > 0) && (ba_range[idx] > BaCtrl.BaWinSize))
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaSize = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaSize = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		Status = CmdExtWtblUpdate(pAd, (UINT8)BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}

	return Status;
}


UINT16 MtAsicGetTidSnByFw(
	IN PRTMP_ADAPTER pAd,
	UCHAR wcid,
	UCHAR tid)
{
	INT32 Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T rWtblBa = {0};
	UINT16 ssn = 0xffff;

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = tid;
	Status = CmdExtWtblUpdate(pAd, (UINT8)wcid,
							  QUERY_WTBL, &rWtblBa, sizeof(CMD_WTBL_BA_T));

	if (Status == NDIS_STATUS_SUCCESS)
		ssn = rWtblBa.u2Sn;

	return ssn;
}


VOID MtAsicAddRemoveKeyTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _ASIC_SEC_INFO *pInfo)
{
	CMD_WTBL_SECURITY_KEY_T rWtblSecurityKey = {0};

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:, wcid=%d, Operation=%d, Direction=%d\n",
			 __func__, pInfo->Wcid, pInfo->Operation, pInfo->Direction));
	rWtblSecurityKey.u2Tag = WTBL_SECURITY_KEY;
	rWtblSecurityKey.u2Length = sizeof(CMD_WTBL_SECURITY_KEY_T);
	fill_wtbl_key_info_struc(pInfo, &rWtblSecurityKey);

	/* Workaround code */
	if ((IS_CIPHER_CCMP128(pInfo->Cipher)) && (rWtblSecurityKey.ucKeyLen == 32)) {
		UINT16 u2LengthTemp, u2TagTemp;
		/* store u2Length and uTag for the second time Wtbl setting, */
		/* because CmdExtWtblUpdate will do endian change.*/
		u2LengthTemp = rWtblSecurityKey.u2Length;
		u2TagTemp = rWtblSecurityKey.u2Tag;
		rWtblSecurityKey.ucAlgorithmId = CIPHER_SUIT_CCMP_256;
		CmdExtWtblUpdate(pAd, (UINT8)pInfo->Wcid, SET_WTBL, (PUCHAR)&rWtblSecurityKey, sizeof(CMD_WTBL_SECURITY_KEY_T));
		rWtblSecurityKey.ucAlgorithmId = CIPHER_SUIT_CCMP_W_MIC;
		rWtblSecurityKey.u2Length = u2LengthTemp;
		rWtblSecurityKey.u2Tag = u2TagTemp;
	}

	CmdExtWtblUpdate(pAd, (UINT8)pInfo->Wcid, SET_WTBL, (PUCHAR)&rWtblSecurityKey, sizeof(CMD_WTBL_SECURITY_KEY_T));
}

VOID MtAsicSetSMPSByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR Smps)
{
	CMD_WTBL_SMPS_T	CmdWtblSmPs = {0};

	CmdWtblSmPs.u2Tag = WTBL_SMPS;
	CmdWtblSmPs.u2Length = sizeof(CMD_WTBL_SMPS_T);
	CmdWtblSmPs.ucSmPs = Smps;
	CmdExtWtblUpdate(pAd, Wcid, SET_WTBL, (PUCHAR)&CmdWtblSmPs, sizeof(CMD_WTBL_SMPS_T));
}


VOID mt_wtbltlv_debug(RTMP_ADAPTER *pAd, UCHAR ucWcid, UCHAR ucCmdId, UCHAR ucAtion)
{
	/* tag 0 */
	if (ucCmdId == WTBL_GENERIC) {
		CMD_WTBL_GENERIC_T		rWtblGeneric = {0};

		rWtblGeneric.u2Tag = WTBL_GENERIC;
		rWtblGeneric.u2Length = sizeof(CMD_WTBL_GENERIC_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

			os_move_mem(rWtblGeneric.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			rWtblGeneric.ucMUARIndex = 0x0;
			rWtblGeneric.ucSkipTx = 0;
			rWtblGeneric.ucCfAck = 0;
			rWtblGeneric.ucQos = 0;
			rWtblGeneric.ucMesh = 0;
			rWtblGeneric.ucAdm = 0;
			rWtblGeneric.u2PartialAID = 0;
			rWtblGeneric.ucBafEn = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

			os_move_mem(rWtblGeneric.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			rWtblGeneric.ucMUARIndex = 0x0e;
			rWtblGeneric.ucSkipTx = 1;
			rWtblGeneric.ucCfAck = 1;
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucMesh = 1;
			rWtblGeneric.ucAdm = 1;
			rWtblGeneric.u2PartialAID = 32;
			rWtblGeneric.ucBafEn = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 1 */
	if (ucCmdId == WTBL_RX) {
		CMD_WTBL_RX_T				rWtblRx = {0};

		rWtblRx.u2Tag = WTBL_RX;
		rWtblRx.u2Length = sizeof(CMD_WTBL_RX_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblRx.ucRcid = 0;
			rWtblRx.ucRca1 = 0;
			rWtblRx.ucRca2 = 0;
			rWtblRx.ucRv = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRx, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblRx.ucRcid = 1;
			rWtblRx.ucRca1 = 1;
			rWtblRx.ucRca2 = 1;
			rWtblRx.ucRv = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRx, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblRx, sizeof(CMD_WTBL_RX_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 2 */
	if (ucCmdId == WTBL_HT) {
		CMD_WTBL_HT_T				rWtblHt = {0};

		rWtblHt.u2Tag = WTBL_HT;
		rWtblHt.u2Length = sizeof(CMD_WTBL_HT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblHt.ucHt = 0;
			rWtblHt.ucLdpc = 0;
			rWtblHt.ucAf = 0;
			rWtblHt.ucMm = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblHt, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblHt.ucHt = 1;
			rWtblHt.ucLdpc = 1;
			rWtblHt.ucAf = 1;
			rWtblHt.ucMm = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblHt, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblHt, sizeof(CMD_WTBL_HT_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 3 */
	if (ucCmdId == WTBL_VHT) {
		CMD_WTBL_VHT_T			rWtblVht = {0};

		rWtblVht.u2Tag = WTBL_VHT;
		rWtblVht.u2Length = sizeof(CMD_WTBL_VHT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblVht.ucLdpcVht = 0;
			rWtblVht.ucDynBw = 0;
			rWtblVht.ucVht = 0;
			rWtblVht.ucTxopPsCap = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblVht, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblVht.ucLdpcVht = 1;
			rWtblVht.ucDynBw = 1;
			rWtblVht.ucVht = 1;
			rWtblVht.ucTxopPsCap = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblVht, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblVht, sizeof(CMD_WTBL_VHT_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 4 */
	if (ucCmdId == WTBL_PEER_PS) {
		CMD_WTBL_PEER_PS_T			rWtblPeerPs = {0};

		rWtblPeerPs.u2Tag = WTBL_PEER_PS;
		rWtblPeerPs.u2Length = sizeof(CMD_WTBL_PEER_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblPeerPs.ucDuIPsm = 0;
			rWtblPeerPs.ucIPsm = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblPeerPs, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblPeerPs.ucDuIPsm = 1;
			rWtblPeerPs.ucIPsm = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblPeerPs, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblPeerPs, sizeof(CMD_WTBL_PEER_PS_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 5 */
	if (ucCmdId == WTBL_TX_PS) {
		CMD_WTBL_TX_PS_T			rWtblTxPs = {0};

		rWtblTxPs.u2Tag = WTBL_TX_PS;
		rWtblTxPs.u2Length = sizeof(CMD_WTBL_TX_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblTxPs.ucTxPs = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblTxPs, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblTxPs.ucTxPs = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblTxPs, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblTxPs, sizeof(CMD_WTBL_TX_PS_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 6 */
	if (ucCmdId == WTBL_HDR_TRANS) {
		CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};

		rWtblHdrTrans.u2Tag = WTBL_HDR_TRANS;
		rWtblHdrTrans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblHdrTrans.ucTd = 0;
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucDisRhtr = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblHdrTrans, sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblHdrTrans.ucTd = 1;
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucDisRhtr = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblHdrTrans, sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblHdrTrans, sizeof(CMD_WTBL_HDR_TRANS_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 7 security */
	/* tag 8 BA */

	/* tag 9 */
	if (ucCmdId == WTBL_RDG) {
		CMD_WTBL_RDG_T			rWtblRdg = {0};

		rWtblRdg.u2Tag = WTBL_RDG;
		rWtblRdg.u2Length = sizeof(CMD_WTBL_RDG_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblRdg.ucRdgBa = 0;
			rWtblRdg.ucR = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRdg, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblRdg.ucRdgBa = 1;
			rWtblRdg.ucR = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRdg, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblRdg, sizeof(CMD_WTBL_RDG_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 10 */
	if (ucCmdId == WTBL_PROTECTION) {
		CMD_WTBL_PROTECTION_T		rWtblProtection = {0};

		rWtblProtection.u2Tag = WTBL_PROTECTION;
		rWtblProtection.u2Length = sizeof(CMD_WTBL_PROTECTION_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblProtection.ucRts = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblProtection, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblProtection.ucRts = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblProtection, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblProtection, sizeof(CMD_WTBL_PROTECTION_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 11 */
	if (ucCmdId == WTBL_CLEAR) {
		CMD_WTBL_CLEAR_T		rWtblClear = {0};

		rWtblClear.u2Tag = WTBL_CLEAR;
		rWtblClear.u2Length = sizeof(CMD_WTBL_CLEAR_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblClear.ucClear = ((0 << 1) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblClear, sizeof(CMD_WTBL_CLEAR_T));
		} else if (ucAtion == 2) {
			/* query */
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 12 */
	if (ucCmdId == WTBL_BF) {
		CMD_WTBL_BF_T		rWtblBf = {0};

		rWtblBf.u2Tag = WTBL_BF;
		rWtblBf.u2Length = sizeof(CMD_WTBL_BF_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblBf.ucTiBf = 0;
			rWtblBf.ucTeBf = 0;
			rWtblBf.ucTibfVht = 0;
			rWtblBf.ucTebfVht = 0;
			rWtblBf.ucGid = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblBf, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblBf.ucTiBf = 1;
			rWtblBf.ucTeBf = 1;
			rWtblBf.ucTibfVht = 1;
			rWtblBf.ucTebfVht = 1;
			rWtblBf.ucGid = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblBf, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblBf, sizeof(CMD_WTBL_BF_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 13 */
	if (ucCmdId == WTBL_SMPS) {
		CMD_WTBL_SMPS_T		rWtblSmps = {0};

		rWtblSmps.u2Tag = WTBL_SMPS;
		rWtblSmps.u2Length = sizeof(CMD_WTBL_SMPS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblSmps.ucSmPs = 0;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblSmps, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblSmps.ucSmPs = 1;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblSmps, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblSmps, sizeof(CMD_WTBL_SMPS_T));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 14 */
	if (ucCmdId == WTBL_RAW_DATA_RW) {
		CMD_WTBL_RAW_DATA_RW_T		rWtblRawDataRw = {0};

		rWtblRawDataRw.u2Tag = WTBL_RAW_DATA_RW;
		rWtblRawDataRw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			rWtblRawDataRw.ucWtblIdx = 1;
			rWtblRawDataRw.ucWhichDW = 0;
			rWtblRawDataRw.u4DwMask = 0xffff00ff;
			rWtblRawDataRw.u4DwValue = 0x12340078;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRawDataRw, sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			rWtblRawDataRw.ucWtblIdx = 1;
			rWtblRawDataRw.ucWhichDW = 0;
			rWtblRawDataRw.u4DwMask = 0xffff00ff;
			rWtblRawDataRw.u4DwValue = 0x12345678;
			CmdExtWtblUpdate(pAd, ucWcid, SET_WTBL, &rWtblRawDataRw, sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 2) {
			/* query */
			rWtblRawDataRw.ucWtblIdx = 1;
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblRawDataRw, sizeof(CMD_WTBL_RAW_DATA_RW_T));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::rWtblRawDataRw.u4DwValue(%x)\n", __func__, rWtblRawDataRw.u4DwValue));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}

	/* tag 15 */
	if (ucCmdId == WTBL_DUMP) {
		CMD_WTBL_DUMP_T		rWtblDump = {0};

		rWtblDump.u2Tag = WTBL_DUMP;
		rWtblDump.u2Length = sizeof(CMD_WTBL_DUMP_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, ucWcid, QUERY_WTBL, &rWtblDump, sizeof(CMD_WTBL_DUMP_T));
			hex_dump("WTBL_DUMP", rWtblDump.aucWtblBuffer, WTBL_BUFFER_SIZE);
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Cmd Error\n", __func__));
	}
}


VOID MtAsicUpdateProtectByFw(
	struct _RTMP_ADAPTER *pAd,
	MT_PROTECT_CTRL_T *protect)
{
	struct _EXT_CMD_UPDATE_PROTECT_T fw_protect;

	fw_protect.ucProtectIdx = UPDATE_PROTECTION_CTRL;
	fw_protect.ucDbdcIdx = protect->band_idx;
	fw_protect.Data.rUpdateProtect.ucLongNav = protect->long_nav;
	fw_protect.Data.rUpdateProtect.ucMMProtect = protect->mix_mode;
	fw_protect.Data.rUpdateProtect.ucGFProtect = protect->gf;
	fw_protect.Data.rUpdateProtect.ucBW40Protect = protect->bw40;
	fw_protect.Data.rUpdateProtect.ucRifsProtect = protect->rifs;
	fw_protect.Data.rUpdateProtect.ucBW80Protect = protect->bw80;
	fw_protect.Data.rUpdateProtect.ucBW160Protect = protect->bw160;
	fw_protect.Data.rUpdateProtect.ucERProtectMask = protect->erp_mask;
	MtCmdUpdateProtect(pAd, &fw_protect);
}


VOID MtAsicUpdateRtsThldByFw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length)
{
	MT_RTS_THRESHOLD_T rts_thld = {0};

	rts_thld.band_idx = HcGetBandByWdev(wdev);
	rts_thld.pkt_num_thld = pkt_num;
	rts_thld.pkt_len_thld = length;
	if (MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3) && pAd->CommonCfg.dbdc_mode) {
		;/* DBDC does not support RTS setting */
	} else {
		struct _EXT_CMD_UPDATE_PROTECT_T fw_rts;

		fw_rts.ucProtectIdx = UPDATE_RTS_THRESHOLD;
		fw_rts.ucDbdcIdx = rts_thld.band_idx;
		fw_rts.Data.rUpdateRtsThld.u4RtsPktLenThreshold = cpu2le32(rts_thld.pkt_len_thld);
		fw_rts.Data.rUpdateRtsThld.u4RtsPktNumThreshold = cpu2le32(rts_thld.pkt_num_thld);
		MtCmdUpdateProtect(pAd, &fw_rts);
	}
}


INT MtAsicSetRDGByFw(RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg)
{
	struct _EXT_CMD_RDG_CTRL_T fw_rdg;

	fw_rdg.u4TxOP = Rdg->Txop;
	fw_rdg.ucLongNav = Rdg->LongNav;
	fw_rdg.ucInit = Rdg->Init;
	fw_rdg.ucResp = Rdg->Resp;
	fw_rdg.ucWlanIdx = Rdg->WlanIdx;
	fw_rdg.ucBand = Rdg->BandIdx;
	MtCmdSetRdg(pAd, &fw_rdg);
	return TRUE;
}


#ifdef DBDC_MODE
INT32  MtAsicGetDbdcCtrlByFw(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 ret;
	UINT32 i = 0, j = 0;
	/*DBDC enable will not need BctrlEntries so minus 1*/
	pbInfo->TotalNum = 0;
	/*PTA*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_PTA;
	pbInfo->BctrlEntries[i].Index = 0;
	i++;
	/*MU*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MU;
	pbInfo->BctrlEntries[i].Index = 0;
	i++;

	/*BF*/
	for (j = 0; j < 3; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BF;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*WMM*/
	for (j = 0; j < 4; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_WMM;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*MGMT*/
	for (j = 0; j < 2; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MGMT;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*MBSS*/
	for (j = 0; j < 15; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MBSS;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*BSS*/
	for (j = 0; j < 5; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BSS;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*Repeater*/
	for (j = 0; j < 32; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_REPEATER;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	pbInfo->TotalNum = i;
	ret = MtCmdGetDbdcCtrl(pAd, pbInfo);
	return ret;
}


INT32 MtAsicSetDbdcCtrlByFw(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 ret = 0;

	ret = MtCmdSetDbdcCtrl(pAd, pbInfo);
	return ret;
}

#endif /*DBDC_MODE*/

UINT32 MtAsicGetWmmParamByFw(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 ret, Value = 0;
	MT_EDCA_CTRL_T EdcaCtrl;

	os_zero_mem(&EdcaCtrl, sizeof(MT_EDCA_CTRL_T));
	EdcaCtrl.ucTotalNum = 1;
	EdcaCtrl.ucAction = EDCA_ACT_GET;
	EdcaCtrl.rAcParam[0].ucAcNum = AcNum;
	ret = MtCmdGetEdca(pAd, &EdcaCtrl);

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		Value = EdcaCtrl.rAcParam[0].u2Txop;
		break;

	case WMM_PARAM_AIFSN:
		Value = EdcaCtrl.rAcParam[0].ucAifs;
		break;

	case WMM_PARAM_CWMIN:
		Value = EdcaCtrl.rAcParam[0].ucWinMin;
		break;

	case WMM_PARAM_CWMAX:
		Value = EdcaCtrl.rAcParam[0].u2WinMax;
		break;

	default:
		Value = 0xdeadbeef;
		break;
	}

	return Value;
}

INT MtAsicGetTsfTimeByFirmware(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	TSF_RESULT_T TsfResult;

	MtCmdGetTsfTime(pAd, HwBssidIdx, &TsfResult);
	*high_part = TsfResult.u4TsfBit63_32;
	*low_part = TsfResult.u4TsfBit0_31;
	return TRUE;
}

UINT32 MtAsicGetChBusyCntByFw(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
	UINT32 msdr16, ret;

	ret = MtCmdGetChBusyCnt(pAd, ch_idx, &msdr16);
	return msdr16;
}


INT32 MtAsicSetMacTxRxByFw(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;

	ret = MtCmdSetMacTxRx(pAd, BandIdx, Enable);
	return ret;
}


INT32 MtAsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;

	ret = MtCmdSetRxvFilter(pAd, BandIdx, Enable);
	return ret;
}

VOID MtAsicDisableSyncByFw(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: BssInfo idx (%d) is INIT currently!!!\n",
				 __func__, wdev->bss_info_argument.ucBssIndex));
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_INITED;
	CmdSetSyncModeByBssInfoUpdate(pAd, wdev->bss_info_argument);
}

VOID MtAsicEnableBssSyncByFw(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: BssInfo idx (%d) is INIT currently!!!\n",
				 __func__, wdev->bss_info_argument.ucBssIndex));
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
	CmdSetSyncModeByBssInfoUpdate(pAd, wdev->bss_info_argument);
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
/* STARec Info */
INT32 MtAsicSetAid(
	RTMP_ADAPTER *pAd,
	UINT16 Aid)
{
	return CmdETxBfAidSetting(pAd,
							  Aid);
}
#endif

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
/* TODO: Carter/Star for Repeater can support DBDC, after define STA/APCLI/Repeater */
INT MtAsicSetReptFuncEnableByFw(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	EXT_CMD_MUAR_T config_muar;

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));

	if (bEnable == TRUE)
		config_muar.ucMuarModeSel = MUAR_REPEATER;
	else
		config_muar.ucMuarModeSel = MUAR_NORMAL;

	MtCmdMuarConfigSet(pAd, (UCHAR *)&config_muar);
	return TRUE;
}

VOID MtAsicInsertRepeaterEntryByFw(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR *pdata = NULL;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("\n%s %02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
			  __func__,
			  pAddr[0],
			  pAddr[1],
			  pAddr[2],
			  pAddr[3],
			  pAddr[4],
			  pAddr[5],
			  CliIdx));
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 1;
	config_muar.ucAccessMode = MUAR_WRITE;
	muar_entry.ucMuarIdx = (CliIdx * 2);
	COPY_MAC_ADDR(muar_entry.aucMacAddr, pAddr);
	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata + sizeof(EXT_CMD_MUAR_T),
				   &muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	os_free_mem(pdata);
}


VOID MtAsicRemoveRepeaterEntryByFw(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	UCHAR *pdata = NULL;
	UCHAR *ptr = NULL;
	UCHAR i = 0;
	UCHAR zeroMac[MAC_ADDR_LEN] = {0};
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 2;
	config_muar.ucAccessMode = MUAR_WRITE;
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) +
				 (config_muar.ucEntryCnt * sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T)));
	ptr = pdata;
	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	ptr = pdata + sizeof(EXT_CMD_MUAR_T);

	for (i = 0; i < config_muar.ucEntryCnt; i++) {
		muar_entry.ucMuarIdx = (CliIdx * 2) + i;
		COPY_MAC_ADDR(muar_entry.aucMacAddr, zeroMac);
		NdisMoveMemory(ptr,
					   &muar_entry,
					   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
		ptr = ptr + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T);
	}

	MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	os_free_mem(pdata);
}

VOID MtAsicInsertRepeaterRootEntryByFw(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	UCHAR *pdata = NULL;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("\n%s %02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
			  __func__,
			  pAddr[0],
			  pAddr[1],
			  pAddr[2],
			  pAddr[3],
			  pAddr[4],
			  pAddr[5],
			  ReptCliIdx));
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 1;
	config_muar.ucAccessMode = MUAR_WRITE;
	muar_entry.ucMuarIdx = (ReptCliIdx * 2) + 1;
	COPY_MAC_ADDR(muar_entry.aucMacAddr, pAddr);
	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata + sizeof(EXT_CMD_MUAR_T),
				   &muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	os_free_mem(pdata);
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

