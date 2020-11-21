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

typedef struct GNU_PACKED _ELM_QBSS_LOAD{

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

} ELM_QBSS_LOAD;

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
 	IN		RTMP_ADAPTER	*pAd);

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
/* handle a alarm */
static VOID QBSS_LoadAlarm(
 	IN		RTMP_ADAPTER	*pAd);
static VOID QBSS_LoadAlarmBusyTimeThresholdReset(
 	IN		RTMP_ADAPTER	*pAd,
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
 	IN		RTMP_ADAPTER	*pAd)
{
	/* suspend alarm until channel switch */
	QBSS_LoadAlarmSuspend(pAd);

	pAd->phy_ctrl.QloadAlarmNumber ++;

	/* check if we have already been 20M bandwidth */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if ((pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset != 0) &&
		(pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth != 0))
	{
		MAC_TABLE *pMacTable;
		UINT32 StaId;


		DBGPRINT(RT_DEBUG_TRACE, ("qbss> Alarm! Change to 20 bw...\n"));

		/* disassociate stations without D3 2040Coexistence function */
		pMacTable = &pAd->MacTab;

		for(StaId=1; StaId<MAX_LEN_OF_MAC_TABLE; StaId++)
		{
			MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[StaId];
			BOOLEAN bDisconnectSta = FALSE;

			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			if (pEntry->Sst != SST_ASSOC)
				continue;

			if (pEntry->BSS2040CoexistenceMgmtSupport)
				bDisconnectSta = TRUE;

			if (bDisconnectSta)
			{
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
					if (NStatus != NDIS_STATUS_SUCCESS)
					{
						DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock); */
						continue;
					}

					Reason = REASON_DEAUTH_STA_LEAVING;
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0,
									pEntry->Addr,
									pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_addr,
									pAd->ApCfg.MBSSID[pEntry->apidx].wdev.bssid);
				    	MakeOutgoingFrame(pOutBuffer, &FrameLen, 
				    	                  sizeof(HEADER_802_11), &DeAuthHdr, 
				    	                  2,                     &Reason, 
				    	                  END_OF_ARGS);
				    	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				    	MlmeFreeMemory(pAd, pOutBuffer);
				}

				DBGPRINT(RT_DEBUG_TRACE, ("qbss> Alarm! Deauth the station "
						"%02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pEntry->Addr)));

				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				continue;
			}
		}

		/* for 11n */
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;

		/* always 20M */
		pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;

		/* mark alarm flag */
		pAd->phy_ctrl.FlgQloadAlarm = TRUE;

		QBSS_LoadAlarmResume(pAd);
	}
	else
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	{
		/* we are in 20MHz bandwidth so try to switch channel */
		DBGPRINT(RT_DEBUG_TRACE, ("qbss> Alarm! Switch channel...\n"));

		/* send command to switch channel */
		RTEnqueueInternalCmd(pAd, CMDTHREAD_CHAN_RESCAN, NULL, 0);
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
 	IN		RTMP_ADAPTER	*pAd,
	IN		UINT32			TimePeriod)
{
	pAd->phy_ctrl.QloadBusyTimeThreshold = TimePeriod;
	pAd->phy_ctrl.QloadBusyTimeThreshold *= pAd->phy_ctrl.QloadAlarmBusyTimeThreshold;
	pAd->phy_ctrl.QloadBusyTimeThreshold /= 100;
	pAd->phy_ctrl.QloadBusyTimeThreshold <<= 10; /* translate mini-sec to micro-sec */
}
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */




