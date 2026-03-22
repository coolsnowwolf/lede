/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

/*
 * Include Files
 */
#if defined(RTK_PHYDRV_IN_LINUX)
  #include "phy_rtl826xb_patch.h"
  #include "construct/conf_rtl8264b.c"
  #include "construct/conf_rtl8261n_c.c"
#else
  #include <common/rt_type.h>
  #include <common/rt_error.h>
  #include <common/debug/rt_log.h>
  #include <soc/type.h>
  #include <hal/common/halctrl.h>
  #include <hal/mac/miim_common_drv.h>
  #include <hal/phy/phy_construct.h>
  #include <osal/time.h>
  #include <hal/phy/construct/conftypes.h>
  #include <hal/phy/phy_probe.h>
  #include <hal/phy/phy_patch.h>
  #include <osal/memory.h>
  #if defined(CONFIG_SDK_RTL826XB)
    #include <hal/phy/phy_rtl826xb.h>
    #include <hal/phy/construct/conf_rtl8264b.c>
    #include <hal/phy/construct/conf_rtl8261n_c.c>
  #endif
#endif
/*
 * Symbol Definition
 */
#define PHY_PATCH_WAIT_TIMEOUT     10000000

#define PHY_PATCH_LOG    LOG_INFO

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */
static uint16 _phy_rtl826xb_mmd_convert(uint16 page, uint16 addr)
{
    uint16 reg = 0;
    if (addr < 16)
    {
        reg = 0xA400 + (page * 2);
    }
    else if (addr < 24)
    {
        reg = (16*page) + ((addr - 16) * 2);
    }
    else
    {
        reg = 0xA430 + ((addr - 24) * 2);
    }
    return reg;
}

static int32
_phy_rtl826xb_patch_wait(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 data, uint32 mask, uint8 patch_mode)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 cnt = 0;
    WAIT_COMPLETE_VAR()

    rtk_port_t  p = 0;
    uint8  smiBus = HWP_PORT_SMI(unit, port);
    uint32 phyChip = HWP_PHY_MODEL_BY_PORT(unit, port);
    uint8  bcast_phyad = HWP_PHY_ADDR(unit, port);;


    if (patch_mode == PHY_PATCH_MODE_BCAST_BUS)
    {
        if ((ret = phy_826xb_ctrl_set(unit, port, RTK_PHY_CTRL_MIIM_BCAST, 0)) != RT_ERR_OK)
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait disable broadcast failed! 0x%X\n", unit, p, ret);
            return ret;
        }

        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if ((HWP_PORT_SMI(unit, p) == smiBus) && (HWP_PHY_MODEL_BY_PORT(unit, p) == phyChip))
            {
                WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
                {
                    if ((ret = phy_common_general_reg_mmd_get(unit, p, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                    {
                        return ret;
                    }
                    ++cnt;

                    if ((rData & mask) == data)
                        break;

                    //osal_time_udelay(10);
                }

                if (WAIT_COMPLETE_IS_TIMEOUT())
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, p, mmdAddr, mmdReg, data, mask, rData, cnt);
                    return RT_ERR_TIMEOUT;
                }
            }
        }

        osal_time_mdelay(1);
        //for port in same SMI bus, set mdio broadcast ENABLE
        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if ((HWP_PORT_SMI(unit, p) == smiBus) && (HWP_PHY_MODEL_BY_PORT(unit, p) == phyChip))
            {
                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST_PHYAD, (uint32)bcast_phyad)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait set broadcast PHYAD failed! 0x%X\n", unit, p, ret);
                    return ret;
                }

                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST, 1)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait enable broadcast failed! 0x%X\n", unit, p, ret);
                    return ret;
                }
            }
        }
    }
    else if (patch_mode == PHY_PATCH_MODE_BCAST)
    {
        if ((ret = phy_826xb_ctrl_set(unit, port, RTK_PHY_CTRL_MIIM_BCAST, 0)) != RT_ERR_OK)
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826x patch wait disable broadcast failed! 0x%X\n", unit, p, ret);
            return ret;
        }

        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if (HWP_PHY_BASE_MACID(unit, p) == HWP_PHY_BASE_MACID(unit, port))
            {
                WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
                {
                    if ((ret = phy_common_general_reg_mmd_get(unit, p, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                    {
                        return ret;
                    }
                    ++cnt;

                    if ((rData & mask) == data)
                        break;
                    //osal_time_udelay(10);
                }

                if (WAIT_COMPLETE_IS_TIMEOUT())
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826x patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, p, mmdAddr, mmdReg, data, mask, rData, cnt);
                    return RT_ERR_TIMEOUT;
                }
            }
        }

        osal_time_mdelay(1);
        //for port in same PHY, set mdio broadcast ENABLE
        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if (HWP_PHY_BASE_MACID(unit, p) == HWP_PHY_BASE_MACID(unit, port))
            {
                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST_PHYAD, (uint32)bcast_phyad)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait set broadcast PHYAD failed! 0x%X\n", unit, p, ret);
                    return ret;
                }

                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST, 1)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait enable broadcast failed! 0x%X\n", unit, p, ret);
                    return ret;
                }
            }
        }
    }
    else
    {
        WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
        {
            if ((ret = phy_common_general_reg_mmd_get(unit, port, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                return ret;

            ++cnt;
            if ((rData & mask) == data)
                break;

            osal_time_mdelay(1);
        }

        if (WAIT_COMPLETE_IS_TIMEOUT())
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, port, mmdAddr, mmdReg, data, mask, rData, cnt);
            return RT_ERR_TIMEOUT;
        }
    }

    return RT_ERR_OK;
}

