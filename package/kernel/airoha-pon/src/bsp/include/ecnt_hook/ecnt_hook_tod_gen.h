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

#ifndef __LINUX_ENCT_HOOK_TOD_GEN_H
#define __LINUX_ENCT_HOOK_TOD_GEN_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>
#include <linux/jiffies.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/* SET function */


#define PON_SET_PHY_START                         		0x0001 			     /* phy_start()           */



/* Get Function */

#define PON_GET_PHY_LOS_STATUS                          0x8001               /* phy_los_status()         */


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

typedef enum ECNT_TOD_GEN_SUBTYPE {
    ECNT_TOD_GEN_HOOK = 0,
}tod_gen_subtype_t;


typedef enum tod_gen_api_type_e{
   XPON_TOD_GEN_GET = 0,
   XPON_TOD_GEN_SET,
}tod_gen_api_type_t;


typedef struct xpon_tod_data_s {
	uint8_t		tod_source;
    uint32_t    superframe ;
	uint16_t   	sec_H16 ;
    uint32_t   	sec_L32 ;
    uint32_t    nanosec ;
	uint8_t		tod_add_unit_int;
	uint32_t	tod_add_unit_nume;
	uint32_t	tod_add_unit_deno;	
}xpon_tod_data_t;



typedef struct tod_gen_hook_data_s {
    tod_gen_api_type_t     api_type;
	xpon_tod_data_t 	   xpon_tod_data;
}tod_gen_hook_data_t;


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


static inline int ECNT_API_XPON_TOD_GEN_SET_HOOK(xpon_tod_data_t tod_data)
{
    tod_gen_hook_data_t data = {0};
    data.api_type  = XPON_TOD_GEN_SET;
	data.xpon_tod_data = tod_data;
            
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_TOD_GEN, ECNT_TOD_GEN_HOOK, (struct ecnt_data * )&data) )
	{
        // printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
		return -1;
    }

	return 0;
}


#endif // __LINUX_ENCT_HOOK_TOD_GEN_H


