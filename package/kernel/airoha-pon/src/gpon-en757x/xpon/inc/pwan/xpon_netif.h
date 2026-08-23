#ifndef _XPON_NETIF_H_
#define _XPON_NETIF_H_

#include <linux/netdevice.h>
#include <linux/version.h>

#include "common/drv_types.h"
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_qdma.h>
#else
#include "qdma_api.h"
#endif

#define CPU_PROTECTION_PKT_RATELIMIT	(6000)

#define FB_SPECIAL_VLAN         4093
#define setVID(x) 				((x) & 0x0FFF)
#define getVID(x)         		((x) & 0x0FFF)
#define VLAN_HEAD_LEN           4
#ifndef htons
#define htons			OS_HTONS
#endif /* htons */

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)

#define LINE_NUM				2
#define BEARER_NUM				2
#define PREEMPT_NUM				2
#define PATH_NUM				(LINE_NUM * BEARER_NUM * PREEMPT_NUM)

#define TX_QUEUE_NUM			8
#define DEF_PRIORITY_PKT_CHK_LEN 100
#define ETHER_TYPE_IP		0x0800
#define ETHER_TYPE_8021Q	0x8100
#define write_reg_word(reg, wdata) 	regWrite32(reg, wdata)

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1  
#endif

#define TCSUPPORT_HW_QOS

#ifdef QOS_REMARKING  
#define QOS_REMARKING_MASK    	0x00000007
#define QOS_REMARKING_FLAG    	0x00000001
#endif

#ifdef TCSUPPORT_QOS
#define		QOS_FILTER_MARK		0xf0
#define 	QOS_HH_PRIORITY		0x10
#define 	QOS_H_PRIORITY		0x20
#define 	QOS_M_PRIORITY		0x30
#define		QOS_L_PRIORITY		0x40

#define		NULLQOS				-1
#define 	QOS_HW_WRR			3
#define		QOS_HW_PQ			4
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#define		QOS_HW_CAR			5
#define		QOS_PRIORITY_DEFAULT 	0x00000080	
#define		CAR_QUEUE_NUM				6
#endif
#endif
#endif


#ifndef ETH_ADDR_LEN
	#define ETH_ADDR_LEN						(6)
#endif /* ETH_ADDR_LEN */
#define SKB_ALIGNMENT							(4)


#define SOFT_LOOPBACK_MODE(priv)				(priv->drvLb)

#define PWAN_BASE_MAC_ADDR         				{0x00, 0x0C, 0xE7, 0x01, 0x23, 0x45}
#define PWAN_MAX_COPY_LENS						(0) /* 0 mean never copy for rx skb */
#define PWAN_MAX_MTU							(2000)
#define PWAN_MIN_TX_COUNT						(3)
#define PWAN_MAX_TX_COUNT						(128)
#define PWAN_MAX_PACKET_COUNT					(CONFIG_TX_DSCP_NUM-8)


typedef enum {
	PWAN_IF_OAM = 0,
	PWAN_IF_OMCI,
	PWAN_IF_DATA,
	PWAN_IF_NUMS
} PWAN_IfType_t ;

enum QueInfo{
	queue0	= 0, /* used for qos*/
	queue1,		 /* used for qos*/
	queue2,		 /* used for qos*/
	queue3,		 /* used for qos*/
	queue4,		 /* reserved */
	queue5,	 	 /* reserved */
	queue6,		 /* used for ppp, dhcp and some high priority packet */
	queue7,	     /* used for oam omci */
};

/****************************************************************
 xPON message format
****************************************************************/
#define TXMSG_FPORT_PDMA						(0x0)
#define TXMSG_FPORT_GSW							(0x1)
#define TXMSG_FPORT_GMAC						(0x2)
#define TXMSG_FPORT_PPE							(0x4)
#define TXMSG_FPORT_QDMA_CPU					(0x5)
#define TXMSG_FPORT_QDMA_HW						(0x6)
#define TXMSG_FPORT_DROP						(0x6)


