/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _XPON_NETIF_H
#define _XPON_NETIF_H


/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/netdevice.h>
#include <linux/version.h>


#include <ecnt_hook/ecnt_hook_qdma.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

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


#define PWAN_BASE_MAC_ADDR         				{0x00, 0x0C, 0xE7, 0x01, 0x23, 0x45}
#define PWAN_MAX_MTU							(2000)
#define PWAN_MIN_TX_COUNT						(3)
#define PWAN_MAX_TX_COUNT						(128)
#define PWAN_MAX_PACKET_COUNT					(CONFIG_TX_DSCP_NUM-8)

#ifdef CONFIG_TP_IMAGE
  /* fix bug 856251, wan mark is conflic with 8021.p remark, cause pbit incorrect and packet is dropped by olt */
  #ifdef CONFIG_8021P_REMARK
  #undef CONFIG_8021P_REMARK
  #endif
#else
#define CONFIG_8021P_REMARK                     
#endif /* CONFIG_TP_IMAGE */
#ifdef CONFIG_8021P_REMARK
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN			    4
#define VLAN_ETH_ALEN			6
#endif

#ifndef ETH_ADDR_LEN
	#define ETH_ADDR_LEN						(6)
#endif /* ETH_ADDR_LEN */

typedef enum {
	PWAN_IF_OAM = 0,
	PWAN_IF_OMCI,
	PWAN_IF_EAPOL,
	PWAN_IF_DATA,
	PWAN_IF_NUMS
} PWAN_IfType_t ;

typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint resv1				: 1 ;
		uint no_mic				: 1 ;
		uint gem				: 16 ;
		uint longf				: 1 ;
		uint runtf				: 1 ;
		uint crcer				: 1 ;
        uint resv2				: 2 ;
		uint oam				: 1 ;	
		uint channel			: 5 ;	
		uint resv3				: 3 ;
#else
		uint resv3				: 3 ;
		uint channel			: 5 ;	
		uint oam				: 1 ;
        uint resv2				: 2 ;
		uint crcer				: 1 ;
		uint runtf				: 1 ;
		uint longf				: 1 ;
		uint gem				: 16 ;
		uint no_mic				: 1 ;
		uint resv1				: 1 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
#if defined(TCSUPPORT_CPU_EN7581) 
		uint dei				: 1 ;
		uint ipv6				: 1 ;
		uint ipv4				: 1 ;	
		uint ipv4f				: 1 ;		
		uint l4vld				: 1 ;	
		uint l4f				: 1 ;	
		uint sport				: 5 ;	
		uint crsn				: 5 ;	
		uint ppe				: 16 ;
#else
		uint resv4				: 1 ;
		uint dei				: 1 ;
		uint ipv6				: 1 ;
		uint ipv4				: 1 ;	
		uint ipv4f				: 1 ;		
		uint l4vld				: 1 ;	
		uint l4f				: 1 ;	
		uint sport				: 5 ;	
		uint crsn				: 5 ;	
		uint ppe				: 15 ;
#endif
#else
#if defined(TCSUPPORT_CPU_EN7581) 
		uint ppe				: 16 ;
		uint crsn				: 5 ;	
		uint sport				: 5 ;	
		uint l4f				: 1 ;	
		uint l4vld				: 1 ;		
		uint ipv4f				: 1 ;	
		uint ipv4				: 1 ;	
		uint ipv6				: 1 ;
		uint dei				: 1 ;
#else
		uint ppe				: 15 ;
		uint crsn				: 5 ;	
		uint sport				: 5 ;	
		uint l4f				: 1 ;	
		uint l4vld				: 1 ;		
		uint ipv4f				: 1 ;	
		uint ipv4				: 1 ;	
		uint ipv6				: 1 ;
		uint dei				: 1 ;
		uint resv4				: 1 ;
#endif
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
#if defined(TCSUPPORT_CPU_EN7581) 
		uint aggCnt				: 8 ;
		uint flush_rsn 			: 4 ;
		uint resv6				: 4 ;
		uint ifc_id				: 8 ;
		uint ifc_hit			: 1 ;	
		uint l2_len				: 7 ;	
#else
		uint resv5				: 18 ;
		uint udf				: 6 ;
		uint resv6				: 1 ;	
		uint l2_len				: 7 ;	
#endif
#else
#if defined(TCSUPPORT_CPU_EN7581) 
		uint l2_len				: 7 ;
		uint ifc_hit			: 1 ;
		uint ifc_id				: 8 ;
		uint resv6				: 4 ;
		uint flush_rsn 			: 4 ;
		uint aggCnt				: 8 ;
