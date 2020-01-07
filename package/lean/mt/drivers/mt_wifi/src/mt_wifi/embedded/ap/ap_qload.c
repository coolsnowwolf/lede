/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

    Abstract:

    Provide information on the current STA population and traffic levels
	in the QBSS.

	This attribute is available only at a QAP. This attribute, when TRUE,
	indicates that the QAP implementation is capable of generating and
	transmitting the QBSS load element in the Beacon and Probe Response frames.

***************************************************************************/

#include "rt_config.h"

#ifdef AP_QLOAD_SUPPORT

struct GNU_PACKED _ELM_QBSS_LOAD {

	UINT8 ElementId;
	UINT8 Length;

	/* the total number of STAs currently associated with this QBSS */
	UINT16 StationCount;

	/*	defined as the percentage of time, nomalized to 255, the QAP sensed the
		medium busy, as indicated by either the physical or virtual carrier
		sense mechanism.
		This percentage is computed using the formula:
			((channel busy time / (dot11ChannelUtilizationBeaconIntervals *
			dot11BeaconPeriod * 1024)) * 255) */
	UINT8 ChanUtil;

	/*	specifies the remaining amount of medium time available via explicit
		admission control, in units of 32 microsecond periods per 1 second.
		The field is helpful for roaming non-AP QSTAs to select a QAP that is
		likely to accept future admission control requests, but it does not
		represent a guarantee that the HC will admit these requests. */
	UINT16 AvalAdmCap;

};

#define ELM_QBSS_LOAD_ID					11
#define ELM_QBSS_LOAD_LEN					5

/*
	We will send a alarm when channel busy time (primary or secondary) >=
	Time Threshold and Num Threshold.

	QBSS_LOAD_ALRAM_BUSY_TIME_THRESHOLD = 0 means alarm function is disabled.

	If you want to enable it, use command
	"iwpriv ra0 set qloadalarmtimethres=90"
*/
#define QBSS_LOAD_ALRAM_BUSY_TIME_THRESHOLD		0 /* unit: % */
#define QBSS_LOAD_ALRAM_BUSY_NUM_THRESHOLD		10 /* unit: 1 */

/* a alarm will not re-issued until QBSS_LOAD_ALARM_DURATION * TBTT */
#define QBSS_LOAD_ALARM_DURATION				100 /* unit: TBTT */


static VOID QBSS_LoadAlarmSuspend(
	IN		RTMP_ADAPTER * pAd);

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
/* handle a alarm */
static VOID QBSS_LoadAlarm(
	IN		RTMP_ADAPTER * pAd);
static VOID QBSS_LoadAlarmBusyTimeThresholdReset(
	IN		RTMP_ADAPTER * pAd,
	IN		UINT32			TimePeriod);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */




/* --------------------------------- Private -------------------------------- */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
/*
========================================================================
Routine Description:
	Handle a alarm.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
	You can use different methods to handle QBSS Load alarm here.

	Current methods are:
	1. Change 20/40 to 20-only.
	2. Change channel to the clear channel.
========================================================================
*/
static VOID QBSS_LoadAlarm(
	IN		RTMP_ADAPTER * pAd)
{
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	struct wifi_dev *wdev;
	UCHAR i;
	UCHAR op_ht_bw;
	UCHAR op_ext_cha;
#endif /*DOT11N_DRAFT3*/
#endif /*DOT11_N_SUPPORT*/
	/* suspend alarm until channel switch */
	QBSS_LoadAlarmSuspend(pAd);
	pQloadCtrl->QloadAlarmNumber++;
	/* check if we have already been 20M bandwidth */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		op_ext_cha = wlan_operate_get_ext_cha(wdev);
		op_ht_bw = wlan_operate_get_ht_bw(wdev);

		if ((op_ext_cha != 0) &&
			(op_ht_bw != HT_BW_20)) {
			MAC_TABLE *pMacTable;
			UINT32 StaId;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("qbss> Alarm! Change to 20 bw...\n"));
			/* disassociate stations without D3 2040Coexistence function */
			pMacTable = &pAd->MacTab;

			/*TODO: Carter, check StaId why start from 1.*/

			for (StaId = 1; VALID_UCAST_ENTRY_WCID(pAd, StaId); StaId++) {
				MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[StaId];
				BOOLEAN bDisconnectSta = FALSE;

				if (!IS_ENTRY_CLIENT(pEntry))
					continue;

				if (pEntry->Sst != SST_ASSOC)
					continue;

				if (pEntry->BSS2040CoexistenceMgmtSupport)
					bDisconnectSta = TRUE;

				if (bDisconnectSta) {
					/* send wireless event - for ageout */
					RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);
					{
						PUCHAR pOutBuffer = NULL;
						NDIS_STATUS NStatus;
						ULONG FrameLen = 0;
						HEADER_802_11 DeAuthHdr;
						USHORT Reason;
						/*  send out a DISASSOC request frame */
						NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

						if (NStatus != NDIS_STATUS_SUCCESS) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
							/*NdisReleaseSpinLock(&pAd->MacTabLock); */
							continue;
						}

						Reason = REASON_DEAUTH_STA_LEAVING;
						MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0,
										 pEntry->Addr,
										 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
										 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
						MakeOutgoingFrame(pOutBuffer, &FrameLen,
										  sizeof(HEADER_802_11), &DeAuthHdr,
										  2,                     &Reason,
										  END_OF_ARGS);
						MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
						MlmeFreeMemory(pOutBuffer);
					}
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("qbss> Alarm! Deauth the station %02x:%02x:%02x:%02x:%02x:%02x\n",
							 PRINT_MAC(pEntry->Addr)));
					mac_entry_delete(pAd, pEntry);
					continue;
				}
			}

			/* for 11n */
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
			/* mark alarm flag */
			pQloadCtrl->FlgQloadAlarm = TRUE;
			QBSS_LoadAlarmResume(pAd);
		} else
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
		{
			/* we are in 20MHz bandwidth so try to switch channel */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("qbss> Alarm! Switch channel...\n"));
			/* send command to switch channel */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_CHAN_RESCAN, (VOID *)&i, sizeof(UCHAR));
		}
	}
}


