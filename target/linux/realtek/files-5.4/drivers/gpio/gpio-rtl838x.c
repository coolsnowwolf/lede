// SPDX-License-Identifier: GPL-2.0-only

#include <linux/gpio/driver.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/mach-rtl838x/mach-rtl83xx.h>

/* RTL8231 registers for LED control */
#define RTL8231_LED_FUNC0			0x0000
#define RTL8231_GPIO_PIN_SEL(gpio)		((0x0002) + ((gpio) >> 4))
#define RTL8231_GPIO_DIR(gpio)			((0x0005) + ((gpio) >> 4))
#define RTL8231_GPIO_DATA(gpio)			((0x001C) + ((gpio) >> 4))

struct rtl838x_gpios {
	struct gpio_chip gc;
	u32 id;
	struct device *dev;
	int irq;
	int num_leds;
	int min_led;
	int leds_per_port;
	u32 led_mode;
	int led_glb_ctrl;
	int led_sw_ctrl;
	int (*led_sw_p_ctrl)(int port);
	int (*led_sw_p_en_ctrl)(int port);
	int (*ext_gpio_dir)(int i);
	int (*ext_gpio_data)(int i);
};

inline int rtl838x_ext_gpio_dir(int i)
{
	return RTL838X_EXT_GPIO_DIR + ((i >>5) << 2);
}

inline int rtl839x_ext_gpio_dir(int i)
{
	return RTL839X_EXT_GPIO_DIR + ((i >>5) << 2);
}

inline int rtl838x_ext_gpio_data(int i)
{
	return RTL838X_EXT_GPIO_DATA + ((i >>5) << 2);
}

inline int rtl839x_ext_gpio_data(int i)
{
	return RTL839X_EXT_GPIO_DATA + ((i >>5) << 2);
}

inline int rtl838x_led_sw_p_ctrl(int p)
{
	return RTL838X_LED_SW_P_CTRL + (p << 2);
}

inline int rtl839x_led_sw_p_ctrl(int p)
{
	return RTL839X_LED_SW_P_CTRL + (p << 2);
}

inline int rtl838x_led_sw_p_en_ctrl(int p)
{
	return RTL838X_LED_SW_P_EN_CTRL + ((p / 10) << 2);
}

inline int rtl839x_led_sw_p_en_ctrl(int p)
{
	return RTL839X_LED_SW_P_EN_CTRL + ((p / 10) << 2);
}

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;


void rtl838x_gpio_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	int bit;
	struct rtl838x_gpios *gpios = gpiochip_get_data(gc);

	pr_debug("rtl838x_set: %d, value: %d\n", offset, value);
	/* Internal GPIO of the RTL8380 */
	if (offset < 32) {
		if (value)
			rtl83xx_w32_mask(0, BIT(offset), RTL838X_GPIO_PABC_DATA);
		else
			rtl83xx_w32_mask(BIT(offset), 0, RTL838X_GPIO_PABC_DATA);
	}

	/* LED driver for PWR and SYS */
	if (offset >= 32 && offset < 64) {
		bit = offset - 32;
		if (value)
			sw_w32_mask(0, BIT(bit), gpios->led_glb_ctrl);
		else
			sw_w32_mask(BIT(bit), 0, gpios->led_glb_ctrl);
		return;
	}

	bit = (offset - 64) % 32;
	/* First Port-LED */
	if (offset >= 64 && offset < 96
	   && offset >= (64 + gpios->min_led)
	   && offset < (64 + gpios->min_led + gpios->num_leds)) {
		if (value)
			sw_w32_mask(7, 5, gpios->led_sw_p_ctrl(bit));
		else
			sw_w32_mask(7, 0, gpios->led_sw_p_ctrl(bit));
	}
	if (offset >= 96 && offset < 128
	    && offset >= (96 + gpios->min_led)
	    && offset < (96 + gpios->min_led + gpios->num_leds)) {
		if (value)
			sw_w32_mask(7 << 3, 5 << 3, gpios->led_sw_p_ctrl(bit));
		else
			sw_w32_mask(7 << 3, 0, gpios->led_sw_p_ctrl(bit));
	}
	if (offset >= 128 && offset < 160
	    && offset >= (128 + gpios->min_led)
	    && offset < (128 + gpios->min_led + gpios->num_leds)) {
		if (value)
			sw_w32_mask(7 << 6, 5 << 6, gpios->led_sw_p_ctrl(bit));
		else
			sw_w32_mask(7 << 6, 0, gpios->led_sw_p_ctrl(bit));
	}
	__asm__ volatile ("sync");
}

