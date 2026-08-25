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
#ifndef _QDMA_DEV_7516_H_
#define _QDMA_DEV_7516_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/netdevice.h>
#include <linux/foe_hook.h>
#include <linux/timer.h>
#include "qdma_reg.h"
#include "qdma_api.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifdef TCSUPPORT_DRIVER_API
#ifdef QDMA_LAN
#define COSNT_QDMA_MAJOR_NUN			119
#define CONST_QDMA_DEV_NAME				"qdma_lan"
#else
#define COSNT_QDMA_MAJOR_NUN			120
#define CONST_QDMA_DEV_NAME				"qdma_wan"
#endif
#endif

#define QDMA_RX_DSCP_MSG_LENS			16
#define QDMA_TX_DSCP_MSG_LENS			8
#define QDMA_HWFWD_DSCP_MSG_LENS		8
#define QDMA_DSCP_MSG_WORD_LENS			4
#define QDMA_DSCP_MSG_TOTAL_WORD_LENS	(QDMA_DSCP_MSG_WORD_LENS+(QDMA_RX_DSCP_MSG_LENS>>2))

#define TX_RATELIMIT_TICKER0_ACCURACY 1 /*kbps -->8000us*/
#define TX_RATELIMIT_TICKER1_ACCURACY 400 /*kbps -->20us*/

#define TXQ_DIS_CFG_REG_NUM				8

#define GET_METER_IDX(index)    (index&0xff)
#define GET_METER_GRP(index)    ((index>>8)&0x3)

/*EN7580: WAN: HWF dscp max: 16K/64K , Per-RingSize max: 4K , IRQ max depth:4K*/
/*EN7580: LAN: HWF dscp max: 16K/64K , Per-RingSize max: 4K , IRQ max depth:4K*/

/*EN7516: WAN: HWF dscp max: 8K , Per-RingSize max: 4k , IRQ max depth:4K*/
/*EN7516: LAN: HWF dscp max: 2K , Per-RingSize max: 1k , IRQ max depth:4K*/

#if defined(QDMA_LAN) // qdma1
    #define CONFIG_QDMA_QUEUE					8
	
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_HWFWD_DSCP_NUM               (1000)	// for csr2 test
#else
#ifdef	TCSUPPORT_DSL_PHYMODE
	#define CONFIG_HWFWD_DSCP_NUM				(512) 	
#else
#if defined(TCSUPPORT_NP_RAM_SHRINK)
    #define CONFIG_HWFWD_DSCP_NUM				(1024) 	/*1024 ->2048 shoule not more than 4096*/
#else
	#define CONFIG_HWFWD_DSCP_NUM				(2048)  /*1024 ->2048 shoule not more than 4096*/
#endif
#endif
#endif
    
#ifdef	TCSUPPORT_DSL_PHYMODE
	#define CONFIG_HWFWD_DSCP_NUM_MAX			(2048)
	
	#define CONFIG_TX1_DSCP_NUM 				(128)
	#define CONFIG_TX0_DSCP_NUM 				(256)
    
	#define CONFIG_RX15_DSCP_NUM				(128)
	#define CONFIG_RX14_DSCP_NUM				(2)
	#define CONFIG_RX13_DSCP_NUM				(2)
	#define CONFIG_RX12_DSCP_NUM				(2)
	#define CONFIG_RX11_DSCP_NUM				(2)
	#define CONFIG_RX10_DSCP_NUM				(2)
	#define CONFIG_RX9_DSCP_NUM 				(2)
	#define CONFIG_RX8_DSCP_NUM 				(2)
	#define CONFIG_RX7_DSCP_NUM 				(256)
	#define CONFIG_RX6_DSCP_NUM 				(2) /*WIFI-5G*/
	#define CONFIG_RX5_DSCP_NUM 				(2) /*WIFI-2.4G*/
	#define CONFIG_RX4_DSCP_NUM 				(2)
	#define CONFIG_RX3_DSCP_NUM 				(2)
	#define CONFIG_RX2_DSCP_NUM 				(2)
#else

#if defined(TCSUPPORT_NP_RAM_SHRINK)
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(128)
    #define CONFIG_RX15_DSCP_NUM				(8)
    #define CONFIG_RX14_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX13_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX12_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX11_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX10_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX9_DSCP_NUM					(2)/*(16)*/
    #define CONFIG_RX8_DSCP_NUM					(2)/*(16)*/
    #define CONFIG_RX7_DSCP_NUM					(2)/*(256)*/
    #define CONFIG_RX6_DSCP_NUM					(560)/*(2048)*/ /*WIFI-5G*/
    #define CONFIG_RX5_DSCP_NUM					(560)/*(2048)*/ /*WIFI-2.4G*/
    #define CONFIG_RX4_DSCP_NUM					(2)
    #define CONFIG_RX3_DSCP_NUM					(2)
    #define CONFIG_RX2_DSCP_NUM					(2)
#else
    #define CONFIG_HWFWD_DSCP_NUM_MAX           (2048)
	
    #define CONFIG_TX1_DSCP_NUM					(128)
#if defined(TCSUPPORT_CPU_EN7516) && defined(TCSUPPORT_WLAN_MT7615_V34)
    #define CONFIG_TX0_DSCP_NUM					(2000)
#else
    #define CONFIG_TX0_DSCP_NUM					(1536)
#endif    
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(16)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #define CONFIG_RX8_DSCP_NUM					(16)
    #define CONFIG_RX7_DSCP_NUM					(256)
    #define CONFIG_RX6_DSCP_NUM					(2048) /*WIFI-5G*/
    #define CONFIG_RX5_DSCP_NUM					(2048) /*WIFI-2.4G*/
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)
#endif
#endif


#if defined(TCSUPPORT_NP)
	#define CONFIG_RX1_DSCP_NUM 				(256) 
#else
#ifdef MT7615E	    
#if (defined(TCSUPPORT_VOIP) || defined(WAN_GPON) || defined(WAN_EPON))
    #define CONFIG_RX1_DSCP_NUM					(1024) 
#else /* TCSUPPORT_VOIP */
    #define CONFIG_RX1_DSCP_NUM					(896)
#endif /* TCSUPPORT_VOIP */
#else
    #define CONFIG_RX1_DSCP_NUM					(1024) 
#endif	
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_JOYME2) || defined(TCSUPPORT_CF_JOYMEV2_PON)
    #define CONFIG_RX0_DSCP_NUM					(512)
#else/*TCSUPPORT_COMPILE*/
    #define CONFIG_RX0_DSCP_NUM					(1024)
#endif/*TCSUPPORT_COMPILE*/

    #define CONFIG_DSCP_NUM_MAX                 (4096)
    #define CONFIG_IRQ_NUM_MAX                  (4095)
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_IRQ_DEPTH					(512)
	#define CONFIG_IRQ2_DEPTH					(512)
#else
    #define CONFIG_IRQ_DEPTH					(2048)
	#define CONFIG_IRQ2_DEPTH					(1024)
#endif
    #define CONFIG_MAX_PKT_LENS					(2048)
	#define HWFWD_LOW_THRESHOLD					(128)

#else // qdma2
    #define CONFIG_QDMA_QUEUE                   8
	
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_HWFWD_DSCP_NUM               (1000)	// for csr2 test
#else
#ifdef	TCSUPPORT_DSL_PHYMODE
    #define CONFIG_HWFWD_DSCP_NUM               (512)
#else
	#define CONFIG_HWFWD_DSCP_NUM				(8191)
#endif
#endif

#ifdef	TCSUPPORT_DSL_PHYMODE
    #define CONFIG_HWFWD_DSCP_NUM_MAX			(8191)
	 
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(256)
		
    #define CONFIG_RX15_DSCP_NUM				(2)
    #define CONFIG_RX14_DSCP_NUM				(2)
    #define CONFIG_RX13_DSCP_NUM				(2)
    #define CONFIG_RX12_DSCP_NUM				(2)
    #define CONFIG_RX11_DSCP_NUM				(2)
    #define CONFIG_RX10_DSCP_NUM				(2)
    #define CONFIG_RX9_DSCP_NUM					(2)
    #define CONFIG_RX8_DSCP_NUM					(2)
    #define CONFIG_RX7_DSCP_NUM					(2)
    #define CONFIG_RX6_DSCP_NUM					(2)
    #define CONFIG_RX5_DSCP_NUM					(2)
    #define CONFIG_RX4_DSCP_NUM					(2)
    #define CONFIG_RX3_DSCP_NUM					(2)
    #define CONFIG_RX2_DSCP_NUM					(2)
    #define CONFIG_RX1_DSCP_NUM					(128)
    #define CONFIG_RX0_DSCP_NUM					(128)
#else
    #define CONFIG_HWFWD_DSCP_NUM_MAX			(8191)
 
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(1536)
	
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(16)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #define CONFIG_RX8_DSCP_NUM					(16)
    #define CONFIG_RX7_DSCP_NUM					(16)
    #define CONFIG_RX6_DSCP_NUM					(16)
    #define CONFIG_RX5_DSCP_NUM					(16)
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)
    #define CONFIG_RX1_DSCP_NUM					(256)
    #define CONFIG_RX0_DSCP_NUM					(1024)
#endif

    #define CONFIG_DSCP_NUM_MAX                 (4096)
    #define CONFIG_IRQ_NUM_MAX                  (4095)
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_IRQ_DEPTH                    (512)
	#define CONFIG_IRQ2_DEPTH                   (512)
#else
    #define CONFIG_IRQ_DEPTH                    (2048)
	#define CONFIG_IRQ2_DEPTH                   (512)
#endif
    #define CONFIG_MAX_PKT_LENS                 (2048)
    #define HWFWD_LOW_THRESHOLD                 (256)
#endif

#define CONFIG_MAX_CNTR_NUM                 (40)	

