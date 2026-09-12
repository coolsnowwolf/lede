/****************************************************************************
*
*
*****************************************************************************/


/*
 *   dts node example:
 *   gpu_3d: gpu@53100000 {
 *       compatible = "phytium,ftg340";
 *       reg = <0 0x53100000 0 0x40000>,
 *               <0 0x54100000 0 0x40000>;
 *       reg-names = "core_major", "core_3d1";
 *       interrupts = <GIC_SPI 64 IRQ_TYPE_LEVEL_HIGH>,
 *                   <GIC_SPI 65 IRQ_TYPE_LEVEL_HIGH>;
 *       interrupt-names = "core_major", "core_3d1";
 *       clocks = <&clk IMX_SC_R_GPU_0_PID0 IMX_SC_PM_CLK_PER>,
 *               <&clk IMX_SC_R_GPU_0_PID0 IMX_SC_PM_CLK_MISC>,
 *               <&clk IMX_SC_R_GPU_1_PID0 IMX_SC_PM_CLK_PER>,
 *               <&clk IMX_SC_R_GPU_1_PID0 IMX_SC_PM_CLK_MISC>;
 *       clock-names = "core_major", "core_major_sh", "core_3d1", "core_3d1_sh";
 *       assigned-clocks = <&clk IMX_SC_R_GPU_0_PID0 IMX_SC_PM_CLK_PER>,
 *                       <&clk IMX_SC_R_GPU_0_PID0 IMX_SC_PM_CLK_MISC>,
 *                       <&clk IMX_SC_R_GPU_1_PID0 IMX_SC_PM_CLK_PER>,
 *                       <&clk IMX_SC_R_GPU_1_PID0 IMX_SC_PM_CLK_MISC>;
 *       assigned-clock-rates = <700000000>, <850000000>, <800000000>, <1000000000>;
 *       power-domains = <&pd IMX_SC_R_GPU_0_PID0>, <&pd IMX_SC_R_GPU_1_PID0>;
 *       power-domain-names = "core_major", "core_3d1";
 *       contiguous-base = <0x0>;
 *       contiguous-size = <0x1000000>;
 *       status = "okay";
 *   };
 */

#include <linux/iopoll.h>

#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel_platform.h"
#include "gc_hal_kernel_platform_ftg.h"
#if gcdSUPPORT_DEVICE_TREE_SOURCE
# include <linux/pm_runtime.h>
# include <linux/pm_domain.h>
# include <linux/clk.h>

struct ftg_gpu_power_ctl ftg_gpu_power_controller;
#elif gcdSUPPORT_DEVICE_ACPI
#endif

gceSTATUS
ft_AdjustParam(gcsPLATFORM *Platform, gcsMODULE_PARAMETERS *Args);

gceSTATUS
ft_set_power(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE GPU, gctBOOL Enable);

gceSTATUS
ft_set_clock(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE GPU, gctBOOL Enable);

gceSTATUS
ft_setClustersPower(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE core, gctUINT32 clusterMask);

gceSTATUS
_GetGPUPhysical(gcsPLATFORM *Platform, gctPHYS_ADDR_T CPUPhysical, gctPHYS_ADDR_T *GPUPhysical);

static struct _gcsPLATFORM_OPERATIONS ftg_ops = {
    .adjustParam        = ft_AdjustParam,
    .setPower           = ft_set_power,
    .setClock           = ft_set_clock,
    .getGPUPhysical     = _GetGPUPhysical,
    .setClustersPower 	= ft_setClustersPower,
};

static struct _gcsPLATFORM ftg_platform = {
    .name = __FILE__,
    .ops = &ftg_ops,
};

const char *core_names[] = {
    "core_major",
    "core_3d1",
    "core_3d2",
    "core_3d3",
    "core_3d4",
    "core_3d5",
    "core_3d6",
    "core_3d7",
    "core_3d8",
    "core_3d9",
    "core_3d10",
    "core_3d11",
    "core_3d12",
    "core_3d13",
    "core_3d14",
    "core_3d15",
    "core_2d",
    "core_2d1",
    "core_2d2",
    "core_2d3",
    "core_vg",
#if gcdDEC_ENABLE_AHB
    "core_dec",
#endif
};

