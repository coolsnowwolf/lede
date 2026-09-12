// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver main entrance.
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

#include <linux/io.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/shmem_fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pm_runtime.h>
#include <drm/drm_modeset_helper.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#ifdef __amd64__
#include <asm/set_memory.h>
#endif
/* Our header */
#include "ftv310_vpu_priv.h"
#include "ftv310e.h"
#include "ftv310_vpu_dec.h"
#include "ftv310_vpu_vcmd.h"
#include "ftv310_vpu_cache.h"
#include "ftv310_vpu_dec400.h"
#include "ftv310_vpu_axife.h"
#include "ftv310_vpu_mmu.h"
#include <linux/of_reserved_mem.h>
#include <linux/of_irq.h>
#include "ftv310_vpu_fpga_mem.h"
#include "ftv310_vpu_power.h"

#ifdef PCIE_EN
#include "ftv310_vpu_pcie.h"
#endif

extern struct vcmd_config vcmd_core_array[1][2];
struct ftv310_vpu_device_handle ftv310_vpu_dev;
static int useirq = 1;
int dec_irq_num = -1;
int enc_irq_num = -1;
long mem_start = 0;
long mem_size = 0;
int dbg_mmap;
int power_enable = 1;

module_param(useirq, int, 0444);
module_param(dbg_mmap, int, 0644);
module_param(power_enable, int, 0644);

void ftv310_vpu_vram_backup(void);
void ftv310_vpu_vram_restore(void);

#if KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE
void debug_dma_alloc_coherent(struct device *dev, size_t size,
			      dma_addr_t dma_addr, void *virt)
{
}

void debug_dma_free_coherent(struct device *dev, size_t size, void *virt,
			     dma_addr_t addr)
{
}
#endif

#ifndef virt_to_bus
static inline unsigned long virt_to_bus(void *address)
{
	return (unsigned long)address;
}
#endif

struct drm_gem_object *
ftv310_vpu_get_gem_from_dmabuf(struct dma_buf *dma_buf)
{
	struct drm_gem_ftv310_vpu_object *cma_obj =
		(struct drm_gem_ftv310_vpu_object
			 *)(((struct dmapriv *)dma_buf->priv)->self);

	if (cma_obj)
		return &cma_obj->base;

	return NULL;
}

int ftv310_vpu_memalloc(struct ftv310_vpu_mem_handle *phandle)
{
	struct slice_info *pslice = getslicenode(phandle->sliceidx);
	unsigned int config = pslice->config;

	if (!pslice)
		return -EINVAL;
	/*mmu part is not really finished yet since it needs op with MMU HW*/
	phandle->size = PAGE_ALIGN(phandle->size);
	if ((GET_ENC_CFG_BITS(config) & CONFIG_FTV310_VPUMMU) ||
		(GET_DEC_CFG_BITS(config) & CONFIG_FTV310_VPUMMU)) {
		phandle->vaddr = vmalloc(phandle->size);
		phandle->paddr = page_to_phys(vmalloc_to_page(phandle->vaddr));
	} else {
		phandle->vaddr = dma_alloc_coherent(pslice->dev, phandle->size,
						    &phandle->paddr,
						    GFP_KERNEL | GFP_DMA);
	}
	if (!phandle->vaddr)
		return -ENOMEM;
	else
		return 0;
}

int ftv310_vpu_memfree(struct ftv310_vpu_mem_handle *phandle)
{
	struct slice_info *pslice = getslicenode(phandle->sliceidx);
	unsigned int config = pslice->config;

	if (!pslice)
		return -EINVAL;

	if ((GET_ENC_CFG_BITS(config) & CONFIG_FTV310_VPUMMU) ||
		(GET_DEC_CFG_BITS(config) & CONFIG_FTV310_VPUMMU))
		vfree(phandle->vaddr);
	else
		dma_free_coherent(pslice->dev, phandle->size, phandle->vaddr,
				  phandle->paddr);
	return 0;
}

#ifdef PHY_CONFIG_PM
static int set_platform_drvdata(struct platform_device *pdev)
{
	void *data;
#ifdef HAS_VCMD
	data = (void *)get_vcmd_slice_head();
#else
	data = (void *)getslicenode(0);
#endif
	ftv310_vpu_dev.ftv310_vpu_data = data;
	platform_set_drvdata(pdev, ftv310_vpu_dev.ftv310_vpu_data);

	return 0;
}
#endif

#ifdef USE_DTB_PROBE

int getnodetype(const char *name)
{
	if (strstr(name, NODENAME_DECODER) == name)
		return FTV310_VPU_CORE_DEC;
	if (strstr(name, NODENAME_ENCODER) == name)
		return FTV310_VPU_CORE_ENC;
	if (strstr(name, NODENAME_CACHE) == name)
		return FTV310_VPU_CORE_CACHE;
	if (strstr(name, NODENAME_DEC400) == name)
		return FTV310_VPU_CORE_DEC400;
	if (strstr(name, NODENAME_AXIFE) == name)
		return FTV310_VPU_CORE_AXIFE;
	if (strstr(name, NODENAME_MMU) == name)
		return FTV310_VPU_CORE_MMU;
	if (strstr(name, NODENAME_VCMD) == name)
		return FTV310_VPU_CORE_VCMD;
	if (strstr(name, NODENAME_IM) == name)
		return FTV310_VPU_CORE_IM;
	if (strstr(name, NODENAME_DECJPG) == name)
		return FTV310_VPU_CORE_DECJPG;
	if (strstr(name, NODENAME_ENCJPG) == name)
		return FTV310_VPU_CORE_ENCJPG;

	return FTV310_VPU_CORE_UNKNOWN;
}

