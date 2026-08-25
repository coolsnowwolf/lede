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

#ifndef __UAPI_ECNT_EVENT_ETH_H_
#define __UAPI_ECNT_EVENT_ETH_H_

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

typedef enum {
	ECNT_EVENT_ETH_DOWN,
	ECNT_EVENT_ETH_UP,
	ECNT_EVENT_ETH_WAN_DOWN,
	ECNT_EVENT_ETH_WAN_UP,
	ECNT_EVENT_ETH_MAX=255
}ECNT_EVENT_ETH_SubType_t;

typedef enum {
	LAN1=0,
	LAN2,
	LAN3,
	LAN4,
	LAN5,
	LAN6,
	LAN7,
	LAN8,
}LAN_PORT_NUM_t;

typedef enum {
	SPEED_10M=0,
	SPEED_100M,
	SPEED_1000M,
}LAN_LINK_SPEED_t;

typedef enum {
	HALF_DUPLEX=0,
	FULL_DUPLEX,
}LAN_LINK_DUPLEX_t;

typedef enum {
	ECNT_EVENT_TRAFFICMIRROR_UPDATE,
	ECNT_EVENT_TRAFFICMIRROR_DELETE,
	ECNT_EVENT_TRAFFICMIRROR_DELETE_ALL,
	ECNT_EVENT_TRAFFICMIRROR_MAX=255
}ECNT_EVENT_MIRROR_SubType_t;

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
struct eth_event_data {
    LAN_PORT_NUM_t lan_port;
    LAN_LINK_SPEED_t link_speed;
    LAN_LINK_DUPLEX_t link_duplex;
};
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif/* __ECNT_EVENT_ETH_H_ */

