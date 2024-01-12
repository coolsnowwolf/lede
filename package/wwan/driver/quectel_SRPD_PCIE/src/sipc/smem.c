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
#include <linux/genalloc.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "../include/sipc.h"
#include "sipc_priv.h"
#define CONFIG_SPRD_IPA_PCIE_WORKROUND
/*
 * workround: Due to orca ipa hardware limitations
 * the sipc share memory must map from
 * 0x2x0000000(orca side) to 0xx0000000(roc1
 * side), and the size must be 256M
 */
#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
#define IPA_GET_SRC_BASE(addr)	(((addr) & 0xf0000000) + 0x200000000)
#define IPA_GET_DST_BASE(addr)	((addr) & 0xf0000000)
#define IPA_SIZE	0x10000000
#endif

struct smem_phead {
	struct list_head	smem_phead;
	spinlock_t		lock;
	u32			poolnum;
};

struct smem_pool {
	struct list_head	smem_head;
	struct list_head	smem_plist;
	spinlock_t		lock;

	void	*pcie_base;
	u32	addr;
	u32	size;
	u32	dst;
	u32	mem_type;

	atomic_t	used;
	struct gen_pool	*gen;
};

struct smem_record {
	struct list_head smem_list;
	struct task_struct *task;
	u32 size;
	u32 addr;
};

struct smem_map_list {
	struct list_head map_head;
	spinlock_t	lock;
	u32		inited;
};

struct smem_map {
	struct list_head	map_list;
	struct task_struct	*task;
	const void		*mem;
	unsigned int		count;
};

static struct smem_phead	sipc_smem_phead;
static struct smem_map_list	mem_mp;

static struct smem_pool *shmem_find_pool(u8 dst)
{
	struct smem_phead *phead = &sipc_smem_phead;
	struct smem_pool *spool = NULL;
	struct smem_pool *pos;
	unsigned long flags;

	/* The num of one pool is 0, means the poll is not ready */
	if (!phead->poolnum)
		return NULL;

	spin_lock_irqsave(&phead->lock, flags);
	list_for_each_entry(pos, &phead->smem_phead, smem_plist) {
		if (pos->dst == dst) {
			spool = pos;
			break;
		}
	}
	spin_unlock_irqrestore(&phead->lock, flags);
	return spool;
}

static void *soc_modem_ram_vmap(phys_addr_t start, size_t size, int noncached)
{
	struct page **pages;
	phys_addr_t page_start;
	unsigned int page_count;
	pgprot_t prot;
	unsigned int i;
	void *vaddr;
	phys_addr_t addr;
	unsigned long flags;
	struct smem_map *map;
	struct smem_map_list *smem = &mem_mp;

	map = kzalloc(sizeof(struct smem_map), GFP_KERNEL);
	if (!map)
		return NULL;

	page_start = start - offset_in_page(start);
	page_count = DIV_ROUND_UP(size + offset_in_page(start), PAGE_SIZE);
	if (noncached)
		prot = pgprot_noncached(PAGE_KERNEL);
	else
		prot = PAGE_KERNEL;

	pages = kmalloc_array(page_count, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		kfree(map);
		return NULL;
	}

	for (i = 0; i < page_count; i++) {
		addr = page_start + i * PAGE_SIZE;
		pages[i] = pfn_to_page(addr >> PAGE_SHIFT);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 5,10,0 ))
	vaddr = vm_map_ram(pages, page_count, -1, prot);
#else
	vaddr = vmap(pages, page_count, -1, prot);
	//vaddr = vm_map_ram(pages, page_count, -1);
#endif
    
	kfree(pages);

	if (!vaddr) {
		pr_err("smem: vm map failed.\n");
		kfree(map);
		return NULL;
	}

	vaddr += offset_in_page(start);
	map->count = page_count;
	map->mem = vaddr;
	map->task = current;

	if (smem->inited) {
		spin_lock_irqsave(&smem->lock, flags);
		list_add_tail(&map->map_list, &smem->map_head);
		spin_unlock_irqrestore(&smem->lock, flags);
	}
	return vaddr;
}

