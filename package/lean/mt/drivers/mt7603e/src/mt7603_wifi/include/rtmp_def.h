/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp_def.h

    Abstract:
    Miniport related definition header

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-01-2002    created
    John Chang  08-05-2003    add definition for 11g & other drafts
*/
#ifndef __RTMP_DEF_H__
#define __RTMP_DEF_H__

#include "oid.h"

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif


#define BAND_5G		1
#define BAND_24G	2
#define BAND_BOTH	(BAND_5G | BAND_24G)


#ifdef SNMP_SUPPORT
/* for snmp, to get manufacturer OUI, 2008_0220 */
#define ManufacturerOUI_LEN			3
#define ManufacturerNAME			("Ralink Technology Company.")
#define	ResourceTypeIdName			("Ralink_ID")
#endif

#define RALINK_2883_VERSION		((UINT32)0x28830300)
#define RALINK_2880E_VERSION	((UINT32)0x28720200)
#define RALINK_3883_VERSION		((UINT32)0x38830400)
#define RALINK_3070_VERSION		((UINT32)0x30700200)

#define MAX_RX_PKT_LEN	1520


#define PCI_VIRT_TO_PHYS(__Addr)	(((UINT32)(__Addr)) & 0x0FFFFFFF)



#ifdef MULTIPLE_CARD_SUPPORT
/* MC: Multple Cards */
#define MAX_NUM_OF_MULTIPLE_CARD		32
#endif /* MULTIPLE_CARD_SUPPORT */

#ifdef MEMORY_OPTIMIZATION
#define MAX_RX_PROCESS		32
#else
#ifdef BB_SOC
#define MAX_RX_PROCESS		64
#else
#define MAX_RX_PROCESS		128	/*64 //32 */
#endif /* BB_SOC */
#endif
#define NUM_OF_LOCAL_TXBUF      2
#define TXD_SIZE		16	/* TXD_SIZE = TxD + TxInfo */
#define RXD_SIZE		16	

#define RXINFO_OFFSET	12

/* TXINFO_SIZE + TXWI_SIZE + 802.11 Header Size + AMSDU sub frame header */
#define TX_DMA_1ST_BUFFER_SIZE  96	/* only the 1st physical buffer is pre-allocated */

/*#define MGMT_DMA_BUFFER_SIZE    1536 //2048 */
/*
	Note 20100212 by SampleLin: do not set MGMT_DMA_BUFFER_SIZE smaller than
	1600; Or kernel will crash in deaggregate_AMSDU_announce() for EAPOL packet
	in enterprise WPA mode.
*/
#define MGMT_DMA_BUFFER_SIZE    1600	/*2048 */

#define RX_BUFFER_AGGRESIZE     3840	/*3904 //3968 //4096 //2048 //4096 */
#define RX_BUFFER_NORMSIZE      3840	/*3904 //3968 //4096 //2048 //4096 */
#define TX_BUFFER_NORMSIZE		RX_BUFFER_NORMSIZE
#define MAX_FRAME_SIZE          2346	/* Maximum 802.11 frame size */
#define MAX_AGGREGATION_SIZE    3840	/*3904 //3968 //4096 */
#define MAX_NUM_OF_TUPLE_CACHE  2
#define MAX_MCAST_LIST_SIZE     32
#define MAX_LEN_OF_VENDOR_DESC  64
/*#define MAX_SIZE_OF_MCAST_PSQ   (NUM_OF_LOCAL_TXBUF >> 2) // AP won't spend more than 1/4 of total buffers on M/BCAST PSQ */
#define MAX_SIZE_OF_MCAST_PSQ               32

#define MAX_RX_PROCESS_CNT	(RX_RING_SIZE)

#ifdef WLAN_SKB_RECYCLE
#define NUM_RX_DESC     128
#endif /* WLAN_SKB_RECYCLE */

#define NUM_OF_UP 9 /*number of user priority: 2set WMM+ non QoS*/


/*
	WMM Note: If memory of your system is not much, please reduce the definition;
	or when you do WMM test, the queue for low priority AC will be full, i.e.
	TX_RING_SIZE + MAX_PACKETS_IN_QUEUE packets for the AC will be buffered in
	WLAN, maybe no any packet buffer can be got in Ethernet driver.

	Sometimes no packet buffer can be got in Ethernet driver, the system will
	send flow control packet to the sender to slow down its sending rate.
	So no WMM can be saw in the air.
*/

/*
	Need to use 64 in vxworks for test case WMM A5-T07
	Two dnlink (10Mbps) from a WMM station to a non-WMM station.
	If use 256, queue is not enough.
	And in rt_main_end.c, clConfig.clNum = RX_RING_SIZE * 3; is changed to
	clConfig.clNum = RX_RING_SIZE * 4;
*/

#define MAX_PACKETS_IN_MCAST_PS_QUEUE		128
#ifdef BB_SOC
#define MAX_PACKETS_IN_PS_QUEUE				64
#else /* BB_SOC */
#define MAX_PACKETS_IN_PS_QUEUE				128	/*32 */
#endif /* !BB_SOC */
#define WMM_NUM_OF_AC                       4	/* AC0, AC1, AC2, and AC3 */

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
#ifdef MEMORY_OPTIMIZATION
#define MAX_LEN_OF_MULTICAST_FILTER_TABLE 16
#else
#define MAX_LEN_OF_MULTICAST_FILTER_TABLE 64
#endif
/* Size of hash tab must be power of 2. */
#define MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE ((MAX_LEN_OF_MULTICAST_FILTER_TABLE) * 2)
#define FREE_MEMBER_POOL_SIZE 64
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#define MAX_AGG_3SS_BALIMIT		31

/* RxFilter */
#define STANORMAL	 0x17f97
#ifdef MIXMODE_SUPPORT
#define APNORMAL	 0x1ce70a
#else 
#define APNORMAL	 0x15f97
#endif

#ifdef EXT_BUILD_CHANNEL_LIST
#define MAX_PRECONFIG_DESP_ENTRY_SIZE  11
#endif /* EXT_BUILD_CHANNEL_LIST */


/*
	RTMP_ADAPTER flags
*/
#define fRTMP_ADAPTER_MAP_REGISTER				0x00000001
#define fRTMP_ADAPTER_INTERRUPT_IN_USE			0x00000002
#define fRTMP_HW_ERR								0x00000004
#define fRTMP_SG									0x00000008	/* Scatter and Gather */

#define fRTMP_PKT_TX_ERR							0x00000010
#define fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS	0x00000020
#define fRTMP_ADAPTER_HALT_IN_PROGRESS			0x00000040
#define fRTMP_ADAPTER_RESET_IN_PROGRESS			0x00000080

#define fRTMP_ADAPTER_NIC_NOT_EXIST				0x00000100
//#define fRTMP_ADAPTER_TX_RING_ALLOCATED			0x00000200
#define fRTMP_ADAPTER_REMOVE_IN_PROGRESS		0x00000400
#define fRTMP_ADAPTER_MIMORATE_INUSED			0x00000800

//#define fRTMP_ADAPTER_RX_RING_ALLOCATED			0x00001000
#define fRTMP_ADAPTER_INTERRUPT_ACTIVE			0x00002000
#define fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS		0x00004000
#define	fRTMP_ADAPTER_REASSOC_IN_PROGRESS		0x00008000

#define	fRTMP_ADAPTER_MEDIA_STATE_PENDING		0x00010000
#define	fRTMP_ADAPTER_RADIO_OFF					0x00020000
#define fRTMP_ADAPTER_BULKOUT_RESET				0x00040000
#define	fRTMP_ADAPTER_BULKIN_RESET				0x00080000

#define fRTMP_ADAPTER_RDG_ACTIVE					0x00100000
#define fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE	0x00200000
#define fRTMP_ADAPTER_RALINK_BURST_MODE			0x00400000
#define fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET  	0x00800000

#define fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD		0x01000000
#define fRTMP_ADAPTER_CMD_RADIO_OFF				0x02000000
#define fRTMP_ADAPTER_SCAN_2040					0x04000000
#define	fRTMP_ADAPTER_RADIO_MEASUREMENT		0x08000000

#define fRTMP_ADAPTER_START_UP					0x10000000	/*Devive already initialized and enabled Tx/Rx. */
//#ifdef CONFIG_PM
//#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
#define fRTMP_ADAPTER_SUSPEND 					0x20000000
//#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
//#endif /* CONFIG_PM */
//#define fRTMP_ADAPTER_MEDIA_STATE_CHANGE		0x20000000
#define fRTMP_ADAPTER_IDLE_RADIO_OFF			0x40000000
#define fRTMP_ADAPTER_POLL_IDLE					0x80000000

enum ASIC_CAP{
	fASIC_CAP_RX_SC = 0x1,
	fASIC_CAP_CSO = 0x2,
	fASIC_CAP_TSO = 0x4,
	fASIC_CAP_MCS_LUT = 0x8,

	fASIC_CAP_PMF_ENC = 0x10,
};

#define fRTMP_ADAPTER_DISABLE_DOT_11N	0x00000001
#define fRTMP_ADAPTER_WSC_PBC_PIN0		0x00000002
#define fRTMP_ADAPTER_DISABLE_DEQUEUE	0x00000004

enum PHY_CAP{
	fPHY_CAP_24G = 0x1,
	fPHY_CAP_5G = 0x2,

	fPHY_CAP_HT = 0x10,
	fPHY_CAP_VHT = 0x20,

	fPHY_CAP_TXBF = 0x100,
	fPHY_CAP_LDPC = 0x200,
};

enum HIF_TYPE{
	HIF_RTMP = 0x0,
	HIF_RLT = 0x1,
	HIF_MT = 0x2,
};

enum MAC_TYPE{
	MAC_RTMP = 0x0,
	MAC_MT = 0x1,
};

enum RF_TYPE {
	RF_RT,
	RF_RLT,
	RF_MT76x2,
	RF_MT,
};

enum BBP_TYPE{
	BBP_RTMP = 0x0,
	BBP_RLT = 0x1,
	BBP_MT = 0x2,
};

#define PHY_CAP_2G(_x)		(((_x) & fPHY_CAP_24G) == fPHY_CAP_24G)
#define PHY_CAP_5G(_x)		(((_x) & fPHY_CAP_5G) == fPHY_CAP_5G)
#define PHY_CAP_N(_x)		(((_x) & fPHY_CAP_HT) == fPHY_CAP_HT)
#define PHY_CAP_AC(_x)		(((_x) & fPHY_CAP_VHT) == fPHY_CAP_VHT)

enum WIFI_MODE{
	WMODE_INVALID = 0,
	WMODE_A = 1 << 0,
	WMODE_B = 1 << 1,
	WMODE_G = 1 << 2,
	WMODE_GN = 1 << 3,
	WMODE_AN = 1 << 4,
	WMODE_AC = 1 << 5,
	WMODE_COMP = 6,	/* total types of supported wireless mode, add this value once yow add new type */
};

#define WMODE_CAP_5G(_x)			(((_x) & (WMODE_A | WMODE_AN | WMODE_AC)) != 0)
#define WMODE_CAP_2G(_x)			(((_x) & (WMODE_B | WMODE_G | WMODE_GN)) != 0)
#define WMODE_CAP_N(_x)			(((_x) & (WMODE_GN | WMODE_AN)) != 0)
#define WMODE_CAP_AC(_x)		(((_x) & (WMODE_AC)) != 0)
#define WMODE_CAP(_x, _mode)	(((_x) & (_mode)) != 0)

#define WMODE_EQUAL(_x, _mode)	((_x) == (_mode))

#define WMODE_5G_ONLY(_x)		(((_x) & (WMODE_B | WMODE_G | WMODE_GN)) == 0)
#define WMODE_2G_ONLY(_x)		(((_x) & (WMODE_A | WMODE_AN | WMODE_AC)) == 0)
#define WMODE_HT_ONLY(_x)		(((_x) & (~(WMODE_GN | WMODE_AN | WMODE_AC))) == 0)
#define WMODE_VHT_ONLY(_x)		(((_x) & (~(WMODE_AC))) == 0)


/*
	STA operation status flags
*/
#define fOP_STATUS_INFRA_ON                 0x00000001
#define fOP_STATUS_ADHOC_ON                 0x00000002
#define fOP_STATUS_BG_PROTECTION_INUSED     0x00000004
#define fOP_STATUS_SHORT_SLOT_INUSED        0x00000008
#define fOP_STATUS_SHORT_PREAMBLE_INUSED    0x00000010
#define fOP_STATUS_RECEIVE_DTIM             0x00000020
/*#define fOP_STATUS_TX_RATE_SWITCH_ENABLED   0x00000040 */
#define fOP_STATUS_MEDIA_STATE_CONNECTED    0x00000080
#define fOP_STATUS_WMM_INUSED               0x00000100
#define fOP_STATUS_DOZE                     0x00000400	/* debug purpose */
#define fOP_STATUS_APSD_INUSED				0x00001000
#define fOP_STATUS_TX_AMSDU_INUSED			0x00002000
#define fOP_STATUS_MAX_RETRY_ENABLED		0x00004000
#define fOP_STATUS_WAKEUP_NOW               0x00008000
#define fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE       0x00020000

#ifdef DOT11V_WNM_SUPPORT
#define fOP_STATUS_FMS_ENABLE       0x00400000
#endif /* DOT11V_WNM_SUPPORT */
#define fOP_AP_STATUS_MEDIA_STATE_CONNECTED	0x00200000



/*
	RTMP_ADAPTER PSFlags : related to advanced power save
*/
/* Indicate whether driver can go to sleep mode from now. This flag is useful AFTER link up */
#define fRTMP_PS_CAN_GO_SLEEP          0x00000001
/* Indicate whether driver has issue a LinkControl command to PCIe L1 */
#define fRTMP_PS_SET_PCI_CLK_OFF_COMMAND          0x00000002
/* Indicate driver should disable kick off hardware to send packets from now. */
#define fRTMP_PS_DISABLE_TX         0x00000004
/* Indicate driver should IMMEDIATELY fo to sleep after receiving AP's beacon in which  doesn't indicate unicate nor multicast packets for me */
/* This flag is used ONLY in RTMPHandleRxDoneInterrupt routine. */
#define fRTMP_PS_GO_TO_SLEEP_NOW         0x00000008
#define fRTMP_PS_TOGGLE_L1		0x00000010	/* Use Toggle L1 mechanism for rt28xx PCIe */

#define fRTMP_PS_MCU_SLEEP		0x00000020

#define WAKE_MCU_CMD				0x31
#define SLEEP_MCU_CMD				0x30
#define RFOFF_MCU_CMD				0x35