#define CONFIG_FLOWCNT_MAX_DROUP_NUM            (2)
#define CONFIG_FLOWCNT_GRP0_MAX_IDX_NUM         (31)
#define CONFIG_FLOWCNT_GRP1_MAX_IDX_NUM         (16)
#define CONFIG_FLOWCNT_GRP2_MAX_IDX_NUM         (0)
#define GENERAL_INGRESS_INDEX_MAX               (127)   /*EN7580: 128 ratelimit. or 64 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP1               (31)   /*grp1: 32 ratelimit. or 16 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP2               (15)   /*grp2: 16 ratelimit. or 8 trtcm*/
#define GENERAL_SLA_INDEX_MAX					(31)    /*EN7581 || EN7523: SLA TRTCM INDEX, 2bit: SLA index, 3bit: queue index*/
#define SLA_MAX_NUM								(4)     /*EN7581 || EN7523: SLA support 4 channels at most*/
#define QUEUE_ALL_NUM                           (CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE)
#define TX_WRED_THR_NUM                         (5)
#define TX_WRED_PROBABILITY_NUM                 (4)

#define CONFIG_PAYLOAD_256_BYTE                 (256)
#define CONFIG_PAYLOAD_512_BYTE                 (512)
#define CONFIG_PAYLOAD_1K_BYTE                  (1024)
#define CONFIG_PAYLOAD_2K_BYTE                  (2048)

#ifdef __BIG_ENDIAN
#define GLB_CFG_VALUE GLB_CFG_DSCP_BYTE_SWAP | GLB_CFG_PAYLOAD_BYTE_SWAP | GLB_CFG_MSG_WORD_SWAP | ((VAL_BST_32_DWARD<<GLB_CFG_BST_SE_SHIFT)&GLB_CFG_BST_SE_MASK) | ((PREFER_TX1_FWD_TX0<<GLB_CFG_DMA_PREFERENCE_SHIFT)&GLB_CFG_DMA_PREFERENCE_MASK)
#else
#define GLB_CFG_VALUE GLB_CFG_PAYLOAD_BYTE_SWAP | GLB_CFG_MSG_WORD_SWAP | ((VAL_BST_32_DWARD<<GLB_CFG_BST_SE_SHIFT)&GLB_CFG_BST_SE_MASK) | ((PREFER_TX1_FWD_TX0<<GLB_CFG_DMA_PREFERENCE_SHIFT)&GLB_CFG_DMA_PREFERENCE_MASK)
#endif

#if defined(QDMA_LAN)
#define BUFF_TOTAL_MAX_THRH 0x3c00
#define BUFF_TOTAL_MIN_THRH 0x1300
#define BUFF_CHN_MAX_THRH 0x3c00
#define BUFF_CHN_MIN_THRH 0x10
#define BUFF_QUE_MAX_THRH 0x1300
#define BUFF_QUE_MIN_THRH gpQdmaPriv->hwFwdDscpNum/340
#else
#define BUFF_TOTAL_MAX_THRH 0x3c00
#define BUFF_TOTAL_MIN_THRH gpQdmaPriv->hwFwdDscpNum/10
#define BUFF_CHN_MAX_THRH 0x3c00
#define BUFF_CHN_MIN_THRH gpQdmaPriv->hwFwdDscpNum/28
#define BUFF_QUE_MAX_THRH gpQdmaPriv->hwFwdDscpNum/10
#define BUFF_QUE_MIN_THRH gpQdmaPriv->hwFwdDscpNum/340
#endif

#define OFFLOAD_FAST_TXRING_IDX 0

#define CSR_MAP_LENGTH 0x1000

#define XSI_FAST_HANDLER_NBQ 4
#define QOS_CHANNEL_MAX_IDX 10
/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define qdmaGetOffsetVal(base, offset)	IO_GREG(QDMA_CSR_OFFSET_ADDR(base, offset))

#define qdmaGetQdmaInfo(base)			IO_GREG(QDMA_CSR_INFO(base))
#define qdmaSetGlbCfg(base, val)		IO_SREG(QDMA_CSR_GLB_CFG(base), val)
#define qdmaGetGlbCfg(base)				IO_GREG(QDMA_CSR_GLB_CFG(base))
#define qdmaIsSetRx2bOffset(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_2B_OFFSET)
#define qdmaIsSetMsgWordSwap(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_MSG_WORD_SWAP)
#define qdmaIsSetDscpByteSwap(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_DSCP_BYTE_SWAP)
#define qdmaIsSetPayloadByteSwap(base)	(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_PAYLOAD_BYTE_SWAP)


#define qdmaSetLpbkToRXRing(base, idx)	IO_SMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_LPBK_RXQ_SEL_MASK, GLB_CFG_LPBK_RXQ_SEL_SHIFT, idx)

#define qdmaEnableVChnlMapEn(base)		IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_VCHNL_MAP_EN)
#define qdmaDisableVChnlMapEn(base)		IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_VCHNL_MAP_EN)
#define qdmaIsVChnlMapEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_VCHNL_MAP_EN)
#define qdmaSetVChnlMap32Queue(base)	IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_VCHNL_MAP_MODE)
#define qdmaSetVChnlMap16Queue(base)	IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_VCHNL_MAP_MODE)
#define qdmaIsVChnlMap32Queue(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_VCHNL_MAP_MODE)

#define qdmaIsEnableSLMRelease(base)	(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_SLM_RELEASE_EN)
#define qdmaEnableSLMRelease(base)		IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_SLM_RELEASE_EN)
#define qdmaDisableSLMRelease(base)		IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_SLM_RELEASE_EN)
#define qdmaIsEnableIrqEn(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_IRQ_EN)
#define qdmaEnableIrqEn(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_IRQ_EN)
#define qdmaDisableIrqEn(base)			IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_IRQ_EN)
#define qdmaIsEnableLoopCnt(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_LOOPCNT_EN)
#define qdmaEnableLoopCnt(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_LOOPCNT_EN)
#define qdmaDisableLoopCnt(base)		IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_LOOPCNT_EN)
#define qdmaEnableUmacLoopback(base)	IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_UMAC_LOOPBACK)
#define qdmaDisableUmacLoopback(base)	IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_UMAC_LOOPBACK)
#define qdmaEnableQdmaLoopback(base)	IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_QDMA_LOOPBACK)
#define qdmaDisableQdmaLoopback(base)	IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_QDMA_LOOPBACK)
#define qdmaEnableCheckDone(base)		IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_CHECK_DONE)
#define qdmaDisableCheckDone(base)		IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_CHECK_DONE)
#define qdmaIsSetCheckDone(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_CHECK_DONE)	
#define qdmaIsSetTxWbDone(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_WB_DONE)	
#define qdmaSetBurstSize(base, val)		IO_SMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_BST_SE_MASK, GLB_CFG_BST_SE_SHIFT, val)
#define qdmaGetBurstSize(base)			IO_GMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_BST_SE_MASK, GLB_CFG_BST_SE_SHIFT)
#define qdmaIsRxDmaBusy(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_DMA_BUSY)
#define qdmaIsSetRxDmaEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_DMA_EN)
#define qdmaEnableRxDma(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_RX_DMA_EN)
#define qdmaDisableRxDma(base)			IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_RX_DMA_EN)
#define qdmaIsTxDmaBusy(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_DMA_BUSY)
#define qdmaIsSetTxDmaEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_DMA_EN)
#define qdmaEnableTxDma(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_TX_DMA_EN)
#define qdmaDisableTxDma(base)			IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_TX_DMA_EN)

#define qdmaSetTxDscpBase(base, idx, val)	IO_SREG(QDMA_CSR_TX_DSCP_BASE(base, idx), val)
#define qdmaGetTxDscpBase(base, idx)		IO_GREG(QDMA_CSR_TX_DSCP_BASE(base, idx))
#define qdmaSetRxDscpBase(base, idx, val)	IO_SREG(QDMA_CSR_RX_DSCP_BASE(base, idx), val)
#define qdmaGetRxDscpBase(base, idx)		IO_GREG(QDMA_CSR_RX_DSCP_BASE(base, idx))
#define qdmaSetTxCpuIdx(base, idx, val)		IO_SMASK(QDMA_CSR_TX_CPU_IDX(base, idx), TX_CPU_IDX_MASK, TX_CPU_IDX_SHIFT, val)
#define qdmaGetTxCpuIdx(base, idx)			IO_GMASK(QDMA_CSR_TX_CPU_IDX(base, idx), TX_CPU_IDX_MASK, TX_CPU_IDX_SHIFT)
#define qdmaSetTxDmaIdx(base, idx, val)		IO_SMASK(QDMA_CSR_TX_DMA_IDX(base, idx), TX_DMA_IDX_MASK, TX_DMA_IDX_SHIFT, val)
#define qdmaGetTxDmaIdx(base, idx)			IO_GMASK(QDMA_CSR_TX_DMA_IDX(base, idx), TX_DMA_IDX_MASK, TX_DMA_IDX_SHIFT)
#define qdmaSetRxCpuIdx(base, idx, val)		IO_SMASK(QDMA_CSR_RX_CPU_IDX(base, idx), RX_CPU_IDX_MASK, RX_CPU_IDX_SHIFT, val)
#define qdmaGetRxCpuIdx(base, idx)			IO_GMASK(QDMA_CSR_RX_CPU_IDX(base, idx), RX_CPU_IDX_MASK, RX_CPU_IDX_SHIFT)
#define qdmaSetRxDmaIdx(base, idx, val)		IO_SMASK(QDMA_CSR_RX_DMA_IDX(base, idx), RX_DMA_IDX_MASK, RX_DMA_IDX_SHIFT, val)
#define qdmaGetRxDmaIdx(base, idx)			IO_GMASK(QDMA_CSR_RX_DMA_IDX(base, idx), RX_DMA_IDX_MASK, RX_DMA_IDX_SHIFT)

#define qdmaSetHwDscpBase(base, val)	IO_SREG(QDMA_CSR_HWFWD_DSCP_BASE(base), val)
#define qdmaGetHwDscpBase(base)			IO_GREG(QDMA_CSR_HWFWD_DSCP_BASE(base))
#define qdmaSetHwBuffBase(base, val)	IO_SREG(QDMA_CSR_HWFWD_BUFF_BASE(base), val)
#define qdmaGetHwBuffBase(base)			IO_GREG(QDMA_CSR_HWFWD_BUFF_BASE(base))
#define qdmaSetHwPayloadSize(base, val)	IO_SMASK(QDMA_CSR_HWFWD_DSCP_CFG(base), HWFWD_PAYLOAD_SIZE_MASK, HWFWD_PAYLOAD_SIZE_SHIFT, val)
#define qdmaGetHwPayloadSize(base)		IO_GMASK(QDMA_CSR_HWFWD_DSCP_CFG(base), HWFWD_PAYLOAD_SIZE_MASK, HWFWD_PAYLOAD_SIZE_SHIFT)

