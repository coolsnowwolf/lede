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
#ifndef _ECNT_HOOK_HSGMII_MAC_TYPE_H_
#define _ECNT_HOOK_HSGMII_MAC_TYPE_H_


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
	HSGMII_LAN_FUNCTION_RX_PACKET=0,
	HSGMII_LAN_FUNCTION_GET_PHY_LINK_STATUS,
	HSGMII_LAN_FUNCTION_GET_PHY_MODE,
	HSGMII_LAN_FUNCTION_SET_PHY_MODE,
	HSGMII_LAN_FUNCTION_SET_MAC_LOGIC_RESET,
	HSGMII_LAN_FUNCTION_TX_PACKET,	
	HSGMII_LAN_FUNCTION_INGRESS_PORT_RATELIMIT,	
	HSGMII_LAN_FUNCTION_SET_MAC_MPI_MBI_DISABLE,
	HSGMII_LAN_FUNCTION_SET_MAC_MPI_MBI_ENABLE,
	HSGMII_LAN_FUNCTION_MAX_NUM,	
} HSGMII_LAN_HookFunction_t ;



typedef struct ECNT_HSGMII_Data {
	HSGMII_LAN_HookFunction_t function_id;
	int retValue;
	uint hsgmii_index;
	union {
		unsigned char mac[6];
		int speedmode;
        struct{
		    void *msg_p;
		    uint msg_len;
		    struct sk_buff *skb;
		    uint rx_len;
		}rx_info;
		struct {
			uint rate;
			uint mode;
		}rate_cfg;
		struct{
			struct sk_buff *skb;
			struct net_device *dev;
		}tx_info;
	} xsi_private;
}ECNT_HSGMII_Data_s;




/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
typedef int (*hsgmii_api_op_t)(struct ECNT_HSGMII_Data *xsi_data);

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_XSI_MAC_TYPE_H_ */

