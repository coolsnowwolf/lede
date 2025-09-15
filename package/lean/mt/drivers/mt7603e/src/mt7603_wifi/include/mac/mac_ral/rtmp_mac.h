/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_mac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_MAC_H__
#define __RTMP_MAC_H__


#ifdef RLT_MAC
#include "mac/mac_ral/nmac/ral_nmac.h"
#endif /* RLT_MAC */

#ifdef RTMP_MAC
#include "mac/mac_ral/omac/ral_omac.h"
#endif /* RTMP_MAC */

#include "mac/mac_ral/rtmp_mac.h"
#ifdef RTMP_MAC_PCI
#include "mac/mac_ral/mac_pci.h"
#endif /* RTMP_MAC_PCI */


/*
	TX / RX ring descriptor format

	TX:
		PCI/RBUS_Descriptor + TXINFO + TXWI + 802.11

	Rx:
		PCI/RBUS/USB_Descripotr + (PCI/RBUS RXFCE_INFO) + (PCI/RBUS  RXINFO) + RXWI + 802.11 + (USB RXINFO)
	
*/

/* the first 24-byte in TXD is called TXINFO and will be DMAed to MAC block through TXFIFO. */
/* MAC block use this TXINFO to control the transmission behavior of this frame. */
#define FIFO_MGMT	0
#define FIFO_HCCA	1
#define FIFO_EDCA	2

#ifndef MT_MAC
typedef	union GNU_PACKED _TXWI_STRUC {
#ifdef RLT_MAC
	struct _TXWI_NMAC TXWI_N;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	struct _TXWI_OMAC TXWI_O;
#endif /* RTMP_MAC */
	UINT32 word;
}TXWI_STRUC;


/*
	RXWI wireless information format, in PBF. invisible in driver.
*/
typedef union GNU_PACKED _RXWI_STRUC {
#ifdef RLT_MAC
	struct _RXWI_NMAC RXWI_N;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	struct _RXWI_OMAC RXWI_O;
#endif /* RTMP_MAC */
}RXWI_STRUC;
#endif /* !MT_MAC */

typedef	union GNU_PACKED _HW_RATE_CTRL_STRUCT_ {
#ifdef RLT_MAC
		struct _NMAC_HW_RATE_CTRL_STRUCT RATE_CTRL_N;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		struct _OMAC_HW_RATE_CTRL_STRUCT RATE_CTRL_O;
#endif /* RTMP_MAC */
		UINT16 word;
}HW_RATE_CTRL_STRUCT;


#define TSO_SIZE		0


/* ================================================================================= */
/* Register format */
/* ================================================================================= */

#define ASIC_VERSION		0x0000


/*
	SCH/DMA registers - base address 0x0200
*/
#define WMM_AIFSN_CFG   0x0214
#ifdef RT_BIG_ENDIAN
typedef	union _AIFSN_CSR_STRUC{
	struct {
	    UINT32   Aifsn7:4;       /* for AC_VO */
	    UINT32   Aifsn6:4;       /* for AC_VI */
	    UINT32   Aifsn5:4;       /* for AC_BK */
	    UINT32   Aifsn4:4;       /* for AC_BE */
	    UINT32   Aifsn3:4;       /* for AC_VO */
	    UINT32   Aifsn2:4;       /* for AC_VI */
	    UINT32   Aifsn1:4;       /* for AC_BK */
	    UINT32   Aifsn0:4;       /* for AC_BE */
	}field;
	UINT32 word;
} AIFSN_CSR_STRUC;
#else
typedef union _AIFSN_CSR_STRUC {
	struct {
	    UINT32   Aifsn0:4;
	    UINT32   Aifsn1:4;
	    UINT32   Aifsn2:4;
	    UINT32   Aifsn3:4;
	    UINT32   Aifsn4:4;
	    UINT32   Aifsn5:4;
	    UINT32   Aifsn6:4;
	    UINT32   Aifsn7:4;
	} field;
	UINT32 word;
} AIFSN_CSR_STRUC;
#endif

/* CWMIN_CSR: CWmin for each EDCA AC */
#define WMM_CWMIN_CFG   0x0218
#ifdef RT_BIG_ENDIAN
typedef	union _CWMIN_CSR_STRUC	{
	struct {
		UINT32   Cwmin7:4;       /* for AC_VO */
		UINT32   Cwmin6:4;       /* for AC_VI */
		UINT32   Cwmin5:4;       /* for AC_BK */
		UINT32   Cwmin4:4;       /* for AC_BE */
		UINT32   Cwmin3:4;       /* for AC_VO */
		UINT32   Cwmin2:4;       /* for AC_VI */
		UINT32   Cwmin1:4;       /* for AC_BK */
		UINT32   Cwmin0:4;       /* for AC_BE */
	} field;
	UINT32 word;
} CWMIN_CSR_STRUC;
#else
typedef	union _CWMIN_CSR_STRUC	{
	struct {
	    UINT32   Cwmin0:4;
	    UINT32   Cwmin1:4;
	    UINT32   Cwmin2:4;
	    UINT32   Cwmin3:4;
	    UINT32   Cwmin4:4;
	    UINT32   Cwmin5:4;
	    UINT32   Cwmin6:4;
	    UINT32   Cwmin7:4;
	} field;
	UINT32 word;
} CWMIN_CSR_STRUC;
#endif


/* CWMAX_CSR: CWmin for each EDCA AC */
#define WMM_CWMAX_CFG   0x021c
#ifdef RT_BIG_ENDIAN
typedef	union _CWMAX_CSR_STRUC {
	struct {
		UINT32   Cwmax7:4;       /* for AC_VO */
		UINT32   Cwmax6:4;       /* for AC_VI */
		UINT32   Cwmax5:4;       /* for AC_BK */
		UINT32   Cwmax4:4;       /* for AC_BE */
		UINT32   Cwmax3:4;       /* for AC_VO */
		UINT32   Cwmax2:4;       /* for AC_VI */
		UINT32   Cwmax1:4;       /* for AC_BK */
		UINT32   Cwmax0:4;       /* for AC_BE */
	} field;
	UINT32 word;
} CWMAX_CSR_STRUC;
#else
typedef	union _CWMAX_CSR_STRUC {
	struct {
	    UINT32   Cwmax0:4;
	    UINT32   Cwmax1:4;
	    UINT32   Cwmax2:4;
	    UINT32   Cwmax3:4;
	    UINT32   Cwmax4:4;
	    UINT32   Cwmax5:4;
	    UINT32   Cwmax6:4;
	    UINT32   Cwmax7:4;
	} field;
	UINT32 word;
}	CWMAX_CSR_STRUC;
#endif


/* AC_TXOP_CSR0: AC_BK/AC_BE TXOP register */
#define WMM_TXOP0_CFG    0x0220
#ifdef RT_BIG_ENDIAN
typedef	union _AC_TXOP_CSR0_STRUC {
	struct {
	    UINT16  Ac1Txop; /* for AC_BE, in unit of 32us */
	    UINT16  Ac0Txop; /* for AC_BK, in unit of 32us */
	} field;
	UINT32 word;
} AC_TXOP_CSR0_STRUC;
#else
typedef	union _AC_TXOP_CSR0_STRUC {
	struct {
	    UINT16  Ac0Txop;
	    UINT16  Ac1Txop;
	} field;
	UINT32 word;
} AC_TXOP_CSR0_STRUC;
#endif


/* AC_TXOP_CSR1: AC_VO/AC_VI TXOP register */
#define WMM_TXOP1_CFG    0x0224
#ifdef RT_BIG_ENDIAN
typedef	union _AC_TXOP_CSR1_STRUC {
	struct {
	    UINT16  Ac3Txop; /* for AC_VO, in unit of 32us */
	    UINT16  Ac2Txop; /* for AC_VI, in unit of 32us */
	} field;
	UINT32 word;
} AC_TXOP_CSR1_STRUC;
#else
typedef	union _AC_TXOP_CSR1_STRUC {
	struct {
	    UINT16 Ac2Txop;
	    UINT16 Ac3Txop;
	} field;
	UINT32 word;
} AC_TXOP_CSR1_STRUC;
#endif


#define WMM_TXOP2_CFG 0x0228
#define WMM_TXOP3_CFG 0x022c

#define WMM_CTRL	0x0230


/*================================================================================= */
/* MAC  registers                                                                                                                                                                 */
/*================================================================================= */
/*  4.1 MAC SYSTEM  configuration registers (offset:0x1000) */
#define MAC_CSR0            0x1000
#ifdef RT_BIG_ENDIAN
typedef	union _ASIC_VER_ID_STRUC {
	struct {
	    UINT16  ASICVer;        /* version */
	    UINT16  ASICRev;        /* reversion */
	} field;
	UINT32 word;
} ASIC_VER_ID_STRUC;
#else
typedef	union _ASIC_VER_ID_STRUC {
	struct {
	    UINT16  ASICRev;
	    UINT16  ASICVer;
	} field;
	UINT32 word;
} ASIC_VER_ID_STRUC;
#endif

#define MAC_SYS_CTRL		0x1004
#define PBF_LOOP_EN			(1 << 5)
#define MAC_SYS_CTRL_TXEN	(1 << 2)
#define MAC_SYS_CTRL_RXEN	(1 << 3)

#define MAC_ADDR_DW0		0x1008
#define MAC_ADDR_DW1		0x100c

/* MAC_CSR2: STA MAC register 0 */
#ifdef RT_BIG_ENDIAN
typedef	union _MAC_DW0_STRUC {
	struct {
		UINT8 Byte3;		/* MAC address byte 3 */
		UINT8 Byte2;		/* MAC address byte 2 */
		UINT8 Byte1;		/* MAC address byte 1 */
		UINT8 Byte0;		/* MAC address byte 0 */
	} field;
	UINT32 word;
} MAC_DW0_STRUC;
#else
typedef	union _MAC_DW0_STRUC {
	struct {
		UINT8 Byte0;
		UINT8 Byte1;
		UINT8 Byte2;
		UINT8 Byte3;
	} field;
	UINT32 word;
} MAC_DW0_STRUC;
#endif


/* MAC_CSR3: STA MAC register 1 */
#ifdef RT_BIG_ENDIAN
typedef	union _MAC_DW1_STRUC {
	struct {
		UINT8		Rsvd1;
		UINT8		U2MeMask;
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Byte4;		/* MAC address byte 4 */
	} field;
	UINT32 word;
} MAC_DW1_STRUC;
#else
typedef	union _MAC_DW1_STRUC {
	struct {
		UINT8 Byte4;
		UINT8 Byte5;
		UINT8 U2MeMask;
		UINT8 Rsvd1;
	} field;
	UINT32 word;
}	MAC_DW1_STRUC;
#endif

#define MAC_BSSID_DW0		0x1010
#define MAC_BSSID_DW1		0x1014
/* MAC_CSR5: BSSID register 1 */
#ifdef RT_BIG_ENDIAN
typedef	union	_MAC_BSSID_DW1_STRUC {
	struct {
		UINT32 NMBssMode3:1;
		UINT32 NMBssMode2:1;
		UINT32 NMBssMode:1;
		UINT32 MBssBcnNum:3;
		UINT32 MBssMode:2; /* 0: one BSSID, 10: 4 BSSID,  01: 2 BSSID , 11: 8BSSID */
		UINT32 Byte5:8;		 /* BSSID byte 5 */
		UINT32 Byte4:8;		 /* BSSID byte 4 */
	} field;
	UINT32 word;
} MAC_BSSID_DW1_STRUC;
#else
typedef	union	_MAC_BSSID_DW1_STRUC {
	struct {
		UINT32 Byte4:8;
		UINT32 Byte5:8;
		UINT32 MBssMode:2;
		UINT32 MBssBcnNum:3;
		UINT32 NMBssMode:1;
		UINT32 NMBssMode2:1;		
		UINT32 NMBssMode3:1;
	} field;
	UINT32 word;
} MAC_BSSID_DW1_STRUC;
#endif

