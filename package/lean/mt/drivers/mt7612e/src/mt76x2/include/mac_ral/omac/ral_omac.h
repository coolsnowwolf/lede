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
	ral_omac.h

	Abstract:
	Ralink Wireless Chip RAL MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RAL_OMAC_H__
#define __RAL_OMAC_H__



/*
	TXINFO
*/

/*
	TXINFO fields defintion:
	
	USBDMATxPktLen[b0~b14]:
		Total bytes of all sub-frame. ONLY for USB bulk Aggregation
	IPOffset[b15~b18]:
		Start byte of IP packet. The base address is from TXINFO.
		0: header will be parsed by hardware.
		This field is like backdoor.
		For AMSDU, this field is useless

	TCPOffset[b19~b23]:
		Start byte of TCP packet from IP packet The base address is IP header.
		0: header will be parsed by hardware.
		This field is like backdoor.
		For AMSDU, this field is useless

	WIV[b24]:
		Wireless Info Valid. 
		1: if Driver already fill WI
		0: if DMA needs to copy WI to correctposition

	QSEL[b25~b26]:
		Select on-chip FIFO ID for 2nd-stage output scheduler.
		0:MGMT, 1:HCCA 2:EDCA

	SwUseLastRound[b27]:
		Software used for USB-based chipset, reserved for other interfaces.

	uso[b28]:
		UDP checksum enable. 
		1: indicate this packet needs to do UDP checksum

	cso[b29]:
		Checksum offload. 
		1: indicate this packet needs to do checksum

	USBDMANextVLD[b30]:
		Used for USB-based chipset, reserved for other interfaces.
		Used ONLY in USB bulk Aggregation, host driver info DMA current 
		frame is not he last frame in current Tx queue

	USBDMATxburst[b31]:
		force DMA transmit frame from current selected endpoint
*/
#ifdef RT_BIG_ENDIAN
typedef	struct _TXINFO_OMAC {
	UINT32 USBDMATxburst:1;
	UINT32 USBDMANextVLD:1;
	UINT32 cso:1;
	UINT32 uso:1;
#ifdef USB_BULK_BUF_ALIGMENT
	UINT32 bFragLasAlignmentsectiontRound:1;
#else
	UINT32 SwUseLastRound:1;
#endif /* USB_BULK_BUF_ALIGMENT */
	UINT32 QSEL:2;
	UINT32 WIV:1;
	UINT32 TCPOffset:5;
	UINT32 IPOffset:4;
	UINT32 USBDMATxPktLen:15;
}TXINFO_OMAC;
#else
typedef	struct _TXINFO_OMAC {
	UINT32 USBDMATxPktLen:15;
	UINT32 IPOffset:4;
	UINT32 TCPOffset:5;
	UINT32 WIV:1;
	UINT32 QSEL:2;
#ifdef USB_BULK_BUF_ALIGMENT
	UINT32 bFragLasAlignmentsectiontRound:1;
#else
	UINT32 SwUseLastRound:1;
#endif /* USB_BULK_BUF_ALIGMENT */
	UINT32 uso:1;
	UINT32 cso:1;
	UINT32 USBDMANextVLD:1;
	UINT32 USBDMATxburst:1;
}TXINFO_OMAC;
#endif /* RT_BIG_ENDIAN */


#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _TXWI_OMAC {
	/* Word 0 */
	UINT32		PHYMODE:2;
	UINT32		iTxBF:1; /* iTxBF enable */
	UINT32		Sounding:1; /* Sounding enable */
	UINT32		eTxBF:1; /* eTxBF enable */
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

#if defined(RT5592) || defined(RT6352)
	/* Word 4 */
	/* For Expert Antenna */
	UINT32		Reserved:11;
	UINT32		CCP:1;
	UINT32		TxPwrAdj:4;
	UINT32		TxStreamMode:8;
	UINT32		EncodedAntID:8;
#endif /* defined(RT5592) || defined(RT6352) */
}	TXWI_OMAC;
#else
typedef	struct GNU_PACKED _TXWI_OMAC {
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
	UINT32		eTxBF:1; /* eTxBF enable */
	UINT32		Sounding:1; /* Sounding enable */
	UINT32		iTxBF:1; /* iTxBF enable */
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

#if defined(RT5592) || defined(RT6352)
	/* Word 4 */
	/* For Expert Antenna */
	UINT32      EncodedAntID:8;
	UINT32      TxStreamMode:8;
	UINT32      TxPwrAdj:4;
	UINT32      CCP:1;
	UINT32      Reserved:11; 	
#endif /* defined(RT5592) || defined(RT6352) */
}	TXWI_OMAC;
#endif


#ifdef RTMP_RBUS_SUPPORT
#endif /* RTMP_RBUS_SUPPORT */

/*
	RXWI wireless information format, in PBF. invisible in driver.
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXWI_OMAC{
	/* Word 0 */
	UINT32		tid:4;
	UINT32		MPDUtotalByteCnt:12;
	UINT32		UDF:3;
	UINT32		bss_idx:3;
	UINT32		key_idx:2;
	UINT32		wcid:8;
	
	/* Word 1 */
	UINT32		phy_mode:2;              /* 1: this RX frame is unicast to me */
	UINT32		iTxBF:1; /* iTxBF enable */
	UINT32		Sounding:1; /* Sounding enable */
	UINT32		eTxBF:1; /* eTxBF enable */
	UINT32		stbc:2;
	UINT32		sgi:1;
	UINT32		bw:1;
	UINT32		mcs:7;
	UINT32		SEQUENCE:12;
	UINT32		FRAG:4;
	
	/* Word 2 */
	UINT32		rsv1:8;
	UINT32		RSSI2:8;
	UINT32		RSSI1:8;
	UINT32		RSSI0:8;
	
	/* Word 3 */
	UINT32		FOFFSET:8;
	UINT32		SNR2:8;
	UINT32		SNR1:8;
	UINT32		SNR0:8;
	
