/* Copyright (c) 2009-2014, 2016, 2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ARCH_ARM_MACH_MSM_SOCINFO_H_
#define _ARCH_ARM_MACH_MSM_SOCINFO_H_

#include <linux/of.h>

#define CPU_IPQ8074 323
#define CPU_IPQ8072 342
#define CPU_IPQ8076 343
#define CPU_IPQ8078 344
#define CPU_IPQ8070 375
#define CPU_IPQ8071 376

#define CPU_IPQ8072A 389
#define CPU_IPQ8074A 390
#define CPU_IPQ8076A 391
#define CPU_IPQ8078A 392
#define CPU_IPQ8070A 395
#define CPU_IPQ8071A 396

#define CPU_IPQ8172  397
#define CPU_IPQ8173  398
#define CPU_IPQ8174  399

#define CPU_IPQ6018 402
#define CPU_IPQ6028 403
#define CPU_IPQ6000 421
#define CPU_IPQ6010 422
#define CPU_IPQ6005 453

#define CPU_IPQ5010 446
#define CPU_IPQ5018 447
#define CPU_IPQ5028 448
#define CPU_IPQ5000 503
#define CPU_IPQ0509 504
#define CPU_IPQ0518 505

#define CPU_IPQ9514 510
#define CPU_IPQ9554 512
#define CPU_IPQ9570 513
#define CPU_IPQ9574 514
#define CPU_IPQ9550 511
#define CPU_IPQ9510 521

static inline int read_ipq_soc_version_major(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "soc_version_major",
				NULL);

	if (!prop)
		return -EINVAL;

	return le32_to_cpu(*prop);
}

static inline int read_ipq_cpu_type(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "cpu_type", NULL);
	/*
	 * Return Default CPU type if "cpu_type" property is not found in DTSI
	 */
	if (!prop)
		return CPU_IPQ8074;

	return le32_to_cpu(*prop);
}

static inline int cpu_is_ipq8070(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8070;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8071(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8071;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8072(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8072;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8074(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8074;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8076(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8076;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8078(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8078;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8072a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8072A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8074a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8074A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8076a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8076A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8078a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8078A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8070a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8070A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8071a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8071A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8172(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8172;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8173(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8173;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8174(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8174;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq6018(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ6018;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq6028(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ6028;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq6000(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ6000;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq6010(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ6010;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq6005(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ6005;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5010(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5010;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5018(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5018;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5028(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5028;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5000(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5000;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq0509(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ0509;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq0518(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ0518;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9514(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9514;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9554(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9554;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9570(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9570;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9574(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9574;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9550(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9550;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9510(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9510;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq807x(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq8072() || cpu_is_ipq8074() ||
		cpu_is_ipq8076() || cpu_is_ipq8078() ||
		cpu_is_ipq8070() || cpu_is_ipq8071() ||
		cpu_is_ipq8072a() || cpu_is_ipq8074a() ||
		cpu_is_ipq8076a() || cpu_is_ipq8078a() ||
		cpu_is_ipq8070a() || cpu_is_ipq8071a() ||
		cpu_is_ipq8172() || cpu_is_ipq8173() ||
		cpu_is_ipq8174();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq60xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq6018() || cpu_is_ipq6028() ||
		cpu_is_ipq6000() || cpu_is_ipq6010() ||
		cpu_is_ipq6005();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq50xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq5010() || cpu_is_ipq5018() ||
		cpu_is_ipq5028() || cpu_is_ipq5000() ||
		cpu_is_ipq0509() || cpu_is_ipq0518();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq95xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq9514() || cpu_is_ipq9554() ||
		cpu_is_ipq9570() || cpu_is_ipq9574() ||
		cpu_is_ipq9550() || cpu_is_ipq9510();
#else
	return 0;
#endif
}

static inline int cpu_is_nss_crypto_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq807x() || cpu_is_ipq60xx() ||
		cpu_is_ipq50xx() || cpu_is_ipq9570() ||
		cpu_is_ipq9550() || cpu_is_ipq9574() ||
		cpu_is_ipq9554();
#else
	return 0;
#endif
}

static inline int cpu_is_internal_wifi_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq807x() || cpu_is_ipq60xx() ||
		cpu_is_ipq50xx() || cpu_is_ipq9514() ||
		cpu_is_ipq9554() || cpu_is_ipq9574();
#else
	return 0;
#endif
}

static inline int cpu_is_uniphy1_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq807x() || cpu_is_ipq60xx() ||
		cpu_is_ipq9554() || cpu_is_ipq9570() ||
		cpu_is_ipq9574() || cpu_is_ipq9550();
#else
	return 0;
#endif
}

static inline int cpu_is_uniphy2_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq807x() || cpu_is_ipq9570() ||
		cpu_is_ipq9574();
#else
	return 0;
#endif
}

#endif /* _ARCH_ARM_MACH_MSM_SOCINFO_H_ */
