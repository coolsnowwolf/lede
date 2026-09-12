// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium I2S ASoC driver
 *
 * Copyright (C) 2023-2024, Phytium Technology Co., Ltd.
 *
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
#include <linux/version.h>
#include <linux/acpi.h>
#include <sound/jack.h>
#include "phytium-i2s-v2.h"

#define PHYT_I2S_V2_VERSION "1.0.9"

static struct snd_soc_jack hs_jack;
static irqreturn_t phyt_i2s_gpio_interrupt(int irq, void *dev_id);
/* Headset jack detection DAPM pins */
static struct snd_soc_jack_pin hs_jack_pins[] = {
	{
		.pin	= "Front",
		.mask	= SND_JACK_HEADPHONE,
	},
};

static struct snd_pcm_hardware phytium_pcm_hardware = {
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
		SNDRV_PCM_FMTBIT_S24_LE),
	.channels_min = 2,
	.channels_max = 4,
	.buffer_bytes_max = 4096*16,
	.period_bytes_min = 1024,
	.period_bytes_max = 4096*4,
	.periods_min = 2,
	.periods_max = 16,
};

static int phyt_pcm_new(struct snd_soc_component *component,
				struct snd_soc_pcm_runtime *rtd)
{
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	size_t size = phytium_pcm_hardware.buffer_bytes_max;
	int ret = 0;

	snd_pcm_lib_preallocate_pages_for_all(rtd->pcm,
				      SNDRV_DMA_TYPE_DEV,
				      priv->dev, size, size);

	if (ret < 0) {
		dev_err(priv->dev, "can't alloc preallocate buffer\n");
		goto failed_preallocate_buffer;
	}

	ret = snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, priv->dev, BDL_SIZE,
				&priv->pcm_config[0].bdl_dmab);
	if (ret < 0) {
		dev_err(priv->dev, "can't alloc bdl0 buffer\n");
		goto failed_alloc_bdl0;
	}

	ret = snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, priv->dev, BDL_SIZE,
				&priv->pcm_config[1].bdl_dmab);
	if (ret < 0) {
		dev_err(priv->dev, "can't alloc bdl1 buffer\n");
		goto failed_alloc_bdl1;
	}
	return 0;

failed_alloc_bdl1:
	snd_dma_free_pages(&priv->pcm_config[0].bdl_dmab);
failed_alloc_bdl0:
	snd_pcm_lib_preallocate_free_for_all(rtd->pcm);
failed_preallocate_buffer:
	return ret;
}

static void phyt_pcm_free(struct snd_soc_component *component,
			     struct snd_pcm *pcm)
{
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));

	snd_dma_free_pages(&priv->pcm_config[1].bdl_dmab);
	snd_dma_free_pages(&priv->pcm_config[0].bdl_dmab);
	snd_pcm_lib_preallocate_free_for_all(pcm);
}

static int phyt_pcm_open(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	struct snd_pcm_runtime *runtime = substream->runtime;

	if (!priv->i2s_dp)
		phytium_pcm_hardware.formats |= SNDRV_PCM_FMTBIT_S32_LE;
	snd_soc_set_runtime_hwparams(substream, &phytium_pcm_hardware);
	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 128);
	runtime->private_data = priv;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		priv->substream_playback = substream;
	else
		priv->substream_capture = substream;

	return 0;
}

static int phyt_pcm_close(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		priv->substream_playback = NULL;
	else
		priv->substream_capture = NULL;

	return 0;
}

static int phyt_pcm_hw_params(struct snd_soc_component *component,
					struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *hw_params)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	int ret = 0;
	uint32_t format_val;

	switch (params_format(hw_params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		format_val = BYTE_2;
		break;

	case SNDRV_PCM_FORMAT_S24_LE:
		format_val = BYTE_4;
		break;

	case SNDRV_PCM_FORMAT_S32_LE:
		format_val = BYTE_4;
		break;

	default:
		dev_err(priv->dev, "phytium-i2s: unsupported PCM fmt");
		return -EINVAL;
	}


	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		priv->pcm_config[DIRECTION_PLAYBACK].format_val = format_val;
	else
		priv->pcm_config[DIRECTION_CAPTURE].format_val = format_val;

	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));

	return ret;
}

static void phyt_bdl_entry_setup(dma_addr_t addr, uint32_t **pbdl,
				uint32_t period_bytes, uint32_t with_ioc)
{
	uint32_t *bdl = *pbdl;
	uint32_t chunk;

	while (period_bytes > 0) {
		bdl[0] = cpu_to_le32((u32)addr);
		bdl[1] = cpu_to_le32(upper_32_bits(addr));
		if (period_bytes > BDL_BUFFER_MAX_SIZE)
			chunk = BDL_BUFFER_MAX_SIZE;
		else
			chunk = period_bytes;

		bdl[2] = cpu_to_le32(chunk);
		period_bytes -= chunk;
		bdl[3] = (period_bytes || !with_ioc) ? 0 : cpu_to_le32(0x01);
		bdl += 4;
	}

	*pbdl = bdl;
}


