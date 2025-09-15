// SPDX-License-Identifier:  GPL-2.0
/*
 * Phytium Device DDMA Controller driver.
 *
 * Copyright (c) 2023 Phytium Technology Co., Ltd.
 */

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
#include "phytium-ddmac.h"


static inline struct phytium_ddma_device *to_ddma_device(struct dma_chan *chan)
{
	return container_of(chan->device, struct phytium_ddma_device, dma_dev);
}

static inline struct phytium_ddma_chan *to_ddma_chan(struct dma_chan *chan)
{
	return container_of(chan, struct phytium_ddma_chan, vchan.chan);
}

static inline struct phytium_ddma_desc *to_ddma_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct phytium_ddma_desc, vdesc);
}

static inline struct device *chan_to_dev(struct phytium_ddma_chan *chan)
{
	return chan->vchan.chan.device->dev;
}

static inline struct phytium_ddma_device *chan_to_ddma(
		struct phytium_ddma_chan *chan)
{
	return to_ddma_device(&chan->vchan.chan);
}

static inline void phytium_ddma_iowrite32(
		const struct phytium_ddma_device *ddma,
		const u32 reg, const u32 val)
{
	iowrite32(val, ddma->base + reg);
}

static inline u32 phytium_ddma_ioread32(const struct phytium_ddma_device *ddma,
					const u32 reg)
{
	return ioread32(ddma->base + reg);
}

static inline void phytium_chan_iowrite32(const struct phytium_ddma_chan *chan,
					const u32 reg, const u32 val)
{
	iowrite32(val, chan->base + reg);
}

static inline u32 phytium_chan_ioread32(const struct phytium_ddma_chan *chan,
					const u32 reg)
{
	return ioread32(chan->base + reg);
}

static void phytium_ddma_disable(const struct phytium_ddma_device *ddma)
{
	dev_dbg(ddma->dev, "ddma disable\n");
	phytium_ddma_iowrite32(ddma, DMA_CTL, !DMA_CTL_EN);
}

static void phytium_ddma_enable(const struct phytium_ddma_device *ddma)
{
	dev_dbg(ddma->dev, "ddma enable\n");
	phytium_ddma_iowrite32(ddma, DMA_CTL, DMA_CTL_EN);
}

static void phytium_ddma_reset(const struct phytium_ddma_device *ddma)
{
	u32 val = 0;

	dev_dbg(ddma->dev, "dma reset\n");
	val = phytium_ddma_ioread32(ddma, DMA_CTL);
	val |= DMA_CTL_SRST;
	phytium_ddma_iowrite32(ddma, DMA_CTL, val);

	udelay(10);
	val &= ~DMA_CTL_SRST;
	phytium_ddma_iowrite32(ddma, DMA_CTL, val);
}

static void phytium_ddma_irq_disable(const struct phytium_ddma_device *ddma)
{
	u32 val = 0;

	dev_dbg(ddma->dev, "ddma irq disable\n");
	val = phytium_ddma_ioread32(ddma, DMA_MASK_INT);
	val |= DMA_INT_EN;
	phytium_ddma_iowrite32(ddma, DMA_MASK_INT, val);
}

static void phytium_ddma_irq_enable(const struct phytium_ddma_device *ddma)
{
	u32 val = 0;

	dev_dbg(ddma->dev, "ddma irq enable\n");
	val = phytium_ddma_ioread32(ddma, DMA_MASK_INT);
	val &= ~DMA_INT_EN;
	phytium_ddma_iowrite32(ddma, DMA_MASK_INT, val);
}

static u32 phytium_ddma_irq_read(const struct phytium_ddma_device *ddma)
{
	u32 val = 0;

	val = phytium_ddma_ioread32(ddma, DMA_STAT);

	return val;
}

static void phytium_chan_irq_disable(struct phytium_ddma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq disable\n", chan->id);
	val = phytium_ddma_ioread32(chan_to_ddma(chan), DMA_MASK_INT);
	val |= DMA_INT_CHAL_EN(chan->id);
	phytium_ddma_iowrite32(chan_to_ddma(chan), DMA_MASK_INT, val);
}

