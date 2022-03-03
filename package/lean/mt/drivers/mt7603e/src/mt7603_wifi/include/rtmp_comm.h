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
    
#ifdef WIFI_FWD_UPDATED
#define MT7615_MT7603_COMBO_FORWARDING		1
#else
#define MT7615_MT7603_COMBO_FORWARDING		0
#endif
#define MT7615_MT7603_COMBO_OID_FOR_WEBUI		0
    
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
//#undef LED_CONTROL_SUPPORT
#endif	/* LED_CONTROL_SUPPORT */
#ifdef WSC_LED_SUPPORT
#undef WSC_LED_SUPPORT
#endif	/* WSC_LED_SUPPORT */
#endif /* VENDOR_FEATURE3_SUPPORT */
    

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
#define MAX_NUM_OF_CHS             		(54 + 5)	/* 5 channels for central channel of VHT 80MHz */
#else
#define MAX_NUM_OF_CHS             		54
#endif /* DOT11_VHT_AC*/
/* 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL termination */
#define MAX_NUM_OF_CHANNELS             MAX_NUM_OF_CHS
#define MAX_NUM_OF_SUB_CHANNELS			MAX_NUM_OF_CHANNELS/2  /*Assume half size for sub channels*/


#include "rtmp_type.h"
#include "rtmp_os.h"
#include "link_list.h"
#include "rtmp_cmd.h"
#include "iface/iface_util.h"
#ifdef CONFIG_SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif /* CONFIG_SNIFFER_SUPPORT */



/* ======================== Debug =========================================== */ 
/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */
#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define RT_DEBUG_INFO       4
#define RT_DEBUG_LOUD       5
#define RT_DEBUG_FPGA		6
#define RT_DEBUG_MAX		RT_DEBUG_FPGA
    
typedef enum{
	DBG_FUNC_RA = 0x100 << 8,	/* debug flag for rate adaptation */
	DBG_FUNC_SA = 0x200 << 8,	/* debug flag for smart antenna */
	DBG_FUNC_TX = 0x400 << 8,
	DBG_FUNC_TXQ = 0x401 << 8,  /* debug flag for tx Q mgmt */
	DBG_FUNC_TMAC_INFO = 0x402 << 8, /* debug flag for tmac info dump */
	DBG_FUNC_RX = 0x800 << 8, /* debug flag for tmac info dump */
	DBG_FUNC_PS = 0x1000 << 8, /* debug flag for power saving */
	DBG_FUNC_UAPSD = 0x2000 << 8, /* debug flag for uapsd */
	DBG_FUNC_MAX = DBG_FUNC_UAPSD,
}RT_DEBUG_FUNC;


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
#define OPMODE_APSTA				2       /* as AP and STA at the same time */
    
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
    (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))
 
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
#ifdef WH_EVENT_NOTIFIER
#define CUSTOM_IE_TOTAL_LEN 128
#endif /* WH_EVENT_NOTIFIER */
    
/* */
/* IEEE 802.11 Structures and definitions */
/* */
#define MAX_TX_POWER_LEVEL              100   /* mW */
#define MAX_RSSI_TRIGGER                -10    /* dBm */
#define MIN_RSSI_TRIGGER                -200   /* dBm */
#define MAX_FRAG_THRESHOLD              2346  /* byte count */
#define MIN_FRAG_THRESHOLD              256   /* byte count */
#define MAX_RTS_THRESHOLD               2347  /* byte count */

typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE 
 { 
Ndis802_11IBSS, 
Ndis802_11Infrastructure, 
Ndis802_11AutoUnknown, 
Ndis802_11Monitor, 
Ndis802_11InfrastructureMax	/* Not a real value, defined as upper bound */
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;




/* ======================== Memory ========================================== */ 
#ifdef VENDOR_FEATURE2_SUPPORT

extern ULONG OS_NumOfPktAlloc, OS_NumOfPktFree;

#define MEM_DBG_PKT_ALLOC_INC(__pPacket)	OS_NumOfPktAlloc ++;
#define MEM_DBG_PKT_FREE_INC(__pPacket)		OS_NumOfPktFree ++;
#else
#define MEM_DBG_PKT_ALLOC_INC(__pPacket)
#define MEM_DBG_PKT_FREE_INC(__pPacket)
#endif /* VENDOR_FEATURE2_SUPPORT */

    
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
typedef enum _RTMP_INF_TYPE_
{	
	RTMP_DEV_INF_UNKNOWN = 0,
	RTMP_DEV_INF_PCI = 1,
	RTMP_DEV_INF_USB = 2,
	RTMP_DEV_INF_RBUS = 4,
	RTMP_DEV_INF_PCIE = 5,
	RTMP_DEV_INF_SDIO= 6,
}RTMP_INF_TYPE;

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)		if(_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)		if(_pAd->OpMode == OPMODE_STA)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)		if (__OpMode == OPMODE_AP)
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
typedef struct __RTMP_IOCTL_INPUT_STRUCT
{
	union
	{
		CHAR *name;
		struct
		{
			CHAR *pointer;
			UINT16 length;
			UINT16 flags;
		} data;
	} u;
} RTMP_IOCTL_INPUT_STRUCT, *PRTMP_IOCTL_INPUT_STRUCT;


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
typedef struct _LIST_RESOURCE_OBJ_ENTRY
{
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
#define CFG80211DBG(__Flg, __pMsg)		DBGPRINT(__Flg, __pMsg)
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


#ifdef TX_PKT_SG
#ifndef MAX_SKB_FRAGS 
#define MAX_SKB_FRAGS (65536/(1UL << 12) + 2)
#endif
typedef struct _PTK_SG_T{
	VOID *data;
	INT len;
}PKT_SG_T;
#endif /* TX_PKT_SG */
/*
	Packet information for NdisQueryPacket
*/
typedef struct  _PACKET_INFO    {
	UINT PhysicalBufferCount;    /* Physical breaks of buffer descripor chained */
	UINT BufferCount;           /* Number of Buffer descriptor chained */
	UINT TotalPacketLength ;     /* Self explained */
	PNDIS_BUFFER pFirstBuffer;   /* Pointer to first buffer descriptor */
#ifdef TX_PKT_SG
	PKT_SG_T sg_list[MAX_SKB_FRAGS];
#endif /* TX_PKT_SG */
} PACKET_INFO, *PPACKET_INFO;


#define MAC_ADDR_LEN                    6

#define IS_BM_MAC_ADDR(Addr)				(((Addr[0]) & 0x01) == 0x01)
#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))



#define RLT_MAC_BASE 0x01

#ifdef USB_IOT_WORKAROUND2
#define W2_IOT_MAX_PADDING_SIZE 1028 //512+512+4
#define W2_MAX_TX_SIZE 3000
#define W2_MAX_HTTX_SIZE 32768
#endif

#ifdef WH_EZ_SETUP
#define	LINE_LEN	(4+33+20+23+9+7+7+3)	/* Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType*/

BOOLEAN ascii2int(RTMP_STRING *in, UINT32 *out);
#endif
#ifdef NEW_IXIA_METHOD
extern unsigned short dectlen_l;
extern unsigned short dectlen_m;
extern unsigned short dectlen_h;
#define IS_OSEXPECTED_LENGTH(len) (((len >= (dectlen_l - 4)) && (len <= dectlen_l))\
		|| ((len >= (dectlen_m - 8)) && (len <= (dectlen_m + 8)))\
		|| ((len >= (dectlen_h - 8)) && (len <= (dectlen_h + 8))))
extern int rx_pkt_len;
extern int rx_pkt_to_os;
#endif
#endif /* __RT_COMM_H__ */

