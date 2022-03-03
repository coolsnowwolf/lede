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
    cmm_cs.c

    Abstract:
    Carrier Sensing related functions

    Revision History:
    Who       When            What
    ---------------------------------------------------------------------
*/
#include "rt_config.h"

#ifdef CARRIER_DETECTION_SUPPORT
static ULONG time[20];
static ULONG idle[20];
static ULONG busy[20];
static ULONG cd_idx=0;

static void ToneRadarProgram(PRTMP_ADAPTER pAd);

#ifdef CONFIG_AP_SUPPORT
static inline VOID CarrierDetectionResetStatus(PRTMP_ADAPTER pAd)
{
	if(pAd->chipCap.carrier_func == TONE_RADAR_V3)
	{
		RTMP_BBP_IO_WRITE32(pAd, TR_R1, 0x3);
	}
#ifdef RTMP_BBP
	else
	{
		RTMP_CARRIER_IO_WRITE8(pAd, 1, 1);
	}		
#endif /* RTMP_BBP */
}

static inline VOID CarrierDetectionStatusGet(PRTMP_ADAPTER pAd,
											PUINT8 pStatus)
{
	*pStatus = 0;

	if(pAd->chipCap.carrier_func == TONE_RADAR_V3)
	{
		UINT32 mac_value = 0;
		RTMP_BBP_IO_READ32(pAd, TR_R1, &mac_value);
		*pStatus = (UINT8) mac_value;
	}
#ifdef RTMP_BBP
	else
	{
		RTMP_CARRIER_IO_READ8(pAd, 1, pStatus);
	}
#endif /* RTMP_BBP */
}

static inline VOID CarrierDetectionEnable(PRTMP_ADAPTER pAd,
										BOOLEAN bEnable)
{
	if(pAd->chipCap.carrier_func == TONE_RADAR_V3)
	{
		RTMP_BBP_IO_WRITE32(pAd, TR_R0, bEnable);
	}
#ifdef RTMP_BBP
	else
	{
		RTMP_CARRIER_IO_WRITE8(pAd, 0, bEnable);
	}
#endif /* RTMP_BBP */
}


/* 
    ==========================================================================
    Description:
	Check current CS state, indicating Silient state (carrier exist) or not 
	Arguments:
	    pAd                    Pointer to our adapter

    Return Value:
        TRUE if the current state is SILENT state, FALSE other wise
    Note:
    ==========================================================================
*/
INT isCarrierDetectExist(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
		return TRUE;
	else
		return FALSE;
}		

/* 
    ==========================================================================
    Description:
        Enable or Disable Carrier Detection feature (AP ioctl).
	Arguments:
	    pAd                    Pointer to our adapter
	    arg                     Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set CarrierDetect=[1/0]
    ==========================================================================
*/
INT Set_CarrierDetect_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR apidx = pObj->ioctl_if;
	UINT Enable;

	if (apidx != MAIN_MBSSID)
		return FALSE;

	Enable = (UINT) simple_strtol(arg, 0, 10);

	pAd->CommonCfg.CarrierDetect.Enable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);
	
	RTMP_CHIP_RADAR_GLRT_COMPENSATE(pAd);
	RTMP_CHIP_CCK_MRC_STATUS_CTRL(pAd);	

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);
	else
		CarrierDetectionStop(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %s\n", __FUNCTION__,
		pAd->CommonCfg.CarrierDetect.Enable == TRUE ? "Enable Carrier Detection":"Disable Carrier Detection"));

	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */


