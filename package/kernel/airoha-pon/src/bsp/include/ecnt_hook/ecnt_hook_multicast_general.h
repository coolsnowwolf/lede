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
#ifndef _ECNT_HOOK_MULTICAST_GENERAL_H
#define _ECNT_HOOK_MULTICAST_GENERAL_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
typedef enum
{
	ECNT_VLAN_TRANSPARENT = 0,
	ECNT_VLAN_REPLACE,
	ECNT_VLAN_REPLACE_AND_REMARK,
	ECNT_VLAN_STRIP,
	ECNT_VLAN_ADD,
	ECNT_VLAN_ADD_AND_REMARK
}ECNT_VLAN_MODE;

/* filter mode */
typedef enum
{
	ECNT_FWD_MODE_IP_AND_VID = 0,
	ECNT_FWD_MODE_MAC,
		ECNT_FWD_MODE_IP_AND_SRC_IP,
	ECNT_FWD_MODE_MAC_AND_VID,
	ECNT_FWD_MODE_MAC_AND_IP,
	ECNT_FWD_MODE_MAC_AND_IP_AND_VID,
}ECNT_MC_FWD_MODE;

typedef enum
{
	ECNT_IGMP_SNOOPING_OFF  = 0,
	ECNT_IGMP_SNOOPING_ON   = 1,
}ECNT_IGMP_SNOOPING_Mode;

typedef enum
{
	ECNT_IGMP_PKT_NONE = 0,
	ECNT_IGMP_PKT_DATA = 1,
	ECNT_IGMP_PKT_CONTROL,
}ECNT_IGMP_PKT_TYPE;

enum ECNT_MULTICAST_GENERAL_SUBTYPE
{
	ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK = 0,
	ECNT_MC_XPON_MULTICAST_EXTERNAL_HOOK,
	ECNT_MC_XPON_MULTICAST_API_HOOK,
};

typedef enum
{
	ECNT_MC_INTERVAL_FUNCTION_DATA_HANDLE_HOOK = 0,
	ECNT_MC_INTERVAL_FUNCTION_ADD_TABLE_HOOK,
	ECNT_MC_INTERVAL_FUNCTION_UPDATE_TABLE_HOOK,
	ECNT_MC_INTERVAL_FUNCTION_DEL_TABLE_HOOK,
	ECNT_MC_INTERVAL_FUNCTION_UPDATE_VLAN_ACTION_HOOK,
	ECNT_MC_INTERNAL_FUNCTION_GET_WIFI_NUM_HOOK,
	ECNT_MC_INTERVAL_FUNCTION_ADD_OR_UPDATE_TABLE_HOOK,	
	ECNT_MC_INTERVAL_FUNCTION_MAX_NUM ,
}ECNT_MULTICAST_INTERNAL_FUNC_ID;

typedef enum
{
	ECNT_MC_EXTERNAL_FUNCTION_GET_PORTMASK_HOOK = 0,
	ECNT_MC_EXTERNAL_FUNCTION_GET_VLAN_ACTION_HOOK,
	ECNT_MC_EXTERVAL_FUNCTION_MAX_NUM ,
}ECNT_MULTICAST_EXTERNAL_FUNC_ID;

typedef enum
{
	ECNT_MC_API_SET_RX_CB_HOOK = 0,
	ECNT_MC_API_XMIT_HOOK,
	ECNT_MC_API_VLAN_ONETON_HOOK,
	ECNT_MC_API_VLAN_HANDLE_HOOK,
	
	ECNT_MC_API_ADD_ENTRY_HOOK,
	ECNT_MC_API_DEL_ENTRY_HOOK,
	ECNT_MC_API_UPDATE_ENTRY_HOOK,
	ECNT_MC_API_CLEAR_ENTRY_BY_PORT_HOOK,
	ECNT_MC_API_CLEAR_ENTRY_ALL_HOOK,
	ECNT_MC_API_GET_ENTRY_HOOK,
	ECNT_MC_API_SET_FILTER_MODE_HOOK,
	ECNT_MC_API_GET_FILTER_MODE_HOOK,
	ECNT_MC_API_SET_SNOOPING_MODE_HOOK,
	ECNT_MC_API_GET_SNOOPING_MODE_HOOK,
		ECNT_MC_API_SET_PORT_TYPE_HOOK,
	ECNT_MC_API_MAX_NUM ,
}ECNT_MULTICAST_API_FUNC_ID;

/************************************************************************
*               M A C R O S
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef int (*ecnt_mc_ctrl_packet_rx)(unsigned char port_id,unsigned char *data,unsigned int data_len);

typedef struct
{    
	unsigned char  mac_address_byte [6];
}ECNT_MAC_ADDRESS ;

typedef struct
{    
	unsigned char ip_address_array [16] ;
} ECNT_IPV6_ADDRESS ;

typedef struct
{ 
	union{        
		/* IPv4*/         
		unsigned int ipv4_address ;          
		/* IPv6*/         
		ECNT_IPV6_ADDRESS ipv6_address ;     
	}IP ;    
		
	/*True = ipv6, False = ipv4*/     
	bool is_ipv6 ;
}ECNT_IP_ADDRESS ;

