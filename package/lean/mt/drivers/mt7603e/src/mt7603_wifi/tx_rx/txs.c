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
	txs.c
*/

#include	"rt_config.h"

#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
	DBGPRINT(RT_DEBUG_ERROR, ("%s():txs d0 me : %d\n", __FUNCTION__, txs_d0->ME));
	if(txs_d0->ME == 0)
	{
		pEntry->TdlsTxFailCount=0;
	}
	else
	{	
		pEntry->TdlsTxFailCount++;
	}

	if(pEntry->TdlsTxFailCount > 15)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): TdlsTxFailCount > 15!!  teardown link with (%02X:%02X:%02X:%02X:%02X:%02X)!!\n"
			, __FUNCTION__,PRINT_MAC(pEntry->Addr)));
		pEntry->TdlsTxFailCount=0;
		cfg_tdls_auto_teardown(pAd,pEntry->Addr);
	}

	return 0;
}
#endif /*CFG_TDLS_SUPPORT*/
#ifdef UAPSD_SUPPORT
INT32 UAPSDTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	//TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	//TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
	//TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	//TXS_D_4 *txs_d4 = &txs_entry->txs_d4;
	pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];

	if(pEntry->UAPSDTxNum > 0)
		pEntry->UAPSDTxNum--;
	else if(pEntry->UAPSDTxNum == 0)
	{
		//this is the txs of eosp
		pEntry->bAPSDFlagEOSPOK = 0;
		UAPSD_SP_END(pAd, pEntry);

		DBGPRINT(RT_DEBUG_TRACE, ("uapsd> [new] TxS close SP ()!\n"));

	}

	return 0;
}
#endif
INT32 BcnTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{	
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	//TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
#if defined(CONFIG_AP_SUPPORT) && defined(DBG)
	TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
#endif /* defined(CONFIG_AP_SUPPORT) && defined(DBG) */
	//TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
	//TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
#if defined(CONFIG_AP_SUPPORT) && defined(DBG)
	TXS_D_4 *txs_d4 = &txs_entry->txs_d4;
#endif /* defined(CONFIG_AP_SUPPORT) && defined(DBG) */

#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP) 
		)
		{
			UCHAR bss_idx = 0;
			BSS_STRUCT *pMbss = NULL;
			bss_idx = Priv;
			pMbss = &pAd->ApCfg.MBSSID[bss_idx];
			pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
			pMbss->bcn_not_idle_time = 0;
			DBGPRINT(RT_DEBUG_INFO, ("%s():idx: %x, change state as idle\n", __FUNCTION__, bss_idx));

#ifdef DBG
			pMbss->TXS_TSF[pMbss->timer_loop] = txs_d1->timestamp;
			pMbss->TXS_SN[pMbss->timer_loop] = txs_d4->sn;
#endif /* DBG */
			pMbss->timer_loop++;
			if (pMbss->timer_loop >= MAX_TIME_RECORD)
				pMbss->timer_loop = 0;

			return 0;
		}
#endif /* CONFIG_AP_SUPPORT */

	return 0;
}

INT32 NullFramePM1TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{	

	return 0;
}

INT32 NullFramePM0TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{

	return 0;
}

INT32 PsDataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
 
        if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_TR_TABLE))
        {      	
                return 0;
        }


	if (pAd->MacTab.tr_entry[txs_d3->wlan_idx].PsDeQWaitCnt) {
		/* After a successfull Tx of dequeued PS data, we clear PsDeQWaitCnt */
		pAd->MacTab.tr_entry[txs_d3->wlan_idx].PsDeQWaitCnt = 0;
	}

	return 0;
}


static INT32 MgmtTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}


static INT32 CtrlTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}


static INT32 DataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}

#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)
INT32 EapReqIdTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	MAC_TABLE_ENTRY *pEntry;
 
	if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_MAC_TABLE))
	{
		return 0;
	}

	DBGPRINT(RT_DEBUG_TRACE,("%s: (RE=%d, LE=%d, ME=%d), wlan_idx = %d\n",__FUNCTION__, txs_d0->RE, txs_d0->LE, txs_d0->ME, txs_d3->wlan_idx));  

	if ((txs_d0->RE == 0) && (txs_d0->LE == 0) && (txs_d0->ME == 0))
	{
		pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
		if (pEntry->bEapReqIdRetryTimerRunning)
		{
			pEntry->bEapReqIdRetryTimerRunning = FALSE;
		}
	}

	return 0;
}
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

