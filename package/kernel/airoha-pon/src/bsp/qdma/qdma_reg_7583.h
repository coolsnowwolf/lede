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
#ifndef _QDMA_REG_7583_H_
#define _QDMA_REG_7583_H_

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
#ifdef TCSUPPORT_CPU_ARMV8
	#define IO_GREG(reg)							get_frame_engine_data(reg)
	#define IO_SREG(reg, value) 					set_frame_engine_data(reg, value)
	#define IO_GMASK(reg, mask, shift)				(((uint)get_frame_engine_data(reg) & mask) >> shift)
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
    #define CONFIG_QDMA_IRQ_1                           (QDMA_LAN0_INTR)    /*careful of 1-bit offset*/
    #define CONFIG_QDMA_IRQ_2                           (QDMA_LAN1_INTR)    /*----Reserve INT2 for LRO_Driver.----*/
    #define CONFIG_QDMA_IRQ_3                           (QDMA_LAN2_INTR)
    #define CONFIG_QDMA_IRQ_4                           (QDMA_LAN3_INTR)	
    #define RING_OFFSET(idx)                            (idx<<5)
#else
    #define CONFIG_QDMA_BASE_ADDR                       (0x1FB56000)
    #define QDMA_REG_BASE                               (0xBFB56000)
    #define FE_GDMA_FWD_CFG                             (0xbfb51500)
    #define CONFIG_QDMA_IRQ_1                           (QDMA_WAN0_INTR)    /*careful of 1-bit offset*/
    #define CONFIG_QDMA_IRQ_2                           (QDMA_WAN1_INTR)    /*----Reserve INT2 for LRO_Driver.----*/	
    #define CONFIG_QDMA_IRQ_3                           (QDMA_WAN2_INTR)
    #define CONFIG_QDMA_IRQ_4                           (QDMA_WAN3_INTR)
    #define RING_OFFSET(idx)                            (idx<<5)
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
#ifdef TCSUPPORT_CPU_ARMV8
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

#define QDMA_CSR_HWFWD_DSCP_BASE(base)				(base+0x0010)
#define QDMA_CSR_HWFWD_BUFF_BASE(base)				(base+0x0014)
#define QDMA_CSR_HWFWD_DSCP_CFG(base)				(base+0x0018)

#define QDMA_CSR_INT_STATUS1(base)					(base+0x0020)
#define QDMA_CSR_INT_STATUS2(base)					(base+0x0024)

#define QDMA_CSR_INT_STATUS3(base)					(base+0x0720)
#define QDMA_CSR_INT_STATUS4(base)					(base+0x0724)
#define QDMA_CSR_INT_STATUS5(base)					(base+0x0730)


#if 1 /*idx: 1~4*/
static inline uint __get_csr_int_enable_register(uint base, int intNo, int enNo)
{
	if((enNo) <= 2)
	{
		return (base + 0x0024 + ((intNo-1)<<3) + (enNo<<2));
	}
	else if((enNo) == 3)
	{
		return (base + 0x0740 + ((intNo-1)<<5));
	}
	else if((enNo) == 4)
	{
		return (base + 0x0744 + ((intNo-1)<<5));
	}
	else
	{
		return (base + 0x0750 + ((intNo-1)<<5));
	}
	return 0;
}

#define QDMA_CSR_INT_EN(base,intNo,enNo)			__get_csr_int_enable_register(base,intNo,enNo)

#else
#define QDMA_CSR_INT_ENABLE1(base,idx)				(base+0x0020+((idx)<<3))
#define QDMA_CSR_INT_ENABLE2(base,idx)				(base+0x0024+((idx)<<3))
#endif

#define QDMA_CSR_IRQ_BASE(base)						(base+0x0050)
#define QDMA_CSR_IRQ_CFG(base)						(base+0x0054)
#define QDMA_CSR_IRQ_CLEAR_LEN(base)				(base+0x0058)
#define QDMA_CSR_IRQ_STATUS(base)					(base+0x005C)
#define QDMA_CSR_IRQ_PTIME(base)					(base+0x0060)

#define QDMA_CSR_IRQ2_BASE(base)					(base+0x0048)
#define QDMA_CSR_IRQ2_CFG(base)						(base+0x004C)
#define QDMA_CSR_IRQ2_CLEAR_LEN(base)				(base+0x0064)
#define QDMA_CSR_IRQ2_STATUS(base)					(base+0x0068)
#define QDMA_CSR_IRQ2_PTIME(base)					(base+0x006C)

#define QDMA_CSR_INGRESS_TRTCM_CFG(base)			(base+0x0070)
#define QDMA_CSR_INGRESS_TRTCM_PARAM_CFG(base)		(base+0x0074)
#define QDMA_CSR_INGRESS_TRTCM_DATA_LO(base)		(base+0x0078)
#define QDMA_CSR_INGRESS_TRTCM_DATA_HI(base)		(base+0x007c)

#define QDMA_CSR_INX_COLOR_RES_CFG(base)		(base+0x00e0)
#define QDMA_CSR_INX_COLOR_RES_CFG1(base)		(base+0x00e4)

#define QDMA_CSR_EGRESS_QUEUE_RATE_CFG(base)		(base+0x11c0)
#define QDMA_CSR_EGRESS_QUEUE_RATE_PARAM_CFG(base)	(base+0x11c4)
#define QDMA_CSR_EGRESS_QUEUE_RATE_DATA_LO(base)	(base+0x11c8)
#define QDMA_CSR_EGRESS_QUEUE_RATE_DATA_HI(base)	(base+0x11cc)

#define QDMA_CSR_INGRESS_WRED_NORM_CFG(base)		(base+0x1090)
#define	QDMA_CSR_INGRESS_WRED_DEI_CFG(base)			(base+0x1094)
#define	QDMA_CSR_INGRESS_WRED_NORM_PROBABILITY(base)	(base+0x0080)
#define	QDMA_CSR_INGRESS_WRED_DEI_PROBABILITY(base)		(base+0x0084)

#define QDMA_CSR_CPU_RX_RED_CFG(base)   			(base+0x0088)

#define QDMA_CSR_CRX_PKT_OVH_CFG(base)   			(base+0x008c)

#define QDMA_CSR_HQOS_MODE_CFG(base)				(base+0x0090)

#define QDMA_CSR_QUEUE_CLOSE_CFG(base, idx)         (base+0x00a0+TXQ_DIS_QUEUE_CLOSE_OFFSET(idx))

#define QDMA_CSR_FLOWCNT_GRP_CFG(base)   			(base+0x00f0)

#define QDMA_CSR_TX_DSCP_BASE(base, idx)			(((idx)<8) ? (base+0x0100+RING_OFFSET(idx)) : (base+0x0b00+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_BLOCKING(base, idx)				(((idx)<8) ? (base+0x0104+RING_OFFSET(idx)) : (base+0x0b04+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_CPU_IDX(base, idx)				(((idx)<8) ? (base+0x0108+RING_OFFSET(idx)) : (base+0x0b08+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_DMA_IDX(base, idx)				(((idx)<8) ? (base+0x010c+RING_OFFSET(idx)) : (base+0x0b0c+RING_OFFSET(((idx)-8))))
#define QDMA_CSR_TX_CPU_DONE_NUM(base, idx)			(((idx)<8) ? (base+0x011c+RING_OFFSET(idx)) : (base+0x0b1C+RING_OFFSET(((idx)-8)))) 

#define QDMA_CSR_RX_DSCP_BASE(base, idx)			(((idx)<16) ? (base+0x0200+RING_OFFSET(idx)) : (base+0x0e00+RING_OFFSET(((idx)-16))))
#define QDMA_CSR_RX_RING_SIZE(base, idx)			(((idx)<16) ? (base+0x0204+RING_OFFSET(idx)) : (base+0x0e04+RING_OFFSET(((idx)-16))))
#define QDMA_CSR_RX_CPU_IDX(base, idx)				(((idx)<16) ? (base+0x0208+RING_OFFSET(idx)) : (base+0x0e08+RING_OFFSET(((idx)-16))))
#define QDMA_CSR_RX_DMA_IDX(base, idx)				(((idx)<16) ? (base+0x020C+RING_OFFSET(idx)) : (base+0x0e0c+RING_OFFSET(((idx)-16))))
#define QDMA_CSR_RX_DELAY_INT_CFG(base, idx)		(((idx)<16) ? (base+0x0210+RING_OFFSET(idx)) : (base+0x0e10+RING_OFFSET(((idx)-16))))

#define QDMA_CSR_DBG_CNTR_CFG(base, i)				(base+0x0400+(i<<3))
#define QDMA_CSR_DBG_CNTR_VAR(base, i)				(base+0x0404+(i<<3))

#define QDMA_CSR_DBG_CNTMEM_EN(base)				(base+0x0600)
#define QDMA_CSR_DBG_CNTMEM_CFG(base)				(base+0x0604)
#define QDMA_CSR_DBG_CNTMEM_VAR(base)				(base+0x0608)

#define QDMA_CSR_DBG_ALL_CTX_NUM(base)				(base+0x0610)
#define QDMA_CSR_DBG_ALL_FTX_NUM(base)				(base+0x0614)
#define QDMA_CSR_DBG_ALL_FRX_NUM(base)				(base+0x0618)
#define QDMA_CSR_DBG_ALL_MULTICAST_DROP_NUM(base)	(base+0x0620)
#define QDMA_CSR_DBG_ALL_AGING_DROP_NUM(base)		(base+0x0624)

#define QDMA_CSR_MULTICAST_MODIFY_FPORT(base, chnl)	(base+0x0880+(chnl<<2))
#define QDMA_CSR_DBG_MULTICAST_DROP_NUM(base, chnl)	(base+0x0680+((chnl>>1)<<2))

/* EN7580 */
#define QDMA_CSR_FLOW_CNT_BYTE_LOW(base, grp, idx)  (base+0x0800+(grp<<10)+(idx<<4))
#define QDMA_CSR_FLOW_CNT_BYTE_HIGH(base, grp, idx) (base+0x0804+(grp<<10)+(idx<<4))
#define QDMA_CSR_FLOW_CNT_PKTCNT(base, grp, idx)    (base+0x0808+(grp<<10)+(idx<<4))

/* EN7581 */
#define QDMA_CSR_SDN_CNTR_CFG(base)					(base+0x0800)
#define QDMA_CSR_SDN_CNTR_PARAM_CFG(base)			(base+0x0804)
#define QDMA_CSR_SDN_CNTR_DATA_LO(base)				(base+0x0808)
#define QDMA_CSR_SDN_CNTR_DATA_HI(base)				(base+0x080c)

/**0x1000**/
#define QDMA_CSR_LMGR_INIT_CFG(base)				(base+0x1000)
#define QDMA_CSR_HWFWD_LOW_THRH(base)				(base+0x1004)
#define QDMA_CSR_TXQ_MIN_DSCP_THRH(base)			(base+0x1008)