static int phyt_pcm_prepare(struct snd_soc_component *component,
			      struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct phytium_pcm_config *pcm_config;
	int periods = 0, i, offset = 0;
	uint32_t *bdl = NULL;
	struct snd_dma_buffer *data_dmab = NULL;
	int direction;
	uint32_t frags = 0;
	uint32_t config = 0, format_val;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		priv->pcm_config[DIRECTION_PLAYBACK].buffer_size =
				snd_pcm_lib_buffer_bytes(substream);
		priv->pcm_config[DIRECTION_PLAYBACK].period_bytes =
				snd_pcm_lib_period_bytes(substream);
		priv->pcm_config[DIRECTION_PLAYBACK].no_period_wakeup =
				runtime->no_period_wakeup;
		direction = DIRECTION_PLAYBACK;
		format_val = priv->pcm_config[DIRECTION_PLAYBACK].format_val;
	} else {
		priv->pcm_config[DIRECTION_CAPTURE].buffer_size =
				snd_pcm_lib_buffer_bytes(substream);
		priv->pcm_config[DIRECTION_CAPTURE].period_bytes =
				snd_pcm_lib_period_bytes(substream);
		priv->pcm_config[DIRECTION_CAPTURE].no_period_wakeup =
				runtime->no_period_wakeup;
		direction = DIRECTION_CAPTURE;
		format_val = priv->pcm_config[DIRECTION_CAPTURE].format_val;
	}

	pcm_config = &priv->pcm_config[direction];

	periods = pcm_config->buffer_size / pcm_config->period_bytes;
	bdl = (uint32_t *)pcm_config->bdl_dmab.area;
	data_dmab = snd_pcm_get_dma_buf(substream);
	for (i = 0; i < periods; i++) {
		phyt_bdl_entry_setup(data_dmab->addr + offset, &bdl,
					pcm_config->period_bytes,
					!pcm_config->no_period_wakeup);
		offset += pcm_config->period_bytes;
		frags += DIV_ROUND_UP(pcm_config->period_bytes, BDL_BUFFER_MAX_SIZE);
	}

	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHAL_CONFG0, CHANNEL_0_1_ENABLE);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_MASK_INT, CHANNEL_0_1_INT_MASK);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_BDLPL(direction),
			(uint32_t)(pcm_config->bdl_dmab.addr));
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_BDLPU(direction),
			upper_32_bits(pcm_config->bdl_dmab.addr));
	if (direction == DIRECTION_PLAYBACK)
		config = PLAYBACK_ADDRESS_OFFSET;
	else
		config = CAPTRUE_ADDRESS_OFFSET;
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_DEV_ADDR(direction),
			LSD_I2S_BASE + config);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_LVI(direction), frags - 1);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_CBL(direction),
			pcm_config->buffer_size);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_DSIZE(direction),
			D_SIZE(format_val, direction));
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_DLENTH(direction), D_LENTH);

	return 0;
}

static int phyt_pcm_hw_free(struct snd_soc_component *component,
			       struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	int direction;
	u32 mask;
	int cnt = 10;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		direction = DIRECTION_PLAYBACK;
	else
		direction = DIRECTION_CAPTURE;

	mask = readl(priv->dma_reg_base + PHYTIUM_DMA_MASK_INT);
	mask &= ~BIT(direction);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_MASK_INT, mask);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_CTL(direction), 0);
	while (cnt--) {
		if (readl(priv->dma_reg_base + PHYTIUM_DMA_CHALX_CTL(direction)) == 0)
			break;
		udelay(200);
	}
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_CTL(direction), 2);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_CTL(direction), 0);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_STS, DMA_TX_DONE);
	else
		phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_STS, DMA_RX_DONE);

	return snd_pcm_lib_free_pages(substream);
}

static int phyt_pcm_trigger(struct snd_soc_component *component,
			       struct snd_pcm_substream *substream, int cmd)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	bool start =  false;
	int direction = 0, value = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		start = true;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		start = false;
		break;
	default:
		return -EINVAL;
	}

	direction = ((substream->stream ==
		 SNDRV_PCM_STREAM_PLAYBACK) ? DIRECTION_PLAYBACK:DIRECTION_CAPTURE);

	if (start)
		value = (substream->stream ==
			SNDRV_PCM_STREAM_PLAYBACK) ? PLAYBACK_START:CAPTURE_START;
	else
		value = CTL_STOP;

	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CHALX_CTL(direction), value);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CTL, DMA_ENABLE);

	return 0;
}

