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
	icap.h
*/

#ifndef __ICAP_H_
#define __ICAP_H_

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
/* RBISTCR */
#define RBIST_BASE								0x810C0000
#define RBISTCR0								(RBIST_BASE + 0x90)
#define RBISTCR1								(RBIST_BASE + 0x94)
#define RBISTCR2								(RBIST_BASE + 0x98)/* Start address of SYSRAM */
#define RBISTCR3								(RBIST_BASE + 0x9c)/* End address of SYSRAM */
#define RBISTCR4								(RBIST_BASE + 0xa0)
#define RBISTCR5								(RBIST_BASE + 0xa4)
#define RBISTCR6								(RBIST_BASE + 0xa8)
#define RBISTCR7								(RBIST_BASE + 0xac)
#define RBISTCR8								(RBIST_BASE + 0xb0)
#define RBISTCR9								(RBIST_BASE + 0xb4)/* Last write address */
#define RBISTCR10								(RBIST_BASE + 0xb8)

/* RBISTCR0 */
#define ICAP_WRAP								17
#define CR_RBIST_CAPTURE						1

/* RBISTCR10 */
#define SYSRAM_INTF_SEL1						26
#define SYSRAM_INTF_SEL2						27
#define SYSRAM_INTF_SEL3						28

/* CR_DBGSGD_MODE */
#define CR_SGD_MODE1							0
#define CR_SGD_MODE4							3
#define CR_SGD_DBG_SEL							15

/* Trigger Event */
#define CAP_FREE_RUN							0

/* Ring Mode */
#define CAP_RING_MODE_ENABLE					1
#define CAP_RING_MODE_DISABLE					0

/* Capture Bit Width */
#define CAP_96BIT								0
#define CAP_128BIT								1

/* Capture Node */
#define MT7615_CAP_WF0_ADC						0x000b
#define MT7615_CAP_WF1_ADC						0x000c
#define MT7615_CAP_WF2_ADC						0x000d
#define MT7615_CAP_WF3_ADC						0x000e
#define MT7615_CAP_WF0_FIIQ						0x200b
#define MT7615_CAP_WF1_FIIQ						0x200c
#define MT7615_CAP_WF2_FIIQ						0x200d
#define MT7615_CAP_WF3_FIIQ						0x200e
#define MT7615_CAP_WF0_FDIQ						0x300b
#define MT7615_CAP_WF1_FDIQ						0x300c
#define MT7615_CAP_WF2_FDIQ						0x300d
#define MT7615_CAP_WF3_FDIQ						0x300e
#define MT7622_CAP_FOUR_WAY_ADC					0x005d
#define MT7622_CAP_FOUR_WAY_FIIQ				0x0049
#define MT7622_CAP_FOUR_WAY_FDIQ				0x0048
#define MT7622_CAP_WF0_ADC						0x100b
#define MT7622_CAP_WF1_ADC						0x100c
#define MT7622_CAP_WF2_ADC						0x100d
#define MT7622_CAP_WF3_ADC						0x100e
#define MT7622_CAP_WF0_FIIQ						0x200b
#define MT7622_CAP_WF1_FIIQ						0x200c
#define MT7622_CAP_WF2_FIIQ						0x200d
#define MT7622_CAP_WF3_FIIQ						0x200e
#define MT7622_CAP_WF0_FDIQ						0x300b
#define MT7622_CAP_WF1_FDIQ						0x300c
#define MT7622_CAP_WF2_FDIQ						0x300d
#define MT7622_CAP_WF3_FDIQ						0x300e

/* Capture Bw */
#define CAP_BW_20								0
#define CAP_BW_40								1
#define CAP_BW_80								2

/* Capture Antenna */
#define CAP_WF0									0
#define CAP_WF1									1
#define CAP_WF2									2
#define CAP_WF3									3

/* I/Q Type */
#define CAP_I_TYPE								0
#define CAP_Q_TYPE								1

/* Parameter Setting */
#define CAP_MODE								0
#define CAP_TRIGGER								1
#define CAP_RING_MODE							2
#define CAP_BBP_EVENT							3
#define CAP_NODE								4
#define CAP_LENGTH								5
#define CAP_STOP_CYCLE							6
#define CAP_BW									7
#define CAP_MAC_EVENT							8
#define CAP_SOURCE_ADDR							9
#define CAP_BAND								10

/* Capture Status */
#define CAP_SUCCESS								0
#define CAP_BUSY								1
#define CAP_FAIL								2

/* Capture BitWidth */
#define CAP_32BITS								32
#define CAP_96BITS								96

/* Dump raw data expired time */
#define CAP_DUMP_DATA_EXPIRE					100000

/* Length of file naming */
#define CAP_FILE_MSG_LEN						64

/*
===================================
	WIFISPECTRUM DEFINE
===================================
*/
/* Wifi-Spectrum Event Data Sample Cnt */
#define SPECTRUM_EVENT_DATA_SAMPLE				256

/* Wifi-Spectrum CMD Response Length */
#define SPECTRUM_DEFAULT_RESP_LEN				0

/* Wifi-Spectrum Wait CMD Response Time */
#define SPECTRUM_DEFAULT_WAIT_RESP_TIME			0
#define SPECTRUM_WAIT_RESP_TIME					10000

/* MT7615 */
#define MT7615_SPECTRUM_TOTAL_SIZE				128/* Unit:KBytes */
/*
===================================
	ICAP DEFINE
===================================
*/
/* ICap Event Data Sample Cnt */
#define ICAP_EVENT_DATA_SAMPLE					256