typedef struct
{
	unsigned char idx;
	unsigned int pkt_base;
	unsigned int pkt_acnt;
	unsigned long long byte_base;
	unsigned long long byte_acnt;
}ECNT_MC_ACNT;

typedef struct
{
	ECNT_VLAN_MODE vlanMode;
	unsigned short outerVid;
	unsigned short innerVid;
	unsigned char outerTci;
	unsigned char innerTci;
}ECNT_VLAN_ACTION;

typedef struct
{
	ECNT_VLAN_MODE vlan_mode;
	unsigned short outer_tpid;
	unsigned short outer_tci;
	unsigned short inner_tpid;
	unsigned short inner_tci;
}ECNT_VLAN_INFO;/* support tpid & tci */

typedef struct
{
	unsigned char port_id;
	short int vlan_id;
	unsigned int pkt_cnt;
	unsigned long long byte_cnt;
	ECNT_IP_ADDRESS group_addr;
		ECNT_IP_ADDRESS src_addr;
	ECNT_MAC_ADDRESS group_mac;
	ECNT_VLAN_INFO vlan_action;
}ECNT_MC_PORT_INFO;

typedef struct
{
	unsigned char port_id;
	int entry_num; /* how many multicast entrys this port has */
	unsigned int pcnt;
	unsigned long long bcnt;
	ECNT_MC_PORT_INFO *pEntry;
}ECNT_MC_PORT_INFO_GET;

typedef struct
{
	struct list_head list;
	int flow_idx;
	unsigned char port_id;
	short int vlan_id;
	ECNT_IP_ADDRESS group_addr;
		ECNT_IP_ADDRESS src_addr;
	ECNT_MAC_ADDRESS group_mac;
	ECNT_VLAN_INFO vlan_action;
	ECNT_MC_ACNT acnt;
}ECNT_MC_FWD_TABLE_ENTRY;

typedef struct
{
	ECNT_MC_FWD_MODE fwd_mode;
	struct list_head fwd_list;
}ECNT_MC_FWD_TABLE;

typedef struct
{
	unsigned char dest_port;
	unsigned char src_port;
	void *data;
	unsigned int data_len;
}ECNT_MC_TX_INFO;

typedef struct
{
	ecnt_mc_ctrl_packet_rx rx_cb_func;
}ECNT_MC_RX_INFO;

