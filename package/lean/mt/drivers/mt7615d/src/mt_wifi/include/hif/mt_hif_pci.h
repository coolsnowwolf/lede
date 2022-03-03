/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_hif_pci.h
*/

#ifndef __MT_HIF_PCI_H__
#define __MT_HIF_PCI_H__

#include "mac/mac.h"
#include "rtmp_dmacb.h"

#define HIF_BASE			0x4000
#define MT_HIF_BASE			0x4000

/*
	CR definitions
*/
#define HIF_SYS_REV			(MT_HIF_BASE + 0x0000)
#define HIF_FUN_CAP			(MT_HIF_BASE + 0x0004)


#define MT_CFG_PCIE_MISC	(MT_HIF_BASE + 0x0014)

#define MT_CFG_LPCR_HOST	(MT_HIF_BASE + 0x01f0)


#define MT_HOST_SET_OWN	(1<<0)
#define MT_HOST_CLR_OWN	(1<<1)


#define MT_CFG_LPCR_FW		(MT_HIF_BASE + 0x01f4)
#define MT_FW_CLR_OWN	(1<<0)

#ifdef ERR_RECOVERY
#define MT_MCU_INT_EVENT         (MT_HIF_BASE + 0x01f8)
#define MT7663_MCU_INT_EVENT     (MT_HIF_BASE + 0x0108)
#define MCU_INT_PDMA0_STOP_DONE	        BIT(0)
#define MCU_INT_PDMA0_INIT_DONE	        BIT(1)
#define MCU_INT_SER_TRIGGER_FROM_HOST   BIT(2)
#define MCU_INT_PDMA0_RECOVERY_DONE	    BIT(3)
#endif /* ERR_RECOVERY .*/

#define MT_INT_SOURCE_CSR	(MT_HIF_BASE + 0x0200)
#define MT_INT_MASK_CSR	(MT_HIF_BASE + 0x0204)

#ifdef ERR_RECOVERY
#define ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD BIT(1)
#define ERROR_DETECT_STOP_PDMA BIT(2)
#define ERROR_DETECT_RESET_DONE BIT(3)
#define ERROR_DETECT_RECOVERY_DONE BIT(4)
#define ERROR_DETECT_N9_NORMAL_STATE  BIT(5)
#define CP_LMAC_HANG_WORKAROUND_STEP1 BIT(8)
#define CP_LMAC_HANG_WORKAROUND_STEP2 BIT(9)
#define ERROR_DETECT_LMAC_ERROR BIT(24)
#define ERROR_DETECT_PSE_ERROR BIT(25)
#define ERROR_DETECT_PLE_ERROR BIT(26)
#define ERROR_DETECT_PDMA_ERROR BIT(27)
#define ERROR_DETECT_PCIE_ERROR BIT(28)
#endif



#if defined(MT7615) || defined(MT7622) || defined(P18)
#define MT_MCU_CMD_CSR	(MT_HIF_BASE + 0x0234)
#endif

#define MT_MCU_CMD_CLEAR_FW_OWN BIT(0)
#ifdef ERR_RECOVERY
#define ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD BIT(1)
#define ERROR_DETECT_STOP_PDMA BIT(2)
#define ERROR_DETECT_RESET_DONE BIT(3)
#define ERROR_DETECT_RECOVERY_DONE BIT(4)
#define ERROR_DETECT_N9_NORMAL_STATE  BIT(5)
#define CP_LMAC_HANG_WORKAROUND_STEP1 BIT(8)
#define CP_LMAC_HANG_WORKAROUND_STEP2 BIT(9)
#define ERROR_DETECT_LMAC_ERROR BIT(24)
#define ERROR_DETECT_PSE_ERROR BIT(25)
#define ERROR_DETECT_PLE_ERROR BIT(26)
#define ERROR_DETECT_PDMA_ERROR BIT(27)
#define ERROR_DETECT_PCIE_ERROR BIT(28)


#if defined(MT7615) || defined(MT7622) || defined(P18)
#define ERROR_DETECT_MASK \
	(ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD \
	  | ERROR_DETECT_STOP_PDMA \
	  | ERROR_DETECT_RESET_DONE \
	  | ERROR_DETECT_RECOVERY_DONE \
	  | ERROR_DETECT_N9_NORMAL_STATE \
	  | ERROR_DETECT_LMAC_ERROR \
	  | ERROR_DETECT_PSE_ERROR \
	  | ERROR_DETECT_PLE_ERROR \
	  | ERROR_DETECT_PDMA_ERROR \
	  | ERROR_DETECT_PCIE_ERROR)
#endif

#define ERROR_RECOVERY_PDMA0_STOP_NOTIFY BIT(0)
#define ERROR_RECOVERY_PDMA0_INIT_DONE_NOTIFY BIT(1)
#endif /* ERR_RECOVERY */

/*
	INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit
	Refer to MT_INT_SOURCE_CSR(0x4000)
*/
#ifdef RT_BIG_ENDIAN
typedef union _MT_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 rsv_31:1;
		UINT32 mcu_cmd_int:1;
		UINT32 wf_mac_int_5:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_0:1;
		UINT32 tx_dly_int:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_coherent:1;
		UINT32 rx_coherent:1;
		UINT32 tx_done_15:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_13:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_0:1;
		UINT32 err_det_int_1:1;
		UINT32 err_det_int_0:1;
		UINT32 rx_done_1:1;
		UINT32 rx_done_0:1;
	} field;
	UINT32 word;
} MT_INT_SOURCE_CSR_STRUC;
#else
typedef union _MT_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 rx_done_0:1;
		UINT32 rx_done_1:1;
		UINT32 err_det_int_0:1;
		UINT32 err_det_int_1:1;
		UINT32 tx_done_0:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_13:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_15:1;
		UINT32 rx_coherent:1;
		UINT32 tx_coherent:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_dly_int:1;
		UINT32 wf_mac_int_0:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_5:1;
		UINT32 mcu_cmd_int:1;
		UINT32 rsv_31:1;
	} field;
	UINT32 word;
} MT_INT_SOURCE_CSR_STRUC;
#endif /* RT_BIG_ENDIAN */


/* INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF */
#define WPDMA_INT_MSK        (HIF_BASE + 0x204)
#ifdef RT_BIG_ENDIAN
typedef union _MT_WPDMA_INT_MASK {
	struct {
		UINT32 rsv_31:1;
		UINT32 mcu_cmd_int:1;
		UINT32 wf_mac_int_5:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_0:1;
		UINT32 tx_dly_int:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_coherent:1;
		UINT32 rx_coherent:1;
		UINT32 tx_done_15:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_13:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_0:1;
		UINT32 err_det_int_1:1;
		UINT32 err_det_int_0:1;
		UINT32 rx_done_1:1;
		UINT32 rx_done_0:1;
	} field;
	UINT32 word;
} MT_WPMDA_INT_MASK;
#else
typedef union _MT_WPDMA_INT_MASK {
	struct {
		UINT32 rx_done_0:1;
		UINT32 rx_done_1:1;
		UINT32 err_det_int_0:1;
		UINT32 err_det_int_1:1;
		UINT32 tx_done_0:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_13:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_15:1;
		UINT32 rx_coherent:1;
		UINT32 tx_coherent:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_dly_int:1;
		UINT32 wf_mac_int_0:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_5:1;
		UINT32 mcu_cmd_int:1;
		UINT32 rsv_31:1;
	} field;
	UINT32 word;
} MT_WPMDA_INT_MASK;
#endif /* RT_BIG_ENDIAN */

#define MT_INT_R0_DONE		(1<<0)
#define MT_INT_R1_DONE		(1<<1)

#define MT_INT_ERR_DET0	(1<<2)
#define MT_INT_ERR_DET1	(1<<3)

#define MT_INT_T0_DONE		(1<<4)
#define MT_INT_T1_DONE		(1<<5)
#define MT_INT_T2_DONE		(1<<6)
#define MT_INT_T3_DONE		(1<<7)
#define MT_INT_T4_DONE		(1<<8)
#define MT_INT_T5_DONE		(1<<9)
#define MT_INT_T6_DONE		(1<<10)
#define MT_INT_T7_DONE		(1<<11)
#define MT_INT_T8_DONE		(1<<12)
#define MT_INT_T9_DONE		(1<<13)
#define MT_INT_T10_DONE	(1<<14)
#define MT_INT_T11_DONE	(1<<15)
#define MT_INT_T12_DONE	(1<<16)
#define MT_INT_T13_DONE	(1<<17)
#define MT_INT_T14_DONE	(1<<18) /* mt7615 don't have this INT*/
#define MT_INT_T15_DONE	(1<<19)

#define MT_INT_TX_DONE (MT_INT_T0_DONE | MT_INT_T1_DONE | MT_INT_T2_DONE | MT_INT_T3_DONE |\
			MT_INT_T4_DONE | MT_INT_T5_DONE | MT_INT_T6_DONE | MT_INT_T7_DONE |\
			MT_INT_T8_DONE | MT_INT_T9_DONE | MT_INT_T10_DONE | MT_INT_T11_DONE |\
			MT_INT_T12_DONE | MT_INT_T13_DONE | MT_INT_T14_DONE | MT_INT_T15_DONE)

#define MT_INT_RX_COHE		(1<<20)
#define MT_INT_TX_COHE		(1<<21)

#define MT_INT_RX_DLY		(1<<22)
#define MT_INT_TX_DLY		(1<<23)

#define WF_MAC_INT_0		(1<<24)
#define WF_MAC_INT_1		(1<<25)
#define WF_MAC_INT_2		(1<<26)
#define WF_MAC_INT_3		(1<<27)
#define WF_MAC_INT_4		(1<<28)
#define WF_MAC_INT_5		(1<<29)


#define MT_INT_MCU_CMD		(1<<30)


#define MT_FW_CLEAR_OWN_BIT		(1<<31)

#ifdef MULTI_LAYER_INTERRUPT
#define MT_INT_WPDMA2HOST_ERR_INT_STS		(1<<24)
#define MT_INT_SUBSYS_INT_STS				(1<<28)
#define MT_INT_MCU2HOST_SW_INT_STS			(1<<29)
#endif


#define MT_RxINT		(MT_INT_R0_DONE | MT_INT_R1_DONE /* | MT_INT_RX_DLY */)	 /* Delayed Rx or indivi rx */

#define MT_TxMgmtInt		(MT_INT_T4_DONE /*| INT_TX_DLY*/)

#define MT_TxCoherent		MT_INT_TX_COHE
#define MT_RxCoherent		MT_INT_RX_COHE
#define MT_TxRxCoherent		(MT_INT_TX_COHE | MT_INT_RX_COHE)
#define MT_MacInt		(WF_MAC_INT_0 | WF_MAC_INT_1 | \
						 WF_MAC_INT_2 | WF_MAC_INT_3 | \
						 WF_MAC_INT_4 | WF_MAC_INT_5)

#define MT_CoherentInt		(MT_INT_RX_COHE | MT_INT_TX_COHE)
#define MT_DelayInt			(MT_INT_RX_DLY | MT_INT_TX_DLY)

#define MT_McuCommand		MT_INT_MCU_CMD	/* mcu */
#define MT_FW_CLR_OWN_INT	MT_FW_CLEAR_OWN_BIT

#define MT_INT_RX			(MT_INT_R0_DONE | MT_INT_R1_DONE)
#define MT_INT_RX_DATA		(MT_INT_R0_DONE)
#define MT_INT_RX_CMD		(MT_INT_R1_DONE)

#define MT_INT_AC0_DLY		(MT_INT_T0_DONE)
#define MT_INT_AC1_DLY		(MT_INT_T1_DONE)
#define MT_INT_AC2_DLY		(MT_INT_T2_DONE)
#define MT_INT_AC3_DLY		(MT_INT_T3_DONE)
#define MT_INT_AC4_DLY		(MT_INT_T4_DONE)
#define MT_INT_AC5_DLY		(MT_INT_T5_DONE)
#define MT_INT_AC6_DLY		(MT_INT_T6_DONE)
#define MT_INT_AC7_DLY		(MT_INT_T7_DONE)
#define MT_INT_AC8_DLY		(MT_INT_T8_DONE)
#define MT_INT_AC9_DLY		(MT_INT_T9_DONE)
#define MT_INT_AC10_DLY		(MT_INT_T10_DONE)
#define MT_INT_AC11_DLY		(MT_INT_T11_DONE)
#define MT_INT_AC12_DLY		(MT_INT_T12_DONE)
#define MT_INT_AC13_DLY		(MT_INT_T13_DONE)
#if defined(MT7622) || defined(P18) || defined(MT7663)
#define MT_INT_AC14_DLY		(MT_INT_T14_DONE)
#endif
#define MT_INT_AC15_DLY		(MT_INT_T15_DONE)

