// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * eSPI controller driver.
 *
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 * Based on U-Boot code:
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 * Author: Mingkai Hu (Mingkai.hu@freescale.com)
 *	   Chuanhua Han (chuanhua.han@nxp.com)
 */

#include <io.h>
#include <stdio.h>
#include <spi.h>

/* eSPI Registers */
typedef struct ccsr_espi {
	uint32_t mode;       /* eSPI mode */
	uint32_t event;      /* eSPI event */
	uint32_t mask;       /* eSPI mask */
	uint32_t com;        /* eSPI command */
	uint32_t tx;         /* eSPI transmit FIFO access */
	uint32_t rx;         /* eSPI receive FIFO access */
	uint8_t  res1[8];    /* reserved */
	uint32_t csmode[4];  /* 0x2c: sSPI CS0/1/2/3 mode */
	uint8_t  res2[4048]; /* fill up to 0x1000 */
} ccsr_espi_t;

struct fsl_spi {
	ccsr_espi_t	*espi;
	uint32_t	cs;
	uint32_t	div16;
	uint32_t	pm;
	uint32_t	mode;
};

#define ESPI_MAX_CS_NUM		4
#define ESPI_FIFO_WIDTH_BIT	32

#define ESPI_EV_RNE		BIT(9)
#define ESPI_EV_TNF		BIT(8)
#define ESPI_EV_DON		BIT(14)
#define ESPI_EV_TXE		BIT(15)
#define ESPI_EV_RFCNT_SHIFT	24
#define ESPI_EV_RFCNT_MASK	(0x3f << ESPI_EV_RFCNT_SHIFT)

#define ESPI_MODE_EN		BIT(31)	/* Enable interface */
#define ESPI_MODE_TXTHR(x)	((x) << 8)	/* Tx FIFO threshold */
#define ESPI_MODE_RXTHR(x)	((x) << 0)	/* Rx FIFO threshold */

#define ESPI_COM_CS(x)		((x) << 30)
#define ESPI_COM_TRANLEN(x)	((x) << 0)

#define ESPI_CSMODE_CI_INACTIVEHIGH	BIT(31)
#define ESPI_CSMODE_CP_BEGIN_EDGCLK	BIT(30)
#define ESPI_CSMODE_REV_MSB_FIRST	BIT(29)
#define ESPI_CSMODE_DIV16		BIT(28)
#define ESPI_CSMODE_PM(x)		((x) << 24)
#define ESPI_CSMODE_POL_ASSERTED_LOW	BIT(20)
#define ESPI_CSMODE_LEN(x)		((x) << 16)
#define ESPI_CSMODE_CSBEF(x)		((x) << 12)
#define ESPI_CSMODE_CSAFT(x)		((x) << 8)
#define ESPI_CSMODE_CSCG(x)		((x) << 3)

#define ESPI_CSMODE_INIT_VAL (ESPI_CSMODE_POL_ASSERTED_LOW | \
		ESPI_CSMODE_CSBEF(0) | ESPI_CSMODE_CSAFT(0) | \
		ESPI_CSMODE_CSCG(1))

#define ESPI_MAX_DATA_TRANSFER_LEN 0x10000

