/*
 * GPIO driver for NCT5104D
 *
 * Author: Tasanakorn Phaipool <tasanakorn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/version.h>
#include <linux/dmi.h>
#include <linux/string.h>

#define DRVNAME "gpio-nct5104d"

/*
 * Super-I/O registers
 */
#define SIO_LDSEL		0x07	/* Logical device select */
#define SIO_CHIPID		0x20	/* Chaip ID (2 bytes) */
#define SIO_GPIO_ENABLE	0x30	/* GPIO enable */
#define SIO_GPIO1_MODE		0xE0	/* GPIO1 Mode OpenDrain/Push-Pull */
#define SIO_GPIO2_MODE		0xE1	/* GPIO2 Mode OpenDrain/Push-Pull */

#define SIO_LD_GPIO		0x07	/* GPIO logical device */
#define SIO_LD_GPIO_MODE	0x0F	/* GPIO mode control device */
#define SIO_UNLOCK_KEY		0x87	/* Key to enable Super-I/O */
#define SIO_LOCK_KEY		0xAA	/* Key to disable Super-I/O */

#define SIO_NCT5104D_ID					0x1061	/* Chip ID */
#define SIO_PCENGINES_APU_NCT5104D_ID1	0xc452	/* Chip ID */
#define SIO_PCENGINES_APU_NCT5104D_ID2	0xc453	/* Chip ID */

enum chips { nct5104d };

static const char * const nct5104d_names[] = {
	"nct5104d"
};

struct nct5104d_sio {
	int addr;
	enum chips type;
};

struct nct5104d_gpio_bank {
	struct gpio_chip chip;
	unsigned int regbase;
	struct nct5104d_gpio_data *data;
};

struct nct5104d_gpio_data {
	struct nct5104d_sio *sio;
	int nr_bank;
	struct nct5104d_gpio_bank *bank;
};

/*
 * Super-I/O functions.
 */

static inline int superio_inb(int base, int reg)
{
	outb(reg, base);
	return inb(base + 1);
}

static int superio_inw(int base, int reg)
{
	int val;

	outb(reg++, base);
	val = inb(base + 1) << 8;
	outb(reg, base);
	val |= inb(base + 1);

	return val;
}

static inline void superio_outb(int base, int reg, int val)
{
	outb(reg, base);
	outb(val, base + 1);
}

static inline int superio_enter(int base)
{
	/* Don't step on other drivers' I/O space by accident. */
	if (!request_muxed_region(base, 2, DRVNAME)) {
		pr_err(DRVNAME "I/O address 0x%04x already in use\n", base);
		return -EBUSY;
	}

	/* According to the datasheet the key must be send twice. */
	outb(SIO_UNLOCK_KEY, base);
	outb(SIO_UNLOCK_KEY, base);

	return 0;
}

static inline void superio_select(int base, int ld)
{
	outb(SIO_LDSEL, base);
	outb(ld, base + 1);
}

static inline void superio_exit(int base)
{
	outb(SIO_LOCK_KEY, base);
	release_region(base, 2);
}

/*
 * GPIO chip.
 */

static int nct5104d_gpio_direction_in(struct gpio_chip *chip, unsigned offset);
static int nct5104d_gpio_get(struct gpio_chip *chip, unsigned offset);
static int nct5104d_gpio_direction_out(struct gpio_chip *chip,
				     unsigned offset, int value);
static void nct5104d_gpio_set(struct gpio_chip *chip, unsigned offset, int value);

#define NCT5104D_GPIO_BANK(_base, _ngpio, _regbase)			\
	{								\
		.chip = {						\
			.label            = DRVNAME,			\
			.owner            = THIS_MODULE,		\
			.direction_input  = nct5104d_gpio_direction_in,	\
			.get              = nct5104d_gpio_get,		\
			.direction_output = nct5104d_gpio_direction_out,	\
			.set              = nct5104d_gpio_set,		\
			.base             = _base,			\
			.ngpio            = _ngpio,			\
			.can_sleep        = true,			\
		},							\
		.regbase = _regbase,					\
	}

#define gpio_dir(base) (base + 0)
#define gpio_data(base) (base + 1)

