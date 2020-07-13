/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


#ifndef COMPOS_WIN

#include "rt_config.h"

#ifdef NEW_RATE_ADAPT_SUPPORT

INT	Set_PerThrdAdj_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	UCHAR i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
		pAd->MacTab.Content[i].perThrdAdj = (BOOLEAN)simple_strtol(arg, 0, 10);

	return TRUE;
}


/* Set_LowTrafficThrd_Proc - set threshold for reverting to default MCS based on RSSI */
INT	Set_LowTrafficThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.lowTrafficThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpRule_Proc - set rule for Quick DRS train up */
INT	Set_TrainUpRule_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpRule = (BOOLEAN)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpRuleRSSI_Proc - set RSSI threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpRuleRSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpRuleRSSI = (SHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpLowThrd_Proc - set low threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpLowThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpLowThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Set_TrainUpHighThrd_Proc - set high threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpHighThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->CommonCfg.TrainUpHighThrd = (USHORT)simple_strtol(arg, 0, 10);
	return TRUE;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_alg;

	ra_alg = simple_strtol(arg, 0, 10);

	if ((ra_alg < RATE_ALG_MAX_NUM) && (ra_alg != pAd->rateAlg)) {
		UINT32 IdEntry;

		pAd->rateAlg = ra_alg;

		for (IdEntry = 0; VALID_UCAST_ENTRY_WCID(pAd, IdEntry); IdEntry++)
			pAd->MacTab.Content[IdEntry].rateAlg = ra_alg;
	}

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set Alg = %d\n", __func__, ra_alg));
	return TRUE;
}


