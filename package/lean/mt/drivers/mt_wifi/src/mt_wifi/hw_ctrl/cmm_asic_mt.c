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
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic_mt.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

extern const UCHAR wmm_aci_2_hw_ac_queue[18];

#ifdef CONFIG_AP_SUPPORT
/* because of the CR arrangement in HW are not in sequence, wrape the table to search fast.*/
static UINT32 LPON_TT0SBOR_CR_MAPPING_TABLE[] = {
	LPON_TT0TPCR,
	LPON_TT0SBOR1,
	LPON_TT0SBOR2,
	LPON_TT0SBOR3,
	LPON_TT0SBOR4,
	LPON_TT0SBOR5,
	LPON_TT0SBOR6,
	LPON_TT0SBOR7,
	LPON_TT0SBOR8,
	LPON_TT0SBOR9,
	LPON_TT0SBOR10,
	LPON_TT0SBOR11,
	LPON_TT0SBOR12,
	LPON_TT0SBOR13,
	LPON_TT0SBOR14,
	LPON_TT0SBOR15,
};
#endif

/**
 * @addtogroup pse
 * @{
 * @name pse HW HAL
 * @{
 */

#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
INT set_get_fid(RTMP_ADAPTER *pAd, char *arg)
{
	/* TODO: Carter, at present, only can read pkt in Port2(LMAC port) */
	UCHAR   q_idx = 0, loop = 0, dw_idx = 0;
	UINT32  head_fid_addr = 0, next_fid_addr = 0, value = 0x00000000L, dw_content;
	/* TODO: Star */
	q_idx = simple_strtol(arg, 0, 10);
	value = 0x00400000 | (q_idx << 16);/* port2. queue by input value. */
	MAC_IO_WRITE32(pAd, 0x8024, value);
	MAC_IO_READ32(pAd, 0x8024, (UINT32 *)&head_fid_addr);/* get head FID. */
	head_fid_addr = head_fid_addr & 0xfff;

	if (head_fid_addr == 0xfff) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, q_idx:%d empty!!\n", __func__, q_idx));
		return TRUE;
	}

	value = (0 | (head_fid_addr << 16));

	while (1) {
		for (dw_idx = 0; dw_idx < 8; dw_idx++) {
			MAC_IO_READ32(pAd, ((MT_PCI_REMAP_ADDR_1 + (((value & 0x0fff0000) >> 16) * 128)) + (dw_idx * 4)), (UINT32 *)&dw_content);/* get head FID. */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pkt:%d, fid:%x, dw_idx = %d, dw_content = 0x%x\n", loop, ((value & 0x0fff0000) >> 16), dw_idx, dw_content));
		}

		MAC_IO_WRITE32(pAd, 0x8028, value);
		MAC_IO_READ32(pAd, 0x8028, (UINT32 *)&next_fid_addr);/* get next FID. */

		if ((next_fid_addr & 0xfff) == 0xfff)
			return TRUE;

		value = (0 | ((next_fid_addr & 0xffff) << 16));
		loop++;

		if (loop > 5)
			return TRUE;
	}

	return TRUE;
}
#endif

/** @} */
/** @} */

/* Static API for local usage */
static VOID Wtbl2TxRateCounterGet(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, TX_CNT_INFO *tx_cnt_info);


static BOOLEAN WtblWaitIdle(RTMP_ADAPTER *pAd, UINT32 WaitCnt, UINT32 WaitDelay)
{
	UINT32 Value, CurCnt = 0;

	do {
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);

		if ((Value & IU_BUSY)  == 0)
			break;

		CurCnt++;
		RtmpusecDelay(WaitDelay);
	} while (CurCnt < WaitCnt);

	if (CurCnt == WaitCnt) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
				 __func__, Value));
		return FALSE;
	}

	return TRUE;
}


UINT32 MtAsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
	return 0;
}


UINT32 MtAsicGetPhyErrCnt(RTMP_ADAPTER *pAd)
{
	return 0;
}


UINT32 MtAsicGetCCACnt(RTMP_ADAPTER *pAd)
{
	return 0;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */


VOID MtAsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
	/* Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	*/
	/* IF ops->AsicResetBbpAgent == NULL, run "else" part */
	/* TODO: shiang-usw, unify the ops */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	if (ops->AsicResetBbpAgent != NULL)
		ops->AsicResetBbpAgent(pAd);
}

/**
 * @addtogroup bss_dev_sta_info
 * @{
 * @name BSS Info/Device Info/STA Record HW HAL
 * @{
 */
/*
 * ==========================================================================
 * Description:
 * Set My BSSID
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
/* CFG_TODO */
VOID MtAsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR BssidIndex)
{
	UINT32 val;
	/* TODO: shiang-7603, now only configure Current BSSID Address 0 */
	ULONG curr_bssid_reg_base = RMAC_CB0R0;/* register for Current_Bssid from 0x60140004 */

	val = (UINT32)((pBssid[0]) |
				   (UINT32)(pBssid[1] << 8) |
				   (UINT32)(pBssid[2] << 16) |
				   (UINT32)(pBssid[3] << 24));
	MAC_IO_WRITE32(pAd, curr_bssid_reg_base + (BssidIndex * 8), val);
	val = (UINT32)(pBssid[4]) | (UINT32)(pBssid[5] << 8) | (1 << 16);
	MAC_IO_WRITE32(pAd, (curr_bssid_reg_base + 4) + (BssidIndex * 8), val);
	/* If we enable BSSID0, we should not enable MBSS0, or the BSSID index will wrong */
	/* MAC_IO_READ32(pAd, RMAC_ACBEN, &val); */
	/* val |= 0x1; */
	/* MAC_IO_WRITE32(pAd, RMAC_ACBEN, val); */
}


INT32 MtAsicSetDevMac(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	UINT32 val;
	ULONG own_mac_reg_base = RMAC_OMA0R0;/* register for Own_Mac from 0x60140024 */

	if (Active) {
		val = (OwnMacAddr[0]) | (OwnMacAddr[1]<<8) |  (OwnMacAddr[2]<<16) | (OwnMacAddr[3]<<24);
		MAC_IO_WRITE32(pAd, own_mac_reg_base + (OwnMacIdx * 8), val);
		val = OwnMacAddr[4] | (OwnMacAddr[5]<<8) |	(1 << 16);
		MAC_IO_WRITE32(pAd, (own_mac_reg_base + 4) + (OwnMacIdx * 8), val);
	}

#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
	AsicDevInfoUpdate(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, DEVINFO_ACTIVE_FEATURE);
#endif
	return TRUE;
}

/** @}  */
/** @}  */

/**
 * @addtogroup tx_rx_path
 * @{
 * @name TX/RX HW HAL
 * @{
 */



#ifdef CONFIG_WTBL_TLV_MODE
INT MtAsicSetRDGByTLV(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UINT8 Wcid)
{
	CMD_WTBL_RDG_T		CmdWtblRdg = {0};

	CmdWtblRdg.u2Tag = WTBL_RDG;
	CmdWtblRdg.u2Length = sizeof(CMD_WTBL_RDG_T);

	if (bEnable) {
		CmdWtblRdg.ucRdgBa = 1;
		CmdWtblRdg.ucR = 1;
	}

	return CmdExtWtblUpdate(pAd, Wcid, SET_WTBL, (PUCHAR)&CmdWtblRdg, sizeof(CMD_WTBL_RDG_T));
}

#else

#ifdef DOT11_N_SUPPORT
INT MtAsicWtblSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UINT8 Wcid)
{
	struct wtbl_entry tb_entry;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;

	os_zero_mem(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
				 __func__, Wcid));
		return FALSE;
	}

	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), &dw2->word);

	if (bEnable) {
		dw2->field.r = 1;
		dw2->field.rdg_ba = 1;
	} else {
		dw2->field.r = 0;
		dw2->field.rdg_ba = 0;
	}

	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), dw2->word);
	return TRUE;
}


INT MtAsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR BandIdx)
{
	UINT32 tmac_tcr, agg_pcr, tmac_trcr;

	MAC_IO_READ32(pAd, TMAC_TCR, &tmac_tcr);
	MAC_IO_READ32(pAd, AGG_PCR, &agg_pcr);
	MAC_IO_READ32(pAd, TMAC_TRCR, &tmac_trcr);

	if (bEnable) {
		/* enable RDG Rsp. also w/ Ralink Mode is necessary */
		tmac_tcr |= (RDG_RA_MODE | RDG_RESP_EN);
		/* LongNAV protect */
		agg_pcr &= ~(PROTECTION_MODE);
		tmac_trcr = tmac_trcr & ~I2T_CHK_EN;
	} else {
		/* disable RDG Rsp. also w/ Ralink Mode is necessary */
		tmac_tcr &= ~(RDG_RA_MODE | RDG_RESP_EN);
		/* single frame protect */
		agg_pcr |= PROTECTION_MODE;
		tmac_trcr = tmac_trcr | I2T_CHK_EN;
	}

	MAC_IO_WRITE32(pAd, TMAC_TCR, tmac_tcr);
	MAC_IO_WRITE32(pAd, AGG_PCR, agg_pcr);
	MAC_IO_WRITE32(pAd, TMAC_TRCR, tmac_trcr);
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */
#endif

INT32 MtAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 Value, Value1, Value2;

	MAC_IO_READ32(pAd, ARB_SCR, &Value);
	MAC_IO_READ32(pAd, ARB_TQCR0, &Value1);
	MAC_IO_READ32(pAd, ARB_RQCR, &Value2);

	switch (TxRx) {
	case ASIC_MAC_TX:
		if (Enable) {
			Value &= ~MT_ARB_SCR_TXDIS;
			Value1 = 0xffffffff;
		} else {
			Value |= MT_ARB_SCR_TXDIS;
			Value1 = 0;
		}

		break;

	case ASIC_MAC_RX:
		if (Enable) {
			Value &= ~MT_ARB_SCR_RXDIS;
			Value2 |= ARB_RQCR_RX_START;
		} else {
			Value |= MT_ARB_SCR_RXDIS;
			Value2 &= ~ARB_RQCR_RX_START;
		}

		break;

	case ASIC_MAC_TXRX:
		if (Enable) {
			Value &= ~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value1 = 0xffffffff;
			Value2 |= ARB_RQCR_RX_START;
		} else {
			Value |= (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value1 = 0;
			Value2 &= ~ARB_RQCR_RX_START;
		}

		break;

	case ASIC_MAC_TXRX_RXV:
		if (Enable) {
			Value &= ~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value1 = 0xffffffff;
			Value2 |= (ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
					   ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		} else {
			Value |= (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value1 = 0;
			Value2 &= ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
						ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		}

		break;

	case ASIC_MAC_RXV:
		if (Enable) {
			Value &= ~MT_ARB_SCR_RXDIS;
			Value2 |= (ARB_RQCR_RXV_START |
					   ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		} else {
			Value2 &= ~(ARB_RQCR_RXV_START |
						ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		}

		break;

	case ASIC_MAC_RX_RXV:
		if (Enable) {
			Value &= ~MT_ARB_SCR_RXDIS;
			Value2 |= (ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
					   ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		} else {
			Value |= MT_ARB_SCR_RXDIS;
			Value2 &= ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
						ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __func__,
				 TxRx));
		break;
	}

	MAC_IO_WRITE32(pAd, ARB_SCR, Value);
	MAC_IO_WRITE32(pAd, ARB_TQCR0, Value1);
	MAC_IO_WRITE32(pAd, ARB_RQCR, Value2);
	return TRUE;
}


VOID MtAsicSetTxSClassifyFilter(RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ,
								UINT32 AggNums, UINT32 Filter, UCHAR BandIdx)
{
	UINT32 Value;

	if (Port == TXS2HOST) {
		MAC_IO_READ32(pAd, DMA_TCFR1, &Value);
		Value &= ~TXS2H_BIT_MAP_MASK;
		Value |= TXS2H_BIT_MAP(Filter);
		Value &= ~TXS2H_AGG_CNT_MASK;
		Value |= TXS2H_AGG_CNT(AggNums);

		if (DestQ == 0)
			Value &= ~TXS2H_QID;
		else
			Value |= TXS2H_QID;

		MAC_IO_WRITE32(pAd, DMA_TCFR1, Value);
	} else if (Port == TXS2MCU) {
		MAC_IO_READ32(pAd, DMA_TCFR0, &Value);
		Value &= ~TXS2M_BIT_MAP_MASK;
		Value |= TXS2M_BIT_MAP(Filter);
		Value &= ~TXS2M_AGG_CNT_MASK;
		Value |= TXS2M_AGG_CNT(AggNums);
		Value &= ~TXS2M_QID_MASK;
		Value |= TXS2M_QID(DestQ);
		MAC_IO_WRITE32(pAd, DMA_TCFR0, Value);
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow Port(%d)\n", __func__, Port));
}




#define RX_PKT_MAX_LENGTH   0x400 /* WORD(4 Bytes) unit */
INT MtAsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	/* TODO: shiang-7603 */
	UINT32 val;
	/* Rx max packet length */
	MAC_IO_READ32(pAd, DMA_DCR0, &val);
	val &= (~0xfffc);
	val |= (RX_PKT_MAX_LENGTH << 2);
	MAC_IO_WRITE32(pAd, DMA_DCR0, val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Set the Max RxPktLen=%d!\n",
			 __func__, __LINE__, RX_PKT_MAX_LENGTH));
	return TRUE;
}

INT MtAsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, TMAC_TCR, &Value);
	Value &= ~TMAC_TCR_TX_STREAM_NUM_MASK;
	Value |= TMAC_TCR_TX_STREAM_NUM(StreamNums - 1);
	MAC_IO_WRITE32(pAd, TMAC_TCR, Value);
	return TRUE;
}

INT MtAsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 Value, Mask = 0;
	INT Ret = TRUE;

	MAC_IO_READ32(pAd, RMAC_RMCR, &Value);
	Value &= ~(RMAC_RMCR_RX_STREAM_0 |
			   RMAC_RMCR_RX_STREAM_1 |
			   RMAC_RMCR_RX_STREAM_2);

	switch (StreamNums) {
	case 3:
		Mask |= RMAC_RMCR_RX_STREAM_2;

	case 2:
		Mask |= RMAC_RMCR_RX_STREAM_1;

	case 1:
		Mask |= RMAC_RMCR_RX_STREAM_0;
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal StreamNums(%d\n", StreamNums));
		Ret = FALSE;
		break;
	}

	if (Ret) {
		Value |= Mask;
		Value &= ~RMAC_RMCR_SMPS_MODE_MASK;
		Value |= RMAC_RMCR_SMPS_MODE(DISABLE_SMPS_RX_BYSETTING);
		MAC_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	return Ret;
}

INT MtAsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx)
{
	UINT32 Value = 0, Mask = 0;
	INT Ret = TRUE;

	MAC_IO_READ32(pAd, RMAC_RMCR, &Value);
	Value &= ~(RMAC_RMCR_RX_STREAM_0 |
			   RMAC_RMCR_RX_STREAM_1 |
			   RMAC_RMCR_RX_STREAM_2);

	switch (RxPathSel) {
	case 0: /* ALL */
		Mask = (RMAC_RMCR_RX_STREAM_0 | RMAC_RMCR_RX_STREAM_1 | RMAC_RMCR_RX_STREAM_2);
		break;

	case 1: /* RX0 */
		Mask = RMAC_RMCR_RX_STREAM_0;
		break;

	case 2: /* RX1 */
		Mask = RMAC_RMCR_RX_STREAM_1;
		break;

	case 3: /* RX2 */
		Mask = RMAC_RMCR_RX_STREAM_2;
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal RxPath(%d\n", RxPathSel));
		Ret = FALSE;
		break;
	}

	if (Ret) {
		Value |= Mask;
		Value &= ~RMAC_RMCR_SMPS_MODE_MASK;
		Value |= RMAC_RMCR_SMPS_MODE(DISABLE_SMPS_RX_BYSETTING);
		MAC_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	return Ret;
}

static VOID RxFilterCfg2Row(UINT32 FilterMask, UINT32 *RowFilterMask)
{
	*RowFilterMask = FilterMask & RX_STBC_BCN_BC_MC ?
					 (*RowFilterMask | DROP_STBC_BCN_BC_MC) : (*RowFilterMask & ~(DROP_STBC_BCN_BC_MC));
	*RowFilterMask = FilterMask & RX_FCS_ERROR ?
					 (*RowFilterMask | DROP_FCS_ERROR_FRAME) : (*RowFilterMask & ~(DROP_FCS_ERROR_FRAME));
	*RowFilterMask = FilterMask & RX_PROTOCOL_VERSION ?
					 (*RowFilterMask | DROP_VERSION_NO_0) : (*RowFilterMask & ~(DROP_VERSION_NO_0));
	*RowFilterMask = FilterMask & RX_PROB_REQ ?
					 (*RowFilterMask | DROP_PROBE_REQ) : (*RowFilterMask & ~(DROP_PROBE_REQ));
	*RowFilterMask = FilterMask & RX_MC_ALL ?
					 (*RowFilterMask | DROP_MC_FRAME) : (*RowFilterMask & ~(DROP_MC_FRAME));
	*RowFilterMask = FilterMask & RX_BC_ALL ?
					 (*RowFilterMask | DROP_BC_FRAME) : (*RowFilterMask & ~(DROP_BC_FRAME));
	*RowFilterMask = FilterMask & RX_MC_TABLE ?
					 (*RowFilterMask | DROP_NOT_IN_MC_TABLE) : (*RowFilterMask & ~(DROP_NOT_IN_MC_TABLE));
	*RowFilterMask = FilterMask & RX_BC_MC_OWN_MAC_A3 ?
					 (*RowFilterMask | DROP_ADDR3_OWN_MAC) : (*RowFilterMask & ~(DROP_ADDR3_OWN_MAC));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A3 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A3) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A3));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A2 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A2) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A2));
	*RowFilterMask = FilterMask & RX_BCN_DIFF_BSSID ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BCN) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BCN));
	*RowFilterMask = FilterMask & RX_CTRL_RSV ?
					 (*RowFilterMask | DROP_CTRL_RSV) : (*RowFilterMask & ~(DROP_CTRL_RSV));
	*RowFilterMask = FilterMask & RX_CTS ?
					 (*RowFilterMask | DROP_CTS) : (*RowFilterMask & ~(DROP_CTS));
	*RowFilterMask = FilterMask & RX_RTS ?
					 (*RowFilterMask | DROP_RTS) : (*RowFilterMask & ~(DROP_RTS));
	*RowFilterMask = FilterMask & RX_DUPLICATE ?
					 (*RowFilterMask | DROP_DUPLICATE) : (*RowFilterMask & ~(DROP_DUPLICATE));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BSSID ?
					 (*RowFilterMask | DROP_NOT_MY_BSSID) : (*RowFilterMask & ~(DROP_NOT_MY_BSSID));
	*RowFilterMask = FilterMask & RX_NOT_OWN_UCAST ?
					 (*RowFilterMask | DROP_NOT_UC2ME) : (*RowFilterMask & ~(DROP_NOT_UC2ME));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BTIM ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BTIM) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BTIM));
	*RowFilterMask = FilterMask & RX_NDPA ?
					 (*RowFilterMask | DROP_NDPA) : (*RowFilterMask & ~(DROP_NDPA));
}

INT MtAsicSetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter)
{
	UINT32 Value = 0;

	if (RxFilter.bPromiscuous) {
#ifdef SNIFFER_MT7615

		if (IS_MT7615(pAd))
			Value = DROP_FCS_ERROR_FRAME | RM_FRAME_REPORT_EN | RX_UNWANTED_CTL_FRM;
		else
#endif
			Value = 0;/* RX_PROMISCUOUS_MODE; */
	} else {
		/*disable frame report & monitor mode*/
		Value &= ~RX_PROMISCUOUS_MODE;

		if (RxFilter.bFrameReport)
			Value |= RM_FRAME_REPORT_EN;
		else
			Value &=  ~(RM_FRAME_REPORT_EN);

		/*enable drop filter by RxfilterMask*/
		RxFilterCfg2Row(RxFilter.filterMask, &Value);
	}

	MAC_IO_WRITE32(pAd, RMAC_RFCR, Value);
	MAC_IO_WRITE32(pAd, RMAC_RFCR1, 0);
	return TRUE;
}

INT32 MtAsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan,
							 BOOLEAN RmVlan, BOOLEAN SwPcP)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, DMA_DCR0, &Value);

	if (En) {
		Value |= RX_HD_TRANS_EN;

		if (ChkBssid)
			Value |= RX_HDR_TRANS_CHK_BSSID;
		else
			Value &= ~RX_HDR_TRANS_CHK_BSSID;

		if (InSVlan)
			Value |= RX_INS_VLAN;
		else
			Value &= ~RX_INS_VLAN;

		if (RmVlan)
			Value |= RX_RM_VLAN;
		else
			Value &= ~RX_RM_VLAN;

		if (!SwPcP)
			Value |= PCP_EQ_TID;
		else
			Value &= ~PCP_EQ_TID;
	} else
		Value &= ~RX_HD_TRANS_EN;

	MAC_IO_WRITE32(pAd, DMA_DCR0, Value);
	return TRUE;
}



