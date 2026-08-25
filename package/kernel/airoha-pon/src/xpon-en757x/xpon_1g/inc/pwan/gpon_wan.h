#ifndef _GPON_WAN_H_
#define _GPON_WAN_H_

#include <ecnt_hook/ecnt_hook.h>
#include "gpon/gpon_const.h"
#include "xmcs/xmcs_if.h"
//#include "gpon/gpon.h"
#include "gpon/gpon_act.h"
#ifdef TCSUPPORT_RA_HWNAT
	#include <linux/foe_hook.h>
#endif

#ifdef TCSUPPORT_WAN_GPON
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

/*port defination*/
typedef enum {
	/*Lan port defination*/
	ECNT_PORT_DEVICE_LAN0 = 0, 
	ECNT_PORT_DEVICE_LAN1, 
	ECNT_PORT_DEVICE_LAN2, 
	ECNT_PORT_DEVICE_LAN3, 
	/*wifi port defination*/
	ECNT_PORT_DEVICE_RA0 = 10, 
	/*Wan port defination*/
	ECNT_PORT_DEVICE_PON0 = 20,
	/*USB port defination*/
	ECNT_PORT_DEVICE_USB = 30,
} ENUM_Port_Dev_Def_t ;

/*FH Vlan_Mapping callback function input para*/
typedef struct {
	struct sk_buff* skb;
	ENUM_Port_Dev_Def_t inPort;
	ENUM_Port_Dev_Def_t outPort;
} FHNet_Mapping_Vlan_Para_In_t ;

/*FH Vlan_Mapping callback function input parameter mapping ret*/
typedef struct {
	unsigned short gemportId;
	unsigned short allocId;
	unsigned char   queueId;
	unsigned char   dropFlag;
} FHNet_Mapping_Result_Out_t;

/*FH L3 Vlan_Mapping callback function input para*/
typedef struct {
	struct sk_buff * skb;
	char interfaceName[IFNAMSIZ];
}FHNet_L3Pkt_Mapping_Vlan_Para_In_t;



/*********************************************************
 *            ENUM_GWanGemCfgType_t                      *
 *  used as one param of gem port config ioctl interface *
 *  to idicate which kind of configuration the upper     *
 *  layer want to do                                     *
 *********************************************************/
typedef enum {
	ENUM_CFG_NETIDX = 1 , /* bind gem port id to some virtual ANI id*/
	ENUM_CFG_CHANNEL    , /* */ 
	ENUM_CFG_ENCRYPTION ,
	ENUM_CFG_LOOPBACK
} ENUM_GWanGemCfgType_t ;

/********************************************************
********************************************************/
typedef struct {
#ifdef TCSUPPORT_CPU_EN7521
	uint		portId			: 12 ;  /* physical gemport id, 0~4095       */
	uint		channel 		: 6 ;   /* channel number, qdma channel idx  */
	uint		ani 			: 9 ;   /* virual ANI idx, internal use only */
	uint		rxLb			: 1 ;   /* rx loop back                      */
	uint		rxEncrypt		: 1 ;
	uint		valid			: 1 ;
	uint		resv			: 2 ;
#else
	uint		portId			: 12 ;
	uint		channel			: 5 ;
	uint		ani				: 6 ;
	uint		rxLb			: 1 ;
	uint 		rxEncrypt		: 1 ;
	uint		valid			: 1 ;
	uint		resv			: 6 ;
#endif
} GWAN_GemInfo_T ;		
	
typedef struct {
	ushort								allocId[GPON_TCONT_MAX_NUM] ;
	ushort								gemIdToIndex[GPON_MAX_GEM_ID] ;	
	struct {
		GWAN_GemInfo_T					info ;
		struct net_device_stats			stats ;
		ushort 							tcontID ; /*from omci me*/
	} gemPort[GPON_GEMPORT_MAX_NUM] ;
	uint								gemNumbers ;
    ulong                               rx_omci_cnt;
	ulong                               rx_omci_extend_cnt;
	struct timer_list					gemMibTimer ;
	struct timer_list					setQdmaTxBuffTimer ;
	uint									hgu_mode_txq ;
} GWAN_Priv_T ;


typedef enum{
    GPON_TRAFFIC_UP   ,
    GPON_TRAFFIC_DOWN ,
}GPON_TRAFFIC_STATUS_t;

/********************************************************
********************************************************/
#ifdef TCSUPPORT_RA_HWNAT			
int gwan_hwnat_hook_tx(struct sk_buff *skb, struct port_info *xpon_info);
#endif
GPON_TRAFFIC_STATUS_t gwan_get_traffic_status(void);
int gwan_process_rx_message(PWAN_FERxMsg_T *pRxMsg, struct sk_buff *skb, uint pktLens, unchar *pFlag) ;
int gwan_init(GWAN_Priv_T *pGWanPriv) ;
void gwan_update_gem_mib_table(TIMER_FUN_PAAM arg) ;
int gwan_create_new_tcont(ushort allocId) ;
int gwan_remove_tcont(ushort allocId) ;
int gwan_remove_all_tcont(void) ;
int gwan_create_new_gemport(ushort gemPortId, unchar channel, unchar encryption, ushort allocId) ;
int gwan_config_gemport(ushort gemPortId, ENUM_GWanGemCfgType_t cfgType, uint value) ;
int gwan_remove_gemport(ushort gemPortId) ;
int gwan_remove_all_gemport(void) ;
int gwan_is_gemport_valid(ushort gemPortId) ;
int gwanCheckAllocIdExist(ushort allocId);
int gwan_remove_all_gemport_for_disable(void);
int gwan_switch_channel_id(void);

void fhnet_set_vlan_mapping_hook(void);
extern struct ecnt_hook_ops ecnt_FhNetDsVlanAction_op;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
extern struct ecnt_hook_ops ecnt_FhNetDsVlanAction_op;
#endif/*TCSUPPORT_COMPILE*/


#endif /* TCSUPPORT_WAN_GPON */
#endif /* _GPON_WAN_H_ */





