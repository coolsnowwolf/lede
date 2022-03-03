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
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC
	
	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


#ifdef CONFIG_STA_SUPPORT
VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pRateTable)
{
	UCHAR					i;
	HT_FBK_CFG0_STRUC		HtCfg0;
	HT_FBK_CFG1_STRUC		HtCfg1;
	LG_FBK_CFG0_STRUC		LgCfg0;
	LG_FBK_CFG1_STRUC		LgCfg1;
#ifdef DOT11N_SS3_SUPPORT
	TX_FBK_CFG_3S_0_STRUC	Ht3SSCfg0;
	TX_FBK_CFG_3S_1_STRUC	Ht3SSCfg1;
#endif /* DOT11N_SS3_SUPPORT */
	RTMP_RA_LEGACY_TB *pCurrTxRate, *pNextTxRate;

#ifdef AGS_SUPPORT
	RTMP_RA_AGS_TB *pCurrTxRate_AGS, *pNextTxRate_AGS;	
	BOOLEAN					bUseAGS = FALSE;

	if (AGS_IS_USING(pAd, pRateTable))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Use AGS\n", __FUNCTION__));
		
		bUseAGS = TRUE;

		Ht3SSCfg0.word = 0x1211100f;
		Ht3SSCfg1.word = 0x16151413;	
	}
#endif /* AGS_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT3883(pAd))
	{
		Ht3SSCfg0.word = 0x12111008;
		Ht3SSCfg1.word = 0x16151413;
	}
#endif /* DOT11N_SS3_SUPPORT */

	/* set to initial value*/
	HtCfg0.word = 0x65432100;
	HtCfg1.word = 0xedcba980;
	LgCfg0.word = 0xedcba988;
	LgCfg1.word = 0x00002100;


#ifdef MT76x2
	if (IS_MT76x2(pAd))
		LgCfg1.word = 0x87872100;
#endif

#ifdef NEW_RATE_ADAPT_SUPPORT
	/* Use standard fallback if using new rate table */
	if (ADAPT_RATE_TABLE(pRateTable))
		goto skipUpdate;
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	if (bUseAGS)
	{
		pNextTxRate_AGS = (RTMP_RA_AGS_TB *)pRateTable+1;
		pNextTxRate = (RTMP_RA_LEGACY_TB *)pNextTxRate_AGS;
	}
	else
#endif /* AGS_SUPPORT */
		pNextTxRate = (RTMP_RA_LEGACY_TB *)pRateTable+1;

	for (i = 1; i < *((PUCHAR) pRateTable); i++)
	{
#ifdef AGS_SUPPORT
		if (bUseAGS)
		{
			pCurrTxRate_AGS = (RTMP_RA_AGS_TB *)pRateTable+1+i;
			pCurrTxRate = (RTMP_RA_LEGACY_TB *)pCurrTxRate_AGS;
		}
		else
#endif /* AGS_SUPPORT */
			pCurrTxRate = (RTMP_RA_LEGACY_TB *)pRateTable+1+i;

		switch (pCurrTxRate->Mode)
		{
			case 0:		/* CCK */
				break;
			case 1:		/* OFDM */
				{
					switch(pCurrTxRate->CurrMCS)
					{
						case 0:
							LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 1:
							LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 2:
							LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 3:
							LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 4:
							LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 5:
							LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 6:
							LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 7:
							LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
					}
				}
				break;
#ifdef DOT11_N_SUPPORT
			case 2:		/* HT-MIX */
			case 3:		/* HT-GF */
				{
					if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
					{
						if (pCurrTxRate->CurrMCS <= 15)
						{
							switch(pCurrTxRate->CurrMCS)
							{
								case 0:
									HtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
									break;
								case 1:
									HtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
									break;
								case 2:
									HtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
									break;
								case 3:
									HtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
									break;
								case 4:
									HtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
									break;
								case 5:
									HtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
									break;
								case 6:
									HtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
									break;
								case 7:
									HtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
									break;
								case 8:
									HtCfg1.field.HTMCS8FBK = 0;//pNextTxRate->CurrMCS;
									break;
								case 9:
									HtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
									break;
								case 10:
									HtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
									break;
								case 11:
									HtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
									break;
								case 12:
									HtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
									break;
								case 13:
									HtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
									break;
								case 14:
									HtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
									break;
								case 15:
									HtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
									break;
							}
						}
						else 
#ifdef AGS_SUPPORT
						if ((bUseAGS == TRUE) && 
							(pCurrTxRate->CurrMCS >= 16) && (pCurrTxRate->CurrMCS <= 23))
						{
							switch(pCurrTxRate->CurrMCS)
							{
								case 16:
									Ht3SSCfg0.field.HTMCS16FBK = pNextTxRate->CurrMCS;
									break;
								case 17:
									Ht3SSCfg0.field.HTMCS17FBK = pNextTxRate->CurrMCS;
									break;
								case 18:
									Ht3SSCfg0.field.HTMCS18FBK = pNextTxRate->CurrMCS;
									break;
								case 19:
									Ht3SSCfg0.field.HTMCS19FBK = pNextTxRate->CurrMCS;
									break;
								case 20:
									Ht3SSCfg1.field.HTMCS20FBK = pNextTxRate->CurrMCS;
									break;
								case 21:
									Ht3SSCfg1.field.HTMCS21FBK = pNextTxRate->CurrMCS;
									break;
								case 22:
									Ht3SSCfg1.field.HTMCS22FBK = pNextTxRate->CurrMCS;
									break;
								case 23:
									Ht3SSCfg1.field.HTMCS23FBK = pNextTxRate->CurrMCS;
									break;
							}
						}
						else
#endif /* AGS_SUPPORT */
							DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support CurrMCS=%d\n", pCurrTxRate->CurrMCS));
					}
				}
				break;
#endif /* DOT11_N_SUPPORT */
		}

		pNextTxRate = pCurrTxRate;
	}

#ifdef AGS_SUPPORT
	if (bUseAGS == TRUE)
	{
		Ht3SSCfg0.field.HTMCS16FBK = 0x8; // MCS 16 -> MCS 8
		HtCfg1.field.HTMCS8FBK = 0x0; // MCS 8 -> MCS 0

		LgCfg0.field.OFDMMCS2FBK = 0x3; // OFDM 12 -> CCK 11
		LgCfg0.field.OFDMMCS1FBK = 0x2; // OFDM 9 -> CCK 5.5
		LgCfg0.field.OFDMMCS0FBK = 0x2; // OFDM 6 -> CCK 5.5
	}
#endif /* AGS_SUPPORT */

#ifdef NEW_RATE_ADAPT_SUPPORT
skipUpdate:
#endif /* NEW_RATE_ADAPT_SUPPORT */

	RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, HtCfg0.word);
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, HtCfg1.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG0, LgCfg0.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG1, LgCfg1.word);

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd)
#ifdef AGS_SUPPORT
		|| (bUseAGS == TRUE)
#endif /* AGS_SUPPORT */ 
	)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, Ht3SSCfg0.word);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, Ht3SSCfg1.word);
		DBGPRINT(RT_DEBUG_TRACE, ("AsicUpdateAutoFallBackTable: Ht3SSCfg0=0x%x, Ht3SSCfg1=0x%x\n", Ht3SSCfg0.word, Ht3SSCfg1.word));
	}
#endif /* DOT11N_SS3_SUPPORT */

}
#endif /* CONFIG_STA_SUPPORT */


INT AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};
	
	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.TxautoFBEnable = ((enable == TRUE) ? 1 : 0);
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);	

	return TRUE;
}


INT AsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#ifdef RANGE_EXTEND
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif // RANGE_EXTEND //
#ifdef DOT11N_SS3_SUPPORT
	if (pAd->CommonCfg.TxStream >= 3)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, 0x12111008);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, 0x16151413);
	}
#endif /* DOT11N_SS3_SUPPORT */

	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.
		
		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
typedef enum _PROT_REG_IDX_{
	REG_IDX_CCK = 0,	/* 0x1364 */
	REG_IDX_OFDM = 1,	/* 0x1368 */
	REG_IDX_MM20 = 2,  /* 0x136c */
	REG_IDX_MM40 = 3, /* 0x1370 */
	REG_IDX_GF20 = 4, /* 0x1374 */
	REG_IDX_GF40 = 5, /* 0x1378 */
}PROT_REG_IDX;


VOID AsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist)	
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6], PhyMode = 0x4000;
	USHORT offset;
	UCHAR i;
	UINT32 MacReg = 0;
#ifdef DOT11_VHT_AC
#ifdef RT65xx
	PROT_CFG_STRUC vht_port_cfg = {.word = 0};
	UINT16 protect_rate = 0;
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
#ifdef APCLI_CERT_SUPPORT
#ifdef DOT11_VHT_AC
	BOOLEAN bStaConnect = FALSE;
#endif /* DOT11_VHT_AC */
#endif /* APCLI_CERT_SUPPORT */
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
		return;

	if (pAd->BATable.numDoneOriginator)
	{
		/* enable the RTS/CTS to avoid channel collision*/
		SetMask |= ALLN_SETPROTECT;
		OperationMode = 8;
	}
#endif /* DOT11_N_SUPPORT */

	/* Config ASIC RTS threshold register*/
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	/* If the user want disable RtsThreshold and enbale Amsdu/Ralink-Aggregation, set the RtsThreshold as 4096*/
        if ((
#ifdef DOT11_N_SUPPORT
			(pAd->CommonCfg.BACapability.field.AmsduEnable) || 
#endif /* DOT11_N_SUPPORT */
			(pAd->CommonCfg.bAggregationCapable == TRUE))
            && pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD)
        {
			MacReg |= (0x1000 << 8);
        }
        else
        {
			MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
        }

	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	/* Initial common protection settings*/
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
	// TODO: shiang, is that a correct way to set 0x2000 here??
	if (IS_RT65XX(pAd))
		PhyMode = 0x2000; /* Bit 15:13, 0:Legacy CCK, 1: Legacy OFDM, 2: HT mix mode, 3: HT green field, 4: VHT mode, 5-7: Reserved */
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */

	/* update PHY mode and rate*/
	if (pAd->OpMode == OPMODE_AP)
	{
		/* update PHY mode and rate*/
		if (pAd->CommonCfg.Channel > 14)
			ProtCfg.field.ProtectRate = PhyMode;
		ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		// Decide Protect Rate for Legacy packet
		if (pAd->CommonCfg.Channel > 14)
		{
			ProtCfg.field.ProtectRate = PhyMode; // OFDM 6Mbps
		}
		else 
		{
			ProtCfg.field.ProtectRate = 0x0000; // CCK 1Mbps
			if (pAd->CommonCfg.MinTxRate > RATE_11)
				ProtCfg.field.ProtectRate |= PhyMode; // OFDM 6Mbps
		}
	}

#ifdef DOT11_VHT_AC
#ifdef RT65xx
	if (IS_RT65XX(pAd))
		protect_rate = ProtCfg.field.ProtectRate;
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_FPGA_MODE
//+++Add by shiang for debug
	if (pAd->fpga_ctl.fpga_on & 0x8)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():RtsRate=%d\n",
					__FUNCTION__, pAd->CommonCfg.RtsRate));
	}
