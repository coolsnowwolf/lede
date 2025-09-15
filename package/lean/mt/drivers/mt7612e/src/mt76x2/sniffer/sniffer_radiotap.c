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


void send_radiotap_monitor_packets(
						PNET_DEV pNetDev,
			  			PNDIS_PACKET pRxPacket,
			  			VOID *dot11_hdr,
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
			  			UCHAR RSSI1,
			  			UCHAR *pDevName,
			  			UCHAR Channel,
			  			UCHAR CentralChannel,
						UCHAR sideband_index,
			  			CHAR MaxRssi,
			  			UINT32 timestamp) {
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

	if (skb_headroom(pOSPkt) < (sizeof(*mtk_rt_hdr) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof(*mtk_rt_hdr) + header_len), 0, GFP_ATOMIC)) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s : Reallocate header size of sk_buff fail!\n",
				  __FUNCTION__));
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

	/* dBm ANT Signal */
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

	if (PHYMODE < MODE_HTMIX) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_RATE);
	}

	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_CHANNEL);
	
	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);

	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_MCS);	
	}

	if (AMPDU) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_AMPDU_STATUS);
	}

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
				*pos = (UCHAR)RalinkRate_Legacy[MCS+4];
				pos++;
		varlen++;
	} else if (PHYMODE == MODE_CCK) {
				*pos = (UCHAR)RalinkRate_Legacy[MCS];
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

	if (Channel > 14) {
		tmp16 = cpu2le16((IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ));
	} else {
		if (PHYMODE == MODE_CCK) {
			tmp16 = cpu2le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ);
		} else {
			tmp16 = cpu2le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ);
		}
	}
	
	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	/* dBm ANT Signal */	
	*pos = MaxRssi;
	pos++;
	varlen++;

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
		if (BW == 0) {
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_20);	
		} else {
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_40);
		}

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
		*pos = (STBC?IEEE80211_RADIOTAP_VHT_FLAG_STBC:0);
		*pos |= (ShortGI?IEEE80211_RADIOTAP_VHT_FLAG_SGI:0);
		*pos |= (LDPC_EX_SYM?IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM:0);

		pos++;
		varlen++;

		/* bandwidth */
		if (BW == 0) {
			*pos = 0;
		} else if (BW == 1) {
				*pos = 1;
		} else if (BW == 2) {
				*pos = 4;

		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:unknow bw(%d)\n", __FUNCTION__, BW));
		}

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
	pOSPkt->mac_header = pOSPkt->data;
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = __constant_htons(ETH_P_80211_RAW);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx_ni(pOSPkt);
	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}


