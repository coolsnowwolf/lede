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
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/version.h>

#include "../include/sprd_mpm.h"
#include "../include/sipc.h"
/*
 * The data struct of modem power manager.
 */
struct sprd_mpm_data {
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	struct wakeup_source	*ws;
#else
	struct wakeup_source	ws;
#endif
	struct list_head	pms_list;
	struct timer_list	timer;
	spinlock_t		mpm_lock;
	char			name[20];
	const char		*last_name;
	unsigned int		dst;
	unsigned int		up_cnt;
	unsigned int		awake_cnt;
	unsigned int		wakelock_cnt;
	unsigned int		mpm_state;
	unsigned long		expires;
	unsigned int		later_idle;

	/* resource ops functions */
	int (*wait_resource)(unsigned int dst, int timeout);
	int (*request_resource)(unsigned int dst);
	int (*release_resource)(unsigned int dst);

	struct work_struct	release_res_work;
	struct work_struct	request_res_work;
};

/*
 * Save all the instance of mpm in here.
 */
static struct sprd_mpm_data *g_sprd_mpm[SIPC_ID_NR];

/**
 * sprd_mpm_print_awake
 * print the wake up list to known who prevent system sleep.
 */
static void sprd_mpm_print_awake(struct sprd_mpm_data *mpm)
{
	struct sprd_pms *pms;
	char *awake_info;
	int len = 0, max_len = 512;

	awake_info = kmalloc(max_len, GFP_KERNEL);
	if (!awake_info)
		return;

	/* print pms list */
	list_for_each_entry(pms, &mpm->pms_list, entry) {
		if (!pms->awake && pms->pre_awake_cnt == pms->awake_cnt)
			continue;

		pms->pre_awake_cnt = pms->awake_cnt;
		snprintf(awake_info + len,
			 max_len - len,
			 "%s is awake, awake_cnt = %d\n",
			 pms->name,
			 pms->awake_cnt);
		len = strlen(awake_info);
	}

	if (len)
		pr_info("mpm: %s\n", awake_info);

	kfree(awake_info);
}

/**
 * sprd_mpm_pm_event
 * monitor the PM_SUSPEND_PREPARE event.
 */
static int sprd_mpm_pm_event(struct notifier_block *notifier,
			     unsigned long pm_event, void *unused)
{
	unsigned int i;
	struct sprd_mpm_data *cur;

	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
	case PM_POST_SUSPEND:
		/* check if has wake lock. */
		for (i = 0; i < SIPC_ID_NR; i++) {
			if (!g_sprd_mpm[i])
				continue;

			cur = g_sprd_mpm[i];
			sprd_mpm_print_awake(cur);
		}
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * The pm event notify data, for the register pm notifier.
 */
static struct notifier_block sprd_mpm_notifier_block = {
	.notifier_call = sprd_mpm_pm_event,
};

/**
 * sprd_mpm_request_resource
 * request resource.
 */
static void sprd_mpm_request_resource(struct sprd_mpm_data *mpm)
{
	if (mpm->request_resource)
		schedule_work(&mpm->request_res_work);
}

/**
 * sprd_mpm_release_resource
 * release resource.
 */
static void sprd_mpm_release_resource(struct sprd_mpm_data *mpm)
{
	if (mpm->release_resource)
		schedule_work(&mpm->release_res_work);
}

/**
 * sprd_mpm_wait_resource -wait resource.
 */
static int sprd_mpm_wait_resource(struct sprd_mpm_data *mpm, int timeout)
{
	int ret = 0;

	if (mpm->wait_resource) {
		ret = mpm->wait_resource(mpm->dst, timeout);
		if (ret < 0 && ret != -ERESTARTSYS && timeout)
			pr_err("mpm: %s wait resource, ret=%d, timeout=%d.\n",
				mpm->name, ret, timeout);
	}

	return ret;
}

/**
 * sprd_mpm_active
 * set the state to busy.
 */
static void sprd_mpm_active(struct sprd_mpm_data *mpm)
{
	pr_debug("mpm: %s active, set state to busy.\n", mpm->name);

	mpm->mpm_state = SPRD_MPM_BUSY;
	sprd_mpm_request_resource(mpm);
}

/**
 * sprd_mpm_deactive
 * del the idle timer,
 * set the state to idle.
 */
static void sprd_mpm_deactive(struct sprd_mpm_data *mpm)
{
	pr_debug("mpm: %s deactive, set state to idle.\n", mpm->name);

	mpm->mpm_state = SPRD_MPM_IDLE;
	mpm->expires = 0;
	sprd_mpm_release_resource(mpm);
}

/**
 * sprd_mpm_start_deactive
 * start the deactive timer.
 */
static void sprd_mpm_start_deactive(struct sprd_mpm_data *mpm)
{
	pr_debug("mpm: %s start deactive.\n", mpm->name);

	mpm->expires = jiffies + msecs_to_jiffies(mpm->later_idle);
	if (!mpm->expires)
		mpm->expires = 1;

	mod_timer(&mpm->timer, mpm->expires);
}

/**
 * sprd_mpm_request_res_work_fn
 * do release resource call in here.
 */
static void sprd_mpm_request_res_work_fn(struct work_struct *work)
{
	struct sprd_mpm_data *mpm = container_of(work, struct sprd_mpm_data,
						 request_res_work);
	int ret;

	pr_debug("mpm: %s request res work.\n", mpm->name);

	ret = mpm->request_resource(mpm->dst);
	if (ret)
		pr_err("mpm: %s request res, ret = %d.\n", mpm->name, ret);
}

/**
 * sprd_mpm_release_res_work_fn
 * do relase resource call in here
 */
static void sprd_mpm_release_res_work_fn(struct work_struct *work)
{
	struct sprd_mpm_data *mpm = container_of(work, struct sprd_mpm_data,
						 release_res_work);
	int ret;

	pr_debug("mpm: %s releae res work.\n", mpm->name);

	ret = mpm->release_resource(mpm->dst);
	if (ret)
		pr_err("mpm: %s request res, ret = %d.\n", mpm->name, ret);
}

/**
 * sprd_mpm_deactive_timer_fn
 * in a period of time (mpm->later_idle),
 * have no modem resource request,
 * we consider that it doesn't need modem resource,
 * than set the state to idle.
 */
static void sprd_mpm_deactive_timer_fn(
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
					unsigned long data)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)data;
