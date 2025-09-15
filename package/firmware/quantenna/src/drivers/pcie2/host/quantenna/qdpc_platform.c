/**
 * Copyright (c) 2012-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

/*
 * Platform dependant implement. Customer needs to modify this file.
 */
#include <qdpc_platform.h>
#include <topaz_vnet.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <asm/gpio.h>
#include <../drivers/pcie2/host/common/qdpc_regs.h>

/* WPS button event reported to user space process */
typedef enum {
	MODE_LED_INIT = 0,
	MODE_LED_EXIT,
	MODE_LED_FLASH,
} MODE_LED_OPS;
#define MODE_LED_GPIO 6

typedef enum {
	WPS_BUTTON_NONE_EVENT = 0,
	WPS_BUTTON_WIRELESS_EVENT,
	WPS_BUTTON_DBGDUMP_EVENT,
	WPS_BUTTON_INVALIDE_EVENT
} WPS_Button_Event;
#define WPS_BUTTON_VALID(e) (WPS_BUTTON_NONE_EVENT < (e) && (e) < WPS_BUTTON_INVALIDE_EVENT)

#define WPS_BUTTON_GPIO 4
#define QDPC_WPS_BUTTON_ACTIVE_LEVEL 0
#define WPS_BUTTON_TIMER_INTERVAL ((3 * HZ) / 10) /* timer interval */

/*
* Queue of processes who access wps_button file
*/
DECLARE_WAIT_QUEUE_HEAD(WPS_Button_WaitQ);

static WPS_Button_Event wps_button_event = WPS_BUTTON_NONE_EVENT;
struct timer_list qdpc_wps_button_timer;
static u32 qdpc_wps_button_last_level = ~QDPC_WPS_BUTTON_ACTIVE_LEVEL;
static u32 qdpc_wps_button_down_jiffies = 0; /* records the jiffies when button down, back to 0 after button released */

static int vmac_rst_rc_en = 1;
struct work_struct detect_ep_rst_work;

void enable_vmac_ints(struct vmac_priv *vmp)
{
	uint32_t temp = readl(QDPC_RC_SYS_CTL_PCIE_INT_MASK);

	if(vmp->msi_enabled) {
		temp |= BIT(10); /* MSI */
	} else {
		temp |= BIT(11); /* Legacy INTx */
	}
	writel(temp, QDPC_RC_SYS_CTL_PCIE_INT_MASK);
}

void disable_vmac_ints(struct vmac_priv *vmp)
{
	uint32_t temp = readl(QDPC_RC_SYS_CTL_PCIE_INT_MASK);

	if(vmp->msi_enabled) {
		temp &= ~BIT(10); /* MSI */
	} else {
		temp &= ~BIT(11); /* Legacy INTx */
	}
	writel(temp, QDPC_RC_SYS_CTL_PCIE_INT_MASK);
}

static ssize_t vmac_reset_get(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", vmac_rst_rc_en);
}

static ssize_t vmac_reset_set(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
        uint8_t cmd;

        cmd = (uint8_t)simple_strtoul(buf, NULL, 10);
	if (cmd == 0)
		vmac_rst_rc_en = 0;
	else
		vmac_rst_rc_en = 1;

        return count;
}
DEVICE_ATTR(enable_reset, S_IWUSR | S_IRUSR, vmac_reset_get, vmac_reset_set);

static void detect_ep_rst(struct work_struct *data)
{
	kernel_restart(NULL);
}

void enable_ep_rst_detection(struct net_device *ndev)
{
        uint32_t temp = readl(QDPC_RC_SYS_CTL_PCIE_INT_MASK);

        temp |= QDPC_INTR_EP_RST_MASK;
        writel(temp, QDPC_RC_SYS_CTL_PCIE_INT_MASK);

	device_create_file(&ndev->dev, &dev_attr_enable_reset);
	INIT_WORK(&detect_ep_rst_work, detect_ep_rst);
}