#if defined(MT_MAC) && defined(CONFIG_AP_SUPPORT)
INT32 APQoSNullTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	MAC_TABLE_ENTRY *pEntry;
 
	if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_MAC_TABLE))
		return 0;

	DBGPRINT(RT_DEBUG_TRACE,("%s: (RE=%d, LE=%d, ME=%d), wlan_idx = %d\n",__FUNCTION__, txs_d0->RE, txs_d0->LE, txs_d0->ME, txs_d3->wlan_idx));  

	if ((txs_d0->RE == 0) && (txs_d0->LE == 0) && (txs_d0->ME == 0))
	{
		pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
		DBGPRINT(RT_DEBUG_TRACE,("%s: wlan_idx = %d,  pEntry->NoDataIdleCount=%lu go to clear!!\n",__FUNCTION__, txs_d3->wlan_idx, pEntry->NoDataIdleCount));
		pEntry->NoDataIdleCount	= 0 ;
	}

	return 0;
}
#endif /* defined(MT_MAC) && defined(CONFIG_AP_SUPPORT) */

#ifdef WH_EZ_SETUP
INT32 ez_action_txs_handler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	BOOLEAN TxError;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev = NULL;
	
	if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_TR_TABLE))
	{
		return 0;
	}
	TxError = (txs_d0->ME || txs_d0->RE || txs_d0->LE || txs_d0->BE || txs_d0->txop || txs_d0->ps || txs_d0->baf);
	pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
	wdev = pEntry->wdev;
	if(wdev && IS_EZ_SETUP_ENABLED(wdev)){
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Raghav: Received TXS: Wcid=%d\n", txs_d3->wlan_idx));
		if(TxError && IS_EZ_SETUP_ENABLED(wdev)){
			ez_handle_action_txstatus(pAd, txs_d3->wlan_idx);
		}
	}

	return 0;
}
#endif /* defined(MT_MAC) && defined(CONFIG_AP_SUPPORT) */
INT32 APAssocRespTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
    TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
    TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
    TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
    
    MAC_TABLE_ENTRY *pEntry = NULL;

    if ( txs_d3 == NULL )
    {
        DBGPRINT(RT_DEBUG_ERROR,("===[%s] ERROR: txs3 = NULL\n", __FUNCTION__));
        return 0;
    }
	if ( txs_d3->wlan_idx >= MAX_LEN_OF_MAC_TABLE )
    {
        DBGPRINT(RT_DEBUG_ERROR,("===[%s]ERROR:  txs_d3->wlan_idx(%d) >= MAX_LEN_OF_TR_TABLE\n", __FUNCTION__, txs_d3->wlan_idx));
        return 0;
    }
	DBGPRINT(RT_DEBUG_WARN, ("===[%s]: (RE=%d, LE=%d, ME=%d), wlan_idx = %d\n", __FUNCTION__,
		txs_d0->RE, txs_d0->LE, txs_d0->ME, txs_d3->wlan_idx));
		if ((txs_d0->RE == 0) && (txs_d0->LE == 0) && (txs_d0->ME == 0))
    {
        pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
        if(pEntry) 
		{
            pEntry->AuthAssocNotInProgressFlag = 1;
			DBGPRINT(RT_DEBUG_WARN, ("===[%s]: wlan_idx = %d,AuthAssocNotInProgressFlag = %d pEntry->bssid %02x:%02x:%02x:%02x:%02x:%02x \n",
				__FUNCTION__, txs_d3->wlan_idx, pEntry->AuthAssocNotInProgressFlag, PRINT_MAC(pEntry->bssid)));
        }
    } else {
			DBGPRINT(RT_DEBUG_WARN, ("===[%s] status  error returned \n", __FUNCTION__));
	}
    return 0;
}

