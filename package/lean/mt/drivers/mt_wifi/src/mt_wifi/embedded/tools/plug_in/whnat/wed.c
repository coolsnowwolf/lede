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

	Module Name: wifi_offload
	wed.c
*/
#include "wed.h"
#include "woe.h"
#include "wed_def.h"
#include "woe_hw.h"
/*global definition*/
#define WED_DEV_NODE "mediatek,wed"


/*
*define local function
*/

/*
*
*/
static void dump_tx_ring(struct whnat_ring *ring)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_desc_base\t: 0x%x\n", ring->hw_desc_base);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_cidx_addr\t: 0x%x\n", ring->hw_cidx_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_didx_addr\t: 0x%x\n", ring->hw_didx_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_cnt_addr\t: 0x%X\n", ring->hw_cnt_addr);
}


/*
*
*/
void dump_token_info(struct wed_buf_res *res, unsigned int id)
{
	struct wed_token_info *info;
	struct list_head *cur;
	unsigned int token_id = (id + WED_TOKEN_START);

	list_for_each(cur, &res->pkt_head) {
		info = list_entry(cur, struct wed_token_info, list);

		if (token_id == info->token_id) {
			WHNAT_DBG(WHNAT_DBG_OFF, "token_id\t: %d\n", info->token_id);
			WHNAT_DBG(WHNAT_DBG_OFF, "desc_len\t: %d\n", info->desc_len);
			WHNAT_DBG(WHNAT_DBG_OFF, "desc_pa\t: 0x%llx\n", info->desc_pa);
			WHNAT_DBG(WHNAT_DBG_OFF, "desc_va\t: 0x%p\n", info->desc_va);
			WHNAT_DBG(WHNAT_DBG_OFF, "len\t: %d\n", info->len);
			WHNAT_DBG(WHNAT_DBG_OFF, "pkt\t: 0x%p\n", info->pkt);
			WHNAT_DBG(WHNAT_DBG_OFF, "pkt_pa\t: 0x%llx\n", info->pkt_pa);
			WHNAT_DBG(WHNAT_DBG_OFF, "pkt_va\t: 0x%p\n", info->pkt_va);
			WHNAT_DBG(WHNAT_DBG_OFF, "fd_len\t: %d\n", info->fd_len);
			WHNAT_DBG(WHNAT_DBG_OFF, "fd_pa\t: 0x%llx\n", info->fdesc_pa);
			WHNAT_DBG(WHNAT_DBG_OFF, "fd_va\t: 0x%p\n", info->fdesc_va);
			whnat_dump_raw("WED_TX_DMAD", info->desc_va, info->desc_len);
			whnat_dump_raw("WED_TX_BM", info->fdesc_va, info->fd_len);
		}
	}
}


/*
*
*/
static void dump_buf_res(struct wed_buf_res *res)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "token_num\t: %d\n", res->token_num);
	WHNAT_DBG(WHNAT_DBG_OFF, "pkt_num\t: %d\n", res->pkt_num);
	WHNAT_DBG(WHNAT_DBG_OFF, "des_buf\t:\n");
	whnat_dump_dmabuf(&res->des_buf);
	WHNAT_DBG(WHNAT_DBG_OFF, "dma_len\t: %d\n", res->dmad_len);
	WHNAT_DBG(WHNAT_DBG_OFF, "fbuf\t:\n");
	whnat_dump_dmabuf(&res->fbuf);
	WHNAT_DBG(WHNAT_DBG_OFF, "buf_len\t: %d\n", res->fd_len);
	WHNAT_DBG(WHNAT_DBG_OFF, "pkt_len\t: %d\n", res->pkt_len);
}

/*
*
*/
static void dump_tx_ring_ctrl_basic(struct wed_tx_ring_ctrl *ring_ctrl)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "txd_len\t: %d\n", ring_ctrl->txd_len);
	WHNAT_DBG(WHNAT_DBG_OFF, "ring_num\t: %d\n", ring_ctrl->ring_num);
	WHNAT_DBG(WHNAT_DBG_OFF, "ring_len\t: %d\n", ring_ctrl->ring_len);
	WHNAT_DBG(WHNAT_DBG_OFF, "desc\t:\n");
	whnat_dump_dmabuf(ring_ctrl->desc);
}


