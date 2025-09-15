/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************
 
    Module Name:
    mlme.c
 
    Abstract:
    Major MLME state machiones here
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#include <stdarg.h>

#ifdef BTCOEX_CONCURRENT
extern void MT7662ReceCoexFromOtherCHip(
	IN UCHAR channel,
	IN UCHAR centralchannel,
	IN UCHAR channel_bw
	);
#endif
#if defined(MAX_CONTINUOUS_TX_CNT) || defined(NEW_IXIA_METHOD)
VOID rtmp_deqmode_detect(RTMP_ADAPTER *pAd);
#endif
#ifdef DOT11_N_SUPPORT

int DetectOverlappingPeriodicRound;


#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	int apidx;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("Bss2040CoexistTimeOut(): Recovery to original setting!\n"));
	
	/* Recovery to original setting when next DTIM Interval. */
	pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_TIMER_FIRED);
	NdisZeroMemory(&pAd->CommonCfg.LastBSSCoexist2040, sizeof(BSS_2040_COEXIST_IE));
	pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
	
	if (pAd->CommonCfg.bBssCoexEnable == FALSE)
	{
		/* TODO: Find a better way to handle this when the timer is fired and we disable the bBssCoexEable support!! */
		DBGPRINT(RT_DEBUG_TRACE, ("Bss2040CoexistTimeOut(): bBssCoexEnable is FALSE, return directly!\n"));
		return;
	}
	
	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID, apidx, 0);
	
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */


VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
#ifdef DOT11_N_SUPPORT
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	if (DetectOverlappingPeriodicRound == 0)
	{
		/* switch back 20/40 */
		if ((pAd->CommonCfg.Channel <=14) && (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{
			pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;
			pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
		}
	}
	else
	{
		if ((DetectOverlappingPeriodicRound == 25) || (DetectOverlappingPeriodicRound == 1))
		{   
   			if ((pAd->CommonCfg.Channel <=14) && (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth==BW_40))
			{                                     
				SendBeaconRequest(pAd, 1);
				SendBeaconRequest(pAd, 2);
                		SendBeaconRequest(pAd, 3);
			}

		}
		DetectOverlappingPeriodicRound--;
	}
#endif /* DOT11_N_SUPPORT */
}


/*
    ==========================================================================
    Description:
        This routine is executed every second -
        1. Decide the overall channel quality
        2. Check if need to upgrade the TX rate to any client
        3. perform MAC table maintenance, including ageout no-traffic clients, 
           and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID APMlmePeriodicExec(
    PRTMP_ADAPTER pAd)
{
#ifdef A4_CONN
		UCHAR mbss_idx;
#endif
    /* 
		Reqeust by David 2005/05/12
		It make sense to disable Adjust Tx Power on AP mode, since we can't 
		take care all of the client's situation
		ToDo: need to verify compatibility issue with WiFi product.
	*/

#ifdef BTCOEX_CONCURRENT
	MT7662ReceCoexFromOtherCHip(pAd->CommonCfg.Channel,pAd->CommonCfg.CentralChannel,pAd->CommonCfg.BBPCurrentBW);
#endif

#ifdef CARRIER_DETECTION_SUPPORT
	if (isCarrierDetectExist(pAd) == TRUE)
	{
		PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;
		if (pCarrierDetect->OneSecIntCount < pCarrierDetect->CarrierGoneThreshold)
		{
			pCarrierDetect->CD_State = CD_NORMAL;
			pCarrierDetect->recheck = pCarrierDetect->recheck1;
			if (pCarrierDetect->Debug != RT_DEBUG_TRACE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Carrier gone\n"));
				/* start all TX actions. */
				APMakeAllBssBeacon(pAd);
				APUpdateAllBeaconFrame(pAd);
				AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Carrier gone\n"));
			}
		}
		pCarrierDetect->OneSecIntCount = 0;
	}
			
#endif /* CARRIER_DETECTION_SUPPORT */

	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pAd->ApCfg.RssiSample);


	/* Disable Adjust Tx Power for WPA WiFi-test. */
	/* Because high TX power results in the abnormal disconnection of Intel BG-STA. */