/* rt2860b max 16k bytes. bit12:13 Maximum PSDU length (power factor) 0:2^13, 1:2^14, 2:2^15, 3:2^16 */
#define MAX_LEN_CFG              0x1018


/* BBP_CSR_CFG: BBP serial control register */
#define BBP_CSR_CFG            	0x101c
#ifdef RT_BIG_ENDIAN
typedef	union _BBP_CSR_CFG_STRUC {
	struct {
		UINT32		:12;		
		UINT32		BBP_RW_MODE:1;	/* 0: use serial mode  1:parallel */
		UINT32		BBP_PAR_DUR:1;		/* 0: 4 MAC clock cycles  1: 8 MAC clock cycles */
		UINT32		Busy:1;				/* 1: ASIC is busy execute BBP programming. */
		UINT32		fRead:1;				/* 0: Write BBP, 1:	Read BBP */
		UINT32		RegNum:8;			/* Selected BBP register */
		UINT32		Value:8;				/* Register value to program into BBP */
	} field;
	UINT32 word;
} BBP_CSR_CFG_STRUC;
#else
typedef	union _BBP_CSR_CFG_STRUC {
	struct {
		UINT32		Value:8;
		UINT32		RegNum:8;
		UINT32		fRead:1;
		UINT32		Busy:1;
		UINT32		BBP_PAR_DUR:1;
		UINT32		BBP_RW_MODE:1;
		UINT32		:12;
	} field;
	UINT32 word;
} BBP_CSR_CFG_STRUC;
#endif


/* RF_CSR_CFG: RF control register */
#define RF_CSR_CFG0            		0x1020
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG0_STRUC {
	struct {
		UINT32 Busy:1;		    /* 0: idle 1: 8busy */
		UINT32 Sel:1;			/* 0:RF_LE0 activate  1:RF_LE1 activate */
		UINT32 StandbyMode:1;	/* 0: high when stand by 1:	low when standby */
		UINT32 bitwidth:5;		/* Selected BBP register */
		UINT32 RegIdAndContent:24;	/* Register value to program into BBP */
	} field;
	UINT32 word;
} RF_CSR_CFG0_STRUC;
#else
typedef	union _RF_CSR_CFG0_STRUC {
	struct {
		UINT32 RegIdAndContent:24;
		UINT32 bitwidth:5;
		UINT32 StandbyMode:1;
		UINT32 Sel:1;
		UINT32 Busy:1;
	} field;
	UINT32 word;
} RF_CSR_CFG0_STRUC;
#endif


#define RF_CSR_CFG1           		0x1024		 
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG1_STRUC {
	struct {
		UINT32 rsv:7;	/* 0: idle 1: 8busy */
		UINT32 RFGap:5;	/* Gap between BB_CONTROL_RF and RF_LE. 0: 3 system clock cycle (37.5usec) 1: 5 system clock cycle (62.5usec) */
		UINT32 RegIdAndContent:24;	/* Register value to program into BBP */
	} field;
	UINT32 word;
} RF_CSR_CFG1_STRUC;
#else
typedef	union _RF_CSR_CFG1_STRUC {
	struct {
		UINT32 RegIdAndContent:24;
		UINT32 RFGap:5;
		UINT32 rsv:7;
	} field;
	UINT32 word;
} RF_CSR_CFG1_STRUC;
#endif


#define RF_CSR_CFG2           		0x1028
#ifdef RT_BIG_ENDIAN
typedef	union _RF_CSR_CFG2_STRUC {
	struct {
		UINT32 rsv:8;		    /* 0: idle 1: 8busy */
		UINT32 RegIdAndContent:24; /* Register value to program into BBP */
	} field;
	UINT32 word;
}	RF_CSR_CFG2_STRUC;
#else
typedef	union _RF_CSR_CFG2_STRUC {
	struct {
		UINT32 RegIdAndContent:24;
		UINT32 rsv:8;
	} field;
	UINT32 word;
} RF_CSR_CFG2_STRUC;
#endif


#define LED_CFG           		0x102c 
#ifdef RT_BIG_ENDIAN
typedef	union _LED_CFG_STRUC {
	struct {
		UINT32		:1;			 
		UINT32		LedPolar:1;			/* Led Polarity.  0: active low1: active high */
		UINT32		YLedMode:2;			/* yellow Led Mode */
		UINT32		GLedMode:2;			/* green Led Mode */
		UINT32		RLedMode:2;			/* red Led Mode    0: off1: blinking upon TX2: periodic slow blinking3: always on */
		UINT32		rsv:2;		    
		UINT32		SlowBlinkPeriod:6;			/* slow blinking period. unit:1ms */
		UINT32		OffPeriod:8;			/* blinking off period unit 1ms */
		UINT32		OnPeriod:8;			/* blinking on period unit 1ms */
	} field;
	UINT32 word;
} LED_CFG_STRUC;
#else
typedef	union _LED_CFG_STRUC {
	struct {
		UINT32		OnPeriod:8;
		UINT32		OffPeriod:8;
		UINT32		SlowBlinkPeriod:6;
		UINT32		rsv:2;
		UINT32		RLedMode:2;
		UINT32		GLedMode:2;
		UINT32		YLedMode:2;	
		UINT32		LedPolar:1;
		UINT32		:1;
	} field;
	UINT32 word;
} LED_CFG_STRUC;
#endif


#define AMPDU_MAX_LEN_20M1S	0x1030
#define AMPDU_MAX_LEN_20M2S	0x1034
#define AMPDU_MAX_LEN_20M1S_MCS0_7	0x1030
#define AMPDU_MAX_LEN_20M1S_MCS8_9	0x1034
#define AMPDU_MAX_LEN_40M1S	0x1038
#define AMPDU_MAX_LEN_40M2S	0x103c
#define AMPDU_MAX_LEN			0x1040


/* The number of the Tx chains */
#define NUM_OF_TX_CHAIN		4

#define TX_CHAIN_ADDR0_L	0x1044		/* Stream mode MAC address registers */
#define TX_CHAIN_ADDR0_H	0x1048
#define TX_CHAIN_ADDR1_L	0x104C
#define TX_CHAIN_ADDR1_H	0x1050
#define TX_CHAIN_ADDR2_L	0x1054
#define TX_CHAIN_ADDR2_H	0x1058
#define TX_CHAIN_ADDR3_L	0x105C
#define TX_CHAIN_ADDR3_H	0x1060

