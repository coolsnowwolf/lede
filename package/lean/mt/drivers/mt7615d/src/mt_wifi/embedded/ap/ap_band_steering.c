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
	ap_band_steering.c
*/

#ifdef BAND_STEERING
#include "rt_config.h"

#define BNDSTRG_DRIVER_VER	"3.1.3"
static inline PBND_STRG_CLI_TABLE Get_BndStrgTableByChannel(
	PRTMP_ADAPTER	pAd,
	UINT8           Channel)
{
	PBND_STRG_CLI_TABLE table = NULL;
	INT i;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		table = P_BND_STRG_TABLE(i);

		if (table->bInitialized && (table->Channel == Channel))
			return table;
	}

	return NULL;
}

inline PBND_STRG_CLI_TABLE Get_BndStrgTable(
	PRTMP_ADAPTER	pAd,
	INT             apidx)
{
	BSS_STRUCT *pMbss = NULL;
	UINT8 Channel;

	if (apidx < HW_BEACON_MAX_NUM)
		pMbss = &pAd->ApCfg.MBSSID[apidx];

	if (pMbss) {
		Channel = pMbss->wdev.channel;
		return Get_BndStrgTableByChannel(pAd, Channel);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (RED("%s:(%d):invalid pMbss. apidx=%d\n"), __func__, __LINE__, apidx));
	return NULL;
}

/* WPS_BandSteering Support */
PWPS_WHITELIST_ENTRY FindWpsWhiteListEntry(
	PLIST_HEADER pWpsWhiteList,
	PUCHAR pMacAddr)
{
	PWPS_WHITELIST_ENTRY	pWpsWhiteListEntry = NULL;
	RT_LIST_ENTRY		        *pListEntry = NULL;

	pListEntry = pWpsWhiteList->pHead;
	pWpsWhiteListEntry = (PWPS_WHITELIST_ENTRY)pListEntry;
	while (pWpsWhiteListEntry != NULL) {

		if (NdisEqualMemory(pWpsWhiteListEntry->addr, pMacAddr, MAC_ADDR_LEN))
			return pWpsWhiteListEntry;
		pListEntry = pListEntry->pNext;
		pWpsWhiteListEntry = (PWPS_WHITELIST_ENTRY)pListEntry;
	}

	return NULL;
}

VOID AddWpsWhiteList(
	PLIST_HEADER pWpsWhiteList,
	PUCHAR pMacAddr)
{
	PWPS_WHITELIST_ENTRY pWpsWhiteListEntry = NULL;

	pWpsWhiteListEntry = FindWpsWhiteListEntry(pWpsWhiteList, pMacAddr);

	if (pWpsWhiteListEntry) {
		/* the Entry already exist */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Found %02x:%02x:%02x:%02x:%02x:%02x in Wps White List\n",
			__func__, PRINT_MAC(pMacAddr)));
	} else {
		/* Add new Entry */
		os_alloc_mem(NULL, (UCHAR **)&pWpsWhiteListEntry, sizeof(WPS_WHITELIST_ENTRY));
		if (pWpsWhiteListEntry) {
			NdisZeroMemory(pWpsWhiteListEntry, sizeof(WPS_WHITELIST_ENTRY));
			pWpsWhiteListEntry->pNext = NULL;
			NdisCopyMemory(pWpsWhiteListEntry->addr, pMacAddr, MAC_ADDR_LEN);
			insertTailList(pWpsWhiteList, (RT_LIST_ENTRY *)pWpsWhiteListEntry);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]%02x:%02x:%02x:%02x:%02x:%02x added to WpsList:size::%d\n",
			 __func__, PRINT_MAC(pWpsWhiteListEntry->addr), pWpsWhiteList->size));
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Mem alloc fail\n", __func__));
	}
}

VOID DelWpsWhiteListExceptMac(
	PLIST_HEADER pWpsWhiteList,
	PUCHAR pMacAddr)
{

	RT_LIST_ENTRY	*pListEntry = NULL, *pDelEntry = NULL;
	PWPS_WHITELIST_ENTRY	pWpsWhiteListEntry = NULL;
	PLIST_HEADER    pListHeader = pWpsWhiteList;
	UCHAR Addr[6];

	NdisCopyMemory(Addr, pMacAddr, MAC_ADDR_LEN);

	if (pListHeader->size == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: WpsWhiteList Empty.\n", __func__));
		return;
	}

	pListEntry = pListHeader->pHead;
	pWpsWhiteListEntry = (PWPS_WHITELIST_ENTRY)pListEntry;

	while (pWpsWhiteListEntry != NULL) {

		if (MAC_ADDR_EQUAL(pWpsWhiteListEntry->addr, Addr)) {
			pListEntry = pListEntry->pNext;
		} else {
			pDelEntry = delEntryList(pWpsWhiteList, pListEntry);
			pListEntry = pDelEntry->pNext;
			os_free_mem(pDelEntry);
		}
		pWpsWhiteListEntry = (PWPS_WHITELIST_ENTRY)pListEntry;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: WpsWhiteList size : %d\n",
	 __func__, pWpsWhiteList->size));
}

VOID ClearWpsWhiteList(
	PLIST_HEADER pWpsWhiteList)
{
	RT_LIST_ENTRY     *pListEntry = NULL;
	PLIST_HEADER    pListHeader = pWpsWhiteList;

	if (pListHeader->size == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: WpsWhiteList already Empty.\n", __func__));
		return;
	}

	pListEntry = pListHeader->pHead;
	while (pListEntry != NULL) {
		/*Remove ListEntry from Header*/
		removeHeadList(pListHeader);
		os_free_mem(pListEntry);
		pListEntry = pListHeader->pHead;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Clean WpsWhiteList.\n", __func__));
}

PBS_LIST_ENTRY FindBsListEntry(
	PLIST_HEADER pBsList,
	PUCHAR pMacAddr)
{
	PBS_LIST_ENTRY	pBsListEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pBsList->pHead;
	pBsListEntry = (PBS_LIST_ENTRY)pListEntry;
	while (pBsListEntry != NULL) {

		if (NdisEqualMemory(pBsListEntry->addr, pMacAddr, MAC_ADDR_LEN))
			return pBsListEntry;
		pListEntry = pListEntry->pNext;
		pBsListEntry = (PBS_LIST_ENTRY)pListEntry;
	}

	return NULL;
}

VOID AddBsListEntry(
	PLIST_HEADER pBsList,
	PUCHAR pMacAddr)
{
	PBS_LIST_ENTRY pBsListEntry = NULL;

	pBsListEntry = FindBsListEntry(pBsList, pMacAddr);

	if (pBsListEntry) {
		/* the Entry already exist */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Found %02x:%02x:%02x:%02x:%02x:%02x in BsList.\n",
			__func__, PRINT_MAC(pMacAddr)));
	} else {
		/* Add new Entry */
		os_alloc_mem(NULL, (UCHAR **)&pBsListEntry, sizeof(BS_LIST_ENTRY));
		if (pBsListEntry) {
			NdisZeroMemory(pBsListEntry, sizeof(BS_LIST_ENTRY));
			pBsListEntry->pNext = NULL;
			NdisCopyMemory(pBsListEntry->addr, pMacAddr, MAC_ADDR_LEN);
			insertTailList(pBsList, (RT_LIST_ENTRY *)pBsListEntry);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] %02x:%02x:%02x:%02x:%02x:%02x added in BsList:size::%d\n",
			 __func__, PRINT_MAC(pBsListEntry->addr), pBsList->size));
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s]Mem alloc fail\n", __func__));
	}
}

VOID DelBsListEntry(
	PLIST_HEADER pBsList,
	PUCHAR pMacAddr)
{
	PBS_LIST_ENTRY pBsListEntry = NULL;
	RT_LIST_ENTRY	*pListEntry = NULL, *pDelEntry = NULL;

	pBsListEntry = FindBsListEntry(pBsList, pMacAddr);

	if (!pBsListEntry) {
		/* the Entry already exist */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Not Found %02x:%02x:%02x:%02x:%02x:%02x in BsList.\n",
		 __func__, PRINT_MAC(pMacAddr)));
	} else {
		pListEntry = (RT_LIST_ENTRY *)pBsListEntry;
		/* Delete the Entry */
		pDelEntry = delEntryList(pBsList, pListEntry);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Sta %02x:%02x:%02x:%02x:%02x:%02x deleted from BsList:size::%d\n",
			 __func__, PRINT_MAC(pBsListEntry->addr), pBsList->size));
		os_free_mem(pDelEntry);
	}
}

VOID ClearBsList(
	PLIST_HEADER pBsList)
{
	RT_LIST_ENTRY     *pListEntry = NULL;
	PLIST_HEADER    pListHeader = pBsList;

	if (pListHeader->size == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: BsList already Empty.\n", __func__));
		return;
	}

	pListEntry = pListHeader->pHead;
	while (pListEntry != NULL) {
		/*Remove ListEntry from Header*/
		removeHeadList(pListHeader);
		os_free_mem(pListEntry);
		pListEntry = pListHeader->pHead;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Clean BsList.\n", __func__));
}

INT Show_BndStrg_Info(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	struct wifi_dev *wdev = NULL;
	BNDSTRG_MSG msg = { 0 };
	UINT8 idx = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if (ifIndex >= HW_BEACON_MAX_NUM)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	if (!pAd->ApCfg.BndStrgBssIdx[wdev->func_idx])
		return TRUE;

	table = Get_BndStrgTable(pAd, ifIndex);

	if (!table)
		return FALSE;

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band Steering Driver Ver - %s\n", BNDSTRG_DRIVER_VER));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t BndStrgBssIdx"));

	for (idx = 0; idx <	pAd->ApCfg.BssidNum; idx++)
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%d", pAd->ApCfg.BndStrgBssIdx[idx]));

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t WhiteListSize:%d\n", table->WhiteList.size));

	if (table->WhiteList.size) {
		PBS_LIST_ENTRY	pBsListEntry = NULL;
		RT_LIST_ENTRY	*pListEntry = NULL;

		pListEntry = table->WhiteList.pHead;
		pBsListEntry = (PBS_LIST_ENTRY)pListEntry;

		while (pBsListEntry != NULL) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t -> %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBsListEntry->addr)));
			pListEntry = pListEntry->pNext;
			pBsListEntry = (PBS_LIST_ENTRY)pListEntry;
		}
	}

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t BlackListSize:%d\n", table->BlackList.size));

	if (table->BlackList.size) {
		PBS_LIST_ENTRY	pBsListEntry = NULL;
		RT_LIST_ENTRY	*pListEntry = NULL;

		pListEntry = table->BlackList.pHead;
		pBsListEntry = (PBS_LIST_ENTRY)pListEntry;

		while (pBsListEntry != NULL) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t -> %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBsListEntry->addr)));
			pListEntry = pListEntry->pNext;
			pBsListEntry = (PBS_LIST_ENTRY)pListEntry;
		}
	}

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	msg.Action = TABLE_INFO;
	BndStrgSendMsg(pAd, &msg);
	return TRUE;
}

