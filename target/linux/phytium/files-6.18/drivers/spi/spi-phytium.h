/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium SPI controller driver.
 *
 * Copyright (c) 2019-2024 Phytium Technology Co., Ltd.
 */
#ifndef PHYTIUM_SPI_HEADER_H
#define PHYTIUM_SPI_HEADER_H

#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/gpio.h>

#define CTRLR0			0x00
#define SSIENR			0x08
#define SER			0x10
#define BAUDR			0x14
#define TXFLTR			0x18
#define TXFLR			0x20
#define RXFLR			0x24
#define SR			0x28
#define IMR			0x2c
#define ISR			0x30
#define RISR			0x34
#define ICR			0x48
#define DMACR			0x4C
#define DMATDLR			0x50
#define DMARDLR			0x54
#define DR			0x60
#define GCSR			0x100

#define FRF_OFFSET		4
#define MODE_OFFSET		6
#define TMOD_OFFSET		8

#define TMOD_MASK		(0x3 << TMOD_OFFSET)
#define	TMOD_TR			0x0
#define TMOD_TO			0x1
#define TMOD_RO			0x2

#define INT_TXEI		(1 << 0)
#define INT_TXOI		(1 << 1)
#define INT_RXUI		(1 << 2)
#define INT_RXOI		(1 << 3)
/* Bit fields in SR, 7 bits */
#define SR_MASK			0x7f		/* cover 7 bits */
#define SR_BUSY			(1 << 0)
#define SR_TF_NOT_FULL		(1 << 1)
#define SR_TF_EMPT		(1 << 2)
#define SR_RF_NOT_EMPT		(1 << 3)
#define SR_RF_FULL		(1 << 4)
#define SR_TX_ERR		(1 << 5)
#define SR_DCOL			(1 << 6)
/* Bit fields in DMACR */
#define SPI_DMA_RDMAE		(1 << 0)
#define SPI_DMA_TDMAE		(1 << 1)
#define SPI_WAIT_RETRIES	5

#define SPI_REGFILE_AP2RV_INTR_STATE	(0x24)
#define SPI_REGFILE_RV2AP_INTR_STATE	(0x2c)
#define SPI_REGFILE_SIZE		(0x48)
#define SPI_REGFILE_DDR_HIGH_REG	(0x4c)

#define SPI_REGFILE_SOFTWARE2		(0x54)
#define  SPI_REGFILE_FULL_DUPLEX	BIT(9)

#define SPI_REGFILE_DEBUG		(0x58)
#define  SPI_REGFILE_DEBUG_VAL		BIT(0)
#define  SPI_REGFILE_ALIVE_VAL		BIT(1)
#define  SPI_REGFILE_HEARTBIT_VAL	BIT(2)
#define  SPI_REGFILE_HAVE_LOG		BIT(3)
#define  SPI_REGFILE_SIZE_MASK		GENMASK(7, 4)
#define  SPI_REGFILE_ADDR_MASK		GENMASK(27, 8)

#define SPI_REGFILE_DDR_HIGH_REG	(0x4c)
#define SPI_REGFILE_VERSION_REG		(0x700)
#define SPI_REGFILE_VERSION_SUB		GENMASK(7, 0)
#define SPI_REGFILE_VERSION		GENMASK(15, 8)
#define SPI_REGFILE_VERSION_DMA		BIT(16)
#define SPI_REGFILE_VERSION_DDR		BIT(17)

#define SPI_REGFILE_RV2AP_INT_CLEAN	(0x74)

#define SPI_DDR_ADDR_HIGH		12
#define SPI_DEBUG_LOG_SIZE		4096

#define SPI_LOG_LINE_MAX_LEN		400

#define SPI_MODULE_OPT_CMD		0x20

#define SPI_TRANS_DATA_SIZE		1024
#define FLASH_PAGE_SIZE			256

#define SPI_MSG_COMPLETE_OK		1
#define SPI_MSG_COMPLETE_KO		0

#define SPI_RESULT_IGNORE_LVL		(0)
#define SPI_RESULT_WRITE_ISR_LVL	(1)
#define SPI_RESULT_READ_ISR_LVL		(2)

#define SPI_SHMEM_TX_MSG_MAX_CNT	1

#define SPI_MASTER_TIMEOUT		8

#define SPI_DEFAULT_CLK			50000000

enum phytspi_msg_cmd_id {
	PHYTSPI_MSG_CMD_DEFAULT = 0,
	PHYTSPI_MSG_CMD_SET,
	PHYTSPI_MSG_CMD_GET,
	PHYTSPI_MSG_CMD_DATA,
	PHYTSPI_MSG_CMD_REPORT,
};

