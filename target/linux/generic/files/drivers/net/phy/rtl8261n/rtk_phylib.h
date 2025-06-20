/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __RTK_PHYLIB_H
#define __RTK_PHYLIB_H

#if defined(RTK_PHYDRV_IN_LINUX)
  #include "type.h"
  #include "rtk_phylib_def.h"
#else
  //#include SDK headers
#endif

#if defined(RTK_PHYDRV_IN_LINUX)
  #define PR_INFO(_fmt, _args...) pr_info(_fmt, ##_args)
  #define PR_DBG(_fmt, _args...)  pr_debug(_fmt, ##_args)
  #define PR_ERR(_fmt, _args...)  pr_err("ERROR: "_fmt, ##_args)

  #define RTK_PHYLIB_ERR_FAILED             (-EPERM)
  #define RTK_PHYLIB_ERR_INPUT              (-EINVAL)
  #define RTK_PHYLIB_ERR_EXCEEDS_CAPACITY   (-ENOSPC)
  #define RTK_PHYLIB_ERR_TIMEOUT            (-ETIME)
  #define RTK_PHYLIB_ERR_ENTRY_NOTFOUND     (-ENODATA)
#else
  #define PR_INFO(_fmt, _args...) RT_LOG(LOG_INFO, (MOD_HAL|MOD_PHY), _fmt, ##_args)
  #define PR_DBG(_fmt, _args...)  RT_LOG(LOG_DEBUG, (MOD_HAL|MOD_PHY), _fmt, ##_args)
  #define PR_ERR(_fmt, _args...)  RT_LOG(LOG_MAJOR_ERR, (MOD_HAL|MOD_PHY), _fmt, ##_args)

  #define RTK_PHYLIB_ERR_FAILED              (RT_ERR_FAILED)
  #define RTK_PHYLIB_ERR_INPUT               (RT_ERR_INPUT)
  #define RTK_PHYLIB_ERR_EXCEEDS_CAPACITY    (RT_ERR_EXCEEDS_CAPACITY)
  #define RTK_PHYLIB_ERR_TIMEOUT             (RT_ERR_BUSYWAIT_TIMEOUT)
  #define RTK_PHYLIB_ERR_ENTRY_NOTFOUND      (RT_ERR_ENTRY_NOTFOUND)
#endif

typedef enum rtk_phylib_phy_e
{
    RTK_PHYLIB_NONE,
    RTK_PHYLIB_RTL8261N,
    RTK_PHYLIB_RTL8264B,
    RTK_PHYLIB_END
} rtk_phylib_phy_t;

struct rtk_phy_priv {
    rtk_phylib_phy_t phytype;
    uint8 isBasePort;
    rt_phy_patch_db_t *patch;
};

#if defined(RTK_PHYDRV_IN_LINUX)
    typedef struct phy_device rtk_phydev;
#else
    struct rtk_phy_dev_s
    {
        uint32 unit;
        rtk_port_t port;

        struct rtk_phy_priv *priv;
    };
    typedef struct rtk_phy_dev_s rtk_phydev;
#endif

#define RTK_PHYLIB_ERR_CHK(op)\
do {\
    if ((ret = (op)) != 0)\
        return ret;\
} while(0)

#define RTK_PHYLIB_VAL_TO_BYTE_ARRAY(_val, _valbytes, _array, _start, _bytes)\
do{\
    uint32 _i = 0;\
    for (_i = 0; _i < _bytes; _i++)\
        _array[_start+_i] = (_val >> (8* (_valbytes - _i - 1)));\
}while(0)

#define RTK_PHYLIB_BYTE_ARRAY_TO_VAL(_val, _array, _start, _bytes)\
do{\
    uint32 _i = 0;\
    for (_i = 0; _i < _bytes; _i++)\
        _val = (_val << 8) | _array[_start + _i];\
}while(0)


/* OSAL */
void rtk_phylib_mdelay(uint32 msec);
void rtk_phylib_udelay(uint32 usec);

/* Register Access APIs */
int32 rtk_phylib_mmd_write(rtk_phydev *phydev, uint32 mmd, uint32 reg, uint8 msb, uint8 lsb, uint32 data);
int32 rtk_phylib_mmd_read(rtk_phydev *phydev, uint32 mmd, uint32 reg, uint8 msb, uint8 lsb, uint32 *pData);

/* Function Driver */
int32 rtk_phylib_c45_power_normal(rtk_phydev *phydev);
int32 rtk_phylib_c45_power_low(rtk_phydev *phydev);
int32 rtk_phylib_c45_pcs_loopback(rtk_phydev *phydev, uint32 enable);


#endif /* __RTK_PHYLIB_H */
