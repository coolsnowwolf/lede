/*
 *  Buffalo WLAE-AG300N board support
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define WLAEAG300N_MAC_OFFSET		0x20c
#define WLAEAG300N_KEYS_POLL_INTERVAL	20      /* msecs */
#define WLAEAG300N_KEYS_DEBOUNCE_INTERVAL (3 * WLAEAG300N_KEYS_POLL_INTERVAL)


static struct gpio_led wlaeag300n_leds_gpio[] __initdata = {
	/*
	 * Note: Writing 1 into GPIO 13 will power down the device.
	 */
	{
		.name		= "buffalo:green:wireless",
		.gpio		= 14,
		.active_low	= 1,
	}, {
		.name		= "buffalo:red:wireless",
		.gpio		= 15,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:status",
		.gpio		= 16,
		.active_low	= 1,
	}, {
		.name		= "buffalo:red:status",
		.gpio		= 17,
		.active_low	= 1,
	}
};


static struct gpio_keys_button wlaeag300n_gpio_keys[] __initdata = {
	{
		.desc		= "function",
		.type		= EV_KEY,
		.code		= KEY_MODE,
		.debounce_interval = WLAEAG300N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 0,
		.active_low	= 1,
	}, {
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WLAEAG300N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 1,
		.active_low	= 1,
	}, {
		.desc		= "power",
		.type		= EV_KEY,
		.code		= KEY_POWER,
		.debounce_interval = WLAEAG300N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 11,
		.active_low	= 1,
	}, {
		.desc		= "aoss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WLAEAG300N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 12,
		.active_low	= 1,
	}
};

static void __init wlaeag300n_setup(void)
{
	u8 *eeprom1 = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 *mac1 = eeprom1 + WLAEAG300N_MAC_OFFSET;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac1, 1);

	ath79_register_mdio(0, ~(BIT(0) | BIT(4)));

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = BIT(4);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wlaeag300n_leds_gpio),
					wlaeag300n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WLAEAG300N_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(wlaeag300n_gpio_keys),
					 wlaeag300n_gpio_keys);

	ath79_register_m25p80(NULL);

	ap91_pci_init(eeprom1, mac1);
}

MIPS_MACHINE(ATH79_MACH_WLAE_AG300N, "WLAE-AG300N",
	     "Buffalo WLAE-AG300N", wlaeag300n_setup);
