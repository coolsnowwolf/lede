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
	sniffer_radiotap.c
*/
#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"
#include "rt_config.h"

#define ETH_P_ECONET 0x0018
#define ETH_P_80211_RAW (ETH_P_ECONET + 1)

#ifdef SNIFFER_MT7615

#ifdef SNIFFER_MT7615_RMAC_INC
#define MT7615_RMAC_LENGTH          68
#else
#define MT7615_RMAC_LENGTH          0
#endif

#define IEEE80211_RADIOTAP_MT7615_RMAC 22

UINT32 MT7615_CCK_Rate[] = {2, 4, 11, 22, 0, 4, 11, 22};
UINT32 MT7615_OFDM_Rate[] = {96, 48, 24, 12, 108, 72, 36, 18};


/* For reference number of AMPDU Status */
static UINT32 ampdu_refno;

void send_radiotap_mt7615_monitor_packets(
	PNET_DEV pNetDev,
	UCHAR *rmac_info,
	UINT32 rxv2_cyc1,
	PNDIS_PACKET pRxPacket,
	UCHAR *pData,
	USHORT DataSize,
	UCHAR *pDevName,
	CHAR MaxRssi,
	UINT32 UP_value)
{
	struct sk_buff *pOSPkt;
	UCHAR temp_header[40] = {0};
	struct mtk_radiotap_header *mtk_rt_hdr;
	UINT32 varlen = 0, padding_len = 0;
	UINT64 tmp64;
	/* UINT32 tmp32; */
	UINT16 tmp16;
	UCHAR *pos;
#ifdef SNIFFER_MT7615_RMAC_INC
	UCHAR *rmac_info_buffer;
#endif
	MT7615_RMAC_STRUCT *rmac_str_info;
	UINT32 MacHdrLen = 0;
	UINT32 HdrOffset = 0;
	UINT32 TxMode = 0;
	UINT32 NonAmpduFrm = 0;
	UINT32 NonAmpduSfrm = 0;
	UINT64 Timestamp = 0;
	UINT32 TxRate = 0;
	UINT32 FragFrm = 0;
	UINT32 FcsErr = 0;
	UINT32 HtShortGi = 0;
	UINT32 ChFreq = 0;
	UINT32 GroupId = 0;
	UINT32 VHTA1_B21_B10 = 0;
	UINT32 HtAdCode = 0;
	UINT32 VHTA2_B8_B3 = 0;
	UINT32 FrMode = 0;
	UINT32 VHT_A2 = 0;
	UINT32 VHTA1_B22 = 0;
	UINT32 HtStbc = 0;
	UINT32 RxVSeq = 0;
	UINT32 HtExtltf = 0;
	UINT32 PayloadFmt = 0;
	rmac_str_info = (MT7615_RMAC_STRUCT *)rmac_info;
	MEM_DBG_PKT_FREE_INC(pRxPacket);
	MacHdrLen = rmac_str_info->RxRMACBase.RxD1.MacHdrLen;
	HdrOffset = rmac_str_info->RxRMACBase.RxD1.HdrOffset;
	TxMode = rmac_str_info->RxRMACGrp3.rxd_14.TxMode;
	NonAmpduFrm = rmac_str_info->RxRMACBase.RxD2.NonAmpduFrm;
	NonAmpduSfrm = rmac_str_info->RxRMACBase.RxD2.NonAmpduSfrm;
	Timestamp = rmac_str_info->RxRMACGrp2.rxd_12.Timestamp;
	TxRate = rmac_str_info->RxRMACGrp3.rxd_14.TxRate;
	FragFrm = rmac_str_info->RxRMACBase.RxD2.FragFrm;
	FcsErr = rmac_str_info->RxRMACBase.RxD2.FcsErr;
	HtShortGi = rmac_str_info->RxRMACGrp3.rxd_14.HtShortGi;
	ChFreq = rmac_str_info->RxRMACBase.RxD1.ChFreq;
	GroupId = rmac_str_info->RxRMACGrp3.rxd_15.GroupId;
	VHTA1_B21_B10 = rmac_str_info->RxRMACGrp3.rxd_16.VHTA1_B21_B10;
	HtAdCode = rmac_str_info->RxRMACGrp3.rxd_14.HtAdCode;
	VHTA2_B8_B3 = rmac_str_info->RxRMACGrp3.rxd_14.VHTA2_B8_B3;
	FrMode = rmac_str_info->RxRMACGrp3.rxd_14.FrMode;
	VHT_A2 = rmac_str_info->RxRMACGrp3.rxd_20.VHT_A2;
	VHTA1_B22 = rmac_str_info->RxRMACGrp3.rxd_14.VHTA1_B22;
	HtStbc = rmac_str_info->RxRMACGrp3.rxd_14.HtStbc;
	RxVSeq = rmac_str_info->RxRMACBase.RxD3.RxVSeq;
	HtExtltf = rmac_str_info->RxRMACGrp3.rxd_14.HtExtltf;
	PayloadFmt = rmac_str_info->RxRMACBase.RxD1.PayloadFmt;
#ifdef SNIFFER_MT7615_RMAC_INC
	os_alloc_mem(NULL, (UCHAR **)&rmac_info_buffer, MT7615_RMAC_LENGTH);

	if (rmac_info_buffer == NULL)
		goto err_free_sk_buff;

	NdisZeroMemory(rmac_info_buffer, MT7615_RMAC_LENGTH);
	memcpy(rmac_info_buffer, rmac_info, MT7615_RMAC_LENGTH - 4);
	memcpy(rmac_info_buffer + MT7615_RMAC_LENGTH - 4, &rxv2_cyc1, 4);
	/* Updating User Position in RMAC (rxv2_cyc1) */
	rmac_info_buffer[67] &= 0xfc;
	rmac_info_buffer[67] |= (UINT8) UP_value;
#endif
	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	DataSize -= MacHdrLen;

	/* Copy Header */
	if (MacHdrLen <= 40)
		NdisMoveMemory(temp_header, pData, MacHdrLen);

	/* skip HW padding */
	if (HdrOffset && PayloadFmt)
		pData += (MacHdrLen + 2);
	else
		pData += MacHdrLen;

	if (DataSize < pOSPkt->len)
		skb_trim(pOSPkt, DataSize);
	else
		skb_put(pOSPkt, (DataSize - pOSPkt->len));

	if ((pData - pOSPkt->data) > 0) {
		skb_put(pOSPkt, (pData - pOSPkt->data));
		skb_pull(pOSPkt, (pData - pOSPkt->data));
	}

	if (skb_headroom(pOSPkt) < (sizeof(*mtk_rt_hdr) + MacHdrLen)) {
		if (pskb_expand_head(pOSPkt, (sizeof(*mtk_rt_hdr) + MacHdrLen), 0, GFP_ATOMIC)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s : Reallocate header size of sk_buff fail!\n",
					  __func__));
			goto err_free_sk_buff;
		}
	}

	if (MacHdrLen > 0)
		NdisMoveMemory(skb_push(pOSPkt, MacHdrLen), temp_header, MacHdrLen);

	/* TSFT */
	padding_len = ((varlen % 8) == 0) ? 0 : (8 - (varlen % 8));
	varlen += (8 + padding_len);
	/* Flags */
	varlen += 1;

	/* Rate */
	if (TxMode < MODE_HTMIX)
		varlen += 1;

	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	varlen += (2 + padding_len);
	/* channel flags */
	varlen += 2;
	/* dBm ANT Signal */
	varlen += 1;

	/* MCS */
	if ((TxMode == MODE_HTMIX) || (TxMode == MODE_HTGREENFIELD)) {
		/* known */
		varlen += 1;
		/* flags */
		varlen += 1;
		/* index */
		varlen += 1;
	}

	/* A-MPDU */
	if (!(NonAmpduFrm) || !(NonAmpduSfrm)) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += (4 + padding_len);
		/* flags */
		varlen += 2;
		/* delimiter crc value */
		varlen += 1;
		/* reserved */
		varlen += 1;
	}

	/* VHT */
	if (TxMode == MODE_VHT) {
		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += (2 + padding_len);
		/* flags */
		varlen += 1;
		/* bandwidth */
		varlen += 1;
		/* mcs_nss */
		varlen += 4;
		/* coding */
		varlen += 1;
		/* group_id */
		varlen += 1;
		/* partial_aid */
		varlen += 2;
	}

	/** Adding RMAC */
	/** Currently Group 1~3 is enabled in RMAC in sniffer mode */
	varlen += MT7615_RMAC_LENGTH;
	mtk_rt_hdr = (struct mtk_radiotap_header *)skb_push(pOSPkt, sizeof(*mtk_rt_hdr) + varlen);
