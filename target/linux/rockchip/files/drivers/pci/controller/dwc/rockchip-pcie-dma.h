/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 Rockchip Electronics Co., Ltd.
 */
#ifndef __SOC_ROCKCHIP_PCIE_DMA_TRX_H
#define __SOC_ROCKCHIP_PCIE_DMA_TRX_H

#include <linux/debugfs.h>

#define PCIE_DMA_TABLE_NUM		32

#define PCIE_DMA_TRX_TYPE_NUM		3

#define PCIE_DMA_CHN0			0x0
#define PCIE_DMA_CHN1			0x1
#define PCIE_DMA_DEFAULT_CHN		PCIE_DMA_CHN0

#define PCIE_DMA_DATA_SND_TABLE_OFFSET		0x0
#define PCIE_DMA_DATA_RCV_ACK_TABLE_OFFSET	0x8
#define PCIE_DMA_DATA_FREE_ACK_TABLE_OFFSET	0x10
#define PCIE_DMA_DATA_READ_REMOTE_TABLE_OFFSET	0x18

/* DMA linked list register filed */
#define PCIE_DWC_DMA_CB				BIT(0)
#define PCIE_DWC_DMA_TCB			BIT(1)
#define PCIE_DWC_DMA_LLP			BIT(2)
#define PCIE_DWC_DMA_LIE			BIT(3)
#define PCIE_DWC_DMA_RIE			BIT(4)
#define PCIE_DWC_DMA_CCS			BIT(8)
#define PCIE_DWC_DMA_LLE			BIT(9)

#define SET_LL_32(ll, value) \
	writel(value, ll)

#define SET_LL_64(ll, value) \
	writeq(value, ll)

enum dma_dir {
	DMA_FROM_BUS,
	DMA_TO_BUS,
};

enum dma_mode {
	RK_PCIE_DMA_BLOCK,
	RK_PCIE_DMA_LL,
};

/**
 * The Channel Control Register for read and write.
 */
union chan_ctrl_lo {
	struct {
		u32	cb		:1;	// 0
		u32	tcb		:1;	// 1
		u32	llp		:1;	// 2
		u32	lie		:1;	// 3
		u32	rie		:1;	// 4
		u32	cs		:2;	// 5:6
		u32	rsvd1		:1;	// 7
		u32	ccs		:1;	// 8
		u32	llen		:1;	// 9
		u32	b_64s		:1;	// 10
		u32	b_64d		:1;	// 11
		u32	pf		:5;	// 12:16
		u32	rsvd2		:7;	// 17:23
		u32	sn		:1;	// 24
		u32	ro		:1;	// 25
		u32	td		:1;	// 26
		u32	tc		:3;	// 27:29
		u32	at		:2;	// 30:31
	};
	u32 asdword;
};

/**
 * The Channel Control Register high part for read and write.
 */
union chan_ctrl_hi {
	struct {
		u32	vfenb		:1;	// 0
		u32	vfunc		:8;	// 1-8
		u32	rsvd0		:23;	// 9-31
	};
	u32 asdword;
};

/**
 * The Channel Weight Register.
 */
union weight {
	struct {
		u32	weight0		:5;	// 0:4
		u32	weight1		:5;	// 5:9
		u32	weight2		:5;	// 10:14
		u32	weight3		:5;	// 15:19
		u32	rsvd		:12;	// 20:31
	};
	u32 asdword;
};

/**
 * The Doorbell Register for read and write.
 */
union db {
	struct {
		u32	chnl		:3;	// 0
		u32	reserved0	:28;	// 3:30
		u32	stop		:1;	// 31
	};
	u32 asdword;
};

/**
 * The Context Registers for read and write.
 */
struct ctx_regs {
	union chan_ctrl_lo		ctrllo;
	union chan_ctrl_hi		ctrlhi;
	u32				xfersize;
	u32				sarptrlo;
	u32				sarptrhi;
	u32				darptrlo;
	u32				darptrhi;
};

