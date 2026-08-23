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
#ifndef _ECNT_HOOK_XFI_H_
#define _ECNT_HOOK_XFI_H_


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
#define XFI_SET_PHY_START                            0x0001
#define XFI_SET_PHY_STOP                             0x0002
#define XFI_SET_PHY_INIT                             0x0003
#define XFI_SET_PHY_RESET                            0x0004
#define XFI_SET_PHY_MODE_CONFIG                      0x0007
#define XFI_SET_PHY_TRANS_SETTING                    0x0008
#define XFI_SET_PHY_COUNTER_CLEAR                    0x0009
#define XFI_SET_PHY_TX_POWER_CONFIG                  0x000b
#define XFI_SET_PHY_TRANS_POWER_SWITCH               0x000c
#define XFI_SET_PHY_TX_BURST_CONFIG                  0x000d
#define XFI_SET_PHY_TRANS_TX_SETTINGS                0x0010
#define XFI_SET_PHY_ROGUE_PRBS_CONFIG                0x0013
#define XFI_SET_PHY_RX_FEC_SETTING                   0x0014
#define XFI_SET_PHY_TRANS_RX_SETTING                 0x0015
	
	
	
	/* Get Function */
#define XFI_GET_PHY_LOS_STATUS                       0x8001
#define XFI_GET_PHY_READY_STATUS                     0x8002
#define XFI_GET_PHY_TRANS_PARAM_TEMPRATRUE           0x8003
#define XFI_GET_PHY_TRANS_PARAM_VOLTAGE              0x8004
#define XFI_GET_PHY_TRANS_PARAM_TX_CURRENT           0x8005
#define XFI_GET_PHY_TRANS_PARAM_TX_POWER             0x8006
#define XFI_GET_PHY_TRANS_PARAM_RX_POWER             0x8007
#define XFI_GET_PHY_TX_FEC_STATUS                    0x8008
#define XFI_GET_PHY_TX_BURST_GETTING                 0x8009
#define XFI_GET_PHY_TRANS_TX                         0x800a
#define XFI_GET_PHY_RX_FEC_COUNTER                   0x800d
#define XFI_GET_PHY_RX_FRAME_COUNTER                 0x8013
#define XFI_GET_PHY_RX_FEC_STATUS                    0x8016
#define XFI_GET_PHY_RX_FEC_GETTING                   0x8017
#define XFI_GET_PHY_TRANS_RX_GETTING                 0x8018
#define XFI_GET_PHY_IS_SYNC                          0x8019
#define XFI_GET_PHY_MODE                             0x801a
#define XFI_GET_PHY_TRANS_STATUS                     0x801c
#define XFI_GET_PHY_GET_TX_POWER_EN_FLAG             0x801d
#define XFI_GET_PHY_LOF_STATUS                       0x801f
#define XFI_GET_PHY_LINK_STATUS                      0x8020




#define ECNT_DRIVER_API				(0)

#define XFI_ENABLE          		(1)
#define XFI_DISABLE         		(0)

#define XFI_PHY_LOS_HAPPEN			(1)
#define XFI_PHY_NO_LOS_HAPPEN		(0)
                            
#define XFI_TRUE            		(1)
#define XFI_FALSE           		(0)


#define XFI_NO_API          		(-1)
#define XFI_SUCCESS         		(0)
#define XFI_FAILURE         		(1)


typedef enum XFI_PHY_Event_Type_e {
    /* 
        xfi phy interrupt event 
    */
    XFI_PHY_EVENT_TRANS_LOS_INT = 0x00  ,
    XFI_PHY_EVENT_PHY_LOF_INT           ,
    XFI_PHY_EVENT_TF_INT                ,
    XFI_PHY_EVENT_TRANS_INT             ,
    XFI_PHY_EVENT_TRANS_SD_FAIL_INT     ,
    XFI_PHY_EVENT_PHYRDY_INT            , 
    XFI_PHY_EVENT_PHY_ILLG_INT          , 
    XFI_PHY_EVENT_I2CM_INT              , 
    XFI_PHY_EVENT_TRANS_LOS_ILLG_INT    , /* LOS and Illegal INT happen simultaneously */

    /* all phy interrupt event id should be less than this  */ 
    XFI_PHY_EVENT_MAX_INT =     0x100   , 

    /* 
        phy non-interrupt event 
    */

    XFI_PHY_EVENT_CALIBRATION_START     ,
    XFI_PHY_EVENT_CALIBRATION_STOP      ,
    XFI_PHY_EVENT_TX_POWER_ON           ,
    XFI_PHY_EVENT_TX_POWER_OFF          ,
    
} XFI_PHY_Event_Type_t ;



typedef enum XFI_PHY_Event_Source_e{
    XFI_PHY_EVENT_SOURCE_HW_IRQ  ,  /* event comes from hard irq*/
    XFI_PHY_EVENT_SOURCE_SW_POLL ,  /* event comes from sw irq polling */
}XFI_PHY_Event_Source_t;


typedef struct XFI_PHY_Event_data_s{
    XFI_PHY_Event_Source_t src;
    XFI_PHY_Event_Type_t   id;
} XFI_PHY_Event_data_t;


enum ECNT_XFI_PHY_SUBTYPE {
    ECNT_XFI_PHY_API,
};



typedef enum{
	XFI_PHY_FUNCTION_GET_LINK_STATUS,
	XFI_PHY_FUNCTION_GET_MODE,
	XFI_PHY_FUNCTION_SET_MODE,
	XFI_PHY_FUNCTION_MAX_NUM,
} XFI_PHY_Hook_Function_t ;


typedef enum XfiPhy_Mode_e{
    PHY_XFI_10G_CONFIG,
    PHY_XFI_5G_CONFIG,
    PHY_XFI_2P5G_CONFIG,
    PHY_UNKNOWN_CONFIG,
} Xfi_Phy_Mode_t ;

/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
 
typedef struct xfi_phy_api_data_s { 
    unsigned int  cmd_id  ;  /* [in ]    API command ID      */
    int           ret     ;  /* [out]    API return value    */
    union{                   /* [in|out]       */
        unsigned int  	* data     ;
		Xfi_Phy_Mode_t	phy_mode_cfg;
        void          	* raw      ;
    };
}xfi_phy_api_data_t,*p_xfi_phy_api_data_t;


typedef enum XFI_PHY_Los_Status_e{
    XFI_PHY_LINK_STATUS_LOS,
    XFI_PHY_LINK_STATUS_READY,
    XFI_PHY_LINK_STATUS_UNKNOWN,
}XFI_PHY_Los_Status_t;




/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/


static inline int XFI_PHY_API_GET_LINK_STATUS(void)
{
    xfi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XFI_PHY_FUNCTION_GET_LINK_STATUS;

    ret = __ECNT_HOOK(ECNT_XFI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

static inline int XFI_PHY_API_GET_MODE(void)
{
    xfi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XFI_PHY_FUNCTION_GET_MODE;

    ret = __ECNT_HOOK(ECNT_XFI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

static inline int XFI_PHY_API_SET_MODE(Xfi_Phy_Mode_t xfi_mode)
{
    xfi_phy_api_data_t in_data;
    int ret=0;
    in_data.cmd_id = XFI_PHY_FUNCTION_SET_MODE;
	in_data.phy_mode_cfg = xfi_mode;

    ret = __ECNT_HOOK(ECNT_XFI_PHY, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}










#endif /* _ECNT_HOOK_QDMA_H_ */

