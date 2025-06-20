/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include "rtk_phylib_rtl826xb.h"

/* Indirect Register Access APIs */
int rtk_phylib_826xb_sds_read(rtk_phydev *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData)
{
    int32  ret = 0;
    uint32 rData = 0;
    uint32 op = (page & 0x3f) | ((reg & 0x1f) << 6) | (0x8000);
    uint32 i = 0;
    uint32 mask = 0;
    mask = UINT32_BITS_MASK(msb,lsb);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 323, 15, 0, op));

    for (i = 0; i < 10; i++)
    {
        RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 30, 323, 15, 15, &rData));
        if (rData == 0)
        {
            break;
        }
        rtk_phylib_udelay(10);
    }
    if (i == 10)
    {
        return -1;
    }

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_read(phydev, 30, 322, 15, 0, &rData));
    *pData = REG32_FIELD_GET(rData, lsb, mask);

    return ret;
}

int rtk_phylib_826xb_sds_write(rtk_phydev *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 data)
{
    int32  ret = 0;
    uint32 wData = 0, rData = 0;
    uint32 op = (page & 0x3f) | ((reg & 0x1f) << 6) | (0x8800);
    uint32 mask = 0;
    mask = UINT32_BITS_MASK(msb,lsb);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_826xb_sds_read(phydev, page, reg, 15, 0, &rData));

    wData = REG32_FIELD_SET(rData, data, lsb, mask);

    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 321, 15, 0, wData));
    RTK_PHYLIB_ERR_CHK(rtk_phylib_mmd_write(phydev, 30, 323, 15, 0, op));

    return ret;
}
