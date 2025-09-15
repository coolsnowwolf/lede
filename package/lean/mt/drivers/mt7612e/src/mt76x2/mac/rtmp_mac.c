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

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


/*
	========================================================================
	
	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
					  
	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003
		
	Return Value:
		None
	
	See also : BASmartHardTransmit()    !!!
	
	========================================================================
*/
VOID RTMPWriteTxWI(
	IN RTMP_ADAPTER *pAd,
	IN TXWI_STRUC *pOutTxWI,
	IN BOOLEAN FRAG,
	IN BOOLEAN CFACK,
	IN BOOLEAN InsTimestamp,
	IN BOOLEAN AMPDU,
	IN BOOLEAN Ack,
	IN BOOLEAN NSeq,		/* HW new a sequence.*/
	IN UCHAR BASize,
	IN UCHAR WCID,
	IN ULONG Length,
	IN UCHAR PID,
	IN UCHAR TID,
	IN UCHAR TxRate,
	IN UCHAR Txopmode,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	PMAC_TABLE_ENTRY pMac = NULL;
	TXWI_STRUC TxWI, *pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT TxEAPId_Cal = 0;
	UCHAR stbc, bw, mcs, sgi, phy_mode, mpdu_density = 0, mimops = 0, ldpc = 0;
	UCHAR tx_stream_mode = 0;
#ifdef TXBF_SUPPORT
	UCHAR eTxBf, iTxBf, sounding, ndp_rate;
#endif /* TXBF_SUPPORT */

	if (WCID < MAX_LEN_OF_MAC_TABLE)
		pMac = &pAd->MacTab.Content[WCID];

	/* 
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	//OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;

#ifdef DOT11_N_SUPPORT
	BASize = 0;
	stbc = pTransmit->field.STBC;
#endif /* DOT11_N_SUPPORT */
	
#ifdef VHT_TXBF_SUPPORT	
	/* If CCK or OFDM, BW must be 20*/
        if (TxRate == SNDG_TYPE_NDP)
        {
	    bw = pTransmit->field.BW;
        }
        else
#endif		
        {
	    bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
        }

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	sgi = pTransmit->field.ShortGI;
	
#ifdef MT76x2
#ifdef THERMAL_PROTECT_SUPPORT
	if (IS_MT76x2(pAd) && pAd->force_one_tx_stream == TRUE) 
	{
		tx_stream_mode = 0; /* SKIP 1SS2T when 1T Status on */
	}
	else
#endif /* THERMAL_PROTECT_SUPPORT */
	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;
#endif /* MT76x2 */
	
#ifdef DOT11_N_SUPPORT
	if (pMac)
	{
		if (pAd->CommonCfg.bMIMOPSEnable)
		{
			UCHAR MaxMcs_1ss;
	
			/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
			A STA in static SM power save mode maintains only a single receive chain active.
	
			An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
			STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
			the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
			*/
#ifdef DOT11_VHT_AC
			if (IS_VHT_STA(pMac))
				MaxMcs_1ss = 9;
			else
#endif /* DOT11_VHT_AC */
				MaxMcs_1ss = 7; 

			if ((pMac->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > MaxMcs_1ss))
			{
				/* Dynamic MIMO Power Save Mode*/
				mimops = 1;
			}
			else if (pMac->MmpsMode == MMPS_STATIC)
			{
				/* Static MIMO Power Save Mode*/
				if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > MaxMcs_1ss)
				{
					mcs = MaxMcs_1ss;
					mimops = 0;
				}
			}
		}

		mpdu_density = pMac->MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		AMPDU = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef TXBF_SUPPORT
	eTxBf = pTransmit->field.eTxBF;
	iTxBf = pTransmit->field.iTxBF;

	// Calculate the partial AID
	TxEAPId_Cal = WCID + (UINT)((pAd->CommonCfg.Bssid[5] >> 4) ^ (pAd->CommonCfg.Bssid[5] & 0x0F))*32;
	TxEAPId_Cal -= ((TxEAPId_Cal >> 9) << 9);

	if (pMac && pAd->chipCap.FlgHwTxBfCap)
	{
		if (pMac->TxSndgType == SNDG_TYPE_NDP  || pMac->TxSndgType == SNDG_TYPE_SOUNDING)
		{
			stbc  = FALSE;
			eTxBf = FALSE;
			iTxBf = FALSE;
			sounding = FALSE;
			ndp_rate = 1;
			//phy_mode = 1;
			//mcs = MCS_RATE_24;
			
			DBGPRINT(RT_DEBUG_TRACE, ("%s : NDP Sounding and NDPSndRate = %d\n", __FUNCTION__, ndp_rate));
			DBGPRINT(RT_DEBUG_TRACE, ("%s : BSSID[5] = %x, txwi_n->TxEAPId = %d\n", __FUNCTION__, pAd->CommonCfg.Bssid[5], TxEAPId_Cal));
		}
	}

	if (iTxBf || eTxBf) stbc = 0; // Force STBC = 0 when TxBf is enabled
	
#endif /* TXBF_SUPPORT */


#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;
		txwi_n->FRAG = FRAG;
		txwi_n->CFACK= CFACK;
		txwi_n->TS = InsTimestamp;
		txwi_n->AMPDU = AMPDU;
		txwi_n->ACK = Ack;
		txwi_n->txop = Txopmode;
		txwi_n->NSEQ = NSeq;
		txwi_n->BAWinSize = BASize;
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;
#ifdef CONFIG_AP_SUPPORT
		if((phy_mode == MODE_CCK) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)) 
		   )
		{
			if (mcs == MCS_LONGP_RATE_2)
				txwi_n->MCS = MCS_SHORTP_RATE_2;
			else if (mcs == MCS_LONGP_RATE_5_5)
				txwi_n->MCS = MCS_SHORTP_RATE_5_5;
			else if (mcs == MCS_LONGP_RATE_11)
				txwi_n->MCS = MCS_SHORTP_RATE_11;
		}
		else 
