// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium I2S ASoC driver
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 *
 * Derived from sound/soc/dwc/dwc-i2s.c
 *    Copyright (C) 2010 ST Microelectronics
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>
#include <linux/clocksource.h>
#include <linux/random.h>
#include <linux/timecounter.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <linux/pci.h>
#include <linux/acpi.h>
#include "local.h"

#define NUM_CAPTURE	1
#define NUM_PLAYBACK	1

#define PHYTIUM_I2S_PLAY	(1 << 0)
#define PHYTIUM_I2S_RECORD	(1 << 1)
#define PHYTIUM_I2S_SLAVE	(1 << 2)
#define PHYTIUM_I2S_MASTER	(1 << 3)

#define PHYTIUM_I2S_QUIRK_16BIT_IDX_OVERRIDE (1 << 2)

#define TWO_CHANNEL_SUPPORT	2	/* up to 2.0 */
#define FOUR_CHANNEL_SUPPORT	4	/* up to 3.1 */
#define SIX_CHANNEL_SUPPORT	6	/* up to 5.1 */
#define EIGHT_CHANNEL_SUPPORT	8	/* up to 7.1 */

struct pdata_px210_mfd {
	struct device		*dev;
	char *name;
	int clk_base;
};

static inline void i2s_write_reg(void __iomem *io_base, int reg, u32 val)
{
	writel(val, io_base + reg);
}

static inline u32 i2s_read_reg(void __iomem *io_base, int reg)
{
	return readl(io_base + reg);
}

static inline void i2s_disable_channels(struct i2s_phytium *dev, u32 stream)
{
	u32 i = 0;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		for (i = 0; i < 4; i++)
			i2s_write_reg(dev->regs, TER(i), 0);
	} else {
		for (i = 0; i < 4; i++)
			i2s_write_reg(dev->regs, RER(i), 0);
	}
}

static int substream_free_pages(struct azx *chip,
				struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

static void stream_update(struct i2sc_bus *bus, struct i2s_stream *s)
{
	struct azx *chip = bus_to_azx(bus);

	struct azx_dev *azx_dev = stream_to_azx_dev(s);

	/* check whether this IRQ is really acceptable */
	if (!chip->ops->position_check ||
	    chip->ops->position_check(chip, azx_dev)) {
		spin_unlock(&bus->reg_lock);
		snd_pcm_period_elapsed(azx_stream(azx_dev)->substream);
		spin_lock(&bus->reg_lock);
	}

}

int snd_i2s_bus_handle_stream_irq(struct i2sc_bus *bus, unsigned int status,
				    void (*ack)(struct i2sc_bus *,
						struct i2s_stream *))
{
	struct i2s_stream *azx_dev;
	u32 sd_status, qc_sd_status;
	int handled = 0;

	list_for_each_entry(azx_dev, &bus->stream_list, list) {

		if (status & azx_dev->sd_int_sta_mask) {
			sd_status = i2s_read_reg(azx_dev->sd_addr, DMA_STS);
			i2s_write_reg(azx_dev->sd_addr, DMA_STS, azx_dev->sd_int_sta_mask);
			qc_sd_status = i2s_read_reg(azx_dev->sd_addr, DMA_STS);
			handled |= 1 << azx_dev->index;
			azx_dev->running = 1;
			if (!azx_dev->substream || !azx_dev->running ||
			    !(sd_status & 0xffffffff)) {
				continue;
			}
			if (ack)
				ack(bus, azx_dev);
		}
	}
	return handled;
}

irqreturn_t azx_i2s_interrupt(int irq, void *dev_id)
{
	struct azx *chip = dev_id;
	struct i2sc_bus *bus = azx_bus(chip);
	u32 status;
	bool active, handled = false;
	int repeat = 0; /* count for avoiding endless loop */

	spin_lock(&bus->reg_lock);

	if (chip->disabled)
		goto unlock;

	do {

		status = i2s_read_reg(bus->remap_addr, DMA_STS);

		if (status == 0)
			break;

		handled = true;
		active = false;
		if (snd_i2s_bus_handle_stream_irq(bus, status, stream_update))
			active = true;


	} while (active && ++repeat < 1);

 unlock:
	spin_unlock(&bus->reg_lock);

	return IRQ_RETVAL(handled);
}

static int azx_acquire_irq(struct azx *chip, int do_disconnect)
{
	struct i2sc_bus *bus = azx_bus(chip);
	struct i2s_phytium *i2s = container_of(chip, struct i2s_phytium, chip);
	int err;

	err = devm_request_irq(i2s->dev, i2s->irq_id, azx_i2s_interrupt, IRQF_SHARED,
				"phytium i2s", chip);

	if (err < 0) {
		dev_err(i2s->dev, "failed to request irq\n");
		return err;
	}

	bus->irq = i2s->irq_id;

	return 0;
}

static void i2s_start(struct i2s_phytium *dev,
		      struct snd_pcm_substream *substream)
{
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->regs, ITER, 1);
	else
		i2s_write_reg(dev->regs, IRER, 1);

	/*enable the clock*/
	i2s_write_reg(dev->regs, CER, 1);

	/*enable the i2s*/
	i2s_write_reg(dev->regs, I2S_IER, 1);
}

