#include <linux/gpio/driver.h>

#include "rtl837x_common.h"

struct rtl837x_gpio 
{
    struct rtk_gsw *gsw;
    struct gpio_chip gp;
};

static int rtl837x_gpio_request(struct gpio_chip *gc, unsigned int offset)
{
    struct rtl837x_gpio *gpio = gpiochip_get_data(gc);
    uint32_t ret = gpio->gsw->pMapper->gpio_muxSel_set(offset);
    if (ret)
    {
        dev_err(gpio->gsw->dev, "gpio %d: failed to request gpio %x", offset, ret);
        return -EINVAL;
    }
    return 0;
}

static void rtl837x_gpio_set(struct gpio_chip *gc, unsigned offset, int value)
{
    struct rtl837x_gpio *gpio = gpiochip_get_data(gc);
    rtk_gpio_level_t val = value == 1 ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
    uint32_t ret = gpio->gsw->pMapper->gpio_pinVal_write(offset, val);
    if (ret)
    {
        dev_err(gpio->gsw->dev, "gpio %d: failed to write gpio val %x", offset, ret);
    }
}

static int rtl837x_gpio_get(struct gpio_chip *gc, unsigned offset)
{
    struct rtl837x_gpio *gpio = gpiochip_get_data(gc);
    rtk_gpio_level_t pval;
    uint32_t ret = gpio->gsw->pMapper->gpio_pinVal_read(offset, &pval);
    if (ret)
        dev_err(gpio->gsw->dev, "gpio %d: failed to read gpio val %x", offset, ret);

    return pval == GPIO_LEVEL_HIGH ? 1 : 0;
}

static int rtl837x_gpio_direction_input(struct gpio_chip *gc, unsigned offset)
{
    struct rtl837x_gpio *gpio = gpiochip_get_data(gc);
    uint32_t ret = gpio->gsw->pMapper->gpio_pinDir_set(offset, GPIO_DIR_INPUT);
    if (ret)
    {
        dev_err(gpio->gsw->dev, "gpio %d: failed to set gpio direction %x", offset, ret);
        return -EINVAL;
    }
    return 0;
}

static int rtl837x_gpio_direction_output(struct gpio_chip *gc,
					unsigned offset, int value)
{
    struct rtl837x_gpio *gpio = gpiochip_get_data(gc);
    uint32_t ret = gpio->gsw->pMapper->gpio_pinDir_set(offset, GPIO_DIR_OUTPUT);
    if (ret)
    {
        dev_err(gpio->gsw->dev, "gpio %d: failed to set gpio direction %x", offset, ret);
        return -EINVAL;
    }
    rtk_gpio_level_t val = value == 1 ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
    ret = gpio->gsw->pMapper->gpio_pinVal_write(offset, val);
    if (ret)
    {
        dev_err(gpio->gsw->dev, "gpio %d: failed to write gpio val %x", offset, ret);
        return -EIO;
    }
    return 0;
}

static const struct gpio_chip rtl837x_gp = {
	.label = "rtl837x-gpio",
	.owner = THIS_MODULE,
    .request = rtl837x_gpio_request,
	.get = rtl837x_gpio_get,
	.set = rtl837x_gpio_set,
	.direction_input = rtl837x_gpio_direction_input,
	.direction_output = rtl837x_gpio_direction_output,
	.can_sleep = true,
	.ngpio = 63,
};

int rtl837x_gpiochip_init(struct rtk_gsw *gsw)
{
	struct device *dev = gsw->dev;
    struct device_node *np = dev->of_node;

    struct rtl837x_gpio *gpio;

    int base = 0;
    if (of_property_read_u32(np, "base", &base))
        base = -1;

    gpio = devm_kmalloc(dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

    gpio->gsw = gsw;
    gpio->gp = rtl837x_gp;
    gpio->gp.base = base;
    gpio->gp.parent = dev;

	return devm_gpiochip_add_data(dev, &gpio->gp, gpio);
}
