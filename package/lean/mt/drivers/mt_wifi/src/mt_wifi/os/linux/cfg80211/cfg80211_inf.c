/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2012, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cfg80211_inf.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	YF Luo		06-28-2012		Init version
			12-26-2013		Integration of NXTC
*/
#define RTMP_MODULE_OS

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if (KERNEL_VERSION(2, 6, 28) <= LINUX_VERSION_CODE)
#ifdef RT_CFG80211_SUPPORT

extern INT apcli_tx_pkt_allowed(
	RTMP_ADAPTER * pAd, struct wifi_dev *wdev,
	PNDIS_PACKET pPacket, UCHAR * pWcid);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, VOID *pData)
{
	PCFG80211_CTRL pCfg80211_ctrl = NULL;
	UINT newType, oldType;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM *pVifParm;

	pVifParm = (CMD_RTPRIV_IOCTL_80211_VIF_PARM *)pData;
	pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	newType = pVifParm->newIfType;
	oldType = pVifParm->oldIfType;
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	CFG80211DBG(DBG_LVL_TRACE, ("80211> @@@ Change from %u  to %u Mode\n", oldType, newType));
	pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_DISABLE;

	if (newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_CLI_UP;
	else if (newType == RT_CMD_80211_IFTYPE_P2P_GO)
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_GO_UP;

#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
	/* Change Device Type */
		if ((newType == RT_CMD_80211_IFTYPE_STATION) ||
			(newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)) {
			CFG80211DBG(DBG_LVL_TRACE, ("80211> Change the Interface to STA Mode\n"));
#ifdef CONFIG_AP_SUPPORT

			if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
				CFG80211DRV_DisableApInterface(pAd);

#endif /* CONFIG_AP_SUPPORT */
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;
		} else if ((newType == RT_CMD_80211_IFTYPE_AP) ||
				   (newType == RT_CMD_80211_IFTYPE_P2P_GO)) {
			CFG80211DBG(DBG_LVL_TRACE, ("80211> Change the Interface to AP Mode\n"));
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		}


	if ((newType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
		(newType == RT_CMD_80211_IFTYPE_P2P_GO))
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pVifParm->net_dev->dev_addr);
	else {
	}

	return TRUE;
}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;

	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	/* Already one VIF in list */
	if (pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn)
		return FALSE;

	pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = TRUE;
	RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType);
	return TRUE;
}

BOOLEAN RTMP_CFG80211_VIF_ON(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	return pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn;
}



static
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByMac(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

VOID RTMP_CFG80211_AddVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pNewVifDev, sizeof(CFG80211_VIF_DEV));

	if (pNewVifDev) {
		os_zero_mem(pNewVifDev, sizeof(CFG80211_VIF_DEV));
		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		os_zero_mem(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);
		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (RT_LIST_ENTRY *)pNewVifDev);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType));
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in alloc mem in New CFG80211 VIF Function.\n"));
}

VOID RTMP_CFG80211_RemoveVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)RTMP_CFG80211_FindVifEntry_ByMac(pAd, pNewNetDev);

	if (pListEntry) {
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(pListEntry);
		pListEntry = NULL;
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in RTMP_CFG80211_RemoveVifEntry.\n"));
}

PNET_DEV RTMP_CFG80211_VirtualIF_Get(
	IN VOID                 *pAdSrc
)
{
	/* PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc; */
	/* return pAd->Cfg80211VifDevSet.Cfg80211VifDev[0].net_dev; */
	return NULL;
}

static INT CFG80211_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAdSrc;
	PRTMP_ADAPTER pAd;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ===> %d,%s\n", __func__, dev_p->ifindex,
			 RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));
	/* if (VIRTUAL_IF_UP(pAd, dev_p) != 0) */
	/* return -1; */
	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(dev_p);

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
#ifdef CFG80211_MULTI_STA
		|| (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */
	   ) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_Open\n"));
		pAd->flg_apcli_init = TRUE;
		ApCli_Open(pAd, dev_p);
		return 0;
	}

	RTMP_OS_NETDEV_START_QUEUE(dev_p);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <=== %s\n", __func__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));
	return 0;
}

static INT CFG80211_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAdSrc;
	PRTMP_ADAPTER pAd;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