/*#ifndef WIFI_TEST */
/*	if (pAd->CommonCfg.bWiFiTest == FALSE) */
	/* for SmartBit 64-byte stream test */
	/* removed based on the decision of Ralink congress at 2011/7/06 */
/*	if (pAd->MacTab.Size > 0) */
	RTMP_CHIP_ASIC_ADJUST_TX_POWER(pAd);
/*#endif // WIFI_TEST */

	RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(pAd);

    /* walk through MAC table, see if switching TX rate is required */

    /* MAC table maintenance */
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)
	{
#if defined(BAND_STEERING) || defined(CUSTOMER_DCC_FEATURE) || defined(WIFI_DIAG)\
	|| defined(WAPP_SUPPORT)
		UINT32 msdr9;

		/* NAV, CCA, or TX active timer in unit of TU, used for Measurement. (channel busy time) */
		RTMP_IO_READ32(pAd, MIB_MSDR9, &msdr9);
		msdr9 &= 0xFFFFFF;
		pAd->OneSecChBusyTime = msdr9;
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering)
			pAd->ApCfg.BndStrgOneSecChBusyTime = (pAd->ApCfg.BndStrgOneSecChBusyTime == 0)? \
						msdr9 : ((msdr9 + pAd->ApCfg.BndStrgOneSecChBusyTime)>>1);
#endif
#endif
		/* one second timer */
	    MacTableMaintenance(pAd);

#ifdef WH_EZ_SETUP
#ifdef NEW_CONNECTION_ALGO
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)) {
			EzMlmeEnqueue(pAd, EZ_STATE_MACHINE, EZ_PERIODIC_EXEC_REQ, 0, NULL, 0);
		}
#endif
#endif
#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_tr_stop)
	{
		INT enable = FALSE, ctrl_type;
		
		/* enable/disable tx/rx*/
		switch (pAd->fpga_ctl.fpga_tr_stop)
		{
			case 3:  //stop tx + rx
				ctrl_type = ASIC_MAC_TXRX;
				break;
			case 2: // stop rx
				ctrl_type = ASIC_MAC_RX;
				break;
			case 1: // stop tx
				ctrl_type = ASIC_MAC_TX;
				break;
			case 4:
			default:
				enable = TRUE;
				ctrl_type = ASIC_MAC_TXRX;
				break;
		}
		AsicSetMacTxRx(pAd, ctrl_type, enable);
	}
#endif /* CONFIG_FPGA_MODE */

		RTMPMaintainPMKIDCache(pAd);

#ifdef WDS_SUPPORT
		WdsTableMaintenance(pAd);
#endif /* WDS_SUPPORT */


#ifdef CLIENT_WDS
	CliWds_ProxyTabMaintain(pAd);
#endif /* CLIENT_WDS */
#ifdef A4_CONN
		for(mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++)
			a4_proxy_maintain(pAd, mbss_idx);
		pAd->a4_need_refresh = FALSE;
#endif /* A4_CONN */
#ifdef WH_EVENT_NOTIFIER
		//WHCMlmePeriodicExec(pAd); // Arvind : Need to confirm that we have to support this event   
#endif /* WH_EVENT_NOTIFIER */

#ifdef WIFI_DIAG
		DiagApMlmeOneSecProc(pAd);
#endif
	}
	
#ifdef AP_SCAN_SUPPORT
	AutoChannelSelCheck(pAd);
#endif /* AP_SCAN_SUPPORT */

	APUpdateCapabilityAndErpIe(pAd);

#ifdef APCLI_SUPPORT
	if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
		ApCliIfMonitor(pAd);

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 1
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		&& (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	) {
		ApCliIfUp(pAd);
	}

	{
		INT loop;
		ULONG Now32;

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			RTMPRepeaterReconnectionCheck(pAd);
		}
#endif /* MAC_REPEATER_SUPPORT */

		NdisGetSystemUpTime(&Now32);
		for (loop = 0; loop < MAX_APCLI_NUM; loop++)
		{
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
			if ((pApCliEntry->Valid == TRUE)
				&& (pApCliEntry->MacTabWCID < MAX_LEN_OF_MAC_TABLE))
			{
				/* update channel quality for Roaming and UI LinkQuality display */
				MlmeCalculateChannelQuality(pAd,
					&pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
			}
		}
	}
