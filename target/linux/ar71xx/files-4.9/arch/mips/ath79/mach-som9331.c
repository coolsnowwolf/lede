/*
 *  OpenEmbed SOM9331 board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  5/27/2016 - Modified by Allan Nick Pedrana <nik9993@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define SOM9331_GPIO_LED_WLAN   27
#define SOM9331_GPIO_LED_SYSTEM 0
#define SOM9331_GPIO_LED_2  13
#define SOM9331_GPIO_LED_3  14
#define SOM9331_GPIO_LED_5  16
#define SOM9331_GPIO_LED_WAN    SOM9331_GPIO_LED_2
#define SOM9331_GPIO_LED_LAN1   SOM9331_GPIO_LED_3
#define SOM9331_GPIO_LED_LAN2   SOM9331_GPIO_LED_5
#define SOM9331_GPIO_BTN_RESET  11

#define SOM9331_KEYS_POLL_INTERVAL  20  /* msecs */
#define SOM9331_KEYS_DEBOUNCE_INTERVAL  (3 * SOM9331_KEYS_POLL_INTERVAL)

static const char *som9331_part_probes[] = {
    "tp-link",
    NULL,
};

static struct flash_platform_data som9331_flash_data = {
    .part_probes    = som9331_part_probes,
};

static struct gpio_led som9331_leds_gpio[] __initdata = {
    {
        .name       = "som9331:red:wlan",
        .gpio       = SOM9331_GPIO_LED_WLAN,
        .active_low = 1,
    },
    {
        .name       = "som9331:orange:wan",
        .gpio       = SOM9331_GPIO_LED_WAN,
        .active_low = 0,
    },
    {
        .name       = "som9331:orange:lan1",
        .gpio       = SOM9331_GPIO_LED_LAN1,
        .active_low = 0,
    },
    {
        .name       = "som9331:orange:lan2",
        .gpio       = SOM9331_GPIO_LED_LAN2,
        .active_low = 0,
    },
    {
        .name       = "som9331:blue:system",
        .gpio       = SOM9331_GPIO_LED_SYSTEM,
        .active_low = 0,
    },
};

static struct gpio_keys_button som9331_gpio_keys[] __initdata = {
    {
        .desc       = "reset",
        .type       = EV_KEY,
        .code       = KEY_RESTART,
        .debounce_interval = SOM9331_KEYS_DEBOUNCE_INTERVAL,
        .gpio       = SOM9331_GPIO_BTN_RESET,
        .active_low = 0,
    }
};

static void __init som9331_setup(void)
{
    u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
    u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

    ath79_setup_ar933x_phy4_switch(true, true);

    ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
                    AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
                    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
                    AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
                    AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

    ath79_register_m25p80(&som9331_flash_data);
    ath79_register_leds_gpio(-1, ARRAY_SIZE(som9331_leds_gpio),
                 som9331_leds_gpio);
    ath79_register_gpio_keys_polled(-1, SOM9331_KEYS_POLL_INTERVAL,
                    ARRAY_SIZE(som9331_gpio_keys),
                    som9331_gpio_keys);

    ath79_register_usb();

    ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
    ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

    ath79_register_mdio(0, 0x0);

    /* LAN ports */
    ath79_register_eth(1);

    /* WAN port */
    ath79_register_eth(0);

    ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_SOM9331, "SOM9331", "OpenEmbed SOM9331", som9331_setup);
