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
#ifndef _ECNT_HOOK_AIR_EN8811_TYPE_H_
#define _ECNT_HOOK_AIR_EN8811_TYPE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <ecnt_hook/ecnt_hook.h>
#include <modules/eth_global_def.h>



/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifndef uint8
#define uint8	unsigned char
#endif
			
#ifndef uint16
#define uint16	unsigned short
#endif
			
#ifndef uint32
#define uint32	unsigned int
#endif

#define ECNT_DRIVER_API  0


typedef enum {
	AIR_EN8811_FUNCTION_POWER_UP=0,
	AIR_EN8811_FUNCTION_LINK_RATE=1,
    AIR_EN8811_FUNCTION_MULTI_POWER_UP  = 2,
    AIR_EN8811_FUNCTION_MULTI_LINK_RATE = 3,
	AIR_EN8811_FUNCTION_LINK_STATUS=4,
	AIR_EN8811_FUNCTION_MAX_NUM,	
} AIR_EN8811_HookFunction_t ;

/*
* speed_index: 2 means 2.5Gbps,3 means 1Gbps,4 means 100Mbps,
*              0 means link down.
* duplex: 1 means full_duplex, 0 means half_duplex.
*/
typedef struct ECNT_AIR_EN8811_Data {
	AIR_EN8811_HookFunction_t function_id;
	int retValue;
    unsigned int serdes_id;
	union {
		unsigned int power_up;
		struct {
			unsigned int speed_index;
			unsigned int duplex;
			bool autoneg_state;
			bool link_state;
		}link_info;
	} air_en8811_private;
}ECNT_AIR_EN8811_Data_s;


/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
typedef int (*air_en8811_api_op_t)(struct ECNT_AIR_EN8811_Data *en8811_data);

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_AIR_EN8811_TYPE_H_ */

