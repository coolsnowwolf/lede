// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for the Phytium keypad port.
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/input/matrix_keypad.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/acpi.h>

/*
 * Keypad Controller registers
 */
#define KPCR		0x00 /* Keypad Control Register */

#define KPSR		0x04 /* Keypad Status Register */
#define KBD_STAT_KPKD	(0x1 << 0) /* Key Press Interrupt Status bit (w1c) */
#define KBD_STAT_KPKR	(0x1 << 1) /* Key Release Interrupt Status bit (w1c) */
#define KBD_STAT_KDSC	(0x1 << 2) /* Key Depress Synch Chain Status bit (w1c)*/
#define KBD_STAT_KRSS	(0x1 << 3) /* Key Release Synch Status bit (w1c)*/
#define KBD_STAT_KDIE	(0x1 << 8) /* Key Depress Interrupt Enable Status bit */
#define KBD_STAT_KRIE	(0x1 << 9) /* Key Release Interrupt Enable */

#define KDDR		0x08 /* Keypad Data Direction Register */
#define KPDR		0x0C /* Keypad Data Register */

#define MAX_MATRIX_KEY_ROWS	8
#define MAX_MATRIX_KEY_COLS	8

#define MAX_MATRIX_KEY_NUM	(MAX_MATRIX_KEY_ROWS * MAX_MATRIX_KEY_COLS)

struct phytium_keypad {
	struct input_dev *input_dev;
	void __iomem *mmio_base;

	int			irq;
	struct timer_list	check_matrix_timer;

	/*
	 * The matrix is stable only if no changes are detected after
	 * PHYTIUM_KEYPAD_SCANS_FOR_STABILITY scans
	 */
#define PHYTIUM_KEYPAD_SCANS_FOR_STABILITY 3

	int			stable_count;

	bool			enabled;

	unsigned int	n_rows;
	unsigned int	n_cols;
	int		row_shift;

	/* Masks for enabled rows/cols */
	unsigned short		rows_en_mask;
	unsigned short		cols_en_mask;

	unsigned short		keycodes[MAX_MATRIX_KEY_NUM];

	/*
	 * Matrix states:
	 * -stable: achieved after a complete debounce process.
	 * -unstable: used in the debouncing process.
	 */
	unsigned short		matrix_stable_state[MAX_MATRIX_KEY_COLS];
	unsigned short		matrix_unstable_state[MAX_MATRIX_KEY_COLS];
};

static u32 phytium_read(struct phytium_keypad *keypad, int reg)
{
	return readl(keypad->mmio_base + reg);
}

static void phytium_write(struct phytium_keypad *keypad,  u32 value, int reg)
{
	writel(value, keypad->mmio_base + reg);
}

/* Scan the matrix and return the new state in *matrix_volatile_state. */
static void phytium_keypad_scan_matrix(struct phytium_keypad *keypad,
				       unsigned short *matrix_volatile_state)
{
	int col;
	u32 reg_val;

	for (col = 0; col < keypad->n_cols; col++) {
		if ((keypad->cols_en_mask & (1 << col)) == 0)
			continue;
		/*
		 * Discharge keypad capacitance:
		 * 2. write 0s on KDDR[KCD], configure columns as input.
		 */
		reg_val = phytium_read(keypad, KDDR);
		reg_val = 0x00000000;
		phytium_write(keypad, reg_val, KDDR);

		/*
		 * 3. Write a single column to 0, others to 1.
		 * 4. Sample row inputs and save data.
		 * 5. Repeat steps 3 - 4 for remaining columns.
		 */
		reg_val = 0;
		reg_val |= (1 << (16 + col));
		phytium_write(keypad, reg_val, KDDR);
		reg_val = phytium_read(keypad, KPDR);
		reg_val = 0x00000000;
		phytium_write(keypad, reg_val, KPDR);

		/*
		 * Delay added to avoid propagating the 0 from column to row
		 * when scanning.
		 */
		udelay(5);

		/*
		 * 1s in matrix_volatile_state[col] means key pressures
		 * throw data from non enabled rows.
		 */
		reg_val = phytium_read(keypad, KPDR);
		matrix_volatile_state[col] = (~reg_val) & keypad->rows_en_mask;
	}

	/*
	 * Return in standby mode:
	 * 6. write 0s to columns
	 */
	/* Configure columns as output, output 0 */
	reg_val = 0;
	reg_val |= (keypad->cols_en_mask & 0xffff) << 16;
	phytium_write(keypad, reg_val, KDDR);
	phytium_write(keypad, 0x00000000, KPDR);
}

/*
 * Compare the new matrix state (volatile) with the stable one stored in
 * keypad->matrix_stable_state and fire events if changes are detected.
 */
