/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    sta.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#ifndef __STA_H__
#define __STA_H__


#define STA_NO_SECURITY_ON(_p)          (IS_CIPHER_NONE(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_WEP_ON(_p)                  (IS_CIPHER_WEP(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_TKIP_ON(_p)                 (IS_CIPHER_TKIP(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))
#define STA_AES_ON(_p)                  (IS_CIPHER_CCMP128(_p->StaCfg[0].wdev.SecConfig.PairwiseCipher))

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg[0].bTGnWifiTest == TRUE)

#define CKIP_KP_ON(_p)				((((_p)->StaCfg[0].CkipFlag) & 0x10) && ((_p)->StaCfg[0].bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg[0].CkipFlag) & 0x08) && ((_p)->StaCfg[0].bCkipCmicOn == TRUE))

#define STA_EXTRA_SETTING(_pAd)
#ifdef DOT11R_FT_SUPPORT
#undef STA_EXTRA_SETTING
#define STA_EXTRA_SETTING(_pAd) \
	{ \
		if ((_pAd)->StaCfg[0].Dot11RCommInfo.bFtSupport && \
			(_pAd)->MlmeAux.MdIeInfo.Len && \
			(_pAd)->StaCfg[0].AuthMode == Ndis802_11AuthModeWPA2PSK) \
			(_pAd)->StaCfg[0].Dot11RCommInfo.bInMobilityDomain = TRUE; \
	}
#endif /* DOT11R_FT_SUPPORT */

#define STA_PORT_SECURED(_pAd) \
	{ \
		BOOLEAN	Cancelled; \
		struct wifi_dev *pwdev = &((_pAd)->StaCfg[0].wdev);  \
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(_pAd, pwdev); \
		MAC_TABLE_ENTRY *pMEntry = NULL;    \
		pMEntry = GetAssociatedAPByWdev(_pAd, pwdev);   \
		pwdev->PortSecured = WPA_802_1X_PORT_SECURED; \
		RTMP_IndicateMediaState(_pAd, NdisMediaStateConnected); \
		NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
		(_pAd)->MacTab.tr_entry[pMEntry->wcid].PortSecured = pwdev->PortSecured; \
		pMEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
		NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
		RTMPCancelTimer(&(pStaCfg->LinkDownTimer), &Cancelled);\
		STA_EXTRA_SETTING(_pAd); \
	}


BOOLEAN RTMPCheckChannel(RTMP_ADAPTER *pAd, UCHAR CentralCh, UCHAR Ch, struct wifi_dev *wdev);

VOID InitChannelRelatedValue(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN BandWidth,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh,
	IN struct wifi_dev *wdev);

VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd,
	IN  PSTA_ADMIN_CONFIG pStaCfg,
	IN  PCIPHER_KEY     pWpaKey);

VOID WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID WpaDisassocApAndBlockAssoc(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID WpaStaGroupKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID WpaSendEapolStart(RTMP_ADAPTER *pAd, UCHAR *pBssid, struct wifi_dev *wdev);


INT sta_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
INT sta_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
VOID sta_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID sta_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT sta_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT sta_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT sta_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
VOID sta_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
BOOLEAN sta_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT adhoc_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT sta_inf_open(struct wifi_dev *wdev);
INT sta_inf_close(struct wifi_dev *wdev);
UINT32 bssinfo_sta_feature_decision(struct wifi_dev *wdev, UCHAR wcid, UINT32 *feature);

INT StaAllowToSendPacket_new(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pWcid);

INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

INT sta_func_init(RTMP_ADAPTER *pAd);

INT STAInitialize(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID rtmp_sta_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID RTMPStaCfgRadioCtrlFromEEPROM(RTMP_ADAPTER *pAd, EEPROM_NIC_CONFIG2_STRUC NicConfig2);

VOID MSTA_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);
VOID MSTA_Remove(RTMP_ADAPTER *pAd);
VOID MSTAStop(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID sta_deauth_act(struct wifi_dev *wdev);

#endif /* __STA_H__ */

