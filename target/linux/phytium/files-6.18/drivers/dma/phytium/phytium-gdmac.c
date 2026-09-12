// SPDX-License-Identifier:  GPL-2.0
/*
 * Phytium Device GDMA Controller driver.
 *
 * Copyright (c) 2023-2024 Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/acpi_dma.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <asm/barrier.h>
#include "phytium-gdmac.h"

#define PHYTIUM_GDMA_DRIVER_VERSION	"1.0.5"

static inline struct phytium_gdma_device *to_gdma_device(struct dma_chan *chan)
{
	return container_of(chan->device, struct phytium_gdma_device, dma_dev);
}

static inline struct phytium_gdma_chan *to_gdma_chan(struct dma_chan *chan)
{
	return container_of(chan, struct phytium_gdma_chan, vchan.chan);
}

static inline struct phytium_gdma_desc *to_gdma_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct phytium_gdma_desc, vdesc);
}

static inline struct device *chan_to_dev(struct phytium_gdma_chan *chan)
{
	return chan->vchan.chan.device->dev;
}

static inline
struct phytium_gdma_device *chan_to_gdma(struct phytium_gdma_chan *chan)
{
	return to_gdma_device(&chan->vchan.chan);
}

static inline void phytium_gdma_write(const struct phytium_gdma_device *gdma,
				      const u32 reg, const u32 val)
{
	iowrite32(val, gdma->base + reg);
}

static inline u32 phytium_gdma_read(const struct phytium_gdma_device *gdma,
					const u32 reg)
{
	return ioread32(gdma->base + reg);
}

static inline void phytium_chan_write(const struct phytium_gdma_chan *chan,
					  const u32 reg, const u32 val)
{
	iowrite32(val, chan->base + reg);
}

static inline u32 phytium_chan_read(const struct phytium_gdma_chan *chan,
					const u32 reg)
{
	return ioread32(chan->base + reg);
}

static void phytium_gdma_dump_reg(struct phytium_gdma_chan *chan)
{
	struct phytium_gdma_device *gdma = chan_to_gdma(chan);

	dev_dbg(chan_to_dev(chan), "gdma registers:\n");
	dev_dbg(chan_to_dev(chan), "\tDMA_CTL: 0x%08x\n",
		phytium_gdma_read(gdma, DMA_CTL));
	dev_dbg(chan_to_dev(chan), "\tDMA_STATE: 0x%08x\n",
		phytium_gdma_read(gdma, DMA_STATE));
	dev_dbg(chan_to_dev(chan), "\tDMA_INTR_CTL: 0x%08x\n",
		phytium_gdma_read(gdma, DMA_INTR_CTL));
	dev_dbg(chan_to_dev(chan), "\tDMA_LP_CTL: 0x%08x\n",
		phytium_gdma_read(gdma, DMA_LP_CTL));
	dev_dbg(chan_to_dev(chan), "\tDMA_QOS_CFG: 0x%08x\n",
		phytium_gdma_read(gdma, DMA_QOS_CFG));

	dev_dbg(chan_to_dev(chan), "gdma channel %d registers\n", chan->id);
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_CTRL: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_CTRL));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_MODE: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_MODE));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_INTR_CTL: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_INTR_CTL));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_STATE: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_STATE));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_LVI: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_LVI));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_TS: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_TS));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_UPSADDR: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_UPSADDR));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_LWSADDR: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_LWSADDR));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_UPDADDR: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_UPDADDR));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_LWDADDR: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_LWDADDR));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_XFER_CFG: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_XFER_CFG));
	dev_dbg(chan_to_dev(chan), "\tDMA_C%d_LCP: 0x%08x\n", chan->id,
		phytium_chan_read(chan, DMA_CX_LCP));
}

static void phytium_gdma_disable(const struct phytium_gdma_device *gdma)
{
	u32 val = phytium_gdma_read(gdma, DMA_CTL);

	dev_dbg(gdma->dev, "gdma disable\n");
	val &= ~DMA_CTL_EN;
	phytium_gdma_write(gdma, DMA_CTL, val);
}

static void phytium_gdma_enable(const struct phytium_gdma_device *gdma)
{
	u32 val = phytium_gdma_read(gdma, DMA_CTL);

	dev_dbg(gdma->dev, "gdma enable\n");
	val |= DMA_CTL_EN;
	phytium_gdma_write(gdma, DMA_CTL, DMA_CTL_EN);
}

static void phytium_gdma_reset(const struct phytium_gdma_device *gdma)
{
	u32 val = 0;

	dev_dbg(gdma->dev, "dma reset\n");
	val = phytium_gdma_read(gdma, DMA_CTL);
	val |= DMA_CTL_SRST;
	phytium_gdma_write(gdma, DMA_CTL, val);

	udelay(10);
	val &= ~DMA_CTL_SRST;
	phytium_gdma_write(gdma, DMA_CTL, val);
}

static void phytium_gdma_irq_disable(const struct phytium_gdma_device *gdma)
{
	u32 val = 0;

	dev_dbg(gdma->dev, "gdma irq disable\n");
	val = phytium_gdma_read(gdma, DMA_INTR_CTL);
	val &= ~DMA_INT_EN;
	phytium_gdma_write(gdma, DMA_INTR_CTL, val);
}

static void phytium_gdma_irq_enable(const struct phytium_gdma_device *gdma)
{
	u32 val = 0;

	dev_dbg(gdma->dev, "gdma irq enable\n");
	val = phytium_gdma_read(gdma, DMA_INTR_CTL);
	val |= DMA_INT_EN;
	phytium_gdma_write(gdma, DMA_INTR_CTL, val);
}

static void __maybe_unused
phytium_gdma_read_mode_set(struct phytium_gdma_device *gdma,
			   enum arbitration_mode mode)
{
	u32 val = phytium_gdma_read(gdma, DMA_CTL);

	switch (mode) {
	case POLLING_MODE:
		dev_dbg(gdma->dev, "set read polling mode\n");
		val &= ~BIT(5);
		break;
	case QOS_MODE:
		dev_dbg(gdma->dev, "set read qos mode\n");
		val |= BIT(5);
		break;
	}
	phytium_gdma_write(gdma, DMA_CTL, val);
}

static void phytium_gdma_write_mode_set(struct phytium_gdma_device *gdma,
					enum arbitration_mode mode)
{
	u32 val = phytium_gdma_read(gdma, DMA_CTL);

	switch (mode) {
	case POLLING_MODE:
		dev_dbg(gdma->dev, "set write polling mode\n");
		val &= ~BIT(4);
		break;
	case QOS_MODE:
		dev_dbg(gdma->dev, "set write qos mode\n");
		val |= BIT(4);
		break;
	}
	phytium_gdma_write(gdma, DMA_CTL, val);
}

static void __maybe_unused
phytium_gdma_outstanding_set(struct phytium_gdma_device *gdma, u32 outstanding)
{
	u32 val = 0;

	dev_dbg(gdma->dev, "set dma outstanding %d\n", outstanding);

	val = phytium_gdma_read(gdma, DMA_CTL);
	val &= ~DMA_OUTSTANDING_MASK;
	val |= ((outstanding - 1) << 8);
	phytium_gdma_write(gdma, DMA_CTL, val);
}

static void __maybe_unused
phytium_gdma_read_qos_set(struct phytium_gdma_device *gdma, u32 qos)
{
	u32 val = 0;

	dev_dbg(gdma->dev, "set read qos %d", qos);
	val = phytium_gdma_read(gdma, DMA_QOS_CFG);
	val &= ~DMA_READ_QOS_MASK;
	val |= (qos << 4);
	phytium_gdma_write(gdma, DMA_QOS_CFG, val);
}

static void __maybe_unused
phytium_gdma_write_qos_set(struct phytium_gdma_device *gdma, u32 qos)
{
	u32 val = phytium_gdma_read(gdma, DMA_QOS_CFG);

	dev_dbg(gdma->dev, "set read qos %d", qos);
	val &= ~DMA_WRITE_QOS_MASK;
	val |= qos;
	phytium_gdma_write(gdma, DMA_QOS_CFG, val);
}

static void phytium_chan_irq_enable(struct phytium_gdma_chan *chan, u32 mask)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq enable\n", chan->id);
	val = phytium_gdma_read(chan_to_gdma(chan), DMA_INTR_CTL);
	val |= DMA_INT_CHAL_EN(chan->id);
	phytium_gdma_write(chan_to_gdma(chan), DMA_INTR_CTL, val);
	phytium_chan_write(chan, DMA_CX_INTR_CTL, mask);
}

static void phytium_chan_irq_disable(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq disable\n", chan->id);
	val = phytium_gdma_read(chan_to_gdma(chan), DMA_INTR_CTL);
	val &= ~DMA_INT_CHAL_EN(chan->id);
	phytium_gdma_write(chan_to_gdma(chan), DMA_INTR_CTL, val);
}

static void phytium_chan_irq_clear(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq clear\n", chan->id);

	val = phytium_chan_read(chan, DMA_CX_STATE);
	phytium_chan_write(chan, DMA_CX_STATE, val);
}

static void phytium_chan_disable(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d disable\n", chan->id);

	/* disable channel */
	val = phytium_chan_read(chan, DMA_CX_CTRL);
	val &= ~DMA_CHAL_EN;
	phytium_chan_write(chan, DMA_CX_CTRL, val);
}