static void *pcie_modem_ram_vmap(phys_addr_t start, size_t size, int noncached)
{
	if (noncached == 0) {
		pr_err("%s: cache not support!\n", __func__);
		return NULL;
	}

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	return sprd_ep_map_memory(PCIE_EP_MODEM, start, size);
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
	return sprd_pci_epf_map_memory(SPRD_FUNCTION_0, start, size);
#endif

	return NULL;
}

static void pcie_modem_ram_unmap(const void *mem)
{
#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	return sprd_ep_unmap_memory(PCIE_EP_MODEM, mem);
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
	return sprd_pci_epf_unmap_memory(SPRD_FUNCTION_0, mem);
#endif
}

static void soc_modem_ram_unmap(const void *mem)
{
	struct smem_map *map, *next;
	unsigned long flags;
	struct smem_map_list *smem = &mem_mp;
	bool found = false;

	if (smem->inited) {
		spin_lock_irqsave(&smem->lock, flags);
		list_for_each_entry_safe(map, next, &smem->map_head, map_list) {
			if (map->mem == mem) {
				list_del(&map->map_list);
				found = true;
				break;
			}
		}
		spin_unlock_irqrestore(&smem->lock, flags);

		if (found) {
			vm_unmap_ram(mem - offset_in_page(mem), map->count);
			kfree(map);
		}
	}
}

static void *shmem_ram_vmap(u8 dst, phys_addr_t start,
			    size_t size,
			    int noncached)
{
	struct smem_pool *spool;

	spool = shmem_find_pool(dst);
	if (spool == NULL) {
		pr_err("%s: pool dst %d is not existed!\n", __func__, dst);
		return NULL;
	}

	if (spool->mem_type == SMEM_PCIE) {
		if (start < spool->addr
		    || start + size > spool->addr + spool->size) {
			pr_info("%s: error, start = 0x%lx, size = 0x%lx.\n",
				__func__,
				(unsigned long)start,
				(unsigned long)size);
			return NULL;
		}

		pr_info("%s: succ, start = 0x%lx, size = 0x%lx.\n",
			__func__, (unsigned long)start, (unsigned long)size);
		return (spool->pcie_base + start - spool->addr);
	}

	return soc_modem_ram_vmap(start, size, noncached);

}

int smem_init(u32 addr, u32 size, u32 dst, u32 mem_type)
{
	struct smem_phead *phead = &sipc_smem_phead;
	struct smem_map_list *smem = &mem_mp;
	struct smem_pool *spool;
	unsigned long flags;

	/* fisrt init, create the pool head */
	if (!phead->poolnum) {
		spin_lock_init(&phead->lock);
		INIT_LIST_HEAD(&phead->smem_phead);
	}

	if (shmem_find_pool(dst))
		return 0;

	spool = kzalloc(sizeof(struct smem_pool), GFP_KERNEL);
	if (!spool)
		return -1;

	spin_lock_irqsave(&phead->lock, flags);
	list_add_tail(&spool->smem_plist, &phead->smem_phead);
	phead->poolnum++;
	spin_unlock_irqrestore(&phead->lock, flags);

	spool->addr = addr;
	spool->dst = dst;
	spool->mem_type = mem_type;

	if (size >= SMEM_ALIGN_POOLSZ)
		size = PAGE_ALIGN(size);
	else
		size = ALIGN(size, SMEM_ALIGN_BYTES);

	spool->size = size;
	atomic_set(&spool->used, 0);
	spin_lock_init(&spool->lock);
	INIT_LIST_HEAD(&spool->smem_head);

	spin_lock_init(&smem->lock);
	INIT_LIST_HEAD(&smem->map_head);
	smem->inited = 1;

	/* allocator block size is times of pages */
	if (spool->size >= SMEM_ALIGN_POOLSZ)
		spool->gen = gen_pool_create(PAGE_SHIFT, -1);
	else
		spool->gen = gen_pool_create(SMEM_MIN_ORDER, -1);

	if (!spool->gen) {
		pr_err("Failed to create smem gen pool!\n");
		return -1;
	}

	if (gen_pool_add(spool->gen, spool->addr, spool->size, -1) != 0) {
		pr_err("Failed to add smem gen pool!\n");
		return -1;
	}
	pr_info("%s: pool addr = 0x%x, size = 0x%x added.\n",
		__func__, spool->addr, spool->size);

	if (mem_type == SMEM_PCIE) {
#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
#ifdef CONFIG_PCIE_EPF_SPRD
		spool->pcie_base = sprd_epf_ipa_map(IPA_GET_SRC_BASE(addr),
                                                   IPA_GET_DST_BASE(addr),
                                                   IPA_SIZE);
		if (!spool->pcie_base)
			return -ENOMEM;

		spool->pcie_base += (addr - IPA_GET_DST_BASE(addr));
#else
		pr_err("Failed to pcie map, can't run here!\n");
		return -ENOMEM;
#endif
#else
		spool->pcie_base = pcie_modem_ram_vmap(addr, size, 1);
#endif
	}
	return 0;
}

