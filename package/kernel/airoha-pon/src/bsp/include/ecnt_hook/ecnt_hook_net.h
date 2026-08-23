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

#ifndef __LINUX_ENCT_HOOK_NET_H
#define __LINUX_ENCT_HOOK_NET_H
/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/socket.h>
#include <ecnt_hook/ecnt_hook.h>
#include "../../net/bridge/br_private.h"

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ecnt_net_core_dev_subtype{
	ECNT_DEV_QUE_XMIT,
	ECNT_NETIF_RCV_SKB
};

enum ecnt_net_vlan_dev_subtype{
	ECNT_VLAN_SKB_RECV
};

enum ecnt_net_br_fdb_subtype{
	ECNT_BR_FDB_CREATE,
	ECNT_BR_FDB_DELETE,
	ECNT_BR_FDB_UPDATE,
	ECNT_BR_FDB_GET,
};

enum ecnt_net_br_forward_subtype{
	ECNT_BR_FORWARD_BR_FORWARD,
	ECNT_BR_FORWARD_BR_FLOOD,
	ECNT_BR_FORWARD_BR_PB_FLOOD,
	ECNT_BR_FORWARD_BR_FORWARD_FINISH,
};


enum ecnt_net_af_subtype{
	ECNT_NET_AF_BIND,	
	ECNT_NET_AF_PACKET_RCV,
};

enum ecnt_net_sock_subtype{
	ECNT_NET_SOCK_ALLOC,
	ECNT_NET_SOCK_RELEASE
};

enum ecnt_net_br_input_subtype
{	
	ECNT_BR_HANDLE_FRAME = 0,
};

enum ecnt_net_udp_subtype
{	
	ECNT_NET_UDP_RCV = 0,
	ECNT_NET_UDPV6_RCV,
};

enum ecnt_net_ppp_generic_subtype
{	
	ECNT_NET_PPP_XMIT = 0,
};

enum ecnt_net_ip_output_subtype
{	
	ECNT_NET_IP_LOCAL_OUT = 0,
};
enum ecnt_net_ip6_output_subtype
{	
	ECNT_NET_IP6_LOCAL_OUT = 0,
};
enum ecnt_net_igmp_subtype
{	
	ECNT_NET_IGMP_GROUP_ADDED = 0,
	ECNT_NET_IGMP_GROUP_DROPPED = 1,
};

enum ecnt_net_gre_subtype
{	
	ECNT_NET_GRE_XMIT,
	ECNT_NET_GRE_RECV,
	ECNT_NET_GRE_TUNNEL_RECV,
	ECNT_NET_GRETAG_XMIT,
	ECNT_NET_GRE_FAST_XMIT,
	ECNT_NET_GRE_NEWLINK,
	ECNT_NET_GRE_DELLINK,
	ECNT_NET_GRE_CHANGELINK,
};

enum ecnt_net_vxlan_subtype
{	
	ECNT_NET_VXLAN_XMIT,
    ECNT_NET_VXLAN_FAST_XMIT,
	ECNT_NET_VXLAN_RECV,
	ECNT_NET_VXLAN_TUNNEL_RECV,
	ECNT_NET_VXLAN_NEWLINK,
	ECNT_NET_VXLAN_DELLINK,
	ECNT_NET_VXLAN_GETLINK
};

enum ecnt_net_nf_subtype
{
    ECNT_NET_NF_DLF,
    ECNT_NET_NF_DLF_CLR,
};

struct ecnt_vlan_skb_rcv_s{
	struct net_device *vlan_dev;
	struct net_device *orig_dev;
	struct packet_type *ptype;
};

struct ecnt_br_fdb_s{
	const unsigned char *addr;
	struct net_bridge_fdb_entry *fdb;
};

struct ecnt_foe_copy_s{
	struct sk_buff **fromskb;
};