/*
*
*/
static void dump_tx_ring_ctrl(struct wed_tx_ring_ctrl *ring_ctrl)
{
	unsigned char i;

	dump_tx_ring_ctrl_basic(ring_ctrl);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		WHNAT_DBG(WHNAT_DBG_OFF, "ring\t: %d\n", i);
		dump_tx_ring(&ring_ctrl->ring[i]);
	}
}


/*
*
*/
static void dump_tx_ctrl(struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_buf_res *res = &tx_ctrl->res;
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;

	WHNAT_DBG(WHNAT_DBG_OFF, "buf_res\t:\n");
	dump_buf_res(res);
	WHNAT_DBG(WHNAT_DBG_OFF, "ring_ctrl\t:\n");
	dump_tx_ring_ctrl(ring_ctrl);
}


/*
*
*/
static void dump_res_ctrl(struct wed_res_ctrl *res_ctrl)
{
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;

	WHNAT_DBG(WHNAT_DBG_OFF, "tx_ctrl:\n");
	dump_tx_ctrl(tx_ctrl);
}

/*
*
*/
void dump_wed_value(struct wed_entry *wed, char *name, unsigned int addr)
{
	unsigned int value;

	WHNAT_IO_READ32(wed, addr, &value);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s\t:%x\n", name, value);
}

/*
*
*/
static void dump_wed_debug_info(struct wed_entry *wed)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED DEBUG INFO:==========\n");
	dump_wed_value(wed, "WED_IRQ_MON", WED_IRQ_MON);
	dump_wed_value(wed, "WED_TX_BM_VB_FREE_0_31", WED_TX_BM_VB_FREE_0_31);
	dump_wed_value(wed, "WED_TX_BM_VB_FREE_32_63", WED_TX_BM_VB_FREE_32_63);
	dump_wed_value(wed, "WED_TX_BM_VB_USED_0_31", WED_TX_BM_VB_USED_0_31);
	dump_wed_value(wed, "WED_TX_BM_VB_USED_32_63", WED_TX_BM_VB_USED_32_63);
	dump_wed_value(wed, "WED_WDMA_INT_MON", WED_WDMA_INT_MON);
	dump_wed_value(wed, "WED_WPDMA_INT_CLR", WED_WPDMA_INT_CLR);
	dump_wed_value(wed, "WED_WPDMA_INT_CTRL", WED_WPDMA_INT_CTRL);
	dump_wed_value(wed, "WED_WPDMA_INT_MSK", WED_WPDMA_INT_MSK);
	dump_wed_value(wed, "WED_WPDMA_INT_MON", WED_WPDMA_INT_MON);
	dump_wed_value(wed, "WED_WPDMA_SPR", WED_WPDMA_SPR);
	dump_wed_value(wed, "WED_PCIE_INTM_REC", WED_PCIE_INTM_REC);
	dump_wed_value(wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	dump_wed_value(wed, "WED_TXD_DW0", WED_TXD_DW0);
	dump_wed_value(wed, "WED_TXD_DW1", WED_TXD_DW1);
	dump_wed_value(wed, "WED_TXD_DW2", WED_TXD_DW2);
	dump_wed_value(wed, "WED_TXD_DW3", WED_TXD_DW3);
	dump_wed_value(wed, "WED_TXD_DW4", WED_TXD_DW4);
	dump_wed_value(wed, "WED_TXD_DW5", WED_TXD_DW5);
	dump_wed_value(wed, "WED_TXD_DW6", WED_TXD_DW6);
	dump_wed_value(wed, "WED_TXD_DW7", WED_TXD_DW7);
	dump_wed_value(wed, "WED_TXP_DW0", WED_TXP_DW0);
	dump_wed_value(wed, "WED_TXP_DW1", WED_TXP_DW1);
	dump_wed_value(wed, "WED_DBG_CTRL", WED_DBG_CTRL);
	dump_wed_value(wed, "WED_DBG_PRB0", WED_DBG_PRB0);
	dump_wed_value(wed, "WED_DBG_PRB1", WED_DBG_PRB1);
	dump_wed_value(wed, "WED_DBG_PRB2", WED_DBG_PRB2);
	dump_wed_value(wed, "WED_DBG_PRB3", WED_DBG_PRB3);
	dump_wed_value(wed, "WED_TX_COHERENT_MIB", WED_TX_COHERENT_MIB);
	dump_wed_value(wed, "WED_TXP_DW0", WED_TXP_DW0);
}

/*
*
*/
static inline void eint_disable(struct wed_entry *wed, unsigned int isr)
{
	wed->ext_int_mask &= ~(1 << isr);
}

