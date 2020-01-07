/****************************************************************************
 *
 *   Module Name:
 *    UTIL/rt_linux.c
 *
 *    Abstract:
 *	All functions provided from OS module are put here.
 *
 *	Note:
 *	1. Can not use sizeof() for a structure with any parameter included
 *	by any compile option, such as RTMP_ADAPTER.
 *
 *	Because the RTMP_ADAPTER size in the UTIL module is different with
 *	DRIVER/NETIF.
 *
 *	2. Do not use any structure with any parameter included by PCI/USB/RBUS/
 *	AP/STA.
 *
 *	Because the structure size in the UTIL module is different with
 *	DRIVER/NETIF.
 *
 *	3. Do not use any structure defined in DRIVER module, EX: pAd.
 *	So we can do module partition.
 *
 *	Revision History:
 *	Who        When          What
 *	---------  ----------    -------------------------------------------
 *
 ***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"
#include "security/dot11i_wpa.h"
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/oom.h>
#include <linux/netdevice.h>
#include <linux/mm.h>
#include <net/sch_generic.h>
#include "rt_os_net.h"
#ifdef MEM_ALLOC_INFO_SUPPORT
#include "meminfo_list.h"
#endif /* MEM_ALLOC_INFO_SUPPORT */

#ifdef CONFIG_FAST_NAT_SUPPORT
#include <net/ra_nat.h>
#endif /*CONFIG_FAST_NAT_SUPPORT*/

/* TODO */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif


#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 3) > LINUX_VERSION_CODE)
static inline void *netdev_priv(struct net_device *dev)
{
	return dev->priv;
}
#endif
#elif (KERNEL_VERSION(2, 4, 27) > LINUX_VERSION_CODE)
static inline void *netdev_priv(struct net_device *dev)
{
	return dev->priv;
}
#endif

/*
 *	Used for backward compatible with previous linux version which
 *	used "net_device->priv" as device driver structure hooking point
 */
static inline void netdev_priv_set(struct net_device *dev, void *priv)
{
#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 3) > LINUX_VERSION_CODE)
	dev->priv = priv;
#endif
#elif (KERNEL_VERSION(2, 4, 27) > LINUX_VERSION_CODE)
	dev->priv = priv;
#endif
}


int DebugLevel = DBG_LVL_ERROR;

UINT32 DebugCategory = DBG_CAT_EN_ALL_MASK
				& ~(0x1 << DBG_CAT_TX)
				& ~(0x1 << DBG_CAT_PS);

UINT32 DebugSubCategory[DBG_LVL_MAX + 1][32] = {
	{
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_0, for DBG_CAT_INIT */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_1, for DBG_CAT_HW */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_2, for DBG_CAT_FW */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_3, for DBG_CAT_HIF */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_4, for DBG_CAT_FPGA */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_5, for DBG_CAT_TEST */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_6, for DBG_CAT_RA */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_7, for DBG_CAT_AP */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_8, for DBG_CAT_CLIENT */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_9, for DBG_CAT_TX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_10, for DBG_CAT_RX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_11, for DBG_CAT_CFG */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_12, for DBG_CAT_MLME */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_13, for DBG_CAT_PROTO */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_14, for DBG_CAT_SEC */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_15, for DBG_CAT_PS */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_16, for DBG_CAT_POWER */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_17, for DBG_CAT_COEX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_18, for DBG_CAT_P2P */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_19 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_20 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_21 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_22 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_23 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_24 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_25 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_26 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_27 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_28 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_29 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_30, for DBG_CAT_RSV1 */
		DBG_SUBCAT_EN_ALL_MASK			/* bir_31, for DBG_CAT_RSV2 */
	},
	{[0 ... 31] = DBG_SUBCAT_EN_ALL_MASK}, {0}, {0}, {0}, {0}, {0}
};

#ifdef OS_ABL_FUNC_SUPPORT
ULONG RTPktOffsetData = 0, RTPktOffsetLen = 0, RTPktOffsetCB = 0;
#endif /* OS_ABL_FUNC_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
extern int (*ra_classifier_hook_rx)(struct sk_buff *skb, unsigned long cycle);
extern volatile unsigned long classifier_cur_cycle;
#endif /* CONFIG_RA_CLASSIFIER */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef MEM_ALLOC_INFO_SUPPORT
MEM_INFO_LIST MemInfoList;
MEM_INFO_LIST PktInfoList;
#endif /* MEM_ALLOC_INFO_SUPPORT */

#ifdef VENDOR_FEATURE4_SUPPORT
ULONG OS_NumOfMemAlloc = 0, OS_NumOfMemFree = 0;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef VENDOR_FEATURE2_SUPPORT
ULONG OS_NumOfPktAlloc = 0, OS_NumOfPktFree = 0;
#endif /* VENDOR_FEATURE2_SUPPORT */

/*
 * the lock will not be used in TX/RX
 * path so throughput should not be impacted
 */
BOOLEAN FlgIsUtilInit = FALSE;
OS_NDIS_SPIN_LOCK UtilSemLock;

BOOLEAN RTMP_OS_Alloc_RscOnly(VOID *pRscSrc, UINT32 RscLen);
BOOLEAN RTMP_OS_Remove_Rsc(LIST_HEADER *pRscList, VOID *pRscSrc);

/*
 * ========================================================================
 * Routine Description:
 * Initialize something in UTIL module.
 *
 * Arguments:
 *	None
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID RtmpUtilInit(VOID)
{
	if (FlgIsUtilInit == FALSE) {
		OS_NdisAllocateSpinLock(&UtilSemLock);
		FlgIsUtilInit = TRUE;
	}
}

/* timeout -- ms */
static inline VOID __RTMP_SetPeriodicTimer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
static inline VOID __RTMP_OS_Init_Timer(
	IN VOID *pReserved,
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN TIMER_FUNCTION function,
	IN PVOID data)
{
	if (!timer_pending(pTimer)) {
		init_timer(pTimer);
		pTimer->data = (unsigned long)data;
		pTimer->function = function;
	}
}

static inline VOID __RTMP_OS_Add_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

static inline VOID __RTMP_OS_Mod_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

static inline VOID __RTMP_OS_Del_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	OUT BOOLEAN *pCancelled)
{
	if (timer_pending(pTimer))
		*pCancelled = del_timer_sync(pTimer);
	else
		*pCancelled = TRUE;
}

static inline VOID __RTMP_OS_Release_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer)
{
	/* nothing to do */
}


/* Unify all delay routine by using udelay */
VOID RtmpusecDelay(ULONG usec)
{
	ULONG i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}


VOID RtmpOsMsDelay(ULONG msec)
{
	mdelay(msec);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time)
{
	time->u.LowPart = jiffies;
}

void RTMP_GetCurrentSystemTick(ULONG *pNow)
{
	*pNow = jiffies;
}

ULONG RTMPMsecsToJiffies(UINT32 m)
{
	return msecs_to_jiffies(m);
}



NDIS_STATUS os_alloc_mem_suspend(
	IN VOID *pReserved,
	OUT UCHAR * *mem,
	IN ULONG size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_KERNEL);

	if (*mem) {
#ifdef VENDOR_FEATURE4_SUPPORT
		OS_NumOfMemAlloc++;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
		MIListAddHead(&MemInfoList, size, *mem, __builtin_return_address(0));
#endif /* MEM_ALLOC_INFO_SUPPORT */
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

PNDIS_PACKET RtmpOSNetPktAlloc(VOID *dummy, int size)
{
	struct sk_buff *skb;
	/* Add 2 more bytes for ip header alignment */
	DEV_ALLOC_SKB(skb, (size + 2));
	return (PNDIS_PACKET) skb;
}

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(VOID *dummy, ULONG len)
{
	struct sk_buff *pkt;

	DEV_ALLOC_SKB(pkt, len);

	if (pkt == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can't allocate frag rx %ld size packet\n", len));
	}

	return (PNDIS_PACKET) pkt;
}




/*
 * The allocated NDIS PACKET must be freed via RTMPFreeNdisPacket()
 */
NDIS_STATUS RTMPAllocateNdisPacket(
	IN VOID *pReserved,
	OUT PNDIS_PACKET *ppPacket,
	IN UCHAR * pHeader,
	IN UINT HeaderLen,
	IN UCHAR * pData,
	IN UINT DataLen)
{
	struct sk_buff *pPacket;
	/* Add LEN_CCMP_HDR + LEN_CCMP_MIC for PMF */
	DEV_ALLOC_SKB(pPacket, (HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING + LEN_CCMP_HDR + LEN_CCMP_MIC));

	if (pPacket == NULL) {
		*ppPacket = NULL;
#ifdef DEBUG
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (KERN_ERR "RTMPAllocateNdisPacket Fail\n\n"));
#endif
		return NDIS_STATUS_FAILURE;
	}

	/* Clone the frame content and update the length of packet */
	if ((HeaderLen > 0) && (pHeader != NULL))
		os_move_mem(pPacket->data, pHeader, HeaderLen);

	if ((DataLen > 0) && (pData != NULL))
		os_move_mem(pPacket->data + HeaderLen, pData, DataLen);

	skb_put(pPacket, HeaderLen + DataLen);
	*ppPacket = (PNDIS_PACKET)pPacket;
	return NDIS_STATUS_SUCCESS;
}


/*
 * ========================================================================
 * Description:
 * This routine frees a miniport internally allocated NDIS_PACKET and its
 * corresponding NDIS_BUFFER and allocated memory.
 * ========================================================================
 */
VOID RTMPFreeNdisPacket(VOID *pReserved, PNDIS_PACKET pPacket)
{
	if (pPacket) {
		dev_kfree_skb_any(RTPKT_TO_OSPKT(pPacket));
		MEM_DBG_PKT_FREE_INC(pPacket);
		pPacket = NULL;
	}
}


VOID RTMPFreeNdisPacketIRQ(VOID *pReserved, PNDIS_PACKET pPacket)
{
	if (pPacket) {
		dev_kfree_skb_irq(RTPKT_TO_OSPKT(pPacket));
		MEM_DBG_PKT_FREE_INC(pPacket);
		pPacket = NULL;
	}
}


void RTMP_QueryPacketInfo(
	IN PNDIS_PACKET pPacket,
	OUT PACKET_INFO * info,
	OUT UCHAR * *pSrcBufVA,
	OUT UINT * pSrcBufLen)
{
	info->BufferCount = 1;
	info->pFirstBuffer = (PNDIS_BUFFER) GET_OS_PKT_DATAPTR(pPacket);
	info->PhysicalBufferCount = 1;
	info->TotalPacketLength = GET_OS_PKT_LEN(pPacket);
	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket);
}




PNDIS_PACKET ClonePacket(PNET_DEV ndev, PNDIS_PACKET pkt, UCHAR *buf, ULONG sz)
{
	struct sk_buff *pRxPkt, *pClonedPkt;

	ASSERT(pkt);
	ASSERT(sz < 1530);
	pRxPkt = RTPKT_TO_OSPKT(pkt);
	/* clone the packet */
	pClonedPkt = skb_clone(pRxPkt, MEM_ALLOC_FLAG);

	if (pClonedPkt) {
		/* set the correct dataptr and data len */
		MEM_DBG_PKT_ALLOC_INC(pClonedPkt);
		pClonedPkt->dev = pRxPkt->dev;
		pClonedPkt->data = buf;
		pClonedPkt->len = sz;
		SET_OS_PKT_DATATAIL(pClonedPkt, pClonedPkt->len);
	}

	return pClonedPkt;
}


PNDIS_PACKET DuplicatePacket(PNET_DEV pNetDev, PNDIS_PACKET pPacket)
{
	struct sk_buff *skb;
	PNDIS_PACKET pRetPacket = NULL;
	USHORT DataSize;
	UCHAR *pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);

	if (skb) {
		MEM_DBG_PKT_ALLOC_INC(skb);
		skb->dev = pNetDev;
		pRetPacket = OSPKT_TO_RTPKT(skb);
	}

	return pRetPacket;
}


PNDIS_PACKET duplicate_pkt_vlan(
	IN PNET_DEV pNetDev,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN ULONG DataSize,
	IN UCHAR *TPID)
{
	struct sk_buff *skb;
	PNDIS_PACKET pPacket = NULL;
	UINT16 VLAN_Size = 0;
	INT skb_len = HdrLen + DataSize + 2;
#ifdef WIFI_VLAN_SUPPORT

	if (VLAN_VID != 0)
		skb_len += LENGTH_802_1Q;

#endif /* WIFI_VLAN_SUPPORT */
	skb = __dev_alloc_skb(skb_len, MEM_ALLOC_FLAG);

	if (skb != NULL) {
		MEM_DBG_PKT_ALLOC_INC(skb);
		skb_reserve(skb, 2);

		/* copy header (maybe with VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
										  pHeader802_3, HdrLen,
										  GET_OS_PKT_DATATAIL(skb),
										  TPID);
		skb_put(skb, HdrLen + VLAN_Size);
		/* copy data body */
		os_move_mem(GET_OS_PKT_DATATAIL(skb), pData, DataSize);

		skb_put(skb, DataSize);
		skb->dev = pNetDev;
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE		8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(VOID *pReserved, PNDIS_PACKET pPacket)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);

	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE) {
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);
		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(skb);

		if (newskb == NULL) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Extend Tx.MIC for packet failed!, dropping packet!\n"));
			return NULL;
		}

		skb = newskb;
		MEM_DBG_PKT_ALLOC_INC(skb);
	}

	return OSPKT_TO_RTPKT(skb);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Send a L2 frame to upper daemon to trigger state machine
 *
 * Arguments:
 * pAd - pointer to our pAdapter context
 *
 * Return Value:
 *
 * Note:
 *
 * ========================================================================
 */
