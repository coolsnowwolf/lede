/****************************************************************************

    Module Name:
	rtmp_comm.h

	Abstract:
	All common definitions and macros for UTIL/DRIVER/NETIF.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_COMM_H__
#define __RT_COMM_H__

#define VENDOR_FEATURE1_SUPPORT
#ifdef BB_SOC
#define VENDOR_FEATURE3_SUPPORT
#endif


/*#define MONITOR_FLAG_11N_SNIFFER_SUPPORT */


#ifdef VENDOR_FEATURE3_SUPPORT
#ifndef BB_SOC
#ifdef DOT1X_SUPPORT
#undef DOT1X_SUPPORT
#endif	/* DOT1X_SUPPORT */
#ifdef SYSTEM_LOG_SUPPORT
#undef SYSTEM_LOG_SUPPORT
#endif	/* SYSTEM_LOG_SUPPORT */
#endif
#ifdef LED_CONTROL_SUPPORT
#undef LED_CONTROL_SUPPORT
#endif	/* LED_CONTROL_SUPPORT */
#ifdef WSC_LED_SUPPORT
#undef WSC_LED_SUPPORT
#endif	/* WSC_LED_SUPPORT */
#endif /* VENDOR_FEATURE3_SUPPORT */

/* following macros are new features, enable it to reduce CPU loading */
/* Group 1 */
#define CFG_CPU_LOADING_DISABLE_IOREAD          1
#define CFG_CPU_LOADING_RXRING_DLY              0

/* Group 2 */
#define CFG_CPU_LOADING_DISABLE_TXDONE0         1
#define CFG_CPU_LOADING_DMADONE                 0


#ifdef VENDOR_FEATURE1_SUPPORT
#define FIFO_STAT_READ_PERIOD		4
#else
#define FIFO_STAT_READ_PERIOD		0
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

#ifndef VENDOR_FEATURE3_SUPPORT
#define AP_QLOAD_SUPPORT
#endif /* VENDOR_FEATURE3_SUPPORT */

#endif	/* CONFIG_AP_SUPPORT */


/* ======================== Before include files ============================ */
/*
	14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0 @Japan + 1 as NULL termination
	Refer to CH_HZ_ID_MAP[] in rt_channel.c
*/
#ifdef DOT11_VHT_AC
#define MAX_NUM_OF_CHS			(54 + 5)	/* 5 channels for central channel of VHT 80MHz */
#else
#define MAX_NUM_OF_CHS			54
#endif /* DOT11_VHT_AC*/
/* 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL termination */
#define MAX_NUM_OF_CHANNELS             MAX_NUM_OF_CHS
#define MAX_NUM_OF_SUB_CHANNELS		(MAX_NUM_OF_CHANNELS/2)  /*Assume half size for sub channels*/


#include "rtmp_type.h"
#include "rtmp_os.h"
#include "common/debug.h"
#include "common/link_list.h"
#include "rtmp_cmd.h"
#include "cmm_cmd.h"
#include "hw_ctrl.h"
#include "iface/iface_util.h"
#ifdef SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif /* SNIFFER_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
#include "meminfo_list.h"
#endif /* MEM_ALLOC_INFO_SUPPORT */


/* ======================== Definition ====================================== */
#ifndef TRUE
#define TRUE						1
#endif
#ifndef FALSE
#define FALSE						0
#endif

#define BIT0		(1 << 0)
#define BIT1		(1 << 1)
#define BIT2		(1 << 2)
#define BIT3		(1 << 3)
#define BIT4		(1 << 4)
#define BIT5		(1 << 5)
#define BIT6		(1 << 6)
#define BIT7		(1 << 7)
#define BIT8		(1 << 8)
#define BIT9		(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)


/* definition of pAd->OpMode */
#define OPMODE_STA                  0
#define OPMODE_AP                   1
#define OPMODE_ADHOC                2   /*Carter add, align the number definition as MT hw arb module. */
#define OPMODE_APSTA                3       /* as AP and STA at the same time */

#define MAIN_MBSSID                 0
#define FIRST_MBSSID                1

/* Endian byte swapping codes */
#define SWAP16(x) \
	((UINT16) (\
			   (((UINT16) (x) & (UINT16) 0x00ffU) << 8) | \
			   (((UINT16) (x) & (UINT16) 0xff00U) >> 8)))

