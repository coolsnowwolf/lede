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

#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/sizes.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>

#include "../include/sipc.h"
#include "sipc_priv.h"

#define MBOX_BAMK	"mbox"
#define PCIE_BAMK	"pcie"

enum {
	NORMAL_MODE = 0,
	CHARGE_MODE,
	CALI_MODE
};

#define CALI_LATENCY	(10000 * 1000)
#define NORMAL_LATENCY	(1 * 1000)

/*
 * In charge mode, will only boot pm system,
 * so just create pm systen sipc.
 */
static u8 g_boot_mode = NORMAL_MODE;
/*
static int __init sipc_early_mode(char *str)
{
	if (!memcmp(str, "charger", 7))
		g_boot_mode = CHARGE_MODE;
	else if (!memcmp(str, "cali", 4))
		g_boot_mode = CALI_MODE;
	else
		g_boot_mode = NORMAL_MODE;

	return 0;
}

early_param("androidboot.mode", sipc_early_mode);
*/

#if defined(CONFIG_DEBUG_FS)
void sipc_debug_putline(struct seq_file *m, char c, int n)
{
	char buf[300];
	int i, max, len;

	/* buf will end with '\n' and 0 */
	max = ARRAY_SIZE(buf) - 2;
	len = (n > max) ? max : n;

	for (i = 0; i < len; i++)
		buf[i] = c;

	buf[i] = '\n';
	buf[i + 1] = 0;

	seq_puts(m, buf);
}
EXPORT_SYMBOL_GPL(sipc_debug_putline);
#endif

static u32 sipc_rxirq_status(u8 dst)
{
	return 0;
}

static void sipc_rxirq_clear(u8 dst)
{

}

static void sipc_txirq_trigger(u8 dst, u64 msg)
{
	struct smsg_ipc *ipc;

	ipc = smsg_ipcs[dst];

	if (ipc) {
#ifdef CONFIG_SPRD_MAILBOX
		if (ipc->type == SIPC_BASE_MBOX) {
			mbox_raw_sent(ipc->core_id, msg);
			return;
		}
#endif

		if (ipc->type == SIPC_BASE_PCIE) {
#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
			sprd_ep_dev_raise_irq(ipc->ep_dev, PCIE_DBELL_SIPC_IRQ);
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
			sprd_pci_epf_raise_irq(ipc->ep_fun, PCIE_MSI_SIPC_IRQ);
#endif
			return;
		}
	}
}

#ifdef SPRD_PCIE_USE_DTS
static int sipc_parse_dt(struct smsg_ipc *ipc,
	struct device_node *np, struct device *dev)
{
	u32 val[3];
	int ret;
	const char *type;

	/* get name */
	ret = of_property_read_string(np, "sprd,name", &ipc->name);
	if (ret)
		return ret;

	pr_info("sipc: name=%s\n", ipc->name);

	/* get sipc type, optional */
	if (of_property_read_string(np, "sprd,type", &type) == 0) {
		pr_info("sipc: type=%s\n", type);
		if (strcmp(MBOX_BAMK, type) == 0)
			ipc->type = SIPC_BASE_MBOX;
		else if (strcmp(PCIE_BAMK, type) == 0)
			ipc->type = SIPC_BASE_PCIE;
	}

	/* get sipc client, optional */
	if (of_property_read_u32_array(np, "sprd,client", val, 1) == 0) {
		ipc->client = (u8)val[0];
		pr_info("sipc: client=%d\n", ipc->client);
	}

	/* get sipc dst */
	ret = of_property_read_u32_array(np, "sprd,dst", val, 1);
	if (!ret) {
		ipc->dst = (u8)val[0];
		pr_info("sipc: dst =%d\n", ipc->dst);
	}

