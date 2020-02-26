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
	cmm_rvr_dbg.c
*/
#include "rt_config.h"

#define MSG_LEN 2048
static const RTMP_STRING *hex_RTMP_STRING = "0123456789abcdefABCDEF";
static const RTMP_STRING *dec_RTMP_STRING = "0123456789";
static struct {
	RTMP_STRING *name;
	INT(*rd_proc) (RTMP_ADAPTER * pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT * wrq);
} *PRTMP_PRIVATE_RD_PROC, RTMP_PRIVATE_RD_SUPPORT_PROC[] = {
	{"view",	rd_view},
	{"view+",	rd_view_plus},
	{"view-",	rd_view_minus},
	/*Alias */
	{"wcid",	rd_wcid},
	{"sta",		rd_wcid},
	{"apcli",	rd_wcid},
	{"ap",		rd_wcid},
	{"reset", rd_reset},
	{"help", rd_help},
	{NULL,}
};
static struct {
	RTMP_STRING *key;
	RTMP_STRING *str;
	INT val;
} *PView_Key_Node, View_Key_Node_List[] = {
	{ "basic", "VIEW_BASICINFO", VIEW_BASICINFO},
	{ "wcid", "VIEW_WCID", VIEW_WCID},
	{ "mac", "VIEW_MACCOUNTER", VIEW_MACCOUNTER},
	{ "phy", "VIEW_PHYCOUNTER", VIEW_PHYCOUNTER},
	{ "noise", "VIEW_NOISE", VIEW_NOISE},
	{ "cn", "VIEW_CNNUMBER", VIEW_CNNUMBER},
	{ "others", "VIEW_OTHERS", VIEW_OTHERS},
	{NULL, NULL,}
};

VOID RTMPIoctlRvRDebug_Init(RTMP_ADAPTER *pAd)
{
	pAd->RVRDBGCtrl.ucViewLevel = VIEW_BASICINFO | VIEW_MACCOUNTER | VIEW_PHYCOUNTER  | VIEW_WCID; /*Default View Info */
	pAd->RVRDBGCtrl.ucWcid = 0;
	pAd->RVRDBGCtrl.ucCNcnt = 0xFF;
}

INT RTMPIoctlRvRDebug(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	this_char = wrq->u.data.pointer;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Before check, this_char = %s\n", __func__, this_char));
	value = strchr(this_char, '=');
	if (value) {
		if (strlen(value) > 1) {
			*value++ = 0;
		} else {
			*value = 0;
			value = NULL;
		}
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): After check, this_char = %s, value = %s\n"
			 , __func__, this_char, (value == NULL ? "" : value)));
	for (PRTMP_PRIVATE_RD_PROC = RTMP_PRIVATE_RD_SUPPORT_PROC; PRTMP_PRIVATE_RD_PROC->name; PRTMP_PRIVATE_RD_PROC++) {
		if (!strcmp(this_char, PRTMP_PRIVATE_RD_PROC->name)) {
			if (!PRTMP_PRIVATE_RD_PROC->rd_proc(pAd, value, wrq)) {
				/*FALSE:Set private failed then return Invalid argument */
				Status = -EINVAL;
			}

			break;  /*Exit for loop. */
		}
	}
	if (PRTMP_PRIVATE_RD_PROC->name == NULL) {
		/*Not found argument */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PRTMP_PRIVATE_RD_PROC->name == NULL"));
		rd_dashboard(pAd, wrq);
		return Status;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return Status;
}

INT rd_dashboard(RTMP_ADAPTER *pAd, IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	if (msg == NULL)
		return 0;
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");
	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " RvR Debug Info ", "====================");
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_BASICINFO) == VIEW_BASICINFO)
		printBasicinfo(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_WCID) == VIEW_WCID)
		printWcid(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_MACCOUNTER) == VIEW_MACCOUNTER) {
		updateBFTxCnt(pAd);
		printMacCounter(pAd, msg);
	}
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_PHYCOUNTER) == VIEW_PHYCOUNTER)
		printPhyCounter(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_NOISE) == VIEW_NOISE)
		printNoise(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_OTHERS) == VIEW_OTHERS)
		printOthers(pAd, msg);
	sprintf(msg+strlen(msg), "%s\n", "========================================================");
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_CNNUMBER) == VIEW_CNNUMBER)
		updateCNNum(pAd, TRUE);
	else
		updateCNNum(pAd, FALSE);
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