static void i2s_stop(struct i2s_phytium *dev,
		struct snd_pcm_substream *substream)
{
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->regs, ITER, 0);
	else
		i2s_write_reg(dev->regs, IRER, 0);

	if (!dev->active) {
		i2s_write_reg(dev->regs, CER, 0);
		i2s_write_reg(dev->regs, I2S_IER, 0);
	}
}

static void phytium_i2s_config(struct i2s_phytium *dev, int stream)
{
	i2s_disable_channels(dev, stream);

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		i2s_write_reg(dev->regs, TCR(0), dev->xfer_resolution);
		i2s_write_reg(dev->regs, TER(0), 1);
	} else {
		i2s_write_reg(dev->regs, RCR(0), dev->xfer_resolution);
		i2s_write_reg(dev->regs, RER(0), 1);
	}
}

static int phytium_i2s_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(dai);
	struct i2s_clk_config_data *config = &dev->config;
	u64 fix, point;
	u32 cfg = 0;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		config->data_width = 16;
		dev->ccr = 0x00;
		dev->xfer_resolution = 0x02;
		break;

	case SNDRV_PCM_FORMAT_S24_LE:
		config->data_width = 24;
		dev->ccr = 0x08;
		dev->xfer_resolution = 0x04;
		break;

	case SNDRV_PCM_FORMAT_S32_LE:
		config->data_width = 32;
		dev->ccr = 0x10;
		dev->xfer_resolution = 0x05;
		break;

	default:
		dev_err(dev->dev, "phytium-i2s: unsupported PCM fmt");
		return -EINVAL;
	}

	config->chan_nr = params_channels(params);

	switch (config->chan_nr) {
	case EIGHT_CHANNEL_SUPPORT:
	case SIX_CHANNEL_SUPPORT:
	case FOUR_CHANNEL_SUPPORT:
	case TWO_CHANNEL_SUPPORT:
		break;
	default:
		dev_err(dev->dev, "channel not supported\n");
		return -EINVAL;
	}

	phytium_i2s_config(dev, substream->stream);

	i2s_write_reg(dev->regs, CCR, dev->ccr);

	config->sample_rate = params_rate(params);
	if (dev->capability & PHYTIUM_I2S_MASTER) {
		fix = dev->clk_base / config->sample_rate / config->data_width / 32;
		point = ((dev->clk_base / config->sample_rate) << 10) / config->data_width / 32;
		point = (point - (fix << 10)) * 10;
		cfg = ((u16) fix << 16) | (u16) point;
		i2s_write_reg(dev->regs, CLK_CFG0, cfg);
		i2s_write_reg(dev->regs, CLK_CFG1, 0xf);
	}
	return 0;
}

static int phytium_i2s_prepare(struct snd_pcm_substream *substream,
			  struct snd_soc_dai *dai)
{
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(dai);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->regs, TXFFR, 1);
	else
		i2s_write_reg(dev->regs, RXFFR, 1);

	return 0;
}

static int phytium_i2s_trigger(struct snd_pcm_substream *substream,
		int cmd, struct snd_soc_dai *dai)
{
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(dai);
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		dev->active++;
		i2s_start(dev, substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		dev->active--;
		i2s_stop(dev, substream);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int phytium_i2s_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(cpu_dai);
	int ret = 0;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		if (dev->capability & PHYTIUM_I2S_SLAVE)
			ret = 0;
		else
			ret = -EINVAL;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		if (dev->capability & PHYTIUM_I2S_MASTER)
			ret = 0;
		else
			ret = -EINVAL;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
	case SND_SOC_DAIFMT_CBS_CFM:
		ret = -EINVAL;
		break;
	default:
		dev_dbg(dev->dev, "phytium/i2s: Invalid master/slave format\n");
		ret = -EINVAL;
		break;
	}
	return ret;
}

static const struct snd_soc_dai_ops phytium_i2s_dai_ops = {
	.hw_params	= phytium_i2s_hw_params,
	.prepare	= phytium_i2s_prepare,
	.trigger	= phytium_i2s_trigger,
	.set_fmt	= phytium_i2s_set_fmt,
};

#ifdef CONFIG_PM

static int phytium_i2s_suspend(struct snd_soc_component *component)
{
	return 0;
}

static int phytium_i2s_resume(struct snd_soc_component *component)
{
	struct i2s_phytium *dev = snd_soc_component_get_drvdata(component);
	struct snd_soc_dai *dai;

	for_each_component_dais(component, dai) {
		if (snd_soc_dai_stream_active(dai, SNDRV_PCM_STREAM_PLAYBACK))
			phytium_i2s_config(dev, SNDRV_PCM_STREAM_PLAYBACK);
		if (snd_soc_dai_stream_active(dai, SNDRV_PCM_STREAM_CAPTURE))
			phytium_i2s_config(dev, SNDRV_PCM_STREAM_CAPTURE);
	}

	return 0;
}

#else
#define phytium_i2s_suspend NULL
#define phytium_i2s_resume  NULL
#endif

