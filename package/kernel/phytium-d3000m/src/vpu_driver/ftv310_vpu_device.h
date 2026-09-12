/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 decoder hardware driver.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef FTV310_VPU_SLICE_H
#define FTV310_VPU_SLICE_H

#include "ftv310_vpu.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* #define MAX(a, b)                 \
 * ({ typeof(a) a_ = (a);            \
 * typeof(b) b_ = (b);               \
 * (a_ > b_) ? a_ : b_; })
 */

#define MULTI_SLICE_LIMIT 8 //MAX slice supported now
#define MAX_CACHE_PERCORE 2 //MAX cache number connected with a core

#define FTV310_VPU_INVALID_ID -1
#define MAX_SLICE_NUM 32

#define ASIC_SWREG_AMOUNT                       543 //from encswhwregister.h

/*supported core type*/
typedef enum {
	FTV310_VPU_CORE_UNKNOWN = -1,
	FTV310_VPU_CORE_SLICE = 1 << 0,
	FTV310_VPU_CORE_DEC = 1 << 1,
	FTV310_VPU_CORE_ENC = 1 << 2,
	FTV310_VPU_CORE_IM = 1 << 3,
	FTV310_VPU_CORE_DECJPG = 1 << 4,
	FTV310_VPU_CORE_ENCJPG = 1 << 5,
	FTV310_VPU_CORE_CACHE = 1 << 6,
	FTV310_VPU_CORE_DEC400 = 1 << 7,
	FTV310_VPU_CORE_MMU = 1 << 8,
	FTV310_VPU_CORE_VCMD = 1 << 9,
	FTV310_VPU_CORE_AXIFE = 1 << 10,
} slice_coretype;

struct ftv310_vpu_slice {
	u32 vcmd_en;
	void *slice_list;
};

struct cache_core_config {
	cache_client_type client;
	unsigned long long base_addr;
	u32 iosize;
	int irq;
	driver_cache_dir dir;
	u32 sliceidx;
	unsigned long long parentaddr;
};

struct cache_dev_t {
	struct cache_core_config
		core_cfg; //config of each core,such as base addr, irq,etc
	unsigned long hw_id; //hw id to indicate project
	u32 core_id; //core id for driver and sw internal use
	u32 is_valid; //indicate this core is ftv310_vpu's core or not
	u32 is_reserved; //indicate this core is occupied by user or not
	struct file *cacheowner; //indicate which process is occupying the core
	u32 irq_received; //indicate this core receives irq
	u32 irq_status;
	char *buffer;
	unsigned int buffsize;
	u8 *hwregs;
	u32 *dec_regs;
	unsigned long long com_base_addr; //common base addr of each L2
	int irqlist[4];

	slice_coretype parenttype;
	u32 parentid; //parent codec core's core_id
	/* either struct ftv310_vpu_enc_t or struct ftv310_vpu_dec_t, or slice itself */
	void *parentcore;
	void *parentslice;

	struct cache_dev_t *next;
};

struct dec400_core_cfg {
	unsigned long long dec400corebase;
	unsigned int iosize;
	u32 sliceidx;
	unsigned long long parentaddr;
};

struct dec400_t {
	struct dec400_core_cfg core_cfg;

	u32 core_id;
	u8 *hwregs;

	slice_coretype parenttype;
	u32 parentid; //parent codec core's core_id
	/* either struct ftv310_vpu_enc_t or struct ftv310_vpu_dec_t, or slice itself */
	void *parentcore;

	void *parentslice;
	struct dec400_t *next;
};

struct mmu_core_cfg {
	unsigned long long mmucorebase;
	unsigned int iosize;
	u32 sliceidx;
	slice_coretype parenttype;
};


struct mmu_tbl_info {
	void *page_table_mutex;
	unsigned int mtlb_size;
	unsigned long long mtlb_physical;
	void *mtlb_virtual;
	unsigned int mtlb_entries;
	unsigned int stlb_size;
	unsigned long long stlb_physical;
	void *stlb_virtual;
	unsigned int page_table_array_size;
	unsigned long long page_table_array_physical;
	void *page_table_array;
	int enabled;
};

struct MMU;
//core
struct mmu_t {
	u8 *hwregs;
	u32 core_id;
	void *hw_mutex;
	struct device *dev;
	struct mmu_core_cfg core_cfg;
	struct MMU *g_mmu;
	struct mmu_tbl_info pg_tbl;
	int mmu_enable;
	void *parentslice;
	struct mmu_t *next;
};

