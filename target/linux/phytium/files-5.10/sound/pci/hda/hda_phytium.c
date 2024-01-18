// SPDX-License-Identifier: GPL-2.0
/*
 * Implementation of primary ALSA driver code for Phytium HD Audio.
 *
 * Copyright (c) 2018-2023 Phytium Technology Co., Ltd.
 */

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/clocksource.h>
#include <linux/time.h>
#include <linux/completion.h>
#include <linux/of_device.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/hdaudio.h>
#include <sound/hda_i915.h>
#include <linux/vgaarb.h>
#include <linux/vga_switcheroo.h>
#include <linux/firmware.h>
#include <linux/acpi.h>
#include <linux/dma-map-ops.h>
#include <sound/hda_codec.h>
#include "hda_controller.h"
#include "hda_phytium.h"

#include "hda_intel_trace.h"

/* position fix mode */
enum {
	POS_FIX_AUTO,
	POS_FIX_LPIB,
	POS_FIX_POSBUF,
	POS_FIX_VIACOMBO,
	POS_FIX_COMBO,
};

/* Define IN stream 0 FIFO size offset in VIA controller */
#define VIA_IN_STREAM0_FIFO_SIZE_OFFSET	0x90

/* FT have 4 playback and 4 capture */
#define FT4C_NUM_CAPTURE	4
#define FT4C_NUM_PLAYBACK	4

#define DWORD_BYTE_WIDTH 4
#define BYTE_BIT_WIDTH   8

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static bool enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;
static char *model[SNDRV_CARDS];
static int position_fix[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS-1)] = 1};
static int bdl_pos_adj[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS-1)] = -1};
static int probe_mask[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS-1)] = -1};
static int probe_only[SNDRV_CARDS];
static int jackpoll_ms[SNDRV_CARDS];
static int single_cmd = -1;
static int enable_msi = -1;
#ifdef CONFIG_SND_HDA_INPUT_BEEP
static bool beep_mode[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS-1)] =
					CONFIG_SND_HDA_INPUT_BEEP_MODE};
#endif

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for Intel HD audio interface.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for Intel HD audio interface.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable Intel HD audio interface.");
module_param_array(model, charp, NULL, 0444);
MODULE_PARM_DESC(model, "Use the given board model.");
module_param_array(position_fix, int, NULL, 0444);
MODULE_PARM_DESC(position_fix, "DMA pointer read method. (-1 = system default, 0 = auto, 1 = LPIB, 2 = POSBUF, 3 = VIACOMBO, 4 = COMBO).");
module_param_array(bdl_pos_adj, int, NULL, 0644);
MODULE_PARM_DESC(bdl_pos_adj, "BDL position adjustment offset.");
module_param_array(probe_mask, int, NULL, 0444);
MODULE_PARM_DESC(probe_mask, "Bitmask to probe codecs (default = -1).");
module_param_array(probe_only, int, NULL, 0444);
MODULE_PARM_DESC(probe_only, "Only probing and no codec initialization.");
module_param_array(jackpoll_ms, int, NULL, 0444);
MODULE_PARM_DESC(jackpoll_ms, "Ms between polling for jack events (default = 0, using unsol events only)");
module_param(single_cmd, bint, 0444);
MODULE_PARM_DESC(single_cmd, "Use single command to communicate with codecs (for debugging only).");
module_param(enable_msi, bint, 0444);
MODULE_PARM_DESC(enable_msi, "Enable Message Signaled Interrupt (MSI)");
#ifdef CONFIG_SND_HDA_INPUT_BEEP
module_param_array(beep_mode, bool, NULL, 0444);
MODULE_PARM_DESC(beep_mode, "Select HDA Beep registration mode (0=off, 1=on) (default=1).");
#endif

#define power_save 0

static int align_buffer_size = -1;
module_param(align_buffer_size, bint, 0644);
MODULE_PARM_DESC(align_buffer_size,
		"Force buffer and period sizes to be multiple of 128 bytes.");

