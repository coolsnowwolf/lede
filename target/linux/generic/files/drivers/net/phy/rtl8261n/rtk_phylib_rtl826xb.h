/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_RTL826XB_H
#define __RTK_PHYLIB_RTL826XB_H

#if defined(RTK_PHYDRV_IN_LINUX)
  #include "rtk_phylib.h"
#else
  //#include SDK headers
#endif

int rtk_phylib_826xb_sds_read(rtk_phydev *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData);
int rtk_phylib_826xb_sds_write(rtk_phydev *phydev, uint32 page, uint32 reg, uint8 msb, uint8 lsb, uint32 data);

#endif /* __RTK_PHYLIB_RTL826XB_H */