#ifdef CFG80211_MULTI_STA
		|| (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */
	   ) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_Close\n"));
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
		RT_MOD_HNAT_DEREG(dev_p);
		RT_MOD_DEC_USE_COUNT();
		return ApCli_Close(pAd, dev_p);
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ===> %s\n", __func__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));
	RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

	if (netif_carrier_ok(dev_p))
		netif_carrier_off(dev_p);

	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(
			pAd,
			pAd->CommonCfg.BeaconPeriod,
			HW_BSSID_0,
			OPMODE_ADHOC);
	else
		AsicDisableSync(pAd, HW_BSSID_0);

	/* VIRTUAL_IF_DOWN(pAd); */
	RT_MOD_HNAT_DEREG(dev_p);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}

static INT CFG80211_PacketSend(PNDIS_PACKET pPktSrc, PNET_DEV pDev, RTMP_NET_PACKET_TRANSMIT Func)
{
	PRTMP_ADAPTER pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);

	/* To Indicate from Which VIF */
	switch (pDev->ieee80211_ptr->iftype) {
	case RT_CMD_80211_IFTYPE_AP:
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_GO:
		;

		if (!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Drop the Packet due P2P GO not in ready state\n"));
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			return 0;
		}

		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_CLIENT:
	case RT_CMD_80211_IFTYPE_STATION:
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	default:
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unknown CFG80211 I/F Type(%d)\n", pDev->ieee80211_ptr->iftype));
		RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
		return 0;
	}

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211 Packet Type  [%s](%d)\n",
			 pDev->name, pDev->ieee80211_ptr->iftype));
	return Func(RTPKT_TO_OSPKT(pPktSrc));
}

static INT CFG80211_VirtualIF_PacketSend(
	struct sk_buff *skb, PNET_DEV dev_p)
{
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s ---> %d\n", __func__, dev_p->ieee80211_ptr->iftype));

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p))) {
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	/* The device not ready to send packt. */
	wdev = RTMP_OS_NETDEV_GET_WDEV(dev_p);
	ASSERT(wdev);

	if (!wdev)
		return -1;

	os_zero_mem((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);
	return CFG80211_PacketSend(skb, dev_p, rt28xx_packet_xmit);
}

static INT CFG80211_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p,
	IN OUT VOID			*rq_p,
	IN INT					cmd)
{
	RTMP_ADAPTER *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s --->\n", __func__));
	return rt28xx_ioctl(dev_p, rq_p, cmd);
}

static struct wifi_dev_ops gc_wdev_ops  = {
	.tx_pkt_allowed = sta_tx_pkt_allowed,
	.fp_tx_pkt_allowed = sta_tx_pkt_allowed,
	.send_data_pkt = sta_send_data_pkt,
	.send_fp_data_pkt = sta_fp_send_data_pkt,
	.send_mlme_pkt = sta_send_mlme_pkt,
	.tx_pkt_handle = sta_tx_pkt_handle,
	.legacy_tx = sta_legacy_tx,
	.ampdu_tx = sta_ampdu_tx,
	.frag_tx = sta_frag_tx,
	.amsdu_tx = sta_amsdu_tx,
	.mlme_mgmtq_tx = sta_mlme_mgmtq_tx,
	.mlme_dataq_tx = sta_mlme_dataq_tx,
	.ieee_802_11_data_tx = sta_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = sta_ieee_802_3_data_tx,
	.fill_non_offload_tx_blk = sta_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = sta_fill_offload_tx_blk,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = sta_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
};

