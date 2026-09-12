// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/page.h>
#include <uapi/linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>
#include <linux/kernel.h>
#include "linux/phytium_npu_dma_buf_heap.h"
#include "phytium_npu.h"
#include "phytium_npu_mmu.h"
#include "phytium_npu_uapi.h"

#define PD_NUM  512
#define PT_NUM  512
#define MMU_DEFAULT_ID 0
struct npu_mmu_config gconfig;//global variable
//#define DEBUG_MMU
#ifdef DEBUG_MMU
#define PRDEBUG(format, ...)	pr_debug(format, ##__VA_ARGS__)
#else
#define PRDEBUG(format, ...)
#endif

static int phytium_npu_release_mmu_source(struct phytium_npu_dev *npudev,
					  struct phytium_npu_session *sess);

int phytium_npu_create_new_mmu_context(struct phytium_npu_dev *npu,
				       struct phytium_npu_session *session)
{
	struct npu_mmu_ctx *ctx;
	struct npu_mmu_catalogue *pc;
	struct phytium_npu_mmu_context *pnmctx;
	size_t i;

	WARN_ON(!session);
	if (!session)
		return -EINVAL;

	ctx = devm_kzalloc(npu->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->mmu_pc = devm_kzalloc(npu->dev, sizeof(*pc), GFP_KERNEL);
	if (!ctx->mmu_pc)
		return -ENOMEM;
	session->share_mctx = ctx;
	for (i = 0; i < ARRAY_SIZE(session->mmu_ctx); i++) {
		pnmctx = &session->mmu_ctx[i];
		ctx->mmu_pc->mctx = session->share_mctx;
		ctx->pnmctx = pnmctx;
		INIT_LIST_HEAD(&ctx->maplist);
		pnmctx->mctx = ctx;
		pnmctx->context_id = MMU_DEFAULT_ID;
		pnmctx->npu_id = 0;
		pnmctx->virt_base = PHYT_MMU_DDR_VIRT_BASE;
		pnmctx->curr_virt_base = PHYT_MMU_DDR_VIRT_BASE;
		pnmctx->dev = npu->dev;
	}
	return 0;
}

int phytium_npu_release_mmu_context(struct phytium_npu_dev *npu, struct phytium_npu_session *sess)
{
	struct npu_mmu_ctx *ctx;
	struct phytium_npu_mmu_context *pnmctx;
	size_t i;

	WARN_ON(!sess);
	if (!sess)
		return -EINVAL;

	phytium_npu_release_mmu_source(npu, sess);//release mmu source first

	ctx = sess->share_mctx;
	if (!ctx) {
		dev_err(npu->dev, "ERR:Mmu context is lost.");
	} else {
		pr_debug("%s,free  mmu ctx:%p\n", __func__, ctx);
		devm_kfree(npu->dev, ctx->mmu_pc);
		devm_kfree(npu->dev, ctx);
		sess->share_mctx = NULL;
	}

	for (i = 0; i < ARRAY_SIZE(sess->mmu_ctx); i++) {
		pnmctx = &sess->mmu_ctx[i];
		pnmctx->mctx = NULL;
		pr_debug("%s[%d]:release point mmu context %lu", __func__, __LINE__, i);
	}
	return 0;
}

static int phytium_npu_mmu_get_dma_buf(struct npu_mctx_map *nmap)
{
	if (!nmap)
		return -EINVAL;

	nmap->dma_buf = dma_buf_get(nmap->dma_buf_fd);
	if (IS_ERR(nmap->dma_buf))
		return PTR_ERR(nmap->dma_buf);
	pr_debug("Get dma_buf(%d) ok.", nmap->dma_buf_fd);

	return 0;
}

#define CONTEXT_ID_SHIFT	4
#define GET_CONTEXT_ID(X, D)	((X)->id % 4 + (D) * CONTEXT_ID_SHIFT)

int phytium_npu_mmu_map_init(struct phytium_npu_dev *npu,
			     struct phytium_npu_session *sess,
							struct npu_memory_map *usr_mmap)
{
	struct npu_mctx_map *ncmap;
	int ctxid;