#ifdef DOT11N_DRAFT3
#define fOP_STATUS_SCAN_2040               	    0x00040000
#endif /* DOT11N_DRAFT3 */

#define CCKSETPROTECT		0x1
#define OFDMSETPROTECT		0x2
#define MM20SETPROTECT		0x4
#define MM40SETPROTECT		0x8
#define GF20SETPROTECT		0x10
#define GR40SETPROTECT		0x20
#define ALLN_SETPROTECT		(GR40SETPROTECT | GF20SETPROTECT | MM40SETPROTECT | MM20SETPROTECT)

/*
	AP's client table operation status flags
*/
#define fCLIENT_STATUS_WMM_CAPABLE			0x00000001	/* CLIENT can parse QOS DATA frame */
#define fCLIENT_STATUS_AGGREGATION_CAPABLE	0x00000002	/* CLIENT can receive Ralink's proprietary TX aggregation frame */
#define fCLIENT_STATUS_PIGGYBACK_CAPABLE		0x00000004	/* CLIENT support piggy-back */
#define fCLIENT_STATUS_AMSDU_INUSED			0x00000008
#define fCLIENT_STATUS_SGI20_CAPABLE			0x00000010
#define fCLIENT_STATUS_SGI40_CAPABLE			0x00000020
#define fCLIENT_STATUS_TxSTBC_CAPABLE			0x00000040
#define fCLIENT_STATUS_RxSTBC_CAPABLE			0x00000080
#define fCLIENT_STATUS_HTC_CAPABLE			0x00000100
#define fCLIENT_STATUS_RDG_CAPABLE			0x00000200
#define fCLIENT_STATUS_MCSFEEDBACK_CAPABLE	0x00000400
#define fCLIENT_STATUS_APSD_CAPABLE			0x00000800	/* UAPSD STATION */

#ifdef DOT11N_DRAFT3
#define fCLIENT_STATUS_BSSCOEXIST_CAPABLE	0x00001000
#endif /* DOT11N_DRAFT3 */
#define fCLIENT_STATUS_SOFTWARE_ENCRYPT		0x00002000	/* Indicate the client encrypt/decrypt by software */
#ifdef DOT11W_PMF_SUPPORT
#define fCLIENT_STATUS_PMF_CAPABLE			0x00004000
#define fCLIENT_STATUS_USE_SHA256				0x00008000
#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11_VHT_AC
#define fCLIENT_STATUS_SGI80_CAPABLE			0x00010000
#define fCLIENT_STATUS_SGI160_CAPABLE			0x00020000
#define fCLIENT_STATUS_VHT_TXSTBC_CAPABLE	0x00040000
#define fCLIENT_STATUS_VHT_RXSTBC_CAPABLE	0x00080000
#endif /* DOT11_VHT_AC */

#define fCLIENT_STATUS_RALINK_CHIPSET			0x00100000

#ifdef CLIENT_WDS
#define fCLIENT_STATUS_CLI_WDS					0x00200000
#endif /* CLIENT_WDS */


#define fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE		0x00800000
#define fCLIENT_STATUS_HT_RX_LDPC_CAPABLE		0x01000000

#ifdef CONFIG_HOTSPOT_R2
#define fCLIENT_STATUS_OSEN_CAPABLE             0x02000000
#endif

/*
	STA configuration flags
*/
/*#define fSTA_CFG_ENABLE_TX_BURST          0x00000001 */

/* 802.11n Operating Mode Definition. 0-3 also used in ASICUPdateProtect switch case */
#define HT_NO_PROTECT	0
#define HT_LEGACY_PROTECT	1
#define HT_40_PROTECT	2
#define HT_2040_PROTECT	3
#define HT_RTSCTS_6M	7
/*following is our own definition in order to turn on our ASIC protection register in INFRASTRUCTURE. */
#define HT_ATHEROS	8	/* rt2860c has problem with atheros chip. we need to turn on RTS/CTS . */
#define HT_FORCERTSCTS	9	/* Force turn on RTS/CTS first. then go to evaluate if this force RTS is necessary. */

/*
	RX Packet Filter control flags. Apply on pAd->PacketFilter
*/
#define fRX_FILTER_ACCEPT_DIRECT            NDIS_PACKET_TYPE_DIRECTED
#define fRX_FILTER_ACCEPT_MULTICAST         NDIS_PACKET_TYPE_MULTICAST
#define fRX_FILTER_ACCEPT_BROADCAST         NDIS_PACKET_TYPE_BROADCAST
#define fRX_FILTER_ACCEPT_ALL_MULTICAST     NDIS_PACKET_TYPE_ALL_MULTICAST
#define fRX_FILTER_ACCEPT_PROMISCUOUS       NDIS_PACKET_TYPE_PROMISCUOUS

/*
	Error code section
*/
/* NDIS_ERROR_CODE_ADAPTER_NOT_FOUND */
#define ERRLOG_READ_PCI_SLOT_FAILED     0x00000101L
#define ERRLOG_WRITE_PCI_SLOT_FAILED    0x00000102L
#define ERRLOG_VENDOR_DEVICE_NOMATCH    0x00000103L

/* NDIS_ERROR_CODE_ADAPTER_DISABLED */
#define ERRLOG_BUS_MASTER_DISABLED      0x00000201L

/* NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION */
#define ERRLOG_INVALID_SPEED_DUPLEX     0x00000301L
#define ERRLOG_SET_SECONDARY_FAILED     0x00000302L

/* NDIS_ERROR_CODE_OUT_OF_RESOURCES */
#define ERRLOG_OUT_OF_MEMORY            0x00000401L
#define ERRLOG_OUT_OF_SHARED_MEMORY     0x00000402L
#define ERRLOG_OUT_OF_MAP_REGISTERS     0x00000403L
#define ERRLOG_OUT_OF_BUFFER_POOL       0x00000404L
#define ERRLOG_OUT_OF_NDIS_BUFFER       0x00000405L
#define ERRLOG_OUT_OF_PACKET_POOL       0x00000406L
#define ERRLOG_OUT_OF_NDIS_PACKET       0x00000407L
#define ERRLOG_OUT_OF_LOOKASIDE_MEMORY  0x00000408L

/* NDIS_ERROR_CODE_HARDWARE_FAILURE */
#define ERRLOG_SELFTEST_FAILED          0x00000501L
#define ERRLOG_INITIALIZE_ADAPTER       0x00000502L
#define ERRLOG_REMOVE_MINIPORT          0x00000503L

/* NDIS_ERROR_CODE_RESOURCE_CONFLICT */
#define ERRLOG_MAP_IO_SPACE             0x00000601L
#define ERRLOG_QUERY_ADAPTER_RESOURCES  0x00000602L
#define ERRLOG_NO_IO_RESOURCE           0x00000603L
#define ERRLOG_NO_INTERRUPT_RESOURCE    0x00000604L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L

/* WDS definition */
#define MAX_WDS_ENTRY               0
#ifdef WDS_SUPPORT
#undef MAX_WDS_ENTRY
#define MAX_WDS_ENTRY               4
#endif /* WDS_SUPPORT */
#define WDS_PAIRWISE_KEY_OFFSET     60	/* WDS links uses pairwise key#60 ~ 63 in ASIC pairwise key table */

#define	WDS_DISABLE_MODE            0
#define	WDS_RESTRICT_MODE           1
#define	WDS_BRIDGE_MODE             2
#define	WDS_REPEATER_MODE           3
#define	WDS_LAZY_MODE               4

#define MAX_MESH_NUM				0

#define MAX_APCLI_NUM				0
#ifdef APCLI_SUPPORT
#undef MAX_APCLI_NUM
#ifdef MULTI_APCLI_SUPPORT
#define MAX_APCLI_NUM				2
#else /* MULTI_APCLI_SUPPORT*/
#define MAX_APCLI_NUM				1
#endif /* !MULTI_APCLI_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef APCLI_SAE_SUPPORT
enum _REPEATER_MLME_ENQ_IDX_CHK_TBL {
	REPT_MLME_START_IDX = 64,
	REPT_MLME_LAST_IDX = 95,
	REPT_MLME_MAX_IDX = 96,/*shall not over this.*/
};
#endif

#define MAX_P2P_NUM				0

#define MAX_MBSSID_NUM(__pAd)		1

#ifdef MAC_APCLI_SUPPORT
#define APCLI_BSS_BASE				8
#else
#define APCLI_BSS_BASE				0
#endif /* MAC_APCLI_SUPPORT */


#ifdef MBSS_SUPPORT
#undef MAX_MBSSID_NUM
#define MAX_MBSSID_NUM(__pAd)		((__pAd)->chipCap.BcnMaxNum)
#define HW_BEACON_MAX_COUNT(__pAd)	((__pAd)->chipCap.BcnMaxHwNum)
#else
#define HW_BEACON_MAX_COUNT(__pAd)	8
#endif /* MBSS_SUPPORT */
#ifdef ECONET_ALPHA_RELEASE
#define HW_BEACON_MAX_NUM			8
#else
#define HW_BEACON_MAX_NUM			16
#endif /* ECONET_ALPHA_RELEASE */


#define WDEV_NUM_MAX		(HW_BEACON_MAX_NUM + MAX_WDS_ENTRY + \
							MAX_APCLI_NUM + MAX_P2P_NUM + MAX_MESH_NUM)


/* sanity check for apidx */
#define MBSS_MR_APIDX_SANITY_CHECK(__pAd, apidx) \
    { if ((apidx >= MAX_MBSSID_NUM(__pAd)) || \
		(apidx >= HW_BEACON_MAX_NUM)) { \
          DBGPRINT(RT_DEBUG_ERROR, ("%s> Error! apidx = %d > MAX_MBSSID_NUM!\n", __FUNCTION__, apidx)); \
	  apidx = MAIN_MBSSID; } }

#define VALID_WCID(_wcid)	((_wcid) > 0 && (_wcid) < MAX_LEN_OF_MAC_TABLE )

#define VALID_MBSS(_pAd, _apidx)	((_apidx < MAX_MBSSID_NUM(_pAd)) && (_apidx < HW_BEACON_MAX_NUM))

#define MAX_BEACON_SIZE				512
#define MAX_TIM_SIZE				256 
/* Carter: TIM ie max length could achieve 257 byte in spec,
   but it will never reach this value in Ralink Driver */


#define HW_RESERVED_WCID(__pAd)		((__pAd)->chipCap.WcidHwRsvNum)

/* Then dedicate wcid of DFS and Carrier-Sense. */
#define DFS_CTS_WCID(__pAd) 		(HW_RESERVED_WCID(__pAd) - 1)
#define CS_CTS_WCID(__pAd) 			(HW_RESERVED_WCID(__pAd) - 2)

#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA)
#define LAST_SPECIFIC_WCID(__pAd)	20
#define MAX_AVAILABLE_CLIENT_WCID(__pAd)	(LAST_SPECIFIC_WCID(__pAd) - MAX_MBSSID_NUM(__pAd))
#else
#define LAST_SPECIFIC_WCID(__pAd)	(HW_RESERVED_WCID(__pAd) - 2)

/* If MAX_MBSSID_NUM is 8, the maximum available wcid for the associated STA is 211. */
/* If MAX_MBSSID_NUM is 7, the maximum available wcid for the associated STA is 228. */
#define MAX_AVAILABLE_CLIENT_WCID(__pAd)	(LAST_SPECIFIC_WCID(__pAd) - MAX_MBSSID_NUM(__pAd) - 1)
#endif /* MT7603_FPGA */


/* TX need WCID to find Cipher Key */
/* these wcid 212 ~ 219 are reserved for bc/mc packets if MAX_MBSSID_NUM is 8. */
#define GET_GroupKey_WCID(__pAd, __wcid, __bssidx) \
	{										\
		__wcid = LAST_SPECIFIC_WCID(__pAd) - (MAX_MBSSID_NUM(__pAd)) + __bssidx;	\
	}

#ifdef DOT11W_PMF_SUPPORT
#define GET_PMF_GroupKey_WCID(__pAd, __wcid, __bssidx) \
	{										\
		__wcid = LAST_SPECIFIC_WCID(__pAd) - (MAX_MBSSID_NUM(__pAd) * 2) + __bssidx;	\
	}
#endif /* DOT11W_PMF_SUPPORT */

/*#define IsGroupKeyWCID(__pAd, __wcid) (((__wcid) < LAST_SPECIFIC_WCID) && ((__wcid) >= (LAST_SPECIFIC_WCID - (MAX_MBSSID_NUM(__pAd))))) */

/* definition to support multiple BSSID */
#define BSS0                            0
#define BSS1                            1
#define BSS2                            2
#define BSS3                            3
#define BSS4                            4
#define BSS5                            5
#define BSS6                            6
#define BSS7                            7

/*============================================================ */
/* Length definitions */
#define PEER_KEY_NO                     2
#define TIMESTAMP_LEN                   8
#define MAX_LEN_OF_SUPPORTED_RATES      MAX_LENGTH_OF_SUPPORT_RATES	/* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
#define MAX_NUM_OF_REGULATORY_CLASS		16
#define MAX_LEN_OF_KEY                  32	/* 32 octets == 256 bits, Redefine for WPA */
/* #define MAX_NUM_OF_CHANNELS             MAX_NUM_OF_CHS */	/* 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL termination */
#define MAX_NUM_OF_11JCHANNELS             20	/* 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL termination */
#define MAX_LEN_OF_SSID                 32
#define CIPHER_TEXT_LEN                 128
#define HASH_TABLE_SIZE                 256	/* Size of hash tab must be power of 2. */
#define MAX_VIE_LEN                     1024	/* New for WPA cipher suite variable IE sizes. */
#define MAX_SUPPORT_MCS             32
#define MAX_NUM_OF_BBP_LATCH             256
#undef MAX_NUM_OF_BBP_LATCH
#define MAX_NUM_OF_BBP_LATCH             255

#define MAX_LEN_OF_CCK_RATES	4
#define MAX_LEN_OF_OFDM_RATES	8
#define MAX_LEN_OF_HT_RATES		24
#ifdef DOT11_VHT_AC
#define MAX_LEN_OF_VHT_RATES		20
#endif /* DOT11_VHT_AC */
#define SUPPORT_CCK_MODE	1
#define SUPPORT_OFDM_MODE	2
#define SUPPORT_HT_MODE		4
#define SUPPORT_VHT_MODE		8

/*============================================================ */
/* ASIC WCID Table definition. */
/*============================================================ */
#define BSSID_WCID		1	/* in infra mode, always put bssid with this WCID */
#define MCAST_WCID	0x0

#ifdef MULTI_APCLI_SUPPORT
#define APCLI_MCAST_WCID(_num)\
	(MAX_LEN_OF_MAC_TABLE + HW_BEACON_MAX_NUM + MAX_APCLI_NUM + _num)
