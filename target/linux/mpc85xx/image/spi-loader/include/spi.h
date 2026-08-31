// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Common SPI Interface: Controller-specific definitions
 *
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 * Based on U-boot's spi.h:
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

#pragma once

#include <types.h>

/* SPI mode flags */
#define SPI_CPHA	BIT(0)	/* clock phase (1 = SPI_CLOCK_PHASE_SECOND) */
#define SPI_CPOL	BIT(1)	/* clock polarity (1 = SPI_POLARITY_HIGH) */
#define SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define SPI_MODE_1	(0|SPI_CPHA)
#define SPI_MODE_2	(SPI_CPOL|0)
#define SPI_MODE_3	(SPI_CPOL|SPI_CPHA)

struct spi_transfer {
	const void *tx_buf;
	void *rx_buf;
	size_t len;
};

static inline size_t spi_message_len(const struct spi_transfer *msg, int n) {
	size_t total = 0;
	for (int i = 0; i < n; i++) {
		total += msg[i].len;
	}
	return total;
}

int spi_init(unsigned int cs, unsigned int max_hz, unsigned int mode);
int spi_claim_bus(void);
void spi_release_bus(void);
int spi_xfer(const struct spi_transfer *msg, int n);
size_t spi_max_xfer(void);
