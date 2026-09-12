// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include "phytium_npu.h"
#include "phytium_npu_mmu.h"
#include "linux/phytium_npu_dma_buf_heap.h"
#include "linux/dma-fence.h"
#include "linux/dma-buf.h"
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/iopoll.h>
#include <linux/acpi.h>
#include <acpi/acpi_lpat.h>
#include <acpi/acpi_bus.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include "phytium_npu_pd2408_reg.h"

struct phytium_npu_dev *gnpu_dev;
#define AP_CPPC2_STAT		0x0
#define AP_CPPC2_SET		0x4
#define CPPC2_DATA0		0x18
#define CPPC2_DATA1		0x1C
#define CONFIG_POWER_D0		0x110400
#define CONFIG_POWER_ON_D1	0x40000008
#define CONFIG_POWER_OFF_D1	0x40000000
#define TIMEOUT		50000 //test for X1
#define PERIOD_T	10

#define NPU_MMU_40BIT		40
#define MMU_DEFAULT_TRANS	0
#define MMU_DEFAULT_CTX_ID	0
#define MMU_DEFAULT_PGSIZE	4096

struct phytium_npu_dev *phytium_npu_get_npudev(void)
{
	return gnpu_dev;
}

static void phytium_npu_mmu_config_init(struct phytium_npu_dev *npu_dev)
{
	npu_dev->nmmu_config.width = NPU_MMU_40BIT;
	npu_dev->nmmu_config.mode_bypass = MMU_DEFAULT_TRANS;
	npu_dev->nmmu_config.is_use_mmu_pte = FALSE;
	npu_dev->nmmu_config.default_ctx_id = MMU_DEFAULT_CTX_ID;
	npu_dev->nmmu_config.page_size = MMU_DEFAULT_PGSIZE;
}

static void _phytium_npu_poweroff_dtb(struct phytium_npu_dev *npudev)
{
	u32 stat;
	int rc;

	if (!npudev->power_reg_base) {
		pr_err("not find power management reg");
		return;
	}
	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + AP_CPPC2_STAT,
					stat, (stat == 0), PERIOD_T, TIMEOUT);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when read CPPC stat: %#08x\n", stat);
		return;
	}
	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + CPPC2_DATA0,
					stat, (stat & 0x1), PERIOD_T, TIMEOUT * 50);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when config channel status: %#08x\n", stat);
		return;
	}

	writel_relaxed(CONFIG_POWER_D0, npudev->power_reg_base + CPPC2_DATA0);
	writel_relaxed(CONFIG_POWER_OFF_D1, npudev->power_reg_base + CPPC2_DATA1);
	writel_relaxed(0x1, npudev->power_reg_base + AP_CPPC2_SET);

	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + CPPC2_DATA0,
					stat, (stat & 0x1), PERIOD_T, TIMEOUT * 50);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when read config channel status: %#08x\n", stat);
		return;
	}
	stat = readl_relaxed(npudev->power_reg_base + CPPC2_DATA1);
	if (!(stat & 0xFFFFFFF))
		pr_debug("Power off");
}

static void _phytium_npu_powerup_dtb(struct phytium_npu_dev *npudev)
{
	u32 stat;
	int rc;

	if (!npudev->power_reg_base) {
		pr_err("not find power management reg");
		return;
	}
	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + AP_CPPC2_STAT,
					stat, (stat == 0), PERIOD_T, TIMEOUT);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when read CPPC stat: %#08x\n", stat);
		return;
	}
	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + CPPC2_DATA0,
					stat, (stat & 0x1), PERIOD_T, TIMEOUT);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when read config channel status: %#08x\n", stat);
		return;
	}

	writel_relaxed(CONFIG_POWER_D0, npudev->power_reg_base + CPPC2_DATA0);
	writel_relaxed(CONFIG_POWER_ON_D1, npudev->power_reg_base + CPPC2_DATA1);
	writel_relaxed(0x1, npudev->power_reg_base + AP_CPPC2_SET);

	rc = readl_relaxed_poll_timeout(npudev->power_reg_base + CPPC2_DATA0,
					stat, (stat & 0x1), PERIOD_T, TIMEOUT * 5);
	if (rc == -ETIMEDOUT) {
		pr_err("Timeout when read config channel status: %#08x\n", stat);
		return;
	}
	stat = readl_relaxed(npudev->power_reg_base + CPPC2_DATA1);
	if ((stat & 0xFFFFFFF) == 0x8)
		pr_debug("Power on");
}

