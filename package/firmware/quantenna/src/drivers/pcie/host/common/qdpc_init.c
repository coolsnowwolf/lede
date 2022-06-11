/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
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
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/firmware.h>
#include <linux/module.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_regs.h"
#include "qdpc_ruby.h"
#include "qdpc_platform.h"

MODULE_AUTHOR("Quantenna");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual ethernet driver for Quantenna Ruby device");
MODULE_VERSION(QDPC_MODULE_VERSION);

#ifdef TOPAZ_PLATFORM
#define QDPC_RUBY_IMG "topaz-linux.lzma.img"
#else
#define QDPC_RUBY_IMG "ruby-linux.lzma.img"
#endif
#define EP_BOOT_FROM_FLASH 1

/* Quantenna PCIE vendor and device identifiers  */
static struct pci_device_id qdpc_pcie_ids[] = {
	{PCI_DEVICE(QDPC_VENDOR_ID, QDPC_DEVICE_ID),},
	{0,}
};

MODULE_DEVICE_TABLE(pci, qdpc_pcie_ids);

static int qdpc_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void qdpc_pcie_remove(struct pci_dev *pdev);
static int qdpc_boot_thread(void *data);
static void qdpc_nl_recv_msg(struct sk_buff *skb);
int qdpc_init_netdev(struct net_device  **net_dev, struct pci_dev *pdev);

char qdpc_pcie_driver_name[] = "qdpc_pcie_device_driver";

static struct pci_driver qdpc_pcie_driver = {
	.name     = qdpc_pcie_driver_name,
	.id_table = qdpc_pcie_ids,
	.probe    = qdpc_pcie_probe,
	.remove   = qdpc_pcie_remove,
};

static struct net_device  *qdpc_net_dev_g  = NULL;
struct sock *qdpc_nl_sk = NULL;
int qdpc_clntPid = 0;

unsigned int (*qdpc_pci_readl)(void *addr) = qdpc_readl;
void (*qdpc_pci_writel)(unsigned int val, void *addr) = qdpc_writel;

static int qdpc_bootpoll(struct qdpc_priv *p, uint32_t state)
{
	while (!kthread_should_stop() && (qdpc_isbootstate(p,state) == 0)) {
		if (qdpc_booterror(p))
			return -1;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(QDPC_SCHED_TIMEOUT);
	}
	return 0;
}
static void booterror(qdpc_pcie_bda_t *bda)
{
	if (PCIE_BDA_TARGET_FWLOAD_ERR & qdpc_pci_readl(&bda->bda_flags))
		printk("EP boot from download firmware failed!\n");
	else if (PCIE_BDA_TARGET_FBOOT_ERR & qdpc_pci_readl(&bda->bda_flags))
		printk("EP boot from flash failed! Please check if there is usable image in Target flash.\n");
	else
		printk("EP boot get in error, dba flag: 0x%x\n", qdpc_pci_readl(&bda->bda_flags));
}

static void qdpc_pci_endian_detect(struct qdpc_priv *priv)
{
	__iomem qdpc_pcie_bda_t *bda = priv->bda;
	volatile uint32_t pci_endian;

	writel(QDPC_PCI_ENDIAN_DETECT_DATA, &bda->bda_pci_endian);
	mmiowb();
	writel(QDPC_PCI_ENDIAN_VALID_STATUS, &bda->bda_pci_pre_status);

	while (readl(&bda->bda_pci_post_status) != QDPC_PCI_ENDIAN_VALID_STATUS) {
		if (kthread_should_stop())
			break;

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(QDPC_SCHED_TIMEOUT);
	}

	pci_endian = readl(&bda->bda_pci_endian);
	if (pci_endian == QDPC_PCI_LITTLE_ENDIAN) {
		qdpc_pci_readl = qdpc_readl;
		qdpc_pci_writel = qdpc_writel;
		printk("PCI memory is little endian\n");
	} else if (pci_endian == QDPC_PCI_BIG_ENDIAN) {
		qdpc_pci_readl = qdpc_le32_readl;
		qdpc_pci_writel = qdpc_le32_writel;
		printk("PCI memory is big endian\n");
	} else {
		qdpc_pci_readl = qdpc_readl;
		qdpc_pci_writel = qdpc_writel;
		printk("PCI memory endian value:%08x is invalid - using little endian\n", pci_endian);
	}

	/* Clear endian flags */
	writel(0, &bda->bda_pci_pre_status);
	writel(0, &bda->bda_pci_post_status);
	writel(0, &bda->bda_pci_endian);
}

