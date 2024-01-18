/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_I2S_LOCAL_H
#define __PHYTIUM_I2S_LOCAL_H

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/types.h>
#include <sound/pcm.h>

/* I2S clk setting*/
#define CLK_CFG0	0xc00
#define CLK_CFG1	0xc04

/* common register for all channel */
#define I2S_IER		0x000
#define IRER		0x004
#define ITER		0x008
#define CER		0x00C

#define RXFFR		0x014
#define TXFFR		0x018

/* Interrupt status register fields */
#define ISR_TXFO	BIT(5)
#define ISR_TXFE	BIT(4)
#define ISR_RXFO	BIT(1)
#define ISR_RXDA	BIT(0)

/* I2STxRxRegisters for all channels */
#define LRBR_LTHR(x)	(0x40 * x + 0x020)
#define RRBR_RTHR(x)	(0x40 * x + 0x024)
#define RER(x)		(0x40 * x + 0x028)

#define RCR(x)		(0x40 * x + 0x030)

#define ISR(x)		(0x40 * x + 0x038)
#define IMR(x)		(0x40 * x + 0x03C)
#define ROR(x)		(0x40 * x + 0x040)
#define TOR(x)		(0x40 * x + 0x044)
#define RFCR(x)		(0x40 * x + 0x048)
#define TFCR(x)		(0x40 * x + 0x04C)
#define RFF(x)		(0x40 * x + 0x050)
#define TFF(x)		(0x40 * x + 0x054)

/*enable txd and rxd block channel0~3 */
#define TER(x)		(0x40 * x + 0x02C)
#define CCR		0x010
#define TCR(x)		(0x40 * x + 0x034)


/* I2SCOMPRegisters */
#define I2S_COMP_PARAM_2	0x01F0
#define I2S_COMP_PARAM_1	0x01F4
#define I2S_COMP_VERSION	0x01F8
#define I2S_COMP_TYPE		0x01FC

/***I2S AND DMA***/

#define DMA_GCAP		0x0024

#define DMA_CHAL_CONFG1		0x0028

#define DMA_CHAL_CONFG0		0x0004
#define DMA_MASK_INT		0x000c
#define DMA_BDLPU(x)		(0x40 * x + 0x0040)
#define DMA_BDLPL(x)		(0x40 * x + 0x0044)
#define DMA_CHALX_DEV_ADDR(x)	(0x40 * x + 0x0048)
#define DMA_CHALX_CBL(x)	(0x40 * x + 0x0054)
#define DMA_CHALX_LVI(x)	(0x40 * x + 0x004c)

#define DMA_CHALX_DSIZE(x)	(0x40 * x + 0x0064)
#define DMA_CHALX_DLENTH(x)	(0x40 * x + 0x0068)
#define DMA_CHALX_CTL(x)	(0x40 * x + 0x0058)


#define DMA_CTL			0x0000

#define DMA_LPIB(x)		(0x40 * x + 0x0050)

#define DMA_STS			0x0008

/****************/


/* max number of fragments - we may use more if allocating more pages for BDL */
#define BDL_SIZE		4096
#define AZX_MAX_BDL_ENTRIES	(BDL_SIZE / 16)

/*
 * Component parameter register fields - define the I2S block's
 * configuration.
 */
#define	COMP1_TX_WORDSIZE_3(r)	(((r) & GENMASK(27, 25)) >> 25)
#define	COMP1_TX_WORDSIZE_2(r)	(((r) & GENMASK(24, 22)) >> 22)
#define	COMP1_TX_WORDSIZE_1(r)	(((r) & GENMASK(21, 19)) >> 19)
#define	COMP1_TX_WORDSIZE_0(r)	(((r) & GENMASK(18, 16)) >> 16)
#define	COMP1_TX_CHANNELS(r)	(((r) & GENMASK(10, 9)) >> 9)
#define	COMP1_RX_CHANNELS(r)	(((r) & GENMASK(8, 7)) >> 7)
#define	COMP1_RX_ENABLED(r)	(((r) & BIT(6)) >> 6)
#define	COMP1_TX_ENABLED(r)	(((r) & BIT(5)) >> 5)
#define	COMP1_MODE_EN(r)	(((r) & BIT(4)) >> 4)
#define	COMP1_FIFO_DEPTH_GLOBAL(r)	(((r) & GENMASK(3, 2)) >> 2)
#define	COMP1_APB_DATA_WIDTH(r)	(((r) & GENMASK(1, 0)) >> 0)

#define	COMP2_RX_WORDSIZE_3(r)	(((r) & GENMASK(12, 10)) >> 10)
#define	COMP2_RX_WORDSIZE_2(r)	(((r) & GENMASK(9, 7)) >> 7)
#define	COMP2_RX_WORDSIZE_1(r)	(((r) & GENMASK(5, 3)) >> 3)
#define	COMP2_RX_WORDSIZE_0(r)	(((r) & GENMASK(2, 0)) >> 0)

/* Number of entries in WORDSIZE and DATA_WIDTH parameter registers */
#define	COMP_MAX_WORDSIZE	(1 << 3)
#define	COMP_MAX_DATA_WIDTH	(1 << 2)

#define MAX_CHANNEL_NUM		8
#define MIN_CHANNEL_NUM		2

#define azx_bus(chip)	(&(chip)->bus.core)
#define bus_to_azx(_bus)	container_of(_bus, struct azx, bus.core)

