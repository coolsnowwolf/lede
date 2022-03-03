/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ral_nmac.h

	Abstract:
	Ralink Wireless Chip RAL MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RAL_NMAC_H__
#define __RAL_NMAC_H__

enum INFO_TYPE {
	NORMAL_PACKET,
	CMD_PACKET,
};

enum D_PORT {
	WLAN_PORT,
	CPU_RX_PORT,
	CPU_TX_PORT,
	HOST_PORT,
	VIRTUAL_CPU_RX_PORT,
	VIRTUAL_CPU_TX_PORT,
	DISCARD,
};

#include "rtmp_type.h"

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_PKT{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 QSEL:2;
	UINT32 wiv:1;
	UINT32 rsv1:2;
	UINT32 cso:1;
	UINT32 tso:1;
	UINT32 pkt_80211:1;
	UINT32 rsv0:1;
	UINT32 tx_burst:1;
	UINT32 next_vld:1;
	UINT32 pkt_len:16;
}TXINFO_NMAC_PKT;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_PKT {
	UINT32 pkt_len:16;
	UINT32 next_vld:1;
	UINT32 tx_burst:1;
	UINT32 rsv0:1;
	UINT32 pkt_80211:1;
	UINT32 tso:1;
	UINT32 cso:1;
	UINT32 rsv1:2;
	UINT32 wiv:1;
	UINT32 QSEL:2;
	UINT32 d_port:3;
	UINT32 info_type:2;
}TXINFO_NMAC_PKT;
#endif /* RT_BIG_ENDIAN */


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_CMD{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 cmd_type:7;
	UINT32 cmd_seq:4;
	UINT32 pkt_len:16;
}TXINFO_NMAC_CMD;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_CMD{
	UINT32 pkt_len:16;
	UINT32 cmd_seq:4;
	UINT32 cmd_type:7;
	UINT32 d_port:3;
	UINT32 info_type:2;
}TXINFO_NMAC_CMD;
#endif /* RT_BIG_ENDIAN */


