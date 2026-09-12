/*
 * Homogeneous Remote Processor Control Driver
 *
 * Copyright (c) 2022-2025 Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License version 2 as published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/arm-smccc.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/remoteproc.h>
#include <linux/psci.h>
#include <linux/cpu.h>
#include <linux/cpuhotplug.h>
#include <linux/ktime.h>
#include <asm/cacheflush.h>
#include <linux/processor.h>
#include <linux/of_device.h>

#include "remoteproc_internal.h"

#define RPROC_RESOURCE_ENTRIES      8

/* Support up to two cores */
#define RPROC_CORE_MAX_NUM          2

/* cpu0 handling interrupt */
#define RPROC_IRQ_HANDLE_CPU        0

/* PSCI cpu_on code */
#define CPU_ON                      0xc4000003

/* stop flag use in resource table */
#define REMOTE_PROC_STOP            0x0001U

/* wait for 1s */
#define HOMO_MAX_WAIT_TIME_NS  (1000 * NSEC_PER_MSEC)

/* offset of the carveout relative to the starting address of the memory region */
#define HOMO_IMGLOAD_OFFSET            0
#define HOMO_VDEV_VIRING0_OFFSET       0x10200000
#define HOMO_VDEV_VIRING1_OFFSET       0x10210000
#define HOMO_VDEV_BUFFER_OFFSET        0x10220000

/* size of each carveout */
#define HOMO_IMGLOAD_SIZE              0x10200000
#define HOMO_VDEV_VIRING0_SIZE         0x00008000
#define HOMO_VDEV_VIRING1_SIZE         0x00008000
#define HOMO_VDEV_BUFFER_SIZE          0x00080000

#define HOMO_ADD_CARVEOUT(dev, base, size, name_fmt) \
    do { \
        struct rproc_mem_entry *mem = rproc_mem_entry_init( \
            dev, NULL, (dma_addr_t)(base), (size), (base), \
            homo_mem_alloc, homo_mem_release, \
            name_fmt); \
        if (!mem) { \
            dev_err(dev, "rproc_mem_entry_init err!\n"); \
            return -ENOMEM; \
        } \
        rproc_add_carveout(rproc, mem); \
    } while (0)

/* Resource table for the homo remote processors */
struct homo_resource_table {
	unsigned int version;
	unsigned int num;
	/* use bit0 of reserved[0] as stop flag */
	unsigned int reserved[2];
	unsigned int offset[RPROC_RESOURCE_ENTRIES];

	/* Note: linux kenrel 'struct fw_rsc_vdev' has no 'type' field, here add to align data structre */
	uint32_t type;
	/* rpmsg vdev entry */
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;
};

struct homo_rproc {
	struct rproc *rproc;
	struct homo_resource_table *rsc;

	u64 phys_addr;
	void *addr;
	u64 size;

	int irq;
	int cpu;
	struct work_struct vq_work;
	int mapped_irq;
};

static struct homo_rproc *g_homo_rproc[RPROC_CORE_MAX_NUM];
static int homo_rproc_num;

#define MPIDR_TO_SGI_AFFINITY(cluster_id, level)        (MPIDR_AFFINITY_LEVEL(cluster_id, level) << ICC_SGI1R_AFFINITY_## level ## _SHIFT)

static int homo_find_rproc_offset_irq(int rproc_irq)
{
	int i;

	for (i = 0; i < homo_rproc_num; i++) {
		if (g_homo_rproc[i]->mapped_irq == rproc_irq) {
			return i;
		}
	}

	return -1;
}

static bool homo_rproc_wait_cpuoff(struct rproc *rproc, ktime_t stop)
{
	int err = 0;
	struct homo_rproc *priv = rproc->priv;

	err = psci_ops.affinity_info(cpu_logical_map(priv->cpu), 0);

	return (err == 1) || ktime_after(ktime_get(), stop);
}

void gicv3_ipi_send_single(int irq, u64 mpidr)
{
	u16 tlist = 0;
	u64 cluster_id;
	u64 sgi1r;

	/* Ensure stores to Normal memory are visible to other CPUs before sending the IPI. */
	wmb();

	cluster_id = mpidr & ~0xffUL;
	tlist |= 1 << (mpidr & 0xf);

	/* Send the IPIs for the target list of this cluster */
	sgi1r = (MPIDR_TO_SGI_AFFINITY(cluster_id, 3) |
			MPIDR_TO_SGI_AFFINITY(cluster_id, 2) |
			irq << 24 |
			MPIDR_TO_SGI_AFFINITY(cluster_id, 1) | tlist);
	gic_write_sgi1r(sgi1r);

	/* Force the above writes to ICC_SGI1R_EL1 to be executed */
	isb();
}

static void homo_rproc_write_stop_flag(struct homo_resource_table *table_ptr)
{
	unsigned int *flag = table_ptr->reserved;

	*flag |= REMOTE_PROC_STOP;
}