BOOLEAN RTMPL2FrameTxAction(
	IN VOID *pCtrlBkPtr,
	IN PNET_DEV pNetDev,
	IN RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN UCHAR apidx,
	IN UCHAR *pData,
	IN UINT32 data_len,
	IN UCHAR OpMode)
{
	struct sk_buff *skb = NULL;

	DEV_ALLOC_SKB(skb, (data_len + 2));

	if (!skb) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s : Error! Can't allocate a skb.\n", __func__));
		return FALSE;
	}

	SET_OS_PKT_NETDEV(skb, pNetDev);
	/* 16 byte align the IP header */
	skb_reserve(skb, 2);
	/* Insert the frame content */
	os_move_mem(GET_OS_PKT_DATAPTR(skb), pData, data_len);
	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s doen\n", __func__));
	_announce_802_3_packet(pCtrlBkPtr, skb, OpMode);
	return TRUE;
}


PNDIS_PACKET ExpandPacket(
	IN VOID *pReserved,
	IN PNDIS_PACKET pPacket,
	IN UINT32 ext_head_len,
	IN UINT32 ext_tail_len)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);

	if (skb_cloned(skb) ||
		(skb_headroom(skb) < ext_head_len) ||
		(skb_tailroom(skb) < ext_tail_len)) {
		UINT32 head_len =
			(skb_headroom(skb) < ext_head_len) ? ext_head_len : skb_headroom(skb);
		UINT32 tail_len =
			(skb_tailroom(skb) < ext_tail_len) ? ext_tail_len : skb_tailroom(skb);
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, head_len, tail_len, GFP_ATOMIC);
		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(skb);

		if (newskb == NULL) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Extend Tx buffer for WPI failed!, dropping packet!\n"));
			return NULL;
		}

		skb = newskb;
		MEM_DBG_PKT_ALLOC_INC(skb);
	}

	return OSPKT_TO_RTPKT(skb);
}


VOID RtmpOsPktInit(
	IN PNDIS_PACKET pNetPkt,
	IN PNET_DEV pNetDev,
	IN UCHAR *pData,
	IN USHORT DataSize)
{
	PNDIS_PACKET pRxPkt;

	pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	SET_OS_PKT_NETDEV(pRxPkt, pNetDev);
	SET_OS_PKT_DATAPTR(pRxPkt, pData);
	SET_OS_PKT_LEN(pRxPkt, DataSize);
	SET_OS_PKT_DATATAIL(pRxPkt, DataSize);
}


void wlan_802_11_to_802_3_packet(
	IN PNET_DEV pNetDev,
	IN UCHAR OpMode,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PNDIS_PACKET pRxPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN PUCHAR pHeader802_3,
	IN UCHAR *TPID)
{
	struct sk_buff *pOSPkt;

	ASSERT(pHeader802_3);
	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	pOSPkt->data = pData;
	pOSPkt->len = DataSize;
	SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);

	/* copy 802.3 header */
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(OpMode) {
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		if (VLAN_VID != 0)
			VLAN_Size = LENGTH_802_1Q;

		data_p = skb_push(pOSPkt, LENGTH_802_3 + VLAN_Size);
		VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
							  pHeader802_3, LENGTH_802_3,
							  data_p, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */
}


void hex_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
#ifdef DBG
	hex_dump_with_lvl(str, pSrcBufVA, SrcBufLen, DBG_LVL_TRACE);
#endif /* DBG */
}

void hex_dump_with_lvl(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen, INT dbglvl)
{
#ifdef DBG
	unsigned char *pt;
	int x;

	if (DebugLevel < dbglvl)
		return;

	pt = pSrcBufVA;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, dbglvl,
			 ("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen));

	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, dbglvl,
					 ("0x%04x : ", x));

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, dbglvl, ("%02x ", ((unsigned char)pt[x])));

		if (x % 16 == 15)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, dbglvl, ("\n"));
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, dbglvl, ("\n"));
#endif /* DBG */
}


#ifdef SYSTEM_LOG_SUPPORT
/*
 * ========================================================================
 *
 * Routine Description:
 * Send log message through wireless event
 *
 * Support standard iw_event with IWEVCUSTOM. It is used below.
 *
 * iwreq_data.data.flags is used to store event_flag that is
 * defined by user. iwreq_data.data.length is the length of the
 * event log.
 *
 * The format of the event log is composed of the entry's MAC
 * address and the desired log message (refer to
 * pWirelessEventText).
 *
 * ex: 11:22:33:44:55:66 has associated successfully
 *
 * p.s. The requirement of Wireless Extension is v15 or newer.
 *
 * ========================================================================
 */
VOID RtmpOsSendWirelessEvent(
	IN VOID *pAd,
	IN USHORT Event_flag,
	IN PUCHAR pAddr,
	IN UCHAR wdev_idx,
	IN CHAR Rssi,
	IN RTMP_OS_SEND_WLAN_EVENT pFunc)
{
#if WIRELESS_EXT >= 15
	pFunc(pAd, Event_flag, pAddr, wdev_idx, Rssi);
#else
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s : The Wireless Extension MUST be v15 or newer.\n",
			  __func__));
#endif /* WIRELESS_EXT >= 15 */
}
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID SendSignalToDaemon(
	IN INT sig,
	RTMP_OS_PID pid,
	unsigned long pid_no)
{
}
#endif /* CONFIG_AP_SUPPORT */


/*******************************************************************************
 *	File open/close related functions.
 *******************************************************************************/
RTMP_OS_FD RtmpOSFileOpen(char *pPath, int flag, int mode)
{
	struct file *filePtr;

	if (flag == RTMP_FILE_RDONLY)
		flag = O_RDONLY;
	else if (flag == RTMP_FILE_WRONLY)
		flag = O_WRONLY;
	else if (flag == RTMP_FILE_CREAT)
		flag = O_CREAT;
	else if (flag == RTMP_FILE_TRUNC)
		flag = O_TRUNC;

	filePtr = filp_open(pPath, flag, 0);

	if (IS_ERR(filePtr)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): Error %ld opening %s\n", __func__,
				  -PTR_ERR(filePtr), pPath));
	}

	return (RTMP_OS_FD) filePtr;
}


int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	filp_close(osfd, NULL);
	return 0;
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	osfd->f_pos = offset;
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	/* The object must have a read method */
#if (KERNEL_VERSION(3, 19, 0) > LINUX_VERSION_CODE)
	if (osfd->f_op && osfd->f_op->read) {
		return osfd->f_op->read(osfd, pDataPtr, readLen, &osfd->f_pos);
#else

	if (osfd->f_mode & FMODE_CAN_READ) {
		return kernel_read(osfd, pDataPtr, readLen, &osfd->f_pos);
#endif
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no file read method\n"));
		return -1;
	}
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
	return osfd->f_op->write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#else
	return kernel_write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#endif
}


static inline void __RtmpOSFSInfoChange(OS_FS_INFO *pOSFSInfo, BOOLEAN bSet)
{
	if (bSet) {
		/* Save uid and gid used for filesystem access. */
		/* Set user and group to 0 (root) */
#if (KERNEL_VERSION(2, 6, 29) > LINUX_VERSION_CODE)
		pOSFSInfo->fsuid = current->fsuid;
		pOSFSInfo->fsgid = current->fsgid;
		current->fsuid = current->fsgid = 0;
#else
#ifdef CONFIG_UIDGID_STRICT_TYPE_CHECKS
		kuid_t uid;
		kgid_t gid;

		uid = current_fsuid();
		gid = current_fsgid();
		pOSFSInfo->fsuid = (int)uid.val;
		pOSFSInfo->fsgid = (int)gid.val;
#else
		/* pOSFSInfo->fsuid = (int)(current_fsuid()); */
		/* pOSFSInfo->fsgid = (int)(current_fsgid()); */
#endif
#endif
		pOSFSInfo->fs = get_fs();
		set_fs(KERNEL_DS);
	} else {
		set_fs(pOSFSInfo->fs);
#if (KERNEL_VERSION(2, 6, 29) > LINUX_VERSION_CODE)
		current->fsuid = pOSFSInfo->fsuid;
		current->fsgid = pOSFSInfo->fsgid;
#endif
	}
}


/*******************************************************************************
 *	Task create/management/kill related functions.
 *******************************************************************************/
static inline NDIS_STATUS __RtmpOSTaskKill(OS_TASK *pTask)
{
	int ret = NDIS_STATUS_FAILURE;
#ifdef KTHREAD_SUPPORT

	if (pTask->kthread_task) {
		if (kthread_stop(pTask->kthread_task) == 0) {
			pTask->kthread_task = NULL;
			ret = NDIS_STATUS_SUCCESS;
		} else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s kthread_task %s stop failed\n", __func__,
				  pTask->taskName));
		}
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s null kthread_task %s\n", __func__,
				  pTask->taskName));

#else
	if (CHECK_PID_LEGALITY(pTask->taskPID)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Terminate the task(%s) with pid(%d)!\n",
				  pTask->taskName, GET_PID_NUMBER(pTask->taskPID)));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);

		if (ret) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 (KERN_WARNING
					  "kill task(%s) with pid(%d) failed(retVal=%d)!\n",
					  pTask->taskName, GET_PID_NUMBER(pTask->taskPID),
					  ret));
		} else {
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif
	return ret;
}


static inline INT __RtmpOSTaskNotifyToExit(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	complete_and_exit(&pTask->taskComplete, 0);
#endif
	return 0;
}


static inline void __RtmpOSTaskCustomize(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	daemonize((RTMP_STRING *) &pTask->taskName[0] /*"%s",pAd->net_dev->name */);
	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;
#else
	unsigned long flags;

	daemonize();
	reparent_to_init();
	strcpy(current->comm, &pTask->taskName[0]);
	siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));
	/* Allow interception of SIGKILL only
	  * Don't allow other signals to interrupt the transmission
	 */
#if (KERNEL_VERSION(2, 4, 22) < LINUX_VERSION_CODE)
	spin_lock_irqsave(&current->sigmask_lock, flags);
	flush_signals(current);
	recalc_sigpending(current);
	spin_unlock_irqrestore(&current->sigmask_lock, flags);
#endif
#endif
	RTMP_GET_OS_PID(pTask->taskPID, current->pid);
	/* signal that we've started the thread */
	complete(&pTask->taskComplete);
#endif
}


static inline NDIS_STATUS __RtmpOSTaskAttach(
	IN OS_TASK *pTask,
	IN RTMP_OS_TASK_CALLBACK fn,
	IN ULONG arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
#ifndef KTHREAD_SUPPORT
	pid_t pid_number = -1;
#endif /* KTHREAD_SUPPORT */
#ifdef KTHREAD_SUPPORT
	pTask->task_killed = 0;

	if (pTask->kthread_task) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s non-null kthread_task %s\n", __func__,
				  pTask->taskName));
		status = NDIS_STATUS_FAILURE;
		goto done;
	}

	pTask->kthread_task = kthread_run((cast_fn)fn, (void *)arg,
									  pTask->taskName);

	if (IS_ERR(pTask->kthread_task)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s kthread_run %s err %ld\n", __func__,
				  pTask->taskName, PTR_ERR(pTask->kthread_task)));
		pTask->kthread_task = NULL;
		status = NDIS_STATUS_FAILURE;
		goto done;
	}

#else
	pid_number =
		kernel_thread((cast_fn) fn, (void *)arg, RTMP_OS_MGMT_TASK_FLAGS);

	if (pid_number < 0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Attach task(%s) failed!\n", pTask->taskName));
		status = NDIS_STATUS_FAILURE;
	} else {
		/* Wait for the thread to start */
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}

#endif
done:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s %s end %d\n", __func__, pTask->taskName, status));
	return status;
}


static inline NDIS_STATUS __RtmpOSTaskInit(
	IN OS_TASK *pTask,
	IN RTMP_STRING *pTaskName,
	IN VOID *pPriv,
	IN LIST_HEADER *pSemList)
{
	int len;

	ASSERT(pTask);
#ifndef KTHREAD_SUPPORT
	os_zero_mem((PUCHAR) (pTask), sizeof(OS_TASK));
#endif
	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN - 1) ? (RTMP_OS_TASK_NAME_LEN - 1) : len;
	os_move_mem(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;
#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema), pSemList);
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	init_completion(&pTask->taskComplete);
#endif
#ifdef KTHREAD_SUPPORT
	init_waitqueue_head(&(pTask->kthread_q));
#endif /* KTHREAD_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


BOOLEAN __RtmpOSTaskWait(
	IN VOID *pReserved,
	IN OS_TASK *pTask,
	IN INT32 * pStatus)
{
#ifdef KTHREAD_SUPPORT
	RTMP_WAIT_EVENT_INTERRUPTIBLE((*pStatus), pTask);

	if ((pTask->task_killed == 1) || ((*pStatus) != 0))
		return FALSE;

#else
	RTMP_SEM_EVENT_WAIT(&(pTask->taskSema), (*pStatus));

	/* unlock the device pointers */
	if ((*pStatus) != 0) {
		/*		RTMP_SET_FLAG_(*pFlags, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
		return FALSE;
	}

#endif /* KTHREAD_SUPPORT */
	return TRUE;
}



#if LINUX_VERSION_CODE <= 0x20402	/* Red Hat 7.1 */
struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *))
{
	struct net_device *dev;
	INT alloc_size;
	/* ensure 32-byte alignment of the private area */
	alloc_size = sizeof(*dev) + sizeof_priv + 31;
	dev = kmalloc(alloc_size, GFP_KERNEL);

	if (dev == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("alloc_netdev: Unable to allocate device memory.\n"));
		return NULL;
	}

	memset(dev, 0, alloc_size);

	if (sizeof_priv)
		dev->priv = (void *)(((long)(dev + 1) + 31) & ~31);

	setup(dev);
	strcpy(dev->name, mask);
	return dev;
}
#endif /* LINUX_VERSION_CODE */