static struct nct5104d_gpio_bank nct5104d_gpio_bank[] = {
	NCT5104D_GPIO_BANK(0 , 8, 0xE0),
	NCT5104D_GPIO_BANK(10, 8, 0xE4)
};

static int nct5104d_gpio_direction_in(struct gpio_chip *chip, unsigned offset)
{
	int err;
	struct nct5104d_gpio_bank *bank =
		container_of(chip, struct nct5104d_gpio_bank, chip);
	struct nct5104d_sio *sio = bank->data->sio;
	u8 dir;

	err = superio_enter(sio->addr);
	if (err)
		return err;
	superio_select(sio->addr, SIO_LD_GPIO);

	dir = superio_inb(sio->addr, gpio_dir(bank->regbase));
	dir |= (1 << offset);
	superio_outb(sio->addr, gpio_dir(bank->regbase), dir);

	superio_exit(sio->addr);

	return 0;
}

static int nct5104d_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int err;
	struct nct5104d_gpio_bank *bank =
		container_of(chip, struct nct5104d_gpio_bank, chip);
	struct nct5104d_sio *sio = bank->data->sio;
	u8 data;

	err = superio_enter(sio->addr);
	if (err)
		return err;
	superio_select(sio->addr, SIO_LD_GPIO);

	data = superio_inb(sio->addr, gpio_data(bank->regbase));

	superio_exit(sio->addr);

	return !!(data & 1 << offset);
}

static int nct5104d_gpio_direction_out(struct gpio_chip *chip,
				     unsigned offset, int value)
{
	int err;
	struct nct5104d_gpio_bank *bank =
		container_of(chip, struct nct5104d_gpio_bank, chip);
	struct nct5104d_sio *sio = bank->data->sio;
	u8 dir, data_out;

	err = superio_enter(sio->addr);
	if (err)
		return err;
	superio_select(sio->addr, SIO_LD_GPIO);

	data_out = superio_inb(sio->addr, gpio_data(bank->regbase));
	if (value)
		data_out |= (1 << offset);
	else
		data_out &= ~(1 << offset);
	superio_outb(sio->addr, gpio_data(bank->regbase), data_out);

	dir = superio_inb(sio->addr, gpio_dir(bank->regbase));
	dir &= ~(1 << offset);
	superio_outb(sio->addr, gpio_dir(bank->regbase), dir);

	superio_exit(sio->addr);

	return 0;
}

static void nct5104d_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	int err;
	struct nct5104d_gpio_bank *bank =
		container_of(chip, struct nct5104d_gpio_bank, chip);
	struct nct5104d_sio *sio = bank->data->sio;
	u8 data_out;

	err = superio_enter(sio->addr);
	if (err)
		return;
	superio_select(sio->addr, SIO_LD_GPIO);

	data_out = superio_inb(sio->addr, gpio_data(bank->regbase));
	if (value)
		data_out |= (1 << offset);
	else
		data_out &= ~(1 << offset);
	superio_outb(sio->addr, gpio_data(bank->regbase), data_out);

	superio_exit(sio->addr);
}

/*
 * Platform device and driver.
 */

static int nct5104d_gpio_probe(struct platform_device *pdev)
{
	int err;
	int i;
	struct nct5104d_sio *sio = pdev->dev.platform_data;
	struct nct5104d_gpio_data *data;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	switch (sio->type) {
	case nct5104d:
		data->nr_bank = ARRAY_SIZE(nct5104d_gpio_bank);
		data->bank = nct5104d_gpio_bank;
		break;
	default:
		return -ENODEV;
	}
	data->sio = sio;

	platform_set_drvdata(pdev, data);

	/* For each GPIO bank, register a GPIO chip. */
	for (i = 0; i < data->nr_bank; i++) {
		struct nct5104d_gpio_bank *bank = &data->bank[i];

		bank->chip.parent = &pdev->dev;
		bank->data = data;

		err = gpiochip_add(&bank->chip);
		if (err) {
			dev_err(&pdev->dev,
				"Failed to register gpiochip %d: %d\n",
				i, err);
			goto err_gpiochip;
		}
	}

	return 0;

err_gpiochip:
	for (i = i - 1; i >= 0; i--) {
		struct nct5104d_gpio_bank *bank = &data->bank[i];

		gpiochip_remove (&bank->chip);
	}

	return err;
}

