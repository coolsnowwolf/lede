/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#ifndef __PHYTIUM_NPU_H__
#define __PHYTIUM_NPU_H__
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/iosys-map.h>
#include "phytium_npu_mmu.h"

#define NPU_FTN300_PCI_DRV_NAME         "NPU_FTN300_pci"
#define NPU_FTN300_PLAT_DRV_NAME        "NPU_FTN300_platform"
#define NPU_FTN300_DRIVER_VERSION       "1.0.1"

#define NPU_SUPPORT_SESSION_NUM		128
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define NPU_STREAM_NONE				0x0
#define NPU_STREAM_IN_HW			0x1
#define NPU_STREAM_SCHEDULED		0x09
#define NPU_STREAM_BUFF_IN_HW		0x10
#define NPU_STREAM_BUFF_NO_READY	0x11
#define NPU_STREAM_BUFF_BUSY		0x12

#define NPU_STREAM_INFER_NONE	0x0
#define NPU_STREAM_INFER_WAIT	0x2
#define NPU_STREAM_INFER_WORK	0x3
#define NPU_STREAM_INFER_DONE	0x4

#define NPU_MMU_CONTEXT_NUM		2
#define NPU_MMU_CONTEXT_MODULE_ID	0
#define NPU_MMU_CONTEXT_IO_ID		1

#define NEED_NOTHING		0x0
#define NEED_SCHEDULE		0x1
#define NEED_QUEUE_STREAM	0x2

#define NPU_AUTO_SUSPEND_TIMEOUT 5000
#define NPU_ACPI_POWER_TYPE	0x1
#define GET_TIME_NS(ts) ktime_get_real_ts64(ts)
/* state of the NPU device: power up or down */
enum npu_power_state {
	NPU_STATE_OFF,
	NPU_STATE_ON
};

struct phytium_npu_stream;

struct phytium_npu_debugfs {
	u32 debug_mode; /* register|mem file */
	/* perf|band|crc|PB_stream|PB_layer|PB_pass|crc_stream|crc_layer|crc|pass */
	u32 debug_type;
	u32 debug_fd; /* debug perf or band memory fd */
	u32 debug_crc_fd; /* debug crc memory fd */
	u32 debug_size;
	u64 debug_addr;
	u64 debug_crc_addr;
	struct iosys_map debug_map;
	struct iosys_map debug_crc_map;
	struct dentry *sess_dbgfs_dir;
};

struct phytium_npu_session {
	struct phytium_npu_dev *npu_dev;
	struct list_head sess_list_entry;
	struct list_head sched_list_entry;
	struct list_head stream_list;
	struct list_head response_list;
	struct list_head map_mmu_list;
	struct phytium_npu_mmu_context mmu_ctx[NPU_MMU_CONTEXT_NUM];
	struct npu_mmu_ctx *share_mctx; //mctx is belong to sesson
	struct phytium_npu_debugfs dbgfs;
	wait_queue_head_t response_wq;
	int id;
};

struct phytium_npu_dev {
	struct platform_device *pdev;
	struct device *dev;
	struct miscdevice miscdev;
	struct list_head sessions_list;
	struct list_head sched_sess_list;
	struct dentry *dbgfs_root_dir;
	struct phytium_npu_stream *activated_stream; /* stream had work on hardware */
	struct phytium_npu_stream *queued_stream; /* stream wait for activate */
	struct workqueue_struct	*stream_wq;
	struct work_struct		stream_work;
	struct delayed_work		rt_delay_work;
	struct npu_mmu_config_global nmmu_config;
	struct mutex	mutex_lock; /* protect stream */
	spinlock_t	spin_irq_lock; /* protect interrupt */
	int used_mmu_context_id[4];
	u32 irq_status;
	int sessions_count;
	int is_cache_stream_on;
	int is_use_repeat;
	int is_platform_dev;
	int irq;
	int power_status;
	int load_status;
	int voltage_val;
	int clock_freq;
	void __iomem *reg_base;
	void __iomem *power_reg_base;
	struct timespec64 ts;
	struct timespec64 te;
	struct timespec64 tspan;
};

struct npu_user_stream_rsp {
	struct phytium_npu_session *session;
	struct list_head stream_rsp_list_entry;
	int rsp_size;
	struct npu_user_rsp ursp;
};

struct phytium_npu_stream {
	struct phytium_npu_session *session;
	struct list_head stream_list_entry;
	struct npu_user_stream_rsp *rsp;
	int stream_status; /* NONE|IN HARDWARE */
	int infer_status; /* NONE|QUEUED|WORK|DONE */
	int user_repeat_count;  /* REPEAT COUNT */
	int all_buf_ready; /* NONE|READY */
	int is_rollback;	/* NONE|ROLLBACK */
	struct npu_user_submit_stream nustream;
};

