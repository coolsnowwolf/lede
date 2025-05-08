/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76333 $
 * $Date: 2017-03-09 09:33:15 +0800 (¶g¥|, 09 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch MII access
 * Feature : MII access functions
 *
 */

#ifndef _RTL8367C_ASICDRV_MII_MGR_H_
#define _RTL8367C_ASICDRV_MII_MGR_H_

#define u32      unsigned int
extern u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

#endif /*_RTL8367C_ASICDRV_MII_MGR_H_*/

