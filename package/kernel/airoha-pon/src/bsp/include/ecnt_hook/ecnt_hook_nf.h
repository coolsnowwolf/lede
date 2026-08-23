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

#ifndef __LINUX_ENCT_HOOK_NF_H
#define __LINUX_ENCT_HOOK_NF_H
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
#include <net/netfilter/nf_conntrack.h>
#include <ecnt_hook/ecnt_hook.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
#define nf_register_hook(x) nf_register_net_hook(NULL,x)
#endif
/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ecnt_nf_ip4_subtype{
	ECNT_NF_IP4_PRE_ROUTING,
	ECNT_NF_IP4_LOCAL_IN,
	ECNT_NF_IP4_FORWARD,
	ECNT_NF_IP4_LOCAL_OUT,
	ECNT_NF_IP4_POST_ROUTING
};

enum ecnt_nf_ip6_subtype{
	ECNT_NF_IP6_PRE_ROUTING,
	ECNT_NF_IP6_LOCAL_IN,
	ECNT_NF_IP6_FORWARD,
	ECNT_NF_IP6_LOCAL_OUT,
	ECNT_NF_IP6_POST_ROUTING
};

enum ecnt_nf_br_subtype{
	ECNT_NF_BR_PRE_ROUTING,
	ECNT_NF_BR_LOCAL_IN,
	ECNT_NF_BR_FORWARD,
	ECNT_NF_BR_LOCAL_OUT,
	ECNT_NF_BR_POST_ROUTING
};

enum ecnt_nf_arp_subtype{
	ECNT_NF_ARP_IN,
	ECNT_NF_ARP_FORWARD,
	ECNT_NF_ARP_OUT
};

enum ecnt_nf_track_subtype{
	ECNT_NF_TRACK_INIT_FREE
};

struct nf_data_s{
	struct sk_buff *skb;
	const struct net_device *in;
	const struct net_device *out;
};

struct ecnt_nf_track_s{
	enum ip_conntrack_events event;
	struct nf_conn *ct;
	struct sk_buff *skb;
};

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define CALL_ECNT_NF_TRACK_INIT_HOOK(maintype, subtype, in_data) {int ret; \
	ret = __ECNT_HOOK(maintype, subtype, (struct ecnt_data *)&in_data); \
}

#define ECNT_NF_TRACK_CORE_INIT_HOOK(_evt, _ct, _skb){ \
	if (!list_empty(&ecnt_hooks[ECNT_NF_TRACK_CORE][ECNT_NF_TRACK_INIT_FREE])){\
		struct ecnt_nf_track_s net_data; \
		net_data.event = _evt; \
		net_data.ct = _ct; \
		net_data.skb = _skb; \
		CALL_ECNT_NF_TRACK_INIT_HOOK(ECNT_NF_TRACK_CORE \
							, ECNT_NF_TRACK_INIT_FREE \
							, net_data); \
	} \
}

#define ECNT_NF_TRACK_CORE_FREE_HOOK ECNT_NF_TRACK_CORE_INIT_HOOK

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
extern int ecnt_net_filter_hook_init(void);

#endif
