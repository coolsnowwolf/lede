/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _DBA_MAIL_H_
#define _DBA_MAIL_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include "dba_mail_type.h"
#include "npuMboxAPI.h"
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
extern struct device* get_gdmpSram_dev(void);
extern int host_notify_npuMbox(npuMboxInfo_t *mboxInfo);

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
//allena npu modify
static inline int DBA_MAIL_API_SET_OLT_PHY_DATA(uint32_t reg, uint32_t val)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->reg = reg;
	in_data->reg_val = val;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_OLT_PHY_DATA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data->reg = reg;
	in_data->reg_val = val;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_OLT_PHY_DATA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	//printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_SET_OLT_MAC_DATA(uint32_t reg, uint32_t val)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->reg = reg;
	in_data->reg_val = val;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_OLT_MAC_DATA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data->reg = reg;
	in_data->reg_val = val;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_OLT_MAC_DATA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	//printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_GET_OLT_PHY_DATA(uint32_t reg, uint32_t* val)
{
	DBA_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(DBA_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->reg = reg;
	in_data->funcType = DBA_GET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_GET_OLT_PHY_DATA;

	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1)
		goto mboxAPI_fail_exit;

	*val = in_data->reg_val;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#endif

	return ret;

mboxAPI_fail_exit:  
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#endif

	//printk("%s fail!\n",__func__);
	return ret;
}

static inline int DBA_MAIL_API_GET_OLT_MAC_DATA(uint32_t reg, uint32_t* val)
{
	DBA_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(DBA_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->reg = reg;
	in_data->funcType = DBA_GET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_GET_OLT_MAC_DATA;

	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	
	if (ret != 1)
		goto mboxAPI_fail_exit;

	*val = in_data->reg_val;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#endif

	return ret;

mboxAPI_fail_exit:  
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#endif

	//printk("%s fail!\n",__func__);
	return ret;
}

//panjun npu modify

static inline int DBA_MAIL_API_SET_DBA_TIMER_FLAG(unsigned int get_dba_timer_en, unsigned int do_dba_timer_en)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->get_dba_timer_en = get_dba_timer_en;
	in_data->do_dba_timer_en = do_dba_timer_en;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_DBA_TIMER_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data->get_dba_timer_en = get_dba_timer_en;
	in_data->do_dba_timer_en = do_dba_timer_en;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_DBA_TIMER_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}


static inline int DBA_MAIL_API_SET_PRINT_DBA_FLAG(unsigned int print_dba_en_flag)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->print_dba_info_en = print_dba_en_flag;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_PRINT_DBA_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data.print_dba_info_en = print_dba_en_flag;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_PRINT_DBA_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}


static inline int DBA_MAIL_API_SET_PRINT_DBA_NO_IDLE_FLAG(unsigned int print_dba_en_flag)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->print_dba_no_idle_en = print_dba_en_flag;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_PRINT_NO_IDLE_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data.print_dba_no_idle_en = print_dba_en_flag;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_PRINT_NO_IDLE_EN;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}


static inline int DBA_MAIL_API_SET_CLEAN_DBA_NO_IDLE_FLAG(unsigned int clean_en_flag)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->clean_dba_no_idle_en = clean_en_flag;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_CLEAN_NO_IDLE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->clean_dba_no_idle_en = clean_en_flag;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_CLEAN_NO_IDLE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}



static inline int DBA_MAIL_API_SET_ONU_STATE(unsigned int onu_id, unsigned int state)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->onu_id = onu_id;
	in_data->state = state;
	
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_ONU_STATE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->onu_id = onu_id;
	in_data->state = state;
	
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_ONU_STATE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_SET_TCONT_DBA(uint32_t alloc_id, uint32_t tcont_id, uint32_t fix_band, uint32_t assure_band, 
	uint32_t max_band, uint32_t dba_type)

{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->alloc_id = alloc_id;
	in_data->fix_band = fix_band;
	in_data->assure_band = assure_band;
	in_data->max_band = max_band;
	in_data->tcont_id = tcont_id;
	in_data->dba_type = dba_type;
	
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_TCONT_DBA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->alloc_id = alloc_id;
	in_data->fix_band = fix_band;
	in_data->assure_band = assure_band;
	in_data->max_band = max_band;
	in_data->tcont_id = tcont_id;
	in_data->dba_type = dba_type;

	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_TCONT_DBA;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_SET_TCONT_STATE(uint8_t tcont_id, uint8_t valid)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->tcont_id = tcont_id;
	in_data->valid = valid;

	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_TCONT_STATE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->tcont_id = tcont_id;
	in_data->valid = valid;

	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_TCONT_STATE;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}


static inline int DBA_MAIL_API_SET_ONU_US_FEC(uint8_t onu_id, uint8_t fec_sts)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->onu_id = onu_id;
	in_data->fec = fec_sts;

	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_ONU_US_FEC;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->onu_id = onu_id;
	in_data->fec = fec_sts;

	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_ONU_US_FEC;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_GET_TCONT_INFO(uint32_t tcont_id)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->tcont_id = tcont_id;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_GET_TCONT_INFO;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	in_data->tcont_id = tcont_id;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_GET_TCONT_INFO;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}

static inline int DBA_MAIL_API_SET_PLOAMU_FLAG(uint8_t tcont_id, uint8_t ploamu_flag)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	
	in_data->tcont_id = tcont_id;
	in_data->ploamu_flag = ploamu_flag;
	
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SET_PLOAMU_FLAG;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data->tcont_id = tcont_id;
	in_data->ploamu_flag = ploamu_flag;

	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SET_PLOAMU_FLAG;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}


static inline int DBA_MAIL_API_SET_SW_BWMAP_TEST_FLAG(unsigned int sw_bwmap_test_flag)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct DBA_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct DBA_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(DBA_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(DBA_MAIL_Data_t));
	in_data->sw_bwmap_test_flag = sw_bwmap_test_flag;
	in_data->funcType = DBA_SET_WAIT;
	in_data->funcId = DBA_MAIL_FUNCTION_SW_BWMAP_TEST;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(DBA_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(DBA_MAIL_Data_t), (void *)va, pa);
#else
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(&in_data, 0, sizeof(in_data));
	
	in_data->sw_bwmap_test_flag = sw_bwmap_test_flag;
	in_data.funcType = DBA_SET_WAIT;
	in_data.funcId = DBA_MAIL_FUNCTION_SW_BWMAP_TEST;
	mboxInfo.core_id = CORE5;
	mboxInfo.func_id = MFUNC_DBA;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);

	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(DBA_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! ret=%d\n", __func__, ret);
	return ret;
}




#endif