struct net_data_s{
	struct sk_buff **pskb;
	union{
		struct ecnt_vlan_skb_rcv_s vlan_skb_rcv_data;
		struct ecnt_br_fdb_s br_fdb_data;
		struct ecnt_foe_copy_s foe_copy_data;
	};
 	struct net_bridge_port *source;
    int vni;
	__be16	protocol;
	unsigned int remote_addr;
    void * device;
};

struct ecnt_net_af_bind_s{
	unsigned short sin_port;
	struct task_struct *task;
};

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define CALL_ECNT_HOOK(maintype,subtype,in_data) {int ret; \
	ret = __ECNT_HOOK(maintype, subtype, (struct ecnt_data *)&in_data); \
	if(ret == ECNT_RETURN_DROP){ \
		kfree_skb(skb); \
		return ret;} \
	if(ret == ECNT_RETURN) \
		return ret; \
}

#define ECNT_NET_HOOK(maintype, subtype, skb) { \
	struct net_data_s net_data; \
	net_data.pskb = &skb; \
	CALL_ECNT_HOOK(maintype, subtype, net_data); \
}

#define ECNT_VLAN_SKB_RCV_HOOK(skb, orig_dev, vlan_dev, ptype){ \
	struct net_data_s net_data; \
	net_data.pskb = &skb; \
	net_data.vlan_skb_rcv_data.orig_dev = orig_dev; \
	net_data.vlan_skb_rcv_data.vlan_dev = vlan_dev; \
	net_data.vlan_skb_rcv_data.ptype = ptype; \
	CALL_ECNT_HOOK(ECNT_NET_VLAN_DEV, ECNT_VLAN_SKB_RECV, net_data); \
}

#define CALL_ECNT_NET_AF_BIND_HOOK(maintype, subtype, in_data) {int ret; \
	ret = __ECNT_HOOK(maintype, subtype, (struct ecnt_data *)&in_data); \
	if(ret == ECNT_RETURN_DROP){ \
		goto out;} \
}

#define ECNT_NET_AF_BIND_HOOK(snum, tsk){ \
	if (!list_empty(&ecnt_hooks[ECNT_NET_AF][ECNT_NET_AF_BIND])){ \
		struct ecnt_net_af_bind_s net_data; \
		net_data.sin_port = snum; \
		net_data.task = tsk; \
		CALL_ECNT_NET_AF_BIND_HOOK(ECNT_NET_AF, ECNT_NET_AF_BIND, net_data); \
	} \
}

#define CALL_ECNT_NET_SOCK_ALLOC_HOOK(maintype, subtype, in_data) {int ret; \
	ret = __ECNT_HOOK(maintype, subtype, (struct ecnt_data *)&in_data); \
	if(ret == ECNT_RETURN_DROP){ \
		destroy_inode(inode); \
		return NULL;} \
}

#define ECNT_NET_SOCK_ALLOC_HOOK(tsk){ \
	if (!list_empty(&ecnt_hooks[ECNT_NET_SOCK][ECNT_NET_SOCK_ALLOC])){ \
		struct ecnt_net_af_bind_s net_data; \
		net_data.sin_port = 0; \
		net_data.task = tsk; \
		CALL_ECNT_NET_SOCK_ALLOC_HOOK(ECNT_NET_SOCK, ECNT_NET_SOCK_ALLOC \
									, net_data); \
	} \
}

#define CALL_ECNT_NET_SOCK_RELEASE_HOOK(maintype, subtype, in_data) {int ret; \
	ret = __ECNT_HOOK(maintype, subtype, (struct ecnt_data *)&in_data); \
}

#define ECNT_NET_SOCK_RELEASE_HOOK(tsk){ \
	if (!list_empty(&ecnt_hooks[ECNT_NET_SOCK][ECNT_NET_SOCK_RELEASE])){ \
		struct ecnt_net_af_bind_s net_data; \
		net_data.sin_port = 0; \
		net_data.task = tsk; \
		CALL_ECNT_NET_SOCK_RELEASE_HOOK(ECNT_NET_SOCK, ECNT_NET_SOCK_RELEASE \
								, net_data); \
	} \
}