INT Show_BndStrg_List(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	UINT32	display_type;
	BNDSTRG_MSG msg = { 0 };
	CHAR	band_str[4][10] = {"", "5G", "2.4G", "2.4G/5G"};
	INT i;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	table = Get_BndStrgTable(pAd, ifIndex);

	if (!table)
		return FALSE;

	if (arg == NULL)
		display_type = 0;
	else
		display_type = (UINT32) simple_strtol(arg, 0, 10);

	if (display_type > 3)
		display_type = 0;

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("\t%s Accessible Clients:  %d\n", band_str[table->Band], table->Size));

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++) {
		if (table->Entry[i].bValid) {
			if (MAC_ADDR_EQUAL(table->MonitorAddr, table->Entry[i].Addr)) {
				BND_STRG_PRINTQAMSG(table, table->Entry[i].Addr,
									(YLW("\t%d: %02x:%02x:%02x:%02x:%02x:%02x [TblIdx:%d]\n"),
									 i, PRINT_MAC(table->Entry[i].Addr), table->Entry[i].TableIndex));
			} else {
				BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
								  ("\t%d: %02x:%02x:%02x:%02x:%02x:%02x [TblIdx:%d]\n",
								   i, PRINT_MAC(table->Entry[i].Addr), table->Entry[i].TableIndex));
			}
		}
	}

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("\tBndStrg Table Entries:\n"));
	msg.Action = ENTRY_LIST;
	msg.data.display_type.display_type = display_type;
	msg.data.display_type.filer_band = table->Band;
	msg.data.display_type.channel = table->Channel;
	BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

INT Set_BndStrg_Enable(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	BOOLEAN         enable;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	enable = (BOOLEAN) simple_strtol(arg, 0, 10);

	if (!(enable ^ pAd->ApCfg.BandSteering)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (GRN("BndStrg is already %s\n"), pAd->ApCfg.BandSteering?"Enable":"Disable"));
		return TRUE;
	}

	if (enable) {
		BSS_STRUCT *pMbss = NULL;
		INT apidx, IdBss;

		apidx = ifIndex;

		if (apidx < HW_BEACON_MAX_NUM)
			pMbss = &pAd->ApCfg.MBSSID[apidx];

		if (pMbss == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (RED("%s:(%d):invalid pMbss. apidx=%d\n"), __func__, __LINE__, apidx));
			return FALSE;
		}

		/* bnstrg table does not init yet, call BndStrg_Init directly */
		BndStrg_Init(pAd);

		/* enable all active mbss BndStrg InfFlags to nitify daemon */
		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			table = Get_BndStrgTable(pAd, IdBss);

			if (table) {
				/* Inform daemon interface ready */
				BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, TRUE);
			}
		}

		pAd->ApCfg.BandSteering = enable;
	} else {
		BndStrg_Release(pAd);
		pAd->ApCfg.BandSteering = enable;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (GRN("BndStrg %s Success\n"), pAd->ApCfg.BandSteering?"Enable":"Disable"));
	return TRUE;
}

INT Set_BndStrg_Param(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	PBND_STRG_CLI_TABLE table;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_param *bndstrg_param = &msg.data.bndstrg_param;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if (ifIndex > HW_BEACON_MAX_NUM)
		return FALSE;

	table = Get_BndStrgTable(pAd, ifIndex);

	if (!table)
		return FALSE;

	msg.Action = BNDSTRG_PARAM;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): BndStrg param = %s\n", __func__, arg));

	bndstrg_param->Band = table->Band;
	bndstrg_param->Channel = table->Channel;
	bndstrg_param->len = strlen(arg);
	strncpy(bndstrg_param->arg, arg, sizeof(bndstrg_param->arg) - 1);
	BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UCHAR					MonitorAddr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	PBND_STRG_CLI_TABLE     table;
	POS_COOKIE		        pObj;
	UCHAR			        ifIndex;
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_mnt_addr *mnt_addr = &msg.data.mnt_addr;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	table = Get_BndStrgTable(pAd, ifIndex);

	if (!table)
		return FALSE;

	if (strlen(arg) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&MonitorAddr[i++], 1);
	}

	COPY_MAC_ADDR(table->MonitorAddr, MonitorAddr);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, PRINT_MAC(table->MonitorAddr)));
	msg.Action = SET_MNT_ADDR;
	COPY_MAC_ADDR(mnt_addr->Addr, table->MonitorAddr);
	BndStrgSendMsg(pAd, &msg);

	return TRUE;
}
#endif /* BND_STRG_DBG */

INT BndStrg_Init(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
	INT max_mbss_check_num;
	INT apidx;

	max_mbss_check_num = pAd->ApCfg.BssidNum;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (YLW("%s()\n"), __func__));

	for (apidx = 0; apidx < max_mbss_check_num; apidx++) {
		if (pAd->ApCfg.BndStrgBssIdx[apidx]) {
			ret_val = BndStrg_TableInit(pAd, apidx);
			if (ret_val != BND_STRG_SUCCESS) {
				BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								  ("Error in %s(), error code = %d on apidx = %d\n",
								   __func__, ret_val, apidx));
			}
		}
	}

	return ret_val;
}

#ifdef DOT11K_RRM_SUPPORT
VOID BndStrg_InsertNeighborRepIE(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE init_table)
{
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo = { {0} };
	BSS_STRUCT *pMbss = NULL;
	RRM_BSSID_INFO BssidInfo;
	UINT8 idx, CondensedPhyType = 0; /* 7:2G,9:5G */

	if (init_table->Channel == 0) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Error zero channel\n", __func__));
		return;
	}

	for (idx = 0; idx < HW_BEACON_MAX_NUM; idx++) {
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if (pMbss->wdev.channel == init_table->Channel && pAd->ApCfg.BndStrgBssIdx[idx])
			break;
	}

	if (idx >= HW_BEACON_MAX_NUM) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("[%s]Error no pMbss init to channel table channel:%d\n", __func__, init_table->Channel));
		return;
	}

	COPY_MAC_ADDR(NeighborRepInfo.Bssid, pMbss->wdev.bssid);
	CondensedPhyType = (pMbss->wdev.channel > 14)?9:7; /* 7:2G,9:5G */
	/* our own info */
	BssidInfo.word = 0;
	BssidInfo.field.APReachAble = 3;
	BssidInfo.field.Security = 1; /* rrm to do. */
	BssidInfo.field.KeyScope = 1; /* "report AP has same authenticator as the AP. */
	BssidInfo.field.SepctrumMng = (pMbss->CapabilityInfo & (1 << 8))?1:0;
	BssidInfo.field.Qos = (pMbss->CapabilityInfo & (1 << 9))?1:0;
	BssidInfo.field.APSD = (pMbss->CapabilityInfo & (1 << 11))?1:0;
	BssidInfo.field.RRM = (pMbss->CapabilityInfo & RRM_CAP_BIT)?1:0;
	BssidInfo.field.DelayBlockAck = (pMbss->CapabilityInfo & (1 << 14))?1:0;
	BssidInfo.field.ImmediateBA = (pMbss->CapabilityInfo & (1 << 15))?1:0;

	NeighborRepInfo.BssidInfo = BssidInfo.word;
	NeighborRepInfo.RegulatoryClass = get_regulatory_class(pAd, pMbss->wdev.channel, pMbss->wdev.PhyMode, &pMbss->wdev);
	NeighborRepInfo.ChNum = pMbss->wdev.channel;
	NeighborRepInfo.PhyType = CondensedPhyType;

	NdisCopyMemory(&init_table->NeighborRepInfo, &NeighborRepInfo, sizeof(RRM_NEIGHBOR_REP_INFO));
}
#endif

INT BndStrg_TableInit(PRTMP_ADAPTER pAd, INT apidx)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	PBND_STRG_CLI_TABLE table = NULL, init_table = NULL;
	INT i;
	UINT8 Band, Channel;

	if (pMbss) {
		Band = WMODE_CAP_5G(pMbss->wdev.PhyMode)?BAND_5G : BAND_24G;
		Channel = pMbss->wdev.channel;
		if (!Channel) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(),Invalid Channel:%d pMbss apidx:%d\n", __func__, Channel, apidx));
			return BND_STRG_UNEXP;
		}

	} else {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						  ("Error in %s(), pMbss is NULL!\n", __func__));
		return BND_STRG_UNEXP;
	}

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		table = P_BND_STRG_TABLE(i);

		if (!table->bInitialized && !init_table) {
			init_table = table;
			continue;
		}

		if (table->bInitialized && (table->Channel == Channel))
			return BND_STRG_SUCCESS;
	}

	if (init_table) {
		NdisZeroMemory(init_table, sizeof(BND_STRG_CLI_TABLE));
		OS_NdisAllocateSpinLock(&init_table->Lock);
/* WPS_BandSteering Support */
		OS_NdisAllocateSpinLock(&init_table->WpsWhiteListLock);
		OS_NdisAllocateSpinLock(&init_table->WhiteListLock);
		OS_NdisAllocateSpinLock(&init_table->BlackListLock);

		init_table->DaemonPid = 0xffffffff;
		init_table->priv = (VOID *) pAd;
		init_table->Band = Band;
		init_table->Channel = Channel;
		init_table->bInitialized = TRUE;
/* As in init time wdev may not be populated need to move when ap up complete */
/* moved to BndStrg_SetInfFlags() function. */
/*
#ifdef DOT11K_RRM_SUPPORT
		BndStrg_InsertNeighborRepIE(pAd, init_table);
#endif
*/
	}
	return BND_STRG_SUCCESS;
}

