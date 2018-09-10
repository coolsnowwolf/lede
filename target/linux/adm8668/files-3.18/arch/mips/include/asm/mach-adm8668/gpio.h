#ifndef __ADM8668_GPIO_H__
#define __ADM8668_GPIO_H__

#define gpio_to_irq(gpio)       -1

#define gpio_get_value __gpio_get_value
#define gpio_set_value __gpio_set_value

#define gpio_cansleep __gpio_cansleep

#include <asm-generic/gpio.h>

#endif