static void homo_rproc_clear_stop_flag(struct homo_resource_table *table_ptr)
{
	unsigned int *flag = table_ptr->reserved;

	*flag &= ~REMOTE_PROC_STOP;
}

static void homo_rproc_vq_irq(struct work_struct *vq_work)
{
	struct homo_rproc *priv = container_of(vq_work, struct homo_rproc, vq_work);
	struct homo_resource_table *rsc = priv->rsc;
	struct rproc *rproc = priv->rproc;

	rproc_vq_interrupt(rproc, rsc->rpmsg_vring0.notifyid);
}

static int homo_rproc_start(struct rproc *rproc)
{
	int err;
	struct homo_rproc *priv = rproc->priv;
	int phys_cpuid = cpu_logical_map(priv->cpu);
	struct arm_smccc_res smc_res;

	err = psci_ops.affinity_info(phys_cpuid, 0);
	if (err == 0)
		remove_cpu(priv->cpu);

	INIT_WORK(&priv->vq_work, homo_rproc_vq_irq);

	priv->rsc = (struct homo_resource_table *)rproc->table_ptr;

	/* ARMv8 requires to clean D-cache and invalidate I-cache for memory containing new instructions. */
	flush_icache_range((unsigned long)priv->addr, (unsigned long)(priv->addr + HOMO_IMGLOAD_SIZE));

	arm_smccc_smc(CPU_ON, phys_cpuid, (unsigned long long)priv->phys_addr, 0, 0, 0, 0, 0, &smc_res);

	return smc_res.a0;
}

static int homo_rproc_stop(struct rproc *rproc)
{
	struct homo_rproc *priv = rproc->priv;
	ktime_t stop;

	if (!priv || !priv->rsc) {
		dev_err(&rproc->dev, "cannot find resource table!\n");
		return -1;
	}

	homo_rproc_write_stop_flag(priv->rsc);

	gicv3_ipi_send_single(priv->irq, cpu_logical_map(priv->cpu));

	stop = ktime_add_ns(ktime_get(), HOMO_MAX_WAIT_TIME_NS);

	spin_until_cond(homo_rproc_wait_cpuoff(rproc, stop));

	if (ktime_after(ktime_get(), stop)) {
		dev_err(&rproc->dev, "wait remote processor stop timeout!\n");
		return -1;
	}

	add_cpu(priv->cpu);

	homo_rproc_clear_stop_flag(priv->rsc);

	return 0;
}

static void *homo_rproc_da_to_va(struct rproc *rproc, u64 da, size_t len,
		bool *is_iomem)
{
	struct homo_rproc *priv = rproc->priv;

	return priv->addr + (da - rproc->bootaddr);
}

static void homo_rproc_kick(struct rproc *rproc, int vqid)
{
	struct homo_rproc *priv = rproc->priv;

	if (rproc->state == RPROC_RUNNING)
		gicv3_ipi_send_single(priv->irq, cpu_logical_map(priv->cpu));

	return;
}

static void __iomem *homo_ioremap_prot(phys_addr_t addr, size_t size, pgprot_t prot)
{
	unsigned long offset, vaddr;
	phys_addr_t last_addr;
	struct vm_struct *area;

	/* Disallow wrap-around or zero size */
	last_addr = addr + size - 1;
	if (!size || last_addr < addr)
		return NULL;

	/* Page-align mappings */
	offset = addr & (~PAGE_MASK);
	addr -= offset;
	size = PAGE_ALIGN(size + offset);

	area = get_vm_area_caller(size, VM_IOREMAP,
			__builtin_return_address(0));
	if (!area)
		return NULL;
	vaddr = (unsigned long)area->addr;

	if (ioremap_page_range(vaddr, vaddr + size, addr, prot)) {
		free_vm_area(area);
		return NULL;
	}

	return (void __iomem *)(vaddr + offset);
}

static irqreturn_t homo_rproc_irq_handler(int irq, void *data)
{
	int offset;
	struct homo_rproc *priv;
	int cpu = smp_processor_id();

	/* Return immediately if not the designated CPU */
	if (cpu != RPROC_IRQ_HANDLE_CPU) {
		return IRQ_HANDLED;
	}

	offset = homo_find_rproc_offset_irq(irq);
	priv = g_homo_rproc[offset];

	schedule_work(&priv->vq_work);
	return IRQ_HANDLED;
}

static int homo_rproc_starting_cpu(unsigned int cpu)
{
	int i;
	int irq;

	for (i = 0; i < homo_rproc_num; i++) {
		irq = g_homo_rproc[i]->mapped_irq;
		enable_percpu_irq(irq, irq_get_trigger_type(irq));
	}

	return 0;
}