static int32
_phy_rtl826xb_patch_wait_not_equal(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 data, uint32 mask, uint8 patch_mode)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 cnt = 0;
    WAIT_COMPLETE_VAR()

    rtk_port_t  p = 0;
    uint8  smiBus = HWP_PORT_SMI(unit, port);
    uint32 phyChip = HWP_PHY_MODEL_BY_PORT(unit, port);
    uint8  bcast_phyad = HWP_PHY_ADDR(unit, port);

    if (patch_mode == PHY_PATCH_MODE_BCAST_BUS)
    {
        if ((ret = phy_826xb_ctrl_set(unit, port, RTK_PHY_CTRL_MIIM_BCAST, 0)) != RT_ERR_OK)
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait disable broadcast failed! 0x%X\n", unit, p, ret);
            return ret;
        }

        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if ((HWP_PORT_SMI(unit, p) == smiBus) && (HWP_PHY_MODEL_BY_PORT(unit, p) == phyChip))
            {
                WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
                {
                    if ((ret = phy_common_general_reg_mmd_get(unit, p, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                    {
                        return ret;
                    }
                    ++cnt;

                    if ((rData & mask) != data)
                        break;

                    //osal_time_udelay(10);
                }
                if (WAIT_COMPLETE_IS_TIMEOUT())
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, p, mmdAddr, mmdReg, data, mask, rData, cnt);
                    return RT_ERR_TIMEOUT;
                }
            }
        }

        osal_time_mdelay(1);
        //for port in same SMI bus, set mdio broadcast ENABLE
        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if ((HWP_PORT_SMI(unit, p) == smiBus) && (HWP_PHY_MODEL_BY_PORT(unit, p) == phyChip))
            {
                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST_PHYAD, (uint32)bcast_phyad)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait set broadcast PHYAD failed! 0x%X\n", unit, p, ret);
                    return ret;
                }

                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST, 1)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait enable broadcast failed! 0x%X\n", unit, p, ret);
                    return ret;
                }
            }
        }
    }
    else if (patch_mode == PHY_PATCH_MODE_BCAST)
    {
        if ((ret = phy_826xb_ctrl_set(unit, port, RTK_PHY_CTRL_MIIM_BCAST, 0)) != RT_ERR_OK)
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826x patch wait disable broadcast failed! 0x%X\n", unit, p, ret);
            return ret;
        }

        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if (HWP_PHY_BASE_MACID(unit, p) == HWP_PHY_BASE_MACID(unit, port))
            {
                WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
                {
                    if ((ret = phy_common_general_reg_mmd_get(unit, p, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                    {
                        return ret;
                    }
                    ++cnt;

                    if (((rData & mask) != data))
                        break;

                    //osal_time_udelay(10);
                }

                if (WAIT_COMPLETE_IS_TIMEOUT())
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, p, mmdAddr, mmdReg, data, mask, rData, cnt);
                    return RT_ERR_TIMEOUT;
                }
            }
        }

        osal_time_mdelay(1);
        //for port in same PHY, set mdio broadcast ENABLE
        HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)
        {
            if (HWP_PHY_BASE_MACID(unit, p) == HWP_PHY_BASE_MACID(unit, port))
            {
                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST_PHYAD, (uint32)bcast_phyad)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait set broadcast PHYAD failed! 0x%X\n", unit, p, ret);
                    return ret;
                }

                if ((ret = phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST, 1)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826XB patch wait enable broadcast failed! 0x%X\n", unit, p, ret);
                    return ret;
                }
            }
        }
    }
    else
    {
        WAIT_COMPLETE(PHY_PATCH_WAIT_TIMEOUT)
        {
            if ((ret = phy_common_general_reg_mmd_get(unit, port, mmdAddr, mmdReg, &rData)) != RT_ERR_OK)
                return ret;

            ++cnt;
            if ((rData & mask) != data)
                break;

            osal_time_mdelay(1);
        }
        if (WAIT_COMPLETE_IS_TIMEOUT())
        {
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u 826xb patch wait[%u,0x%X,0x%X,0x%X]:0x%X cnt:%u\n", unit, port, mmdAddr, mmdReg, data, mask, rData, cnt);
            return RT_ERR_TIMEOUT;
        }

    }

    return RT_ERR_OK;
}

