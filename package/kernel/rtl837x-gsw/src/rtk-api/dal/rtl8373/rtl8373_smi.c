/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
*
* Program : Control smi connected RTL8366
* Abstract :
* Author : Yu-Mei Pan (ympan@realtek.com.cn)
*  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
*/
#include <rtk_types.h>
#include <rtl8373_smi.h>
#include "rtk_error.h"

// we need refactoring here to avoid global variable
struct regmap *gsw_regmap;

rtk_int32 rtl8373_smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
    rtk_int32 ret;
	ret = regmap_read(gsw_regmap, mAddrs, rData);
    // printk("rtl8373_smi_read ret:%d\n",ret);
    return ret;
}

rtk_int32 rtl8373_smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
    rtk_int32 ret;
	ret = regmap_write(gsw_regmap, mAddrs, rData);
    // printk("rtl8373_smi_write ret:%d\n",ret);
    return ret;
}
