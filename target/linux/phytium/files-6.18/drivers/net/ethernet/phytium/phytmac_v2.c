// SPDX-License-Identifier: GPL-2.0-only
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/time64.h>
#include <linux/ptp_classify.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/net_tstamp.h>
#include <linux/circ_buf.h>
#include <linux/spinlock.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/acpi.h>
#include "phytmac.h"
#include "phytmac_v2.h"

static int phytmac_v2_msg_send(struct phytmac *pdata, u16 cmd_id,
			    u16 cmd_subid, void *data, int len, int wait)
{
	u32 tx_head, tx_tail, ring_size;
	struct phytmac_msg_info msg;
	struct phytmac_msg_info msg_rx;
	unsigned long flags;
	u32 retry = 0;
	int ret = 0;

	spin_lock_irqsave(&pdata->msg_ring.msg_lock, flags);
	tx_head = PHYTMAC_READ(pdata, PHYTMAC_TX_MSG_HEAD) & 0xff;
	tx_tail = phytmac_v2_tx_ring_wrap(pdata, pdata->msg_ring.tx_msg_wr_tail);
	pdata->msg_ring.tx_msg_rd_tail = tx_tail;
	ring_size = pdata->msg_ring.tx_msg_ring_size;

	while ((tx_tail + 1) % ring_size == tx_head) {
		udelay(1);
		tx_head = PHYTMAC_READ(pdata, PHYTMAC_TX_MSG_HEAD) & 0xff;
		retry++;
		if (retry >= PHYTMAC_RETRY_TIMES) {
			netdev_err(pdata->ndev,
				   "Time out waiting for Tx msg ring free, tx_tail:0x%x, tx_head:0x%x",
				   tx_tail, tx_head);
			ret = -EINVAL;
			goto err_out;
		}
	}

	retry = 0;
	wait = 1;
	memset(&msg, 0, sizeof(msg));
	memset(&msg_rx, 0, sizeof(msg_rx));
	msg.cmd_type = cmd_id;
	msg.cmd_subid = cmd_subid;
	if (len > 0 && len <= PHYTMAC_MSG_PARA_LEN) {
		memcpy(&msg.para[0], data, len);
	} else if (len > PHYTMAC_MSG_PARA_LEN) {
		netdev_err(pdata->ndev, "Tx msg para len %d is greater than the max len %d",
			   len, PHYTMAC_MSG_PARA_LEN);
		ret = -EINVAL;
		goto err_out;
	}

	if (netif_msg_hw(pdata)) {
		netdev_info(pdata->ndev, "Tx msg: cmdid:%d, subid:%d, status0:%d, len:%d, tail:%d",
			    msg.cmd_type, msg.cmd_subid, msg.status0, len, tx_tail);
	}

	memcpy_toio(pdata->msg_regs + PHYTMAC_MSG(tx_tail), &msg, sizeof(msg));
	tx_tail = phytmac_v2_tx_ring_wrap(pdata, ++tx_tail);
	PHYTMAC_WRITE(pdata, PHYTMAC_TX_MSG_TAIL, tx_tail | PHYTMAC_BIT(TX_MSG_INT));
	pdata->msg_ring.tx_msg_wr_tail = tx_tail;

	if (wait) {
		tx_head = PHYTMAC_READ(pdata, PHYTMAC_TX_MSG_HEAD) & 0xff;
		while (tx_head != tx_tail) {
			udelay(1);
			tx_head = PHYTMAC_READ(pdata, PHYTMAC_TX_MSG_HEAD) & 0xff;
			retry++;
			if (retry >= PHYTMAC_RETRY_TIMES) {
				netdev_err(pdata->ndev, "Msg process time out!");
				ret = -EINVAL;
				goto err_out;
			}
		}

		memcpy_fromio(&msg_rx, pdata->msg_regs +
			      PHYTMAC_MSG(pdata->msg_ring.tx_msg_rd_tail),
			      PHYTMAC_MSG_HDR_LEN);
		if (!(msg_rx.status0 & PHYTMAC_CMD_PRC_SUCCESS)) {
			netdev_err(pdata->ndev, "Msg process error, cmdid:%d, subid:%d, status0:%d, tail:%d",
				   msg.cmd_type, msg.cmd_subid, msg.status0, tx_tail);
			ret = -EINVAL;
			goto err_out;
		}
	}

err_out:
	spin_unlock_irqrestore(&pdata->msg_ring.msg_lock, flags);
	return ret;
}

static void phytmac_v2_reset_hw(struct phytmac *pdata)
{
	int q;
	u16 cmd_id, cmd_subid;
	struct phytmac_ring_info ring;

	/* Disable and clear all interrupts and disable queues */
	for (q = 0; q < pdata->queues_max_num; ++q) {
		PHYTMAC_WRITE(pdata, PHYTMAC_INT_DR(q), -1);
		PHYTMAC_WRITE(pdata, PHYTMAC_INT_SR(q), -1);
		PHYTMAC_WRITE(pdata, PHYTMAC_TX_PTR(q), 0);
		PHYTMAC_WRITE(pdata, PHYTMAC_RX_PTR(q), 0);
	}

	/* reset hw rx/tx enable */
	cmd_id = PHYTMAC_MSG_CMD_DEFAULT;
	cmd_subid = PHYTMAC_MSG_CMD_DEFAULT_RESET_HW;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);

	/* reset tx ring */
	memset(&ring, 0, sizeof(ring));
	ring.queue_num = pdata->queues_max_num;
	cmd_subid = PHYTMAC_MSG_CMD_DEFAULT_RESET_TX_QUEUE;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&ring), sizeof(ring), 0);

	/* reset rx ring */
	cmd_subid = PHYTMAC_MSG_CMD_DEFAULT_RESET_RX_QUEUE;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&ring), sizeof(ring), 1);
}

static int phytmac_v2_get_mac_addr(struct phytmac *pdata, u8 *addr)
{
	int index;
	u16 cmd_id, cmd_subid;
	struct phytmac_mac para;

	cmd_id = PHYTMAC_MSG_CMD_GET;
	cmd_subid = PHYTMAC_MSG_CMD_GET_ADDR;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	index = phytmac_v2_tx_ring_wrap(pdata, pdata->msg_ring.tx_msg_rd_tail);
	memcpy_fromio(&para, pdata->msg_regs + PHYTMAC_MSG(index) + PHYTMAC_MSG_HDR_LEN,
		      sizeof(struct phytmac_mac));

	addr[0] = para.addrl & 0xff;
	addr[1] = (para.addrl >> 8) & 0xff;
	addr[2] = (para.addrl >> 16) & 0xff;
	addr[3] = (para.addrl >> 24) & 0xff;
	addr[4] = para.addrh & 0xff;
	addr[5] = (para.addrh >> 8) & 0xff;

	return 0;
}