/*
========================================================================
Routine Description:
	Re-calculate busy time threshold.

Arguments:
	pAd					- WLAN control block pointer
	TimePeriod			- TBTT

Return Value:
	None

Note:
	EX: TBTT=100ms, 90%, pAd->QloadBusyTimeThreshold = 90ms
========================================================================
*/
static VOID QBSS_LoadAlarmBusyTimeThresholdReset(
	IN		RTMP_ADAPTER * pAd,
	IN		UINT32			TimePeriod)
{
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->QloadBusyTimeThreshold = TimePeriod;
	pQloadCtrl->QloadBusyTimeThreshold *= pQloadCtrl->QloadAlarmBusyTimeThreshold;
	pQloadCtrl->QloadBusyTimeThreshold /= 100;
	pQloadCtrl->QloadBusyTimeThreshold <<= 10; /* translate mini-sec to micro-sec */
}
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */


/*
========================================================================
Routine Description:
	Initialize ASIC Channel Busy Calculation mechanism.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
	Init Condition: WMM must be enabled.
========================================================================
*/
VOID QBSS_LoadInit(RTMP_ADAPTER *pAd)
{
	UINT32 IdBss;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	/* check whether any BSS enables WMM feature */
	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		if ((pAd->ApCfg.MBSSID[IdBss].wdev.bWmmCapable)
#ifdef DOT11K_RRM_SUPPORT
			|| (IS_RRM_ENABLE(&pAd->ApCfg.MBSSID[IdBss].wdev))
#endif /* DOT11K_RRM_SUPPORT */
			) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.channel > 14)
				pQloadCtrl = HcGetQloadCtrlByRf(pAd, RFIC_5GHZ);
			else
				pQloadCtrl = HcGetQloadCtrlByRf(pAd, RFIC_24GHZ);
			if (!pQloadCtrl)
				continue;
			if (pQloadCtrl->FlgQloadEnable)
				continue;

			pQloadCtrl->FlgQloadEnable = TRUE;
			/* default value is 50, please reference to IEEE802.11e 2005 Annex D */
			/* pQloadCtrl->QloadChanUtilBeaconInt = 50; */
			pQloadCtrl->QloadChanUtilBeaconInt = 5;

			AsicSetChBusyStat(pAd, pQloadCtrl->FlgQloadEnable);

			pQloadCtrl->QloadChanUtilTotal = 0;
			pQloadCtrl->QloadUpTimeLast = 0;

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
			/* clear busy time statistics */
			NdisZeroMemory(pQloadCtrl->QloadBusyCountPri, sizeof(pQloadCtrl->QloadBusyCountPri));
			NdisZeroMemory(pQloadCtrl->QloadBusyCountSec, sizeof(pQloadCtrl->QloadBusyCountSec));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
			/* init threshold before QBSS_LoadAlarmReset */
			pQloadCtrl->QloadAlarmBusyTimeThreshold = QBSS_LOAD_ALRAM_BUSY_TIME_THRESHOLD;
			pQloadCtrl->QloadAlarmBusyNumThreshold = QBSS_LOAD_ALRAM_BUSY_NUM_THRESHOLD;

			QBSS_LoadAlarmReset(pAd);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

		}
	}
}