#else
					struct timer_list *t)
{
	struct sprd_mpm_data *mpm = from_timer(mpm, t, timer);
#endif

	unsigned long flags;

	pr_debug("mpm: %s deactive timer.\n", mpm->name);

	spin_lock_irqsave(&mpm->mpm_lock, flags);
	/* expires is 0, means the timer has been cancelled. */
	if (mpm->expires)
		sprd_mpm_deactive(mpm);
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);
}

/**
 * sprd_pms_cancel_timer
 * cancel the pms wakelock timer.
 */
static void sprd_pms_cancel_timer(struct sprd_pms *pms)
{
	unsigned long flags;
	bool print = false;

	spin_lock_irqsave(&pms->expires_lock, flags);
	if (pms->expires) {
		print = true;
		pms->expires = 0;
		del_timer(&pms->wake_timer);
	}
	spin_unlock_irqrestore(&pms->expires_lock, flags);

	if (print)
		pr_debug("pms: %s del timer.\n", pms->name);

}

/**
 * sprd_mpm_cancel_timer
 * cancel the deactive timer.
 */
static void sprd_mpm_cancel_timer(struct sprd_mpm_data *mpm)
{
	if (mpm->expires) {
		pr_debug("mpm: %s del timer.\n", mpm->name);

		mpm->expires = 0;
		del_timer(&mpm->timer);
	}
}

/**
 * sprd_mpm_up
 * modem power manger power up.
 */
static void sprd_mpm_up(struct sprd_mpm_data *mpm, const char *name)
{
	unsigned long flags;

	spin_lock_irqsave(&mpm->mpm_lock, flags);

	/* first cancel deactive timer */
	sprd_mpm_cancel_timer(mpm);
	mpm->last_name = name;

	mpm->up_cnt++;
	/* when up_cnt is change form 0 to 1, ready active pms.
	 * Although the cnt is 0, but later down, the state may is still busy,
	 * so here must see whether the mpm state is idle.
	 */
	if (mpm->up_cnt == 1 &&
		mpm->mpm_state == SPRD_MPM_IDLE)
		sprd_mpm_active(mpm);

	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	pr_debug("mpm: %s up, up_cnt=%d.\n", mpm->name, mpm->up_cnt);
}

