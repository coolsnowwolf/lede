/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __HAL_PHY_PHY_RTL826XB_PATCH_H__
#define __HAL_PHY_PHY_RTL826XB_PATCH_H__

/*
 * Include Files
 */
#if defined(RTK_PHYDRV_IN_LINUX)
  #include "rtk_osal.h"
  #include "rtk_phylib_def.h"
#else
  #include <common/rt_type.h>
  #include <rtk/port.h>
#endif

/* Function Name:
 *      phy_rtl826xb_patch
 * Description:
 *      apply patch data to PHY
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
extern int32 phy_rtl826xb_patch(uint32 unit, rtk_port_t baseport, uint8 portOffset);

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
extern int32 phy_rtl826xb_broadcast_patch(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 perChip);

extern int32 phy_rtl826xb_patch_db_init(uint32 unit, rtk_port_t port, rt_phy_patch_db_t **pPhy_patchDb);
#endif /* __HAL_PHY_PHY_RTL826XB_PATCH_H__ */