#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_L_STRUC {
	struct {
		UINT8 TxChainAddr0L_Byte3; /* Destination MAC address of Tx chain0 (byte 3) */
		UINT8 TxChainAddr0L_Byte2; /* Destination MAC address of Tx chain0 (byte 2) */
		UINT8 TxChainAddr0L_Byte1; /* Destination MAC address of Tx chain0 (byte 1) */
		UINT8 TxChainAddr0L_Byte0; /* Destination MAC address of Tx chain0 (byte 0) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR0_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_L_STRUC {
	struct {
		UINT8 TxChainAddr0L_Byte0;
		UINT8 TxChainAddr0L_Byte1;
		UINT8 TxChainAddr0L_Byte2;
		UINT8 TxChainAddr0L_Byte3;
	} field;
	UINT32 word;
} TX_CHAIN_ADDR0_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR0_H_STRUC {
	struct {
		UINT16	Reserved:12; /* Reserved */
		UINT16	TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr0H_Byte5; /* Destination MAC address of Tx chain0 (byte 5) */
		UINT8	TxChainAddr0H_Byte4; /* Destination MAC address of Tx chain0 (byte 4) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR0_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR0_H_STRUC {
	struct {
		UINT8	TxChainAddr0H_Byte4; /* Destination MAC address of Tx chain0 (byte 4) */
		UINT8	TxChainAddr0H_Byte5; /* Destination MAC address of Tx chain0 (byte 5) */
		UINT16	TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT16	Reserved:12; /* Reserved */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR0_H_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_L_STRUC {
	struct {
		UINT8	TxChainAddr1L_Byte3; /* Destination MAC address of Tx chain1 (byte 3) */
		UINT8	TxChainAddr1L_Byte2; /* Destination MAC address of Tx chain1 (byte 2) */
		UINT8	TxChainAddr1L_Byte1; /* Destination MAC address of Tx chain1 (byte 1) */
		UINT8	TxChainAddr1L_Byte0; /* Destination MAC address of Tx chain1 (byte 0) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR1_L_STRUC {
	struct {
		UINT8	TxChainAddr1L_Byte0;
		UINT8	TxChainAddr1L_Byte1;
		UINT8	TxChainAddr1L_Byte2;
		UINT8	TxChainAddr1L_Byte3;
	} field;
	UINT32 word;
} TX_CHAIN_ADDR1_L_STRUC, *PTX_CHAIN_ADDR1_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR1_H_STRUC {
	struct {
		UINT16	Reserved:12; /* Reserved */
		UINT16	TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr1H_Byte5; /* Destination MAC address of Tx chain1 (byte 5) */
		UINT8	TxChainAddr1H_Byte4; /* Destination MAC address of Tx chain1 (byte 4) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR1_H_STRUC ;
#else
typedef union _TX_CHAIN_ADDR1_H_STRUC {
	struct {
		UINT8	TxChainAddr1H_Byte4; 
		UINT8	TxChainAddr1H_Byte5; 
		UINT16	TxChainSel0:4; 
		UINT16	Reserved:12; 
	} field;
	UINT32 word;
} TX_CHAIN_ADDR1_H_STRUC ;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_L_STRUC {
	struct {
		UINT8	TxChainAddr2L_Byte3; /* Destination MAC address of Tx chain2 (byte 3) */
		UINT8	TxChainAddr2L_Byte2; /* Destination MAC address of Tx chain2 (byte 2) */
		UINT8	TxChainAddr2L_Byte1; /* Destination MAC address of Tx chain2 (byte 1) */
		UINT8	TxChainAddr2L_Byte0; /* Destination MAC address of Tx chain2 (byte 0) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR2_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_L_STRUC {
	struct {
		UINT8	TxChainAddr2L_Byte0;
		UINT8	TxChainAddr2L_Byte1;
		UINT8	TxChainAddr2L_Byte2;
		UINT8	TxChainAddr2L_Byte3;
	} field;
	UINT32 word;
} TX_CHAIN_ADDR2_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR2_H_STRUC {
	struct {
		UINT16	Reserved:12; /* Reserved */
		UINT16	TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr2H_Byte5; /* Destination MAC address of Tx chain2 (byte 5) */
		UINT8	TxChainAddr2H_Byte4; /* Destination MAC address of Tx chain2 (byte 4) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR2_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR2_H_STRUC {
	struct {
		UINT8	TxChainAddr2H_Byte4;
		UINT8	TxChainAddr2H_Byte5;
		UINT16	TxChainSel0:4;
		UINT16	Reserved:12;
	} field;
	UINT32 word;
} TX_CHAIN_ADDR2_H_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_L_STRUC {
	struct {
		UINT8	TxChainAddr3L_Byte3; /* Destination MAC address of Tx chain3 (byte 3) */
		UINT8	TxChainAddr3L_Byte2; /* Destination MAC address of Tx chain3 (byte 2) */
		UINT8	TxChainAddr3L_Byte1; /* Destination MAC address of Tx chain3 (byte 1) */
		UINT8	TxChainAddr3L_Byte0; /* Destination MAC address of Tx chain3 (byte 0) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_L_STRUC {
	struct {
		UINT8	TxChainAddr3L_Byte0;
		UINT8	TxChainAddr3L_Byte1;
		UINT8	TxChainAddr3L_Byte2;
		UINT8	TxChainAddr3L_Byte3;
	} field;
	UINT32 word;
}	TX_CHAIN_ADDR3_L_STRUC, *PTX_CHAIN_ADDR3_L_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _TX_CHAIN_ADDR3_H_STRUC {
	struct {
		UINT16	Reserved:12; /* Reserved */
		UINT16	TxChainSel0:4; /* Selection value of Tx chain0 */
		UINT8	TxChainAddr3H_Byte5; /* Destination MAC address of Tx chain3 (byte 5) */
		UINT8	TxChainAddr3H_Byte4; /* Destination MAC address of Tx chain3 (byte 4) */
	} field;
	UINT32 word;
} TX_CHAIN_ADDR3_H_STRUC, *PTX_CHAIN_ADDR3_H_STRUC;
#else
typedef union _TX_CHAIN_ADDR3_H_STRUC {
	struct {
		UINT8	TxChainAddr3H_Byte4;
		UINT8	TxChainAddr3H_Byte5;
		UINT16	TxChainSel0:4;
		UINT16	Reserved:12;
	} field;
	UINT32 word;
} TX_CHAIN_ADDR3_H_STRUC;
#endif

#define TX_BCN_BYPASS_MASK          0x108C 
typedef union _BCN_BYPASS_MASK_STRUC {
        struct {
                UINT32 BeaconDropMask:16;  
        } field;
        UINT32 word;
} BCN_BYPASS_MASK_STRUC;

/*  4.2 MAC TIMING  configuration registers (offset:0x1100) */
#define XIFS_TIME_CFG             0x1100
#define XIFS_TIME_OFDM_SIFS_MASK (0xff << 8)
#define XIFS_TIME_OFDM_SIFS(p) (((p) & 0xff) << 8)
#ifdef RT_BIG_ENDIAN
typedef	union _IFS_SLOT_CFG_STRUC {
	struct {
	    UINT32  rsv:2;         
	    UINT32  BBRxendEnable:1;        /*  reference RXEND signal to begin XIFS defer */
	    UINT32  EIFS:9;        /*  unit 1us */
	    UINT32  OfdmXifsTime:4;        /*OFDM SIFS. unit 1us. Applied after OFDM RX when MAC doesn't reference BBP signal BBRXEND */
	    UINT32  OfdmSifsTime:8;        /*  unit 1us. Applied after OFDM RX/TX */
	    UINT32  CckmSifsTime:8;        /*  unit 1us. Applied after CCK RX/TX */
	} field;
	UINT32 word;
} IFS_SLOT_CFG_STRUC;
#else
typedef	union _IFS_SLOT_CFG_STRUC {
	struct {
	    UINT32  CckmSifsTime:8;
	    UINT32  OfdmSifsTime:8;
	    UINT32  OfdmXifsTime:4;
	    UINT32  EIFS:9;
	    UINT32  BBRxendEnable:1;
	    UINT32  rsv:2;         
	} field;
	UINT32 word;
} IFS_SLOT_CFG_STRUC;
#endif

#define BKOFF_SLOT_CFG		0x1104
#define BKOFF_SLOT_CFG_CC_DELAY_TIME_MASK (0x0f << 8)
#define BKOFF_SLOT_CFG_CC_DELAY_TIME(p) (((p) & 0x0f) << 8)

#define NAV_TIME_CFG		0x1108
#define CH_TIME_CFG			0x110C
#define PBF_LIFE_TIMER		0x1110	/*TX/RX MPDU timestamp timer (free run)Unit: 1us */


/* BCN_TIME_CFG : Synchronization control register */
#define BCN_TIME_CFG             0x1114
#ifdef RT_BIG_ENDIAN
typedef	union _BCN_TIME_CFG_STRUC {
	struct {
		UINT32 TxTimestampCompensate:8;
		UINT32 :3;
		UINT32 bBeaconGen:1;		/* Enable beacon generator */
		UINT32 bTBTTEnable:1;
		UINT32 TsfSyncMode:2;		/* Enable TSF sync, 00: disable, 01: infra mode, 10: ad-hoc mode */
		UINT32 bTsfTicking:1;		/* Enable TSF auto counting */
		UINT32 BeaconInterval:16;  /* in unit of 1/16 TU */
	} field;
	UINT32 word;
} BCN_TIME_CFG_STRUC;
#else
typedef union _BCN_TIME_CFG_STRUC {
	struct {
		UINT32 BeaconInterval:16;
		UINT32 bTsfTicking:1;
		UINT32 TsfSyncMode:2;
		UINT32 bTBTTEnable:1;
		UINT32 bBeaconGen:1;
		UINT32 :3;
		UINT32 TxTimestampCompensate:8;
	} field;
	UINT32 word;
} BCN_TIME_CFG_STRUC;
#endif


#define TBTT_SYNC_CFG		0x1118
#define TSF_TIMER_DW0		0x111c	/* Local TSF timer lsb 32 bits. Read-only */
#define TSF_TIMER_DW1		0x1120	/* msb 32 bits. Read-only. */
#define TBTT_TIMER			0x1124	/* TImer remains till next TBTT. Read-only */
#define INT_TIMER_CFG		0x1128
#define INT_TIMER_EN		0x112c	/* GP-timer and pre-tbtt Int enable */
#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2

#define CH_IDLE_STA			0x1130	/* channel idle time */
#define CH_BUSY_STA			0x1134	/* channle busy time */
#define CH_BUSY_STA_SEC	0x1138	/* channel busy time for secondary channel */


/*  4.2 MAC POWER  configuration registers (offset:0x1200) */
#define MAC_STATUS_CFG		0x1200
#define PWR_PIN_CFG		0x1204


/* AUTO_WAKEUP_CFG: Manual power control / status register */
#define AUTO_WAKEUP_CFG	0x1208
#ifdef RT_BIG_ENDIAN
typedef	union _AUTO_WAKEUP_STRUC {
	struct {
		UINT32 :16;
		UINT32 EnableAutoWakeup:1;	/* 0:sleep, 1:awake */
		UINT32 NumofSleepingTbtt:7;          /* ForceWake has high privilege than PutToSleep when both set */
		UINT32 AutoLeadTime:8;
	} field;
	UINT32 word;
} AUTO_WAKEUP_STRUC;
#else
typedef	union _AUTO_WAKEUP_STRUC {
	struct {
		UINT32 AutoLeadTime:8;
		UINT32 NumofSleepingTbtt:7;
		UINT32 EnableAutoWakeup:1;
		UINT32 :16;
	} field;
	UINT32 word;
} AUTO_WAKEUP_STRUC;
#endif

#define AUX_CLK_CFG			0x120C
#define MIMO_PS_CFG		0x1210



/*  4.3 MAC TX  configuration registers (offset:0x1300) */
#define EDCA_AC0_CFG	0x1300
#define EDCA_AC1_CFG	0x1304
#define EDCA_AC2_CFG	0x1308
#define EDCA_AC3_CFG	0x130c
#ifdef RT_BIG_ENDIAN
typedef	union _EDCA_AC_CFG_STRUC {
	struct {
	    UINT32 :12;
	    UINT32 Cwmax:4;	/* unit power of 2 */
	    UINT32 Cwmin:4;
	    UINT32 Aifsn:4;	/* # of slot time */
	    UINT32 AcTxop:8;	/*  in unit of 32us */
	} field;
	UINT32 word;
} EDCA_AC_CFG_STRUC;
#else
typedef	union _EDCA_AC_CFG_STRUC {
	struct {
	    UINT32 AcTxop:8;
	    UINT32 Aifsn:4;
	    UINT32 Cwmin:4;
	    UINT32 Cwmax:4;
	    UINT32 :12;
	} field;
	UINT32 word;
} EDCA_AC_CFG_STRUC;
#endif

#define EDCA_TID_AC_MAP	0x1310


/* Default Tx power */
#define DEFAULT_TX_POWER	0x6

#define TX_PWR_CFG_0		0x1314
#define TX_PWR_CCK_1_2_MASK (0x3f)
#define TX_PWR_CCK_1_2(p) (((p) & 0x3f))
#define TX_PWR_CCK_5_11_MASK (0x3f << 8)
#define TX_PWR_CCK_5_11(p) (((p) & 0x3f) << 8)
#define TX_PWR_OFDM_6_9_MASK (0x3f << 16)
#define TX_PWR_OFDM_6_9(p) (((p) & 0x3f) << 16)
#define TX_PWR_OFDM_12_18_MASK (0x3f << 24)
#define TX_PWR_OFDM_12_18(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_0_EXT	0x1390

#define TX_PWR_CFG_1		0x1318
#define TX_PWR_OFDM_24_36_MASK (0x3f)
#define TX_PWR_OFDM_24_36(p) (((p) & 0x3f))
#define TX_PWR_OFDM_48_MASK (0x3f << 8)
#define TX_PWR_OFDM_48(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_VHT_1SS_MCS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_VHT_1SS_MCS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_VHT_1SS_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_VHT_1SS_MCS_2_3(p) (((p) & 0x3f) << 16)

#define TX_PWR_CFG_1_EXT	0x1394

#define TX_PWR_CFG_2		0x131C
#define TX_PWR_HT_VHT_1SS_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_VHT_1SS_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_VHT_1SS_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_VHT_1SS_MCS_6(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_MCS_8_9_VHT_2SS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_MCS_8_9_VHT_2SS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_MCS_10_11_VHT_2SS_MCS_2_3(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_2_EXT	0x1398

#define TX_PWR_CFG_3		0x1320
#define TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_MCS_12_13_VHT_2SS_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_MCS_14_VHT_2SS_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_14_VHT_2SS_MCS_6(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_VHT_STBC_MCS_0_1_MASK (0x3f << 16)
#define TX_PWR_HT_VHT_STBC_MCS_0_1(p) (((p) & 0x3f) << 16)
#define TX_PWR_HT_VHT_STBC_MCS_2_3_MASK (0x3f << 24)
#define TX_PWR_HT_VHT_STBC_MCS_2_3(p) (((p) & 0x3f) << 24)

#define GF20_PORT_CFG       0x1374
#define TX_PWR_CFG_3_EXT	0x139C

#define TX_PWR_CFG_4		0x1324
#define TX_PWR_HT_VHT_STBC_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_VHT_STBC_MCS_4_5(p) (((p) & 0x3f))
#define TX_PWR_HT_VHT_STBC_MCS_6_MASK (0x3f << 8)
#define TX_PWR_HT_VHT_STBC_MCS_6(p) (((p) & 0x3f) << 8)

#define TX_PWR_CFG_0_EXT	0x1390
#define TX_PWR_CFG_1_EXT	0x1394
#define TX_PWR_CFG_2_EXT	0x1398
#define TX_PWR_CFG_3_EXT	0x139C
#define TX_PWR_CFG_4_EXT	0x13A0

#define TX_PWR_CFG_5		0x1384
#define TX_PWR_CFG_6		0x1388

#define TX_PWR_CFG_7		0x13D4
#define TX_PWR_OFDM_54_MASK (0x3f)
#define TX_PWR_OFDM_54(p) (((p) & 0x3f))
#define TX_PWR_VHT_2SS_MCS_8_MASK (0x3f << 8)
#define TX_PWR_VHT_2SS_MCS_8(p) (((p) & 0x3f) << 8)
#define TX_PWR_HT_MCS_7_VHT_1SS_MCS_7_MASK (0x3f << 16)
#define TX_PWR_HT_MCS_7_VHT_1SS_MCS_7(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_2SS_MCS_9_MASK (0X3f << 24)
#define TX_PWR_VHT_2SS_MCS_9(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_8		0x13D8
#define TX_PWR_HT_MCS_15_VHT_2SS_MCS7_MASK (0x3f)
#define TX_PWR_HT_MCS_15_VHT_2SS_MCS7(p) (((p) & 0x3f))
#define TX_PWR_VHT_1SS_MCS_8_MASK (0x3f << 16)
#define TX_PWR_VHT_1SS_MCS_8(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_1SS_MCS_9_MASK (0X3f << 24)
#define TX_PWR_VHT_1SS_MCS_9(p) (((p) & 0x3f) << 24)

#define TX_PWR_CFG_9		0x13DC
#define TX_PWR_HT_VHT_STBC_MCS_7_MASK (0x3f)
#define TX_PWR_HT_VHT_STBC_MCS_7(p) (((p) & 0x3f))
#define TX_PWR_VHT_STBC_MCS_8_MASK (0x3f << 16)
#define TX_PWR_VHT_STBC_MCS_8(p) (((p) & 0x3f) << 16)
#define TX_PWR_VHT_STBC_MCS_9_MASK (0x3f << 24)
#define TX_PWR_VHT_STBC_MCS_9(p) (((p) & 0x3f) << 24)

#ifdef RT_BIG_ENDIAN
typedef	union _TX_PWR_CFG_STRUC {
	struct {
	    UINT32 Byte3:8;
	    UINT32 Byte2:8;
	    UINT32 Byte1:8;
	    UINT32 Byte0:8;
	} field;
	UINT32 word;
} TX_PWR_CFG_STRUC;
#else
typedef	union _TX_PWR_CFG_STRUC {
	struct {
	    UINT32 Byte0:8;
	    UINT32 Byte1:8;
	    UINT32 Byte2:8;
	    UINT32 Byte3:8;
	} field;
	UINT32 word;
} TX_PWR_CFG_STRUC;
#endif


#define TX_PIN_CFG		0x1328		 
#define TX_BAND_CFG	0x132c	/* 0x1 use upper 20MHz. 0 juse lower 20MHz */
#define TX_SW_CFG0		0x1330
#define TX_SW_CFG1		0x1334
#define TX_SW_CFG2		0x1338


#define TXOP_THRES_CFG	0x133c
#ifdef RT_BIG_ENDIAN
typedef union _TXOP_THRESHOLD_CFG_STRUC {
	struct {
		UINT32	TXOP_REM_THRES:8; /* Remaining TXOP threshold (unit: 32us) */
		UINT32	CF_END_THRES:8; /* CF-END threshold (unit: 32us) */
		UINT32	RDG_IN_THRES:8; /* Rx RDG threshold (unit: 32us) */
		UINT32	RDG_OUT_THRES:8; /* Tx RDG threshold (unit: 32us) */
	} field;
	UINT32 word;
} TXOP_THRESHOLD_CFG_STRUC;
#else
typedef union _TXOP_THRESHOLD_CFG_STRUC {
	struct {
		UINT32	RDG_OUT_THRES:8;
		UINT32	RDG_IN_THRES:8;
		UINT32	CF_END_THRES:8;
		UINT32	TXOP_REM_THRES:8;
	} field;
	UINT32 word;
} TXOP_THRESHOLD_CFG_STRUC;
#endif

#define TXOP_CTRL_CFG 0x1340


#define TX_RTS_CFG 0x1344
#ifdef RT_BIG_ENDIAN
typedef	union _TX_RTS_CFG_STRUC {
	struct {
	    UINT32 rsv:7;     
	    UINT32 RtsFbkEn:1;    /* enable rts rate fallback */
	    UINT32 RtsThres:16;    /* unit:byte */
	    UINT32 AutoRtsRetryLimit:8;
	} field;
	UINT32 word;
} TX_RTS_CFG_STRUC;
#else
typedef	union _TX_RTS_CFG_STRUC	 {
	struct {
	    UINT32 AutoRtsRetryLimit:8;
	    UINT32 RtsThres:16;
	    UINT32 RtsFbkEn:1;
	    UINT32 rsv:7;
	} field;
	UINT32 word;
} TX_RTS_CFG_STRUC;
#endif



#define TX_TXBF_CFG_0 0x138c
#define TX_TXBF_CFG_1 0x13A4
#define TX_TXBF_CFG_2 0x13A8
#define TX_TXBF_CFG_3 0x13AC
typedef	union _TX_TXBF_CFG_0_STRUC {
	struct {
#ifdef RT_BIG_ENDIAN
	    UINT32       EtxbfFbkRate:16;     
	    UINT32       EtxbfFbkEn:1;
	    UINT32       EtxbfFbkSeqEn:1;
	    UINT32       EtxbfFbkCoef:2;
	    UINT32       EtxbfFbkCode:2;
	    UINT32       EtxbfFbkNg:2;
	    UINT32       CsdBypass:1;
	    UINT32       EtxbfForce:1;
	    UINT32       EtxbfEnable:1;
	    UINT32       AutoTxbfEn:3;
	    UINT32       ItxbfForce:1;
	    UINT32       ItxbfEn:1;
#else
	    UINT32       ItxbfEn:1;
	    UINT32       ItxbfForce:1;
	    UINT32       AutoTxbfEn:3;
	    UINT32       EtxbfEnable:1;
	    UINT32       EtxbfForce:1;
	    UINT32       CsdBypass:1;
	    UINT32       EtxbfFbkNg:2;
	    UINT32       EtxbfFbkCode:2;
	    UINT32       EtxbfFbkCoef:2;
	    UINT32       EtxbfFbkSeqEn:1;
	    UINT32       EtxbfFbkEn:1;
	    UINT32       EtxbfFbkRate:16;     
#endif
	} field;
	UINT32 word;
} TX_TXBF_CFG_0_STRUC;


#define TX_TIMEOUT_CFG	0x1348
#ifdef RT_BIG_ENDIAN
typedef	union _TX_TIMEOUT_CFG_STRUC {
	struct {
	    UINT32 rsv2:8;     
	    UINT32 TxopTimeout:8;	/*TXOP timeout value for TXOP truncation.  It is recommended that (SLOT_TIME) > (TX_OP_TIMEOUT) > (RX_ACK_TIMEOUT) */
	    UINT32 RxAckTimeout:8;	/* unit:slot. Used for TX precedure */
	    UINT32 MpduLifeTime:4;    /*  expiration time = 2^(9+MPDU LIFE TIME)  us */
	    UINT32 rsv:4;     
	} field;
	UINT32 word;
} TX_TIMEOUT_CFG_STRUC;
#else
typedef	union _TX_TIMEOUT_CFG_STRUC {
	struct {
	    UINT32 rsv:4;     
	    UINT32 MpduLifeTime:4;
	    UINT32 RxAckTimeout:8;
	    UINT32 TxopTimeout:8;
	    UINT32 rsv2:8;
	} field;
	UINT32 word;
} TX_TIMEOUT_CFG_STRUC;
#endif


#define TX_RTY_CFG	0x134c
#define TX_RTY_CFG_RTY_LIMIT_SHORT	0x1
#define TX_RTY_CFG_RTY_LIMIT_LONG		0x2

#ifdef RT_BIG_ENDIAN
typedef	union _TX_RTY_CFG_STRUC {
	struct {
	    UINT32 rsv:1;     
	    UINT32 TxautoFBEnable:1;    /* Tx retry PHY rate auto fallback enable */
	    UINT32 AggRtyMode:1;	/* Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer */
	    UINT32 NonAggRtyMode:1;	/* Non-Aggregate MPDU retry mode.  0:expired by retry limit, 1: expired by mpdu life timer */
	    UINT32 LongRtyThre:12;	/* Long retry threshoold */
	    UINT32 LongRtyLimit:8;	/*long retry limit */
	    UINT32 ShortRtyLimit:8;	/*  short retry limit */
	} field;
	UINT32 word;
} TX_RTY_CFG_STRUC;
#else
typedef	union _TX_RTY_CFG_STRUC {
	struct {
	    UINT32 ShortRtyLimit:8;
	    UINT32 LongRtyLimit:8;
	    UINT32 LongRtyThre:12;
	    UINT32 NonAggRtyMode:1;
	    UINT32 AggRtyMode:1;
	    UINT32 TxautoFBEnable:1;
	    UINT32 rsv:1;
	} field;
	UINT32 word;
} TX_RTY_CFG_STRUC;
#endif


#define TX_LINK_CFG	0x1350
#ifdef RT_BIG_ENDIAN
typedef	union _TX_LINK_CFG_STRUC {
	struct {
	    UINT32       RemotMFS:8;	/*remote MCS feedback sequence number */
	    UINT32       RemotMFB:8;    /*  remote MCS feedback */
	    UINT32       rsv:3;	/* */
	    UINT32       TxCFAckEn:1;	/*   Piggyback CF-ACK enable */
	    UINT32       TxRDGEn:1;	/* RDG TX enable */
	    UINT32       TxMRQEn:1;	/*  MCS request TX enable */
	    UINT32       RemoteUMFSEnable:1;	/*  remote unsolicit  MFB enable.  0: not apply remote remote unsolicit (MFS=7) */
	    UINT32       MFBEnable:1;	/*  TX apply remote MFB 1:enable */
	    UINT32       RemoteMFBLifeTime:8;	/*remote MFB life time. unit : 32us */
	} field;
	UINT32 word;
} TX_LINK_CFG_STRUC;
#else
typedef	union _TX_LINK_CFG_STRUC {
	struct {
	    UINT32       RemoteMFBLifeTime:8;
	    UINT32       MFBEnable:1;
	    UINT32       RemoteUMFSEnable:1;
	    UINT32       TxMRQEn:1;
	    UINT32       TxRDGEn:1;
	    UINT32       TxCFAckEn:1;
	    UINT32       rsv:3;
	    UINT32       RemotMFB:8;
	    UINT32       RemotMFS:8;
	} field;
	UINT32 word;
} TX_LINK_CFG_STRUC;
#endif


#define HT_FBK_CFG0	0x1354
#ifdef RT_BIG_ENDIAN
typedef	union _HT_FBK_CFG0_STRUC {
	struct {
	    UINT32 HTMCS7FBK:4;
	    UINT32 HTMCS6FBK:4;
	    UINT32 HTMCS5FBK:4;
	    UINT32 HTMCS4FBK:4;
	    UINT32 HTMCS3FBK:4;
	    UINT32 HTMCS2FBK:4;
	    UINT32 HTMCS1FBK:4;
	    UINT32 HTMCS0FBK:4;
	} field;
	UINT32 word;
} HT_FBK_CFG0_STRUC;
#else
typedef	union _HT_FBK_CFG0_STRUC {
	struct {
	    UINT32 HTMCS0FBK:4;
	    UINT32 HTMCS1FBK:4;
	    UINT32 HTMCS2FBK:4;
	    UINT32 HTMCS3FBK:4;
	    UINT32 HTMCS4FBK:4;
	    UINT32 HTMCS5FBK:4;
	    UINT32 HTMCS6FBK:4;
	    UINT32 HTMCS7FBK:4;
	} field;
	UINT32 word;
} HT_FBK_CFG0_STRUC;
#endif


#define HT_FBK_CFG1	0x1358
#ifdef RT_BIG_ENDIAN
typedef	union _HT_FBK_CFG1_STRUC {
	struct {
	    UINT32       HTMCS15FBK:4;
	    UINT32       HTMCS14FBK:4;
	    UINT32       HTMCS13FBK:4;
	    UINT32       HTMCS12FBK:4;
	    UINT32       HTMCS11FBK:4;
	    UINT32       HTMCS10FBK:4;
	    UINT32       HTMCS9FBK:4;
	    UINT32       HTMCS8FBK:4;
	} field;
	UINT32 word;
} HT_FBK_CFG1_STRUC;
#else
typedef	union _HT_FBK_CFG1_STRUC {
	struct {
	    UINT32       HTMCS8FBK:4;
	    UINT32       HTMCS9FBK:4;
	    UINT32       HTMCS10FBK:4;
	    UINT32       HTMCS11FBK:4;
	    UINT32       HTMCS12FBK:4;
	    UINT32       HTMCS13FBK:4;
	    UINT32       HTMCS14FBK:4;
	    UINT32       HTMCS15FBK:4;
	} field;
	UINT32 word;
} HT_FBK_CFG1_STRUC;
#endif


#define LG_FBK_CFG0	0x135c
#ifdef RT_BIG_ENDIAN
typedef	union _LG_FBK_CFG0_STRUC {
	struct {
	    UINT32       OFDMMCS7FBK:4;
	    UINT32       OFDMMCS6FBK:4;
	    UINT32       OFDMMCS5FBK:4;
	    UINT32       OFDMMCS4FBK:4;
	    UINT32       OFDMMCS3FBK:4;
	    UINT32       OFDMMCS2FBK:4;
	    UINT32       OFDMMCS1FBK:4;
	    UINT32       OFDMMCS0FBK:4;
	} field;
	UINT32 word;
} LG_FBK_CFG0_STRUC;
#else
typedef	union _LG_FBK_CFG0_STRUC {
	struct {
	    UINT32       OFDMMCS0FBK:4;
	    UINT32       OFDMMCS1FBK:4;
	    UINT32       OFDMMCS2FBK:4;
	    UINT32       OFDMMCS3FBK:4;
	    UINT32       OFDMMCS4FBK:4;
	    UINT32       OFDMMCS5FBK:4;
	    UINT32       OFDMMCS6FBK:4;
	    UINT32       OFDMMCS7FBK:4;
	} field;
	UINT32 word;
} LG_FBK_CFG0_STRUC;
#endif


#define LG_FBK_CFG1		0x1360
#ifdef RT_BIG_ENDIAN
typedef	union _LG_FBK_CFG1_STRUC {
	struct {
	    UINT32       rsv:16;
	    UINT32       CCKMCS3FBK:4;
	    UINT32       CCKMCS2FBK:4;
	    UINT32       CCKMCS1FBK:4;
	    UINT32       CCKMCS0FBK:4;
	} field;
	UINT32 word;
}	LG_FBK_CFG1_STRUC;
#else
typedef	union _LG_FBK_CFG1_STRUC {
	struct {
	    UINT32       CCKMCS0FBK:4;
	    UINT32       CCKMCS1FBK:4;
	    UINT32       CCKMCS2FBK:4;
	    UINT32       CCKMCS3FBK:4;
	    UINT32       rsv:16;
	}	field;
	UINT32 word;
}	LG_FBK_CFG1_STRUC;
#endif


/*======================================================= */
/*                                     Protection Paramater                                                         */
/*======================================================= */
#define ASIC_SHORTNAV	1
#define ASIC_LONGNAV	2
#define ASIC_RTS		1
#define ASIC_CTS		2

#define CCK_PROT_CFG	0x1364	/* CCK Protection */
#define OFDM_PROT_CFG	0x1368	/* OFDM Protection */
#define MM20_PROT_CFG	0x136C	/* MM20 Protection */
#define MM40_PROT_CFG	0x1370	/* MM40 Protection */
#define GF20_PROT_CFG	0x1374	/* GF20 Protection */
#define GF40_PROT_CFG	0x1378	/* GR40 Protection */
#ifdef RT_BIG_ENDIAN
typedef	union _PROT_CFG_STRUC {
	struct {
#ifdef DOT11_VHT_AC
		UINT32		 ProtectTxop:3; /* TXOP allowance */
		UINT32       DynCbw:1;  /* RTS use dynamic channel bandwidth when TX signaling mode is turned on */
		UINT32       RtsTaSignal:1; /* RTS TA signaling mode */
#else
	    UINT32       rsv:5;	 
#endif
	    UINT32       RTSThEn:1;	/*RTS threshold enable on CCK TX */
	    UINT32       TxopAllowGF40:1;	/*CCK TXOP allowance.0:disallow. */
	    UINT32       TxopAllowGF20:1;	/*CCK TXOP allowance.0:disallow. */
	    UINT32       TxopAllowMM40:1;	/*CCK TXOP allowance.0:disallow. */
	    UINT32       TxopAllowMM20:1;	/*CCK TXOP allowance. 0:disallow. */
	    UINT32       TxopAllowOfdm:1;	/*CCK TXOP allowance.0:disallow. */
	    UINT32       TxopAllowCck:1;	/*CCK TXOP allowance.0:disallow. */
	    UINT32       ProtectNav:2;	/*TXOP protection type for CCK TX. 0:None, 1:ShortNAVprotect,  2:LongNAVProtect, 3:rsv */
	    UINT32       ProtectCtrl:2;	/*Protection control frame type for CCK TX. 1:RTS/CTS, 2:CTS-to-self, 0:None, 3:rsv */
	    UINT32       ProtectRate:16;	/*Protection control frame rate for CCK TX(RTS/CTS/CFEnd). */
	} field;
	UINT32 word;
} PROT_CFG_STRUC;
#else
typedef	union _PROT_CFG_STRUC {
	struct {
	    UINT32       ProtectRate:16;
	    UINT32       ProtectCtrl:2;
	    UINT32       ProtectNav:2;
	    UINT32       TxopAllowCck:1;
	    UINT32       TxopAllowOfdm:1;
	    UINT32       TxopAllowMM20:1;
	    UINT32       TxopAllowMM40:1;
	    UINT32       TxopAllowGF20:1;
	    UINT32       TxopAllowGF40:1;
	    UINT32       RTSThEn:1;
#ifdef DOT11_VHT_AC
		UINT32       RtsTaSignal:1; /* RTS TA signaling mode */
		UINT32       DynCbw:1;  /* RTS use dynamic channel bandwidth when TX signaling mode is turned on */
		UINT32		 ProtectTxop:3; /* TXOP allowance */
#else
	    UINT32       rsv:5;	 
#endif /* !DOT11_VHT_AC */
	} field;
	UINT32 word;
} PROT_CFG_STRUC;
#endif

#define EXP_CTS_TIME	0x137C
#define EXP_ACK_TIME	0x1380


#define HT_FBK_TO_LEGACY	0x1384

#define TX_FBK_LIMIT		0x1398

#ifdef DOT11N_SS3_SUPPORT
#define TX_FBK_CFG_3S_0	0x13c4
#ifdef RT_BIG_ENDIAN
typedef	union _TX_FBK_CFG_3S_0_STRUC {
	struct {
		UINT32       rsv0:3;
		UINT32       HTMCS19FBK:5;
		UINT32       rsv1:3;
		UINT32       HTMCS18FBK:5;
		UINT32       rsv2:3;
		UINT32       HTMCS17FBK:5;
		UINT32       rsv3:3;
		UINT32       HTMCS16FBK:5;
	} field;
	UINT32 word;
} TX_FBK_CFG_3S_0_STRUC;
#else
typedef	union _TX_FBK_CFG_3S_0_STRUC {
	struct {
		UINT32       HTMCS16FBK:5;
		UINT32       rsv3:3;
		UINT32       HTMCS17FBK:5;
		UINT32       rsv2:3;
		UINT32       HTMCS18FBK:5;
		UINT32       rsv1:3;
		UINT32       HTMCS19FBK:5;
		UINT32       rsv0:4;
	} field;
	UINT32 word;
} TX_FBK_CFG_3S_0_STRUC;
#endif

#define TX_FBK_CFG_3S_1	0x13c8
#ifdef RT_BIG_ENDIAN
typedef	union _TX_FBK_CFG_3S_1_STRUC {
	struct {
		UINT32       rsv0:3;
		UINT32       HTMCS23FBK:5;
		UINT32       rsv1:3;
		UINT32       HTMCS22FBK:5;
		UINT32       rsv2:3;
		UINT32       HTMCS21FBK:5;
		UINT32       rsv3:3;
		UINT32       HTMCS20FBK:5;
	} field;
	UINT32 word;
} TX_FBK_CFG_3S_1_STRUC;
#else
typedef	union _TX_FBK_CFG_3S_1_STRUC {
	struct {
		UINT32       HTMCS20FBK:5;
		UINT32       rsv3:3;
		UINT32       HTMCS21FBK:5;
		UINT32       rsv2:3;
		UINT32       HTMCS22FBK:5;
		UINT32       rsv1:3;
		UINT32       HTMCS23FBK:5;
		UINT32       rsv0:3;
	} field;
	UINT32 word;
} TX_FBK_CFG_3S_1_STRUC;
#endif
#endif /* DOT11N_SS3_SUPPORT */

#define TX_AC_RTY_LIMIT		0x13cc
#define TX_AC_FBK_SPEED	0x13d0



/*  4.4 MAC RX configuration registers (offset:0x1400) */

/* RX_FILTR_CFG:  RX configuration register */
#define RX_FILTR_CFG	0x1400
#ifdef RT_BIG_ENDIAN
typedef	union _RX_FILTR_CFG_STRUC {
	struct {
		UINT32 rsv:15;		
		UINT32 DropRsvCntlType:1;
        	UINT32 DropBAR:1;
		UINT32 DropBA:1;	
		UINT32 DropPsPoll:1;		/* Drop Ps-Poll */
		UINT32 DropRts:1;		/* Drop Ps-Poll */
		UINT32 DropCts:1;		/* Drop Ps-Poll */
		UINT32 DropAck:1;		/* Drop Ps-Poll */
		UINT32 DropCFEnd:1;		/* Drop Ps-Poll */
		UINT32 DropCFEndAck:1;		/* Drop Ps-Poll */
		UINT32 DropDuplicate:1;		/* Drop duplicate frame */
		UINT32 DropBcast:1;		/* Drop broadcast frames */
		UINT32 DropMcast:1;		/* Drop multicast frames */
		UINT32 DropVerErr:1;	    /* Drop version error frame */
		UINT32 DropNotMyBSSID:1;			/* Drop fram ToDs bit is true */
		UINT32 DropNotToMe:1;		/* Drop not to me unicast frame */
		UINT32 DropPhyErr:1;		/* Drop physical error */
		UINT32 DropCRCErr:1;		/* Drop CRC error */
	} field;
	UINT32 word;
} RX_FILTR_CFG_STRUC;
#else
typedef	union _RX_FILTR_CFG_STRUC {
	struct {
		UINT32 DropCRCErr:1;
		UINT32 DropPhyErr:1;
		UINT32 DropNotToMe:1;
		UINT32 DropNotMyBSSID:1;
		UINT32 DropVerErr:1;
		UINT32 DropMcast:1;
		UINT32 DropBcast:1;
		UINT32 DropDuplicate:1;
		UINT32 DropCFEndAck:1;
		UINT32 DropCFEnd:1;
		UINT32 DropAck:1;
		UINT32 DropCts:1;
		UINT32 DropRts:1;
		UINT32 DropPsPoll:1;
		UINT32 DropBA:1;
        	UINT32  DropBAR:1;
		UINT32 DropRsvCntlType:1;
		UINT32 rsv:15;
	} field;
	UINT32 word;
}	RX_FILTR_CFG_STRUC;
#endif


/* AUTO_RSP_CFG: Auto-Responder */
#define AUTO_RSP_CFG	0x1404
#ifdef RT_BIG_ENDIAN
typedef union _AUTO_RSP_CFG_STRUC {
	struct {
		UINT32        :24;    
		UINT32       AckCtsPsmBit:1;   /* Power bit value in conrtrol frame */
		UINT32       DualCTSEn:1;   /* Power bit value in conrtrol frame */
		UINT32       rsv:1;   /* Power bit value in conrtrol frame */
		UINT32       AutoResponderPreamble:1;    /* 0:long, 1:short preamble */
		UINT32       CTS40MRef:1;  /* Response CTS 40MHz duplicate mode */
		UINT32       CTS40MMode:1;  /* Response CTS 40MHz duplicate mode */
		UINT32       BACAckPolicyEnable:1;    /* 0:long, 1:short preamble */
		UINT32       AutoResponderEnable:1;
	} field;
	UINT32 word;
} AUTO_RSP_CFG_STRUC;
#else
typedef union _AUTO_RSP_CFG_STRUC {
	struct {
		UINT32       AutoResponderEnable:1;
		UINT32       BACAckPolicyEnable:1;
		UINT32       CTS40MMode:1;
		UINT32       CTS40MRef:1;
		UINT32       AutoResponderPreamble:1;
		UINT32       rsv:1;
		UINT32       DualCTSEn:1;
		UINT32       AckCtsPsmBit:1;
		UINT32        :24;    
	} field;
	UINT32   word;
} AUTO_RSP_CFG_STRUC;
#endif


#define LEGACY_BASIC_RATE	0x1408
#define HT_BASIC_RATE		0x140c
#define HT_CTRL_CFG			0x1410
#define SIFS_COST_CFG		0x1414
#define RX_PARSER_CFG		0x1418	/*Set NAV for all received frames */

#define EXT_CCA_CFG			0x141c

#ifdef MAC_APCLI_SUPPORT
#define APCLI_BSSID_IDX			8
#define MAC_APCLI_BSSID_DW0		0x1090
#define MAC_APCLI_BSSID_DW1		0x1094
#endif /* MAC_APCLI_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#define MAC_ADDR_EXT_EN			0x147C
#define MAC_ADDR_EXT0_31_0		0x1480
#define MAC_ADDR_EXT0_47_32		0x1484
#define MAX_EXT_MAC_ADDR_SIZE	16

#define UNKOWN_APCLI_IDX		0xFF
#define CLIENT_APCLI			0x00
#define CLIENT_STA				0x01
#define CLIENT_ETH				0x02
#define EXTERNDER_CLI			0x08
#endif /* MAC_REPEATER_SUPPORT */


/*  4.5 MAC Security configuration (offset:0x1500) */
#define TX_SEC_CNT0		0x1500
#define RX_SEC_CNT0		0x1504
#define CCMP_FC_MUTE	0x1508


/*  4.6 HCCA/PSMP (offset:0x1600) */
#define TXOP_HLDR_ADDR0		0x1600		 
#define TXOP_HLDR_ADDR1		0x1604		 
#define TXOP_HLDR_ET			0x1608		 
#define QOS_CFPOLL_RA_DW0		0x160c
#define QOS_CFPOLL_A1_DW1		0x1610
#define QOS_CFPOLL_QC			0x1614


/*  4.7 MAC Statistis registers (offset:0x1700) */
/* RX_STA_CNT0_STRUC: RX PLCP error count & RX CRC error count */
#define RX_STA_CNT0		0x1700
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT0_STRUC {
	struct {
	    UINT16  PhyErr;
	    UINT16  CrcErr;
	} field;
	UINT32 word;
} RX_STA_CNT0_STRUC;
#else
typedef	union _RX_STA_CNT0_STRUC {
	struct {
	    UINT16  CrcErr;
	    UINT16  PhyErr;
	} field;
	UINT32 word;
} RX_STA_CNT0_STRUC;
#endif


/* RX_STA_CNT1_STRUC: RX False CCA count & RX LONG frame count */
#define RX_STA_CNT1		0x1704
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT1_STRUC {
	struct {
	    UINT16  PlcpErr;
	    UINT16  FalseCca;
	} field;
	UINT32 word;
} RX_STA_CNT1_STRUC;
#else
typedef	union _RX_STA_CNT1_STRUC {
	struct {
	    UINT16  FalseCca;
	    UINT16  PlcpErr;
	} field;
	UINT32 word;
} RX_STA_CNT1_STRUC;
#endif


/* RX_STA_CNT2_STRUC: */
#define RX_STA_CNT2		0x1708
#ifdef RT_BIG_ENDIAN
typedef	union _RX_STA_CNT2_STRUC {
	struct {
	    UINT16  RxFifoOverflowCount;
	    UINT16  RxDupliCount;
	} field;
	UINT32 word;
} RX_STA_CNT2_STRUC;
#else
typedef	union _RX_STA_CNT2_STRUC {
	struct {
	    UINT16  RxDupliCount;
	    UINT16  RxFifoOverflowCount;
	} field;
	UINT32 word;
} RX_STA_CNT2_STRUC;
#endif

#ifndef MT_MAC
/* STA_CSR3: TX Beacon count */
#define TX_STA_CNT0		0x170C
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT0_STRUC {
	struct {
	    UINT16  TxBeaconCount;
	    UINT16  TxFailCount;
	} field;
	UINT32 word;
} TX_STA_CNT0_STRUC;
#else
typedef	union _TX_STA_CNT0_STRUC {
	struct {
	    UINT16  TxFailCount;
	    UINT16  TxBeaconCount;
	} field;
	UINT32 word;
} TX_STA_CNT0_STRUC;
#endif



/* TX_STA_CNT1: TX tx count */
#define TX_STA_CNT1		0x1710
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT1_STRUC {
	struct {
	    UINT16  TxRetransmit;
	    UINT16  TxSuccess;
	} field;
	UINT32 word;
} TX_STA_CNT1_STRUC;
#else
typedef	union _TX_STA_CNT1_STRUC {
	struct {
	    UINT16  TxSuccess;
	    UINT16  TxRetransmit;
	} field;
	UINT32 word;
} TX_STA_CNT1_STRUC;
#endif


/* TX_STA_CNT2: TX tx count */
#define TX_STA_CNT2		0x1714
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_CNT2_STRUC {
	struct {
	    UINT16  TxUnderFlowCount;
	    UINT16  TxZeroLenCount;
	} field;
	UINT32 word;
} TX_STA_CNT2_STRUC;
#else
typedef	union _TX_STA_CNT2_STRUC {
	struct {
	    UINT16  TxZeroLenCount;
	    UINT16  TxUnderFlowCount;
	} field;
	UINT32 word;
} TX_STA_CNT2_STRUC;
#endif


/* TX_STA_FIFO_STRUC: TX Result for specific PID status fifo register */
#define TX_STA_FIFO		0x1718
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_FIFO_STRUC {
	struct {
		UINT32		Reserve:2;
		UINT32		iTxBF:1; /* iTxBF enable */
		UINT32		Sounding:1; /* Sounding enable */
		UINT32		eTxBF:1; /* eTxBF enable */
		UINT32		SuccessRate:11;	/*include MCS, mode ,shortGI, BW settingSame format as TXWI Word 0 Bit 31-16. */
		UINT32		wcid:8;		/*wireless client index */
		UINT32       	TxAckRequired:1;    /* ack required */
		UINT32       	TxAggre:1;    /* Tx is aggregated */
		UINT32       	TxSuccess:1;   /* Tx success. whether success or not */
		UINT32       	PidType:4;
		UINT32       	bValid:1;   /* 1:This register contains a valid TX result */
	} field;
	UINT32 word;
} TX_STA_FIFO_STRUC;
#else
typedef	union _TX_STA_FIFO_STRUC {
	struct {
		UINT32       	bValid:1;
		UINT32       	PidType:4;
		UINT32       	TxSuccess:1;
		UINT32       	TxAggre:1;
		UINT32       	TxAckRequired:1;
		UINT32		wcid:8;
		UINT32		SuccessRate:11;
		UINT32		eTxBF:1;
		UINT32		Sounding:1;
		UINT32		iTxBF:1;
		UINT32		Reserve:2;
	} field;
	UINT32 word;
} TX_STA_FIFO_STRUC;
#endif
#endif /* !MT_MAC */

/* 
	Debug counters
*/
#define TX_AGG_CNT		0x171c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_NAG_AGG_CNT_STRUC {
	struct {
	    UINT16  AggTxCount;
	    UINT16  NonAggTxCount;
	} field;
	UINT32 word;
} TX_NAG_AGG_CNT_STRUC;
#else
typedef	union _TX_NAG_AGG_CNT_STRUC {
	struct {
	    UINT16  NonAggTxCount;
	    UINT16  AggTxCount;
	} field;
	UINT32 word;
} TX_NAG_AGG_CNT_STRUC;
#endif


