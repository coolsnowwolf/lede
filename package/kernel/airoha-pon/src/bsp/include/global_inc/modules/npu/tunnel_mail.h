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
#ifndef _TUNNEL_MAIL_H_
#define _TUNNEL_MAIL_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include "tunnel_mail_type.h"
#include "npuMboxAPI.h"
#include <linux/dma-mapping.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include <linux/netdevice.h>
#include <asm/tc3162/tc3162.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifdef TCSUPPORT_NPU
extern struct device* get_gdmpSram_dev(void);
extern int host_notify_npuMbox(npuMboxInfo_t *mboxInfo);

#ifdef TCSUPPORT_NPU_V2
static inline int tunnel_mail_store_vxlan_hdr(unsigned char idx, unsigned char* p_data) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_STORE_VXLAN_HDR;
	in_data->tunnel_mail_private.vxlan_hdr.idx = idx;
    memmove(in_data->tunnel_mail_private.vxlan_hdr.data,p_data,VXLAN_HDR_SIZE);

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_STORE_VXLAN_HDR fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_release_vxlan_hdr(unsigned char idx) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_RELEASE_VXLAN_HDR;
	in_data->tunnel_mail_private.vxlan_hdr.idx = idx;

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_RELEASE_VXLAN_HDR fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_set_vxlan_mtu(unsigned int mtu) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_SET_VXLAN_MTU;
	in_data->tunnel_mail_private.vxlan_mtu = mtu;

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_SET_VXLAN_MTU fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}


static inline int tunnel_mail_store_srv6_hdr(unsigned char idx, unsigned char* p_data, int total_len) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_STORE_SRV6_HDR;
	in_data->tunnel_mail_private.srv6_hdr.idx = idx;
	in_data->tunnel_mail_private.srv6_hdr.tot_len = total_len;
    memmove(in_data->tunnel_mail_private.srv6_hdr.data, p_data, total_len);

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_STORE_SRV6_HDR fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_set_srv6_myip(unsigned char* p_data) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_SET_SRV6_MYIP;
    memmove(in_data->tunnel_mail_private.my_ip6, p_data, 16);

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_SET_SRV6_MYIP fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_set_frag_mtu(unsigned char idx, unsigned int mtu) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_SET_FRAG_MTU;
    in_data->tunnel_mail_private.frag_mtu.idx = idx;
	in_data->tunnel_mail_private.frag_mtu.mtu = mtu;

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_SET_FRAG_MTU fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_set_npu_bridge_debug(NPU_BRIDGE_DEBUG_CMD_t debug_cmd, unsigned int debug_level) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_SET_NPU_BRIDGE_DEBUG;
	in_data->tunnel_mail_private.debug.debug_cmd = debug_cmd;
	in_data->tunnel_mail_private.debug.debug_level = debug_level;

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 50000; /* 5000ms, for log print */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_SET_NPU_BRIDGE_DEBUG fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

static inline int tunnel_mail_store_mapt_info_addr(unsigned long long mapt_info_phy_addr) 
{
	TUNNEL_MAIL_Data_t *in_data;
	unsigned long data_va;
	dma_addr_t data_pa;
	npuMboxInfo_t mboxInfo;
	int ret = 0;
	struct device *dev=NULL;

	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {		
		printk("\nget_gdmpSram_dev failed\n");
		return ret;
	}

	data_va = (ulong)dma_alloc_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), &data_pa, GFP_KERNEL);
	in_data = (TUNNEL_MAIL_Data_t *)data_va;
	
	memset(in_data, 0, sizeof(TUNNEL_MAIL_Data_t));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data->funcId = TUNNLE_MAIL_FUNCTION_STORE_MAPT_ADDR;
	in_data->tunnel_mail_private.mapt_info_addr = mapt_info_phy_addr;
	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = data_va;
	mboxInfo.physAddr = data_pa & 0xFFFFFFFF;
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	ret = host_notify_npuMbox(&mboxInfo);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}

	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
	
mboxAPI_fail_exit:
	
	printk("TUNNLE_MAIL_FUNCTION_STORE_MAPT_ADDR fail! \n");
	dma_free_coherent(dev, sizeof(TUNNEL_MAIL_Data_t), (void *)data_va, data_pa);
	return ret;
}

#else
static inline int tunnel_mail_store_vxlan_hdr(unsigned char idx, unsigned char* p_data) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_STORE_VXLAN_HDR;
	in_data.tunnel_mail_private.vxlan_hdr.idx = idx;
    memmove(in_data.tunnel_mail_private.vxlan_hdr.data,p_data,VXLAN_HDR_SIZE);

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_STORE_VXLAN_HDR fail! \n");
	return ret;
}

static inline int tunnel_mail_release_vxlan_hdr(unsigned char idx) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_RELEASE_VXLAN_HDR;
	in_data.tunnel_mail_private.vxlan_hdr.idx = idx;

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_RELEASE_VXLAN_HDR fail! \n");
	return ret;
}