static int phytmac_v2_set_mac_addr(struct phytmac *pdata, const u8 *addr)
{
	u16 cmd_id;
	u16 cmd_subid;
	struct phytmac_mac para;

	memset(&para, 0, sizeof(para));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_ADDR;
	para.addrl = cpu_to_le32(*((u32 *)addr));
	para.addrh = cpu_to_le16(*((u16 *)(addr + 4)));

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

	return 0;
}

static int phytmac_v2_pcs_software_reset(struct phytmac *pdata, int reset)
{
	u16 cmd_id;
	u16 cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (reset)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_PCS_RESET;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_PCS_RESET;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_init_hw(struct phytmac *pdata)
{
	u16 cmd_id, cmd_subid;
	struct phytmac_dma_info dma;
	struct phytmac_eth_info eth;
	u32 ptrconfig = 0;

	if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
		ptrconfig |= PHYTMAC_BIT(TXTAIL_EN);
	if (pdata->capacities & PHYTMAC_CAPS_RXPTR)
		ptrconfig |= PHYTMAC_BIT(RXTAIL_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_TAILPTR_ENABLE, ptrconfig);

	if (pdata->mii_bus) {
		cmd_id = PHYTMAC_MSG_CMD_SET;
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_MDIO;
		phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);
	}

	phytmac_v2_set_mac_addr(pdata, pdata->ndev->dev_addr);

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (pdata->capacities & PHYTMAC_CAPS_JUMBO)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_JUMBO;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_1536_FRAMES;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);

	if (pdata->ndev->flags & IFF_PROMISC) {
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_PROMISE;
		phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);
	}

	if (pdata->ndev->features & NETIF_F_RXCSUM) {
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_RXCSUM;
		phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);
	}

	if (pdata->ndev->flags & IFF_BROADCAST)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_BC;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_BC;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);

	cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_PAUSE;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);

	cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_STRIPCRC;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 0);

	memset(&dma, 0, sizeof(dma));
	cmd_subid = PHYTMAC_MSG_CMD_SET_DMA;
	dma.dma_burst_length = pdata->dma_burst_length;
	if (pdata->dma_addr_width)
		dma.hw_dma_cap |= HW_DMA_CAP_64B;
	if (pdata->ndev->features & NETIF_F_HW_CSUM)
		dma.hw_dma_cap |= HW_DMA_CAP_CSUM;
	if (IS_REACHABLE(CONFIG_PHYTMAC_ENABLE_PTP))
		dma.hw_dma_cap |= HW_DMA_CAP_PTP;
	if (pdata->dma_data_width == PHYTMAC_DBW32)
		dma.hw_dma_cap |= HW_DMA_CAP_DDW32;
	if (pdata->dma_data_width == PHYTMAC_DBW64)
		dma.hw_dma_cap |= HW_DMA_CAP_DDW64;
	if (pdata->dma_data_width == PHYTMAC_DBW128)
		dma.hw_dma_cap |= HW_DMA_CAP_DDW128;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)&dma, sizeof(dma), 0);

	memset(&eth, 0, sizeof(eth));
	cmd_subid = PHYTMAC_MSG_CMD_SET_ETH_MATCH;
	eth.index = 0;
	eth.etype = (uint16_t)ETH_P_IP;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)&eth, sizeof(eth), 1);

	if (phy_interface_mode_is_8023z(pdata->phy_interface))
		phytmac_v2_pcs_software_reset(pdata, 1);

	return 0;
}

static int phytmac_v2_enable_multicast(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_MC;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_MC;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);
	return 0;
}

static int phytmac_v2_set_mc_hash(struct phytmac *pdata, unsigned long *mc_filter)
{
	u16 cmd_id, cmd_subid;
	struct phytmac_mc_info para;

	memset(&para, 0, sizeof(para));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_HASH_MC;
	para.mc_bottom = (u32)mc_filter[0];
	para.mc_top = (u32)mc_filter[1];
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

	return 0;
}

static int phytmac_v2_init_ring_hw(struct phytmac *pdata)
{
	u16 cmd_id, cmd_subid;
	struct phytmac_ring_info rxring;
	struct phytmac_ring_info txring;
	struct phytmac_rxbuf_info rxbuf;
	struct phytmac_queue *queue;
	u32 q;

	memset(&rxring, 0, sizeof(rxring));
	memset(&txring, 0, sizeof(txring));
	memset(&rxbuf, 0, sizeof(rxbuf));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_INIT_TX_RING;
	txring.queue_num = pdata->queues_num;
	rxring.queue_num = pdata->queues_num;
	txring.hw_dma_cap |= HW_DMA_CAP_64B;
	rxring.hw_dma_cap |= HW_DMA_CAP_64B;
	for (q = 0, queue = pdata->queues; q < pdata->queues_num; ++q, ++queue) {
		PHYTMAC_WRITE(pdata, PHYTMAC_TX_PTR(q), queue->tx_head);
		txring.addr[q] = queue->tx_ring_addr;
		rxring.addr[q] = queue->rx_ring_addr;
	}

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&txring), sizeof(txring), 0);

	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_DMA_RX_BUFSIZE;
	rxbuf.queue_num = pdata->queues_num;
	rxbuf.buffer_size = pdata->rx_buffer_len / 64;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&rxbuf), sizeof(rxbuf), 0);

	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_INIT_RX_RING;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&rxring), sizeof(rxring), 1);

	return 0;
}

