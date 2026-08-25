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
#ifndef _ECNT_HOOK_XSI_H_
#define _ECNT_HOOK_XSI_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include <ecnt_hook/ecnt_hook.h>
#include <linux/jiffies.h>


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
	/* SET function */
#define XSI_SET_PHY_START                            0x0001
#define XSI_SET_PHY_STOP                             0x0002
#define XSI_SET_PHY_INIT                             0x0003
#define XSI_SET_PHY_RESET                            0x0004
#define XSI_SET_PHY_MODE_CONFIG                      0x0007
#define XSI_SET_PHY_TRANS_SETTING                    0x0008
#define XSI_SET_PHY_COUNTER_CLEAR                    0x0009
#define XSI_SET_PHY_TX_POWER_CONFIG                  0x000b
#define XSI_SET_PHY_TRANS_POWER_SWITCH               0x000c
#define XSI_SET_PHY_TX_BURST_CONFIG                  0x000d
#define XSI_SET_PHY_TRANS_TX_SETTINGS                0x0010
#define XSI_SET_PHY_ROGUE_PRBS_CONFIG                0x0013
#define XSI_SET_PHY_RX_FEC_SETTING                   0x0014
#define XSI_SET_PHY_TRANS_RX_SETTING                 0x0015
	
	
	
	/* Get Function */
#define XSI_GET_PHY_LOS_STATUS                       0x8001
#define XSI_GET_PHY_READY_STATUS                     0x8002
#define XSI_GET_PHY_TRANS_PARAM_TEMPRATRUE           0x8003
#define XSI_GET_PHY_TRANS_PARAM_VOLTAGE              0x8004
#define XSI_GET_PHY_TRANS_PARAM_TX_CURRENT           0x8005
#define XSI_GET_PHY_TRANS_PARAM_TX_POWER             0x8006
#define XSI_GET_PHY_TRANS_PARAM_RX_POWER             0x8007
#define XSI_GET_PHY_TX_FEC_STATUS                    0x8008
#define XSI_GET_PHY_TX_BURST_GETTING                 0x8009
#define XSI_GET_PHY_TRANS_TX                         0x800a
#define XSI_GET_PHY_RX_FEC_COUNTER                   0x800d
#define XSI_GET_PHY_RX_FRAME_COUNTER                 0x8013
#define XSI_GET_PHY_RX_FEC_STATUS                    0x8016
#define XSI_GET_PHY_RX_FEC_GETTING                   0x8017
#define XSI_GET_PHY_TRANS_RX_GETTING                 0x8018
#define XSI_GET_PHY_IS_SYNC                          0x8019
#define XSI_GET_PHY_MODE                             0x801a
#define XSI_GET_PHY_TRANS_STATUS                     0x801c
#define XSI_GET_PHY_GET_TX_POWER_EN_FLAG             0x801d
#define XSI_GET_PHY_LOF_STATUS                       0x801f
#define XSI_GET_PHY_LINK_STATUS                      0x8020

#define ECNT_DRIVER_API				(0)

#define XSI_ENABLE          		(1)
#define XSI_DISABLE         		(0)

#define XSI_PHY_LOS_HAPPEN			(1)
#define XSI_PHY_NO_LOS_HAPPEN		(0)
                            
#define XSI_TRUE            		(1)
#define XSI_FALSE           		(0)

#define XSI_NO_API          		(-1)
#define XSI_SUCCESS         		(0)
#define XSI_FAILURE         		(1)

typedef enum{
	XSI_PHY_FUNCTION_GET_LINK_STATUS,
	XSI_PHY_FUNCTION_GET_MODE,
	XSI_PHY_FUNCTION_SET_MODE,
	XSI_PHY_FUNCTION_MAX_NUM,
} XSI_PHY_Hook_Function_t ;