static void phytium_chan_irq_enable(struct phytium_ddma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq enable\n", chan->id);
	val = phytium_ddma_ioread32(chan_to_ddma(chan), DMA_MASK_INT);
	val &= ~DMA_INT_CHAL_EN(chan->id);
	phytium_ddma_iowrite32(chan_to_ddma(chan), DMA_MASK_INT, val);
}

static void phytium_chan_irq_clear(struct phytium_ddma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d irq clear\n", chan->id);
	val = DMA_STAT_CHAL(chan->id);
	phytium_ddma_iowrite32(chan_to_ddma(chan), DMA_STAT, val);
}

static int phytium_chan_disable(struct phytium_ddma_chan *chan)
{
	u32 val = 0;
	int ret = 0;

	dev_dbg(chan_to_dev(chan), "channel %d disable\n", chan->id);
	val = phytium_chan_ioread32(chan, DMA_CHALX_CTL);
	if (val | DMA_CHAL_EN) {
		val &= ~DMA_CHAL_EN;
		phytium_chan_iowrite32(chan, DMA_CHALX_CTL, val);

		ret = readl_relaxed_poll_timeout_atomic(
			chan->base + DMA_CHALX_CTL, val,
			!(val & DMA_CHAL_EN), 0, 100000);
	}
	return ret;
}

static void phytium_chan_enable(struct phytium_ddma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d enable\n", chan->id);
	val = phytium_chan_ioread32(chan, DMA_CHALX_CTL);
	val |= DMA_CHAL_EN;
	phytium_chan_iowrite32(chan, DMA_CHALX_CTL, val);
}

static bool phytium_chan_is_running(const struct phytium_ddma_chan *chan)
{
	u32 val;

	val = phytium_chan_ioread32(chan, DMA_CHALX_CTL);

	if (val & DMA_CHAL_EN)
		return true;
	else
		return false;
}

static void phytium_chan_reset(struct phytium_ddma_chan *chan)
{
	u32 val = 0;

	dev_dbg(chan_to_dev(chan), "channel %d reset\n", chan->id);
	val = phytium_chan_ioread32(chan, DMA_CHALX_CTL);
	val |= DMA_CHAL_SRST;
	phytium_chan_iowrite32(chan, DMA_CHALX_CTL, val);

	udelay(10);
	val &= ~DMA_CHAL_SRST;
	phytium_chan_iowrite32(chan, DMA_CHALX_CTL, val);
}

static void phytium_ddma_vdesc_free(struct virt_dma_desc *vd)
{
	kfree(to_ddma_desc(vd));
}

static int phytium_chan_pause(struct dma_chan *chan)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	int ret = 0;

	ret = phytium_chan_disable(pchan);
	pchan->busy = false;
	pchan->is_pasued = true;

	return ret;
}

static int phytium_chan_resume(struct dma_chan *chan)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);

	phytium_chan_enable(pchan);
	pchan->is_pasued = false;

	return 0;
}

