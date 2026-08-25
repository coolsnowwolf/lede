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
#ifndef _QDMA_REG_H_
#define _QDMA_REG_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <asm/tc3162/tc3162.h>
#include <asm/io.h>


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/*******************************************************
 register access utility
********************************************************/
#ifdef CONFIG_SIMULATION
	#define IO_GREG(reg)							0
	#define IO_SREG(reg, value)					
	#define IO_SBITS(reg, bit)					
	#define IO_CBITS(reg, bit)					
	#define IO_GREG_REP(reg, buf, count)			0
	#define IO_SREG_REP(reg, buf, count)		
#else

#ifdef TCSUPPORT_CPU_NDS32
	#define IO_GREG(reg)							get_frame_engine_data(reg)
	#define IO_SREG(reg, value) 					set_frame_engine_data(reg, value)
	#define IO_GMASK(reg, mask, shift)				((get_frame_engine_data(reg) & mask) >> shift)
	#define IO_SMASK(reg, mask, shift, value)		{ uint t = get_frame_engine_data(reg); set_frame_engine_data(reg, ((t&~(mask))|((value<<shift)&mask))); }
	#define IO_SBITS(reg, bit)						{ uint t = get_frame_engine_data(reg); set_frame_engine_data(reg, (t|bit)); }
	#define IO_CBITS(reg, bit)						{ uint t = get_frame_engine_data(reg); set_frame_engine_data(reg, (t&~(bit))); }
#else
	#define IO_GREG(reg)							regRead32(reg)
	#define IO_SREG(reg, value)						regWrite32(reg, value)
	#define IO_GMASK(reg, mask, shift)				((regRead32(reg) & mask) >> shift)
	#define IO_SMASK(reg, mask, shift, value)		{ uint t = regRead32(reg); regWrite32(reg, ((t&~(mask))|((value<<shift)&mask))); }
	#define IO_SBITS(reg, bit)						{ uint t = regRead32(reg); regWrite32(reg, (t|bit)); }
	#define IO_CBITS(reg, bit)						{ uint t = regRead32(reg); regWrite32(reg, (t&~(bit))); }
#endif
	#define IO_GREG_REP(reg, buf, count)			ioread32_rep((void __iomem *)(reg), buf, count)
	#define IO_SREG_REP(reg, buf, count)			iowrite32_rep((void __iomem *)(reg), buf, count)

#endif /* CONFIG_SIMULATION */

/*******************************************************
    QDAM2 & QDMA1 different
********************************************************/
#if defined(QDMA_LAN)
    #define CONFIG_QDMA_BASE_ADDR                       (0x1FB54000)
    #define QDMA_REG_BASE                               (0xBFB54000)
    #define FE_GDMA_FWD_CFG                             (0xbfb50500)
    #define CONFIG_QDMA_IRQ_1                           QDMA_LAN0_INTR
    #define CONFIG_QDMA_IRQ_2                           QDMA_LAN1_INTR
    #define CONFIG_QDMA_IRQ_3                           QDMA_LAN2_INTR
    #define CONFIG_QDMA_IRQ_4                           QDMA_LAN3_INTR
    #define RING_OFFSET(idx)                            ((idx&0xE)?((idx-2)<<5):(idx<<8))
#else
    #define CONFIG_QDMA_BASE_ADDR                       (0x1FB55000)
    #define QDMA_REG_BASE                               (0xBFB55000)
    #define FE_GDMA_FWD_CFG                             (0xbfb51500)
    #define CONFIG_QDMA_IRQ_1                           QDMA_WAN0_INTR
    #define CONFIG_QDMA_IRQ_2                           QDMA_WAN1_INTR
    #define CONFIG_QDMA_IRQ_3                           QDMA_WAN2_INTR
    #define CONFIG_QDMA_IRQ_4                           QDMA_WAN3_INTR
    #define RING_OFFSET(idx)                            ((idx&0xE)?((idx-2)<<5):(idx<<8))
#endif


/*******************************************************
 SLM Register Map
********************************************************/
#define SLM_SECTOR_SIZE					0x100

#define SLM_BASE						0xBFA60000
#define SLM_GLO_CFG     				(SLM_BASE + 0x0000)
#define SLM_EN		     				(SLM_BASE + 0x0004)
#define SLM_VIR_BASE					(SLM_BASE + 0x0010)
#define SLM_VIR_SIZE					(SLM_BASE + 0x0014)
#define SLM_PHY_BASE					(SLM_BASE + 0x0018)
#define SLM_PHY_SIZE					(SLM_BASE + 0x001C)
#define SLM_FREE_MIN_CNT				(SLM_BASE + 0x0020)
#define SLM_FREE_CUR_CNT				(SLM_BASE + 0x0024)
#define SLM_FREE_THRH					(SLM_BASE + 0x0028)
#define SLM_API_CMD						(SLM_BASE + 0x0030)
#define SLM_API_BASE					(SLM_BASE + 0x0034)
#define SLM_API_RD_ADDR					(SLM_BASE + 0x0038)
#define SLM_INT_STATUS					(SLM_BASE + 0x0040)
#define SLM_INT_MASK					(SLM_BASE + 0x0044)
#define SLM_BUS_RD_NULL_ADDR			(SLM_BASE + 0x0048)
#define SLM_DROP_CMD					(SLM_BASE + 0x0050)
#define SLM_DROP_ADDR					(SLM_BASE + 0x0054)
#define SLM_DROP_CNT					(SLM_BASE + 0x0058)

#define SLM_GLO_CFG_VALUE     			IO_GREG(SLM_BASE)
#define SLM_EN_VALUE		     		IO_GREG(SLM_EN)
#define SLM_VIR_BASE_VALUE				IO_GREG(SLM_VIR_BASE)
#define SLM_VIR_SIZE_VALUE				IO_GREG(SLM_VIR_SIZE)
#define SLM_PHY_BASE_VALUE				IO_GREG(SLM_PHY_BASE)
#define SLM_PHY_SIZE_VALUE				IO_GREG(SLM_PHY_SIZE)
#define SLM_FREE_MIN_CNT_VALUE			IO_GREG(SLM_FREE_MIN_CNT)
#define SLM_FREE_CUR_CNT_VALUE			IO_GREG(SLM_FREE_CUR_CNT)
#define SLM_FREE_THRH_VALUE				IO_GREG(SLM_FREE_THRH)
#define SLM_API_CMD_VALUE				IO_GREG(SLM_API_CMD)
#define SLM_API_BASE_VALUE				IO_GREG(SLM_API_BASE)
#define SLM_API_RD_ADDR_VALUE			IO_GREG(SLM_API_RD_ADDR)
#define SLM_INT_STATUS_VALUE			IO_GREG(SLM_INT_STATUS)
#define SLM_INT_MASK_VALUE				IO_GREG(SLM_INT_MASK)
#define SLM_BUS_RD_NULL_ADDR_VALUE		IO_GREG(SLM_BUS_RD_NULL_ADDR)
#define SLM_DROP_CMD_VALUE				IO_GREG(SLM_DROP_CMD)
#define SLM_DROP_ADDR_VALUE				IO_GREG(SLM_DROP_ADDR)
#define SLM_DROP_CNT_VALUE				IO_GREG(SLM_DROP_CNT)

#ifdef TCSUPPORT_CPU_NDS32
#define K0_TO_K1(x)       				(x)  /* kseg0 to kseg1 */
#define K1_TO_K0(x)       				(x)  /* kseg1 to kseg0 */
#define K0_TO_PHYSICAL(x) 				(virt_to_phys(x))  /* kseg0 to physical */
#define PHYSICAL_TO_K0(x) 				(phys_to_virt(x))  /* physical to kseg0 */
#else
#define K0_TO_K1(x)       				((uint)(x) | 0x20000000)  /* kseg0 to kseg1 */
#define K1_TO_K0(x)       				((uint)(x) & 0x9fffffff)  /* kseg1 to kseg0 */
#define K0_TO_PHYSICAL(x) 				((uint32)(x) & 0x1fffffff)  /* kseg0 to physical */
#define K1_TO_PHYSICAL(x) 				((uint32)(x) & 0x1fffffff)  /* kseg1 to physical */
#define PHYSICAL_TO_K0(x) 				((uint32)(x) | 0x80000000)  /* physical to kseg0 */
#define PHYSICAL_TO_K1(x) 				((uint32)(x) | 0xa0000000)  /* physical to kseg1 */
#endif

/*******************************************************
 CSR for QDMA
********************************************************/
#define QDMA_CSR_OFFSET_ADDR(base, offset)			(base+offset)
#define QDMA_CSR_INFO(base)							(base+0x0000)
#define QDMA_CSR_GLB_CFG(base)						(base+0x0004)
#define QDMA_CSR_TX_DSCP_BASE(base, idx)			(base+0x0008+RING_OFFSET(idx))
#define QDMA_CSR_RX_DSCP_BASE(base, idx)			((idx&0xE)?(base+0x0800+RING_OFFSET(idx)):((base+0x000C+RING_OFFSET(idx))))
#define QDMA_CSR_TX_CPU_IDX(base, idx)				(base+0x0010+RING_OFFSET(idx))
#define QDMA_CSR_TX_DMA_IDX(base, idx)				(base+0x0014+RING_OFFSET(idx))
#define QDMA_CSR_RX_CPU_IDX(base, idx)				((idx&0xE)?(base+0x0808+RING_OFFSET(idx)):(base+0x0018+RING_OFFSET(idx)))
#define QDMA_CSR_RX_DMA_IDX(base, idx)				((idx&0xE)?(base+0x080c+RING_OFFSET(idx)):(base+0x001c+RING_OFFSET(idx)))


