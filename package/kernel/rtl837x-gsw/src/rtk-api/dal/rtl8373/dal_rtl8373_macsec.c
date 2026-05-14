/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in LUT module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_macsec.h>
#include <dal/rtl8373/dal_rtl8373_macsec_ip_reg.h>
#include <dal/rtl8373/dal_rtl8373_drv.h>
#include <rtl8373_asicdrv.h>
#include <string.h>

/* Function Name:
 *      dal_rtl8373_macsec_enable_set
 * Description:
 *      Configure macsec enable.
 * Input:
 *      port   - port id
 *      ingress_en  - ingress enable
 *      egress_en  -  egress enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec enable.
 */
rtk_api_ret_t dal_rtl8373_macsec_enable_set(rtk_uint32 port, rtk_uint32 ingress_en, rtk_uint32 egress_en)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }

    else if(port == 4)
       {
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
       }
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_enable_get
 * Description:
 *      get macsec enable status.
 * Input:
 *      port   - port id
 * Output:
 *      ingress_en  - ingress enable
 *      egress_en  -  egress enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec enable status.
 */
rtk_api_ret_t dal_rtl8373_macsec_enable_get(rtk_uint32 port, rtk_uint32 *ingress_en, rtk_uint32 *egress_en)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 4)
       {
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT4_ADDR, RTL8373_MACSEC_PM_CTRL_PORT4_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
       }
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT5_ADDR, RTL8373_MACSEC_PM_CTRL_PORT5_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT6_ADDR, RTL8373_MACSEC_PM_CTRL_PORT6_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_TX_ICG_EN_OFFSET, egress_en)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_PM_CTRL_PORT7_ADDR, RTL8373_MACSEC_PM_CTRL_PORT7_MACSEC_RX_ICG_EN_OFFSET, ingress_en)) != RT_ERR_OK)
        		return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_reset
 * Description:
 *      reset macsec ip.
 * Input:
 *      port   - port id
 *      ingress_rst  - ingress reset
 *      egress_rst  -  egress reset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will reset macsec.
 */