static snd_pcm_uframes_t phyt_pcm_pointer(struct snd_soc_component *component,
					    struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(snd_soc_rtd_to_cpu(rtd, 0));
	uint32_t pos = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		pos = readl(priv->dma_reg_base + PHYTIUM_DMA_LPIB(DIRECTION_PLAYBACK));
	else
		pos = readl(priv->dma_reg_base + PHYTIUM_DMA_LPIB(DIRECTION_CAPTURE));

	return bytes_to_frames(substream->runtime, pos);
}

static int phyt_i2s_msg_set_cmd(struct phytium_i2s *priv, bool is_gpio)
{
	struct phyti2s_cmd *ans_msg;
	int timeout = 100, ret = 0;

	if (is_gpio) {
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_AP2RV_INT_STATE,
				SEND_GPIO_INTR);
		ans_msg = priv->sharemem_base + PHYTIUM_GPIO_OFFSET;
	} else {
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_AP2RV_INT_STATE, SEND_INTR);
		ans_msg = priv->sharemem_base;
	}

	while ((ans_msg->complete == PHYTI2S_COMPLETE_NONE
			|| ans_msg->complete == PHYTI2S_COMPLETE_GOING)
			&& timeout) {
		udelay(500);
		timeout--;
	}

	if (timeout == 0) {
		dev_err(priv->dev, "wait cmd reply timeout\n");
		ret = -EBUSY;
	}

	if (ans_msg->complete == PHYTI2S_COMPLETE_ERROR) {
		dev_err(priv->dev, "handle cmd failed\n");
		ret = -EINVAL;
	}

	if (ans_msg->complete == PHYTI2S_COMPLETE_ID_NOT_SUPPORTED) {
		dev_err(priv->dev, "cmd not support!\n");
		ret = -EINVAL;
	}

	if (ans_msg->complete == PHYTI2S_COMPLETE_SUBID_NOT_SUPPORTED) {
		dev_err(priv->dev, "cmd subid not support!\n");
		ret = -EINVAL;
	}

	if (ans_msg->complete == PHYTI2S_COMPLETE_INVALID_PARAMETERS) {
		dev_err(priv->dev, "cmd params not support!\n");
		ret = -EINVAL;
	}

	return ret;
}

static int phyt_i2s_enable_gpio(struct phytium_i2s *priv)
{
	struct phyti2s_cmd *msg = priv->sharemem_base + PHYTIUM_GPIO_OFFSET;
	struct gpio_i2s_data *data = &msg->cmd_para.gpio_i2s_data;
	int ret = 0;

	msg->id = PHYTIUM_I2S_LSD_ID;
	msg->cmd_id = PHYTI2S_MSG_CMD_SET;
	msg->cmd_subid = PHYTI2S_MSG_CMD_SET_GPIO;
	msg->complete = 0;
	data->enable = 1;
	ret = phyt_i2s_msg_set_cmd(priv, true);
	if (ret)
		dev_err(priv->dev, "PHYTI2S_MSG_CMD_SET_GPIO enable failed: %d\n", ret);

	return ret;
}

static int phyt_i2s_disable_gpioint(struct phytium_i2s *priv)
{
	struct phyti2s_cmd *msg = priv->sharemem_base + PHYTIUM_GPIO_OFFSET;
	struct gpio_i2s_data *data = &msg->cmd_para.gpio_i2s_data;
	int ret = 0;

	msg->id = PHYTIUM_I2S_LSD_ID;
	msg->cmd_id = PHYTI2S_MSG_CMD_SET;
	msg->cmd_subid = PHYTI2S_MSG_CMD_SET_GPIO;
	msg->complete = 0;
	data->enable = 0;
	ret = phyt_i2s_msg_set_cmd(priv, true);
	if (ret)
		dev_err(priv->dev, "PHYTIUM_MSG_CMD_SET_GPIO disable failed: %d\n", ret);

	return ret;
}

static int phyt_pcm_suspend(struct snd_soc_component *component)
{
	return 0;
}

