/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_reg_pcirbus.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_REG_PCIRBUS_H__
#define __RTMP_REG_PCIRBUS_H__


#ifdef RTMP_MAC_PCI

// TODO: shiang, for RT3290, make sure following definition is correct to put as here

#define OSCCTL				0x38
#ifdef RT_BIG_ENDIAN
typedef	union _OSCCTL_STRUC{
	struct{
		UINT32       	ROSC_EN:1;
		UINT32       	CAL_REQ:1;
		UINT32       	CLK_32K_VLD:1;
		UINT32		CAL_ACK:1;
		UINT32		CAL_CNT:12;
		UINT32		Rsv:3;
		UINT32		REF_CYCLE:13;
	} field;
	UINT32 word;
}OSCCTL_STRUC;
#else
typedef union _OSCCTL_STRUC{
	struct{
		UINT32		REF_CYCLE:13;
		UINT32		Rsv:3;
		UINT32		CAL_CNT:12;
		UINT32		CAL_ACK:1;
		UINT32       	CLK_32K_VLD:1;
		UINT32       	CAL_REQ:1;
		UINT32       	ROSC_EN:1;
	} field;
	UINT32 word;
}OSCCTL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define COEXCFG0			0x40
#define COEXCFG0_COEX_EN (1 << 0)
#define COEXCFG0_FIX_WL_DI_ANT (1 << 1)
#define COEXCFG0_FIX_WL_ANT_EN (1 << 2)
#define COEXCFG0_FIX_WL_TX_PWR_MASK (0x3 << 6)
#define COEXCFG0_FIX_WL_GCRF_EN (1 << 8)
#define COEXCFG0_FIX_WL_RF_LNA0_MASK (0x3 << 12)
#define COEXCFG0_FIX_WL_RF_LNA1_MASK (0x3 << 14)
#define COEXCFG0_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG0_COEX_CFG1_MASK (0xff << 24)

#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG0_STRUC{
	struct{
		UINT32       	COEX_CFG1:8;
		UINT32       	COEX_CFG0:8;		
		UINT32       	FIX_WL_RF_LNA:2;
		UINT32		FIX_BT_H_PA:3;
		UINT32		FIX_BT_L_PA:3;
		UINT32		FIX_WL_TX_PWR:2;
		UINT32		Rsv:3;
		UINT32		FIX_WL_ANT_EN:1;
		UINT32		FIX_WL_DI_ANT:1;
		UINT32		COEX_ENT:1;
	} field;
	UINT32 word;
}COEXCFG0_STRUC;
#else
typedef union _COEXCFG0_STRUC{
	struct{
		UINT32		COEX_ENT:1;
		UINT32		FIX_WL_DI_ANT:1;
		UINT32		FIX_WL_ANT_EN:1;
		UINT32		Rsv:3;
		UINT32		FIX_WL_TX_PWR:2;
		UINT32		FIX_BT_L_PA:3;
		UINT32		FIX_BT_H_PA:3;
		UINT32       	FIX_WL_RF_LNA:2;

		UINT32       	COEX_CFG0:8;
		UINT32       	COEX_CFG1:8;
	} field;
	UINT32 word;
}COEXCFG0_STRUC;
#endif /* RT_BIG_ENDIAN */


#define COEXCFG1			0x44
#define COEXCFG1_DIS_WL_TR_DELAY_MASK (0xff << 0)
#define COEXCFG1_DIS_WL_PA_DELAY_MASK (0xff << 8)
#define COEXCFG1_DIS_WL_RF_DELAY_MASK (0xff << 16)
#define COEXCFG1_FIX_WL_TX_GCRF0_MASK (0xf << 24)
#define COEXCFG1_FIX_WL_TX_GCRF1_MASK (0xf << 28)

#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32       	Rsv:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_TR_DELY:8;
	} field;
	UINT32 word;
}COEXCFG1_STRUC;
#else
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32		DIS_WL_TR_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32       	Rsv:8;
	} field;
	UINT32 word;
}COEXCFG1_STRUC;
#endif


#define COEXCFG2			0x48
#define COEXCFG2_WL_COEX_CFG0_MASK (0xff << 0)
#define COEXCFG2_WL_COEX_CFG1_MASK (0xff << 8)
#define COEXCFG2_BT_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG2_BT_COEX_CFG1_MASK (0xff << 24)