void disable_ep_rst_detection(struct net_device *ndev)
{
        uint32_t temp = readl(QDPC_RC_SYS_CTL_PCIE_INT_MASK);

        temp &= ~QDPC_INTR_EP_RST_MASK;
        writel(temp, QDPC_RC_SYS_CTL_PCIE_INT_MASK);

	device_remove_file(&ndev->dev, &dev_attr_enable_reset);
}

void handle_ep_rst_int(struct net_device *ndev)
{
	uint32_t status = readl(QDPC_RC_SYS_CTL_PCIE_INT_STAT);

	if ((status & QDPC_INTR_EP_RST_MASK) == 0)
		return;

	/* Clear pending interrupt */
	writel(QDPC_INTR_EP_RST_MASK, QDPC_RC_SYS_CTL_PCIE_INT_STAT);

	printk("Detected reset of Endpoint\n");

	if (vmac_rst_rc_en == 1) {
		netif_stop_queue(ndev);
		schedule_work(&detect_ep_rst_work);
	}
}

static void qdpc_mode_led(struct net_device *ndev, MODE_LED_OPS op)
{
	static int inited = 0;
	static u32 led_bk = 0;

	switch(op) {
	case MODE_LED_INIT:
		if (gpio_request(MODE_LED_GPIO, ndev->name) < 0)
			printk(KERN_INFO "%s: Failed to request GPIO%d for GPIO reset\n",
			       ndev->name, MODE_LED_GPIO);

		led_bk = gpio_get_value(MODE_LED_GPIO);
		gpio_direction_output(MODE_LED_GPIO, led_bk);
		inited = 1;

		break;

	case MODE_LED_EXIT:
		if (inited) {
			gpio_set_value(MODE_LED_GPIO, led_bk);
			gpio_free(MODE_LED_GPIO);
			inited = 0;
		}

		break;

	case MODE_LED_FLASH:
		if (inited)
			gpio_set_value(MODE_LED_GPIO, ~gpio_get_value(MODE_LED_GPIO) & 0x01);

		break;
	}
}


static void qdpc_wps_button_event_wakeup(struct net_device *ndev, WPS_Button_Event event)
{
	struct vmac_priv *priv = netdev_priv(ndev);

	if (!WPS_BUTTON_VALID(event))
		return;

	/* notify local watcher */
	wps_button_event = event;
	wake_up_all(&WPS_Button_WaitQ);

	/* notify ep the offline dbg info, if ep is ready*/
	if (priv->ep_ready && event == WPS_BUTTON_DBGDUMP_EVENT)
		writel(TOPAZ_SET_INT(IPC_OFFLINE_DBG), priv->ep_ipc_reg);
}

static ssize_t qdpc_wps_button_read(struct device *dev,
				    struct device_attribute *attr,
				    char *buff)
{
	int i = 0;

	/* As usual, this read is always blocked untill wps button is pressed
	 * so increase the module reference to prevent it being unload during
	 * blocking read
	 */
	if (!try_module_get(THIS_MODULE))
		return 0;

	/* wait for valid WPS button event */
	wait_event_interruptible(WPS_Button_WaitQ, WPS_BUTTON_VALID(wps_button_event));

	/* read back empty string in signal wakeup case */
	for (i = 0; i < _NSIG_WORDS; i++) {
		if (current->pending.signal.sig[i] & ~current->blocked.sig[i]) {
			module_put(THIS_MODULE);
			return 0;
		}
	}

	sprintf(buff, "%d\n", wps_button_event);

	/* after new event been handled, reset to none event */
	wps_button_event = WPS_BUTTON_NONE_EVENT;

	module_put(THIS_MODULE);

	return strlen(buff);
}