#ifdef WIRESHARK_1_12_2_1
	NdisZeroMemory(mtk_rt_hdr, sizeof(*mtk_rt_hdr) + varlen - MT7615_RMAC_LENGTH);
#else
	NdisZeroMemory(mtk_rt_hdr, sizeof(*mtk_rt_hdr) + varlen);
#endif
	mtk_rt_hdr->rt_hdr.it_version = PKTHDR_RADIOTAP_VERSION;
	mtk_rt_hdr->rt_hdr.it_pad = 0;
#ifdef WIRESHARK_1_12_2_1
	mtk_rt_hdr->rt_hdr.it_len = cpu2le16(sizeof(*mtk_rt_hdr) + varlen - MT7615_RMAC_LENGTH);
#else
	mtk_rt_hdr->rt_hdr.it_len = cpu2le16(sizeof(*mtk_rt_hdr) + varlen);
#endif
	mtk_rt_hdr->rt_hdr.it_present = cpu2le32(
										(1 << IEEE80211_RADIOTAP_TSFT) |
										(1 << IEEE80211_RADIOTAP_FLAGS));
#ifdef SNIFFER_MT7615_RMAC_INC
	/** RMAC in Radiotap Header Flag */
	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_MT7615_RMAC);
#endif

	if (TxMode < MODE_HTMIX)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_RATE);

	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_CHANNEL);
	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);

	if ((TxMode == MODE_HTMIX) || (TxMode == MODE_HTGREENFIELD))
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_MCS);

	if (!(NonAmpduFrm) || !(NonAmpduSfrm))
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_AMPDU_STATUS);

	if (TxMode == MODE_VHT)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_VHT);

	varlen = 0;
	pos = mtk_rt_hdr->variable;
	/* TSFT */
	/* Timestamp is present in DW12 (DW4-DW7 is absent) of RMAC */
	tmp64 = Timestamp;
	NdisMoveMemory(pos, &tmp64, 8);
	pos += 8;
	varlen += 8;
	/** Flags */
	*pos = 0;

	/* Short Preamble */
	if ((TxMode == MODE_CCK) && ((TxRate & 0x07) > 4))
		*pos |= IEEE80211_RADIOTAP_F_SHORTPRE;

	/* Fragmentation */
	if (FragFrm)
		*pos |= IEEE80211_RADIOTAP_F_FRAG;

	/* Bad FCS */
	if (FcsErr)
		*pos |= IEEE80211_RADIOTAP_F_BADFCS;

	/* Short GI */
	if (HtShortGi)
		*pos |= IEEE80211_RADIOTAP_F_SHORTGI;

	pos++;
	varlen++;

	/** Legacy Rate */
	if (TxMode == MODE_OFDM || TxMode == MODE_CCK) {
		/* CCK Rate */
		if (TxMode == MODE_CCK)
			*pos = (UCHAR)MT7615_CCK_Rate[(TxRate & 0x07)];
		/* OFDM Rate */
		else
			*pos = (UCHAR)MT7615_OFDM_Rate[(TxRate & 0x07)];

		pos++;
		varlen++;
	}

	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	pos += padding_len;
	varlen += padding_len;
