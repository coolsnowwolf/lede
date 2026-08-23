/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software”) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet”) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE”) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS” 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER’S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _TRAFFIC_CLASSIFY_GLOBAL_H_
#define _TRAFFIC_CLASSIFY_GLOBAL_H_


/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
//#include <linux/types.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define TRAFFIC_CLASSIFY_MAJOR   				(236)
#define CLASSIFY_MODULE_NAME   					"classify"
#define CLASSIFY_IOC_MAGIC						'e'
#define MAX_IDENIFY_VLAN_LAYER   				(4)
#define MAX_ACTION_BIT_MAP_VALUE 				(5)
#define MAX_FLD_MAP_VALUE        				(32)
#define ACL_RULE_PER_PORT        				(8)
#define TRAFFIC_CLASSIFY_PROC_DIR              "traffic_classify"
#define TRAFFIC_CLASSIFY_PROC_DEBUG_FILE       "debug"
#define DHCP_MARK_OFFSET						(0)
#define PPOE_MARK_OFFSET						(1)

typedef enum _multicast_debug_level
{
    E_NO_INFO_LEVEL      = 0,
    E_ERROR_INFO_LEVLE   = 1 << 0,
    E_CRITIC_INFO_LEVLE  = 1 << 1,
    E_NOTICE_INFO_LEVLE  = 1 << 2,
    E_WARN_INFO_LEVLE    = 1 << 3,
    E_DEBUG_INFO_LEVLE   = 1 << 4,
    E_TRACE_INFO_LEVLE   = 1 << 5,
}e_classify_debug_level;

#define TRAFFIC_CLASSIFY_ADD_ENTRY  	_IOW(CLASSIFY_IOC_MAGIC,   0, unsigned long)
#define TRAFFIC_CLASSIFY_DEL_ENTRY  	_IOW(CLASSIFY_IOC_MAGIC,   1, unsigned long)
#define TRAFFIC_CLASSIFY_CLEAN  	    _IOW(CLASSIFY_IOC_MAGIC,   2, unsigned long)

typedef enum
{
	ECN_FLD_SRC_PORT			= (1 << 0),		/* 基于源PORT 匹配*/
	ECN_FLD_SA_MAC 				= (1 << 1),  	/* 基于SA MAC分类(SA MAC)*/
	ECN_FLD_DA_MAC 				= (1 << 2),  	/* 基于DA MAC分类(DA MAC)*/
	ECN_FLD_SA_IP  				= (1 << 3),  	/* 基于源IP地址分类(SA IP)*/
	ECN_FLD_DA_IP  			 	= (1 << 4),  	/* 基于目的IP地址分类(DA IP)*/
	ECN_FLD_VLAN_ID     	 	= (1 << 5),  	/* 基于VLAN ID分类(VLAN ID)*/
	ECN_FLD_L2_ETH_TYPE 	 	= (1 << 6),  	/* 基于以太网类型(Ethernet type)*/
	ECN_FLD_IP_PROTOCOL_TYPE	= (1 << 7), 	/* 基于IP协议类型(IP protocol type)*/
	ECN_FLD_VLAN_PRIO           = (1 << 8),  	/* 基于VLAN PRIO分类(pbit)*/
	ECN_FLD_IPV4_TRAFFIC_CLASS  = (1 << 9),  	/* 基于IP TOS/DSCP（IP v4）分类(TOS/DSCP（IP v4）)*/
	ECN_FLD_L4_SA_PORT			= (1 << 10),  	/* 基于L4 源PORT分类(L4 SA PORT)*/
	ECN_FLD_L4_DA_PORT 			= (1 << 11), 	/* 基于L4 目的PORT分类(L4 DA PORT)*/
	ECN_FLD_TTL 				= (1 << 12),    /* 基于生命周期的分类(IPv4生存时间值)*/

	/*IPV6相关*/
	ECN_FLD_IPV6_DSCP			= (1 << 13),    /* 基于IP DSCP分类(IP v6)*/
	ECN_FLD_IP_VER				= (1 << 14),    /* 基于IP版本号(IPV4/IPV6)*/
	ECN_FLD_IPV6_FLOW_LABEL		= (1 << 15),    /* 基于IPv6 flow label*/  
	ECN_FLD_DA_IPV6				= (1 << 16),    /* 基于IPv6目的地址*/
	ECN_FLD_SA_IPV6				= (1 << 17),    /* 基于IPv6源地址*/  
	ECN_FLD_DA_IPV6_HEAD		= (1 << 18),    /* 基于IPV6目的地址前缀*/
	ECN_FLD_SA_IPV6_HEAD		= (1 << 19),    /* 基于IPV6源地址前缀*/
	ECN_FLD_IPV6_NEXT_HEADER	= (1 << 20),   	/* 基于IPv6 next header（如TCP、UDP、ICMPv6、IGMPv6等）分类*/
	ECN_FLD_IPV6_TRAFFIC_CLASS	= (1 << 21), 	/* 基于IPv6  traffic class*/
	ECN_FLD_IPV6_HOP_LIMIT		= (1 << 22),    /* 基于IPv6 hop limit    */
	ECN_FLD_IPV6_L4_SOURCE_PORT	= (1 << 23),	/* 基于IPv6 的L4 源PORT分类(IPv6 L4 SA PORT)    */
	ECN_FLD_IPV6_L4_DEST_PORT	= (1 << 24),  	/* 基于IPv6 的L4 目的PORT分类(IPv6 L4 DA PORT) */
	/*以下暂不支持*/
	ECN_FLD_PHY_DEST_PORT		= (1 << 25),	/* 基于物理目的端口分类(Phy Dest Port)*/
	ECN_FLD_DHCPV6_MESSAGE_TYPE = (1 << 26),	/* 基于DHCPv6消息类型*/
		
	/* IP mask & port range for CMCCV2 */
	ECN_FLD_DEST_IP_MASK_PORT_RANGE_TYPE = (1 << 27),
	ECN_FLD_SRC_IP_MASK_PORT_RANGE_TYPE = (1 << 28),
	ECN_FLD_DEST_IPV6_MASK_PORT_RANGE_TYPE = (1 << 29),
	ECN_FLD_SRC_IPV6_MASK_PORT_RANGE_TYPE = (1 << 30),

}ecnt_fld_bit_map;