#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG2_STRUC{
	struct{
		UINT32		BT_COEX_CFG1_Bit31_Rsv:1;
		UINT32		BT_COEX_CFG1_Bit30_Rsv:1;	
		UINT32		BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;		
		
		UINT32		BT_COEX_CFG0_Bit23_Rsv:1;
		UINT32		BT_COEX_CFG0_Bit22_Rsv:1;	
		UINT32		BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
		
		UINT32		WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;
		UINT32		WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;		
		
		UINT32		WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;
		UINT32		WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;			
	} field;
	UINT32 word;
}COEXCFG2_STRUC;
#else
typedef union _COEXCFG2_STRUC{
	struct{
		UINT32		WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;	
		UINT32		WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;		
		UINT32		WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;

		UINT32		WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;
		UINT32		WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;		
		UINT32		WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
		
		UINT32		BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit22_Rsv:1;	
		UINT32		BT_COEX_CFG0_Bit23_Rsv:1;
		
		UINT32		BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit30_Rsv:1;			
		UINT32		BT_COEX_CFG1_Bit31_Rsv:1;
	}field;
	UINT32 word;
}COEXCFG2_STRUC;
#endif


#define COEXCFG3 0x004c
#define COEXCFG3_COEX_VER (1 << 0)
#define COEXCFG3_TDM_EN (1 << 1)
#define COEXCFG3_IO_TR_SW0_MODE (1 << 2)
#define COEXCFG3_CSR_FRC_TR_SW0 (1 << 3)
#define COEXCFG3_FIX_IO_ANT_SEL_EN (1 << 4)
#define COEXCFG3_REG_IO_ANT_SEL_EN (1 << 5)
#define COEXCFG3_PTA_CNT_CLEAR (1 << 6)
#define COEXCFG3_PTA_CNT_EN (1 << 7)
#define COEXCFG3_BT_TX_STATUS (1 << 16)


#define PLL_CTRL		0x50
#ifdef RT_BIG_ENDIAN
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		UINT32		VBGBK_EN:1;
		UINT32      	LOCK_DETECT_WINDOW_CTRL:3;
		UINT32		PFD_DELAY_CTRL:2;
		UINT32		CP_CURRENT_CTRL:2;
		UINT32		LPF_C2_CTRL:2;
		UINT32		LPF_C1_CTRL:2;
		UINT32		LPF_R1:1;
		UINT32		VCO_FIXED_CURRENT_CONTROL:3;
		UINT32		RESERVED_INPUT2:8;
		UINT32		RESERVED_INPUT1:8;
	}	field;
	UINT32			word;
}	PLL_CTRL_STRUC;
#else
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		UINT32		RESERVED_INPUT1:8;
		UINT32		RESERVED_INPUT2:8;
		UINT32		VCO_FIXED_CURRENT_CONTROL:3;
		UINT32		LPF_R1:1;
		UINT32		LPF_C1_CTRL:2;
		UINT32		LPF_C2_CTRL:2;
		UINT32		CP_CURRENT_CTRL:2;
		UINT32		PFD_DELAY_CTRL:2;
		UINT32      	LOCK_DETECT_WINDOW_CTRL:3;
		UINT32		VBGBK_EN:1;
	} field;
	UINT32			word;
} PLL_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */



#if defined(RTMP_MAC) || defined(RLT_MAC)
/* INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit */
#define INT_SOURCE_CSR		0x200
/* INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF */
#define INT_MASK_CSR		0x204
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#define WPDMA_GLO_CFG		0x208
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef RT_BIG_ENDIAN
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct	{
		UINT32 rx_2b_offset:1;
		UINT32 clk_gate_dis:1;
		UINT32 rsv:14;
#ifdef DESC_32B_SUPPORT
		UINT32 RXHdrScater:7;
		UINT32 Desc32BEn:1;
#else
		UINT32 HDR_SEG_LEN:8;
#endif /* DESC_32B_SUPPORT */
		UINT32 BigEndian:1;
		UINT32 EnTXWriteBackDDONE:1;
		UINT32 WPDMABurstSIZE:2;
		UINT32 RxDMABusy:1;
		UINT32 EnableRxDMA:1;
		UINT32 TxDMABusy:1;
		UINT32 EnableTxDMA:1;
	} field;
	UINT32 word;
}WPDMA_GLO_CFG_STRUC;
#else
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct {
		UINT32 EnableTxDMA:1;
		UINT32 TxDMABusy:1;
		UINT32 EnableRxDMA:1;
		UINT32 RxDMABusy:1;
		UINT32 WPDMABurstSIZE:2;
		UINT32 EnTXWriteBackDDONE:1;
		UINT32 BigEndian:1;
#ifdef DESC_32B_SUPPORT
		UINT32 Desc32BEn:1;
		UINT32 RXHdrScater:7;
#else
		UINT32 HDR_SEG_LEN:8;
#endif /* DESC_32B_SUPPORT */
		UINT32 rsv:14;
		UINT32 clk_gate_dis:1;
		UINT32 rx_2b_offset:1;
	} field;
	UINT32 word;
} WPDMA_GLO_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */


#if defined(RTMP_MAC) || defined(RLT_MAC)
#define WPDMA_RST_IDX 	0x20c
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef RT_BIG_ENDIAN
typedef union _WPDMA_RST_IDX_STRUC {
	struct {
		UINT32       	:14;
		UINT32       	RST_DRX_IDX1:1;
		UINT32       	RST_DRX_IDX0:1;
		UINT32       	rsv:2;
		UINT32		RST_DTX_IDX13:1;
		UINT32		RST_DTX_IDX12:1;
		UINT32		RST_DTX_IDX11:1;
		UINT32		RST_DTX_IDX10:1;
		UINT32       	RST_DTX_IDX9:1;
		UINT32       	RST_DTX_IDX8:1;
		UINT32       	RST_DTX_IDX7:1;
		UINT32       	RST_DTX_IDX6:1;
		UINT32       	RST_DTX_IDX5:1;
		UINT32       	RST_DTX_IDX4:1;
		UINT32		RST_DTX_IDX3:1;
		UINT32		RST_DTX_IDX2:1;
		UINT32		RST_DTX_IDX1:1;
		UINT32		RST_DTX_IDX0:1;
	} field;
	UINT32			word;
}WPDMA_RST_IDX_STRUC;
#else
typedef union _WPDMA_RST_IDX_STRUC {
	struct {
		UINT32		RST_DTX_IDX0:1;
		UINT32		RST_DTX_IDX1:1;
		UINT32		RST_DTX_IDX2:1;
		UINT32		RST_DTX_IDX3:1;
		UINT32       	RST_DTX_IDX4:1;
		UINT32       	RST_DTX_IDX5:1;
		UINT32       	RST_DTX_IDX6:1;
		UINT32       	RST_DTX_IDX7:1;		
		UINT32       	RST_DTX_IDX8:1;
		UINT32       	RST_DTX_IDX9:1;
		UINT32		RST_DTX_IDX10:1;
		UINT32		RST_DTX_IDX11:1;
		UINT32		RST_DTX_IDX12:1;
		UINT32		RST_DTX_IDX13:1;
		UINT32       	rsv:2;
		UINT32       	RST_DRX_IDX0:1;
		UINT32       	RST_DRX_IDX1:1;
		UINT32       	:14;
	} field;
	UINT32			word;
} WPDMA_RST_IDX_STRUC;
#endif /* RT_BIG_ENDIAN */


#if defined(RTMP_MAC) || defined(RLT_MAC)
#define DELAY_INT_CFG  0x0210
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef RT_BIG_ENDIAN
typedef	union _DELAY_INT_CFG_STRUC {
	struct {
		UINT32       	TXDLY_INT_EN:1;
		UINT32       	TXMAX_PINT:7;
		UINT32       	TXMAX_PTIME:8;
		UINT32       	RXDLY_INT_EN:1;
		UINT32       	RXMAX_PINT:7;
		UINT32		RXMAX_PTIME:8;
	} field;
	UINT32			word;
}DELAY_INT_CFG_STRUC;
#else
typedef	union _DELAY_INT_CFG_STRUC {
	struct {
		UINT32		RXMAX_PTIME:8;
		UINT32       	RXMAX_PINT:7;
		UINT32       	RXDLY_INT_EN:1;
		UINT32       	TXMAX_PTIME:8;
		UINT32       	TXMAX_PINT:7;
		UINT32       	TXDLY_INT_EN:1;
	} field;
	UINT32			word;
} DELAY_INT_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */


/*
	Hardware Periodic Timer interrupt setting.
	Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
*/
#define RTMP_HW_TIMER_INT_SET(_pAd, _V)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(_pAd, INT_TIMER_CFG, &temp);		\
		temp &= 0x0000ffff;								\
		temp |= _V << 20; 								\
		RTMP_IO_WRITE32(_pAd, INT_TIMER_CFG, temp);		\
	}

/* Enable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_ENABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp |=0x2;										\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}

/* Disable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_DISABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp &=~(0x2);									\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}
#endif /* RTMP_MAC_PCI */


#endif /* __RTMP_REG_PCIRBUS_H__ */

