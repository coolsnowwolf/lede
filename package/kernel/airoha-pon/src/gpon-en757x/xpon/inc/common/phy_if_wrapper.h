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


#ifndef _PHY_IF_WRAPPER_H_
#define _PHY_IF_WRAPPER_H_

#include <linux/kernel.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "drv_global.h"
#include "drv_types.h"
#include "xpondrv.h"




void XPON_PHY_SET_MODE(Xpon_Phy_Mode_t mode);
void check_sync_result(unsigned long arg);
void try_set_phy_mode(unsigned long arg);

static inline void schedule_phy_mode_auto_detection(void)
{
    gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_NOT_START ;
    XPON_START_TIMER(gpPhyData->delay_start_detect_timer);
}

static inline int should_stop_phy_mode_detect(void)
{
    return ( XPON_PHY_SYNC_STATUS_SYNCED  == gpPhyData->detect_status ||\
             XPON_PHY_SYNC_STATUS_STOPPED == gpPhyData->detect_status );
}

static inline void stop_phy_mode_detect(void)
{
    if(XPON_PHY_SYNC_STATUS_SYNCING == gpPhyData->detect_status){
        gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_STOPPED ;
    }
}

static inline int CALL_PON_PHY_ENCT_HOOK(struct xpon_phy_api_data_s * data)
{

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_PHY, ECNT_XPON_PHY_API, (struct ecnt_data * )data) ){
        panic("ECNT_HOOK_ERROR occur with cmd_id:0x%x\n", data->cmd_id);
    }

    if(PHY_NO_API == data->ret){
        panic("No such API with type:%d, cmd_id:0x%x\n", data->api_type, data->cmd_id);
    }

    return data->ret;
}

static inline int XPON_PHY_SET_API(uint cmd_id, void * in_data)
{
    struct xpon_phy_api_data_s phy_data = {0};
    
    phy_data.api_type = XPON_PHY_API_TYPE_SET;
    phy_data.cmd_id   = cmd_id  ;
    phy_data.raw     = in_data ;
    
    return CALL_PON_PHY_ENCT_HOOK(&phy_data);
}


static inline int XPON_PHY_GET_API(uint cmd_id, void * in_data) 
{
    struct xpon_phy_api_data_s phy_data = {0};
    
    phy_data.api_type = XPON_PHY_API_TYPE_GET;
    phy_data.cmd_id   = cmd_id  ;
    phy_data.raw     = in_data ;

    return CALL_PON_PHY_ENCT_HOOK(&phy_data);
}

static inline int XPON_PHY_GET(uint cmd_id) 
{
    return XPON_PHY_GET_API(cmd_id, NULL);
}


static inline int XPON_PHY_SET(uint cmd_id)
{
    return XPON_PHY_SET_API(cmd_id, NULL);
}

static inline int XPON_PHY_FW_READY_ENABLE(void)
{
    int data = PHY_ENABLE; 
    return XPON_PHY_SET_API(PON_SET_PHY_FW_READY, &data);
}

static inline int XPON_PHY_FW_READY_DISABLE(void)
{
    int data = PHY_DISABLE; 
    return XPON_PHY_SET_API(PON_SET_PHY_FW_READY, &data);
}

static inline void XPON_PHY_COUNTER_CLEAR(int counter_mask)
{
    XPON_PHY_SET_API(PON_SET_PHY_COUNTER_CLEAR, &counter_mask);
}

static inline int XPON_PHY_SET_BIT_DELAY(int bit_delay)
{
    return XPON_PHY_SET_API(PON_SET_PHY_BIT_DELAY, &bit_delay );
}

static inline void XPON_PHY_TX_POWER_CONFIG(int enable)
{
    XPON_PHY_SET_API(PON_SET_PHY_TX_POWER_CONFIG, &enable );
}

static inline void XPON_PHY_GET_TRANS_STATUS(PHY_Trans_Status_t * phy_status)
{
    XPON_PHY_GET_API(PON_GET_PHY_TRANS_STATUS, phy_status );
}

static inline int XPON_PHY_SET_ROGUE(int mode)
{
    return XPON_PHY_SET_API(PON_SET_PHY_ROGUE_PRBS_CONFIG, &mode);
}

static inline int XPON_PHY_TX_ENABLE(void)
{
    int data = PHY_ENABLE;
    return XPON_PHY_SET_API(PON_SET_PHY_TRANS_POWER_SWITCH, &data);
}

static inline int XPON_PHY_TX_DISABLE(void)
{
    int data = PHY_DISABLE;
    return XPON_PHY_SET_API(PON_SET_PHY_TRANS_POWER_SWITCH, &data);
}


static inline int XPON_PHY_SET_RX_FEC(int enable)
{
    return XPON_PHY_SET_API(PON_SET_PHY_RX_FEC_SETTING, &enable);
}

static inline int XPON_PHY_SET_TX_BURST_MODE(int mode)
{
    return  XPON_PHY_SET_API(PON_SET_PHY_TX_BURST_CONFIG, &mode);

}

static inline int XPON_PHY_SET_TRANS_RX(int mode)
{
    return XPON_PHY_SET_API(PON_SET_PHY_TRANS_RX_SETTING, &mode);
}

#ifdef TCSUPPORT_CPU_EN7521
static inline int XPON_PHY_SET_EPON_TS_CONTINUE_MODE(int mode)
{
    return XPON_PHY_SET_API(PON_SET_PHY_EPON_TS_CONTINUE_MODE, &mode);
}
#endif
#endif