static struct snd_soc_dai_driver phytium_i2s_dai = {
	.playback = {
		.stream_name = "i2s-Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S8 |
			   SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S20_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.stream_name = "i2s-Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S8 |
			   SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S20_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops     = &phytium_i2s_dai_ops,
	.symmetric_rates = 1,
};

static const struct snd_pcm_hardware phytium_pcm_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.rates = SNDRV_PCM_RATE_8000 |
		SNDRV_PCM_RATE_32000 |
		SNDRV_PCM_RATE_44100 |
		SNDRV_PCM_RATE_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.formats = (SNDRV_PCM_FMTBIT_S8 |
		SNDRV_PCM_FMTBIT_S16_LE |
		SNDRV_PCM_FMTBIT_S20_LE |
		SNDRV_PCM_FMTBIT_S24_LE |
		SNDRV_PCM_FMTBIT_S32_LE),
	.channels_min = 2,
	.channels_max = 2,
	.buffer_bytes_max = 4096*16,
	.period_bytes_min = 1024,
	.period_bytes_max = 4096*4,
	.periods_min = 2,
	.periods_max = 16,
	.fifo_size = 16,
};

struct i2s_stream *snd_i2s_stream_assign(struct i2sc_bus *bus,
					   struct snd_pcm_substream *substream)
{
	struct i2s_stream *azx_dev;
	struct i2s_stream *res = NULL;

	/* make a non-zero unique key for the substream */
	int key = (substream->pcm->device << 16) | (substream->number << 2) |
		(substream->stream + 1);

	list_for_each_entry(azx_dev, &bus->stream_list, list) {
		if (azx_dev->direction != substream->stream)
			continue;

		azx_dev->opened = 0;

		if (azx_dev->assigned_key == key) {
			res = azx_dev;
			break;
		}

		if (!res || bus->reverse_assign)
			res = azx_dev;
	}

	if (res) {
		spin_lock_irq(&bus->reg_lock);
		res->opened = 1;
		res->running = 0;
		res->assigned_key = key;
		res->substream = substream;
		spin_unlock_irq(&bus->reg_lock);
	}

	return res;
}

/* assign a stream for the PCM */
static inline struct azx_dev *
azx_assign_device(struct azx *chip, struct snd_pcm_substream *substream)
{
	struct i2s_stream *s;

	s = snd_i2s_stream_assign(azx_bus(chip), substream);
	if (!s)
		return NULL;
	return stream_to_azx_dev(s);
}

static int phytium_pcm_open(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));
	struct azx *chip = &dev->chip;
	struct azx_dev *azx_dev;
	struct snd_pcm_runtime *runtime = substream->runtime;

	azx_dev = azx_assign_device(chip, substream);
	if (azx_dev == NULL)
		return -EBUSY;

	snd_soc_set_runtime_hwparams(substream, &phytium_pcm_hardware);
	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 128);
	runtime->private_data = azx_dev;

	return 0;
}

static int phytium_pcm_close(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));
	struct azx *chip = &dev->chip;
	struct azx_dev *azx_dev = get_azx_dev(substream);

	mutex_lock(&chip->open_mutex);
	azx_stream(azx_dev)->opened = 0;
	azx_stream(azx_dev)->running = 0;
	azx_stream(azx_dev)->substream = NULL;

	mutex_unlock(&chip->open_mutex);
	return 0;
}

static int phytium_pcm_new(struct snd_soc_component *component,
				struct snd_soc_pcm_runtime *rtd)
{
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));
	size_t size = phytium_pcm_hardware.buffer_bytes_max;

	snd_pcm_set_managed_buffer_all(rtd->pcm,
				      SNDRV_DMA_TYPE_DEV,
				      dev->pdev, size, size);

	return 0;
}

static const struct i2s_io_ops axi_i2s_io_ops;
static const struct i2s_controller_ops axi_i2s_ops;

static int phytium_pcm_hw_params(struct snd_soc_component *component,
					struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *hw_params)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	struct azx *chip = &dev->chip;
	struct azx_dev *azx_dev = get_azx_dev(substream);
	int ret;

	azx_dev->core.bufsize = 0;
	azx_dev->core.period_bytes = 0;
	azx_dev->core.format_val = 0;

	ret = chip->ops->substream_alloc_pages(chip, substream,
					  params_buffer_bytes(hw_params));

	return ret;
}
/*
 * set up a BDL entry
 */
static int setup_bdle(struct i2sc_bus *bus,
		      struct snd_dma_buffer *dmab,
		      struct i2s_stream *azx_dev, __le32 **bdlp,
		      int ofs, int size, int with_ioc)
{
	struct snd_pcm_substream *substream = azx_dev->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	__le32 *bdl = *bdlp;

	dmab->addr = runtime->dma_addr;
	while (size > 0) {
		dma_addr_t addr;
		int chunk;

		if (azx_dev->frags >= AZX_MAX_BDL_ENTRIES)
			return -EINVAL;

		addr = snd_sgbuf_get_addr(dmab, ofs);

		/* program the address field of the BDL entry */
		bdl[0] = cpu_to_le32((u32)addr);

		bdl[1] = cpu_to_le32(upper_32_bits(addr));

		/* program the size field of the BDL entry */
		chunk = snd_sgbuf_get_chunk_size(dmab, ofs, size);

		bdl[2] = cpu_to_le32(chunk);

		/* program the IOC to enable interrupt
		 * only when the whole fragment is processed
		 */
		size -= chunk;
		bdl[3] = (size || !with_ioc) ? 0 : cpu_to_le32(0x01);

		bdl += 4;
		azx_dev->frags++;
		ofs += chunk;
	}
	*bdlp = bdl;
	return ofs;
}