static void phytium_keypad_fire_events(struct phytium_keypad *keypad,
				       unsigned short *matrix_volatile_state)
{
	struct input_dev *input_dev = keypad->input_dev;
	int row, col;

	for (col = 0; col < keypad->n_cols; col++) {
		unsigned short bits_changed;
		int code;

		if ((keypad->cols_en_mask & (1 << col)) == 0)
			continue; /* Column is not enabled */

		bits_changed = keypad->matrix_stable_state[col] ^
						matrix_volatile_state[col];

		if (bits_changed == 0)
			continue; /* Column does not contain changes */

		for (row = 0; row < keypad->n_rows; row++) {
			if ((keypad->rows_en_mask & (1 << row)) == 0)
				continue; /* Row is not enabled */
			if ((bits_changed & (1 << row)) == 0)
				continue; /* Row does not contain changes */

			code = MATRIX_SCAN_CODE(row, col, keypad->row_shift);
			input_event(input_dev, EV_MSC, MSC_SCAN, code);
			input_report_key(input_dev, keypad->keycodes[code],
					 matrix_volatile_state[col] & (1 << row));
			dev_dbg(&input_dev->dev, "Event code: %d, val: %d",
				keypad->keycodes[code],
				matrix_volatile_state[col] & (1 << row));
		}
	}
	input_sync(input_dev);
}

/*
 * phytium_keypad_check_for_events is the timer handler.
 */
static void phytium_keypad_check_for_events(struct timer_list *t)
{
	struct phytium_keypad *keypad = from_timer(keypad, t, check_matrix_timer);
	unsigned short matrix_volatile_state[MAX_MATRIX_KEY_COLS];
	u32 reg_val;
	bool state_changed, is_zero_matrix;
	int i;

	memset(matrix_volatile_state, 0, sizeof(matrix_volatile_state));

	phytium_keypad_scan_matrix(keypad, matrix_volatile_state);

	state_changed = false;
	for (i = 0; i < keypad->n_cols; i++) {
		if ((keypad->cols_en_mask & (1 << i)) == 0)
			continue;

		if (keypad->matrix_unstable_state[i] ^ matrix_volatile_state[i]) {
			state_changed = true;
			break;
		}
	}

	/*
	 * If the matrix state is changed from the previous scan
	 *   (Re)Begin the debouncing process, saving the new state in
	 *    keypad->matrix_unstable_state.
	 * else
	 *   Increase the count of number of scans with a stable state.
	 */
	if (state_changed) {
		memcpy(keypad->matrix_unstable_state, matrix_volatile_state,
			sizeof(matrix_volatile_state));
		keypad->stable_count = 0;
	} else {
		keypad->stable_count++;
	}

	/*
	 * If the matrix is not as stable as we want reschedule scan
	 * in the near future.
	 */
	if (keypad->stable_count < PHYTIUM_KEYPAD_SCANS_FOR_STABILITY) {
		mod_timer(&keypad->check_matrix_timer,
			  jiffies + msecs_to_jiffies(10));
		return;
	}

	/*
	 * If the matrix state is stable, fire the events and save the new
	 * stable state. Note, if the matrix is kept stable for longer
	 * (keypad->stable_count > PHYTIUM_KEYPAD_SCANS_FOR_STABILITY) all
	 * events have already been generated.
	 */
	if (keypad->stable_count == PHYTIUM_KEYPAD_SCANS_FOR_STABILITY) {
		phytium_keypad_fire_events(keypad, matrix_volatile_state);
		memcpy(keypad->matrix_stable_state, matrix_volatile_state,
			sizeof(matrix_volatile_state));
	}

	is_zero_matrix = true;
	for (i = 0; i < keypad->n_cols; i++) {
		if (matrix_volatile_state[i] != 0) {
			is_zero_matrix = false;
			break;
		}
	}

	if (is_zero_matrix) {
		/*
		 * All keys have been released. Enable only the KDI
		 * interrupt for future key presses (clear the KDI
		 * status bit and its sync chain before that).
		 */
		reg_val = phytium_read(keypad, KPSR);
		reg_val |= KBD_STAT_KPKD | KBD_STAT_KDSC;
		phytium_write(keypad, reg_val, KPSR);

		reg_val = phytium_read(keypad, KPSR);
		reg_val |= KBD_STAT_KDIE;
		reg_val &= ~KBD_STAT_KRIE;
		phytium_write(keypad, reg_val, KPSR);
	} else {
		/*
		 * Some keys are still pressed. Schedule a rescan in
		 * attempt to detect multiple key presses and enable
		 * the KRI interrupt to react quickly to key release
		 * event.
		 */
		mod_timer(&keypad->check_matrix_timer,
			  jiffies + msecs_to_jiffies(60));

		reg_val = phytium_read(keypad, KPSR);
		reg_val |= KBD_STAT_KPKR | KBD_STAT_KRSS;
		phytium_write(keypad, reg_val, KPSR);

		reg_val = phytium_read(keypad, KPSR);
		reg_val |= KBD_STAT_KRIE;
		reg_val &= ~KBD_STAT_KDIE;
		phytium_write(keypad, reg_val, KPSR);
	}
}