#ifdef DBG
#ifdef MT_MAC
INT Set_Fixed_Rate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	RA_PHY_CFG_T TxPhyCfg;
	UINT_32 rate[8];
	UINT32 ret;
	INT32 i4Recv = 0;
	UINT32 u4WCID = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4VhtNss = 0;
	UINT32 u4SGI = 0, u4Preamble = 0, u4STBC = 0, u4LDPC = 0, u4SpeEn = 0;
	UCHAR ucNsts;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(u4WCID),
							&(u4Mode), &(u4Bw), &(u4Mcs), &(u4VhtNss),
							&(u4SGI), &(u4Preamble), &(u4STBC), &(u4LDPC), &(u4SpeEn));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID = %d, Mode = %d, BW = %d, MCS = %d, VhtNss = %d\n"
					 "\t\t\t\tSGI = %d, Preamble = %d, STBC = %d, LDPC = %d, SpeEn = %d\n",
					 __func__, u4WCID, u4Mode, u4Bw, u4Mcs, u4VhtNss,
					 u4SGI, u4Preamble, u4STBC, u4LDPC, u4SpeEn));

			if (i4Recv != 10) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
				fgStatus = FALSE;
				break;
			}

			if (!VALID_UCAST_ENTRY_WCID(pAd, u4WCID)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WCID exceed pAd->MaxUcastEntryNum!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Mode > MODE_VHT) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Mode!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Bw > 4) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow BW!\n"));
				fgStatus = FALSE;
				break;
			}

			if (((u4Mode == MODE_CCK) && (u4Mcs > 3)) ||
				((u4Mode == MODE_OFDM) && (u4Mcs > 7)) ||
				((u4Mode == MODE_HTMIX) && (u4Mcs > 32)) ||
				((u4Mode == MODE_VHT) && (u4Mcs > 9))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow MCS!\n"));
				fgStatus = FALSE;
				break;
			}

			if ((u4Mode == MODE_VHT) && (u4VhtNss > 4)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow VhtNss!\n"));
				fgStatus = FALSE;
				break;
			}

			RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);
			pEntry = &pAd->MacTab.Content[u4WCID];

			if (IS_ENTRY_NONE(pEntry)) {
				RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
				break;
			}

			os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
			TxPhyCfg.BW = u4Bw;
			TxPhyCfg.ShortGI = u4SGI;

			if (u4LDPC)
				TxPhyCfg.ldpc = HT_LDPC | VHT_LDPC;
			else
				TxPhyCfg.ldpc = 0;

			if (u4Preamble == 0)
				u4Preamble = LONG_PREAMBLE;
			else
				u4Preamble = SHORT_PREAMBLE;

			u4STBC = raStbcSettingCheck(u4STBC, u4Mode, u4Mcs, u4VhtNss, 0, 0);
			pEntry->HTPhyMode.field.MODE = u4Mode;
			pEntry->HTPhyMode.field.iTxBF = 0;
			pEntry->HTPhyMode.field.eTxBF = 0;
			pEntry->HTPhyMode.field.STBC = u4STBC ? 1 : 0;
			pEntry->HTPhyMode.field.ShortGI = u4SGI ? 1 : 0;
			pEntry->HTPhyMode.field.BW = u4Bw;
			pEntry->HTPhyMode.field.ldpc = u4LDPC ? 1 : 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

			if (u4Mode == MODE_VHT)
				pEntry->HTPhyMode.field.MCS = (((u4VhtNss - 1) & 0x3) << 4) + u4Mcs;
			else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			{
				pEntry->HTPhyMode.field.MCS = u4Mcs;
			}

			pEntry->LastTxRate = pEntry->HTPhyMode.word;
			pAd->LastTxRate = pEntry->HTPhyMode.word;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				pEntry->bAutoTxRateSwitch = FALSE;
				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.FixedRateCfg.MODE = u4Mode;
				rRaParam.FixedRateCfg.STBC = u4STBC;
				rRaParam.FixedRateCfg.ShortGI = u4SGI;
				rRaParam.FixedRateCfg.BW = u4Bw;
				rRaParam.FixedRateCfg.ldpc = TxPhyCfg.ldpc;
				rRaParam.FixedRateCfg.MCS = u4Mcs;
				rRaParam.FixedRateCfg.VhtNss = u4VhtNss;
				rRaParam.ucShortPreamble = u4Preamble;
				rRaParam.ucSpeEn = u4SpeEn;
				rRaParam.u4Field = RA_PARAM_FIXED_RATE;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					INT	apidx = 0;

					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_AP_SUPPORT */
				ucNsts = get_nsts_by_mcs(u4Mode, u4Mcs, u4STBC, u4VhtNss);
				rate[0] = tx_rate_to_tmi_rate(u4Mode,
											  u4Mcs,
											  ucNsts,
											  u4STBC,
											  u4Preamble);
				rate[0] &= 0xfff;
				rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
				MtAsicTxCapAndRateTableUpdate(pAd, u4WCID, &TxPhyCfg, rate, u4SpeEn);
			}

			RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set FixedRate=[WCID]-[Mode]-[BW]-[MCS]-[VhtNss]-[SGI]-[Preamble]-[STBC]-[LDPC]-[SPE_EN]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[WCID]Wireless Client ID\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[BW]BW20=0, BW40=1, BW80=2,BW160=3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[VhtNss]VHT=1~4, Other=ignore\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Preamble]Long=0, Other=Short\n"));
	} else
		dump_wtbl_info(pAd, u4WCID);

	return fgStatus;
}


INT Set_Fixed_Rate_With_FallBack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	RA_PHY_CFG_T TxPhyCfg;
	UINT_32 rate[8];
	UINT32 ret;
	INT32 i4Recv = 0;
	UINT32 u4WCID = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4VhtNss = 0;
	UINT32 u4SGI = 0, u4Preamble = 0, u4STBC = 0, u4LDPC = 0, u4SpeEn = 0, u4Is5G = 0;
	UCHAR ucNsts;
	MAC_TABLE_ENTRY *pEntry = NULL;