static int homo_rproc_dying_cpu(unsigned int cpu)
{
	int i;
	int irq;

	for (i = 0; i < homo_rproc_num; i++) {
		irq = g_homo_rproc[i]->mapped_irq;
		disable_percpu_irq(irq);
	}

	return 0;
}

static int homo_mem_alloc(struct rproc *rproc, struct rproc_mem_entry *mem)
{
	struct device *dev = rproc->dev.parent;
	void *va;
	struct homo_rproc *priv = rproc->priv;
	char load_name[32];

	va = homo_ioremap_prot(mem->dma, mem->len, PAGE_KERNEL_EXEC);
	if (!va) {
		dev_err(dev, "Unable to map memory region: %pa+%zx\n",
			&mem->dma, mem->len);
		return -ENOMEM;
	}

	snprintf(load_name, sizeof(load_name), "imgload");
	if (!strcmp(mem->name, load_name)) {
		priv->addr = va;
		priv->phys_addr = mem->dma;
	}

	dev_dbg(dev, "name: %s, ioremap: phys_addr = %016llx, addr = %llx, size = %lx\n",
			mem->name, mem->dma, (u64)va, mem->len);

	/* Update memory entry va */
	mem->va = va;

	return 0;
}

static int homo_mem_release(struct rproc *rproc, struct rproc_mem_entry *mem)
{
	vunmap(mem->va);

	return 0;
}

static int homo_rproc_prepare(struct rproc *rproc)
{
	struct device *dev = rproc->dev.parent;
	struct device_node *np = dev->of_node;

	struct device_node *np_mem;
	struct resource res;
	int ret;
	struct homo_rproc *priv = rproc->priv;

	np_mem = of_parse_phandle(np, "memory-region", 0);
	ret = of_address_to_resource(np_mem, 0, &res);
	if (ret) {
		dev_err(dev, "can't find memory-region for Baremetal\n");
		return -ENOMEM;
	}

	priv->rsc = NULL;

	priv->phys_addr = res.start;
	priv->size = resource_size(&res);

	HOMO_ADD_CARVEOUT(dev, priv->phys_addr + HOMO_IMGLOAD_OFFSET, HOMO_IMGLOAD_SIZE, "imgload");
	HOMO_ADD_CARVEOUT(dev, priv->phys_addr + HOMO_VDEV_VIRING0_OFFSET, HOMO_VDEV_VIRING0_SIZE, "vdev0vring0");
	HOMO_ADD_CARVEOUT(dev, priv->phys_addr + HOMO_VDEV_VIRING1_OFFSET, HOMO_VDEV_VIRING1_SIZE, "vdev0vring1");
	HOMO_ADD_CARVEOUT(dev, priv->phys_addr + HOMO_VDEV_BUFFER_OFFSET, HOMO_VDEV_BUFFER_SIZE, "vdev0buffer");

	return 0;
}

static const struct rproc_ops homo_rproc_ops = {
	.prepare = homo_rproc_prepare,
	.start = homo_rproc_start,
	.stop = homo_rproc_stop,
	.kick = homo_rproc_kick,
	.da_to_va = homo_rproc_da_to_va,
};

static int homo_core_of_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev_of_node(dev), *p;
	struct homo_rproc *priv;
	unsigned int ipi, cpu;
	int ret;
	struct of_phandle_args oirq;
	int rproc_irq;
	struct rproc *rproc;
	const char *fw_name;

	if (!devres_open_group(dev, homo_core_of_init, GFP_KERNEL))
		return -ENOMEM;

	ret = rproc_of_parse_firmware(dev, 0, &fw_name);
	if (ret) {
		dev_err(dev, "failed to parse firmware-name property, ret = %d\n", ret);
		return ret;
	}

	rproc = rproc_alloc(dev, np->name, &homo_rproc_ops, fw_name, sizeof(*priv));
	if (!rproc)
		return -ENOMEM;

	rproc->auto_boot = false;
	rproc->has_iommu = false;

	platform_set_drvdata(pdev, rproc);

	priv = g_homo_rproc[homo_rproc_num] = rproc->priv;
	priv->rproc = rproc;
	priv->rsc = NULL;
	priv->addr = NULL;

	/* The following values can be modified through devicetree 'homo_rproc' node */
	if (of_property_read_u32(np, "remote-processor", &cpu)) {
		dev_err(dev, "not found 'remote-processor' property\n");
		ret = -EINVAL;
		goto err_free;
	}

	if (of_property_read_u32(np, "inter-processor-interrupt", &ipi)) {
		dev_err(dev, "not found 'inter-processor-interrupt' property\n");
		ret = -EINVAL;
		goto err_free;
	}

	/* The gic-v3 driver has registered the 0-7 range of SGI interrupt for system purpose */
	if (ipi < 8) {
		dev_err(dev, "'inter-processor-interrupt' is %d, should be between 9~15\n", ipi);
		ret = -EINVAL;
		goto err_free;
	}

	priv->cpu = cpu;
	priv->irq = ipi;

	dev_info(dev, "remote-processor = %d, inter-processor-interrupt = %d\n", cpu, ipi);

	/* Look for the interrupt parent. */
	p = of_irq_find_parent(np);
	if (p == NULL) {
		ret = -EINVAL;
		goto err_free;
	}

	oirq.np = p;
	oirq.args_count = 1;
	oirq.args[0] = ipi;
	rproc_irq = irq_create_of_mapping(&oirq);
	if (rproc_irq <= 0) {
		ret = -EINVAL;
		goto err_free;
	}

	priv->mapped_irq = rproc_irq;

	ret = request_percpu_irq(rproc_irq, homo_rproc_irq_handler, "homo-rproc-ipi", &cpu_number);
	if (ret) {
		dev_err(dev, "failed to request percpu irq, status = %d\n", ret);
		goto err_free;
	}

	ret = rproc_add(rproc);
	if (ret) {
		dev_err(dev, "failed to add register device with remoteproc core, status = %d\n", ret);
		goto err_free;
	}

	devres_close_group(dev, homo_core_of_init);

	return 0;