#define QDMA_CSR_HWFWD_DSCP_BASE(base)				(base+0x0020)
#define QDMA_CSR_HWFWD_BUFF_BASE(base)				(base+0x0024)
#define QDMA_CSR_HWFWD_DSCP_CFG(base)				(base+0x0028)
#define QDMA_CSR_MIN_DSCPTHR_CFG(base)				(base+0x002c)
#define QDMA_CSR_LMGR_INIT_CFG(base)				(base+0x0030)
#define QDMA_CSR_LMGR_CHNL_RETIRE(base)				(base+0x0040)

#define QDMA_CSR_INT_STATUS(base)					(base+0x0050)
#define QDMA_CSR_INT_ENABLE(base)					(base+0x0054)
#define QDMA_CSR_TX_DELAY_INT_CFG(base)				(base+0x0058)
#define QDMA_CSR_RX_DELAY_INT_CFG(base, idx)		((idx&0xE)?(base+0x0810+RING_OFFSET(idx)):(base+0x005c))
#define QDMA_CSR_IRQ_BASE(base)						(base+0x0060)
#define QDMA_CSR_IRQ_CFG(base)						(base+0x0064)
#define QDMA_CSR_IRQ_CLEAR_LEN(base)				(base+0x0068)
#define QDMA_CSR_IRQ_STATUS(base)					(base+0x006C)
#define QDMA_CSR_IRQ_PTIME(base)					(base+0x0070)

#define QDMA_CSR_TXWRR_MODE_CFG(base)				(base+0x0080)
#define QDMA_CSR_TXWRR_WEIGHT_CFG(base)				(base+0x0088)
#define QDMA_CSR_DMA_PREFETCH_CFG(base)				(base+0x008C)  /*not used in 7516 */
#define QDMA_CSR_PSE_BUF_USAGE_CFG(base)			(base+0x0090)
#define QDMA_CSR_EGRESS_RATEMETER_CFG(base)			(base+0x0094)
#define QDMA_CSR_EGRESS_RATELIMIT_CFG(base)			(base+0x0098)
#define QDMA_CSR_RATELIMIT_PARAMETER_CFG(base)		(base+0x009C)

#define QDMA_CSR_TXQ_CNGST_CFG(base)				(base+0x00A0)
#define QDMA_CSR_TXQ_DYN_TOTALTHR(base)				(base+0x00A4)
#define QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base)			(base+0x00A8)
#define QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base)			(base+0x00AC)
#define QDMA_CSR_STATIC_QUEUE_THR(base, i)			(base+0x00B0+(i<<2))

#define QDMA_CSR_DBG_MEM_XS_CFG(base)				(base+0x00E0)
#define QDMA_CSR_DBG_MEM_XS_DATA_LO(base)			(base+0x00E4)
#define QDMA_CSR_DBG_MEM_XS_DATA_HI(base)			(base+0x00E8)

#define QDMA_CSR_DBG_LMGR_STATUS(base)				(base+0x00F0)
#define QDMA_CSR_DBG_LMGR_USAGE(base)				(base+0x00F4)
#define QDMA_CSR_DBG_QDMA_PROBE_LO(base)			(base+0x00F8)
#define QDMA_CSR_DBG_QDMA_PROBE_HI(base)			(base+0x00FC)

#define QDMA_CSR_RX_RING_CFG(base)					(base+0x0100)
#define QDMA_CSR_RX_RING_THR(base)					(base+0x0104)

#define QDMA_CSR_RX_PROTECT_CFG(base)				(base+0x0120)
#define QDMA_CSR_RX_RATELIMIT_CFG(base)				(base+0x0124)
#define QDMA_CSR_RX_TICK_CFG(base)					(base+0x0128)
#define QDMA_CSR_RX_PKTOVH_CFG(base)				(base+0x012c)
#define QDMA_CSR_VCHNL_WEIGHT_CFG(base)				(base+0x013C)
#define QDMA_CSR_VCHNL_QOS_MODE(base)				(base+0x0140)
#define QDMA_CSR_RATELIMIT_EN_CFG(base)				(base+0x0160)

#define QDMA_CSR_PERCHNL_QOS_MODE(base, i)			(base+0x0180+(i<<2))
#define QDMA_CSR_PERCHNL_DATARATE(base, i)			(base+0x0200+(i<<2))
#define QDMA_CSR_PERCHNL_QUEUE_NORM_DROP(base, i)	(base+0x0280+(i<<2))
#define QDMA_CSR_PERCHNL_QUEUE_DEI_DROP(base, i)	(base+0x02C0+(i<<2))
#define QDMA_CSR_DBG_CNTR_CFG(base, i)				(base+0x0300+(i<<3))
#define QDMA_CSR_DBG_CNTR_VAR(base, i)				(base+0x0304+(i<<3))

#define QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL(base)	(base+0x0600)
#define QDMA_CSR_EGRESS_RATELIMIT_CFG2(base)		(base+0x0620)
#define QDMA_CSR_DBA_CHEAT_CFG(base)				(base+0x062c)
#define QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG(base)			(base+0x63c)
#define QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG(base)		(base+0x640)
#define QDMA_CSR_DBG_CHNLQLVD(base,chnlIdx)			(base+0x0680+(chnlIdx&1c))

#define QDMA_CSR_INT_STATUS1(base)					(base+0x0700) /*63~32*/
#define QDMA_CSR_INT_STATUS2(base)					(base+0x0704) /*95~64*/
#define QDMA_CSR_INT_EN(base,intNo,enNo)			((((intNo)==1)&&((enNo)==0))?(base+0x054):(base+0x704+(((intNo)-1)*12)+((enNo)<<2)))	/*intNo:1~4, enNo:0~2*/
#define QDMA_CSR_RX_RING_SIZE_2_15(base, idx)		(base+0x0804+RING_OFFSET(idx))
/*******************************************************************************
*******************************************************************************/
/* QDMA_CSR_INFO(base) */

/* QDMA_CSR_GLB_CFG(base) */
#define GLB_CFG_RX_2B_OFFSET						(1<<31)
#define GLB_CFG_DMA_PREFERENCE_SHIFT				(29)
#define GLB_CFG_DMA_PREFERENCE_MASK					(0x3<<GLB_CFG_DMA_PREFERENCE_SHIFT)
#define PREFER_ROIND_ROBIN							(0x00)
#define PREFER_FWD_TX1_TX0							(0x01)
#define PREFER_TX1_FWD_TX0							(0x10)
#define PREFER_TX1_TX0_FWD							(0x11)
#define GLB_CFG_MSG_WORD_SWAP						(1<<28)
#define GLB_CFG_DSCP_BYTE_SWAP						(1<<27)
#define GLB_CFG_PAYLOAD_BYTE_SWAP					(1<<26)
#define GLB_CFG_VCHNL_MAP_EN						(1<<25)
#define GLB_CFG_VCHNL_MAP_MODE						(1<<24)
#define GLB_CFG_QDMA_QUICK_RESET					(1<<23)
#define GLB_CFG_LPBK_RXQ_SEL						(1<<22)/*not used in 7516*/
#define GLB_CFG_SLM_RELEASE_EN						(1<<21)
#define GLB_CFG_TX_IMMEDIATE_DONE					(1<<20)
#define GLB_CFG_IRQ_EN								(1<<19)
#define GLB_CFG_LOOPCNT_EN							(1<<18)
#define GLB_CFG_UMAC_LOOPBACK						(1<<17)
#define GLB_CFG_QDMA_LOOPBACK						(1<<16)
#define GLB_CFG_LPBK_RXQ_SEL_SHIFT					(8)
#define GLB_CFG_LPBK_RXQ_SEL_MASK					(0xF<<GLB_CFG_LPBK_RXQ_SEL_SHIFT)
#define GLB_CFG_CHECK_DONE							(1<<7)
#define GLB_CFG_TX_WB_DONE							(1<<6)
#define GLB_CFG_BST_SE_SHIFT						(4)
#define GLB_CFG_BST_SE_MASK							(0x3<<GLB_CFG_BST_SE_SHIFT)
#define VAL_BST_4_DWORD								(0x0)
#define VAL_BST_8_DWORD								(0x1)
#define VAL_BST_16_DWARD							(0x2)
#define VAL_BST_32_DWARD							(0x3)
#define GLB_CFG_RX_DMA_BUSY							(1<<3)
#define GLB_CFG_RX_DMA_EN							(1<<2)
#define GLB_CFG_TX_DMA_BUSY							(1<<1)
#define GLB_CFG_TX_DMA_EN							(1<<0)

/* QDMA_CSR_TX_DSCP_BASE(base) */

/* QDMA_CSR_RX_DSCP_BASE(base) */

/* QDMA_CSR_TX_CPU_IDX(base) */
#if  defined(TCSUPPORT_CPU_EN7517)
//for 7517 setting
#define TX_CPU_IDX_SHIFT							(0)
#define TX_CPU_IDX_MASK								(0xFFFF<<TX_CPU_IDX_SHIFT)

/* QDMA_CSR_TX_DMA_IDX(base) */
#define TX_DMA_IDX_SHIFT							(0)
#define TX_DMA_IDX_MASK								(0xFFFF<<TX_DMA_IDX_SHIFT)

/* QDMA_CSR_RX_CPU_IDX(base) */
#define RX_CPU_IDX_SHIFT							(0)
#define RX_CPU_IDX_MASK								(0xFFFF<<RX_CPU_IDX_SHIFT)

/* QDMA_CSR_RX_DMA_IDX(base) */
#define RX_DMA_IDX_SHIFT							(0)
#define RX_DMA_IDX_MASK								(0xFFFF<<RX_DMA_IDX_SHIFT)
#else
#define TX_CPU_IDX_SHIFT							(0)
#define TX_CPU_IDX_MASK								(0xFFF<<TX_CPU_IDX_SHIFT)