static void phytium_chan_enable(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d enable\n", chan->id);

	/* enable channel */
	val = phytium_chan_read(chan, DMA_CX_CTRL);
	val |= DMA_CHAL_EN;
	phytium_chan_write(chan, DMA_CX_CTRL, val);
}

static void phytium_chan_clk_enable(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d clk enable\n", chan->id);

	/* enable channel clock */
	val = phytium_gdma_read(chan_to_gdma(chan), DMA_LP_CTL);
	val &= ~DMA_CX_CLK_EN(chan->id);
	phytium_gdma_write(chan_to_gdma(chan), DMA_LP_CTL, val);
}

static void phytium_chan_clk_disable(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d clk disable\n", chan->id);

	/* disable channel clock */
	val = phytium_gdma_read(chan_to_gdma(chan), DMA_LP_CTL);
	val |= DMA_CX_CLK_EN(chan->id);
	phytium_gdma_write(chan_to_gdma(chan), DMA_LP_CTL, val);
}

static void phytium_chan_reset(struct phytium_gdma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d reset\n", chan->id);
	val = phytium_chan_read(chan, DMA_CX_CTRL);
	val |= DMA_CHAL_SRST;
	phytium_chan_write(chan, DMA_CX_CTRL, val);

	udelay(10);
	val = phytium_chan_read(chan, DMA_CX_CTRL);
	val &= ~DMA_CHAL_SRST;
	phytium_chan_write(chan, DMA_CX_CTRL, val);
}