/* driver types */
enum {
	AZX_DRIVER_ICH,
	AZX_DRIVER_PCH,
	AZX_DRIVER_SCH,
	AZX_DRIVER_HDMI,
	AZX_DRIVER_ATI,
	AZX_DRIVER_ATIHDMI,
	AZX_DRIVER_ATIHDMI_NS,
	AZX_DRIVER_VIA,
	AZX_DRIVER_SIS,
	AZX_DRIVER_ULI,
	AZX_DRIVER_NVIDIA,
	AZX_DRIVER_TERA,
	AZX_DRIVER_CTX,
	AZX_DRIVER_CTHDA,
	AZX_DRIVER_CMEDIA,
	AZX_DRIVER_GENERIC,
	AZX_DRIVER_FT,
	AZX_NUM_DRIVERS, /* keep this as last entry */
};

/* NOP for other archs */
static inline void mark_pages_wc(struct azx *chip, struct snd_dma_buffer *buf,
				 bool on)
{
}

static inline void mark_runtime_wc(struct azx *chip, struct azx_dev *azx_dev,
				   struct snd_pcm_substream *substream, bool on)
{
}

static int azx_acquire_irq(struct azx *chip, int do_disconnect);

/* calculate runtime delay from LPIB */
static int azx_get_delay_from_lpib(struct azx *chip, struct azx_dev *azx_dev,
				   unsigned int pos)
{
	struct snd_pcm_substream *substream = azx_dev->core.substream;
	int stream = substream->stream;
	unsigned int lpib_pos = azx_get_pos_lpib(chip, azx_dev);
	int delay;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK)
		delay = pos - lpib_pos;
	else
		delay = lpib_pos - pos;
	if (delay < 0) {
		if (delay >= azx_dev->core.delay_negative_threshold)
			delay = 0;
		else
			delay += azx_dev->core.bufsize;
	}

	if (delay >= azx_dev->core.period_bytes) {
		dev_info(chip->card->dev,
			 "Unstable LPIB (%d >= %d); disabling LPIB delay counting\n",
			 delay, azx_dev->core.period_bytes);
		delay = 0;
		chip->driver_caps &= ~AZX_DCAPS_COUNT_LPIB_DELAY;
		chip->get_delay[stream] = NULL;
	}

	return bytes_to_frames(substream->runtime, delay);
}

static int azx_position_ok(struct azx *chip, struct azx_dev *azx_dev);

/* called from IRQ */
static int azx_position_check(struct azx *chip, struct azx_dev *azx_dev)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);
	int ok;

	ok = azx_position_ok(chip, azx_dev);
	if (ok == 1) {
		azx_dev->irq_pending = 0;
		return ok;
	} else if (ok == 0) {
		/* bogus IRQ, process it later */
		azx_dev->irq_pending = 1;
		schedule_work(&hda->irq_pending_work);
	}
	return 0;
}

static int azx_ft_link_power(struct azx *chip, bool enable)
{
	return 0;
}

/*
 * Check whether the current DMA position is acceptable for updating
 * periods.  Returns non-zero if it's OK.
 *
 * Many HD-audio controllers appear pretty inaccurate about
 * the update-IRQ timing.  The IRQ is issued before actually the
 * data is processed.  So, we need to process it afterwords in a
 * workqueue.
 */
static int azx_position_ok(struct azx *chip, struct azx_dev *azx_dev)
{
	struct snd_pcm_substream *substream = azx_dev->core.substream;
	int stream = substream->stream;
	u32 wallclk;
	unsigned int pos;

	wallclk = (azx_readl(chip, WALLCLK) - azx_dev->core.start_wallclk);

	if (wallclk < (azx_dev->core.period_wallclk * 2) / 3)
		return -1;	/* bogus (too early) interrupt */

	if (chip->get_position[stream])
		pos = chip->get_position[stream](chip, azx_dev);
	else { /* use the position buffer as default */
		pos = azx_get_pos_posbuf(chip, azx_dev);
		if (!pos || pos == (u32)-1) {
			dev_info(chip->card->dev,
				 "Invalid position buffer, using LPIB read method instead.\n");
			chip->get_position[stream] = azx_get_pos_lpib;
			if (chip->get_position[0] == azx_get_pos_lpib &&
			    chip->get_position[1] == azx_get_pos_lpib)
				azx_bus(chip)->use_posbuf = false;
			pos = azx_get_pos_lpib(chip, azx_dev);
			chip->get_delay[stream] = NULL;
		} else {
			chip->get_position[stream] = azx_get_pos_posbuf;
			if (chip->driver_caps & AZX_DCAPS_COUNT_LPIB_DELAY)
				chip->get_delay[stream] = azx_get_delay_from_lpib;
		}
	}

	if (pos >= azx_dev->core.bufsize)
		pos = 0;

	if (WARN_ONCE(!azx_dev->core.period_bytes,
		      "hda-ft: zero azx_dev->period_bytes"))
		return -1; /* this shouldn't happen! */
	if (wallclk < (azx_dev->core.period_wallclk * 5) / 4 &&
	    pos % azx_dev->core.period_bytes > azx_dev->core.period_bytes / 2)
		/* NG - it's below the first next period boundary */
		return chip->bdl_pos_adj ? 0 : -1;

	azx_dev->core.start_wallclk += wallclk;

	return 1; /* OK, it's fine */
}