#define TX_AGG_CNT0	0x1720
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT0_STRUC {
	struct {
	    UINT16  AggSize2Count;
	    UINT16  AggSize1Count;
	} field;
	UINT32 word;
} TX_AGG_CNT0_STRUC;
#else
typedef	union _TX_AGG_CNT0_STRUC {
	struct {
	    UINT16  AggSize1Count;
	    UINT16  AggSize2Count;
	} field;
	UINT32 word;
} TX_AGG_CNT0_STRUC;
#endif


#define TX_AGG_CNT1	0x1724
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT1_STRUC {
	struct {
	    UINT16  AggSize4Count;
	    UINT16  AggSize3Count;
	} field;
	UINT32 word;
} TX_AGG_CNT1_STRUC;
#else
typedef	union _TX_AGG_CNT1_STRUC {
	struct {
	    UINT16  AggSize3Count;
	    UINT16  AggSize4Count;
	} field;
	UINT32 word;
} TX_AGG_CNT1_STRUC;
#endif


#define TX_AGG_CNT2	0x1728
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT2_STRUC {
	struct {
	    UINT16  AggSize6Count;
	    UINT16  AggSize5Count;
	} field;
	UINT32 word;
} TX_AGG_CNT2_STRUC;
#else
typedef	union _TX_AGG_CNT2_STRUC {
	struct {
	    UINT16  AggSize5Count;
	    UINT16  AggSize6Count;
	} field;
	UINT32 word;
} TX_AGG_CNT2_STRUC;
#endif