typedef union {
	struct {
#ifdef TCSUPPORT_CPU_EN7521
#ifdef __BIG_ENDIAN
		uint resv1				: 8 ;
		uint gem				: 12 ;
		uint oam				: 1 ;
		uint channel			: 8 ;
		uint longf				: 1 ;	
		uint runtf				: 1 ;	
		uint crcer				: 1 ;
#else
		uint crcer				: 1 ;
		uint runtf				: 1 ;	
		uint longf				: 1 ;
		uint channel			: 8 ;
		uint oam				: 1 ;
		uint gem				: 12 ;
		uint resv1				: 8 ;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
		uint resv1				: 12 ;
		uint gem				: 12 ;
		uint crcer				: 1 ;	
		uint longf				: 1 ;	
		uint runtf				: 1 ;	
		uint oam				: 1 ;
		uint channel			: 4 ;
#else
		uint channel			: 4 ;
		uint oam				: 1 ;
		uint runtf				: 1 ;	
		uint longf				: 1 ;	
		uint crcer				: 1 ;	
		uint gem				: 12 ;
		uint resv1				: 12 ;
#endif /* __BIG_ENDIAN */
#endif

#ifdef __BIG_ENDIAN
		uint resv2				: 3 ;
		uint ipv6				: 1 ;
		uint ipv4				: 1 ;	
		uint ipv4f				: 1 ;	
		uint tack				: 1 ;	
		uint l4vld				: 1 ;	
		uint l4f				: 1 ;	
		uint sport				: 4 ;	
		uint crsn				: 5 ;	
		uint ppe				: 14 ;
#else
		uint ppe				: 14 ;
		uint crsn				: 5 ;	
		uint sport				: 4 ;	
		uint l4f				: 1 ;	
		uint l4vld				: 1 ;	
		uint tack				: 1 ;	
		uint ipv4f				: 1 ;	
		uint ipv4				: 1 ;	
		uint ipv6				: 1 ;
		uint resv2				: 3 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
		uint resv3				: 13 ;
		uint vlan				: 1 ;
		uint tpid				: 2 ;	
		uint vid				: 16 ;	
#else
		uint vid				: 16 ;	
		uint tpid				: 2 ;	
		uint vlan				: 1 ;
		uint resv3				: 13 ;
#endif /* __BIG_ENDIAN */

		uint timestamp ;
	} raw ;
	uint word[4] ;
} PWAN_FERxMsg_T ;