static int hda_ft_dma_configure(struct device *dev)
{
	const struct of_device_id *match_of;
	const struct acpi_device_id *match_acpi;

	if (dev->of_node) {
		match_of = of_match_device(dev->driver->of_match_table, dev);
		if (!match_of) {
			dev_err(dev, "Error DT match data is missing\n");
			return -ENODEV;
		}
		set_dma_ops(dev, NULL);
		/*
		 * Because there is no way to transfer to non-coherent dma in
		 * of_dma_configure if 'dma-coherent' is described in DT,
		 * use acpi_dma_configure to alloc dma_ops correctly.
		 */
		acpi_dma_configure(dev, DEV_DMA_NON_COHERENT);
	} else if (has_acpi_companion(dev)) {
		match_acpi = acpi_match_device(dev->driver->acpi_match_table, dev);
		if (!match_acpi) {
			dev_err(dev, "Error ACPI match data is missing\n");
			return -ENODEV;
		}
		set_dma_ops(dev, NULL);
		acpi_dma_configure(dev, DEV_DMA_NON_COHERENT);
	}

	return 0;
}

/* The work for pending PCM period updates. */
static void azx_irq_pending_work(struct work_struct *work)
{
	struct hda_ft *hda = container_of(work, struct hda_ft, irq_pending_work);
	struct azx *chip = &hda->chip;
	struct hdac_bus *bus = azx_bus(chip);
	struct hdac_stream *s;
	int pending, ok;

	if (!hda->irq_pending_warned) {
		dev_info(chip->card->dev,
			 "IRQ timing workaround is activated for card #%d. Suggest a bigger bdl_pos_adj.\n",
			 chip->card->number);
		hda->irq_pending_warned = 1;
	}

	for (;;) {
		pending = 0;
		spin_lock_irq(&bus->reg_lock);
		list_for_each_entry(s, &bus->stream_list, list) {
			struct azx_dev *azx_dev = stream_to_azx_dev(s);

			if (!azx_dev->irq_pending ||
			    !s->substream || !s->running)
				continue;
			ok = azx_position_ok(chip, azx_dev);
			if (ok > 0) {
				azx_dev->irq_pending = 0;
				spin_unlock(&bus->reg_lock);
				snd_pcm_period_elapsed(s->substream);
				spin_lock(&bus->reg_lock);
			} else if (ok < 0) {
				pending = 0;	/* too early */
			} else {
				pending++;
			}
		}
		spin_unlock_irq(&bus->reg_lock);
		if (!pending)
			return;
		udelay(1000);
	}
}

/* clear irq_pending flags and assure no on-going workq */
static void azx_clear_irq_pending(struct azx *chip)
{
	struct hdac_bus *bus = azx_bus(chip);
	struct hdac_stream *s;

	spin_lock_irq(&bus->reg_lock);
	list_for_each_entry(s, &bus->stream_list, list) {
		struct azx_dev *azx_dev = stream_to_azx_dev(s);

		azx_dev->irq_pending = 0;
	}
	spin_unlock_irq(&bus->reg_lock);
}

static int azx_acquire_irq(struct azx *chip, int do_disconnect)
{
	struct hdac_bus *bus = azx_bus(chip);

	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);
	struct platform_device *pdev = to_platform_device(hda->dev);
	int irq_id = platform_get_irq(pdev, 0);
	int err;

	err = request_irq(irq_id, azx_interrupt,
			     IRQF_SHARED, KBUILD_MODNAME, chip);
	if (err) {
		dev_err(chip->card->dev,
			"unable to request IRQ %d, disabling device\n",
			irq_id);
		if (do_disconnect)
			snd_card_disconnect(chip->card);
		return err;
	}
	bus->irq = irq_id;

	return 0;
}