/*
*
*/
static inline void eint_enable(struct wed_entry *wed, unsigned int isr)
{
	wed->ext_int_mask |= (1 << isr);
	whnat_hal_eint_ctrl(wed->whnat, TRUE);
}


/*
*
*/
struct sk_buff *alloc_dma_tx_pkt(
	struct platform_device *pdev,
	unsigned int len,
	void **vaddr,
	dma_addr_t *paddr)
{
	struct sk_buff *pkt;
#ifdef BB_SOC
	pkt = skbmgr_dev_alloc_skb4k();
#else
	pkt = dev_alloc_skb(len);
#endif

	if (pkt == NULL)
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): can't allocate tx %d size packet\n", __func__, len);

	if (pkt) {
		*vaddr = (void *) pkt->data;
		*paddr = dma_map_single(&pdev->dev, *vaddr, len, PCI_DMA_TODEVICE);
	} else {
		*vaddr = (void *) NULL;
		*paddr = (dma_addr_t) 0;
	}

	return pkt;
}


/*
*
*/
int free_dma_tx_pkt(
	struct platform_device *pdev,
	struct sk_buff *pkt,
	dma_addr_t pa,
	unsigned int len)
{
	if (pkt == NULL)
		return -1;

	dma_unmap_single(&pdev->dev, pa, len, PCI_DMA_FROMDEVICE);
	dev_kfree_skb_any(pkt);
	return 0;
}


/*
*
*/
static void token_buf_exit(
	struct wed_entry *entry,
	struct wed_buf_res *res,
	unsigned int sid,
	unsigned int size)

{
}

/*
*
*/
static int token_buf_init(
	struct wed_entry *entry,
	struct wed_buf_res *res,
	unsigned int sid,
	unsigned int size)
{
	struct wed_token_info *info;
	struct list_head *cur;
	struct whnat_txdmad *txdma = NULL;
	unsigned int eid = sid+size;
	unsigned int id;

	list_for_each(cur, &res->pkt_head) {
		info = list_entry(cur, struct wed_token_info, list);

		if (info->token_id > WED_TOKEN_END) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): token id out of range (%d,%d)!\n", __func__,
					info->token_id, WED_TOKEN_END);
			goto err;
		}

		id = info->token_id - WED_TOKEN_START;

		if (id >= sid && id < eid) {
			/*init token entry*/
			txdma = (struct whnat_txdmad *)info->desc_va;
			txdma->sdp0 = info->fdesc_pa;
			txdma->sdl0 = info->fd_len;
			txdma->sdp1 = info->pkt_pa;
			txdma->sdlen1 = info->len;
			txdma->last0 = 0;
			txdma->last1 = 1;
			txdma->burst = 0;
			txdma->ddone = 0;
			/*init firt buf with MAC TXD+CR4 TXP*/
			wifi_fbuf_init((unsigned char *)info->fdesc_va, txdma->sdp1, info->token_id);
		}
	}
	return 0;
err:
	token_buf_exit(entry, res, sid, size);
	return -1;
}

/*
*
*/
static void token_info_free(
	struct wed_entry *entry,
	struct wed_buf_res *res,
	unsigned int sid,
	unsigned int size)
{
	struct wed_token_info *info;
	struct list_head *cur;
	struct list_head *next;
	struct platform_device *pdev = entry->pdev;
	unsigned int id;
	unsigned int eid = sid+size;

	list_for_each_safe(cur, next, &res->pkt_head) {
		info = list_entry(cur, struct wed_token_info, list);
		id = info->token_id-WED_TOKEN_START;

		if (id >= sid && id < eid) {
			free_dma_tx_pkt(pdev, info->pkt, info->pkt_pa, info->len);
			list_del(&info->list);
			memset(info, 0, sizeof(*info));
			kfree(info);
		}
	}
}