#if defined(RATE_ADAPT_AGBS_SUPPORT) && !defined(RACTRL_FW_OFFLOAD_SUPPORT)
	UINT_32 u4TableSize;
	UINT_16 *pu2FallbackTable = NULL;
	UINT_8 ucIndex;
	BOOL fgFound = FALSE;
	extern UINT_16 HwFallbackTable11B[32];
	extern UINT_16 HwFallbackTable11G[64];
	extern UINT_16 HwFallbackTable11BG[56];
	extern UINT_16 HwFallbackTable11N1SS[80];
	extern UINT_16 HwFallbackTable11N2SS[80];
	extern UINT_16 HwFallbackTable11N3SS[80];
	extern UINT_16 HwFallbackTable11N4SS[80];
	extern UINT_16 HwFallbackTableBGN1SS[80];
	extern UINT_16 HwFallbackTableBGN2SS[80];
	extern UINT_16 HwFallbackTableBGN3SS[80];
	extern UINT_16 HwFallbackTableBGN4SS[80];
	extern UINT_16 HwFallbackTableVht1SS[80];
	extern UINT_16 HwFallbackTableVht2SS[80];
	extern UINT_16 HwFallbackTableVht3SS[80];
	extern UINT_16 HwFallbackTableVht4SS[80];
#endif /* RATE_ADAPT_AGBS_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(u4WCID),
							&(u4Mode), &(u4Bw), &(u4Mcs), &(u4VhtNss),
							&(u4SGI), &(u4Preamble), &(u4STBC), &(u4LDPC), &(u4SpeEn), &(u4Is5G));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID = %d, Mode = %d, BW = %d, MCS = %d, VhtNss = %d\n"
					 "\t\t\t\tSGI = %d, Preamble = %d, STBC = %d, LDPC = %d, SpeEn = %d, Is5G = %d\n",
					 __func__, u4WCID, u4Mode, u4Bw, u4Mcs, u4VhtNss,
					 u4SGI, u4Preamble, u4STBC, u4LDPC, u4SpeEn, u4Is5G));

			if (i4Recv != 11) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
				fgStatus = FALSE;
				break;
			}

			if (!VALID_UCAST_ENTRY_WCID(pAd, u4WCID)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WCID exceed pAd->MaxUcastEntryNum!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Mode > MODE_VHT) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Mode!\n"));
				fgStatus = FALSE;
				break;
			}

			if (u4Bw > 4) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow BW!\n"));
				fgStatus = FALSE;
				break;
			}

			if (((u4Mode == MODE_CCK) && (u4Mcs > 3)) ||
				((u4Mode == MODE_OFDM) && (u4Mcs > 7)) ||
				((u4Mode == MODE_HTMIX) && (u4Mcs > 32)) ||
				((u4Mode == MODE_VHT) && (u4Mcs > 9))) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow MCS!\n"));
				fgStatus = FALSE;
				break;
			}

			if ((u4Mode == MODE_VHT) && (u4VhtNss > 4)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow VhtNss!\n"));
				fgStatus = FALSE;
				break;
			}

			RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);
			pEntry = &pAd->MacTab.Content[u4WCID];

			if (IS_ENTRY_NONE(pEntry)) {
				RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
				break;
			}

			os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
			TxPhyCfg.BW = u4Bw;
			TxPhyCfg.ShortGI = u4SGI;

			if (u4LDPC)
				TxPhyCfg.ldpc = HT_LDPC | VHT_LDPC;
			else
				TxPhyCfg.ldpc = 0;

			if (u4Preamble == 0)
				u4Preamble = LONG_PREAMBLE;
			else
				u4Preamble = SHORT_PREAMBLE;

			u4STBC = raStbcSettingCheck(u4STBC, u4Mode, u4Mcs, u4VhtNss, 0, 0);
			pEntry->HTPhyMode.field.MODE = u4Mode;
			pEntry->HTPhyMode.field.iTxBF = 0;
			pEntry->HTPhyMode.field.eTxBF = 0;
			pEntry->HTPhyMode.field.STBC = u4STBC ? 1 : 0;
			pEntry->HTPhyMode.field.ShortGI = u4SGI ? 1 : 0;
			pEntry->HTPhyMode.field.BW = u4Bw;
			pEntry->HTPhyMode.field.ldpc = u4LDPC ? 1 : 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

			if (u4Mode == MODE_VHT)
				pEntry->HTPhyMode.field.MCS = (((u4VhtNss - 1) & 0x3) << 4) + u4Mcs;
			else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			{
				pEntry->HTPhyMode.field.MCS = u4Mcs;
			}

			pEntry->LastTxRate = pEntry->HTPhyMode.word;
			pAd->LastTxRate = pEntry->HTPhyMode.word;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				pEntry->bAutoTxRateSwitch = FALSE;
				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.FixedRateCfg.MODE = u4Mode;
				rRaParam.FixedRateCfg.STBC = u4STBC;
				rRaParam.FixedRateCfg.ShortGI = u4SGI;
				rRaParam.FixedRateCfg.BW = u4Bw;
				rRaParam.FixedRateCfg.ldpc = TxPhyCfg.ldpc;
				rRaParam.FixedRateCfg.MCS = u4Mcs;
				rRaParam.FixedRateCfg.VhtNss = u4VhtNss;
				rRaParam.ucShortPreamble = u4Preamble;
				rRaParam.ucSpeEn = u4SpeEn;
				rRaParam.fgIs5G = u4Is5G ? TRUE : FALSE;
				rRaParam.u4Field = RA_PARAM_FIXED_RATE_FALLBACK;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					INT	apidx = 0;

					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch = FALSE;
				}
