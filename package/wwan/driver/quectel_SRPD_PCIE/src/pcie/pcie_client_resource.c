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
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mdm_ctrl.h>
#include <linux/pcie-epf-sprd.h>
#include <linux/sched.h>
#include <linux/sipc.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "../include/sprd_pcie_resource.h"
#ifdef CONFIG_SPRD_SIPA_RES
#include "pcie_sipa_res.h"
#endif

enum ep_msg {
	RC_SCANNED_MSG = 0,
	RC_REMOVING_MSG,
	EPC_UNLINK_MSG,
	EPC_LINKUP_MSG
};

enum pcie_ep_state {
	SPRD_PCIE_WAIT_FIRST_READY = 0,
	SPRD_PCIE_WAIT_SCANNED,
	SPRD_PCIE_SCANNED,
	SPRD_PCIE_WAIT_REMOVED,
	SPRD_PCIE_REMOVED,
	SPRD_PCIE_WAIT_POWER_OFF
};

struct sprd_pci_res_notify {
	void	(*notify)(void *p);
	void	*data;
};

struct sprd_pcie_res {
	u32	dst;
	u32	ep_fun;
	enum pcie_ep_state	state;
	bool	msi_later;
	bool	wakeup_later;

#ifdef CONFIG_SPRD_SIPA_RES
	void *sipa_res;
#endif

	/*
	 * in client(Orca), The PCIE module wll blocks the chip Deep,
	 * so we must get a wake lock when pcie work to avoid this situation:
	 * the system is deep, but the PCIE is still working.
	 */
	struct wakeup_source	ws;
	wait_queue_head_t		wait_pcie_ready;
	struct sprd_pci_res_notify	first_ready_notify;
};

static struct sprd_pcie_res *g_pcie_res[SIPC_ID_NR];

/* the state machine of ep, init SPRD_PCIE_WAIT_FIRST_READY.
 * SPRD_PCIE_WAIT_FIRST_READY (receive RC scanned) ==> SPRD_PCIE_SCANNED
 * SPRD_PCIE_SCANNED (receive RC removing)==> SPRD_PCIE_WAIT_REMOVED
 * SPRD_PCIE_WAIT_REMOVED(receive epc unlink)==>SPRD_PCIE_REMOVED
 * SPRD_PCIE_REMOVED(receive epc linkup)==>SPRD_PCIE_WAIT_SCANNED
 * SPRD_PCIE_WAIT_SCANNED(receive RC scanned)==>SPRD_PCIE_SCANNED
 * SPRD_PCIE_WAIT_POWER_OFF can do nothing, just wait shutdown.
 */
static const char *change_msg[EPC_LINKUP_MSG + 1] = {
	"rc scanned",
	"rc removing",
	"epc unlink",
	"epc linkup"
};

static const char *state_msg[SPRD_PCIE_REMOVED + 1] = {
	"wait first ready",
	"wait sacanned",
	"scanned",
	"wait remove",
	"removed"
};

static void pcie_resource_client_change_state(struct sprd_pcie_res *res,
					      enum ep_msg msg)
{
	u32 old_state = res->state;