#define qdmaSetHwLowThrshld(base, val)	IO_SMASK(QDMA_CSR_HWFWD_DSCP_CFG(base), HWFWD_DSCP_LOW_THRSHLD_MASK, HWFWD_DSCP_LOW_THRSHLD_SHIFT, val)
#define qdmaGetHwLowThrshld(base)		IO_GMASK(QDMA_CSR_HWFWD_DSCP_CFG(base), HWFWD_DSCP_LOW_THRSHLD_MASK, HWFWD_DSCP_LOW_THRSHLD_SHIFT)

#define qdmaSetHWInitStart(base)		IO_SBITS(QDMA_CSR_LMGR_INIT_CFG(base), LMGR_INIT_START)
#define qdmaGetHWInitStart(base)		(IO_GREG(QDMA_CSR_LMGR_INIT_CFG(base)) & LMGR_INIT_START)

#define qdmaSetHwDscpNum(base, val)		IO_SMASK(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_DSCP_NUM_MASK, HWFWD_DSCP_NUM_SHIFT, val)
#define qdmaGetHwDscpNum(base)			IO_GMASK(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_DSCP_NUM_MASK, HWFWD_DSCP_NUM_SHIFT)
#define qdmaEnablePktSizeOverHead(base)		IO_SBITS(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_PKTSIZE_OVERHEAD_EN)
#define qdmaDisablePktSizeOverHead(base)	IO_CBITS(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_PKTSIZE_OVERHEAD_EN)
#define qdmaIsPktSizeOverHeadEnable(base)	(IO_GREG(QDMA_CSR_LMGR_INIT_CFG(base)) & HWFWD_PKTSIZE_OVERHEAD_EN)
#define qdmaSetPktSizeOverHeadVal(base, val)	IO_SMASK(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_PKTSIZE_OVERHEAD_VALUE_MASK, HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT, val)
#define qdmaGetPktSizeOverHeadVal(base)			IO_GMASK(QDMA_CSR_LMGR_INIT_CFG(base), HWFWD_PKTSIZE_OVERHEAD_VALUE_MASK, HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT)
#define qdmaSetRxDelayIntCfg(base, idx, val)	IO_SMASK(QDMA_CSR_RX_DELAY_INT_CFG(base, idx), DLY_INT_RXDLY_MASK(idx), DLY_INT_RXDLY_SHIFT(idx), val)
#define qdmaSetRxDelayIntVal(base, idx, val)			IO_SREG(QDMA_CSR_RX_DELAY_INT_CFG(base, idx), val)
#define qdmaGetRxDelayIntCfg(base, idx)				IO_GREG(QDMA_CSR_RX_DELAY_INT_CFG(base, idx))
//#define qdmaClearIntStatus(base, bit)	IO_SBITS(QDMA_CSR_INT_STATUS(base), bit)
//#define qdmaSetIntStatus(base, val)		IO_SREG(QDMA_CSR_INT_STATUS(base), val)
#define qdmaClearIntStatus1(base, val)	IO_SREG(QDMA_CSR_INT_STATUS1(base), val)
#define qdmaGetIntStatus1(base)			IO_GREG(QDMA_CSR_INT_STATUS1(base))
#define qdmaClearIntStatus2(base, val)	IO_SREG(QDMA_CSR_INT_STATUS2(base), val)
#define qdmaGetIntStatus2(base)			IO_GREG(QDMA_CSR_INT_STATUS2(base))
#define qdmaClearIntStatus(base, val)	IO_SREG(QDMA_CSR_INT_STATUS(base), val)
#define qdmaGetIntStatus(base)			IO_GREG(QDMA_CSR_INT_STATUS(base))

#define qdmaSetTxDelayIntCfg(base, idx, val)	IO_SMASK(QDMA_CSR_TX_DELAY_INT_CFG(base), DLY_INT_TXDLY_MASK(idx), DLY_INT_TXDLY_SHIFT(idx), val)
#define qdmaSetTxDelayIntVal(base, val)			IO_SREG(QDMA_CSR_TX_DELAY_INT_CFG(base), val)
#define qdmaGetTxDelayIntCfg(base)				IO_GREG(QDMA_CSR_TX_DELAY_INT_CFG(base))

#define qdmaSetIrqBase(base, val)		IO_SREG(QDMA_CSR_IRQ_BASE(base), val)
#define qdmaGetIrqBase(base)			IO_GREG(QDMA_CSR_IRQ_BASE(base))
#define qdmaSetIrqConfig(base, val)		IO_SREG(QDMA_CSR_IRQ_CFG(base), val)
#define qdmaGetIrqConfig(base)			IO_GREG(QDMA_CSR_IRQ_CFG(base))
#define qdmaSetIrqThreshold(base, val)	IO_SMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_THRESHOLD_MASK, IRQ_CFG_THRESHOLD_SHIFT, val)
#define qdmaGetIrqThreshold(base)		IO_GMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_THRESHOLD_MASK, IRQ_CFG_THRESHOLD_SHIFT)
#define qdmaSetIrqDepth(base, val)		IO_SMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_DEPTH_MASK, IRQ_CFG_DEPTH_SHIFT, val)
#define qdmaGetIrqDepth(base)			IO_GMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_DEPTH_MASK, IRQ_CFG_DEPTH_SHIFT)
#define qdmaSetIrqClearLen(base, val)	IO_SMASK(QDMA_CSR_IRQ_CLEAR_LEN(base), IRQ_CLEAR_LEN_MASK, IRQ_CLEAR_LEN_SHIFT, val)
#define qdmaGetIrqClearLen(base)		IO_GMASK(QDMA_CSR_IRQ_CLEAR_LEN(base), IRQ_CLEAR_LEN_MASK, IRQ_CLEAR_LEN_SHIFT)
#define qdmaGetIrqStatus(base)			IO_GREG(QDMA_CSR_IRQ_STATUS(base))
#define qdmaGetIrqEntryLen(base)		IO_GMASK(QDMA_CSR_IRQ_STATUS(base), IRQ_STATUS_ENTRY_LEN_MASK, IRQ_STATUS_ENTRY_LEN_SHIFT)
#define qdmaGetIrqHeadIdx(base)			IO_GMASK(QDMA_CSR_IRQ_STATUS(base), IRQ_STATUS_HEAD_IDX_MASK, IRQ_STATUS_HEAD_IDX_SHIFT)
#define qdmaSetIrqPtime(base, val)		IO_SMASK(QDMA_CSR_IRQ_PTIME(base), IRQ_PTIME_MASK, IRQ_PTIME_SHIFT, val)
#define qdmaGetIrqPtime(base)			IO_GMASK(QDMA_CSR_IRQ_PTIME(base), IRQ_PTIME_MASK, IRQ_PTIME_SHIFT)

#define qdmaEnableTxRateLimit(base)			IO_SBITS(QDMA_CSR_EGRESS_RATELIMIT_CFG(base), EGRESS_RATELIMIT_EN)
#define qdmaDisableTxRateLimit(base)		IO_CBITS(QDMA_CSR_EGRESS_RATELIMIT_CFG(base), EGRESS_RATELIMIT_EN)
#define qdmaIsTxRateLimitEnable(base)		(IO_GREG(QDMA_CSR_EGRESS_RATELIMIT_CFG(base)) & EGRESS_RATELIMIT_EN)
#define qdmaSetTxRateLimitBucketScale(base, val)	IO_SMASK(QDMA_CSR_EGRESS_RATELIMIT_CFG(base), EGRESS_RATELIMIT_BUCKETSCALE_MASK, EGRESS_RATELIMIT_BUCKETSCALE_SHIFT, val)
#define qdmaGetTxRateLimitBucketScale(base)			IO_GMASK(QDMA_CSR_EGRESS_RATELIMIT_CFG(base), EGRESS_RATELIMIT_BUCKETSCALE_MASK, EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)

#define qdmaSetTxRateLimitTicksel(base, ticksel_idx, val)		IO_SMASK((((ticksel_idx)==1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG(base) : QDMA_CSR_EGRESS_RATELIMIT_CFG2(base)),EGRESS_RATELIMIT_TICKSEL_MASK, EGRESS_RATELIMIT_TICKSEL_SHIFT, val)
#define qdmaGetTxRateLimitTicksel(base,ticksel_idx)	IO_GMASK((((ticksel_idx)==1) ? QDMA_CSR_EGRESS_RATELIMIT_CFG(base) : QDMA_CSR_EGRESS_RATELIMIT_CFG2(base)),EGRESS_RATELIMIT_TICKSEL_MASK, EGRESS_RATELIMIT_TICKSEL_SHIFT)
#define qdmaIsTxRateLimitChnlTickSel1(base,idx) 	(IO_GREG(QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL(base)) & (1<<(idx)))
#define qdmaSetTxRateLimitPerChnlTickSelIdx(base,idx,ticksel_idx)		IO_SMASK(QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL(base), 0x1<<(idx), idx, ticksel_idx)

#define qdmaSetTxQosWeightByPacket(base)	IO_CBITS(QDMA_CSR_TXWRR_MODE_CFG(base), TXWRR_WEIGHT_BASE)
#define qdmaSetTxQosWeightByByte(base)		IO_SBITS(QDMA_CSR_TXWRR_MODE_CFG(base), TXWRR_WEIGHT_BASE)
#define qdmaIsTxQosWeightByByte(base)		(IO_GREG(QDMA_CSR_TXWRR_MODE_CFG(base)) & TXWRR_WEIGHT_BASE)
#define qdmaSetTxQosWeightScale64(base)		IO_CBITS(QDMA_CSR_TXWRR_MODE_CFG(base), TXWRR_WEIGHT_SCALE)
#define qdmaSetTxQosWeightScale16(base)		IO_SBITS(QDMA_CSR_TXWRR_MODE_CFG(base), TXWRR_WEIGHT_SCALE)
#define qdmaIsTxQosWeightScale16(base)		(IO_GREG(QDMA_CSR_TXWRR_MODE_CFG(base)) & TXWRR_WEIGHT_SCALE)

