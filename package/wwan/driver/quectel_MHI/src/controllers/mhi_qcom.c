/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include "../core/mhi.h"
#include "mhi_qcom.h"

#if 1
#ifndef PCI_IRQ_MSI
#define PCI_IRQ_MSI		(1 << 1) /* Allow MSI interrupts */

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,10,53 ))
int pci_enable_msi_range(struct pci_dev *dev, int minvec, int maxvec)
{
	int nvec = maxvec;
	int rc;

	if (maxvec < minvec)
		return -ERANGE;

	do {
		rc = pci_enable_msi_block(dev, nvec);
		if (rc < 0) {
			return rc;
		} else if (rc > 0) {
			if (rc < minvec)
				return -ENOSPC;
			nvec = rc;
		}
	} while (rc);

	return nvec;
}
#endif

static int pci_alloc_irq_vectors(struct pci_dev *dev, unsigned int min_vecs,
		      unsigned int max_vecs, unsigned int flags)
{
	return pci_enable_msi_range(dev, min_vecs, max_vecs);
}

static void pci_free_irq_vectors(struct pci_dev *dev)
{
	pci_disable_msi(dev);
}

static int pci_irq_vector(struct pci_dev *dev, unsigned int nr)
{
    return dev->irq + nr;
}
#endif
#endif

static struct pci_device_id mhi_pcie_device_id[] = {
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0303)}, //SDX20
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0304)}, //SDX24
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0305)},
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0306)}, //SDX55
	{PCI_DEVICE(0x2C7C, 0x0512)},
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, MHI_PCIE_DEBUG_ID)},
	{0},
};

MODULE_DEVICE_TABLE(pci, mhi_pcie_device_id);

static struct pci_driver mhi_pcie_driver;

void mhi_deinit_pci_dev(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct pci_dev *pci_dev = mhi_dev->pci_dev;

	pci_free_irq_vectors(pci_dev);
	iounmap(mhi_cntrl->regs);
	mhi_cntrl->regs = NULL;
	pci_clear_master(pci_dev);
	pci_release_region(pci_dev, mhi_dev->resn);
	pci_disable_device(pci_dev);
}

