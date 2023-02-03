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
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/version.h>

#ifdef CONFIG_SPRD_SIPA_RES
#include "pcie_sipa_res.h"
#endif

#include "../include/pcie-rc-sprd.h"
#include "../include/sipc.h"
//#include "../include/mdm_ctrl.h"
#include "../include/sprd_pcie_ep_device.h"
#include "../include/sprd_mpm.h"
#include "../include/sprd_pcie_resource.h"

#define PCIE_REMOVE_SCAN_GAP	msecs_to_jiffies(200)
#define MAX_PMS_WAIT_TIME	5000
#define MAX_PMS_DEFECTIVE_CHIP_FIRST_WAIT_TIME	(55 * 1000)

enum rc_state {
	SPRD_PCIE_WAIT_FIRST_READY = 0,
	SPRD_PCIE_WAIT_SCANNED,
	SPRD_PCIE_SCANNED,
	SPRD_PCIE_WAIT_REMOVED,
	SPRD_PCIE_REMOVED,
	SPRD_PCIE_SCANNED_2BAR,
	SPRD_PCIE_WAIT_POWER_OFF
};

struct sprd_pcie_res {
	u32	dst;
	u32	ep_dev;
	u32	state;
	u32	scan_cnt;
	u32	max_wait_time;
	bool	ep_power_on;
	bool	ep_dev_probe;
	bool	smem_send_to_ep;
	unsigned long	action_jiff;

	struct sprd_pms	*pms;
	char		pms_name[20];

	wait_queue_head_t	wait_pcie_ready;
	bool			ep_ready_for_rescan;
	wait_queue_head_t	wait_load_ready;
	wait_queue_head_t	wait_first_rescan;
	struct task_struct	*thread;

#ifdef CONFIG_SPRD_SIPA_RES
	void *sipa_res;
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	struct wakeup_source	*ws;
#else
	struct wakeup_source	ws;
#endif
	struct work_struct	scan_work;
	struct work_struct	remove_work;
	struct workqueue_struct *wq;

	struct platform_device		*pcie_dev;
	struct sprd_pcie_register_event	reg_event;
};

static int sprd_pcie_resource_rescan(struct sprd_pcie_res *res);

static struct sprd_pcie_res *g_pcie_res[SIPC_ID_NR];

static void sprd_pcie_resource_host_first_rescan_do(struct sprd_pcie_res *res)
{
	int ret = sprd_pcie_register_event(&res->reg_event);

	if (ret)
		pr_err("pcie res: register pci ret=%d.\n", ret);

	/* power up for ep after the first scan. */
	res->ep_power_on = true;
	sprd_pms_power_up(res->pms);

#ifdef CONFIG_SPRD_SIPA_RES
	/*
	 * in host side, producer res id is SIPA_RM_RES_PROD_PCIE3,
	 * consumer res id is SIPA_RM_RES_CONS_WWAN_UL.
	 */
	res->sipa_res = pcie_sipa_res_create(res->dst,
					     SIPA_RM_RES_PROD_PCIE3,
					     SIPA_RM_RES_CONS_WWAN_UL);
	if (!res->sipa_res)
		pr_err("pcie res:create ipa res failed.\n");
#endif

}