#define qdmaEnablePrefetchOverDrag(base)	IO_SBITS(QDMA_CSR_DMA_PREFETCH_CFG(base), TXBUF_PREFETCH_OVERDRAG_EN)
#define qdmaDisablePrefetchOverDrag(base)	IO_CBITS(QDMA_CSR_DMA_PREFETCH_CFG(base), TXBUF_PREFETCH_OVERDRAG_EN)
#define qdmaSetPrefetchOverDragCnt(base, val)	IO_SMASK(QDMA_CSR_DMA_PREFETCH_CFG(base), TXBUF_PREFETCH_OVERDRAG_CNT_MASK, TXBUF_PREFETCH_OVERDRAG_CNT_SHIFT, val)
#define qdmaGetPrefetchOverDragCnt(base, val)	IO_GMASK(QDMA_CSR_DMA_PREFETCH_CFG(base), TXBUF_PREFETCH_OVERDRAG_CNT_MASK, TXBUF_PREFETCH_OVERDRAG_CNT_SHIFT, val)

#define qdmaEnableTxDmaPrefetch(base)		IO_SBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_PREFETCH_EN)
#define qdmaDisableTxDmaPrefetch(base)		IO_CBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_PREFETCH_EN)
#define qdmaEnableTxBufCtrl(base)			IO_SBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_CTRL_EN)
#define qdmaDisableTxBufCtrl(base)			IO_CBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_CTRL_EN)
#define qdmaIsTxBufCtrlEnable(base)			(IO_GREG(QDMA_CSR_PSE_BUF_USAGE_CFG(base)) & PSE_BUF_CTRL_EN)
#define qdmaEnableTxBufEstimate(base)		IO_SBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_ESTIMATE_EN)
#define qdmaDisableTxBufEstimate(base)		IO_CBITS(QDMA_CSR_PSE_BUF_USAGE_CFG(base), PSE_BUF_ESTIMATE_EN)
#define qdmaSetTxBufChnnelThreshold(base, val)	IO_SMASK(QDMA_CSR_PSE_BUF_USAGE_CFG(base), TXBUF_CHNL_THRSHLD_MASK, TXBUF_CHNL_THRSHLD_SHIFT, val)
#define qdmaGetTxBufChnnelThreshold(base)		IO_GMASK(QDMA_CSR_PSE_BUF_USAGE_CFG(base), TXBUF_CHNL_THRSHLD_MASK, TXBUF_CHNL_THRSHLD_SHIFT)
#define qdmaSetTxBufTotalThreshold(base, val)	IO_SMASK(QDMA_CSR_PSE_BUF_USAGE_CFG(base), TXBUF_TOTAL_THRSHLD_MASK, TXBUF_TOTAL_THRSHLD_SHIFT, val)
#define qdmaGetTxBufTotalThreshold(base)		IO_GMASK(QDMA_CSR_PSE_BUF_USAGE_CFG(base), TXBUF_TOTAL_THRSHLD_MASK, TXBUF_TOTAL_THRSHLD_SHIFT)

#define qdmaEnableGeneralTrtcm(trtcm_base)			IO_SBITS(trtcm_base, TRTCM_EN_MASK)
#define qdmaDisableGeneralTrtcm(trtcm_base)			IO_CBITS(trtcm_base, TRTCM_EN_MASK)
#define qdmaIsGeneralTrtcmEnable(trtcm_base)		IO_GMASK(trtcm_base, TRTCM_EN_MASK, TRTCM_EN_SHIFT)
#define qdmaSetGeneralTrtcmMode(trtcm_base,val)		IO_SMASK(trtcm_base, TRTCM_MODE_MASK, TRTCM_MODE_SHIFT, val)
#define qdmaGetGeneralTrtcmMode(trtcm_base)			IO_GMASK(trtcm_base, TRTCM_MODE_MASK, TRTCM_MODE_SHIFT)
#define qdmaSetGeneralFastTick(trtcm_base,val)		IO_SMASK(trtcm_base, TRTCM_FAST_TICK_MASK, TRTCM_FAST_TICK_SHIFT, val)
#define qdmaGetGeneralFastTick(trtcm_base)			IO_GMASK(trtcm_base, TRTCM_FAST_TICK_MASK, TRTCM_FAST_TICK_SHIFT)
#define qdmaSetGeneralSlowRatio(trtcm_base,val)		IO_SMASK(trtcm_base, TRTCM_SLOW_TICKRATIO_MASK, TRTCM_SLOW_TICKRATIO_SHIFT, val)
#define qdmaGetGeneralSlowRatio(trtcm_base)			IO_GMASK(trtcm_base, TRTCM_SLOW_TICKRATIO_MASK, TRTCM_SLOW_TICKRATIO_SHIFT)

/*WRED enable /disable*/
#define qdmaEnableTxWred(base)                              IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_WRED_EN)
#define qdmaDisableTxWred(base)                             IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_WRED_EN)
#define qdmaIsTxWredEnable(base)                            IO_GMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_WRED_EN, TXQ_CNGST_WRED_EN_SHIFT)
/*WRED Probability setting & getting*/
#define qdmaSetWredNormProbability(base, val)               IO_SREG(QDMA_CSR_INGRESS_WRED_NORM_PROBABILITY(base), val)
#define qdmaGetWredNormProbability(base)                    IO_GREG(QDMA_CSR_INGRESS_WRED_NORM_PROBABILITY(base))

#define qdmaSetWredNormProbabilitySingle(base, thrIdx, val) IO_SMASK(QDMA_CSR_INGRESS_WRED_NORM_PROBABILITY(base), WRED_PROBABILITY_MASK(thrIdx), WRED_PROBABILITY_SHIFT(thrIdx), val)
#define qdmaGetWredNormProbabilitySingle(base, thrIdx)      IO_GMASK(QDMA_CSR_INGRESS_WRED_NORM_PROBABILITY(base), WRED_PROBABILITY_MASK(thrIdx), WRED_PROBABILITY_SHIFT(thrIdx))

#define qdmaSetWredDeiProbability(base, val)                IO_SREG(QDMA_CSR_INGRESS_WRED_DEI_PROBABILITY(base), val)
#define qdmaGetWredDeiProbability(base)                     IO_GREG(QDMA_CSR_INGRESS_WRED_DEI_PROBABILITY(base))

#define qdmaSetWredDeiProbabilitySingle(base, thrIdx, val)  IO_SMASK(QDMA_CSR_INGRESS_WRED_DEI_PROBABILITY(base), WRED_PROBABILITY_MASK(thrIdx), WRED_PROBABILITY_SHIFT(thrIdx), val)
#define qdmaGetWredDeiProbabilitySingle(base, thrIdx)       IO_GMASK(QDMA_CSR_INGRESS_WRED_DEI_PROBABILITY(base), WRED_PROBABILITY_MASK(thrIdx), WRED_PROBABILITY_SHIFT(thrIdx))

/*WRED threshold setting & getting*/
#define qdmaSetWredNormThreshold(base, val)                 IO_SREG(QDMA_CSR_CNGST_WRED_NORM_CFG(base), val)
#define qdmaGetWredNormThreshold(base)                      IO_GREG(QDMA_CSR_CNGST_WRED_NORM_CFG(base))
#define qdmaSetWredNormThresholdSingle(base, thrIdx, val)   IO_SMASK(QDMA_CSR_CNGST_WRED_NORM_CFG(base), WRED_THR_MASK(thrIdx), WRED_THR_SHIFT(thrIdx), val)
#define qdmaGetWredNormThresholdSingle(base, thrIdx)        IO_GMASK(QDMA_CSR_CNGST_WRED_NORM_CFG(base), WRED_THR_MASK(thrIdx), WRED_THR_SHIFT(thrIdx))

#define qdmaSetWredDeiThreshold(base, val)                  IO_SREG(QDMA_CSR_CNGST_WRED_DEI_CFG(base), val)
#define qdmaGetWredDeiThreshold(base)                       IO_GREG(QDMA_CSR_CNGST_WRED_DEI_CFG(base))
#define qdmaSetWredDeiThresholdSingle(base, thrIdx, val)    IO_SMASK(QDMA_CSR_CNGST_WRED_DEI_CFG(base), WRED_THR_MASK(thrIdx), WRED_THR_SHIFT(thrIdx), val)
#define qdmaGetWredDeiThresholdSingle(base, thrIdx)         IO_GMASK(QDMA_CSR_CNGST_WRED_DEI_CFG(base), WRED_THR_MASK(thrIdx), WRED_THR_SHIFT(thrIdx))

/*rx RED Probability setting & getting*/
#define qdmaSetCpuRxRedNormProbability(base,val)            IO_SMASK(QDMA_CSR_CPU_RX_RED_CFG(base), CPU_RX_RED_NORM_PROBABILITY_MASK, CPU_RX_RED_NORM_SHIFT, val)
#define qdmaGetCpuRxRedNormProbability(base)                IO_GMASK(QDMA_CSR_CPU_RX_RED_CFG(base), CPU_RX_RED_NORM_PROBABILITY_MASK, CPU_RX_RED_NORM_SHIFT)
#define qdmaSetCpuRxRedDeiProbability(base,val)             IO_SMASK(QDMA_CSR_CPU_RX_RED_CFG(base), CPU_RX_RED_DEI_PROBABILITY_MASK, CPU_RX_RED_DEI_SHIFT, val)
#define qdmaGetCpuRxRedDeiProbability(base)                 IO_GMASK(QDMA_CSR_CPU_RX_RED_CFG(base), CPU_RX_RED_DEI_PROBABILITY_MASK, CPU_RX_RED_DEI_SHIFT)

