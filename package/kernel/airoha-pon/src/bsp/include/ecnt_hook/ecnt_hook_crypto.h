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
#ifndef _ECNT_HOOK_CRYPTO_H_
#define _ECNT_HOOK_CRYPTO_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include "ecnt_hook.h"
#include <net/xfrm.h>
#include <net/mtk_esp.h>
#include <linux/skbuff.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ECNT_DRIVER_API  0

#define DP_CRYPTO_E_0   	100	
#define DP_CRYPTO_E_MAX   116		//modify when ipsec max entry index changes
#define DP_CRYPTO_D_0		200	
#define DP_CRYPTO_D_MAX	216		//modify when ipsec max entry index changes

typedef enum {
    CRYPTO_FUNCTION_FREE_ADAPTER = 0,
    CRYPTO_FUNCTION_DO_IPSEC_ESP_OUTPUT,
    CRYPTO_FUNCTION_DO_IPSEC_ESP_INPUT,
    CRYPTO_FUNCTION_DO_IPSEC_ESP_INPUT_PT,
    CRYPTO_FUNCTION_GET_XFRM_HEADER_LEN,
    CRYPTO_FUNCTION_GET_XFRM_INPUT_RET,
    CRYPTO_FUNCTION_ESP_OUTPUT_PT_LEARN,
    CRYPTO_FUNCTION_ESP_PKTPUT_PT_SPEED,
    CRYPTO_FUNCTION_ESP_PKTPUT_FINISH_PT_SPEED,
    CRYPTO_FUNCTION_ESP_ADAPTER_PARA_SET,
    CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG,
    CRYPTO_FUNCTION_MAX_NUM,
} CRYPTO_HookFunction_t ;

struct ECNT_CRYPTO_Data {
	CRYPTO_HookFunction_t function_id;	/* need put at first item */
	int retValue;

	struct xfrm_state *x; 
	struct sk_buff *skb;
	unsigned int xferhr_len;
	unsigned int adapter_spi;
	struct{
		int flag;
		int entry;
	} hwnat;
    unsigned int para_type;
    unsigned int escape_ip;
};


/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
static inline int CRYPTO_API_FREE_ADAPTER(unsigned int mainType, unsigned int spi){
#ifdef MTK_CRYPTO_DRIVER
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) // IPSecTransform = AH
        return ECNT_HOOK_ERROR;

    in_data.function_id = CRYPTO_FUNCTION_FREE_ADAPTER;
    in_data.adapter_spi = spi ;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
#endif
        return ECNT_HOOK_ERROR;
}

static inline int CRYPTO_API_DO_IPSEC_ESP_OUTPUT(
    unsigned int mainType, 
    struct xfrm_state *x, 
    struct sk_buff *skb
){
#ifdef MTK_CRYPTO_DRIVER    
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) // IPSecTransform = AH
        return ECNT_CONTINUE;

    in_data.function_id = CRYPTO_FUNCTION_DO_IPSEC_ESP_OUTPUT;
    in_data.x = x;
    in_data.skb = skb;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

    if(ret == ECNT_RETURN)
        return in_data.retValue;
    else
        return ECNT_CONTINUE;
#else
    return ECNT_CONTINUE;
#endif
}

static inline int CRYPTO_API_DO_IPSEC_ESP_INPUT(
    unsigned int mainType, 
    struct xfrm_state *x, 
    struct sk_buff *skb
){
#ifdef MTK_CRYPTO_DRIVER
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) // IPSecTransform = AH
        return ECNT_CONTINUE;

    in_data.function_id = CRYPTO_FUNCTION_DO_IPSEC_ESP_INPUT;
    in_data.x = x;
    in_data.skb = skb;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret == ECNT_RETURN)
        return in_data.retValue;
    else
        return ECNT_CONTINUE;
#else
    return ECNT_CONTINUE;
#endif
}

static inline int CRYPTO_API_DO_IPSEC_ESP_INPUT_PT(
    unsigned int mainType,     
    struct xfrm_state *x,
    struct sk_buff *skb
){
#if defined(TCSUPPORT_IPSEC_PASSTHROUGH) || defined(MTK_CRYPTO_DRIVER)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) // IPSecTransform = AH
        return ECNT_CONTINUE;

    in_data.function_id = CRYPTO_FUNCTION_DO_IPSEC_ESP_INPUT_PT;
    in_data.skb = skb;
    in_data.x = x;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_CONTINUE)
        return in_data.retValue;
    else
