#ifndef _RB91X_NAND_H_
#define _RB91X_NAND_H_

#define RB91X_NAND_DRIVER_NAME	"rb91x-nand"

struct rb91x_nand_platform_data {
	int gpio_nce;	/* chip enable, active low */
	int gpio_ale;	/* address latch enable */
	int gpio_cle;   /* command latch enable */
	int gpio_rdy;
	int gpio_read;
	int gpio_nrw;	/* read/write enable, active low */
	int gpio_nle;	/* latch enable, active low */
};

#endif /* _RB91X_NAND_H_ */