/* MT7622 */
#define MT7622_ICAP_BANK_SAMPLE_CNT				4096
#define MT7622_ICAP_FOUR_WAY_ADC_IQ_DATA_CNT	(4096 * 6)/* (4096(Samples/Bank) * 6Banks * 3(IQSamples/Sample) * 32bits)/96bits */
#define MT7622_ICAP_FOUR_WAY_IQC_IQ_DATA_CNT	(4096 * 2)/* (4096(Samples/Bank) * 6Banks * 1(IQSamples/Sample) * 32bits)/96bits */

/* MT7663 */
#define MT7663_ICAP_BANK_SAMPLE_CNT				8192
#define MT7663_ICAP_FOUR_WAY_ADC_IQ_DATA_CNT	8192
#define MT7663_ICAP_FOUR_WAY_IQC_IQ_DATA_CNT	8192

#define MAX_ICAP_IQ_DATA_CNT					MT7622_ICAP_FOUR_WAY_ADC_IQ_DATA_CNT
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
typedef enum _ICAP_WIFI_SPECTRUM_MODE {
	ICAP_MODE = 0,
	WIFI_SPECTRUM_MODE
} ICAP_WIFI_SPECTRUM_MODE;

typedef struct GNU_PACKED _RBIST_IQ_DATA_T {
	INT32 IQ_Array[4][2]; /* IQ_Array[WF][IQ] */
} RBIST_IQ_DATA_T, *P_RBIST_IQ_DATA_T;

typedef struct GNU_PACKED _RBIST_CAP_START_T {
	UINT32 fgTrigger;
	UINT32 fgRingCapEn;
	UINT32 u4TriggerEvent;
	UINT32 u4CaptureNode;
	UINT32 u4CaptureLen;    /* Unit : IQ Sample */
	UINT32 u4CapStopCycle;  /* Unit : IQ Sample */
	UINT32 u4MACTriggerEvent;
	UINT32 u4SourceAddressLSB;
	UINT32 u4SourceAddressMSB;
	UINT32 u4BandIdx;
	UINT32 u4BW;
	UINT32 u4En96b128b;/* 0:96b, 1:128b */
	UINT32 u4Reserved[8];
} RBIST_CAP_START_T, *P_RBIST_CAP_START_T;

typedef struct GNU_PACKED _RBIST_DUMP_RAW_DATA_T {
	UINT32 u4Address;
	UINT32 u4AddrOffset;
	UINT32 u4Bank;
	UINT32 u4BankSize;/* Uint:Kbytes */
	UINT32 u4Reserved[8];
} RBIST_DUMP_RAW_DATA_T, *P_RBIST_DUMP_RAW_DATA_T;

typedef struct GNU_PACKED _RBIST_DESC_T {
	UINT32 u4Address;
	UINT32 u4AddrOffset;
	UINT32 u4Bank;
	UINT32 u4BankSize; /* Uint:KBytes */
	UINT8  ucBitWidth; /* Uint:Bit */
	UINT8  ucADCRes;   /* Uint:Bit */
	UINT8  ucIQCRes;   /* Uint:Bit */
	PUINT8 pLBank;
	PUINT8 pMBank;
	PUINT8 pHBank;
} RBIST_DESC_T, *P_RBIST_DESC_T;

typedef struct GNU_PACKED _EXT_CMD_SPECTRUM_CTRL_T {
	UINT32 u4FuncIndex;
	RBIST_CAP_START_T rSpectrumInfo;
	RBIST_DUMP_RAW_DATA_T rSpectrumDump;
} EXT_CMD_SPECTRUM_CTRL_T, *P_EXT_CMD_SPECTRUM_CTRL_T;

typedef struct _EXT_EVENT_SPECTRUM_RESULT_T {
	UINT32 u4FuncIndex;
	UINT32 u4FuncLength;
	UINT8  aucEvent[0];
} EXT_EVENT_SPECTRUM_RESULT_T, *PEXT_EVENT_SPECTRUM_RESULT_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_ADDR_T {
	UINT32 u4FuncIndex;
	UINT32 u4FuncLength;
	UINT32 u4StartAddr1;
	UINT32 u4StartAddr2;
	UINT32 u4StartAddr3;
	UINT32 u4EndAddr;
	UINT32 u4StopAddr;
	UINT32 u4Wrap;
} EXT_EVENT_RBIST_ADDR_T, *P_EXT_EVENT_RBIST_ADDR_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_DUMP_RAW_DATA_T {
	UINT32 u4FuncIndex;
	UINT32 u4PktNum;
	UINT32 u4Bank;
	UINT32 u4DataLength;
	UINT32 u4Reserved[8];
	UINT32 u4Data[256];
} EXT_EVENT_RBIST_DUMP_RAW_DATA_T, *P_EXT_EVENT_RBIST_DUMP_RAW_DATA_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_CAP_STATUS_T {
	UINT32 u4FuncIndex;
	UINT32 u4CapDone;
	UINT32 u4CapStartAddr;
	UINT32 u4CapEndAddr;
	UINT32 u4CapStopAddr;
	UINT32 u4Wrap;
	UINT32 u4TotalBufferSize;/* Uint:KBytes */
	UINT32 u4EMIStartAddr;
	UINT32 u4EMIEndAddr;
	UINT32 u4Reserved[8];
} EXT_EVENT_RBIST_CAP_STATUS_T, *P_EXT_EVENT_RBIST_CAP_STATUS_T;

typedef enum _ENUM_SPECTRUM_CTRL_FUNCID_T {
	SPECTRUM_CTRL_FUNCID_SET_PARAMETER = 0,
	SPECTRUM_CTRL_FUNCID_GET_CAPTURE_STATUS,
	SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA
} ENUM_SPECTRUM_CTRL_FUNCID_T;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/


#endif /* __ICAP_H_ */