typedef union GNU_PACKED _TXINFO_NMAC{
	struct _TXINFO_NMAC_PKT txinfo_pkt;
	struct _TXINFO_NMAC_CMD txinfo_cmd;
}TXINFO_NMAC;


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXWI_NMAC {
	/* Word 0 */
	UINT32		PHYMODE:3;
	UINT32		iTxBF:1;
	UINT32		eTxBF:1;
	UINT32		STBC:1;
	UINT32		ShortGI:1;
	UINT32		BW:2;			/* channel bandwidth 20/40/80 MHz */
	UINT32		LDPC:1;
	UINT32		MCS:6;
	
	UINT32		lut_en:1;
	UINT32		Sounding:1;
	UINT32		NDPSndBW:2;	/* NDP sounding BW */
	UINT32		RTSBWSIG:1;
	UINT32		NDPSndRate:1;	/* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		txop:2;

	UINT32		MpduDensity:3;
	UINT32		AMPDU:1;
	UINT32		TS:1;
	UINT32		CFACK:1;
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	
	/* Word 1 */
	UINT32		TIM:1;
	UINT32		TXBF_PT_SCA:1;
	UINT32		MPDUtotalByteCnt:14;
	UINT32		wcid:8;
	UINT32		BAWinSize:6;
	UINT32		NSEQ:1;
	UINT32		ACK:1;

	/* Word 2 */
	UINT32		IV;
	/* Word 3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		TxPktId:8;
	UINT32     GroupID:1;
	UINT32     Rsv4:3;
	//UINT32		Rsv4:4;
	UINT32		TxPwrAdj:4;
	UINT32		TxStreamMode:8;
	UINT32		TxEAPId:8;
}	TXWI_NMAC;
#else
typedef	struct GNU_PACKED _TXWI_NMAC {
	/* Word	0 */
	/* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		CFACK:1;
	UINT32		TS:1;
	UINT32		AMPDU:1;
	UINT32		MpduDensity:3;

	UINT32		txop:2;
	UINT32		NDPSndRate:1; /* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		RTSBWSIG:1;
	UINT32		NDPSndBW:2; /* NDP sounding BW */
	UINT32		Sounding:1;
	UINT32		lut_en:1;
	
	UINT32		MCS:6;
	UINT32		LDPC:1;
	UINT32		BW:2;		/*channel bandwidth 20/40/80 MHz */
	UINT32		ShortGI:1;
	UINT32		STBC:1;
	UINT32		eTxBF:1;
	UINT32		iTxBF:1;
	UINT32		PHYMODE:3;  

	/* Word1 */
	/* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCnt = 0x38 */
	UINT32		ACK:1;
	UINT32		NSEQ:1;
	UINT32		BAWinSize:6;
	UINT32		wcid:8;
	UINT32		MPDUtotalByteCnt:14;
	UINT32		TXBF_PT_SCA:1;
	UINT32      TIM:1;
	
	/*Word2 */
	UINT32		IV;
	
	/*Word3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		TxEAPId:8;
	UINT32		TxStreamMode:8;
	UINT32		TxPwrAdj:4;
	//UINT32		Rsv4:4;	
	UINT32     Rsv4:3;
	UINT32     GroupID:1;
	UINT32		TxPktId:8;
}	TXWI_NMAC;
#endif


#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _TXWI_XMAC {
	/* Word 0 */
	UINT32		PHYMODE:2;
	UINT32		rsv0_27:3;
	UINT32		STBC:2;	/*channel bandwidth 20MHz or 40 MHz */
	UINT32		ShortGI:1;
	UINT32		BW:1;	/*channel bandwidth 20MHz or 40 MHz */
	UINT32		MCS:7;
	
	UINT32		TXLUT:1;
	UINT32		TXRPT:1;
	UINT32		Autofallback:1; /* TX rate auto fallback disable */
	UINT32		NDPSndBW:1; /* NDP sounding BW */
	UINT32		NDPSndRate:2; /* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		txop:2;
	UINT32		MpduDensity:3;
	UINT32		AMPDU:1;
	
	UINT32		TS:1;
	UINT32		CFACK:1;
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */

	/* Word 1 */
	UINT32		PacketId:4;
	UINT32		MPDUtotalByteCnt:12;
	UINT32		wcid:8;
	UINT32		BAWinSize:6;
	UINT32		NSEQ:1;
	UINT32		ACK:1;

	/* Word 2 */
	UINT32		IV;

	/* Word 3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		rsv4_23:9;
	UINT32		PIFSREV:1;
	UINT32		rsv4_21:1;
	UINT32		CCP:1;
	UINT32		TxPwrAdj:4;
	UINT32		TxStreamMode:8;
	UINT32		EncodedAntID:8;
}	TXWI_XMAC;
#else
typedef	struct GNU_PACKED _TXWI_XMAC {
	/* Word	0 */
	/* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		CFACK:1;
	UINT32		TS:1;
		
	UINT32		AMPDU:1;
	UINT32		MpduDensity:3;
	UINT32		txop:2;	/*FOR "THIS" frame. 0:HT TXOP rule , 1:PIFS TX ,2:Backoff, 3:sifs only when previous frame exchange is successful. */
	UINT32		NDPSndRate:2; /* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		NDPSndBW:1; /* NDP sounding BW */
	UINT32		Autofallback:1; /* TX rate auto fallback disable */
	UINT32		TXRPT:1;
	UINT32		TXLUT:1;
	
	UINT32		MCS:7;
	UINT32		BW:1;	/*channel bandwidth 20MHz or 40 MHz */
	UINT32		ShortGI:1;
	UINT32		STBC:2;	/* 1: STBC support MCS =0-7,   2,3 : RESERVE */
	UINT32		rsv0_27:3;
	UINT32		PHYMODE:2;

	/* Word1 */
	/* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCnt = 0x38 */
	UINT32		ACK:1;
	UINT32		NSEQ:1;
	UINT32		BAWinSize:6;
	UINT32		wcid:8;
	UINT32		MPDUtotalByteCnt:12;
	UINT32		PacketId:4;

	/*Word2 */
	UINT32		IV;

	/*Word3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		EncodedAntID:8;
	UINT32		TxStreamMode:8;
	UINT32		TxPwrAdj:4;
	UINT32		rsv4_21:1;
	UINT32		CCP:1;
	UINT32		PIFSREV:1;
	UINT32		rsv4_23:9;
}	TXWI_XMAC;
#endif



#ifdef HDR_TRANS_SUPPORT
#define TX_WIFI_INFO_SIZE		4
#define RX_WIFI_INFO_SIZE		36
#define RX_WIFI_INFO_PAD		2
#else
#define TX_WIFI_INFO_SIZE		0
#define RX_WIFI_INFO_SIZE		0
#define RX_WIFI_INFO_PAD		0
#endif
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TX_WIFI_INFO {
	struct {
    	UINT32 More_Data:1;
    	UINT32 WEP:1;
    	UINT32 PS:1;
    	UINT32 RDG:1;
    	UINT32 QoS:1;
    	UINT32 EOSP:1;
    	UINT32 TID:4;
    	UINT32 Mode:2;
    	UINT32 VLAN:1;
    	UINT32 Rsv:3;
    	UINT32 BssIdx:4;
    	UINT32 Seq_Num:12;
	} field;
	UINT32 word;
} TX_WIFI_INFO;
#else
typedef union GNU_PACKED _TX_WIFI_INFO {
	struct {
    	UINT32 Seq_Num:12;
    	UINT32 BssIdx:4;
    	UINT32 Rsv:3;
    	UINT32 VLAN:1;
    	UINT32 Mode:2;
    	UINT32 TID:4;
    	UINT32 EOSP:1;
    	UINT32 QoS:1;
    	UINT32 RDG:1;
    	UINT32 PS:1;
    	UINT32 WEP:1;
    	UINT32 More_Data:1;
	} field;
	UINT32 word;
} TX_WIFI_INFO;
#endif /* RT_BIG_ENDIAN */


typedef struct GNU_PACKED _RX_WIFI_INFO{
	UINT8 wifi_hdr[36];
}RX_WIFI_INFO;




/*
	Rx Memory layout:

	1. Rx Descriptor
		Rx Descriptor(12 Bytes) + RX_FCE_Info(4 Bytes)
	2. Rx Buffer
		RxInfo(4 Bytes) + RXWI() + 802.11 packet
*/


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXFCE_INFO{
	UINT32 info_type:2;
	UINT32 s_port:3;
	UINT32 qsel:2;
	UINT32 pcie_intr:1;

	UINT32 mac_len:3;
	UINT32 l3l4_done:1;
	UINT32 pkt_80211:1;
	UINT32 ip_err:1;
	UINT32 tcp_err:1;
	UINT32 udp_err:1;
	
	UINT32 rsv:2;
	UINT32 pkt_len:14;
}RXFCE_INFO;
#else
typedef struct GNU_PACKED _RXFCE_INFO{
	UINT32 pkt_len:14;
	UINT32 rsv:2;

	UINT32 udp_err:1;
	UINT32 tcp_err:1;
	UINT32 ip_err:1;
	UINT32 pkt_80211:1;
	UINT32 l3l4_done:1;
	UINT32 mac_len:3;

	UINT32 pcie_intr:1;
	UINT32 qsel:2;
	UINT32 s_port:3;
	UINT32 info_type:2;
}RXFCE_INFO;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXFCE_INFO_CMD{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 qsel:2;
	UINT32 pcie_intr:1;
	UINT32 evt_type:4;
	UINT32 cmd_seq:4;
	UINT32 self_gen:1;
	UINT32 rsv:1;
	UINT32 pkt_len:14;
}RXFCE_INFO_CMD;
#else
typedef struct GNU_PACKED _RXFCE_INFO_CMD{
	UINT32 pkt_len:14;
	UINT32 rsv:1;
	UINT32 self_gen:1;
	UINT32 cmd_seq:4;
	UINT32 evt_type:4;
	UINT32 pcie_intr:1;
	UINT32 qsel:2;
	UINT32 d_port:3;
	UINT32 info_type:2;
}RXFCE_INFO_CMD;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXINFO_NMAC{
	UINT32 hdr_trans:1;
	UINT32 vlan_tag:1;
	UINT32 rsv:1;
	UINT32 action_wanted:1;
	UINT32 deauth:1;
	UINT32 disasso:1;
	UINT32 beacon:1;
	UINT32 probe_rsp:1;
	UINT32 sw_fc_type1:1;
	UINT32 sw_fc_type0:1;
	UINT32 pn_len:3;
	UINT32 wapi_kid:1;
	UINT32 bssid_idx3:1;
	UINT32 dec:1;
	UINT32 ampdu:1;
	UINT32 l2pad:1;
	UINT32 rssi:1;
	UINT32 htc:1;
	UINT32 amsdu:1;
	UINT32 mic_err:1;
	UINT32 icv_err:1;
	UINT32 crc_err:1;
	UINT32 mybss:1;
	UINT32 bc:1;
	UINT32 mc:1;
	UINT32 u2me:1;
	UINT32 frag:1;
	UINT32 null:1;
	UINT32 data:1;
	UINT32 ba:1;
}RXINFO_NMAC;
#else
typedef struct GNU_PACKED _RXINFO_NMAC{
	UINT32 ba:1;
	UINT32 data:1;
	UINT32 null:1;
	UINT32 frag:1;
	UINT32 u2me:1;
	UINT32 mcast:1;
	UINT32 bcast:1;
	UINT32 mybss:1;
	UINT32 crc_err:1;
	UINT32 icv_err:1;	
	UINT32 mic_err:1;
	UINT32 amsdu:1;
	UINT32 htc:1;
	UINT32 rssi:1;
	UINT32 l2pad:1;
	UINT32 ampdu:1;
	UINT32 dec:1;
	UINT32 bssid_idx3:1;
	UINT32 wapi_kid:1;
	UINT32 pn_len:3;
	UINT32 sw_fc_type0:1;
	UINT32 sw_fc_type1:1;
	UINT32 probe_rsp:1;
	UINT32 beacon:1;
	UINT32 disasso:1;
	UINT32 deauth:1;
	UINT32 action_wanted:1;
	UINT32 rsv:1;
	UINT32 vlan_tag:1;
	UINT32 hdr_trans:1;
}RXINFO_NMAC;
#endif /* RT_BIG_ENDIAN */


/*
	RXWI wireless information format.
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXWI_NMAC{
	/* Word 0 */
	UINT32 eof:1;
	UINT32 rsv:1;
	UINT32 MPDUtotalByteCnt:14; /* = rxfceinfo_len - rxwi_len- rxinfo_len - l2pad */
	UINT32 udf:3;
	UINT32 bss_idx:3;
	UINT32 key_idx:2;
	UINT32 wcid:8;

	/* Word 1 */
	UINT32 phy_mode:3;
	UINT32 rsv_1:1;
	UINT32 ldpc_ex_sym:1;
	UINT32 stbc:1;
	UINT32 sgi:1;
	UINT32 bw:2;
	UINT32 ldpc:1;
	UINT32 mcs:6;
	UINT32 sn:12;
	UINT32 tid:4;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
}	RXWI_NMAC;
#else
typedef	struct GNU_PACKED _RXWI_NMAC {
	/* Word 0 */
	UINT32 wcid:8;
	UINT32 key_idx:2;
	UINT32 bss_idx:3;
	UINT32 udf:3;
	UINT32 MPDUtotalByteCnt:14;
	UINT32 rsv:1;
	UINT32 eof:1;

	/* Word 1 */
	UINT32 tid:4;
	UINT32 sn:12;
	UINT32 mcs:6;
	UINT32 ldpc:1;
	UINT32 bw:2;
	UINT32 sgi:1;
	UINT32 stbc:1;
	UINT32 ldpc_ex_sym:1;
	UINT32 rsv_1:1;
	UINT32 phy_mode:3;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
}	RXWI_NMAC;
#endif /* RT_BIG_ENDIAN */




