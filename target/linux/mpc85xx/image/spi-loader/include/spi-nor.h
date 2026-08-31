// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#pragma once

#include <types.h>

int spi_nor_read_id(void);
int spi_nor_read_data(void *dest, size_t pos, size_t len);
