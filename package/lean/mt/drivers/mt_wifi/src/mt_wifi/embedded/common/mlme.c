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
	mlme.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include <stdarg.h>
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef WSC_INCLUDED
#include "arris_wps_gpio_handler.h"
#endif
#endif

UCHAR OfdmRateToRxwiMCS[12] = {
	0,  0,	0,  0,
	0,  1,	2,  3,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};

UCHAR RxwiMCSToOfdmRate[12] = {
	RATE_6,  RATE_9,	RATE_12,  RATE_18,
	RATE_24,  RATE_36,	RATE_48,  RATE_54,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};

extern UCHAR ZeroSsid[MAX_LEN_OF_SSID];


/* since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.*/
/* otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate*/
ULONG BasicRateMask[12] = {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
						   0xfffff01f /* 6 */, 0xfffff03f /* 9 */, 0xfffff07f /* 12 */, 0xfffff0ff /* 18 */,
						   0xfffff1ff /* 24 */, 0xfffff3ff /* 36 */, 0xfffff7ff /* 48 */, 0xffffffff /* 54 */
						  };

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than*/
/*		this value, then it's quaranteed capable of operating in 36 mbps TX rate in*/
/*		clean environment.*/
/*								  TxRate: 1   2   5.5	11	 6	  9    12	18	 24   36   48	54	 72  100*/
CHAR RssiSafeLevelForTxRate[] = {  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[] = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR SsidIe = IE_SSID;
UCHAR SupRateIe = IE_SUPP_RATES;
UCHAR ExtRateIe = IE_EXT_SUPP_RATES;
#ifdef DOT11_N_SUPPORT
UCHAR HtCapIe = IE_HT_CAP;
UCHAR AddHtInfoIe = IE_ADD_HT;
UCHAR NewExtChanIe = IE_SECONDARY_CH_OFFSET;
UCHAR BssCoexistIe = IE_2040_BSS_COEXIST;
UCHAR ExtHtCapIe = IE_EXT_CAPABILITY;
#endif /* DOT11_N_SUPPORT */
UCHAR ExtCapIe = IE_EXT_CAPABILITY;
UCHAR ErpIe = IE_ERP;
UCHAR DsIe = IE_DS_PARM;
UCHAR TimIe = IE_TIM;
UCHAR WpaIe = IE_WPA;
UCHAR Wpa2Ie = IE_WPA2;
UCHAR IbssIe = IE_IBSS_PARM;
UCHAR WapiIe = IE_WAPI;

#ifdef DYNAMIC_VGA_SUPPORT
void periodic_monitor_false_cca_adjust_vga(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable) && (cap->dynamic_vga_support) &&
		OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		UCHAR val1, val2;
		UINT32 bbp_val1, bbp_val2;

		RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val1);
		val1 = ((bbp_val1 & (0x00007f00)) >> 8) & 0x7f;
		RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val2);
		val2 = ((bbp_val2 & (0x00007f00)) >> 8) & 0x7f;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("vga_init_0 = %x, vga_init_1 = %x\n",
				 pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0, pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1));
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("one second False CCA=%d, fixed agc_vga_0:0%x, fixed agc_vga_1:0%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, val1,
				  val2));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.lna_vga_ctl.nFalseCCATh) {
			if (val1 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 - 0x10)) {
				val1 -= 0x02;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
			}

			if (pAd->Antenna.field.RxPath >= 2) {
				if (val2 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1 - 0x10)) {
					val2 -= 0x02;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		} else if (pAd->RalinkCounters.OneSecFalseCCACnt <
				   pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh) {
			if (val1 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0) {
				val1 += 0x02;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
			}

			if (pAd->Antenna.field.RxPath >= 2) {
				if (val2 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1) {
					val2 += 0x02;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		}
	}
}

#endif

VOID set_default_ap_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid = TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 1;
	pEdca->Aifsn[3] = 1;
	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;
	pEdca->Cwmax[0] = 6;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;
	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;
	pEdca->Txop[3]  = 47;
}


VOID set_default_sta_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid = TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 2;
	pEdca->Aifsn[3] = 2;
	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;
	pEdca->Cwmax[0] = 10;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;
	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;	/*96; */
	pEdca->Txop[3]  = 47;	/*48; */
}


UCHAR dot11_max_sup_rate(INT SupRateLen, UCHAR *SupRate, INT ExtRateLen, UCHAR *ExtRate)
{
	INT idx;
	UCHAR MaxSupportedRateIn500Kbps = 0;

	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (idx = 0; idx < SupRateLen; idx++) {
		if (MaxSupportedRateIn500Kbps < (SupRate[idx] & 0x7f))
			MaxSupportedRateIn500Kbps = SupRate[idx] & 0x7f;
	}

	if (ExtRateLen > 0 && ExtRate != NULL) {
		for (idx = 0; idx < ExtRateLen; idx++) {
			if (MaxSupportedRateIn500Kbps < (ExtRate[idx] & 0x7f))
				MaxSupportedRateIn500Kbps = ExtRate[idx] & 0x7f;
		}
	}

	return MaxSupportedRateIn500Kbps;
}


UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps)
{
	UCHAR MaxSupportedRate;

	switch (MaxSupportedRateIn500Kbps) {
	case 108:
		MaxSupportedRate = RATE_54;
		break;

	case 96:
		MaxSupportedRate = RATE_48;
		break;

	case 72:
		MaxSupportedRate = RATE_36;
		break;

	case 48:
		MaxSupportedRate = RATE_24;
		break;

	case 36:
		MaxSupportedRate = RATE_18;
		break;

	case 24:
		MaxSupportedRate = RATE_12;
		break;

	case 18:
		MaxSupportedRate = RATE_9;
		break;

	case 12:
		MaxSupportedRate = RATE_6;
		break;

	case 22:
		MaxSupportedRate = RATE_11;
		break;

	case 11:
		MaxSupportedRate = RATE_5_5;
		break;

	case 4:
		MaxSupportedRate = RATE_2;
		break;

	case 2:
		MaxSupportedRate = RATE_1;
		break;

	default:
		MaxSupportedRate = RATE_11;
		break;
	}

	return MaxSupportedRate;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPSuspendMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCANNING, suspend MSDU transmission ...\n"));
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */

	/* no carrier detection when scanning */
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStop(pAd);

#endif
#endif /* CONFIG_AP_SUPPORT */
	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);
	MSDU_FORBID_SET(wdev, MSDU_FORBID_CHANNEL_MISMATCH);
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPResumeMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct qm_ops *qm_ops = pAd->qm_ops;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCAN done, resume MSDU transmission ...\n"));
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT

	/* no carrier detection when scanning*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0) {
		pAd->BbpTuning.R66CurrentValue = 0x38;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}

	bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);
	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CHANNEL_MISMATCH);

	qm_ops->schedule_tx_que(pAd);
}


/*
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;
	MAC_TABLE_ENTRY *pEntry;
#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAd, pAddr);
#endif
	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;

	if (NState == NDIS_STATUS_SUCCESS) {
		frm_len = sizeof(HEADER_802_11);
#ifdef CONFIG_AP_SUPPORT

		/* IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
		if (pEntry && (pEntry->wdev->wdev_type == WDEV_TYPE_AP
					   || pEntry->wdev->wdev_type == WDEV_TYPE_GO)) {
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							 pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							 pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			goto body;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
body:
#endif /* CONFIG_AP_SUPPORT */
		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull) {
			UCHAR *qos_p = ((UCHAR *)pNullFr) + frm_len;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;
			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			frm_len += 2;
		} else
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate],
				 AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (PUCHAR)pNullFr, frm_len);
		MlmeFreeMemory(pFrame);
	}
}


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
VOID ApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN		bQosNull,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	IN	USHORT			PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	STA_TR_ENTRY *tr_entry;
	PAPCLI_STRUCT pApCliEntry = NULL;
	struct wifi_dev *wdev;

	if (!pMacEntry) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): pMacEntry is null!", __func__));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
	wdev = &pApCliEntry->wdev;

	/* WPA 802.1x secured port control */
	/* TODO: shiang-usw, check [wdev/tr_entry]->PortSecured! */
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
		(tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);
	wifi_hdr = (HEADER_802_11 *)NullFrame;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = SUBTYPE_DATA_NULL;
	wifi_hdr->FC.ToDs = 1;
	COPY_MAC_ADDR(wifi_hdr->Addr1, pMacEntry->Addr);
#ifdef MAC_REPEATER_SUPPORT

	if (pMacEntry->bReptCli == TRUE)
		COPY_MAC_ADDR(wifi_hdr->Addr2, pMacEntry->ReptCliAddr);
	else
#endif /* MAC_REPEATER_SUPPORT */
		COPY_MAC_ADDR(wifi_hdr->Addr2, pApCliEntry->wdev.if_addr);

	COPY_MAC_ADDR(wifi_hdr->Addr3, pMacEntry->Addr);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = PwrMgmt;

	wifi_hdr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);
	/* sequence is increased in MlmeHardTx */
	wifi_hdr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		wifi_hdr->FC.SubType = SUBTYPE_QOS_NULL;
		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);
}
#endif/*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */



static VOID mlme_suspend(struct _MLME_STRUCT *mlme)
{
	NdisAcquireSpinLock(&mlme->TaskLock);
	mlme->suspend = TRUE;
	NdisReleaseSpinLock(&mlme->TaskLock);
}

static VOID mlme_resume(struct _MLME_STRUCT *mlme)
{
	NdisAcquireSpinLock(&mlme->TaskLock);
	mlme->suspend = FALSE;
	NdisReleaseSpinLock(&mlme->TaskLock);
}

static BOOLEAN mlme_requeue(struct _MLME_STRUCT *mlme, struct _MLME_QUEUE_ELEM *elem)
{
	INT tail;
	MLME_QUEUE	 *Queue = &mlme->Queue;

	NdisAcquireSpinLock(&(Queue->Lock));
	tail = Queue->Tail;

	/*Double check for safety in multi-thread system*/
	if (Queue->Entry[tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	os_move_mem(&Queue->Entry[tail], elem, sizeof(Queue->Entry[tail]));
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

static INT mlme_bss_clear_by_wdev(struct _MLME_STRUCT *mlme, struct wifi_dev *wdev)
{
	struct _MLME_QUEUE_ELEM *elem = NULL;
	INT elem_num = mlme->Queue.Num;
	INT i;

	NdisAcquireSpinLock(&mlme->TaskLock);
	for (i = 0 ; i < elem_num ; i++) {
		if (!MlmeDequeue(&mlme->Queue, &elem))
			break;
		/*if not owned by this bss, enqueue again*/
		if (elem->wdev != wdev)
			mlme_requeue(mlme, elem);
		/* free MLME element*/
		elem->Occupied = FALSE;
		elem->MsgLen = 0;
	}
	NdisReleaseSpinLock(&mlme->TaskLock);

	return TRUE;
}

static BOOLEAN mlme_bss_clear(struct _MLME_STRUCT *mlme, struct wifi_dev *wdev)
{
	INT cnt = 0;
	BOOLEAN ret = TRUE;
	/*suspend mlme a while*/
	mlme_suspend(mlme);
	/*check mlme is idle*/
	while (mlme->bRunning && cnt < 10) {
		OS_WAIT(100);
		cnt++;
	}
	/*check mlme polling idle status*/
	if (mlme->bRunning) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("mlme can't polling to idle, timeout\n"));
		ret = FALSE;
		goto end;
	}
	/*clear bss related mlme entry*/
	mlme_bss_clear_by_wdev(mlme, wdev);
	/*resume mlme now*/
end:
	mlme_resume(mlme);
	return ret;
}

/*
	==========================================================================
	Description:
		main loop of the MLME
	Pre:
		Mlme has to be initialized, and there are something inside the queue
	Note:
		This function is invoked from MPSetInformation and MPReceive;
		This task guarantee only one FSM will run.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeHandler(RTMP_ADAPTER *pAd)
{
	MLME_QUEUE_ELEM *Elem = NULL;
#ifdef APCLI_SUPPORT
	SHORT apcliIfIndex;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xff;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /*MAC_REPEATER_SUPPORT*/
#endif /* APCLI_SUPPORT */
	/* Only accept MLME and Frame from peer side, no other (control/data) frame should*/
	/* get into this state machine*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);

	if (pAd->Mlme.bRunning) {
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} else {
		pAd->Mlme.bRunning = TRUE;
	}

	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND) ||
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
		   ) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("System halted, removed or MlmeRest, exit MlmeTask!(QNum = %ld)\n",
					  pAd->Mlme.Queue.Num));
			break;
		}

#ifdef CONFIG_ATE

		if (ATE_ON(pAd)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Driver is in ATE mode\n", __func__));
			break;
		}

#endif /* CONFIG_ATE */

		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) {

			/* if dequeue success*/
			switch (Elem->Machine) {
				/* STA state machines*/

			case ACTION_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine,
										  Elem, pAd->Mlme.ActMachine.CurrState);
				break;
#ifdef CONFIG_AP_SUPPORT

			/* AP state amchines*/

			case AP_ASSOC_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.ApAssocMachine,
										  Elem, pAd->Mlme.ApAssocMachine.CurrState);
				break;

			case AP_AUTH_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.ApAuthMachine,
										  Elem, pAd->Mlme.ApAuthMachine.CurrState);
				break;

			case AP_SYNC_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.ApSyncMachine,
										  Elem, pAd->Mlme.ApSyncMachine.CurrState);
				break;