static void _phytium_npu_power_manager_acpi(struct phytium_npu_dev *npudev, int on)
{
	acpi_handle handle = ACPI_HANDLE(npudev->dev);
	union acpi_object args[3];
	acpi_status status;
	unsigned long long rv;

	struct acpi_object_list arg_list = {
		.pointer = args,
		.count = ARRAY_SIZE(args),
	};

	if (!handle)
		return;

	pr_debug("set npu power %d\n", on);
	args[0].type = ACPI_TYPE_INTEGER;
	args[0].integer.value = NPU_ACPI_POWER_TYPE;
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = 0;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = 0;

	if (on) {
		status = acpi_evaluate_integer(handle, "PPWO", &arg_list, &rv);
		if (ACPI_FAILURE(status))
			pr_err("NO PS0 Method\n");
		if (rv)
			pr_err("Failed to power on\n");
		pr_debug("acpi_evaluate_integer PPWO ok\n");
	} else {
		status = acpi_evaluate_integer(handle, "PPWD", &arg_list, &rv);
		if (ACPI_FAILURE(status))
			pr_err("NO PS3 Method\n");

		if (rv)
			pr_err("Failed to power off\n");
		pr_debug("acpi_evaluate_integer PPWD OK\n");
	}
}

static void _phytium_npu_get_voltage_acpi(struct phytium_npu_dev *npudev, int *voltage)
{
	acpi_handle handle = ACPI_HANDLE(npudev->dev);
	union acpi_object args[3];
	acpi_status status;
	unsigned long long rv;

	struct acpi_object_list arg_list = {
		.pointer = args,
		.count = ARRAY_SIZE(args),
	};

	if (!handle)
		return;

	pr_debug("Get NPU voltage\n");
	args[0].type = ACPI_TYPE_INTEGER;
	args[0].integer.value = NPU_ACPI_POWER_TYPE;
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = 0;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = 0;

	status = acpi_evaluate_integer(handle, "PGDV", &arg_list, &rv);
	if (ACPI_FAILURE(status))
		pr_err("NO PGDV Method\n");
	if (rv < 0)
		pr_err("Failed to get voltage\n");
	pr_debug("Get PGDV ok,voltage:%llu\n", rv);
	if (rv & 0x80000000)
		*voltage = -1;
	else
		*voltage = (int)rv;
}

void _phytium_npu_voltage_read(struct phytium_npu_dev *npudev)
{
	if (!npudev->voltage_val)
		_phytium_npu_get_voltage_acpi(npudev, &npudev->voltage_val);
	pr_debug("voltage value:%d", npudev->voltage_val);
}

static void _phytium_npu_poweroff(struct phytium_npu_dev *npudev)
{
	if (npudev->power_reg_base)
		_phytium_npu_poweroff_dtb(npudev);
	else
		_phytium_npu_power_manager_acpi(npudev, FALSE);
}

static void _phytium_npu_powerup(struct phytium_npu_dev *npudev)
{
	if (npudev->power_reg_base)
		_phytium_npu_powerup_dtb(npudev);
	else
		_phytium_npu_power_manager_acpi(npudev, TRUE);
}

static int phytium_npu_resume(struct phytium_npu_dev *npudev)
{
	_phytium_npu_powerup(npudev);
	phytium_npu_hw_reset_self(npudev);
	_phytium_npu_voltage_read(npudev);
	pr_debug("NPU set power up Ok, start npu hw reset");
	return 0;
}

static int phytium_npu_suspend(struct phytium_npu_dev *npudev)
{
	phytium_npu_config_clock(npudev, FALSE);
	_phytium_npu_poweroff(npudev);

	return 0;
}

void phytium_npu_try_resume_work(struct phytium_npu_dev *npudev)
{
	dev_dbg(npudev->dev, "%s: resume npu!\n", __func__);
	if (npudev->power_status == NPU_STATE_ON) {
		if (cancel_delayed_work_sync(&npudev->rt_delay_work))
			pr_debug("cancel_delay_work success");
		else
			pr_debug("cancel delay work failed");
	} else if (npudev->power_status == NPU_STATE_OFF) {
		phytium_npu_resume(npudev);
		npudev->power_status = NPU_STATE_ON;
		npudev->load_status = 100;
	}
}