	ncmap = devm_kzalloc(npu->dev, sizeof(*ncmap), GFP_KERNEL);
	if (!ncmap)
		return -ENOMEM;

	PRDEBUG("alloc ncmap :%p for fd %d", ncmap, usr_mmap->fd);
	ncmap->dma_buf_fd = usr_mmap->fd;
	ncmap->map_type = usr_mmap->map_type;
	ncmap->virt_addr = usr_mmap->vaddr;
	INIT_LIST_HEAD(&ncmap->mctx_map_entry);
	if (usr_mmap->map_type & NPU_MAP_TYPE_INFERENCE)
		ctxid = NPU_MMU_CONTEXT_MODULE_ID;
	else if (usr_mmap->map_type & NPU_MAP_TYPE_BUF)
		ctxid = NPU_MMU_CONTEXT_IO_ID;
	else
		ctxid = NPU_MMU_CONTEXT_IO_ID;

	sess->mmu_ctx[ctxid].map_type = usr_mmap->map_type;

	sess->mmu_ctx[ctxid].context_id = GET_CONTEXT_ID(sess, ctxid);

	ncmap->mctx = sess->mmu_ctx[ctxid].mctx;
	list_add(&ncmap->mctx_map_entry, &sess->mmu_ctx[ctxid].mctx->maplist);
	phytium_npu_mmu_get_dma_buf(ncmap);
	PRDEBUG("%s type ctxid %d mmu hw contex id %u", __func__,
		ctxid, sess->mmu_ctx[ctxid].context_id);
	return 0;
}

static inline int phytium_npu_mmu_map_destroy(struct phytium_npu_dev *npu,
					      struct npu_mctx_map *ncmap)
{
	list_del(&ncmap->mctx_map_entry);
	devm_kfree(npu->dev, ncmap);
	PRDEBUG("free ncmap :%p", ncmap);
	return 0;
}

struct npu_mctx_map *phytium_npu_find_mmu_ctx_map(struct phytium_npu_session *sess, int dma_buf_fd)
{
	struct npu_mctx_map *ncmap = NULL, *tmp = NULL;
	struct phytium_npu_mmu_context *pnmctx;
	size_t i;

	WARN_ON(!sess);
	for (i = 0; i < ARRAY_SIZE(sess->mmu_ctx); i++) {
		pnmctx = &sess->mmu_ctx[i];
		if (!pnmctx->mctx) {
			pr_err("pnmctx->mctx is NULL in mmu_ctx[%lu],%s", i, __func__);
			continue;
		}

		list_for_each_entry_safe(ncmap, tmp, &pnmctx->mctx->maplist, mctx_map_entry) {
			if (ncmap->dma_buf_fd == dma_buf_fd)
				return ncmap;
		}
	}

	return NULL;
}

struct dma_buf *phytium_npu_check_and_get_dma_buf(struct phytium_npu_session *sess, int dma_buf_fd)
{
	struct npu_mctx_map *ncmap = NULL, *tmp = NULL;
	struct phytium_npu_mmu_context *pnmctx;
	size_t i;

