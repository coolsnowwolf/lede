/*
 * Copyright (C) 2018-2019 Unisoc Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/completion.h>

#include "../include/sipa.h"
#include "../include/sipc.h"
#include "../include/sprd_pcie_ep_device.h"
#include "sipa_core.h"

#define SIPA_PCIE_DL_CMN_FIFO_REG_OFFSET 0x980
#define SIPA_PCIE_UL_CMN_FIFO_REG_OFFSET 0x200

static int sipa_dele_start_req_work(void)
{
	struct smsg msg;

	msg.channel = SMSG_CH_COMM_SIPA;
	msg.type = SMSG_TYPE_CMD;
	msg.flag = SMSG_FLG_DELE_REQUEST;
	msg.value = 0;

	return smsg_send(SIPC_ID_MINIAP, &msg, -1);
}

static int sipa_init_cmn_fifo_reg_addr(struct sipa_core *ipa)
{
	ipa->reg_mapped = sprd_ep_ipa_map(PCIE_IPA_TYPE_REG,
					  ipa->reg_res->start,
					  resource_size(ipa->reg_res));
#ifndef devm_ioremap_nocache
#define devm_ioremap_nocache devm_ioremap
#endif
	ipa->virt_reg_addr = devm_ioremap_nocache(ipa->dev,
						  (resource_size_t)ipa->reg_mapped,
						  (resource_size_t)(resource_size(ipa->reg_res)));
	if (!ipa->virt_reg_addr) {
		dev_err(ipa->dev, "ipa reg base remap fail\n");
		return -ENOMEM;
	}

	ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL].fifo_reg_base =
		ipa->virt_reg_addr + SIPA_PCIE_DL_CMN_FIFO_REG_OFFSET;
	ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL].fifo_reg_base =
		ipa->virt_reg_addr + SIPA_PCIE_UL_CMN_FIFO_REG_OFFSET;

	return 0;
}

static int conn_thread(void *data)
{
	struct smsg mrecv;
	int ret, timeout = 500;
	struct sipa_core *ipa = data;

	/* since the channel open may hang, we call it in the thread context */
	ret = smsg_ch_open(SIPC_ID_MINIAP, SMSG_CH_COMM_SIPA, -1);
	if (ret != 0) {
		dev_err(ipa->dev, "sipa_delegator failed to open dst %d channel %d\n",
			SIPC_ID_MINIAP, SMSG_CH_COMM_SIPA);
		/* assign NULL to thread poniter as failed to open channel */
		return ret;
	}

	while (sipa_dele_start_req_work() && timeout--)
		usleep_range(5000, 10000);

	/* start listen the smsg events */
	while (!kthread_should_stop()) {
		/* monitor seblock recv smsg */
		smsg_set(&mrecv, SMSG_CH_COMM_SIPA, 0, 0, 0);
		ret = smsg_recv(SIPC_ID_MINIAP, &mrecv, -1);
		if (ret == -EIO || ret == -ENODEV) {
			/* channel state is FREE */
			usleep_range(5000, 10000);
			continue;
		}

		dev_dbg(ipa->dev, "sipa type=%d, flag=0x%x, value=0x%08x\n",
			mrecv.type, mrecv.flag, mrecv.value);

		switch (mrecv.type) {
		case SMSG_TYPE_OPEN:
			/* just ack open */
			smsg_open_ack(SIPC_ID_AP, SMSG_CH_COMM_SIPA);
			break;
		case SMSG_TYPE_CLOSE:
			/* handle channel close */
			smsg_close_ack(SIPC_ID_AP, SMSG_CH_COMM_SIPA);
			break;
		case SMSG_TYPE_CMD:
			/* handle commads */
			break;
		case SMSG_TYPE_DONE:
			sipa_init_cmn_fifo_reg_addr(ipa);
			dev_info(ipa->dev, "remote ipa ready reg_mapped = 0x%llx\n", (long long unsigned int)ipa->reg_mapped);
			sipa_receiver_open_cmn_fifo(ipa->receiver);
			sipa_sender_open_cmn_fifo(ipa->sender);
			sipa_nic_check_flow_ctrl();
			ipa->remote_ready = true;
			/* handle cmd done */
			break;
		case SMSG_TYPE_EVENT:
			/* handle events */
			break;
		default:
			ret = 1;
			break;
		};

		if (ret) {
			dev_info(ipa->dev, "unknown msg in conn_thrd: %d, %d, %d\n",
				 mrecv.type, mrecv.flag, mrecv.value);
			ret = 0;
		}
	}

	return ret;
}

int sipa_create_smsg_channel(struct sipa_core *ipa)
{
	/* create channel thread for this seblock channel */
	ipa->smsg_thread = kthread_create(conn_thread, ipa, "sipa-dele");
	if (IS_ERR(ipa->smsg_thread)) {
		dev_err(ipa->dev, "Failed to create monitor smsg kthread\n");
		return PTR_ERR(ipa->smsg_thread);
	}

	wake_up_process(ipa->smsg_thread);

	return 0;
}
EXPORT_SYMBOL(sipa_create_smsg_channel);