/*
*
*/
static int token_info_alloc(
	struct wed_entry *entry,
	struct wed_buf_res *res,
	unsigned int sid,
	unsigned int size)
{
	unsigned int i;
	unsigned int eid = sid+size;
	struct wed_token_info *info;

	/*prepare info and add to list */
	for (i = sid; i < eid; i++) {
		/*check token id*/
		if (i >= WED_TOKEN_CNT_MAX) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate wrong token id %d,sid=%d,size=%d!\n",
					__func__, i, sid, eid);
			goto err;
		}

		/*allocate token info*/
		info = kmalloc(sizeof(struct wed_token_info), GFP_KERNEL);

		if (info == NULL) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate token %d info fail!\n", __func__, i);
			goto err;
		}

		memset(info, 0, sizeof(struct wed_token_info));
		info->token_id = (WED_TOKEN_START + i);
		info->len = res->pkt_len;
		/*allocate skb*/
		info->pkt = alloc_dma_tx_pkt(entry->pdev, info->len, &info->pkt_va, &info->pkt_pa);

		if (info->pkt == NULL) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate pkt for token %d fail!\n", __func__, i);
			kfree(info);
			goto err;
		}

		/*allocate txd*/
		info->desc_len = res->dmad_len;
		info->desc_pa = res->des_buf.alloc_pa+(i*info->desc_len);
		info->desc_va = res->des_buf.alloc_va+(i*info->desc_len);
		/*allocate first buffer*/
		info->fd_len = res->fd_len;
		info->fdesc_pa = res->fbuf.alloc_pa+(i*info->fd_len);
		info->fdesc_va = res->fbuf.alloc_va+(i*info->fd_len);
		/*insert to list*/
		list_add_tail(&info->list, &res->pkt_head);
	}

	return 0;
err:
	token_info_free(entry, res, sid, size);
	return -1;
}

/*
*
*/
static void token_info_exit(struct wed_entry *entry, struct wed_buf_res *res)
{
	struct platform_device *pdev = entry->pdev;

	if (!res->des_buf.alloc_va)
		return;

	token_info_free(entry, res, 0, res->pkt_num);
	whnat_dma_buf_free(pdev, &res->des_buf);
	whnat_dma_buf_free(pdev, &res->fbuf);
}

/*
*
*/
static int token_info_init(struct wed_entry *entry, struct wed_buf_res *res)
{
	unsigned int len;

	len = res->dmad_len * res->token_num;

	/*allocate wed descript buffer*/
	if (whnat_dma_buf_alloc(entry->pdev, &res->des_buf, len) < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate txd buffer fail!\n", __func__);
		goto err;
	}

	/*for test only*/
	{
		unsigned int i;
		struct whnat_txdmad *txd;

		for (i = 0; i < res->token_num; i++) {
			txd = res->des_buf.alloc_va+(i*res->dmad_len);
			txd->rsv = i;
		}
	}
	/*allocate wed descript buffer*/
	len = res->fd_len * res->token_num;

	if (whnat_dma_buf_alloc(entry->pdev, &res->fbuf, len) < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate txd buffer fail!\n", __func__);
		goto err;
	}

	/*initial list */
	INIT_LIST_HEAD(&res->pkt_head);

	if (token_info_alloc(entry, res, 0, res->pkt_num) < 0)
		goto err;

	return 0;
err:
	token_info_exit(entry, res);
	return -1;
}

#ifdef WED_DYNAMIC_BM_SUPPORT
/*
*
*/
static int token_buf_expend(struct wed_entry *entry, unsigned int grp_num)
{
	struct wed_tx_ctrl *tx_ctrl = &entry->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	unsigned int size = grp_num*WED_TOKEN_EXPEND_SIZE;

	/*tx resource allocate*/
	if (token_info_alloc(entry, res, res->pkt_num, size) < 0)
		goto err;

	if (token_buf_init(entry, res, res->pkt_num, size) < 0)
		goto err;

	res->pkt_num += WED_TOKEN_EXPEND_SIZE;
	return 0;
err:
	/*error handle*/
	token_info_free(entry, res, res->pkt_num, size);
	return -1;
}

/*
*
*/
static void token_buf_reduce(struct wed_entry *entry, unsigned int grp_num)
{
	struct wed_tx_ctrl *tx_ctrl = &entry->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	unsigned int size = grp_num*WED_TOKEN_EXPEND_SIZE;

	res->pkt_num -= size;
	/*tx buffer deinit*/
	token_buf_exit(entry, res, res->pkt_num, size);
	/*tx resource allocate*/
	token_info_free(entry, res, res->pkt_num, size);
}

/*
*
*/
static void token_free_task(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	unsigned int size = res->pkt_num-(1*WED_TOKEN_EXPEND_SIZE);

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): old packet num:%d\n", __func__, res->pkt_num);

	if (size > WED_TOKEN_CNT) {
		token_buf_reduce(wed, 1);
		whnat_hal_bfm_update(wed, TRUE);
		eint_enable(wed, (WED_EX_INT_STA_FLD_TX_FBUF_LTH | WED_EX_INT_STA_FLD_TX_FBUF_HTH));
	}
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): update packet num:%d\n", __func__, res->pkt_num);
}

