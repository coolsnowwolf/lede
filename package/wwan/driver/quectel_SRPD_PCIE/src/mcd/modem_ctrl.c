/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/gpio/consumer.h>
#include <linux/reboot.h>
#ifdef CONFIG_PCIE_PM_NOTIFY
#include <linux/pcie_notifier.h>
#endif

#include "../include/sprd_pcie_resource.h"
#include "../include/sipc.h"
#include "../include/mdm_ctrl.h"

enum {
	ROC1_SOC = 0,
	ORCA_SOC
};

static char *const mdm_stat[] = {
	"mdm_power_off", "mdm_power_on", "mdm_warm_reset", "mdm_cold_reset",
	"mdm_watchdog_reset", "mdm_assert", "mdm_panic"
};

#define REBOOT_MODEM_DELAY    1000
#define POWERREST_MODEM_DELAY 2000
#define RESET_MODEM_DELAY    50

char cdev_name[] = "mdm_ctrl";

struct modem_ctrl_init_data {
	char *name;
	struct gpio_desc *gpio_poweron;  /* Poweron */
	struct gpio_desc *gpio_reset;    /* Reset modem */
	struct gpio_desc *gpio_preset;   /* Pcie reset */
	struct gpio_desc *gpio_cpwatchdog;
	struct gpio_desc *gpio_cpassert;
	struct gpio_desc *gpio_cppanic;
	struct gpio_desc *gpio_cppoweroff;
	u32 irq_cpwatchdog;
	u32 irq_cpassert;
	u32 irq_cppanic;
	u32 irq_cppoweroff;
	u32 modem_status;
	bool enable_cp_event;
};

struct modem_ctrl_device {
	struct modem_ctrl_init_data *init;
	int major;
	int minor;
	struct cdev cdev;
	struct device *dev;
	int soc_type;
};

static struct class *modem_ctrl_class;
static struct modem_ctrl_device *mcd_dev;

/* modem control evnet notify  */
static ATOMIC_NOTIFIER_HEAD(modem_ctrl_chain);

int modem_ctrl_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&modem_ctrl_chain, nb);
}
EXPORT_SYMBOL(modem_ctrl_register_notifier);

void modem_ctrl_unregister_notifier(struct notifier_block *nb)
{
	atomic_notifier_chain_unregister(&modem_ctrl_chain, nb);
}
EXPORT_SYMBOL(modem_ctrl_unregister_notifier);

static void send_event_msg(struct kobject *kobj)
{
	char *msg[3];
	char buff[100];
	char mbuff[100];

	memset(mbuff, 0, sizeof(mbuff));
	if (!mcd_dev || !mcd_dev->init || !kobj)
		return;

	snprintf(buff, sizeof(buff), "MODEM_STAT=%d",
		 mcd_dev->init->modem_status);
	snprintf(mbuff, sizeof(mbuff), "MODEM_EVENT=%s",
		 mdm_stat[mcd_dev->init->modem_status]);
	msg[0] = buff;
	msg[1] = mbuff;
	msg[2] = NULL;
	kobject_uevent_env(kobj, KOBJ_CHANGE, msg);
	dev_dbg(mcd_dev->dev, "send uevent to userspace\n");
}

static irqreturn_t cpwatchdogtriger_handler(int irq, void *dev_id)
{
	if (!mcd_dev || !mcd_dev->init || !mcd_dev->init->enable_cp_event)
		return IRQ_NONE;

	mcd_dev->init->modem_status = MDM_WATCHDOG_RESET;
	atomic_notifier_call_chain(&modem_ctrl_chain, MDM_WATCHDOG_RESET, NULL);
	send_event_msg(&mcd_dev->dev->kobj);
	return IRQ_HANDLED;
}

static irqreturn_t cpasserttriger_handler(int irq, void *dev_id)
{
	if (!mcd_dev || !mcd_dev->init || !mcd_dev->init->enable_cp_event)
		return IRQ_NONE;

	mcd_dev->init->modem_status = MDM_ASSERT;
	atomic_notifier_call_chain(&modem_ctrl_chain, MDM_ASSERT, NULL);
	send_event_msg(&mcd_dev->dev->kobj);
	return IRQ_HANDLED;
}

