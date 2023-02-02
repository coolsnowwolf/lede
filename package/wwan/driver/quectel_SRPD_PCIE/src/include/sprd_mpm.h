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

 /* MPM: modem power manger
  * PMS: power manage source which be used to request
  * a modem power manage resource.
  */
#ifndef _SPRD_MPM_H
#define _SPRD_MPM_H
/*
 * MPM modem powermanger source state define,
 * if in idle state, we can release
 * the related resources(such as pcie) of modem.
 */
enum {
	SPRD_MPM_IDLE = 0,
	SPRD_MPM_BUSY
};

/*
 * @sprd_pms: the power manager source data struct,
 * can usd it to request wake lock or request modem resource.
 *
 * @name: the name of a pms.
 * @data: the point of MPM.
 * @multitask: whether to support multitasking, default is false.
 * false, the source can only be used in single task context.
 * true, the source can be used  multitask context.
 * @awake: whether stay awake.
 * @awake_cnt: total awake times.
 * @pre_awake_cnt pre_awake_cnt.
 * @active_cnt: the active counter of the pms.
 * @expires: the timer expires value.
 * @active_lock:  use for protect the active_cnt member.
 * @expires_lock:  use for protect expires member.
 * @entry: an entry of all pms list.
 * @wake_timer: used for delay release wakelock.
 */
struct sprd_pms {
	const char	*name;
	void		*data;
	bool		multitask;
	bool			awake;
	unsigned int	awake_cnt;
	unsigned int	pre_awake_cnt;
	unsigned int	active_cnt;
	unsigned long	expires;
	spinlock_t	active_lock;
	spinlock_t	expires_lock;
	struct list_head	entry;
	struct timer_list	wake_timer;
};

/**
 * sprd_mpm_create - create a modem powermanger source instacnce.
 *
 * @dst, which mpm (PSCP, SP, WCN, etc.) will be created.
 * @later_idle, will release resource later (in ms).
 */
int sprd_mpm_create(unsigned int dst,
		    const char *name,
		    unsigned int later_idle);

/**
 * sprd_mpm_init_resource_ops - int resource ops for mpm.
 *
 * @wait_resource, used to wait request resource ready.
 * @request_resource, used to request a resource
 * @release_resource, used to release a resource
 */
int sprd_mpm_init_resource_ops(unsigned int dst,
			       int (*wait_resource)(unsigned int dst,
						    int timeout),
			       int (*request_resource)(unsigned int dst),
			       int (*release_resource)(unsigned int dst));

/**
 * sprd_mpm_destroy - destroy a modem powermanger source instacnce.
 *
 * @dst, which mpm (PSCP, SP, WCN, etc.) will be destroyed.
 */
int sprd_mpm_destroy(unsigned int dst);

/**
 * sprd_pms_create - init a pms,
 * a module which used it to request a modem power manage resource.
 * All the pms interface are not safe in multi-thread or multi-cpu.
 * if you want use in multi-thread, please use the pms_ext interface.
 *
 * @dst, the pms belong to which mpm.
 * @name, the name of this pms.
 * @pms, the point of this pms.
 * @multitask: support  multitask.
 *
 * Returns: NULL failed, > 0 succ.
 */
struct sprd_pms *sprd_pms_create(unsigned int dst,
				 const char *name, bool multitask);

/**
 * sprd_pms_destroy - destroy a pms.
 *
 * @pms, the point of this pms.
 */
void sprd_pms_destroy(struct sprd_pms *pms);

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
int sprd_pms_request_resource(struct sprd_pms *pms, int timeout);

/**
 * sprd_pms_release_resource - release mpm resource.
 *
 * @pms, the point of this pms.
 */
void sprd_pms_release_resource(struct sprd_pms *pms);

/**
 * sprd_pms_request_wakelock - request wakelock
 *
 * @pms, the point of this pms.
 */
void sprd_pms_request_wakelock(struct sprd_pms *pms);

/**
 * sprd_pms_release_wakelock - release wakelock
 *
 * @pms, the point of this pms.
 */
void sprd_pms_release_wakelock(struct sprd_pms *pms);

/**
 * sprd_pms_request_wakelock_period -
 * request wake lock, and will auto reaslse in msec ms.
 *
 * @pms, the point of this pms.
 * @msec, will auto reaslse in msec ms
 */
void sprd_pms_request_wakelock_period(struct sprd_pms *pms, unsigned int msec);

/**
 * sprd_pms_release_wakelock_later - release wakelock later.
 *
 * @pms, the point of this pms.
 * @msec, later time (in ms).
 */
void sprd_pms_release_wakelock_later(struct sprd_pms *pms,
				     unsigned int msec);

/**
 * sprd_pms_power_up - just powe up, not wait result.
 *
 * @pms, the point of this pms.
 */
void sprd_pms_power_up(struct sprd_pms *pms);

/**
 * sprd_pms_power_up - just power down,.
 *
 * @pms, the point of this pms.
 * @immediately, whether immediately power down.
 */
void sprd_pms_power_down(struct sprd_pms *pms, bool immediately);

#endif