INT32 MtAsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
	UINT32 Value, Value1;

	MAC_IO_READ32(pAd, DMA_DCR0, &Value);

	switch (Index) {
	case ETBL_INDEX0:
		if (En) {
			Value |= ETBL0_EN;
			MAC_IO_READ32(pAd, ETBLR0, &Value1);
			Value1 &= ~ETBL0_MASK;
			Value1 |= ETBL0(EthType);
			MAC_IO_WRITE32(pAd, ETBLR0, Value1);
		} else
			Value &= ~ETBL0_EN;

		break;

	case ETBL_INDEX1:
		if (En) {
			Value |= ETBL1_EN;
			MAC_IO_READ32(pAd, ETBLR0, &Value1);
			Value1 &= ~ETBL1_MASK;
			Value1 |= ETBL1(EthType);
			MAC_IO_WRITE32(pAd, ETBLR0, Value1);
		} else
			Value &= ~ETBL1_EN;

		break;

	case ETBL_INDEX2:
		if (En) {
			Value |= ETBL2_EN;
			MAC_IO_READ32(pAd, ETBLR1, &Value1);
			Value1 &= ~ETBL2_MASK;
			Value1 |= ETBL2(EthType);
			MAC_IO_WRITE32(pAd, ETBLR1, Value1);
		} else
			Value &= ~ETBL2_EN;

		break;

	case ETBL_INDEX3:
		if (En) {
			Value |= ETBL3_EN;
			MAC_IO_READ32(pAd, ETBLR1, &Value1);
			Value1 &= ~ETBL3_MASK;
			Value1 |= ETBL3(EthType);
			MAC_IO_WRITE32(pAd, ETBLR1, Value1);
		} else
			Value &= ~ETBL3_EN;

		break;

	case ETBL_INDEX4:
		if (En) {
			Value |= ETBL4_EN;
			MAC_IO_READ32(pAd, ETBLR2, &Value1);
			Value1 &= ~ETBL4_MASK;
			Value1 |= ETBL4(EthType);
			MAC_IO_WRITE32(pAd, ETBLR2, Value1);
		} else
			Value &= ~ETBL4_EN;

		break;

	case ETBL_INDEX5:
		if (En) {
			Value |= ETBL5_EN;
			MAC_IO_READ32(pAd, ETBLR2, &Value1);
			Value1 &= ~ETBL5_MASK;
			Value1 |= ETBL5(EthType);
			MAC_IO_WRITE32(pAd, ETBLR2, Value1);
		} else
			Value &= ~ETBL5_EN;

		break;

	case ETBL_INDEX6:
		if (En) {
			Value |= ETBL6_EN;
			MAC_IO_READ32(pAd, ETBLR3, &Value1);
			Value1 &= ~ETBL6_MASK;
			Value1 |= ETBL6(EthType);
			MAC_IO_WRITE32(pAd, ETBLR3, Value1);
		} else
			Value &= ~ETBL6_EN;

		break;

	case ETBL_INDEX7:
		if (En) {
			Value |= ETBL7_EN;
			MAC_IO_READ32(pAd, ETBLR3, &Value1);
			Value1 &= ~ETBL7_MASK;
			Value1 |= ETBL7(EthType);
			MAC_IO_WRITE32(pAd, ETBLR3, Value1);
		} else
			Value &= ~ETBL7_EN;

		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown Index(%d)\n", __func__, Index));
		break;
	}

	MAC_IO_WRITE32(pAd, DMA_DCR0, Value);
	return TRUE;
}

INT32  MtAsicUpdateBASessionByDriver(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	struct wtbl_entry ent;
	struct wtbl_2_struc *wtbl_2;
	UINT32 range_mask = 0x7 << (BaCtrl.Tid * 3);
	UINT32 reg, value;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR *ba_range = cap->ba_range;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			value = (BaCtrl.PeerAddr[0] | (BaCtrl.PeerAddr[1] << 8) |
					 (BaCtrl.PeerAddr[2] << 16) | (BaCtrl.PeerAddr[3] << 24));
			MAC_IO_WRITE32(pAd, BSCR0, value);
			MAC_IO_READ32(pAd, BSCR1, &value);
			value &= ~(BA_MAC_ADDR_47_32_MASK | RST_BA_TID_MASK | RST_BA_SEL_MASK);
			value |= BA_MAC_ADDR_47_32((BaCtrl.PeerAddr[4] | (BaCtrl.PeerAddr[5] << 8)));
			value |= (RST_BA_SEL(RST_BA_MAC_TID_MATCH) | RST_BA_TID(BaCtrl.Tid) | START_RST_BA_SB);
			MAC_IO_WRITE32(pAd, BSCR1, value);
		}
	} else {
		os_zero_mem((UCHAR *)(&ent), sizeof(struct wtbl_entry));
		mt_wtbl_get_entry234(pAd, BaCtrl.Wcid, &ent);
		wtbl_2 = &ent.wtbl_2;

		if (BaCtrl.isAdd) {
			INT idx = 0;

			MtAsicSetMacTxRx(pAd, ASIC_MAC_TX, FALSE, 0);

			/* Clear WTBL2. SN: Direct Updating */
			switch (BaCtrl.Tid) {
			case 0:
				reg = ent.wtbl_addr[1] + (4 * 2); /* WTBL2.DW2 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d2.word);
				wtbl_2->wtbl_2_d2.field.tid_ac_0_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d2.word);
				break;

			case 1:
				reg = ent.wtbl_addr[1] + (4 * 2); /* WTBL2.DW2 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d2.word);
				wtbl_2->wtbl_2_d2.field.tid_ac_1_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d2.word);
				break;

			case 2:
				reg = ent.wtbl_addr[1] + (4 * 2); /* WTBL2.DW2 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d2.word);
				wtbl_2->wtbl_2_d2.field.tid_ac_2_sn_0 = (BaCtrl.Sn & 0xff);
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d2.word);
				reg = ent.wtbl_addr[1] + (4 * 3); /* WTBL2.DW3 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d3.word);
				wtbl_2->wtbl_2_d3.field.tid_ac_2_sn_9 = ((BaCtrl.Sn & (0xf << 8)) >> 8);
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d3.word);
				break;

			case 3:
				reg = ent.wtbl_addr[1] + (4 * 3); /* WTBL2.DW3 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d3.word);
				wtbl_2->wtbl_2_d3.field.tid_ac_3_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d3.word);
				break;

			case 4:
				reg = ent.wtbl_addr[1] + (4 * 3); /* WTBL2.DW3 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d3.word);
				wtbl_2->wtbl_2_d3.field.tid_4_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d3.word);
				break;

			case 5:
				reg = ent.wtbl_addr[1] + (4 * 3); /* WTBL2.DW3 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d3.word);
				wtbl_2->wtbl_2_d3.field.tid_5_sn_0 = (BaCtrl.Sn & 0xf);
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d3.word);
				reg = ent.wtbl_addr[1] + (4 * 4); /* WTBL2.DW4 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d4.word);
				wtbl_2->wtbl_2_d4.field.tid_5_sn_5 = ((BaCtrl.Sn & (0xff << 4)) >> 4);
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d4.word);
				break;

			case 6:
				reg = ent.wtbl_addr[1] + (4 * 4); /* WTBL2.DW4 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d4.word);
				wtbl_2->wtbl_2_d4.field.tid_6_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d4.word);
				break;

			case 7:
				reg = ent.wtbl_addr[1] + (4 * 4); /* WTBL2.DW4 */
				HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d4.word);
				wtbl_2->wtbl_2_d4.field.tid_7_sn = BaCtrl.Sn;
				HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d4.word);
				break;

			default:
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown tid(%d)\n", __func__, BaCtrl.Tid));
				break;
			}

			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &value);
			value &= ~WLAN_IDX_MASK;
			value |= WLAN_IDX(BaCtrl.Wcid);
			value |= WTBL2_UPDATE_FLAG;
			MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, value);
			MtAsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE, 0);

			/*get ba win size from range */
			while (ba_range[idx] < BaCtrl.BaWinSize) {
				if (idx == 7)
					break;

				idx++;
			};

			if (ba_range[idx] > BaCtrl.BaWinSize)
				idx--;

			reg = ent.wtbl_addr[1] + (15 * 4);
			HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d15.word);
			wtbl_2->wtbl_2_d15.field.ba_en |= 1 << BaCtrl.Tid;
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid &= (~range_mask);
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid |= (idx << (BaCtrl.Tid * 3));
			HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d15.word);
		} else {
			reg = ent.wtbl_addr[1] + (15 * 4);
			HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d15.word);
			wtbl_2->wtbl_2_d15.field.ba_en &=  (~(1 << BaCtrl.Tid));
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid &= (~range_mask);
			HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d15.word);
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ent->wtbl_addr[1]=0x%x, val=0x%08x\n",
				 __func__, reg, wtbl_2->wtbl_2_d15.word));
	}

	return 0;
}


INT32 MtAsicUpdateBASession(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	return MtAsicUpdateBASessionByDriver(pAd, BaCtrl);
}


VOID MtAsicSetRxGroup(RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable)
{
	UINT32 Value;

	if (Port == HIF_PORT) {
		MCU_IO_READ32(pAd, RXINF, &Value);

		if (Enable) {
			if (Group & RXS_GROUP1)
				Value |= RXSH_GROUP1_EN;

			if (Group & RXS_GROUP2)
				Value |= RXSH_GROUP2_EN;

			if (Group & RXS_GROUP3)
				Value |= RXSH_GROUP3_EN;
		} else {
			if (Group & RXS_GROUP1)
				Value &= ~RXSH_GROUP1_EN;

			if (Group & RXS_GROUP2)
				Value &= ~RXSH_GROUP2_EN;

			if (Group & RXS_GROUP3)
				Value &= ~RXSH_GROUP3_EN;
		}

		MCU_IO_WRITE32(pAd, RXINF, Value);
	} else if (Port == MCU_PORT) {
		MAC_IO_READ32(pAd, DMA_DCR1, &Value);

		if (Enable) {
			if (Group & RXS_GROUP1)
				Value |= RXSM_GROUP1_EN;

			if (Group & RXS_GROUP2)
				Value |= RXSM_GROUP2_EN;

			if (Group & RXS_GROUP3)
				Value |= RXSM_GROUP3_EN;
		} else {
			if (Group & RXS_GROUP1)
				Value &= ~RXSM_GROUP1_EN;

			if (Group & RXS_GROUP2)
				Value &= ~RXSM_GROUP2_EN;

			if (Group & RXS_GROUP3)
				Value &= ~RXSM_GROUP3_EN;
		}

		MAC_IO_WRITE32(pAd, DMA_DCR1, Value);
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal port (%d\n", Port));
}


INT MtAsicSetBAWinSizeRange(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;
	UINT32 TxAggLimit;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR *ba_range = cap->ba_range;
	/* A-MPDU BA WinSize control */
	MAC_IO_READ32(pAd, AGG_AWSCR, &mac_val);
	mac_val &= ~WINSIZE0_MASK;
	mac_val |= WINSIZE0(ba_range[0]);
	mac_val &= ~WINSIZE1_MASK;
	mac_val |= WINSIZE1(ba_range[1]);
	mac_val &= ~WINSIZE2_MASK;
	mac_val |= WINSIZE2(ba_range[2]);
	mac_val &= ~WINSIZE3_MASK;
	mac_val |= WINSIZE3(ba_range[3]);
	MAC_IO_WRITE32(pAd, AGG_AWSCR, mac_val);
	MAC_IO_READ32(pAd, AGG_AWSCR1, &mac_val);
	mac_val &= ~WINSIZE4_MASK;
	mac_val |= WINSIZE4(ba_range[4]);
	mac_val &= ~WINSIZE5_MASK;
	mac_val |= WINSIZE5(ba_range[5]);
	mac_val &= ~WINSIZE6_MASK;
	mac_val |= WINSIZE6(ba_range[6]);
	mac_val &= ~WINSIZE7_MASK;
	mac_val |= WINSIZE7(ba_range[7]);
	MAC_IO_WRITE32(pAd, AGG_AWSCR1, mac_val);
	/* A-MPDU Agg limit control */
	TxAggLimit = cap->TxAggLimit;
	MAC_IO_READ32(pAd, AGG_AALCR, &mac_val);
	mac_val &= ~AC0_AGG_LIMIT_MASK;
	mac_val |= AC0_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC1_AGG_LIMIT_MASK;
	mac_val |= AC1_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC2_AGG_LIMIT_MASK;
	mac_val |= AC2_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC3_AGG_LIMIT_MASK;
	mac_val |= AC3_AGG_LIMIT(TxAggLimit);
	MAC_IO_WRITE32(pAd, AGG_AALCR, mac_val);
	MAC_IO_READ32(pAd, AGG_AALCR1, &mac_val);
	mac_val &= ~AC10_AGG_LIMIT_MASK;
	mac_val |= AC10_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC11_AGG_LIMIT_MASK;
	mac_val |= AC11_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC12_AGG_LIMIT_MASK;
	mac_val |= AC12_AGG_LIMIT(TxAggLimit);
	mac_val &= ~AC13_AGG_LIMIT_MASK;
	mac_val |= AC13_AGG_LIMIT(TxAggLimit);
	MAC_IO_WRITE32(pAd, AGG_AALCR1, mac_val);
	return TRUE;
}
VOID MtAsicSetBARTxCntLimit(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, AGG_MRCR, &Value);

	if (Enable) {
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(Count);
	} else {
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(0);
	}

#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef CONFIG_AP_SUPPORT
static VOID MtAsicSetRTSRetryCnt(RTMP_ADAPTER *pAd)
{
	UINT32 Value;
	UINT32 Count = pAd->ApCfg.rts_retry_cnt;

	if (Count != 0) {
		/* TODO: RTY_MODE0/1 ?? */
		MAC_IO_READ32(pAd, AGG_MRCR, &Value);
		Value &= ~RTS_RTY_CNT_LIMIT_MASK;
		Value |= RTS_RTY_CNT_LIMIT(Count);
		MAC_IO_WRITE32(pAd, AGG_MRCR, Value);
	}
}
#endif /* CONFIG_AP_SUPPORT */
#endif
#ifndef MAC_INIT_OFFLOAD
/*
 * Init TxD short format template which will copy by PSE-Client to LMAC
 */
static INT MtAsicSetTmacInfoTemplate(RTMP_ADAPTER *pAd)
{
	UINT32 dw[5];
	TMAC_TXD_2 *dw2 = (TMAC_TXD_2 *)(&dw[0]);
	TMAC_TXD_3 *dw3 = (TMAC_TXD_3 *)(&dw[1]);
	TMAC_TXD_5 *dw5 = (TMAC_TXD_5 *)(&dw[3]);

	os_zero_mem((UCHAR *)(&dw[0]), sizeof(dw));
	dw2->HtcExist = 0;
	dw2->Frag = 0;
	dw2->MaxTxTime = 0;
	dw2->FixRate = 0;
	dw3->RemainTxCnt = MT_TX_SHORT_RETRY;
	dw3->SnVld = 0;
	dw3->PnVld = 0;
	dw5->PktId = PID_DATA_AMPDU;
	dw5->TxSFmt = 0;
	dw5->TxS2Host = 0;
	dw5->BarSsnCtrl = 0; /* HW */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	dw5->PwrMgmt = TMI_PM_BIT_CFG_BY_HW;
#else
	dw5->PwrMgmt = TMI_PM_BIT_CFG_BY_SW;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */
#ifdef RTMP_MAC_PCI
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
#endif /* RTMP_PCI_SUPPORT */
	/* For short format */
	/* TODO: shiang-usw, for windows, need to change the CR mapping table for 0xc0000 to 0x800c0000 */
	HW_IO_WRITE32(pAd, 0xc0040, dw[0]);
	HW_IO_WRITE32(pAd, 0xc0044, dw[1]);
	HW_IO_WRITE32(pAd, 0xc0048, dw[2]);
	HW_IO_WRITE32(pAd, 0xc004c, dw[3]);
	HW_IO_WRITE32(pAd, 0xc0050, dw[4]);
#ifdef RTMP_MAC_PCI
	/* After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region */
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_PCI_SUPPORT */
	return TRUE;
}
#endif

INT MtAsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


VOID MtAsicUpdateRtsThld(PRTMP_ADAPTER pAd, MT_RTS_THRESHOLD_T *rts_thrld)
{
	UINT32 Value = 0;
	/* Config ASIC RTS threshold register*/
	MAC_IO_READ32(pAd, AGG_PCR1, &Value);
	Value &= ~RTS_THRESHOLD_MASK;
	Value &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	Value |= RTS_THRESHOLD(rts_thrld->pkt_len_thld);
	Value |= RTS_PKT_NUM_THRESHOLD(rts_thrld->pkt_num_thld);
	MAC_IO_WRITE32(pAd, AGG_PCR1, Value);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Set MAC register value according operation mode.
 * OperationMode AND bNonGFExist are for MM and GF Proteciton.
 * If MM or GF mask is not set, those passing argument doesn't not take effect.
 *
 * Operation mode meaning:
 * = 0 : Pure HT, no preotection.
 * = 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
 * = 0x10: No Transmission in 40M is protected.
 * = 0x11: Transmission in both 40M and 20M shall be protected
 * if (bNonGFExist)
 * we should choose not to use GF. But still set correct ASIC registers.
 * ========================================================================
 */
VOID MtAsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN MT_PROTECT_CTRL_T * Protect)
{
	UINT32 val = 0;

	MAC_IO_READ32(pAd, AGG_PCR, &val);
	/* B/G Protection */
	val &= ~ERP_PROTECTION_MASK;
	val |= ERP_PROTECTION(Protect->erp_mask);
	/* RIFS Protection */
	val &= ~RIFS_PROTECTION;

	if (Protect->rifs) {
		/* TX_RIFS_EN of RCR is enable,
		 * HW will switch to Long NAV protection,
		 * ignoring PROTECTION_MODE bit
		 */
		val |= RIFS_PROTECTION;
	}

	/* NAV Protection */
	if (Protect->long_nav)
		val &= ~PROTECTION_MODE;
	else
		val |= PROTECTION_MODE;

	/* Green Field Protectio */
	val &= ~GF_PROTECTION;

	if (Protect->gf)
		val |= GF_PROTECTION;

	/* Other HT Protection */
	val &= ~(MM_PROTECTION|BW40_PROTECTION
			 |BW80_PROTECTION|BW160_PROTECTION);

	if (Protect->mix_mode)
		val |= MM_PROTECTION;

	if (Protect->bw40)
		val |= BW40_PROTECTION;

	if (Protect->bw80)
		val |= BW80_PROTECTION;

	if (Protect->bw160)
		val |= BW160_PROTECTION;

	MAC_IO_WRITE32(pAd, AGG_PCR, val);
}




/** @} */
/** @} */

/*
 * ========================================================================
 * Routine Description:
 * Set/reset MAC registers according to bPiggyBack parameter
 *
 * Arguments:
 * pAd          - Adapter pointer
 * bPiggyBack   - Enable / Disable Piggy-Back
 *
 * Return Value:
 * None
 *
 * ========================================================================
 */
VOID MtAsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2
static INT SetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
	/* UINT32 mask, time_mask; */
	/* UINT32 Value; */
	return 0;
}

/**
 * @addtogroup ap
 * @{
 * @name AP feature HW HAL
 * @{
 */
INT MtAsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR HwBssidIdx)
{
	UINT32 timeout = 0, bitmask = 0;

	ASSERT(HwBssidIdx <= 3);
	bitmask = 0xff << (HwBssidIdx * 8);

	if (bEnable == TRUE) {
		/*
		 * each HW BSSID has its own PreTBTT interval,
		 * unit is 64us, 0x00~0xff is configurable.
		 * Base on RTMP chip experience,
		 * Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		 */
		MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		timeout |= (0x50 << (HwBssidIdx * 8));
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): HwBssidIdx=%d, PreTBTT timeout = 0x%x\n",
				 __func__, HwBssidIdx, timeout));
	} else {
		MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): HwBssidIdx=%d, PreTBTT timeout = 0x%x\n",
				 __func__, HwBssidIdx, timeout));
	}

	return TRUE;
}