int snd_i2s_stream_setup_periods(struct i2s_stream *azx_dev)
{
	struct i2sc_bus *bus = azx_dev->bus;
	struct snd_pcm_substream *substream = azx_dev->substream;
	struct snd_pcm_runtime *runtime = substream->runtime;
	__le32 *bdl;
	int i, ofs, periods, period_bytes;
	int pos_adj, pos_align;

	period_bytes = azx_dev->period_bytes;
	periods = azx_dev->bufsize / period_bytes;

	/* program the initial BDL entries */
	bdl = (__le32 *)azx_dev->bdl.area;

	ofs = 0;
	azx_dev->frags = 0;

	pos_adj = bus->bdl_pos_adj;

	if (!azx_dev->no_period_wakeup && pos_adj > 0) {

		pos_align = pos_adj;
		pos_adj = (pos_adj * runtime->rate + 47999) / 48000;
		if (!pos_adj)
			pos_adj = pos_align;
		else
			pos_adj = ((pos_adj + pos_align - 1) / pos_align) *
				pos_align;
		pos_adj = frames_to_bytes(runtime, pos_adj);
		if (pos_adj >= period_bytes) {
			dev_warn(bus->dev, "Too big adjustment %d\n",
				 pos_adj);
			pos_adj = 0;
		} else {

			ofs = setup_bdle(bus, snd_pcm_get_dma_buf(substream),
					 azx_dev,
					 &bdl, ofs, pos_adj, true);
			if (ofs < 0)
				goto error;
		}
	} else
		pos_adj = 0;

	for (i = 0; i < periods; i++) {
		if (i == periods - 1 && pos_adj)
			ofs = setup_bdle(bus, snd_pcm_get_dma_buf(substream),
					 azx_dev, &bdl, ofs,
					 period_bytes - pos_adj, 0);
		else
			ofs = setup_bdle(bus, snd_pcm_get_dma_buf(substream),
					 azx_dev, &bdl, ofs,
					 period_bytes,
					 !azx_dev->no_period_wakeup);
		if (ofs < 0)
			goto error;
	}
	return 0;

 error:
	dev_err(bus->dev, "Too many BDL entries: buffer=%d, period=%d\n",
		azx_dev->bufsize, period_bytes);
	return -EINVAL;
}

int snd_i2s_stream_set_params(struct i2s_stream *azx_dev,
				 unsigned int format_val)
{
	unsigned int bufsize, period_bytes;
	struct snd_pcm_substream *substream = azx_dev->substream;
	struct snd_pcm_runtime *runtime;
	int err;

	if (!substream)
		return -EINVAL;

	runtime = substream->runtime;
	bufsize = snd_pcm_lib_buffer_bytes(substream);
	period_bytes = snd_pcm_lib_period_bytes(substream);
	if (bufsize != azx_dev->bufsize ||
	    period_bytes != azx_dev->period_bytes ||
	    format_val != azx_dev->format_val ||
	    runtime->no_period_wakeup != azx_dev->no_period_wakeup) {

		azx_dev->bufsize = bufsize;
		azx_dev->period_bytes = period_bytes;
		azx_dev->format_val = format_val;
		azx_dev->no_period_wakeup = runtime->no_period_wakeup;
		err = snd_i2s_stream_setup_periods(azx_dev);
		if (err < 0)
			return err;
	}

	return 0;
}

int snd_i2s_stream_setup(struct i2s_stream *azx_dev, int pcie, u32 paddr)
{
	struct snd_pcm_runtime *runtime;

	if (azx_dev->substream)
		runtime = azx_dev->substream->runtime;
	else
		runtime = NULL;

	i2s_write_reg(azx_dev->sd_addr, DMA_CHAL_CONFG0, 0x8180);
	i2s_write_reg(azx_dev->sd_addr, DMA_MASK_INT, 0x80000003);

	if (azx_dev->direction == SNDRV_PCM_STREAM_PLAYBACK) {
		i2s_write_reg(azx_dev->sd_addr, DMA_BDLPL(0), (u32)azx_dev->bdl.addr);
		i2s_write_reg(azx_dev->sd_addr, DMA_BDLPU(0), upper_32_bits(azx_dev->bdl.addr));
		if (pcie)
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DEV_ADDR(0), 0x1c8);
		else
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DEV_ADDR(0), paddr + 0x1c8);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CBL(0), azx_dev->bufsize);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_LVI(0), azx_dev->frags - 1);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DSIZE(0), 0x2);//0x2
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DLENTH(0), 0x0);//0x0
	} else {
		i2s_write_reg(azx_dev->sd_addr, DMA_BDLPL(1), (u32)azx_dev->bdl.addr);
		i2s_write_reg(azx_dev->sd_addr, DMA_BDLPU(1), upper_32_bits(azx_dev->bdl.addr));
		if (pcie)
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DEV_ADDR(1), 0x1c0);
		else
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DEV_ADDR(1), paddr + 0x1c0);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CBL(1), azx_dev->bufsize);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_LVI(1), azx_dev->frags - 1);
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DSIZE(1), 0x8);//0x8
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_DLENTH(1), 0x0);
	}

	if (runtime && runtime->period_size > 64)
		azx_dev->delay_negative_threshold =
			-frames_to_bytes(runtime, 64);
	else
		azx_dev->delay_negative_threshold = 0;

	return 0;
}