/*
========================================================================
Routine Description:
	Reset alarm function.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID QBSS_LoadAlarmReset(
	IN		RTMP_ADAPTER	*pAd)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->FlgQloadAlarm = FALSE;
	pQloadCtrl->QloadAlarmDuration = 0;
	pQloadCtrl->QloadAlarmNumber = 0;
	pQloadCtrl->FlgQloadAlarmIsSuspended = FALSE;
	QBSS_LoadAlarmBusyTimeThresholdReset(pAd, pAd->CommonCfg.BeaconPeriod);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
}


/*
========================================================================
Routine Description:
	Resume alarm function.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID QBSS_LoadAlarmResume(
	IN		RTMP_ADAPTER	*pAd)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->FlgQloadAlarmIsSuspended = FALSE;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
}


/*
========================================================================
Routine Description:
	Suspend alarm function.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
static VOID QBSS_LoadAlarmSuspend(
	IN		RTMP_ADAPTER	*pAd)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->FlgQloadAlarmIsSuspended = TRUE;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
}


/*
========================================================================
Routine Description:
	Get average busy time in current channel.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	average busy time

Note:
========================================================================
*/
UINT32 QBSS_LoadBusyTimeGet(
	IN		RTMP_ADAPTER	*pAd)
{
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	if (pQloadCtrl->QloadChanUtilBeaconCnt == 0)
		return pQloadCtrl->QloadChanUtilTotal;

	return (pQloadCtrl->QloadChanUtilTotal / pQloadCtrl->QloadChanUtilBeaconCnt);
}


/*
========================================================================
Routine Description:
	Check if a alarm is occurred and clear the alarm.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	TRUE				- alarm occurs
	FALSE				- no alarm

Note:
	We will clear the alarm in the function.
========================================================================
*/
BOOLEAN QBSS_LoadIsAlarmIssued(
	IN		RTMP_ADAPTER	*pAd)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
	BOOLEAN FlgQloadAlarm = pQloadCtrl->FlgQloadAlarm;

	pQloadCtrl->FlgQloadAlarm = FALSE;
	return FlgQloadAlarm;
#else
	return FALSE;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
}


/*
========================================================================
Routine Description:
	Check if the busy time is accepted.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	TURE				- ok
	FALSE				- fail

Note:
========================================================================
*/
BOOLEAN QBSS_LoadIsBusyTimeAccepted(
	IN		RTMP_ADAPTER	*pAd,
	IN		UINT32			BusyTime)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	if (pQloadCtrl->QloadAlarmBusyTimeThreshold == 0)
		return TRUE; /* always ok */

	if (BusyTime >= pQloadCtrl->QloadBusyTimeThreshold)
		return FALSE;

#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	return TRUE;
}

#ifdef CONFIG_HOTSPOT_R2
UINT32 QBSS_LoadElementAppend_HSTEST(
	IN		RTMP_ADAPTER	*pAd,
	OUT		UINT8			*pBeaconBuf,
	IN		UCHAR			apidx)
{
	struct _ELM_QBSS_LOAD load, *pLoad = &load;
	ULONG ElmLen;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	/* check whether channel busy time calculation is enabled */
	if (pQloadCtrl->FlgQloadEnable == 0)
		return 0;

	/* End of if */
	/* init */
	pLoad->ElementId = ELM_QBSS_LOAD_ID;
	pLoad->Length = ELM_QBSS_LOAD_LEN;
	pLoad->StationCount = le2cpu16(pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadStaCnt);
	pLoad->ChanUtil = pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadCU;
	/* because no ACM is supported, the available bandwidth is 1 sec */
	pLoad->AvalAdmCap = le2cpu16(0xffff); /* 0x7a12 * 32us = 1 second */
	/* copy the element to the frame */
	MakeOutgoingFrame(pBeaconBuf, &ElmLen,
					  sizeof(struct _ELM_QBSS_LOAD),	pLoad,
					  END_OF_ARGS);
	return ElmLen;
} /* End of QBSS_LoadElementAppend */
#endif