static void phytium_chan_start_xfer(struct phytium_ddma_chan *chan)
{
	struct virt_dma_desc *vdesc = NULL;
	struct phytium_ddma_sg_req *sg_req = NULL;
	char *tmp = NULL;
	int i = 0;
	unsigned long flags = 0;

	/* chan first xfer settings */
	if (!chan->desc) {
		vdesc = vchan_next_desc(&chan->vchan);
		if (!vdesc)
			return;

		list_del(&vdesc->node);
		chan->desc = to_ddma_desc(vdesc);
		chan->next_sg = 0;
		chan->current_sg = NULL;
		dev_dbg(chan_to_dev(chan), "xfer start\n");
	}

	if (chan->next_sg == chan->desc->num_sgs)
		chan->next_sg = 0;

	sg_req = &chan->desc->sg_req[chan->next_sg];
	chan->current_sg = sg_req;
	/* fill to 4 bytes */
	switch (sg_req->direction) {
	case DMA_MEM_TO_DEV:
		tmp = phys_to_virt(sg_req->mem_addr_l);
		memset(chan->buf, 0, sg_req->len * 4);
		for (i = 0; i < sg_req->len; i++)
			chan->buf[i * 4] = tmp[i];
		break;

	case DMA_DEV_TO_MEM:
		memset(chan->buf, 0, sg_req->len * 4);
		break;

	default:
		break;
	}

	/* start transfer */
	phytium_chan_iowrite32(chan, DMA_CHALX_DDR_LWADDR,
			chan->paddr & 0xFFFFFFFF);
	phytium_chan_iowrite32(chan, DMA_CHALX_DDR_UPADDR,
			(chan->paddr >> 32) & 0xFFFFFFFF);
	phytium_chan_iowrite32(chan, DMA_CHALX_DEV_ADDR, sg_req->dev_addr);
	phytium_chan_iowrite32(chan, DMA_CHALX_TS, sg_req->len * 4);

	spin_lock_irqsave(&chan_to_ddma(chan)->lock, flags);
	phytium_chan_irq_enable(chan);
	spin_unlock_irqrestore(&chan_to_ddma(chan)->lock, flags);
	phytium_chan_enable(chan);

	chan->next_sg++;
	chan->busy = true;
}

static void phytium_chan_xfer_done(struct phytium_ddma_chan *chan)
{
	struct phytium_ddma_sg_req *sg_req = chan->current_sg;
	char *tmp = NULL;
	int i = 0;

	if (chan->desc) {
		if (sg_req->direction ==  DMA_DEV_TO_MEM) {
			tmp = phys_to_virt(sg_req->mem_addr_l);
			for (i = 0; i < sg_req->len; i++)
				tmp[i] = chan->buf[i * 4];
		}

		chan->busy = false;
		if (chan->next_sg == chan->desc->num_sgs) {
			dev_dbg(chan_to_dev(chan), "xfer complete\n");
			vchan_cookie_complete(&chan->desc->vdesc);
			chan->desc = NULL;
			chan->current_sg = NULL;
		}
		phytium_chan_disable(chan);
		phytium_chan_irq_clear(chan);
		phytium_chan_start_xfer(chan);
	}
}

static void phytium_dma_hw_init(struct phytium_ddma_device *ddma)
{
	u32 i = 0;
	int ret = 0;

	phytium_ddma_disable(ddma);
	phytium_ddma_reset(ddma);
	phytium_ddma_irq_enable(ddma);
	phytium_ddma_enable(ddma);

	for (i = 0; i < ddma->dma_channels; i++) {
		phytium_chan_irq_disable(&ddma->chan[i]);
		ret = phytium_chan_disable(&ddma->chan[i]);
		if (ret)
			dev_err(ddma->dev, "can't disable channel %d\n", i);
	}
}

static size_t phytium_ddma_desc_residue(struct phytium_ddma_chan *chan)
{
	u32 trans_cnt = 0;
	u32 residue = 0;
	int i = 0;

	trans_cnt = phytium_chan_ioread32(chan, DMA_CHALX_TRANS_CNT);
	residue = chan->current_sg->len - trans_cnt;

	for (i = chan->next_sg; i < chan->desc->num_sgs; i++)
		residue += chan->desc->sg_req[i].len;

	return residue;
}

static enum dma_status phytium_ddma_tx_status(struct dma_chan *chan,
					   dma_cookie_t cookie,
					   struct dma_tx_state *txstate)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	struct virt_dma_desc *vd = NULL;
	enum dma_status ret = 0;
	unsigned long flags = 0;
	size_t residue = 0;

	ret = dma_cookie_status(chan, cookie, txstate);
	if ((ret == DMA_COMPLETE) || !txstate)
		return ret;

	spin_lock_irqsave(&pchan->vchan.lock, flags);
	vd = vchan_find_desc(&pchan->vchan, cookie);
	if (pchan->desc && cookie == pchan->desc->vdesc.tx.cookie)
		residue = phytium_ddma_desc_residue(pchan);

	dma_set_residue(txstate, residue);
	spin_unlock_irqrestore(&pchan->vchan.lock, flags);

	if (pchan->is_pasued && ret == DMA_IN_PROGRESS)
		ret = DMA_PAUSED;

	return ret;
}