static int32
_phy_rtl826xb_patch_top_get(uint32 unit, rtk_port_t port, uint32 topPage, uint32 topReg, uint32 *pData)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 topAddr = (topPage * 8) + (topReg - 16);

    if ((ret = phy_common_general_reg_mmd_get(unit, port, PHY_MMD_VEND1, topAddr, &rData)) != RT_ERR_OK)
        return ret;
    *pData = rData;
    return RT_ERR_OK;
}

static int32
_phy_rtl826xb_patch_top_set(uint32 unit, rtk_port_t port, uint32 topPage, uint32 topReg, uint32 wData)
{
    int32  ret = 0;
    uint32 topAddr = (topPage * 8) + (topReg - 16);
    if ((ret = phy_common_general_reg_mmd_set(unit, port, PHY_MMD_VEND1, topAddr, wData)) != RT_ERR_OK)
        return ret;
    return RT_ERR_OK;
}

static int32
_phy_rtl826xb_patch_sds_get(uint32 unit, rtk_port_t port, uint32 sdsPage, uint32 sdsReg, uint32 *pData)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 sdsAddr = 0x8000 + (sdsReg << 6) + sdsPage;

    if ((ret = _phy_rtl826xb_patch_top_set(unit, port, 40, 19, sdsAddr)) != RT_ERR_OK)
        return ret;
    if ((ret = _phy_rtl826xb_patch_top_get(unit, port, 40, 18, &rData)) != RT_ERR_OK)
        return ret;
    *pData = rData;
    return _phy_rtl826xb_patch_wait(unit, port, PHY_MMD_VEND1, 0x143, 0, BIT_15, PHY_PATCH_MODE_NORMAL);
}

static int32
_phy_rtl826xb_patch_sds_set(uint32 unit, rtk_port_t port, uint32 sdsPage, uint32 sdsReg, uint32 wData, uint8 patch_mode)
{
    int32  ret = 0;
    uint32 sdsAddr = 0x8800 + (sdsReg << 6) + sdsPage;

    if ((ret = _phy_rtl826xb_patch_top_set(unit, port, 40, 17, wData)) != RT_ERR_OK)
        return ret;
    if ((ret = _phy_rtl826xb_patch_top_set(unit, port, 40, 19, sdsAddr)) != RT_ERR_OK)
        return ret;
    return _phy_rtl826xb_patch_wait(unit, port, PHY_MMD_VEND1, 0x143, 0, BIT_15, patch_mode);
}

