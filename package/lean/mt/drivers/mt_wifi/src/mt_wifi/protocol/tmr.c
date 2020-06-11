/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    tmr.c

    Abstract:
    Support Timing Measurment function.

    Who             When            What
    --------------  ----------      ----------------------------------------------
    Carter          2014-1120       created
*/

#ifdef MT_MAC

#include "rt_config.h"

/*
 *   the format is for QA tool, TMR as prefix,
 *   QA will latch the prefix string then output to file.
 */
void tmr_raw_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen));

	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TMR 0x%04x : ", x));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%02x ", ((unsigned char)pt[x])));

		if (x % 16 == 15)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
}

#ifdef FTM_SUPPORT
inline VOID FillTodToa(
	IN UINT32 tod_high,
	IN UINT32 tod_low,
	IN UINT32 toa_high,
	IN UINT32 toa_low,
	OUT UINT64 * p_tod,
	OUT UINT64 * p_toa)
{
	*p_tod = (((UINT64)(tod_high)) << 32) | tod_low;
	*p_toa = (((UINT64)(toa_high)) << 32) | toa_low;
}
#endif /* FTM_SUPPORT */

VOID TmrReportParser(RTMP_ADAPTER *pAd, TMR_FRM_STRUC *tmr, BOOLEAN fgFinalResult, UINT32 TOAECalibrationResult)
{
	struct _RMAC_RXD_0_TMR *ptmr_d0 = &tmr->TmrD0;
	UINT32 *ptod_0 = &tmr->ToD0;
	UINT32 *ptoa_0 = &tmr->ToA0;
	TMR_D_6 *tmr_d6 = &tmr->TmrD6;
	TMR_D_2 *ptmr_d2 = &tmr->TmrD2;
	TMR_D_1 *tmr_d1 = &tmr->TmrD1;
	UCHAR   *pta_16 = (UCHAR *)&tmr->Ta16;
#ifdef TMR_GEN2
	TMR_D_7 *tmr_d7 = &tmr->TmrD7;
	TMR_D_8 *tmr_d8 = &tmr->TmrD8;
	INT32 LtfSyncAddr = 0;
	UINT32 DacOutput = 0;
#endif /* TMR_GEN2 */
	UCHAR   PeerAddr[MAC_ADDR_LEN] = {0};
	static UINT32 lastest_TOA;
	static UINT32 lastest_sn;
	UINT32 tod_fine = tmr_d1->field_init.TodFine;
	UINT32 tod_low = 0;
	UINT32 toa_low = 0;
	UINT32 delta_low = 0;
	UINT32 tod_high = 0;
	UINT32 toa_high = 0;
	UINT32 delta_high = 0;
	UCHAR dbg_lvl = DBG_LVL_OFF;
	UCHAR dbg_lvl_error = DBG_LVL_OFF;
#ifdef FTM_SUPPORT
	UINT64 tod = 0;
	UINT64 toa = 0;
	UINT64 delta = 0;
#endif /* FTM_SUPPORT */

	if (pAd->pTmrCtrlStruct == NULL)
		return;

	if (pAd->pTmrCtrlStruct->TmrEnable == TMR_DISABLE)
		return;

	pAd->pTmrCtrlStruct->TmrCalResult = TOAECalibrationResult;

#ifdef FTM_SUPPORT
	if (ptmr_d0->IR == TMR_IR0_TX)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TMR(%d%d)PID:0x%02X\n",
			 ptmr_d0->ToaVld, ptmr_d0->TodVld, tmr_d1->field_init.Pid));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TMR: TYPE=%x, SUB_TYPE=%x, SN=0x%04X\n",
			 ptmr_d0->Type, ptmr_d0->SubType, ptmr_d2->field.SnField));

	dbg_lvl = DBG_LVL_INFO;
	dbg_lvl_error = DBG_LVL_WARN;

	if (DebugLevel >= dbg_lvl)
#endif /* FTM_SUPPORT */
		tmr_raw_dump("TMR RAW data: ", (UCHAR *)tmr, sizeof(TMR_FRM_STRUC));

	tod_low = tmr->ToD0;
	toa_low = tmr->ToA0;
	tod_high = tmr->TmrD6.field.ToD32;
	toa_high = tmr->TmrD6.field.ToA32;
