/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note.
 *
 * UNISOC 'virt sipa' driver
 *
 * Qingsheng.Li <qingsheng.li@unisoc.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/mfd/syscon.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/pm_wakeup.h>
#include <linux/pm_runtime.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "../include/sipa.h"
#include "../include/sprd_pcie_ep_device.h"
#include "../include/sipc.h"
#include "sipa_core.h"

#define DRV_NAME "virt_sipa"

struct sipa_core *sipa_ctrl;

struct sipa_core *sipa_get_ctrl_pointer(void)
{
	return sipa_ctrl;
}
EXPORT_SYMBOL(sipa_get_ctrl_pointer);

static void sipa_notify_sender_flow_ctrl(struct work_struct *work)
{
	struct sipa_core *sipa_ctrl = container_of(work, struct sipa_core,
						   flow_ctrl_work);

	if (sipa_ctrl->sender && sipa_ctrl->sender->free_notify_net)
		wake_up(&sipa_ctrl->sender->free_waitq);
}

static int sipa_init_cmn_fifo(struct sipa_core *ipa,
			      enum sipa_cmn_fifo_index id)
{
	size_t size;
	dma_addr_t dma_addr;
	struct sipa_cmn_fifo_cfg_tag *cmn_fifo;

	cmn_fifo = &ipa->cmn_fifo_cfg[id];
	cmn_fifo->fifo_id = id;
	cmn_fifo->dst = SIPA_TERM_VCP;
	cmn_fifo->cur = SIPA_TERM_PCIE0;
	size = cmn_fifo->tx_fifo.depth *
		sizeof(struct sipa_node_description_tag);
	cmn_fifo->tx_fifo.virtual_addr = dma_alloc_coherent(ipa->pci_dev, size,
							    &dma_addr,
							    GFP_KERNEL);
	if (!cmn_fifo->tx_fifo.virtual_addr)
		return -ENOMEM;
	cmn_fifo->tx_fifo.dma_ptr = dma_addr;
	memset(cmn_fifo->tx_fifo.virtual_addr, 0, size);
	pr_info("comfifo%d tx_fifo addr-0x%lx\n", id, (long unsigned int)cmn_fifo->tx_fifo.virtual_addr);

	cmn_fifo->tx_fifo.fifo_base_addr_l = lower_32_bits(dma_addr);
	cmn_fifo->tx_fifo.fifo_base_addr_h = 0x2;

	size = cmn_fifo->rx_fifo.depth *
		sizeof(struct sipa_node_description_tag);
	cmn_fifo->rx_fifo.virtual_addr = dma_alloc_coherent(ipa->pci_dev, size,
							    &dma_addr,
							    GFP_KERNEL);
	if (!cmn_fifo->rx_fifo.virtual_addr)
		return -ENOMEM;
	cmn_fifo->rx_fifo.dma_ptr = dma_addr;
	memset(cmn_fifo->rx_fifo.virtual_addr, 0, size);
	pr_info("comfifo%d rx_fifo addr-0x%lx\n", id, (long unsigned int)cmn_fifo->rx_fifo.virtual_addr);

	cmn_fifo->rx_fifo.fifo_base_addr_l = lower_32_bits(dma_addr);
	cmn_fifo->rx_fifo.fifo_base_addr_h = 0x2;

	return 0;
}

static void sipa_free_cmn_fifo(struct sipa_core *ipa, enum sipa_cmn_fifo_index id)
{
	size_t size;
	struct sipa_cmn_fifo_cfg_tag *cmn_fifo;

	cmn_fifo = &ipa->cmn_fifo_cfg[id];
	size = cmn_fifo->tx_fifo.depth * sizeof(struct sipa_node_description_tag);
	dma_free_coherent(ipa->dev, size, cmn_fifo->tx_fifo.virtual_addr, cmn_fifo->tx_fifo.dma_ptr);
	size = cmn_fifo->rx_fifo.depth * sizeof(struct sipa_node_description_tag);
	dma_free_coherent(ipa->dev, size, cmn_fifo->rx_fifo.virtual_addr, cmn_fifo->rx_fifo.dma_ptr);	
}

static void sipa_init_ep(struct sipa_core *ipa)
{
	struct sipa_endpoint *ep = &ipa->ep;

	ep->send_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL];
	ep->recv_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL];
}

#ifdef SPRD_PCIE_USE_DTS
static int sipa_parse_dts_configuration(struct platform_device *pdev,
					struct sipa_core *ipa)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *cmn_fifo;

	ipa->reg_res = platform_get_resource_byname(pdev,
						    IORESOURCE_MEM, "ipa-base");
	if (!ipa->reg_res) {
		dev_err(&pdev->dev, "get ipa-base res fail\n");
		return -EINVAL;
	}

	cmn_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL];
	ret = of_property_read_u32(pdev->dev.of_node, "pcie-dl-tx-fifo-depth",
				   &cmn_fifo->tx_fifo.depth);
	if (ret) {
		dev_err(&pdev->dev,
			"get pcie-dl-tx-fifo-depth ret = %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "pcie-dl-rx-fifo-depth",
				   &cmn_fifo->rx_fifo.depth);
	if (ret) {
		dev_err(&pdev->dev,
			"get pcie-dl-rx-fifo-depth ret = %d\n", ret);
		return ret;
	}

	cmn_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL];
	ret = of_property_read_u32(pdev->dev.of_node, "pcie-ul-tx-fifo-depth",
				   &cmn_fifo->tx_fifo.depth);
	if (ret) {
		dev_err(&pdev->dev,
			"get pcie-ul-tx-fifo-depth ret = %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "pcie-ul-rx-fifo-depth",
				   &cmn_fifo->rx_fifo.depth);
	if (ret) {
		dev_err(&pdev->dev,
			"get pcie-ul-rx-fifo-depth ret = %d\n", ret);
		return ret;
	}

	return 0;
}
#else
static struct resource ipa_res = {
	.start = 0x2e000000,
	.end  = 0x2e000000 + 0x2000 -1,
        .flags = IORESOURCE_MEM,
};