static int rtl838x_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	pr_debug("%s: %d\n", __func__, offset);

	if (offset < 32) {
		rtl83xx_w32_mask(BIT(offset), 0, RTL838X_GPIO_PABC_DIR);
		return 0;
	}

	/* Internal LED driver does not support input */
	return -ENOTSUPP;
}

static int rtl838x_direction_output(struct gpio_chip *gc, unsigned int offset, int value)
{
	pr_debug("%s: %d\n", __func__, offset);
	if (offset < 32)
		rtl83xx_w32_mask(0, BIT(offset), RTL838X_GPIO_PABC_DIR);
	rtl838x_gpio_set(gc, offset, value);

	/* LED for PWR and SYS driver is direction output by default */
	return 0;
}

static int rtl838x_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	u32 v = 0;

	pr_debug("%s: %d\n", __func__, offset);
	if (offset < 32) {
		v = rtl83xx_r32(RTL838X_GPIO_PABC_DIR);
		if (v & BIT(offset))
			return 0;
		return 1;
	}

	/* LED driver for PWR and SYS is direction output by default */
	if (offset >= 32 && offset < 64)
		return 0;

	return 0;
}

static int rtl838x_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	u32 v;
	struct rtl838x_gpios *gpios = gpiochip_get_data(gc);

	pr_debug("%s: %d\n", __func__, offset);

	/* Internal GPIO of the RTL8380 */
	if (offset < 32) {
		v = rtl83xx_r32(RTL838X_GPIO_PABC_DATA);
		if (v & BIT(offset))
			return 1;
		return 0;
	}

	/* LED driver for PWR and SYS */
	if (offset >= 32 && offset < 64) {
		v = sw_r32(gpios->led_glb_ctrl);
		if (v & BIT(offset-32))
			return 1;
		return 0;
	}

/* BUG:
	bit = (offset - 64) % 32;
	if (offset >= 64 && offset < 96) {
		if (sw_r32(RTL838X_LED1_SW_P_EN_CTRL) & BIT(bit))
			return 1;
		return 0;
	}
	if (offset >= 96 && offset < 128) {
		if (sw_r32(RTL838X_LED1_SW_P_EN_CTRL) & BIT(bit))
			return 1;
		return 0;
	}
	if (offset >= 128 && offset < 160) {
		if (sw_r32(RTL838X_LED1_SW_P_EN_CTRL) & BIT(bit))
			return 1;
		return 0;
	}
	*/ 
	return 0;
}