	if (old_state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	pr_debug("pcie res: change state msg=%s, old_state=%s.\n",
		 change_msg[msg], state_msg[old_state]);

	switch (msg) {
	case RC_SCANNED_MSG:
		if (old_state != SPRD_PCIE_WAIT_FIRST_READY
		    && old_state != SPRD_PCIE_WAIT_SCANNED) {
			pr_err("pcie res: %s msg err, old state=%s",
			       change_msg[msg], state_msg[old_state]);
			return;
		}
		res->state = SPRD_PCIE_SCANNED;
		break;

	case RC_REMOVING_MSG:
		if (old_state != SPRD_PCIE_SCANNED) {
			pr_err("pcie res: %s msg err, old state=%s",
			       change_msg[msg], state_msg[old_state]);
			return;
		}
		res->state = SPRD_PCIE_WAIT_REMOVED;
		break;

	case EPC_UNLINK_MSG:
		if (old_state != SPRD_PCIE_WAIT_REMOVED) {
			if (old_state != SPRD_PCIE_WAIT_FIRST_READY)
				pr_err("pcie res: %s msg err, old state=%s",
				       change_msg[msg], state_msg[old_state]);
			return;
		}
		res->state = SPRD_PCIE_REMOVED;
		break;

	case EPC_LINKUP_MSG:
		if (old_state != SPRD_PCIE_REMOVED) {
			if (old_state != SPRD_PCIE_WAIT_FIRST_READY)
				pr_err("pcie res: %s msg err, old state=%s",
				       change_msg[msg], state_msg[old_state]);
			return;
		}
		res->state = SPRD_PCIE_WAIT_SCANNED;
		break;
	}

	pr_info("pcie res: change state from %s to %s.\n",
		 state_msg[old_state], state_msg[res->state]);
}

static void sprd_pcie_resource_first_ready_notify(struct sprd_pcie_res *res)
{
	void (*notify)(void *p);

	pr_info("pcie res: first ready.\n");

#ifdef CONFIG_SPRD_SIPA_RES
	/*
	 * in client side, producer res id is SIPA_RM_RES_PROD_PCIE_EP,
	 * consumer res id is SIPA_RM_RES_CONS_WWAN_DL.
	 */
	res->sipa_res = pcie_sipa_res_create(res->dst,
					     SIPA_RM_RES_PROD_PCIE_EP,
					     SIPA_RM_RES_CONS_WWAN_DL);
	if (!res->sipa_res)
		pr_err("pcie res:create ipa res failed.\n");
#endif

	notify = res->first_ready_notify.notify;
	if (notify)
		notify(res->first_ready_notify.data);
}

static void pcie_resource_client_epf_notify(int event, void *private)
{
	struct sprd_pcie_res *res = (struct sprd_pcie_res *)private;

	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return;

	switch (event) {
	case SPRD_EPF_BIND:
		pr_info("pcie res: epf be binded.\n");
		if (sprd_pcie_is_defective_chip())
			sprd_pci_epf_raise_irq(res->ep_fun,
					 PCIE_MSI_EP_READY_FOR_RESCAN);
		break;

	case SPRD_EPF_UNBIND:
		pr_info("pcie res: epf be unbinded.\n");
		break;

	case SPRD_EPF_REMOVE:
		pr_info("pcie res: epf be removed.\n");
		break;

	case SPRD_EPF_LINK_UP:
		/* get a wakelock */
		__pm_stay_awake(&res->ws);

		pr_info("pcie res: epf linkup.\n");
		pcie_resource_client_change_state(res, EPC_LINKUP_MSG);

		/* first ready notify */
		if (res->state == SPRD_PCIE_WAIT_FIRST_READY)
			sprd_pcie_resource_first_ready_notify(res);

		break;

	case SPRD_EPF_UNLINK:
		/* Here need this log to debug pcie scan and remove */
		pr_info("pcie res: epf unlink.\n");
		pcie_resource_client_change_state(res, EPC_UNLINK_MSG);

		/*  if has wakeup pending, send wakeup to rc */
		if (res->wakeup_later) {
			res->wakeup_later = false;
			pr_info("pcie res: send wakeup to rc.\n");
			if (sprd_pci_epf_start(res->ep_fun))
				pr_err("pcie res: send wakeup to rc failed.\n");
		}

		/* relax a wakelock */
		__pm_relax(&res->ws);
		break;

	default:
		break;
	}
}

static irqreturn_t pcie_resource_client_irq_handler(int irq, void *private)
{
	struct sprd_pcie_res *res = (struct sprd_pcie_res *)private;

	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return IRQ_HANDLED;

	if (irq == PCIE_DBEL_EP_SCANNED) {
		pcie_resource_client_change_state(res, RC_SCANNED_MSG);

		/* wakeup all blocked thread */
		pr_info("pcie res: scanned, wakup all.\n");
		wake_up_interruptible_all(&res->wait_pcie_ready);

		/*  if has msi pending, send msi to rc */
		if (res->msi_later) {
			res->msi_later = false;
			pr_info("pcie res: request msi to rc.\n");
			sprd_pci_epf_raise_irq(res->ep_fun,
					     PCIE_MSI_REQUEST_RES);
		}
	} else if (irq == PCIE_DBEL_EP_REMOVING) {
		pr_info("pcie res: removing.\n");
		pcie_resource_client_change_state(res, RC_REMOVING_MSG);
	}

	return IRQ_HANDLED;
}

static int sprd_pcie_resource_client_mcd(struct notifier_block *nb,
				      unsigned long mode, void *cmd)
{
	struct sprd_pcie_res *res;
	int i;

	pr_info("pcie res: mcd event mode=%ld.\n", mode);

	if (mode != MDM_POWER_OFF)
		return NOTIFY_DONE;

	for (i = 0; i < SIPC_ID_NR; i++) {
		res = g_pcie_res[i];
		if (res)
			res->state = SPRD_PCIE_WAIT_POWER_OFF;
	}

	return NOTIFY_DONE;
}

static struct notifier_block mcd_notify = {
	.notifier_call = sprd_pcie_resource_client_mcd,
	.priority = 149,
};

int sprd_pcie_resource_client_init(u32 dst, u32 ep_fun)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR)
		return -EINVAL;

	res = kzalloc(sizeof(*res), GFP_KERNEL);
	if (!res)
		return -ENOMEM;

	res->dst = dst;
	res->state = SPRD_PCIE_WAIT_FIRST_READY;
	res->ep_fun = ep_fun;

	wakeup_source_init(&res->ws, "pcie_res");

	init_waitqueue_head(&res->wait_pcie_ready);
	sprd_pci_epf_register_irq_handler_ex(res->ep_fun,
					     PCIE_DBEL_EP_SCANNED,
					     PCIE_DBEL_EP_REMOVING,
					     pcie_resource_client_irq_handler,
					     res);
	sprd_pci_epf_register_notify(res->ep_fun,
				     pcie_resource_client_epf_notify,
				     res);

	modem_ctrl_register_notifier(&mcd_notify);

	g_pcie_res[dst] = res;

	return 0;
}