INT rd_view(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg;
	INT button = Case_SHOW;
	UINT8 ucViewLevel_val = 0;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");

	if (arg != NULL) {
		if (strlen(arg) > 2) {
			button = Case_ERROR;
		} else {
			if (strspn(arg, hex_RTMP_STRING) == strlen(arg)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Vaild strspn=%d,strlen=%d,", (INT)strspn(arg, hex_RTMP_STRING), (INT)strlen(arg)));
				button = Case_SET;
				ucViewLevel_val = (UINT8) os_str_tol(arg, 0, 16);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Vaild strspn=%d,strlen=%d,", (INT)strspn(arg, hex_RTMP_STRING), (INT)strlen(arg)));
				button = Case_ERROR;
			}
		}
	}

	switch (button) {
	case Case_ERROR:
		sprintf(msg + strlen(msg), "No corresponding parameter !!!\n");
		sprintf(msg + strlen(msg), "ex: iwpriv ra0 rd view=FF(bit8)\n");
		break;
	case Case_SHOW:
		printView(pAd, msg);
		break;
	case Case_SET:
		pRVRDBGCtrl->ucViewLevel  = ucViewLevel_val;
		sprintf(msg + strlen(msg), "pRVRDBGCtrl->ucViewLevel = %x", pRVRDBGCtrl->ucViewLevel);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}
INT rd_view_plus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg;
	INT view_val = VIEW_ERROR;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");

	if (arg)
		view_val = getViewLevelValue(arg);
	switch (view_val) {
	case VIEW_BASICINFO:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_BASICINFO;
		sprintf(msg + strlen(msg), "VIEW_BASICINFO Enable");
		break;
	case VIEW_WCID:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_WCID;
		sprintf(msg + strlen(msg), "VIEW_WCID Enable");
		break;
	case VIEW_MACCOUNTER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_MACCOUNTER;
		sprintf(msg + strlen(msg), "VIEW_MACCOUNTER Enable");
		break;
	case VIEW_PHYCOUNTER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_PHYCOUNTER;
		sprintf(msg + strlen(msg), "VIEW_PHYCOUNTER Enable");
		break;
	case VIEW_CNNUMBER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_CNNUMBER;
		sprintf(msg + strlen(msg), "VIEW_CNNUMBER Enable");
		break;
	case VIEW_NOISE:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_NOISE;
		sprintf(msg + strlen(msg), "VIEW_NOISE Enable");
		break;
	case VIEW_6:
		break;
	case VIEW_OTHERS:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_OTHERS;
		sprintf(msg + strlen(msg), "VIEW_OTHERS Enable");
		break;
	case VIEW_ERROR:
		sprintf(msg + strlen(msg), "No corresponding parameter !!!\n");
		sprintf(msg + strlen(msg), "ex: iwpriv ra0 rd view+=rate\n");
		printView(pAd, msg);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}
INT rd_view_minus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg;
	INT view_val = VIEW_ERROR;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");

	if (arg)
		view_val = getViewLevelValue(arg);
	switch (view_val) {
	case VIEW_BASICINFO:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_BASICINFO;
		sprintf(msg + strlen(msg), "VIEW_BASICINFO Disable");
		break;
	case VIEW_WCID:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_WCID;
		sprintf(msg + strlen(msg), "VIEW_WCID Disable");
		break;
	case VIEW_MACCOUNTER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_MACCOUNTER;
		sprintf(msg + strlen(msg), "VIEW_MACCOUNTER Disable");
		break;
	case VIEW_PHYCOUNTER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_PHYCOUNTER;
		sprintf(msg + strlen(msg), "VIEW_PHYCOUNTER Disable");
		break;
	case VIEW_CNNUMBER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_CNNUMBER;
		sprintf(msg + strlen(msg), "VIEW_CNNUMBER Disable");
		break;
	case VIEW_NOISE:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_NOISE;
		sprintf(msg + strlen(msg), "VIEW_NOISE Disable");
		break;
	case VIEW_6:

		break;
	case VIEW_OTHERS:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_OTHERS;
		sprintf(msg + strlen(msg), "VIEW_OTHERS Disable");
		break;
	case VIEW_ERROR:
		sprintf(msg + strlen(msg), "No corresponding parameter !!!\n");
		sprintf(msg + strlen(msg), "ex: iwpriv ra0 rd view-=rate\n");
		printView(pAd, msg);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

INT rd_wcid(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg;
	INT button = Case_SHOW;
	LONG input;
	UINT8 ucWcid = 0;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");
	if (arg != NULL) {
		if (strlen(arg) > 3) {
			button = Case_ERROR;
		} else{
			if (strspn(arg, dec_RTMP_STRING) == strlen(arg)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Vaild strspn=%d,strlen=%d,", (INT)strspn(arg, dec_RTMP_STRING), (INT)strlen(arg)));
				input = os_str_tol(arg, 0, 10);
				if (input < 0 || input > MAX_LEN_OF_MAC_TABLE) {
					button = Case_ERROR;
				} else {
					ucWcid = input;
					button = Case_SET;
				}
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Invaild strspn=%d,strlen=%d,", (INT)strspn(arg, dec_RTMP_STRING), (INT)strlen(arg)));
				button = Case_ERROR;
			}
		}
	}
	switch (button) {
	case Case_ERROR:
		sprintf(msg + strlen(msg), "No corresponding parameter !!!\n");
		sprintf(msg + strlen(msg), "ex: iwpriv ra0 rd sta=1~%d\n", MAX_LEN_OF_MAC_TABLE);
		sprintf(msg + strlen(msg), "or  iwpriv ra0 rd sta=0 for auto search first sta\n");
		break;
	case Case_SHOW:
		Show_MacTable_Proc(pAd, ENTRY_NONE);
		break;
	case Case_SET:
		pRVRDBGCtrl->ucWcid  = ucWcid;
		sprintf(msg + strlen(msg), "pRVRDBGCtrl->ucWcid = %d", pRVRDBGCtrl->ucWcid);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

INT rd_reset(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg;
	INT button = Case_SHOW;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");

	switch (button) {
	case Case_SHOW:
		sprintf(msg + strlen(msg), "Reset all counter!\n");
		Set_ResetStatCounter_Proc(pAd, NULL);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

INT rd_help(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg;
	INT button = Case_SHOW;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
	memset(msg, 0x00, MSG_LEN);
	sprintf(msg, "\n");
	switch (button) {
	case Case_SHOW:
		sprintf(msg + strlen(msg), "%s", "iwpriv [Interface] rd [Sub-command]\n");
		sprintf(msg + strlen(msg), "%s", "Sub-command List\n");
		sprintf(msg + strlen(msg), "%-25s %s", "view", "Show view level status\n");
		sprintf(msg + strlen(msg), "%-25s %s", "view=", "Set view level by hex value(8bits 00~FF)\n");
		sprintf(msg + strlen(msg), "%-25s %s", "view+=", "Enable view level by string\n");
		sprintf(msg + strlen(msg), "%-25s %s", "view-=", "Disable view level by string\n");
		sprintf(msg + strlen(msg), "%-25s %s", "wcid,sta,ap,apcli", "Show mac table\n");
		sprintf(msg + strlen(msg), "%-25s %s", "wcid=,sta=,ap=,apcli=", "Set WCID\n");
		sprintf(msg + strlen(msg), "%-25s %s", "reset", "Reset all counter\n");
		sprintf(msg + strlen(msg), "%-25s %s", "help", "Show support command info\n");
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return TRUE;
}

INT printBasicinfo (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	UINT32 temperature = 0;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;

	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " BASIC ", "====================");
	sprintf(msg+strlen(msg), "%-32s= %d\n", "Current Band ", ucBand);
	RTMP_GET_TEMPERATURE(pAd, &temperature);
	sprintf(msg+strlen(msg), "%-32s= %d\n", "Current Temperature ", temperature);
	return TRUE;
}

VOID printView(RTMP_ADAPTER *pAd, IN RTMP_STRING *msg)
{
	PRvR_Debug_CTRL pRVRDBGCtrl;
	INT view_bits = 0;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	sprintf(msg + strlen(msg), "%-4s | %-6s | %-15s | %s\n", "bit", "arg", "info", "Status");
	for (PView_Key_Node = View_Key_Node_List; PView_Key_Node->key; PView_Key_Node++) {
		sprintf(msg + strlen(msg), "%-4d | %-6s | %-15s | %s\n",
		view_bits++,
		PView_Key_Node->key,
		PView_Key_Node->str,
		(pRVRDBGCtrl->ucViewLevel & PView_Key_Node->val ? "Enable":"Disable"));
	}
}

INT printWcid (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	INT 	first_sta = 0;
	UINT32 lastRxRate;
	UINT32 lastTxRate;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " WCID ", "====================");

	/* User assign aid, default = 0 will auto search first sta  */
	if (pRVRDBGCtrl->ucWcid == 0)
		for (first_sta = 1; VALID_UCAST_ENTRY_WCID(pAd, first_sta); first_sta++) {
			pEntry = &pAd->MacTab.Content[first_sta];
			if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
				break;
		}
	else if (VALID_UCAST_ENTRY_WCID(pAd, pRVRDBGCtrl->ucWcid))
		pEntry = &pAd->MacTab.Content[pRVRDBGCtrl->ucWcid];

	if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
		sprintf(msg+strlen(msg), "%-32s= %d\n", "AID ", (int)pEntry->Aid);
		sprintf(msg+strlen(msg), "%-32s= %02X:%02X:%02X:%02X:%02X:%02X\n", "MAC Addr ", PRINT_MAC(pEntry->Addr));
		snprintf(tmp_str, temp_str_len, "%d %d %d %d", pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
			 pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
		sprintf(msg+strlen(msg), "%-32s= %s\n", "RSSI0/1/2/3 ", tmp_str);
		lastRxRate = pEntry->LastRxRate;
		lastTxRate = pEntry->LastTxRate;
		if (cap->fgRateAdaptFWOffload == TRUE) {
			if (pEntry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				HTTRANSMIT_SETTING LastTxRate;

				MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1:0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1:0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
				if (LastTxRate.field.MODE == MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
				lastTxRate = (UINT32)(LastTxRate.word);
						}
		}
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			StatRateToString(pAd, msg, 0, lastTxRate);
			StatRateToString(pAd, msg, 1, lastRxRate);
		}
	}
	return TRUE;
}

INT printMacCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	UINT32 txCount = 0;
	UINT32 rxCount = 0;
	ULONG txper, rxper;
	COUNTER_802_11 *WlanCounter = &pAd->WlanCounters[ucBand];
	ULONG mpduper = 0;
	ULONG mpduTXCount = 0;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;
	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " MAC COUNTER ", "====================");

	/* Tx Count */
	txCount = pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart;
	sprintf(msg+strlen(msg), "%-32s= %d\n", "Tx success count ", txCount);
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		txper = txCount == 0 ? 0 : 1000*(pAd->WlanCounters[ucBand].FailedCount.u.LowPart)/(pAd->WlanCounters[ucBand].FailedCount.u.LowPart+txCount);
		sprintf(msg+strlen(msg), "%-32s= %ld PER=%ld.%1ld%%\n",
			"Tx fail count ",
			(ULONG)pAd->WlanCounters[ucBand].FailedCount.u.LowPart,
			txper/10, txper % 10);
	} else {
		txper = txCount == 0 ? 0 : 1000*(pAd->WlanCounters[ucBand].RetryCount.u.LowPart+pAd->WlanCounters[ucBand].FailedCount.u.LowPart)/(pAd->WlanCounters[ucBand].RetryCount.u.LowPart+pAd->WlanCounters[ucBand].FailedCount.u.LowPart+txCount);
		sprintf(msg+strlen(msg), "%-32s= %ld, PER=%ld.%1ld%%\n",
			"Tx retry count ",
			(ULONG)pAd->WlanCounters[ucBand].RetryCount.u.LowPart,
			txper/10, txper % 10);
	}
	/*BF */
	sprintf(msg+strlen(msg), "%-32s= %x %x\n",
		"Tx BF count(iBF/eBF) ", pRVRDBGCtrl->uiiBFTxcnt, pRVRDBGCtrl->uieBFTxcnt);
	/*AMPDU */
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Tx AGG Range 1 (1)", (LONG)(WlanCounter->TxAggRange1Count.u.LowPart));
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Tx AGG Range 2 (2~5)", (LONG)(WlanCounter->TxAggRange2Count.u.LowPart));
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Tx AGG Range 3 (6~15)", (LONG)(WlanCounter->TxAggRange3Count.u.LowPart));
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Tx AGG Range 4 (>15)", (LONG)(WlanCounter->TxAggRange4Count.u.LowPart));
	mpduTXCount = WlanCounter->AmpduSuccessCount.u.LowPart;
	sprintf(msg+strlen(msg), "%-32s= %ld\n", "Tx AMPDU success", mpduTXCount);
	mpduper = mpduTXCount == 0 ? 0 : 1000 * (WlanCounter->AmpduFailCount.u.LowPart) / (WlanCounter->AmpduFailCount.u.LowPart + mpduTXCount);
	sprintf(msg+strlen(msg), "%-32s= %ld PER=%ld.%1ld%%\n",
		"Tx AMPDU fail count", (ULONG)WlanCounter->AmpduFailCount.u.LowPart, mpduper/10, mpduper % 10);
	/* Rx Count */
	rxCount = pAd->WlanCounters[ucBand].ReceivedFragmentCount.QuadPart;
	sprintf(msg+strlen(msg), "%-32s= %d\n", "Rx success ", rxCount);
	rxper = pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart == 0 ? 0 : 1000*(pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart)/(pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart+pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart);
	sprintf(msg+strlen(msg), "%-32s= %ld, PER=%ld.%1ld%%\n",
		"Rx with CRC ",
		(ULONG)pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart,
		rxper/10, rxper % 10);
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Rx drop(out of resource)", (ULONG)pAd->Counters8023.RxNoBuffer);
	sprintf(msg+strlen(msg), "%-32s= %ld\n",
		"Rx duplicate frame", (ULONG)pAd->WlanCounters[ucBand].FrameDuplicateCount.u.LowPart);

	return  TRUE;
}


INT printPhyCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;
	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " PHY COUNTER ", "====================");
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_CNNUMBER) == VIEW_CNNUMBER)
		printCNNum(pAd, msg);
	return  TRUE;
}