/* QDMA_CSR_TX_DMA_IDX(base) */
#define TX_DMA_IDX_SHIFT							(0)
#define TX_DMA_IDX_MASK								(0xFFF<<TX_DMA_IDX_SHIFT)

/* QDMA_CSR_RX_CPU_IDX(base) */
#define RX_CPU_IDX_SHIFT							(0)
#define RX_CPU_IDX_MASK								(0xFFF<<RX_CPU_IDX_SHIFT)

/* QDMA_CSR_RX_DMA_IDX(base) */
#define RX_DMA_IDX_SHIFT							(0)
#define RX_DMA_IDX_MASK								(0xFFF<<RX_DMA_IDX_SHIFT)
#endif

/*QDMA_CSR_TXQ_DIS_CFG_CHN*/
/*offset 0x00:CHN0~3, 0x04:CHN4~7, 0x08:CHN8~11, 0x0c:CHN12~15*/
/*offset 0x10:CHN16~19, 0x14:CHN20~23, 0x18:CHN24~27, 0x1c:CHN28~31*/
#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)		((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_QUEUE_OFFSET(chnl,queue)	(1<<((queue)+(((chnl)&0x03)*8)))
#define QDMA_CSR_QUEUE_CLOSE_CFG(base, idx)		(base+0x01a0+TXQ_DIS_QUEUE_CLOSE_OFFSET(idx))
#define TXQ_DIS_CHANNEL_SHIFT(chnl)                         (((chnl)&0x03)*8)
#define TXQ_DIS_CHANNEL_MASK(chnl)                          (0xFF<<TXQ_DIS_CHANNEL_SHIFT(chnl))

/* QDMA_CSR_HWFWD_DSCP_BASE(base) */

/* QDMA_CSR_HWFWD_BUFF_BASE(base) */

/* QDMA_CSR_HWFWD_DSCP_CFG(base) */
#define HWFWD_PAYLOAD_SIZE_SHIFT					(28)
#define HWFWD_PAYLOAD_SIZE_MASK						(0x3<<HWFWD_PAYLOAD_SIZE_SHIFT)
#define HWFWD_PAYLOAD_SIZE_2K						(0x0)
#define HWFWD_PAYLOAD_SIZE_4K						(0x1)
#define HWFWD_PAYLOAD_SIZE_8K						(0x2)
#define HWFWD_PAYLOAD_SIZE_16K						(0x3)
#define HWFWD_DSCP_LOW_THRSHLD_SHIFT				(0)
#define HWFWD_DSCP_LOW_THRSHLD_MASK					(0x1FFF<<HWFWD_DSCP_LOW_THRSHLD_SHIFT)

/* QDMA_CSR_LMGR_INIT_CFG(base) */
#define LMGR_INIT_START								(1<<31)
#define HWFWD_PKTSIZE_OVERHEAD_EN					(1<<24)
#define HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT			(16)
#define HWFWD_PKTSIZE_OVERHEAD_VALUE_MASK			(0xFF<<HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT)
#define HWFWD_DSCP_NUM_SHIFT						(0)
#define HWFWD_DSCP_NUM_MASK							(0x1FFF<<HWFWD_DSCP_NUM_SHIFT)

/* QDMA_CSR_LMGR_CHNL_RETIRE(base) */
#define LMGR_CHNL_RETIRE_CMD						(1<<31)
#define LMGR_CHNL_RETIRE_DONE						(1<<30)
#define LMGR_CHNL_RETIRE_CHN_SHIFT					(0)
#define LMGR_CHNL_RETIRE_CHN_MASK					(0x1F<<LMGR_CHNL_RETIRE_CHN_SHIFT)

/*default setting for INT1~4 bind TX&RX Ring info*/
#if defined(QDMA_LAN)
/*INT1*/
#define INT1_RX_RING_MASK					        (0x8187)        /*Rx_Ring_0/1/2/7/8/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2*/
#define INT2_RX_RING_MASK					        (0x418)          /*Rx_Ring_3/4/10 for reserve*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x20)          /*Rx_Ring_5 for WIFI-2.4G,Ring_9 for left to right*/
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x40)          /*Rx_Ring_6 for WIFI-5G*/
#define INT4_TX_RING_MASK					        (0x0)
#else
#if  defined(TCSUPPORT_CPU_EN7517)
/*INT1 for wan*/
#define INT1_RX_RING_MASK					        (0xff00)        /*Rx_Ring_8-15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2 for lan*/
#define INT2_RX_RING_MASK					        (0x00ff)         /*Rx_Ring_0-7 binded to INT2*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x0)
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x0)
#define INT4_TX_RING_MASK					        (0x0)

#else
/*INT1*/
#define INT1_RX_RING_MASK					        (0x8187)        /*Rx_Ring_0/1/2/7/8/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0x3)           /*Tx_Ring_0~1 binded to INT1*/
/*INT2*/
#define INT2_RX_RING_MASK					        (0x478)          /*Rx_Ring_3/4/5/6 for reserve*/
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x0)
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x0)
#define INT4_TX_RING_MASK					        (0x0)
#endif
#endif

/* QDMA_CSR_INT1_STATUS0(base) */
#define INT_STATUS_RX1_DSCP_LOW						(1<<31)
#define INT_STATUS_RX0_DSCP_LOW						(1<<30)
#define INT_STATUS_XPON_PHY							(1<<24)
#define INT_STATUS_EPON_MAC							(1<<17)
#define INT_STATUS_GPON_MAC							(1<<16)
#define INT_STATUS_RX1_COHERENT						(1<<15)
#define INT_STATUS_TX1_COHERENT						(1<<14)
#define INT_STATUS_RX0_COHERENT						(1<<13)
#define INT_STATUS_TX0_COHERENT						(1<<12)
#define INT_STATUS_RX_PKT_OVERFLOW                  (1<<11)
#define INT_STATUS_HWFWD_DSCP_LOW					(1<<10)
#define INT_STATUS_IRQ_FULL							(1<<9)
#define INT_STATUS_HWFWD_DSCP_EMPTY					(1<<8)
#define INT_STATUS_NO_RX1_CPU_DSCP					(1<<7)
#define INT_STATUS_NO_TX1_CPU_DSCP					(1<<6)
#define INT_STATUS_RX1_DONE							(1<<5)
#define INT_STATUS_TX1_DONE							(1<<4)
#define INT_STATUS_NO_RX0_CPU_DSCP					(1<<3)
#define INT_STATUS_NO_TX0_CPU_DSCP					(1<<2)
#define INT_STATUS_RX0_DONE							(1<<1)
#define INT_STATUS_TX0_DONE							(1<<0)
#define INT_STATUS_TX_DONE							(INT_STATUS_TX0_DONE | INT_STATUS_TX1_DONE)
#define INT_STATUS_QDMA_FAULT						(0x00005744)
#define INT_STATUS_EXTERNAL							(0x01030000)
#define INT_STATUS_RX_DSCP_LOW_0_1					(INT_STATUS_RX1_DSCP_LOW | INT_STATUS_RX0_DSCP_LOW)


/* QDMA_CSR_INT_STATUS1(base) */
#define INT_STATUS_NO_RX_CPU_DSCP(idx)				(1<<(14+idx)) /*RX2 ~RX15, idx :2~15*/
#define INT_STATUS_RX_DONE(idx)						(1<<(idx-2))  /*RX2 ~RX15, idx :2~15*/
#define INT_STATUS_RX_DONE_2_15						(0x3FFF)
#define INT_STATUS_NO_RX_CPU_DSCP_2_15				(0x3FFF<<16)

/* QDMA_CSR_INT_STATUS2(base) */
#define INT_STATUS_RX_DSCP_LOW(idx)					(1<<(14+idx)) /*RX2 ~RX15, idx :2~15*/
#define INT_STATUS_RX_COHERENT(idx)					(1<<(idx-2))  /*RX2 ~RX15, idx :2~15*/
#define INT_STATUS_RX_DSCP_LOW_2_15					(0x3FFF<<16)
#define INT_STATUS_RX_COHERENT_2_15					(0x3FFF)

/*INT1 Status Setting */
#define INT1_STATUS_NO_RX0_CPU_DSCP					((int1_rx_ring&0x1)<<3)
#define INT1_STATUS_NO_RX1_CPU_DSCP					((int1_rx_ring&0x2)<<6)
#define INT1_STATUS_NO_RX_CPU_DSCP_0_1				(INT1_MASK_NO_RX0_CPU_DSCP|INT1_MASK_NO_RX1_CPU_DSCP)	
#define INT1_STATUS_NO_RX_CPU_DSCP(rxIdx)			((int1_rx_ring&(1<<rxIdx))<<14)
#define INT1_STATUS_NO_RX_CPU_DSCP_2_15				((int1_rx_ring&0xFFFC)<<14) 	/*(>>2)<<16*/

#define INT1_STATUS_RX0_DONE						((int1_rx_ring&0x1)<<1)
#define INT1_STATUS_RX1_DONE						((int1_rx_ring&0x2)<<4)
#define INT1_STATUS_RX_DONE_0_1						(INT1_MASK_RX0_DONE|INT1_MASK_RX1_DONE)
#define INT1_STATUS_RX_DONE(rxIdx)					((int1_rx_ring&(1<<rxIdx))>>2)  /*Rx2~Rx15*/
#define INT1_STATUS_RX_DONE_2_15					((int1_rx_ring&0xFFFC)>>2)

#define INT1_STATUS_RX0_DSCP_LOW					((int1_rx_ring&0x1)<<30)
#define INT1_STATUS_RX1_DSCP_LOW					((int1_rx_ring&0x2)<<30)
#define INT1_STATUS_RX_DSCP_LOW_0_1					(INT1_MASK_RX0_DSCP_LOW|INT1_MASK_RX1_DSCP_LOW)
#define INT1_STATUS_RX_DSCP_LOW(rxIdx)				((int1_rx_ring&(1<<rxIdx))<<14) /*Rx2~Rx15*/
#define INT1_STATUS_RX_DSCP_LOW_2_15				((int1_rx_ring&0xFFFC)<<14)