//---Add by shiang for debug
#endif /* CONFIG_FPGA_MODE */

	/* Handle legacy(B/G) protection*/
	if (bDisableBGProtect)
	{
		/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
		ProtCfg.field.ProtectCtrl = 0;
		Protect[REG_IDX_CCK] = ProtCfg.word;
		Protect[REG_IDX_OFDM] = ProtCfg.word;
		pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14) {
			/*ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;*/
			ProtCfg.field.ProtectCtrl = 0;			/* CCK do not need to be protected*/
			Protect[REG_IDX_CCK] = ProtCfg.word;
			ProtCfg.field.ProtectCtrl = ASIC_CTS;	/* OFDM needs using CCK to protect*/
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 1; /* CTS-self is used */
		} else {
			ProtCfg.field.ProtectCtrl = 0;
			Protect[REG_IDX_CCK] = ProtCfg.word;
			Protect[REG_IDX_OFDM] = ProtCfg.word;
			pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
		}
	}

#ifdef DOT11_N_SUPPORT
	/* Decide HT frame protection.*/
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				/* NO PROTECT */
				/* 1.All STAs in the BSS are 20/40 MHz HT*/
				/* 2. in ai 20/40MHz BSS*/
				/* 3. all STAs are 20MHz in a 20MHz BSS*/
				/* Pure HT. no protection.*/

				/* MM20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[2] = 0x01744004;

				/* MM40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None) */
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[3] = 0x03f44084;

				/* CF20_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 010111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)*/
				Protect[4] = 0x01744004;

				/* CF40_PROT_CFG*/
				/*	Reserved (31:27)*/
				/* 	PROT_TXOP(25:20) -- 111111*/
				/*	PROT_NAV(19:18)  -- 01 (Short NAV protection)*/
				/*  PROT_CTRL(17:16) -- 00 (None)*/
				/* 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)*/
				Protect[5] = 0x03f44084;

				if (bNonGFExist)
				{
					/* PROT_NAV(19:18)  -- 01 (Short NAV protectiion)*/
					/* PROT_CTRL(17:16) -- 01 (RTS/CTS)*/
					Protect[REG_IDX_GF20] = 0x01754004;
					Protect[REG_IDX_GF40] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
 			case 1:
				/* This is "HT non-member protection mode."*/
				/* If there may be non-HT STAs my BSS*/
				ProtCfg.word = 0x01744004;	/* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					PROT_CFG_STRUC vht_port_cfg;

					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */

				break;
				
			case 2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets*/
				ProtCfg.word = 0x01744004;  /* PROT_CTRL(17:16) : 0 (None)*/
				ProtCfg4.word = 0x03f44084; /* duplicaet legacy 24M. BW set 1.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083; */
				} 
				/*Assign Protection method for 40MHz packets*/
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = 0;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
			case 3:
				/* HT mixed mode. PROTECT ALL!*/
				/* Assign Rate*/
				ProtCfg.word = 0x01744004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f44084;
				/* both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f40003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				/*Assign Protection method for 20&40 MHz packets*/
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[REG_IDX_MM20] = ProtCfg.word;
				Protect[REG_IDX_MM40] = ProtCfg4.word;
				Protect[REG_IDX_GF20] = ProtCfg.word;
				Protect[REG_IDX_GF40] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;

#ifdef DOT11_VHT_AC
#ifdef RT65xx
				// TODO: shiang-6590, fix me for this protection mechanism
				if (IS_RT65XX(pAd))
				{
					// Temporary turn on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
					RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);  

					RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
					vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
					vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
					vht_port_cfg.field.ProtectRate = protect_rate;
					RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);  
				}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
				break;
				
			case 8:
				/* Special on for Atheros problem n chip.*/
				ProtCfg.word = 0x01754004;	/*duplicaet legacy 24M. BW set 1.*/
				ProtCfg4.word = 0x03f54084;
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	/*ERP use Protection bit is set, use protection rate at Clause 18..*/
					ProtCfg4.word = 0x03f50003; /* Don't duplicate RTS/CTS in CCK mode. 0x03f40083*/
				}
				
#ifdef APCLI_CERT_SUPPORT // for TGAC 5.2.35
#ifdef DOT11_VHT_AC
				if (pAd->MacTab.Size > 0) {
					MAC_TABLE_ENTRY *pEntry = NULL;
					
					for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
						pEntry = &pAd->MacTab.Content[i];
						if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
						{
							bStaConnect = TRUE;
						}
					}
				}
#endif /* DOT11_VHT_AC */				
#endif /* APCLI_CERT_SUPPORT */


#ifdef DOT11_VHT_AC
#ifdef RT65xx
                               if (IS_RT65XX(pAd)
#ifdef APCLI_CERT_SUPPORT							   	
					&&(bStaConnect)   	
#endif					
							   	)
                               {
                                       // Temporary tuen on RTS in VHT, MAC: TX_PROT_CFG6, TX_PROT_CFG7, TX_PROT_CFG8
                                       PROT_CFG_STRUC vht_port_cfg;

                                       RTMP_IO_READ32(pAd, TX_PROT_CFG6, &vht_port_cfg.word);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       RTMP_IO_WRITE32(pAd, TX_PROT_CFG6, vht_port_cfg.word);

                                       RTMP_IO_READ32(pAd, TX_PROT_CFG7, &vht_port_cfg.word);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       RTMP_IO_WRITE32(pAd, TX_PROT_CFG7, vht_port_cfg.word);

                                       RTMP_IO_READ32(pAd, TX_PROT_CFG8, &vht_port_cfg.word);
                                       vht_port_cfg.field.ProtectCtrl = ASIC_RTS;
                                       vht_port_cfg.field.ProtectNav = ASIC_SHORTNAV;
                                       vht_port_cfg.field.ProtectRate = protect_rate;
                                       RTMP_IO_WRITE32(pAd, TX_PROT_CFG8, vht_port_cfg.word);
                               }
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */

				Protect[REG_IDX_MM20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_MM40] = ProtCfg4.word; /*0x03f54084;*/
				Protect[REG_IDX_GF20] = ProtCfg.word; 	/*0x01754004;*/
				Protect[REG_IDX_GF40] = ProtCfg4.word; /*0x03f54084;*/
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;		
		}
	}
#endif /* DOT11_N_SUPPORT */
	
	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
	{
		if ((SetMask & (1<< i)))
		{
#ifdef RT65xx
			if (IS_RT65XX(pAd)) {
				if ((Protect[i] & 0x4000) == 0x4000)
					Protect[i] = ((Protect[i] & (~0x4000)) | 0x2000);
			}
#endif /* RT65xx */
		}
		RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);
	}

#ifdef DOT11_VHT_AC
#ifdef RT65xx
	if (IS_RT65XX(pAd))
	{
		UINT32 cfg_reg;
		for (cfg_reg = TX_PROT_CFG6; cfg_reg <= TX_PROT_CFG8; cfg_reg += 4)
		{
			RTMP_IO_READ32(pAd, cfg_reg, &MacReg);
			MacReg &= (~0x18000000);
			if (pAd->CommonCfg.vht_bw_signal)
			{
				if (pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_STATIC) /* static */
					MacReg |= 0x08000000;
				else if (pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC)/* dynamic */
					MacReg |= 0x18000000;
			}
			RTMP_IO_WRITE32(pAd, cfg_reg, MacReg);
		}
	}
#endif /* RT65xx */
#endif /* DOT11_VHT_AC */
}


VOID AsicBBPAdjust(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-6590, now this function only used for AP mode, why we need this differentation?
	if (pAd->chipOps.ChipBBPAdjust != NULL)
		pAd->chipOps.ChipBBPAdjust(pAd);
}

	
/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan)
{
	UCHAR bw;
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return; 

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

			if( (RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == 1)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): autopm_resume success\n", __FUNCTION__));
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
			}
			else if ((RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == (-1))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): autopm_resume fail\n", __FUNCTION__));
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
				return;
			}
			else
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): autopm_resume do nothing\n", __FUNCTION__));

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef CUSTOMER_DCC_FEATURE
	/* later need to move this BssTableInit to other place */
	BssTableInit(&pAd->AvailableBSS);
#endif
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->chipOps.ChipSwitchChannel)
		pAd->chipOps.ChipSwitchChannel(pAd, Channel, bScan);
	else
		DBGPRINT(RT_DEBUG_ERROR, ("For this chip, no specified channel switch function!\n"));

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	if (bScan)
		bw = BW_20;
	else {
		bw = pAd->CommonCfg.BBPCurrentBW;
	}
	RTMPSetAGCInitValue(pAd, bw);
	
#ifdef TXBF_SUPPORT
	rtmp_asic_set_bf(pAd); // FW will initialize TxBf HW status. Re-calling this AP could recover previous status
	
#ifdef MT76x2
	if (IS_MT76x2(pAd))
	{
		UINT32 value32;
		// Disable BF HW to apply profile to packets when nSS == 2.
		// Maybe it can be initialized at chip init but removing the same CR initialization from FW will be better
		RTMP_IO_READ32(pAd, TXO_R4, &value32);
		value32 |= 0x2000000;
		RTMP_IO_WRITE32(pAd, TXO_R4, value32);
	}
#endif /* MT76x2 */
#endif /* TXBF_SUPPORT */	

#ifdef SMART_MESH_MONITOR
	if (!bScan)
	{
		struct nsmpif_drvevnt_buf drvevnt;
		drvevnt.data.channel_change.type = NSMPIF_DRVEVNT_CHANNEL_CHANGE;
		drvevnt.data.channel_change.channel = pAd->CommonCfg.Channel;
		NdisZeroMemory(drvevnt.data.channel_change.op_channels,sizeof(drvevnt.data.channel_change.op_channels));
		drvevnt.data.channel_change.op_channels[0] = pAd->CommonCfg.Channel;
#ifdef DOT11_N_SUPPORT		
		if(bw == BW_40)
			drvevnt.data.channel_change.op_channels[1] = N_GetSecondaryChannel(pAd);
#endif /* DOT11_N_SUPPORT */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_CHANNEL_CHANGE,
								NULL, (PUCHAR)&drvevnt.data.channel_change, sizeof(drvevnt.data.channel_change));
	}
#endif /* SMART_MESH_MONITOR */
}


/*
	==========================================================================
	Description:
		This function is required for 2421 only, and should not be used during
		site survey. It's only required after NIC decided to stay at a channel
		for a longer period.
		When this function is called, it's always after AsicSwitchChannel().

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel) 
{
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */


VOID AsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
	BBP_CSR_CFG_STRUC	BbpCsr;

	/* Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	*/
	/* IF chipOps.AsicResetBbpAgent == NULL, run "else" part */
	if (pAd->chipOps.AsicResetBbpAgent != NULL)
		pAd->chipOps.AsicResetBbpAgent(pAd);
	else if (pAd->chipCap.MCUType != ANDES)
	{
		DBGPRINT(RT_DEBUG_INFO, ("Reset BBP Agent busy bit.!! \n"));
		RTMP_IO_READ32(pAd, H2M_BBP_AGENT, &BbpCsr.word);
		BbpCsr.field.Busy = 0;
		RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
	}
}


#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup 
		automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
		the wakeup timer timeout. Driver has to issue a separate command to wake
		PHY up.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
	RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever manual wakeup is required
		AsicForceSleep() should only be used when not in INFRA BSS. When
		in INFRA BSS, we should use AsicSleepThenAutoWakeup() instead.
	==========================================================================
 */
VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd)
{

}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
		expired.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN    bFromTx)
{
    DBGPRINT(RT_DEBUG_INFO, ("--> AsicForceWakeup \n"));
    RTMP_STA_FORCE_WAKEUP(pAd, bFromTx);	
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
 /* CFG_TODO */
VOID AsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid)
{
	UINT32 Addr4;
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	UINT32 regValue;
#endif /* defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) */

	DBGPRINT(RT_DEBUG_TRACE, ("===> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
				PRINT_MAC(pBssid)));
	
	Addr4 = (UINT32)(pBssid[0]) | 
			(UINT32)(pBssid[1] << 8)  | 
			(UINT32)(pBssid[2] << 16) |
			(UINT32)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW0, Addr4);
#endif /* HDR_TRANS_SUPPORT */

	Addr4 = 0;
	/* always one BSSID in STA mode*/
	Addr4 = (UINT32)(pBssid[4]) | (ULONG)(pBssid[5] << 8);


	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

#ifdef HDR_TRANS_SUPPORT
	Addr4 |= 0x18000000;
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, Addr4);
#endif /* HDR_TRANS_SUPPORT */

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
	{
		PUCHAR pP2PBssid = &pAd->CurrentAddress[0];

		Addr4 = (UINT32)(pP2PBssid[0]) | 
				(UINT32)(pP2PBssid[1] << 8)  | 
				(UINT32)(pP2PBssid[2] << 16) |
				(UINT32)(pP2PBssid[3] << 24);
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

#ifdef HDR_TRANS_SUPPORT
		RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW0, Addr4);
#endif /* HDR_TRANS_SUPPORT */

		Addr4 = 0;

		/* always one BSSID in STA mode */
		Addr4 = (UINT32)(pP2PBssid[4]) | (ULONG)(pP2PBssid[5] << 8);

		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

		RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
		regValue &= 0x0000FFFF;
		regValue |= (1 << 16);		

		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE0)
		{
			if ((pAd->CurrentAddress[5] % 2 != 0)
			)
			DBGPRINT(RT_DEBUG_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));
		
		}
		else
		{
			/*set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014) 
			to disable/enable the new MAC address assignment.  */
		    regValue |= (1 << 21);
		}
		
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);
#ifdef HDR_TRANS_SUPPORT
		/*
			point WCID MAC table to 0x1800
			This is for debug.
			But HDR_TRANS doesn't work if you remove it.
			Check after IC formal release.
		*/
		regValue |= 0x18000000;
		RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, regValue);
#endif /* HDR_TRANS_SUPPORT */
	}
#endif /* P2P_SUPPORT */
}


INT AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;

	csr2.field.Byte0 = addr[0];
	csr2.field.Byte1 = addr[1];
	csr2.field.Byte2 = addr[2];
	csr2.field.Byte3 = addr[3];
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);

	csr3.word = 0;
	csr3.field.Byte4 = addr[4];
	{
		csr3.field.Byte5 = addr[5];
		csr3.field.U2MeMask = 0xff;
	}
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("SetDevMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(addr)));

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW0, csr2.word);
	csr3.word &= 0xff00ffff;
	csr3.word |= 0x00410000;				// HW test code
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW1, csr3.word);
#endif /* HDR_TRANS_SUPPORT */

	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns)
{
	UCHAR NumOfMacs;
	UINT32 regValue;

	RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
	regValue &= 0x0000FFFF;

	/* 
		Note:
			1.The MAC address of Mesh and AP-Client link are different from Main BSSID.
			2.If the Mesh link is included, its MAC address shall follow the last MBSSID's MAC by increasing 1.
			3.If the AP-Client link is included, its MAC address shall follow the Mesh interface MAC by increasing 1.
	*/
	NumOfMacs = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAX_APCLI_NUM;


	/* set Multiple BSSID mode */
	if (NumOfMacs <= 1)
	{
		pAd->ApCfg.MacMask = ~(1-1);
		/*regValue |= 0x0; */
	}	
	else if (NumOfMacs <= 2)
	{
#ifndef NEW_MBSSID_MODE
		if ((pAd->CurrentAddress[5] % 2 != 0)
		)
			DBGPRINT(RT_DEBUG_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));
#endif
		regValue |= (1<<16);
		pAd->ApCfg.MacMask = ~(2-1);
	}
	else if (NumOfMacs <= 4)
	{
#ifndef NEW_MBSSID_MODE
		if (pAd->CurrentAddress[5] % 4 != 0)
			DBGPRINT(RT_DEBUG_ERROR, ("The 4-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 4\n"));
#endif
		regValue |= (2<<16);
		pAd->ApCfg.MacMask = ~(4-1);
	}
	else if (NumOfMacs <= 8)
	{
#ifndef NEW_MBSSID_MODE
		if (pAd->CurrentAddress[5] % 8 != 0)
			DBGPRINT(RT_DEBUG_ERROR, ("The 8-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 8\n"));
#endif
		regValue |= (3<<16);
		pAd->ApCfg.MacMask = ~(8-1);
	}
	else if (NumOfMacs <= 16)
	{
		/* Set MULTI_BSSID_MODE_BIT4 in MAC register 0x1014 */
		regValue |= (1<<22);
		pAd->ApCfg.MacMask = ~(16-1);
	}

	/* set Multiple BSSID Beacon number */
	if (NumOfBcns > 1)
	{
		if (NumOfBcns > 8)
			regValue |= (((NumOfBcns - 1) >> 3) << 23);
		regValue |= (((NumOfBcns - 1) & 0x7)  << 18);	
	}
	
	/* 	set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014) 
		to disable/enable the new MAC address assignment.  */
	if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
	{
		regValue |= (1 << 21);
#ifdef ENHANCE_NEW_MBSSID_MODE
		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE2)
			regValue |=  (1 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE3)
			regValue |=  (2 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE4)
			regValue |=  (3 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE5)
			regValue |=  (4 << 24);
		else if (pAd->chipCap.MBSSIDMode == MBSSID_MODE6)
			regValue |=  (5 << 24);
#endif /* ENHANCE_NEW_MBSSID_MODE */
	}

	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);

#ifdef HDR_TRANS_SUPPORT
	/*
		point WCID MAC table to 0x1800
		This is for debug.
		But HDR_TRANS doesn't work if you remove it.
		Check after IC formal release.
	*/
	regValue |= 0x18000000;
	RTMP_IO_WRITE32(pAd, HT_MAC_BSSID_DW1, regValue);
#endif /* HDR_TRANS_SUPPORT */
}
#endif /* CONFIG_AP_SUPPORT */


INT AsicSetRxFilter(RTMP_ADAPTER *pAd)
{
	UINT32 rx_filter_flag = APNORMAL;
		
	/* enable RX of MAC block*/
	if ((pAd->OpMode == OPMODE_AP)
	)
	{
		rx_filter_flag = APNORMAL;

#ifdef CONFIG_AP_SUPPORT
#ifdef IDS_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.IdsEnable)
				rx_filter_flag &= (~0x4);	/* Don't drop those not-U2M frames*/
		}
#endif /* IDS_SUPPORT */			
#endif /* CONFIG_AP_SUPPORT */
	}
#ifdef CONFIG_STA_SUPPORT
	else
	{		
#ifdef XLINK_SUPPORT
		if (pAd->StaCfg.PSPXlink)
			rx_filter_flag = PSPXLINK;
		else
#endif /* XLINK_SUPPORT */
			rx_filter_flag = STANORMAL;     /* Staion not drop control frame will fail WiFi Certification.*/
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_SNIFFER_SUPPORT
	/* Enable Rx with promiscuous reception */
	if ((MONITOR_ON(pAd)) && 
	(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_FULL || pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_FULL_NO_CLONE)) 
			rx_filter_flag = 0x3;
#endif
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);

	return TRUE;
}


#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	TX_LINK_CFG_STRUC TxLinkCfg;
	UINT32 Data = 0;
	
	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn =  (bEnable ? 1 : 0);
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	Data &= 0xFFFFFF00;
	if (bEnable) {
		Data |= 0x80;
	} else {
		/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
			&& (pAd->CommonCfg.bEnableTxBurst == TRUE)
#ifdef DOT11_N_SUPPORT
			&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif /* DOT11_N_SUPPORT */
		)
			Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);


	if (bEnable)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
	else
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);

	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter
        
    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None
        
    ========================================================================
*/
VOID RTMPSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
	TX_LINK_CFG_STRUC  TxLinkCfg;
    
	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);

	TxLinkCfg.field.TxCFAckEn = bPiggyBack;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

VOID AsicCtrlBcnMask(PRTMP_ADAPTER pAd, INT mask)
{
	BCN_BYPASS_MASK_STRUC bms;

	RTMP_IO_READ32(pAd, TX_BCN_BYPASS_MASK, &bms.word);
	bms.field.BeaconDropMask = mask;
	RTMP_IO_WRITE32(pAd, TX_BCN_BYPASS_MASK, bms.word);
}

static INT AsicSetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
	UINT32 val = 0, mask = 0, time_mask = 0;

	if (type == INT_TIMER_EN_PRE_TBTT) {
		mask = 0x1;
		timeout = (timeout & 0xffff);
		time_mask = 0xffff;
	}
	else if (type == INT_TIMER_EN_GP_TIMER) {
		mask = 0x2;
		timeout = (timeout & 0xffff) << 16;
		time_mask = (0xffff << 16);
	}
	else
		mask = 0x3;
	
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &val);
	if (enable == FALSE)
		val &= (~mask);
	else
		val |= mask;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, val);

	if (enable) {
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &val);
		val = (val & (~time_mask)) | timeout;
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, val);
	}

	return TRUE;
}


INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 timeout = 0;

	if (enable == TRUE)
		timeout = 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */

	return AsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_PRE_TBTT, timeout);
}


INT AsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return AsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--->Disable TSF synchronization\n"));

	pAd->TbttTickCount = 0;
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);

}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableBssSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;
#ifdef APCLI_SUPPORT 
                UCHAR apidx;
                BOOLEAN bMaskBcn;     
#endif /* APCLI_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

#ifdef APCLI_SUPPORT 
                // for apcli DFS, if ra0 not up, don,t send bcn
                bMaskBcn = TRUE;
                
                for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
                {
                                if(BeaconTransmitRequired(pAd, apidx, &pAd->ApCfg.MBSSID[apidx]))
                                {
                                                bMaskBcn = FALSE;
                                                break;
                                }
                }

                if (bMaskBcn &&  APCLI_IF_UP_CHECK(pAd, 0)) {
							DBGPRINT(RT_DEBUG_OFF, ("Apcli DFS need mask beacon!!!\n"));					
                                                AsicCtrlBcnMask(pAd, 0xFF);
                }
                else {
                                AsicCtrlBcnMask(pAd, 0x0);
                }
#endif /* APCLI_SUPPORT */

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
/*	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, 0x00000000);*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
		csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 1; /* sync TSF in INFRASTRUCTURE mode*/
		csr.field.bBeaconGen  = 0; /* do NOT generate BEACON*/
		csr.field.bTBTTEnable = 1;
	}
#endif /* CONFIG_STA_SUPPORT */	
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}

/*CFG_TODO*/
VOID AsicEnableApBssSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

	csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr.field.bTsfTicking = 1;
	csr.field.TsfSyncMode = 3; /* sync TSF similar as in ADHOC mode?*/
	csr.field.bBeaconGen  = 1; /* AP should generate BEACON*/
	csr.field.bTBTTEnable = 1;

	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}