#if gcdSUPPORT_DEVICE_TREE_SOURCE
int ftg_gpu_se_send_cmd(gcsPLATFORM *Platform,enum ftg_gpu_se_cmd_id cmd, u32* data_in, u32* data_out)
{
    int res;
    u32 cppc_status,cppc_data0;
    void* reg_base=ftg_gpu_power_controller.power_reg_base;
    u32 cmd_data0, cmd_data1;
    u8 payload0;
    u32 payload1;
    u64 cmd_time_start,cmd_time_end;
    u32  opp_num;

    gcmkASSERT(reg_base != NULL);

    pr_info("Send cmd from GPU to SE: %d", cmd);

    cmd_time_start = ktime_get();

    switch (cmd) {
    case FTG_GPU_SE_CMD_POWER_ON_ALL:
        cmd_data0 = 0x110400;
        cmd_data1 = 0x40000008;
        break;

    case FTG_GPU_SE_CMD_POWER_OFF_ALL:
        cmd_data0 = 0x110400;
        cmd_data1 = 0x40000000;
        break;
    
    case FTG_GPU_SE_CMD_POWER_ON_CLUSTER:
        cmd_data0 = 0x110400;
        cmd_data1 = 0x40000008; /* TODO:*/
        break;

    case FTG_GPU_SE_CMD_POWER_OFF_CLUSTER:
        cmd_data0 = 0x110400;
        cmd_data1 = 0x40000000; /* TODO:*/
        break;

    case FTG_GPU_SE_CMD_GET_OPP_COUNT:
        cmd_data0 = 0x140300;  /*messaage id:03  protocol id: 0x14*/
        cmd_data1 = 0;
        break;

    case FTG_GPU_SE_CMD_GET_OPP_FREQ:
        opp_num =*data_in;
        cmd_data0 = 0x140400 | (opp_num << 24);  /*messaage id:04  protocol id: 0x14*/
        cmd_data1 = 0;
        break;

    case FTG_GPU_SE_CMD_GET_OPP_VOLT:
        cmd_data0 = 0x140000;  /* TODO:messaage id:0x?  protocol id: 0x14*/
        cmd_data1 = 0;
        break;

    case FTG_GPU_SE_CMD_SET_OPP:
        cmd_data0 = 0x140500; /*mesage id: 0x5 protocol id: 0x14 */
        cmd_data1 = *data_in;
        break;

    default:
        pr_err("gpu->se:invald command");
        return -1;
    }

    /*acquire gpu power controller lock */
    mutex_lock(&ftg_gpu_power_controller.mutex);

    /* wait cppc status to be 0 */    
    res = readl_relaxed_poll_timeout(reg_base + AP_CPPC1_STAT, cppc_status, 
                (cppc_status==0), FT_SE_POLL_INTERVAL, FT_SE_TIMEOUT);
    if (res == -ETIMEDOUT ) {
        pr_err("%s %d, timeout read cppc stat: %x\n", __func__, __LINE__, cppc_status);
        mutex_unlock(&ftg_gpu_power_controller.mutex);
        return -1;
    }

    /* wait channel to be free*/
    res = readl_relaxed_poll_timeout(reg_base + AP_CPPC1_DATA0, cppc_data0, 
                (cppc_data0 & 0x1), FT_SE_POLL_INTERVAL, FT_SE_TIMEOUT);
    if (res == -ETIMEDOUT ) {
        pr_err("%s %d, timeout read channel stats: %x\n", __func__, __LINE__, cppc_data0);
        mutex_unlock(&ftg_gpu_power_controller.mutex);
        return -1;
    }

    /* popluate cmd date in shared memory, and mark channel status to 1*/
    writel_relaxed(cmd_data1, reg_base + AP_CPPC1_DATA1);
    writel_relaxed(cmd_data0, reg_base + AP_CPPC1_DATA0);

    /* trigger SE interrupt */
    writel_relaxed(0x1, reg_base + AP_CPPC1_SET);

    /* wait SE to finish the cmd*/
    res = readl_relaxed_poll_timeout(reg_base + AP_CPPC1_DATA0, cppc_data0, 
                (cppc_data0 & 0x1), FT_SE_POLL_INTERVAL, FT_SE_TIMEOUT);
    if (res == -ETIMEDOUT ) {
        pr_err("%s %d, timeout wait for se cmd finish: %x\n", 
            __func__, __LINE__, cppc_data0);
        return -1;
    }

    /*command post process*/
    payload0 = readl_relaxed(reg_base + AP_CPPC1_DATA0) >> 24;
    payload1 = readl_relaxed(reg_base + AP_CPPC1_DATA1);
    
    /* release gpu power controller lock*/
    mutex_unlock(&ftg_gpu_power_controller.mutex);

    cmd_time_end = ktime_get();
    printk(KERN_DEBUG "gpu_se_command %d processed in %lld usecs\n", cmd,
        ktime_us_delta(cmd_time_end,cmd_time_start));

    switch (cmd) {
    case FTG_GPU_SE_CMD_POWER_ON_ALL:
        if (payload0) {
            pr_err(" error to power on gpu\n");
            return -1;
        }
        break;

    case FTG_GPU_SE_CMD_POWER_OFF_ALL:
        if (payload0) {
            pr_err(" error to power off gpu\n");
            return -1;
        }
        break;
     case FTG_GPU_SE_CMD_GET_OPP_COUNT:
        if (payload0) {
            pr_err(" error to get opp count of gpu\n");
            return -1;
        } else if (data_out != NULL){
            *data_out = payload1;
        }
        break;

    case FTG_GPU_SE_CMD_GET_OPP_FREQ:
        if (payload0) {
            pr_err(" error to get gpu freq @opp %d\n", (u8)*data_in);
            return -1;
        } else if (data_out != NULL){
            *data_out = payload1;
        }
        break;

    case FTG_GPU_SE_CMD_GET_OPP_VOLT:
        if (payload0) {
            pr_err(" error to get gpu voltage @opp %d\n", (u8)*data_in);
            return -1;
        } else if (data_out != NULL){
            *data_out = payload1;
        }
        break; 

    case FTG_GPU_SE_CMD_SET_OPP:
        if (payload0) {
            pr_err(" error to set gpu opp %d\n", (u32)*data_in);
            return -1;
        }
        break; 

    default:
        pr_err("gpu invalid cmd");
        return -1;
    }

    return 0;
}
#endif

