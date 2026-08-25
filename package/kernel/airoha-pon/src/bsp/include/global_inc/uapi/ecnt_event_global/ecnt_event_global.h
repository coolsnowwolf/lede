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

#ifndef __UAPI_GLOBAL_ECNT_EVENT_H_
#define __UAPI_GLOBAL_ECNT_EVENT_H_

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define MAX_MSGSIZE 1024

enum ecnt_event_maintype{
	ECNT_EVENT_DSL,
	ECNT_EVENT_XPON,
	ECNT_EVENT_USB,
	ECNT_EVENT_WIFI,
	ECNT_EVENT_ETH,
	ECNT_EVENT_VOIP,
	ECNT_EVENT_PROBE,
	ECNT_EVENT_DBUS,
	ECNT_EVENT_ALINK,
	ECNT_EVENT_LANHOST,
	ECNT_EVENT_SYSTEM,
	ECNT_EVENT_VETH,	
	ECNT_EVENT_ETHER_WAN,
	ECNT_EVENT_TRAFFICMIRROR,
	ECNT_EVENT_APP_ROUTE,
	ECNT_EVENT_APP_FILTER,
	ECNT_EVENT_WADT,
	ECNT_EVENT_MAX=255
};

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*				D A T A   D E C L A R A T I O N S
*************************************************************************
*/

struct ecnt_event_data;

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
struct ecnt_event_handle {
	unsigned char	maintype;
	unsigned char	subtype;
	void		(*handle)(struct ecnt_event_data *event_data);
};

#ifdef __KERNEL__
struct ecnt_event_source{
	struct ecnt_hook_ops *opt;
};
#else
struct ecnt_event_source {
	pthread_t thread;
	int sock_fd;
};
#endif


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif/* __GLOBAL_ECNT_EVENT_H_ */

