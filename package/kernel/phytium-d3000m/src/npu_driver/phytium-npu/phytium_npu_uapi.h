/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#ifndef _PHYTIUM_NPU_USER_API_H_
#define _PHYTIUM_NPU_USER_API_H_

#include <linux/version.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/dma-fence.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>

/* Cache attributes mask */
#define MEM_ATTR_CACHE_MASK 0xf
#define NPU_RSP_OK			0
enum {
	/* System errors */
	NPU_RSP_ERROR_HW_SYS_AXI_ERROR_SHF = 0,
	NPU_RSP_ERROR_HW_SYS_MMU_PAGE_FAULT_SHF,
	NPU_RSP_ERROR_HW_SYS_SYS_MEM_WDT_SHF,
	NPU_RSP_ERROR_HW_SYS_AXI_MEMORY_PARITY_ERROR_SHF,
	NPU_RSP_ERROR_HW_SYS_MMU_PARITY_ERROR_SHF,
	NPU_RSP_ERROR_HW_SYS_RAM_CORRECTION_SHF,
	NPU_RSP_ERROR_HW_SYS_RAM_DETECTION_SHF,
	NPU_RSP_ERROR_HW_SYS_LSYNC_INV_REQ_SHF,
	NPU_RSP_ERROR_HW_SYS_LOGIC_ERROR_SHF,
	NPU_RSP_ERROR_SW_SYS_EVNT_PARITY_ERROR_SHF,
	NPU_RSP_ERROR_SW_WDT_EXPIRED_SHF,
	/* CNN core status errors. */
	NPU_RSP_ERROR_HW_CORE_LOGIC_ERROR_SHF,
	NPU_RSP_ERROR_HW_RAM_CORRECTION_SHF,
	NPU_RSP_ERROR_HW_RAM_DETECTION_SHF,
	NPU_RSP_ERROR_HW_CORE_SYNC_ERROR_SHF,
	NPU_RSP_ERROR_HW_CORE_WDT_SHF,
	NPU_RSP_ERROR_HW_CORE_MEM_WDT_SHF,
	NPU_RSP_ERROR_HW_CORE_CNN_ERROR_SHF,
	/* Interconnect status errors. */
	NPU_RSP_ERROR_HW_LOCKSTEP_ERROR_SHF,
	NPU_RSP_ERROR_HW_IC_LOGIC_ERROR_SHF,
	NPU_RSP_ERROR_HW_SOCIF_READ_MISMATCH_SHF,
	NPU_RSP_ERROR_HW_SOCIF_READ_UNRESPONSIVE_SHF,
	NPU_RSP_ERROR_SW_IC_PARITY_ERROR_SHF,
	/* Workload submit errors. */
	NPU_RSP_ERROR_SW_SKIP_CMD_SHF,
	NPU_RSP_ERROR_SW_KICK_BIT_READ_BACK_FAILURE_SHF,
	NPU_RSP_ERROR_SW_HW_BUSY_SHF,
	NPU_RSP_ERROR_SW_INVALID_CMD_INFO_SHF,
	NPU_RSP_ERROR_SW_INVALID_CMD_TYPE_SHF,
	NPU_RSP_ERROR_SW_MMU_SETUP_FAILURE_SHF,

	/* Special flag to mark critical errors */
	NPU_RSP_ERROR_CRITICAL_SHF
};

#define NPU_RSP_ERROR(err) ((1ull << (NPU_RSP_ERROR_##err##_SHF) | \
							(1ull << NPU_RSP_ERROR_CRITICAL_SHF)))
#define NPU_RSP_WARNING(err) (1ull << (NPU_RSP_ERROR_##err##_SHF))

#define NPU_RSP_SET_ERROR(err_flags) ((err_flags) |= (1ull << NPU_RSP_ERROR_CRITICAL_SHF))
#define NPU_RSP_IS_CRITICAL(err) ((((err) >> NPU_RSP_ERROR_CRITICAL_SHF) & 0x1) ? true : false)

/* memory attributes */
enum mem_attr {
	MEM_ATTR_CACHED        = 0x00000001,
	MEM_ATTR_UNCACHED      = 0x00000002,
	MEM_ATTR_WRITECOMBINE  = 0x00000004,

	/* Special */
	MEM_ATTR_SECURE        = 0x00000010,
	MEM_ATTR_NOMAP         = 0x00000020,
	MEM_ATTR_NOSYNC        = 0x00000040,

	/* Internal */
	MEM_ATTR_MMU           = 0x10000000,
	MEM_ATTR_OCM           = 0x20000000,
};

enum debug_mode {
	DEBUG_MODE_NONE = 0,
	DEBUG_MODE_REGISTER,
	DEBUG_MODE_MEMORY_FILE,
};