#define ECN_FLD_PORT_RANGE_TYPE (ECN_FLD_DEST_IP_MASK_PORT_RANGE_TYPE | ECN_FLD_SRC_IP_MASK_PORT_RANGE_TYPE | ECN_FLD_DEST_IPV6_MASK_PORT_RANGE_TYPE | ECN_FLD_SRC_IPV6_MASK_PORT_RANGE_TYPE)

#ifndef __KERNEL__
#define ETH_ALEN  6  

typedef enum _port_list_
{
	E_ETH_PORT_0  = 0,
	E_ETH_PORT_1  = 1,
	E_ETH_PORT_2  = 2,
	E_ETH_PORT_3  = 3,
	
	E_WIFI_PORT_0 = 4,
	E_WIFI_PORT_1 = 5,
	E_WIFI_PORT_2 = 6,
	E_WIFI_PORT_3 = 7,
	
	E_WAN_PORT    = 15,
	E_MAX_PORT    = 16,
}e_port_list;
#endif

typedef enum
{
	E_IPV4_PACKET = 4,
	E_IPV6_PACKET = 6,
}e_ip_version;

typedef enum {
	ACL_RULE_LIST,
	QOS_RUEL_LIST
}e_rule_type;

typedef enum
{
	E_MATCH_FLD_BIT       = 1,
	E_NOT_MATCH_FLD_BIT   = 2,
	E_MATCH_FLD_ENTRY     = 3,
	E_NOT_MATCH_FLD_ENTRY = 4,
}e_match_fld_type;

typedef enum
{
	E_MATCH_IP_IGNORE		= 0,
	E_MATCH_IP_ONLY_TYPE	= 1,
	E_MATCH_IP_MASK_TYPE	= 2,
	E_MATCH_IP_ALL_TYPE		= 3,
}e_match_ip_type;

typedef enum
{
	E_MATCH_PORT_IGNORE			= 0,
	E_MATCH_PORT_ONLY_TYPE		= 1,
	E_MATCH_EXCLUDE_PORT_TYPE	= 2,
	E_MATCH_PORT_RANGE_TYPE		= 3,
	E_MATCH_PORT_ALL_TYPE		= 4,
}e_match_port_type;


