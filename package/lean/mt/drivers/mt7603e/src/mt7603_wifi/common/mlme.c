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

#ifdef DOT11V_WNM_SUPPORT
#include "wnm.h"
#endif /* DOT11V_WNM_SUPPORT */

UCHAR CISCO_OUI[] = {0x00, 0x40, 0x96};
UCHAR RALINK_OUI[]  = {0x00, 0x0c, 0x43};
#if (defined(WH_EZ_SETUP) || defined(MWDS) || defined(STA_FORCE_ROAM_SUPPORT)) || defined(WAPP_SUPPORT)
UCHAR MTK_OUI[]  = {0x00, 0x0c, 0xe7};
#endif
UCHAR WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR WAPI_OUI[] = {0x00, 0x14, 0x72};
UCHAR WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR BROADCOM_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};


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


#ifdef SW_ATF_SUPPORT
static UINT32 badCnt;
static UINT32 goodCnt;
#endif



/* since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.*/
/* otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate*/
ULONG BasicRateMask[12] = {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
							0xfffff01f /* 6 */	 , 0xfffff03f /* 9 */	  , 0xfffff07f /* 12 */ , 0xfffff0ff /* 18 */,
							0xfffff1ff /* 24 */	 , 0xfffff3ff /* 36 */	  , 0xfffff7ff /* 48 */ , 0xffffffff /* 54 */};

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than*/
/*		this value, then it's quaranteed capable of operating in 36 mbps TX rate in*/
/*		clean environment.*/
/*								  TxRate: 1   2   5.5	11	 6	  9    12	18	 24   36   48	54	 72  100*/
CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

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

extern UCHAR	WPA_OUI[];
UCHAR APPLE_OUI[] =    {0x00, 0x17, 0xf2};  /* For IOS immediately connect */
UCHAR SES_OUI[] = {0x00, 0x90, 0x4c};

UCHAR ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};




#ifdef DYNAMIC_VGA_SUPPORT
void periodic_monitor_false_cca_adjust_vga(RTMP_ADAPTER *pAd)
{
	if ((pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable) && (pAd->chipCap.dynamic_vga_support) &&
		OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		UCHAR val1, val2;
		UINT32 bbp_val1, bbp_val2;

		RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val1);
		val1 = ((bbp_val1 & (0x00007f00)) >> 8) & 0x7f;
		RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val2);
		val2 = ((bbp_val2 & (0x00007f00)) >> 8) & 0x7f;

		DBGPRINT(RT_DEBUG_INFO, ("vga_init_0 = %x, vga_init_1 = %x\n",  pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0, pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1));
		DBGPRINT(RT_DEBUG_INFO,
			("one second False CCA=%d, fixed agc_vga_0:0%x, fixed agc_vga_1:0%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, val1, val2));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.lna_vga_ctl.nFalseCCATh) {
			if (val1 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 - 0x10)) {
				val1 -= 0x02;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
#ifdef DFS_SUPPORT
       		pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->CommonCfg.RxStream >= 2) {
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
#ifdef DFS_SUPPORT
				pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->CommonCfg.RxStream >= 2) {
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


VOID set_default_ap_edca_param(RTMP_ADAPTER *pAd)
{
	pAd->CommonCfg.APEdcaParm.bValid = TRUE;
	pAd->CommonCfg.APEdcaParm.Aifsn[0] = 3;
	pAd->CommonCfg.APEdcaParm.Aifsn[1] = 7;
	pAd->CommonCfg.APEdcaParm.Aifsn[2] = 1;
	pAd->CommonCfg.APEdcaParm.Aifsn[3] = 1;

	pAd->CommonCfg.APEdcaParm.Cwmin[0] = 4;
	pAd->CommonCfg.APEdcaParm.Cwmin[1] = 4;
	pAd->CommonCfg.APEdcaParm.Cwmin[2] = 3;
	pAd->CommonCfg.APEdcaParm.Cwmin[3] = 2;

	pAd->CommonCfg.APEdcaParm.Cwmax[0] = 6;
	pAd->CommonCfg.APEdcaParm.Cwmax[1] = 10;
	pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
	pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

	pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
	pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
	pAd->CommonCfg.APEdcaParm.Txop[2]  = 94;	
	pAd->CommonCfg.APEdcaParm.Txop[3]  = 47;	
}


#ifdef CONFIG_AP_SUPPORT
VOID set_default_sta_edca_param(RTMP_ADAPTER *pAd)
{
	pAd->ApCfg.BssEdcaParm.bValid = TRUE;
	pAd->ApCfg.BssEdcaParm.Aifsn[0] = 3;
	pAd->ApCfg.BssEdcaParm.Aifsn[1] = 7;
	pAd->ApCfg.BssEdcaParm.Aifsn[2] = 2;
	pAd->ApCfg.BssEdcaParm.Aifsn[3] = 2;

	pAd->ApCfg.BssEdcaParm.Cwmin[0] = 4;
	pAd->ApCfg.BssEdcaParm.Cwmin[1] = 4;
	pAd->ApCfg.BssEdcaParm.Cwmin[2] = 3;
	pAd->ApCfg.BssEdcaParm.Cwmin[3] = 2;

	pAd->ApCfg.BssEdcaParm.Cwmax[0] = 10;
	pAd->ApCfg.BssEdcaParm.Cwmax[1] = 10;
	pAd->ApCfg.BssEdcaParm.Cwmax[2] = 4;
	pAd->ApCfg.BssEdcaParm.Cwmax[3] = 3;

	pAd->ApCfg.BssEdcaParm.Txop[0]  = 0;
	pAd->ApCfg.BssEdcaParm.Txop[1]  = 0;
	pAd->ApCfg.BssEdcaParm.Txop[2]  = 94;	/*96; */
	pAd->ApCfg.BssEdcaParm.Txop[3]  = 47;	/*48; */
}
#endif /* CONFIG_AP_SUPPORT */


UCHAR dot11_max_sup_rate(INT SupRateLen, UCHAR *SupRate, INT ExtRateLen, UCHAR *ExtRate)
{
	INT idx;
	UCHAR MaxSupportedRateIn500Kbps = 0;
	
	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (idx = 0; idx < SupRateLen; idx++) {
		if (MaxSupportedRateIn500Kbps < (SupRate[idx] & 0x7f))
			MaxSupportedRateIn500Kbps = SupRate[idx] & 0x7f;
	}

	if (ExtRateLen > 0 && ExtRate != NULL)
	{
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
	
	switch (MaxSupportedRateIn500Kbps)
	{
		case 108: MaxSupportedRate = RATE_54;   break;
		case 96:  MaxSupportedRate = RATE_48;   break;
		case 72:  MaxSupportedRate = RATE_36;   break;
		case 48:  MaxSupportedRate = RATE_24;   break;
		case 36:  MaxSupportedRate = RATE_18;   break;
		case 24:  MaxSupportedRate = RATE_12;   break;
		case 18:  MaxSupportedRate = RATE_9;    break;
		case 12:  MaxSupportedRate = RATE_6;    break;
		case 22:  MaxSupportedRate = RATE_11;   break;
		case 11:  MaxSupportedRate = RATE_5_5;  break;
		case 4:   MaxSupportedRate = RATE_2;    break;
		case 2:   MaxSupportedRate = RATE_1;    break;
		default:  MaxSupportedRate = RATE_11;   break;
	}

	return MaxSupportedRate;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	Note:
	
	========================================================================
*/
VOID RTMPSuspendMsduTransmission(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

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

	pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;

	RTMPSetAGCInitValue(pAd, BW_20);
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPResumeMsduTransmission(RTMP_ADAPTER *pAd)
{  
	DBGPRINT(RT_DEBUG_TRACE,("SCAN done, resume MSDU transmission ...\n"));

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
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}
	bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);
	
	pAd->CommonCfg.BBPCurrentBW = pAd->hw_cfg.bbp_bw;
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
/* sample, for IRQ LOCK to SEM LOCK */
/*
	IrqState = pAd->irq_disabled;
	if (IrqState)
		RTMPDeQueuePacket(pAd, TRUE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	else
*/
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);
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
	pEntry = MacTableLookup(pAd, pAddr);

	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;

	if (NState == NDIS_STATUS_SUCCESS) 
	{
		frm_len = sizeof(HEADER_802_11);

#ifdef CONFIG_AP_SUPPORT
//		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) 
		if (pEntry && pEntry->wdev->wdev_type == WDEV_TYPE_AP)
		{
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr, 
							pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			goto body;
		}
#endif /* CONFIG_AP_SUPPORT */


body:
		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull)
		{
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

		DBGPRINT(RT_DEBUG_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate], AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (PUCHAR)pNullFr, frm_len);

		MlmeFreeMemory(pAd, pFrame);
	}
}


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
VOID ApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull,
	IN 	PMAC_TABLE_ENTRY pMacEntry,
	IN	USHORT          PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	STA_TR_ENTRY *tr_entry;
	PAPCLI_STRUCT pApCliEntry = NULL;
	struct wifi_dev *wdev;
	
	pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
	wdev = &pApCliEntry->wdev;

    /* WPA 802.1x secured port control */
	// TODO: shiang-usw, check [wdev/tr_entry]->PortSecured!
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
	if (pMacEntry && (pMacEntry->bReptCli == TRUE))
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



VOID RtmpPrepareHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR OpMode,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN CHAR Index)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR *pNullFrame, *tmac_info = (UCHAR *)&pAd->NullTxWI;
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	ULONG Length;
	UCHAR *ptr;
	UINT i;
	UINT32 longValue;
	MAC_TX_INFO mac_info;

	// TODO: shiang-7603!! fix me
	if (IS_MT7603(pAd) || IS_MT7628(pAd)) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): MT7603 Not support yet!\n", __FUNCTION__));
		return;
	}

	NState = MlmeAllocateMemory(pAd, (PUCHAR *)&pNullFrame);

	NdisZeroMemory(pNullFrame, 48);
	NdisZeroMemory(tmac_info, TXWISize);

	if (NState == NDIS_STATUS_SUCCESS) 
	{
		pNullFr = (PHEADER_802_11) pNullFrame;
		Length = sizeof(HEADER_802_11);
		
		pNullFr->FC.Type = FC_TYPE_DATA;
		pNullFr->FC.SubType = SUBTYPE_DATA_NULL;
		if (Index == 1)
			pNullFr->FC.ToDs = 0;
		else
		pNullFr->FC.ToDs = 1;
		pNullFr->FC.FrDs = 0;

		COPY_MAC_ADDR(pNullFr->Addr1, pEntry->Addr);
		{
			COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);
		}

		pNullFr->FC.PwrMgmt = PwrMgmt;
	
		pNullFr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, pAd->CommonCfg.TxRate, 14);

		/* sequence is increased in MlmeHardTx */
		pNullFr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence  */

		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + Length;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			Length += 2;
		}

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));	
	mac_info.FRAG = FALSE;
	
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = FALSE;
	mac_info.AMPDU = FALSE;
	
	mac_info.BM = IS_BM_MAC_ADDR(pNullFr->Addr1);
	mac_info.Ack = TRUE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	
	mac_info.WCID = pEntry->Aid;
	mac_info.Length = Length;
	
	mac_info.TID = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.SpeEn = 1;
	
		if (Index == 1)
		{
			HTTRANSMIT_SETTING Transmit;
				
			Transmit.word = pEntry->MaxHTPhyMode.word;
			Transmit.field.BW = 0;
			if (Transmit.field.MCS > 7)
				Transmit.field.MCS = 7;

			mac_info.PID = (UCHAR)Transmit.field.MCS;
			mac_info.TxRate = (UCHAR)Transmit.field.MCS;
			write_tmac_info(pAd, tmac_info, &mac_info, &Transmit);
		}
		else
		{
			mac_info.PID = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
			mac_info.TxRate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
			write_tmac_info(pAd, tmac_info, &mac_info,
							&pAd->CommonCfg.MlmeTransmit);
		}

		if (bWaitACK) {
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP) {
				TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

				pTxWI->TXWI_O.TXRPT = 1;
			}
#endif /* RTMP_MAC */
#ifdef RLT_MAC
			// TODO: shiang, how about RT65xx series??
#endif /* RLT_MAC */
		}

		ptr = (PUCHAR)&pAd->NullTxWI;
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
		for (i=0; i < TXWISize; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0)
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[0] + i, longValue);
			else if (Index == 1)
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[1] + i, longValue);
				
			ptr += 4;
		}
		
		ptr = pNullFrame;
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif /* RT_BIG_ENDIAN */
		for (i= 0; i< Length; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0) //for ra0 
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[0] + TXWISize+ i, longValue);
			else if (Index == 1) //for p2p0
				RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[1] + TXWISize+ i, longValue);
				
			ptr += 4;
		}
	}

	if (pNullFrame)
		MlmeFreeMemory(pAd, pNullFrame);
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
#ifdef WH_EZ_SETUP 
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_mlme_sync_lock;
#endif
#endif
 
VOID MlmeHandler(RTMP_ADAPTER *pAd) 
{
	MLME_QUEUE_ELEM *Elem = NULL;
#ifdef APCLI_SUPPORT
	SHORT apcliIfIndex;
#endif /* APCLI_SUPPORT */

	/* Only accept MLME and Frame from peer side, no other (control/data) frame should*/
	/* get into this state machine*/
#ifdef WH_EZ_SETUP 	
	EZ_ACQUIRE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
#endif

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning) 
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#ifdef WH_EZ_SETUP 		
		EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
#endif
		
		return;
	} 
	else 
	{
#ifdef WH_EZ_SETUP
	if (ez_is_other_band_mlme_running(&pAd->ApCfg.MBSSID[0].wdev)) {
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
		return;
	}
#endif
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#ifdef WH_EZ_SETUP 		
	EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
#endif
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue) 
#ifdef EAPOL_QUEUE_SUPPORT
		|| !(EAPMlmeQueueEmpty(&pAd->Mlme.EAP_Queue))
#endif /* EAPOL_QUEUE_SUPPORT */
		) 
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND)
		)
		{
			DBGPRINT(RT_DEBUG_OFF, ("System halted, removed or MlmeRest, exit MlmeTask!(QNum = %ld)\n",
						pAd->Mlme.Queue.Num));
			break;
		}
		
#ifdef CONFIG_ATE			
		if(ATE_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_INFO, ("%s(): Driver is in ATE mode\n", __FUNCTION__));
			break;
		}	
#endif /* CONFIG_ATE */

		/*From message type, determine which state machine I should drive*/

		Elem = NULL;
#ifdef EAPOL_QUEUE_SUPPORT
		if (!EAPMlmeQueueEmpty(&pAd->Mlme.EAP_Queue))
			EAPMlmeDequeue(&pAd->Mlme.EAP_Queue, &Elem);
		else if (!MlmeQueueEmpty(&pAd->Mlme.Queue))
#endif /* EAPOL_QUEUE_SUPPORT */
			MlmeDequeue(&pAd->Mlme.Queue, &Elem);

		if (Elem) 
		{

			if (Elem->Machine == WPA_STATE_MACHINE && Elem->MsgType == MT2_EAPOLKey)
			{
						if (((!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) &&
				( !OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)))

				)
				{
#ifdef EAPOL_QUEUE_SUPPORT				
					EAP_MLME_QUEUE	*Queue = (EAP_MLME_QUEUE *)&pAd->Mlme.EAP_Queue;
#else /* EAPOL_QUEUE_SUPPORT */
					MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#endif /* !EAPOL_QUEUE_SUPPORT */
					ULONG Tail;
					NdisAcquireSpinLock(&(Queue->Lock));
					Tail = Queue->Tail;
					Queue->Tail++;
					Queue->Num++;
					if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
						Queue->Tail = 0;
					
					Queue->Entry[Tail].Wcid = RESERVED_WCID;
					Queue->Entry[Tail].Occupied = TRUE;
					Queue->Entry[Tail].Machine = Elem->Machine;
					Queue->Entry[Tail].MsgType = Elem->MsgType;
					Queue->Entry[Tail].MsgLen = Elem->MsgLen;	
					Queue->Entry[Tail].Priv = Elem->Priv;
					NdisZeroMemory(Queue->Entry[Tail].Msg, MGMT_DMA_BUFFER_SIZE);

					NdisMoveMemory(Queue->Entry[Tail].Msg, Elem->Msg, Elem->MsgLen);
					
						
					NdisReleaseSpinLock(&(Queue->Lock));

					Elem->Occupied = FALSE;
					Elem->MsgLen = 0;
					continue;
					
				}
			}

			/* if dequeue success*/
			switch (Elem->Machine) 
			{
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
#ifdef WH_EZ_SETUP	
#ifdef EZ_MOD_SUPPORT
				case EZ_STATE_MACHINE:
//! Levarage from MP1.0 CL#170063
					StateMachinePerformAction(pAd, &pAd->Mlme.EzMachine,
									Elem, pAd->Mlme.EzMachine.CurrState);
					break;					

#else
				case EZ_ROAM_STATE_MACHINE:
//! Levarage from MP1.0 CL#170063
					StateMachinePerformAction(pAd, &pAd->Mlme.EzRoamMachine,
									Elem, pAd->Mlme.EzRoamMachine.CurrState);
					break;					
				case AP_TRIBAND_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApTriBandMachine,
									Elem, pAd->Mlme.ApTriBandMachine.CurrState);
					break;