#else /* MULTI_APCLI_SUPPORT */
#define APCLI_MCAST_WCID    (MAX_LEN_OF_MAC_TABLE + HW_BEACON_MAX_NUM + MAX_APCLI_NUM)
#endif /* !MULTI_APCLI_SUPPORT */
#define BSS0Mcast_WCID	0x0
#define BSS1Mcast_WCID	0xf8
#define BSS2Mcast_WCID	0xf9
#define BSS3Mcast_WCID	0xfa
#define BSS4Mcast_WCID	0xfb
#define BSS5Mcast_WCID	0xfc
#define BSS6Mcast_WCID	0xfd
#define BSS7Mcast_WCID	0xfe
#define RESERVED_WCID	0xff
#define WCID_ALL		0xff


#define MAX_NUM_OF_ACL_LIST				MAX_NUMBER_OF_ACL

#if defined(MAC_REPEATER_SUPPORT) /*&& !defined(MT_MAC)*/
#define MAX_LEN_OF_MAC_TABLE (MAX_NUMBER_OF_MAC + ((MAX_EXT_MAC_ADDR_SIZE + 1) * MAX_APCLI_NUM))
#else
#define MAX_LEN_OF_MAC_TABLE            MAX_NUMBER_OF_MAC	/* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#endif

#define MAX_LEN_OF_TR_TABLE 	(MAX_LEN_OF_MAC_TABLE + HW_BEACON_MAX_NUM)

#define VALID_TR_WCID(_wcid)		((_wcid) < MAX_LEN_OF_TR_TABLE)

/*#if MAX_LEN_OF_MAC_TABLE>MAX_AVAILABLE_CLIENT_WCID */
/*#error MAX_LEN_OF_MAC_TABLE can not be larger than MAX_AVAILABLE_CLIENT_WCID!!!! */
/*#endif */

#define MAX_NUM_OF_WDS_LINK_PERBSSID	            3
/*#define MAX_NUM_OF_WDS_LINK	            (MAX_NUM_OF_WDS_LINK_PERBSSID*MAX_MBSSID_NUM) // no use */
#define MAX_NUM_OF_EVENT                MAX_NUMBER_OF_EVENT
#define WDS_LINK_START_WCID				(MAX_LEN_OF_MAC_TABLE-1)

#define NUM_OF_TID			8
#define MAX_AID_BA                    4

#define MAX_LEN_OF_BA_REC_TABLE          ((NUM_OF_TID * MAX_LEN_OF_MAC_TABLE)/2)	/*   (NUM_OF_TID*MAX_AID_BA + 32)        //Block ACK recipient */
#define MAX_LEN_OF_BA_ORI_TABLE          ((NUM_OF_TID * MAX_LEN_OF_MAC_TABLE)/2)	/*   (NUM_OF_TID*MAX_AID_BA + 32)   // Block ACK originator */

#ifdef MEMORY_OPTIMIZATION
#define MAX_LEN_OF_BSS_TABLE             1
#define MAX_REORDERING_MPDU_NUM			 256
#else
#ifdef BB_SOC
#ifdef NEIGHBORING_AP_STAT
#define MAX_LEN_OF_BSS_TABLE             64
#else
#define MAX_LEN_OF_BSS_TABLE             20
#endif /*NEIGHBORING_AP_STAT*/
#else /* BB_SOC */
#ifdef ECONET_ALPHA_RELEASE
#define MAX_LEN_OF_BSS_TABLE             96 /* 64 */
#else
#define MAX_LEN_OF_BSS_TABLE             128 /* 64 */
#endif /* ECONET_ALPHA_RELEASE */
#endif /* !BB_SOC */
#define MAX_REORDERING_MPDU_NUM			 512
#endif

#if defined(MAX_CONTINUOUS_TX_CNT) || defined(NEW_IXIA_METHOD)
#undef MAX_REORDERING_MPDU_NUM
#define MAX_REORDERING_MPDU_NUM 2048
#endif
/* key related definitions */
#define SHARE_KEY_NUM                   4
#define MAX_LEN_OF_SHARE_KEY            16	/* byte count */
#define MAX_LEN_OF_PEER_KEY             16	/* byte count */
#define PAIRWISE_KEY_NUM                64	/* in MAC ASIC pairwise key table */
#define GROUP_KEY_NUM                   4
#ifdef WH_EZ_SETUP
#define LEN_PSK							64
#endif
#define PMK_LEN                         32
#define WDS_PAIRWISE_KEY_OFFSET         60	/* WDS links uses pairwise key#60 ~ 63 in ASIC pairwise key table */
#define	PMKID_NO                        4	/* Number of PMKID saved supported */
#define MAX_LEN_OF_MLME_BUFFER          2048

/* power status related definitions */
#define PWR_ACTIVE                      0
#define PWR_SAVE                        1
#define PWR_MMPS                        2	/*MIMO power save */

/* For AP PS retrieve status */
#define APPS_RETRIEVE_IDLE       0
#define APPS_RETRIEVE_CR_PADDING	1
#define APPS_RETRIEVE_START_PS		2
#define APPS_RETRIEVE_GOING			3
#define APPS_RETRIEVE_WAIT_EVENT	4
#define APPS_RETRIEVE_DONE			5

#ifdef MTFWD
enum nl_msg_id {
	FWD_CMD_ADD_TX_SRC = 3,
	FWD_CMD_DEL_TX_SRC = 4,
};
#endif

/*#define PWR_UNKNOWN                   2 */

/* Auth and Assoc mode related definitions */
#define AUTH_MODE_OPEN		0x00
#define AUTH_MODE_KEY			0x01
#define AUTH_MODE_FT			0x02
#define AUTH_MODE_SAE			0x03
#define AUTH_MODE_VENDOR		0xffff

/* BSS Type definitions */
#define BSS_ADHOC                       0	/* = Ndis802_11IBSS */
#define BSS_INFRA                       1	/* = Ndis802_11Infrastructure */
#define BSS_ANY                         2	/* = Ndis802_11AutoUnknown */
#define BSS_MONITOR			            3	/* = Ndis802_11Monitor */

/* Reason code definitions */
#define REASON_RESERVED                 0
#define REASON_UNSPECIFY                1
#define REASON_NO_LONGER_VALID          2
#define REASON_DEAUTH_STA_LEAVING       3
#define REASON_DISASSOC_INACTIVE        4
#define REASON_DISASSPC_AP_UNABLE       5
#define REASON_CLS2ERR                  6
#define REASON_CLS3ERR                  7
#define REASON_DISASSOC_STA_LEAVING     8
#define REASON_STA_REQ_ASSOC_NOT_AUTH   9
#define REASON_INVALID_IE               13
#define REASON_MIC_FAILURE              14
#define REASON_4_WAY_TIMEOUT            15
#define REASON_GROUP_KEY_HS_TIMEOUT     16
#define REASON_IE_DIFFERENT             17
#define REASON_MCIPHER_NOT_VALID        18
#define REASON_UCIPHER_NOT_VALID        19
#define REASON_AKMP_NOT_VALID           20
#define REASON_UNSUPPORT_RSNE_VER       21
#define REASON_INVALID_RSNE_CAP         22
#define REASON_8021X_AUTH_FAIL          23
#define REASON_CIPHER_SUITE_REJECTED    24
#define REASON_DECLINED                 37

#define REASON_QOS_UNSPECIFY              32
#define REASON_QOS_LACK_BANDWIDTH         33
#define REASON_POOR_CHANNEL_CONDITION     34
#define REASON_QOS_OUTSIDE_TXOP_LIMITION  35
#define REASON_QOS_QSTA_LEAVING_QBSS      36
#define REASON_QOS_UNWANTED_MECHANISM     37
#define REASON_QOS_MECH_SETUP_REQUIRED    38
#define REASON_QOS_REQUEST_TIMEOUT        39
#define REASON_QOS_CIPHER_NOT_SUPPORT     45

#ifdef WIFI_DIAG
#define REASON_STANDARD_MAX				0xFFFF	/* 65535 */
#define REASON_UNKNOWN					(REASON_STANDARD_MAX + 1)
#define REASON_AGING_TIME_OUT			(REASON_STANDARD_MAX + 2)
#define REASON_CONTINUE_TX_FAIL			(REASON_STANDARD_MAX + 3)
#define REASON_RSSI_TOO_LOW				(REASON_STANDARD_MAX + 4)
#define REASON_AUTH_WRONG_ALGORITHM		(REASON_STANDARD_MAX + 5)
#define REASON_CHALLENGE_FAIL			(REASON_STANDARD_MAX + 6)
#define REASON_4WAY_HS_MSG1_FAIL		(REASON_STANDARD_MAX + 7)
#define REASON_4WAY_HS_MSG2_FAIL		(REASON_STANDARD_MAX + 8)
#define REASON_4WAY_HS_MSG3_FAIL		(REASON_STANDARD_MAX + 9)
#define REASON_4WAY_HS_MSG4_FAIL		(REASON_STANDARD_MAX + 10)
#define REASON_2WAY_HS_MSG1_FAIL		(REASON_STANDARD_MAX + 11)
#define REASON_2WAY_HS_MSG2_FAIL		(REASON_STANDARD_MAX + 12)
#define REASON_DECRYPTION_FAIL			(REASON_STANDARD_MAX + 13)
#endif

#ifdef DOT11V_WNM_SUPPORT
#define REASON_DISASSOC_DUE_TO_BSS_TRANSITION_MANAGEMENT	12
#endif /* DOT11V_WNM_SUPPORT */

#define REASON_FT_INVALID_FTIE				55

/* Status code definitions */
#define MLME_SUCCESS                    0
#define MLME_UNSPECIFY_FAIL             1
#define MLME_SECURITY_WEAK              5
#define MLME_CANNOT_SUPPORT_CAP         10
#define MLME_REASSOC_DENY_ASSOC_EXIST   11
#define MLME_ASSOC_DENY_OUT_SCOPE       12
#define MLME_ALG_NOT_SUPPORT            13
#define MLME_SEQ_NR_OUT_OF_SEQUENCE     14
#define MLME_REJ_CHALLENGE_FAILURE      15
#define MLME_REJ_TIMEOUT                  16
#define MLME_ASSOC_REJ_UNABLE_HANDLE_STA  17
#define MLME_ASSOC_REJ_DATA_RATE          18

#define MLME_ASSOC_REJ_NO_EXT_RATE        22
#define MLME_ASSOC_REJ_NO_EXT_RATE_PBCC   23
#define MLME_ASSOC_REJ_NO_CCK_OFDM        24

#ifdef DOT11W_PMF_SUPPORT
#define MLME_ASSOC_REJ_TEMPORARILY		  30
#define MLME_ROBUST_MGMT_POLICY_VIOLATION 31
#endif /* DOT11W_PMF_SUPPORT */

#ifdef WH_EZ_SETUP
#ifdef NEW_CONNECTION_ALGO
#undef MLME_ASSOC_REJ_TEMPORARILY
#define MLME_ASSOC_REJ_TEMPORARILY		  30
#endif
#endif

#define MLME_QOS_UNSPECIFY                32
#define MLME_REQUEST_DECLINED             37
#define MLME_REQUEST_WITH_INVALID_PARAM   38
#define MLME_INVALID_INFORMATION_ELEMENT  40
#define MLME_INVALID_GROUP_CIPHER   	  41
#define MLME_INVALID_PAIRWISE_CIPHER   	  42
#define MLME_INVALID_AKMP   			  43
#define MLME_NOT_SUPPORT_RSN_VERSION	  44
#define	MLME_INVALID_RSN_CAPABILITIES	  45
#define MLME_INVALID_SECURITY_POLICY      46 /* Cipher suite rejected because of security policy */
#define MLME_DLS_NOT_ALLOW_IN_QBSS        48
#define MLME_DEST_STA_NOT_IN_QBSS         49
#define MLME_DEST_STA_IS_NOT_A_QSTA       50

#define MLME_INVALID_FORMAT             0x51
#define MLME_FAIL_NO_RESOURCE           0x52
#define MLME_STATE_MACHINE_REJECT       0x53
#define MLME_MAC_TABLE_FAIL             0x54

#ifdef DOT11_SAE_SUPPORT
#define MLME_INVALID_PMKID                53
#define MLME_ANTI_CLOGGING_TOKEN_REQ      76
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#define MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED 77
#endif

#ifdef CONFIG_OWE_SUPPORT
#define IE_WLAN_EXTENSION	255
#define IE_EXTENSION_ID_ECDH	32
#endif
#ifdef WH_EZ_SETUP
#define MLME_EZ_CONNECTION_LOOP			110
#define MLME_EZ_DISCONNECT_NON_EZ		111
#endif

/* IE code */
#define IE_SSID                         0
#define IE_SUPP_RATES                   1
#define IE_FH_PARM                      2
#define IE_DS_PARM                      3
#define IE_CF_PARM                      4
#define IE_TIM                          5
#define IE_IBSS_PARM                    6
#define IE_COUNTRY                      7	/* 802.11d */
#define IE_802_11D_REQUEST              10	/* 802.11d */
#define IE_QBSS_LOAD                    11	/* 802.11e d9 */
#define IE_EDCA_PARAMETER               12	/* 802.11e d9 */
#define IE_TSPEC                        13	/* 802.11e d9 */
#define IE_TCLAS                        14	/* 802.11e d9 */
#define IE_SCHEDULE                     15	/* 802.11e d9 */
#define IE_CHALLENGE_TEXT               16
#define IE_POWER_CONSTRAINT             32	/* 802.11h d3.3 */
#define IE_POWER_CAPABILITY             33	/* 802.11h d3.3 */
#define IE_TPC_REQUEST                  34	/* 802.11h d3.3 */
#define IE_TPC_REPORT                   35	/* 802.11h d3.3 */
#define IE_SUPP_CHANNELS                36	/* 802.11h d3.3 */
#define IE_CHANNEL_SWITCH_ANNOUNCEMENT  37	/* 802.11h d3.3 */
#define IE_MEASUREMENT_REQUEST          38	/* 802.11h d3.3 */
#define IE_MEASUREMENT_REPORT           39	/* 802.11h d3.3 */
#define IE_QUIET                        40	/* 802.11h d3.3 */
#define IE_IBSS_DFS                     41	/* 802.11h d3.3 */
#define IE_ERP                          42	/* 802.11g */
#define IE_TS_DELAY                     43	/* 802.11e d9 */
#define IE_TCLAS_PROCESSING             44	/* 802.11e d9 */
#define IE_QOS_CAPABILITY               46	/* 802.11e d6 */
#define IE_HT_CAP                       45	/* 802.11n d1. HT CAPABILITY. ELEMENT ID TBD */
#define IE_AP_CHANNEL_REPORT			51	/* 802.11k d6 */
#define IE_HT_CAP2                         52	/* 802.11n d1. HT CAPABILITY. ELEMENT ID TBD */
#define IE_RSN                          48	/* 802.11i d3.0 */
#define IE_WPA2                         48	/* WPA2 */
#define IE_EXT_SUPP_RATES               50	/* 802.11g */
#define IE_TIMEOUT_INTERVAL             56      /* 802.11w */
#define IE_SUPP_REG_CLASS               59	/* 802.11y. Supported regulatory classes. */
#define IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT	60	/* 802.11n */
#define IE_ADD_HT                         61	/* 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD */
#define IE_ADD_HT2                        53	/* 802.11n d1. ADDITIONAL HT CAPABILITY. ELEMENT ID TBD */