#define qdmaEnableTxRateMeter(base)			IO_SBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_EN)
#define qdmaDisableTxRateMeter(base)		IO_CBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_EN)
#define qdmaIsTxRateMeterEnable(base)		(IO_GREG(QDMA_CSR_EGRESS_RATEMETER_CFG(base)) & EGRESS_RATEMETER_EN)
#define qdmaEnableTxRateMeterPeakRate(base)			IO_SBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_EN)
#define qdmaDisableTxRateMeterPeakRate(base)		IO_CBITS(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_EN)
#define qdmaIsTxRateMeterPeakRateEnable(base)		(IO_GREG(QDMA_CSR_EGRESS_RATEMETER_CFG(base)) & EGRESS_RATEMETER_PEEKRATE_EN)
#define qdmaSetTxPeekRateDuration(base, val)		IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_DURATION_MASK, EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT, val)
#define qdmaGetTxPeekRateDuration(base)			IO_GMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_PEEKRATE_DURATION_MASK, EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT)
#define qdmaSetTxRateMeterTimeDivisor(base, val)	IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMEDIVESEL_MASK, EGRESS_RATEMETER_TIMEDIVESEL_SHIFT, val)
#define qdmaGetTxRateMeterTimeDivisor(base)			IO_GMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMEDIVESEL_MASK, EGRESS_RATEMETER_TIMEDIVESEL_SHIFT)
#define qdmaSetTxRateMeterTimeSlice(base, val)		IO_SMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMESLICE_MASK, EGRESS_RATEMETER_TIMESLICE_SHIFT, val)
#define qdmaGetTxRateMeterTimeSlice(base)			IO_GMASK(QDMA_CSR_EGRESS_RATEMETER_CFG(base), EGRESS_RATEMETER_TIMESLICE_MASK, EGRESS_RATEMETER_TIMESLICE_SHIFT)

#define qdmaEnableTxqCngstDrop(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_DROP_EN)
#define qdmaDisableTxqCngstDrop(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_DROP_EN)
#define qdmaIsTxqCngstDropEnable(base)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_CNGST_DROP_EN)
#define qdmaEnableTxqCngstDeiDrop(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_DEI_DROP_EN)
#define qdmaDisableTxqCngstDeiDrop(base)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_CNGST_DEI_DROP_EN)
#define qdmaIsTxqCngstDeiDropEnable(base)	(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_CNGST_DEI_DROP_EN)
#define qdmaEnableTxqDynCngstEn(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_EN)
#define qdmaDisableTxqDynCngstEn(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_EN)
#define qdmaIsTxqDynCngstEnable(base)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_DYN_CNGSTCTL_EN)

#define qdmaEnableCngstMaxThrhEn(base, idx)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx==1)?TXQ_DYN_CNGSTCTL_MAX_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MAX_THRH_TXR0_EN))
#define qdmaDisableCngstMaxThrhEn(base, idx)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx==1)?TXQ_DYN_CNGSTCTL_MAX_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MAX_THRH_TXR0_EN))
#define qdmaIsCngstMaxThrhEnable(base, idx)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & ((idx==1)?TXQ_DYN_CNGSTCTL_MAX_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MAX_THRH_TXR0_EN))
#define qdmaEnableCngstMinThrhEn(base, idx)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx==1)?TXQ_DYN_CNGSTCTL_MIN_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MIN_THRH_TXR0_EN))
#define qdmaDisableCngstMinThrhEn(base, idx)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx==1)?TXQ_DYN_CNGSTCTL_MIN_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MIN_THRH_TXR0_EN))
#define qdmaIsCngstMinThrhEnable(base, idx)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & ((idx==1)?TXQ_DYN_CNGSTCTL_MIN_THRH_TXR1_EN:TXQ_DYN_CNGSTCTL_MIN_THRH_TXR0_EN))

#define qdmaSetDynCngstDeiThrhScale(base, val)	IO_SMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_MASK, TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_SHIFT, val)
#define qdmaGetDynCngstDeiThrhScale(base)		IO_GMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_MASK, TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_SHIFT)
#define qdmaSetTxPeekRateMargin(base, val)		IO_SMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_THR_MARGIN_MASK, TXQ_DYN_CNGSTCTL_THR_MARGIN_SHIFT, val)
#define qdmaGetTxPeekRateMargin(base)			IO_GMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_THR_MARGIN_MASK, TXQ_DYN_CNGSTCTL_THR_MARGIN_SHIFT)

#define qdmaEnableCngstCpuTxDropEn(base, idx)	IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx == RING_IDX_1)?TXQ_DYN_CNGSTCTL_CTX_RING1_DROP_EN:TXQ_DYN_CNGSTCTL_CTX_RING0_DROP_EN))
#define qdmaDisableCngstCpuTxDropEn(base, idx)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), ((idx == RING_IDX_1)?TXQ_DYN_CNGSTCTL_CTX_RING1_DROP_EN:TXQ_DYN_CNGSTCTL_CTX_RING0_DROP_EN))
#define qdmaIsCngstCpuTxDropEnable(base, idx)	(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & ((idx == RING_IDX_1)?TXQ_DYN_CNGSTCTL_CTX_RING1_DROP_EN:TXQ_DYN_CNGSTCTL_CTX_RING0_DROP_EN))

#define qdmaEnableCngstModeConfigTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER)
#define qdmaDisableCngstModeConfigTrig(base)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER)
#define qdmaIsCngstModeConfigTrigEnable(base)	(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER)
#define qdmaEnableCngstModePacketTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER)
#define qdmaDisableCngstModePacketTrig(base)	IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER)
#define qdmaIsCngstModePacketgTrigEnable(base)	(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER)
#define qdmaEnableCngstModeTimeTrig(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER)
#define qdmaDisableCngstModeTimeTrig(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER)
#define qdmaIsCngstModeTimeTrigEnable(base)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER)
#define qdmaSetTxqDynCngstTicksel(base, val)	IO_SMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_TICKSEL_MASK, TXQ_DYN_CNGSTCTL_TICKSEL_SHIFT, val)
#define qdmaGetTxqDynCngstTicksel(base)			IO_GMASK(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DYN_CNGSTCTL_TICKSEL_MASK, TXQ_DYN_CNGSTCTL_TICKSEL_SHIFT)


#define qdmaSetTxqDynTotalMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MAX_THR_MASK, TXQ_CNGST_TOTAL_MAX_THR_SHIFT, val)
#define qdmaGetTxqDynTotalMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MAX_THR_MASK, TXQ_CNGST_TOTAL_MAX_THR_SHIFT)
#define qdmaSetTxqDynTotalMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MIN_THR_MASK, TXQ_CNGST_TOTAL_MIN_THR_SHIFT, val)
#define qdmaGetTxqDynTotalMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_TOTALTHR(base), TXQ_CNGST_TOTAL_MIN_THR_MASK, TXQ_CNGST_TOTAL_MIN_THR_SHIFT)

#define qdmaSetTxqDynChnlMaxThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MAX_THR_MASK, TXQ_CNGST_CHNL_MAX_THR_SHIFT, val)
#define qdmaGetTxqDynChnlMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MAX_THR_MASK, TXQ_CNGST_CHNL_MAX_THR_SHIFT)
#define qdmaSetTxqDynChnlMinThrh(base, val)		IO_SMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MIN_THR_MASK, TXQ_CNGST_CHNL_MIN_THR_SHIFT, val)
#define qdmaGetTxqDynChnlMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base), TXQ_CNGST_CHNL_MIN_THR_MASK, TXQ_CNGST_CHNL_MIN_THR_SHIFT)

#define qdmaSetTxqDynQueueMaxThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MAX_THR_MASK, TXQ_CNGST_QUEUE_MAX_THR_SHIFT, val)
#define qdmaGetTxqDynQueueMaxThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MAX_THR_MASK, TXQ_CNGST_QUEUE_MAX_THR_SHIFT)
#define qdmaSetTxqDynQueueMinThrh(base, val)	IO_SMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MIN_THR_MASK, TXQ_CNGST_QUEUE_MIN_THR_SHIFT, val)
#define qdmaGetTxqDynQueueMinThrh(base)			IO_GMASK(QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base), TXQ_CNGST_QUEUE_MIN_THR_MASK, TXQ_CNGST_QUEUE_MIN_THR_SHIFT)

#define qdmaSetTxqStaticQueueDeiThrh(base, idx, val)		IO_SMASK(QDMA_CSR_STATIC_QUEUE_THR(base, idx), TXQ_STATIC_QUEUE_DEI_THR_MASK, TXQ_STATIC_QUEUE_DEI_THR_SHIFT, val)
#define qdmaGetTxqStaticQueueDeiThrh(base, idx)				IO_GMASK(QDMA_CSR_STATIC_QUEUE_THR(base, idx), TXQ_STATIC_QUEUE_DEI_THR_MASK, TXQ_STATIC_QUEUE_DEI_THR_SHIFT)
#define qdmaSetTxqStaticQueueNormalThrh(base, idx, val)		IO_SMASK(QDMA_CSR_STATIC_QUEUE_THR(base, idx), TXQ_STATIC_QUEUE_NORM_THR_MASK, TXQ_STATIC_QUEUE_NORM_THR_SHIFT, val)
#define qdmaGetTxqStaticQueueNormalThrh(base, idx)			IO_GMASK(QDMA_CSR_STATIC_QUEUE_THR(base, idx), TXQ_STATIC_QUEUE_NORM_THR_MASK, TXQ_STATIC_QUEUE_NORM_THR_SHIFT)

#define qdmaEnableTxqCngstQueueNonblocking(base, idx)		IO_CBITS(QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG(base), TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx))
#define qdmaDisableTxqCngstQueueNonblocking(base, idx)		IO_SBITS(QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG(base), TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx))
#define qdmaIsTxqCngstQueueNonblockingEnable(base, idx)	       (IO_GREG(QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG(base)) & TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx))

#define qdmaEnableTxqCngstChannelNonblocking(base, idx)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG(base), TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx))
#define qdmaDisableTxqCngstChannelNonblocking(base, idx)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG(base), TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx))
#define qdmaIsTxqCngstChannelNonblockingEnable(base, idx)	       (IO_GREG(QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG(base)) & TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx))

#define qdmaLmgrFreeCount(base)				IO_GMASK(QDMA_CSR_DBG_LMGR_STATUS(base), DBG_LMGR_FREE_CNT_MASK, DBG_LMGR_FREE_CNT_SHIFT)

