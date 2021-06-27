/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 48989 $
 * $Date: 2014-07-01 15:45:24 +0800 (¶g¤G, 01 ¤C¤ë 2014) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367C
 * Feature :
 *
 */

#ifndef _RTL8367C_ASICDRV_EEE_H_
#define _RTL8367C_ASICDRV_EEE_H_

#include <rtl8367c_asicdrv.h>

#define EEE_OCP_PHY_ADDR    (0xA5D0)

extern ret_t rtl8367c_setAsicEee100M(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicEee100M(rtk_uint32 port, rtk_uint32 *enable);
extern ret_t rtl8367c_setAsicEeeGiga(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicEeeGiga(rtk_uint32 port, rtk_uint32 *enable);


#endif /*_RTL8367C_ASICDRV_EEE_H_*/
