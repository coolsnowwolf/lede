// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 FriendlyElec Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#include <common.h>
#include <dm.h>
#include <linux/delay.h>
#include <log.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch-rockchip/gpio.h>

/*
 * ID info:
 *  ID : Volts : ADC value :   Bucket
 *  ==   =====   =========   ===========
 *   0 : 0.102V:        58 :    0 -   81
 *   1 : 0.211V:       120 :   82 -  150
 *   2 : 0.319V:       181 :  151 -  211
 *   3 : 0.427V:       242 :  212 -  274
 *   4 : 0.542V:       307 :  275 -  342
 *   5 : 0.666V:       378 :  343 -  411
 *   6 : 0.781V:       444 :  412 -  477
 *   7 : 0.900V:       511 :  478 -  545
 *   8 : 1.023V:       581 :  546 -  613
 *   9 : 1.137V:       646 :  614 -  675
 *  10 : 1.240V:       704 :  676 -  733
 *  11 : 1.343V:       763 :  734 -  795
 *  12 : 1.457V:       828 :  796 -  861
 *  13 : 1.576V:       895 :  862 -  925
 *  14 : 1.684V:       956 :  926 -  989
 *  15 : 1.800V:      1023 :  990 - 1023
 */
static const int id_readings[] = {
	 81, 150, 211, 274, 342, 411, 477, 545,
	613, 675, 733, 795, 861, 925, 989, 1023
};

static int cached_board_id = -1;

#define SARADC_BASE		0xFF100000
#define SARADC_DATA		(SARADC_BASE + 0)
#define SARADC_CTRL		(SARADC_BASE + 8)

static u32 get_saradc_value(int chn)
{
	int timeout = 0;
	u32 adc_value = 0;

	writel(0, SARADC_CTRL);
	udelay(2);

	writel(0x28 | chn, SARADC_CTRL);
	udelay(50);

	timeout = 0;
	do {
		if (readl(SARADC_CTRL) & 0x40) {
			adc_value = readl(SARADC_DATA) & 0x3FF;
			goto stop_adc;
		}

		udelay(10);
	} while (timeout++ < 100);

stop_adc:
	writel(0, SARADC_CTRL);

	return adc_value;
}

static uint32_t get_adc_index(int chn)
{
	int i;
	int adc_reading;

	if (cached_board_id != -1)
		return cached_board_id;

	adc_reading = get_saradc_value(chn);
	for (i = 0; i < ARRAY_SIZE(id_readings); i++) {
		if (adc_reading <= id_readings[i]) {
			debug("ADC reading %d, ID %d\n", adc_reading, i);
			cached_board_id = i;
			return i;
		}
	}

	/* should die for impossible value */
	return 0;
}

/*
 * Board revision list: <GPIO4_D1 | GPIO4_D0>
 *  0b00 - NanoPC-T4
 *  0b01 - NanoPi M4
 *
 * Extended by ADC_IN4
 * Group A:
 *  0x04 - NanoPi NEO4
 *  0x06 - SOC-RK3399
 *  0x07 - SOC-RK3399 V2
 *  0x09 - NanoPi R4S 1GB
 *  0x0A - NanoPi R4S 4GB
 *
 * Group B:
 *  0x21 - NanoPi M4 Ver2.0
 *  0x22 - NanoPi M4B
 */
static int pcb_rev = -1;

void bd_hwrev_init(void)
{
#define GPIO4_BASE	0xff790000
	struct rockchip_gpio_regs *regs = (void *)GPIO4_BASE;

#ifdef CONFIG_SPL_BUILD
	struct udevice *dev;

	if (uclass_get_device_by_driver(UCLASS_CLK,
				DM_DRIVER_GET(clk_rk3399), &dev))
		return;
#endif

	if (pcb_rev >= 0)
		return;

	/* D1, D0: input mode */
	clrbits_le32(&regs->swport_ddr, (0x3 << 24));
	pcb_rev = (readl(&regs->ext_port) >> 24) & 0x3;

	if (pcb_rev == 0x3) {
		/* Revision group A: 0x04 ~ 0x13 */
		pcb_rev = 0x4 + get_adc_index(4);

	} else if (pcb_rev == 0x1) {
		int idx = get_adc_index(4);

		/* Revision group B: 0x21 ~ 0x2f */
		if (idx > 0) {
			pcb_rev = 0x20 + idx;
		}
	}
}

#ifdef CONFIG_SPL_BUILD
static struct board_ddrtype {
	int rev;
	const char *type;
} ddrtypes[] = {
	{ 0x00, "lpddr3-samsung-4GB-1866" },
	{ 0x01, "lpddr3-samsung-4GB-1866" },
	{ 0x04,   "ddr3-1866" },
	{ 0x06,   "ddr3-1866" },
	{ 0x07, "lpddr4-100"  },
	{ 0x09,   "ddr3-1866" },
	{ 0x0a, "lpddr4-100"  },
	{ 0x21, "lpddr4-100"  },
	{ 0x22,   "ddr3-1866" },
};

const char *rk3399_get_ddrtype(void) {
	int i;

	bd_hwrev_init();
	printf("Board: rev%02x\n", pcb_rev);

	for (i = 0; i < ARRAY_SIZE(ddrtypes); i++) {
		if (ddrtypes[i].rev == pcb_rev)
			return ddrtypes[i].type;
	}

	/* fallback to first subnode (ie, first included dtsi) */
	return NULL;
}
#endif

/* To override __weak symbols */
u32 get_board_rev(void)
{
	return pcb_rev;
}