#endif /* APCLI_SUPPORT */

#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.bHTProtect) {
		/*APUpdateCapabilityAndErpIe(pAd); */
		APUpdateOperationMode(pAd);
		if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
		    	AsicUpdateProtect(pAd, (USHORT)pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, 
		    						ALLN_SETPROTECT, FALSE, pAd->MacTab.fAnyStationNonGF);
	}
#endif /* DOT11_N_SUPPORT */

#ifdef A_BAND_SUPPORT
	if ( (pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1))
	{
#ifdef DFS_SUPPORT
		ApRadarDetectPeriodic(pAd);
#else
		pAd->Dot11_H.InServiceMonitorCount++;
		if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
		{
			if (pAd->Dot11_H.RDCount++ > pAd->Dot11_H.ChMovingTime)
			{
				AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
				pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
			}
		}
#endif /* !DFS_SUPPORT */
		}
#endif /* A_BAND_SUPPORT */
#ifdef MBO_SUPPORT
	MboCheckBssTermination(pAd);
#endif/* MBO_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_R1KHInfoMaintenance(pAd);
#endif /* DOT11R_FT_SUPPORT */
#if defined(MAX_CONTINUOUS_TX_CNT) || defined(NEW_IXIA_METHOD)
	if(pAd->MonitorFlag == TRUE)
		rtmp_deqmode_detect(pAd); /* 1s detect*/
#endif
#ifdef BAND_STEERING
	BndStrgHeartBeatMonitor(pAd);
#endif
#ifdef NEW_IXIA_METHOD
	if (pAd->Mlme.OneSecPeriodicRound % pAd->chkTmr == 0) {
		if ((txpktdetect2s < pAd->pktthld)
			&& (rxpktdetect2s < pAd->pktthld)) {/*Threshold*/
			pAd->tmrlogctrl++;
			if (pAd->tmrlogctrl <= 1)
				wifi_txrx_parmtrs_dump(pAd);
			else
				pAd->tmrlogctrl = 10;/*prevent pAd->tmrlogctrl overflow*/
		} else
			pAd->tmrlogctrl = 0;
		txpktdetect2s = 0;
		rxpktdetect2s = 0;
	}
#endif
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN APMsgTypeSubst(
    IN PRTMP_ADAPTER pAd,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType) 
{
	USHORT Seq;
#ifdef DOT11_SAE_SUPPORT
	USHORT Alg;
#endif /* DOT11_SAE_SUPPORT */
	UCHAR  EAPType;
	BOOLEAN     Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;

#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	/*
		TODO:
		only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; 
		otherwise, ignore this frame
	*/

	/* wpa EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) 
	{    
#ifdef WSC_AP_SUPPORT
		WSC_CTRL *wsc_ctrl;
		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
		if (pEntry &&
			((pEntry->bWscCapable) ||
			(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode < Ndis802_11AuthModeWPA)))
		{
			/*
				WSC AP only can service one WSC STA in one WPS session.
				Forward this EAP packet to WSC SM if this EAP packets is from 
				WSC STA that WSC AP services or WSC AP doesn't service any 
				WSC STA now.
			*/
			wsc_ctrl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl;
			if ((MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, pEntry->Addr) || 
				MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)) &&
				IS_ENTRY_CLIENT(pEntry) && 
				(wsc_ctrl->WscConfMode != WSC_DISABLE))
			{
				*Machine = WSC_STATE_MACHINE;
				EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
				EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
				Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
			}
		}