#define SWAP32(x) \
	((UINT32) (\
			   (((UINT32) (x) & (UINT32) 0x000000ffUL) << 24) | \
			   (((UINT32) (x) & (UINT32) 0x0000ff00UL) << 8) | \
			   (((UINT32) (x) & (UINT32) 0x00ff0000UL) >> 8) | \
			   (((UINT32) (x) & (UINT32) 0xff000000UL) >> 24)))

#define SWAP64(x) \
	((UINT64)( \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
			   (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56)))

#ifdef RT_BIG_ENDIAN

#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))

#else /* Little_Endian */

#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))

#endif /* RT_BIG_ENDIAN */


#define MAX_CUSTOM_LEN 128

#ifdef DBDC_MODE
#define MONITOR_MAX_DEV_NUM	2
#else
#define MONITOR_MAX_DEV_NUM	1
#endif
#define RX_BUFFER_SIZE_MIN	14
/* */
/* IEEE 802.11 Structures and definitions */
/* */
#define MAX_TX_POWER_LEVEL              100   /* mW */
#define MAX_RSSI_TRIGGER                -10    /* dBm */
#define MIN_RSSI_TRIGGER                -200   /* dBm */
#define MAX_FRAG_THRESHOLD              2346  /* byte count */
#define MIN_FRAG_THRESHOLD              256   /* byte count */
#define DEFAULT_FRAG_THLD	(MAX_FRAG_THRESHOLD)
#define MAX_RTS_PKT_THRESHOLD   0xFF   /* max. pkt count */
#define MAX_RTS_THRESHOLD               0xfffff  /* max. byte count */
#define DEFAULT_RTS_PKT_THLD	0x2 /*2*/
#define DEFAULT_RTS_LEN_THLD	0x92B /*2347*/

typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE {
	Ndis802_11IBSS,
	Ndis802_11Infrastructure,
	Ndis802_11AutoUnknown,
	Ndis802_11Monitor,
	Ndis802_11InfrastructureMax	/* Not a real value, defined as upper bound */
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;




/* ======================== Memory ========================================== */
#ifdef VENDOR_FEATURE2_SUPPORT

extern ULONG OS_NumOfPktAlloc, OS_NumOfPktFree;

#define NUMOfPKTALLOC() (OS_NumOfPktAlloc++);
#define NUMOfPKTFREE() (OS_NumOfPktFree++);
#else
#define NUMOfPKTALLOC()
#define NUMOfPKTFREE()
#endif /* VENDOR_FEATURE2_SUPPORT */

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST PktInfoList;

#define MEM_DBG_PKT_ALLOC_INC(__pPacket) \
	{\
		MIListAddHead(&PktInfoList, 0, __pPacket, __builtin_return_address(0));\
		NUMOfPKTALLOC();\
	}
#define MEM_DBG_PKT_FREE_INC(__pPacket)	\
	{\
		if (MIListRemove(&PktInfoList, __pPacket) == NULL) {\
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,\
					 ("the packet has not been allocated\n"));\
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,\
					 ("mem addr = %p, caller is %pS\n", __pPacket, __builtin_return_address(0)));\
			dump_stack();\
		} \
		NUMOfPKTFREE();\
	}
#else
#define MEM_DBG_PKT_ALLOC_INC(__pPacket)
#define MEM_DBG_PKT_FREE_INC(__pPacket)
#endif /* MEM_ALLOC_INFO_SUPPORT */

/* All PHY rate summary in TXD */
/* Preamble MODE in TxD */
#define MODE_CCK	0
#define MODE_OFDM   1
#ifdef DOT11_N_SUPPORT
#define MODE_HTMIX	2
#define MODE_HTGREENFIELD	3
#endif /* DOT11_N_SUPPORT */
#define MODE_VHT	4

#ifdef NO_CONSISTENT_MEM_SUPPORT
/* current support RXD_SIZE = 16B and cache line = 16 or 32B */
#define RTMP_DCACHE_FLUSH(__AddrStart, __Size)							\
	RtmpOsDCacheFlush((ULONG)(__AddrStart), (ULONG)(__Size))
#else
#define RTMP_DCACHE_FLUSH(__AddrStart, __Size)
#endif /* NO_CONSISTENT_MEM_SUPPORT */


/* ======================== Interface ======================================= */


#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)	if (_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)	if (_pAd->OpMode == OPMODE_STA)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif



/***********************************************************************************
 * IOCTL related definitions and data structures.
 **********************************************************************************/