static int nct5104d_gpio_remove(struct platform_device *pdev)
{
	int i;
	struct nct5104d_gpio_data *data = platform_get_drvdata(pdev);

	for (i = 0; i < data->nr_bank; i++) {
		struct nct5104d_gpio_bank *bank = &data->bank[i];

		gpiochip_remove (&bank->chip);
	}

	return 0;
}

static int __init nct5104d_find(int addr, struct nct5104d_sio *sio)
{
	int err;
	u16 devid;
	u8 gpio_cfg;

	err = superio_enter(addr);
	if (err)
		return err;

	err = -ENODEV;

	devid = superio_inw(addr, SIO_CHIPID);
	switch (devid) {
	case SIO_NCT5104D_ID:
	case SIO_PCENGINES_APU_NCT5104D_ID1:
	case SIO_PCENGINES_APU_NCT5104D_ID2:
		sio->type = nct5104d;
		/* enable GPIO0 and GPIO1 */
		superio_select(addr, SIO_LD_GPIO);
		gpio_cfg = superio_inb(addr, SIO_GPIO_ENABLE);
		gpio_cfg |= 0x03;
		superio_outb(addr, SIO_GPIO_ENABLE, gpio_cfg);
		break;
	default:
		pr_info(DRVNAME ": Unsupported device 0x%04x\n", devid);
		goto err;
	}
	sio->addr = addr;
	err = 0;

	pr_info(DRVNAME ": Found %s at %#x chip id 0x%04x\n",
		nct5104d_names[sio->type],
		(unsigned int) addr,
		(int) superio_inw(addr, SIO_CHIPID));

        superio_select(sio->addr, SIO_LD_GPIO_MODE);
        superio_outb(sio->addr, SIO_GPIO1_MODE, 0x0);
        superio_outb(sio->addr, SIO_GPIO2_MODE, 0x0);

err:
	superio_exit(addr);
	return err;
}

static struct platform_device *nct5104d_gpio_pdev;

static int __init
nct5104d_gpio_device_add(const struct nct5104d_sio *sio)
{
	int err;

	nct5104d_gpio_pdev = platform_device_alloc(DRVNAME, -1);
	if (!nct5104d_gpio_pdev)
		pr_err(DRVNAME ": Error platform_device_alloc\n");
	if (!nct5104d_gpio_pdev)
		return -ENOMEM;

	err = platform_device_add_data(nct5104d_gpio_pdev,
				       sio, sizeof(*sio));
	if (err) {
		pr_err(DRVNAME "Platform data allocation failed\n");
		goto err;
	}

	err = platform_device_add(nct5104d_gpio_pdev);
	if (err) {
		pr_err(DRVNAME "Device addition failed\n");
		goto err;
	}
	pr_info(DRVNAME ": Device added\n");
	return 0;

err:
	platform_device_put(nct5104d_gpio_pdev);

	return err;
}

/*
 */

static struct platform_driver nct5104d_gpio_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
	.probe		= nct5104d_gpio_probe,
	.remove		= nct5104d_gpio_remove,
};

static int __init nct5104d_gpio_init(void)
{
	int err;
	struct nct5104d_sio sio;
	const char *board_vendor = dmi_get_system_info(DMI_BOARD_VENDOR);
	const char *board_name = dmi_get_system_info(DMI_BOARD_NAME);

	if (nct5104d_find(0x2e, &sio) &&
	    nct5104d_find(0x4e, &sio))
		return -ENODEV;

	err = platform_driver_register(&nct5104d_gpio_driver);
	if (!err) {
		pr_info(DRVNAME ": platform_driver_register\n");
		err = nct5104d_gpio_device_add(&sio);
		if (err)
			platform_driver_unregister(&nct5104d_gpio_driver);
	}

	return err;
}
subsys_initcall(nct5104d_gpio_init);

static void __exit nct5104d_gpio_exit(void)
{
	platform_device_unregister(nct5104d_gpio_pdev);
	platform_driver_unregister(&nct5104d_gpio_driver);
}
module_exit(nct5104d_gpio_exit);

MODULE_DESCRIPTION("GPIO driver for Super-I/O chips NCT5104D");
MODULE_AUTHOR("Tasanakorn Phaipool <tasanakorn@gmail.com>");
MODULE_LICENSE("GPL");