static int qdpc_firmware_load(struct pci_dev *pdev, struct qdpc_priv *priv)
{
#define DMABLOCKSIZE	(64*1024)
#define NBLOCKS(size)  ((size)/(DMABLOCKSIZE) + (((size)%(DMABLOCKSIZE) > 0) ? 1 : 0))

	int result = SUCCESS;
	const struct firmware *fw;
	__iomem qdpc_pcie_bda_t  *bda = priv->bda;


	/* Request compressed firmware from user space */
	if ((result = request_firmware(&fw, QDPC_RUBY_IMG, &pdev->dev)) == -ENOENT) {
		/*
		 * No firmware found in the firmware directory, skip firmware downloading process
		 * boot from flash directly on target
		 */
		printk( "no firmware found skip fw downloading\n");
		qdpc_pcie_posted_write((PCIE_BDA_HOST_NOFW_ERR |
					qdpc_pci_readl(&bda->bda_flags)), &bda->bda_flags);
		return FAILURE;
	} else if (result == SUCCESS) {
		uint32_t nblocks = NBLOCKS(fw->size);
		uint32_t remaining = fw->size;
		uint32_t count;
		uint32_t dma_offset = qdpc_pci_readl(&bda->bda_dma_offset);
		void *data = kmalloc(DMABLOCKSIZE, GFP_KERNEL|GFP_DMA);
		const uint8_t *curdata = fw->data;
		dma_addr_t handle = 0;

		if (!data) {
			printk(KERN_ERR "Allocation failed for memory size[%u] Download firmware failed!\n", DMABLOCKSIZE);
			release_firmware(fw);
			qdpc_pcie_posted_write((PCIE_BDA_HOST_MEMALLOC_ERR |
				qdpc_pci_readl(&bda->bda_flags)), &bda->bda_flags);
			return FAILURE;
		}

		handle = pci_map_single(priv->pdev, data ,DMABLOCKSIZE, PCI_DMA_TODEVICE);
		if (!handle) {
			printk("Pci map for memory data block 0x%p error, Download firmware failed!\n", data);
			kfree(data);
			release_firmware(fw);
			qdpc_pcie_posted_write((PCIE_BDA_HOST_MEMMAP_ERR |
				qdpc_pci_readl(&bda->bda_flags)), &bda->bda_flags);
			return FAILURE;
		}

		qdpc_setbootstate(priv, QDPC_BDA_FW_HOST_LOAD);
		qdpc_bootpoll(priv, QDPC_BDA_FW_EP_RDY);

		/* Start loading firmware */
		for (count = 0 ; count < nblocks; count++)
		{
			uint32_t size = (remaining > DMABLOCKSIZE) ? DMABLOCKSIZE : remaining;

			memcpy(data, curdata, size);
			/* flush dcache */
			pci_dma_sync_single_for_device(priv->pdev, handle ,size, PCI_DMA_TODEVICE);

			qdpc_pcie_posted_write(handle + dma_offset, &bda->bda_img);
			qdpc_pcie_posted_write(size, &bda->bda_img_size);
			printk("FW Data[%u]: VA:0x%p PA:0x%p Sz=%u..\n", count, (void *)curdata, (void *)handle, size);

			qdpc_setbootstate(priv, QDPC_BDA_FW_BLOCK_RDY);
			qdpc_bootpoll(priv, QDPC_BDA_FW_BLOCK_DONE);

			remaining = (remaining < size) ? remaining : (remaining - size);
			curdata += size;
			printk("done!\n");
		}

		pci_unmap_single(priv->pdev,handle, DMABLOCKSIZE, PCI_DMA_TODEVICE);
		/* Mark end of block */
		qdpc_pcie_posted_write(0, &bda->bda_img);
		qdpc_pcie_posted_write(0, &bda->bda_img_size);
		qdpc_setbootstate(priv, QDPC_BDA_FW_BLOCK_RDY);
		qdpc_bootpoll(priv, QDPC_BDA_FW_BLOCK_DONE);

		qdpc_setbootstate(priv, QDPC_BDA_FW_BLOCK_END);

		PRINT_INFO("Image. Sz:%u State:0x%x\n", (uint32_t)fw->size, qdpc_pci_readl(&bda->bda_bootstate));
		qdpc_bootpoll(priv, QDPC_BDA_FW_LOAD_DONE);

		kfree(data);
		release_firmware(fw);
		PRINT_INFO("Image downloaded....!\n");
	} else {
		PRINT_ERROR("Failed to load firmware:%d\n", result);
		return result;
     }
	return result;
}

