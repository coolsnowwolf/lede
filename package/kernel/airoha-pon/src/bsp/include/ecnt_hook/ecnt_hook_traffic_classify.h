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

#ifndef __LINUX_ENCT_TRAFFIC_CLASSIFY_H
#define __LINUX_ENCT_TRAFFIC_CLASSIFY_H
/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <ecnt_hook/ecnt_hook.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define PORT_MASK 					(0xF)
#define PORT_MASK_OFFSET 			(0x1)

#define QUQUE_REMARK_MASK   		(0x7)
#define QUEUE_REMARK_OFFSET 		(0xB)

#define LIMIT_GROUP_REMARK_MASK   	(0x3F)
#define LIMIT_GROUP_REMARK_OFFSET 	(0x5)

#define PRIO_REMARK_MASK   			(0x7)

#define QUQUE_REMARK_EXIST_OFFSET   (0x10)
#define PRIO_REMARK_EXIST_OFFSET    (0xF)

#define QUEUE_MARK_MASK   			(0x0F)
#define QUEUE_FROM_LAN				(0x0E)
#define QUEUE_FROM_WAN				(0x0F)
#define QUEUE_MAX_NUM				(0x8)
#define QUEUE_DSCP_MASK   			(0x7F0)
#define QUEUE_DSCP_MASK_OFFSET  	(0x4)

#define SKB_FROM_LAN_MASK           (0x1)
#define SKB_FROM_LAN_OFFSET         (0x11)

#define BRIDGE_WAN_NAME				"nas0"

typedef enum _port_list_
{
	E_ETH_PORT_0  = 0,
	E_ETH_PORT_1  = 1,
	E_ETH_PORT_2  = 2,
	E_ETH_PORT_3  = 3,
	
	E_WIFI_PORT_0 = 4,
	E_WIFI_PORT_1 = 5,
	E_WIFI_PORT_2 = 6,
	E_WIFI_PORT_3 = 7,
	
	E_WAN_PORT    = 15,
	E_MAX_PORT    = 16,
}e_port_list;


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ecnt_traffic_classify_subtype
{
	/*Traffic classify control based on Filed*/
	ECNT_TCCBF_SUBTYPE = 0,
};

enum _ecn_forward_drop_
{
	E_ECNT_FORWARD = 0,
	E_ECNT_DROP    = 1 ,
};


typedef struct _traffic_classify_data_s{
	struct sk_buff *skb;
	unsigned char  *forward;
	
}traffic_classify_data_t;

/************************************************************************
*               M A C R O S
*************************************************************************
*/
static inline int ECNT_TRAFFIC_CLASSIFY_HOOK (struct sk_buff *_skb, unsigned char *_f)
{
	int ret;
	traffic_classify_data_t data; 
	memset(&data, 0, sizeof(data));
	
	if(NULL != _skb && NULL != _f)
	{
		data.skb     = _skb;
		data.forward = _f;
	}
	else
	{
		return 0;
	}
	
	ret = __ECNT_HOOK(ECNT_TRAFFIC_CLASSIFY, ECNT_TCCBF_SUBTYPE, (struct ecnt_data *)&data);
	
	return ret;
}


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/

#endif