rtk_api_ret_t dal_rtl8373_macsec_reset(rtk_uint32 port, rtk_uint32 ingress_rst, rtk_uint32 egress_rst)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
                return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
                return retVal;
    }
    else if(port == 4)
       {
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
        		return retVal;
       }
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
        		return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPRST_N_OFFSET, egress_rst)) != RT_ERR_OK)
        		return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPRST_N_OFFSET, ingress_rst)) != RT_ERR_OK)
        		return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_macsec_egress_set
 * Description:
 *      Configure macsec egress rule.
 * Input:
 *      port  - port id
 *      addr - macsec ip core register address
 *      value - data for rule
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec egress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t dal_rtl8373_macsec_egress_set(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
	rtk_api_ret_t retVal;
	rtk_uint32 phy_data;

	 if(port == 0)
    {
#if 1 //pending
        if ((retVal = dal_rtl8373_phy_read(0, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT4_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT4_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT4_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
#if 1 //pending    
        if ((retVal = dal_rtl8373_phy_read(1, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT5_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT5_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT5_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8373_phy_read(2, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT6_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT6_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT6_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8373_phy_read(3, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT7_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT7_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT7_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
	else if(port == 4)
       {
       	if ((retVal = dal_rtl8373_phy_read(4, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT4_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT4_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT4_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
       }
	else if(port == 5)
	{
		if ((retVal = dal_rtl8373_phy_read(5, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT5_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT5_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT5_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = dal_rtl8373_phy_read(6, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT6_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT6_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT6_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = dal_rtl8373_phy_read(7, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT7_REG_DATA_AE_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, RTL8373_MACSEC_REG_RWDH_AE_PORT7_REG_DATA_AE_H_MASK, (value >> 16) & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT7_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_WR_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_egress_get
 * Description:
 *      get macsec egress rule.
 * Input:
 *      port  -  port id
 *      addr -  macsec ip core register address
 * Output:
 *      value  - data for rule
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec egress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t dal_rtl8373_macsec_egress_get(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 *value)
{
	rtk_api_ret_t retVal;
	rtk_uint32 phy_data,tmp;

	if(port == 0)
    {
#if 1 //pending

           if ((retVal = dal_rtl8373_phy_read(0, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif  

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT4_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, &tmp)) != RT_ERR_OK)
                    return retVal;
        //rtlglue_printf("4\n");
        *value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
    }
    else if(port == 1)
    {
#if 1 //pending

        if ((retVal = dal_rtl8373_phy_read(1, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT5_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;
        
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, &tmp)) != RT_ERR_OK)
                    return retVal;

        *value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8373_phy_read(2, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT6_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;
        
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, &tmp)) != RT_ERR_OK)
                    return retVal;

        *value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8373_phy_read(3, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AE_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT7_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, &tmp)) != RT_ERR_OK)
                    return retVal;

        *value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
    }
	else if(port == 4)
       {
       	if ((retVal = dal_rtl8373_phy_read(4, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT4_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_RWDH_AE_PORT4_ADDR, &tmp)) != RT_ERR_OK)
        			return retVal;
		
		*value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
       }
	else if(port == 5)
	{
		if ((retVal = dal_rtl8373_phy_read(5, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT5_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;
		
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_RWDH_AE_PORT5_ADDR, &tmp)) != RT_ERR_OK)
        			return retVal;

		*value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
	}
	else if(port == 6)
	{
		if ((retVal = dal_rtl8373_phy_read(6, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT6_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;
		
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_RWDH_AE_PORT6_ADDR, &tmp)) != RT_ERR_OK)
        			return retVal;

		*value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
	}
	else if(port == 7)
	{
		if ((retVal = dal_rtl8373_phy_read(7, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AE_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AE_PORT7_REG_ADDR_AE_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_RD_REQ_AE_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_RWDH_AE_PORT7_ADDR, &tmp)) != RT_ERR_OK)
        			return retVal;

		*value = ((tmp >> 16) & 0xffff) | ((tmp & 0xffff) << 16);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ingress_set
 * Description:
 *      Configure macsec ingress rule.
 * Input:
 *      port  - port id
 *      addr - macsec ip core register address
 *      value - data for rule
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec ingress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t dal_rtl8373_macsec_ingress_set(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
	rtk_api_ret_t retVal;
	rtk_uint32 phy_data;

	if(port == 0)
    {
#if 1 //pending

        if ((retVal = dal_rtl8373_phy_read(0, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDL_AI_PORT4_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT4_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
#if 1 //pending

        if ((retVal = dal_rtl8373_phy_read(1, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif 
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDL_AI_PORT5_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT5_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8373_phy_read(2, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDL_AI_PORT6_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT6_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8373_phy_read(3, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_RWDL_AI_PORT7_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT7_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
    }
	else if(port == 4)
    {
       	if ((retVal = dal_rtl8373_phy_read(4, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT4_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT4_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
    }
	else if(port == 5)
	{
		if ((retVal = dal_rtl8373_phy_read(5, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT5_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT5_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = dal_rtl8373_phy_read(6, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT6_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT6_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = dal_rtl8373_phy_read(7, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_DATA_AI_H_MASK, (value >> 16) &0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT7_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT7_REG_DATA_AI_L_MASK, value & 0xffff)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_WR_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ingress_get
 * Description:
 *      get macsec egress rule.
 * Input:
 *      port  -  port id
 *      addr -  macsec ip core register address
 * Output:
 *      value  - data for rule
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec ingress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t dal_rtl8373_macsec_ingress_get(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 *value)
{
	rtk_api_ret_t retVal;
       rtk_uint32 phy_data;
	rtk_uint32 value_H,value_L;

	if(port == 0)
    {
#if 1 //pending

        if ((retVal = dal_rtl8373_phy_read(0, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_RWDL_AI_PORT4_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT4_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
                    return retVal;

        *value = (value_H << 16) | (value_L & 0xffff);

    }
    else if(port == 1)
    {
#if 1 //pending

        if ((retVal = dal_rtl8373_phy_read(1, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
#endif
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_RWDL_AI_PORT5_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT5_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
                    return retVal;

        *value = (value_H << 16) | (value_L & 0xffff);
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8373_phy_read(2, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_RWDL_AI_PORT6_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT6_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
                    return retVal;

        *value = (value_H << 16) | (value_L & 0xffff);
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8373_phy_read(3, 7, 1, &phy_data)) != RT_ERR_OK)
                      return retVal;

        if(((phy_data >> 2) & 0x1) != 1)
                return RT_ERR_PHY_LINK_DOWN;
        
        if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
                    return retVal;

        if ((retVal = dal_rtl8224_top_regbits_read(RTL8373_MACSEC_REG_RWDL_AI_PORT7_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT7_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
                    return retVal;

        *value = (value_H << 16) | (value_L & 0xffff);
    }
	else if(port == 4)
       {
       	if ((retVal = dal_rtl8373_phy_read(4, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT4_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT4_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT4_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT4_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT4_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT4_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
        			return retVal;

		*value = (value_H << 16) | (value_L & 0xffff);

       }
	else if(port == 5)
	{
		if ((retVal = dal_rtl8373_phy_read(5, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT5_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT5_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT5_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT5_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT5_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT5_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
        			return retVal;

		*value = (value_H << 16) | (value_L & 0xffff);
	}
	else if(port == 6)
	{
		if ((retVal = dal_rtl8373_phy_read(6, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT6_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT6_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT6_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT6_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT6_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT6_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
        			return retVal;

		*value = (value_H << 16) | (value_L & 0xffff);
	}
	else if(port == 7)
	{
		if ((retVal = dal_rtl8373_phy_read(7, 7, 1, &phy_data)) != RT_ERR_OK)
		      		return retVal;

		if(((phy_data >> 2) & 0x1) != 1)
				return RT_ERR_PHY_LINK_DOWN;
		
		if ((retVal = rtl8373_setAsicRegBits(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_ADDR_AI_MASK, addr)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_ADDR_AI_PORT7_ADDR, RTL8373_MACSEC_REG_ADDR_AI_PORT7_REG_RD_REQ_AI_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_CMD_AE_PORT7_ADDR, RTL8373_MACSEC_REG_CMD_AE_PORT7_REG_DATA_AI_H_MASK, &value_H)) != RT_ERR_OK)
        			return retVal;

		if ((retVal = rtl8373_getAsicRegBits(RTL8373_MACSEC_REG_RWDL_AI_PORT7_ADDR, RTL8373_MACSEC_REG_RWDL_AI_PORT7_REG_DATA_AI_L_MASK, &value_L)) != RT_ERR_OK)
        			return retVal;

		*value = (value_H << 16) | (value_L & 0xffff);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxgating_set
 * Description:
 *      Configure macsec rx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec rx gating value,before set the packet flow path called this API.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxgating_set(rtk_uint32 port)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value,tmp_gvalue;
	rtk_uint32 pollcnt;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < 0xffff; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }
        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
	else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxgating_cancel
 * Description:
 *      Configure macsec rx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec rx gating value, called after packet flow path changed.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxgating_cancel(rtk_uint32 port)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value,tmp_gvalue;
	rtk_uint32 pollcnt;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;

                if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
	else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_RXDV_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;

				if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txgating_set
 * Description:
 *      Configure macsec tx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec tx gating value, before set the packet flow path called this API.
 */
rtk_api_ret_t dal_rtl8373_macsec_txgating_set(rtk_uint32 port)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value,tmp_gvalue;
	rtk_uint32 pollcnt;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }
        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 1) && (tmp_gvalue == 1))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
	else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
		
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
		
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
		
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_GATING_OFFSET, 1)) != RT_ERR_OK)
        			return retVal;
		
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 1) && (tmp_gvalue == 1))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txgating_cancel
 * Description:
 *      Configure macsec tx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec tx gating value, called after packet flow path changed..
 */
rtk_api_ret_t dal_rtl8373_macsec_txgating_cancel(rtk_uint32 port)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value,tmp_gvalue;
	rtk_uint32 pollcnt;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
        
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
                        return retVal;

                if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
                        return retVal;
                
                       if((tmp_value == 0) && (tmp_gvalue == 0))
                    break;
              }

        if(pollcnt == RTL8373_MACSEC_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
	else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
		
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT4_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT5_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT6_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_GATING_OFFSET, 0)) != RT_ERR_OK)
        			return retVal;
              
              for(pollcnt = 0; pollcnt < RTL8373_MACSEC_POLLCNT; pollcnt++)
              {
				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TX_XGMASK_OFFSET, &tmp_value)) != RT_ERR_OK)
        				return retVal;

				if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_MASK_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_MASK_PORT7_TXEN_GMASK_OFFSET, &tmp_gvalue)) != RT_ERR_OK)
        				return retVal;
				
               		if((tmp_value == 0) && (tmp_gvalue == 0))
					break;
              }

		if(pollcnt == RTL8373_MACSEC_POLLCNT)
			return RT_ERR_BUSYWAIT_TIMEOUT;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxIPbypass_set
 * Description:
 *      Configure macsec bypass in MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass in MACsec IP function.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxIPbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 1)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 2)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 3)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 4)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 5)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 6)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 7)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxIPbypass_get
 * Description:
 *      get macsec bypass in MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass in MACsec IP function status.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxIPbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txIPbypass_set
 * Description:
 *      Configure macsec bypass in MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass in MACsec IP function.
 */
rtk_api_ret_t dal_rtl8373_macsec_txIPbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 1)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 2)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 3)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 4)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 5)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 6)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 7)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txIPbypass_get
 * Description:
 *      get macsec bypass MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass MACsec IP function status.
 */
rtk_api_ret_t dal_rtl8373_macsec_txIPbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_IPBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxIPbypass_set
 * Description:
 *      Configure macsec bypass MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass MACsec IP function.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 1)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 2)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 3)
    {
        dal_rtl8373_macsec_rxgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_rxgating_cancel(port);
    }
    else if(port == 4)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 5)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 6)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else if(port == 7)
	{
		dal_rtl8373_macsec_rxgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_rxgating_cancel(port);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_rxbypass_get
 * Description:
 *      get macsec bypass MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass  MACsec IP function status.
 */
rtk_api_ret_t dal_rtl8373_macsec_rxbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txbypass_set
 * Description:
 *      Configure macsec bypass MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass MACsec IP function.
 */
rtk_api_ret_t dal_rtl8373_macsec_txbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 1)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 2)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 3)
    {
        dal_rtl8373_macsec_txgating_set(port);
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
        dal_rtl8373_macsec_txgating_cancel(port);
    }
    else if(port == 4)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 5)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 6)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else if(port == 7)
	{
		dal_rtl8373_macsec_txgating_set(port);
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
		dal_rtl8373_macsec_txgating_cancel(port);
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_txbypass_get
 * Description:
 *      get macsec bypass MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass MACsec IP function status.
 */
rtk_api_ret_t dal_rtl8373_macsec_txbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_MACSECBYPASS_EN_OFFSET, enable)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_int_control_set
 * Description:
 *      Configure MACsec interrupt.
 * Input:
 *      port  -  port id
 *      type  -  interrupt type
 *      enable - enable interrupt
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure MACsec interrupt.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB
 *      - INT_TYPE_TX_IPESECFAIL
 *      - INT_TYPE_TX_IPELOCK
 *      - INT_TYPE_TX_IPELOCK_XG
 *      - INT_TYPE_RX_IPI_GLB
 *      - INT_TYPE_RX_IPISECFAIL
 *      - INT_TYPE_RX_IPILOCK
 *      - INT_TYPE_RX_IPILOCK_XG
 */
rtk_api_ret_t dal_rtl8373_wrapper_int_control_set(rtk_uint32 port, rtk_macsec_int_type_t type, rtk_enable_t enable)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(enable == ENABLED)
        {
            tmp_value |=(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }
        else if(enable == DISABLED)
        {
            tmp_value &= ~(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }         
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(enable == ENABLED)
        {
            tmp_value |=(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }
        else if(enable == DISABLED)
        {
            tmp_value &= ~(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }         
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(enable == ENABLED)
        {
            tmp_value |=(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }
        else if(enable == DISABLED)
        {
            tmp_value &= ~(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }         
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(enable == ENABLED)
        {
            tmp_value |=(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }
        else if(enable == DISABLED)
        {
            tmp_value &= ~(1 << type);
            
            if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  tmp_value)) != RT_ERR_OK)
                    return retVal;
        }         
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(enable == ENABLED)
		{
			tmp_value |=(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}
		else if(enable == DISABLED)
		{
			tmp_value &= ~(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}		 
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(enable == ENABLED)
		{
			tmp_value |=(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}
		else if(enable == DISABLED)
		{
			tmp_value &= ~(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}		 
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(enable == ENABLED)
		{
			tmp_value |=(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}
		else if(enable == DISABLED)
		{
			tmp_value &= ~(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}		 
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(enable == ENABLED)
		{
			tmp_value |=(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}
		else if(enable == DISABLED)
		{
			tmp_value &= ~(1 << type);
			
			if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  tmp_value)) != RT_ERR_OK)
        			return retVal;
		}		 
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_int_control_get
 * Description:
 *      gonfigure MACsec interrupt.
 * Input:
 *      port  -  port id
 *      type  -  interrupt type
 * Output:
 *      pEnable - enable interrupt
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get MACsec interrupt.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB
 *      - INT_TYPE_TX_IPESECFAIL
 *      - INT_TYPE_TX_IPELOCK
 *      - INT_TYPE_TX_IPELOCK_XG
 *      - INT_TYPE_RX_IPI_GLB
 *      - INT_TYPE_RX_IPISECFAIL
 *      - INT_TYPE_RX_IPILOCK
 *      - INT_TYPE_RX_IPILOCK_XG
 */
rtk_api_ret_t dal_rtl8373_wrapper_int_control_get(rtk_uint32 port, rtk_macsec_int_type_t type, rtk_enable_t *pEnable)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(0 == (tmp_value & (1 << type)))
        {
            *pEnable = DISABLED;
        }
        else if(1 == (tmp_value & (1 << type)))
        {
            *pEnable = ENABLED;
        }         
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(0 == (tmp_value & (1 << type)))
        {
            *pEnable = DISABLED;
        }
        else if(1 == (tmp_value & (1 << type)))
        {
            *pEnable = ENABLED;
        }     
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(0 == (tmp_value & (1 << type)))
        {
            *pEnable = DISABLED;
        }
        else if(1 == (tmp_value & (1 << type)))
        {
            *pEnable = ENABLED;
        }         
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;

        if(0 == (tmp_value & (1 << type)))
        {
            *pEnable = DISABLED;
        }
        else if(1 == (tmp_value & (1 << type)))
        {
            *pEnable = ENABLED;
        }         
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(0 == (tmp_value & (1 << type)))
		{
			*pEnable = DISABLED;
		}
		else if(1 == (tmp_value & (1 << type)))
		{
			*pEnable = ENABLED;
		}		 
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(0 == (tmp_value & (1 << type)))
		{
			*pEnable = DISABLED;
		}
		else if(1 == (tmp_value & (1 << type)))
		{
			*pEnable = ENABLED;
		}	 
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(0 == (tmp_value & (1 << type)))
		{
			*pEnable = DISABLED;
		}
		else if(1 == (tmp_value & (1 << type)))
		{
			*pEnable = ENABLED;
		}		 
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_IMR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;

		if(0 == (tmp_value & (1 << type)))
		{
			*pEnable = DISABLED;
		}
		else if(1 == (tmp_value & (1 << type)))
		{
			*pEnable = ENABLED;
		}		 
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_int_status_set
 * Description:
 *      Configure MACsec interrupt status.
 * Input:
 *      port  -  port id
 *      statusMask  -  interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will clean MACsec interrupt status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB [bit[0]]
 *      - INT_TYPE_TX_IPESECFAIL [bit[1]]
 *      - INT_TYPE_TX_IPELOCK [bit[2]]
 *      - INT_TYPE_TX_IPELOCK_XG [bit[3]]
 *      - INT_TYPE_RX_IPI_GLB [bit[8]]
 *      - INT_TYPE_RX_IPISECFAIL [bit[9]]
 *      - INT_TYPE_RX_IPILOCK [bit[10]]
 *      - INT_TYPE_RX_IPILOCK_XG [bit[11]]
 */
rtk_api_ret_t dal_rtl8373_wrapper_int_status_set(rtk_uint32 port, rtk_uint32 statusMask)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_ISR_PORT4_ADDR,  statusMask)) != RT_ERR_OK)
                    return retVal;        
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_ISR_PORT5_ADDR,  statusMask)) != RT_ERR_OK)
                    return retVal;     
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_ISR_PORT6_ADDR,  statusMask)) != RT_ERR_OK)
                    return retVal;     
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_reg_write(RTL8373_MACSEC_REG_GLB_ISR_PORT7_ADDR,  statusMask)) != RT_ERR_OK)
                    return retVal;         
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT4_ADDR,  statusMask)) != RT_ERR_OK)
        			return retVal;		
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT5_ADDR,  statusMask)) != RT_ERR_OK)
        			return retVal;	 
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT6_ADDR,  statusMask)) != RT_ERR_OK)
        			return retVal;	 
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT7_ADDR,  statusMask)) != RT_ERR_OK)
        			return retVal;		 
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_int_status_get
 * Description:
 *      Configure MACsec interrupt status.
 * Input:
 *      port  -  port id
 
 * Output:
 *      pStatusMask  -  interrupt status mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will clean MACsec interrupt status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB [bit[0]]
 *      - INT_TYPE_TX_IPESECFAIL [bit[1]]
 *      - INT_TYPE_TX_IPELOCK [bit[2]]
 *      - INT_TYPE_TX_IPELOCK_XG [bit[3]]
 *      - INT_TYPE_RX_IPI_GLB [bit[8]]
 *      - INT_TYPE_RX_IPISECFAIL [bit[9]]
 *      - INT_TYPE_RX_IPILOCK [bit[10]]
 *      - INT_TYPE_RX_IPILOCK_XG [bit[11]]
 */