#define I2S_UNSOL_QUEUE_SIZE	64
#define I2S_MAX_CODECS		8	/* limit by controller side */


#define azx_stream(dev)		(&(dev)->core)

struct i2s_clk_config_data {
	int chan_nr;
	u32 data_width;
	u32 sample_rate;
};

struct i2sc_bus {
	struct device *dev;
	const struct i2s_bus_ops *ops;
	const struct i2s_io_ops *io_ops;
	const struct i2s_ext_bus_ops *ext_ops;

	/* h/w resources */
	unsigned long addr;
	void __iomem *remap_addr;
	int irq;

	/* codec linked list */
	struct list_head codec_list;
	unsigned int num_codecs;

	unsigned int unsol_rp, unsol_wp;
	struct work_struct unsol_work;

	struct snd_dma_buffer bdl0;
	struct snd_dma_buffer bdl1;

	/* i2s_stream linked list */
	struct list_head stream_list;

	bool reverse_assign;		/* assign devices in reverse order */

	int bdl_pos_adj;		/* BDL position adjustment */

	/* locks */
	spinlock_t reg_lock;
};

struct i2s_bus {
	struct i2sc_bus core;

	struct snd_card *card;

	struct pci_dev *pci;

	struct mutex prepare_mutex;
};


/*
 * i2s stream
 */
struct i2s_stream {
	struct i2sc_bus *bus;
	struct snd_dma_buffer bdl; /* BDL buffer */
	__le32 *posbuf;		/* position buffer pointer */
	int direction;		/* playback / capture (SNDRV_PCM_STREAM_*) */

	unsigned int bufsize;	/* size of the play buffer in bytes */
	unsigned int period_bytes; /* size of the period in bytes */
	unsigned int frags;	/* number for period in the play buffer */
	unsigned int fifo_size;	/* FIFO size */

	void __iomem *sd_addr;	/* stream descriptor pointer */

	u32 sd_int_sta_mask;	/* stream int status mask */

	/* pcm support */
	struct snd_pcm_substream *substream;	/* assigned substream,
						 * set in PCM open
						 */
	unsigned int format_val;	/* format value to be set in the
					 * controller and the codec
					 */
	unsigned char stream_tag;	/* assigned stream */
	unsigned char index;		/* stream index */
	int assigned_key;		/* last device# key assigned to */

	bool opened;
	bool running;
	bool prepared;
	bool no_period_wakeup;

	int delay_negative_threshold;

	struct list_head list;

};


struct azx_dev {
	struct i2s_stream core;
	unsigned int irq_pending:1;
};



/* PCM setup */
static inline struct azx_dev *get_azx_dev(struct snd_pcm_substream *substream)
{
	return substream->runtime->private_data;
}


#define AZX_MAX_CODECS		HDA_MAX_CODECS
#define AZX_DEFAULT_CODECS	4

#define stream_to_azx_dev(s)	container_of(s, struct azx_dev, core)

struct azx;

struct i2s_controller_ops {
	int (*substream_alloc_pages)(struct azx *chip,
				     struct snd_pcm_substream *substream,
				     size_t size);
	int (*substream_free_pages)(struct azx *chip,
				    struct snd_pcm_substream *substream);
	int (*position_check)(struct azx *chip, struct azx_dev *azx_dev);
};

struct i2s_io_ops {
	int (*dma_alloc_pages)(struct i2sc_bus *bus, int type, size_t size,
			       struct snd_dma_buffer *buf);
	void (*dma_free_pages)(struct i2sc_bus *bus,
			       struct snd_dma_buffer *buf);
};

struct azx {
	struct i2s_bus bus;

	struct snd_card *card;
	struct pci_dev *pci;
	int dev_index;

	int playback_streams;
	int playback_index_offset;
	int capture_streams;
	int capture_index_offset;
	int num_streams;

	/* Register interaction. */
	const struct i2s_controller_ops *ops;

	/* locks */
	struct mutex open_mutex; /* Prevents concurrent open/close operations */

	/* PCM */
	struct list_head pcm_list; /* azx_pcm list */

	/* flags */
	int bdl_pos_adj;
	unsigned int running:1;
	unsigned int region_requested:1;
	unsigned int disabled:1;
};
struct i2s_phytium {
	struct azx chip;
	struct snd_pcm_substream *substream;
	struct device *dev;
	struct device *pdev;
	u32 paddr;
	void __iomem *regs;
	void __iomem *regs_db;
	int irq_id;

	/* for pending irqs */
	struct work_struct irq_pending_work;

	/* sync probing */
	struct completion probe_wait;
	struct work_struct probe_work;

	/* extra flags */
	unsigned int pcie:1;
	unsigned int irq_pending_warned:1;
	unsigned int probe_continued:1;
	unsigned int i2s_dp:1;

	unsigned int i2s_reg_comp1;
	unsigned int i2s_reg_comp2;
	struct clk *clk;
	unsigned int capability;
	unsigned int quirks;
	u32 fifo_th;
	int active;
	u32 xfer_resolution;
	u32 ccr;
	u32 clk_base;

	struct i2s_clk_config_data config;

	  /*azx_dev*/
	struct i2s_stream core;
};

#define azx_alloc_stream_pages(chip) \
	snd_i2s_bus_alloc_stream_pages(azx_bus(chip))

#endif