/**
 * The Enable Register for read and write.
 */
union enb {
	struct {
		u32	enb		:1;	// 0
		u32	reserved0	:31;	// 1:31
	};
	u32 asdword;
};

/**
 * The Interrupt Status Register for read and write.
 */
union int_status {
	struct {
		u32	donesta		:8;
		u32	rsvd0		:8;
		u32	abortsta	:8;
		u32	rsvd1		:8;
	};
	u32 asdword;
};

/**
 * The Interrupt Clear Register for read and write.
 */
union int_clear {
	struct {
		u32	doneclr		:8;
		u32	rsvd0		:8;
		u32	abortclr	:8;
		u32	rsvd1		:8;
	};
	u32 asdword;
};

struct dma_table {
	u32				*descs;
	int				chn;
	phys_addr_t			phys_descs;
	u32				dir;
	u32				type;
	struct list_head		tbl_node;
	union enb			enb;
	struct ctx_regs			ctx_reg;
	union weight			weilo;
	union weight			weihi;
	union db			start;
	phys_addr_t			local;
	phys_addr_t			bus;
	size_t				buf_size;
	u32				dma_mode;
};

struct rk_edma_lli {
	u32 control;
	u32 transfer_size;
	union {
		u64 reg;
		struct {
			u32 lsb;
			u32 msb;
		};
	} sar;
	union {
		u64 reg;
		struct {
			u32 lsb;
			u32 msb;
		};
	} dar;
} __packed;

struct rk_edma_llp {
	u32 control;
	u32 reserved;
	union {
		u64 reg;
		struct {
			u32 lsb;
			u32 msb;
		};
	} llp;
} __packed;

struct dma_trx_obj {
	struct device			*dev;
	int				loop_count;
	int				loop_count_threshold;
	void				*local_mem_base;
	phys_addr_t			local_mem_start;
	size_t				local_mem_size;
	phys_addr_t			remote_mem_start;
	void				*region_base;
	phys_addr_t			region_start;
	size_t				region_size;
	int				dma_free;
	unsigned long			local_write_available;
	unsigned long			local_read_available;
	unsigned long			remote_write_available;
	spinlock_t			tbl_list_lock; /* lock dma table */
	struct list_head		tbl_list;
	struct work_struct		dma_trx_work;
	wait_queue_head_t		event_queue;
	struct workqueue_struct		*dma_trx_wq;
	struct dma_table		*table[PCIE_DMA_TABLE_NUM];
	struct dma_table		*cur;
	struct hrtimer			scan_timer;
	int				busno;
	void				*priv;
	struct completion		done;
	int				ref_count;
	struct mutex			count_mutex;
	unsigned long			irq_num;
	struct dentry			*pcie_root;
	struct pcie_misc_dev		*pcie_dev;
	void				(*start_dma_func)(struct dma_trx_obj *obj, struct dma_table *table);
	void				(*config_dma_func)(struct dma_table *table);
	int				(*get_dma_status)(struct dma_trx_obj *obj, u8 chn, enum dma_dir dir);
	int				(*cb)(struct dma_trx_obj *obj, u32 chn, enum dma_dir dir);
	void				(*dma_debug)(struct dma_trx_obj *obj, struct dma_table *table);
	ktime_t				begin;
	ktime_t				end;
	u64				cache_time_total;
	u64				cache_time_avarage;
	u32				buffer_size;
	u32				rd_buf_size;
	u32				wr_buf_size;
	u32				ack_base;
	u32				set_data_check_pos;
	u32				set_local_idx_pos;
	u32				set_buf_size_pos;
	u32				set_chk_sum_pos;
	u32				version;
	int				addr_reverse;
};

static inline struct dma_trx_obj *pcie_dw_dmatest_register(struct device *dev, bool irq_en)
{
	return NULL;
}

#endif