#define MT_INT_CMD			(MT_INT_T5_DONE)

#define MT_INT_MGMT_DLY	    (MT_INT_T4_DONE)
#define MT_INT_BMC_DLY		(MT_INT_T8_DONE) /* (MT_INT_T6_DONE) */
#define MT_INT_BCN_DLY		(MT_INT_T7_DONE) /* (MT_INT_T15_DONE) */

#define MT_DELAYINTMASK	0x3FFBFFFF
#define MT_INTMASK			0x3FFBFFFF


/*
	CR WPDMA GLO CFG
*/
#define MT_WPDMA_GLO_CFG	    (MT_HIF_BASE + 0x0208)
#define TX_DMA_EN               (1 << 0)
#define TX_DMA_BUSY             (1 << 1)
#define RX_DMA_EN               (1 << 2)
#define RX_DMA_BUSY             (1 << 3)
#define TX_WB_DDONE             (1 << 6)
#define BIG_ENDIAN              (1 << 7)
/* #define 32B_DESP_EN             (1 << 8) */
#define SHARE_FIFO_EN           (1 << 9)

#define FIFO_LITTLE_ENDIAN      (1 << 12)

#define SW_RST                  (1 << 24)
#define FORCE_TX_EOF            (1 << 25)
#define OMIT_RX_INFO            (1 << 27)
#define OMIT_TX_INFO            (1 << 28)
#define BYTE_SWAP               (1 << 29)
#define CLK_GATE_DIS            (1 << 30)
#define RX_2B_OFFSET            (1 << 31)

#define WPDMA_RST_PTR		(MT_HIF_BASE + 0x020c)
#ifdef RT_BIG_ENDIAN
typedef union _WPDMA_RST_IDX_STRUC {
	struct {
		UINT32		rsv_18_31:14;
		UINT32		RST_DRX_IDX1:1;
		UINT32		RST_DRX_IDX0:1;
		UINT32		rsv_2_15:14;
		UINT32		RST_DTX_IDX1:1;
		UINT32		RST_DTX_IDX0:1;
	} field;
	UINT32			word;
} WPDMA_RST_IDX_STRUC;
#else
typedef union _WPDMA_RST_IDX_STRUC {
	struct {
		UINT32		RST_DTX_IDX0:1;
		UINT32		RST_DTX_IDX1:1;
		UINT32		rsv_2_15:14;
		UINT32		RST_DRX_IDX0:1;
		UINT32		RST_DRX_IDX1:1;
		UINT32		rsv_18_31:14;
	} field;
	UINT32			word;
} WPDMA_RST_IDX_STRUC;
#endif /* RT_BIG_ENDIAN */


#define MT_DELAY_INT_CFG	(MT_HIF_BASE + 0x0210)
#ifdef RT_BIG_ENDIAN
typedef	union _DELAY_INT_CFG_STRUC {
	struct {
		UINT32		TXDLY_INT_EN:1;
		UINT32		TXMAX_PINT:7;
		UINT32		TXMAX_PTIME:8;
		UINT32		RXDLY_INT_EN:1;
		UINT32		RXMAX_PINT:7;
		UINT32		RXMAX_PTIME:8;
	} field;
	UINT32			word;
} DELAY_INT_CFG_STRUC;
#else
typedef	union _DELAY_INT_CFG_STRUC {
	struct {
		UINT32		RXMAX_PTIME:8;
		UINT32		RXMAX_PINT:7;
		UINT32		RXDLY_INT_EN:1;
		UINT32		TXMAX_PTIME:8;
		UINT32		TXMAX_PINT:7;
		UINT32		TXDLY_INT_EN:1;
	} field;
	UINT32			word;
} DELAY_INT_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */


#define MT_WPDMA_TX_DMAD_RNG	(MT_HIF_BASE + 0x0214)
#define MT_WPDMA_RX_DMAD_RNG	(MT_HIF_BASE + 0x0218)
#define MT_WPDMA_TX_PLD_RNG	    (MT_HIF_BASE + 0x021C)
#define MT_WPDMA_RX_PLD_RNG	    (MT_HIF_BASE + 0x0220)
#define MT_WPDMA_MEM_RNG_ERR	(MT_HIF_BASE + 0x0224)

#define MT_WPDMA_PAUSE_RX_Q_TH10	(MT_HIF_BASE + 0x0260)
#define TX_PRE_ADDR_ALIGN_MODE_128	BITS(28, 30)
#define TX_PRE_ADDR_ALIGN_MODE_UMASK	~(BITS(28, 30))

#define MT_WPDMA_TX_RING0_CTRL0	(MT_HIF_BASE + 0x0300)
#define MT_WPDMA_TX_RING0_CTRL1	(MT_HIF_BASE + 0x0304)
#define MT_WPDMA_TX_RING0_CTRL2	(MT_HIF_BASE + 0x0308)
#define MT_WPDMA_TX_RING0_CTRL3	(MT_HIF_BASE + 0x030C)

#define MT_WPDMA_TX_RING1_CTRL0	(MT_HIF_BASE + 0x0310)
#define MT_WPDMA_TX_RING1_CTRL1	(MT_HIF_BASE + 0x0314)
#define MT_WPDMA_TX_RING1_CTRL2	(MT_HIF_BASE + 0x0318)
#define MT_WPDMA_TX_RING1_CTRL3	(MT_HIF_BASE + 0x031C)

#define MT_WPDMA_TX_RING2_CTRL0	(MT_HIF_BASE + 0x0320)
#define MT_WPDMA_TX_RING2_CTRL1	(MT_HIF_BASE + 0x0324)
#define MT_WPDMA_TX_RING2_CTRL2	(MT_HIF_BASE + 0x0328)
#define MT_WPDMA_TX_RING2_CTRL3	(MT_HIF_BASE + 0x032C)