static UINT32 RtmpOSWirelessEventTranslate(IN UINT32 eventType)
{
	switch (eventType) {
	case RT_WLAN_EVENT_CUSTOM:
		eventType = IWEVCUSTOM;
		break;

	case RT_WLAN_EVENT_CGIWAP:
		eventType = SIOCGIWAP;
		break;
#if WIRELESS_EXT > 17

	case RT_WLAN_EVENT_ASSOC_REQ_IE:
		eventType = IWEVASSOCREQIE;
		break;
#endif /* WIRELESS_EXT */
#if WIRELESS_EXT >= 14

	case RT_WLAN_EVENT_SCAN:
		eventType = SIOCGIWSCAN;
		break;
#endif /* WIRELESS_EXT */

	case RT_WLAN_EVENT_EXPIRED:
		eventType = IWEVEXPIRED;
		break;

	default:
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Unknown event: 0x%x\n", eventType));
		break;
	}

	return eventType;
}


int RtmpOSWrielessEventSend(
	IN PNET_DEV pNetDev,
	IN UINT32 eventType,
	IN INT flags,
	IN PUCHAR pSrcMac,
	IN PUCHAR pData,
	IN UINT32 dataLen)
{
	union iwreq_data wrqu;
	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);
	memset(&wrqu, 0, sizeof(wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;
	else
		wrqu.data.length = 0;

	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


int RtmpOSWrielessEventSendExt(
	IN PNET_DEV pNetDev,
	IN UINT32 eventType,
	IN INT flags,
	IN PUCHAR pSrcMac,
	IN PUCHAR pData,
	IN UINT32 dataLen,
	IN UINT32 family)
{
	union iwreq_data wrqu;
	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);
	/* translate event type */
	memset(&wrqu, 0, sizeof(wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;

	wrqu.addr.sa_family = family;
	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the network interface is up.
 *
 * Arguments:
 * *pDev                        - Network Interface
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOSNetDevIsUp(VOID *pDev)
{
	struct net_device *pNetDev = (struct net_device *)pDev;

	if ((pNetDev == NULL) || !(pNetDev->flags & IFF_UP))
		return FALSE;

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Assign sys_handle data pointer (pAd) to the priv info structured linked to
 * the OS network interface.
 *
 * Arguments:
 * pDev                 - the os net device data structure
 * pPriv                        - the sys_handle want to assigned
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSetNetDevPriv(VOID *pDev, VOID *pPriv)
{
	struct mt_dev_priv *priv_info = NULL;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->sys_handle = (VOID *)pPriv;
	priv_info->priv_flags = 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Get wifi_dev from the priv info linked to OS network interface data structure.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * sys_handle
 *
 * Note:
 * ========================================================================
 */
VOID *RtmpOsGetNetDevPriv(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->sys_handle;
}


VOID RtmpOsSetNetDevWdev(VOID *net_dev, VOID *wdev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->wifi_dev = wdev;
}


VOID *RtmpOsGetNetDevWdev(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->wifi_dev;
}


/*
 * ========================================================================
 * Routine Description:
 * Get private flags from the network interface.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * pPriv                        - the pointer
 *
 * Note:
 * ========================================================================
 */
USHORT RtmpDevPrivFlagsGet(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->priv_flags;
}


/*
 * ========================================================================
 * Routine Description:
 * Get private flags from the network interface.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * pPriv                        - the pointer
 *
 * Note:
 * ========================================================================
 */
VOID RtmpDevPrivFlagsSet(VOID *pDev, USHORT PrivFlags)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->priv_flags = PrivFlags;
}

UCHAR get_sniffer_mode(VOID *pDev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	return priv_info->sniffer_mode;
}

VOID set_sniffer_mode(VOID *net_dev, UCHAR sniffer_mode)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->sniffer_mode = sniffer_mode;
}

/*
 * ========================================================================
 * Routine Description:
 *	Get network interface name.
 *
 * Arguments:
 *	pDev			- the device
 *
 * Return Value:
 *	the name
 * ========================================================================
 */
unsigned long RtmpOSGetNetDevState(VOID *pDev)
{
	return ((PNET_DEV) pDev)->state;
}



unsigned int RtmpOSGetNetDevFlag(VOID *pDev)
{
	return ((PNET_DEV)pDev)->flags;
}


unsigned int RtmpOSGetNetDevQNum(VOID *pDev)
{
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	return ((PNET_DEV)pDev)->num_tx_queues;
#else
	return 0;
#endif
}


unsigned long RtmpOSGetNetDevQState(VOID *pDev, unsigned int q_idx)
{
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	struct netdev_queue *que = netdev_get_tx_queue((PNET_DEV)pDev, q_idx);

	if (que)
		return que->state;
	else
		return ~0x0;

#else
	return ~0x0;
#endif
}


char *RtmpOsGetNetDevName(VOID *pDev)
{
	return ((PNET_DEV) pDev)->name;
}


UINT32 RtmpOsGetNetIfIndex(IN VOID *pDev)
{
	return ((PNET_DEV) pDev)->ifindex;
}


int RtmpOSNetDevAddrSet(
	IN UCHAR OpMode,
	IN PNET_DEV pNetDev,
	IN PUCHAR pMacAddr,
	IN PUCHAR dev_name)
{
	struct net_device *net_dev = (struct net_device *)pNetDev;
	os_move_mem(net_dev->dev_addr, pMacAddr, 6);
	return 0;
}


/* Assign the network dev name for created Ralink WiFi interface.*/
static int RtmpOSNetDevRequestName(
	IN INT32 MC_RowID,
	IN UINT32 *pIoctlIF,
	IN PNET_DEV dev,
	IN RTMP_STRING *pPrefixStr,
	IN INT devIdx,
	IN BOOLEAN autoSuffix)
{
	PNET_DEV existNetDev;
	RTMP_STRING suffixName[IFNAMSIZ];
	RTMP_STRING desiredName[IFNAMSIZ];
	int ifNameIdx,
		prefixLen,
		slotNameLen;
	int Status;

	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));

	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++) {
		memset(suffixName, 0, IFNAMSIZ);
		memset(desiredName, 0, IFNAMSIZ);
		strncpy(&desiredName[0], pPrefixStr, prefixLen);
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RT_SOC_SUPPORT

		if (MC_RowID > 0)
			sprintf(suffixName, "i%d", ifNameIdx);
		else
#else
		if (MC_RowID >= 0)
			sprintf(suffixName, "%02d_%d", MC_RowID, ifNameIdx);
		else
#endif /* RT_SOC_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */
			sprintf(suffixName, "%d", ifNameIdx);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));

		if (autoSuffix)
			strncat(desiredName, suffixName, slotNameLen);

		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);

		if (existNetDev == NULL)
			break;
		else if (autoSuffix == FALSE && existNetDev) {
			ifNameIdx = 32;	/* Tend to leave loop then return failure */
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Cannot request DevName with string(%s) from OS!\n", pPrefixStr));
		}

		RtmpOSNetDeviceRefPut(existNetDev);
	}

	if (ifNameIdx < 32) {
#ifdef HOSTAPD_SUPPORT
		*pIoctlIF = ifNameIdx;
#endif /*HOSTAPD_SUPPORT */
		strncpy(&dev->name[0], &desiredName[0], sizeof(dev->name));
		Status = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n",
				  pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}

void RtmpOSNetDevClose(PNET_DEV pNetDev)
{
	dev_close(pNetDev);
}

void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
	ASSERT(pNetDev);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	free_netdev(pNetDev);
#else
	kfree(pNetDev);
#endif
#ifdef VENDOR_FEATURE4_SUPPORT
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("OS_NumOfMemAlloc = %ld, OS_NumOfMemFree = %ld\n",
			  OS_NumOfMemAlloc, OS_NumOfMemFree));
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("OS_NumOfPktAlloc = %ld, OS_NumOfPktFree = %ld\n",
			  OS_NumOfPktAlloc, OS_NumOfPktFree));
#endif /* VENDOR_FEATURE2_SUPPORT */
}

INT RtmpOSNetDevAlloc(
	IN PNET_DEV *new_dev_p,
	IN UINT32 privDataSize)
{
	*new_dev_p = NULL;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Allocate a net device with private data size=%d!\n",
			  privDataSize));
#if LINUX_VERSION_CODE <= 0x20402	/* Red Hat 7.1 */
	*new_dev_p = alloc_netdev(privDataSize, "eth%d", ether_setup);
#else
	*new_dev_p = alloc_etherdev(privDataSize);
#endif /* LINUX_VERSION_CODE */

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
INT RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps)
{
	*pNetDevOps = (PVOID) vmalloc(sizeof(struct net_device_ops));

	if (*pNetDevOps) {
		os_zero_mem(*pNetDevOps, sizeof(struct net_device_ops));
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}
#endif


PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, RTMP_STRING *pDevName)
{
	PNET_DEV pTargetNetDev = NULL;
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 26) <= LINUX_VERSION_CODE)
	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);
#else
	ASSERT(pNetDev);
	pTargetNetDev = dev_get_by_name(pNetDev->nd_net, pDevName);
#endif
#else
	pTargetNetDev = dev_get_by_name(pDevName);
#endif /* KERNEL_VERSION(2,6,24) */
#else
	int devNameLen;

	devNameLen = strlen(pDevName);
	ASSERT((devNameLen <= IFNAMSIZ));

	for (pTargetNetDev = dev_base; pTargetNetDev != NULL;
		 pTargetNetDev = pTargetNetDev->next) {
		if (strncmp(pTargetNetDev->name, pDevName, devNameLen) == 0)
			break;
	}

#endif /* KERNEL_VERSION(2,5,0) */
	return pTargetNetDev;
}


void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev)
{
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)

	/*
	 *  every time dev_get_by_name is called, and it has returned a valid struct
	 *  net_device*, dev_put should be called afterwards, because otherwise the
	 *  machine hangs when the device is unregistered (since dev->refcnt > 1).
	 */
	if (pNetDev)
		dev_put(pNetDev);

#endif /* LINUX_VERSION_CODE */
}


INT RtmpOSNetDevDestory(VOID *pReserved, PNET_DEV pNetDev)
{
	/* TODO: Need to fix this */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("WARNING: This function(%s) not implement yet!!!\n",
			  __func__));
	return 0;
}


void RtmpOSNetDevDetach(PNET_DEV pNetDev)
{
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif
	unregister_netdevice(pNetDev);
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	vfree(pNetDevOps);
#endif
}


void RtmpOSNetDevProtect(BOOLEAN lock_it)
{
	if (lock_it)
		rtnl_lock();
	else
		rtnl_unlock();
}

#if (KERNEL_VERSION(2, 6, 18) <= LINUX_VERSION_CODE)
static void RALINK_ET_DrvInfoGet(
	struct net_device *pDev,
	struct ethtool_drvinfo *pInfo)
{
	strncpy(pInfo->driver, "RALINK WLAN", sizeof("RALINK WLAN"));
	sprintf(pInfo->bus_info, "CSR 0x%lx", pDev->base_addr);
}

static struct ethtool_ops RALINK_Ethtool_Ops = {
	.get_drvinfo = RALINK_ET_DrvInfoGet,
};
#endif