#endif
#endif

#ifdef APCLI_SUPPORT
				case APCLI_AUTH_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG AuthCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

					if (Elem->Priv >= 64) {
						CliIdx = ((Elem->Priv - 64) % 16);
						AuthCurrState =
		pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].AuthCurrState;
					} else
#endif /* MAC_REPEATER_SUPPORT */
							AuthCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AuthCurrState;

						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAuthMachine,
								Elem, AuthCurrState);
					}
					break;

				case APCLI_ASSOC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))		
					{
						ULONG AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AssocCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

						apcliIfIndex = Elem->Priv;

						if (apcliIfIndex >= 64)
						{
							CliIdx = ((apcliIfIndex - 64) % 16);
							apcliIfIndex = ((apcliIfIndex - 64) / 16);
							AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].AssocCurrState;
						}
#endif /* MAC_REPEATER_SUPPORT */
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAssocMachine,
								Elem, AssocCurrState);
					}
					break;

				case APCLI_SYNC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MULTI_APCLI_SUPPORT
					DBGPRINT(RT_DEBUG_TRACE, ("\x1b[35m APCLI_SYNC_STATE_MACHINE  apcliIfindex = %d  SyncCurrState=%lu \x1b[m\n", apcliIfIndex, pAd->ApCfg.ApCliTab[apcliIfIndex].SyncCurrState));
#endif /* MULTI_APCLI_SUPPORT */
					if(isValidApCliIf(apcliIfIndex))
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliSyncMachine, Elem,
							(pAd->ApCfg.ApCliTab[apcliIfIndex].SyncCurrState));
					break;

				case APCLI_CTRL_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;

#ifdef MULTI_APCLI_SUPPORT
					DBGPRINT(RT_DEBUG_TRACE, ("\x1b[34m APCLI_CTRL_STATE_MACHINE  apcliIfindex=%d  CtrlCurrState=%lu \x1b[m\n", apcliIfIndex, pAd->ApCfg.ApCliTab[apcliIfIndex].CtrlCurrState));
#endif /* MULTI_APCLI_SUPPORT */
					
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].CtrlCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

						apcliIfIndex = Elem->Priv;

						if (apcliIfIndex >= 64)
						{
							CliIdx = ((apcliIfIndex - 64) % 16);
							apcliIfIndex = ((apcliIfIndex - 64) / 16);
							CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].CtrlCurrState;
						}
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
					if (pAd->pWscElme)
					{
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

#ifdef DOT11U_INTERWORKING
				case GAS_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.GASMachine, Elem,
										GASPeerCurrentState(pAd, Elem));
					break;
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

#ifdef MIXMODE_SUPPORT
				case MIX_MODE_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->MixModeCtrl.MixModeStatMachine, Elem,
										pAd->MixModeCtrl.MixModeStatMachine.CurrState);
					break;
#endif /* MIXMODE_SUPPORT */
#ifdef WIFI_DIAG
				case WIFI_DAIG_STATE_MACHINE:
					DiagLogFileWrite();
					DiagAssocErrorFileWrite();
					break;
#endif
				default:
					DBGPRINT(RT_DEBUG_TRACE, ("%s(): Illegal SM %ld\n",
								__FUNCTION__, Elem->Machine));
					break;
			} /* end of switch*/

			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("%s(): MlmeQ empty\n", __FUNCTION__));
		}
	}

#ifdef WH_EZ_SETUP
	EZ_ACQUIRE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
#endif
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#ifdef WH_EZ_SETUP
	EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd);
#endif
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

	while (!RTMP_OS_TASK_IS_KILLED(pTask))
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
			
		/* lock the device pointers , need to check if required*/
		/*down(&(pAd->usbdev_semaphore)); */
		
		if (!pAd->PM_FlgSuspend)
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
	DBGPRINT(RT_DEBUG_TRACE,( "<---%s\n",__FUNCTION__));
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
#ifdef WH_EZ_SETUP
#ifdef EZ_MOD_SUPPORT
				EzStateMachineInit(pAd, &pAd->Mlme.EzMachine, pAd->Mlme.EzFunc);
#else
		//! Levarage from MP1.0 CL#170063
				EzRoamStateMachineInit(pAd, &pAd->Mlme.EzRoamMachine, pAd->Mlme.EzRoamFunc);
				APTriBandStateMachineInit(pAd, &pAd->Mlme.ApTriBandMachine, pAd->Mlme.ApTriBandFunc);
#endif
#endif		
		
}
#endif /* CONFIG_AP_SUPPORT */

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

	DBGPRINT(RT_DEBUG_TRACE, ("--> MLME Initialize\n"));

	do 
	{

#ifdef EAPOL_QUEUE_SUPPORT
		Status = MlmeQueueInit(pAd, &pAd->Mlme.EAP_Queue, &pAd->Mlme.Queue);
#else /* EAPOL_QUEUE_SUPPORT */	
		Status = MlmeQueueInit(pAd, &pAd->Mlme.Queue);
#endif /* !EAPOL_QUEUE_SUPPORT */

		if(Status != NDIS_STATUS_SUCCESS) 
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(pAd, &pAd->Mlme.TaskLock);

		
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
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
#endif/*CONFIG_HOTSPOT*/
#ifdef DOT11U_INTERWORKING
		GASStateMachineInit(pAd, &pAd->Mlme.GASMachine, pAd->Mlme.GASFunc);
#endif

#ifdef DOT11K_RRM_SUPPORT
		RRMBcnReqStateMachineInit(pAd, &pAd->Mlme.BCNMachine, pAd->Mlme.BCNFunc);
		NRStateMachineInit(pAd, &pAd->Mlme.NRMachine, pAd->Mlme.NRFunc);	
#endif

#ifdef CONFIG_DOT11V_WNM
		WNMCtrlInit(pAd);
		BTMStateMachineInit(pAd, &pAd->Mlme.BTMMachine, pAd->Mlme.BTMFunc);
#ifdef CONFIG_HOTSPOT_R2
		WNMNotifyStateMachineInit(pAd, &pAd->Mlme.WNMNotifyMachine, pAd->Mlme.WNMNotifyFunc);
#endif
#endif

#ifdef ACL_V2_SUPPORT
		ACL_V2_CtrlInit(pAd);
#endif /* ACL_V2_SUPPORT */


#ifdef SNIFFER_MIB_CMD
		sniffer_mib_ctrlInit(pAd);
#endif /* SNIFFER_MIB_CMD */


		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);

		/* Init mlme periodic timer*/
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExecTimer), pAd, TRUE);

		/* Set mlme periodic timer*/
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

		/* software-based RX Antenna diversity*/
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Init APSD periodic timer*/
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);

			/* Init APQuickResponseForRateUp timer.*/
			RTMPInitTimer(pAd, &pAd->ApCfg.ApQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(APQuickResponeForRateUpExec), pAd, FALSE);
			pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */



	} while (FALSE);

	{
		RTMP_OS_TASK *pTask;

		/* Creat MLME Thread */
		pTask = &pAd->mlmeTask;
		RTMP_OS_TASK_INIT(pTask, "RtmpMlmeTask", pAd);
		Status = RtmpOSTaskAttach(pTask, MlmeThread, (ULONG)pTask);
		if (Status == NDIS_STATUS_FAILURE) {
			DBGPRINT (RT_DEBUG_ERROR,  ("%s: unable to start MlmeThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

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
		
	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeHalt\n"));

	/* Terminate Mlme Thread */
	pTask = &pAd->mlmeTask;
	if (RtmpOSTaskKill(pTask) == NDIS_STATUS_FAILURE) {
		DBGPRINT(RT_DEBUG_ERROR, ("kill mlme task failed!\n"));
	}
	
#if (defined(MT_WOW_SUPPORT) && defined(WOW_IFDOWN_SUPPORT))
	if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd)))
#endif
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/* disable BEACON generation and other BEACON related hardware timers*/
		AsicDisableSync(pAd);
	}
	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);


	RTMPCancelTimer(&pAd->Mlme.RxAntEvalTimer, &Cancelled);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		UCHAR idx = 0;
#endif /* APCLI_SUPPORT */
		RTMPCancelTimer(&pAd->Mlme.APSDPeriodicTimer, &Cancelled);

		if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
			RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);

#ifdef APCLI_SUPPORT
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
		{
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
			RTMPCancelTimer(&pApCliEntry->MlmeAux.ProbeTimer, &Cancelled);
			RTMPCancelTimer(&pApCliEntry->MlmeAux.ApCliAssocTimer, &Cancelled);
			RTMPCancelTimer(&pApCliEntry->MlmeAux.ApCliAuthTimer, &Cancelled);

#ifdef WSC_AP_SUPPORT
			if (pApCliEntry->WscControl.WscProfileRetryTimerRunning)
		{
				pApCliEntry->WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPCancelTimer(&pApCliEntry->WscControl.WscProfileRetryTimer, &Cancelled);
		}
#endif /* WSC_AP_SUPPORT */
		}
#endif /* APCLI_SUPPORT */
		RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	}

#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_HOTSPOT
	HSCtrlHalt(pAd);
	HSCtrlExit(pAd);
#endif

#ifdef DOT11U_INTERWORKING
	GASCtrlExit(pAd);
#endif

#ifdef CONFIG_DOT11V_WNM
	WNMCtrlExit(pAd);
#endif

#ifdef ACL_V2_SUPPORT
	ACL_V2_CtrlExit(pAd);
#endif /* ACL_V2_SUPPORT */

#ifdef SNIFFER_MIB_CMD
		sniffer_mib_ctrlExit(pAd);
#endif /* SNIFFER_MIB_CMD */



	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
		
#ifdef LED_CONTROL_SUPPORT		
		/* Set LED*/
		RTMPSetLED(pAd, LED_HALT);
		RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it.*/
#endif /* LED_CONTROL_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		if (!(pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd))
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)*/
			if (pChipOps->AsicHaltAction)
				pChipOps->AsicHaltAction(pAd);
	}

	RtmpusecDelay(5000);    /*  5 msec to gurantee Ant Diversity timer canceled*/

#ifdef EAPOL_QUEUE_SUPPORT
	MlmeQueueDestroy(&pAd->Mlme.EAP_Queue, &pAd->Mlme.Queue);
#else /* EAPOL_QUEUE_SUPPORT */
	MlmeQueueDestroy(&pAd->Mlme.Queue);
#endif /* !EAPOL_QUEUE_SUPPORT */

	NdisFreeSpinLock(&pAd->Mlme.TaskLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeHalt\n"));
}


VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;

#ifdef CONFIG_ATE
	if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
		/* for performace enchanement */
		NdisZeroMemory(&pAd->RalinkCounters,
						(UINT32)&pAd->RalinkCounters.OneSecEnd -
						(UINT32)&pAd->RalinkCounters.OneSecStart);

	return;
}

int TX_IDLE_TIME = 100;
int Tx_Idle_Count[MAX_LEN_OF_MAC_TABLE];
int pkg_cnt[MAX_LEN_OF_MAC_TABLE];
void TIMMonitor(RTMP_ADAPTER *pAd)
{
	STA_TR_ENTRY *tr_entry = NULL;
	int wcid=0;
	UINT32 ent_type = ENTRY_CLIENT;
	int  i,j;
	int Total_Packet_Number=0;
	static int init=0;
#ifdef UAPSD_SUPPORT
	QUEUE_HEADER *pQueApsd;
#endif

	if (init == 0)
	{
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			Tx_Idle_Count[i]=TX_IDLE_TIME;
			pkg_cnt[i] = 10;
		}
		init = 1;
	}

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		
		/* dump MacTable entries which match the EntryType */
		if (pEntry->EntryType != ent_type)
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) ) && (pEntry->Sst != SST_ASSOC))
			continue;
		
		wcid = pEntry->wcid;
		tr_entry = &pAd->MacTab.tr_entry[wcid];

		for (j=0;j< WMM_NUM_OF_AC;j++)
		{
			Total_Packet_Number = Total_Packet_Number + tr_entry->tx_queue[j].Number;
		}
		Total_Packet_Number = Total_Packet_Number + tr_entry->ps_queue.Number;
		
#ifdef UAPSD_SUPPORT			
		for(j=(WMM_NUM_OF_AC-1); j>=0; j--)
		{
			pQueApsd = &(pEntry->UAPSDQueue[j]);
			Total_Packet_Number = Total_Packet_Number + pQueApsd->Number;
		}
#endif
		if (Total_Packet_Number != 0)
		{
			//WLAN_MR_TIM_BIT_SET(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
		   Tx_Idle_Count[wcid] = TX_IDLE_TIME;
		   pkg_cnt[wcid]=10;	
		}
		else
		{
			if (Tx_Idle_Count[wcid])
			{
				Tx_Idle_Count[wcid]--;
			}
			else
		    {
				WLAN_MR_TIM_BIT_CLEAR1(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
		    }	

			if (pkg_cnt[wcid])
				pkg_cnt[wcid]--;
			else
				pkg_cnt[wcid]=10;	
		}
	}
}


#ifdef SW_ATF_SUPPORT