INT BndStrg_Release(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
	INT apidx;
	PBND_STRG_CLI_TABLE table = NULL;
	BSS_STRUCT *pMbss = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (YLW("%s()\n"), __func__));

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		table = Get_BndStrgTable(pAd, apidx);

		if (table) {
			pMbss = &pAd->ApCfg.MBSSID[apidx];
			BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
		}
	}

	if (table)
		BndStrg_TableRelease(table);

	return ret_val;
}

INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table)
{
	INT ret_val = BND_STRG_SUCCESS;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;

	OS_NdisFreeSpinLock(&table->Lock);

	NdisAcquireSpinLock(&table->WhiteListLock);
	ClearBsList(&table->WhiteList);
	NdisReleaseSpinLock(&table->WhiteListLock);

	NdisAcquireSpinLock(&table->BlackListLock);
	ClearBsList(&table->BlackList);
	NdisReleaseSpinLock(&table->BlackListLock);

	NdisAcquireSpinLock(&table->WpsWhiteListLock);
	ClearWpsWhiteList(&table->WpsWhiteList);
	NdisReleaseSpinLock(&table->WpsWhiteListLock);

	OS_NdisFreeSpinLock(&table->WpsWhiteListLock);
	OS_NdisFreeSpinLock(&table->WhiteListLock);
	OS_NdisFreeSpinLock(&table->BlackListLock);
	table->bInitialized = FALSE;

	if (ret_val != BND_STRG_SUCCESS) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						  ("Error in %s(), error code = %d!\n", __func__, ret_val));
	}
	return ret_val;
}

INT BndStrg_InsertEntry(
	PBND_STRG_CLI_TABLE table,
	struct bnd_msg_cli_add *cli_add,
	PBND_STRG_CLI_ENTRY *entry_out)
{
	INT i;
	UCHAR HashIdx;
	PBND_STRG_CLI_ENTRY entry = NULL, this_entry = NULL;
	INT ret_val = BND_STRG_SUCCESS;

	if (table->Size >= BND_STRG_MAX_TABLE_SIZE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): Table is full!\n", __func__));
		return BND_STRG_TABLE_FULL;
	}

	NdisAcquireSpinLock(&table->Lock);

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++) {
		entry = &table->Entry[i];

		/* pick up the first available vacancy*/
		if (entry->bValid == FALSE)	{
			NdisZeroMemory(entry, sizeof(BND_STRG_CLI_ENTRY));
			/* Fill Entry */
			RTMP_GetCurrentSystemTick(&entry->jiffies);
			COPY_MAC_ADDR(entry->Addr, cli_add->Addr);
			entry->TableIndex = cli_add->TableIndex;
			entry->BndStrg_Sta_State = BNDSTRG_STA_INIT;
			entry->bValid = TRUE;
			break;
		}

		entry = NULL;
	}

	if (entry) {
		/* add this MAC entry into HASH table */
		HashIdx = MAC_ADDR_HASH_INDEX(cli_add->Addr);

		if (table->Hash[HashIdx] == NULL)
			table->Hash[HashIdx] = entry;

		else {
			this_entry = table->Hash[HashIdx];

			while (this_entry->pNext != NULL)
				this_entry = this_entry->pNext;

			this_entry->pNext = entry;
		}

		*entry_out = entry;
		table->Size++;
	}

	NdisReleaseSpinLock(&table->Lock);
	return ret_val;
}

INT BndStrg_DeleteEntry(PBND_STRG_CLI_TABLE table, PUCHAR pAddr, UINT32 Index)
{
	USHORT HashIdx;
	PBND_STRG_CLI_ENTRY entry, pre_entry, this_entry;
	INT ret_val = BND_STRG_SUCCESS;

	NdisAcquireSpinLock(&table->Lock);

	if (Index >= BND_STRG_MAX_TABLE_SIZE) {
		if (pAddr == NULL)
			return BND_STRG_INVALID_ARG;

		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		entry = table->Hash[HashIdx];

		while (entry) {
			if (MAC_ADDR_EQUAL(pAddr, entry->Addr)) {
				/* this is the entry we're looking for */
				break;
			} else
				entry = entry->pNext;
		}

		if (entry == NULL) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							  ("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, Entry not found.\n",
							   __func__, Index, PRINT_MAC(pAddr)));
			NdisReleaseSpinLock(&table->Lock);
			return BND_STRG_INVALID_ARG;
		}
	} else {
		entry = &table->Entry[Index];
		HashIdx = MAC_ADDR_HASH_INDEX(entry->Addr);
	}

	if (entry->bValid) {
		{
			pre_entry = NULL;
			this_entry = table->Hash[HashIdx];
			ASSERT(this_entry);

			if (this_entry != NULL) {
				/* update Hash list*/
				do {
					if (this_entry == entry) {
						if (pre_entry == NULL)
							table->Hash[HashIdx] = entry->pNext;
						else
							pre_entry->pNext = entry->pNext;

						break;
					}

					pre_entry = this_entry;
					this_entry = this_entry->pNext;
				} while (this_entry);
			}
			/* not found !!!*/
			ASSERT(this_entry != NULL);

			NdisZeroMemory(entry->Addr, MAC_ADDR_LEN);
			entry->pNext = NULL;
			entry->bValid = FALSE;
			table->Size--;
		}
	} else {
	}

	NdisReleaseSpinLock(&table->Lock);

	return ret_val;
}


PBND_STRG_CLI_ENTRY BndStrg_TableLookup(PBND_STRG_CLI_TABLE table, PUCHAR pAddr)
{
	ULONG HashIdx;
	BND_STRG_CLI_ENTRY *entry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = table->Hash[HashIdx];

	while (entry && entry->bValid) {
		if (MAC_ADDR_EQUAL(entry->Addr, pAddr))
			break;
		else
			entry = entry->pNext;
	}

	return entry;
}

BOOLEAN BndStrg_CheckConnectionReq(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev,
	PUCHAR pSrcAddr,
	struct raw_rssi_info *rssi_info,
	ULONG MsgType,
	PEER_PROBE_REQ_PARAM * ProbeReqParam)
{
	UINT8 FrameType = MsgType;
	CHAR Rssi[4] = {0};
	PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_cli_event *cli_event = &msg.data.cli_event;
	CHAR i, rssi_max;
	PBND_STRG_CLI_ENTRY entry = NULL;

	if (!pAd->ApCfg.BndStrgBssIdx[wdev->func_idx])
		return TRUE;

	if (!table || !table->bEnabled)
		return TRUE;

	/*  send response to white listed clients*/
	if (table->WhiteList.size > 0) {

		PBS_LIST_ENTRY bs_whitelist_entry = NULL;

		bs_whitelist_entry = FindBsListEntry(&table->WhiteList, pSrcAddr);

		if (bs_whitelist_entry) {
			BND_STRG_PRINTQAMSG(table, pSrcAddr, ("BndStrg STA %02x:%02x:%02x:%02x:%02x:%02x  whitelisted\n", PRINT_MAC(pSrcAddr)));
			return TRUE;
		}
	}

	Rssi[0] = rssi_info->raw_rssi[0] ? ConvertToRssi(pAd, (rssi_info), RSSI_IDX_0) : 0;
	Rssi[1] = rssi_info->raw_rssi[1] ? ConvertToRssi(pAd, (rssi_info), RSSI_IDX_1) : 0;
	Rssi[2] = rssi_info->raw_rssi[2] ? ConvertToRssi(pAd, (rssi_info), RSSI_IDX_2) : 0;
	Rssi[3] = rssi_info->raw_rssi[3] ? ConvertToRssi(pAd, (rssi_info), RSSI_IDX_3) : 0;

#ifdef DBDC_MODE

	if (pAd->CommonCfg.dbdc_mode) {
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			rssi_max = pAd->dbdc_band0_rx_path;
		else
			rssi_max = pAd->dbdc_band1_rx_path;
	} else
#endif
		rssi_max = pAd->Antenna.field.RxPath;

	msg.Action = CLI_EVENT;

	if (WMODE_CAP_2G(wdev->PhyMode) &&
		wdev->channel <= 14) {
		cli_event->Band = BAND_24G;
	}

	if (WMODE_CAP_5G(wdev->PhyMode) &&
		wdev->channel > 14)
		cli_event->Band = BAND_5G;

	cli_event->Channel = wdev->channel;
	cli_event->FrameType = FrameType;

	if (FrameType == APMT2_PEER_PROBE_REQ) {
		struct bnd_msg_cli_probe *cli_probe = &cli_event->data.cli_probe;

		if (ProbeReqParam->IsHtSupport && WMODE_CAP_N(wdev->PhyMode))
			cli_probe->bAllowStaConnectInHt = TRUE;
		if (ProbeReqParam->IsVhtSupport && WMODE_CAP_AC(wdev->PhyMode))
			cli_probe->bVHTCapable = TRUE;

		if (ProbeReqParam->IsFromIos)
			cli_probe->bIosCapable = TRUE;

		cli_probe->Nss = GetNssFromHTCapRxMCSBitmask(ProbeReqParam->RxMCSBitmask);

		memset(cli_probe->Rssi, 0x80, sizeof(cli_probe->Rssi));
		for (i = 0; i < rssi_max; i++)
			cli_probe->Rssi[i] = Rssi[i];
	} else if (FrameType == APMT2_PEER_AUTH_REQ) {
		struct bnd_msg_cli_auth *cli_auth = &cli_event->data.cli_auth;

		memset(cli_auth->Rssi, 0x80, sizeof(cli_auth->Rssi));
		for (i = 0; i < rssi_max; i++)
			cli_auth->Rssi[i] = Rssi[i];
	}
	COPY_MAC_ADDR(cli_event->Addr, pSrcAddr);
	BndStrgSendMsg(pAd, &msg);

	/* check for backlist client, stop response for them */
	if (table->BlackList.size > 0) {
		PBS_LIST_ENTRY bs_blacklist_entry = NULL;

		bs_blacklist_entry = FindBsListEntry(&table->BlackList, pSrcAddr);

		if (bs_blacklist_entry) {
			BND_STRG_PRINTQAMSG(table, pSrcAddr, ("BndStrg STA %02x:%02x:%02x:%02x:%02x:%02x  blacklisted\n", PRINT_MAC(pSrcAddr)));
			return FALSE;
		}
	}

/* WPS_BandSteering Support */
	{
		PWSC_CTRL pWscControl;

		pWscControl = &pAd->ApCfg.MBSSID[wdev->func_idx].WscControl;

		if (FrameType == APMT2_PEER_PROBE_REQ) {
			if (pWscControl->bWscTrigger) {
				if (ProbeReqParam->bWpsCapable) {
					NdisAcquireSpinLock(&table->WpsWhiteListLock);
					AddWpsWhiteList(&table->WpsWhiteList, pSrcAddr);
					NdisReleaseSpinLock(&table->WpsWhiteListLock);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("channel %u: Probe req: STA %02x:%02x:%02x:%02x:%02x:%02x wps whitelisted\n",
					table->Channel, PRINT_MAC(pSrcAddr)));
					BND_STRG_PRINTQAMSG(table, pSrcAddr, ("STA %02x:%02x:%02x:%02x:%02x:%02x channel %u  added in WPS Whitelist\n",
					PRINT_MAC(pSrcAddr), table->Channel));
				}
				return TRUE;
			}
		}

		if (FrameType == APMT2_PEER_AUTH_REQ) {
			if (pWscControl->bWscTrigger) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("channel %u: Auth req: STA %02x:%02x:%02x:%02x:%02x:%02x  wps whitelisted\n",
				table->Channel, PRINT_MAC(pSrcAddr)));
				BND_STRG_PRINTQAMSG(table, pSrcAddr, ("STA %02x:%02x:%02x:%02x:%02x:%02x  channel %u allowed Auth as per WPS Whitelist\n",
				PRINT_MAC(pSrcAddr), table->Channel));
				return TRUE;
			}
		}
	}

	if (table->BndStrgMode == POST_CONNECTION_STEERING) {
		return TRUE;
	}

	entry = BndStrg_TableLookup(table, pSrcAddr);

	if (entry && (FrameType == APMT2_PEER_AUTH_REQ) && (entry->BndStrg_Sta_State == BNDSTRG_STA_ASSOC)) {
		BND_STRG_PRINTQAMSG(table, pSrcAddr,
		(RED("%s: (ch%d)  check %s request failed. client's (%02x:%02x:%02x:%02x:%02x:%02x) request is ignored. Client disconnected without DeAuth.!!Waiting for bndstrg result!!\n"
		), (table->Band == BAND_24G ? "2.4G" : "5G"), table->Channel, "Auth", PRINT_MAC(pSrcAddr)));
		return FALSE;
	}

	if (entry) {
#ifdef BND_STRG_QA
		BND_STRG_PRINTQAMSG(table, pSrcAddr,
		(GRN("%s: (ch%d)  check %s request ok. client's (%02x:%02x:%02x:%02x:%02x:%02x) request is accepted.\n"
		), (table->Band == BAND_24G ? "2.4G" : "5G"), table->Channel,
		FrameType == 0 ? ("probe") : (FrameType == 3 ? "auth" : "unknow"), PRINT_MAC(pSrcAddr)));
#endif
		return TRUE;
	} else {
#ifdef BND_STRG_QA
		BND_STRG_PRINTQAMSG(table, pSrcAddr,
		(RED("%s: (ch%d)  check %s request failed. client's (%02x:%02x:%02x:%02x:%02x:%02x) request is ignored.\n"
		), (table->Band == BAND_24G ? "2.4G" : "5G"), table->Channel,
		FrameType == 0 ? ("probe") : (FrameType == 3 ? "auth" : "unknow"), PRINT_MAC(pSrcAddr)));
#endif
		return FALSE;
	}
	return TRUE;
}