static void phytium_chan_set_bdl_mode(struct phytium_gdma_chan *chan, bool bdl)
{
	u32 val = 0;

	val = phytium_chan_read(chan, DMA_CX_MODE);
	if (bdl) {
		dev_dbg(chan_to_dev(chan), "channel %d set bdl mode\n",
			chan->id);
		val |= DMA_CHAN_BDL_MODE;
	} else {
		dev_dbg(chan_to_dev(chan), "channel %d set direct mode\n",
			chan->id);
		val &= ~DMA_CHAN_BDL_MODE;
	}
	phytium_chan_write(chan, DMA_CX_MODE, val);
}

static u32 __maybe_unused
phytium_chan_get_bdl_num(struct phytium_gdma_chan *chan)
{
	u32 val = phytium_chan_read(chan, DMA_CX_LVI) + 1;

	return val;
}

static u32 phytium_gdma_set_xfer_ctrl(u32 burst_width, u32 burst_length,
					  enum burst_type burst_type)
{
	u32 xfer_ctrl = 0;

	xfer_ctrl = burst_type;
	xfer_ctrl |= (ffs(burst_width) - 1) << 4;
	xfer_ctrl |= (burst_length - 1) << 8;

	return xfer_ctrl;
}

static void phytium_chan_set_xfer_cfg(struct phytium_gdma_chan *chan,
				     u32 burst_width, u32 burst_length,
				     enum burst_type type)
{
	u32 xfer_ctl = phytium_gdma_set_xfer_ctrl(burst_width, burst_length,
						  type);
	u32 val = xfer_ctl << 16 | xfer_ctl;

	dev_dbg(chan_to_dev(chan), "channel %d set xfer cfg 0x%08x",
		chan->id, val);
	phytium_chan_write(chan, DMA_CX_XFER_CFG, val);
}

static void phytium_gdma_vdesc_free(struct virt_dma_desc *vd)
{
	struct phytium_gdma_desc *desc = to_gdma_desc(vd);
	struct phytium_gdma_chan *chan = desc->chan;

	if (desc->bdl_mode) {
		dma_free_coherent(chan_to_dev(desc->chan),
			sizeof(struct phytium_gdma_bdl) * desc->bdl_size,
			chan->bdl_list, chan->paddr);
	}
	kfree(desc);
}