rtk_api_ret_t dal_rtl8373_wrapper_int_status_get(rtk_uint32 port, rtk_uint32 *pStatusMask)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_ISR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;        
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_ISR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;     
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_ISR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;     
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_REG_GLB_ISR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
                    return retVal;         
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT4_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;		
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT5_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;	 
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT6_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;	 
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_REG_GLB_ISR_PORT7_ADDR,  &tmp_value)) != RT_ERR_OK)
        			return retVal;		 
	}
	else
		return RT_ERR_INPUT;

	*pStatusMask = tmp_value & 0xf0f;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_mib_reset
 * Description:
 *      Configure wrapper mib reset.
 * Input:
 *      port  - port id
 *      reset  -  reset value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure wrapper mib reset.
 */
rtk_api_ret_t dal_rtl8373_wrapper_mib_reset(rtk_uint32 port, rtk_uint32 reset)
{
	rtk_api_ret_t retVal;

	if(port == 0)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT4_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 1)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT5_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 2)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT6_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 3)
    {
        if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT7_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
                    return retVal;
    }
    else if(port == 4)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT4_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 5)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT5_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 6)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT6_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
        			return retVal;
	}
	else if(port == 7)
	{
		if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR,  RTL8373_MACSEC_REG_GLB_SET1_PORT7_MIBCNT_SWRST_N_OFFSET, reset?0:1)) != RT_ERR_OK)
        			return retVal;
	}
	else
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_wrapper_mib_counter
 * Description:
 *      get wrapper mib counters.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get wrapper mib counters.
 */
