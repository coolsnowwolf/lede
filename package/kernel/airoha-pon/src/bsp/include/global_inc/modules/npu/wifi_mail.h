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
#ifndef _WIFI_MAIL_H_
#define _WIFI_MAIL_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include "wifi_mail_type.h"
#include "npuMboxAPI.h"
#include <linux/dma-mapping.h>
//#include <ecnt_hook/ecnt_hook_qdma.h>
#include <linux/netdevice.h>
//extern struct device* get_gdmpSram_dev(void);
extern struct device* get_ecnt_npu_dev(void);
extern int host_notify_npuMbox(npuMboxInfo_t *mboxInfo);

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

static inline int WIFI_MAIL_API_SET_WAIT_PCIE_ADDR(unsigned int interfaceID, unsigned int PCIEAddr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_ADDR;
	in_data->wifi_mail_private.PCIEAddr = PCIEAddr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
//printk("PCIE:in_data.interfaceID=%d,in_data.funcType =%d, in_data.funcId =%d, in_data.wifi_mail_private.PCIEAddr=%lx\n",  in_data->interfaceID, in_data->funcType, in_data->funcId, in_data->wifi_mail_private.PCIEAddr);
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_ADDR;
	in_data.wifi_mail_private.PCIEAddr = PCIEAddr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_PCIE_ADDR fail! \n");
	return ret;
}
static inline int WIFI_MAIL_API_SET_RATELIMIT(unsigned int band_idx,unsigned int bssid_idx, unsigned int ctrl) 
{
	struct WIFI_MAIL_Data in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = 0;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_RATELIMIT_CTRL;
	in_data.wifi_mail_private.rate_limit_ctrl.band_idx = band_idx;
	in_data.wifi_mail_private.rate_limit_ctrl.bssid_idx = bssid_idx;
	in_data.wifi_mail_private.rate_limit_ctrl.ctrl = ctrl;
    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1)
	{
        goto mboxAPI_fail_exit;
	}
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_PCIE_ADDR fail! \n");
	return ret;
}


//#ifdef TCSUPPORT_NPU_WIFI_TX
//for lan->wifi case, sending address of buffer pool which is for slow path and qdma rx
static inline int WIFI_MAIL_API_SET_WAIT_TX_PKT_BUF_ADDR(unsigned int interfaceID, unsigned int npuPktBufAddr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_PKT_BUF_ADDR;
	in_data->wifi_mail_private.npuPktBufAddr = npuPktBufAddr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_PKT_BUF_ADDR;
	in_data.wifi_mail_private.npuPktBufAddr = npuPktBufAddr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;

	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_TX_PKT_BUF_ADDR fail! \n");
	return ret;
}

//phy addr of pcie of wifi tx ring. It depends on port0 <-> port0 or port1 <-> port0
static inline int WIFI_MAIL_API_SET_WAIT_TX_RING_PCIE_ADDR(unsigned int interfaceID, unsigned int PCIEAddr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_RING_PCIE_ADDR;
	in_data->wifi_mail_private.PCIEAddr = PCIEAddr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_RING_PCIE_ADDR;
	in_data.wifi_mail_private.PCIEAddr = PCIEAddr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_TX_RING_PCIE_ADDR fail! \n");
	return ret;
}

//wifi tx ring phy addr of tx descriptor
static inline int WIFI_MAIL_API_SET_WAIT_TX_DESC_BASE(unsigned int interfaceID, unsigned int phy_base_addr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_DESC_HW_BASE;
	in_data->wifi_mail_private.phy_addr = phy_base_addr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_DESC_HW_BASE;
	in_data.wifi_mail_private.phy_addr = phy_base_addr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_TX_DESC_BASE fail! \n");
	return ret;
}

//wifi tx ring's dma buf ring which is for tmac and txp info
static inline int WIFI_MAIL_API_SET_WAIT_TX_BUF_SPACE_BASE(unsigned int interfaceID, unsigned int phy_base_addr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_BUF_SPACE_HW_BASE;
	in_data->wifi_mail_private.phy_addr = phy_base_addr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TX_BUF_SPACE_HW_BASE;
	in_data.wifi_mail_private.phy_addr = phy_base_addr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_TX_BUF_SPACE_BASE fail! \n");
	return ret;
}

