/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#include "rt_config.h"

/*----- 802.11H -----*/

/* Periodic Radar detection, switch channel will occur in RTMPHandleTBTTInterrupt()*/
/* Before switch channel, driver needs doing channel switch announcement.*/
VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd)
{
	INT i, ChIdx = 0;


	/* 
		1. APStart(), CalBufTime = 0;
		2. if bAnyUnavailableChannel, CalBufTime = DEFAULT_CAL_BUF_TIME;
		3. if Calibrated, CalBufTime = DEFAULT_CAL_BUF_TIME_MAX;
	*/
	for (i=0; i<pAd->ChannelListNum; i++)
	{

		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
		{
			ChIdx = i;
		}
	}


	if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
	{
		/* In Silent  Mode, RDCount is use to check with the CAC Time */
	if (pAd->Dot11_H.RDCount++ > pAd->Dot11_H.ChMovingTime &&
		pAd->ChannelList[ChIdx].RemainingTimeForUse == 0)
	{
			DBGPRINT(RT_DEBUG_TRACE,
					("Not found radar signal, start send beacon and radar detection in service monitor\n\n"));
		    pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
		    AsicEnableBssSync(pAd);
#ifdef MT76x2
	        if (IS_MT76x2(pAd)) {
			    
			    mt76x2_tssi_calibration(pAd, pAd->hw_cfg.cent_ch);
#ifdef TXBF_SUPPORT			    
			    if (pAd->hw_cfg.cent_ch > 14) 
			        rtmp_ate_txbf_fix_tank_code(pAd, pAd->hw_cfg.cent_ch, 0);  // load tank code from efuse, iBF only for A band
#endif /* TXBF_SUPPORT */
				mt76x2_calibration(pAd, pAd->hw_cfg.cent_ch);
			}
#endif /* MT76x2 */
			pAd->Dot11_H.RDCount = 0;
		}
	}
}

/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT 	i;
	BOOLEAN result = FALSE;

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (Ch == pAd->ChannelList[i].Channel)
		{
			result = pAd->ChannelList[i].DfsReq;
			break;
		}
	}

	return result;
}


/*
	========================================================================

	Routine Description:
		Determine the current radar state

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarStateCheck(
	IN PRTMP_ADAPTER	pAd)
{
	pAd->Dot11_H.CalBufTime = 0;

	if ((pAd->CommonCfg.Channel > 14) &&
		(pAd->CommonCfg.bIEEE80211H == 1) &&
		RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
	{
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
		pAd->Dot11_H.RDCount = 0;
		pAd->Dot11_H.InServiceMonitorCount = 0;
	}
	else
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;

#ifdef CARRIER_DETECTION_SUPPORT
	if ((pAd->CommonCfg.RDDurRegion == JAP)
		|| (pAd->CommonCfg.RDDurRegion == JAP_W53)
		|| (pAd->CommonCfg.RDDurRegion == JAP_W56))
	{

		if ((pAd->CommonCfg.Channel > 14) ||
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40))
		{
			pAd->CommonCfg.CarrierDetect.Enable = TRUE;
		}
	}
#endif /* CARRIER_DETECTION_SUPPORT */
}


ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};

	if (pAd->CommonCfg.RDDurRegion != JAP)
	{
		return pAd->CommonCfg.RDDurRegion;
	}

	for (i=0; i<15; i++)
	{
		if (pAd->CommonCfg.Channel == Channel[i])
		{
			break;
		}
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/

}


UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode)
{
	UCHAR ch = 0;
	INT ch_idx;

	switch (mode)
	{
		case 1: 
		{
			USHORT min_time = 0xFFFF;
			/* select channel with least RemainingTimeForUse */
			for ( ch_idx = 0; ch_idx <  pAd->ChannelListNum; ch_idx++)
			{
				if (pAd->ChannelList[ch_idx].RemainingTimeForUse < min_time)
				{
#ifdef DOT11_N_SUPPORT
					/* If the channel not in 40MHz/80MHz group, ignore it. */
					if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40 &&
						!(pAd->ChannelList[ch_idx].Flags & CHANNEL_40M_CAP))
						continue;
#ifdef DOT11_VHT_AC
					if (pAd->CommonCfg.vht_bw == VHT_BW_80 &&
						!(pAd->ChannelList[ch_idx].Flags & CHANNEL_80M_CAP))
						continue;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

					min_time = pAd->ChannelList[ch_idx].RemainingTimeForUse;
					ch = pAd->ChannelList[ch_idx].Channel;
				}
			}
			break;
		}

		default:
		{
			ch = FirstChannel(pAd);
			break;
		}
	}	

    DBGPRINT(RT_DEBUG_TRACE,("%s(): mode = %u, ch = %u\n",
							 __FUNCTION__, mode, ch));
	return ch;
}
		