int sprd_pcie_resource_trash(u32 dst)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

#ifdef CONFIG_SPRD_SIPA_RES
	if (res->sipa_res)
		pcie_sipa_res_destroy(res->sipa_res);
#endif

	sprd_pci_epf_unregister_irq_handler_ex(res->ep_fun,
		PCIE_DBEL_EP_SCANNED,
		PCIE_DBEL_EP_REMOVING);
	sprd_pci_epf_unregister_notify(res->ep_fun);
	modem_ctrl_unregister_notifier(&mcd_notify);

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

	/*  pcie ready, return succ immediately */
	if (res->state == SPRD_PCIE_SCANNED)
		return 0;

	if (timeout == 0)
		return -ETIME;

	if (timeout < 0) {
		wait = wait_event_interruptible(
						res->wait_pcie_ready,
						res->state == SPRD_PCIE_SCANNED
						);
		ret = wait;
	} else {
		/*
		 * timeout must add 1s,
		 * because the pcie rescan may took some time.
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
	}

	if (ret < 0 && ret != -ERESTARTSYS)
		pr_err("pcie res: wait resource, val=%d.\n", ret);

	return ret;
}

int sprd_pcie_request_resource(u32 dst)
{
	struct sprd_pcie_res *res;
	int ret = 0;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return -EINVAL;

	pr_debug("pcie res: request res, state=%d.\n", res->state);

	switch (res->state) {
	case SPRD_PCIE_WAIT_FIRST_READY:
	case SPRD_PCIE_WAIT_SCANNED:
		pr_info("pcie res: later send request msi to rc.\n");
		res->msi_later = true;
		break;

	case SPRD_PCIE_WAIT_REMOVED:
		pr_info("pcie res: later send wakeup to rc.\n");
		res->wakeup_later = true;
		break;

	case SPRD_PCIE_SCANNED:
		/*
		 * if pcie state is SCANNED, just send
		 * PCIE_MSI_REQUEST_RES to the host.
		 * After host receive res msi interrupt,
		 * it will increase one vote in modem power manger.
		 */
		pr_info("pcie res: send request msi to rc.\n");
		ret = sprd_pci_epf_raise_irq(res->ep_fun,
					     PCIE_MSI_REQUEST_RES);
		break;

	case SPRD_PCIE_REMOVED:
		/*
		 * if pcie state is removed, poll wake_up singnal
		 * to host, and he host will rescan the pcie.
		 */
		pr_info("pcie res: send wakeup to rc.\n");
		if (sprd_pci_epf_start(res->ep_fun) == 0)
			break;

		/* may receive ep reset, wait linkup and scanned */
		pr_info("pcie res: later send request msi to rc.\n");
		res->msi_later = true;
		break;

	default:
		pr_err("pcie res: request res err, state=%d.\n",
		       res->state);
		ret = -EPERM;
		break;
	}

	return ret;
}

int sprd_pcie_release_resource(u32 dst)
{
	struct sprd_pcie_res *res;
	int ret = 0;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	if (res->state == SPRD_PCIE_WAIT_POWER_OFF)
		return -EINVAL;

	switch (res->state) {
	case SPRD_PCIE_SCANNED:
		/*
		 * if pcie state is SCANNED, send PCIE_MSI_RELEASE_RES
		 * to the host, else, do nothing. After host receive res msi
		 * interrupt, it will decrease one vote in modem power manger,
		 * and if modem power manger is idle, the host will remove
		 * the pcie.
		 */
		pr_info("pcie res: send release msi to rc.\n");
		ret = sprd_pci_epf_raise_irq(res->ep_fun,
					     PCIE_MSI_RELEASE_RES);
		break;

	case SPRD_PCIE_WAIT_FIRST_READY:
		/* if has msi pending, remove it */
		if (res->msi_later)
			res->msi_later = false;
		break;

	default:
		pr_err("pcie res: release res state=%d.\n", res->state);
		ret = -EPERM;
		break;
	}

	return ret;
}

int sprd_register_pcie_resource_first_ready(u32 dst,
					    void (*notify)(void *p), void *data)
{
	struct sprd_pcie_res *res;

	if (dst >= SIPC_ID_NR || !g_pcie_res[dst])
		return -EINVAL;

	res = g_pcie_res[dst];

	res->first_ready_notify.data = data;
	res->first_ready_notify.notify = notify;

	return 0;
}

bool sprd_pcie_is_defective_chip(void)
{
	static bool first_read = true, defective;

	if (first_read) {
		first_read = false;
		defective = sprd_kproperty_chipid("UD710-AB") == 0;
	}

	return defective;
}