/*
*
*/
static void token_alloc_task(unsigned long data)
{
	struct wed_entry *wed = (struct wed_entry *)data;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	unsigned int size = res->pkt_num+(1*WED_TOKEN_EXPEND_SIZE);

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): old packet num:%d\n", __func__, res->pkt_num);

	if (size <= WED_TOKEN_CNT_MAX) {
		token_buf_expend(wed, 1);
		whnat_hal_bfm_update(wed, FALSE);
		eint_enable(wed, (WED_EX_INT_STA_FLD_TX_FBUF_LTH | WED_EX_INT_STA_FLD_TX_FBUF_HTH));
	}
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): update packet num:%d\n", __func__, res->pkt_num);
}
#endif /*WED_DYNAMIC_BM_SUPPORT*/

/*
*
*/
static void tx_dma_cb_exit(struct wed_entry *entry, struct whnat_dma_cb *dma_cb)
{
	struct _TXD_STRUC *txd;
	struct sk_buff *pkt;

	txd = (TXD_STRUC *) (dma_cb->alloc_va);
	pkt = dma_cb->pkt;

	if (pkt)
		free_dma_tx_pkt(entry->pdev, pkt, txd->SDPtr0, txd->SDLen0);

	/*Always assign pNdisPacket as NULL after clear*/
	dma_cb->pkt = NULL;
	memset(dma_cb, 0, sizeof(struct whnat_dma_cb));
}

/*
* assign tx description for tx ring entry
*/
static int tx_dma_cb_init(struct whnat_dma_buf *desc,
						  unsigned int idx,
						  struct whnat_dma_cb *dma_cb)
{
	dma_cb->pkt = NULL;
	/* Init Tx Ring Size, Va, Pa variables */
	dma_cb->alloc_size = WIFI_PDMA_TXD_SIZE;
	dma_cb->alloc_va = desc->alloc_va+(idx*dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa+(idx*dma_cb->alloc_size);
	/* advance to next ring descriptor address */
	WIFI_TXD_INIT(dma_cb->alloc_va);
	return 0;
}



/*
*
*/
static void tx_ring_exit(
	struct wed_entry *entry,
	struct whnat_ring *ring,
	struct whnat_dma_buf *desc)
{
	unsigned int i;

	for (i = 0; i < WIFI_TX_RING_SIZE; i++)
		tx_dma_cb_exit(entry, &ring->cell[i]);

	whnat_dma_buf_free(entry->pdev, desc);
}

/*
*
*/
static int tx_ring_init(
	struct wed_entry *entry,
	unsigned char idx,
	struct wed_tx_ring_ctrl *ring_ctrl)
{
	unsigned int i;
	unsigned int len;
	struct whnat_ring *ring = &ring_ctrl->ring[idx];
	struct whnat_dma_buf *desc = &ring_ctrl->desc[idx];
	unsigned int offset = idx*WIFI_RING_OFFSET;

	len = ring_ctrl->txd_len*ring_ctrl->ring_len;

	if (whnat_dma_buf_alloc(entry->pdev, desc, len) < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate desc fail, len=%d\n", __func__, len);
		return -1;
	}

	/*should find wifi cr & control it*/
	ring->hw_desc_base = whnat_wifi_cr_get(WHNAT_CR_WED, WED_TX0_CTRL0+offset);
	ring->hw_cnt_addr  = whnat_wifi_cr_get(WHNAT_CR_WED, WED_TX0_CTRL1+offset);
	ring->hw_cidx_addr = whnat_wifi_cr_get(WHNAT_CR_WED, WED_TX0_CTRL2+offset);
	ring->hw_didx_addr = whnat_wifi_cr_get(WHNAT_CR_WED, WED_TX0_CTRL3+offset);

	for (i = 0; i < ring_ctrl->ring_len; i++)
		tx_dma_cb_init(desc, i, &ring->cell[i]);

	return 0;
}

/*
*
*/
static int tx_ring_reset(
	struct wed_entry *entry,
	unsigned char idx,
	struct wed_tx_ring_ctrl *ring_ctrl)
{
	unsigned int i;
	struct whnat_ring *ring = &ring_ctrl->ring[idx];
	struct whnat_dma_buf *desc = &ring_ctrl->desc[idx];
	struct whnat_dma_cb *dma_cb;

