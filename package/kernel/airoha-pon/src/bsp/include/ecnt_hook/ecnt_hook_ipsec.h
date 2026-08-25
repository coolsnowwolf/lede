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
#ifndef __ECNT_HOOK_IPSEC_H__
#define __ECNT_HOOK_IPSEC_H__

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>
#include <net/xfrm.h>
#include <linux/skbuff.h>

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define ECNT_DRIVER_API  0

#ifndef DECR_DIR
#define DECR_DIR							0
#endif
#ifndef ENCR_DIR
#define	ENCR_DIR							1
#endif
#ifndef UNKNOWN_DIR
#define UNKNOWN_DIR							2
#endif

#define ALG_SOE_NOT_SUPPORT					0
#define ALG_SOE_SUPPORT						1

#define SOE_SA_NOT_CREATE					0
#define SOE_SA_CREATE						1

#define HOP_FLAG_SOE_ENC					3
#define HOP_FLAG_SOE_DEC					4
#define HOP_FLAG_SOE_ICV_ERR				5
#define HOP_FLAG_SOE_SN_ERR					6
#define HOP_FLAG_SOE_PADDING_ERR			7
/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef enum {
	IPSEC_FUNCTION_ALG_SOE_SUPPORT = 0,
	IPSEC_FUNCTION_ALLOC_SOE_SA_ID,
	IPSEC_FUNCTION_SEND_PACKET_TO_SOE,
	IPSEC_FUNCTION_RCV_PACKET_FROM_SOE,
	IPSEC_FUNCTION_FREE_SA,
	IPSEC_FUNCTION_SLT_LOCALTEST,
	IPSEC_FUNCTION_MAX_NUM,
} IPSEC_HookFunction_t ;

typedef struct ECNT_IPSEC_Data {
	IPSEC_HookFunction_t function_id;
	int retValue;
	struct xfrm_state *x; 
	struct sk_buff *skb;
	unsigned int skb_rx_len;
	unsigned int sa_index;
	unsigned int direction;
	unsigned int hop_flags;
}ECNT_IPSEC_Data_t,*ECNT_IPSEC_Data_Ptr;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int IPSEC_API_ALG_SOE_SUPPORT(struct xfrm_state *x){
	ECNT_IPSEC_Data_t in_data;
	int ret = 0;

	in_data.function_id = IPSEC_FUNCTION_ALG_SOE_SUPPORT;
	in_data.x = x;
	ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else{
		return ECNT_HOOK_ERROR;
	}
}

static inline int IPSEC_API_ALLOC_SOE_SA_ID(struct xfrm_state *x){
    ECNT_IPSEC_Data_t in_data;
    int ret = 0;

    in_data.function_id = IPSEC_FUNCTION_ALLOC_SOE_SA_ID;
	in_data.x = x;
    ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int IPSEC_API_SEND_PACKET_TO_SOE(struct sk_buff *skb, struct xfrm_state *x, unsigned int direction){
    ECNT_IPSEC_Data_t in_data;
    int ret = 0;

    in_data.function_id = IPSEC_FUNCTION_SEND_PACKET_TO_SOE;
	in_data.skb = skb;
	in_data.x = x;
	in_data.direction = direction;
    ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret == ECNT_RETURN)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int IPSEC_API_RCV_PACKET_FROM_SOE(struct sk_buff *skb, unsigned int rx_len, unsigned int sa_index, unsigned int hop_flags){
    ECNT_IPSEC_Data_t in_data;
    int ret = 0;

    in_data.function_id = IPSEC_FUNCTION_RCV_PACKET_FROM_SOE;
	in_data.skb = skb;
	in_data.skb_rx_len = rx_len;
	in_data.sa_index = sa_index;
	in_data.hop_flags = hop_flags;
    ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int IPSEC_API_FREE_SA(struct xfrm_state *x){
    ECNT_IPSEC_Data_t in_data;
    int ret = 0;

    in_data.function_id = IPSEC_FUNCTION_FREE_SA;
	in_data.x = x;
    ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int IPSEC_API_SLT_LOCALTEST(void){
    ECNT_IPSEC_Data_t in_data;
    int ret = 0;

    in_data.function_id = IPSEC_FUNCTION_SLT_LOCALTEST;

    ret = __ECNT_HOOK(ECNT_IPSEC, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}


#endif //__ECNT_HOOK_IPSEC_H__