	WARN_ON(!sess);
	for (i = 0; i < ARRAY_SIZE(sess->mmu_ctx); i++) {
		pnmctx = &sess->mmu_ctx[i];
		if (!pnmctx->mctx) {
			pr_err("pnmctx->mctx is NULL in mmu_ctx[%ld],%s", i, __func__);
			continue;
		}

		list_for_each_entry_safe(ncmap, tmp, &pnmctx->mctx->maplist, mctx_map_entry) {
			if (ncmap->dma_buf_fd == dma_buf_fd)
				goto find_ctx_map;
		}
		ncmap = NULL;
	}

find_ctx_map:
	if (ncmap)
		return ncmap->dma_buf;
	return NULL;
}

static int phytium_npu_get_mmu_base_addr(struct phytium_npu_session *sess)
{
	struct phytium_npu_mmu_context *nmctx = &sess->mmu_ctx[NPU_MMU_CONTEXT_MODULE_ID];

	if (nmctx->virt_base == nmctx->curr_virt_base)
		return nmctx->virt_base;

	return nmctx->curr_virt_base;
}

static u32 phytium_npu_get_mmu_pc_entry(u64 virt)
{
	return (virt & PHYT_MMU_PC_MASK) >> PHYT_MMU_PC_SHIFT;
}

static u32 phytium_npu_get_mmu_pd_entry(u64 virt)
{
	return (virt & PHYT_MMU_PD_MASK) >> PHYT_MMU_PD_SHIFT;
}

/* TODO :support other pagesize */
static u32 phytium_npu_get_mmu_entry(u64 virt)
{
	return (virt & PHYT_MMU_PT_MASK) >> PHYT_MMU_PT_SHIFT;
}

static int phytium_npu_get_pc_entry_valid(struct npu_mmu_catalogue *mmu_pc, u64 virt)
{
	u32 *ptr = (u32 *)mmu_pc->desc.cpu_addr;

	return ptr[phytium_npu_get_mmu_pc_entry(virt)] & PHYT_MMU_ENTRY_VALID;
}

static u64
phytium_npu_get_pd_phy_addr_in_pc_entry(struct npu_mmu_catalogue *mmu_pc, u64 virt)
{
	u32 *ptr = (u32 *)mmu_pc->desc.cpu_addr;
	u64 pd_cpu_addr;

	pd_cpu_addr = ptr[phytium_npu_get_mmu_pc_entry(virt)];
	pd_cpu_addr = (pd_cpu_addr & PHYT_MMU_PC_ENTRY_MASK) << PC_SHIFT;

	return pd_cpu_addr;
}

static int phytium_npu_prepare_mmu_source(struct phytium_npu_mmu_context *nmctx,
					  struct npu_mmu_catalogue *mmu_pc,
					   u64 virt)
{
	struct npu_mmu_pd *pd;
	struct npu_mmu_pt *pt;
	u32 entry, pd_entry;
	u64 *addr;
	int ret = 0;
	static int is_new_pd_entry;
	dma_addr_t dma_addr;

	struct phytium_npu_dev *npu = phytium_npu_get_npudev();

	if (!mmu_pc->desc.phys) {
		addr = dma_alloc_coherent(npu->dev, PHYT_MMU_ALLOC_PAGE_SIZE,
					  &dma_addr, GFP_KERNEL);
		if (!addr)
			return -ENOMEM;

		nmctx->pc_base_phys_addr = dma_addr >> PHYT_MMU_PC_ADDR_SHIFT;
		PRDEBUG("create page catalogue phys address %#llx, Write context:%#x",
			dma_addr, nmctx->pc_base_phys_addr);
		/* create page directory , page catalog store address info himself*/
		mmu_pc->desc.cpu_addr = (u64)addr;
		mmu_pc->desc.phys = dma_addr;
		mmu_pc->desc.npu_addr_base = virt;
	}

	if (!mmu_pc->pd) {
		/* page directory is NULL */
		mmu_pc->pd = kcalloc(PD_NUM, sizeof(struct npu_mmu_pd *), GFP_KERNEL);
		if (!mmu_pc->pd) {
			ret = -ENOMEM;
			goto no_pd;
		}
		PRDEBUG("create page directory virt address %p", mmu_pc->pd);
	}

	pd_entry = phytium_npu_get_mmu_pd_entry(virt);
	if (!mmu_pc->pd[pd_entry]) {
		mmu_pc->pd[pd_entry] = kzalloc(sizeof(*mmu_pc->pd[pd_entry]), GFP_KERNEL);
		if (!mmu_pc->pd[pd_entry]) {
			ret = -ENOMEM;
			goto no_pd_entry;
		}
		is_new_pd_entry = 1;
		PRDEBUG("create page directory entry %d virt address %p",
			pd_entry, mmu_pc->pd[pd_entry]);
	}

	pd = mmu_pc->pd[pd_entry];

	if (!pd->desc.phys) {
		if (!phytium_npu_get_pc_entry_valid(mmu_pc, virt)) {
			pd->desc.cpu_addr = (u64)dma_alloc_coherent(npu->dev,
							    PHYT_MMU_ALLOC_PAGE_SIZE,
							    &dma_addr, GFP_KERNEL);
			pd->desc.phys = dma_addr;
			mmu_pc->pd_ext.pd_desc[mmu_pc->pd_ext.valid_pd] =
				kzalloc(sizeof(*mmu_pc->pd_ext.pd_desc[mmu_pc->pd_ext.valid_pd]),
					GFP_KERNEL);
			memcpy(mmu_pc->pd_ext.pd_desc[mmu_pc->pd_ext.valid_pd],
			       &pd->desc, sizeof(pd->desc));
			mmu_pc->pd_ext.valid_pd++;
		} else {
			pd->desc.phys = phytium_npu_get_pd_phy_addr_in_pc_entry(mmu_pc, virt);
			pd->desc.cpu_addr = (u64)phys_to_virt(pd->desc.phys);
		}

		if (!pd->desc.cpu_addr) {
			pr_err("No mem for page directory.");
			ret = -ENOMEM;
			goto no_pd_store_page;
		}
		pd->desc.npu_addr_base = virt;
		PRDEBUG("create page directory entry desc virt address %llx", pd->desc.cpu_addr);
	}

	if (!pd->pt) {
		/* page table is NULL */
		pd->pt = kzalloc(sizeof(*pd->pt), GFP_KERNEL);
		PRDEBUG("create page table address %p", pd->pt);
		if (!pd->pt) {
			ret = -ENOMEM;
			goto no_pt;
		}
	}

	entry = phytium_npu_get_mmu_entry(virt);
	pt = pd->pt;

	if (!pt->desc.phys) {
		if (is_new_pd_entry) {
			pt->desc.cpu_addr = (u64)dma_alloc_coherent(npu->dev,
							    PHYT_MMU_ALLOC_PAGE_SIZE,
							    &dma_addr, GFP_KERNEL);
			is_new_pd_entry = 0;
			pd->pt_page_addr[pd_entry] = pt->desc.cpu_addr;
			PRDEBUG("create page table entry desc virt address %#llx",
				pt->desc.cpu_addr);
		} else {
			pt->desc.cpu_addr = pd->pt_page_addr[pd_entry];
			PRDEBUG("reuse page table entry desc virt address %#llx, pd->page:%#llx",
				pt->desc.cpu_addr, pd->desc.cpu_addr);
		}

		if (!pt->desc.cpu_addr) {
			pr_err("No mem for page table.");
			ret = -ENOMEM;
			goto no_pt_entry;
		}
		pt->desc.phys = virt_to_phys((u64 *)pt->desc.cpu_addr);
		pt->desc.npu_addr_base = virt;
	}
	return 0;

no_pt_entry:
	kfree(pd->pt);
no_pt:
	kfree((u64 *)pd->desc.cpu_addr);
no_pd_store_page:
	kfree(mmu_pc->pd[pd_entry]);
no_pd_entry:
	kfree(mmu_pc->pd);
no_pd:
	kfree((u64 *)mmu_pc->desc.cpu_addr);
	mmu_pc->desc.cpu_addr = 0;
	mmu_pc->desc.phys = 0;
	return ret;
}

static int phytium_npu_write_mmu_pt(struct npu_mmu_pt *mmu_pt, int entry,
				    u64 phys, int flags)
{
	u64 *ptr = (u64 *)mmu_pt->desc.cpu_addr;

	ptr[entry] = phys | flags;
	PRDEBUG("pt entry %d value %llx", entry, ptr[entry]);
	return 0;
}

static int phytium_npu_write_mmu_pd(struct npu_mmu_pd *mmu_pd, int entry,
				    struct npu_mmu_pt *mmu_pt, int flags)
{
	u64 *ptr = (u64 *)mmu_pd->desc.cpu_addr;