#define QDMA_CSR_EGRESS_RATEMETER_CFG(base)			(base+0x100c)
#define QDMA_CSR_EGRESS_TRTCM_CFG(base)				(base+0x1010)
#define QDMA_CSR_EGRESS_TRTCM_PARAM_CFG(base)		(base+0x1014)
#define QDMA_CSR_EGRESS_TRTCM_DATA_LO(base)			(base+0x1018)
#define QDMA_CSR_EGRESS_TRTCM_DATA_HI(base)			(base+0x101c)

#define QDMA_CSR_TXWRR_MODE_CFG(base)				(base+0x1020)
#define QDMA_CSR_TXWRR_WEIGHT_CFG(base)				(base+0x1024)

/*EN7580: new PSE buf manager engine, the function is not needed*/
#define QDMA_CSR_PSE_BUF_USAGE_CFG(base)			(base+0x1028)
#define QDMA_CSR_PSE_BUF_USAGE_CFG1(base)			(base+0x10d0)

#define QDMA_CSR_TXQ_CNGST_QUEUE_NONBLOCKING_CFG(base)			(base+0x102c)
#define QDMA_CSR_TXQ_CNGST_CHANNEL_NONBLOCKING_CFG(base)		(base+0x1030)
#define QDMA_CSR_PERCHNL_QOS_MODE(base, i)			(base+0x1040+(i<<2))

#define QDMA_CSR_GLB_RATECTL_CFG(base)              (base+0x1080)
#define QDMA_CSR_GLB_RATECTL_PARAM_CFG(base)        (base+0x1084)
#define QDMA_CSR_GLB_RATECTL_DATA_LO(base)          (base+0x1088)
#define QDMA_CSR_GLB_RATECTL_DATA_HI(base)          (base+0x108c)

#define QDMA_CSR_CNGST_WRED_NORM_CFG(base)          (base+0x1090)
#define QDMA_CSR_CNGST_WRED_DEI_CFG(base)           (base+0x1094)

#define QDMA_CSR_TXQ_CNGST_CFG(base)				(base+0x10a0)
#define QDMA_CSR_TXQ_DYN_TOTALTHR(base)				(base+0x10a4)
#define QDMA_CSR_TXQ_DYN_CHNLTHR_CFG(base)			(base+0x10a8)
#define QDMA_CSR_TXQ_DYN_QUEUETHR_CFG(base)			(base+0x10ac)
#define QDMA_CSR_STATIC_THR_CFG(base)				(base+0x10b0)
#define QDMA_CSR_STATIC_QUEUE_RATIO(base)			(base+0x10b4)

#define QDMA_CSR_QOS_AGING_CFG(base)				(base+0x10bc)
#define QDMA_CSR_QUEUE_AGING_EN_CFG(base, chnl)		(base+0x1180+((chnl/4)<<2))

#define QDMA_CSR_TXQ_DEI_TOTALTHR(base)				(base+0x10d4)
#define QDMA_CSR_TXQ_DEI_CHNLTHR_CFG(base)			(base+0x10d8)
#define QDMA_CSR_TXQ_DEI_QUEUETHR_CFG(base)			(base+0x10dc)

#define QDMA_CSR_DBG_LMGR_STATUS(base)				(base+0x10f0)
#define QDMA_CSR_DBG_MIN_LMGR_STATUS(base)			(base+0x110c)
#define QDMA_CSR_DBG_LMGR_USAGE(base)				(base+0x10f4)
#define QDMA_CSR_DBG_QDMA_PROBE_LO(base)			(base+0x10F8)
#define QDMA_CSR_DBG_QDMA_PROBE_HI(base)			(base+0x10FC)

#define QDMA_CSR_DBG_MEM_XS_CFG(base)				(base+0x1100)
#define QDMA_CSR_DBG_MEM_XS_DATA_LO(base)			(base+0x1104)
#define QDMA_CSR_DBG_MEM_XS_DATA_HI(base)			(base+0x1108)

#define	QDMA_CSR_EPON_DBA_MODE_CFG(base)			(base+0x1120)

#define QDMA_CSR_LMGR_VCHNL_CFG(base)				(base+0x1130)
#define QDMA_CSR_VCHNL_WEIGHT_CFG(base)				(base+0x1134)
#define QDMA_CSR_VCHNL_QOS_MODE(base)				(base+0x1140)

#define QDMA_CSR_SLA_CHNL_CFG(base)					(base+0x114C)
#define QDMA_CSR_SLA_TRTCM_CFG(base)				(base+0x1150)
#define QDMA_CSR_SLA_TRTCM_PARAM_CFG(base)			(base+0x1154)
#define QDMA_CSR_SLA_TRTCM_DATA_LO(base)			(base+0x1158)
#define QDMA_CSR_SLA_TRTCM_DATA_HI(base)			(base+0x115C)

#define QDMA_CSR_PERCHNL_DATARATE(base, i)			(base+0x1200+(i<<2))
/*0x1280: CHN0~3, 0x1284: CHN4~7, 0x1288: CHN8~11, 0x128c: CHN12~15*/
/*0x1290: CHN16~19, 0x1294: CHN20~23, 0x1298: CHN24~27, 0x129c: CHN28~31*/
#define QDMA_CSR_CHNL_QUEUE_EMPTY(base, chnl)			(base+0x1280+((chnl)&0xFC))

/*******************************************************************************
*******************************************************************************/
/* QDMA_CSR_INFO(base) */

/* QDMA_CSR_GLB_CFG(base) */
#define GLB_CFG_RX_2B_OFFSET						(1<<31)

#define GLB_CFG_DMA_PREFERENCE_SHIFT				(29)
#define GLB_CFG_DMA_PREFERENCE_MASK					(0x3<<GLB_CFG_DMA_PREFERENCE_SHIFT)
#define PREFER_ROIND_ROBIN							(0x0)
#define PREFER_FWD_CTX                              (0x1)
#define PREFER_RESERVE                              (0x2)
#define PREFER_CTX_FWD                              (0x3)

#define GLB_CFG_CPU_TXR_ROUND_ROBIN                 (1<<28)
#define GLB_CFG_DSCP_BYTE_SWAP						(1<<27)
#define GLB_CFG_PAYLOAD_BYTE_SWAP					(1<<26)
#define GLB_CFG_MULTICAST_MODIFY_FP                 (1<<25)
#define GLB_CFG_MULTICAST_MODIFY_SHIFT              (25) 
#define GLB_CFG_MULTICAST_MODIFY_MASK               (1<<GLB_CFG_MULTICAST_MODIFY_SHIFT) 
#define GLB_CFG_OAM_MODIFY_FP                       (1<<24)
#define GLB_CFG_OAM_MODIFY_SHIFT                    (24) 
#define GLB_CFG_OAM_MODIFY_MASK                     (1<<GLB_CFG_OAM_MODIFY_SHIFT) 
#define GLB_CFG_QDMA_QUICK_RESET					(1<<23)
#define GLB_CFG_QDMA_QUICK_RESET_SHIFT				(23)
#define GLB_CFG_QDMA_QUICK_RESET_MASK				(1<<GLB_CFG_QDMA_QUICK_RESET_SHIFT)
#define GLB_CFG_QDMA_QUICK_RESET_DONE_SHIFT         (22)
#define GLB_CFG_QDMA_QUICK_RESET_DONE_MASK          (1<<GLB_CFG_QDMA_QUICK_RESET_DONE_SHIFT)
#define GLB_CFG_MULTICAST_EN						(1<<21)
#define GLB_CFG_MULTICAST_EN_SHIFT					(21) 
#define GLB_CFG_MULTICAST_EN_MASK					(1<<GLB_CFG_MULTICAST_EN_SHIFT) 
#define GLB_CFG_SLM_RELEASE_EN						(1<<20) /* not support in EN7580 */
#define GLB_CFG_IRQ2_EN								(1<<20)
#define GLB_CFG_IRQ_EN                              (1<<19)

#define GLB_CFG_LOOPCNT_EN							(1<<18)
#define GLB_CFG_MAX_ISSUE_NUM_BIT2						(1<<17)
#define GLB_CFG_QDMA_LOOPBACK						(1<<16)
#define GLB_CFG_LPBK_RXQ_SEL_SHIFT					(8)
#define GLB_CFG_LPBK_RXQ_SEL_MASK					(0x1F<<GLB_CFG_LPBK_RXQ_SEL_SHIFT)

#define GLB_CFG_CHECK_DONE							(1<<7)
#define GLB_CFG_TX_WB_DONE							(1<<6)
#define GLB_CFG_MAX_ISSUE_NUM_SHIFT					(4)
#define GLB_CFG_MAX_ISSUE_NUM_MASK					(0x3<<GLB_CFG_MAX_ISSUE_NUM_SHIFT)
#define DMA_MAX_ISSUE_1							    (0x01)
#define DMA_MAX_ISSUE_2	    						(0x02)
#define DMA_MAX_ISSUE_3		    					(0x03)
#define GLB_CFG_RX_DMA_BUSY							(1<<3)
#define GLB_CFG_RX_DMA_EN							(1<<2)
#define GLB_CFG_TX_DMA_BUSY							(1<<1)
#define GLB_CFG_TX_DMA_EN							(1<<0)

/* QDMA_CSR_HWFWD_DSCP_BASE(base) */

/* QDMA_CSR_HWFWD_BUFF_BASE(base) */

/* QDMA_CSR_HWFWD_DSCP_CFG(base) */
#define HWFWD_PAYLOAD_SIZE_SHIFT					(28)
#define HWFWD_PAYLOAD_SIZE_MASK						(0x3<<HWFWD_PAYLOAD_SIZE_SHIFT)
#define HWFWD_PAYLOAD_SIZE_2K						(0x0)
#define HWFWD_PAYLOAD_SIZE_1K						(0x1)
#define HWFWD_PAYLOAD_SIZE_512						(0x2)
#define HWFWD_PAYLOAD_SIZE_256						(0x3)

/*for INT1~4 bind tx&rx ring info*/
#if defined(QDMA_LAN)
/*INT1*/
/*
  en7523:
  ring2 for icmp/icmpv6/igmp
  ring3 for ftp parse
  ring4 for vip
  ring7 for arp
  ring8 for other broadcast
*/
#define INT1_RX_RING_MASK					        (0x839F)        /*Rx_Ring_0/1/2/3/4/7/8/9/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0xFFFFFFFF)    /*7581 Tx_Ring_0~31 binded to INT1*/
/*INT2*/
#define INT2_RX_RING_MASK					        (0x7FE00000)    /*Rx_Ring_23-30 binded to INT2, for LRO; Ring_21 for fragment Ring_22 for Reassemble */ 
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x20)          /*Rx_Ring_5 for WIFI-2.4G */
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#if !defined(TCSUPPORT_NPU_WIFI_OFFLOAD)
#define INT4_RX_RING_MASK					        (0x40)          /*Rx_Ring_6 for WIFI-5G*/
#else
#define INT4_RX_RING_MASK					        (0x0)
#endif
#define INT4_TX_RING_MASK					        (0x0)
/*IRQ*/
#define IRQ_TX_RING_MASK							(0x40)			/* Tx_Ring-6 binded to IRQ2, for NPU, 0: IRQ1, 1: IRQ1*/
#else
/*INT1*/
/*ring2/3/4/7/8 is same as qdma_lan for en7523*/
#define INT1_RX_RING_MASK					        (0x839F)        /*Rx_Ring_0/1/2/3/4/7/8/9/15 binded to INT1*/
#define INT1_TX_RING_MASK					        (0xFFFFFFFF)    /*7581 Tx_Ring_0~31 binded to INT1*/