#define MT_WPDMA_TX_RING3_CTRL0	(MT_HIF_BASE + 0x0330)
#define MT_WPDMA_TX_RING3_CTRL1	(MT_HIF_BASE + 0x0334)
#define MT_WPDMA_TX_RING3_CTRL2	(MT_HIF_BASE + 0x0338)
#define MT_WPDMA_TX_RING3_CTRL3	(MT_HIF_BASE + 0x033C)

#define MT_WPDMA_RX_RING0_CTRL0	(MT_HIF_BASE + 0x0400)
#define MT_WPDMA_RX_RING0_CTRL1	(MT_HIF_BASE + 0x0404)
#define MT_WPDMA_RX_RING0_CTRL2	(MT_HIF_BASE + 0x0408)
#define MT_WPDMA_RX_RING0_CTRL3	(MT_HIF_BASE + 0x040C)

#define MT_WPDMA_RX_RING1_CTRL0	(MT_HIF_BASE + 0x0410)
#define MT_WPDMA_RX_RING1_CTRL1	(MT_HIF_BASE + 0x0414)
#define MT_WPDMA_RX_RING1_CTRL2	(MT_HIF_BASE + 0x0418)
#define MT_WPDMA_RX_RING1_CTRL3	(MT_HIF_BASE + 0x041C)

enum {
	TX_RING_LOW,
	TX_RING_HIGH,
};

enum {
	TX_RING_HIGH_TO_HIGH,
	TX_RING_HIGH_TO_LOW,
	TX_RING_LOW_TO_LOW,
	TX_RING_LOW_TO_HIGH,
	TX_RING_UNKNOW_CHANGE,
};

/*@!Release
	Tx Ring Layout and assignments

	Totally we have 10 Tx Rings and assigned as following usage:
	1. RT85592
		TxRing 0~3: for TxQ Channel 1 with AC_BK/BE/VI/VO
		TxRing 4    : for TxQ CTRL
		TxRing 5    : for TxQ MGMT
		TxRing 6~9: for TxQ Channel 2 with AC_BK/BE/VI/VO

	2. MT7650
		TxRing 0~3: for TxQ Channel 1 with AC_BK/BE/VI/VO
		TxRing 4~7: for TxQ Channel 2 with AC_BK/BE/VI/VO
		TxRing 8    : for TxQ CTRL
		TxRing 9    : for TxQ MGMT

	For each TxRing, we have four register to control it
		TX_RINGn_CTRL0 (0x0): base address of this ring(4-DWORD aligned address)
		TX_RINGn_CTRL1 (0x4): maximum number of TxD count in this ring
		TX_RINGn_CTRL2 (0x8): Point to the next TxD CPU wants to use
		TX_RINGn_CTRL3 (0xc): Point to the next TxD DMA wants to use
*/
#define MT_RINGREG_DIFF		0x10

#define MT_TX_RING_BASE		(MT_HIF_BASE + 0x0300)
#define MT_TX_RING_PTR			(MT_TX_RING_BASE + 0x0)
#define MT_TX_RING_CNT			(MT_TX_RING_BASE + 0x4)
#define MT_TX_RING_CIDX		(MT_TX_RING_BASE + 0x8)
#define MT_TX_RING_DIDX		(MT_TX_RING_BASE + 0xc)

/* following address is base on TX_CHAN_BASE_X */
#define MT_TX_RING_BK_BASE		0x0
#define MT_TX_RING_BK_CNT		(MT_TX_RING_BK_BASE + 0x04)
#define MT_TX_RING_BK_CIDX		(MT_TX_RING_BK_BASE + 0x08)
#define MT_TX_RING_BK_DIDX		(MT_TX_RING_BK_BASE + 0x0c)

#define MT_TX_RING_BE_BASE		(MT_TX_RING_BK_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_BE_CNT		(MT_TX_RING_BE_BASE + 0x04)
#define MT_TX_RING_BE_CIDX		(MT_TX_RING_BE_BASE + 0x08)
#define MT_TX_RING_BE_DIDX		(MT_TX_RING_BE_BASE + 0x0c)

#define MT_TX_RING_VI_BASE		(MT_TX_RING_BE_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_VI_CNT		(MT_TX_RING_VI_BASE + 0x04)
#define MT_TX_RING_VI_CIDX		(MT_TX_RING_VI_BASE + 0x08)
#define MT_TX_RING_VI_DIDX		(MT_TX_RING_VI_BASE + 0x0c)

#define MT_TX_RING_VO_BASE	(MT_TX_RING_VI_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_VO_CNT		(MT_TX_RING_VO_BASE + 0x04)
#define MT_TX_RING_VO_CIDX		(MT_TX_RING_VO_BASE + 0x08)
#define MT_TX_RING_VO_DIDX		(MT_TX_RING_VO_BASE + 0x0c)


#define MT_TX_RING_BCN_IDX		7

/*
	Rx Ring Layput and assignments

	Totally we have 2 Rx Rings and assigned as following usage:
		RxRing 0: for all received data packets
		RxRing 1: for internal ctrl/info packets generated by on-chip CPU.

	For each TxRing, we have four register to control it
		RX_RING_CTRL0 (0x0): base address of this ring(4-DWORD aligned address)
		RX_RING_CTRL1 (0x4): maximum number of RxD count in this ring
		RX_RING_CTRL2 (0x8): Point to the next RxD CPU wants to use
		RX_RING_CTRL3 (0xc): Point to the next RxD DMA wants to use
*/
#define MT_RX_RING_BASE	(HIF_BASE + 0x0400)
#define MT_RX_RING1_BASE	(HIF_BASE + 0x0410)
#define MT_RX_RING_NUM	2
#define MT_RX_RING_PTR		(MT_RX_RING_BASE + 0x00)
#define MT_RX_RING_CNT		(MT_RX_RING_BASE + 0x04)
#define MT_RX_RING_CIDX	(MT_RX_RING_BASE + 0x08)
#define MT_RX_RING_DIDX	(MT_RX_RING_BASE + 0x0c)