#define ieee80211chan2mhz(x)	\
	(((x) <= 14) ? \
	 (((x) == 14) ? 2484 : ((x) * 5) + 2407) : \
	 ((x) + 1000) * 5)
	tmp16 = cpu2le16(ieee80211chan2mhz(ChFreq));
	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	if (ChFreq > 14)
		tmp16 = cpu2le16((IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ));
	else {
		if (TxMode == MODE_CCK)
			tmp16 = cpu2le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ);
		else
			tmp16 = cpu2le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ);
	}

	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;
	/* dBm ANT Signal */
	*pos = MaxRssi;
	pos++;
	varlen++;

	/** HT Information */
	if ((TxMode == MODE_HTMIX) || (TxMode == MODE_HTGREENFIELD)) {
		*pos = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
				IEEE80211_RADIOTAP_MCS_HAVE_MCS |
				IEEE80211_RADIOTAP_MCS_HAVE_GI |
				IEEE80211_RADIOTAP_MCS_HAVE_FMT |
				IEEE80211_RADIOTAP_MCS_HAVE_FEC |
				IEEE80211_RADIOTAP_MCS_HAVE_STBC);
		/* Ness Known */
		*pos |= (1 << 6);
		/* Ness data - bit 1 (MSB) of Number of extension spatial streams */
		*pos |= (HtExtltf << 6) & 0x80;
		pos++;
		varlen++;

		/* BW */
		if (FrMode == 0)
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_20);
		else if (FrMode == 1)
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_40);
		else {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s:unknown HT BW (%d)\n", __func__, FrMode));
		}

		/* HT GI */
		*pos |= HT_GI(HtShortGi);

		/* HT format */
		if (TxMode == MODE_HTMIX)
			*pos |= HT_FORMAT(0);
		else if (TxMode == MODE_HTGREENFIELD)
			*pos |= HT_FORMAT(1);

		/* HT FEC type */
		*pos |= HT_FEC_TYPE(HtAdCode);
		/* Number of STBC streams */
		*pos |= ((HtStbc) << 5) & 0x60;
		/* Ness data - bit 0 (LSB) of Number of extension spatial streams */
		*pos |= (HtExtltf & 0x1) << 7;
		pos++;
		varlen++;
		/* HT mcs index */
		*pos = TxRate;
		pos++;
		varlen++;
	}

	/** AMPDU */
	if (!(NonAmpduFrm) || !(NonAmpduSfrm)) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += padding_len;
		pos += padding_len;
		ampdu_refno = RxVSeq ? RxVSeq : ampdu_refno;
		/* tmp32 = ampdu_refno; */
		NdisMoveMemory(pos, &ampdu_refno, 4);
		pos += 4;
		varlen += 2;
		/* flags */
		tmp16 = 0;
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
		/* delimiter CRC value */
		*pos = 0;
		pos++;
		varlen++;
		/* reserved */
		*pos = 0;
		pos++;
		varlen++;
	}