/* 
    ==========================================================================
    Description:
        When h/w interrupt is not available for CS, this function monitor necessary parameters that determine the CS
        state periodically. (every 100ms)
        
	Arguments:
	    pAd                    Pointer to our adapter

    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID CarrierDetectionPeriodicStateCtrl(
	IN PRTMP_ADAPTER pAd)
{
	PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;
	CD_STATE *pCD_State = &pCarrierDetect->CD_State;
	ULONG *pOneSecIntCount = &pCarrierDetect->OneSecIntCount;

#ifdef RALINK_ATE		
	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

	if (pCarrierDetect->bCsInit == FALSE)
		return;

	if (pAd->chipCap.carrier_func == TONE_RADAR_V3)
	{
		UINT8 TrStatus = 0;
		
		CarrierDetectionStatusGet(pAd, &TrStatus);
		if (TrStatus)
		{
			if (*pCD_State == CD_NORMAL &&
				(--pCarrierDetect->recheck) == 0)
			{
				DBGPRINT(RT_DEBUG_OFF, 
						   ("Carrier Detected ! (TrStatus = 0x%x)\n", TrStatus));
				*pCD_State = CD_SILENCE;
				/* stop all TX actions including Beacon sending.*/
				AsicDisableSync(pAd);
			}
			else if (*pCD_State == CD_SILENCE)
			{
				*pOneSecIntCount  = pCarrierDetect->CarrierGoneThreshold;
			}
			CarrierDetectionResetStatus(pAd);
		}
		else
		{
			pCarrierDetect->recheck = pCarrierDetect->recheck1;
			*pOneSecIntCount  = 0;
		}
		/*CarrierDetectionResetStatus(pAd);*/
	}

#ifdef CARRIER_DETECTION_FIRMWARE_SUPPORT
			
	if (pAd->chipCap.carrier_func == TONE_RADAR_V2)
	{
		CARRIER_DETECT_PARAM CarrDetectParam;

	/* tell firmware to prepare Recheck and RadarToneCount  */
	AsicSendCommandToMcu(pAd, CD_CHECKOUT_MCU_CMD, 0xff, 0x00, 0x00, FALSE);
	/* Debug */
	if (pAd->CommonCfg.CarrierDetect.Debug == RT_DEBUG_TRACE)
	{
		CARRIER_DETECT_DEBUG CarrDetectDebug;
		RTUSBMultiRead(pAd, 0x4CB0, (PUCHAR) &CarrDetectDebug, sizeof(CarrDetectDebug));
		printk("delta_div = 0x%02X, rRadarToneCount = %u, Recheck = %u, Criteria = %u, Threshold = 0x%08X, VGA_Mask = 0x%04X\n",
				CarrDetectDebug.delta_div,
				CarrDetectDebug.RadarToneCount,
				CarrDetectDebug.ReCheck,
				CarrDetectDebug.Criteria << 6, /* ms -> 16us*/
				CarrDetectDebug.Threshold,
				CarrDetectDebug.VGA_Mask);
	}

	RTUSBMultiRead(pAd, RADAR_TONE_COUNT, (PUCHAR) &CarrDetectParam, sizeof(CarrDetectParam));
	switch(*pCD_State)
	{
		case CD_NORMAL:
			if (CarrDetectParam.ReCheck == 0)
			{
				*pCD_State = CD_SILENCE;				
				if (pAd->CommonCfg.CarrierDetect.Debug != RT_DEBUG_TRACE)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Carrier Detected\n"));
				
					/* stop all TX actions including Beacon sending.*/
					AsicDisableSync(pAd);
				}
				else
				printk("Carrier Detected\n");
			}
			break;
			
		case CD_SILENCE:
                     *pOneSecIntCount += CarrDetectParam.RadarToneCount;
			break;
			
		default:
			break;
	}			 
}
#endif /* CARRIER_DETECTION_FIRMWARE_SUPPORT */

}


/* 
    ==========================================================================
    Description:
        When there is no f/w taking care of CS operation, this function depends on h/w interrupts for every possible carrier
        tone to judge the CS state 
                
	Arguments:
	    pAd                    Pointer to our adapter

    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID RTMPHandleRadarInterrupt(PRTMP_ADAPTER  pAd)
{
	UINT32 value, delta;
	UCHAR bbp=0;
	PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;

	if (pAd->chipCap.carrier_func > TONE_RADAR_V2)
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandleRadarInterrupt()\n"));
	RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &value);
	RTMP_IO_READ32(pAd, CH_IDLE_STA, &pCarrierDetect->idle_time);
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &pCarrierDetect->busy_time);
	delta = (value >> 4) - pCarrierDetect->TimeStamp;
	pCarrierDetect->TimeStamp = value >> 4;
	pCarrierDetect->OneSecIntCount++;

	if(pAd->chipCap.carrier_func==TONE_RADAR_V2)
	{
#ifdef RTMP_BBP
		CarrierDetectionStatusGet(pAd, &bbp);
		if (!(bbp & 0x1))
			return;
		else
#endif /* RTMP_BBP */
		{
			UINT32 RadarInt = 0;
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT) 
				RadarInt = RLT_RadarInt;
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
				RadarInt = RTMP_RadarInt;