static void phytium_ddma_issue_pending(struct dma_chan *chan)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	unsigned long flags = 0;

	spin_lock_irqsave(&pchan->vchan.lock, flags);

	if (vchan_issue_pending(&pchan->vchan) && !pchan->desc && !pchan->busy)
		phytium_chan_start_xfer(pchan);

	spin_unlock_irqrestore(&pchan->vchan.lock, flags);
}

static int phytium_ddma_terminate_all(struct dma_chan *chan)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	unsigned long flags = 0;
	LIST_HEAD(head);

	spin_lock_irqsave(&pchan->vchan.lock, flags);
	if (pchan->desc) {
		vchan_terminate_vdesc(&pchan->desc->vdesc);
		if (pchan->busy) {
			u32 tmp_ctl, timeout;
			phytium_chan_disable(pchan);
			/* save some registers, reset will clear it */
			timeout = phytium_chan_ioread32(pchan,
						DMA_CHALX_TIMEOUT_CNT);
			tmp_ctl = phytium_chan_ioread32(pchan,
						DMA_CHALX_CTL);
			spin_lock(&chan_to_ddma(pchan)->lock);
			phytium_chan_irq_disable(pchan);
			spin_unlock(&chan_to_ddma(pchan)->lock);
			/* need reset when terminate */
			phytium_chan_reset(pchan);
			phytium_chan_irq_clear(pchan);
			/* recover it */
			phytium_chan_iowrite32(pchan,
						DMA_CHALX_CTL, tmp_ctl);
			phytium_chan_iowrite32(pchan,
						DMA_CHALX_TIMEOUT_CNT, timeout);
			pchan->busy = false;
		}
		pchan->desc = NULL;
	}

	vchan_get_all_descriptors(&pchan->vchan, &head);
	spin_unlock_irqrestore(&pchan->vchan.lock, flags);
	vchan_dma_desc_free_list(&pchan->vchan, &head);

	return 0;
}

static int phytium_ddma_alloc_chan_resources(struct dma_chan *chan)
{
	struct phytium_ddma_device *ddma = to_ddma_device(chan);
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	u32 bind_status = 0;
	int ret = 0;
	unsigned long flags = 0;

	bind_status = phytium_ddma_ioread32(ddma, DMA_CHAL_BIND);

	if ((pchan->is_used) || (bind_status & BIT(pchan->id))) {
		dev_err(ddma->dev, "channel %d already used\n", pchan->id);
		ret = -EBUSY;
		goto out;
	}

	/* prepare channel */
	ret = phytium_chan_disable(pchan);
	if (ret) {
		dev_err(ddma->dev, "can't disable channel %d\n", pchan->id);
		goto out;
	}
	phytium_chan_reset(pchan);
	phytium_chan_irq_clear(pchan);

	/* channel bind */
	spin_lock_irqsave(&chan_to_ddma(pchan)->lock, flags);
	bind_status |= BIT(pchan->id);
	phytium_ddma_iowrite32(ddma, DMA_CHAL_BIND, bind_status);
	pchan->is_used = true;
	spin_unlock_irqrestore(&chan_to_ddma(pchan)->lock, flags);

	/* alloc dma memory */
	pchan->buf = dma_alloc_coherent(ddma->dev, 4 * PAGE_SIZE, &pchan->paddr,
					GFP_KERNEL);
	if (!pchan->buf) {
		ret = -EBUSY;
		dev_err(ddma->dev, "failed to alloc dma memory\n");
	}

	dev_info(ddma->dev, "alloc channel %d\n", pchan->id);

out:
	return ret;
}