/*
========================================================================
Routine Description:
	Append the QBSS Load element to the beacon frame.

Arguments:
	pAd					- WLAN control block pointer
	*pBeaconBuf			- the beacon or probe response frame

Return Value:
	the element total Length

Note:
	Append Condition: You must check whether WMM is enabled before the
	function is using.
========================================================================
*/
UINT32 QBSS_LoadElementAppend(RTMP_ADAPTER *pAd, UINT8 *pBeaconBuf, QLOAD_CTRL *pQloadCtrl)
{
	struct _ELM_QBSS_LOAD load, *pLoad = &load;
	ULONG ElmLen;

	/* check whether channel busy time calculation is enabled */
	if (pQloadCtrl->FlgQloadEnable == 0)
		return 0;

	/* init */
	pLoad->ElementId = ELM_QBSS_LOAD_ID;
	pLoad->Length = ELM_QBSS_LOAD_LEN;
	pLoad->StationCount = le2cpu16(MacTableAssocStaNumGet(pAd));
	pLoad->ChanUtil = pQloadCtrl->QloadChanUtil;
	/* because no ACM is supported, the available bandwidth is 1 sec */
	pLoad->AvalAdmCap = le2cpu16(0x7a12); /* 0x7a12 * 32us = 1 second */
	/* copy the element to the frame */
	MakeOutgoingFrame(pBeaconBuf, &ElmLen,
					  sizeof(struct _ELM_QBSS_LOAD),	pLoad,
					  END_OF_ARGS);
	return ElmLen;
}