INT32 InitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	DBGPRINT(RT_DEBUG_TRACE,("%s %d\n",__FUNCTION__,__LINE__));  
	
	/* Per Pkt */
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
	}

	/* Per Pkt Type */
	for (Index = 0; Index < 3; Index++)
	{
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++)
		{
			NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
		}
	}

	for (Index = 0; Index < TXS_STATUS_NUM; Index++)
	{ 
		NdisZeroMemory(&TxSCtl->TxSStatus[Index], sizeof(TXS_STATUS));
	}

	AddTxSTypePerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_BEACON, TXS_FORMAT0, BcnTxSHandler);
	TxSTypeCtlPerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_BEACON, TXS_WLAN_IDX_ALL, 
							TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
	AddTxSTypePerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_ALL, TXS_FORMAT0, MgmtTxSHandler);
	AddTxSTypePerPktType(pAd, FC_TYPE_CNTL, SUBTYPE_ALL, TXS_FORMAT0, CtrlTxSHandler);
	AddTxSTypePerPktType(pAd, FC_TYPE_DATA, SUBTYPE_ALL, TXS_FORMAT0, DataTxSHandler);
	/* Assoc_resp sync */
	AddTxSTypePerPkt(pAd, PID_MGMT_ASSOC_RSP, TXS_FORMAT0, APAssocRespTxSHandler);
    TxSTypeCtlPerPkt(pAd, PID_MGMT_ASSOC_RSP, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
	/* PsDataTxSHandler */
	AddTxSTypePerPkt(pAd, PID_PS_DATA, TXS_FORMAT0, PsDataTxSHandler); 
	TxSTypeCtlPerPkt(pAd, PID_PS_DATA, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
#ifdef UAPSD_SUPPORT
#ifdef MT_MAC //ADD UAPSD TxsType
	if (pAd->chipCap.hif_type == HIF_MT) {
		//printk("ggggggggggggggggggggggggggggggggggg\n");
		AddTxSTypePerPkt(pAd, PID_UAPSD, TXS_FORMAT0, UAPSDTxSHandler);
		TxSTypeCtlPerPkt(pAd, PID_UAPSD, TXS_FORMAT0, FALSE, TRUE, FALSE, 0); 
	}
#endif //MT_MAC
#endif

#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	
	AddTxSTypePerPkt(pAd, PID_WSC_EAP, TXS_FORMAT0, EapReqIdTxSHandler); 
	TxSTypeCtlPerPkt(pAd, PID_WSC_EAP, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	 */

#if defined(MT_MAC) && defined(CONFIG_AP_SUPPORT)	
	AddTxSTypePerPkt(pAd, PID_QOS_NULL_FRAME, TXS_FORMAT0, APQoSNullTxSHandler); 
	TxSTypeCtlPerPkt(pAd, PID_QOS_NULL_FRAME, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
#endif /* defined(MT_MAC) && defined(CONFIG_AP_SUPPORT)	 */

#ifdef WH_EZ_SETUP
	AddTxSTypePerPkt(pAd,PID_EZ_ACTION, TXS_FORMAT0, ez_action_txs_handler);
	TxSTypeCtlPerPkt(pAd, PID_EZ_ACTION, TXS_FORMAT0, FALSE, TRUE, FALSE, TXS_DUMP_REPEAT);
#endif

	TxSCtl->TxSValid = TRUE;

	return 0;
}

INT32 ExitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;
	//TXS_STATUS *TxSStatus = NULL, *TmpTxSStatus = NULL;

	TxSCtl->TxSValid = FALSE;
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[Index], 
							TXS_TYPE, List) 
		{
			DlListDel(&TxSType->List);
			os_free_mem(NULL, TxSType);
		}
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		NdisFreeSpinLock(&TxSCtl->TxSTypePerPktLock[Index]);
	}

	for (Index = 0; Index < 3; Index++)
	{	
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++)
		{
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[Index][Index1], 
								TXS_TYPE, List) 
			{
				DlListDel(&TxSType->List);
				os_free_mem(NULL, TxSType);
			}
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			NdisFreeSpinLock(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
		}
	}

	return 0;
}

INT32 AddTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format,
						TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: %d \n", __FUNCTION__, __LINE__));
	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));
	
	if (!TxSType) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], 
										TXS_TYPE, List) 
	{
		if ((SearchTxSType->PktPid == PktPid) && (SearchTxSType->Format == Format))
		{
			DBGPRINT(RT_DEBUG_OFF, ("%s: already registered TxSType (PktPid = %d, Format = %d\n",
									 __FUNCTION__, PktPid, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], 
										&Flags);
			os_free_mem(NULL, TxSType);
			return -1;
		}
	}

	TxSType->Type = TXS_TYPE0;
	TxSType->PktPid = PktPid;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	
	DlListAddTail(&TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], &TxSType->List);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	return 0;
}


INT32 RemoveTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], 
						TXS_TYPE, List) 
	{
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format))
		{
			DlListDel(&TxSType->List);
			os_free_mem(NULL, TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], 
											&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	return -1;
}


INT32 TxSTypeCtlPerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format, BOOLEAN TxS2Mcu, 
						BOOLEAN TxS2Host, BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) 
	{
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format))
		{
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2McUStatusPerPkt &= ~(1 << PktPid);

			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2HostStatusPerPkt &= ~(1 << PktPid);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxSFormatPerPkt &= ~(1 << PktPid);

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;
		
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], 
											&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: can not find TxSType(PktPID = %d, Format = %d)\n", 
								__FUNCTION__, PktPid, Format));
	return -1;
}


