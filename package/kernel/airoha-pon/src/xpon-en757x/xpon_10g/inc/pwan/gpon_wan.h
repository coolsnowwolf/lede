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
#ifndef _GPON_WAN_H
#define _GPON_WAN_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>
#include <xmcs/xmcs_const.h>
#include "gpon/gpon_const.h"
#ifdef TCSUPPORT_RA_HWNAT
	#include <linux/foe_hook.h>
#endif
/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define BROADCAST_OFFSET			(0x10)
#define MULTICAST_OFFSET			(0x20)
#define UNKNOWN_UNICAST_OFFSET		(0x30)
#define GPON_PACKET_LEN_UPPER_LIMIT	(2000)
#define GPON_PACKET_LEN_LOWER_LIMIT	(60)

#define RETURN_RET_SUCCESS 0
#define RETURN_RET_NORMAL_FAIL -1

#define FHNET_IS_DROP_TRUE 1
#define FHNET_IS_DROP_FALSE 0

#define ETH_SKB_MARK_PORT_WAN       (0x0f)
#define ETH_SKB_MARK_PORT_MAX_LAN   (0x03)
#define ETH_SKB_MARK_PORT_MAX_WIFI  (0x0b)
#define ETH_SKB_MARK_PORT_MAX_USB   (0x0d)


#define ETH_GET_LAN_PORT(skb) (((skb->mark) >> 1)&0x0f)
#define ETH_GET_WAN_PORT(skb) (((skb->mark) >> 29)&0x0f)


typedef enum {
	ENUM_CFG_NETIDX = 1 , /* bind gem port id to some virtual ANI id*/
	ENUM_CFG_CHANNEL    , /* */ 
	ENUM_CFG_ENCRYPTION ,
	ENUM_CFG_LOOPBACK
} ENUM_GWanGemCfgType_t ;


typedef struct {
	uint		portId			;    /* physical gemport id, 0~65535      */    
	uint		ani 			;   /* virual ANI idx, internal use only */
	unchar		channel 		;   /* channel number, qdma channel idx  */
	unchar		rxLb			;   /* rx loop back                      */
	unchar		rxEncrypt		;
    unchar		txEncrypt		;
	unchar		valid			;
	ushort      allocId         ; /*from omci tcont me*/
} GWAN_GemInfo_T ;

typedef struct {
	ushort								allocId[CONFIG_GPON_10G_MAX_TCONT] ;
	ushort								gemIdToIndex[GPON_10G_MAX_GEM_ID] ; 
	struct {
		GWAN_GemInfo_T					info ;
		struct net_device_stats 		stats ;
	} gemPort[CONFIG_GPON_10G_MAX_GEMPORT] ;
	uint								gemNumbers ;
    ulong                               rx_omci_cnt;
    ulong                               rx_omci_extend_cnt;
	struct timer_list					gemMibTimer ;	
	uint								hgu_mode_txq ;
} GWAN_Priv_T ;

typedef enum{
    GPON_TRAFFIC_UP =0 ,
    GPON_TRAFFIC_DOWN ,
}GPON_TRAFFIC_STATUS_t;

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
GPON_TRAFFIC_STATUS_t gwan_get_traffic_status(void);
int gwan_create_new_tcont(ushort allocId);
int gwan_remove_tcont(ushort allocId);
int gwan_remove_all_tcont(void);
int gwanCheckAllocIdExist(ushort allocId);
int gwan_create_new_gemport(ushort gemPortId, unchar channel, unchar type, ushort allocId);
int gwan_config_gemport(ushort gemPortId, ENUM_GWanGemCfgType_t cfgType, uint value);
int gwan_remove_gemport(ushort gemPortId);
int gwan_remove_all_gemport_for_disable(void);
int gwan_remove_all_gemport(void);
void gwan_get_gem_mib_table(void);

int gwan_prepare_tx_message(PWAN_FETxMsg_T *pTxMsg, unchar netIdx, struct sk_buff *skb, int txq, struct port_info *xpon_info) ;
int gwan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag) ;
int gwan_init(GWAN_Priv_T *pGWanPriv) ;


#endif /*_GPON_WAN_H*/