#define qdmaLmgrTotalBufUsage(base)			IO_GMASK(QDMA_CSR_DBG_LMGR_USAGE(base), DBG_LMGR_TOTALBUF_USAGE_MASK, DBG_LMGR_TOTALBUF_USAGE_SHIFT)

#define qdmaSetRxRingSize(base, idx, val)	IO_SMASK(((idx&0xE)?(QDMA_CSR_RX_RING_SIZE_2_15(base,idx)):(QDMA_CSR_RX_RING_CFG(base))), RX_RING_SIZE_MASK(idx), RX_RING_SIZE_SHIFT(idx), val)
#define qdmaGetRxRingSize(base, idx)		IO_GMASK(((idx&0xE)?(QDMA_CSR_RX_RING_SIZE_2_15(base,idx)):(QDMA_CSR_RX_RING_CFG(base))), RX_RING_SIZE_MASK(idx), RX_RING_SIZE_SHIFT(idx))
#define qdmaSetRxRingThrh(base, idx, val)	IO_SMASK(((idx&0xE)?(QDMA_CSR_RX_RING_SIZE_2_15(base,idx)):(QDMA_CSR_RX_RING_THR(base))), RX_RING_LOW_THR_MASK(idx), RX_RING_LOW_THR_SHIFT(idx), val)
#define qdmaGetRxRingThrh(base, idx)		IO_GMASK(((idx&0xE)?(QDMA_CSR_RX_RING_SIZE_2_15(base,idx)):(QDMA_CSR_RX_RING_THR(base))), RX_RING_LOW_THR_MASK(idx), RX_RING_LOW_THR_SHIFT(idx))

#define qdmaEnableRxDrop(base)				IO_SBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_DROP_EN)
#define qdmaDisableRxDrop(base)				IO_CBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_DROP_EN)
#define qdmaIsRxDropEnable(base)			(IO_GREG(QDMA_CSR_RX_PROTECT_CFG(base)) & RX_DROP_EN)
#define qdmaEnableRxRateLimit(base)			IO_SBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_EN)
#define qdmaDisableRxRateLimit(base)		IO_CBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_EN)
#define qdmaIsRxRateLimitEnable(base)		(IO_GREG(QDMA_CSR_RX_PROTECT_CFG(base)) & RX_RATELIMIT_EN)
#define qdmaSetRxRateLimitByPacket(base)		IO_SBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_PKTMODE)
#define qdmaSetRxRateLimitByByte(base)			IO_CBITS(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_PKTMODE)
#define qdmaIsRxRateLimitByPacket(base)			(IO_GREG(QDMA_CSR_RX_PROTECT_CFG(base)) & RX_RATELIMIT_PKTMODE)
#define qdmaSetRxRateLimitBucketScale(base, val)	IO_SMASK(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_BUCKET_SCALE_MASK, RX_RATELIMIT_BUCKET_SCALE_SHIFT, val)
#define qdmaGetRxRateLimitBucketScale(base)			IO_GMASK(QDMA_CSR_RX_PROTECT_CFG(base), RX_RATELIMIT_BUCKET_SCALE_MASK, RX_RATELIMIT_BUCKET_SCALE_SHIFT)
#define qdmaSetRxRateLimitTickVal(base, tickSel, tickVal)	IO_SMASK(((tickSel==1)? (QDMA_CSR_RX_PROTECT_CFG(base)):(QDMA_CSR_RX_TICK_CFG(base))), RX_RATELIMIT_TICKSEL_MASK, RX_RATELIMIT_TICKSEL_SHIFT, tickVal)
#define qdmaGetRxRateLimitTicksel(base, tickSel)			IO_GMASK(((tickSel==1)? (QDMA_CSR_RX_PROTECT_CFG(base)):(QDMA_CSR_RX_TICK_CFG(base))), RX_RATELIMIT_TICKSEL_MASK, RX_RATELIMIT_TICKSEL_SHIFT)
#define qdmaSetRxRatelimitSelTick(base, tickSel, rxIdx)		IO_SMASK(QDMA_CSR_RX_TICK_CFG(base), RX_RATELIMIT_SELTICKER_MASK(rxIdx), RX_RATELIMIT_SELTICKER_SHIFT(rxIdx), tickSel)
#define qdmaGetRxRatelimitSelTick(base, rxIdx)				IO_GMASK(QDMA_CSR_RX_TICK_CFG(base), RX_RATELIMIT_SELTICKER_MASK(rxIdx), RX_RATELIMIT_SELTICKER_SHIFT(rxIdx))
#define qdmaIsRxBindTicker1(base,rxIdx)						(IO_GREG(QDMA_CSR_RX_TICK_CFG(base)) & (1<<rxIdx))
#define qdmaSetRxBindToTick1Specified(base, val) 			IO_SMASK(QDMA_CSR_RX_TICK_CFG(base), (0xFFFF<<16), 16, val)
#define qdmaSetRxBindToTick0Specified(base, val) 			IO_CBITS(QDMA_CSR_RX_TICK_CFG(base), val<<16)
#define qdmaDisableRxPktOverHead(base)						IO_CBITS(QDMA_CSR_RX_PKTOVH_CFG(base), CPU_RX_PKTSIZE_OVERHEAD_EN)
#define qdmaEnableRxPktOverHead(base)						IO_SBITS(QDMA_CSR_RX_PKTOVH_CFG(base), CPU_RX_PKTSIZE_OVERHEAD_EN)
#define qdmaSetRxPkTOverHead(base, val)						IO_SMASK(QDMA_CSR_RX_PKTOVH_CFG(base), CPU_RX_PKTSIZE_OVERHEAD_MASK, CPU_RX_PKTSIZE_OVERHEAD_SHIFT, val)
#define qdmaGetRxPkTOverHead(base)							IO_GMASK(QDMA_CSR_RX_PKTOVH_CFG(base), CPU_RX_PKTSIZE_OVERHEAD_MASK, CPU_RX_PKTSIZE_OVERHEAD_SHIFT)

#define qdmaEnableChnlRateLimit(base, idx)		IO_SBITS(QDMA_CSR_RATELIMIT_EN_CFG(base), TX_CHNL_RATELIMIT_EN(idx))
#define qdmaDisableChnlRateLimit(base, idx)		IO_CBITS(QDMA_CSR_RATELIMIT_EN_CFG(base), TX_CHNL_RATELIMIT_EN(idx))
#define qdmaIsChnlRateLimitEnable(base, idx)	(IO_GREG(QDMA_CSR_RATELIMIT_EN_CFG(base)) & TX_CHNL_RATELIMIT_EN(idx))

#define qdmaSetQosMode(base, idx, val)				IO_SREG(QDMA_CSR_PERCHNL_QOS_MODE(base, idx), val)
#define qdmaSetPerChnlQosMode(base, chnl, val)		IO_SMASK(QDMA_CSR_PERCHNL_QOS_MODE(base, (chnl>>3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), TXQOS_CHNL_QOS_MODE_SHIFT(chnl), val)
#define qdmaGetPerChnlQosMode(base, chnl)			IO_GMASK(QDMA_CSR_PERCHNL_QOS_MODE(base, (chnl>>3)), TXQOS_CHNL_QOS_MODE_MASK(chnl), TXQOS_CHNL_QOS_MODE_SHIFT(chnl))

#define qdmaGetTxPerChnlDataRate(base, chnl)		IO_GMASK(QDMA_CSR_PERCHNL_DATARATE(base, (chnl>>1)), TXQOS_CHNL_EGRESS_DATARATE_MASK(chnl), TXQOS_CHNL_EGRESS_DATARATE_SHIFT(chnl))

#define qdmaIsCntrEnable(base, idx)			(IO_GREG(QDMA_CSR_DBG_CNTR_CFG(base, idx)) & DBG_CNTR_EN)
#define qdmaIsCntrChnlAll(base, idx)		(IO_GREG(QDMA_CSR_DBG_CNTR_CFG(base, idx)) & DBG_CNTR_ALL_CHNL)
#define qdmaIsCntrQueueAll(base, idx)		(IO_GREG(QDMA_CSR_DBG_CNTR_CFG(base, idx)) & DBG_CNTR_ALL_QUEUE)
#define qdmaIsCntrDscpRingAll(base, idx)	(IO_GREG(QDMA_CSR_DBG_CNTR_CFG(base, idx)) & DBG_CNTR_ALL_DSCPRING)
#define qdmaEnableCntrCfg(base, idx)		IO_SBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_EN)
#define qdmaDisableCntrCfg(base, idx)		IO_CBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_EN)
#define qdmaGetCntrSrc(base, idx)			IO_GMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_SRC_MASK, DBG_CNTR_SRC_SHIFT)
#define qdmaSetCntrSrc(base, idx, val)		IO_SMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_SRC_MASK, DBG_CNTR_SRC_SHIFT, val)
#define qdmaSetCntrChnlAll(base, idx)		IO_SBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_CHNL)
#define qdmaSetCntrChnlSingle(base, idx)	IO_CBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_CHNL)
#define qdmaSetCntrQueueAll(base, idx)		IO_SBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_QUEUE)
#define qdmaSetCntrQueueSingle(base, idx)	IO_CBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_QUEUE)
#define qdmaSetCntrDscpAll(base, idx)		IO_SBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_DSCPRING)
#define qdmaSetCntrDscpSingle(base, idx)	IO_CBITS(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_ALL_DSCPRING)
#define qdmaGetCntrDscpIdx(base, idx)		IO_GMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_DSCPRING_IDX_MASK, DBG_CNTR_DSCPRING_IDX_SHIFT)
#define qdmaSetCntrDscpIdx(base, idx, val)	IO_SMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_DSCPRING_IDX_MASK, DBG_CNTR_DSCPRING_IDX_SHIFT, val)
#define qdmaGetCntrChnlIdx(base, idx)		IO_GMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_CHNL_MASK, DBG_CNTR_CHNL_SHIFT)
#define qdmaSetCntrChnlIdx(base, idx, val)	IO_SMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_CHNL_MASK, DBG_CNTR_CHNL_SHIFT, val)
#define qdmaGetCntrQueueIdx(base, idx)		IO_GMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_QUEUE_MASK, DBG_CNTR_QUEUE_SHIFT)
#define qdmaSetCntrQueueIdx(base, idx, val)	IO_SMASK(QDMA_CSR_DBG_CNTR_CFG(base, idx), DBG_CNTR_QUEUE_MASK, DBG_CNTR_QUEUE_SHIFT, val)
#define qdmaGetCntrCfg(base, idx)			IO_GREG(QDMA_CSR_DBG_CNTR_CFG(base, idx))
#define qdmaGetCntrCounter(base, idx)		IO_GREG(QDMA_CSR_DBG_CNTR_VAR(base, idx))
#define qdmaClearCntrCounter(base, idx)		IO_SREG(QDMA_CSR_DBG_CNTR_VAR(base, idx), 0)

