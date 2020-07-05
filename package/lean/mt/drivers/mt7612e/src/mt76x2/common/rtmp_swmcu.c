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
	rtmp_swmcu.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#ifdef CONFIG_SWMCU_SUPPORT
#include	"rt_config.h"



int RtmpAsicSendCommandToSwMcu(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR Command, 
	IN UCHAR Token,
	IN UCHAR Arg0, 
	IN UCHAR Arg1,
	IN BOOLEAN FlgIsNeedLocked)
{
	BBP_CSR_CFG_STRUC  BbpCsr, BbpCsr2;
	int             j, k;
#ifdef LED_CONTROL_SUPPORT
	UINT16 Temp;
	PSWMCU_LED_CONTROL pSWMCULedCntl = &pAd->LedCntl.SWMCULedCntl;
#endif /* LED_CONTROL_SUPPORT */
	
	switch(Command)
	{
		case 0x80:
			RTMP_IO_READ32(pAd, H2M_BBP_AGENT, &BbpCsr.word);
			if ((BbpCsr.field.Busy != 1) || (BbpCsr.field.BBP_RW_MODE != 1))
				DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 1\n"));
			
			for (j=0; j<MAX_BUSY_COUNT; j++)
			{
				RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr2.word);
				if (BbpCsr2.field.Busy == BUSY)
				{
					continue;
				}
				BbpCsr2.word = BbpCsr.word;
				RTMP_IO_WRITE32(pAd, BBP_CSR_CFG, BbpCsr2.word);
				
				
				if (BbpCsr.field.fRead == 1)
				{
					/* read*/
					for (k=0; k<MAX_BUSY_COUNT; k++)
					{
						RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr2.word);
						if (BbpCsr2.field.Busy == IDLE)
							break;
					}
					if (k == MAX_BUSY_COUNT)
						DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 2\n"));
					
					if ((BbpCsr2.field.Busy == IDLE) && (BbpCsr2.field.RegNum == BbpCsr.field.RegNum))
					{
						BbpCsr.field.Value = BbpCsr2.field.Value;
						BbpCsr.field.Busy = IDLE;
						RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
						break;
					}
				}
				else
				{
					/*write*/
					BbpCsr.field.Busy = IDLE;
					RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
					pAd->BbpWriteLatch[BbpCsr.field.RegNum] = BbpCsr2.field.Value;
					break;
				}
			}
			
			if (j == MAX_BUSY_COUNT)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 3\n"));
				if (BbpCsr.field.Busy != IDLE)
				{
					BbpCsr.field.Busy = IDLE;
					RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
				}
			}
			break;
		case 0x30:
			break;
		case 0x31:
			break;
#ifdef LED_CONTROL_SUPPORT
		case MCU_SET_LED_MODE:
			pSWMCULedCntl->LedParameter.LedMode = Arg0;
	        pSWMCULedCntl->LinkStatus = Arg1;
			SetLedLinkStatus(pAd);
	                        break;
		case MCU_SET_LED_GPIO_SIGNAL_CFG:
			pSWMCULedCntl->GPIOPolarity = Arg1;
			pSWMCULedCntl->SignalStrength = Arg0;
	                        break;
		case MCU_SET_LED_AG_CFG:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&pSWMCULedCntl->LedParameter.LedAgCfg, &Temp, 2);
			break;
		case MCU_SET_LED_ACT_CFG:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&pSWMCULedCntl->LedParameter.LedActCfg, &Temp, 2);
			break;
		case MCU_SET_LED_POLARITY:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&pSWMCULedCntl->LedParameter.LedPolarityCfg, &Temp, 2);
			break;
#endif /* LED_CONTROL_SUPPORT */

		default:
			break;
	}

	return 0;
}

#endif /* CONFIG_SWMCU_SUPPORT */