typedef struct
{
	ECNT_MULTICAST_INTERNAL_FUNC_ID in_function_id;	
	ECNT_MULTICAST_EXTERNAL_FUNC_ID ex_function_id;
	ECNT_MULTICAST_API_FUNC_ID api_function_id;	
	struct sk_buff *skb;
	union
	{
		struct 
		{
			unsigned short  vid;
			ECNT_MAC_ADDRESS mac_addr;
			ECNT_IP_ADDRESS src_ip;
			ECNT_IP_ADDRESS group_ip;
			unsigned int *fwd_port_mask;
		}port_mask;
		struct
		{
			unsigned int port_id;
			ECNT_IP_ADDRESS group_ip;
			ECNT_VLAN_ACTION *action;
		}vlan_action;
		struct
		{
			unsigned int foe_index;
			unsigned int* wifi_num;
			unsigned int* port_mask;
		}wifi_cfg;
		struct
		{
			ECNT_IP_ADDRESS group_ip;
			unsigned int port_mask;
		}update_entry;
			struct
			{
				int type;
				char *name;
			}port_type;
		/* --------- BSP_General new APIs --------- */
		unsigned char port_id;
		ECNT_MC_FWD_MODE *filter_mode;
		unsigned char *snooping_mode;
		ecnt_mc_ctrl_packet_rx rx_cb;
		ECNT_MC_TX_INFO tx_data;
		ECNT_MC_PORT_INFO *port_info;
		ECNT_MC_PORT_INFO_GET *port_info_get;
	}mc_private;
	int retValue;
}ecnt_mc_general_data_s;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int ECNT_HOOK_MULTICAST_DATA_HANLDE(struct sk_buff *skb)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_DATA_HANDLE_HOOK;
	in_data.skb = skb;
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_ADD_IPTV_TABLE_ENTRY(ECNT_IP_ADDRESS group_ip,unsigned int port_mask)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_ADD_TABLE_HOOK;

	memmove(&in_data.mc_private.update_entry.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	in_data.mc_private.update_entry.port_mask = port_mask;
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_UPDATE_IPTV_TABLE_ENTRY(ECNT_IP_ADDRESS group_ip,unsigned int port_mask)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_UPDATE_TABLE_HOOK;

	memmove(&in_data.mc_private.update_entry.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	in_data.mc_private.update_entry.port_mask = port_mask;
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_DEL_IPTV_TABLE_ENTRY(ECNT_IP_ADDRESS group_ip)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_DEL_TABLE_HOOK;
	memmove(&in_data.mc_private.update_entry.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_UPDATE_VLAN_ACTION_ENTRY(ECNT_IP_ADDRESS group_ip)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_UPDATE_VLAN_ACTION_HOOK;
	memmove(&in_data.mc_private.update_entry.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_MULTICAST_GET_WIFI_NUM_AND_MASK(unsigned int foe_index, unsigned int* wifi_num,unsigned int* port_mask)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERNAL_FUNCTION_GET_WIFI_NUM_HOOK;
	in_data.mc_private.wifi_cfg.foe_index = foe_index;
	in_data.mc_private.wifi_cfg.wifi_num = wifi_num;
	in_data.mc_private.wifi_cfg.port_mask = port_mask;

	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_GET_FWD_PORTMAK(unsigned short  vid,ECNT_MAC_ADDRESS mac_addr, ECNT_IP_ADDRESS src_ip,ECNT_IP_ADDRESS group_ip,unsigned int *fwd_port_mask)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.ex_function_id = ECNT_MC_EXTERNAL_FUNCTION_GET_PORTMASK_HOOK;
	in_data.mc_private.port_mask.vid = vid;
	memmove(&in_data.mc_private.port_mask.mac_addr,&mac_addr,sizeof(ECNT_MAC_ADDRESS));
	memmove(&in_data.mc_private.port_mask.src_ip,&src_ip,sizeof(ECNT_IP_ADDRESS));
	memmove(&in_data.mc_private.port_mask.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	in_data.mc_private.port_mask.fwd_port_mask = fwd_port_mask;
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_EXTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_MULTICAST_GET_VLAN_ACTION(unsigned int port_id, ECNT_IP_ADDRESS group_ip,ECNT_VLAN_ACTION* aciton)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.ex_function_id = ECNT_MC_EXTERNAL_FUNCTION_GET_VLAN_ACTION_HOOK;
	in_data.mc_private.vlan_action.port_id = port_id;
	memmove(&in_data.mc_private.vlan_action.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	in_data.mc_private.vlan_action.action = aciton;
	
	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_EXTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_HOOK_ADD_OR_UPDATE_IPTV_TABLE_ENTRY(unsigned short group_vid, ECNT_IP_ADDRESS group_ip)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.in_function_id = ECNT_MC_INTERVAL_FUNCTION_ADD_OR_UPDATE_TABLE_HOOK;
	in_data.mc_private.port_mask.vid = group_vid;
	memmove(&in_data.mc_private.port_mask.group_ip,&group_ip,sizeof(ECNT_IP_ADDRESS));
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL,ECNT_MC_XPON_MULTICAST_INTERNAL_HOOK,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

//////////////////////////////BSP_General New APIs/////////////////////////////////
/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_SET_RECV_CALLBACK
**      descriptions:
**      	customers call this function to register IGMP control packet receive function
**      input parameters:
**		rx_cb: ecnt_mc_ctrl_packet_rx
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_SET_RECV_CALLBACK(ecnt_mc_ctrl_packet_rx rx_cb)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	if (rx_cb == NULL){
		return ECNT_HOOK_ERROR;
	}
	in_data.api_function_id = ECNT_MC_API_SET_RX_CB_HOOK;
	in_data.mc_private.rx_cb = rx_cb;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

	return ret;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_XMIT
**      descriptions:
**      	customers call this function to send packet, if can not handle
**      input parameters:
**		dest_port:   packet dest port
**		src_port:     packet source port
**		data:          packet data
**		data_len:    packet data len
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_XMIT(unsigned char dest_port, unsigned char src_port, void *data, unsigned int data_len)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_XMIT_HOOK;
	in_data.mc_private.tx_data.dest_port = dest_port;
	in_data.mc_private.tx_data.src_port = src_port;
	in_data.mc_private.tx_data.data_len = data_len;
	in_data.mc_private.tx_data.data = data;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

	return ret;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_IS_ONE_TO_N
**      descriptions:
**      	whether multicast vlan 1:N, for hw_nat tx handler.
**          If yes, learn hit bind & force to CPU flow
**          If not, learn full hw flow
**      input parameters:
**		skb: data flow
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_IS_ONE_TO_N(struct sk_buff *skb)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_VLAN_ONETON_HOOK;
	in_data.skb = skb;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

	return ret;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_VLAN_HANDLE
**      descriptions:
**      	multicast data vlan handle, hit bind & force to CPU
**      input parameters:
**		skb: data flow
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_VLAN_HANDLE(struct sk_buff *skb)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_VLAN_HANDLE_HOOK;
	in_data.skb = skb;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

	return ret;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_ADD_ENTRY
**      descriptions:
**      	add multicast entry
**      input parameters:
**		pdata:   ECNT_MC_PORT_INFO
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_ADD_ENTRY(ECNT_MC_PORT_INFO *pdata)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_ADD_ENTRY_HOOK;
	in_data.mc_private.port_info = pdata;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_DEL_ENTRY
**      descriptions:
**      	del multicast entry
**      input parameters:
**		pdata:   ECNT_MC_PORT_INFO
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_DEL_ENTRY(ECNT_MC_PORT_INFO *pdata)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_DEL_ENTRY_HOOK;
	in_data.mc_private.port_info = pdata;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_UPDATE_ENTRY
**      descriptions:
**      	update multicast entry
**      input parameters:
**		pdata:   ECNT_MC_PORT_INFO
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_UPDATE_ENTRY(ECNT_MC_PORT_INFO *pdata)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_UPDATE_ENTRY_HOOK;
	in_data.mc_private.port_info = pdata;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_CLEAR_ENTRY_BY_PORT
**      descriptions:
**      	clear multicast entry by port
**      input parameters:
**		port_id:   
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_CLEAR_ENTRY_BY_PORT(unsigned char port_id)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_CLEAR_ENTRY_BY_PORT_HOOK;
	in_data.mc_private.port_id = port_id;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_CLEAR_ENTRY_ALL
**      descriptions:
**      	clear multicast entry all
**      input parameters:
**		N/A  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_CLEAR_ENTRY_ALL(void)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_CLEAR_ENTRY_ALL_HOOK;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_GET_ENTRY
**      descriptions:
**      	get multicast entrys 
**      input parameters:
**		N/A  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_GET_ENTRY(ECNT_MC_PORT_INFO_GET *port_info_get)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_GET_ENTRY_HOOK;
	in_data.mc_private.port_info_get = port_info_get;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_SET_FILTER_MODE
**      descriptions:
**      	set filter mode 
**      input parameters:
**		filter_mode  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_SET_FILTER_MODE(ECNT_MC_FWD_MODE *filter_mode)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_SET_FILTER_MODE_HOOK;
	in_data.mc_private.filter_mode = filter_mode;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_GET_FILTER_MODE
**      descriptions:
**      	set filter mode 
**      input parameters:
**		filter_mode  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_GET_FILTER_MODE(ECNT_MC_FWD_MODE *filter_mode)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_GET_FILTER_MODE_HOOK;
	in_data.mc_private.filter_mode = filter_mode;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_SET_SNOOPING_MODE
**      descriptions:
**      	set filter mode 
**      input parameters:
**		snooping_mode  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_SET_SNOOPING_MODE(unsigned char *snooping_mode)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_SET_SNOOPING_MODE_HOOK;
	in_data.mc_private.snooping_mode = snooping_mode;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

/*___________________________________________________________________________
**      function name: ECNT_HOOK_MC_API_GET_SNOOPING_MODE
**      descriptions:
**      	set filter mode 
**      input parameters:
**		snooping_mode  
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
static inline int ECNT_HOOK_MC_API_GET_SNOOPING_MODE(unsigned char *snooping_mode)
{
	int ret = 0;
	ecnt_mc_general_data_s in_data;

	in_data.api_function_id = ECNT_MC_API_GET_SNOOPING_MODE_HOOK;
	in_data.mc_private.snooping_mode = snooping_mode;
	
 	ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

	/*___________________________________________________________________________
	**		function name: ECNT_HOOK_MC_API_SET_PORT_TYPE
	**		descriptions:
	**			set port type
	**		input parameters:
	**		protName, portType  
	**		output parameters:
	**			N/A
	**
	**		return:
	**			success:	> 0
	**		failure:	-1
	**___________________________________________________________________________
	*/
	static inline int ECNT_HOOK_MC_API_SET_PORT_TYPE(char *portName, int portType)
	{
		int ret = 0;
		ecnt_mc_general_data_s in_data;
	
		in_data.api_function_id = ECNT_MC_API_SET_PORT_TYPE_HOOK;
		in_data.mc_private.port_type.type = portType;
		in_data.mc_private.port_type.name = portName;  
		
		ret = __ECNT_HOOK(ECNT_MULTICAST_GENERAL, ECNT_MC_XPON_MULTICAST_API_HOOK, (struct ecnt_data *)&in_data);
	
		if(ret != ECNT_HOOK_ERROR)
			return in_data.retValue;
		else
			return ECNT_HOOK_ERROR;
	}
	
#endif /*_ECNT_HOOK_MULTICAST_GENERAL_H*/