#endif /* CONFIG_AP_SUPPORT */
		txwi_n->MCS = mcs;

		txwi_n->BW = bw;
		txwi_n->PHYMODE= phy_mode;
		txwi_n->MpduDensity = mpdu_density;
		txwi_n->MIMOps = mimops;
		txwi_n->wcid = WCID;
		txwi_n->MPDUtotalByteCnt = Length;
		txwi_n->TxPktId = mcs; // PID is not used now!

#ifdef CONFIG_AP_SUPPORT
		txwi_n->GroupID = TRUE;
		txwi_n->TxEAPId = TxEAPId_Cal;
#endif /*CONFIG_AP_SUPPORT*/		

#ifdef CONFIG_STA_SUPPORT
		txwi_n->GroupID = FALSE;
		txwi_n->TxEAPId = pAd->CommonCfg.Bssid[5];
#endif /*CONFIG_STA_SUPPORT*/				

		txwi_n->TxStreamMode = tx_stream_mode;
#ifdef TXBF_SUPPORT
		txwi_n->Sounding = sounding;
		txwi_n->eTxBF = eTxBf;
		txwi_n->iTxBF = iTxBf;
		txwi_n->NDPSndRate = ndp_rate;
		txwi_n->NDPSndBW = bw;
		txwi_n->TXBF_PT_SCA = (eTxBf | iTxBf) ? TRUE : FALSE;
#endif /* TXBF_SUPPORT */

		/* Calculate TxPwrAdj */
		txwi_n->TxPwrAdj = 0;
		
#ifdef SPECIFIC_TX_POWER_SUPPORT
		if (pMac)
		{
			txwi_n->TxPwrAdj += pAd->ApCfg.MBSSID[pMac->apidx].TxPwrAdj;
			DBGPRINT(RT_DEBUG_TRACE, ("%s :%d pMac->apidx = %d, txwi_n->TxPwrAdj = %d\n"
				, __FUNCTION__,__LINE__,pMac->apidx, txwi_n->TxPwrAdj));
		}
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef SINGLE_SKU_V2	
		RTMP_CHIP_ASIC_SKU_TX_POWER_ADJUST(pAd,txwi_n);	
#endif /* SINGLE_SKU_V2 */

		/* increase low rate Tx Power by TXWI */
		RTMP_CHIP_ASIC_TX_POWER_BOOST(pAd,txwi_n);
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		txwi_o->FRAG = FRAG;
		txwi_o->CFACK = CFACK;
		txwi_o->TS = InsTimestamp;
		txwi_o->AMPDU = AMPDU;
		txwi_o->ACK = Ack;
		txwi_o->txop = Txopmode;
		txwi_o->NSEQ = NSeq;
		txwi_o->BAWinSize = BASize;
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
#ifdef CONFIG_AP_SUPPORT
		if((phy_mode == MODE_CCK) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)) 
		   )
		{
			if (mcs == MCS_LONGP_RATE_2)
				txwi_o->MCS = MCS_SHORTP_RATE_2;
			else if (mcs == MCS_LONGP_RATE_5_5)
				txwi_o->MCS = MCS_SHORTP_RATE_5_5;
			else if (mcs == MCS_LONGP_RATE_11)
				txwi_o->MCS = MCS_SHORTP_RATE_11;
		}
		else 
#endif /* CONFIG_AP_SUPPORT */
		txwi_o->MCS = mcs;
		txwi_o->BW = bw;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->MpduDensity = mpdu_density;
		txwi_o->MIMOps = mimops;
		txwi_o->wcid = WCID;
		txwi_o->MPDUtotalByteCnt = Length;
		txwi_o->PacketId = mcs; // PID is not used now!
	}
#endif /* RTMP_MAC */

	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);
#ifdef DBG
//+++Add by shiang for debug
if (0){
	hex_dump("TxWI", (UCHAR *)pOutTxWI, TXWISize);
	dump_txwi(pAd, pOutTxWI);
}
//---Add by shiang for debug
#endif
}


VOID RTMPWriteTxWI_Data(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit;
	MAC_TABLE_ENTRY *pMacEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR wcid, pkt_id;
	UCHAR sgi, mcs, bw, stbc, phy_mode, ldpc;
#ifdef DOT11_N_SUPPORT
	UCHAR basize, ampdu, mimops = 0, mpdu_density = 0;
#endif /* DOT11_N_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR iTxBf, eTxBf, sounding, ndp_rate, ndp_bw;
#endif /* TXBF_SUPPORT */
#ifdef MCS_LUT_SUPPORT
	BOOLEAN lut_enable = 0;
	UCHAR mbc_wcid;
#endif /* MCS_LUT_SUPPORT */
	UCHAR tx_stream_mode = 0;


	ASSERT(pTxWI);

	pTransmit = pTxBlk->pTransmit;
	pMacEntry = pTxBlk->pMacEntry;

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	//OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(pTxWI, TXWISize);

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
	if (pMacEntry && IS_ENTRY_DLS(pMacEntry) &&
		(pAd->StaCfg.BssType == BSS_INFRA))
		wcid = BSSID_WCID;
	else
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		wcid = pTxBlk->Wcid;

	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	/* If CCK or OFDM, BW must be 20 */
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

#ifdef MT76x2
#ifdef THERMAL_PROTECT_SUPPORT
        if (IS_MT76x2(pAd) && pAd->force_one_tx_stream == TRUE)
        {
                tx_stream_mode = 0;
        }
        else
#endif /* THERMAL_PROTECT_SUPPORT */
	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;
#endif /* MT76x2 */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

	ampdu = ((pTxBlk->TxFrameType == TX_AMPDU_FRAME) ? TRUE : FALSE);
	basize = pAd->CommonCfg.TxBASize;
	if(ampdu && pMacEntry)
	{
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pAd->Antenna.field.TxPath == 2)
			&& (pMacEntry->bIAmBadAtheros) && (pMacEntry->WepStatus == Ndis802_11EncryptionDisabled))
		{
			basize = 7;
		}
		else 
		{
			UCHAR RABAOriIdx = pTxBlk->pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
			basize = pAd->BATable.BAOriEntry[RABAOriIdx].BAWinSize;
		}
	}