VOID RTMP_CFG80211_VirtualIF_Init(
	IN VOID		*pAdSrc,
	IN CHAR * pDevName,
	IN UINT32                DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV	new_dev_p;
	APCLI_STRUCT	*pApCliEntry;
	struct wifi_dev *wdev;
	UINT apidx = MAIN_MBSSID;
#ifdef MT_MAC
	INT32 Value;
	UCHAR MacByte = 0;
#endif /* MT_MAC */
	CHAR tr_tb_idx = MAX_LEN_OF_MAC_TABLE + apidx;
	CHAR preIfName[12];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = pDevName[devNameLen - 1] - 48;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct wireless_dev *pWdev;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen - 1);
	pNetDevOps = &netDevHook;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ---> (%s, %s, %d)\n", __func__, pDevName, preIfName, preIfIndex));
	/* init operation functions and flags */
	os_zero_mem(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_VirtualIF_Open;	     /* device opem hook point */
	netDevHook.stop = CFG80211_VirtualIF_Close;	     /* device close hook point */
	netDevHook.xmit = CFG80211_VirtualIF_PacketSend; /* hard transmit hook point */
	netDevHook.ioctl = CFG80211_VirtualIF_Ioctl;	 /* ioctl hook point */
#if WIRELESS_EXT >= 12
	/* netDevHook.iw_handler = (void *)&rt28xx_ap_iw_handler_def; */
#endif /* WIRELESS_EXT >= 12 */
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), preIfName, TRUE);

	if (new_dev_p == NULL) {
		/* allocation fail, exit */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Allocate network device fail (CFG80211)...\n"));
		return;
	}
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));

	new_dev_p->destructor =  free_netdev;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	pNetDevOps->needProtcted = TRUE;
	os_move_mem(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
#ifdef MT_MAC
	/* TODO: shall we make choosing which byte to be selectable??? */
	Value = 0x00000000L;
	RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);/* read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address. */
	Value = Value | (0x2 << 29);/* Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address */
	RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);
	MacByte = Value >> 29;
	pNetDevOps->devAddr[0] |= 0x2;

	switch (MacByte) {
	case 0x1: /* choose bit[23:20]*/
		pNetDevOps->devAddr[2] = (pNetDevOps->devAddr[2] = pNetDevOps->devAddr[2] & 0x0f);
		break;

	case 0x2: /* choose bit[31:28]*/
		pNetDevOps->devAddr[3] = (pNetDevOps->devAddr[3] = pNetDevOps->devAddr[3] & 0x0f);
		break;

	case 0x3: /* choose bit[39:36]*/
		pNetDevOps->devAddr[4] = (pNetDevOps->devAddr[4] = pNetDevOps->devAddr[4] & 0x0f);
		break;

	case 0x4: /* choose bit [47:44]*/
		pNetDevOps->devAddr[5] = (pNetDevOps->devAddr[5] = pNetDevOps->devAddr[5] & 0x0f);
		break;

	default: /* choose bit[15:12]*/
		pNetDevOps->devAddr[1] = (pNetDevOps->devAddr[1] = pNetDevOps->devAddr[1] & 0x0f);
		break;
	}

#else

	/* CFG_TODO */
	/*
	 *	Bit1 of MAC address Byte0 is local administration bit
	 *	and should be set to 1 in extended multiple BSSIDs'
	 *	Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	 */
	if (cap->MBSSIDMode == MBSSID_MODE1)
		pNetDevOps->devAddr[0] += 2; /* NEW BSSID */
	else {
#ifdef P2P_ODD_MAC_ADJUST

		if (pNetDevOps->devAddr[5] & 0x01 == 0x01)
			pNetDevOps->devAddr[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
			pNetDevOps->devAddr[5] += FIRST_MBSSID;
	}

#endif /* MT_MAC */

	switch (DevType) {

	case RT_CMD_80211_IFTYPE_P2P_GO:
		/* Only ForceGO init from here, */
		/* Nego as GO init on AddBeacon Ops.*/
		pNetDevOps->priv_flags = INT_P2P;
		/* The Behivaor in SetBeacon Ops */
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		wdev_init(pAd, wdev, WDEV_TYPE_GO, new_dev_p, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (void *)pAd);
		wdev_ops_register(wdev, WDEV_TYPE_GO, &ap_wdev_ops,
						  cap->wmm_detect_method);
		wdev_attr_update(pAd, wdev);
		wdev->bss_info_argument.OwnMacIdx = wdev->OmacIdx;
		/* BC/MC Handling */
		TRTableInsertMcastEntry(pAd, tr_tb_idx, wdev);
		/* for concurrent purpose */
		wdev->hw_bssid_idx = CFG_GO_BSSID_IDX;
		bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);

		if (wdev_idx_reg(pAd, wdev) < 0) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Assign wdev idx for %s failed, free net device!\n",
					 __func__, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pNetDevOps->devAddr);
		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pNetDevOps->devAddr);
		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
		wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
				BSS_INFO_BASIC_FEATURE |
				BSS_INFO_RF_CH_FEATURE |
				BSS_INFO_SYNC_MODE_FEATURE);
		AsicBssInfoUpdate(pAd, wdev->bss_info_argument);
		break;

	default:
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown CFG80211 I/F Type (%d)\n", DevType));
	}

	/* CFG_TODO : should be move to VIF_CHG */
	if ((DevType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
		(DevType == RT_CMD_80211_IFTYPE_P2P_GO))
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pNetDevOps->devAddr);

	pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
	pWdev->netdev = new_dev_p;
	pWdev->iftype = DevType;
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	COPY_MAC_ADDR(pWdev->address, pNetDevOps->devAddr);
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	/* Record the pNetDevice to Cfg80211VifDevList */
	RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s <---\n", __func__));
}

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN  VOID				 *pAdSrc,
	IN	PNET_DEV			  dev_p,
	IN  UINT32                DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	BOOLEAN isGoOn = FALSE;
	struct wifi_dev *wdev;
	UINT apidx = MAIN_MBSSID;

	if (dev_p) {
		pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = FALSE;
		RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
		RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			if (pAd->flg_apcli_init) {
				wdev = &pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
				/* actually not mcc still need to check this! */

				if (pAd->Mlme.bStartMcc == TRUE) {
					struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("GC remove stop mcc\n"));
					cap->tssi_enable = TRUE; /* let host do tssi */
					Stop_MCC(pAd, 0);
					pAd->Mlme.bStartMcc = FALSE;
				} else
					/* if (pAd->Mlme.bStartScc == TRUE) */
				{
					struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
					UCHAR ht_bw = wlan_config_get_ht_bw(p2p_dev);
					UCHAR cen_ch;

					wlan_operate_set_ht_bw(p2p_dev, ht_bw);
					cen_ch = wlan_operate_get_cen_ch_1(p2p_dev);

					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("GC remove & switch to Infra BW = %d  CentralChannel %d\n",
						ht_bw, cen_ch));
				}

				wdev->channel = 0;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
				OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
				cfg80211_disconnected(dev_p, 0, NULL, 0, GFP_KERNEL);
				os_zero_mem(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, MAC_ADDR_LEN);
				RtmpOSNetDevDetach(dev_p);
				wdev_deinit(pAd, wdev);
				pAd->flg_apcli_init = FALSE;
				wdev->if_dev = NULL;
			} else /* Never Opened When New Netdevice on */
				RtmpOSNetDevDetach(dev_p);

		kfree(dev_p->ieee80211_ptr);
		dev_p->ieee80211_ptr = NULL;
	}
}