/* get the current DMA position with correction on VIA chips */
static unsigned int azx_via_get_position(struct azx *chip,
					 struct azx_dev *azx_dev)
{
	unsigned int link_pos, mini_pos, bound_pos;
	unsigned int mod_link_pos, mod_dma_pos, mod_mini_pos;
	unsigned int fifo_size;

	link_pos = snd_hdac_stream_get_pos_lpib(azx_stream(azx_dev));
	if (azx_dev->core.substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* Playback, no problem using link position */
		return link_pos;
	}

	/* Capture */
	/* For new chipset,
	 * use mod to get the DMA position just like old chipset
	 */
	mod_dma_pos = le32_to_cpu(*azx_dev->core.posbuf);
	mod_dma_pos %= azx_dev->core.period_bytes;

	/* azx_dev->fifo_size can't get FIFO size of in stream.
	 * Get from base address + offset.
	 */
	fifo_size = readw(azx_bus(chip)->remap_addr +
			  VIA_IN_STREAM0_FIFO_SIZE_OFFSET);

	if (azx_dev->insufficient) {
		/* Link position never gather than FIFO size */
		if (link_pos <= fifo_size)
			return 0;

		azx_dev->insufficient = 0;
	}

	if (link_pos <= fifo_size)
		mini_pos = azx_dev->core.bufsize + link_pos - fifo_size;
	else
		mini_pos = link_pos - fifo_size;

	/* Find nearest previous boudary */
	mod_mini_pos = mini_pos % azx_dev->core.period_bytes;
	mod_link_pos = link_pos % azx_dev->core.period_bytes;
	if (mod_link_pos >= fifo_size)
		bound_pos = link_pos - mod_link_pos;
	else if (mod_dma_pos >= mod_mini_pos)
		bound_pos = mini_pos - mod_mini_pos;
	else {
		bound_pos = mini_pos - mod_mini_pos + azx_dev->core.period_bytes;
		if (bound_pos >= azx_dev->core.bufsize)
			bound_pos = 0;
	}

	/* Calculate real DMA position we want */
	return bound_pos + mod_dma_pos;
}

#ifdef CONFIG_PM
static DEFINE_MUTEX(card_list_lock);
static LIST_HEAD(card_list);

static void azx_add_card_list(struct azx *chip)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);

	mutex_lock(&card_list_lock);
	list_add(&hda->list, &card_list);
	mutex_unlock(&card_list_lock);
}

static void azx_del_card_list(struct azx *chip)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);

	mutex_lock(&card_list_lock);
	list_del_init(&hda->list);
	mutex_unlock(&card_list_lock);
}

#else
#define azx_add_card_list(chip) /* NOP */
#define azx_del_card_list(chip) /* NOP */
#endif /* CONFIG_PM */

#if defined(CONFIG_PM_SLEEP)
/* power management */
static int azx_suspend(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct azx *chip;
	struct hda_ft *hda;
	struct hdac_bus *bus;

	if (!card)
		return 0;

	chip = card->private_data;
	hda = container_of(chip, struct hda_ft, chip);
	if (chip->disabled || !chip->running)
		return 0;

	bus = azx_bus(chip);
	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	azx_clear_irq_pending(chip);
	azx_stop_chip(chip);
	if (bus->irq >= 0) {
		free_irq(bus->irq, (void *)chip);
		bus->irq = -1;
	}

	return 0;
}

static int azx_resume(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct azx *chip;
	struct hda_ft *hda;
	struct hdac_bus *bus;
	int index;
	struct snd_pcm_substream *substream;
	struct azx_dev *azx_dev;
	int err;

	if (!card)
		return 0;

	chip = card->private_data;
	hda = container_of(chip, struct hda_ft, chip);
	bus = azx_bus(chip);
	if (chip->disabled || !chip->running)
		return 0;

	if (azx_acquire_irq(chip, 1) < 0)
		return -EIO;

	index = chip->dev_index;

	snd_hdac_bus_exit_link_reset(bus);
	usleep_range(1000, 1200);

	azx_init_chip(chip, 0);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);

	if (hda->substream && hda->substream->runtime) {
		substream = hda->substream;

		if (substream->runtime->status->state == SNDRV_PCM_STATE_SUSPENDED) {
			substream->runtime->status->state =
				substream->runtime->status->suspended_state;
			err = substream->ops->prepare(substream);
			if (err < 0)
				return err;
		}

		azx_dev = get_azx_dev(substream);
		hda->substream = NULL;
	}

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

