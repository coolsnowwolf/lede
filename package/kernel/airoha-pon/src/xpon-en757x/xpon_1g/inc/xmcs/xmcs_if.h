#ifndef _XMCS_IF_H_
#define _XMCS_IF_H_

#include "xpon_driver_global.h"

#define XMCS_RET_SUCCESS        0
#define XMCS_RET_FAILED         1

#define XMCS_RET_VLAN_REACH_MAX       10
#define XMCS_RET_VLAN_ALREADY_EXIST   11
#define XMCS_RET_VLAN_NOT_EXIST       12
#define XMCS_RET_VLAN_CNT_ZERO        13

typedef enum {
	XPON_AUTO_MODE,
	XPON_FIX_MODE,
}preXponMode_t;

/******************************************************************************
 Descriptor:	get the current wan interface mode
 Input Args:	the pointer of WAN link status.
 				arg1: system connection mode
 				arg2: wan detection mode
 				arg3: wan link status
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_get_wan_link_status(struct XMCS_WanLinkConfig_S *pSysLinkStatus) ;
int xpon_set_qos(u8 enable, u8 queueNum);
int xmcs_set_channel_scheduler(struct XMCS_ChannelQoS_S *pScheduler);
int xmcs_get_onu_type(XMCSIF_OnuType_t *type);
int xmcs_get_onu_mac(XMCSIF_OnuMac_t *onuMac);


#ifdef TCSUPPORT_WAN_GPON
/******************************************************************************
 Descriptor:	create a new gemport and mapping gem port to tcont
 Input Args:	the pointer of gem port create struct.
 				arg1: gem port id
 				arg2: tcont id
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_create_gem_port(struct XMCS_GemPortCreate_S *pGemCreate) ;

/******************************************************************************
 Descriptor:	remove an exist gem port
 Input Args:	arg1: gem port id
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_remove_gem_port(ushort gemPortId) ;

/******************************************************************************
 Descriptor:	remove all exist gem port
 Input Args:	N/A
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_remove_all_gem_port(void) ;

/******************************************************************************
 Descriptor:	get all exist gem port information
 Input Args:	the pointer of gem port info struct
 				arg1: gem info, include: port id, alloc id, if index, 
 				      loopback mode and encryption mode
 				arg2: entry number for gem info
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_get_gem_port_info(struct XMCS_GemPortInfo_S *pGemInfo) ;

/******************************************************************************
 Descriptor:	get all exist tcont information
 Input Args:	the pointer of tcont info struct
 				arg1: tcont info, include: alloc id, channel number
 				arg2: entry number for tcont info
 Ret Value: 	success for 0 and fail for <0
******************************************************************************/
int xmcs_get_tcont_info(struct XMCS_TcontInfo_S *pTcontInfo) ;

/***************************************************************
***************************************************************/
int xmcs_create_tcont_info(struct XMCS_TcontCfg_S *pTcontInfo) ;

/***************************************************************
***************************************************************/
int xmcs_remove_tcont_info(ushort allocId) ;

/***************************************************************
***************************************************************/
			
int xmcs_check_allocid_exist(ushort allocId);
	
/***************************************************************
***************************************************************/

void showTransVlanInfo(void);
void assignGemportId(ushort gemPortId);


/***************************************************************
***************************************************************/
int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo);

int xmcs_set_rogue_state(unchar rogue_mode);

#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
/***************************************************************
 Descriptor:	Create a new LLID. After creating the LLID, the 
 				net interface will be allocated automatically. 
 Input Args:	the pointer of LLID create struct.
 				arg1: the index of the llid.
 				arg2: llid value
 Ret Value: 	success for 0 and fail for <0
***************************************************************/
int xmcs_create_llid(struct XMCS_EponLlidCreate_S *pLlidCreate) ;

/***************************************************************
 Descriptor:	Remove the LLID. After removing the LLID, the 
 				corresponding net interface will be deleted.
 Input Args:	the index of the llid
 Ret Value: 	success for 0 and fail for <0
***************************************************************/
int xmcs_remove_llid(unchar idx) ;

#endif /* TCSUPPORT_WAN_EPON */

/******************************************************************************
******************************************************************************/
int if_cmd_proc(uint cmd, ulong arg) ;

/******************************************************************************
******************************************************************************/
int xmcs_set_connection_start(XPON_Mode_t mode);

/******************************************************************************
******************************************************************************/
int xmcs_set_epon_llid_config(uint index);

int xpon_reset_qdma_tx_buf(void );
int xmcs_get_llid_info(struct XMCS_EponLlidInfo_S *pLlidInfo);


void prepare_epon(preXponMode_t mode);

#endif /* _XMCS_IF_H_ */

