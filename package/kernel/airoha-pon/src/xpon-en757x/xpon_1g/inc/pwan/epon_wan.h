#ifndef _EPON_WAN_H_
#define _EPON_WAN_H_

#include "common/drv_types.h"

#ifdef TCSUPPORT_WAN_EPON

/********************************************************
********************************************************/
typedef struct {
	uint		llid			: 16 ;
	uint		channel			: 4 ;
	uint		rxDrop			: 1 ;
	uint		rxLb			: 1 ;
	uint		txDrop			: 1 ;
	uint		valid			: 1 ;
	uint		resv			: 8 ;
} EWAN_LlidInfo_T ;
		
typedef struct {
	struct {
		EWAN_LlidInfo_T					info ;
		struct net_device_stats			stats ;
	} llid[EPON_1G_MAX_LLID_NUM] ;
} EWAN_Priv_T ;

/********************************************************
********************************************************/
#ifdef TCSUPPORT_RA_HWNAT			
int ewan_hwnat_hook_tx(struct sk_buff *skb, struct port_info *xpon_info);
#endif
int ewan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag) ;
int ewan_init(EWAN_Priv_T *pEWanPriv) ;

#endif /* TCSUPPORT_WAN_EPON */
#endif /* _EPON_WAN_H_ */