/*INT2*/
#define INT2_RX_RING_MASK					        (0x7F800400)    /*Rx_Ring_23-30 binded to INT2, for LRO; Ring_10 for forward left to right*/ 
#define INT2_TX_RING_MASK					        (0x0)
/*INT3*/
#define INT3_RX_RING_MASK					        (0x0)
#define INT3_TX_RING_MASK					        (0x0)
/*INT4*/
#define INT4_RX_RING_MASK					        (0x40)          /*WAN Rx_Ring_6 for multicast*/
#define INT4_TX_RING_MASK					        (0x0)
/*IRQ0*/
#define IRQ_TX_RING_MASK							(0x00)			/* default irq bind, 0: IRQ1, 1: IRQ1 */
#endif

/* QDMA_CSR_INT_STATUS1(base) */
#define INT_STATUS_RX15_COHERENT					(1<<31)
#define INT_STATUS_RX14_COHERENT					(1<<30)
#define INT_STATUS_RX13_COHERENT					(1<<29)
#define INT_STATUS_RX12_COHERENT					(1<<28)
#define INT_STATUS_RX11_COHERENT					(1<<27)
#define INT_STATUS_RX10_COHERENT					(1<<26)
#define INT_STATUS_RX9_COHERENT						(1<<25)
#define INT_STATUS_RX8_COHERENT						(1<<24)
#define INT_STATUS_RX7_COHERENT						(1<<23)
#define INT_STATUS_RX6_COHERENT						(1<<22)
#define INT_STATUS_RX5_COHERENT						(1<<21)
#define INT_STATUS_RX4_COHERENT						(1<<20)
#define INT_STATUS_RX3_COHERENT						(1<<19)
#define INT_STATUS_RX2_COHERENT						(1<<18)
#define INT_STATUS_RX1_COHERENT						(1<<17)
#define INT_STATUS_RX0_COHERENT						(1<<16)

#define INT_STATUS_TX7_COHERENT						(1<<15)
#define INT_STATUS_TX6_COHERENT						(1<<14)
#define INT_STATUS_TX5_COHERENT						(1<<13)
#define INT_STATUS_TX4_COHERENT						(1<<12)
#define INT_STATUS_TX3_COHERENT						(1<<11)
#define INT_STATUS_TX2_COHERENT						(1<<10)
#define INT_STATUS_TX1_COHERENT						(1<<9)
#define INT_STATUS_TX0_COHERENT						(1<<8)

#define INT_STATUS_CNT_OVER_FLOW					(1<<7)
#define INT_STATUS_IRQ2_FULL						(1<<5)
#define INT_STATUS_IRQ2_INT							(1<<4)
#define INT_STATUS_HWFWD_DSCP_LOW					(1<<3)
#define INT_STATUS_HWFWD_DSCP_EMPTY					(1<<2)
#define INT_STATUS_IRQ_FULL							(1<<1)
#define INT_STATUS_IRQ_INT							(1<<0)

#define INT_STATUS_RX_COHERENT						(0xFFFF0000)
#define INT_STATUS_TX_COHERENT						(0x0000FF00)
#define INT1_STATUS_RX_COHERENT						(int1_rx_ring<<16)
#define INT1_STATUS_TX_COHERENT						(INT1_TX_RING_MASK<<8)
#define INT2_STATUS_RX_COHERENT						(int2_rx_ring<<16)
#define INT2_STATUS_TX_COHERENT						(INT2_TX_RING_MASK<<8)
#define INT3_STATUS_RX_COHERENT						(int3_rx_ring<<16)
#define INT3_STATUS_TX_COHERENT						(INT3_TX_RING_MASK<<8)
#define INT4_STATUS_RX_COHERENT						(int4_rx_ring<<16)
#define INT4_STATUS_TX_COHERENT						(INT4_TX_RING_MASK<<8)
#define INT_STATUS_QDMA_FAULT						(0x0000008E)

#define INT1_STATUS_RX_COHERENT_LOW_16_RING			(int1_rx_ring<<16)
#define INT1_STATUS_RX_COHERENT_HIGH_16_RING		(int1_rx_ring&INT_STATUS_RX_COHERENT)
#define INT1_STATUS_TX_COHERENT_LOW_8_RING			((INT1_TX_RING_MASK&0xFF)<<8)
#define INT1_STATUS_TX_COHERENT_HIGH_24_RING		(INT1_TX_RING_MASK&0xFFFFFF00)
#define INT2_STATUS_RX_COHERENT_LOW_16_RING			(int2_rx_ring<<16)
#define INT2_STATUS_RX_COHERENT_HIGH_16_RING		(int2_rx_ring&INT_STATUS_RX_COHERENT)
#define INT2_STATUS_TX_COHERENT_LOW_8_RING			((INT2_TX_RING_MASK&0xFF)<<8)
#define INT2_STATUS_TX_COHERENT_HIGH_24_RING		(INT2_TX_RING_MASK&0xFFFFFF00)
#define INT3_STATUS_RX_COHERENT_LOW_16_RING			(int3_rx_ring<<16)
#define INT3_STATUS_RX_COHERENT_HIGH_16_RING		(int3_rx_ring&INT_STATUS_RX_COHERENT)
#define INT3_STATUS_TX_COHERENT_LOW_8_RING			((INT3_TX_RING_MASK&0xFF)<<8)
#define INT3_STATUS_TX_COHERENT_HIGH_24_RING		(INT3_TX_RING_MASK&0xFFFFFF00)
#define INT4_STATUS_RX_COHERENT_LOW_16_RING			(int4_rx_ring<<16)
#define INT4_STATUS_RX_COHERENT_HIGH_16_RING		(int4_rx_ring&INT_STATUS_RX_COHERENT)
#define INT4_STATUS_TX_COHERENT_LOW_8_RING			((INT4_TX_RING_MASK&0xFF)<<8)
#define INT4_STATUS_TX_COHERENT_HIGH_24_RING		(INT4_TX_RING_MASK&0xFFFFFF00)

/* QDMA_CSR_INT_STATUS2(base) */
#define INT_STATUS_NO_RX15_CPU_DSCP					(1<<31)
#define INT_STATUS_NO_RX14_CPU_DSCP					(1<<30)
#define INT_STATUS_NO_RX13_CPU_DSCP					(1<<29)
#define INT_STATUS_NO_RX12_CPU_DSCP					(1<<28)
#define INT_STATUS_NO_RX11_CPU_DSCP					(1<<27)
#define INT_STATUS_NO_RX10_CPU_DSCP					(1<<26)
#define INT_STATUS_NO_RX9_CPU_DSCP					(1<<25)
#define INT_STATUS_NO_RX8_CPU_DSCP					(1<<24)
#define INT_STATUS_NO_RX7_CPU_DSCP					(1<<23)
#define INT_STATUS_NO_RX6_CPU_DSCP					(1<<22)
#define INT_STATUS_NO_RX5_CPU_DSCP					(1<<21)
#define INT_STATUS_NO_RX4_CPU_DSCP					(1<<20)
#define INT_STATUS_NO_RX3_CPU_DSCP					(1<<19)
#define INT_STATUS_NO_RX2_CPU_DSCP					(1<<18)
#define INT_STATUS_NO_RX1_CPU_DSCP					(1<<17)
#define INT_STATUS_NO_RX0_CPU_DSCP					(1<<16)

#define INT_STATUS_RX15_DONE						(1<<15)
#define INT_STATUS_RX14_DONE						(1<<14)
#define INT_STATUS_RX13_DONE						(1<<13)
#define INT_STATUS_RX12_DONE						(1<<12)
#define INT_STATUS_RX11_DONE						(1<<11)
#define INT_STATUS_RX10_DONE						(1<<10)
#define INT_STATUS_RX9_DONE							(1<<9)
#define INT_STATUS_RX8_DONE							(1<<8)
#define INT_STATUS_RX7_DONE							(1<<7)
#define INT_STATUS_RX6_DONE							(1<<6)
#define INT_STATUS_RX5_DONE							(1<<5)
#define INT_STATUS_RX4_DONE							(1<<4)
#define INT_STATUS_RX3_DONE							(1<<3)
#define INT_STATUS_RX2_DONE							(1<<2)
#define INT_STATUS_RX1_DONE							(1<<1)
#define INT_STATUS_RX0_DONE							(1<<0)

/* QDMA_CSR_INT_STATUS3(base) */
#define INT_STATUS_NO_RX31_CPU_DSCP					(1<<31)
#define INT_STATUS_NO_RX30_CPU_DSCP					(1<<30)
#define INT_STATUS_NO_RX29_CPU_DSCP					(1<<29)
#define INT_STATUS_NO_RX28_CPU_DSCP					(1<<28)
#define INT_STATUS_NO_RX27_CPU_DSCP					(1<<27)
#define INT_STATUS_NO_RX26_CPU_DSCP					(1<<26)
#define INT_STATUS_NO_RX25_CPU_DSCP					(1<<25)
#define INT_STATUS_NO_RX24_CPU_DSCP					(1<<24)
#define INT_STATUS_NO_RX23_CPU_DSCP					(1<<23)
#define INT_STATUS_NO_RX22_CPU_DSCP					(1<<22)
#define INT_STATUS_NO_RX21_CPU_DSCP					(1<<21)
#define INT_STATUS_NO_RX20_CPU_DSCP					(1<<20)
#define INT_STATUS_NO_RX19_CPU_DSCP					(1<<19)
#define INT_STATUS_NO_RX18_CPU_DSCP					(1<<18)
#define INT_STATUS_NO_RX17_CPU_DSCP					(1<<17)
#define INT_STATUS_NO_RX16_CPU_DSCP					(1<<16)

#define INT_STATUS_RX31_DONE						(1<<15)
#define INT_STATUS_RX30_DONE						(1<<14)
#define INT_STATUS_RX29_DONE						(1<<13)
#define INT_STATUS_RX28_DONE						(1<<12)
#define INT_STATUS_RX27_DONE						(1<<11)
#define INT_STATUS_RX26_DONE						(1<<10)
#define INT_STATUS_RX25_DONE						(1<<9)
#define INT_STATUS_RX24_DONE						(1<<8)
#define INT_STATUS_RX23_DONE						(1<<7)
#define INT_STATUS_RX22_DONE						(1<<6)
#define INT_STATUS_RX21_DONE						(1<<5)
#define INT_STATUS_RX20_DONE						(1<<4)
#define INT_STATUS_RX19_DONE						(1<<3)
#define INT_STATUS_RX18_DONE						(1<<2)
#define INT_STATUS_RX17_DONE						(1<<1)
#define INT_STATUS_RX16_DONE						(1<<0)