#define TX_AGG_CNT3	0x172c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT3_STRUC {
	struct {
	    UINT16  AggSize8Count;
	    UINT16  AggSize7Count;
	} field;
	UINT32 word;
} TX_AGG_CNT3_STRUC;
#else
typedef	union _TX_AGG_CNT3_STRUC {
	struct {
	    UINT16  AggSize7Count;
	    UINT16  AggSize8Count;
	} field;
	UINT32 word;
} TX_AGG_CNT3_STRUC;
#endif


#define TX_AGG_CNT4	0x1730
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT4_STRUC {
	struct {
	    UINT16  AggSize10Count;
	    UINT16  AggSize9Count;
	} field;
	UINT32 word;
} TX_AGG_CNT4_STRUC;
#else
typedef	union _TX_AGG_CNT4_STRUC {
	struct {
	    UINT16  AggSize9Count;
	    UINT16  AggSize10Count;
	} field;
	UINT32 word;
} TX_AGG_CNT4_STRUC;
#endif


#define TX_AGG_CNT5	0x1734
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT5_STRUC {
	struct {
	    UINT16  AggSize12Count;
	    UINT16  AggSize11Count;
	} field;
	UINT32 word;
} TX_AGG_CNT5_STRUC;
#else
typedef	union _TX_AGG_CNT5_STRUC {
	struct {
	    UINT16  AggSize11Count;
	    UINT16  AggSize12Count;
	} field;
	UINT32 word;
} TX_AGG_CNT5_STRUC;
#endif