static int espi_xfer(struct fsl_spi *fsl, const struct spi_transfer *msg, int n)
{
	ccsr_espi_t *espi = fsl->espi;
	size_t len = spi_message_len(msg, n);

	if (len > ESPI_MAX_DATA_TRANSFER_LEN)
		return -1;

	/* clear the RXCNT and TXCNT */
	out_be32(&espi->mode, in_be32(&espi->mode) & (~ESPI_MODE_EN));
	out_be32(&espi->mode, in_be32(&espi->mode) | ESPI_MODE_EN);
	out_be32(&espi->com, ESPI_COM_CS(fsl->cs) | ESPI_COM_TRANLEN(len - 1));

	int last_msg = n - 1;
	int tx_msg = -1, rx_msg = -1;
	size_t tx_len = 0, rx_len = 0, tx_pos = 0, rx_pos = 0;

	while (true) {
		if (tx_pos == tx_len && tx_msg < last_msg) {
			tx_msg++;
			tx_pos = 0;
			tx_len = msg[tx_msg].len;
		}
		if (rx_pos == rx_len && rx_msg < last_msg) {
			rx_msg++;
			rx_pos = 0;
			rx_len = msg[rx_msg].len;
		}
		if (rx_pos == rx_len)
			break;

		const uint8_t *tx_buf = msg[tx_msg].tx_buf;
		uint8_t *rx_buf = msg[rx_msg].rx_buf;

		uint32_t event = in_be32(&espi->event);

		/* TX */
		if ((event & ESPI_EV_TNF) && tx_len > 0) {
			uint8_t v = 0;
			if (tx_buf)
				v = tx_buf[tx_pos];
			out_8((uint8_t *)&espi->tx, v);
			tx_pos++;
		}

		/* RX */
		if (event & ESPI_EV_RNE) {
			uint8_t v = in_8((uint8_t *)&espi->rx);
			if (rx_buf)
				rx_buf[rx_pos] = v;
			rx_pos++;
		}
	}

	return 0;
}

static void espi_claim_bus(struct fsl_spi *fsl)
{
	ccsr_espi_t *espi = fsl->espi;
	uint32_t csmode;
	int i;

	/* Enable eSPI interface */
	out_be32(&espi->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);

	out_be32(&espi->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
		out_be32(&espi->csmode[i], ESPI_CSMODE_INIT_VAL);

	csmode = ESPI_CSMODE_INIT_VAL;

	/* Set eSPI BRG clock source */
	csmode |= ESPI_CSMODE_PM(fsl->pm) | fsl->div16;

	/* Set eSPI mode */
	if (fsl->mode & SPI_CPHA)
		csmode |= ESPI_CSMODE_CP_BEGIN_EDGCLK;
	if (fsl->mode & SPI_CPOL)
		csmode |= ESPI_CSMODE_CI_INACTIVEHIGH;

	/* Character bit order: msb first */
	csmode |= ESPI_CSMODE_REV_MSB_FIRST;

	/* Character length in bits, between 0x3~0xf, i.e. 4bits~16bits */
	csmode |= ESPI_CSMODE_LEN(7);

	out_be32(&espi->csmode[fsl->cs], csmode);
}

static void espi_release_bus(struct fsl_spi *fsl)
{
	/* Disable the SPI hardware */
	out_be32(&fsl->espi->mode,
		in_be32(&fsl->espi->mode) & (~ESPI_MODE_EN));
}

static void espi_setup_spi(struct fsl_spi *fsl, unsigned int max_hz)
{
	unsigned long spibrg;
	uint32_t pm;

	spibrg = CONFIG_FREQ_SYSTEMBUS / 2;
	fsl->div16 = 0;
	if ((spibrg / max_hz) > 32) {
		fsl->div16 = ESPI_CSMODE_DIV16;
		pm = spibrg / (max_hz * 16 * 2);
		if (pm > 16) {
			/* max_hz too low */
			pm = 16;
		}
	} else {
		pm = spibrg / (max_hz * 2);
	}
	if (pm)
		pm--;
	fsl->pm = pm;
}

static struct fsl_spi spi;

int spi_init(unsigned int cs, unsigned int max_hz, unsigned int mode)
{
	if (cs >= ESPI_MAX_CS_NUM)
		return -1;

	spi.espi = (ccsr_espi_t *)CONFIG_SPI_FSL_ESPI_REG_BASE;
	spi.cs = cs;
	spi.mode = mode;

	espi_setup_spi(&spi, max_hz);

	return 0;
}

int spi_claim_bus(void)
{
	espi_claim_bus(&spi);

	return 0;
}

void spi_release_bus(void)
{
	espi_release_bus(&spi);
}

int spi_xfer(const struct spi_transfer *msg, int n)
{
	return espi_xfer(&spi, msg, n);
}

size_t spi_max_xfer(void)
{
	return ESPI_MAX_DATA_TRANSFER_LEN;
}