typedef struct GNU_PACKED _NMAC_HW_RATE_CTRL_STRUCT{
#ifdef RT_BIG_ENDIAN
	UINT16 PHYMODE:3;
	UINT16 iTxBF:1;
	UINT16 eTxBF:1;
	UINT16 STBC:1;
	UINT16 ShortGI:1;
	UINT16 BW:2;			/* channel bandwidth 20/40/80 MHz */
	UINT16 ldpc:1;
	UINT16 MCS:6;
#else
	UINT16 MCS:6;
	UINT16 ldpc:1;
	UINT16 BW:2;
	UINT16 ShortGI:1;
	UINT16 STBC:1;
	UINT16 eTxBF:1;
	UINT16 iTxBF:1;
	UINT16 PHYMODE:3;  
#endif /* RT_BIG_ENDIAN */
}NMAC_HW_RATE_CTRL_STRUCT;




/* ================================================================================= */
/* Register format */
/* ================================================================================= */
#define CMB_CTRL 0x0020
#define CMB_CTRL_AUX_OPT_MASK (0xffff)
#define CMB_CTRL_AUX_OPT_ANTSEL (1 << 12)
#define CMB_CTRL_AUX_OPT_GPIO_SETTING (1 << 14)
#define CMB_CTRL_CSR_UART_MODE (1 << 16)
#define CMB_CTRL_GPIO_MODE_LED1 (1 << 17)
#define CMB_CTRL_GPIO_MODE_LED2 (1 << 18)
#define CMB_CTRL_FOR_CLK_XTAL (1 << 19)
#define CMB_CTRL_CSR_UART_NFC (1 << 20)
#define CMB_CTRL_XTAL_RDY (1 << 22)
#define CMB_CTRL_PLL_LD (1 << 23)

