/****************************************************************************
 *
 *   Module Name:
 *   vr_ikans.c
 *
 *   Abstract:
 *   Only for IKANOS Vx160 or Vx180 platform.
 *
 *   Revision History:
 *   Who        When          What
 *   ---------  ----------    ----------------------------------------------
 *   Sample Lin	01-28-2008    Created
 *
***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#define MODULE_IKANOS

#include "rt_config.h"
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <netpro/apprehdr.h>


#ifdef IKANOS_VX_1X0

#define IKANOS_PERAP_ID		7 /* IKANOS Fix Peripheral ID */
#define K0_TO_K1(x)			((unsigned)(x)|0xA0000000) /* kseg0 to kseg1 */
/*#define IKANOS_DEBUG */


extern INT rt28xx_send_packets(struct sk_buff *skb_p, struct net_device *net_dev);
static INT32 IKANOS_WlanDataFramesTx(void *_pAdBuf, struct net_device *pNetDev);
static void IKANOS_WlanPktFromAp(apPreHeader_t *pFrame);


/* --------------------------------- Public -------------------------------- */

/*
 * ========================================================================
 * Routine Description:
 *	Init IKANOS fast path function.
 *
 * Arguments:
 *	pApMac			- the MAC of AP
 *
 * Return Value:
 *	None
 *
 * Note:
 *	If you want to enable RX fast path, you must call the function.
 * ========================================================================
 */
void VR_IKANOS_FP_Init(UINT8 BssNum, UINT8 *pApMac)
{
	UINT32 i;
	UINT8 mac[6];

	memcpy(mac, pApMac, 6);

	/* add all MAC of multiple BSS */
	for (i = 0; i < BssNum; i++) {
		apMacAddrConfig(7, mac, 0xAD);
		mac[5]++;
	}
}


/*
 * ========================================================================
 * Routine Description:
 *	Ikanos LAN --> WLAN transmit fast path function.
 *
 * Arguments:
 *	skb				- the transmitted packet (SKB packet format)
 *	netdev			- our WLAN network device
 *
 * Return Value:
 * ========================================================================
 */
INT32 IKANOS_DataFramesTx(struct sk_buff *pSkb, struct net_device *pNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pNetDev->priv;
	IkanosWlanTxCbFuncP *fp = &IKANOS_WlanDataFramesTx;

	pSkb->apFlowData.txDev = pNetDev;
	pSkb->apFlowData.txApId = IKANOS_PERAP_ID;
	pAd->IkanosTxInfo.netdev = pNetDev;
	pAd->IkanosTxInfo.fp = fp;
	pSkb->apFlowData.txHandle = &(pAd->IkanosTxInfo);
	ap2apFlowProcess(pSkb, pNetDev);
#ifdef IKANOS_DEBUG
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ikanos> tx no fp\n")); /* debug use */
#endif /* IKANOS_DEBUG */
	return rt28xx_send_packets(pSkb, pNetDev);
}


/*
 * ========================================================================
 * Routine Description:
 *	Ikanos WLAN --> LAN transmit fast path function.
 *
 * Arguments:
 *	pAd				- WLAN control block
 *	pRxParam		-
 *	pSkb			- the transmitted packet (SKB packet format)
 *	Length			- packet length
 *
 * Return Value:
 *	None
 *
 * Note:
 *	Because no unsigned long private parameters in apPreHeader_t can be used,
 *	we use a global variable to record pAd.
 *	So we can not use multiple card function in Ikanos platform.
 * ========================================================================
 */
PRTMP_ADAPTER	pIkanosAd;
void IKANOS_DataFrameRx(RTMP_ADAPTER *pAd, struct sk_buff *pSkb)
{
	apPreHeader_t *apBuf;
	void *pRxParam = pSkb->dev;
	UINT32 Length = pSkb->len;
	UCHAR wdev_idx, mbss_idx;

	apBuf = (apPreHeader_t *)(translateMbuf2Apbuf(pSkb, 0));
	apBuf->flags1 = 1 << AP_FLAG1_IS_ETH_BIT;
	wdev_idx = RTMP_GET_PACKET_WDEV(pSkb);
	mbss_idx = pAd->wdev_list[wdev_idx]->func_idx;
	apBuf->specInfoElement = mbss_idx; /* MBSS */
	pIkanosAd = pAd;
	apBuf->flags2 = 0;
	apClassify(IKANOS_PERAP_ID, apBuf, (void *)IKANOS_WlanPktFromAp);
	dev_kfree_skb(pSkb);
}