static int phytium_pcm_prepare(struct snd_soc_component *component,
			      struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	struct azx *chip = &dev->chip;
	struct azx_dev *azx_dev = get_azx_dev(substream);
	struct i2sc_bus *bus = azx_bus(chip);
	struct i2s_stream *hstr_p;
	int err;

	dev->substream = substream;
	azx_dev->core.substream = substream;
	azx_dev->core.sd_addr = dev->regs_db;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		azx_dev->core.bdl.area = bus->bdl0.area;
		azx_dev->core.bdl.addr = bus->bdl0.addr;
	} else {
		azx_dev->core.bdl.area = bus->bdl1.area;
		azx_dev->core.bdl.addr = bus->bdl1.addr;
	}

	if (!substream)
		return -EINVAL;

	hstr_p = azx_stream(azx_dev);
	hstr_p->direction = substream->stream;

	err = snd_i2s_stream_set_params(azx_stream(azx_dev), 0);
	if (err < 0)
		goto unlock;

	snd_i2s_stream_setup(azx_stream(azx_dev), dev->pcie, dev->paddr);

 unlock:
	if (!err)
		azx_stream(azx_dev)->prepared = 1;

	return err;
}

void snd_i2s_stream_clear(struct i2s_stream *azx_dev)
{
	if (azx_dev->direction == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0), 0x0);
	else
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1), 0x0);

	azx_dev->running = false;
}

void snd_i2s_stream_stop(struct i2s_stream *azx_dev)
{
	snd_i2s_stream_clear(azx_dev);
}

void snd_i2s_stream_start(struct i2s_stream *azx_dev, bool fresh_start)
{
	if (azx_dev->direction == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0), 0x1);
	else
		i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1), 0x5);

	azx_dev->running = true;
}

static int phytium_pcm_trigger(struct snd_soc_component *component,
			       struct snd_pcm_substream *substream, int cmd)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	struct azx *chip = &dev->chip;
	struct i2sc_bus *bus = azx_bus(chip);
	struct azx_dev *azx_dev = get_azx_dev(substream);
	struct snd_pcm_substream *s;
	struct i2s_stream *hstr;
	bool start;
	int sbits = 0;

	hstr = azx_stream(azx_dev);
	hstr->direction = substream->stream;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_RESUME:
		start = true;
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_STOP:
		start = false;
		break;
	default:
		return -EINVAL;
	}

	snd_pcm_group_for_each_entry(s, substream) {
		if (s->pcm->card != substream->pcm->card)
			continue;
		azx_dev = get_azx_dev(s);
		sbits |= 1 << azx_dev->core.index;
		snd_pcm_trigger_done(s, substream);
	}

	spin_lock(&bus->reg_lock);

	snd_pcm_group_for_each_entry(s, substream) {
		if (s->pcm->card != substream->pcm->card)
			continue;
		azx_dev = get_azx_dev(s);
		if (start)
			snd_i2s_stream_start(azx_stream(azx_dev), true);
		else
			snd_i2s_stream_stop(azx_stream(azx_dev));
	}

	i2s_write_reg(dev->regs_db, DMA_CTL, 0x1);
	spin_unlock(&bus->reg_lock);

	return 0;
}

void snd_i2s_stream_cleanup(struct i2s_stream *azx_dev)
{
	int cnt = 10;
	u32 mask;

	if (azx_dev->sd_addr) {
		if (azx_dev->direction == SNDRV_PCM_STREAM_PLAYBACK) {
			mask = i2s_read_reg(azx_dev->sd_addr, DMA_MASK_INT);
			mask &= ~BIT(0);
			i2s_write_reg(azx_dev->sd_addr, DMA_MASK_INT, mask);
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0), 0);
			while (cnt--) {
				if (i2s_read_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0)) == 0)
					break;
			}
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0), 2);
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(0), 0);
			i2s_write_reg(azx_dev->sd_addr, DMA_BDLPL(0), 0);
			i2s_write_reg(azx_dev->sd_addr, DMA_BDLPU(0), 0);
		} else {
			mask = i2s_read_reg(azx_dev->sd_addr, DMA_MASK_INT);
			mask &= ~BIT(1);
			i2s_write_reg(azx_dev->sd_addr, DMA_MASK_INT, mask);
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1), 0);
			while (cnt--) {
				if (i2s_read_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1)) == 0)
					break;
			}
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1), 2);
			i2s_write_reg(azx_dev->sd_addr, DMA_CHALX_CTL(1), 0);
			i2s_write_reg(azx_dev->sd_addr, DMA_BDLPL(1), 0);
			i2s_write_reg(azx_dev->sd_addr, DMA_BDLPU(1), 0);
		}
	}
}

static int phytium_pcm_hw_free(struct snd_soc_component *component,
			       struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	struct azx *chip = &dev->chip;
	struct i2s_stream *hstr_p;
	struct azx_dev *azx_dev = get_azx_dev(substream);
	int err;

	hstr_p = azx_stream(azx_dev);
	hstr_p->direction = substream->stream;
	snd_i2s_stream_cleanup(azx_stream(azx_dev));

	err = chip->ops->substream_free_pages(chip, substream);
	azx_stream(azx_dev)->prepared = 0;

	return err;
}

