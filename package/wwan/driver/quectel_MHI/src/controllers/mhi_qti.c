// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.*/

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/msi.h>
#include <linux/interrupt.h>
#define MAX_MHI 8
#ifdef CONFIG_PCI_MSM
#define QCOM_AP_AND_EFUSE_PCIE_SLEEP
#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
#include <linux/platform_device.h>
#include <linux/msm_pcie.h>
#endif
#endif
//#define QCOM_AP_SDM845_IOMMU_MAP
#ifdef QCOM_AP_SDM845_IOMMU_MAP
#include <linux/dma-mapping.h>
#include <asm/dma-iommu.h>
#include <linux/iommu.h>
#endif
#include "../core/mhi.h"
#include "../core/mhi_internal.h"
#include "mhi_qti.h"

#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
extern int pci_write_config_byte(const struct pci_dev *dev, int where, u8 val);
struct arch_info {
	struct mhi_dev *mhi_dev;
	struct msm_bus_scale_pdata *msm_bus_pdata;
	u32 bus_client;
	struct pci_saved_state *pcie_state;
	struct pci_saved_state *ref_pcie_state;
	struct dma_iommu_mapping *mapping;
};
#endif

#if 1
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,10,65 ))
static inline int dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	int rc = dma_set_mask(dev, mask);
	if (rc == 0)
		dma_set_coherent_mask(dev, mask);
	return rc;
}
#endif

#ifdef PCI_IRQ_NOMSIX
#define PCI_IRQ_MSI PCI_IRQ_NOMSIX
#endif

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
#if 0//defined(CONFIG_PINCTRL_IPQ5018)
	struct pcie_port *pp = dev->bus->sysdata;
	pp->msi[nr]; //msi maybe not continuous
#endif
    return dev->irq + nr;
}
#endif
#endif

struct firmware_info {
	unsigned int dev_id;
	const char *fw_image;
	const char *edl_image;
};

static const struct firmware_info firmware_table[] = {
	{.dev_id = 0x306, .fw_image = "sdx55m/sbl1.mbn"},
	{.dev_id = 0x305, .fw_image = "sdx50m/sbl1.mbn"},
	{.dev_id = 0x304, .fw_image = "sbl.mbn", .edl_image = "edl.mbn"},
	/* default, set to debug.mbn */
	{.fw_image = "debug.mbn"},
};

static int debug_mode;
module_param_named(debug_mode, debug_mode, int, 0644);

int mhi_debugfs_trigger_m0(void *data, u64 val)
{
	struct mhi_controller *mhi_cntrl = data;
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	MHI_LOG("Trigger M3 Exit\n");
	pm_runtime_get(&mhi_dev->pci_dev->dev);
	pm_runtime_put(&mhi_dev->pci_dev->dev);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debugfs_trigger_m0_fops, NULL,
			mhi_debugfs_trigger_m0, "%llu\n");

int mhi_debugfs_trigger_m3(void *data, u64 val)
{
	struct mhi_controller *mhi_cntrl = data;
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	MHI_LOG("Trigger M3 Entry\n");
	pm_runtime_mark_last_busy(&mhi_dev->pci_dev->dev);
	pm_request_autosuspend(&mhi_dev->pci_dev->dev);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debugfs_trigger_m3_fops, NULL,
			mhi_debugfs_trigger_m3, "%llu\n");

void mhi_deinit_pci_dev(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct pci_dev *pci_dev = mhi_dev->pci_dev;

	pm_runtime_mark_last_busy(&pci_dev->dev);
	pm_runtime_dont_use_autosuspend(&pci_dev->dev);
	pm_runtime_disable(&pci_dev->dev);
	pci_free_irq_vectors(pci_dev);
	kfree(mhi_cntrl->irq);
	mhi_cntrl->irq = NULL;
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
	resource_size_t len;
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

#if 1 //some SOC like rpi_4b need next codes
	ret = -EIO;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0))
	if (!dma_set_mask(&pci_dev->dev, DMA_BIT_MASK(64))) {
		ret =  dma_set_coherent_mask(&pci_dev->dev, DMA_BIT_MASK(64));
	} else if (!dma_set_mask(&pci_dev->dev, DMA_BIT_MASK(32))) {
		ret = dma_set_coherent_mask(&pci_dev->dev, DMA_BIT_MASK(32));
	}