enum phytspi_set_subid {
	PHYTSPI_MSG_CMD_SET_MODULE_EN = 0,
	PHYTSPI_MSG_CMD_SET_DATA_WIDTH,
	PHYTSPI_MSG_CMD_SET_MODE,
	PHYTSPI_MSG_CMD_SET_TMOD,
	PHYTSPI_MSG_CMD_SET_BAUDR,
	PHYTSPI_MSG_CMD_SET_INT_TI,
	PHYTSPI_MSG_CMD_SET_NDF,
	PHYTSPI_MSG_CMD_SET_CS,
	PHYTSPI_MSG_CMD_SET_DMA_RESET,
};

enum phytspi_data_subid {
	PHYTSPI_MSG_CMD_DATA_TX = 0,
	PHYTSPI_MSG_CMD_DATA_RX,
	PHYTSPI_MSG_CMD_DATA_FLASH_TX,
	PHYTSPI_MSG_CMD_FLASH_ERASE,
	PHYTSPI_MSG_CMD_DATA_DMA_TX,
	PHYTSPI_MSG_CMD_DATA_DMA_RX,
	PHYTSPI_MSG_CMD_DATA_FLASH_DMA_TX,
	PHYTSPI_MSG_CMD_DATA_XFER,
	PHYTSPI_MSG_CMD_DATA_DMA_XFER,
};

struct msg {
	u8 reserved;
	u8 seq;
	u8 cmd_id;
	u8 cmd_subid;
	u16 len;
	u8 status1;
	u8 status0;
	u8 data[56];
};

struct spi_trans_msg_info {
	u32 msg_total_num;
	u32 shmem_data_addr;
	int result;
};

struct phytium_spi;
struct phytium_spi_dma_ops {
	int (*dma_init)(struct device *dev, struct phytium_spi *fts);
	void (*dma_exit)(struct phytium_spi *fts);
	int (*dma_setup)(struct phytium_spi *fts, struct spi_transfer *xfer);
	bool (*can_dma)(struct spi_controller *master, struct spi_device *spi,
			struct spi_transfer *xfer);
	int (*dma_transfer)(struct phytium_spi *fts, struct spi_transfer *xfer);
	void (*dma_stop)(struct phytium_spi *fts);
};

struct phytium_spi {
	struct spi_controller	*master;
	char			name[16];

	void __iomem		*regs;
	void __iomem		*regfile;
	void __iomem		*tx_shmem_addr;
	void			*rx_shmem_addr;

	struct msg		*msg;
	u32			mem_tx_physic;
	u32			mem_rx_physic;
	u64			mem_tx;
	u64			mem_rx;

	u16			clk_div;
	int			module;

	bool			global_cs;
	bool			dma_en;
	bool			half_duplex;
	unsigned long		paddr;
	int			irq;
	u32			fifo_len;
	u32			max_freq;

	u32			reg_io_width;
	u16			bus_num;
	u16			num_cs;
	int			*cs;

	size_t			len;
	void			*tx;
	void			*tx_end;
	void			*rx;
	void			*rx_end;
	u8			n_bytes;
	int			dma_mapped;
	struct clk		*clk;
	irqreturn_t		(*transfer_handler)(struct phytium_spi *fts);

	int			cmd_err;
	u32			cur_tx_tail;
	struct completion	cmd_completion;

	u8			flags;
	u8			spi_write_flag;
	u8			flash_erase;
	u8			flash_read;
	u8			flash_write;
	u8			read_sr;
	u8			flash_cmd;

	bool			debug_enabled;
	bool			alive_enabled;
	struct timer_list	timer;
	u32			runtimes; // for debug
	u64			ddr_paddr;
	char			*log;
	u32			log_size;
	void			(*watchdog)(struct phytium_spi *fts);
	void			(*handle_debug_err)(struct phytium_spi *fts);
	u32			regfile_version;

	/* DMA info */
	u32			current_freq; /* frequency in hz */
	struct dma_chan		*txchan;
	u32			txburst;
	struct dma_chan		*rxchan;
	u32			rxburst;
	u32			dma_sg_burst;
	unsigned long		dma_chan_busy;
	dma_addr_t		dma_addr; /* phy address of the Data register */
	const struct phytium_spi_dma_ops *dma_ops;
	struct completion	dma_completion;

	bool			dma_get_ddrdata;
};

static inline u32 phytium_readl(struct phytium_spi *fts, u32 offset)
{
	return __raw_readl(fts->regs + offset);
}

