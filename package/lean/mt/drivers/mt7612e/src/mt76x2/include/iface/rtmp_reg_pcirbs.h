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


// TODO: shiang, for RT3290, make sure following definition is correct to put as here

#ifdef RT_BIG_ENDIAN
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32       	LDO0_EN:1;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	PLL_LD:1;
		UINT32       	XTAL_RDY:1;
#ifdef RT65xx
		UINT32		Rsv:3;
		UINT32		GPIOModeLed2:1;
		UINT32		GPIOModeLed1:1;
		UINT32		CsrUartMode:1;
#else
		UINT32     Rsv:2;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_LEVEL:2;
#endif
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#else
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;	
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
#ifdef RT65xx
		UINT32		CsrUartMode:1;
		UINT32		GPIOModeLed1:1;
		UINT32		GPIOModeLed2:1;
		UINT32		Rsv:3;
#else
		UINT32		LDO25_LEVEL:2;
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32       	Rsv:2;
#endif /* RT65xx */
		UINT32       	XTAL_RDY:1;
		UINT32       	PLL_LD:1;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO0_EN:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#endif


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
	}field;
	UINT32 word;
}OSCCTL_STRUC, *POSCCTL_STRUC;
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
	}field;
	UINT32 word;
}OSCCTL_STRUC, *POSCCTL_STRUC;
#endif

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
	}field;
	UINT32 word;
}COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
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
	}field;
	UINT32 word;
}COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32       	Rsv:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_TR_DELY:8;
	}field;
	UINT32 word;
}COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#else
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32		DIS_WL_TR_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32       	Rsv:8;
	}field;
	UINT32 word;
}COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#endif

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
	}field;
	UINT32 word;
}COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
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
}COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
#endif


#define PLL_CTRL		0x50
#ifdef RT_BIG_ENDIAN
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		ULONG		VBGBK_EN:1;
		ULONG      	LOCK_DETECT_WINDOW_CTRL:3;
		ULONG		PFD_DELAY_CTRL:2;
		ULONG		CP_CURRENT_CTRL:2;
		ULONG		LPF_C2_CTRL:2;
		ULONG		LPF_C1_CTRL:2;
		ULONG		LPF_R1:1;
		ULONG		VCO_FIXED_CURRENT_CONTROL:3;
		ULONG		RESERVED_INPUT2:8;
		ULONG		RESERVED_INPUT1:8;
	}	field;
	ULONG			word;
}	PLL_CTRL_STRUC, *PPLL_CTRL_STRUC;
#else
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		ULONG		RESERVED_INPUT1:8;
		ULONG		RESERVED_INPUT2:8;
		ULONG		VCO_FIXED_CURRENT_CONTROL:3;
		ULONG		LPF_R1:1;
		ULONG		LPF_C1_CTRL:2;
		ULONG		LPF_C2_CTRL:2;
		ULONG		CP_CURRENT_CTRL:2;
		ULONG		PFD_DELAY_CTRL:2;
		ULONG      	LOCK_DETECT_WINDOW_CTRL:3;
		ULONG		VBGBK_EN:1;
	}	field;
	ULONG			word;
} PLL_CTRL_STRUC, *PPLL_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define WPDMA_RST_IDX 	0x20c
#ifdef RT_BIG_ENDIAN
typedef	union _WPDMA_RST_IDX_STRUC {
	struct	{
		UINT32       	:14;
		UINT32       	RST_DRX_IDX1:1;
		UINT32       	RST_DRX_IDX0:1;
		UINT32       	rsv:6;
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
	}	field;
	UINT32			word;
}WPDMA_RST_IDX_STRUC, *PWPDMA_RST_IDX_STRUC;
#else
typedef	union _WPDMA_RST_IDX_STRUC {
	struct	{
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
		UINT32       	rsv:6;
		UINT32       	RST_DRX_IDX0:1;
		UINT32       	RST_DRX_IDX1:1;
		UINT32       	:14;
	}	field;
	UINT32			word;
} WPDMA_RST_IDX_STRUC, *PWPDMA_RST_IDX_STRUC;
#endif
#define DELAY_INT_CFG  0x0210
#ifdef RT_BIG_ENDIAN
typedef	union _DELAY_INT_CFG_STRUC {
	struct	{
		UINT32       	TXDLY_INT_EN:1;
		UINT32       	TXMAX_PINT:7;
		UINT32       	TXMAX_PTIME:8;
		UINT32       	RXDLY_INT_EN:1;
		UINT32       	RXMAX_PINT:7;
		UINT32		RXMAX_PTIME:8;
	}	field;
	UINT32			word;
}DELAY_INT_CFG_STRUC, *PDELAY_INT_CFG_STRUC;
#else
typedef	union _DELAY_INT_CFG_STRUC {
	struct	{
		UINT32		RXMAX_PTIME:8;
		UINT32       	RXMAX_PINT:7;
		UINT32       	RXDLY_INT_EN:1;
		UINT32       	TXMAX_PTIME:8;
		UINT32       	TXMAX_PINT:7;
		UINT32       	TXDLY_INT_EN:1;
	}	field;
	UINT32			word;
} DELAY_INT_CFG_STRUC, *PDELAY_INT_CFG_STRUC;
#endif

#endif /* __RTMP_REG_PCIRBUS_H__ */