typedef struct _ecn_fld_element_
{
	unsigned int 	    	 		     fld_bit_map;
	unsigned char  	 			   dst_mac[ETH_ALEN];
	unsigned char    			   src_mac[ETH_ALEN];
	struct in_addr  					 ipv4_src_ip;
	struct in_addr  					 ipv4_dst_ip;
	unsigned short 								 vid;
	unsigned short 	       				  ether_type;
	unsigned char 								pbit;
	unsigned char 								 tos;
	unsigned int 					l4_ipv4_src_port;
	unsigned int 					l4_ipv4_dst_port;
	unsigned char   					    ipv4_ttl;
	/* ip & port mask mode for CMCC start */
	unsigned int 				    ipv4_src_ip_mask;
	unsigned int 				    ipv4_dst_ip_mask;
	unsigned short 				   ipv4_src_port_end;
	unsigned short 				   ipv4_dst_port_end;
	e_match_ip_type				  ipv4_ip_match_type;
	e_match_port_type			ipv4_port_match_type;
	/* ip & port mask mode for CMCC end */
	/*ipv6_dscp : IPv6  traffic class  & 0xFC*/
	unsigned char			     		   ipv6_dscp; 
	unsigned char 						  ip_version;
	unsigned char 						 flow_lbl[3];
	struct in6_addr 					  ipv6_saddr;
	struct in6_addr 					  ipv6_daddr;
	unsigned char 							 nexthdr;
	unsigned char				  ipv6_traffic_class;
	unsigned char 						   hop_limit;
	unsigned int 					l4_ipv6_src_port;
	unsigned int 					l4_ipv6_dst_port;
	/* ipv6 & port mask mode for CMCC start */
	unsigned short 				 ipv6_src_ip_mask[8];
	unsigned short 				 ipv6_dst_ip_mask[8];
	unsigned short 				   ipv6_src_port_end;
	unsigned short 				   ipv6_dst_port_end;
	e_match_ip_type				  ipv6_ip_match_type;
	e_match_port_type			ipv6_port_match_type;
	/* ipv6 & port mask mode for CMCC end */
	struct in6_addr 				  ipv6_saddr_hdr;
	struct in6_addr 				  ipv6_daddr_hdr;
	unsigned char                    dhcpv6_msg_type;
	unsigned char                            ip_type; 
}ecn_fld_element_t, *pt_ecn_fld_element;

typedef enum _classify_action_enum_
{
	E_ACL_ACTION			 = (1 << 0),
	E_QUEUE_MAPPING_ACTION   = (1 << 1),
	E_PRIORITY_REMARK_ACTION = (1 << 2),
	E_LIMIT_ACTION           = (1 << 3),
	E_TOS_REMARK_ACTION		 = (1 << 4),
}classify_action_enum;

#define CLASSIFY_ACTION_MASK  	  (E_ACL_ACTION | E_QUEUE_MAPPING_ACTION | E_PRIORITY_REMARK_ACTION | E_LIMIT_ACTION | E_TOS_REMARK_ACTION)
#define CLASSIFY_QOS_ACTION_MASK  (E_QUEUE_MAPPING_ACTION | E_PRIORITY_REMARK_ACTION | E_LIMIT_ACTION | E_TOS_REMARK_ACTION)


typedef enum
{
	E_FORWARD = 0,
	E_DROP 	  = 1,
}e_acl_mode;

typedef struct _classify_action_info_
{
	unsigned char 			action_bit_map;
	unsigned char 	 		remark_pbit;
	unsigned char  	 		remark_queue;
	unsigned int     		limit_group_id;
	unsigned int     		remark_tos;
	e_acl_mode				acl_mode;
}ecn_classify_action_info_t, *pt_ecn_classify_action_info;

typedef struct _ecn_traffic_classify_info_
{
	e_port_list					       port;
	ecn_fld_element_t			fld_element;
	ecn_classify_action_info_t	act_element;

}ecn_traffic_classify_info_t, *pt_ecn_traffic_classify_info;

#ifdef __KERNEL__
typedef struct _ecn_traffic_classify_list_
{
	struct list_head    		  list;
	struct rcu_head               head;
	ecn_traffic_classify_info_t  entry;
}ecn_traffic_classify_list_t, *pt_ecn_traffic_classify_list;
#endif

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/


#endif /* _TRAFFIC_CLASSIFY_GLOBAL_H_ */