int RtmpOSNetDevAttach(
	IN UCHAR OpMode,
	IN PNET_DEV pNetDev,
	IN RTMP_OS_NETDEV_OP_HOOK * pDevOpHook)
{
	int ret,
		rtnl_locked = FALSE;
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RtmpOSNetDevAttach()--->\n"));

	/* If we need hook some callback function to the net device structrue, now do it. */
	if (pDevOpHook) {
#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_CFG80211_REGISTER(pNetDev);
#endif /* RT_CFG80211_SUPPORT */
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
		pNetDevOps->ndo_open = pDevOpHook->open;
		pNetDevOps->ndo_stop = pDevOpHook->stop;
		pNetDevOps->ndo_start_xmit =
			(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDevOps->ndo_do_ioctl = pDevOpHook->ioctl;
#else
		pNetDev->open = pDevOpHook->open;
		pNetDev->stop = pDevOpHook->stop;
		pNetDev->hard_start_xmit =
			(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDev->do_ioctl = pDevOpHook->ioctl;
#endif
#if (KERNEL_VERSION(2, 6, 18) <= LINUX_VERSION_CODE)
		pNetDev->ethtool_ops = &RALINK_Ethtool_Ops;
#endif

		/* if you don't implement get_stats, just leave the callback function as NULL, a dummy
		  *  function will make kernel panic.
		  */
		if (pDevOpHook->get_stats)
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
			pNetDevOps->ndo_get_stats = pDevOpHook->get_stats;

#else
			pNetDev->get_stats = pDevOpHook->get_stats;
#endif
		/* OS specific flags, here we used to indicate if we are virtual interface */
		/*		pNetDev->priv_flags = pDevOpHook->priv_flags; */
		RT_DEV_PRIV_FLAGS_SET(pNetDev, pDevOpHook->priv_flags);
#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
		/*		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats; */
		pNetDev->get_wireless_stats = pDevOpHook->get_wstats;
#endif
#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12

		if (OpMode == OPMODE_AP)
			pDevOpHook->iw_handler = &rt28xx_ap_iw_handler_def;

#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef CONFIG_WIRELESS_EXT

		if (pDevOpHook->iw_handler)
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;

#endif /* CONFIG_WIRELESS_EXT */
		/* copy the net device mac address to the net_device structure. */
		os_move_mem(pNetDev->dev_addr, &pDevOpHook->devAddr[0],
					MAC_ADDR_LEN);
		rtnl_locked = pDevOpHook->needProtcted;
	}

#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	pNetDevOps->ndo_validate_addr = NULL;
	/*pNetDev->netdev_ops = ops; */
#else
	pNetDev->validate_addr = NULL;
#endif
#endif

	if (rtnl_locked)
		ret = register_netdevice(pNetDev);
	else
		ret = register_netdev(pNetDev);

	netif_stop_queue(pNetDev);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---RtmpOSNetDevAttach(), ret=%d\n", ret));

	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevCreate(
	IN INT32 MC_RowID,
	IN UINT32 *pIoctlIF,
	IN INT devType,
	IN INT devNum,
	IN INT privMemSize,
	IN char *pNamePrefix,
	IN BOOLEAN autoSuffix)
{
	struct net_device *pNetDev = NULL;
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	struct net_device_ops *pNetDevOps = NULL;
#endif
	int status;
	/* allocate a new network device */
	status = RtmpOSNetDevAlloc(&pNetDev, privMemSize);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Allocate network device fail (%s)...\n", pNamePrefix));
		return NULL;
	}

#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	status = RtmpOSNetDevOpsAlloc((PVOID) &pNetDevOps);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Allocate net device ops fail!\n"));
		RtmpOSNetDevFree(pNetDev);
		return NULL;
	}
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Allocate net device ops success!\n"));
	pNetDev->netdev_ops = pNetDevOps;

#endif
	/* find a available interface name, max 32 interfaces */
	status = RtmpOSNetDevRequestName(MC_RowID, pIoctlIF, pNetDev, pNamePrefix, devNum, autoSuffix);

	if (status != NDIS_STATUS_SUCCESS) {
		/* error! no any available ra name can be used! */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Assign inf name (%s with suffix 0~32) failed\n", pNamePrefix));
		RtmpOSNetDevFree(pNetDev);
		return NULL;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The name of the new %s interface is %s\n",
			 pNamePrefix, pNetDev->name));

	return pNetDev;
}


UCHAR VLAN_8023_Header_Copy(
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	OUT PUCHAR pData,
	IN UCHAR *TPID)
{
	UINT16 TCI;
	UCHAR VLAN_Size = 0;

	if (VLAN_VID != 0) {
		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;
		/* make up TCI field */
		TCI = (VLAN_VID & 0x0fff) | ((VLAN_Priority & 0x7) << 13);
#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif /* RT_BIG_ENDIAN */
		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);
		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE) = *(UINT16 *) TPID;
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + 2) = TCI;
		/* copy type/len (2B) */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q) =
			*(UINT16 *) &pHeader802_3[LENGTH_802_3 -
									  LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
			memcpy(pData + LENGTH_802_3 + LENGTH_802_1Q, pHeader802_3 + LENGTH_802_3, HdrLen - LENGTH_802_3);
	} else {
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	}

	return VLAN_Size;
}


/*
 * ========================================================================
 * Routine Description:
 *    Allocate memory for adapter control block.
 *
 * Arguments:
 *    pAd					Pointer to our adapter
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS
 *	NDIS_STATUS_FAILURE
 *	NDIS_STATUS_RESOURCES
 * ========================================================================
 */
NDIS_STATUS AdapterBlockAllocateMemory(VOID *handle, VOID **ppAd, UINT32 SizeOfpAd)
{
#ifdef OS_ABL_FUNC_SUPPORT
	/* get offset for sk_buff */
	{
		struct sk_buff *pPkt = NULL;

		pPkt = kmalloc(sizeof(struct sk_buff), GFP_ATOMIC);

		if (pPkt == NULL) {
			*ppAd = NULL;
			return NDIS_STATUS_FAILURE;
		}

		RTPktOffsetData = (ULONG) (&(pPkt->data)) - (ULONG) pPkt;
		RTPktOffsetLen = (ULONG) (&(pPkt->len)) - (ULONG) pPkt;
		RTPktOffsetCB = (ULONG) (pPkt->cb) - (ULONG) pPkt;
		kfree(pPkt);

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("packet> data offset = %lu\n", RTPktOffsetData));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("packet> len offset = %lu\n", RTPktOffsetLen));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("packet> cb offset = %lu\n", RTPktOffsetCB));
	}
#endif /* OS_ABL_FUNC_SUPPORT */
	*ppAd = (PVOID) vmalloc(SizeOfpAd);

	if (*ppAd) {
		os_zero_mem(*ppAd, SizeOfpAd);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}


/* ========================================================================== */

UINT RtmpOsWirelessExtVerGet(VOID)
{
	return WIRELESS_EXT;
}


VOID RtmpDrvAllMacPrint(
	IN VOID *pReserved,
	IN UINT32 *pBufMac,
	IN UINT32 AddrStart,
	IN UINT32 AddrEnd,
	IN UINT32 AddrStep)
{
	struct file *file_w;
	RTMP_STRING *fileName = "MacDump.txt";
	mm_segment_t orig_fs;
	RTMP_STRING *msg;
	UINT32 macAddr = 0, macValue = 0;

	os_alloc_mem(NULL, (UCHAR **)&msg, 1024);

	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);
	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op) {
			file_w->f_pos = 0;
			macAddr = AddrStart;

			while (macAddr <= AddrEnd) {
				/*				RTMP_IO_READ32(pAd, macAddr, &macValue); // sample */
				macValue = *pBufMac++;
				sprintf(msg, "%04x = %08x\n", macAddr, macValue);
				/* write data to file */
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
			if (file_w->f_op->write) {
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
			} else{
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no file write method\n"));
			}
#else
			kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#endif
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s", msg));
				macAddr += AddrStep;
			}

			snprintf(msg, 1024, "\nDump all MAC values to %s\n", fileName);
		}

		filp_close(file_w, NULL);
	}

	set_fs(orig_fs);
	os_free_mem(msg);
}


VOID RtmpDrvAllE2PPrint(
	IN VOID *pReserved,
	IN USHORT *pMacContent,
	IN UINT32 AddrEnd,
	IN UINT32 AddrStep)
{
	struct file *file_w;
	RTMP_STRING *fileName = "EEPROMDump.txt";
	mm_segment_t orig_fs;
	RTMP_STRING *msg;
	USHORT eepAddr = 0;
	USHORT eepValue;

	os_alloc_mem(NULL, (UCHAR **)&msg, 1024);

	if (!msg)
		return;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);
	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op) {
			file_w->f_pos = 0;
			eepAddr = 0x00;

			while (eepAddr <= AddrEnd) {
				eepValue = *pMacContent;
				sprintf(msg, "%08x = %04x\n", eepAddr, eepValue);
				/* write data to file */
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
				if (file_w->f_op->write) {
					file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
				} else{
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no file write method\n"));
				}
#else
				kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#endif
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s", msg));
				eepAddr += AddrStep;
				pMacContent += (AddrStep >> 1);
			}

			snprintf(msg, 1024 ,"\nDump all EEPROM values to %s\n",
					fileName);
		}

		filp_close(file_w, NULL);
	}

	set_fs(orig_fs);
	os_free_mem(msg);
}


VOID RtmpDrvAllRFPrint(
	IN VOID *pReserved,
	IN UCHAR *pBuf,
	IN UINT32 BufLen)
{
	struct file *file_w;
	RTMP_STRING *fileName = "RFDump.txt";
	mm_segment_t orig_fs;

	orig_fs = get_fs();
	set_fs(KERNEL_DS);
	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName));
	} else {
		if (file_w->f_op) {
			file_w->f_pos = 0;
			/* write data to file */
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
			if (file_w->f_op->write) {
				file_w->f_op->write(file_w, pBuf, BufLen, &file_w->f_pos);
			} else{
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no file write method\n"));
			}
#else
			kernel_write(file_w, pBuf, BufLen, &file_w->f_pos);
#endif
		}

		filp_close(file_w, NULL);
	}

	set_fs(orig_fs);
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up the command thread.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pCmdQTask);
#ifdef KTHREAD_SUPPORT
	pTask->kthread_running = TRUE;
	wake_up(&pTask->kthread_q);
#else
	if (CHECK_PID_LEGALITY(pTask->taskPID))
		RTMP_SEM_EVENT_UP(&(pTask->taskSema));
#endif /* KTHREAD_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up USB Mlme thread.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsMlmeUp(IN RTMP_OS_TASK *pMlmeQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pMlmeQTask);
#ifdef KTHREAD_SUPPORT

	if ((pTask != NULL) && (pTask->kthread_task)) {
		pTask->kthread_running = TRUE;
		wake_up(&pTask->kthread_q);
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s invalid pTask 0x%p or kthread_task 0x%p\n",
				  __func__, pTask,
				  (pTask) ? pTask->kthread_task : NULL));
	}

#else

	if (pTask != NULL) {
		if (CHECK_PID_LEGALITY(pTask->taskPID))
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
	}

#endif /* KTHREAD_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the file is error.
 *
 * Arguments:
 * pFile                        - the file
 *
 * Return Value:
 * OK or any error
 *
 * Note:
 * rt_linux.h, not rt_drv.h
 * ========================================================================
 */
INT32 RtmpOsFileIsErr(IN VOID *pFile)
{
	return IS_FILE_OPEN_ERR(pFile);
}

int RtmpOSIRQRelease(
	IN PNET_DEV pNetDev,
	IN UINT32 infType,
	IN PPCI_DEV pci_dev,
	IN BOOLEAN *pHaveMsi)
{
#ifdef RTMP_PCI_SUPPORT

	if (infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_PCIE) {
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
		synchronize_irq(pci_dev->irq);
#endif
		free_irq(pci_dev->irq, ((struct net_device *)pNetDev));
	}

#endif /* RTMP_PCI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT

	if (infType == RTMP_DEV_INF_RBUS) {
		struct net_device *net_dev = (struct net_device *)pNetDev;
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
		synchronize_irq(net_dev->irq);
#endif
		free_irq(net_dev->irq, (net_dev));
	}

#endif /* RTMP_RBUS_SUPPORT */
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Enable or disable wireless event sent.
 *
 * Arguments:
 * pReserved            - Reserved
 * FlgIsWEntSup - TRUE or FALSE
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsWlanEventSet(
	IN VOID *pReserved,
	IN BOOLEAN *pCfgWEnt,
	IN BOOLEAN FlgIsWEntSup)
{
#if WIRELESS_EXT >= 15
	/*	pAd->CommonCfg.bWirelessEvent = FlgIsWEntSup; */
	*pCfgWEnt = FlgIsWEntSup;
#else
	*pCfgWEnt = 0;		/* disable */
#endif
}

/*
 * ========================================================================
 * Routine Description:
 * vmalloc
 *
 * Arguments:
 * Size                 - memory size
 *
 * Return Value:
 * the memory
 *
 * Note:
 * ========================================================================
 */
VOID *RtmpOsVmalloc(ULONG Size)
{
	return vmalloc(Size);
}

/*
 * ========================================================================
 * Routine Description:
 * vfree
 *
 * Arguments:
 * pMem                 - the memory
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsVfree(VOID *pMem)
{
	if (pMem != NULL)
		vfree(pMem);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign protocol to the packet.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktProtocolAssign(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
}


BOOLEAN RtmpOsStatsAlloc(
	IN VOID **ppStats,
	IN VOID **ppIwStats)
{
	os_alloc_mem(NULL, (UCHAR **) ppStats, sizeof(struct net_device_stats));

	if ((*ppStats) == NULL)
		return FALSE;

	os_zero_mem((UCHAR *) *ppStats, sizeof(struct net_device_stats));
#if WIRELESS_EXT >= 12
	os_alloc_mem(NULL, (UCHAR **) ppIwStats, sizeof(struct iw_statistics));

	if ((*ppIwStats) == NULL) {
		os_free_mem(*ppStats);
		return FALSE;
	}

	os_zero_mem((UCHAR *)*ppIwStats, sizeof(struct iw_statistics));
#endif
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Pass the received packet to OS.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktRcvHandle(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
#ifdef CONFIG_CSO_SUPPORT
	struct net_device *pNetDev =  GET_OS_PKT_NETDEV(pNetPkt);
#endif /* CONFIG_CSO_SUPPORT */
#ifdef CONFIG_CSO_SUPPORT

	if (pNetDev->features & NETIF_F_HW_CSUM) {
		if (RTMP_GET_TCP_CHKSUM_FAIL(pNetPkt))
			pRxPkt->ip_summed = CHECKSUM_NONE;
		else
			pRxPkt->ip_summed = CHECKSUM_UNNECESSARY;
	}

#endif /* CONFIG_CSO_SUPPORT */
	netif_rx(pRxPkt);
}


