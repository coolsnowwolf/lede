/****************************************************************************
*
*****************************************************************************/


#ifndef _gc_hal_kernel_platform_ftg_h_
#define _gc_hal_kernel_platform_ftg_h_

#if gcdSUPPORT_DEVICE_TREE_SOURCE
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include "gc_hal.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel.h"
#include "gc_hal_kernel_platform.h"

typedef struct _gcsPOWER_DOMAIN {
    struct generic_pm_domain base;
    gceCORE                  core_id;
    gctUINT32                flags;
    struct platform_device  *pdev;
} gcsPOWER_DOMAIN;

/* register interface with SE*/
#define AP_CPPC1_STAT		0
#define AP_CPPC1_SET		0x4
#define AP_CPPC1_DATA0		0x18
#define AP_CPPC1_DATA1		0x1c

#define FT_SE_POLL_INTERVAL	10
#define FT_SE_TIMEOUT		30000

enum ftg_gpu_se_cmd_id {
    FTG_GPU_SE_CMD_POWER_ON_ALL,
    FTG_GPU_SE_CMD_POWER_OFF_ALL,
    FTG_GPU_SE_CMD_POWER_ON_CLUSTER,
    FTG_GPU_SE_CMD_POWER_OFF_CLUSTER,
    FTG_GPU_SE_CMD_GET_OPP_COUNT,
    FTG_GPU_SE_CMD_GET_OPP_FREQ,
    FTG_GPU_SE_CMD_GET_OPP_VOLT,
    FTG_GPU_SE_CMD_SET_OPP,
};

struct ftg_gpu_power_ctl {
    void __iomem *power_reg_base;
    struct mutex mutex;
};

#endif
#endif
