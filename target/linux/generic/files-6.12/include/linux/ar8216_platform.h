/*
 * AR8216 switch driver platform data
 *
 * Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef AR8216_PLATFORM_H
#define AR8216_PLATFORM_H

enum ar8327_pad_mode {
	AR8327_PAD_NC = 0,
	AR8327_PAD_MAC2MAC_MII,
	AR8327_PAD_MAC2MAC_GMII,
	AR8327_PAD_MAC_SGMII,
	AR8327_PAD_MAC2PHY_MII,
	AR8327_PAD_MAC2PHY_GMII,
	AR8327_PAD_MAC_RGMII,
	AR8327_PAD_PHY_GMII,
	AR8327_PAD_PHY_RGMII,
	AR8327_PAD_PHY_MII,
};

enum ar8327_clk_delay_sel {
	AR8327_CLK_DELAY_SEL0 = 0,
	AR8327_CLK_DELAY_SEL1,
	AR8327_CLK_DELAY_SEL2,
	AR8327_CLK_DELAY_SEL3,
};

struct ar8327_pad_cfg {
	enum ar8327_pad_mode mode;
	bool rxclk_sel;
	bool txclk_sel;
	bool pipe_rxclk_sel;
	bool txclk_delay_en;
	bool rxclk_delay_en;
	bool sgmii_delay_en;
	enum ar8327_clk_delay_sel txclk_delay_sel;
	enum ar8327_clk_delay_sel rxclk_delay_sel;
	bool mac06_exchange_dis;
};

enum ar8327_port_speed {
	AR8327_PORT_SPEED_10 = 0,
	AR8327_PORT_SPEED_100,
	AR8327_PORT_SPEED_1000,
};

struct ar8327_port_cfg {
	int force_link:1;
	enum ar8327_port_speed speed;
	int txpause:1;
	int rxpause:1;
	int duplex:1;
};

struct ar8327_sgmii_cfg {
	u32 sgmii_ctrl;
	bool serdes_aen;
};

struct ar8327_led_cfg {
	u32 led_ctrl0;
	u32 led_ctrl1;
	u32 led_ctrl2;
	u32 led_ctrl3;
	bool open_drain;
};

enum ar8327_led_num {
	AR8327_LED_PHY0_0 = 0,
	AR8327_LED_PHY0_1,
	AR8327_LED_PHY0_2,
	AR8327_LED_PHY1_0,
	AR8327_LED_PHY1_1,
	AR8327_LED_PHY1_2,
	AR8327_LED_PHY2_0,
	AR8327_LED_PHY2_1,
	AR8327_LED_PHY2_2,
	AR8327_LED_PHY3_0,
	AR8327_LED_PHY3_1,
	AR8327_LED_PHY3_2,
	AR8327_LED_PHY4_0,
	AR8327_LED_PHY4_1,
	AR8327_LED_PHY4_2,
};

enum ar8327_led_mode {
	AR8327_LED_MODE_HW = 0,
	AR8327_LED_MODE_SW,
};

struct ar8327_led_info {
	const char *name;
	const char *default_trigger;
	bool active_low;
	enum ar8327_led_num led_num;
	enum ar8327_led_mode mode;
};

#define AR8327_LED_INFO(_led, _mode, _name) {	\
	.name = (_name), 	   		\
	.led_num = AR8327_LED_ ## _led,		\
	.mode = AR8327_LED_MODE_ ## _mode 	\
}

struct ar8327_platform_data {
	struct ar8327_pad_cfg *pad0_cfg;
	struct ar8327_pad_cfg *pad5_cfg;
	struct ar8327_pad_cfg *pad6_cfg;
	struct ar8327_sgmii_cfg *sgmii_cfg;
	struct ar8327_port_cfg port0_cfg;
	struct ar8327_port_cfg port6_cfg;
	struct ar8327_led_cfg *led_cfg;

	int (*get_port_link)(unsigned port);

	unsigned num_leds;
	const struct ar8327_led_info *leds;
};

#endif /* AR8216_PLATFORM_H */