#define MT_RX_RING1_PTR	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1)
#define MT_RX_RING1_CNT	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x04)
#define MT_RX_RING1_CIDX	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x08)
#define MT_RX_RING1_DIDX	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x0c)

#define PCI_CFG_DEVICE_CONTROL				(MT_HIF_BASE + 0x2088)
#define PCI_CFG_MAX_PAYLOAD_SIZE_4K			(BIT(5)|BIT(7))
#define PCI_CFG_MAX_READ_REQ_4K				(BIT(12)|BIT(14))
#define PCI_CFG_MAX_PAYLOAD_SIZE_UMASK			~(BITS(5, 7))
#define PCI_CFG_MAX_READ_REQ_UMASK			~(BITS(12, 14))

#define PCI_K_CNT2					(MT_HIF_BASE + 0x3014)
#define K_CNT_MAX_PAYLOAD_SIZE_4K			(BIT(0)|BIT(2))
#define K_CNT_MAX_PAYLOAD_SIZE_UMASK			~(BITS(0, 2))

#define PCI_K_CONF_FUNC0_4				(MT_HIF_BASE + 0x3110)
#define K_CONF_MAX_PAYLOAD_SIZE_4K			(BIT(0)|BIT(2))
#define K_CONF_MAX_PAYLOAD_SIZE_UMASK			~(BITS(0, 2))

/* =================================================================================
	PCI/RBUS TX / RX Frame Descriptors format

	Memory Layout

	1. Tx Descriptor
			TxD (12 bytes) + TXINFO (4 bytes)
	2. Packet Buffer
			TXWI + 802.11
	 31                                                                                                                             0
	+--------------------------------------------------------------------------+
	|                                   SDP0[31:0]                                                                               |
	+-+--+---------------------+-+--+-----------------------------------------+
	|D |L0|       SDL0[13:0]              |B|L1|                    SDL1[13:0]                                    |
	+-+--+---------------------+-+--+-----------------------------------------+
	|                                   SDP1[31:0]                                                                               |
	+--------------------------------------------------------------------------+
	|                                        TXINFO                                                                                |
	+--------------------------------------------------------------------------+
================================================================================= */
/*
	TX descriptor format for Tx Data/Mgmt Rings
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word 0 */
	UINT32		SDPtr0;

	/* Word 1 */
	UINT32		DMADONE:1;
	UINT32		LastSec0:1;
	UINT32		SDLen0:14;
	UINT32		Burst:1;
	UINT32		LastSec1:1;
	UINT32		SDLen1:14;
	/* Word 2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#else
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word	0 */
	UINT32		SDPtr0;

	/* Word	1 */
	UINT32		SDLen1:14;
	UINT32		LastSec1:1;
	UINT32		Burst:1;
	UINT32		SDLen0:14;
	UINT32		LastSec0:1;
	UINT32		DMADONE:1;
	/*Word2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#endif /* RT_BIG_ENDIAN */


/*
	Rx descriptor format for Rx Rings
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXD_STRUC {
	/* Word 0 */
	UINT32		SDP0;

	/* Word 1 */
	UINT32		DDONE:1;
	UINT32		LS0:1;
	UINT32		SDL0:14;
	UINT32		BURST:1;
	UINT32		LS1:1;
	UINT32		SDL1:14;

	/* Word 2 */
	UINT32		SDP1;
} RXD_STRUC;
#else
typedef	struct GNU_PACKED _RXD_STRUC {
	/* Word	0 */
	UINT32		SDP0;

	/* Word	1 */
	UINT32		SDL1:14;
	UINT32		LS1:1;
	UINT32		BURST:1;
	UINT32		SDL0:14;
	UINT32		LS0:1;
	UINT32		DDONE:1;

	/* Word	2 */
	UINT32		SDP1;
} RXD_STRUC;
#endif /* RT_BIG_ENDIAN */

typedef struct _RTMP_MGMT_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_MGMT_RING;