VOID RtmpOsTaskPidInit(RTMP_OS_PID *pPid)
{
	*pPid = THREAD_PID_INIT_VALUE;
}

/*
 * ========================================================================
 * Routine Description:
 * Get the network interface for the packet.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
PNET_DEV RtmpOsPktNetDevGet(VOID *pPkt)
{
	return GET_OS_PKT_NETDEV(pPkt);
}


#ifdef IAPP_SUPPORT
/* Layer 2 Update frame to switch/bridge */
/* For any Layer2 devices, e.g., bridges, switches and other APs, the frame
  * can update their forwarding tables with the correct port to reach the new
  * location of the STA
  */
typedef struct GNU_PACKED _RT_IAPP_L2_UPDATE_FRAME {

	UCHAR DA[ETH_ALEN];	/* broadcast MAC address */
	UCHAR SA[ETH_ALEN];	/* the MAC address of the STA that has just associated or reassociated */
	USHORT Len;		/* 8 octets */
	UCHAR DSAP;		/* null */
	UCHAR SSAP;		/* null */
	UCHAR Control;		/* reference to IEEE Std 802.2 */
	UCHAR XIDInfo[3];	/* reference to IEEE Std 802.2 */
} RT_IAPP_L2_UPDATE_FRAME, *PRT_IAPP_L2_UPDATE_FRAME;


PNDIS_PACKET RtmpOsPktIappMakeUp(
	IN PNET_DEV pNetDev,
	IN UINT8 * pMac)
{
	RT_IAPP_L2_UPDATE_FRAME frame_body;
	INT size = sizeof(RT_IAPP_L2_UPDATE_FRAME);
	PNDIS_PACKET pNetBuf;

	if (pNetDev == NULL)
		return NULL;

	pNetBuf = RtmpOSNetPktAlloc(NULL, size);

	if (!pNetBuf) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error! Can't allocate a skb.\n"));
		return NULL;
	}

	/* init the update frame body */
	os_zero_mem(&frame_body, size);
	memset(frame_body.DA, 0xFF, ETH_ALEN);
	memcpy(frame_body.SA, pMac, ETH_ALEN);
	frame_body.Len = OS_HTONS(ETH_ALEN);
	frame_body.DSAP = 0;
	frame_body.SSAP = 0x01;
	frame_body.Control = 0xAF;
	frame_body.XIDInfo[0] = 0x81;
	frame_body.XIDInfo[1] = 1;
	frame_body.XIDInfo[2] = 1 << 1;
	SET_OS_PKT_NETDEV(pNetBuf, pNetDev);
	skb_reserve(pNetBuf, 2);
	memcpy(skb_put(pNetBuf, size), &frame_body, size);
	return pNetBuf;
}
#endif /* IAPP_SUPPORT */


#ifdef CONFIG_FAST_NAT_SUPPORT
VOID RtmpOsPktNatMagicTag(IN PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_WLAN;
}
#endif /*CONFIG_FAST_NAT_SUPPORT*/

/*
 * ========================================================================
 * Routine Description:
 * Flush a data cache line.
 *
 * Arguments:
 * AddrStart            - the start address
 * Size                 - memory size
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsDCacheFlush(
	IN ULONG AddrStart,
	IN ULONG Size)
{
	RTMP_UTIL_DCACHE_FLUSH(AddrStart, Size);
}




void OS_SPIN_LOCK_IRQSAVE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_lock_irqsave((spinlock_t *)(lock), *flags);
}

void OS_SPIN_UNLOCK_IRQRESTORE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_unlock_irqrestore((spinlock_t *)(lock), *flags);
}

void OS_SPIN_LOCK(NDIS_SPIN_LOCK *lock)
{
	spin_lock((spinlock_t *)(lock));
}

void OS_SPIN_UNLOCK(NDIS_SPIN_LOCK *lock)
{
	spin_unlock((spinlock_t *)(lock));
}

void OS_SPIN_LOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_lock_irq((spinlock_t *)(lock));
}

void OS_SPIN_UNLOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_unlock_irq((spinlock_t *)(lock));
}

int OS_TEST_BIT(int bit, ULONG *flags)
{
	return test_bit(bit, flags);
}

void OS_SET_BIT(int bit, ULONG *flags)
{
	set_bit(bit, flags);
}

void OS_CLEAR_BIT(int bit, ULONG *flags)
{
	clear_bit(bit, flags);
}

void OS_LOAD_CODE_FROM_BIN(unsigned char **image, char *bin_name, void *inf_dev, UINT32 *code_len)
{
	struct device *dev = NULL;
	const struct firmware *fw_entry;
#ifdef RTMP_PCI_SUPPORT
	dev = (struct device *)(&(((struct pci_dev *)(inf_dev))->dev));
#endif
#ifdef RTMP_RBUS_SUPPORT
	dev = (struct device *) (&(((PPCI_DEV)(inf_dev))->dev));
#endif /*RTMP_RBUS_SUPPORT*/

	if (request_firmware(&fw_entry, bin_name, dev) != 0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:fw not available(/lib/firmware/%s)\n", __func__, bin_name));
		*image = NULL;
		return;
	}

	*image = kmalloc(fw_entry->size, GFP_KERNEL);
	memcpy(*image, fw_entry->data, fw_entry->size);
	*code_len = fw_entry->size;
	release_firmware(fw_entry);
}


void os_load_code_from_bin(void *pAd, unsigned char **image, char *bin_name, UINT32 *code_len)
{
	struct device *dev;
	const struct firmware *fw_entry;

	dev = rtmp_get_dev(pAd);

	if (request_firmware(&fw_entry, bin_name, dev) != 0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s:fw not available(/lib/firmware/%s)\n", __func__, bin_name));
		*image = NULL;
		return;
	}

	os_alloc_mem(pAd, image, fw_entry->size);
	memcpy(*image, fw_entry->data, fw_entry->size);
	*code_len = fw_entry->size;
	release_firmware(fw_entry);
}



#ifdef OS_ABL_FUNC_SUPPORT
/*
 * ========================================================================
 * Routine Description:
 * Change/Recover file UID/GID.
 *
 * Arguments:
 * pOSFSInfoOrg - the file
 * bSet                 - Change (TRUE) or Recover (FALSE)
 *
 * Return Value:
 * None
 *
 * Note:
 * rt_linux.h, not rt_drv.h
 * ========================================================================
 */
void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet)
{
	OS_FS_INFO *pOSFSInfo;

	if (bSet == TRUE) {
		os_alloc_mem(NULL, (UCHAR **) &(pOSFSInfoOrg->pContent),
					 sizeof(OS_FS_INFO));

		if (pOSFSInfoOrg->pContent == NULL) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc file info fail!\n", __func__));
			return;
		}

		memset(pOSFSInfoOrg->pContent, 0, sizeof(OS_FS_INFO));
	}

	pOSFSInfo = (OS_FS_INFO *) (pOSFSInfoOrg->pContent);

	if (pOSFSInfo == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pOSFSInfo == NULL!\n", __func__));
		return;
	}

	__RtmpOSFSInfoChange(pOSFSInfo, bSet);

	if (bSet == FALSE) {
		if (pOSFSInfoOrg->pContent != NULL) {
			os_free_mem(pOSFSInfoOrg->pContent);
			pOSFSInfoOrg->pContent = NULL;
		}
	}
}

VOID RtmpOsInitCompletion(RTMP_OS_COMPLETION *pCompletion)
{
	RTMP_OS_Alloc_RscOnly(pCompletion, sizeof(struct completion));
	init_completion((struct completion *)(pCompletion->pContent));
}

VOID RtmpOsExitCompletion(RTMP_OS_COMPLETION *pCompletion)
{
	if (pCompletion->pContent == NULL)
		return FALSE;

	os_free_mem(pCompletion->pContent);
	pCompletion->pContent = NULL;
}

VOID RtmpOsComplete(RTMP_OS_COMPLETION *pCompletion)
{
	if (pCompletion->pContent == NULL)
		return FALSE;

	complete((struct completion *)(pCompletion->pContent));
	return TRUE;
}

ULONG RtmpOsWaitForCompletionTimeout(RTMP_OS_COMPLETION *pCompletion, ULONG Timeout)
{
	return wait_for_completion_timeout((struct completion *)(pCompletion->pContent), Timeout);
}


/*
 * ========================================================================
 * Routine Description:
 * Activate a tasklet.
 *
 * Arguments:
 * pTasklet             - the tasklet
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsTaskletSche(RTMP_NET_TASK_STRUCT *pTasklet)
{
	if (pTasklet->pContent == NULL)
		return FALSE;

#ifdef WORKQUEUE_BH
	schedule_work((struct work_struct *)(pTasklet->pContent));
#else
	tasklet_hi_schedule((OS_NET_TASK_STRUCT *) (pTasklet->pContent));
#endif /* WORKQUEUE_BH */
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Initialize a tasklet.
 *
 * Arguments:
 * pTasklet             - the tasklet
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsTaskletInit(
	RTMP_NET_TASK_STRUCT *pTasklet,
	VOID (*pFunc)(unsigned long data),
	ULONG Data,
	LIST_HEADER *pTaskletList)
{
#ifdef WORKQUEUE_BH

	if (RTMP_OS_Alloc_RscOnly(pTasklet, sizeof(struct work_struct)) == FALSE)
		return FALSE;

	INIT_WORK((struct work_struct *)(pTasklet->pContent), pFunc);
#else

	if (RTMP_OS_Alloc_RscOnly(pTasklet, sizeof(OS_NET_TASK_STRUCT)) == FALSE)
		return FALSE;

	tasklet_init((OS_NET_TASK_STRUCT *) (pTasklet->pContent), pFunc, Data);
#endif /* WORKQUEUE_BH */
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Delete a tasklet.
 *
 * Arguments:
 * pTasklet             - the tasklet
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsTaskletKill(RTMP_NET_TASK_STRUCT *pTasklet)
{
	if (pTasklet->pContent != NULL) {
#ifdef WORKQUEUE_BH
		cancel_work_sync((OS_NET_TASK_STRUCT *) (pTasklet->pContent));
#else
		tasklet_kill((OS_NET_TASK_STRUCT *) (pTasklet->pContent));
#endif /* WORKQUEUE_BH */
		os_free_mem(pTasklet->pContent);
		pTasklet->pContent = NULL;
	}

	return TRUE;
}


VOID RtmpOsTaskletDataAssign(RTMP_NET_TASK_STRUCT *pTasklet, ULONG Data)
{
#ifndef WORKQUEUE_BH

	if (pTasklet->pContent != NULL)
		((OS_NET_TASK_STRUCT *) (pTasklet->pContent))->data = Data;

#endif /* WORKQUEUE_BH */
}


INT32 RtmpOsTaskIsKilled(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return 1;

	return pTask->task_killed;
}


VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return;

#ifdef KTHREAD_SUPPORT
	WAKE_UP(pTask);
#else
	RTMP_SEM_EVENT_UP(&pTask->taskSema);
#endif
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the task is legal.
 *
 * Arguments:
 * pPkt                 - the packet
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsCheckTaskLegality(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (!pTask)
		return FALSE;

#ifdef KTHREAD_SUPPORT

	if (pTask->kthread_task == NULL)
#else
	if (CHECK_PID_LEGALITY(pTask->taskPID))
	else
#endif
		return FALSE;

	return TRUE;
}


/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(NDIS_MINIPORT_TIMER *pTimerOrg, ULONG timeout)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer) {
		if (timer_pending(pTimer))
			return;

		__RTMP_SetPeriodicTimer(pTimer, timeout);
	}
}


/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
	VOID *pReserved,
	NDIS_MINIPORT_TIMER *pTimerOrg,
	TIMER_FUNCTION function,
	PVOID data,
	LIST_HEADER *pTimerList)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	if (RTMP_OS_Alloc_RscOnly(pTimerOrg, sizeof(OS_NDIS_MINIPORT_TIMER)) == FALSE)
		return;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer)
		__RTMP_OS_Init_Timer(pReserved, pTimer, function, data);
}


VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, ULONG timeout)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer) {
		if (timer_pending(pTimer))
			return;

		__RTMP_OS_Add_Timer(pTimer, timeout);
	}
}


VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, ULONG timeout)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer)
		__RTMP_OS_Mod_Timer(pTimer, timeout);
}


VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer)
		__RTMP_OS_Del_Timer(pTimer, pCancelled);
}


VOID RTMP_OS_Release_Timer(NDIS_MINIPORT_TIMER *pTimerOrg)
{
	OS_NDIS_MINIPORT_TIMER *pTimer;

	pTimer = (OS_NDIS_MINIPORT_TIMER *) (pTimerOrg->pContent);

	if (pTimer) {
		__RTMP_OS_Release_Timer(pTimer);
		os_free_mem(pTimer);
		pTimerOrg->pContent = NULL;
	}
}


