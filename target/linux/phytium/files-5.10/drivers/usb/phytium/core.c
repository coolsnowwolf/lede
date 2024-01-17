// SPDX-License-Identifier: GPL-2.0

#include "core.h"

int phytium_core_reset(struct phytium_cusb *config, bool skip_wait)
{
	if (!config)
		return 0;

	spin_lock_init(&config->lock);

	return 0;
}

uint32_t phytium_read32(uint32_t *address)
{
	return readl(address);
}

void phytium_write32(uint32_t *address, uint32_t value)
{
	writel(value, address);
}

uint16_t phytium_read16(uint16_t *address)
{
	return readw(address);
}

void phytium_write16(uint16_t *address, uint16_t value)
{
	writew(value, address);
}

uint8_t phytium_read8(uint8_t *address)
{
	return readb(address);
}

void phytium_write8(uint8_t *address, uint8_t value)
{
	writeb(value, address);
}

