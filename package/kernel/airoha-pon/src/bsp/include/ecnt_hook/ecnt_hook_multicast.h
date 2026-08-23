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
#ifndef _ECNT_HOOK_MULTICAST_H
#define _ECNT_HOOK_MULTICAST_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               M A C R O S
*************************************************************************
*/
#ifndef uint8
#define uint8	unsigned char
#endif
		
#ifndef uint16
#define uint16	unsigned short
#endif
		
#ifndef uint32
#define uint32	unsigned int
#endif


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef int (*mc_ctrl_packet_rx)(uint8 port_id,unsigned char *data,unsigned int data_len);

enum ECNT_MULTICAST_SUBTYPE
{
    ECNT_MC_XMIT_HOOK,
	ECNT_MC_RECV_HOOK,
	ECNT_MC_ADD_ENTRY_HOOK,
	ECNT_MC_DEL_ENTRY_HOOK,
	ECNT_MC_SET_RX_CB_HOOK,
	ECNT_MC_VLAN_ONETON_HOOK,
	ECNT_MC_VLAN_HANDLE_HOOK,
};

typedef struct
{
	uint8 dest_port;
	uint8 src_port;
	void *data;
	unsigned int data_len;
}ecnt_mc_tx_info;

typedef struct
{
	mc_ctrl_packet_rx rx_cb_func;
}ecnt_mc_rx_info;

typedef struct
{
	struct sk_buff *skb;
	int cpu_reason;
}ecnt_mc_data_s;

typedef struct
{    
	unsigned char  mac_address_byte [6];
}ECNT_IGMP_MAC_ADDRESS ;

typedef struct
{    
	unsigned char ip_address_array [16] ;
} ECNT_IGMP_IPV6_ADDRESS ;

typedef struct
{ 
	union{        
		/* IPv4*/         
		uint32 ipv4_address ;          
		/* IPv6*/         
		ECNT_IGMP_IPV6_ADDRESS ipv6_address ;     
		}IP ;    
		
		/*True = ipv6, False = ipv4*/     
		bool is_ipv6 ;
}ECNT_IGMP_IP_ADDRESS ;

/* information provided by App */
struct vlan_info
{
	uint16 vlan_mode;
	uint16 outer_tpid;
	uint16 outer_tci;
	uint16 inner_tpid;
	uint16 inner_tci;
};

typedef enum
{
	IGMP_PKT_NONE = 0,
	IGMP_PKT_DATA = 1,
	IGMP_PKT_CONTROL,
}igmp_pkt_type;

typedef struct
{
	uint8 port_id;
	uint16 vlan_id;
	ECNT_IGMP_IP_ADDRESS group_addr;
	ECNT_IGMP_MAC_ADDRESS group_mac;
	struct vlan_info vlan_action;
}mc_fwd_tbl_entry;
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
int ecnt_mc_api_set_recv_callback(mc_ctrl_packet_rx rx_cb)
{
	int ret = 0;

	if (rx_cb)
	{
    	ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_SET_RX_CB_HOOK,(struct ecnt_data *)&rx_cb);
	}

	return ret;
}

int ecnt_mc_api_xmit(uint8 dest_port, uint8 src_port, void *data,unsigned int data_len)
{
	int ret = 0;
    ecnt_mc_tx_info in_data;

	memset(&in_data,0,sizeof(ecnt_mc_tx_info));
	
	in_data.dest_port = dest_port;
	in_data.src_port = src_port;
	in_data.data_len = data_len;
	in_data.data = data;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_XMIT_HOOK,(struct ecnt_data *)&in_data);

	return ret;
}

int ecnt_mc_api_entry_add(mc_fwd_tbl_entry *pdata)
{
	int ret = 0;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_ADD_ENTRY_HOOK,(struct ecnt_data *)pdata);

	return ret;
}

int ecnt_mc_api_entry_del(mc_fwd_tbl_entry *pdata)
{
	int ret = 0;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_DEL_ENTRY_HOOK,(struct ecnt_data *)pdata);

	return ret;
}

/*___________________________________________________________________________
**      function name: ecnt_mc_api_vlan_IsOneToN
**      descriptions:
**      	whether multicast vlan 1:N
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
int ecnt_mc_api_vlan_IsOneToN(struct sk_buff *skb)
{
	int ret = 0;
	ecnt_mc_data_s in_data;
	
	in_data.skb = skb;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_VLAN_ONETON_HOOK,(struct ecnt_data *)&in_data);

	return ret;
}

/*___________________________________________________________________________
**      function name: ecnt_mc_api_vlan_handle
**      descriptions:
**      	multicast data vlan handle
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
int ecnt_mc_api_vlan_handle(struct sk_buff *skb)
{
	int ret = 0;
	ecnt_mc_data_s in_data;
	
	in_data.skb = skb;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_VLAN_HANDLE_HOOK,(struct ecnt_data *)&in_data);

	return ret;
}


/*___________________________________________________________________________
**      function name: ecnt_mc_api_recv
**      descriptions:
**      	recv packet from ethernet rx
**      input parameters:
**		skb: data flow
**		cpu_reason: foe cpu reason
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
int ecnt_mc_api_recv(struct sk_buff *skb,int cpu_reason)
{
	int ret = 0;
	ecnt_mc_data_s in_data;
	
	in_data.skb = skb;
	in_data.cpu_reason = cpu_reason;
	
    ret = __ECNT_HOOK(ECNT_MULTICAST,ECNT_MC_RECV_HOOK,(struct ecnt_data *)&in_data);

	return ret;
}

#endif /*_ECNT_HOOK_MULTICAST_H*/
