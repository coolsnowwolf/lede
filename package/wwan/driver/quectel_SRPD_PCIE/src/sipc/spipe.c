/*
 * Copyright (C) 2018 Spreadtrum Communications Inc.
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

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "../include/sipc.h"
#include "sipc_priv.h"
#include "spipe.h"

#define SPIPE_NR_BASE_NUM	MKDEV(254, 0)
#define SDIAG_NR_BASE_NUM	MKDEV(253, 0)
#define STTY_NR_BASE_NUM	MKDEV(252, 0)
#define SNV_NR_BASE_NUM		MKDEV(251, 0)

struct spipe_device {
	struct spipe_init_data	*init;
	int			major;
	int			minor;
	struct cdev		cdev;
};

struct spipe_sbuf {
	u8			dst;
	u8			channel;
	u32		bufid;
};

static struct class		*spipe_class;

static int spipe_open(struct inode *inode, struct file *filp)
{
	int minor = iminor(filp->f_path.dentry->d_inode);
	struct spipe_device *spipe;
	struct spipe_sbuf *sbuf;

	spipe = container_of(inode->i_cdev, struct spipe_device, cdev);
	if (sbuf_status(spipe->init->dst, spipe->init->channel) != 0) {
		printk("spipe %d-%d not ready to open!\n",
			spipe->init->dst, spipe->init->channel);
		filp->private_data = NULL;
		return -ENODEV;
	}

	sbuf = kmalloc(sizeof(struct spipe_sbuf), GFP_KERNEL);
	if (!sbuf)
		return -ENOMEM;
	filp->private_data = sbuf;

	sbuf->dst = spipe->init->dst;
	sbuf->channel = spipe->init->channel;
	sbuf->bufid = minor - spipe->minor;

	return 0;
}

static int spipe_release(struct inode *inode, struct file *filp)
{
	struct spipe_sbuf *sbuf = filp->private_data;

	kfree(sbuf);

	return 0;
}

static ssize_t spipe_read(struct file *filp,
		char __user *buf, size_t count, loff_t *ppos)
{
	struct spipe_sbuf *sbuf = filp->private_data;
	int timeout = -1;

	if (filp->f_flags & O_NONBLOCK)
		timeout = 0;

	return sbuf_read(sbuf->dst, sbuf->channel, sbuf->bufid,
			(void *)buf, count, timeout);
}

static ssize_t spipe_write(struct file *filp,
		const char __user *buf, size_t count, loff_t *ppos)
{
	struct spipe_sbuf *sbuf = filp->private_data;
	int timeout = -1;

	if (filp->f_flags & O_NONBLOCK)
		timeout = 0;

	return sbuf_write(sbuf->dst, sbuf->channel, sbuf->bufid,
			(void *)buf, count, timeout);
}

static unsigned int spipe_poll(struct file *filp, poll_table *wait)
{
	struct spipe_sbuf *sbuf = filp->private_data;

	return sbuf_poll_wait(sbuf->dst, sbuf->channel, sbuf->bufid,
			filp, wait);
}

static long spipe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static const struct file_operations spipe_fops = {
	.open		= spipe_open,
	.release	= spipe_release,
	.read		= spipe_read,
	.write		= spipe_write,
	.poll		= spipe_poll,
	.unlocked_ioctl	= spipe_ioctl,
	.owner		= THIS_MODULE,
	.llseek		= default_llseek,
};

#ifdef SPRD_PCIE_USE_DTS
static int spipe_parse_dt(struct spipe_init_data **init,
	struct device_node *np, struct device *dev, dev_t *devid)
{
	struct spipe_init_data *pdata = NULL;
	int ret;
	u32 data;

	pdata = devm_kzalloc(dev, sizeof(struct spipe_init_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	ret = of_property_read_string(np,
				      "sprd,name",
				      (const char **)&pdata->name);
	if (ret)
		goto error;

	if (!strcmp(pdata->name, "spipe_nr"))
		*devid = SPIPE_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "sdiag_nr"))
		*devid = SDIAG_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "stty_nr"))
		*devid = STTY_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "snv_nr"))
		*devid = SNV_NR_BASE_NUM;

	ret = of_property_read_u32(np, "sprd,dst", (u32 *)&data);
	if (ret)
		goto error;
	pdata->dst = (u8)data;

	ret = of_property_read_u32(np, "sprd,channel", (u32 *)&data);
	if (ret)
		goto error;
	pdata->channel = (u8)data;

	ret = of_property_read_u32(np,
				   "sprd,ringnr",
				   (u32 *)&pdata->ringnr);
	if (ret)
		goto error;

	ret = of_property_read_u32(np,
				   "sprd,size-rxbuf",
				   (u32 *)&pdata->rxbuf_size);
	if (ret)
		goto error;

	ret = of_property_read_u32(np,
				   "sprd,size-txbuf",
				   (u32 *)&pdata->txbuf_size);
	if (ret)
		goto error;

	*init = pdata;
	return ret;
error:
	devm_kfree(dev, pdata);
	*init = NULL;
	return ret;
}
#else
static int spipe_parse_dt(struct spipe_init_data **init,
	struct device_node *np, struct device *dev, dev_t *devid)
{
	struct spipe_init_data *pdata = *init;

	if (!strcmp(pdata->name, "spipe_nr"))
		*devid = SPIPE_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "sdiag_nr"))
		*devid = SDIAG_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "stty_nr"))
		*devid = STTY_NR_BASE_NUM;
	else if (!strcmp(pdata->name, "snv_nr"))
		*devid = SNV_NR_BASE_NUM;

	return 0;
}
#endif

static inline void spipe_destroy_pdata(struct spipe_init_data **init,
	struct device *dev)
{
	*init = NULL;
}

static int spipe_probe(struct platform_device *pdev)
{
	struct spipe_init_data *init = pdev->dev.platform_data;
	struct spipe_device *spipe;
	dev_t devid;
	int i, rval;
	struct device_node *np;

	printk("%s!\n", __func__);

	if (1) {
		np = pdev->dev.of_node;
		rval = spipe_parse_dt(&init, np, &pdev->dev, &devid);
		if (rval) {
			pr_err("Failed to parse spipe device tree, ret=%d\n", rval);
			return rval;
		}

		printk("spipe: after parse device tree, name=%s, dst=%u, channel=%u, ringnr=%u,  rxbuf_size=0x%x, txbuf_size=0x%x\n",
			init->name,
			init->dst,
			init->channel,
			init->ringnr,
			init->rxbuf_size,
			init->txbuf_size);

		rval = sbuf_create(init->dst, init->channel, init->ringnr,
				   init->txbuf_size, init->rxbuf_size);
		if (rval != 0) {
			printk("Failed to create sbuf: %d\n", rval);
			spipe_destroy_pdata(&init, &pdev->dev);
			return rval;
		}

		spipe = devm_kzalloc(&pdev->dev,
				     sizeof(struct spipe_device),
				     GFP_KERNEL);
		if (spipe == NULL) {
			sbuf_destroy(init->dst, init->channel);
			spipe_destroy_pdata(&init, &pdev->dev);
			printk("Failed to allocate spipe_device\n");
			return -ENOMEM;
		}

		rval = alloc_chrdev_region(&devid, 0, init->ringnr, init->name);
		//rval = register_chrdev_region(devid, init->ringnr, init->name);
		if (rval != 0) {
			sbuf_destroy(init->dst, init->channel);
			devm_kfree(&pdev->dev, spipe);
			spipe_destroy_pdata(&init, &pdev->dev);
			printk("Failed to alloc spipe chrdev\n");
			return rval;
		}

		cdev_init(&(spipe->cdev), &spipe_fops);
		rval = cdev_add(&(spipe->cdev), devid, init->ringnr);
		if (rval != 0) {
			sbuf_destroy(init->dst, init->channel);
			devm_kfree(&pdev->dev, spipe);
			unregister_chrdev_region(devid, init->ringnr);
			spipe_destroy_pdata(&init, &pdev->dev);
			printk("Failed to add spipe cdev\n");
			return rval;
		}

		spipe->major = MAJOR(devid);
		spipe->minor = MINOR(devid);
		if (init->ringnr > 1) {
			for (i = 0; i < init->ringnr; i++) {
				device_create(spipe_class, NULL,
					MKDEV(spipe->major, spipe->minor + i),
					NULL, "%s%d", init->name, i);
			}
		} else {
			device_create(spipe_class, NULL,
				MKDEV(spipe->major, spipe->minor),
				NULL, "%s", init->name);
		}

		spipe->init = init;

		platform_set_drvdata(pdev, spipe);
	}

	return 0;
}

static int  spipe_remove(struct platform_device *pdev)
{
	struct spipe_device *spipe = platform_get_drvdata(pdev);
	int i;

	if (spipe) {
		for (i = 0; i < spipe->init->ringnr; i++) {
			device_destroy(spipe_class, MKDEV(spipe->major, spipe->minor + i));
		}
		cdev_del(&(spipe->cdev));
		unregister_chrdev_region(MKDEV(spipe->major, spipe->minor), spipe->init->ringnr);

		sbuf_destroy(spipe->init->dst, spipe->init->channel);

		spipe_destroy_pdata(&spipe->init, &pdev->dev);

		devm_kfree(&pdev->dev, spipe);

		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

#ifdef SPRD_PCIE_USE_DTS
static const struct of_device_id spipe_match_table[] = {
	{.compatible = "sprd,spipe", },
	{ },
};
#endif

static struct platform_driver spipe_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "spipe",
#ifdef SPRD_PCIE_USE_DTS
		.of_match_table = spipe_match_table,
#endif
	},
	.probe = spipe_probe,
	.remove = spipe_remove,
};

static struct platform_device *spipe_pdev[MAX_SPIPE_CHN_NUM];
static struct spipe_init_data spipe_data[MAX_SPIPE_CHN_NUM] = {
	{
		.sipc_name = SPIPE_DRVIER_NMAE,
		.name = "spipe_nr",
		.dst = 1,
                .channel = 4,
		.ringnr = 15,
                .txbuf_size = 0x1000,
		.rxbuf_size = 0x1000
        },
	{
		.sipc_name = SPIPE_DRVIER_NMAE,
		.name = "sdiag_nr",
		.dst = 1,
                .channel = 21,
		.ringnr = 1,
                .txbuf_size = 0x40000,
		.rxbuf_size = 0x8000
          },
	{
		.sipc_name = SPIPE_DRVIER_NMAE,
		.name = "stty_nr",
		.dst = 1,
                .channel = 6,
		.ringnr = 32,
                 .txbuf_size = 0x0800,
		.rxbuf_size = 0x0800
        },
	{
		.sipc_name = SPIPE_DRVIER_NMAE,
		.name = "snv_nr",
		.dst = 1,
                .channel = 40,
		.ringnr = 1,
                .txbuf_size = 0x40400,
		.rxbuf_size = 0x1000
        }

};

static int spipe_platform_device_reigster(void) 
{
	int retval = -ENOMEM;
	int i;

        for(i = 0; i < MAX_SPIPE_CHN_NUM; i++) {
		spipe_pdev[i] = platform_device_alloc(SPIPE_DRVIER_NMAE, i);
		if (!spipe_pdev[i]) {
			i--;
			while (i >= 0)
				platform_device_put(spipe_pdev[i--]);
			return retval;
		}
        }

	for (i = 0; i < MAX_SPIPE_CHN_NUM; i++) {
		retval = platform_device_add_data(spipe_pdev[i], &spipe_data[i],
				sizeof(struct spipe_init_data));
		if (retval)
			goto err_add_pdata;
	}

	for (i = 0; i < MAX_SPIPE_CHN_NUM; i++) {
		retval = platform_device_add(spipe_pdev[i]);
		if (retval < 0) {
			i--;
			while (i >= 0)
				platform_device_del(spipe_pdev[i]);
			goto err_add_pdata;
		}
	}

        return retval;

err_add_pdata:
	for (i = 0; i < MAX_SPIPE_CHN_NUM; i++)
		platform_device_put(spipe_pdev[i]);
	return retval;             
}

static void spipe_platform_device_unreigster(void) 
{
	int i;
	for (i = 0; i < MAX_SPIPE_CHN_NUM; i++) {
		platform_device_unregister(spipe_pdev[i]);
	}
}


int spipe_init(void)
{
	int ret;

	spipe_class = class_create(THIS_MODULE, "spipe");
	if (IS_ERR(spipe_class))
		return PTR_ERR(spipe_class);
#ifndef SPRD_PCIE_USE_DTS
	if((ret = spipe_platform_device_reigster()))
		return ret;
#endif
      
	if((ret = platform_driver_register(&spipe_driver))) {
#ifndef SPRD_PCIE_USE_DTS
		spipe_platform_device_unreigster();
#endif
		return ret;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(spipe_init);

void spipe_exit(void)
{
	platform_driver_unregister(&spipe_driver);
#ifndef SPRD_PCIE_USE_DTS
	spipe_platform_device_unreigster();
#endif
	class_destroy(spipe_class);
}

void spipe_device_down(void) 
{
	int retval = -ENOMEM;
	int i;

    for(i = 0; i < MAX_SPIPE_CHN_NUM; i++) {
        sbuf_down(spipe_data[i].dst, spipe_data[i].channel);
	}
}

EXPORT_SYMBOL_GPL(spipe_exit);
EXPORT_SYMBOL_GPL(spipe_device_down);
