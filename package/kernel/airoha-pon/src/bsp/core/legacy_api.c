#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include "ledctrl_kernel_api.h"

static int __init legacy_api_init(void)
{
	struct proc_dir_entry *legacy_api_proc_dir;
	printk("legacy_api_initialized\n");
	
	legacy_api_proc_dir = proc_mkdir("legacy_api", NULL);

	led_init();
	gpioctl_init();

	return 0;
}

static void __exit legacy_api_exit(void)
{
	led_exit();
	gpioctl_exit();
	remove_proc_entry("legacy_api", NULL);
	printk("legacy_api_removed\n");
}

module_init(legacy_api_init);
module_exit(legacy_api_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Legacy API driver");