//send phy addr of wifi rx ring which is task for Tx done event
static inline int WIFI_MAIL_API_SET_WAIT_RX_RING_FOR_TXDONE_HW_BASE(unsigned int interfaceID, unsigned int phy_base_addr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_RX_RING_FOR_TXDONE_HW_BASE;
	in_data->wifi_mail_private.phy_addr = phy_base_addr;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_RX_RING_FOR_TXDONE_HW_BASE;
	in_data.wifi_mail_private.phy_addr = phy_base_addr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
	goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_RX_RING_FOR_TXDONE_HW_BASE fail! \n");
	return ret;
}
//#endif //TCSUPPORT_NPU_WIFI_TX

static inline int WIFI_MAIL_API_SET_WAIT_DESC(unsigned int interfaceID, unsigned int desc) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DESC;
	in_data->wifi_mail_private.desc = desc&0xffffffff;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DESC;
	in_data.wifi_mail_private.desc = desc;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif	
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk(" WIFI_MAIL_API_SET_WAIT_DESC fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_NPU_INIT_DONE(unsigned int interfaceID, unsigned int npuInitDone) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_NPU_INIT_DONE;
	in_data->wifi_mail_private.npuInitDone = npuInitDone;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_NPU_INIT_DONE;
	in_data.wifi_mail_private.npuInitDone = npuInitDone;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_NPU_INIT_DONE fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_TRAN_TO_CPU(unsigned int interfaceID, unsigned int tran2cpu) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TRAN_TO_CPU;
	in_data->wifi_mail_private.tran2cpu = tran2cpu;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TRAN_TO_CPU;
	in_data.wifi_mail_private.tran2cpu = tran2cpu;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_TRAN_TO_CPU fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_BA_WIN_SIZE(unsigned int interfaceID,unsigned int wcid, unsigned int tid,unsigned int BAWinSize,unsigned int sn)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa,  (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_BA_WIN_SIZE;
	in_data->wifi_mail_private.BAWinSize = ((sn&0xfff)<<20)| ((BAWinSize&0x1ff)<<11)| ((wcid&0xff)<<3) |(tid&0x7);// wcid max 255 
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_BA_WIN_SIZE;
	in_data.wifi_mail_private.BAWinSize = ((sn&0xfff)<<20)| ((BAWinSize&0x1ff)<<11)| ((wcid&0xff)<<3) |(tid&0x7);// wcid max 255 

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_BA_WIN_SIZE fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_DRIVER_MODEL(unsigned int interfaceID, unsigned int DriverModel)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	printk("DriverModel=%d\n", DriverModel);
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DRIVER_MODEL;
	in_data->wifi_mail_private.DriverModel = DriverModel;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DRIVER_MODEL;
	in_data.wifi_mail_private.DriverModel = DriverModel;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_BA_WIN_SIZE fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_DRAM_BA_NODE_ADDR(unsigned int interfaceID, unsigned int dramBaNode_pa)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	printk("dramBaEntry_pa=%x\n", dramBaNode_pa);
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DRAM_BA_NODE_ADDR;
	in_data->wifi_mail_private.dramBaNodeAddr = dramBaNode_pa&0xffffffff;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
//printk("BA_NODE:in_data.interfaceID=%d,in_data.funcType =%d, in_data.funcId =%d, in_data.wifi_mail_private.dramBaNodeAddr=%lx\n",  in_data->interfaceID, in_data->funcType, in_data->funcId, in_data->wifi_mail_private.dramBaNodeAddr);
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DRAM_BA_NODE_ADDR;
	in_data.wifi_mail_private.dramBaNodeAddr = dramBaNode_pa;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}
static inline int WIFI_MAIL_API_SET_WAIT_IS_FORCE_TO_CPU(unsigned int interfaceID, unsigned int isForceToCpu)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	printk("isForceToCpu=%x\n", isForceToCpu);
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_IS_FORCE_TO_CPU;
	in_data->wifi_mail_private.isForceToCpu_flag = isForceToCpu;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_IS_FORCE_TO_CPU;
	in_data.wifi_mail_private.isForceToCpu_flag = isForceToCpu;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}