/* --------------------------------- Private -------------------------------- */

/*
 * ========================================================================
 * Routine Description:
 *	Ikanos LAN --> WLAN transmit fast path function.
 *
 * Arguments:
 *	_pAdBuf			- the transmitted packet (Ikanos packet format)
 *	netdev			- our WLAN network device
 *
 * Return Value:
 * ========================================================================
 */
static INT32 IKANOS_WlanDataFramesTx(void *_pAdBuf, struct net_device *pNetDev)
{
	apPreHeader_t *pApBuf = (apPreHeader_t *)_pAdBuf;
	struct sk_buff *sk = NULL;

	sk = (struct sk_buff *)translateApbuf2Mbuf(pApBuf);

	if (sk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ikanos> translateApbuf2Mbuf returned NULL!\n"));
		return 1;
	}

	sk->apFlowData.flags2 = 0;
	sk->apFlowData.wlanFlags = 0;
	sk->protocol = ETH_P_IP;
	sk->dev = pNetDev;
	sk->priority = 0;
	return rt28xx_send_packets(sk, pNetDev);
}


/*
 * ========================================================================
 * Routine Description:
 *	Get real interface index, used in get_netdev_from_bssid()
 *
 * Arguments:
 *	pAd				-
 *	FromWhichBSSID	-
 *
 * Return Value:
 *	None
 * ========================================================================
 */
static INT32 GetSpecInfoIdxFromBssid(PRTMP_ADAPTER pAd, INT32 FromWhichBSSID)
{
	INT32 IfIdx = MAIN_MBSSID;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
			IfIdx = MAX_MBSSID_NUM(pAd) + MAX_WDS_ENTRY;
		else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS) {
				INT WdsIndex = FromWhichBSSID - MIN_NET_DEVICE_FOR_WDS;

				IfIdx = MAX_MBSSID_NUM(pAd) + WdsIndex;
			} else
#endif /* WDS_SUPPORT */
			{
				IfIdx = FromWhichBSSID;
			}
	}

#endif /* CONFIG_AP_SUPPORT */
	return IfIdx; /* return one of MBSS */
}


/*
 * ========================================================================
 * Routine Description:
 *	Ikanos WLAN --> LAN transmit fast path function.
 *
 * Arguments:
 *	pFrame			- the received packet (Ikanos packet format)
 *
 * Return Value:
 *	None
 *
 * Note:
 *	Ikanos platform supports only 8 VAPs
 * ========================================================================
 */
static void IKANOS_WlanPktFromAp(apPreHeader_t *pFrame)
{
	RTMP_ADAPTER *pAd;
	struct net_device *dev = NULL;
	struct sk_buff *skb;
	INT32 index;
	apPreHeader_t *apBuf = K0_TO_K1(pFrame);

	pAd = pIkanosAd;
	/*index = apBuf->specInfoElement; */
	/*dev = pAd->ApCfg.MBSSID[index].MSSIDDev; */
	index = GetSpecInfoIdxFromBssid(pAd, apBuf->specInfoElement);
	dev = get_netdev_from_bssid(pAd, apBuf->specInfoElement);

	if (dev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ikanos> %s: ERROR null device ***************\n", __func__));
		return;
	}

	skb = (struct sk_buff *)translateApbuf2Mbuf(apBuf);

	if (skb == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ikanos> %s: skb is null *********************\n", __func__))
		return;
	}

	pAd->IkanosRxInfo[index].netdev = dev;
	pAd->IkanosRxInfo[index].fp = &IKANOS_WlanDataFramesTx;
	skb->dev = dev;
	skb->apFlowData.rxApId = IKANOS_PERAP_ID;
	/*skb->apFlowData.txHandle = &(txinforx[index]); */
	skb->apFlowData.rxHandle = &(pAd->IkanosRxInfo[index]);
	skb->protocol = eth_type_trans(skb, skb->dev);
#ifdef IKANOS_DEBUG
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ikanos> rx no fp!\n")); /* debug use */
#endif /* IKANOS_DEBUG */
	netif_rx(skb);
}

#endif /* IKANOS_VX_1X0 */

