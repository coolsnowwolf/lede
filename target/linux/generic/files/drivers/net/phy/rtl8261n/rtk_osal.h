/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHY_OSAL_H
#define __RTK_PHY_OSAL_H

#include <linux/kernel.h>
#include <linux/phy.h>
#include "type.h"
#include "error.h"
#include "phy_patch.h"
#include "rtk_phylib.h"

#ifdef PHYPATCH_DB_GET
    #undef PHYPATCH_DB_GET
#endif

#define PHYPATCH_DB_GET(_unit, _pPhy_device, _pPatchDb) \
    do { \
        struct rtk_phy_priv *_pPriv = (_pPhy_device)->priv; \
        rt_phy_patch_db_t *_pDb = _pPriv->patch; _pPatchDb = _pDb; \
        /*printk("[PHYPATCH_DB_GET] ? [%s]\n", (_pDb != NULL) ? "E":"N");*/ \
    } while(0)

#define HWP_9300_FAMILY_ID(_unit)       0
#define HWP_9310_FAMILY_ID(_unit)       0
#define RTK_9300_FAMILY_ID(_unit)       0
#define RTK_9310_FAMILY_ID(_unit)       0
#define RTK_9311B_FAMILY_ID(_unit)      0
#define RTK_9330_FAMILY_ID(_unit)       0

#ifndef WAIT_COMPLETE_VAR
#define WAIT_COMPLETE_VAR() \
    osal_usecs_t    _t, _now, _t_wait=0, _timeout;  \
    int32           _chkCnt=0;

#define WAIT_COMPLETE(_timeout_us)     \
    _timeout = _timeout_us;  \
    for(osal_time_usecs_get(&_t),osal_time_usecs_get(&_now),_t_wait=0,_chkCnt=0 ; \
        (_t_wait <= _timeout); \
        osal_time_usecs_get(&_now), _chkCnt++, _t_wait += ((_now >= _t) ? (_now - _t) : (0xFFFFFFFF - _t + _now)),_t = _now \
       )

#define WAIT_COMPLETE_IS_TIMEOUT()   (_t_wait > _timeout)
#endif

/* OSAL */
#include <linux/slab.h>
int32 osal_time_usecs_get(osal_usecs_t *pUsec);
void *osal_alloc(uint32 size);
#define osal_time_mdelay  mdelay

#include <linux/ctype.h>    /* for Kernel Space */
#include <linux/kernel.h>
#include <linux/string.h>
#define osal_strlen   strlen
#define osal_strcmp   strcmp
#define osal_strcpy   strcpy
#define osal_strncpy  strncpy
#define osal_strcat   strcat
#define osal_strchr   strchr
#define osal_memset   memset
#define osal_memcpy   memcpy
#define osal_memcmp   memcmp
#define osal_strdup   strdup
#define osal_strncmp  strncmp
#define osal_strstr   strstr
#define osal_strtok   strtok
#define osal_strtok_r   strtok_r
#define osal_toupper  toupper

#define osal_printf   printk

/* HWP */
#define HWP_PORT_SMI(unit, port)               0
#define HWP_PHY_MODEL_BY_PORT(unit, port)      0
#define HWP_PHY_ADDR(unit, port)               0
#define HWP_PHY_BASE_MACID(unit, p)            0
#define HWP_PORT_TRAVS_EXCEPT_CPU(unit, p)     if (bcast_phyad < 0x1F && p != NULL)


/* RT_LOG */
//#define RT_LOG(level, module, fmt, args...)               do { printk("RT_LOG:"fmt, ## args); } while(0)
#define RT_LOG(level, module, fmt, args...)               do {} while(0)
#define RT_ERR(error_code, module, fmt, args...)          do {} while(0)
#define RT_INIT_ERR(error_code, module, fmt, args...)     do {} while(0)
#define RT_INIT_MSG(fmt, args...)                         do {} while(0)

#define phy_826xb_ctrl_set(unit, p, RTK_PHY_CTRL_MIIM_BCAST_PHYAD, bcast_phyad)  0

/* reg access */
int32 phy_common_general_reg_mmd_get(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 *pData);
int32 phy_common_general_reg_mmd_set(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 data);


#endif /* __RTK_PHY_OSAL_H */