static void sprd_pcie_resource_host_ep_notify(int event, void *data)
{
	struct sprd_pcie_res *res = (struct sprd_pcie_res *)data;
	u32 base, size;

	/* wait power off, do nothing */
	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	switch (event) {
	case PCIE_EP_PROBE:
		/* set state to scanned */
		res->state = SPRD_PCIE_SCANNED;
		res->scan_cnt++;
		res->ep_dev_probe = true;
		//modem_ctrl_enable_cp_event();
		
		if (smem_get_area(SIPC_ID_MINIAP, &base, &size) == 0)
			sprd_ep_dev_pass_smem(res->ep_dev, base, size);

		pr_info("pcie res: ep_notify, probed cnt=%d.\n",
			res->scan_cnt);

		/*  firsrt scan do somtehing */
		if (res->scan_cnt == 1)
			sprd_pcie_resource_host_first_rescan_do(res);

		/* clear removed irq and notify ep scanned */
		sprd_ep_dev_clear_doolbell_irq(res->ep_dev,
					       PCIE_DBEL_EP_REMOVING);
		sprd_ep_dev_raise_irq(res->ep_dev, PCIE_DBEL_EP_SCANNED);

		/* wakeup all blocked thread */
		wake_up_interruptible_all(&res->wait_pcie_ready);
		break;

	case PCIE_EP_REMOVE:
		pr_info("pcie res: ep_notify, removed.\n");
		res->state = SPRD_PCIE_REMOVED;
		res->ep_dev_probe = false;
		break;

	case PCIE_EP_PROBE_BEFORE_SPLIT_BAR:
		res->state = SPRD_PCIE_SCANNED_2BAR;
		res->ep_dev_probe = true;
		pr_info("pcie res: probed before split bar.\n");
		if (!res->ep_ready_for_rescan) {
			wake_up_interruptible_all(&res->wait_load_ready);
		} else {
			pr_info("pcie res: bar err, rescan.\n");
			sprd_pcie_resource_rescan(res);
		}
		break;

	default:
		break;
	}
}

static irqreturn_t sprd_pcie_resource_host_irq_handler(int irq, void *private)
{
	struct sprd_pcie_res *res = (struct sprd_pcie_res *)private;

	if (irq == PCIE_MSI_REQUEST_RES) {
		pr_info("pcie res: ep request res.\n");
		/*
		 * client modem power up,
		 * no need wake lock and no need wait resource.
		 */
		if (!res->ep_power_on) {
			res->ep_power_on = true;
			sprd_pms_power_up(res->pms);
		}

		/* only after received ep request can backup the ep configs. */
		sprd_ep_dev_set_backup(res->ep_dev);
	} else if (irq == PCIE_MSI_RELEASE_RES) {
		pr_info("pcie res: ep release res.\n");
		/*
		 * client modem power down,
		 * no need wake lock.
		 */
		if (res->ep_power_on) {
			res->ep_power_on = false;
			sprd_pms_power_down(res->pms, false);
		}
	} else if (irq == PCIE_MSI_EP_READY_FOR_RESCAN) {
		pr_info("pcie res: ep ready for rescan.\n");
		res->ep_ready_for_rescan = true;
		wake_up_interruptible_all(&res->wait_first_rescan);
	}

	return IRQ_HANDLED;
}

static void sprd_pcie_resource_scan_fn(struct work_struct *work)
{
	unsigned long diff;
	unsigned int delay;
	int ret;
	struct sprd_pcie_res *res = container_of(work, struct sprd_pcie_res,
						 scan_work);

	/* wait power off, do nothing */
	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	/* request wakelock */
	sprd_pms_request_wakelock(res->pms);

	diff = jiffies - res->action_jiff;
	if (diff < PCIE_REMOVE_SCAN_GAP) {
		/* must ensure that the scan starts after a period of remove. */
		delay = jiffies_to_msecs(PCIE_REMOVE_SCAN_GAP - diff);
		msleep(delay);
	}

	pr_info("pcie res: scan\n");

	ret = sprd_pcie_configure_device(res->pcie_dev);
	if (ret)
		pr_err("pcie res: scan error = %d!\n", ret);

	/* record the last scan jiffies */
	res->action_jiff = jiffies;

	/* release wakelock */
	sprd_pms_release_wakelock(res->pms);
}