#ifdef APCLI_SUPPORT

			case APCLI_AUTH_STATE_MACHINE:
				apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT

				if (apcliIfIndex >= REPT_MLME_START_IDX) {
					CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
					ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
					apcliIfIndex = pReptEntry->wdev->func_idx;
				} else
					CliIdx = 0xff;

#endif /* MAC_REPEATER_SUPPORT */

				if (isValidApCliIf(apcliIfIndex)) {
					ULONG AuthCurrState;
#ifdef MAC_REPEATER_SUPPORT

					if (CliIdx != 0xff)
						AuthCurrState = pReptEntry->AuthCurrState;
					else
#endif /* MAC_REPEATER_SUPPORT */
						AuthCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AuthCurrState;

					StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAuthMachine,
											  Elem, AuthCurrState);
				}

				break;

			case APCLI_ASSOC_STATE_MACHINE:
				apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT

				if (apcliIfIndex >= REPT_MLME_START_IDX) {
					CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
					ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
					apcliIfIndex = pReptEntry->wdev->func_idx;
				} else
					CliIdx = 0xff;

#endif /* MAC_REPEATER_SUPPORT */

				if (isValidApCliIf(apcliIfIndex)) {
					ULONG AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AssocCurrState;
#ifdef MAC_REPEATER_SUPPORT

					if (CliIdx != 0xff)
						AssocCurrState = pReptEntry->AssocCurrState;

#endif /* MAC_REPEATER_SUPPORT */
					StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAssocMachine,
											  Elem, AssocCurrState);
				}

				break;

			case APCLI_SYNC_STATE_MACHINE:
				apcliIfIndex = Elem->Priv;

				if (isValidApCliIf(apcliIfIndex))
					StateMachinePerformAction(pAd, &pAd->Mlme.ApCliSyncMachine, Elem,
											  (pAd->ApCfg.ApCliTab[apcliIfIndex].SyncCurrState));

				break;

			case APCLI_CTRL_STATE_MACHINE:
				apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT

				if (apcliIfIndex >= REPT_MLME_START_IDX) {
					CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
					ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
					apcliIfIndex = pReptEntry->wdev->func_idx;
				} else
					CliIdx = 0xff;

#endif /* MAC_REPEATER_SUPPORT */

				if (isValidApCliIf(apcliIfIndex)) {
					ULONG CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].CtrlCurrState;
#ifdef MAC_REPEATER_SUPPORT

					if (CliIdx != 0xff)
						CtrlCurrState = pReptEntry->CtrlCurrState;

#endif /* MAC_REPEATER_SUPPORT */
					StateMachinePerformAction(pAd, &pAd->Mlme.ApCliCtrlMachine, Elem, CtrlCurrState);
				}

				break;
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			case WPA_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WpaMachine, Elem, pAd->Mlme.WpaMachine.CurrState);
				break;
#ifdef WSC_INCLUDED

			case WSC_STATE_MACHINE:
				if (pAd->pWscElme) {
					RTMP_SEM_LOCK(&pAd->WscElmeLock);
					NdisMoveMemory(pAd->pWscElme, Elem, sizeof(MLME_QUEUE_ELEM));
					RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
					RtmpOsTaskWakeUp(&(pAd->wscTask));
				}

				break;
#endif /* WSC_INCLUDED */
#ifdef CONFIG_HOTSPOT

			case HSCTRL_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.HSCtrlMachine, Elem,
										  HSCtrlCurrentState(pAd, Elem));
				break;
#endif
#ifdef CONFIG_DOT11U_INTERWORKING

			case GAS_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.GASMachine, Elem,
										  GASPeerCurrentState(pAd, Elem));
				break;
#endif

#ifdef CONFIG_11KV_API_SUPPORT
#ifdef DOT11K_RRM_SUPPORT
		case BCN_STATE_MACHINE:
			StateMachinePerformAction(pAd, &pAd->Mlme.BCNMachine, Elem,
							BCNPeerCurrentState(pAd, Elem));
			break;
		case NEIGHBOR_STATE_MACHINE:
			StateMachinePerformAction(pAd, &pAd->Mlme.NRMachine, Elem,
							NRPeerCurrentState(pAd, Elem));
			break;
#endif
#endif

#ifdef CONFIG_DOT11V_WNM

			case BTM_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.BTMMachine, Elem,
										  BTMPeerCurrentState(pAd, Elem));
				break;
#ifdef CONFIG_HOTSPOT_R2

			case WNM_NOTIFY_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WNMNotifyMachine, Elem,
										  WNMNotifyPeerCurrentState(pAd, Elem));
				break;
#endif
#endif
#ifdef BACKGROUND_SCAN_SUPPORT

			case BGND_SCAN_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, Elem,
										  pAd->BgndScanCtrl.BgndScanStatMachine.CurrState);
				break;
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef MT_DFS_SUPPORT

			case DFS_STATE_MACHINE: /* Jelly20150402 */
				StateMachinePerformAction(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, Elem,
										  pAd->CommonCfg.DfsParameter.DfsStatMachine.CurrState);
				break;
#endif /* MT_DFS_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			case AUTO_CH_SEL_STATE_MACHINE: {
				UCHAR BandIdx = HcGetBandByWdev((struct wifi_dev *)Elem->Priv);
				AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
				StateMachinePerformAction(pAd, &pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine, Elem,
										  pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState);
				break;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WDS_SUPPORT
			case WDS_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WdsMachine,
										  Elem, pAd->Mlme.WdsMachine.CurrState);
				break;
#endif
			default:
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Illegal SM %ld\n",
						 __func__, Elem->Machine));
				break;
			} /* end of switch*/

			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): MlmeQ empty\n", __func__));
	}

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}

/*
========================================================================
Routine Description:
    MLME kernel thread.

Arguments:
	Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
static INT MlmeThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		goto LabelExit;

	RtmpOSTaskCustomize(pTask);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		/* lock the device pointers , need to check if required*/
		/*down(&(pAd->usbdev_semaphore)); */

		if (!pAd->PM_FlgSuspend && !pAd->Mlme.suspend)
			MlmeHandler(pAd);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
LabelExit:
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---%s\n", __func__));
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}



#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
static VOID ApCliMlmeInit(RTMP_ADAPTER *pAd)
{
	/* init apcli state machines*/
	ASSERT(APCLI_AUTH_FUNC_SIZE == APCLI_MAX_AUTH_MSG * APCLI_MAX_AUTH_STATE);
	ApCliAuthStateMachineInit(pAd, &pAd->Mlme.ApCliAuthMachine, pAd->Mlme.ApCliAuthFunc);
	ASSERT(APCLI_ASSOC_FUNC_SIZE == APCLI_MAX_ASSOC_MSG * APCLI_MAX_ASSOC_STATE);
	ApCliAssocStateMachineInit(pAd, &pAd->Mlme.ApCliAssocMachine, pAd->Mlme.ApCliAssocFunc);
	ASSERT(APCLI_SYNC_FUNC_SIZE == APCLI_MAX_SYNC_MSG * APCLI_MAX_SYNC_STATE);
	ApCliSyncStateMachineInit(pAd, &pAd->Mlme.ApCliSyncMachine, pAd->Mlme.ApCliSyncFunc);
	ASSERT(APCLI_CTRL_FUNC_SIZE == APCLI_MAX_CTRL_MSG * APCLI_MAX_CTRL_STATE);
	ApCliCtrlStateMachineInit(pAd, &pAd->Mlme.ApCliCtrlMachine, pAd->Mlme.ApCliCtrlFunc);
}
#endif /* APCLI_SUPPORT */

static VOID ApMlmeInit(RTMP_ADAPTER *pAd)
{
	/* init AP state machines*/
	APAssocStateMachineInit(pAd, &pAd->Mlme.ApAssocMachine, pAd->Mlme.ApAssocFunc);
	APAuthStateMachineInit(pAd, &pAd->Mlme.ApAuthMachine, pAd->Mlme.ApAuthFunc);
	APSyncStateMachineInit(pAd, &pAd->Mlme.ApSyncMachine, pAd->Mlme.ApSyncFunc);
#ifdef WDS_SUPPORT
	WdsStateMachineInit(pAd, &pAd->Mlme.WdsMachine, pAd->Mlme.WdsFunc);
#endif
	/* for Dot11H */
}
#endif /* CONFIG_AP_SUPPORT */

static INT mlme_for_wsys_notify_handle(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct wsys_notify_info *info = data;
	struct _MLME_STRUCT *mlme = ne->priv;
	struct wifi_dev *wdev = info->wdev;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): event_id: %d, wdev=%d\n", __func__, event_id, info->wdev->wdev_idx));

	switch (event_id) {
	case WSYS_NOTIFY_CLOSE:
		mlme_bss_clear(mlme, wdev);
		break;
	case WSYS_NOTIFY_OPEN:
	case WSYS_NOTIFY_CONNT_ACT:
	case WSYS_NOTIFY_DISCONNT_ACT:
	case WSYS_NOTIFY_LINKUP:
	case WSYS_NOTIFY_LINKDOWN:
	case WSYS_NOTIFY_STA_UPDATE:
	default:
		break;
	}
	return ret;
}

static INT mlme_notify_register(struct _RTMP_ADAPTER *ad, struct _MLME_STRUCT *mlme)
{
	INT ret;
	struct notify_entry *ne = &mlme->wsys_ne;

	/*fill notify entry for wifi system chain*/
	ne->notify_call = mlme_for_wsys_notify_handle;
	ne->priority = WSYS_NOTIFY_PRIORITY_MLME;
	ne->priv = mlme;
	/*register wifi system notify chain*/
	ret = register_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}