static irqreturn_t cppanictriger_handler(int irq, void *dev_id)
{
	if (!mcd_dev || !mcd_dev->init || !mcd_dev->init->enable_cp_event)
		return IRQ_NONE;

	mcd_dev->init->modem_status = MDM_PANIC;
	atomic_notifier_call_chain(&modem_ctrl_chain, MDM_PANIC, NULL);
	send_event_msg(&mcd_dev->dev->kobj);
	return IRQ_HANDLED;
}

static irqreturn_t cppoweroff_handler(int irq, void *dev_id)
{
	if (!mcd_dev || !mcd_dev->init)
		return IRQ_NONE;
	/* To this reserve here for receve power off event from AP*/
	atomic_notifier_call_chain(&modem_ctrl_chain,
				   MDM_POWER_OFF, NULL);
	kernel_power_off();
	return IRQ_HANDLED;
}

static int request_gpio_to_irq(struct gpio_desc *cp_gpio,
			       struct modem_ctrl_device *mcd_dev)
{
	int ret = 0;

	if (!mcd_dev || !mcd_dev->init)
		return -EINVAL;

	ret = gpiod_to_irq(cp_gpio);
	if (ret < 0) {
		dev_err(mcd_dev->dev, "requset irq  %d failed\n", ret);
		return ret;
	}
	dev_dbg(mcd_dev->dev, "gpio to irq  %d\n", ret);
	if (cp_gpio == mcd_dev->init->gpio_cpwatchdog) {
		mcd_dev->init->irq_cpwatchdog = ret;
		ret = devm_request_threaded_irq(mcd_dev->dev,
						mcd_dev->init->irq_cpwatchdog,
					NULL, cpwatchdogtriger_handler,
					IRQF_ONESHOT | IRQF_TRIGGER_FALLING,
					"cpwatchdog_irq", mcd_dev);
		if (ret < 0) {
			dev_err(mcd_dev->dev, "can not request irq for cp watchdog\n");
			return ret;
		}
		enable_irq_wake(mcd_dev->init->irq_cpwatchdog);
	} else if (cp_gpio == mcd_dev->init->gpio_cpassert) {
		mcd_dev->init->irq_cpassert = ret;
		ret = devm_request_threaded_irq(mcd_dev->dev,
						mcd_dev->init->irq_cpassert,
					NULL, cpasserttriger_handler,
					IRQF_ONESHOT | IRQF_TRIGGER_FALLING,
					"cpassert_irq", mcd_dev);
		if (ret < 0) {
			dev_err(mcd_dev->dev, "can not request irq for cp assert\n");
			return ret;
		}
		enable_irq_wake(mcd_dev->init->irq_cpassert);
	} else if (cp_gpio == mcd_dev->init->gpio_cppanic) {
		mcd_dev->init->irq_cppanic = ret;
		ret = devm_request_threaded_irq(mcd_dev->dev,
						mcd_dev->init->irq_cppanic,
					NULL, cppanictriger_handler,
					IRQF_ONESHOT | IRQF_TRIGGER_FALLING,
					"cppanic_irq", mcd_dev);
		if (ret < 0) {
			dev_err(mcd_dev->dev,
				"can not request irq for panic\n");
			return ret;
		}
		enable_irq_wake(mcd_dev->init->irq_cppanic);
	}  else if (cp_gpio == mcd_dev->init->gpio_cppoweroff) {
		mcd_dev->init->irq_cppoweroff = ret;
		ret = devm_request_threaded_irq(mcd_dev->dev,
						mcd_dev->init->irq_cppoweroff,
						NULL, cppoweroff_handler,
						IRQF_ONESHOT | IRQF_TRIGGER_LOW,
						"cppoweroff_irq", mcd_dev);
		if (ret < 0) {
			dev_err(mcd_dev->dev,
				"can not request irq for cppoweroff\n");
			return ret;
		}
		enable_irq_wake(mcd_dev->init->irq_cppoweroff);
	}
	return 0;
}