/* For 802.11n D3.03 */
/*#define IE_NEW_EXT_CHA_OFFSET             62    // 802.11n d1. New extension channel offset elemet */
#define IE_SECONDARY_CH_OFFSET		62	/* 802.11n D3.03        Secondary Channel Offset element */
#define IE_WAPI							68	/* WAPI information element. Same as Bss Ac Access Dealy Element. */
#define IE_TIME_ADVERTISEMENT			69	  // 802.11p
#define IE_2040_BSS_COEXIST               72	/* 802.11n D3.0.3 */
#define IE_2040_BSS_INTOLERANT_REPORT     73	/* 802.11n D3.03 */
#define IE_OVERLAPBSS_SCAN_PARM           74	/* 802.11n D3.03 */
#define IE_CHANNEL_USAGE					97	/* Cisco advertises suggested channel using this IE. */
#define IE_TIME_ZONE			98	/* 802.11V */
#define IE_INTERWORKING			107 /* 802.11u */
#define IE_ADVERTISEMENT_PROTO	108 /* 802.11u */
#define IE_QOS_MAP_SET			110 /* 802.11u */
#define IE_ROAMING_CONSORTIUM	111 /* 802.11u */
#define IE_EXT_CAPABILITY                127	/* 802.11n D3.03 */
#define IE_LAST_BCN_REPORT_INDICATION_REQUEST	164 /*Last Beacon Report Indication Request*/
#define IE_OPERATING_MODE_NOTIFY	199

#define IE_WPA                          221	/* WPA */
#define IE_VENDOR_SPECIFIC              221	/* Wifi WMM (WME) */
#define	IE_WFA_WSC							221

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11K_RRM_SUPPORT)
#define IE_FT_MDIE				54
#endif

#define OUI_P2P					0x09
#define OUI_HS2_INDICATION		0x10
#define OUI_BROADCOM_HT              51	/* */
#define OUI_BROADCOM_HTADD           52	/* */
#define OUI_PREN_HT_CAP              51	/* */
#define OUI_PREN_ADD_HT              52	/* */

/* CCX information */
#define IE_AIRONET_CKIP                 133	/* CCX1.0 ID 85H for CKIP */
#define IE_AP_TX_POWER                  150	/* CCX 2.0 for AP transmit power */
#define IE_MEASUREMENT_CAPABILITY       221	/* CCX 2.0 */
#define IE_CCX_V2                       221
#define IE_AIRONET_IPADDRESS            149	/* CCX ID 95H for IP Address */
#define IE_AIRONET_CCKMREASSOC          156	/* CCX ID 9CH for CCKM Reassociation Request element */
#define CKIP_NEGOTIATION_LENGTH         30
#define AIRONET_IPADDRESS_LENGTH        10
#define AIRONET_CCKMREASSOC_LENGTH      24

/* ======================================================== */
/* MLME state machine definition */
/* ======================================================== */

/* STA MLME state mahcines */
#define ASSOC_STATE_MACHINE             1
#define AUTH_STATE_MACHINE              2
#define AUTH_RSP_STATE_MACHINE          3
#define SYNC_STATE_MACHINE              4
#define MLME_CNTL_STATE_MACHINE         5
#define WPA_PSK_STATE_MACHINE           6
/*#define LEAP_STATE_MACHINE              7 */
#define AIRONET_STATE_MACHINE           8
#define ACTION_STATE_MACHINE           9

/* AP MLME state machines */
#define AP_ASSOC_STATE_MACHINE          11
#define AP_AUTH_STATE_MACHINE           12
#define AP_SYNC_STATE_MACHINE           14
#define AP_CNTL_STATE_MACHINE           15
#define WSC_STATE_MACHINE            17
#define WSC_UPNP_STATE_MACHINE		    18

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define APCLI_AUTH_STATE_MACHINE			19
#define APCLI_ASSOC_STATE_MACHINE			20
#define APCLI_SYNC_STATE_MACHINE			21
#define APCLI_CTRL_STATE_MACHINE			22
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#define WPA_STATE_MACHINE            		23



#ifdef DOT11R_FT_SUPPORT
#define FT_OTA_AUTH_STATE_MACHINE      	27
#define FT_OTD_ACT_STATE_MACHINE      	28
#endif /* DOT11R_FT_SUPPORT */




#ifdef DOT11V_WNM_SUPPORT
#define WNM_BSS_TM_STATE_MACHINE      	37
#endif /* DOT11V_WNM_SUPPORT */



#ifdef DOT11U_INTERWORKING
#define GAS_STATE_MACHINE		39
#endif

#ifdef CONFIG_DOT11V_WNM
#define BTM_STATE_MACHINE		40
#define WNM_NOTIFY_STATE_MACHINE	41
#endif

#ifdef CONFIG_HOTSPOT
#define HSCTRL_STATE_MACHINE		42
#endif

#ifdef MIXMODE_SUPPORT
#define MIX_MODE_STATE_MACHINE 50//48
#endif

#ifdef WH_EZ_SETUP
#ifdef EZ_MOD_SUPPORT
#define EZ_STATE_MACHINE		98
#else
//! Levarage from MP1.0 CL#170063
#define EZ_ROAM_STATE_MACHINE		98
#define AP_TRIBAND_STATE_MACHINE	99
#endif
#endif

#ifdef DOT11K_RRM_SUPPORT
#define NEIGHBOR_STATE_MACHINE  48
#define BCN_STATE_MACHINE  49
#endif

#ifdef WIFI_DIAG
#define WIFI_DAIG_STATE_MACHINE 51
#endif

/*
	STA's CONTROL/CONNECT state machine: states, events, total function #
*/
#define CNTL_IDLE                       0
#define CNTL_WAIT_DISASSOC              1
#define CNTL_WAIT_JOIN                  2
#define CNTL_WAIT_REASSOC               3
#define CNTL_WAIT_START                 4
#define CNTL_WAIT_AUTH                  5
#define CNTL_WAIT_ASSOC                 6
#define CNTL_WAIT_AUTH2                 7
#define CNTL_WAIT_OID_LIST_SCAN         8
#define CNTL_WAIT_OID_DISASSOC          9
#define CNTL_WAIT_SCAN_FOR_CONNECT      10


#define MT2_ASSOC_CONF                  34
#define MT2_AUTH_CONF                   35
#define MT2_DEAUTH_CONF                 36
#define MT2_DISASSOC_CONF               37
#define MT2_REASSOC_CONF                38
#define MT2_PWR_MGMT_CONF               39
#define MT2_JOIN_CONF                   40
#define MT2_SCAN_CONF                   41
#define MT2_START_CONF                  42
#define MT2_GET_CONF                    43
#define MT2_SET_CONF                    44
#define MT2_RESET_CONF                  45
#define MT2_FT_OTD_CONF					46
#define MT2_MLME_ROAMING_REQ            52

#define CNTL_FUNC_SIZE                  1

/*
	STA's ASSOC state machine: states, events, total function #
*/
#define ASSOC_IDLE                      0
#define ASSOC_WAIT_RSP                  1
#define REASSOC_WAIT_RSP                2
#define DISASSOC_WAIT_RSP               3
#define MAX_ASSOC_STATE                 4

#define ASSOC_MACHINE_BASE              0
#define MT2_MLME_ASSOC_REQ              0
#define MT2_MLME_REASSOC_REQ            1
#define MT2_MLME_DISASSOC_REQ           2
#define MT2_PEER_DISASSOC_REQ           3
#define MT2_PEER_ASSOC_REQ              4
#define MT2_PEER_ASSOC_RSP              5
#define MT2_PEER_REASSOC_REQ            6
#define MT2_PEER_REASSOC_RSP            7
#define MT2_DISASSOC_TIMEOUT            8
#define MT2_ASSOC_TIMEOUT               9
#define MT2_REASSOC_TIMEOUT             10
#define MAX_ASSOC_MSG                   11

#define ASSOC_FUNC_SIZE                 (MAX_ASSOC_STATE * MAX_ASSOC_MSG)

/*
	ACT state machine: states, events, total function #
*/
#define ACT_IDLE                      0
#define MAX_ACT_STATE                 1

#define ACT_MACHINE_BASE              0

/*
	Those PEER_xx_CATE number is based on real Categary value in IEEE spec.
	Please doesn't modify it by yourself.
 */
/*Category */
#define MT2_PEER_SPECTRUM_CATE	0
#define MT2_PEER_QOS_CATE			1
#define MT2_PEER_DLS_CATE			2
#define MT2_PEER_BA_CATE			3
#define MT2_PEER_PUBLIC_CATE		4
#define MT2_PEER_RM_CATE			5
/* "FT_CATEGORY_BSS_TRANSITION equal to 6" is defined file of "dot11r_ft.h" */
#define MT2_PEER_HT_CATE			7	/* 7.4.7 */
#define MT2_PEER_PMF_CATE			8	/* defined in IEEE 802.11w-D8.0 7.3.1.11 */
#define MT2_PEER_RESV_9			9
#define MT2_PEER_RESV_10			10
#define MT2_PEER_RESV_11			11
#define MT2_PEER_RESV_12			12
#define MT2_PEER_RESV_13			13
#define MT2_PEER_RESV_14			14
#define MT2_PEER_RESV_15			15
#define MT2_PEER_RESV_16			16
/*
	In WMM spec v1.1. the category must be 17
	(see Table 7 Management Action Frame Fields)
*/
#define MT2_PEER_WMM				17
#define WNM_CATEGORY_BSS_TRANSITION		18
#define MT2_PEER_RESV_19			19
#define MT2_PEER_RESV_20			20
#define MT2_PEER_VHT_CATE			21
#define MAX_IEEE_STD_CATE			21	/* Indicate the maximum category code defined in IEEE-802.11-Std */
#define MAX_PEER_CATE_MSG			MAX_IEEE_STD_CATE

#define MT2_MLME_ADD_BA_CATE		(MAX_IEEE_STD_CATE + 1)
#define MT2_MLME_ORI_DELBA_CATE	(MAX_IEEE_STD_CATE + 2)
#define MT2_MLME_REC_DELBA_CATE	(MAX_IEEE_STD_CATE + 3)
#define MT2_MLME_QOS_CATE			(MAX_IEEE_STD_CATE + 4)
#define MT2_MLME_DLS_CATE			(MAX_IEEE_STD_CATE + 5)
#define MT2_ACT_INVALID			(MAX_IEEE_STD_CATE + 6)

#define MAX_ACT_MSG				(MAX_IEEE_STD_CATE + 7)

#ifdef DOT11V_WNM_SUPPORT
#define WNM_CATEGORY_BSS_TRANSITION  			18
#undef MAX_ACT_MSG
#define MAX_ACT_MSG						(MAX_IEEE_STD_CATE + 8)
#undef MAX_PEER_CATE_MSG
#define MAX_PEER_CATE_MSG                   (MAX_IEEE_STD_CATE + 8)
#endif /* DOT11V_WNM_SUPPORT */

#define MT2_ACT_VENDOR				0x7F

/* Category field */
#define CATEGORY_SPECTRUM		0
#define CATEGORY_QOS			1
#define CATEGORY_DLS			2
#define CATEGORY_BA			3
#define CATEGORY_PUBLIC		4
#define CATEGORY_RM			5
#define CATEGORY_FT				6
#define CATEGORY_HT			7
#ifdef DOT11W_PMF_SUPPORT
#define CATEGORY_SA			8	/* defined in IEEE 802.11w-D8.0 7.3.1.11*/
#define CATEGORY_PD			9	/* Protected Dual of Action defined in IEEE 802.11w */
#define CATEGORY_WNM 10
#define CATEGORY_VSP			126	/* Vendor-specific Protected defined in IEEE 802.11w */
#endif // DOT11W_PMF_SUPPORT //
//#ifdef WFD_SUPPORT
#define CATEGORY_VENDOR_SPECIFIC_WFD	127 //CFG_TODO CATEGORY_VENDOR, WHY ONLY WFD ?
//#endif /* WFD_SUPPORT */


#ifdef DOT11_VHT_AC
#define CATEGORY_VHT		21

#define ACT_VHT_COMPRESS_BF		0	/* VHT Compressed Beamforming */
#define ACT_VHT_GRP_ID_MGMT		1	/* Group ID Management */
#define ACT_VHT_OPMODE_NOTIFY		2	/* Operating Mode Notification */
#endif /* DOT11_VHT_AC */

#ifdef DOT11W_PMF_SUPPORT
/* SA Query Action frame definition */
#define ACTION_SAQ_REQUEST			0
#define ACTION_SAQ_RESPONSE			1
#endif // DOT11W_PMF_SUPPORT //

/* DLS Action frame definition */
#define ACTION_DLS_REQUEST		0
#define ACTION_DLS_RESPONSE	1
#define ACTION_DLS_TEARDOWN	2

/* Spectrum  Action field value 802.11h 7.4.1 */
#define SPEC_MRQ	0	/* Request */
#define SPEC_MRP	1	/*Report */
#define SPEC_TPCRQ	2
#define SPEC_TPCRP	3
#define SPEC_CHANNEL_SWITCH	4

/* BA  Action field value */
#define ADDBA_REQ	0
#define ADDBA_RESP	1
#define DELBA   2