enum debug_type {
	DEBUG_TYPE_NONE = 0,
	DEBUG_TYPE_REG_PERF = 0x1,
	DEBUG_TYPE_REG_BAND = 0x2,
	DEBUG_TYPE_BAND_STREAM = 0x10,
	DEBUG_TYPE_BAND_LAYER = 0x20,
	DEBUG_TYPE_BAND_PASS = 0x30,
	DEBUG_TYPE_PERF_STREAM = 0x100,
	DEBUG_TYPE_PERF_LAYER = 0x200,
	DEBUG_TYPE_PERF_PASS = 0x300,
	DEBUG_TYPE_CRC_STREAM = 0x1000,
	DEBUG_TYPE_CRC_LAYER = 0x2000,
	DEBUG_TYPE_CRC_PASS = 0x3000,
};

#define NPU_OCM_MAX_NUM_PAGES 128
#define NPU_MAX_DMA_ADDRS 16

// represents OCM types,
#define NPU_LOCAL_OCM  0  /* Local OCM */
#define NPU_SHARED_OCM 1  /* Shared OCM */
#define NPU_OCM_TYPE_MAX 2

struct npu_info {
	u64	pid;
	u64	version;
	u32	is_have_mmu;
	u32	mmu_page_size;
	u32	mefficiency;
	bool	use_debug;
	u8	core_num;
	u32	l1_size;
	u32	l3_size;
	u32	l3_percore_size;
	u32	clock_freq;
} __aligned(8);

/* compute stream sent to device */
enum npu_compute_type {
	NPU_INVALID		= 0x000,
	NPU_STREAM_SUBMIT	= 0x201,
	NPU_STREAM_DEBUG
};

/* optional flags for commands */
#define NPU_COMPUTEFLAG_NOTIFY		0x0001 /* send response when cmd complete */
#define NPU_CHECK_CRC			0x0002 /* check the combined CRCs */
#define NPU_EXEC_TIME_SET		0x0004 /* execution time is valid */

struct npu_excute_stream {
	u16 sflags;	/* NPU_STREAM FLAG_xxx */
	u16 stype;		/* enum stream_type */
	u32 sid;		/* arbitrary id for stream */
	u8 sp1;		/* priority */
	u8 sp2;		/* padding  */
	u8 all;		/* total number of buffers */
	u8 in;			/* number of input buffers */
	u32 t;
	u32 fd[NPU_MAX_DMA_ADDRS];	/* dma-buf fd */
};

struct npu_user_submit_stream {
	struct npu_excute_stream estream;
	s32 stream_off;
	s32 stream_fd;		/* bufid of stream buffer */
	s32 stream_size;
	u32 bufoffsets[NPU_MAX_DMA_ADDRS];	/* offsets into inbufs and outbufs buffers */
	u32 bufsizes[NPU_MAX_DMA_ADDRS];	/* sizes of the inbufs and outbufs buffers */
	u8  idx[NPU_MAX_DMA_ADDRS];	/* register to be used for inbufs and outbufs */
	u32 cycles;	/* estimated number of cycles for this command */
	u32 num;	/* number of subsegments in subseg_info array */
} __aligned(8);

/*
 * response from kernel module to user.
 */
struct npu_user_rsp {
	u64 rsp_err_flags;
	u32 sid;	/* arbitrary id to identify stream */
	u32 err_no;	/* 0 if successful, else -ve */
	u32 session_id;
};

#define MAX_NPU_USER_RSP_SIZE (sizeof(struct npu_user_rsp))
/*
 * response returned after CNN_SUBMIT.
 */
struct npu_user_cnn_submit_rsp {
	struct npu_user_rsp msg;
	u64 last_proc_us;	/* processing time in us, measured with system clock */
	u32 mem_usage;	/* device memory used */
	u32 hw_cycles;	/* hardware cycles used */
} __aligned(8);

#define MAX_NPU_UCNN_RSP_SIZE (sizeof(struct npu_user_cnn_submit_rsp))

struct npu_free_buffer {
	s32 fd;		/* [IN] fd of dma buffer to free */
};

enum npu_map_type {
	NPU_MAP_TYPE_NONE = 0x0,
	NPU_MAP_TYPE_BUF,
	NPU_MAP_TYPE_INFERENCE,
	NPU_MAP_TYPE_RO = 0x4,
};

struct npu_map2cache {
	u64 vaddr;		/* [IN] Device virtual address of a mapping */
	u32 fd;		/* [IN] fd of dma buffer to map to VHA */
	u32 page_size;	/* [IN] Page size */
	u32 num_pages;	/* [IN] The number of pages to be mapped */
	u32 page_idxs[128];//驱动定义最大page128--516K
							/* [IN] Indexes of pages to be mapped */
} __aligned(8);