INT BndStrg_Tbl_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable, CHAR *IfName)
{
	BNDSTRG_MSG msg = { 0 };
	PRTMP_ADAPTER pAd = NULL;
	struct bnd_msg_onoff *onoff = &msg.data.onoff;

	if (table == NULL)
		return BND_STRG_TABLE_IS_NULL;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;

	if (!(table->bEnabled ^ enable)) {
		/* Already enabled/disabled */
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
						  (GRN("%s(): Band steering is already %s.\n"),
						   __func__, (enable ? "enabled" : "disabled")));
		return BND_STRG_SUCCESS;
	}

	if (enable) {
		table->bEnabled = TRUE;
		strncpy(table->ucIfName, IfName, sizeof(table->ucIfName) - 1); /* decide it by daemon */
	} else
		table->bEnabled = FALSE;

	pAd = (PRTMP_ADAPTER) table->priv;
	msg.Action = BNDSTRG_ONOFF;
	onoff->OnOff = table->bEnabled;
	onoff->Band = table->Band;
	onoff->Channel = table->Channel;
	strncpy(onoff->ucIfName, IfName, sizeof(onoff->ucIfName) - 1);
	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_BNDSTRG_MSG,
		NULL,
		(UCHAR *)&msg,
		sizeof(msg));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					  (GRN("%s(): Band steering %s running.\n"),
					   __func__, (enable ? "start" : "stop")));
	return BND_STRG_SUCCESS;
}

INT BndStrgSendMsg(
	PRTMP_ADAPTER pAd,
	BNDSTRG_MSG *msg)
{
	return	RtmpOSWrielessEventSend(
				pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_BNDSTRG_MSG,
				NULL,
				(UCHAR *) msg,
				sizeof(BNDSTRG_MSG));
}

INT BndStrg_SetInfFlags(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PBND_STRG_CLI_TABLE table,
	BOOLEAN bInfReady)
{
	INT ret_val = BND_STRG_SUCCESS;
	UINT8 Band;
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_inf_status_rsp *inf_status_rsp = &msg.data.inf_status_rsp;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): wdev is NULL!\n", __func__));
		return BND_STRG_UNEXP;
	}

	if (!wdev->if_dev) {
		/* inf down up issue*/
		wdev->bInfReady = FALSE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): if_dev is NULL!\n", __func__));
		return BND_STRG_UNEXP;
	}

	if (!IS_VALID_MAC(wdev->bssid)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): Invalid bssid(%02x:%02x:%02x:%02x:%02x:%02x)!\n", __func__, PRINT_MAC(wdev->bssid)));
		return BND_STRG_UNEXP;
	}

	Band = WMODE_CAP_5G(wdev->PhyMode)?BAND_5G : BAND_24G;

	if (!(wdev->bInfReady^bInfReady)) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						  (GRN("%s(): %s [%d] Inf %s Band steering is already %s.\n"), __func__,
						   (IS_5G_BAND(Band) ? "5G" : "2G"), wdev->channel, wdev->if_dev->name,
						   (bInfReady ? "up" : "down")));
		return BND_STRG_SUCCESS;
	}

	if (bInfReady) { /* Exec. by each interface up */
		table->uIdx = wdev->func_idx;
		/* table->Channel = wdev->channel; */

		if (WMODE_CAP_5G(wdev->PhyMode) && WMODE_CAP_AC(wdev->PhyMode))
			table->bVHTCapable = TRUE;
		else
			table->bVHTCapable = FALSE;

		table->ActiveCount++;
/* Add Neighbor report*/
#ifdef DOT11K_RRM_SUPPORT
		BndStrg_InsertNeighborRepIE(pAd, table);
#endif
	} else { /* Exec. by each interface down */
		if (table->ActiveCount > 0)
			table->ActiveCount--;
	}

	wdev->bInfReady = bInfReady;

	table->nss = wlan_config_get_tx_stream(wdev);
	msg.Action = INF_STATUS_RSP;

	inf_status_rsp->band = Band;
	inf_status_rsp->bInfReady = bInfReady;
	inf_status_rsp->Channel = wdev->channel;
	inf_status_rsp->bVHTCapable = table->bVHTCapable;
	inf_status_rsp->nss = table->nss;
	inf_status_rsp->table_src_addr = (ULONG)table;
	inf_status_rsp->table_size = BND_STRG_MAX_TABLE_SIZE;
	strncpy(inf_status_rsp->ucIfName, wdev->if_dev->name, sizeof(inf_status_rsp->ucIfName));
#ifdef VENDOR_FEATURE5_SUPPORT
	inf_status_rsp->nvram_support = 1;
#else
	inf_status_rsp->nvram_support = 0;
#endif
	BndStrgSendMsg(pAd, &msg);
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  (BLUE("%s(): BSS(%02X:%02X:%02X:%02X:%02X:%02X)")
					   BLUE(" set %s Inf %s %s.\n"), __func__,
					   PRINT_MAC(wdev->bssid), IS_5G_BAND(Band) ? "5G" : "2G",
					   wdev->if_dev->name, bInfReady ? "ready" : "not ready"));

	if (table->bInfReady ^ bInfReady) {
		if (bInfReady) {
			table->bInfReady = TRUE;
			table->Band |= Band;
		} else {
			if (!bInfReady && (table->ActiveCount == 0)) {
				table->bInfReady = FALSE;

				if (table->bEnabled)
					BndStrg_Tbl_Enable(table, FALSE, table->ucIfName);
			}
		}
	}

	return ret_val;
}

void BndStrg_UpdateEntry(PRTMP_ADAPTER pAd,
						 MAC_TABLE_ENTRY *pEntry,
						 IE_LISTS *ie_list,
						 BOOLEAN bConnStatus)
{
	struct wifi_dev *wdev;
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_cli_event *cli_event = &msg.data.cli_event;
	UINT8 Nss = 1;
	UINT32 RxMCSBitmask = 0;
	PBND_STRG_CLI_ENTRY entry = NULL;
	PBND_STRG_CLI_TABLE table = NULL;

	if (!pEntry || !pEntry->wdev || (pAd->ApCfg.BndStrgBssIdx[pEntry->func_tb_idx] != 1))
		return;

	table = Get_BndStrgTable(pAd, pEntry->wdev->func_idx);

	if (!table)
		return;

	wdev = pEntry->wdev;

	if (WMODE_CAP_5G(wdev->PhyMode))
		cli_event->Band = BAND_5G;
	else
		cli_event->Band = BAND_24G;

	cli_event->Channel = wdev->channel;
	COPY_MAC_ADDR(cli_event->Addr, pEntry->Addr);
	msg.Action = CLI_EVENT;

	if (bConnStatus) {
		struct bnd_msg_cli_assoc *cli_assoc = &cli_event->data.cli_assoc;

		cli_event->FrameType = APMT2_PEER_ASSOC_REQ;
		if (ie_list->ht_cap_len > 0) {
			cli_assoc->bAllowStaConnectInHt = TRUE;
			RxMCSBitmask = *(UINT32 *)(ie_list->HTCapability.MCSSet);
			Nss = GetNssFromHTCapRxMCSBitmask(RxMCSBitmask);
		}
		if (ie_list->vht_cap_len > 0)
			cli_assoc->bVHTCapable = TRUE;
		cli_assoc->Nss = Nss;
/* WPS_BandSteering Support */
		cli_assoc->bWpsAssoc = ie_list->bWscCapable;
#ifdef CONFIG_DOT11V_WNM
		cli_assoc->BTMSupport = pEntry->BssTransitionManmtSupport;
#endif
	} else {
		cli_event->FrameType = APMT2_PEER_DISASSOC_REQ;
	}

	BndStrgSendMsg(pAd, &msg);
	entry = BndStrg_TableLookup(table, pEntry->Addr);

	if (entry) {
		if (bConnStatus)
			entry->BndStrg_Sta_State = BNDSTRG_STA_ASSOC;
		else
			entry->BndStrg_Sta_State = BNDSTRG_STA_DISASSOC;
		entry->bConnStatus = bConnStatus;
	}
	return;
}

