// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */
#include "phytium_se_communicate.h"

static inline void
phytium_se_write_reg(struct ftd330_drm_private *priv, uint32_t reg, uint32_t data)
{
	writel(data, priv->se_regs + reg);
}

static inline uint32_t
phytium_se_read_reg(struct ftd330_drm_private *priv, uint32_t reg)
{
	uint32_t data = 0;

	data = readl(priv->se_regs + reg);

	return data;
}


static uint32_t phytium_se_read_reg_wait(struct ftd330_drm_private *priv, uint32_t reg, uint32_t expect)
{
	uint32_t read_count = 0;
	uint32_t ret = 0;

	while (1) {
		if ((phytium_se_read_reg(priv, reg) & 0x01) == expect) {
			read_count = 0;
			ret = 0;
			break;
		}

		if (read_count >= 5) {
			pr_info("se error read reg 0x80 over 5 times\n");
			ret = -1;
			break;
		}
		read_count++;
		msleep(50);
	}

	return ret;
}

static uint32_t phytium_write_se(struct ftd330_drm_private *priv, uint32_t value_0, uint32_t value_1)
{
	int ret = 0;

	ret = phytium_se_read_reg_wait(priv, SE_AP_STAT, 0x00);
	if (ret)
		goto err_read_se_reg;

	ret = phytium_se_read_reg_wait(priv, SE_PAYLOAD_0, 0x01);
	if (ret)
		goto err_read_se_reg;

	phytium_se_write_reg(priv, SE_PAYLOAD_0, value_0);
	phytium_se_write_reg(priv, SE_PAYLOAD_1, value_1);

	/*ring doorbell*/
	phytium_se_write_reg(priv, SE_AP_SET, 0x01);

	/*wait se config finish*/
	ret = phytium_se_read_reg_wait(priv, SE_PAYLOAD_0, 0x01);

	return (phytium_se_read_reg(priv, SE_PAYLOAD_0) >> 24);

err_read_se_reg:
	pr_info("phytium_dplp_write_se failed\n");
	return ret;
}

void phytium_display_power_request_se(struct ftd330_drm_private *priv, bool enable, uint32_t display_id)
{
	int ret = 0;

	uint32_t payload_0 = 0;
	uint32_t value_0 = 0;
	uint32_t protocol_id = DCDP_POWER_PROTOCAL_ID;
	uint32_t message_id = DCDP_POWER_MESSAGE_ID;
	uint32_t value_1 = DCDP_POWER_PAYLOAD1_RESERVEE;
	uint32_t domain_id = display_id;

	value_0 = ((payload_0 << 24) | (protocol_id << 16) | (message_id << 8) | (domain_id << 1));

	value_1 |= (enable ? DCDP_POWER_STATE_ENABLE : DCDP_POWER_STATE_DISABLE);

	ret = phytium_write_se(priv, value_0, value_1);
	if (ret)
		pr_info("PHY %s se config failed result is %d\n", __func__, ret);

}

void phytium_change_pxlclk_se(struct ftd330_drm_private *priv, uint32_t display_id, uint32_t pixel_clk)
{
	uint32_t payload_0 = 0;
	uint32_t value_0 = 0;
	uint32_t protocol_id = DCDP_CHANGE_PXL_CLK_PROTOCAL_ID;
	uint32_t message_id = DCDP_CHANGE_PXL_CLK_MESSAGE_ID;
	uint32_t domain_id = phytium_display_virtual_to_physical(priv->info.pipe_mask, display_id);
	int ret = 0;

	value_0 = ((payload_0 << 24) | (protocol_id << 16) | (message_id << 8) | (domain_id << 1));

	ret = phytium_write_se(priv, value_0, pixel_clk);
	if (ret)
		pr_info("%s se config failed result is %d\n", __func__, ret);

}

#ifdef CONFIG_PHYTIUM_LOW_FPS
void phytium_change_edp_lowfps_pxlclk_se(struct ftd330_drm_private *priv,
											uint32_t physical_display_id, uint32_t pixel_clk)
{
	uint32_t payload_0 = 0;
	uint32_t value_0 = 0;
	uint32_t protocol_id = DCDP_CHANGE_PXL_CLK_PROTOCAL_ID;
	uint32_t message_id = DCDP_CHANGE_PXL_CLK_MESSAGE_ID;
	int ret = 0;

	value_0 = ((payload_0 << 24) | (protocol_id << 16) | (message_id << 8) | (physical_display_id << 1));

	ret = phytium_write_se(priv, value_0, pixel_clk);
	if (ret)
		pr_info("%s se config failed result is %d\n", __func__, ret);

}
#endif