#ifdef RT_BIG_ENDIAN
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32       	LDO0_EN:1;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	PLL_LD:1;
		UINT32       	XTAL_RDY:1;
		UINT32      Rsv:2;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_LEVEL:2;
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
}CMB_CTRL_STRUC;
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
		UINT32		LDO25_LEVEL:2;
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32       	Rsv:2;
		UINT32       	XTAL_RDY:1;
		UINT32       	PLL_LD:1;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO0_EN:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC;
#endif

#define COEXCFG0 0x0040
#define COEXCFG0_COEX_EN (1 << 0)
#define COEXCFG0_FIX_WL_DI_ANT (1 << 1)
#define COEXCFG0_FIX_WL_ANT_EN (1 << 2)
#define COEXCFG0_FIX_WL_TX_PWR_MASK (0x3 << 6)
#define COEXCFG0_FIX_WL_GCRF_EN (1 << 8)
#define COEXCFG0_FIX_WL_RF_LNA0_MASK (0x3 << 12)
#define COEXCFG0_FIX_WL_RF_LNA1_MASK (0x3 << 14)
#define COEXCFG0_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG0_COEX_CFG1_MASK (0xff << 24)

#define COEXCFG1 0x0044
#define COEXCFG1_DIS_WL_TR_DELAY_MASK (0xff << 0)
#define COEXCFG1_DIS_WL_PA_DELAY_MASK (0xff << 8)
#define COEXCFG1_DIS_WL_RF_DELAY_MASK (0xff << 16)
#define COEXCFG1_FIX_WL_TX_GCRF0_MASK (0xf << 24)
#define COEXCFG1_FIX_WL_TX_GCRF1_MASK (0xf << 28)