static int mhi_init_pci_dev(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int ret;
	resource_size_t start, len;
	int i;

	mhi_dev->resn = MHI_PCI_BAR_NUM;
	ret = pci_assign_resource(pci_dev, mhi_dev->resn);
	if (ret) {
		MHI_ERR("Error assign pci resources, ret:%d\n", ret);
		return ret;
	}

	ret = pci_enable_device(pci_dev);
	if (ret) {
		MHI_ERR("Error enabling device, ret:%d\n", ret);
		goto error_enable_device;
	}

	ret = pci_request_region(pci_dev, mhi_dev->resn, "mhi");
	if (ret) {
		MHI_ERR("Error pci_request_region, ret:%d\n", ret);
		goto error_request_region;
	}

	pci_set_master(pci_dev);

	start = pci_resource_start(pci_dev, mhi_dev->resn);
	len = pci_resource_len(pci_dev, mhi_dev->resn);
	mhi_cntrl->regs = ioremap_nocache(start, len);
	MHI_LOG("mhi_cntrl->regs = %p\n", mhi_cntrl->regs);
	if (!mhi_cntrl->regs) {
		MHI_ERR("Error ioremap region\n");
		goto error_ioremap;
	}

	ret = pci_alloc_irq_vectors(pci_dev, 1, mhi_cntrl->msi_required, PCI_IRQ_MSI);
	if (IS_ERR_VALUE((ulong)ret) || ret < mhi_cntrl->msi_required) {
		if (ret == -ENOSPC) {
			/* imx_3.14.52_1.1.0_ga
			diff --git a/drivers/pci/host/pcie-designware.c b/drivers/pci/host/pcie-designware.c
			index f06e8f0..6a9614f 100644
			--- a/drivers/pci/host/pcie-designware.c
			+++ b/drivers/pci/host/pcie-designware.c
			@@ -376,6 +376,13 @@ static int dw_msi_setup_irq(struct msi_chip *chip, struct pci_dev *pdev,
				if (msgvec > 5)
					msgvec = 0;

			+#if 1 //Add by Quectel 20190419
			+       if (msgvec > 0 && pdev->vendor == 0x17cb) {
			+               dev_info(&pdev->dev, "%s quectel fixup pos=%d, msg_ctr=%04x, msgvec=%d\n",  __func__, desc->msi_attrib.pos, msg_ctr, msgvec);
			+               msgvec = 0;
			+       }
			+#endif
			+
				irq = assign_irq((1 << msgvec), desc, &pos);
				if (irq < 0)
					return irq;
			*/
		}
		//imx_4.1.15_2.0.0_ga & DELL_OPTIPLEX_7010 only alloc one msi interrupt for one pcie device
		if (ret != 1) {
			MHI_ERR("Failed to enable MSI, ret=%d, msi_required=%d\n", ret, mhi_cntrl->msi_required);
			goto error_req_msi;
		}
	}

	mhi_cntrl->msi_allocated = ret;
	MHI_LOG("msi_required = %d, msi_allocated = %d, msi_irq = %u\n", mhi_cntrl->msi_required, mhi_cntrl->msi_allocated, pci_dev->irq);

	for (i = 0; i < mhi_cntrl->msi_allocated; i++) {
		mhi_cntrl->irq[i] = pci_irq_vector(pci_dev, i);
		if (mhi_cntrl->irq[i] < 0) {
			ret = mhi_cntrl->irq[i];
			goto error_get_irq_vec;
		}
	}

#if 0
	/* configure runtime pm */
	pm_runtime_set_autosuspend_delay(&pci_dev->dev, MHI_RPM_SUSPEND_TMR_MS);
	pm_runtime_dont_use_autosuspend(&pci_dev->dev);
	pm_suspend_ignore_children(&pci_dev->dev, true);

	/*
	 * pci framework will increment usage count (twice) before
	 * calling local device driver probe function.
	 * 1st pci.c pci_pm_init() calls pm_runtime_forbid
	 * 2nd pci-driver.c local_pci_probe calls pm_runtime_get_sync
	 * Framework expect pci device driver to call
	 * pm_runtime_put_noidle to decrement usage count after
	 * successful probe and and call pm_runtime_allow to enable
	 * runtime suspend.
	 */
	pm_runtime_mark_last_busy(&pci_dev->dev);
	pm_runtime_put_noidle(&pci_dev->dev);
#endif

	return 0;

error_get_irq_vec:
	pci_free_irq_vectors(pci_dev);

error_req_msi:
	iounmap(mhi_cntrl->regs);

error_ioremap:
	pci_clear_master(pci_dev);

error_request_region:
	pci_disable_device(pci_dev);

error_enable_device:
	pci_release_region(pci_dev, mhi_dev->resn);

	return ret;
}

#ifdef CONFIG_PM
static int mhi_runtime_idle(struct device *dev)
{
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);

	MHI_LOG("Entered returning -EBUSY\n");

	/*
	 * RPM framework during runtime resume always calls
	 * rpm_idle to see if device ready to suspend.
	 * If dev.power usage_count count is 0, rpm fw will call
	 * rpm_idle cb to see if device is ready to suspend.
	 * if cb return 0, or cb not defined the framework will
	 * assume device driver is ready to suspend;
	 * therefore, fw will schedule runtime suspend.
	 * In MHI power management, MHI host shall go to
	 * runtime suspend only after entering MHI State M2, even if
	 * usage count is 0.  Return -EBUSY to disable automatic suspend.
	 */
	return -EBUSY;
}

