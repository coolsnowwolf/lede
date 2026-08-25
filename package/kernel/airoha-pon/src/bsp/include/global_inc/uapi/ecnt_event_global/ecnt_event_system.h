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

#ifndef __UAPI_ECNT_EVENT_SYSTEM_H_
#define __UAPI_ECNT_EVENT_SYSTEM_H_

/************************************************************************
*               I N C L U D E S
************************************************************************/
#include <uapi/ecnt_event_global/ecnt_event_serdes.h>
#ifdef TCSUPPORT_BOARD_SELECT
#include <uapi/ecnt_event_global/ecnt_event_board.h>
#endif
/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
************************************************************************/

typedef enum {
	ECNT_EVENT_SNMP,
	ECNT_EVENT_WEB_LOGIN,
	ECNT_EVENT_V6_ADDR_TIMEOUT,
	ECNT_EVENT_SNMP_MAX=255
}ECNT_EVENT_SYSTEM_SubType_t;

/************************************************************************
*               M A C R O S
************************************************************************/
#define EVT_ADDR_TIMEOUT 1

/************************************************************************
*               D A T A   T Y P E S
************************************************************************/
struct web_event_data {
	unsigned char ip[64];
	unsigned char flag; /*1:login  0:logout */
	unsigned char ipver;
};

struct ecnt_ipv6_addrinfo_data
{
	int		st_code;
	char	dev_name[32];

	union {
	unsigned char		u6_addr8[16];
	unsigned short		u6_addr16[8];
	unsigned int		u6_addr32[4];
	} in6_u_val;
#define e_s6_addr			in6_u_val.u6_addr8
#define e_s6_addr16			in6_u_val.u6_addr16
#define e_s6_addr32			in6_u_val.u6_addr32
};


/************************************************************************
*               D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************/

#endif/* __UAPI_ECNT_EVENT_SYSTEM_H_ */