#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
	Note: 
		BEACON frame in shared memory should be built ok before this routine
		can be called. Otherwise, a garbage frame maybe transmitted out every
		Beacon period.

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableIbssSync(RTMP_ADAPTER *pAd)
{
	BCN_TIME_CFG_STRUC csr9;
	UCHAR *ptr;
	UINT i;
	ULONG beaconBaseLocation = 0;
	USHORT beaconLen = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT32 longptr;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
			beaconLen = pAd->BeaconTxWI.TXWI_N.MPDUtotalByteCnt;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		beaconLen = pAd->BeaconTxWI.TXWI_O.MPDUtotalByteCnt;
#endif /* RTMP_MAC */

#ifdef RT_BIG_ENDIAN
	{
		TXWI_STRUC localTxWI;
	
		NdisMoveMemory((PUCHAR)&localTxWI, (PUCHAR)&pAd->BeaconTxWI, TXWISize);
		RTMPWIEndianChange(pAd, (PUCHAR)&localTxWI, TYPE_TXWI);
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
				beaconLen = localTxWI.TXWI_N.MPDUtotalByteCnt;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			beaconLen = localTxWI.TXWI_O.MPDUtotalByteCnt;
#endif /* RTMP_MAC */
	}
#endif /* RT_BIG_ENDIAN */

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode, beaconLen=%d)\n",
				beaconLen));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
	beaconBaseLocation = HW_BEACON_BASE0(pAd);

#ifdef RTMP_MAC_PCI
	/* move BEACON TXD and frame content to on-chip memory*/
	ptr = (PUCHAR)&pAd->BeaconTxWI;
	for (i=0; i < TXWISize; i+=4)
	{
		longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, HW_BEACON_BASE0(pAd) + i, longptr, 4);
		ptr += 4;
	}

	/* start right after the 16-byte TXWI field*/
	ptr = pAd->BeaconBuf;
	for (i=0; i< beaconLen; i+=4)
	{
		longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, HW_BEACON_BASE0(pAd) + TXWISize + i, longptr, 4);
		ptr +=4;
	}
#endif /* RTMP_MAC_PCI */

	
	/*
		For Wi-Fi faily generated beacons between participating stations.
		Set TBTT phase adaptive adjustment step to 8us (default 16us)
	*/
	/* don't change settings 2006-5- by Jerry*/
	/*RTMP_IO_WRITE32(pAd, TBTT_SYNC_CFG, 0x00001010);*/
	
	/* start sending BEACON*/
	csr9.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU*/
	csr9.field.bTsfTicking = 1;
#ifdef IWSC_SUPPORT
	/*
		 SYNC with nobody
		 If Canon loses our Beacon over 5 seconds, Canon will delete us silently.
	*/
	csr9.field.TsfSyncMode = 3; // sync TSF in IBSS mode
#else /* IWSC_SUPPORT */
	/*
		(STA ad-hoc mode) Upon the reception of BEACON frame from associated BSS, 
		local TSF is updated with remote TSF only if the remote TSF is greater than local TSF
	*/
	csr9.field.TsfSyncMode = 2; /* sync TSF in IBSS mode*/
#endif /* !IWSC_SUPPORT */
	csr9.field.bTBTTEnable = 1;
	csr9.field.bBeaconGen = 1;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm)
{
	EDCA_AC_CFG_STRUC Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC AifsnCsr;
	CWMIN_CSR_STRUC CwminCsr;
	CWMAX_CSR_STRUC CwmaxCsr;
	int i;

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicSetEdcaParm\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) || IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		/*========================================================*/
		/*      MAC Register has a copy .*/
		/*========================================================*/
		if( pAd->CommonCfg.bEnableTxBurst )		
		{
			/* For CWC test, change txop from 0x30 to 0x20 in TxBurst mode*/
			Ac0Cfg.field.AcTxop = 0x20; /* Suggest by John for TxBurst in HT Mode*/
		}
		else
			Ac0Cfg.field.AcTxop = 0;	/* QID_AC_BE*/
			
		Ac0Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac0Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac0Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	/* QID_AC_BK*/
		Ac1Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac1Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac1Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		{
			Ac2Cfg.field.AcTxop = 192;	/* AC_VI: 192*32us ~= 6ms*/
			Ac3Cfg.field.AcTxop = 96;	/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			Ac2Cfg.field.AcTxop = 96;	/* AC_VI: 96*32us ~= 3ms*/
			Ac3Cfg.field.AcTxop = 48;	/* AC_VO: 48*32us ~= 1.5ms*/
		}
		Ac2Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac2Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac2Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = pAd->wmm_cw_min;
		Ac3Cfg.field.Cwmax = pAd->wmm_cw_max;
		Ac3Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = 0;		/* QID_AC_BE*/
		csr0.field.Ac1Txop = 0;		/* QID_AC_BK*/
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		{
			csr1.field.Ac2Txop = 192;		/* AC_VI: 192*32us ~= 6ms*/
			csr1.field.Ac3Txop = 96;		/* AC_VO: 96*32us  ~= 3ms*/
		}
		else
		{
			csr1.field.Ac2Txop = 96;		/* AC_VI: 96*32us ~= 3ms*/
			csr1.field.Ac3Txop = 48;		/* AC_VO: 48*32us ~= 1.5ms*/
		}
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin1 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin2 = pAd->wmm_cw_min;
		CwminCsr.field.Cwmin3 = pAd->wmm_cw_min;
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax1 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax2 = pAd->wmm_cw_max;
		CwmaxCsr.field.Cwmax3 = pAd->wmm_cw_max;
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, 0x00002222);

		NdisZeroMemory(&pAd->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));

	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		/*========================================================*/
		/*      MAC Register has a copy.*/
		/*========================================================*/
		
		/* Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27*/
		/* To degrade our VIDEO Queue's throughput for WiFi WMM S3T07 Issue.*/
		
		/*pEdcaParm->Txop[QID_AC_VI] = pEdcaParm->Txop[QID_AC_VI] * 7 / 10;  rt2860c need this		*/

		Ac0Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BE];
		Ac0Cfg.field.Cwmin= pEdcaParm->Cwmin[QID_AC_BE];
		Ac0Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BE];
		Ac0Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BE]; /*+1;*/

		Ac1Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BK];
		Ac1Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_BK]; /*+2; */
		Ac1Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BK];
		Ac1Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BK]; /*+1;*/


		Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
#ifdef RTMP_RBUS_SUPPORT
		if(pAd->Antenna.field.TxPath == 1)
		{
			Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI] + 1;
			Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI] + 1;			
		}
		else
#endif 			
		{
			Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI];
			Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI];
		}
		/*sync with window 20110524*/
		Ac2Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VI] + 1; /* 5.2.27 T6 Pass Tx VI+BE, but will impack 5.2.27/28 T7. Tx VI*/
		
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			Ac2Cfg.field.Aifsn = 0x3; /*for WiFi WMM A1-T07.*/
#endif /* CONFIG_AP_SUPPORT */
#endif /* INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				Ac2Cfg.field.Aifsn -= 1; 

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}
			
		}
#endif /* CONFIG_STA_SUPPORT */

		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

		if (pAd->CommonCfg.bWiFiTest)
		{
			if (Ac3Cfg.field.AcTxop == 102)
			{
			Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
				Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; /* AIFSN must >= 1 */
			Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
				Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
			Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
			}
		}

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
		/* STA TestBed changes in this item: for sta wifitest 5.2.32, 2011/04/11 */
		/* just for 5390 5392 pci, 5370 5372 not need this patch */
		if((IS_RT5390(pAd) || IS_RT5392(pAd)) && pEdcaParm->Aifsn[QID_AC_VI] == 10)
		{                   
			Ac0Cfg.field.AcTxop = 38;
		}
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */


		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


		/*========================================================*/
		/*      DMA Register has a copy too.*/
		/*========================================================*/
		csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
		csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);

		csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
		csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
		CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
		CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO];
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; /*for TGn wifi test*/
#endif /* CONFIG_STA_SUPPORT */
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
		CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
		CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
		CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		AifsnCsr.word = 0;
		AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BE];*/
		AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_BK];*/
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
		AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VI];*/
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
			AifsnCsr.field.Aifsn2 = 0x2; /*pEdcaParm->Aifsn[QID_AC_VI]; for WiFi WMM A1-T07.*/
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Tuning for Wi-Fi WMM S06*/
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

			/* Tuning for TGn Wi-Fi 5.2.32*/
			/* STA TestBed changes in this item: connexant legacy sta ==> broadcom 11n sta*/
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				AifsnCsr.field.Aifsn0 = 3;
				AifsnCsr.field.Aifsn2 = 7;
			}

			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; /*pEdcaParm->Aifsn[QID_AC_VO]*/
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; /*pEdcaParm->Aifsn[QID_AC_VO]; for TGn wifi test*/

			/* TODO: Is this modification also suitable for RT3052/RT3050 ???*/
			if (0 
			)
			{
				AifsnCsr.field.Aifsn2 = 0x2; /*pEdcaParm->Aifsn[QID_AC_VI]; for WiFi WMM S4-T04.*/
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

		NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}

	}

	pAd->CommonCfg.RestoreBurstMode = Ac0Cfg.word;
}



INT AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
	TX_RTY_CFG_STRUC tx_rty_cfg;
	
	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		tx_rty_cfg.field.ShortRtyLimit = limit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		tx_rty_cfg.field.LongRtyLimit = limit;
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);
	
	return TRUE;
}


UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
	TX_RTY_CFG_STRUC tx_rty_cfg = {.word = 0};
	
	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	if (type == TX_RTY_CFG_RTY_LIMIT_SHORT)
		return tx_rty_cfg.field.ShortRtyLimit;
	else if (type == TX_RTY_CFG_RTY_LIMIT_LONG)
		return tx_rty_cfg.field.LongRtyLimit;

	return 0;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime) 
{
	ULONG	SlotTime;
	UINT32	RegValue = 0;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = TRUE;
#endif /* CONFIG_STA_SUPPORT */

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
#endif /* DOT11_N_SUPPORT */
			)
		{
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is TRUE.*/
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	
	/* For some reasons, always set it to short slot time.*/
	/* ToDo: Should consider capability with 11B*/
#ifdef CONFIG_STA_SUPPORT 
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.BssType == BSS_ADHOC)	
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	RTMP_IO_WRITE32(pAd, BKOFF_SLOT_CFG, RegValue);
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPGetTxTscFromAsic(RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc)
{
	USHORT Wcid;
	USHORT offset;
	UCHAR IvEiv[8];
	INT i;

	/* Sanity check of apidx */
	if (apidx >= MAX_MBSSID_NUM(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPGetTxTscFromAsic : invalid apidx(%d)\n", apidx));
		return;
	}

	/* Initial value */
	NdisZeroMemory(IvEiv, 8);
	NdisZeroMemory(pTxTsc, 6);

	/* Get apidx for this BSSID */
	GET_GroupKey_WCID(pAd, Wcid, apidx);	

	/* When the group rekey action is triggered, a count-down(3 seconds) is started. 
	   During the count-down, use the initial PN as TSC.
	   Otherwise, get the IVEIV from ASIC. */
	if (pAd->ApCfg.MBSSID[apidx].RekeyCountDown > 0)
	{
		/*
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding 
		temporal key is initialized or refreshed. */	
		IvEiv[0] = 1;
	}
	else
	{
		UINT32 temp1, temp2;
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */
		/* Read IVEIV from Asic */
		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);
		
		/* Use Read32 to avoid endian problem */
		RTMP_IO_READ32(pAd, offset, &temp1);
		RTMP_IO_READ32(pAd, offset+4, &temp2);
		for ( i=0; i<4; i++)
		{
			IvEiv[i] = (UCHAR)(temp1 >> (i*8));
			IvEiv[i+4] = (UCHAR)(temp2 >> (i*8));
		}
	}

	/* Record current TxTsc */
	if (pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus == Ndis802_11AESEnable)
	{	/* AES */
		*pTxTsc 	= IvEiv[0];
		*(pTxTsc+1) = IvEiv[1];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];					
	}
	else
	{	/* TKIP */
		*pTxTsc 	= IvEiv[2];
		*(pTxTsc+1) = IvEiv[0];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];	
	}
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPGetTxTscFromAsic : WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n", 
									Wcid, *pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));
			

}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================
	Description:
		Add Shared key information into ASIC. 
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.
		
    Return:
	========================================================================