typedef struct __RTMP_IOCTL_INPUT_STRUCT {
	union{
		char ifrn_name[IFNAMSIZ]; /* if name, e.g. "eth0" */
	} ifr_ifrn;
	union {
		CHAR *name;
		struct {
			CHAR *pointer;
			UINT16 length;
			UINT16 flags;
		} data;
	} u;
} RTMP_IOCTL_INPUT_STRUCT;


#define RT_CMD_STATUS_TRANSLATE(__Status)				\
	{													\
		if (__Status == RTMP_IO_EINVAL)					\
			__Status = -EINVAL;							\
		else if (__Status == RTMP_IO_EOPNOTSUPP)		\
			__Status = -EOPNOTSUPP;						\
		else if (__Status == RTMP_IO_EFAULT)			\
			__Status = -EFAULT;							\
		else if (__Status == RTMP_IO_E2BIG)				\
			__Status = -E2BIG;							\
		else if (__Status == RTMP_IO_ENOMEM)			\
			__Status = -ENOMEM;							\
		else if (__Status == RTMP_IO_EAGAIN)			\
			__Status = -EAGAIN;							\
		else if (__Status == RTMP_IO_ENOTCONN)			\
			__Status = -ENOTCONN;						\
	}




/* ======================== Timer =========================================== */
typedef struct _LIST_RESOURCE_OBJ_ENTRY {
	struct _LIST_RESOURCE_OBJ_ENTRY *pNext;
	VOID *pRscObj;
} LIST_RESOURCE_OBJ_ENTRY, *PLIST_RESOURCE_OBJ_ENTRY;




/* ======================== IC =========================================== */
#define RFIC_24GHZ		0x01
#define RFIC_5GHZ		0x02
#define RFIC_DUAL_BAND		(RFIC_24GHZ | RFIC_5GHZ)




/* ======================== CFG80211 ======================================== */
#define RT_CFG80211_DEBUG /* debug use */

#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, __Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif /* RT_CFG80211_DEBUG */

/* 1 ~ 14 */
#define CFG80211_NUM_OF_CHAN_2GHZ			14

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#define CFG80211_NUM_OF_CHAN_5GHZ			\
	(sizeof(Cfg80211_Chan)-CFG80211_NUM_OF_CHAN_2GHZ)




/* ======================== Packet ========================================== */
#define LENGTH_802_11               24
#define LENGTH_802_11_AND_H         30
#define LENGTH_802_11_CRC_H         34
#define LENGTH_802_11_CRC           28
#define LENGTH_802_11_WITH_ADDR4    30
#define LENGTH_802_3                14
#define LENGTH_802_3_TYPE           2
#define LENGTH_802_1_H              8
#define LENGTH_EAPOL_H              4
#define LENGTH_WMMQOS_H				2
#define LENGTH_CRC                  4
#define MAX_SEQ_NUMBER              0x0fff
#define LENGTH_802_3_NO_TYPE		12
#define LENGTH_802_1Q				4 /* VLAN related */


/*
	Packet information for NdisQueryPacket
*/
typedef struct  _PACKET_INFO    {
	UINT PhysicalBufferCount;    /* Physical breaks of buffer descripor chained */
	UINT BufferCount;           /* Number of Buffer descriptor chained */
	UINT TotalPacketLength;     /* Self explained */
	PNDIS_BUFFER pFirstBuffer;   /* Pointer to first buffer descriptor */
} PACKET_INFO, *PPACKET_INFO;


#define MAC_ADDR_LEN                    6

#define IS_BM_MAC_ADDR(Addr)				(((Addr[0]) & 0x01) == 0x01)
#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))



#define RLT_MAC_BASE 0x01
#define MT_MAC_BASE 0x2

#ifdef USB_IOT_WORKAROUND2
#define W2_IOT_MAX_PADDING_SIZE 1028 /* 512+512+4 */
#define W2_MAX_TX_SIZE 3000
#define W2_MAX_HTTX_SIZE 32768
#endif

#define MAX_NUM_OF_INF		3

#ifdef MULTI_INF_SUPPORT
/* Index 0 for 2.4G, 1 for 5Ghz Card */
extern VOID *adapt_list[MAX_NUM_OF_INF];
#endif /* MULTI_INF_SUPPORT */
#ifdef CREATE_ALL_INTERFACE_AT_INIT
#define MAX_MBSS_NUM 4
#endif
#endif /* __RT_COMM_H__ */
