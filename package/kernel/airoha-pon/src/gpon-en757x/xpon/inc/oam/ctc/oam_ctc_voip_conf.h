/*
********************************************************************************
Copyright Statement:

This software/firmware and related documentation ("EcoNet Software") are 
protected under relevant copyright laws. The information contained herein is 
confidential and proprietary to EcoNet (HK) Limited ("EcoNet") and/or its 
licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND 
AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("ECONET SOFTWARE") 
RECEIVED FROM ECONET AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN 
"AS IS" BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, WHETHER 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. NOR DOES
ECONET PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD
PARTIES WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET
SOFTWARE. RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS
RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER
LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES MADE TO
RECEIVER'S SPECIFICATION OR CONFORMING TO A PARTICULAR STANDARD OR OPEN FORUM.
RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND ECONET'S ENTIRE AND CUMULATIVE
LIABILITY WITH RESPECT TO THE ECONET SOFTWARE RELEASED HEREUNDER SHALL BE, AT
ECONET'S SOLE OPTION, TO REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR
REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET
FOR SUCH ECONET SOFTWARE.
********************************************************************************

Module Name:
    oam_ctc_voip_conf.h

Abstract:
    This file contains function declarations which oam_ctc_voip_conf.c wish to
    expose to other modules.

Revision History:
Who                  When             What
---------            -----------      -----------------------------------------
Name                 Date             Modification logs

Archer.Shi mtk06084   2013/12/26      Create

*/


#ifndef OAM_CTC_VOIP_CONF_H_
#define OAM_CTC_VOIP_CONF_H_

#include <ctc/oam_ctc_dspch.h>

void 
db_voip_conf_update_glb_param_cfg(OamCtcDB_t *pDB);

void
db_voip_conf_update_pots_status(OamCtcDB_t *pDB);


#endif // OAM_CTC_VOIP_CONF_H_