#ifdef TXBF_SUPPORT
	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		ampdu = FALSE;

	sounding = ndp_bw = ndp_rate = iTxBf = eTxBf = 0;
	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING)
	{
		sounding = 1;
		iTxBf = FALSE;
		eTxBf = FALSE;
		stbc = FALSE;
        
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): sending normal sounding, eTxBF=%d\n",
					__FUNCTION__, pTransmit->field.eTxBF));
		iTxBf = 0;
	}
	else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP)
	{
		ndp_bw = pTransmit->field.BW;
		iTxBf = FALSE;
		eTxBf = FALSE;
		stbc = FALSE;
		if (pTxBlk->TxNDPSndgMcs >= 16)
			ndp_rate = 2;
		else if (pTxBlk->TxNDPSndgMcs >= 8)
			ndp_rate = 1;
		else
			ndp_rate = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): NDP sounding, eTxBF=%d, ndp_rate = %d\n",
					__FUNCTION__, eTxBf, ndp_rate));
	}
	else
	{
#ifdef MFB_SUPPORT
		if (pMacEntry && (pMacEntry->mrqCnt >0) && (pMacEntry->toTxMrq == TRUE))
			eTxBf = ~(pTransmit->field.eTxBF);
		else
#endif	/* MFB_SUPPORT */
			eTxBf = pTransmit->field.eTxBF;
		iTxBf = pTransmit->field.iTxBF;
	}

	if (iTxBf || eTxBf) stbc = FALSE; // Force STBC = FALSE when TxBf is enabled
	
#endif /* TXBF_SUPPORT */

#endif /* DOT11_N_SUPPORT */
	
	
#ifdef TXBF_SUPPORT
	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
	{
		mcs = 0;
		ampdu = FALSE;
	}
#endif /* TXBF_SUPPORT */

#ifdef DOT11_N_SUPPORT
	if (pMacEntry)
	{
		UCHAR MaxMcs_1ss;

		/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
		A STA in static SM power save mode maintains only a single receive chain active.

		An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
		STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
		the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
		*/
#ifdef DOT11_VHT_AC
		if (IS_VHT_STA(pMacEntry))
			MaxMcs_1ss = 9;
		else
#endif /* DOT11_VHT_AC */
			MaxMcs_1ss = 7; 

		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (mcs > MaxMcs_1ss))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			if ((pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) && 
				(mcs > MaxMcs_1ss))
			{
				mcs = MaxMcs_1ss;
				mimops = 0;
			}
		}

		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pMacEntry->bIAmBadAtheros))
			mpdu_density = 7;
		else
			mpdu_density = pMacEntry->MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	/* for rate adapation*/
	pkt_id = mcs;

#ifdef INF_AMAZON_SE
	/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
	if( RTMP_GET_PACKET_NOBULKOUT(pTxBlk->pPacket))
	{
		if(phy_mode == MODE_CCK)
			pkt_id = 6;
	}	
#endif /* INF_AMAZON_SE */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		ampdu = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x6)
	{
		phy_mode = pAd->fpga_ctl.tx_data_phy;
		mcs = pAd->fpga_ctl.tx_data_mcs;
		ldpc = pAd->fpga_ctl.tx_data_ldpc;
		bw = pAd->fpga_ctl.tx_data_bw;
		sgi = pAd->fpga_ctl.tx_data_gi;
		if (pAd->fpga_ctl.data_basize)
			basize = pAd->fpga_ctl.data_basize;
	}
#endif /* CONFIG_FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	if ((RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT)) && 
		(wcid < 128) && 
		(pMacEntry && pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;

		rate_ctrl.field.MODE = phy_mode;
#ifdef TXBF_SUPPORT
		rate_ctrl.field.iTxBF = iTxBf;
		rate_ctrl.field.eTxBF = eTxBf;
#endif /* TXBF_SUPPORT */
		rate_ctrl.field.STBC = stbc;
		rate_ctrl.field.ShortGI = sgi;
		rate_ctrl.field.BW = bw;
		rate_ctrl.field.ldpc = ldpc;
		rate_ctrl.field.MCS = mcs; 
		if (rate_ctrl.word == pTransmit->word)
			lut_enable = TRUE;
	}
#ifdef PEER_DELBA_TX_ADAPT
	GET_GroupKey_WCID(pAd, mbc_wcid, pTxBlk->apidx);
	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket) || (wcid == mbc_wcid) || 
		(pMacEntry && (pMacEntry->MmpsMode == MMPS_STATIC)))
		lut_enable = FALSE;
	else
		lut_enable = TRUE;
#endif /* PEER_DELBA_TX_ADAPT */
#endif /* MCS_LUT_SUPPORT */

#ifdef TXBF_SUPPORT
#ifdef MCS_LUT_SUPPORT
	if (pMacEntry && pAd->chipCap.FlgHwTxBfCap)
	{
		if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP  || pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING)
			lut_enable = FALSE;
	}
#endif /* MCS_LUT_SUPPORT */
#endif /* TXBF_SUPPORT */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;

		txwi_n->FRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
		txwi_n->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
		if (RTMP_GET_PACKET_TDLS_WAIT_ACK(pTxBlk->pPacket))
		{			
			txwi_n->TxPktId |= 0x80;
			DBGPRINT(RT_DEBUG_INFO,("PktID |= 0x80 : [%x]\n",txwi_n->TxPktId));
		}
		else
		{		
			txwi_n->TxPktId &= 0x7f;
			DBGPRINT(RT_DEBUG_INFO,("PktID : [%x]\n",txwi_n->TxPktId));
		}
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_n->ACK = 0;
#endif /* WFA_VHT_PF */