/* 0: means cnt group_0 ; 1: means cnt group_1 */
#define qdmaIsFlowCntIncDropEnable(base, grp)       (IO_GREG(QDMA_CSR_FLOWCNT_GRP_CFG(base)) & (FLOWCNT_INC_DROP_ENABLE<<grp))
#define qdmaEnableFlowCntIncDrop(base, grp)         IO_SBITS(QDMA_CSR_FLOWCNT_GRP_CFG(base), (FLOWCNT_INC_DROP_ENABLE<<grp))
#define qdmaDisableFlowCntIncDrop(base, grp)        IO_CBITS(QDMA_CSR_FLOWCNT_GRP_CFG(base), (FLOWCNT_INC_DROP_ENABLE<<grp))

/*cnt grp0 index: 0~30 ; cnt grp1 index: 0~15*/
#define qdmaGetFlowCntByteLow(base, grp, idx)       IO_GREG(QDMA_CSR_FLOW_CNT_BYTE_LOW(base, grp, idx))
#define qdmaGetFlowCntByteHigh(base, grp, idx)      IO_GREG(QDMA_CSR_FLOW_CNT_BYTE_HIGH(base, grp, idx))
#define qdmaClearFlowCntByte(base, grp, idx)        IO_SREG(QDMA_CSR_FLOW_CNT_BYTE_LOW(base, grp, idx), 1)
#define qdmaGetFlowCntPkt(base, grp, idx)           IO_GREG(QDMA_CSR_FLOW_CNT_PKTCNT(base, grp, idx))
#define qdmaClearFlowCntPkt(base, grp, idx)         IO_SREG(QDMA_CSR_FLOW_CNT_PKTCNT(base, grp, idx), 1)

#define qdmaIsChannelEmpty(base,chnl)               ((IO_GREG(QDMA_CSR_CHNL_QUEUE_EMPTY(base, chnl)) & DBG_CHANNEL_EMPTY_MASK(chnl))>> DBG_CHANNEL_EMPTY_SHIFT(chnl))

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
typedef struct {
	uint 	resv1 ;
	struct {
#ifdef __BIG_ENDIAN
		uint done				: 1 ;
		uint overflow			: 1 ;
		uint nls				: 1	;
		uint resv2				: 13 ;
		uint pkt_len			: 16 ;
#else
		uint pkt_len			: 16 ;
		uint resv2				: 13 ;
		uint nls				: 1	;
		uint overflow			: 1 ;
		uint done				: 1 ;
#endif /* __BIG_ENDIAN */
	} ctrl ;
	uint 	pkt_addr ;
#ifdef __BIG_ENDIAN
	uint resv3					: 20 ;
	uint next_idx				: 12 ;
#else
	uint next_idx				: 12 ;
	uint resv3					: 20 ;
#endif /* __BIG_ENDIAN */
	uint msg[QDMA_DSCP_MSG_WORD_LENS] ;
} QDMA_DMA_DSCP_T ;

typedef struct {
	uint tcp_ts_rply;
	uint resv1;
	uint resv2;
	uint tcp_ack_seq;
} LRO_RX_DSCP_T ;

#if 0
typedef struct {
	uint word[QDMA_DSCP_MSG_TOTAL_WORD_LENS] ;
} QDMA_DMA_DSCP_MSG_T ;
#endif

typedef struct {
	uint 	pkt_addr ;
	struct {
#ifdef __BIG_ENDIAN
		uint ctx				: 1 ;
		uint resv				: 2 ;
		uint ctx_ring			: 1 ;
		uint ctx_idx			: 12 ;
		uint pkt_len			: 16 ;
#else
		uint pkt_len			: 16 ;
		uint ctx_idx			: 12 ;
		uint ctx_ring			: 1 ;
		uint resv				: 2 ;
		uint ctx				: 1 ;
#endif /* __BIG_ENDIAN */
	} ctrl ;
	uint msg[2] ;
} QDMA_HWFWD_DMA_DSCP_T ;

typedef union
{
    struct
    {
#ifdef __BIG_ENDIAN
		uint              : 4;
		uint sp_tag       : 16;
		uint oam          : 1;
		uint channel      : 8;
		uint queue        : 3;
#else
		uint queue        : 3;
		uint channel      : 8;
		uint oam          : 1;
		uint sp_tag       : 16;
		uint              : 4;
#endif
    };
    uint word;
} txMsgWord0_t;

typedef union
{
    struct
    {
#ifdef __BIG_ENDIAN
		uint ico		: 1;
		uint uco		: 1;
		uint tco		: 1;
		uint tso		: 1;
		uint            : 6;
		uint fport		: 3;
		uint 			: 19;
#else
		uint            : 19;
		uint fport		: 3;
		uint 			: 6;
		uint tso		: 1;
		uint tco		: 1;
		uint uco		: 1;
		uint ico		: 1;
#endif /* __BIG_ENDIAN */
    };
    uint word;
} txMsgWord1_t;

typedef union
{
    struct
    {
        uint    		: 32 ;
    };
    uint word;
} txMsgWord2_t;

typedef struct txMsgWord_s{
	txMsgWord0_t *pTxMsgW0;
	txMsgWord1_t *pTxMsgW1;
    txMsgWord2_t *pTxMsgW2;
} txMsgWord_t;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 rsv			:20;
		uint32 oam		    :1;
		uint32 channel		:8;
		uint32 longf		:1;
		uint32 runtf		:1;
		uint32 crcer		:1;
#else
		uint32 crcer		:1;
		uint32 runtf		:1;
		uint32 longf		:1;
		uint32 channel		:8;
		uint32 oam		    :1;
        uint32 rsv			:20;
#endif
	}raw;
    uint32 word;
} rxMsgWord0_t;

typedef union
{
    struct {
#ifdef __BIG_ENDIAN
        uint32 		    	:3;
        uint32 ip6			:1;
        uint32 ip4			:1;
        uint32 ip4f         :1;
        uint32 l2tp         :1;
        uint32 l4vld        :1;
        uint32 l4f          :1;
        uint32 sport        :4;
        uint32 crsn         :5;
        uint32 ppe_entry    :14;
#else
        uint32 ppe_entry    :14;
        uint32 crsn         :5;
        uint32 sport        :5;
        uint32 l4f          :1;
        uint32 l4vld        :1;
        uint32 ip4f         :1;
        uint32 ip4			:1;
        uint32 ip6			:1;
        uint32 dei			:1;
        uint32  			:3;
#endif
    }raw;
    uint32 word;
} rxMsgWord1_t;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32				:24;
		uint32 l2_len		:7;
        uint32 untag		:1;
#else
        uint32 untag		:1;
		uint32 l2_len		:7;
		uint32				:24;
#endif
    }raw;
    uint32 word;
} rxMsgWord2_t;

typedef union
{
	uint32  resv;
    uint32  word;
} rxMsgWord3_t;