#ifdef CONFIG_PM
static int azx_runtime_suspend(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct azx *chip;
	struct hda_ft *hda;

	if (!card)
		return 0;

	chip = card->private_data;
	hda = container_of(chip, struct hda_ft, chip);
	if (chip->disabled)
		return 0;

	if (!azx_has_pm_runtime(chip))
		return 0;

	azx_stop_chip(chip);
	azx_enter_link_reset(chip);
	azx_clear_irq_pending(chip);

	return 0;
}

static int azx_runtime_resume(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct azx *chip;
	struct hda_ft *hda;
	struct hdac_bus *bus;
	struct hda_codec *codec;
	int status;
	int index;

	if (!card)
		return 0;

	chip = card->private_data;
	hda = container_of(chip, struct hda_ft, chip);
	bus = azx_bus(chip);
	if (chip->disabled)
		return 0;

	if (!azx_has_pm_runtime(chip))
		return 0;

	/* Read STATESTS before controller reset */
	status = azx_readw(chip, STATESTS);

	index = chip->dev_index;

	snd_hdac_bus_exit_link_reset(bus);
	usleep_range(1000, 1200);

	azx_init_chip(chip, 0);

	if (status) {
		list_for_each_codec(codec, &chip->bus)
			if (status & (1 << codec->addr))
				schedule_delayed_work(&codec->jackpoll_work,
						      codec->jackpoll_interval);
	}

	return 0;
}

static int azx_runtime_idle(struct device *dev)
{
	struct snd_card *card = dev_get_drvdata(dev);
	struct azx *chip;
	struct hda_ft *hda;

	if (!card)
		return 0;

	chip = card->private_data;
	hda = container_of(chip, struct hda_ft, chip);
	if (chip->disabled)
		return 0;

	if (!azx_has_pm_runtime(chip) ||
	    azx_bus(chip)->codec_powered || !chip->running)
		return -EBUSY;

	return 0;
}

static const struct dev_pm_ops azx_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(azx_suspend, azx_resume)
	SET_RUNTIME_PM_OPS(azx_runtime_suspend, azx_runtime_resume, azx_runtime_idle)
};

#define hda_ft_pm	(&azx_pm)
#else
#define hda_ft_pm	NULL
#endif /* CONFIG_PM */

static int azx_probe_continue(struct azx *chip);

/*
 * destructor
 */
static int azx_free(struct azx *chip)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);
	struct hdac_bus *bus = azx_bus(chip);
	struct platform_device *pdev = to_platform_device(hda->dev);
	struct device *hddev = hda->dev;
	struct resource *res;
	resource_size_t size;

	if (azx_has_pm_runtime(chip) && chip->running)
		pm_runtime_get_noresume(&pdev->dev);

	azx_del_card_list(chip);

	complete_all(&hda->probe_wait);

	if (bus->chip_init) {
		azx_clear_irq_pending(chip);
		azx_stop_all_streams(chip);
		azx_stop_chip(chip);
	}

	if (bus->irq >= 0) {
		free_irq(bus->irq, (void*)chip);
		bus->irq = -1;
	}

	devm_iounmap(hddev, bus->remap_addr);

	azx_free_stream_pages(chip);
	azx_free_streams(chip);
	snd_hdac_bus_exit(bus);

	if (chip->region_requested){
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		size = resource_size(res);
		devm_release_mem_region(hddev, res->start, size);
	}

	return 0;
}

static int azx_dev_disconnect(struct snd_device *device)
{
	struct azx *chip = device->device_data;

	chip->bus.shutdown = 1;
	return 0;
}

static int azx_dev_free(struct snd_device *device)
{
	return azx_free(device->device_data);
}

static int check_position_fix(struct azx *chip, int fix)
{
	switch (fix) {
	case POS_FIX_AUTO:
	case POS_FIX_LPIB:
	case POS_FIX_POSBUF:
	case POS_FIX_VIACOMBO:
	case POS_FIX_COMBO:
		return fix;
	}

	if (chip->driver_caps & AZX_DCAPS_POSFIX_LPIB) {
		dev_dbg(chip->card->dev, "Using LPIB position fix\n");
		return POS_FIX_LPIB;
	}
	return POS_FIX_AUTO;
}