static int phytmac_v2_init_msg_ring(struct phytmac *pdata)
{
	u32 tx_msg_tail;

	pdata->msg_ring.tx_msg_ring_size = PHYTMAC_READ_BITS(pdata, PHYTMAC_SIZE, TXRING_SIZE);
	tx_msg_tail = PHYTMAC_READ(pdata, PHYTMAC_TX_MSG_TAIL) & 0xff;
	pdata->msg_ring.tx_msg_wr_tail = phytmac_v2_tx_ring_wrap(pdata, tx_msg_tail);
	pdata->msg_ring.tx_msg_rd_tail = pdata->msg_ring.tx_msg_wr_tail;
	PHYTMAC_WRITE(pdata, PHYTMAC_MSG_IMR, 0xfffffffe);
	if (netif_msg_hw(pdata))
		netdev_info(pdata->ndev, "Msg ring size:%d, tx msg tail=%d\n",
			    pdata->msg_ring.tx_msg_ring_size, tx_msg_tail);

	return 0;
}

static int phytmac_v2_get_feature_all(struct phytmac *pdata)
{
	u16 cmd_id, cmd_subid;
	int index;
	struct phytmac_feature para;

	memset(&para, 0, sizeof(para));
	cmd_id = PHYTMAC_MSG_CMD_GET;
	cmd_subid = PHYTMAC_MSG_CMD_GET_CAPS;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);
	index = phytmac_v2_tx_ring_wrap(pdata, pdata->msg_ring.tx_msg_rd_tail);
	memcpy_fromio(&para, pdata->msg_regs + PHYTMAC_MSG(index) + PHYTMAC_MSG_HDR_LEN,
		      sizeof(struct phytmac_feature));

	pdata->queues_max_num = para.queue_num;
	if (para.dma_addr_width)
		pdata->dma_addr_width = 64;
	else
		pdata->dma_addr_width = 32;
	pdata->dma_data_width = para.dma_data_width;
	pdata->max_rx_fs = para.max_rx_fs;

	if (para.tx_bd_prefetch)
		pdata->tx_bd_prefetch = (2 << (para.tx_bd_prefetch - 1)) *
					sizeof(struct phytmac_dma_desc);
	if (para.rx_bd_prefetch)
		pdata->rx_bd_prefetch = (2 << (para.rx_bd_prefetch - 1)) *
					sizeof(struct phytmac_dma_desc);

	if (netif_msg_hw(pdata)) {
		netdev_info(pdata->ndev, "feature qnum=%d, daw=%d, dbw=%d, rxfs=%d, rxbd=%d, txbd=%d\n",
			    pdata->queues_num, pdata->dma_addr_width, pdata->dma_data_width,
			    pdata->max_rx_fs, pdata->rx_bd_prefetch, pdata->tx_bd_prefetch);
	}

	return 0;
}

static void phytmac_v2_get_regs(struct phytmac *pdata, u32 *reg_buff)
{
	u16 cmd_id, cmd_subid;
	int index;
	struct phytmac_ethtool_reg msg;

	memset(&msg, 0, sizeof(msg));
	cmd_id = PHYTMAC_MSG_CMD_GET;
	cmd_subid = PHYTMAC_MSG_CMD_GET_REGS_FOR_ETHTOOL;
	msg.interface = pdata->phytmac_v2_interface;
	/* There are 16 regs in total, read 14 regs at first time, read 2 regs at last time */
	msg.cnt = 0;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&msg), sizeof(msg), 1);
	index = phytmac_v2_tx_ring_wrap(pdata, pdata->msg_ring.tx_msg_rd_tail);
	memcpy_fromio(reg_buff, pdata->msg_regs + PHYTMAC_MSG(index) + PHYTMAC_MSG_HDR_LEN,
		      PHYTMAC_MSG_PARA_LEN);

	msg.cnt = 1;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&msg), sizeof(msg), 1);
	index = phytmac_v2_tx_ring_wrap(pdata, pdata->msg_ring.tx_msg_rd_tail);
	memcpy_fromio(reg_buff + PHYTMAC_MSG_PARA_LEN / sizeof(u32), pdata->msg_regs +
		      PHYTMAC_MSG(index) + PHYTMAC_MSG_HDR_LEN,
		      PHYTMAC_ETHTOOLD_REGS_LEN - PHYTMAC_MSG_PARA_LEN);
}

static void phytmac_v2_get_hw_stats(struct phytmac *pdata)
{
	u32 stats[PHYTMAC_STATIS_REG_NUM];
	int i, j;
	u64 val;
	u64 *p = &pdata->stats.tx_octets;

	for (i = 0 ; i < PHYTMAC_STATIS_REG_NUM; i++)
		stats[i] = PHYTMAC_READ(pdata, PHYTMAC_OCT_TX + i * 4);

	for (i = 0, j = 0; i < PHYTMAC_STATIS_REG_NUM; i++) {
		if (i == 0 || i == 20) {
			val = (u64)stats[i + 1] << 32 | stats[i];
			*p += val;
			pdata->ethtool_stats[j] = *p;
			++j;
			++p;
		} else {
			if (i != 1 && i != 21) {
				val = stats[i];
				*p += val;
				pdata->ethtool_stats[j] = *p;
				++j;
				++p;
			}
		}
	}
}

static int phytmac_v2_mdio_idle(struct phytmac *pdata)
{
	u32 val;
	int ret;

	/* wait for end of transfer */
	ret = readx_poll_timeout(PHYTMAC_READ_NSR, pdata, val, val & PHYTMAC_BIT(MIDLE),
				 1, PHYTMAC_MDIO_TIMEOUT);
	if (ret)
		netdev_err(pdata->ndev, "mdio wait for idle time out!");

	return ret;
}

static int phytmac_v2_mdio_data_read_c22(struct phytmac *pdata, int mii_id, int regnum)
{
	u16 data;

	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C22)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C22_READ)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, regnum)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);
	data = PHYTMAC_READ(pdata, PHYTMAC_MDIO) & 0xffff;
	phytmac_v2_mdio_idle(pdata);
	return data;
}

static int phytmac_v2_mdio_data_write_c22(struct phytmac *pdata, int mii_id,
				   int regnum, u16 data)
{
	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C22)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C22_WRITE)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, regnum)
		      | PHYTMAC_BITS(VALUE, data)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);

	return 0;
}

static int phytmac_v2_mdio_data_read_c45(struct phytmac *pdata, int mii_id, int devad, int regnum)
{
	u16 data;

	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C45)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C45_ADDR)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, devad & 0x1F)
		      | PHYTMAC_BITS(VALUE, regnum & 0xFFFF)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);
	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C45)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C45_READ)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, devad & 0x1F)
		      | PHYTMAC_BITS(VALUE, regnum & 0xFFFF)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);
	data = PHYTMAC_READ(pdata, PHYTMAC_MDIO) & 0xffff;
	phytmac_v2_mdio_idle(pdata);
	return data;
}