typedef struct rxMsgWord_s
{
    rxMsgWord0_t rxMsgW0;
    rxMsgWord1_t rxMsgW1;
    rxMsgWord2_t rxMsgW2;
    rxMsgWord3_t rxMsgW3;
}rxMsgWord_t;
/************************************************************************
        I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
static inline void qdma_init_txmsg(txMsgWord_t* txMsg,uint mode)
{
    txMsg->pTxMsgW0->word = 0;
    txMsg->pTxMsgW1->word = 0;
    txMsg->pTxMsgW2->word = 0;

    return;
}

static inline void qdma_set_txmsg_whole_txmsg0(txMsgWord_t* txMsg,uint msg0)
{
    txMsg->pTxMsgW0->word = msg0;
    
    return;
}

static inline void qdma_set_txmsg_whole_txmsg1(txMsgWord_t* txMsg,uint msg1)
{
    txMsg->pTxMsgW1->word = msg1;
    
    return;
}

static inline void qdma_set_txmsg_whole_txmsg2(txMsgWord_t* txMsg,uint msg2)
{
    txMsg->pTxMsgW2->word = msg2;
    
    return;
}

static inline void qdma_set_txmsg_mic_idx(txMsgWord_t* txMsg,uint idx)
{    
    return;
}

static inline void qdma_set_txmsg_sptag(txMsgWord_t* txMsg,uint sptag)
{
    txMsg->pTxMsgW0->gem_port_id = sptag&0xffff;

    return;
}

static inline void qdma_set_txmsg_gemport(txMsgWord_t* txMsg,uint gem)
{
    txMsg->pTxMsgW0->gem_port_id = gem&0xffff;

    return;
}

static inline void qdma_set_txmsg_ico(txMsgWord_t* txMsg,uint ico)
{
    txMsg->pTxMsgW1->ico = ico&0x1;

    return;
}

static inline void qdma_set_txmsg_uco(txMsgWord_t* txMsg,uint uco)
{
    txMsg->pTxMsgW1->uco = uco&0x1;

    return;
}

static inline void qdma_set_txmsg_tco(txMsgWord_t* txMsg,uint tco)
{
    txMsg->pTxMsgW1->tco = tco&0x1;

    return;
}

static inline void qdma_set_txmsg_tso(txMsgWord_t* txMsg,uint tso)
{
    txMsg->pTxMsgW1->tso= tso&0x1;

    return;
}

static inline void qdma_set_txmsg_fast(txMsgWord_t* txMsg,uint fast)
{
    return;
}

static inline void qdma_set_txmsg_oam(txMsgWord_t* txMsg,uint oam)
{
    txMsg->pTxMsgW0->oam = oam&0x1;
    
    return;
}

static inline void qdma_set_txmsg_chn(txMsgWord_t* txMsg,uint chn)
{
    txMsg->pTxMsgW0->channel = chn&0xff;

    return;
}

static inline void qdma_set_txmsg_qid(txMsgWord_t* txMsg,uint qid)
{
    txMsg->pTxMsgW0->queue = qid&0x7;

    return;
}

static inline void qdma_set_txmsg_nodrop(txMsgWord_t* txMsg,uint nodrop)
{
    return;
}

static inline void qdma_set_txmsg_mtr0(txMsgWord_t* txMsg,uint mtr0)
{    
    return;
}

static inline void qdma_set_txmsg_fport(txMsgWord_t* txMsg,uint fport)
{
    txMsg->pTxMsgW1->fport = fport&0x7;
    
    return;
}

static inline void qdma_set_txmsg_nbq(txMsgWord_t* txMsg,uint nbq)
{    
    return;
}

static inline void qdma_set_txmsg_hwf(txMsgWord_t* txMsg,uint hwf)
{    
    return;
}

static inline void qdma_set_txmsg_hop(txMsgWord_t* txMsg,uint hop)
{    
    return;
}

static inline void qdma_set_txmsg_ptp(txMsgWord_t* txMsg,uint ptp)
{    
    return;
}

static inline void qdma_set_txmsg_acnt0(txMsgWord_t* txMsg,uint acnt0)
{    
    return;
}

static inline void qdma_set_txmsg_acnt1(txMsgWord_t* txMsg,uint acnt1)
{    
    return;
}

static inline void qdma_set_txmsg_sw_udf(txMsgWord_t* txMsg,uint udf)
{    
    return;
}

static inline void qdma_set_txmsg_mtr1(txMsgWord_t* txMsg,uint mtr1)
{    
    return;
}

static inline void qdma_set_txmsg_mtr2(txMsgWord_t* txMsg,uint mtr2)
{    
    return;
}

static inline void qdma_set_txmsg_acnt2(txMsgWord_t* txMsg,uint acnt2)
{    
    return;
}

static inline void qdma_init_trtcm(uint base,uint* p_cfgBase,uint* p_bucketByteUnit,uint* p_bucketPacketUnit)
{
    p_cfgBase[GLB_RATECTL]=QDMA_CSR_GLB_RATECTL_CFG(base);
	p_bucketByteUnit[GLB_RATECTL] = 256;
	p_bucketPacketUnit[GLB_RATECTL] = 256;
    
    return;
}

/*rx msg*/
static inline void qdma_get_rxmsg_whole_rxmsg0(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW0.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg1(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg2(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.word;
    
    return;
}

static inline void qdma_get_rxmsg_whole_rxmsg3(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW3.word;
    
    return;
}

static inline void qdma_get_rxmsg_hopflags(rxMsgWord_t* pRxMsg,uint *pValue)
{
    return;
}

static inline void qdma_get_rxmsg_ptp(rxMsgWord_t* pRxMsg,uint *pValue)
{ 
    return;
}

static inline void qdma_get_rxmsg_sptag(rxMsgWord_t* pRxMsg,uint *pValue)
{    
    return;
}

static inline void qdma_get_rxmsg_foe_index(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ppe_entry;
    
    return;
}

static inline void qdma_get_rxmsg_crsn(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.crsn;
    
    return;
}

static inline void qdma_get_rxmsg_sport(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.sport;
    
    return;
}

static inline void qdma_get_rxmsg_ip4f(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip4f;
    
    return;
}

static inline void qdma_get_rxmsg_l4f(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.l4f;
    
    return;
}

static inline void qdma_get_rxmsg_l4vld(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.l4vld;
    
    return;
}

static inline void qdma_get_rxmsg_ip4(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip4;
    
    return;
}

static inline void qdma_get_rxmsg_ip6(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW1.raw.ip6;
    
    return;
}

static inline void qdma_get_rxmsg_l2_len(rxMsgWord_t* pRxMsg,uint *pValue)
{
    *pValue = pRxMsg->rxMsgW2.raw.l2_len;
    
    return;
}

static inline void qdma_get_rxmsg_ifc_hit(rxMsgWord_t* pRxMsg,uint *pValue)
{    
    return;
}

static inline void qdma_get_rxmsg_ifc_id(rxMsgWord_t* pRxMsg,uint *pValue)
{    
    return;
}

static inline void qdma_get_rxmsg_swudf(rxMsgWord_t* pRxMsg,uint *pValue)
{
    return;
}

/* disable tx buffer estimate function to avoid qmda counting error issue, 20150912 */
/* now we should enable this function to avoid channel retire fail issue, 20151117 */
static inline void qdma_set_txbuf_estimate(uint base)
{
    #ifdef QDMA_LAN
    	qdmaDisableTxBufEstimate(base);
    #else	
    	qdmaEnableTxBufEstimate(base) ;
    #endif
    
    return;
}

static inline int qdma_set_txbuf_threshold_inline(unsigned int base,QDMA_Mode_t mode,unsigned char chnThrh,unsigned char totalThrh)
{
    if(QDMA_ENABLE == mode) {
        qdmaEnableTxBufCtrl(base) ;

        qdmaSetTxBufChnnelThreshold(base, chnThrh) ;
        qdmaSetTxBufTotalThreshold(base, totalThrh) ;
    } else if(QDMA_DISABLE == mode) {
        qdmaDisableTxBufCtrl(base) ;
    } else {
        return -EINVAL ;
    }

	return 0 ;
}

static inline int qdma_get_txbuf_threshold_inline(unsigned int base,QDMA_Mode_t *p_mode,unsigned char *p_chnThrh,unsigned char *p_totalThrh)
{
    *p_mode = (qdmaIsTxBufCtrlEnable(base)?(QDMA_ENABLE):(QDMA_DISABLE));
	*p_chnThrh = qdmaGetTxBufChnnelThreshold(base);
	*p_totalThrh = qdmaGetTxBufTotalThreshold(base);

    return 0;
}

static inline int qdma_set_prefetch_mode_inline(unsigned int base,QDMA_Mode_t mode,unsigned int overDragCnt)
{
    if(QDMA_DISABLE == mode) {
        qdmaDisableTxDmaPrefetch(base) ;
    } else {
        qdmaEnableTxDmaPrefetch(base) ;
    }

    if(QDMA_DISABLE == mode) {
        qdmaDisablePrefetchOverDrag(base) ;
    } else {
        qdmaEnablePrefetchOverDrag(base) ;
    }
    
    qdmaSetPrefetchOverDragCnt(base,overDragCnt) ;
    
    return 0;
}

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/
typedef struct {
	struct cdev *pQdmaDev ;
} QDMA_Dev_T ;

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
*************************************************************************
*/
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	int __inline__ pseSetWanPcpConfig(unchar type, unchar mode) ;
	int __inline__ pseGetWanPcpConfig(unchar type) ;
	int __inline__ qdmaSetChannelRetire(unchar channel) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
int qdmaSetTxQosScheduler(unchar channel, unchar mode, ushort weight[8]) ;
int qdmaGetTxQosScheduler(unchar channel, unchar *pMode, ushort weight[8]) ;

int qdmaSetRxRateLimitConfig(unchar ringIdx, unchar type, ushort value) ;
int qdmaGetRxRateLimitConfig(unchar ringIdx, unchar type) ;
int qdmaSetTxRateLimitConfig(unchar chnlIdx, unchar type, ushort value) ;
int qdmaGetTxRateLimitConfig(unchar chnlIdx, unchar type) ;
int qdmaSetVirtualChannelQos(unchar phyChnl, unchar virChnlMax, unchar mode, unchar weight[4]) ;
int qdmaGetVirtualChannelQos(unchar phyChnl, unchar virChnlMax, unchar *pMode, unchar weight[4]) ;
int qdmaGetLimitRateMax(int ticksel_idx) ;
int qdmaGetLimitRateMaxChnl(int ticksel_idx, int channel);
int qdmaUpdateAllTxRateLimitValue(ushort curUnit, ushort newUnit, int ticksel_idx) ;
int qdmaUpdateAllRxRateLimitValue(ushort curUnit, ushort newUnit, unchar ticker);

int qdma_clear_and_set_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr) ;
int qdma_get_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr) ;
int qdma_set_dbg_cntr_default_config(void) ;

int qdmaSetDbgMemXsConfig(QDMA_DbgMemXsMemSel_t memSel, unchar byteSel, ushort addr, uint valueLow, uint valueHigh) ;
int qdmaGetDbgMemXsConfig(QDMA_DbgMemXsMemSel_t memSel, unchar byteSel, ushort addr, uint *value) ;

#ifdef TCSUPPORT_QOS
int qdma_get_ringIdx(struct sk_buff *skb, struct port_info *pMacInfo) ;
int qdma_qos_mark(struct sk_buff *skb) ;
#else
int qdma_qos_remark(struct sk_buff *skb) ;
#endif

int macResourceLimit(struct sk_buff *skb, uint channel, uint queue) ;
int qdmaEnableInt(uint base, uint bit, QDMA_InterruptNo_t intIdx,QDMA_IntEnable_t enableIdx);
int qdmaDisableInt(uint base, uint bit, QDMA_InterruptNo_t intIdx,QDMA_IntEnable_t enableIdx);
int qdmaSetIntMask(uint base, QDMA_InterruptNo_t intIdx, QDMA_IntEnable_t enableIdx, uint value);
int qdmaGetIntMask(uint base,  QDMA_InterruptNo_t intIdx, QDMA_IntEnable_t enableIdx);
int qdmaSetIntBind(uint base, QDMA_InterruptNo_t intIdx);
int qdmaSetIrqBind(uint base, QDMA_IrqNo_t intIdx);

void qdma_dma_busy_timer(TIMER_FUN_PAAM arg) ;
void qdma_trigger_timer(TIMER_FUN_PAAM arg) ;
void qdma_reg_polling_timer(TIMER_FUN_PAAM arg) ;

#ifdef TCSUPPORT_DRIVER_API
void qdma_dev_destroy(void) ;
#endif
int qdma_dev_init(void) ;

#endif /* _QDMA_DEV_7516_H_ */