static void phytium_ddma_free_chan_resources(struct dma_chan *chan)
{
	struct phytium_ddma_device *ddma = to_ddma_device(chan);
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	u32 bind_status = 0;
	unsigned long flags = 0;

	if (!pchan->is_used)
		return;

	dev_dbg(ddma->dev, "free channel %d\n", pchan->id);
	spin_lock_irqsave(&chan_to_ddma(pchan)->lock, flags);
	bind_status = phytium_ddma_ioread32(ddma, DMA_CHAL_BIND);
	bind_status &= ~BIT(pchan->id);
	phytium_ddma_iowrite32(ddma, DMA_CHAL_BIND, bind_status);
	spin_unlock_irqrestore(&chan_to_ddma(pchan)->lock, flags);

	phytium_chan_disable(pchan);

	spin_lock_irqsave(&chan_to_ddma(pchan)->lock, flags);
	phytium_chan_irq_disable(pchan);
	spin_unlock_irqrestore(&chan_to_ddma(pchan)->lock, flags);

	vchan_free_chan_resources(to_virt_chan(chan));
	pchan->is_used = false;

	if (pchan->buf)
		dma_free_coherent(ddma->dev, 4 * PAGE_SIZE,
				pchan->buf, pchan->paddr);
}

static int phytium_ddma_slave_config(struct dma_chan *chan,
				  struct dma_slave_config *config)
{
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	u32 chal_cfg = 0;
	u32 req_mode = 0;
	const u32 timeout = 0xffff;
	unsigned long flag = 0;

	/* Check if chan will be configured for slave transfers */
	if (!is_slave_direction(config->direction))
		return -EINVAL;

	memcpy(&pchan->dma_config, config, sizeof(*config));

	/* set channel config reg */
	spin_lock_irqsave(&chan_to_ddma(pchan)->lock, flag);
	if (pchan->id > 3) {
		chal_cfg = phytium_ddma_ioread32(chan_to_ddma(pchan),
				DMA_CHAL_CFG_H);
		chal_cfg &= ~(0xFF << ((pchan->id - 4) * 8));
		chal_cfg |= DMA_CHAL_SEL((pchan->id - 4), pchan->request_line);
		chal_cfg |= DMA_CHAL_SEL_EN(pchan->id - 4);
		phytium_ddma_iowrite32(chan_to_ddma(pchan), DMA_CHAL_CFG_H,
				chal_cfg);
	} else {
		chal_cfg = phytium_ddma_ioread32(chan_to_ddma(pchan),
				DMA_CHAL_CFG_L);
		chal_cfg &= ~(0xFF << (pchan->id * 8));
		chal_cfg |= DMA_CHAL_SEL((pchan->id), pchan->request_line);
		chal_cfg |= DMA_CHAL_SEL_EN(pchan->id);
		phytium_ddma_iowrite32(chan_to_ddma(pchan), DMA_CHAL_CFG_L,
				chal_cfg);
	}
	spin_unlock_irqrestore(&chan_to_ddma(pchan)->lock, flag);

	/* set channel mode */
	req_mode = (config->direction == DMA_DEV_TO_MEM) ?
		DMA_RX_REQ : DMA_TX_REQ;
	phytium_chan_iowrite32(pchan, DMA_CHALX_CTL, req_mode << 2);

	/* set channel timeout */
	phytium_chan_iowrite32(pchan, DMA_CHALX_TIMEOUT_CNT,
			timeout | DMA_CHAL_TIMEOUT_EN);

	return 0;
}