#ifdef TXBF_SUPPORT
		if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
			txwi_n->txop = IFS_BACKOFF; // Reserve larger TXOP to prevent sounding packet from collision
		else
#endif /* WFA_VHT_PF */
			txwi_n->txop = pTxBlk->FrameGap;
		txwi_n->wcid = wcid;
		txwi_n->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_n->CFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;
		txwi_n->TxStreamMode = tx_stream_mode;
		
#ifdef CONFIG_AP_SUPPORT
		if((phy_mode == MODE_CCK) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)) 
		   )
		{
			if (mcs == MCS_LONGP_RATE_2)
				txwi_n->MCS = MCS_SHORTP_RATE_2;
			else if (mcs == MCS_LONGP_RATE_5_5)
				txwi_n->MCS = MCS_SHORTP_RATE_5_5;
			else if (mcs == MCS_LONGP_RATE_11)
				txwi_n->MCS = MCS_SHORTP_RATE_11;
		}
		else 
#endif /* CONFIG_AP_SUPPORT */
		txwi_n->MCS = mcs;
		txwi_n->PHYMODE = phy_mode;
		txwi_n->BW = bw;
		txwi_n->TxPktId = pkt_id;
			
#ifdef DOT11_N_SUPPORT
		txwi_n->AMPDU = ampdu;
		txwi_n->BAWinSize = basize;
		txwi_n->MIMOps = mimops;
		txwi_n->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
		txwi_n->Sounding = sounding;
		txwi_n->iTxBF = iTxBf;
		txwi_n->eTxBF = eTxBf;
		txwi_n->NDPSndRate = ndp_rate;
		txwi_n->NDPSndBW = ndp_bw;
		txwi_n->TXBF_PT_SCA = (eTxBf | iTxBf) ? TRUE : FALSE;
#endif /* TXBF_SUPPORT */

#ifdef MCS_LUT_SUPPORT
		txwi_n->lut_en = lut_enable;
#endif /* MCS_LUT_SUPPORT */


		/* Calculate TxPwrAdj */
		txwi_n->TxPwrAdj = 0;

#ifdef SPECIFIC_TX_POWER_SUPPORT
		if (pMacEntry)
		{
			txwi_n->TxPwrAdj += pAd->ApCfg.MBSSID[pMacEntry->apidx].TxPwrAdj;
			DBGPRINT(RT_DEBUG_TRACE, ("%s :%d pMacEntry->apidx = %d, txwi_n->TxPwrAdj = %d\n"
				, __FUNCTION__,__LINE__,pMacEntry->apidx, txwi_n->TxPwrAdj));
		}
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef SINGLE_SKU_V2	
		RTMP_CHIP_ASIC_SKU_TX_POWER_ADJUST(pAd,txwi_n);	
#endif /* SINGLE_SKU_V2 */

		/* increase low rate Tx Power by TXWI */
		RTMP_CHIP_ASIC_TX_POWER_BOOST(pAd,txwi_n);

	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		
		txwi_o->FRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
		txwi_o->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_o->ACK = 0;
#endif /* WFA_VHT_PF */
		txwi_o->txop = pTxBlk->FrameGap;
		txwi_o->wcid = wcid;
		txwi_o->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_o->CFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
#ifdef CONFIG_AP_SUPPORT
		if((phy_mode == MODE_CCK) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)) 
		   )
		{
			if (mcs == MCS_LONGP_RATE_2)
				txwi_o->MCS = MCS_SHORTP_RATE_2;
			else if (mcs == MCS_LONGP_RATE_5_5)
				txwi_o->MCS = MCS_SHORTP_RATE_5_5;
			else if (mcs == MCS_LONGP_RATE_11)
				txwi_o->MCS = MCS_SHORTP_RATE_11;
		}
		else 
#endif /* CONFIG_AP_SUPPORT */
		txwi_o->MCS = mcs;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->BW = bw;
		txwi_o->PacketId = pkt_id;
			
#ifdef DOT11_N_SUPPORT
		txwi_o->AMPDU = ampdu;
		txwi_o->BAWinSize = basize;
		txwi_o->MIMOps = mimops;
		txwi_o->MpduDensity= mpdu_density;
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
		txwi_o->Sounding = sounding;
		txwi_o->iTxBF = iTxBf;
		txwi_o->eTxBF = eTxBf;
		txwi_o->NDPSndRate = ndp_rate;
		txwi_o->NDPSndBW = ndp_bw;
#endif /* TXBF_SUPPORT */
#ifdef MCS_LUT_SUPPORT
		txwi_o->TXLUT = lut_enable;
#endif /* MCS_LUT_SUPPORT */
	}
#endif /* RTMP_MAC */
}