static void assign_position_fix(struct azx *chip, int fix)
{
	static azx_get_pos_callback_t callbacks[] = {
		[POS_FIX_AUTO] = NULL,
		[POS_FIX_LPIB] = azx_get_pos_lpib,
		[POS_FIX_POSBUF] = azx_get_pos_posbuf,
		[POS_FIX_VIACOMBO] = azx_via_get_position,
		[POS_FIX_COMBO] = azx_get_pos_lpib,
	};

	chip->get_position[0] = chip->get_position[1] = callbacks[fix];

	/* combo mode uses LPIB only for playback */
	if (fix == POS_FIX_COMBO)
		chip->get_position[1] = NULL;

	if (fix == POS_FIX_POSBUF &&
	    (chip->driver_caps & AZX_DCAPS_COUNT_LPIB_DELAY)) {
		chip->get_delay[0] = chip->get_delay[1] =
			azx_get_delay_from_lpib;
	}

}

#define AZX_FORCE_CODEC_MASK	0x100

static void check_probe_mask(struct azx *chip, int dev)
{
	chip->codec_probe_mask = probe_mask[dev];

	/* check forced option */
	if (chip->codec_probe_mask != -1 &&
	    (chip->codec_probe_mask & AZX_FORCE_CODEC_MASK)) {
		azx_bus(chip)->codec_mask = chip->codec_probe_mask & 0xff;
		dev_info(chip->card->dev, "codec_mask forced to 0x%x\n",
			 (int)azx_bus(chip)->codec_mask);
	}
}

static void azx_probe_work(struct work_struct *work)
{
	struct hda_ft *hda = container_of(work, struct hda_ft, probe_work);

	azx_probe_continue(&hda->chip);
}

/*
 * constructor
 */
static const struct hda_controller_ops axi_hda_ops;

static int hda_ft_create(struct snd_card *card, struct platform_device *pdev,
			int dev, unsigned int driver_caps,
			struct azx **rchip)
{
	static struct snd_device_ops ops = {
		.dev_disconnect = azx_dev_disconnect,
		.dev_free = azx_dev_free,
	};
	struct hda_ft *hda;
	struct azx *chip;
	int err;

	*rchip = NULL;

	hda = devm_kzalloc(&pdev->dev, sizeof(*hda), GFP_KERNEL);
	if (!hda)
		return -ENOMEM;
	hda->dev = &pdev->dev;
	chip = &hda->chip;
	mutex_init(&chip->open_mutex);
	chip->card = card;
	chip->ops = &axi_hda_ops;
	chip->driver_caps = driver_caps;
	chip->driver_type = driver_caps & 0xff;
	chip->dev_index = dev;
	if (jackpoll_ms[dev] >= 50 && jackpoll_ms[dev] <= 60000)
		chip->jackpoll_interval = msecs_to_jiffies(jackpoll_ms[dev]);
	INIT_LIST_HEAD(&chip->pcm_list);
	INIT_WORK(&hda->irq_pending_work, azx_irq_pending_work);
	INIT_LIST_HEAD(&hda->list);

	init_completion(&hda->probe_wait);
	assign_position_fix(chip, check_position_fix(chip, position_fix[dev]));
	check_probe_mask(chip, dev);

	if (single_cmd < 0) /* allow fallback to single_cmd at errors */
		chip->fallback_to_single_cmd = 0;
	else /* explicitly set to single_cmd or not */
		chip->single_cmd = single_cmd;

	if (bdl_pos_adj[dev] < 0) {
		switch (chip->driver_type) {
		case AZX_DRIVER_FT:
			bdl_pos_adj[dev] = 32;
			break;
		default:
			bdl_pos_adj[dev] = 32;
			break;
		}
	}
	chip->bdl_pos_adj = bdl_pos_adj[dev];

	err = azx_bus_init(chip, model[dev]);
	if (err < 0) {
		return err;
	}

	chip->bus.core.aligned_mmio = 1;

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (err < 0) {
		dev_err(card->dev, "Error creating device [card]!\n");
		azx_free(chip);
		return err;
	}

	/* continue probing in work context as may trigger request module */
	INIT_WORK(&hda->probe_work, azx_probe_work);

