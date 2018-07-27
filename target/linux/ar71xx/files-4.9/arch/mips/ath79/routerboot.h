/*
 *  RouterBoot definitions
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_ROUTERBOOT_H_
#define _ATH79_ROUTERBOOT_H_

struct rb_info {
	unsigned int hard_cfg_offs;
	unsigned int hard_cfg_size;
	void *hard_cfg_data;
	unsigned int soft_cfg_offs;

	const char *board_name;
	u32 hw_options;
};

/* Bit definitions for hardware options */
#define RB_HW_OPT_UART_ABSENT		BIT(0)
#define RB_HW_OPT_HAS_VOLTAGE		BIT(1)
#define RB_HW_OPT_HAS_USB		BIT(2)
#define RB_HW_OPT_HAS_ATTINY		BIT(3)
#define RB_HW_OPT_NO_NAND		BIT(14)
#define RB_HW_OPT_HAS_LCD		BIT(15)
#define RB_HW_OPT_HAS_POE_OUT		BIT(16)
#define RB_HW_OPT_HAS_uSD		BIT(17)
#define RB_HW_OPT_HAS_SFP		BIT(20)
#define RB_HW_OPT_HAS_WIFI		BIT(21)
#define RB_HW_OPT_HAS_TS_FOR_ADC	BIT(22)
#define RB_HW_OPT_HAS_PLC		BIT(29)

static inline bool
rb_hw_option_match(const struct rb_info *info, u32 mask, u32 val)
{
	return (info->hw_options & (val | mask)) == val;
}

static inline bool
rb_has_hw_option(const struct rb_info *info, u32 mask)
{
	return rb_hw_option_match(info, mask, mask);
}

#ifdef CONFIG_ATH79_ROUTERBOOT
const struct rb_info *rb_init_info(void *data, unsigned int size);
void *rb_get_wlan_data(void);
void *rb_get_ext_wlan_data(u16 id);

int routerboot_find_tag(u8 *buf, unsigned int buflen, u16 tag_id,
			u8 **tag_data, u16 *tag_len);
int routerboot_find_magic(u8 *buf, unsigned int buflen, u32 *offset, bool hard);
#else
static inline const struct rb_info *
rb_init_info(void *data, unsigned int size)
{
	return NULL;
}

static inline void *rb_get_wlan_data(void)
{
	return NULL;
}

static inline void *rb_get_wlan_data(u16 id)
{
	return NULL;
}

static inline int
routerboot_find_tag(u8 *buf, unsigned int buflen, u16 tag_id,
		    u8 **tag_data, u16 *tag_len)
{
	return -ENOENT;
}

static inline int
routerboot_find_magic(u8 *buf, unsigned int buflen, u32 *offset, bool hard)
{
	return -ENOENT;
}
#endif

#endif /* _ATH79_ROUTERBOOT_H_ */