void rtl8380_led_test(struct rtl838x_gpios *gpios, u32 mask)
{
	int i;
	u32 led_gbl = sw_r32(gpios->led_glb_ctrl);
	u32 mode_sel, led_p_en;

	if (soc_info.family == RTL8380_FAMILY_ID) {
		mode_sel = sw_r32(RTL838X_LED_MODE_SEL);
		led_p_en = sw_r32(RTL838X_LED_P_EN_CTRL);
	}

	/* 2 Leds for ports 0-23 and 24-27, 3 would be 0x7 */
	sw_w32_mask(0x3f, 0x3 | (0x3 << 3), gpios->led_glb_ctrl);

	if(soc_info.family == RTL8380_FAMILY_ID) {
		/* Enable all leds */
		sw_w32(0xFFFFFFF, RTL838X_LED_P_EN_CTRL);
	}
	/* Enable software control of all leds */
	sw_w32(0xFFFFFFF, gpios->led_sw_ctrl);
	sw_w32(0xFFFFFFF, gpios->led_sw_p_en_ctrl(0));
	sw_w32(0xFFFFFFF, gpios->led_sw_p_en_ctrl(10));
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(20));

	for (i = 0; i < 28; i++) {
		if (mask & BIT(i))
			sw_w32(5 | (5 << 3) | (5 << 6), gpios->led_sw_p_ctrl(i));
	}
	msleep(3000);

	if (soc_info.family == RTL8380_FAMILY_ID)
		sw_w32(led_p_en, RTL838X_LED_P_EN_CTRL);
	/* Disable software control of all leds */
	sw_w32(0x0000000, gpios->led_sw_ctrl);
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(0));
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(10));
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(20));

	sw_w32(led_gbl, gpios->led_glb_ctrl);
	if (soc_info.family == RTL8380_FAMILY_ID)
		sw_w32(mode_sel, RTL838X_LED_MODE_SEL);
}

void take_port_leds(struct rtl838x_gpios *gpios)
{
	int leds_per_port = gpios->leds_per_port;
	int mode = gpios->led_mode;

	pr_info("%s, %d, %x\n", __func__, leds_per_port, mode);
	pr_debug("Bootloader settings: %x %x %x\n",
		sw_r32(gpios->led_sw_p_en_ctrl(0)),
		sw_r32(gpios->led_sw_p_en_ctrl(10)),
		sw_r32(gpios->led_sw_p_en_ctrl(20))
	);

	if (soc_info.family == RTL8380_FAMILY_ID) {
		pr_debug("led glb: %x, sel %x\n",
			sw_r32(gpios->led_glb_ctrl), sw_r32(RTL838X_LED_MODE_SEL));
		pr_debug("RTL838X_LED_P_EN_CTRL: %x", sw_r32(RTL838X_LED_P_EN_CTRL));
		pr_debug("RTL838X_LED_MODE_CTRL: %x", sw_r32(RTL838X_LED_MODE_CTRL));
		sw_w32_mask(3, 0, RTL838X_LED_MODE_SEL);
		sw_w32(mode, RTL838X_LED_MODE_CTRL);
	}

	/* Enable software control of all leds */
	sw_w32(0xFFFFFFF, gpios->led_sw_ctrl);
	if (soc_info.family == RTL8380_FAMILY_ID)
		sw_w32(0xFFFFFFF, RTL838X_LED_P_EN_CTRL);

	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(0));
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(10));
	sw_w32(0x0000000, gpios->led_sw_p_en_ctrl(20));

	sw_w32_mask(0x3f, 0, gpios->led_glb_ctrl);
	switch (leds_per_port) {
	case 3:
		sw_w32_mask(0, 0x7 | (0x7 << 3), gpios->led_glb_ctrl);
		sw_w32(0xFFFFFFF, gpios->led_sw_p_en_ctrl(20));
		/* FALLTHRU */
	case 2:
		sw_w32_mask(0, 0x3 | (0x3 << 3), gpios->led_glb_ctrl);
		sw_w32(0xFFFFFFF, gpios->led_sw_p_en_ctrl(10));
		/* FALLTHRU */
	case 1:
		sw_w32_mask(0, 0x1 | (0x1 << 3), gpios->led_glb_ctrl);
		sw_w32(0xFFFFFFF, gpios->led_sw_p_en_ctrl(0));
		break;
	default:
		pr_err("No LEDS configured for software control\n");
	}
}

static const struct of_device_id rtl838x_gpio_of_match[] = {
	{ .compatible = "realtek,rtl838x-gpio" },
	{},
};

MODULE_DEVICE_TABLE(of, rtl838x_gpio_of_match);

