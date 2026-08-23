/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2016, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hybird_wan.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wei.sun		2016/10/17		Create
*/
#ifndef _HYBIRD_WAN_H_
#define _HYBIRD_WAN_H_

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)

#include <ecnt_hook/ecnt_hook.h>
#include "xmcs/xmcs_gpon.h"




#define HYBIRD_PWAN_IF_GPON0    0
#define HYBIRD_PWAN_IF_WAN0     1
#define HYBIRD_PWAN_IF_NONE     2

#define HYBIRD_ROUTE_MAC_MAX    4

typedef struct HYBIRD_Route_MAC_s{
    unsigned char valid;
    unsigned char macAddr[ETH_ADDR_LEN];
} HYBIRD_Route_MAC_t;

typedef struct {    
    HYBIRD_Route_MAC_t              routeMac[HYBIRD_ROUTE_MAC_MAX];
    struct proc_dir_entry *         route_mac_proc;
    HYBIRD_Route_MAC_t              myMac;
    spinlock_t				        macLock; /* spin lock for hybird mac read and write */
} PWAN_HybirdPriv_T ;


typedef enum {
	HYBIRD_ROUTE_MAC_ADD_ACTION = 0,
	HYBIRD_ROUTE_MAC_DEL_ACTION,
	HYBIRD_ROUTE_MAC_MAX_ACTION,
} GPON_HYBIRD_MAC_ACTION_TYPE_t;


int hybird_pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen);
int hybird_pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev);
int hybird_pwan_set_route_mac(GPON_HYBIRD_ROUTE_MAC_T * pRouteMac, GPON_HYBIRD_MAC_ACTION_TYPE_t type);
int pwan_hybird_init(void);

#endif/*TCSUPPORT_COMPILE*/
#endif /* _HYBIRD_WAN_H_ */