/* Decide if wcid in tx flow */
static BOOLEAN isTxFlow(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry)
{
/*
* according deqCnt to decide if wcid in tx flow
* period 100ms, pkt len = 1514, target TP = 5M, so deqCount = 43
*/
	if (pEntry->atfPara.deqCount_100ms > pAd->AtfParaSet.wcidTxThr) {
		pEntry->atfPara.inTxFlow = TRUE;
		return TRUE;
	} else {
		pEntry->atfPara.inTxFlow = FALSE;
		pEntry->atfPara.deqCount_100ms = 0;
		pEntry->atfPara.isBadNode = FALSE;
		return FALSE;
	}
}
static void findBadNode(PRTMP_ADAPTER pAd, UINT32 *wcidInTxFlow)
{
	PMAC_TABLE_ENTRY pEntry1, pEntry2;
	UINT32 tmp;
	LONG rssi0, rssi1, avgRssi1, avgRssi2;
/* Just condsider 2 STA, n mdoe and b/g mode */
	if (pAd->AtfParaSet.txWcidNum == 2) {
		pEntry1 = &(pAd->MacTab.Content[wcidInTxFlow[1]]);
		pEntry2 = &(pAd->MacTab.Content[wcidInTxFlow[2]]);
		goodCnt = pEntry1->atfPara.deqCount_100ms;
		badCnt = pEntry2->atfPara.deqCount_100ms;
		pEntry1->atfPara.deqCount_100ms = 0;
		pEntry2->atfPara.deqCount_100ms = 0;
		if (pEntry1->SupportRateMode & SUPPORT_HT_MODE) {
			if (!(pEntry2->SupportRateMode & SUPPORT_HT_MODE)) {
				pEntry1->atfPara.isBadNode = FALSE;
				pEntry2->atfPara.isBadNode = TRUE;
				pAd->AtfParaSet.flagATF = TRUE;
				/* default set badNode index to 1 */
				tmp = wcidInTxFlow[1];
				wcidInTxFlow[1] = wcidInTxFlow[2];
				wcidInTxFlow[2] = tmp;
				return;
			}
		} else if (pEntry2->SupportRateMode & SUPPORT_HT_MODE) {
			if (!(pEntry1->SupportRateMode & SUPPORT_HT_MODE)) {
				pEntry1->atfPara.isBadNode = TRUE;
				pEntry2->atfPara.isBadNode = FALSE;
				pAd->AtfParaSet.flagATF = TRUE;
				tmp = goodCnt;
				goodCnt = badCnt;
				badCnt = tmp;
				return;
			}
		}
			rssi0 = pEntry1->RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta;
			rssi1 = pEntry1->RssiSample.AvgRssi[1] - pAd->BbpRssiToDbmDelta;
			avgRssi1 = (rssi0 + rssi1) / 2;
			rssi0 = pEntry2->RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta;
			rssi1 = pEntry2->RssiSample.AvgRssi[1] - pAd->BbpRssiToDbmDelta;
			avgRssi2 = (rssi0 + rssi1) / 2;
			if ((avgRssi2 > -45) && (avgRssi1 < -60) && (pEntry1->HTPhyMode.field.MCS != 15)) {
				pEntry1->atfPara.isBadNode = TRUE;
				pEntry2->atfPara.isBadNode = FALSE;
				pAd->AtfParaSet.flagATF = TRUE;
				tmp = goodCnt;
				goodCnt = badCnt;
				badCnt = tmp;
				return;
			} else if ((avgRssi1 > -45) && (avgRssi2 < -60) && (pEntry2->HTPhyMode.field.MCS != 15)) {
				pEntry2->atfPara.isBadNode = TRUE;
				pEntry1->atfPara.isBadNode = FALSE;
				tmp = wcidInTxFlow[1];
				wcidInTxFlow[1] = wcidInTxFlow[2];
				wcidInTxFlow[2] = tmp;
				pAd->AtfParaSet.flagATF = TRUE;
				return;

			}
	}
	pAd->AtfParaSet.flagATF = FALSE;
	return;
}
static void setGoodNodeEntryPara(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY entry)
{
	entry->atfThr.deqMaxThr = pAd->AtfParaSet.deq_goodNodeMaxThr;
	entry->atfThr.deqMinThr = pAd->AtfParaSet.deq_goodNodeMinThr;

}
static void setBadNodeEntryPara(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY entry)
{
	entry->atfThr.enqMaxThr = pAd->AtfParaSet.enq_badNodeMaxThr;
	entry->atfThr.enqMinThr = pAd->AtfParaSet.enq_badNodeMinThr;
	entry->atfThr.deqMinThr = pAd->AtfParaSet.deq_badNodeMinThr;
}
static void setWcidenqThr(PRTMP_ADAPTER pAd, UINT32 *wcidInTxFlow)
{
	PMAC_TABLE_ENTRY goodNodeEntry = NULL;
	PMAC_TABLE_ENTRY badNodeEntry = NULL;

	goodNodeEntry = &(pAd->MacTab.Content[wcidInTxFlow[2]]);
	badNodeEntry = &(pAd->MacTab.Content[wcidInTxFlow[1]]);
	setGoodNodeEntryPara(pAd, goodNodeEntry);
	setBadNodeEntryPara(pAd, badNodeEntry);
	DBGPRINT(RT_DEBUG_INFO,
		("notBadWcid:%d,deqCount:%d\n", goodNodeEntry->wcid, goodNodeEntry->atfPara.deqCount_100ms));
	DBGPRINT(RT_DEBUG_INFO,
		("badWcid:%d,deqCount:%d\n", badNodeEntry->wcid, badNodeEntry->atfPara.deqCount_100ms));
	if (goodCnt > goodNodeEntry->atfThr.deqMaxThr) {
		if (pAd->AtfParaSet.flagOnce == FALSE) {
			if (pAd->AtfParaSet.enq_badNodeCurrent >= 10)
				pAd->AtfParaSet.enq_badNodeCurrent = 10;
			else
				pAd->AtfParaSet.enq_badNodeCurrent++;
			badNodeEntry->atfThr.enqCurrentThr = pAd->AtfParaSet.enq_badNodeCurrent;
		} else
			badNodeEntry->atfThr.enqCurrentThr = badNodeEntry->atfThr.enqMaxThr;
	} else if (goodCnt < goodNodeEntry->atfThr.deqMinThr) {
		if (pAd->AtfParaSet.flagOnce == FALSE) {
			if (pAd->AtfParaSet.enq_badNodeCurrent <= 2)
				pAd->AtfParaSet.enq_badNodeCurrent = 2;
			else
				pAd->AtfParaSet.enq_badNodeCurrent--;
			badNodeEntry->atfThr.enqCurrentThr = pAd->AtfParaSet.enq_badNodeCurrent;
		} else
			badNodeEntry->atfThr.enqCurrentThr = badNodeEntry->atfThr.enqMinThr;
	}
	if (badCnt < badNodeEntry->atfThr.deqMinThr) {
		if (pAd->AtfParaSet.flagOnce == FALSE) {
			if (pAd->AtfParaSet.enq_badNodeCurrent >= 10)
				pAd->AtfParaSet.enq_badNodeCurrent = 10;
			else
				pAd->AtfParaSet.enq_badNodeCurrent++;
			badNodeEntry->atfThr.enqCurrentThr = pAd->AtfParaSet.enq_badNodeCurrent;
		} else
			badNodeEntry->atfThr.enqCurrentThr = badNodeEntry->atfThr.enqMaxThr;
	}
	badNodeEntry->atfPara.deqCount_100ms = 0;
	goodNodeEntry->atfPara.deqCount_100ms = 0;
}

static void badNodeDetect(PRTMP_ADAPTER pAd)
{
	UINT32 i = 0, j = 1;
	UINT32 wcidInTxFlow[MAX_LEN_OF_MAC_TABLE] = {0};
	PMAC_TABLE_ENTRY pEntry;

	pAd->AtfParaSet.txWcidNum = 0;
/*
*step1: Find associated wcid in tx flow.
*step2: According to per wcid, we should set wcid priority and through get deq parameter to set enq drop threshold.
*/
/* step1 */
	if (pAd->RalinkCounters.OneSecFalseCCACnt >= pAd->AtfParaSet.atfFalseCCAThr) {
		pAd->AtfParaSet.flagATF = FALSE;
		return;
	}
	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &(pAd->MacTab.Content[i]);
		if (pEntry != NULL && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			if (isTxFlow(pAd, pEntry)) {
				pAd->AtfParaSet.txWcidNum++;
				wcidInTxFlow[j++] = pEntry->wcid;
			}
		}
	}
/*step2 */
	if (pAd->AtfParaSet.txWcidNum == 2) {
		/*find badNode*/
		findBadNode(pAd, wcidInTxFlow);
		if (pAd->AtfParaSet.flagATF) {
			setWcidenqThr(pAd, wcidInTxFlow);
			return;
		}
	}
	pAd->AtfParaSet.flagATF = FALSE;
	return;
}
#endif
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

VOID MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	RTEnqueueInternalCmd(pAd, CMDTHREAD_MLME_PERIOIDC_EXEC, NULL, 0);
}

NTSTATUS MlmePeriodicExec(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	ULONG TxTotalCnt;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	/* No More 0x84 MCU CMD from v.30 FW*/

//CFG

#ifdef MICROWAVE_OVEN_SUPPORT
	/* update False CCA count to an array */
	NICUpdateRxStatusCnt1(pAd, pAd->Mlme.PeriodicRound%10);

	if (pAd->CommonCfg.MO_Cfg.bEnable)
	{
		UINT8 stage = pAd->Mlme.PeriodicRound%10;

		if (stage == MO_MEAS_PERIOD)
		{
			ASIC_MEASURE_FALSE_CCA(pAd);
			pAd->CommonCfg.MO_Cfg.nPeriod_Cnt = 0;
		}
		else if (stage == MO_IDLE_PERIOD)
		{
			UINT16 Idx;

			for (Idx = MO_MEAS_PERIOD + 1; Idx < MO_IDLE_PERIOD + 1; Idx++)
				pAd->CommonCfg.MO_Cfg.nFalseCCACnt += pAd->RalinkCounters.FalseCCACnt_100MS[Idx];

			//printk("%s: fales cca1 %d\n", __FUNCTION__, pAd->CommonCfg.MO_Cfg.nFalseCCACnt);
			if (pAd->CommonCfg.MO_Cfg.nFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				ASIC_MITIGATE_MICROWAVE(pAd);

		}
	}
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef INF_AMAZON_SE
#endif /* INF_AMAZON_SE */


	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RADIO_MEASUREMENT |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
		return NDIS_STATUS_SUCCESS;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return NDIS_STATUS_SUCCESS;

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_status_chk_and_get(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */



	pAd->bUpdateBcnCntDone = FALSE;
	
/*	RECBATimerTimeout(SystemSpecific1,FunctionContext,SystemSpecific2,SystemSpecific3);*/
	pAd->Mlme.PeriodicRound ++;
	pAd->Mlme.GPIORound++;

#ifdef CONFIG_BA_REORDER_MONITOR
	ba_timeout_monitor(pAd);
#endif

#ifndef WFA_VHT_PF
#endif /* WFA_VHT_PF */

#if defined(MT_MAC) && defined(CONFIG_STA_SUPPORT)
	if((pAd->Mlme.PeriodicRound % 10) == 0)
	{
		periodic_monitor_ac_counter(pAd,QID_AC_VI);
	}
#endif


#ifdef MT_MAC
	if ((pAd->Mlme.PeriodicRound % 1) == 0)
	{
#ifdef RTMP_MAC_PCI
		if (pAd->PDMAWatchDogEn)
		{
			PDMAWatchDog(pAd);
		}

		if (pAd->PSEWatchDogEn)
		{
#ifdef DMA_RESET_SUPPORT
			//check every second
			if ((pAd->Mlme.PeriodicRound % 10) == 0)
			{
				PSEACStuckWatchDog(pAd);
			}
#endif /* DMA_RESET_SUPPORT */
		
			PSEWatchDog(pAd);	
		}
#endif
	}
#ifdef MT_PS
	if ((pAd->Mlme.PeriodicRound % 10) == 0) {
		UINT32 i;
		STA_TR_ENTRY *tr_entry;
		MAC_TABLE_ENTRY *pMacEntry;
		ULONG Now32;

		NdisGetSystemUpTime(&Now32);

		NdisAcquireSpinLock(&pAd->MacTabLock);
		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
			tr_entry = &pAd->MacTab.tr_entry[i];
			pMacEntry = &pAd->MacTab.Content[i];
			if (IS_VALID_ENTRY(tr_entry) && IS_ENTRY_CLIENT(pMacEntry)) {
				if ((tr_entry->ps_state != APPS_RETRIEVE_DONE) &&
					(tr_entry->ps_state != APPS_RETRIEVE_IDLE) &&
					RTMP_TIME_AFTER((unsigned long)Now32,
					(unsigned long)(tr_entry->ps_start_time + 4 * OS_HZ))) {
					PsRetrieveTimeout(pAd, tr_entry);
				}
			}
		}
		NdisReleaseSpinLock(&pAd->MacTabLock);
	}
#endif

    /* Following is the TxOP scenario, monitor traffic in every minutes */
    if ((pAd->Mlme.PeriodicRound % 10) == 0)
    {
#ifdef DYNAMIC_WMM
	if (pAd->CommonCfg.DynamicWmm == 1)
		DynamicWMMDetectAction(pAd);
	else
#endif
		TxOPUpdatingAlgo(pAd);
    }
	
#endif /* MT_MAC */


#ifdef SW_ATF_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				badNodeDetect(pAd);
#endif
#endif


	/* by default, execute every 500ms */
	if ((pAd->ra_interval) && 
		((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0))
	{

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			//AsicRssiUpdate(pAd);
			//AsicTxCntUpdate(pAd, 0);
		}
#endif /* MT_MAC */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			APMlmeDynamicTxRateSwitching(pAd);
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		 IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			 TIMMonitor(pAd);
		 }
#endif /* CONFIG_AP_SUPPORT */




	/* Normal 1 second Mlme PeriodicExec.*/
	if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
	{
		pAd->Mlme.OneSecPeriodicRound ++;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dynamic_tune_be_tx_op(pAd, 50);	/* change form 100 to 50 for WMM WiFi test @20070504*/
#endif /* CONFIG_AP_SUPPORT */

		NdisGetSystemUpTime(&pAd->Mlme.Now32);

		/* add the most up-to-date h/w raw counters into software variable, so that*/
		/* the dynamic tuning mechanism below are based on most up-to-date information*/
		/* Hint: throughput impact is very serious in the function */
#ifndef RTMP_SDIO_SUPPORT
		NICUpdateRawCounters(pAd);
#endif /*leonardo mark it for FPGA debug*/
		RTMP_SECOND_CCA_DETECTION(pAd);

#ifdef ANTI_INTERFERENCE_SUPPORT
		if (pAd->bDynamicRaInterval)
			DynamicRaInterval(pAd);
#endif /* ANTI_INTERFERENCE_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
		dynamic_vga_adjust(pAd);
#endif /* DYNAMIC_VGA_SUPPORT */


#ifdef DOT11_N_SUPPORT
   		/* Need statistics after read counter. So put after NICUpdateRawCounters*/
		ORIBATimerTimeout(pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_WIFI_TEST
		if (pAd->Mlme.OneSecPeriodicRound % 1 == 0)
		{
#ifdef RTMP_PCI_SUPPORT
			MonitorTxRing(pAd);
			MonitorTxRing(pAd);
			MonitorTxRing(pAd);
			MonitorTxRing(pAd);
#endif
		}
#endif

#ifdef DYNAMIC_VGA_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#endif /* DYNAMIC_VGA_SUPPORT */

	/*
		if (pAd->RalinkCounters.MgmtRingFullCount >= 2)
			RTMP_SET_FLAG(pAd, fRTMP_HW_ERR);
		else
			pAd->RalinkCounters.MgmtRingFullCount = 0;
	*/

		/* The time period for checking antenna is according to traffic*/
		{
			if (pAd->Mlme.bEnableAutoAntennaCheck)
			{
				TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
								 pAd->RalinkCounters.OneSecTxRetryOkCount + 
								 pAd->RalinkCounters.OneSecTxFailCount;
				
				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50)
				{
					if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
						AsicEvaluateRxAnt(pAd);
				}
				else
				{
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

#ifdef MT76x0_TSSI_CAL_COMPENSATION
		if (IS_MT76x0(pAd) &&
			(pAd->chipCap.bInternalTxALC) &&
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE))
		{
			if ((pAd->Mlme.OneSecPeriodicRound % 1) == 0)
			{
				/* TSSI compensation */
				MT76x0_IntTxAlcProcess(pAd);
			}
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */


		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE)
		{
			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)
			{
				{
				}
			}
		}

#ifdef DOT11_N_SUPPORT
#ifdef MT_MAC
        if (pAd->chipCap.hif_type == HIF_MT)
        {
            /* Not RDG, update the TxOP else keep the default RDG's TxOP */
            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == FALSE)
            {
            }
        }
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			APMlmePeriodicExec(pAd);
#ifdef CONFIG_HOTSPOT_R2
#ifdef WAPP_SUPPORT
			wapp_bss_load_check(pAd);
#endif/* WAPP_SUPPORT */
#endif/* CONFIG_HOTSPOT_R2 */

			if ((pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H.RDMode != RD_SILENCE_MODE))				
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode != WDS_BRIDGE_MODE)		
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
				)
				pAd->macwd ++;
			else
				pAd->macwd = 0;

			if (pAd->macwd > 1)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MAC specific condition \n"));

				AsicSetMacWD(pAd);

#ifdef AP_QLOAD_SUPPORT
				Show_QoSLoad_Proc(pAd, NULL);
#endif /* AP_QLOAD_SUPPORT */
			}
		}
#endif /* CONFIG_AP_SUPPORT */



#ifdef SMART_CARRIER_SENSE_SUPPORT

			if (pAd->SCSCtrl.SCSEnable == SCS_ENABLE)
			{
				Smart_Carrier_Sense(pAd);				
			}	