#else
	if (!pci_set_dma_mask(pci_dev, DMA_BIT_MASK(64))) {
		ret = pci_set_consistent_dma_mask(pci_dev, DMA_BIT_MASK(64));
	} else if (!pci_set_dma_mask(pci_dev, DMA_BIT_MASK(32))) {
		ret = pci_set_consistent_dma_mask(pci_dev, DMA_BIT_MASK(32));
	}
#endif
	if (ret) {
		MHI_ERR("Error dma mask\n");
	}
#endif

	mhi_cntrl->base_addr = pci_resource_start(pci_dev, mhi_dev->resn);
	len = pci_resource_len(pci_dev, mhi_dev->resn);
#ifndef ioremap_nocache //4bdc0d676a643140bdf17dbf7eafedee3d496a3c
#define ioremap_nocache ioremap
#endif
	mhi_cntrl->regs = ioremap_nocache(mhi_cntrl->base_addr, len);
	if (!mhi_cntrl->regs) {
		MHI_ERR("Error ioremap region\n");
		goto error_ioremap;
	}

#if 0
	ret = pci_alloc_irq_vectors(pci_dev, mhi_cntrl->msi_required,
				    mhi_cntrl->msi_required, PCI_IRQ_NOMSIX);
	if (IS_ERR_VALUE((ulong)ret) || ret < mhi_cntrl->msi_required) {
		MHI_ERR("Failed to enable MSI, ret:%d\n", ret);
		goto error_req_msi;
	}
#else
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
	MHI_LOG("msi_required = %d, msi_allocated = %d, msi_irq = %u\n", mhi_cntrl->msi_required, ret, pci_dev->irq);
#endif

	mhi_cntrl->msi_allocated = ret;
	mhi_cntrl->irq = kmalloc_array(mhi_cntrl->msi_allocated,
				       sizeof(*mhi_cntrl->irq), GFP_KERNEL);
	if (!mhi_cntrl->irq) {
		ret = -ENOMEM;
		goto error_alloc_msi_vec;
	}

	for (i = 0; i < mhi_cntrl->msi_allocated; i++) {
		mhi_cntrl->irq[i] = pci_irq_vector(pci_dev, i);
		if (mhi_cntrl->irq[i] < 0) {
			ret = mhi_cntrl->irq[i];
			goto error_get_irq_vec;
		}
	}

	dev_set_drvdata(&pci_dev->dev, mhi_cntrl);

	/* configure runtime pm */
	pm_runtime_set_autosuspend_delay(&pci_dev->dev, MHI_RPM_SUSPEND_TMR_MS);
	pm_runtime_use_autosuspend(&pci_dev->dev);
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

	return 0;

error_get_irq_vec:
	kfree(mhi_cntrl->irq);
	mhi_cntrl->irq = NULL;

error_alloc_msi_vec:
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
static int mhi_runtime_suspend(struct device *dev)
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

	if (mhi_cntrl->ee != MHI_EE_AMSS) {
		MHI_LOG("Not AMSS, return busy\n");
		mutex_unlock(&mhi_cntrl->pm_mutex);
		return -EBUSY;
	}

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

static int mhi_runtime_idle(struct device *dev)
{
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);

	if ((mhi_cntrl->dev_state == MHI_STATE_M0 || mhi_cntrl->dev_state == MHI_STATE_M3)
		&& mhi_cntrl->ee == MHI_EE_AMSS) {
		return 0;
	}
	MHI_LOG("Entered returning -EBUSY, mhi_state:%s exec_env:%s\n",
		   TO_MHI_STATE_STR(mhi_get_mhi_state(mhi_cntrl)), TO_MHI_EXEC_STR(mhi_get_exec_env(mhi_cntrl)));

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
		//pm_runtime_set_active(dev);
		//pm_runtime_enable(dev);
	}

	return ret;
}