VOID RTMPWriteTxWI_Cache(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR pkt_id;
	UCHAR bw, mcs, stbc, phy_mode, sgi, ldpc;
#ifdef DOT11_N_SUPPORT
	UCHAR ampdu, basize = 0, mimops, mpdu_density = 0;
#endif /* DOT11_N_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR sounding, iTxBf, eTxBf, ndp_rate, ndp_bw;
#endif /* TXBF_SUPPORT */
#ifdef MCS_LUT_SUPPORT
	BOOLEAN lut_enable;
#endif /* MCS_LUT_SUPPORT */
	UCHAR tx_stream_mode = 0;


	/* If CCK or OFDM, BW must be 20*/
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	pMacEntry->LastTxRate = pTransmit->word;

#ifdef MT76x2
#ifdef THERMAL_PROTECT_SUPPORT
        if (IS_MT76x2(pAd) && pAd->force_one_tx_stream == TRUE)
        {
                tx_stream_mode = 0;
        }
        else
#endif /* THERMAL_PROTECT_SUPPORT */
	if (MT_REV_GTE(pAd, MT76x2, REV_MT76x2E4))
		tx_stream_mode = 0x13;
	else if (MT_REV_ET(pAd, MT76x2, REV_MT76x2E3))
		tx_stream_mode = (pTransmit->field.MODE <= MODE_OFDM) ? 0x93 : 0x0;
#endif /* MT76x2 */

#ifdef DOT11_N_SUPPORT
	ampdu = ((pMacEntry->NoBADataCountDown == 0) ? TRUE: FALSE);
#ifdef TXBF_SUPPORT
	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		ampdu = FALSE;
#endif /* TXBF_SUPPORT */

#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

	mimops = 0;
	if (pAd->CommonCfg.bMIMOPSEnable)
	{
		UCHAR MaxMcs_1ss;

		/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
		A STA in static SM power save mode maintains only a single receive chain active.

		An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
		STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
		the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
		*/
#ifdef DOT11_VHT_AC
		if (IS_VHT_STA(pMacEntry))
			MaxMcs_1ss = 9;
		else
#endif /* DOT11_VHT_AC */
			MaxMcs_1ss = 7;			
			
		/* MIMO Power Save Mode*/
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > MaxMcs_1ss))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			if ((pTransmit->field.MODE >= MODE_HTMIX) && (pTransmit->field.MCS > MaxMcs_1ss))
			{
				mcs = MaxMcs_1ss;
				mimops = 0;
			}
		}
	}

	if(ampdu && pMacEntry)
	{
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pMacEntry->bIAmBadAtheros))
		{
			mpdu_density = 7;
			if ((pAd->Antenna.field.TxPath == 2) &&
				(pMacEntry->WepStatus == Ndis802_11EncryptionDisabled))
			{
				basize = 7;
			}
		}

	}
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

#ifdef TXBF_SUPPORT
	sounding = eTxBf = iTxBf = ndp_bw = ndp_rate = 0;
	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING)
	{
		sounding = 1;
		stbc = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): sending normal sounding, eTxBF=%d\n",
					__FUNCTION__, pTransmit->field.eTxBF));
	}
	else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP)
	{
		stbc = FALSE;
		if (pTxBlk->TxNDPSndgMcs>=16)
			ndp_rate = 2;
		else if (pTxBlk->TxNDPSndgMcs>=8)
			ndp_rate = 1;
		else
			ndp_rate = 0;
		ndp_bw = pTransmit->field.BW;

		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in %s(): NDP sounding, eTxBF=%d, ndp_rate = %d\n",
					__FUNCTION__, eTxBf, ndp_rate));
	}
	else
	{
#ifdef MFB_SUPPORT
		if (pMacEntry && pMacEntry->mrqCnt >0 && pMacEntry->toTxMrq == 1)
		{
			eTxBf = ~(pTransmit->field.eTxBF);
			DBGPRINT_RAW(RT_DEBUG_TRACE,("ETxBF in AP_AMPDU_Frame_Tx(): invert eTxBF\n"));
		}
		else
#endif	/* MFB_SUPPORT */
			eTxBf = pTransmit->field.eTxBF;

		iTxBf = pTransmit->field.iTxBF;
	}

	if (eTxBf || iTxBf) stbc = FALSE; //Force STBC = FALSE when TxBf is enabled

	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
	{
		mcs = 0;
		ampdu = FALSE;
	}
#endif /* TXBF_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		ampdu = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x6)
	{
		HTTRANSMIT_SETTING tmpTransmit;

		phy_mode = pAd->fpga_ctl.tx_data_phy;
		mcs = pAd->fpga_ctl.tx_data_mcs;
		ldpc = pAd->fpga_ctl.tx_data_ldpc;
		bw = pAd->fpga_ctl.tx_data_bw;
		sgi = pAd->fpga_ctl.tx_data_gi;
		stbc = pAd->fpga_ctl.tx_data_stbc;
		if (pAd->fpga_ctl.data_basize)
			basize = pAd->fpga_ctl.data_basize;
	
		tmpTransmit.field.BW = bw; 
		tmpTransmit.field.ShortGI = sgi;
		tmpTransmit.field.STBC = stbc;
		tmpTransmit.field.ldpc = ldpc;
		tmpTransmit.field.MCS = mcs;
		tmpTransmit.field.MODE = phy_mode;

		pMacEntry->LastTxRate = tmpTransmit.word;
	}
#endif /* CONFIG_FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	lut_enable = FALSE;
	if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) && 
		(pTxBlk->Wcid < 128) && 
		(pMacEntry && pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;
		
		rate_ctrl.field.MODE = phy_mode;
#ifdef TXBF_SUPPORT
		rate_ctrl.field.iTxBF = iTxBf;
		rate_ctrl.field.eTxBF = eTxBf;
#endif /* TXBF_SUPPORT */
		rate_ctrl.field.STBC = stbc;
		rate_ctrl.field.ShortGI = sgi;
		rate_ctrl.field.BW = bw;
		rate_ctrl.field.ldpc = ldpc;
		rate_ctrl.field.MCS = mcs; 
		if (rate_ctrl.word == pTransmit->word)
			lut_enable = TRUE;
	}
#endif /* MCS_LUT_SUPPORT */

		/* set PID for TxRateSwitching*/
		pkt_id = mcs;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;

#ifdef TXBF_SUPPORT
		if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
			txwi_n->txop = IFS_BACKOFF; // Reserve larger TXOP to prevent sounding packet from collision
		else
#endif			
		txwi_n->txop = IFS_HTTXOP;
		txwi_n->BW = bw;
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;
		txwi_n->TxStreamMode = tx_stream_mode;
		txwi_n->MCS = mcs;
		txwi_n->PHYMODE = phy_mode;
		txwi_n->TxPktId = pkt_id;
		txwi_n->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_n->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_n->ACK = 0;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		txwi_n->AMPDU = ampdu;
		if (basize)
			txwi_n->BAWinSize = basize;
		txwi_n->MIMOps = mimops;
		if (mpdu_density)
			txwi_n->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
		txwi_n->Sounding = sounding;
		txwi_n->eTxBF = eTxBf;
		txwi_n->iTxBF = iTxBf;
		txwi_n->NDPSndRate = ndp_rate;
		txwi_n->NDPSndBW = ndp_bw;
		txwi_n->TXBF_PT_SCA = (eTxBf | iTxBf) ? TRUE : FALSE;