static snd_pcm_uframes_t phytium_pcm_pointer(struct snd_soc_component *component,
					     struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct i2s_phytium *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	int stream = substream->stream;

	u32 pos = i2s_read_reg(dev->regs_db, DMA_LPIB(stream));

	return bytes_to_frames(substream->runtime, pos);
}

static const struct snd_soc_component_driver phytium_i2s_component = {
	.name		= "phytium-i2s",
	.pcm_construct  = phytium_pcm_new,
	.suspend = phytium_i2s_suspend,
	.resume  = phytium_i2s_resume,

	.open = phytium_pcm_open,
	.close = phytium_pcm_close,
	.hw_params = phytium_pcm_hw_params,
	.prepare = phytium_pcm_prepare,
	.hw_free = phytium_pcm_hw_free,
	.trigger = phytium_pcm_trigger,
	.pointer = phytium_pcm_pointer,
};

/* Maximum bit resolution of a channel - not uniformly spaced */
static const u32 fifo_width[COMP_MAX_WORDSIZE] = {
	12, 16, 20, 24, 32, 0, 0, 0
};

/* Width of (DMA) bus */
static const u32 bus_widths[COMP_MAX_DATA_WIDTH] = {
	DMA_SLAVE_BUSWIDTH_1_BYTE,
	DMA_SLAVE_BUSWIDTH_2_BYTES,
	DMA_SLAVE_BUSWIDTH_4_BYTES,
	DMA_SLAVE_BUSWIDTH_UNDEFINED
};

/* PCM format to support channel resolution */
static const u32 formats[COMP_MAX_WORDSIZE] = {
	SNDRV_PCM_FMTBIT_S16_LE,
	SNDRV_PCM_FMTBIT_S16_LE,
	SNDRV_PCM_FMTBIT_S24_LE,
	SNDRV_PCM_FMTBIT_S24_LE,
	SNDRV_PCM_FMTBIT_S32_LE,
	0,
	0,
	0
};

static int phytium_configure_dai(struct i2s_phytium *dev)
{
	u32 comp1 = i2s_read_reg(dev->regs, dev->i2s_reg_comp1);
	u32 comp2 = i2s_read_reg(dev->regs, dev->i2s_reg_comp2);
	u32 fifo_depth = 1 << (1 + COMP1_FIFO_DEPTH_GLOBAL(comp1));
	u32 idx;

	if (COMP1_TX_ENABLED(comp1)) {
		dev_dbg(dev->dev, " phytium: play supported\n");
		idx = COMP1_TX_WORDSIZE_0(comp1);
		if (WARN_ON(idx >= ARRAY_SIZE(formats)))
			return -EINVAL;
	}

	if (COMP1_RX_ENABLED(comp1)) {
		dev_dbg(dev->dev, "phytium: record supported\n");
		idx = COMP2_RX_WORDSIZE_0(comp2);
		if (WARN_ON(idx >= ARRAY_SIZE(formats)))
			return -EINVAL;
		if (dev->quirks & PHYTIUM_I2S_QUIRK_16BIT_IDX_OVERRIDE)
			idx = 1;
	}

	if (COMP1_MODE_EN(comp1)) {
		dev_dbg(dev->dev, "phytium: i2s master mode supported\n");
		dev->capability |= PHYTIUM_I2S_MASTER;
	} else {
		dev_dbg(dev->dev, "phytium: i2s slave mode supported\n");
		dev->capability |= PHYTIUM_I2S_SLAVE;
	}

	dev->fifo_th = fifo_depth / 2;
	return 0;
}

static int phytium_configure_dai_by_dt(struct i2s_phytium *dev)
{
	u32 comp1 = i2s_read_reg(dev->regs, I2S_COMP_PARAM_1);
	u32 comp2 = i2s_read_reg(dev->regs, I2S_COMP_PARAM_2);
	u32 idx = COMP1_APB_DATA_WIDTH(comp1);
	u32 idx2;
	int ret;

	if (WARN_ON(idx >= ARRAY_SIZE(bus_widths)))
		return -EINVAL;

	ret = phytium_configure_dai(dev);
	if (ret < 0)
		return ret;

	if (COMP1_TX_ENABLED(comp1)) {
		idx2 = COMP1_TX_WORDSIZE_0(comp1);
		dev->capability |= PHYTIUM_I2S_PLAY;
	}
	if (COMP1_RX_ENABLED(comp1)) {
		idx2 = COMP2_RX_WORDSIZE_0(comp2);
		dev->capability |= PHYTIUM_I2S_RECORD;
	}

	return 0;
}

static int phytium_dma_alloc_pages(struct i2sc_bus *bus, int type, size_t size,
			       struct snd_dma_buffer *buf)
{
	int err;

	err = snd_dma_alloc_pages(type, bus->dev, size, buf);
	if (err < 0)
		return err;

	return 0;
}