//extern shared function
int phytium_npu_session_init(struct phytium_npu_dev *npu, struct phytium_npu_session *session);
struct phytium_npu_session *phytium_npu_session_create(struct device *dev);
int phytium_npu_register_dev(struct device *dev, void *plat_data, void __iomem *reg_base);
int phytium_npu_register_misc(struct phytium_npu_dev *npudev);
int phytium_npu_unregister_misc(struct phytium_npu_dev *npudev);
int phytium_npu_session_release(struct phytium_npu_dev *npu, struct phytium_npu_session *session);
struct npu_mctx_map *phytium_npu_find_mmu_ctx_map(struct phytium_npu_session *sess, int dma_buf_fd);
struct dma_buf *phytium_npu_check_and_get_dma_buf(struct phytium_npu_session *sess, int dma_buf_fd);
struct phytium_npu_dev *phytium_npu_get_npudev(void);
int phytium_npu_mmu_dev_init(struct phytium_npu_dev *npudev, int page_size);
int phytium_npu_create_new_mmu_context(struct phytium_npu_dev *npu,
				       struct phytium_npu_session *session);
int phytium_npu_mmu_map_init(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			     struct npu_memory_map *usr_mmap);
int phytium_npu_import_dmabuf(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			      struct npu_memory_map *usr_mmap);
int phytium_npu_release_mmu_context(struct phytium_npu_dev *npu,
				    struct phytium_npu_session *session);
int phytium_npu_mmu_unmap(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			  struct npu_memory_unmap *usr_unmmap);
void phytium_npu_mmu_output_status(struct phytium_npu_dev *npudev);
int phytium_npu_mmu_config_dev_mmu(struct phytium_npu_session *sess);
int phytium_npu_mmu_dev_flush_tlb(struct phytium_npu_dev *npudev, int ctxid);
int phytium_npu_write_stream(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			     const char *stream_buf, size_t size);
int phytium_npu_delete_stream(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			      struct npu_delete_stream *stream);
int phytium_npu_handle_irq(struct device *dev);
u32 phytium_npu_get_irq_status(struct phytium_npu_dev *npudev);
u32 phytium_npu_get_irq_enable_status(struct phytium_npu_dev *npudev);
void phytium_npu_clear_msi_irq(struct phytium_npu_dev *npudev);
void phytium_npu_clear_irq_status(struct phytium_npu_dev *npudev, u32 event);
void phytium_npu_config_preload(struct phytium_npu_dev *npudev);
void phytium_npu_config_start_inference(struct phytium_npu_dev *npudev,
					struct phytium_npu_session *sess,
						struct phytium_npu_stream *nstream);
int phytium_npu_handle_thread_irq(struct device *dev);
int phytium_npu_hw_reset_self(struct phytium_npu_dev *npudev);
void phytium_npu_config_hl_wdt(struct phytium_npu_dev *npudev);
void phytium_npu_config_event(struct phytium_npu_dev *npudev, u32 events, int is_enable);
u32 phytium_npu_get_axi_err_status(struct phytium_npu_dev *npudev);
void phytium_npu_output_mem_wdt_err(struct phytium_npu_dev *npudev);
void do_work(struct work_struct *work);
void phytium_npu_schedule_stream_queues(struct phytium_npu_dev *npu, bool asynchronous);
int phytium_npu_check_stream_buf_is_ready(struct phytium_npu_stream *nstream);
void phytium_npu_set_stream_buf_status_with_fd(struct phytium_npu_session *sess,
					       int fd, int status);
void phytium_npu_repeat_stream(struct phytium_npu_session *sess, int stream_id, int is_repeat);

void phytium_npu_change_schedule_session(struct phytium_npu_session *sess,
					 struct phytium_npu_dev *npu,
								int is_change);
int phytium_npu_config_dma_address(struct phytium_npu_session *sess,
				   struct phytium_npu_stream *nstream);
void phytium_npu_config_clock(struct phytium_npu_dev *sess, int is_enable);
int phytium_npu_submit_stream(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			      struct phytium_npu_stream *nstream);
int phytium_npu_prepare_hw_4_queued_stream(struct phytium_npu_dev *npu,
					   struct phytium_npu_session *sess,
					struct phytium_npu_stream *nstream);
int phytium_npu_try_excute_queued_stream(struct phytium_npu_dev *npu);
int phytium_npu_rollback_stream(struct phytium_npu_dev *npu);
int phytium_npu_update_activated_stream_4_err_mmu(struct phytium_npu_dev *npu);
int phytium_npu_session_release_all(struct phytium_npu_dev *npudev);
int phytium_npu_unregister_dev(struct device *dev);
int phytiun_npu_check_debug_fs_cfg(struct phytium_npu_session *sess);
void phytium_npu_debug_set_hw_register(struct phytium_npu_dev *npu,
				       struct phytium_npu_session *sess);
void phytium_npu_debugfs_init(struct phytium_npu_dev *npu);
void phytium_npu_debugfs_session_init(struct phytium_npu_dev *npu,
				      struct phytium_npu_session *sess);
void phytium_npu_debug_show_register(struct phytium_npu_dev *npu, struct phytium_npu_session *sess);
void phytium_npu_debugfs_session_remove(struct phytium_npu_session *sess);
void phytium_npu_debugfs_remove(struct phytium_npu_dev *npu);
void phytium_npu_try_resume_work(struct phytium_npu_dev *npudev);
int phytium_npu_schedule_suspend(struct phytium_npu_dev *npudev, u32 delay_ms);
int phytium_npu_common_resume(struct device *dev);
int phytium_npu_common_suspend(struct device *dev);
void phytium_npu_get_time_span(struct timespec64 *start, struct timespec64 *end,
			       struct timespec64 *span);
#endif