VOID RTMP_CFG80211_AllVirtualIF_Remove(
	IN VOID		*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV           pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while ((pDevEntry != NULL) && (pCacheList->size != 0)) {
		RtmpOSNetDevProtect(1);
		RTMP_CFG80211_VirtualIF_Remove(pAd, pDevEntry->net_dev, pDevEntry->net_dev->ieee80211_ptr->iftype);
		RtmpOSNetDevProtect(0);
		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */


#ifdef CFG80211_MULTI_STA
BOOLEAN RTMP_CFG80211_MULTI_STA_ON(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	if (!pNewNetDev)
		return FALSE;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN)
			&& (pNewNetDev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION))
			return TRUE;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return FALSE;
}

VOID RTMP_CFG80211_MutliStaIf_Init(VOID *pAdSrc)
{
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()\n", __func__));
#define INF_CFG80211_MULTI_STA_NAME "muti-sta0"
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	vifInfo.vifType = RT_CMD_80211_IFTYPE_STATION;
	vifInfo.vifNameLen = strlen(INF_CFG80211_MULTI_STA_NAME);
	os_zero_mem(vifInfo.vifName, sizeof(vifInfo.vifName));
	NdisCopyMemory(vifInfo.vifName, INF_CFG80211_MULTI_STA_NAME, vifInfo.vifNameLen);

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" %s() VIF Add error.\n", __func__));
		return;
	}

	cfg80211_ctrl->multi_sta_net_dev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_STATION);
	cfg80211_ctrl->flg_cfg_multi_sta_init = TRUE;
}

VOID RTMP_CFG80211_MutliStaIf_Remove(VOID *pAdSrc)
{
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()\n", __func__));
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (cfg80211_ctrl->multi_sta_net_dev) {
		RtmpOSNetDevProtect(1);
		RTMP_DRIVER_80211_VIF_DEL(pAd, cfg80211_ctrl->multi_sta_net_dev,
								  RT_CMD_80211_IFTYPE_STATION);
		RtmpOSNetDevProtect(0);
		cfg80211_ctrl->flg_cfg_multi_sta_init = FALSE;
	}
}
#endif /* CFG80211_MULTI_STA */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE: 2.6.28 */