#endif /* SMART_CARRIER_SENSE_SUPPORT */			


		RTMP_SECOND_CCA_DETECTION(pAd);

		MlmeResetRalinkCounters(pAd);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

		RTMP_MLME_HANDLER(pAd);
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_PARTIAL_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((pAd->ApCfg.bPartialScanning == TRUE)  &&
			(pAd->ApCfg.PartialScanChannelNum == DEFLAUT_PARTIAL_SCAN_CH_NUM))/* pAd->ApCfg.PartialScanChannelNum == DEFLAUT_PARTIAL_SCAN_CH_NUM means that one partial scan is finished */
		{
			if (((pAd->ApCfg.PartialScanBreakTime++)%DEFLAUT_PARTIAL_SCAN_BREAK_TIME) == 0)
				ApSiteSurvey(pAd, NULL, SCAN_ACTIVE, FALSE, NULL);
		}
	}
#endif /* AP_PARTIAL_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef WSC_INCLUDED
	WSC_HDR_BTN_MR_HANDLE(pAd);
#endif /* WSC_INCLUDED */




	/*add for hook function on Mlme timer interrupt*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_MLME_PERIOD,NULL,1,pAd);

	pAd->bUpdateBcnCntDone = FALSE;
	
	if(pAd->ed_chk != FALSE)
		ed_status_read(pAd);
	return NDIS_STATUS_SUCCESS;
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
		return (FALSE);

	/* Check each character value*/
	for (index = 0; index < SsidLen; index++)
	{
		if (pSsid[index] < 0x20)
			return (FALSE);
	}

	/* All checked*/
	return (TRUE);
}



VOID RTMPSetEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//pAd->StaCfg.PwrMgmt.bEnterPsmNull = TRUE;
	pPwrMgmt->bEnterPsmNull = TRUE;	
}

VOID RTMPClearEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//pAd->StaCfg.PwrMgmt.bEnterPsmNull = FALSE;
	pPwrMgmt->bEnterPsmNull = FALSE;
}

BOOLEAN RTMPEnterPsmNullBitStatus(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//return (pAd->StaCfg.PwrMgmt.bEnterPsmNull);
	return (pPwrMgmt->bEnterPsmNull);
}


/*
	==========================================================================
	Description:
		This routine calculates TxPER, RxPER of the past N-sec period. And 
		according to the calculation result, ChannelQuality is calculated here 
		to decide if current AP is still doing the job. 

		If ChannelQuality is not good, a ROAMing attempt may be tried later.
	Output:
		StaCfg.ChannelQuality - 0..100

	IRQL = DISPATCH_LEVEL

	NOTE: This routine decide channle quality based on RX CRC error ratio.
		Caller should make sure a function call to NICUpdateRawCounters(pAd)
		is performed right before this routine, so that this routine can decide
		channel quality based on the most up-to-date information
	==========================================================================
 */
VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt/*, TxPER*/, TxPRR;
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

#if (defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT)) && defined(CONFIG_STA_SUPPORT)
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;
#endif




		if (pMacEntry != NULL)
		{
			pRssiSample = &pMacEntry->RssiSample;
			OneSecTxNoRetryOkCount = pMacEntry->OneSecTxNoRetryOkCount;
			OneSecTxRetryOkCount = pMacEntry->OneSecTxRetryOkCount;
			OneSecTxFailCount = pMacEntry->OneSecTxFailCount;
			OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
			OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
		}
		else
		{
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
	if (TxCnt < 5) 
	{
		//TxPER = 0;
		TxPRR = 0;
	}
	else 
	{
		//TxPER = (OneSecTxFailCount * 100) / TxCnt; 
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	
	/* calculate RX PER - don't take RxPER into consideration if too few sample*/
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;
	if (RxCnt < 5)
		RxPER = 0;	
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

#if (defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT)) && defined(CONFIG_STA_SUPPORT)
	if (INFRA_ON(pAd) && (pMacEntry->wcid == BSSID_WCID) && 
		(pAd->StaCfg.PwrMgmt.bBeaconLost) && 
		OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		pAd->Mlme.ChannelQuality = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("%s::MT7636 BEACON lost meet\n", __FUNCTION__)); 
	}
	else
#endif		
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
								   RX_WEIGHTING* (100 - RxPER)) / 100;
	}
	}


#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		if (pMacEntry != NULL)
			pMacEntry->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
		pAd->Mlme.ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
	}
#endif /* CONFIG_AP_SUPPORT */

	
}


VOID MlmeSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/

	if (TxPreamble == Rt802_11PreambleLong)
	{
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED); 
	}
	else
	{
		/* NOTE: 1Mbps should always use long preamble*/
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble = %s PREAMBLE\n",
				((TxPreamble == Rt802_11PreambleLong) ? "LONG" : "SHORT")));
			
	AsicSetTxPreamble(pAd, TxPreamble);
}


/*
    ==========================================================================
    Description:
        Update basic rate bitmap
    ==========================================================================
*/
#ifdef DYNAMIC_RX_RATE_ADJ
VOID UpdateBasicRateBitmap(RTMP_ADAPTER *pAdapter, UCHAR apidx)
#else
VOID UpdateBasicRateBitmap(RTMP_ADAPTER *pAdapter)
#endif /* DYNAMIC_RX_RATE_ADJ */
{
    INT  i, j;
                  /* 1  2  5.5, 11,  6,  9, 12, 18, 24, 36, 48,  54 */
    UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
    UCHAR *sup_p = pAdapter->CommonCfg.SupRate;
    UCHAR *ext_p = pAdapter->CommonCfg.ExtRate;
    ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;

#ifdef DYNAMIC_RX_RATE_ADJ
	if (apidx < MIN_NET_DEVICE_FOR_WDS)
	{
		sup_p = pAdapter->ApCfg.MBSSID[apidx].SupRate;
		ext_p = pAdapter->ApCfg.MBSSID[apidx].ExtRate;
	}
#endif /* DYNAMIC_RX_RATE_ADJ */

    /* if A mode, always use fix BasicRateBitMap */
    /*if (pAdapter->CommonCfg.Channel == WMODE_A)*/
	if (pAdapter->CommonCfg.Channel > 14)
	{
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF)
		{
			/* no 11b rate in 5G band */
			pAdapter->CommonCfg.BasicRateBitmapOld = \
										pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in a-band */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
    }
	else
	{
		/* no need to modify in 2.4G (bg mixed) */
		pAdapter->CommonCfg.BasicRateBitmap = \
										pAdapter->CommonCfg.BasicRateBitmapOld;
	}
	bitmap = pAdapter->CommonCfg.BasicRateBitmap;  /* renew bitmap value */

    if (pAdapter->CommonCfg.BasicRateBitmap > 4095)
    {
        /* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
        return;
    }

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        sup_p[i] &= 0x7f;
        ext_p[i] &= 0x7f;
    }

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (bitmap & (1 << i))
        {
            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (sup_p[j] == rate[i])
                    sup_p[j] |= 0x80;
            }

            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (ext_p[j] == rate[i])
                    ext_p[j] |= 0x80;
            }
        }
    }
}


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


	/* find max desired rate*/
#ifdef DYNAMIC_RX_RATE_ADJ
	if (apidx < MIN_NET_DEVICE_FOR_WDS)
		UpdateBasicRateBitmap(pAd, apidx);

	UpdateBasicRateBitmap(pAd, 0xFF);
#else
	UpdateBasicRateBitmap(pAd);
#endif /* DYNAMIC_RX_RATE_ADJ */	
	
	num = 0;
	auto_rate_cur_p = NULL;
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		switch (pAd->CommonCfg.DesireRate[i] & 0x7f)
		{
			case 2:  Rate = RATE_1;   num++;   break;
			case 4:  Rate = RATE_2;   num++;   break;
			case 11: Rate = RATE_5_5; num++;   break;
			case 22: Rate = RATE_11;  num++;   break;
			case 12: Rate = RATE_6;   num++;   break;
			case 18: Rate = RATE_9;   num++;   break;
			case 24: Rate = RATE_12;  num++;   break;
			case 36: Rate = RATE_18;  num++;   break;
			case 48: Rate = RATE_24;  num++;   break;
			case 72: Rate = RATE_36;  num++;   break;
			case 96: Rate = RATE_48;  num++;   break;
			case 108: Rate = RATE_54; num++;   break;
			/*default: Rate = RATE_1;   break;*/
		}
		if (MaxDesire < Rate)  MaxDesire = Rate;
	}

/*===========================================================================*/
	do
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT	
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{			
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
			
			if (idx < MAX_APCLI_NUM)
			{
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid idx(%d)\n", __FUNCTION__, idx));
				return;
			}
		}
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{
				UCHAR idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY)
				{
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
					return;
				}
			}
#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) &&
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
			{
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

	} while(FALSE);

	if (wdev)
	{
		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;

		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
		HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	}

	pAd->CommonCfg.MaxDesiredRate = MaxDesire;

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
		pSupRate = &pAd->CommonCfg.SupRate[0];
		pExtRate = &pAd->CommonCfg.ExtRate[0];
		SupRateLen = pAd->CommonCfg.SupRateLen;
		ExtRateLen = pAd->CommonCfg.ExtRateLen;
	}

	/* find max supported rate */
	for (i=0; i<SupRateLen; i++)
	{
		switch (pSupRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;	break;
		}
		
		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}
	
	for (i=0; i<ExtRateLen; i++)
	{
		switch (pExtRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;
				break;
		}
		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	// TODO: shiang-7603, need to revise for MT7603!!
#if defined(MT7603) || defined(MT7628)
	if (IS_MT7603(pAd) || IS_MT7628(pAd))
		;
	else
#endif /* MT7603 */
	{
		RTMP_IO_WRITE32(pAd, LEGACY_BASIC_RATE, BasicRateBitmap);
	}

	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;
		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	DBGPRINT(RT_DEBUG_TRACE,("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
				__FUNCTION__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));
	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		pAd->CommonCfg.MaxTxRate = MaxSupport;
	else
		pAd->CommonCfg.MaxTxRate = MaxDesire;

	pAd->CommonCfg.MinTxRate = MinSupport;
	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p)
	{
		short dbm = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dbm =0;
#endif /* CONFIG_AP_SUPPORT */
		if (bLinkUp == TRUE)
			pAd->CommonCfg.TxRate = RATE_24;
		else
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate; 

		if (dbm < -75)
			pAd->CommonCfg.TxRate = RATE_11;
		else if (dbm < -70)
			pAd->CommonCfg.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (pAd->CommonCfg.TxRate > pAd->CommonCfg.MaxTxRate)
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate; 

		pAd->CommonCfg.TxRateIndex = 0;

	}
	else
	{
		pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6)
		{
			if (HtMcs <= MCS_7)		
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		}
		else
		{
			if (HtMcs <= MCS_3)		
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}
		
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC = pHtPhy->field.STBC;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI = pHtPhy->field.ShortGI;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS = pHtPhy->field.MCS;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE	= pHtPhy->field.MODE;

	}

	if (pAd->CommonCfg.TxRate <= RATE_11)
	{
		pMaxHtPhy->field.MODE = MODE_CCK;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */	

	}
	else
	{
		pMaxHtPhy->field.MODE = MODE_OFDM;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */				

	}

	pHtPhy->word = (pMaxHtPhy->word);
	if (bLinkUp && (pAd->OpMode == OPMODE_STA))
	{
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word = pHtPhy->word;
		pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word = pMaxHtPhy->word;
		pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word = pMinHtPhy->word;
	}
	else
	{
		if (WMODE_CAP(pAd->CommonCfg.PhyMode, WMODE_B) &&
			pAd->CommonCfg.Channel <= 14)
		{
			pAd->CommonCfg.MlmeRate = RATE_1;
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
			pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;				
			pAd->CommonCfg.RtsRate = RATE_11;
		}
		else
		{
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
			pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}
		
		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word = pAd->CommonCfg.MlmeTransmit.word;
		if (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_6];
		else
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = RATE_1;
		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;

#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */	
			HTTRANSMIT_SETTING tPhyMode;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
			if (memcmp(&pAd->CommonCfg.MCastPhyMode, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0)
			{
				memmove(&pAd->CommonCfg.MCastPhyMode, &pAd->MacTab.Content[MCAST_WCID].HTPhyMode,
							sizeof(HTTRANSMIT_SETTING));
			}
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	DBGPRINT(RT_DEBUG_TRACE, (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n", 
				__FUNCTION__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
				RateIdToMbps[pAd->CommonCfg.MaxTxRate],
				RateIdToMbps[pAd->CommonCfg.MinTxRate], 
			 /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	DBGPRINT(RT_DEBUG_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n", 
				__FUNCTION__, RateIdToMbps[pAd->CommonCfg.TxRate],
				RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): (MlmeTransmit=0x%x, MinHTPhyMode=%x, MaxHTPhyMode=0x%x, HTPhyMode=0x%x)\n", 
				__FUNCTION__, pAd->CommonCfg.MlmeTransmit.word,
				pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word,
				pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word,
				pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word ));
}


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		This function update HT Rate setting.
		Input Wcid value is valid for 2 case :
		1. it's used for Station in infra mode that copy AP rate to Mactable.
		2. OR Station 	in adhoc mode to copy peer's HT rate to Mactable. 

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, UCHAR apidx)
{
	//UCHAR StbcMcs;
	RT_HT_CAPABILITY *pRtHtCap = NULL;
	RT_PHY_INFO *pActiveHtPhy = NULL;	
	//ULONG BasicMCS;
	RT_PHY_INFO *pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING pHtPhy = NULL;
	PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING pMinHtPhy = NULL;	
	BOOLEAN *auto_rate_cur_p;
	struct wifi_dev *wdev = NULL;


	DBGPRINT(RT_DEBUG_TRACE,("%s()===> \n", __FUNCTION__));

	auto_rate_cur_p = NULL;

	do
	{
#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT	
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
		
			if (idx < MAX_APCLI_NUM)
			{
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid idx(%d)\n", __FUNCTION__, idx));
				return;
			}
		}
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;

			if (idx < MAX_WDS_ENTRY)
			{
				wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
				return;
			}
			}
#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) && (apidx < HW_BEACON_MAX_NUM))
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

	} while (FALSE);

	if (wdev)
	{
		pDesireHtPhy = &wdev->DesiredHtPhyInfo;
		pActiveHtPhy = &wdev->DesiredHtPhyInfo;
		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;		

		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	}

	{
		if ((!pDesireHtPhy) || pDesireHtPhy->bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->CommonCfg.DesiredHtPhy;
		//StbcMcs = (UCHAR)pAd->CommonCfg.AddHTInfo.AddHtInfo3.StbcMcs;
		//BasicMCS = pAd->CommonCfg.AddHTInfo.MCSSet[0]+(pAd->CommonCfg.AddHTInfo.MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pAd->Antenna.field.TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	/* Decide MAX ht rate.*/
	if ((pRtHtCap->GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

    if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth) && (pRtHtCap->ChannelWidth))
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

    if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 & pRtHtCap->ShortGIfor20);
	else
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 & pRtHtCap->ShortGIfor40);

	if (pDesireHtPhy->MCSSet[4] != 0)
	{
		pMaxHtPhy->field.MCS = 32;	
	}

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
	
	DBGPRINT(RT_DEBUG_TRACE, (" %s():<---.AMsduSize = %d  \n", __FUNCTION__, pAd->CommonCfg.DesiredHtPhy.AmsduSize ));
	DBGPRINT(RT_DEBUG_TRACE,("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,  \n", pActiveHtPhy->MCSSet[0],pHtPhy->field.MCS,
		pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	DBGPRINT(RT_DEBUG_TRACE,("%s():<=== \n", __FUNCTION__));
}


VOID BATableInit(RTMP_ADAPTER *pAd, BA_TABLE *Tab) 
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	Tab->numDoneOriginator = 0;
	NdisAllocateSpinLock(pAd, &pAd->BATabLock);
#ifdef CONFIG_BA_REORDER_MONITOR
#define REORDERING_PACKET_TIMEOUT		((100 * OS_HZ)/1000)	/* system ticks -- 100 ms*/
		pAd->BATable.ba_timeout_check = FALSE;
		pAd->BATable.ba_reordering_packet_timeout = REORDERING_PACKET_TIMEOUT;
		NdisZeroMemory((UCHAR *)&pAd->BATable.ba_timeout_bitmap[0], sizeof(UINT32) * 16);
#endif
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) 
	{
		Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(pAd, &(Tab->BARecEntry[i].RxReRingLock));
	}
	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++) 
	{
		Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
	}
}