#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Channel switching count down process upon radar detection

	Arguments:
		pAd 	Pointer to our adapter

	========================================================================
*/
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s():Channel Switching...(%d/%d)\n",
				__FUNCTION__, pAd->Dot11_H.CSCount, pAd->Dot11_H.CSPeriod));
	
	pAd->Dot11_H.CSCount++;
	if (pAd->Dot11_H.CSCount >= pAd->Dot11_H.CSPeriod)
	{
#ifdef DFS_SUPPORT
		pAd->CommonCfg.RadarDetect.DFSAPRestart = 1;
		schedule_dfs_task(pAd);
#else
		APStop(pAd);
		APStartUp(pAd);
#endif /* !DFS_SUPPORT */		
	}
}
#endif /* CONFIG_AP_SUPPORT */

/* 
    ==========================================================================
    Description:
        Set channel switch Period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->Dot11_H.CSPeriod = (USHORT) simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H.CSPeriod));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	USHORT Value;

	Value = (USHORT) simple_strtol(arg, 0, 10);

	pAd->Dot11_H.ChMovingTime = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: %d\n", __FUNCTION__,
		pAd->Dot11_H.ChMovingTime));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
		Reset channel block status.
	Arguments:
	    pAd				Pointer to our adapter
	    arg				Not used

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	INT i;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: Reset channel block status.\n", __FUNCTION__));	
	
	for (i=0; i<pAd->ChannelListNum; i++)
		pAd->ChannelList[i].RemainingTimeForUse = 0;

	return TRUE;
}


#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)

INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef DFS_SUPPORT
	UINT8 idx;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;
	PCHAR RDMode[]= {"Normal State", "Switching State", "Silent State"};

		printk("DFSUseTasklet = %d\n", pRadarDetect->use_tasklet);
		printk("McuRadarDebug = %x\n", (unsigned int)pRadarDetect->McuRadarDebug);
		printk("PollTime = %d\n", pRadarDetect->PollTime);
		printk("ChEnable = %d (0x%x)\n", pDfsProgramParam->ChEnable, pDfsProgramParam->ChEnable);
		printk("DeltaDelay = %d\n", pDfsProgramParam->DeltaDelay);
		printk("PeriodErr = %d\n", pDfsSwParam->dfs_period_err);
		printk("MaxPeriod = %d\n", (unsigned int)pDfsSwParam->dfs_max_period);
		printk("Ch0LErr = %d\n", pDfsSwParam->dfs_width_ch0_err_L);
		printk("Ch0HErr = %d\n", pDfsSwParam->dfs_width_ch0_err_H);
		printk("Ch1Shift = %d\n", pDfsSwParam->dfs_width_diff_ch1_Shift);
		printk("Ch2Shift = %d\n", pDfsSwParam->dfs_width_diff_ch2_Shift);
		printk("DfsRssiHigh = %d\n", pRadarDetect->DfsRssiHigh);
		printk("DfsRssiLow = %d\n", pRadarDetect->DfsRssiLow);
		printk("DfsSwDisable = %u\n", pRadarDetect->bDfsSwDisable);
		printk("CheckLoop = %d\n", pDfsSwParam->dfs_check_loop);
		printk("DeclareThres = %d\n", pDfsSwParam->dfs_declare_thres);
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("sw_idx[%u] = %u\n", idx, pDfsSwParam->sw_idx[idx]);
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("hw_idx[%u] = %u\n", idx, pDfsSwParam->hw_idx[idx]);

	printk("pAd->Dot11_H.ChMovingTime = %d\n", pAd->Dot11_H.ChMovingTime);
	printk("pAd->Dot11_H.RDMode = %s\n", RDMode[pAd->Dot11_H.RDMode]);
	printk("pAd->Dot11_H.RDCount = %d\n", pAd->Dot11_H.RDCount);
	printk("pAd->Dot11_H.CalBufTime = %lu\n", pAd->Dot11_H.CalBufTime);
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	printk("pAd->CommonCfg.CarrierDetect.CD_State = %d\n", pAd->CommonCfg.CarrierDetect.CD_State);
	printk("pAd->CommonCfg.CarrierDetect.criteria = %d\n", pAd->CommonCfg.CarrierDetect.criteria);
	printk("pAd->CommonCfg.CarrierDetect.Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	printk("pAd->CommonCfg.CarrierDetect.DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	printk("pAd->CommonCfg.CarrierDetect.Threshold = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
#endif /* CARRIER_DETECTION_SUPPORT */

	return TRUE;
}

/*
	========================================================================
       Routine Description:
               Control CCK_MRC Status
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID CckMrcStatusCtrl(IN PRTMP_ADAPTER pAd)
{
}


/*
       ========================================================================
       Routine Description:
               Enhance DFS/CS when using GLRT.
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID RadarGLRTCompensate(IN PRTMP_ADAPTER pAd)
{
}
#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) */