static int phyt_pcm_resume(struct snd_soc_component *component)
{
	struct phytium_i2s *priv = snd_soc_component_get_drvdata(component);
	struct snd_soc_dai *dai;
	struct phyti2s_cmd *msg = priv->sharemem_base;
	struct set_mode_data *data = &msg->cmd_para.set_mode_data;
	int ret = 0;

	for_each_component_dais(component, dai) {
		if (snd_soc_dai_stream_active(dai, SNDRV_PCM_STREAM_PLAYBACK)) {
			data->direction = DIRECTION_PLAYBACK;
			data->data_width = priv->data_width;
			data->sample_rate = priv->sample_rate;
			data->chan_nr = priv->chan_nr;
			data->clk_base = priv->clk_base;
			data->enable = 1;
			msg->id = PHYTIUM_I2S_LSD_ID;
			msg->cmd_id = PHYTI2S_MSG_CMD_SET;
			msg->cmd_subid = PHYTI2S_MSG_CMD_SET_MODE;
			msg->complete = 0;
			ret = phyt_i2s_msg_set_cmd(priv, false);
			if (ret) {
				dev_err(priv->dev, "phytium-i2s: resume failed: %d\n", ret);
				ret = -EINVAL;
				goto error;
			}
		}
		if (snd_soc_dai_stream_active(dai, SNDRV_PCM_STREAM_CAPTURE)) {
			data->direction = DIRECTION_CAPTURE;
			data->data_width = priv->data_width;
			data->sample_rate = priv->sample_rate;
			data->chan_nr = priv->chan_nr;
			data->clk_base = priv->clk_base;
			data->enable = 1;
			msg->id = PHYTIUM_I2S_LSD_ID;
			msg->cmd_id = PHYTI2S_MSG_CMD_SET;
			msg->cmd_subid = PHYTI2S_MSG_CMD_SET_MODE;
			msg->complete = 0;
			ret = phyt_i2s_msg_set_cmd(priv, false);
			if (ret) {
				dev_err(priv->dev, "phytium-i2s: resume failed: %d\n", ret);
				ret = -EINVAL;
				goto error;
			}
		}
	}
	if (priv->insert >= 0)
		phyt_i2s_enable_gpio(priv);
error:
	return ret;
}

static int phyt_pcm_component_probe(struct snd_soc_component *component)
{
	struct phytium_i2s *priv = snd_soc_component_get_drvdata(component);
	struct snd_soc_card *card = component->card;
	int ret;

	if (priv->insert < 0)
		return 0;

	ret = snd_soc_card_jack_new_pins(card, "Headset Jack", SND_JACK_HEADSET,
				&hs_jack, hs_jack_pins,
				ARRAY_SIZE(hs_jack_pins));
	if (ret < 0) {
		dev_err(component->dev, "Cannot create jack\n");
		return ret;
	}
	ret = phyt_i2s_enable_gpio(priv);
	if (ret < 0) {
		dev_err(component->dev, "failed to enable gpio\n");
		return ret;
	}
	phyt_i2s_gpio_interrupt(priv->gpio_irq, priv);
	return 0;
}

static const struct snd_soc_component_driver phytium_i2s_component = {
	.name = "phytium-i2s",
	.use_dai_pcm_id = true,
	.probe_order = SND_SOC_COMP_ORDER_LATE,
	.pcm_construct = phyt_pcm_new,
	.pcm_destruct = phyt_pcm_free,
	.suspend = phyt_pcm_suspend,
	.resume  = phyt_pcm_resume,
	.probe = phyt_pcm_component_probe,
	.open = phyt_pcm_open,
	.close = phyt_pcm_close,
	.hw_params = phyt_pcm_hw_params,
	.prepare = phyt_pcm_prepare,
	.hw_free = phyt_pcm_hw_free,
	.trigger = phyt_pcm_trigger,
	.pointer = phyt_pcm_pointer,
	.legacy_dai_naming = 1,
};

static int phyt_i2s_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(dai);
	struct phyti2s_cmd *msg = priv->sharemem_base;
	struct set_mode_data *data = &msg->cmd_para.set_mode_data;
	int ret = 0;

	memset(msg, 0, sizeof(struct phyti2s_cmd));

	data->direction = ((substream->stream ==
		SNDRV_PCM_STREAM_PLAYBACK) ? DIRECTION_PLAYBACK:DIRECTION_CAPTURE);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		data->data_width = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		data->data_width = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		data->data_width = 32;
		break;
	default:
		dev_err(priv->dev, "phytium-i2s: unsupported dai fmt");
		ret = -EINVAL;
		goto error;
	}

	data->sample_rate = params_rate(params);
	data->chan_nr = params_channels(params);
	data->enable = 1;
	data->clk_base = priv->clk_base;
	priv->data_width = data->data_width;
	priv->sample_rate = data->sample_rate;
	priv->chan_nr = data->chan_nr;
	msg->id = PHYTIUM_I2S_LSD_ID;
	msg->cmd_id = PHYTI2S_MSG_CMD_SET;
	msg->cmd_subid = PHYTI2S_MSG_CMD_SET_MODE;
	msg->complete = 0;
	ret = phyt_i2s_msg_set_cmd(priv, false);
	if (ret) {
		dev_err(priv->dev, "phytium-i2s: PHYTI2S_MSG_CMD_SET_MODE failed: %d\n", ret);
		ret = -EINVAL;
	}