#endif /* RTMP_MAC */
			/* Disable carrier detection and clear the status bit*/
			CarrierDetectionEnable(pAd, 0);
			CarrierDetectionResetStatus(pAd);
			/* Clear interrupt */
			RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, RadarInt);
		}
	}

	if (pCarrierDetect->Debug)
	{
		if (cd_idx < 20)
		{
			time[cd_idx] = delta;
			idle[cd_idx] = pCarrierDetect->idle_time;
			busy[cd_idx] = pCarrierDetect->busy_time;
			cd_idx++;
		}
		else
		{
			int i;
			pCarrierDetect->Debug = 0;
			for (i = 0; i < 20; i++)
			{
				printk("%3d %4ld %ld %ld\n", i, time[i], idle[i], busy[i]);
			}
			cd_idx = 0;
			
		}
	}

	if (pCarrierDetect->CD_State == CD_NORMAL)
	{
		if ((delta < pCarrierDetect->criteria) && (pCarrierDetect->recheck))
			pCarrierDetect->recheck --;
		else
			pCarrierDetect->recheck = pCarrierDetect->recheck1;
		
		if (pCarrierDetect->recheck == 0)
		{
			/* declare carrier sense*/
			pCarrierDetect->CD_State = CD_SILENCE;

			if (pCarrierDetect->Debug != RT_DEBUG_TRACE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Carrier Detected\n"));
				
				/* stop all TX actions including Beacon sending.*/
				AsicDisableSync(pAd);
			}
			else
			{
				printk("Carrier Detected\n");
			}
		}
	}

	if(pAd->chipCap.carrier_func == TONE_RADAR_V2)
	{
		CarrierDetectionStatusGet(pAd, &bbp);

		if (bbp & 0x1)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("CS bit not cleared!!!\n"));
			CarrierDetectionResetStatus(pAd);
		}
		/* re-enable carrier detection */
		CarrierDetectionEnable(pAd, 1);
	}
	else if(pAd->chipCap.carrier_func == TONE_RADAR_V1 &&
		pCarrierDetect->Enable)
	{
		ToneRadarProgram(pAd);
	}
}

/* 
    ==========================================================================
    Description:
	Reset CS state to NORMAL state.
	Arguments:
	    pAd                    Pointer to our adapter

    Return Value:
        None

    Note:
    ==========================================================================
*/
INT CarrierDetectReset(
	IN PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;
	return 0;
}

/* 
    ==========================================================================
    Description:
	Criteria in CS is a timing difference threshold for a pair of carrier tones. This function is a ioctl uesed to adjust the 
	Criteria. (unit: 16us)
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT Set_CarrierCriteria_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING			arg)
{
	UINT32 Value;

	Value = simple_strtol(arg, 0, 10);

	pAd->CommonCfg.CarrierDetect.criteria = Value;
#ifdef CARRIER_DETECTION_FIRMWARE_SUPPORT
	{
		USHORT sVal = (USHORT) (Value >> 6); /* convert unit from 16us to ms:(2^4 /2^10)  */
		RTUSBMultiWrite(pAd, CD_CRITERIA, (PUCHAR) &sVal, 2, FALSE);
		/* send enable cmd to mcu to take effect */
		AsicSendCommandToMcu(pAd, CD_ONOFF_MCU_CMD, 0xff, 0x01, 0x00, FALSE);
	}
#endif /* CARRIER_DETECTION_FIRMWARE_SUPPORT */
	return TRUE;
}