/**
 * sprd_mpm_down
 * modem power manger power down.
 */
static void sprd_mpm_down(struct sprd_mpm_data *mpm, bool immediately)
{
	unsigned long flags;

	/*
	 * when up_cnt count is change form 1 to 0,
	 * start deactive pms.
	 */
	spin_lock_irqsave(&mpm->mpm_lock, flags);
	mpm->up_cnt--;
	if (!mpm->up_cnt) {
		if (mpm->later_idle && !immediately)
			sprd_mpm_start_deactive(mpm);
		else
			sprd_mpm_deactive(mpm);
	}
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	pr_debug("mpm: %s down, up_cnt=%d.\n", mpm->name, mpm->up_cnt);
}

/**
 * sprd_mpm_stay_awake
 * modem power manager stay awake.
 */
static void sprd_mpm_stay_awake(struct sprd_mpm_data *mpm)
{
	unsigned long flags;

	/*
	 * when wakelock_cnt is change form 0 to 1,
	 * get the system wake lock.
	 */
	spin_lock_irqsave(&mpm->mpm_lock, flags);
	mpm->wakelock_cnt++;
	if (mpm->wakelock_cnt == 1) {
		mpm->awake_cnt++;
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
		__pm_stay_awake(mpm->ws);
#else
		__pm_stay_awake(&mpm->ws);
#endif
	}
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	pr_debug("mpm: %s wake, wake_cnt=%d\n",
		mpm->name, mpm->wakelock_cnt);
}

/**
 * sprd_mpm_relax
 * modem power manager relax wakelock.
 */
static void sprd_mpm_relax(struct sprd_mpm_data *mpm)
{
	unsigned long flags;

	/*
	 * when wakelock_cnt is change form 0 to 1,
	 * release the system wake lock.
	 */
	spin_lock_irqsave(&mpm->mpm_lock, flags);
	mpm->wakelock_cnt--;
	if (!mpm->wakelock_cnt)
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
		__pm_relax(mpm->ws);
#else
		__pm_relax(&mpm->ws);
#endif
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	pr_debug("mpm: %s relax wake, wake_cnt=%d\n",
		mpm->name, mpm->wakelock_cnt);
}

/**
 * sprd_pms_do_up_single
 * do pms power up.
 */
static void sprd_pms_do_up_single(struct sprd_pms *pms)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;

	/*
	 * when active_cnt is change form 0 to 1,  mpm up.
	 */
	pms->active_cnt++;
	if (pms->active_cnt == 1)
		sprd_mpm_up(mpm, pms->name);

	pr_debug("pms: %s up, active_cnt=%d.\n",
		pms->name, pms->active_cnt);
}

/**
 * sprd_pms_do_up_multi
 * do pms power up.
 */
static void sprd_pms_do_up_multi(struct sprd_pms *pms)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;
	unsigned long flags;
	bool active = false;

	/*
	 * when active_cnt is change form 0 to 1,  mpm up.
	 */
	spin_lock_irqsave(&pms->active_lock, flags);

	pms->active_cnt++;
	if (pms->active_cnt == 1)
		active = true;

	spin_unlock_irqrestore(&pms->active_lock, flags);

	pr_debug("pms: %s up, active_cnt=%d.\n",
		pms->name, pms->active_cnt);

	if (active)
		sprd_mpm_up(mpm, pms->name);
}

static void sprd_pms_do_up(struct sprd_pms *pms)
{
	if (pms->multitask)
		sprd_pms_do_up_multi(pms);
	else
		sprd_pms_do_up_single(pms);
}

/**
 * sprd_pms_do_down_single
 * do pms power down.
 */
static void sprd_pms_do_down_single(struct sprd_pms *pms, bool immediately)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;
	/*
	 * when active_cnt is change form 1 to 0,  mpm down.
	 */
	if (pms->active_cnt > 0) {
		pms->active_cnt--;
		if (pms->active_cnt == 0)
			sprd_mpm_down(mpm, immediately);
	}

	pr_debug("pms: %s down, active_cnt=%d.\n",
		pms->name, pms->active_cnt);
}

/**
 * sprd_pms_do_down
 * do pms power down.
 */
