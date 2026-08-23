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

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "../inc/common/phy_if_wrapper.h"
#include "../inc/common/drv_global.h"


XPON_PHY_Data_t * gpPhyData     = NULL ;

void XPON_PHY_SET_MODE(Xpon_Phy_Mode_t phy_mode )
{    
    PHY_Mode_Cfg_t mode_cfg = { .mode = phy_mode,  .txEnable = PHY_ENABLE };
    
    XPON_DPRINT_MSG("phy_mode:%d\n", phy_mode);

    if(PHY_GPON_CONFIG == phy_mode && gpGponPriv->emergencystate){
        mode_cfg.txEnable = PHY_DISABLE ;
    }

    XPON_PHY_SET_API(PON_SET_PHY_MODE_CONFIG,  &mode_cfg);

    XPON_PHY_SET(PON_SET_PHY_RESET_COUNTER);
}

void check_sync_result(unsigned long arg)
{
    ulong flags;
    Xpon_Phy_Mode_t current_mode = PHY_UNKNOWN_CONFIG;
    
    if( should_stop_phy_mode_detect() ) {
        return;
    }
    
    /* if fiber not plugged in, no need to change phy mode and try */
    if( PHY_LOS_HAPPEN == XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) )
    {
        XPON_START_TIMER(gpPhyData->check_sync_timer);
        return;
    }

    current_mode = XPON_PHY_GET(PON_GET_PHY_MODE) ;
    
    if( XPON_PHY_GET(PON_GET_PHY_IS_SYNC) ){
        gpPhyData->working_mode  = current_mode ;
        gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_SYNCED  ;
        printk("XPON PHY sync on %s mode\n", 
            PHY_GPON_CONFIG == current_mode  ? "GPON" : "EPON");
    }else{
        try_set_phy_mode(0);
    }
}

void try_set_phy_mode(unsigned long arg)
{
    Xpon_Phy_Mode_t detect_mode = 0;

    if( should_stop_phy_mode_detect() ) {
        return;
    }

    Xpon_Phy_Mode_t current_mode = XPON_PHY_GET(PON_GET_PHY_MODE) ;
    
    detect_mode = (PHY_GPON_CONFIG == current_mode) ? PHY_EPON_CONFIG: PHY_GPON_CONFIG;

    gpPhyData->detect_mode   = detect_mode ;
    gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_SYNCING ;

    XPON_DPRINT_MSG("[%s]detect_mode: %d\n", __FUNCTION__ ,detect_mode);

    if(PHY_GPON_CONFIG == detect_mode){
        prepare_gpon();
    }else{
        prepare_epon();
    }

    XPON_START_TIMER(gpPhyData->check_sync_timer);
}