#endif /* TXBF_SUPPORT */

		/* Calculate TxPwrAdj */
		txwi_n->TxPwrAdj = 0;

#ifdef SPECIFIC_TX_POWER_SUPPORT
		if (pMacEntry)
		{
			txwi_n->TxPwrAdj = pAd->ApCfg.MBSSID[pMacEntry->apidx].TxPwrAdj;
			DBGPRINT(RT_DEBUG_INFO, ("%s :%d pMacEntry->apidx = %d, txwi_n->TxPwrAdj = %d\n"
			, __FUNCTION__,__LINE__,pMacEntry->apidx, txwi_n->TxPwrAdj));
		}
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef SINGLE_SKU_V2	
		RTMP_CHIP_ASIC_SKU_TX_POWER_ADJUST(pAd,txwi_n);	
#endif /* SINGLE_SKU_V2 */

		/* increase low rate Tx Power by TXWI */
		RTMP_CHIP_ASIC_TX_POWER_BOOST(pAd,txwi_n);
		
#ifdef MCS_LUT_SUPPORT
		txwi_n->lut_en = lut_enable;
#endif /* MCS_LUT_SUPPORT */
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		
		txwi_o->txop = IFS_HTTXOP;
		txwi_o->BW = bw;
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
		txwi_o->MCS = mcs;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->PacketId = pkt_id;
		txwi_o->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_o->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_o->ACK = 0;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		txwi_o->AMPDU = ampdu;
		if (basize)
			txwi_o->BAWinSize = basize;
		txwi_o->MIMOps = mimops;
		if (mpdu_density)
			txwi_o->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
		txwi_o->Sounding = sounding;
		txwi_o->eTxBF = eTxBf;
		txwi_o->iTxBF = iTxBf;
		txwi_o->NDPSndRate = ndp_rate;
		txwi_o->NDPSndBW = ndp_bw;
#endif /* TXBF_SUPPORT */
#ifdef MCS_LUT_SUPPORT
		txwi_o->TXLUT = lut_enable;
#endif /* MCS_LUT_SUPPORT */
	}
#endif /* RTMP_MAC */
}


INT get_pkt_rssi_by_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *rxwi, INT size, CHAR *rssi)
{
	INT status = 0;
	
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		status = rlt_get_rxwi_rssi(rxwi, size, rssi);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		status = rtmp_get_rxwi_rssi(rxwi, size, rssi);
#endif /* RTMP_MAC */

	return status;
}


INT get_pkt_snr_by_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *rxwi, INT size, UCHAR *snr)
{
	INT status = 0;
	
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		status = rlt_get_rxwi_snr(pAd, rxwi, size, snr);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		status = rtmp_get_rxwi_snr(rxwi, size, snr);
#endif /* RTMP_MAC */

	return status;
}


INT get_pkt_phymode_by_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *rxwi)
{
	INT status = 0;
	
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		status = rlt_get_rxwi_phymode(rxwi);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		status = rtmp_get_rxwi_phymode(rxwi);
#endif /* RTMP_MAC */

	return status;

}

#ifdef MCS_LUT_SUPPORT
INT set_lut_phy_rate(
	RTMP_ADAPTER *pAd, UINT8 wcid,
	UINT8 mcs, UINT8 bw, 	UINT8 gi,
	UINT8 stbc, UINT8 mode)
{
	UINT32 mac_reg = 0;
	USHORT reg_id = 0x1C00 + (wcid << 3);
	
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		mac_reg = (mcs | (bw << 7) | (gi << 9) | (stbc << 10) | (mode << 13));
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		mac_reg = (mcs | (bw << 7) | (gi << 8) | (stbc << 9) | (mode << 14));
	}
#endif /* RTMP_MAC */

	if (mac_reg)
		RTMP_IO_WRITE32(pAd, reg_id, mac_reg);

	return TRUE;
}
#endif /* MCS_LUT_SUPPORT */

INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	UINT32 val, band_cfg;


	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x6);
	switch (band)
	{
		case BAND_5G:
			val |= 0x02;
			break;
		case BAND_24G:
		default:
			val |= 0x4;
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);

	return TRUE;
}


INT rtmp_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	UINT32 val, band_cfg;

	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x1);
	switch (extch)
	{
		case EXTCHA_ABOVE:
			val &= (~0x1);
			break;
		case EXTCHA_BELOW:
			val |= (0x1);
			break;
		case EXTCHA_NONE:
			val &= (~0x1);
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);
	
	return TRUE;
}


INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	UINT32 mac_val, org_val;

	RTMP_IO_READ32(pAd, 0x1210, &org_val);
	mac_val = org_val;
	if (ReduceCorePower)
		mac_val |= 0x09;
	else
		mac_val &= ~0x09;

	if (mac_val != org_val)
		RTMP_IO_WRITE32(pAd, 0x1210, mac_val);

	return TRUE;
}


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	UINT32  Offset;
	BCN_TIME_CFG_STRUC csr;
		
	Offset = (pAd->TbttTickCount) % (BCN_TBTT_OFFSET);
	pAd->TbttTickCount++;

	/*
		The updated BeaconInterval Value will affect Beacon Interval after two TBTT
		beacasue the original BeaconInterval had been loaded into next TBTT_TIMER
	*/
	if (Offset == (BCN_TBTT_OFFSET-2))
	{
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

		/* ASIC register in units of 1/16 TU = 64us*/
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod << 4) - 1 ;
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
	else if (Offset == (BCN_TBTT_OFFSET-1))
	{
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod) << 4;
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
}