static dtbnode *trycreatenode(struct platform_device *pdev,
			      struct device_node *ofnode, int sliceidx,
			      int parenttype, phys_addr_t parentaddr)
{
	struct fwnode_handle *fwnode;
	struct resource r;
	int i, na, ns, ret;
	int endian = of_device_is_big_endian(ofnode);
	u32 reg_u32[4];
	u64 ioaddress, iosize;

	dtbnode *pnode = kzalloc(sizeof(dtbnode), GFP_KERNEL);

	if (!pnode)
		return NULL;

	pr_info("try create node %s", ofnode->name);
	pnode->type = getnodetype(ofnode->name);
	pnode->parentaddr = parentaddr;
	pnode->parenttype = parenttype;
	pnode->sliceidx = sliceidx;
	pnode->ofnode = ofnode;
	fwnode = &ofnode->fwnode;

	na = of_n_addr_cells(ofnode);
	ns = of_n_size_cells(ofnode);
	if (na > 2 || ns > 2) {
		pr_err("cell size too big");
		kfree(pnode);
		return NULL;
	}

	fwnode_property_read_u32_array(fwnode, "reg", reg_u32, na + ns);
	if (na == 2) {
		if (!endian) {
			ioaddress = reg_u32[0];
			ioaddress <<= 32;
			ioaddress |= reg_u32[1];
		} else {
			ioaddress = reg_u32[1];
			ioaddress <<= 32;
			ioaddress |= reg_u32[0];
		}
	} else {
		ioaddress = reg_u32[0];
	}
	if (ns == 2) {
		if (!endian) {
			iosize = reg_u32[na];
			iosize <<= 32;
			iosize |= reg_u32[na + 1];
		} else {
			iosize = reg_u32[na + 1];
			iosize <<= 32;
			iosize |= reg_u32[na];
		}
	} else {
		iosize = reg_u32[na];
	}
	pnode->ioaddr = ioaddress;
	pnode->iosize = iosize;
	pr_info("node regio =%llx:%llx", ioaddress, iosize);

	for (i = 0; i < 4; i++) {
		pnode->irq[i] = -1;
		if (of_irq_to_resource(ofnode, i, &r) > 0) {
			int irq = of_irq_get(ofnode, i);

			pr_info("irq %d:%s = %lld:%d", i, r.name, r.start, irq);
			if (irq > 0)
				pnode->irq[i] = irq;
		}
	}

	switch (pnode->type) {
	case FTV310_VPU_CORE_DEC:
#ifdef HAS_VCD
		ret = ftv310_vpu_dec_probe(pnode, useirq, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_ENC:
#ifdef HAS_VCE
		ret = ftv310_vpu_enc_probe(pnode, useirq, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_CACHE:
#ifdef HAS_CACHECORE
		ret = cache_probe(pnode, useirq, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_DEC400:
#ifdef HAS_DEC400
		ret = ftv310_vpu_dec400_probe(pnode, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_AXIFE:
#ifdef HAS_AXIFE
		ret = ftv310_vpu_axife_probe(pnode, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_MMU:
#ifdef HAS_MMU
		ret = ftv310_vpu_MMUprobe(pnode, 0, NULL, pdev, 0, 0, NULL);
#endif
		break;
	case FTV310_VPU_CORE_VCMD:
		ret = 0;
		//ret;	//add VCMD node data analyze here
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (ret < 0) {
		kfree(pnode);
		pnode = NULL;
	}
	return pnode;
}

static int ftv310_vpu_analyze_subnode(struct platform_device *pdev,
				  struct device_node *slice, int sliceidx)
{
	dtbnode *head, *nhead, *newtail, *node;

	pr_info("dev node %s", slice->name);

	head = kzalloc(sizeof(dtbnode), GFP_KERNEL);
	if (!head)
		return -ENOMEM;
	head->type = FTV310_VPU_CORE_SLICE;
	head->parenttype = FTV310_VPU_CORE_SLICE;
	head->ofnode = slice;
	head->ioaddr = -1;
	head->iosize = 0;
	head->next = NULL;

	/*
	 *this is a wide first tree structure iteration,
	 *result is stored in slice info
	 */
	while (head) {
		newtail = NULL;
		nhead = newtail;
		while (head) {
			struct device_node *child, *ofnode = head->ofnode;

			for_each_child_of_node(ofnode, child) {
				node = trycreatenode(pdev, child, sliceidx,
						     head->type, head->ioaddr);
				if (node) {
					if (!nhead) {
						newtail = node;
						nhead = newtail;
					} else {
						newtail->next = node;
					}
					node->next = NULL;
					newtail = node;
				}
			}
			node = head->next;
			kfree(head);
			head = node;
		}
		head = nhead;
	}
	return 0;
}
#endif //USE_DTB_PROBE

static struct ftv310_vpu_slice ftv310_vpu_slice_head[MAX_SLICE_NUM] = { 0 };

int ftv310_vpu_get_vcmdsup(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	struct ftv310_vpu_slice *ftv310_vpu_slice_node;
	ftv310_vpu_ioctl_id ioctl_id_par;
	u32 slice;

	ioctl_id_par.data = *(unsigned int *)data;
	slice = ioctl_id_par.ID_PAR.node_idx;

	if (slice >= MAX_SLICE_NUM)
		return -1;

	ftv310_vpu_slice_node = &ftv310_vpu_slice_head[slice];
	return ftv310_vpu_slice_node->vcmd_en;
}

static int ftv310_vpu_set_vcmdsup(u32 sliceidx, u32 vcmd_en)
{
	struct ftv310_vpu_slice *ftv310_vpu_slice_node;

	if (sliceidx >= MAX_SLICE_NUM)
		return -1;

	ftv310_vpu_slice_node = &ftv310_vpu_slice_head[sliceidx];
	ftv310_vpu_slice_node->vcmd_en = vcmd_en;
	return 1;
}

int ftv310_vpu_get_irqinfo(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	return useirq;
}

int ftv310_vpu_get_pmsup(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	int pm_support = 0;

#ifdef PHY_CONFIG_PM
	pm_support = 1;
#else
	pm_support = 0;
#endif
	return pm_support;
}

#ifdef USE_DTB_PROBE
static int ftv310_vpu_check_vcmd(struct platform_device *pdev,
			     struct device_node *slice)
{
	struct device_node *child_level_0, *child_level_1;
	int type;

	for_each_child_of_node(slice, child_level_0) {
		for_each_child_of_node(child_level_0, child_level_1) {
			type = getnodetype(child_level_0->name);
			if (type == FTV310_VPU_CORE_VCMD)
				return 1;
		}
	}
	return 0;
}
#endif //USE_DTB_PROBE

#ifndef HAS_VCMD
static int map_register(struct vm_area_struct *vma, unsigned long busaddr, unsigned long size)
{
	size_t mapsize = (vma->vm_end - vma->vm_start);
	int ret;

#if KERNEL_VERSION(6, 3, 0) > LINUX_VERSION_CODE
	vma->vm_flags |= VM_IO;
#else
	vm_flags_set(vma, VM_IO);
#endif
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, busaddr >> PAGE_SHIFT, mapsize,
		vma->vm_page_prot) ? -EAGAIN : 0;

	ftv310_vpu_mmaplog("%s:%lx:%ld=%d", __func__, busaddr, size, ret);
	return ret;
}
#endif

static int map_memblock(struct vm_area_struct *vma, unsigned long busaddr, unsigned long blocksize)
{
	size_t mapsize = vma->vm_end - vma->vm_start;
	int ret = 0;

	if (!(vma->vm_flags & VM_MAYSHARE))
		ret = -EPERM;
	else {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		ret = remap_pfn_range(vma, vma->vm_start, busaddr >> PAGE_SHIFT,
					mapsize, vma->vm_page_prot) ? -EAGAIN : 0;
	}
	ftv310_vpu_mmaplog("%s:%lx:%ld=%d", __func__, busaddr, blocksize, ret);
	return ret;
}


int ftv310_vpu_map_internal_address(struct file *filp, struct vm_area_struct *vma)
{
	ftv310_vpu_ioctl_id coreid;
	u32 maincoretype;

	coreid.data = (u32)vma->vm_pgoff;
	ftv310_vpu_mmaplog("%s:nodeid=%d,maincore=%d:subtype=%d:coreidx=%d", __func__,
		coreid.ID_PAR.node_idx, coreid.ID_PAR.group_idx, coreid.ID_PAR.sub_mod_idx, coreid.ID_PAR.codec_idx);
	if (coreid.ID_PAR.group_idx == CODEC_DEC_FLGA)
		maincoretype = NODE_TYPE_DEC;
	else if (coreid.ID_PAR.group_idx == CODEC_ENC_FLGA)
		maincoretype = NODE_TYPE_ENC;
	else
		return -ENODEV;
	switch (coreid.ID_PAR.sub_mod_idx) {
#ifndef HAS_VCMD
	case CORE_FLAG:
		if (maincoretype == NODE_TYPE_DEC) {
			struct ftv310_vpu_dec_t *pdec = get_decnodes(coreid.ID_PAR.node_idx, coreid.ID_PAR.codec_idx);

			if (pdec)
				return map_register(vma, pdec->multicorebase_actual, pdec->iosize);
			else
				return -ENODEV;
		}
		if (maincoretype == NODE_TYPE_ENC) {
			struct ftv310_vpu_enc_t *penc = get_encnodes(coreid.ID_PAR.node_idx, coreid.ID_PAR.codec_idx);

			if (penc)
				return map_register(vma, penc->core_cfg.base_addr, penc->core_cfg.iosize);
			else
				return -ENODEV;
		}
		break;
	case DEC400_FLAG:
		{
			struct dec400_t *pdec400 = get_dec400nodebytype(coreid.ID_PAR.node_idx, maincoretype, coreid.ID_PAR.codec_idx);

			if (pdec400)
				return map_register(vma, pdec400->core_cfg.dec400corebase, pdec400->core_cfg.iosize);
			else
				return -ENODEV;
		}
	case CACHE_FLAG:
		{
			struct cache_dev_t *pcache = get_cachenodebytype(coreid.ID_PAR.node_idx, maincoretype, coreid.ID_PAR.codec_idx);

			if (pcache)
				return map_register(vma, pcache->core_cfg.base_addr, pcache->core_cfg.iosize);
			else
				return -ENODEV;
		}
	case MMU0_FLAG:
	case MMU1_FLAG:
		{
			struct mmu_t *pmmu = get_mmunodebytype(coreid.ID_PAR.node_idx, maincoretype);

			if (pmmu)
				return map_register(vma, pmmu->core_cfg.mmucorebase, pmmu->core_cfg.iosize);
			else
				return -ENODEV;
		}
#else
	case VCMD_FLAG:
	case VCMD_BUF_FLAG:
		{
			vcmd_dev_str *subsys_dev = get_dev_by_sliceidx(coreid.ID_PAR.node_idx, coreid.ID_PAR.group_idx);

			if (subsys_dev) {
				if (coreid.ID_PAR.sub_mod_idx == VCMD_FLAG)
					return map_memblock(vma, subsys_dev->vcmd_buf_mem_pool->busAddress, subsys_dev->vcmd_buf_mem_pool->size);
				else
					return map_memblock(vma, subsys_dev->vcmd_status_buf_mem_pool->busAddress, subsys_dev->vcmd_status_buf_mem_pool->size);
			} else
				return -ENODEV;
		}
#endif
	default:
		return -ENXIO;
	}
	return 0;
}

static int ftv310_vpu_drm_probe(struct platform_device *pdev)
{
#ifdef USE_DTB_PROBE
	struct device *dev = &pdev->dev;
	int result = 0;
	int sliceidx = -1;

	if (dev->of_node) {
		if (ftv310_vpu_check_vcmd(pdev, dev->of_node)) {
			sliceidx = ftv310_vpu_vcmd_probe(pdev, useirq, dev->of_node, 0, 0);
			ftv310_vpu_set_vcmdsup(sliceidx, 1);
			ftv310_vpu_dev.config |= get_vcmd_slice_config(sliceidx);
			transfer_vcmdslice_to_norslice(sliceidx);
			ftv310_vpu_vcmd_init(sliceidx);

		} else {
			//probe from system DTB
			/*try to attach 1st rsv mem to dtb node*/
			result = of_reserved_mem_device_init(dev);
			pr_info("try reserve mem =%d", result);

			if (result == 0)
				sliceidx = addslice(dev, -1, 0);
			else
				/*leave to end of init,
				 *set to default drm platform dev
				 *and default cma area
				 */
				sliceidx = addslice(NULL, -1, 0);

#if USE_HW == 1
			/*go throug all sub dtb node' resources */
			if (sliceidx >= 0 && dev->of_node)
				ftv310_vpu_analyze_subnode(pdev, dev->of_node,
						       sliceidx);
#endif
			ftv310_vpu_set_vcmdsup(sliceidx, 0);
		}
	}
#endif//  USE_DTB_PROBE

	pr_info("dev %s probe", pdev->name);

	return 0;
}

static int ftv310_vpu_drm_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct platform_device_id ftv310_vpu_drm_platform_ids[] = {
	{
		.name = DRIVER_NAME,
	},
	{},
};
MODULE_DEVICE_TABLE(platform, ftv310_vpu_drm_platform_ids);

static const struct of_device_id ftv310_vpu_of_match[] = {
	/*to match dtb, else reg io will fail*/
	{
		.compatible = "thunderbay,ftv310_vpu",
	},
	{ /* sentinel */ }
};

static int ftv310_vpu_power_on(struct device *kdev, vcmd_slice_str *slice)
{
	int ret = 0;

	if(slice){
		ret = ftv310_vpu_power_request(kdev, slice, true, true);
		if(ret) {
			pr_err("phytium: decoder power on is failed!\n");
			return -EBUSY;
		}

		slice->dec_power_status = 1;
			
		ret = ftv310_vpu_power_request(kdev, slice, false, true);
		if(ret){
			pr_err("phytium: encoder power on is failed!\n");
			return -EBUSY;
		}
		slice->enc_power_status = 1;
	}else{
		return -ENOENT;
	}

	return 0;
}

static int ftv310_vpu_power_down(struct device *kdev, vcmd_slice_str *slice)
{
	int ret = 0;
	
	if(slice){
		ret = ftv310_vpu_power_request(kdev, slice, true, false);
		if(ret) {
			pr_err("phytium: decoder power down is failed!\n");
			return -EBUSY;
		}

		slice->dec_power_status = 0;

		ret = ftv310_vpu_power_request(kdev, slice, false, false);
		if(ret) {
			pr_err("phytium: encoder power down is failed!\n");
			return -EBUSY;
		}

		slice->enc_power_status = 0;
	}else{
		return -ENOENT;
	}

	return 0;
}

#ifdef HAS_VCMD
static void has_vcmd_pm_suspend(vcmd_slice_str *slice)
{
	while (slice) {
#ifdef HAS_VCD
		if (slice->dec_vcmd.subsys_num != 0)
			vcmd_pm_suspend(&slice->dec_vcmd);
#endif // HAS_VCD
#ifdef HAS_VCE
		if (slice->enc_vcmd.subsys_num != 0)
			vcmd_pm_suspend(&slice->enc_vcmd);
#endif // HAS_VCE
		slice = slice->slice_next;
	}
}
static void has_vcmd_pm_resume(vcmd_slice_str *slice)
{
	while (slice) {
#ifdef HAS_VCD
		if (slice->dec_vcmd.subsys_num != 0) {
			vcmd_pm_resume(&slice->dec_vcmd);
		}
#endif // HAS_VCD
#ifdef HAS_VCE
		if (slice->enc_vcmd.subsys_num != 0) {
			vcmd_pm_resume(&slice->enc_vcmd);
		}
#endif // HAS_VCE
		slice = slice->slice_next;
	}
}
#else 
static void no_vcmd_pm_suspend(slice_info *slice)
{
	while (slice) {
#ifdef HAS_VCD
		if (slice->deccore_num != 0)
			dec_pm_suspend(slice->dechdr);
#endif // HAS_VCD
#ifdef HAS_VCE
		if (slice->enccore_num != 0)
			enc_pm_suspend(slice->enchdr);
#endif // HAS_VCE
		slice = slice->slice_next;
	}
}
static void no_vcmd_pm_resume(slice_info *slice)
{
	while (slice) {
#ifdef HAS_VCD
		if (slice->deccore_num != 0)
			dec_pm_resume(slice->dechdr);
#endif // HAS_VCD
#ifdef HAS_VCE
		if (slice->enccore_num != 0)
			enc_pm_resume(slice->enchdr);
#endif // HAS_VCE
		slice = slice->slice_next;
	}
}
#endif

static int ftv310_vpu_pm_suspend(struct device *kdev)
{
#ifdef PHY_CONFIG_PM
#ifdef HAS_VCMD
	int ret = 0;
	vcmd_slice_str *slice = dev_get_drvdata(kdev);

	pr_info("ftv310_vpu: enter suspend\n");
	has_vcmd_pm_suspend(slice);
	mdelay(1);
	ret = ftv310_vpu_power_down(kdev,dev_get_drvdata(kdev));
	if(!ret)
		pr_info("phytium: Power down by SE");
#else // HAS_VCMD
	struct slice_info *slice = dev_get_drvdata(kdev);
	no_vcmd_pm_suspend(slice);
#endif // HAS_VCMD
	pr_info("ftv310_vpu: pm suspend successful!\n");
#endif // PHY_CONFIG_PM
	return 0;
}

static int ftv310_vpu_pm_freeze(struct device *dev) {
	int ret;

	pr_info("ftv310_vpu: enter freeze\n");

	ret =  ftv310_vpu_pm_suspend(dev);
	if (ret)
		return ret;
#ifdef PHY_CONFIG_PM
	if (mem_start)
		ftv310_vpu_vram_backup();
#endif

	return 0;
}

static int ftv310_vpu_pm_resume(struct device *kdev)
{
#ifdef PHY_CONFIG_PM
#ifdef HAS_VCMD
	vcmd_slice_str *slice = dev_get_drvdata(kdev);
	int ret = 0;
	
	pr_info("ftv310_vpu: enter resume\n");

	ret = ftv310_vpu_power_on(kdev,slice);
	if(!ret)
		pr_info("phytium: Power on by SE");

	has_vcmd_pm_resume(slice);
#else // HAS_VCMD
	struct slice_info *slice = dev_get_drvdata(kdev);
	no_vcmd_pm_resume(slice);
#endif // HAS_VCMD
	pr_info("ftv310_vpu: pm reseume successful!\n");
#endif // PHY_CONFIG_PM
	return 0;
}

static int ftv310_vpu_pm_restore(struct device *dev) {
	pr_info("ftv310_vpu: enter restore\n");

#ifdef PHY_CONFIG_PM
	if (mem_start)
		ftv310_vpu_vram_restore();
#endif

	return ftv310_vpu_pm_resume(dev);
}

static int ftv310_vpu_pm_thaw(struct device *dev) {
	pr_info("ftv310_vpu: enter thaw\n");
#ifdef PHY_CONFIG_PM
	if (mem_start)
		ftv310_vpu_vram_restore();
#endif
	return ftv310_vpu_pm_resume(dev);
}

static int ftv310_vpu_pm_runtime_suspend(struct device *kdev)
{
	int ret = 0;
	vcmd_slice_str *slice = dev_get_drvdata(kdev);

	if(!power_enable) {
		pr_info("ftv310_vpu: disable runtime power suspend control!\n");
		return 0;
	}
	
	has_vcmd_pm_suspend(slice);

	ret = ftv310_vpu_power_down(kdev, slice);
	if(!ret)
		pr_info("phytium: runtime suspend is successful!");
	/* Add CLk contrl*/
	return 0;
}

static int ftv310_vpu_pm_runtime_resume(struct device *kdev)
{
	int ret = 0;
	vcmd_slice_str *slice = dev_get_drvdata(kdev);

	if(!power_enable) {
		pr_info("ftv310_vpu: disable runtime power resume control!\n");
		return 0;
	}

	ret = ftv310_vpu_power_on(kdev, slice);
	if(!ret)
		pr_info("phytium: runtime resume is successful!");

	has_vcmd_pm_resume(slice);
	/* Add CLk contrl*/
	return 0;
}

static void ftv310_vpu_pm_runtime_enable(struct device *kdev)
{
	//pm_runtime_use_autosuspend(kdev);

	pm_runtime_set_active(kdev);
	pm_runtime_enable(kdev);
}

static void ftv310_vpu_pm_runtime_disable(struct device *kdev)
{
	pm_runtime_disable(kdev);
}

void ftv310_vpu_pm_runtime_get(struct device *kdev)
{
	pm_runtime_get_sync(kdev);
}

void ftv310_vpu_pm_runtime_put(struct device *kdev)
{
	pm_runtime_put_sync(kdev);
}

static const struct dev_pm_ops ftv310_vpu_pm_ops = {
	/* since we only support S3, only several interfaces should be supported
	 * echo -n "freeze" (or sth else) > /sys/power/state will trigger them
	 * current suspend and resume seem to be enough
	 * maybe suspend_noirq and resume_noirq will be inserted in future
	 */
	//.prepare
	.suspend = ftv310_vpu_pm_suspend,
	.freeze = ftv310_vpu_pm_freeze,
	.restore = ftv310_vpu_pm_restore,
	.thaw = ftv310_vpu_pm_thaw,
	//.suspend_late
	//.suspend_noirq

	//.resume_noirq
	//.resume_early
	.resume = ftv310_vpu_pm_resume,
	//.complete
	.runtime_suspend = ftv310_vpu_pm_runtime_suspend,
	.runtime_resume = ftv310_vpu_pm_runtime_resume,
};

static struct platform_driver ftv310_vpu_drm_platform_driver = {
	.probe = ftv310_vpu_drm_probe,
	.remove = ftv310_vpu_drm_remove,
	.driver = {
			.name = DRIVER_NAME,
			.owner = THIS_MODULE,
			.of_match_table = ftv310_vpu_of_match,
			.pm = &ftv310_vpu_pm_ops,
		},
	.id_table = ftv310_vpu_drm_platform_ids,
};

static const struct platform_device_info ftv310_vpu_platform_info = {
	.name = DRIVER_NAME,
	.id = -1,
	.dma_mask = DMA_BIT_MASK(64),
};

static void release_norslice_node(void)
{
	struct slice_info *post, *prev;
	int i, slicen = get_slicenumber();
	struct ftv310_vpu_enc_t *pcore, *pnext;

	pr_debug("%s slicen %d\n", __func__, slicen);
	for (i = 0; i < slicen; i++) {
		pcore = get_encnodes(i, 0);
		while (pcore) {
			pnext = pcore->next;
			pr_debug("%s vfree\n", __func__);
			vfree(pcore);
			pcore = pnext;
		}
	}

	prev = getslicenode_ininit(0);
	post = prev;
	while (prev) {
		post = prev->next;
		pr_debug("%s kfree\n", __func__);
		kfree(prev);
		prev = post;
	}
}

#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
static int ftv310_vpu_major = 1; /* dynamic */
#endif

static void ftv310_vpu_cleanup(void)
{
	int vcmd_en;
	ftv310_vpu_ioctl_id ioctl_id_par;

	ftv310_vpu_dev.config = 0;

	ioctl_id_par.ID_PAR.node_idx = 0;
	vcmd_en = ftv310_vpu_get_vcmdsup(NULL, &ioctl_id_par.data, NULL);

	if (vcmd_en == 0) {
		ftv310_vpu_unlinksysfsAPI(); //this must be before slice clean up

#ifdef HAS_VCD
		ftv310_vpu_dec_cleanup();
#endif

#ifdef HAS_VCE
		ftv310_vpu_enc_cleanup();
#endif

#ifdef HAS_CACHECORE
		cache_cleanup();
#endif
#ifdef HAS_DEC400
		ftv310_vpu_dec400_cleanup();
#endif
#ifdef HAS_MMU
		ftv310_vpu_MMUCleanup();
#endif
#ifdef HAS_AXIFE
		ftv310_vpu_axife_cleanup();
#endif
		/*this one must be after above ones to maintain list*/
		slice_remove();
	} else {
#ifdef HAS_MMU
		ftv310_vpu_MMUCleanup();
#endif

		release_norslice_node();
#ifdef HAS_VCMD
		ftv310_vpu_vcmd_cleanup();
#endif
	}

#ifdef PHY_FPGA_MEM
	ftv310_vpu_fpga_memrelease();
#else
	if (mem_start)
		ftv310_vpu_phytium_mem_release();
#endif

#ifdef PCIE_EN
	pcie_exit(ftv310_vpu_dev.dev);
#endif

	releaseFenceData();
	drm_dev_unregister(ftv310_vpu_dev.drm_dev);
#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
	drm_dev_fini(ftv310_vpu_dev.drm_dev);
#else
	drm_dev_put(ftv310_vpu_dev.drm_dev);
#endif
#ifdef PHY_CONFIG_PM
	if (power_enable)
		ftv310_vpu_pm_runtime_disable(&ftv310_vpu_dev.platformdev->dev);
#endif
}

#ifndef HAS_VCMD
static void __init probe_ftv310_vpu_HW(unsigned long reg_base,
				  unsigned long ddr_base)
{
	int i, k, coren;
	int ret;

	for (i = 0; i < get_slicenumber(); i++) {
		struct slice_info *pslice = getslicenode_ininit(i);

#ifdef HAS_VCD
		coren = get_slicecorenum(i, FTV310_VPU_CORE_DEC);
		for (k = 0; k < coren; k++) {
			struct ftv310_vpu_dec_t *decnode = get_decnodes(i, k);

			if (!decnode)
				break;
			decnode->multicorebase += reg_base;
			decnode->multicorebase_actual += reg_base;

			ret = ftv310_vpu_dec_probe(NULL, useirq, 1, decnode);
			if (ret < 0)
				remove_node(decnode, FTV310_VPU_CORE_DEC);
		}
#endif

#ifdef HAS_VCE
		coren = get_slicecorenum(i, FTV310_VPU_CORE_ENC);
		for (k = 0; k < coren; k++) {
			struct ftv310_vpu_enc_t *encnode = get_encnodes(i, k);

			if (!encnode)
				break;
			encnode->core_cfg.base_addr += reg_base;

			ret = ftv310_vpu_enc_probe(NULL, useirq, 1, encnode);
			if (ret < 0)
				remove_node(encnode, FTV310_VPU_CORE_ENC);
		}
#endif

#ifdef HAS_CACHECORE
		coren = get_slicecorenum(i, FTV310_VPU_CORE_CACHE);
		for (k = 0; k < coren; k++) {
			struct cache_dev_t *cache = get_cachenodes(i, k);

			if (!cache)
				break;
			cache->com_base_addr += reg_base;
			cache->core_cfg.base_addr += reg_base;

			ret = cache_probe(NULL, useirq, 1, cache);
			if (ret < 0)
				remove_node(cache, FTV310_VPU_CORE_CACHE);
		}
#endif

#ifdef HAS_DEC400
		coren = get_slicecorenum(i, FTV310_VPU_CORE_DEC400);
		for (k = 0; k < coren; k++) {
			struct dec400_t *dec400 = get_dec400nodes(i, k);

			if (!dec400)
				break;
			dec400->core_cfg.dec400corebase += reg_base;
			ret = ftv310_vpu_dec400_probe(NULL, 1, dec400);
			if (ret < 0)
				remove_node(dec400, FTV310_VPU_CORE_DEC400);
		}
#endif

#ifdef HAS_AXIFE
		coren = get_slicecorenum(i, FTV310_VPU_CORE_AXIFE);
		for (k = 0; k < coren; k++) {
			struct axife_t *axife = get_axifenodes(i, k);

			if (!axife)
				break;
			axife->core_cfg.axifecorebase += reg_base;
			ret = ftv310_vpu_axife_probe(NULL, 1, axife);
			if (ret < 0)
				remove_node(axife, FTV310_VPU_CORE_AXIFE);
		}
#endif

#ifdef HAS_MMU
		coren = get_slicecorenum(i, FTV310_VPU_CORE_MMU);
		for (k = 0; k < coren; k++) {
			struct mmu_t *mmu = get_mmunode(i, k);

			if (!mmu)
				break;
			mmu->core_cfg.mmucorebase += reg_base;
			ret = ftv310_vpu_MMUprobe(NULL, 1, mmu,
					     ftv310_vpu_dev.platformdev, ddr_base, NULL);
			if (ret < 0)
				remove_node(mmu, FTV310_VPU_CORE_MMU);
		}
#endif
		ftv310_vpu_dev.config |= pslice->config;
	}
}
#endif

static int ftv310_vpu_init(struct platform_device *pdev, uint64_t base, uint32_t irq)
{
	int result, i;
	int slice_num = 0;
	struct ftv310_vpu_base_addr subsystem_base_addr;
#ifdef PCIE_EN
	struct ftv310_vpu_pci_t pci_par;
#endif

	/*
	 *_init functions will init static vairables,
	 *while probe will init dynamic emelemts from DTB
	 */
	/*slice init must be in first to clear list*/
	slice_init();

#ifdef HAS_VCE
	ftv310_vpu_enc_init();
#endif

#ifdef HAS_VCD
	ftv310_vpu_dec_init();
#endif
#ifdef HAS_CACHECORE
	cache_init();
#endif
#ifdef HAS_DEC400
	ftv310_vpu_dec400_init();
#endif
#ifdef HAS_AXIFE
	ftv310_vpu_axife_init();
#endif

	ftv310_vpu_dev.config = 0;
	ftv310_vpu_dev.platformdev = pdev;
	dma_set_mask(&pdev->dev, DMA_BIT_MASK(64));
	dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(64));

	/*it must be here instead of in probe*/
	ftv310_vpu_dev.drm_dev =
		drm_dev_alloc(&ftv310_vpu_drm_driver, &ftv310_vpu_dev.platformdev->dev);
	if (IS_ERR(ftv310_vpu_dev.drm_dev)) {
		DBG("init drm failed\n");
		platform_device_unregister(ftv310_vpu_dev.platformdev);
		platform_driver_unregister(&ftv310_vpu_drm_platform_driver);
		return PTR_ERR(ftv310_vpu_dev.drm_dev);
	}

	ftv310_vpu_dev.drm_dev->dev = &ftv310_vpu_dev.platformdev->dev;
	drm_mode_config_init(ftv310_vpu_dev.drm_dev);
	result = drm_dev_register(ftv310_vpu_dev.drm_dev, 0);

	if (result < 0) {
		drm_dev_unregister(ftv310_vpu_dev.drm_dev);
#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
		drm_dev_fini(ftv310_vpu_dev.drm_dev);
#else
		drm_dev_put(ftv310_vpu_dev.drm_dev);
#endif
		platform_device_unregister(ftv310_vpu_dev.platformdev);
		platform_driver_unregister(&ftv310_vpu_drm_platform_driver);
		return result;
	}
	initFenceData();

#ifndef USE_DTB_PROBE //static table analyze, dec and enc must be in the front
#ifdef PCIE_EN //get reg/ddr base info
	if (pcie_init(&pci_par) < 0)
		pr_debug("%s,%d err pci init failed\n", __func__, __LINE__);
	subsystem_base_addr.reg_base = pci_par.pci_base_reg_hw;
	subsystem_base_addr.ddr_base = pci_par.pci_base_ddr_hw;
	ftv310_vpu_dev.dev = pci_par.dev;
#else
//customer set correctly values
	subsystem_base_addr.reg_base = base;
	subsystem_base_addr.ddr_base = 0;
	pr_info("Maybe need customized region info in %s, line %d\n", __func__, __LINE__);
#endif

#ifdef PHY_FPGA_MEM
	ftv310_vpu_fpga_meminit((unsigned long)pci_par.pci_base_ddr_hw);
#else
	if (mem_start)
		ftv310_vpu_phytium_meminit(pdev, (unsigned long)mem_start, (unsigned long)mem_size);
	//customer init memmory region here if needed
	pr_info("Maybe need customized in %s, line %d\n", __func__, __LINE__);
#endif
#ifdef HAS_VCMD
	slice_num = ftv310_vpu_vcmd_probe(ftv310_vpu_dev.platformdev, useirq, NULL, subsystem_base_addr.ddr_base, subsystem_base_addr.reg_base);
	for (i = 0; i < slice_num; i++) {
		ftv310_vpu_set_vcmdsup(i, 1);
		ftv310_vpu_dev.config |= get_vcmd_slice_config(i);
	}

	result = ftv310_vpu_vcmd_init(&subsystem_base_addr);
	if (result < 0) {
		pr_err("ftv310_vpu_vcmd_init fail\n");
		return result;
	}
#else

#ifdef HAS_VCD
	result = ftv310_vpu_dec_probe(NULL, useirq, 0, NULL);
#endif
#ifdef HAS_VCE
	result = ftv310_vpu_enc_probe(NULL, useirq, 0, NULL);
#endif
#ifdef HAS_CACHECORE
	result = cache_probe(NULL, useirq, 0, NULL);
#endif
#ifdef HAS_DEC400
	result = ftv310_vpu_dec400_probe(NULL, 0, NULL);
#endif
#ifdef HAS_AXIFE
	result = ftv310_vpu_axife_probe(NULL, 0, NULL);
#endif
#ifdef HAS_MMU
	result =
		ftv310_vpu_MMUprobe(NULL, 0, NULL, ftv310_vpu_dev.platformdev,
					   subsystem_base_addr.ddr_base, NULL);
#endif
#endif

#endif //USE_DTB_PROBE

#ifndef HAS_VCMD
	if (get_slicenumber() == 0)
		addslice(ftv310_vpu_dev.drm_dev->dev, -1,
			 0); //for PC, no HW, create a default dev
	for (i = 0; i < get_slicenumber(); i++) {
		struct slice_info *pslice = getslicenode_ininit(i);

		if (!pslice->dev)
			pslice->dev = ftv310_vpu_dev.drm_dev->dev;
		result = ftv310_vpu_createsysfsAPI(i, pslice->dev);
		if (result != 0)
			pr_info("create sysfs %d fail", i);
	}
	slice_printdebug();

	probe_ftv310_vpu_HW(subsystem_base_addr.reg_base, subsystem_base_addr.ddr_base);
	slice_init_finish();
	slice_printdebug();
#endif
#ifdef PHY_CONFIG_PM
	set_platform_drvdata(ftv310_vpu_dev.platformdev);
	if (power_enable)
		ftv310_vpu_pm_runtime_enable(&ftv310_vpu_dev.platformdev->dev);
#endif // PHY_CONFIG_PM

	pr_info("ftv310_vpu device created");
	return 0;
}

static int ftv310_vpu_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct resource *reserved_mem;
	struct device *dev = &pdev->dev;
	int irq;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		DRM_ERROR("get decode register address failed\n");
		goto error;
	}

	if (dev->of_node) {
		reserved_mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);
		if (reserved_mem == NULL) {
			pr_info("get memory from dts failed, use vmalloc!\n");
			mem_start = 0;
			mem_size = 0;
		} else {
			mem_start = reserved_mem->start;
			mem_size = reserved_mem->end - reserved_mem->start + 1;
		}
	} else {
		reserved_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);

		if (reserved_mem) {
			mem_size = resource_size(reserved_mem);
			mem_start = reserved_mem->start;
		} else {
			mem_start = 0;
			mem_size = 0;
		}
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get enc irq\n");
	} else {
		vcmd_core_array[0][0].vcmd_irq = irq;
	}

	irq = platform_get_irq(pdev, 1);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get dec irq\n");
	} else {
		vcmd_core_array[0][1].vcmd_irq = irq;
	}

	ftv310_vpu_init(pdev, res->start, 0);

	return 0;