#ifdef DOT11_VHT_AC

	/* VHT */
	if (TxMode == MODE_VHT) {
		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += padding_len;
		pos += padding_len;
		tmp16 = cpu2le16(IEEE80211_RADIOTAP_VHT_KNOWN_STBC |
						 IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA |
						 IEEE80211_RADIOTAP_VHT_KNOWN_GI |
						 IEEE80211_RADIOTAP_VHT_KNOWN_SGI_NSYM_DIS |
						 IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM |
						 IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH |
						 IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID);

		if ((GroupId == 0) || (GroupId == 63))
			tmp16 |= cpu2le16(IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED |
							  IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID);

		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
		/** flags */
		/* VHT STBC is present in HtStbc Bit 0 */
		*pos = ((HtStbc & 0x1) ? IEEE80211_RADIOTAP_VHT_FLAG_STBC : 0);
		/* TXOP PS Not Allowed */
		*pos |= ((VHTA1_B22) ? IEEE80211_RADIOTAP_VHT_FLAG_TXOP_PS_NA : 0);
		/* Short GI */
		*pos |= (HtShortGi ? IEEE80211_RADIOTAP_VHT_FLAG_SGI : 0);
		/* Short GI NSYM disambiguation (Present in Bit 0 of VHT_SIG_A2[B2:B1] */
		*pos |= ((VHT_A2 & 0x1) ? IEEE80211_RADIOTAP_VHT_FLAG_SGI_NSYM_M10_9 : 0);
		/* LDPC Extra OFDM symbol (Present in Bit 0 of VHT_SIG_A2[B8:B3] */
		*pos |= ((VHTA2_B8_B3 & 0x01) ? IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM : 0);

		/* Beamformed (For SU), (Present in Bit 5 of VHT_SIG_A2[B8:B3] */
		if ((GroupId == 0) || (GroupId == 63))
			*pos |= ((VHTA2_B8_B3 & 0x20) ? IEEE80211_RADIOTAP_VHT_FLAG_BEAMFORMED : 0);

		pos++;
		varlen++;

		/* bandwidth */
		if (FrMode == 0)
			*pos = 0;
		else if (FrMode == 1)
			*pos = 1;
		else if (FrMode == 2)
			*pos = 4;
		else
			*pos = 11;

		/* mcs_nss */
		pos++;
		varlen++;

		/* vht_mcs_nss[0] and MCS */
		if ((GroupId == 0) || (GroupId == 63)) {
			/* Nsts for SU Data */
			*pos = (VHTA1_B21_B10 & 0x007) + 1;
			/* MCS for SU Data */
			*pos |= (GET_VHT_MCS(TxRate) << 4);
		} else {
			/* Nsts for MU Data */
			*pos = (VHTA1_B21_B10 & 0x007);

			/* TODO: MCS for MU-MIMO will be received in VHT-SIG-B,
			   but this is not passed to RMAC/RXV, So, keeping MCS 0 */
			if (UP_value == 0)
				*pos |= (GET_VHT_MCS(TxRate) << 4);
			else
				*pos |= 0;
		}

		pos++;
		varlen++;
		/* vht_mcs_nss[1] */
		*pos = 0;

		if ((GroupId > 0) && (GroupId < 63)) {
			*pos = (VHTA1_B21_B10 & 0x038) >> 3;

			if (UP_value == 1)
				*pos |= (GET_VHT_MCS(TxRate) << 4);
			else
				*pos |= 0;
		}

		pos++;
		varlen++;
		/* vht_mcs_nss[2] */
		*pos = 0;

		if ((GroupId > 0) && (GroupId < 63)) {
			*pos = (VHTA1_B21_B10 & 0x1c0) >> 6;

			if (UP_value == 2)
				*pos |= (GET_VHT_MCS(TxRate) << 4);
			else
				*pos |= 0;

			/* TODO: MCS */
		}

		pos++;
		varlen++;
		/* vht_mcs_nss[3] */
		*pos = 0;

		if ((GroupId > 0) && (GroupId < 63)) {
			*pos = (VHTA1_B21_B10 & 0xe00) >> 9;

			if (UP_value == 3)
				*pos |= (GET_VHT_MCS(TxRate) << 4);
			else
				*pos |= 0;

			/* TODO: MCS */
		}

		pos++;
		varlen++;
		/* coding */
		*pos = 0;

		if (HtAdCode)
			*pos |= 1;

		if ((GroupId > 0) && (GroupId < 63))
			*pos |= (VHTA2_B8_B3 & 0x0e);

		pos++;
		varlen++;
		/* group_id */
		*pos = GroupId;
		pos++;
		varlen++;
		/* partial aid */
		tmp16 = 0;

		if ((GroupId == 0) || (GroupId == 63))
			tmp16 = VHTA1_B21_B10;

		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
	}