static int phytmac_v2_mdio_data_write_c45(struct phytmac *pdata, int mii_id, int devad,
				   int regnum, u16 data)
{
	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C45)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C45_ADDR)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, (regnum >> 16) & 0x1F)
		      | PHYTMAC_BITS(VALUE, regnum & 0xFFFF)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);
	PHYTMAC_WRITE(pdata, PHYTMAC_MDIO, (PHYTMAC_BITS(CLAUSESEL, PHYTMAC_C45)
		      | PHYTMAC_BITS(MDCOPS, PHYTMAC_C45_WRITE)
		      | PHYTMAC_BITS(PHYADDR, mii_id)
		      | PHYTMAC_BITS(REGADDR, (regnum >> 16) & 0x1F)
		      | PHYTMAC_BITS(VALUE, data)
		      | PHYTMAC_BITS(CONST, 2)));
	phytmac_v2_mdio_idle(pdata);

	return 0;
}

static int phytmac_v2_powerup_hw(struct phytmac *pdata, int on)
{
	u32 status, data0, data1, rdata1;
	int ret;

	if (!(pdata->capacities & PHYTMAC_CAPS_PWCTRL)) {
		pdata->power_state = on;
		return 0;
	}

	if (has_acpi_companion(pdata->dev)) {
#ifdef CONFIG_ACPI
		acpi_handle handle;
		union acpi_object args[3];
		struct acpi_object_list arg_list = {
			.pointer = args,
			.count = ARRAY_SIZE(args),
		};
		acpi_status acpi_sts;
		unsigned long long rv;

		handle = ACPI_HANDLE(pdata->dev);

		netdev_info(pdata->ndev, "set gmac power %s\n",
			on == PHYTMAC_POWERON ? "on" : "off");
		args[0].type = ACPI_TYPE_INTEGER;
		args[0].integer.value = PHYTMAC_PWCTL_GMAC_ID;
		args[1].type = ACPI_TYPE_INTEGER;
		args[1].integer.value = PHYTMAC_PWCTL_DEFAULT_VAL;
		args[2].type = ACPI_TYPE_INTEGER;
		args[2].integer.value = PHYTMAC_PWCTL_DEFAULT_VAL;

		if (on == PHYTMAC_POWERON) {
			acpi_sts = acpi_evaluate_integer(handle, "PPWO", &arg_list, &rv);
			if (ACPI_FAILURE(acpi_sts))
				netdev_err(pdata->ndev, "NO PPWO Method\n");
			if (rv)
				netdev_err(pdata->ndev, "Failed to power on\n");
		} else {
			acpi_sts = acpi_evaluate_integer(handle, "PPWD", &arg_list, &rv);
			if (ACPI_FAILURE(acpi_sts))
				netdev_err(pdata->ndev, "NO PPWD Method\n");
			if (rv)
				netdev_err(pdata->ndev, "Failed to power off\n");
		}
#endif
	} else {
		ret = readx_poll_timeout(PHYTMAC_READ_STAT, pdata, status, !status,
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh status is busy");

		ret = readx_poll_timeout(PHYTMAC_READ_DATA0, pdata, data0,
					 data0 & PHYTMAC_BIT(DATA0_FREE),
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh data0 is busy");

		data0 = 0;
		data0 = PHYTMAC_SET_BITS(data0, DATA0_MSG, PHYTMAC_MSG_PM);
		data0 = PHYTMAC_SET_BITS(data0, DATA0_PRO, PHYTMAC_PRO_ID);
		PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA0, data0);
		data1 = 0;

		if (on == PHYTMAC_POWERON) {
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STAT, PHYTMAC_STATON);
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STATTYPE, PHYTMAC_STATTYPE);
			PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA1, data1);
		} else {
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STAT, PHYTMAC_STATOFF);
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STATTYPE, PHYTMAC_STATTYPE);
			PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA1, data1);
		}

		PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_AP_CPP_SET, 1);
		ret = readx_poll_timeout(PHYTMAC_READ_DATA0, pdata, data0,
					 data0 & PHYTMAC_BIT(DATA0_FREE),
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh data0 is busy\n");

		rdata1 = PHYTMAC_MHU_READ(pdata, PHYTMAC_MHU_CPP_DATA1);
		if (rdata1 == data1)
			netdev_err(pdata->ndev, "gmac power %s success, data1 = %x, rdata1=%x\n",
					   on == PHYTMAC_POWERON ? "up" : "down", data1, rdata1);
		else
			netdev_err(pdata->ndev, "gmac power %s failed, data1 = %x, rdata1=%x\n",
					   on == PHYTMAC_POWERON ? "up" : "down", data1, rdata1);
	}

	pdata->power_state = on;

	return 0;
}

static int phytmac_v2_set_wake(struct phytmac *pdata, int wake)
{
	u16 cmd_id, cmd_subid;
	struct phytmac_wol para;
	u32 wol_type = 0;

	if (wake & PHYTMAC_WAKE_MAGIC)
		wol_type |= PHYTMAC_BIT(MAGIC);
	if (wake & PHYTMAC_WAKE_ARP)
		wol_type |= PHYTMAC_BIT(ARP);
	if (wake & PHYTMAC_WAKE_UCAST)
		wol_type |= PHYTMAC_BIT(UCAST);
	if (wake & PHYTMAC_WAKE_MCAST)
		wol_type |= PHYTMAC_BIT(MCAST);

	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_WOL;
	memset(&para, 0, sizeof(para));
	para.wol_type = cpu_to_le32(wol_type);
	para.wake = (u8)wake;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

	return 0;
}

static int phytmac_v2_enable_promise(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;
	u8 rxcsum = 0;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable) {
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_PROMISE;
	} else {
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_PROMISE;
		if (pdata->ndev->features & NETIF_F_RXCSUM)
			rxcsum = 1;
	}

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&rxcsum), 1, 1);

	return 0;
}