int snd_i2s_bus_alloc_stream_pages(struct i2sc_bus *bus)
{
	struct i2s_stream *s;
	int num_streams = 0;
	int err;

	list_for_each_entry(s, &bus->stream_list, list) {

		/* allocate memory for the BDL for each stream */
		err = bus->io_ops->dma_alloc_pages(bus, SNDRV_DMA_TYPE_DEV,
						   BDL_SIZE, &s->bdl);
		if (num_streams == 0) {
			bus->bdl0.addr = s->bdl.addr;
			bus->bdl0.area = s->bdl.area;
		} else {
			bus->bdl1.addr = s->bdl.addr;
			bus->bdl1.area = s->bdl.area;
		}
		num_streams++;
		if (err < 0)
			return -ENOMEM;
	}

	if (WARN_ON(!num_streams))
		return -EINVAL;

	return 0;
}

static int stream_direction(struct azx *chip, unsigned char index)
{
	if (index >= chip->playback_index_offset &&
	    index < chip->playback_index_offset + chip->playback_streams)
		return SNDRV_PCM_STREAM_PLAYBACK;
	return SNDRV_PCM_STREAM_CAPTURE;

}

void snd_i2s_stream_init(struct i2sc_bus *bus, struct i2s_stream *azx_dev,
			  int idx, int direction, int tag)
{
	azx_dev->bus = bus;
	azx_dev->sd_addr = bus->remap_addr;

	if (idx == 0)
		azx_dev->sd_int_sta_mask = 1 << idx;
	else
		azx_dev->sd_int_sta_mask = 1 << 8;

	azx_dev->index = idx;
	azx_dev->direction = direction;
	azx_dev->stream_tag = tag;

	list_add_tail(&azx_dev->list, &bus->stream_list);

}

int azx_i2s_init_streams(struct azx *chip)
{
	int i;

	for (i = 0; i < chip->num_streams; i++) {
		struct azx_dev *azx_dev = kzalloc(sizeof(*azx_dev), GFP_KERNEL);
		int dir, tag;

		if (!azx_dev)
			return -ENOMEM;

		dir = stream_direction(chip, i);

		tag = i + 1;

		snd_i2s_stream_init(azx_bus(chip), azx_stream(azx_dev),
				     i, dir, tag);
	}

	return 0;
}

static int azx_first_init(struct azx *chip)
{
	struct i2s_phytium *i2s = container_of(chip, struct i2s_phytium, chip);
	struct platform_device *pdev = to_platform_device(i2s->dev);
	struct device *i2sdev = i2s->dev;
	struct i2sc_bus *bus = azx_bus(chip);
	struct resource *res;
	int err;
	unsigned int dma_bits = 64;

	chip->region_requested = 1;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	bus->addr = res->start;
	bus->remap_addr = i2s->regs_db;
	bus->dev = i2s->pdev;

	if (bus->remap_addr == NULL) {
		dev_err(i2sdev, "ioremap error\n");
		return -ENXIO;
	}

	if (azx_acquire_irq(chip, 0) < 0)
		return -EBUSY;

	synchronize_irq(bus->irq);

	spin_lock_init(&bus->reg_lock);

	if (!dma_set_mask(i2sdev, DMA_BIT_MASK(dma_bits))) {
		err = dma_set_coherent_mask(i2sdev, DMA_BIT_MASK(dma_bits));
	} else {
		err = dma_set_mask(i2sdev, DMA_BIT_MASK(32));
		err = dma_set_coherent_mask(i2sdev, DMA_BIT_MASK(32));
	}

	chip->playback_streams = NUM_PLAYBACK;
	chip->capture_streams  = NUM_CAPTURE;

	chip->playback_index_offset = 0;
	chip->capture_index_offset = chip->playback_streams;
	chip->num_streams = chip->playback_streams + chip->capture_streams;

	err = azx_i2s_init_streams(chip);
	if (err < 0)
		return err;

	err = azx_alloc_stream_pages(chip);
	if (err < 0)
		return err;

	return 0;
}

static int azx_probe_continue(struct azx *chip)
{
	struct i2s_phytium *i2s = container_of(chip, struct i2s_phytium, chip);
	int err;

	i2s->probe_continued = 1;

	err = azx_first_init(chip);
	if (err < 0)
		goto out_free;

	chip->running = 1;

out_free:
	return err;
}

static void azx_probe_work(struct work_struct *work)
{
	struct i2s_phytium *i2s = container_of(work, struct i2s_phytium, probe_work);

	azx_probe_continue(&i2s->chip);
}

int azx_i2s_bus_init(struct azx *chip,
		 const struct i2s_io_ops *io_ops)
{
	struct i2s_bus *bus = &chip->bus;

	bus->core.io_ops = io_ops;

	INIT_LIST_HEAD(&bus->core.stream_list);
	bus->card = chip->card;
	mutex_init(&bus->prepare_mutex);
	bus->pci = chip->pci;

	bus->core.bdl_pos_adj = chip->bdl_pos_adj;
	return 0;
}

static int i2s_phytium_create(struct platform_device *pdev,
			int dev, struct azx **rchip, struct i2s_phytium *i2s)
{
	struct azx *chip;
	int err;

	*rchip = NULL;

	if (!i2s)
		return -ENOMEM;
	chip = &i2s->chip;

	mutex_init(&chip->open_mutex);

	chip->ops = &axi_i2s_ops;
	chip->dev_index = dev;

	INIT_LIST_HEAD(&chip->pcm_list);
	init_completion(&i2s->probe_wait);

	chip->bdl_pos_adj = 32;
	err = azx_i2s_bus_init(chip, &axi_i2s_io_ops);
	if (err < 0) {
		kfree(i2s);
		return err;
	}

	INIT_WORK(&i2s->probe_work, azx_probe_work);
	*rchip = chip;
	return 0;
}