static int modem_gpios_init(struct modem_ctrl_device *mcd_dev, int soc_type)
{
	int ret;

	if (!mcd_dev || !mcd_dev->init)
		return -EINVAL;
	if (soc_type == ROC1_SOC) {
		gpiod_direction_input(mcd_dev->init->gpio_cpwatchdog);
		gpiod_direction_input(mcd_dev->init->gpio_cpassert);
		gpiod_direction_input(mcd_dev->init->gpio_cppanic);

		ret = request_gpio_to_irq(mcd_dev->init->gpio_cpwatchdog,
					  mcd_dev);
		if (ret)
			return ret;
		ret = request_gpio_to_irq(mcd_dev->init->gpio_cpassert,
					  mcd_dev);
		if (ret)
			return ret;
		ret = request_gpio_to_irq(mcd_dev->init->gpio_cppanic,
					  mcd_dev);
		if (ret)
			return ret;

		/* IRQF_TRIGGER_LOW, default must set to high */
		gpiod_set_value_cansleep(mcd_dev->init->gpio_cppoweroff, 1);
	} else {
		gpiod_direction_input(mcd_dev->init->gpio_cppoweroff);
		ret = request_gpio_to_irq(mcd_dev->init->gpio_cppoweroff,
					  mcd_dev);
		if (ret)
			return ret;

		/* TRIGGER_FALLING, defaultmust  set to high */
		gpiod_set_value_cansleep(mcd_dev->init->gpio_cpwatchdog, 1);
		gpiod_set_value_cansleep(mcd_dev->init->gpio_cpassert, 1);
		gpiod_set_value_cansleep(mcd_dev->init->gpio_cppanic, 1);
	}
	return 0;
}

void modem_ctrl_enable_cp_event(void)
{
	if (mcd_dev && mcd_dev->init)
		mcd_dev->init->enable_cp_event = true;
}
EXPORT_SYMBOL_GPL(modem_ctrl_enable_cp_event);

void modem_ctrl_send_abnormal_to_ap(int status)
{
	struct gpio_desc *gpiodesc;

	if (!mcd_dev || !mcd_dev->init)
		return;
	if (mcd_dev->soc_type != ORCA_SOC) {
		dev_err(mcd_dev->dev, "operation not be allowed for %d\n",
			mcd_dev->soc_type);
		return;
	}
	switch (status) {
	case MDM_WATCHDOG_RESET:
		gpiodesc = mcd_dev->init->gpio_cpwatchdog;
		break;
	case MDM_ASSERT:
		gpiodesc = mcd_dev->init->gpio_cpassert;
		break;
	case MDM_PANIC:
		gpiodesc = mcd_dev->init->gpio_cppanic;
		break;
	default:
		dev_info(mcd_dev->dev,
		"get status %d is not right for operation\n", status);
		return;
	}
	mcd_dev->init->modem_status = status;
	dev_info(mcd_dev->dev,
		"operation unnormal status %d send to ap\n",
		status);
	if (!IS_ERR(gpiodesc))
		gpiod_set_value_cansleep(gpiodesc, 0);
}

static void modem_ctrl_send_cmd_to_cp(int status)
{
	struct gpio_desc *gpiodesc = NULL;

	if (!mcd_dev || !mcd_dev->init)
		return;
	if (mcd_dev->soc_type != ROC1_SOC) {
		dev_err(mcd_dev->dev, "operation not be allowed for %d\n",
			mcd_dev->soc_type);
		return;
	}
	if (status == MDM_POWER_OFF)
		gpiodesc = mcd_dev->init->gpio_cppoweroff;

	mcd_dev->init->modem_status = status;
	dev_info(mcd_dev->dev,
		"operation  cmd %d ms send to cp\n",
		status);
	if (!IS_ERR(gpiodesc)) {
		gpiod_set_value_cansleep(gpiodesc, 0);
		msleep(20);
		gpiod_set_value_cansleep(gpiodesc, 20);
	}
}

static void modem_ctrl_notify_abnormal_status(int status)
{
	if (!mcd_dev || !mcd_dev->init)
		return;
	if (mcd_dev->soc_type != ORCA_SOC) {
		dev_err(mcd_dev->dev, "operation not be allowed for %d\n",
			mcd_dev->soc_type);
		return;
	}
	if (status < MDM_WATCHDOG_RESET || status > MDM_PANIC) {
		dev_err(mcd_dev->dev,
		       "operation not be allowed for status %d\n", status);
		return;
	}
	modem_ctrl_send_abnormal_to_ap(status);
}