	ptr[entry] = mmu_pt->desc.phys | flags;
	PRDEBUG("pd entry %d value %llx", entry, ptr[entry]);
	return 0;
}

static int phytium_npu_write_mmu_pc(struct npu_mmu_catalogue *mmu_pc, int entry,
				    struct npu_mmu_pd *mmu_pd, int flags)
{
	u32 *ptr = (u32 *)mmu_pc->desc.cpu_addr;

	ptr[entry] = (mmu_pd->desc.phys >> PC_SHIFT) | flags;
	PRDEBUG("pc entry %d value %llx", entry, ptr[entry]);
	return 0;
}

static void phytium_npu_debug_mmu_tlb_info(struct phytium_npu_mmu_context *nmctx, u64 virt)
{
	struct npu_mmu_catalogue *mmu_pc = nmctx->mctx->mmu_pc;
	u32 *pc_ptr;
	u64 *ptr2, *ptr3;

	pc_ptr = (u32 *)mmu_pc->desc.cpu_addr;
	/* debug info */
	PRDEBUG("------mmu pc info-------");
	PRDEBUG("pc_base_phys_addr:%#x, virt_base:%#llx", nmctx->pc_base_phys_addr,
		nmctx->virt_base);
	PRDEBUG("map to virt addr %#llx", virt);
	PRDEBUG("pc phys:%#llx, cpu_addr:%#llx, nvaddr base:%#llx, store pd phy addr:%#x",
		mmu_pc->desc.phys, mmu_pc->desc.cpu_addr, mmu_pc->desc.npu_addr_base,
		pc_ptr[phytium_npu_get_mmu_pc_entry(virt)]);

	struct npu_mmu_pd *mmu_pd = mmu_pc->pd[phytium_npu_get_mmu_pd_entry(virt)];

	ptr2 = (u64 *)mmu_pd->desc.cpu_addr;
	PRDEBUG("pd phys:%#llx, cpu_addr:%#llx, nvaddr base:%#llx, store pt phy addr:%#llx",
		mmu_pd->desc.phys, mmu_pd->desc.cpu_addr, mmu_pd->desc.npu_addr_base,
		ptr2[phytium_npu_get_mmu_pd_entry(virt)]);
	struct npu_mmu_pt *mmu_pt = mmu_pd->pt;

	ptr3 = (u64 *)mmu_pt->desc.cpu_addr;
	PRDEBUG("pt phys:%#llx, cpu_addr:%#llx, nvaddr base:%#llx,"
		mmu_pt->desc.phys, mmu_pt->desc.cpu_addr, mmu_pt->desc.npu_addr_base);
	PRDEBUG("store user  addr:%#llx, entry:%d",
		ptr3[phytium_npu_get_mmu_entry(virt)], phytium_npu_get_mmu_entry(virt));
}

static int phytium_npu_mmu_unmap_addr(struct phytium_npu_mmu_context *pnmctx,
				      struct npu_mctx_map *ncmap, u64 virt,
					int pagenum)
{
	struct npu_mmu_pd *pd;
	struct npu_mmu_pt *pt;
	int entry, pd_entry, pt_entry;
	u64 unmap_virt;
	struct npu_mmu_ctx *nmctx = pnmctx->mctx;
	struct npu_mmu_catalogue *mmu_pc;
	int i;

	if (!nmctx) {
		pr_err("mpu mmu ctx is NULL point, pnmctx:%p,ncmap:%p", pnmctx, ncmap);
		return -EINVAL;
	}
	mmu_pc = nmctx->mmu_pc;
	if (!nmctx) {
		pr_err("mpu mmu catalogue is NULL point");
		return -EINVAL;
	}

	unmap_virt = pagenum * PHYT_MMU_ALLOC_PAGE_SIZE + virt;

	entry = phytium_npu_get_mmu_pc_entry(unmap_virt);
	pd_entry = phytium_npu_get_mmu_pd_entry(unmap_virt);
	pt_entry = phytium_npu_get_mmu_entry(unmap_virt);
	PRDEBUG("pc entry:%d, pd entry:%d pt entry %d", entry, pd_entry, pt_entry);

	if (!mmu_pc->ref_count) {
		pr_info("mmu page catalogue is NULL!");
		return -11;
	}

	mmu_pc->ref_count ? mmu_pc->ref_count-- : mmu_pc->ref_count;

	pd = mmu_pc->pd[pd_entry];
	if (pd)	{
		pd->ref_count ? pd->ref_count-- : pd->ref_count;
		PRDEBUG("pd valid:%lld", pd->ref_count);
		pt = pd->pt;
		if (pt)	{
			if (pt->valid == 1)	{
				dma_free_coherent(pnmctx->dev, PHYT_MMU_ALLOC_PAGE_SIZE,
						  (u64 *)pt->desc.cpu_addr, pt->desc.phys);

				pt->desc.cpu_addr = 0;
				pt->desc.phys = 0;
				PRDEBUG("**free only pt cpu address :%p", pt);
				pt->valid ? pt->valid-- : pt->valid;
				pt = NULL;
			} else if (pt->valid) {
				phytium_npu_write_mmu_pt(pt, pt_entry, 0, PHYT_MMU_ENTRY_INVALID);
				pt->valid ? pt->valid-- : pt->valid;
				PRDEBUG("pt valid:%d", pt->valid);
			}
		}

		if (!pd->ref_count) {
			pd->desc.cpu_addr = 0;
			pd->desc.phys = 0;
			kfree(pd->pt); //free page table point
			pd->pt = NULL;
			kfree(pd);
			mmu_pc->pd[pd_entry] = NULL;
		}
	}

	if (!mmu_pc->ref_count) {
		for (i = 0; i < mmu_pc->pd_ext.valid_pd; i++) {
			dma_free_coherent(pnmctx->dev, PHYT_MMU_ALLOC_PAGE_SIZE,
					  (u64 *)mmu_pc->pd_ext.pd_desc[i]->cpu_addr,
					   mmu_pc->pd_ext.pd_desc[i]->phys);
			kfree(mmu_pc->pd_ext.pd_desc[i]);
			mmu_pc->pd_ext.pd_desc[i] = NULL;
		}

		dma_free_coherent(pnmctx->dev, PHYT_MMU_ALLOC_PAGE_SIZE,
				  (u64 *)mmu_pc->desc.cpu_addr, mmu_pc->desc.phys);

		mmu_pc->desc.cpu_addr = 0;
		mmu_pc->desc.phys = 0;
		PRDEBUG("**free only pd :%p", mmu_pc->pd);

		kfree(mmu_pc->pd);
		mmu_pc->pd = NULL;
	}

	return 0;
}

static int phytium_npu_mmu_map_addr(struct phytium_npu_mmu_context *pnmctx,
				    struct npu_mctx_map *ncmap,
					u64 virt, u64 phys,
					int pagenum, int ptflags)
{
	struct npu_mmu_catalogue *mmu_pc = pnmctx->mctx->mmu_pc;
	struct npu_mmu_pd *pd;
	struct npu_mmu_pt *pt;
	int ret, entry, pd_entry;
	u64 map_virt;

	map_virt = pagenum * PHYT_MMU_ALLOC_PAGE_SIZE + virt;
	ret = phytium_npu_prepare_mmu_source(pnmctx, mmu_pc, map_virt);
	if (ret) {
		pr_err("Err for preparing mmu source.");
		return -EINVAL;
	}
	entry = phytium_npu_get_mmu_pc_entry(map_virt);
	pd_entry = phytium_npu_get_mmu_pd_entry(map_virt);
	pd = mmu_pc->pd[pd_entry];
	phytium_npu_write_mmu_pc(mmu_pc, entry, pd, PHYT_MMU_ENTRY_VALID);
	mmu_pc->ref_count++;

	entry = phytium_npu_get_mmu_entry(map_virt);
	pt = pd->pt;

	phytium_npu_write_mmu_pd(pd, pd_entry, pt, PHYT_MMU_ENTRY_VALID);
	pd->ref_count++;
	phytium_npu_write_mmu_pt(pt, entry, phys, ptflags);
	pt->valid++;
	phytium_npu_debug_mmu_tlb_info(pnmctx, map_virt);
	return 0;
}

static void phytium_npu_mmu_get_ctx_pc_base_address(struct phytium_npu_mmu_context *pnmctx)
{
	struct npu_mmu_ctx *mctx = pnmctx->mctx;

	pnmctx->pc_base_phys_addr = mctx->mmu_pc->desc.phys >> PHYT_MMU_PC_ADDR_SHIFT;
}

static int phytium_npu_mmu_map_sg(struct phytium_npu_session *sess, struct npu_mctx_map *ncmap,
				  struct sg_table *sgt)
{
	struct scatterlist *sg;
	struct phytium_npu_mmu_context *pnmctx;
	u64 phys_addr;
	int len, i, flags = 0;

