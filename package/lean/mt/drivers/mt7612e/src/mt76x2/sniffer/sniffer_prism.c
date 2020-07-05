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
	sniffer_prism.c
*/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"

void send_prism_monitor_packets(IN PNET_DEV pNetDev,
			  IN PNDIS_PACKET pRxPacket,
			  IN VOID *dot11_hdr,
			  IN UCHAR *pData,
			  IN USHORT DataSize,
			  IN UCHAR L2PAD,
			  IN UCHAR PHYMODE,
			  IN UCHAR BW,
			  IN UCHAR ShortGI,
			  IN UCHAR MCS,
			  IN UCHAR AMPDU,
			  IN UCHAR STBC,
			  IN UCHAR RSSI1,
			  IN UCHAR BssMonitorFlag11n,
			  IN UCHAR *pDevName,
			  IN UCHAR Channel,
			  IN UCHAR CentralChannel,
			  IN UINT32 MaxRssi) {
	struct sk_buff *pOSPkt;
	wlan_ng_prism2_header *ph;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	ETHEREAL_RADIO h, *ph_11n33; /* for new 11n sniffer format */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	int rate_index = 0;
	USHORT header_len = 0;
	UCHAR temp_header[40] = {0};
	DOT_11_HDR *pHeader = (DOT_11_HDR *)dot11_hdr;

	MEM_DBG_PKT_FREE_INC(pRxPacket);

	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	if (pHeader->FC.Type == 0x2 /* FC_TYPE_DATA */) {
		DataSize -= LENGTH_802_11;
		if ((pHeader->FC.ToDs == 1) && (pHeader->FC.FrDs == 1))
			header_len = LENGTH_802_11_WITH_ADDR4;
		else
			header_len = LENGTH_802_11;

		/* QOS */
		if (pHeader->FC.SubType & 0x08) {
			header_len += 2;
			/* Data skip QOS contorl field */
			DataSize -= 2;
		}

		/* Order bit: A-Ralink or HTC+ */
		if (pHeader->FC.Order) {
			header_len += 4;
			/* Data skip HTC contorl field */
			DataSize -= 4;
		}

		/* Copy Header */
		if (header_len <= 40)
			NdisMoveMemory(temp_header, pData, header_len);

		/* skip HW padding */
		if (L2PAD)
			pData += (header_len + 2);
		else
			pData += header_len;
	}

	if (DataSize < pOSPkt->len) {
		skb_trim(pOSPkt, DataSize);
	} else {
		skb_put(pOSPkt, (DataSize - pOSPkt->len));
	}

	if ((pData - pOSPkt->data) > 0) {
		skb_put(pOSPkt, (pData - pOSPkt->data));
		skb_pull(pOSPkt, (pData - pOSPkt->data));
	}

	if (skb_headroom(pOSPkt) < (sizeof (wlan_ng_prism2_header) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof (wlan_ng_prism2_header) + header_len), 0, GFP_ATOMIC)) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s : Reallocate header size of sk_buff fail!\n",
				  __FUNCTION__));
			goto err_free_sk_buff;
		}
	}

	if (header_len > 0)
		NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header,
			       header_len);

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	if (BssMonitorFlag11n == 0)
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	{
		ph = (wlan_ng_prism2_header *) skb_push(pOSPkt,
							sizeof(wlan_ng_prism2_header));
		NdisZeroMemory(ph, sizeof(wlan_ng_prism2_header));

		ph->msgcode = DIDmsg_lnxind_wlansniffrm;
		ph->msglen = sizeof (wlan_ng_prism2_header);
		strcpy((PSTRING) ph->devname, (PSTRING) pDevName);

		ph->hosttime.did = DIDmsg_lnxind_wlansniffrm_hosttime;
		ph->hosttime.status = 0;
		ph->hosttime.len = 4;
		ph->hosttime.data = jiffies;

		ph->mactime.did = DIDmsg_lnxind_wlansniffrm_mactime;
		ph->mactime.status = 0;
		ph->mactime.len = 0;
		ph->mactime.data = 0;

		ph->istx.did = DIDmsg_lnxind_wlansniffrm_istx;
		ph->istx.status = 0;
		ph->istx.len = 0;
		ph->istx.data = 0;

		ph->channel.did = DIDmsg_lnxind_wlansniffrm_channel;
		ph->channel.status = 0;
		ph->channel.len = 4;

		ph->channel.data = (u_int32_t)Channel;

		ph->rssi.did = DIDmsg_lnxind_wlansniffrm_rssi;
		ph->rssi.status = 0;
		ph->rssi.len = 4;
		ph->rssi.data = MaxRssi;
		ph->signal.did = DIDmsg_lnxind_wlansniffrm_signal;
		ph->signal.status = 0;
		ph->signal.len = 4;
		ph->signal.data = 0;	/*rssi + noise; */

		ph->noise.did = DIDmsg_lnxind_wlansniffrm_noise;
		ph->noise.status = 0;
		ph->noise.len = 4;
		ph->noise.data = 0;
		RtmpDrvRateGet(NULL, PHYMODE, ShortGI, BW, MCS, newRateGetAntenna(MCS, PHYMODE), &ph->rate.data);
	ph->rate.data /= 1000000;
		ph->rate.did = DIDmsg_lnxind_wlansniffrm_rate;
		ph->rate.status = 0;
		ph->rate.len = 4;

		ph->frmlen.did = DIDmsg_lnxind_wlansniffrm_frmlen;
		ph->frmlen.status = 0;
		ph->frmlen.len = 4;
		ph->frmlen.data = (u_int32_t) DataSize;
	}
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	else {
		ph_11n33 = &h;
		NdisZeroMemory((unsigned char *)ph_11n33,
			       sizeof (ETHEREAL_RADIO));

		/*802.11n fields */
		if (MCS > 15)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_3x3;

		if (PHYMODE == MODE_HTGREENFIELD)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_GF;

		if (BW == 1) {
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW40;
		} else if (Channel < CentralChannel) {
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20U;
		} else if (Channel > CentralChannel) {
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20D;
		} else {
			ph_11n33->Flag_80211n |=
			    (WIRESHARK_11N_FLAG_BW20U |
			     WIRESHARK_11N_FLAG_BW20D);
		}

		if (ShortGI == 1)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_SGI;

		if (AMPDU)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_AMPDU;

		if (STBC)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_STBC;

		ph_11n33->signal_level = (UCHAR) RSSI1;

		/* data_rate is the rate index in the wireshark rate table */
		if (PHYMODE >= MODE_HTMIX) {
			if (MCS == 32) {
				if (ShortGI)
					ph_11n33->data_rate = 16;
				else
					ph_11n33->data_rate = 4;
			} else if (MCS > 15)
				ph_11n33->data_rate =
				    (16 * 4 + ((UCHAR) BW * 16) +
				     ((UCHAR) ShortGI * 32) + ((UCHAR) MCS));
			else
				ph_11n33->data_rate =
				    16 + ((UCHAR) BW * 16) +
				    ((UCHAR) ShortGI * 32) + ((UCHAR) MCS);
		} else if (PHYMODE == MODE_OFDM)
			ph_11n33->data_rate = (UCHAR) (MCS) + 4;
		else
			ph_11n33->data_rate = (UCHAR) (MCS);

		/*channel field */
		ph_11n33->channel = (UCHAR) Channel;

		NdisMoveMemory(skb_put(pOSPkt, sizeof (ETHEREAL_RADIO)),
			       (UCHAR *) ph_11n33, sizeof (ETHEREAL_RADIO));
	}
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = eth_type_trans(pOSPkt, pOSPkt->dev);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx(pOSPkt);
	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

