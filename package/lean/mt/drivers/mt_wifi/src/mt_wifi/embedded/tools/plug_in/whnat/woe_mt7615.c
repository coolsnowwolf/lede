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

	Module Name: whnat
	whnat_mt7615.c
*/

#ifdef MT7615
#include "woe_mt7615.h"
#include "woe.h"
#include <net/ra_nat.h>
#include <linux/pci.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

/*Local function part*/
/*
*
*/
static void wifi_pcie_match(struct wifi_entry *wifi)
{
	unsigned char idx = 0;
	idx = wifi_slot_get(wifi->cookie);
	wifi->slot_id = idx;
	wifi->wpdma_base = wifi_wpdma_base_get(wifi->cookie);
}



/*Gloable function*/
/*
*
*/
void wifi_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid)
{
	TMAC_TXD_L *txd;
	TMAC_TXD_0 *txd0;
	TMAC_TXD_1 *txd1;
	CR4_TXP_MSDU_INFO *txp;

	txd = (TMAC_TXD_L *)fbuf;
	txp = (CR4_TXP_MSDU_INFO *)(fbuf+sizeof(TMAC_TXD_L));
	memset(txd, 0, sizeof(*txd));
	memset(txp, 0, sizeof(*txp));
	/*initial txd*/
	txd0 = &txd->TxD0;
	txd0->TxByteCount = sizeof(*txd);
	txd0->p_idx = P_IDX_LMAC;
	txd0->q_idx = 0;
	txd1 = &txd->TxD1;
	txd1->ft = TMI_FT_LONG;
	txd1->txd_len = 0;
	txd1->pkt_ft = TMI_PKT_FT_HIF_CT;
	txd1->hdr_format = TMI_HDR_FT_NON_80211;
	TMI_HDR_INFO_VAL(TMI_HDR_FT_NON_80211, 0, 0, 0, 0, 0, 0, 0, txd1->hdr_info);
	txd1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
	/*init txp*/
	txp->msdu_token = tkid;
	/*without TXD, CR4 will take care it*/
	txp->type_and_flags = 0;
	txp->buf_num = 1;
	txp->buf_ptr[0] = pkt_pa;
	txp->buf_len[0] = 0;
}

/*
*
*/
static inline void wifi_tx_info_wrapper(unsigned char *tx_info, struct wlan_tx_info *info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;

	info->pkt = txblk->pPacket;
	info->bssidx = txblk->wdev->bss_info_argument.ucBssIndex;
	info->ringidx = txblk->dbdc_band;
#ifdef MAC_REPEATER_SUPPORT

	if (txblk->pMacEntry && IS_ENTRY_REPEATER(txblk->pMacEntry))
		info->wcid = txblk->pMacEntry->wcid;
	else
#endif
	{
		info->wcid = 0xff;
	}
}

#ifdef WHNAT_DBG_EN
/*
*
*/
static void wifi_dump_skb(
	struct whnat_entry *entry,
	struct wlan_tx_info *info,
	struct sk_buff *skb)
{
	struct iphdr *hdr = ip_hdr(skb);

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s(): add entry: wdma=%d,ringId=%d,wcid=%d,bssid=%d\n",
		__func__,
		entry->idx,
		info->ringidx,
		info->wcid,
		info->bssidx);

	if( hdr->version != 4)
		return;

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s():src=%d.%d.%d.%d\n",
		__func__,
		(0xff & hdr->saddr),
		(0xff00 & hdr->saddr) >> 8,
		(0xff0000 & hdr->saddr) >> 16,
		(0xff000000 & hdr->saddr) >>24);

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s(): dst=%d.%d.%d.%d\n",
		__func__,
		(0xff & hdr->daddr),
		(0xff00 & hdr->daddr) >> 8,
		(0xff0000 & hdr->daddr) >> 16,
		(0xff000000 & hdr->daddr) >>24);

	if( hdr->protocol == IPPROTO_TCP) {
		struct tcphdr *tcph = tcp_hdr(skb);
		WHNAT_DBG(WHNAT_DBG_INF,
			"%s(): protocol=TCP,sport=%d,dstport=%d\n",
			__func__,
			tcph->source,
			tcph->dest);
	}

	if( hdr->protocol == IPPROTO_UDP) {
		struct udphdr *udph = udp_hdr(skb);
		WHNAT_DBG(WHNAT_DBG_INF,
			"%s(): protocol=UDP,sport=%d,dstport=%d\n",
			__func__,
			udph->source,
			udph->dest);
	}
}
#endif /*WHNAT_DBG_EN*/

/*
*
*/
void wifi_tx_tuple_add(void *entry, unsigned char *tx_info)
{
	struct whnat_entry *whnat = (struct whnat_entry *)entry;
	struct wlan_tx_info t, *info =  &t;

	memset(info, 0, sizeof(*info));
	wifi_tx_info_wrapper(tx_info, info);
	WHNAT_DBG(WHNAT_DBG_INF, "WDMAID: %d,RingID: %d, Wcid: %d, Bssid: %d\n",
			whnat->idx, info->ringidx, info->wcid, info->bssidx);

	if (whnat && ra_sw_nat_hook_tx && whnat->cfg.hw_tx_en) {
		struct sk_buff *skb = (struct sk_buff *)info->pkt;

		if (skb_headroom(skb) < FOE_INFO_LEN)
			return;

		/*WDMA idx*/
		FOE_WDMA_ID(skb) = whnat->idx;
		/*Ring idx*/
		FOE_RX_ID(skb) = info->ringidx;
		/*wtable Idx*/
		FOE_WC_ID(skb) = info->wcid;
		/*Bssidx*/
		FOE_BSS_ID(skb) = info->bssidx;
		/*use port for specify which hw_nat architecture*/
		ra_sw_nat_hook_tx(skb, WHNAT_WDMA_PORT);
#ifdef WHNAT_DBG_EN
		wifi_dump_skb(whnat, info, skb);
#endif /*WHNAT_DBG_EN*/
	}
}