INT32 AddTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
							UINT8 Format, TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;
	
	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) 
	{
		if ((SearchTxSType->PktType == PktType) && (SearchTxSType->PktSubType == PktSubType)
					&& (SearchTxSType->Format == Format))
		{
			DBGPRINT(RT_DEBUG_OFF, ("%s: already registered TxSType (PktType = %d, PktSubType = %d, Format = %d\n", __FUNCTION__, PktType, PktSubType, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			os_free_mem(NULL, TxSType);
			return -1;
		}
	}

	TxSType->Type = TXS_TYPE1;
	TxSType->PktType = PktType;
	TxSType->PktSubType = PktSubType;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	DlListAddTail(&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &TxSType->List);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	return 0;
}


INT32 RemoveTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType, 
										UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) 
	{
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
					&& (TxSType->Format == Format))
		{
			DlListDel(&TxSType->List);
			os_free_mem(NULL, TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	return -1;
}


INT32 TxSTypeCtlPerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType, UINT16 WlanIdx, 
							UINT8 Format, BOOLEAN TxS2Mcu, BOOLEAN TxS2Host, BOOLEAN DumpTxSReport,
							ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) 
	{
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
				&& (TxSType->Format == Format))
		{
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2McUStatusPerPktType[PktType] &= ~(1 << PktSubType);

			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2HostStatusPerPktType[PktType] &= ~(1 << PktSubType);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxSFormatPerPktType[PktType] &= ~(1 << PktSubType);	 
	
			if (WlanIdx < 64)
			{
				TxSCtl->TxSStatusPerWlanIdx[0] |= (1 << WlanIdx);
			}
			else if (WlanIdx >= 64 && WlanIdx < 128)
			{
				TxSCtl->TxSStatusPerWlanIdx[1] |= (1 << WlanIdx);
			}
			else
			{
				TxSCtl->TxSStatusPerWlanIdx[0] = 0xffffffff;
				TxSCtl->TxSStatusPerWlanIdx[1] = 0xffffffff;
			}

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;

			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: can not find TxSType(PktType = %d, PktSubType = %d, Format = %d)\n", 
								__FUNCTION__, PktType, PktSubType, Format));
	return -1;
}



static VOID DumpTxSFormat(RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{ 
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
	TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	TXS_D_4 *txs_d4 = &txs_entry->txs_d4;

	DBGPRINT(RT_DEBUG_OFF, ("\t\t(TXSFM=%d, TXS2M=%d, TXS2H=%d)\n", txs_d0->txsfm, txs_d0->txs2m, txs_d0->txs2h));
	
	DBGPRINT(RT_DEBUG_OFF, ("\t\tFR=%d, TxRate=0x%x\n", txs_d0->fr, txs_d0->tx_rate));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tME=%d, RE=%d, LE=%d, BE=%d, TxOPLimitErr=%d\n",\
                                   txs_d0->ME, txs_d0->RE, txs_d0->LE, txs_d0->BE, txs_d0->txop));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tPS=%d, BA Fail=%d, tid=%d, Ant_Id=%d\n",\
                                    txs_d0->ps, txs_d0->baf, txs_d0->tid, txs_d0->ant_id));
	
	if (Format == TXS_FORMAT0)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\t\tTimeStamp=0x%x, FrontTime=0x%x(unit 32us)\n",\
                                    txs_d1->timestamp, txs_d2->field_ft.front_time));

	}
	else if (Format == TXS_FORMAT1)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\t\tNoise0=0x%x, Noise1=0x%x, Noise2=0x%x\n",\
                                    txs_d1->field_noise.noise_0, txs_d1->field_noise.noise_1,
									txs_d1->field_noise.noise_2));
		
		DBGPRINT(RT_DEBUG_OFF, ("\t\tRCPI0=0x%x, RCPI1=0x%x, RCPI2=0x%x\n",\
                                    txs_d2->field_rcpi.rcpi_0, txs_d2->field_rcpi.rcpi_1,
									txs_d2->field_rcpi.rcpi_2));
	}
	
	DBGPRINT(RT_DEBUG_OFF, ("\t\tTxPwr(dbm)=0x%x\n", txs_d2->field_ft.tx_pwr_dBm));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tTxDelay=0x%x(unit 32us), RxVSeqNum=0x%x, Wlan Idx=0x%x\n",\
                             txs_d3->transmission_delay, txs_d3->rxv_sn, txs_d3->wlan_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tSN=0x%x, TxBW=0x%x, AMPDU=%d, Final MPDU=%d PID=0x%x, MPDU TxCnt=%d, MCS Idx=%d\n", txs_d4->sn, txs_d4->tbw, txs_d4->am, txs_d4->fm, txs_d4->pid, txs_d4->mpdu_tx_cnt, txs_d4->last_tx_rate_idx));
}
 