/* QDMA_CSR_INT_STATUS4(base) */
#define INT_STATUS_RX31_COHERENT					(1<<31)
#define INT_STATUS_RX30_COHERENT					(1<<30)
#define INT_STATUS_RX29_COHERENT					(1<<29)
#define INT_STATUS_RX28_COHERENT					(1<<28)
#define INT_STATUS_RX27_COHERENT					(1<<27)
#define INT_STATUS_RX26_COHERENT					(1<<26)
#define INT_STATUS_RX25_COHERENT					(1<<25)
#define INT_STATUS_RX24_COHERENT					(1<<24)
#define INT_STATUS_RX23_COHERENT					(1<<23)
#define INT_STATUS_RX22_COHERENT					(1<<22)
#define INT_STATUS_RX21_COHERENT					(1<<21)
#define INT_STATUS_RX20_COHERENT					(1<<20)
#define INT_STATUS_RX19_COHERENT					(1<<19)
#define INT_STATUS_RX18_COHERENT					(1<<18)
#define INT_STATUS_RX17_COHERENT					(1<<17)
#define INT_STATUS_RX16_COHERENT					(1<<16)

/* QDMA_CSR_INT_STATUS5(base) */
#define INT_STATUS_TX31_COHERENT					(1<<31)
#define INT_STATUS_TX30_COHERENT					(1<<30)
#define INT_STATUS_TX29_COHERENT					(1<<29)
#define INT_STATUS_TX28_COHERENT					(1<<28)
#define INT_STATUS_TX27_COHERENT					(1<<27)
#define INT_STATUS_TX26_COHERENT					(1<<26)
#define INT_STATUS_TX25_COHERENT					(1<<25)
#define INT_STATUS_TX24_COHERENT					(1<<24)
#define INT_STATUS_TX23_COHERENT					(1<<23)
#define INT_STATUS_TX22_COHERENT					(1<<22)
#define INT_STATUS_TX21_COHERENT					(1<<21)
#define INT_STATUS_TX20_COHERENT					(1<<20)
#define INT_STATUS_TX19_COHERENT					(1<<19)
#define INT_STATUS_TX18_COHERENT					(1<<18)
#define INT_STATUS_TX17_COHERENT					(1<<17)
#define INT_STATUS_TX16_COHERENT					(1<<16)
#define INT_STATUS_TX15_COHERENT					(1<<15)
#define INT_STATUS_TX14_COHERENT					(1<<14)
#define INT_STATUS_TX13_COHERENT					(1<<13)
#define INT_STATUS_TX12_COHERENT					(1<<12)
#define INT_STATUS_TX11_COHERENT					(1<<11)
#define INT_STATUS_TX10_COHERENT					(1<<10)
#define INT_STATUS_TX9_COHERENT						(1<<9)
#define INT_STATUS_TX8_COHERENT						(1<<8)

#define INT_STATUS_NO_RX_CPU_DSCP					(0xFFFF0000)
#define INT_STATUS_RX_DONE							(0x0000FFFF)
#define INT1_STATUS_NO_RX_CPU_DSCP					(int1_rx_ring<<16)
#define INT1_STATUS_RX_DONE						    (int1_rx_ring)
#define INT2_STATUS_NO_RX_CPU_DSCP					(int2_rx_ring<<16)
#define INT2_STATUS_RX_DONE						    (int2_rx_ring)
#define INT3_STATUS_NO_RX_CPU_DSCP					(int3_rx_ring<<16)
#define INT3_STATUS_RX_DONE						    (int3_rx_ring)
#define INT4_STATUS_NO_RX_CPU_DSCP					(int4_rx_ring<<16)
#define INT4_STATUS_RX_DONE						    (int4_rx_ring)

#define INT1_STATUS_RX_DONE_LOW_16_RING				(int1_rx_ring&0x0000FFFF)
#define INT1_STATUS_RX_DONE_HIGH_16_RING			(int1_rx_ring>>16)
#define INT1_STATUS_NO_RX_CPU_DSCP_LOW_16_RING		(int1_rx_ring<<16)
#define INT1_STATUS_NO_RX_CPU_DSCP_HIGH_16_RING		(int1_rx_ring&0xFFFF0000)
#define INT2_STATUS_RX_DONE_LOW_16_RING				(int2_rx_ring&0x0000FFFF)
#define INT2_STATUS_RX_DONE_HIGH_16_RING			(int2_rx_ring>>16)
#define INT2_STATUS_NO_RX_CPU_DSCP_LOW_16_RING		(int2_rx_ring<<16)
#define INT2_STATUS_NO_RX_CPU_DSCP_HIGH_16_RING		(int2_rx_ring&0xFFFF0000)
#define INT3_STATUS_RX_DONE_LOW_16_RING				(int3_rx_ring&0x0000FFFF)
#define INT3_STATUS_RX_DONE_HIGH_16_RING			(int3_rx_ring>>16)
#define INT3_STATUS_NO_RX_CPU_DSCP_LOW_16_RING		(int3_rx_ring<<16)
#define INT3_STATUS_NO_RX_CPU_DSCP_HIGH_16_RING		(int3_rx_ring&0xFFFF0000)
#define INT4_STATUS_RX_DONE_LOW_16_RING				(int4_rx_ring&0x0000FFFF)
#define INT4_STATUS_RX_DONE_HIGH_16_RING			(int4_rx_ring>>16)
#define INT4_STATUS_NO_RX_CPU_DSCP_LOW_16_RING		(int4_rx_ring<<16)
#define INT4_STATUS_NO_RX_CPU_DSCP_HIGH_16_RING		(int4_rx_ring&0xFFFF0000)

/* QDMA_CSR_INT_ENABLE1(base) */
#define INT_MASK_RX15_COHERENT						(1<<31)
#define INT_MASK_RX14_COHERENT						(1<<30)
#define INT_MASK_RX13_COHERENT						(1<<29)
#define INT_MASK_RX12_COHERENT						(1<<28)
#define INT_MASK_RX11_COHERENT						(1<<27)
#define INT_MASK_RX10_COHERENT						(1<<26)
#define INT_MASK_RX9_COHERENT						(1<<25)
#define INT_MASK_RX8_COHERENT						(1<<24)
#define INT_MASK_RX7_COHERENT						(1<<23)
#define INT_MASK_RX6_COHERENT						(1<<22)
#define INT_MASK_RX5_COHERENT						(1<<21)
#define INT_MASK_RX4_COHERENT						(1<<20)
#define INT_MASK_RX3_COHERENT						(1<<19)
#define INT_MASK_RX2_COHERENT						(1<<18)
#define INT_MASK_RX1_COHERENT						(1<<17)
#define INT_MASK_RX0_COHERENT						(1<<16)

#define INT_MASK_TX7_COHERENT						(1<<15)
#define INT_MASK_TX6_COHERENT						(1<<14)
#define INT_MASK_TX5_COHERENT						(1<<13)
#define INT_MASK_TX4_COHERENT						(1<<12)
#define INT_MASK_TX3_COHERENT						(1<<11)
#define INT_MASK_TX2_COHERENT						(1<<10)
#define INT_MASK_TX1_COHERENT						(1<<9)
#define INT_MASK_TX0_COHERENT						(1<<8)

#define INT_MASK_CNT_OVER_FLOW                      (1<<7)
#define INT_MASK_IRQ2_FULL							(1<<5)
#define INT_MASK_IRQ2_INT							(1<<4)
#define INT_MASK_HWFWD_DSCP_LOW						(1<<3)
#define INT_MASK_HWFWD_DSCP_EMPTY					(1<<2)
#define INT_MASK_IRQ_FULL							(1<<1)
#define INT_MASK_IRQ_INT							(1<<0)

#define INT_MASK_RX_COHERENT						(0xFFFF0000)
#define INT_MASK_TX_COHERENT						(0x0000FF00)
#define INT1_MASK_RX_COHERENT						(int1_rx_ring<<16)
#define INT1_MASK_TX_COHERENT						(INT1_TX_RING_MASK<<8)
#define INT2_MASK_RX_COHERENT						(int2_rx_ring<<16)
#define INT2_MASK_TX_COHERENT						(INT2_TX_RING_MASK<<8)
#define INT3_MASK_RX_COHERENT						(int3_rx_ring<<16)
#define INT3_MASK_TX_COHERENT						(INT3_TX_RING_MASK<<8)
#define INT4_MASK_RX_COHERENT						(int4_rx_ring<<16)
#define INT4_MASK_TX_COHERENT						(INT4_TX_RING_MASK<<8)

#define INT_MASK_TX_COHERENT_HIGH_24_RING			(0xFFFFFF00)
#define INT_MASK_RX_COHERENT_HIGH_16_RING			(0xFFFF0000)
#define INT1_MASK_RX_COHERENT_LOW_16_RING			(int1_rx_ring<<16)
#define INT1_MASK_RX_COHERENT_HIGH_16_RING			(int1_rx_ring&0xFFFF0000)
#define INT1_MASK_TX_COHERENT_LOW_8_RING			((INT1_TX_RING_MASK&0xFF)<<8)
#define INT1_MASK_TX_COHERENT_HIGH_24_RING			(INT1_TX_RING_MASK&0xFFFFFF00)
#define INT2_MASK_RX_COHERENT_LOW_16_RING			(int2_rx_ring<<16)
#define INT2_MASK_RX_COHERENT_HIGH_16_RING			(int2_rx_ring&0xFFFF0000)
#define INT2_MASK_TX_COHERENT_LOW_8_RING			((INT2_TX_RING_MASK&0xFF)<<8)
#define INT2_MASK_TX_COHERENT_HIGH_24_RING			(INT2_TX_RING_MASK&0xFFFFFF00)
#define INT3_MASK_RX_COHERENT_LOW_16_RING			(int3_rx_ring<<16)
#define INT3_MASK_RX_COHERENT_HIGH_16_RING			(int3_rx_ring&0xFFFF0000)
#define INT3_MASK_TX_COHERENT_LOW_8_RING			((INT3_TX_RING_MASK&0xFF)<<8)
#define INT3_MASK_TX_COHERENT_HIGH_24_RING			(INT3_TX_RING_MASK&0xFFFFFF00)
#define INT4_MASK_RX_COHERENT_LOW_16_RING			(int4_rx_ring<<16)
#define INT4_MASK_RX_COHERENT_HIGH_16_RING			(int4_rx_ring&0xFFFF0000)
#define INT4_MASK_TX_COHERENT_LOW_8_RING			((INT4_TX_RING_MASK&0xFF)<<8)
#define INT4_MASK_TX_COHERENT_HIGH_24_RING			(INT4_TX_RING_MASK&0xFFFFFF00)