int mhi_system_suspend(struct device *dev)
{
	struct mhi_controller *mhi_cntrl = dev_get_drvdata(dev);
	int ret;

	MHI_LOG("Entered\n");

	if (atomic_read(&mhi_cntrl->pending_pkts)) {
		MHI_LOG("Abort due to pending_pkts:%d\n", atomic_read(&mhi_cntrl->pending_pkts));
		return -EBUSY;
	}

	/* if rpm status still active then force suspend */
	if (!pm_runtime_status_suspended(dev)) {
		ret = mhi_runtime_suspend(dev);
		if (ret) {
			MHI_LOG("suspend failed ret:%d\n", ret);
			return ret;
		}
	}

	//pm_runtime_set_suspended(dev);
	//pm_runtime_disable(dev);

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

/* disable PCIe L1 */
static int mhi_lpm_disable(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int lnkctl = pci_dev->pcie_cap + PCI_EXP_LNKCTL;
	u8 val;
	int ret;

	ret = pci_read_config_byte(pci_dev, lnkctl, &val);
	if (ret) {
		MHI_ERR("Error reading LNKCTL, ret:%d\n", ret);
		return ret;
	}

	/* L1 is not supported or already disabled */
	if (!(val & PCI_EXP_LNKCTL_ASPM_L1))
		return 0;

	val &= ~PCI_EXP_LNKCTL_ASPM_L1;
	ret = pci_write_config_byte(pci_dev, lnkctl, val);
	if (ret) {
		MHI_ERR("Error writing LNKCTL to disable LPM, ret:%d\n", ret);
		return ret;
	}

	mhi_dev->lpm_disabled = true;

	return ret;
}

/* enable PCIe L1 */
static int mhi_lpm_enable(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int lnkctl = pci_dev->pcie_cap + PCI_EXP_LNKCTL;
	u8 val;
	int ret;

	/* L1 is not supported or already disabled */
	if (!mhi_dev->lpm_disabled)
		return 0;

	ret = pci_read_config_byte(pci_dev, lnkctl, &val);
	if (ret) {
		MHI_ERR("Error reading LNKCTL, ret:%d\n", ret);
		return ret;
	}

	val |= PCI_EXP_LNKCTL_ASPM_L1;
	ret = pci_write_config_byte(pci_dev, lnkctl, val);
	if (ret) {
		MHI_ERR("Error writing LNKCTL to enable LPM, ret:%d\n", ret);
		return ret;
	}

	mhi_dev->lpm_disabled = false;

	return ret;
}

static int mhi_power_up(struct mhi_controller *mhi_cntrl)
{
	enum mhi_dev_state dev_state = mhi_get_mhi_state(mhi_cntrl);
	const u32 delayus = 10;
	int itr = DIV_ROUND_UP(mhi_cntrl->timeout_ms * 1000, delayus);
	int ret;

	MHI_LOG("dev_state:%s\n", TO_MHI_STATE_STR(mhi_get_mhi_state(mhi_cntrl)));

	/*
	 * It's possible device did not go thru a cold reset before
	 * power up and still in error state. If device in error state,
	 * we need to trigger a soft reset before continue with power
	 * up
	 */
	if (dev_state == MHI_STATE_SYS_ERR) {
		mhi_set_mhi_state(mhi_cntrl, MHI_STATE_RESET);
		while (itr--) {
			dev_state = mhi_get_mhi_state(mhi_cntrl);
			if (dev_state != MHI_STATE_SYS_ERR)
				break;
			usleep_range(delayus, delayus << 1);
		}
		MHI_LOG("dev_state:%s\n", TO_MHI_STATE_STR(mhi_get_mhi_state(mhi_cntrl)));

		/* device still in error state, abort power up */
		if (dev_state == MHI_STATE_SYS_ERR)
			return -EIO;
	}

	ret = mhi_async_power_up(mhi_cntrl);

	/* power up create the dentry */
	if (mhi_cntrl->dentry) {
		debugfs_create_file("m0", 0444, mhi_cntrl->dentry, mhi_cntrl,
				    &debugfs_trigger_m0_fops);
		debugfs_create_file("m3", 0444, mhi_cntrl->dentry, mhi_cntrl,
				    &debugfs_trigger_m3_fops);
	}

	return ret;
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

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put(dev);
}

static void mhi_runtime_mark_last_busy(struct mhi_controller *mhi_cntrl, void *priv)
{
	struct mhi_dev *mhi_dev = priv;
	struct device *dev = &mhi_dev->pci_dev->dev;

	pm_runtime_mark_last_busy(dev);
}

#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
static void mhi_pci_event_cb(struct msm_pcie_notify *notify)
{
	struct pci_dev *pci_dev = notify->user;
	struct device *dev = &pci_dev->dev;

	dev_info(&pci_dev->dev, "Received PCIe event %d", notify->event);
	switch (notify->event) {
	case MSM_PCIE_EVENT_WAKEUP:
	if (dev && pm_runtime_status_suspended(dev)) {
		pm_request_resume(dev);
		pm_runtime_mark_last_busy(dev);
	}
	break;
	default:
	break;
	}
}

static struct msm_pcie_register_event mhi_pcie_events[MAX_MHI];
#endif

static void mhi_status_cb(struct mhi_controller *mhi_cntrl,
			  void *priv,
			  enum MHI_CB reason)
{
	struct mhi_dev *mhi_dev = priv;
	struct device *dev = &mhi_dev->pci_dev->dev;

	switch (reason) {
	case MHI_CB_FATAL_ERROR:
	case MHI_CB_SYS_ERROR:
		pm_runtime_forbid(dev);
		break;
	case MHI_CB_EE_MISSION_MODE:
		//pm_runtime_allow(dev);
		break;
	default:
		break;
	}
}

/* capture host SoC XO time in ticks */
static u64 mhi_time_get(struct mhi_controller *mhi_cntrl, void *priv)
{
	return 0;
}

static ssize_t timeout_ms_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;

	/* buffer provided by sysfs has a minimum size of PAGE_SIZE */
	return snprintf(buf, PAGE_SIZE, "%u\n", mhi_cntrl->timeout_ms);
}