#define INT1_STATUS_RX0_COHERENT					((int1_rx_ring&0x1)<<13)
#define INT1_STATUS_RX1_COHERENT					((int1_rx_ring&0x2)<<14)
#define INT1_STATUS_RX_COHERENT_0_1					(INT1_MASK_RX0_COHERENT|INT1_MASK_RX1_COHERENT)
#define INT1_STATUS_RX_COHERENT(rxIdx)				((int1_rx_ring&(1<<rxIdx))>>2) /*Rx2~Rx15*/
#define INT1_STATUS_RX_COHERENT_2_15				((int1_rx_ring&0xFFFC)>>2)

/*INT2 Status Setting */
#define INT2_STATUS_NO_RX0_CPU_DSCP					((int2_rx_ring&0x1)<<3)
#define INT2_STATUS_NO_RX1_CPU_DSCP					((int2_rx_ring&0x2)<<6)
#define INT2_STATUS_NO_RX_CPU_DSCP_0_1				(INT2_MASK_NO_RX0_CPU_DSCP|INT2_MASK_NO_RX1_CPU_DSCP)	
#define INT2_STATUS_NO_RX_CPU_DSCP(rxIdx)			((int2_rx_ring&(1<<rxIdx))<<14)
#define INT2_STATUS_NO_RX_CPU_DSCP_2_15				((int2_rx_ring&0xFFFC)<<14) 	/*(>>2)<<16*/

#define INT2_STATUS_RX0_DONE						((int2_rx_ring&0x1)<<1)
#define INT2_STATUS_RX1_DONE						((int2_rx_ring&0x2)<<4)
#define INT2_STATUS_RX_DONE_0_1						(INT2_MASK_RX0_DONE|INT2_MASK_RX1_DONE)
#define INT2_STATUS_RX_DONE(rxIdx)					((int2_rx_ring&(1<<rxIdx))>>2)  /*Rx2~Rx15*/
#define INT2_STATUS_RX_DONE_2_15					((int2_rx_ring&0xFFFC)>>2)

#define INT2_STATUS_RX0_DSCP_LOW					((int2_rx_ring&0x1)<<30)
#define INT2_STATUS_RX1_DSCP_LOW					((int2_rx_ring&0x2)<<30)
#define INT2_STATUS_RX_DSCP_LOW_0_1					(INT2_MASK_RX0_DSCP_LOW|INT2_MASK_RX1_DSCP_LOW)
#define INT2_STATUS_RX_DSCP_LOW(rxIdx)				((int2_rx_ring&(1<<rxIdx))<<14) /*Rx2~Rx15*/
#define INT2_STATUS_RX_DSCP_LOW_2_15				((int2_rx_ring&0xFFFC)<<14)

#define INT2_STATUS_RX0_COHERENT					((int2_rx_ring&0x1)<<13)
#define INT2_STATUS_RX1_COHERENT					((int2_rx_ring&0x2)<<14)
#define INT2_STATUS_RX_COHERENT_0_1					(INT2_MASK_RX0_COHERENT|INT2_MASK_RX1_COHERENT)
#define INT2_STATUS_RX_COHERENT(rxIdx)				((int2_rx_ring&(1<<rxIdx))>>2) /*Rx2~Rx15*/
#define INT2_STATUS_RX_COHERENT_2_15				((int2_rx_ring&0xFFFC)>>2)

/*INT3 Status Setting */
#define INT3_STATUS_NO_RX0_CPU_DSCP					((int3_rx_ring&0x1)<<3)
#define INT3_STATUS_NO_RX1_CPU_DSCP					((int3_rx_ring&0x2)<<6)
#define INT3_STATUS_NO_RX_CPU_DSCP_0_1				(INT3_MASK_NO_RX0_CPU_DSCP|INT3_MASK_NO_RX1_CPU_DSCP)	
#define INT3_STATUS_NO_RX_CPU_DSCP(rxIdx)			((int3_rx_ring&(1<<rxIdx))<<14)
#define INT3_STATUS_NO_RX_CPU_DSCP_2_15				((int3_rx_ring&0xFFFC)<<14) 	/*(>>2)<<16*/

#define INT3_STATUS_RX0_DONE						((int3_rx_ring&0x1)<<1)
#define INT3_STATUS_RX1_DONE						((int3_rx_ring&0x2)<<4)
#define INT3_STATUS_RX_DONE_0_1						(INT3_MASK_RX0_DONE|INT3_MASK_RX1_DONE)
#define INT3_STATUS_RX_DONE(rxIdx)					((int3_rx_ring&(1<<rxIdx))>>2)  /*Rx2~Rx15*/
#define INT3_STATUS_RX_DONE_2_15					((int3_rx_ring&0xFFFC)>>2)

#define INT3_STATUS_RX0_DSCP_LOW					((int3_rx_ring&0x1)<<30)
#define INT3_STATUS_RX1_DSCP_LOW					((int3_rx_ring&0x2)<<30)
#define INT3_STATUS_RX_DSCP_LOW_0_1					(INT3_MASK_RX0_DSCP_LOW|INT3_MASK_RX1_DSCP_LOW)
#define INT3_STATUS_RX_DSCP_LOW(rxIdx)				((int3_rx_ring&(1<<rxIdx))<<14) /*Rx2~Rx15*/
#define INT3_STATUS_RX_DSCP_LOW_2_15				((int3_rx_ring&0xFFFC)<<14)

#define INT3_STATUS_RX0_COHERENT					((int3_rx_ring&0x1)<<13)
#define INT3_STATUS_RX1_COHERENT					((int3_rx_ring&0x2)<<14)
#define INT3_STATUS_RX_COHERENT_0_1					(INT3_MASK_RX0_COHERENT|INT3_MASK_RX1_COHERENT)
#define INT3_STATUS_RX_COHERENT(rxIdx)				((int3_rx_ring&(1<<rxIdx))>>2) /*Rx2~Rx15*/
#define INT3_STATUS_RX_COHERENT_2_15				((int3_rx_ring&0xFFFC)>>2)

/*INT4 Status Setting */
#define INT4_STATUS_NO_RX0_CPU_DSCP					((int4_rx_ring&0x1)<<3)
#define INT4_STATUS_NO_RX1_CPU_DSCP					((int4_rx_ring&0x2)<<6)
#define INT4_STATUS_NO_RX_CPU_DSCP_0_1				(INT4_MASK_NO_RX0_CPU_DSCP|INT4_MASK_NO_RX1_CPU_DSCP)	
#define INT4_STATUS_NO_RX_CPU_DSCP(rxIdx)			((int4_rx_ring&(1<<rxIdx))<<14)
#define INT4_STATUS_NO_RX_CPU_DSCP_2_15				((int4_rx_ring&0xFFFC)<<14) 	/*(>>2)<<16*/

#define INT4_STATUS_RX0_DONE						((int4_rx_ring&0x1)<<1)
#define INT4_STATUS_RX1_DONE						((int4_rx_ring&0x2)<<4)
#define INT4_STATUS_RX_DONE_0_1						(INT4_MASK_RX0_DONE|INT4_MASK_RX1_DONE)
#define INT4_STATUS_RX_DONE(rxIdx)					((int4_rx_ring&(1<<rxIdx))>>2)  /*Rx2~Rx15*/
#define INT4_STATUS_RX_DONE_2_15					((int4_rx_ring&0xFFFC)>>2)

#define INT4_STATUS_RX0_DSCP_LOW					((int4_rx_ring&0x1)<<30)
#define INT4_STATUS_RX1_DSCP_LOW					((int4_rx_ring&0x2)<<30)
#define INT4_STATUS_RX_DSCP_LOW_0_1					(INT4_MASK_RX0_DSCP_LOW|INT4_MASK_RX1_DSCP_LOW)
#define INT4_STATUS_RX_DSCP_LOW(rxIdx)				((int4_rx_ring&(1<<rxIdx))<<14) /*Rx2~Rx15*/
#define INT4_STATUS_RX_DSCP_LOW_2_15				((int4_rx_ring&0xFFFC)<<14)

#define INT4_STATUS_RX0_COHERENT					((int4_rx_ring&0x1)<<13)
#define INT4_STATUS_RX1_COHERENT					((int4_rx_ring&0x2)<<14)
#define INT4_STATUS_RX_COHERENT_0_1					(INT4_MASK_RX0_COHERENT|INT4_MASK_RX1_COHERENT)
#define INT4_STATUS_RX_COHERENT(rxIdx)				((int4_rx_ring&(1<<rxIdx))>>2) /*Rx2~Rx15*/
#define INT4_STATUS_RX_COHERENT_2_15				((int4_rx_ring&0xFFFC)>>2)