/* QDMA_CSR_INT_ENABLE2(base) */
#define INT_MASK_NO_RX15_CPU_DSCP					(1<<31)
#define INT_MASK_NO_RX14_CPU_DSCP					(1<<30)
#define INT_MASK_NO_RX13_CPU_DSCP					(1<<29)
#define INT_MASK_NO_RX12_CPU_DSCP					(1<<28)
#define INT_MASK_NO_RX11_CPU_DSCP					(1<<27)
#define INT_MASK_NO_RX10_CPU_DSCP					(1<<26)
#define INT_MASK_NO_RX9_CPU_DSCP					(1<<25)
#define INT_MASK_NO_RX8_CPU_DSCP					(1<<24)
#define INT_MASK_NO_RX7_CPU_DSCP					(1<<23)
#define INT_MASK_NO_RX6_CPU_DSCP					(1<<22)
#define INT_MASK_NO_RX5_CPU_DSCP					(1<<21)
#define INT_MASK_NO_RX4_CPU_DSCP					(1<<20)
#define INT_MASK_NO_RX3_CPU_DSCP					(1<<19)
#define INT_MASK_NO_RX2_CPU_DSCP					(1<<18)
#define INT_MASK_NO_RX1_CPU_DSCP					(1<<17)
#define INT_MASK_NO_RX0_CPU_DSCP					(1<<16)
#define INT_MASK_RX15_DONE							(1<<15)
#define INT_MASK_RX14_DONE							(1<<14)
#define INT_MASK_RX13_DONE							(1<<13)
#define INT_MASK_RX12_DONE							(1<<12)
#define INT_MASK_RX11_DONE							(1<<11)
#define INT_MASK_RX10_DONE							(1<<10)
#define INT_MASK_RX9_DONE							(1<<9)
#define INT_MASK_RX8_DONE							(1<<8)
#define INT_MASK_RX7_DONE							(1<<7)
#define INT_MASK_RX6_DONE							(1<<6)
#define INT_MASK_RX5_DONE							(1<<5)
#define INT_MASK_RX4_DONE							(1<<4)
#define INT_MASK_RX3_DONE							(1<<3)
#define INT_MASK_RX2_DONE							(1<<2)
#define INT_MASK_RX1_DONE							(1<<1)
#define INT_MASK_RX0_DONE							(1<<0)

/* QDMA_CSR_INT_ENABLE3(base) */
#define INT_MASK_NO_RX31_CPU_DSCP					(1<<31)
#define INT_MASK_NO_RX30_CPU_DSCP					(1<<30)
#define INT_MASK_NO_RX29_CPU_DSCP					(1<<29)
#define INT_MASK_NO_RX28_CPU_DSCP					(1<<28)
#define INT_MASK_NO_RX27_CPU_DSCP					(1<<27)
#define INT_MASK_NO_RX26_CPU_DSCP					(1<<26)
#define INT_MASK_NO_RX25_CPU_DSCP					(1<<25)
#define INT_MASK_NO_RX24_CPU_DSCP					(1<<24)
#define INT_MASK_NO_RX23_CPU_DSCP					(1<<23)
#define INT_MASK_NO_RX22_CPU_DSCP					(1<<22)
#define INT_MASK_NO_RX21_CPU_DSCP					(1<<21)
#define INT_MASK_NO_RX20_CPU_DSCP					(1<<20)
#define INT_MASK_NO_RX19_CPU_DSCP					(1<<19)
#define INT_MASK_NO_RX18_CPU_DSCP					(1<<18)
#define INT_MASK_NO_RX17_CPU_DSCP					(1<<17)
#define INT_MASK_NO_RX16_CPU_DSCP					(1<<16)
#define INT_MASK_RX31_DONE							(1<<15)
#define INT_MASK_RX30_DONE							(1<<14)
#define INT_MASK_RX29_DONE							(1<<13)
#define INT_MASK_RX28_DONE							(1<<12)
#define INT_MASK_RX27_DONE							(1<<11)
#define INT_MASK_RX26_DONE							(1<<10)
#define INT_MASK_RX25_DONE							(1<<9)
#define INT_MASK_RX24_DONE							(1<<8)
#define INT_MASK_RX23_DONE							(1<<7)
#define INT_MASK_RX22_DONE							(1<<6)
#define INT_MASK_RX21_DONE							(1<<5)
#define INT_MASK_RX20_DONE							(1<<4)
#define INT_MASK_RX19_DONE							(1<<3)
#define INT_MASK_RX18_DONE							(1<<2)
#define INT_MASK_RX17_DONE							(1<<1)
#define INT_MASK_RX16_DONE							(1<<0)

/* QDMA_CSR_INT_ENABLE4(base) */
#define INT_MASK_RX31_COHERENT						(1<<31)
#define INT_MASK_RX30_COHERENT						(1<<30)
#define INT_MASK_RX29_COHERENT						(1<<29)
#define INT_MASK_RX28_COHERENT						(1<<28)
#define INT_MASK_RX27_COHERENT						(1<<27)
#define INT_MASK_RX26_COHERENT						(1<<26)
#define INT_MASK_RX25_COHERENT						(1<<25)
#define INT_MASK_RX24_COHERENT						(1<<24)
#define INT_MASK_RX23_COHERENT						(1<<23)
#define INT_MASK_RX22_COHERENT						(1<<22)
#define INT_MASK_RX21_COHERENT						(1<<21)
#define INT_MASK_RX20_COHERENT						(1<<20)
#define INT_MASK_RX19_COHERENT						(1<<19)
#define INT_MASK_RX18_COHERENT						(1<<18)
#define INT_MASK_RX17_COHERENT						(1<<17)
#define INT_MASK_RX16_COHERENT						(1<<16)

/* QDMA_CSR_INT_ENABLE5(base) */
#define INT_MASK_TX31_COHERENT						(1<<31)
#define INT_MASK_TX30_COHERENT						(1<<30)
#define INT_MASK_TX29_COHERENT						(1<<29)
#define INT_MASK_TX28_COHERENT						(1<<28)
#define INT_MASK_TX27_COHERENT						(1<<27)
#define INT_MASK_TX26_COHERENT						(1<<26)
#define INT_MASK_TX25_COHERENT						(1<<25)
#define INT_MASK_TX24_COHERENT						(1<<24)
#define INT_MASK_TX23_COHERENT						(1<<23)
#define INT_MASK_TX22_COHERENT						(1<<22)
#define INT_MASK_TX21_COHERENT						(1<<21)
#define INT_MASK_TX20_COHERENT						(1<<20)
#define INT_MASK_TX19_COHERENT						(1<<19)
#define INT_MASK_TX18_COHERENT						(1<<18)
#define INT_MASK_TX17_COHERENT						(1<<17)
#define INT_MASK_TX16_COHERENT						(1<<16)
#define INT_MASK_TX15_COHERENT						(1<<15)
#define INT_MASK_TX14_COHERENT						(1<<14)
#define INT_MASK_TX13_COHERENT						(1<<13)
#define INT_MASK_TX12_COHERENT						(1<<12)
#define INT_MASK_TX11_COHERENT						(1<<11)
#define INT_MASK_TX10_COHERENT						(1<<10)
#define INT_MASK_TX9_COHERENT						(1<<9)
#define INT_MASK_TX8_COHERENT						(1<<8)

#define INT_MASK_NO_RX_CPU_DSCP						(0xFFFF0000)
#define INT_MASK_RX_DONE							(0x0000FFFF)
#define INT1_MASK_NO_RX_CPU_DSCP					(int1_rx_ring<<16)
#define INT1_MASK_RX_DONE							(int1_rx_ring)
#define INT2_MASK_NO_RX_CPU_DSCP					(int2_rx_ring<<16)
#define INT2_MASK_RX_DONE							(int2_rx_ring)
#define INT3_MASK_NO_RX_CPU_DSCP					(int3_rx_ring<<16)
#define INT3_MASK_RX_DONE							(int3_rx_ring)
#define INT4_MASK_NO_RX_CPU_DSCP					(int4_rx_ring<<16)
#define INT4_MASK_RX_DONE							(int4_rx_ring)

#define INT1_MASK_RX_DONE_LOW_16_RING				(int1_rx_ring&0x0000FFFF)
#define INT1_MASK_RX_DONE_HIGH_16_RING				(int1_rx_ring>>16)
#define INT1_MASK_NO_RX_CPU_DSCP_LOW_16_RING		(int1_rx_ring<<16)
#define INT1_MASK_NO_RX_CPU_DSCP_HIGH_16_RING		(int1_rx_ring&0xFFFF0000)
#define INT2_MASK_RX_DONE_LOW_16_RING				(int2_rx_ring&0x0000FFFF)
#define INT2_MASK_RX_DONE_HIGH_16_RING				(int2_rx_ring>>16)
#define INT2_MASK_NO_RX_CPU_DSCP_LOW_16_RING		(int2_rx_ring<<16)
#define INT2_MASK_NO_RX_CPU_DSCP_HIGH_16_RING		(int2_rx_ring&0xFFFF0000)
#define INT3_MASK_RX_DONE_LOW_16_RING				(int3_rx_ring&0x0000FFFF)
#define INT3_MASK_RX_DONE_HIGH_16_RING				(int3_rx_ring>>16)
#define INT3_MASK_NO_RX_CPU_DSCP_LOW_16_RING		(int3_rx_ring<<16)
#define INT3_MASK_NO_RX_CPU_DSCP_HIGH_16_RING		(int3_rx_ring&0xFFFF0000)
#define INT4_MASK_RX_DONE_LOW_16_RING				(int4_rx_ring&0x0000FFFF)
#define INT4_MASK_RX_DONE_HIGH_16_RING				(int4_rx_ring>>16)
#define INT4_MASK_NO_RX_CPU_DSCP_LOW_16_RING		(int4_rx_ring<<16)
#define INT4_MASK_NO_RX_CPU_DSCP_HIGH_16_RING		(int4_rx_ring&0xFFFF0000)

/* QDMA_CSR_IRQ_BASE(base) */

/* QDMA_CSR_IRQ_CFG(base) */
#define IRQ_CFG_THRESHOLD_SHIFT						(16)
#define IRQ_CFG_THRESHOLD_MASK						(0xFFF<<IRQ_CFG_THRESHOLD_SHIFT)
#define IRQ_CFG_DEPTH_SHIFT							(0)
#define IRQ_CFG_DEPTH_MASK							(0xFFF<<IRQ_CFG_DEPTH_SHIFT)

/*IRQ Queue Structure*/
#define IRQ_CFG_RINGIDX_SHIFT						(16)
#define IRQ_CFG_RINGIDX_MASK						(0x1F<<IRQ_CFG_RINGIDX_SHIFT) /*32 tx rings*/
#define IRQ_CFG_IDX_MASK							0xFFFF						 /*64K tx dscp*/

/* QDMA_CSR_IRQ_CLEAR_LEN(base) */
#define IRQ_CLEAR_LEN_SHIFT							(0)
#define IRQ_CLEAR_LEN_MASK							(0xFF<<IRQ_CLEAR_LEN_SHIFT)

/* QDMA_CSR_IRQ_STATUS(base) */
#define IRQ_STATUS_ENTRY_LEN_SHIFT					(16)
#define IRQ_STATUS_ENTRY_LEN_MASK					(0xFFF<<IRQ_STATUS_ENTRY_LEN_SHIFT)
#define IRQ_STATUS_HEAD_IDX_SHIFT					(0)
#define IRQ_STATUS_HEAD_IDX_MASK					(0xFFF<<IRQ_STATUS_HEAD_IDX_SHIFT)
                                               	
/* QDMA_CSR_IRQ_PTIME(base) */
#define IRQ_PTIME_SHIFT								(0)
#define IRQ_PTIME_MASK								(0xFFFF<<IRQ_PTIME_SHIFT)