/*
*
*/
char wifi_hw_tx_allow(void *cookie, unsigned char *tx_info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;
	struct wifi_dev *wdev = txblk->wdev;

	if (!wdev)
		return FALSE;

	if (wlan_operate_get_frag_thld(wdev) != DEFAULT_FRAG_THLD)
		return FALSE;

	return TRUE;
}

/*
*
*/
void wifi_dma_cfg_wrapper(int wifi_cfg, unsigned char *dma_cfg)
{
	if (wifi_cfg == -1)
		*dma_cfg = WHNAT_DMA_DISABLE;
	else {
		switch (wifi_cfg) {
		case PDMA_TX_RX:
			*dma_cfg = WHNAT_DMA_TXRX;
			break;

		case PDMA_TX:
			*dma_cfg = WHNAT_DMA_TX;
			break;

		case PDMA_RX:
			*dma_cfg = PDMA_TX_RX;
			break;
		}
	}
}

/*
*
*/
void wifi_tx_tuple_reset(void)
{
	/* FoeTblClean(); */
}

/*
*
*/
unsigned int wifi_ser_status(void *ser_ctrl)
{
#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T *ctrl = (ERR_RECOVERY_CTRL_T *)ser_ctrl;

	if (ctrl)
		return ctrl->errRecovState;

#endif /*ERR_RECOVERY*/
	return WIFI_ERR_RECOV_NONE;
}

/*
* Wifi function part
*/

/*
*
*/
void dump_wifi_value(struct wifi_entry *wifi, char *name, unsigned int addr)
{
	unsigned int value;

	WHNAT_IO_READ32(wifi, addr, &value);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s\t:%x\n", name, value);
}

/*
*
*/
void wifi_dump_tx_ring_info(struct wifi_entry *wifi, unsigned char ring_id, unsigned int idx)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct _PCI_HIF_T *pci_cfg = &ad->PciHif;
	RTMP_DMACB *cb = &pci_cfg->TxRing[ring_id].Cell[idx];

	WHNAT_DBG(WHNAT_DBG_OFF, "AllocPA\t: 0x%llx\n", cb->AllocPa);
	WHNAT_DBG(WHNAT_DBG_OFF, "AllocVa\t: %p\n", cb->AllocVa);
	WHNAT_DBG(WHNAT_DBG_OFF, "Size\t: %lu\n", cb->AllocSize);
	WHNAT_DBG(WHNAT_DBG_OFF, "pNdisPacket\t: %p\n", cb->pNdisPacket);
	WHNAT_DBG(WHNAT_DBG_OFF, "token_id\t: %d\n", cb->token_id);
	whnat_dump_raw("WED_TX_RING", cb->AllocVa, cb->AllocSize);
}

/*
*
*/
unsigned int wifi_chip_id_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	return ad->ChipID;
}

/*
*
*/
unsigned int wifi_whnat_en_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	return ad->CommonCfg.whnat_en;
}

/*
*
*/
void wifi_whnat_en_set(void *cookie, unsigned int en)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	ad->CommonCfg.whnat_en = en;
}

/*
*
*/
void wifi_chip_cr_mirror_set(struct wifi_entry *wifi, unsigned char enable)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (enable) {
		ops->hif_io_read32 = NULL;
		ops->hif_io_write32 = NULL;
	} else {
		ops->hif_io_read32 = whnat_hal_io_read;
		ops->hif_io_write32 = whnat_hal_io_write;
	}
}

/*
* CHIP related setting
*/
void wifi_chip_probe(struct wifi_entry *wifi, unsigned int irq)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	struct net_device *dev = ad->net_dev;

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Chang CHIP IRQ: %d to WHNAT IRQ: %d\n", __func__, pci_dev->irq, irq);
	wifi->irq = pci_dev->irq;
	pci_dev->irq = irq;
	dev->irq = irq;
	/*always disable hw cr mirror first */
	wifi_chip_cr_mirror_set(wifi, FALSE);
	wifi->base_addr = (unsigned long)ad->PciHif.CSRBaseAddress;
	wifi->int_mask = &ad->PciHif.IntEnableReg;
	wifi_pcie_match(wifi);
}

/*
*
*/
void wifi_chip_remove(struct wifi_entry *wifi)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Chang WED IRQ: %d to CHIP IRQ: %d\n", __func__, pci_dev->irq, wifi->irq);
	/*revert pci irq as original irq*/
	pci_dev->irq = wifi->irq;
	wifi->irq = 0;
	ops->hif_io_read32 = NULL;
	ops->hif_io_write32 = NULL;
	wifi->base_addr = 0;
	wifi->int_mask = NULL;
}

/*
*
*/
int wifi_slot_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *) cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	unsigned int id = 1;
	if(pci_dev->bus) {
		id = (pci_dev->bus->self->devfn >> 3) & 0x1f;
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): bus name=%s, funid=%d, get slot id=%d\n",
			__func__,
			pci_dev->bus->name,
			pci_dev->bus->self->devfn,
			id);
	}
	return id;
}



/*
*
*/
unsigned int wifi_wpdma_base_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *) cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	unsigned int wpdma_base = 0;

	if(pci_dev->bus) {
		wpdma_base = (unsigned int) pci_resource_start(pci_dev, 0);
		wpdma_base |= WPDMA_OFFSET;
	}
	return wpdma_base;
}


/*
*
*/
void wifi_cap_get(struct wifi_entry *wifi)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	wifi->tx_ring_num = cap->num_of_tx_ring;
}

#endif /*MT7615*/