VOID BATableExit(RTMP_ADAPTER *pAd)
{
	int i;
	
	for(i=0; i<MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		NdisFreeSpinLock(&pAd->BATable.BARecEntry[i].RxReRingLock);
	}
	NdisFreeSpinLock(&pAd->BATabLock);
}
#endif /* DOT11_N_SUPPORT */


VOID MlmeRadioOff(RTMP_ADAPTER *pAd)
{
#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */

#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
		PciMlmeRadioOFF(pAd);
#endif /* RTMP_MAC_PCI */
}


VOID MlmeRadioOn(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
		PciMlmeRadioOn(pAd);
#endif /* RTMP_MAC_PCI */
#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* LED_CONTROL_SUPPORT */

}


/*! \brief initialize BSS table
 *	\param p_tab pointer to the table
 *	\return none
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
VOID BssTableInit(BSS_TABLE *Tab) 
{
	int i;

	Tab->BssNr = 0;
	Tab->BssOverlapNr = 0;

	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
	{
		UCHAR *pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;

		NdisZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));

		Tab->BssEntry[i].Rssi = -127;	/* initial the rssi as a minimum value */
		if (pOldAddr)
		{
			RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
			Tab->BssEntry[i].pVarIeFromProbRsp = pOldAddr;
		}
	}
}


/*! \brief search the BSS table by SSID
 *	\param p_tab pointer to the bss table
 *	\param ssid SSID string 
 *	\return index of the table, BSS_NOT_FOUND if not in the table
 *	\pre
 *	\post
 *	\note search by sequential search

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSearch(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		
		/*
			Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
			We should distinguish this case.
		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel) 
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		
		/* Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.*/
		/* We should distinguish this case.*/
		/*		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			(NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) || 
			(NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearchBySSID(BSS_TABLE *Tab, UCHAR *pSsid, UCHAR SsidLen)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


VOID BssTableDeleteEntry(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i, j;
#ifdef WH_EZ_SETUP
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BssTableDeleteEntry called for : %02x-%02x-%02x-%02x-%02x-%02x, Channel=%d\n",
		pBssid[0],pBssid[1],pBssid[2],pBssid[3],pBssid[4],pBssid[5],Channel));
#endif

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if ((Tab->BssEntry[i].Channel == Channel) && 
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)))
		{
			UCHAR *pOldAddr = NULL;
			
			for (j = i; j < Tab->BssNr - 1; j++)
			{
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				NdisMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));
				if (pOldAddr)
				{
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr, 
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp, 
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}

			pOldAddr = Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp;
			NdisZeroMemory(&(Tab->BssEntry[Tab->BssNr - 1]), sizeof(BSS_ENTRY));
			if (pOldAddr)
			{
				RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
				Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp = pOldAddr;
			}
			
			Tab->BssNr -= 1;
			return;
		}
	}
}

#ifdef APCLI_OWE_SUPPORT
static VOID update_bss_by_owe_trans(struct _RTMP_ADAPTER *ad,
				    ULONG bss_idx,
				    UCHAR *pair_bssid,
				    UCHAR *pair_ssid,
				    UCHAR pair_ssid_len)
{
	BSS_ENTRY *extracted_trans_bss = &ad->ScanTab.BssEntry[bss_idx];

	if (MAC_ADDR_EQUAL(extracted_trans_bss->Bssid, pair_bssid)) {
		if (extracted_trans_bss->Hidden == 1) {
			/*double confirm the hidden bss is OWE AKM*/
			if (extracted_trans_bss->AuthMode != Ndis802_11AuthModeOWE)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : %02x-%02x-%02x-%02x-%02x-%02x, hidden SSID but not OWE_AKM:0x%x!?\n",
					__func__,
					PRINT_MAC(extracted_trans_bss->Bssid),
					extracted_trans_bss->AuthMode));

				extracted_trans_bss->hide_owe_bss = TRUE;

				extracted_trans_bss->bhas_owe_trans_ie = TRUE;


			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : %02x-%02x-%02x-%02x-%02x-%02x, update hidden SSID:%s\n",
					__func__,
					PRINT_MAC(extracted_trans_bss->Bssid),
					extracted_trans_bss->Ssid));
		}
	}
}
#endif


/*! \brief
 *	\param 
 *	\return
 *	\pre
 *	\post
 */
VOID BssEntrySet(
	IN RTMP_ADAPTER *pAd, 
	OUT BSS_ENTRY *pBss, 
	IN BCN_IE_LIST *ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,	
	IN PNDIS_802_11_VARIABLE_IEs pVIE 
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	,
	IN UCHAR	Snr0,
	IN UCHAR	Snr1
#endif					
	) 
{
	COPY_MAC_ADDR(pBss->Bssid, ie_list->Bssid);
	/* Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID*/
	pBss->Hidden = 1;
	pBss->FromBcnReport = ie_list->FromBcnReport;
	if (ie_list->SsidLen > 0)
	{
		/* For hidden SSID AP, it might send beacon with SSID len equal to 0*/
		/* Or send beacon /probe response with SSID len matching real SSID length,*/
		/* but SSID is all zero. such as "00-00-00-00" with length 4.*/
		/* We have to prevent this case overwrite correct table*/
		if (NdisEqualMemory(ie_list->Ssid, ZeroSsid, ie_list->SsidLen) == 0)
		{
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
			pBss->SsidLen = ie_list->SsidLen;
			pBss->Hidden = 0;
		}
	}
	else
	{
		/* avoid  Hidden SSID form beacon to overwirite correct SSID from probe response */
		if (NdisEqualMemory(pBss->Ssid, ZeroSsid, pBss->SsidLen))
		{
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			pBss->SsidLen = 0;
		}
	}
	
	pBss->BssType = ie_list->BssType;
	pBss->BeaconPeriod = ie_list->BeaconPeriod;
	if (ie_list->BssType == BSS_INFRA) 
	{
		if (ie_list->CfParm.bValid) 
		{
			pBss->CfpCount = ie_list->CfParm.CfpCount;
			pBss->CfpPeriod = ie_list->CfParm.CfpPeriod;
			pBss->CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pBss->CfpDurRemaining = ie_list->CfParm.CfpDurRemaining;
		}
	} 
	else 
	{
		pBss->AtimWin = ie_list->AtimWin;
	}

	NdisGetSystemUpTime(&pBss->LastBeaconRxTime);
	pBss->CapabilityInfo = ie_list->CapabilityInfo;
	/* The privacy bit indicate security is ON, it maight be WEP, TKIP or AES*/
	/* Combine with AuthMode, they will decide the connection methods.*/
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)		
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, ie_list->SupRateLen);
	else		
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, MAX_LEN_OF_SUPPORTED_RATES);	
	pBss->SupRateLen = ie_list->SupRateLen;
	ASSERT(ie_list->ExtRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (ie_list->ExtRateLen > MAX_LEN_OF_SUPPORTED_RATES)
		ie_list->ExtRateLen = MAX_LEN_OF_SUPPORTED_RATES;
	NdisMoveMemory(pBss->ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
	pBss->NewExtChanOffset = ie_list->NewExtChannelOffset;
	pBss->ExtRateLen = ie_list->ExtRateLen;
	pBss->Erp = ie_list-> Erp;
	pBss->Channel = ie_list->Channel;
	pBss->CentralChannel = ie_list->Channel;
	pBss->Rssi = Rssi;
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	pBss->Snr0 = Snr0;
	pBss->Snr1 = Snr1;
#endif
#ifdef NEIGHBORING_AP_STAT
	pBss->DtimPeriod = ie_list->DtimPeriod;
#endif
	/* Update CkipFlag. if not exists, the value is 0x0*/
	pBss->CkipFlag = ie_list->CkipFlag;

	/* New for microsoft Fixed IEs*/
	NdisMoveMemory(pBss->FixIEs.Timestamp, &ie_list->TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = ie_list->BeaconPeriod;
	pBss->FixIEs.Capabilities = ie_list->CapabilityInfo;
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	pBss->LastBeaconRxTime = jiffies_to_msecs(jiffies);
#endif

#ifdef APCLI_OWE_SUPPORT
	NdisZeroMemory(pBss->owe_trans_ie, MAX_VIE_LEN);
	pBss->owe_trans_ie_len = 0;
	pBss->bhas_owe_trans_ie = FALSE;
#endif

	/* New for microsoft Variable IEs*/
	if (LengthVIE != 0)
	{
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	}
	else
	{
		pBss->VarIELen = 0;
	}


	pBss->AddHtInfoLen = 0;
	pBss->HtCapabilityLen = 0;
#ifdef DOT11_N_SUPPORT
	if (ie_list->HtCapabilityLen> 0)
	{
		pBss->HtCapabilityLen = ie_list->HtCapabilityLen;
		NdisMoveMemory(&pBss->HtCapability, &ie_list->HtCapability, ie_list->HtCapabilityLen);
		if (ie_list->AddHtInfoLen > 0)
		{
			pBss->AddHtInfoLen = ie_list->AddHtInfoLen;
			NdisMoveMemory(&pBss->AddHtInfo, &ie_list->AddHtInfo, ie_list->AddHtInfoLen);

			pBss->CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
											&ie_list->HtCapability);
		}

#ifdef DOT11_VHT_AC
		if (ie_list->vht_cap_len) {
			NdisMoveMemory(&pBss->vht_cap_ie, &ie_list->vht_cap_ie, ie_list->vht_cap_len);
			pBss->vht_cap_len = ie_list->vht_cap_len;
		}
		
		if (ie_list->vht_op_len) {
			VHT_OP_IE *vht_op;
					
			NdisMoveMemory(&pBss->vht_op_ie, &ie_list->vht_op_ie, ie_list->vht_op_len);
			pBss->vht_op_len = ie_list->vht_op_len;
			vht_op = &ie_list->vht_op_ie;
			if ((vht_op->vht_op_info.ch_width > 0) &&
				(ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
				(ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
				(pBss->CentralChannel != ie_list->AddHtInfo.ControlChan))
			{
				UCHAR cent_ch;
				
				cent_ch = vht_cent_ch_freq(pAd, ie_list->AddHtInfo.ControlChan);
				DBGPRINT(RT_DEBUG_TRACE, ("%s():VHT cent_ch=%d, vht_op_info->center_freq_1=%d, Bss->CentCh=%d, change from CentralChannel to cent_ch!\n",
											__FUNCTION__, cent_ch, vht_op->vht_op_info.center_freq_1, pBss->CentralChannel));
				pBss->CentralChannel = vht_op->vht_op_info.center_freq_1;
			}
		}
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

	BssCipherParse(pBss);

	/* new for QOS*/
	if (ie_list->EdcaParm.bValid)
		NdisMoveMemory(&pBss->EdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;
	if (ie_list->QosCapability.bValid)
		NdisMoveMemory(&pBss->QosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;
	if (ie_list->QbssLoad.bValid)
		NdisMoveMemory(&pBss->QbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

#ifdef WH_EZ_SETUP // Rakesh: recognize an easy enabled network only if enabled on own device too, acceptable??
		//! differentiate between EZ and NON Ez beacons from other triband repeaters
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
			if (ie_list->vendor_ie.support_easy_setup && !(ie_list->vendor_ie.non_ez_beacon))
			{
				pBss->support_easy_setup = 1;
				pBss->easy_setup_capability = ie_list->vendor_ie.ez_capability;
			}		
			else 
			{
				//! if it is a non ez beacon, disable EZ setup from that BSS
				pBss->non_ez_beacon = ie_list->vendor_ie.non_ez_beacon;
				pBss->support_easy_setup = 0;
				pBss->easy_setup_capability = 0;
			}
#ifdef NEW_CONNECTION_ALGO
			ez_update_bss_entry(pBss, ie_list);
#endif
		}
#endif /* WH_EZ_SETUP */

	{
		PEID_STRUCT pEid;
		USHORT Length = 0;

#ifdef WSC_INCLUDED
		pBss->WpsAP = 0x00;
		pBss->WscDPIDFromWpsAP = 0xFFFF;
#endif /* WSC_INCLUDED */

		pEid = (PEID_STRUCT) pVIE;
		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE)    
		{
#define WPS_AP		0x01
			switch(pEid->Eid)
			{
				case IE_WPA:
					if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
						)
					{
#ifdef WSC_INCLUDED
						pBss->WpsAP |= WPS_AP;
						WscCheckWpsIeFromWpsAP(pAd, 
													pEid, 
													&pBss->WscDPIDFromWpsAP);
#endif /* WSC_INCLUDED */
						break;
					}
#ifdef MWDS
					check_vendor_ie(pAd, (UCHAR *)pEid, &(ie_list->vendor_ie));
					if(ie_list->vendor_ie.mtk_cap_found)
					{
						if(ie_list->vendor_ie.support_mwds)
							pBss->bSupportMWDS = TRUE;
						else
							pBss->bSupportMWDS = FALSE;
					}
#endif /* MWDS */
#ifdef APCLI_OWE_SUPPORT
					if (NdisEqualMemory(pEid->Octet, OWE_TRANS_OUI, 4)) {
						ULONG bss_idx = BSS_NOT_FOUND;
						UCHAR pair_ch = 0;
						UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
						UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
						UCHAR pair_band = 0;
						UCHAR pair_ssid_len = 0;

						NdisZeroMemory(pBss->owe_trans_ie, MAX_VIE_LEN);
						NdisMoveMemory(pBss->owe_trans_ie, pEid->Octet + 4, pEid->Len - 4);
						pBss->owe_trans_ie_len = pEid->Len - 4;

						pBss->bhas_owe_trans_ie = TRUE;

						extract_pair_owe_bss_info(pEid->Octet + 4,
									  pEid->Len - 4,
									  pair_bssid,
									  pair_ssid,
									  &pair_ssid_len,
									  &pair_band,
									  &pair_ch);
						if (pair_ch != 0)
							bss_idx = BssTableSearch(&pAd->ScanTab, pair_bssid, pair_ch);
						else
							bss_idx = BssTableSearch(&pAd->ScanTab,
										pair_bssid, ie_list->Channel);

						if (bss_idx != BSS_NOT_FOUND)
							update_bss_by_owe_trans(pAd,
										bss_idx,
										pair_bssid,
										pair_ssid,
										pair_ssid_len);
					}
#endif
					break;

			}
			Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
		}
	}
}



/*! 
 *	\brief insert an entry into the bss table
 *	\param p_tab The BSS table
 *	\param Bssid BSSID
 *	\param ssid SSID
 *	\param ssid_len Length of SSID
 *	\param bss_type
 *	\param beacon_period
 *	\param timestamp
 *	\param p_cf
 *	\param atim_win
 *	\param cap
 *	\param rates
 *	\param rates_len
 *	\param channel_idx
 *	\return none
 *	\pre
 *	\post
 *	\note If SSID is identical, the old entry will be replaced by the new one
	 
 IRQL = DISPATCH_LEVEL
 
 */
ULONG BssTableSetEntry(
	IN PRTMP_ADAPTER pAd,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST *ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,	
	IN PNDIS_802_11_VARIABLE_IEs pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	,
	IN UCHAR	Snr0,
	IN UCHAR	Snr1
#endif
	)
{
	ULONG	Idx;
#ifdef APCLI_SUPPORT
	BOOLEAN bInsert = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	UCHAR i;
#endif /* APCLI_SUPPORT */


	Idx = BssTableSearch(Tab, ie_list->Bssid, ie_list->Channel);
	if (Idx == BSS_NOT_FOUND) 
	{
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE)
	    {
			/*
				It may happen when BSS Table was full. 
				The desired AP will not be added into BSS Table
				In this case, if we found the desired AP then overwrite BSS Table.
			*/
#ifdef APCLI_SUPPORT
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++)
			{
				pApCliEntry = &pAd->ApCfg.ApCliTab[i];
				if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid)
					|| SSID_EQUAL(pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen, ie_list->Ssid, ie_list->SsidLen))
				{
					bInsert = TRUE;
					break;
				}
			}
#endif /* APCLI_SUPPORT */
			if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) ||
				!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
			{
				if (MAC_ADDR_EQUAL(pAd->ScanCtrl.Bssid, ie_list->Bssid) ||
					SSID_EQUAL(pAd->ScanCtrl.Ssid, pAd->ScanCtrl.SsidLen, ie_list->Ssid, ie_list->SsidLen)
#ifdef APCLI_SUPPORT
					|| bInsert
#endif /* APCLI_SUPPORT */
					)
				{
					Idx = Tab->BssOverlapNr;
					NdisZeroMemory(&(Tab->BssEntry[Idx]), sizeof(BSS_ENTRY));
					BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
							, Snr0, Snr1
#endif
										);
					Tab->BssOverlapNr += 1;
					Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE;
				}
				return Idx;
			}
			else
			{
				return BSS_NOT_FOUND;
			}
		}
		Idx = Tab->BssNr;
		BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
					, Snr0, Snr1