/* QDMA_CSR_INT_ENABLE(base) */
#define INT_MASK_RX1_DSCP_LOW						(1<<31)
#define INT_MASK_RX0_DSCP_LOW						(1<<30)
#define INT_MASK_XPON_PHY							(1<<24)
#define INT_MASK_EPON_MAC							(1<<17)
#define INT_MASK_GPON_MAC							(1<<16)
#define INT_MASK_RX1_COHERENT						(1<<15)
#define INT_MASK_TX1_COHERENT						(1<<14)
#define INT_MASK_RX0_COHERENT						(1<<13)
#define INT_MASK_TX0_COHERENT						(1<<12)
#define INT_MASK_RX_PKT_OVERFLOW					(1<<11)
#define INT_MASK_HWFWD_DSCP_LOW						(1<<10)
#define INT_MASK_IRQ_FULL							(1<<9)
#define INT_MASK_HWFWD_DSCP_EMPTY					(1<<8)
#define INT_MASK_NO_RX1_CPU_DSCP					(1<<7)
#define INT_MASK_NO_TX1_CPU_DSCP					(1<<6)
#define INT_MASK_RX1_DONE							(1<<5)
#define INT_MASK_TX1_DONE							(1<<4)
#define INT_MASK_NO_RX0_CPU_DSCP					(1<<3)
#define INT_MASK_NO_TX0_CPU_DSCP					(1<<2)
#define INT_MASK_RX0_DONE							(1<<1)
#define INT_MASK_TX0_DONE							(1<<0)
#define INT_MASK_NO_TX_CPU_DSCP						(INT_MASK_NO_TX1_CPU_DSCP|INT_MASK_NO_TX0_CPU_DSCP)
#define INT_MASK_TX_COHERENT						(INT_MASK_TX0_COHERENT|INT_MASK_TX1_COHERENT)
#define INT_MASK_RX_DSCP_LOW_0_1					(INT_MASK_RX0_DSCP_LOW|INT_MASK_RX1_DSCP_LOW)

/*INT-1*/
#define INT1_MASK_NO_RX0_CPU_DSCP					((int1_rx_ring&0x1)<<3)
#define INT1_MASK_NO_RX1_CPU_DSCP					((int1_rx_ring&0x2)<<6)
#define INT1_MASK_NO_RX_CPU_DSCP_0_1				(INT1_MASK_NO_RX0_CPU_DSCP|INT1_MASK_NO_RX1_CPU_DSCP)	
#define INT1_MASK_NO_RX_CPU_DSCP_2_15				((int1_rx_ring&0xFFFC)<<14) /*(>>2)<<16*/

#define INT1_MASK_RX0_DONE							((int1_rx_ring&0x1)<<1)
#define INT1_MASK_RX1_DONE							((int1_rx_ring&0x2)<<4)
#define INT1_MASK_RX_DONE_0_1						(INT1_MASK_RX0_DONE|INT1_MASK_RX1_DONE)
#define INT1_MASK_RX_DONE_2_15						((int1_rx_ring&0xFFFC)>>2)

#define INT1_MASK_RX0_DSCP_LOW						((int1_rx_ring&0x1)<<30)
#define INT1_MASK_RX1_DSCP_LOW						((int1_rx_ring&0x2)<<30)
#define INT1_MASK_RX_DSCP_LOW_0_1					(INT1_MASK_RX0_DSCP_LOW|INT1_MASK_RX1_DSCP_LOW)
#define INT1_MASK_RX_DSCP_LOW_2_15					((int1_rx_ring&0xFFFC)<<14)

#define INT1_MASK_RX0_COHERENT						((int1_rx_ring&0x1)<<13)
#define INT1_MASK_RX1_COHERENT						((int1_rx_ring&0x2)<<14)
#define INT1_MASK_RX_COHERENT_0_1					(INT1_MASK_RX0_COHERENT|INT1_MASK_RX1_COHERENT)
#define INT1_MASK_RX_COHERENT_2_15					((int1_rx_ring&0xFFFC)>>2)

/*INT-2*/
#define INT2_MASK_NO_RX0_CPU_DSCP					((int2_rx_ring&0x1)<<3)
#define INT2_MASK_NO_RX1_CPU_DSCP					((int2_rx_ring&0x2)<<6)
#define INT2_MASK_NO_RX_CPU_DSCP_0_1				(INT2_MASK_NO_RX0_CPU_DSCP|INT2_MASK_NO_RX1_CPU_DSCP)	
#define INT2_MASK_NO_RX_CPU_DSCP_2_15				((int2_rx_ring&0xFFFC)<<14) /*(>>2)<<16*/

#define INT2_MASK_RX0_DONE							((int2_rx_ring&0x1)<<1)
#define INT2_MASK_RX1_DONE							((int2_rx_ring&0x2)<<4)
#define INT2_MASK_RX_DONE_0_1						(INT2_MASK_RX0_DONE|INT2_MASK_RX1_DONE)
#define INT2_MASK_RX_DONE_2_15						((int2_rx_ring&0xFFFC)>>2)

#define INT2_MASK_RX0_DSCP_LOW						((int2_rx_ring&0x1)<<30)
#define INT2_MASK_RX1_DSCP_LOW						((int2_rx_ring&0x2)<<30)
#define INT2_MASK_RX_DSCP_LOW_0_1					(INT2_MASK_RX0_DSCP_LOW|INT2_MASK_RX1_DSCP_LOW)
#define INT2_MASK_RX_DSCP_LOW_2_15					((int2_rx_ring&0xFFFC)<<14)

#define INT2_MASK_RX0_COHERENT						((int2_rx_ring&0x1)<<13)
#define INT2_MASK_RX1_COHERENT						((int2_rx_ring&0x2)<<14)
#define INT2_MASK_RX_COHERENT_0_1					(INT2_MASK_RX0_COHERENT|INT2_MASK_RX1_COHERENT)
#define INT2_MASK_RX_COHERENT_2_15					((int2_rx_ring&0xFFFC)>>2)

/*INT-3*/
#define INT3_MASK_NO_RX0_CPU_DSCP					((int3_rx_ring&0x1)<<3)
#define INT3_MASK_NO_RX1_CPU_DSCP					((int3_rx_ring&0x2)<<6)
#define INT3_MASK_NO_RX_CPU_DSCP_0_1				(INT3_MASK_NO_RX0_CPU_DSCP|INT3_MASK_NO_RX1_CPU_DSCP)	
#define INT3_MASK_NO_RX_CPU_DSCP_2_15				((int3_rx_ring&0xFFFC)<<14) /*(>>2)<<16*/

#define INT3_MASK_RX0_DONE							((int3_rx_ring&0x1)<<1)
#define INT3_MASK_RX1_DONE							((int3_rx_ring&0x2)<<4)
#define INT3_MASK_RX_DONE_0_1						(INT3_MASK_RX0_DONE|INT3_MASK_RX1_DONE)
#define INT3_MASK_RX_DONE_2_15						((int3_rx_ring&0xFFFC)>>2)

#define INT3_MASK_RX0_DSCP_LOW						((int3_rx_ring&0x1)<<30)
#define INT3_MASK_RX1_DSCP_LOW						((int3_rx_ring&0x2)<<30)
#define INT3_MASK_RX_DSCP_LOW_0_1					(INT3_MASK_RX0_DSCP_LOW|INT3_MASK_RX1_DSCP_LOW)
#define INT3_MASK_RX_DSCP_LOW_2_15					((int3_rx_ring&0xFFFC)<<14)

#define INT3_MASK_RX0_COHERENT						((int3_rx_ring&0x1)<<13)
#define INT3_MASK_RX1_COHERENT						((int3_rx_ring&0x2)<<14)
#define INT3_MASK_RX_COHERENT_0_1					(INT3_MASK_RX0_COHERENT|INT3_MASK_RX1_COHERENT)
#define INT3_MASK_RX_COHERENT_2_15					((int3_rx_ring&0xFFFC)>>2)

/*INT-4*/
#define INT4_MASK_NO_RX0_CPU_DSCP					((int4_rx_ring&0x1)<<3)
#define INT4_MASK_NO_RX1_CPU_DSCP					((int4_rx_ring&0x2)<<6)
#define INT4_MASK_NO_RX_CPU_DSCP_0_1				(INT4_MASK_NO_RX0_CPU_DSCP|INT4_MASK_NO_RX1_CPU_DSCP)	
#define INT4_MASK_NO_RX_CPU_DSCP_2_15				((int4_rx_ring&0xFFFC)<<14) /*(>>2)<<16*/

#define INT4_MASK_RX0_DONE							((int4_rx_ring&0x1)<<1)
#define INT4_MASK_RX1_DONE							((int4_rx_ring&0x2)<<4)
#define INT4_MASK_RX_DONE_0_1						(INT4_MASK_RX0_DONE|INT4_MASK_RX1_DONE)
#define INT4_MASK_RX_DONE_2_15						((int4_rx_ring&0xFFFC)>>2)

#define INT4_MASK_RX0_DSCP_LOW						((int4_rx_ring&0x1)<<30)
#define INT4_MASK_RX1_DSCP_LOW						((int4_rx_ring&0x2)<<30)
#define INT4_MASK_RX_DSCP_LOW_0_1					(INT4_MASK_RX0_DSCP_LOW|INT4_MASK_RX1_DSCP_LOW)
#define INT4_MASK_RX_DSCP_LOW_2_15					((int4_rx_ring&0xFFFC)<<14)

#define INT4_MASK_RX0_COHERENT						((int4_rx_ring&0x1)<<13)
#define INT4_MASK_RX1_COHERENT						((int4_rx_ring&0x2)<<14)
#define INT4_MASK_RX_COHERENT_0_1					(INT4_MASK_RX0_COHERENT|INT4_MASK_RX1_COHERENT)
#define INT4_MASK_RX_COHERENT_2_15					((int4_rx_ring&0xFFFC)>>2)


/* QDMA_CSR_INT_ENABLE1(base) */
#define INT_MASK_NO_RX_CPU_DSCP(idx)				(1<<(14+idx)) /*RX2 ~RX15, idx :2~15*/
#define INT_MASK_RX_DONE(idx)						(1<<(idx-2))  /*RX2 ~RX15, idx :2~15*/
#define INT_MASK_NO_RX_CPU_DSCP_2_15				(0x3FFF<<16)
#define INT_MASK_RX_DONE_2_15						(0x3FFF)