*/
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; /*, csr0;*/
	SHAREDKEY_MODE_STRUC csr1;
	UCHAR org_bssindex;
#ifdef RTMP_MAC_PCI
	INT   i;
#endif /* RTMP_MAC_PCI */

	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicAddSharedKeyEntry BssIndex=%d, KeyIdx=%d\n", BssIndex,KeyIdx));
/*============================================================================================*/

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddSharedKeyEntry: %s key #%d\n", CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
/*============================================================================================*/

	org_bssindex = BssIndex;
	if (BssIndex >= 8)
		BssIndex -= 8;

	{
		/* fill key material - key + TX MIC + RX MIC*/
		UINT32 share_key_base = 0, share_key_size = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (org_bssindex >= 8)
				share_key_base = RLT_SHARED_KEY_TABLE_BASE_EXT;
			else
			share_key_base = RLT_SHARED_KEY_TABLE_BASE;
			share_key_size = RLT_HW_KEY_ENTRY_SIZE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			if (org_bssindex >= 8)
				share_key_base = SHARED_KEY_TABLE_BASE_EXT;
			else
			share_key_base = SHARED_KEY_TABLE_BASE;
			share_key_size = HW_KEY_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		offset = share_key_base + (4*BssIndex + KeyIdx)*share_key_size;
#ifdef RTMP_MAC_PCI
		{
			for (i=0; i<MAX_LEN_OF_SHARE_KEY; i++) 
			{
				RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
			}

			offset += MAX_LEN_OF_SHARE_KEY;
			if (pTxMic)
			{
				for (i=0; i<8; i++)
				{
					RTMP_IO_WRITE8(pAd, offset + i, pTxMic[i]);
				}
			}

			offset += 8;
			if (pRxMic)
			{
				for (i=0; i<8; i++)
				{
					RTMP_IO_WRITE8(pAd, offset + i, pRxMic[i]);
				}
			}
		}
#endif /* RTMP_MAC_PCI */

	}

	{
		UINT32 share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (org_bssindex >= 8)
				share_key_mode_base = RLT_SHARED_KEY_MODE_BASE_EXT;
			else
			share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			if (org_bssindex >= 8)
				share_key_mode_base = SHARED_KEY_MODE_BASE_EXT;
			else
			share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		/* Update cipher algorithm. WSTA always use BSS0*/
		RTMP_IO_READ32(pAd, share_key_mode_base + 4 * (BssIndex/2), &csr1.word);
		DBGPRINT(RT_DEBUG_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
		if ((BssIndex%2) == 0)
		{
			if (KeyIdx == 0)
				csr1.field.Bss0Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss0Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss0Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss0Key3CipherAlg = CipherAlg;
		}
		else
		{
			if (KeyIdx == 0)
				csr1.field.Bss1Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss1Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss1Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss1Key3CipherAlg = CipherAlg;
		}
		DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		RTMP_IO_WRITE32(pAd, share_key_mode_base+ 4 * (BssIndex/2), csr1.word);
	}
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	/*ULONG SecCsr0;*/
	SHAREDKEY_MODE_STRUC csr1;

	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

	{
		UINT32 share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			share_key_mode_base= RLT_SHARED_KEY_MODE_BASE;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			share_key_mode_base = SHARED_KEY_MODE_BASE;
#endif /* RTMP_MAC */

		RTMP_IO_READ32(pAd, share_key_mode_base+4*(BssIndex/2), &csr1.word);
		if ((BssIndex%2) == 0)
		{
			if (KeyIdx == 0)
				csr1.field.Bss0Key0CipherAlg = 0;
			else if (KeyIdx == 1)
				csr1.field.Bss0Key1CipherAlg = 0;
			else if (KeyIdx == 2)
				csr1.field.Bss0Key2CipherAlg = 0;
			else
				csr1.field.Bss0Key3CipherAlg = 0;
		}
		else
		{
			if (KeyIdx == 0)
				csr1.field.Bss1Key0CipherAlg = 0;
			else if (KeyIdx == 1)
				csr1.field.Bss1Key1CipherAlg = 0;
			else if (KeyIdx == 2)
				csr1.field.Bss1Key2CipherAlg = 0;
			else
				csr1.field.Bss1Key3CipherAlg = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
		RTMP_IO_WRITE32(pAd, share_key_mode_base+4*(BssIndex/2), csr1.word);
	}
	ASSERT(BssIndex < 4);
	ASSERT(KeyIdx < 4);

}


VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;
	UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
		iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
		iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
	}
#endif /* RTMP_MAC */

	offset = iveiv_tb_base + (WCID * iveiv_tb_size);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: wcid(%d) 0x%08lx, 0x%08lx \n", 
									__FUNCTION__, WCID, uIV, uEIV));	
}


VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT WCID,
	IN PUCHAR pAddr)
{
	ULONG offset;
	ULONG Addr;

	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);	
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);
}
	

/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:

    Note :
		The key table selection rule :
    	1.	Wds-links and Mesh-links always use Pair-wise key table. 	
		2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled, 
			it needs to set key into Pair-wise Key Table.
		3.	The pair-wise key security mode is set NONE, it means as no security.
		4.	In STA Adhoc mode, it always use shared key table.
		5.	Otherwise, use shared key table

	========================================================================
*/
VOID AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;	
	USHORT offset;
	UINT32 wcid_attr_base = 0, wcid_attr_size = 0;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
		wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
		wcid_attr_size = HW_WCID_ATTRI_SIZE;
	}
#endif /* RTMP_MAC */

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* The limitation of HW WCID table */
	if (Wcid > 254)
	{		
		DBGPRINT(RT_DEBUG_WARN, ("%s:Invalid wcid(%d)\n", __FUNCTION__, Wcid));
		return;
	}

	/* Update the pairwise key security mode.
	   Use bit10 and bit3~1 to indicate the pairwise cipher mode */	
	WCIDAttri.field.PairKeyModeExt = ((CipherAlg & 0x08) >> 3);
	WCIDAttri.field.PairKeyMode = (CipherAlg & 0x07);

	/* Update the MBSS index.
	   Use bit11 and bit6~4 to indicate the BSS index */	
	WCIDAttri.field.BSSIdxExt = ((BssIdx & 0x08) >> 3);
	WCIDAttri.field.BSSIdx = (BssIdx & 0x07);

#ifdef WAPI_SUPPORT
	/* Update WAPI related information */
	if (CipherAlg == CIPHER_SMS4)
	{
		if (KeyTabFlag == SHAREDKEYTABLE)
			WCIDAttri.field.WAPI_MCBC = 1;
		WCIDAttri.field.WAPIKeyIdx = ((KeyIdx == 0) ? 0 : 1); 
	}
#endif /* WAPI_SUPPORT */
	
	/* Assign Key Table selection */		
	WCIDAttri.field.KeyTab = KeyTabFlag;

	/* Update related information to ASIC */
	offset = wcid_attr_base + (Wcid * wcid_attr_size);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:WCID #%d, KeyIdx #%d, WCIDAttri=0x%x, Alg=%s\n",
					__FUNCTION__, Wcid, KeyIdx, WCIDAttri.word, CipherName[CipherAlg]));
}


/*
	==========================================================================
	Description:   

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx) 
{
	UINT32 offset;
	UCHAR cnt, cnt_s, cnt_e;
#ifdef MCS_LUT_SUPPORT
	UINT32 mcs_tb_offset = 0;
#endif /* MCS_LUT_SUPPORT */


	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>wcid_idx = 0x%x\n",wcid_idx));
	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WCID_ALL - 1);
	} else {
#ifdef MCS_LUT_SUPPORT
		if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT))
			mcs_tb_offset = 0x400;
#endif /* MCS_LUT_SUPPORT */
		cnt_s = cnt_e = wcid_idx;
	}
	
	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++)
	{
		offset = MAC_WCID_BASE + cnt * HW_WCID_ENTRY_SIZE;
		RTMP_IO_WRITE32(pAd, offset, 0x0);
		RTMP_IO_WRITE32(pAd, offset + 4, 0x0);
#ifdef MCS_LUT_SUPPORT
		if (mcs_tb_offset) {
			offset += mcs_tb_offset;
			RTMP_IO_WRITE32(pAd, offset, 0x0);
			RTMP_IO_WRITE32(pAd, offset + 4, 0x0);
		}
#endif /* MCS_LUT_SUPPORT */
	}
}
	

/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC. 
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table
				
    Return:
	========================================================================
*/
VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG offset;
	UINT32 pairwise_key_base = 0, pairwise_key_len = 0;
	UCHAR *pKey = pCipherKey->Key;
	UCHAR *pTxMic = pCipherKey->TxMic;
	UCHAR *pRxMic = pCipherKey->RxMic;
	UCHAR CipherAlg = pCipherKey->CipherAlg;
#ifdef RTMP_MAC
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	unsigned long irqFlag = 0;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		pairwise_key_base = RLT_PAIRWISE_KEY_TABLE_BASE;
		pairwise_key_len = RLT_HW_KEY_ENTRY_SIZE;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		pairwise_key_base = PAIRWISE_KEY_TABLE_BASE;
		pairwise_key_len = HW_KEY_ENTRY_SIZE;
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
		RTMP_MAC_SHR_MSEL_LOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */
	}
#endif /* RTMP_MAC */

	/* EKEY */
	offset = pairwise_key_base + (WCID * pairwise_key_len);
#ifdef RTMP_MAC_PCI
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif /* RTMP_MAC_PCI */
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i+=4)
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, offset + i, &Value);
	}
	offset += MAX_LEN_OF_PEER_KEY;

	/*  MIC KEY */
	if (pTxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pTxMic[i]);
		}
#endif /* RTMP_MAC_PCI */
	}
	offset += 8;

	if (pRxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pRxMic[i]);
		}
#endif /* RTMP_MAC_PCI */
	}
#ifdef RTMP_MAC
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
	}
#endif /* SPECIFIC_BCN_BUF_SUPPORT*/
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_MAC */
	DBGPRINT(RT_DEBUG_TRACE,("AsicAddPairwiseKeyEntry: WCID #%d Alg=%s\n",WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
}


/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC. 

    Return:
	========================================================================
*/	
VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	AsicUpdateWcidAttributeEntry(pAd, 
							  BSS0,
							  0,
							  CIPHER_NONE, 
							  Wcid,
							  PAIRWISEKEYTABLE);
}


BOOLEAN AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef RT65xx
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd))
		return TRUE;
#endif /* RT65xx */


#ifdef RTMP_PCI_SUPPORT
	if (IS_PCI_INF(pAd))
		in_atomic = TRUE;
#endif /* RTMP_USB_SUPPORT */
	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
	else
		return FALSE;
}