#endif /* CONFIG_AP_SUPPORT */
				ucNsts = get_nsts_by_mcs(u4Mode, u4Mcs, u4STBC, u4VhtNss);
				rate[0] = tx_rate_to_tmi_rate(u4Mode,
											  u4Mcs,
											  ucNsts,
											  u4STBC,
											  u4Preamble);
				rate[0] &= 0xfff;
#if defined(RATE_ADAPT_AGBS_SUPPORT) && !defined(RACTRL_FW_OFFLOAD_SUPPORT)

				if (u4Mode == MODE_CCK) {
					pu2FallbackTable = HwFallbackTable11B;
					u4TableSize = sizeof(HwFallbackTable11B) / 2;
				} else if (u4Mode == MODE_OFDM) {
					if (u4Is5G == TRUE) {
						pu2FallbackTable = HwFallbackTable11G;
						u4TableSize = sizeof(HwFallbackTable11G) / 2;
					} else {
						pu2FallbackTable = HwFallbackTable11BG;
						u4TableSize = sizeof(HwFallbackTable11BG) / 2;
					}
				} else if ((u4Mode == MODE_HTMIX) || (u4Mode == MODE_HTGREENFIELD)) {
					UINT_8 ucHtNss = 1;

					ucHtNss += (u4Mcs >> 3);

					if (u4Is5G == TRUE) {
						switch (ucHtNss) {
						case 1:
							pu2FallbackTable = HwFallbackTable11N1SS;
							u4TableSize = sizeof(HwFallbackTable11N1SS) / 2;
							break;

						case 2:
							pu2FallbackTable = HwFallbackTable11N2SS;
							u4TableSize = sizeof(HwFallbackTable11N2SS) / 2;
							break;

						case 3:
							pu2FallbackTable = HwFallbackTable11N3SS;
							u4TableSize = sizeof(HwFallbackTable11N3SS) / 2;
							break;

						case 4:
							pu2FallbackTable = HwFallbackTable11N4SS;
							u4TableSize = sizeof(HwFallbackTable11N4SS) / 2;
							break;

						default:
							MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
							break;
						}
					} else {
						switch (ucHtNss) {
						case 1:
							pu2FallbackTable = HwFallbackTableBGN1SS;
							u4TableSize = sizeof(HwFallbackTableBGN1SS) / 2;
							break;

						case 2:
							pu2FallbackTable = HwFallbackTableBGN2SS;
							u4TableSize = sizeof(HwFallbackTableBGN2SS) / 2;
							break;

						case 3:
							pu2FallbackTable = HwFallbackTableBGN3SS;
							u4TableSize = sizeof(HwFallbackTableBGN3SS) / 2;
							break;

						case 4:
							pu2FallbackTable = HwFallbackTableBGN4SS;
							u4TableSize = sizeof(HwFallbackTableBGN4SS) / 2;
							break;

						default:
							MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
							break;
						}
					}
				} else if (u4Mode == MODE_VHT) {
					switch (u4VhtNss) {
					case 1:
						pu2FallbackTable = HwFallbackTableVht1SS;
						u4TableSize = sizeof(HwFallbackTableVht1SS) / 2;
						break;

					case 2:
						pu2FallbackTable = HwFallbackTableVht2SS;
						u4TableSize = sizeof(HwFallbackTableVht2SS) / 2;
						break;

					case 3:
						{
							pu2FallbackTable = HwFallbackTableVht3SS;
							u4TableSize = sizeof(HwFallbackTableVht3SS) / 2;
						}

						break;

					case 4:
						pu2FallbackTable = HwFallbackTableVht4SS;
						u4TableSize = sizeof(HwFallbackTableVht4SS) / 2;
						break;

					default:
						MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", u4VhtNss));
						break;
					}
				}

				if (pu2FallbackTable != NULL) {
					for (ucIndex = 0; ucIndex < u4TableSize; ucIndex += 8) {
						union RA_RATE_CODE rInitialRate;

						rInitialRate.word = *(pu2FallbackTable + ucIndex);

						if (rInitialRate.field.mcs == u4Mcs) {
							fgFound = TRUE;
							break;
						}
					}

					if (fgFound) {
						UINT_8 ucIdx;
						union RA_RATE_CODE rRateCode;

						for (ucIdx = 1; ucIdx < 8; ucIdx++) {
							rRateCode.word = *(pu2FallbackTable + ucIndex + ucIdx);

							if (((u4Mode == MODE_HTMIX) || (u4Mode == MODE_VHT))
								&& u4STBC && (rRateCode.field.nsts == 0)) {
								rRateCode.field.nsts = 1;
								rRateCode.field.stbc = 1;
							}

							rate[ucIdx] = rRateCode.word;
						}
					}
				}

				if (!fgFound) {
					rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Cannot find fallback table!\n"));
				}