#endif /* WSC_AP_SUPPORT */
		if (!Return)
		{
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, (INT *) MsgType);
		}
		return Return;
	}

	if (pFrame->Hdr.FC.Type != FC_TYPE_MGMT)
		return FALSE;

	switch (pFrame->Hdr.FC.SubType) 
	{
		case SUBTYPE_ASSOC_REQ:
			*Machine = AP_ASSOC_STATE_MACHINE;
			*MsgType = APMT2_PEER_ASSOC_REQ;

			break;
		/*
		case SUBTYPE_ASSOC_RSP:
			*Machine = AP_ASSOC_STATE_MACHINE;
			*MsgType = APMT2_PEER_ASSOC_RSP;
			break;
		*/
		case SUBTYPE_REASSOC_REQ:
			*Machine = AP_ASSOC_STATE_MACHINE;
			*MsgType = APMT2_PEER_REASSOC_REQ;
			break;
		/*
		case SUBTYPE_REASSOC_RSP:
			*Machine = AP_ASSOC_STATE_MACHINE;
			*MsgType = APMT2_PEER_REASSOC_RSP;
			break;
		*/
		case SUBTYPE_PROBE_REQ:
			*Machine = AP_SYNC_STATE_MACHINE;              
			*MsgType = APMT2_PEER_PROBE_REQ;
			break;
	
		/* For Active Scan */
		case SUBTYPE_PROBE_RSP:
			*Machine = AP_SYNC_STATE_MACHINE;
			*MsgType = APMT2_PEER_PROBE_RSP;
			break;
		case SUBTYPE_BEACON:
			*Machine = AP_SYNC_STATE_MACHINE;
			*MsgType = APMT2_PEER_BEACON;
			break;
		/*
		case SUBTYPE_ATIM:
			*Machine = AP_SYNC_STATE_MACHINE;
			*MsgType = APMT2_PEER_ATIM;
			break;
		*/
		case SUBTYPE_DISASSOC:
			*Machine = AP_ASSOC_STATE_MACHINE;
			*MsgType = APMT2_PEER_DISASSOC_REQ;
			break;
		case SUBTYPE_AUTH:
			/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
#ifdef DOT11_SAE_SUPPORT
			NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
			NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));

			*Machine = AP_AUTH_STATE_MACHINE;
			if (Seq == 1
#ifdef DOT11_SAE_SUPPORT
				|| (Alg == AUTH_MODE_SAE && Seq == 2)
#endif /* DOT11_SAE_SUPPORT */
				)
				*MsgType = APMT2_PEER_AUTH_REQ;
			else if (Seq == 3)
				*MsgType = APMT2_PEER_AUTH_CONFIRM;
			else 
			{
				DBGPRINT(RT_DEBUG_TRACE,("wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n",
					Seq,
					pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3], 
					pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
				return FALSE;
			}
			break;

		case SUBTYPE_DEAUTH:
			*Machine = AP_AUTH_STATE_MACHINE; /*AP_AUTH_RSP_STATE_MACHINE;*/
			*MsgType = APMT2_PEER_DEAUTH;
			break;

		case SUBTYPE_ACTION:
		case SUBTYPE_ACTION_NO_ACK:
			*Machine = ACTION_STATE_MACHINE;
			/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
			{
				*MsgType = MT2_ACT_INVALID;
			} 
			else
			{
				*MsgType = (pFrame->Octet[0]&0x7F);
			} 
			break;

		default:
			return FALSE;
			break;
	}

	return TRUE;
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
VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG	TxTotalCnt;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */
#ifdef CARRIER_DETECTION_SUPPORT
	if(pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
	return;
#endif /* CARRIER_DETECTION_SUPPORT */





	
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPActive == TRUE)
		bbp_set_rxpath(pAd, 1);
	else
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
		bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
					pAd->RalinkCounters.OneSecTxRetryOkCount + 
					pAd->RalinkCounters.OneSecTxFailCount;

	if (TxTotalCnt > 50)
	{
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
		pAd->Mlme.bLowThroughput = FALSE;
	}
	else
	{
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
		pAd->Mlme.bLowThroughput = TRUE;
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
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd)
{
	CHAR rssi[3], *target_rssi;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

	/* if the traffic is low, use average rssi as the criteria */
	if (pAd->Mlme.bLowThroughput == TRUE)
		target_rssi = &pAd->ApCfg.RssiSample.LastRssi[0];
	else
		target_rssi = &pAd->ApCfg.RssiSample.AvgRssi[0];
	NdisMoveMemory(&rssi[0], target_rssi, 3);

#ifdef DOT11N_SS3_SUPPORT
	if(pAd->Antenna.field.RxPath == 3)
	{
		CHAR larger = -127;

		larger = max(rssi[0], rssi[1]);
		if (pAd->CommonCfg.RxStream >= 3)
			pAd->Mlme.RealRxPath = 3;
		else
		{
			if (larger > (rssi[2] + 20))
				pAd->Mlme.RealRxPath = 2;
			else
				pAd->Mlme.RealRxPath = 3;
		}
	}
#endif /* DOT11N_SS3_SUPPORT */

	/* Disable the below to fix 1T/2R issue. It's suggested by Rory at 2007/7/11. */

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPActive == TRUE)
		bbp_set_rxpath(pAd, 1);
	else
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
		bbp_set_rxpath(pAd, pAd->Mlme.RealRxPath);

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
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT*	              RssiAvg)  
{
		    SHORT	realavgrssi;
		    LONG         realavgrssi1;
		    ULONG	recvPktNum = pAd->RxAnt.RcvPktNum[AntSelect];

		    realavgrssi1 = pAd->RxAnt.Pair1AvgRssiGroup1[AntSelect];

		    if(realavgrssi1 == 0)
		    {      
		        *RssiAvg = 0;
		        return;
		    }

		    realavgrssi = (SHORT) (realavgrssi1 / recvPktNum);

		    pAd->RxAnt.Pair1AvgRssiGroup1[0] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup2[0] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
		    pAd->RxAnt.RcvPktNum[0] = 0;
		    pAd->RxAnt.RcvPktNum[1] = 0;
		    *RssiAvg = realavgrssi - 256;
}
#if defined(MAX_CONTINUOUS_TX_CNT) || defined(NEW_IXIA_METHOD)
BOOLEAN Rtmp_Max_Continuous_Tx_OnOff(RTMP_ADAPTER *pAd)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT i;
	CHAR MaxRssi  = -127,MinRssi  = -127,myAvgRssi = -127; //for RSSI
	ULONG DataRate=0; //for datarate.
	for (i=0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (!(IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)))
			continue;
		myAvgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);// get my rssi average.
		getRate(pEntry->HTPhyMode, &DataRate);
		
		if(DataRate < pAd->RateTh)
		{
			DBGPRINT(RT_DEBUG_WARN,("%s(%d)Some sta's rate is low. DataRate = %lu\n",__FUNCTION__,__LINE__,DataRate));
			return FALSE;
		}
		if((MaxRssi == -127) && (MinRssi == -127))
		{
			MaxRssi= myAvgRssi;
			MinRssi = myAvgRssi;
		}
		else
		{
			MaxRssi = RTMPMaxRssi(pAd,MaxRssi,myAvgRssi,0);// find the max rssi in mactable size.
			MinRssi = RTMPMinRssi(pAd,MinRssi,myAvgRssi,0);// find the min rssi in mactable size.
		}
		
	}
		DBGPRINT(RT_DEBUG_WARN,("%s(%d)MaxRssi = %-3d,MinRssi = %-3d\n",__FUNCTION__,__LINE__,MaxRssi,MinRssi));
		if((MaxRssi - MinRssi) < pAd->DeltaRssiTh) /*pAd->DeltaRssiTh default 10 dBm*/
		{
			DBGPRINT(RT_DEBUG_WARN,("%s(%d) in in in in in in \n",__FUNCTION__,__LINE__));
			return TRUE;
		}
		else	
			return FALSE;
}

VOID rtmp_deqmode_detect(RTMP_ADAPTER *pAd)
{
	if(pAd->MacTab.Size >=5)
	{
		DBGPRINT(RT_DEBUG_WARN,("%s(%d) MacTab.Size = %d\n",__FUNCTION__,__LINE__,pAd->MacTab.Size));
		if(Rtmp_Max_Continuous_Tx_OnOff(pAd) == TRUE)
		{
			pAd->ContinousTxCnt = CONTINUOUS_TX_CNT;/*set tx 21 packets one time*/
			DBGPRINT(RT_DEBUG_WARN,("%s(%d)Countinous to Tx %d Packets\n",__FUNCTION__,__LINE__,pAd->ContinousTxCnt));
		}
		else
		{
			pAd->ContinousTxCnt = 1; /*default tx 1 packet one time*/
			DBGPRINT(RT_DEBUG_WARN,("%s(%d)tx 1 packet!\n",__FUNCTION__,__LINE__));
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN,("%s(%d)client num = %d\n",__FUNCTION__,__LINE__, pAd->MacTab.Size));
		pAd->ContinousTxCnt = 1; /*default tx 1 packet one time*/
	}
}
#endif

