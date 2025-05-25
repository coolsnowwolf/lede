/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

/*
 * Include Files
 */
#if defined(RTK_PHYDRV_IN_LINUX)
  #include "rtk_osal.h"
#else
  #include <common/rt_type.h>
  #include <common/rt_error.h>
  #include <common/debug/rt_log.h>
  #include <hal/common/halctrl.h>
  #include <hal/phy/phy_patch.h>
#endif

/*
 * Function Declaration
 */
uint8 phy_patch_op_translate(uint8 patch_mode, uint8 patch_op, uint8 compare_op)
{
    if (patch_mode != PHY_PATCH_MODE_CMP)
    {
        return patch_op;
    }
    else
    {
        switch (compare_op)
        {
            case RTK_PATCH_CMP_WS:
                return RTK_PATCH_OP_SKIP;
            case RTK_PATCH_CMP_W:
            case RTK_PATCH_CMP_WC:
            case RTK_PATCH_CMP_SWC:
            default:
                return RTK_PATCH_OP_TO_CMP(patch_op, compare_op);
        }
    }
}

int32 phy_patch_op(rt_phy_patch_db_t *pPhy_patchDb, uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_op, uint16 portmask, uint16 pagemmd, uint16 addr, uint8 msb, uint8 lsb, uint16 data, uint8 patch_mode)
{
    rtk_hwpatch_t op;

    op.patch_op = patch_op;
    op.portmask = portmask;
    op.pagemmd  = pagemmd;
    op.addr     = addr;
    op.msb      = msb;
    op.lsb      = lsb;
    op.data     = data;
    op.compare_op = RTK_PATCH_CMP_W;

    return pPhy_patchDb->fPatch_op(unit, port, portOffset, &op, patch_mode);
}

static int32 _phy_patch_process(uint32 unit, rtk_port_t port, uint8 portOffset, rtk_hwpatch_t *pPatch, int32 size, uint8 patch_mode)
{
    int32 i = 0;
    int32 ret = 0;
    int32 chk_ret = RT_ERR_OK;
    int32 n;
    rtk_hwpatch_t *patch = pPatch;
    rt_phy_patch_db_t *pPatchDb = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    if (size <= 0)
    {
        return RT_ERR_OK;
    }
    n = size / sizeof(rtk_hwpatch_t);

    for (i = 0; i < n; i++)
    {
        ret = pPatchDb->fPatch_op(unit, port, portOffset, &patch[i], patch_mode);
        if ((ret != RT_ERR_ABORT) && (ret != RT_ERR_OK))
        {
            if ((ret == RT_ERR_CHECK_FAILED) && (patch_mode == PHY_PATCH_MODE_CMP))
            {
                osal_printf("PATCH CHECK: Failed entry:%u|%u|0x%X|0x%X|%u|%u|0x%X\n",
                            i + 1, patch[i].patch_op, patch[i].pagemmd, patch[i].addr, patch[i].msb, patch[i].lsb, patch[i].data);
                chk_ret = RT_ERR_CHECK_FAILED;
                continue;
            }
            else
            {
                RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u %s failed! %u[%u][0x%X][0x%X][0x%X] ret=0x%X\n", unit, port, __FUNCTION__,
                       i+1, patch[i].patch_op, patch[i].pagemmd, patch[i].addr, patch[i].data, ret);
                return ret;
            }
        }

    }
    return (chk_ret == RT_ERR_CHECK_FAILED) ? chk_ret : RT_ERR_OK;
}

/* Function Name:
 *      phy_patch
 * Description:
 *      apply initial patch data to PHY
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      portOffset - the index offset of port based the base port in the PHY chip
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHECK_FAILED
 *      RT_ERR_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 phy_patch(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_mode)
{
    int32 ret = RT_ERR_OK;
    int32 chk_ret = RT_ERR_OK;
    uint32 i = 0;
    uint8 patch_type = 0;
    rt_phy_patch_db_t *pPatchDb = NULL;
    rtk_hwpatch_seq_t *table = NULL;

    PHYPATCH_DB_GET(unit, port, pPatchDb);

    if ((pPatchDb == NULL) || (pPatchDb->fPatch_op == NULL) || (pPatchDb->fPatch_flow == NULL))
    {
        RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u phy_patch, db is NULL\n", unit, port);
        return RT_ERR_DRIVER_NOT_SUPPORTED;
    }

    if (patch_mode == PHY_PATCH_MODE_CMP)
    {
        table = pPatchDb->cmp_table;
    }
    else
    {
        table = pPatchDb->seq_table;
    }
    RT_LOG(LOG_INFO, (MOD_HAL | MOD_PHY), "phy_patch: U%u P%u portOffset:%u  patch_mode:%u\n", unit, port, portOffset, patch_mode);

    for (i = 0; i < RTK_PATCH_SEQ_MAX; i++)
    {
        patch_type = table[i].patch_type;
        RT_LOG(LOG_INFO, (MOD_HAL | MOD_PHY), "phy_patch: table[%u] patch_type:%u\n", i, patch_type);

        if (RTK_PATCH_TYPE_IS_DATA(patch_type))
        {
            ret = _phy_patch_process(unit, port, portOffset, table[i].patch.data.conf, table[i].patch.data.size, patch_mode);

            if (ret == RT_ERR_CHECK_FAILED)
                chk_ret = ret;
            else if (ret  != RT_ERR_OK)
            {
                RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u patch_mode:%u id:%u patch-%u failed. ret:0x%X\n", unit, port, patch_mode, i, patch_type, ret);
                return ret;
            }
        }
        else if (RTK_PATCH_TYPE_IS_FLOW(patch_type))
        {
            RT_ERR_CHK_EHDL(pPatchDb->fPatch_flow(unit, port, portOffset, table[i].patch.flow_id, patch_mode),
                            ret, RT_LOG(LOG_MAJOR_ERR, (MOD_HAL | MOD_PHY), "U%u P%u patch_mode:%u id:%u patch-%u failed. ret:0x%X\n", unit, port, patch_mode, i, patch_type, ret););
        }
        else
        {
            break;
        }
    }

    return (chk_ret == RT_ERR_CHECK_FAILED) ? chk_ret : RT_ERR_OK;
}