INT Set_BndStrg_BssIdx(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT8 i;
	RTMP_STRING *macptr;

	for (i = 0, macptr = rstrtok(arg, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		pAd->ApCfg.BndStrgBssIdx[i] = simple_strtoul(macptr, 0, 10);
	}

	return TRUE;
}

#ifdef VENDOR_FEATURE5_SUPPORT
INT Show_BndStrg_NvramTable(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PBNDSTRG_NVRAM_CLIENT	table_nvram_client;
	int i;
	char PhyMode[10];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	table = Get_BndStrgTable(pAd, ifIndex);

	if (!table)
		return FALSE;

	printk("%s bndstrg_nvram_client_count %d\n", __func__, table->bndstrg_nvram_client_count);
	printk("Addr		Band\tPhymde\t\t\t\tNSS\tManipulable\n");

	for (i = 0; i < table->bndstrg_nvram_client_count; i++) {
		table_nvram_client = &table->nvram_entry[i];
		memset(PhyMode, 0x00, 10);

		if (table_nvram_client->PhyMode == fPhyMode_Legacy)
			sprintf(PhyMode, "%s", "Legacy");
		else if (table_nvram_client->PhyMode == fPhyMode_HT)
			sprintf(PhyMode, "%s", "11n");
		else if (table_nvram_client->PhyMode == fPhyMode_VHT)
			sprintf(PhyMode, "%s", "11ac");

		printk("%02x:%02x:%02x:%02x:%02x:%02x \t%-3s\t%-20s\t\t%d\t%s\n", PRINT_MAC(table_nvram_client->Addr),
			   (IS_5G_BAND(table_nvram_client->Band) ? "5G":"2G"), PhyMode, table_nvram_client->Nss, (table_nvram_client->Manipulable ? "YES":"NO"));
	}
	return TRUE;
}

static BOOLEAN BndStrg_NvramTableLookup(PBND_STRG_CLI_TABLE table, unsigned char *pAddr)
{
	int i;

	for (i = 0; i < NVRAM_TABLE_SIZE; i++) {
		if (MAC_ADDR_EQUAL(table->nvram_entry[i].Addr, pAddr))
			return TRUE;
	}
	return FALSE;
}

static BOOLEAN BndStrg_NvramInsertEntry(PBND_STRG_CLI_TABLE table,  PBNDSTRG_NVRAM_CLIENT msg)
{
	PBNDSTRG_NVRAM_CLIENT nvram_entry = NULL;
	int i = 0;

	if (!BndStrg_NvramTableLookup(table, msg->Addr)) {
		if (table->bndstrg_nvram_client_count < NVRAM_TABLE_SIZE)	{
			nvram_entry = &table->nvram_entry[table->bndstrg_nvram_client_count];
			memset(nvram_entry, 0, sizeof(BNDSTRG_NVRAM_CLIENT));
			memcpy(nvram_entry->Addr, msg->Addr, MAC_ADDR_LEN);
			table->bndstrg_nvram_client_count++;
		}
	} else {
		for (i = 0; i < table->bndstrg_nvram_client_count; i++) {
			if (MAC_ADDR_EQUAL(table->nvram_entry[i].Addr, msg->Addr))
				break;
		}

		nvram_entry = &table->nvram_entry[i];
	}

	if (nvram_entry) {
		nvram_entry->Band = msg->Band;
		nvram_entry->Nss = msg->Nss;
		nvram_entry->Manipulable = msg->Manipulable;
		nvram_entry->PhyMode = msg->PhyMode;
	} else {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: nvram_entry is NULL!\n", __func__));
		return FALSE;
	}

	return TRUE;
}

void BndStrg_SetNvram(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx)
{
	BNDSTRG_NVRAM_CLIENT bsdmsg;
	PBND_STRG_CLI_TABLE table = NULL;
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_nvram_entry_update *entry_update = &msg.data.entry_update;
	INT Status;

	table = Get_BndStrgTable(pAd, apidx);

	if (!table ||
		(table->bInitialized == FALSE) ||
		(wrq->u.data.length != sizeof(BNDSTRG_NVRAM_CLIENT)))
		return;

	Status = copy_from_user(&bsdmsg, wrq->u.data.pointer, wrq->u.data.length);

	if ((BndStrg_NvramInsertEntry(table, &bsdmsg))) {
		msg.Action = NVRAM_UPDATE;
		memcpy(entry_update->nvram_entry.Addr, bsdmsg.Addr, MAC_ADDR_LEN);
		memcpy(entry_update->Addr, bsdmsg.Addr, MAC_ADDR_LEN);
		entry_update->nvram_entry.Band = bsdmsg.Band;
		entry_update->nvram_entry.Manipulable = bsdmsg.Manipulable;
		entry_update->nvram_entry.Nss = bsdmsg.Nss;
		entry_update->nvram_entry.PhyMode = bsdmsg.PhyMode;
		BndStrgSendMsg(pAd, &msg);
	} else
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: NVRAM table full\n", __func__));
}

void BndStrg_GetNvram(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx)
{
	PBND_STRG_CLI_TABLE table = NULL;
	BNDSTRG_NVRAM_LIST *nvram_list = NULL;
	PBNDSTRG_NVRAM_CLIENT	nvram_client;
	PBNDSTRG_NVRAM_CLIENT	table_nvram_client;
	int i;

	table = Get_BndStrgTable(pAd, apidx);

	if (!table ||
		(table->bInitialized == FALSE))
		return;

	os_alloc_mem(pAd, (UCHAR **)&nvram_list, sizeof(BNDSTRG_NVRAM_LIST));

	if (nvram_list == NULL)
		return;

	NdisZeroMemory(nvram_list, sizeof(BNDSTRG_NVRAM_LIST));
	nvram_list->Num = table->bndstrg_nvram_client_count;

	for (i = 0; i < nvram_list->Num; i++) {
		table_nvram_client = &table->nvram_entry[i];
		nvram_client = &nvram_list->nvram_entry[i];
		memcpy(nvram_client->Addr, table_nvram_client->Addr, MAC_ADDR_LEN);
		nvram_client->Band = table_nvram_client->Band;
		nvram_client->Manipulable = table_nvram_client->Manipulable;
		nvram_client->PhyMode = table_nvram_client->PhyMode;
		nvram_client->Nss = table_nvram_client->Nss;
	}

	wrq->u.data.length = sizeof(BNDSTRG_NVRAM_LIST);
	copy_to_user(wrq->u.data.pointer, nvram_list, wrq->u.data.length);

	if (nvram_list != NULL)
		os_free_mem(nvram_list);

	return;
}
#endif /* VENDOR_FEATURE5_SUPPORT */

UINT8 GetNssFromHTCapRxMCSBitmask(UINT32 RxMCSBitmask)
{
	UCHAR	RxMCS[4];
	UINT8	nss;
	*((UINT32 *)RxMCS) = RxMCSBitmask;

	if (RxMCS[3] != 0)
		nss = 4;
	else if (RxMCS[2] != 0)
		nss = 3;
	else if (RxMCS[1] != 0)
		nss = 2;
	else
		nss = 1;

	return nss;
}

VOID BndStrg_CLIStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	if (table->bInitialized == TRUE) {
		BNDSTRG_MSG new_msg = { 0 };
		struct bnd_msg_cli_status_rsp *cli_status_rsp = &new_msg.data.cli_status_rsp;
		MAC_TABLE_ENTRY *pEntry = NULL;
		PBND_STRG_CLI_ENTRY entry = NULL;
		int i = 0;
		/* Send to daemon */
		new_msg.Action = CLI_STATUS_RSP;
		memset(cli_status_rsp, 0x00, sizeof(struct bnd_msg_cli_status_rsp));

		for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++) {
			entry = &table->Entry[i];

			/* pick up the first available vacancy*/
			if (!entry || entry->bValid == FALSE || !entry->bConnStatus)
				continue;

			cli_status_rsp->TableIndex = entry->TableIndex;
			pEntry = MacTableLookup(pAd, entry->Addr);

			if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->pMbss != NULL)) {
				INT32	avgrssi = 0;
				/* get information */
				/* RSSI */
				/* avgrssi = RTMPAvgRssi(pAd, &pEntry->RssiSample); */
				{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

					if (cap->fgRateAdaptFWOffload == TRUE && (pEntry->bAutoTxRateSwitch == TRUE)) {
						ULONG DataRate = 0;
						ULONG DataRate_r = 0;
						UCHAR phy_mode, rate, bw, sgi, stbc;
						UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
						UCHAR vht_nss;
						UCHAR vht_nss_r;
#endif
						UINT32 RawData;
						UINT32 RawData_r;
						UINT32 lastTxRate = pEntry->LastTxRate;
						UINT32 lastRxRate = pEntry->LastRxRate;

						if (pEntry->bAutoTxRateSwitch == TRUE) {
							EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
							HTTRANSMIT_SETTING LastTxRate;
							HTTRANSMIT_SETTING LastRxRate;

							MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
							LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
							LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
							LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1:0;
							LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1:0;
							LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

							if (LastTxRate.field.MODE == MODE_VHT)
								LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
							else if (LastTxRate.field.MODE == MODE_OFDM)
								LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
							else
								LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

							lastTxRate = (UINT32)(LastTxRate.word);
							LastRxRate.word = (USHORT)lastRxRate;
							RawData = lastTxRate;
							phy_mode = (RawData>>13) & 0x7;
							rate = RawData & 0x3F;
							bw = (RawData>>7) & 0x3;
							sgi = (RawData>>9) & 0x1;
							stbc = ((RawData>>10) & 0x1);
							/* ---- */
							RawData_r = lastRxRate;
							phy_mode_r = (RawData_r>>13) & 0x7;
							rate_r = RawData_r & 0x3F;
							bw_r = (RawData_r>>7) & 0x3;
							sgi_r = (RawData_r>>9) & 0x1;
							stbc_r = ((RawData_r>>10) & 0x1);
#ifdef DOT11_VHT_AC

							if (phy_mode == MODE_VHT) {
								vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
								rate = rate & 0xF;
								/* snprintf(tmp_str,temp_str_len,"%dS-M%d/",vht_nss, rate); */
							} else
#endif /* DOT11_VHT_AC */
							{
								/* snprintf(tmp_str,temp_str_len,"%d/",rate); */
							}

#ifdef DOT11_VHT_AC

							if (phy_mode_r == MODE_VHT) {
								vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
								rate_r = rate_r & 0xF;
								/* snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%dS-M%d",vht_nss_r, rate_r); */
							} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
								if (phy_mode_r >= MODE_HTMIX) {
									/* snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",rate_r); */
								} else
#endif
									if (phy_mode_r == MODE_OFDM) {
										if (rate_r == TMI_TX_RATE_OFDM_6M)
											LastRxRate.field.MCS = 0;
										else if (rate_r == TMI_TX_RATE_OFDM_9M)
											LastRxRate.field.MCS = 1;
										else if (rate_r == TMI_TX_RATE_OFDM_12M)
											LastRxRate.field.MCS = 2;
										else if (rate_r == TMI_TX_RATE_OFDM_18M)
											LastRxRate.field.MCS = 3;
										else if (rate_r == TMI_TX_RATE_OFDM_24M)
											LastRxRate.field.MCS = 4;
										else if (rate_r == TMI_TX_RATE_OFDM_36M)
											LastRxRate.field.MCS = 5;
										else if (rate_r == TMI_TX_RATE_OFDM_48M)
											LastRxRate.field.MCS = 6;
										else if (rate_r == TMI_TX_RATE_OFDM_54M)
											LastRxRate.field.MCS = 7;
										else
											LastRxRate.field.MCS = 0;

										/* snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS); */
									} else if (phy_mode_r == MODE_CCK) {
										if (rate_r == TMI_TX_RATE_CCK_1M_LP)
											LastRxRate.field.MCS = 0;
										else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
											LastRxRate.field.MCS = 1;
										else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
											LastRxRate.field.MCS = 2;
										else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
											LastRxRate.field.MCS = 3;
										else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
											LastRxRate.field.MCS = 1;
										else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
											LastRxRate.field.MCS = 2;
										else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
											LastRxRate.field.MCS = 3;
										else
											LastRxRate.field.MCS = 0;

										/* snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS); */
									}

							getRate(LastTxRate, &DataRate);
							getRate(LastRxRate, &DataRate_r);
							cli_status_rsp->data_tx_Rate = DataRate;
							cli_status_rsp->data_rx_Rate = DataRate_r;
							cli_status_rsp->data_tx_Phymode = phy_mode;
							cli_status_rsp->data_rx_Phymode = phy_mode_r;

							if (LastTxRate.field.MODE >= MODE_VHT) {
								cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS & 0xf;
								cli_status_rsp->data_tx_ant = (LastTxRate.field.MCS>>4) + 1;
							} else if (LastTxRate.field.MODE >= MODE_HTMIX) {
								cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
								cli_status_rsp->data_tx_ant = (LastTxRate.field.MCS >> 3)+1;

								if (cli_status_rsp->data_tx_mcs > 7)
									cli_status_rsp->data_tx_mcs %= 8;
							} else if (LastTxRate.field.MODE >= MODE_OFDM) {
								cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
								cli_status_rsp->data_tx_ant = 1;
							} else if (LastTxRate.field.MODE >= MODE_CCK) {
								cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
								cli_status_rsp->data_tx_ant = 1;
							}

							if (LastRxRate.field.MODE >= MODE_VHT) {
								cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS & 0xf;
								cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS>>4) + 1;
							} else if (LastRxRate.field.MODE >= MODE_HTMIX) {
								cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
								cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS >> 3)+1;

								if (cli_status_rsp->data_rx_mcs > 7)
									cli_status_rsp->data_rx_mcs %= 8;
							} else if (LastRxRate.field.MODE >= MODE_OFDM) {
								cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
								cli_status_rsp->data_rx_ant = 1;
							} else if (LastRxRate.field.MODE >= MODE_CCK) {
								cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
								cli_status_rsp->data_rx_ant = 1;
							}

							cli_status_rsp->data_tx_bw = LastTxRate.field.BW;
							cli_status_rsp->data_rx_bw = LastRxRate.field.BW;
							cli_status_rsp->data_tx_sgi = LastTxRate.field.ShortGI;
							cli_status_rsp->data_rx_sgi = LastRxRate.field.ShortGI;
							cli_status_rsp->data_tx_stbc = LastTxRate.field.STBC;
							cli_status_rsp->data_rx_stbc = LastRxRate.field.STBC;
							cli_status_rsp->data_tx_packets = pEntry->TxPackets.QuadPart;
							cli_status_rsp->data_rx_packets = pEntry->RxPackets.QuadPart;
						}
					} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
					{
						ULONG DataRate = 0;
						ULONG DataRate_r = 0;
						UINT32 lastRxRate = pEntry->LastRxRate;
						HTTRANSMIT_SETTING LastRxRate;
						UCHAR phy_mode_r, rate_r;

						LastRxRate.word = (USHORT)lastRxRate;
						phy_mode_r = LastRxRate.field.MODE;
						rate_r = LastRxRate.word & 0x3F;

						if (phy_mode_r == MODE_OFDM) {
							if (rate_r == TMI_TX_RATE_OFDM_6M)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_OFDM_9M)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_OFDM_12M)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_OFDM_18M)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_OFDM_24M)
								LastRxRate.field.MCS = 4;
							else if (rate_r == TMI_TX_RATE_OFDM_36M)
								LastRxRate.field.MCS = 5;
							else if (rate_r == TMI_TX_RATE_OFDM_48M)
								LastRxRate.field.MCS = 6;
							else if (rate_r == TMI_TX_RATE_OFDM_54M)
								LastRxRate.field.MCS = 7;
							else
								LastRxRate.field.MCS = 0;
						} else if (phy_mode_r == MODE_CCK) {
							if (rate_r == TMI_TX_RATE_CCK_1M_LP)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
								LastRxRate.field.MCS = 3;
							else
								LastRxRate.field.MCS = 0;
						}

						getRate(pEntry->HTPhyMode, &DataRate);
						getRate(LastRxRate, &DataRate_r);
						cli_status_rsp->data_tx_Rate = DataRate;
						cli_status_rsp->data_rx_Rate = DataRate_r;
						cli_status_rsp->data_tx_Phymode = pEntry->HTPhyMode.field.MODE;
						cli_status_rsp->data_rx_Phymode = 0;

						if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
							cli_status_rsp->data_tx_mcs = (pEntry->HTPhyMode.field.MCS & 0xf);
							cli_status_rsp->data_tx_ant = (pEntry->HTPhyMode.field.MCS>>4) + 1;
						} else if (pEntry->HTPhyMode.field.MODE >= MODE_HTMIX) {
							cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
							cli_status_rsp->data_tx_ant = (pEntry->HTPhyMode.field.MCS >> 3)+1;

							if (cli_status_rsp->data_tx_mcs > 7)
								cli_status_rsp->data_tx_mcs %= 8;
						} else if (pEntry->HTPhyMode.field.MODE >= MODE_OFDM) {
							cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
							cli_status_rsp->data_tx_ant = 1;
						} else if (pEntry->HTPhyMode.field.MODE >= MODE_CCK) {
							cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
							cli_status_rsp->data_tx_ant = 1;
						}

						if (LastRxRate.field.MODE >= MODE_VHT) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS & 0xf;
							cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS>>4) + 1;
						} else if (LastRxRate.field.MODE >= MODE_HTMIX) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS >> 3)+1;

							if (cli_status_rsp->data_rx_mcs > 7)
								cli_status_rsp->data_rx_mcs %= 8;
						} else if (LastRxRate.field.MODE >= MODE_OFDM) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = 1;
						} else if (LastRxRate.field.MODE >= MODE_CCK) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = 1;
						}

						cli_status_rsp->data_tx_bw = pEntry->HTPhyMode.field.BW;
						cli_status_rsp->data_rx_bw = LastRxRate.field.BW;
						cli_status_rsp->data_tx_sgi = pEntry->HTPhyMode.field.ShortGI;
						cli_status_rsp->data_rx_sgi = LastRxRate.field.ShortGI;
						cli_status_rsp->data_tx_stbc = pEntry->HTPhyMode.field.STBC;
						cli_status_rsp->data_rx_stbc = LastRxRate.field.STBC;
						cli_status_rsp->data_tx_packets = pEntry->TxPackets.QuadPart;
						cli_status_rsp->data_rx_packets = pEntry->RxPackets.QuadPart;
					}
				}
				/* Calculate Avg data RSSI */
				{
					INT rssi_tmp = 0, i = 0;
					UINT32	rx_stream;

					if (pAd->CommonCfg.dbdc_mode)
						rx_stream = 2;
					else
						rx_stream = pAd->Antenna.field.RxPath;

					for (i = 0; i < rx_stream; i++)
						rssi_tmp += pEntry->RssiSample.AvgRssi[i];

					avgrssi = (CHAR)(rssi_tmp/rx_stream);
				}
				cli_status_rsp->data_Rssi = (char)avgrssi;
				/* cli_status_rsp->data_tx_TP = pEntry->AvgTxBytes >> 17; //Mbps */
				/* cli_status_rsp->data_rx_TP = pEntry->AvgRxBytes >> 17; //Mbps */
				cli_status_rsp->data_tx_Byte = pEntry->AvgTxBytes;
				cli_status_rsp->data_rx_Byte = pEntry->AvgRxBytes;
				memcpy(cli_status_rsp->Addr, entry->Addr, MAC_ADDR_LEN);
				cli_status_rsp->ReturnCode = BND_STRG_SUCCESS;
				BndStrgSendMsg(pAd, &new_msg);
			}
		}
	}

	return;
}