static void phytium_chan_start_desc(struct phytium_gdma_chan *chan)
{
	struct virt_dma_desc *vd = vchan_next_desc(&chan->vchan);
	struct phytium_gdma_desc *desc = NULL;

	if (!vd) {
		chan->desc = NULL;
		return;
	}

	list_del(&vd->node);
	desc = to_gdma_desc(vd);
	chan->desc = desc;

	phytium_chan_clk_enable(chan);

	if (desc->bdl_mode) {
		phytium_chan_set_bdl_mode(chan, true);
		phytium_chan_write(chan, DMA_CX_UPSADDR,
				   upper_32_bits(chan->paddr));
		phytium_chan_write(chan, DMA_CX_LWSADDR,
				   lower_32_bits(chan->paddr));
		phytium_chan_write(chan, DMA_CX_UPDADDR, 0);
		phytium_chan_write(chan, DMA_CX_LWDADDR, 0);
		phytium_chan_write(chan, DMA_CX_TS, 0);
		phytium_chan_write(chan, DMA_CX_LVI, desc->bdl_size - 1);
		phytium_chan_write(chan, DMA_CX_XFER_CFG, 0);
	} else {
		phytium_chan_set_bdl_mode(chan, false);
		phytium_chan_set_xfer_cfg(chan, desc->burst_width,
					  desc->burst_length, BURST_INCR);
		phytium_chan_write(chan, DMA_CX_UPSADDR,
				   upper_32_bits(desc->src));
		phytium_chan_write(chan, DMA_CX_LWSADDR,
				   lower_32_bits(desc->src));
		phytium_chan_write(chan, DMA_CX_UPDADDR,
				   upper_32_bits(desc->dst));
		phytium_chan_write(chan, DMA_CX_LWDADDR,
				   lower_32_bits(desc->dst));
		phytium_chan_write(chan, DMA_CX_TS, desc->len);
	}

	phytium_gdma_outstanding_set(chan_to_gdma(chan), desc->outstanding);
	phytium_gdma_write_mode_set(chan_to_gdma(chan), POLLING_MODE);
	phytium_chan_irq_enable(chan, GDMA_CX_INT_CTRL_TRANS_END_ENABLE);
	phytium_gdma_dump_reg(chan);
	phytium_chan_enable(chan);
}

static void phytium_gdma_hw_init(struct phytium_gdma_device *gdma)
{
	u32 i = 0;

	phytium_gdma_disable(gdma);
	phytium_gdma_reset(gdma);
	phytium_gdma_irq_enable(gdma);
	phytium_gdma_enable(gdma);

	for (i = 0; i < gdma->dma_channels; i++) {
		phytium_chan_irq_disable(&gdma->chan[i]);
		phytium_chan_disable(&gdma->chan[i]);
		phytium_chan_clk_disable(&gdma->chan[i]);
	}
}

static int phytium_gdma_terminate_all(struct dma_chan *chan)
{
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);
	unsigned long flags = 0;
	LIST_HEAD(head);

	spin_lock_irqsave(&gdma_chan->vchan.lock, flags);

	phytium_gdma_dump_reg(gdma_chan);

	if (gdma_chan->desc) {
		vchan_terminate_vdesc(&gdma_chan->desc->vdesc);
		phytium_gdma_vdesc_free(&gdma_chan->desc->vdesc);
		gdma_chan->desc = NULL;
		phytium_chan_disable(gdma_chan);
		phytium_chan_reset(gdma_chan);
		phytium_chan_irq_disable(gdma_chan);
		phytium_chan_clk_disable(gdma_chan);
	}

	vchan_get_all_descriptors(&gdma_chan->vchan, &head);
	spin_unlock_irqrestore(&gdma_chan->vchan.lock, flags);
	vchan_dma_desc_free_list(&gdma_chan->vchan, &head);

	return 0;
}

static int phytium_gdma_alloc_chan_resources(struct dma_chan *chan)
{
	struct phytium_gdma_device *gdma = to_gdma_device(chan);
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);

	/* prepare channel */
	phytium_chan_disable(gdma_chan);
	phytium_chan_reset(gdma_chan);
	phytium_chan_irq_clear(gdma_chan);

	dev_info(gdma->dev, "alloc channel %d\n", gdma_chan->id);

	return 0;
}

static void phytium_gdma_free_chan_resources(struct dma_chan *chan)
{
	struct phytium_gdma_device *gdma = to_gdma_device(chan);
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);

	phytium_chan_disable(gdma_chan);
	phytium_chan_irq_disable(gdma_chan);
	phytium_chan_clk_disable(gdma_chan);

	vchan_free_chan_resources(&gdma_chan->vchan);

	dev_info(gdma->dev, "free channel %d\n", gdma_chan->id);
}