error:
	return ret;
}

static int phyt_i2s_trigger(struct snd_pcm_substream *substream,
		int cmd, struct snd_soc_dai *dai)
{
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(dai);
	bool start = false;
	int ret = 0, cfg = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		priv->running += 1;
		start = true;
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		priv->running -= 1;
		start = false;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (!start && priv->running)
		goto error;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		cfg = start ? TX_EN : TX_DIS;
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_ITER, cfg);
	} else {
		cfg = start ? RX_EN : RX_DIS;
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_IRER, cfg);
	}

error:
	return ret;
}

static int phyt_i2s_hw_free(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct phytium_i2s *priv = snd_soc_dai_get_drvdata(dai);
	struct phyti2s_cmd *msg = priv->sharemem_base;
	struct set_mode_data *data = &msg->cmd_para.set_mode_data;
	int ret = 0;

	if (priv->running > 0)
		return 0;

	memset(msg, 0, sizeof(struct phyti2s_cmd));

	data->direction = ((substream->stream ==
		SNDRV_PCM_STREAM_PLAYBACK) ? DIRECTION_PLAYBACK:DIRECTION_CAPTURE);
	data->enable = 0;
	msg->id = PHYTIUM_I2S_LSD_ID;
	msg->cmd_id = PHYTI2S_MSG_CMD_SET;
	msg->cmd_subid = PHYTI2S_MSG_CMD_SET_MODE;
	msg->complete = 0;
	ret = phyt_i2s_msg_set_cmd(priv, false);
	if (ret) {
		dev_err(priv->dev, "phytium-i2s: SET_MODE disable failed: %d\n", ret);
		ret = -EINVAL;
	}

	return ret;
}

static const struct snd_soc_dai_ops phytium_i2s_dai_ops = {
	.hw_params	= phyt_i2s_hw_params,
	.trigger	= phyt_i2s_trigger,
	.hw_free	= phyt_i2s_hw_free,
};

static void phyt_i2s_gpio_jack_work(struct work_struct *work)
{
	struct phytium_i2s *priv = container_of(work, struct phytium_i2s,
		phyt_i2s_gpio_work.work);
	struct phyti2s_cmd *msg = priv->sharemem_base + PHYTIUM_GPIO_OFFSET;
	struct gpio_i2s_data *data;
	int ret = 0;
	u32 unplug = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_HPDET);

	if (!hs_jack.card->snd_card)
		return;

	if (unplug & 0x1) {
		if (hs_jack.jack) {
			snd_soc_jack_report(&hs_jack, HEADPHONE_DISABLE, SND_JACK_HEADSET);
			priv->insert = 0;
		}
	} else {
		if (hs_jack.jack) {
			snd_soc_jack_report(&hs_jack, HEADPHONE_ENABLE, SND_JACK_HEADSET);
			priv->insert = 1;
		}
	}

	data = &msg->cmd_para.gpio_i2s_data;

	msg->id = PHYTIUM_I2S_LSD_ID;
	msg->cmd_id = PHYTI2S_MSG_CMD_SET;
	msg->cmd_subid = PHYTI2S_MSG_CMD_SET_GPIO;
	msg->complete = 0;
	data->enable = -1;
	data->insert = priv->insert;
	ret = phyt_i2s_msg_set_cmd(priv, true);
	if (ret)
		dev_err(priv->dev, "PHYTI2S_MSG_CMD_SET_GPIO report jack failed: %d\n", ret);
}

static irqreturn_t phyt_i2s_gpio_interrupt(int irq, void *dev_id)
{
	struct phytium_i2s *priv = dev_id;

	queue_delayed_work(system_power_efficient_wq, &priv->phyt_i2s_gpio_work,
		      msecs_to_jiffies(100));

	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_GPIO_PORTA_EOI, BIT(0));
	return IRQ_HANDLED;
}