static int rtl838x_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct rtl838x_gpios *gpios;
	int err;

	pr_info("Probing RTL838X GPIOs\n");

	if (!np) {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	gpios = devm_kzalloc(dev, sizeof(*gpios), GFP_KERNEL);
	if (!gpios)
		return -ENOMEM;

	gpios->id = soc_info.id;

	switch (gpios->id) {
	case 0x8332:
		pr_debug("Found RTL8332M GPIO\n");
		break;
	case 0x8380:
		pr_debug("Found RTL8380M GPIO\n");
		break;
	case 0x8381:
		pr_debug("Found RTL8381M GPIO\n");
		break;
	case 0x8382:
		pr_debug("Found RTL8382M GPIO\n");
		break;
	case 0x8391:
		pr_debug("Found RTL8391 GPIO\n");
		break;
	case 0x8393:
		pr_debug("Found RTL8393 GPIO\n");
		break;
	default:
		pr_err("Unknown GPIO chip id (%04x)\n", gpios->id);
		return -ENODEV;
	}

	if (soc_info.family == RTL8380_FAMILY_ID) {
		gpios->led_glb_ctrl = RTL838X_LED_GLB_CTRL;
		gpios->led_sw_ctrl = RTL838X_LED_SW_CTRL;
		gpios->led_sw_p_ctrl = rtl838x_led_sw_p_ctrl;
		gpios->led_sw_p_en_ctrl = rtl838x_led_sw_p_en_ctrl;
		gpios->ext_gpio_dir = rtl838x_ext_gpio_dir;
		gpios->ext_gpio_data = rtl838x_ext_gpio_data;
	}

	if (soc_info.family == RTL8390_FAMILY_ID) {
		gpios->led_glb_ctrl = RTL839X_LED_GLB_CTRL;
		gpios->led_sw_ctrl = RTL839X_LED_SW_CTRL;
		gpios->led_sw_p_ctrl = rtl839x_led_sw_p_ctrl;
		gpios->led_sw_p_en_ctrl = rtl839x_led_sw_p_en_ctrl;
		gpios->ext_gpio_dir = rtl839x_ext_gpio_dir;
		gpios->ext_gpio_data = rtl839x_ext_gpio_data;
	}

	gpios->dev = dev;
	gpios->gc.base = 0;
	/* 0-31: internal
	 * 32-63, LED control register
	 * 64-95: PORT-LED 0
	 * 96-127: PORT-LED 1
	 * 128-159: PORT-LED 2
	 */
	gpios->gc.ngpio = 160;
	gpios->gc.label = "rtl838x";
	gpios->gc.parent = dev;
	gpios->gc.owner = THIS_MODULE;
	gpios->gc.can_sleep = true;
	gpios->irq = 31;

	gpios->gc.direction_input = rtl838x_direction_input;
	gpios->gc.direction_output = rtl838x_direction_output;
	gpios->gc.set = rtl838x_gpio_set;
	gpios->gc.get = rtl838x_gpio_get;
	gpios->gc.get_direction = rtl838x_get_direction;

	if (of_property_read_bool(np, "take-port-leds")) {
		if (of_property_read_u32(np, "leds-per-port", &gpios->leds_per_port))
			gpios->leds_per_port = 2;
		if (of_property_read_u32(np, "led-mode", &gpios->led_mode))
			gpios->led_mode = (0x1ea << 15) | 0x1ea;
		if (of_property_read_u32(np, "num-leds", &gpios->num_leds))
			gpios->num_leds = 32;
		if (of_property_read_u32(np, "min-led", &gpios->min_led))
			gpios->min_led = 0;
		take_port_leds(gpios);
	}

	err = devm_gpiochip_add_data(dev, &gpios->gc, gpios);
	return err;
}

static struct platform_driver rtl838x_gpio_driver = {
	.driver = {
		.name = "rtl838x-gpio",
		.of_match_table	= rtl838x_gpio_of_match,
	},
	.probe = rtl838x_gpio_probe,
};

module_platform_driver(rtl838x_gpio_driver);

MODULE_DESCRIPTION("Realtek RTL838X GPIO API support");
MODULE_LICENSE("GPL v2");
