#ifndef _GPIO_LATCH_H_
#define _GPIO_LATCH_H_

#define GPIO_LATCH_DRIVER_NAME	"gpio-latch"

struct gpio_latch_platform_data {
	int base;
	int num_gpios;
	int *gpios;
	int le_gpio_index;
	bool le_active_low;
};

#endif /* _GPIO_LATCH_H_ */