typedef union {
	struct {
#ifdef TCSUPPORT_CPU_EN7521
#ifdef __BIG_ENDIAN
		uint resv				: 1 ;
		uint tsid				: 5 ;
		uint tse				: 1 ;
		uint dei				: 1 ;
		uint gem				: 12 ;
		uint oam				: 1 ;
		uint channel			: 8 ;
		uint queue				: 3 ;	
#else
		uint queue				: 3 ;	
		uint channel			: 8 ;
		uint oam				: 1 ;
		uint gem				: 12 ;
		uint dei				: 1 ;
		uint tse				: 1 ;
		uint tsid				: 5 ;
		uint resv				: 1 ;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
		uint resv2				: 1 ;
		uint tsid				: 5 ;
		uint tse				: 1 ;
		uint dei				: 1 ;
		uint resv1				: 3 ;
		uint gem				: 12 ;
		uint oam				: 1 ;
		uint channel			: 4 ;
		uint queue				: 4 ;	
#else
		uint queue				: 4 ;	
		uint channel			: 4 ;
		uint oam				: 1 ;
		uint gem				: 12 ;
		uint resv1				: 3 ;
		uint dei				: 1 ;
		uint tse				: 1 ;
		uint tsid				: 5 ;
		uint resv2				: 1 ;
#endif /* __BIG_ENDIAN */
#endif

#ifdef __BIG_ENDIAN
		uint ico				: 1 ;
		uint uco				: 1 ;
		uint tco				: 1 ;
		uint tso				: 1 ;
		uint pmap				: 6 ;
		uint fport				: 3 ;
		uint insv				: 1 ;
		uint tpid				: 2 ;	
		uint vid				: 16 ;	
#else
		uint vid				: 16 ;	
		uint tpid				: 2 ;	
		uint insv				: 1 ;
		uint fport				: 3 ;
		uint pmap				: 6 ;
		uint tso				: 1 ;
		uint tco				: 1 ;
		uint uco				: 1 ;
		uint ico				: 1 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint word[2] ;
} PWAN_FETxMsg_T ;


typedef struct {
	unchar						netIdx ;
	unchar						drvLb ;
#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct napi_struct  		napi ;
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	struct net_device_stats		stats ;
} PWAN_NetPriv_T ;

/********************************************************
********************************************************/
#include "pwan/gpon_wan.h"
#include "pwan/epon_wan.h"
#include "xmcs/xmcs_if.h"


typedef struct {
	struct {
		unchar						isRandomLb 	: 1 ;
		unchar						isQosUp		: 1 ;
		unchar                      isTxDropOmcc: 1 ;
		unchar						resv 		: 5 ;
	} flags ;
#ifndef TCSUPPORT_CPU_EN7521
	uint							txPacketCount ;
#endif
} PWAN_Config_T ;

typedef struct {
	struct net_device  				*pPonNetDev[PWAN_IF_NUMS] ;
	uint							dropUnknownPackets ;
	uint 							dropForHookBuf;
	uint 							activeChannelNum;
	unchar						greenMaxthreshold;
#ifdef TCSUPPORT_WAN_GPON
	GWAN_Priv_T						gpon ;
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
	EWAN_Priv_T						epon ;
#endif /* TCSUPPORT_WAN_GPON */
#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
	struct timer_list pwan_timer;
#endif
	struct timer_list txDropTimer;
	struct XMCS_VLAN_CFG_INFO_S 	fhVlan;
	PWAN_Config_T					devCfg ;
	spinlock_t						rxLock ;
#ifndef TCSUPPORT_CPU_EN7521
	spinlock_t						txLock ;
#endif
} PWAN_GlbPriv_T ;


/********************************************************
********************************************************/
void __dump_skb(struct sk_buff *skb, uint pktLen) ;

void pwan_destroy(void) ;
int pwan_init(void) ;

int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev);
int get_pon_link_type(void);
/* enable CPU path us traffic */
void enable_cpu_us_traffic(void);
/* disable CPU path us traffic */
void disable_cpu_us_traffic(void);

int pwan_prepare_rx_buffer(PWAN_FERxMsg_T *pRxBmMsg, struct sk_buff *skb) ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	int pwan_cb_prepare_tx_message(PWAN_FETxMsg_T *pMsg, struct sk_buff *skb) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
int pwan_tls_pon_passthrough(struct sk_buff *skb);
#endif
#ifndef TCSUPPORT_CPU_EN7521
int pwan_cb_tx_finished(void *msg_p, struct sk_buff *skb) ;
#endif
int pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen) ;
int pwan_cb_event_handler(QDMA_EventType_t event) ;
int pwan_create_net_interface(uint idx) ;
int pwan_delete_net_interface(uint idx) ;

#ifdef TCSUPPORT_EPON_DUMMY
    extern struct sk_buff * gDummySkb;

    static inline int isDummyPkt(struct sk_buff * skb)
    {
    	return  gDummySkb == skb;
    	
    }
#else
    #define isDummyPkt(skb) (0)
#endif /*TCSUPPORT_EPON_DUMMY*/

int getVlanType(unsigned short TPID);
#ifdef TCSUPPORT_XPON_HAL_API_MCST
extern int (*mtk_multicast_data_handle_hook)(struct sk_buff *skb);
#endif
int fb_pwan_tx_vlan_proc(struct sk_buff *skb);
int fb_pwan_rx_vlan_proc(struct sk_buff *skb);
int fb_pwan_tx_vlan_trans_proc(struct sk_buff *skb);


#endif /* _XPON_NETIF_H_ */





