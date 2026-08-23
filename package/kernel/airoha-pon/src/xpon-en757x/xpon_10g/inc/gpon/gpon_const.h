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
#ifndef _GPON_CONST_H
#define _GPON_CONST_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include "xpon_const.h"

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ONU_TYPE_FLASH_OFFSET 		0xff9c

//#define GPON_MAX_CHANNEL_NUMBER         (32)
//#define GPON_OMCI_INTERFACE         (0x1FF)
//#define GPON_MAX_ANI_INTERFACE      (256)

#define GPON_MULTICAST_CHANNEL              (CONFIG_GPON_10G_MAX_TCONT)
#define GPON_UNKNOWN_CHANNEL                (CONFIG_GPON_10G_MAX_TCONT + 1)
#define GPON_CURR_STATE                     (gpGponPriv->state)
#define GPON_ONU_ID                         (gpGponPriv->gponCfg.onu_id)
#define GPON_OMCC_ID                        (gpGponPriv->gponCfg.omcc)
#define GPON_UNASSIGN_ONU_ID                (0x3FF)
#define GPON_UNASSIGN_GEM_ID                (0xFFFF)
#define GPON_GEM_IDX_MASK                   (0x7FFF)
#define GPON_GEM_ENCRY_MASK                 (0x8000)

#define GPON_TAG_LENS						(8)
#define GPON_PREAMBLE_PATTERN_LENS			(36)
#define GPON_ACT_TO1_TIMER					(10000)
#define GPON_ACT_TO2_TIMER					(100)
#define GPON_ACT_TO3_TIMER					(50)
#define GPON_ACT_TO4_TIMER					(2000)
#define GPON_ACT_TO5_TIMER					(20000)
//#define GPON_ACT_TO6_TIMER					(10050)
#define GPON_HARDWARE_TIMER					(1000)
#define GPON_ACT_TOZ_TIMER					(60000)
#define GPON_SECURITY_TK4_TIMER				(100)
#define GPON_SECURITY_TK5_TIMER				(20)
#define GPON_ACT_SILENCE_TIMER				(70000)
#define GPON_SET_CHN_RETIRE_DONE_TIMER		(100)

#define JIFFIES_TIMEOUT_HZ_CNT				(300)
#define GPON_KEY_GROUP_NUM   				(2)
#define GPON_SK_LENS						(16)
#define GPON_PLOAM_IK_LENS					(16)
#define GPON_OMCI_IK_LENS					(16)
#define GPON_KEK_LENS						(16)
#define GPON_DATA_ENCRYPT_KEY_LENS			(16)

#define CONFIG_GPON_10G_MAX_TCONT			(32)
#define CONFIG_GPON_10G_MAX_GEMPORT			(256)
#define GNT_SIZE_GTC_NUM 					(0xAFC80) //about 90s

#define BWM_CHK_CTRL_SETTING                (0x00820000) 
#define BWM_LEN_LIMIT_SETTING                (0x7FFFFFFF)

#define FIFO_ERR_ENABLE_SETTING				(0x0003FFFF)
#define TX_ERR_ENABLE_SETTING				(0x00000007)
#define RX_ERR_ENABLE_SETTING				(0x00003FFF)

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

#endif /*_GPON_CONST_H*/