static INT mlme_notify_unregister(struct _RTMP_ADAPTER *ad, struct _MLME_STRUCT *mlme)
{
	INT ret;
	struct notify_entry *ne = &mlme->wsys_ne;

	/*register wifi system notify chain*/
	ret = unregister_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}

/*
	==========================================================================
	Description:
		initialize the MLME task and its data structure (queue, spinlock,
		timer, state machines).

	IRQL = PASSIVE_LEVEL

	Return:
		always return NDIS_STATUS_SUCCESS

	==========================================================================
*/
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> MLME Initialize\n"));

	do {
		Status = MlmeQueueInit(pAd, &pAd->Mlme.Queue);

		if (Status != NDIS_STATUS_SUCCESS)
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(pAd, &pAd->Mlme.TaskLock);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			ApMlmeInit(pAd);
#ifdef APCLI_SUPPORT
			ApCliMlmeInit(pAd);
#endif /* APCLI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
		/* Init Wsc state machine */
		ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
		WscStateMachineInit(pAd, &pAd->Mlme.WscMachine, pAd->Mlme.WscFunc);
#endif /* WSC_INCLUDED */
		WpaStateMachineInit(pAd, &pAd->Mlme.WpaMachine, pAd->Mlme.WpaFunc);
#ifdef CONFIG_HOTSPOT
		HSCtrlStateMachineInit(pAd, &pAd->Mlme.HSCtrlMachine, pAd->Mlme.HSCtrlFunc);
#endif /*CONFIG_HOTSPOT*/
#ifdef CONFIG_DOT11U_INTERWORKING
		GASStateMachineInit(pAd, &pAd->Mlme.GASMachine, pAd->Mlme.GASFunc);
#endif /*CONFIG_DOT11U_INTERWORKING*/

#ifdef CONFIG_11KV_API_SUPPORT
#ifdef DOT11K_RRM_SUPPORT
		RRMBcnReqStateMachineInit(pAd, &pAd->Mlme.BCNMachine, pAd->Mlme.BCNFunc);
		NRStateMachineInit(pAd, &pAd->Mlme.NRMachine, pAd->Mlme.NRFunc);
#endif
#endif /* CONFIG_11KV_API_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
		WNMCtrlInit(pAd);
		BTMStateMachineInit(pAd, &pAd->Mlme.BTMMachine, pAd->Mlme.BTMFunc);
#ifdef CONFIG_HOTSPOT_R2
		WNMNotifyStateMachineInit(pAd, &pAd->Mlme.WNMNotifyMachine, pAd->Mlme.WNMNotifyFunc);
#endif /*CONFIG_HOTSPOT_R2*/
#endif
		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);
		/* Init mlme periodic timer*/
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExecTimer), pAd, TRUE);
		/* Set mlme periodic timer*/
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
		/* software-based RX Antenna diversity*/
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* Init APSD periodic timer*/
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);
			/* Init APQuickResponseForRateUp timer.*/
			RTMPInitTimer(pAd, &pAd->ApCfg.ApQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(APQuickResponeForRateUpExec), pAd,
						  FALSE);
			pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
		/*CFG_TODO*/
		ApMlmeInit(pAd);
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		ApCliMlmeInit(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#endif /* RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */
	} while (FALSE);

	{
		RTMP_OS_TASK *pTask;
		/* Creat MLME Thread */
		pTask = &pAd->mlmeTask;
		RTMP_OS_TASK_INIT(pTask, "RtmpMlmeTask", pAd);
		Status = RtmpOSTaskAttach(pTask, MlmeThread, (ULONG)pTask);

		if (Status == NDIS_STATUS_FAILURE)
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unable to start MlmeThread\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
	}
	/*mlme is ready, register notify handle for wifi system event*/
	mlme_notify_register(pAd, &pAd->Mlme);
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- MLME Initialize\n"));
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	return Status;
}


/*
	==========================================================================
	Description:
		Destructor of MLME (Destroy queue, state machine, spin lock and timer)
	Parameters:
		Adapter - NIC Adapter pointer
	Post:
		The MLME task will no longer work properly

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID MlmeHalt(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
	RTMP_OS_TASK *pTask;
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> MlmeHalt\n"));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	/* Terminate Mlme Thread */
	pTask = &pAd->mlmeTask;

	if (RtmpOSTaskKill(pTask) == NDIS_STATUS_FAILURE)
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("kill mlme task failed!\n"));

	/*unregister notify for wifi system*/
	mlme_notify_unregister(pAd, &pAd->Mlme);
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			/* disable BEACON generation and other BEACON related hardware timers*/
			AsicDisableSync(pAd, HW_BSSID_0);
		}

	RTMPReleaseTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);
	RTMPReleaseTimer(&pAd->Mlme.RxAntEvalTimer, &Cancelled);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR idx;

		idx = 0;
		RTMPReleaseTimer(&pAd->Mlme.APSDPeriodicTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
#ifdef APCLI_SUPPORT

		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[idx];

			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ProbeTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ApCliAssocTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ApCliAuthTimer, &Cancelled);
#ifdef APCLI_CERT_SUPPORT
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.WpaDisassocAndBlockAssocTimer, &Cancelled);
#endif /* APCLI_CERT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
			RTMPReleaseTimer(&pApCliEntry->TrialConnectTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectPhase2Timer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectRetryTimer, &Cancelled);
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef WSC_AP_SUPPORT

			if (pApCliEntry->wdev.WscControl.WscProfileRetryTimerRunning) {
				pApCliEntry->wdev.WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPReleaseTimer(&pApCliEntry->wdev.WscControl.WscProfileRetryTimer, &Cancelled);
			}

#endif /* WSC_AP_SUPPORT */
		}

#endif /* APCLI_SUPPORT */
		RTMPReleaseTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_HOTSPOT
	HSCtrlHalt(pAd);
	HSCtrlExit(pAd);
#endif
#ifdef CONFIG_DOT11U_INTERWORKING
	GASCtrlExit(pAd);
#endif
#ifdef CONFIG_DOT11V_WNM
	WNMCtrlExit(pAd);
#endif

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
#ifdef LED_CONTROL_SUPPORT
		/* Set LED*/
		RTMPSetLED(pAd, LED_HALT);
		RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it.*/
#endif /* LED_CONTROL_SUPPORT */
			if ((pChipOps->AsicHaltAction))
				pChipOps->AsicHaltAction(pAd);
	}

	RtmpusecDelay(5000);    /*  5 msec to gurantee Ant Diversity timer canceled*/
	MlmeQueueDestroy(&pAd->Mlme.Queue);
	NdisFreeSpinLock(&pAd->Mlme.TaskLock);
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== MlmeHalt\n"));
}


VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
		/* for performace enchanement */
		NdisZeroMemory(&pAd->RalinkCounters,
					   (LONG)&pAd->RalinkCounters.OneSecEnd -
					   (LONG)&pAd->RalinkCounters.OneSecStart);

	return;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically to -
		1. Decide if it's a right time to turn on PwrMgmt bit of all
		   outgoiing frames
		2. Calculate ChannelQuality based on statistics of the last
		   period, so that TX rate won't toggling very frequently between a
		   successful TX and a failed TX.
		3. If the calculated ChannelQuality indicated current connection not
		   healthy, then a ROAMing attempt is tried here.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  /* 8 sec*/
VOID MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MlmePeriodicExec(SystemSpecific1, FunctionContext, SystemSpecific2, SystemSpecific3);
}

VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ULONG TxTotalCnt;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef NF_SUPPORT

	if (IS_MT7615(pAd)) {
		UINT8 NF_index = 0;
		UINT32 Value = 0, value[11] = {0}, total = 0;
		INT16 NF = 0;
		INT16 NF_Power[] = {-92, -89, -86, -83, -80, -75, -70, -65, -60, -55, -52};
		for (NF_index = 0; NF_index <= 10; NF_index++) {
			 MAC_IO_READ32(pAd, (0x12250 + 4 * NF_index), &value[NF_index]);
			 total += value[NF_index];
		}
		for (NF_index = 0; NF_index <= 10; NF_index++) {
			 NF += (NF_Power[NF_index] * (INT16)value[NF_index])/(INT16)total;
		}
		pAd->Avg_NFx16 = (pAd->Avg_NF != 0) ? (NF - pAd->Avg_NF + pAd->Avg_NFx16) : NF << 4;
		pAd->Avg_NF = pAd->Avg_NFx16 >> 4;

		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): NF %d Avg_NF = %d \t Avg_NFx16 = %d\n",  __FUNCTION__,
				 NF, pAd->Avg_NF, pAd->Avg_NFx16));

		/* Reset */
		HW_IO_READ32(pAd, PHY_RXTD_12, &Value);
		Value |= (1 << B0IrpiSwCtrlResetOffset);
		Value |= (1 << B0IrpiSwCtrlOnlyOffset);
		HW_IO_WRITE32(pAd, PHY_RXTD_12, Value);
		HW_IO_WRITE32(pAd, PHY_RXTD_12, Value);
		/* Enable badn0 IPI control */
		HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5, &Value);
		Value |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);
		HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5, Value);
	}
#endif

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	if (pAd->fgAdaptRxBlock) {
		/* check RSSI each 100 ms  */
		if (pAd->u1TimeCnt >= pAd->u1CheckTime) {
			RTMP_CHECK_RSSI(pAd);
			pAd->u1TimeCnt = 0;
		} else
			pAd->u1TimeCnt++;
	}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

	/* CFG MCC */
#ifdef MICROWAVE_OVEN_SUPPORT
	/* update False CCA count to an array */
	NICUpdateRxStatusCnt1(pAd, pAd->Mlme.PeriodicRound % 10);

	if (pAd->CommonCfg.MO_Cfg.bEnable) {
		UINT8 stage = pAd->Mlme.PeriodicRound % 10;

		if (stage == MO_MEAS_PERIOD) {
			ASIC_MEASURE_FALSE_CCA(pAd);
			pAd->CommonCfg.MO_Cfg.nPeriod_Cnt = 0;
		} else if (stage == MO_IDLE_PERIOD) {
			UINT16 Idx;

			for (Idx = MO_MEAS_PERIOD + 1; Idx < MO_IDLE_PERIOD + 1; Idx++)
				pAd->CommonCfg.MO_Cfg.nFalseCCACnt += pAd->RalinkCounters.FalseCCACnt_100MS[Idx];

			/* printk("%s: fales cca1 %d\n", __func__, pAd->CommonCfg.MO_Cfg.nFalseCCACnt); */
			if (pAd->CommonCfg.MO_Cfg.nFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				ASIC_MITIGATE_MICROWAVE(pAd);
		}
	}

#endif /* MICROWAVE_OVEN_SUPPORT */

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
							  fRTMP_ADAPTER_RADIO_MEASUREMENT |
							  fRTMP_ADAPTER_NIC_NOT_EXIST)))
		|| IsHcAllSupportedBandsRadioOff(pAd)) {
		return;
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;
	pAd->bUpdateBcnCntDone = FALSE;
	/*	RECBATimerTimeout(SystemSpecific1,FunctionContext,SystemSpecific2,SystemSpecific3);*/
	pAd->Mlme.PeriodicRound++;
	pAd->Mlme.GPIORound++;
#if defined(MT7615) || defined(MT7622)

	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
#ifdef CONFIG_AP_SUPPORT
		BcnCheck(pAd);
#endif /* CONFIG_AP_SUPPORT */

		if ((pAd->Mlme.PeriodicRound % 5) == 0) { /* 500ms update */
			Update_Mib_Bucket_500Ms(pAd);
#ifdef SMART_CARRIER_SENSE_SUPPORT
			Smart_Carrier_Sense(pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		}
	}

#endif /* MT7615 || MT7622 */
#ifdef RANDOM_PKT_GEN
	regular_pause_umac(pAd);
#endif

	if (ops->heart_beat_check)
		ops->heart_beat_check(pAd);

	ba_timeout_monitor(pAd);
#ifdef MT_MAC

	/* Following is the TxOP scenario, monitor traffic in every minutes */
	if ((pAd->Mlme.PeriodicRound % 1) == 0) {
#ifdef RTMP_MAC_PCI

		if (pAd->PDMAWatchDogEn)
			PDMAWatchDog(pAd);

#endif

		if (pAd->PSEWatchDogEn)
			PSEWatchDog(pAd);
	}

#endif /* MT_MAC */

	/* by default, execute every 500ms */
	if ((pAd->ra_interval) &&
		((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0)) {
#ifdef MT_MAC
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			if (cap->fgRateAdaptFWOffload == TRUE) {
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
				/* AsicRssiUpdate(pAd); */
				/* AsicTxCntUpdate(pAd, 0); */
			}
		}

#endif /* MT_MAC */

		if (RTMPAutoRateSwitchCheck(pAd) == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				APMlmeDynamicTxRateSwitching(pAd);
#endif /* CONFIG_AP_SUPPORT */
			}
		} else {
#ifdef MT_MAC

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				MAC_TABLE_ENTRY *pEntry;
				MT_TX_COUNTER TxInfo;
				UINT16 i;

				/* TODO:Carter, check why start from 1 */
				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					if (i >= MAX_LEN_OF_MAC_TABLE)
						break;

					/* point to information of the individual station */
					pEntry = &pAd->MacTab.Content[i];

					if (IS_ENTRY_NONE(pEntry))
						continue;

					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#ifdef APCLI_SUPPORT

					if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#ifdef MAC_REPEATER_SUPPORT

					if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#endif
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

					if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx))
						continue;

