/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __LINUX_ENCT_HOOK_H
#define __LINUX_ENCT_HOOK_H

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
#include <linux/compiler.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               M A C R O S
*************************************************************************
*/
/* Responses from hook functions. */	
#define ECNT_MAX_SUBTYPE 	8

#define ECNT_REGISTER_FAIL 	-1
#define ECNT_REGISTER_SUCCESS		0

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef enum {
	ECNT_HOOK_ERROR = -1,
	ECNT_RETURN_DROP= 0,	/*free skb before return this type*/
	ECNT_CONTINUE ,
	ECNT_RETURN,
}ecnt_ret_val;

enum ecnt_maintype{
	ECNT_NET_CORE_DEV,
	ECNT_NET_VLAN_DEV,
	ECNT_NET_BR_FDB,
	ECNT_NET_BR_FORWARD,
	ECNT_NET_BR_INPUT,
	ECNT_NET_PPP_GENERIC,
	ECNT_NET_UDP,
	ECNT_NET_UDPV6,
	ECNT_NET_AF,
	ECNT_NET_SOCK,
	ECNT_NET_IP_OUTPUT,
	ECNT_NET_IGMP,
	ECNT_NF_BR,
	ECNT_NF_ARP,
	ECNT_NF_IPV4,
	ECNT_NF_IPV6,
	ECNT_NF_TRACK_CORE,
	ECNT_QDMA_WAN,
	ECNT_QDMA_LAN,
	ECNT_FE,
	ECNT_PPE,
	ECNT_ATM,
	ECNT_PTM,
	ECNT_ETHER_SWITCH,
	ECNT_ETHER_PHY,
	ECNT_XPON_MAC,
	ECNT_XPON_PHY,
	ECNT_QDMA_7510_20,
	ECNT_PCIE,	
	ECNT_MULTICAST,	
	ECNT_L2TP,
	ECNT_TRAFFIC_CLASSIFY,
	ECNT_SMUX,
	ECNT_VOIP,
	ECNT_CRYPTO,
	ECNT_HOOK_EVENT,
	ECNT_MULTICAST_GENERAL,	
	ECNT_IRQ_NUM,   /* please do not add Main Type before ECNT_IRQ_NUM */
	ECNT_SAR,
	ECNT_TSO_WAN,
	ECNT_TSO_LAN,
	ECNT_LRO_WAN,
	ECNT_LRO_LAN,
	ECNT_XPON_VLAN,
	ECNT_XPON_MAPPING,
	ECNT_XPON_IGMP,
	ECNT_SPI_NAND,
	ECNT_AUTOBENCH_USB,
	ECNT_AUTOBENCH_DMT,
	ECNT_UART,
	ECNT_XSI_MAC,
	ECNT_XSI_PHY,
	ECNT_IFC,
	ECNT_NET_GRE,
	ECNT_DGASP,
	ECNT_NET_VXLAN,
	ECNT_NET_IP6_OUTPUT,
    ECNT_NET_NF,
	ECNT_THERMAL_PHY,
	ECNT_GPON_FLOW,
	ECNT_VLAN,
	ECNT_VLAN_FILTER,
	ECNT_ASYM,
	ECNT_XSI_WAN_MAC,
	ECNT_HSGMII_LAN_MAC,
	ECNT_AE_WAN_MAC,
	ECNT_CPU_POWER,
	ECNT_IPSEC,
	ECNT_TOD_GEN,
	ECNT_PTP_1588,
	ECNT_LDDLA,
	ECNT_BBF247,
	ECNT_AIR_EN8811,
	ECNT_OLT_PHY,
	ECNT_OLT_XPON_MAC,
	ECNT_COMBO_PON,
	ECNT_AS21XX,
	ECNT_NUM_MAINTYPE
};

struct ecnt_data;

typedef ecnt_ret_val ecnt_hookfn(struct ecnt_data *in_data);

struct net_info_s{
	
};

struct nf_info_s{
	
};

struct ecnt_ops_info{
	unsigned int drop_num;
	union{
		struct nf_info_s nf_info;
		struct net_info_s net_info;
	};
};
struct ecnt_hook_ops {
	struct list_head list;
	unsigned int hook_id;
	struct ecnt_ops_info info;
	/* User fills in from here down. */
	const char *name;
	unsigned int is_execute;
	ecnt_hookfn *hookfn;
	unsigned int maintype;
	unsigned int subtype;
	/* Hooks are ordered in ascending priority. */
	int priority;
};

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern struct list_head ecnt_hooks[ECNT_NUM_MAINTYPE][ECNT_MAX_SUBTYPE];


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
extern void ecnt_hook_init(void);
extern int __ECNT_HOOK(unsigned int maintype, unsigned int subtype,struct ecnt_data *in_data);
extern int ecnt_register_hook(struct ecnt_hook_ops *reg);
extern void ecnt_unregister_hook(struct ecnt_hook_ops *reg);
extern int show_all_ecnt_hookfn(void);
extern int set_ecnt_hookfn_execute_or_not(unsigned int maintype, unsigned int subtype, unsigned int hook_id, unsigned int is_execute);
extern int ecnt_ops_unregister(unsigned int maintype, unsigned int subtype, unsigned int hook_id);
extern int get_ecnt_hookfn(unsigned int maintype, unsigned int subtype);
extern int ecnt_register_hooks(struct ecnt_hook_ops *reg, unsigned int n);
extern void ecnt_unregister_hooks(struct ecnt_hook_ops *reg, unsigned int n);

#endif
