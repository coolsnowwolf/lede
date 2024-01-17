/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Implementation of primary ALSA driver code base for Phytium HD Audio.
 *
 * Copyright (c) 2018-2023 Phytium Technology Co., Ltd.
 */
#ifndef __SOUND_HDA_PHYTIUM_H__
#define __SOUND_HDA_PHYTIUM_H__

#include "hda_controller.h"

struct hda_ft {
	struct azx chip;
	struct snd_pcm_substream *substream;
	struct device *dev;
	void __iomem *regs;

	/* for pending irqs */
	struct work_struct irq_pending_work;

	/* sync probing */
	struct completion probe_wait;
	struct work_struct probe_work;

	/* card list (for power_save trigger) */
	struct list_head list;

	/* extra flags */
	unsigned int irq_pending_warned:1;
	unsigned int probe_continued:1;

};

#endif