static int phytmac_v2_enable_rxcsum(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_RXCSUM;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_RXCSUM;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_enable_txcsum(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_TXCSUM;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_TXCSUM;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_enable_mdio(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;
	u8 mdc;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_MDC;
	mdc = PHYTMAC_CLK_DIV96;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&mdc), sizeof(mdc), 0);

	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_MDIO;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_MDIO;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_enable_autoneg(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_AUTONEG;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_AUTONEG;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	pdata->autoneg = enable;
	return 0;
}

static int phytmac_v2_enable_pause(struct phytmac *pdata, int enable)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_PAUSE;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_PAUSE;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_enable_network(struct phytmac *pdata, int enable, int rx_tx)
{
	u16 cmd_id, cmd_subid;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	if (enable)
		cmd_subid = PHYTMAC_MSG_CMD_SET_ENABLE_NETWORK;
	else
		cmd_subid = PHYTMAC_MSG_CMD_SET_DISABLE_NETWORK;

	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);

	return 0;
}

static int phytmac_v2_add_fdir_entry(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow)
{
	struct ethtool_tcpip4_spec *tp4sp_v, *tp4sp_m;
	struct phytmac_fdir_info fdir;
	u16 cmd_id, cmd_subid;

	memset(&fdir, 0, sizeof(fdir));

	tp4sp_v = &rx_flow->h_u.tcp_ip4_spec;
	tp4sp_m = &rx_flow->m_u.tcp_ip4_spec;
	if (tp4sp_m->ip4src == 0xFFFFFFFF) {
		fdir.ipsrc_en = true;
		fdir.ip4src = tp4sp_v->ip4src;
	}

	if (tp4sp_m->ip4dst == 0xFFFFFFFF) {
		fdir.ipdst_en = true;
		fdir.ip4dst = tp4sp_v->ip4dst;
	}

	if (tp4sp_m->psrc == 0xFFFF || tp4sp_m->pdst == 0xFFFF) {
		fdir.port_en = true;
		fdir.dstport = tp4sp_v->pdst;
		fdir.srcport = tp4sp_v->psrc;
		fdir.dstport_mask = tp4sp_m->pdst;
		fdir.srcport_mask = tp4sp_m->psrc;
	}

	fdir.location = (u8)(rx_flow->location);
	fdir.queue = (u8)(rx_flow->ring_cookie);

	if (fdir.ipsrc_en || fdir.ipdst_en || fdir.port_en) {
		cmd_id = PHYTMAC_MSG_CMD_SET;
		cmd_subid = PHYTMAC_MSG_CMD_SET_ADD_FDIR;
		phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&fdir), sizeof(fdir), 1);
	}

	return 0;
}

static int phytmac_v2_del_fdir_entry(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow)
{
	struct phytmac_fdir_info fdir;
	u16 cmd_id, cmd_subid;

	memset(&fdir, 0, sizeof(fdir));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_DEL_FDIR;
	fdir.location = (u8)(rx_flow->location);
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&fdir), sizeof(fdir), 1);

	return 0;
}

static void phytmac_v2_tx_start(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;

	PHYTMAC_WRITE(pdata, PHYTMAC_TX_PTR(queue->index), queue->tx_tail);
	queue->tx_xmit_more = 0;
}

static u32 phytmac_v2_get_irq_mask(u32 mask)
{
	u32 value = 0;

	value |= (mask & PHYTMAC_INT_TX_COMPLETE) ? PHYTMAC_BIT(TXCOMP) : 0;
	value |= (mask & PHYTMAC_INT_TX_ERR) ? PHYTMAC_BIT(DMA_ERR) : 0;
	value |= (mask & PHYTMAC_INT_RX_COMPLETE) ? PHYTMAC_BIT(RXCOMP) : 0;
	value |= (mask & PHYTMAC_INT_RX_OVERRUN) ? PHYTMAC_BIT(RXOVERRUN) : 0;
	value |= (mask & PHYTMAC_INT_RX_DESC_FULL) ? PHYTMAC_BIT(RUSED) : 0;

	return value;
}

static u32 phytmac_v2_get_irq_status(u32 value)
{
	u32 status = 0;

	status |= (value & PHYTMAC_BIT(TXCOMP)) ? PHYTMAC_INT_TX_COMPLETE : 0;
	status |= (value & PHYTMAC_BIT(DMA_ERR)) ? PHYTMAC_INT_TX_ERR : 0;
	status |= (value & PHYTMAC_BIT(RXCOMP)) ? PHYTMAC_INT_RX_COMPLETE : 0;
	status |= (value & PHYTMAC_BIT(RXOVERRUN)) ? PHYTMAC_INT_RX_OVERRUN : 0;
	status |= (value & PHYTMAC_BIT(RUSED)) ? PHYTMAC_INT_RX_DESC_FULL : 0;

	return status;
}

static void phytmac_v2_enable_irq(struct phytmac *pdata,
			       int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_v2_get_irq_mask(mask);
	PHYTMAC_WRITE(pdata, PHYTMAC_INT_ER(queue_index), value);
}

static void phytmac_v2_disable_irq(struct phytmac *pdata,
				int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_v2_get_irq_mask(mask);
	PHYTMAC_WRITE(pdata, PHYTMAC_INT_DR(queue_index), value);
}

static void phytmac_v2_clear_irq(struct phytmac *pdata,
			      int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_v2_get_irq_mask(mask);
	PHYTMAC_WRITE(pdata, PHYTMAC_INT_SR(queue_index), value);
}

static unsigned int phytmac_v2_get_irq(struct phytmac *pdata, int queue_index)
{
	u32 status;
	u32 value;

	value = PHYTMAC_READ(pdata, PHYTMAC_INT_SR(queue_index));
	status = phytmac_v2_get_irq_status(value);

	return status;
}

static void phytmac_v2_interface_config(struct phytmac *pdata, unsigned int mode,
				     const struct phylink_link_state *state)
{
	struct phytmac_interface_info para;
	u16 cmd_id, cmd_subid;
	u8 autoneg = 0;

	if (state->interface == PHY_INTERFACE_MODE_SGMII) {
		if (mode == MLO_AN_FIXED)
			autoneg = 0;
		else
			autoneg = 1;
	}

	if (state->interface == PHY_INTERFACE_MODE_1000BASEX)
		autoneg = 1;
	if (state->interface == PHY_INTERFACE_MODE_2500BASEX)
		autoneg = 0;

	memset(&para, 0, sizeof(para));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_INIT_MAC_CONFIG;
	para.interface = pdata->phytmac_v2_interface;
	para.autoneg = autoneg;
	para.speed = state->speed;
	para.duplex = state->duplex;
	pdata->autoneg = para.autoneg;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);
}