/* QDMA_CSR_INT_ENABLE2(base) */
#define INT_MASK_RX_DSCP_LOW(idx)					(1<<(14+idx)) /*RX2 ~RX15, idx :2~15*/
#define INT_MASK_RX_COHERENT(idx)					(1<<(idx-2))  /*RX2 ~RX15, idx :2~15*/ 
#define INT_MASK_RX_DSCP_LOW_2_15					(0x3FFF<<16)
#define INT_MASK_RX_COHERENT_2_15					(0x3FFF)

/* QDMA_CSR_TX_DELAY_INT_CFG(base) */
#define DLY_INT_TXDLY_SHIFT(idx)					(idx<<4)
#define DLY_INT_TXDLY_MASK(idx)						(0xFFFF<<DLY_INT_TXDLY_SHIFT(idx))
#define DLY_INT_TXDLY_INT_EN(idx)					(1<<(15+(idx<<4)))
#define DLY_INT_TXMAX_PINT_SHIFT(idx)				(8+(idx<<4))
#define DLY_INT_TXMAX_PINT_MASK(idx)				(0x7F<<DLY_INT_TXMAX_PINT_SHIFT(idx))
#define DLY_INT_TXMAX_PTIME_SHIFT(idx)				(0+(idx<<4))
#define DLY_INT_TXMAX_PTIME_MASK(idx)				(0xFF<<DLY_INT_TXMAX_PTIME_SHIFT(idx))

/* QDMA_CSR_RX_DELAY_INT_CFG(base) */
#define DLY_INT_RXDLY_SHIFT(idx)					(((idx) == 1)?16:0)
#define DLY_INT_RXDLY_MASK(idx)						(0xFFFF<<DLY_INT_RXDLY_SHIFT(idx))
#define DLY_INT_RXDLY_INT_EN(idx)					(((idx) == 1)?(1<<31):(1<<15))
#define DLY_INT_RXMAX_PINT_SHIFT(idx)				(((idx) == 1)?24:8)
#define DLY_INT_RXMAX_PINT_MASK(idx)				(0x7F<<DLY_INT_RXMAX_PINT_SHIFT(idx))
#define DLY_INT_RXMAX_PTIME_SHIFT(idx)				(((idx) == 1)?16:0)
#define DLY_INT_RXMAX_PTIME_MASK(idx)				(0xFF<<DLY_INT_RXMAX_PTIME_SHIFT(idx))

/* QDMA_CSR_IRQ_BASE(base) */

/* QDMA_CSR_IRQ_CFG(base) */
#if  defined(TCSUPPORT_CPU_EN7517)
#define IRQ_CFG_THRESHOLD_SHIFT						(16)
#define IRQ_CFG_THRESHOLD_MASK						(0xFFFF<<IRQ_CFG_THRESHOLD_SHIFT)
#define IRQ_CFG_DEPTH_SHIFT							(0)
#define IRQ_CFG_DEPTH_MASK							(0xFFFF<<IRQ_CFG_DEPTH_SHIFT)

#define IRQ_CFG_RINGIDX_SHIFT						(16)
#define IRQ_CFG_RINGIDX_MASK						(0x1<<IRQ_CFG_RINGIDX_SHIFT)
#define IRQ_CFG_IDX_MASK							0xFFFF

/* QDMA_CSR_IRQ_CLEAR_LEN(base) */
#define IRQ_CLEAR_LEN_SHIFT							(0)
#define IRQ_CLEAR_LEN_MASK							(0xFF<<IRQ_CLEAR_LEN_SHIFT)

/* QDMA_CSR_IRQ_STATUS(base) */
#define IRQ_STATUS_ENTRY_LEN_SHIFT					(16)
#define IRQ_STATUS_ENTRY_LEN_MASK					(0xFFFF<<IRQ_STATUS_ENTRY_LEN_SHIFT)
#define IRQ_STATUS_HEAD_IDX_SHIFT					(0)
#define IRQ_STATUS_HEAD_IDX_MASK					(0xFFFF<<IRQ_STATUS_HEAD_IDX_SHIFT)
#else
#define IRQ_CFG_THRESHOLD_SHIFT						(16)
#define IRQ_CFG_THRESHOLD_MASK						(0xFFF<<IRQ_CFG_THRESHOLD_SHIFT)
#define IRQ_CFG_DEPTH_SHIFT							(0)
#define IRQ_CFG_DEPTH_MASK							(0xFFF<<IRQ_CFG_DEPTH_SHIFT)

#define IRQ_CFG_RINGIDX_SHIFT						(16)
#define IRQ_CFG_RINGIDX_MASK						(0x1<<IRQ_CFG_RINGIDX_SHIFT)
#define IRQ_CFG_IDX_MASK							0xFFF

/* QDMA_CSR_IRQ_CLEAR_LEN(base) */
#define IRQ_CLEAR_LEN_SHIFT							(0)
#define IRQ_CLEAR_LEN_MASK							(0xFF<<IRQ_CLEAR_LEN_SHIFT)

/* QDMA_CSR_IRQ_STATUS(base) */
#define IRQ_STATUS_ENTRY_LEN_SHIFT					(16)
#define IRQ_STATUS_ENTRY_LEN_MASK					(0xFFF<<IRQ_STATUS_ENTRY_LEN_SHIFT)
#define IRQ_STATUS_HEAD_IDX_SHIFT					(0)
#define IRQ_STATUS_HEAD_IDX_MASK					(0xFFF<<IRQ_STATUS_HEAD_IDX_SHIFT)
#endif                                               	
/* QDMA_CSR_IRQ_PTIME(base) */
#define IRQ_PTIME_SHIFT								(0)
#define IRQ_PTIME_MASK								(0xFFFF<<IRQ_PTIME_SHIFT)

/* QDMA_CSR_TXWRR_MODE_CFG	 */
#define TXWRR_WEIGHT_SCALE							(1<<31)
#define TXWRR_WEIGHT_BASE							(1<<3)

/* QDMA_CSR_TXWRR_WEIGHT_CFG	 */
#define TXWRR_RWCMD									(1<<31)
#define TXWRR_RWCMD_DONE							(1<<30)
#define TXWRR_CHNL_IDX_SHIFT						(19)
#define TXWRR_CHNL_IDX_MASK							(0x1F<<TXWRR_CHNL_IDX_SHIFT)
#define TXWRR_QUEUE_IDX_SHIFT						(16)
#define TXWRR_QUEUE_IDX_MASK						(0x7<<TXWRR_QUEUE_IDX_SHIFT)
#define TXWRR_WRR_VALUE_SHIFT						(0)
#define TXWRR_WRR_VALUE_MASK						(0xFF<<TXWRR_WRR_VALUE_SHIFT)

/* QDMA_CSR_DMA_PREFETCH_CFG	 */
#define TXBUF_PREFETCH_OVERDRAG_EN					(1<<15)
#define TXBUF_PREFETCH_OVERDRAG_CNT_SHIFT			(0)
#define TXBUF_PREFETCH_OVERDRAG_CNT_MASK			(0x7FFF<<TXBUF_PREFETCH_OVERDRAG_CNT_SHIFT)

/* QDMA_CSR_PSE_BUF_USAGE_CFG	 */
#define PSE_BUF_CTRL_EN								(1<<31)
#define PSE_BUF_PREFETCH_EN							(1<<30)
#define PSE_BUF_ESTIMATE_EN							(1<<29)
#define TXBUF_CHNL_THRSHLD_SHIFT					(16)
#define TXBUF_CHNL_THRSHLD_MASK						(0xFF<<TXBUF_CHNL_THRSHLD_SHIFT)
#define TXBUF_TOTAL_THRSHLD_SHIFT					(0)
#define TXBUF_TOTAL_THRSHLD_MASK					(0xFFF<<TXBUF_TOTAL_THRSHLD_SHIFT)

/* QDMA_CSR_EGRESS_RATEMETER_CFG	 */
#define EGRESS_RATEMETER_EN							(1<<31)
#define EGRESS_RATEMETER_PEEKRATE_EN				(1<<30)
#define EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT	(20)
#define EGRESS_RATEMETER_PEEKRATE_DURATION_MASK		(0xFF<<EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT)
#define EGRESS_RATEMETER_TIMEDIVESEL_SHIFT			(16)
#define EGRESS_RATEMETER_TIMEDIVESEL_MASK			(0x3<<EGRESS_RATEMETER_TIMEDIVESEL_SHIFT)
#define EGRESS_RATEMETER_TIMESLICE_SHIFT			(0)
#define EGRESS_RATEMETER_TIMESLICE_MASK				(0xFFFF<<EGRESS_RATEMETER_TIMESLICE_SHIFT)

/* QDMA_CSR_EGRESS_RATELIMIT_CFG	 */
#define EGRESS_RATELIMIT_EN							(1<<31)
#define EGRESS_RATELIMIT_BUCKETSCALE_SHIFT			(16)
#define EGRESS_RATELIMIT_BUCKETSCALE_MASK			(0xF<<EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)
#define EGRESS_RATELIMIT_TICKSEL_SHIFT				(0)
#define EGRESS_RATELIMIT_TICKSEL_MASK				(0xFFFF<<EGRESS_RATELIMIT_TICKSEL_SHIFT)

/* QDMA_CSR_RATELIMIT_PARAMETER_CFG	 */
#define EGRESS_RATELIMIT_PARA_RWCMD					(1<<31)
#define EGRESS_RATELIMIT_PARA_RWCMD_DONE			(1<<30)
#define EGRESS_RATELIMIT_PARA_TYPE_SHIFT			(28)
#define EGRESS_RATELIMIT_PARA_TYPE_MASK				(0x3<<EGRESS_RATELIMIT_PARA_TYPE_SHIFT)
#define EGRESS_RATELIMIT_PARA_IDX_SHIFT				(16)
#define EGRESS_RATELIMIT_PARA_IDX_MASK				(0x1F<<EGRESS_RATELIMIT_PARA_IDX_SHIFT)
#define EGRESS_RATELIMIT_PARA_VALUE_SHIFT			(0)
#define EGRESS_RATELIMIT_PARA_VALUE_MASK			(0xFFFF<<TXWRR_WRR_VALUE_SHIFT)

