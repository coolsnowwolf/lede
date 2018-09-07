#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clk.h>

#include <asm/time.h>
#include <adm8668.h>

void __init plat_time_init(void)
{
	struct clk *sys_clk;

	adm8668_init_clocks();

	sys_clk = clk_get(NULL, "sys");
	if (IS_ERR(sys_clk))
		panic("unable to get system clock\n");

	mips_hpt_frequency = clk_get_rate(sys_clk) / 2;
}