	for (i = 0; i < ring_ctrl->ring_len; i++) {
		dma_cb = &ring->cell[i];

		if (dma_cb->pkt)
			tx_dma_cb_exit(entry, dma_cb);

		tx_dma_cb_init(desc, i, dma_cb);
	}

	return 0;
}


/*
*
*/
static void wed_tx_ring_exit(struct wed_entry *entry, struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;
	unsigned int len;
	unsigned char i;

	len = sizeof(struct whnat_ring)*ring_ctrl->ring_num;

	/*free skb in ring*/
	for (i = 0; i < ring_ctrl->ring_num; i++)
		tx_ring_exit(entry, &ring_ctrl->ring[i], &ring_ctrl->desc[i]);

	/*free wed tx ring*/
	kfree(ring_ctrl->ring);
	/*free desc*/
	kfree(ring_ctrl->desc);
}

/*
*
*/
static int wed_tx_ring_init(struct wed_entry *entry, struct wed_tx_ctrl *tx_ctrl)
{
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;
	struct whnat_entry *whnat = (struct whnat_entry *)entry->whnat;
	unsigned int len;
	unsigned char i;

	ring_ctrl->ring_num = whnat->wifi.tx_ring_num;
	ring_ctrl->ring_len = WIFI_TX_RING_SIZE;
	ring_ctrl->txd_len = WIFI_PDMA_TXD_SIZE;
	/*allocate wed descript for original chip ring*/
	len = sizeof(struct whnat_dma_buf) * ring_ctrl->ring_num;
	ring_ctrl->desc = kmalloc(len, GFP_KERNEL);
	memset(ring_ctrl->desc, 0, len);

	if (!ring_ctrl->desc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate tx desc faild\n", __func__);
		goto err;
	}

	/*allocate wed tx ring, assign initial value */
	len = sizeof(struct whnat_ring)*ring_ctrl->ring_num;
	ring_ctrl->ring = kmalloc(len, GFP_KERNEL);
	memset(ring_ctrl->ring, 0, len);

	if (!ring_ctrl->ring) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate tx ring faild\n", __func__);
		goto err;
	}

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		/*initial for "PDMA" TX ring*/
		if (tx_ring_init(entry, i, ring_ctrl) < 0) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): init tx ring faild\n", __func__);
			goto err;
		}
	}

	return 0;
err:
	wed_tx_ring_exit(entry, tx_ctrl);
	return -1;
}

/*
*
*/
static int wed_tx_ring_reset(struct wed_entry *entry, struct wed_tx_ctrl *tx_ctrl)
{
	unsigned int i;
	struct wed_tx_ring_ctrl *ring_ctrl = &tx_ctrl->ring_ctrl;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		/*reset for "PDMA" TX ring*/
		tx_ring_reset(entry, i, ring_ctrl);
	}

	return 0;
}

/*
*
*/
int wed_init(struct platform_device *pdev, unsigned char idx, struct wed_entry *wed)
{
	/*assign to pdev*/
	struct resource *res;
	unsigned int irq;
	unsigned long base_addr;

	res = platform_get_resource(pdev, IORESOURCE_MEM, idx);
	irq = platform_get_irq(pdev, idx);
	base_addr = (unsigned long)devm_ioremap(&pdev->dev, res->start, resource_size(res));
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(%d): irq=%d,base_addr=0x%lx\n", __func__, idx, irq, base_addr);
	wed->base_addr = base_addr;
	wed->pdev = pdev;
	wed->irq = irq;
	/*allocate ring first*/
#ifdef WED_TX_SUPPORT
	wed_ring_init(wed);
#endif
	/*assign tx ring to ad*/
#ifdef WED_HW_TX_SUPPORT
	wed_token_buf_init(wed);
#endif /*WED_HW_TX_SUPPORT*/
#ifdef WED_DYNAMIC_BM_SUPPORT
	tasklet_init(&wed->tbuf_alloc_task, token_alloc_task, (unsigned long)wed);
	tasklet_init(&wed->tbuf_free_task, token_free_task, (unsigned long)wed);
#endif /*WED_DYNAMIC_BM_SUPPORT*/
#ifdef ERR_RECOVERY
	wed_ser_init(wed);
#endif /*ERR_RECOVERY*/
	return 0;
}