#endif /* WDS_SUPPORT */

					if (IS_VALID_ENTRY(pEntry))
						AsicTxCntUpdate(pAd, pEntry->wcid, &TxInfo);
				}
			}

#endif /* MT_MAC */
		}
	}


#ifdef SMART_ANTENNA
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if ((pAd->ra_interval) &&
			((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0) &&
			RTMP_SA_WORK_ON(pAd)) {
			if (RTMPAutoRateSwitchCheck(pAd) == FALSE) {
				TX_STA_CNT1_STRUC StaTx1;
				TX_STA_CNT0_STRUC TxStaCnt0;
				RTMP_SA_TRAINING_PARAM *pTrainEntry = &pAd->pSAParam->trainEntry[0];
				/* Update statistic counter */
				NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

				if (pTrainEntry->pMacEntry && (pTrainEntry->trainStage != SA_INVALID_STAGE))
					pTrainEntry->mcsStableCnt++;
			}

			/* Check if need to run antenna adaptation */
			RtmpSAChkAndGo(pAd);
		}
	}
#endif /* SMART_ANTENNA */

	/* Normal 1 second Mlme PeriodicExec.*/
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) {
		pAd->Mlme.OneSecPeriodicRound++;

			if (IS_ASIC_CAP(pAd, fASIC_CAP_WMM_PKTDETECT_OFFLOAD)) {
				MtCmdCr4QueryBssAcQPktNum(pAd, CR4_GET_BSS_ACQ_PKT_NUM_CMD_DEFAULT);
			} else {
				mt_dynamic_wmm_be_tx_op(pAd, ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
			}

		NdisGetSystemUpTime(&pAd->Mlme.Now32);
		/* add the most up-to-date h/w raw counters into software variable, so that*/
		/* the dynamic tuning mechanism below are based on most up-to-date information*/
		/* Hint: throughput impact is very serious in the function */
		/* NICUpdateRawCountersNew(pAd); */
		RTMP_UPDATE_RAW_COUNTER(pAd);
		RTMP_SECOND_CCA_DETECTION(pAd);
#ifdef DYNAMIC_VGA_SUPPORT
		dynamic_vga_adjust(pAd);
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef DOT11_N_SUPPORT
		/* Need statistics after read counter. So put after NICUpdateRawCountersNew*/
		ORIBATimerTimeout(pAd);
#endif /* DOT11_N_SUPPORT */
		/*
			if (pAd->RalinkCounters.MgmtRingFullCount >= 2)
				RTMP_SET_FLAG(pAd, fRTMP_HW_ERR);
			else
				pAd->RalinkCounters.MgmtRingFullCount = 0;
		*/
		/* The time period for checking antenna is according to traffic*/
		{
			if (pAd->Mlme.bEnableAutoAntennaCheck) {
				TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
							 pAd->RalinkCounters.OneSecTxRetryOkCount +
							 pAd->RalinkCounters.OneSecTxFailCount;

				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50) {
					if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
						AsicEvaluateRxAnt(pAd);
				} else {
					if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
						AsicEvaluateRxAnt(pAd);
				}
			}
		}

#ifdef VIDEO_TURBINE_SUPPORT
		/*
			VideoTurbineUpdate(pAd);
			VideoTurbineDynamicTune(pAd);
		*/
#endif /* VIDEO_TURBINE_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef MT_MAC

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				/* Not RDG, update the TxOP else keep the default RDG's TxOP */
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == FALSE) {
				}
			}

#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */
		/* update RSSI each 1 second*/
		RTMP_SET_UPDATE_RSSI(pAd);


		/*Update some MIB counter per second */
		Update_Mib_Bucket_One_Sec(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef CONFIG_HOTSPOT_R2
#ifdef AP_QLOAD_SUPPORT
			/* QBSS_LoadUpdate from Mib_Bucket */
			hotspot_update_ap_qload_to_bcn(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_HOTSPOT_R2 */
			APMlmePeriodicExec(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT

			if (pAd->BgndScanCtrl.BgndScanSupport) {
				ChannelQualityDetection(pAd);
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				CCI_ACI_scenario_maintain(pAd);
#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
			Mumimo_scenario_maintain(pAd);
#endif /* MT_MAC && VHT_TXBF_SUPPORT */
			}


			if ((pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H[0].RDMode != RD_SILENCE_MODE))
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode != WDS_BRIDGE_MODE)
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
			   )
				pAd->macwd++;
			else
				pAd->macwd = 0;


			if (pAd->macwd > 1) {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MAC specific condition\n"));
				AsicSetMacWD(pAd);
#ifdef AP_QLOAD_SUPPORT
				Show_QoSLoad_Proc(pAd, NULL);
#endif /* AP_QLOAD_SUPPORT */
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		RTMP_SECOND_CCA_DETECTION(pAd);
		MlmeResetRalinkCounters(pAd);
		RTMP_MLME_HANDLER(pAd);
	}

#ifdef CONFIG_AP_SUPPORT

	/* (pAd->ScanCtrl.PartialScan.NumOfChannels == DEFLAUT_PARTIAL_SCAN_CH_NUM) means that one partial scan is finished */
	if (pAd->ScanCtrl.PartialScan.bScanning == TRUE  &&
		pAd->ScanCtrl.PartialScan.NumOfChannels == DEFLAUT_PARTIAL_SCAN_CH_NUM) {
		UCHAR ScanType = SCAN_ACTIVE;

		if ((pAd->ScanCtrl.PartialScan.BreakTime++) % DEFLAUT_PARTIAL_SCAN_BREAK_TIME == 0) {
			struct wifi_dev *pwdev = pAd->ScanCtrl.PartialScan.pwdev;
			if (pwdev) {
				if (!ApScanRunning(pAd, pwdev)) {
#ifdef WSC_AP_SUPPORT
#ifdef APCLI_SUPPORT
						if ((pwdev->wdev_type == WDEV_TYPE_APCLI) &&
							(pwdev->func_idx < MAX_APCLI_NUM)) {
							WSC_CTRL *pWpsCtrl = &pAd->ApCfg.ApCliTab[pwdev->func_idx].WscControl;

							if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
								(pWpsCtrl->bWscTrigger == TRUE))
								ScanType = SCAN_WSC_ACTIVE;
						}
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
						ApSiteSurvey_by_wdev(pAd, NULL, ScanType, FALSE, pwdev);
				}
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
	WSC_HDR_BTN_MR_HANDLE(pAd);
#ifdef VENDOR_FEATURE7_SUPPORT
	if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[0].wdev.PhyMode))
		arris_wps_led_handler(WIFI_50_RADIO);
	else
		arris_wps_led_handler(WIFI_24_RADIO);
#endif
#endif /* WSC_INCLUDED */
	pAd->bUpdateBcnCntDone = FALSE;
}


/*
	==========================================================================
	Validate SSID for connection try and rescan purpose
	Valid SSID will have visible chars only.
	The valid length is from 0 to 32.
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
BOOLEAN MlmeValidateSSID(UCHAR *pSsid, UCHAR SsidLen)
{
	int index;

	if (SsidLen > MAX_LEN_OF_SSID)
		return FALSE;

	/* Check each character value*/
	for (index = 0; index < SsidLen; index++) {
		if (pSsid[index] < 0x20)
			return FALSE;
	}

	/* All checked*/
	return TRUE;
}



#ifdef STA_LP_PHASE_1_SUPPORT
VOID RTMPSetEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	/* pAd->StaCfg[0].PwrMgmt.bEnterPsmNull = TRUE; */
	pPwrMgmt->bEnterPsmNull = TRUE;
}

VOID RTMPClearEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	/* pAd->StaCfg[0].PwrMgmt.bEnterPsmNull = FALSE; */
	pPwrMgmt->bEnterPsmNull = FALSE;
}

BOOLEAN RTMPEnterPsmNullBitStatus(PPWR_MGMT_STRUCT pPwrMgmt)
{
	/* return (pAd->StaCfg[0].PwrMgmt.bEnterPsmNull); */
	return pPwrMgmt->bEnterPsmNull;
}
#endif /* STA_LP_PHASE_1_SUPPORT */

/*
	==========================================================================
	Description:
		This routine calculates TxPER, RxPER of the past N-sec period. And
		according to the calculation result, ChannelQuality is calculated here
		to decide if current AP is still doing the job.

		If ChannelQuality is not good, a ROAMing attempt may be tried later.
	Output:
		StaCfg[0].ChannelQuality - 0..100

	IRQL = DISPATCH_LEVEL

	NOTE: This routine decide channle quality based on RX CRC error ratio.
		Caller should make sure a function call to NICUpdateRawCountersNew(pAd)
		is performed right before this routine, so that this routine can decide
		channel quality based on the most up-to-date information
	==========================================================================
 */
VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	RSSI_SAMPLE *pRssiSample = NULL;
	UINT32 OneSecTxNoRetryOkCount = 0;
	UINT32 OneSecTxRetryOkCount = 0;
	UINT32 OneSecTxFailCount = 0;
	UINT32 OneSecRxOkCnt = 0;
	UINT32 OneSecRxFcsErrCnt = 0;
	ULONG ChannelQuality = 0;  /* 0..100, Channel Quality Indication for Roaming*/
#ifdef CONFIG_MULTI_CHANNEL

	if (pAd->Mlme.bStartMcc)
		BeaconLostTime += (8 * OS_HZ); /* increase 8 seconds */
	else
		BeaconLostTime = pStaCfg->BeaconLostTime;