static int phytmac_v2_interface_linkup(struct phytmac *pdata, phy_interface_t interface,
				    int speed, int duplex)
{
	struct phytmac_interface_info para;
	u16 cmd_id, cmd_subid;

	if (interface == PHY_INTERFACE_MODE_SGMII) {
		if (speed == SPEED_2500)
			pdata->autoneg = 0;
		else
			pdata->autoneg = 1;
	}

	memset(&para, 0, sizeof(para));
	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_MAC_LINK_CONFIG;
	para.interface = pdata->phytmac_v2_interface;
	para.duplex = duplex;
	para.speed = speed;
	para.autoneg = pdata->autoneg;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

	return 0;
}

static int phytmac_v2_interface_linkdown(struct phytmac *pdata)
{
	return 0;
}

static int phytmac_v2_pcs_linkup(struct phytmac *pdata, phy_interface_t interface,
			      int speed, int duplex)
{
	u16 cmd_id, cmd_subid;

	if (interface == PHY_INTERFACE_MODE_USXGMII ||
	    interface == PHY_INTERFACE_MODE_10GBASER) {
		cmd_id = PHYTMAC_MSG_CMD_SET;
		cmd_subid = PHYTMAC_MSG_CMD_SET_PCS_LINK_UP;

		phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, NULL, 0, 1);
	}

	return 0;
}

static int phytmac_v2_pcs_linkdown(struct phytmac *pdata)
{
	return 0;
}

static unsigned int phytmac_v2_pcs_get_link(struct phytmac *pdata, phy_interface_t interface)
{
	if (interface == PHY_INTERFACE_MODE_SGMII ||
	    interface == PHY_INTERFACE_MODE_1000BASEX ||
	    interface == PHY_INTERFACE_MODE_2500BASEX)
		return PHYTMAC_READ_BITS(pdata, PHYTMAC_NETWORK_STATUS, LINK);
	else if (interface == PHY_INTERFACE_MODE_USXGMII ||
		 interface == PHY_INTERFACE_MODE_10GBASER)
		return PHYTMAC_READ_BITS(pdata, PHYTMAC_USX_LINK_STATUS, USX_LINK);

	return 0;
}

static unsigned int phytmac_v2_tx_map_desc(struct phytmac_queue *queue,
					 u32 tx_tail, struct packet_info *packet)
{
	unsigned int i, ctrl;
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc;
	struct phytmac_tx_skb *tx_skb;
	unsigned int eof = 1;

	i = tx_tail;

	do {
		i--;
		tx_skb = phytmac_get_tx_skb(queue, i);
		desc = phytmac_get_tx_desc(queue, i);

		ctrl = (u32)tx_skb->length;
		if (eof) {
			ctrl |= PHYTMAC_BIT(TXLAST);
			eof = 0;
		}

		if (unlikely(i == (pdata->tx_ring_size - 1)))
			ctrl |= PHYTMAC_BIT(TXWRAP);

		if (i == queue->tx_tail) {
			ctrl |= PHYTMAC_BITS(TXLSO, packet->lso);
			ctrl |= PHYTMAC_BITS(TXTCP_SEQ_SRC, packet->seq);
			if (packet->nocrc)
				ctrl |= PHYTMAC_BIT(TXNOCRC);
		} else {
			ctrl |= PHYTMAC_BITS(MSSMFS, packet->mss);
		}

		desc->desc2 = upper_32_bits(tx_skb->addr);
		desc->desc0 = lower_32_bits(tx_skb->addr);
		/* make newly desc1 to hardware */
		wmb();
		desc->desc1 = ctrl;
	} while (i != queue->tx_tail);

	return 0;
}

static void phytmac_v2_init_rx_map_desc(struct phytmac_queue *queue,
				     u32 index)
{
	struct phytmac_dma_desc *desc;

	desc = phytmac_get_rx_desc(queue, index);

	desc->desc1 = 0;
	/* Make newly descriptor to hardware */
	dma_wmb();
	desc->desc0 |= PHYTMAC_BIT(RXUSED);
}

static unsigned int phytmac_v2_rx_map_desc(struct phytmac_queue *queue, u32 index, dma_addr_t addr)
{
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc;

	desc = phytmac_get_rx_desc(queue, index);

	if (addr) {
		if (unlikely(index == (pdata->rx_ring_size - 1)))
			addr |= PHYTMAC_BIT(RXWRAP);
		desc->desc1 = 0;
		desc->desc2 = upper_32_bits(addr);
		/* Make newly descriptor to hardware */
		if (!(pdata->capacities & PHYTMAC_CAPS_RXPTR))
			dma_wmb();
		desc->desc0 = lower_32_bits(addr);
	} else {
		desc->desc1 = 0;
		/* make newly descriptor to hardware */
		if (!(pdata->capacities & PHYTMAC_CAPS_RXPTR))
			dma_wmb();
		desc->desc0 &= ~PHYTMAC_BIT(RXUSED);
	}

	return 0;
}

static unsigned int phytmac_v2_zero_rx_desc_addr(struct phytmac_dma_desc *desc)
{
	desc->desc2 = 0;
	desc->desc0 = (desc->desc0 & PHYTMAC_BIT(RXTSVALID)) | PHYTMAC_BIT(RXUSED);

	return 0;
}

static unsigned int phytmac_v2_zero_tx_desc(struct phytmac_dma_desc *desc)
{
	desc->desc2 = 0;
	desc->desc0 = 0;
	desc->desc1 &= ~PHYTMAC_BIT(TXUSED);

	return 0;
}

static void phytmac_v2_update_rx_tail(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;

	if (pdata->capacities & PHYTMAC_CAPS_RXPTR)
		PHYTMAC_WRITE(pdata, PHYTMAC_RX_PTR(queue->index), queue->rx_head);
}

static int phytmac_v2_tx_complete(const struct phytmac_dma_desc *desc)
{
	return PHYTMAC_GET_BITS(desc->desc1, TXUSED);
}

static bool phytmac_v2_rx_complete(const struct phytmac_dma_desc *desc)
{
	dma_addr_t addr;
	bool used;

	used = PHYTMAC_GET_BITS(desc->desc0, RXUSED);
	addr = ((u64)(desc->desc2) << 32);
	addr |= desc->desc0 & 0xfffffff8;

	if (used != 0 && addr != 0)
		return true;
	else
		return false;
}

