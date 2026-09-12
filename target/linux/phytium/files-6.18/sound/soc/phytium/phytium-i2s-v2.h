/* SPDX-License-Identifier: GPL-2.0
 *
 * Phytium I2S ASoC driver
 *
 * Copyright (C) 2023-2024, Phytium Technology Co., Ltd.
 *
 */

#ifndef __PHYTIUM_VIRT_I2S_H
#define __PHYTIUM_VIRT_I2S_H

#include <linux/io.h>
#include <sound/memalloc.h>

struct phytium_pcm_config {
	uint32_t buffer_size;
	uint32_t period_bytes;
	uint32_t frags;
	uint32_t format_val;
	unsigned int no_period_wakeup: 1;
	struct snd_dma_buffer bdl_dmab;
};

struct phytium_i2s {
	void __iomem *regfile_base;
	void __iomem *sharemem_base;
	void __iomem *dma_reg_base;
	struct device *dev;
	struct device *parent;

	struct snd_pcm_substream *substream_playback;
	struct snd_pcm_substream *substream_capture;
	u32 clk_base;
	struct phytium_pcm_config pcm_config[2];
	struct delayed_work phyt_i2s_gpio_work;
	int running;
	struct timer_list timer;
	bool heart_enable;
	uint32_t chan_nr;
	uint32_t data_width;
	uint32_t sample_rate;
	int insert;
	int gpio_irq;
	bool i2s_dp;
	void __iomem *log_addr;
	u32 log_size;
};

struct pdata_pd230x_mfd {
	struct device *dev;
	char *name;
	int clk_base;
};

#define MAX_CHANNEL_NUM         8
#define MIN_CHANNEL_NUM         2

#define BDL_SIZE		4096
#define BDL_BUFFER_MAX_SIZE	0x100000

#define DIRECTION_PLAYBACK	0
#define DIRECTION_CAPTURE	1

#define HEADPHONE_DISABLE	0
#define HEADPHONE_ENABLE	1

/* *******************msg*****************************/
#define PHYTIUM_I2S_LSD_ID		0x00

enum phyti2s_msg_cmd_id {
	PHYTI2S_MSG_CMD_DEFAULT = 0,
	PHYTI2S_MSG_CMD_SET,
	PHYTI2S_MSG_CMD_GET,
	PHYTI2S_MSG_CMD_DATA,
	PHYTI2S_MSG_CMD_REPORT,
	PHYTI2S_SYS_PROTOCOL = 0x10,
};

enum phyti2s_set_subid {
	PHYTI2S_MSG_CMD_SET_MODE = 0,
	PHYTI2S_MSG_CMD_SET_TRIGGER,
	PHYTI2S_MSG_CMD_SET_GPIO,
};

enum phyti2s_complete {
	PHYTI2S_COMPLETE_NONE = 0,
	PHYTI2S_COMPLETE_DONE,
	PHYTI2S_COMPLETE_GOING,
	PHYTI2S_COMPLETE_ERROR = 0x10,
	PHYTI2S_COMPLETE_ID_NOT_SUPPORTED,
	PHYTI2S_COMPLETE_SUBID_NOT_SUPPORTED,
	PHYTI2S_COMPLETE_INVALID_PARAMETERS,
};

struct set_mode_data {
	int direction;
	uint32_t chan_nr;
	uint32_t data_width;
	uint32_t sample_rate;
	uint32_t enable;
	uint32_t clk_base;
};

struct trigger_i2s_data {
	int direction;
	uint32_t start;
};

struct gpio_i2s_data {
	int enable;
	int insert;
};

struct phyti2s_cmd {
	uint16_t id;
	uint8_t cmd_id;
	uint8_t cmd_subid;
	uint16_t len;
	uint16_t complete;
	union {
		uint8_t para[56];
		struct set_mode_data set_mode_data;
		struct trigger_i2s_data trigger_i2s_data;
		struct gpio_i2s_data gpio_i2s_data;
	} cmd_para;
};
/* *******************msg end ****************************/


