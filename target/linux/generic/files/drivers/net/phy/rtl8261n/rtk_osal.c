/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include "type.h"
#include "error.h"
#include "rtk_phylib_def.h"

#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/sched/signal.h>
#include <linux/phy.h>

int32
osal_time_usecs_get(osal_usecs_t *pUsec)
{
    struct timespec64 ts;

    RT_PARAM_CHK((NULL == pUsec), RT_ERR_NULL_POINTER);

    ktime_get_ts64(&ts);
    *pUsec = (osal_usecs_t)((ts.tv_sec * USEC_PER_SEC) + (ts.tv_nsec / NSEC_PER_USEC));
    return RT_ERR_OK;
}

void *
osal_alloc(uint32 size)
{
    void *p;
    p = kmalloc((size_t)size, GFP_ATOMIC);
    return p;
}

int32
phy_common_general_reg_mmd_get(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 *pData)
{
    int32 rData = 0;
    rData = phy_read_mmd(port, mmdAddr, mmdReg);
    if (rData < 0)
        return RT_ERR_FAILED;
    *pData = (uint32)rData;
    return RT_ERR_OK;
}

int32
phy_common_general_reg_mmd_set(uint32 unit, rtk_port_t port, uint32 mmdAddr, uint32 mmdReg, uint32 data)
{
    int ret = phy_write_mmd(port, mmdAddr, mmdReg, data);
    return (ret < 0) ? RT_ERR_FAILED : RT_ERR_OK;
}