/* ****************************************************************** */

int smem_get_area(u8 dst, u32 *base, u32 *size)
{
	struct smem_pool *spool;

	if (!base || !size)
		return -EINVAL;

	spool = shmem_find_pool(dst);
	if (!spool) {
		pr_err("%s: err, dst = %d!\n", __func__, dst);
		return -EINVAL;
	}

	pr_info("%s: addr = 0x%x, size = 0x%x.\n",
		__func__, spool->addr, spool->size);

	*base = spool->addr;
	*size = spool->size;

	return 0;
}
EXPORT_SYMBOL_GPL(smem_get_area);

u32 smem_alloc(u8 dst, u32 size)
{
	struct smem_pool *spool;
	struct smem_record *recd;
	unsigned long flags;
	u32 addr = 0;

	spool = shmem_find_pool(dst);
	if (spool == NULL) {
		pr_err("%s: pool dst %d is not existed!\n", __func__, dst);
		return 0;
	}

	recd = kzalloc(sizeof(struct smem_record), GFP_KERNEL);
	if (!recd)
		return 0;

	if (spool->size >= SMEM_ALIGN_POOLSZ)
		size = PAGE_ALIGN(size);
	else
		size = ALIGN(size, SMEM_ALIGN_BYTES);

	addr = gen_pool_alloc(spool->gen, size);
	if (!addr) {
		pr_err("%s:pool dst=%d, size=0x%x failed to alloc smem!\n",
		       __func__, dst, size);
		kfree(recd);
		return 0;
	}

	/* record smem alloc info */
	atomic_add(size, &spool->used);
	recd->size = size;
	recd->task = current;
	recd->addr = addr;
	spin_lock_irqsave(&spool->lock, flags);
	list_add_tail(&recd->smem_list, &spool->smem_head);
	spin_unlock_irqrestore(&spool->lock, flags);

	return addr;
}
EXPORT_SYMBOL_GPL(smem_alloc);

void smem_free(u8 dst, u32 addr, u32 size)
{
	struct smem_pool *spool;
	struct smem_record *recd, *next;
	unsigned long flags;

	spool = shmem_find_pool(dst);
	if (spool == NULL) {
		pr_err("%s: pool dst %d is not existed!\n", __func__, dst);
		return;
	}

	if (size >= SMEM_ALIGN_POOLSZ)
		size = PAGE_ALIGN(size);
	else
		size = ALIGN(size, SMEM_ALIGN_BYTES);

	atomic_sub(size, &spool->used);
	gen_pool_free(spool->gen, addr, size);
	/* delete record node from list */
	spin_lock_irqsave(&spool->lock, flags);
	list_for_each_entry_safe(recd, next, &spool->smem_head, smem_list) {
		if (recd->addr == addr) {
			list_del(&recd->smem_list);
			kfree(recd);
			break;
		}
	}
	spin_unlock_irqrestore(&spool->lock, flags);
}
EXPORT_SYMBOL_GPL(smem_free);

