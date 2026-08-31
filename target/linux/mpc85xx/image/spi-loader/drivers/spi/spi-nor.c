// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#include <spi.h>
#include <spi-nor.h>
#include <stdio.h>

#define CMD_READ 0x03
#define CMD_READ_ID 0x9F

int spi_nor_read_id(void)
{
	int ret;

	const uint8_t tx_buf[1] = {CMD_READ_ID};
	uint8_t rx_buf[3] = {};
	struct spi_transfer t[2] = {{
		.tx_buf = tx_buf,
		.rx_buf = NULL,
		.len = sizeof(tx_buf),
	}, {
		.tx_buf = NULL,
		.rx_buf = rx_buf,
		.len = sizeof(rx_buf),
	}};

	ret = spi_xfer(t, ARRAY_SIZE(t));
	if (ret) {
		puts("SPI transfer failed\n");
		return ret;
	}

	puts("Flash JECED ID: ");
	put_array(rx_buf, sizeof(rx_buf));

	return 0;
}

int spi_nor_read_data(void *dest, size_t pos, size_t len)
{
	int ret;

	while (len > 0) {
		uint8_t cmd[4] = {CMD_READ, pos >> 16, pos >> 8, pos};
		size_t block_len = min(len, spi_max_xfer() - sizeof(cmd));

		struct spi_transfer t[2] = {{
			.tx_buf = cmd,
			.rx_buf = NULL,
			.len = sizeof(cmd),
		}, {
			.tx_buf = NULL,
			.rx_buf = dest,
			.len = block_len,
		}};

		ret = spi_xfer(t, ARRAY_SIZE(t));
		if (ret) {
			puts("SPI transfer failed\n");
			return ret;
		}

		pos += block_len;
		dest += block_len;
		len -= block_len;
	}

	return 0;
}