struct axife_core_cfg {
	unsigned long long axifecorebase;
	unsigned int iosize;
	u32 sliceidx;
	unsigned long long parentaddr;
};

struct axife_t {
	struct axife_core_cfg core_cfg;

	u32 core_id;
	u8 *hwregs;
	u32 *dec_regs;

	slice_coretype parenttype;
	u32 parentid; //parent codec core's core_id
	/* either struct ftv310_vpu_enc_t or struct ftv310_vpu_dec_t, or slice itself */
	void *parentcore;

	void *parentslice;
	struct axife_t *next;
};

typedef struct {
	unsigned long long base_addr;
	u32 iosize;
	int irq;
	//resource_shared indicate core share resources with other cores.
	//If 1, cores can not work at same time.
	u32 resource_shared;
	u32 sliceidx;
} CORE_CONFIG;

struct ftv310_vpu_enc_t {
	CORE_CONFIG core_cfg; //config of each core,such as base addr, irq,etc
	u32 hw_id; //hw id to indicate project
	u32 core_id; //core id for driver and sw internal use
	u32 is_reserved; //indicate this core is occupied by user or not
	int pid; //indicate which process is occupying the core
	u32 irq_received; //indicate this core receives irq
	u32 irq_status;
	char *buffer;
	unsigned int buffsize;
	u8 *hwregs;
	struct fasync_struct *async_queue;
	int irqlist[4];
	SUBSYS_CORE_INFO core_info;

	void *parentslice;
	struct ftv310_vpu_enc_t *next;

#ifdef PHY_CONFIG_PM
	u32 reg_buf[ASIC_SWREG_AMOUNT];
	struct semaphore core_suspend_sem;
	u32 reg_corrupt;
#endif
};

#define FTV310_VPU_G1_DEC_REGS 155 /*G1 total regs*/
#define FTV310_VPU_G2_DEC_REGS 337 /*G2 total regs*/
#define FTV310_VPU_FTV310D_REGS 768 /*FTV310D total regs*/
#define DEC_IO_SIZE_MAX                                                        \
	(MAX(MAX(FTV310_VPU_G2_DEC_REGS, FTV310_VPU_G1_DEC_REGS),                      \
	     FTV310_VPU_FTV310D_REGS) *                                            \
	 4)

struct ftv310_vpu_dec_t {
	u32 cfg;
	int core_id;
	unsigned int iosize;
	u32 cfg_backup;
	/* indicate if main core exist */
	struct ftv310_vpu_dec_t *its_main_core_id;
	/* indicate if aux core exist */
	struct ftv310_vpu_dec_t *its_aux_core_id;
	/*all access to hwregs are through readl/writel
	 * so volatile is removed according to doc "volatile is evil"
	 */
	u8 *hwregs;
	int hw_id;

	unsigned long long multicorebase;
	/* Because one core may contain multi-pipeline,
	 * so multicore base may be changed
	 */
	unsigned long long multicorebase_actual;

	u32 dec_regs[DEC_IO_SIZE_MAX / 4];
	int irqlist[4];

	u32 sliceidx;

	struct file *dec_owner;
	struct file *pp_owner;

	void *parentslice;
	struct ftv310_vpu_dec_t *next;
#ifdef PHY_CONFIG_PM
	int hw_active;
#endif
};

struct ftv310_vpu_vcmd_t {
	unsigned long long base_addr;
	u32 iosize;
	int vcmd_irq;
	int sub_module_type; /*input vce=0,IM=1,vcd=2,jpege=3, jpegd=4*/
	unsigned long long parentaddr;

	u32 sliceidx;
	u32 core_id;
	slice_coretype parenttype;
	u32 parentid; //parent codec core's core_id
	/* either struct ftv310_vpu_enc_t or struct ftv310_vpu_dec_t, or slice itself */
	void *parentcore;
	void *parentslice;

	struct ftv310_vpu_vcmd_t *next;
};

/*
 * current internal slice data structure will look like this:
 *	slicehdr->	slice 0		-> slice 1 -> ...
 *				   |
 *		___________________________________
 *		|			|		   |		  |
 *	dec core 0	enc core 0	dec400 0  cache core 0
 *		|			|		   |		  |
 *	dec core 1	enc core 1	dec400 1  cache core 1
 *		|			|		   |		  |
 *	 .......	 ......		 .....		.....
 *
 * Each core node contains its own info: io region,
 * irq, hwid, direction, etc., depending on its type.
 * Each dec400 core has pointer to a dec or enc core.
 * Each dec/enc core has a pointer to dec400 core.
 * Each cache core has pointer to a dec or enc core.
 * Each dec/enc core has pointers to dec400 core.
 * We do it this way since we don't know which
 * one will be probed first, dec/enc or dec400/cache.
 * And only ID to connect dec/enc to dec400/cache core is their HW address.
 */