static int phytium_gdma_slave_config(struct dma_chan *chan,
				  struct dma_slave_config *config)
{
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);

	gdma_chan->dma_config = *config;

	return 0;
}


static enum dma_status phytium_gdma_tx_status(struct dma_chan *chan,
			dma_cookie_t cookie, struct dma_tx_state *txstate)
{
	enum dma_status ret;

	ret = dma_cookie_status(chan, cookie, txstate);

	return ret;
}

static int phytium_gdma_calcu_burst_width(dma_addr_t src, dma_addr_t dst,
					  u32 len)
{
	enum dma_slave_buswidth max_width = DMA_SLAVE_BUSWIDTH_16_BYTES;

	for (max_width = DMA_SLAVE_BUSWIDTH_16_BYTES;
	     max_width > DMA_SLAVE_BUSWIDTH_1_BYTE;
	     max_width >>= 1) {
		if ((((len | src | dst) & (max_width - 1)) == 0) &&
		    len > max_width)
			break;
	}

	return max_width;
}

static int phytium_gdma_xfer_bdl_mode(struct phytium_gdma_desc *desc,
				       dma_addr_t dst, dma_addr_t src)
{
	struct phytium_gdma_chan *chan = desc->chan;
	struct phytium_gdma_bdl *bdl = NULL;
	int i = 0;
	int ret = 0;
	u32 burst_width = 0;
	u32 burst_length = 0;
	size_t buf_len = desc->len;

	chan->bdl_list = dma_alloc_coherent(chan_to_dev(chan),
			sizeof(struct phytium_gdma_bdl) * desc->bdl_size,
			&chan->paddr, GFP_KERNEL);
	if (!chan->bdl_list) {
		dev_err(chan_to_dev(chan), "channel %d: unable to allocate bdl list\n",
			chan->id);
		ret = -ENOMEM;
		goto error_bdl;
	}

	for (i = 0; i < desc->bdl_size; i++) {
		bdl = &chan->bdl_list[i];

		/* set bdl info */
		bdl->src_addr_l = lower_32_bits(src);
		bdl->src_addr_h = upper_32_bits(src);
		bdl->dst_addr_l = lower_32_bits(dst);
		bdl->dst_addr_h = upper_32_bits(dst);

		bdl->length = min_t(u32, buf_len, GDMA_MAX_LEN);
		buf_len -= bdl->length;
		src += bdl->length;
		dst += bdl->length;
		desc->len = bdl->length;

		/* calculate burst width and burst length */
		burst_width = phytium_gdma_calcu_burst_width(src, dst,
							     bdl->length);
		burst_length = min_t(u32, GDMA_MAX_BURST_LENGTH,
				   bdl->length / burst_width);
		bdl->src_xfer_cfg =  phytium_gdma_set_xfer_ctrl(burst_width,
						burst_length, BURST_INCR);

		bdl->dst_xfer_cfg = bdl->src_xfer_cfg;

		/* not trigger interrupt after bdl transferred */
		bdl->intr_ctl = 0;

		dev_dbg(chan_to_dev(chan),
			"channel %d: bdl_mode, frame %d, len %d, burst_width %d, burst_length %d, xfer_cfg 0x%08x, outstanding: %d\n",
			chan->id, i, bdl->length, burst_width, burst_length,
			bdl->src_xfer_cfg, desc->outstanding);
	}

error_bdl:
	return ret;
}