static struct dma_async_tx_descriptor *phytium_ddma_prep_slave_sg(
	struct dma_chan *chan, struct scatterlist *sgl,
	u32 sg_len, enum dma_transfer_direction direction,
	unsigned long flags, void *context)
{
	struct phytium_ddma_device *ddma = to_ddma_device(chan);
	struct phytium_ddma_chan *pchan = to_ddma_chan(chan);
	struct dma_slave_config	*sconfig = &pchan->dma_config;
	struct phytium_ddma_desc *desc = NULL;
	struct scatterlist *sg = NULL;
	int i = 0;
	char *tmp;

	if (unlikely(!is_slave_direction(direction))) {
		dev_err(ddma->dev, "invalid dma direction\n");
		return NULL;
	}

	if (unlikely(sg_len < 1)) {
		dev_err(ddma->dev, "invalid segment length: %d\n", sg_len);
		return NULL;
	}

	desc = kzalloc(struct_size(desc, sg_req, sg_len), GFP_NOWAIT);
	if (!desc)
		return NULL;

	/* set sg list */
	for_each_sg(sgl, sg, sg_len, i) {
		tmp = phys_to_virt(sg_dma_address(sg));
		desc->sg_req[i].direction = direction;

		switch (direction) {
		case DMA_MEM_TO_DEV:
			desc->sg_req[i].len = sg_dma_len(sg);
			desc->sg_req[i].mem_addr_l =
				sg_dma_address(sg) & 0xFFFFFFFF;
			desc->sg_req[i].mem_addr_h =
				(sg_dma_address(sg) >> 32) & 0xFFFFFFFF;
			desc->sg_req[i].dev_addr =
				sconfig->dst_addr & 0xFFFFFFFF;
			break;

		case DMA_DEV_TO_MEM:
			desc->sg_req[i].len = sg_dma_len(sg);
			desc->sg_req[i].mem_addr_l =
				sg_dma_address(sg) & 0xFFFFFFFF;
			desc->sg_req[i].mem_addr_h =
				(sg_dma_address(sg) >> 32) & 0xFFFFFFFF;
			desc->sg_req[i].dev_addr =
				sconfig->src_addr & 0xFFFFFFFF;
			break;

		default:
			return NULL;
		}
	}

	desc->num_sgs = sg_len;

	return vchan_tx_prep(&pchan->vchan, &desc->vdesc, flags);
}

static irqreturn_t phytium_dma_interrupt(int irq, void *dev_id)
{
	struct phytium_ddma_device *ddma = dev_id;
	struct phytium_ddma_chan *chan;
	u32 irq_status = 0;
	u32 i = 0;
	u32 val = 0;

	phytium_ddma_irq_disable(ddma);

	irq_status = phytium_ddma_irq_read(ddma);
	val = phytium_ddma_ioread32(ddma, DMA_CTL);

	/* Poll, clear and process every chanel interrupt status */
	for (i = 0; i < ddma->dma_channels; i++) {
		if (!(irq_status & BIT(i * 4)))
			continue;

		chan = &ddma->chan[i];
		phytium_chan_xfer_done(chan);
	}

	phytium_ddma_irq_enable(ddma);

	return IRQ_HANDLED;
}


static struct dma_chan *phytium_ddma_of_xlate(struct of_phandle_args *dma_spec,
						struct of_dma *ofdma)
{
	struct phytium_ddma_device *ddma = ofdma->of_dma_data;
	struct device *dev = ddma->dev;
	struct phytium_ddma_chan *chan = NULL;
	struct dma_chan *c = NULL;
	u32 channel_id = 0;

	channel_id = dma_spec->args[0];

	if (channel_id > ddma->dma_channels) {
		dev_err(dev, "bad channel %d\n", channel_id);
		return NULL;
	}

	chan = &ddma->chan[channel_id];
	chan->request_line = dma_spec->args[1];
	c = dma_get_slave_channel(&chan->vchan.chan);
	if (!c) {
		dev_err(dev, "no more channels available\n");
		return NULL;
	}

	return c;
}