/*
 * ========================================================================
 * Routine Description:
 * Allocate a OS resource.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 * pRsc                 - the resource
 * RscLen                       - resource length
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RTMP_OS_Alloc_Rsc(
	LIST_HEADER *pRscList,
	VOID *pRscSrc,
	UINT32 RscLen)
{
	OS_RSTRUC *pRsc = (OS_RSTRUC *) pRscSrc;

	if (pRsc->pContent == NULL) {
		/* new entry */
		os_alloc_mem(NULL, (UCHAR **) &(pRsc->pContent), RscLen);

		if (pRsc->pContent == NULL) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: alloc timer fail!\n", __func__));
			return FALSE;
		}

		{
			LIST_RESOURCE_OBJ_ENTRY *pObj;
			/* allocate resource record entry */
			os_alloc_mem(NULL, (UCHAR **) &(pObj),
						 sizeof(LIST_RESOURCE_OBJ_ENTRY));

			if (pObj == NULL) {
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: alloc timer obj fail!\n",
						  __func__));
				os_free_mem(pRsc->pContent);
				pRsc->pContent = NULL;
				return FALSE;
			}

			memset(pRsc->pContent, 0, RscLen);
			pObj->pRscObj = (VOID *) pRsc;
			OS_SEM_LOCK(&UtilSemLock);
			insertTailList(pRscList, (RT_LIST_ENTRY *) pObj);
			OS_SEM_UNLOCK(&UtilSemLock);
		}
	}

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Allocate a OS resource.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 * pRsc                 - the resource
 * RscLen                       - resource length
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RTMP_OS_Alloc_RscOnly(VOID *pRscSrc, UINT32 RscLen)
{
	OS_RSTRUC *pRsc = (OS_RSTRUC *) pRscSrc;

	if (pRsc->pContent == NULL) {
		/* new entry */
		os_alloc_mem(NULL, (UCHAR **) &(pRsc->pContent), RscLen);

		if (pRsc->pContent == NULL) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: alloc timer fail!\n", __func__));
			return FALSE;
		}

		memset(pRsc->pContent, 0, RscLen);
	}

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Remove a OS resource.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 * pRsc                 - the resource
 *
 * Return Value:
 * TRUE or FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RTMP_OS_Remove_Rsc(
	LIST_HEADER *pRscList,
	VOID *pRscSrc)
{
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	OS_RSTRUC *pRscHead, *pRsc, *pRscRev = (OS_RSTRUC *) pRscSrc;

	pRscHead = NULL;
	OS_SEM_LOCK(&UtilSemLock);

	while (TRUE) {
		pObj = (LIST_RESOURCE_OBJ_ENTRY *) removeHeadList(pRscList);

		if (pRscHead == NULL)
			pRscHead = pObj->pRscObj; /* backup first entry */
		else if (((ULONG)pRscHead) == ((ULONG)(pObj->pRscObj)))
			break; /* has searched all entries */

		pRsc = (OS_RSTRUC *) (pObj->pRscObj);

		if ((ULONG)pRsc == (ULONG)pRscRev)
			break; /* find it */

		/* re-insert it */
		insertTailList(pRscList, (RT_LIST_ENTRY *) pObj);
	}

	OS_SEM_UNLOCK(&UtilSemLock);
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Free all timers.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RTMP_OS_Free_Rscs(LIST_HEADER *pRscList)
{
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	OS_RSTRUC *pRsc;

	OS_SEM_LOCK(&UtilSemLock);

	while (TRUE) {
		pObj = (LIST_RESOURCE_OBJ_ENTRY *) removeHeadList(pRscList);

		if (pObj == NULL)
			break;

		pRsc = (OS_RSTRUC *) (pObj->pRscObj);

		if (pRsc->pContent != NULL) {
			/* free the timer memory */
			os_free_mem(pRsc->pContent);
			pRsc->pContent = NULL;
		} else {
			/*
			 * The case is possible because some timers are released during
			 * the driver life time, EX: we will release some timers in
			 * MacTableDeleteEntry().
			 * But we do not recommend the behavior, i.e. not to release
			 * timers in the driver life time; Or we can not cancel the
			 * timer for timer preemption problem.
			 */
		}

		os_free_mem(pObj);	/* free the timer record entry */
	}

	OS_SEM_UNLOCK(&UtilSemLock);
}


/*
 * ========================================================================
 * Routine Description:
 * Allocate a kernel task.
 *
 * Arguments:
 * pTask                        - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOSTaskAlloc(RTMP_OS_TASK *pTask, LIST_HEADER *pTaskList)
{
	if (RTMP_OS_Alloc_RscOnly(pTask, sizeof(OS_TASK)) == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: alloc task fail!\n", __func__));
		return FALSE;	/* allocate fail */
	}

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Free a kernel task.
 *
 * Arguments:
 * pTask                        - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOSTaskFree(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask != NULL) {
		os_free_mem(pTask);
		pTaskOrg->pContent = NULL;
	}
}


/*
 * ========================================================================
 * Routine Description:
 * Kill a kernel task.
 *
 * Arguments:
 * pTaskOrg             - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;
	NDIS_STATUS Status;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask != NULL) {
		Status = __RtmpOSTaskKill(pTask);
		RtmpOSTaskFree(pTaskOrg);
		return Status;
	}

	return NDIS_STATUS_FAILURE;
}


/*
 * ========================================================================
 * Routine Description:
 * Notify kernel the task exit.
 *
 * Arguments:
 * pTaskOrg             - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return 0;

	return __RtmpOSTaskNotifyToExit(pTask);
}


/*
 * ========================================================================
 * Routine Description:
 * Customize the task.
 *
 * Arguments:
 * pTaskOrg             - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOSTaskCustomize(RTMP_OS_TASK *pTaskOrg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask)
		__RtmpOSTaskCustomize(pTask);
}


/*
 * ========================================================================
 * Routine Description:
 * Activate a kernel task.
 *
 * Arguments:
 * pTaskOrg             - the task
 * fn                   - task handler
 * arg                  - task input argument
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
NDIS_STATUS RtmpOSTaskAttach(
	RTMP_OS_TASK *pTaskOrg,
	RTMP_OS_TASK_CALLBACK fn,
	ULONG arg)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return NDIS_STATUS_FAILURE;

	return __RtmpOSTaskAttach(pTask, fn, arg);
}


/*
 * ========================================================================
 * Routine Description:
 * Initialize a kernel task.
 *
 * Arguments:
 * pTaskOrg             - the task
 * pTaskName            - task name
 * pPriv                        - task input argument
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
NDIS_STATUS RtmpOSTaskInit(
	RTMP_OS_TASK *pTaskOrg,
	RTMP_STRING *pTaskName,
	VOID *pPriv,
	LIST_HEADER *pTaskList,
	LIST_HEADER *pSemList)
{
	OS_TASK *pTask;

	if (RtmpOSTaskAlloc(pTaskOrg, pTaskList) == FALSE)
		return NDIS_STATUS_FAILURE;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return NDIS_STATUS_FAILURE;

	return __RtmpOSTaskInit(pTask, pTaskName, pPriv, pSemList);
}


/*
 * ========================================================================
 * Routine Description:
 * Wait for a event in the task.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 * pTaskOrg             - the task
 *
 * Return Value:
 * TRUE
 * FALSE
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOSTaskWait(VOID *pReserved, RTMP_OS_TASK *pTaskOrg, INT32 *pStatus)
{
	OS_TASK *pTask;

	pTask = (OS_TASK *) (pTaskOrg->pContent);

	if (pTask == NULL)
		return FALSE;

	return __RtmpOSTaskWait(pReserved, pTask, pStatus);
}


/*
 * ========================================================================
 * Routine Description:
 * Get private data for the task.
 *
 * Arguments:
 * pTaskOrg             - the task
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID *RtmpOsTaskDataGet(RTMP_OS_TASK *pTaskOrg)
{
	if (pTaskOrg->pContent == NULL)
		return NULL;

	return ((OS_TASK *) (pTaskOrg->pContent))->priv;
}


/*
 * ========================================================================
 * Routine Description:
 * Allocate a lock.
 *
 * Arguments:
 * pLock                        - the lock
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsAllocateLock(NDIS_SPIN_LOCK *pLock, LIST_HEADER *pLockList)
{
	if (RTMP_OS_Alloc_RscOnly(pLock, sizeof(OS_NDIS_SPIN_LOCK)) == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: alloc lock fail!\n", __func__));
		return FALSE;	/* allocate fail */
	}

	OS_NdisAllocateSpinLock(pLock->pContent);
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Free a lock.
 *
 * Arguments:
 * pLockOrg             - the lock
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsFreeSpinLock(NDIS_SPIN_LOCK *pLockOrg)
{
	/* we will free all locks memory in RTMP_OS_FREE_LOCK() */
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_MINIPORT_TIMER *) (pLockOrg->pContent);

	if (pLock != NULL) {
		OS_NdisFreeSpinLock(pLock);
		os_free_mem(pLock);
		pLockOrg->pContent = NULL;
	}
}


/*
 * ========================================================================
 * Routine Description:
 * Spin lock bh.
 *
 * Arguments:
 * pLockOrg             - the lock
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSpinLockBh(NDIS_SPIN_LOCK *pLockOrg)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (pLockOrg->pContent);

	if (pLock != NULL)
		OS_SEM_LOCK(pLock);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}


/*
 * ========================================================================
 * Routine Description:
 * Spin unlock bh.
 *
 * Arguments:
 * pLockOrg             - the lock
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSpinUnLockBh(NDIS_SPIN_LOCK *pLockOrg)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (pLockOrg->pContent);

	if (pLock != NULL)
		OS_SEM_UNLOCK(pLock);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}


/*
 * ========================================================================
 * Routine Description:
 * Interrupt lock.
 *
 * Arguments:
 * pLockOrg             - the lock
 * pIrqFlags            - the lock flags
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsIntLock(NDIS_SPIN_LOCK *pLockOrg, ULONG *pIrqFlags)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (pLockOrg->pContent);

	if (pLock != NULL)
		OS_INT_LOCK(pLock, *pIrqFlags);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}


/*
 * ========================================================================
 * Routine Description:
 * Interrupt unlock.
 *
 * Arguments:
 * pLockOrg             - the lock
 * IrqFlags             - the lock flags
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsIntUnLock(NDIS_SPIN_LOCK *pLockOrg, ULONG IrqFlags)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (pLockOrg->pContent);

	if (pLock != NULL)
		OS_INT_UNLOCK(pLock, IrqFlags);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}

void RtmpOsSpinLockIrqSave(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (lock->pContent);

	if (pLock != NULL)
		spin_lock_irqsave((spinlock_t *)(pLock), *flags);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}

void RtmpOsSpinUnlockIrqRestore(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (lock->pContent);

	if (pLock != NULL)
		spin_unlock_irqrestore((spinlock_t *)(pLock), *flags);
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}

void RtmpOsSpinLockIrq(NDIS_SPIN_LOCK *lock)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (lock->pContent);

	if (pLock != NULL)
		spin_lock_irq((spinlock_t *)(pLock));
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}

void RtmpOsSpinUnlockIrq(NDIS_SPIN_LOCK *lock)
{
	OS_NDIS_SPIN_LOCK *pLock;

	pLock = (OS_NDIS_SPIN_LOCK *) (lock->pContent);

	if (pLock != NULL)
		spin_unlock_irq((spinlock_t *)(pLock));
	else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("lock> warning! the lock was freed!\n"));
}

/*
 * ========================================================================
 * Routine Description:
 * Get MAC address for the network interface.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
unsigned char *RtmpOsNetDevGetPhyAddr(VOID *pDev)
{
	return RTMP_OS_NETDEV_GET_PHYADDR((PNET_DEV) pDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Start network interface TX queue.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsNetQueueStart(PNET_DEV pDev)
{
	RTMP_OS_NETDEV_START_QUEUE(pDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Stop network interface TX queue.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsNetQueueStop(PNET_DEV pDev)
{
	RTMP_OS_NETDEV_STOP_QUEUE(pDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up network interface TX queue.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsNetQueueWake(PNET_DEV pDev)
{
	RTMP_OS_NETDEV_WAKE_QUEUE(pDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign network interface to the packet.
 *
 * Arguments:
 * pPkt                 - the packet
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSetPktNetDev(VOID *pPkt, VOID *pDev)
{
	SET_OS_PKT_NETDEV(pPkt, (PNET_DEV) pDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign network interface type.
 *
 * Arguments:
 * pDev                 - the device
 * Type                 - the type
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSetNetDevType(VOID *pDev, USHORT Type)
{
	RTMP_OS_NETDEV_SET_TYPE((PNET_DEV) pDev, Type);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign network interface type for monitor mode.
 *
 * Arguments:
 * pDev                 - the device
 * Type                 - the type
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSetNetDevTypeMonitor(VOID *pDev)
{
	RTMP_OS_NETDEV_SET_TYPE((PNET_DEV) pDev, ARPHRD_IEEE80211_PRISM);
}


/*
 * ========================================================================
 * Routine Description:
 * Get PID.
 *
 * Arguments:
 * pPkt                 - the packet
 * pDev                 - the device
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsGetPid(IN ULONG *pDst,
				  IN ULONG PID)
{
	RT_GET_OS_PID(*pDst, PID);
}


/*
 * ========================================================================
 * Routine Description:
 * Wait for a moment.
 *
 * Arguments:
 * Time                 - micro second
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsWait(UINT32 Time)
{
	OS_WAIT(Time);
}


/*
 * ========================================================================
 * Routine Description:
 * Check if b is smaller than a.
 *
 * Arguments:
 * Time                 - micro second
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsTimerAfter(ULONG a, ULONG b)
{
	return RTMP_TIME_AFTER(a, b);
}


/*
 * ========================================================================
 * Routine Description:
 * Check if b is not smaller than a.
 *
 * Arguments:
 * Time                 - micro second
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsTimerBefore(ULONG a, ULONG b)
{
	return RTMP_TIME_BEFORE(a, b);
}


/*
 * ========================================================================
 * Routine Description:
 * Get current system time.
 *
 * Arguments:
 * pTime                        - system time (tick)
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsGetSystemUpTime(ULONG *pTime)
{
	NdisGetSystemUpTime(pTime);
}

/*
 * ========================================================================
 * Routine Description:
 * Get OS tick unit.
 *
 * Arguments:
 * pOps                 - Utility table
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsTickUnitGet(VOID)
{
	return HZ;
}


/*
 * ========================================================================
 * Routine Description:
 * ntohs
 *
 * Arguments:
 * Value                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT16 RtmpOsNtohs(UINT16 Value)
{
	return OS_NTOHS(Value);
}


/*
 * ========================================================================
 * Routine Description:
 * htons
 *
 * Arguments:
 * Value                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT16 RtmpOsHtons(UINT16 Value)
{
	return OS_HTONS(Value);
}


/*
 * ========================================================================
 * Routine Description:
 * ntohl
 *
 * Arguments:
 * Value                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsNtohl(UINT32 Value)
{
	return OS_NTOHL(Value);
}

/*
 * ========================================================================
 * Routine Description:
 * htonl
 *
 * Arguments:
 * Value                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsHtonl(UINT32 Value)
{
	return OS_HTONL(Value);
}


/*
 * ========================================================================
 * Routine Description:
 * get_unaligned for 16-bit value.
 *
 * Arguments:
 * pWord                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT16 RtmpOsGetUnaligned(UINT16 *pWord)
{
	return get_unaligned(pWord);
}

/*
 * ========================================================================
 * Routine Description:
 * get_unaligned for 32-bit value.
 *
 * Arguments:
 * pWord                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
UINT32 RtmpOsGetUnaligned32(UINT32 *pWord)
{
	return get_unaligned(pWord);
}

/*
 * ========================================================================
 * Routine Description:
 * get_unaligned for long-bit value.
 *
 * Arguments:
 * pWord                        - the value
 *
 * Return Value:
 * the value
 *
 * Note:
 * ========================================================================
 */