	if (ret || ipc->dst >= SIPC_ID_NR) {
		pr_err("sipc: dst err, ret =%d.\n", ret);
		return ret;
	}

#ifdef CONFIG_SPRD_MAILBOX
	if (ipc->type == SIPC_BASE_MBOX) {
		/* get core id */
		ipc->core_id = (u8)MBOX_INVALID_CORE;
		ret = of_property_read_u32_array(np, "sprd,core", val, 1);
		if (!ret) {
			ipc->core_id = (u8)val[0];
			pr_info("sipc: core=%d\n", ipc->core_id);
		} else {
			pr_err("sipc: core err, ret =%d.\n", ret);
			return ret;
		}

		/* get core sensor id, optional*/
		ipc->core_sensor_id = (u8)MBOX_INVALID_CORE;
		if (of_property_read_u32_array(np, "sprd,core_sensor",
					       val, 1) == 0) {
			ipc->core_sensor_id = (u8)val[0];
			pr_info("sipc: core_sensor=%d\n", ipc->core_sensor_id);
		}
	}
#endif

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	if (ipc->type == SIPC_BASE_PCIE) {
#ifdef CONFIG_SPRD_PCIE
		struct device_node *pdev_node;
#endif

		ret = of_property_read_u32_array(np,
						 "sprd,ep-dev",
						 &ipc->ep_dev,
						 1);
		pr_info("sipc: ep_dev=%d\n", ipc->ep_dev);
		if (ret || ipc->ep_dev >= PCIE_EP_NR) {
			pr_err("sipc: ep_dev err, ret =%d.\n", ret);
			return ret;
		}

#ifdef CONFIG_SPRD_PCIE
		/* get pcie rc ctrl device */
		pdev_node = of_parse_phandle(np, "sprd,rc-ctrl", 0);
		if (!pdev_node) {
			pr_err("sipc: sprd,rc-ctrl err.\n");
			return -ENODEV;
		}
		ipc->pcie_dev = of_find_device_by_node(pdev_node);
		of_node_put(pdev_node);

		if (!ipc->pcie_dev) {
			pr_err("sipc: find pcie_dev err.\n");
			return -ENODEV;
		}
#endif
	}
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
	if (ipc->type == SIPC_BASE_PCIE) {
		ret = of_property_read_u32_array(np,
						"sprd,ep-fun",
						&ipc->ep_fun,
						1);
		pr_info("sipc: ep_fun=%d\n", ipc->ep_fun);
		if (ret || ipc->ep_fun >= SPRD_FUNCTION_MAX) {
			pr_err("sipc: ep_fun err, ret =%d.\n", ret);
			return ret;
		}

		/* parse doolbell irq */
		ret = of_irq_get(np, 0);
		if (ret < 0) {
			pr_err("sipc: doorbell irq err, ret=%d\n", ret);
			return -EINVAL;
		}
		ipc->irq = ret;
		pr_info("sipc: irq=%d\n", ipc->irq);
	}
#endif

	/* get smem type */
	ret = of_property_read_u32_array(np,
					 "sprd,smem-type",
					 &val[0],
					 1);
	if (!ret)
		ipc->smem_type = (enum smem_type)val[0];
	else
		ipc->smem_type = SMEM_LOCAL;

	pr_info("sipc: smem_type = %d, ret =%d\n", ipc->smem_type, ret);

	/* get smem info */
	ret = of_property_read_u32_array(np,
					 "sprd,smem-info",
					 val,
					 3);
	if (ret) {
		pr_err("sipc: parse smem info failed.\n");
		return ret;
	}
	ipc->smem_base = val[0];
	ipc->dst_smem_base = val[1];
	ipc->smem_size = val[2];
	pr_info("sipc: smem_base=0x%x, dst_smem_base=0x%x, smem_size=0x%x\n",
		ipc->smem_base, ipc->dst_smem_base, ipc->smem_size);

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	/* try to get high_offset */
	ret = of_property_read_u32(np,
					 "sprd,high-offset",
					 val);
	if (!ret) {
		ipc->high_offset = val[0];
		pr_info("sipc: high_offset=0x%xn", ipc->high_offset);
	}
#endif

	if (ipc->type == SIPC_BASE_PCIE) {
		/* pcie sipc, the host must use loacal SMEM_LOCAL */
		if (!ipc->client && ipc->smem_type != SMEM_LOCAL) {
			pr_err("sipc: host must use local smem!");
			return -EINVAL;
		}

		if (ipc->client && ipc->smem_type != SMEM_PCIE) {
			pr_err("sipc: client must use pcie smem!");
			return -EINVAL;
		}
	}

	return 0;
}
#else
static u32 sipc_get_smem_base(size_t size)
{
	unsigned long order = get_order(size);
	struct page *page, *p, *e;
	
	page = alloc_pages(GFP_KERNEL, order);
	if(page == NULL) {
		printk("sipc alloc pages fail\n");
		return 0;
	}
	split_page(page, order);
	for (p = page +(size >> PAGE_SHIFT), e = page + (1 << order); p < e; p++)
		__free_page(p);
	
	if (PageHighMem(page)) {
		phys_addr_t base = __pfn_to_phys(page_to_pfn(page));
		//phys_addr_t end = base + size;
		while (size > 0) {
			void *ptr = kmap_atomic(page);
		        memset(ptr, 0, PAGE_SIZE);
			kunmap_atomic(ptr);
			page++;
			size -= PAGE_SIZE;
		}
		
		return base;
	} else {
		void *ptr = page_address(page);
		memset(ptr, 0, size);	
		return __pa(ptr);
	}
}

static int sipc_parse_dt(struct smsg_ipc *ipc,
	struct device_node *np, struct device *dev)
{
	u32 val[3];
	int ret = 0;
    //dma_addr_t *dma_handle;

	/* get name */
        ipc->name = "sprd,sipc";
	pr_info("sipc: name=%s\n", ipc->name);

	/* get sipc type, optional */
	ipc->type = SIPC_BASE_PCIE;
	pr_info("sipc: type=%d\n", ipc->type);

	/* get sipc client, optional */

	/* get sipc dst */
	ipc->dst = 1;
	pr_info("sipc: dst =%d\n", ipc->dst);