gceSTATUS
ft_set_clock(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE GPU, gctBOOL Enable)
{
    /* TODO
        There are two clk for GPU : core , shader.
        Can we set these two clk independently, and how?
    */
    return gcvSTATUS_OK;
}

gceSTATUS
_GetGPUPhysical(gcsPLATFORM *Platform, gctPHYS_ADDR_T CPUPhysical, gctPHYS_ADDR_T *GPUPhysical)
{

    *GPUPhysical = CPUPhysical;

    return gcvSTATUS_OK;
}

#if gcdSUPPORT_DEVICE_ACPI
gctUINT32 curCluster = 0;

gceSTATUS
ft_setClustersPower(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE core, gctUINT32 clusterMask)
{
	gceSTATUS status = gcvSTATUS_OK;
	struct device *dev = &Platform->device->dev;

	if (clusterMask == curCluster)
	{
		return gcvSTATUS_OK;
	}

	if (clusterMask == 0x1) {
		if (curCluster == 0x0) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x1, NULL);
		} else if (curCluster == 0x3) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x2, NULL);
		} else if (curCluster == 0x7) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x6, NULL);
		} else if (curCluster == 0xf) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0xe, NULL);
		}
	}

	if (clusterMask == 0x3) {
		if (curCluster == 0x1) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x2, NULL);
		} else if (curCluster == 0x0) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x3, NULL);
		} else if (curCluster == 0x7) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x4, NULL);
		} else if (curCluster == 0xf) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0xc, NULL);
		}
	}

	if (clusterMask == 0x7) {
		if (curCluster == 0x1) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x6, NULL);
		} else if (curCluster == 0x3) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x4, NULL);
		} else if (curCluster == 0x0) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x7, NULL);
		} else if (curCluster == 0xf) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x8, NULL);
		}
	}

	if (clusterMask == 0xf) {
		if (curCluster == 0x1) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0xe, NULL);
		} else if (curCluster == 0x3) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0xc, NULL);
		} else if (curCluster == 0x7) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0x8, NULL);
		} else if (curCluster == 0x0) {
			status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, 0xf, NULL);
		}
	}

	if (clusterMask == 0x0) {
		if (curCluster == 0x1) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x1, NULL);
		} else if (curCluster == 0x3) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x3, NULL);
		} else if (curCluster == 0x7) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0x7, NULL);
		} else if (curCluster == 0xf) {
			status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, 0xf, NULL);
		}
	}

	curCluster = clusterMask;

	return status;
}

