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
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/soc/sprd/sprd_mpm.h>
#include <linux/workqueue.h>

#include "pcie_sipa_res.h"
#include "../include/sprd_pcie_resource.h"

struct pcie_sipa_res_prod {
	u8  dst;
	enum sipa_rm_res_id	prod_id; /* producer res id */
	enum sipa_rm_res_id	cons_id; /* consumer res id */
	struct sprd_pms		*pms;
	char			pms_name[20];
	struct work_struct	wait_work;
	struct delayed_work	rm_work;
};

static void pcie_sipa_res_wait_res_work_fn(struct work_struct *work)
{
	int ret;
	struct pcie_sipa_res_prod *res = container_of(work,
						      struct pcie_sipa_res_prod,
						      wait_work);

	ret = sprd_pcie_wait_resource(res->dst, -1);

	/* pcie not ready, just return. */
	if (ret) {
		pr_err("pcie_sipa_res: wait res error = %d!\n", ret);
		return;
	}

	/* notify ipa module  that pcie is ready. */
	sipa_rm_notify_completion(SIPA_RM_EVT_GRANTED,
			  res->prod_id);
}

static int pcie_sipa_res_request_resource(void *data)
{
	int ret;
	struct pcie_sipa_res_prod *res = data;

	pr_info("pcie_sipa_res: request resource.\n");

	sprd_pms_power_up(res->pms);

	/*
	 * when the resource is not ready, the IPA module doesn't want be
	 * blocked in here until the pcie ready, the IPA owner designed
	 * a notification api sipa_rm_notify_completion to notify the
	 * IPA module that the resource requested by IPA is ready.
	 * The designated error value is -EINPROGRESS, so we must override the
	 * return value -ETIME to -EINPROGRESS.
	 */
	ret = sprd_pcie_wait_resource(res->dst, 0);

	if (ret == -ETIME) {
		/* add a work to wait pcie ready */
		schedule_work(&res->wait_work);
		ret = -EINPROGRESS;
	}

	return ret;
}

static int pcie_sipa_res_release_resource(void *data)
{
	struct pcie_sipa_res_prod *res = data;

	pr_info("pcie_sipa_res: release resource.\n");

	sprd_pms_release_resource(res->pms);

	return 0;
}

static void pcie_sipa_res_create_rm_work_fn(struct work_struct *work)
{
	int ret;
	struct sipa_rm_create_params rm_params;
	struct pcie_sipa_res_prod *res = container_of(to_delayed_work(work),
						      struct pcie_sipa_res_prod,
						      rm_work);

	rm_params.name = res->prod_id;
	rm_params.floor_voltage = 0;
	rm_params.reg_params.notify_cb = NULL;
	rm_params.reg_params.user_data = res;
	rm_params.request_resource = pcie_sipa_res_request_resource;
	rm_params.release_resource = pcie_sipa_res_release_resource;
	ret = sipa_rm_create_resource(&rm_params);

	/* defer to create rm */
	if (ret == -EPROBE_DEFER) {
		schedule_delayed_work(&res->rm_work, msecs_to_jiffies(1000));
		return;
	}

	/* add dependencys */
	ret = sipa_rm_add_dependency(res->cons_id, res->prod_id);
	if (ret < 0 && ret != -EINPROGRESS) {
		pr_err("pcie_sipa_res: add_dependency error = %d!\n", ret);
		sipa_rm_delete_resource(res->prod_id);
		sprd_pms_destroy(res->pms);
		kfree(res);
	}
}

void *pcie_sipa_res_create(u8 dst, enum sipa_rm_res_id prod_id,
			   enum sipa_rm_res_id cons_id)
{
	int ret;
	struct sipa_rm_create_params rm_params;
	struct pcie_sipa_res_prod *res;

	res = kzalloc(sizeof(*res), GFP_KERNEL);
	if (!res)
		return NULL;

	/* init wait pcie res work */
	INIT_WORK(&res->wait_work, pcie_sipa_res_wait_res_work_fn);
	INIT_DELAYED_WORK(&res->rm_work, pcie_sipa_res_create_rm_work_fn);

	/* create pms */
	strncpy(res->pms_name, "sipa", sizeof(res->pms_name));
	res->pms = sprd_pms_create(dst, res->pms_name, false);
	if (!res->pms) {
		pr_err("pcie_sipa_res: create pms failed!\n");
		kfree(res);
		return NULL;
	}

	res->dst = dst;
	res->prod_id = prod_id;
	res->cons_id = cons_id;

	/* create prod */
	rm_params.name = prod_id;
	rm_params.floor_voltage = 0;
	rm_params.reg_params.notify_cb = NULL;
	rm_params.reg_params.user_data = res;
	rm_params.request_resource = pcie_sipa_res_request_resource;
	rm_params.release_resource = pcie_sipa_res_release_resource;
	ret = sipa_rm_create_resource(&rm_params);

	/* defer to create rm */
	if (ret == -EPROBE_DEFER) {
		schedule_delayed_work(&res->rm_work, msecs_to_jiffies(1000));
		return res;
	} else if (ret) {
		pr_err("pcie_sipa_res: create rm error = %d!\n", ret);
		sprd_pms_destroy(res->pms);
		kfree(res);
		return NULL;
	}

	/* add dependencys */
	ret = sipa_rm_add_dependency(cons_id, prod_id);
	if (ret < 0 && ret != -EINPROGRESS) {
		pr_err("pcie_sipa_res: add_dependency error = %d!\n", ret);
		sipa_rm_delete_resource(prod_id);
		sprd_pms_destroy(res->pms);
		kfree(res);
		return NULL;
	}

	return res;
}

void pcie_sipa_res_destroy(void *data)
{
	struct pcie_sipa_res_prod *res = data;

	cancel_work_sync(&res->wait_work);
	cancel_delayed_work_sync(&res->rm_work);

	sprd_pms_destroy(res->pms);
	sipa_rm_delete_dependency(res->cons_id, res->prod_id);
	sipa_rm_delete_resource(res->prod_id);
	kfree(res);
}