static void phytium_npu_runtime_suspend_work(struct work_struct *work)
{
	struct phytium_npu_dev *npudev =
			container_of(work, struct phytium_npu_dev, rt_delay_work.work);

	mutex_lock(&npudev->mutex_lock);
	dev_dbg(npudev->dev, "%s: runtime expired!\n", __func__);

	if (npudev->power_status == NPU_STATE_ON) {
		phytium_npu_suspend(npudev);
		npudev->power_status = NPU_STATE_OFF;
		npudev->load_status = NPU_STATE_OFF;
	} else {
		pr_info("NPU is poweroff");
	}
	mutex_unlock(&npudev->mutex_lock);
}

int phytium_npu_schedule_suspend(struct phytium_npu_dev *npudev, uint32_t delay_ms)
{
	unsigned long cur_time = jiffies + msecs_to_jiffies(delay_ms);
	int ret;

	ret = schedule_delayed_work(&npudev->rt_delay_work, cur_time - jiffies);
	if (!ret) {
		mod_delayed_work(system_wq, &npudev->rt_delay_work, cur_time - jiffies);
		pr_debug("Restart delay work");
	}
	return ret;
}

static int phytium_npu_common_init(struct phytium_npu_dev *npu)
{
	int ret = 0;

	/* config power state */
	npu->power_status = NPU_STATE_OFF;
	npu->load_status = NPU_STATE_OFF;
	npu->voltage_val = 0;
	phytium_npu_try_resume_work(npu);
	phytium_npu_mmu_config_init(npu);
	phytium_npu_mmu_dev_init(npu, npu->nmmu_config.page_size);

	INIT_DELAYED_WORK(&npu->rt_delay_work, phytium_npu_runtime_suspend_work);
	INIT_WORK(&npu->stream_work, do_work);
	npu->stream_wq = alloc_workqueue("npu-stream", WQ_HIGHPRI | WQ_UNBOUND,
					 WQ_UNBOUND_MAX_ACTIVE);
	if (!npu->stream_wq) {
		ret = -ENOMEM;
		return ret;
	}
	npu->is_cache_stream_on = TRUE;
	mutex_init(&npu->mutex_lock);
	spin_lock_init(&npu->spin_irq_lock);
	phytium_npu_hw_reset_self(npu);
	phytium_npu_config_preload(npu);
	phytium_npu_debugfs_init(npu);
	phytium_npu_schedule_suspend(npu, NPU_AUTO_SUSPEND_TIMEOUT);
	return ret;
}

int phytium_npu_register_dev(struct device *dev, void *plat_data, void __iomem *reg_base)
{
	int ret;

	struct phytium_npu_dev *npu = devm_kzalloc(dev, sizeof(struct phytium_npu_dev), GFP_KERNEL);

	if (!npu) {
		ret = -ENOMEM;
		goto free_dev;
	}
	gnpu_dev = npu;
	npu->reg_base = reg_base;
	npu->power_reg_base = plat_data;
	npu->activated_stream = NULL;
	npu->queued_stream = NULL;
	INIT_LIST_HEAD(&npu->sessions_list);
	INIT_LIST_HEAD(&npu->sched_sess_list);
	dev_set_drvdata(dev, npu);
	npu->dev = dev;
	ret = phytium_npu_register_misc(npu);
	if (ret) {
		dev_err(dev, "%s: failed to add npu dev!", __func__);
		goto free_dev;
	}

	phytium_npu_common_init(npu);

	return ret;

free_dev:
	gnpu_dev = NULL;
	devm_kfree(dev, npu);
	return ret;
}
EXPORT_SYMBOL(phytium_npu_register_dev);

int phytium_npu_unregister_dev(struct device *dev)
{
	struct phytium_npu_dev *npudev = dev_get_drvdata(dev);
	int ret;

	WARN_ON(!npudev);
	if (!npudev) {
		pr_err("npudev is NULL");
		return -EINVAL;
	}

	flush_workqueue(npudev->stream_wq);
	destroy_workqueue(npudev->stream_wq);
	cancel_delayed_work_sync(&npudev->rt_delay_work);
	phytium_npu_session_release_all(npudev);
	phytium_npu_debugfs_remove(npudev);
	ret = phytium_npu_unregister_misc(npudev);
	if (ret) {
		pr_err("unregister misc npu err");
		return ret;
	}
	devm_kfree(dev, npudev);
	return ret;
}
EXPORT_SYMBOL(phytium_npu_unregister_dev);