#define COEXCFG2 0x0048
#define COEXCFG2_WL_COEX_CFG0_MASK (0xff << 0)
#define COEXCFG2_WL_COEX_CFG1_MASK (0xff << 8)
#define COEXCFG2_BT_COEX_CFG0_MASK (0xff << 16)
#define COEXCFG2_BT_COEX_CFG1_MASK (0xff << 24)

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

#define MISC_CTRL	0x64

#define WLAN_FUN_CTRL		0x80
#define WLAN_FUN_CTRL_WLAN_EN (1 << 0)
#define WLAN_FUN_CTRL_WLAN_CLK_EN (1 << 1)
#define WLAN_FUN_CTRL_WLAN_RESET_RF (1 << 2)

/* MT76x0 definition */
#define WLAN_FUN_CTRL_WLAN_RESET (1 << 3)

/* MT76x2 definition */
#define WLAN_FUN_CTRL_CSR_F20M_CKEN (1 << 3)

#define WLAN_FUN_CTRL_PCIE_APP0_CLK_REQ (1 << 4)
#define WLAN_FUN_CTRL_FRC_WL_ANT_SEL (1 << 5)
#define WLAN_FUN_CTRL_INV_ANT_SEL (1 << 6)
#define WLAN_FUN_CTRL_WAKE_HOST_F0 (1 << 7)

/* MT76x0 definition */
#define WLAN_FUN_CTRL_GPIO0_IN_MASK (0xff << 8)
#define WLAN_FUN_CTRL_GPIO0_OUT_MASK (0xff << 16)
#define WLAN_FUN_CTRL_GPIO0_OUT_OE_N_MASK (0xff << 24) 

/* MT76x2 definition */
#define WLAN_FUN_CTRL_THERM_RST (1 << 8)
#define WLAN_FUN_CTRL_THERM_CKEN (1 << 9)

#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32 GPIO0_OUT_OE_N:8;
		UINT32 GPIO0_OUT:8;
		UINT32 GPIO0_IN:8;
		UINT32 WLAN_ACC_BT:1;
		UINT32 INV_TR_SW0:1;
		UINT32 FRC_WL_ANT_SET:1;
		UINT32 PCIE_APP0_CLK_REQ:1;
		UINT32 WLAN_RESET:1;
		UINT32 WLAN_RESET_RF:1;
		UINT32 WLAN_CLK_EN:1;
		UINT32 WLAN_EN:1;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#else
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32 WLAN_EN:1;
		UINT32 WLAN_CLK_EN:1;
		UINT32 WLAN_RESET_RF:1;
		UINT32 WLAN_RESET:1;
		UINT32 PCIE_APP0_CLK_REQ:1;
		UINT32 FRC_WL_ANT_SET:1;
		UINT32 INV_TR_SW0:1;
		UINT32 WLAN_ACC_BT:1;
		UINT32 GPIO0_IN:8;
		UINT32 GPIO0_OUT:8;
		UINT32 GPIO0_OUT_OE_N:8;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#endif


#define WLAN_FUN_INFO		0x84
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
		UINT32		Rsv1:26;		
		UINT32		COEX_MODE:5; /* WLAN function enable */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#else
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		COEX_MODE:5; /* WLAN function enable */
		UINT32		Rsv1:26;
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#endif