static ssize_t timeout_ms_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	u32 timeout_ms;

	if (kstrtou32(buf, 0, &timeout_ms) < 0)
		return -EINVAL;

	mhi_cntrl->timeout_ms = timeout_ms;

	return count;
}
static DEVICE_ATTR_RW(timeout_ms);

static ssize_t power_up_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t count)
{
	int ret;
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;

	ret = mhi_power_up(mhi_cntrl);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR_WO(power_up);

static struct attribute *mhi_attrs[] = {
	&dev_attr_timeout_ms.attr,
	&dev_attr_power_up.attr,
	NULL
};

static const struct attribute_group mhi_group = {
	.attrs = mhi_attrs,
};

static struct mhi_controller *mhi_register_controller(struct pci_dev *pci_dev)
{
	struct mhi_controller *mhi_cntrl;
	struct mhi_dev *mhi_dev;
	struct device_node *of_node = pci_dev->dev.of_node;
	const struct firmware_info *firmware_info;
	bool use_bb;
	u64 addr_win[2];
	int ret, i;

	//if (!of_node)
	//	return ERR_PTR(-ENODEV);

	mhi_cntrl = mhi_alloc_controller(sizeof(*mhi_dev));
	if (!mhi_cntrl)
		return ERR_PTR(-ENOMEM);

	mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	mhi_cntrl->dev = &pci_dev->dev;
	mhi_cntrl->domain = pci_domain_nr(pci_dev->bus);
	mhi_cntrl->vendor = pci_dev->vendor;
	mhi_cntrl->dev_id = pci_dev->device;
	mhi_cntrl->bus = pci_dev->bus->number;
	mhi_cntrl->slot = PCI_SLOT(pci_dev->devfn);

#if 0
	use_bb = of_property_read_bool(of_node, "mhi,use-bb");

	/*
	 * if s1 translation enabled or using bounce buffer pull iova addr
	 * from dt
	 */
	if (use_bb || (mhi_dev->smmu_cfg & MHI_SMMU_ATTACH &&
		       !(mhi_dev->smmu_cfg & MHI_SMMU_S1_BYPASS))) {
		ret = of_property_count_elems_of_size(of_node, "qti,addr-win",
						      sizeof(addr_win));
		if (ret != 1)
			goto error_register;
		ret = of_property_read_u64_array(of_node, "qti,addr-win",
						 addr_win, 2);
		if (ret)
			goto error_register;
	} else {
		addr_win[0] = memblock_start_of_DRAM();
		addr_win[1] = memblock_end_of_DRAM();
	}
#else
	use_bb = false;
	(void)use_bb;
	addr_win[0] = 0x000000000;
	addr_win[1] = 0x2000000000; //MHI_MEM_SIZE_DEFAULT
	if (sizeof(dma_addr_t) == 4) {
		addr_win[1] = 0xFFFFFFFF;
	}
#endif

	mhi_dev->iova_start = addr_win[0];
	mhi_dev->iova_stop = addr_win[1];

	/*
	 * If S1 is enabled, set MHI_CTRL start address to 0 so we can use low
	 * level mapping api to map buffers outside of smmu domain
	 */
	if (mhi_dev->smmu_cfg & MHI_SMMU_ATTACH &&
	    !(mhi_dev->smmu_cfg & MHI_SMMU_S1_BYPASS))
		mhi_cntrl->iova_start = 0;
	else
		mhi_cntrl->iova_start = addr_win[0];

	mhi_cntrl->iova_stop = mhi_dev->iova_stop;
	mhi_cntrl->of_node = of_node;

	mhi_dev->pci_dev = pci_dev;

	/* setup power management apis */
	mhi_cntrl->status_cb = mhi_status_cb;
	mhi_cntrl->runtime_get = mhi_runtime_get;
	mhi_cntrl->runtime_put = mhi_runtime_put;
	mhi_cntrl->runtime_mark_last_busy = mhi_runtime_mark_last_busy;
	mhi_cntrl->link_status = mhi_link_status;

	mhi_cntrl->lpm_disable = mhi_lpm_disable;
	mhi_cntrl->lpm_enable = mhi_lpm_enable;
	mhi_cntrl->time_get = mhi_time_get;

	ret = of_register_mhi_controller(mhi_cntrl);
	if (ret)
		goto error_register;

	for (i = 0; i < ARRAY_SIZE(firmware_table); i++) {
		firmware_info = firmware_table + i;

		/* debug mode always use default */
		if (!debug_mode && mhi_cntrl->dev_id == firmware_info->dev_id)
			break;
	}

#if 0
	mhi_cntrl->fw_image = firmware_info->fw_image;
	mhi_cntrl->edl_image = firmware_info->edl_image;
#endif

	if (sysfs_create_group(&mhi_cntrl->mhi_dev->dev.kobj, &mhi_group))
		MHI_ERR("Error while creating the sysfs group\n");

	return mhi_cntrl;

error_register:
	mhi_free_controller(mhi_cntrl);

	return ERR_PTR(-EINVAL);
}

static bool mhi_pci_is_alive(struct pci_dev *pdev)
{
	u16 vendor = 0;

	if (pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor))
		return false;

	if (vendor == (u16) ~0 || vendor == 0)
		return false;

	return true;
}