static inline int WIFI_MAIL_API_SET_WAIT_PKT_BUF_ADDR(unsigned int interfaceID, unsigned int npuPktBufAddr)
{
	//printk("npuPktBufAddr=%x\n", npuPktBufAddr);
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PKT_BUF_ADDR;
	in_data->wifi_mail_private.npuPktBufAddr = npuPktBufAddr & 0xffffffff;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PKT_BUF_ADDR;
	in_data.wifi_mail_private.npuPktBufAddr = npuPktBufAddr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}


/*static inline int WIFI_MAIL_API_SET_WAIT_TDMA_TX_PKT_BUF_ADDR(unsigned int interfaceID, unsigned int npuPktBufAddr)
{
	//printk("npuPktBufAddr=%x\n", npuPktBufAddr);
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	//just only get vaild dev
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TDMA_TX_PKT_BUF_ADDR;
	in_data->wifi_mail_private.npuPktBufAddr = npuPktBufAddr & 0xffffffff;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; // 100ms 
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_TDMA_TX_PKT_BUF_ADDR;
	in_data.wifi_mail_private.npuPktBufAddr = npuPktBufAddr;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; // 100ms 
    mboxInfo.cb = NULL;
	
	// dma_map_single will clean (wback) dcache for DMA_TO_DEVICE
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
    if (ret != 1)
        goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}*/


static inline int WIFI_MAIL_API_SET_WAIT_NOBA_FLAG(unsigned int interfaceID, unsigned int isforTestNoBaFlag)
{
//	printk("isforTestNoBaFlag=%x\n", isforTestNoBaFlag);
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_IS_TEST_NOBA;
	in_data->wifi_mail_private.isforTestNoBaFlag = isforTestNoBaFlag;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_IS_TEST_NOBA;
	in_data.wifi_mail_private.isforTestNoBaFlag = isforTestNoBaFlag;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_FAST_FLAG(unsigned int interfaceID, unsigned char fastFlag) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;

#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
	if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
#ifdef TCSUPPORT_NPU_V2
		va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
		in_data=(WIFI_MAIL_Data_t*)va;
		memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
		memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
		in_data->interfaceID = interfaceID;
		in_data->funcType = SET_WAIT;
		in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FAST_FLAG;
		printk("\nfastFlag=%d",fastFlag);
		in_data->wifi_mail_private.fastFlag = fastFlag;
		printk("\nin_data->wifi_mail_private.fastFlag=%d",	in_data->wifi_mail_private.fastFlag);
		mboxInfo.core_id = CORE0;
		mboxInfo.func_id = MFUNC_WIFI;
		mboxInfo.virtAddr = va; 
		mboxInfo.physAddr = pa &0xffffffff;
		mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
		mboxInfo.flags.isBlockingMode = 1;
		mboxInfo.blockTimeout = 10000; /* 100ms */
		mboxInfo.cb = NULL;
		//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
		ret = host_notify_npuMbox(&mboxInfo);
	
		dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);		
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FAST_FLAG;
	in_data.wifi_mail_private.fastFlag = fastFlag;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_FAST_FLAG fail! \n");
	return ret;
}


static inline int WIFI_MAIL_API_SET_WAIT_NPU_BAND0_ONCPU(unsigned int interfaceID, unsigned int band0onCpu)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
	if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_NPU_BAND0_ONCPU;
	in_data->wifi_mail_private.band0OnCpuFlag = band0onCpu;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_NPU_BAND0_ONCPU;
	in_data.wifi_mail_private.band0OnCpuFlag = band0onCpu;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_PCIE_STATE(unsigned int interfaceID, unsigned int PCIEAddr) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_STATE;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_STATE;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_PCIE_STATE fail! \n");
	return ret;
}
static inline int WIFI_MAIL_API_SET_WAIT_PCIE_PORT_TYPE(unsigned int interfaceID, unsigned int PciePortType)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
	if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_PORT_TYPE;
	in_data->wifi_mail_private.PciePortType = PciePortType;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_PORT_TYPE;
	in_data.wifi_mail_private.PciePortType = PciePortType;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_PCIE_PORT_TYPE fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_BAR_INFO(unsigned int interfaceID,unsigned int wcid, unsigned int tid,unsigned int sn)
{
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	//unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return 0;
    }

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_BAR_INFO;
	in_data->wifi_mail_private.BARINFO = ((sn&0xfff)<<11)| ((wcid&0xff)<<3) |(tid&0x7);;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_BAR_INFO;
	in_data.wifi_mail_private.BARINFO = ((sn&0xfff)<<11)| ((wcid&0xff)<<3) |(tid&0x7);

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_BA_WIN_SIZE fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_FLUSHONE_TIMEOUT(unsigned int interfaceID, unsigned int flushone_timeout)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return 0;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHONE_TIMEOUT;
	in_data->wifi_mail_private.flushone_timeout = flushone_timeout;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHONE_TIMEOUT;
	in_data.wifi_mail_private.flushone_timeout = flushone_timeout;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}