struct npu_memory_map {
	u32 map_type;		/* [IN] Mapping flags, see NPU_map_type */
	s32 fd;				/* [IN] fd of dma buffer  if necessary */
	u64 vaddr;			/* [IN] Device virtual address to map     */
} __aligned(8);

struct npu_memory_unmap {
	s32 fd;			/* [IN] fd of dma buffer  if necessary*/
};

enum npu_buf_status {
	NPU_BUF_UNUPDATE,
	NPU_BUF_UPDATED_BY_SW,
	NPU_BUF_UPDATED_BY_HW
};

#define NPU_SYNC_NONE (-1)

struct npu_set_buffer {
	s32	fd;
	u32	set_state;		/* enum npu_buf_status */
	int	input_sync_fd;	/* input sync to attach */
	bool	is_output_sync;	/* output sync signal */
} __aligned(8);

enum npu_sync_op {
	NPU_SYNC_OP_CREATE_OUT, /* create output sync_fd */
	NPU_SYNC_OP_MERGE_IN,   /* merge input sync_fds */
	NPU_SYNC_OP_RELEASE     /* release syncs */
};

/* parameters to manage sync_fds */
#define NPU_SYNC_MAX_DMA_IDS     (NPU_MAX_DMA_ADDRS)
#define NPU_SYNC_MAX_IN_SYNC_FDS (NPU_MAX_DMA_ADDRS)

struct npu_sync_buffer {
	enum npu_sync_op op;
	u32 num;					/* [IN] number of output buffers */
	int fd[NPU_SYNC_MAX_DMA_IDS];	/* [IN] list of output buffer ids */
	int syncfd;	/* [OUT] output sync_fd/sync_fd for merged input sync_fds */
} __aligned(8);

struct npu_delete_stream {
	u32 stream_id;			/* [IN] masked ID of commands to be deleted */
	u32 stream_id_mask;	/* [IN] mask for command IDs to be deleted */
	bool	is_res;			/* [IN] if true, respond to this cancel request */
} __aligned(8);

struct npu_repeat_stream {
	u32 stream_id;			/* [IN] masked ID of commands to be deleted */
	u32 stream_id_mask;	/* [IN] mask for command IDs to be deleted */
	bool	is_repeat;			/* [IN] if true, repeat this stream*/
} __aligned(8);

struct npu_debug_perf {
	u32 debug_mode; /* register|mem file */
	u32 debug_type; /* perf|band|crc */
	u32 debug_fd; /* debug perf or band memory fd */
	u32 debug_crc_fd; /* debug crc memory fd */
};

#define NPU_INFO				_IOR('p', 0xB0, struct npu_info)
#define NPU_MAP2CACHE				_IOW('p', 0xB1, struct npu_map2cache)
#define NPU_MEMORY_IMPORT			_IOW('p', 0xB2, struct npu_memory_map)
#define NPU_MEMORY_UNMAP			_IOW('p', 0xB3, struct npu_memory_unmap)
#define NPU_SET_BUF				_IOW('p', 0xB4, struct npu_set_buffer)
#define NPU_SYNC_BUF				_IOWR('p', 0xB5, struct npu_sync_buffer)
#define NPU_DELET_STREAM			_IOW('p', 0xB6, struct npu_delete_stream)
#define NPU_REPEAT_STREAM			_IOW('p', 0xB7, struct npu_repeat_stream)
#define NPU_DEBUG_PERF				_IOW('p', 0xB8, struct npu_debug_perf)

/*
 * npu scope context
 */
struct npu_trace_ctx {
	u32 model_id;		/* model id */
	u32 frame_id;		/* inference id */
	u32 dev_id;		/* device id */
	u32 osid;			/* device id */
	u32 pid;			/* process id */
	u32 tid;			/* thread id */
};

/* Event information, available from NPU_info */
struct npu_timing_data {
	u32 evt_type;		/* event type */
	u32 seqno;			/* continually increments */
	u32 dev_id;		/* device id */
	u32 timestamp_lo;	/* in microseconds */
	u32 timestamp_hi;
	u32 type;			/* either SUBMIT or COMPLETE or ERROR */
	u32 cycles;		/* HW cycle count */
	u32 pid;			/* process id */
};

enum npu_scope_evt_type {
	NPU_EVENT_TIMING,
	NPU_EVENT_NUM
};

enum npu_timing_data_type {
	NPU_EVENT_TYPE_ENQUEUE,
	NPU_EVENT_TYPE_SUBMIT,
	NPU_EVENT_TYPE_COMPLETE,
	NPU_EVENT_TYPE_ERROR,
	NPU_EVENT_TYPE_NUM
};

#endif