#endif
								);
		Tab->BssNr++;
	} 
	else
	{
			BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
						, Snr0, Snr1
#endif
									);
	}

	return Idx;
}



#if (defined(CONFIG_STA_SUPPORT) || defined(WH_EZ_SETUP))
UCHAR wmode_2_rfic(UCHAR PhyMode)
{
        if(WMODE_CAP_2G(PhyMode) && WMODE_CAP_5G(PhyMode))
        {
                return RFIC_DUAL_BAND;
        }else
        if(WMODE_CAP_2G(PhyMode))
        {
                return RFIC_24GHZ;
        }else
        if(WMODE_CAP_5G(PhyMode))
        {
                return RFIC_5GHZ;
        }
        return RFIC_24GHZ;
}
//#ifdef WH_EZ_SETUP
VOID BssTableSsidSort(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,	
	OUT BSS_TABLE *OutTab, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen) 
{
	INT i;
#ifdef WSC_STA_SUPPORT
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
#endif /* WSC_STA_SUPPORT */
	//struct wifi_dev *wdev = &pAd->StaCfg.wdev;
#ifdef WH_EZ_SETUP
	UCHAR RfIC;
	UCHAR BssType;
#endif

	BssTableInit(OutTab);

#ifdef WH_EZ_SETUP
	BssType = BSS_INFRA;
#endif


#ifdef WH_EZ_SETUP
	RfIC = wmode_2_rfic(wdev->PhyMode);
#endif
	for (i = 0; i < pAd->ScanTab.BssNr; i++) 
	{
		BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
		BOOLEAN	bIsHiddenApIncluded = FALSE;

#ifdef WH_EZ_SETUP
#ifdef NEW_CONNECTION_ALGO
	if(IS_EZ_SETUP_ENABLED(wdev))
	{
		if (ez_is_weight_same(wdev,pInBss->beacon_info.network_weight))
		{
			continue;
		}
	}

#endif
#endif
		
		if (((pAd->CommonCfg.bIEEE80211H == 1) && 
#ifdef WH_EZ_SETUP
			(RfIC & RFIC_5GHZ) &&
#else		
			(pAd->MlmeAux.Channel > 14) && 
#endif				
			 RadarChannelCheck(pAd, pInBss->Channel))
#ifdef WIFI_REGION32_HIDDEN_SSID_SUPPORT
			 ||((pInBss->Channel == 12) || (pInBss->Channel == 13))
#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier             */
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
            )
		{
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
		}            
#if(defined(DBDC_MODE) && defined(WH_EZ_SETUP))
		if( (pAd->CommonCfg.dbdc_mode) && (IS_EZ_SETUP_ENABLED(wdev)) ){
			if( ( WMODE_2G_ONLY(wdev->PhyMode) && (pInBss->Channel <= 14) ) ||
				( WMODE_5G_ONLY(wdev->PhyMode) && (pInBss->Channel > 14) ) )
			{
				// Own band network
			}
			else{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Ignore Other Band network.\n"));
				continue;
			}
		}
#endif


		if (
#ifdef WH_EZ_SETUP
			(pInBss->BssType == BssType) &&
#else
			(pInBss->BssType == pAd->StaCfg.BssType) && 
#endif			
			(SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen) || bIsHiddenApIncluded))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				(WMODE_HT_ONLY(pAd->CommonCfg.PhyMode)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((pAd->CommonCfg.PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */



			/* New for WPA2*/
			/* Check the Authmode first*/
			if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
			{
				/* Check AuthMode and AuthModeAux for matching, in case AP support dual-mode*/
				if ((wdev->AuthMode != pInBss->AuthMode) && (wdev->AuthMode != pInBss->AuthModeAux))
					/* None matched*/
					continue;
				
				/* Check cipher suite, AP must have more secured cipher than station setting*/
				if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA.bMixMode == FALSE)
						if (wdev->WepStatus != pInBss->WPA.GroupCipher)
							continue;
						
					/* check group cipher*/
					if ((wdev->WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP40Enabled) && 
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP104Enabled))
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
						(wdev->WepStatus != pInBss->WPA.PairCipher) && 
						(wdev->WepStatus != pInBss->WPA.PairCipherAux))
						continue;						
				}
				else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA2.bMixMode == FALSE)
						if (wdev->WepStatus != pInBss->WPA2.GroupCipher)
							continue;
						
					/* check group cipher*/
					if ((wdev->WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP40Enabled) && 
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP104Enabled))
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
						(wdev->WepStatus != pInBss->WPA2.PairCipher) && 
						(wdev->WepStatus != pInBss->WPA2.PairCipherAux))
						continue;						
				}
#ifdef WAPI_SUPPORT
				else if ((wdev->AuthMode == Ndis802_11AuthModeWAICERT) || (wdev->AuthMode == Ndis802_11AuthModeWAIPSK))
				{					
					/* check cipher algorithm*/
					if ((wdev->WepStatus != pInBss->WAPI.GroupCipher) || 
						(wdev->WepStatus != pInBss->WAPI.PairCipher))
						continue;											
				}
#endif /* WAPI_SUPPORT */
			}			
			/* Bss Type matched, SSID matched. */
			/* We will check wepstatus for qualification Bss*/
			else if (wdev->WepStatus != pInBss->WepStatus)
			{
				DBGPRINT(RT_DEBUG_TRACE,("StaCfg.WepStatus=%d, while pInBss->WepStatus=%d\n", wdev->WepStatus, pInBss->WepStatus));
				
				/* For the SESv2 case, we will not qualify WepStatus.*/
				
				if (!pInBss->bSES)
					continue;
			}

			/* Since the AP is using hidden SSID, and we are trying to connect to ANY*/
			/* It definitely will fail. So, skip it.*/
			/* CCX also require not even try to connect it!!*/
			if (SsidLen == 0)
				continue;
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
		else if (
#ifdef WH_EZ_SETUP
				(pInBss->BssType == BssType) && 
#else
				(pInBss->BssType == pAd->StaCfg.BssType) && 
#endif
				(SsidLen == 0))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

#ifdef WSC_STA_SUPPORT
			if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && pWpsCtrl->bWscTrigger)
			{
				/* copy matching BSS from InTab to OutTab*/
				NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
				OutTab->BssNr++;
				continue;
			}
#endif /* WSC_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				WMODE_HT_ONLY(pAd->CommonCfg.PhyMode))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((pAd->CommonCfg.PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */

			/* New for WPA2*/
			/* Check the Authmode first*/
			if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
			{
				/* Check AuthMode and AuthModeAux for matching, in case AP support dual-mode*/
				if ((wdev->AuthMode != pInBss->AuthMode) && (wdev->AuthMode != pInBss->AuthModeAux))
					/* None matched*/
					continue;
				
				/* Check cipher suite, AP must have more secured cipher than station setting*/
				if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA.bMixMode == FALSE)
						if (wdev->WepStatus != pInBss->WPA.GroupCipher)
							continue;
						
					/* check group cipher*/
					if (wdev->WepStatus < pInBss->WPA.GroupCipher)
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
						(wdev->WepStatus != pInBss->WPA.PairCipher) && 
						(wdev->WepStatus != pInBss->WPA.PairCipherAux))
						continue;						
				}
				else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA2.bMixMode == FALSE)
						if (wdev->WepStatus != pInBss->WPA2.GroupCipher)
							continue;
						
					/* check group cipher*/
					if (wdev->WepStatus < pInBss->WPA2.GroupCipher)
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
						(wdev->WepStatus != pInBss->WPA2.PairCipher) && 
						(wdev->WepStatus != pInBss->WPA2.PairCipherAux))
						continue;						
				}
#ifdef WAPI_SUPPORT
				else if ((wdev->AuthMode == Ndis802_11AuthModeWAICERT) || (wdev->AuthMode == Ndis802_11AuthModeWAIPSK))
				{					
					/* check cipher algorithm*/
					if ((wdev->WepStatus != pInBss->WAPI.GroupCipher) || 
						(wdev->WepStatus != pInBss->WAPI.PairCipher))
						continue;											
				}
#endif /* WAPI_SUPPORT */				
			}
			/* Bss Type matched, SSID matched. */
			/* We will check wepstatus for qualification Bss*/
			else if (wdev->WepStatus != pInBss->WepStatus)
					continue;
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
#ifdef WSC_STA_SUPPORT		
		else if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && 
				 (pWpsCtrl->bWscTrigger) &&
				 MAC_ADDR_EQUAL(pWpsCtrl->WscBssid, pInBss->Bssid))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			/*
				Linksys WRT610N WPS AP will change the SSID from linksys to linksys_WPS_<four random characters> 
				when the Linksys WRT610N is in the state 'WPS Unconfigured' after set to factory default.
			*/
			NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->MlmeAux.Ssid, pInBss->Ssid, pInBss->SsidLen);
			pAd->MlmeAux.SsidLen = pInBss->SsidLen;

			
			/* Update Reconnect Ssid, that user desired to connect.*/
			
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
			pAd->MlmeAux.AutoReconnectSsidLen = pAd->MlmeAux.SsidLen;

			OutTab->BssNr++;
			continue;
		}
#endif /* WSC_STA_SUPPORT */

		if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			break;
	}

	BssTableSortByRssi(OutTab, FALSE);
}

#endif
/*#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_AUTO_CONNECT_SUPPORT) || defined(WH_EZ_SETUP)*/
/* IRQL = DISPATCH_LEVEL*/
VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder)
{
	INT i, j;
	BSS_ENTRY *pTmpBss = NULL;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pTmpBss, sizeof(BSS_ENTRY));
	if (pTmpBss == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < OutTab->BssNr - 1; i++) 
	{
		for (j = i+1; j < OutTab->BssNr; j++) 
		{
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi ?
				!isInverseOrder : isInverseOrder)
			{
				if (OutTab->BssEntry[j].Rssi != OutTab->BssEntry[i].Rssi )
				{
					NdisMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
				}
			}
		}
	}

	if (pTmpBss != NULL)
		os_free_mem(NULL, pTmpBss);
}
/*#endif  defined(CONFIG_STA_SUPPORT) || defined(APCLI_AUTO_CONNECT_SUPPORT) */

VOID BssCipherParse(BSS_ENTRY *pBss)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	INT								Length;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
	UCHAR end_field = 0;
	UCHAR res = TRUE;
#endif

	
	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	
	if (pBss->Privacy)
	{
		pBss->WepStatus 	= Ndis802_11WEPEnabled;
	}
	else
	{
		pBss->WepStatus 	= Ndis802_11WEPDisabled;
	}
	/* Set default to disable & open authentication before parsing variable IE*/
	pBss->AuthMode		= Ndis802_11AuthModeOpen;
	pBss->AuthModeAux	= Ndis802_11AuthModeOpen;
#ifdef DOT11W_PMF_SUPPORT
        pBss->IsSupportSHA256KeyDerivation = FALSE;
#endif /* DOT11W_PMF_SUPPORT */            

	/* Init WPA setting*/
	pBss->WPA.PairCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA.GroupCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.RsnCapability = 0;
	pBss->WPA.bMixMode		= FALSE;

	/* Init WPA2 setting*/
	pBss->WPA2.PairCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA2.GroupCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.RsnCapability = 0;
	pBss->WPA2.bMixMode 	 = FALSE;

#ifdef WAPI_SUPPORT
	/* Init WAPI setting*/
	pBss->WAPI.PairCipher	 = Ndis802_11WEPDisabled;
	pBss->WAPI.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WAPI.GroupCipher	 = Ndis802_11WEPDisabled;
	pBss->WAPI.RsnCapability = 0;
	pBss->WAPI.bMixMode 	 = FALSE;
#endif /* WAPI_SUPPORT */
	
	Length = (INT) pBss->VarIELen;

	while (Length > 0)
	{
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - Length;
		pEid = (PEID_STRUCT) pTmp;
		switch (pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
				{
					pBss->bSES = TRUE;
					break;
				}				
				else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					/* if unsupported vendor specific IE*/
					break;
				}				
				/*
					Skip OUI, version, and multicast suite
					This part should be improved in the future when AP supported multiple cipher suite.
					For now, it's OK since almost all APs have fixed cipher suite supported.
				*/
				/* pTmp = (PUCHAR) pEid->Octet;*/
				pTmp   += 11;

				/* 
					Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
					Value	   Meaning
					0			None
					1			WEP-40
					2			Tkip
					3			WRAP
					4			AES
					5			WEP-104
				*/
				/* Parse group cipher*/
				switch (*pTmp)
				{
					case 1:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						pBss->WPA.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}
				/* number of unicast suite*/
				pTmp   += 1;

				/* skip all unicast cipher suites*/
				/*Count = *(PUSHORT) pTmp;				*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WPA.PairCipherAux = pBss->WPA.PairCipher;
						pBss->WPA.PairCipher	= TmpCipher;
					}
					else
					{
						pBss->WPA.PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}
				
				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				pTmp   += 3;
				
				switch (*pTmp)
				{
					case 1:
						/* Set AP support WPA-enterprise mode*/
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPA;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPA;
						break;
					case 2:
						/* Set AP support WPA-PSK mode*/
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPAPSK;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPAPSK;
						break;
					default:
						break;
				}
				pTmp   += 1;

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->AuthMode	  = Ndis802_11AuthModeWPANone;
					pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
					pBss->WepStatus   = pBss->WPA.GroupCipher;
					/* Patched bugs for old driver*/
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				else
					pBss->WepStatus   = pBss->WPA.PairCipher;					
				
				/* Check the Pair & Group, if different, turn on mixed mode flag*/
				if (pBss->WPA.GroupCipher != pBss->WPA.PairCipher)
					pBss->WPA.bMixMode = TRUE;
				
				break;

			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				res = wpa_rsne_sanity(pTmp, le2cpu16(pRsnHeader->Length) + 2, &end_field);
				if (res == FALSE)
					break;

				if (end_field < RSN_FIELD_GROUP_CIPHER)
					pBss->WPA2.GroupCipher = Ndis802_11AESEnable;
				if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
					pBss->WPA2.GroupCipher = Ndis802_11AESEnable;
				if (end_field < RSN_FIELD_AKM)
					pBss->AuthMode = Ndis802_11AuthModeWPA2PSK;
#endif
				/* 0. Version must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Check group cipher*/
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_GROUP_CIPHER)
					break;
#endif
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA2.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						pBss->WPA2.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);

				/* 2. Get pairwise cipher counts*/
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
					break;
#endif
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);			

				/* 3. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (pCipher->Type)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA2.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WPA2.PairCipherAux = pBss->WPA2.PairCipher;
						pBss->WPA2.PairCipher	 = TmpCipher;
					}
					else
					{
						pBss->WPA2.PairCipherAux = TmpCipher;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}
				
				/* 4. get AKM suite counts*/
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_AKM)
					break;