static void sprd_pcie_resource_remove_fn(struct work_struct *work)
{
	unsigned long diff;
	unsigned int delay;
	int ret;
	struct sprd_pcie_res *res = container_of(work, struct sprd_pcie_res,
						 remove_work);
	/* request wakelock */
	sprd_pms_request_wakelock(res->pms);

	pr_info("pcie res: remove work!\n");

	diff = jiffies - res->action_jiff;
	if (diff < PCIE_REMOVE_SCAN_GAP) {
		/* must ensure that the remove starts after a period of scan. */
		delay = jiffies_to_msecs(PCIE_REMOVE_SCAN_GAP - diff);
		msleep(delay);
	}

	/*
	 * in wait power off state, or ep device is not probing,
	 * can't access ep.
	 */
	if (res->state == SPRD_PCIE_WAIT_POWER_OFF ||
	    !res->ep_dev_probe) {
		/* release wakelock */
		sprd_pms_release_wakelock(res->pms);
		return;
	}

	/* notify ep removed, must before removed */
	sprd_ep_dev_clear_doolbell_irq(res->ep_dev, PCIE_DBEL_EP_SCANNED);
	sprd_ep_dev_raise_irq(res->ep_dev, PCIE_DBEL_EP_REMOVING);

	/* waiting for the doorbell irq to ep */
	msleep(50);

	pr_info("pcie res: remove\n");

	/* start removed ep*/
	ret = sprd_pcie_unconfigure_device(res->pcie_dev);
	if (ret)
		pr_err("pcie res: remove error = %d.\n!", ret);

	/* record the last remov jiffies */
	res->action_jiff = jiffies;

	/* release wakelock */
	sprd_pms_release_wakelock(res->pms);
}

static void sprd_pcie_resource_start_scan(struct sprd_pcie_res *res)
{
	if (res->state == SPRD_PCIE_SCANNED ||
		res->state == SPRD_PCIE_WAIT_SCANNED) {
		pr_info("pcie res: scanned, do nothing!\n");
	} else {
		pr_info("pcie res: start scan!\n");
		queue_work(res->wq, &res->scan_work);
	}
}

static void sprd_pcie_resource_start_remove(struct sprd_pcie_res *res)
{
	/* wait power off, do nothing */
	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	if (res->state == SPRD_PCIE_SCANNED ||
		res->state == SPRD_PCIE_WAIT_FIRST_READY
		|| (res->state == SPRD_PCIE_SCANNED_2BAR)
		) {
		res->state = SPRD_PCIE_WAIT_REMOVED;
		pr_info("pcie res: start remove.");
		queue_work(res->wq, &res->remove_work);
	} else {
		pr_err("pcie res: start remove, err=%d.", res->state);
	}
}

static void sprd_pcie_resource_event_process(enum sprd_pcie_event event,
					    void *data)
{
	struct sprd_pcie_res *res = data;

	if (event == SPRD_PCIE_EVENT_WAKEUP) {
		pr_info("pcie res: wakeup by ep, event=%d.\n", event);
		if (!res->ep_power_on) {
			res->ep_power_on = true;
			sprd_pms_power_up(res->pms);
		}
	}
}

/*
 * sprd_pcie_resource_rescan
 * Because the ep bar can only be split by ep itself,
 * After all modem images be loaded at the first time,
 * the ep will run and split 2 64bit bar to 4 32bit bar.
 * host must rescan the pcie ep device agian by this api,
 * after receive ep driver ready for rescan msg and all
 * modem images load done.
 */
static int sprd_pcie_resource_rescan(struct sprd_pcie_res *res)
{
	pr_info("pcie res: rescan.\n");

	sprd_pcie_resource_start_remove(res);
	sprd_pcie_resource_start_scan(res);

	return 0;
}

static int sprd_pcie_resource_check_first_rescan(void *data)
{
	struct sprd_pcie_res *res = data;
	int ret;

	pr_info("pcie res: check first rescan.\n");

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(
			res->wait_first_rescan,
			res->ep_ready_for_rescan);
		if (!ret) {
			pr_info("pcie res:first resacn ready.\n");
			sprd_pcie_resource_rescan(res);
			break;
		}
	}

	/* After the first rescan, restore the normal wait time. */
	if (sprd_pcie_is_defective_chip())
		res->max_wait_time = MAX_PMS_WAIT_TIME;

	res->thread = NULL;
	return 0;
}

#if 0
static int sprd_pcie_resource_host_mcd(struct notifier_block *nb,
				      unsigned long mode, void *cmd)
{
	struct sprd_pcie_res *res;
	int i;
	u32 state;

	pr_info("pcie res: mcd mode=%ld.\n", mode);

	switch (mode) {
	case MDM_POWER_OFF:
		state = SPRD_PCIE_WAIT_POWER_OFF;
		break;

	default:
		return NOTIFY_DONE;
	}

	for (i = 0; i < SIPC_ID_NR; i++) {
		res = g_pcie_res[i];

		/* wait power off, do nothing */
		if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
			continue;

		if (res) {
			res->state = state;
			cancel_work_sync(&res->scan_work);
			cancel_work_sync(&res->remove_work);
		}
	}

	return NOTIFY_DONE;
}

