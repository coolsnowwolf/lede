#include <linux/init.h>
#include <linux/reboot.h>

#include <asm/reboot.h>
#include <adm8668.h>

static void adm8668_restart(char *cmd)
{
	int i;

	/* the real deal */
	for (i = 0; i < 1000; i++)
		;
	ADM8668_CONFIG_REG(ADM8668_CR1) = 1;
}

void __init plat_mem_setup(void)
{
	_machine_restart = adm8668_restart;
}

const char *get_system_type(void)
{
        unsigned long chipid = ADM8668_CONFIG_REG(ADM8668_CR0);
        int product, revision;
	static char ret[32];

        product = chipid >> 16;
        revision = chipid & 0xffff;

	/* i getting fancy :\ */
	snprintf(ret, sizeof(ret), "ADM%xr%x", product, revision);

	return ret;
}

