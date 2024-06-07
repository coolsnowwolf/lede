/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PHYTIUM_SPI_HEADER_H
#define PHYTIUM_SPI_HEADER_H

#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/gpio.h>

#define CTRLR0			0x00
#define SSIENR			0x08
#define SER				0x10
#define BAUDR			0x14
#define TXFLTR			0x18
#define TXFLR			0x20
#define RXFLR			0x24
#define SR				0x28
#define IMR				0x2c
#define ISR				0x30
#define RISR			0x34
#define ICR				0x48
#define DMACR			0x4C
#define DMATDLR			0x50
#define DMARDLR			0x54
#define DR				0x60
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
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL		(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT		(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)

/* Bit fields in DMACR */
#define SPI_DMA_RDMAE		(1 << 0)
#define SPI_DMA_TDMAE		(1 << 1)

#define SPI_WAIT_RETRIES	5

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
	struct spi_master	*master;
	char			name[16];

	void __iomem		*regs;
	bool			global_cs;
	bool dma_en;
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
	u8				n_bytes;
	int				dma_mapped;
	struct clk		*clk;
	irqreturn_t		(*transfer_handler)(struct phytium_spi *fts);
	u32				current_freq; /* frequency in hz */

	/* DMA info */
	struct dma_chan		*txchan;
	u32			txburst;
	struct dma_chan		*rxchan;
	u32			rxburst;
	u32			dma_sg_burst;
	unsigned long	dma_chan_busy;
	dma_addr_t		dma_addr; /* phy address of the Data register */
	const struct phytium_spi_dma_ops *dma_ops;
	struct completion	dma_completion;
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

static inline void phytium_write_io_reg(struct phytium_spi *fts, u32 offset, u32 val)
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
	global_cs_en = (phytium_readl(fts, GCSR) | mask) & setmask;

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

extern int phytium_spi_add_host(struct device *dev, struct phytium_spi *fts);
extern void phytium_spi_remove_host(struct phytium_spi *fts);
extern int phytium_spi_suspend_host(struct phytium_spi *fts);
extern int phytium_spi_resume_host(struct phytium_spi *fts);
extern void phytium_spi_dmaops_set(struct phytium_spi *fts);
extern int phytium_spi_check_status(struct phytium_spi *fts, bool raw);

#endif /* PHYTIUM_SPI_HEADER_H */
