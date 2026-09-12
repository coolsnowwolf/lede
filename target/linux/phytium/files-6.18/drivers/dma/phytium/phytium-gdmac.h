/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Phytium Device GDMA Controller driver.
 *
 * Copyright (c) 2023-2024 Phytium Technology Co., Ltd.
 */

#ifndef _PHYTIUM_GDMAC_H
#define _PHYTIUM_GDMAC_H

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/types.h>
#include "../virt-dma.h"


#define GDMA_MAX_LEN		SZ_1G
#define GDMA_MAX_BURST_LENGTH	8
#define GDMA_MAX_BURST_SIZE	16

/* GDMA register address offset */
#define DMA_REG_OFFSET		0x20
#define CHAN_REG_OFFSET		0x60

#define DMA_CTL			0x00
#define DMA_STATE		0x04
#define DMA_INTR_CTL		0x08
#define DMA_LP_CTL		0x0C
#define DMA_QOS_CFG		0x10

#define DMA_CX_CTRL		0x00
#define DMA_CX_MODE		0x04
#define DMA_CX_INTR_CTL		0x08
#define DMA_CX_STATE		0x0C
#define DMA_CX_LVI		0x10
#define DMA_CX_TS		0x14
#define DMA_CX_UPSADDR		0x18
#define DMA_CX_LWSADDR		0x1C
#define DMA_CX_UPDADDR		0x20
#define DMA_CX_LWDADDR		0x24
#define DMA_CX_XFER_CFG		0x28
#define DMA_CX_LCP		0x2C
#define DMA_CX_SECCTL		0x30
#define DMA_CX_SEC_ATST		0x34
#define DMA_CX_NSID_STRMID	0x38
#define DMA_CX_AW_CFG		0x3C
#define DMA_CX_AR_CFG		0x40
#define DMA_CX_SECRSP		0x44

/* DMA_CTL register */
#define DMA_CTL_EN		BIT(0)
#define DMA_CTL_SRST		BIT(1)

#define DMA_OUTSTANDING_MASK	GENMASK(11, 8)
#define DMA_WRITE_QOS_MASK	GENMASK(3, 0)
#define DMA_READ_QOS_MASK	GENMASK(7, 4)

/* DMA_STATE register */
#define DMA_CX_INTR_STATE(id)	BIT(id)

/* DMA_INTR_CTL register */
#define DMA_INT_EN		BIT(31)
#define DMA_INT_CHAL_EN(id)	BIT(id)

/* DMA_LP_CTL register */
#define DMA_CX_CLK_EN(id)	BIT(id)

/* DMA_CX_MODE register */
#define DMA_CHAN_BDL_MODE	BIT(0)

/* DMA_CX_CTRL register */
#define DMA_CHAL_EN		BIT(0)
#define DMA_CHAL_SRST		BIT(4)

/* DMA_CX_INTR_CTL register, channel interrupt control bits */
#define GDMA_CX_INT_CTRL_TRANS_END_ENABLE	BIT(3)
#define GDMA_CX_INT_CTRL_BDL_END_ENABLE		BIT(2)
#define GDMA_CX_INT_CTRL_FIFO_FULL_ENABLE	BIT(1)
#define GDMA_CX_INT_CTRL_FIFO_EMPTY_ENABLE	BIT(0)


/* DMA_CX_STATE register, channel interrupt states */
#define GDMA_CX_INT_STATE_BUSY			BIT(4)
#define GDMA_CX_INT_STATE_TRANS_END		BIT(3)
#define GDMA_CX_INT_STATE_BDL_END		BIT(2)
#define GDMA_CX_INT_STATE_FIFO_FULL		BIT(1)
#define GDMA_CX_INT_STATE_FIFO_EMPTY		BIT(0)


/* channel modes */
enum arbitration_mode {
	POLLING_MODE,
	QOS_MODE
};

/* burst type */
enum burst_type {
	BURST_FIX,
	BURST_INCR
};

/**
 * @brief struct phytium_gdma_bdl - describe for bdl list
 * @src_addr_l: low 32 bits of src addr
 * @src_addr_h: high 32 bits of src addr
 * @dst_addr_l: low 32 bits of dst addr
 * @dst_addr_h: high 32 bits of dst addr
 * @src_xfer_cfg: xfer config for read
 * @dst_xfer_cfg: xfer config for write
 * @length: xfer length
 * @intr_ctl: interrupt enable control
 */
struct phytium_gdma_bdl {
	u32 src_addr_l;
	u32 src_addr_h;
	u32 dst_addr_l;
	u32 dst_addr_h;
	u32 src_xfer_cfg;
	u32 dst_xfer_cfg;
	u32 length;
	u32 intr_ctl;
};

/**
 * struct phytium_gdma_desc - the struct holding info describing gdma request
 * descriptor
 * @chan: the channel belonging to this descriptor
 * @vdesc: gdma request descriptor
 * @len: total len of dma request
 * @bdl_size: store bdl size in bdl mode
 * @bdl_mode: the bdl xfer mode flag
 * @burst_widht: store burst width in direct xfer mode
 * @burst_length: store burst length in direct xfer mode
 * @dst: store dst addr in direct xfer mode
 * @src: store src addr in direct xfer mode
 * @outstanding: store outstanding
 */
struct phytium_gdma_desc {
	struct phytium_gdma_chan *chan;
	struct virt_dma_desc vdesc;
	size_t len;
	u32 bdl_size;
	bool bdl_mode;
	u32 burst_width;
	u32 burst_length;
	dma_addr_t dst;
	dma_addr_t src;
	u32 outstanding;
};

/**
 * struct phytium_gdma_chan - the struct holding info describing dma channel
 * @vchan: virtual dma channel
 * @dma_config: config parameters for dma channel
 * @desc: the transform request descriptor
 * @bdl_list: the pointer of bdl
 * @paddr: the dma address of bdl
 * @id: the id of gdma physical channel
 * @base: the mapped register I/O of dma physical channel
 * @state: channel transfer state
 */
struct phytium_gdma_chan {
	struct virt_dma_chan vchan;
	struct dma_slave_config dma_config;
	struct phytium_gdma_desc *desc;
	struct phytium_gdma_bdl *bdl_list;
	dma_addr_t paddr;

	void __iomem *base;
	u32 id;
	u32 state;
};

/**
 * struct phytium_gdma_device - the struct holding info describing gdma device
 * @dma_dev: an instance for struct dma_device
 * @base: the mapped register I/O base of this gdma
 * @dma_channels: the number of gdma channels
 * @max_outstanding: the max outstanding support
 * @chan: the phyical channels of gdma
 */
struct phytium_gdma_device {
	struct dma_device dma_dev;
	struct device *dev;
	void __iomem *base;
	u32 dma_channels;
	u32 max_outstanding;
	struct phytium_gdma_chan *chan;
};

#endif /* _PHYTIUM_GDMAC_H */