rtk_api_ret_t dal_rtl8373_wrapper_mib_counter(rtk_uint32 port, RTL8373_WRAPPER_MIBCOUNTER mibIdx, rtk_uint64* pCounter)
{
	rtk_api_ret_t retVal;
	rtk_uint32 tmp_value_L;
	rtk_uint32 tmp_value_H;
	rtk_uint64 tmp;

	if(port == 0)
    {
        if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
                        return retVal;

            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
                        return retVal;

            tmp = tmp_value_H;
            *pCounter = (tmp << 32) | tmp_value_L;
            
        }
        else
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
                        return retVal;
        }
    }
    else if(port == 1)
    {
        if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
                        return retVal;

            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
                        return retVal;

            tmp = tmp_value_H;
            *pCounter = (tmp << 32) | tmp_value_L;
        }
        else
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
                        return retVal;
        }
    }
    else if(port == 2)
    {
        if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
                        return retVal;

            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
                        return retVal;

            tmp = tmp_value_H;
            *pCounter = (tmp << 32) | tmp_value_L;
        }
        else
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
                        return retVal;
        }
    }
    else if(port == 3)
    {
        if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
                        return retVal;

            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
                        return retVal;

            tmp = tmp_value_H;
            *pCounter = (tmp << 32) | tmp_value_L;
        }
        else
        {
            if ((retVal = dal_rtl8224_top_reg_read(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
                        return retVal;
        }
    }
    else if(port == 4)
	{
		if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
	        			return retVal;

			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
	        			return retVal;

			tmp = tmp_value_H;
			*pCounter = (tmp << 32) | tmp_value_L;
			
		}
		else
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT4_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
	        			return retVal;
		}
	}
	else if(port == 5)
	{
		if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
	        			return retVal;

			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
	        			return retVal;

			tmp = tmp_value_H;
			*pCounter = (tmp << 32) | tmp_value_L;
		}
		else
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT5_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
	        			return retVal;
		}
	}
	else if(port == 6)
	{
		if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
	        			return retVal;

			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
	        			return retVal;

			tmp = tmp_value_H;
			*pCounter = (tmp << 32) | tmp_value_L;
		}
		else
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT6_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
	        			return retVal;
		}
	}
	else if(port == 7)
	{
		if((mibIdx == TXSYS_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK) || (mibIdx == TXLINE_OK))
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + mibIdx * 0x4, &tmp_value_L)) != RT_ERR_OK)
	        			return retVal;

			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + (mibIdx + 1) * 0x4, &tmp_value_H)) != RT_ERR_OK)
	        			return retVal;

			tmp = tmp_value_H;
			*pCounter = (tmp << 32) | tmp_value_L;
		}
		else
		{
			if ((retVal = rtl8373_getAsicReg(RTL8373_MACSEC_TXSYSCRCERR_CNT_PORT7_ADDR + mibIdx * 0x4, (rtk_uint32 *)pCounter)) != RT_ERR_OK)
	        			return retVal;
		}
	}
	else 
		return RT_ERR_INPUT;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ipg_len_set
 * Description:
 *      mac mode MACsec ipg length set.
 * Input:
 *      port   -  port number
 *      length - ipg length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec ipg length.
 */