#define TX_AGG_CNT6		0x1738
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT6_STRUC {
	struct {
	    UINT16  AggSize14Count;
	    UINT16  AggSize13Count;
	} field;
	UINT32 word;
} TX_AGG_CNT6_STRUC;
#else
typedef	union _TX_AGG_CNT6_STRUC {
	struct {
	    UINT16  AggSize13Count;
	    UINT16  AggSize14Count;
	} field;
	UINT32 word;
} TX_AGG_CNT6_STRUC;
#endif


#define TX_AGG_CNT7		0x173c
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT7_STRUC {
	struct {
	    UINT16  AggSize16Count;
	    UINT16  AggSize15Count;
	} field;
	UINT32 word;
} TX_AGG_CNT7_STRUC;
#else
typedef	union _TX_AGG_CNT7_STRUC {
	struct {
	    UINT16  AggSize15Count;
	    UINT16  AggSize16Count;
	} field;
	UINT32 word;
} TX_AGG_CNT7_STRUC;
#endif


#define TX_AGG_CNT8	0x174c	/* AGG_SIZE  = 17,18 */
#define TX_AGG_CNT9	0x1750	/* AGG_SIZE  = 19,20 */
#define TX_AGG_CNT10	0x1754	/* AGG_SIZE  = 21,22 */
#define TX_AGG_CNT11	0x1758	/* AGG_SIZE  = 23,24 */
#define TX_AGG_CNT12	0x175c	/* AGG_SIZE  = 25,26 */
#define TX_AGG_CNT13	0x1760	/* AGG_SIZE  = 27,28 */
#define TX_AGG_CNT14	0x1764	/* AGG_SIZE  = 29,30 */
#define TX_AGG_CNT15	0x1768	/* AGG_SIZE  = 31,32 */
#define TX_AGG_CNT16	0x179C  /* AGG_SIZE = 33, 34 */
#define TX_AGG_CNT17	0x17a0	/* AGG_SIZE = 35, 36 */
#define TX_AGG_CNT18	0x17a4  /* AGG_SIZE = 37, 38 */
#define TX_AGG_CNT19	0x17a8  /* AGG_SIZE = 39, 40 */
#define TX_AGG_CNT20	0x17ac	/* AGG_SIZE = 41, 42 */
#define TX_AGG_CNT21	0x17b0	/* AGG_SIZE = 43, 44 */
#define TX_AGG_CNT22	0x17b4  /* AGG_SIZE = 45, 46 */
#define TX_AGG_CNT23	0x17b8  /* AGG_SIZE = 47, 48 */ 
#ifdef RT_BIG_ENDIAN
typedef	union _TX_AGG_CNT_STRUC {
	struct {
	    UINT16  AggCnt_y;	/* the count of aggregation size = x + 1 */
	    UINT16  AggCnt_x;	/* the count of aggregation size = x */
	} field;
	UINT32 word;
} TX_AGG_CNT_STRUC;
#else
typedef	union _TX_AGG_CNT_STRUC {
	struct {
	    UINT16  AggCnt_x;
	    UINT16  AggCnt_y;
	} field;
	UINT32 word;
} TX_AGG_CNT_STRUC;
#endif