#define BT_FUN_CTRL		0xC0
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_CTRL_STRUC{
	struct{
		UINT32		GPIO1_OUT_OE_N:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_IN:8;
		UINT32		BT_ACC_WLAN:1;
		UINT32		INV_TR_SW1:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		BT_RESET:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_EN:1;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#else
typedef union _BT_FUN_CTRL_STRUC	{
	struct{
		UINT32		BT_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_RESET:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		INV_TR_SW1:1;
		UINT32		BT_ACC_WLAN:1;
		UINT32		GPIO1_IN:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_OUT_OE_N:8;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#endif


#define BT_FUN_INFO		0xC4
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_INFO_STRUC{
	struct{
		UINT32		WLAN_EEP_BUSY:1;
		UINT32		BTPower1:7;	/* Peer */
		UINT32		BTPower0:8; /* Self */
		UINT32		AFH_END_CH:8;		
		UINT32		AFH_START_CH:8;
	}field;
	UINT32 word;
} BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#else
typedef	union _BT_FUN_INFO_STRUC	{
	struct{
		UINT32		AFH_START_CH:8;
		UINT32		AFH_END_CH:8;
		UINT32		BTPower0:8;	/* Self */
		UINT32		BTPower1:7;	/* Peer */
		UINT32		WLAN_EEP_BUSY:1;			
	}field;
	UINT32 word;
}BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#endif

// TODO: shiang, this data structure is not defined for register. may can move to other place
typedef struct _WLAN_BT_COEX_SETTING
{
	BOOLEAN					ampduOff;
	BOOLEAN					coexSettingRunning;
	BOOLEAN					RateSelectionForceToUseRSSI;
	UCHAR					TxQualityFlag;
	ULONG					alc;
	ULONG					slna;
}WLAN_BT_COEX_SETTING, *PWLAN_BT_COEX_SETTING;

#define XO_CTRL0 0x0100
#define XO_CTRL1 0x0104
#define XO_CTRL2 0x0108
#define XO_CTRL3 0x010C
#define XO_CTRL4 0x0110
#define XO_CTRL5 0x0114
#define XO_CTRL6 0x0118

#define CLK_ENABLE 0x014C

#define MCU_CMD_CFG 0x0234


#define TSO_CTRL	0x0250
#ifdef RT_BIG_ENDIAN
typedef union _TSO_CTRL_STRUC {
	struct {
		UINT32 rsv:13;
		UINT32 TSO_WR_LEN_EN:1;
		UINT32 TSO_SEG_EN:1;
		UINT32 TSO_EN:1;
		UINT32 RXWI_LEN:4;
		UINT32 RX_L2_FIX_LEN:4;
		UINT32 TXWI_LEN:4;
		UINT32 TX_L2_FIX_LEN:4;
	} field;
	UINT32 word;
} TSO_CTRL_STRUC;
#else
typedef union _TSO_CTRL_STRUC {
	struct {
		UINT32 TX_L2_FIX_LEN:4;
		UINT32 TXWI_LEN:4;
		UINT32 RX_L2_FIX_LEN:4;
		UINT32 RXWI_LEN:4;
		UINT32 TSO_EN:1;
		UINT32 TSO_SEG_EN:1;
		UINT32 TSO_WR_LEN_EN:1;
		UINT32 rsv:13;
	} field;
	UINT32 word;
} TSO_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define TX_PROT_CFG6    0x13E0    // VHT 20 Protection
#define TX_PROT_CFG7    0x13E4    // VHT 40 Protection
#define TX_PROT_CFG8    0x13E8    // VHT 80 Protection
#define PIFS_TX_CFG     0x13EC    // PIFS setting

//----------------------------------------------------------------
// Header Translation 
//----------------------------------------------------------------

/* 
	header translation control register 
	bit0 --> TX translation enable
	bit1 --> RX translation enable 
*/
#define HEADER_TRANS_CTRL_REG	0x0260
#define HT_TX_ENABLE			0x1
#define HT_RX_ENABLE			0x2

#define HT_MAC_ADDR_DW0		0x02A4
#define HT_MAC_ADDR_DW1		0x02A8
#define HT_MAC_BSSID_DW0		0x02AC
#define HT_MAC_BSSID_DW1		0x02B0

#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	UINT32 Rsv:30;
    	UINT32 Rx_En:1;
    	UINT32 Tx_En:1;
	} field;
	UINT32 word;	
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#else
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	UINT32 Tx_En:1;
    	UINT32 Rx_En:1;
    	UINT32 Rsv:30;
	} field;
	UINT32 word;
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation enable by WCID */ 
#define HT_RX_WCID_EN_BASE	0x0264
#define HT_RX_WCID_OFFSET	32
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	UINT32 RX_WCID31_TRAN_EN:1;
    	UINT32 RX_WCID30_TRAN_EN:1;
    	UINT32 RX_WCID29_TRAN_EN:1;
    	UINT32 RX_WCID28_TRAN_EN:1;
    	UINT32 RX_WCID27_TRAN_EN:1;
    	UINT32 RX_WCID26_TRAN_EN:1;
    	UINT32 RX_WCID25_TRAN_EN:1;
    	UINT32 RX_WCID24_TRAN_EN:1;
    	UINT32 RX_WCID23_TRAN_EN:1;
    	UINT32 RX_WCID22_TRAN_EN:1;
    	UINT32 RX_WCID21_TRAN_EN:1;
    	UINT32 RX_WCID20_TRAN_EN:1;
    	UINT32 RX_WCID19_TRAN_EN:1;
    	UINT32 RX_WCID18_TRAN_EN:1;
    	UINT32 RX_WCID17_TRAN_EN:1;
    	UINT32 RX_WCID16_TRAN_EN:1;
    	UINT32 RX_WCID15_TRAN_EN:1;
    	UINT32 RX_WCID14_TRAN_EN:1;
    	UINT32 RX_WCID13_TRAN_EN:1;
    	UINT32 RX_WCID12_TRAN_EN:1;
    	UINT32 RX_WCID11_TRAN_EN:1;
    	UINT32 RX_WCID10_TRAN_EN:1;
    	UINT32 RX_WCID9_TRAN_EN:1;
    	UINT32 RX_WCID8_TRAN_EN:1;
    	UINT32 RX_WCID7_TRAN_EN:1;
    	UINT32 RX_WCID6_TRAN_EN:1;
    	UINT32 RX_WCID5_TRAN_EN:1;
    	UINT32 RX_WCID4_TRAN_EN:1;
    	UINT32 RX_WCID3_TRAN_EN:1;
    	UINT32 RX_WCID2_TRAN_EN:1;
    	UINT32 RX_WCID1_TRAN_EN:1;
    	UINT32 RX_WCID0_TRAN_EN:1;
	} field;
	UINT32 word;	
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#else
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	UINT32 RX_WCID0_TRAN_EN:1;
    	UINT32 RX_WCID1_TRAN_EN:1;
    	UINT32 RX_WCID2_TRAN_EN:1;
    	UINT32 RX_WCID3_TRAN_EN:1;
    	UINT32 RX_WCID4_TRAN_EN:1;
    	UINT32 RX_WCID5_TRAN_EN:1;
    	UINT32 RX_WCID6_TRAN_EN:1;
    	UINT32 RX_WCID7_TRAN_EN:1;
    	UINT32 RX_WCID8_TRAN_EN:1;
    	UINT32 RX_WCID9_TRAN_EN:1;
    	UINT32 RX_WCID10_TRAN_EN:1;
    	UINT32 RX_WCID11_TRAN_EN:1;
    	UINT32 RX_WCID12_TRAN_EN:1;
    	UINT32 RX_WCID13_TRAN_EN:1;
    	UINT32 RX_WCID14_TRAN_EN:1;
    	UINT32 RX_WCID15_TRAN_EN:1;
    	UINT32 RX_WCID16_TRAN_EN:1;
    	UINT32 RX_WCID17_TRAN_EN:1;
    	UINT32 RX_WCID18_TRAN_EN:1;
    	UINT32 RX_WCID19_TRAN_EN:1;
    	UINT32 RX_WCID20_TRAN_EN:1;
    	UINT32 RX_WCID21_TRAN_EN:1;
    	UINT32 RX_WCID22_TRAN_EN:1;
    	UINT32 RX_WCID23_TRAN_EN:1;
    	UINT32 RX_WCID24_TRAN_EN:1;
    	UINT32 RX_WCID25_TRAN_EN:1;
    	UINT32 RX_WCID26_TRAN_EN:1;
    	UINT32 RX_WCID27_TRAN_EN:1;
    	UINT32 RX_WCID28_TRAN_EN:1;
    	UINT32 RX_WCID29_TRAN_EN:1;
    	UINT32 RX_WCID30_TRAN_EN:1;
    	UINT32 RX_WCID31_TRAN_EN:1;
	} field;
	UINT32 word;
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation - black list */
#define HT_RX_BL_BASE		0x0284
#define HT_RX_BL_OFFSET		2
#define HT_RX_BL_SIZE		8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	UINT32 BLACK_ETHER_TYPE1:16;
    	UINT32 BLACK_ETHER_TYPE0:16;
	} field;
	UINT32 word;	
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#else
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	UINT32 BLACK_ETHER_TYPE0:16;
    	UINT32 BLACK_ETHER_TYPE1:16;
	} field;
	UINT32 word;
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX VLAN Mapping (TCI) */
#define HT_BSS_VLAN_BASE	0x0294
#define HT_BSS_VLAN_OFFSET	2
#define HT_BSS_VLAN_SIZE	8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	UINT32 TCI1_VID:12;
    	UINT32 TCI1_CFI:1;
    	UINT32 TCI1_PCP:3;
    	UINT32 TCI0_VID:12;
    	UINT32 TCI0_CFI:1;
    	UINT32 TCI0_PCP:3;
	} field;
	UINT32 word;	
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#else
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	UINT32 TCI0_PCP:3;
    	UINT32 TCI0_CFI:1;
    	UINT32 TCI0_VID:12;
    	UINT32 TCI1_PCP:3;
    	UINT32 TCI1_CFI:1;
    	UINT32 TCI1_VID:12;
	} field;
	UINT32 word;
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#endif /* RT_BIG_ENDIAN */