static struct notifier_block mcd_notify = {
	.notifier_call = sprd_pcie_resource_host_mcd,
	.priority = 149,
};
#endif

/* Because the ep bar can only be split by ep itself,
 * After all modem images be loaded, notify the pcie resource.
 */
void sprd_pcie_resource_notify_load_done(u32 dst)
{
	struct sprd_pcie_res *res;

	pr_info("pcie res: load done.\n");

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return;

	res = g_pcie_res[dst];

	res->thread = kthread_create(sprd_pcie_resource_check_first_rescan, res,
		"first rescan");
	if (IS_ERR(res->thread))
		pr_err("pcie res: Failed to create rescan thread.\n");
	else
		wake_up_process(res->thread);
}

int sprd_pcie_wait_load_resource(u32 dst)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	/* can load image, return immediately */
	if (res->state == SPRD_PCIE_SCANNED ||
		res->state == SPRD_PCIE_SCANNED_2BAR)
		return 0;

	return wait_event_interruptible(
		res->wait_load_ready,
		(res->state == SPRD_PCIE_SCANNED ||
		res->state == SPRD_PCIE_SCANNED_2BAR));
}

void sprd_pcie_resource_reboot_ep(u32 dst)
{
	struct sprd_pcie_res *res;

	pr_info("pcie res: reboot ep.\n");

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return;

	res = g_pcie_res[dst];

	/* wait power off, do nothing */
	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	res->state = SPRD_PCIE_WAIT_FIRST_READY;
	res->smem_send_to_ep = false;
	res->ep_ready_for_rescan = false;

	/* The defective chip , the first wait time must be enough long. */
	if (sprd_pcie_is_defective_chip())
		res->max_wait_time = MAX_PMS_DEFECTIVE_CHIP_FIRST_WAIT_TIME;
	else
		res->max_wait_time = MAX_PMS_WAIT_TIME;

	/* after ep reboot, can't backup ep configs*/
	sprd_ep_dev_clear_backup(res->ep_dev);

	sprd_pcie_resource_start_remove(res);
	//modem_ctrl_poweron_modem(MDM_CTRL_COLD_RESET);
	sprd_pcie_resource_start_scan(res);
}

int sprd_pcie_resource_host_init(u32 dst, u32 ep_dev,
				 struct platform_device *pcie_dev)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR)
		return -EINVAL;

	res = kzalloc(sizeof(*res), GFP_KERNEL);
	if (!res)
		return -ENOMEM;

	res->wq = create_singlethread_workqueue("pcie_res");
	if (!res->wq) {
		pr_err("pcie res:create wq failed.\n");
		kfree(res);
		return -ENOMEM;
	}

	init_waitqueue_head(&res->wait_load_ready);
	init_waitqueue_head(&res->wait_first_rescan);
	
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	res->ws = wakeup_source_register(NULL, "pcie_res");   
#else
	wakeup_source_init(&res->ws, "pcie_res");
#endif

	res->dst = dst;
	res->state = SPRD_PCIE_WAIT_FIRST_READY;
	res->pcie_dev = pcie_dev;

	/* The defective chip , the first wait time must be enough long. */
	if (sprd_pcie_is_defective_chip())
		res->max_wait_time = MAX_PMS_DEFECTIVE_CHIP_FIRST_WAIT_TIME;
	else
		res->max_wait_time = MAX_PMS_WAIT_TIME;

	init_waitqueue_head(&res->wait_pcie_ready);
	INIT_WORK(&res->scan_work, sprd_pcie_resource_scan_fn);
	INIT_WORK(&res->remove_work, sprd_pcie_resource_remove_fn);

	sprintf(res->pms_name, "ep-request-%d", dst);
	res->pms = sprd_pms_create(dst, res->pms_name, false);
	if (!res->pms)
		pr_err("pcie res:create pms failed.\n");

	sprd_ep_dev_register_irq_handler_ex(res->ep_dev,
			   PCIE_MSI_REQUEST_RES,
			   PCIE_MSI_RELEASE_RES,
			   sprd_pcie_resource_host_irq_handler, res);

	sprd_ep_dev_register_notify(res->ep_dev,
		sprd_pcie_resource_host_ep_notify, res);

	//modem_ctrl_register_notifier(&mcd_notify);

	/* init wake up event callback */
	res->reg_event.events = SPRD_PCIE_EVENT_WAKEUP;
	res->reg_event.pdev = pcie_dev;
	res->reg_event.callback = sprd_pcie_resource_event_process;
	res->reg_event.data = res;

	g_pcie_res[dst] = res;

	return 0;
}