#ifdef TMR_GEN2
	LtfSyncAddr = tmr_d8->field_ofdm.LtfSyncStartAddr;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("LtfSyncAddr = %x, in unit(12.5ns)\n", LtfSyncAddr));
	DacOutput = tmr_d7->field.DacOutput;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DacOutput = %x\n", DacOutput));

	if (fgFinalResult == FALSE) {
		toa_low = toa_low + LtfSyncAddr * 50; /* for now, always use CBW 80. */
		tod_low = tod_low - (tod_fine * 33 / 4) + (DacOutput * 33 / 8);
	}

#endif /* TMR_GEN2 */

	/* calculate delta time */
	if (ptmr_d0->IR == TMR_IR1_RX) {
		delta_low = tod_low - toa_low;
		delta_high = tod_high - toa_high;
	} else {
		delta_low = toa_low - tod_low;
		delta_high = toa_high - tod_high;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
		("DWORD_0: ByteCnt=%d, NC=%d, TMF=%d, ToaVld=%d, TodVld=%d, tod_fine=%x\n",
		ptmr_d0->RxByteCnt, ptmr_d0->Nc, ptmr_d0->Tmf,
		ptmr_d0->ToaVld, ptmr_d0->TodVld, tod_fine));

#ifdef FTM_SUPPORT
	FillTodToa(tod_high, tod_low, toa_high, toa_low, &tod, &toa);
	FtmConvertTodToa(pAd, &tod, &toa);

	if (ptmr_d0->IR == TMR_IR1_RX)
		delta = tod - toa;
	else
		delta = toa - tod;

#endif /* FTM_SUPPORT */

	if (ptmr_d0->IR == TMR_IR1_RX) {
		/* TMR Responder, Rx case */
		if (lastest_sn == ptmr_d2->field.SnField)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl_error, ("##### latest sn is same as last time\n"));

		lastest_sn = ptmr_d2->field.SnField;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TYPE=%x, SUB_TYPE=%x, SN=%x\n",
			 ptmr_d0->Type, ptmr_d0->SubType, ptmr_d2->field.SnField));

		PeerAddr[0] = ptmr_d2->field.Ta0;
		PeerAddr[1] = ptmr_d2->field.Ta0 >> 8;
		NdisCopyMemory(PeerAddr + 2, (UCHAR *)pta_16, 4);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
			("Readable TA = %02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(PeerAddr)));

	} else {
		/* TMR Initiator, Tx case */
		if (*ptoa_0 == lastest_TOA) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("##### latest TOA is same as last time\n"));
			/* 2015.06.22 location plugfest: do not return */
#ifndef FTM_SUPPORT
			return;
#endif
		}

		lastest_TOA = *ptoa_0;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TOAECalibrationResult=0x%X\n", TOAECalibrationResult));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_4: TOD[0:31]=0x%x\n", *ptod_0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_6: TOD[32:47]=0x%x\n", tmr_d6->field.ToD32));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_5: TOA[0:31]=0x%x\n", *ptoa_0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_6: TOA[32:47]=0x%x\n", tmr_d6->field.ToA32));
#ifndef FTM_SUPPORT
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TMR Report: ir = %d, delta_high = %d, delta_low = %d\n\n",
			 ptmr_d0->IR, delta_high, delta_low));
#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TMR Report: ir = %d, delta = 0x%016llX (unit converted)\n\n",
			 ptmr_d0->IR, delta));
#endif /* FTM_SUPPORT */
#ifdef FTM_SUPPORT
	{
		PFTM_PEER_ENTRY pFtmEntry = NULL;

		/* Get Peer FTM Entry */
		if (ptmr_d0->IR == TMR_IR1_RX)
			pFtmEntry = FtmEntrySearch(pAd, PeerAddr);
		else
			pFtmEntry = FtmGetPidPendingNode(pAd, tmr_d1->field_init.Pid);

		if (!pFtmEntry) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO, ("%s()#%d: invalid FTM entry !!\n", __func__, __LINE__));
			return;
		}

		/* Record TMR */
		if (ptmr_d0->IR == TMR_IR0_TX) {
			/* Tx case: for FTM responder */
			if (pFtmEntry->bGotTmr)
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN, ("%s()#%d: bGotTmr again\n", __func__, __LINE__));
			else
				pFtmEntry->bGotTmr = TRUE;

			NdisCopyMemory(&pFtmEntry->Tmr.HwReport, tmr, sizeof(TMR_FRM_STRUC));
			pFtmEntry->Tmr.toa = toa;
			pFtmEntry->Tmr.tod = tod;
		}