rtk_api_ret_t dal_rtl8373_macsec_ipg_len_set(rtk_uint32 port, rtk_uint32 length)
{
   	rtk_api_ret_t retVal;

	if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_IPG_CFG_ADDR(port), RTL8373_MAC_MACSEC_IPG_CFG_MACSEC_IPG_LENGTH_MASK, length)) != RT_ERR_OK)
        		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ipg_len_get
 * Description:
 *      mac mode MACsec ipg length get.
 * Input:
 *      port   -  port number
 * Output:
 *      plength - ipg length
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec ipg length.
 */
rtk_api_ret_t dal_rtl8373_macsec_ipg_len_get(rtk_uint32 port, rtk_uint32 *plength)
{
   	rtk_api_ret_t retVal;
	rtk_uint32 length = 0;

	if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_IPG_CFG_ADDR(port), RTL8373_MAC_MACSEC_IPG_CFG_MACSEC_IPG_LENGTH_MASK, &length)) != RT_ERR_OK)
        		return retVal;

	*plength = length;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ipg_mode_set
 * Description:
 *      mac mode MACsec ipg mode set.
 * Input:
 *      port   -  port number
 *      mode -  ipg config mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec ipg mode.
 *	  mode[1:0]:
		0: don't insert ipg for macsec
		1: insert ipg for macsec according to macsec feedback signal
		2: insert ipg for masec according to ethertype & cfg_macsec_ipg_length
		3: always insert ipg for macsec, length according to cfg_macsec_ipg_length
 */