/*
*
*/
void wed_exit(struct platform_device *pdev, struct wed_entry *wed)
{
#ifdef ERR_RECOVERY
	wed_ser_exit(wed);
#endif /*ERR_RECOVERY*/
#ifdef WED_TX_SUPPORT
	wed_ring_exit(wed);
#endif
#ifdef WED_HW_TX_SUPPORT
	wed_token_buf_exit(wed);
#endif
	/*assign to pdev*/
	devm_iounmap(&pdev->dev, (void *)wed->base_addr);
	memset(wed, 0, sizeof(*wed));
}

/*
*
*/
unsigned char wed_num_get(void)
{
	struct device_node *node = NULL;
	unsigned int num = 0;

	node = of_find_compatible_node(NULL, NULL, WED_DEV_NODE);

	if (of_property_read_u32_index(node, "wed_num", 0, &num)) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): get WED number from DTS fail!!\n", __func__);
		return 0;
	}

	return (unsigned char)num;
}

/*
*
*/
unsigned char wed_slot_map_get(unsigned int idx)
{
	struct device_node *node = NULL;
	unsigned int num = 0;

	node = of_find_compatible_node(NULL, NULL, WED_DEV_NODE);

	if (of_property_read_u32_index(node, "pci_slot_map", idx, &num)) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): get WED slot from DTS fail!!\n", __func__);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): assign default value: (slot0->devfn0), (slot1->devfn1)!!\n", __func__);
		num = (idx==0) ? 0 : 1;
	}
	WHNAT_DBG(WHNAT_DBG_OFF,
		"%s(): assign slot_id:%d for entry: %d!\n", __func__, num, idx);
	return (unsigned char)num;
}

/*
*
*/
int wed_ring_init(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	int ret;

	ret = wed_tx_ring_init(entry, tx_ctrl);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed ring init result = %d\n", __func__, ret);
	return ret;
}

/*
*
*/
int wed_ring_reset(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;
	int ret;

	ret = wed_tx_ring_reset(entry, tx_ctrl);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed ring reset result = %d\n", __func__, ret);
	return ret;
}

/*
*
*/
void wed_ring_exit(struct wed_entry *entry)
{
	struct wed_res_ctrl *res_ctrl = &entry->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res_ctrl->tx_ctrl;

	wed_tx_ring_exit(entry, tx_ctrl);
}



/*
*
*/
void wed_token_buf_exit(struct wed_entry *entry)
{
	struct wed_tx_ctrl *tx_ctrl = &entry->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	/*tx resource free*/
	token_buf_exit(entry, res, 0, res->pkt_num);
	token_info_exit(entry, res);
}

/*
*
*/
int wed_token_buf_init(struct wed_entry *entry)
{
	struct wed_tx_ctrl *tx_ctrl = &entry->res_ctrl.tx_ctrl;
	struct wed_buf_res *res = &tx_ctrl->res;
	/*tx resource allocate*/
	res->token_num = WED_TOKEN_CNT_MAX;
	res->pkt_num = WED_TOKEN_CNT;
	res->dmad_len = WIFI_PDMA_TXD_SIZE;
	res->fd_len = WIFI_TX_1ST_BUF_SIZE;
	res->pkt_len = WIFI_TX_BUF_SIZE;

	if (token_info_init(entry, res) < 0)
		goto err;

	if (token_buf_init(entry, res, 0, res->pkt_num) < 0)
		goto err;

	return 0;
err:
	/*error handle*/
	wed_token_buf_exit(entry);
	return -1;
}

/*
*
*/
void dump_wed_basic(struct wed_entry *wed)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "virtual addr:\t 0x%lx\n", wed->base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "irq:\t %d\n", wed->irq);
	dump_res_ctrl(&wed->res_ctrl);
}