#ifdef FTM_INITIATOR
		else {
			/* Rx case: for FTM initiator */
			ULONG IrqFlags = 0;
			PFTM_MAPPING pNode = NULL;

			if (os_alloc_mem(NULL, (UCHAR **)&pNode, sizeof(FTM_MAPPING)) == NDIS_STATUS_FAILURE)
				return;

			NdisZeroMemory(pNode, sizeof(FTM_MAPPING));
			RTMP_IRQ_LOCK(&pFtmEntry->RxTmrQLock, IrqFlags);
			NdisCopyMemory(&pNode->tmr.HwReport, tmr, sizeof(TMR_FRM_STRUC));
			pNode->tmr.toa = toa;
			pNode->tmr.tod = tod;
			pNode->sn = ptmr_d2->field.SnField;
			DlListAddTail(&pFtmEntry->RxTmrQ, &pNode->list);
			RTMP_IRQ_UNLOCK(&pFtmEntry->RxTmrQLock, IrqFlags);
		}

#endif /* FTM_INITIATOR */
	}
#endif /* FTM_SUPPORT */
	return;
}

INT TmrCtrlInit(RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver)
{
	INT Ret = NDIS_STATUS_SUCCESS;
	TMR_CTRL_STRUCT *TmrCtrlStructMem = NULL;

	if (pAd->pTmrCtrlStruct != NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				 ("%s(): pTmrCtrlStruct is allocated.\n", __func__));
	} else {
		Ret = os_alloc_mem(pAd, (PUCHAR *)&TmrCtrlStructMem, sizeof(TMR_CTRL_STRUCT));

		if (!TmrCtrlStructMem) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					 ("%s(): alloc TmrCtrl fail!\n", __func__));
			return Ret;
		}

		pAd->pTmrCtrlStruct = TmrCtrlStructMem;
	}

	if (!pAd->pTmrCtrlStruct)
		return NDIS_STATUS_FAILURE;

	if (TmrType == TMR_INITIATOR) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: enable TMR report, as Initialiter\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: enable TMR report, as Responder\n", __func__));
	}

	pAd->pTmrCtrlStruct->TmrEnable = TmrType;
	MtSetTmrCR(pAd, TmrType);
	return Ret;
}

VOID TmrCtrlExit(RTMP_ADAPTER *pAd)
{
	if (pAd->pTmrCtrlStruct != NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: disable TMR report\n", __func__));
		pAd->pTmrCtrlStruct->TmrEnable = TMR_DISABLE;
		os_free_mem(pAd->pTmrCtrlStruct);
		pAd->pTmrCtrlStruct = NULL;
	}
}

/*
 *   Carter, for now, MT7615 TMR mode is global control.
 *   we shall refine DBDC case in someday.
*/
/* VOID MtSetTmrEnable(RTMP_ADAPTER *pAd, UCHAR TmrType) */
VOID TmrCtrl(RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver)
{
	if (TmrType != TMR_DISABLE)
		TmrCtrlInit(pAd, TmrType, Ver);
	else
		TmrCtrlExit(pAd);

	switch (TmrType) {
	case TMR_INITIATOR:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: enable TMR report, as Initialiter\n", __func__));
		AsicSetTmrCR(pAd, TmrType, 0);
		AsicSetTmrCR(pAd, TmrType, 1);
		break;

	case TMR_RESPONDER:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: enable TMR report, as Responser\n", __func__));
		AsicSetTmrCR(pAd, TmrType, 0);
		AsicSetTmrCR(pAd, TmrType, 1);
		break;

	case TMR_DISABLE:
	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: disable TMR report\n", __func__));
		AsicSetTmrCR(pAd, TmrType, 0);
		AsicSetTmrCR(pAd, TmrType, 1);
		break;
	}

	return;
}

#endif /* MT_MAC */