/* 
    ==========================================================================
    Description:
	ReCheck in CS is a value indicating how many continuous incoming carrier tones is enough us to announce  that there 
	is carrier tone (and hence enter SILENT state). This function is a ioctl uesed to adjust the ReCheck value.
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT Set_CarrierReCheck_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	pAd->CommonCfg.CarrierDetect.recheck1 = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("Set Recheck = %u\n", pAd->CommonCfg.CarrierDetect.recheck1));
#ifdef CARRIER_DETECTION_FIRMWARE_SUPPORT
	RTMP_IO_WRITE8(pAd, CD_CHECK_COUNT, pAd->CommonCfg.CarrierDetect.recheck1);
	/* send enable cmd to mcu to take effect */
	AsicSendCommandToMcu(pAd, CD_ONOFF_MCU_CMD, 0xff, 0x01, 0x00, FALSE);
#endif /* CARRIER_DETECTION_FIRMWARE_SUPPORT */
	
	return TRUE;
}

/* 
    ==========================================================================
    Description:
	CarrierGoneThreshold is used to determine whether we should leave SILENT state. When the number of carrier 
	tones in a certain period of time is less than CarrierGoneThreshold, we should return to NORMAL state. This function 
	is a ioctl uesed to adjust the CarrierGoneThreshold.
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT Set_CarrierGoneThreshold_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("Set CarrierGoneThreshold = %u\n", pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold));
	return TRUE;
}

/* 
    ==========================================================================
    Description:
	Setting up the carrier debug level. set 0 means to turning off the carrier debug
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT	Set_CarrierDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.Debug = simple_strtol(arg, 0, 10);
	printk("pAd->CommonCfg.CarrierDetect.Debug = %ld\n", pAd->CommonCfg.CarrierDetect.Debug);
	return TRUE;
}

/* 
    ==========================================================================
    Description:
	Delta control the delay line characteristic of the cross correlation energy calculation.
	This function is a ioctl uesed to adjust the Delta value.
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT	Set_CarrierDelta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.delta = simple_strtol(arg, 0, 10);
	printk("Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	CarrierDetectionStart(pAd);

	return TRUE;
}

/* 
	==========================================================================
	Description:
	To set ON/OFF of the "Not Divide Flag"
	
	Arguments:
		pAd 		Pointer to our adapter
		arg 		Pointer to the ioctl argument
		
	Return Value:
		None

	Note:
	==========================================================================
*/
INT	Set_CarrierDivFlag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.div_flag = simple_strtol(arg, 0, 10);
	printk("DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	CarrierDetectionStart(pAd);

	return TRUE;
}

/* 
    ==========================================================================
    Description:
	Carrier Threshold is the energy threshold for h/w to determine a carrier tone or not.
	This function is a ioctl uesed to adjust the Threshold value.
	
	Arguments:
	    pAd			Pointer to our adapter
	    arg			Pointer to the ioctl argument
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
INT	Set_CarrierThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.threshold = simple_strtol(arg, 0, 10);
	printk("CarrThrd = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
	CarrierDetectionStart(pAd);

	return TRUE;
}

/* 
	==========================================================================
	Description:
	Carrier SymRund is the number of round bits in Radar Symmetric Round Bits Option.
	This function is a ioctl uesed to adjust the SymRund. (unit: bit)
	
	Arguments:
		pAd 		Pointer to our adapter
		arg 		Pointer to the ioctl argument
		
	Return Value:
		None

	Note:
	==========================================================================
*/
INT	Set_CarrierSymRund_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.SymRund= simple_strtol(arg, 0, 10);
	printk("SymRund = %d\n", pAd->CommonCfg.CarrierDetect.SymRund);
	CarrierDetectionStart(pAd);

	return TRUE;
}

/* 
	==========================================================================
	Description:
	Carrier Masks are used to prevent false trigger while doing Rx_PE, Packet_End, and AGC tuning.
	This function is a ioctl uesed to adjust these three mask. (unit: 100ns)
	
	Arguments:
		pAd 		Pointer to our adapter
		arg 		Pointer to the ioctl argument
		
	Return Value:
		None

	Note:
	==========================================================================
*/
INT Set_CarrierMask_Proc(
        IN      PRTMP_ADAPTER   pAd,
        IN      PSTRING                 arg)
{
	pAd->CommonCfg.CarrierDetect.VGA_Mask = simple_strtol(arg, 0, 10);
	pAd->CommonCfg.CarrierDetect.Packet_End_Mask = simple_strtol(arg, 0, 10);
	pAd->CommonCfg.CarrierDetect.Rx_PE_Mask = simple_strtol(arg, 0, 10);
	printk("CarrMask = %u(%x)\n", pAd->CommonCfg.CarrierDetect.VGA_Mask, pAd->CommonCfg.CarrierDetect.VGA_Mask);
	CarrierDetectionStart(pAd);

	return TRUE;
}