static inline int WIFI_MAIL_API_SET_WAIT_FLUSHALL_TIMEOUT(unsigned int interfaceID, unsigned int flushall_timeout)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return 0;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHALL_TIMEOUT;
	in_data->wifi_mail_private.flushall_timeout = flushall_timeout;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv(((unsigned long int)(mboxInfo)), sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
	
#else


	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHALL_TIMEOUT,
	in_data.wifi_mail_private.flushall_timeout = flushall_timeout;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}
	
	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s fail! \n", __func__);
	return ret;
}

//#ifdef TCSUPPORT_WLAN_INODE
static inline int WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR(unsigned int interfaceID, unsigned int dir,
unsigned int inCounterAddress,unsigned int outStatusAddress,unsigned int outCounterAddress) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	int ret = 0;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif

	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data = (WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_TXRX_REG_ADDR;
	in_data->wifi_mail_private.txrx_addr.dir = dir & 0xffffffff;
	in_data->wifi_mail_private.txrx_addr.inCounterAddr = inCounterAddress & 0xffffffff;
	in_data->wifi_mail_private.txrx_addr.outStatusAddr = outStatusAddress & 0xffffffff;
	in_data->wifi_mail_private.txrx_addr.outCounterAddr= outCounterAddress & 0xffffffff;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa & 0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 60000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_TXRX_REG_ADDR;
	in_data.wifi_mail_private.txrx_addr.dir=dir;
	in_data.wifi_mail_private.txrx_addr.inCounterAddr = inCounterAddress;
	in_data.wifi_mail_private.txrx_addr.outStatusAddr =outStatusAddress;
	in_data.wifi_mail_private.txrx_addr.outCounterAddr=outCounterAddress;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_INODE_TXRX_REG_ADDR fail! dir:%d %x %x %x\n",
		dir, inCounterAddress, outStatusAddress, outCounterAddress);
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_INODE_DEBUG_FLAG(unsigned int interfaceID,
					unsigned int debugFlag) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	int ret = 0;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_DEBUG_FLAG;
	in_data->wifi_mail_private.npuInfo.info= debugFlag;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	//ecnt_dcache_wback_inv((unsigned long int)(mboxInfo),  sizeof(npuMboxInfo_t));
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_DEBUG_FLAG;
	in_data.wifi_mail_private.npuInfo.info= debugFlag;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	printk("WIFI_MAIL_API_SET_WAIT_INODE_DEBUG_FLAG fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_INODE_HW_CFG_INFO(unsigned int interfaceID,
					unsigned char rx_hd_ep_mask,unsigned char rx_hd_vap_mask) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	int ret = 0;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_HW_CFG_INFO;
	in_data->wifi_mail_private.cfg_info.ep_mask = rx_hd_ep_mask;
	in_data->wifi_mail_private.cfg_info.vap_mask = rx_hd_vap_mask;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_HW_CFG_INFO;
	in_data.wifi_mail_private.cfg_info.ep_mask= rx_hd_ep_mask;
	in_data.wifi_mail_private.cfg_info.vap_mask=rx_hd_vap_mask;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	printk("WIFI_MAIL_API_SET_WAIT_INODE_HW_CFG_INFO fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_INODE_STOP_ACTION(unsigned int interfaceID) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	int ret = 0;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_STOP_ACTION;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_STOP_ACTION;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_INODE_STOP_ACTION fail! \n");
	return ret;
}
static inline int WIFI_MAIL_API_SET_WAIT_INODE_PCIE_SWAP(unsigned int interfaceID,unsigned int is_swap) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	int ret = 0;
#ifdef TCSUPPORT_NPU_V2
	struct WIFI_MAIL_Data *in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_PCIE_SWAP;
	in_data->wifi_mail_private.npuInfo.info = is_swap;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa&0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	ret = host_notify_npuMbox(&mboxInfo);
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_INODE_PCIE_SWAP;
	in_data.wifi_mail_private.npuInfo.info=is_swap;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("WIFI_MAIL_API_SET_WAIT_INODE_PCIE_SWAP fail! \n");
	return ret;
}