static int phytium_ddma_probe(struct platform_device *pdev)
{
	struct phytium_ddma_device *ddma;
	struct dma_device *dma_dev;
	struct resource *mem;
	u32 i = 0;
	int ret = 0;
	u32 nr_channels = 0;

	ddma = devm_kzalloc(&pdev->dev, sizeof(*ddma), GFP_KERNEL);
	if (!ddma) {
		ret = -ENOMEM;
		goto out;
	}

	dma_dev = &ddma->dma_dev;
	ddma->dev = &pdev->dev;

	spin_lock_init(&ddma->lock);

	ddma->irq = platform_get_irq(pdev, 0);
	if (ddma->irq < 0) {
		dev_err(&pdev->dev, "no irq resource\n");
		ret = -EINVAL;
		goto out;
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ddma->base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(ddma->base)) {
		dev_err(&pdev->dev, "no resource address");
		ret = PTR_ERR(ddma->base);
		goto out;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "dma-channels",
			&nr_channels);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"can't get the number of dma channels: %d\n", ret);
		goto out;
	}

	if (nr_channels > DDMA_MAX_NR_PCHANNELS) {
		dev_warn(&pdev->dev, "over the max number of channels\n");
		nr_channels = DDMA_MAX_NR_PCHANNELS;
	}

	ddma->dma_channels = DDMA_MAX_NR_PCHANNELS;

	ret = devm_request_irq(&pdev->dev, ddma->irq, phytium_dma_interrupt,
			       IRQF_SHARED, dev_name(&pdev->dev), ddma);
	if (ret) {
		dev_err(&pdev->dev, "could not to request irq %d", ddma->irq);
		goto out;
	}

	/* Set capabilities */
	dma_cap_set(DMA_SLAVE, ddma->dma_dev.cap_mask);

	/* DMA capabilities */
	dma_dev->dev = ddma->dev;
	dma_dev->chancnt = ddma->dma_channels;
	dma_dev->src_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);
	dma_dev->dst_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);
	dma_dev->directions = BIT(DMA_DEV_TO_MEM) | BIT(DMA_MEM_TO_DEV);
	dma_dev->residue_granularity = DMA_RESIDUE_GRANULARITY_DESCRIPTOR;

	/* function callback */
	dma_dev->device_tx_status = phytium_ddma_tx_status;
	dma_dev->device_issue_pending = phytium_ddma_issue_pending;
	dma_dev->device_terminate_all = phytium_ddma_terminate_all;
	dma_dev->device_alloc_chan_resources =
		phytium_ddma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = phytium_ddma_free_chan_resources;
	dma_dev->device_config = phytium_ddma_slave_config;
	dma_dev->device_prep_slave_sg = phytium_ddma_prep_slave_sg;
	dma_dev->device_pause = phytium_chan_pause;
	dma_dev->device_resume = phytium_chan_resume;

	/* init dma physical channels */
	INIT_LIST_HEAD(&dma_dev->channels);
	ddma->chan = devm_kcalloc(ddma->dev, ddma->dma_channels,
				sizeof(*ddma->chan), GFP_KERNEL);
	if (!ddma->chan) {
		ret = -ENOMEM;
		goto out;
	}
	for (i = 0; i < ddma->dma_channels; i++) {
		ddma->chan[i].id = i;
		ddma->chan[i].buf = NULL;
		ddma->chan[i].base = ddma->base + DMA_REG_LEN +
			i * CHAN_REG_LEN;
		ddma->chan[i].vchan.desc_free = phytium_ddma_vdesc_free;
		ddma->chan[i].desc = NULL;
		ddma->chan[i].current_sg = NULL;
		vchan_init(&ddma->chan[i].vchan, dma_dev);
	}

	phytium_dma_hw_init(ddma);

	ret = dma_async_device_register(dma_dev);
	if (ret)
		goto out;

	ret = of_dma_controller_register(pdev->dev.of_node,
			phytium_ddma_of_xlate, ddma);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"phytium ddma of register failed %d\n", ret);
		goto err_unregister;
	}

	platform_set_drvdata(pdev, ddma);
	dev_info(ddma->dev, "phytium DDMA Controller registered\n");

	return 0;

err_unregister:
	dma_async_device_unregister(dma_dev);

out:
	return ret;
}

static void phytium_ddma_chan_remove(struct phytium_ddma_chan *chan)
{
	phytium_chan_irq_disable(chan);
	phytium_chan_disable(chan);

	if (chan->buf)
		dma_free_coherent(chan_to_dev(chan), 4 * PAGE_SIZE, chan->buf,
				chan->paddr);

	tasklet_kill(&chan->vchan.task);
	list_del(&chan->vchan.chan.device_node);
}

