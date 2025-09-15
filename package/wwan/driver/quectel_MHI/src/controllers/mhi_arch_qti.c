// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.*/

#include <linux/async.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/msm-bus.h>
#include <linux/msm_pcie.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include "../core/mhi.h"
#include "mhi_qti.h"

struct arch_info {
	struct mhi_dev *mhi_dev;
	struct msm_bus_scale_pdata *msm_bus_pdata;
	u32 bus_client;
	struct pci_saved_state *pcie_state;
	struct pci_saved_state *ref_pcie_state;
	struct dma_iommu_mapping *mapping;
};

struct mhi_bl_info {
	struct mhi_device *mhi_device;
	async_cookie_t cookie;
	void *ipc_log;
};

/* ipc log markings */
#define DLOG "Dev->Host: "
#define HLOG "Host: "

#ifdef CONFIG_MHI_DEBUG

#define MHI_IPC_LOG_PAGES (100)
enum MHI_DEBUG_LEVEL  mhi_ipc_log_lvl = MHI_MSG_LVL_VERBOSE;

#else

#define MHI_IPC_LOG_PAGES (10)
enum MHI_DEBUG_LEVEL  mhi_ipc_log_lvl = MHI_MSG_LVL_ERROR;

#endif

static int mhi_arch_set_bus_request(struct mhi_controller *mhi_cntrl, int index)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;

	MHI_LOG("Setting bus request to index %d\n", index);

	if (arch_info->bus_client)
		return msm_bus_scale_client_update_request(
							arch_info->bus_client,
							index);

	/* default return success */
	return 0;
}

static void mhi_bl_dl_cb(struct mhi_device *mhi_dev,
			 struct mhi_result *mhi_result)
{
	struct mhi_bl_info *mhi_bl_info = mhi_device_get_devdata(mhi_dev);
	char *buf = mhi_result->buf_addr;

	/* force a null at last character */
	buf[mhi_result->bytes_xferd - 1] = 0;

	ipc_log_string(mhi_bl_info->ipc_log, "%s %s", DLOG, buf);
}

static void mhi_bl_dummy_cb(struct mhi_device *mhi_dev,
			    struct mhi_result *mhi_result)
{
}

static void mhi_bl_remove(struct mhi_device *mhi_dev)
{
	struct mhi_bl_info *mhi_bl_info = mhi_device_get_devdata(mhi_dev);

	ipc_log_string(mhi_bl_info->ipc_log, HLOG "Received Remove notif.\n");

	/* wait for boot monitor to exit */
	async_synchronize_cookie(mhi_bl_info->cookie + 1);
}

static void mhi_bl_boot_monitor(void *data, async_cookie_t cookie)
{
	struct mhi_bl_info *mhi_bl_info = data;
	struct mhi_device *mhi_device = mhi_bl_info->mhi_device;
	struct mhi_controller *mhi_cntrl = mhi_device->mhi_cntrl;
	/* 15 sec timeout for booting device */
	const u32 timeout = msecs_to_jiffies(15000);

	/* wait for device to enter boot stage */
	wait_event_timeout(mhi_cntrl->state_event, mhi_cntrl->ee == MHI_EE_AMSS
			   || mhi_cntrl->ee == MHI_EE_DISABLE_TRANSITION,
			   timeout);

	if (mhi_cntrl->ee == MHI_EE_AMSS) {
		ipc_log_string(mhi_bl_info->ipc_log, HLOG
			       "Device successfully booted to mission mode\n");

		mhi_unprepare_from_transfer(mhi_device);
	} else {
		ipc_log_string(mhi_bl_info->ipc_log, HLOG
			       "Device failed to boot to mission mode, ee = %s\n",
			       TO_MHI_EXEC_STR(mhi_cntrl->ee));
	}
}

static int mhi_bl_probe(struct mhi_device *mhi_dev,
			const struct mhi_device_id *id)
{
	char node_name[32];
	struct mhi_bl_info *mhi_bl_info;

	mhi_bl_info = devm_kzalloc(&mhi_dev->dev, sizeof(*mhi_bl_info),
				   GFP_KERNEL);
	if (!mhi_bl_info)
		return -ENOMEM;

	snprintf(node_name, sizeof(node_name), "mhi_bl_%04x_%02u.%02u.%02u",
		 mhi_dev->dev_id, mhi_dev->domain, mhi_dev->bus, mhi_dev->slot);

	mhi_bl_info->ipc_log = ipc_log_context_create(MHI_IPC_LOG_PAGES,
						      node_name, 0);
	if (!mhi_bl_info->ipc_log)
		return -EINVAL;

	mhi_bl_info->mhi_device = mhi_dev;
	mhi_device_set_devdata(mhi_dev, mhi_bl_info);

	ipc_log_string(mhi_bl_info->ipc_log, HLOG
		       "Entered SBL, Session ID:0x%x\n",
		       mhi_dev->mhi_cntrl->session_id);

	/* start a thread to monitor entering mission mode */
	mhi_bl_info->cookie = async_schedule(mhi_bl_boot_monitor, mhi_bl_info);

	return 0;
}

static const struct mhi_device_id mhi_bl_match_table[] = {
	{ .chan = "BL" },
	{},
};

static struct mhi_driver mhi_bl_driver = {
	.id_table = mhi_bl_match_table,
	.remove = mhi_bl_remove,
	.probe = mhi_bl_probe,
	.ul_xfer_cb = mhi_bl_dummy_cb,
	.dl_xfer_cb = mhi_bl_dl_cb,
	.driver = {
		.name = "MHI_BL",
		.owner = THIS_MODULE,
	},
};

int mhi_arch_pcie_init(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;
	char node[32];

	if (!arch_info) {
		arch_info = devm_kzalloc(&mhi_dev->pci_dev->dev,
					 sizeof(*arch_info), GFP_KERNEL);
		if (!arch_info)
			return -ENOMEM;

		mhi_dev->arch_info = arch_info;

		snprintf(node, sizeof(node), "mhi_%04x_%02u.%02u.%02u",
			 mhi_cntrl->dev_id, mhi_cntrl->domain, mhi_cntrl->bus,
			 mhi_cntrl->slot);
		mhi_cntrl->log_buf = ipc_log_context_create(MHI_IPC_LOG_PAGES,
							    node, 0);
		mhi_cntrl->log_lvl = mhi_ipc_log_lvl;

		/* save reference state for pcie config space */
		arch_info->ref_pcie_state = pci_store_saved_state(
							mhi_dev->pci_dev);

		mhi_driver_register(&mhi_bl_driver);
	}

	return mhi_arch_set_bus_request(mhi_cntrl, 1);
}

void mhi_arch_pcie_deinit(struct mhi_controller *mhi_cntrl)
{
	mhi_arch_set_bus_request(mhi_cntrl, 0);
}

int mhi_arch_link_off(struct mhi_controller *mhi_cntrl, bool graceful)
{
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

	/* release the resources */
	mhi_arch_set_bus_request(mhi_cntrl, 0);

	MHI_LOG("Exited\n");

	return 0;
}

int mhi_arch_link_on(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);
	struct arch_info *arch_info = mhi_dev->arch_info;
	struct pci_dev *pci_dev = mhi_dev->pci_dev;
	int ret;

	MHI_LOG("Entered\n");

	/* request resources and establish link trainning */
	ret = mhi_arch_set_bus_request(mhi_cntrl, 1);
	if (ret)
		MHI_LOG("Could not set bus frequency, ret:%d\n", ret);

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

	return 0;
}