ULONG RtmpOsGetUnalignedlong(ULONG *pWord)
{
	return get_unaligned(pWord);
}


/*
 * ========================================================================
 * Routine Description:
 * Get maximum scan data length.
 *
 * Arguments:
 * None
 *
 * Return Value:
 * length
 *
 * Note:
 * Used in site servey.
 * ========================================================================
 */
ULONG RtmpOsMaxScanDataGet(VOID)
{
	return IW_SCAN_MAX_DATA;
}


/*
 * ========================================================================
 * Routine Description:
 * copy_from_user
 *
 * Arguments:
 * to                           -
 * from                 -
 * n                            - size
 *
 * Return Value:
 * copy size
 *
 * Note:
 * ========================================================================
 */
ULONG RtmpOsCopyFromUser(VOID *to, const void *from, ULONG n)
{
	return copy_from_user(to, from, n);
}


/*
 * ========================================================================
 * Routine Description:
 * copy_to_user
 *
 * Arguments:
 * to                           -
 * from                 -
 * n                            - size
 *
 * Return Value:
 * copy size
 *
 * Note:
 * ========================================================================
 */
ULONG RtmpOsCopyToUser(VOID *to, const void *from, ULONG n)
{
	return copy_to_user(to, from, n);
}


/*
 * ========================================================================
 * Routine Description:
 * Initialize a semaphore.
 *
 * Arguments:
 * pSem                 - the semaphore
 *
 * Return Value:
 * TRUE                 - Successfully
 * FALSE                        - Fail
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsSemaInitLocked(RTMP_OS_SEM *pSem, LIST_HEADER *pSemList)
{
	if (RTMP_OS_Alloc_RscOnly(pSem, sizeof(OS_SEM)) == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc semaphore fail!\n", __func__));
		return FALSE;
	}

	OS_SEM_EVENT_INIT_LOCKED((OS_SEM *) (pSem->pContent));
	return TRUE;
}



/*
 * ========================================================================
 * Routine Description:
 * Initialize a semaphore.
 *
 * Arguments:
 * pSemOrg                      - the semaphore
 *
 * Return Value:
 * TRUE                 - Successfully
 * FALSE                        - Fail
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsSemaInit(RTMP_OS_SEM *pSem, LIST_HEADER *pSemList)
{
	if (RTMP_OS_Alloc_RscOnly(pSem, sizeof(OS_SEM)) == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: alloc semaphore fail!\n", __func__));
		return FALSE;
	}

	OS_SEM_EVENT_INIT((OS_SEM *) (pSem->pContent));
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Destroy a semaphore.
 *
 * Arguments:
 * pSemOrg                      - the semaphore
 *
 * Return Value:
 * TRUE                 - Successfully
 * FALSE                        - Fail
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsSemaDestory(RTMP_OS_SEM *pSemOrg)
{
	OS_SEM *pSem;

	pSem = (OS_SEM *) (pSemOrg->pContent);

	if (pSem != NULL) {
		OS_SEM_EVENT_DESTORY(pSem);
		os_free_mem(pSem);
		pSemOrg->pContent = NULL;
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("sem> warning! double-free sem!\n"));

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Wait a semaphore.
 *
 * Arguments:
 * pSemOrg                      - the semaphore
 *
 * Return Value:
 * 0                            - Successfully
 * Otherwise            - Fail
 *
 * Note:
 * ========================================================================
 */
INT32 RtmpOsSemaWaitInterruptible(RTMP_OS_SEM *pSemOrg)
{
	OS_SEM *pSem;
	INT Status = -1;

	pSem = (OS_SEM *) (pSemOrg->pContent);

	if (pSem != NULL)
		OS_SEM_EVENT_WAIT(pSem, Status);

	return Status;
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up a semaphore.
 *
 * Arguments:
 * pSemOrg                      - the semaphore
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSemaWakeUp(RTMP_OS_SEM *pSemOrg)
{
	OS_SEM *pSem;

	pSem = (OS_SEM *) (pSemOrg->pContent);

	if (pSem != NULL)
		OS_SEM_EVENT_UP(pSem);
}


/*
 * ========================================================================
 * Routine Description:
 * Check if we are in a interrupt.
 *
 * Arguments:
 * None
 *
 * Return Value:
 * 0                            - No
 * Otherwise            - Yes
 *
 * Note:
 * ========================================================================
 */
INT32 RtmpOsIsInInterrupt(VOID)
{
	return in_interrupt();
}


/*
 * ========================================================================
 * Routine Description:
 * Copy the data buffer to the packet frame body.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 * pNetPkt                      - the packet
 * ThisFrameLen - copy length
 * pData                        - the data buffer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktBodyCopy(
	PNET_DEV pNetDev,
	PNDIS_PACKET pNetPkt,
	ULONG ThisFrameLen,
	PUCHAR pData)
{
	memcpy(skb_put(pNetPkt, ThisFrameLen), pData, ThisFrameLen);
	SET_OS_PKT_NETDEV(pNetPkt, pNetDev);
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the packet is cloned.
 *
 * Arguments:
 * pNetPkt                      - the packet
 *
 * Return Value:
 * TRUE                 - Yes
 * Otherwise            - No
 *
 * Note:
 * ========================================================================
 */
INT RtmpOsIsPktCloned(PNDIS_PACKET pNetPkt)
{
	return OS_PKT_CLONED(pNetPkt);
}


/*
 * ========================================================================
 * Routine Description:
 * Duplicate a packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 *
 * Return Value:
 * the new packet
 *
 * Note:
 * ========================================================================
 */
PNDIS_PACKET RtmpOsPktCopy(PNDIS_PACKET pNetPkt)
{
	return skb_copy(RTPKT_TO_OSPKT(pNetPkt), GFP_ATOMIC);
}


/*
 * ========================================================================
 * Routine Description:
 * Clone a packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 *
 * Return Value:
 * the cloned packet
 *
 * Note:
 * ========================================================================
 */
PNDIS_PACKET RtmpOsPktClone(PNDIS_PACKET pNetPkt)
{
	return skb_clone(RTPKT_TO_OSPKT(pNetPkt), MEM_ALLOC_FLAG);
}

/*
 * ========================================================================
 * Routine Description:
 * Assign the data pointer for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * *pData                       - the data buffer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktDataPtrAssign(PNDIS_PACKET pNetPkt, UCHAR *pData)
{
	SET_OS_PKT_DATAPTR(pNetPkt, pData);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign the data length for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * Len                          - the data length
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktLenAssign(PNDIS_PACKET pNetPkt, LONG Len)
{
	SET_OS_PKT_LEN(pNetPkt, Len);
}


/*
 * ========================================================================
 * Routine Description:
 * Adjust the tail pointer for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * removedTagLen        - the size for adjustment
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktTailAdjust(PNDIS_PACKET pNetPkt, UINT removedTagLen)
{
	OS_PKT_TAIL_ADJUST(pNetPkt, removedTagLen);
}


/*
 * ========================================================================
 * Routine Description:
 * Adjust the data pointer for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * Len                          - the size for adjustment
 *
 * Return Value:
 * the new data pointer for the packet
 *
 * Note:
 * ========================================================================
 */
PUCHAR RtmpOsPktTailBufExtend(PNDIS_PACKET pNetPkt, UINT Len)
{
	return OS_PKT_TAIL_BUF_EXTEND(pNetPkt, Len);
}


/*
 * ========================================================================
 * Routine Description:
 * adjust headroom for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * Len                          - the size for adjustment
 *
 * Return Value:
 * the new data pointer for the packet
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktReserve(PNDIS_PACKET pNetPkt, UINT Len)
{
	OS_PKT_RESERVE(pNetPkt, Len);
}


/*
 * ========================================================================
 * Routine Description:
 * Adjust the data pointer for the packet.
 *
 * Arguments:
 * pNetPkt                      - the packet
 * Len                          - the size for adjustment
 *
 * Return Value:
 * the new data pointer for the packet
 *
 * Note:
 * ========================================================================
 */
PUCHAR RtmpOsPktHeadBufExtend(PNDIS_PACKET pNetPkt, UINT Len)
{
	return OS_PKT_HEAD_BUF_EXTEND(pNetPkt, Len);
}


INT32 RtmpThreadPidKill(RTMP_OS_PID PID)
{
	return KILL_THREAD_PID(PID, SIGTERM, 1);
}


long RtmpOsSimpleStrtol(const char *cp, char **endp, unsigned int base)
{
	return simple_strtol(cp, endp, base);
}


BOOLEAN RtmpOsPktOffsetInit(VOID)
{
	struct sk_buff *pPkt = NULL;

	if ((RTPktOffsetData == 0) && (RTPktOffsetLen == 0)
		&& (RTPktOffsetCB == 0)) {
		pPkt = kmalloc(sizeof(struct sk_buff), GFP_ATOMIC);

		if (pPkt == NULL)
			return FALSE;

		RTPktOffsetData = (ULONG) (&(pPkt->data)) - (ULONG) pPkt;
		RTPktOffsetLen = (ULONG) (&(pPkt->len)) - (ULONG) pPkt;
		RTPktOffsetCB = (ULONG) (pPkt->cb) - (ULONG) pPkt;
		kfree(pPkt);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("packet> data offset = %lu\n", RTPktOffsetData));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("packet> len offset = %lu\n", RTPktOffsetLen));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("packet> cb offset = %lu\n", RTPktOffsetCB));
	}

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Initialize the OS atomic_t.
 *
 * Arguments:
 * pAtomic                      - the atomic
 *
 * Return Value:
 * TRUE                 - allocation successfully
 * FALSE                        - allocation fail
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOsAtomicInit(RTMP_OS_ATOMIC *pAtomic, LIST_HEADER *pAtomicList)
{
	if (RTMP_OS_Alloc_RscOnly(pAtomic, sizeof(atomic_t)) == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc atomic fail!\n", __func__));
		return FALSE;	/* allocate fail */
	}

	return TRUE;
}

/*
 * ========================================================================
 * Routine Description:
 * Initialize the OS atomic_t.
 *
 * Arguments:
 * pAtomic                      - the atomic
 *
 * Return Value:
 * TRUE                 - allocation successfully
 * FALSE                        - allocation fail
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsAtomicDestroy(RTMP_OS_ATOMIC *pAtomic)
{
	if (pAtomic->pContent) {
		os_free_mem(pAtomic->pContent);
		pAtomic->pContent = NULL;
	}
}

/*
 * ========================================================================
 * Routine Description:
 * Atomic read a variable.
 *
 * Arguments:
 * pAtomic                      - the atomic
 *
 * Return Value:
 * content
 *
 * Note:
 * ========================================================================
 */
LONG RtmpOsAtomicRead(RTMP_OS_ATOMIC *pAtomicSrc)
{
	if (pAtomicSrc->pContent)
		return atomic_read((atomic_t *) (pAtomicSrc->pContent));
	else
		return 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Atomic dec a variable.
 *
 * Arguments:
 * pAtomic                      - the atomic
 *
 * Return Value:
 * content
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsAtomicDec(RTMP_OS_ATOMIC *pAtomicSrc)
{
	if (pAtomicSrc->pContent)
		atomic_dec((atomic_t *) (pAtomicSrc->pContent));
}


/*
 * ========================================================================
 * Routine Description:
 * Sets a 32-bit variable to the specified value as an atomic operation.
 *
 * Arguments:
 * pAtomic                      - the atomic
 * Value                        - the value to be exchanged
 *
 * Return Value:
 * the initial value of the pAtomicSrc parameter
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsAtomicInterlockedExchange(
	RTMP_OS_ATOMIC *pAtomicSrc,
	LONG Value)
{
	if (pAtomicSrc->pContent)
		InterlockedExchange((atomic_t *) (pAtomicSrc->pContent), Value);
}


/*
 * ========================================================================
 * Routine Description:
 * Initialize the OS utilities.
 *
 * Arguments:
 * pOps                 - Utility table
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsOpsInit(RTMP_OS_ABL_OPS *pOps)
{
	pOps->ra_printk = (RTMP_PRINTK)printk;
	pOps->ra_snprintf = (RTMP_SNPRINTF)snprintf;
}

#else /* OS_ABL_FUNC_SUPPORT */


void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet)
{
	__RtmpOSFSInfoChange(pOSFSInfoOrg, bSet);
}