// TODO: shiang-6590, following definitions are dummy and not used for RT6590, shall remove/correct these!
#define GPIO_CTRL_CFG	0x0228
#define RLT_PBF_MAX_PCNT	0x0408 //TX_MAX_PCNT
// TODO:shiang-6590 --------------------------


#define RF_MISC	0x0518
#ifdef RT_BIG_ENDIAN
typedef union _RF_MISC_STRUC{
	struct{
		UINT32 Rsv1:29;
		UINT32 EXT_PA_EN:1;
		UINT32 ADDAC_LDO_ADC9_EN:1;
		UINT32 ADDAC_LDO_ADC6_EN:1;
	}field;
	UINT32 word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#else
typedef union _RF_MISC_STRUC{
	struct{
		UINT32 ADDAC_LDO_ADC6_EN:1;
		UINT32 ADDAC_LDO_ADC9_EN:1;
		UINT32 EXT_PA_EN:1;
		UINT32 Rsv1:29;
	}field;
	UINT32 word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#endif

#define PAMODE_PWR_ADJ0 0x1228
#define PAMODE_PWR_ADJ1 0x122c
#define DACCLK_EN_DLY_CFG	0x1264

#define RLT_PAIRWISE_KEY_TABLE_BASE     0x8000      /* 32-byte * 256-entry =  -byte */
#define RLT_HW_KEY_ENTRY_SIZE           0x20

#define RLT_PAIRWISE_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define RLT_MAC_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define RLT_HW_IVEIV_ENTRY_SIZE   8

#define RLT_MAC_WCID_ATTRIBUTE_BASE     0xa800      /* 4-byte * 256-entry =  -byte */
#define RLT_HW_WCID_ATTRI_SIZE   4

#define RLT_SHARED_KEY_TABLE_BASE       0xac00      /* 32-byte * 16-entry = 512-byte */
#define RLT_SHARED_KEY_MODE_BASE       0xb000      /* 32-byte * 16-entry = 512-byte */

#define RLT_HW_SHARED_KEY_MODE_SIZE   4

#define RLT_SHARED_KEY_TABLE_BASE_EXT      0xb400      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */
#define RLT_SHARED_KEY_MODE_BASE_EXT       0xb3f0      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */

/* This resgiser is ONLY be supported for RT3883 or later.
   It conflicted with BCN#0 offset of previous chipset. */
#define RLT_WAPI_PN_TABLE_BASE		0xb800		
#define RLT_WAPI_PN_ENTRY_SIZE   		8

struct _RTMP_ADAPTER;
struct _RXINFO_STRUC;
union _TXINFO_STRUC;
union _TXWI_STRUC;
union _RXWI_STRUC;

VOID dump_rlt_rxinfo(struct _RTMP_ADAPTER *pAd, struct _RXINFO_STRUC *pRxInfo);
VOID dump_rlt_txinfo(struct _RTMP_ADAPTER *pAd, union _TXINFO_STRUC *pTxInfo);
VOID dump_rlt_txwi(struct _RTMP_ADAPTER *pAd, union _TXWI_STRUC *pTxWI);
VOID dump_rlt_rxwi(struct _RTMP_ADAPTER *pAd, union _RXWI_STRUC *pRxWI);
VOID dumpRxFCEInfo(struct _RTMP_ADAPTER *pAd, RXFCE_INFO *pRxFceInfo);

VOID rlt_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

INT rlt_wlan_chip_onoff(struct _RTMP_ADAPTER *pAd, BOOLEAN bOn, BOOLEAN bResetWLAN);


#endif /* __RAL_NMAC_H__ */

