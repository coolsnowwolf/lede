/*
 *	ADM5120_WDT 0.01: Infineon ADM5120 SoC watchdog driver
 *	Copyright (c) Ondrej Zajicek <santiago@crfreenet.org>, 2007
 *
 *	based on
 *
 *	RC32434_WDT 0.01: IDT Interprise 79RC32434 watchdog driver
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/irq.h>

#include <asm/bootinfo.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>

#define DEFAULT_TIMEOUT	15		/* (secs) Default is 15 seconds */
#define MAX_TIMEOUT	327
/* Max is 327 seconds, counter is 15-bit integer, step is 10 ms */

#define NAME "adm5120_wdt"
#define VERSION "0.1"

static int expect_close;
static int access;
static unsigned int timeout = DEFAULT_TIMEOUT;

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");
MODULE_LICENSE("GPL");


static inline void wdt_set_timeout(void)
{
	u32 val = (1 << 31) | (((timeout * 100) & 0x7FFF) << 16);
	SW_WRITE_REG(SWITCH_REG_WDOG0, val);
}

/*
   It looks like WDOG0-register-write don't modify counter,
   but WDOG0-register-read resets counter.
*/

static inline void wdt_reset_counter(void)
{
	SW_READ_REG(SWITCH_REG_WDOG0);
}

static inline void wdt_disable(void)
{
	SW_WRITE_REG(SWITCH_REG_WDOG0, 0x7FFF0000);
}



static int wdt_open(struct inode *inode, struct file *file)
{
	/* Allow only one person to hold it open */
	if (access)
		return -EBUSY;

	if (nowayout)
		__module_get(THIS_MODULE);

	/* Activate timer */
	wdt_reset_counter();
	wdt_set_timeout();
	printk(KERN_INFO NAME ": enabling watchdog timer\n");
	access = 1;
	return 0;
}

static int wdt_release(struct inode *inode, struct file *file)
{
	/*
	 * Shut off the timer.
	 * Lock it in if it's a module and we set nowayout
	 */
	if (expect_close && (nowayout == 0)) {
		wdt_disable();
		printk(KERN_INFO NAME ": disabling watchdog timer\n");
		module_put(THIS_MODULE);
	} else
		printk(KERN_CRIT NAME ": device closed unexpectedly.  WDT will not stop!\n");

	access = 0;
	return 0;
}

static ssize_t wdt_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/* Refresh the timer. */
	if (len) {
		if (!nowayout) {
			size_t i;

			/* In case it was set long ago */
			expect_close = 0;

			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					expect_close = 1;
			}
		}
		wdt_reset_counter();
		return len;
	}
	return 0;
}

static long wdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int new_timeout;
	static struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_KEEPALIVEPING |
					WDIOF_MAGICCLOSE,
		.firmware_version =	0,
		.identity =		"ADM5120_WDT Watchdog",
	};
	switch (cmd) {
	default:
		return -ENOTTY;
	case WDIOC_GETSUPPORT:
		if (copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident)))
			return -EFAULT;
		return 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, (int *)arg);
	case WDIOC_KEEPALIVE:
		wdt_reset_counter();
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(new_timeout, (int *)arg))
			return -EFAULT;
		if (new_timeout < 1)
			return -EINVAL;
		if (new_timeout > MAX_TIMEOUT)
			return -EINVAL;
		timeout = new_timeout;
		wdt_set_timeout();
		/* Fall */
	case WDIOC_GETTIMEOUT:
		return put_user(timeout, (int *)arg);
	}
}

static const struct file_operations wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= wdt_write,
	.unlocked_ioctl	= wdt_ioctl,
	.open		= wdt_open,
	.release	= wdt_release,
};

static struct miscdevice wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &wdt_fops,
};

static char banner[] __initdata = KERN_INFO NAME ": Watchdog Timer version " VERSION "\n";

static int __init watchdog_init(void)
{
	int ret;

	ret = misc_register(&wdt_miscdev);

	if (ret)
		return ret;

	wdt_disable();
	printk(banner);

	return 0;
}

static void __exit watchdog_exit(void)
{
	misc_deregister(&wdt_miscdev);
}

module_init(watchdog_init);
module_exit(watchdog_exit);