	*rchip = chip;

	return 0;
}

static int azx_first_init(struct azx *chip)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);
	struct platform_device *pdev = to_platform_device(hda->dev);
	struct device *hddev = hda->dev;

	int dev = chip->dev_index;
	bool full_reset;

	struct snd_card *card = chip->card;
	struct hdac_bus *bus = azx_bus(chip);
	int err;
	unsigned short gcap;
	unsigned int dma_bits = 64;

	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	hda->regs = devm_ioremap_resource(hddev, res);
	if (IS_ERR(hda->regs))
		return PTR_ERR(hda->regs);
	chip->region_requested = 1;

	bus->addr = res->start;
	bus->remap_addr = hda->regs;
	if (bus->remap_addr == NULL) {
		dev_err(card->dev, "ioremap error\n");
		return -ENXIO;
	}

	bus->cmd_resend = 1;

	synchronize_irq(bus->irq);

	gcap = azx_readw(chip, GCAP);
	dev_dbg(card->dev, "chipset global capabilities = 0x%x\n", gcap);

	/* disable 64bit DMA address on some devices */
	if (chip->driver_caps & AZX_DCAPS_NO_64BIT) {
		dev_dbg(card->dev, "Disabling 64bit DMA\n");
		gcap &= ~AZX_GCAP_64OK;
	}

	/* disable buffer size rounding to 128-byte multiples if supported */
	if (align_buffer_size >= 0)
		chip->align_buffer_size = !!align_buffer_size;
	else {
		if (chip->driver_caps & AZX_DCAPS_NO_ALIGN_BUFSIZE)
			chip->align_buffer_size = 0;
		else
			chip->align_buffer_size = 1;
	}

	err = hda_ft_dma_configure(hddev);
	if (err < 0)
		return err;

	/* allow 64bit DMA address if supported by H/W */
	if (!(gcap & AZX_GCAP_64OK))
		dma_bits = 32;
	if (!dma_set_mask(hddev, DMA_BIT_MASK(dma_bits))) {
		dma_set_coherent_mask(hddev, DMA_BIT_MASK(dma_bits));
	} else {
		dma_set_mask(hddev, DMA_BIT_MASK(32));
		dma_set_coherent_mask(hddev, DMA_BIT_MASK(32));
	}

	/* read number of streams from GCAP register instead of using
	 * hardcoded value
	 */
	chip->capture_streams = (gcap >> 8) & 0x0f;
	chip->playback_streams = (gcap >> 12) & 0x0f;
	if (!chip->playback_streams && !chip->capture_streams) {
		/* gcap didn't give any info, switching to old method */
		chip->playback_streams = FT4C_NUM_PLAYBACK;
		chip->capture_streams  = FT4C_NUM_CAPTURE;
	}
	chip->capture_index_offset = 0;
	chip->playback_index_offset = chip->capture_streams;
	chip->num_streams = chip->playback_streams + chip->capture_streams;

	/* initialize streams */
	err = azx_init_streams(chip);
	if (err < 0)
		return err;

	err = azx_alloc_stream_pages(chip);
	if (err < 0)
		return err;

	full_reset = (probe_only[dev] & 2) ? false : true;
	azx_init_chip(chip, full_reset);

	/* codec detection */
	if (!azx_bus(chip)->codec_mask) {
		dev_err(card->dev, "no codecs found!\n");
		return -ENODEV;
	}

	if (azx_acquire_irq(chip, 0) < 0)
		return -EBUSY;

	strcpy(card->driver, "ft-hda");
	strcpy(card->shortname, "ft-hda");
	snprintf(card->longname, sizeof(card->longname),
		 "%s at 0x%lx irq %i",
		 card->shortname, bus->addr, bus->irq);

	return 0;
}

static void pcm_mmap_prepare(struct snd_pcm_substream *substream,
			     struct vm_area_struct *area)
{

}

static const struct hda_controller_ops axi_hda_ops = {
	.pcm_mmap_prepare = pcm_mmap_prepare,
	.position_check = azx_position_check,
	.link_power = azx_ft_link_power,
};

static DECLARE_BITMAP(probed_devs, SNDRV_CARDS);