	/* the driver will use self virtual address when
	 * the user's config was not included virt_addr
	 */
	if (!ncmap->virt_addr)
		ncmap->virt_addr = phytium_npu_get_mmu_base_addr(sess);
	PRDEBUG("%s: current virt base  %llx\n", __func__, ncmap->virt_addr);
	if (ncmap->map_type & NPU_MAP_TYPE_INFERENCE)
		pnmctx = &sess->mmu_ctx[NPU_MMU_CONTEXT_MODULE_ID];
	else
		pnmctx = &sess->mmu_ctx[NPU_MMU_CONTEXT_IO_ID];
	flags = ncmap->map_type & NPU_MAP_TYPE_RO ? 0x2 : 0;
	flags |= PHYT_MMU_ENTRY_VALID;

	if (unlikely(!pnmctx->pc_base_phys_addr))
		phytium_npu_mmu_get_ctx_pc_base_address(pnmctx);

	for_each_sgtable_dma_sg(sgt, sg, i) {
		phys_addr = sg_dma_address(sg);
		len = sg_dma_len(sg);
		if (!PAGE_ALIGNED(phys_addr) || !PAGE_ALIGNED(len))
			PRDEBUG("%s:ERROR addr %llx or len %x is not page aligned!\n",
				__func__, phys_addr, len);

		phytium_npu_mmu_map_addr(pnmctx, ncmap, ncmap->virt_addr, phys_addr, i, flags);
	}