void modem_ctrl_poweron_modem(int on)
{
	if (!mcd_dev || !mcd_dev->init)
		return;
	switch (on) {
	case MDM_CTRL_POWER_ON:
		if (!IS_ERR(mcd_dev->init->gpio_poweron)) {
			atomic_notifier_call_chain(&modem_ctrl_chain,
						   MDM_CTRL_POWER_ON, NULL);
			dev_info(mcd_dev->dev, "set modem_poweron: %d\n", on);
			gpiod_set_value_cansleep(mcd_dev->init->gpio_poweron,
						 1);
			/* Base the spec modem boot flow that need to wait 1s */
			msleep(REBOOT_MODEM_DELAY);
			mcd_dev->init->modem_status = MDM_CTRL_POWER_ON;
			gpiod_set_value_cansleep(mcd_dev->init->gpio_poweron,
						 0);
		}
		break;
	case MDM_CTRL_POWER_OFF:
		/*
		 *To do
		 */
		break;
	case MDM_CTRL_SET_CFG:
		/*
		 *To do
		 */
		break;
	case MDM_CTRL_WARM_RESET:
		if (!IS_ERR(mcd_dev->init->gpio_reset)) {
			atomic_notifier_call_chain(&modem_ctrl_chain,
						   MDM_CTRL_WARM_RESET, NULL);
			dev_dbg(mcd_dev->dev, "set warm reset: %d\n", on);
			gpiod_set_value_cansleep(mcd_dev->init->gpio_reset, 1);
			/* Base the spec modem that need to wait 50ms */
			msleep(RESET_MODEM_DELAY);
			mcd_dev->init->modem_status = MDM_CTRL_WARM_RESET;
			gpiod_set_value_cansleep(mcd_dev->init->gpio_reset, 0);
		}
		break;
	case MDM_CTRL_COLD_RESET:
		if (!IS_ERR(mcd_dev->init->gpio_poweron)) {
			mcd_dev->init->enable_cp_event = false;
			atomic_notifier_call_chain(&modem_ctrl_chain,
						   MDM_CTRL_COLD_RESET, NULL);
			dev_info(mcd_dev->dev, "modem_power reset: %d\n", on);
			gpiod_set_value_cansleep(mcd_dev->init->gpio_poweron,
						 1);
			/* Base the spec modem boot flow that need to wait 2s */
			msleep(POWERREST_MODEM_DELAY);
			mcd_dev->init->modem_status = MDM_CTRL_COLD_RESET;
			gpiod_set_value_cansleep(mcd_dev->init->gpio_poweron,
						 0);
		}
		break;
	case MDM_CTRL_PCIE_RECOVERY:
#ifdef CONFIG_PCIE_PM_NOTIFY
		pcie_ep_pm_notify(PCIE_EP_POWER_OFF);
		/* PCIE poweroff to poweron need 100ms*/
		msleep(100);
		pcie_ep_pm_notify(PCIE_EP_POWER_ON);
#endif
		break;
	case MDM_POWER_OFF:
		atomic_notifier_call_chain(&modem_ctrl_chain,
					   MDM_POWER_OFF, NULL);
		modem_ctrl_send_cmd_to_cp(MDM_POWER_OFF);
		break;
	default:
		dev_err(mcd_dev->dev, "cmd not support: %d\n", on);
	}
}
EXPORT_SYMBOL_GPL(modem_ctrl_poweron_modem);

#if defined(CONFIG_DEBUG_FS)
static int modem_ctrl_debug_show(struct seq_file *m, void *private)
{
	dev_dbg(mcd_dev->dev, "%s\n", __func__);
	return 0;
}

static int modem_ctrl_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, modem_ctrl_debug_show, inode->i_private);
}