/* QDMA_CSR_TX_DSCP_BASE(base) */

/* QDMA_CSR_TX_BLOCKING(base) */
#define CPU_TX_RING_IRQ_CFG_SHIFT					(4)
#define CPU_TX_RING_IRQ_CFG_MASK					(1<<4)
#define CPU_TX_RING_IRQ_CFG							(1<<4)	/*added for EN7581 & EN7523*/
#define TXQ_DYN_CNGSTCTL_CPU_TX_DROP_EN				(1<<2)	/*added for EN7580*/
#define TXQ_DYN_CNGSTCTL_MAX_THRH_TXRING_EN			(1<<1)
#define TXQ_DYN_CNGSTCTL_MIN_THRH_TXRING_EN			(1<<0)

/* QDMA_CSR_TX_CPU_IDX(base) */
#define TX_CPU_IDX_SHIFT							(0)
#define TX_CPU_IDX_MASK								(0xFFFF<<TX_CPU_IDX_SHIFT)

/* QDMA_CSR_TX_DMA_IDX(base) */
#define TX_DMA_IDX_SHIFT							(0)
#define TX_DMA_IDX_MASK								(0xFFFF<<TX_DMA_IDX_SHIFT)

/* QDMA_CSR_RX_DSCP_BASE(base) */

/* QDMA_CSR_RX_RING_CFG */
#define RX_RING_SIZE_SHIFT							(0)
#define RX_RING_SIZE_MASK							(0x3FFFF<<RX_RING_SIZE_SHIFT)
#define RX_RING_LOW_THR_SHIFT						(18)
#define RX_RING_LOW_THR_MASK						(0x1FFF<<RX_RING_LOW_THR_SHIFT)

/* QDMA_CSR_RX_CPU_IDX(base) */
#define RX_CPU_IDX_SHIFT							(0)
#define RX_CPU_IDX_MASK								(0x3FFFF<<RX_CPU_IDX_SHIFT) 

/* QDMA_CSR_RX_DMA_IDX(base) */
#define RX_DMA_IDX_SHIFT							(0)
#define RX_DMA_IDX_MASK								(0x3FFFF<<RX_DMA_IDX_SHIFT) 

/* QDMA_CSR_RX_DELAY_INT_CFG(base) */ 
#define DLY_INT_RXDLY_SHIFT							(0)
#define DLY_INT_RXDLY_MASK							(0xFFFF<<DLY_INT_RXDLY_SHIFT)
#define DLY_INT_RXMAX_PINT_SHIFT					(8)
#define DLY_INT_RXMAX_PINT_MASK						(0xFF<<DLY_INT_RXMAX_PINT_SHIFT)
#define DLY_INT_RXMAX_PTIME_SHIFT					(0)
#define DLY_INT_RXMAX_PTIME_MASK					(0xFF<<DLY_INT_RXMAX_PTIME_SHIFT)

/* QDMA_CSR_INGRESS_TRTCM_CFG(base) */
/* QDMA_CSR_EGRESS_TRTCM_CFG(base) */
#define TRTCM_CFG(trtcm_base)				(trtcm_base)
#define TRTCM_PARAM_CFG(trtcm_base)			(trtcm_base+0x4)
#define TRTCM_DATA_LO(trtcm_base)			(trtcm_base+0x8)
#define TRTCM_DATA_HI(trtcm_base)			(trtcm_base+0xc)

#define TRTCM_EN_SHIFT						(31)
#define TRTCM_EN_MASK						(1<<TRTCM_EN_SHIFT)
#define TRTCM_MODE_SHIFT					(30)
#define TRTCM_MODE_MASK						(1<<TRTCM_MODE_SHIFT)
#define TRTCM_SLOW_TICKRATIO_SHIFT			(16)
#define TRTCM_SLOW_TICKRATIO_MASK			(0x3FFF<<TRTCM_SLOW_TICKRATIO_SHIFT)
#define TRTCM_FAST_TICK_SHIFT				(0)
#define TRTCM_FAST_TICK_MASK				(0xFFFF<<TRTCM_FAST_TICK_SHIFT)

#define TRTCM_METER_MODE					(1<<2)
#define TRTCM_PKT_MODE						(1<<1)
#define TRTCM_TICK_SEL						(1<<0)

#define TRTCM_TOKEN_RATE_INTEGER_SHIFT		(6)
#define TRTCM_TOKEN_RATE_INTEGER_MASK		(0x3FFFF<<TRTCM_TOKEN_RATE_INTEGER_SHIFT)
#define TRTCM_TOKEN_RATE_FRACTION_MASK		(0x3F)

#define TRTCM_BUCKET_SIZE_SHIFT_MASK		(0x1F)

/*30-bit integer with 6-bit fraction*/
#define TRTCM_BUCKET_COUNTER_FRACTION_MASK  (0x3F)
#define TRTCM_BUCKET_COUNTER_INTEGER_SHIFT	(6)
#define TRTCM_BUCKET_COUNTER_LO_MASK		(0x3FFFFFF<<TRTCM_BUCKET_COUNTER_INTEGER_SHIFT)
#define TRTCM_BUCKET_COUNTER_HI_MASK		(0x7)
#define TRTCM_BUCKET_COUNTER_SIGN_BIT_HI_MASK		(0x1<<3)
#define TRTCM_BUCKET_COUNTER_INTEGER_FIN_SHIFT		(26)


/* QDMA_CSR_INGRESS_TRTCM_PARAM_CFG(base) */
/* QDMA_CSR_EGRESS_TRTCM_PARAM_CFG(base) */
#define TRTCM_PARA_RWCMD				(1<<31)
#define TRTCM_PARA_RWCMD_DONE			(1<<30)
#define TRTCM_PARA_TYPE_SHIFT			(28)
#define TRTCM_PARA_TYPE_MASK			(0x3<<TRTCM_PARA_TYPE_SHIFT)
#define TRTCM_PARA_METER_GROUP_SHIFT    (26)
#define TRTCM_PARA_IDX_RATE_TYPE_SHIFT	(16)
#define TRTCM_PARA_IDX_RATE_TYPE_MASK	(1<<TRTCM_PARA_IDX_RATE_TYPE_SHIFT)  //pir&pbs or cir&cbs
#define TRTCM_PARA_IDX_INDEX_SHIFT		(17)
#define TRTCM_PARA_IDX_INDEX_MASK		(0x3F<<TRTCM_PARA_IDX_INDEX_SHIFT)	 //7580

#define RATELIMIT_PARA_RWCMD				(1<<31)
#define RATELIMIT_PARA_RWCMD_DONE			(1<<30)
#define RATELIMIT_PARA_TYPE_SHIFT			(28)
#define RATELIMIT_PARA_TYPE_MASK			(0x3<<RATELIMIT_PARA_TYPE_SHIFT)
#define RATELIMIT_PARA_IDX_INDEX_SHIFT		(16)
#define RATELIMIT_PARA_IDX_INDEX_MASK		(0xFF<<RATELIMIT_PARA_IDX_INDEX_SHIFT)	 //7581

/*QDMA_CSR_INGRESS_WRED_NORM/DEI_PROBABILITY*/
#define WRED_PROBABILITY_SHIFT(thrIdx)         (thrIdx<<3)
#define WRED_PROBABILITY_MASK(thrIdx)          (0xFF<<WRED_PROBABILITY_SHIFT(thrIdx))

/*QDNA_CSR_CPU_RX_RED_CFG*/
#define CPU_RX_RED_NORM_SHIFT             (0)
#define CPU_RX_RED_DEI_SHIFT              (8)
#define CPU_RX_RED_NORM_PROBABILITY_MASK      (0xFF<<CPU_RX_RED_NORM_SHIFT)
#define CPU_RX_RED_DEI_PROBABILITY_MASK       (0xFF<<CPU_RX_RED_DEI_SHIFT)

/*QDMA_CSR_HQOS_MODE_CFG*/
#define HQOS_MODE_EN_SHIFT					(31)
#define HQOS_MODE_EN						(0x1<<HQOS_MODE_EN_SHIFT)

/*QDMA_CSR_TXQ_DIS_CFG_CHN*/
/*offset 0x00:CHN0~3, 0x04:CHN4~7, 0x08:CHN8~11, 0x0c:CHN12~15*/
/*offset 0x10:CHN16~19, 0x14:CHN20~23, 0x18:CHN24~27, 0x1c:CHN28~31*/
#define TXQ_DIS_QUEUE_CLOSE_OFFSET(chnl)                    ((chnl)&0xFC)
#define TXQ_DIS_CHANNEL_QUEUE_OFFSET(chnl,queue)            (1<<((queue)+(((chnl)&0x03)*8)))
#define TXQ_DIS_CHANNEL_SHIFT(chnl)                         (((chnl)&0x03)*8)
#define TXQ_DIS_CHANNEL_MASK(chnl)                          (0xFF<<TXQ_DIS_CHANNEL_SHIFT(chnl))

/*********** 0x800 *************/

/* QDMA_CSR_LMGR_VCHNL_CFG(base) */
#define VCHNL_MAP_EN								(1<<1)
#define VCHNL_MAP_MODE								(1<<0)

/* QDMA_CSR_HWFWD_LOW_THRH(base) */
#define HWFWD_DSCP_LOW_THRSHLD_SHIFT				(0)
#define HWFWD_DSCP_LOW_THRSHLD_MASK					(0x3FFF<<HWFWD_DSCP_LOW_THRSHLD_SHIFT)

/* QDMA_CSR_TXQ_MIN_DSCP_THRH(base) */
#define TXC_MIN_DSCP_THRSHLD_SHIFT					(16)
#define TXC_MIN_DSCP_THRSHLD_MASK					(0xFFFF<<TXC_MIN_DSCP_THRSHLD_SHIFT)
#define TXQ_MIN_DSCP_THRSHLD_SHIFT					(0)
#define TXQ_MIN_DSCP_THRSHLD_MASK					(0xFFFF<<TXQ_MIN_DSCP_THRSHLD_SHIFT)

/* QDMA_CSR_LMGR_INIT_CFG(base) */
#define LMGR_INIT_START								(1<<31)
#define LMGR_DSCP_RAM_MODE_SHIFT					(30)
#define LMGR_DSCP_RAM_MODE_MASK						(1<<LMGR_DSCP_RAM_MODE_SHIFT)	/*0:DRAM 64K dscp; 1:SRAM 16K dscp*/
#define HWFWD_PKTSIZE_OVERHEAD_EN					(1<<28)
#define HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT			(20)
#define HWFWD_PKTSIZE_OVERHEAD_VALUE_MASK			(0xFF<<HWFWD_PKTSIZE_OVERHEAD_VALUE_SHIFT)
#define HWFWD_DSCP_NUM_SHIFT						(0)
#define HWFWD_DSCP_NUM_MASK							(0x1FFFF<<HWFWD_DSCP_NUM_SHIFT)

/* QDMA_CSR_TXWRR_MODE_CFG */
#define TXWRR_WEIGHT_SCALE							(1<<31)
#define TXWRR_WEIGHT_BASE							(1<<3)