error:
	return -1;
}

static int ftv310_vpu_remove(struct platform_device *pdev)
{

	ftv310_vpu_cleanup();

	return 0;
}

static const struct of_device_id vpu_of_match[] = {
	{
#if defined HAS_VCD && defined HAS_VCE
		.compatible = "phytium,vpu",
#elif defined HAS_VCD
		.compatible = "phytium,decode",
#else
		.compatible = "phytium,encode",
#endif
	},
	{ }
};

static const struct acpi_device_id vpu_acpi_ids[] = {
	{
#ifdef HAS_VCD
		.id = "PHYT0049",
#else
		.id = "PHYT1111",
#endif
	},
	{},
};

struct platform_driver vpu_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(vpu_of_match),
		.acpi_match_table = ACPI_PTR(vpu_acpi_ids),
		.pm = &ftv310_vpu_pm_ops,
	},
	.probe = ftv310_vpu_probe,
	.remove = ftv310_vpu_remove,
};


static int __init phytium_vpu_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&vpu_platform_driver);

	return ret;
}

static void __exit phytium_vpu_exit(void)
{
	platform_driver_unregister(&vpu_platform_driver);
}

module_init(phytium_vpu_init);
module_exit(phytium_vpu_exit);

#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
MODULE_IMPORT_NS(DMA_BUF);
#endif
/* module description */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Phytium");
MODULE_VERSION("1.0.4");
MODULE_DESCRIPTION("ftv310 DRM manager");