static int mhi_runtime_suspend(struct device *dev)
{
	int ret = 0;
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);

	MHI_LOG("Enter\n");

	mutex_lock(&mhi_cntrl->pm_mutex);

	ret = mhi_pm_suspend(mhi_cntrl);
	if (ret) {
		MHI_LOG("Abort due to ret:%d\n", ret);
		goto exit_runtime_suspend;
	}

	ret = mhi_arch_link_off(mhi_cntrl, true);
	if (ret)
		MHI_ERR("Failed to Turn off link ret:%d\n", ret);

exit_runtime_suspend:
	mutex_unlock(&mhi_cntrl->pm_mutex);
	MHI_LOG("Exited with ret:%d\n", ret);

	return ret;
}

static int mhi_runtime_resume(struct device *dev)
{
	int ret = 0;
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	MHI_LOG("Enter\n");

	mutex_lock(&mhi_cntrl->pm_mutex);

	if (!mhi_dev->powered_on) {
		MHI_LOG("Not fully powered, return success\n");
		mutex_unlock(&mhi_cntrl->pm_mutex);
		return 0;
	}

	/* turn on link */
	ret = mhi_arch_link_on(mhi_cntrl);
	if (ret)
		goto rpm_resume_exit;

	/* enter M0 state */
	ret = mhi_pm_resume(mhi_cntrl);

rpm_resume_exit:
	mutex_unlock(&mhi_cntrl->pm_mutex);
	MHI_LOG("Exited with :%d\n", ret);

	return ret;
}

static int mhi_system_resume(struct device *dev)
{
	int ret = 0;
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);

	ret = mhi_runtime_resume(dev);
	if (ret) {
		MHI_ERR("Failed to resume link\n");
	} else {
		pm_runtime_set_active(dev);
		pm_runtime_enable(dev);
	}

	return ret;
}

int mhi_system_suspend(struct device *dev)
{
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);

	MHI_LOG("Entered\n");

	/* if rpm status still active then force suspend */
	if (!pm_runtime_status_suspended(dev))
		return mhi_runtime_suspend(dev);

	pm_runtime_set_suspended(dev);
	pm_runtime_disable(dev);

	MHI_LOG("Exit\n");
	return 0;
}
#endif

/* checks if link is down */
static int mhi_link_status(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	u16 dev_id;
	int ret;

	/* try reading device id, if dev id don't match, link is down */
	ret = pci_read_config_word(mhi_dev->pci_dev, PCI_DEVICE_ID, &dev_id);

	return (ret || dev_id != mhi_cntrl->dev_id) ? -EIO : 0;
}

static int mhi_runtime_get(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	struct device *dev = &mhi_dev->pci_dev->dev;

	return pm_runtime_get(dev);
}

static void mhi_runtime_put(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	struct device *dev = &mhi_dev->pci_dev->dev;

	pm_runtime_put_noidle(dev);
}

static void mhi_status_cb(struct mhi_controller *mhi_cntrl,
			  void *priv,
			  enum MHI_CB reason)
{
	struct mhi_dev *mhi_dev = priv;
	struct device *dev = &mhi_dev->pci_dev->dev;

	if (reason == MHI_CB_IDLE) {
		MHI_LOG("Schedule runtime suspend 1\n");
		pm_runtime_mark_last_busy(dev);
		pm_request_autosuspend(dev);
	}
}

int mhi_debugfs_trigger_m0(void *data, u64 val)
{
	struct mhi_controller *mhi_cntrl = data;
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	MHI_LOG("Trigger M3 Exit\n");
	pm_runtime_get(&mhi_dev->pci_dev->dev);
	pm_runtime_put(&mhi_dev->pci_dev->dev);

	return 0;
}