static int phytium_ddma_remove(struct platform_device *pdev)
{
	struct phytium_ddma_device *ddma = platform_get_drvdata(pdev);
	struct phytium_ddma_chan *chan = NULL;
	int i = 0;

	of_dma_controller_free(pdev->dev.of_node);
	dma_async_device_unregister(&ddma->dma_dev);

	for (i = 0; i < ddma->dma_channels; i++) {
		chan = &ddma->chan[i];
		phytium_ddma_chan_remove(chan);
	}

	phytium_ddma_irq_disable(ddma);
	phytium_ddma_disable(ddma);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phytium_ddma_suspend(struct device *dev)
{
	struct phytium_ddma_device *ddma = dev_get_drvdata(dev);
	int i = 0;

	for (i = 0; i < ddma->dma_channels; i++) {
		if (phytium_chan_is_running(&ddma->chan[i])) {
			dev_warn(dev,
				"suspend is prevented by channel %d\n", i);
			return -EBUSY;
		}
	}

	ddma->dma_reg.dma_chal_cfg0 =
		phytium_ddma_ioread32(ddma, DMA_CHAL_CFG_L);
	ddma->dma_reg.dma_chal_bind =
		phytium_ddma_ioread32(ddma, DMA_CHAL_BIND);
	ddma->dma_reg.dma_chal_cfg1 =
		phytium_ddma_ioread32(ddma, DMA_CHAL_CFG_H);

	for (i = 0; i < ddma->dma_channels; i++) {
		struct phytium_ddma_chan *chan = &ddma->chan[i];

		if (!chan->is_used)
			continue;
		ddma->dma_chal_reg[i].dma_chalx_ctl =
			phytium_chan_ioread32(chan, DMA_CHALX_CTL);
		ddma->dma_chal_reg[i].dma_chalx_timeout_cnt =
			phytium_chan_ioread32(chan, DMA_CHALX_TIMEOUT_CNT);
	}

	phytium_ddma_irq_disable(ddma);
	phytium_ddma_disable(ddma);
	pm_runtime_force_suspend(dev);

	return 0;
}

static int phytium_ddma_resume(struct device *dev)
{
	struct phytium_ddma_device *ddma = dev_get_drvdata(dev);
	u32 i = 0;
	int ret = 0;

	phytium_dma_hw_init(ddma);
	phytium_ddma_iowrite32(ddma, DMA_CHAL_CFG_L,
		ddma->dma_reg.dma_chal_cfg0);
	phytium_ddma_iowrite32(ddma, DMA_CHAL_BIND,
		ddma->dma_reg.dma_chal_bind);
	phytium_ddma_iowrite32(ddma, DMA_CHAL_CFG_H,
		ddma->dma_reg.dma_chal_cfg1);

	for (i = 0; i < ddma->dma_channels; i++) {
		struct phytium_ddma_chan *chan = &ddma->chan[i];

		if (!chan->is_used)
			continue;
		phytium_chan_iowrite32(chan, DMA_CHALX_CTL,
			ddma->dma_chal_reg[i].dma_chalx_ctl);
		phytium_chan_iowrite32(chan, DMA_CHALX_TIMEOUT_CNT,
			ddma->dma_chal_reg[i].dma_chalx_timeout_cnt);
	}

	ret = pm_runtime_force_resume(dev);

	return ret;
}
#endif

static const struct dev_pm_ops phytium_ddma_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(phytium_ddma_suspend,
			phytium_ddma_resume)
};

static const struct of_device_id phytium_dma_of_id_table[] = {
	{ .compatible = "phytium,ddma" },
	{}
};
MODULE_DEVICE_TABLE(of, phytium_dma_of_id_table);

static struct platform_driver phytium_driver = {
	.probe		= phytium_ddma_probe,
	.remove		= phytium_ddma_remove,
	.driver = {
		.name	= "phytium-ddma",
		.of_match_table = of_match_ptr(phytium_dma_of_id_table),
		.pm = &phytium_ddma_pm_ops,
	},
};

module_platform_driver(phytium_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium DDMA Controller platform driver");
MODULE_AUTHOR("HuangJie <huangjie1663@phytium.com.cn>");