static int32 _phy_rtl826xb_flow_r1(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1, patch_mode), ret);

    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready = 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait(unit, port, 31, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT_6, BIT_6, patch_mode), ret);

    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 $0x3802
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x3802, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 27 0xB82E
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0xB82E, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x1, patch_mode), ret);

     return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_r12(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x1, patch_mode), ret);

    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready = 1
    RT_ERR_CHK(_phy_rtl826xb_patch_wait(unit, port, 31, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT_6, BIT_6, patch_mode), ret);

    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 $0x3800
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x3800, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 27 0xB82E
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0xB82E, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x1, patch_mode), ret);

    return RT_ERR_OK;
}


static int32 _phy_rtl826xb_flow_r2(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg w $PHYID 0xa43 27 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x0000, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x0000, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xB82 23 0 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xB82, 23, 0, 0, 0x0, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 27 $0x8023
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 27, 15, 0, 0x8023, patch_mode), ret);
    //PHYReg w $PHYID 0xa43 28 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa43, 28, 15, 0, 0x0000, patch_mode), ret);

    //PHYReg_bit w $PHYID 0xb82 16 4 4 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xb82, 16, 4, 4, 0x0, patch_mode), ret);
    //set patch_rdy [PHYReg_bit r $PHYID 0xb80 16 6 6] ; Wait for patch ready != 1
    RT_ERR_CHK( _phy_rtl826xb_patch_wait_not_equal(unit, port, 31, _phy_rtl826xb_mmd_convert(0xb80, 16), BIT_6, BIT_6, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_l1(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x1, patch_mode), ret);

    //set pcs_state [PHYReg_bit r $PHYID 0xa60 16 7 0] ; Wait for pcs state = 1
    RT_ERR_CHK( _phy_rtl826xb_patch_wait(unit, port, 31, _phy_rtl826xb_mmd_convert(0xa60, 16), 0x1, 0xFF, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_l2(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa4a 16 10 10 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa4a, 16, 10, 10, 0x0, patch_mode), ret);

    //set pcs_state [PHYReg_bit r $PHYID 0xa60 16 7 0] ; Wait for pcs state != 1
    RT_ERR_CHK( _phy_rtl826xb_patch_wait_not_equal(unit, port, 31, _phy_rtl826xb_mmd_convert(0xa60, 16), 0x1, 0xFF, patch_mode), ret);

     return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_pi(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;
    uint32 rData = 0, cnt = 0;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    _phy_rtl826xb_flow_l1(unit, port, portOffset, patch_mode);

    //  PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x1; #SS_EN_XG = 1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 8 8 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 8, 8, 0x0, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 5 5 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 5, 5, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 4 4 0x1;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 4, 4, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x0, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x0, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x0;
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x0, patch_mode), ret);

    //PP_PHYReg_bit r $PHYID 0xbc62 12 8
    if ((ret = phy_common_general_reg_mmd_get(unit, port, PHY_MMD_VEND2, 0xbc62, &rData)) != RT_ERR_OK)
        return ret;
    rData = REG32_FIELD_GET(rData, 8, 0x1F00);
    for (cnt = 0; cnt <= rData; cnt++)
    {
        //PP_PHYReg_bit w $PHYID 0xbc62 12 8 $t
        RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc62, 12, 8, cnt, patch_mode), ret);
    }

    //   PP_PHYReg_bit w $PHYID 0xbc02 2 2 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc02, 2, 2, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbc02 3 3 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc02, 3, 3, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 6 6 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 6, 6, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 9 9 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 9, 9, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbf86 7 7 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbf86, 7, 7, 0x1, patch_mode), ret);
    //   PP_PHYReg_bit w $PHYID 0xbc04 9 2 0xff
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHYOCP, 0xFF, 31, 0xbc04, 9, 2, 0xff, patch_mode), ret);

    _phy_rtl826xb_flow_l2(unit, port, portOffset, patch_mode);
    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n01(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0000, patch_mode), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n02(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0000, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);
    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n11(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0010
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0010, patch_mode), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n12(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0010
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0010, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n21(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x1
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x1, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0020
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0020, patch_mode), ret);
    //# PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    //RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_n22(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    //PHYReg_bit w $PHYID 0xa01 21 15 0 0x0
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 21, 15, 0, 0x0, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 19 15 0 0x0020
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 19, 15, 0, 0x0020, patch_mode), ret);
    //PHYReg_bit w $PHYID 0xa01 17 15 0 0x0000
    RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PHY, 0xFF, 0xa01, 17, 15, 0, 0x0000, patch_mode), ret);

    return RT_ERR_OK;
}

