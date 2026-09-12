/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ftv310 power control.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You may obtain a copy of the GNU General Public License
 * Version 2 at the following locations:
 * https://opensource.org/licenses/gpl-2.0.php
 */

#include "ftv310_vpu_power_reg.h"

int ftv310_vpu_acpi_power_request(struct device *dev, bool is_decode, bool enable)
{

	unsigned long long ret;

	acpi_handle handle = ACPI_HANDLE(dev);

	union acpi_object args[3];
		struct acpi_object_list arg_list = {
				.pointer = args,
				.count = ARRAY_SIZE(args),
	};
		acpi_status status;

	pr_info("TODO:set power:%d---\n", enable);


	args[0].type = ACPI_TYPE_INTEGER;
	if (is_decode)
		args[0].integer.value = 3;/* decode */
	else
		args[0].integer.value = 4; /* encode */
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = 0;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = 0;


	if (!has_acpi_companion(dev))
		pr_err("get acpi device failed\n");

	if (enable) {
		status = acpi_evaluate_integer(handle, "PPWO", &arg_list, &ret);
		if (ACPI_FAILURE(status)) {
			pr_err("No PS0 Method\n");
			return -1;
		}
		if (ret < 0) {
			pr_err("Failed to suspend");
			return -1;
		}
	} else {
		status = acpi_evaluate_integer(handle, "PPWD", &arg_list, &ret);
		if (ACPI_FAILURE(status)) {
			pr_err("No PS3 Method\n");
			return -1;
		}
		if (ret < 0) {
			pr_err("Failed to resume");
			return -1;
		}
	}

        pr_info("acpi power operation succeed\n");
	
	return 0;
}

static inline void
phytium_writel_reg(void __iomem *base, uint32_t reg_offset, uint32_t data)
{
	writel(data, base + reg_offset);
}

static inline unsigned int
phytium_readl_reg(void __iomem *base, uint32_t reg_offset)
{
	unsigned int data;

	data = readl(base + reg_offset);

	return data;
}

int phytium_wait_cmd_done(void __iomem *module_reg)
{
	int timeout = 500, config = 0, ret = 0;

	do {
		mdelay(1);
		timeout--;
		config = phytium_readl_reg(module_reg, AP_CPPC1_STAT);
	} while (config && timeout);

	if (timeout == 0) {
		pr_err("wait CPPC state idle timeout\n");
		ret = -EBUSY;
	} else {
		timeout = 500;
		do {
			mdelay(1);
			timeout--;
			config = phytium_readl_reg(module_reg, SE_PAYLOAD_0);
		} while (!(config & CHANNEL_STATUS_FREE) && timeout);
		if (timeout == 0) {
			pr_err("wait channel state idle timeout\n");
			ret = -EBUSY;
		}
	}
	mdelay(5);

	return ret;
}


int ftv310_vpu_dts_power_control(vcmd_slice_str *slice, bool is_decode, bool on)
{
	int ret = 0;
	int config = 0;
	void __iomem *se_reg_base = NULL;

	se_reg_base = slice->se_reg_base + (is_decode ? DECODE_REG_OFFSET : ENCODE_REG_OFFSET);
	config = on ? STATE_ON : STATE_OFF;

	ret = phytium_wait_cmd_done(se_reg_base);
	if (ret < 0)
		goto failed;

	phytium_writel_reg(se_reg_base, SE_PAYLOAD_0,
				PROTOCAL_ID | MESSAGE_ID);


	phytium_writel_reg(se_reg_base, SE_PAYLOAD_1,
				STATE_TYPE | config);

	phytium_writel_reg(se_reg_base, AP_CPPC1_SET, AP_TRIGER);

	ret = phytium_wait_cmd_done(se_reg_base);
	if (ret < 0)
		goto failed;

	ret = (phytium_readl_reg(se_reg_base, SE_PAYLOAD_0) >> PAYLOAD_STATE_SHIFT);

	return ret;

failed:
	return ret;
}

int ftv310_vpu_power_request(struct device *dev, vcmd_slice_str *slice, bool is_decode, bool on)
{
	int ret = 0;

	if (dev->of_node)
		ret = ftv310_vpu_dts_power_control(slice, is_decode, on);
	else if (has_acpi_companion(dev))
		ret = ftv310_vpu_acpi_power_request(dev, is_decode, on);

	return ret;
}

int ftv310_vpu_power_init(struct platform_device *pdev, vcmd_slice_str *slice)
{
	struct resource *res;
	int ret = 0; 

	if (pdev->dev.of_node) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		slice->se_reg_base = devm_ioremap_resource(&pdev->dev, res);
		if (slice->se_reg_base == NULL) {
			pr_err("ioremap fail, addr:0x%llx, size:0x%llx\n", res->start, res->end);
			return -ENOMEM;
		}
	}

	ret = ftv310_vpu_power_request(&pdev->dev, slice, true, true);
	if(ret) {
		pr_err("decode power control failed!\n");
		return -EBUSY;
	}

	slice->dec_power_status = 1;

	ret = ftv310_vpu_power_request(&pdev->dev, slice, false, true);
	if(ret) {
		pr_err("encode power control failed!\n");
		return -EBUSY;
	}

	slice->enc_power_status = 1;

	return 0;

}
