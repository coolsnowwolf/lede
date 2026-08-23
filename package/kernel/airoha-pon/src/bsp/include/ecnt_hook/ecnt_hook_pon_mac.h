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

#ifndef __LINUX_ENCT_HOOK_PON_MAC_H
#define __LINUX_ENCT_HOOK_PON_MAC_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/netdevice.h>
#include <ecnt_hook/ecnt_hook.h>


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

typedef struct PON_SET_QOS_S{
    uint8_t     enable;
    uint8_t     queueNum;
}PON_SET_QOS_t;

typedef struct WAN_XMIT_S{
    struct sk_buff *skb;
    struct net_device *dev;
    int ret;
}WAN_XMIT_t;


typedef enum Event_Src_Module_e{
    XPON_PHY_MODULE,
	XPON_PUB_MODULE,
	XPON_INT_MODULE,
}Event_Src_Module_t;

typedef enum Pub_Sub_Type_e{
	XPON_SN_SET,
	XPON_MAC_MODE_GET,
	XPON_MAC_RX_DIS_SET,
	XPON_ONU_TYPE_GET,
	XPON_TCONT_GET,
	XPON_GEMPORT_REMOVE,
	XPON_QOS_SET,
	XPON_TCONT_INFO_GET,
	XPON_GEMPORT_CREATE,
	XPON_CHANNEL_QOS_SET,
	XPON_WANLINK_CONFIG_GET,
	XPON_MULITCAST_ANI_GET,
	XPON_WAN_NET_START_XMIT,
	XPON_SLT,
	XPON_EVENT_REPORT,
	XPON_RESET,
	XPON_DBRU_SLIGHT_MODIFY,
}Pub_Sub_Type_t;

typedef enum PHY_Event_Source_e{
    PON_PHY_EVENT_SOURCE_HW_IRQ  ,  /* event comes from hard irq*/
    PON_PHY_EVENT_SOURCE_SW_POLL ,  /* event comes from sw irq polling */
    PON_PHY_EVENT_COMBO_PON_MODE ,  /* event comes from task*/ 
}PHY_Event_Source_t;

typedef enum PHY_Event_Type_e {
    /* 
        phy interrupt event 
    */
    PHY_EVENT_TRANS_LOS_INT = 0x00  ,
    PHY_EVENT_PHY_LOF_INT           ,
    PHY_EVENT_TF_INT                ,
    PHY_EVENT_TRANS_INT             ,
    PHY_EVENT_TRANS_SD_FAIL_INT     ,
    PHY_EVENT_PHYRDY_INT            , 
    PHY_EVENT_PHY_ILLG_INT          , 
    PHY_EVENT_I2CM_INT              , 
    PHY_EVENT_TRANS_LOS_ILLG_INT    , /* LOS and Illegal INT happen simultaneously */

    /* all phy interrupt event id should be less than this  */ 
    PHY_EVENT_MAX_INT =     0x100   , 

    /* 
        phy non-interrupt event 
    */
    PHY_EVENT_START_ROGUE_MODE      ,
    PHY_EVENT_STOP_ROGUE_MODE       ,

    PHY_EVENT_CALIBRATION_START     ,
    PHY_EVENT_CALIBRATION_STOP      ,
    PHY_EVENT_TX_POWER_ON           ,
    PHY_EVENT_TX_POWER_OFF          ,
    PHY_EVENT_NO_LOS_NO_READY		,
	
    PHY_EVENT_DETECT_ONLY_XGSPON    ,
    PHY_EVENT_DETECT_ONLY_GPON      ,
	PHY_EVENT_DETECT_COMBO_XGSPON   ,
	PHY_EVENT_DETECT_COMBO_GPON     ,
    
} PHY_Event_Type_t ;

typedef struct PON_PHY_Event_data_s{
    PHY_Event_Source_t src;
    PHY_Event_Type_t   id;
} PON_PHY_Event_data_t;

typedef enum ECNT_XPON_MAC_Mode_e {
    ECNT_XPON_MAC_MODE_OFF  = 0x0,
    ECNT_XPON_MAC_MODE_GPON      ,
    ECNT_XPON_MAC_MODE_EPON      ,
}ECNT_XPON_MAC_Mode_t;


typedef enum ECNT_XPON_MAC_Rx_Dis_e {
    ECNT_XPON_MAC_RX_DISABLE       = 0x0,
    ECNT_XPON_MAC_RX_ENABLE      ,
}ECNT_XPON_MAC_Rx_Dis_t;


enum ECNT_XPON_MAC_SUBTYPE {
    ECNT_XPON_MAC_HOOK,
    ECNT_XPON_CUSTOMER_HOOK,
};

enum ECNT_COMBO_PON_SUBTYPE {
    ECNT_COMBO_PON_HOOK,
};

typedef struct gemportid_to_tcont_s{
    uint gemportid;
    int *tcont;
}gemportid_to_tcont_t;

typedef struct xpon_report_id_value_s{
    uint id;
    uint vlaue;
}xpon_report_id_value_t;