#ifdef MAC_APCLI_SUPPORT
/*
 * ==========================================================================
 * Description:
 * Set BSSID of Root AP
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN UCHAR index)
{
	UINT32 val;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5]));
	val = (UINT32)((pBssid[0]) |
				   (UINT32)(pBssid[1] << 8) |
				   (UINT32)(pBssid[2] << 16) |
				   (UINT32)(pBssid[3] << 24));
	MAC_IO_WRITE32(pAd, RMAC_CB1R0, val);
	val = (UINT32)(pBssid[4]) | (UINT32)(pBssid[5] << 8) | (1 << 16);
	MAC_IO_WRITE32(pAd, RMAC_CB1R1, val);

#endif /* MAC_APCLI_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfMacs)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): No Function for HIF_MT!\n",
			 __func__, __LINE__));
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT MtAsicSetReptFuncEnableByDriver(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	RMAC_MORE_STRUC rmac_more;

	MAC_IO_READ32(pAd, RMAC_MORE, &rmac_more.word);

	if (bEnable == 0)
		rmac_more.field.muar_mode_sel = 0;
	else
		rmac_more.field.muar_mode_sel = 1;

	MAC_IO_WRITE32(pAd, RMAC_MORE, rmac_more.word);
	return TRUE;
}


VOID MtAsicInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR tempMAC[MAC_ADDR_LEN];
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	COPY_MAC_ADDR(tempMAC, pAddr);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n%s %02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
			 __func__, tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5], CliIdx));
	os_zero_mem(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = tempMAC[0] + (tempMAC[1] << 8) + (tempMAC[2] << 16) + (tempMAC[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	os_zero_mem(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = tempMAC[4];
	rmac_mcbcs1.field.addr_47_40 = tempMAC[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
	rmac_mcbcs1.field.multicast_addr_index = CliIdx;
	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);
}


VOID MtAsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;
	/* TODO: Carter, not finish yet! */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" %s.\n", __func__));
	os_zero_mem(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	os_zero_mem(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	if (CliIdx <= 15)
		rmac_mcbcs1.field.multicast_addr_index = CliIdx;/* start from idx 0 */

	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);/* clear client entry first. */
	os_zero_mem(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	os_zero_mem(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	if (CliIdx <= 15)
		rmac_mcbcs1.field.multicast_addr_index = 0x10 + CliIdx;/* start from idx 16 */

	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);/* clear rootap entry. */
}

#ifdef CONFIG_WTBL_TLV_MODE
VOID MtAsicInsertRepeaterRootEntryByWtblTlv(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	NDIS_STATUS					Status = NDIS_STATUS_SUCCESS;
	UCHAR						*pTlvBuffer = NULL;
	UCHAR						*pTempBuffer = NULL;
	UINT32						u4TotalTlvLen = 0;
	UCHAR						ucTotalTlvNumber = 0;
	CMD_WTBL_GENERIC_T		rWtblGeneric = {0};
	CMD_WTBL_RX_T				rWtblRx = {0};
	P_CMD_WTBL_GENERIC_T		prWtblGeneric = NULL;
	P_CMD_WTBL_RX_T			prWtblRx = NULL;
	RMAC_MAR0_STRUC			rmac_mcbcs0;
	RMAC_MAR1_STRUC			rmac_mcbcs1;
	/* Allocate TLV msg */
	Status = os_alloc_mem(pAd, (UCHAR **)&pTlvBuffer, MAX_BUF_SIZE_OF_WTBL_INFO);

	if ((Status != NDIS_STATUS_SUCCESS) || (pTlvBuffer == NULL))
		goto error;

	pTempBuffer = pTlvBuffer;
	u4TotalTlvLen = 0;
	ucTotalTlvNumber = 0;
	/* Query Generic and Rx structure data */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  WTBL_GENERIC,
					  sizeof(CMD_WTBL_GENERIC_T),
					  &rWtblGeneric,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  WTBL_RX,
					  sizeof(CMD_WTBL_RX_T),
					  &rWtblRx,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	CmdExtWtblUpdate(pAd, Wcid, QUERY_WTBL, pTlvBuffer, u4TotalTlvLen);
	pTempBuffer = pTlvBuffer;
	prWtblGeneric = (P_CMD_WTBL_GENERIC_T)pTempBuffer;
	pTempBuffer += sizeof(CMD_WTBL_GENERIC_T);
	prWtblRx = (P_CMD_WTBL_RX_T)pTempBuffer;
	/* Set MuarIndex and Rca1 */
	prWtblGeneric->ucMUARIndex = 0x20 + ReptCliIdx;
	prWtblRx->ucRca1 = 1;
	CmdExtWtblUpdate(pAd, Wcid, SET_WTBL, pTlvBuffer, u4TotalTlvLen);

	/* Free TLV msg */
	if (pTlvBuffer)
		os_free_mem(pTlvBuffer);

	/* Query debug information */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 DW0 Value:0x%x\n", __func__, ReptCliIdx, WtblDwQuery(pAd, Wcid, 1, 0)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 DW1 Value:0x%x\n", __func__, ReptCliIdx, WtblDwQuery(pAd, Wcid, 1, 1)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 DW2 Value:0x%x\n", __func__, ReptCliIdx, WtblDwQuery(pAd, Wcid, 1, 2)));
	/* Set RMAC_MAR0/1 */
	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = pAddr[0] + (pAddr[1] << 8) + (pAddr[2] << 16) + (pAddr[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = pAddr[4];
	rmac_mcbcs1.field.addr_47_40 = pAddr[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	if (ReptCliIdx <= 15)
		rmac_mcbcs1.field.multicast_addr_index = 0x10 + ReptCliIdx;/* start from idx 16 */

	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(Ret = %d)\n", __func__, Status));
}
#else
VOID MtAsicInsertRepeaterRootEntryByDriver(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;
	STA_TR_ENTRY *tr_entry;

	tr_entry = &pAd->MacTab.tr_entry[Wcid];
	tb_entry.wtbl_addr[0] = wtbl_ctrl->wtbl_base_addr[0] + Wcid * wtbl_ctrl->wtbl_entry_size[0];
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0], &tb_entry.wtbl_1.wtbl_1_d0.word);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 4, &tb_entry.wtbl_1.wtbl_1_d1.word);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 8, &tb_entry.wtbl_1.wtbl_1_d2.word);
	dw0->field.wm = 0;
	dw0->field.muar_idx = 0x20 + ReptCliIdx;
	dw0->field.rc_a1 = 1;
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, ReptCliIdx, tb_entry.wtbl_addr[0], dw0->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, ReptCliIdx,  tb_entry.wtbl_addr[0] + 4, dw1->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, ReptCliIdx, tb_entry.wtbl_addr[0] + 8, dw2->word));
	os_zero_mem(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = pAddr[0] + (pAddr[1] << 8) + (pAddr[2] << 16) + (pAddr[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	os_zero_mem(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = pAddr[4];
	rmac_mcbcs1.field.addr_47_40 = pAddr[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	if (ReptCliIdx <= 15)
		rmac_mcbcs1.field.multicast_addr_index = 0x10 + ReptCliIdx;/* start from idx 16 */

	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);
}
#endif /* CONFIG_WTBL_TLV_MODE */

VOID MtAsicInsertRepeaterRootEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
#ifdef CONFIG_WTBL_TLV_MODE
	MtAsicInsertRepeaterRootEntryByWtblTlv(pAd, Wcid, pAddr, ReptCliIdx));
#else
	MtAsicInsertRepeaterRootEntryByDriver(pAd, Wcid, pAddr, ReptCliIdx);
#endif /* CONFIG_WTBL_TLV_MODE */
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


BOOLEAN MtAsicSetBmcQCR(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Operation,
	IN UCHAR CrReadWrite,
	IN UCHAR HwBssidIdx,
	IN UINT32 apidx,
	IN OUT UINT32    *pcr_val)
{
	UINT32  cr_base = 0;
	UINT32  cr_val = 0;
	UCHAR   fcnt = 0;
	BOOLEAN result = FALSE;

	switch (Operation) {
	case BMC_FLUSH: {
		cr_base = ARB_BMCQCR1;

		if (HwBssidIdx > 0)
			cr_val = 1 << HwBssidIdx;
		else
			cr_val = *pcr_val;

		MAC_IO_WRITE32(pAd, cr_base, cr_val);

		/* check flush result */
		for (fcnt = 0; fcnt < 100; fcnt++) {
			MAC_IO_READ32(pAd, cr_base, &cr_val);

			if (cr_val == 0)
				break;
		}

		*pcr_val = cr_val;

		if (fcnt == 100) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: flush take too long, flush cnt=%d\n",
					  __func__,
					  fcnt)
					);
			return result;
		}
	}
	break;

	case BMC_ENABLE: {
		cr_base = ARB_BMCQCR0;

		if (HwBssidIdx > 0)
			cr_val = 1 << HwBssidIdx;
		else
			cr_val = *pcr_val;

		MAC_IO_WRITE32(pAd, cr_base, cr_val);
	}
	break;

	case BMC_CNT_UPDATE: {
		if (HwBssidIdx > 0)
			cr_base = ARB_BMCQCR2;
		else {
			if (apidx <= 4)
				cr_base = ARB_BMCQCR2;
			else if ((apidx >= 5) && (apidx <= 12))
				cr_base = ARB_BMCQCR3;
			else if ((apidx >= 13) && (apidx <= 15))
				cr_base = ARB_BMCQCR4;
			else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: apidx(%d) not support\n", __func__, apidx));
				return result;
			}
		}

		if (CrReadWrite == CR_READ) {
			MAC_IO_READ32(pAd, cr_base, &cr_val);
			*pcr_val = cr_val;
		} else if (CrReadWrite == CR_WRITE) {
			cr_val = 0x01;/* add 1 to cnt. */

			if (HwBssidIdx > 0)
				MAC_IO_WRITE32(pAd, cr_base, (cr_val << ((HwBssidIdx) * 4)));
			else {
				if (apidx == 0)
					MAC_IO_WRITE32(pAd, cr_base, cr_val);
				else if (apidx >= 1 && apidx <= 4)
					MAC_IO_WRITE32(pAd, cr_base, (cr_val << (16 + ((apidx - 1) * 4))));
				else if (apidx >= 5 && apidx <= 12)
					MAC_IO_WRITE32(pAd, cr_base, (cr_val << ((apidx - 5) * 4)));
				else if (apidx >= 13 && apidx <= 15)
					MAC_IO_WRITE32(pAd, cr_base, (cr_val << ((apidx - 13) * 4)));
			}
		}
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): Operation Error (%d)\n",
				  __func__,
				  Operation)
				);
		return result;
	}

	result = TRUE;
	return result;
}

BOOLEAN MtAsicSetBcnQCR(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Operation,
	IN UCHAR HwBssidIdx,
	IN UINT32 apidx)
{
	UINT32  cr_base = 0;
	UINT32  cr_val = 0, temp = 0;
	UINT16  fcnt = 0;
	BOOLEAN result = FALSE;

	switch (Operation) {
	case BCN_FLUSH: {
		cr_base = ARB_BCNQCR1;

		if (HwBssidIdx == 0) {
			if (apidx > 0)
				cr_val = cr_val | (1 << (apidx+15));
			else
				cr_val = 1;
		} else if (HwBssidIdx > 0)
			cr_val = 1 << HwBssidIdx;

		MAC_IO_WRITE32(pAd, cr_base, cr_val);

		while (1) {
			/* check bcn_flush cr status */
			MAC_IO_READ32(pAd, cr_base, &temp);

			if (temp & cr_val) {
				fcnt++;
				os_msec_delay(1);

				if (fcnt > 1000) {
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>  Apidx = %x, flush bcnQ too long!!\n", apidx));
					return result;
				}
			} else
				break;
		}
	}
	break;

	case BCN_ENABLE: {
		cr_base = ARB_BCNQCR0;

		if (HwBssidIdx == 0) {
			if (apidx > 0)
				cr_val = cr_val | (1 << (apidx+15));
			else
				cr_val = 1;
		} else if (HwBssidIdx > 0)
			cr_val = 1 << HwBssidIdx;

		MAC_IO_WRITE32(pAd, cr_base, cr_val);
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): Operation Error (%d)\n",
				  __func__,
				  Operation)
				);
		return result;
	}

	result = TRUE;
	return result;
}

typedef struct _SYNC_MODE_CR_TABLE_T {
	UINT32              u4ArbOpModeCR;
	UINT32              u4ArbAifsCR;
	UINT32              u4ArbCwminCR;
	UINT32              u4LponMacTimerCr;
	UINT32              u4LponTbttCtrlCR;
	UINT32              u4LponPreTbttTime;/* set pretbtt time */
	UINT32              u4LponSyncModeCR;/* sync mode CR*/
	UINT32              u4IntEnableCR;
} SYNC_MODE_CR_TABLE_T, *PSYNC_MODE_CR_TABLE_T;

static SYNC_MODE_CR_TABLE_T g_arDisableSyncModeMapTable[HW_BSSID_MAX] = {
	/*WMM cr set band0 first, change it when is checked it link to Band1. */
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR1, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR1, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR3, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR3, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicDisableSyncByDriver(RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
	UINT32 value = 0;

	if (HWBssidIdx > HW_BSSID_MAX) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("--->%s(): HW OmacIdx:%x passed in is not correct\n",
				  __func__, HWBssidIdx));
		return;
	}

	cr_set = g_arDisableSyncModeMapTable[HWBssidIdx];

	/*1. disable hif interrupt pin*/
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		MAC_IO_READ32(pAd, cr_set.u4IntEnableCR, &value);

		switch (HWBssidIdx) {
		case HW_BSSID_0:
			value &= ~HWIER3_TBTT0;
			value &= ~HWIER3_PRETBTT0;
			break;

		case HW_BSSID_1:
			value &= ~HWIER0_TBTT1;
			value &= ~HWIER0_PRETBTT1;
			break;

		case HW_BSSID_2:
			value &= ~HWIER0_TBTT2;
			value &= ~HWIER0_PRETBTT2;
			break;

		case HW_BSSID_3:
			value &= ~HWIER0_TBTT3;
			value &= ~HWIER0_PRETBTT3;
			break;

		default:
			ASSERT(HWBssidIdx < HW_BSSID_MAX);
			break;
		}

		MAC_IO_WRITE32(pAd, cr_set.u4IntEnableCR, value);
	}

	/*2. disable BeaconPeriodEn */
	MAC_IO_READ32(pAd, cr_set.u4LponTbttCtrlCR, &value);
	value &= ~TBTTn_CAL_EN;
	MAC_IO_WRITE32(pAd, cr_set.u4LponTbttCtrlCR, value);
	/*3. disable MPTCR pin*/
	/*NOTE: disable is write another CR at the same bit to disable. */
	value = 0;
	value |= (TBTT_TIMEUP_EN |
			  TBTT_PERIOD_TIMER_EN |
			  PRETBTT_TIMEUP_EN |
			  PRETBTT_INT_EN);

	if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
		value = (value << 8);

	MAC_IO_WRITE32(pAd, cr_set.u4LponMacTimerCr, value);
	/*4. recover BCN AIFS, CWmin, and HW TSF sync mode.*/
	MAC_IO_READ32(pAd, cr_set.u4LponSyncModeCR, &value);
	value = value & TSF_TIMER_HW_MODE_FULL;
	MAC_IO_WRITE32(pAd, cr_set.u4LponSyncModeCR, value);
	MAC_IO_READ32(pAd, cr_set.u4ArbAifsCR, &value);
	value &= ~ARB_AIFSR1_BCN_AIFS_MASK;
	value |= ARB_AIFSR1_BCN_AIFS_HW_DEFAULT;
	MAC_IO_WRITE32(pAd, cr_set.u4ArbAifsCR, value);
	MAC_IO_READ32(pAd, cr_set.u4ArbCwminCR, &value);
	value &= ~ARB_ACCWIR1_BCN_CWMIN_MASK;
	value |= ARB_ACCWIR1_BCN_CWMIN_HW_DEFAULT;
	MAC_IO_WRITE32(pAd, cr_set.u4ArbCwminCR, value);
	/*5. set ARB OPMODE */
	MAC_IO_READ32(pAd, cr_set.u4ArbOpModeCR, &value);
	value &= ~(MT_ARB_SCR_OPMODE_MASK << (HWBssidIdx * 2));/* clean opmode */
	MAC_IO_WRITE32(pAd, cr_set.u4ArbOpModeCR, value);
}


static SYNC_MODE_CR_TABLE_T g_arEnableSyncModeMapTable[HW_BSSID_MAX] = {
	/*WMM cr set band0 first, change it when is checked it link to Band1. */
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR0, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR0, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR2, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
	{ARB_SCR, ARB_AIFSR0, ARB_ACCWIR1, LPON_MPTCR2, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

VOID MtAsicEnableBssSyncByDriver(
	RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
	UINT32 value = 0;

	if (HWBssidIdx > HW_BSSID_MAX) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("--->%s(): HW OmacIdx:%x passed in is not correct\n",
				  __func__, HWBssidIdx));
		return;
	}

	cr_set = g_arEnableSyncModeMapTable[HWBssidIdx];
	/*1. set ARB OPMODE */
	MAC_IO_READ32(pAd, cr_set.u4ArbOpModeCR, &value);

	if (OPMode == OPMODE_AP) {
		value |=  (MT_ARB_SCR_BM_CTRL |
				   MT_ARB_SCR_BCN_CTRL |
				   MT_ARB_SCR_BCN_EMPTY);
	}

	value |= (OPMode << (HWBssidIdx * 2));
	MAC_IO_WRITE32(pAd, cr_set.u4ArbOpModeCR, value);
		if (OPMode == OPMODE_AP) {
			MAC_IO_READ32(pAd, cr_set.u4LponSyncModeCR, &value);
			value = value | TSF_TIMER_HW_MODE_TICK_ONLY;
			MAC_IO_WRITE32(pAd, cr_set.u4LponSyncModeCR, value);
		}

	/*3. set Pretbtt time. */
	MAC_IO_READ32(pAd, cr_set.u4LponPreTbttTime, &value);
	value |= DEFAULT_PRETBTT_INTERVAL_IN_MS << (HWBssidIdx * 8);
	MAC_IO_WRITE32(pAd, cr_set.u4LponPreTbttTime, value);
	/*4. set MPTCR */
	value = 0;
	value |= (TBTT_TIMEUP_EN |
			  TBTT_PERIOD_TIMER_EN |
			  PRETBTT_TIMEUP_EN |
			  PRETBTT_INT_EN);

	if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
		value = (value << 8);

	MAC_IO_WRITE32(pAd, cr_set.u4LponMacTimerCr, value);
	/*5. set BeaconPeriod */
	value = 0;
	{
		value |= BEACONPERIODn(BeaconPeriod);
	}

	value |= TBTTn_CAL_EN;
	MAC_IO_WRITE32(pAd, cr_set.u4LponTbttCtrlCR, value);
#ifndef BCN_OFFLOAD_SUPPORT

	/*6. enable HOST interrupt pin.*/
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		MAC_IO_READ32(pAd, cr_set.u4IntEnableCR, &value);

		switch (HWBssidIdx) {
		case HW_BSSID_0:
			value |= HWIER3_TBTT0;
			value |= HWIER3_PRETBTT0;
			break;

		case HW_BSSID_1:
			value |= HWIER0_TBTT1;
			value |= HWIER0_PRETBTT1;
			break;

		case HW_BSSID_2:
			value |= HWIER0_TBTT2;
			value |= HWIER0_PRETBTT2;
			break;

		case HW_BSSID_3:
			value |= HWIER0_TBTT3;
			value |= HWIER0_PRETBTT3;
			break;

		default:
			ASSERT(HWBssidIdx < HW_BSSID_MAX);
			break;
		}

		MAC_IO_WRITE32(pAd, cr_set.u4IntEnableCR, value);
	}

#endif /* BCN_OFFLOAD_SUPPORT */
}

INT MtAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return SetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


INT MtAsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return TRUE;
}


typedef struct _RTMP_WMM_PAIR {
	UINT32 Address;
	UINT32 Mask;
	UINT32 Shift;
} RTMP_WMM_PAIR, *PRTMP_WMM_PAIR;


static RTMP_WMM_PAIR wmm_txop_mask[] = {
	{TMAC_ACTXOPLR1, 0x0000ffff, 0}, /* AC0 - BK */
	{TMAC_ACTXOPLR1, 0xffff0000, 16}, /* AC1 - BE */
	{TMAC_ACTXOPLR0, 0x0000ffff, 0}, /* AC2 - VI */
	{TMAC_ACTXOPLR0, 0xffff0000, 16}, /* AC3 - VO */
};