/* 
    ==========================================================================
    Description:
	Initialize CS parameters.
	
	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID CSInit(
		IN PRTMP_ADAPTER pAd)
{
	PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;

	pCarrierDetect->TimeStamp = 0;
	pCarrierDetect->recheck = pCarrierDetect->recheck1;
	pCarrierDetect->OneSecIntCount = 0;
	pCarrierDetect->bCsInit = FALSE;
}

/* 
    ==========================================================================
    Description:
	To trigger CS start
	
	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID CarrierDetectionStart(PRTMP_ADAPTER pAd)
{		
	/*ULONG Value;*/
	/* Enable Bandwidth usage monitor*/
	DBGPRINT(RT_DEBUG_TRACE, ("CarrierDetectionStart\n"));
	/*RTMP_IO_READ32(pAd, CH_TIME_CFG, &Value);*/
	/*RTMP_IO_WRITE32(pAd, CH_TIME_CFG, Value | 0x1f);	*/

	/* Init Carrier Detect*/
	if (pAd->CommonCfg.CarrierDetect.Enable)
	{
		CSInit(pAd);
		ToneRadarProgram(pAd);
#ifdef CARRIER_DETECTION_FIRMWARE_SUPPORT
		{
		USHORT criteria = (USHORT) (pAd->CommonCfg.CarrierDetect.criteria >> 6); /* convert unit from 16us to 1ms:(2^4 /2^10)  */
		RTUSBMultiWrite(pAd, CD_CRITERIA, (PUCHAR) &criteria, 2, FALSE);
		RTMP_IO_WRITE8(pAd, CD_CHECK_COUNT, pAd->CommonCfg.CarrierDetect.recheck1);
		AsicSendCommandToMcu(pAd, CD_ONOFF_MCU_CMD, 0xff, 0x01, 0x00, FALSE);
		}
#else 
#ifndef MT76x2
		/* trun on interrupt polling for pcie device */
		if (pAd->infType == RTMP_DEV_INF_PCIE &&
			 pAd->chipCap.carrier_func == TONE_RADAR_V2)
		{
			AsicSendCommandToMcu(pAd, CD_INT_POLLING_CMD, 0xff, 0x01, 0x00, FALSE);
		}
#endif /* MT76x2 */
#endif /* CARRIER_DETECTION_FIRMWARE_SUPPORT */	
		pAd->CommonCfg.CarrierDetect.bCsInit = TRUE;
	}
}

/* 
    ==========================================================================
    Description:
	To stop CS
	
	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID CarrierDetectionStop(IN PRTMP_ADAPTER	pAd)
{
	CarrierDetectReset(pAd);
	CarrierDetectionEnable(pAd, 0);
#ifdef CARRIER_DETECTION_FIRMWARE_SUPPORT
	/* Stop firmware CS action */
	AsicSendCommandToMcu(pAd, CD_ONOFF_MCU_CMD, 0xff, 0x00, 0x00, FALSE);
#endif /* CARRIER_DETECTION_FIRMWARE_SUPPORT */
	if (pAd->chipCap.carrier_func == TONE_RADAR_V3 
#ifdef DFS_SUPPORT
		&& pAd->CommonCfg.RadarDetect.bDfsInit == FALSE
#endif
	)
	{
		RTMP_IO_WRITE32(pAd, 0x212C, 0x00);
	}
	return;
}

/* 
    ==========================================================================
    Description:
	To program CS related BBP registers (CS initialization)

	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
static VOID ToneRadarProgram(PRTMP_ADAPTER pAd)
{
	ULONG threshold;
	/* if wireless mode is 20Mhz mode, then the threshold should div by 2 */	
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20)		
		threshold = pAd->CommonCfg.CarrierDetect.threshold >> 1;	
	else		
		threshold = pAd->CommonCfg.CarrierDetect.threshold;
	/* Call ToneRadarProgram_v1/ToneRadarProgram_v2*/
	RTMP_CHIP_CARRIER_PROGRAM(pAd, threshold);
}