static int phytmac_v2_rx_pkt_len(struct phytmac *pdata, const struct phytmac_dma_desc *desc)
{
	if (pdata->capacities & PHYTMAC_CAPS_JUMBO)
		return desc->desc1 & PHYTMAC_RXJFRMLEN_MASK;
	else
		return desc->desc1 & PHYTMAC_RXFRMLEN_MASK;
}

static bool phytmac_v2_rx_checksum(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;
	u32 check = value >> PHYTMAC_RXCSUM_INDEX & 0x3;

	return (check == PHYTMAC_RXCSUM_IP_TCP || check == PHYTMAC_RXCSUM_IP_UDP);
}

static bool phytmac_v2_rx_single_buffer(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return ((value & PHYTMAC_BIT(RXSOF)) && (value & PHYTMAC_BIT(RXEOF)));
}

static bool phytmac_v2_rx_sof(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return (value & PHYTMAC_BIT(RXSOF));
}

static bool phytmac_v2_rx_eof(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return (value & PHYTMAC_BIT(RXEOF));
}

static void phytmac_v2_clear_rx_desc(struct phytmac_queue *queue, int begin, int end)
{
	unsigned int frag;
	unsigned int tmp = end;
	struct phytmac_dma_desc *desc;

	if (begin > end)
		tmp = end + queue->pdata->rx_ring_size;

	for (frag = begin; frag != tmp; frag++) {
		desc = phytmac_get_rx_desc(queue, frag);
		desc->desc0 &= ~PHYTMAC_BIT(RXUSED);
	}
}

static void phytmac_v2_clear_tx_desc(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc = NULL;
	struct phytmac_tx_skb *tx_skb = NULL;
	int i;

	for (i = 0; i < queue->pdata->tx_ring_size; i++) {
		desc = phytmac_get_tx_desc(queue, i);
		tx_skb = phytmac_get_tx_skb(queue, i);
		desc->desc2 = upper_32_bits(tx_skb->addr);
		desc->desc0 = lower_32_bits(tx_skb->addr);
		/* make newly desc to hardware */
		wmb();
		desc->desc1 = PHYTMAC_BIT(TXUSED);
	}
	desc->desc1 |= PHYTMAC_BIT(TXWRAP);
	PHYTMAC_WRITE(pdata, PHYTMAC_TX_PTR(queue->index), queue->tx_tail);
}

static void phytmac_v2_get_time(struct phytmac *pdata, struct timespec64 *ts)
{
	u32 ns, secl, sech;

	ns = PHYTMAC_READ(pdata, PHYTMAC_TIMER_NSEC);
	secl = PHYTMAC_READ(pdata, PHYTMAC_TIMER_SEC);
	sech = PHYTMAC_READ(pdata, PHYTMAC_TIMER_MSB_SEC);

	ts->tv_nsec = ns;
	ts->tv_sec = (((u64)sech << 32) | secl) & TIMER_SEC_MAX_VAL;
}

static void phytmac_v2_set_time(struct phytmac *pdata, time64_t sec, long nsec)
{
	u32 secl, sech;

	secl = (u32)sec;
	sech = (sec >> 32) & (0xffff);

	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_NSEC, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_MSB_SEC, sech);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_SEC, secl);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_NSEC, nsec);
}

static void phytmac_v2_clear_time(struct phytmac *pdata)
{
	u32 value;

	pdata->ts_incr.sub_ns = 0;
	pdata->ts_incr.ns = 0;

	value = PHYTMAC_READ(pdata, PHYTMAC_TIMER_INCR);
	value = PHYTMAC_SET_BITS(value, INCR_NSEC, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_INCR, value);

	value = PHYTMAC_READ(pdata, PHYTMAC_TIMER_INCR_SUB_NSEC);
	value = PHYTMAC_SET_BITS(value, INCR_SNSEC, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_INCR_SUB_NSEC, value);

	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_ADJUST, 0);
}

static int phytmac_v2_set_tsmode(struct phytmac *pdata, struct ts_ctrl *ctrl)
{
	u16 cmd_id, cmd_subid;
	struct phytmac_ts_config para;

	cmd_id = PHYTMAC_MSG_CMD_SET;
	cmd_subid = PHYTMAC_MSG_CMD_SET_TS_CONFIG;
	para.tx_mode = ctrl->tx_control;
	para.rx_mode = ctrl->rx_control;
	para.one_step = ctrl->one_step;
	phytmac_v2_msg_send(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

	return 0;
}

static int phytmac_v2_set_tsincr(struct phytmac *pdata, struct ts_incr *incr)
{
	u32 value;

	value = PHYTMAC_BITS(INCR_SNSEC, incr->sub_ns);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_INCR_SUB_NSEC, value);
	PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_INCR, incr->ns);

	return 0;
}

static void phytmac_v2_ptp_init_hw(struct phytmac *pdata)
{
	struct timespec64 ts;

	ts = ns_to_timespec64(ktime_to_ns(ktime_get_real()));
	phytmac_v2_set_time(pdata, ts.tv_sec, ts.tv_nsec);

	phytmac_v2_set_tsincr(pdata, &pdata->ts_incr);
}

static int phytmac_v2_adjust_fine(struct phytmac *pdata, long ppm, bool negative)
{
	struct ts_incr ts_incr;
	u32 tmp;
	u64 adj;

	ts_incr.ns = pdata->ts_incr.ns;
	ts_incr.sub_ns = pdata->ts_incr.sub_ns;

	/* scaling: unused(8bit) | ns(8bit) | fractions(16bit) */
	tmp = ((u64)ts_incr.ns << PHYTMAC_INCR_SNSECL_INDEX) + ts_incr.sub_ns;
	adj = ((u64)ppm * tmp + (USEC_PER_SEC >> 1)) >> PHYTMAC_INCR_SNSECL_INDEX;

	adj = div_u64(adj, USEC_PER_SEC);
	adj = negative ? (tmp - adj) : (tmp + adj);

	ts_incr.ns = (adj >> PHYTMAC_INCR_SNSEC_WIDTH)
			& ((1 << PHYTMAC_INCR_NSEC_WIDTH) - 1);
	ts_incr.sub_ns = adj & ((1 << PHYTMAC_INCR_SNSEC_WIDTH) - 1);

	phytmac_v2_set_tsincr(pdata, &ts_incr);

	return 0;
}

