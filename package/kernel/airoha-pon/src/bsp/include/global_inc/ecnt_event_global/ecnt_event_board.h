/***************************************************************
Copyright Statement:

This software/firmware and related documentation (AIROHA Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to AIROHA Limited (AIROHA) and/or 
its licensors. Without the prior written permission of AIROHA and/or its licensors, 
any reproduction, modification, use or disclosure of AIROHA Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

AIROHA Limited AIROHA. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (AIROHA SOFTWARE) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

#ifndef __ECNT_EVENT_BOARD_H_
#define __ECNT_EVENT_BOARD_H_

/************************************************************************
*               I N C L U D E S
************************************************************************/
#include <linux/types.h>
#include <uapi/ecnt_event_global/ecnt_event_system.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
************************************************************************/
#define BOARD_CFG_MAJOR					(270)

#define BOOTARGS_RFB_NO_STR		            ("rfb_no")
#define BOOTARGS_RFB_ID1_STR                ("rfb_id1")
#define BOOTARGS_RFB_ID2_STR                ("rfb_id2")
#define BOOTARGS_RFB_CFG_STR                ("rfb_cfg")

/************************************************************************
*               M A C R O S
************************************************************************/

/************************************************************************
*               D A T A   T Y P E S
************************************************************************/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************/
void get_rfb_id(RFB_ID_t *);

#endif/* __ECNT_EVENT_BOARD_H_ */

