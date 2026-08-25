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
#ifndef _XMCS_IF_H
#define _XMCS_IF_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include "xpon_driver_global.h"
	
#define XMCS_RET_SUCCESS        0
#define XMCS_RET_FAILED         1
	
#define XMCS_RET_VLAN_REACH_MAX       10
#define XMCS_RET_VLAN_ALREADY_EXIST   11
#define XMCS_RET_VLAN_NOT_EXIST       12
#define XMCS_RET_VLAN_CNT_ZERO        13
	
	
/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/




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
int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo);


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
int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection);

/******************************************************************************
******************************************************************************/
int xmcs_set_epon_llid_config(uint index);

int xpon_reset_qdma_tx_buf(void );
int getPonMacfromflash(unchar * mac_addr);
void prepare_epon(void);

#endif /*_XMCS_IF_H*/