static void mhi_pci_show_link(struct mhi_controller *mhi_cntrl, struct pci_dev *pci_dev)
{
	int pcie_cap_reg;
	u16 stat;
	u32 caps;
	const char *speed;

	pcie_cap_reg = pci_find_capability(pci_dev, PCI_CAP_ID_EXP);

	if (!pcie_cap_reg)
		return;

	pci_read_config_word(pci_dev,
			     pcie_cap_reg + PCI_EXP_LNKSTA,
			     &stat);
	pci_read_config_dword(pci_dev,
			      pcie_cap_reg + PCI_EXP_LNKCAP,
			      &caps);

	switch (caps & PCI_EXP_LNKCAP_SLS) {
		case PCI_EXP_LNKCAP_SLS_2_5GB: speed = "2.5"; break;
		case PCI_EXP_LNKCAP_SLS_5_0GB: speed = "5"; break;
		case 3: speed = "8"; break;
		case 4: speed = "16"; break;
		case 5: speed = "32"; break;
		case 6: speed = "64"; break;
		default: speed = "0"; break;
	}

	MHI_LOG("LnkCap:	Speed %sGT/s, Width x%d\n", speed,
		(caps & PCI_EXP_LNKCAP_MLW) >> 4);

	switch (stat & PCI_EXP_LNKSTA_CLS) {
		case PCI_EXP_LNKSTA_CLS_2_5GB: speed = "2.5"; break;
		case PCI_EXP_LNKSTA_CLS_5_0GB: speed = "5"; break;
		case 3: speed = "8"; break;
		case 4: speed = "16"; break;
		case 5: speed = "32"; break;
		case 6: speed = "64"; break;
		default: speed = "0"; break;
	}

	MHI_LOG("LnkSta:	Speed %sGT/s, Width x%d\n", speed,
		(stat & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT);

}

int mhi_pci_probe(struct pci_dev *pci_dev,
		  const struct pci_device_id *device_id)
{
	struct mhi_controller *mhi_cntrl;
	u32 domain = pci_domain_nr(pci_dev->bus);
	u32 bus = pci_dev->bus->number;
	u32 dev_id = pci_dev->device;
	u32 slot = PCI_SLOT(pci_dev->devfn);
	struct mhi_dev *mhi_dev;
	int ret;

	pr_info("%s pci_dev->name = %s, domain=%d, bus=%d, slot=%d, vendor=%04X, device=%04X\n",
		__func__, dev_name(&pci_dev->dev), domain, bus, slot, pci_dev->vendor, pci_dev->device);

#if !defined(CONFIG_PCI_MSI)
        /* MT7621 RTL8198D EcoNet-EN7565 */
	#error "pcie msi is not support by this soc! and i donot support INTx (SW1SDX55-2688)"
#endif

	if (!mhi_pci_is_alive(pci_dev)) {
		/*
		root@OpenWrt:~# hexdump /sys/bus/pci/devices/0000:01:00.0/config
		0000000 ffff ffff ffff ffff ffff ffff ffff ffff
		*
		0001000
		*/
		pr_err("mhi_pci is not alive! pcie link is down\n");
		pr_err("double check by 'hexdump /sys/bus/pci/devices/%s/config'\n", dev_name(&pci_dev->dev));
		return -EIO;
	}

	/* see if we already registered */
	mhi_cntrl = mhi_bdf_to_controller(domain, bus, slot, dev_id);
	if (!mhi_cntrl)
		mhi_cntrl = mhi_register_controller(pci_dev);

	if (IS_ERR(mhi_cntrl))
		return PTR_ERR(mhi_cntrl);

	mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	mhi_dev->powered_on = true;

	mhi_arch_iommu_init(mhi_cntrl);

	ret = mhi_arch_pcie_init(mhi_cntrl);
	if (ret)
		goto error_init_pci_arch;

	mhi_cntrl->dev = &pci_dev->dev;
	ret = mhi_init_pci_dev(mhi_cntrl);
	if (ret)
		goto error_init_pci;

	/* start power up sequence */
	if (!debug_mode) {
		ret = mhi_power_up(mhi_cntrl);
		if (ret)
			goto error_power_up;
	}

	pm_runtime_mark_last_busy(&pci_dev->dev);

	mhi_pci_show_link(mhi_cntrl, pci_dev);

#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
	{
		struct msm_pcie_register_event *pcie_event = &mhi_pcie_events[mhi_cntrl->cntrl_idx];

		pcie_event->events = MSM_PCIE_EVENT_WAKEUP;
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,14,117 ))
		pcie_event->pcie_event.user = pci_dev;
		pcie_event->pcie_event.mode = MSM_PCIE_TRIGGER_CALLBACK;
		pcie_event->pcie_event.callback = mhi_pci_event_cb;
#else
		pcie_event->user = pci_dev;
		pcie_event->mode = MSM_PCIE_TRIGGER_CALLBACK;
		pcie_event->callback = mhi_pci_event_cb;
#endif

		ret = msm_pcie_register_event(pcie_event);
		if (ret) {
			MHI_LOG("Failed to register for PCIe event");
		}
	}