/* Public's  Action field value in Public Category.  Some in 802.11y and some in 11n */
#define ACTION_BSS_2040_COEXIST				0	/* 11n */
#define ACTION_DSE_ENABLEMENT					1	/* 11y D9.0 */
#define ACTION_DSE_DEENABLEMENT				2	/* 11y D9.0 */
#define ACTION_DSE_REG_LOCATION_ANNOUNCE	3	/* 11y D9.0 */
#define ACTION_EXT_CH_SWITCH_ANNOUNCE		4	/* 11y D9.0 */
#define ACTION_DSE_MEASUREMENT_REQ			5	/* 11y D9.0 */
#define ACTION_DSE_MEASUREMENT_REPORT		6	/* 11y D9.0 */
#define ACTION_MEASUREMENT_PILOT_ACTION		7	/* 11y D9.0 */
#define ACTION_DSE_POWER_CONSTRAINT			8	/* 11y D9.0 */
#define ACTION_WIFI_DIRECT					9 	/* 11y */
#define ACTION_GAS_INITIAL_REQ				10 	/* 11U */
#define ACTION_GAS_INITIAL_RSP				11 	/* 11U */
#define ACTION_GAS_COMEBACK_REQ				12 	/* 11U */
#define ACTION_GAS_COMEBACK_RSP				13 	/* 11U */
#define ACTION_TDLS_DISCOVERY_RSP			14	/* 11z D13.0 */
#define ACTION_VENDOR_USAGE					221

/*HT  Action field value */
#define NOTIFY_BW_ACTION				0
#define SMPS_ACTION						1
#define PSMP_ACTION   					2
#define SETPCO_ACTION					3
#define MIMO_CHA_MEASURE_ACTION		4
#define MIMO_N_BEACONFORM				5	/* non-compressed beamforming report */
#define MIMO_BEACONFORM				6	/* compressed beamforming report */
#define ANTENNA_SELECT					7

/* VHT Action field value */
#define ACT_VHT_COMP_BFING		0	/* VHT Compressed Beamforming */
#define ACT_VHT_GRP_ID_MGMT		1	/* Group ID Management */
#define ACT_VHT_OPMODE_NOTIFY		2	/* Operating Mode Notification */


#define ACT_FUNC_SIZE                 (MAX_ACT_STATE * MAX_ACT_MSG)
/*
	STA's AUTHENTICATION state machine: states, evvents, total function #
*/
#define AUTH_REQ_IDLE                   0
#define AUTH_WAIT_SEQ2                  1
#define AUTH_WAIT_SEQ4                  2
#define MAX_AUTH_STATE                  3

#define AUTH_MACHINE_BASE               0
#define MT2_MLME_AUTH_REQ               0
#define MT2_PEER_AUTH_EVEN              1
#define MT2_AUTH_TIMEOUT                2
#define MAX_AUTH_MSG                    3

#define AUTH_FUNC_SIZE                  (MAX_AUTH_STATE * MAX_AUTH_MSG)

/*
	STA's AUTH_RSP state machine: states, events, total function #
*/
#define AUTH_RSP_IDLE                   0
#define AUTH_RSP_WAIT_CHAL              1
#define MAX_AUTH_RSP_STATE              2

#define AUTH_RSP_MACHINE_BASE           0
#define MT2_AUTH_CHALLENGE_TIMEOUT      0
#define MT2_PEER_AUTH_ODD               1
#define MT2_PEER_DEAUTH                 2
#define MAX_AUTH_RSP_MSG                3

#define AUTH_RSP_FUNC_SIZE              (MAX_AUTH_RSP_STATE * MAX_AUTH_RSP_MSG)

/*
	STA's SYNC state machine: states, events, total function #
*/
#define SYNC_IDLE                       0	/* merge NO_BSS,IBSS_IDLE,IBSS_ACTIVE and BSS in to 1 state */
#ifdef WH_EZ_SETUP

#ifdef EZ_MOD_SUPPORT

#define EZ_IDLE                  							0
#define EZ_MAX_STATE            							1
#define EZ_MAX_MSG              							4
#define EZ_MACHINE_BASE										0

#define EZ_ROAM_REQ											0
#define EZ_TRIBAND_RESTART_NON_EZ_REQ 						1
#define EZ_PERIODIC_EXEC_REQ									2
#define EZ_UPDATE_SSID_PSK_REQ									3

#define EZ_FUNC_SIZE               							(EZ_MAX_STATE * EZ_MAX_MSG)

#else
//! Levarage from MP1.0 CL#170063
#define EZ_ROAM_IDLE                    0
#define EZ_ROAM_MAX_STATE            1
#define EZ_ROAM_MAX_MSG              1
#define EZ_ROAM_MACHINE_BASE		0

#define EZ_ROAM_REQ		0
#define EZ_ROAM_FUNC_SIZE               (EZ_ROAM_MAX_STATE * EZ_ROAM_MAX_MSG)


#define AP_TRIBAND_IDLE                    0
#define AP_MAX_TRIBAND_STATE            1
#define AP_MAX_TRIBAND_MSG              2
#define AP_TRIBAND_MACHINE_BASE		0

#define AP_TRIBAND_UPDATE_CHANNEL_REQ		0
#define AP_TRIBAND_RESTART_NON_EZ_REQ		1
#define AP_TRIBAND_FUNC_SIZE               (AP_MAX_TRIBAND_STATE * AP_MAX_TRIBAND_MSG)
#endif
#endif

#define JOIN_WAIT_BEACON                1
#define SCAN_LISTEN                     2
#define SCAN_PENDING                    3
#define MAX_SYNC_STATE                  4

#define SYNC_MACHINE_BASE               0
#define MT2_MLME_SCAN_REQ               0
#define MT2_MLME_JOIN_REQ               1
#define MT2_MLME_START_REQ              2
#define MT2_PEER_BEACON                 3
#define MT2_PEER_PROBE_RSP              4
#define MT2_PEER_ATIM                   5
#define MT2_SCAN_TIMEOUT                6
#define MT2_BEACON_TIMEOUT              7
#define MT2_ATIM_TIMEOUT                8
#define MT2_PEER_PROBE_REQ              9
#define MT2_MLME_FORCE_JOIN_REQ 	10
#define MT2_MLME_FORCE_SCAN_REQ 	11
#define MAX_SYNC_MSG                    	12

#define SYNC_FUNC_SIZE                  (MAX_SYNC_STATE * MAX_SYNC_MSG)

/*Messages for the DLS state machine */
#define DLS_IDLE						0
#define MAX_DLS_STATE					1

#define DLS_MACHINE_BASE				0
#define MT2_MLME_DLS_REQ			    0
#define MT2_PEER_DLS_REQ			    1
#define MT2_PEER_DLS_RSP			    2
#define MT2_MLME_DLS_TEAR_DOWN		    3
#define MT2_PEER_DLS_TEAR_DOWN		    4
#define MAX_DLS_MSG				        5

#define DLS_FUNC_SIZE					(MAX_DLS_STATE * MAX_DLS_MSG)


/*
	WSC State machine: states, events, total function #
*/
#ifdef WSC_INCLUDED
/*Messages for the WSC State machine */
#define	WSC_IDLE						0
#define	MAX_WSC_STATE					1
#define	WSC_FUNC_SIZE					(MAX_WSC_STATE * MAX_WSC_MSG)

#endif /* WSC_INCLUDED */

/*
	AP's CONTROL/CONNECT state machine: states, events, total function #
*/
#define AP_CNTL_FUNC_SIZE               1

/*
	AP's ASSOC state machine: states, events, total function #
*/
#define AP_ASSOC_IDLE                   0
#define AP_MAX_ASSOC_STATE              1

#define AP_ASSOC_MACHINE_BASE           0
#define APMT2_MLME_DISASSOC_REQ         0
#define APMT2_PEER_DISASSOC_REQ         1
#define APMT2_PEER_ASSOC_REQ            2
#define APMT2_PEER_REASSOC_REQ          3
#define APMT2_CLS3ERR                   4
#define AP_MAX_ASSOC_MSG                5

#define AP_ASSOC_FUNC_SIZE              (AP_MAX_ASSOC_STATE * AP_MAX_ASSOC_MSG)

/*
	AP's AUTHENTICATION state machine: states, events, total function #
*/
#define AP_AUTH_REQ_IDLE                0
#define AP_MAX_AUTH_STATE               1

#define AP_AUTH_MACHINE_BASE            0
#define APMT2_MLME_DEAUTH_REQ           0
#define APMT2_CLS2ERR                   1
#define APMT2_PEER_DEAUTH				2
#define APMT2_PEER_AUTH_REQ				3
#define APMT2_PEER_AUTH_CONFIRM			4
#define AP_MAX_AUTH_MSG                 5

#define AP_AUTH_FUNC_SIZE               (AP_MAX_AUTH_STATE * AP_MAX_AUTH_MSG)

/*
	AP's SYNC state machine: states, events, total function #
*/
#define AP_SYNC_IDLE                    0
#ifdef AP_SCAN_SUPPORT
#define AP_SCAN_LISTEN					1
#define AP_MAX_SYNC_STATE               2
#else
#define AP_MAX_SYNC_STATE               1
#endif

#define AP_SYNC_MACHINE_BASE		0
#define APMT2_PEER_PROBE_REQ		0
#define APMT2_PEER_BEACON			1
#define APMT2_PEER_PROBE_RSP		2
#ifdef AP_SCAN_SUPPORT
#define APMT2_MLME_SCAN_REQ		3
#define APMT2_SCAN_TIMEOUT		4
#define APMT2_MLME_SCAN_CNCL		5
#define AP_MAX_SYNC_MSG			6
#else
#define AP_MAX_SYNC_MSG			3
#endif

#define AP_SYNC_FUNC_SIZE               (AP_MAX_SYNC_STATE * AP_MAX_SYNC_MSG)

#ifdef APCLI_SUPPORT
/*ApCli authentication state machine */
#define APCLI_AUTH_REQ_IDLE                0
#define APCLI_AUTH_WAIT_SEQ2               1
#define APCLI_AUTH_WAIT_SEQ4               2
#ifdef APCLI_SAE_SUPPORT
#define APCLI_AUTH_WAIT_SAE					3
#define APCLI_MAX_AUTH_STATE               4
#else
#define APCLI_MAX_AUTH_STATE               3
#endif

#define APCLI_AUTH_MACHINE_BASE            0
#define APCLI_MT2_MLME_AUTH_REQ            0
#define APCLI_MT2_MLME_DEAUTH_REQ          1
#define APCLI_MT2_PEER_AUTH_EVEN           2
#define APCLI_MT2_PEER_DEAUTH              3
#define APCLI_MT2_AUTH_TIMEOUT             4
#ifdef APCLI_SAE_SUPPORT
#define APCLI_MT2_MLME_SAE_AUTH_REQ        5
#define APCLI_MT2_MLME_SAE_AUTH_COMMIT     6
#define APCLI_MT2_MLME_SAE_AUTH_CONFIRM    7
#define APCLI_MAX_AUTH_MSG                 8
#else
#define APCLI_MAX_AUTH_MSG                 5
#endif

#define APCLI_AUTH_FUNC_SIZE               (APCLI_MAX_AUTH_STATE * APCLI_MAX_AUTH_MSG)

/*ApCli association state machine */
#define APCLI_ASSOC_IDLE                   0
#define APCLI_ASSOC_WAIT_RSP               1
#define APCLI_MAX_ASSOC_STATE              2

#define APCLI_ASSOC_MACHINE_BASE           0
#define APCLI_MT2_MLME_ASSOC_REQ           0
#define APCLI_MT2_MLME_DISASSOC_REQ        1
#define APCLI_MT2_PEER_DISASSOC_REQ        2
#define APCLI_MT2_PEER_ASSOC_RSP           3
#define APCLI_MT2_ASSOC_TIMEOUT            4
#define APCLI_MAX_ASSOC_MSG                5

#define APCLI_ASSOC_FUNC_SIZE              (APCLI_MAX_ASSOC_STATE * APCLI_MAX_ASSOC_MSG)

/*ApCli sync state machine */
#define APCLI_SYNC_IDLE                   0	/* merge NO_BSS,IBSS_IDLE,IBSS_ACTIVE and BSS in to 1 state */
#define APCLI_JOIN_WAIT_PROBE_RSP         1
#define APCLI_MAX_SYNC_STATE              2

#define APCLI_SYNC_MACHINE_BASE           0
#define APCLI_MT2_MLME_PROBE_REQ          0
#define APCLI_MT2_PEER_PROBE_RSP          1
#define APCLI_MT2_PEER_BEACON			2
#define APCLI_MT2_PROBE_TIMEOUT           3
#define APCLI_MAX_SYNC_MSG                4

#define APCLI_SYNC_FUNC_SIZE              (APCLI_MAX_SYNC_STATE * APCLI_MAX_SYNC_MSG)

/*ApCli ctrl state machine */
#define APCLI_CTRL_DISCONNECTED           0	/* merge NO_BSS,IBSS_IDLE,IBSS_ACTIVE and BSS in to 1 state */
#define APCLI_CTRL_PROBE                  1
#define APCLI_CTRL_AUTH                   2
#define APCLI_CTRL_AUTH_2                 3
#define APCLI_CTRL_ASSOC                  4
#define APCLI_CTRL_DEASSOC                5
#define APCLI_CTRL_CONNECTED              6
#define APCLI_MAX_CTRL_STATE              7

#define APCLI_CTRL_MACHINE_BASE           0
#define APCLI_CTRL_JOIN_REQ               0
#define APCLI_CTRL_PROBE_RSP              1
#define APCLI_CTRL_AUTH_RSP               2
#define APCLI_CTRL_DISCONNECT_REQ         3
#define APCLI_CTRL_PEER_DISCONNECT_REQ    4
#define APCLI_CTRL_ASSOC_RSP              5
#define APCLI_CTRL_DEASSOC_RSP            6
#define APCLI_CTRL_JOIN_REQ_TIMEOUT       7
#define APCLI_CTRL_AUTH_REQ_TIMEOUT       8
#define APCLI_CTRL_ASSOC_REQ_TIMEOUT      9
#define APCLI_CTRL_MT2_AUTH_REQ			  10
#define APCLI_CTRL_MT2_ASSOC_REQ		  11
#define APCLI_CTRL_DEL_MACREPENTRY         12
//#define APCLI_MAX_CTRL_MSG                12
#ifndef APCLI_CONNECTION_TRIAL
#define APCLI_MAX_CTRL_MSG                13
#ifdef WH_EZ_SETUP
#undef APCLI_MAX_CTRL_MSG
#define APCLI_CTRL_JOIN_FAIL	          14
#define APCLI_MAX_CTRL_MSG			      15
#endif /* WH_EZ_SETUP */
#else
#undef APCLI_MAX_CTRL_MSG
#define	APCLI_CTRL_TRIAL_CONNECT		  14
#define APCLI_CTRL_TRIAL_CONNECT_TIMEOUT  15
#define APCLI_CTRL_TRIAL_PHASE2_TIMEOUT	  16
#define APCLI_CTRL_TRIAL_RETRY_TIMEOUT	  17
#define APCLI_MAX_CTRL_MSG				  18
#endif /* APCLI_CONNECTION_TRIAL */