static struct dma_async_tx_descriptor *phytium_gdma_prep_dma_memcpy(
	struct dma_chan *chan, dma_addr_t dst, dma_addr_t src,
	size_t len, unsigned long flags)
{
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);
	struct phytium_gdma_desc *desc = NULL;
	struct dma_async_tx_descriptor *tx_desc = NULL;
	u32 frames = 0;
	u32 max_outstanding = 0;
	int ret = 0;

	if (!src || !dst || !len)
		return NULL;

	frames = DIV_ROUND_UP(len, GDMA_MAX_LEN);

	desc = kzalloc(sizeof(struct phytium_gdma_desc), GFP_KERNEL);
	if (IS_ERR_OR_NULL(desc))
		return NULL;

	dev_dbg(chan_to_dev(gdma_chan),
		"memcpy: src %llx, dst %llx, len %ld, frames %d\n",
		src, dst, len, frames);

	desc->len = len;
	desc->chan = gdma_chan;
	max_outstanding = chan_to_gdma(gdma_chan)->max_outstanding;

	if (frames > 1) {
		/* bdl xfer */
		desc->bdl_mode = true;
		desc->bdl_size = frames;
		desc->outstanding = min_t(u32, max_outstanding,
			len / GDMA_MAX_BURST_SIZE / GDMA_MAX_BURST_LENGTH);
		ret = phytium_gdma_xfer_bdl_mode(desc, dst, src);
	} else {
		/* direct xfer */
		desc->bdl_mode = false;
		desc->burst_width = phytium_gdma_calcu_burst_width(src, dst,
								   len);
		desc->burst_length = min_t(u32, GDMA_MAX_BURST_LENGTH,
					   len / desc->burst_width);
		desc->src = src;
		desc->dst = dst;
		desc->outstanding = min_t(u32, max_outstanding,
			len / desc->burst_length / desc->burst_width);
		dev_dbg(chan_to_dev(gdma_chan), "channel %d: direct mode, len %ld, burst_width %d, burst_length %d, outstanding %d\n",
			gdma_chan->id, desc->len, desc->burst_width,
			desc->burst_length, desc->outstanding);
	}

	if (!ret)
		tx_desc = vchan_tx_prep(&gdma_chan->vchan, &desc->vdesc, flags);

	return tx_desc;
}

static void phytium_gdma_issue_pending(struct dma_chan *chan)
{
	struct phytium_gdma_chan *gdma_chan = to_gdma_chan(chan);
	unsigned long flags = 0;

	spin_lock_irqsave(&gdma_chan->vchan.lock, flags);
	if (vchan_issue_pending(&gdma_chan->vchan) && !gdma_chan->desc)
		phytium_chan_start_desc(gdma_chan);

	spin_unlock_irqrestore(&gdma_chan->vchan.lock, flags);
}

static void phytium_chan_irq_handler(struct phytium_gdma_chan *chan)
{
	struct phytium_gdma_desc *desc = NULL;
	unsigned long flags = 0;
	u32 state = 0;
	int ret = 0;

	dev_dbg(chan_to_dev(chan), "channel %d int state %08x", chan->id,
		chan->state);

	/* transfer busy */
	if (chan->state & GDMA_CX_INT_STATE_BUSY) {
		dev_info(chan_to_dev(chan), "gdma channel %d busy\n", chan->id);
		ret = readl_poll_timeout_atomic(chan->base + DMA_CX_STATE, state,
					 state & GDMA_CX_INT_STATE_TRANS_END,
					 10, 2000);
		if (ret) {
			dev_err(chan_to_dev(chan),
				"gdma channel %d xfer timeout\n", chan->id);
		}
	}

	/* transfer complete */
	if (chan->state & GDMA_CX_INT_STATE_TRANS_END) {
		dev_dbg(chan_to_dev(chan), "channel %d xfer complete\n", chan->id);
		desc = chan->desc;
		spin_lock_irqsave(&chan->vchan.lock, flags);
		vchan_cookie_complete(&desc->vdesc);
		phytium_chan_start_desc(chan);
		spin_unlock_irqrestore(&chan->vchan.lock, flags);
	}

	/* fifo fulll */
	if (chan->state & GDMA_CX_INT_STATE_FIFO_FULL) {
		dev_err(chan_to_dev(chan),
			"gdma channel %d fifo full\n", chan->id);
	}
}

static irqreturn_t phytium_dma_interrupt(int irq, void *dev_id)
{
	struct phytium_gdma_device *gdma = dev_id;
	struct phytium_gdma_chan *gdma_chan = NULL;
	u32 state = 0;
	int i = 0;

	state = phytium_gdma_read(gdma, DMA_STATE);
	dev_dbg(gdma->dev, "gdma interrupt, state %04x", state);

	for (i = 0; i < gdma->dma_channels; i++) {
		if (state & DMA_CX_INTR_STATE(i)) {
			gdma_chan = &gdma->chan[i];
			gdma_chan->state = phytium_chan_read(gdma_chan,
							     DMA_CX_STATE);
			phytium_chan_irq_clear(gdma_chan);

			if (gdma->chan[i].desc) {
				phytium_chan_irq_handler(gdma_chan);
			}
		}
	}

	return IRQ_HANDLED;
}

static struct dma_chan *phytium_gdma_of_xlate(struct of_phandle_args *dma_spec,
						struct of_dma *ofdma)
{
	struct phytium_gdma_device *gdma = ofdma->of_dma_data;
	struct device *dev = gdma->dev;
	struct phytium_gdma_chan *chan = NULL;
	struct dma_chan *c = NULL;
	u32 channel_id = 0;