static irqreturn_t phyt_i2s_interrupt(int irq, void *dev_id)
{
	struct phytium_i2s *priv = dev_id;
	uint32_t status;
	int ret = IRQ_NONE;

	status = readl(priv->dma_reg_base + PHYTIUM_DMA_STS);

	if (status & DMA_TX_DONE) {
		snd_pcm_period_elapsed(priv->substream_playback);
		writel(DMA_TX_DONE, priv->dma_reg_base + PHYTIUM_DMA_STS);
		ret = IRQ_HANDLED;
	}

	if (status & DMA_RX_DONE) {
		snd_pcm_period_elapsed(priv->substream_capture);
		writel(DMA_RX_DONE, priv->dma_reg_base + PHYTIUM_DMA_STS);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static int phyt_configure_dai_driver(struct phytium_i2s *priv,
					    struct snd_soc_dai_driver *dai_driver)
{
	dai_driver->playback.stream_name = "i2s-Playback";
	dai_driver->playback.channels_min = MIN_CHANNEL_NUM;
	dai_driver->playback.channels_max = 4;
	dai_driver->playback.rates = SNDRV_PCM_RATE_8000_192000;
	dai_driver->playback.formats = SNDRV_PCM_FMTBIT_S8 |
				       SNDRV_PCM_FMTBIT_S16_LE |
				       SNDRV_PCM_FMTBIT_S20_LE |
				       SNDRV_PCM_FMTBIT_S24_LE;

	dai_driver->symmetric_rate = 1;
	dai_driver->ops = &phytium_i2s_dai_ops;
	if (priv->i2s_dp)
		return 0;

	dai_driver->playback.formats |= SNDRV_PCM_FMTBIT_S32_LE;
	dai_driver->capture.stream_name = "i2s-Capture";
	dai_driver->capture.channels_min = 2;
	dai_driver->capture.channels_max = 4;
	dai_driver->capture.rates = SNDRV_PCM_RATE_8000_192000;
	dai_driver->capture.formats = SNDRV_PCM_FMTBIT_S8 |
				      SNDRV_PCM_FMTBIT_S16_LE |
				      SNDRV_PCM_FMTBIT_S20_LE |
				      SNDRV_PCM_FMTBIT_S24_LE |
				      SNDRV_PCM_FMTBIT_S32_LE;

	return 0;
}

static void phyt_i2s_heartbeat(struct phytium_i2s *priv)
{
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG,
		debug_reg | HEARTBEAT);
}

static void phyt_i2s_enable_heartbeat(struct phytium_i2s *priv)
{
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	priv->heart_enable = true;
	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG,
		debug_reg | HEART_ENABLE);
}

static void phyt_i2s_disable_heartbeat(struct phytium_i2s *priv)
{
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	priv->heart_enable = false;
	debug_reg &= ~HEART_ENABLE;
	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG, debug_reg);
}

static void phyt_i2s_enable_debug(struct phytium_i2s *priv)
{
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG,
		debug_reg | DEBUG_ENABLE);
}

static void phyt_i2s_disable_debug(struct phytium_i2s *priv)
{
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	debug_reg &= ~DEBUG_ENABLE;
	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG, debug_reg);
}

static void phyt_i2s_timer_handler(struct timer_list *timer)
{
	struct phytium_i2s *priv = timer_container_of(priv, timer, timer);

	if (priv->heart_enable)
		phyt_i2s_heartbeat(priv);

	mod_timer(&priv->timer, jiffies + msecs_to_jiffies(2000));
}

void phyt_i2s_show_log(struct phytium_i2s *priv)
{
	u32 reg, len;
	u8 *plog;
	int i, cur_len;

	if (!priv->log_addr)
		return;
	reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	plog = priv->log_addr;
	if (reg & LOG_MASK) {
		len = strnlen((char *)priv->log_addr, LOG_SIZE_MAX);
		dev_info(priv->dev, "log len :%d, addr:0x%llx, size:%d\n", len,
				(u64)priv->log_addr, priv->log_size);

		for (i = 0; i < len; i += LOG_LINE_MAX_LEN) {
			cur_len = (((len - i) < LOG_LINE_MAX_LEN) ? (len - i) : LOG_LINE_MAX_LEN);
			dev_info(priv->dev, "(DEV)%.*s\n", cur_len, &plog[i]);
		}

		for (i = 0; i < priv->log_size; i++)
			plog[i] = 0;
	}
	reg &= ~LOG_MASK;
	phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG, reg);
}

int phyt_i2s_init_log(struct phytium_i2s *priv)
{
	u32 reg;
	u64 phy_addr;

	reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);
	reg = reg | DEBUG_ENABLE;
	phy_addr = ((reg & ADDR_MASK) >> ADDR_LOW_SHIFT) << ADDR_SHIFT;

	priv->log_size = ((reg & LOG_SIZE_MASK) >> LOG_SIZE_LOW_SHIFT) * 1024;
	priv->log_addr = devm_ioremap_wc(priv->dev, phy_addr, priv->log_size);
	if (IS_ERR(priv->log_addr)) {
		dev_err(priv->dev, "log_addr alloc failed\n");
		return -ENOMEM;
	}
	return 0;
}