#endif /* DOT11_VHT_AC */
	pOSPkt->dev = pOSPkt->dev;
	pOSPkt->mac_header = pOSPkt->data - pOSPkt->head;
	pOSPkt->mac_len = mtk_rt_hdr->rt_hdr.it_len;
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = __constant_htons(ETH_P_80211_RAW);
	pOSPkt->ip_summed = CHECKSUM_NONE;
#ifdef SNIFFER_MT7615_RMAC_INC
#ifdef WIRESHARK_1_12_2_1
	memcpy((pOSPkt->data + mtk_rt_hdr->rt_hdr.it_len), rmac_info_buffer, MT7615_RMAC_LENGTH);
#else
	memcpy((pOSPkt->data + mtk_rt_hdr->rt_hdr.it_len - MT7615_RMAC_LENGTH), rmac_info_buffer, MT7615_RMAC_LENGTH);
#endif
#endif
	netif_rx_ni(pOSPkt);
#ifdef SNIFFER_MT7615_RMAC_INC
	os_free_mem(rmac_info_buffer);
#endif
	return;
err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

#endif /* SNIFFER_MT7615 */

void send_radiotap_monitor_packets(
	PNET_DEV pNetDev,
	PNDIS_PACKET pRxPacket,
	VOID *fc_field,
	UCHAR *pData,
	USHORT DataSize,
	UCHAR L2PAD,
	UCHAR PHYMODE,
	UCHAR BW,
	UCHAR ShortGI,
	UCHAR MCS,
	UCHAR LDPC,
	UCHAR LDPC_EX_SYM,
	UCHAR AMPDU,
	UCHAR STBC,
	CHAR RSSI1,
	UCHAR *pDevName,
	UCHAR Channel,
	UCHAR CentralChannel,
	UCHAR sideband_index,
	CHAR MaxRssi,
	UINT32 timestamp)
{
	struct sk_buff *pOSPkt;
	int rate_index = 0;
	USHORT header_len = 0;
	UCHAR temp_header[40] = {0};
	struct mtk_radiotap_header *mtk_rt_hdr;
	UINT32 varlen = 0, padding_len = 0;
	UINT64 tmp64;
	UINT32 tmp32;
	UINT16 tmp16;
	UCHAR *pos;
	FC_FIELD fc = *((FC_FIELD *)fc_field);
	MEM_DBG_PKT_FREE_INC(pRxPacket);
#ifdef RT_BIG_ENDIAN
	fc = SWAP16((UINT16)fc);
#endif /* RT_BIG_ENDIAN */
	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;

	if (fc.Type == 0x2 /* FC_TYPE_DATA */) {
		DataSize -= LENGTH_802_11;

		if ((fc.ToDs == 1) && (fc.FrDs == 1))
			header_len = LENGTH_802_11_WITH_ADDR4;
		else
			header_len = LENGTH_802_11;

		/* QOS */
		if (fc.SubType & 0x08) {
			header_len += 2;
			/* Data skip QOS contorl field */
			DataSize -= 2;
		}

		/* Order bit: A-Ralink or HTC+ */
		if (fc.Order) {
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

	if (DataSize < pOSPkt->len)
		skb_trim(pOSPkt, DataSize);
	else
		skb_put(pOSPkt, (DataSize - pOSPkt->len));

	if ((pData - pOSPkt->data) > 0) {
		skb_put(pOSPkt, (pData - pOSPkt->data));
		skb_pull(pOSPkt, (pData - pOSPkt->data));
	}

	if (skb_headroom(pOSPkt) < (sizeof(*mtk_rt_hdr) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof(*mtk_rt_hdr) + header_len), 0, GFP_ATOMIC)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s : Reallocate header size of sk_buff fail!\n",
					  __func__));
			goto err_free_sk_buff;
		}
	}

	if (header_len > 0)
		NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header, header_len);

	/* tsf */
	padding_len = ((varlen % 8) == 0) ? 0 : (8 - (varlen % 8));
	varlen += (8 + padding_len);
	/* flags */
	varlen += 1;

	/* rate */
	if (PHYMODE < MODE_HTMIX)
		varlen += 1;

	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	varlen += (2 + padding_len);
	/* channel flags */
	varlen += 2;

	/* MCS */
	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
		/* known */
		varlen += 1;
		/* flags */
		varlen += 1;
		/* index */
		varlen += 1;
	}

	/* A-MPDU */
	if (AMPDU) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += (4 + padding_len);
		/* flags */
		varlen += 2;
		/* delimiter crc value */
		varlen += 1;
		/* reserved */
		varlen += 1;
	}

	/* VHT */
	if (PHYMODE == MODE_VHT) {
		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += (2 + padding_len);
		/* flags */
		varlen += 1;
		/* bandwidth */
		varlen += 1;
		/* mcs_nss */
		varlen += 4;
		/* coding */
		varlen += 1;
		/* group_id */
		varlen += 1;
		/* partial_aid */
		varlen += 2;
	}

	mtk_rt_hdr = (struct mtk_radiotap_header *)skb_push(pOSPkt, sizeof(*mtk_rt_hdr) + varlen);
	NdisZeroMemory(mtk_rt_hdr, sizeof(*mtk_rt_hdr) + varlen);
	mtk_rt_hdr->rt_hdr.it_version = PKTHDR_RADIOTAP_VERSION;
	mtk_rt_hdr->rt_hdr.it_pad = 0;
	mtk_rt_hdr->rt_hdr.it_len = cpu2le16(sizeof(*mtk_rt_hdr) + varlen);
	mtk_rt_hdr->rt_hdr.it_present = cpu2le32(
										(1 << IEEE80211_RADIOTAP_TSFT) |
										(1 << IEEE80211_RADIOTAP_FLAGS));

	if (PHYMODE < MODE_HTMIX)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_RATE);

	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_CHANNEL);

	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD))
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_MCS);

	if (AMPDU)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_AMPDU_STATUS);

	if (PHYMODE == MODE_VHT)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_VHT);

	varlen = 0;
	pos = mtk_rt_hdr->variable;
	padding_len = ((varlen % 8) == 0) ? 0 : (8 - (varlen % 8));
	pos += padding_len;
	varlen += padding_len;
	/* tsf */
	tmp64 = timestamp;
	NdisMoveMemory(pos, &tmp64, 8);
	pos += 8;
	varlen += 8;
	/* flags */
	*pos = 0;
	pos++;
	varlen++;

	/* rate */
	if (PHYMODE == MODE_OFDM) {
		rate_index = (UCHAR)(MCS) + 4;
		*pos = ralinkrate[rate_index];
		pos++;
		varlen++;
	} else if (PHYMODE == MODE_CCK) {
		rate_index = (UCHAR)(MCS);
		*pos = ralinkrate[rate_index];
		pos++;
		varlen++;
	}

	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	pos += padding_len;
	varlen += padding_len;