#endif

	MHI_LOG("Return successful\n");

	return 0;

	mhi_unregister_mhi_controller(mhi_cntrl);
error_power_up:
	mhi_deinit_pci_dev(mhi_cntrl);

error_init_pci:
	mhi_arch_pcie_deinit(mhi_cntrl);
error_init_pci_arch:
	mhi_arch_iommu_deinit(mhi_cntrl);

	return ret;
}

void mhi_pci_device_removed(struct pci_dev *pci_dev)
{
	struct mhi_controller *mhi_cntrl;
	u32 domain = pci_domain_nr(pci_dev->bus);
	u32 bus = pci_dev->bus->number;
	u32 dev_id = pci_dev->device;
	u32 slot = PCI_SLOT(pci_dev->devfn);

	mhi_cntrl = mhi_bdf_to_controller(domain, bus, slot, dev_id);

	if (mhi_cntrl) {

		struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
		{
			struct msm_pcie_register_event *pcie_event = &mhi_pcie_events[mhi_cntrl->cntrl_idx];

			msm_pcie_deregister_event(pcie_event);
		}
#endif

		pm_stay_awake(&mhi_cntrl->mhi_dev->dev);

		/* if link is in drv suspend, wake it up */
		pm_runtime_get_sync(&pci_dev->dev);

		mutex_lock(&mhi_cntrl->pm_mutex);
		if (!mhi_dev->powered_on) {
			MHI_LOG("Not in active state\n");
			mutex_unlock(&mhi_cntrl->pm_mutex);
			pm_runtime_put_noidle(&pci_dev->dev);
			return;
		}
		mhi_dev->powered_on = false;
		mutex_unlock(&mhi_cntrl->pm_mutex);

		pm_runtime_put_noidle(&pci_dev->dev);

		MHI_LOG("Triggering shutdown process\n");
		mhi_power_down(mhi_cntrl, false);

		/* turn the link off */
		mhi_deinit_pci_dev(mhi_cntrl);
		mhi_arch_link_off(mhi_cntrl, false);

		mhi_arch_pcie_deinit(mhi_cntrl);
		mhi_arch_iommu_deinit(mhi_cntrl);

		pm_relax(&mhi_cntrl->mhi_dev->dev);

		mhi_unregister_mhi_controller(mhi_cntrl);
	}
}