void *shmem_ram_vmap_nocache(u8 dst, phys_addr_t start, size_t size)
{
	return shmem_ram_vmap(dst, start, size, 1);
}
EXPORT_SYMBOL_GPL(shmem_ram_vmap_nocache);


void *shmem_ram_vmap_cache(u8 dst, phys_addr_t start, size_t size)
{
	return shmem_ram_vmap(dst, start, size, 0);
}
EXPORT_SYMBOL_GPL(shmem_ram_vmap_cache);

void shmem_ram_unmap(u8 dst, const void *mem)
{
	struct smem_pool *spool;

	spool = shmem_find_pool(dst);
	if (spool == NULL) {
		pr_err("%s: pool dst %d is not existed!\n", __func__, dst);
		return;
	}

	if (spool->mem_type == SMEM_PCIE)
		/* do nothing,  because it also do nothing in shmem_ram_vmap */
		return;
	else
		return soc_modem_ram_unmap(mem);
}
EXPORT_SYMBOL_GPL(shmem_ram_unmap);

void *modem_ram_vmap_nocache(u32 modem_type, phys_addr_t start, size_t size)
{
	if (modem_type == PCIE_MODEM)
		return pcie_modem_ram_vmap(start, size, 1);
	else
		return soc_modem_ram_vmap(start, size, 1);
}
EXPORT_SYMBOL_GPL(modem_ram_vmap_nocache);


void *modem_ram_vmap_cache(u32 modem_type, phys_addr_t start, size_t size)
{
	if (modem_type == PCIE_MODEM)
		return pcie_modem_ram_vmap(start, size, 0);
	else
		return soc_modem_ram_vmap(start, size, 0);
}
EXPORT_SYMBOL_GPL(modem_ram_vmap_cache);

void modem_ram_unmap(u32 modem_type, const void *mem)
{
	if (modem_type == PCIE_MODEM)
		return pcie_modem_ram_unmap(mem);
	else
		return soc_modem_ram_unmap(mem);
}
EXPORT_SYMBOL_GPL(modem_ram_unmap);

#ifdef CONFIG_DEBUG_FS
static int smem_debug_show(struct seq_file *m, void *private)
{
	struct smem_phead *phead = &sipc_smem_phead;
	struct smem_pool *spool, *pos;
	struct smem_record *recd;
	u32 fsize;
	unsigned long flags;
	u32 cnt = 1;

	spin_lock_irqsave(&phead->lock, flags);
	list_for_each_entry(pos, &phead->smem_phead, smem_plist) {
		spool = pos;
		fsize = gen_pool_avail(spool->gen);

		sipc_debug_putline(m, '*', 80);
		seq_printf(m, "%d, dst:%d, name: %s, smem pool info:\n",
			   cnt++, spool->dst,
			   (smsg_ipcs[spool->dst])->name);
		seq_printf(m, "phys_addr=0x%x, total=0x%x, used=0x%x, free=0x%x\n",
			spool->addr, spool->size, spool->used.counter, fsize);
		seq_puts(m, "smem record list:\n");

		list_for_each_entry(recd, &spool->smem_head, smem_list) {
			seq_printf(m, "task %s: pid=%u, addr=0x%x, size=0x%x\n",
				recd->task->comm,
				recd->task->pid,
				recd->addr,
				recd->size);
		}
	}
	spin_unlock_irqrestore(&phead->lock, flags);
	return 0;
}

static int smem_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, smem_debug_show, inode->i_private);
}

static const struct file_operations smem_debug_fops = {
	.open = smem_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int smem_init_debugfs(void *root)
{
	if (!root)
		return -ENXIO;
	debugfs_create_file("smem", 0444,
			    (struct dentry *)root, NULL,
			    &smem_debug_fops);
	return 0;
}
EXPORT_SYMBOL_GPL(smem_init_debugfs);

#endif /* endof CONFIG_DEBUG_FS */


MODULE_AUTHOR("Chen Gaopeng");
MODULE_DESCRIPTION("SIPC/SMEM driver");
MODULE_LICENSE("GPL v2");