rtk_api_ret_t dal_rtl8373_macsec_ipg_mode_set(rtk_uint32 port, rtk_uint32 mode)
{
   	rtk_api_ret_t retVal;

	if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_IPG_CFG_ADDR(port), RTL8373_MAC_MACSEC_IPG_CFG_MACSEC_IPG_MODE_MASK, mode)) != RT_ERR_OK)
        		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_ipg_mode_get
 * Description:
 *      mac mode MACsec ipg mode get.
 * Input:
 *      port   -  port number
 * Output:
 *      pmode - ipg config mode
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec ipg mode.
 *	  mode[1:0]:
		0: don't insert ipg for macsec
		1: insert ipg for macsec according to macsec feedback signal
		2: insert ipg for masec according to ethertype & cfg_macsec_ipg_length
		3: always insert ipg for macsec, length according to cfg_macsec_ipg_length
 */
rtk_api_ret_t dal_rtl8373_macsec_ipg_mode_get(rtk_uint32 port, rtk_uint32 *pmode)
{
   	rtk_api_ret_t retVal;
	rtk_uint32 mode = 0;

	if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_IPG_CFG_ADDR(port), RTL8373_MAC_MACSEC_IPG_CFG_MACSEC_IPG_MODE_MASK, &mode)) != RT_ERR_OK)
        		return retVal;

	*pmode = mode;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_eth_set
 * Description:
 *      mac mode MACsec eth set.
 * Input:
 *      port   -  port number
 *      entry - entry number(0-7)
 *      ethertype - ether type value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec eth.
 */