static const struct dev_pm_ops pm_ops = {
	SET_RUNTIME_PM_OPS(mhi_runtime_suspend,
			   mhi_runtime_resume,
			   mhi_runtime_idle)
	SET_SYSTEM_SLEEP_PM_OPS(mhi_system_suspend, mhi_system_resume)
};

static struct pci_device_id mhi_pcie_device_id[] = {
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0303)},
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0304)}, //SDX20
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0305)}, //SDX24
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0306)}, //SDX55
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, 0x0308)}, //SDX62
	{PCI_DEVICE(0x1eac, 0x1001)}, //EM120
	{PCI_DEVICE(0x1eac, 0x1002)}, //EM160
	{PCI_DEVICE(0x1eac, 0x1004)}, //RM520
	{PCI_DEVICE(MHI_PCIE_VENDOR_ID, MHI_PCIE_DEBUG_ID)},
	{0},
};

MODULE_DEVICE_TABLE(pci, mhi_pcie_device_id);

static struct pci_driver mhi_pcie_driver = {
	.name = "mhi_q",
	.id_table = mhi_pcie_device_id,
	.probe = mhi_pci_probe,
	.remove = mhi_pci_device_removed,
	.driver = {
		.pm = &pm_ops
	}
};

#if 0
module_pci_driver(mhi_pcie_driver);
#else
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

#ifdef QCOM_AP_SDM845_IOMMU_MAP
struct dma_iommu_mapping *mhi_smmu_mapping[MAX_MHI];

#define SMMU_BASE 0x10000000
#define SMMU_SIZE  0x40000000
static struct dma_iommu_mapping * sdm845_smmu_init(struct pci_dev *pdev) {
	int ret = 0;
	int atomic_ctx = 1;
	int s1_bypass = 1;
	struct dma_iommu_mapping *mapping;

	mapping = arm_iommu_create_mapping(&platform_bus_type, SMMU_BASE, SMMU_SIZE);
	if (IS_ERR(mapping)) {
		ret = PTR_ERR(mapping);
		dev_err(&pdev->dev, "Create mapping failed, err = %d\n", ret);
		return NULL;
	}

	ret = iommu_domain_set_attr(mapping->domain, DOMAIN_ATTR_ATOMIC, &atomic_ctx);
	if (ret < 0) {
		dev_err(&pdev->dev, "Set atomic_ctx attribute failed, err = %d\n", ret);
		goto set_attr_fail;
	}

	ret = iommu_domain_set_attr(mapping->domain, DOMAIN_ATTR_S1_BYPASS, &s1_bypass);
	if (ret < 0) {
		dev_err(&pdev->dev, "Set s1_bypass attribute failed, err = %d\n", ret);
		arm_iommu_release_mapping(mapping);
		goto set_attr_fail;
	}

	ret = arm_iommu_attach_device(&pdev->dev, mapping);
		if (ret < 0) {
		dev_err(&pdev->dev, "Attach device failed, err = %d\n", ret);
		goto attach_fail;
	}

	return mapping;

attach_fail:
set_attr_fail:
	arm_iommu_release_mapping(mapping);
	return NULL;    
}
#endif

int mhi_arch_iommu_init(struct mhi_controller *mhi_cntrl)
{
#ifdef QCOM_AP_SDM845_IOMMU_MAP
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	mhi_smmu_mapping[mhi_cntrl->cntrl_idx] = sdm845_smmu_init(mhi_dev->pci_dev);
#endif

    return 0;
}