#endif
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* 5. Get AKM ciphers*/
				/* Parsing all AKM ciphers*/
				while (Count > 0)
				{					
					pAKM = (PAKM_SUITE_STRUCT) pTmp;
					if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
						break;

					switch (pAKM->Type)
					{
						case 0:
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPANone;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
							break;                                                        
						case 1:
#ifdef DOT11R_FT_SUPPORT
						case 3:
#endif /* DOT11R_FT_SUPPORT */
							/* Set AP support WPA-enterprise mode*/
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPA2;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPA2;
							break;
						case 2:
#ifdef DOT11R_FT_SUPPORT
						case 4:
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
						case 6:
#endif /* DOT11W_PMF_SUPPORT */                                                        
							/* Set AP support WPA-PSK mode*/
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPA2PSK;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPA2PSK;

#ifdef DOT11W_PMF_SUPPORT
                                                        if (pAKM->Type == 6)
                                                                pBss->IsSupportSHA256KeyDerivation = TRUE;
#endif /* DOT11W_PMF_SUPPORT */

							break;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
						case 8:
							pBss->AuthMode = Ndis802_11AuthModeWPA3PSK;
							break;

						case 18:
							pBss->AuthMode = Ndis802_11AuthModeOWE;
							break;
#endif

						default:
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeMax;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeMax;
							break;
					}
					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->WPA.PairCipherAux = pBss->WPA2.PairCipherAux;
					pBss->WPA.GroupCipher	= pBss->WPA2.GroupCipher;
					pBss->WepStatus 		= pBss->WPA.GroupCipher;
					/* Patched bugs for old driver*/
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				pBss->WepStatus   = pBss->WPA2.PairCipher;					
				
				/* 6. Get RSN capability*/
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_RSN_CAP)
					break;
#endif
				/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
				pBss->WPA2.RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);
				
				/* Check the Pair & Group, if different, turn on mixed mode flag*/
				if (pBss->WPA2.GroupCipher != pBss->WPA2.PairCipher)
					pBss->WPA2.bMixMode = TRUE;
				
				break;
#ifdef WAPI_SUPPORT
			case IE_WAPI:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				/* 0. The version number must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Get AKM suite counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));	
    			Count = cpu2le16(Count);				
				pTmp += sizeof(USHORT);

				/* 2. Get AKM ciphers*/
				pAKM = (PAKM_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;

				switch (pAKM->Type)
				{					
					case 1:
						/* Support WAI certificate authentication*/
						pBss->AuthMode = Ndis802_11AuthModeWAICERT;						
						break;
					case 2:
						/* Support WAI PSK*/
						pBss->AuthMode = Ndis802_11AuthModeWAIPSK;						
						break;
					default:
						break;
				}
				pTmp += (Count * sizeof(AKM_SUITE_STRUCT));

				/* 3. Get pairwise cipher counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));	
    			Count = cpu2le16(Count);	
				pTmp += sizeof(USHORT);			

				/* 4. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
						break;
				
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;					
					TmpCipher = Ndis802_11WEPDisabled;
					switch (pCipher->Type)
					{
						case 1:						
							TmpCipher = Ndis802_11EncryptionSMS4Enabled;
							break;
						default:
							break;
					}
					
					if (TmpCipher > pBss->WAPI.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WAPI.PairCipherAux = pBss->WAPI.PairCipher;
						pBss->WAPI.PairCipher	 = TmpCipher;
					}
					else
					{
						pBss->WAPI.PairCipherAux = TmpCipher;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}
				
				/* 5. Check group cipher*/
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;
				
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;				
				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						pBss->WAPI.GroupCipher = Ndis802_11EncryptionSMS4Enabled;
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp += sizeof(CIPHER_SUITE_STRUCT);

				/* update the encryption type*/
				pBss->WepStatus = pBss->WAPI.PairCipher;

				/* update the WAPI capability*/
				pBss->WAPI.RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);

				break;
#endif /* WAPI_SUPPORT */				
			default:
				break;
		}
		Length -= (pEid->Len + 2);
	}
}


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
	{
		pAddr[i] = RandomByte(pAd);
	}

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
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
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
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
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
 *	\pre the station has the following information in the pAd->StaCfg
 *	 - bssid
 *	 - station address
 *	\post
 *	\note this function initializes the following field
 */
VOID MgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd, 
	INOUT HEADER_802_11 *pHdr80211, 
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
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
#endif /* CONFIG_AP_SUPPORT */
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
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);
	do 
	{
		leng = va_arg(Args, int);
		if (leng == END_OF_ARGS) 
		{
			break;
		}
		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

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
NDIS_STATUS MlmeQueueInit(
	IN PRTMP_ADAPTER pAd,
#ifdef EAPOL_QUEUE_SUPPORT
	IN EAP_MLME_QUEUE *EAP_Queue,
#endif /* EAPOL_QUEUE_SUPPORT */
	IN MLME_QUEUE *Queue)
{
	INT i;

	NdisAllocateSpinLock(pAd, &Queue->Lock);

	Queue->Num	= 0;
	Queue->Head = 0;
	Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++) 
	{
		Queue->Entry[i].Occupied = FALSE;
		Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
	}

#ifdef EAPOL_QUEUE_SUPPORT
	NdisAllocateSpinLock(pAd, &EAP_Queue->Lock);

	EAP_Queue->Num = 0;
	EAP_Queue->Head = 0;
	EAP_Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_EAP_QUEUE; i++) 
	{
		EAP_Queue->Entry[i].Occupied = FALSE;
		EAP_Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(EAP_Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
	}
#endif /* EAPOL_QUEUE_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


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
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("MlmeEnqueue: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}
	
	if (MlmeQueueFull(Queue, 1)) 
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
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
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}
		
	NdisReleaseSpinLock(&(Queue->Lock));

	return TRUE;
}



#ifdef EAPOL_QUEUE_SUPPORT
BOOLEAN EAPMlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv)
{
	INT Tail;
	EAP_MLME_QUEUE	*Queue = (EAP_MLME_QUEUE *)&pAd->Mlme.EAP_Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("%s: msg too large, size = %ld \n", __FUNCTION__, MsgLen));
		return FALSE;
	}
	
	if (EAPMlmeQueueFull(Queue)) 
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_EAP_QUEUE) 
		Queue->Tail = 0;
	
	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;	
	Queue->Entry[Tail].Priv = Priv;
	
	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}
		
	NdisReleaseSpinLock(&(Queue->Lock));

	return TRUE;
}
#endif /* EAPOL_QUEUE_SUPPORT */


/*! \brief	 This function is used when Recv gets a MLME message
 *	\param	*Queue			 The MLME Queue
 *	\param	 TimeStampHigh	 The upper 32 bit of timestamp
 *	\param	 TimeStampLow	 The lower 32 bit of timestamp
 *	\param	 Rssi			 The receiving RSSI strength
 *	\param	 MsgLen 		 The length of the message
 *	\param	*Msg			 The message pointer
 *	\return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *	\pre
 *	\post
 */
BOOLEAN MlmeEnqueueForRecv(
	IN RTMP_ADAPTER *pAd, 
	IN ULONG Wcid, 
	IN struct raw_rssi_info *rssi_info,
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	IN UCHAR Snr0,
	IN UCHAR Snr1,
#endif
	IN ULONG MsgLen, 
	IN VOID *Msg,
	IN UCHAR OpMode)
{
	INT Tail, Machine = 0xff;
	UINT32 TimeStampHigh, TimeStampLow;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT MsgType = 0x0;

#ifdef EAPOL_QUEUE_SUPPORT
	EAP_MLME_QUEUE	*EAP_Queue = (EAP_MLME_QUEUE *)&pAd->Mlme.EAP_Queue;
#endif /* EAPOL_QUEUE_SUPPORT */

	MLME_QUEUE *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef APCLI_SUPPORT
	UCHAR ApCliIdx = 0;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0;
#endif /* MAC_REPEATER_SUPPORT */

	if (!pFrame) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s() has NULL parameter: Msg\n", __func__));
		return FALSE;
	}

#ifdef CONFIG_ATE			
	/* Nothing to do in ATE mode */
	if(ATE_ON(pAd))
		return FALSE;
#endif /* CONFIG_ATE */

	/*
		Do nothing if the driver is starting halt state.
		This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		DBGPRINT_ERR(("%s(): fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("%s(): frame too large, size = %ld \n", __FUNCTION__, MsgLen));
		return FALSE;
	}

#ifdef EAPOL_QUEUE_SUPPORT
	if (MlmeQueueFull(Queue, 0) && EAPMlmeQueueFull(EAP_Queue)) 
#else /* EAPOL_QUEUE_SUPPORT */
	if (MlmeQueueFull(Queue, 0)) 
#endif /* !EAPOL_QUEUE_SUPPORT */
	{
		RTMP_MLME_HANDLER(pAd);

		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

#ifdef APCLI_SUPPORT
		/*
			Beacon must be handled by ap-sync state machine.
			Probe-rsp must be handled by apcli-sync state machine.
			Those packets don't need to check its MAC address
		*/
		do
		{
			BOOLEAN bToApCli = FALSE;
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
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].MlmeAux.Bssid, pFrame->Hdr.Addr2)
#ifdef MULTI_APCLI_SUPPORT
					|| MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].wdev.if_addr, pFrame->Hdr.Addr1)
#endif /* MULTI_APCLI_SUPPORT */
					
					)
				{
#ifdef MULTI_APCLI_SUPPORT
					ApCliIdx  = i;
#endif /* MULTI_APCLI_SUPPORT */
					bToApCli = TRUE;
					break;
				}
			}

			if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr1, pAd->CurrentAddress) &&
				preCheckMsgTypeSubset(pAd, pFrame, &Machine, &MsgType))
				break;

			if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr1, pAd->CurrentAddress) && bToApCli)
			{
				if (ApCliMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
					break;
			}
#ifndef WH_EZ_SETUP // make it same as 7615 handling.
			else
#endif
			{
				if (APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
					break;
			}

			DBGPRINT_ERR(("%s(): un-recongnized mgmt->subtype=%d, STA-%02x:%02x:%02x:%02x:%02x:%02x\n", 
						__FUNCTION__, pFrame->Hdr.FC.SubType, PRINT_MAC(pFrame->Hdr.Addr2)));
			return FALSE;

		} while (FALSE);
#else
		if (!APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
		{
			DBGPRINT_ERR(("%s(): un-recongnized mgmt->subtype=%d\n",
							__FUNCTION__, pFrame->Hdr.FC.SubType));
			return FALSE;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */	

	TimeStampHigh = TimeStampLow = 0;
#ifdef RTMP_MAC_PCI
	if (!IS_USB_INF(pAd))
		AsicGetTsfTime(pAd, &TimeStampHigh, &TimeStampLow);
#endif /* RTMP_MAC_PCI */


#ifdef EAPOL_QUEUE_SUPPORT
	if (Machine == WPA_STATE_MACHINE)
	{

		if (EAPMlmeQueueFull(EAP_Queue)) 
		{
			RTMP_MLME_HANDLER(pAd);
			return FALSE;
		}
		
		/* OK, we got all the informations, it is time to put things into queue*/
		NdisAcquireSpinLock(&(EAP_Queue->Lock));
		Tail = EAP_Queue->Tail;
		/*
			Double check for safety in multi-thread system.
		*/
		if (EAP_Queue->Entry[Tail].Occupied)
		{
			NdisReleaseSpinLock(&(EAP_Queue->Lock));
			return FALSE;
		}
		EAP_Queue->Tail++;
		EAP_Queue->Num++;
		if (EAP_Queue->Tail == MAX_LEN_OF_EAP_QUEUE) 
			EAP_Queue->Tail = 0;

		EAP_Queue->Entry[Tail].Occupied = TRUE;
		EAP_Queue->Entry[Tail].Machine = Machine;
		EAP_Queue->Entry[Tail].MsgType = MsgType;
		EAP_Queue->Entry[Tail].MsgLen  = MsgLen;
		EAP_Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
		EAP_Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
		NdisMoveMemory(&EAP_Queue->Entry[Tail].rssi_info, rssi_info, sizeof(struct raw_rssi_info));
		EAP_Queue->Entry[Tail].Signal = rssi_info->raw_snr;
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
		EAP_Queue->Entry[Tail].Snr0= Snr0;
		EAP_Queue->Entry[Tail].Snr1= Snr1;
#endif
		EAP_Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
		EAP_Queue->Entry[Tail].OpMode = (ULONG)OpMode;
		EAP_Queue->Entry[Tail].Channel = pAd->CommonCfg.BBPCurrentBW == BW_40 ?
								pAd->CommonCfg.Channel : pAd->LatchRfRegs.Channel;
		EAP_Queue->Entry[Tail].Priv = 0;
#ifdef APCLI_SUPPORT
		EAP_Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ApCliIdx].RepeaterCli[CliIdx].CurrentAddress, pFrame->Hdr.Addr1))
				{
					EAP_Queue->Entry[Tail].Priv = (64 + (MAX_EXT_MAC_ADDR_SIZE * ApCliIdx) + CliIdx);
					break;
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

		if (Msg != NULL)
		{
			NdisMoveMemory(EAP_Queue->Entry[Tail].Msg, Msg, MsgLen);
		}

		NdisReleaseSpinLock(&(EAP_Queue->Lock));
	}
	else
#endif /* EAPOL_QUEUE_SUPPORT */
	{

		if (MlmeQueueFull(Queue, 0)) 
		{
 			RTMP_MLME_HANDLER(pAd);
			return FALSE;
		}
		
		/* OK, we got all the informations, it is time to put things into queue*/
		NdisAcquireSpinLock(&(Queue->Lock));
		Tail = Queue->Tail;
		/*
			Double check for safety in multi-thread system.
		*/
		if (Queue->Entry[Tail].Occupied)
		{
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
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
		Queue->Entry[Tail].Snr0= Snr0;
		Queue->Entry[Tail].Snr1= Snr1;
#endif
		Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
		Queue->Entry[Tail].OpMode = (ULONG)OpMode;
		Queue->Entry[Tail].Channel = pAd->CommonCfg.BBPCurrentBW == BW_40 ?
								pAd->CommonCfg.Channel : pAd->LatchRfRegs.Channel;
		Queue->Entry[Tail].Priv = 0;
#ifdef APCLI_SUPPORT
		Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ApCliIdx].RepeaterCli[CliIdx].CurrentAddress, pFrame->Hdr.Addr1))
				{
					Queue->Entry[Tail].Priv = (64 + (MAX_EXT_MAC_ADDR_SIZE * ApCliIdx) + CliIdx);
					break;
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

		if (Msg != NULL)
		{
			NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
		}

		NdisReleaseSpinLock(&(Queue->Lock));
	}


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
	IN VOID *Msg) 
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> MlmeEnqueueForWsc\n"));
    /* Do nothing if the driver is starting halt state.*/
    /* This might happen when timer already been fired before cancel timer with mlmehalt*/
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
        DBGPRINT_ERR(("MlmeEnqueueForWsc: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}
	
    if (MlmeQueueFull(Queue, 1)) 
    {
        
        return FALSE;
    }

    /* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
    Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
    Queue->Tail++;
    Queue->Num++;
    if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
    {
        Queue->Tail = 0;
    }
    
    Queue->Entry[Tail].Occupied = TRUE;
    Queue->Entry[Tail].Machine = Machine;
    Queue->Entry[Tail].MsgType = MsgType;
    Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = eventID;
	Queue->Entry[Tail].TimeStamp.u.HighPart = senderID;
    if (Msg != NULL)
        NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

    NdisReleaseSpinLock(&(Queue->Lock));

	DBGPRINT(RT_DEBUG_TRACE, ("<----- MlmeEnqueueForWsc\n"));
	
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
	{
		Queue->Head = 0;
	}
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

#ifdef EAPOL_QUEUE_SUPPORT
BOOLEAN EAPMlmeDequeue(
	IN EAP_MLME_QUEUE *Queue, 
	OUT MLME_QUEUE_ELEM **Elem) 
{
	NdisAcquireSpinLock(&(Queue->Lock));
	*Elem = &(Queue->Entry[Queue->Head]);    
	Queue->Num--;
	Queue->Head++;
	if (Queue->Head == MAX_LEN_OF_EAP_QUEUE) 
	{
		Queue->Head = 0;
	}
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}
#endif /* EAPOL_QUEUE_SUPPORT */

VOID MlmeRestartStateMachine(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM *Elem = NULL;
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine \n"));

#ifdef RTMP_MAC_PCI
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning) 
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} 
	else 
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

#ifdef EAPOL_QUEUE_SUPPORT
	while (!EAPMlmeQueueEmpty(&pAd->Mlme.EAP_Queue)) 
	{
		/*From message type, determine which state machine I should drive*/
		if (EAPMlmeDequeue(&pAd->Mlme.EAP_Queue, &Elem)) 
		{
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeRestartStateMachine: EAP MlmeQueue empty\n"));
		}
	}
#endif /* EAPOL_QUEUE_SUPPORT */

	/* Remove all Mlme queues elements*/
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
	{
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
		{
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeRestartStateMachine: MlmeQueue empty\n"));
		}
	}
#endif /* RTMP_MAC_PCI */


	/* Change back to original channel in case of doing scan*/
	{
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);
	}

	/* Resume MSDU which is turned off durning scan*/
	RTMPResumeMsduTransmission(pAd);

	
