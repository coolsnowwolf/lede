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
	greenap.c

	Abstract:
	Ralink Wireless driver green ap related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"


#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower)
{
#if defined (MT7603) || defined (MT7628)
	if (pAd->chipCap.hif_type == HIF_MT)  {
		if (pAd->Antenna.field.RxPath == 2)
			AsicSetRxStream(pAd, 1);
	}
#else	
	{
		bbp_set_mmps(pAd, ReduceCorePower);
		rtmp_mac_set_mmps(pAd, ReduceCorePower);
	}
#endif /* defined (MT7603) || defined (MT7628) */

	pAd->ApCfg.bGreenAPActive=TRUE;
	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPSNew, 30xx changes the # of antenna to 1\n"));
}


VOID DisableAPMIMOPSv2(RTMP_ADAPTER *pAd)
{
#if defined (MT7603) || defined (MT7628)
	if (pAd->chipCap.hif_type == HIF_MT)  {
		if (pAd->Antenna.field.RxPath == 2)
			AsicSetRxStream(pAd, pAd->Antenna.field.RxPath);
	}
#else	
	{
		bbp_set_mmps(pAd, FALSE);
		rtmp_mac_set_mmps(pAd, FALSE);
	}
#endif /* defined (MT7603) || defined (MT7628) */

	pAd->ApCfg.bGreenAPActive=FALSE;
	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPSNew, 30xx reserve only one antenna\n"));
}

#if defined (MT7603) || defined (MT7628)
#else
VOID EnableAPMIMOPSv1(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN ReduceCorePower)
{
	UCHAR BBPR3 = 0,BBPR1 = 0;
	ULONG TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/
	UCHAR BBPR4=0;
	UCHAR CentralChannel;



	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;
		
	TxPinCfg &= 0xFFFFFFF3;
	TxPinCfg &= 0xFFFFF3FF;
	pAd->ApCfg.bGreenAPActive=TRUE;

	CentralChannel = pAd->CommonCfg.CentralChannel;
	DBGPRINT(RT_DEBUG_INFO, ("Run with BW_20\n"));
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	CentralChannel = pAd->CommonCfg.Channel;

	/* Set BBP registers to BW20 */
	bbp_set_bw(pAd, BW_20);

	/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
	if (pAd->Antenna.field.RxPath>1||pAd->Antenna.field.TxPath>1)
	{
		/*Tx/Rx Stream*/
		bbp_set_txdac(pAd, 0);
		bbp_set_rxpath(pAd, 1);
		
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

	}
	AsicSwitchChannel(pAd, CentralChannel, FALSE);

	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n"));
}


VOID DisableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3=0,BBPR1=0;
	ULONG	TxPinCfg = 0x00050F0A; /* Gary 2007/08/09 0x050A0A */
	UCHAR	CentralChannel;
	UINT32	Value=0;



	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;
	/* Turn off unused PA or LNA when only 1T or 1R*/
	if (pAd->Antenna.field.TxPath == 1)
		TxPinCfg &= 0xFFFFFFF3;
	if (pAd->Antenna.field.RxPath == 1)
		TxPinCfg &= 0xFFFFF3FF;

	pAd->ApCfg.bGreenAPActive=FALSE;

	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) && (pAd->CommonCfg.Channel != 14))
	{
		UINT8 ext_ch = EXTCHA_NONE;

		DBGPRINT(RT_DEBUG_INFO, ("Run with BW_40\n"));
		/* Set CentralChannel to work for BW40 */
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
			ext_ch = EXTCHA_ABOVE;
		}
		else if ((pAd->CommonCfg.Channel > 2) && (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW))
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
			ext_ch = EXTCHA_BELOW;
		}
		CentralChannel = pAd->CommonCfg.CentralChannel;
		AsicSetChannel(pAd, CentralChannel, BW_40, ext_ch, FALSE);
	}

	/*Tx Stream*/
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && (pAd->Antenna.field.TxPath == 2))
		bbp_set_txdac(pAd, 2);
	else
		bbp_set_txdac(pAd, 0);

	/*Rx Stream*/
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);


	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPS, 305x/28xx reserve only one antenna\n"));
}
#endif /* defined (MT7603) || defined (MT7628) */
#endif /* GREENAP_SUPPORT */