#endif		
       return ECNT_CONTINUE;
}

static inline int CRYPTO_API_GET_XFRM_HEADER_LEN(
    unsigned int mainType, 
    struct xfrm_state *x,
    unsigned int *len_p
){
#ifdef MTK_CRYPTO_DRIVER
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) // IPSecTransform = AH
        return ECNT_HOOK_ERROR;

    in_data.function_id = CRYPTO_FUNCTION_GET_XFRM_HEADER_LEN;
    in_data.x = x;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    *len_p = in_data.xferhr_len;
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
#endif
        return ECNT_HOOK_ERROR;
}

static inline int CRYPTO_API_GET_XFRM_INPUT_RET(
    unsigned int mainType,
    int retVal
){
#ifdef MTK_CRYPTO_DRIVER
    struct ECNT_CRYPTO_Data in_data_ah_flag;
    int ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ECNT_CONTINUE;

    if (retVal != -EINPROGRESS)
        return ECNT_RETURN_DROP;
#endif
    return ECNT_CONTINUE;
}

static inline int CRYPTO_API_ESP_OUTPUT_PT_LEARN(
    unsigned int mainType,     
    struct xfrm_state *x,
    struct sk_buff *skb
){
#if defined(TCSUPPORT_IPSEC_PASSTHROUGH)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ECNT_CONTINUE;
	
    in_data.function_id = CRYPTO_FUNCTION_ESP_OUTPUT_PT_LEARN;
    in_data.skb = skb;
    in_data.x = x;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_CONTINUE)
        return in_data.retValue;
    else
#endif		
        return ECNT_CONTINUE;
}


static inline int CRYPTO_API_ESP_PKTPUT_PT_SPEED(
    unsigned int mainType,     
    struct sk_buff *skb,
    int index
){
    int ret = ECNT_CONTINUE;

#if defined(TCSUPPORT_IPSEC_PASSTHROUGH)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ret;
	
    in_data.function_id = CRYPTO_FUNCTION_ESP_PKTPUT_PT_SPEED;
    in_data.skb = skb;
    in_data.hwnat.entry = index;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
#endif

	return ret;
}

static inline int CRYPTO_API_ESP_PKTPUT_FINISH_PT(
    unsigned int mainType,     
    struct sk_buff *skb,
    int index
){	
    int ret = ECNT_CONTINUE;
	
#if defined(TCSUPPORT_IPSEC_PASSTHROUGH)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ret;
	
    in_data.function_id = CRYPTO_FUNCTION_ESP_PKTPUT_FINISH_PT_SPEED;
    in_data.skb = skb;
    in_data.hwnat.entry = index;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
#endif

	return ret;
}

static inline int CRYPTO_API_ESP_ADATPDER_SET(
    unsigned int mainType,     
    struct sk_buff *skb
){
#if defined(TCSUPPORT_IPSEC_PASSTHROUGH)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ECNT_RETURN;

    in_data.function_id = CRYPTO_FUNCTION_ESP_ADAPTER_PARA_SET;
    in_data.skb = skb;
    in_data.para_type = 0;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
#endif

	return ECNT_RETURN;
}

static inline int CRYPTO_API_ESP_ADATPDER_SET_ESCAPE_IP(
    unsigned int mainType,     
    struct sk_buff *skb,
    unsigned int escape_ip
){
#if defined(TCSUPPORT_IPSEC_PASSTHROUGH)
    struct ECNT_CRYPTO_Data in_data, in_data_ah_flag;
    int ret=0, ret_ah_flag=0;

    in_data_ah_flag.function_id = CRYPTO_FUNCTION_GET_IPSEC_AH_FLAG;
    ret_ah_flag = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data_ah_flag);
    if((ret_ah_flag == 1) || (ret_ah_flag == -1)) //IPSecTransform = AH
        return ECNT_RETURN;

    in_data.function_id = CRYPTO_FUNCTION_ESP_ADAPTER_PARA_SET;
    in_data.skb = skb;
    in_data.para_type = 1;
    in_data.escape_ip = escape_ip;
    ret = __ECNT_HOOK(mainType, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
#endif

	return ECNT_RETURN;
}

#endif