#endif /* CONFIG_MULTI_CHANNEL */

	if (pMacEntry != NULL) {
		pRssiSample = &pMacEntry->RssiSample;
		OneSecTxNoRetryOkCount = pMacEntry->OneSecTxNoRetryOkCount;
		OneSecTxRetryOkCount = pMacEntry->OneSecTxRetryOkCount;
		OneSecTxFailCount = pMacEntry->OneSecTxFailCount;
		OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
		OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
	} else {
		pRssiSample = &pAd->MacTab.Content[0].RssiSample;
		OneSecTxNoRetryOkCount = pAd->RalinkCounters.OneSecTxNoRetryOkCount;
		OneSecTxRetryOkCount = pAd->RalinkCounters.OneSecTxRetryOkCount;
		OneSecTxFailCount = pAd->RalinkCounters.OneSecTxFailCount;
		OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
		OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
	}

	if (pRssiSample == NULL)
		return;

	MaxRssi = RTMPMaxRssi(pAd, pRssiSample->LastRssi[0],
						  pRssiSample->LastRssi[1],
						  pRssiSample->LastRssi[2]);
	/*
		calculate TX packet error ratio and TX retry ratio - if too few TX samples,
		skip TX related statistics
	*/
	TxOkCnt = OneSecTxNoRetryOkCount + OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + OneSecTxFailCount;

	if (TxCnt < 5) {
		TxPER = 0;
		TxPRR = 0;
	} else {
		TxPER = (OneSecTxFailCount * 100) / TxCnt;
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	/* calculate RX PER - don't take RxPER into consideration if too few sample*/
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;

	if (RxCnt < 5)
		RxPER = 0;
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

#if defined(CONFIG_STA_SUPPORT) && (defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT))

	if (INFRA_ON(pStaCfg) && pMacEntry &&
		(pMacEntry->wcid == pMacEntry->wcid) &&
		(pStaCfg->PwrMgmt.bBeaconLost) &&
		pStaCfg->PwrMgmt.bDoze) {
		pStaCfg->ChannelQuality = 0;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::MT7636 BEACON lost meet\n", __func__));
	} else
#endif /* CONFIG_STA_SUPPORT && (STA_LP_PHASE_1_SUPPORT || STA_LP_PHASE_2_SUPPORT) */
	{
		/* decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER*/
		{
			/* Normalize Rssi*/
			if (MaxRssi > -40)
				NorRssi = 100;
			else if (MaxRssi < -90)
				NorRssi = 0;
			else
				NorRssi = (MaxRssi + 90) * 2;

			/* ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER	 (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)*/
			ChannelQuality = (RSSI_WEIGHTING * NorRssi +
							  TX_WEIGHTING * (100 - TxPRR) +
							  RX_WEIGHTING * (100 - RxPER)) / 100;
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d), ChannelQuality(%lu)\n", __func__, __LINE__,
					 ChannelQuality));
		}

	}

#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		if (pMacEntry != NULL)
			pMacEntry->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
	}

#endif /* CONFIG_AP_SUPPORT */
}

VOID MlmeSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	if (TxPreamble == Rt802_11PreambleLong)
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	else {
		/* NOTE: 1Mbps should always use long preamble*/
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeSetTxPreamble = %s PREAMBLE\n",
			 ((TxPreamble == Rt802_11PreambleLong) ? "LONG" : "SHORT")));
}


/*
    ==========================================================================
    Description:
	Update basic rate bitmap
    ==========================================================================
*/
VOID UpdateBasicRateBitmap(RTMP_ADAPTER *pAdapter, struct wifi_dev *wdev)
{
	INT  i, j;
	/* 1  2  5.5, 11,  6,  9, 12, 18, 24, 36, 48,  54 */
	UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
	UCHAR *sup_p = wdev->rate.SupRate;
	UCHAR *ext_p = wdev->rate.ExtRate;
	ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;

	/* if A mode, always use fix BasicRateBitMap */
	/*if (wdev->channel)*/
	if (wdev->channel > 14) {
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF) {
			/* no 11b rate in 5G band */
			pAdapter->CommonCfg.BasicRateBitmapOld = \
					pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in a-band */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
	}
	else {
		/* no need to modify in 2.4G (bg mixed) */
		pAdapter->CommonCfg.BasicRateBitmap = \
											  pAdapter->CommonCfg.BasicRateBitmapOld;
	}

	if (pAdapter->CommonCfg.BasicRateBitmap > 4095) {
		/* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
		return;
	}

	bitmap = pAdapter->CommonCfg.BasicRateBitmap;  /* renew bitmap value */

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		sup_p[i] &= 0x7f;
		ext_p[i] &= 0x7f;
	}

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (bitmap & (1 << i)) {
			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (sup_p[j] == rate[i])
					sup_p[j] |= 0x80;
			}

			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (ext_p[j] == rate[i])
					ext_p[j] |= 0x80;
			}
		}
	}
}

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRates(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, UCHAR apidx)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;
	struct wifi_dev *wdev = NULL;
	wdev = get_wdev_by_idx(pAd, apidx);

	if (!wdev)
		return;

	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd, wdev);
	num = 0;
	auto_rate_cur_p = NULL;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		switch (wdev->rate.DesireRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;
			num++;
			break;

		case 4:
			Rate = RATE_2;
			num++;
			break;

		case 11:
			Rate = RATE_5_5;
			num++;
			break;

		case 22:
			Rate = RATE_11;
			num++;
			break;

		case 12:
			Rate = RATE_6;
			num++;
			break;

		case 18:
			Rate = RATE_9;
			num++;
			break;

		case 24:
			Rate = RATE_12;
			num++;
			break;

		case 36:
			Rate = RATE_18;
			num++;
			break;

		case 48:
			Rate = RATE_24;
			num++;
			break;

		case 72:
			Rate = RATE_36;
			num++;
			break;

		case 96:
			Rate = RATE_48;
			num++;
			break;

		case 108:
			Rate = RATE_54;
			num++;
			break;
			/*default: Rate = RATE_1;   break;*/
		}

		if (MaxDesire < Rate)
			MaxDesire = Rate;
	}

	/*===========================================================================*/
	do {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM) {
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				break;
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid idx(%d)\n", __func__, idx));
				return;
			}
		}

#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT

			if (apidx >= MIN_NET_DEVICE_FOR_WDS) {
				UCHAR idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY) {
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				} else {
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __func__, apidx));
					return;
				}
			}

#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) &&
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __func__, apidx));

			break;
		}
#endif /* CONFIG_AP_SUPPORT */
	} while (FALSE);

	if (wdev) {
		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;
		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
		HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	}

	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;

	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	{
		pSupRate = &wdev->rate.SupRate[0];
		pExtRate = &wdev->rate.ExtRate[0];
		SupRateLen = wdev->rate.SupRateLen;
		ExtRateLen = wdev->rate.ExtRateLen;
	}

	/* find max supported rate */
	for (i = 0; i < SupRateLen; i++) {
		switch (pSupRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i = 0; i < ExtRateLen; i++) {
		switch (pExtRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}


	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;

		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
			 __func__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));

	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;

	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p) {
		short dbm = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		dbm = 0;
#endif /* CONFIG_AP_SUPPORT */

		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;
	} else {
		wdev->rate.TxRate = wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6) {
			if (HtMcs <= MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		} else {
			if (HtMcs <= MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}

	}

	if (wdev->rate.TxRate <= RATE_11) {
		pMaxHtPhy->field.MODE = MODE_CCK;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
		pMaxHtPhy->field.MODE = MODE_OFDM;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	pHtPhy->word = (pMaxHtPhy->word);

	if (bLinkUp && (pAd->OpMode == OPMODE_STA)) {
	} else {
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			wdev->channel <= 14) {
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		} else {
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;

		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;

		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
			HTTRANSMIT_SETTING tPhyMode, *pTransmit;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
			pTransmit  = (wdev->channel > 14) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);

			if (memcmp(pTransmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0) {
				memmove(pTransmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
						sizeof(HTTRANSMIT_SETTING));
			}

			/*
			printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __func__,
				(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
				pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
			*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
			  __func__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
			  RateIdToMbps[wdev->rate.MaxTxRate],
			  RateIdToMbps[wdev->rate.MinTxRate],
			  /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
			 __func__, RateIdToMbps[wdev->rate.TxRate],
			 RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
}


/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRatesWdev(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, struct wifi_dev *wdev)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;

	if (!wdev)
		return;

	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd, wdev);
	num = 0;
	auto_rate_cur_p = NULL;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		switch (wdev->rate.DesireRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;
			num++;
			break;

		case 4:
			Rate = RATE_2;
			num++;
			break;

		case 11:
			Rate = RATE_5_5;
			num++;
			break;

		case 22:
			Rate = RATE_11;
			num++;
			break;

		case 12:
			Rate = RATE_6;
			num++;
			break;

		case 18:
			Rate = RATE_9;
			num++;
			break;

		case 24:
			Rate = RATE_12;
			num++;
			break;

		case 36:
			Rate = RATE_18;
			num++;
			break;

		case 48:
			Rate = RATE_24;
			num++;
			break;

		case 72:
			Rate = RATE_36;
			num++;
			break;

		case 96:
			Rate = RATE_48;
			num++;
			break;

		case 108:
			Rate = RATE_54;
			num++;
			break;
			/*default: Rate = RATE_1;   break;*/
		}

		if (MaxDesire < Rate)
			MaxDesire = Rate;
	}

	pHtPhy = &wdev->HTPhyMode;
	pMaxHtPhy = &wdev->MaxHTPhyMode;
	pMinHtPhy = &wdev->MinHTPhyMode;
	auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;

	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	{
		pSupRate = &wdev->rate.SupRate[0];
		pExtRate = &wdev->rate.ExtRate[0];
		SupRateLen = wdev->rate.SupRateLen;
		ExtRateLen = wdev->rate.ExtRateLen;
	}

	/* find max supported rate */
	for (i = 0; i < SupRateLen; i++) {
		switch (pSupRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i = 0; i < ExtRateLen; i++) {
		switch (pExtRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}


	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;

		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
			 __func__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));

	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;

	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p) {
		short dbm = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		dbm = 0;
#endif /* CONFIG_AP_SUPPORT */

		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;
	} else {
		wdev->rate.TxRate = wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6) {
			if (HtMcs <= MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		} else {
			if (HtMcs <= MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}

	}

	if (wdev->rate.TxRate <= RATE_11) {
		pMaxHtPhy->field.MODE = MODE_CCK;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
		pMaxHtPhy->field.MODE = MODE_OFDM;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	pHtPhy->word = (pMaxHtPhy->word);

	if (bLinkUp && (pAd->OpMode == OPMODE_STA)) {
	} else {
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			wdev->channel <= 14) {
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		} else {
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;

		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;

		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
			HTTRANSMIT_SETTING tPhyMode, *pTransmit;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
			pTransmit  = (wdev->channel > 14) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);

			if (memcmp(pTransmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0) {
				memmove(pTransmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
						sizeof(HTTRANSMIT_SETTING));
			}

			/*
			printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __func__,
					(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
					pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
			*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
			  __func__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
			  RateIdToMbps[wdev->rate.MaxTxRate],
			  RateIdToMbps[wdev->rate.MinTxRate],
			  /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
			 __func__, RateIdToMbps[wdev->rate.TxRate],
			 RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
}


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		This function update HT Rate setting.
		Input Wcid value is valid for 2 case :
		1. it's used for Station in infra mode that copy AP rate to Mactable.
		2. OR Station	in adhoc mode to copy peer's HT rate to Mactable.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, UCHAR apidx)
{
	UCHAR StbcMcs;
	HT_CAPABILITY_IE *curr_ht_cap;
	RT_PHY_INFO *pActiveHtPhy = NULL;
	ULONG BasicMCS;
	RT_PHY_INFO *pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING pHtPhy = NULL;
	PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	struct wifi_dev *wdev = NULL;
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;
	UCHAR gf, stbc;
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()===>\n", __func__));
	auto_rate_cur_p = NULL;
	wdev = get_wdev_by_idx(pAd, apidx);

	if (!wdev) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __func__, apidx));
		return;
	}

	pDesireHtPhy = &wdev->DesiredHtPhyInfo;
	pActiveHtPhy = &wdev->DesiredHtPhyInfo;
	pHtPhy = &wdev->HTPhyMode;
	pMaxHtPhy = &wdev->MaxHTPhyMode;
	pMinHtPhy = &wdev->MinHTPhyMode;
	auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	addht = wlan_operate_get_addht(wdev);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	{
		if ((!pDesireHtPhy) || pDesireHtPhy->bHtEnable == FALSE)
			return;

		curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
		stbc = curr_ht_cap->HtCapInfo.RxSTBC;
		gf = curr_ht_cap->HtCapInfo.GF;
		StbcMcs = (UCHAR) addht->AddHtInfo3.StbcMcs;
		BasicMCS = addht->MCSSet[0] + (addht->MCSSet[1] << 8) + (StbcMcs << 16);

		if ((curr_ht_cap->HtCapInfo.TxSTBC) && (pAd->Antenna.field.TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	/* Decide MAX ht rate.*/
	if (gf)
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

	if (cfg_ht_bw)
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

	if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = curr_ht_cap->HtCapInfo.ShortGIfor20;
	else
		pMaxHtPhy->field.ShortGI = curr_ht_cap->HtCapInfo.ShortGIfor40;

	if (pDesireHtPhy->MCSSet[4] != 0)
		pMaxHtPhy->field.MCS = 32;

	pMaxHtPhy->field.MCS = get_ht_max_mcs(pAd, &pDesireHtPhy->MCSSet[0],
										  &pActiveHtPhy->MCSSet[0]);
	/* Copy MIN ht rate.  rt2860???*/
	pMinHtPhy->field.BW = BW_20;
	pMinHtPhy->field.MCS = 0;
	pMinHtPhy->field.STBC = 0;
	pMinHtPhy->field.ShortGI = 0;
	/*If STA assigns fixed rate. update to fixed here.*/
	/* Decide ht rate*/
	pHtPhy->field.STBC = pMaxHtPhy->field.STBC;
	pHtPhy->field.BW = pMaxHtPhy->field.BW;
	pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
	pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
	pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;

	/* use default now. rt2860*/
	if (pDesireHtPhy->MCSSet[0] != 0xff)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesireHtPhy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, wdev, pDesireHtPhy);

		if (pDesireHtPhy->bVhtEnable == TRUE) {
			PHTTRANSMIT_SETTING pHtPhy = NULL;
			PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
			PHTTRANSMIT_SETTING pMinHtPhy = NULL;

			pHtPhy = &wdev->HTPhyMode;
			pMaxHtPhy = &wdev->MaxHTPhyMode;
			pMinHtPhy = &wdev->MinHTPhyMode;
			pMaxHtPhy->field.MODE = MODE_VHT;

			if (pDesireHtPhy->vht_bw == VHT_BW_2040) {/* use HT BW setting */
				if (pHtPhy->field.BW == BW_20)
					pMaxHtPhy->field.MCS = 8;
				else
					pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_80) {
				pMaxHtPhy->field.BW = BW_80;
				pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_160) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_8080) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			}

			pMaxHtPhy->field.ShortGI = wlan_config_get_vht_sgi(wdev);
			/* Decide ht rate*/
			pHtPhy->field.BW = pMaxHtPhy->field.BW;
			pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
			pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
			pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;
		}
	}