gceSTATUS
ft_set_power(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE GPU, gctBOOL Enable)
{
	gceSTATUS status = gcvSTATUS_OK;
	struct device *dev = &Platform->device->dev;
	gctUINT userclusterMasks = 0;
    
	userclusterMasks = Platform->params.userClusterMasks[0];

	if (Enable)
		status = gpu_get_data_from_acpi(dev, "PPWO", FT_3D, userclusterMasks, NULL);
	else
		status = gpu_get_data_from_acpi(dev, "PPWD", FT_3D, userclusterMasks, NULL);

	curCluster = userclusterMasks;
	dev_info(dev, "TODO:set gpu power:%d,userClusterMasks = %d,---\n", Enable, Platform->params.userClusterMasks[0]);

	return status;
}
#endif

#if gcdSUPPORT_DEVICE_TREE_SOURCE
gceSTATUS
ft_setClustersPower(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE core, gctUINT32 clusterMask)
{
	return 0;
}

gceSTATUS
ft_set_power(gcsPLATFORM *Platform, gctUINT32 DevIndex, gceCORE GPU, gctBOOL Enable)
{
    int rc;

    pr_err("TODO:set gpu power:%d---\n",Enable);
    
    if (Enable) {
        rc = ftg_gpu_se_send_cmd(Platform,FTG_GPU_SE_CMD_POWER_ON_ALL, NULL, NULL);
    } else {
        rc = ftg_gpu_se_send_cmd(Platform,FTG_GPU_SE_CMD_POWER_OFF_ALL, NULL,NULL);
    }

    if(rc) {
        pr_err("%s %d set gpu power failed", __func__, __LINE__);
        return gcvSTATUS_GENERIC_IO;
    }

    return gcvSTATUS_OK;
}
#endif