static int phytium_npu_response_stream(struct phytium_npu_session *sess,
				       struct phytium_npu_stream *nstream,
				       int sid, int err_no, int errflag)
{
	struct npu_user_stream_rsp *nustream_rsp;
	struct npu_user_submit_stream *nustream = &nstream->nustream;
	int size, alloc_size;

	alloc_size = sizeof(*nustream_rsp);
	if (nstream->nustream.estream.stype == NPU_STREAM_SUBMIT) {
		size = MAX_NPU_UCNN_RSP_SIZE;
		alloc_size += (MAX_NPU_UCNN_RSP_SIZE - MAX_NPU_USER_RSP_SIZE);
	} else {
		size = MAX_NPU_USER_RSP_SIZE;
	}
	nustream_rsp = kzalloc(alloc_size, GFP_KERNEL);
	if (!nustream_rsp)
		return -ENOMEM;

	if (nustream->estream.stype & NPU_COMPUTEFLAG_NOTIFY) {
		if (!nstream->rsp) {
			nstream->rsp = nustream_rsp;
			INIT_LIST_HEAD(&nustream_rsp->stream_rsp_list_entry);
		}

		nustream_rsp->ursp.sid = sid;
		nustream_rsp->ursp.err_no = err_no;
		nustream_rsp->ursp.rsp_err_flags = errflag;
		nustream_rsp->ursp.session_id = sess->id;
		nustream_rsp->rsp_size = size;
		nustream_rsp->session = sess;

		list_add_tail(&nustream_rsp->stream_rsp_list_entry, &sess->response_list);
		pr_debug("session id:%d response stream id:%#x, err_no:%d errflag:%x",
			 sess->id, sid, err_no, errflag);
		wake_up(&sess->response_wq);
		if (!sess->npu_dev->is_use_repeat) {
			nustream_rsp->session = NULL;
			nstream->rsp = NULL;
			list_del(&nstream->stream_list_entry);
			kfree(nstream);
			pr_debug("free stream id:%#x", sid);
		}
	}

	return 0;
}

void phytium_npu_repeat_stream(struct phytium_npu_session *sess, int stream_id, int is_repeat)
{
	struct phytium_npu_stream *curr_stream, *next_stream;
	struct npu_user_submit_stream *nustream;
	int is_find = 0;

	list_for_each_entry_safe(curr_stream, next_stream, &sess->stream_list, stream_list_entry) {
		nustream = &curr_stream->nustream;
		if (nustream->estream.sid == stream_id && is_repeat) {
			curr_stream->stream_status = NPU_STREAM_NONE;
			curr_stream->infer_status = NPU_STREAM_INFER_NONE;
			is_find = 1;
		}
	}

	if (is_find)
		pr_debug("find stream id %d and repeat %s", stream_id,
			 is_repeat ? "TRUE" : "FALSE");
}

static inline void
phytium_npu_set_dma_buf_status(struct npu_unified_heap_buffer *buffer, int status)
{
	buffer->buffer_status = status;
}

void phytium_npu_set_stream_buf_status_with_fd(struct phytium_npu_session *sess, int fd, int status)
{
	struct npu_mctx_map *nmmap;

	nmmap = phytium_npu_find_mmu_ctx_map(sess, fd);
	if (nmmap) {
		phytium_npu_set_dma_buf_status(nmmap->dma_buf->priv, status);
		pr_debug("Set dma buffer %d status %d", fd, status);
	}
}

static void phytium_npu_update_stream_buf_status(struct phytium_npu_stream *nstream)
{
	struct npu_user_submit_stream *nustream = &nstream->nustream;
	struct npu_excute_stream *estream = &nustream->estream;
	struct npu_mctx_map *nmmap;
	size_t i;

	for (i = 0; i < estream->in; i++) {
		nmmap = phytium_npu_find_mmu_ctx_map(nstream->session, estream->fd[i]);
		if (nmmap) {
			phytium_npu_set_dma_buf_status(nmmap->dma_buf->priv, NPU_BUF_UPDATED_BY_HW);
			pr_debug("Stream complete and set dma buffer %d status %d",
				 estream->fd[i], NPU_BUF_UPDATED_BY_HW);
		}
	}
}

int phytium_npu_check_stream_buf_is_ready(struct phytium_npu_stream *nstream)
{
	struct npu_user_submit_stream *nustream = &nstream->nustream;
	struct npu_excute_stream *estream = &nustream->estream;
	struct npu_mctx_map *nmmap;
	struct npu_unified_heap_buffer *buffer;
	size_t i;

	if (nstream->all_buf_ready)
		return TRUE;

	for (i = 0; i < estream->in; i++) {
		nmmap = phytium_npu_find_mmu_ctx_map(nstream->session, estream->fd[i]);
		if (nmmap) {
			buffer = nmmap->dma_buf->priv;
			if (buffer->buffer_status == NPU_BUF_UNUPDATE)
				return FALSE;
		} else {
			return FALSE;
		}
	}
	nstream->all_buf_ready = TRUE;
	pr_debug("Set stream %d  all buf ready status %d", estream->fd[i], nstream->all_buf_ready);
	return TRUE;
}