INT printNoise (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;
	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " NOISE ", "====================");
	sprintf(msg+strlen(msg), "%-32s= %s\n",
		"MibBucket ", pAd->OneSecMibBucket.Enabled[ucBand] ? "Enable":"Disable");
	sprintf(msg+strlen(msg), "%-32s= %d\n",
		"Channel Busy Time ", pAd->OneSecMibBucket.ChannelBusyTime[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %d\n",
		"OBSS Air Time ", pAd->OneSecMibBucket.OBSSAirtime[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %d\n",
		"Tx Air Time ", pAd->OneSecMibBucket.MyTxAirtime[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %d\n",
		"Rx Air Time ", pAd->OneSecMibBucket.MyRxAirtime[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %d\n",
		"EDCCA Time ", pAd->OneSecMibBucket.EDCCAtime[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %x\n",
		"PD count ", pAd->OneSecMibBucket.PdCount[ucBand]);
	sprintf(msg+strlen(msg), "%-32s= %x\n",
		"MDRDY Count ", pAd->OneSecMibBucket.MdrdyCount[ucBand]);
	return TRUE;
}

INT printOthers (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	/*
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;
	*/
	sprintf(msg+strlen(msg), "%s%-16s%s\n", "====================", " OTHERS ", "====================");
	return TRUE;
}

INT printCNNum (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {

	PRvR_Debug_CTRL pRVRDBGCtrl;
	UINT8 idx;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (pRVRDBGCtrl->ucCNcnt != 10) {
		setCNNum(pAd, FALSE);
		setRXV2(pAd, FALSE);
	}
	sprintf(msg+strlen(msg), "%-32s= ",	"Condition Number ");
	for (idx = 0; idx < 10; idx++)
		sprintf(msg+strlen(msg), "%-2d ", pAd->rxv2_cyc3[idx]);
	sprintf(msg+strlen(msg), "\n");
	}
	return TRUE;
}

INT updateBFTxCnt(RTMP_ADAPTER *pAd)
{
#ifdef MT7615
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		UINT32 CrValue;
		PRvR_Debug_CTRL pRVRDBGCtrl;
		pRVRDBGCtrl = &pAd->RVRDBGCtrl;
		RTMP_IO_READ32(pAd, WF_ETBF_BASE + 0xa09c, &CrValue);
		pRVRDBGCtrl->uieBFTxcnt = CrValue & BF_MASK;
		pRVRDBGCtrl->uiiBFTxcnt = ((CrValue & ~BF_MASK) >> BF_OFSET);
	}
#endif /* MT7615 */
	return TRUE;
}

INT updateCNNum(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable)
{
#ifdef MT7615
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
	setCNNum(pAd, Is_Enable);
	setRXV2(pAd, Is_Enable);
	}
#endif /* MT7615 */
	return TRUE;
}

INT setRXV2(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable)
{
#ifdef MT7615
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		UINT32 value;
		if (Is_Enable) {
			RTMP_IO_READ32(pAd, ARB_RQCR, &value);
			value |= ARB_RQCR_RXV_R_EN;
			if (pAd->CommonCfg.dbdc_mode) {
				value |= ARB_RQCR_RXV1_R_EN;
			}
			RTMP_IO_WRITE32(pAd, ARB_RQCR, value);
		} else {
			RTMP_IO_READ32(pAd, ARB_RQCR, &value);
			value &= ~ARB_RQCR_RXV_R_EN;
			if (pAd->CommonCfg.dbdc_mode)
				value &= ~ARB_RQCR_RXV1_R_EN;
			RTMP_IO_WRITE32(pAd, ARB_RQCR, value);
		}
	}
#endif /* MT7615 */
	return TRUE;
}

INT setCNNum(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable)
{
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		UINT8 idx;
		UINT32 value;
		PRvR_Debug_CTRL pRVRDBGCtrl;

		pRVRDBGCtrl = &pAd->RVRDBGCtrl;
		if (Is_Enable) {
			pRVRDBGCtrl->ucCNcnt = 0;
			for (idx = 0; idx < 10; idx++)
				pAd->rxv2_cyc3[idx] = 0xFFFFFFFF;
			RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x66c, &value);
			value |= 0xD << 4;
			RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x66c, value);
		} else {
			RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x66c, &value);
			value &= ~0xF0;
			RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x66c, value);
		}
	}
	return TRUE;
}

INT getViewLevelValue(RTMP_STRING *arg)
{
	if (arg == NULL)
		return VIEW_ERROR;
	for (PView_Key_Node = View_Key_Node_List; PView_Key_Node->key; PView_Key_Node++)
		if (!strcasecmp(arg, PView_Key_Node->key))
			return PView_Key_Node->val;
	return VIEW_ERROR;
}