#else
		uint l2_len				: 7 ;	
		uint resv6				: 1 ;	
		uint udf				: 6 ;
		uint resv5				: 18 ;
#endif
#endif /* __BIG_ENDIAN */
		uint timestamp ;
	} raw ;
	uint word[4] ;
} PWAN_FERxMsg_T ;

typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint resv1				: 1 ;
		uint mic_idx			: 1 ;
		uint gem				: 16 ;
		uint ico		        : 1 ;
		uint uco		        : 1 ;
		uint tco		        : 1 ;
		uint tso		        : 1 ;
		uint fast				: 1 ;
		uint oam				: 1 ;
		uint channel			: 5 ;
		uint queue				: 3 ;	
#else
		uint queue				: 3 ;	
		uint channel			: 5 ;
		uint oam				: 1 ;
		uint fast				: 1 ;
		uint tso		        : 1 ;
		uint tco		        : 1 ;
		uint uco		        : 1 ;
		uint ico		        : 1 ;
		uint gem				: 16 ;
		uint mic_idx			: 1 ;
		uint resv1				: 1 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
		uint ndp				: 1 ;
		uint mtr_g				: 7 ;
#if defined(TCSUPPORT_CPU_EN7581)
		uint fport				: 4 ;
		uint nboq				: 5 ;
		uint resv3				: 4 ;
		uint acnt_g1	        : 5 ;
        uint acnt_g0	        : 6 ; 
#else
		uint fport				: 3 ;
		uint nboq				: 5 ;
		uint resv3				: 6 ;
		uint acnt_g1	        : 5 ;
        uint acnt_g0	        : 5 ; 
#endif
#else

#if defined(TCSUPPORT_CPU_EN7581)
		uint acnt_g0	        : 6 ;    /*0x1F means no count*/
		uint acnt_g1	        : 5 ;    /*bit4=1 means no count*/
		uint resv3				: 4 ;
		uint nboq				: 5 ;
		uint fport				: 4 ;
#else
		uint acnt_g0	        : 5 ;    /*0x1F means no count*/
		uint acnt_g1	        : 5 ;    /*bit4=1 means no count*/
		uint resv3				: 6 ;
		uint nboq				: 5 ;
		uint fport				: 3 ;
#endif
		uint mtr_g				: 7 ;
		uint ndp				: 1 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint word[2] ;
} PWAN_FETxMsg_T ;

#include "pwan/gpon_wan.h"
#include "pwan/epon_wan.h"



typedef struct {
	struct {
		unchar						isRandomLb 	: 1 ;
		unchar						isQosUp		: 1 ;
		unchar                      isTxDropOmcc: 1 ;
		unchar						resv 		: 5 ;
	} flags ;
} PWAN_Config_T ;


typedef struct {
	struct net_device  				*pPonNetDev[PWAN_IF_NUMS] ;	
	uint							dropUnknownPackets ;
	uint 							dropForHookBuf;
	uint							activeChannelNum;
	unchar							greenMaxthreshold;
	GWAN_Priv_T						gpon ;	
#ifdef TCSUPPORT_WAN_EPON
	EWAN_Priv_T 					epon ;
#endif /* TCSUPPORT_WAN_GPON */
#if defined(TCSUPPORT_XPON_LED)
	struct timer_list 				pwan_timer;
#endif
	PWAN_Config_T					devCfg ;
	spinlock_t						rxLock ;
} PWAN_GlbPriv_T ;


typedef struct {
	unchar						netIdx ;
	unchar						drvLb ;
#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct napi_struct  		napi ;
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	struct net_device_stats		stats ;
} PWAN_NetPriv_T ;


#define SOFT_LOOPBACK_MODE(priv)				(priv->drvLb)

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev);
int pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen) ;
int pwan_cb_event_handler(QDMA_EventType_t event) ;
int pwan_init(void) ;
void pwan_destroy(void);
int get_pon_link_type(void);
void __dump_skb(struct sk_buff *skb, uint pktLen) ;
int pwan_create_net_interface(uint idx) ;
#ifdef TCSUPPORT_XPON_HAL_API_MCST
extern int (*mtk_multicast_data_handle_hook)(struct sk_buff *skb);
#endif
bool is_8021x_packet(unsigned char* data);

#endif /*_XPON_NETIF_H*/