int phytium_npu_prepare_hw_4_queued_stream(struct phytium_npu_dev *npu,
					   struct phytium_npu_session *sess,
								struct phytium_npu_stream *nstream)
{
	phytium_npu_mmu_config_dev_mmu(sess);
	phytium_npu_config_dma_address(sess, nstream);
	return 0;
}

int phytium_npu_submit_stream(struct phytium_npu_dev *npu,
			      struct phytium_npu_session *sess,
								struct phytium_npu_stream *nstream)
{
	GET_TIME_NS(&npu->ts);
	phytium_npu_try_resume_work(npu);
	phytium_npu_config_clock(npu, TRUE);
	phytium_npu_config_hl_wdt(npu);
	phytium_npu_mmu_config_dev_mmu(sess);
	phytium_npu_config_dma_address(sess, nstream);
	phytium_npu_clear_irq_status(npu, NPU_ALL_EVENT);
	phytium_npu_debug_set_hw_register(npu, npu->activated_stream->session);
	phytium_npu_config_event(npu, NPU_ALL_EVENT, TRUE); /* enable all event */
	phytium_npu_config_start_inference(npu, sess, nstream);
	GET_TIME_NS(&npu->te);
	phytium_npu_get_time_span(&npu->ts, &npu->te, &npu->tspan);
	GET_TIME_NS(&npu->ts);
	return 0;
}

static void phytium_npu_inference_complete(struct phytium_npu_dev *npudev)
{
	struct phytium_npu_stream *nstream = npudev->activated_stream;
	struct phytium_npu_session *sess;

	if ((npudev->irq_status != NPU_INFERENCE_COMPLETE_EVENT) || !nstream)
		return;
	sess = nstream->session;
	phytium_npu_update_stream_buf_status(nstream);
	nstream->infer_status = NPU_STREAM_INFER_DONE;
	GET_TIME_NS(&npudev->te);
	phytium_npu_get_time_span(&npudev->ts, &npudev->te, &npudev->tspan);
	phytium_npu_response_stream(sess, nstream, nstream->nustream.estream.sid, NPU_RSP_OK, 0);
	phytium_npu_debug_show_register(npudev, sess);
	phytium_npu_schedule_suspend(npudev, NPU_AUTO_SUSPEND_TIMEOUT);
}

int phytium_npu_common_resume(struct device *dev)
{
	struct phytium_npu_dev *npudev = dev_get_drvdata(dev);

	mutex_lock(&npudev->mutex_lock);
	_phytium_npu_powerup(npudev);
	phytium_npu_schedule_stream_queues(npudev, TRUE);
	mutex_unlock(&npudev->mutex_lock);

	return 0;
}
EXPORT_SYMBOL(phytium_npu_common_resume);

int phytium_npu_common_suspend(struct device *dev)
{
	struct phytium_npu_dev *npudev = dev_get_drvdata(dev);

	mutex_lock(&npudev->mutex_lock);
	phytium_npu_rollback_stream(npudev);
	phytium_npu_config_clock(npudev, FALSE);
	if (!pm_runtime_status_suspended(dev))
		pm_runtime_put_sync(dev);
	_phytium_npu_poweroff(npudev);
	mutex_unlock(&npudev->mutex_lock);
	return 0;
}
EXPORT_SYMBOL(phytium_npu_common_suspend);

/* top half */
int phytium_npu_handle_irq(struct device *dev)
{
	struct phytium_npu_dev *npu_dev = dev_get_drvdata(dev);
	int status;
	int irq_mask = phytium_npu_get_irq_enable_status(npu_dev);

	GET_TIME_NS(&npu_dev->te);
	phytium_npu_get_time_span(&npu_dev->ts, &npu_dev->te, &npu_dev->tspan);
	GET_TIME_NS(&npu_dev->ts);
	status = phytium_npu_get_irq_status(npu_dev);
	if (!status) {
		pr_debug("No irq here.");
		return IRQ_NONE;
	}
	//clear msi irq status with PCI interface
	if (!npu_dev->is_platform_dev)
		phytium_npu_clear_msi_irq(npu_dev);

	if (!(status & irq_mask))
		return IRQ_NONE;

	phytium_npu_clear_irq_status(npu_dev, status);
	spin_lock(&npu_dev->spin_irq_lock);
	npu_dev->irq_status = status & irq_mask;
	spin_unlock(&npu_dev->spin_irq_lock);
	dev_dbg(npu_dev->dev, "irq status (%#x), bottom status:%#x", status, npu_dev->irq_status);
	return IRQ_WAKE_THREAD;
}
EXPORT_SYMBOL(phytium_npu_handle_irq);

