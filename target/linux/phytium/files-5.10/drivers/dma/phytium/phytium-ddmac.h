/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Phytium Device DDMA Controller driver.
 *
 * Copyright (c) 2023 Phytium Technology Co., Ltd.
 */

#ifndef _PHYTIUM_DDMAC_H
#define _PHYTIUM_DDMAC_H

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/types.h>
#include "../virt-dma.h"

/* the number of physical channel */
#define DDMA_MAX_NR_PCHANNELS	8

#define DMAC_MAX_MASTERS	1
#define DMAC_MAX_BLK_SIZE	PAGE_SIZE

#define CHAN_REG_LEN		0x40
#define DMA_REG_LEN		0x40

#define DMA_CTL			0x00
#define DMA_CHAL_CFG_L		0x04
#define DMA_CHAL_CFG_H		0x28
#define DMA_STAT		0x08
#define DMA_MASK_INT		0x0C
#define DMA_CHAL_BIND		0x20
#define DMA_GCAP		0x24

#define DMA_CHALX_DDR_UPADDR	0x00
#define DMA_CHALX_DDR_LWADDR	0x04
#define DMA_CHALX_DEV_ADDR	0x08
#define DMA_CHALX_TS		0x0C
#define DMA_CHALX_CRT_UPADDR	0x10
#define DMA_CHALX_CRT_LWADDR	0x14
#define DMA_CHALX_CTL		0x18
#define DMA_CHALX_STS		0x1C
#define DMA_CHALX_TIMEOUT_CNT	0x20
#define DMA_CHALX_TRANS_CNT	0x24

#define DMA_CTL_EN		BIT(0)
#define DMA_CTL_SRST		BIT(1)

#define DMA_CHAL_SEL(id, x)	(min_t(unsigned int, x, 0x7F) << ((id) * 8))
#define DMA_CHAL_SEL_EN(id)	BIT((id) * 8 + 7)

#define DMA_STAT_CHAL(id)	BIT((id) * 4)

#define DMA_INT_EN		BIT(31)
#define DMA_INT_CHAL_EN(id)	BIT(id)

#define DMA_CHAL_EN		BIT(0)
#define DMA_CHAL_SRST		BIT(1)
#define DMA_CHAL_MODE		BIT(2)

#define DMA_RX_REQ		1
#define DMA_TX_REQ		0

#define DMA_CHAL_TIMEOUT_EN	BIT(31)
#define DMA_CHAL_TIMEOUT_CNT(x)	min_t(unsigned int, x, 0xFFFFF)

#define DMA_TIMEOUT		10

/**
 * struct phytium_ddma_sg_req - scatter-gatter list data info
 * @len: number of bytes to transform
 * @mem_addr_l: bus address low 32bit
 * @mem_addr_h: bus address high 32bit
 * @dev_addr: dma cousumer data reg addr
 * @direction: dma transmit direction
 */
struct phytium_ddma_sg_req {
	u32 len;
	u32 mem_addr_l;
	u32 mem_addr_h;
	u32 dev_addr;
	enum dma_transfer_direction direction;
};

/**
 * struct phytium_ddma_desc - the struct holding info describing ddma request
 * descriptor
 * @vdesc: ddma request descriptor
 * @num_sgs: the size of scatter-gatter list
 * @sg_req: use to save scatter-gatter list info
 */
struct phytium_ddma_desc {
	struct virt_dma_desc vdesc;
	u32 num_sgs;
	struct phytium_ddma_sg_req sg_req[];
};

/**
 * struct phytium_ddma_chan - the struct holding info describing dma channel
 * @vchan: virtual dma channel
 * @base: the mapped register I/O of dma physical channel
 * @id: the id of ddma physical channel
 * @request_line: the request line of ddma channel
 * @desc: the transform request descriptor
 * @dma_config: config parameters for dma channel
 * @busy: the channel busy flag, this flag set when channel is tansferring
 * @is_used: the channel bind flag, this flag set when channel binded
 * @next_sg: the index of next scatter-gatter
 * @current_sg: use to save the current transfer scatter-gatter info
 * @paddr: use to align data between dma provider and consumer
 */
struct phytium_ddma_chan {
	struct virt_dma_chan vchan;
	void __iomem *base;
	u32 id;
	u32 request_line;
	struct phytium_ddma_desc *desc;
	struct dma_slave_config dma_config;
	bool busy;
	bool is_used;
	bool is_pasued;
	u32 next_sg;
	struct phytium_ddma_sg_req *current_sg;
	dma_addr_t paddr;
	char *buf;
};

struct global_reg {
	u32 dma_chal_cfg0;
	u32 dma_chal_bind;
	u32 dma_chal_cfg1;
};

struct channel_reg {
	u32 dma_chalx_ctl;
	u32 dma_chalx_timeout_cnt;
};

/**
 * struct phytium_ddma_device - the struct holding info describing DDMA device
 * @dma_dev: an instance for struct dma_device
 * @irq: the irq that DDMA using
 * @base: the mapped register I/O base of this DDMA
 * @core_clk: DDMA clock
 * @dma_channels: the number of DDMA physical channels
 * @chan: the phyical channels of DDMA
 * @lock: spinlock to lock when set global registers
 * @dma_reg: store global register value which need recover after resume
 * @dma_chal_reg: store channel register value which need recover after resume
 */
struct phytium_ddma_device {
	struct dma_device dma_dev;
	struct device *dev;
	int	irq;
	void __iomem *base;
	struct clk *core_clk;
	u32 dma_channels;
	struct phytium_ddma_chan *chan;
	spinlock_t lock;
	struct global_reg dma_reg;
	struct channel_reg dma_chal_reg[DDMA_MAX_NR_PCHANNELS];
};

#endif /* _PHYTIUM_DDMAC_H */