/* QDMA_CSR_TXQ_CNGST_CFG	 */
#define TXQ_CNGST_DROP_EN							(1<<31)
#define TXQ_CNGST_DEI_DROP_EN						(1<<30)
#define TXQ_DYN_CNGSTCTL_EN							(1<<29)
#define TXQ_DYN_CNGSTCTL_CTX_RING1_DROP_EN			(1<<28)
#define TXQ_DYN_CNGSTCTL_MAX_THRH_TXR1_EN			(1<<27)
#define TXQ_DYN_CNGSTCTL_MIN_THRH_TXR1_EN			(1<<26)
#define TXQ_DYN_CNGSTCTL_MAX_THRH_TXR0_EN			(1<<25)
#define TXQ_DYN_CNGSTCTL_MIN_THRH_TXR0_EN			(1<<24)
#define TXQ_DYN_CNGSTCTL_THR_MARGIN_SHIFT			(22)
#define TXQ_DYN_CNGSTCTL_THR_MARGIN_MASK			(0x3<<TXQ_DYN_CNGSTCTL_THR_MARGIN_SHIFT)
#define TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_SHIFT		(20)
#define TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_MASK			(0x3<<TXQ_DYN_CNGSTCTL_DEI_THR_SCALE_SHIFT)
#define TXQ_DYN_CNGSTCTL_CTX_RING0_DROP_EN			(1<<19)
#define TXQ_DYN_CNGSTCTL_MODE_CONFIG_TRIGGER		(1<<18)
#define TXQ_DYN_CNGSTCTL_MODE_PACKET_TRIGGER		(1<<17)
#define TXQ_DYN_CNGSTCTL_MODE_TIME_TRIGGER			(1<<16)
#define TXQ_DYN_CNGSTCTL_TICKSEL_SHIFT				(0)
#define TXQ_DYN_CNGSTCTL_TICKSEL_MASK				(0xFFFF<<TXQ_DYN_CNGSTCTL_TICKSEL_SHIFT)

/* QDMA_CSR_TXQ_DYN_TOTALTHR	 */
#define TXQ_CNGST_TOTAL_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_TOTAL_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_TOTAL_MAX_THR_SHIFT)
#define TXQ_CNGST_TOTAL_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_TOTAL_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_TOTAL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DYN_CHNLTHR_CFG	 */
#define TXQ_CNGST_CHNL_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_CHNL_MAX_THR_MASK					(0xFFFF<<TXQ_CNGST_CHNL_MAX_THR_SHIFT)
#define TXQ_CNGST_CHNL_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_CHNL_MIN_THR_MASK					(0xFFFF<<TXQ_CNGST_CHNL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DYN_QUEUETHR_CFG	 */
#define TXQ_CNGST_QUEUE_MAX_THR_SHIFT				(16)
#define TXQ_CNGST_QUEUE_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_QUEUE_MAX_THR_SHIFT)
#define TXQ_CNGST_QUEUE_MIN_THR_SHIFT				(0)
#define TXQ_CNGST_QUEUE_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_QUEUE_MIN_THR_SHIFT)

/* QDMA_CSR_STATIC_QUEUE_THR	 */
#define TXQ_STATIC_QUEUE_DEI_THR_SHIFT				(16)
#define TXQ_STATIC_QUEUE_DEI_THR_MASK				(0xFFFF<<TXQ_STATIC_QUEUE_DEI_THR_SHIFT)
#define TXQ_STATIC_QUEUE_NORM_THR_SHIFT				(0)
#define TXQ_STATIC_QUEUE_NORM_THR_MASK				(0xFFFF<<TXQ_STATIC_QUEUE_NORM_THR_SHIFT)


/* QDMA_CSR_DBG_MEM_XS_CFG  */
#define DBG_MEM_XS_RWCMD							(1<<31)
#define DBG_MEM_XS_RWCMD_DONE						(1<<30)
#define DBG_MEM_XS_MEMSEL_SHIFT						(24)
#define DBG_MEM_XS_MEMSEL_MASK						(0x7<<DBG_MEM_XS_MEMSEL_SHIFT)
#define DBG_MEM_XS_BYTESEL_SHIFT					(16)
#define DBG_MEM_XS_BYTESEL_MASK						(0x1F<<DBG_MEM_XS_BYTESEL_SHIFT)
#define DBG_MEM_XS_ADDR_SHIFT						(0)
#define DBG_MEM_XS_ADDR_MASK						(0xFFFF<<DBG_MEM_XS_ADDR_SHIFT)

/* QDMA_CSR_DBG_MEM_XS_DATA_LO  */
#define DBG_MEM_XS_QUEUE_INFO_CNT_SHIFT             (0)
#define DBG_MEM_XS_BUF_SHIFT_0						(0)
#define DBG_MEM_XS_BUF_SHIFT_24						(24)

#define DBG_MEM_XS_BUF_MASK							(0xFFFFFF)
#if defined(QDMA_LAN)
#define DBG_MEM_XS_QUEUE_INFO_CNT_MASK              (0x7FF)
#define DBG_MEM_XS_QUEUE_INFO_TAIL_SHIFT            (11)
#define DBG_MEM_XS_QUEUE_INFO_TAIL_MASK             (0x3FF)
#define DBG_MEM_XS_QUEUE_INFO_HEAD_SHIFT            (21)
#define DBG_MEM_XS_QUEUE_INFO_HEAD_MASK             (0x3FF)
#define DBG_MEM_XS_DSCP_IDX_MASK                    (0x3FF)
#else
#define DBG_MEM_XS_QUEUE_INFO_CNT_MASK              (0x1FFF)
#define DBG_MEM_XS_QUEUE_INFO_TAIL_SHIFT            (13)
#define DBG_MEM_XS_QUEUE_INFO_TAIL_MASK             (0xFFF)
#define DBG_MEM_XS_QUEUE_INFO_HEAD_SHIFT            (25)
#define DBG_MEM_XS_QUEUE_INFO_HEAD_MASK             (0xFFF)
#define DBG_MEM_XS_DSCP_IDX_MASK                    (0xFFF)
#endif



/* QDMA_CSR_DBG_MEM_XS_DATA_HI  */


/* QDMA_CSR_DBG_LMGR_STATUS	 */
#define DBG_LMGR_FREE_CNT_SHIFT						(0)
#define DBG_LMGR_FREE_CNT_MASK						(0x1FFF<<DBG_LMGR_FREE_CNT_SHIFT)

/* QDMA_CSR_DBG_LMGR_USAGE	 */
#define DBG_LMGR_TOTALBUF_USAGE_SHIFT				(0)
#define DBG_LMGR_TOTALBUF_USAGE_MASK				(0xFFFFFF<<DBG_LMGR_FREE_CNT_SHIFT)

#if  defined(TCSUPPORT_CPU_EN7517)
/* QDMA_CSR_RX_RING_CFG */ 
//7517 
#define RX_RING_SIZE_SHIFT(idx)						((idx&0xE)?(0):(idx<<4))
#define RX_RING_SIZE_MASK(idx)						((idx&0xE)?(0xFFFF):(0xFFFF<<RX_RING_SIZE_SHIFT(idx)))

/* QDMA_CSR_RX_RING_THR */
#define RX_RING_LOW_THR_SHIFT(idx)					((idx == 0)? 0 : 16)
#define RX_RING_LOW_THR_MASK(idx)					(0xFFFF<<RX_RING_LOW_THR_SHIFT(idx))

#else
/* QDMA_CSR_RX_RING_CFG */
#define RX_RING_SIZE_SHIFT(idx)						((idx&0xE)?(0):(idx<<4))
#define RX_RING_SIZE_MASK(idx)						((idx&0xE)?(0xFFF):(0xFFF<<RX_RING_SIZE_SHIFT(idx)))

/* QDMA_CSR_RX_RING_THR */
#define RX_RING_LOW_THR_SHIFT(idx)					((idx == 0)? 0 : 16)
#define RX_RING_LOW_THR_MASK(idx)					(0xFFF<<RX_RING_LOW_THR_SHIFT(idx))
#endif

/* QDMA_CSR_RX_PROTECT_CFG */
#define RX_DROP_EN									(1<<31)
#define RX_RATELIMIT_EN								(1<<30)
#define RX_RATELIMIT_PKTMODE						(1<<29)
#define RX_RATELIMIT_BUCKET_SCALE_SHIFT				(24)
#define RX_RATELIMIT_BUCKET_SCALE_MASK				(0xF<<RX_RATELIMIT_BUCKET_SCALE_SHIFT)
#define RX_RATELIMIT_TICKSEL_SHIFT					(0)
#define RX_RATELIMIT_TICKSEL_MASK					(0xFFFF<<RX_RATELIMIT_TICKSEL_SHIFT)

/* QDMA_CSR_RX_RATELIMIT_CFG */
#define RX_RATELIMIT_RWCMD							(1<<31)
#define RX_RATELIMIT_RWCMD_DONE						(1<<30)
#define RX_RATELIMIT_TYPE_SHIFT						(28) 
#define RX_RATELIMIT_TYPE_MASK						(0x3<<RX_RATELIMIT_TYPE_SHIFT) 
#define RX_RATELIMIT_RING_IDX(idx)					((idx&0xF)<<16)

/*QDMA_CSR_RX_TICK_CFG*/
#define RX_RATELIMIT_SELTICKER_SHIFT(ringIdx)		(16+ringIdx)
#define RX_RATELIMIT_SELTICKER_MASK(ringIdx)		(1<<RX_RATELIMIT_SELTICKER_SHIFT(ringIdx))

