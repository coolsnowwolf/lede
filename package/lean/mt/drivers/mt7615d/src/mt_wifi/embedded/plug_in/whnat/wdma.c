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
	wdma.c
*/

#include "wdma.h"
#include "woe.h"

/*global definition*/
#define WDMA_DEV_NODE "mediatek,wed-wdma"


/*
*
*/
static void dump_wdma_rx_ring(struct whnat_ring *ring)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_desc_base:\t%x\n", ring->hw_desc_base);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_cnt_addr:\t%x\n", ring->hw_cnt_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_cidx_addr:\t%x\n", ring->hw_cidx_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "hw_didx_addr:\t%x\n", ring->hw_didx_addr);
}
/*
*
*/
static void dump_wdma_basic(struct wdma_entry *wdma)
{
	int i;
	struct wdma_rx_ring_ctrl *ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;

	WHNAT_DBG(WHNAT_DBG_OFF, "Base Addr:\t%lx\n", wdma->base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "IRQ:\t%d:%d:%d\n", wdma->irq[0], wdma->irq[1], wdma->irq[2]);
	WHNAT_DBG(WHNAT_DBG_OFF, "Pdev:\t%p\n", wdma->pdev);
	WHNAT_DBG(WHNAT_DBG_OFF, "Proc:\t%p\n", wdma->proc);
	WHNAT_DBG(WHNAT_DBG_OFF, "ring_num:\t%d\n", ring_ctrl->ring_num);
	WHNAT_DBG(WHNAT_DBG_OFF, "ring_len:\t%d\n", ring_ctrl->ring_len);
	WHNAT_DBG(WHNAT_DBG_OFF, "rxd_len:\t%d\n", ring_ctrl->rxd_len);

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		dump_wdma_rx_ring(&ring_ctrl->ring[i]);
		whnat_dump_dmabuf(&ring_ctrl->desc[i]);
	}
}

/*
*
*/
static void dump_rx_ring_raw(struct wdma_entry *wdma, unsigned char ring_id, unsigned int idx)
{
	struct whnat_ring *ring = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl.ring[ring_id];
	unsigned char *addr;
	unsigned int size;
	/*WDMA Tx Ring content*/
	WHNAT_DBG(WHNAT_DBG_OFF, "==========WDMA RX RING RAW (%d/0x%x)==========\n", ring_id, idx);
	whnat_dump_dmacb(&ring->cell[idx]);
	addr = (unsigned char *) ring->cell[idx].alloc_va;
	size = ring->cell[idx].alloc_size;
	whnat_dump_raw("WDMA_RX_RING", addr, size);
}

/*
* assign tx description for tx ring entry
*/
#define DMADONE_DONE 1
static int rx_dma_cb_init(struct whnat_dma_buf *desc,
						  unsigned int idx,
						  struct whnat_dma_cb *dma_cb)
{
	struct WDMA_RXD *rxd;

	dma_cb->pkt = NULL;
	/* Init Tx Ring Size, Va, Pa variables */
	dma_cb->alloc_size = sizeof(struct WDMA_RXD);
	dma_cb->alloc_va = desc->alloc_va+(idx*dma_cb->alloc_size);
	dma_cb->alloc_pa = desc->alloc_pa+(idx*dma_cb->alloc_size);
	/* link the pre-allocated TxBuf to TXD */
	rxd = (struct WDMA_RXD *)dma_cb->alloc_va;
	/* advance to next ring descriptor address */
	rxd->ddone = DMADONE_DONE;
	return 0;
}

/*
* local function
*/
static void rx_ring_exit(struct wdma_entry *entry, unsigned char idx, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	struct whnat_dma_buf *desc = &ring_ctrl->desc[idx];

	whnat_dma_buf_free(entry->pdev, desc);
}

/*
*
*/
static int rx_ring_init(struct wdma_entry *entry, unsigned char idx, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i;
	unsigned int len = 0;
	struct whnat_dma_buf *desc = &ring_ctrl->desc[idx];
	struct whnat_ring *ring = &ring_ctrl->ring[idx];
	unsigned int offset = idx*WIFI_RING_OFFSET;

	len = ring_ctrl->rxd_len*ring_ctrl->ring_len;
	ring->hw_desc_base = WDMA_RX_BASE_PTR_0+offset;
	ring->hw_cnt_addr  = WDMA_RX_MAX_CNT_0+offset;
	ring->hw_cidx_addr = WDMA_RX_CRX_IDX_0+offset;
	ring->hw_didx_addr = WDMA_RX_DRX_IDX_0+offset;

	if (whnat_dma_buf_alloc(entry->pdev, desc, len) < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): allocate rx ring fail!\n", __func__);
		return -1;
	}

	for (i = 0; i < ring_ctrl->ring_len; i++)
		rx_dma_cb_init(desc, i, &ring->cell[i]);

	return 0;
}