static void sprd_pms_do_down_multi(struct sprd_pms *pms, bool immediately)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;
	unsigned long flags;
	bool deactive = false;

	/*
	 * when active_cnt is change form 1 to 0,  mpm down.
	 */
	spin_lock_irqsave(&pms->active_lock, flags);

	if (pms->active_cnt > 0) {
		pms->active_cnt--;
		if (pms->active_cnt == 0)
			deactive = true;
	}

	spin_unlock_irqrestore(&pms->active_lock, flags);

	pr_debug("pms: %s down, active_cnt=%d.\n",
		pms->name, pms->active_cnt);

	if (deactive)
		sprd_mpm_down(mpm, immediately);
}

static void sprd_pms_do_down(struct sprd_pms *pms, bool immediately)
{
	if (pms->multitask)
		sprd_pms_do_down_multi(pms, immediately);
	else
		sprd_pms_do_down_single(pms, immediately);
}

/**
 * sprd_pms_stay_awake
 * power manger source stay awake.
 */
static void sprd_pms_stay_awake(struct sprd_pms *pms)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;

	pr_debug("pms: %s stay awake.\n", pms->name);

	pms->awake_cnt++;
	if (!pms->awake) {
		pms->awake = true;
		sprd_mpm_stay_awake(mpm);
	}
}

/**
 * sprd_pms_relax
 * power manger source release wakelock.
 */
static void sprd_pms_relax(struct sprd_pms *pms)
{
	struct sprd_mpm_data *mpm = (struct sprd_mpm_data *)pms->data;

	pr_debug("pms: %s relax awake.\n", pms->name);

	if (pms->awake) {
		pms->awake = false;
		sprd_mpm_relax(mpm);
	}
}

/**
 * sprd_pms_relax_wakelock_timer
 * the timer process function of pms delay release wakelock.
 */
static void sprd_pms_relax_wakelock_timer(
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
					unsigned long data)
{
	struct sprd_pms *pms = (struct sprd_pms *)data;
#else
					struct timer_list *t)
{
	struct sprd_pms *pms = from_timer(pms, t, wake_timer);
#endif

	unsigned long flags;
	bool relax = false;

	pr_debug("pms: %s timer down.\n", pms->name);

	spin_lock_irqsave(&pms->expires_lock, flags);
	/*
	 * if jiffies < pms->expires, mpm called has been canceled,
	 * don't call sprd_pms_down.
	 */
	if (pms->expires && time_after_eq(jiffies, pms->expires)) {
		pms->expires = 0;
		relax = true;
	}
	spin_unlock_irqrestore(&pms->expires_lock, flags);

	if (relax)
		sprd_pms_relax(pms);
}

int sprd_mpm_create(unsigned int dst, const char *name,
		    unsigned int later_idle)
{
	struct sprd_mpm_data *mpm;

	if (dst >= SIPC_ID_NR)
		return -EINVAL;

	mpm = kzalloc(sizeof(*mpm), GFP_KERNEL);
	if (!mpm)
		return -ENOMEM;

	snprintf(mpm->name, sizeof(mpm->name), "%s-mpm-%d", name, dst);

	mpm->dst = dst;
	mpm->later_idle = later_idle;

	spin_lock_init(&mpm->mpm_lock);
	INIT_LIST_HEAD(&mpm->pms_list);
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,18,0 ))
	mpm->ws = wakeup_source_register(NULL, mpm->name);
#else
	wakeup_source_init(&mpm->ws, mpm->name);
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
	setup_timer(&mpm->timer,
		    sprd_mpm_deactive_timer_fn,
		    (unsigned long)mpm);
#else
	timer_setup(&mpm->timer,
		    sprd_mpm_deactive_timer_fn,
		    0);
#endif

	INIT_WORK(&mpm->request_res_work, sprd_mpm_request_res_work_fn);
	INIT_WORK(&mpm->release_res_work, sprd_mpm_release_res_work_fn);

	g_sprd_mpm[dst] = mpm;

	return 0;
}

int sprd_mpm_init_resource_ops(unsigned int dst,
				int (*wait_resource)(unsigned int dst,
						     int timeout),
				int (*request_resource)(unsigned int dst),
				int (*release_resource)(unsigned int dst))
{
	struct sprd_mpm_data *mpm;

	if (dst >= SIPC_ID_NR)
		return -EINVAL;

	mpm = g_sprd_mpm[dst];
	if (!mpm)
		return -ENODEV;

	mpm->wait_resource = wait_resource;
	mpm->request_resource = request_resource;
	mpm->release_resource = release_resource;

	return 0;
}

