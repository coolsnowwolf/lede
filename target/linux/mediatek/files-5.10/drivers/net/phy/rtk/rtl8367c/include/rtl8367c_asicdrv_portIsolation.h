/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Port isolation related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_PORTISOLATION_H_
#define _RTL8367C_ASICDRV_PORTISOLATION_H_

#include <rtl8367c_asicdrv.h>

extern ret_t rtl8367c_setAsicPortIsolationPermittedPortmask(rtk_uint32 port, rtk_uint32 permitPortmask);
extern ret_t rtl8367c_getAsicPortIsolationPermittedPortmask(rtk_uint32 port, rtk_uint32 *pPermitPortmask);
extern ret_t rtl8367c_setAsicPortIsolationEfid(rtk_uint32 port, rtk_uint32 efid);
extern ret_t rtl8367c_getAsicPortIsolationEfid(rtk_uint32 port, rtk_uint32 *pEfid);

#endif /*_RTL8367C_ASICDRV_PORTISOLATION_H_*/
