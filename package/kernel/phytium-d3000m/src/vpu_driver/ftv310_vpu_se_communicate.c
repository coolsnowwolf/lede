/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 power control by dts.
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

#include "ftv310_vpu_se_reg.h"

static inline void
phytium_se_write_reg(void __iomem *se_regs, uint32_t reg, uint32_t data)
{	
	writel(data, se_regs + reg);
}

static inline uint32_t
phytium_se_read_reg(void __iomem *se_regs, uint32_t reg)
{
	uint32_t data = 0;

	data = readl(se_regs + reg);

	return data;
}


static uint32_t phytium_se_read_reg_wait(void __iomem *se_regs, uint32_t reg, uint32_t expect)
{
	uint32_t read_count = 0;
	uint32_t ret = 0;

	while(1) {
		if((phytium_se_read_reg(se_regs, reg) & 0x01) == expect) {
			read_count = 0;
			ret = 0;
			break;
		}

		if(read_count >= 5){
			pr_err("se error read reg 0x80 over 5 times\n");
			ret = -1;
			break;
		}
		read_count++;
		msleep(50);
	}

	return ret;
}

static uint32_t phytium_write_se(void __iomem *se_regs, uint32_t value_0, uint32_t value_1)
{	
	int ret = 0;
	
	ret = phytium_se_read_reg_wait(se_regs, SE_AP_STAT, 0x00);
	if(ret)
		goto err_read_se_reg;

	ret = phytium_se_read_reg_wait(se_regs, SE_PAYLOAD_0, 0x01);
	if(ret)
		goto err_read_se_reg;

	phytium_se_write_reg(se_regs, SE_PAYLOAD_0, value_0);
	phytium_se_write_reg(se_regs, SE_PAYLOAD_1, value_1);

/*ring doorbell*/
	phytium_se_write_reg(se_regs, SE_AP_SET, 0x01);

	/*wait se config finish*/
	ret = phytium_se_read_reg_wait(se_regs, SE_PAYLOAD_0, 0x01);

	return (phytium_se_read_reg(se_regs, SE_PAYLOAD_0) >> 24);

err_read_se_reg:
	pr_err("ftv310_vpu_write_se failed\n");
	return ret;
}

void ftv310_vpu_power_request_se(vcmd_slice_str *slice, bool is_decode, bool enable)
{
	int ret = 0;

	uint32_t payload_0 = 0;
	uint32_t value_0 = 0;
	uint32_t protocol_id = VPU_POWER_PROTOCAL_ID;
	uint32_t message_id = VPU_POWER_MESSAGE_ID;
	uint32_t value_1 = VPU_POWER_PAYLOAD1_RESERVEE;
	uint32_t domain_id = 0;
	void __iomem *se_regs = NULL;

	value_0 = ((payload_0 << 24) | (protocol_id << 16) | (message_id << 8) | (domain_id << 1));

	value_1 |= (enable ? VPU_POWER_STATE_ENABLE : VPU_POWER_STATE_DISABLE);

	se_regs = (is_decode ? slice->se_reg_base + DECODE_SE_REG_OFFSET : slice->se_reg_base + ENCODE_SE_REG_OFFSET);
	
	ret = phytium_write_se(se_regs, value_0, value_1);
	if (ret) {
		pr_err("vpu %s se config failed result is %d\n", __func__, ret);
	}

	return;
} 