static RTMP_WMM_PAIR wmm_aifsn_mask[] = {
	{ARB_AIFSR0, 0x0000000f, 0}, /* AC0 - BK */
	{ARB_AIFSR0, 0x000000f0, 4}, /* AC1 - BE */
	{ARB_AIFSR0, 0x00000f00, 8}, /* AC2  - VI */
	{ARB_AIFSR0, 0x0000f000, 12}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmin_mask[] = {
	{ARB_ACCWIR0, 0x000000ff, 0}, /* AC0 - BK */
	{ARB_ACCWIR0, 0x0000ff00, 8}, /* AC1 - BE */
	{ARB_ACCWIR0, 0x00ff0000, 16}, /* AC2  - VI */
	{ARB_ACCWIR0, 0xff000000, 24}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmax_mask[] = {
	{ARB_ACCWXR0, 0x0000ffff, 0}, /* AC0 - BK */
	{ARB_ACCWXR0, 0xffff0000, 16}, /* AC1 - BE */
	{ARB_ACCWXR1, 0x0000ffff, 0}, /* AC2  - VI */
	{ARB_ACCWXR1, 0xffff0000, 16}, /* AC3 - VO */
};


/**
 * @addtogroup wmm
 * @{
 * @name WMM HW HAL
 * @{
 */
UINT32 MtAsicGetWmmParam(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 addr = 0, cr_val, mask = 0, shift = 0;

	if (AcNum <= WMM_PARAM_AC_3) {
		switch (EdcaType) {
		case WMM_PARAM_TXOP:
			addr = wmm_txop_mask[AcNum].Address;
			mask = wmm_txop_mask[AcNum].Mask;
			shift = wmm_txop_mask[AcNum].Shift;
			break;

		case WMM_PARAM_AIFSN:
			addr = wmm_aifsn_mask[AcNum].Address;
			mask = wmm_aifsn_mask[AcNum].Mask;
			shift = wmm_aifsn_mask[AcNum].Shift;
			break;

		case WMM_PARAM_CWMIN:
			addr = wmm_cwmin_mask[AcNum].Address;
			mask = wmm_cwmin_mask[AcNum].Mask;
			shift = wmm_cwmin_mask[AcNum].Shift;
			break;

		case WMM_PARAM_CWMAX:
			addr = wmm_cwmax_mask[AcNum].Address;
			mask = wmm_cwmax_mask[AcNum].Mask;
			shift = wmm_cwmax_mask[AcNum].Shift;
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __func__, __LINE__, EdcaType));
			break;
		}
	}

	if (addr && mask) {
		MAC_IO_READ32(pAd, addr, &cr_val);
		cr_val = (cr_val & mask) >> shift;
		return cr_val;
	}

	return 0xdeadbeef;
}



INT MtAsicSetWmmParam(RTMP_ADAPTER *pAd, UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue)
{
	CMD_EDCA_SET_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;

	os_zero_mem(&EdcaParam, sizeof(CMD_EDCA_SET_T));
	EdcaParam.ucTotalNum = 1;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)AcNum;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop = (UINT16)EdcaValue;
		break;

	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax = (UINT16)EdcaValue;
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __func__, __LINE__, EdcaType));
		break;
	}

#if defined(COMPOS_TESTMODE_WIN)
	;/* no function */
#else
	MtCmdEdcaParameterSet(pAd, EdcaParam);
#endif
	return TRUE;
}


VOID MtAsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm)
{
	CMD_EDCA_SET_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac = 0, index = 0;

	os_zero_mem(&EdcaParam, sizeof(CMD_EDCA_SET_T));

	if ((pEdcaParm != NULL)  &&  (pEdcaParm->bValid != FALSE)) {
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;

		for (ac = 0; ac < CMD_EDCA_AC_MAX;  ac++) {
			index = wmm_aci_2_hw_ac_queue[ac];
			pAcParam = &EdcaParam.rAcParam[index];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum = (UINT8)ac;
			pAcParam->ucAifs = pEdcaParm->Aifsn[index];
			pAcParam->ucWinMin = pEdcaParm->Cwmin[index];
			pAcParam->u2WinMax = pEdcaParm->Cwmax[index];
			pAcParam->u2Txop = pEdcaParm->Txop[index];
		}
	}

#if defined(COMPOS_TESTMODE_WIN)
	;/* no function */
#else
	MtCmdEdcaParameterSet(pAd, EdcaParam);
#endif
}

/** @} */
/** @} */

INT MtAsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


UINT32 MtAsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return 0;
}


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "10M"};
char *get_bw_str(int bandwidth)
{
	if (bandwidth >= BW_20 && bandwidth <= BW_80)
		return phy_bw_str[bandwidth];
	else if (bandwidth == BW_10)
		return phy_bw_str[3];
	else
		return "N/A";
}

/**
 * @addtogroup auto_rate
 * @{
 * @name Auto Rate HW HAL
 * @{
 */



INT MtAsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


INT32 MtAsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, AGG_ARUCR, &Value);
	Value &= ~RATE1_UP_MPDU_LIMIT_MASK;
	Value |= RATE1_UP_MPDU_LINIT(2);
	Value &= ~RATE2_UP_MPDU_LIMIT_MASK;
	Value |= RATE2_UP_MPDU_LIMIT(2);
	Value &= ~RATE3_UP_MPDU_LIMIT_MASK;
	Value |= RATE3_UP_MPDU_LIMIT(2);
	Value &= ~RATE4_UP_MPDU_LIMIT_MASK;
	Value |= RATE4_UP_MPDU_LIMIT(2);
	Value &= ~RATE5_UP_MPDU_LIMIT_MASK;
	Value |= RATE5_UP_MPDU_LIMIT(1);
	Value &= ~RATE6_UP_MPDU_LIMIT_MASK;
	Value |= RATE6_UP_MPDU_LIMIT(1);
	Value &= ~RATE7_UP_MPDU_LIMIT_MASK;
	Value |= RATE7_UP_MPDU_LIMIT(1);
	Value &= ~RATE8_UP_MPDU_LIMIT_MASK;
	Value |= RATE8_UP_MPDU_LIMIT(1);
	MAC_IO_WRITE32(pAd, AGG_ARUCR, Value);
	MAC_IO_READ32(pAd, AGG_ARDCR, &Value);
	Value &= ~RATE1_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE1_DOWN_MPDU_LIMIT(2);
	Value &= ~RATE2_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE2_DOWN_MPDU_LIMIT(2);
	Value &= ~RATE3_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE3_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE4_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE4_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE5_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE5_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE6_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE6_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE7_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE7_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE8_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE8_DOWN_MPDU_LIMIT(0);
	MAC_IO_WRITE32(pAd, AGG_ARDCR, Value);
	MAC_IO_READ32(pAd, AGG_ARCR, &Value);
	Value |= INI_RATE1;
	Value &= ~RTS_RATE_DOWN_TH_MASK;
	Value &= ~RATE_DOWN_EXTRA_RATIO_MASK;
	Value |= RATE_DOWN_EXTRA_RATIO(1);
	Value |= RATE_DOWN_EXTRA_RATIO_EN;
	Value &= ~RATE_UP_EXTRA_TH_MASK;
	Value |= RATE_UP_EXTRA_TH(4);
	MAC_IO_WRITE32(pAd, AGG_ARCR, Value);
	return TRUE;
}



/**
 * Wtbl2RateTableUpdate
 *
 *
 *
 */
/* #ifdef CONFIG_WTBL_TLV_MODE */
VOID MtAsicTxCapAndRateTableUpdateByDriver(RTMP_ADAPTER *pAd, UCHAR ucWcid, RA_PHY_CFG_T *prTxPhyCfg, UINT32 *Rate, BOOL fgSpeEn)
{
	union WTBL_2_DW9 wtbl_2_d9;
	UINT32 u4RegVal;
	UCHAR bw;

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	wtbl_2_d9.word = 0;

	switch (prTxPhyCfg->BW) {
	case BW_80:
		bw = 2;
		break;

	case BW_40:
		bw = 1;
		break;

	case BW_20:

	/* case BW_10: */
	default:
		bw = 0;
		break;
	}

	wtbl_2_d9.field.fcap = bw;
	wtbl_2_d9.field.ccbw_sel = bw;
	wtbl_2_d9.field.cbrn = 7; /* change bw as (fcap/2) if rate_idx > 7, temporary code */

	if (prTxPhyCfg->ShortGI) {
		wtbl_2_d9.field.g2 = 1;
		wtbl_2_d9.field.g4 = 1;
		wtbl_2_d9.field.g8 = 1;
		wtbl_2_d9.field.g16 = 1;
	} else {
		wtbl_2_d9.field.g2 = 0;
		wtbl_2_d9.field.g4 = 0;
		wtbl_2_d9.field.g8 = 0;
		wtbl_2_d9.field.g16 = 0;
	}

	wtbl_2_d9.field.rate_idx = 0;

	if (fgSpeEn == TRUE)
		wtbl_2_d9.field.spe_en = 1;

	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR0, wtbl_2_d9.word);
	u4RegVal = (Rate[0] | (Rate[1] << 12) | (Rate[2] << 24));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR1, u4RegVal);
	u4RegVal = ((Rate[2] >> 8) | (Rate[3] << 4) | (Rate[4] << 16) | (Rate[5] << 28));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR2, u4RegVal);
	u4RegVal = ((Rate[5] >> 4) | (Rate[6] << 8) | (Rate[7] << 20));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR3, u4RegVal);
	/* TODO: shiang-MT7603, shall we also clear TxCnt/RxCnt/AdmCnt here?? */
	u4RegVal = (ucWcid | (1 << 13) | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}

VOID MtAsicTxCapAndRateTableUpdate(RTMP_ADAPTER *pAd, UCHAR ucWcid, RA_PHY_CFG_T *prTxPhyCfg, UINT32 *Rate, BOOL fgSpeEn)
{
	/* #ifdef CONFIG_WTBL_TLV_MODE */
	MtAsicTxCapAndRateTableUpdateByDriver(pAd, ucWcid, prTxPhyCfg, Rate, fgSpeEn);
}


/**
 * Wtbl2TxRateCounterGet
 *
 *
 *
 */

static VOID Wtbl2TxRateCounterGetByDriver(RTMP_ADAPTER *pAd, UCHAR ucWcid, TX_CNT_INFO *tx_cnt_info)
{
	UINT32 u4RegVal;
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UCHAR wtbl_idx;
	UINT32 addr;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (ucWcid < wtbl_ctrl->wtbl_entry_cnt[0] ? ucWcid : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __func__));
		return;
	}

	addr = pAd->mac_ctrl.wtbl_base_addr[1] + wtbl_idx * pAd->mac_ctrl.wtbl_entry_size[1];
	HW_IO_READ32(pAd, addr + 5 * 4, &(tx_cnt_info->wtbl_2_d5.word));
	HW_IO_READ32(pAd, addr + 6 * 4, &(tx_cnt_info->wtbl_2_d6.word));
	HW_IO_READ32(pAd, addr + 7 * 4, &(tx_cnt_info->wtbl_2_d7.word));
	HW_IO_READ32(pAd, addr + 8 * 4, &(tx_cnt_info->wtbl_2_d8.word));
	HW_IO_READ32(pAd, addr + 9 * 4, &(tx_cnt_info->wtbl_2_d9.word));

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	u4RegVal = (ucWcid | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}

static VOID Wtbl2TxRateCounterGet(RTMP_ADAPTER *pAd, UCHAR ucWcid, TX_CNT_INFO *tx_cnt_info)
{
	/* #ifdef CONFIG_WTBL_TLV_MODE */
	Wtbl2TxRateCounterGetByDriver(pAd, ucWcid, tx_cnt_info);
}

/*
 * Wtbl2RcpiGet
 *
 *
 *
 */


VOID MtAsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo)
{
	TX_CNT_INFO tx_cnt_info;

	Wtbl2TxRateCounterGet(pAd, (UINT8)Wcid, &tx_cnt_info);
	pTxInfo->TxCount = tx_cnt_info.wtbl_2_d7.field.current_bw_tx_cnt;
	pTxInfo->TxCount += tx_cnt_info.wtbl_2_d8.field.other_bw_tx_cnt;
	pTxInfo->TxFailCount = tx_cnt_info.wtbl_2_d7.field.current_bw_fail_cnt;
	pTxInfo->TxFailCount += tx_cnt_info.wtbl_2_d8.field.other_bw_fail_cnt;
	pTxInfo->Rate1TxCnt = (UINT16)tx_cnt_info.wtbl_2_d5.field.rate_1_tx_cnt;
	pTxInfo->Rate1FailCnt = (UINT16)tx_cnt_info.wtbl_2_d5.field.rate_1_fail_cnt;
	pTxInfo->Rate2TxCnt = (CHAR)tx_cnt_info.wtbl_2_d6.field.rate_2_tx_cnt;
	pTxInfo->Rate3TxCnt = (CHAR)tx_cnt_info.wtbl_2_d6.field.rate_3_tx_cnt;
	pTxInfo->Rate4TxCnt = (CHAR)tx_cnt_info.wtbl_2_d6.field.rate_4_tx_cnt;
	pTxInfo->Rate5TxCnt = (CHAR)tx_cnt_info.wtbl_2_d6.field.rate_5_tx_cnt;
	pTxInfo->RateIndex = (CHAR)tx_cnt_info.wtbl_2_d9.field.rate_idx;
}


VOID MtAsicRssiGet(RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR *RssiSet)
{
	/*union WTBL_2_DW13 wtbl_2_d13;*/
	return;
}


VOID MtAsicRcpiReset(RTMP_ADAPTER *pAd, UCHAR ucWcid)
{
	UINT32 u4RegVal;

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	u4RegVal = (ucWcid | (1 << 15));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}

/** @} */
/** @} */


#define MT_MCAST_WCID 0x0

#ifndef CONFIG_WTBL_TLV_MODE
static BOOLEAN WtblClearCounter(RTMP_ADAPTER *pAd, UINT8 Wcid)
{
	UINT32 Value;
	/* RX Counter Clear */
	MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
	Value &= ~WLAN_IDX_MASK;
	Value |= WLAN_IDX(Wcid);
	Value |= RX_CNT_CLEAR;
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return FALSE;

	/* TX Counter Clear */
	MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
	Value &= ~WLAN_IDX_MASK;
	Value |= WLAN_IDX(Wcid);
	Value |= TX_CNT_CLEAR;
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return FALSE;

	/* Admission Control Counter Clear */
	MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
	Value &= ~WLAN_IDX_MASK;
	Value |= WLAN_IDX(Wcid);
	Value |= ADM_CNT_CLEAR;
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return FALSE;

	return TRUE;
}
#endif


/**
 * @addtogroup wtbl
 * @{
 * @name Wtbl HW HAL
 * @{
 */
VOID MtAsicUpdateRxWCIDTableByDriver(RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo)
{
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;
	UINT32 Value, Index;

	os_zero_mem(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, (UCHAR)WtblInfo.Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
				 __func__, WtblInfo.Wcid));
		return;
	}

	dw0->field.wm = 0;

	if (WtblInfo.WcidType == MT_WCID_TYPE_BMCAST) {
		dw0->field.muar_idx = 0xe;
		dw2->field.adm = 0;
		dw2->field.cipher_suit = WTBL_CIPHER_NONE;
	} else if (WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST) {
		dw0->field.muar_idx = 0xe;
		dw0->field.rv = 1;
		dw2->field.adm = 0;
		dw2->field.cipher_suit = WTBL_CIPHER_NONE;
		dw0->field.rc_a2 = 1;
	} else {
		dw0->field.muar_idx = WtblInfo.MacAddrIdx;

		if ((WtblInfo.WcidType == MT_WCID_TYPE_APCLI) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_REPEATER))
			dw0->field.rc_a1 = 1;

		/*Windows not enable RV bit, why?*/
		dw0->field.rv = 1;
		dw0->field.rc_a2 = 1;
		/* TODO: shiang-MT7603, in which case we need to check A1??? */
		/* dw0->field.rc_a1 = 1; */
		dw2->field.adm = 1;
		dw2->field.qos = (WtblInfo.SupportQoS) ? 1:0;
		dw2->field.cipher_suit = WtblInfo.CipherSuit;
		dw0->field.rkv = (WtblInfo.CipherSuit != WTBL_CIPHER_NONE) ? 1:0;
		dw4->field.partial_aid = WtblInfo.Aid;
#ifdef HDR_TRANS_TX_SUPPORT

		switch (WtblInfo.WcidType) {
		case MT_WCID_TYPE_AP:
			dw2->field.td = 1;
			dw2->field.fd = 0;
			break;

		case MT_WCID_TYPE_CLI:
			dw2->field.td = 1;
			dw2->field.fd = 0;
			break;

		case MT_WCID_TYPE_APCLI:
		case MT_WCID_TYPE_REPEATER:
			dw2->field.td = 1;
			dw2->field.fd = 0;
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: Unknown entry type(%d) do not support header translation\n", __func__, WtblInfo.WcidType));
			break;
		}

#endif

		if (WtblInfo.SupportHT) {
			dw2->field.ht = 1;

			if (WtblInfo.SupportRDG) {
				dw2->field.r = 1;
				dw2->field.rdg_ba = 1;
			}

			if (WtblInfo.SmpsMode == MMPS_DYNAMIC)
				dw2->field.smps = 1;
			else
				dw2->field.smps = 0;

			if (WtblInfo.SupportVHT)
				dw2->field.vht = 1;
		}
	}

	dw0->field.addr_4 = WtblInfo.Addr[4];
	dw0->field.addr_5 = WtblInfo.Addr[5];
	dw1->word = WtblInfo.Addr[0] | (WtblInfo.Addr[1] << 8)  | (WtblInfo.Addr[2] << 16)  | (WtblInfo.Addr[3] << 24);
	dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
	dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
	dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
	dw3->field.psm = 0;
	dw3->field.i_psm = 1;
	dw3->field.du_i_psm = 1;
	dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
	dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
	dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, WtblInfo.Wcid, tb_entry.wtbl_addr[0], dw0->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, WtblInfo.Wcid,  tb_entry.wtbl_addr[0] + 4, dw1->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, WtblInfo.Wcid, tb_entry.wtbl_addr[0] + 8, dw2->word));
	MAC_IO_READ32(pAd, WTBL1OR, &Value);
	Value |= PSM_W_FLAG;
	MAC_IO_WRITE32(pAd, WTBL1OR, Value);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, WtblInfo.Wcid, tb_entry.wtbl_addr[0] + 12, dw3->word));
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 16, dw4->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
			 __func__, WtblInfo.Wcid, tb_entry.wtbl_addr[0] + 16, dw4->word));
	MAC_IO_READ32(pAd, WTBL1OR, &Value);
	Value &= ~PSM_W_FLAG;
	MAC_IO_WRITE32(pAd, WTBL1OR, Value);
	/* Clear BA Information */
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (15 * 4), tb_entry.wtbl_2.wtbl_2_d15.word);
	/* Clear BA Information */
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (15 * 4), tb_entry.wtbl_2.wtbl_2_d15.word);

	/*Clear Wtbl Counter*/
	if (WtblClearCounter(pAd, (UCHAR)WtblInfo.Wcid) != TRUE)
		return;

	/*Must after RX counter clear*/

	if (WtblInfo.WcidType != MT_WCID_TYPE_BMCAST && WtblInfo.WcidType !=  MT_WCID_TYPE_APCLI_MCAST && WtblInfo.SupportHT) {
		{
			dw2->field.mm = WtblInfo.MpduDensity;
			dw2->field.af = WtblInfo.MaxRAmpduFactor;
			HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
		}
	}

	/* Clear Cipher Key */
	for (Index = 0; Index < 8; Index++)
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[2] + (4 * Index), 0x0);
}

VOID MtAsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo)
{
	MtAsicUpdateRxWCIDTableByDriver(pAd, WtblInfo);
}



/*
 * ==========================================================================
 * Description:
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicDelWcidTabByDriver(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
	UCHAR cnt, cnt_s, cnt_e;
	struct wtbl_entry tb_entry;
	UINT32 Index = 0, Value;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():wcid_idx=0x%x\n",
			 __func__, wcid_idx));

	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WCID_ALL - 1);
	} else
		cnt_s = cnt_e = wcid_idx;

	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++) {
		cnt = cnt_s;
		os_zero_mem(&tb_entry, sizeof(tb_entry));

		if (mt_wtbl_get_entry234(pAd, cnt, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					 __func__, cnt));
			return;
		}

		dw0->field.wm = 0;
		dw0->field.rc_a2 = 1;
		dw0->field.rv = 1;
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], tb_entry.wtbl_1.wtbl_1_d0.word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, tb_entry.wtbl_1.wtbl_1_d1.word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, tb_entry.wtbl_1.wtbl_1_d2.word);
		MAC_IO_READ32(pAd, WTBL1OR, &Value);
		Value |= PSM_W_FLAG;
		MAC_IO_WRITE32(pAd, WTBL1OR, Value);
		dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
		dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
		dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
		dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
		dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
		dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 16, dw4->word);
	MAC_IO_READ32(pAd, WTBL1OR, &Value);
	Value &= ~PSM_W_FLAG;
	MAC_IO_WRITE32(pAd, WTBL1OR, Value);
	/* Clear BA Information */
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (15 * 4), tb_entry.wtbl_2.wtbl_2_d15.word);

	/* Clear Cipher Key */
	for (Index = 0; Index < 16; Index++)
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[2] + (4 * Index), 0x0);

	WtblClearCounter(pAd, wcid_idx);
}
}