	channel_id = dma_spec->args[0];

	if (channel_id >= gdma->dma_channels) {
		dev_err(dev, "bad channel %d\n", channel_id);
		return NULL;
	}

	chan = &gdma->chan[channel_id];
	c = dma_get_slave_channel(&chan->vchan.chan);
	if (!c) {
		dev_err(dev, "no more channels available\n");
		return NULL;
	}

	return c;
}

static struct dma_chan *phytium_gdma_acpi_xlate(struct acpi_dma_spec *dma_spec,
						struct acpi_dma *acpidma)
{
	struct phytium_gdma_device *gdma = (struct phytium_gdma_device *)acpidma->data;
	struct device *dev = gdma->dev;
	struct phytium_gdma_chan *chan = NULL;
	struct dma_chan *c = NULL;
	u32 channel_id = 0;

	channel_id = dma_spec->chan_id;

	if (channel_id >= gdma->dma_channels) {
		dev_err(dev, "bad channel %d\n", channel_id);
		return NULL;
	}

	chan = &gdma->chan[channel_id];
	c = dma_get_slave_channel(&chan->vchan.chan);
	if (!c) {
		dev_err(dev, "no more channels available\n");
		return NULL;
	}

	return c;
}

static int phytium_gdma_probe(struct platform_device *pdev)
{
	struct phytium_gdma_device *gdma;
	struct dma_device *dma_dev;
	struct resource *mem;
	u32 i = 0;
	u32 nr_channels = 0;
	u32 max_outstanding = 0;
	int irq_count = 0;
	int irq = 0;
	int ret = 0;

	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret) {
		dev_err(&pdev->dev, "Unable to set DMA mask\n");
		goto out;
	}

	gdma = devm_kzalloc(&pdev->dev, sizeof(*gdma), GFP_KERNEL);
	if (!gdma) {
		ret = -ENOMEM;
		goto out;
	}

	dma_dev = &gdma->dma_dev;
	gdma->dev = &pdev->dev;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gdma->base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(gdma->base)) {
		dev_err(&pdev->dev, "no resource address");
		ret = PTR_ERR(gdma->base);
		goto out;
	}

	ret = device_property_read_u32(&pdev->dev, "dma-channels",
					   &nr_channels);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"can't get the number of dma channels: %d\n", ret);
		goto out;
	}
	gdma->dma_channels = nr_channels;

	ret = device_property_read_u32(&pdev->dev, "max-outstanding",
					   &max_outstanding);
	if (ret < 0) {
		dev_err(&pdev->dev, "can't get max outstanding %d\n", ret);
		goto out;
	}
	gdma->max_outstanding = max_outstanding;

	irq_count = platform_irq_count(pdev);
	if (irq_count <= 0) {
		dev_err(&pdev->dev, "no irq found\n");
		ret = -EINVAL;
		goto out;
	}
	for (i = 0; i < irq_count; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "can't get irq %d\n", i);
			continue;
		}
		ret = devm_request_irq(&pdev->dev, irq,
				       phytium_dma_interrupt, IRQF_SHARED,
				       dev_name(&pdev->dev), gdma);
		if (ret) {
			dev_err(&pdev->dev, "could not to request irq %d", irq);
			goto out;
		}
	}

	/* Set capabilities */
	dma_cap_set(DMA_MEMCPY, gdma->dma_dev.cap_mask);

	/* DMA capabilities */
	dma_dev->dev = gdma->dev;
	dma_dev->chancnt = gdma->dma_channels;
	dma_dev->max_burst = 8;
	dma_dev->src_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
				   BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_4_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_8_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_16_BYTES);
	dma_dev->dst_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
				   BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_4_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_8_BYTES) |
				   BIT(DMA_SLAVE_BUSWIDTH_16_BYTES);
	dma_dev->directions = BIT(DMA_MEM_TO_MEM);
	dma_dev->residue_granularity = DMA_RESIDUE_GRANULARITY_DESCRIPTOR;

	/* function callback */
	dma_dev->device_alloc_chan_resources =
		phytium_gdma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = phytium_gdma_free_chan_resources;
	dma_dev->device_tx_status = phytium_gdma_tx_status;
	dma_dev->device_config = phytium_gdma_slave_config;
	dma_dev->device_prep_dma_memcpy = phytium_gdma_prep_dma_memcpy;
	dma_dev->device_issue_pending = phytium_gdma_issue_pending;
	dma_dev->device_terminate_all = phytium_gdma_terminate_all;

	/* init dma channels */
	INIT_LIST_HEAD(&dma_dev->channels);
	gdma->chan = devm_kcalloc(gdma->dev, gdma->dma_channels,
				sizeof(*gdma->chan), GFP_KERNEL);
	if (!gdma->chan) {
		ret = -ENOMEM;
		goto out;
	}
	for (i = 0; i < gdma->dma_channels; i++) {
		gdma->chan[i].id = i;
		gdma->chan[i].base = gdma->base + DMA_REG_OFFSET +
			i * CHAN_REG_OFFSET;
		gdma->chan[i].vchan.desc_free = phytium_gdma_vdesc_free;
		gdma->chan[i].desc = NULL;
		vchan_init(&gdma->chan[i].vchan, dma_dev);
	}

	phytium_gdma_hw_init(gdma);

	ret = dma_async_device_register(dma_dev);
	if (ret)
		goto out;

	if (has_acpi_companion(&pdev->dev))
		ret = acpi_dma_controller_register(&pdev->dev, phytium_gdma_acpi_xlate, gdma);
	else
		ret = of_dma_controller_register(pdev->dev.of_node,
				phytium_gdma_of_xlate, gdma);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"phytium gdma of register failed %d\n", ret);
		goto err_unregister;
	}

	platform_set_drvdata(pdev, gdma);
	dev_info(gdma->dev, "phytium GDMA Controller registered\n");

	return 0;