void mhi_arch_iommu_deinit(struct mhi_controller *mhi_cntrl)
{
#ifdef QCOM_AP_SDM845_IOMMU_MAP
	if (mhi_smmu_mapping[mhi_cntrl->cntrl_idx]) {
		struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

		arm_iommu_detach_device(&mhi_dev->pci_dev->dev);
		arm_iommu_release_mapping(mhi_smmu_mapping[mhi_cntrl->cntrl_idx]);
		mhi_smmu_mapping[mhi_cntrl->cntrl_idx] = NULL;
	}
#endif
}

static int mhi_arch_set_bus_request(struct mhi_controller *mhi_cntrl, int index)
{
	MHI_LOG("Setting bus request to index %d\n", index);
	return 0;
}

int mhi_arch_pcie_init(struct mhi_controller *mhi_cntrl)
{
#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;

	if (!arch_info) {
		arch_info = devm_kzalloc(&mhi_dev->pci_dev->dev,
					 sizeof(*arch_info), GFP_KERNEL);
		if (!arch_info)
			return -ENOMEM;

		mhi_dev->arch_info = arch_info;

		/* save reference state for pcie config space */
		arch_info->ref_pcie_state = pci_store_saved_state(
							mhi_dev->pci_dev);
	}
#endif

	return mhi_arch_set_bus_request(mhi_cntrl, 1);
}

void mhi_arch_pcie_deinit(struct mhi_controller *mhi_cntrl)
{
	mhi_arch_set_bus_request(mhi_cntrl, 0);
}

int mhi_arch_platform_init(struct mhi_dev *mhi_dev)
{
	return 0;
}

void mhi_arch_platform_deinit(struct mhi_dev *mhi_dev)
{
}

int mhi_arch_link_off(struct mhi_controller *mhi_cntrl,
				    bool graceful)
{
#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int ret;

	MHI_LOG("Entered\n");

	if (graceful) {
		pci_clear_master(pci_dev);
		ret = pci_save_state(mhi_dev->pci_dev);
		if (ret) {
			MHI_ERR("Failed with pci_save_state, ret:%d\n", ret);
			return ret;
		}

		arch_info->pcie_state = pci_store_saved_state(pci_dev);
		pci_disable_device(pci_dev);
	}

	/*
	 * We will always attempt to put link into D3hot, however
	 * link down may have happened due to error fatal, so
	 * ignoring the return code
	 */
	pci_set_power_state(pci_dev, PCI_D3hot);

	ret = msm_pcie_pm_control(MSM_PCIE_SUSPEND, mhi_cntrl->bus, pci_dev,
				    NULL, 0);
	MHI_ERR("msm_pcie_pm_control(MSM_PCIE_SUSPEND), ret:%d\n", ret);

	/* release the resources */
	mhi_arch_set_bus_request(mhi_cntrl, 0);

	MHI_LOG("Exited\n");
#endif

	return 0;
}

int mhi_arch_link_on(struct mhi_controller *mhi_cntrl)
{
#ifdef QCOM_AP_AND_EFUSE_PCIE_SLEEP
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int ret;

	MHI_LOG("Entered\n");

	/* request resources and establish link trainning */
	ret = mhi_arch_set_bus_request(mhi_cntrl, 1);
	if (ret)
		MHI_LOG("Could not set bus frequency, ret:%d\n", ret);

	ret = msm_pcie_pm_control(MSM_PCIE_RESUME, mhi_cntrl->bus, pci_dev,
				  NULL, 0);
	MHI_LOG("msm_pcie_pm_control(MSM_PCIE_RESUME), ret:%d\n", ret);
	if (ret) {
		MHI_ERR("Link training failed, ret:%d\n", ret);
		return ret;
	}

	ret = pci_set_power_state(pci_dev, PCI_D0);
	if (ret) {
		MHI_ERR("Failed to set PCI_D0 state, ret:%d\n", ret);
		return ret;
	}

	ret = pci_enable_device(pci_dev);
	if (ret) {
		MHI_ERR("Failed to enable device, ret:%d\n", ret);
		return ret;
	}

	ret = pci_load_and_free_saved_state(pci_dev, &arch_info->pcie_state);
	if (ret)
		MHI_LOG("Failed to load saved cfg state\n");

	pci_restore_state(pci_dev);
	pci_set_master(pci_dev);

	MHI_LOG("Exited\n");
#endif

	return 0;
}
#endif