static int phytmac_v2_adjust_time(struct phytmac *pdata, s64 delta, int neg)
{
	u32 adj;

	if (delta > PHYTMAC_ASEC_MAX) {
		struct timespec64 now, then;

		if (neg)
			then = ns_to_timespec64(-delta);
		else
			then = ns_to_timespec64(delta);
		phytmac_v2_get_time(pdata, &now);
		now = timespec64_add(now, then);
		phytmac_v2_set_time(pdata, now.tv_sec, now.tv_nsec);
	} else {
		adj = (neg << PHYTMAC_AADD_INDEX) | delta;
		PHYTMAC_WRITE(pdata, PHYTMAC_TIMER_ADJUST, adj);
	}

	return 0;
}

static int phytmac_v2_ts_valid(struct phytmac *pdata, struct phytmac_dma_desc *desc, int direction)
{
	int ts_valid = 0;

	if (direction == PHYTMAC_TX)
		ts_valid = desc->desc1 & PHYTMAC_BIT(TXTSVALID);
	else if (direction == PHYTMAC_RX)
		ts_valid = desc->desc0 & PHYTMAC_BIT(RXTSVALID);
	return ts_valid;
}

static void phytmac_v2_get_dma_ts(struct phytmac *pdata, u32 ts_1, u32 ts_2, struct timespec64 *ts)
{
	struct timespec64 ts2;

	ts->tv_sec = (PHYTMAC_GET_BITS(ts_2, TS_SECH) << 2) |
				PHYTMAC_GET_BITS(ts_1, TS_SECL);
	ts->tv_nsec = PHYTMAC_GET_BITS(ts_1, TS_NSEC);

	phytmac_v2_get_time(pdata, &ts2);

	if (((ts->tv_sec ^ ts2.tv_sec) & (PHYTMAC_TS_SEC_TOP >> 1)) != 0)
		ts->tv_sec -= PHYTMAC_TS_SEC_TOP;

	ts->tv_sec += (ts2.tv_sec & (~PHYTMAC_TS_SEC_MASK));
}

static unsigned int phytmac_v2_get_ts_rate(struct phytmac *pdata)
{
	return 300000000;
}

struct phytmac_hw_if phytmac_2p0_hw = {
	.init_msg_ring = phytmac_v2_init_msg_ring,
	.reset_hw = phytmac_v2_reset_hw,
	.init_hw = phytmac_v2_init_hw,
	.init_ring_hw = phytmac_v2_init_ring_hw,
	.get_feature = phytmac_v2_get_feature_all,
	.get_regs = phytmac_v2_get_regs,
	.get_stats = phytmac_v2_get_hw_stats,
	.set_mac_address = phytmac_v2_set_mac_addr,
	.get_mac_address = phytmac_v2_get_mac_addr,
	.mdio_idle = phytmac_v2_mdio_idle,
	.mdio_read = phytmac_v2_mdio_data_read_c22,
	.mdio_write = phytmac_v2_mdio_data_write_c22,
	.mdio_read_c45 = phytmac_v2_mdio_data_read_c45,
	.mdio_write_c45 = phytmac_v2_mdio_data_write_c45,
	.poweron = phytmac_v2_powerup_hw,
	.set_wol = phytmac_v2_set_wake,
	.enable_promise = phytmac_v2_enable_promise,
	.enable_multicast = phytmac_v2_enable_multicast,
	.set_hash_table = phytmac_v2_set_mc_hash,
	.enable_rx_csum = phytmac_v2_enable_rxcsum,
	.enable_tx_csum = phytmac_v2_enable_txcsum,
	.enable_mdio_control = phytmac_v2_enable_mdio,
	.enable_autoneg = phytmac_v2_enable_autoneg,
	.enable_pause = phytmac_v2_enable_pause,
	.enable_network = phytmac_v2_enable_network,
	.add_fdir_entry = phytmac_v2_add_fdir_entry,
	.del_fdir_entry = phytmac_v2_del_fdir_entry,

	/* mac config */
	.mac_config = phytmac_v2_interface_config,
	.mac_linkup = phytmac_v2_interface_linkup,
	.mac_linkdown = phytmac_v2_interface_linkdown,
	.pcs_linkup = phytmac_v2_pcs_linkup,
	.pcs_linkdown = phytmac_v2_pcs_linkdown,
	.get_link = phytmac_v2_pcs_get_link,

	/* irq */
	.enable_irq = phytmac_v2_enable_irq,
	.disable_irq = phytmac_v2_disable_irq,
	.clear_irq = phytmac_v2_clear_irq,
	.get_irq = phytmac_v2_get_irq,

	/* tx and rx */
	.tx_map = phytmac_v2_tx_map_desc,
	.transmit = phytmac_v2_tx_start,
	.update_rx_tail = phytmac_v2_update_rx_tail,
	.tx_complete = phytmac_v2_tx_complete,
	.rx_complete = phytmac_v2_rx_complete,
	.get_rx_pkt_len = phytmac_v2_rx_pkt_len,
	.init_rx_map = phytmac_v2_init_rx_map_desc,
	.rx_map = phytmac_v2_rx_map_desc,
	.rx_checksum = phytmac_v2_rx_checksum,
	.rx_single_buffer = phytmac_v2_rx_single_buffer,
	.rx_pkt_start = phytmac_v2_rx_sof,
	.rx_pkt_end = phytmac_v2_rx_eof,
	.clear_rx_desc = phytmac_v2_clear_rx_desc,
	.clear_tx_desc = phytmac_v2_clear_tx_desc,
	.zero_rx_desc_addr = phytmac_v2_zero_rx_desc_addr,
	.zero_tx_desc = phytmac_v2_zero_tx_desc,

	/* ptp */
	.init_ts_hw = phytmac_v2_ptp_init_hw,
	.set_time = phytmac_v2_set_time,
	.clear_time = phytmac_v2_clear_time,
	.get_time = phytmac_v2_get_time,
	.set_ts_config = phytmac_v2_set_tsmode,
	.set_incr = phytmac_v2_set_tsincr,
	.adjust_fine = phytmac_v2_adjust_fine,
	.adjust_time = phytmac_v2_adjust_time,
	.ts_valid = phytmac_v2_ts_valid,
	.get_timestamp = phytmac_v2_get_dma_ts,
	.get_ts_rate = phytmac_v2_get_ts_rate,
};
EXPORT_SYMBOL_GPL(phytmac_2p0_hw);