#define ieee80211chan2mhz(x)	\
	(((x) <= 14) ? \
	 (((x) == 14) ? 2484 : ((x) * 5) + 2407) : \
	 ((x) + 1000) * 5)
	tmp16 = cpu2le16(ieee80211chan2mhz(Channel));
	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	if (Channel > 14)
		tmp16 = cpu2le16((IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ));
	else {
		if (PHYMODE == MODE_CCK)
			tmp16 = cpu2le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ);
		else
			tmp16 = cpu2le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ);
	}

	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	/* HT MCS */
	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
		*pos = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
				IEEE80211_RADIOTAP_MCS_HAVE_MCS |
				IEEE80211_RADIOTAP_MCS_HAVE_GI |
				IEEE80211_RADIOTAP_MCS_HAVE_FMT |
				IEEE80211_RADIOTAP_MCS_HAVE_FEC);
		pos++;
		varlen++;

		/* BW */
		if (BW == 0)
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_20);
		else
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_40);

		/* HT GI */
		*pos |= HT_GI(ShortGI);

		/* HT format */
		if (PHYMODE == MODE_HTMIX)
			*pos |= HT_FORMAT(0);
		else if (PHYMODE == MODE_HTGREENFIELD)
			*pos |= HT_FORMAT(1);

		/* HT FEC type */
		*pos |= HT_FEC_TYPE(LDPC);
		pos++;
		varlen++;
		/* HT mcs index */
		*pos = MCS;
		pos++;
		varlen++;
	}

	if (AMPDU) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += padding_len;
		pos += padding_len;
		tmp32 = 0;
		NdisMoveMemory(pos, &tmp32, 4);
		pos += 4;
		varlen += 2;
		/* flags */
		tmp16 = 0;
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
		/* delimiter CRC value */
		*pos = 0;
		pos++;
		varlen++;
		/* reserved */
		*pos = 0;
		pos++;
		varlen++;
	}