#else
				rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
#endif /* RATE_ADAPT_AGBS_SUPPORT */
				MtAsicTxCapAndRateTableUpdate(pAd, u4WCID, &TxPhyCfg, rate, u4SpeEn);
			}

			RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set FixedRateFallback=[WCID]-[Mode]-[BW]-[MCS]-[VhtNss]-[SGI]-[Preamble]-[STBC]-[LDPC]-[SPE_EN]-[is5G]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[WCID]Wireless Client ID\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[BW]BW20=0, BW40=1, BW80=2,BW160=3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[VhtNss]VHT=1~4, Other=ignore\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[Preamble]Long=0, Other=Short\n"));
	} else
		dump_wtbl_info(pAd, u4WCID);

	return fgStatus;
}

INT
Set_RA_Debug_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	UINT32 u4WlanIndex = 0, u4DebugType = 0;
	RTMP_STRING *pWlanIndex  = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: arg = %s\n", __func__, arg));
	pWlanIndex = strsep(&arg, ":");

	if (pWlanIndex == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	u4WlanIndex = os_str_toul(pWlanIndex, 0, 10);
	u4DebugType = os_str_toul(arg, 0, 10);

	if (!VALID_UCAST_ENTRY_WCID(pAd, u4WlanIndex)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("u4WlanIndex exceed pAd->MaxUcastEntryNum!\n"));
		return FALSE;
	}
	pEntry = &pAd->MacTab.Content[u4WlanIndex];

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

		if (u4DebugType < RA_PARAM_MAX)
			return FALSE;

		NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

		rRaParam.u4Field = u4DebugType;
		RAParamUpdate(pAd, pEntry, &rRaParam);
	} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	{
		return FALSE;
	}

	return TRUE;
}

#endif /* MT_MAC */
#endif /* DBG */

#endif /* COMPOS_WIN */