BOOLEAN AsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	BOOLEAN cmd_done = TRUE;
	
	AsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
	
#ifdef RTMP_MAC_PCI
	cmd_done = AsicCheckCommanOk(pAd, Token);
#endif /* RTMP_MAC_PCI */

	return cmd_done;
}


BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
#ifdef RT65xx
	// TODO: shiang-6590, fix me, currently firmware is not ready yet, so ignore it!
	if (IS_RT65XX(pAd))
		return TRUE;
#endif /* RT65xx */


	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);
	else
		return FALSE;
}

/*
	========================================================================
	Description:
		For 1x1 chipset : 2070 / 3070 / 3090 / 3370 / 3390 / 5370 / 5390 
		Usage :	1. Set Default Antenna as initialize
				2. Antenna Diversity switching used
				3. iwpriv command switch Antenna

    Return:
	========================================================================
 */
VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	if (pAd->chipOps.SetRxAnt)
		pAd->chipOps.SetRxAnt(pAd, Ant);
}


VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER pAd, 
	IN	UCHAR		Channel) 
{
	if (pAd->chipOps.AsicRfTurnOff)
	{
		pAd->chipOps.AsicRfTurnOff(pAd);
	}
	else
	{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
		/* RF R2 bit 18 = 0*/
		UINT32			R1 = 0, R2 = 0, R3 = 0;
		UCHAR			index;
		RTMP_RF_REGS	*RFRegTable;
	
		RFRegTable = RF2850RegTable;
#endif /* defined(RT28xx) || defined(RT2880) || defined(RT2883) */

		switch (pAd->RfIcType)
		{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
#if defined(RT28xx) || defined(RT2880)
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif /* defined(RT28xx) || defined(RT2880) */
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R1 = RFRegTable[index].R1 & 0xffffdfff;
						R2 = RFRegTable[index].R2 & 0xfffbffff;
						R3 = RFRegTable[index].R3 & 0xfff3ffff;

						RTMP_RF_IO_WRITE32(pAd, R1);
						RTMP_RF_IO_WRITE32(pAd, R2);

						/* Program R1b13 to 1, R3/b18,19 to 0, R2b18 to 0. */
						/* Set RF R2 bit18=0, R3 bit[18:19]=0*/
						/*if (pAd->StaCfg.bRadio == FALSE)*/
						if (1)
						{
							RTMP_RF_IO_WRITE32(pAd, R3);

							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x,  R3 = 0x%08x \n",
								Channel, pAd->RfIcType, R2, R3));
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x \n",
								Channel, pAd->RfIcType, R2));
						break;
					}
				}
				break;
#endif /* defined(RT28xx) || defined(RT2880) || defined(RT2883) */
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d : Unkonwn RFIC=%d\n",
											Channel, pAd->RfIcType));
				break;
		}
	}
}


#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high)
{
	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		UINT32 offset;
		UINT32 wapi_pn_base = 0, wapi_pn_size = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wapi_pn_base = RLT_WAPI_PN_TABLE_BASE;
			wapi_pn_size = RLT_WAPI_PN_ENTRY_SIZE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wapi_pn_base = WAPI_PN_TABLE_BASE;
			wapi_pn_size = WAPI_PN_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		offset = wapi_pn_base + (WCID * wapi_pn_size);

		RTMP_IO_WRITE32(pAd, offset, pn_low);
		RTMP_IO_WRITE32(pAd, offset + 4, pn_high);
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n", 
								__FUNCTION__));
	}
	
}
#endif /* WAPI_SUPPORT */



#ifdef VCORECAL_SUPPORT
VOID AsicVCORecalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR RFValue = 0;
	UINT32 TxPinCfg = 0;
	UINT8 mode = pAd->chipCap.FlgIsVcoReCalMode;

	if (mode == VCO_CAL_DISABLE)
		return;

#ifdef RT6352
	if (IS_RT6352(pAd) && (pAd->bCalibrationDone == FALSE))
		return;
#endif /* RT6352 */


#ifdef RTMP_INTERNAL_TX_ALC
#endif /* RTMP_INTERNAL_TX_ALC */

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	TxPinCfg &= 0xFCFFFFF0;
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

	switch (mode)
	{
		case VCO_CAL_MODE_1:
			RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x01; /* bit 0 = vcocal_en */
			RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);
			break;

		case VCO_CAL_MODE_2:
			RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7 = vcocal_en */
			RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
			break;

		case VCO_CAL_MODE_3:
#ifdef RT6352
			if (IS_RT6352(pAd)) {
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R05, 0x40);
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, 0x0C);

				RT635xReadRFRegister(pAd, RF_BANK0, RF_R04, &RFValue);
				RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, RFValue);
			}
#endif /* RT6352 */
#ifdef RT8592
			if (IS_RT8592(pAd))
			{
				RT30xxReadRFRegister(pAd, RF_R05, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x80; /* bit 7 = vcocal_en */
				RT30xxWriteRFRegister(pAd, RF_R05, (UCHAR)RFValue);
			}
#endif /* RT8592 */
			break;
			
		default:
			return;
	}

	if (mode == VCO_CAL_MODE_3 && (!IS_RT6352(pAd)))
		RtmpusecDelay(100);
	else
		RtmpOsMsDelay(1);

	RTMP_IO_READ32(pAd, TX_PIN_CFG, &TxPinCfg);
	if (pAd->CommonCfg.Channel <= 14)
	{
		if (pAd->Antenna.field.TxPath == 1
#ifdef GREENAP_SUPPORT
			|| pAd->ApCfg.bGreenAPActive == TRUE	 /* avoid to corrupt GreenAP operation */
#endif /* GREENAP_SUPPORT */
		)
			TxPinCfg |= 0x2;
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg |= 0xA;
		else if (pAd->Antenna.field.TxPath == 3)
			TxPinCfg |= 0x0200000A;
	}
	else
	{
		if (pAd->Antenna.field.TxPath == 1
#ifdef GREENAP_SUPPORT
			|| pAd->ApCfg.bGreenAPActive == TRUE	 /* avoid to corrupt GreenAP operation */
#endif /* GREENAP_SUPPORT */
		)
			TxPinCfg |= 0x1;
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg |= 0x5;
		else if (pAd->Antenna.field.TxPath == 3)
			TxPinCfg |= 0x01000005;
	}
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef TXBF_SUPPORT
		// Do a Divider Calibration and update BBP registers
		if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_CAL)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			pAd->chipOps.fITxBfDividerCalibration(pAd, 2, 0, NULL);
		}

		if (pAd->CommonCfg.ETxBfEnCond)
		{
			INT idx;
			
			for (idx = 1; idx < MAX_LEN_OF_MAC_TABLE; idx++)
			{
				MAC_TABLE_ENTRY *pEntry;

				pEntry = &pAd->MacTab.Content[idx];
				if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->eTxBfEnCond))
				{
					BOOLEAN Cancelled;

					RTMPCancelTimer(&pEntry->eTxBfProbeTimer, &Cancelled);
					pEntry->bfState = READY_FOR_SNDG0;
					eTxBFProbing(pAd, pEntry);
				}
			}
		}
#endif // TXBF_SUPPORT //
}
#endif /* VCORECAL_SUPPORT */


#ifdef STREAM_MODE_SUPPORT
// StreamModeRegVal - return MAC reg value for StreamMode setting
UINT32 StreamModeRegVal(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 streamWord;

	switch (pAd->CommonCfg.StreamMode)
	{
		case 1:
			streamWord = 0x030000;
			break;
		case 2:
			streamWord = 0x0c0000;
			break;
		case 3:
			streamWord = 0x0f0000;
			break;
		default:
			streamWord = 0x0;
			break;
	}

	return streamWord;
}


/*
	========================================================================
	Description:
		configure the stream mode of specific MAC or all MAC and set to ASIC. 

	Prameters:
		pAd		 --- 
		pMacAddr ---
		bClear	 --- disable the stream mode for specific macAddr when
						(pMacAddr!=NULL)
	
    Return:
	========================================================================
*/
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	UINT32 streamWord;
	UINT32 regAddr, regVal;

	
	if (!pAd->chipCap.FlgHwStreamMode)
		return;
		
	streamWord = StreamModeRegVal(pAd);
	if (!bEnabled)
		streamWord = 0;

	regAddr = TX_CHAIN_ADDR0_L + chainIdx * 8;
	RTMP_IO_WRITE32(pAd, regAddr,  
					(UINT32)(pMacAddr[0]) | 
					(UINT32)(pMacAddr[1] << 8)  | 
					(UINT32)(pMacAddr[2] << 16) | 
					(UINT32)(pMacAddr[3] << 24));
	
	RTMP_IO_READ32(pAd, regAddr + 4, &regVal);
	regVal &= (~0x000f0000);
	RTMP_IO_WRITE32(pAd, regAddr + 4, 
					(regVal | streamWord) | 
					(UINT32)(pMacAddr[4]) | 
					(UINT32)(pMacAddr[5] << 8));
	
}


VOID RtmpStreamModeInit(RTMP_ADAPTER *pAd)
{
	int chainIdx;
	UCHAR *pMacAddr;

	if (pAd->chipCap.FlgHwStreamMode == FALSE)
		return;	
	
	for (chainIdx = 0; chainIdx < STREAM_MODE_STA_NUM; chainIdx++)
	{
		pMacAddr = &pAd->CommonCfg.StreamModeMac[chainIdx][0];
		AsicSetStreamMode(pAd, pMacAddr, chainIdx, TRUE);
	}
}
#endif // STREAM_MODE_SUPPORT //


VOID AsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	AUTO_RSP_CFG_STRUC csr4;
		
	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &csr4.word);
	if (TxPreamble == Rt802_11PreambleLong)
		csr4.field.AutoResponderPreamble = 0;
	else
		csr4.field.AutoResponderPreamble = 1;
	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, csr4.word);
}

	
#ifdef DOT11_N_SUPPORT
INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	if (enable)
	{
	pAd->CommonCfg.RestoreBurstMode = Data;
	Data  &= 0xFFF00000;
	Data  |= 0x86380;
	} else {
	Data = pAd->CommonCfg.RestoreBurstMode;
	Data &= 0xFFFFFF00;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif // DOT11_N_SUPPORT //
	)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
			Data |= 0x80;
		else if (pAd->CommonCfg.bEnableTxBurst)
			Data |= 0x20;
	}
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

	if (enable)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);
	else
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE);
	
	return TRUE;
}
#endif // DOT11_N_SUPPORT //




#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	IN PRTMP_ADAPTER pAd
)
{
	if (pAd->chipOps.AsicMeasureFalseCCA)
		pAd->chipOps.AsicMeasureFalseCCA(pAd);
}

VOID AsicMitigateMicrowave(
	IN PRTMP_ADAPTER pAd
)
{
	if (pAd->chipOps.AsicMitigateMicrowave)
		pAd->chipOps.AsicMitigateMicrowave(pAd);
}
#endif /* MICROWAVE_OVEN_SUPPORT */