int sprd_mpm_destroy(unsigned int dst)
{
	struct sprd_pms *pms, *temp;
	struct sprd_mpm_data *mpm;
	unsigned long flags;

	if (dst >= SIPC_ID_NR)
		return -EINVAL;

	mpm = g_sprd_mpm[dst];
	if (!mpm)
		return -ENODEV;

	sprd_mpm_cancel_timer(mpm);
	cancel_work_sync(&mpm->request_res_work);
	cancel_work_sync(&mpm->release_res_work);

	spin_lock_irqsave(&mpm->mpm_lock, flags);
	list_for_each_entry_safe(pms,
				 temp,
				 &mpm->pms_list,
				 entry) {
		sprd_pms_cancel_timer(pms);
		list_del(&pms->entry);
	}
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	kfree(mpm);
	g_sprd_mpm[dst] = NULL;

	return 0;
}

struct sprd_pms *sprd_pms_create(unsigned int dst,
				 const char *name, bool multitask)
{
	unsigned long flags;
	struct sprd_pms *pms;
	struct sprd_mpm_data *mpm;

	if (dst >= SIPC_ID_NR)
		return NULL;

	mpm = g_sprd_mpm[dst];
	if (!mpm) {
		pr_err("mpm: %s pms init failed, dst=%d.\n", name, dst);
		return NULL;
	}

	pms = kzalloc(sizeof(*pms), GFP_KERNEL);
	if (!pms)
		return NULL;

	pms->multitask = multitask;
	pms->name = name;
	pms->data = (void *)mpm;

	spin_lock_init(&pms->expires_lock);
	spin_lock_init(&pms->active_lock);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
	setup_timer(&pms->wake_timer,
		    sprd_pms_relax_wakelock_timer, (unsigned long)pms);
#else
	timer_setup(&pms->wake_timer,
		    sprd_pms_relax_wakelock_timer, 0);
#endif

	spin_lock_irqsave(&mpm->mpm_lock, flags);
	list_add(&pms->entry, &mpm->pms_list);
	spin_unlock_irqrestore(&mpm->mpm_lock, flags);

	return pms;
}

void sprd_pms_destroy(struct sprd_pms *pms)
{
	unsigned long flags;
	struct sprd_mpm_data *mpm;

	if (pms) {
		sprd_pms_cancel_timer(pms);
		mpm = (struct sprd_mpm_data *)pms->data;
		spin_lock_irqsave(&mpm->mpm_lock, flags);
		list_del(&pms->entry);
		spin_unlock_irqrestore(&mpm->mpm_lock, flags);
		kfree(pms);
	}
}

/**
 * sprd_pms_request_resource - request mpm resource
 *
 * @pms, the point of this pms.
 * @timeout, in ms.
 *
 *  Returns:
 *  0 resource ready,
 *  < 0 resoure not ready,
 *  -%ERESTARTSYS if it was interrupted by a signal.
 */
int sprd_pms_request_resource(struct sprd_pms *pms, int timeout)
{
	int ret;
	struct sprd_mpm_data *mpm;

	if (!pms)
		return -EINVAL;

	sprd_pms_do_up(pms);

	/* wait resource */
	mpm = (struct sprd_mpm_data *)pms->data;
	ret = sprd_mpm_wait_resource(mpm, timeout);
	if (ret)
		sprd_pms_do_down(pms, false);

	return ret;
}

/**
 * sprd_pms_release_resource - release mpm resource.
 *
 * @pms, the point of this pms.
 */
void sprd_pms_release_resource(struct sprd_pms *pms)
{
	if (pms)
		sprd_pms_do_down(pms, false);
}

/**
 * sprd_pms_request_wakelock - request wakelock
 *
 * @pms, the point of this pms.
 */
void sprd_pms_request_wakelock(struct sprd_pms *pms)
{
	if (pms) {
		sprd_pms_cancel_timer(pms);
		sprd_pms_stay_awake(pms);
	}
}

/**
 * sprd_pms_release_wakelock - release wakelock
 *
 * @pms, the point of this pms.
 */
void sprd_pms_release_wakelock(struct sprd_pms *pms)
{
	if (pms) {
		sprd_pms_cancel_timer(pms);
		sprd_pms_relax(pms);
	}
}