typedef enum XsiPhy_Mode_e{
    PHY_XFI_10G_CONFIG,
    PHY_XFI_5G_CONFIG,
    PHY_XFI_2P5G_CONFIG,
    PHY_HSGMII_5G_CONFIG,
    PHY_HSGMII_2P5G_CONFIG,
    PHY_HSGMII_1G_CONFIG,    
    PHY_XSI_UNKNOWN_CONFIG,
} Xsi_Phy_Mode_t ;


typedef enum XSI_PHY_Event_Source_e{
    XSI_PHY_EVENT_SOURCE_HW_IRQ  ,  /* event comes from hard irq*/
    XSI_PHY_EVENT_SOURCE_SW_POLL ,  /* event comes from sw irq polling */
}XSI_PHY_Event_Source_t;

typedef enum XSI_PHY_Event_Type_e {
    /* 
        phy interrupt event 
    */
    PHY_EVENT_LINK_DOWN_INT = 0x00  ,
    PHY_EVENT_LINK_UP_INT            ,
    PHY_EVENT_FAIL_SYNC_XOR_INT_INT  ,
    PHY_EVENT_RX_BLOCK_LOCK_INT      ,
    PHY_EVENT_HI_BER_INTL_INT     	 ,

    /* all phy interrupt event id should be less than this  */ 
    XSI_PHY_EVENT_MAX_INT =     0x100   , 

    /* 
        phy non-interrupt event 
    */
    
} XSI_PHY_Event_Type_t ;


typedef struct XSI_PHY_Event_data_s{
    XSI_PHY_Event_Source_t src;
    XSI_PHY_Event_Type_t   id;
} XSI_PHY_Event_data_t;


/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

enum ECNT_XSI_PHY_SUBTYPE {
    ECNT_XSI_PHY_API,
};

typedef struct{
    Xsi_Phy_Mode_t   mode    ;
    //int              txEnable;
} XSI_PHY_Mode_Cfg_t;


typedef struct xsi_phy_api_data_s { 
	int             api_type  ;  /* [in ]    get or set API      */
	unsigned int  	cmd_id    ;  /* [in ]    API command ID      */
    int           	ret       ;  /* [out]    API return value    */
    union{                       /* [in|out]       */
        unsigned int  	       * data;
		Xsi_Phy_Mode_t     	   * phy_mode_cfg    ;
        void          		   * raw;
		XSI_PHY_Event_data_t   * pEvent;
    };
}xsi_phy_api_data_t,*p_xsi_phy_api_data_t;

typedef enum XSI_PHY_Los_Status_e{
    XSI_PHY_LINK_STATUS_LOS,
    XSI_PHY_LINK_STATUS_READY,
    XSI_PHY_LINK_STATUS_UNKNOWN,
}XSI_PHY_Los_Status_t;


/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/


static inline void XSI_MAC_EVENT_HANDLER(XSI_PHY_Event_data_t * pEvent)
{
    xsi_phy_api_data_t data = {0} ;
    data.pEvent      = pEvent ;
#if 1	
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XSI_PHY, ECNT_DRIVER_API, (struct ecnt_data * )&data) )	// need to confirm ECNT_DRIVER_API
	{
        panic("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
    }	
#endif
	//XSI_PHY_PRINT(XSI_PHY_MSG_DBG,"ECNT_HOOK_DONE!! \n");
}


static inline int XSI_PHY_API_GET_LINK_STATUS(void)
{
    xsi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XSI_PHY_FUNCTION_GET_LINK_STATUS;

    ret = __ECNT_HOOK(ECNT_XSI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.ret;
}

static inline int XSI_PHY_API_GET_MODE(void)
{
    xsi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XSI_PHY_FUNCTION_GET_MODE;

    ret = __ECNT_HOOK(ECNT_XSI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return in_data.ret;
}

static inline int XSI_PHY_API_SET_MODE(Xsi_Phy_Mode_t xsi_mode)
{
    xsi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XSI_PHY_FUNCTION_SET_MODE;
	int phy_mode=xsi_mode;	
	in_data.phy_mode_cfg= &phy_mode;

    ret = __ECNT_HOOK(ECNT_XSI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}


#endif /* _ECNT_HOOK_XSI_H_ */