static inline int tunnel_mail_set_vxlan_mtu(unsigned int mtu) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_SET_VXLAN_MTU;
	in_data.tunnel_mail_private.vxlan_mtu = mtu;

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_SET_VXLAN_MTU fail! \n");
	return ret;
}

static inline int tunnel_mail_store_srv6_hdr(unsigned char idx, unsigned char* p_data, int total_len) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_STORE_SRV6_HDR;
	in_data.tunnel_mail_private.srv6_hdr.idx = idx;
	in_data.tunnel_mail_private.srv6_hdr.tot_len = total_len;
    memmove(in_data.tunnel_mail_private.srv6_hdr.data, p_data, total_len);

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_STORE_SRV6_HDR fail! \n");
	return ret;
}

static inline int tunnel_mail_set_srv6_myip(unsigned char* p_data) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_SET_SRV6_MYIP;
    memmove(in_data.tunnel_mail_private.my_ip6, p_data, 16);

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_SET_SRV6_MYIP fail! \n");
	return ret;
}

static inline int tunnel_mail_set_frag_mtu(unsigned char idx, unsigned int mtu) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return -1;
    }

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_SET_FRAG_MTU;
    in_data.tunnel_mail_private.frag_mtu.idx = idx;
	in_data.tunnel_mail_private.frag_mtu.mtu = mtu;

    mboxInfo.core_id = CORE7;
    mboxInfo.func_id = MFUNC_TUNNEL;
    mboxInfo.virtAddr = (unsigned long)(&in_data);
    mboxInfo.physAddr = virt_to_phys(&in_data);
    mboxInfo.len = sizeof(in_data);
    mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
    mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
    phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
        goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
    if (ret != 1) {
        goto mboxAPI_fail_exit;
    }
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_SET_FRAG_MTU fail! \n");
	return ret;
}

static inline int tunnel_mail_set_npu_bridge_debug(NPU_BRIDGE_DEBUG_CMD_t debug_cmd, unsigned int debug_level) 
{
	TUNNEL_MAIL_Data_t in_data;
	npuMboxInfo_t mboxInfo;
	struct device *dev=NULL;
	unsigned long phy_src_addr = 0;
	int ret = 0;
	
	/*just only get vaild dev*/
	if ((dev=get_gdmpSram_dev())==NULL) {
		printk("\nget_gdmpSram_dev failed\n");
		return -1;
	}

	memset(&in_data, 0, sizeof(in_data));
	memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));	
	in_data.funcId = TUNNLE_MAIL_FUNCTION_SET_NPU_BRIDGE_DEBUG;
	in_data.tunnel_mail_private.debug.debug_cmd = debug_cmd;
	in_data.tunnel_mail_private.debug.debug_level = debug_level;

	mboxInfo.core_id = CORE7;
	mboxInfo.func_id = MFUNC_TUNNEL;
	mboxInfo.virtAddr = (unsigned long)(&in_data);
	mboxInfo.physAddr = virt_to_phys(&in_data);
	mboxInfo.len = sizeof(in_data);
	mboxInfo.flags.isBlockingMode = 1;
	mboxInfo.blockTimeout = 1000; /* 100ms */
	mboxInfo.cb = NULL;
	
	/* dma_map_single will clean (wback) dcache for DMA_TO_DEVICE */
	phy_src_addr = dma_map_single(dev, (void *)(&in_data), sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, phy_src_addr)) {
		printk("dma_map_single TO_DEVICE error\n");
		goto mboxAPI_fail_exit;
	}

	ret = host_notify_npuMbox(&mboxInfo);
	dma_unmap_single(dev, phy_src_addr, sizeof(TUNNEL_MAIL_Data_t), DMA_TO_DEVICE);
	if (ret != 1) {
		goto mboxAPI_fail_exit;
	}
	
	return ret;

mboxAPI_fail_exit: 
	
	printk("TUNNLE_MAIL_FUNCTION_SET_NPU_BRIDGE_DEBUG fail! \n");
	return ret;
}

static inline int tunnel_mail_store_mapt_info_addr(unsigned long long mapt_info_phy_addr) 
{
	return 1;
}

#endif

#else
static inline int tunnel_mail_store_vxlan_hdr(unsigned char idx, unsigned char* p_data)
{
    return 1;
}

static inline int tunnel_mail_release_vxlan_hdr(unsigned char idx) 
{
    return 1;
}

static inline int tunnel_mail_set_vxlan_mtu(unsigned int mtu) 
{
    return 1;
}

static inline int tunnel_mail_store_srv6_hdr(unsigned char idx, unsigned char* p_data, int total_len) 
{
    return 1;
}

static inline int tunnel_mail_set_srv6_myip(unsigned char* p_data) 
{
	return 1;
}

static inline int tunnel_mail_set_frag_mtu(unsigned char idx, unsigned int mtu) 
{
	return 1;
}

static inline int tunnel_mail_set_npu_bridge_debug(unsigned char idx, unsigned int mtu) 
{
	return 1;
}

static inline int tunnel_mail_store_mapt_info_addr(unsigned long long mapt_info_phy_addr) 
{
	return 1;
}

#endif

#endif

