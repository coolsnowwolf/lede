/***************************************************************
Copyright Statement:

This software/firmware and related documentation ("Airoha Software")
are protected under relevant copyright laws. The information contained herein
is confidential and proprietary to Airoha Limited ("Airoha") and/or
its licensors. Without the prior written permission of Airoha and/or its licensors,
any reproduction, modification, use or disclosure of Airoha Software, and
information contained herein, in whole or in part, shall be strictly prohibited.

Airoha Limited. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS
DOCUMENTATIONS ("AIROHA SOFTWARE") RECEIVED FROM AIROHA
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS IS"
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE.
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES
MADE TO RECEIVER'S SPECIFICATION OR CONFORMING TO A PARTICULAR
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH
AIROHA SOFTWARE.
***************************************************************/

#ifndef __LINUX_ENCT_HOOK_BBF247_H
#define __LINUX_ENCT_HOOK_BBF247_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>
#include <linux/jiffies.h>
#include <ecnt_hook/ecnt_hook_qdma_type.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

#define ECNT_DRIVER_API  0

typedef enum{
	BBF247_PKT_US_HANDLE = 0,
	BBF247_PKT_DS_HANDLE,
	BBF247_SET_US_TXQ,
	BBF247_SET_DS_TXQ,
	BBF247_SET_HWNAT_TTL,
	BBF247_SET_HWNAT_QID,
	BBF247_MATCH_DS_PRI,
	BBF247_FUNC_MAX_NUM,
}BBF247_HookFunc_t;


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

typedef struct bbf247_para_s{
	struct sk_buff *skb;
	union{
		int  txq;
		int  ttl;
	};
	QDMA_TxQosScheduler_T *txQos;
}bbf247_para_t;

typedef struct BBF247_hook_data_s{
	BBF247_HookFunc_t func_id;
	bbf247_para_t     para;
} BBF247_hook_data_t,*BBF247_hook_data_ptr;


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

static inline int ECNT_API_BBF247_PKT_US_HANDLE(struct sk_buff *skb)
{
    BBF247_hook_data_t data = {0};
	ecnt_ret_val ret = ECNT_CONTINUE;

    data.func_id  = BBF247_PKT_US_HANDLE;
	data.para.skb = skb;

	ret = __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data);
    if(ret != ECNT_CONTINUE)
	{
		//printk("ECNT_CONTINUE %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_RETURN_DROP;
    }

	return ECNT_CONTINUE;
}

static inline int ECNT_API_BBF247_PKT_DS_HANDLE(struct sk_buff *skb)
{
    BBF247_hook_data_t data = {0};
	ecnt_ret_val ret = ECNT_CONTINUE;

    data.func_id  = BBF247_PKT_DS_HANDLE;
	data.para.skb = skb;

	ret = __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data);
    if(ret != ECNT_CONTINUE)
	{
        //printk("ECNT_CONTINUE %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_RETURN_DROP;
    }

	return ECNT_CONTINUE;
}

static inline int ECNT_API_BBF247_SET_US_TXQ(struct sk_buff *skb, unsigned int *txq)
{
    BBF247_hook_data_t data = {0};
    data.func_id  = BBF247_SET_US_TXQ;
	data.para.skb = skb;
	data.para.txq = -1;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data) )
	{
        //printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_CONTINUE;
    }

	if(data.para.txq >= 0 && data.para.txq <= 7) //  [0,7] valid
		*txq = data.para.txq;

	return ECNT_CONTINUE;
}

static inline int ECNT_API_BBF247_SET_DS_TXQ(struct sk_buff *skb, unsigned int *txq,  QDMA_TxQosScheduler_T *txQos)
{
    BBF247_hook_data_t data = {0};
    data.func_id  = BBF247_SET_DS_TXQ;
	data.para.skb = skb;
	data.para.txq = -1;
	data.para.txQos = txQos;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data) )
	{
        //printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_CONTINUE;
    }

	if(data.para.txq >= 0 && data.para.txq <= 7) //  [0,7] valid
		*txq = data.para.txq;

	return ECNT_CONTINUE;
}


static inline int ECNT_API_BBF247_SET_HWNAT_TTL(int *ttl)
{
    BBF247_hook_data_t data = {0};
    data.func_id  = BBF247_SET_HWNAT_TTL;
	data.para.ttl = -1;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data) )
	{
        //printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_CONTINUE;
    }

	if(data.para.ttl >= 0){
		*ttl = 0;
	}
	return ECNT_CONTINUE;
}


static inline int ECNT_API_BBF247_SET_HWNAT_QID(int *qid,int txq)
{
    BBF247_hook_data_t data = {0};
    data.func_id  = BBF247_SET_HWNAT_QID;
	data.para.txq = txq;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data) )
	{
        //printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
		return ECNT_CONTINUE;
    }

	*qid = txq;
	return ECNT_CONTINUE;
}


static inline int ECNT_API_BBF247_MATCH_DS_PBIT(int flag)
{
    BBF247_hook_data_t data = {0};
    data.func_id  = BBF247_MATCH_DS_PRI;

	if(flag == 0){
    	if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_BBF247, ECNT_DRIVER_API, (struct ecnt_data *)&data) )
		{
        	//printk("ECNT_HOOK_ERROR occur. %s:%d\n", __FUNCTION__, __LINE__);
			return ECNT_RETURN;
    	}
		return ECNT_CONTINUE;
	}else{
		return ECNT_RETURN;
	}
}


#endif // __LINUX_ENCT_HOOK_BBF247_H