static int32 _phy_rtl826xb_flow_s(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *pPatchDb = NULL;

    if (PHYPATCH_IS_RTKSDS(unit))
    {
        PHYPATCH_DB_GET(unit, port, pPatchDb);
        RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PSDS0, 0xff, 0x07, 0x10, 15, 0, 0x80aa, patch_mode), ret);
        RT_ERR_CHK(phy_patch_op(pPatchDb, unit, port, portOffset, RTK_PATCH_OP_PSDS0, 0xff, 0x06, 0x12, 15, 0, 0x5078, patch_mode), ret);
    }

    return RT_ERR_OK;
}

static int32 phy_rtl826xb_patch_op(uint32 unit, rtk_port_t port, uint8 portOffset, rtk_hwpatch_t *pPatch_data, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    uint32 rData = 0, wData = 0;
    uint16 reg = 0;
    uint8 patch_op = 0;
    uint32 mask = 0;

    if ((pPatch_data->portmask & (1 << portOffset)) == 0)
    {
        return RT_ERR_ABORT;
    }
    mask = UINT32_BITS_MASK(pPatch_data->msb, pPatch_data->lsb);
    patch_op = phy_patch_op_translate(patch_mode, pPatch_data->patch_op, pPatch_data->compare_op);

    #if 0
    osal_printf("[%s,%d]u%up%u, patch_mode:%u/patch_op:%u/compare_op:%u => op: %u\n", __FUNCTION__, __LINE__, unit, port,
                           patch_mode, pPatch_data->patch_op, pPatch_data->compare_op,
                           patch_op);
    #endif

    switch (patch_op)
    {
        case RTK_PATCH_OP_PHY:
            reg = _phy_rtl826xb_mmd_convert(pPatch_data->pagemmd, pPatch_data->addr);
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, reg, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(phy_common_general_reg_mmd_set(unit, port, 31, reg, wData), ret);
            break;
        case RTK_PATCH_OP_CMP_PHY:
            reg = _phy_rtl826xb_mmd_convert(pPatch_data->pagemmd, pPatch_data->addr);
            RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, reg, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;
        case RTK_PATCH_OP_CMP_SRAM_PHY:
            reg = _phy_rtl826xb_mmd_convert(pPatch_data->sram_p, pPatch_data->sram_rw);
            RT_ERR_CHK(phy_common_general_reg_mmd_set(unit, port, 31, reg, pPatch_data->sram_a), ret);
            reg = _phy_rtl826xb_mmd_convert(pPatch_data->sram_p, pPatch_data->sram_rr);
            RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, reg, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;

        case RTK_PATCH_OP_PHYOCP:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, pPatch_data->addr, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(phy_common_general_reg_mmd_set(unit, port, 31, pPatch_data->addr, wData), ret);
            break;
        case RTK_PATCH_OP_CMP_PHYOCP:
            RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, pPatch_data->addr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;
        case RTK_PATCH_OP_CMP_SRAM_PHYOCP:
            RT_ERR_CHK(phy_common_general_reg_mmd_set(unit, port, 31, pPatch_data->sram_rw, pPatch_data->sram_a), ret);
            RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 31, pPatch_data->sram_rr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;

        case RTK_PATCH_OP_TOP:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(_phy_rtl826xb_patch_top_get(unit, port, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(_phy_rtl826xb_patch_top_set(unit, port, pPatch_data->pagemmd, pPatch_data->addr, wData), ret);
            break;
        case RTK_PATCH_OP_CMP_TOP:
            RT_ERR_CHK(_phy_rtl826xb_patch_top_get(unit, port, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;
        case RTK_PATCH_OP_CMP_SRAM_TOP:
            RT_ERR_CHK(_phy_rtl826xb_patch_top_set(unit, port, pPatch_data->sram_p, pPatch_data->sram_rw, pPatch_data->sram_a), ret);
            RT_ERR_CHK(_phy_rtl826xb_patch_top_get(unit, port, pPatch_data->sram_p, pPatch_data->sram_rr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;

        case RTK_PATCH_OP_PSDS0:
            if ((pPatch_data->msb != 15) || (pPatch_data->lsb != 0))
            {
                RT_ERR_CHK(_phy_rtl826xb_patch_sds_get(unit, port, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            }
            wData = REG32_FIELD_SET(rData, pPatch_data->data, pPatch_data->lsb, mask);
            RT_ERR_CHK(_phy_rtl826xb_patch_sds_set(unit, port, pPatch_data->pagemmd, pPatch_data->addr, wData, patch_mode), ret);
            break;
        case RTK_PATCH_OP_CMP_PSDS0:
            RT_ERR_CHK(_phy_rtl826xb_patch_sds_get(unit, port, pPatch_data->pagemmd, pPatch_data->addr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;
        case RTK_PATCH_OP_CMP_SRAM_PSDS0:
            RT_ERR_CHK(_phy_rtl826xb_patch_sds_set(unit, port, pPatch_data->sram_p, pPatch_data->sram_rw, pPatch_data->sram_a, patch_mode), ret);
            RT_ERR_CHK(_phy_rtl826xb_patch_sds_get(unit, port, pPatch_data->sram_p, pPatch_data->sram_rr, &rData), ret);
            PHYPATCH_COMPARE(pPatch_data->pagemmd, pPatch_data->addr, pPatch_data->msb, pPatch_data->lsb, pPatch_data->data, rData, mask);
            break;

        case RTK_PATCH_OP_SKIP:
            return RT_ERR_ABORT;

        default:
            RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u patch_op:%u not implemented yet!\n", unit, port, pPatch_data->patch_op);
            return RT_ERR_DRIVER_NOT_SUPPORTED;
    }

    return ret;
}

static int32 phy_rtl826xb_patch_flow(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_flow, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;

    RT_LOG(LOG_INFO, (MOD_HAL | MOD_PHY), "[%s]U%u,P%u,flow%u\n", __FUNCTION__, unit, port, (patch_flow - PHY_PATCH_TYPE_END));
    switch (patch_flow)
    {
        case RTK_PATCH_TYPE_FLOW(0):
            RT_ERR_CHK(_phy_rtl826xb_flow_r1(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(1):
            RT_ERR_CHK(_phy_rtl826xb_flow_r2(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(2):
            RT_ERR_CHK(_phy_rtl826xb_flow_l1(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(3):
            RT_ERR_CHK(_phy_rtl826xb_flow_l2(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(4):
            RT_ERR_CHK(_phy_rtl826xb_flow_n01(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(5):
            RT_ERR_CHK(_phy_rtl826xb_flow_n02(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(6):
            RT_ERR_CHK(_phy_rtl826xb_flow_n11(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(7):
            RT_ERR_CHK(_phy_rtl826xb_flow_n12(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(8):
            RT_ERR_CHK(_phy_rtl826xb_flow_n21(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(9):
            RT_ERR_CHK(_phy_rtl826xb_flow_n22(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(10):
            RT_ERR_CHK(_phy_rtl826xb_flow_s(unit, port, portOffset, patch_mode), ret);
            break;

        case RTK_PATCH_TYPE_FLOW(11):
            RT_ERR_CHK(_phy_rtl826xb_flow_pi(unit, port, portOffset, patch_mode), ret);
            break;
        case RTK_PATCH_TYPE_FLOW(12):
            RT_ERR_CHK(_phy_rtl826xb_flow_r12(unit, port, portOffset, patch_mode), ret);
            break;

        default:
            return RT_ERR_INPUT;
    }
    return RT_ERR_OK;
}

int32 phy_rtl826xb_patch_db_init(uint32 unit, rtk_port_t port, rt_phy_patch_db_t **pPhy_patchDb)
{
    int32 ret = RT_ERR_OK;
    rt_phy_patch_db_t *patch_db = NULL;
    uint32 rData = 0;

    patch_db = osal_alloc(sizeof(rt_phy_patch_db_t));
    RT_PARAM_CHK(NULL == patch_db, RT_ERR_MEM_ALLOC);
    osal_memset(patch_db, 0x0, sizeof(rt_phy_patch_db_t));

    /* patch callback */
    patch_db->fPatch_op = phy_rtl826xb_patch_op;
    patch_db->fPatch_flow = phy_rtl826xb_patch_flow;

    /* patch table */
    RT_ERR_CHK(phy_common_general_reg_mmd_get(unit, port, 30, 0x104, &rData), ret);
    if ((rData & 0x7) == 0x0)
    {
        /* patch */
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  0, RTK_PATCH_TYPE_FLOW(12), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_NCTL0, rtl8264b_nctl0_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_NCTL1, rtl8264b_nctl1_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  3, PHY_PATCH_TYPE_NCTL2, rtl8264b_nctl2_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_UC2, rtl8264b_uc2_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  5, PHY_PATCH_TYPE_UC, rtl8264b_uc_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  6, PHY_PATCH_TYPE_DATARAM, rtl8264b_dataram_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  7, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(2), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  9, PHY_PATCH_TYPE_ALGXG, rtl8264b_algxg_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_ALG1G, rtl8264b_alg_giga_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 11, PHY_PATCH_TYPE_NORMAL, rtl8264b_normal_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_TOP, rtl8264b_top_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_SDS, rtl8264b_sds_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 14, PHY_PATCH_TYPE_AFE, rtl8264b_afe_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_RTCT, rtl8264b_rtct_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(3), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 17, RTK_PATCH_TYPE_FLOW(11), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 18, RTK_PATCH_TYPE_FLOW(10), NULL);

        /* compare */
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  0, PHY_PATCH_TYPE_TOP, rtl8264b_top_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_SDS, rtl8264b_sds_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_AFE, rtl8264b_afe_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  3, RTK_PATCH_TYPE_FLOW(4), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_NCTL0, rtl8264b_nctl0_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  5, RTK_PATCH_TYPE_FLOW(5), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  6, RTK_PATCH_TYPE_FLOW(6), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  7, PHY_PATCH_TYPE_NCTL1, rtl8264b_nctl1_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(7), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  9, RTK_PATCH_TYPE_FLOW(8), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_NCTL2, rtl8264b_nctl2_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 11, RTK_PATCH_TYPE_FLOW(9), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_UC, rtl8264b_uc_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_UC2, rtl8264b_uc2_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 14, RTK_PATCH_TYPE_FLOW(12), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_DATARAM, rtl8264b_dataram_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 17, PHY_PATCH_TYPE_ALGXG, rtl8264b_algxg_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 18, PHY_PATCH_TYPE_ALG1G, rtl8264b_alg_giga_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 19, PHY_PATCH_TYPE_NORMAL, rtl8264b_normal_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 20, PHY_PATCH_TYPE_RTCT, rtl8264b_rtct_conf);
    }
    else
    {
        /* patch */
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  0, RTK_PATCH_TYPE_FLOW(0), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_NCTL0, rtl8261n_c_nctl0_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_NCTL1, rtl8261n_c_nctl1_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  3, PHY_PATCH_TYPE_NCTL2, rtl8261n_c_nctl2_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_UC2, rtl8261n_c_uc2_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  5, PHY_PATCH_TYPE_UC, rtl8261n_c_uc_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  6, PHY_PATCH_TYPE_DATARAM, rtl8261n_c_dataram_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  7, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(2), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db,  9, PHY_PATCH_TYPE_ALGXG, rtl8261n_c_algxg_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_ALG1G, rtl8261n_c_alg_giga_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 11, PHY_PATCH_TYPE_NORMAL, rtl8261n_c_normal_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_TOP, rtl8261n_c_top_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_SDS, rtl8261n_c_sds_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 14, PHY_PATCH_TYPE_AFE, rtl8261n_c_afe_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_RTCT, rtl8261n_c_rtct_conf);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(3), NULL);
        PHYPATCH_SEQ_TABLE_ASSIGN(patch_db, 17, RTK_PATCH_TYPE_FLOW(10), NULL);

        /* compare */
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  0, PHY_PATCH_TYPE_TOP, rtl8261n_c_top_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  1, PHY_PATCH_TYPE_SDS, rtl8261n_c_sds_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  2, PHY_PATCH_TYPE_AFE, rtl8261n_c_afe_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  3, RTK_PATCH_TYPE_FLOW(4), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  4, PHY_PATCH_TYPE_NCTL0, rtl8261n_c_nctl0_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  5, RTK_PATCH_TYPE_FLOW(5), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  6, RTK_PATCH_TYPE_FLOW(6), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  7, PHY_PATCH_TYPE_NCTL1, rtl8261n_c_nctl1_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  8, RTK_PATCH_TYPE_FLOW(7), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db,  9, RTK_PATCH_TYPE_FLOW(8), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 10, PHY_PATCH_TYPE_NCTL2, rtl8261n_c_nctl2_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 11, RTK_PATCH_TYPE_FLOW(9), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 12, PHY_PATCH_TYPE_UC, rtl8261n_c_uc_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 13, PHY_PATCH_TYPE_UC2, rtl8261n_c_uc2_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 14, RTK_PATCH_TYPE_FLOW(0), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 15, PHY_PATCH_TYPE_DATARAM, rtl8261n_c_dataram_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 16, RTK_PATCH_TYPE_FLOW(1), NULL);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 17, PHY_PATCH_TYPE_ALGXG, rtl8261n_c_algxg_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 18, PHY_PATCH_TYPE_ALG1G, rtl8261n_c_alg_giga_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 19, PHY_PATCH_TYPE_NORMAL, rtl8261n_c_normal_conf);
        PHYPATCH_CMP_TABLE_ASSIGN(patch_db, 20, PHY_PATCH_TYPE_RTCT, rtl8261n_c_rtct_conf);
    }
    *pPhy_patchDb = patch_db;
    return ret;
}

/* Function Name:
 *      phy_rtl826xb_patch
 * Description:
 *      apply initial patch data to PHY
 * Input:
 *      unit       - unit id
 *      baseport   - base port id on the PHY chip
 *      portOffset - the index offset base on baseport for the port to patch
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_SUPPORTED
 *      RT_ERR_ABORT
 * Note:
 *      None
 */
int32 phy_rtl826xb_patch(uint32 unit, rtk_port_t port, uint8 portOffset)
{
    return phy_patch( unit, port, portOffset, PHY_PATCH_MODE_NORMAL);
}

/* Function Name:
 *      phy_rtl826xb_broadcast_patch
 * Description:
 *      apply patch data to PHY
 * Input:
 *      unit       - unit id
 *      baseport   - base port id on the PHY chip
 *      portOffset - the index offset base on baseport for the port to patch
 *      perChip    - 1 for per-chip mode, 0 for per-bus mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_SUPPORTED
 *      RT_ERR_ABORT
 * Note:
 *      None
 */
int32 phy_rtl826xb_broadcast_patch(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 perChip)
{
    int32 ret = 0;
    if (perChip == 0)
    {
        ret = phy_patch(unit, port, portOffset, PHY_PATCH_MODE_BCAST_BUS);
    }
    else
    {
        ret = phy_patch(unit, port, portOffset, PHY_PATCH_MODE_BCAST);
    }
    return ret;
}