/* QDMA_CSR_TXWRR_WEIGHT_CFG */
#define TXWRR_RWCMD									(1<<31)
#define TXWRR_RWCMD_DONE							(1<<30)
#define TXWRR_CHNL_IDX_SHIFT						(19)
#define TXWRR_CHNL_IDX_MASK							(0x1F<<TXWRR_CHNL_IDX_SHIFT)
#define TXWRR_QUEUE_IDX_SHIFT						(16)
#define TXWRR_QUEUE_IDX_MASK						(0x7<<TXWRR_QUEUE_IDX_SHIFT)
#define TXWRR_WRR_VALUE_SHIFT						(0)
#define TXWRR_WRR_VALUE_MASK						(0xFFFF<<TXWRR_WRR_VALUE_SHIFT)

/* QDMA_CSR_PSE_BUF_USAGE_CFG */
#define PSE_BUF_ESTIMATE_EN							(1<<29)
#if 0
#define PSE_BUF_CTRL_EN								(1<<31)
#define PSE_BUF_PREFETCH_EN							(1<<30)
#define PSE_BUF_ESTIMATE_EN							(1<<29)
#define TXBUF_CHNL_THRSHLD_SHIFT					(16)
#define TXBUF_CHNL_THRSHLD_MASK						(0xFF<<TXBUF_CHNL_THRSHLD_SHIFT)
#define TXBUF_TOTAL_THRSHLD_SHIFT					(0)
#define TXBUF_TOTAL_THRSHLD_MASK					(0xFF<<TXBUF_TOTAL_THRSHLD_SHIFT)
#endif


/* QDMA_CSR_EGRESS_RATEMETER_CFG */
#define EGRESS_RATEMETER_EN							(1<<31)
#define EGRESS_RATEMETER_PEEKRATE_EN				(1<<30)
#define EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT	(18)
#define EGRESS_RATEMETER_PEEKRATE_DURATION_MASK		(0xFFF<<EGRESS_RATEMETER_PEEKRATE_DURATION_SHIFT)
#define EGRESS_RATEMETER_EQUALRATE_EN				(1<<17)
#define EGRESS_RATEMETER_WINDSZ_SHIFT				(12)
#define EGRESS_RATEMETER_WINDSZ_MASK				(0x1F<<EGRESS_RATEMETER_WINDSZ_SHIFT)
#define EGRESS_RATEMETER_TIMESLICE_SHIFT			(0)
#define EGRESS_RATEMETER_TIMESLICE_MASK				(0x7FF<<EGRESS_RATEMETER_TIMESLICE_SHIFT)

#if 0
/* QDMA_CSR_EGRESS_RATELIMIT_CFG */
#define EGRESS_RATELIMIT_EN							(1<<31)
#define EGRESS_RATELIMIT_BUCKETSCALE_SHIFT			(16)
#define EGRESS_RATELIMIT_BUCKETSCALE_MASK			(0xF<<EGRESS_RATELIMIT_BUCKETSCALE_SHIFT)
#define EGRESS_RATELIMIT_TICKSEL1_SHIFT				(0)
#define EGRESS_RATELIMIT_TICKSEL1_MASK				(0xFFFF<<EGRESS_RATELIMIT_TICKSEL1_SHIFT)

/* QDMA_CSR_EGRESS_RATELIMIT_CFG2 */
#define EGRESS_RATELIMIT_TICKSEL0_SHIFT				(0)
#define EGRESS_RATELIMIT_TICKSEL0_MASK				(0xFFFF<<EGRESS_RATELIMIT_TICKSEL0_SHIFT)

/* QDMA_CSR_EGRESS_RATELIMIT_TICK */
#define TX_CHNL_RATELIMIT_TICKSEL(idx)				(1<<idx)

/* QDMA_CSR_RATELIMIT_EN_CFG */
#define TX_CHNL_RATELIMIT_EN(idx)					(1<<idx)

/* QDMA_CSR_RATELIMIT_PARAMETER_CFG	 */
#define EGRESS_RATELIMIT_PARA_RWCMD					(1<<31)
#define EGRESS_RATELIMIT_PARA_RWCMD_DONE			(1<<30)
#define EGRESS_RATELIMIT_PARA_TYPE_SHIFT			(28)
#define EGRESS_RATELIMIT_PARA_TYPE_MASK				(0x3<<EGRESS_RATELIMIT_PARA_TYPE_SHIFT)
#define EGRESS_RATELIMIT_PARA_IDX_SHIFT				(16)
#define EGRESS_RATELIMIT_PARA_IDX_MASK				(0x1F<<EGRESS_RATELIMIT_PARA_IDX_SHIFT)
#define EGRESS_RATELIMIT_PARA_VALUE_SHIFT			(0)
#define EGRESS_RATELIMIT_PARA_VALUE_MASK			(0xFFFF<<TXWRR_WRR_VALUE_SHIFT)
#endif

/*QDMA_CSR_PSE_BUF_USAGE_CFG*/
#define PSE_BUF_FLOW_CTRL_SHIFT						(31)
#define PSE_BUF_FLOW_CTRL_MASK						(0x1<<PSE_BUF_FLOW_CTRL_SHIFT)
#define PSE_BUF_ESTIMATE_EN_SHIFT					(29)
#define PSE_BUF_ESTIMATE_EN_MASK					(0x1<<PSE_BUF_ESTIMATE_EN_SHIFT)
#define PSE_BUF_FAST_PATH_ESTIMATE_SHIFT			(28)
#define PSE_BUF_FAST_PATH_ESTIMATE_MASK				(0x1<<PSE_BUF_FAST_PATH_ESTIMATE_SHIFT)
#define PSE_BUF_ESTIMATE_PKTOVH_SHIFT				(0)
#define PSE_BUF_ESTIMATE_PKTOVH_MASK				(0xFF<<PSE_BUF_ESTIMATE_PKTOVH_SHIFT)

/*QDMA_CSR_PSE_BUF_USAGE_CFG1*/
#define PSE_BUF_CHNL_THRSHLD_SHIFT					(16)
#define PSE_BUF_CHNL_THRSHLD_MASK					(0x7FF<<PSE_BUF_CHNL_THRSHLD_SHIFT)
#define PSE_BUF_TOTAL_THRSHLD_SHIFT					(0)
#define PSE_BUF_TOTAL_THRSHLD_MASK					(0x7FF<<PSE_BUF_TOTAL_THRSHLD_SHIFT)

/*QDMA_CSR_CNGST_WRED_NORM_CFG*/
#define WRED_THR_SHIFT_SHIFT(thrIdx)                (thrIdx*5+3)
#define WRED_THR_SHIFT_MASK(thrIdx)                 (0x3<< WRED_THR_SHIFT_SHIFT(thrIdx))
#define WRED_THR_CFG_SHIFT(thrIdx)                  (thrIdx*5)
#define WRED_THR_CFG_MASK(thrIdx)                   (0x7<<WRED_THR_CFG_SHIFT(thrIdx) )

#define WRED_THR_SHIFT(thrIdx)                      (thrIdx*5)
#define WRED_THR_MASK(thrIdx)                       (0x1F<<WRED_THR_SHIFT(thrIdx) )
/* QDMA_CSR_TXQ_MIN_DSCPTHR_CFG */
#define TXQ_MIN_DSCPTHR_SHIFT				(0)
#define TXQ_MIN_DSCPTHR_MASK				(0x3FFF<<TXQ_MIN_DSCPTHR_SHIFT)

/* QDMA_CSR_TXQ_CNGST_CFG	 */
#define TXQ_CNGST_DROP_EN							(1<<31)
#define TXQ_CNGST_DEI_DROP_EN						(1<<30)
#define TXQ_DYN_CNGSTCTL_EN							(1<<29)
#define TXQ_CNGST_WRED_EN							(1<<TXQ_CNGST_WRED_EN_SHIFT)
#define TXQ_CNGST_WRED_EN_SHIFT                     (28)
#define TXQ_CNGST_THR_SCALE_SHIFT					(20)
#define TXQ_CNGST_THR_SCALE_MASK					(0x3<<TXQ_CNGST_THR_SCALE_SHIFT)
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

/* QDMA_CSR_TXQ_DEI_TOTALTHR	 */
#define TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_TOTAL_MAX_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_TOTAL_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_TOTAL_MIN_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_TOTAL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DEI_CHNLTHR_CFG	 */
#define TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_CHNL_MAX_THR_MASK				(0xFFFF<<TXQ_CNGST_DEI_CHNL_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_CHNL_MIN_THR_MASK				(0xFFFF<<TXQ_CNGST_DEI_CHNL_MIN_THR_SHIFT)

/* QDMA_CSR_TXQ_DEI_QUEUETHR_CFG	 */
#define TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT			(16)
#define TXQ_CNGST_DEI_QUEUE_MAX_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_QUEUE_MAX_THR_SHIFT)
#define TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT			(0)
#define TXQ_CNGST_DEI_QUEUE_MIN_THR_MASK			(0xFFFF<<TXQ_CNGST_DEI_QUEUE_MIN_THR_SHIFT)


/* QDMA_CSR_STATIC_QUEUE_THR	 */
#define TXQ_STATIC_QUEUE_DEI_THR_SHIFT				(16)
#define TXQ_STATIC_QUEUE_DEI_THR_MASK				(0xFFFF<<TXQ_STATIC_QUEUE_DEI_THR_SHIFT)
#define TXQ_STATIC_QUEUE_NORM_THR_SHIFT				(0)
#define TXQ_STATIC_QUEUE_NORM_THR_MASK				(0xFFFF<<TXQ_STATIC_QUEUE_NORM_THR_SHIFT)
/* TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 start */
#define TXQ_STATIC_CHNL_EN_CFG_SHIFT(chnl)			(chnl)
#define TXQ_STATIC_CHNL_EN_CFG_MASK(chnl)			(0x1<<TXQ_STATIC_CHNL_EN_CFG_SHIFT(chnl))

#define TXQ_STATIC_QUEUE_RATIO_SHIFT(qidx)			(qidx<<2)
#define TXQ_STATIC_QUEUE_RATIO_MASK(qidx)			(0xF<<TXQ_STATIC_QUEUE_RATIO_SHIFT(qidx))
/* TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 end */

/* QDMA_CSR_QOS_AGING_CFG */
#define QOS_AGING_EN_SHIFT							(31)
#define QOS_AGING_EN_MASK							(0x1<<QOS_AGING_EN_SHIFT)
#define QOS_AGING_METHOD_SHIFT						(30)
#define QOS_AGING_METHOD_MASK						(0x1<<QOS_AGING_METHOD_SHIFT)
#define QOS_AGING_FAST_REPLACE_SHIFT				(29)
#define QOS_AGING_FAST_REPLACE_MASK					(0x1<<QOS_AGING_FAST_REPLACE_SHIFT)
#define QOS_AGING_TIME_SHIFT						(0)
#define QOS_AGING_TIME_MASK							(0xFFFF<<QOS_AGING_TIME_SHIFT)