INT AsicSetPreTbttInt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 val;
	
	RTMP_IO_READ32(pAd, INT_TIMER_CFG, &val);
	val &= 0xffff0000;
	val |= 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */
	RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, val);
	/* Enable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &val);
	val |=0x1;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, val);

	return TRUE;
}


BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;


	do {
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
			DBGPRINT(RT_DEBUG_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return TRUE;
		}
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;
		RtmpusecDelay(wait_us);
	}while ((i++) < round);

	DBGPRINT(RT_DEBUG_TRACE, ("==>  DMABusy, GloCfg=0x%x\n", GloCfg.word));
	
	return FALSE;
}


INT rtmp_asic_top_init(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

#ifdef RLT_MAC
#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		UINT32 MacValue;
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacValue);
		pAd->MACVersion = MacValue;
	
		if ((pAd->MACVersion == 0xffffffff) || (pAd->MACVersion == 0))
			mt76x2_pwrOn(pAd);
	}
#endif

	if (IS_MT76x0(pAd) || IS_MT76x2(pAd) || IS_MT7601(pAd)) {
		if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
			rlt_wlan_chip_onoff(pAd, TRUE, FALSE);
	}
#endif /* RLT_MAC */


#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	    	/* If dirver doesn't wake up firmware here,*/
	    	/* NICLoadFirmware will hang forever when interface is up again.*/
	    	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) &&
	        	OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	    	{
	        	AUTO_WAKEUP_STRUC AutoWakeupCfg;

			AsicForceWakeup(pAd, TRUE);
	        	AutoWakeupCfg.word = 0;
		    	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	        	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
	    	}
	}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	/* Make sure MAC gets ready.*/
	if (WaitForAsicReady(pAd) != TRUE)
		return FALSE;

#ifdef RTMP_MAC_PCI

#if defined(RT65xx) || defined(MT7601)
	if (IS_RT65XX(pAd) || IS_MT7601(pAd))
		mac_val = 0x0;
	else
#endif /* defined(RT65xx) || defined(MT7601) */
		mac_val = 0x2;	/* To fix driver disable/enable hang issue when radio off*/
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, mac_val);
#endif /* RTMP_MAC_PCI */

	return TRUE;
}


INT StopDmaRx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	PNDIS_PACKET pRxPacket;
	RX_BLK RxBlk, *pRxBlk;
	UINT32 RxPending = 0, MacReg = 0, MTxCycle = 0;
	BOOLEAN bReschedule = FALSE;
	BOOLEAN bCmdRspPacket = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/*
		process whole rx ring
	*/
	while (1)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return 0;
		pRxBlk = &RxBlk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, &bCmdRspPacket, 0);
		if ((RxPending == 0) && (bReschedule == FALSE))
		{
			if (pRxPacket)
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			
			break;
		}
		else
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
	}

	/*
		Check DMA Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI

		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if (MacReg & 0x8)
		{
			RtmpusecDelay(50);
		}
		else
			break;
		
#endif

		
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:RX DMA busy!! DMA_CFG = 0x%08x\n", __FUNCTION__, MacReg));
	}

	if (Level == RTMP_HALT)
	{
		/* Disable DMA RX */
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x4);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT StopDmaTx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	UINT32 MacReg = 0, MTxCycle = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if ((MacReg & 0x2) == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("==>  DMA Tx Idle, MacReg=0x%x\n", MacReg));
			break;
		}
#endif

		
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("TX DMA busy!! DMA_CFG(%x)\n", MacReg));
	}

	if (Level == RTMP_HALT)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x00000001);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif

	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


#ifdef DOT11_N_SUPPORT
#ifdef MT76x2
#define MAX_AGG_CNT	48
#elif defined(RT65xx) || defined(MT7601)
#define MAX_AGG_CNT	32
#elif defined(RT2883) || defined(RT3883)
#define MAX_AGG_CNT	16
#else
#define MAX_AGG_CNT	8
#endif
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	UINT32 reg_addr;
	TX_AGG_CNT_STRUC reg_val;
	int i, cnt, seg;
	static USHORT aggReg[] = {
						TX_AGG_CNT, TX_AGG_CNT3,
#if MAX_AGG_CNT > 8
						TX_AGG_CNT4, TX_AGG_CNT7,
#endif
#if MAX_AGG_CNT > 16
						TX_AGG_CNT8, TX_AGG_CNT15, 
#endif
#if MAX_AGG_CNT > 32
						TX_AGG_CNT16, TX_AGG_CNT23,
#endif
	};


	NdisZeroMemory(aggCnt, cnt_len * sizeof(ULONG));
	seg = (sizeof(aggReg) /sizeof(USHORT));

	cnt = 0;
	for (i = 0; i < seg; i += 2)
	{
		for (reg_addr = aggReg[i] ; reg_addr <= aggReg[i+1] ; reg_addr += 4)
		{
			RTMP_IO_READ32(pAd, reg_addr, &reg_val.word);
			if (cnt < (cnt_len -1)) {
				aggCnt[cnt] = reg_val.field.AggCnt_x;
				aggCnt[cnt+1] = reg_val.field.AggCnt_y;
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) with val(0x%08x) [AGG_%d=>%ld, AGG_%d=>%ld]\n",
						__FUNCTION__, reg_addr, reg_val.word, cnt, aggCnt[cnt], cnt+1, aggCnt[cnt+1]));
				cnt += 2;
			} else {
				DBGPRINT(RT_DEBUG_TRACE, ("%s():Get AggSize at Reg(0x%x) failed, no enough buffer(cnt_len=%d, cnt=%d)\n",
							__FUNCTION__, reg_addr, cnt_len, cnt));
			}
		}
	}

	return TRUE;
}

#endif /* DOT11_N_SUPPORT */


INT AsicSetChannel(RTMP_ADAPTER *pAd, UCHAR ch, UINT8 bw, UINT8 ext_ch, BOOLEAN bScan)
{
	bbp_set_bw(pAd, bw);

	/*  Tx/RX : control channel setting */
	bbp_set_ctrlch(pAd, ext_ch);
	rtmp_mac_set_ctrlch(pAd, ext_ch);

	/* Let BBP register at 20MHz to do scan */
	AsicSwitchChannel(pAd, ch, bScan);
	AsicLockChannel(pAd, ch);

#ifdef RT28xx
	RT28xx_ch_tunning(pAd, bw);
#endif /* RT28xx */

	return 0;
}


#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pBssid,
	IN UCHAR index) 
{
	UINT32		  Addr4 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s():%x:%x:%x:%x:%x:%x\n",
				__FUNCTION__, PRINT_MAC(pBssid)));
	
	Addr4 = (UINT32)(pBssid[0]) |
			(UINT32)(pBssid[1] << 8)  |
			(UINT32)(pBssid[2] << 16) |
			(UINT32)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_APCLI_BSSID_DW0, Addr4);

	Addr4 = 0;
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);
	/* Enable APCLI mode */
	Addr4 |= 0x10000;

	RTMP_IO_WRITE32(pAd, MAC_APCLI_BSSID_DW1, Addr4);
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(
	IN PRTMP_ADAPTER pAd)
{
	NEW_WOW_MASK_CFG_STRUCT mask_cfg;
	NEW_WOW_SEC_CFG_STRUCT sec_cfg;
	NEW_WOW_INFRA_CFG_STRUCT infra_cfg;
	NEW_WOW_P2P_CFG_STRUCT p2p_cfg;
	NEW_WOW_PARAM_STRUCT wow_param;
	struct CMD_UNIT CmdUnit;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	INT32 Ret;
	MAC_TABLE_ENTRY *pEntry = NULL;


	NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));

	/* WOW enable */
	NdisZeroMemory(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_ENABLE; /* WOW enable */
	wow_param.Value = TRUE;

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	RtmpOsMsDelay(1);
	/* mask configuration */
	NdisZeroMemory(&mask_cfg, sizeof(mask_cfg));

	mask_cfg.Config_Type = WOW_MASK_CFG; 	/* detect mask config */
	mask_cfg.Function_Enable = TRUE;
	mask_cfg.Detect_Mask = 1UL << WOW_MAGIC_PKT;	/* magic packet */
	mask_cfg.Event_Mask = 0;
	
	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_MASK_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&mask_cfg;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed!!(%d/%d)\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, mask_cfg.Config_Type);
		return;
	}

	RtmpOsMsDelay(1);

	/* security configuration */
	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPAPSK)
	{
		NdisZeroMemory(&sec_cfg, sizeof(sec_cfg));
	
		sec_cfg.Config_Type = WOW_SEC_CFG; 	/* security config */

		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
			sec_cfg.WPA_Ver = 0;
		else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
			sec_cfg.WPA_Ver = 1;
		
		pEntry = &pAd->MacTab.Content[BSSID_WCID];
		
		NdisCopyMemory(sec_cfg.PTK, pEntry->PTK, 64);
		NdisCopyMemory(sec_cfg.R_COUNTER, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
		
		sec_cfg.Key_Id = pAd->StaCfg.DefaultKeyId;
		sec_cfg.Cipher_Alg = pEntry->WepStatus;
		printk("\x1b[31m%s: wep status %d\x1b[m\n", __FUNCTION__, pEntry->WepStatus);
		sec_cfg.Group_Cipher = pAd->StaCfg.GroupCipher;
		printk("\x1b[31m%s: group status %d\x1b[m\n", __FUNCTION__, sec_cfg.Group_Cipher);
		printk("\x1b[31m%s: aid %d\x1b[m\n", __FUNCTION__, pEntry->Aid);
		sec_cfg.WCID = BSSID_WCID;
		
		CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
		CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_SEC_CFG_STRUCT);
		CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&sec_cfg;
	
		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
	
		if (Ret != NDIS_STATUS_SUCCESS)
		{
			printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, sec_cfg.Config_Type);
			return;
		}
	
		RtmpOsMsDelay(1);
	}

	/* Infra configuration */

	NdisZeroMemory(&infra_cfg, sizeof(infra_cfg));

	infra_cfg.Config_Type = WOW_INFRA_CFG; 	/* infra config */

	COPY_MAC_ADDR(infra_cfg.STA_MAC, pAd->CurrentAddress);
	COPY_MAC_ADDR(infra_cfg.AP_MAC, pAd->CommonCfg.Bssid);

	CmdUnit.u.ANDES.Type = CMD_WOW_CONFIG; /* WOW config */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_INFRA_CFG_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&infra_cfg;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		infra_cfg.AP_Status = TRUE;
	else
		infra_cfg.AP_Status = FALSE;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, infra_cfg.Config_Type);
		return;
	}

	RtmpOsMsDelay(1);
	

	/* P2P configuration */

	/* Wakeup Option */
	NdisZeroMemory(&wow_param, sizeof(wow_param));

	wow_param.Parameter = WOW_WAKEUP; /* Wakeup Option */
	if (pAd->WOW_Cfg.bInBand)
	{
#ifdef RTMP_MAC_PCI
		wow_param.Value = WOW_WAKEUP_BY_PCIE;
#else
		wow_param.Value = WOW_WAKEUP_BY_USB;
#endif /* RTMP_MAC_PCI */
	}
	else
	{
		INT32 Value;
		
		wow_param.Value = WOW_WAKEUP_BY_GPIO;

		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */ 
		RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);
	}

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}

	RtmpOsMsDelay(1);


	/* traffic to Andes */
	NdisZeroMemory(&wow_param, sizeof(wow_param));
	wow_param.Parameter = WOW_TRAFFIC; /* Traffic switch */
	wow_param.Value = WOW_PKT_TO_ANDES;	/* incoming packet to FW */

	CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* feature enable */
	CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&wow_param.Parameter;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

	if (Ret != NDIS_STATUS_SUCCESS)
	{
		printk("\x1b[31m%s: send WOW config command failed(%d/%d)!!\x1b[m\n", __FUNCTION__,
					CmdUnit.u.ANDES.Type, wow_param.Parameter);
		return;
	}
	
	RtmpOsMsDelay(1);

    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
}