#define APCLI_CTRL_FUNC_SIZE              (APCLI_MAX_CTRL_STATE * APCLI_MAX_CTRL_MSG)

#ifdef WH_EZ_SETUP
#ifdef NEW_CONNECTION_ALGO
#define APCLI_DISCONNECT_SUB_REASON_APCLI_EZ_CONNECTION_LOOP	11
#endif
#endif


#endif /* APCLI_SUPPORT */


/* ============================================================================= */


/* ACK policy of QOS Control field bit 6:5 */
#define NORMAL_ACK                  0x00	/* b6:5 = 00 */
#define NO_ACK                      0x20	/* b6:5 = 01 */
#define NO_EXPLICIT_ACK             0x40	/* b6:5 = 10 */
#define BLOCK_ACK                   0x60	/* b6:5 = 11 */

#ifdef USB_BULK_BUF_ALIGMENT
#ifndef BUF_ALIGMENT_RINGSIZE
#undef BUF_ALIGMENT_RINGSIZE
#define BUF_ALIGMENT_RINGSIZE         6	/*BUF_ALIGMENT_RINGSIZE must  >= 3 */
#endif
#endif /* USB_BULK_BUF_ALIGMENT */


/* STA_CSR4.field.TxResult */
#define TX_RESULT_SUCCESS           0
#define TX_RESULT_ZERO_LENGTH       1
#define TX_RESULT_UNDER_RUN         2
#define TX_RESULT_OHY_ERROR         4
#define TX_RESULT_RETRY_FAIL        6


/* MCS for CCK.  BW.SGI.STBC are reserved */
#define MCS_LONGP_RATE_1                      0	/* long preamble CCK 1Mbps */
#define MCS_LONGP_RATE_2                      1	/* long preamble CCK 1Mbps */
#define MCS_LONGP_RATE_5_5                    2
#define MCS_LONGP_RATE_11                     3
#define MCS_SHORTP_RATE_1                      4	/* long preamble CCK 1Mbps. short is forbidden in 1Mbps */
#define MCS_SHORTP_RATE_2                      5	/* short preamble CCK 2Mbps */
#define MCS_SHORTP_RATE_5_5                    6
#define MCS_SHORTP_RATE_11                     7
/* To send duplicate legacy OFDM. set BW=BW_40.  SGI.STBC are reserved */
#define MCS_RATE_6                      0	/* legacy OFDM */
#define MCS_RATE_9                      1	/* OFDM */
#define MCS_RATE_12                     2	/* OFDM */
#define MCS_RATE_18                     3	/* OFDM */
#define MCS_RATE_24                     4	/* OFDM */
#define MCS_RATE_36                     5	/* OFDM */
#define MCS_RATE_48                     6	/* OFDM */
#define MCS_RATE_54                     7	/* OFDM */
/* HT */
#define MCS_0		0	/* 1S */
#define MCS_1		1
#define MCS_2		2
#define MCS_3		3
#define MCS_4		4
#define MCS_5		5
#define MCS_6		6
#define MCS_7		7
#define MCS_8		8	/* 2S */
#define MCS_9		9
#define MCS_10		10
#define MCS_11		11
#define MCS_12		12
#define MCS_13		13
#define MCS_14		14
#define MCS_15		15
#define MCS_16		16	/* 3*3 */
#define MCS_17		17
#define MCS_18		18
#define MCS_19		19
#define MCS_20		20
#define MCS_21		21
#define MCS_22		22
#define MCS_23		23
#define MCS_32		32
#define MCS_AUTO		33

#ifdef DOT11_VHT_AC
#define MCS_VHT_2SS_MCS9	0x29
#define MCS_VHT_2SS_MCS8	0x28
#define MCS_VHT_2SS_MCS7	0x27
#define MCS_VHT_2SS_MCS6	0x26
#define MCS_VHT_2SS_MCS5	0x25
#define MCS_VHT_2SS_MCS4	0x24
#define MCS_VHT_2SS_MCS3	0x23
#define MCS_VHT_2SS_MCS2	0x22
#define MCS_VHT_2SS_MCS1	0x21
#define MCS_VHT_2SS_MCS0	0x20

#define MCS_VHT_1SS_MCS9	0x19
#define MCS_VHT_1SS_MCS8	0x18
#define MCS_VHT_1SS_MCS7	0x17
#define MCS_VHT_1SS_MCS6	0x16
#define MCS_VHT_1SS_MCS5	0x15
#define MCS_VHT_1SS_MCS4	0x14
#define MCS_VHT_1SS_MCS3	0x13
#define MCS_VHT_1SS_MCS2	0x12
#define MCS_VHT_1SS_MCS1	0x11
#define MCS_VHT_1SS_MCS0	0x10

#define VHT_RATE_IDX_1SS_MCS0	0
#define VHT_RATE_IDX_1SS_MCS1	1
#define VHT_RATE_IDX_1SS_MCS2	2
#define VHT_RATE_IDX_1SS_MCS3	3
#define VHT_RATE_IDX_1SS_MCS4	4
#define VHT_RATE_IDX_1SS_MCS5	5
#define VHT_RATE_IDX_1SS_MCS6	6
#define VHT_RATE_IDX_1SS_MCS7	7
#define VHT_RATE_IDX_1SS_MCS8	8
#define VHT_RATE_IDX_1SS_MCS9	9

#define VHT_RATE_IDX_2SS_MCS0	10
#define VHT_RATE_IDX_2SS_MCS1	11
#define VHT_RATE_IDX_2SS_MCS2	12
#define VHT_RATE_IDX_2SS_MCS3	13
#define VHT_RATE_IDX_2SS_MCS4	14
#define VHT_RATE_IDX_2SS_MCS5	15
#define VHT_RATE_IDX_2SS_MCS6	16
#define VHT_RATE_IDX_2SS_MCS7	17
#define VHT_RATE_IDX_2SS_MCS8	18
#define VHT_RATE_IDX_2SS_MCS9	19
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
/* OID_HTPHYMODE */
/* MODE */
#define HTMODE_MM	0
#define HTMODE_GF	1
#endif /* DOT11_N_SUPPORT */

/* Fixed Tx MODE - HT, CCK or OFDM */
#define FIXED_TXMODE_HT	0
#define FIXED_TXMODE_CCK	1
#define FIXED_TXMODE_OFDM 	2
#define FIXED_TXMODE_VHT	3

/* BW */
#define BW_20		BAND_WIDTH_20
#define BW_40		BAND_WIDTH_40
#define BW_80		BAND_WIDTH_80
#define BW_160		BAND_WIDTH_160
#define BW_10		BAND_WIDTH_10	/* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */
#define BW_5		BAND_WIDTH_5
#define BW_8080		BAND_WIDTH_8080
#define BW_25		BAND_WIDTH_25
#define BW_NUM		BAND_WIDTH_NUM

#define RF_BW_20	1
#define RF_BW_40	2
#define RF_BW_10	4
#define RF_BW_80	8

#define RF_MODE_CCK	1
#define RF_MODE_OFDM	2

#ifdef DOT11_N_SUPPORT
#define HT_BW_20		0
#define HT_BW_40		1
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
#define VHT_BW_2040	0
#define VHT_BW_80		1
#define VHT_BW_160		2
#define VHT_BW_8080	3
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
/* SHORTGI */
#define GI_400		GAP_INTERVAL_400	/* only support in HT mode */
#define GI_BOTH		GAP_INTERVAL_BOTH
#endif /* DOT11_N_SUPPORT */
#define GI_800		GAP_INTERVAL_800

/* STBC */
#define STBC_NONE	0
#ifdef DOT11_N_SUPPORT
#define STBC_USE	1	/* limited use in rt2860b phy */
#define RXSTBC_ONE	1	/* rx support of one spatial stream */
#define RXSTBC_TWO	2	/* rx support of 1 and 2 spatial stream */
#define RXSTBC_THR	3	/* rx support of 1~3 spatial stream */
/* MCS FEEDBACK */
#define MCSFBK_NONE	0	/* not support mcs feedback / */
#define MCSFBK_RSV	1	/* reserved */
#define MCSFBK_UNSOLICIT	2	/* only support unsolict mcs feedback */
#define MCSFBK_MRQ	3	/* response to both MRQ and unsolict mcs feedback */

/* MIMO power safe */
#define MMPS_STATIC		0
#define MMPS_DYNAMIC	1
#define MMPS_RSV		2
#define MMPS_DISABLE	3

/* A-MSDU size */
#define	AMSDU_0	0
#define	AMSDU_1	1

#endif /* DOT11_N_SUPPORT */

/* MCS use 7 bits */
#define TXRATEMIMO		0x80
#define TXRATEMCS		0x7F
#define TXRATEOFDM		0x7F
#define RATE_1                      0
#define RATE_2                      1
#define RATE_5_5                    2
#define RATE_11                     3
#define RATE_6                      4	/* OFDM */
#define RATE_9                      5	/* OFDM */
#define RATE_12                     6	/* OFDM */
#define RATE_18                     7	/* OFDM */
#define RATE_24                     8	/* OFDM */
#define RATE_36                     9	/* OFDM */
#define RATE_48                     10	/* OFDM */
#define RATE_54                     11	/* OFDM */
#define RATE_FIRST_OFDM_RATE        RATE_6
#define RATE_LAST_OFDM_RATE        	RATE_54
#define RATE_6_5                    12	/* HT mix */
#define RATE_13                     13	/* HT mix */
#define RATE_19_5                   14	/* HT mix */
#define RATE_26                     15	/* HT mix */
#define RATE_39                     16	/* HT mix */
#define RATE_52                     17	/* HT mix */
#define RATE_58_5                   18	/* HT mix */
#define RATE_65                     19	/* HT mix */
#define RATE_78                     20	/* HT mix */
#define RATE_104                    21	/* HT mix */
#define RATE_117                    22	/* HT mix */
#define RATE_130                    23	/* HT mix */
/*#define RATE_AUTO_SWITCH            255 // for StaCfg.FixedTxRate only */
#define HTRATE_0                      12
#define RATE_FIRST_MM_RATE        HTRATE_0
#define RATE_FIRST_HT_RATE        HTRATE_0
#define RATE_LAST_HT_RATE        HTRATE_0

/* pTxWI->txop */
#define IFS_HTTXOP                 0	/* The txop will be handles by ASIC. */
#define IFS_PIFS                    1
#define IFS_SIFS                    2
#define IFS_BACKOFF                 3

/* pTxD->RetryMode */
#define LONG_RETRY                  1
#define SHORT_RETRY                 0

/* Country Region definition */
#define REGION_MINIMUM_BG_BAND            0
#define REGION_0_BG_BAND                  0	/* 1-11 */
#define REGION_1_BG_BAND                  1	/* 1-13 */
#define REGION_2_BG_BAND                  2	/* 10-11 */
#define REGION_3_BG_BAND                  3	/* 10-13 */
#define REGION_4_BG_BAND                  4	/* 14 */
#define REGION_5_BG_BAND                  5	/* 1-14 */
#define REGION_6_BG_BAND                  6	/* 3-9 */
#define REGION_7_BG_BAND                  7	/* 5-13 */
#define REGION_31_BG_BAND                 31	/* 5-13 */
#define REGION_32_BG_BAND                 32	/* 1 - 13 */
#define REGION_33_BG_BAND                 33	/* 1 - 14 */
#define REGION_MAXIMUM_BG_BAND            7

#define REGION_MINIMUM_A_BAND             0
#define REGION_0_A_BAND                   0	/* 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165 */
#define REGION_1_A_BAND                   1	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 */
#define REGION_2_A_BAND                   2	/* 36, 40, 44, 48, 52, 56, 60, 64 */
#define REGION_3_A_BAND                   3	/* 52, 56, 60, 64, 149, 153, 157, 161 */
#define REGION_4_A_BAND                   4	/* 149, 153, 157, 161, 165 */
#define REGION_5_A_BAND                   5	/* 149, 153, 157, 161 */
#define REGION_6_A_BAND                   6	/* 36, 40, 44, 48 */
#define REGION_7_A_BAND                   7	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 169, 173 */
#define REGION_8_A_BAND                   8	/* 52, 56, 60, 64 */
#define REGION_9_A_BAND                   9	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165 */
#define REGION_10_A_BAND                  10	/* 36, 40, 44, 48, 149, 153, 157, 161, 165 */
#define REGION_11_A_BAND                  11	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161 */
#define REGION_12_A_BAND                  12	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 */
#define REGION_13_A_BAND                  13	/* 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161 */
#define REGION_14_A_BAND                  14	/* 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165 */
#define REGION_15_A_BAND                  15	/* 149, 153, 157, 161, 165, 169, 173 */
#define REGION_16_A_BAND                  16	/* 52, 56, 60, 64, 149, 153, 157, 161, 165 */
#define REGION_17_A_BAND                  17
#define REGION_18_A_BAND                  18
#define REGION_19_A_BAND                  19
#define REGION_20_A_BAND                  20
#define REGION_21_A_BAND                  21
#define REGION_22_A_BAND                  22
#define REGION_MAXIMUM_A_BAND             37

/* The security mode definition in MAC register */
#define CIPHER_NONE                 0
#define CIPHER_WEP64                1
#define CIPHER_WEP128               2
#define CIPHER_TKIP                 3
#define CIPHER_AES                  4
#define CIPHER_CKIP64               5
#define CIPHER_CKIP128              6
#define CIPHER_CKIP152          	7
#define CIPHER_SMS4					8
#define CIPHER_WEP152               9
#define CIPHER_BIP	               10


/* RC4 init value, used fro WEP & TKIP */
#define PPPINITFCS32                0xffffffff	/* Initial FCS value */

/* value domain of pAd->StaCfg.PortSecured. 802.1X controlled port definition */
#define WPA_802_1X_PORT_SECURED     1
#define WPA_802_1X_PORT_NOT_SECURED 2

#define PAIRWISE_KEY                1
#define GROUP_KEY                   2



/* Rate Adaptation timing */
#define RA_RATE		5					/* RA every fifth 100msec period */
#define RA_INTERVAL		(RA_RATE*100)	/* RA Interval in msec */

/* Rate Adaptation simpling interval setting */
#define DEF_QUICK_RA_TIME_INTERVAL	100

#define DEF_RA_TIME_INTRVAL			500

/*definition of DRS */
#define MAX_TX_RATE_INDEX			33		/* Maximum Tx Rate Table Index value */

#define SWIFT_TRAIN_UP_RECORD		8

/* pre-allocated free NDIS PACKET/BUFFER poll for internal usage */
#define MAX_NUM_OF_FREE_NDIS_PACKET 128