typedef	union _TX_AGG_CNTN_STRUC {
	struct {
#ifdef RT_BIG_ENDIAN
	    UINT16  AggSizeHighCount;
	    UINT16  AggSizeLowCount;
#else
	    UINT16  AggSizeLowCount;
	    UINT16  AggSizeHighCount;
#endif
	} field;
	UINT32 word;
} TX_AGG_CNTN_STRUC;


#define MPDU_DENSITY_CNT		0x1740
#ifdef RT_BIG_ENDIAN
typedef	union _MPDU_DEN_CNT_STRUC {
	struct {
	    UINT16  RXZeroDelCount;	/*RX zero length delimiter count */
	    UINT16  TXZeroDelCount;	/*TX zero length delimiter count */
	} field;
	UINT32 word;
} MPDU_DEN_CNT_STRUC;
#else
typedef	union _MPDU_DEN_CNT_STRUC {
	struct {
	    UINT16  TXZeroDelCount;
	    UINT16  RXZeroDelCount;
	} field;
	UINT32 word;
} MPDU_DEN_CNT_STRUC;
#endif


/* TX_STA_FIFO_EXT_STRUC: TX retry cnt for specific frame */
#define TX_STA_FIFO_EXT		0x1798		/* Only work after RT53xx */
#ifdef RT_BIG_ENDIAN
typedef	union _TX_STA_FIFO_EXT_STRUC {
	struct {
		UINT32 rsv:16;
		UINT32 pkt_id_65xx:8; /* pkt_id when run as rt65xx based chips */
		UINT32 txRtyCnt:8;   /* frame Tx retry cnt */
	} field;
	UINT32 word;
} TX_STA_FIFO_EXT_STRUC;
#else
typedef	union _TX_STA_FIFO_EXT_STRUC {
	struct {
		UINT32 txRtyCnt:8;
		UINT32 pkt_id_65xx:8;
		UINT32 rsv:16;
	} field;
	UINT32 word;
} TX_STA_FIFO_EXT_STRUC;
#endif


#ifdef FIFO_EXT_SUPPORT
#define WCID_TX_CNT_0	0x176c
#define WCID_TX_CNT_1	0x1770
#define WCID_TX_CNT_2	0x1774
#define WCID_TX_CNT_3	0x1778
#define WCID_TX_CNT_4	0x177c
#define WCID_TX_CNT_5	0x1780
#define WCID_TX_CNT_6	0x1784
#define WCID_TX_CNT_7	0x1788
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_TX_CNT_STRUC {
	struct {
		UINT32		reTryCnt:16;
		UINT32		succCnt:16;
	} field;
	UINT32 word;
} WCID_TX_CNT_STRUC;
#else
typedef	union _WCID_TX_CNT_STRUC {
	struct {
		UINT32		succCnt:16;
		UINT32		reTryCnt:16;
	} field;
	UINT32 word;
} WCID_TX_CNT_STRUC;
#endif


#define WCID_MAPPING_0	0x178c
#define WCID_MAPPING_1	0x1790
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_MAPPING_STRUC {
	struct {
		UINT32		wcid3:8;
		UINT32		wcid2:8;
		UINT32		wcid1:8;
		UINT32		wcid0:8;
	} field;
	UINT32 word;
} WCID_MAPPING_STRUC;
#else
typedef	union _WCID_MAPPING_STRUC {
	struct {
		UINT32		wcid0:8;
		UINT32		wcid1:8;
		UINT32		wcid2:8;
		UINT32		wcid3:8;
	} field;
	UINT32 word;
} WCID_MAPPINGT_STRUC;
#endif
#endif /* FIFO_EXT_SUPPORT */

#define TX_REPORT_CNT	0x1794


/* Security key table memory, base address = 0x1000 */
#define MAC_WCID_BASE		0x1800 /*8-bytes(use only 6-bytes) * 256 entry = */
#define HW_WCID_ENTRY_SIZE   8

#ifdef MCS_LUT_SUPPORT
#define MAC_MCS_LUT_BASE	0x1c00
#endif /* MCS_LUT_SUPPORT */

#ifdef RT_BIG_ENDIAN
typedef	union _SHAREDKEY_MODE_STRUC {
	struct {
		UINT32       Bss1Key3CipherAlg:4;
		UINT32       Bss1Key2CipherAlg:4;
		UINT32       Bss1Key1CipherAlg:4;
		UINT32       Bss1Key0CipherAlg:4;
		UINT32       Bss0Key3CipherAlg:4;
		UINT32       Bss0Key2CipherAlg:4;
		UINT32       Bss0Key1CipherAlg:4;
		UINT32       Bss0Key0CipherAlg:4;
	} field;
	UINT32 word;
} SHAREDKEY_MODE_STRUC;
#else
typedef	union _SHAREDKEY_MODE_STRUC {
	struct {
		UINT32       Bss0Key0CipherAlg:4;
		UINT32       Bss0Key1CipherAlg:4;
		UINT32       Bss0Key2CipherAlg:4;
		UINT32       Bss0Key3CipherAlg:4;
		UINT32       Bss1Key0CipherAlg:4;
		UINT32       Bss1Key1CipherAlg:4;
		UINT32       Bss1Key2CipherAlg:4;
		UINT32       Bss1Key3CipherAlg:4;
	} field;
	UINT32 word;
} SHAREDKEY_MODE_STRUC;
#endif


/* 64-entry for pairwise key table, 8-byte per entry  */
typedef struct _HW_WCID_ENTRY {
    UINT8   Address[6];
    UINT8   Rsv[2];
} HW_WCID_ENTRY;


/* ================================================================================= */
/* WCID  format */
/* ================================================================================= */
/*7.1	WCID  ENTRY  format  : 8bytes */
typedef	struct _WCID_ENTRY_STRUC {
	UINT8		RXBABitmap7;    /* bit0 for TID8, bit7 for TID 15 */
	UINT8		RXBABitmap0;    /* bit0 for TID0, bit7 for TID 7 */
	UINT8		MAC[6];	/* 0 for shared key table.  1 for pairwise key table */
} WCID_ENTRY_STRUC;


/*8.1.1	SECURITY  KEY  format  : 8DW */
/* 32-byte per entry, total 16-entry for shared key table, 64-entry for pairwise key table */
typedef struct _HW_KEY_ENTRY {
    UINT8   Key[16];
    UINT8   TxMic[8];
    UINT8   RxMic[8];
} HW_KEY_ENTRY;


/*8.1.2	IV/EIV  format  : 2DW */

/* RX attribute entry format  : 1DW */
#ifdef RT_BIG_ENDIAN
typedef	union _WCID_ATTRIBUTE_STRUC {
	struct {
		UINT32		WAPIKeyIdx:8;
		UINT32		WAPI_rsv:8;
		UINT32		WAPI_MCBC:1;
		UINT32		rsv:3;
		UINT32		BSSIdxExt:1;
		UINT32		PairKeyModeExt:1;
		UINT32		RXWIUDF:3;
		UINT32		BSSIdx:3; /*multipleBSS index for the WCID */
		UINT32		PairKeyMode:3;
		UINT32		KeyTab:1;	/* 0 for shared key table.  1 for pairwise key table */
	} field;
	UINT32 word;
} WCID_ATTRIBUTE_STRUC;
#else
typedef	union _WCID_ATTRIBUTE_STRUC {
	struct {
		UINT32		KeyTab:1;	/* 0 for shared key table.  1 for pairwise key table */
		UINT32		PairKeyMode:3;
		UINT32		BSSIdx:3; 		/*multipleBSS index for the WCID */
		UINT32		RXWIUDF:3;
		UINT32		PairKeyModeExt:1;
		UINT32		BSSIdxExt:1;
		UINT32		rsv:3;
		UINT32		WAPI_MCBC:1;
		UINT32		WAPI_rsv:8;
		UINT32		WAPIKeyIdx:8;
	} field;
	UINT32 word;
} WCID_ATTRIBUTE_STRUC;
#endif