	return 0;
}

static int phytium_npu_mmu_unmap_sg(struct npu_mctx_map *ncmap, struct sg_table *sgt)
{
	struct scatterlist *sg;
	struct phytium_npu_mmu_context *pnmctx = ncmap->mctx->pnmctx;
	int i;

	if (!ncmap->mctx || !ncmap->dma_buf) {
		pr_err("ncmap->mctx or ncmap->dma_buf is NULL");
		return 0;
	}
	if (!pnmctx) {
		PRDEBUG("----------------------------------------------------.");
		return 0;
	}
	pr_debug("unmap mmu base virt:%#llx.pnmctx:%p,ncmap:%p", ncmap->virt_addr, pnmctx, ncmap);
	for_each_sgtable_dma_sg(sgt, sg, i) {
		PRDEBUG("unmap mmu virt:%#llx.", ncmap->virt_addr + i * 4096);
		phytium_npu_mmu_unmap_addr(pnmctx, ncmap, ncmap->virt_addr, i);
	}
	if (ncmap->attach && ncmap->sgt && ncmap->dma_buf) {
		dma_buf_unmap_attachment(ncmap->attach, ncmap->sgt, DMA_BIDIRECTIONAL);
		dma_buf_detach(ncmap->dma_buf, ncmap->attach);
		dma_buf_put(ncmap->dma_buf);
		pr_debug("-----put dma buf fd:%d", ncmap->dma_buf_fd);
	} else {
		pr_err("who is null ? attach:%s, sgt:%s, dam_buf:%s", ncmap->attach ? "OK" : NULL,
		       ncmap->sgt ? "OK" : NULL, ncmap->dma_buf ? "OK" : NULL);
	}
	return 0;
}

int phytium_npu_mmu_unmap(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			  struct npu_memory_unmap *usr_unmmap)
{
	struct npu_mctx_map *ncmap;

	pr_debug("%s, unmap buf fd:%d", __func__, usr_unmmap->fd);
	ncmap = phytium_npu_find_mmu_ctx_map(sess, usr_unmmap->fd);
	if (!ncmap)	{
		dev_err(npu->dev, "Not find npu mmu ctx form sess(%d).", sess->id);
		return -EINVAL;
	}

	phytium_npu_mmu_unmap_sg(ncmap, ncmap->sgt);
	phytium_npu_mmu_map_destroy(npu, ncmap);
	ncmap = NULL;

	return 0;
}

int phytium_npu_import_dmabuf(struct phytium_npu_dev *npu,
			      struct phytium_npu_session *sess,
								struct npu_memory_map *usr_mmap)
{
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct npu_mctx_map *ncmap;
	struct dma_buf *buf;
	int err;