/** @} */
/** @} */
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
VOID MtAsicAddRemoveKeyByDriver(PRTMP_ADAPTER pAd, ASIC_SEC_INFO *pInfo)
{
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	UINT32 addr = 0, index = 0;
	UCHAR KeyLen = 0;
	UCHAR Key[64];
	UINT32 *pKeyData = (UINT32 *) Key;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Wcid[%d]. Operation=%d, Direction=%d\n", __func__, pInfo->Wcid, pInfo->Operation, pInfo->Direction));
	os_zero_mem(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, pInfo->Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
				 __func__, pInfo->Wcid));
		return;
	}

	addr = pAd->mac_ctrl.wtbl_base_addr[2] + pInfo->Wcid * pAd->mac_ctrl.wtbl_entry_size[2];
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0], &dw0->word);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+4, &dw1->word);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+8, &dw2->word);
	dw0->field.wm = 0;
	dw0->field.addr_4 = pInfo->PeerAddr[4];
	dw0->field.addr_5 = pInfo->PeerAddr[5];
	dw1->word = pInfo->PeerAddr[0] +
				(pInfo->PeerAddr[1] << 8) +
				(pInfo->PeerAddr[2] << 16) +
				(pInfo->PeerAddr[3] << 24);
	os_zero_mem(Key, sizeof(Key));

	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		dw0->field.rv = 0;
		dw0->field.rkv = 0;
		dw2->field.cipher_suit = 0;
		dw0->field.kid = 0;
		dw2->field.wpi_even = 0;
		KeyLen = 64; /* Clear cipher: 16 DWs */
	} else {
		/* Add Key */
		UCHAR KeyId = pInfo->KeyIdx;
		SEC_KEY_INFO *pSecKey = &pInfo->Key;

		KeyLen = pSecKey->KeyLen;
		os_move_mem(Key, pSecKey->Key, pSecKey->KeyLen);

		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			addr = KeyId*16 + addr;

			if (KeyLen == 5)
				dw2->field.cipher_suit = CIPHER_SUIT_WEP_40;
			else if (KeyLen == 13)
				dw2->field.cipher_suit = CIPHER_SUIT_WEP_104;
			else if (KeyLen == 16)
				dw2->field.cipher_suit = CIPHER_SUIT_WEP_128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			dw2->field.cipher_suit = CIPHER_SUIT_TKIP_W_MIC;
			os_move_mem(&Key[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&Key[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			dw2->field.cipher_suit = CIPHER_SUIT_CCMP_W_MIC;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			dw2->field.cipher_suit = CIPHER_SUIT_CCMP_256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			dw2->field.cipher_suit = CIPHER_SUIT_GCMP_128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			dw2->field.cipher_suit = CIPHER_SUIT_GCMP_256;
		else if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher)) {
			addr = addr + 16;
			dw2->field.cipher_suit = CIPHER_SUIT_BIP;
		}

		else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support Cipher[0x%x]\n",
					 __func__, __LINE__, pInfo->Cipher));
			return;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_TX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			dw0->field.rv = 0;
			dw0->field.rkv = 0;
			dw0->field.rc_a1 = 0;
			dw0->field.rc_a2 = 0;
			dw0->field.rc_id = 0;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_RX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			dw0->field.rv = 1;
			dw0->field.rkv = 1;
			dw0->field.rc_a1 = 1;
			dw0->field.rc_a2 = 1;
			dw0->field.rc_id = 1;

			if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher))
				dw0->field.ikv = 1;
		}

		dw0->field.kid = KeyId;
	}

	hex_dump("key", Key, KeyLen);

	for (index = 0; index < KeyLen; index += 4) {
		pKeyData = (UINT32 *) &Key[index];
#ifdef RT_BIG_ENDIAN
		*pKeyData = SWAP32(*pKeyData);
#endif /* RT_BIG_ENDIAN */
		HW_IO_WRITE32(pAd, addr+index, *pKeyData);
	}

	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(wcid =%d)\n",
			 __func__, pInfo->Wcid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tWrite WTBL 1 Addr:0x%x, Value:0x%x\n",
			 tb_entry.wtbl_addr[0], dw0->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tWrite WTBL 1 Addr:0x%x, Value:0x%x\n",
			 tb_entry.wtbl_addr[0] + 4, dw1->word));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tWrite WTBL 1 Addr:0x%x, Value:0x%x\n",
			 tb_entry.wtbl_addr[0] + 8, dw2->word));
}
#endif

/*
 * ========================================================================
 * Description:
 * Add Pair-wise key material into ASIC.
 * Update pairwise key, TxMic and RxMic to Asic Pair-wise key table
 *
 * Return:
 * ========================================================================
 */
VOID MtAsicAddPairwiseKeyEntry(RTMP_ADAPTER *pAd, UCHAR WCID, CIPHER_KEY *pKey)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));





/*
 * ========================================================================
 * Description:
 * Add Shared key information into ASIC.
 * Update shared key, TxMic and RxMic to Asic Shared key table
 * Update its cipherAlg to Asic Shared key Mode.
 *
 * Return:
 * ========================================================================
 */
VOID MtAsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BssIndex,
	IN UCHAR			KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


/*	IRQL = DISPATCH_LEVEL*/
VOID MtAsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}



/** @} */
/** @} */

INT MtAsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


BOOLEAN MtAsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	BOOLEAN cmd_done = TRUE;

	MtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
	return cmd_done;
}


BOOLEAN MtAsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


VOID MtAsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}




#ifdef STREAM_MODE_SUPPORT
UINT32 MtStreamModeRegVal(RTMP_ADAPTER *pAd)
{
	return 0x0;
}


/*
 * ========================================================================
 * Description:
 * configure the stream mode of specific MAC or all MAC and set to ASIC.
 *
 * Prameters:
 * pAd   ---
 * pMacAddr ---
 * bClear        --- disable the stream mode for specific macAddr when
 * (pMacAddr!=NULL)
 *
 * Return:
 * ========================================================================
 */
VOID MtAsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


VOID MtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}
#endif /* STREAM_MODE_SUPPORT // */


VOID MtAsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	/* AUTO_RSP_CFG_STRUC csr4; */
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


#ifdef DOT11_N_SUPPORT
INT MtAsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	os_zero_mem(aggCnt, cnt_len * sizeof(ULONG));
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


INT MtAsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


#endif /* DOT11_N_SUPPORT // */


/**
 * @addtogroup hif
 * @{
 * @name hif operation HW HAL
 * @{
 */

INT MtAsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable, UINT8 WPDMABurstSIZE)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_set_dma)
		return ops->hif_set_dma(pAd, TxRx, enable, cap->WPDMABurstSIZE);

	return FALSE;
}


BOOLEAN MtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_wait_dma_idle)
		return ops->hif_wait_dma_idle(pAd, round, wait_us);

	return FALSE;
}

/** @} */
/** @} */

INT MtAsicSetMacWD(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


/**
 * @addtogroup radio_mgt
 * @{
 * @name radio management HW HAL
 * @{
 */

VOID MtAsicSwitchChannel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	UINT32 val;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* TODO: shiang-usw, unify the ops */
	if (ops->ChipSwitchChannel)
		ops->ChipSwitchChannel(pAd, SwChCfg);
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("For this chip, no specified channel switch function!\n"));

	/* TODO: shiang-7603 */
	MAC_IO_READ32(pAd, RMAC_CHFREQ, &val);
	val = 1;
	MAC_IO_WRITE32(pAd, RMAC_CHFREQ, val);
}

INT MtAsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
	UINT32 val;

	MAC_IO_READ32(pAd, AGG_BWCR, &val);
	val &= (~0x0c);

	switch (bw) {
	case BW_20:
		val |= (0);
		break;

	case BW_40:
		val |= (0x1 << 2);
		break;

	case BW_80:
		val |= (0x2 << 2);
		break;
	}

	MAC_IO_WRITE32(pAd, AGG_BWCR, val);
#ifndef COMPOS_WIN
	/* TODO: shiang-usw, some CR setting in bbp_set_bw() need to take care!! */
	bbp_set_bw(pAd, bw, BandIdx);
#endif /* COMPOS_WIN */
	return TRUE;
}


#ifdef COMPOS_WIN
BOOLEAN BBPSetBW(RTMP_ADAPTER *pAd, ULONG bw)
{
	/* TODO: shiang-7603, revise following code */
	ULONG MACValue;

	HW_IO_READ32(pAd, AGG_BWCR, &MACValue);
	MACValue &= (~0x0c);

	switch (bw) {
	case BW_20:
		MACValue |= (0);
		break;

	case BW_40:
		MACValue |= (0x1 << 2);
		break;

	case BW_80:
		MACValue |= (0x2 << 2);
		break;
	}

	HW_IO_WRITE32(pAd, AGG_BWCR, MACValue);
	return TRUE;
}


static VOID MT7603BBPInit(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	HW_IO_READ32(pAd, 0x106c8, &Value);
	Value &= ~(1 << 30);
	HW_IO_WRITE32(pAd, 0x106c8, Value);
	HW_IO_READ32(pAd, 0x106cc, &Value);
	Value &= ~(1 << 0);
	HW_IO_WRITE32(pAd, 0x106cc, Value);
}

#endif /* COMPOS_WIN */





/** @} */
/** @} */


/**
 * @addtogroup qa_ate
 * @{
 * @name QA/ATE HW HAL
 * @{
 */

#ifdef CONFIG_ATE
UINT32 MtAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	UINT32 value = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type:%d\n", __func__, type));

	switch (type) {
	case HQA_RX_STAT_MACFCSERRCNT:
		MAC_IO_READ32(pAd, MIB_MSDR4, &value);
		value = (value >> 16) & 0xFFFF; /* [31:16] FCS ERR */
		break;

	case HQA_RX_STAT_MAC_MDRDYCNT:
		MAC_IO_READ32(pAd, MIB_MSDR10, &value);
		break;

	case HQA_RX_STAT_PHY_MDRDYCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS5, &value);
		break;

	case HQA_RX_STAT_PHY_FCSERRCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS4, &value);
		break;

	case HQA_RX_STAT_PD:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS0, &value);
		break;

	case HQA_RX_STAT_CCK_SIG_SFD:
		/* [31:16] SIG [15:0] SFD */
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS1, &value);
		break;

	case HQA_RX_STAT_OFDM_SIG_TAG:
		/* [31:16] SIG [15:0] TAG */
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS2, &value);
		break;

	case HQA_RX_STAT_RSSI:
		if (IS_MT7637(pAd)) {
			/*Inst RSSI WB R0[29:20] Inst RSSI IB R0[19:10]*/
			PHY_IO_READ32(pAd, MT7367_RO_AGC_DEBUG_2, &value);
		} else {
			/*[31:24]IBRSSI0 [23:16]WBRSSI0 [15:8]IBRSSI1 [7:0]WBRSSI1*/
			PHY_IO_READ32(pAd, RO_AGC_DEBUG_2, &value);
		}

		break;

	case HQA_RX_RESET_PHY_COUNT:
		if (IS_MT76x6(pAd) || IS_MT7637(pAd)) {
			PHY_IO_READ32(pAd, CR_PHYMUX_5, &value);
			value &= 0xFF0FFFFF;
			PHY_IO_WRITE32(pAd, CR_PHYMUX_5, value);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, -->\n", __func__));
			PHY_IO_READ32(pAd, CR_PHYMUX_5, &value);
			value |= 0x00500000;
			PHY_IO_WRITE32(pAd, CR_PHYMUX_5, value);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, <--\n", __func__));
		} else {
			PHY_IO_READ32(pAd, CR_PHYCTRL_2, &value);
			value |= (1<<6); /* BIT6: CR_STSCNT_RST */
			PHY_IO_WRITE32(pAd, CR_PHYCTRL_2, value);
			value &= (~(1<<6));
			PHY_IO_WRITE32(pAd, CR_PHYCTRL_2, value);
			value |= (1<<7); /* BIT7: CR_STSCNT_EN */
			PHY_IO_WRITE32(pAd, CR_PHYCTRL_2, value);
		}

		break;

	case HQA_RX_RESET_MAC_COUNT:
		MAC_IO_READ32(pAd, MIB_MSDR4, &value);
		MAC_IO_READ32(pAd, MIB_MSDR10, &value);
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS5, &value);
		PHY_IO_READ32(pAd, RO_PHYCTRL_STS4, &value);
		break;

	case HQA_RX_ACI_HIT:
		/* [27] ACI_hit */
		PHY_IO_READ32(pAd, CR_ACI_HIT, &value);
		break;

	default:
		break;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type(%d):%x\n", __func__, type, value));
	return value;
}

INT MtAsicSetTxTonePower(RTMP_ADAPTER *pAd, INT dec0, INT dec1)
{
	INT Ret = TRUE;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s-v2 dec0 = %d, dec1 = %d\n", __func__, dec0, dec1));

	if (dec0 < 0 || dec0 > 0xF || dec1 > 31 || dec1 <  -32) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s dec value invalid\n", __func__));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s dec0 0~F, dec1 -32~31\n", __func__));

		if (dec0 > 0xf)
			dec0 = 0xf;

		if (dec1 > 31)
			dec1 = 31;
	}

	/* if((MTK_REV_GTE(pAd, MT7603, MT7603E1))|| */
	if (MTK_REV_GTE(pAd, MT7603, MT7603E2)) {
		UCHAR PowerDec0 = dec0&0x0F;
		UINT32 SetValue = 0x04000000;
		ULONG Tempdec1 = 0;

		SetValue |= PowerDec0<<20;
		/* RF Gain 1 db */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s RF 1db SetValue = 0x%x\n", __func__, SetValue));
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)/* 1       todo fw Cmd */
		MtCmdRFRegAccessWrite(pAd, 0, 0x70, SetValue); /* bit 26(0x04000000) is enable */
		MtCmdRFRegAccessWrite(pAd, 1, 0x70, SetValue);
#endif

		/* DC Gain */
		if (dec1 < 0)
			Tempdec1 = (0x40+dec1);
		else
			Tempdec1 = dec1;

		SetValue = 0x40000000|(Tempdec1<<20);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s DC 0.25db SetValue = 0x%x\n", __func__, SetValue));
		HW_IO_WRITE32(pAd, CR_TSSI_9, SetValue); /* 0x10D24, bit 30(0x40000000) isenable */
		HW_IO_WRITE32(pAd, CR_WF1_TSSI_9, SetValue); /* 0x11D24, bit 30(0x40000000) isenable */
	} else {
	}

	return Ret;
}

INT MtAsicSetRfFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	UINT32 Value = 0;
	INT Ret = TRUE;

	if (FreqOffset > 127)
		FreqOffset = 127;

	HW_IO_READ32(pAd, XTAL_CTL13, &Value);
	Value &= ~DA_XO_C2_MASK; /* [14:8] (DA_XO_C2) */
	Value |= DA_XO_C2(0x3C); /* set 60(DEC) */
	HW_IO_WRITE32(pAd, XTAL_CTL13, Value);
	HW_IO_READ32(pAd, XTAL_CTL14, &Value);
	Value &= ~DA_XO_C2_MASK;
	Value |= DA_XO_C2(0x7F);
	HW_IO_WRITE32(pAd, XTAL_CTL14, Value);
	HW_IO_READ32(pAd, XTAL_CTL13, &Value);
	Value &= ~DA_XO_C2_MASK; /* [14:8] (DA_XO_C2) */
	Value |= DA_XO_C2(FreqOffset); /* set 60(DEC) */
	HW_IO_WRITE32(pAd, XTAL_CTL13, Value);
	return Ret;
}


INT MtAsicSetTSSI(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	UINT32 WF0Offset = 0x10D04; /* WF_PHY_CR_FRONT CR_WF0_TSSI_1 */
	UINT32 WF1Offset = 0x11D04; /* WF_PHY_CR_FRONT CR_WF1_TSSI_1 */
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE)
		CRValue = 0xE3F3F800;
	else
		CRValue = 0xE1010800;

	if ((WFSelect == 0) || (WFSelect == 2)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Set WF#%d TSSI off\n", __func__, WFSelect));
		PHY_IO_WRITE32(pAd, WF0Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd, 0x10D18, 0x0);
		}
	}

	if ((WFSelect == 1) || (WFSelect == 2)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Set WF#%d TSSI on\n", __func__, WFSelect));
		PHY_IO_WRITE32(pAd, WF1Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd, 0x11D18, 0x0);
		}
	}

	return Ret;
}


INT MtAsicSetDPD(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	UINT32 WF0Offset = 0x10A08;
	UINT32 WF1Offset = 0x11A08;
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE) {
		/* WF0 */
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd, WF0Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd, WF0Offset, CRValue);
		}

		/* WF1 */
		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd, WF1Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd, WF1Offset, CRValue);
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Set WFSelect: %d DPD off\n", __func__, WFSelect));
	} else {
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd, WF0Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd, WF0Offset, CRValue);
		}

		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd, WF1Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd, WF1Offset, CRValue);
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Set WFSelect: %d DPD on\n", __func__, WFSelect));
	}

	return Ret;
}

INT MtAsicSetTxToneTest(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR Type)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, bOnOff:%d Type:%d\n", __func__, bOnOff, Type));

	if (bOnOff == 0) { /* 0 = off 1 = on */
		/* WF0 */
		PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000000); /* 0x10000 */
		PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000000); /* 0x10004 */
		PHY_IO_WRITE32(pAd, CR_TSSI_0, 0x80274027); /* 0x10D00 */
		PHY_IO_WRITE32(pAd, CR_TSSI_1, 0xC0000800); /* 0x10D04 */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x00000008); /* 0x1420C */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000580); /* 0x14214 */
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); /* 0x14704 */
		PHY_IO_WRITE32(pAd, CR_TSSI_9, 0x00000000); /* 0x10D24 */
		PHY_IO_WRITE32(pAd, CR_TXFE_3, 0x00000000); /* 0x10A08 */
		PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x00000000); /* 0x101A0 */
		PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000); /* 0x101A4 */
		PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00000000); /* 0x1020C */
		PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x00000000); /* 0x10A0C */
		PHY_IO_WRITE32(pAd, CR_DPD_CAL_03, 0x20300604); /* 0x1090C */
		/* WF1 */
		PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000000); /* 0x10000 */
		PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000000); /* 0x10004 */
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, 0x80274027); /* 0x11D00 */
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0xC0000800); /* 0x11D04 */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x00000008); /* 0x1520C */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000580); /* 0x15214 */
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); /* 0x14704 */
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_9, 0x00000000); /* 0x11D24 */
		PHY_IO_WRITE32(pAd, CR_TXFE1_3, 0x00000000); /* 0x11A08 */
		PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x00000000); /* 0x101A0 */
		PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000); /* 0x101A4 */
		PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00000000); /* 0x1020C */
		PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x00000000); /* 0x11A0C */
		PHY_IO_WRITE32(pAd, CR_DPD_CAL_03, 0x20300604); /* 0x1090C */
	} else if (bOnOff == 1) {
		/* WF0 */
		if (Type == WF0_TX_ONE_TONE_5M || Type == WF0_TX_TWO_TONE_5M ||
			Type == WF0_TX_ONE_TONE_10M || Type == WF0_TX_ONE_TONE_DC) {
			/* 1. clock setup */
			PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000021);
			PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000021);
			/* 2. TX setup */
			PHY_IO_WRITE32(pAd, CR_TSSI_0, 0x00274027);
			PHY_IO_WRITE32(pAd, CR_TSSI_1, 0xC0000400);
			PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x80000008);
			PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597);
			PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x10000000);
			PHY_IO_WRITE32(pAd, CR_TSSI_9, 0x60000000);
			PHY_IO_WRITE32(pAd, CR_TXFE_3, 0xF0000000);

			/* 3. Gen Tone */
			if (Type == WF0_TX_ONE_TONE_5M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C100C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
			} else if (Type == WF0_TX_TWO_TONE_5M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C104C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
			} else if (Type == WF0_TX_ONE_TONE_10M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C101C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00011122);
				PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
			} else if (Type == WF0_TX_ONE_TONE_DC) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C1048);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x07000700);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
			} else
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s Type = %d error!!!\n", __func__, Type));
		} else if (Type == WF1_TX_ONE_TONE_5M || Type == WF1_TX_TWO_TONE_5M ||
				   Type == WF1_TX_ONE_TONE_10M || Type == WF1_TX_ONE_TONE_DC) {
			/* 1. clock setup */
			PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000021);
			PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000021);
			/* 2. TX setup */
			PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, 0x00274027);
			PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0xC0000400);
			PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80000008);
			PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000597);
			PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x10000000);
			PHY_IO_WRITE32(pAd, CR_WF1_TSSI_9, 0x60000000);
			PHY_IO_WRITE32(pAd, CR_TXFE1_3, 0xF0000000);

			/* 3. Gen Tone */
			if (Type == WF1_TX_ONE_TONE_5M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C100C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
			} else if (Type == WF1_TX_TWO_TONE_5M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C104C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
			} else if (Type == WF1_TX_ONE_TONE_10M) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C101C);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00011122);
				PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
			} else if (Type == WF1_TX_ONE_TONE_DC) {
				PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C1048);
				PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x07000700);
				PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
				PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
			} else
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s Type = %d error!!!\n", __func__, Type));
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s Type = %d error!!!\n", __func__, Type));
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s bOnOff = %d error!!!\n", __func__, bOnOff));

	return 0;
}