static int substream_alloc_pages(struct azx *chip,
				 struct snd_pcm_substream *substream,
				 size_t size)
{
	int ret;

	ret = snd_pcm_lib_malloc_pages(substream, size);
	if (ret < 0)
		return ret;

	return 0;
}

static void phytium_dma_free_pages(struct i2sc_bus *bus,
			       struct snd_dma_buffer *buf)
{
	snd_dma_free_pages(buf);
}

static const struct i2s_io_ops axi_i2s_io_ops = {
	.dma_alloc_pages = phytium_dma_alloc_pages,
	.dma_free_pages = phytium_dma_free_pages,
};

static const struct i2s_controller_ops axi_i2s_ops = {
	.substream_alloc_pages = substream_alloc_pages,
	.substream_free_pages = substream_free_pages,
};


static int phytium_i2s_probe(struct platform_device *pdev)
{
	struct i2s_phytium *i2s;
	struct azx *chip;
	struct resource *res;
	struct pdata_px210_mfd *pdata;
	struct snd_soc_dai_driver *dai_drv;
	struct clk *clk;
	int err, ret;
	int card_num = 1;
	bool schedule_probe;
	struct fwnode_handle *np;

	i2s = devm_kzalloc(&pdev->dev, sizeof(*i2s), GFP_KERNEL);
	if (!i2s)
		return -ENOMEM;

	dai_drv = devm_kzalloc(&pdev->dev, sizeof(*dai_drv), GFP_KERNEL);
	if (!dai_drv)
		return -ENOMEM;
	memcpy(dai_drv, &phytium_i2s_dai, sizeof(phytium_i2s_dai));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2s->paddr = res->start;
	i2s->regs = devm_ioremap_resource(&pdev->dev, res);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	i2s->regs_db = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(i2s->regs))
		return PTR_ERR(i2s->regs);

	i2s->irq_id = platform_get_irq(pdev, 0);

	if (i2s->irq_id < 0)
		return i2s->irq_id;

	i2s->i2s_reg_comp1 = I2S_COMP_PARAM_1;
	i2s->i2s_reg_comp2 = I2S_COMP_PARAM_2;

	ret = phytium_configure_dai_by_dt(i2s);
	if (ret < 0)
		return ret;

	err = i2s_phytium_create(pdev, card_num, &chip, i2s);
	if (err < 0)
		return err;
	i2s = container_of(chip, struct i2s_phytium, chip);
	schedule_probe = !chip->disabled;

	dev_set_drvdata(&pdev->dev, i2s);

	pdata = dev_get_platdata(&pdev->dev);
	i2s->dev = &pdev->dev;
	if (pdata) {
		dai_drv->name = pdata->name;
		i2s->pdev = pdata->dev;
		i2s->clk_base = pdata->clk_base;
		i2s->pcie = 1;
	} else if (pdev->dev.of_node) {
		device_property_read_string(&pdev->dev, "dai-name", &dai_drv->name);
		i2s->pdev = &pdev->dev;
		clk = devm_clk_get(&pdev->dev, NULL);
		i2s->clk_base = clk_get_rate(clk);
	} else if (has_acpi_companion(&pdev->dev)) {
		np = dev_fwnode(&(pdev->dev));
		ret = fwnode_property_read_string(np, "dai-name", &dai_drv->name);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing dai-name property from acpi\n");
			goto failed_get_dai_name;
		}
		i2s->pdev = &pdev->dev;
		ret = fwnode_property_read_u32(np, "i2s_clk", &i2s->clk_base);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing i2s_clk property from acpi\n");
			goto failed_get_dai_name;
		}
	}
	ret = devm_snd_soc_register_component(&pdev->dev, &phytium_i2s_component,
						 dai_drv, 1);
	if (ret != 0)
		dev_err(&pdev->dev, "not able to register dai\n");

	if (schedule_probe)
		schedule_work(&i2s->probe_work);

	if (chip->disabled)
		complete_all(&i2s->probe_wait);

	return 0;
failed_get_dai_name:
	return ret;
}

static int phytium_i2s_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);
	return 0;
}

static const struct of_device_id phytium_i2s_of_match[] = {
	{ .compatible = "phytium,i2s", },
	{},
};
MODULE_DEVICE_TABLE(of, phytium_i2s_of_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_i2s_acpi_match[] = {
	{ "PHYT0016", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, phytium_i2s_acpi_match);
#else
#define phytium_i2s_acpi_match NULL
#endif

static struct platform_driver phytium_i2s_driver = {
	.probe	= phytium_i2s_probe,
	.remove	= phytium_i2s_remove,
	.driver	= {
		.name = "phytium-i2s",
		.of_match_table = of_match_ptr(phytium_i2s_of_match),
		.acpi_match_table = phytium_i2s_acpi_match,
	},
};

module_platform_driver(phytium_i2s_driver);

MODULE_DESCRIPTION("Phytium I2S Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhang Yiqun <zhangyiqun@phytium.com.cn>");