/*
========================================================================
Routine Description:
	Update Channel Utilization.

Arguments:
	pAd					- WLAN control block pointer
	UpTime				- current up time

Return Value:
	None

Note:
	UpTime is used in QLOAD_FUNC_BUSY_TIME_STATS & QLOAD_FUNC_BUSY_TIME_ALARM

	If UpTime != 0, it means that the time period calling the function
	maybe not TBTT so we need to re-calculate the time period.

	If you call the function in kernel thread, the time period sometimes
	will not accurate due to kernel thread is not real-time, so we need to
	recalculate the time period.
========================================================================
*/
VOID QBSS_LoadUpdate(
	IN		RTMP_ADAPTER	*pAd,
	IN		ULONG			UpTime)
{
	UINT32 ChanUtilNu, ChanUtilDe;
	UINT32 BusyTime = 0;
	UINT32 BusyTimeId;
	UINT32 TimePeriod = pAd->CommonCfg.BeaconPeriod;
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	BOOLEAN FlgIsBusyOverThreshold = FALSE;
	BOOLEAN FlgIsAlarmNeeded = FALSE;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
	UINT8 UpdateBands = 1, i = 0;

	UpdateBands = (pAd->CommonCfg.dbdc_mode == 0)?1:2;

	for (i = 0; i < UpdateBands; i++) {
		if (pAd->CommonCfg.dbdc_mode != 0) {
			UCHAR PhyMode = HcGetRadioPhyModeByBandIdx(pAd, i);
			UINT32 RfIC = (WMODE_CAP_5G(PhyMode)) ? RFIC_5GHZ : RFIC_24GHZ;
			pQloadCtrl = HcGetQloadCtrlByRf(pAd, RfIC);
			if (!pQloadCtrl)
				continue;
		}

		/* check whether channel busy time calculation is enabled */
		if ((pQloadCtrl->FlgQloadEnable == 0) ||
			(pQloadCtrl->FlgQloadAlarmIsSuspended == TRUE))
			continue;

		/* calculate new time period if needed */
		if ((UpTime > 0) &&
			(pQloadCtrl->QloadUpTimeLast > 0) &&
			(UpTime > pQloadCtrl->QloadUpTimeLast)) {
			/* re-calculate time period */
			TimePeriod = (UINT32)(UpTime - pQloadCtrl->QloadUpTimeLast);
			/* translate to mini-second */
			TimePeriod = (TimePeriod*1000)/OS_HZ;
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM

			/* re-calculate QloadBusyTimeThreshold */
			if (TimePeriod != pQloadCtrl->QloadTimePeriodLast)
				QBSS_LoadAlarmBusyTimeThresholdReset(pAd, TimePeriod);

			pQloadCtrl->QloadTimePeriodLast = TimePeriod;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
		}

		/* update up time */
		pQloadCtrl->QloadUpTimeLast = UpTime;
		/* do busy time statistics */
#ifdef DOT11_N_SUPPORT
		{
			struct freq_oper oper;

			if (hc_radio_query_by_index(pAd, i, &oper) != HC_STATUS_OK)
				continue;

			if ((oper.ext_cha != 0) &&
				(oper.ht_bw != 0)) {
				/* in 20MHz, no need to check busy time of secondary channel */
				BusyTime = AsicGetChBusyCnt(pAd, 1);
				pQloadCtrl->QloadLatestChannelBusyTimeSec = BusyTime;
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
				BusyTimeId = BusyTime >> 10; /* translate us to ms */
				/* ex:95ms, 95*20/100 = 19 */
				BusyTimeId = (BusyTimeId*QLOAD_BUSY_INTERVALS)/TimePeriod;

				if (BusyTimeId >= QLOAD_BUSY_INTERVALS)
					BusyTimeId = QLOAD_BUSY_INTERVALS - 1;

				pQloadCtrl->QloadBusyCountSec[BusyTimeId]++;
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM

				if ((pQloadCtrl->FlgQloadAlarmIsSuspended == FALSE) &&
					(pQloadCtrl->QloadAlarmBusyTimeThreshold > 0)) {
					/* Alarm is not suspended and is enabled */
					if ((pQloadCtrl->QloadBusyTimeThreshold != 0) &&
						(BusyTime >= pQloadCtrl->QloadBusyTimeThreshold))
						FlgIsBusyOverThreshold = TRUE;
				}

#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
			}
		}

#endif /* DOT11_N_SUPPORT */
		/* do busy time statistics for primary channel */
		/* BusyTime = AsicGetChBusyCnt(pAd, 0); */
		/* update BusyTime from  OneSecMibBucket[i] */
		BusyTime = pAd->OneSecMibBucket.ChannelBusyTime[i];
		pQloadCtrl->QloadLatestChannelBusyTimePri = BusyTime;
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
		BusyTimeId = BusyTime >> 10; /* translate us to ms */
		/* ex:95ms, 95*20/100 = 19 */
		BusyTimeId = (BusyTimeId*QLOAD_BUSY_INTERVALS)/TimePeriod;

		if (BusyTimeId >= QLOAD_BUSY_INTERVALS)
			BusyTimeId = QLOAD_BUSY_INTERVALS - 1;

		pQloadCtrl->QloadBusyCountPri[BusyTimeId]++;
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM

		if ((pQloadCtrl->FlgQloadAlarmIsSuspended == FALSE) &&
			(pQloadCtrl->QloadAlarmBusyTimeThreshold > 0)) {
			/* Alarm is not suspended and is enabled */
			if ((pQloadCtrl->QloadBusyTimeThreshold != 0) &&
				(BusyTime >= pQloadCtrl->QloadBusyTimeThreshold))
				FlgIsBusyOverThreshold = TRUE;
		}

#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
		/* accumulate channel busy time for primary channel */
		pQloadCtrl->QloadChanUtilTotal += BusyTime;

		/* update new channel utilization for primary channel */
		if (++pQloadCtrl->QloadChanUtilBeaconCnt >= pQloadCtrl->QloadChanUtilBeaconInt) {
			ChanUtilNu = pQloadCtrl->QloadChanUtilTotal;
			ChanUtilNu *= 255;
			ChanUtilDe = pQloadCtrl->QloadChanUtilBeaconInt;
			ChanUtilDe *= 1000000; /* sec to us */
			pQloadCtrl->QloadChanUtil = (UINT8)(ChanUtilNu/ChanUtilDe);

			if ((ChanUtilNu/ChanUtilDe) >= 255)
				pQloadCtrl->QloadChanUtil = 255;

			/* re-accumulate channel busy time */
			pQloadCtrl->QloadChanUtilBeaconCnt = 0;
			pQloadCtrl->QloadChanUtilTotal = 0;
		}

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM

		/* check if alarm function is enabled */
		if ((pQloadCtrl->FlgQloadAlarmIsSuspended == FALSE) &&
			(pQloadCtrl->QloadAlarmBusyTimeThreshold > 0)) {
			/* Alarm is not suspended and is enabled */

			/* check if we need to issue a alarm */
			if (FlgIsBusyOverThreshold == TRUE) {
				if (pQloadCtrl->QloadAlarmDuration == 0) {
					/* last alarm ended so we can check new alarm */
					pQloadCtrl->QloadAlarmBusyNum++;

					if (pQloadCtrl->QloadAlarmBusyNum >= pQloadCtrl->QloadAlarmBusyNumThreshold) {
						/*
							The continued number of busy time >= threshold is larger
							than number threshold so issuing a alarm.
						*/
						FlgIsAlarmNeeded = TRUE;
						pQloadCtrl->QloadAlarmDuration++;
					}
				}
			} else
				pQloadCtrl->QloadAlarmBusyNum = 0;

			if (pQloadCtrl->QloadAlarmDuration > 0) {
				/*
					New alarm occurs so we can not re-issue new alarm during
					QBSS_LOAD_ALARM_DURATION * TBTT.
				*/
				if (++pQloadCtrl->QloadAlarmDuration >= QBSS_LOAD_ALARM_DURATION) {
					/* can re-issue next alarm */
					pQloadCtrl->QloadAlarmDuration = 0;
					pQloadCtrl->QloadAlarmBusyNum = 0;
				}
			}

			if (FlgIsAlarmNeeded == TRUE)
				QBSS_LoadAlarm(pAd);
		} else {
			/* clear statistics counts */
			pQloadCtrl->QloadAlarmBusyNum = 0;
			pQloadCtrl->QloadAlarmDuration = 0;
			pQloadCtrl->FlgQloadAlarm = FALSE;
		}

#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	}
}


/*
========================================================================
Routine Description:
	Clear QoS Load information.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID QBSS_LoadStatusClear(
	IN		RTMP_ADAPTER	*pAd,
	IN		UCHAR			Channel)
{
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	if (pAd->CommonCfg.dbdc_mode == 0)
		pQloadCtrl = HcGetQloadCtrl(pAd);
	else
		pQloadCtrl = (Channel > 14) ? HcGetQloadCtrlByRf(pAd, RFIC_5GHZ) : HcGetQloadCtrlByRf(pAd, RFIC_24GHZ);

	if (pQloadCtrl) {
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	/* clear busy time statistics */
		NdisZeroMemory(pQloadCtrl->QloadBusyCountPri, sizeof(pQloadCtrl->QloadBusyCountPri));
		NdisZeroMemory(pQloadCtrl->QloadBusyCountSec, sizeof(pQloadCtrl->QloadBusyCountSec));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
		/* clear alarm function variables */
		pQloadCtrl->QloadChanUtilTotal = 0;
		pQloadCtrl->FlgQloadAlarm = FALSE;
		pQloadCtrl->QloadAlarmBusyNum = 0;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	}
}