INT MtAsicStopContinousTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 val = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000005); /* 0x14704 */
	PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); /* 0x14704 */
	PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000045); /* 0x10000 */
	PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000045); /* 0x10004 */
	PHY_IO_WRITE32(pAd, CR_FFT_MANU_CTRL, 0x00000000); /* 0x10704 */
	PHY_IO_WRITE32(pAd, CR_PHYCTRL_0, 0x00000000); /* 0x14100 */
	PHY_IO_WRITE32(pAd, CR_PHYCTRL_DBGCTRL, 0x00000000); /* 0x14140 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x7C900408); /* 0x1420C */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000580); /* 0x14214 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000000); /* 0x15214 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_10, 0x00000000); /* 0x14228 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0x00000000); /* 0x1422C */
	PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x00000000); /* 0x10A0C */
	PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x00000000); /* 0x11A0C */
	PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x00000000); /* 0x14700 */
	PHY_IO_WRITE32(pAd, CR_TXFD_3, 0x00000000); /* 0x1470C */
	MAC_IO_WRITE32(pAd, TMAC_PCTSR, 0x00000000); /* 0x21708 */
	PHY_IO_WRITE32(pAd, CR_FR_RST_CTRL, 0xFFFFFFFF); /* 0x10008 */
	PHY_IO_WRITE32(pAd, CR_BK_RST_CTRL, 0xFFFFFFFF); /* 0x14004 */
	ATEOp->RfRegRead(pAd, ATECtrl->TxAntennaSel, 0x0D4, &val);
	val &= 0x7fffffff;
	ATEOp->RfRegWrite(pAd, ATECtrl->TxAntennaSel, 0x0D4, val);
	ATEOp->RfRegRead(pAd, ATECtrl->TxAntennaSel, 0x0D4, &val);
	val &= 0x00000fff;
	ATEOp->RfRegWrite(pAd, ATECtrl->TxAntennaSel, 0x0D8, val);
	/* Restore TSSI CR */
	MtTestModeRestoreCr(pAd, CR_TSSI_0);	/* 0x0D00 */
	MtTestModeRestoreCr(pAd, CR_WF1_TSSI_0);	/* 0x1D00 */
	MtTestModeRestoreCr(pAd, CR_TSSI_1);	/* 0x0D04 */
	MtTestModeRestoreCr(pAd, CR_WF1_TSSI_1);	/* 0x1D04 */
	MtTestModeRestoreCr(pAd, CR_PHYMUX_27);	/* 0x1426C */
	/* PHY_IO_WRITE32(pAd, CR_TSSI_0, ATECtrl->tssi0_wf0_cr);	//0x0D00 */
	/* PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, ATECtrl->tssi0_wf1_cr);	//0x1D00 */
	/* PHY_IO_WRITE32(pAd, CR_TSSI_1, ATECtrl->tssi1_wf0_cr);	//0x0D04 */
	/* PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, ATECtrl->tssi1_wf1_cr);	//0x1D04 */
	/* Restore Band Edge Patch CR */
	PHY_IO_WRITE32(pAd, CR_RFINTF_01, 0x00000000); /* 0x0204 */
	PHY_IO_WRITE32(pAd, CR_WF1_RFINTF_01, 0x00000000); /* 0x1204 */
	/* PHY_IO_WRITE32(pAd, CR_PHYMUX_27, ATECtrl->phy_mux_27); //0x1426C */
	return 0;
}

INT MtAsicStartContinousTx(RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Mcs, UINT32 WFSel)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 value = 0;
	UINT32 wf_txgc = 0;
	UINT32 tx_pwr = 0;
	UINT32 rf_pa = 0;
	UINT32 pa_bias = 0;
	UINT32 pa_gc = 0;
	/* Band Edge */
	INT32 gc_thresh = 0;
	INT32 pa_mode = 0;
	INT32 pwr_dbm = 0;
	INT32 gc = 0;	/* for 0x0204, 0x1204 [24:20] */
	UINT32 cr_hilo = 0;
	UINT32 bit = 0;
	UINT32 i = 0;
	UINT32 reg = 0;
	UINT32 reg1 = 0;
	UINT32 reg2 = 0;
#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
	/* TSSI Patch */
	ATECtrl->Sgi = 0;
	/* Change TSSI Training Time */
	/* Get Target Power*/
	PHY_IO_READ32(pAd, CR_TSSI_6, &reg); /* 0x0D18 */
	PHY_IO_READ32(pAd, CR_TSSI_13, &reg1);	/* 0x1D18 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): 0x0D18: 0x%x, tx_pwr(0x0D34):0x%x\n", __func__, reg, reg1));

	for (i = 0; i < 2; i++) {
		mdelay(500);
		ATECtrl->TxCount = 8;
		ATEOp->StartTx(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): finish Start Tx\n", __func__));
		mdelay(500);
		ATEOp->StopTx(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): finish StOP Tx\n", __func__));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): finish Tx TSSI Train\n",	__func__));
	ATEOp->RfRegRead(pAd, WFSel, 0x410, &rf_pa); /* wf?_pa_gc = 0x410[18:16], wf?_pa_bias = 0x410[15:0];*/

	switch (WFSel) {
	case 0:
	case 1:
		PHY_IO_READ32(pAd, CR_TSSI_13, &tx_pwr);
		break;

	case 2:
		PHY_IO_READ32(pAd, CR_WF1_TSSI_13, &tx_pwr);
		break;

	default:
		break;
	}

	/* Band Edge */
	pwr_dbm = (((INT32)tx_pwr) >> 24) & 0x0000007f;
	PHY_IO_READ32(pAd, CR_PHYMUX_24, &reg); /* 0x4260 */
	gc_thresh = (INT32)reg;
	pa_mode = (gc_thresh>>24) & 0x0000007f; /* Half PA[30:24] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pwr_dbm:%d, pa_mode:%d\n", pwr_dbm, pa_mode));

	if (pwr_dbm >= pa_mode)
		gc = 0x1;
	else
		gc = 0x0;

	pa_mode = (gc_thresh>>16) & 0x7f; /* Full PA[22:16] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pwr_dbm:%d, pa_mode:%d\n", pwr_dbm, pa_mode));

	if (pwr_dbm >= pa_mode)
		gc = 0x2;

	pa_mode = (gc_thresh>>8) & 0x7f; /* Super PA[14:8] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pwr_dbm:%d, pa_mode:%d\n", pwr_dbm, pa_mode));

	if (pwr_dbm >= pa_mode)
		gc = 0x3;

	pa_mode = (gc_thresh>>1) & 0x1; /* TX_PA_DYNA[1] */

	if (pa_mode != 1)
		gc = (gc_thresh>>2) & 0x3;

	/* gc write back to [24:20] */
	gc = ((gc<<2)|0x00000010)<<20;
	PHY_IO_WRITE32(pAd, CR_RFINTF_01, gc); /* 0x0204 */
	PHY_IO_WRITE32(pAd, CR_WF1_RFINTF_01, gc); /* 0x1204 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): gc[0x0204]: 0x%x,gc_thresh:%x\n", __func__, gc, gc_thresh));
#endif
	/* Original Flow */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Modulation = %d, BW=%d, pri_ch = %d, rate = %d, WFSelect = %d-->\n",
			 __func__, PhyMode, BW, PriCh, Mcs, WFSel));
	PHY_IO_READ32(pAd, CR_TSSI_6, &reg);
	reg1 = reg;
	PHY_IO_READ32(pAd, CR_WF1_TSSI_6, &reg2);
	PHY_IO_WRITE32(pAd, CR_FR_RST_CTRL, 0xFFFFFFFF); /* 0x10008 */
	PHY_IO_WRITE32(pAd, CR_BK_RST_CTRL, 0xFFFFFFFF); /* 0x14004 */
	PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL,	0x00000078);	/* 0x10000 */
	PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000078);	/* 0x10004 */
	MtTestModeBkCr(pAd, CR_TSSI_0, TEST_PHY_BKCR);
	MtTestModeBkCr(pAd, CR_WF1_TSSI_0, TEST_PHY_BKCR);
	MtTestModeBkCr(pAd, CR_TSSI_1, TEST_PHY_BKCR);
	MtTestModeBkCr(pAd, CR_WF1_TSSI_1, TEST_PHY_BKCR);
	/* Contiuous Tx power patch */
	PHY_IO_WRITE32(pAd, CR_TSSI_0,		0x024041C0);	/* 0x0D00 */
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0,	0x024041C0);	/* 0x1D00 */
	PHY_IO_WRITE32(pAd, CR_TSSI_1,		0x23F3F800);	/* 0x0D04 */
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0x23F3F800); /* 0x1D04 */
	PHY_IO_WRITE32(pAd, CR_TSSI_6, reg1);	/* 0x0D18 */
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_6, reg2);	/* 0x1D18 */
	PHY_IO_WRITE32(pAd, CR_FFT_MANU_CTRL, 0x0AA00000); /* 0x10704 */

	if (BW_20 == BW)
		value = 0x00000000;
	else if (BW_40 == BW)
		value = 0x01000000;
	else if (BW_80 == BW)
		value = 0x02000000;
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s BW = %d error!!!\n", __func__, BW));

	if (PriCh == 0)
		value |= 0x00000000;
	else if (PriCh == 1)
		value |= 0x00100000;
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s pri_ch = %d error!!!\n", __func__, PriCh));

	PHY_IO_WRITE32(pAd, CR_PHYCTRL_0, value); /* 0x14100 */
	PHY_IO_WRITE32(pAd, CR_PHYCTRL_DBGCTRL, 0x80000030); /* 0x14140 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x80680008); /* 0x1420C */

	switch (WFSel) {
	case 0:
		PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597); /* 0x14214 */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000010); /* 0x15214 */
		break;

	case 1:
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80680008); /* 0x1520C */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000590); /* 0x14214 */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000017); /* 0x15214 */
		break;

	case 2:
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80680008); /* 0x1520C */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597); /* 0x14214 */
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000017); /* 0x15214 */
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s WFSelect = %d error!!!\n", __func__, WFSel)); /* No ALL?*/
		break;
	}

	if (BW_20 == BW)
		PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x00030000); /* 0x14700 */
	else if (BW_40 == BW)
		PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x14030000); /* 0x14700 */
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s BW = %d error!!!\n", __func__, BW));

	if (PhyMode == 0) { /* MODULATION_SYSTEM_CCK */
		value = 0;

		switch (Mcs) {
		case MCS_0:
		case MCS_8:
			value = 0x00000000;
			break;

		case MCS_1:
		case MCS_9:
			value = 0x00200000;
			break;

		case MCS_2:
		case MCS_10:
			value = 0x00400000;
			break;

		case MCS_3:
		case MCS_11:
			value = 0x00600000;
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s CCK rate = %d error!!!\n", __func__, Mcs));
			break;
		}

		PHY_IO_WRITE32(pAd, CR_TXFD_3, value);
	} else if (PhyMode == 1) { /* MODULATION_SYSTEM_OFDM */
		PHY_IO_READ32(pAd, CR_PHYMUX_26, &cr_hilo); /* 0x4268 */
		value = 0;

		switch (Mcs) {
		case MCS_0:
			value = 0x01600000;
			cr_hilo &= BIT(0);
			break;

		case MCS_1:
			value = 0x01E00000;
			cr_hilo &= BIT(1);
			break;

		case MCS_2:
			value = 0x01400000;
			cr_hilo &= BIT(2);
			break;

		case MCS_3:
			value = 0x01C00000;
			cr_hilo &= BIT(3);
			break;

		case MCS_4:
			value = 0x01200000;
			cr_hilo &= BIT(4);
			break;

		case MCS_5:
			value = 0x01900000;
			cr_hilo &= BIT(5);
			break;

		case MCS_6:
			value = 0x01000000;
			cr_hilo &= BIT(6);
			break;

		case MCS_7:
			value = 0x01800000;
			cr_hilo &= BIT(7);
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s OFDM rate = %d error!!!\n", __func__, Mcs));
			break;
		}

		PHY_IO_WRITE32(pAd, CR_TXFD_3, value); /* 0x1470C */
		MtTestModeBkCr(pAd, CR_PHYMUX_27, TEST_PHY_BKCR);
		PHY_IO_READ32(pAd, CR_PHYMUX_27, &value); /* 0x1426C */

		if (cr_hilo == 0) { /* Low Rate */
			bit = value & BIT(12);
			value |= bit << 2;
		} else { /* High Rate */
			bit = value & BIT(13);
			value |= bit << 1;
		}

		PHY_IO_WRITE32(pAd, CR_PHYMUX_27, value); /* 0x1426C */
	} else if (2 == PhyMode || 3 == PhyMode) {
		/* MODULATION_SYSTEM_HT20 || MODULATION_SYSTEM_HT40 */
		PHY_IO_READ32(pAd, CR_PHYMUX_25, &cr_hilo); /* 0x4264 */
		value = 0;

		switch (Mcs) {
		case MCS_0:
			value = 0x00000000;
			cr_hilo &= BIT(0);
			break;

		case MCS_1:
			value = 0x00200000;
			cr_hilo &= BIT(1);
			break;

		case MCS_2:
			value = 0x00400000;
			cr_hilo &= BIT(2);
			break;

		case MCS_3:
			value = 0x00600000;
			cr_hilo &= BIT(3);
			break;

		case MCS_4:
			value = 0x00800000;
			cr_hilo &= BIT(4);
			break;

		case MCS_5:
			value = 0x00A00000;
			cr_hilo &= BIT(5);
			break;

		case MCS_6:
			value = 0x00C00000;
			cr_hilo &= BIT(6);
			break;

		case MCS_7:
			value = 0x00E00000;
			cr_hilo &= BIT(7);
			break;

		case MCS_32:
			value = 0x04000000;
			cr_hilo &= BIT(0);
			break;

		default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s OFDM HT MCS = %d error!!!\n", __func__, Mcs));
			break;
		}

		PHY_IO_WRITE32(pAd, CR_TXFD_3, value); /* 0x1470C */
		MtTestModeBkCr(pAd, CR_PHYMUX_27, TEST_PHY_BKCR);
		PHY_IO_READ32(pAd, CR_PHYMUX_27, &value); /* 0x1426C */

		if (cr_hilo == 0) { /* Low Rate */
			bit = value & BIT(12);
			value |= bit << 2;
		} else { /* High Rate */
			bit = value & BIT(13);
			value |= bit << 1;
		}

		PHY_IO_WRITE32(pAd, CR_PHYMUX_27, value); /* 0x1426C */
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s Modulation = %d error!!!\n", __func__, PhyMode));

#ifdef COMPOS_TESTMODE_WIN
	PHY_IO_WRITE32(pAd, CR_PHYMUX_10, 0x09400000);
#else
	/* load the tag_pwr */
	wf_txgc |= 0x08000000;	/* TXGC_MANUAL_ENABLE */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr:0x%x\n", tx_pwr));
	tx_pwr = (tx_pwr>>4)&0x0FF00000;
	wf_txgc |= tx_pwr;
	PHY_IO_WRITE32(pAd, CR_PHYMUX_10, wf_txgc);
	PHY_IO_READ32(pAd, CR_PHYMUX_10, &reg);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr:0x%x, wf_txgc:0x%08x, CR_PHYMUX_10:0x%08x\n", tx_pwr, wf_txgc, reg));
	/* Pre-Load Setting to Continuous Tx */
	ATEOp->RfRegRead(pAd, WFSel, 0x0D4, &pa_bias);
	pa_bias |= 0x80000000; /* pa_bias manual enablei */
	ATEOp->RfRegWrite(pAd, WFSel, 0x0D4, pa_bias);
	/* load pa_gc & pa_bias */
	pa_gc |= 0x00008000; /* bit[15] = 1*/
	pa_gc |= (rf_pa&0x0000ffff)<<16; /* bit[31:16] = wf0_pa_gc */
	pa_gc |= (rf_pa>>4)&0x00007000; /* bit[14:12] */
	ATEOp->RfRegWrite(pAd, WFSel, 0x0D8, pa_gc);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pa_gc:0x%x,<-- rf_pa:0x%x\n", pa_gc, rf_pa));
#endif

	if (WFSel == 0)
		PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0xA0000000); /* 0x1422C */
	else if (WFSel == 1)
		PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0x90000000); /* 0x1422C */
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s WFSelect = %d error!!!\n", __func__, WFSel)); /* No ALL?*/

	if (PhyMode == 0) /* PREAMBLE_CCK */
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x300000F8); /* 0x14704 */
	else if (PhyMode == 1) /* PREAMBLE_OFDM */
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x310000F2); /* 0x14704 */
	else if (2 == PhyMode || 3 == PhyMode) /* PREAMBLE_GREEN_FIELD */
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x320000F2); /* 0x14704 */
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s Modulation = %d error!!!\n", __func__, PhyMode));

	PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0); /* 0x10A0C */
	PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0); /* 0x11A0C */
	return 0;
}

#ifdef COMPOS_TESTMODE_WIN
/**
 * Tx Set Frequency Offset
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param iOffset value
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxSetFrequencyOffset(RTMP_ADAPTER *pAd, UINT32 iOffset, BOOLEAN HasBeenSet)
{
	UINT32 Value = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s iOffset=0x%x-->\n", __func__, iOffset));

	if (HasBeenSet == FALSE) {
		if (IS_MT7603(pAd)) {
			/* RG_XO_C2[8:14]  Set 60 (0x3C )at first */
			MAC_IO_READ32(pAd, RG_XO_C2, &Value);
			Value = (Value & 0xFFFF80FF) | (0x3C << 8);
			MAC_IO_WRITE32(pAd, RG_XO_C2, Value);
			/* RG_XO_C2_MANUAL [8:14]  Set 0x7F at first */
			MAC_IO_READ32(pAd, RG_XO_C2_MANUAL, &Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd, RG_XO_C2_MANUAL, Value);
			/* only set at first time */
		} else if (IS_MT76x6(pAd) || IS_MT7637(pAd)) {
			MAC_IO_READ32(pAd, FREQ_OFFSET_MANUAL_ENABLE, &Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_ENABLE, Value);
		}
	}

	if (IS_MT7603(pAd)) {
		HW_IO_READ32(pAd, RG_XO_C2, &Value);
		Value = (Value & 0xFFFF80FF) | (iOffset << 8);
		HW_IO_WRITE32(pAd, RG_XO_C2, Value);
	}

	return 0;
}
/**
 * Set Tx Power Range
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param ucMaxPowerDbm, Max Power Dbm
 * @param ucMinPowerDbm, Min Power Dbm
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxConfigPowerRange(RTMP_ADAPTER *pAd, IN UCHAR ucMaxPowerDbm, IN UCHAR ucMinPowerDbm)
{
	UINT32 u4RegValue;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s-->\n", __func__));
	MAC_IO_READ32(pAd, TMAC_FPCR, &u4RegValue);
	u4RegValue &= ~(FPCR_FRAME_POWER_MAX_DBM | FPCR_FRAME_POWER_MIN_DBM);
	u4RegValue |= ((ucMaxPowerDbm << FPCR_FRAME_POWER_MAX_DBM_OFFSET) & FPCR_FRAME_POWER_MAX_DBM);
	u4RegValue |= ((ucMinPowerDbm << FPCR_FRAME_POWER_MIN_DBM_OFFSET) & FPCR_FRAME_POWER_MIN_DBM);
	MAC_IO_WRITE32(pAd, TMAC_FPCR, u4RegValue);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<--\n", __func__));
	return 0;
}

INT MTAsicSetTMR(RTMP_ADAPTER *pAd, UCHAR enable)
{
	UINT32 value = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, -->\n", __func__));

	switch (enable) {
	case 1: { /* initialiter */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: enable TMR report, as Initialiter\n", __func__));
		MAC_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value & ~BIT30;
		value = value | 0x34;/* Action frame register */
		MAC_IO_WRITE32(pAd, RMAC_TMR_PA, value);
	}
	break;

	case 2: { /* responder */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: enable TMR report, as Responser\n", __func__));
		MAC_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value | BIT30;
		value = value | 0x34;/* Action frame register */
		MAC_IO_WRITE32(pAd, RMAC_TMR_PA, value);
	}
	break;

	case 0:/* disable */
	default: {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: disable TMR report\n", __func__));
		MAC_IO_READ32(pAd, RMAC_TMR_PA, &value);
		value = value & ~BIT31;
		MAC_IO_WRITE32(pAd, RMAC_TMR_PA, value);
	}
	}

	return 0;
}
#endif
#endif /* CONFIG_ATE */