int mhi_debugfs_trigger_m3(void *data, u64 val)
{
	struct mhi_controller *mhi_cntrl = data;
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	MHI_LOG("Trigger M3 Entry\n");
	pm_runtime_mark_last_busy(&mhi_dev->pci_dev->dev);
	pm_request_autosuspend(&mhi_dev->pci_dev->dev);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_trigger_m0_fops, NULL,
			mhi_debugfs_trigger_m0, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(debugfs_trigger_m3_fops, NULL,
			mhi_debugfs_trigger_m3, "%llu\n");

static int mhi_init_debugfs_trigger_go(void *data, u64 val)
{
	struct mhi_controller *mhi_cntrl = data;

	MHI_LOG("Trigger power up sequence\n");

	mhi_async_power_up(mhi_cntrl);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(mhi_init_debugfs_trigger_go_fops, NULL,
			mhi_init_debugfs_trigger_go, "%llu\n");


int mhi_init_debugfs_debug_show(struct seq_file *m, void *d)
{
	seq_puts(m, "Enable debug mode to debug  external soc\n");
	seq_puts(m,
		 "Usage:  echo 'devid,timeout,domain,smmu_cfg' > debug_mode\n");
	seq_puts(m, "No spaces between parameters\n");
	seq_puts(m, "\t1.  devid : 0 or pci device id to register\n");
	seq_puts(m, "\t2.  timeout: mhi cmd/state transition timeout\n");
	seq_puts(m, "\t3.  domain: Rootcomplex\n");
	seq_puts(m, "\t4.  smmu_cfg: smmu configuration mask:\n");
	seq_puts(m, "\t\t- BIT0: ATTACH\n");
	seq_puts(m, "\t\t- BIT1: S1 BYPASS\n");
	seq_puts(m, "\t\t-BIT2: FAST_MAP\n");
	seq_puts(m, "\t\t-BIT3: ATOMIC\n");
	seq_puts(m, "\t\t-BIT4: FORCE_COHERENT\n");
	seq_puts(m, "\t\t-BIT5: GEOMETRY\n");
	seq_puts(m, "\tAll timeout are in ms, enter 0 to keep default\n");
	seq_puts(m, "Examples inputs: '0x307,10000'\n");
	seq_puts(m, "\techo '0,10000,1'\n");
	seq_puts(m, "\techo '0x307,10000,0,0x3d'\n");
	seq_puts(m, "firmware image name will be changed to debug.mbn\n");

	return 0;
}

static int mhi_init_debugfs_debug_open(struct inode *node, struct file *file)
{
	return single_open(file, mhi_init_debugfs_debug_show, NULL);
}

static ssize_t mhi_init_debugfs_debug_write(struct file *fp,
					    const char __user *ubuf,
					    size_t count,
					    loff_t *pos)
{
	char *buf = kmalloc(count + 1, GFP_KERNEL);
	/* #,devid,timeout,domain,smmu-cfg */
	int args[5] = {0};
	static char const *dbg_fw = "debug.mbn";
	int ret;
	struct mhi_controller *mhi_cntrl = fp->f_inode->i_private;
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct pci_device_id *id;

	if (!buf)
		return -ENOMEM;

	ret = copy_from_user(buf, ubuf, count);
	if (ret)
		goto error_read;
	buf[count] = 0;
	get_options(buf, ARRAY_SIZE(args), args);
	kfree(buf);

	/* override default parameters */
	mhi_cntrl->fw_image = dbg_fw;
	mhi_cntrl->edl_image = dbg_fw;

	if (args[0] >= 2 && args[2])
		mhi_cntrl->timeout_ms = args[2];

	if (args[0] >= 3 && args[3])
		mhi_cntrl->domain = args[3];

	if (args[0] >= 4 && args[4])
		mhi_dev->smmu_cfg = args[4];

	/* If it's a new device id register it */
	if (args[0] && args[1]) {
		/* find the debug_id  and overwrite it */
		for (id = mhi_pcie_device_id; id->vendor; id++)
			if (id->device == MHI_PCIE_DEBUG_ID) {
				id->device = args[1];
				pci_unregister_driver(&mhi_pcie_driver);
				ret = pci_register_driver(&mhi_pcie_driver);
			}
	}

	mhi_dev->debug_mode = true;
	debugfs_create_file("go", 0444, mhi_cntrl->parent, mhi_cntrl,
			    &mhi_init_debugfs_trigger_go_fops);
	pr_info(
		"%s: ret:%d pcidev:0x%x smm_cfg:%u timeout:%u\n",
		__func__, ret, args[1], mhi_dev->smmu_cfg,
		mhi_cntrl->timeout_ms);
	return count;

error_read:
	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_debug_ops = {
	.open = mhi_init_debugfs_debug_open,
	.release = single_release,
	.read = seq_read,
	.write = mhi_init_debugfs_debug_write,
};

static struct mhi_controller * mhi_platform_probe(struct pci_dev *pci_dev)
{
	struct mhi_controller *mhi_cntrl;
	struct mhi_dev *mhi_dev;
	u64 addr_win[2];
	int ret;

	mhi_cntrl = mhi_alloc_controller(sizeof(*mhi_dev));
	if (!mhi_cntrl) {
		pr_err("mhi_alloc_controller fail\n");
		return NULL;
	}

	mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	mhi_cntrl->dev_id = pci_dev->device;
	mhi_cntrl->domain = pci_domain_nr(pci_dev->bus);
	mhi_cntrl->bus = pci_dev->bus->number;
	mhi_cntrl->slot = PCI_SLOT(pci_dev->devfn);
	mhi_dev->smmu_cfg = 0;
	#if 0 //def CONFIG_HAVE_MEMBLOCK
	addr_win[0] = memblock_start_of_DRAM();
	addr_win[1] = memblock_end_of_DRAM();
	#else
#define MHI_MEM_BASE_DEFAULT         0x000000000
#define MHI_MEM_SIZE_DEFAULT         0x2000000000
	addr_win[0] = MHI_MEM_BASE_DEFAULT;
	addr_win[1] = MHI_MEM_SIZE_DEFAULT;
	if (sizeof(dma_addr_t) == 4) {
		addr_win[1] = 0xFFFFFFFF;
	}
	#endif

	mhi_cntrl->iova_start = addr_win[0];
	mhi_cntrl->iova_stop = addr_win[1];

	mhi_dev->pci_dev = pci_dev;
	mhi_cntrl->pci_dev = pci_dev;

	/* setup power management apis */
	mhi_cntrl->status_cb = mhi_status_cb;
	mhi_cntrl->runtime_get = mhi_runtime_get;
	mhi_cntrl->runtime_put = mhi_runtime_put;
	mhi_cntrl->link_status = mhi_link_status;

	ret = mhi_arch_platform_init(mhi_dev);
	if (ret)
		goto error_probe;

	ret = mhi_register_mhi_controller(mhi_cntrl);
	if (ret)
		goto error_register;

	if (mhi_cntrl->parent)
		debugfs_create_file("debug_mode", 0444, mhi_cntrl->parent,
				    mhi_cntrl, &debugfs_debug_ops);

	return mhi_cntrl;

error_register:
	mhi_arch_platform_deinit(mhi_dev);

error_probe:
	mhi_free_controller(mhi_cntrl);

	return NULL;
}	

int mhi_pci_probe(struct pci_dev *pci_dev,
		  const struct pci_device_id *device_id)
{
	struct mhi_controller *mhi_cntrl = NULL;
	u32 domain = pci_domain_nr(pci_dev->bus);
	u32 bus = pci_dev->bus->number;
	u32 slot = PCI_SLOT(pci_dev->devfn);
	struct mhi_dev *mhi_dev;
	int ret;

	pr_info("%s pci_dev->name = %s, domain=%d, bus=%d, slot=%d, vendor=%04X, device=%04X\n",
		__func__, dev_name(&pci_dev->dev), domain, bus, slot, pci_dev->vendor, pci_dev->device);
	
	mhi_cntrl = mhi_platform_probe(pci_dev);
	if (!mhi_cntrl) {
		pr_err("mhi_platform_probe fail\n");
		return -EPROBE_DEFER;
	}
	
	mhi_cntrl->dev_id = pci_dev->device;
	mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	mhi_dev->pci_dev = pci_dev;
	mhi_dev->powered_on = true;

	ret = mhi_arch_pcie_init(mhi_cntrl);
	if (ret) {
		MHI_ERR("Error mhi_arch_pcie_init, ret:%d\n", ret);
		return ret;
	}

	ret = mhi_arch_iommu_init(mhi_cntrl);
	if (ret) {
		MHI_ERR("Error mhi_arch_iommu_init, ret:%d\n", ret);
		goto error_iommu_init;
	}

	ret = mhi_init_pci_dev(mhi_cntrl);
	if (ret) {
		MHI_ERR("Error mhi_init_pci_dev, ret:%d\n", ret);
		goto error_init_pci;
	}

	/* start power up sequence if not in debug mode */
	if (!mhi_dev->debug_mode) {
		ret = mhi_async_power_up(mhi_cntrl);
		if (ret) {
			MHI_ERR("Error mhi_async_power_up, ret:%d\n", ret);
			goto error_power_up;
		}
	}

#if 0
	pm_runtime_mark_last_busy(&pci_dev->dev);
	pm_runtime_allow(&pci_dev->dev);
	pm_runtime_disable(&pci_dev->dev);
#endif

	if (mhi_cntrl->dentry) {
		debugfs_create_file("m0", 0444, mhi_cntrl->dentry, mhi_cntrl,
				    &debugfs_trigger_m0_fops);
		debugfs_create_file("m3", 0444, mhi_cntrl->dentry, mhi_cntrl,
				    &debugfs_trigger_m3_fops);
	}

	dev_set_drvdata(&pci_dev->dev, mhi_cntrl);
	MHI_LOG("Return successful\n");

	return 0;

error_power_up:
	mhi_deinit_pci_dev(mhi_cntrl);

error_init_pci:
	mhi_arch_iommu_deinit(mhi_cntrl);

error_iommu_init:
	mhi_arch_pcie_deinit(mhi_cntrl);

	return ret;
}

static void mhi_pci_remove(struct pci_dev *pci_dev)
{
	struct mhi_controller *mhi_cntrl = (struct mhi_controller *)dev_get_drvdata(&pci_dev->dev);

	if (mhi_cntrl && mhi_cntrl->pci_dev == pci_dev) {
		struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
		MHI_LOG("%s\n", dev_name(&pci_dev->dev));
		if (!mhi_dev->debug_mode) {
			mhi_power_down(mhi_cntrl, 1);
		}
		mhi_deinit_pci_dev(mhi_cntrl);
		mhi_arch_iommu_deinit(mhi_cntrl);
		mhi_arch_pcie_deinit(mhi_cntrl);
		mhi_unregister_mhi_controller(mhi_cntrl);
	}
}

static const struct dev_pm_ops pm_ops = {
	SET_RUNTIME_PM_OPS(mhi_runtime_suspend,
			   mhi_runtime_resume,
			   mhi_runtime_idle)
	SET_SYSTEM_SLEEP_PM_OPS(mhi_system_suspend, mhi_system_resume)
};

static struct pci_driver mhi_pcie_driver = {
	.name = "mhi",
	.id_table = mhi_pcie_device_id,
	.probe = mhi_pci_probe,
	.remove = mhi_pci_remove,
	.driver = {
		.pm = &pm_ops
	}
};

int __init mhi_controller_qcom_init(void)
{	
	return pci_register_driver(&mhi_pcie_driver);
};

void mhi_controller_qcom_exit(void)
{
	pr_info("%s enter\n", __func__);
	pci_unregister_driver(&mhi_pcie_driver);
	pr_info("%s exit\n", __func__);
}