static ssize_t qdpc_wps_button_write(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	int input = 0;
	struct net_device *ndev;

	input = simple_strtoul(buf, NULL, 10);

	ndev = (struct net_device*)dev_get_drvdata(dev);

	switch (input) {
	case 1:
		qdpc_mode_led(ndev, MODE_LED_INIT);

		qdpc_mode_led(ndev, MODE_LED_FLASH);
		msleep(300);
		qdpc_mode_led(ndev, MODE_LED_FLASH);
		msleep(300);
		qdpc_mode_led(ndev, MODE_LED_FLASH);
		msleep(300);

		qdpc_mode_led(ndev, MODE_LED_EXIT);

		break;
	default:
		printk(KERN_INFO "WPS button: unknow cmd (%d)\n", input);
	}

	return count;
}

DEVICE_ATTR(wps_button, S_IWUSR | S_IRUSR, qdpc_wps_button_read, qdpc_wps_button_write); /* dev_attr_wps_button */

static void qdpc_wps_button_device_file_create(struct net_device *ndev)
{
	device_create_file(&(ndev->dev), &dev_attr_wps_button);
}


static void qdpc_wps_polling_button_notifier(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	u32 current_level;

	current_level = gpio_get_value(WPS_BUTTON_GPIO);

	/* records the falling edge jiffies */
	if ((current_level == QDPC_WPS_BUTTON_ACTIVE_LEVEL)
	    && (qdpc_wps_button_last_level != QDPC_WPS_BUTTON_ACTIVE_LEVEL)) {

		qdpc_mode_led(dev, MODE_LED_INIT);
		qdpc_wps_button_down_jiffies = jiffies;
	}

	/* at rising edge */
	if ((current_level != QDPC_WPS_BUTTON_ACTIVE_LEVEL)
	    && (qdpc_wps_button_last_level == QDPC_WPS_BUTTON_ACTIVE_LEVEL)) {

		/* WPS button event is rising triggered -- when button
		 * being changed from active to inactive level.
		 *
		 * Different press time trigger different event
		 */
		if ((jiffies - qdpc_wps_button_down_jiffies) >= 10 * HZ) {

			/* wakeup the event waiting processes */
			qdpc_wps_button_event_wakeup(dev, WPS_BUTTON_DBGDUMP_EVENT);

			printk(KERN_INFO "WPS: button long press polling at %u\n", (unsigned int) jiffies);
		} else {
			/* wakeup the event waiting processes */
			qdpc_wps_button_event_wakeup(dev, WPS_BUTTON_WIRELESS_EVENT);

			printk(KERN_INFO "WPS: button short press polling at %u\n", (unsigned int) jiffies);
		}

		/* back to 0 after rising edge */
		qdpc_wps_button_down_jiffies = 0;
		qdpc_mode_led(dev, MODE_LED_EXIT);
	}

	/* after button down more than 10s, begin change the mode led's state to notify user to release button */
	if (qdpc_wps_button_down_jiffies != 0 && ((jiffies - qdpc_wps_button_down_jiffies) >= 10 * HZ)) {
		qdpc_mode_led(dev, MODE_LED_FLASH);
	}

	/* Restart the timer */
	mod_timer(&qdpc_wps_button_timer, jiffies + WPS_BUTTON_TIMER_INTERVAL);

	qdpc_wps_button_last_level = current_level;

	return;
}

int qdpc_wps_button_init(struct net_device *dev)
{
	/*
	 * Set up timer to poll the button.
	 * Request the GPIO resource and export it for userspace
	 */
	if (gpio_request(WPS_BUTTON_GPIO, dev->name) < 0)
		printk(KERN_INFO "%s: Failed to request GPIO%d for GPIO reset\n",
		       dev->name, WPS_BUTTON_GPIO);

	init_timer(&qdpc_wps_button_timer);
	qdpc_wps_button_timer.function = qdpc_wps_polling_button_notifier;
	qdpc_wps_button_timer.data = (unsigned long)dev;
	qdpc_wps_button_timer.expires = jiffies + WPS_BUTTON_TIMER_INTERVAL;
	add_timer(&qdpc_wps_button_timer);

	/* creeate the device file for user space use */
	qdpc_wps_button_device_file_create(dev);

	return 0;
}

void qdpc_wps_button_exit(void)
{
	del_timer(&qdpc_wps_button_timer);
}