static inline u16 phytium_readw(struct phytium_spi *fts, u32 offset)
{
	return __raw_readw(fts->regs + offset);
}
static inline void phytium_writel(struct phytium_spi *fts, u32 offset, u32 val)
{
	__raw_writel(val, fts->regs + offset);
}
static inline void phytium_writew(struct phytium_spi *fts, u32 offset, u16 val)
{
	__raw_writew(val, fts->regs + offset);
}
static inline u32 phytium_read_io_reg(struct phytium_spi *fts, u32 offset)
{
	switch (fts->reg_io_width) {
	case 2:
		return phytium_readw(fts, offset);
	case 4:
	default:
		return phytium_readl(fts, offset);
	}
}
static inline void phytium_write_io_reg(struct phytium_spi *fts,
				u32 offset, u32 val)
{
	switch (fts->reg_io_width) {
	case 2:
		phytium_writew(fts, offset, val);
		break;
	case 4:
	default:
		phytium_writel(fts, offset, val);
		break;
	}
}
static inline void spi_enable_chip(struct phytium_spi *fts, int enable)
{
	phytium_writel(fts, SSIENR, (enable ? 1 : 0));
}
static inline void spi_set_clk(struct phytium_spi *fts, u16 div)
{
	phytium_writel(fts, BAUDR, div);
}
static inline void spi_mask_intr(struct phytium_spi *fts, u32 mask)
{
	u32 new_mask;

	new_mask = phytium_readl(fts, IMR) & ~mask;
	phytium_writel(fts, IMR, new_mask);
}
static inline void spi_umask_intr(struct phytium_spi *fts, u32 mask)
{
	u32 new_mask;

	new_mask = phytium_readl(fts, IMR) | mask;
	phytium_writel(fts, IMR, new_mask);
}
static inline void spi_global_cs(struct phytium_spi *fts)
{
	u32 global_cs_en, mask, setmask;

	mask = GENMASK(fts->num_cs-1, 0) << fts->num_cs;
	setmask = ~GENMASK(fts->num_cs-1, 0);
	global_cs_en =	(phytium_readl(fts, GCSR) | mask) & setmask;
	phytium_writel(fts, GCSR, global_cs_en);
}
static inline void spi_reset_chip(struct phytium_spi *fts)
{
	spi_enable_chip(fts, 0);
	if (fts->global_cs)
		spi_global_cs(fts);
	spi_mask_intr(fts, 0xff);
	spi_enable_chip(fts, 1);
}
static inline void spi_shutdown_chip(struct phytium_spi *fts)
{
	spi_enable_chip(fts, 0);
	spi_set_clk(fts, 0);
	fts->current_freq = 0;
}

static inline u32 phytium_read_regfile(struct phytium_spi *fts, u32 reg_off)
{
	return readl_relaxed(fts->regfile + reg_off);
}

static inline void phytium_write_regfile(struct phytium_spi *fts, u32 reg_off, u32 val)
{
	writel_relaxed(val, fts->regfile + reg_off);
}

extern void spi_phytium_default(struct phytium_spi *fts);
extern void spi_phytium_set_cmd8(struct phytium_spi *fts, u16 sub_cmd, u8 data);
extern void spi_phytium_set_cmd16(struct phytium_spi *fts, u16 sub_cmd, u16 data);
extern void spi_phytium_set_cmd32(struct phytium_spi *fts, u16 sub_cmd, u32 data);
extern void spi_phytium_data_cmd_write(struct phytium_spi *fts, u16 sub_cmd);
extern void spi_phytium_data_cmd_read(struct phytium_spi *fts, u16 sub_cmd);
extern void spi_phytium_write_pre(struct phytium_spi *fts, u8 cs, u8 dfs, u8 mode, u8 tmode,
		u8 flags, u8 spi_write_flag);
extern int spi_phytium_flash_erase(struct phytium_spi *fts, u8 cs, u8 dfs, u8 mode,
		u8 tmode, u8 flags, u8 cmd);
extern int spi_phytium_flash_write(struct phytium_spi *fts, u8 cmd);
extern int spi_phytium_write(struct phytium_spi *fts, u8 cs, u8 dfs, u8 mode,
		u8 tmode, u8 flags, u8 spi_write_flag);
extern int spi_phytium_read(struct phytium_spi *fts, u8 cs, u8 dfs, u8 mode,
		u8 tmode, u8 flags);
extern int spi_phytium_xfer(struct phytium_spi *fts, u8 cs, u8 dfs, u8 mode,
		u8 tmode, u8 flags);
extern int spi_phyt_add_host(struct device *dev, struct phytium_spi *fts);
extern void spi_phyt_remove_host(struct phytium_spi *fts);
extern int spi_phyt_suspend_host(struct phytium_spi *fts);
extern int spi_phyt_resume_host(struct phytium_spi *fts);

extern int phytium_spi_add_host(struct device *dev, struct phytium_spi *fts);
extern void phytium_spi_remove_host(struct phytium_spi *fts);
extern int phytium_spi_suspend_host(struct phytium_spi *fts);
extern int phytium_spi_resume_host(struct phytium_spi *fts);
extern void phytium_spi_dmaops_set(struct phytium_spi *fts);
extern int phytium_spi_check_status(struct phytium_spi *fts, bool raw);

#endif /* PHYTIUM_SPI_HEADER_H */