static irqreturn_t phytium_keypad_irq_handler(int irq, void *dev_id)
{
	struct phytium_keypad *keypad = dev_id;
	u32 reg_val;

	reg_val = phytium_read(keypad, KPSR);
	/* Disable both interrupt types */
	reg_val &= ~(KBD_STAT_KRIE | KBD_STAT_KDIE);
	/* Clear interrupts status bits */
	reg_val |= KBD_STAT_KPKR | KBD_STAT_KPKD;
	phytium_write(keypad, reg_val,  KPSR);

	if (keypad->enabled) {
		/* The matrix is supposed to be changed */
		keypad->stable_count = 0;

		/* Schedule the scanning procedure near in the future */
		mod_timer(&keypad->check_matrix_timer,
			  jiffies + msecs_to_jiffies(2));
	}

	return IRQ_HANDLED;
}

static void phytium_keypad_config(struct phytium_keypad *keypad)
{
	u32 reg_val;

	/*
	 * Include enabled rows in interrupt generation (KPCR[15:0])
	 * Configure keypad columns as open-drain (KPCR[31:16])
	 */
	reg_val = phytium_read(keypad, KPCR);
	reg_val |= keypad->rows_en_mask & 0xffff;	  /* rows */
	reg_val |= (keypad->cols_en_mask & 0xffff) << 16; /* cols */
	phytium_write(keypad, reg_val, KPCR);

	/* Configure columns as output, output 0 */
	reg_val = 0;
	reg_val |= (keypad->cols_en_mask & 0xffff) << 16;
	phytium_write(keypad, reg_val, KDDR);
	phytium_write(keypad, 0x00000000, KPDR);

	/*
	 * Clear Key Depress and Key Release status bit.
	 * Clear both synchronizer chain.
	 */
	reg_val = phytium_read(keypad, KPSR);
	reg_val |= KBD_STAT_KPKR | KBD_STAT_KPKD |
		   KBD_STAT_KDSC | KBD_STAT_KRSS;
	phytium_write(keypad, reg_val, KPSR);

	/* Enable KDI and disable KRI (avoid false release events). */
	reg_val |= KBD_STAT_KDIE;
	reg_val &= ~KBD_STAT_KRIE;
	phytium_write(keypad, reg_val, KPSR);
}

static void phytium_keypad_inhibit(struct phytium_keypad *keypad)
{
	unsigned short reg_val;

	/* Inhibit KDI and KRI interrupts. */
	reg_val = phytium_read(keypad, KPSR);
	reg_val &= ~(KBD_STAT_KRIE | KBD_STAT_KDIE);
	reg_val |= KBD_STAT_KPKR | KBD_STAT_KPKD;
	phytium_write(keypad, reg_val, KPSR);
}

static void phytium_keypad_close(struct input_dev *dev)
{
	struct phytium_keypad *keypad = input_get_drvdata(dev);

	dev_dbg(&dev->dev, ">%s\n", __func__);

	/* Mark keypad as being inactive */
	keypad->enabled = false;
	synchronize_irq(keypad->irq);
	del_timer_sync(&keypad->check_matrix_timer);

	phytium_keypad_inhibit(keypad);
}

