#ifndef _NPU_HOST_ADPT_H_
#define _NPU_HOST_ADPT_H_

#define dma_addr_t  unsigned long int	//npu v2 is 64bit address, but npu v1 is 32bit
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long int

#define HOSTADPT_RX_DSCP_WORD_LENS    4
#define HOSTADPT_RX_RING_NUM          2

#ifdef INCLUDE_WIFI_MTK_MT7992
#define HOSTADPT_RX0_DSCP_NUM         1024
#else /* !INCLUDE_WIFI_MTK_MT7992 */
#define HOSTADPT_RX0_DSCP_NUM         512
#endif /* INCLUDE_WIFI_MTK_MT7992 */

#define HOSTADPT_RX1_DSCP_NUM         512
#define HOSTADPT_RX_TOTAL_DSCP_NUM    (HOSTADPT_RX0_DSCP_NUM + HOSTADPT_RX1_DSCP_NUM)

#if defined(TCSUPPORT_NPU_WIFI_TX) || defined(TCSUPPORT_WLAN_INODE)
#define HOSTADPT_TX_RING_NUM          2
#define HOSTADPT_TX0_DSCP_NUM         1024
#define HOSTADPT_TX1_DSCP_NUM         1024
#define HOSTADPT_TX_TOTAL_DSCP_NUM    (HOSTADPT_TX0_DSCP_NUM + HOSTADPT_TX1_DSCP_NUM)
#endif

#ifdef TCSUPPORT_WLAN_INODE
#define INODE_RX_OFFLOAD 1
#endif
#define HOSTAPD_BUFFER_LEN		     3500//4096
#define HOSTADPT_RX_RING_0            0
#define HOSTADPT_RX_RING_1            1

//#define HOSTADPT_TEST

#define DBG_OFF     0
#define DBG_ERR     1
#define DBG_WRN     2
#define DBG_CATTER     3
#define DBG_LOG     4
#define DBG_MAX     5//must be button

#define DBG_LEVEL   DBG_OFF


typedef struct {
    
    struct {
		u32 done               : 1 ;
		u32 cur_len             : 14;
		u32 len                : 14;
		u32 is_last	           : 1;
		u32 resv1	           : 2;
    } pkt_ctrl ;
    union{
    struct {
#if (defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552) || defined(TCSUPPORT_CPU_AN7583))
		u32 foe_number		: 16;
		u32 crsn		: 5 ;
		u32 src_port		: 5 ;
		u32 pkt_idx              : 6;
#else
		u32 foe_number		: 15;
		u32 crsn		: 5 ;
		u32 src_port		: 5 ;
		u32 pkt_idx              : 7;
#endif
    } pkt_info;
	u32 pktInfo;
     };

	union{
//#ifdef TCSUPPORT_WLAN_INODE
		struct {
			u32 frametype	: 2;
			u32 vap_id		: 4;
			u32 radio_id	: 4;
			u32 priority	: 4;
			u32 fragtype	: 2;
			u32 sid			: 16;
		}pkt_info_inode;
//#endif
		u32 vend_specific;
	};

    u32 	pkt_addr ;
#if defined(TCSUPPORT_CPU_ARMV8_64)
    u64		skb_p;//npuv2 is 64bit address, but npuv1 is 32bit
#else
    u32		skb_p;//npuv2 is 64bit address, but npuv1 is 32bit
    u32		resv4;
#endif
} __attribute__ ((packed)) HOSTADPT_DMA_DSCP_T;


#ifdef TCSUPPORT_WLAN_INODE
#define HOSTADPT_TX_VEND_SPEC_LEN 6
#else
#define HOSTADPT_TX_VEND_SPEC_LEN 192
#endif

typedef struct {
	union {
		u32 word1;
		struct {
			u32 done		: 1 ;
			u32 vnd_len		: 17;
			u32 len			: 13;
			u32 reschedule	: 1 ;
	    } pkt_ctrl ;
	};
	u32		pkt_addr;
#if defined(TCSUPPORT_CPU_ARMV8_64)
	u64		skb_p;//npuv2 is 64bit address, but npuv1 is 32bit
#else
	u32		skb_p;//npuv2 is 64bit address, but npuv1 is 32bit
	u32		resv4;
#endif
//#ifdef TCSUPPORT_NPU_WIFI_TX
	unsigned char vnd_spec[HOSTADPT_TX_VEND_SPEC_LEN];
//#endif
} HOSTADPT_DMA_TX_DSCP_T;


#ifdef TCSUPPORT_NPU_WIFI_TX
struct wifi_tx_info_mt7916{
	unsigned char rept_wds_wcid;
	unsigned char BssInfoIdx;
};

struct wifi_tx_info_eagle{
	unsigned short rept_wds_wcid;
	unsigned char BssInfoIdx;
};
#endif

#ifdef TCSUPPORT_WLAN_INODE
struct wifi_tx_info_ione{
	unsigned char vapid; //4bit
	unsigned char epid; //4bit
	unsigned char sid; //8bit
	unsigned char mcf;//1bit
	unsigned char tid;//4bit
	unsigned char hw_id;//2bit
};
#endif

#endif
