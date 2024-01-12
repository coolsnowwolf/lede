/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SPOOL_H
#define __SPOOL_H

struct spool_init_data {
	char	*name;
	u8	dst;
	u8	channel;
	u8	nodev;
	/* Preconfigured channel */
	int	pre_cfg;
	u32	txblocknum;
	u32	txblocksize;
	u32	rxblocknum;
	u32	rxblocksize;
};
#endif