VOID RT28xxAndesWOWDisable(
    IN PRTMP_ADAPTER pAd)
{
    NEW_WOW_PARAM_STRUCT param;
    struct CMD_UNIT CmdUnit;
    RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
    INT32 Ret;
    UINT32 Value;
    MAC_TABLE_ENTRY *pEntry = NULL;

    printk("\x1b[31m%s: ...\x1b[m", __FUNCTION__);

    /* clean BulkIn Reset flag */
    //pAd->Flags &= ~0x80000;
    RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

    NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));

    /* WOW disable */
    NdisZeroMemory(&param, sizeof(param));
    param.Parameter = WOW_ENABLE;
    param.Value = FALSE;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE; /* WOW enable */
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    RtmpOsMsDelay(1);


    /* traffic to Host */
    NdisZeroMemory(&param, sizeof(param));
    param.Parameter = WOW_TRAFFIC;
    param.Value = WOW_PKT_TO_HOST;

    CmdUnit.u.ANDES.Type = CMD_WOW_FEATURE;
    CmdUnit.u.ANDES.CmdPayloadLen = sizeof(NEW_WOW_PARAM_STRUCT);
    CmdUnit.u.ANDES.CmdPayload = (PUCHAR)&param;

    Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

    if (Ret != NDIS_STATUS_SUCCESS)
    {
        printk("\x1b[31m%s: send WOW config command failed!!\x1b[m\n", __FUNCTION__);
        return;
    }

    RtmpOsMsDelay(1);


    /* Restore MAC TX/RX */
    RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
    Value |= 0xC;
    RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);


    RTUSBBulkReceive(pAd);
    RTUSBBulkCmdRspEventReceive(pAd);

    //printk("\x1b[31m%s: pendingRx %d\x1b[m\n", __FUNCTION__, pAd->PendingRx);
    //printk("\x1b[31m%s: BulkInReq %d\x1b[m\n", __FUNCTION__, pAd->BulkInReq);

    /* restore hardware remote wakeup flag */
    RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
    printk("\x1b[31m%s: 0x80 %08x\x1b[m\n", __FUNCTION__, Value);
    Value &= ~0x80;
    RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);

	if (pAd->WOW_Cfg.bInBand == FALSE)
	{
		INT32 Value;
		
		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &Value);
		printk("\x1b[31m%s: 0x80 = %x\x1b[m\n", __FUNCTION__, Value);
		Value &= ~0x01010000; /* GPIO0(ouput) --> 0(data) */ 
		RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, Value);
	}
}

#endif /* NEW_WOW_SUPPORT */

#ifdef THERMAL_PROTECT_SUPPORT
VOID thermal_protection(
	IN RTMP_ADAPTER 	*pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	INT32 temp_diff = 0, current_temp = 0;
#ifdef CONFIG_STA_SUPPORT
#endif /* RTMP_MAC_USB  */

	if (pAd->chipCap.ThermalProtectSup == FALSE)
		return;

	/* If MT7662U go into suspend mode, thermal clock will also be disabled.
	After resume, MCU will hang if driver retrieve thermal value without calibration. */
#ifdef CONFIG_STA_SUPPORT
#endif /* RTMP_MAC_USB  */

#ifdef MT76x2
	UINT32 mac_reg = 0;
	if (IS_MT76x2(pAd))
	{
		current_temp = pAd->chipCap.current_temp;
		/* 2T2R to 1T2R */
		if (pAd->thermal_HighEn == TRUE)
		{
			if ((current_temp > pAd->thermal_HighTempTh) &&
               (pAd->force_one_tx_stream == FALSE))
			{
				pAd->force_one_tx_stream = TRUE;
				
				/* 0x504: Trun on BIT[13][14] */
				RTMP_IO_READ32(pAd, RLT_RF_BYPASS_0, &mac_reg);
				mac_reg |= (3 << 13);
				RTMP_IO_WRITE32(pAd, RLT_RF_BYPASS_0, mac_reg);
		 
				/* 0x50C: Trun off BIT[13][14] */
				RTMP_IO_READ32(pAd, RLT_RF_SETTING_0, &mac_reg);
				mac_reg &= ~(3 << 13);
				RTMP_IO_WRITE32(pAd, RLT_RF_SETTING_0, mac_reg);
				
				DBGPRINT(RT_DEBUG_OFF, ("%s - current temp=%d > HighTempTh =%d switch to 1T\n",
					__FUNCTION__, current_temp, pAd->thermal_HighTempTh));
			}
		}

		/* 1T2R to 2T2R */
		if (pAd->thermal_LowEn == TRUE)
        {       
            if ((current_temp < pAd->thermal_LowTempTh) &&
	    		(pAd->force_one_tx_stream == TRUE))
			{	
				
                /* 0x504: Trun off BIT[13][14] */
                RTMP_IO_READ32(pAd, RLT_RF_BYPASS_0, &mac_reg);
                mac_reg &= ~(3 << 13);
                RTMP_IO_WRITE32(pAd, RLT_RF_BYPASS_0, mac_reg);
 
                /* 0x50C: Trun off BIT[13][14] */
                RTMP_IO_READ32(pAd, RLT_RF_SETTING_0, &mac_reg);
                mac_reg &= ~(3 << 13);
                RTMP_IO_WRITE32(pAd, RLT_RF_SETTING_0, mac_reg);
                                
                DBGPRINT(RT_DEBUG_OFF, ("%s - current temp=%d > HighTempTh =%d switch to 1T\n",
                        __FUNCTION__, current_temp, pAd->thermal_HighTempTh));

                pAd->force_one_tx_stream = FALSE;
				DBGPRINT(RT_DEBUG_OFF, ("%s - current temp=%d < HighTempTh =%d restore to 2T\n",
                                        __FUNCTION__, current_temp, pAd->thermal_LowTempTh));
			}
                }		

		return;
	}
#endif /* MT76x2 */

	RTMP_CHIP_GET_CURRENT_TEMP(pAd, current_temp);
	temp_diff = current_temp - pAd->last_thermal_pro_temp;
	pAd->last_thermal_pro_temp = current_temp;

	DBGPRINT(RT_DEBUG_INFO, ("%s - current temp=%d, temp diff=%d\n", 
					__FUNCTION__, current_temp, temp_diff));
	
	if (temp_diff > 0) {
		if (current_temp > (pAd->thermal_pro_criteria + 10) /* 90 */)
			RTMP_CHIP_THERMAL_PRO_2nd_COND(pAd);
		else if (current_temp > pAd->thermal_pro_criteria /* 80 */)
			RTMP_CHIP_THERMAL_PRO_1st_COND(pAd);
		else
			RTMP_CHIP_THERMAL_PRO_DEFAULT_COND(pAd);
	} else if (temp_diff < 0) {
		if (current_temp < (pAd->thermal_pro_criteria - 5) /* 75 */)
			RTMP_CHIP_THERMAL_PRO_DEFAULT_COND(pAd);
		else if (current_temp < (pAd->thermal_pro_criteria + 5) /* 85 */)
			RTMP_CHIP_THERMAL_PRO_1st_COND(pAd);
		else
			RTMP_CHIP_THERMAL_PRO_2nd_COND(pAd);
	}
}
#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef DROP_MASK_SUPPORT
VOID asic_set_drop_mask(
	PRTMP_ADAPTER ad,
	USHORT	wcid,
	BOOLEAN enable)
{
	UINT32 mac_reg = 0, mac_old, reg_id, group_index;
	UINT32 drop_mask = (1U << (wcid % 32));

	/* each group has 32 entries */
	group_index = (wcid - (wcid % 32)) >> 5 /* divided by 32 */;
	reg_id = (TX_WCID_DROP_MASK0 + 4*group_index);

	NdisAcquireSpinLock(&ad->drop_mask_lock);

	RTMP_IO_READ32(ad, reg_id, &mac_reg);
	mac_old = mac_reg;
	mac_reg = (enable ? (mac_reg | drop_mask) : (mac_reg & ~drop_mask));
	if (mac_reg != mac_old)
		RTMP_IO_WRITE32(ad, reg_id, mac_reg);

	NdisReleaseSpinLock(&ad->drop_mask_lock);

	DBGPRINT(RT_DEBUG_TRACE,
			("%s(%u):, wcid = %u, reg_id = 0x%08x, mac_reg = 0x%08x, group_index = %u, drop_mask = 0x%08x\n",
			__FUNCTION__, enable, wcid, reg_id, mac_reg, group_index, drop_mask));
}


VOID asic_drop_mask_reset(
	PRTMP_ADAPTER ad)
{
	UINT32 i, reg_id;

	NdisAcquireSpinLock(&ad->drop_mask_lock);

	for ( i = 0; i < 8 /* num of drop mask group */; i++)
	{
		reg_id = (TX_WCID_DROP_MASK0 + i*4);
		RTMP_IO_WRITE32(ad, reg_id, 0);
	}

	NdisReleaseSpinLock(&ad->drop_mask_lock);

	DBGPRINT(RT_DEBUG_TRACE, ("%s()\n", __FUNCTION__));
}
#endif /* DROP_MASK_SUPPORT */

#ifdef MULTI_CLIENT_SUPPORT
VOID asic_change_tx_retry(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{
	UINT32	TxRtyCfg, MacReg = 0;

	if (num < 3)
	{
		/* Tx data retry 31/15 (thres 2000) */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg);
		TxRtyCfg &= 0xf0000000;
		TxRtyCfg |= 0x07d01f0f;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry default 32, disable RTS fallback */
		RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x20;
		RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
	}
	else
	{
		/* Tx data retry 8/10 (thres 256)  */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg);
		TxRtyCfg &= 0xf0000000;
		TxRtyCfg |= 0x0100080A;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry 3, enable RTS fallback */
		RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x01000003;
		RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
	}
}

VOID pkt_aggr_num_change(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{
#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		if (num < 5)
		{
			/* use default */
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M1S, 0x77777777);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M2S, 0x77777777);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M1S, 0x77777777);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M2S, 0x77777777);
		}
		else
		{
			/* modify by MCS */
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M1S, 0x77754433);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M2S, 0x77765543);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M1S, 0x77765544);
			RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M2S, 0x77765544);
		}
	}
#endif
}

VOID asic_tune_be_wmm(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{
	UCHAR  bssCwmin = 4, apCwmin = 4, apCwmax = 6;

	if (num <= 4)
	{
		/* use profile cwmin */
		if (pAd->CommonCfg.APCwmin > 0 && pAd->CommonCfg.BSSCwmin > 0 && pAd->CommonCfg.APCwmax > 0)
		{
			apCwmin = pAd->CommonCfg.APCwmin;
			apCwmax = pAd->CommonCfg.APCwmax;
			bssCwmin = pAd->CommonCfg.BSSCwmin;
		}
	}
	else if (num > 4 && num <= 8)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 5;
	}
	else if (num > 8 && num <= 16)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 6;
	}
	else if (num > 16 && num <= 64)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 7;
	}
	else if (num > 64 && num <= 128)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 8;
	}

	pAd->CommonCfg.APEdcaParm.Cwmin[0] = apCwmin;
	pAd->CommonCfg.APEdcaParm.Cwmax[0] = apCwmax;
	pAd->ApCfg.BssEdcaParm.Cwmin[0] = bssCwmin;

	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
}
#endif /* MULTI_CLIENT_SUPPORT */