static ssize_t phyt_i2s_debug_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct phytium_i2s *priv = dev_get_drvdata(dev);
	u32 debug_reg = phyt_readl_reg(priv->regfile_base, PHYTIUM_REGFILE_DEBUG);

	dev_info(dev, "echo debug(1)/alive(0) enable(1)/disable(0) > debug\n");

	return sprintf(buf, "%x\n", debug_reg);
}

static ssize_t phyt_i2s_debug_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct phytium_i2s *priv = dev_get_drvdata(dev);
	char *p;
	char *token;
	u8 loc, dis_en;
	int ret;
	long value;

	p = kmalloc(size, GFP_KERNEL);
	if (p == NULL)
		return -EINVAL;
	strscpy(p, buf, sizeof(p));

	token = strsep(&p, " ");
	if (!token) {
		ret = -EINVAL;
		goto error;
	}

	ret = kstrtol(token, 0, &value);
	if (ret)
		goto error;
	loc = (u8)value;

	token = strsep(&p, " ");
	if (!token) {
		ret = -EINVAL;
		goto error;
	}

	ret = kstrtol(token, 0, &value);
	if (ret)
		goto error;
	dis_en = value;

	if (loc == 1) {
		if (dis_en) {
			phyt_i2s_enable_debug(priv);
			phyt_i2s_show_log(priv);
		} else {
			phyt_i2s_disable_debug(priv);
		}
	} else if (loc == 0) {
		if (dis_en)
			phyt_i2s_enable_heartbeat(priv);
		else
			phyt_i2s_disable_heartbeat(priv);
	}

	kfree(p);
	return size;
error:
	kfree(p);
	return ret;
}

static DEVICE_ATTR_RW(phyt_i2s_debug);

static ssize_t phyt_i2s_control_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct phytium_i2s *priv = dev_get_drvdata(dev);
	char *p, *token;
	u8 loc;
	int ret;
	long value;

	p = kmalloc(size, GFP_KERNEL);
	if (p == NULL)
		return -EINVAL;
	strscpy(p, buf, sizeof(p));

	token = strsep(&p, " ");
	if (!token) {
		ret = -EINVAL;
		goto error;
	}

	ret = kstrtol(token, 0, &value);
	if (ret)
		goto error;
	loc = (u8)value;

	if (loc == 1) {
		//Enable I2S and DMA
		phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CTL, 1);
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_ITER, TX_EN);
	} else if (loc == 0) {
		//Disable I2S and DMA
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_ITER, TX_DIS);
		phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_CTL, 0);
	}

	kfree(p);
	return size;
error:
	kfree(p);
	return ret;
}

static DEVICE_ATTR_WO(phyt_i2s_control);

static struct attribute *phyt_i2s_device_attrs[] = {
	&dev_attr_phyt_i2s_debug.attr,
	&dev_attr_phyt_i2s_control.attr,
	NULL,
};

static const struct attribute_group phyt_i2s_device_group = {
	.attrs = phyt_i2s_device_attrs,
};