err_unregister:
	dma_async_device_unregister(dma_dev);

out:
	return ret;
}

static void phytium_gdma_chan_remove(struct phytium_gdma_chan *chan)
{
	phytium_chan_irq_disable(chan);
	phytium_chan_disable(chan);

	tasklet_kill(&chan->vchan.task);
	list_del(&chan->vchan.chan.device_node);
}

static void phytium_gdma_remove(struct platform_device *pdev)
{
	struct phytium_gdma_device *gdma = platform_get_drvdata(pdev);
	struct phytium_gdma_chan *chan = NULL;
	int i = 0;

	if (has_acpi_companion(&pdev->dev))
		acpi_dma_controller_free(&pdev->dev);
	else
		of_dma_controller_free(pdev->dev.of_node);
	dma_async_device_unregister(&gdma->dma_dev);

	for (i = 0; i < gdma->dma_channels; i++) {
		chan = &gdma->chan[i];
		phytium_gdma_chan_remove(chan);
	}

	phytium_gdma_irq_disable(gdma);
	phytium_gdma_disable(gdma);

}

#ifdef CONFIG_PM_SLEEP
static int phytium_gdma_suspend(struct device *dev)
{
	struct phytium_gdma_device *gdma = dev_get_drvdata(dev);

	phytium_gdma_irq_disable(gdma);
	phytium_gdma_disable(gdma);

	return 0;
}

static int phytium_gdma_resume(struct device *dev)
{
	struct phytium_gdma_device *gdma = dev_get_drvdata(dev);
	int ret = 0;

	phytium_gdma_hw_init(gdma);

	return ret;
}
#endif

static const struct dev_pm_ops phytium_gdma_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(phytium_gdma_suspend,
			phytium_gdma_resume)
};

static const struct of_device_id phytium_dma_of_id_table[] = {
	{ .compatible = "phytium,gdma" },
	{}
};
MODULE_DEVICE_TABLE(of, phytium_dma_of_id_table);

static const struct acpi_device_id phytium_gdma_acpi_match[] = {
	{ "PHYT0026", 0 },
	{ }
};
MODULE_DEVICE_TABLE(acpi, phytium_gdma_acpi_match);

static struct platform_driver phytium_gdma_driver = {
	.probe		= phytium_gdma_probe,
	.remove		= phytium_gdma_remove,
	.driver = {
		.name	= "phytium-gdma",
		.of_match_table = of_match_ptr(phytium_dma_of_id_table),
		.acpi_match_table = ACPI_PTR(phytium_gdma_acpi_match),
		.pm = &phytium_gdma_pm_ops,
	},
};

static __init int phytium_gdma_init(void)
{
	return platform_driver_register(&phytium_gdma_driver);
}

static __exit void phytium_gdma_exit(void)
{
	return platform_driver_unregister(&phytium_gdma_driver);
}

subsys_initcall(phytium_gdma_init);
module_exit(phytium_gdma_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium gdma Controller platform driver");
MODULE_AUTHOR("HuangJie <huangjie1663@phytium.com.cn>");
MODULE_VERSION(PHYTIUM_GDMA_DRIVER_VERSION);