typedef struct _RTMP_CTRL_RING {
	RTMP_DMACB Cell[CTL_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_CTRL_RING;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
typedef struct _RTMP_FWDWLO_RING {
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;    /* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
	UINT32 ring_size;
	NDIS_SPIN_LOCK RingLock;    /* Ring spinlock */
	RTMP_DMABUF DescRing;    /* Shared memory for CTRL descriptors */
	RTMP_DMACB Cell[MGMT_RING_SIZE];
} RTMP_FWDWLO_RING;

typedef struct _RTMP_RING {
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;    /* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
	UINT32 ring_size;
	NDIS_SPIN_LOCK RingLock;    /* Ring spinlock */
	RTMP_DMABUF DescRing;    /* Shared memory for Ring descriptors */
	RTMP_DMACB Cell[0];
} RTMP_RING;
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */

#ifdef MT_MAC
typedef struct _RTMP_BCN_RING {
	RTMP_DMACB Cell[BCN_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
	UINT32 hw_desc_base;
	UINT32 hw_cidx_addr;
	UINT32 hw_didx_addr;
	UINT32 hw_cnt_addr;
} RTMP_BCN_RING;
#endif /* MT_MAC */

#define INC_INDEX(_idx, _RingSize)    \
	{                                          \
		(_idx) = (_idx+1) % (_RingSize);       \
	}

typedef struct _RTMP_TX_RING {
	UINT32 hw_didx_addr;
	UINT32 TxDmaIdx;
	RTMP_DMACB *Cell;
	UINT32 TxSwFreeIdx;
	BOOLEAN tx_ring_state;
	UINT32 tx_ring_low_water_mark;
	UINT32 tx_ring_high_water_mark;
	UINT32 tx_ring_full_cnt;
	UINT32 hw_cidx_addr;
	UINT32 TxCpuIdx;
	UINT32 hw_desc_base;
	UINT32 hw_cnt_addr;
} RTMP_TX_RING ____cacheline_aligned;

enum {
	FREE_BUF_1k,
	FREE_BUF_64k,
};

typedef struct _RTMP_RX_RING {
	UINT16 max_rx_process_cnt;
	UINT32 hw_didx_addr;
	UINT32 RxDmaIdx;
	UINT32 RxSwReadIdx;
	UINT16 RxRingSize;
	RTMP_DMACB *Cell;
	UINT16 free_buf_head;
	UINT16 free_buf_tail;
	UINT16 free_buf_size;
	RTMP_DMABUF *free_buf;
	UINT8 cur_free_buf_len;
	UINT16 RxBufferSize;
	UINT16 sw_read_idx_inc;
	UINT32 RxCpuIdx;
	UINT32 hw_cidx_addr;
	UINT16 free_buf_64k_head;
	UINT16 free_buf_64k_tail;
	UINT16 free_buf_64k_size;
	RTMP_DMABUF *free_buf_64k;
	UINT32 hw_desc_base;
	UINT32 hw_cnt_addr;
} RTMP_RX_RING ____cacheline_aligned;

enum PACKET_TYPE {
	TX_DATA,
	TX_DATA_HIGH_PRIO,
	TX_MGMT,
	TX_ALTX,
	TX_CMD,
	TX_FW_DL,
	TX_DATA_PS,
	PACKET_TYPE_NUM,
};

typedef struct _PCI_HIF_T {
	UINT32 IntEnableReg ____cacheline_aligned;
	UINT32 intDisableMask;
	UINT32 IntPending;
	RTMP_DMABUF *RxDescRing;
	NDIS_SPIN_LOCK *RxRingLock;
	RTMP_RX_RING *RxRing;
	RTMP_DMABUF *TxBufSpace;
	RTMP_DMABUF *TxDescRing;
	NDIS_SPIN_LOCK *TxRingLock;
	RTMP_TX_RING *TxRing;
	BOOLEAN (*dma_done_handle[PACKET_TYPE_NUM])(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);

	PUINT8	CSRBaseAddress; /* PCI MMIO Base Address, all access will use */

	/* System Information */
	UINT16 VendorID;         /* Read from PCI config */
	UINT16 DeviceID;         /* Read from PCI config */
	UINT16 SubVendorID;      /* Read from PCI config */
	UINT16 SubSystemID;      /* Read from PCI config */

	UINT16 SpecificVendorID; /* Read form Registry */
	UINT16 SpecificDeviceID; /* Read form Registry */
	UINT16 RevsionID;        /* Read from PCI config */
	RTMP_DMABUF MgmtDescRing;	/* Shared memory for MGMT descriptors */
	RTMP_MGMT_RING MgmtRing;
	NDIS_SPIN_LOCK MgmtRingLock;	/* Prio Ring spinlock */

#ifdef MT_MAC
	RTMP_DMABUF BcnDescRing;	/* Shared memory for Beacon descriptors */
	RTMP_BCN_RING BcnRing;
	NDIS_SPIN_LOCK BcnRingLock;	/* Beacon Ring spinlock */
#endif

#ifdef CONFIG_ANDES_SUPPORT
	RTMP_DMABUF CtrlDescRing;	/* Shared memory for CTRL descriptors */
	RTMP_CTRL_RING CtrlRing;
	NDIS_SPIN_LOCK CtrlRingLock;	/* Ctrl Ring spinlock */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	RTMP_FWDWLO_RING FwDwloRing;
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	RTMP_DMABUF TxBmcBufSpace;	/* Shared memory of all 1st pre-allocated TxBuf associated with each TXD */
	RTMP_DMABUF TxBmcDescRing;	/* Shared memory for Tx descriptors */
	NDIS_SPIN_LOCK TxBmcRingLock;	/* Beacon Ring spinlock */
	RTMP_TX_RING TxBmcRing;		/* BMC */
#endif /* MT_MAC */
#ifdef CONFIG_WIFI_MSI_SUPPORT
	BOOLEAN is_msi;
#endif /* CONFIG_WIFI_MSI_SUPPORT */
} PCI_HIF_T, *PPCI_HIF_T;

#define MT_WPDMA_GLO_CFG_1      (MT_HIF_BASE + 0x0500)
#define TXP_ACTIVE_MODE_MASK (0x3 << 0)
#define TXP_ACTIVE_MODE(p) (((p) & 0x3) << 0)
#define GET_TXP_ACTIVE_MODE(p) (((p) & TXP_ACTIVE_MODE_MASK) >> 0)
#define MI_DEPTH_WR_2_0_MASK (0x7 << 13)
#define MI_DEPTH_WR_2_0(p) (((p) & 0x3) << 13)
#define GET_MI_DEPTH_WR_2_0(p) (((p) & MI_DEPTH_WR_2_0_MASK) >> 13)
#define MI_DEPTH_WR_5_3_MASK (0x7 << 16)
#define MI_DEPTH_WR_5_3(p) (((p) & 0x3) << 16)
#define GET_MI_DEPTH_WR_5_3(p) (((p) & MI_DEPTH_WR_5_3_MASK) >> 16)
#define MI_DEPTH_WR_8_6_MASK (0x7 << 19)
#define MI_DEPTH_WR_8_6(p) (((p) & 0x3) << 19)
#define GET_MI_DEPTH_WR_8_6(p) (((p) & MI_DEPTH_WR_8_6_MASK) >> 19)

#define MT_WPDMA_TX_PRE_CFG        (MT_HIF_BASE + 0x0510)
#define TX_RING0_DMAD_PRE_NUM_MASK (0x3 << 0)
#define TX_RING0_DMAD_PRE_NUM(p) (((p) & 0x3) << 0)
#define GET_TX_RING0_DMAD_PRE_NUM(p) (((p) & TX_RING0_DMAD_PRE_NUM_MASK) >> 0)
#define TX_RING1_DMAD_PRE_NUM_MASK (0x3 << 2)
#define TX_RING1_DMAD_PRE_NUM(p) (((p) & 0x3) << 2)
#define GET_TX_RING1_DMAD_PRE_NUM(p) (((p) & TX_RING1_DMAD_PRE_NUM_MASK) >> 2)
#define TX_RING0_DMAD_PRE_HI_PRI_NUM_MASK (0x3 << 16)
#define TX_RING0_DMAD_PRE_HI_PRI_NUM(p) (((p) & 0x3) << 16)
#define GET_TX_RING0_DMAD_PRE_HI_PRI_NUM(p) (((p) & TX_RING0_DMAD_PRE_HI_PRI_NUM_MASK) >> 16)
#define TX_RING1_DMAD_PRE_HI_PRI_NUM_MASK (0x3 << 18)
#define TX_RING1_DMAD_PRE_HI_PRI_NUM(p) (((p) & 0x3) << 18)
#define GET_TX_RING1_DMAD_PRE_HI_PRI_NUM(p) (((p) & TX_RING1_DMAD_PRE_HI_PRI_NUM_MASK) >> 18)

#define MT_WPDMA_RX_PRE_CFG        (MT_HIF_BASE + 0x0520)
#define RX_RING0_DMAD_PRE_NUM_MASK (0x7f << 0)
#define RX_RING0_DMAD_PRE_NUM(p) (((p) & 0x7f) << 0)
#define GET_RX_RING0_DMAD_PRE_NUM(p) (((p) & RX_RING0_DMAD_PRE_NUM_MASK) >> 0)
#define RX_RING1_DMAD_PRE_NUM_MASK (0xf << 8)
#define RX_RING1_DMAD_PRE_NUM(p) (((p) & 0xf) << 8)
#define GET_RX_RING1_DMAD_PRE_NUM(p) (((p) & RX_RING1_DMAD_PRE_NUM_MASK) >> 8)
#define RX_RING0_DMAD_PRE_HI_PRI_NUM_MASK (0x7f << 16)
#define RX_RING0_DMAD_PRE_HI_PRI_NUM(p) (((p) & 0x7f) << 16)
#define GET_RX_RING0_DMAD_PRE_HI_PRI_NUM(p) (((p) & RX_RING0_DMAD_PRE_HI_PRI_NUM_MASK) >> 16)
#define RX_RING1_DMAD_PRE_HI_PRI_NUM_MASK (0xf << 24)
#define RX_RING1_DMAD_PRE_HI_PRI_NUM(p) (((p) & 0xf) << 24)
#define GET_RX_RING1_DMAD_PRE_HI_PRI_NUM(p) (((p) & RX_RING1_DMAD_PRE_HI_PRI_NUM_MASK) >> 24)

#define MT_WPDMA_ABT_CFG           (MT_HIF_BASE + 0x0530)
#define TRX_DFET_ABT_TYPE_MASK (0x3 << 0)
#define TRX_DFET_ABT_TYPE(p) (((p) & 0x3) << 0)
#define GET_TRX_DFET_ABT_TYPE(p) (((p) & TRX_DFET_ABT_TYPE_MASK) >> 0)
#define TRX_PFET_ABT_TYPE_MASK (0x3 << 2)
#define TRX_PFET_ABT_TYPE(p) (((p) & 0x3) << 2)
#define GET_TRX_PFET_ABT_TYPE(p) (((p) & TRX_PFET_ABT_TYPE_MASK) >> 2)
#define PDMA_DFET_PFET_ABT_TYPE_MASK (0x3 << 4)
#define PDMA_DFET_PFET_ABT_TYPE(p) (((p) & 0x3) << 4)
#define GET_PDMA_DFET_PFET_ABT_TYPE(p) (((p) & PDMA_DFET_PFET_ABT_TYPE_MASK) >> 4)
#define PDMA_DFET_PFET_ABT_W_FULL_IN_DIS_MASK (0x1 << 6)
#define PDMA_DFET_PFET_ABT_W_FULL_IN_DIS(p) (((p) & 0x1) << 6)
#define GET_PDMA_DFET_PFET_ABT_W_FULL_IN_DIS(p) (((p) & PDMA_DFET_PFET_ABT_W_FULL_IN_DIS_MASK) >> 6)
#define REQ_MASK_DIS_MASK (0x1 << 7)
#define REQ_MASK_DIS(p) (((p) & 0x1) << 7)
#define GET_REQ_MASK_DIS(p) (((p) & REQ_MASK_DIS_MASK) >> 7)
#define TXDMAD_PRI_WIN_DIS_MASK (0x1 << 8)
#define TXDMAD_PRI_WIN_DIS(p) (((p) & 0x1) << 8)
#define GET_TXDMAD_PRI_WIN_DIS(p) (((p) & TXDMAD_PRI_WIN_DIS_MASK) >> 8)
#define RXDMAD_PRI_WIN_DIS_MASK (0x1 << 9)
#define RXDMAD_PRI_WIN_DIS(p) (((p) & 0x1) << 9)
#define GET_RXDMAD_PRI_WIN_DIS(p) (((p) & RXDMAD_PRI_WIN_DIS_MASK) >> 9)
#define TXP_PRI_WIN_DIS_MASK (0x1 << 10)
#define TXP_PRI_WIN_DIS(p) (((p) & 0x1) << 10)
#define GET_TXP_PRI_WIN_DIS(p) (((p) & TXP_PRI_WIN_DIS_MASK) >> 10)
#define TXD_PRI_WIN_DIS_MASK (0x1 << 11)
#define TXD_PRI_WIN_DIS(p) (((p) & 0x1) << 11)
#define GET_TXD_PRI_WIN_DIS(p) (((p) & TXD_PRI_WIN_DIS_MASK) >> 11)
#define WRR_DIS_MASK (0x1 << 12)
#define WRR_DIS(p) (((p) & 0x1) << 12)
#define GET_WRR_DIS(p) (((p) & WRR_DIS_MASK) >> 12)
#define D_SP_FIXED_PRI_MASK (0x7 << 13)
#define D_SP_FIXED_PRI(p) (((p) & 0x7) << 13)
#define GET_D_SP_FIXED_PRI(p) (((p) & D_SP_FIXED_PRI_MASK) >> 13)
#define WRR_TIME_SLOT_DURATION_MASK (0xffff << 16)
#define WRR_TIME_SLOT_DURATION(p) (((p) & 0xffff) << 16)
#define GET_WRR_TIME_SLOT_DURATION(p) (((p) & WRR_TIME_SLOT_DURATION_MASK) >> 16)

#define MT_WPDMA_ABT_CFG1          (MT_HIF_BASE + 0x0534)
#define WRR_TIME_SLOT_PRIORITY_0_MASK (0x3 << 0)
#define WRR_TIME_SLOT_PRIORITY_0(p) (((p) & 0x3) << 0)
#define GET_WRR_TIME_SLOT_PRIORITY_0(p) (((p) & WRR_TIME_SLOT_PRIORITY_0_MASK) >> 0)
#define WRR_TIME_SLOT_PRIORITY_1_MASK (0x3 << 2)
#define WRR_TIME_SLOT_PRIORITY_1(p) (((p) & 0x3) << 2)
#define GET_WRR_TIME_SLOT_PRIORITY_1(p) (((p) & WRR_TIME_SLOT_PRIORITY_1_MASK) >> 2)
#define WRR_TIME_SLOT_PRIORITY_2_MASK (0x3 << 4)
#define WRR_TIME_SLOT_PRIORITY_2(p) (((p) & 0x3) << 4)
#define GET_WRR_TIME_SLOT_PRIORITY_2(p) (((p) & WRR_TIME_SLOT_PRIORITY_2_MASK) >> 4)
#define WRR_TIME_SLOT_PRIORITY_3_MASK (0x3 << 6)
#define WRR_TIME_SLOT_PRIORITY_3(p) (((p) & 0x3) << 6)
#define GET_WRR_TIME_SLOT_PRIORITY_3(p) (((p) & WRR_TIME_SLOT_PRIORITY_3_MASK) >> 6)
#define WRR_TIME_SLOT_PRIORITY_4_MASK (0x3 << 8)
#define WRR_TIME_SLOT_PRIORITY_4(p) (((p) & 0x3) << 8)
#define GET_WRR_TIME_SLOT_PRIORITY_4(p) (((p) & WRR_TIME_SLOT_PRIORITY_4_MASK) >> 8)
#define WRR_TIME_SLOT_PRIORITY_5_MASK (0x3 << 10)
#define WRR_TIME_SLOT_PRIORITY_5(p) (((p) & 0x3) << 10)
#define GET_WRR_TIME_SLOT_PRIORITY_5(p) (((p) & WRR_TIME_SLOT_PRIORITY_5_MASK) >> 10)
#define WRR_TIME_SLOT_PRIORITY_6_MASK (0x3 << 12)
#define WRR_TIME_SLOT_PRIORITY_6(p) (((p) & 0x3) << 12)
#define GET_WRR_TIME_SLOT_PRIORITY_6(p) (((p) & WRR_TIME_SLOT_PRIORITY_6_MASK) >> 12)
#define WRR_TIME_SLOT_PRIORITY_7_MASK (0x3 << 14)
#define WRR_TIME_SLOT_PRIORITY_7(p) (((p) & 0x3) << 14)
#define GET_WRR_TIME_SLOT_PRIORITY_7(p) (((p) & WRR_TIME_SLOT_PRIORITY_7_MASK) >> 14)
#define WRR_TIME_SLOT_PRIORITY_8_MASK (0x3 << 16)
#define WRR_TIME_SLOT_PRIORITY_8(p) (((p) & 0x3) << 16)
#define GET_WRR_TIME_SLOT_PRIORITY_8(p) (((p) & WRR_TIME_SLOT_PRIORITY_8_MASK) >> 16)
#define WRR_TIME_SLOT_PRIORITY_9_MASK (0x3 << 18)
#define WRR_TIME_SLOT_PRIORITY_9(p) (((p) & 0x3) << 18)
#define GET_WRR_TIME_SLOT_PRIORITY_9(p) (((p) & WRR_TIME_SLOT_PRIORITY_9_MASK) >> 18)
#define WRR_TIME_SLOT_PRIORITY_10_MASK (0x3 << 20)
#define WRR_TIME_SLOT_PRIORITY_10(p) (((p) & 0x3) << 20)
#define GET_WRR_TIME_SLOT_PRIORITY_10(p) (((p) & WRR_TIME_SLOT_PRIORITY_10_MASK) >> 20)
#define WRR_TIME_SLOT_PRIORITY_11_MASK (0x3 << 22)
#define WRR_TIME_SLOT_PRIORITY_11(p) (((p) & 0x3) << 22)
#define GET_WRR_TIME_SLOT_PRIORITY_11(p) (((p) & WRR_TIME_SLOT_PRIORITY_11_MASK) >> 22)
#define WRR_TIME_SLOT_PRIORITY_12_MASK (0x3 << 24)
#define WRR_TIME_SLOT_PRIORITY_12(p) (((p) & 0x3) << 24)
#define GET_WRR_TIME_SLOT_PRIORITY_12(p) (((p) & WRR_TIME_SLOT_PRIORITY_12_MASK) >> 24)
#define WRR_TIME_SLOT_PRIORITY_13_MASK (0x3 << 26)
#define WRR_TIME_SLOT_PRIORITY_13(p) (((p) & 0x3) << 26)
#define GET_WRR_TIME_SLOT_PRIORITY_13(p) (((p) & WRR_TIME_SLOT_PRIORITY_13_MASK) >> 26)
#define WRR_TIME_SLOT_PRIORITY_14_MASK (0x3 << 28)
#define WRR_TIME_SLOT_PRIORITY_14(p) (((p) & 0x3) << 28)
#define GET_WRR_TIME_SLOT_PRIORITY_14(p) (((p) & WRR_TIME_SLOT_PRIORITY_14_MASK) >> 28)
#define WRR_TIME_SLOT_PRIORITY_15_MASK (0x3 << 30)
#define WRR_TIME_SLOT_PRIORITY_15(p) (((p) & 0x3) << 30)
#define GET_WRR_TIME_SLOT_PRIORITY_15(p) (((p) & WRR_TIME_SLOT_PRIORITY_15_MASK) >> 30)

#define K_GBL_1 (HIF_BASE + 0x3000)
#define HIF_K_GBL_CDC_IMP_MASK (1 << 23)
#define GET_HIF_K_GBL_CDC_IMP(p) (((p) & HIF_K_GBL_CDC_IMP_MASK) >> 23)

#define K_CPL_RESOURCE (HIF_BASE + 0x3158)
#define HIF_K_RX_SPLIT_EN_MASK (1 << 16)
#define GET_HIF_K_RX_SPLIT_EN(p) (((p) & HIF_K_RX_SPLIT_EN_MASK) >> 16)

#endif /* __MT_HIF_PCI_H__ */