/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_SetPeriodicTimer(pTimerOrg, timeout);
}


/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
	VOID *pReserved,
	NDIS_MINIPORT_TIMER *pTimerOrg,
	TIMER_FUNCTION function,
	PVOID data,
	LIST_HEADER *pTimerList)
{
	__RTMP_OS_Init_Timer(pReserved, pTimerOrg, function, data);
}


VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Add_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Mod_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled)
{
	__RTMP_OS_Del_Timer(pTimerOrg, pCancelled);
}


VOID RTMP_OS_Release_Timer(NDIS_MINIPORT_TIMER *pTimerOrg)
{
	__RTMP_OS_Release_Timer(pTimerOrg);
}


NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskKill(pTask);
}


INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskNotifyToExit(pTask);
}


void RtmpOSTaskCustomize(RTMP_OS_TASK *pTask)
{
	__RtmpOSTaskCustomize(pTask);
}


NDIS_STATUS RtmpOSTaskAttach(
	RTMP_OS_TASK *pTask,
	RTMP_OS_TASK_CALLBACK fn,
	ULONG arg)
{
	return __RtmpOSTaskAttach(pTask, fn, arg);
}


NDIS_STATUS RtmpOSTaskInit(
	RTMP_OS_TASK *pTask,
	RTMP_STRING *pTaskName,
	VOID *pPriv,
	LIST_HEADER *pTaskList,
	LIST_HEADER *pSemList)
{
	return __RtmpOSTaskInit(pTask, pTaskName, pPriv, pSemList);
}


BOOLEAN RtmpOSTaskWait(VOID *pReserved, RTMP_OS_TASK *pTask, INT32 *pStatus)
{
	return __RtmpOSTaskWait(pReserved, pTask, pStatus);
}


VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTask)
{
#ifdef KTHREAD_SUPPORT
	WAKE_UP(pTask);
#else
	RTMP_SEM_EVENT_UP(&pTask->taskSema);
#endif
}

#endif /* OS_ABL_FUNC_SUPPORT */


/* pAd MUST allow to be NULL */

#ifdef MEM_ALLOC_INFO_SUPPORT
VOID MemInfoListInital(VOID)
{
	MIListInit(&MemInfoList);
	MIListInit(&PktInfoList);
}

UINT32 ShowMemAllocInfo(VOID)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Show General Allocated Memory Info:\n"));
	ShowMIList(&MemInfoList);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Maximum Allocated Memory Size: %u\n", MemInfoList.MaxAlcSize));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Current Allocated Memory Size: %u\n", MemInfoList.CurAlcSize));
	return MemInfoList.EntryNumber;
}

UINT32 ShowPktAllocInfo(VOID)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Show Packet Allocated Memory Info:\n"));
	ShowMIList(&PktInfoList);
	return PktInfoList.EntryNumber;
}
#endif /* MEM_ALLOC_INFO_SUPPORT */

/*Unify Utility APIs*/
INT os_alloc_mem(
	VOID *pAd,
	UCHAR **mem,
	ULONG size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_ATOMIC);

	if (*mem) {
#ifdef VENDOR_FEATURE4_SUPPORT
		OS_NumOfMemAlloc++;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
		MIListAddHead(&MemInfoList, size, *mem, __builtin_return_address(0));
#endif /* MEM_ALLOC_INFO_SUPPORT */
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

VOID os_free_mem(
	PVOID mem)
{
#ifdef MEM_ALLOC_INFO_SUPPORT
	MEM_INFO_LIST_ENTRY *delEntry;

	delEntry = MIListRemove(&MemInfoList, mem);

	if (delEntry == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("the memory has not been allocated\n"));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("mem addr = %p, caller is %pS\n", mem, __builtin_return_address(0)));
		dump_stack();
	} else
#endif /* MEM_ALLOC_INFO_SUPPORT */
	{
		ASSERT(mem);
		kfree(mem);
	}

#ifdef VENDOR_FEATURE4_SUPPORT
	OS_NumOfMemFree++;
#endif /* VENDOR_FEATURE4_SUPPORT */
}

VOID os_zero_mem(
	PVOID ptr,
	ULONG length)
{
	memset(ptr, 0, length);
}

VOID os_move_mem(
	PVOID pDest,
	PVOID pSrc,
	ULONG length)
{
	memmove(pDest, pSrc, length);
}

VOID os_fill_mem(
	PVOID pBuf,
	ULONG Length,
	UCHAR Fill)
{
	memset(pBuf, Fill, Length);
}

INT os_cmp_mem(
	PVOID Destination,
	PVOID Source,
	ULONG Length)
{
	return memcmp(Destination, Source, Length);
}

INT os_equal_mem(
	PVOID pBuf1,
	PVOID pBuf2,
	ULONG length)
{
	return  !memcmp(pBuf1, pBuf2, length);
}


CHAR *os_str_chr(CHAR *str, INT character)
{
	return strchr(str, character);
}

CHAR *os_str_pbrk(CHAR *str1, CHAR  *str2)
{
	return strpbrk(str1, str2);
}

UINT32 os_str_spn(CHAR *str1, CHAR *str2)
{
	return strspn(str1, str2);
}

LONG os_str_tol(const CHAR *str, CHAR **endptr, INT32 base)
{
	return simple_strtol(str, (char **)endptr, base);
}

LONG os_str_toul(const CHAR *str, CHAR **endptr, INT32 base)
{
	return simple_strtoul(str, (char **)endptr, base);
}

CHAR *os_str_str(const CHAR *str1, const CHAR *str2)
{
	return strstr(str1, str2);
}

void os_kref_init(os_kref *ref)
{
	kref_init(ref);
}

void os_kref_get(os_kref *ref)
{
	kref_get(ref);
}

int os_kref_put(os_kref *ref, void (*release)(os_kref *ref))
{
	return kref_put(ref, release);
}

/*test only*/
#ifdef DBG_STARVATION
static inline void block_entry_free(struct starv_log_entry *entry)
{
	struct starv_dbg_block *block = entry->block;
	struct starv_log *ctrl = block->ctrl;

	DlListDel(&entry->list);
	if (entry->log)
		os_free_mem(entry->log);
	os_zero_mem(entry, sizeof(struct starv_log_entry));
	DlListAdd(&ctrl->free, &entry->list);
}

static inline void block_init(struct starv_dbg_block *block)
{
	struct starv_log_entry *entry, *tmp;

	DlListForEachSafe(entry, tmp, &block->log_head, struct starv_log_entry, list) {
		block_entry_free(entry);
	}

	block->avg_dur = 0;
	block->max_dur = 0;
	block->min_dur = 1;
	block->count = 0;
	block->timeout_cnt = 0;
}

static struct starv_log_entry *starv_log_insert(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;
	struct starv_log *ctrl = block->ctrl;
	struct starv_log_entry *entry = NULL;

	NdisAcquireSpinLock(&ctrl->lock);
	entry = DlListFirst(&ctrl->free, struct starv_log_entry, list);
	if (!entry)
		goto end;

	DlListDel(&entry->list);
	entry->duration = starv->msec;
	entry->block = starv->block;
	DlListAddTail(&block->log_head, &entry->list);
end:
	NdisReleaseSpinLock(&ctrl->lock);
	return entry;
}

static void strav_timeout_handle(struct starv_dbg *starv)
{
	struct starv_log_entry *entry;
	struct starv_dbg_block *block = starv->block;

	entry = starv_log_insert(starv);

	if (block->timeout_fn && entry)
		block->timeout_fn(starv, entry);

	block->timeout_cnt++;
}

VOID starv_dbg_init(struct starv_dbg_block *block, struct starv_dbg *starv)
{
	/*initial value*/
	starv->msec = 0;
	starv->start_jiffies = 0;
	starv->end_jiffies = 0;
	starv->block = block;
	return;
}

VOID starv_dbg_get(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;

	starv->start_jiffies = jiffies;

	if (block->entry_fn)
		block->entry_fn(starv);
}

VOID starv_dbg_put(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;

	starv->end_jiffies = jiffies;
	starv->msec = jiffies_to_msecs(abs(starv->end_jiffies - starv->start_jiffies));
	/*update block*/
	block->max_dur = (starv->msec > block->max_dur) ? starv->msec : block->max_dur;
	block->min_dur = (starv->msec < block->min_dur) ? starv->msec : block->min_dur;
	block->avg_dur = (starv->msec + block->avg_dur * block->count) / (block->count + 1);
	block->count++;
	/*check timeout*/
	if (starv->msec > block->timeout)
		strav_timeout_handle(starv);

	if (block->leave_fn)
		block->leave_fn(starv);
}

VOID starv_log_dump(struct starv_log *ctrl)
{
	struct starv_log_entry *entry, *tmp;
	struct starv_dbg_block *block, *btmp;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===========================================================================\n"));

	NdisAcquireSpinLock(&ctrl->lock);
	DlListForEachSafe(block, btmp, &ctrl->block_head, struct starv_dbg_block, list) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[block %s]\t min: %d, max: %d, avg: %d, timeout cnt: %d, total cnt: %d\n",
			block->name,
			block->min_dur,
			block->max_dur,
			block->avg_dur,
			block->timeout_cnt,
			block->count));
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("===========================================================================\n"));

		DlListForEachSafe(entry, tmp, &block->log_head, struct starv_log_entry, list) {
			if (block->log_fn)
				block->log_fn(entry);
		}
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("===========================================================================\n"));
		block_init(block);
	}

	NdisReleaseSpinLock(&ctrl->lock);
}

void starv_timeout_log_basic(struct starv_log_entry *entry)
{
	struct starv_log_basic *log = entry->log;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s] id: %d, qsize: %d, duration: %d ms\n",
				entry->block->name, log->id, log->qsize, entry->duration));
}

INT register_starv_block(struct starv_dbg_block *block)
{
	struct starv_log *ctrl = block->ctrl;

	NdisAcquireSpinLock(&ctrl->lock);
	DlListInit(&block->log_head);
	block_init(block);
	DlListAddTail(&ctrl->block_head, &block->list);
	NdisReleaseSpinLock(&ctrl->lock);
	return 0;
}

VOID unregister_starv_block(struct starv_dbg_block *block)
{
	struct starv_log *ctrl = block->ctrl;

	NdisAcquireSpinLock(&ctrl->lock);
	DlListDel(&block->list);
	block_init(block);
	NdisReleaseSpinLock(&ctrl->lock);
}

INT starv_log_init(struct starv_log *ctrl)
{
	INT i;
	struct starv_log_entry *entry = NULL;

	os_zero_mem(ctrl, sizeof(struct starv_log));
	NdisAllocateSpinLock(NULL, &ctrl->lock);
	DlListInit(&ctrl->free);
	DlListInit(&ctrl->block_head);
	/*add to free pool*/
	for (i = 0; i < STARV_ENTRY_SIZE; i++) {
		entry = &ctrl->pool[i];
		DlListAddTail(&ctrl->free, &entry->list);
	}
	return 0;
}

VOID starv_log_exit(struct starv_log *ctrl)
{
	starv_log_dump(ctrl);
	DlListInit(&ctrl->free);
	DlListInit(&ctrl->block_head);
	NdisFreeSpinLock(&ctrl->lock);
	os_zero_mem(&ctrl, sizeof(struct starv_log));
}

#endif /*DBG_STARVATION*/


#ifdef CONFIG_DBG_QDISC
/*
*
*/
void os_system_tx_queue_dump(PNET_DEV dev)
{
	struct netdev_queue *txq;
	struct Qdisc *q;
	struct sk_buff *skb;
	int i=0,j=0,k=0;
	struct page *p;
	size_t page_size;
	int len;

	for (i = 0; i < dev->num_tx_queues; i++) {
		txq = netdev_get_tx_queue(dev, i);
		q = rcu_dereference(txq->qdisc);
		len = qdisc_qlen(q);
		printk("%s(): devname:%s, q=%p, qlen=%d, page size:%lu\n", __func__,dev->name, q, len, PAGE_SIZE);
		skb = q->ops->peek(q);
		/* If we run with a TX queue, check if the queue is too long*/
		for (j=0; j < len; j++) {
			printk("%s(): skb=%p,len:%d, skb->head: %p,%p , dev:%s, \n",
				__func__, skb, skb->len, skb->data, skb->data, skb->dev->name);
			if (skb->head) {
				p = virt_to_head_page(skb->head);
				page_size = PAGE_SIZE << compound_order(p);
				printk("%s(): index:%d, page:%p ,rfcnt:%d, page size:%lu, order:%u, dtor:%d\n", __func__, k++, p,
					atomic_read(&p->_count), page_size , compound_order(p), p->compound_dtor);
			}
			skb = skb->next;
		}
	}
}
#endif /*CONFIG_DBG_QDISC*/