VOID BndStrg_ChannelLoadStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	if (table->bInitialized == TRUE) {
		BNDSTRG_MSG new_msg = { 0 };
		struct bnd_msg_chanload_status_rsp *chanload_status_rsp = &new_msg.data.chanload_status_rsp;
		/* Send to daemon */
		new_msg.Action = CHANLOAD_STATUS_RSP;
		chanload_status_rsp->ReturnCode = BND_STRG_SUCCESS;
		chanload_status_rsp->band = table->Band;
		chanload_status_rsp->Channel = table->Channel;
		{
			UINT32  ChanBusyTime[DBDC_BAND_NUM] = {0};
			UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
			UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
			UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
			UINT32  EDCCATime[DBDC_BAND_NUM] = {0};
			UCHAR   ChanBusyOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyTxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyRxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   EdccaOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR	i;

			i = HcGetBandByChannel(pAd, table->Channel);
			ChanBusyTime[i] = pAd->OneSecMibBucket.ChannelBusyTime[i];
			ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
			MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
			MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
			EDCCATime[i] = Get_EDCCA_Time(pAd, i);

			if (ChanBusyTime[i] != 0)
				ChanBusyOccupyPercentage[i] = (ChanBusyTime[i]*100)/ONE_SEC_2_US;

			if (ObssAirTime[i] != 0)
				ObssAirOccupyPercentage[i] = (ObssAirTime[i]*100)/ONE_SEC_2_US;

			if (MyTxAirTime[i] != 0 || MyRxAirTime[i] != 0)
				MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) * 100)/ONE_SEC_2_US;

			if (MyTxAirTime[i] != 0)
				MyTxAirOccupyPercentage[i] = (MyTxAirTime[i] * 100) / ONE_SEC_2_US;

			if (MyRxAirTime[i] != 0)
				MyRxAirOccupyPercentage[i] = (MyRxAirTime[i] * 100) / ONE_SEC_2_US;

			if (EDCCATime[i] != 0)
				EdccaOccupyPercentage[i] = (EDCCATime[i] * 100) / ONE_SEC_2_US;

			chanload_status_rsp->chanload = MyAirOccupyPercentage[i] + ObssAirOccupyPercentage[i];
		}
		BndStrgSendMsg(pAd, &new_msg);
	}
}