/*
========================================================================
Routine Description:
	Show QoS Load information.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT Show_QoSLoad_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	UINT32 BusyTimeId;
	UINT32 Time;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	Time = pAd->CommonCfg.BeaconPeriod / QLOAD_BUSY_INTERVALS;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\tPrimary Busy Time\tTimes\n"));

	for (BusyTimeId = 0; BusyTimeId < QLOAD_BUSY_INTERVALS; BusyTimeId++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%dms ~ %dms\t\t%d\n",
				 BusyTimeId*Time,
				 (BusyTimeId+1)*Time,
				 pQloadCtrl->QloadBusyCountPri[BusyTimeId]));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\tSecondary Busy Time\tTimes\n"));

	for (BusyTimeId = 0; BusyTimeId < QLOAD_BUSY_INTERVALS; BusyTimeId++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%dms ~ %dms\t\t%d\n",
				 BusyTimeId*Time,
				 (BusyTimeId+1)*Time,
				 pQloadCtrl->QloadBusyCountSec[BusyTimeId]));
	}

#else
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBusy time statistics is not included into the driver!\n"));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	return TRUE;
}


/*
========================================================================
Routine Description:
	Command for QoS Load information clear.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT Set_QloadClr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	QBSS_LoadStatusClear(pAd, 1);
	QBSS_LoadStatusClear(pAd, 36);
	return TRUE;
}


/*
========================================================================
Routine Description:
	Command for QoS Alarm Time Threshold set.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT Set_QloadAlarmTimeThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->QloadAlarmBusyTimeThreshold = (UCHAR)os_str_tol(Arg, 0, 10);
	QBSS_LoadAlarmReset(pAd);
	pQloadCtrl->QloadTimePeriodLast = pAd->CommonCfg.BeaconPeriod;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	return TRUE;
}


/*
========================================================================
Routine Description:
	Command for QoS Alarm Number Threshold set.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT Set_QloadAlarmNumThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pQloadCtrl->QloadAlarmBusyNumThreshold = (UCHAR)os_str_tol(Arg, 0, 10);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	return TRUE;
}

#endif /* AP_QLOAD_SUPPORT */

