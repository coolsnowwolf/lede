// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/current.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include "phytium_npu.h"
#include "phytium_npu_mmu.h"

int global_session_id;

struct phytium_npu_session *phytium_npu_session_create(struct device *dev)
{
	return devm_kzalloc(dev, sizeof(struct phytium_npu_session), GFP_KERNEL);
}

int phytium_npu_session_init(struct phytium_npu_dev *npu, struct phytium_npu_session *session)
{
	size_t i;
	struct phytium_npu_mmu_context *pnmctx;

	session->npu_dev = npu;

	if (global_session_id >= NPU_SUPPORT_SESSION_NUM)
		global_session_id = 0;

	session->id = global_session_id++;
	session->dbgfs.sess_dbgfs_dir = NULL;

	init_waitqueue_head(&session->response_wq);
	INIT_LIST_HEAD(&session->sess_list_entry);
	INIT_LIST_HEAD(&session->sched_list_entry);
	INIT_LIST_HEAD(&session->stream_list);
	INIT_LIST_HEAD(&session->response_list);
	list_add_tail(&session->sess_list_entry, &npu->sessions_list);
	list_add(&session->sched_list_entry, &npu->sched_sess_list);

	for (i = 0; i < ARRAY_SIZE(session->mmu_ctx); i++) {
		pnmctx = &session->mmu_ctx[i];
		pnmctx->mctx = NULL;
		pnmctx->dev = NULL;
	}
	npu->sessions_count++;
	pr_debug("current npu session count %d, session id:%d", npu->sessions_count, session->id);
	return 0;
}

int phytium_npu_session_release(struct phytium_npu_dev *npu, struct phytium_npu_session *session)
{
	struct phytium_npu_session *curr, *next;

	if (!npu || !session)
		return -EINVAL;
	phytium_npu_debugfs_session_remove(session);
	phytium_npu_release_mmu_context(npu, session);
	list_for_each_entry_safe(curr, next, &npu->sessions_list, sess_list_entry) {
		if (curr == session) {
			list_del(&curr->sess_list_entry);
			list_del(&curr->sched_list_entry);
			npu->sessions_count--;
			pr_debug("release session :%p", session);
			devm_kfree(npu->dev, curr);
			break;
		}
	}

	return 0;
}

int phytium_npu_session_release_all(struct phytium_npu_dev *npudev)
{
	struct phytium_npu_session *curr, *next;

	list_for_each_entry_safe(curr, next, &npudev->sessions_list, sess_list_entry) {
		phytium_npu_release_mmu_context(npudev, curr);
		list_del(&curr->sess_list_entry);
		list_del(&curr->sched_list_entry);
		npudev->sessions_count--;
		devm_kfree(npudev->dev, curr);
	}
	return 0;
}

void phytium_npu_change_schedule_session(struct phytium_npu_session *sess,
					 struct phytium_npu_dev *npu, int is_change)
{
	if (sess != list_entry(&npu->sched_sess_list, struct phytium_npu_session, sched_list_entry))
		while (list_first_entry(&npu->sched_sess_list,
					struct phytium_npu_session, sched_list_entry) != sess)
			list_rotate_left(&npu->sched_sess_list);

	if (is_change)
		list_rotate_left(&npu->sched_sess_list);
}