VOID rtmp_mac_bcn_buf_init(IN RTMP_ADAPTER *pAd)
{
	int idx, tb_size;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	
	
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++)
		pAd->BeaconOffset[idx] = pChipCap->BcnBase[idx];

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tFlgIsSupSpecBcnBuf = %s\n", pChipCap->FlgIsSupSpecBcnBuf ? "TRUE" : "FALSE"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = %d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = %d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = 0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = %d\n", pChipCap->WcidHwRsvNum));
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++) {
		DBGPRINT(RT_DEBUG_TRACE, ("\t\tBcnBase[%d] = 0x%x, pAd->BeaconOffset[%d]=0x%x\n",
					idx, pChipCap->BcnBase[idx], idx, pAd->BeaconOffset[idx]));
	}

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
#ifdef SPECIFIC_BCN_BUF_SUPPORT
		if (pAd->chipCap.FlgIsSupSpecBcnBuf == TRUE)
		{
			RTMP_REG_PAIR BcnSpecMACRegTable[] = {
				/* 	
					That means all beacon's size are 512 bytes 
					and their starting address are "0x4000, 0x4200, 0x4400, 0x4600, ....." 
					in the second(higher) 8KB shared memory . 

					The formula is : 0x4000 + BCNx_OFFSET*64
						ex : the address of BSS0 = 0x4000 + 0x00 * 64 = 0x4000
							 the address of BSS1 = 0x4000 + 0x08 * 64 = 0x4200
				*/
				{BCN_OFFSET0,			0x18100800}, 
				{BCN_OFFSET1,			0x38302820}, 
				{BCN_OFFSET2,			0x58504840}, 
				{BCN_OFFSET3,			0x78706860}, 
			};

			tb_size = (sizeof(BcnSpecMACRegTable) / sizeof(RTMP_REG_PAIR));
			/* re-set beacon offset */
			for(idx = 0; idx < tb_size; idx++)
			{
				RTMP_IO_WRITE32(pAd, (USHORT)BcnSpecMACRegTable[idx].Register, 
										BcnSpecMACRegTable[idx].Value);
			}
		}
		else
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
		{
			RTMP_REG_PAIR bcn_legacy_reg_tb[] = {
#if defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x200)
				{BCN_OFFSET0,			0xf8f0e8e0}, 
				{BCN_OFFSET1,			0x6f77d0c8},
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x100)
				{BCN_OFFSET0,			0xece8e4e0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
				{BCN_OFFSET1,			0xfcf8f4f0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
#endif /* HW_BEACON_OFFSET */
			};
			
			tb_size = (sizeof(bcn_legacy_reg_tb) / sizeof(RTMP_REG_PAIR));
			for (idx = 0; idx < tb_size; idx++)
			{
				RTMP_IO_WRITE32(pAd, (USHORT)bcn_legacy_reg_tb[idx].Register, 
										bcn_legacy_reg_tb[idx].Value);
			}
		}
	}
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMP_REG_PAIR bcn_mac_reg_tb[] = {
			{RLT_BCN_OFFSET0, 0x18100800},
			{RLT_BCN_OFFSET1, 0x38302820},
			{RLT_BCN_OFFSET2, 0x58504840},
			{RLT_BCN_OFFSET3, 0x78706860},
		};

		tb_size = (sizeof(bcn_mac_reg_tb) / sizeof(RTMP_REG_PAIR));
		for (idx = 0; idx < tb_size; idx ++)
		{
			RTMP_IO_WRITE32(pAd, (USHORT)bcn_mac_reg_tb[idx].Register, 
									bcn_mac_reg_tb[idx].Value);
		}
	}
#endif /* RLT_MAC */
}


INT rtmp_mac_pbf_init(RTMP_ADAPTER *pAd)
{
	INT idx, tb_size = 0;
	RTMP_REG_PAIR *pbf_regs = NULL;
#ifdef RLT_MAC
	RTMP_REG_PAIR rlt_pbf_regs[]={
		{TX_MAX_PCNT,		0xefef3f1f},
		{RX_MAX_PCNT,		0xfebf},
	};
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	RTMP_REG_PAIR rtmp_pbf_regs[]={
#ifdef INF_AMAZON_SE
		/*
			iverson modify for usb issue, 2008/09/19
			6F + 6F < total page count FE
			so that RX doesn't occupy TX's buffer space when WMM congestion.
		*/
		{PBF_MAX_PCNT,			0x1F3F6F6F}, 
#else
		{PBF_MAX_PCNT,			0x1F3FBF9F}, 	/* 0x1F3f7f9f},		Jan, 2006/04/20 */
#endif /* INF_AMAZON_SE */
	};
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		pbf_regs = &rlt_pbf_regs[0];
		tb_size = (sizeof(rlt_pbf_regs) / sizeof(RTMP_REG_PAIR));
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		pbf_regs = &rtmp_pbf_regs[0];
		tb_size = (sizeof(rtmp_pbf_regs) / sizeof(RTMP_REG_PAIR));
	}
#endif /* RTMP_MAC */

	if ((pbf_regs != NULL) && (tb_size > 0))
	{
		for (idx = 0; idx < tb_size; idx++) {
			RTMP_IO_WRITE32(pAd, pbf_regs->Register, pbf_regs->Value);
			pbf_regs++;
		}
	}
	return TRUE;
}