#ifdef PPI_HEADER
void send_ppi_monitor_packets(
						PNET_DEV pNetDev,
			  			PNDIS_PACKET pRxPacket,
			  			VOID *dot11_hdr,
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
			  			UCHAR RSSI1,
			  			UCHAR *pDevName,
			  			UCHAR Channel,
			  			UCHAR CentralChannel,
						UCHAR sideband_index,
			  			CHAR MaxRssi,
			  			UINT32 timestamp) {
	struct sk_buff *pOSPkt;
	int rate_index = 0;
	USHORT header_len = 0;
	UCHAR temp_header[40] = {0};
	struct mtk_radiotap_header *mtk_rt_hdr;

        ppi_packetheader_t *ppi_pkthead;
        ppi_fieldheader_t *ppi_fieldhead;
        UCHAR *field_pos;
        UINT16 nss;
        UINT16 isht;
        UINT32 itmp;
        UINT16 mcs;  
  
	UINT32 varlen = 0, padding_len = 0;
	UINT64 tmp64;
	UINT32 tmp32;
	UINT16 tmp16;
	UCHAR *pos;
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

	if (skb_headroom(pOSPkt) < (sizeof(ppi_packetheader_t) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof(ppi_packetheader_t) + header_len), 0, GFP_ATOMIC)) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s : Reallocate header size of sk_buff fail!\n",
				  __FUNCTION__));
			goto err_free_sk_buff;
		}
	}

	if (header_len > 0)
		NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header, header_len);

        isht=0;
        if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)){
            isht = sizeof(ppi_fieldheader_t) + SIZE_PPI_FIELD_80211n_MACPHYEXT;
           // printk("HT");            
        }   
        ppi_pkthead = (ppi_packetheader_t *)skb_push(pOSPkt, sizeof(ppi_packetheader_t) + sizeof(ppi_fieldheader_t)
                         + SIZE_PPI_FIELD_80211_COMMON + isht);
                         
        NdisZeroMemory(ppi_pkthead, sizeof(ppi_packetheader_t) + sizeof(ppi_fieldheader_t) 
                         + SIZE_PPI_FIELD_80211_COMMON + isht);

        ppi_pkthead->pph_version = 0;
        ppi_pkthead->pph_flags = 0;
        ppi_pkthead->pph_len = sizeof(ppi_packetheader_t) + sizeof(ppi_fieldheader_t) + SIZE_PPI_FIELD_80211_COMMON + isht;
        ppi_pkthead->pph_dlt = 105;
        field_pos = (UCHAR *)ppi_pkthead;
        
        ppi_fieldhead = (ppi_fieldheader_t *)(field_pos + sizeof(ppi_packetheader_t));
        field_pos = (UCHAR *)ppi_fieldhead;

        tmp16 = 2;
        ppi_fieldhead->pfh_type = tmp16;
        tmp16 = SIZE_PPI_FIELD_80211_COMMON;
        ppi_fieldhead->pfh_datalen = tmp16;
        
        field_pos = (UCHAR *)(field_pos + sizeof(ppi_fieldheader_t));
        
        //TSF-Timer
        tmp64 = timestamp;
	NdisMoveMemory(field_pos, &tmp64, 8);
	field_pos += 8;
	
	//Flags
	tmp16 = 0;
	NdisMoveMemory(field_pos, &tmp16, 2);
	field_pos += 2;

       /* rate */
	if (PHYMODE == MODE_OFDM) {
		tmp16 = (UINT16)RalinkRate_Legacy[MCS+4];
		NdisMoveMemory(field_pos, &tmp16, 2);
		field_pos += 2;
	} else if (PHYMODE == MODE_CCK) {
		tmp16 = (UINT16)RalinkRate_Legacy[MCS];
		NdisMoveMemory(field_pos, &tmp16, 2);
		field_pos += 2;
	}else if((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)){
	       //For HT omnipeek will calculate rates
		field_pos += 2;
	}else if(PHYMODE == MODE_VHT){
	        nss = (GET_VHT_NSS(MCS) & 0x0f);
                mcs = (GET_VHT_MCS(MCS) & 0x0f);
	        tmp16 = (UINT16)(ieee80211_vhtinfo[mcs][BW][ShortGI][nss]);
	        tmp16 = tmp16*2;
               // printk("-VHT %u %u-",tmp16,timestamp);
	        NdisMoveMemory(field_pos, &tmp16, 2);
		field_pos += 2;
	}else{
	        field_pos += 2;
	}
	
#define ieee80211chan2mhz(x)	\
	(((x) <= 14) ? \
	(((x) == 14) ? 2484 : ((x) * 5) + 2407) : \
	 ((x) + 1000) * 5)

	tmp16 = cpu2le16(ieee80211chan2mhz(Channel));
	NdisMoveMemory(field_pos, &tmp16, 2);
	field_pos += 2;
	
	if (Channel > 14) {
		tmp16 = cpu2le16((IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ));
	} else {
		if (PHYMODE == MODE_CCK) {
			tmp16 = cpu2le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ);
		} else {
			tmp16 = cpu2le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ);
		}
	}
	
	
	NdisMoveMemory(field_pos, &tmp16, 2);
	field_pos += 2;
	
	//FHSS-Hopset
	field_pos += 1;
	
	//FHSS-Pattern
	field_pos += 1;
	
	/* dBm ANT Signal */	
	*(field_pos) = MaxRssi;
	field_pos += 1;
	
	//dBm ANT Noise
	field_pos += 1;
	
	//802.11n MAC+PHY Extension
        if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
        
            ppi_fieldhead = (ppi_fieldheader_t *)(field_pos);

            tmp16 = 4;
            ppi_fieldhead->pfh_type = tmp16;
            tmp16 = SIZE_PPI_FIELD_80211n_MACPHYEXT;
            ppi_fieldhead->pfh_datalen = tmp16;
            field_pos = field_pos + sizeof(ppi_fieldheader_t);

            tmp32 = 0;
            itmp = 1;
            
            if(PHYMODE == MODE_HTGREENFIELD){
              tmp32 |= itmp;
            }
            
            if(BW == 1){
              tmp32 |= itmp<<1;
            }
            
            if(ShortGI == 1){
              tmp32 |= itmp<<2;
            }
            
            NdisMoveMemory(field_pos, &tmp32, 4);
            
            field_pos += 4;
            
            //A-MPDU-ID
            field_pos += 4;
            
            //Num-Delimiters
            field_pos += 1;
            
            //MCS
            *field_pos = MCS;
             field_pos += 1;
            
            //Num-Streams
            field_pos += 1;
            
            //RSSI-Combined
            field_pos += 1;
            
            //RSSI-Ant0Ctl
            field_pos += 1;
            
            //RSSI-Ant1Ctl
            field_pos += 1;
            
            //RSSI-Ant2Ctl
            field_pos += 1;
            
            //RSSI-Ant3Ctl
            field_pos += 1;
            
            //RSSI-Ant0Ext
            field_pos += 1;
            
            //RSSI-Ant1Ext
            field_pos += 1;
            
            //RSSI-Ant2Ext
            field_pos += 1;
            
            //RSSI-Ant3Ext
            field_pos += 1;
            
            //Extension Channel-Freq
            field_pos += 2;
            
            //Extension Channel-Flags
            field_pos += 2;
            
            //dBm-Ant0signal
            field_pos += 1;
            
            //dBm-Ant0signal
            field_pos += 1;
            
            //dBm-Ant1signal
            field_pos += 1;
            
            //dBm-Ant1noise
            field_pos += 1;
            
            //dBm-Ant2signal
            field_pos += 1;
            
            //dBm-Ant2noise
            field_pos += 1;
            
            //dBm-Ant3signal
            field_pos += 1;
            
            //dBm-Ant3noise
            field_pos += 1;
            
            //EVM0
            field_pos += 4;
            
            //EVM1
            field_pos += 4;
            
            //EVM2
            field_pos += 4;
            
            //EVM3
            field_pos += 4;
            
        }
	

	pOSPkt->dev = pOSPkt->dev;
	pOSPkt->mac_header = pOSPkt->data;
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = __constant_htons(ETH_P_80211_RAW);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx_ni(pOSPkt);
	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}