/*Block ACK */
#define MAX_TX_REORDERBUF   64
#define MAX_RX_REORDERBUF   64
#define DEFAULT_TX_TIMEOUT   30
#define DEFAULT_RX_TIMEOUT   30
#ifdef CONFIG_AP_SUPPORT
#define MAX_BARECI_SESSION   16
#endif /* CONFIG_AP_SUPPORT */

/* definition of Recipient or Originator */
#define I_RECIPIENT                  TRUE
#define I_ORIGINATOR                   FALSE

#define DEFAULT_BBP_TX_POWER        0
#define DEFAULT_RF_TX_POWER         5
#define DEFAULT_BBP_TX_FINE_POWER_CTRL 0

#define MAX_INI_BUFFER_SIZE		4096
#define MAX_PARAM_BUFFER_SIZE		(2048)	/* enough for ACL (18*64) */
											/*18 : the length of Mac address acceptable format "01:02:03:04:05:06;") */
											/*64 : MAX_NUM_OF_ACL_LIST */

#ifdef RT_BIG_ENDIAN
#define DIR_READ                    0
#define DIR_WRITE                   1
#define TYPE_TXD                    0
#define TYPE_RXD                    1
#define TYPE_TXINFO					0
#define TYPE_RXINFO					1
#define TYPE_TXWI					0
#define TYPE_RXWI					1
#define TYPE_TMACINFO				0
#define TYPE_RMACINFO				1
#endif

/* ========================= AP rtmp_def.h =========================== */
/* value domain for pAd->EventTab.Log[].Event */
#define EVENT_RESET_ACCESS_POINT    0	/* Log = "hh:mm:ss   Restart Access Point" */
#define EVENT_ASSOCIATED            1	/* Log = "hh:mm:ss   STA 00:01:02:03:04:05 associated" */
#define EVENT_DISASSOCIATED         2	/* Log = "hh:mm:ss   STA 00:01:02:03:04:05 left this BSS" */
#define EVENT_AGED_OUT              3	/* Log = "hh:mm:ss   STA 00:01:02:03:04:05 was aged-out and removed from this BSS" */
#define EVENT_COUNTER_M             4
#define EVENT_INVALID_PSK           5
#define EVENT_MAX_EVENT_TYPE        6
/* ==== end of AP rtmp_def.h ============ */

/* definition RSSI Number */
#define RSSI_IDX_0					0
#define RSSI_IDX_1					1
#define RSSI_IDX_2					2

/* definition of radar detection */
#define RD_NORMAL_MODE				0	/* Not found radar signal */
#define RD_SWITCHING_MODE			1	/* Found radar signal, and doing channel switch */
#define RD_SILENCE_MODE				2	/* After channel switch, need to be silence a while to ensure radar not found */

/*Driver defined cid for mapping status and command. */
#define  SLEEPCID	0x11
#define  WAKECID	0x22
#define  QUERYPOWERCID	0x33
#define  OWNERMCU	0x1
#define  OWNERCPU	0x0

/* MBSSID definition */
#define ENTRY_NOT_FOUND             0xFF

/* The signal threshold (RSSI) over new rate adaption */
#define SIGNAL_THRESHOLD_OVER_NEW_RATE_ADAPT    -65

/* After Linux 2.6.9, 
 * VLAN module use Private (from user) interface flags (netdevice->priv_flags). 
 * #define IFF_802_1Q_VLAN 0x1         --    802.1Q VLAN device.  in if.h     
 * ref to ip_sabotage_out() [ out->priv_flags & IFF_802_1Q_VLAN ] in br_netfilter.c
 * 
 * For this reason, we MUST use EVEN value in priv_flags
 */
#define INT_MAIN			0x0100
#define INT_MBSSID			0x0200
#define INT_WDS				0x0300
#define INT_APCLI			0x0400
#define INT_MESH			0x0500
#define INT_P2P				0x0600
#define INT_MONITOR			0x0700
#define MAX_INT_TYPES		7

// TODO: Shiang-usw, need to revise this, consider both wdev->wdev_type and pEntry->EntryType!!


/*
	Connected peers can be seperate as following two sub groups:
		Category + Type(Properties)
	1. Category:
		NONE - Invalid Category to indicate/un-used entry
		STA - entry which use 802.11 Auth/Assoc method to connect to us
		AP - entry which provide 802.11 BSS feature and work as a AP mode
		WDS - entry which are 4-address repeater mode
		MESH - entry which are 4-address and follow 802.11s

		MCAST - entry which used internally for binding MCAT/BCAST info for a
				specifi wifi_dev instance which work as a AP mode
		WDEV - entry which are used internally for binding to a specific
				wifi_dev instance
		
	2. Type
		
*/
#define ENTRY_CAT_NONE			0x000
#define ENTRY_CAT_STA			0x001
#define ENTRY_CAT_AP			0x002
#define ENTRY_CAT_WDS			0x004
#define ENTRY_CAT_MESH			0x008
#ifdef AIR_MONITOR

#define ENTRY_CAT_MONITOR		0x010
#endif
#define ENTRY_CAT_MCAST		0x400
#define ENTRY_CAT_WDEV			0x800
#define ENTRY_CAT_MASK			0xfff

typedef struct _WIFI_NODE_TYPE {
	UINT32 rsv:26;
	UINT32 type: 3;
	UINT32 cat:3;
}WIFI_NODE_TYPE;



#define ENTRY_AP			ENTRY_CAT_AP
#define ENTRY_GO			(ENTRY_CAT_AP | 0x01000)


#define ENTRY_INFRA			ENTRY_CAT_STA
#define ENTRY_GC			(ENTRY_CAT_STA | 0x01000)
#define ENTRY_ADHOC		(ENTRY_CAT_STA | 0x02000)
#define ENTRY_APCLI			(ENTRY_CAT_STA | 0x04000)
#define ENTRY_DLS			(ENTRY_CAT_STA | 0x08000)
#define ENTRY_TDLS			(ENTRY_CAT_STA | 0x10000)
#define ENTRY_CLIENT		(ENTRY_CAT_STA | 0x20000)

#define ENTRY_WDS			ENTRY_CAT_WDS
#define ENTRY_MESH			ENTRY_CAT_MESH
#ifdef AIR_MONITOR
#define ENTRY_MONITOR			ENTRY_CAT_MONITOR
#endif
#define ENTRY_NONE			ENTRY_CAT_NONE


#define IS_ENTRY_NONE(_x)		((_x)->EntryType == ENTRY_CAT_NONE)
#define IS_ENTRY_CLIENT(_x)		((_x)->EntryType == ENTRY_CLIENT)
#define IS_ENTRY_WDS(_x)		((_x)->EntryType & ENTRY_CAT_WDS)
#ifdef AIR_MONITOR
#define IS_ENTRY_MONITOR(_x)    	((_x)->EntryType == ENTRY_CAT_MONITOR)
#endif /* AIR_MONITOR */
#define IS_ENTRY_APCLI(_x)		((_x)->EntryType == ENTRY_APCLI)
#define IS_ENTRY_ADHOC(_x)		((_x)->EntryType & ENTRY_ADHOC)
#define IS_ENTRY_AP(_x)			((_x)->EntryType & ENTRY_CAT_AP)
#define IS_ENTRY_MESH(_x)		((_x)->EntryType & ENTRY_CAT_MESH)
#define IS_ENTRY_DLS(_x)		((_x)->EntryType == ENTRY_DLS)
#define IS_ENTRY_TDLS(_x)		((_x)->EntryType == ENTRY_TDLS)
#ifdef CLIENT_WDS
#define IS_ENTRY_CLIWDS(_x)		CLIENT_STATUS_TEST_FLAG((_x), fCLIENT_STATUS_CLI_WDS)
#endif /* CLIENT_WDS */

#define IS_VALID_ENTRY(_x)		(((_x) != NULL) && ((_x)->EntryType != ENTRY_NONE))

#define SET_ENTRY_NONE(_x)		((_x)->EntryType = ENTRY_NONE)
#define SET_ENTRY_CLIENT(_x)	((_x)->EntryType = ENTRY_CLIENT)
#define SET_ENTRY_WDS(_x)		((_x)->EntryType = ENTRY_WDS)
#define SET_ENTRY_APCLI(_x)		((_x)->EntryType = ENTRY_APCLI)
#define SET_ENTRY_AP(_x)		((_x)->EntryType = ENTRY_AP)
#ifdef AIR_MONITOR
#define SET_ENTRY_MONITOR(_x)   	((_x)->EntryType = ENTRY_MONITOR)
#endif /* AIR_MONITOR */
#define SET_ENTRY_ADHOC(_x)                ((_x)->EntryType = ENTRY_ADHOC)
#define SET_ENTRY_MESH(_x)		((_x)->EntryType = ENTRY_MESH)
#define SET_ENTRY_DLS(_x)		((_x)->EntryType = ENTRY_DLS)
#define SET_ENTRY_TDLS(_x)		((_x)->EntryType = ENTRY_TDLS)
#ifdef CLIENT_WDS
#define SET_ENTRY_CLIWDS(_x)	CLIENT_STATUS_SET_FLAG((_x), fCLIENT_STATUS_CLI_WDS)
#endif /* CLIENT_WDS */
//CFG_TODO
#define SET_PKT_OPMODE_AP(_x)		((_x)->OpMode = OPMODE_AP)
#define SET_PKT_OPMODE_STA(_x)		((_x)->OpMode = OPMODE_STA)
#define IS_PKT_OPMODE_AP(_x)		((_x)->OpMode == OPMODE_AP)
#define IS_PKT_OPMODE_STA(_x)		((_x)->OpMode == OPMODE_STA)


#define IS_OPMODE_AP(_x)		((_x)->OpMode == OPMODE_AP)
#define IS_OPMODE_STA(_x)		((_x)->OpMode == OPMODE_STA)

#if defined(ANDROID_SUPPORT) || defined(RT_CFG80211_SUPPORT)
#if CONFIG_RTPCI_AP_RF_OFFSET == 0x48000
#define INF_MAIN_DEV_NAME		"wlani"
#define INF_MBSSID_DEV_NAME		"wlani"
#else
#ifdef HE_BD_CFG80211_SUPPORT 
#define INF_MAIN_DEV_NAME		"ra"
#define INF_MBSSID_DEV_NAME		"ra"
#else
#define INF_MAIN_DEV_NAME		"wlan"
#define INF_MBSSID_DEV_NAME		"wlan"
#endif /* HE_BD_CFG80211_SUPPORT */

#endif

#else // !ANDROID_SUPPORT || RT_CFG80211_SUPPORT
#if CONFIG_RTPCI_AP_RF_OFFSET == 0x48000
#define INF_MAIN_DEV_NAME		"rai"
#define INF_MBSSID_DEV_NAME		"rai"
#else
#define INF_MAIN_DEV_NAME		"ra"
#define INF_MBSSID_DEV_NAME		"ra"
#endif
#endif /* ANDROID_SUPPORT */


#if CONFIG_RTPCI_AP_RF_OFFSET == 0x48000
#define INF_WDS_DEV_NAME		"wdsi"
#define INF_APCLI_DEV_NAME		"apclii"
#define INF_MESH_DEV_NAME		"meshi"
#define INF_P2P_DEV_NAME		"p2pi"
#define INF_MONITOR_DEV_NAME	"moni"
#else
#define INF_WDS_DEV_NAME		"wds"
#define INF_APCLI_DEV_NAME		"apcli"
#define INF_MESH_DEV_NAME		"mesh"
#define INF_P2P_DEV_NAME		"p2p"
#define INF_MONITOR_DEV_NAME	"mon"
#endif

/* WEP Key TYPE */
#define WEP_HEXADECIMAL_TYPE    0
#define WEP_ASCII_TYPE          1

/* WIRELESS EVENTS definition */
/* Max number of char in custom event, refer to wireless_tools.28/wireless.20.h */
#define IW_CUSTOM_MAX_LEN				  			255	/* In bytes */

/* For system event - start */
#define	IW_SYS_EVENT_FLAG_START                     0x0200
#define	IW_ASSOC_EVENT_FLAG                         0x0200
#define	IW_DISASSOC_EVENT_FLAG                      0x0201
#define	IW_DEAUTH_EVENT_FLAG                      	0x0202
#define	IW_AGEOUT_EVENT_FLAG                      	0x0203
#define	IW_COUNTER_MEASURES_EVENT_FLAG              0x0204
#define	IW_REPLAY_COUNTER_DIFF_EVENT_FLAG           0x0205
#define	IW_RSNIE_DIFF_EVENT_FLAG           			0x0206
#define	IW_MIC_DIFF_EVENT_FLAG           			0x0207
#define IW_ICV_ERROR_EVENT_FLAG						0x0208
#define IW_MIC_ERROR_EVENT_FLAG						0x0209
#define IW_GROUP_HS_TIMEOUT_EVENT_FLAG				0x020A
#define	IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG			0x020B
#define IW_RSNIE_SANITY_FAIL_EVENT_FLAG				0x020C
#define IW_SET_KEY_DONE_WPA1_EVENT_FLAG				0x020D
#define IW_SET_KEY_DONE_WPA2_EVENT_FLAG				0x020E
#define IW_STA_LINKUP_EVENT_FLAG					0x020F
#define IW_STA_LINKDOWN_EVENT_FLAG					0x0210
#define IW_SCAN_COMPLETED_EVENT_FLAG				0x0211
#define IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG				0x0212
#define IW_CHANNEL_CHANGE_EVENT_FLAG				0x0213
#define IW_STA_MODE_EVENT_FLAG						0x0214
#define IW_MAC_FILTER_LIST_EVENT_FLAG				0x0215
#define IW_AUTH_REJECT_CHALLENGE_FAILURE			0x0216
#define IW_SCANNING_EVENT_FLAG						0x0217
#define IW_START_IBSS_FLAG							0x0218
#define IW_JOIN_IBSS_FLAG							0x0219
#define IW_SHARED_WEP_FAIL							0x021A
#define IW_WPS_END_EVENT_FLAG						0x021B
/* if add new system event flag, please upadte the IW_SYS_EVENT_FLAG_END */
#define	IW_SYS_EVENT_FLAG_END                       0x021B
#define	IW_SYS_EVENT_TYPE_NUM						(IW_SYS_EVENT_FLAG_END - IW_SYS_EVENT_FLAG_START + 1)
/* For system event - end */

