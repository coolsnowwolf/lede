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


#define STA_NO_SECURITY_ON(_p)          (_p->StaCfg.wdev.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)                  (_p->StaCfg.wdev.WepStatus == Ndis802_11WEPEnabled)
#define STA_TKIP_ON(_p)                 (_p->StaCfg.wdev.WepStatus == Ndis802_11TKIPEnable)
#define STA_AES_ON(_p)                  (_p->StaCfg.wdev.WepStatus == Ndis802_11AESEnable)

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg.bTGnWifiTest == TRUE)

#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))

#define STA_EXTRA_SETTING(_pAd)
#ifdef DOT11R_FT_SUPPORT
#undef STA_EXTRA_SETTING
#define STA_EXTRA_SETTING(_pAd) \
{ \
	if ((_pAd)->StaCfg.Dot11RCommInfo.bFtSupport && \
		(_pAd)->MlmeAux.MdIeInfo.Len && \
		(_pAd)->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) \
		(_pAd)->StaCfg.Dot11RCommInfo.bInMobilityDomain = TRUE; \
}
#endif /* DOT11R_FT_SUPPORT */

#define STA_PORT_SECURED(_pAd) \
{ \
	BOOLEAN	Cancelled; \
	(_pAd)->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_SECURED; \
	RTMP_IndicateMediaState(_pAd, NdisMediaStateConnected); \
	NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
	(_pAd)->MacTab.tr_entry[BSSID_WCID].PortSecured = (_pAd)->StaCfg.wdev.PortSecured; \
	(_pAd)->MacTab.Content[BSSID_WCID].PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
	RTMPCancelTimer(&((_pAd)->Mlme.LinkDownTimer), &Cancelled);\
	STA_EXTRA_SETTING(_pAd); \
}


BOOLEAN RTMPCheckChannel(RTMP_ADAPTER *pAd, UCHAR CentralCh, UCHAR Ch);

VOID InitChannelRelatedValue(RTMP_ADAPTER *pAd);

VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN BandWidth,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh);

VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PCIPHER_KEY     pWpaKey);

VOID WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID WpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(RTMP_ADAPTER *pAd);
VOID WpaStaGroupKeySetting(RTMP_ADAPTER *pAd);
VOID WpaSendEapolStart(RTMP_ADAPTER *pAd, UCHAR *pBssid);


NDIS_STATUS	STAHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
INT STASendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
INT STASendPacket_New(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);

INT StaAllowToSendPacket(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	UCHAR *pWcid);

INT StaAllowToSendPacket_new(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pWcid);

INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

INT sta_func_init(RTMP_ADAPTER *pAd);

INT STAInitialize(RTMP_ADAPTER *pAd);

/* AD-HOC Related Function */ 
BOOLEAN Adhoc_AddPeerfromBeacon(RTMP_ADAPTER *pAd, BCN_IE_LIST *bcn_ie_list,
                                NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE);

VOID Adhoc_checkPeerBeaconLost(RTMP_ADAPTER *pAd);
VOID LinkUp_Adhoc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
ULONG MakeIbssBeacon(RTMP_ADAPTER *pAd);
#endif /* __STA_H__ */