/* QDMA_CSR_PER_QUEUE_AGING_EN_CFG */
#define PER_QUEUE_AGING_EN_SHIFT(chnl, qidx)		(((chnl%4)<<3)+qidx)
#define PER_QUEUE_AGING_EN_MASK(chnl, qidx)			(0x1<<PER_QUEUE_AGING_EN_SHIFT(chnl,qidx))

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
#define DBG_LMGR_FREE_CNT_MASK						(0x1FFFF<<DBG_LMGR_FREE_CNT_SHIFT)

/* QDMA_CSR_DBG_MIN_LMGR_STATUS	 */
#define DBG_LMGR_MIN_CNT_SHIFT						(0)
#define DBG_LMGR_MIN_CNT_MASK						(0x1FFFF<<DBG_LMGR_MIN_CNT_SHIFT)

/* QDMA_CSR_DBG_LMGR_USAGE	 */
#define DBG_LMGR_TOTALBUF_USAGE_SHIFT				(0)
#define DBG_LMGR_TOTALBUF_USAGE_MASK				(0xFFFFFF<<DBG_LMGR_FREE_CNT_SHIFT)

/* QDMA_CSR_VCHNL_CFG */
#define GLB_CFG_VCHNL_MAP_EN						(1<<1)
#define GLB_CFG_VCHNL_MAP_MODE						(1)

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

/* QDMA_CSR_SLA_CHNL_CFG */
#define SLA_CHNL_CFG_EN_SHIFT(idx)					(((idx)<<3)+7) /* bit-7,15,23,31 */
#define SLA_CHNL_CFG_EN_MASK(idx)					(0x1<<SLA_CHNL_CFG_EN_SHIFT(idx))
#define SLA_CHNL_CFG_CHIDX_SHIFT(idx)				((idx)<<3)
#define SLA_CHNL_CFG_CHIDX_MASK(idx)				(0x1F<<SLA_CHNL_CFG_CHIDX_SHIFT(idx))

/* QDMA_CSR_DBG_CNTR_CFG */
#define DBG_CNTR_EN_SHIFT							(31)
#define DBG_CNTR_EN									(1<<DBG_CNTR_EN_SHIFT)
#define DBG_CNTR_ENABLE								(1)
#define DBG_CNTR_DISABLE							(0)
#define DBG_CNTR_ALL_CHNL_SHIFT						(30)
#define DBG_CNTR_ALL_CHNL							(1<<DBG_CNTR_ALL_CHNL_SHIFT)
#define DBG_CNTR_ALL_QUEUE_SHIFT					(29)
#define DBG_CNTR_ALL_QUEUE							(1<<DBG_CNTR_ALL_QUEUE_SHIFT)
#define DBG_CNTR_ALL_DSCPRING_SHIFT					(28)
#define DBG_CNTR_ALL_DSCPRING						(1<<DBG_CNTR_ALL_DSCPRING_SHIFT)
#define DBG_CNTR_SRC_SHIFT							(24)
#define DBG_CNTR_SRC_MASK							(0xF<<DBG_CNTR_SRC_SHIFT)
#define DBG_CNTR_SRC_CPU_TX							(0)
#define DBG_CNTR_SRC_FWD_TX							(1)
#define DBG_CNTR_SRC_CPU_RX							(2)
#define DBG_CNTR_SRC_FWD_RX							(3)
#define DBG_CNTR_CPU_TX_DROP						(4)
#define DBG_CNTR_FWD_RX_DROP						(5)
#define DBG_CNTR_CPU_RX_DROP						(6)
#define DBG_CNTR_MULTICAST_DROP                     (7)
#define DBG_CNTR_MULTICAST_ALL_DROP                 (8)
#define DBG_CNTR_AGE_OUT_DROP		                (9)

#define DBG_CNTR_DSCPRING_IDX_SHIFT					(16)
#define DBG_CNTR_DSCPRING_IDX_MASK					(0xF<<DBG_CNTR_DSCPRING_IDX_SHIFT)
#define DBG_CNTR_CHNL_SHIFT							(3)
#define DBG_CNTR_CHNL_MASK							(0x1F<<DBG_CNTR_CHNL_SHIFT)
#define DBG_CNTR_QUEUE_SHIFT						(0)
#define DBG_CNTR_QUEUE_MASK							(0x7<<DBG_CNTR_QUEUE_SHIFT)

#define MULTICAST_PORT_NUM                  (16)
#define DBG_CNTR_INDEX_MAX					(64)
#define DBG_CNTR_FWD_CHNL_GROUP_NUM 		(16)
#ifdef QDMA_LAN
#define DBG_CNTR_DEFAULT_CONFIG_NUM  		(5+TX_RING_NUM+RX_RING_NUM+MULTICAST_PORT_NUM) /*tx_cpu, tx_total, rx_cpu, rx_total, tx ring0~7, rx ring0~15 ,chnl0-5 multicast drop cntr ,all chnl multicast drop cntr*/
#define DBG_CNTR_DEFAULT_CONFIG_OFFSET  	(28) /*for multicast drop cntr*/
#else
#define DBG_CNTR_DEFAULT_CONFIG_NUM  		(4+TX_RING_NUM+RX_RING_NUM) /*tx_cpu, tx_total, rx_cpu, rx_total, tx ring0~7, rx ring0~15*/
#define DBG_CNTR_DEFAULT_CONFIG_OFFSET  	(32)
#endif

/* QDMA_CSR_DBG_CNTMEM_EN */
#define DBG_CNTRMEM_EN								(1<<31)

/* QDMA_CSR_DBG_CNTMEM_CFG */
#define DBG_CNTRMEM_PARA_RWCMD						(1<<31)
#define DBG_CNTRMEM_PARA_RWCMD_DONE					(1<<30)

#define CHNL_IDX_MASK	0x3f
#define DBG_CNTRMEM_TYPE_SHIFT						(0)
#define DBG_CNTRMEM_CHNL_IDX_SHIFT					(2)
#define DBG_CNTRMEM_TYPE_MASK						(0x3<<DBG_CNTRMEM_TYPE_SHIFT)
#define DBG_CNTRMEM_CHNL_IDX_MASK					(CHNL_IDX_MASK<<DBG_CNTRMEM_CHNL_IDX_SHIFT)


/* QDMA_CSR_MULTICAST_MODIFY_FPORT */
#define MULTICAST_SPTAG_KEEP_HI_EN_SHIFT			(31)
#define MULTICAST_SPTAG_KEEP_HI_EN_MASK				(0x1<<MULTICAST_SPTAG_KEEP_HI_EN_SHIFT)
#define MULTICAST_SPTAG_KEEP_HI_ENABLE				(1)
#define MULTICAST_SPTAG_KEEP_HI_DISABLE				(0)
#define MULTICAST_SPTAG_SHIFT						(0)
#define MULTICAST_SPTAG_MASK						(0xFFFF<<MULTICAST_SPTAG_SHIFT)
#define MULTICAST_FPORT_SHIFT						(16)
#define MULTICAST_FPORT_MASK						(0x1FF<<MULTICAST_FPORT_SHIFT)

/* QDMA_CSR_DBG_MULTICAST_DROP_NUM */
#define DBG_MULTICAST_DROP_SHIFT(chnl)				(16 * (chnl%2))
#define DBG_MULTICAST_DROP_MASK(chnl)				(0xFFFF<<DBG_MULTICAST_DROP_SHIFT(chnl))

/* QDMA_CSR_FLOWCNT_GRP_CFG */
#define FLOWCNT_INC_DROP_ENABLE                     (1)
#define FLOWCNT_INC_DROP_DISABLE                    (0)

/* QDMA_CSR_SDN_CNTR_CFG */
#define SDN_CNTR_ENABLE								(1)
#define SDN_CNTR_DISABLE							(0)
#define SDN_CNTR_EN_SHIFT							(31)
#define SDN_CNTR_EN_MASK							(0x1<<SDN_CNTR_EN_SHIFT)
#define SDN_CNTR_CPU_RX_EN_SHIFT					(3)
#define SDN_CNTR_CPU_RX_EN_MASK						(0x1<<SDN_CNTR_CPU_RX_EN_SHIFT)
#define SDN_CNTR_INC_DROP_SHIFT(grp)				(grp)
#define SDN_CNTR_INC_DROP_MASK(grp)					(0x1<<SDN_CNTR_INC_DROP_SHIFT(grp))

#define SDN_CNTR_GRP1_INC_DROP_SHIFT				(1)
#define SDN_CNTR_GRP1_INC_DROP_MASK					(0x1<<SDN_CNTR_GRP1_INC_DROP_SHIFT)
#define SDN_CNTR_GRP0_INC_DROP_SHIFT				(0)
#define SDN_CNTR_GRP0_INC_DROP_MASK					(0x1<<SDN_CNTR_GRP0_INC_DROP_SHIFT)

/* QDMA_CSR_SDN_CNTR_PARAM_CFG */
#define SDN_CNTR_PARAM_CFG_RWCMD					(1<<31)
#define SDN_CNTR_PARAM_CFG_RWCMD_DONE				(1<<30)
#define SDN_CNTR_PARAM_CMD_TYPE(grp)				(grp)
#define SDN_CNTR_PARAM_CMD_TYPE_SHIFT				(17)
#define SDN_CNTR_PARAM_CMD_TYPE_MASK				(0x3<<SDN_CNTR_PARAM_CMD_TYPE_SHIFT)
#define SDN_CNTR_PARAM_SEL_PKT						(1)
#define SDN_CNTR_PARAM_SEL_BYTE						(0)
#define SDN_CNTR_PARAM_PKTCNTSEL_SHIFT				(16)
#define SDN_CNTR_PARAM_PKTCNTSEL_MASK				(0x1<<SDN_CNTR_PARAM_PKTCNTSEL_SHIFT)
#define SDN_CNTR_PARAM_TABLE_IDX_SHIFT				(0)
#define SDN_CNTR_PARAM_TABLE_IDX_MASK				(0xFFFF<<SDN_CNTR_PARAM_TABLE_IDX_SHIFT)


/* QDMA_CSR_DBG_CHNLQLVD_CHN */
#define DBG_CHANNEL_EMPTY_SHIFT(chnl)               (((chnl)&0x03)*8)
#define DBG_CHANNEL_EMPTY_MASK(chnl)                (0xFF<<DBG_CHANNEL_EMPTY_SHIFT(chnl))


/* QDMA_AGGREGATION_CFG*/
#define AGGREGATE_EN_SHIFT							(31)
#define MAX_AGGREGATE_CNT_SHIFT						(8)
#define MAX_AGGREGATE_CNT_MASK						(0x1F<<MAX_AGGREGATE_CNT_SHIFT)
#define AGGREGATE_TIMEOUT_SHIFT						(0)
#define AGGREGATE_TIMEOUT_MASK						(0xFF<<AGGREGATE_TIMEOUT_SHIFT)
#define AGGREGATION_CFG(base)						(base+0x0700)


/* QDMA max burst len setup */
#define RX_MAX_BURST_LEN_SHIFT						(15)
#define TX_MAX_BURST_LEN_SHIFT						(14)

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
extern uint irq_tx_ring;

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _QDMA_REG_7583_H_ */
