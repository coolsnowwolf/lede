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
	mt_phy.c
*/


#include "rt_config.h"
static INT32 MTBbpInit(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Init BBP Registers\n", __FUNCTION__));

	if (pAd->chipOps.AsicBbpInit != NULL)
		pAd->chipOps.AsicBbpInit(pAd);

	return NDIS_STATUS_SUCCESS;
}


INT32 MTShowPartialBBP(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 Offset, Value;

	for (Offset = Start; Offset <= End; Offset += 4)
	{
		RTMP_IO_READ32(pAd, Offset, &Value);
		DBGPRINT(RT_DEBUG_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__,  __LINE__,Offset, Value));
	}

	return TRUE;
}


INT32 MTShowAllBBP(RTMP_ADAPTER *pAd)
{
	UINT32 Offset, Value;

	for (Offset = 0x10000; Offset <= 0x20000; Offset += 4)
	{
		RTMP_IO_READ32(pAd, Offset, &Value);
		DBGPRINT(RT_DEBUG_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__,  __LINE__,Offset, Value));
	}

	return TRUE;
}


static INT mt_bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw)
{
	INT ret = FALSE;

	pAd->CommonCfg.BBPCurrentBW = bw;

	if ((bw == BW_20) || (bw == BW_40))
	{
		pAd->CommonCfg.BBPCurrentBW = bw;


		ret = TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("unknow bw setting = %d\n", bw));
		ret = FALSE;
	}

	return ret;
}
#ifdef SMART_CARRIER_SENSE_SUPPORT
INT MTSmartCarrierSense(RTMP_ADAPTER *pAd)
{
	UINT16 RxRatio=0;
	UCHAR idx = 0, RtsEnable = 0, Action = Keep_Range;
	UINT32 CrValue = 0, MaxRtsRtyCount = 0, MaxRtsCount = 0, TempValue = 0, tmpCrValue=0, TotalTp=0;
	INT32	AdjustStep=0;
	CHAR	RSSIBoundary = 0;
		RxRatio = ((pAd->RalinkCounters.OneSecReceivedByteCount) * 100 / (pAd->RalinkCounters.OneSecReceivedByteCount + pAd->RalinkCounters.OneSecTransmittedByteCount));
	//DBGPRINT(RT_DEBUG_ERROR, ("%s():Enter ---> BssNr=%d, miniRSSI=%d, TotalByteCount=%d RxByteCount=%d TxByteCount=%d EDCCA=%d RxRatio=%d\n", __FUNCTION__,
	//	pAd->SCSCtrl.SCSBssTab.BssNr, pAd->SCSCtrl.SCSMinRssi, (pAd->RalinkCounters.OneSecReceivedByteCount + pAd->RalinkCounters.OneSecTransmittedByteCount), 
	//	pAd->RalinkCounters.OneSecReceivedByteCount, pAd->RalinkCounters.OneSecTransmittedByteCount,
	//	pAd->SCSCtrl.EDCCA_Status, RxRatio));

	for (idx = 0; idx < 4; idx++)
	{
		RTMP_IO_READ32(pAd, MIB_MB0SDR0 + idx * 0x10, &CrValue);
		TempValue = (CrValue >> 16) & 0x0000ffff;
		if (TempValue > MaxRtsRtyCount) {                                
			MaxRtsRtyCount = TempValue;
			MaxRtsCount = CrValue & 0x0000ffff;
		}

		pAd->SCSCtrl.RtsCount = MaxRtsCount;
		pAd->SCSCtrl.RtsRtyCount = MaxRtsRtyCount;
	}
	
	if (MaxRtsCount > 0 || MaxRtsRtyCount >0)
		RtsEnable=1;
	
	RSSIBoundary = pAd->SCSCtrl.SCSMinRssi - pAd->SCSCtrl.SCSMinRssiTolerance;
	RSSIBoundary -= (RSSIBoundary % 2);
	pAd->SCSCtrl.RSSIBoundary = min (RSSIBoundary, pAd->SCSCtrl.FixedRssiBond);
	TotalTp = pAd->RalinkCounters.OneSecReceivedByteCount + pAd->RalinkCounters.OneSecTransmittedByteCount;
	
	if ((((pAd->SCSCtrl.ForceMode == 1) && (TotalTp > pAd->SCSCtrl.SCSTrafficThreshold)) || (RtsEnable ==1)) 
		&& ((pAd->MacTab.Size > 0) && (pAd->SCSCtrl.SCSMinRssi < 0) && (RxRatio < 90)))
	{
		if (RtsEnable ==1) /* Consider RTS PER & False-CCA */
		{
			if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1)) && (pAd ->SCSCtrl.FalseCCA) > (pAd ->SCSCtrl.FalseCcaUpBond))
				Action = Decrease_Range;
			else if ((MaxRtsCount + (MaxRtsCount >>1)) < MaxRtsRtyCount || (pAd ->SCSCtrl.FalseCCA) <(pAd ->SCSCtrl.FalseCcaLowBond))
				Action = Increase_Range;
			else
				Action = Keep_Range;
		}
		else /* Consider False-CCA only */
		{
			if (pAd ->SCSCtrl.FalseCCA >(pAd ->SCSCtrl.FalseCcaUpBond))
				Action = Decrease_Range;
			else if ( pAd ->SCSCtrl.FalseCCA <(pAd ->SCSCtrl.FalseCcaLowBond))
				Action = Increase_Range;
			else
				Action = Keep_Range;
		
		}

		
		DBGPRINT(RT_DEBUG_TRACE, ("Action=%d, RtsCount=%d, RtsRtyCount=%d, FalseCCA=%d \n", Action, MaxRtsCount, MaxRtsRtyCount, pAd ->SCSCtrl.FalseCCA));
		
			if (Action == Decrease_Range)
			{ 	
				if (pAd->SCSCtrl.SCSStatus == SCS_STATUS_DEFAULT)
				{ /*First time inital */
					if (pAd ->SCSCtrl.SCSMinRssi > -86)
					{
						pAd->SCSCtrl.AdjustSensitivity = -92;
						//DBGPRINT(RT_DEBUG_ERROR, ("%s(): SCS=M\n", __FUNCTION__));
		}	
				}
				else //adjust
		{
					if (pAd->SCSCtrl.CurrSensitivity +2 <= pAd->SCSCtrl.RSSIBoundary )
					{
						pAd->SCSCtrl.AdjustSensitivity = pAd->SCSCtrl.CurrSensitivity +2;
					}
					else if (pAd->SCSCtrl.CurrSensitivity > pAd->SCSCtrl.RSSIBoundary )
						pAd->SCSCtrl.AdjustSensitivity = pAd->SCSCtrl.RSSIBoundary ;
				}
			}
			else if (Action == Increase_Range)
			{
				if (pAd->SCSCtrl.SCSStatus == SCS_STATUS_DEFAULT)
				{ /*First time inital */
					//No thing to do.
				}
				else //adjust
			{
					if (pAd->SCSCtrl.CurrSensitivity -2 >= (-100))
					{
						pAd->SCSCtrl.AdjustSensitivity = pAd->SCSCtrl.CurrSensitivity -2;
						if (pAd->SCSCtrl.AdjustSensitivity > pAd->SCSCtrl.RSSIBoundary )
							pAd->SCSCtrl.AdjustSensitivity = pAd->SCSCtrl.RSSIBoundary ;
					}						
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("CurrSen=%d, AdjustSen=%d \n", pAd->SCSCtrl.CurrSensitivity, pAd->SCSCtrl.AdjustSensitivity));
			if (pAd->SCSCtrl.AdjustSensitivity >-54)
				pAd->SCSCtrl.AdjustSensitivity = -54;

			if (pAd->SCSCtrl.CurrSensitivity != pAd->SCSCtrl.AdjustSensitivity)
			{
				//Apply to CR
				if (pAd->SCSCtrl.AdjustSensitivity >= -100 &&  pAd->SCSCtrl.AdjustSensitivity <= -84) //M
				{
					AdjustStep = (pAd->SCSCtrl.AdjustSensitivity+92 )/2;
					CrValue = 0x56F0076f;
					tmpCrValue = 0x7+AdjustStep;
					CrValue |= (tmpCrValue << 12);
					CrValue |= (tmpCrValue << 16);
					DBGPRINT(RT_DEBUG_TRACE, ("M AdjustSensi=%d AdjustStep=%d CrValue = %x \n", pAd->SCSCtrl.AdjustSensitivity, AdjustStep, CrValue));
					RTMP_IO_WRITE32(pAd, CR_AGC_0, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_3, 0x81D0D5E3);
					RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, 0x81D0D5E3);
					pAd->SCSCtrl.CurrSensitivity = pAd->SCSCtrl.AdjustSensitivity;
					pAd->SCSCtrl.SCSStatus = SCS_STATUS_MIDDLE;
				
				}
				else if (pAd->SCSCtrl.AdjustSensitivity > -84 &&  pAd->SCSCtrl.AdjustSensitivity <= -72) //L
				{
					AdjustStep = (pAd->SCSCtrl.AdjustSensitivity+80 )/2;
					CrValue = 0x6AF0006f;
					tmpCrValue = 0x7+AdjustStep;
					CrValue |= (tmpCrValue << 8);
					CrValue |= (tmpCrValue << 12);
					CrValue |= (tmpCrValue << 16);					
					DBGPRINT(RT_DEBUG_TRACE, ("L AdjustSensi=%d AdjustStep=%d CrValue = %x \n", pAd->SCSCtrl.AdjustSensitivity, AdjustStep, CrValue));
					RTMP_IO_WRITE32(pAd, CR_AGC_0, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_3, 0x8181D5E3);
					RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, 0x8181D5E3);	
					pAd->SCSCtrl.CurrSensitivity = pAd->SCSCtrl.AdjustSensitivity;
					pAd->SCSCtrl.SCSStatus = SCS_STATUS_LOW;
				
				}
				else if (pAd->SCSCtrl.AdjustSensitivity >=-70 &&  pAd->SCSCtrl.AdjustSensitivity <= -54) //UL
				{
					AdjustStep = (pAd->SCSCtrl.AdjustSensitivity+62 )/2;
					CrValue = 0x7FF0000f;
					tmpCrValue = 0x6+AdjustStep;
					CrValue |= (tmpCrValue << 4);
					CrValue |= (tmpCrValue << 8);
					CrValue |= (tmpCrValue << 12);
					CrValue |= (tmpCrValue << 16);					
					DBGPRINT(RT_DEBUG_TRACE, ("UL AdjustSensi=%d AdjustStep=%d CrValue = %x \n", pAd->SCSCtrl.AdjustSensitivity, AdjustStep, CrValue));
					RTMP_IO_WRITE32(pAd, CR_AGC_0, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, CrValue);
					RTMP_IO_WRITE32(pAd, CR_AGC_3, 0x818181E3);
					RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, 0x818181E3);
					pAd->SCSCtrl.CurrSensitivity = pAd->SCSCtrl.AdjustSensitivity;
					pAd->SCSCtrl.SCSStatus = SCS_STATUS_ULTRA_LOW;
				}
				else
				{

					pAd->SCSCtrl.SCSStatus = SCS_STATUS_DEFAULT;
				RTMP_IO_WRITE32(pAd, CR_AGC_0, pAd->SCSCtrl.CR_AGC_0_default);
				RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, pAd->SCSCtrl.CR_AGC_0_default);
				RTMP_IO_WRITE32(pAd, CR_AGC_3, pAd->SCSCtrl.CR_AGC_3_default);
				RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, pAd->SCSCtrl.CR_AGC_3_default);
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): SCS=??? (Default)\n", __FUNCTION__));
			}
		}				

		DBGPRINT(RT_DEBUG_TRACE, ("%s(): miniRSSI=%d, RSSIBoundary=%d Action=%d AdjustSensitivity=%d\n", __FUNCTION__,		
		pAd->SCSCtrl.SCSMinRssi, pAd->SCSCtrl.RSSIBoundary, Action, pAd->SCSCtrl.AdjustSensitivity));
	}
	else	
	{
		if (pAd->SCSCtrl.SCSStatus != SCS_STATUS_DEFAULT)
		{
			RTMP_IO_WRITE32(pAd, CR_AGC_0, pAd->SCSCtrl.CR_AGC_0_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, pAd->SCSCtrl.CR_AGC_0_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_3, pAd->SCSCtrl.CR_AGC_3_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, pAd->SCSCtrl.CR_AGC_3_default);
			pAd->SCSCtrl.SCSStatus = SCS_STATUS_DEFAULT;
			pAd->SCSCtrl.CurrSensitivity = -102;
			pAd->SCSCtrl.AdjustSensitivity= -102;
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): CSC=H (Default)\n", __FUNCTION__));			
		}
	}
	
	
	return TRUE;

}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

static struct phy_ops mt_phy_ops = {
	.bbp_init = MTBbpInit,
	.bbp_set_bw = mt_bbp_set_bw,
	.ShowPartialBBP = MTShowPartialBBP,
	.ShowAllBBP = MTShowAllBBP,
	.ShowPartialRF = MTShowPartialRF,
	.ShowAllRF = MTShowAllRF,
#ifdef SMART_CARRIER_SENSE_SUPPORT
	.Smart_Carrier_Sense = MTSmartCarrierSense,
#endif /* SMART_CARRIER_SENSE_SUPPORT */
};


INT mt_phy_probe(RTMP_ADAPTER *pAd)
{
	pAd->phy_op = &mt_phy_ops;

	return TRUE;
}