#endif /* DOT11_VHT_AC */
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s():<---.AMsduSize = %d\n", __func__,
			 curr_ht_cap->HtCapInfo.AMsduSize));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,\n", pActiveHtPhy->MCSSet[0], pHtPhy->field.MCS,
			  pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():<===\n", __func__));
}




VOID BATableInit(RTMP_ADAPTER *pAd, BA_TABLE *Tab)
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	Tab->numDoneOriginator = 0;
	NdisAllocateSpinLock(pAd, &pAd->BATabLock);
	pAd->BATable.ba_timeout_check = FALSE;
	os_zero_mem((UCHAR *)&pAd->BATable.ba_timeout_bitmap[0], sizeof(UINT32) * BA_TIMEOUT_BITMAP_LEN);

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(pAd, &(Tab->BARecEntry[i].RxReRingLock));
	}

	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++)
		Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
}


VOID BATableExit(RTMP_ADAPTER *pAd)
{
	int i;

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
		NdisFreeSpinLock(&pAd->BATable.BARecEntry[i].RxReRingLock);

	NdisFreeSpinLock(&pAd->BATabLock);
}
#endif /* DOT11_N_SUPPORT */

VOID MlmeRadioOff(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
	wdev->fgRadioOnRequest = FALSE;
	MTRadioOff(pAd, wdev);
}

VOID MlmeRadioOn(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	wdev->fgRadioOnRequest = TRUE;
	MTRadioOn(pAd, wdev);
	RTMP_OS_NETDEV_START_QUEUE(wdev->if_dev);
#ifdef NF_SUPPORT
	if (IS_MT7615(pAd))
		enable_nf_support(pAd);
#endif
}

VOID MlmeLpEnter(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return;
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		if (IS_MT7622(pAd)) {
			MlmeRadioOff(pAd, wdev);
		} else
			MTMlmeLpEnter(pAd, wdev);
	}

#endif /* RTMP_MAC_PCI */
}


VOID MlmeLpExit(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[/*MAIN_MBSSID*/pObj->ioctl_if].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		if (IS_MT7622(pAd))
			MlmeRadioOn(pAd, wdev);
		else
			MTMlmeLpExit(pAd, wdev);
	}

#endif /* RTMP_MAC_PCI */
}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(RTMP_ADAPTER *pAd)
{
	UCHAR i;

	for (i = 0; i < MAX_TRIGGER_EVENT; i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;

	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}


INT TriEventTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB * Tab,
	IN UCHAR *pBssid,
	IN HT_CAPABILITY_IE * pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN UCHAR RegClass,
	IN UCHAR ChannelNo)
{
	/* Event A, legacy AP exist.*/
	if (HtCapabilityLen == 0) {
		UCHAR index;

		/*
			Check if we already set this entry in the Event Table.
		*/
		for (index = 0; index < MAX_TRIGGER_EVENT; index++) {
			if ((Tab->EventA[index].bValid == TRUE) &&
				(Tab->EventA[index].Channel == ChannelNo) &&
				(Tab->EventA[index].RegClass == RegClass)
			   ) {
				return 0;
			}
		}

		/*
			If not set, add it to the Event table
		*/
		if (Tab->EventANo < MAX_TRIGGER_EVENT) {
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;

			if (RegClass != 0) {
				/* Beacon has Regulatory class IE. So use beacon's*/
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			} else {
				/* Use Station's Regulatory class instead.*/
				/* If no Reg Class in Beacon, set to "unknown"*/
				/* TODO:  Need to check if this's valid*/
				Tab->EventA[Tab->EventANo].RegClass = 0; /* ????????????????? need to check*/
			}

			Tab->EventANo++;
		}
	}

	else if (pHtCapability->HtCapInfo.Forty_Mhz_Intolerant) {
		/* Event B.   My BSS beacon has Intolerant40 bit set*/
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}

	return 0;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */

/*! \brief generates a random mac address value for IBSS BSSID
 *	\param Addr the bssid location
 *	\return none
 *	\pre
 *	\post
 */
VOID MacAddrRandomBssid(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;

	for (i = 0; i < MAC_ADDR_LEN; i++)
		pAddr[i] = RandomByte(pAd);

	pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  /* the first 2 bits must be 01xxxxxxxx*/
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT * AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID DisassocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT * DisassocReq,
	IN PUCHAR pAddr,
	IN USHORT Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}


/*! \brief init the management mac frame header
 *	\param p_hdr mac header
 *	\param subtype subtype of the frame
 *	\param p_ds destination address, don't care if it is a broadcast address
 *	\return none
 *	\pre the station has the following information in the pAd->StaCfg[0]
 *	 - bssid
 *	 - station address
 *	\post
 *	\note this function initializes the following field
 */
VOID MgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 * pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	COPY_MAC_ADDR(pHdr80211->Addr2, pSA);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
#endif /* CONFIG_AP_SUPPORT */
#endif /* P2P_SUPPORT */
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


VOID MgtMacHeaderInitExt(
	IN RTMP_ADAPTER *pAd,
	IN OUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pAddr1,
	IN UCHAR *pAddr2,
	IN UCHAR *pAddr3)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pAddr1);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAddr2);
	COPY_MAC_ADDR(pHdr80211->Addr3, pAddr3);
}


/*!***************************************************************************
 * This routine build an outgoing frame, and fill all information specified
 * in argument list to the frame body. The actual frame size is the summation
 * of all arguments.
 * input params:
 *		Buffer - pointer to a pre-allocated memory segment
 *		args - a list of <int arg_size, arg> pairs.
 *		NOTE NOTE NOTE!!!! the last argument must be NULL, otherwise this
 *						   function will FAIL!!!
 * return:
 *		Size of the buffer
 * usage:
 *		MakeOutgoingFrame(Buffer, output_length, 2, &fc, 2, &dur, 6, p_addr1, 6,p_addr2, END_OF_ARGS);

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 ****************************************************************************/
ULONG MakeOutgoingFrame(UCHAR *Buffer, ULONG *FrameLen, ...)
{
	UCHAR   *p;
	int	leng;
	ULONG	TotLeng;
	va_list Args;
	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);

	do {
		leng = va_arg(Args, int);

		if (leng == END_OF_ARGS)
			break;

		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while (TRUE);

	va_end(Args); /* clean up */
	*FrameLen = TotLeng;
	return TotLeng;
}


/*! \brief	Initialize The MLME Queue, used by MLME Functions
 *	\param	*Queue	   The MLME Queue
 *	\return Always	   Return NDIS_STATE_SUCCESS in this implementation
 *	\pre
 *	\post
 *	\note	Because this is done only once (at the init stage), no need to be locked

 IRQL = PASSIVE_LEVEL

 */