/* bottom half */
int phytium_npu_handle_thread_irq(struct device *dev)
{
	struct phytium_npu_dev *npu_dev = dev_get_drvdata(dev);
	struct phytium_npu_stream *nstream = npu_dev->activated_stream;
	int err = 0, err_flag = 0, status;

	mutex_lock(&npu_dev->mutex_lock);
	spin_lock(&npu_dev->spin_irq_lock);
	status = npu_dev->irq_status;
	spin_unlock(&npu_dev->spin_irq_lock);
	if (status & (NPU_AXI_EVENT | NPU_ERR_EVENT)) {
		/* must reset hw */
		err = npu_dev->irq_status & (NPU_AXI_EVENT | NPU_ERR_EVENT);
		err_flag |= NPU_RSP_ERROR_HW_SYS_AXI_ERROR_SHF;
		phytium_npu_config_event(npu_dev, 0, 0); /* disable event*/
		phytium_npu_clear_irq_status(npu_dev, NPU_ALL_EVENT);
		phytium_npu_hw_reset_self(npu_dev);
	}

	if (status & NPU_WDT_EVENT) {
		err |= status & NPU_WDT_EVENT;
		err_flag |= NPU_RSP_ERROR_SW_WDT_EXPIRED_SHF;
	}

	if (status & NPU_MMU_EVENT) {
		err |= status & NPU_MMU_EVENT;
		err_flag |= NPU_RSP_ERROR_HW_SYS_MMU_PAGE_FAULT_SHF;
		phytium_npu_mmu_output_status(npu_dev);
	}

	if (err) {
		dev_err(npu_dev->dev, "NPU hardware err %#x ", err);
		if (err & NPU_AXI_EVENT)
			dev_err(npu_dev->dev, "AXI ERR %#x",
				phytium_npu_get_axi_err_status(npu_dev));
		if (err & NPU_MEM_WDT_EVENT)
			phytium_npu_output_mem_wdt_err(npu_dev);

		if (err & NPU_ERR_EVENT) {
			//TODO. will do rollback
			phytium_npu_rollback_stream(npu_dev);
			if (nstream)
				phytium_npu_response_stream(nstream->session, nstream,
							    nstream->nustream.estream.sid,
							    NPU_RSP_ERROR_CRITICAL_SHF,
							    err_flag);
			goto schedule_stream;
		}

		if (err & NPU_MMU_EVENT && nstream) {
			phytium_npu_response_stream(nstream->session, nstream,
						    nstream->nustream.estream.sid,
						NPU_RSP_ERROR_HW_SYS_MMU_PAGE_FAULT_SHF,
						err_flag);
			phytium_npu_update_activated_stream_4_err_mmu(npu_dev);
			phytium_npu_schedule_suspend(npu_dev, NPU_AUTO_SUSPEND_TIMEOUT);
		}
	}

	if (status & NPU_INFERENCT_ERR_EVENT) {
		phytium_npu_rollback_stream(npu_dev);
		if (nstream)
			phytium_npu_response_stream(nstream->session, nstream,
						    nstream->nustream.estream.sid,
						    NPU_RSP_ERROR_HW_CORE_CNN_ERROR_SHF,
						    err_flag);
		goto schedule_stream;
	}

	if (status & NPU_INFERENCE_COMPLETE_EVENT) {
		pr_debug("DEBUG:NPU infers complete here %#x", npu_dev->irq_status);
		phytium_npu_inference_complete(npu_dev);
		phytium_npu_try_excute_queued_stream(npu_dev);
	}

schedule_stream:

	phytium_npu_schedule_stream_queues(npu_dev, FALSE);
	mutex_unlock(&npu_dev->mutex_lock);
	return IRQ_HANDLED;
}
EXPORT_SYMBOL(phytium_npu_handle_thread_irq);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium NPU driver common lib");
MODULE_IMPORT_NS("DMA_BUF");
MODULE_LICENSE("GPL");