#ifdef RTMP_BBP
/* 
    ==========================================================================
    Description:
	To program CS v1 related BBP registers (CS initialization)

	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID ToneRadarProgram_v1(PRTMP_ADAPTER pAd, ULONG threshold)
{
	UCHAR bbp;

	DBGPRINT(RT_DEBUG_TRACE, ("ToneRadarProgram v1\n"));
	/* programe delta delay & division bit*/
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0xf0);
	bbp = pAd->CommonCfg.CarrierDetect.delta << 4;
	bbp |= (pAd->CommonCfg.CarrierDetect.div_flag & 0x1) << 3;
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, bbp);

	/* program threshold*/
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x34);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (threshold & 0xff000000) >> 24);

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x24);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (threshold & 0xff0000) >> 16);

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x14);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (threshold & 0xff00) >> 8);

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x04);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, threshold & 0xff);

	/* ToneRadarEnable v1 */
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x05);
}

/* 
    ==========================================================================
    Description:
	To program CS v2 related BBP registers (CS initialization)

	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID ToneRadarProgram_v2(PRTMP_ADAPTER pAd, ULONG threshold)
{
	UCHAR bbp;

	/* programe delta delay & division bit*/
	DBGPRINT(RT_DEBUG_TRACE, ("ToneRadarProgram v2\n"));
	bbp = pAd->CommonCfg.CarrierDetect.delta |							\
			((pAd->CommonCfg.CarrierDetect.SymRund & 0x3) << 4)	 |		\
			((pAd->CommonCfg.CarrierDetect.div_flag & 0x1) << 6) |		\
			0x80;	/* Full 40MHz Detection Mode */
	RTMP_CARRIER_IO_WRITE8(pAd, 5, bbp);
	
	/* program *_mask*/
	RTMP_CARRIER_IO_WRITE8(pAd, 2, pAd->CommonCfg.CarrierDetect.VGA_Mask);
	RTMP_CARRIER_IO_WRITE8(pAd, 3, pAd->CommonCfg.CarrierDetect.Packet_End_Mask);
	RTMP_CARRIER_IO_WRITE8(pAd, 4, pAd->CommonCfg.CarrierDetect.Rx_PE_Mask);

	/* program threshold*/
	RTMP_CARRIER_IO_WRITE8(pAd, 6, threshold & 0xff);
	RTMP_CARRIER_IO_WRITE8(pAd, 7, (threshold & 0xff00) >> 8);
	RTMP_CARRIER_IO_WRITE8(pAd, 8, (threshold & 0xff0000) >> 16);
	RTMP_CARRIER_IO_WRITE8(pAd, 9, (threshold & 0xff000000) >> 24);

	/* ToneRadarEnable v2 */
	CarrierDetectionEnable(pAd, 1);	
}
#endif /* RTMP_BBP */

/* 
    ==========================================================================
    Description:
	To program CS v3 related BBP registers (CS initialization)

	Arguments:
	    pAd			Pointer to our adapter
	    
    Return Value:
        None

    Note:
    ==========================================================================
*/
VOID ToneRadarProgram_v3(PRTMP_ADAPTER pAd, ULONG threshold)
{
	/* 
		Carrier Sense (Tone Radar) BBP initialization 
		(MT7650 Carrier sense programming guide_v1_20120824.docx)
	*/
	DBGPRINT(RT_DEBUG_TRACE, ("ToneRadarProgram v3\n"));
	CarrierDetectionEnable(pAd, 0);	
	RTMP_BBP_IO_WRITE32(pAd, TR_R2, 0x002d002d);
	RTMP_BBP_IO_WRITE32(pAd, TR_R3, 0x0003002d);
	RTMP_BBP_IO_WRITE32(pAd, TR_R5, 0x80000000);
	RTMP_BBP_IO_WRITE32(pAd, TR_R6, 0x80100000);
	CarrierDetectionEnable(pAd, 1);
	RTMP_IO_WRITE32(pAd, 0x212C, 0x0c350001);

}


#endif /* CARRIER_DETECTION_SUPPORT */