/** @} */
/** @} */

/**
 * @addtogroup dma_sch
 * @{
 * @name DMA scheduler HW HAL
 * @{
 */


#ifdef DMA_SCH_SUPPORT


/*
 * DMA scheduer reservation page assignment
 * Q0~Q3: WMM1
 * Q4: Management queue
 * Q5: MCU CMD
 * Q7: Beacon
 * Q8: MC/BC
 * Q9~Q12: WMM2
 * Q13: Management queue
 */
#define MAX_BEACON_SIZE		512
#define MAX_BMCAST_SIZE		1536
#define MAX_BMCAST_COUNT	3
#define MAX_MCUCMD_SIZE	4096 /*must >= MAX_DATA_SIZE */
#define MAX_DATA_SIZE		1792 /* 0xe*128=1792 */
#define MAX_AMSDU_DATA_SIZE	4096
/*
 * HYBRID Mode: DMA scheduler would ignore the tx op time information from LMAC, and also use FFA and RSV for enqueue cal.
 * BYPASS Mode: Only for Firmware download
 */

INT32 MtAsicDMASchedulerInit(RTMP_ADAPTER *pAd, MT_DMASCH_CTRL_T DmaSchCtrl)
{
	UINT32 mac_val;
	UINT32 page_size = 128;
	UINT32 page_cnt = 0x1ae;
	INT dma_mode = DmaSchCtrl.mode;
#ifdef RTMP_PCI_SUPPORT
	UINT32 mac_restore_val;
#if defined(MT7636_FPGA) || defined(MT7637_FPGA)
	UINT32 RemapBase, RemapOffset;
#endif
#endif /* RTMP_MAC_PCI */
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
	page_size = 256;
#endif /* MT7603_FPGA */
#if defined(MT7636_FPGA) || defined(MT7637_FPGA)
	page_size = 128;
#endif /* MT7636_FPGA || MT7637_FPGA */
	/* Got PSE P0 MAX Quota */
#ifdef RTMP_MAC_PCI
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
	HW_IO_READ32(pAd, 0x80120, &mac_val);
	page_cnt = (mac_val & 0x0fff0000) >> 16;
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#endif /* RTMP_MAC_PCI */
	/* Setting PSE page free priority,P0(HIF) > P1(MCU) >  P2Q2(TXS) >	P2Q1(RXV) =  P2Q0(Rxdata)*/
	mac_val = 0x00004037;
	MAC_IO_WRITE32(pAd, FC_FRP, mac_val);

	if (dma_mode == DMA_SCH_BYPASS) {
		MAC_IO_WRITE32(pAd, MT_SCH_REG_4, 1<<5);
		/* Disable DMA scheduler */
		MAC_IO_READ32(pAd, AGG_DSCR1, &mac_val);
		mac_val |= 0x80000000;
		MAC_IO_WRITE32(pAd, AGG_DSCR1, mac_val);
#ifdef RTMP_MAC_PCI
		/*
		  *	Wei-Guo's comment:
		  *	2DW/7DW => 0x800C_006C[14:12] = 3'b0
		  *	3DW/8DW =>0x800C_006C[14:12] = 3'b1
		  */
		/* In FPGA mode, we need to change tx pad by different DMA scheduler setting! */
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
		HW_IO_READ32(pAd, 0xc006c, &mac_val);
		mac_val &= (~(7<<12));
		mac_val |= (1<<12);
		HW_IO_WRITE32(pAd, 0xc006c, mac_val);
		/* After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region */
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_MAC_PCI */
		/* work around for un-sync of TxD between HIF and LMAC */
		MAC_IO_READ32(pAd, DMA_DCR1, &mac_val);
		mac_val &= (~(0x7<<8));
		mac_val |= (0x1<<8);
		MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val);
	}


	if ((dma_mode == DMA_SCH_LMAC) || (dma_mode == DMA_SCH_HYBRID)) {
#ifdef CONFIG_AP_SUPPORT
		UINT32 max_beacon_page_count = MAX_BEACON_SIZE/page_size;
#endif
		UINT32 max_bmcast_page_count = MAX_BMCAST_SIZE/page_size;
		UINT32 max_mcucmd_page_count = MAX_MCUCMD_SIZE/page_size;
		UINT32 max_data_page_count = MAX_DATA_SIZE/page_size;
		UINT32 total_restore_val;
		UINT32 mcu_restore_val;
		UINT32 bcn_restore_val = 0;
		UINT32 mbc_restore_val;
		/* Highest Priority:Q7: Beacon > Q8: MC/BC > Q5: MCU CMD */
		mac_val = 0x55555555;
		MAC_IO_WRITE32(pAd, MT_HIGH_PRIORITY_1, mac_val);
		mac_val = 0x78555555;
		MAC_IO_WRITE32(pAd, MT_HIGH_PRIORITY_2, mac_val);
		/* Queue Priority */
		mac_val = 0x2b1a096e;
		MAC_IO_WRITE32(pAd, MT_QUEUE_PRIORITY_1, mac_val);
		mac_val = 0x785f4d3c;
		MAC_IO_WRITE32(pAd, MT_QUEUE_PRIORITY_2, mac_val);
		MAC_IO_WRITE32(pAd, MT_PRIORITY_MASK, 0xffffffff);
		/* Schedule Priority, page size/FFA, FFA = (page_cnt * page_size) */
		mac_val = (2 << 28) | (page_cnt);
		MAC_IO_WRITE32(pAd, MT_SCH_REG_1, mac_val);
		mac_val = MAX_DATA_SIZE / page_size;
		MAC_IO_WRITE32(pAd, MT_SCH_REG_2, mac_val);
		/* Resvervation page */
#ifdef DOT11_N_SUPPORT

		if (pAd->CommonCfg.BACapability.field.AmsduEnable == TRUE)
			max_data_page_count = MAX_AMSDU_DATA_SIZE/page_size;

#endif
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_0, max_data_page_count);
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_1, max_data_page_count);
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_2, max_data_page_count);
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_3, max_data_page_count);
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_4, max_data_page_count);
		total_restore_val = max_data_page_count*5;
		mcu_restore_val = max_mcucmd_page_count;
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_5, mcu_restore_val);
		total_restore_val += mcu_restore_val;
#ifdef CONFIG_AP_SUPPORT

		if (pAd->ApCfg.BssidNum  > 1)
			bcn_restore_val = max_beacon_page_count*(pAd->ApCfg.BssidNum - 1) + max_data_page_count;
		else
			bcn_restore_val = max_data_page_count;

		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_7, bcn_restore_val);
#endif /* CONFIG_AP_SUPPORT */
		total_restore_val += bcn_restore_val;
		mbc_restore_val = max_bmcast_page_count*MAX_BMCAST_COUNT + max_data_page_count;
		MAC_IO_WRITE32(pAd, MT_PAGE_CNT_8, mbc_restore_val);
		total_restore_val += mbc_restore_val;
		/* FFA1 max threshold */
		MAC_IO_WRITE32(pAd, MT_RSV_MAX_THD, (page_cnt - total_restore_val));
		/* Group Threshold */
		MAC_IO_WRITE32(pAd, MT_GROUP_THD_0, page_cnt);
		MAC_IO_WRITE32(pAd, MT_BMAP_0, 0xffff);

		if (dma_mode == DMA_SCH_LMAC) {
			/* config as LMAC prediction mode */
			MAC_IO_WRITE32(pAd, MT_SCH_REG_4, 0x0);
		}

		if (dma_mode == DMA_SCH_HYBRID) {
			/* config as hybrid mode */
			MAC_IO_WRITE32(pAd, MT_SCH_REG_4, 1<<6);
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
#ifdef RTMP_PCI_SUPPORT
			/*                      Wei-Guo's comment:
			 * 2DW/7DW => 0x800C_006C[14:12] = 3'b0
			 * 3DW/8DW =>0x800C_006C[14:12] = 3'b1
			 */
			/* In FPGA mode, we need to change tx pad by different DMA scheduler setting */
			HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
			HW_IO_READ32(pAd, 0xc006c, &mac_val);
			mac_val &= (~(7<<12));
			HW_IO_WRITE32(pAd, 0xc006c, mac_val);
			/* After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region */
			HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT7603_FPGA */
			/* Disable TxD padding */
			MAC_IO_READ32(pAd, DMA_DCR1, &mac_val);
			mac_val &= (~(0x7<<8));
			MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val);
		}
	}

	/* if (MTK_REV_GTE(pAd, MT7603,MT7603E1)) { */
	mac_val = 0xfffff;
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_0, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_1, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_2, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_3, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_4, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_5, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_6, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_7, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_8, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_9, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_10, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_11, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_12, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_13, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_14, mac_val);
	MAC_IO_WRITE32(pAd, MT_TXTIME_THD_15, mac_val);
	MAC_IO_READ32(pAd, MT_SCH_REG_4, &mac_val);
	mac_val |= 0x40;
	MAC_IO_WRITE32(pAd, MT_SCH_REG_4, mac_val);
	/* } */
#if defined(MT7636_FPGA) || defined(MT7637_FPGA)
#ifdef RTMP_PCI_SUPPORT
	/* MAC_IO_READ32(pAd, MT_FPGA_PSE_CLIENT_CNT, &mac_val); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(MT_FPGA_PSE_CLIENT_CNT) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(MT_FPGA_PSE_CLIENT_CNT);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_READ32(pAd, 0x80000 + RemapOffset, &mac_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#ifdef MT7636_FPGA
	/* MAC_IO_WRITE32(pAd, MT_FPGA_PSE_SET_0, 0xffffffff); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(MT_FPGA_PSE_SET_0) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(MT_FPGA_PSE_SET_0);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_WRITE32(pAd, 0x80000 + RemapOffset, 0xffffffff);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
	/* MAC_IO_WRITE32(pAd, MT_FPGA_PSE_SET_1, 0xffffffff); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(MT_FPGA_PSE_SET_1) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(MT_FPGA_PSE_SET_1);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_WRITE32(pAd, 0x80000 + RemapOffset, 0xffffffff);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
	mac_val |= (1 << 1);
#endif /* MT7636_FPGA */
	/* FPGA PSE client will hornor this CR seeting and copy type/subtype from packet to TxD*/
#ifdef MT7637_FPGA
	mac_val &= ~((1 << 12) | (1 << 13) | (1 << 14));
#endif /*MT7637_FPGA */
	/* MAC_IO_WRITE32(pAd, MT_FPGA_PSE_CLIENT_CNT, mac_val); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(MT_FPGA_PSE_CLIENT_CNT) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(MT_FPGA_PSE_CLIENT_CNT);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_WRITE32(pAd, 0x80000 + RemapOffset, mac_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT7636_FPGA || MT7637_FPGA */
	/* Allen Wu WEP BB under run test */
#if defined(MT7637_FPGA)
#ifdef RTMP_PCI_SUPPORT
	/* MAC_IO_READ32(pAd, 0x60140d04, &mac_val); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(0x60140d04) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(0x60140d04);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_READ32(pAd, 0x80000 + RemapOffset, &mac_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
	mac_val |= ((0x1ff << 7) | (0xff << 16) | (0xf << 28));
	/* MAC_IO_WRITE32(pAd, 0x60140d04, mac_val); */
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	RemapBase = GET_REMAP_2_BASE(0x60140d04) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(0x60140d04);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	HW_IO_WRITE32(pAd, 0x80000 + RemapOffset, mac_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT7637_FPGA */
	return TRUE;
}

#endif /* DMA_SCH_SUPPORT */


INT32 MtAsicSetDmaByPassMode(RTMP_ADAPTER *pAd, BOOLEAN isByPass)
{
#ifdef DMA_SCH_SUPPORT
	UINT32 value;

	if (isByPass) {
		/* switch to bypass mode */
		MAC_IO_READ32(pAd, SCH_REG4, &value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): SCH_REG4: 0x%x\n", __func__, value));
		value &= ~SCH_REG4_BYPASS_MODE_MASK;
		value |= SCH_REG4_BYPASS_MODE(1);
#ifdef RTMP_PCI_SUPPORT
		value &= ~SCH_REG4_FORCE_QID_MASK;
		value |= SCH_REG4_FORCE_QID(5);
#endif
		MAC_IO_WRITE32(pAd, SCH_REG4, value);
	} else {
		/* Switch to normal mode */
		MAC_IO_READ32(pAd, SCH_REG4, &value);
		value &= ~SCH_REG4_BYPASS_MODE_MASK;
		value |= SCH_REG4_BYPASS_MODE(0);
		value &= ~SCH_REG4_FORCE_QID_MASK;
		value |= SCH_REG4_FORCE_QID(0);
		MAC_IO_WRITE32(pAd, SCH_REG4, value);
		MAC_IO_READ32(pAd, SCH_REG4, &value);
		value |= (1 << 8);
		MAC_IO_WRITE32(pAd, SCH_REG4, value);
		MAC_IO_READ32(pAd, SCH_REG4, &value);
		value &= ~(1 << 8);
		MAC_IO_WRITE32(pAd, SCH_REG4, value);
	}

#endif /*DMA_SCH_SUPPORT*/
	return TRUE;
}




/** @} */
/** @} */


/**
 * @addtogroup others
 * @{
 * @name Other HW HAL
 * @{
 */
VOID MtAsicInitMac(RTMP_ADAPTER *pAd)
{
#ifndef MAC_INIT_OFFLOAD
	UINT32 mac_val;
#endif
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
#ifndef MAC_INIT_OFFLOAD
	/* Preparation of TxD DW2~DW6 when we need run 3DW format */
	MtAsicSetTmacInfoTemplate(pAd);
	MtAsicSetBAWinSizeRange(pAd);
	/* Vector report queue setting */
	MAC_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val |= BIT13;
	MAC_IO_WRITE32(pAd, DMA_VCFR0, mac_val);
	/* TMR report queue setting */
	MAC_IO_READ32(pAd, DMA_TMCFR0, &mac_val);
	mac_val |= BIT13;/* TMR report send to HIF q1. */
	mac_val = mac_val & ~(BIT0);
	mac_val = mac_val & ~(BIT1);
	MAC_IO_WRITE32(pAd, DMA_TMCFR0, mac_val);
#ifdef COMPOS_TESTMODE_WIN/* set default  tx status from RX1 */
	MAC_IO_READ32(pAd, DMA_TCFR1, &mac_val);
	mac_val |= BIT(14);
	MAC_IO_WRITE32(pAd, DMA_TCFR1, mac_val);
#endif
	MAC_IO_READ32(pAd, RMAC_TMR_PA, &mac_val);
	mac_val = mac_val & ~BIT31;
	MAC_IO_WRITE32(pAd, RMAC_TMR_PA, mac_val);
	/* In MT7636, this CR will be set by FW, mgmt frame will be forward to MCU (LP, MCC) then HOST if necessary*/
	/* Configure all rx packets to HIF, except WOL2M packet */
	MAC_IO_READ32(pAd, DMA_RCFR0, &mac_val);
	mac_val = 0x00010000; /* drop duplicate */
	mac_val |= 0xc0200000; /* receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED */

	if (pAd->rx_pspoll_filter)
		mac_val |= 0x00000008; /* Non-BAR Control frame to MCU */

	MAC_IO_WRITE32(pAd, DMA_RCFR0, mac_val);
	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val &= (~0x1); /* To HIF */
	mac_val |= 0x2000; /* RxRing 1 */
	MAC_IO_WRITE32(pAd, DMA_VCFR0, mac_val);
	/* RMAC dropping criteria for max/min recv. packet length */
	MAC_IO_READ32(pAd, RMAC_RMACDR, &mac_val);
	mac_val |= SELECT_RXMAXLEN_20BIT;
	MAC_IO_WRITE32(pAd, RMAC_RMACDR, mac_val);
	MAC_IO_READ32(pAd, RMAC_MAXMINLEN, &mac_val);
	mac_val &= ~RMAC_DROP_MAX_LEN_MASK;
	mac_val |= RMAC_DROP_MAX_LEN;
	mac_val |= RMAC_DROP_MIN_LEN << 24;
	MAC_IO_WRITE32(pAd, RMAC_MAXMINLEN, mac_val);
#ifndef COMPOS_WIN
	/* Enable RX Group to HIF */
	MtAsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
	MtAsicSetRxGroup(pAd, MCU_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
#else
	MtAsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP3, TRUE);
	MtAsicSetRxGroup(pAd, MCU_PORT, RXS_GROUP3, TRUE);
#endif /* !COMPOS_WIN */
	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	MtAsicSetBARTxCntLimit(pAd, TRUE, 1);
	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd, AGG_ACR, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd, AGG_ACR, mac_val);
	/* AMPDU Statistics Range Control setting
	 * 0 < agg_cnt - 1 <= range_cr(0),                      => 1
	 * range_cr(0) < agg_cnt - 1 <= range_cr(4),            => 2~5
	 * range_cr(4) < agg_cnt - 1 <= range_cr(14),   => 6~15
	 * range_cr(14) < agg_cnt - 1,                          => 16~
	 */
	MAC_IO_READ32(pAd, AGG_ASRCR, &mac_val);
	mac_val =  (0 << 0) | (4 << 8) | (14 << 16);
	MAC_IO_WRITE32(pAd, AGG_ASRCR, mac_val);
	/* Enable MIB counters */
	MAC_IO_WRITE32(pAd, MIB_MSCR, 0x7fffffff);
	MAC_IO_WRITE32(pAd, MIB_MPBSCR, 0xffffffff);
	/* CCA Setting */
	MAC_IO_READ32(pAd, TMAC_TRCR, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd, TMAC_TRCR, mac_val);
	/* RCPI include ACK and Data */
	MAC_IO_READ32(pAd, WTBL_OFF_RMVTCR, &mac_val);
	mac_val |= RX_MV_MODE;
	MAC_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);
	/* Turn on RX RIFS Mode */
	MAC_IO_READ32(pAd, TMAC_TCR, &mac_val);
	mac_val |= RX_RIFS_MODE;
	MAC_IO_WRITE32(pAd, TMAC_TCR, mac_val);
	/* IOT issue with Realtek at CCK mode */
	mac_val = 0x003000E7;
	MAC_IO_WRITE32(pAd, TMAC_CDTR, mac_val);
	/* IOT issue with Linksys WUSB6300. Cannot receive BA after TX finish */
	mac_val = 0x4;
	MAC_IO_WRITE32(pAd, TMAC_RRCR, mac_val);
	/* send RTS/CTS if agg size >= 2 */
	MAC_IO_READ32(pAd, AGG_PCR1, &mac_val);
	mac_val &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	mac_val |= RTS_PKT_NUM_THRESHOLD(3);
	MAC_IO_WRITE32(pAd, AGG_PCR1, mac_val);
#ifdef VENDOR_FEATURE7_SUPPORT
/* Set RTS Retry from profile */
#ifdef CONFIG_AP_SUPPORT
	MtAsicSetRTSRetryCnt(pAd);
#endif
#endif
	/* When WAPI + RDG, don't mask ORDER bit  */
	MAC_IO_READ32(pAd, SEC_SCR, &mac_val);
	mac_val &= 0xfffffffc;
	MAC_IO_WRITE32(pAd, SEC_SCR, mac_val);
	/* Enable Spatial Extension for RTS/CTS  */
	MAC_IO_READ32(pAd, TMAC_PCR, &mac_val);
	mac_val |= PTEC_SPE_EN;
	MAC_IO_WRITE32(pAd, TMAC_PCR, mac_val);
	/* Enable Spatial Extension for ACK/BA/CTS */
	MAC_IO_READ32(pAd, TMAC_B0BRR0, &mac_val);
	mac_val |= BSSID00_RESP_SPE_EN;
	MAC_IO_WRITE32(pAd, TMAC_B0BRR0, mac_val);