struct slice_info {
	struct device *dev; //related dev, for drm usage
	phys_addr_t rsvmem_addr;
	phys_addr_t memsize;
	u32 config;

	int deccore_num;
	int enccore_num;
	int dec400core_num;
	int axifecore_num;
	int cachecore_num;
	int mmucore_num;
	int vcmdcore_num;

	struct ftv310_vpu_dec_t *dechdr;
	struct ftv310_vpu_enc_t *enchdr;
	struct cache_dev_t *cachehdr;
	struct dec400_t *dec400hdr;
	struct axife_t *axifehdr;
	struct mmu_t *mmuhdr;
	struct ftv310_vpu_vcmd_t *vcmdhdr;

	/* orig cache global vars*/
	wait_queue_head_t cache_hw_queue;
	wait_queue_head_t cache_wait_queue;
	/* cache mutex */
	spinlock_t cache_owner_lock;

	/*mmu resource*/
	void *mmu_page_table_mutex;
	int is_share_pgtbl_in_slice; // default equal 1
	struct mmu_tbl_info pg_tbl;

	/*orig enc global vars*/
	struct semaphore enc_core_sem;
	/* encoder hw mutex */
	wait_queue_head_t enc_hw_queue;
	/* enc owner mutex */
	spinlock_t enc_owner_lock;
	wait_queue_head_t enc_wait_queue;

	/*orig dec global vars*/
	int dec_irq;
	int pp_irq;
	/* resource mutex */
	spinlock_t owner_lock;
	wait_queue_head_t dec_wait_queue;
	wait_queue_head_t pp_wait_queue;
	wait_queue_head_t hw_queue;
	struct semaphore dec_core_sem;
	struct semaphore pp_core_sem;

	struct slice_info *next;
};

int findslice_bydev(struct device *dev);
int addslice(struct device *dev, phys_addr_t sliceaddr, phys_addr_t slicesize);
struct slice_info *getslicenode(u32 sliceindex);
struct slice_info *getslicenode_ininit(u32 sliceindex);
int get_slicecorenum(u32 sliceindex, slice_coretype type);
struct ftv310_vpu_dec_t *get_decnodes(u32 sliceindex, u32 nodeidx);
struct mmu_t *get_mmunode(u32 sliceindex, u32 nodeidx);
struct ftv310_vpu_dec_t *getfirst_decnodes(struct slice_info *pslice);
struct ftv310_vpu_enc_t *get_encnodes(u32 sliceindex, u32 nodeidx);
struct cache_dev_t *get_cachenodes(u32 sliceindex, u32 nodeidx);
struct cache_dev_t *get_cachenodebytype(u32 sliceindex, u32 parenttype,
					u32 parentnodeidx);
struct dec400_t *get_dec400nodes(u32 sliceindex, u32 nodeidx);
struct dec400_t *get_dec400nodebytype(u32 sliceindex, u32 parenttype,
				      u32 parentnodeidx);
struct axife_t *get_axifenodes(u32 sliceindex, u32 nodeidx);
struct axife_t *get_axifenodebytype(u32 sliceindex, u32 parenttype,
				    u32 parentnodeidx);
struct mmu_t *get_mmunodebytype(u32 sliceindex, u32 nodetype);
int add_decnode(u32 sliceindex, struct ftv310_vpu_dec_t *deccore);
int add_encnode(u32 sliceindex, struct ftv310_vpu_enc_t *enccore);
int add_dec400node(u32 sliceindex, struct dec400_t *dec400core);
int add_axifenode(u32 sliceindex, struct axife_t *axifecore);
int add_cachenode(u32 sliceindex, struct cache_dev_t *cachecore);
int add_mmunode(u32 sliceindex, struct mmu_t *mmucore);
void remove_node(void *pnode, int type);
int get_slicenumber(void);
struct slice_info *getparentslice(void *node, int type);
int slice_remove(void);
int slice_init(void);
void slice_init_finish(void);
long ftv310_vpu_slice_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int getnodetype(const char *name);

void slice_printdebug(void);

#endif /*#define FTV310_VPU_SLICE_H*/