#ifdef IDS_SUPPORT
/* For spoof attack event - start */
#define	IW_SPOOF_EVENT_FLAG_START                   0x0300
#define IW_CONFLICT_SSID_EVENT_FLAG					0x0300
#define IW_SPOOF_ASSOC_RESP_EVENT_FLAG				0x0301
#define IW_SPOOF_REASSOC_RESP_EVENT_FLAG			0x0302
#define IW_SPOOF_PROBE_RESP_EVENT_FLAG				0x0303
#define IW_SPOOF_BEACON_EVENT_FLAG					0x0304
#define IW_SPOOF_DISASSOC_EVENT_FLAG				0x0305
#define IW_SPOOF_AUTH_EVENT_FLAG					0x0306
#define IW_SPOOF_DEAUTH_EVENT_FLAG					0x0307
#define IW_SPOOF_UNKNOWN_MGMT_EVENT_FLAG			0x0308
#define IW_REPLAY_ATTACK_EVENT_FLAG					0x0309
/* if add new spoof attack event flag, please upadte the IW_SPOOF_EVENT_FLAG_END */
#define	IW_SPOOF_EVENT_FLAG_END                     0x0309
#define	IW_SPOOF_EVENT_TYPE_NUM						(IW_SPOOF_EVENT_FLAG_END - IW_SPOOF_EVENT_FLAG_START + 1)
/* For spoof attack event - end */

/* For flooding attack event - start */
#define	IW_FLOOD_EVENT_FLAG_START                   0x0400
#define IW_FLOOD_AUTH_EVENT_FLAG					0x0400
#define IW_FLOOD_ASSOC_REQ_EVENT_FLAG				0x0401
#define IW_FLOOD_REASSOC_REQ_EVENT_FLAG				0x0402
#define IW_FLOOD_PROBE_REQ_EVENT_FLAG				0x0403
#define IW_FLOOD_DISASSOC_EVENT_FLAG				0x0404
#define IW_FLOOD_DEAUTH_EVENT_FLAG					0x0405
#define IW_FLOOD_EAP_REQ_EVENT_FLAG					0x0406
/* if add new flooding attack event flag, please upadte the IW_FLOOD_EVENT_FLAG_END */
#define	IW_FLOOD_EVENT_FLAG_END                   	0x0406
#define	IW_FLOOD_EVENT_TYPE_NUM						(IW_FLOOD_EVENT_FLAG_END - IW_FLOOD_EVENT_FLAG_START + 1)
/* For flooding attack - end */
#endif /* IDS_SUPPORT */

#ifdef WSC_INCLUDED
/* For WSC wireless event - start */
#define	IW_WSC_EVENT_FLAG_START                   	0x0500
#define IW_WSC_PBC_SESSION_OVERLAP					0x0500
#define IW_WSC_REGISTRAR_SUPPORT_PBC				0x0501
#define IW_WSC_REGISTRAR_SUPPORT_PIN				0x0502
#define	IW_WSC_STATUS_SUCCESS              			0x0503
#define	IW_WSC_STATUS_FAIL             				0x0504
#define	IW_WSC_2MINS_TIMEOUT           				0x0505
#define	IW_WSC_SEND_EAPOL_START    					0x0506
#define	IW_WSC_SEND_WSC_START      					0x0507
#define	IW_WSC_SEND_M1           					0x0508
#define	IW_WSC_SEND_M2           					0x0509
#define	IW_WSC_SEND_M3           					0x050a
#define	IW_WSC_SEND_M4           					0x050b
#define	IW_WSC_SEND_M5           					0x050c
#define	IW_WSC_SEND_M6           					0x050d
#define	IW_WSC_SEND_M7           					0x050e
#define	IW_WSC_SEND_M8           					0x050f
#define	IW_WSC_SEND_DONE           					0x0510
#define	IW_WSC_SEND_ACK           					0x0511
#define	IW_WSC_SEND_NACK           					0x0512
#define	IW_WSC_RECEIVE_WSC_START   					0x0513
#define	IW_WSC_RECEIVE_M1          					0x0514
#define	IW_WSC_RECEIVE_M2          					0x0515
#define	IW_WSC_RECEIVE_M3          					0x0516
#define	IW_WSC_RECEIVE_M4          					0x0517
#define	IW_WSC_RECEIVE_M5          					0x0518
#define	IW_WSC_RECEIVE_M6          					0x0519
#define	IW_WSC_RECEIVE_M7          					0x051a
#define	IW_WSC_RECEIVE_M8          					0x051b
#define	IW_WSC_RECEIVE_DONE        					0x051c
#define	IW_WSC_RECEIVE_ACK         					0x051d
#define	IW_WSC_RECEIVE_NACK        					0x051e
#define	IW_WSC_MANY_CANDIDATE 						0x051f
#define IW_WSC_NEXT_CANDIDATE						0x0520
#define	IW_WSC_T1_TIMER_TIMEOUT        				0x0521
#define	IW_WSC_T2_TIMER_TIMEOUT        				0x0522
#define	IW_WSC_EVENT_FLAG_END                   	0x0522
#define	IW_WSC_EVENT_TYPE_NUM						(IW_WSC_EVENT_FLAG_END - IW_WSC_EVENT_FLAG_START + 1)
/* For WSC wireless event - end */
#endif /* WSC_INCLUDED */
/* End - WIRELESS EVENTS definition */


#ifdef WH_EZ_SETUP
// For whole home coverage - easy setup wireless event - start
#define	IW_WH_EZ_EVENT_FLAG_START                  	0x0700
#define	IW_WH_EZ_PROVIDER_SEARCHING                 0x0700
#define	IW_WH_EZ_PROVIDER_FOUND       				0x0701
#define	IW_WH_EZ_PROVIDER_STOP_SEARCHING            0x0702
#define	IW_WH_EZ_CONFIGURED_AP_SEARCHING            0x0703
#define	IW_WH_EZ_CONFIGURED_AP_FOUND                0x0704
#define	IW_WH_EZ_MY_APCLI_CONNECTED                 0x0705
#define	IW_WH_EZ_MY_APCLI_DISCONNECTED              0x0706
#define	IW_WH_EZ_MY_AP_HAS_APCLI                    0x0707
#define	IW_WH_EZ_MY_AP_DOES_NOT_HAS_APCLI           0x0708
#define	IW_WH_EZ_BECOME_CONFIGURED                  0x0709
#define	IW_WH_EZ_EVENT_FLAG_END                     0x0709
#define	IW_WH_EZ_EVENT_TYPE_NUM						(IW_WH_EZ_EVENT_FLAG_END - IW_WH_EZ_EVENT_FLAG_START + 1)
/* For whole home coverage - easy setup wireless event - end */
#endif /* WH_EZ_SETUP */
#ifdef ALL_NET_EVENT
#define	IW_ALL_NET_EVENT        				0x0800
#endif /* ALL_NET_EVENT */

#ifdef MCAST_RATE_SPECIFIC
#define MCAST_DISABLE	0
#define MCAST_CCK		1
#define MCAST_OFDM		2
#define MCAST_HTMIX		3
#endif /* MCAST_RATE_SPECIFIC */

/* For AsicRadioOff/AsicRadioOn function */
// TODO: shiang-usw, check those RADIO ON/OFF values here!!!
#define DOT11POWERSAVE		0
#define GUIRADIO_OFF		1
#define RTMP_HALT		    2
#define GUI_IDLE_POWER_SAVE		3

enum {
	RESUME_RADIO_ON,
	SUSPEND_RADIO_OFF,
	MLME_RADIO_ON,
	MLME_RADIO_OFF,
	DOT11_RADIO_ON,
	DOT11_RADIO_OFF,
};

/* -- */

/* definition for WpaSupport flag */
#define WPA_SUPPLICANT_DISABLE				0x00
#define WPA_SUPPLICANT_ENABLE				0x01
#define	WPA_SUPPLICANT_ENABLE_WITH_WEB_UI	0x02
#define	WPA_SUPPLICANT_ENABLE_WPS			0x80

#ifdef MICROWAVE_OVEN_SUPPORT
/* definition for mitigating microwave interference */
#define MO_FALSE_CCA_TH	25
#define MO_MEAS_PERIOD	0	/* 0 ~ 100 ms */
#define MO_IDLE_PERIOD	1	/* 100 ~ 1000 ms */
#endif /* MICROWAVE_OVEN_SUPPORT */

typedef enum _ETHER_BAND_BINDDING {
	EtherTrafficBand2G,
 	EtherTrafficBand5G,
} ETHER_BAND_BINDDING, *PETHER_BAND_BINDDING;

/* definition for Antenna Diversity flag */
typedef enum {
	ANT_DIVERSITY_DISABLE,
	ANT_DIVERSITY_ENABLE ,
	ANT_FIX_ANT0,
	ANT_FIX_ANT1,
	ANT_SW_DIVERSITY_ENABLE,
	ANT_HW_DIVERSITY_ENABLE,
	ANT_DIVERSITY_DEFAULT
}ANT_DIVERSITY_TYPE;

enum IEEE80211_BAND {
  IEEE80211_BAND_2G,
  IEEE80211_BAND_5G,
  IEEE80211_BAND_NUMS
};

#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
#define CHANNEL_SWITCH_OFFLOAD 0x68 
#define CHANNEL_MCU_READY 0x7064
#endif /* CONFIG_SWITCH_CHANNEL_OFFLOAD */

#if defined(CONFIG_MULTI_CHANNEL) || defined(DOT11Z_TDLS_SUPPORT)
#define HW_NULL_FRAME_1_OFFSET 0x7700
#define HW_NULL_FRAME_2_OFFSET 0x7780

enum {
	EDCA_AC0_DEQUEUE_DISABLE = (1 << 0),
	EDCA_AC1_DEQUEUE_DISABLE = (1 << 1),
	EDCA_AC2_DEQUEUE_DISABLE = (1 << 2),
	EDCA_AC3_DEQUEUE_DISBALE = (1 << 3),
	HCCA_DEQUEUE_DISABLE = (1 << 4)
};

enum {
	HCCA_TO_EDCA,
	EDCA_TO_HCCA = 0x55
};

#define MUL_CHANNEL_ENABLE 0x77
#define HCCA_TIMEOUT	400
#define EDCA_TIMEOUT	400
#endif /* defined(CONFIG_MULTI_CHANNEL) || defined(DOT11Z_TDLS_SUPPORT) */

#ifdef MT_PS
#define I_PSM_ENABLE	1
#define I_PSM_DISABLE	0
#endif /* MT_PS */

#ifdef SMART_CARRIER_SENSE_SUPPORT
enum {
	SCS_DISABLE,
	SCS_ENABLE,
};

enum {
	SCS_STATUS_DEFAULT,
	SCS_STATUS_MIDDLE,
	SCS_STATUS_LOW,
	SCS_STATUS_ULTRA_LOW,
};

enum {
	Keep_Range,
	Decrease_Range,
	Increase_Range
};
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#define ABS(_x, _y) ((_x) > (_y)) ? ((_x) -(_y)) : ((_y) -(_x))

#define A2Dec(_X, _p) 				\
{									\
	UCHAR *p;						\
	_X = 0;							\
	p = _p;							\
	while (((*p >= '0') && (*p <= '9')))		\
	{												\
		if ((*p >= '0') && (*p <= '9'))		\
			_X = _X * 10 + *p - 48;					\
		p++;										\
	}												\
}

#define A2Hex(_X, _p) 				\
do{									\
	char *__p;						\
	(_X) = 0;							\
	__p = (char *)(_p);							\
	while (((*__p >= 'a') && (*__p <= 'f')) || ((*__p >= 'A') && (*__p <= 'F')) || ((*__p >= '0') && (*__p <= '9')))		\
	{												\
		if ((*__p >= 'a') && (*__p <= 'f'))				\
			(_X) = (_X) * 16 + *__p - 87;					\
		else if ((*__p >= 'A') && (*__p <= 'F'))		\
			(_X) = (_X) * 16 + *__p - 55;					\
		else if ((*__p >= '0') && (*__p <= '9'))		\
			(_X) = (_X) * 16 + *__p - 48;					\
		__p++;										\
	}												\
}while(0)


#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif


/* ========================================================================== */
/*
	The full range (1-4,095) of VLAN IDs must be supported by the 802.1Q
	implementation.
	VLAN ID 0 is reserved.
*/

#define RT_VLAN_8023_HEADER_COPY(__pAd, __VLAN_VID, __VLAN_Priority,		\
								__pHeader8023, __HdrLen, __pData, __TPID)	\
{																			\
	VLAN_8023_Header_Copy(__VLAN_VID, __VLAN_Priority,					\
								__pHeader8023, __HdrLen, __pData,			\
								__TPID);					\
}


#define RT_80211_TO_8023_PACKET(__pAd, __VLAN_VID, __VLAN_Priority,			\
								__pRxBlk, __pHeader802_3,					\
								_wdev_idx, __TPID)					\
{																			\
	wlan_802_11_to_802_3_packet(											\
				get_netdev_from_bssid(__pAd, _wdev_idx),				\
				__pRxBlk->OpMode,												\
				__VLAN_VID, __VLAN_Priority,								\
				__pRxBlk->pRxPacket, __pRxBlk->pData, __pRxBlk->DataSize,	\
				__pHeader802_3, __TPID);					\
}

// TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx
#define RTMP_L2_FRAME_TX_ACTION(__pAd, __ApIdx, __FrameBuf, __FrameLen)		\
	RTMPL2FrameTxAction(__pAd, get_netdev_from_bssid(__pAd, __ApIdx),		\
						announce_802_3_packet, __ApIdx, __FrameBuf, __FrameLen, __pAd->OpMode)

#define RTMP_UPDATE_OS_PACKET_INFO(__pAd, __pRxBlk, _wdev_idx)		\
	RtmpOsPktInit(__pRxBlk->pRxPacket,										\
					get_netdev_from_bssid(__pAd, _wdev_idx),			\
					__pRxBlk->pData, __pRxBlk->DataSize);

#ifdef SYSTEM_LOG_SUPPORT
/*
	RTMPSendWirelessEvent --> RtmpOsSendWirelessEvent --> RtmpDrvSendWirelessEvent
*/
#define RTMPSendWirelessEvent(__pAd, __Event_flag, __pAddr, _wdev_idx, __Rssi)	\
	RtmpOsSendWirelessEvent(__pAd, __Event_flag, __pAddr, _wdev_idx, __Rssi,		\
							RtmpDrvSendWirelessEvent);
#else
#define RTMPSendWirelessEvent(__pAd, __Event_flag, __pAddr, __BssIdx, __Rssi)
#endif /* SYSTEM_LOG_SUPPORT */

#define RTMP_OS_TASK_INIT(__pTask, __pTaskName, __pAd)		\
	RtmpOSTaskInit(__pTask, __pTaskName, __pAd, &(__pAd)->RscTaskMemList, &(__pAd)->RscSemMemList);
#define VALID_UCAST_ENTRY_WCID(_wcid)  (_wcid < MAX_LEN_OF_MAC_TABLE)
#endif /* __RTMP_DEF_H__ */