/*
*
*/
static void wdma_rx_ring_exit(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	int i;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		rx_ring_exit(wdma, i, ring_ctrl);

	if (ring_ctrl->ring)
		kfree(ring_ctrl->ring);

	if (ring_ctrl->desc)
		kfree(ring_ctrl->desc);

	memset(ring_ctrl, 0, sizeof(*ring_ctrl));
}

/*
*
*/
static int wdma_rx_ring_init(struct wdma_entry *wdma, struct wdma_rx_ring_ctrl *ring_ctrl)
{
	unsigned int len;
	struct whnat_entry *whnat = (struct whnat_entry *)wdma->whnat;
	int i;

	ring_ctrl->rxd_len = sizeof(struct WDMA_RXD);
	ring_ctrl->ring_num = whnat->wifi.tx_ring_num;
	ring_ctrl->ring_len = WDMA_TX_BM_RING_SIZE;
	len = sizeof(struct whnat_dma_buf)*ring_ctrl->ring_num;
	ring_ctrl->desc = kmalloc(len, GFP_KERNEL);
	memset(ring_ctrl->desc, 0, len);

	if (!ring_ctrl->desc)
		goto err;

	len = sizeof(struct whnat_ring) * ring_ctrl->ring_num;
	ring_ctrl->ring = kmalloc(len, GFP_KERNEL);
	memset(ring_ctrl->ring, 0, len);

	if (!ring_ctrl->ring)
		goto err;

	len = ring_ctrl->ring_len * ring_ctrl->rxd_len;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		if (rx_ring_init(wdma, i, ring_ctrl) < 0)
			goto err;
	}

	return 0;
err:
	WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx ring init faild\n", __func__);
	wdma_rx_ring_exit(wdma, ring_ctrl);
	return -1;
}

/*
*
*/
int wdma_init(struct platform_device *pdev, unsigned char idx, struct wdma_entry *wdma)
{
	struct device_node *node = NULL;
	unsigned char i = 0;
	unsigned char irq_idx = 0;

	node = of_find_compatible_node(NULL, NULL, WDMA_DEV_NODE);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): get node=%p\n", __func__, &node);
	/* iomap registers */
	wdma->base_addr = (unsigned long)of_iomap(node, idx);
	wdma->pdev = pdev;
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wdma(%d) base addr=%lx\n", __func__, idx, wdma->base_addr);

	for (i = 0; i < WDMA_IRQ_NUM; i++) {
		irq_idx = i+(idx*WDMA_IRQ_NUM);
		wdma->irq[i] = irq_of_parse_and_map(node, irq_idx);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wdma(%d) irq[%d]=%d\n", __func__, idx, i, wdma->irq[i]);
	}

	memset(&wdma->res_ctrl, 0, sizeof(wdma->res_ctrl));
	/*initial wdma related resource*/
#ifdef WED_HW_TX_SUPPORT
	wdma_ring_init(wdma);
#endif
	return 0;
}

/*
*
*/
int wdma_exit(struct platform_device *pdev, struct wdma_entry *wdma)
{
	if (wdma) {
#ifdef WED_HW_TX_SUPPORT
		/*remove ring & token buffer*/
		wdma_ring_exit(wdma);
#endif
		iounmap((void *)wdma->base_addr);
		memset(wdma, 0, sizeof(*wdma));
	}

	return 0;
}

/*
*
*/
int	wdma_ring_init(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;

	wdma_rx_ring_init(wdma, &rx_ctrl->rx_ring_ctrl);
	return 0;
}

/*
*
*/
void wdma_ring_exit(struct wdma_entry *wdma)
{
	struct wdma_res_ctrl *res = &wdma->res_ctrl;
	struct wdma_rx_ctrl *rx_ctrl = &res->rx_ctrl;

	wdma_rx_ring_exit(wdma, &rx_ctrl->rx_ring_ctrl);
}

/*
*
*/
void dump_wdma_value(struct wdma_entry *wdma, char *name, unsigned int addr)
{
	unsigned int value;

	WHNAT_IO_READ32(wdma, addr, &value);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s\t:%x\n", name, value);
}

/*
*
*/
void wdma_proc_handle(struct wdma_entry *wdma, char choice, char *arg)
{
	unsigned int i;
	char *str;
	char *end;
	unsigned char idx;

	switch (choice) {
	case WDMA_PROC_BASIC:
		dump_wdma_basic(wdma);
		break;

	case WDMA_PROC_RX_CELL: {
		str = strsep(&arg, " ");
		str = strsep(&arg, " ");
		idx =  whnat_str_tol(str, &end, 10);
		str = strsep(&arg, " ");
		i =  whnat_str_tol(str, &end, 16);
		dump_rx_ring_raw(wdma, idx, i);
	}
	break;

	default:
		break;
	}
}