static const struct file_operations modem_ctrl_debug_fops = {
	.open = modem_ctrl_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

#endif /* CONFIG_DEBUG_FS */

static int modem_ctrl_open(struct inode *inode, struct file *filp)
{
	struct modem_ctrl_device *modem_ctrl;

	modem_ctrl = container_of(inode->i_cdev,
				  struct modem_ctrl_device, cdev);
	filp->private_data = modem_ctrl;
	dev_dbg(modem_ctrl->dev, "modem_ctrl: %s\n", __func__);
	return 0;
}

static int modem_ctrl_release(struct inode *inode, struct file *filp)
{
	struct modem_ctrl_device *modem_ctrl;

	modem_ctrl = container_of(inode->i_cdev,
				  struct modem_ctrl_device, cdev);
	dev_dbg(modem_ctrl->dev, "modem_ctrl: %s\n", __func__);

	return 0;
}

static ssize_t modem_ctrl_read(struct file *filp,
			       char __user *buf,
			       size_t count,
			       loff_t *ppos)
{
	char tmpbuf[30];
	int r;
	struct modem_ctrl_device *mcd_dev  = filp->private_data;

	if (!mcd_dev || !mcd_dev->init)
		return -EINVAL;

	r = snprintf(tmpbuf, sizeof(tmpbuf), "%s\n",
		     mdm_stat[mcd_dev->init->modem_status]);

	return simple_read_from_buffer(buf, count, ppos, tmpbuf, r);
}

static ssize_t modem_ctrl_write(struct file *filp,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	char sbuf[100];
	int ret;
	u32 mcd_cmd;
	struct modem_ctrl_device *mcd_dev  = filp->private_data;

	if (!mcd_dev)
		return -EINVAL;

	if (unalign_copy_from_user((void *)sbuf, buf, count)) {
		dev_err(mcd_dev->dev, "copy buf %s error\n", buf);
		return -EFAULT;
	}
	dev_dbg(mcd_dev->dev, "get info:%s", sbuf);
	sbuf[count - 1] = '\0';
	ret = kstrtouint(sbuf, 10, &mcd_cmd);
	if (ret) {
		dev_err(mcd_dev->dev, "Invalid input!\n");
		return ret;
	}
	if (mcd_dev->soc_type == ROC1_SOC) {
		if (mcd_cmd >= MDM_CTRL_POWER_OFF &&
		    mcd_cmd <= MDM_CTRL_SET_CFG)
			modem_ctrl_poweron_modem(mcd_cmd);
		else
			dev_info(mcd_dev->dev, "cmd not support!\n");
	} else {
		modem_ctrl_notify_abnormal_status(mcd_cmd);
	}
	return count;
}

static long modem_ctrl_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg)
{

	if (!mcd_dev || mcd_dev->soc_type == ORCA_SOC)
		return -EINVAL;
	switch (cmd) {
	case MDM_CTRL_POWER_OFF:
		modem_ctrl_poweron_modem(MDM_CTRL_POWER_OFF);
		break;
	case MDM_CTRL_POWER_ON:
		modem_ctrl_poweron_modem(MDM_CTRL_POWER_ON);
		break;
	case MDM_CTRL_WARM_RESET:
		modem_ctrl_poweron_modem(MDM_CTRL_WARM_RESET);
		break;
	case MDM_CTRL_COLD_RESET:
		modem_ctrl_poweron_modem(MDM_CTRL_COLD_RESET);
		break;
	case MDM_CTRL_PCIE_RECOVERY:
		modem_ctrl_poweron_modem(MDM_CTRL_PCIE_RECOVERY);
		break;
	case MDM_CTRL_SET_CFG:
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations modem_ctrl_fops = {
	.open		= modem_ctrl_open,
	.release	= modem_ctrl_release,
	.read		= modem_ctrl_read,
	.write		= modem_ctrl_write,
	.unlocked_ioctl	= modem_ctrl_ioctl,
	.owner		= THIS_MODULE,
	.llseek		= default_llseek,
};

static int modem_ctrl_parse_modem_dt(struct modem_ctrl_init_data **init,
			       struct device *dev)
{
	struct modem_ctrl_init_data *pdata = NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;
	pdata->name = cdev_name;

	/* Triger watchdog,assert,panic of orca */
	pdata->gpio_cpwatchdog = devm_gpiod_get(dev,
					       "cpwatchdog",
					       GPIOD_OUT_HIGH);
	if (IS_ERR(pdata->gpio_cpwatchdog))
		return PTR_ERR(pdata->gpio_cpwatchdog);

	pdata->gpio_cpassert = devm_gpiod_get(dev, "cpassert", GPIOD_OUT_HIGH);
	if (IS_ERR(pdata->gpio_cpassert))
		return PTR_ERR(pdata->gpio_cpassert);

	pdata->gpio_cppanic = devm_gpiod_get(dev, "cppanic", GPIOD_OUT_HIGH);
	if (IS_ERR(pdata->gpio_cppanic))
		return PTR_ERR(pdata->gpio_cppanic);

	pdata->gpio_cppoweroff = devm_gpiod_get(dev, "cppoweroff", GPIOD_IN);
	if (IS_ERR(pdata->gpio_cpassert))
		return PTR_ERR(pdata->gpio_cppoweroff);

	*init = pdata;
	return 0;
}

static int modem_ctrl_parse_dt(struct modem_ctrl_init_data **init,
			       struct device *dev)
{
	struct modem_ctrl_init_data *pdata;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;
	pdata->name = cdev_name;
	pdata->gpio_poweron = devm_gpiod_get(dev, "poweron", GPIOD_OUT_LOW);
	if (IS_ERR(pdata->gpio_poweron))
		return PTR_ERR(pdata->gpio_poweron);

	pdata->gpio_reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(pdata->gpio_reset))
		return PTR_ERR(pdata->gpio_reset);

	/* Triger watchdog,assert,panic of orca */
	pdata->gpio_cpwatchdog = devm_gpiod_get(dev, "cpwatchdog", GPIOD_IN);
	if (IS_ERR(pdata->gpio_cpwatchdog))
		return PTR_ERR(pdata->gpio_cpwatchdog);

	pdata->gpio_cpassert = devm_gpiod_get(dev, "cpassert", GPIOD_IN);
	if (IS_ERR(pdata->gpio_cpassert))
		return PTR_ERR(pdata->gpio_cpassert);

	pdata->gpio_cppanic = devm_gpiod_get(dev, "cppanic", GPIOD_IN);
	if (IS_ERR(pdata->gpio_cppanic))
		return PTR_ERR(pdata->gpio_cppanic);

	pdata->gpio_cppoweroff = devm_gpiod_get(dev,
						"cppoweroff", GPIOD_OUT_HIGH);
	if (IS_ERR(pdata->gpio_cpassert))
		return PTR_ERR(pdata->gpio_cppoweroff);

	pdata->modem_status = MDM_CTRL_POWER_OFF;
	*init = pdata;
	return 0;
}

static inline void
modem_ctrl_destroy_pdata(struct modem_ctrl_init_data **init)
{
	struct modem_ctrl_init_data *pdata = *init;

	pdata = NULL;
}

static int modem_ctrl_restart_handle(struct notifier_block *this,
				     unsigned long mode, void *cmd)
{
	if (!mcd_dev || mcd_dev->soc_type == ROC1_SOC)
		return NOTIFY_DONE;
	modem_ctrl_notify_abnormal_status(MDM_PANIC);
	while (1)
		;
	return NOTIFY_DONE;
}

static struct notifier_block modem_ctrl_restart_handler = {
	.notifier_call = modem_ctrl_restart_handle,
	.priority = 150,
};

static int modem_ctrl_probe(struct platform_device *pdev)
{
	struct modem_ctrl_init_data *init = pdev->dev.platform_data;
	struct modem_ctrl_device *modem_ctrl_dev;
	dev_t devid;
	int rval;
	struct device *dev = &pdev->dev;

	modem_ctrl_dev = devm_kzalloc(dev, sizeof(*modem_ctrl_dev), GFP_KERNEL);
	if (!modem_ctrl_dev)
		return -ENOMEM;
	mcd_dev = modem_ctrl_dev;
	if (of_device_is_compatible(pdev->dev.of_node, "sprd,roc1-modem-ctrl"))
		modem_ctrl_dev->soc_type = ROC1_SOC;
	else
		modem_ctrl_dev->soc_type = ORCA_SOC;

	if (modem_ctrl_dev->soc_type == ROC1_SOC) {
		rval = modem_ctrl_parse_dt(&init, &pdev->dev);
		if (rval) {
			dev_err(dev,
				"Failed to parse modem_ctrl device tree, ret=%d\n",
				rval);
			return rval;
		}
	} else {
		rval = modem_ctrl_parse_modem_dt(&init, &pdev->dev);
		if (rval) {
			dev_err(dev,
				"Failed to parse modem_ctrl device tree, ret=%d\n",
				rval);
			return rval;
		}
	}

	dev_dbg(dev, "after parse device tree, name=%s soctype=%d\n",
		init->name,
		modem_ctrl_dev->soc_type);

	rval = alloc_chrdev_region(&devid, 0, 1, init->name);
	if (rval != 0) {
		dev_err(dev, "Failed to alloc modem_ctrl chrdev\n");
		goto  error3;
	}
	cdev_init(&modem_ctrl_dev->cdev, &modem_ctrl_fops);
	rval = cdev_add(&modem_ctrl_dev->cdev, devid, 1);
	if (rval != 0) {
		dev_err(dev, "Failed to add modem_ctrl cdev\n");
		goto error2;
	}

	modem_ctrl_dev->major = MAJOR(devid);
	modem_ctrl_dev->minor = MINOR(devid);
	modem_ctrl_dev->dev = device_create(modem_ctrl_class, NULL,
					    MKDEV(modem_ctrl_dev->major,
						  modem_ctrl_dev->minor),
					    NULL, "%s", init->name);
	if (!modem_ctrl_dev->dev) {
		dev_err(dev, "create dev failed\n");
		rval = -ENODEV;
		goto error1;
	}
	modem_ctrl_dev->init = init;
	platform_set_drvdata(pdev, modem_ctrl_dev);
	rval = modem_gpios_init(modem_ctrl_dev, modem_ctrl_dev->soc_type);
	if (rval) {
		dev_err(dev, "request gpios error\n");
		goto error0;
	}

	rval = register_restart_handler(&modem_ctrl_restart_handler);
	if (rval) {
		dev_err(dev, "cannot register restart handler err=%d\n", rval);
		goto error0;
	}
	return 0;
error0:
	device_destroy(modem_ctrl_class,
		       MKDEV(modem_ctrl_dev->major,
			     modem_ctrl_dev->minor));
error1:
	cdev_del(&modem_ctrl_dev->cdev);
error2:
	unregister_chrdev_region(devid, 1);
error3:
	modem_ctrl_destroy_pdata(&init);
	return rval;
}

static int modem_ctrl_remove(struct platform_device *pdev)
{
	struct modem_ctrl_device *modem_ctrl_dev = platform_get_drvdata(pdev);

	unregister_reboot_notifier(&modem_ctrl_restart_handler);
	device_destroy(modem_ctrl_class,
		       MKDEV(modem_ctrl_dev->major,
			     modem_ctrl_dev->minor));
	cdev_del(&modem_ctrl_dev->cdev);
	unregister_chrdev_region(MKDEV(modem_ctrl_dev->major,
				       modem_ctrl_dev->minor), 1);
	modem_ctrl_destroy_pdata(&modem_ctrl_dev->init);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void modem_ctrl_shutdown(struct platform_device *pdev)
{
	if (mcd_dev->soc_type == ROC1_SOC) {
		atomic_notifier_call_chain(&modem_ctrl_chain,
					   MDM_POWER_OFF, NULL);
		/*
		 * sleep 50 ms for other module to do something
		 * before orca power down.
		 */
		msleep(50);
		modem_ctrl_send_cmd_to_cp(MDM_POWER_OFF);
		/* Sleep 500ms for cp to deal power down process otherwise
		 * cp will not power down clearly.
		 */
		msleep(500);
	}
}

static const struct of_device_id modem_ctrl_match_table[] = {
	{.compatible = "sprd,roc1-modem-ctrl", },
	{.compatible = "sprd,orca-modem-ctrl", },
};

static struct platform_driver modem_ctrl_driver = {
	.driver = {
		.name = "modem_ctrl",
		.of_match_table = modem_ctrl_match_table,
	},
	.probe = modem_ctrl_probe,
	.remove = modem_ctrl_remove,
	.shutdown = modem_ctrl_shutdown,
};

int modem_ctrl_init(void)
{
	modem_ctrl_class = class_create(THIS_MODULE, "modem_ctrl");
	if (IS_ERR(modem_ctrl_class))
		return PTR_ERR(modem_ctrl_class);
	return platform_driver_register(&modem_ctrl_driver);
}
EXPORT_SYMBOL_GPL(modem_ctrl_init);

void modem_ctrl_exit(void)
{
	class_destroy(modem_ctrl_class);
	platform_driver_unregister(&modem_ctrl_driver);
}
EXPORT_SYMBOL_GPL(modem_ctrl_exit);
