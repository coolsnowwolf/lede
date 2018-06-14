#ifndef __LEDS_NU801_H__
#define __LEDS_NU801_H__

/*
 * Definitions for LED driver for NU801
 *
 * Kevin Paul Herbert
 * Copyright (c) 2012, Meraki, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/leds.h>

struct led_nu801_template {
	const char *device_name;	/* Name of the platform device */
	const char *name;	/* Name of this LED chain */
	int num_leds;		/* Number of LEDs in the chain */
	unsigned cki;		/* GPIO pin for CKI */
	unsigned sdi;		/* GPIO pin for SDI */
	int lei;		/* GPIO pin for LEI; < 0 if none */
	u32 ndelay;		/* Delay in nanoseconds */
	enum led_brightness init_brightness[3]; /* Default rgb state */
#ifdef CONFIG_LEDS_TRIGGERS
	const char *default_trigger; /* default trigger */
#endif
	const char *led_colors[3];  /* rgb color order */
};

struct led_nu801_platform_data {
	int num_controllers;	/* Numnber of controllers */
	struct led_nu801_template *template; /* Template per controller */
};

#endif /* __LEDS_NU801_H__ */