typedef struct xpon_mac_Pub_info_s {
    Pub_Sub_Type_t     type;
    union {
        unsigned char sn[8];
        unsigned char * mode;
        unsigned char rx_dis;
        unsigned int onu_type;
        gemportid_to_tcont_t gemtotcont;
        xpon_report_id_value_t event;
		unsigned char dbru_modify_flag;
        PON_SET_QOS_t ponsetqos;
        unsigned short Remove_gemportID;
        int mulitcast_ani;
        void *pTcontInfo;
        void *pGemCreate;
        void *pScheduler;
        void *pSysLinkStatus;
        WAN_XMIT_t wan_xmit;

    };
}xpon_mac_Pub_info_t;

typedef struct xpon_mac_hook_data_s {
    Event_Src_Module_t     src_module;
    union {
        PON_PHY_Event_data_t   * pEvent;
        xpon_mac_Pub_info_t pub_info;
    };
}xpon_mac_hook_data_t;

/************************************************************************
*               M A C R O S
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
static inline void ECNT_API_XPON_DBRU_SLIGHT_MODIFY(unsigned char modify_flag)
{
	xpon_mac_hook_data_t data = {0} ;
	data.src_module  = XPON_PUB_MODULE;
	data.pub_info.type	= XPON_DBRU_SLIGHT_MODIFY;
	data.pub_info.dbru_modify_flag = modify_flag;
			
	if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
	}
}

static inline void ECNT_API_XPON_SN_SET(unsigned char sn[8])
{
    xpon_mac_hook_data_t data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_SN_SET;
    memcpy(data.pub_info.sn, sn, 8);
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

static inline void ECNT_API_XPON_MODE_GET(unsigned char * mode)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_MAC_MODE_GET;
    data.pub_info.mode = mode;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

static inline void ECNT_API_XPON_MAC_RX_DIS_SET(unsigned char rx_dis)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_MAC_RX_DIS_SET;
    data.pub_info.rx_dis = rx_dis;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

static inline void ECNT_API_XPON_ONU_TYPE_GET(unsigned int *onu_type)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_ONU_TYPE_GET;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }

    *onu_type = data.pub_info.onu_type;
}

static inline void ECNT_API_XPON_TCONT_GET(uint gemportid, int *tcont)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_TCONT_GET;
    data.pub_info.gemtotcont.gemportid = gemportid;
    data.pub_info.gemtotcont.tcont = tcont;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

static inline int ECNT_API_XPON_GEMPORT_REMOVE(unsigned short gemProtId)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_GEMPORT_REMOVE;
    data.pub_info.Remove_gemportID= gemProtId;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return 0;
}

static inline int ECNT_API_XPON_QOS_SET(unsigned char enable , unsigned char queueNum)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_QOS_SET;
    data.pub_info.ponsetqos.enable = enable;
    data.pub_info.ponsetqos.queueNum = queueNum;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return 0;
}


static inline int ECNT_API_XPON_TCONT_INFO_GET(void *tcontInfo)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_TCONT_INFO_GET;
    data.pub_info.pTcontInfo = tcontInfo;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return 0;
}

static inline int ECNT_API_XPON_GEMPORT_CREATE(void *GemCreate)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_GEMPORT_CREATE;
    data.pub_info.pGemCreate = GemCreate;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return 0;
}

static inline int ECNT_API_XPON_CHANNEL_QOS_SET(void *Scheduler)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_CHANNEL_QOS_SET;
    data.pub_info.pScheduler = Scheduler;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
    return 0;
}

static inline int ECNT_API_XPON_WANLINK_CONFIG_GET(void *SysLinkStatus)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_WANLINK_CONFIG_GET;
    data.pub_info.pSysLinkStatus = SysLinkStatus;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return 0;
}

static inline void ECNT_API_XPON_MULITCAST_ANI_GET(int *mulitcast_ani)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_MULITCAST_ANI_GET;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }

    *mulitcast_ani = data.pub_info.mulitcast_ani;
}

static inline int ECNT_API_XPON_WAN_NET_START_XMIT(struct sk_buff *skb, struct net_device *dev)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_WAN_NET_START_XMIT;
    data.pub_info.wan_xmit.skb = skb;
    data.pub_info.wan_xmit.dev = dev;
    data.pub_info.wan_xmit.ret = NETDEV_TX_OK;
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        return -1;
    }
    return data.pub_info.wan_xmit.ret;
}

static inline int ECNT_API_XPON_SLT(void)
{
    struct xpon_mac_hook_data_s data = {0} ;
    int ret = 0;

    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_SLT;
    
    ret = __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data);

    if(ECNT_HOOK_ERROR == ret){
    }

    return ret;
}
static inline int ECNT_API_XPON_RESET(void)
{
    struct xpon_mac_hook_data_s data = {0} ;
    int ret = 0;
    
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  = XPON_RESET;
    
    ret = __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data);

    if(ECNT_HOOK_ERROR == ret){
    }

    return ret;
}


static inline void ECNT_API_XPON_EVENT_REPORT(uint id, uint value)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PUB_MODULE;
    data.pub_info.type  =  XPON_EVENT_REPORT;
    data.pub_info.event.id = id;
    data.pub_info.event.vlaue = value;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

static inline void ECNT_API_XPON_INT_NOTIFY(void)
{
    xpon_mac_hook_data_t data = {0} ;
    data.src_module  = XPON_INT_MODULE;            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
    }
}

#endif // __LINUX_ENCT_HOOK_PON_MAC_H