/*
*
*/
static void dump_tx_ring_raw(struct wed_entry *wed, unsigned char ring_id, unsigned int idx)
{
	struct whnat_entry *whnat = (struct whnat_entry *)wed->whnat;
	struct wifi_entry *wifi = &whnat->wifi;
	struct whnat_ring *ring = &wed->res_ctrl.tx_ctrl.ring_ctrl.ring[ring_id];
	unsigned char *addr;
	unsigned int size;

	WHNAT_DBG(WHNAT_DBG_OFF, "==========WED TX RING RAW (%d/0x%x)==========\n", ring_id, idx);
	wifi_dump_tx_ring_info(wifi, ring_id, idx);
	/*WED_WPDMA Tx Ring content*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WPDAM TX RING RAW (%d/0x%x)==========\n", ring_id, idx);
	whnat_dump_dmacb(&ring->cell[idx]);
	addr = (unsigned char *) ring->cell[idx].alloc_va;
	size = ring->cell[idx].alloc_size;
	whnat_dump_raw("WPDMA_TX_RING", addr, size);
}

/*
*
*/
void wed_proc_handle(struct wed_entry *wed, char choice, char *arg)
{
	unsigned int i;
	char *str;
	char *end;
	unsigned char idx;
	struct wed_res_ctrl *res = &wed->res_ctrl;
	struct wed_tx_ctrl *tx_ctrl = &res->tx_ctrl;
	struct wed_buf_res *buf_res = &tx_ctrl->res;

	switch (choice) {
	case WED_PROC_TX_RING_BASIC:
		dump_wed_basic(wed);
		break;

	case WED_PROC_TX_BUF_BASIC:
		dump_buf_res(buf_res);
		break;

	case WED_PROC_TX_BUF_INFO: {
		str = strsep(&arg, " ");
		str = strsep(&arg, " ");
		i =  whnat_str_tol(str, &end, 16);
		dump_token_info(buf_res, i);
	}
	break;

	case WED_PROC_TX_RING_CELL: {
		str = strsep(&arg, " ");
		str = strsep(&arg, " ");
		idx =  whnat_str_tol(str, &end, 10);
		str = strsep(&arg, " ");
		i =  whnat_str_tol(str, &end, 16);
		dump_tx_ring_raw(wed, idx, i);
	}
	break;

	case WED_PROC_DBG_INFO:
		dump_wed_debug_info(wed);
		break;
#ifdef WED_DYNAMIC_BM_SUPPORT

	case WED_PROC_TX_DYNAMIC_ALLOC: {
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_LTH);
		token_alloc_task((unsigned long)wed);
	}
	break;

	case WED_PROC_TX_DYNAMIC_FREE: {
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_HTH);
		token_free_task((unsigned long)wed);
	}
	break;
#endif
#ifdef WED_HW_TX_SUPPORT

	case WED_PROC_TX_FREE_CNT:
		whnat_hal_bfm_freecnt(wed, &i);
		break;
#endif

	case WED_PROC_TX_RESET:
	case WED_PROC_RX_RESET:
		whnat_hal_hw_reset(wed->whnat, WHNAT_RESET_IDX_ONLY);
		break;

	default:
		break;
	}
}

/*
*
*/
void wed_eint_handle(struct wed_entry *wed, unsigned int status)
{
#ifdef WED_DYNAMIC_RX_BM_SUPPORT

	if (status & (1 << WED_EX_INT_STA_FLD_RX_FBUF_HTH))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx rbuf high threshold!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_FBUF_LTH))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx rbuf low threshold!\n", __func__);

#endif

	if (status & (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): tx free notify len error!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): tx free token has no packet to point!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_FBUF_HTH)) {
#ifdef WED_DYNAMIC_BM_SUPPORT
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_HTH);
		if ((WED_PKT_NUM_GET(wed)-WED_TOKEN_EXPEND_SIZE) >= WED_TOKEN_CNT) {
			WHNAT_DBG(WHNAT_DBG_INF, "%s(): tx buf high threshold!\n", __func__);
			tasklet_hi_schedule(&wed->tbuf_free_task);
		}
#endif /*WED_DYNAMIC_BM_SUPPORT*/
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_FBUF_LTH)) {
#ifdef WED_DYNAMIC_BM_SUPPORT
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_LTH);
		if ((WED_PKT_NUM_GET(wed)+WED_TOKEN_EXPEND_SIZE) <= WED_TOKEN_CNT_MAX) {
			WHNAT_DBG(WHNAT_DBG_INF, "%s(): tx buf low threshold!\n", __func__);
			tasklet_hi_schedule(&wed->tbuf_alloc_task);
		}

#endif /*WED_DYNAMIC_BM_SUPPORT*/
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): tx dma write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): tx dma read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLE))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx drv inti wdma enable!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT))
		WHNAT_DBG(WHNAT_DBG_LOU, "%s(): rx drv coherent!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx drv write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx drv read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_TITO_INVLD))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): tx free token id is invaild!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_BM_DMAD_COHERENT))
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx drv buffer mgmt dmad coherent!\n", __func__);
#ifdef WED_WDMA_RECYCLE
	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_DMA_RECYCLE))
		WHNAT_DBG(WHNAT_DBG_LOU, "%s(): rx drv dma recycle!\n", __func__);
#endif /*WED_WDMA_RECYCLE*/
}