rtk_api_ret_t dal_rtl8373_macsec_eth_set(rtk_uint32 port, rtk_uint32 entry, rtk_uint32 ethertype)
{
   	rtk_api_ret_t retVal;

	switch(entry)
	{
		case 0:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_1_0_ADDR(port), RTL8373_MAC_MACSEC_ETH_1_0_MACSEC_ETH_0_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 1:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_1_0_ADDR(port), RTL8373_MAC_MACSEC_ETH_1_0_MACSEC_ETH_1_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 2:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_3_2_ADDR(port), RTL8373_MAC_MACSEC_ETH_3_2_MACSEC_ETH_2_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 3:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_3_2_ADDR(port), RTL8373_MAC_MACSEC_ETH_3_2_MACSEC_ETH_3_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 4:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_5_4_ADDR(port), RTL8373_MAC_MACSEC_ETH_5_4_MACSEC_ETH_4_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 5:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_5_4_ADDR(port), RTL8373_MAC_MACSEC_ETH_5_4_MACSEC_ETH_5_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 6:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_7_6_ADDR(port), RTL8373_MAC_MACSEC_ETH_7_6_MACSEC_ETH_6_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 7:
			if ((retVal = rtl8373_setAsicRegBits(RTL8373_MAC_MACSEC_ETH_7_6_ADDR(port), RTL8373_MAC_MACSEC_ETH_7_6_MACSEC_ETH_7_MASK, ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		default:
			return RT_ERR_INPUT;       
	}

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_eth_get
 * Description:
 *      mac mode MACsec eth get.
 * Input:
 *      port   -  port number
 *      entry - entry number(0-7)
 * Output:
 *      pethertype - ether type value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec eth.
 */
rtk_api_ret_t dal_rtl8373_macsec_eth_get(rtk_uint32 port, rtk_uint32 entry, rtk_uint32 *pethertype)
{
   	rtk_api_ret_t retVal;
	rtk_uint32 ethertype = 0;

	switch(entry)
	{
		case 0:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_1_0_ADDR(port), RTL8373_MAC_MACSEC_ETH_1_0_MACSEC_ETH_0_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 1:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_1_0_ADDR(port), RTL8373_MAC_MACSEC_ETH_1_0_MACSEC_ETH_1_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 2:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_3_2_ADDR(port), RTL8373_MAC_MACSEC_ETH_3_2_MACSEC_ETH_2_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 3:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_3_2_ADDR(port), RTL8373_MAC_MACSEC_ETH_3_2_MACSEC_ETH_3_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 4:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_5_4_ADDR(port), RTL8373_MAC_MACSEC_ETH_5_4_MACSEC_ETH_4_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 5:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_5_4_ADDR(port), RTL8373_MAC_MACSEC_ETH_5_4_MACSEC_ETH_5_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 6:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_7_6_ADDR(port), RTL8373_MAC_MACSEC_ETH_7_6_MACSEC_ETH_6_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		case 7:
			if ((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_MACSEC_ETH_7_6_ADDR(port), RTL8373_MAC_MACSEC_ETH_7_6_MACSEC_ETH_7_MASK, &ethertype)) != RT_ERR_OK)
	        			return retVal;
			break;
		default:
			return RT_ERR_INPUT;       
	}

	*pethertype = ethertype;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_macsec_init
 * Description:
 *      Initialize MACsec information.
 * Input:
 *      port_mask   -  port mask, bit[4:7]
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will initialize MACsec information.
 */
rtk_api_ret_t dal_rtl8373_macsec_init(rtk_uint32 port_mask)
{
	rtk_api_ret_t retVal;
	rtk_uint32 phy_data;
	rtk_uint32 tmp_value;
	rtk_uint32 index = 0;
	rtk_uint32 port;

	dal_rtl8373_mdc_en(ENABLED);
	
       for(port = 0; port < 8; port++)
       {
       	if(port_mask & (1 << port))
	       {
	              /* check phy if link up*/
			if ((retVal = dal_rtl8373_phy_read(port, 7, 1, &phy_data)) != RT_ERR_OK)
		      			return retVal;

			if(((phy_data >> 2) & 0x1) != 1)
					return RT_ERR_PHY_LINK_DOWN;
			
			/* enable macsec egress enable and ingress enable */
			if((retVal = dal_rtl8373_macsec_enable_set(port, ENABLED, ENABLED)) != RT_ERR_OK)
					return retVal;

			if((retVal = dal_rtl8373_macsec_egress_get(port, RTL8373_MACSEC_EIP160_VERSION, &tmp_value)) != RT_ERR_OK)
					return retVal;

			dal_rtl8373_macsec_rxIPbypass_set(port, DISABLED);
			dal_rtl8373_macsec_txIPbypass_set(port, DISABLED);
			dal_rtl8373_macsec_rxbypass_set(port, DISABLED);
			dal_rtl8373_macsec_txbypass_set(port, DISABLED);


			if(port == 0)
            {
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
            }
            else if(port == 1)
            {
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
            }
            else if(port == 2)
            {
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
            }
            else if(port == 3)
            {
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
                if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
                        return retVal;
            }
            else if(port == 4)
			{
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT4_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT4_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
			}
			else if(port == 5)
			{
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT5_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT5_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
			}
			else if(port == 6)
			{
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT6_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT6_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
			}
			else if(port == 7)
			{
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_TX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
				if ((retVal = rtl8373_setAsicRegBit(RTL8373_MACSEC_REG_GLB_SET1_PORT7_ADDR, RTL8373_MACSEC_REG_GLB_SET1_PORT7_RX_SWRST_EN_OFFSET, 1)) != RT_ERR_OK)
	        			return retVal;
			}
			else
				return RT_ERR_PORT_ID;
			
			/* check egress and ingress version*/
			if((tmp_value & 0xffff) != 0x5fa0)
					return RT_ERR_MACSEC_EGRESS_DEVICE;

			if((retVal = dal_rtl8373_macsec_ingress_get(port, RTL8373_MACSEC_EIP160_VERSION, &tmp_value)) != RT_ERR_OK)
					return retVal;

			if((tmp_value & 0xffff) != 0x5fa0)
					return RT_ERR_MACSEC_INGRESS_DEVICE;

			/* initial egress and ingress transform records*/
			for(index = 0; index < 16; index++)
			{
				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_CTRL_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_ID_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Key0_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Key1_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Key2_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Key3_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Hkey0_Key4_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Hkey1_Key5_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Hkey2_Key6_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Hkey3_Key7_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Seq0_Hkey0_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Seq1_Hkey1_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Zero_Hkey2_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_IS0_Hkey3_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_IS1_Seq0_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_IS2_Seq1_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_IV0_Zero_E_64(index), 0)) != RT_ERR_OK)
						return retVal;
				
				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_IV1_IS0_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Zero_IS1_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_SAupd_IS2_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Zero_IV0_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Zero_IV1_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_TR_Zero_SAupd_E_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_CTRL_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_ID_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Key0_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Key1_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Key2_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Key3_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Hkey0_Key4_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Hkey1_Key5_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Hkey2_Key6_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Hkey3_Key7_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Seq0_Hkey0_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Seq1_Hkey1_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Mask_Hkey2_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_IV0_Hkey3_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_IV1_Seq0_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_IV2_Seq1_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Zero_Mask_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Zero_IV0_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Zero_IV1_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_TR_Zero_IV2_I_64(index), 0)) != RT_ERR_OK)
						return retVal;

				//initial SA match rule and flow index
				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_SA_LO(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_SA_HI(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_DA_LO(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_DA_HI(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_MISC(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_SCI_LO(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_SCI_HI(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_MASK(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_EXT(index), 0)) != RT_ERR_OK)
						return retVal;

				if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_SAM_FLOW_CTRL(index), 0)) != RT_ERR_OK)
						return retVal;
			}

			/* configure egress and ingress context size*/
			if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_EIP62_CONTEXT_CTRL, 0xe5880218)) != RT_ERR_OK)
						return retVal;

			if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_EIP62_CONTEXT_CTRL, 0xe5880214)) != RT_ERR_OK)
						return retVal;

			/* configure context update control*/
			if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_EIP62_CONTEXT_UPD_CTRL, 0x0003)) != RT_ERR_OK)
						return retVal;

			if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_EIP62_CONTEXT_UPD_CTRL, 0x0003)) != RT_ERR_OK)
						return retVal;

			/* configure MACsec fix latency and xform size*/
			if((retVal = dal_rtl8373_macsec_egress_set(port, RTL8373_MACSEC_MISC_CONTROL, 0x02000030)) != RT_ERR_OK)
						return retVal;

			if((retVal = dal_rtl8373_macsec_ingress_set(port, RTL8373_MACSEC_MISC_CONTROL, 0x01000833)) != RT_ERR_OK)
						return retVal;
	       }
				
       }

	return RT_ERR_OK;

}


