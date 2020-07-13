/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

    Abstract:

	All related POWER SAVE function body.

***************************************************************************/

#include "rt_config.h"




/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID MtHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN       IsDequeu = FALSE;
	INT           DequeuAC = QID_AC_BK;
	INT           DequeuCOUNT;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

	ASSERT(wcid < GET_MAX_UCAST_NUM(pAd));
	pMacEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (isActive == FALSE) { /* ps poll */
		/*
			Need to check !! @20140212
			New architecture has per AC sw-Q for per entry.
			We should check packets by ACs priority --> 1. VO, 2. VI, 3. BE, 4. BK
		*/
		DequeuAC = QID_AC_BK;
		IsDequeu = TRUE;
		DequeuCOUNT = 1;
		tr_entry->PsQIdleCount = 0;
	} else { /* Receive Power bit 0 frame */
		WLAN_MR_TIM_BIT_CLEAR(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RtmpHandleRxPsPoll null0/1 wcid = %x mt_ps_queue.Number = %d\n",
				 tr_entry->wcid,
				 tr_entry->ps_queue.Number));
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d) tx_queue.Number = BE:%d, BK:%d, VI:%d, VO:%d, ps_state:%x,  tx_queue.TokenCount = BE:%d, BK:%d, VI:%d, VO:%d\n",
				 __func__, __LINE__,
				 tr_entry->tx_queue[QID_AC_BE].Number,
				 tr_entry->tx_queue[QID_AC_BK].Number,
				 tr_entry->tx_queue[QID_AC_VI].Number,
				 tr_entry->tx_queue[QID_AC_VO].Number,
				 tr_entry->ps_state,
				 tr_entry->TokenCount[QID_AC_BE],
				 tr_entry->TokenCount[QID_AC_BK],
				 tr_entry->TokenCount[QID_AC_VI],
				 tr_entry->TokenCount[QID_AC_VO]));
#ifdef UAPSD_SUPPORT

		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)) {
			/* deliver all queued UAPSD packets */
			UAPSD_AllPacketDeliver(pAd, pMacEntry);
			/* end the SP if exists */
			UAPSD_MR_ENTRY_RESET(pAd, pMacEntry);
		}

#endif /* UAPSD_SUPPORT */

		if (tr_entry->enqCount > 0) {
			IsDequeu = TRUE;
			DequeuAC = num_of_tx_ring;

			if (tr_entry->enqCount > 8 /* MAX_TX_PROCESS */)
				DequeuCOUNT =  8 /* MAX_TX_PROCESS */;
			else
				DequeuCOUNT = tr_entry->enqCount;
		}
	}

	if (IsDequeu == TRUE) {
		RTMPDeQueuePacket(pAd, FALSE, DequeuAC, tr_entry->wcid, DequeuCOUNT);
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RtmpHandleRxPsPoll IsDequeu == TRUE tr_entry->wcid=%x DequeuCOUNT=%d, ps_state=%d\n", tr_entry->wcid, DequeuCOUNT, tr_entry->ps_state));
	}

	return;
#endif /* CONFIG_AP_SUPPORT */
}


/*
	==========================================================================
	Description:
		Update the station current power save mode. Calling this routine also
		prove the specified client is still alive. Otherwise AP will age-out
		this client once IdleCount exceeds a threshold.
	==========================================================================
 */
BOOLEAN MtPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm)
{
	MAC_TABLE_ENTRY *pEntry;
	UCHAR old_psmode;
	STA_TR_ENTRY *tr_entry;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return PWR_ACTIVE;

	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	/*
		Change power save mode first because we will call
		RTMPDeQueuePacket() in RtmpHandleRxPsPoll().

		Or when Psm = PWR_ACTIVE, we will not do Aggregation in
		RTMPDeQueuePacket().
	*/
	old_psmode = pEntry->PsMode;
	pEntry->NoDataIdleCount = 0;
	pEntry->PsMode = Psm;
	pAd->MacTab.tr_entry[wcid].PsMode = Psm;

	if ((old_psmode == PWR_SAVE) && (Psm == PWR_ACTIVE)) {
		/*
			STA wakes up.
		*/
		if (tr_entry->ps_state == APPS_RETRIEVE_DONE) {
			tr_entry->ps_state = APPS_RETRIEVE_IDLE;
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): STA wakes up!\n", __func__, __LINE__));
			MtHandleRxPsPoll(pAd, pAddr, wcid, TRUE);
		} else
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d):wcid=%d, old_psmode=%d, now_psmode=%d, wrong ps_state=%d ???\n",
					 __func__, __LINE__, wcid, old_psmode, Psm, tr_entry->ps_state));
	} else if ((old_psmode == PWR_ACTIVE) && (Psm == PWR_SAVE)) {
		/*
			STA goes to sleep.
		*/
		if (tr_entry->ps_state == APPS_RETRIEVE_IDLE) {
			tr_entry->ps_state = APPS_RETRIEVE_DONE;
		} else
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d):wcid=%d, old_psmode=%d, now_psmode=%d, wrong ps_state=%d ???\n",
					 __func__, __LINE__, wcid, old_psmode, Psm, tr_entry->ps_state));
	} else {
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): ps state is not changed, do nothing here.\n",
				 __func__, __LINE__));
	}

	return old_psmode;
}