#endif

VOID Monitor_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{

	PNET_DEV new_dev_p;
	INT idx=0;
	struct wifi_dev *wdev;
	
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

	if(pAd->monitor_ctrl.monitor_initiated != FALSE)
	{
		printk("monitor interface already initiated.\n");		
		return;
	}

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */

	dev_name = get_dev_name_prefix(pAd, INT_MONITOR);
	//dev_name = "mon";
	
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MONITOR, idx,sizeof(struct mt_dev_priv), dev_name);
	if (!new_dev_p) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Create net_device for %s(%d) fail!\n",
					__FUNCTION__, dev_name, idx));
		return;
	}
		
	wdev = &pAd->monitor_ctrl.monitor_wdev;
		
	//wdev->func_dev = pApCliEntry;
	wdev->sys_handle = (void *)pAd;
	wdev->if_dev = new_dev_p;
	//wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
	if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
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
	pAd->monitor_ctrl.monitor_initiated = TRUE;	
	return;
}


VOID Monitor_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;
	wdev = &pAd->monitor_ctrl.monitor_wdev;
	if (wdev->if_dev)
	{
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(wdev->if_dev);
		RtmpOSNetDevProtect(0);
		rtmp_wdev_idx_unreg(pAd, wdev);
		RtmpOSNetDevFree(wdev->if_dev);

		wdev->if_dev = NULL;
		pAd->monitor_ctrl.monitor_initiated = FALSE;
	}
	
}
BOOLEAN Monitor_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	UINT32	Value = 0;
	if (pAd->monitor_ctrl.monitor_wdev.if_dev == dev_p)
	{
#ifdef PPI_HEADER
#define PPI_ARPHRD 1000

	        RTMP_OS_NETDEV_SET_TYPE(pAd->monitor_ctrl.monitor_wdev.if_dev,PPI_ARPHRD);
#else
		RTMP_OS_NETDEV_SET_TYPE(pAd->monitor_ctrl.monitor_wdev.if_dev,ARPHRD_IEEE80211_RADIOTAP);
#endif
		
//RxFilter will be decided by iwpriv set MonitorMode	
	}

	return TRUE;
}


BOOLEAN Monitor_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{	
	UINT32	Value = 0;
	if (pAd->monitor_ctrl.monitor_wdev.if_dev == dev_p)
	{
		/* RTMP_OS_NETDEV_STOP_QUEUE(dev_p); */
		
		/* resume normal settings */
#ifdef CONFIG_STA_SUPPORT
		pAd->StaCfg.BssType = BSS_INFRA;
#else
		pAd->ApCfg.BssType = BSS_INFRA;
#endif
		
		AsicSetRxFilter(pAd);
		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value); 
		Value &= ~(0x80); 
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);		
		
		return TRUE;
	}


	
	return FALSE;
}