static int sipa_parse_dts_configuration(struct platform_device *pdev,
					struct sipa_core *ipa)
{
	struct sipa_cmn_fifo_cfg_tag *cmn_fifo;
	ipa->reg_res  = &ipa_res;
	cmn_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL];
	cmn_fifo->tx_fifo.depth = 4096;
	cmn_fifo->rx_fifo.depth = 4096;
	cmn_fifo = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL];
	cmn_fifo->tx_fifo.depth = 4096;
	cmn_fifo->rx_fifo.depth = 4096;
	return 0;
}
#endif

static int sipa_plat_drv_probe(struct platform_device *pdev)
{
	int ret;
	struct sipa_core *ipa;
	struct device *dev = &pdev->dev;
	struct device *pci_dev;

	pci_dev = (struct device *)dev_get_drvdata(dev);
	if(!pci_dev)
		return -1;
	
	ipa = devm_kzalloc(dev, sizeof(*ipa), GFP_KERNEL);
	if (!ipa)
		return -ENOMEM;

	sipa_ctrl = ipa;
	ipa->dev = dev;
	ipa->pci_dev = pci_dev;
	ipa->pcie_mem_offset = SIPA_PCIE_MEM_OFFSET;
	dev_set_drvdata(dev, ipa);
	ret = sipa_parse_dts_configuration(pdev, ipa);
	if (ret)
		return ret;

	ret = sipa_init_cmn_fifo(ipa, SIPA_FIFO_PCIE_DL);
	if (ret)
		return ret;

	ret = sipa_init_cmn_fifo(ipa, SIPA_FIFO_PCIE_UL);
	if (ret)
		return ret;

	sipa_init_ep(ipa);

	sipa_fifo_ops_init(&ipa->hal_ops);
	INIT_WORK(&ipa->flow_ctrl_work, sipa_notify_sender_flow_ctrl);

	create_sipa_skb_receiver(&ipa->ep, &ipa->receiver);
	create_sipa_skb_sender(&ipa->ep, &ipa->sender);
	device_init_wakeup(dev, true);

	sipa_create_smsg_channel(ipa);

	sprd_ep_dev_register_irq_handler(PCIE_EP_MODEM, PCIE_MSI_IPA,
					 (irq_handler_t)sipa_int_callback_func,
					 (void *)ipa);
	sipa_init_debugfs(ipa);

	return 0;
}

extern void destroy_sipa_skb_receiver(struct sipa_skb_receiver *receiver);
extern void destroy_sipa_skb_sender(struct sipa_skb_sender *sender);

static int sipa_plat_drv_remove(struct platform_device *pdev)
{
	struct sipa_core *ipa;

	ipa = dev_get_drvdata(&pdev->dev);
	smsg_ch_close(SIPC_ID_MINIAP, SMSG_CH_COMM_SIPA,  1000);
	if(ipa->smsg_thread){  
		kthread_stop(ipa->smsg_thread);  
        ipa->smsg_thread = NULL;  
	}
	destroy_sipa_skb_sender(ipa->sender);
	destroy_sipa_skb_receiver(ipa->receiver);
	cancel_work_sync(&ipa->flow_ctrl_work);
    mdelay(1000);
	sipa_free_cmn_fifo(ipa, SIPA_FIFO_PCIE_UL);
	sipa_free_cmn_fifo(ipa, SIPA_FIFO_PCIE_DL);
	if (!IS_ERR_OR_NULL(ipa->dentry))
		debugfs_remove_recursive(ipa->dentry);
	devm_kfree(&pdev->dev, ipa);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef SPRD_PCIE_USE_DTS
static const struct of_device_id sipa_plat_drv_match[] = {
	{ .compatible = "sprd,virt-sipa"},
};
#endif

static struct platform_driver sipa_plat_drv = {
	.probe = sipa_plat_drv_probe,
	.remove =  sipa_plat_drv_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
#ifdef SPRD_PCIE_USE_DTS
		.of_match_table = sipa_plat_drv_match,
#endif
	},
};

#ifndef SPRD_PCIE_USE_DTS
static struct platform_device *sipa_plat_dev;
static int sipa_core_platform_device_reigster(struct device *dev) 
{
	int retval = -ENOMEM;
	
	sipa_plat_dev = platform_device_alloc("virt_sipa", -1);
	if (!sipa_plat_dev)
		return retval;
		
	sipa_plat_dev->dev.dma_mask = dev->dma_mask;
	sipa_plat_dev->dev.coherent_dma_mask = dev->coherent_dma_mask;
	sipa_plat_dev->dev.archdata = dev->archdata;
	dev_set_drvdata(&sipa_plat_dev->dev, dev);
	retval = platform_device_add(sipa_plat_dev);
	if (retval < 0)
		platform_device_put(sipa_plat_dev);

	return retval;           
}
#endif

int sipa_module_init(struct device  *dev)
{
#ifndef SPRD_PCIE_USE_DTS
	sipa_core_platform_device_reigster(dev);
#endif 
	return platform_driver_register(&sipa_plat_drv);
}
EXPORT_SYMBOL(sipa_module_init);

void sipa_module_exit(void)
{
	platform_driver_unregister(&sipa_plat_drv);
#ifndef SPRD_PCIE_USE_DTS
	platform_device_unregister(sipa_plat_dev);
 #endif  
}
EXPORT_SYMBOL(sipa_module_exit);