static int qdpc_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct qdpc_priv *priv = NULL;
	struct net_device *ndev = NULL;
	int result = SUCCESS;

	/* Net device initialization */
	if ((result = qdpc_init_netdev(&ndev, pdev)) < 0) {
		return result;
	}

	qdpc_net_dev_g = ndev;
	priv = netdev_priv(ndev);

	/* Check if the device has PCI express capability */
	if (!pci_find_capability(pdev, PCI_CAP_ID_EXP)) {
		PRINT_ERROR(KERN_ERR "The device %x does not have PCI Express capability\n",
	                pdev->device);
		result = -ENOSYS;
		goto out;
	} else {
		PRINT_DBG(KERN_INFO "The device %x has PCI Express capability\n", pdev->device);
	}

	/*  Wake up the device if it is in suspended state and allocate IO,
	 *  memory regions and IRQ if not
	 */
	if (pci_enable_device(pdev)) {
		PRINT_ERROR(KERN_ERR "Failed to initialize PCI device with device ID %x\n",
				pdev->device);

		result = -EIO;
		goto out;
	} else {
		PRINT_DBG(KERN_INFO "Initialized PCI device with device ID %x\n", pdev->device);
	}

	/*
	 * Check if the PCI device can support DMA addressing properly.
	 * The mask gives the bits that the device can address
	 */
	pci_set_master(pdev);

	/* Initialize PCIE layer  */
	if (( result = qdpc_pcie_init_intr_and_mem(ndev)) < 0) {
		PRINT_DBG("Interrupt & Memory Initialization failed \n");
		goto release_memory;
	}


	/* Create and start the thread to initiate the INIT Handshake*/
	priv->init_thread = kthread_run(qdpc_boot_thread, priv, "qdpc_init_thread");
	if (priv->init_thread == NULL) {
		PRINT_ERROR("Init thread creation failed \n");
		goto free_mem_interrupt;
	}

	/* Register net device with the kernel  */
	if ((result = register_netdev(ndev))) {
		PRINT_DBG("veth: error %d registering net device \"%s\"\n",
				result, ndev->name);
		goto stop_init_kthread;
	}


	/* Create netlink & register with kernel */
	priv->netlink_socket = netlink_kernel_create(&init_net, QDPC_NETLINK_RPC_PCI, 0, qdpc_nl_recv_msg,
	                            NULL, THIS_MODULE);
	if (priv->netlink_socket) {
		return SUCCESS;
	}

	PRINT_ERROR(KERN_ALERT "Error creating netlink socket.\n");
	result = FAILURE;

	/* If any failure is happened. */
stop_init_kthread:
	if (priv->init_thread)
		kthread_stop(priv->init_thread);

free_mem_interrupt:
	qdpc_pcie_free_mem(pdev);
	qdpc_free_interrupt(pdev);

release_memory:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
	/* Releasing the memory region if any error occured */
	pci_clear_master(pdev);
#endif

	pci_disable_device(pdev);

out:
	free_netdev(ndev);
	/* Any failure in probe, so it can directly return in remove */
	pci_set_drvdata(pdev, NULL);

	return result;
}