#define ECNT_CORE_DEV_HOOK(subtype, skb){ \
	if (!list_empty(&ecnt_hooks[ECNT_NET_CORE_DEV][subtype])){ \
		struct net_data_s net_data; \
		if(skb != NULL) \
			net_data.pskb = &skb; \
		else \
			net_data.pskb = NULL; \
		CALL_ECNT_HOOK(ECNT_NET_CORE_DEV, subtype, net_data); \
	} \
}
#define ECNT_BR_FRWARD_FINISH_HOOK(subtype, skb){ \
	if (!list_empty(&ecnt_hooks[ECNT_NET_BR_FORWARD][subtype])){ \
		struct net_data_s br_data; \
		br_data.pskb = &skb; \
		__ECNT_HOOK(ECNT_NET_BR_FORWARD, subtype, (struct ecnt_data *)&br_data); \
	} \
}
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int ECNT_BR_FDB_HOOK(int subtype,  struct net_data_s *net_data){
	int ret;
	if (!list_empty(&ecnt_hooks[ECNT_NET_BR_FDB][subtype])){	
		ret = __ECNT_HOOK(ECNT_NET_BR_FDB, subtype, (struct ecnt_data *)net_data);
		return ret;
	}
	return ECNT_CONTINUE;
}

static inline int ECNT_BR_FORWARD_HOOK(int subtype, struct net_data_s *net_data){
	int ret = ECNT_CONTINUE ;

	if (!list_empty(&ecnt_hooks[ECNT_NET_BR_FORWARD][subtype]))
	{	
		ret = __ECNT_HOOK(ECNT_NET_BR_FORWARD, subtype, (struct ecnt_data *)net_data);
	}
	return ret;
}

static inline int ECNT_BR_INPUT_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_BR_INPUT][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_BR_INPUT, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_UDP_RCV_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_UDP][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_UDP, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_UDPV6_RCV_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_UDPV6][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_UDPV6, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_PPP_GENERIC_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_PPP_GENERIC][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_PPP_GENERIC, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_IP_OUTPUT_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_IP_OUTPUT][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_IP_OUTPUT, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_IP6_OUTPUT_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_IP6_OUTPUT][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_IP6_OUTPUT, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_IGMP_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_IGMP][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_IGMP, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_PACKET_RCV_HOOK(int subtype, struct sk_buff *to, struct sk_buff *from)
{
	struct net_data_s net_data;
	int ret = ECNT_CONTINUE;

	if (!list_empty(&ecnt_hooks[ECNT_NET_AF][subtype])){
		if(from != NULL)
			net_data.foe_copy_data.fromskb = &from;
		else
			net_data.foe_copy_data.fromskb = NULL;
	
		if(to != NULL)
			net_data.pskb = &to;
		else
			net_data.pskb = NULL;
		
		ret = __ECNT_HOOK(ECNT_NET_AF, subtype, (struct ecnt_data *)&net_data);
		return ret;
	}
	return ECNT_CONTINUE;
}

static inline int ECNT_GRE_GENERIC_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_GRE][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_GRE, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_VXLAN_GENERIC_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_VXLAN][subtype]))
	{
		ret = __ECNT_HOOK(ECNT_NET_VXLAN, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_VLAN_DO_RCV_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_VLAN_DEV][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_VLAN_DEV, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

static inline int ECNT_NF_GENERIC_HOOK(int subtype, struct net_data_s *data)
{	
	int ret = ECNT_CONTINUE;
	
	if (!list_empty(&ecnt_hooks[ECNT_NET_NF][subtype]))
	{

		ret = __ECNT_HOOK(ECNT_NET_NF, subtype, (struct ecnt_data *)data);
	}

	return ret;
}

#endif

