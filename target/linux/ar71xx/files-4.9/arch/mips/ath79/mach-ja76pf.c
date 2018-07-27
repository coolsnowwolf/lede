/*
 *  jjPlus JA76PF board support
 */

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define JA76PF_KEYS_POLL_INTERVAL	20	/* msecs */
#define JA76PF_KEYS_DEBOUNCE_INTERVAL	(3 * JA76PF_KEYS_POLL_INTERVAL)

#define JA76PF_GPIO_I2C_SCL		0
#define JA76PF_GPIO_I2C_SDA		1
#define JA76PF_GPIO_LED_1		5
#define JA76PF_GPIO_LED_2		4
#define JA76PF_GPIO_LED_3		3
#define JA76PF_GPIO_BTN_RESET		11

static struct gpio_led ja76pf_leds_gpio[] __initdata = {
	{
		.name		= "jjplus:green:led1",
		.gpio		= JA76PF_GPIO_LED_1,
		.active_low	= 1,
	}, {
		.name		= "jjplus:green:led2",
		.gpio		= JA76PF_GPIO_LED_2,
		.active_low	= 1,
	}, {
		.name		= "jjplus:green:led3",
		.gpio		= JA76PF_GPIO_LED_3,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ja76pf_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = JA76PF_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= JA76PF_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct i2c_gpio_platform_data ja76pf_i2c_gpio_data = {
	.sda_pin	= JA76PF_GPIO_I2C_SDA,
	.scl_pin	= JA76PF_GPIO_I2C_SCL,
};

static struct platform_device ja76pf_i2c_gpio_device = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev = {
		.platform_data  = &ja76pf_i2c_gpio_data,
	}
};

static const char *ja76pf_part_probes[] = {
	"RedBoot",
	NULL,
};

static struct flash_platform_data ja76pf_flash_data = {
	.part_probes	= ja76pf_part_probes,
};

#define JA76PF_WAN_PHYMASK	(1 << 4)
#define JA76PF_LAN_PHYMASK	((1 << 0) | (1 << 1) | (1 << 2) | (1 < 3))
#define JA76PF_MDIO_PHYMASK	(JA76PF_LAN_PHYMASK | JA76PF_WAN_PHYMASK)

static void __init ja76pf_init(void)
{
	ath79_register_m25p80(&ja76pf_flash_data);

	ath79_register_mdio(0, ~JA76PF_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = JA76PF_LAN_PHYMASK;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = JA76PF_WAN_PHYMASK;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_eth(1);

	platform_device_register(&ja76pf_i2c_gpio_device);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ja76pf_leds_gpio),
					ja76pf_leds_gpio);

	ath79_register_gpio_keys_polled(-1, JA76PF_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(ja76pf_gpio_keys),
					 ja76pf_gpio_keys);

	ath79_register_usb();
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_JA76PF, "JA76PF", "jjPlus JA76PF", ja76pf_init);

#define JA76PF2_GPIO_LED_D2		5
#define JA76PF2_GPIO_LED_D3		4
#define JA76PF2_GPIO_LED_D4		3
#define JA76PF2_GPIO_BTN_RESET		7
#define JA76PF2_GPIO_BTN_WPS		8

static struct gpio_led ja76pf2_leds_gpio[] __initdata = {
	{
		.name		= "jjplus:green:led1",
		.gpio		= JA76PF2_GPIO_LED_D2,
		.active_low	= 1,
	}, {
		.name		= "jjplus:green:led2",
		.gpio		= JA76PF2_GPIO_LED_D3,
		.active_low	= 0,
	}, {
		.name		= "jjplus:green:led3",
		.gpio		= JA76PF2_GPIO_LED_D4,
		.active_low	= 0,
	}
};

static struct gpio_keys_button ja76pf2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = JA76PF_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= JA76PF2_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = JA76PF_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= JA76PF2_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

#define JA76PF2_LAN_PHYMASK	BIT(0)
#define JA76PF2_WAN_PHYMASK	BIT(4)
#define JA76PF2_MDIO_PHYMASK	(JA76PF2_LAN_PHYMASK | JA76PF2_WAN_PHYMASK)

static void __init ja76pf2_init(void)
{
	ath79_register_m25p80(&ja76pf_flash_data);

	ath79_register_mdio(0, ~JA76PF2_MDIO_PHYMASK);

	/* MAC0 is connected to the CPU port of the AR8316 switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);

	/* MAC1 is connected to the PHY4 of the AR8316 switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = BIT(4);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ja76pf2_leds_gpio),
				 ja76pf2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, JA76PF_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ja76pf2_gpio_keys),
					ja76pf2_gpio_keys);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_JA76PF2, "JA76PF2", "jjPlus JA76PF2", ja76pf2_init);