static void qdpc_pcie_remove(struct pci_dev *pdev)
{
	struct net_device *ndev = pci_get_drvdata(pdev);
	struct qdpc_priv *priv = NULL;

	/* Any failure in probe */
	if (ndev == NULL) {
		return;
	}

	priv  = netdev_priv(ndev);
	/* Net device cleanup */
	unregister_netdev(ndev);
	qdpc_net_dev_g = NULL;

	/* Stoping kthread */
	if (priv->init_thread)
		kthread_stop(priv->init_thread);
	qdpc_datapath_uninit(priv);
	qdpc_unmap_iomem(priv);
	if (priv->netlink_socket) {
		/* release netlink socket */
		netlink_kernel_release(priv->netlink_socket);
	}

	/* Free allocated memory */
	qdpc_pcie_free_mem(pdev);
	/* Free interrupt line. */
	qdpc_free_interrupt(pdev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
	pci_clear_master(pdev);
#endif
	/* Disable the device  */
	pci_disable_device(pdev);
	/* Free netdevice */
	free_netdev(ndev);

	return;
}

static int __init qdpc_init_module(void)
{
	int ret;

	PRINT_DBG(KERN_INFO "Quantenna pcie driver initialization\n");

	if (qdpc_platform_init()) {
		PRINT_ERROR("Platform initilization failed \n");
		ret = FAILURE;
		return ret;
	}

	/*  Register the pci driver with device*/
	if ((ret = pci_register_driver(&qdpc_pcie_driver)) < 0 ) {
		PRINT_ERROR("Could not register the driver to pci : %d\n", ret);
		ret = -ENODEV;
		return ret;
	}

	return ret;
}


static void __exit qdpc_exit_module(void)
{
	/* Release netlink */
	qdpc_platform_exit();

	/* Unregister the pci driver with the device */
	pci_unregister_driver(&qdpc_pcie_driver);

	return;
}

static int qdpc_boot_thread(void *data)
{
	struct qdpc_priv *priv = (struct qdpc_priv *)data;
	__iomem qdpc_pcie_bda_t  *bda = priv->bda;
	struct net_device *ndev;
	unsigned char macaddr[ETH_ALEN] = {0};
	uint32_t tmp;

	qdpc_pci_endian_detect(priv);

	printk("Setting HOST ready...\n");
	qdpc_setbootstate(priv, QDPC_BDA_FW_HOST_RDY);
	qdpc_bootpoll(priv, QDPC_BDA_FW_TARGET_RDY);

	if (qdpc_set_dma_mask(priv)){
		printk("Failed to map DMA mask.\n");
		priv->init_thread = NULL;
		do_exit(-1);
	}

	if ((PCIE_BDA_FLASH_PRESENT & qdpc_pci_readl(&bda->bda_flags)) && EP_BOOT_FROM_FLASH) {
		printk("EP have fw in flash, boot from flash\n");
		qdpc_pcie_posted_write((PCIE_BDA_FLASH_BOOT |
			qdpc_pci_readl(&bda->bda_flags)), &bda->bda_flags);
		qdpc_setbootstate(priv, QDPC_BDA_FW_TARGET_BOOT);
		qdpc_bootpoll(priv, QDPC_BDA_FW_FLASH_BOOT);
		goto fw_start;
	}
	qdpc_setbootstate(priv, QDPC_BDA_FW_TARGET_BOOT);
	printk("EP FW load request...\n");
	qdpc_bootpoll(priv, QDPC_BDA_FW_LOAD_RDY);

	printk("Start Firmware download...\n");
	if (qdpc_firmware_load(priv->pdev, priv)){
		printk("Failed to download firmware.\n");
		priv->init_thread = NULL;
		do_exit(-1);
	}

fw_start:
	qdpc_setbootstate(priv, QDPC_BDA_FW_START);
	printk("Start booting EP...\n");
	if (qdpc_bootpoll(priv,QDPC_BDA_FW_CONFIG)) {
		booterror(bda);
		priv->init_thread = NULL;
		do_exit(-1);
	}
	printk("EP boot successful, starting config...\n");
	if (qdpc_dma_setup(priv)) {
		printk("Failed to initialize DMA.\n");
		priv->init_thread = NULL;
		do_exit(-1);
	}

	qdpc_setbootstate(priv, QDPC_BDA_FW_RUN);
	qdpc_bootpoll(priv,QDPC_BDA_FW_RUNNING);

	ndev = priv->ndev;

	/*
	 * Get MAC address from boot data area.
	 * Two fields (bda_pci_pre_status and bda_pci_endian) are overloaded for
	 * this purpose in order to avoid updating the bootloader.
	 */
	tmp = qdpc_pci_readl(&bda->bda_pci_pre_status);
	macaddr[0] = tmp & 0xFF;
	macaddr[1] = (tmp >> 8) & 0xFF;
	macaddr[2] = (tmp >> 16) & 0xFF;
	macaddr[3] = (tmp >> 24) & 0xFF;
	tmp = qdpc_pci_readl(&bda->bda_pci_endian);
	macaddr[4] = tmp & 0xFF;
	macaddr[5] = (tmp >> 8) & 0xFF;
	memcpy(ndev->dev_addr, macaddr, ETH_ALEN);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	ndev->netdev_ops->ndo_open(ndev);
#else
	ndev->open(ndev);
#endif

	PRINT_INFO("Connection established with Target BBIC3 board\n");

	tasklet_enable(&priv->rx_tasklet);
	tasklet_enable(&priv->txd_tasklet);

	priv->init_thread = NULL;
	do_exit(0);
}

static void qdpc_nl_recv_msg(struct sk_buff *skb)
{
	struct qdpc_priv *priv = netdev_priv(qdpc_net_dev_g);
	struct nlmsghdr *nlh  = (struct nlmsghdr*)skb->data;
	struct sk_buff *skb2;
	/* Parsing the netlink message */

	PRINT_DBG(KERN_INFO "%s line %d Netlink received pid:%d, size:%d, type:%d\n",
		__FUNCTION__, __LINE__, nlh->nlmsg_pid, nlh->nlmsg_len, nlh->nlmsg_type);

	switch (nlh->nlmsg_type) {
		case QDPC_NETLINK_TYPE_CLNT_REGISTER:
			if (nlh->nlmsg_len) {
				PRINT_DBG(KERN_INFO "%s line %d Netlink received client register size:%d\n",
					__FUNCTION__, __LINE__, nlh->nlmsg_len);
			}

			priv->netlink_pid = nlh->nlmsg_pid; /*pid of sending process */;
			return;
		case QDPC_NETLINK_TYPE_CLNT_REQUEST:
			break;
		default:
			PRINT_DBG(KERN_INFO "%s line %d Netlink Invalid type %d\n",
				__FUNCTION__, __LINE__, nlh->nlmsg_type);
			return;
	}

	/*
	 * make a new skb. The old skb will freed in netlink_unicast_kernel,
	 * but we must hold the skb before DMA transfer done
	 */
	skb2 = alloc_skb(nlh->nlmsg_len+sizeof(qdpc_cmd_hdr_t), GFP_ATOMIC);
	if (skb2) {
		qdpc_cmd_hdr_t *cmd_hdr;
		cmd_hdr = (qdpc_cmd_hdr_t *)skb2->data;
		memcpy(cmd_hdr->dst_magic, QDPC_NETLINK_DST_MAGIC, ETH_ALEN);
		memcpy(cmd_hdr->src_magic, QDPC_NETLINK_SRC_MAGIC, ETH_ALEN);
		cmd_hdr->type = htons(QDPC_APP_NETLINK_TYPE);
		cmd_hdr->len = htons(nlh->nlmsg_len);
		memcpy(skb2->data+sizeof(qdpc_cmd_hdr_t), skb->data+sizeof(struct nlmsghdr), nlh->nlmsg_len);

		skb_put(skb2, nlh->nlmsg_len+sizeof(qdpc_cmd_hdr_t));
		qdpc_send_packet(skb2, qdpc_net_dev_g);
	}
}


module_init(qdpc_init_module);
module_exit(qdpc_exit_module);