INT32 ParseTxSPacket(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;
	UINT8 Type, PktPid, PktType, PktSubType;
	UINT16 TxRate;
	UINT32 Priv;

	if (TxSCtl->TxSValid == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Ignore this TxValid\n", __FUNCTION__));
		DumpTxSFormat(pAd, Format, Data);
		return -1;
	}

	RemoveTxSStatus(pAd, Pid, &Type, &PktPid, &PktType, &PktSubType, &TxRate, &Priv);
	
	if (Type == TXS_TYPE0)
	{ 
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

                TxSType = DlListFirst(&TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List);
                if (TxSType == NULL)
                {
                        RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
                        return -1;
                }
			
		DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) 
		{
			if (TxSType->PktPid == PktPid && TxSType->Format == Format)
			{
				if (TxSType->DumpTxSReport)
				{
					if (TxSType->DumpTxSReportTimes > 0 || TxSType->DumpTxSReportTimes == 
							TXS_DUMP_REPEAT)
					{
						DBGPRINT(RT_DEBUG_OFF, ("\tPktPid = 0x%x, Orignal TxRate = 0x%x, Priv = 0x%x\n",
												PktPid, TxRate, Priv));
						DumpTxSFormat(pAd, Format, Data);

						if (TxSType->DumpTxSReportTimes != TXS_DUMP_REPEAT)
							TxSType->DumpTxSReportTimes--;
					}
				}

				TxSType->TxSHandler(pAd, Data, Priv);
				RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], 
														&Flags);
				return 0;
			}
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	}
	else if (Type == TXS_TYPE1)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
		
                TxSType = DlListFirst (&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List);
                if (TxSType == NULL)
                {
                        printk("QQ 2\n");
                        RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
                        return -1;
                }

		
		DlListForEach(TxSType,
			&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) 
		{
			if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
					&& (TxSType->Format == Format))
			{
				if (TxSType->DumpTxSReport)
				{
					if (TxSType->DumpTxSReportTimes > 0 || TxSType->DumpTxSReportTimes 
											== TXS_DUMP_REPEAT)
					{
						DBGPRINT(RT_DEBUG_OFF, ("\tPktType = 0x%x, PktSubType = 0x%x, Orignal TxRate = 0x%x, Priv = 0x%x\n", PktType, PktSubType, TxRate, Priv));
						DumpTxSFormat(pAd, Format, Data);
						
						if (TxSType->DumpTxSReportTimes != TXS_DUMP_REPEAT)
							TxSType->DumpTxSReportTimes--;
					}
				}

				RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
				TxSType->TxSHandler(pAd, Data, Priv);
				return 0;
			}
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	}

	return -1;
}


UINT8 AddTxSStatus(RTMP_ADAPTER *pAd, UINT8 Type, UINT8 PktPid, UINT8 PktType,
						UINT8 PktSubType, UINT16 TxRate, UINT32 Priv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

	if (TxSCtl->TxSStatus[TxSCtl->TxSPid].State == TXS_USED)
	{
		TxSCtl->TxSFailCount++;
	}

	TxSCtl->TxSStatus[TxSCtl->TxSPid].TxSPid = TxSCtl->TxSPid;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].State = TXS_USED;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].Type = Type;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].PktPid = PktPid;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].PktType = PktType;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].PktSubType = PktSubType;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].TxRate = TxRate;
	TxSCtl->TxSStatus[TxSCtl->TxSPid].Priv = Priv;
	TxSCtl->TxSPid++;
	
	return (TxSCtl->TxSPid - 1);
}


INT32 RemoveTxSStatus(RTMP_ADAPTER *pAd, UINT8 TxSPid, UINT8 *Type, UINT8 *PktPid, 
								UINT8 *PktType, UINT8 *PktSubType, UINT16 *TxRate, UINT32 *TxSPriv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	
	TxSCtl->TxSStatus[TxSPid].State = TXS_UNUSED;
	
	*Type = TxSCtl->TxSStatus[TxSPid].Type;
	*PktPid = TxSCtl->TxSStatus[TxSPid].PktPid;
	*PktType = TxSCtl->TxSStatus[TxSPid].PktType;
	*PktSubType = TxSCtl->TxSStatus[TxSPid].PktSubType;
	*TxRate = TxSCtl->TxSStatus[TxSCtl->TxSPid].TxRate;
	*TxSPriv = TxSCtl->TxSStatus[TxSPid].Priv;

	return 0;
}

