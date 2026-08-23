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

#ifndef __LINUX_ENCT_HOOK_LDDLA_H
#define __LINUX_ENCT_HOOK_LDDLA_H

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
#define LDDLA_SET_PHY_TRAFFIC_STATUS                      0x0100 			     /* phy_set_traffic_status()*/
#define LDDLA_SET_TRANS_SWITCH_RESET         			  0x0101 			     /* phy_set_traffic_status()*/
#define LDDLA_SET_7572_TX_MODE                            0x0102                 /* AdaptivePon(UINT8 mode) */

/* Get Function */
#define LDDLA_GET_PHY_TRANS_STATUS						0xa001
#define LDDLA_GET_PHY_EN7571_VER						0xa002



/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

enum {
    LDDLA_API_TYPE_GET = 0,
    LDDLA_API_TYPE_SET = 1,
};



#define LDDLA_NO_API                              (-1)
#define LDDLA_SUCCESS                             (0)
#define LDDLA_FAILURE                             (1)



enum ECNT_LDDLA_SUBTYPE {
    ECNT_LDDLA_API,
};

typedef enum {
    LDDLA_TRAFFIC_DOWN     = 0x0,
    LDDLA_TRAFFIC_UP       = 0x1,
} ENUM_lddlaTrafficStatus;

typedef struct
{
    ushort 	 temprature;
    ushort 	 supply_voltage;
    ushort 	 tx_current;
    ushort 	 tx_power;
    ushort   rx_power;
}LDDLA_TransParam_T, *PLDDLA_TransParam_T;


typedef struct{
    LDDLA_TransParam_T params;
    uint             alarms;
} LDDLA_Trans_Status_t;



typedef struct lddla_api_data_s {
    int             api_type  ;  /* [in ]    get or set API      */
    unsigned int    cmd_id    ;  /* [in ]    API command ID      */
    int             ret       ;  /* [out]    API return value    */

    union{                       /* [in|out]                     */
        int                 * data            ;
        ENUM_lddlaTrafficStatus      * traffic_status  ;
        LDDLA_Trans_Status_t  * trasn_status    ;
        void                * raw             ;
    };
}lddla_api_data_t,*p_lddla_api_data_t;


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

#endif // __LINUX_ENCT_HOOK_PON_PHY_H