NDIS_STATUS MlmeQueueInit(RTMP_ADAPTER *pAd, MLME_QUEUE *Queue)
{
	INT i;

	NdisAllocateSpinLock(pAd, &Queue->Lock);
	Queue->Num	= 0;
	Queue->Head = 0;
	Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++) {
		Queue->Entry[i].Occupied = FALSE;
		Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(Queue->Entry[i].Msg, MAX_MGMT_PKT_LEN);
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT

/*! \brief	 Enqueue a message for other threads, if they want to send messages to MLME thread
 *	\param	*Queue	  The MLME Queue
 *	\param	 Machine  The State Machine Id
 *	\param	 MsgType  The Message Type
 *	\param	 MsgLen   The Message length
 *	\param	*Msg	  The message pointer
 *	\return  TRUE if enqueue is successful, FALSE if the queue is full
 *	\pre
 *	\post
 *	\note	 The message has to be initialized
 */
BOOLEAN MlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv)
{
	INT Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

#endif

BOOLEAN MlmeEnqueueWithWdev(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv,
	IN struct wifi_dev *wdev)
{
	INT Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	ASSERT(wdev);

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/*check wdev is not ready*/
	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev(%d) state: if down!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


/*! \brief	 This function is used when Recv gets a MLME message
 *	\param	*Queue			 The MLME Queue
 *	\param	 TimeStampHigh	 The upper 32 bit of timestamp
 *	\param	 TimeStampLow	 The lower 32 bit of timestamp
 *	\param	 Rssi			 The receiving RSSI strength
 *	\param	 MsgLen		 The length of the message
 *	\param	*Msg			 The message pointer
 *	\return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *	\pre
 *	\post
 */
BOOLEAN MlmeEnqueueForRecv(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Wcid,
	IN struct raw_rssi_info *rssi_info,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN UCHAR OpMode,
	IN struct wifi_dev *wdev,
	IN UCHAR RxPhyMode)
{
	INT Tail, Machine = 0xff;
	UINT32 TimeStampHigh, TimeStampLow;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT MsgType = 0x0;
	MLME_QUEUE *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	BOOLEAN bToApCli = FALSE;
	UCHAR ApCliIdx = 0;
	BOOLEAN ApCliIdx_find = FALSE;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */
#endif
#if defined(CONFIG_AP_SUPPORT) && defined(MBSS_SUPPORT)
#ifdef APCLI_SUPPORT
	UCHAR MBSSIdx = 0;
	BOOLEAN ApBssIdx_find = FALSE;
#endif /* APCLI_SUPPORT */
#endif
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return FALSE;

#endif /* CONFIG_ATE */

	/*
		Do nothing if the driver is starting halt state.
		This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __func__));
		return FALSE;
	}
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():drop frame,wdev is Null\n", __func__));
		return FALSE;
	}


	/*check if wdev is ready except for BMC packet, because it doesn't need wdev */
	if (!wdev->if_up_down_state && Wcid != RESERVED_WCID) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): wdev (%d) not ready\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame too large, size = %ld\n", __func__, MsgLen));
		return FALSE;
	}

	if (Msg == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame is Null\n", __func__));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 0)) {
		RTMP_MLME_HANDLER(pAd);
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)

	if (OpMode == OPMODE_AP)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
#ifdef APCLI_SUPPORT

		/*
			Beacon must be handled by ap-sync state machine.
			Probe-rsp must be handled by apcli-sync state machine.
			Those packets don't need to check its MAC address
		*/
		do {
			UCHAR i;

			/*
			   1. When P2P GO On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: when GO On skip preCheckMsgTypeSubset redirect to APMsgTypeSubst
			   2. When P2P Cli On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: handle MsgType == APCLI_MT2_PEER_PROBE_RSP on ApCli Sync state machine
					when ApCli on idle state.
			*/
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].MlmeAux.Bssid, pFrame->Hdr.Addr2)) {
					/* APCLI_CONNECTION_TRIAL don't need to seperate the ApCliIdx, otherwise the ApCliIdx will be wrong on apcli DBDC mode. */
					ApCliIdx = i;
					bToApCli = TRUE;
					break;
				}
			}

			/* check if da is to apcli */
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].wdev.if_addr, pFrame->Hdr.Addr1)) {
					ApCliIdx = i;
					bToApCli = TRUE;
					ApCliIdx_find = TRUE;
					break;
				}
			}

			/* check if da is to ap */
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, pFrame->Hdr.Addr1)) {
#if defined(CONFIG_AP_SUPPORT) && defined(MBSS_SUPPORT)
					MBSSIdx = i;
					ApBssIdx_find = TRUE;
#endif
					break;
				}
			}

			if ((ApBssIdx_find == FALSE) &&
				preCheckMsgTypeSubset(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case and apcli case*/
				if (bToApCli == TRUE) {
					if ((Machine == AP_SYNC_STATE_MACHINE) &&
						(MsgType == APMT2_PEER_BEACON)) {
						ULONG Now32;

						NdisGetSystemUpTime(&Now32);
						pAd->ApCfg.ApCliTab[ApCliIdx].ApCliRcvBeaconTime_MlmeEnqueueForRecv = Now32;
						pAd->ApCfg.ApCliTab[ApCliIdx].ApCliRcvBeaconTime_MlmeEnqueueForRecv_2 = pAd->Mlme.Now32;
					}
				}

				break;
			}

			if ((ApBssIdx_find == FALSE) &&
				bToApCli) {
				if (ApCliMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
					/* apcli and repeater case */
					break;
				}
			}

#ifdef WDS_SUPPORT
			if (WdsMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
				/* wds case */
				break;
			}
#endif

			if (APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case */
				break;
			}

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): un-recongnized mgmt->subtype=%d, STA-%02x:%02x:%02x:%02x:%02x:%02x\n",
					  __func__, pFrame->Hdr.FC.SubType, PRINT_MAC(pFrame->Hdr.Addr2)));
			return FALSE;
		} while (FALSE);

#else

		if (!APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): un-recongnized mgmt->subtype=%d\n",
					 __func__, pFrame->Hdr.FC.SubType));
			return FALSE;
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
	TimeStampHigh = TimeStampLow = 0;
#ifdef RTMP_MAC_PCI

	if ((!IS_USB_INF(pAd)) && !IS_HIF_TYPE(pAd, HIF_MT))
		AsicGetTsfTime(pAd, &TimeStampHigh, &TimeStampLow, HW_BSSID_0);

#endif /* RTMP_MAC_PCI */
	/* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
	Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
	NdisMoveMemory(&Queue->Entry[Tail].rssi_info, rssi_info, sizeof(struct raw_rssi_info));
	Queue->Entry[Tail].Signal = rssi_info->raw_snr;
	Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
	Queue->Entry[Tail].OpMode = (ULONG)OpMode;
	Queue->Entry[Tail].Channel = (rssi_info->Channel == 0) ? pAd->LatchRfRegs.Channel : rssi_info->Channel;
	Queue->Entry[Tail].Priv = 0;
	Queue->Entry[Tail].RxPhyMode = RxPhyMode;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

	if ((pAd->ApCfg.bMACRepeaterEn) && (bToApCli == TRUE)) {
		NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

		for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

			if (MAC_ADDR_EQUAL(pReptEntry->CurrentAddress, pFrame->Hdr.Addr1)) {
				Queue->Entry[Tail].Priv = (REPT_MLME_START_IDX + CliIdx);
				break;
			}
		}

		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	}

#endif /* MAC_REPEATER_SUPPORT */
#endif /*CONFIG_AP_SUPPORT*/
	Queue->Entry[Tail].wdev = wdev;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}


#ifdef WSC_INCLUDED
/*! \brief   Enqueue a message for other threads, if they want to send messages to MLME thread
 *  \param  *Queue    The MLME Queue
 *  \param   TimeStampLow    The lower 32 bit of timestamp, here we used for eventID.
 *  \param   Machine  The State Machine Id
 *  \param   MsgType  The Message Type
 *  \param   MsgLen   The Message length
 *  \param  *Msg      The message pointer
 *  \return  TRUE if enqueue is successful, FALSE if the queue is full
 *  \pre
 *  \post
 *  \note    The message has to be initialized
 */
BOOLEAN MlmeEnqueueForWsc(
	IN RTMP_ADAPTER *pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN struct wifi_dev *wdev)
{
	INT Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----> MlmeEnqueueForWsc\n"));

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return FALSE;

	/*check wdev is not ready*/
	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev(%d) state: if down!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueueForWsc: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	/* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = eventID;
	Queue->Entry[Tail].TimeStamp.u.HighPart = senderID;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	Queue->Entry[Tail].wdev = wdev;
	NdisReleaseSpinLock(&(Queue->Lock));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- MlmeEnqueueForWsc\n"));
	return TRUE;
}
#endif /* WSC_INCLUDED */


/*! \brief	 Dequeue a message from the MLME Queue
 *	\param	*Queue	  The MLME Queue
 *	\param	*Elem	  The message dequeued from MLME Queue
 *	\return  TRUE if the Elem contains something, FALSE otherwise
 *	\pre
 *	\post
 */
BOOLEAN MlmeDequeue(MLME_QUEUE *Queue, MLME_QUEUE_ELEM **Elem)
{
	NdisAcquireSpinLock(&(Queue->Lock));
	*Elem = &(Queue->Entry[Queue->Head]);
	Queue->Num--;
	Queue->Head++;

	if (Queue->Head == MAX_LEN_OF_MLME_QUEUE)
		Queue->Head = 0;

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


VOID MlmeRestartStateMachine(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM *Elem = NULL;
#endif /* RTMP_MAC_PCI */
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeRestartStateMachine\n"));
#ifdef RTMP_MAC_PCI
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);

	if (pAd->Mlme.bRunning) {
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} else
		pAd->Mlme.bRunning = TRUE;

	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	/* Remove all Mlme queues elements*/
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) {
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) {
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeRestartStateMachine: MlmeQueue empty\n"));
	}

#endif /* RTMP_MAC_PCI */
	/* Change back to original channel in case of doing scan*/
	{
		hc_reset_radio(pAd);
	}

#ifdef RTMP_MAC_PCI
	/* Remove running state*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif /* RTMP_MAC_PCI */
	/* CFG_TODO for SCAN */
#ifdef RT_CFG80211_SUPPORT
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* RT_CFG80211_SUPPORT */
}


/*! \brief	test if the MLME Queue is empty
 *	\param	*Queue	  The MLME Queue
 *	\return TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueEmpty(MLME_QUEUE *Queue)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));
	return Ans;
}

/*! \brief	 test if the MLME Queue is full
 *	\param	 *Queue	 The MLME Queue
 *	\return  TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueFull(MLME_QUEUE *Queue, UCHAR SendId)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));

	if (SendId == 0)
		Ans = ((Queue->Num >= (MAX_LEN_OF_MLME_QUEUE / 2)) || Queue->Entry[Queue->Tail].Occupied);
	else
		Ans = ((Queue->Num == MAX_LEN_OF_MLME_QUEUE) || Queue->Entry[Queue->Tail].Occupied);

	NdisReleaseSpinLock(&(Queue->Lock));
	return Ans;
}


/*! \brief	 The destructor of MLME Queue
 *	\param
 *	\return
 *	\pre
 *	\post
 *	\note	Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL

 */
VOID MlmeQueueDestroy(MLME_QUEUE *pQueue)
{
	NdisAcquireSpinLock(&(pQueue->Lock));
	pQueue->Num  = 0;
	pQueue->Head = 0;
	pQueue->Tail = 0;
	NdisReleaseSpinLock(&(pQueue->Lock));
	NdisFreeSpinLock(&(pQueue->Lock));
}


/*! \brief	 To substitute the message type if the message is coming from external
 *	\param	pFrame		   The frame received
 *	\param	*Machine	   The state machine
 *	\param	*MsgType	   the message type for the state machine
 *	\return TRUE if the substitution is successful, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */


/*! \brief Initialize the state machine.
 *	\param *S			pointer to the state machine
 *	\param	Trans		State machine transition function
 *	\param	StNr		number of states
 *	\param	MsgNr		number of messages
 *	\param	DefFunc	default function, when there is invalid state/message combination
 *	\param	InitState	initial state of the state machine
 *	\param	Base		StateMachine base, internal use only
 *	\pre p_sm should be a legal pointer
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineInit(
	IN STATE_MACHINE * S,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base)
{
	ULONG i, j;
	/* set number of states and messages*/
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;
	S->TransFunc  = Trans;

	/* init all state transition to default function*/
	for (i = 0; i < StNr; i++) {
		for (j = 0; j < MsgNr; j++)
			S->TransFunc[i * MsgNr + j] = DefFunc;
	}

	/* set the starting state*/
	S->CurrState = InitState;
}