	ncmap = phytium_npu_find_mmu_ctx_map(sess, usr_mmap->fd);
	if (!ncmap)	{
		dev_err(npu->dev, "Not find npu mmu ctx form sess(%d).", sess->id);
		return -EINVAL;
	}

	buf = ncmap->dma_buf;
	attach = dma_buf_attach(buf, npu->dev);
	if (IS_ERR(attach)) {
		err = PTR_ERR(attach);
		goto free;
	}

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = PTR_ERR(sgt);
		goto detach;
	}
	/* map to mmu tlb */
	if (!npu->nmmu_config.mode_bypass) {
		err = phytium_npu_mmu_map_sg(sess, ncmap, sgt);
		if (err < 0)
			goto detach;
	}
	ncmap->dma_buf = buf;
	ncmap->attach = attach;
	ncmap->sgt = sgt;
	ncmap->virt_addr = usr_mmap->vaddr;

	return 0;
detach:
	if (!IS_ERR_OR_NULL(sgt))
		dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);

	dma_buf_detach(buf, attach);
	dma_buf_put(buf);
	pr_debug("put dma buf fd:%d", usr_mmap->fd);
free:
	kfree(attach);

	return err;
}

static int phytium_npu_release_mmu_source(struct phytium_npu_dev *npudev,
					  struct phytium_npu_session *sess)
{
	struct phytium_npu_mmu_context *pnmctx;
	struct npu_mctx_map *ncmap = NULL, *tmp;
	size_t i;

	WARN_ON(!sess);

	for (i = 0; i < ARRAY_SIZE(sess->mmu_ctx); i++) {
		pnmctx = &sess->mmu_ctx[i];
		if (!pnmctx->mctx) {
			pr_err("pnmctx->mctx is NULL in mmu_ctx[%ld],%s", i, __func__);
			continue;
		}
		list_for_each_entry_safe(ncmap, tmp, &pnmctx->mctx->maplist, mctx_map_entry) {
			phytium_npu_mmu_unmap_sg(ncmap, ncmap->sgt);
			phytium_npu_mmu_map_destroy(npudev, ncmap);
			ncmap = NULL;
		}
	}

	return 0;
}
