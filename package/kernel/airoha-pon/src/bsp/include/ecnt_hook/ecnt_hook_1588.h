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

#ifndef __LINUX_ENCT_HOOK_PTP_H
#define __LINUX_ENCT_HOOK_PTP_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <linux/skbuff.h>
#include <ecnt_hook/ecnt_hook.h>
#include <asm/tc3162/tc3162.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


#define ECNT_DRIVER_API  0

/* Warning: same sequence with function array 'qdma_operation' */
typedef enum { 
	ARHT_1588_NETINF_HW_TSTAMP_CONFIG = 0,
	ARHT_1588_NETINF_HW_TSTAMP_INFO,
	ARHT_1588_FUN_MAX_NUM
} PTP_HookFunc_t;

typedef struct ECNT_PTP_1588_Data {
	PTP_HookFunc_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		struct {
			struct sk_buff *skb;
			uint32 *msg0;
			uint32 *msg1;
		} ptp_check;
		struct {
			struct sk_buff *skb;
		} ptp_tx_tstamp;
		struct {
			struct sk_buff *skb;
			uint32 rxMsg;
	    } ptp_rx_tstamp;
		struct {
			struct net_device *netdev;
			struct hwtstamp_config *hwt_cfg;
		} itf_hwtstamp_config;
		struct {
			struct ethtool_ts_info *info;
		} itf_hwtstamp_info;
	} ptp_1588_private;
} ECNT_PTP_1588_Data_s;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/


static inline unsigned int PTP_1588_API_NETINF_HW_TSTAMP_CONFIG(struct net_device *netdev, struct hwtstamp_config *hwt_cfg)
{
	ECNT_PTP_1588_Data_s in_data;
	int ret = 0;

	in_data.function_id = ARHT_1588_NETINF_HW_TSTAMP_CONFIG;
	in_data.ptp_1588_private.itf_hwtstamp_config.netdev = netdev;
	in_data.ptp_1588_private.itf_hwtstamp_config.hwt_cfg = hwt_cfg;

    ret = __ECNT_HOOK(ECNT_PTP_1588, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline unsigned int PTP_1588_API_NETINF_HW_TSTAMP_INFO(struct ethtool_ts_info *info)
{
	ECNT_PTP_1588_Data_s in_data;
	int ret = 0;

	in_data.function_id = ARHT_1588_NETINF_HW_TSTAMP_INFO;
	in_data.ptp_1588_private.itf_hwtstamp_info.info = info;

    ret = __ECNT_HOOK(ECNT_PTP_1588, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

#endif
