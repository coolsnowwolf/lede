// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#include <image.h>
#include <init.h>
#include <spi.h>
#include <spi-nor.h>
#include <stdio.h>
#include <string.h>

static bool check_image_header(const image_header_t *header)
{
	if (header->ih_magic != cpu_to_be32(IH_MAGIC_OKLI)) {
		puts("Invalid image magic\n");
		return false;
	}

	if (header->ih_comp != cpu_to_be32(IH_COMP_NONE)) {
		puts("Unsupported compressed image\n");
		return false;
	}

	return true;
}

static uint32_t do_load(void)
{
	image_header_t header;
	uint32_t ih_size, ih_load, ih_ep;

	if (spi_nor_read_id())
		return UINT32_MAX;

	puts("Reading image header...\n");
	if (spi_nor_read_data(&header, CONFIG_IMAGE_OFFSET, sizeof(header)))
		return UINT32_MAX;

	if (!check_image_header(&header))
		return UINT32_MAX;

	header.ih_name[sizeof(header.ih_name) - 1] = 0;
	ih_size = be32_to_cpu(header.ih_size);
	ih_load = be32_to_cpu(header.ih_load);
	ih_ep = be32_to_cpu(header.ih_ep);

	put_with_label("Image Name:   ", puts, (const char *)header.ih_name);
	put_with_label("Data Size:    ", put_u32, ih_size);
	put_with_label("Load Address: ", put_u32, ih_load);
	put_with_label("Entry Point:  ", put_u32, ih_ep);

	puts("Reading image data...\n");
	void *loadaddr = (void *)ih_load;
	if (spi_nor_read_data(loadaddr, CONFIG_IMAGE_OFFSET + sizeof(header),
			      ih_size))
		return false;

	flush_cache(loadaddr, ih_size);

	return ih_ep;
}

static void enter_image(uint32_t addr)
{
	typedef void (*entry_t)(void);
	entry_t entry = (entry_t)addr;

	puts("Starting image...\n");
	entry();
}

static void load(void)
{
	uint32_t addr;
	int ret;

	ret = spi_init(0, CONFIG_SPI_MAX_HZ, SPI_MODE_0);
	if (ret) {
		puts("Failed to initialize SPI controller\n");
		return;
	}

	ret = spi_claim_bus();
	if (ret) {
		puts("Failed to enable SPI controller\n");
		return;
	}

	addr = do_load();

	spi_release_bus();

	if (addr != UINT32_MAX)
		enter_image(addr);
}

void start(void)
{
	serial_console_init();
	puts("=== " CONFIG_PROGRAM_NAME " ===\n");

	load();

	puts("Halting execution.\n");
	while (true) {}
}