#ifdef DOT11_VHT_AC

	/* VHT */
	if (PHYMODE == MODE_VHT) {
		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += padding_len;
		pos += padding_len;
		tmp16 = cpu2le16(IEEE80211_RADIOTAP_VHT_KNOWN_STBC |
						 IEEE80211_RADIOTAP_VHT_KNOWN_GI |
						 IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM |
						 IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH);
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
		/* flags */
		*pos = (STBC ? IEEE80211_RADIOTAP_VHT_FLAG_STBC : 0);
		*pos |= (ShortGI ? IEEE80211_RADIOTAP_VHT_FLAG_SGI : 0);
		*pos |= (LDPC_EX_SYM ? IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM : 0);
		pos++;
		varlen++;

		/* bandwidth */
		if (BW == 0)
			*pos = 0;
		else if (BW == 1)
			*pos = 1;
		else if (BW == 2) {
			*pos = 4;
		} else
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:unknow bw(%d)\n", __func__, BW));

		/* mcs_nss */
		pos++;
		varlen++;
		/* vht_mcs_nss[0] */
		*pos = (GET_VHT_NSS(MCS) & 0x0f);
		*pos |= ((GET_VHT_MCS(MCS) & 0x0f) << 4);
		pos++;
		varlen++;
		/* vht_mcs_nss[1] */
		*pos = 0;
		pos++;
		varlen++;
		/* vht_mcs_nss[2] */
		*pos = 0;
		pos++;
		varlen++;
		/* vht_mcs_nss[3] */
		*pos = 0;
		pos++;
		varlen++;

		/* coding */
		if (LDPC)
			*pos = 1;
		else
			*pos = 0;

		pos++;
		varlen++;
		/* group_id */
		*pos = 0;
		pos++;
		varlen++;
		/* partial aid */
		tmp16 = 0;
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
	}