#if defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT5592) || defined(RT6352)
	UINT32		rsv3;
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT5592) || defined(RT6352) */

#if defined(RT5592) || defined(RT6352)
	/* Word 5 */
	/* For Exper Antenna */
	UINT32		rsv4:24;
	UINT32		EANT_ID:8;
#endif /* defined(RT5592) || defined(RT6352) */
}	RXWI_OMAC;
#else
typedef	struct GNU_PACKED _RXWI_OMAC{
	/* Word	0 */
	UINT32		wcid:8;
	UINT32		key_idx:2;
	UINT32		bss_idx:3;
	UINT32		UDF:3;
	UINT32		MPDUtotalByteCnt:12;
	UINT32		tid:4;

	/* Word	1 */
	UINT32		FRAG:4;
	UINT32		SEQUENCE:12;
	UINT32		mcs:7;
	UINT32		bw:1;
	UINT32		sgi:1;
	UINT32		stbc:2;
	UINT32		eTxBF:1; /* eTxBF enable */
	UINT32		Sounding:1; /* Sounding enable */
	UINT32		iTxBF:1; /* iTxBF enable */
	UINT32		phy_mode:2;              /* 1: this RX frame is unicast to me */

	/*Word2 */
	UINT32		RSSI0:8;
	UINT32		RSSI1:8;
	UINT32		RSSI2:8;
	UINT32		rsv1:8;

	/*Word3 */
	UINT32		SNR0:8;
	UINT32		SNR1:8;
	UINT32		SNR2:8;
	UINT32		FOFFSET:8;

#if defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT5592) || defined(RT6352)
	UINT32		rsv3;
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT5592) || defined(RT6352) */

#if defined(RT5592) || defined(RT6352)
	/* Word 5 */
	/* For Exper Antenna */
	UINT32      EANT_ID:8;
	UINT32      rsv4:24;
#endif /* defined(RT5592) || defined(RT6352) */
}	RXWI_OMAC;
#endif


typedef struct GNU_PACKED _OMAC_HW_RATE_CTRL_STRUCT{
#ifdef RT_BIG_ENDIAN
	UINT16 PHYMODE:2;
	UINT16 iTxBF:1;
	UINT16 Sounding:1; /* Sounding enable */
	UINT16 eTxBF:1;
	UINT16 STBC:2;
	UINT16 ShortGI:1;
	UINT16 BW:1;			/* channel bandwidth 20/40 */
	UINT16 MCS:7;
#else
	UINT16 MCS:7;
	UINT16 BW:1;
	UINT16 ShortGI:1;
	UINT16 STBC:2;
	UINT16 eTxBF:1;
	UINT16 Sounding:1;
	UINT16 iTxBF:1;
	UINT16 PHYMODE:2;  
#endif /* RT_BIG_ENDIAN */
}OMAC_HW_RATE_CTRL_STRUCT;


/* ================================================================================= */
/* Register format */
/* ================================================================================= */

#define GPIO_CTRL_CFG	0x0228
#define RLT_MCU_CMD_CFG	0x022c


#define PAIRWISE_KEY_TABLE_BASE     0x4000      /* 32-byte * 256-entry =  -byte */
#define HW_KEY_ENTRY_SIZE           0x20

#define PAIRWISE_IVEIV_TABLE_BASE     0x6000      /* 8-byte * 256-entry =  -byte */
#define MAC_IVEIV_TABLE_BASE     0x6000      /* 8-byte * 256-entry =  -byte */
#define HW_IVEIV_ENTRY_SIZE   8

#define MAC_WCID_ATTRIBUTE_BASE     0x6800      /* 4-byte * 256-entry =  -byte */
#define HW_WCID_ATTRI_SIZE   4

#define SHARED_KEY_TABLE_BASE       0x6c00      /* 32-byte * 16-entry = 512-byte */
#define SHARED_KEY_MODE_BASE       0x7000      /* 32-byte * 16-entry = 512-byte */

#define HW_SHARED_KEY_MODE_SIZE   4

#define SHARED_KEY_TABLE_BASE_EXT      0x7400      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */
#define SHARED_KEY_MODE_BASE_EXT       0x73F0      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */

/* This resgiser is ONLY be supported for RT3883 or later.
   It conflicted with BCN#0 offset of previous chipset. */
#define WAPI_PN_TABLE_BASE		0x7800		
#define WAPI_PN_ENTRY_SIZE   		8


struct _RTMP_ADAPTER;
struct _RXINFO_STRUC;
union _RXWI_STRUC;
union _TXWI_STRUC;

#ifdef DBG
VOID dump_rtmp_rxwi(struct _RTMP_ADAPTER *pAd, union _RXWI_STRUC *pRxWI);
VOID dump_rtmp_txwi(struct _RTMP_ADAPTER *pAd, union _TXWI_STRUC *pTxWI);
VOID dump_rtmp_rxinfo(struct _RTMP_ADAPTER *pAd, struct _RXINFO_STRUC *pRxInfo);
#endif

INT rtmp_get_rxwi_phymode(union _RXWI_STRUC *rxwi);
INT rtmp_get_rxwi_rssi(union _RXWI_STRUC *rxwi, INT size, CHAR *rssi);
INT rtmp_get_rxwi_snr(union _RXWI_STRUC *rxwi, INT size, UCHAR *snr);
VOID rtmp_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

#endif /* __RAL_OMAC_H__ */