/* 
	ASIC register initialization sets
*/
RTMP_REG_PAIR MACRegTable[] = {
	{LEGACY_BASIC_RATE,		0x0000013f}, /*  Basic rate set bitmap*/
	{HT_BASIC_RATE,		0x00008003}, /* Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.*/
	{MAC_SYS_CTRL,		0x00}, /* 0x1004, , default Disable RX*/
	{RX_FILTR_CFG,		0x17f97}, /*0x1400  , RX filter control,  */
	{BKOFF_SLOT_CFG,	0x209}, /* default set short slot time, CC_DELAY_TIME should be 2	 */
	{TX_SW_CFG1,		0x80606}, /* Gary,2006-08-23 */
	{TX_LINK_CFG,		0x1020},		/* Gary,2006-08-23 */
	/*{TX_TIMEOUT_CFG,	0x00182090},	 CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT*/
	{TX_TIMEOUT_CFG,	0x000a2090},	/* CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT , Modify for 2860E ,2007-08-01*/

	// TODO: shiang-usw, why MT7601 don't need to set this register??
	{LED_CFG,		0x7f031e46}, /* Gary, 2006-08-23*/

	/*{TX_RTY_CFG,			0x6bb80408},	 Jan, 2006/11/16*/
/* WMM_ACM_SUPPORT*/
/*	{TX_RTY_CFG,			0x6bb80101},	 sample*/
	{TX_RTY_CFG,			0x47d01f0f},	/* Jan, 2006/11/16, Set TxWI->ACK =0 in Probe Rsp Modify for 2860E ,2007-08-03*/
	
	{AUTO_RSP_CFG,			0x00000013},	/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
	{CCK_PROT_CFG,			0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */
	{OFDM_PROT_CFG,		0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */


	{GF20_PROT_CFG,			0x01744004},    /* set 19:18 --> Short NAV for MIMO PS*/
	{GF40_PROT_CFG,			0x03F44084},    
	{MM20_PROT_CFG,		0x01744004},    

#ifdef RTMP_MAC_PCI
	{MM40_PROT_CFG,		0x03F54084},	
#endif /* RTMP_MAC_PCI */

	{TXOP_CTRL_CFG,			0x0000583f, /*0x0000243f*/ /*0x000024bf*/},	/*Extension channel backoff.*/
	{TX_RTS_CFG,			0x00092b20},	

	{EXP_ACK_TIME,			0x002400ca},	/* default value */
	{TXOP_HLDR_ET, 			0x00000002},

	/* Jerry comments 2008/01/16: we use SIFS = 10us in CCK defaultly, but it seems that 10us
		is too small for INTEL 2200bg card, so in MBSS mode, the delta time between beacon0
		and beacon1 is SIFS (10us), so if INTEL 2200bg card connects to BSS0, the ping
		will always lost. So we change the SIFS of CCK from 10us to 16us. */
	{XIFS_TIME_CFG,			0x33a41010},
};

#ifdef RTMP_MAC
RTMP_REG_PAIR MACRegTable_RTMP[] = {
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},	/* 0x3018, MAX frame length. Max PSDU = 16kbytes.*/
	{PWR_PIN_CFG,		0x3},	/* patch for 2880-E*/
	/*{TX_SW_CFG0,		0x40a06},  Gary,2006-08-23 */
	{TX_SW_CFG0,		0x0}, 	/* Gary,2008-05-21 for CWC test */
};
#endif /* RTMP_MAC */

#ifdef CONFIG_AP_SUPPORT
RTMP_REG_PAIR APMACRegTable[] = {
	{WMM_AIFSN_CFG,		0x00001173},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034a6},
	{WMM_TXOP0_CFG,		0x00100020},
	{WMM_TXOP1_CFG,		0x002F0038},
	{TBTT_SYNC_CFG,		0x00012000},
#ifdef STREAM_MODE_SUPPORT
	{TX_CHAIN_ADDR0_L,	0xFFFFFFFF},	/* Broadcast frames are in stream mode*/
	{TX_CHAIN_ADDR0_H,	0x3FFFF},
#endif /* STREAM_MODE_SUPPORT */
};
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
RTMP_REG_PAIR STAMACRegTable[] = {
	{WMM_AIFSN_CFG,	0x00002273},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034aa},
};
#endif /* CONFIG_STA_SUPPORT */


#define NUM_MAC_REG_PARMS			(sizeof(MACRegTable) / sizeof(RTMP_REG_PAIR))
#ifdef CONFIG_AP_SUPPORT
#define NUM_AP_MAC_REG_PARMS		(sizeof(APMACRegTable) / sizeof(RTMP_REG_PAIR))
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#define NUM_STA_MAC_REG_PARMS	(sizeof(STAMACRegTable) / sizeof(RTMP_REG_PAIR))
#endif /* CONFIG_STA_SUPPORT */
#ifdef RTMP_MAC
#define NUM_RTMP_MAC_REG_PARAMS	(sizeof(MACRegTable_RTMP)/ sizeof(RTMP_REG_PAIR))
#endif /* RTMP_MAC */

INT rtmp_mac_init(RTMP_ADAPTER *pAd)
{
	INT idx;

	for (idx = 0; idx < NUM_MAC_REG_PARMS; idx++)
	{
		RTMP_IO_WRITE32(pAd, MACRegTable[idx].Register, MACRegTable[idx].Value);
	}

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		UINT32 reg, val;
	
		for (idx = 0; idx < NUM_RTMP_MAC_REG_PARAMS; idx++){
			reg = MACRegTable_RTMP[idx].Register;
			val = MACRegTable_RTMP[idx].Value;
			RTMP_IO_WRITE32(pAd, reg, val);
		}
	}
#endif /* RTMP_MAC */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (idx = 0; idx < NUM_AP_MAC_REG_PARMS; idx++)
		{
			RTMP_IO_WRITE32(pAd,
				APMACRegTable[idx].Register,
				APMACRegTable[idx].Value);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		for (idx = 0; idx < NUM_STA_MAC_REG_PARMS; idx++)
		{
			RTMP_IO_WRITE32(pAd,
				STAMACRegTable[idx].Register,
				STAMACRegTable[idx].Value);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	rtmp_mac_pbf_init(pAd);

	/* re-set specific MAC registers for individual chip */
	if (pAd->chipOps.AsicMacInit != NULL)
		pAd->chipOps.AsicMacInit(pAd);

	return TRUE;
}

