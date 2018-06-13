/*
 * Platform data definition for the Vitesse VSC7385 ethernet switch driver
 *
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

struct vsc7385_platform_data {
	void		(*reset)(void);
	char		*ucode_name;
	struct {
		u32	tx_ipg:5;
		u32	bit2:1;
		u32	clk_sel:3;
	} mac_cfg;
};