int sprd_pcie_resource_trash(u32 dst)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	if (!IS_ERR_OR_NULL(res->thread))
		kthread_stop(res->thread);

#ifdef CONFIG_SPRD_SIPA_RES
	if (res->sipa_res)
		pcie_sipa_res_destroy(res->sipa_res);
#endif

	cancel_work_sync(&res->scan_work);
	cancel_work_sync(&res->remove_work);
	destroy_workqueue(res->wq);

	sprd_pcie_deregister_event(&res->reg_event);

	sprd_ep_dev_unregister_irq_handler_ex(res->ep_dev,
			   PCIE_MSI_REQUEST_RES,
			   PCIE_MSI_RELEASE_RES);
	sprd_ep_dev_unregister_notify(res->ep_dev);
	//modem_ctrl_unregister_notifier(&mcd_notify);
	sprd_pms_destroy(res->pms);

	kfree(res);
	g_pcie_res[dst] = NULL;

	return 0;
}

int sprd_pcie_wait_resource(u32 dst, int timeout)
{
	struct sprd_pcie_res *res;
	int ret, wait;
	unsigned long delay;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	/* pcie ready, return succ immediately. */
	if (res->state == SPRD_PCIE_SCANNED)
		return 0;

	if (timeout == 0)
		return -ETIME;

	/*
	 * In some case, orca may has an exception, And the pcie
	 * resource may never ready again. So we	must set a
	 * maximum wait time for let user to know thereis an
	 * exception in pcie, and can return an error code to the user.
	 */
	if (timeout < 0 || timeout > res->max_wait_time)
		timeout = res->max_wait_time;

	/*
	 * timeout must add 1s,
	 * because the pcie scan may took some time.
	 */
	delay = msecs_to_jiffies(timeout + 1000);
	wait = wait_event_interruptible_timeout(res->wait_pcie_ready,
						res->state ==
						SPRD_PCIE_SCANNED,
						delay);
	if (wait == 0)
		ret = -ETIME;
	else if (wait > 0)
		ret = 0;
	else
		ret = wait;


	if (ret < 0 && ret != -ERESTARTSYS)
		pr_err("pcie res: wait resource, val=%d.\n", ret);

	return ret;
}

int sprd_pcie_request_resource(u32 dst)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	/* get a wakelock */
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	__pm_stay_awake(res->ws);
#else
	__pm_stay_awake(&res->ws);
#endif

	pr_info("pcie res: request resource, state=%d.\n", res->state);

#ifdef CONFIG_SPRD_PCIE
	/* The first scan is start by pcie driver automatically. */
	if (res->state != SPRD_PCIE_WAIT_FIRST_READY)
		sprd_pcie_resource_start_scan(res);
#endif

	return 0;
}

int sprd_pcie_release_resource(u32 dst)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	/* relax a wakelock */
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	__pm_relax(res->ws);
#else
	__pm_relax(&res->ws);
#endif

#ifdef CONFIG_SPRD_PCIE
	pr_info("pcie res: release resource.\n");

	sprd_pcie_resource_start_remove(res);
#endif
	return 0;
}

bool sprd_pcie_is_defective_chip(void)
{
#ifndef CONFIG_SPRD_PCIE
	return false;
#else
	static bool first_read = true, defective;

	if (first_read) {
		first_read = false;
		defective = sprd_kproperty_chipid("UD710-AB") == 0;
	}

	return defective;
#endif
}