#ifndef BCN_OFFLOAD_SUPPORT
	/*Make bcnQ move pkt to FreeQ after sent out.*/
	MAC_IO_WRITE32(pAd, AGG_BQCR, 0x23);
#endif
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
	RTMP_UPDATE_RTS_THRESHOLD(pAd, pAd->CommonCfg.RtsPktThreshold, pAd->CommonCfg.RtsThreshold);
#endif
#endif /* MAC_INIT_OFFLOAD */
}


INT32 MtAsicGetMacInfo(RTMP_ADAPTER *pAd, UINT32 *ChipId, UINT32 *HwVer, UINT32 *FwVer)
{
	UINT32 Value;

	HW_IO_READ32(pAd, TOP_HVR, &Value);
	*HwVer = Value;
	HW_IO_READ32(pAd, TOP_FVR, &Value);
	*FwVer = Value;
	HW_IO_READ32(pAd, TOP_HCR, &Value);
	*ChipId = Value;
	return TRUE;
}

INT32 MtAsicGetAntMode(RTMP_ADAPTER *pAd, UCHAR *AntMode)
{
	UINT32 Value;

	HW_IO_READ32(pAd, STRAP_STA, &Value);
	*AntMode = (UCHAR)((Value >> 24) & 0x1);
	return TRUE;
}


BOOLEAN MtAsicGetMcuStatus(RTMP_ADAPTER *pAd, MCU_STAT State)
{
	UINT32 value;

	HW_IO_READ32(pAd, TOP_MISC2, &value);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TOP_MSIC = %x\n", value));

	/* check ram code if running, if it is, need to do optional cmd procedure */
	switch (State) {
	case METH1_RAM_CODE: {
		if ((value & 0x02) == 0x02)
			return TRUE;
	}
	break;

	case METH1_ROM_CODE: {
		if ((value & 0x01) == 0x01 && !(value & 0x02))
			return TRUE;
	}
	break;

	case METH3_FW_RELOAD: {
		if ((value & (BIT0|BIT1|BIT2)) == (BIT0|BIT1|BIT2))
			return TRUE;
	}
	break;

	case METH3_INIT_STAT: {
		if ((value & BIT0) == 0x0)
			return TRUE;
	}
	break;

	case METH3_FW_PREPARE_LOAD: {
		if ((value & BIT0) == BIT0)
			return TRUE;
	}
	break;

	case METH3_FW_RUN: {
		if ((value & (BIT1 | BIT2)) == (BIT1 | BIT2))
			return TRUE;
	}
	break;

	default:
		return FALSE;
	}

	return FALSE;
}

INT32 MtAsicGetFwSyncValue(RTMP_ADAPTER *pAd)
{
	UINT32 value;
#define SW_SYN0 0x81021250
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd)) {
		UINT32 RemapBase, RemapOffset;
		UINT32 RestoreValue;

		RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(SW_SYN0) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(SW_SYN0);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &value);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	}

#endif
#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)

	if (IS_RBUS_INF(pAd) || IS_USB_INF(pAd) || IS_SDIO_INF(pAd)) {
		MAC_IO_READ32(pAd, SW_SYN0, &value);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Current SW_SYN0(%d)\n", __func__, value));
	}

#endif
	return value;
}

VOID MtAsicSetSMPSByDriver(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Smps)
{
	struct wtbl_entry tb_entry;
	struct wtbl_1_struc wtbl_1;

	os_zero_mem((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
	os_zero_mem(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cannot found WTBL2/3/4 for WCID(%d)\n",
				 __func__, Wcid));
		return;
	}

	/* WTBL1.dw2 bit19, support Dynamic SMPS */
	HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 8, &wtbl_1.wtbl_1_d2.word);
	wtbl_1.wtbl_1_d2.field.smps = Smps;
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, wtbl_1.wtbl_1_d2.word);
}

VOID MtAsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Smps)
{
	MtAsicSetSMPSByDriver(pAd, Wcid, Smps);
}

INT32 MtAsicSetDevMacByDriver(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	UINT32 val;
	ULONG own_mac_reg_base = RMAC_OMA0R0;

	/* We only need to consider BandIdx in FW offload case */
	if (Active) {
		val = (OwnMacAddr[0]) | (OwnMacAddr[1]<<8) |  (OwnMacAddr[2]<<16) | (OwnMacAddr[3]<<24);
		MAC_IO_WRITE32(pAd, own_mac_reg_base + (OwnMacIdx * 8), val);
		val = OwnMacAddr[4] | (OwnMacAddr[5]<<8) | (1 << 16);
		MAC_IO_WRITE32(pAd, (own_mac_reg_base + 4) + (OwnMacIdx * 8), val);
	}

	return NDIS_STATUS_SUCCESS;
}

INT32 MtAsicSetBssidByDriver(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T bss_info_argument)
{
	UINT32 val;
	ULONG curr_bssid_reg_base = RMAC_CB0R0;
#ifdef CONFIG_AP_SUPPORT
	UINT8 BssIdx = 0;
#endif
	UCHAR OwnMacIdx = bss_info_argument.OwnMacIdx;
	UINT8 Active = FALSE;
	UCHAR *Bssid = bss_info_argument.Bssid;

	if (bss_info_argument.bss_state >= BSS_ACTIVE)
		Active = TRUE;

	if (OwnMacIdx < HW_BSSID_MAX) {
		if (Active) {
			val = (UINT32)((Bssid[0]) |
						   (UINT32)(Bssid[1] << 8) |
						   (UINT32)(Bssid[2] << 16) |
						   (UINT32)(Bssid[3] << 24));
			MAC_IO_WRITE32(pAd, curr_bssid_reg_base + (OwnMacIdx * 8), val);
			val = (UINT32)(Bssid[4]) | (UINT32)(Bssid[5] << 8) | (1 << 16);
			MAC_IO_WRITE32(pAd, (curr_bssid_reg_base + 4) + (OwnMacIdx * 8), val);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	else {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		BssIdx = (OwnMacIdx & ~cap->ExtMbssOmacStartIdx);
		MtSmacSetExtMbssEnableCR(pAd, BssIdx, Active);/* enable rmac 0_1~0_15 bit */
		MtSmacSetMbssHwCRSetting(pAd, BssIdx, Active);/* enable lp timing setting for 0_1~0_15 */
	}

#endif
	return NDIS_STATUS_SUCCESS;
}

INT32 MtAsicSetStaRecByDriver(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg)
{
	/* Not supported this fucntion in driver */
	return NDIS_STATUS_SUCCESS;
}


#ifdef LINUX
BOOLEAN MtSmacAsicSetDmaFqcr(
	RTMP_ADAPTER *pAd,
	UINT8 OmacIdx,
	UINT32 filter_mode,/* 0: use widx, 1: use OMAC */
	UCHAR dest_port,
	UCHAR dest_queue,
	UCHAR target_queue,
	UCHAR widx /* only work if filter mode is by widx. */
)
{
	BOOLEAN flushResult = TRUE;
	UINT32 val = 0, temp = 0, own_mac = 0;
	UCHAR j = 0;

	val = (DMA_FQCR0_FQ_EN |
		   filter_mode |
		   DMA_FQCR0_FQ_DEST_QID(dest_queue) |
		   DMA_FQCR0_FQ_DEST_PID(dest_port) |
		   DMA_FQCR0_FQ_TARG_QID(target_queue));

	if (filter_mode == DMA_FQCR0_FQ_MODE_OMAC) {
		own_mac = OmacIdx;
		val = val | DMA_FQCR0_FQ_TARG_OM(own_mac);
	} else if (filter_mode == DMA_FQCR0_FQ_MODE_WIDX)
		val = val | DMA_FQCR0_FQ_TARG_WIDX(widx);

	MAC_IO_WRITE32(pAd, DMA_FQCR0, val);

	while (1) {
		MAC_IO_READ32(pAd, DMA_FQCR0, &temp);/* check flush status */

		if (temp & DMA_FQCR0_FQ_EN) {
			j++;
			os_msec_delay(1);

			if (j > 200) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s, flush take too long!\n", __func__));
				flushResult = FALSE;
				return flushResult;
			}
		} else
			break;
	}

	/* check filter resilt */
	MAC_IO_READ32(pAd, DMA_FQCR1, &temp);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("flush result = %x\n", temp));

	if (temp == 0xffffff) {
		flushResult = FALSE;
		return flushResult;
	}

	return flushResult;
}


BOOLEAN MtSmacAsicSetArbBcnQCR(
	RTMP_ADAPTER *pAd,
	UINT8 OmacIdx,
	BOOLEAN enable
)
{
	UINT32 val = 0, temp = 0, own_mac = 0, operBit = 0;
	UCHAR j = 0;
	BOOLEAN flushResult = TRUE;

	own_mac = OmacIdx;

	if (own_mac > HW_BSSID_MAX) {
		/*ext start from bit16. ownMac is start from 0x11 */
		operBit = own_mac - 1;
	} else
		operBit = own_mac;

	val = val | (1 << operBit);

	if (enable == FALSE) {
		/* Flush Beacon Queue */
		MAC_IO_WRITE32(pAd, ARB_BCNQCR1, val);

		while (1) {
			/* check bcn_flush cr status */
			MAC_IO_READ32(pAd, ARB_BCNQCR1, &temp);

			if (temp & val) {
				j++;
				os_msec_delay(1);

				if (j > 200) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("%s, bcn_flush too long!\n", __func__));
					flushResult = FALSE;
					return flushResult;
				}
			} else
				break;
		}
	} else if (enable == TRUE) {
		MAC_IO_READ32(pAd, ARB_BCNQCR0, &temp);/* re-enable bcn_start */
		temp = temp | val;
		MAC_IO_WRITE32(pAd, ARB_BCNQCR0, temp);
	}

	return flushResult;
}

BOOLEAN MtSmacAsicEnableBeacon(RTMP_ADAPTER *pAd, VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BOOLEAN OperationResult = TRUE;
	BCN_BUF_STRUC *bcn_info = &wdev->bcn_buf;

	if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW) {
		/* FW help to disable beacon. */
	} else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT) {
		OperationResult = MtSmacAsicSetArbBcnQCR(pAd, wdev->OmacIdx, TRUE);

		if (OperationResult == FALSE)
			return FALSE;

		/* beacon start for USB/SDIO */
		asic_bss_beacon_start(pAd);
	}

	return TRUE;
}

BOOLEAN MtSmacAsicDisableBeacon(RTMP_ADAPTER *pAd, VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BCN_BUF_STRUC *bcn_info = &wdev->bcn_buf;
	BOOLEAN OperationResult = TRUE;

	if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW) {
		/* FW help to disable beacon. */
	} else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT) {
		/* beacon stopfor USB/SDIO */
		asic_bss_beacon_stop(pAd);

		OperationResult = MtSmacAsicSetArbBcnQCR(pAd, wdev->OmacIdx, FALSE);

		if (OperationResult == FALSE)
			return FALSE;

		OperationResult = MtSmacAsicSetDmaFqcr(
							  pAd,
							  wdev->OmacIdx,
							  DMA_FQCR0_FQ_MODE_OMAC,
							  ENUM_SWITCH_PORT_3,
							  ENUM_SWITCH_FREE_Q_8,
							  ENUM_LMAC_TX_BEACON_Q_7,
							  0);

		if (OperationResult == FALSE)
			return FALSE;

		RTMP_SEM_LOCK(&pAd->BcnRingLock);
		bcn_info->bcn_state = BCN_TX_IDLE;
		RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
	}

	return TRUE;
}

#endif /*LINUX*/

UINT32 MtAsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
	UINT32	msdr16;

	MAC_IO_READ32(pAd, MIB_MSDR16, &msdr16);
	msdr16 &= 0x00ffffff;
	return msdr16;
}


INT MtAsicGetTsfTimeByDriver(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	UINT32 Value = 0;

	MAC_IO_READ32(pAd, LPON_T0CR, &Value);
	Value = (Value & TSF_TIMER_HW_MODE_MASK) | TSF_TIMER_VALUE_READ;/* keep HW mode value. */
	/* Value = Value | TSF_TIMER_VALUE_READ; */
	MAC_IO_WRITE32(pAd, LPON_T0CR, Value);
	MAC_IO_READ32(pAd, LPON_UTTR0, low_part);
	MAC_IO_READ32(pAd, LPON_UTTR1, high_part);
	return TRUE;
}



/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx)
{
	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;

	MtCmdSlotTimeSet(pAd, (UINT8)SlotTime, (UINT8)SifsTime, (UINT8)RifsTime, (UINT16)EifsTime, BandIdx);
}

#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
VOID MtAsicGetTxTscByDriver(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	/* Do nothing for testmode */
}
#else
VOID MtAsicGetTxTscByDriver(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	USHORT Wcid;
	struct wtbl_entry tb_entry;
	UINT32 addr = 0, val = 0;

	GET_GroupKey_WCID(wdev, Wcid);
	os_zero_mem(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
				 __func__, Wcid));
		return;
	}

	addr = pAd->mac_ctrl.wtbl_base_addr[1] + Wcid * pAd->mac_ctrl.wtbl_entry_size[1];
	HW_IO_READ32(pAd, addr, &val);
	*pTxTsc	= val & 0xff;
	*(pTxTsc+1) = (val >> 8) & 0xff;
	*(pTxTsc+2) = (val >> 16) & 0xff;
	*(pTxTsc+3) = (val >> 24) & 0xff;
	HW_IO_READ32(pAd, addr+4, &val);
	*(pTxTsc+4) = val & 0xff;
	*(pTxTsc+5) = (val >> 8) & 0xff;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\n",
			 __func__, Wcid,
			 *pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));
}
#endif

INT MtAsicTOPInit(RTMP_ADAPTER *pAd)
{
#if defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA)
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->chk_top_default_cr_setting)
		ops->chk_top_default_cr_setting(pAd);
	if (ops->chk_hif_default_cr_setting)
		ops->chk_hif_default_cr_setting(pAd);
#endif
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
	/* TODO: shiang-7603 */
#ifdef MT7628_FPGA
	UINT32 mac_val;
	/* enable MAC circuit */
	HW_IO_READ32(pAd, 0x2108, &mac_val);
	mac_val &= (~0x7ff0);
	HW_IO_WRITE32(pAd, 0x2108, mac_val);
	mac_val = 0x3e013;
	MAC_IO_WRITE32(pAd, 0x2d004, mac_val);
#endif /* MT7628_FPGA */
	MAC_IO_WRITE32(pAd, 0x24088, 0x900); /* Set 40MHz Clock */
	MAC_IO_WRITE32(pAd, 0x2d034, 0x64180003);	/* Set 32k clock, this clock is used for lower power. */
#endif /* defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)*/
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_SBTOR1;
	BOOLEAN any_other_mbss_enable = FALSE;

	ASSERT(mbss_idx >= 1);

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd, bssid_reg_base, &Value);
		Value |= SBSS_TBTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
		MAC_IO_READ32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value |= SUB_BSSID0_TIME_OFFSET_n(mbss_idx * BCN_TRANSMIT_ESTIMATE_TIME);
		Value |= TBTT0_n_INT_EN;
		Value |= PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
		pAd->ApCfg.ext_mbss_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_enable_bitmap)
			any_other_mbss_enable = TRUE;

		/* if there is any ext bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd, bssid_reg_base, &Value);

		if (any_other_mbss_enable == TRUE)
			Value |= SBSS_TBTT0_TSF0_EN;
		else
			Value &= ~SBSS_TBTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
		MAC_IO_READ32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value &= ~TBTT0_n_INT_EN;
		Value &= ~PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
	}
}

VOID MtSmacSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtAsicSetMbssLPOffset(pAd, mbss_idx, enable);
}

VOID MtSmacSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 regValue;

	if (enable) {
		MAC_IO_READ32(pAd, RMAC_ACBEN, &regValue);
		regValue |=  (1 << mbss_idx);
		MAC_IO_WRITE32(pAd, RMAC_ACBEN, regValue);
	} else {
		MAC_IO_READ32(pAd, RMAC_ACBEN, &regValue);
		regValue &=  ~(1 << mbss_idx);
		MAC_IO_WRITE32(pAd, RMAC_ACBEN, regValue);
	}
}


VOID MtAsicSetExtTTTTLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[mbss_idx];
	UINT32 first_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[1];
	BOOLEAN any_other_mbss_tttt_enable = FALSE;

	if (mbss_idx == 0)
		return;

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd, first_reg_base, &Value);
		Value |= SBSS_TTTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd, first_reg_base, Value);
		MAC_IO_READ32(pAd, bssid_reg_base, &Value);
		Value &= ~SUB_BSSID0_TTTT_OFFSET_n_MASK;
		Value |= DEFAULT_TTTT_OFFSET_IN_MS;
		Value |= TTTT0_n_INT_EN;
		Value |= PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_tttt_enable_bitmap)
			any_other_mbss_tttt_enable = TRUE;

		/* if there is any ext bssid is enable. this bit shall be 1 always. */
		MAC_IO_READ32(pAd, first_reg_base, &Value);

		if (any_other_mbss_tttt_enable == TRUE)
			Value |= SBSS_TTTT0_TSF0_EN;
		else
			Value &= ~SBSS_TTTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd, first_reg_base, Value);
		MAC_IO_READ32(pAd, bssid_reg_base, &Value);
		Value &= ~TTTT0_n_INT_EN;
		Value &= ~PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
	}
}

VOID MtSmacSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtAsicSetExtTTTTLPOffset(pAd, mbss_idx, enable);
}
#endif /* CONFIG_AP_SUPPORT */


#ifndef COMPOS_TESTMODE_WIN
VOID MtSetTmrCal(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR TmrType,
	IN UCHAR Channel,
	IN UCHAR Bw)
{
	UINT32  value = 0;

	MAC_IO_READ32(pAd, TMAC_B0BRR0, &value);

	if (TmrType == TMR_DISABLE)
		/* Enanle Spatial Extension for ACK/BA/CTS after TMR Disable*/
		value |= BSSID00_RESP_SPE_EN;
	else
		/* Disable Spatial Extension for ACK/BA/CTS when TMR Enable*/
		value &= ~BSSID00_RESP_SPE_EN;

	MAC_IO_WRITE32(pAd, TMAC_B0BRR0, value);
	MtCmdTmrCal(pAd,
				TmrType,
				(Channel > 14 ? _A_BAND : _G_BAND),
				Bw,
				0,/* Ant 0 at present */
				TmrType);
}

VOID MtSetTmrCR(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR TmrType)
{
	UINT32  value = 0;

	MAC_IO_READ32(pAd, RMAC_TMR_PA, &value);
	/* Clear role bit */
	value &= ~RMAC_TMR_ROLE;

	if (TmrType == TMR_DISABLE)
		value &= ~RMAC_TMR_ENABLE;
	else {
		value |= RMAC_TMR_ENABLE;

		if (TmrType == TMR_RESPONDER) {
			value |= RMAC_TMR_ROLE;
			/* Set type/sub_type to Action frame */
			value &= 0xffffffc0;    /* clear type/sub_type field */
			value |= (SUBTYPE_ACTION << 2) | FC_TYPE_MGMT;
		}
	}

	MAC_IO_WRITE32(pAd, RMAC_TMR_PA, value);
}
#endif


#ifdef LINUX
INT MtSmacAsicArchOpsInit(RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	arch_ops->archDisableBeacon = MtSmacAsicDisableBeacon;
	arch_ops->archEnableBeacon = MtSmacAsicEnableBeacon;
#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetMbssWdevIfAddr = MtAsicSetMbssWdevIfAddrGen1;
	arch_ops->archSetMbssHwCRSetting = MtSmacSetMbssHwCRSetting;
	arch_ops->archSetExtTTTTHwCRSetting = MtSmacSetExtTTTTHwCRSetting;
	arch_ops->archSetExtMbssEnableCR = MtSmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archGetTsfTime = MtAsicGetTsfTimeByDriver;
	arch_ops->archSetPreTbtt = MtAsicSetPreTbtt;
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	arch_ops->archSetReptFuncEnable = MtAsicSetReptFuncEnableByDriver;
	arch_ops->archInsertRepeaterEntry = MtAsicInsertRepeaterEntry;
	arch_ops->archRemoveRepeaterEntry = MtAsicRemoveRepeaterEntry;
	arch_ops->archInsertRepeaterRootEntry = MtAsicInsertRepeaterRootEntry;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
	arch_ops->archDisableSync = MtAsicDisableSyncByDriver;
	arch_ops->archSetSyncModeAndEnable = MtAsicEnableBssSyncByDriver;
	arch_ops->archRxHeaderTransCtl = NULL;
	arch_ops->archRxHeaderTaranBLCtl = NULL;
	return TRUE;
}
#endif /*LINUX*/