#if gcdSUPPORT_DEVICE_ACPI
static int gpu_parse_acpi(struct platform_device *pdev, gcsMODULE_PARAMETERS *params)
{
    struct resource *res;
    gctUINT32 i;
    int irqs_debug;
    struct device *dev = &pdev->dev;

    if (has_acpi_companion(dev)) {
		acpi_dma_configure(dev, DEV_DMA_COHERENT);
		dma_set_mask(dev, DMA_BIT_MASK(40));
		dma_set_coherent_mask(dev, DMA_BIT_MASK(40));
    } else {
    	pr_err("failed to get acpi device\n");
    }

    for (i = 0; i < gcvCORE_COUNT; i++) {
        irqs_debug = platform_get_irq(pdev, i);
        if (irqs_debug > 0)
        {
            params->irqs[i] = irqs_debug;
            break;
        }
        else
            gcmkPRINT("-1,,,,");
    }
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res) {
        params->registerBases[0] = res->start;
        params->registerSizes[0] = res->end - res->start + 1;
    } else {
        gcmkPRINT("failed to parse the register config by platform_get_resource \n ");
    }
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (res) {
        params->contiguousBases[0] = res->start;
        params->contiguousSizes[0] = res->end - res->start + 1;
    } else {
        gcmkPRINT("failed to parse the contiguous mem by platform_get_resource \n ");
    }
    
    return 0;
}
#endif
#if gcdSUPPORT_DEVICE_TREE_SOURCE
static int gpu_parse_dt(struct platform_device *pdev, gcsMODULE_PARAMETERS *params)
{
    struct resource *res;
    gctUINT32 i;
    int irq_debug;

        for (i = 0; i < 1/*gcvCORE_COUNT*/; i++) {
        res = platform_get_resource_byname(pdev, IORESOURCE_IRQ, core_names[i]);
        if (res){
            gcmkPRINT("platform_get_resource_byname ok : %lld,", res->start);
            params->irqs[i] = res->start;
            //break;
        }else{
            gcmkPRINT("-1,");
            res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
            if (res){
                gcmkPRINT("platform_get_resource ok : %lld,", res->start);
                params->irqs[i] = res->start;
                //break;
            }
            else{
                gcmkPRINT("-1,,");
                irq_debug = platform_get_irq_byname(pdev, core_names[i]);
                if (irq_debug > 0){
                    gcmkPRINT("platform_get_irq_byname ok : %lld,", irq_debug);
                    params->irqs[i] = irq_debug;
                    //break;
                }
                else{
                    gcmkPRINT("-1,,,");
                    irq_debug = platform_get_irq(pdev, i);
                    if (irq_debug > 0){
                        gcmkPRINT("platform_get_irq ok : %lld,", irq_debug);
                        params->irqs[i] = irq_debug;
                        //break;
                    }
                    else{
                        gcmkPRINT("-1,,,,");
                    }
                }
            }
        }
    }

       for (i = 0; i < 1 /*gcvCORE_COUNT*/; i++) {
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, core_names[i]);
        if (res) {
            params->registerBases[i] = res->start;
            params->registerSizes[i] = res->end - res->start + 1;
            gcmkPRINT("parse the register config by platform_get_resource_byname ok\n ");
            gcmkPRINT("registerBases = %lld, registerSizes = %lld\n", params->registerBases[i], params->registerSizes[i]);
            //break;
        }else{
            res = platform_get_resource(pdev, IORESOURCE_MEM, i);
            if (res){
                params->registerBases[i] = res->start;
                params->registerSizes[i] = res->end - res->start + 1;
                gcmkPRINT("parse the register config by platform_get_resource ok\n ");
                gcmkPRINT("registerBases = %lld, registerSizes = %lld\n", params->registerBases[i], params->registerSizes[i]);
                //break;
            }
            else{
                gcmkPRINT("failed to parse the register config by platform_get_resource or platform_get_resource_byname\n ");
                //break;
            }
        }
    }
 
    return 0;
}
#endif

#if gcdSUPPORT_DEVICE_ACPI
static const struct acpi_device_id ftg_acpi_ids[] = {
	{ .id = "PHYT0048", },
	{},
};
MODULE_DEVICE_TABLE(acpi, ftg_acpi_ids);
#endif

#if gcdSUPPORT_DEVICE_TREE_SOURCE
static const struct of_device_id gpu_dt_ids[] = {
    { .compatible = "phytium,ftg340", },

    { /* sentinel */ }
};
#endif

gceSTATUS
ft_AdjustParam(gcsPLATFORM *Platform, gcsMODULE_PARAMETERS *Args)
{
#if gcdSUPPORT_DEVICE_TREE_SOURCE
    gpu_parse_dt(Platform->device, Args);
#endif
#if gcdSUPPORT_DEVICE_ACPI
    gpu_parse_acpi(Platform->device, Args);
#endif

    return gcvSTATUS_OK;
}


int gckPLATFORM_Init(struct platform_driver *pdrv, struct _gcsPLATFORM **platform)
{
    int ret = 0;

#if gcdSUPPORT_DEVICE_ACPI
    pdrv->driver.acpi_match_table = ftg_acpi_ids;
#endif

#if gcdSUPPORT_DEVICE_TREE_SOURCE
    pdrv->driver.of_match_table = gpu_dt_ids;
    /*For now ,we simply ioremap gpu power controller channel.*/
    ftg_gpu_power_controller.power_reg_base = ioremap(0x26fcc040 /*GPU_3D base*/, SZ_32);

    mutex_init(&ftg_gpu_power_controller.mutex);
#endif

    *platform = (gcsPLATFORM *)&ftg_platform;

    return ret;
}

int gckPLATFORM_Terminate(struct _gcsPLATFORM *platform)
{
#if gcdSUPPORT_DEVICE_TREE_SOURCE
    iounmap(ftg_gpu_power_controller.power_reg_base);
    pr_err("%s %d ftg platform exit\n", __func__, __LINE__);
#endif
    return 0;
}