static inline int WIFI_MAIL_API_GET_WAIT_DMA_ADDR(unsigned int interfaceID,unsigned int dir, unsigned int *addr)
{
	WIFI_MAIL_Data_t *in_data;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	unsigned long pa = 0, va=0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
	if ((dev=get_ecnt_npu_dev())==NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va) {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_DMA_ADDR;
	in_data->wifi_mail_private.dir = dir;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1)
		goto mboxAPI_fail_exit;

	*addr = in_data->wifi_mail_private.npuInfo.info;
	printk("dir=%x addr=%x\n", in_data->wifi_mail_private.dir, *addr);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;

mboxAPI_fail_exit:   
	printk("WIFI_MAIL_API_GET_WAIT_DMA_ADDR fail!\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;
}


static inline int WIFI_MAIL_API_GET_WAIT_RING_SIZE(unsigned int interfaceID, unsigned int dir,unsigned int *size)
{
	WIFI_MAIL_Data_t *in_data;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	unsigned long pa = 0, va=0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev = NULL;
	/*just only get vaild dev*/
	if ((dev = get_ecnt_npu_dev()) == NULL) {
		printk("\nget_ecnt_npu_dev failed\n");
		return ret;
	}
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}	
	in_data=(WIFI_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_RING_SIZE;
	in_data->wifi_mail_private.dir=dir;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1)
		goto mboxAPI_fail_exit;

	*size = in_data->wifi_mail_private.npuInfo.info;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;

mboxAPI_fail_exit:   
	printk("WIFI_MAIL_API_GET_WAIT_RING_SIZE fail!\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;
}
//endif TCSUPPORT_WLAN_INODE

static inline int WIFI_MAIL_API_GET_WAIT_RXDESC_BASE(unsigned int interfaceID, npu_info_t *pnpuInfo)
{
	
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
        printk("%s dma_alloc_coherent failed\n", __func__);
        return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_RXDESC_BASE;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	
	if (ret != 1)
		goto mboxAPI_fail_exit;

	pnpuInfo->info = in_data->wifi_mail_private.npuInfo.info;
	//printk("WIFI_MAIL_API_GET_WAIT_RXDESC_BASE info=%x\n",pnpuInfo->info);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif

	return ret;

mboxAPI_fail_exit: 
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif

	printk("WIFI_MAIL_API_GET_WAIT_RXDESC_BASE fail!\n");
	return ret;
}
static inline int WIFI_MAIL_API_GET_WAIT_NPU_INFO(unsigned int interfaceID, npu_info_t *pnpuInfo)
{
	
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_NPU_INFO;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	
	if (ret != 1)
		goto mboxAPI_fail_exit;

	pnpuInfo->info = in_data->wifi_mail_private.npuInfo.info;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif

	return ret;

mboxAPI_fail_exit:  
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif

	printk("WIFI_MAIL_API_GET_WAIT_NPU_INFO fail!\n");
	return ret;
}


static inline int WIFI_MAIL_API_GET_WAIT_LAST_RATE(unsigned int interfaceID, last_rate_t *plastRate)
{
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif

	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_LAST_RATE;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;


	ret = host_notify_npuMbox(&mboxInfo);

	if (ret != 1)
		goto mboxAPI_fail_exit;

	plastRate->perSta = in_data->wifi_mail_private.lastRate.perSta;
	plastRate->total = in_data->wifi_mail_private.lastRate.total;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif

	return ret;

mboxAPI_fail_exit: 
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	printk("WIFI_MAIL_API_GET_WAIT_LAST_RATE fail!\n");
	return ret;
}


static inline int WIFI_MAIL_API_GET_WAIT_COUNTER(unsigned int interfaceID, counter_t *pcount)
{
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) 
	struct device *dev;
#endif

	int i;
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	static unsigned long last_time = 0;
	if (last_time > jiffies)
		last_time = jiffies;
	
	if (jiffies < last_time + 20) {
		return -1;
	}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va =  (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	/*struct device * */dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif

	if (!va)  {
		printk("%s L%d ma_alloc_coherent failed\n", __func__,__LINE__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;

	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_COUNTER;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr =va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;


	ret = host_notify_npuMbox(&mboxInfo);

	if (ret != 1)
		goto mboxAPI_fail_exit;

	pcount->txCount= in_data->wifi_mail_private.count.txCount;

	for(i=0; i<MAX_SSID_PER_BAND; i++)
	{
		pcount->rxByteCount2G[i] = in_data->wifi_mail_private.count.rxByteCount2G[i];
		pcount->rxByteCount5G[i] = in_data->wifi_mail_private.count.rxByteCount5G[i];
		pcount->rxCount2G[i] = in_data->wifi_mail_private.count.rxCount2G[i];
		pcount->rxCount5G[i] = in_data->wifi_mail_private.count.rxCount5G[i];	
		pcount->omacIdx2G[i] = in_data->wifi_mail_private.count.omacIdx2G[i];
		pcount->omacIdx5G[i] = in_data->wifi_mail_private.count.omacIdx5G[i];
	}

	//pcount->rxCount= in_data->wifi_mail_private.count.rxCount;
	pcount->rxApcliByteCount2G = in_data->wifi_mail_private.count.rxApcliByteCount2G;
	pcount->rxApcliByteCount5G = in_data->wifi_mail_private.count.rxApcliByteCount5G;
	pcount->rxApcliCount2G = in_data->wifi_mail_private.count.rxApcliCount2G;
	pcount->rxApcliCount5G = in_data->wifi_mail_private.count.rxApcliCount5G;

	for(i=0; i<MAX_PER_ENTRY; i++)  
    {       
    	//BAND0
        pcount->rx_pkts_entry[0][i] = in_data->wifi_mail_private.count.rx_pkts_entry[0][i];      
        pcount->rx_bytes_entry[0][i] = in_data->wifi_mail_private.count.rx_bytes_entry[0][i];
		//BAND1
		pcount->rx_pkts_entry[1][i] = in_data->wifi_mail_private.count.rx_pkts_entry[1][i];      
        pcount->rx_bytes_entry[1][i] = in_data->wifi_mail_private.count.rx_bytes_entry[1][i];
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	last_time = jiffies;
#endif

	return ret;

mboxAPI_fail_exit:
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	printk("WIFI_MAIL_API_GET_WAIT_COUNTER fail!\n");
	return ret;
}

static inline int WIFI_MAIL_API_GET_WAIT_DBG_COUNTER(unsigned int interfaceID, dbg_counter_t *pdbgCount)
{
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	struct device *dev=NULL;
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_DBG_COUNTER;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;


	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1)
		goto mboxAPI_fail_exit;

	pdbgCount->errCount = in_data->wifi_mail_private.dbgCount.errCount;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;

mboxAPI_fail_exit:  
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t),(void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	printk("WIFI_MAIL_API_GET_WAIT_DBG_COUNTER fail!\n");
	return ret;
}

static inline int WIFI_MAIL_API_GET_WAIT_WCID_DBG_COUNTER(unsigned int interfaceID, dbg_counter_t *pdbgCount)
{
	WIFI_MAIL_Data_t* in_data;
	npuMboxInfo_t mboxInfo;
	unsigned long pa = 0, va=0;
	int ret = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0) 
	struct device *dev=NULL;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	va = (unsigned long)dma_alloc_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)&pa, GFP_KERNEL);
#else
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }
	va =  (unsigned long)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa, (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
#endif
	
	if (!va)  {
		printk("%s dma_alloc_coherent failed\n", __func__);
		return ret;
	}
	in_data=(WIFI_MAIL_Data_t*)va;
	
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = GET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_GET_WAIT_WCID_DBG_COUNTER;

	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va;
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1)
		goto mboxAPI_fail_exit;

	pdbgCount->errCount = in_data->wifi_mail_private.dbgCount.errCount;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	return ret;