/**
 * sprd_pms_request_wakelock_period -
 * request wake lock, and will auto reaslse in msec ms.
 *
 * @pms, the point of this pms.
 * @msec, will auto reaslse in msec ms
 */
void sprd_pms_request_wakelock_period(struct sprd_pms *pms, unsigned int msec)
{
	sprd_pms_request_wakelock(pms);
	sprd_pms_release_wakelock_later(pms, msec);
}

/**
 * sprd_pms_release_wakelock_later - release wakelock later.
 *
 * @pms, the point of this pms.
 * @msec, later time (in ms).
 */
void sprd_pms_release_wakelock_later(struct sprd_pms *pms,
				     unsigned int msec)
{
	unsigned long expires;
	unsigned long flags;

	if (pms) {
		pr_debug("pms: %s release wakelock after %d ms.\n",
			 pms->name, msec);

		spin_lock_irqsave(&pms->expires_lock, flags);
		expires = jiffies + msecs_to_jiffies(msec);
		if (!expires)
			expires = 1;

		/* always update the timer with new time */
		pms->expires = expires;
		mod_timer(&pms->wake_timer, expires);
		spin_unlock_irqrestore(&pms->expires_lock, flags);
	}
}

void sprd_pms_power_up(struct sprd_pms *pms)
{
	if (pms)
		sprd_pms_do_up(pms);
}

void sprd_pms_power_down(struct sprd_pms *pms, bool immediately)
{
	if (pms)
		sprd_pms_do_down(pms, immediately);
}

#if defined(CONFIG_DEBUG_FS)
static int sprd_mpm_stats_show(struct seq_file *m, void *unused)
{
	unsigned long flags;
	struct sprd_pms *pms;
	struct sprd_mpm_data *cur;
	unsigned int i, ms;

	seq_puts(m, "---------------------------------------------\n");
	seq_puts(m, "All mpm list:\n");

	for (i = 0; i < SIPC_ID_NR; i++) {
		if (!g_sprd_mpm[i])
			continue;

		cur = g_sprd_mpm[i];
		seq_puts(m, "------------------------------------\n");
		seq_printf(m, "mpm = %s info:\n", cur->name);
		seq_printf(m, "last up module = %s info:\n",
			cur->last_name ? cur->last_name : "null");

		if (cur->expires > 0) {
			ms = jiffies_to_msecs(cur->expires - jiffies);
			seq_printf(m, "left %d ms to idle\n", ms);
		}

		seq_printf(m, "up_cnt=%d, state=%d.\n",
			 cur->up_cnt, cur->mpm_state);
		seq_printf(m, "wakelock_cnt=%d, awake_cnt=%d\n",
			cur->wakelock_cnt, cur->awake_cnt);
		seq_puts(m, "------------------------------------\n");

		seq_puts(m, "active pms list:\n");
		spin_lock_irqsave(&cur->mpm_lock, flags);
		list_for_each_entry(pms, &cur->pms_list, entry) {
			if (!pms->active_cnt && !pms->awake)
				continue;

			seq_printf(m, "  %s: active_cnt=%d, awake=%d\n",
				pms->name, pms->active_cnt, pms->awake);
		}
		spin_unlock_irqrestore(&cur->mpm_lock, flags);
	}

	seq_puts(m, "---------------------------------------------\n");

	return 0;
}

static int sprd_mpm_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, sprd_mpm_stats_show, NULL);
}

static const struct file_operations sprd_mpm_stats_fops = {
	.owner = THIS_MODULE,
	.open = sprd_mpm_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int sprd_mpm_init_debugfs(void)
{
	struct dentry *root = debugfs_create_dir("mpm", NULL);

	if (!root)
		return -ENXIO;

	debugfs_create_file("power_manage", 0444,
			    (struct dentry *)root,
			    NULL, &sprd_mpm_stats_fops);
	return 0;
}
#endif


int modem_power_manager_init(void)
{
	register_pm_notifier(&sprd_mpm_notifier_block);

#if defined(CONFIG_DEBUG_FS)
	sprd_mpm_init_debugfs();
#endif

	return 0;
}
EXPORT_SYMBOL(modem_power_manager_init);

void modem_power_manager_exit(void)
{
	unregister_pm_notifier(&sprd_mpm_notifier_block);
}
EXPORT_SYMBOL(modem_power_manager_exit);