err_free:
	rproc_free(rproc);

	devres_release_group(dev, homo_core_of_init);
	return ret;
}

/*
 * release each remote processor
 */
static void homo_core_of_exit(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rproc *rproc = platform_get_drvdata(pdev);

	rproc_del(rproc);
	rproc_free(rproc);

	platform_set_drvdata(pdev, NULL);
	devres_release_group(dev, homo_core_of_init);
}

/*
 * free the resources when init fail or driver remove
 */
static void homo_cluster_of_exit(void)
{
	struct rproc *rproc;
	struct platform_device *cpdev;
	int i;

	for (i = 0; i < homo_rproc_num; i++) {
		rproc = g_homo_rproc[i]->rproc;
		cpdev = to_platform_device(rproc->dev.parent);
		homo_core_of_exit(cpdev);
	}

	homo_rproc_num = 0;
}

/*
 * Create remote processors by identifying the child nodes of homo_rproc in the dts
 */
static int homo_cluster_of_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev_of_node(dev);
	struct platform_device *cpdev;
	struct device_node *child;
	int ret;

	for_each_available_child_of_node(np, child) {
		const char *node_name = child->name;
		cpdev = of_find_device_by_node(child);
		if (!cpdev) {
			ret = -ENODEV;
			dev_err(dev, "could not get core platform device for node %s\n", node_name);
			of_node_put(child);
			goto fail;
		}

		ret = homo_core_of_init(cpdev);
		if (ret) {
			dev_err(dev, "homo_core_of_init failed, ret = %d\n", ret);
			put_device(&cpdev->dev);
			of_node_put(child);
			goto fail;
		}

		homo_rproc_num++;
		put_device(&cpdev->dev);
	}

	return 0;

fail:
	homo_cluster_of_exit();
	return ret;
}

static int homo_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;
	int num_cores;

	num_cores = of_get_available_child_count(np);

	if (num_cores > RPROC_CORE_MAX_NUM) {
		dev_err(dev, "core number (%d) out of range\n", num_cores);
		return -ENODEV;
	}

	ret = devm_of_platform_populate(dev);
	if (ret) {
		dev_err(dev, "devm_of_platform_populate failed, ret = %d\n",
			ret);
		return ret;
	}

	ret = homo_cluster_of_init(pdev);
	if (ret) {
		dev_err(dev, "homo_cluster_of_init failed, ret = %d\n", ret);
		return ret;
	}

	ret = cpuhp_setup_state(CPUHP_AP_HOMO_RPROC_STARTING, "remoteproc/homo_rproc:starting", homo_rproc_starting_cpu, homo_rproc_dying_cpu);
	if (ret) {
		dev_err(dev, "cpuhp setup state failed, status = %d\n", ret);
		return ret;
	}

	return 0;
}

static void homo_rproc_remove(struct platform_device *pdev)
{
	homo_cluster_of_exit();

}

static const struct of_device_id homo_rproc_ids[] = {
	{ .compatible = "homo,rproc", },
	{ }
};
MODULE_DEVICE_TABLE(of, homo_rproc_ids);

static struct platform_driver homo_rproc_driver = {
	.probe = homo_rproc_probe,
	.remove = homo_rproc_remove,
	.driver = {
		.name = "homo-rproc",
		.of_match_table = of_match_ptr(homo_rproc_ids),
	},
};
module_platform_driver(homo_rproc_driver);

MODULE_DESCRIPTION("Homogeneous Remote Processor Control Driver");
MODULE_AUTHOR("Shaojun Yang <yangshaojun@phytium.com.cn>");
MODULE_AUTHOR("Yuming Hu <huyuming1672@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