/*! \brief This function fills in the function pointer into the cell in the state machine
 *	\param *S	pointer to the state machine
 *	\param St	state
 *	\param Msg	incoming message
 *	\param f	the function to be executed when (state, message) combination occurs at the state machine
 *	\pre *S should be a legal pointer to the state machine, st, msg, should be all within the range, Base should be set in the initial state
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineSetAction(
	IN STATE_MACHINE * S,
	IN ULONG St,
	IN ULONG Msg,
	IN STATE_MACHINE_FUNC Func)
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg) {
		/* boundary checking before setting the action*/
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	}
}


/*! \brief	 This function does the state transition
 *	\param	 *Adapter the NIC adapter pointer
 *	\param	 *S	  the state machine
 *	\param	 *Elem	  the message to be executed
 *	\return   None
 */
VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd,
	IN STATE_MACHINE * S,
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState)
{
	if (S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base])
		(*(S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}


/*
	==========================================================================
	Description:
		The drop function, when machine executes this, the message is simply
		ignored. This function does nothing, the message is freed in
		StateMachinePerformAction()
	==========================================================================
 */
VOID Drop(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}


/*
	==========================================================================
	Description:
	==========================================================================
 */
UCHAR RandomByte(RTMP_ADAPTER *pAd)
{
	ULONG i;
	UCHAR R, Result;

	R = 0;

	if (pAd->Mlme.ShiftReg == 0)
		NdisGetSystemUpTime((ULONG *)&pAd->Mlme.ShiftReg);

	for (i = 0; i < 8; i++) {
		if (pAd->Mlme.ShiftReg & 0x00000001) {
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		} else {
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}

		R = (R << 1) | Result;
	}

	return R;
}


UCHAR RandomByte2(RTMP_ADAPTER *pAd)
{
	UINT32 a, b;
	UCHAR value, seed = 0;

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return 0;
	}

	/*MAC statistic related*/
	a = AsicGetCCACnt(pAd);
	a &= 0x0000ffff;
	b = AsicGetCrcErrCnt(pAd);
	b &= 0x0000ffff;
	value = (a << 16) | b;
	/*get seed by RSSI or SNR related info */
	seed = get_random_seed_by_phy(pAd);
	return value ^ seed ^ RandomByte(pAd);
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPCheckRates(RTMP_ADAPTER *pAd, UCHAR SupRate[], UCHAR *SupRateLen, UCHAR PhyMode)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12], NewRateLen;

	NdisZeroMemory(NewRate, sizeof(NewRate));
	NewRateLen = 0;

	if (WMODE_EQUAL(PhyMode, WMODE_B))
		RateIdx = 4;
	else
		RateIdx = 12;

	/* Check for support rates exclude basic rate bit */
	for (i = 0; i < *SupRateLen; i++)
		for (j = 0; j < RateIdx; j++)
			if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
				NewRate[NewRateLen++] = SupRate[i];

	*SupRateLen = NewRateLen;
	NdisMoveMemory(SupRate, NewRate, NewRateLen);
}



CHAR RTMPAvgRssi(RTMP_ADAPTER *pAd, RSSI_SAMPLE *pRssi)
{
	CHAR Rssi;
	INT Rssi_temp;
	UINT32	rx_stream;

	rx_stream = pAd->Antenna.field.RxPath;

	if (rx_stream == 4)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2] + pRssi->AvgRssi[3]) >> 2;
	else if (rx_stream == 3)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2]) / 3;
	else if (rx_stream == 2)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1]) >> 1;
	else
		Rssi_temp = pRssi->AvgRssi[0];

	Rssi = (CHAR)Rssi_temp;
	return Rssi;
}


CHAR RTMPMaxRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2)
{
	CHAR	larger = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 != 0))
		larger = Rssi0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 != 0))
		larger = max(Rssi0, Rssi1);

	if ((pAd->Antenna.field.RxPath == 3) && (Rssi2 != 0))
		larger = max(larger, Rssi2);

	if (larger == -127)
		larger = 0;

	return larger;
}

CHAR RTMPMinRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2, CHAR Rssi3)
{
	CHAR	smaller = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 != 0))
		smaller = Rssi0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 != 0))
		smaller = min(Rssi0, Rssi1);

	if ((pAd->Antenna.field.RxPath >= 3) && (Rssi2 != 0))
		smaller = min(smaller, Rssi2);

	if ((pAd->Antenna.field.RxPath == 4) && (Rssi3 != 0))
		smaller = min(smaller, Rssi3);

	if (smaller == -127)
		smaller = 0;

	return smaller;
}

CHAR RTMPMinSnr(RTMP_ADAPTER *pAd, CHAR Snr0, CHAR Snr1)
{
	CHAR	smaller = Snr0;

	if (pAd->Antenna.field.RxPath == 1)
		smaller = Snr0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Snr1 != 0))
		smaller = min(Snr0, Snr1);

	return smaller;
}


/*
    ========================================================================
    Routine Description:
	Periodic evaluate antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID AsicEvaluateRxAnt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
					   fRTMP_ADAPTER_NIC_NOT_EXIST |
					   fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
		|| IsHcAllSupportedBandsRadioOff(pAd)
	   )
		return;

#ifdef MT_MAC

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT))
		return;

#endif /* MT_MAC */
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			/* for SmartBit 64-byte stream test */
			if (pAd->MacTab.Size > 0)
				APAsicEvaluateRxAnt(pAd);

			return;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}


/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER	 *pAd = (RTMP_ADAPTER *)FunctionContext;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
					   fRTMP_ADAPTER_NIC_NOT_EXIST)
		|| IsHcAllSupportedBandsRadioOff(pAd)
	   )
		return;

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			APAsicRxAntEvalTimeout(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}


VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
}


/*
    ========================================================================
    Routine Description:
	check if this entry need to switch rate automatically

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN result = TRUE;

	if ((!pEntry) || (!(pEntry->wdev))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): entry(%p) or wdev(%p) is NULL!\n",
				 __func__, pEntry,
				 pEntry ? pEntry->wdev : NULL));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		result = pEntry->wdev->bAutoTxRateSwitch;
	}
#endif /* CONFIG_AP_SUPPORT */
#if defined(TXBF_SUPPORT) && (!defined(MT_MAC))

	if (result == FALSE) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		/* Force MCS will be fixed */
		if (cap->FlgHwTxBfCap)
			eTxBFProbing(pAd, pEntry);
	}

#endif /* TXBF_SUPPORT */
	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT	apidx = 0;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if (pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#ifdef WDS_SUPPORT

		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++) {
			if (pAd->WdsTab.WdsEntry[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT

		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++) {
			if (pAd->ApCfg.ApCliTab[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	return FALSE;
}


/*
    ========================================================================
    Routine Description:
	check if this entry need to fix tx legacy rate

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
UCHAR RTMPStaFixedTxMode(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UCHAR tx_mode = FIXED_TXMODE_HT;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

	if (pEntry) {
		if (IS_ENTRY_CLIENT(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;

#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			tx_mode = (UCHAR)pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
	return tx_mode;
}


/*
    ========================================================================
    Routine Description:
	Overwrite HT Tx Mode by Fixed Legency Tx Mode, if specified.

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
VOID RTMPUpdateLegacyTxSetting(UCHAR fixed_tx_mode, MAC_TABLE_ENTRY *pEntry)
{
	HTTRANSMIT_SETTING TransmitSetting;

	if ((fixed_tx_mode != FIXED_TXMODE_CCK) &&
		(fixed_tx_mode != FIXED_TXMODE_OFDM)
#ifdef DOT11_VHT_AC
		&& (fixed_tx_mode != FIXED_TXMODE_VHT)
#endif /* DOT11_VHT_AC */
	   )
		return;

	TransmitSetting.word = 0;
	TransmitSetting.field.MODE = pEntry->HTPhyMode.field.MODE;
	TransmitSetting.field.MCS = pEntry->HTPhyMode.field.MCS;
#ifdef DOT11_VHT_AC

	if (fixed_tx_mode == FIXED_TXMODE_VHT) {
		UCHAR nss, mcs;

		TransmitSetting.field.MODE = MODE_VHT;
		TransmitSetting.field.BW = pEntry->MaxHTPhyMode.field.BW;
		nss = (TransmitSetting.field.MCS >> 4) & 0x3;
		mcs = (TransmitSetting.field.MCS & 0xf);

		if ((TransmitSetting.field.BW == BW_20) && (mcs > MCS_8))
			mcs = MCS_8;

		if ((TransmitSetting.field.BW == BW_40) && (mcs > MCS_9))
			mcs = MCS_9;

		if (TransmitSetting.field.BW == BW_80) {
			if (mcs > MCS_9)
				mcs = MCS_9;

			if ((nss == 2) && (mcs == MCS_6))
				mcs = MCS_5;
		}

		TransmitSetting.field.MCS = ((nss << 4) + mcs);
	} else
#endif /* DOT11_VHT_AC */
		if (fixed_tx_mode == FIXED_TXMODE_CCK) {
			TransmitSetting.field.MODE = MODE_CCK;

			/* CCK mode allow MCS 0~3*/
			if (TransmitSetting.field.MCS > MCS_3)
				TransmitSetting.field.MCS = MCS_3;
		} else {
			TransmitSetting.field.MODE = MODE_OFDM;

			/* OFDM mode allow MCS 0~7*/
			if (TransmitSetting.field.MCS > MCS_7)
				TransmitSetting.field.MCS = MCS_7;
		}

	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE) {
		pEntry->HTPhyMode.word = TransmitSetting.word;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d\n",
				 pEntry->wcid, get_phymode_str(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the fixed TxMode is invalid\n", __func__));
}


/*
========================================================================
Routine Description:
	Check if the channel has the property.

Arguments:
	pAd				- WLAN control block pointer
	ChanNum			- channel number
	Property		- channel property, CHANNEL_PASSIVE_SCAN, etc.

Return Value:
	TRUE			- YES
	FALSE			- NO

Note:
========================================================================
*/
BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property)
{
	UINT32 IdChan;
	UCHAR BandIdx = HcGetBandByChannel(pAd, ChanNum);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	/* look for all registered channels */
	for (IdChan = 0; IdChan < pChCtrl->ChListNum; IdChan++) {
		if (pChCtrl->ChList[IdChan].Channel == ChanNum) {
			if ((pChCtrl->ChList[IdChan].Flags & Property) == Property)
				return TRUE;

			break;
		}
	}

	return FALSE;
}

NDIS_STATUS RtmpEnqueueTokenFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState = NDIS_STATUS_FAILURE;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;

	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);

	if (NState == NDIS_STATUS_SUCCESS) {
		UCHAR *qos_p;

		pNullFr = (PHEADER_802_11) pFrame;
		frm_len = sizeof(HEADER_802_11);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							 pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							 pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			pNullFr->Frag = 0x0f;
		}
#endif /* CONFIG_AP_SUPPORT */
		pNullFr->FC.Type = FC_TYPE_DATA;
		qos_p = ((UCHAR *)pNullFr) + frm_len;
		pNullFr->FC.SubType = SUBTYPE_QOS_NULL;
		/* copy QOS control bytes */
		qos_p[0] = OldUP;
		qos_p[1] = PS_RETRIEVE_TOKEN;
		frm_len += 2;
		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);
		NState = MiniportMMRequest(pAd, OldUP | MGMT_USE_QUEUE_FLAG, (PUCHAR)pNullFr, frm_len);
		MlmeFreeMemory(pFrame);
	}

	return NState;
}
