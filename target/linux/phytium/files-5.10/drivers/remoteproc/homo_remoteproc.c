/*
 * Homogeneous Remote Processor Control Driver
 *
 * Copyright (c) 2022-2023 Phytium Technology Co., Ltd.
 * Author: Shaojun Yang <yangshaojun@phytium.com.cn>
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

#include "remoteproc_internal.h"

#define RPROC_RESOURCE_ENTRIES      8

#define PSCI_VERSION                0x84000000
#define CPU_SUSPEND                 0xc4000001
#define CPU_OFF                     0x84000002
#define CPU_ON                      0xc4000003
#define AFFINITY_INFO               0xc4000004
#define MIGRATE                     0xc4000005

/* Resource table for the homo remote processors */
struct homo_resource_table {
	unsigned int version;
	unsigned int num;
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
};

static int homo_rproc_irq;
static struct homo_rproc *g_priv;
static struct work_struct workqueue;

#define MPIDR_TO_SGI_AFFINITY(cluster_id, level)        (MPIDR_AFFINITY_LEVEL(cluster_id, level) << ICC_SGI1R_AFFINITY_## level ## _SHIFT)

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

static void homo_rproc_vq_irq(struct work_struct *work)
{
	struct homo_rproc *priv = g_priv;
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

	INIT_WORK(&workqueue, homo_rproc_vq_irq);

	priv->rsc = (struct homo_resource_table *)rproc->table_ptr;

	/* ARMv8 requires to clean D-cache and invalidate I-cache for memory containing new instructions. */
	flush_icache_range((unsigned long)priv->addr, (unsigned long)(priv->addr + priv->size));

	arm_smccc_smc(CPU_ON, phys_cpuid, (unsigned long long)priv->phys_addr, 0, 0, 0, 0, 0, &smc_res);

	return smc_res.a0;
}

static int homo_rproc_stop(struct rproc *rproc)
{
	int err;
	struct homo_rproc *priv = rproc->priv;

	err = psci_ops.affinity_info(cpu_logical_map(priv->cpu), 0);
	if (err == 1)
		add_cpu(priv->cpu);

	return 0;
}

static void *homo_rproc_da_to_va(struct rproc *rproc, u64 da, size_t len)
{
	struct homo_rproc *priv = rproc->priv;

	return priv->addr + (da - rproc->bootaddr);
}

static void homo_rproc_kick(struct rproc *rproc, int vqid)
{
	struct homo_rproc *priv = rproc->priv;

	if (rproc->state == RPROC_RUNNING)
		gicv3_ipi_send_single(priv->irq, cpu_logical_map(priv->cpu));

	return ;
}

static const struct rproc_ops homo_rproc_ops = {
	.start = homo_rproc_start,
	.stop = homo_rproc_stop,
	.kick = homo_rproc_kick,
	.da_to_va = homo_rproc_da_to_va,
};

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
	schedule_work(&workqueue);
	return IRQ_HANDLED;
}

static int homo_rproc_starting_cpu(unsigned int cpu)
{
	enable_percpu_irq(homo_rproc_irq, irq_get_trigger_type(homo_rproc_irq));
	return 0;
}

static int homo_rproc_dying_cpu(unsigned int cpu)
{
	disable_percpu_irq(homo_rproc_irq);
	return 0;
}

static int homo_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node, *p;
	struct device_node *np_mem;
	struct resource res;
	struct rproc *rproc;
	const char *fw_name;
	struct homo_rproc *priv;
	int ret;
	unsigned int ipi, cpu;
	struct of_phandle_args oirq;

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

	priv = g_priv = rproc->priv;
	priv->rproc = rproc;

	/* The following values can be modified through devicetree 'homo_rproc' node */
	if (of_property_read_u32(np, "remote-processor", &cpu)) {
		dev_err(dev, "not found 'remote-processor' property\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "inter-processor-interrupt", &ipi)) {
		dev_err(dev, "not found 'inter-processor-interrupt' property\n");
		return -EINVAL;
	}

	/* The gic-v3 driver has registered the 0-7 range of SGI interrupt for system purpose */
	if (ipi < 8) {
		dev_err(dev, "'inter-processor-interrupt' is %d, should be between 9~15\n", ipi);
		return -EINVAL;
	}

	priv->cpu = cpu;
	priv->irq = ipi;

	dev_info(dev, "remote-processor = %d, inter-processor-interrupt = %d\n", cpu, ipi);

	np_mem = of_parse_phandle(np, "memory-region", 0);
	ret = of_address_to_resource(np_mem, 0, &res);
	if (ret) {
		dev_err(dev, "can't find memory-region for Baremetal\n");
		return ret;
	}

	priv->rsc = NULL;
	priv->addr = NULL;

	priv->phys_addr = res.start;
	priv->size = resource_size(&res);

	/* Map physical memory region reserved for homo remote processor. */
	priv->addr = homo_ioremap_prot(priv->phys_addr, priv->size, PAGE_KERNEL_EXEC);
	if (!priv->addr) {
		dev_err(dev, "ioremap failed\n");
		return -ENOMEM;
	}
	dev_info(dev, "ioremap: phys_addr = %016llx, addr = %llx, size = %lld\n",
			priv->phys_addr, (u64)(priv->addr), priv->size);

	/* Look for the interrupt parent. */
	p = of_irq_find_parent(np);
	if (p == NULL) {
		ret = -EINVAL;
		goto err;
	}

	oirq.np = p;
	oirq.args_count = 1;
	oirq.args[0] = ipi;
	homo_rproc_irq = irq_create_of_mapping(&oirq);
	if (homo_rproc_irq <= 0) {
		ret = -EINVAL;
		goto err;
	}

	ret = request_percpu_irq(homo_rproc_irq, homo_rproc_irq_handler, "homo-rproc-ipi", &cpu_number);
	if (ret) {
		dev_err(dev, "failed to request percpu irq, status = %d\n", ret);
		goto err;
	}

	ret = cpuhp_setup_state(CPUHP_AP_HOMO_RPROC_STARTING, "remoteproc/homo_rproc:starting", homo_rproc_starting_cpu, homo_rproc_dying_cpu);
	if (ret) {
		dev_err(dev, "cpuhp setup state failed, status = %d\n", ret);
		goto err;
	}

	ret = rproc_add(rproc);
	if (ret) {
		dev_err(dev, "failed to add register device with remoteproc core, status = %d\n", ret);
		goto err;
	}

	return 0;

err:
	vunmap((void *)((unsigned long)priv->addr & PAGE_MASK));
	return ret;
}

static int homo_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);

	rproc_del(rproc);
	rproc_free(rproc);

	return 0;
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
MODULE_LICENSE("GPL v2");