/*********************register ***************************/
/* regfile */
#define PHYTIUM_REGFILE_TX_HEAD	0x00
	#define TX_HEAD_INTR		(1 << 16)
#define PHYTIUM_REGFILE_TX_TAIL	0X04

#define PHYTIUM_REGFILE_AP2RV_INT_MASK	0x20
#define PHYTIUM_REGFILE_AP2RV_INT_STATE	0x24
	#define SEND_INTR		(1 << 4)
	#define SEND_GPIO_INTR		(1 << 8)
#define PHYTIUM_REGFILE_GPIO_PORTA_EOI		0x30
#define PHYTIUM_REGFILE_DEBUG			0x58
	#define DEBUG_ENABLE	(1 << 0)
	#define HEART_ENABLE	(1 << 1)
	#define HEARTBEAT		(1 << 2)
	#define LOG_MASK		(1 << 3)
	#define LOG_SIZE_LOW_SHIFT	4
	#define LOG_SIZE_MASK			GENMASK(7, 4)
	#define LOG_SIZE_MAX		8192
	#define ADDR_LOW_SHIFT		8
	#define ADDR_MASK				GENMASK(27, 8)
	#define ADDR_SHIFT			12
	#define LOG_LINE_MAX_LEN		400
#define PHYTIUM_REGFILE_HPDET	0x34
#define PHYTIUM_REGFILE_IRER	0x38
#define RX_EN 1
#define RX_DIS 0
#define PHYTIUM_REGFILE_ITER	0x3c
#define TX_EN 1
#define TX_DIS 0

/*gpio share memory*/
#define PHYTIUM_GPIO_OFFSET 0x40

/* DMA register */
#define PHYTIUM_DMA_CTL			0x0000
	#define DMA_ENABLE			0x1
#define PHYTIUM_DMA_CHAL_CONFG0		0x0004
	#define CHANNEL_0_1_ENABLE		0x8180
#define PHYTIUM_DMA_STS			0x0008
	#define DMA_TX_DONE			0x1
	#define DMA_RX_DONE			0x100
#define PHYTIUM_DMA_MASK_INT		0x000c
	#define	CHANNEL_0_1_INT_MASK		0x80000003
#define PHYTIUM_DMA_BDLPU(x)		(0x40 * x + 0x0040)
#define PHYTIUM_DMA_BDLPL(x)		(0x40 * x + 0x0044)
#define PHYTIUM_DMA_CHALX_DEV_ADDR(x)	(0x40 * x + 0x0048)
	#define LSD_I2S_BASE		0x18009000
	#define PLAYBACK_ADDRESS_OFFSET		0x1c8
	#define CAPTRUE_ADDRESS_OFFSET		0x1c0
#define PHYTIUM_DMA_CHALX_LVI(x)	(0x40 * x + 0x004c)
#define PHYTIUM_DMA_LPIB(x)		(0x40 * x + 0x0050)
#define PHYTIUM_DMA_CHALX_CBL(x)	(0x40 * x + 0x0054)
#define PHYTIUM_DMA_CHALX_CTL(x)	(0x40 * x + 0x0058)
	#define PLAYBACK_START			0x1
	#define CAPTURE_START			0x5
	#define CTL_STOP			0x0
#define PHYTIUM_DMA_CHALX_DSIZE(x)	(0x40 * x + 0x0064)
	#define BYTE_4				0x0
	#define BYTE_2				0x2
	#define D_SIZE(byte_mode, dir)		(byte_mode << (dir*2))
#define PHYTIUM_DMA_CHALX_DLENTH(x)	(0x40 * x + 0x0068)
	#define D_LENTH				0x0
/*********************register end ***************************/


static inline unsigned int
phyt_readl_reg(void *base, uint32_t offset)
{
	unsigned int data;

	data = readl(base + offset);
	return data;
}

static inline void
phyt_writel_reg(void *base, uint32_t offset, uint32_t data)
{
	writel(data, base + offset);
}
#endif