mboxAPI_fail_exit:   
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	dma_free_coherent(NULL, sizeof(WIFI_MAIL_Data_t),(void *)va, pa);
#else
	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#endif
	printk("WIFI_MAIL_API_GET_WAIT_WCID_DBG_COUNTER fail!\n");
	return ret;
}


static inline int WIFI_MAIL_API_SET_WAIT_ADD_DEL_STA(unsigned int interfaceID, unsigned int wcid) 
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
		return ret;
    }

#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa,  (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DEL_STA;
	in_data->wifi_mail_private.wcid = wcid;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa &0xffffffff;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout =10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else
	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.interfaceID = interfaceID;
	in_data.funcType = SET_WAIT;
	in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_DEL_STA;
	in_data.wifi_mail_private.wcid = wcid;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;
	

//	printk("%s L%d send! \n",__func__,__LINE__);
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif

	if (ret != 1)
		goto mboxAPI_fail_exit;
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("%s L%d fail! \n",__func__,__LINE__);
	return ret;
}

static inline int WIFI_MAIL_API_SET_WAIT_ERROR_RETRY_TIMES(unsigned int interfaceID, unsigned int retryTimes)
{
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
#ifdef TCSUPPORT_NPU_V2
	WIFI_MAIL_Data_t* in_data;
	unsigned long int pa = 0, va=0;
#else
	struct WIFI_MAIL_Data in_data;
	unsigned long phy_src_addr = 0;
#endif
    int ret = 0;

    /*just only get vaild dev*/
    if ((dev=(struct device *)get_ecnt_npu_dev())==NULL) {
        printk("\nget_ecnt_npu_dev failed\n");
        return 0;
        }
#ifdef TCSUPPORT_NPU_V2
	va = (unsigned long int)dma_alloc_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)&pa,  (GFP_ATOMIC|__GFP_NOWARN|GFP_DMA));
	in_data=(WIFI_MAIL_Data_t*)va;
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
	memset(in_data, 0, sizeof(WIFI_MAIL_Data_t));
	in_data->interfaceID = interfaceID;
	in_data->funcType = SET_WAIT;
	in_data->funcId = WIFI_MAIL_FUNCTION_SET_WAIT_ERROR_RETRY_TIMES;
	in_data->wifi_mail_private.retryTimes = retryTimes;
	mboxInfo.core_id = CORE0;
	mboxInfo.func_id = MFUNC_WIFI;
	mboxInfo.virtAddr = va; 
	mboxInfo.physAddr = pa;
	mboxInfo.len = sizeof(WIFI_MAIL_Data_t);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout =10000; /* 100ms */
	mboxInfo.cb = NULL;

	ret = host_notify_npuMbox(&mboxInfo);

	dma_free_coherent(dev, sizeof(WIFI_MAIL_Data_t), (void *)va, pa);
#else

    memset(&in_data, 0, sizeof(in_data));
    memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
    in_data.interfaceID = interfaceID;
    in_data.funcType = SET_WAIT;
    in_data.funcId = WIFI_MAIL_FUNCTION_SET_WAIT_ERROR_RETRY_TIMES;
    in_data.wifi_mail_private.retryTimes = retryTimes;

    mboxInfo.core_id = CORE0;
    mboxInfo.func_id = MFUNC_WIFI;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
    mboxInfo.blockTimeout = 10000; /* 100ms */
    mboxInfo.cb = NULL;

    /* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
    if (dma_mapping_error(dev, phy_src_addr)) {
        printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
    }

    ret = host_notify_npuMbox(&mboxInfo);
    dma_unmap_single(dev, phy_src_addr, sizeof(WIFI_MAIL_Data_t), DMA_TO_DEVICE);
#endif
	if (ret != 1)
		goto mboxAPI_fail_exit;

    return ret;

mboxAPI_fail_exit:

    printk("WIFI_MAIL_API_SET_WAIT__ERROR_RETRY_TIMES fail! \n");
    return ret;
}


#endif