static int phytium_keypad_open(struct input_dev *dev)
{
	struct phytium_keypad *keypad = input_get_drvdata(dev);

	dev_dbg(&dev->dev, ">%s\n", __func__);

	/* We became active from now */
	keypad->enabled = true;

	phytium_keypad_config(keypad);

	/* Sanity control, not all the rows must be activated now. */
	if ((phytium_read(keypad, KPDR) & keypad->rows_en_mask) == 0) {
		dev_err(&dev->dev,
			"too many keys pressed, control pins initialisation\n");
		goto open_err;
	}

	return 0;

open_err:
	phytium_keypad_close(dev);
	return -EIO;
}

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_keypad_acpi_ids[] = {
       { "PHYT0028", 0 },
       { /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_keypad_acpi_ids);
#endif

#ifdef CONFIG_OF
static const struct of_device_id phytium_keypad_of_match[] = {
	{ .compatible = "phytium,keypad", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, phytium_keypad_of_match);
#endif

static int phytium_keypad_probe(struct platform_device *pdev)
{
	const struct matrix_keymap_data *keymap_data = dev_get_platdata(&pdev->dev);
	struct phytium_keypad *keypad;
	struct input_dev *input_dev;
	struct resource *res;
	int irq, error, i, row, col;

	if (!keymap_data && !pdev->dev.of_node && !has_acpi_companion(&pdev->dev)) {
		dev_err(&pdev->dev, "no keymap defined\n");
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq defined in platform data\n");
		return irq;
	}

	input_dev = devm_input_allocate_device(&pdev->dev);
	if (!input_dev) {
		dev_err(&pdev->dev, "failed to allocate the input device\n");
		return -ENOMEM;
	}

	keypad = devm_kzalloc(&pdev->dev, sizeof(*keypad), GFP_KERNEL);
	if (!keypad)
		return -ENOMEM;

	keypad->input_dev = input_dev;
	keypad->irq = irq;
	keypad->stable_count = 0;

	timer_setup(&keypad->check_matrix_timer,
		    phytium_keypad_check_for_events, 0);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	keypad->mmio_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(keypad->mmio_base))
		return PTR_ERR(keypad->mmio_base);

	/* Init the Input device */
	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;
	input_dev->open = phytium_keypad_open;
	input_dev->close = phytium_keypad_close;

	error = matrix_keypad_parse_properties(&pdev->dev, &keypad->n_rows, &keypad->n_cols);
	if (error) {
		dev_err(&pdev->dev, "failed to parse phytium kp params\n");
		return error;
	}

	error = matrix_keypad_build_keymap(keymap_data, NULL,
					   keypad->n_rows,
					   keypad->n_cols,
					   keypad->keycodes, input_dev);
	if (error) {
		dev_err(&pdev->dev, "failed to build keymap\n");
		return error;
	}

	keypad->row_shift = get_count_order(keypad->n_cols);

	/* Search for rows and cols enabled */
	for (row = 0; row < keypad->n_rows; row++) {
		for (col = 0; col <  keypad->n_cols; col++) {
			i = MATRIX_SCAN_CODE(row, col, keypad->row_shift);
			if (keypad->keycodes[i] != KEY_RESERVED) {
				keypad->rows_en_mask |= 1 << row;
				keypad->cols_en_mask |= 1 << col;
			}
		}
	}

	__set_bit(EV_REP, input_dev->evbit);
	input_set_capability(input_dev, EV_MSC, MSC_SCAN);
	input_set_drvdata(input_dev, keypad);

	phytium_keypad_inhibit(keypad);

	error = devm_request_irq(&pdev->dev, irq, phytium_keypad_irq_handler, 0,
			    pdev->name, keypad);
	if (error) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		return error;
	}

	/* Register the input device */
	error = input_register_device(input_dev);
	if (error) {
		dev_err(&pdev->dev, "failed to register input device\n");
		return error;
	}

	platform_set_drvdata(pdev, keypad);
	device_init_wakeup(&pdev->dev, 1);

	return 0;
}

static int phytium_keypad_remove(struct platform_device *pdev)
{
	struct phytium_keypad *keypad = platform_get_drvdata(pdev);

	input_unregister_device(keypad->input_dev);
	devm_kfree(&pdev->dev, keypad);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phytium_keypad_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct phytium_keypad *keypad = platform_get_drvdata(pdev);
	struct input_dev *input_dev = keypad->input_dev;

	mutex_lock(&input_dev->mutex);

	if (input_dev->users)
		phytium_keypad_inhibit(keypad);

	mutex_unlock(&input_dev->mutex);

	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(keypad->irq);

	return 0;
}

static int phytium_keypad_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct phytium_keypad *keypad = platform_get_drvdata(pdev);
	struct input_dev *input_dev = keypad->input_dev;
	int ret = 0;

	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(keypad->irq);

	mutex_lock(&input_dev->mutex);

	if (input_dev->users)
		phytium_keypad_config(keypad);

	mutex_unlock(&input_dev->mutex);

	return ret;
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_keypad_pm_ops, phytium_keypad_suspend, phytium_keypad_resume);

static struct platform_driver phytium_keypad_driver = {
	.driver		= {
		.name	= "phytium-keypad",
		.pm	= &phytium_keypad_pm_ops,
		.of_match_table = of_match_ptr(phytium_keypad_of_match),
		.acpi_match_table = ACPI_PTR(phytium_keypad_acpi_ids),
	},
	.probe      = phytium_keypad_probe,
	.remove     = phytium_keypad_remove,
};
module_platform_driver(phytium_keypad_driver);

MODULE_AUTHOR("Song Wenting <songwenting@phytium.com>");
MODULE_DESCRIPTION("PHYTIUM Keypad Port Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:phytium-keypad");