static int phyt_i2s_probe(struct platform_device *pdev)
{
	struct phytium_i2s *priv;
	struct snd_soc_dai_driver *dai_driver;
	struct resource *res;
	int ret, irq, gpio_irq;
	struct clk *clk;
	struct fwnode_handle *np;
	uint32_t status;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto failed_alloc_phytium_i2s;
	}

	dev_set_drvdata(&pdev->dev, priv);
	priv->dev = &pdev->dev;

	dai_driver = devm_kzalloc(&pdev->dev, sizeof(*dai_driver), GFP_KERNEL);
	if (!dai_driver) {
		ret = -ENOMEM;
		goto failed_alloc_dai_driver;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->regfile_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->regfile_base)) {
		dev_err(&pdev->dev, "failed to ioremap resource0\n");
		ret = PTR_ERR(priv->regfile_base);
		goto failed_ioremap_res0;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	priv->sharemem_base = devm_ioremap_wc(&pdev->dev, res->start, resource_size(res));
	if (IS_ERR(priv->sharemem_base)) {
		dev_err(&pdev->dev, "failed to ioremap resource1\n");
		ret = PTR_ERR(priv->sharemem_base);
		goto failed_ioremap_res1;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	priv->dma_reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->dma_reg_base)) {
		dev_err(&pdev->dev, "failed to ioremap resource2\n");
		ret = PTR_ERR(priv->dma_reg_base);
		goto failed_ioremap_res2;
	}

	ret = phyt_i2s_init_log(priv);
	if (ret != 0) {
		dev_err(&pdev->dev, "failed to init log\n");
		goto failed_init_log;
	}

	status = readl(priv->dma_reg_base + PHYTIUM_DMA_STS);
	if (status & DMA_TX_DONE)
		writel(DMA_TX_DONE, priv->dma_reg_base + PHYTIUM_DMA_STS);
	if (status & DMA_RX_DONE)
		writel(DMA_RX_DONE, priv->dma_reg_base + PHYTIUM_DMA_STS);
	phyt_writel_reg(priv->dma_reg_base, PHYTIUM_DMA_MASK_INT, 0x0);

	irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(&pdev->dev, irq, phyt_i2s_interrupt, IRQF_SHARED,
		pdev->name, priv);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request irq\n");
		goto failed_request_irq;
	}

	gpio_irq = platform_get_irq_optional(pdev, 1);
	priv->insert = -1;
	if (gpio_irq > 0) {
		priv->gpio_irq = gpio_irq;
		INIT_DELAYED_WORK(&priv->phyt_i2s_gpio_work, phyt_i2s_gpio_jack_work);
		phyt_writel_reg(priv->regfile_base, PHYTIUM_REGFILE_GPIO_PORTA_EOI, BIT(0));
		ret = phyt_i2s_disable_gpioint(priv);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to disable gpioint\n");
			goto failed_disable_gpioint;
		}
		priv->insert = 0;
		ret = devm_request_irq(&pdev->dev, gpio_irq, phyt_i2s_gpio_interrupt,
				IRQF_SHARED, pdev->name, priv);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to request gpio irq\n");
			goto failed_request_irq;
		}
	}

	if (pdev->dev.of_node) {
		ret = device_property_read_string(&pdev->dev, "dai-name", &dai_driver->name);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing dai-name property from device tree\n");
			goto failed_get_dai_name;
		}
		clk = devm_clk_get(&pdev->dev, NULL);
		priv->clk_base = clk_get_rate(clk);
	} else if (has_acpi_companion(&pdev->dev)) {
		np = dev_fwnode(&(pdev->dev));
		ret = fwnode_property_read_string(np, "dai-name", &dai_driver->name);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing dai-name property from acpi\n");
			goto failed_get_dai_name;
		}
		ret = fwnode_property_read_u32(np, "i2s_clk", &priv->clk_base);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing i2s_clk property from acpi\n");
			goto failed_get_dai_name;
		}
	}

	if (strstr(dai_driver->name, "dp"))
		priv->i2s_dp = 1;
	else
		priv->i2s_dp = 0;
	phyt_configure_dai_driver(priv, dai_driver);

	ret = devm_snd_soc_register_component(&pdev->dev, &phytium_i2s_component,
					      dai_driver, 1);
	if (ret != 0) {
		dev_err(&pdev->dev, "not able to register dai\n");
		goto failed_register_com;
	}

	if (sysfs_create_group(&priv->dev->kobj, &phyt_i2s_device_group))
		dev_warn(&pdev->dev, "failed create sysfs\n");

	phyt_i2s_disable_heartbeat(priv);
	phyt_i2s_disable_debug(priv);
	priv->timer.expires = jiffies + msecs_to_jiffies(5000);
	timer_setup(&priv->timer, phyt_i2s_timer_handler, 0);
	add_timer(&priv->timer);

	return 0;
failed_register_com:
failed_get_dai_name:
failed_request_irq:
failed_disable_gpioint:
failed_init_log:
failed_ioremap_res2:
failed_ioremap_res1:
failed_ioremap_res0:
failed_alloc_dai_driver:
failed_alloc_phytium_i2s:
	return ret;
}

static void phyt_i2s_remove(struct platform_device *pdev)
{
	struct phytium_i2s *priv = dev_get_drvdata(&pdev->dev);

	sysfs_remove_group(&priv->dev->kobj, &phyt_i2s_device_group);
	timer_delete(&priv->timer);
}

static const struct of_device_id phyt_i2s_of_match[] = {
	{ .compatible = "phytium,i2s-2.0", },
	{ },
};
MODULE_DEVICE_TABLE(of, phyt_i2s_of_match);

static const struct acpi_device_id phyt_i2s_acpi_match[] = {
	{ "PHYT0062", 0},
	{ },
};
MODULE_DEVICE_TABLE(acpi, phyt_i2s_acpi_match);

static struct platform_driver phyt_i2s_driver = {
	.probe	= phyt_i2s_probe,
	.remove	= phyt_i2s_remove,
	.driver	= {
		.name = "phytium-i2s-v2",
		.of_match_table = of_match_ptr(phyt_i2s_of_match),
		.acpi_match_table = phyt_i2s_acpi_match,
	},
};

module_platform_driver(phyt_i2s_driver);

MODULE_DESCRIPTION("Phytium I2S V2 Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang Xun <yangxun@phytium.com.cn>");
MODULE_VERSION(PHYT_I2S_V2_VERSION);