	if (ipc->dst >= SIPC_ID_NR) {
		pr_err("sipc: dst err\n");
		return ret;
	}

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	if (ipc->type == SIPC_BASE_PCIE) {
                ipc->ep_dev = 0;
		pr_info("sipc: ep_dev=%d\n", ipc->ep_dev);
		if (ipc->ep_dev >= PCIE_EP_NR) {
			pr_err("sipc: ep_dev err\n");
			return -1;
		}

	}
#endif
	/* get smem type */
	ipc->smem_type = SMEM_LOCAL;

	pr_info("sipc: smem_type = %d\n", ipc->smem_type);

	/* get smem info */
    val[0] = sipc_get_smem_base(0x0300000);
    val[1] = val[0];
    val[2] = 0x0300000;
	ipc->smem_base = val[0];
	ipc->dst_smem_base = val[1];
	ipc->smem_size = val[2];
	pr_info("sipc: smem_base=0x%x, dst_smem_base=0x%x, smem_size=0x%x\n",
		ipc->smem_base, ipc->dst_smem_base, ipc->smem_size);

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	/* try to get high_offset */
	ipc->high_offset = 0;
	pr_info("sipc: high_offset=0x%xn", ipc->high_offset);

#endif

	if (ipc->type == SIPC_BASE_PCIE) {
		/* pcie sipc, the host must use loacal SMEM_LOCAL */
		if (!ipc->client && ipc->smem_type != SMEM_LOCAL) {
			pr_err("sipc: host must use local smem!");
			return -EINVAL;
		}

		if (ipc->client && ipc->smem_type != SMEM_PCIE) {
			pr_err("sipc: client must use pcie smem!");
			return -EINVAL;
		}
	}

	return 0;
}
#endif

static int sipc_probe(struct platform_device *pdev)
{
	struct smsg_ipc *ipc;
	struct device_node *np;

	if (1) {
		np = pdev->dev.of_node;
		ipc = devm_kzalloc(&pdev->dev,
			sizeof(struct smsg_ipc),
			GFP_KERNEL);
		if (!ipc)
			return -ENOMEM;

		if (sipc_parse_dt(ipc, np, &pdev->dev)) {
			pr_err("%s: failed to parse dt!\n", __func__);
			return -ENODEV;
		}

		/*
		 * In charge mode, will only boot pm system,
		 * so just create pm systen sipc.
		 */
		if (g_boot_mode == CHARGE_MODE && ipc->dst != SIPC_ID_PM_SYS)
			return -ENODEV;

		ipc->rxirq_status = sipc_rxirq_status;
		ipc->rxirq_clear = sipc_rxirq_clear;
		ipc->txirq_trigger = sipc_txirq_trigger;
		spin_lock_init(&ipc->txpinlock);

		if (ipc->type == SIPC_BASE_PCIE) {
			/* init mpm delay enter idle time for pcie. */
			if (g_boot_mode == CALI_MODE)
				ipc->latency = CALI_LATENCY;
			else
				ipc->latency = NORMAL_LATENCY;
		}

		smsg_ipc_create(ipc);
		platform_set_drvdata(pdev, ipc);
	}
	return 0;
}

static int sipc_remove(struct platform_device *pdev)
{
	struct smsg_ipc *ipc = platform_get_drvdata(pdev);

	smsg_ipc_destroy(ipc);

	devm_kfree(&pdev->dev, ipc);
	return 0;
}

#ifdef  SPRD_PCIE_USE_DTS
static const struct of_device_id sipc_match_table[] = {
	{ .compatible = "sprd,sipc", },
	{ },
};
#endif

static struct platform_driver sipc_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "sipc",
#ifdef  SPRD_PCIE_USE_DTS
		.of_match_table = sipc_match_table,
#endif
	},
	.probe = sipc_probe,
	.remove = sipc_remove,
};

#ifndef SPRD_PCIE_USE_DTS
static void sipc_platform_device_release(struct device *dev) {}
static struct platform_device sipc_device = {
    	.name        = "sipc",
    	.id        = -1,
		.dev = {
			.release = sipc_platform_device_release,
		}
};
#endif

int sipc_init(void)
{
	int ret;

	smsg_init_channel2index();
#ifndef SPRD_PCIE_USE_DTS
        if((ret = platform_device_register(&sipc_device)))
		return ret;
#endif

	if((ret  = platform_driver_register(&sipc_driver))) {
#ifndef SPRD_PCIE_USE_DTS
		platform_device_unregister(&sipc_device);
#endif
		return ret;
        }

	return ret;
}
EXPORT_SYMBOL_GPL(sipc_init);

void sipc_exit(void)
{
	platform_driver_unregister(&sipc_driver);
                printk("dayin is here0\n");
#ifndef SPRD_PCIE_USE_DTS
	platform_device_unregister(&sipc_device);
#endif
}
EXPORT_SYMBOL_GPL(sipc_exit);