/* ================================================================================= */
/* HOST-MCU communication data structure */
/* ================================================================================= */

/* H2M_MAILBOX_CSR: Host-to-MCU Mailbox */
#ifdef RT_BIG_ENDIAN
typedef union _H2M_MAILBOX_STRUC {
    struct {
        UINT32       Owner:8;
        UINT32       CmdToken:8;    /* 0xff tells MCU not to report CmdDoneInt after excuting the command */
        UINT32       HighByte:8;
        UINT32       LowByte:8;
    } field;
    UINT32 word;
} H2M_MAILBOX_STRUC;
#else
typedef union _H2M_MAILBOX_STRUC {
    struct {
        UINT32       LowByte:8;
        UINT32       HighByte:8;
        UINT32       CmdToken:8;
        UINT32       Owner:8;
    } field;
    UINT32 word;
} H2M_MAILBOX_STRUC;
#endif


/* M2H_CMD_DONE_CSR: MCU-to-Host command complete indication */
#ifdef RT_BIG_ENDIAN
typedef union _M2H_CMD_DONE_STRUC {
    struct {
        UINT32       CmdToken3;
        UINT32       CmdToken2;
        UINT32       CmdToken1;
        UINT32       CmdToken0;
    } field;
    UINT32 word;
} M2H_CMD_DONE_STRUC;
#else
typedef union _M2H_CMD_DONE_STRUC {
    struct {
        UINT32       CmdToken0;
        UINT32       CmdToken1;
        UINT32       CmdToken2;
        UINT32       CmdToken3;
    } field;
    UINT32 word;
} M2H_CMD_DONE_STRUC;
#endif


/* HOST_CMD_CSR: For HOST to interrupt embedded processor */
#ifdef RT_BIG_ENDIAN
typedef	union _HOST_CMD_CSR_STRUC {
	struct {
	    UINT32   Rsv:24;
	    UINT32   HostCommand:8;
	} field;
	UINT32 word;
} HOST_CMD_CSR_STRUC;
#else
typedef	union _HOST_CMD_CSR_STRUC {
	struct {
	    UINT32   HostCommand:8;
	    UINT32   Rsv:24;
	} field;
	UINT32 word;
} HOST_CMD_CSR_STRUC;
#endif


// TODO: shiang-6590, Need to check following definitions are useful or not!!!
/* AIFSN_CSR: AIFSN for each EDCA AC */


/* E2PROM_CSR: EEPROM control register */
#ifdef RT_BIG_ENDIAN
typedef	union _E2PROM_CSR_STRUC {
	struct {
		UINT32		Rsvd:25;
		UINT32       LoadStatus:1;   /* 1:loading, 0:done */
		UINT32		Type:1;			/* 1: 93C46, 0:93C66 */
		UINT32		EepromDO:1;
		UINT32		EepromDI:1;
		UINT32		EepromCS:1;
		UINT32		EepromSK:1;
		UINT32		Reload:1;		/* Reload EEPROM content, write one to reload, self-cleared. */
	} field;
	UINT32 word;
} E2PROM_CSR_STRUC;
#else
typedef	union _E2PROM_CSR_STRUC {
	struct {
		UINT32		Reload:1;
		UINT32		EepromSK:1;
		UINT32		EepromCS:1;
		UINT32		EepromDI:1;
		UINT32		EepromDO:1;
		UINT32		Type:1;
		UINT32       LoadStatus:1;
		UINT32		Rsvd:25;
	} field;
	UINT32 word;
} E2PROM_CSR_STRUC;
#endif


/* QOS_CSR0: TXOP holder address0 register */
#ifdef RT_BIG_ENDIAN
typedef	union _QOS_CSR0_STRUC {
	struct {
		UINT8		Byte3;		/* MAC address byte 3 */
		UINT8		Byte2;		/* MAC address byte 2 */
		UINT8		Byte1;		/* MAC address byte 1 */
		UINT8		Byte0;		/* MAC address byte 0 */
	} field;
	UINT32 word;
} QOS_CSR0_STRUC;
#else
typedef	union _QOS_CSR0_STRUC {
	struct {
		UINT8		Byte0;
		UINT8		Byte1;
		UINT8		Byte2;
		UINT8		Byte3;
	} field;
	UINT32 word;
} QOS_CSR0_STRUC;
#endif


/* QOS_CSR1: TXOP holder address1 register */
#ifdef RT_BIG_ENDIAN
typedef	union _QOS_CSR1_STRUC {
	struct {
		UINT8		Rsvd1;
		UINT8		Rsvd0;
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Byte4;		/* MAC address byte 4 */
	} field;
	UINT32 word;
} QOS_CSR1_STRUC;
#else
typedef	union _QOS_CSR1_STRUC {
	struct {
		UINT8		Byte4;		/* MAC address byte 4 */
		UINT8		Byte5;		/* MAC address byte 5 */
		UINT8		Rsvd0;
		UINT8		Rsvd1;
	} field;
	UINT32 word;
} QOS_CSR1_STRUC;
#endif

// TODO: shiang-6590, check upper definitions are useful or not!



/* Other on-chip shared memory space, base = 0x2000 */

/* CIS space - base address = 0x2000 */
#define HW_CIS_BASE             0x2000

/* Carrier-sense CTS frame base address. It's where mac stores carrier-sense frame for carrier-sense function. */
#define HW_CS_CTS_BASE			0x7700
/* DFS CTS frame base address. It's where mac stores CTS frame for DFS. */
#define HW_DFS_CTS_BASE			0x7780
#define HW_CTS_FRAME_SIZE		0x80


/* 
	On-chip BEACON frame space -
   	1. HW_BEACON_OFFSET/64B must be 0;
   	2. BCN_OFFSETx(0~) must also be changed in MACRegTable(common/rtmp_init.c)
 */
#define HW_BEACON_OFFSET		0x0200 

/* NullFrame buffer */
#define HW_NULL_BASE            0x7700
#define HW_NULL2_BASE			0x7780



/* 	In order to support maximum 8 MBSS and its maximum length is 512 for each beacon
	Three section discontinue memory segments will be used.
	1. The original region for BCN 0~3
	2. Extract memory from FCE table for BCN 4~5
	3. Extract memory from Pair-wise key table for BCN 6~7
  		It occupied those memory of wcid 238~253 for BCN 6 
		and wcid 222~237 for BCN 7  	*/
/*#define HW_BEACON_MAX_COUNT     8 */
#define HW_BEACON_MAX_SIZE(__pAd)      ((__pAd)->chipCap.BcnMaxHwSize)
#define HW_BEACON_BASE0(__pAd)         ((__pAd)->chipCap.BcnBase[0])
/*#define HW_BEACON_BASE1         0x7A00 */
/*#define HW_BEACON_BASE2         0x7C00 */
/*#define HW_BEACON_BASE3         0x7E00 */
/*#define HW_BEACON_BASE4         0x7200 */
/*#define HW_BEACON_BASE5         0x7400 */
/*#define HW_BEACON_BASE6         0x5DC0 */
/*#define HW_BEACON_BASE7         0x5BC0 */


/* Higher 8KB shared memory */
#define HW_BEACON_BASE0_REDIRECTION	0x4000
#define HW_BEACON_BASE1_REDIRECTION	0x4200
#define HW_BEACON_BASE2_REDIRECTION	0x4400
#define HW_BEACON_BASE3_REDIRECTION	0x4600
#define HW_BEACON_BASE4_REDIRECTION	0x4800
#define HW_BEACON_BASE5_REDIRECTION	0x4A00
#define HW_BEACON_BASE6_REDIRECTION	0x4C00
#define HW_BEACON_BASE7_REDIRECTION	0x4E00


/* HOST-MCU shared memory - base address = 0x2100 */
#define HOST_CMD_CSR		0x404
#define H2M_MAILBOX_CSR         0x7010
#define H2M_MAILBOX_CID         0x7014
#define H2M_MAILBOX_STATUS      0x701c
#define H2M_INT_SRC             0x7024
#define H2M_BBP_AGENT           0x7028
#define M2H_CMD_DONE_CSR        0x000c
#define MCU_TXOP_ARRAY_BASE     0x000c   /* TODO: to be provided by Albert */
#define MCU_TXOP_ENTRY_SIZE     32       /* TODO: to be provided by Albert */
#define MAX_NUM_OF_TXOP_ENTRY   16       /* TODO: must be same with 8051 firmware */
#define MCU_MBOX_VERSION        0x01     /* TODO: to be confirmed by Albert */
#define MCU_MBOX_VERSION_OFFSET 5        /* TODO: to be provided by Albert */


#define E2PROM_CSR          0x0004
#define IO_CNTL_CSR         0x77d0



/* ================================================================ */
/* Tx /	Rx / Mgmt ring descriptor definition */
/* ================================================================ */

/* Host DMA registers - base address 0x200 .  TX0-3=EDCAQid0-3, TX4=HCCA, TX5=MGMT, */
/*  DMA RING DESCRIPTOR */

/* the following PID values are used to mark outgoing frame type in TXD->PID so that */
/* proper TX statistics can be collected based on these categories */
/* b3-2 of PID field - */


#define WMM_QUE_NUM		4 /* each sta has 4 Queues to mapping to each WMM AC */

/* value domain of pTxD->HostQId (4-bit: 0~15) */
#define QID_AC_BK               1   /* meet ACI definition in 802.11e */
#define QID_AC_BE               0   /* meet ACI definition in 802.11e */
#define QID_AC_VI               2
#define QID_AC_VO               3
#define QID_HCCA                4
#define NUM_OF_TX_RING		5

#define NUM_OF_RX_RING		1
#ifdef CONFIG_ANDES_SUPPORT
#undef NUM_OF_RX_RING
#define NUM_OF_RX_RING		2
#endif /* CONFIG_ANDES_SUPPORT */

#define QID_MGMT                13
#define QID_RX                  14
#define QID_OTHER               15
#ifdef CONFIG_ANDES_SUPPORT
#define QID_CTRL				16
#endif /* CONFIG_ANDES_SUPPORT */




#define RTMP_MAC_SHR_MSEL_PROTECT_LOCK(__pAd, __IrqFlags)	__IrqFlags = __IrqFlags;
#define RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(__pAd, __IrqFlags) __IrqFlags = __IrqFlags;


#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE		1



struct _RTMP_ADAPTER;

VOID ReSyncBeaconTime(struct _RTMP_ADAPTER *pAd);


INT rtmp_mac_fifo_stat_update(struct _RTMP_ADAPTER *pAd);

INT rtmp_mac_set_band(struct _RTMP_ADAPTER *pAd, int  band);
INT rtmp_mac_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 extch);
INT rtmp_mac_set_mmps(struct  _RTMP_ADAPTER *pAd, INT ReduceCorePower);

INT rtmp_hw_tb_init(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);
VOID rtmp_mac_bcn_buf_init(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);

INT rtmp_mac_sys_reset(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);

INT rtmp_mac_init(struct _RTMP_ADAPTER *pAd);
#endif /* __RTMP_MAC_H__ */

