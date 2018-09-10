/*
 * arch/arm/mach-mcs814x/common.c
 *
 * Core functions for Moschip MCS814x SoCs
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/reboot.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/mcs814x.h>
#include <mach/cpu.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>

void __iomem *mcs814x_sysdbg_base;

static struct map_desc mcs814x_io_desc[] __initdata = {
	{
		.virtual	= MCS814X_IO_BASE,
		.pfn		= __phys_to_pfn(MCS814X_IO_START),
		.length		= MCS814X_IO_SIZE,
		.type		= MT_DEVICE
	},
};

struct cpu_mode {
	const char *name;
	int gpio_start;
	int gpio_end;
};

static const struct cpu_mode cpu_modes[] = {
	{
		.name		= "I2S",
		.gpio_start	= 4,
		.gpio_end	= 8,
	},
	{
		.name		= "UART",
		.gpio_start	= 4,
		.gpio_end	= 9,
	},
	{
		.name		= "External MII",
		.gpio_start	= 0,
		.gpio_end	= 16,
	},
	{
		.name		= "Normal",
		.gpio_start	= -1,
		.gpio_end	= -1,
	},
};

static void mcs814x_eth_hardware_filter_set(u8 value)
{
	u32 reg;

	reg = readl_relaxed(MCS814X_VIRT_BASE + MCS814X_DBGLED);
	if (value)
		reg |= 0x80;
	else
		reg &= ~0x80;
	writel_relaxed(reg, MCS814X_VIRT_BASE + MCS814X_DBGLED);
}

static void mcs814x_eth_led_cfg_set(u8 cfg)
{
	u32 reg;

	reg = readl_relaxed(mcs814x_sysdbg_base + SYSDBG_BS2);
	reg &= ~LED_CFG_MASK;
	reg |= cfg;
	writel_relaxed(reg, mcs814x_sysdbg_base + SYSDBG_BS2);
}

static void mcs814x_eth_buffer_shifting_set(u8 value)
{
	u8 reg;

	reg = readb_relaxed(mcs814x_sysdbg_base + SYSDBG_SYSCTL_MAC);
	if (value)
		reg |= BUF_SHIFT_BIT;
	else
		reg &= ~BUF_SHIFT_BIT;
	writeb_relaxed(reg, mcs814x_sysdbg_base + SYSDBG_SYSCTL_MAC);
}

static struct of_device_id mcs814x_eth_ids[] __initdata = {
	{ .compatible = "moschip,nuport-mac", },
	{ /* sentinel */ },
};

/* Configure platform specific knobs based on ethernet device node
 * properties */
static void mcs814x_eth_init(void)
{
	struct device_node *np;
	const unsigned int *intspec;

	np = of_find_matching_node(NULL, mcs814x_eth_ids);
	if (!np)
		return;

	/* hardware filter must always be enabled */
	mcs814x_eth_hardware_filter_set(1);

	intspec = of_get_property(np, "nuport-mac,buffer-shifting", NULL);
	if (!intspec)
		mcs814x_eth_buffer_shifting_set(0);
	else
		mcs814x_eth_buffer_shifting_set(1);

	intspec = of_get_property(np, "nuport-mac,link-activity", NULL);
	if (intspec)
		mcs814x_eth_led_cfg_set(be32_to_cpup(intspec));

	of_node_put(np);
}

void __init mcs814x_init_machine(void)
{
	u32 bs2, cpu_mode;
	int gpio;

	bs2 = readl_relaxed(mcs814x_sysdbg_base + SYSDBG_BS2);
	cpu_mode = (bs2 >> CPU_MODE_SHIFT) & CPU_MODE_MASK;

	pr_info("CPU mode: %s\n", cpu_modes[cpu_mode].name);

	/* request the gpios since the pins are muxed for functionnality */
	for (gpio = cpu_modes[cpu_mode].gpio_start;
		gpio == cpu_modes[cpu_mode].gpio_end; gpio++) {
		if (gpio != -1)
			gpio_request(gpio, cpu_modes[cpu_mode].name);
	}

	mcs814x_eth_init();
}

void __init mcs814x_map_io(void)
{
	iotable_init(mcs814x_io_desc, ARRAY_SIZE(mcs814x_io_desc));

	mcs814x_sysdbg_base = ioremap(MCS814X_IO_START + MCS814X_SYSDBG,
					MCS814X_SYSDBG_SIZE);
	if (!mcs814x_sysdbg_base)
		panic("unable to remap sysdbg base");
}

void mcs814x_restart(enum reboot_mode mode, const char *cmd)
{
	writel_relaxed(~(1 << 31), mcs814x_sysdbg_base);
}
