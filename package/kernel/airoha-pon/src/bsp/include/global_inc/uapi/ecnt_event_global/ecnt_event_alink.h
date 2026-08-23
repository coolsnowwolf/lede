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
#ifndef _UAPI_ECNT_EVENT_ALINK_H_
#define _UAPI_ECNT_EVENT_ALINK_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************/

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
typedef enum {
	ECNT_EVENT_ALINK_INFORM,
	ECNT_EVENT_ALINK_IF3NOTIFY,
	ECNT_EVENT_ALINK_IF6NOTIFY,
	ECNT_EVENT_ALINK_BRIDGE_DEV,
	ECNT_EVENT_ALINK_MAX=255
}ECNT_EVENT_ALINK_SubType_t;

typedef enum {
	ECNT_EVENT_IF3NOTIFY_START_BRIDGE,
	ECNT_EVENT_IF3NOTIFY_START_APCLI,
	ECNT_EVENT_IF3NOTIFY_START_APCLI_ASSIGN_SUCCESS,
	ECNT_EVENT_IF3NOTIFY_STOP,
	ECNT_EVENT_IF6NOTIFY_START,
	ECNT_EVENT_IF6NOTIFY_STOP,
	ECNT_EVENT_IF_3_6_NOTIFY_MAX=255
}ECNT_EVENT_ALINK_IF_3_6_NOTIFY_SubType_t;

/************************************************************************
*                  M A C R O S
*************************************************************************/
#define ECNT_IF3_EVENT_TYPE_AUTO  (0)
#define ECNT_IF3_EVENT_TYPE_FIXED (1)

#define ECNT_EVENT_WANMODE_BRIDGE (0)
#define ECNT_EVENT_WANMODE_ROUTE  (1)
#define ECNT_EVENT_ALINK_IF6MODE  (2)

#define ECNT_APCLI_SYNC_NONE      (0)
#define ECNT_APCLI_SYNC_ALINK     (1)
#define ECNT_APCLI_SYNC_USER      (2)

#define ECNT_NP_BR_DEV_IPVER_4	 (1)
#define ECNT_NP_BR_DEV_IPVER_6	 (2)
/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************/
typedef struct _ecnt_alink_inform_data_
{
	char		eventType[32];
	int			respCode;
	char		respCont[64];
}ecnt_alink_inform_data, *p_ecnt_alink_inform_data;

typedef struct _ecnt_alink_if_3_6_notify_data_
{
	int			eventType;
	int			wanMode;
	int			apAutoMode;
	char		apUplinkType[24];
	char		deviceMAC[16];
	int			syncSource;
}ecnt_alink_if_3_6_notify_data, *p_ecnt_alink_if_3_6_notify_data;

typedef struct _ecnt_alink_bridge_dev_data_
{
	int			ipversion;
	char		ipaddr[40];
	char		macaddr[20];
}ecnt_alink_bridge_dev_data, *p_ecnt_alink_bridge_dev_data;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/


/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************/


/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/

#endif