/* --------------------------------- Public -------------------------------- */

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
VOID QBSS_LoadInit(
 	IN		RTMP_ADAPTER	*pAd)
{
	UINT32 IdBss;


	/* check whether any BSS enables WMM feature */
	for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
	{
		if ((pAd->ApCfg.MBSSID[IdBss].wdev.bWmmCapable)
#ifdef DOT11K_RRM_SUPPORT
			|| (IS_RRM_ENABLE(pAd, IdBss))
#endif /* DOT11K_RRM_SUPPORT */
			)
		{
			pAd->phy_ctrl.FlgQloadEnable = TRUE;
			break;
		}
	}

	if (pAd->phy_ctrl.FlgQloadEnable == TRUE)
	{
		/* Count EIFS, NAV, RX busy, TX busy as channel busy and
			enable Channel statistic timer (bit 0) */

		/* Note: if bit 0 == 0, the function will be disabled */
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, 0x0000001F);
#ifdef MT76x2
		if(IS_MT76x2(pAd))
		{			
			RTMP_IO_WRITE32(pAd, CH_TIME_CFG, 0x0000011F);
			/*ENABLE bit[9:8] = 1 ,to set CH_BUSY reg ReadClear*/
		}
#endif

		/* default value is 50, please reference to IEEE802.11e 2005 Annex D */
		pAd->phy_ctrl.QloadChanUtilBeaconInt = 50;
	}
	else
	{
		/* no any WMM is enabled */
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, 0x00000000);
	}

	pAd->phy_ctrl.QloadChanUtilTotal = 0;
	pAd->phy_ctrl.QloadUpTimeLast = 0;

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	/* clear busy time statistics */
	NdisZeroMemory(pAd->phy_ctrl.QloadBusyCountPri, sizeof(pAd->phy_ctrl.QloadBusyCountPri));
	NdisZeroMemory(pAd->phy_ctrl.QloadBusyCountSec, sizeof(pAd->phy_ctrl.QloadBusyCountSec));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	/* init threshold before QBSS_LoadAlarmReset */
	pAd->phy_ctrl.QloadAlarmBusyTimeThreshold = QBSS_LOAD_ALRAM_BUSY_TIME_THRESHOLD;
	pAd->phy_ctrl.QloadAlarmBusyNumThreshold = QBSS_LOAD_ALRAM_BUSY_NUM_THRESHOLD;

	QBSS_LoadAlarmReset(pAd);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
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
	pAd->phy_ctrl.FlgQloadAlarm = FALSE;
	pAd->phy_ctrl.QloadAlarmDuration = 0;
	pAd->phy_ctrl.QloadAlarmNumber = 0;

	pAd->phy_ctrl.FlgQloadAlarmIsSuspended = FALSE;

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
	pAd->phy_ctrl.FlgQloadAlarmIsSuspended = FALSE;
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
	pAd->phy_ctrl.FlgQloadAlarmIsSuspended = TRUE;
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
	if (pAd->phy_ctrl.QloadChanUtilBeaconCnt == 0)
		return pAd->phy_ctrl.QloadChanUtilTotal;

	return (pAd->phy_ctrl.QloadChanUtilTotal / pAd->phy_ctrl.QloadChanUtilBeaconCnt);
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
	BOOLEAN FlgQloadAlarm = pAd->phy_ctrl.FlgQloadAlarm;

	pAd->phy_ctrl.FlgQloadAlarm = FALSE;
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
	if (pAd->phy_ctrl.QloadAlarmBusyTimeThreshold == 0)
		return TRUE; /* always ok */

	if (BusyTime >= pAd->phy_ctrl.QloadBusyTimeThreshold)
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
	ELM_QBSS_LOAD load, *pLoad = &load;
	ULONG ElmLen;

	/* check whether channel busy time calculation is enabled */
	if (pAd->phy_ctrl.FlgQloadEnable == 0)
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
						sizeof(ELM_QBSS_LOAD),	pLoad,
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
UINT32 QBSS_LoadElementAppend(
 	IN		RTMP_ADAPTER	*pAd,
	OUT		UINT8			*pBeaconBuf)
{
	ELM_QBSS_LOAD load, *pLoad = &load;
	ULONG ElmLen;


	/* check whether channel busy time calculation is enabled */
	if (pAd->phy_ctrl.FlgQloadEnable == 0)
		return 0;

	/* init */
	pLoad->ElementId = ELM_QBSS_LOAD_ID;
	pLoad->Length = ELM_QBSS_LOAD_LEN;

	pLoad->StationCount = le2cpu16(MacTableAssocStaNumGet(pAd));
	pLoad->ChanUtil = pAd->phy_ctrl.QloadChanUtil;

	/* because no ACM is supported, the available bandwidth is 1 sec */
	pLoad->AvalAdmCap = le2cpu16(0x7a12); /* 0x7a12 * 32us = 1 second */
	

	/* copy the element to the frame */
    MakeOutgoingFrame(pBeaconBuf, &ElmLen,
						sizeof(ELM_QBSS_LOAD),	pLoad,
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
	
	UINT32  ChanUtilNu, ChanUtilDe;
	UINT32 BusyTime = 0;
	UINT32 BusyTimeId;
	UINT32 TimePeriod = pAd->CommonCfg.BeaconPeriod;
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	BOOLEAN FlgIsBusyOverThreshold = FALSE;
	BOOLEAN FlgIsAlarmNeeded = FALSE;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */


	/* check whether channel busy time calculation is enabled */
	if ((pAd->phy_ctrl.FlgQloadEnable == 0) ||
		(pAd->phy_ctrl.FlgQloadAlarmIsSuspended == TRUE))
		return;

	/* calculate new time period if needed */
	if ((UpTime > 0) &&
		(pAd->phy_ctrl.QloadUpTimeLast > 0) &&
		(UpTime > pAd->phy_ctrl.QloadUpTimeLast))
	{
		/* re-calculate time period */
		TimePeriod = (UINT32)(UpTime - pAd->phy_ctrl.QloadUpTimeLast);

		/* translate to mini-second */
		TimePeriod = (TimePeriod*1000)/OS_HZ;

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
		/* re-calculate phy_ctrl.QloadBusyTimeThreshold */
		if (TimePeriod != pAd->phy_ctrl.QloadTimePeriodLast)
			QBSS_LoadAlarmBusyTimeThresholdReset(pAd, TimePeriod);

		pAd->phy_ctrl.QloadTimePeriodLast = TimePeriod;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	}

	/* update up time */
	pAd->phy_ctrl.QloadUpTimeLast = UpTime;

	/* do busy time statistics */
#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset != 0) &&
		(pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth != 0))
	{
		/* in 20MHz, no need to check busy time of secondary channel */
		RTMP_IO_READ32(pAd, CH_BUSY_STA_SEC, &BusyTime);
		pAd->phy_ctrl.QloadLatestChannelBusyTimeSec = BusyTime;

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
		BusyTimeId = BusyTime >> 10; /* translate us to ms */

		/* ex:95ms, 95*20/100 = 19 */
		BusyTimeId = (BusyTimeId*QLOAD_BUSY_INTERVALS)/TimePeriod;

		if (BusyTimeId >= QLOAD_BUSY_INTERVALS)
			BusyTimeId = QLOAD_BUSY_INTERVALS - 1;

		pAd->phy_ctrl.QloadBusyCountSec[BusyTimeId] ++;
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
		if ((pAd->phy_ctrl.FlgQloadAlarmIsSuspended == FALSE) &&
			(pAd->phy_ctrl.QloadAlarmBusyTimeThreshold > 0))
		{
			/* Alarm is not suspended and is enabled */

			if ((pAd->phy_ctrl.QloadBusyTimeThreshold != 0) &&
				(BusyTime >= pAd->phy_ctrl.QloadBusyTimeThreshold))
			{
				FlgIsBusyOverThreshold = TRUE;
			}
		}
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
	}
#endif /* DOT11_N_SUPPORT */

	/* do busy time statistics for primary channel */
#ifdef CUSTOMER_DCC_FEATURE
	if(pAd->EnableChannelStatsCheck)
	{
		BusyTime = pAd->ChannelStats.ChBusyTime100msecValue;
	}
	else if(!(ApScanRunning(pAd)))
#endif
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &BusyTime);
	pAd->phy_ctrl.QloadLatestChannelBusyTimePri = BusyTime;
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	BusyTimeId = BusyTime >> 10; /* translate us to ms */

	/* ex:95ms, 95*20/100 = 19 */
	BusyTimeId = (BusyTimeId*QLOAD_BUSY_INTERVALS)/TimePeriod;

	if (BusyTimeId >= QLOAD_BUSY_INTERVALS)
		BusyTimeId = QLOAD_BUSY_INTERVALS - 1;

	pAd->phy_ctrl.QloadBusyCountPri[BusyTimeId] ++;
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	if ((pAd->phy_ctrl.FlgQloadAlarmIsSuspended == FALSE) &&
		(pAd->phy_ctrl.QloadAlarmBusyTimeThreshold > 0))
	{
		/* Alarm is not suspended and is enabled */

		if ((pAd->phy_ctrl.QloadBusyTimeThreshold != 0) &&
			(BusyTime >= pAd->phy_ctrl.QloadBusyTimeThreshold))
		{
			FlgIsBusyOverThreshold = TRUE;
		}
	}
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

	/* accumulate channel busy time for primary channel */
	pAd->phy_ctrl.QloadChanUtilTotal += BusyTime;

	/* update new channel utilization for primary channel */
	if (++pAd->phy_ctrl.QloadChanUtilBeaconCnt >= pAd->phy_ctrl.QloadChanUtilBeaconInt)
	{
		ChanUtilNu = pAd->phy_ctrl.QloadChanUtilTotal;
		ChanUtilNu *= 255;

		ChanUtilDe = pAd->phy_ctrl.QloadChanUtilBeaconInt;

		/*
			Still use pAd->CommonCfg.BeaconPeriod.
			Because we change QloadChanUtil not every TBTT.
		*/
		ChanUtilDe *= pAd->CommonCfg.BeaconPeriod;

		ChanUtilDe <<= 10; /* ms to us */
		pAd->phy_ctrl.QloadChanUtil = (UINT8)(ChanUtilNu/ChanUtilDe);
		if((ChanUtilNu/ChanUtilDe) >= 255)
			pAd->phy_ctrl.QloadChanUtil = 255;
		/* re-accumulate channel busy time */
		pAd->phy_ctrl.QloadChanUtilBeaconCnt = 0;
		pAd->phy_ctrl.QloadChanUtilTotal = 0;
	}

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	/* check if alarm function is enabled */
	if ((pAd->phy_ctrl.FlgQloadAlarmIsSuspended == FALSE) &&
		(pAd->phy_ctrl.QloadAlarmBusyTimeThreshold > 0))
	{
		/* Alarm is not suspended and is enabled */

		/* check if we need to issue a alarm */
		if (FlgIsBusyOverThreshold == TRUE)
		{
			if (pAd->phy_ctrl.QloadAlarmDuration == 0)
			{
				/* last alarm ended so we can check new alarm */

				pAd->phy_ctrl.QloadAlarmBusyNum ++;

				if (pAd->phy_ctrl.QloadAlarmBusyNum >= pAd->phy_ctrl.QloadAlarmBusyNumThreshold)
				{
					/*
						The continued number of busy time >= threshold is larger
						than number threshold so issuing a alarm.
					*/
					FlgIsAlarmNeeded = TRUE;
					pAd->phy_ctrl.QloadAlarmDuration ++;
				}
			}
		}
		else
			pAd->phy_ctrl.QloadAlarmBusyNum = 0;

		if (pAd->phy_ctrl.QloadAlarmDuration > 0)
		{
			/*
				New alarm occurs so we can not re-issue new alarm during
				QBSS_LOAD_ALARM_DURATION * TBTT.
			*/
			if (++pAd->phy_ctrl.QloadAlarmDuration >= QBSS_LOAD_ALARM_DURATION)
			{
				/* can re-issue next alarm */
				pAd->phy_ctrl.QloadAlarmDuration = 0;
				pAd->phy_ctrl.QloadAlarmBusyNum = 0;
			}
		}

		if (FlgIsAlarmNeeded == TRUE)
			QBSS_LoadAlarm(pAd);
	}
	else
	{
		/* clear statistics counts */
		pAd->phy_ctrl.QloadAlarmBusyNum = 0;
		pAd->phy_ctrl.QloadAlarmDuration = 0;
		pAd->phy_ctrl.FlgQloadAlarm = FALSE;
	}
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
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
 	IN		RTMP_ADAPTER	*pAd)
{
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	/* clear busy time statistics */
	NdisZeroMemory(pAd->phy_ctrl.QloadBusyCountPri, sizeof(pAd->phy_ctrl.QloadBusyCountPri));
	NdisZeroMemory(pAd->phy_ctrl.QloadBusyCountSec, sizeof(pAd->phy_ctrl.QloadBusyCountSec));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	/* clear alarm function variables */
	pAd->phy_ctrl.QloadChanUtilTotal = 0;
	pAd->phy_ctrl.FlgQloadAlarm = FALSE;
	pAd->phy_ctrl.QloadAlarmBusyNum = 0;
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
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
INT	Show_QoSLoad_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_STATS
	UINT32 BusyTimeId;
	UINT32 Time;


	Time = pAd->CommonCfg.BeaconPeriod / QLOAD_BUSY_INTERVALS;

	DBGPRINT(RT_DEBUG_OFF, ("\n\tPrimary Busy Time\tTimes\n"));

	for(BusyTimeId=0; BusyTimeId<QLOAD_BUSY_INTERVALS; BusyTimeId++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\t%dms ~ %dms\t\t%d\n",
				BusyTimeId*Time,
				(BusyTimeId+1)*Time,
				pAd->phy_ctrl.QloadBusyCountPri[BusyTimeId]));
	}

	DBGPRINT(RT_DEBUG_OFF, ("\n\tSecondary Busy Time\tTimes\n"));

	for(BusyTimeId=0; BusyTimeId<QLOAD_BUSY_INTERVALS; BusyTimeId++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\t%dms ~ %dms\t\t%d\n",
				BusyTimeId*Time,
				(BusyTimeId+1)*Time,
				pAd->phy_ctrl.QloadBusyCountSec[BusyTimeId]));
	}
#else

	DBGPRINT(RT_DEBUG_OFF, ("\tBusy time statistics is not included into the driver!\n"));
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

	DBGPRINT(RT_DEBUG_OFF, ("\n"));
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
INT	Set_QloadClr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			Arg)
{
	QBSS_LoadStatusClear(pAd);
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
INT	Set_QloadAlarmTimeThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			Arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	pAd->phy_ctrl.QloadAlarmBusyTimeThreshold = (UCHAR)simple_strtol(Arg, 0, 10);

	QBSS_LoadAlarmReset(pAd);

	pAd->phy_ctrl.QloadTimePeriodLast = pAd->CommonCfg.BeaconPeriod;
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
INT	Set_QloadAlarmNumThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			Arg)
{
#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
	pAd->phy_ctrl.QloadAlarmBusyNumThreshold = (UCHAR)simple_strtol(Arg, 0, 10);
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

	return TRUE;
}

#endif /* AP_QLOAD_SUPPORT */