/*QDMA_CSR_RX_PKTOVH_CFG*/
#define CPU_RX_PKTSIZE_OVERHEAD_EN					(1<<8)
#define CPU_RX_PKTSIZE_OVERHEAD_SHIFT 				(0)
#define CPU_RX_PKTSIZE_OVERHEAD_MASK 				(0xFF<<CPU_RX_PKTSIZE_OVERHEAD_SHIFT)
#define RX_RATELIMIT_VALUE_SHIFT					(0)
#define RX_RATELIMIT_VALUE_MASK						(0xFFFF<<RX_RATELIMIT_VALUE_SHIFT)

/* QDMA_CSR_VCHNL_WEIGHT_CFG */
#define VCHNL_WEIGHT_RWCMD							(1<<31)
#define VCHNL_WEIGHT_DONE							(1<<30)
#define PHYSICAL_CHNL_IDX_SHIFT						(18)
#define PHYSICAL_CHNL_IDX_MASK						(0xF<<PHYSICAL_CHNL_IDX_SHIFT)
#define VIRTUAL_CHNL_IDX_SHIFT						(16)
#define VIRTUAL_CHNL_IDX_MASK						(0x3<<VIRTUAL_CHNL_IDX_SHIFT)
#define VCHNL_WRR_VALUE_SHIFT						(0)
#define VCHNL_WRR_VALUE_MASK						(0xFF<<VCHNL_WRR_VALUE_SHIFT)

/* QDMA_CSR_VCHNL_QOS_MODE */
#define PHYSICAL_CHNL_QOS_MODE_SHIFT(idx)			((idx&0xF)<<1)
#define PHYSICAL_CHNL_QOS_MODE_MASK(idx)			(0x3<<PHYSICAL_CHNL_QOS_MODE_SHIFT(idx))

/* QDMA_CSR_RATELIMIT_EN_CFG */
#define TX_CHNL_RATELIMIT_EN(idx)					(1<<idx)

/* QDMA_CSR_PERCHNL_QOS_MODE */
#define TXQOS_CHNL_QOS_MODE_SHIFT(idx)				((idx&0x7)<<2)
#define TXQOS_CHNL_QOS_MODE_MASK(idx)				(0x7<<TXQOS_CHNL_QOS_MODE_SHIFT(idx))

/* QDMA_CSR_PERCHNL_DATARATE */
#define TXQOS_CHNL_EGRESS_DATARATE_SHIFT(idx)		((idx&0x1)<<4)
#define TXQOS_CHNL_EGRESS_DATARATE_MASK(idx)		(0xFFFF<<TXQOS_CHNL_EGRESS_DATARATE_SHIFT(idx))

/* QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG */
#define TXQ_CNGST_QUEUE_NONBLOCKING_EN(idx)					(1<<idx)
/* QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG */
#define TXQ_CNGST_CHANNEL_NONBLOCKING_EN(idx)				(1<<idx)

/* QDMA_CSR_DBG_CNTR_CFG */
#define DBG_CNTR_EN									(1<<31)
#define DBG_CNTR_ENABLE								(1)
#define DBG_CNTR_DISABLE							(0)
#define DBG_CNTR_SRC_SHIFT							(28)
#define DBG_CNTR_SRC_MASK							(0x7<<DBG_CNTR_SRC_SHIFT)
#define DBG_CNTR_SRC_CPU_TX							(0)
#define DBG_CNTR_SRC_FWD_TX							(1)
#define DBG_CNTR_SRC_CPU_RX							(2)
#define DBG_CNTR_SRC_FWD_RX							(3)
#define DBG_CNTR_SRC_UNKNOWN						(4)
#define DBG_CNTR_ALL_CHNL							(1<<27)
#define DBG_CNTR_ALL_QUEUE							(1<<26)
#define DBG_CNTR_ALL_DSCPRING						(1<<25)
#define DBG_CNTR_DSCPRING_IDX_SHIFT					(16)
#define DBG_CNTR_DSCPRING_IDX_MASK					(0xF<<DBG_CNTR_DSCPRING_IDX_SHIFT) /*4 bit*/
#define DBG_CNTR_CHNL_SHIFT							(3)
#define DBG_CNTR_CHNL_MASK							(0x1F<<DBG_CNTR_CHNL_SHIFT)
#define DBG_CNTR_QUEUE_SHIFT						(0)
#define DBG_CNTR_QUEUE_MASK							(0x7<<DBG_CNTR_QUEUE_SHIFT)

#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7517)
#define DBG_CNTR_INDEX_MAX					(64)
#else
#define DBG_CNTR_INDEX_MAX					(40)
#endif
#if defined(TCSUPPORT_CPU_EN7517)
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(12)
#else
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(16)
#endif
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7517)
#define DBG_CNTR_DEFAULT_CONFIG_NUM  		(4+TX_RING_NUM+RX_RING_NUM) /*tx_cpu, tx_total, rx_cpu, rx_total, tx ring0~1, rx ring0~15*/
#else
#define DBG_CNTR_DEFAULT_CONFIG_NUM  		(8)
#endif
#define DBG_CNTR_DEFAULT_CONFIG_OFFSET  	(32)


/* for EN7528 */
#define QDMA_CSR_GLB_RATECTL_CFG(base)		(base+0x150)
#define QDMA_CSR_GLB_RATECTL_PARAM_CFG(base)	(base+0x154)
#define QDMA_CSR_GLB_RATECTL_DATA_LO(base)	(base+0x158)
#define QDMA_CSR_GLB_RATECTL_DATA_HI(base)	(base+0x15c)

#define TRTCM_PARA_RWCMD			(1<<31)
#define TRTCM_PARA_RWCMD_DONE			(1<<30)
#define TRTCM_PARA_TYPE_SHIFT			(28)
#define TRTCM_PARA_TYPE_MASK			(0x3<<TRTCM_PARA_TYPE_SHIFT)
#define TRTCM_PARA_IDX_RATE_TYPE_SHIFT		(16)
#define TRTCM_PARA_IDX_RATE_TYPE_MASK		(1<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)  //pir&pbs or cir&cbs
#define TRTCM_PARA_IDX_INDEX_SHIFT		(17)
#define TRTCM_PARA_IDX_INDEX_MASK		(0x3F<<TRTCM_PARA_IDX_INDEX_SHIFT)	 //7580

#define RATELIMIT_PARA_RWCMD			(1<<31)
#define RATELIMIT_PARA_RWCMD_DONE		(1<<30)
#define RATELIMIT_PARA_TYPE_SHIFT		(28)
#define RATELIMIT_PARA_TYPE_MASK		(0x3<<RATELIMIT_PARA_TYPE_SHIFT)
#define RATELIMIT_PARA_IDX_INDEX_SHIFT		(16)
#define RATELIMIT_PARA_IDX_INDEX_MASK		(0x7F<<RATELIMIT_PARA_IDX_INDEX_SHIFT)	 //7580

#define TRTCM_CFG(trtcm_base)			(trtcm_base)
#define TRTCM_PARAM_CFG(trtcm_base)		(trtcm_base+0x4)
#define TRTCM_DATA_LO(trtcm_base)		(trtcm_base+0x8)
#define TRTCM_DATA_HI(trtcm_base)		(trtcm_base+0xc)

#define TRTCM_EN_SHIFT				(31)
#define TRTCM_EN_MASK				(1<<TRTCM_EN_SHIFT)
#define TRTCM_MODE_SHIFT			(30)
#define TRTCM_MODE_MASK				(1<<TRTCM_MODE_SHIFT)
#define TRTCM_SLOW_TICKRATIO_SHIFT		(16)
#define TRTCM_SLOW_TICKRATIO_MASK		(0x3FFF<<TRTCM_SLOW_TICKRATIO_SHIFT)
#define TRTCM_FAST_TICK_SHIFT			(0)
#define TRTCM_FAST_TICK_MASK			(0xFFFF<<TRTCM_FAST_TICK_SHIFT)

#define TRTCM_METER_MODE			(1<<2)
#define TRTCM_PKT_MODE				(1<<1)
#define TRTCM_TICK_SEL				(1<<0)

#define TRTCM_TOKEN_RATE_INTEGER_SHIFT		(6)
#define TRTCM_TOKEN_RATE_INTEGER_MASK		(0x3FFFF<<TRTCM_TOKEN_RATE_INTEGER_SHIFT)
#define TRTCM_TOKEN_RATE_FRACTION_MASK		(0x3F)

#define TRTCM_BUCKET_SIZE_SHIFT_MASK		(0xF)

/*30-bit integer with 6-bit fraction*/
#define TRTCM_BUCKET_COUNTER_FRACTION_MASK  	(0x3F)
#define TRTCM_BUCKET_COUNTER_INTEGER_SHIFT	(6)
#define TRTCM_BUCKET_COUNTER_LO_MASK		(0x3FFFFFF<<TRTCM_BUCKET_COUNTER_INTEGER_SHIFT)
#define TRTCM_BUCKET_COUNTER_HI_MASK		(0x7)
#define TRTCM_BUCKET_COUNTER_SIGN_BIT_HI_MASK	(0x1<<3)
#define TRTCM_BUCKET_COUNTER_INTEGER_FIN_SHIFT	(26)


/*QDMA_CSR_RATELIMIT_PER_CHNL_TICKSEL*/

/*QDMA_CSR_EGRESS_RATELIMIT_CFG2*/

/*QDMA_CSR_DBA_CHEAT_CFG*/

/**/


/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern uint int1_rx_ring;
extern uint int2_rx_ring;
extern uint int3_rx_ring;
extern uint int4_rx_ring;

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _QDMA_REG_H_ */