#ifdef RTMP_MAC_PCI	
	/* Remove running state*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif /* RTMP_MAC_PCI */

//CFG_TODO for SCAN
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

#ifdef EAPOL_QUEUE_SUPPORT
BOOLEAN EAPMlmeQueueEmpty(
	IN EAP_MLME_QUEUE *Queue) 
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}
#endif /* EAPOL_QUEUE_SUPPORT */

/*! \brief	 test if the MLME Queue is full
 *	\param	 *Queue 	 The MLME Queue
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

#ifdef EAPOL_QUEUE_SUPPORT
BOOLEAN EAPMlmeQueueFull(
	IN EAP_MLME_QUEUE *Queue) 
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == MAX_LEN_OF_EAP_QUEUE);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}
#endif /* EAPOL_QUEUE_SUPPORT */

/*! \brief	 The destructor of MLME Queue
 *	\param 
 *	\return
 *	\pre
 *	\post
 *	\note	Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL
 
 */
VOID MlmeQueueDestroy(
#ifdef EAPOL_QUEUE_SUPPORT
	IN EAP_MLME_QUEUE *pEAP_Queue,
#endif
	IN MLME_QUEUE *pQueue) 
{
	NdisAcquireSpinLock(&(pQueue->Lock));
	pQueue->Num  = 0;
	pQueue->Head = 0;
	pQueue->Tail = 0;
	NdisReleaseSpinLock(&(pQueue->Lock));
	NdisFreeSpinLock(&(pQueue->Lock));

#ifdef EAPOL_QUEUE_SUPPORT
	NdisAcquireSpinLock(&(pEAP_Queue->Lock));
	pEAP_Queue->Num  = 0;
	pEAP_Queue->Head = 0;
	pEAP_Queue->Tail = 0;
	NdisReleaseSpinLock(&(pEAP_Queue->Lock));
	NdisFreeSpinLock(&(pEAP_Queue->Lock));	
#endif /* EAPOL_QUEUE_SUPPORT */

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
 *	\param	DefFunc 	default function, when there is invalid state/message combination 
 *	\param	InitState	initial state of the state machine 
 *	\param	Base		StateMachine base, internal use only
 *	\pre p_sm should be a legal pointer
 *	\post

 IRQL = PASSIVE_LEVEL
 
 */
VOID StateMachineInit(
	IN STATE_MACHINE *S, 
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
	for (i = 0; i < StNr; i++) 
	{
		for (j = 0; j < MsgNr; j++) 
		{
			S->TransFunc[i * MsgNr + j] = DefFunc;
		}
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
	IN STATE_MACHINE *S, 
	IN ULONG St, 
	IN ULONG Msg, 
	IN STATE_MACHINE_FUNC Func) 
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg) 
	{
		/* boundary checking before setting the action*/
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	} 
}


/*! \brief	 This function does the state transition
 *	\param	 *Adapter the NIC adapter pointer
 *	\param	 *S 	  the state machine
 *	\param	 *Elem	  the message to be executed
 *	\return   None
 */
VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd, 
	IN STATE_MACHINE *S, 
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

	for (i = 0; i < 8; i++) 
	{
		if (pAd->Mlme.ShiftReg & 0x00000001) 
		{
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		} 
		else 
		{
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}
		R = (R << 1) | Result;
	}

	return R;
}


UCHAR RandomByte2(RTMP_ADAPTER *pAd)
{
	UINT32 a,b;
	UCHAR value, seed = 0;

	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
					__FUNCTION__, __LINE__));
		return 0;
	}
	
	/*MAC statistic related*/
	a = AsicGetCCACnt(pAd);
	a &= 0x0000ffff;
	b = AsicGetCrcErrCnt(pAd);
	b &= 0x0000ffff;
	value = (a<<16)|b;

	/*get seed by RSSI or SNR related info */
	seed = get_random_seed_by_phy(pAd);

	return value ^ seed ^ RandomByte(pAd);
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPCheckRates(RTMP_ADAPTER *pAd, UCHAR SupRate[], UCHAR *SupRateLen)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12]={0}, NewRateLen;
	
	NewRateLen = 0;
	
	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
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

	if(pAd->Antenna.field.RxPath == 3)
	{
		Rssi = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2])/3;
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		Rssi = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1])>>1;
	}
	else
	{
		Rssi = pRssi->AvgRssi[0];
	}

	return Rssi;
}


CHAR RTMPMaxRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2)
{
	CHAR	larger = -127;
	
	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 <= 0))
	{
		larger = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 <= 0))
	{
		larger = max(Rssi0, Rssi1);
	}
	
	if ((pAd->Antenna.field.RxPath == 3) && (Rssi2 <= 0))
	{
		larger = max(larger, Rssi2);
	}

	if (larger == -127)
		larger = 0;

	return larger;
}

CHAR RTMPMinRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2)
{
	CHAR	smaller = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 <= 0))
	{
		smaller = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 <= 0))
	{
		smaller = min(Rssi0, Rssi1);
	}
	
	if ((pAd->Antenna.field.RxPath == 3) && (Rssi2 <= 0))
	{
		smaller = min(smaller, Rssi2);
	}

	if (smaller == -127)
		smaller = 0;

	return smaller;
}


CHAR RTMPMinSnr(RTMP_ADAPTER *pAd, CHAR Snr0, CHAR Snr1)
{
	CHAR	smaller = Snr0;
	
	if (pAd->Antenna.field.RxPath == 1) 
	{
		smaller = Snr0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Snr1 != 0))
	{
		smaller = min(Snr0, Snr1);
	}
 
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


	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS |
							fRTMP_ADAPTER_HALT_IN_PROGRESS |
							fRTMP_ADAPTER_RADIO_OFF |
							fRTMP_ADAPTER_NIC_NOT_EXIST |
							fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
		|| OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) 
	)
		return;
	

#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		return;
#endif /* MT_MAC */

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
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
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)FunctionContext;



#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS |
							fRTMP_ADAPTER_HALT_IN_PROGRESS |
							fRTMP_ADAPTER_RADIO_OFF |
							fRTMP_ADAPTER_NIC_NOT_EXIST) 
		|| OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) 
	)
		return;

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
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
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
		!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
		return;

	pAd->CommonCfg.TriggerTimerCount++;

/* Driver should not send trigger frame, it should be send by application layer*/
/*
	if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable
		&& (pAd->CommonCfg.bNeedSendTriggerFrame ||
		(((pAd->CommonCfg.TriggerTimerCount%20) == 19) && (!pAd->CommonCfg.bAPSDAC_BE || !pAd->CommonCfg.bAPSDAC_BK || !pAd->CommonCfg.bAPSDAC_VI || !pAd->CommonCfg.bAPSDAC_VO))))
	{
		DBGPRINT(RT_DEBUG_TRACE,("Sending trigger frame and enter service period when support APSD\n"));
		RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
		pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
		pAd->CommonCfg.TriggerTimerCount = 0;
		pAd->CommonCfg.bInServicePeriod = TRUE;
	}*/
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
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): entry or wdev is NULL!\n", 
					__FUNCTION__));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		result = pEntry->wdev->bAutoTxRateSwitch;
	}
#endif /* CONFIG_AP_SUPPORT */






	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd)	
{
#ifdef CONFIG_AP_SUPPORT		
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT	apidx = 0;
	
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			if (pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}			
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
		{
			if (pAd->WdsTab.WdsEntry[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
		{
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
	if (pEntry)
	{
		if (IS_ENTRY_CLIENT(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			tx_mode = (UCHAR)pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_APCLI(pEntry))
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
	if (fixed_tx_mode == FIXED_TXMODE_VHT)
	{
		TransmitSetting.field.MODE = MODE_VHT;
		TransmitSetting.field.BW = pEntry->MaxHTPhyMode.field.BW;
		/* CCK mode allow MCS 0~3*/
		if (TransmitSetting.field.MCS > ((1 << 4) + MCS_7))
			TransmitSetting.field.MCS = ((1 << 4) + MCS_7);
	}
	else
#endif /* DOT11_VHT_AC */
	if (fixed_tx_mode == FIXED_TXMODE_CCK)
	{
		TransmitSetting.field.MODE = MODE_CCK;
		/* CCK mode allow MCS 0~3*/
		if (TransmitSetting.field.MCS > MCS_3)
			TransmitSetting.field.MCS = MCS_3;
	}
	else
	{
		TransmitSetting.field.MODE = MODE_OFDM;
		/* OFDM mode allow MCS 0~7*/
		if (TransmitSetting.field.MCS > MCS_7)
			TransmitSetting.field.MCS = MCS_7;
	}
	
	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE)
	{
		pEntry->HTPhyMode.word = TransmitSetting.word;
		DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d \n", 
				pEntry->wcid, get_phymode_str(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));		
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : the fixed TxMode is invalid \n", __FUNCTION__));	
	}
}



VOID RTMPSetAGCInitValue(RTMP_ADAPTER *pAd, UCHAR BandWidth)
{
	if (pAd->chipOps.ChipAGCInit != NULL)
		pAd->chipOps.ChipAGCInit(pAd, BandWidth);
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

	/* look for all registered channels */
	for(IdChan=0; IdChan<pAd->ChannelListNum; IdChan++)
	{
		if (pAd->ChannelList[IdChan].Channel == ChanNum)
		{
			if ((pAd->ChannelList[IdChan].Flags & Property) == Property)
				return TRUE;

			break;
		}
	}

	return FALSE;
}

#ifdef P2P_CHANNEL_LIST_SEPARATE

BOOLEAN P2P_CHAN_PropertyCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UINT32			ChanNum,
	IN UCHAR			Property)
{
	UINT32 IdChan;
	PCFG80211_CTRL pCfg80211_Ctrl = &pAd->cfg80211_ctrl;


	/* look for all registered channels */
	for(IdChan = 0; IdChan < pCfg80211_Ctrl->ChannelListNum; IdChan++)
	{
		if (pCfg80211_Ctrl->ChannelList[IdChan].Channel == ChanNum)
		{
			if ((pCfg80211_Ctrl->ChannelList[IdChan].Flags & Property) == Property)
				return TRUE;

			break;
		}
	}

	return FALSE;
}
#endif /* P2P_CHANNEL_LIST_SEPARATE */



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
	if (NState == NDIS_STATUS_SUCCESS) 
	{
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
		MlmeFreeMemory(pAd, pFrame);
	}

   return NState;
}


#ifdef DYNAMIC_RX_RATE_ADJ
VOID UpdateSuppRateBitmap(
	IN RTMP_ADAPTER *pAd)
{
	UCHAR idx = 0, j = 0, supp_rate_num = 0;
	UCHAR ext_rate_num = 0, des_rate_num = 0;
	ULONG bitmap = 0;
	BOOLEAN bABandPhyMode = FALSE;
	UCHAR apidx = 0;

	for(apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		idx = 0;
		j = 0;
		supp_rate_num = 0;
		ext_rate_num = 0;
		des_rate_num = 0;
		bitmap = pAd->ApCfg.MBSSID[apidx].SuppRateBitmap;
		bABandPhyMode = FALSE;

		if (pAd->ApCfg.MBSSID[apidx].wdev.PhyMode == WMODE_B)
			pAd->ApCfg.MBSSID[apidx].SuppRateBitmap &= 0x0F;

		if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[apidx].wdev.PhyMode))
		{
			pAd->ApCfg.MBSSID[apidx].SuppRateBitmap &= 0xFF0;
			bABandPhyMode = TRUE;
		}

	    for(idx=0; idx < MAX_LEN_OF_SUPPORTED_RATES; idx++)
	    {
	        if (bitmap & (1 << idx))
	        {
				switch (idx)
				{
					case 0 :
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x02;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 2;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_1 */
					case 1 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x04;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 4;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_2 */
					case 2 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x0B;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 11;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_5_5 */
					case 3 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x16;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 22;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_11 */
					case 4 :
							if (bABandPhyMode == TRUE)
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x8C;
								supp_rate_num++;
							}
							else
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedExtRate[ext_rate_num]= 0x0C;
								ext_rate_num++;
							}
							
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 12;
							des_rate_num++;
							break; /* RATE_6 */
					case 5 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x12;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 18;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_9 */
					case 6 :
							if (bABandPhyMode == TRUE)
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x98;
								supp_rate_num++;
							}
							else
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedExtRate[ext_rate_num]= 0x18;
								ext_rate_num++;
							}

							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 24;
							des_rate_num++;
							break; /* RATE_12 */
					case 7 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x24;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 36;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_18 */
					case 8 :
							if (bABandPhyMode == TRUE)
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0xB0;
								supp_rate_num++;
							}
							else
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedExtRate[ext_rate_num]= 0x30;
								ext_rate_num++;
							}
							
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 48;
							des_rate_num++;
							break; /* RATE_24 */
					case 9 :	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x48;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 72;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_36 */
					case 10:
							if (bABandPhyMode == TRUE)
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x60;
								supp_rate_num++;
							}
							else
							{
								pAd->ApCfg.MBSSID[apidx].ExpectedExtRate[ext_rate_num]= 0x60;
								ext_rate_num++;
							}
							
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 96;
							des_rate_num++;
							break; /* RATE_48 */
					case 11:	
							pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate[supp_rate_num]= 0x6C;
							pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate[des_rate_num] = 108;
							supp_rate_num++;
							des_rate_num++;
							break; /* RATE_54 */
				}
	        }
	    }

		pAd->ApCfg.MBSSID[apidx].ExpectedSuppRateLen = supp_rate_num;
		pAd->ApCfg.MBSSID[apidx].ExpectedExtRateLen = ext_rate_num;

		NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].DesireRate, MAX_LEN_OF_SUPPORTED_RATES);

		NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].SupRate, pAd->ApCfg.MBSSID[apidx].ExpectedSuppRate, supp_rate_num);
		pAd->ApCfg.MBSSID[apidx].SupRateLen = supp_rate_num;
		DBGPRINT(RT_DEBUG_OFF, ("BSS%d SupRateLen=%x\n", apidx, pAd->ApCfg.MBSSID[apidx].SupRateLen));
		NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].ExtRate, pAd->ApCfg.MBSSID[apidx].ExpectedExtRate, ext_rate_num);
		pAd->ApCfg.MBSSID[apidx].ExtRateLen = ext_rate_num;
		DBGPRINT(RT_DEBUG_OFF, ("BSS%d ExtRateLen=%x\n", apidx, pAd->ApCfg.MBSSID[apidx].ExtRateLen));
		NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].DesireRate, pAd->ApCfg.MBSSID[apidx].ExpectedDesireRate, des_rate_num);
	}
}

VOID UpdateSuppHTRateBitmap(
	IN RTMP_ADAPTER *pAd)
{
	UCHAR idx = 0, j = 0;
	ULONG bitmap = 0;
	UCHAR apidx = 0;

	for(apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		idx = 0;
		j = 0;
		bitmap = pAd->ApCfg.MBSSID[apidx].SuppHTRateBitmap;

		NdisZeroMemory(&pAd->ApCfg.MBSSID[apidx].ExpectedSuppHTMCSSet[0], 16);

		for(idx=0; idx < MAX_LEN_OF_HT_RATES; idx++)
		{
			if (bitmap & (1 << idx))
			{
				j = idx / 8;
				pAd->ApCfg.MBSSID[apidx].ExpectedSuppHTMCSSet[j] |= (1 << (idx % 8));
			}
		}
	}
}
#endif /* DYNAMIC_RX_RATE_ADJ */