VOID BndStrg_InfStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	struct bnd_msg_inf_status_req *inf_status_req = &msg->data.inf_status_req;

	if (table->bInitialized == TRUE) {
		BNDSTRG_MSG new_msg = { 0 };
		struct bnd_msg_inf_status_rsp *inf_status_rsp = &new_msg.data.inf_status_rsp;
		/* Send to daemon */
		new_msg.Action = INF_STATUS_RSP;

		inf_status_rsp->band = table->Band;
		inf_status_rsp->bInfReady = table->bInfReady;
		inf_status_rsp->Channel = table->Channel;
		inf_status_rsp->bVHTCapable = table->bVHTCapable;
		inf_status_rsp->nss = table->nss;
		inf_status_rsp->table_src_addr = (ULONG)table;
		inf_status_rsp->table_size = BND_STRG_MAX_TABLE_SIZE;
		strncpy(inf_status_rsp->ucIfName, inf_status_req->ucIfName, sizeof(inf_status_rsp->ucIfName) - 1);
#ifdef VENDOR_FEATURE5_SUPPORT
		inf_status_rsp->nvram_support = 1;
#else
		inf_status_rsp->nvram_support = 0;
#endif
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%s:INF [%s]STATUS QUERY ON\n", __func__, inf_status_req->ucIfName));
		BndStrgSendMsg(pAd, &new_msg);
	}
}

VOID BndStrg_KickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS     NStatus;
	UCHAR           BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (apidx < pAd->ApCfg.BssidNum) {
		/* Send out a Deauthentication request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Send DISASSOC frame(%d) with ra%d\n", Reason, apidx));
		/* 802.11 Header */
		NdisZeroMemory(&DisassocHdr, sizeof(HEADER_802_11));
		DisassocHdr.FC.Type = FC_TYPE_MGMT;
		DisassocHdr.FC.SubType = SUBTYPE_DISASSOC;
		DisassocHdr.FC.ToDs = 0;
		DisassocHdr.FC.Wep = 0;
		COPY_MAC_ADDR(DisassocHdr.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		COPY_MAC_ADDR(DisassocHdr.Addr3, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2, &Reason,
						  END_OF_ARGS);

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
	}
}

VOID BndStrg_handle_onoff_event(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	struct bnd_msg_onoff *onoff = &msg->data.onoff;
	UINT32	ap_idx;
	UINT32 i;

	if (onoff->OnOff && (table->bEnabled ^ onoff->OnOff)) {
		/* disconnect all connected STA to keep the link status
		 * between bndstrg daemon and driver
		 */
		for (ap_idx = MAIN_MBSSID; ap_idx < pAd->ApCfg.BssidNum; ap_idx++) {
			if (pAd->ApCfg.BndStrgBssIdx[ap_idx] == TRUE)
				BndStrg_KickOutAllSta(pAd, ap_idx, REASON_DEAUTH_STA_LEAVING);
		}

		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry) && pAd->ApCfg.BndStrgBssIdx[pEntry->func_tb_idx] == TRUE) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s: MacTableDeleteEntry %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, PRINT_MAC(pEntry->Addr)));
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			}
		}

		BndStrg_Send_NeighborReport(pAd, table);
		table->BndStrgMode = onoff->BndStrgMode;
	} else if ((onoff->OnOff == 0) && (table->bEnabled ^ onoff->OnOff)) {
		if (table->Size > 0) {
			PBND_STRG_CLI_ENTRY entry = NULL;

			for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++) {
				entry = &table->Entry[i];

				if (entry->bValid == TRUE)
					BndStrg_DeleteEntry(table, entry->Addr, i);
			}
		}
	}

	BndStrg_Tbl_Enable(table, onoff->OnOff, onoff->ucIfName);

	if (table->bEnabled)
		table->DaemonPid = current->pid;
	else
		table->DaemonPid = 0xffffffff;
				return;
}

VOID BndStrg_UpdateWhiteBlackList(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	struct bnd_msg_update_white_black_list *update_list = &msg->data.update_white_black_list;
	PLIST_HEADER pUpdateList = NULL;
	NDIS_SPIN_LOCK *pBsListLock = NULL;

	BND_STRG_PRINTQAMSG(table, update_list->Addr, ("%s: client[%02x:%02x:%02x:%02x:%02x:%02x] %s %s\n", __func__, PRINT_MAC(update_list->Addr),
		(update_list->deladd ? "Add to":"Remove From"), ((update_list->list_type == bndstrg_whitelist) ? "WhiteList" : "BlackList")));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: client[%02x:%02x:%02x:%02x:%02x:%02x] %s %s\n", __func__, PRINT_MAC(update_list->Addr),
		(update_list->deladd ? "Add to":"Remove From"), ((update_list->list_type == bndstrg_whitelist) ? "WhiteList" : "BlackList")));

	if (update_list->list_type == bndstrg_whitelist) {
		pUpdateList = &table->WhiteList;
		pBsListLock = &table->WhiteListLock;
	} else if (update_list->list_type == bndstrg_blacklist) {
		pUpdateList = &table->BlackList;
		pBsListLock = &table->BlackListLock;
	} else
		return;

	if (update_list->deladd) {
		NdisAcquireSpinLock(pBsListLock);
		AddBsListEntry(pUpdateList, update_list->Addr);
		NdisReleaseSpinLock(pBsListLock);
	} else {
		NdisAcquireSpinLock(pBsListLock);
		DelBsListEntry(pUpdateList, update_list->Addr);
		NdisReleaseSpinLock(pBsListLock);
	}
}

INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx)
{
	PBND_STRG_CLI_ENTRY entry = NULL;
	BNDSTRG_MSG msg_copy = { 0 };
	BNDSTRG_MSG *msg = &msg_copy;
	PBND_STRG_CLI_TABLE table = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	table = Get_BndStrgTable(pAd, apidx);

	if (!table || (table->bInitialized == FALSE)) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						  ("%s: BND_STRG_NOT_INITIALIZED on apidex[%d]!\n", __func__, apidx));
		return BND_STRG_NOT_INITIALIZED;
	}

	if (wrq->u.data.length > sizeof(BNDSTRG_MSG)) {
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						  ("%s: The length of message not match!\n", __func__));
		return BND_STRG_INVALID_ARG;
	} else {
		Status = copy_from_user(&msg_copy, wrq->u.data.pointer, wrq->u.data.length);
	}

		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: action code (%d)\n", __func__, msg->Action));

		if ((table->DaemonPid != 0xffffffff) && (table->DaemonPid != current->pid)) {
			BNDSTRG_MSG new_msg = { 0 };

			new_msg.Action = REJECT_EVENT;
			new_msg.data.reject_body.DaemonPid = table->DaemonPid;
			BndStrgSendMsg(pAd, &new_msg);
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Unknown BndStrg PID\n", __func__));
			return BND_STRG_SUCCESS;
		}

		switch (msg->Action) {
		case CLI_ADD: {
			struct bnd_msg_cli_add *cli_add = &msg->data.cli_add;

#ifdef BND_STRG_QA
		BND_STRG_PRINTQAMSG(table, cli_add->Addr, (("%s[%d][Channel:%d]: Add (%02x:%02x:%02x:%02x:%02x:%02x) client to %s driver table\n\r"),
			__func__, __LINE__, table->Channel, PRINT_MAC(cli_add->Addr), (IS_5G_BAND(table->Band) ? "5G":"2.4G")));
#endif

			entry = BndStrg_TableLookup(table, cli_add->Addr);

		if (entry == NULL) {
			if (BndStrg_InsertEntry(table, cli_add, &entry) == BND_STRG_SUCCESS) {
				if (table->BndStrgMode == POST_CONNECTION_STEERING) {
					entry->bConnStatus = TRUE;
					entry->BndStrg_Sta_State = BNDSTRG_STA_ASSOC;
				}
			}
		}
			else
				entry->BndStrg_Sta_State = BNDSTRG_STA_INIT;
		}
		break;

		case CLI_DEL: {
			struct bnd_msg_cli_del *cli_del = &msg->data.cli_del;
			MAC_TABLE_ENTRY *pEntry = NULL;

			if (BndStrg_TableLookup(table, cli_del->Addr)) {
				/*remove sta if sta is existed*/
				pEntry = MacTableLookup(pAd, cli_del->Addr);

				if (pEntry && (apidx == pEntry->func_tb_idx)
						&& pAd->ApCfg.BndStrgBssIdx[pEntry->func_tb_idx]) {
#ifdef BND_STRG_QA
				BND_STRG_PRINTQAMSG(table, cli_del->Addr, (("%s[%d]: kick out client's (%02x:%02x:%02x:%02x:%02x:%02x)\n\r"),
										 __func__, __LINE__, PRINT_MAC(cli_del->Addr)));
#endif
					MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}

				BndStrg_DeleteEntry(table, cli_del->Addr, 0xFF);
#ifdef BND_STRG_QA
			BND_STRG_PRINTQAMSG(table, cli_del->Addr, (("%s[%d][Channel:%d]:DEL (%02x:%02x:%02x:%02x:%02x:%02x) client from %s driver table\n\r"),
				__func__, __LINE__, table->Channel, PRINT_MAC(cli_del->Addr), (IS_5G_BAND(table->Band) ? "5G":"2.4G")));
#endif
			}
		}
		break;
		case CLI_STATUS_REQ:
			if (table->bInfReady)
				BndStrg_CLIStatusRsp(pAd, table, msg);

			break;

		case CHANLOAD_STATUS_REQ:
			if (table->bInfReady)
				BndStrg_ChannelLoadStatusRsp(pAd, table, msg);

			break;

		case INF_STATUS_QUERY:
			if (table->bInfReady)
				BndStrg_InfStatusRsp(pAd, table, msg);

			break;

		case HEARTBEAT_MONITOR:
			if (table->bInfReady)
				pAd->ApCfg.BndStrgHeartbeatCount++;

			break;

		case BNDSTRG_ONOFF:
		if (table && table->bInitialized)
			BndStrg_handle_onoff_event(pAd, table, msg);
			break;
#ifdef VENDOR_FEATURE5_SUPPORT
		case NVRAM_UPDATE: {
			struct bnd_msg_nvram_entry_update *entry_update = &msg->data.entry_update;

			if (!(BndStrg_NvramInsertEntry(table, &entry_update->nvram_entry)))
				BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: NVRAM table full\n", __func__));
		}
		break;
#endif /* VENDOR_FEATURE5_SUPPORT */

		case BNDSTRG_WNM_BTM:
#ifdef CONFIG_DOT11V_WNM
		{
			struct wnm_command *cmd_data = (struct wnm_command *)&msg->data.wnm_cmd_data;

			hex_dump("BNDSTRG_WNM_BTM MSG", (unsigned char *)msg, sizeof(BNDSTRG_MSG));
			if (cmd_data->command_id == OID_802_11_WNM_CMD_SEND_BTM_REQ) {
				MAC_TABLE_ENTRY  *pEntry;
				unsigned char *request_mode;
				struct btm_req_data *req_data;
#define DISASSOC_IMMINENT (0x01 << 2)
				req_data = (struct btm_req_data *)cmd_data->command_body;
				request_mode = (unsigned char *)req_data->btm_req;
				if ((*request_mode) & DISASSOC_IMMINENT) {
					/* imminent dis-assoc BTM Req, delete entry from driver table */
					BndStrg_DeleteEntry(table, req_data->peer_mac_addr, 0xFF);
				}
				pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);
				if (pEntry != NULL) {
					BndStrg_send_BTM_req(pAd,
						req_data->peer_mac_addr,
						req_data->btm_req,
						req_data->btm_req_len,
						table);
				}
			} else{
				hex_dump("BNDSTRG_WNM_BTM", (unsigned char *)cmd_data, cmd_data->command_len);
				BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow wnm cmd id %d\n", __func__, cmd_data->command_id));
			}
		}
#endif
		break;

		case UPDATE_WHITE_BLACK_LIST:
			if (table->bInfReady) {
				BndStrg_UpdateWhiteBlackList(pAd, table, msg);
			}
		break;

		default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: unknown action code. (%d)\n", __func__, msg->Action));
		break;
	}

	return BND_STRG_SUCCESS;
}

void BndStrgHeartBeatMonitor(PRTMP_ADAPTER	pAd)
{
	if (pAd->ApCfg.BndStrgTable[BAND0].bEnabled
#ifdef DBDC_MODE
		|| pAd->ApCfg.BndStrgTable[BAND1].bEnabled
#endif
	   ) {
		if (pAd->ApCfg.BndStrgHeartbeatMonitor != pAd->ApCfg.BndStrgHeartbeatCount) {
			pAd->ApCfg.BndStrgHeartbeatMonitor = pAd->ApCfg.BndStrgHeartbeatCount;
			pAd->ApCfg.BndStrgHeartbeatNoChange = 0;
			return;
		} else
			pAd->ApCfg.BndStrgHeartbeatNoChange++;

		if (pAd->ApCfg.BndStrgHeartbeatNoChange == 20) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:BndStrg Daemon Killed\n", __func__));
			pAd->ApCfg.BndStrgHeartbeatNoChange = 0;
			BndStrg_Release(pAd);
		}
	}
}

void BndStrgSetProfileParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	INT	i = 0;
	RTMP_STRING *macptr = NULL;

	if (RTMPGetKeyParameter("BandSteering", tmpbuf, 10, pBuffer, TRUE)) {
		pAd->ApCfg.BandSteering = (UCHAR) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("BandSteering=%d\n", pAd->ApCfg.BandSteering));
	}

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, 50, pBuffer, TRUE)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("BndStrgBssIdx=%s\n", tmpbuf));

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			pAd->ApCfg.BndStrgBssIdx[i] = simple_strtoul(macptr, 0, 10);

		if (i == 0)
			pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
	} else
		pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
}
#ifdef CONFIG_DOT11V_WNM
void BndStrg_send_BTM_req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *BTMReq,
	IN UINT32 BTMReqLen,
	PBND_STRG_CLI_TABLE table)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	BTM_EVENT_DATA *Event;
	BTM_PEER_ENTRY *BTMPeerEntry;
#ifdef DOT11K_RRM_SUPPORT
	RRM_NEIGHBOR_REP_INFO *NeighborReport = NULL;
	ULONG FrameLen = 0;
#endif
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

		if (!BTMPeerEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
			goto error0;
		}

		NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));

		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(BTMPeerEntry->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
		BTMPeerEntry->DialogToken = 1;
		BTMPeerEntry->Priv = pAd;

		RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
				GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	}

#ifdef DOT11K_RRM_SUPPORT
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + BTMReqLen + sizeof(RRM_NEIGHBOR_REP_INFO) + 3);
													/* Size 3 is for the preference sub element in neighbor report*/
#else
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + BTMReqLen);
#endif

	if (!Buf) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Not available memory\n", __func__));
		goto error1;
	}

#ifdef DOT11K_RRM_SUPPORT
	NdisZeroMemory(Buf, sizeof(*Event) + BTMReqLen + sizeof(RRM_NEIGHBOR_REP_INFO) + 3);
										/* Size 3 is for the preference sub element in neighbor report*/
#else
	NdisZeroMemory(Buf, sizeof(*Event) + BTMReqLen);
#endif

	Event = (BTM_EVENT_DATA *)Buf;

	Event->ControlIndex = APIndex;
	Len += 1;

	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;

	Event->EventType = BTM_REQ;
	Len += 2;

	Event->u.BTM_REQ_DATA.DialogToken = BTMPeerEntry->DialogToken;
	Len += 1;

	Event->u.BTM_REQ_DATA.BTMReqLen = BTMReqLen;
	Len += 2;

	NdisMoveMemory(Event->u.BTM_REQ_DATA.BTMReq, BTMReq, BTMReqLen);

	BTMReq += BTMReqLen; /*advance pointer to neighbour report*/
#ifdef DOT11K_RRM_SUPPORT
	hex_dump("BndStrg_send_BTM_req NeighborReport", (unsigned char *)BTMReq, sizeof(RRM_NEIGHBOR_REP_INFO));
	NeighborReport = (RRM_NEIGHBOR_REP_INFO *)BTMReq;

	RRM_InsertNeighborRepIE(pAd, (Event->u.BTM_REQ_DATA.BTMReq + BTMReqLen), &FrameLen,
		sizeof(RRM_NEIGHBOR_REP_INFO)+3, NeighborReport); /* Neighbor report IE + Preferance Sub-IE length*/
	BTMReqLen += FrameLen;

	{
		ULONG TempLen = 0;
		UINT8 IEId = 3;
		UINT8 Len = 1;
		UINT8 BssPreference = 255;

		MakeOutgoingFrame((Event->u.BTM_REQ_DATA.BTMReq + BTMReqLen), &TempLen,
							1,		&IEId,
							1,		&Len,
							1,		&BssPreference,
							END_OF_ARGS);
		BTMReqLen += TempLen;
	}
	Event->u.BTM_REQ_DATA.BTMReqLen = BTMReqLen;
	Len += BTMReqLen;
#endif
	hex_dump("BndStrg_send_BTM_req Enque", (unsigned char *)Buf, Len);
	hex_dump("BndStrg_send_BTM_req Event length", (unsigned char *)Event->u.BTM_REQ_DATA.BTMReq, Event->u.BTM_REQ_DATA.BTMReqLen);
	MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ, Len, Buf, 0);

	os_free_mem(Buf);

	return;

error1:
	if (!IsFound)
		os_free_mem(BTMPeerEntry);
error0:
	return;
}

#endif

void BndStrg_Send_NeighborReport(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table)
{
#ifdef	DOT11K_RRM_SUPPORT
	BNDSTRG_MSG msg = { 0 };
	struct bnd_msg_neighbor_report *NeighborReport = &msg.data.Neighbor_Report;

	msg.Action = BNDSTRG_NEIGHBOR_REPORT;

	NeighborReport->Band = table->Band;
	NeighborReport->Channel = table->Channel;
	NdisCopyMemory(&NeighborReport->NeighborRepInfo, &table->NeighborRepInfo, sizeof(RRM_NEIGHBOR_REP_INFO));
	BndStrgSendMsg(pAd, &msg);
#endif
	return;
}

#endif /* BAND_STEERING */