static int hda_ft_probe(struct platform_device *pdev)
{
	const unsigned int driver_flags = AZX_DRIVER_FT;
	struct snd_card *card;
	struct hda_ft *hda;
	struct azx *chip;
	bool schedule_probe;
	int err;
	int dev;

	dev = find_first_zero_bit(probed_devs, SNDRV_CARDS);

	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		set_bit(dev, probed_devs);
		return -ENOENT;
	}

	err = snd_card_new(&pdev->dev, index[dev], id[dev], THIS_MODULE,
			   0, &card);
	if (err < 0) {
		dev_err(&pdev->dev, "Error creating card!\n");
		return err;
	}

	err = hda_ft_create(card, pdev,dev, driver_flags, &chip);
	if (err < 0)
		goto out_free;
	card->private_data = chip;
	hda = container_of(chip, struct hda_ft, chip);

	dev_set_drvdata(&pdev->dev, card);

	schedule_probe = !chip->disabled;

	if (schedule_probe)
		schedule_work(&hda->probe_work);

	set_bit(dev, probed_devs);
	if (chip->disabled)
		complete_all(&hda->probe_wait);
	return 0;

out_free:
	snd_card_free(card);
	return err;
}

/* number of codec slots for each chipset: 0 = default slots (i.e. 4) */
static unsigned int azx_max_codecs[AZX_NUM_DRIVERS] = {
	[AZX_DRIVER_FT] = 4,
};

static int azx_probe_continue(struct azx *chip)
{
	struct hda_ft *hda = container_of(chip, struct hda_ft, chip);
	struct device *hddev = hda->dev;
	int dev = chip->dev_index;
	int err;
	struct hdac_bus *bus = azx_bus(chip);

	hda->probe_continued = 1;

	err = azx_first_init(chip);
	if (err < 0)
		goto out_free;

#ifdef CONFIG_SND_HDA_INPUT_BEEP
	chip->beep_mode = beep_mode[dev];
#endif

	/* create codec instances */
	err = azx_probe_codecs(chip, azx_max_codecs[chip->driver_type]);
	if (err < 0)
		goto out_free;

	if ((probe_only[dev] & 1) == 0) {
		err = azx_codec_configure(chip);
		if (err < 0)
			goto out_free;
	}

	err = snd_card_register(chip->card);
	if (err < 0)
		goto out_free;

	chip->running = 1;
	azx_add_card_list(chip);
	snd_hda_set_power_save(&chip->bus, power_save * 1000);

	if (azx_has_pm_runtime(chip))
			pm_runtime_put_noidle(hddev);
	return err;

out_free:
	if (bus->irq >= 0) {
		free_irq(bus->irq, (void *)chip);
		bus->irq = -1;
	}
	return err;
}

static int hda_ft_remove(struct platform_device *pdev)
{
	struct snd_card *card = dev_get_drvdata(&pdev->dev);
	struct azx *chip;
	struct hda_ft *hda;

	if (card) {
		/* cancel the pending probing work */
		chip = card->private_data;
		hda = container_of(chip, struct hda_ft, chip);
		cancel_work_sync(&hda->probe_work);
		clear_bit(chip->dev_index, probed_devs);

		snd_card_free(card);
		return 0;
	}
	return 0;
}

static void hda_ft_shutdown(struct platform_device *pdev)
{
	struct snd_card *card = dev_get_drvdata(&pdev->dev);
	struct azx *chip;

	if (!card)
		return;
	chip = card->private_data;
	if (chip && chip->running)
		azx_stop_chip(chip);
}

static const struct of_device_id hda_ft_of_match[] = {
	{ .compatible = "phytium,hda" },
	{},
};
MODULE_DEVICE_TABLE(of, hda_ft_of_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id hda_ft_acpi_match[] = {
	{ .id = "PHYT0006" },
	{}
};
MODULE_DEVICE_TABLE(acpi, hda_ft_acpi_match);
#else
#define hda_ft_acpi_match NULL
#endif

static struct platform_driver ft_platform_hda = {
	.driver = {
		.name = "ft-hda",
		.pm = hda_ft_pm,
		.of_match_table = hda_ft_of_match,
		.acpi_match_table = hda_ft_acpi_match,
	},
	.probe = hda_ft_probe,
	.remove = hda_ft_remove,
	.shutdown = hda_ft_shutdown,
};

module_platform_driver(ft_platform_hda);

MODULE_DESCRIPTION("FT HDA bus driver");
MODULE_LICENSE("GPL v2");