#endif /* DOT11_VHT_AC */
	pOSPkt->dev = pOSPkt->dev;
	pOSPkt->mac_header = (UINT)pOSPkt->data;
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = __constant_htons(ETH_P_80211_RAW);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx_ni(pOSPkt);
	return;
err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

VOID Monitor_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	PNET_DEV new_dev_p;
	INT idx = 0;
	struct wifi_dev *wdev;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

	if (pAd->monitor_ctrl.bMonitorInitiated != FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("monitor interface already initiated.\n"));
		return;
	}

#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
	dev_name = get_dev_name_prefix(pAd, INT_MONITOR);
	/* dev_name = "mon"; */
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MONITOR, idx, sizeof(struct mt_dev_priv), dev_name);

	if (!new_dev_p) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): Create net_device for %s(%d) fail!\n", __func__, dev_name, idx));
		return;
	}

	wdev = &pAd->monitor_ctrl.wdev;
	wdev->sys_handle = (void *)pAd;
	wdev->if_dev = new_dev_p;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);

	if (wdev_idx_reg(pAd, wdev) < 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Assign wdev idx for %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
		RtmpOSNetDevFree(new_dev_p);
		return;
	}

	/* init MAC address of virtual network interface */
	COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
	pNetDevOps->priv_flags = INT_MONITOR; /* we are virtual interface */
	pNetDevOps->needProtcted = TRUE;
	pNetDevOps->wdev = wdev;
	NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);
	/* register this device to OS */
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	pAd->monitor_ctrl.bMonitorInitiated = TRUE;
	return;
}


VOID Monitor_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;
	wdev = &pAd->monitor_ctrl.wdev;

	if (wdev->if_dev) {
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(wdev->if_dev);
		RtmpOSNetDevProtect(0);
		wdev_deinit(pAd, wdev);
		RtmpOSNetDevFree(wdev->if_dev);
		wdev->if_dev = NULL;
		pAd->monitor_ctrl.bMonitorInitiated = FALSE;
	}
}
BOOLEAN Monitor_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	if (pAd->monitor_ctrl.wdev.if_dev == dev_p)
		RTMP_OS_NETDEV_SET_TYPE(pAd->monitor_ctrl.wdev.if_dev, ARPHRD_IEEE80211_RADIOTAP);

	return TRUE;
}


BOOLEAN Monitor_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_SNIFFER_MODE_T SnifferFWCmd;
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (pAd->monitor_ctrl.wdev.if_dev == dev_p) {
		/* RTMP_OS_NETDEV_STOP_QUEUE(dev_p); */
		/* resume normal settings */
		pAd->monitor_ctrl.bMonitorOn = FALSE;
#ifdef CONFIG_HW_HAL_OFFLOAD
		SnifferFWCmd.ucDbdcIdx = 0;
		SnifferFWCmd.ucSnifferEn = 0;
		MtCmdSetSnifferMode(pAd, &SnifferFWCmd);
#else
		AsicSetRxFilter(pAd);
#endif /* CONFIG_HW_HAL_OFFLOAD */
		return TRUE;
	}

	return FALSE;
}


